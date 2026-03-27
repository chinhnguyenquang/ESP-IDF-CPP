#include "main.h"
#define LOG_LEVEL_LOCAL ESP_LOG_VERBOSE
#include "esp_log.h"
#define LOG_TAG "MAIN"

static Main _main;


char tmp[20]={0};
size_t len=10;
esp_err_t initialize_system(void)
{

    esp_err_t status{_main.Wifi.init()};

    status |= _main.led.init();
    
    status |= _main.nvs_cfg.init();

    constexpr static const char*    key{"ctr"};
    
    uint16_t  ctr{0};
    char ttt[20]="hellojnfg";
    status |= _main.nvs_cfg.get(key, ctr);
    //status |= _main.nvs_cfg.set_buffer("test", ttt, strlen(ttt));
    _main.nvs_cfg.get_buffer("test", tmp, len);
    ESP_LOGI("nvs_tag", "%s", esp_err_to_name(status));
    if (ESP_OK == status)
    {
        ESP_LOGI("nvs_tag", "Counter: %u", ctr);

        ++ctr;
        ESP_LOGI("nvs_tag", "Setting key \"%s\" to %u", key, ctr);
        status = _main.nvs_cfg.set(key, ctr);
        ESP_LOGI("nvs_tag", "%s", esp_err_to_name(status));
    }
    else{
        ESP_LOGI("nvs_tag", "faileddddddddd:");
        status = _main.nvs_cfg.set(key, ctr);
    }

    _main.nvs_cfg.close();





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
    //_main.sntp.init();



    while(1){
        //ESP_LOGI(LOG_TAG,"MA MAC CUA ESP %s",_main.Wifi.get_mac());
       
        if(_main.Wifi.get_state() == WIFI::Wifi_pro::state_e::CONNECTED){
            ESP_LOGI(LOG_TAG,"OKKKKKKKKKKKK");
            status_led = !status_led;
            _main.led.set(status_led);
            // ESP_LOGI("main", "Time is %s", _main.sntp.ascii_time_now());

        }
        ESP_LOGI(LOG_TAG,"NVSSS %s ......%zu", tmp, len);
        printf("hellow rod\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}




