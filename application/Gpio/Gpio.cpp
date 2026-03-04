#include "Gpio.h"

namespace Gpio
{
    [[nodiscard]] esp_err_t GpioBase::init(void){
        esp_err_t status{ESP_OK};

        status |= gpio_config(&_cfg);

        return status;
    }

    [[nodiscard]] esp_err_t GpioOutput::init(void){
    esp_err_t status{GpioBase::init()};

    status |= set(false);

    return status;
    }


    [[nodiscard]] esp_err_t GpioInput::init(void){return {GpioBase::init()};}


    esp_err_t GpioOutput::set(const bool state){
        _state=state;
        return gpio_set_level(_pin,state);
    }

} // namespace Gpio

/*
isr
    //change gpio interrupt type for one pin
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);

    //remove isr handler for gpio number.
    gpio_isr_handler_remove(GPIO_INPUT_IO_0);
    //hook isr handler for specific gpio pin again
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);

    static void IRAM_ATTR gpio_isr_handler(void* arg)
    {
        uint32_t gpio_num = (uint32_t) arg;
    }




*/