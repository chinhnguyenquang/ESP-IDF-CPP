#include "Wifi_pro.h"


namespace WIFI
{

// Wifi statics
char                                        Wifi_pro::mac_addr_cstr[]{};    
std::mutex                                  Wifi_pro::init_mutx{};   
std::mutex                                  Wifi_pro::state_mutx{};
char                                        Wifi_pro::service_name[12] = {0};       
Wifi_pro::state_e                           Wifi_pro::_state{state_e::NOT_INITIALISED};                 ///< Current WiFi state
wifi_init_config_t                          Wifi_pro::wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();    ///< WiFi init config
wifi_config_t                               Wifi_pro::wifi_config{};        ///< WiFi config containing SSID & password
wifi_prov_mgr_config_t                      Wifi_pro::wifi_ble_config{};
char                                        Wifi_pro::qr_payload[100]={0};
uint8_t                                     Wifi_pro::wifipro_retries{0};

// Constructor
Wifi_pro::Wifi_pro(void)
{
    std::lock_guard<std::mutex> guard(init_mutx);

    if (!get_mac()[0])
    {
        if (ESP_OK != _get_mac())
            esp_restart();
        else {
            snprintf(service_name, sizeof(service_name),
                                                        "PROV_%.6s",
                                                        &mac_addr_cstr[6]);
            
            snprintf(qr_payload, sizeof(qr_payload),
        "{\"ver\":\"v1\",\"name\":\"%s\",\"pop\":\"%s\",\"transport\":\"ble\"}",
        service_name,
        pop);


            ESP_LOGI("QR", "QR payload: %s", qr_payload);
            ESP_LOGI("WIFI", "If QR code is not visible, copy paste the below URL in a browser.\n%s?data=%s", QRCODE_BASE_URL, qr_payload);
        }
    }
}



// Get MAC
esp_err_t Wifi_pro::_get_mac(void)
{
    uint8_t mac[6]{};

    const esp_err_t status =
        esp_efuse_mac_get_default(mac);

    if (status == ESP_OK)
    {
        snprintf(mac_addr_cstr,
                 sizeof(mac_addr_cstr),
                 "%02X%02X%02X%02X%02X%02X",
                 mac[0], mac[1], mac[2],
                 mac[3], mac[4], mac[5]);
    }

    return status;
}

    void Wifi_pro::RSSI_value(uint8_t *rssi)
    {
        if(esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK)
        { 
            int8_t rssi_real= ap_info.rssi;  

            if (rssi_real > -50) *rssi =100;
            else if (rssi_real < -70) *rssi = 0;
            else *rssi = 2*(rssi_real + 100);
        }
        else *rssi = 0; 
    }

esp_err_t Wifi_pro::endpointHandler(uint32_t session_id,
                                     const uint8_t *inbuf,
                                     ssize_t inlen,
                                     uint8_t **outbuf,
                                     ssize_t *outlen,
                                     void *priv_data)
{
    Wifi_pro* self = static_cast<Wifi_pro*>(priv_data);

    if(self && self->endpoint_cb)
    {
        self->endpoint_cb(inbuf, inlen);
    }

     if (inbuf) {
        ESP_LOGI("HANDLED ENDPOINT", "Received data: %.*s", inlen, (char *)inbuf);
    }
    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL) {
        ESP_LOGE("HANDLED ENDPOINT", "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}



esp_err_t Wifi_pro::_init(void)
{
    std::lock_guard<std::mutex> init_guard(init_mutx);

    esp_err_t status{ESP_OK};

    //std::lock_guard<std::mutex> state_guard(state_mutx);


    if(state_e::NOT_INITIALISED==_state)
    {
        status = esp_netif_init();

        

        if(ESP_OK==status){
            ESP_LOGI("WIFI", "Creating handled event ...");
            status= esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &pro_event_handler, NULL);
            status |= esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
            status |= esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, this);
            if (ESP_OK != status ){
                ESP_LOGI("WIFI", "DANG KI HANDLED BI LOI ....");
                _state=state_e::ERROR;
                return ESP_FAIL;
            }
        }

        
        const esp_netif_t* const p_netif = esp_netif_create_default_wifi_sta();

        if (!p_netif) 
        {
            ESP_LOGI("WIFI","Faile esp_netif_create_default_wifi_sta");
            _state=state_e::ERROR;
            return ESP_FAIL;
        }
        status = esp_wifi_init(&wifi_init_config);
        if (status != ESP_OK) {
            ESP_LOGE("WIFI", "esp_wifi_init failed: %s", esp_err_to_name(status));
            _state = state_e::ERROR;
            return status;
        }

        /* Wi-Fi must be started before provisioning can run. */
        wifi_config.sta.threshold.authmode  = WIFI_AUTH_WPA2_PSK;
        wifi_config.sta.pmf_cfg.capable     = true;
        wifi_config.sta.pmf_cfg.required    = false;
        
        // Config ble provision ......
        wifi_ble_config.scheme              = wifi_prov_scheme_ble;
        wifi_ble_config.scheme_event_handler=WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BLE;

        status = wifi_prov_mgr_init(wifi_ble_config);


        bool provisioned = false;

            ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

            if(!provisioned)
            {
                ESP_LOGI("_init wifi", "Starting provisioning");


                wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
                
               
                uint8_t custom_service_uuid[] = {
                        0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
                        0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
                    };
                wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);
                wifi_prov_mgr_endpoint_create("custom-data");

                ESP_LOGI("WIFI", "Calling wifi_prov_mgr_start_provisioning ...");
                esp_err_t prov_status = wifi_prov_mgr_start_provisioning(security, pop, service_name, NULL);
                ESP_LOGI("WIFI", "wifi_prov_mgr_start_provisioning returned %s", esp_err_to_name(prov_status));

                wifi_prov_mgr_endpoint_register("custom-data", endpointHandler, this);

            }else{
                ESP_LOGI("WIFI", "Already provisioned");
                wifi_prov_mgr_deinit();
                /* Start Wi-Fi in station mode */
                ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
                ESP_ERROR_CHECK(esp_wifi_start());

            }

            if (ESP_OK == status) _state = state_e::INITIALISED;


    }



