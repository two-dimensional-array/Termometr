idf_component_register(SRCS "./lib/htu21d.c" "./htu21d_hal.c"
                       INCLUDE_DIRS "." "./lib/"
                       REQUIRES esp_driver_i2c
)