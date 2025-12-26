idf_component_register(
    SRCS "web_server.c" "web_pages.c"
    INCLUDE_DIRS "."
    REQUIRES esp_http_server mdns json
    EMBED_TXTFILES "www/index.html" "www/main.js"
) 
