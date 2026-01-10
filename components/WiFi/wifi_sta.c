#include "wifi_sta.h"

#include <string.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef enum
{
    WIFI_CONNECTED_BIT = BIT0,
    WIFI_FAIL_BIT      = BIT1,
} wifi_event_bits_t;

#define WIFI_EVENTS_BIT_MASK (WIFI_CONNECTED_BIT | WIFI_FAIL_BIT)

static EventGroupHandle_t sta_event_group = NULL;
static esp_netif_t* sta_netif = NULL;

static const char *TAG = "wifi_sta";

static void wifi_sta_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

esp_err_t wifi_connect_sta(const char *ssid, const char *password, uint32_t timeout_ms)
{
    esp_event_handler_instance_t wifi_event_instance = NULL;
    esp_event_handler_instance_t ip_event_instance = NULL;

    if (!ssid || (strlen(ssid) == 0))
    {
        return ESP_ERR_INVALID_ARG;
    }

    sta_event_group = xEventGroupCreate();

    if (sta_event_group == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_sta_event_handler, NULL, &wifi_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL, &ip_event_instance));

    if (sta_netif == NULL)
    {
        sta_netif = esp_netif_create_default_wifi_sta();
    }

    wifi_config_t wifi_config = { 0 };
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid)-1);
    if (password)
    {
        strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    const BaseType_t wait_all_bits = pdFALSE;
    const BaseType_t clear_on_exit = pdTRUE;
    EventBits_t bits = xEventGroupWaitBits(sta_event_group, WIFI_EVENTS_BIT_MASK, clear_on_exit, wait_all_bits, pdMS_TO_TICKS(timeout_ms));

    if (wifi_event_instance)
    {
        esp_event_handler_instance_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, wifi_event_instance);
    }

    if (ip_event_instance)
    {
        esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_event_instance);
    }

    if (sta_event_group)
    {
        vEventGroupDelete(sta_event_group);
        sta_event_group = NULL;
    }

    return (bits & WIFI_CONNECTED_BIT) ? ESP_OK : ESP_FAIL;
}

static void wifi_sta_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_DISCONNECTED))
    {
        xEventGroupSetBits(sta_event_group, WIFI_FAIL_BIT);
    }
}

static void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if ((event_base == IP_EVENT) && (event_id == IP_EVENT_STA_GOT_IP))
    {
        ESP_LOGI(TAG, "Got IP address " IPSTR, IP2STR(&((ip_event_got_ip_t*)event_data)->ip_info.ip));
        xEventGroupSetBits(sta_event_group, WIFI_CONNECTED_BIT);
    }
}
