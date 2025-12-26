#include "wifi.h"
#include "wifi_soft_ap.h"
#include "wifi_sta.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"

#define WIFI_CONECT_STA_TIMEOUT_MS 10000

static const char *TAG = "wifi";

static nvs_handle_t wifi_creds_handle;
static char sta_ssid[32] = {0};
static char sta_password[32] = {0};

static bool read_wifi_credentials(char *out_ssid, size_t ssid_size, char *out_password, size_t password_size);

void wifi_init()
{
    size_t ssid_len = sizeof(sta_ssid);
    size_t password_len = sizeof(sta_password);
    bool start_sta = false;

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    if (read_wifi_credentials(sta_ssid, ssid_len, sta_password, password_len))
    {
        ESP_LOGI(TAG, "Found saved WiFi credentials: ssid=%s, password=%s", sta_ssid, sta_password[0] ? sta_password : "<empty>");
        start_sta = wifi_connect_sta(sta_ssid, sta_password, WIFI_CONECT_STA_TIMEOUT_MS) == ESP_OK;
    }
    else
    {
        ESP_LOGI(TAG, "No saved WiFi credentials found");
    }

    if (!start_sta)
    {
        wifi_start_softap();
    }
}

void wifi_save_sta_credentials(const char *ssid, const char *password)
{
    if (!ssid || (strlen(ssid) == 0)) return;

    ESP_ERROR_CHECK(nvs_open("wifi_creds", NVS_READWRITE, &wifi_creds_handle));

    ESP_ERROR_CHECK(nvs_set_str(wifi_creds_handle, "ssid", ssid));
    strncpy(sta_ssid, ssid, sizeof(sta_ssid) - 1);

    if (password && (strlen(password) > 0))
    {
        ESP_ERROR_CHECK(nvs_set_str(wifi_creds_handle, "password", password));
        strncpy(sta_password, password, sizeof(sta_password) - 1);
    }
    else
    {
        ESP_ERROR_CHECK(nvs_erase_key(wifi_creds_handle, "password"));
        sta_password[0] = '\0';
    }
    ESP_ERROR_CHECK(nvs_commit(wifi_creds_handle));

    nvs_close(wifi_creds_handle);

    ESP_LOGI(TAG, "Saved WiFi credentials: ssid=%s, password=%s", ssid, password ? password : "<empty>");
}

void wifi_reset_sta_credentials()
{
    ESP_ERROR_CHECK(nvs_open("wifi_creds", NVS_READWRITE, &wifi_creds_handle));

    ESP_ERROR_CHECK(nvs_erase_key(wifi_creds_handle, "ssid"));
    ESP_ERROR_CHECK(nvs_erase_key(wifi_creds_handle, "password"));
    ESP_ERROR_CHECK(nvs_commit(wifi_creds_handle));

    nvs_close(wifi_creds_handle);

    sta_ssid[0] = '\0';
    sta_password[0] = '\0';

    ESP_LOGI(TAG, "Reset WiFi credentials");
}

const char* wifi_get_sta_ssid()
{
    return sta_ssid;
}

const char* wifi_get_sta_password()
{
    return sta_password;
}

static bool read_wifi_credentials(char *out_ssid, size_t ssid_size, char *out_password, size_t password_size)
{
    size_t ssid_len = ssid_size;
    size_t password_len = password_size;

    esp_err_t err = nvs_open("wifi_creds", NVS_READONLY, &wifi_creds_handle);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        /* No saved credentials namespace */
        return false;
    }

    ESP_ERROR_CHECK(err);

    err = nvs_get_str(wifi_creds_handle, "ssid", out_ssid, &ssid_len);
    if (err == ESP_OK)
    {
        err = nvs_get_str(wifi_creds_handle, "password", out_password, &password_len);
    }

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        /* No saved credentials */
        return false;
    }

    nvs_close(wifi_creds_handle);

    ESP_ERROR_CHECK(err);

    return (err == ESP_OK);
}
