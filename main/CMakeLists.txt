idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES "index.html" "style.css"
                    PRIV_REQUIRES freertos esp_wifi esp_event log nvs_flash esp_http_server driver esp_netif esp_timer)