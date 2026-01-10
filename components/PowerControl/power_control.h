#ifndef __POWER_CONTROL_H__
#define __POWER_CONTROL_H__

#include <esp_err.h>
#include <stdbool.h>

esp_err_t power_control_init(void);
esp_err_t power_control_set_state(bool state);

#endif // __POWER_CONTROL_H__
