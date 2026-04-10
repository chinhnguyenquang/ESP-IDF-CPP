#pragma once

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include <algorithm>
#include <mutex>
#include "esp_mac.h"
#include <cstring>


#include <wifi_provisioning/manager.h>

#include <wifi_provisioning/scheme_ble.h>

#include "qrcode.h"

#include <inttypes.h> //ESP_LOGW


namespace WIFI
{

class Wifi_pro
{
    constexpr static const char *QRCODE_BASE_URL{"https://espressif.github.io/esp-jumpstart/qrcode.html"};
    static char service_name[12];
    static char qr_payload[100];
    static uint8_t wifipro_retries;
    constexpr static const char *pop{"abcd1234"};


public:

    enum class state_e
    {
        NOT_INITIALISED,
        INITIALISED,
        CONNECTING,
        WAITING_FOR_IP,
        CONNECTED,
        DISCONNECTED,
        ERROR
    }; ///< WiFi states





    // "Rule of 5" Constructors and assignment operators
    // Ref: https://en.cppreference.com/w/cpp/language/rule_of_three
    Wifi_pro(void);
    ~Wifi_pro(void)                     = default;
    Wifi_pro(const Wifi_pro&)               = default;
    Wifi_pro(Wifi_pro&&)                    = default;
    Wifi_pro& operator=(const Wifi_pro&)    = default;
    Wifi_pro& operator=(Wifi_pro&&)         = default;

    

    esp_err_t init(void){return _init();};

    static const char* get_mac(void) 
        { return mac_addr_cstr; }


    

    constexpr static const state_e& get_state(void) { return _state; } ///< Current WiFi state


private:
    esp_err_t _init(void);
    // Get the MAC from the API and convert to ASCII HEX
    static esp_err_t _get_mac(void);

    static char mac_addr_cstr[13];  ///< Buffer to hold MAC as cstring
    static std::mutex init_mutx;    ///< Initialisation mutex
    static std::mutex state_mutx;   ///<State_e mutex
   

    static wifi_init_config_t           wifi_init_config;   ///< WiFi init config
    static wifi_config_t                wifi_config;        ///< WiFi config containing SSID & passw
    static wifi_prov_mgr_config_t       wifi_ble_config;
    static state_e _state;


    //event 
    static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data);
    static void ip_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data);
    static void pro_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data); ///< provision ble config

public:
    // NGUOI DUNG TU CUSTOME ENDPOINT CO THE HOAC KHONG (data,len)
    //custom endpoint cua config wifi_prov_mgr_config_t, khi co data gui den endpoint nay thi se goi callback nay, nguoi dung co the dat callback nay de xu ly data gui den endpoint nay
    using EndpointCallback = std::function<void(const uint8_t*, size_t)>;
    using ConnectedCallback = std::function<void(void)>;
    void setEndpointCallback(EndpointCallback cb)
    {
        endpoint_cb = cb;
    }
    void setConnectedCallback(ConnectedCallback cb)
    {
        connected_cb = cb;
    }


private:
    EndpointCallback endpoint_cb;
    ConnectedCallback connected_cb;
    static esp_err_t endpointHandler(uint32_t session_id,const uint8_t *inbuf,
                                    ssize_t inlen,uint8_t **outbuf,
                                    ssize_t *outlen,void *priv_data);

};

} // namespace WIFI