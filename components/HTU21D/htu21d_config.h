#ifndef __HTU21D_CONFIG_H__
#define __HTU21D_CONFIG_H__

#include "driver/i2c_master.h"

typedef struct
{
    i2c_master_bus_handle_t *bus;
    i2c_master_dev_handle_t device;
} htu21d_i2c_t;

#endif // __HTU21D_CONFIG_H__
