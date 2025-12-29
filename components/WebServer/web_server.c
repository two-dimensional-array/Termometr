#include "esp_log.h"
#include "esp_err.h"
#include "mdns.h"

#include "web_server.h"
#include "web_pages.h"

static const char *TAG = "web_server";
static httpd_handle_t server = NULL;
static web_server_api_t api_handlers[WEBSERVER_API_HANDLERS_MAX_COUNT] = {0};

static esp_err_t web_page_general_handler(httpd_req_t *req);
static esp_err_t api_general_handler(httpd_req_t *req);

static void start_mdns(void)
{
    if (mdns_init() == ESP_OK)
    {
        ESP_ERROR_CHECK(mdns_hostname_set(WEBSERVER_HOSTNAME));
        ESP_ERROR_CHECK(mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0));
        ESP_LOGI(TAG, "mDNS initialized: http://" WEBSERVER_HOSTNAME ".local/");
    }
    else
    {
        ESP_LOGW(TAG, "mDNS init failed, hostname access may not work on all clients");
    }
}

static void stop_mdns(void)
{
    mdns_free();
}

void web_server_start(void)
{
    httpd_config_t config = {
        .task_priority      = tskIDLE_PRIORITY+5,
        .stack_size         = 4096,
        .core_id            = tskNO_AFFINITY,
        .task_caps          = (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
        .max_req_hdr_len    = CONFIG_HTTPD_MAX_REQ_HDR_LEN,
        .max_uri_len        = CONFIG_HTTPD_MAX_URI_LEN,
        .server_port        = 80,
        .ctrl_port          = ESP_HTTPD_DEF_CTRL_PORT,
        .max_open_sockets   = 7,
        .max_uri_handlers   = 12,
        .max_resp_headers   = 12,
        .backlog_conn       = 5,
        .lru_purge_enable   = false,
        .recv_wait_timeout  = 5,
        .send_wait_timeout  = 5,
        .global_user_ctx = NULL,
        .global_user_ctx_free_fn = NULL,
        .global_transport_ctx = NULL,
        .global_transport_ctx_free_fn = NULL,
        .enable_so_linger = false,
        .linger_timeout = 0,
        .keep_alive_enable = false,
        .keep_alive_idle = 0,
        .keep_alive_interval = 0,
        .keep_alive_count = 0,
        .open_fn = NULL,
        .close_fn = NULL,
        .uri_match_fn = NULL
    };

    start_mdns();

    if (httpd_start(&server, &config) == ESP_OK)
    {
        for (size_t i = 0; i < WEB_PAGES_COUNT; i++)
        {
            httpd_uri_t page_uri = {
                .uri = WEB_PAGES[i].uri,
                .method = HTTP_GET,
                .handler = web_page_general_handler,
                .user_ctx = (void*)&WEB_PAGES[i]
            };
            httpd_register_uri_handler(server, &page_uri);
        }

        for (size_t i = 0; i < WEBSERVER_API_HANDLERS_MAX_COUNT; i++)
        {
            if (api_handlers[i].uri != NULL)
            {
                httpd_uri_t custom_uri = {
                    .uri = api_handlers[i].uri,
                    .method = api_handlers[i].type,
                    .handler = api_general_handler,
                    .user_ctx = &api_handlers[i]
                };
                httpd_register_uri_handler(server, &custom_uri);
            }
        }

        ESP_LOGI(TAG, "HTTP server started");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to start HTTP server");
    }
}

void web_server_stop(void)
{
    stop_mdns();

    if (server)
    {
        httpd_stop(server);
        server = NULL;
    }
}

size_t web_server_register_api_handler(const web_server_api_t* handler)
{
    for (size_t i = 0; i < WEBSERVER_API_HANDLERS_MAX_COUNT; i++)
    {
        if (api_handlers[i].uri == NULL)
        {
            api_handlers[i] = *handler;
            return i;
        }
    }
    return SIZE_MAX;
}

void web_server_unregister_api_handler(size_t handler_id)
{
    if (handler_id < WEBSERVER_API_HANDLERS_MAX_COUNT)
    {
        memset(&api_handlers[handler_id], 0, sizeof(web_server_api_t));
    }
}

static bool cjson_is_empty(const cJSON *item)
{
    if (item == NULL) return true;
    if (item->type == cJSON_NULL) return true;
    if (cJSON_IsObject(item) || cJSON_IsArray(item)) return (item->child == NULL);
    if (cJSON_IsString(item)) return (item->valuestring == NULL || item->valuestring[0] == '\0');
    /* numbers and booleans are considered non-empty */
    return false;
}

static esp_err_t web_page_general_handler(httpd_req_t *req)
{
    web_page_t* info = (web_page_t*)req->user_ctx;
    size_t content_length = info->content_end - info->content_start - 1;
    httpd_resp_set_type(req, info->content_type);
    httpd_resp_send(req, info->content_start, content_length);
    return ESP_OK;
}

static esp_err_t api_general_handler(httpd_req_t *req)
{
    web_server_api_t* api = (web_server_api_t*)req->user_ctx;

    static char payload_str_buf[WEBSERVER_API_MAX_PAYLOAD_SIZE];
    const size_t payload_len = httpd_req_recv(req, payload_str_buf, sizeof(payload_str_buf) - 1);

    payload_str_buf[payload_len] = '\0';

    char *response_str = NULL;
    cJSON *payload_json = cJSON_Parse(payload_str_buf);
    cJSON *response_json = cJSON_CreateObject();

    if (api->handler_callback(payload_json, response_json))
    {
        if (cjson_is_empty(response_json))
        {
            httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
        }
        else
        {
            response_str = cJSON_PrintUnformatted(response_json);
            httpd_resp_set_type(req, "application/json");
            httpd_resp_send(req, response_str, HTTPD_RESP_USE_STRLEN);
        }
    }
    else
    {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "Invalid request", HTTPD_RESP_USE_STRLEN);
    }

    if (response_str)
    {
        cJSON_free(response_str);
    }

    if (payload_json)
    {
        cJSON_Delete(payload_json);
    }

    if (response_json)
    {
        cJSON_Delete(response_json);
    }

    return ESP_OK;
}