    return status;
}

void Wifi_pro::wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (WIFI_EVENT == event_base)
    {
    
       switch(event_id)
       {
            case(WIFI_EVENT_STA_START):
                {
                    ESP_LOGI("WIFI","WIFI EVENT STA START-> esp_wifi_connect");
                    esp_wifi_connect();
                    std::lock_guard<std::mutex>state_guard(state_mutx);
                    _state = state_e::CONNECTING;
                    break;
                    
                }
            case(WIFI_EVENT_STA_CONNECTED):
            {
                std::lock_guard<std::mutex>state_guard(state_mutx);
                _state = state_e ::WAITING_FOR_IP;
                ESP_LOGI("WIFI", "Waititng for ip");
                break;

            }
            case(WIFI_EVENT_STA_DISCONNECTED):
            {
                ESP_LOGI("WIFI", "Disconnected. Connecting to the AP again...");
                esp_wifi_connect();
                std::lock_guard<std::mutex>state_guard(state_mutx);
                _state = state_e::DISCONNECTED;
                break;
            }
            default:
                //
                ESP_LOGW("WIFI", "%s:%d Default switch case (%" PRId32 ")", __func__, __LINE__, event_id);
                break;
       }

    }
}





void Wifi_pro::ip_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (IP_EVENT == event_base)
    {
        
        const ip_event_t event_type{static_cast<ip_event_t>(event_id)};
        switch(event_type)
        {
        case IP_EVENT_STA_GOT_IP:
        {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            std::lock_guard<std::mutex>state_guard(state_mutx);
            _state = state_e::CONNECTED;
            ESP_LOGI("IP", "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
            break;
        }
        case IP_EVENT_STA_LOST_IP:
        {
            std::lock_guard<std::mutex>state_guard(state_mutx);
            _state = state_e::WAITING_FOR_IP;
            ESP_LOGI("IP", "%s:%d WAITING_FOR_IP", __func__, __LINE__);
            break;
        }

        default:
            // TODO IP6
            ESP_LOGW("IP", "%s:%d Default switch case (%" PRId32 ")", __func__, __LINE__, event_id);
            break;
        }

        if (IP_EVENT_STA_GOT_IP==event_id)
        {
            auto instance = static_cast<Wifi_pro*>(arg);
                
                // ✅ Kiểm tra instance trước
            if (instance && instance->connected_cb)
            {
                ESP_LOGI("WIFI", "Calling connected callback");
                instance->connected_cb();
            }
            else
            {
                // ✅ Log lý do không vào if
                if (!instance) {
                    ESP_LOGE("WIFI", "instance is NULL!");
                }
                else if (!instance->connected_cb) {
                    ESP_LOGE("WIFI", "connected_cb is not set or empty!");
                    ESP_LOGE("WIFI", "connected_cb address: %p", 
                            reinterpret_cast<void*>(&instance->connected_cb));
            }
            }
        }


    }
}

void Wifi_pro::pro_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (WIFI_PROV_EVENT == event_base)
    {
        
        switch(event_id)
        {
            case WIFI_PROV_START:
                ESP_LOGI("WIFI PRO", "Provisioning started");
                break;
            case WIFI_PROV_CRED_RECV: {
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                ESP_LOGI("WIFI PRO", "Received Wi-Fi credentials"
                         "\n\tSSID     : %s\n\tPassword : %s",
                         (const char *) wifi_sta_cfg->ssid,
                         (const char *) wifi_sta_cfg->password);
                break;
            }
            case WIFI_PROV_CRED_FAIL: {
                wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
                ESP_LOGE("WIFI PRO", "Provisioning failed!\n\tReason : %s"
                         "\n\tPlease reset to factory and retry provisioning",
                         (*reason == WIFI_PROV_STA_AUTH_ERROR) ?
                         "Wi-Fi station authentication failed" : "Wi-Fi ble not found");

                wifipro_retries++;
                //5 lan provision lai
                if (wifipro_retries >= 5) {
                    ESP_LOGI("WIFI PRO", "Failed to connect with provisioned AP, reseting provisioned credentials");
                    wifi_prov_mgr_reset_sm_state_on_failure();
                    wifipro_retries = 0;
                }

                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI("WIFI PRO", "Provisioning successful");
                wifipro_retries = 0;
                break;
            case WIFI_PROV_END:
                ESP_LOGI("WIFI PRO", "SUCCESS -> DEINIT ");
                wifi_prov_mgr_deinit();
                break;
            default:
                ESP_LOGW("WIFI PRO", "%s:%d Default switch case (%" PRId32 ")", __func__, __LINE__, event_id);
                break;
        }
    }
}

} // namespace WIFI