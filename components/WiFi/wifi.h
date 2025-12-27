#ifndef __WIFI_H__
#define __WIFI_H__

typedef enum
{
    WIFI_STATE_NOT_INITIALIZED = 0,
    WIFI_STATE_SOFT_AP,
    WIFI_STATE_STA,
} wifi_state_t;

wifi_state_t wifi_init();
void wifi_stop();

void wifi_save_sta_credentials(const char *ssid, const char *password);
void wifi_reset_sta_credentials();
const char* wifi_get_sta_ssid();
const char* wifi_get_sta_password();

#endif // __WIFI_H__
