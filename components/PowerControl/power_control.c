#include "power_control.h"

#include "driver/gpio.h"

#define POWER_CONTROL_PIN GPIO_NUM_5

esp_err_t power_control_init(void)
{
    const gpio_config_t config = {
        .pin_bit_mask = (1ULL << POWER_CONTROL_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    return gpio_config(&config);
}

esp_err_t power_control_set_state(bool state)
{
    return gpio_set_level(POWER_CONTROL_PIN, state);
}
