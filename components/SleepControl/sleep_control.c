#include "sleep_control.h"

#include "esp_sleep.h"

#define SLEEP_TIME_US (15 * 60 * 1000 * 1000) // 15 minutes


sleep_contol_wake_up_cause_t sleep_contol_init(void)
{
    esp_sleep_wakeup_cause_t wake_up_cause = esp_sleep_get_wakeup_cause();
    esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);

    return (wake_up_cause == ESP_SLEEP_WAKEUP_TIMER) ? SLEEP_CONTROL_WAKE_UP_CAUSE_TIMER : SLEEP_CONTROL_WAKE_UP_CAUSE_UNDEFINED;
}

void sleep_contol_sleep(void)
{
    esp_deep_sleep_start();
}
