#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "secrets.h"

#include "battery_monitor.h"
#include "motor_control.h"
#include "temperature_sensor.h"

static const char* TAG = "SERVO_WEB_CONTROL";

// 웹 페이지 HTML
extern const char _binary_index_html_start[];
extern const char _binary_index_html_end[];
extern const char _binary_style_css_start[];
extern const char _binary_style_css_end[];

// 토글 핸들러
esp_err_t toggle_handler(httpd_req_t* req) {
  bool is_on = get_is_on();
  is_on = !is_on;
  set_is_on(is_on);
  ESP_LOGI(TAG, "Servo %s", is_on ? "ON" : "OFF");

  if (is_on) {
    set_servo_angle_on();
  } else {
    set_servo_angle_off();
  }
  update_servo_activity();

  httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

// 상태 핸들러
esp_err_t status_handler(httpd_req_t* req) {
  char resp[64];
  sprintf(resp, "{\"status\": \"%s\"}", get_is_on() ? "ON" : "OFF");
  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, resp, strlen(resp));
  return ESP_OK;
}

// CSS 핸들러
esp_err_t style_css_handler(httpd_req_t* req) {
  const size_t css_size = (_binary_style_css_end - _binary_style_css_start);
  httpd_resp_set_type(req, "text/css");
  httpd_resp_send(req, _binary_style_css_start, css_size);
  return ESP_OK;
}

// 메인 페이지 핸들러
esp_err_t root_handler(httpd_req_t* req) {
  const size_t html_size = (_binary_index_html_end - _binary_index_html_start);
  httpd_resp_send(req, _binary_index_html_start, html_size);
  return ESP_OK;
}

// 배터리 핸들러
esp_err_t battery_handler(httpd_req_t* req) {
  battery_stats_t battery_stats = get_battery_stats();
  char resp[256];
  sprintf(resp,
          "{\"level\": %d, \"voltage\": %.2f, \"adc\": %ld, \"min_v\": %.2f, "
          "\"max_v\": %.2f}",
          battery_stats.percentage, battery_stats.voltage,
          battery_stats.adc_reading, battery_stats.min_v, battery_stats.max_v);
  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, resp, strlen(resp));
  return ESP_OK;
}

// 온도 핸들러
esp_err_t temperature_handler(httpd_req_t* req) {
  char resp[64];
  sprintf(resp, "{\"temperature\": %.2f}", get_temperature_celsius());
  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, resp, strlen(resp));
  return ESP_OK;
}

httpd_handle_t server = NULL;

// 웹 서버 시작
void start_webserver() {
  if (server) {
    return;
  }
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_uri_t root = {.uri = "/", .method = HTTP_GET, .handler = root_handler};
    httpd_register_uri_handler(server, &root);
    httpd_uri_t style_css = {
        .uri = "/style.css", .method = HTTP_GET, .handler = style_css_handler};
    httpd_register_uri_handler(server, &style_css);
    httpd_uri_t toggle = {
        .uri = "/toggle", .method = HTTP_GET, .handler = toggle_handler};
    httpd_register_uri_handler(server, &toggle);
    httpd_uri_t status = {
        .uri = "/status", .method = HTTP_GET, .handler = status_handler};
    httpd_register_uri_handler(server, &status);
    httpd_uri_t battery = {
        .uri = "/battery", .method = HTTP_GET, .handler = battery_handler};
    httpd_register_uri_handler(server, &battery);
    httpd_uri_t temperature = {.uri = "/temperature",
                               .method = HTTP_GET,
                               .handler = temperature_handler};
    httpd_register_uri_handler(server, &temperature);
  }
}

// 와이파이 연결 이벤트 핸들러
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
    if (server) {
      httpd_stop(server);
      server = NULL;
    }
    if (get_is_on()) {
      ESP_LOGI(TAG, "Servo was ON, setting to OFF due to Wi-Fi disconnection.");
      set_servo_angle_off();
    }
    esp_wifi_connect();
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
    ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    start_webserver();
  }
}

void app_main(void) {
  esp_log_level_set(TAG, ESP_LOG_INFO);
  ESP_LOGI(TAG, "app_main started");
  // NVS 초기화
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  ESP_LOGI(TAG, "NVS initialized.");

  // 모듈 초기화
  configure_servo();
  configure_adc();
  temperature_sensor_init();

  // 와이파이 초기화
  ESP_LOGI(TAG, "Initializing WiFi.");
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = WIFI_SSID,
              .password = WIFI_PASSWORD,
          },
  };
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI(TAG, "WiFi started. Connecting...");

  xTaskCreate(&servo_power_management_task, "servo_power_task", 2048, NULL, 5,
              NULL);
}