#pragma once

#include "driver/gpio.h"


namespace Gpio{
class GpioBase{
protected:
    const gpio_num_t _pin;
    const gpio_config_t _cfg;
public:
    constexpr GpioBase(const gpio_num_t pin,const gpio_config_t &config):
        _pin{pin},
        _cfg{config}
        {

        }

    virtual bool state(void)=0;
    virtual esp_err_t set(const bool state) =0;
    [[nodiscard]] esp_err_t init(void);
};


class GpioOutput : public GpioBase
{
    bool _state =false;

public:
    constexpr GpioOutput(const gpio_num_t pin):
        GpioBase(pin,
            gpio_config_t{
                .pin_bit_mask = static_cast<uint64_t>(1) << pin,
                .mode = GPIO_MODE_OUTPUT,
                .pull_up_en=GPIO_PULLUP_DISABLE,
                .pull_down_en=GPIO_PULLDOWN_ENABLE,
                .intr_type=GPIO_INTR_DISABLE
            })
    {

    }

    [[nodiscard]] esp_err_t init(void);

    esp_err_t set(const bool state);

    bool state(void){return _state;}
};

class GpioInput : public GpioBase
{
public:
    constexpr GpioInput(const gpio_num_t pin,const bool pullup,const bool pulldown,gpio_int_type_t intr):
    GpioBase(pin,
        gpio_config_t{
            .pin_bit_mask=static_cast<uint64_t>(1) << pin,
            .mode = GPIO_MODE_INPUT,
            .pull_up_en=pullup ? GPIO_PULLUP_ENABLE:GPIO_PULLUP_DISABLE,
            .pull_down_en=pulldown ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
            .intr_type=intr
        })
        {
          
        }

    [[nodiscard]] esp_err_t init(void);
    esp_err_t set (const bool state) {return ESP_OK;}
    bool state(void) {return gpio_get_level(_pin);}
};


};