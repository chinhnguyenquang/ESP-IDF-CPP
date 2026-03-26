#include "main.h"
#define LOG_LEVEL_LOCAL ESP_LOG_VERBOSE
#include "esp_log.h"
#define LOG_TAG "MAIN"

static Main _main;



esp_err_t initialize_system(void)
{

    esp_err_t status{_main.Wifi.init()};

    status |= _main.led.init();
    
    status |= _main.nvs_cfg.open();

    constexpr static const char*    key{"ctr"};
    
    size_t  ctr{0};

    size_t len{1};
    status |= _main.nvs_cfg.get_buf(key, &ctr, len);
    ESP_LOGI("nvs_tag", "%s", esp_err_to_name(status));
    if (ESP_OK == status)
    {
        ESP_LOGI("nvs_tag", "Counter: %u", ctr);

        ++ctr;
        ESP_LOGI("nvs_tag", "Setting key \"%s\" to %u", key, ctr);
        status = _main.nvs_cfg.set_buffer(key, &ctr);
        ESP_LOGI("nvs_tag", "%s", esp_err_to_name(status));
    }
    else{
        ESP_LOGI("nvs_tag", "faileddddddddd:");
        status = _main.nvs_cfg.set_buffer(key, &ctr);
    }

    _main.nvs_cfg.close();





    return status;

} 


extern "C" void app_main(void)
{

    
    esp_err_t ret =nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* NVS partition was truncated
         * and needs to be erased */
        ESP_ERROR_CHECK(nvs_flash_erase());

        /* Retry nvs_flash_init */
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(esp_event_loop_create_default());


    if (ESP_OK != initialize_system())
    {
        ESP_LOGE(LOG_TAG, "System initialization failed");
        //esp_restart();
    }
    bool status_led=true;
    _main.sntp.init();



    while(1){
        //ESP_LOGI(LOG_TAG,"MA MAC CUA ESP %s",_main.Wifi.get_mac());
       
        if(_main.Wifi.get_state() == WIFI::Wifi_pro::state_e::CONNECTED){
            ESP_LOGI(LOG_TAG,"OKKKKKKKKKKKK");
            status_led = !status_led;
            _main.led.set(status_led);
            // ESP_LOGI("main", "Time is %s", _main.sntp.ascii_time_now());

        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}




