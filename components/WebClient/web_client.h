#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t web_client_init();

esp_err_t web_client_post(const char *path, const char* payload_raw_str, const char* content_type_str);

void web_client_set_host_info(const char* url_str, uint16_t port, const char* device_name_str);
void web_client_reset_host_info();

const char* web_client_get_host_url();
uint16_t web_client_get_host_port();
const char* web_client_get_device_name();

#ifdef __cplusplus
}
#endif
