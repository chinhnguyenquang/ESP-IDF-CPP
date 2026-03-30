#include "main.h"
#define LOG_LEVEL_LOCAL ESP_LOG_VERBOSE
#include "esp_log.h"
#define LOG_TAG "MAIN"

static Main _main;



esp_err_t initialize_system(void)
{

    esp_err_t status{_main.Wifi.init()};

    status |= _main.led.init();
    



    return status;

} 


extern "C" void app_main(void)
{

     ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_err_t ret =nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* NVS partition was truncated
         * and needs to be erased */
        ESP_ERROR_CHECK(nvs_flash_erase());

        /* Retry nvs_flash_init */
        ESP_ERROR_CHECK(nvs_flash_init());
        
    }
   


    if (ESP_OK != initialize_system())
    {
        ESP_LOGE(LOG_TAG, "System initialization failed");
        esp_restart();
    }
    bool status_led=true;
    _main.sntp.init();// chi khi co wifi, neu khong se bi treo o day doi wifi connect, nen de sau khi co wifi connect moi init sntp de tranh treo o day doi wifi connect
    ESP_LOGI(LOG_TAG, "System initialization complete");
    xTaskCreate(&ota_update_task, "ota_update_task", 4096, NULL, 5, NULL);

    while(1){
       
        if(_main.Wifi.get_state() == WIFI::Wifi_pro::state_e::CONNECTED){
            ESP_LOGI(LOG_TAG,"OKKKKKKKKKKKK");
            status_led = !status_led;
            _main.led.set(status_led);
            // ESP_LOGI("main", "Time is %s", _main.sntp.ascii_time_now());

        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}




