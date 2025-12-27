#include "web_client.h"

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "nvs_flash.h"

static const char *TAG = "web_client";
static nvs_handle_t host_info_handle;

static char host_url[32] = {0};
static uint16_t host_port = 0;
static char device_name[32] = {0};

esp_err_t web_client_init()
{
    size_t host_url_len = sizeof(host_url);

    esp_err_t err = nvs_open("host_info", NVS_READONLY, &host_info_handle);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        /* No saved credentials namespace */
        return ESP_ERR_NVS_NOT_FOUND;
    }

    ESP_ERROR_CHECK(err);

    err = nvs_get_str(host_info_handle, "url", host_url, &host_url_len);
    if (err == ESP_OK)
    {
        err = nvs_get_u16(host_info_handle, "port", &host_port);
    }

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        /* No saved credentials */
        return ESP_ERR_NVS_NOT_FOUND;
    }

    host_url_len = sizeof(device_name);
    err = nvs_get_str(host_info_handle, "device_name", device_name, &host_url_len);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        /* No saved credentials */
        return ESP_ERR_NVS_NOT_FOUND;
    }

    nvs_close(host_info_handle);

    ESP_ERROR_CHECK(err);

    return err;
}

esp_err_t web_client_post(const char *path, const char* payload_raw_str, const char* content_type_str)
{
    if (host_url[0] == '\0') {
        ESP_LOGE(TAG, "Invalid server url");
        return ESP_ERR_INVALID_STATE;
    }

    esp_http_client_config_t config = {
        .host = host_url,
        .port = host_port,
        .path = path,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize http client");
        return ESP_FAIL;
    }

    esp_err_t err;
    err = esp_http_client_set_header(client, "Content-Type", content_type_str);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set header: %d", err);
    }

    err = esp_http_client_set_post_field(client, payload_raw_str, strlen(payload_raw_str));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set post field: %d", err);
        esp_http_client_cleanup(client);
        return err;
    }

    err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        int content_len = esp_http_client_get_content_length(client);
        ESP_LOGI(TAG, "POST ok, status: %d, content_length: %d", status, content_len);
    } else {
        ESP_LOGE(TAG, "HTTP POST failed: %d", err);
    }

    esp_http_client_cleanup(client);

    return err;
}

void web_client_set_host_info(const char* url_str, uint16_t port, const char* device_name_str)
{
    if (url_str == NULL) {
        ESP_LOGE(TAG, "Invalid URL string");
        return;
    }

    if (device_name_str == NULL) {
        ESP_LOGE(TAG, "Invalid device name string");
        return;
    }

    strncpy(device_name, device_name_str, sizeof(device_name) - 1);
    device_name[sizeof(device_name) - 1] = '\0';

    strncpy(host_url, url_str, sizeof(host_url) - 1);
    host_url[sizeof(host_url) - 1] = '\0';
    host_port = port;

    ESP_ERROR_CHECK(nvs_open("host_info", NVS_READWRITE, &host_info_handle));

    ESP_ERROR_CHECK(nvs_set_str(host_info_handle, "url", url_str));
    ESP_ERROR_CHECK(nvs_set_u16(host_info_handle, "port", port));
    ESP_ERROR_CHECK(nvs_set_str(host_info_handle, "device_name", device_name_str));

    ESP_ERROR_CHECK(nvs_commit(host_info_handle));

    nvs_close(host_info_handle);
}

void web_client_reset_host_info()
{
    ESP_ERROR_CHECK(nvs_open("host_info", NVS_READWRITE, &host_info_handle));

    ESP_ERROR_CHECK(nvs_erase_key(host_info_handle, "url"));
    ESP_ERROR_CHECK(nvs_erase_key(host_info_handle, "port"));
    ESP_ERROR_CHECK(nvs_commit(host_info_handle));

    nvs_close(host_info_handle);

    host_url[0] = '\0';
    host_port = 0;
}

const char* web_client_get_host_url()
{
    return host_url;
}

uint16_t web_client_get_host_port()
{
    return host_port;
}

const char* web_client_get_device_name()
{
    return device_name;
}
