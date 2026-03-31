#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "Gpio.h"
#include "Wifi_pro.h"
#include "SntpTime.h"
#include "Nvs32.h"
#include "OTA.h"


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

};