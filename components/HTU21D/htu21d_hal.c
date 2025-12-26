#include "htu21d_hal.h"

static const int TRANSFER_TIMEOUT_MS = 10000;

bool HTU21D_HAL_I2C_Init(htu21d_i2c_t* i2c_handler)
{
    const i2c_device_config_t config =
    {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x40,
        .scl_speed_hz = 100000,
    };

    return (i2c_master_bus_add_device(*i2c_handler->bus, &config, &i2c_handler->device) == ESP_OK);
}

bool HTU21D_HAL_I2C_Write(htu21d_i2c_t* i2c_handler, const uint8_t *write_buffer, size_t write_size)
{
    return (i2c_master_transmit(i2c_handler->device, write_buffer, write_size, TRANSFER_TIMEOUT_MS) == ESP_OK);
}

bool HTU21D_HAL_I2C_Read(htu21d_i2c_t* i2c_handler, uint8_t address, uint8_t *read_buffer, size_t read_size)
{
    return (i2c_master_transmit_receive(i2c_handler->device, &address, sizeof(address), read_buffer, read_size, TRANSFER_TIMEOUT_MS) == ESP_OK);
}
