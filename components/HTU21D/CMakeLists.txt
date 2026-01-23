idf_component_register(SRCS "htu21d_hal.c" "./lib/htu21d.c" "./htu21d_hal.c"
                       INCLUDE_DIRS "." "./lib/"
                       REQUIRES esp_driver_i2c
)