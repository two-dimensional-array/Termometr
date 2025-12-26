#include "termometr.h"

#include "htu21d.h"

#include "driver/i2c_master.h"

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#define HTU21D_I2C_SDA_GPIO GPIO_NUM_8
#define HTU21D_I2C_SCL_GPIO GPIO_NUM_9

static i2c_master_bus_handle_t bus_handle;
static htu21d_t sensor;

static StaticTimer_t measurement_timer;
static TimerHandle_t measurement_timer_handle = NULL;

static float temperature = 0.0f;
static float humidity = 0.0f;

static void measurement_timer_callback(TimerHandle_t xTimer);

void termometr_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = HTU21D_I2C_SDA_GPIO,
        .scl_io_num = HTU21D_I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    sensor.i2c_handler.bus = &bus_handle;
    sensor.measure_resolution = HTU21D_MEASURE_RESOLUTION_RH11_TEMP11;
    HTU21D_Init(&sensor);

    measurement_timer_handle = xTimerCreateStatic(
        "TermometrMeasurementTimer",
        pdMS_TO_TICKS(TERMOMETR_MEASUREMENT_INTERVAL_MS),
        pdTRUE,
        NULL,
        measurement_timer_callback,
        &measurement_timer
    );
}

void termometr_deinit(void)
{
    if (measurement_timer_handle)
    {
        xTimerDelete(measurement_timer_handle, portMAX_DELAY);
        measurement_timer_handle = NULL;
    }
}

void termometr_start(void)
{
    if (measurement_timer_handle)
    {
        xTimerStart(measurement_timer_handle, portMAX_DELAY);
    }
}

void termometr_stop(void)
{
    if (measurement_timer_handle)
    {
        xTimerStop(measurement_timer_handle, portMAX_DELAY);
    }
}

float termometr_get_temperature(void)
{
    return temperature;
}

float termometr_get_humidity(void)
{
    return humidity;
}

static void measurement_timer_callback(TimerHandle_t xTimer)
{
    temperature = HTU21D_GetTemperature(&sensor);
    humidity = HTU21D_GetRelativeHumidity(&sensor);
}
