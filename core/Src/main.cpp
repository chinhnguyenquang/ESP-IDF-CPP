#include "main.h"
#define LOG_LEVEL_LOCAL ESP_LOG_VERBOSE
#include "esp_log.h"
#define LOG_TAG "MAIN"

static Main _main;

extern "C" void app_main(void)
{
    ESP_LOGI(LOG_TAG, "Creating default event loop");
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(LOG_TAG, "Initialising NVS");
    ESP_ERROR_CHECK(nvs_flash_init());


    ESP_LOGI(LOG_TAG,"BAT DAU NEEEEEEEE");
    esp_err_t status{ESP_OK};
    status |= _main.led.init();
    status |= _main.button.init();

    ESP_ERROR_CHECK(status);
    
        
    while(1){
    //ESP_LOGI(LOG_TAG, "ESP CLASS");
    ESP_ERROR_CHECK(_main.led.set(true));
    ESP_LOGI("TRANG THAI DEN ","INPUT LA %d", _main.button.state());
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_ERROR_CHECK(_main.led.set(false));
    vTaskDelay(pdMS_TO_TICKS(1000));
    }
}




