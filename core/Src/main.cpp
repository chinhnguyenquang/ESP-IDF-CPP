#include "main.h"
#define LOG_LEVEL_LOCAL ESP_LOG_VERBOSE
#include "esp_log.h"
#define LOG_TAG "MAIN"

#include "cJSON.h"

static Main _main;


static esp_err_t initialize_system(void)
{

    esp_err_t status{_main.Wifi.init()};

    status |= _main.led.init();
    
    return status;

} 


void Event_Connected_Handler(void)
{
    ESP_LOGI(LOG_TAG, "Connected to WiFi. Performing post-connection tasks...");
    if (!_main.FirstBoot)
    {
        _main.sntp.init();
    }

    xTaskCreate(&ota_update_task, "ota_update_task", 4096, NULL, 5, NULL);
}


void Task_Get_HTTP_IotVision(void *pvParameter)
{
    while (true)
    {
        if (WIFI::Wifi_pro::state_e::CONNECTED == WIFI::Wifi_pro::get_state())
        {
            ESP_LOGI(LOG_TAG, "Performing HTTP GET request to IoTVision API...");
            _main.http_client.SetQuery("CheDo=1&key=ABCDEF123456");
            _main.http_client.Get(_main.responseBuffer);

            if (_main.responseBuffer.size() > 0) {
            
                _main.responseBuffer.push_back('\0');
                
                // In ra log với thẻ "HTTP"
                ESP_LOGI("HTTP", "Response: %s", _main.responseBuffer.data());
            }

            ESP_LOGI(LOG_TAG, "Performing HTTP POST request to IoTVision API...");
            
            cJSON *json = cJSON_CreateObject();
            cJSON_AddStringToObject(json, "ID", "ABCDEF123456");
            cJSON_AddStringToObject(json, "S", "1111177;23.5;82.9;12:26:13 12/04/2024");
            char *message_payload = cJSON_Print(json);

            //const char * json ="{\"ID\":\"ABCDEF123456\",\"S\":\"1111177;23.5;82.9;12:26:13 12/04/2024\"}";
            _main.http_client.Post(message_payload,_main.postResponseBuffer);
 
            if (_main.postResponseBuffer.size() > 0) {
            
                _main.postResponseBuffer.push_back('\0');
                
                // In ra log với thẻ "HTTP"
                ESP_LOGI("HTTP", "POST Response: %s", _main.postResponseBuffer.data());
             }
            cJSON_Delete(json);
            free(message_payload);

        }

        vTaskDelay(pdMS_TO_TICKS(5000)); 
    }
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



    _main.Wifi.setConnectedCallback(Event_Connected_Handler);
    xTaskCreatePinnedToCore(&Task_Get_HTTP_IotVision, "Task cap nhat thong tin tu IoTVision", 4096, NULL, 5, NULL,1);
    
    while(1){
       

        ESP_LOGI(LOG_TAG, "OTA1");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}




