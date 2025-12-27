idf_component_register(
    SRCS "web_client.c"
    INCLUDE_DIRS "."
    REQUIRES esp_http_client nvs_flash
) 
