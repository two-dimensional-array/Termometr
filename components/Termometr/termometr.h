#ifndef __TERMOMETR_H__
#define __TERMOMETR_H__

#include <stdint.h>
#include <stdbool.h>

#define TERMOMETR_MEASUREMENT_INTERVAL_MS 3000

void termometr_init(void);
void termometr_deinit(void);

void termometr_start(void);
void termometr_stop(void);

float termometr_get_temperature(void);
float termometr_get_humidity(void);

#endif // __TERMOMETR_H__
