#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "driver/ledc.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

static const char *TAG = "SERVO_WEB_CONTROL";

// 설정
#define SERVO_PIN 14

// SG90 서보 설정 (50Hz, 13-bit 해상도 기준)
// 0도: 약 164 (0.5ms), 90도: 약 410 (1.25ms), 180도: 약 655 (2.0ms)
#define SERVO_MIN_DUTY 205  // 0.5ms (0도)
#define SERVO_MAX_DUTY 819  // 2.0ms (180도)
#define SERVO_45_DEGREE_DUTY \
  SERVO_MIN_DUTY + (70 * (SERVO_MAX_DUTY - SERVO_MIN_DUTY) / 180)

static int64_t last_motor_activity_time = 0;
static bool is_servo_active = true;

// 서보 모터 PWM 설정
void configure_servo() {
  ledc_timer_config_t ledc_timer = {.speed_mode = LEDC_LOW_SPEED_MODE,
                                    .timer_num = LEDC_TIMER_0,
                                    .duty_resolution = LEDC_TIMER_13_BIT,
                                    .freq_hz = 50,
                                    .clk_cfg = LEDC_AUTO_CLK};
  ledc_timer_config(&ledc_timer);

  ledc_channel_config_t ledc_channel = {.speed_mode = LEDC_LOW_SPEED_MODE,
                                        .channel = LEDC_CHANNEL_0,
                                        .timer_sel = LEDC_TIMER_0,
                                        .intr_type = LEDC_INTR_DISABLE,
                                        .gpio_num = SERVO_PIN,
                                        .duty = SERVO_45_DEGREE_DUTY,  // 초기 위치
                                        .hpoint = 0};
  ledc_channel_config(&ledc_channel);
  is_servo_active = true;
}

// 서보 모터 각도 설정
void set_servo_angle(uint32_t duty) {
  // ledc_stop()으로 비활성화된 서보를 다시 활성화하기 위해
  // 먼저 duty를 설정하고 업데이트합니다.
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
  ESP_LOGI(TAG, "Servo set to duty: %lu", duty);
}

// 웹 페이지 HTML
extern const char _binary_index_html_start[];
extern const char _binary_index_html_end[];

// ON 버튼 핸들러
esp_err_t on_handler(httpd_req_t *req) {
  ESP_LOGI(TAG, "Servo ON");
  is_servo_active = true;  // 서보가 활성 상태임을 표시
  last_motor_activity_time = esp_timer_get_time();
  set_servo_angle(SERVO_MIN_DUTY);
  httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

// OFF 버튼 핸들러
esp_err_t off_handler(httpd_req_t *req) {
  ESP_LOGI(TAG, "Servo OFF");
  is_servo_active = true;  // 서보가 활성 상태임을 표시
  last_motor_activity_time = esp_timer_get_time();
  set_servo_angle(SERVO_45_DEGREE_DUTY);
  httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

// 메인 페이지 핸들러
esp_err_t root_handler(httpd_req_t *req) {
  const size_t html_size = (_binary_index_html_end - _binary_index_html_start);
  httpd_resp_send(req, _binary_index_html_start, html_size);
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
    httpd_uri_t root = {
        .uri = "/", .method = HTTP_GET, .handler = root_handler};
    httpd_register_uri_handler(server, &root);
    httpd_uri_t on = {.uri = "/on", .method = HTTP_GET, .handler = on_handler};
    httpd_register_uri_handler(server, &on);
    httpd_uri_t off = {
        .uri = "/off", .method = HTTP_GET, .handler = off_handler};
    httpd_register_uri_handler(server, &off);
  }
}

// 와이파이 연결 이벤트 핸들러
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    start_webserver();
  }
}

// 서보 모터의 전력을 관리하는 태스크
void servo_power_management_task(void *pvParameters) {
  while (1) {
    // 60초 동안 활동이 없으면 서보 모터 신호 차단
    if (is_servo_active &&
        (esp_timer_get_time() - last_motor_activity_time) > 60 * 1000 * 1000) {
      ESP_LOGI(TAG, "Detaching servo motor due to inactivity to save power.");
      ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);  // PWM 신호 중지
      is_servo_active = false;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));  // 1초마다 체크
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

  // 서보 설정
  configure_servo();
  ESP_LOGI(TAG, "Servo configured.");

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
              .ssid = CONFIG_WIFI_SSID,
              .password = CONFIG_WIFI_PASSWORD,
          },
  };
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI(TAG, "WiFi started. Connecting...");

  last_motor_activity_time = esp_timer_get_time();
  xTaskCreate(&servo_power_management_task, "servo_power_task", 2048, NULL, 5,
              NULL);
}