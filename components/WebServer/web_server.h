#ifndef __WEB_SERVER_H__
#define __WEB_SERVER_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "cJSON.h"
#include "esp_http_server.h"

#define WEBSERVER_HOSTNAME "termometr"

#define WEBSERVER_API_HANDLERS_MAX_COUNT 8
#define WEBSERVER_API_MAX_PAYLOAD_SIZE 256

typedef bool(*web_server_api_handler_t)(const cJSON * payload, cJSON * response);

typedef struct
{
    const char*              uri;
    httpd_method_t           type;
    web_server_api_handler_t handler_callback;
} web_server_api_t;

void web_server_start(void);
void web_server_stop(void);

size_t web_server_register_api_handler(const web_server_api_t* handler);
void web_server_unregister_api_handler(size_t handler_id);

#endif // __WEB_SERVER_H__
