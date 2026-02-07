idf_component_register(SRCS "main.c" "motor_control.c" "battery_monitor.c" "temperature_sensor.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES "index.html" "style.css"
                    PRIV_REQUIRES freertos esp_wifi esp_event log nvs_flash esp_http_server driver esp_netif esp_timer esp_adc onewire_bus ds18b20 esp_system)