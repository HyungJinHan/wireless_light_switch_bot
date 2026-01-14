idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES freertos esp_wifi esp_event log nvs_flash esp_http_server driver esp_netif esp_timer)
target_add_binary_data(__idf_main "index.html" TEXT)