
#include "esp_log.h"
#include "esp_wifi.h"
#include "wifi_soft_ap.h"

static const char *TAG = "wifi_soft_ap";

void wifi_start_softap(void)
{    
    esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SOFT_AP_SSID,
            .ssid_len = 0,
            .channel = 1,
#ifdef WIFI_SOFT_AP_PASSWORD
            .password = WIFI_SOFT_AP_PASSWORD,
#endif
            .max_connection = 4,
#ifdef WIFI_SOFT_AP_PASSWORD
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
#else
            .authmode = WIFI_AUTH_OPEN,
#endif
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "SoftAP started. SSID:%s password:%s", wifi_config.ap.ssid, wifi_config.ap.password);
}
