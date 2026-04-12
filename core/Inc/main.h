#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "Gpio.h"
#include "Wifi_pro.h"
#include "SntpTime.h"
#include "Nvs32.h"
#include <OTA.h>
#include "Mqtt.h"
#include "httpClient.h"
#include "Mqtt.h"

#include "Json.h"

class Main final{
    public: 
     Main(void)
        : sntp { SNTP::Sntp::get_instance() }
        {

        }
        Gpio::GpioOutput led{GPIO_NUM_2};
        Gpio::GpioInput button{GPIO_NUM_13,true,false,GPIO_INTR_DISABLE};
        WIFI::Wifi_pro Wifi;
        SNTP::Sntp& sntp;
        NVS::Nvs nvs_cfg;
        HTTP_CLIENT::HttpClient http_client{"App.IoTVision.vn", "/api/BoardSTR423_DuLieuGuiXuongBoard"};
        
        std::vector<char> responseBuffer; //bien de luu tru get response tu http client (HTTP GET)
        std::vector<char> postResponseBuffer; //bien de luu tru post response tu http client (HTTP POST)
        bool FirstBoot_to_Wifi{false};
        bool FirstBoot_to_Mqtt{false};
        ESP32MQTT::MQTTClient _Mqtt{"mqtts://11783fdcba0e4a889fe8980cc7d51777.s1.eu.hivemq.cloud:8883", "chinh", "Chinh12345"};
        //Data Json.h
        struct Data_S_IOTVision data_iotvision; //data post du lieu tu board len server, sau khi parse tu json se duoc luu tru trong struct nay
        
        
};