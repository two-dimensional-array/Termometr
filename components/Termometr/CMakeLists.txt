idf_component_register(SRCS "termometr.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_driver_i2c HTU21D
)
