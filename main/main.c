#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "esp_mac.h"
#include "cJSON.h"

#include "web_server.h"
#include "wifi.h"
#include "termometr.h"
#include "web_client.h"

static bool get_termometr_request(const cJSON * payload, cJSON * response);
static bool set_wifi_credentials_request(const cJSON * payload, cJSON * response);
static bool get_wifi_credentials_request(const cJSON * payload, cJSON * response);
static bool reset_wifi_request(const cJSON * payload, cJSON * response);
static bool set_host_info_request(const cJSON * payload, cJSON * response);
static bool get_host_info_request(const cJSON * payload, cJSON * response);
static bool reset_host_info_request(const cJSON * payload, cJSON * response);

web_server_api_t termometr_api_handler = {
    .uri = "/api/readings",
    .type = HTTP_GET,
    .handler_callback = get_termometr_request
};

web_server_api_t wifi_connect_api_handler = {
    .uri = "/api/connect_wifi",
    .type = HTTP_POST,
    .handler_callback = set_wifi_credentials_request
};

web_server_api_t wifi_credentials_api_handler = {
    .uri = "/api/wifi_credentials",
    .type = HTTP_GET,
    .handler_callback = get_wifi_credentials_request
};

web_server_api_t wifi_reset_api_handler = {
    .uri = "/api/reset_wifi",
    .type = HTTP_POST,
    .handler_callback = reset_wifi_request
};

web_server_api_t host_connect_api_handler = {
    .uri = "/api/connect_server",
    .type = HTTP_POST,
    .handler_callback = set_host_info_request
};

web_server_api_t host_info_api_handler = {
    .uri = "/api/server_info",
    .type = HTTP_GET,
    .handler_callback = get_host_info_request
};

web_server_api_t host_reset_api_handler = {
    .uri = "/api/reset_server",
    .type = HTTP_POST,
    .handler_callback = reset_host_info_request
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

    if ((wifi_init() == WIFI_STATE_STA) && (web_client_init() == ESP_OK))
    {
        uint64_t id = 0ull;

        esp_efuse_mac_get_default((uint8_t*)&id);
        termometr_single_measurement();
        
        // Build JSON payload
        cJSON *root = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "id", id);
        cJSON_AddStringToObject(root, "name", web_client_get_device_name());
        cJSON_AddNumberToObject(root, "temperature", termometr_get_temperature());
        cJSON_AddNumberToObject(root, "humidity", termometr_get_humidity());

        char *json_str = cJSON_PrintUnformatted(root);

        esp_err_t web_client_result = web_client_post("/termometer", json_str, "application/json");

        if (json_str)
        {
            cJSON_free(json_str);
        }

        if (root)
        {
            cJSON_Delete(root);
        }

        if (web_client_result == ESP_OK)
        {
            wifi_stop();
            esp_sleep_enable_timer_wakeup(30 * 1000 * 1000); // 30 seconds
            esp_deep_sleep_start();
            esp_restart();
            return;
        }
    }
    
    termometr_start();

    web_server_register_api_handler(&termometr_api_handler);
    web_server_register_api_handler(&wifi_connect_api_handler);
    web_server_register_api_handler(&wifi_credentials_api_handler);
    web_server_register_api_handler(&wifi_reset_api_handler);
    web_server_register_api_handler(&host_connect_api_handler);
    web_server_register_api_handler(&host_info_api_handler);
    web_server_register_api_handler(&host_reset_api_handler);
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

static bool get_wifi_credentials_request(const cJSON * payload, cJSON * response)
{
    const char* ssid = wifi_get_sta_ssid();
    const char* password = wifi_get_sta_password();

    cJSON_AddStringToObject(response, "ssid", ssid ? ssid : "");
    cJSON_AddStringToObject(response, "password", password ? password : "");
    return true;
}

static bool reset_wifi_request(const cJSON * payload, cJSON * response)
{
    wifi_reset_sta_credentials();
    esp_restart();  // Restart to apply new WiFi settings
    return true;
}

static bool set_host_info_request(const cJSON * payload, cJSON * response)
{
    const cJSON *url_json = cJSON_GetObjectItem(payload, "url");
    const cJSON *port_json = cJSON_GetObjectItem(payload, "port");
    const cJSON *device_name_json = cJSON_GetObjectItem(payload, "device_name");

    if (!cJSON_IsString(url_json) || (url_json->valuestring == NULL))
    {
        return false;
    }

    const char *url = url_json->valuestring;
    uint16_t port = (cJSON_IsNumber(port_json) && (port_json->valueint >= 0)) ? port_json->valueint : 0;
    const char *device_name = (cJSON_IsString(device_name_json) && (device_name_json->valuestring != NULL)) ? device_name_json->valuestring : "";

    web_client_set_host_info(url, port, device_name);
    esp_restart();  // Restart to apply new host settings
    return true;
}

static bool get_host_info_request(const cJSON * payload, cJSON * response)
{
    const char* url = web_client_get_host_url();
    uint16_t port = web_client_get_host_port();
    const char* device_name = web_client_get_device_name();

    cJSON_AddStringToObject(response, "url", url ? url : "");
    cJSON_AddNumberToObject(response, "port", port);
    cJSON_AddStringToObject(response, "device_name", device_name ? device_name : "");

    return true;
}

static bool reset_host_info_request(const cJSON * payload, cJSON * response)
{
    web_client_reset_host_info();
    esp_restart();  // Restart to apply new WiFi settings
    return true;
}
