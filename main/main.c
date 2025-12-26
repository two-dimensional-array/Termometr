#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_system.h"

#include "web_server.h"
#include "wifi.h"
#include "termometr.h"

static bool get_termometr_request(const cJSON * payload, cJSON * response);
static bool set_wifi_credentials_request(const cJSON * payload, cJSON * response);

web_server_api_t termometr_api_handler = {
    .uri = "/api/readings",
    .type = HTTP_GET,
    .handler_callback = get_termometr_request
};

web_server_api_t wifi_config_api_handler = {
    .uri = "/configure",
    .type = HTTP_POST,
    .handler_callback = set_wifi_credentials_request
};

void app_main(void)
{
    esp_err_t nvs_result = nvs_flash_init();
    if ((nvs_result == ESP_ERR_NVS_NO_FREE_PAGES) || (nvs_result == ESP_ERR_NVS_NEW_VERSION_FOUND))
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_result = nvs_flash_init();
    }

    ESP_ERROR_CHECK(nvs_result);

    termometr_init();
    termometr_start();

    web_server_register_api_handler(&termometr_api_handler);
    web_server_register_api_handler(&wifi_config_api_handler);

    wifi_init();
    web_server_start();
}

static bool get_termometr_request(const cJSON * payload, cJSON * response)
{
    cJSON_AddNumberToObject(response, "temperature", termometr_get_temperature());
    cJSON_AddNumberToObject(response, "relative_humidity", termometr_get_humidity());
    return true;
}

static bool set_wifi_credentials_request(const cJSON * payload, cJSON * response)
{
    const cJSON *ssid_json = cJSON_GetObjectItem(payload, "ssid");
    const cJSON *password_json = cJSON_GetObjectItem(payload, "password");

    if (!cJSON_IsString(ssid_json) || (ssid_json->valuestring == NULL)) {
        return false;
    }

    const char *ssid = ssid_json->valuestring;
    const char *password = (cJSON_IsString(password_json) && (password_json->valuestring != NULL)) ? password_json->valuestring : "";

    wifi_save_sta_credentials(ssid, password);
    esp_restart();  // Restart to apply new WiFi settings
    return true;
}
