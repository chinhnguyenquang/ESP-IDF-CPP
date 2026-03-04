#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "Gpio.h"

class Main final{
    public: 

        Gpio::GpioOutput led{GPIO_NUM_2};
        Gpio::GpioInput button{GPIO_NUM_13,true,false,GPIO_INTR_DISABLE};
};