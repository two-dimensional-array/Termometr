#ifndef __WIFI_STA_H__
#define __WIFI_STA_H__

#include "esp_err.h"

esp_err_t wifi_connect_sta(const char *ssid, const char *password, uint32_t timeout_ms);

#endif // __WIFI_STA_H__
