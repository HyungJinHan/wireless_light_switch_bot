#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "driver/adc.h"
#include "driver/ledc.h"
#include "esp_adc_cal.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "secrets.h"

static const char* TAG = "SERVO_WEB_CONTROL";

// ADC 설정
#define ADC_CHANNEL ADC1_CHANNEL_6  // GPIO 34
#define DEFAULT_VREF \
  1100  // Use adc2_vref_to_gpio() to get value on ADC2_CHANNEL2
#define NO_OF_SAMPLES 64           // Multisampling
#define VOLTAGE_DIVIDER_RATIO 2.0  // 10k + 10k voltage divider, so ratio is 2

static esp_adc_cal_characteristics_t* adc_chars;
static const adc_unit_t unit = ADC_UNIT_1;
static const adc_atten_t atten = ADC_ATTEN_DB_11;

// 배터리 전압 읽기 함수 프로토타입
typedef struct {
  int percentage;
  float voltage;
  uint32_t adc_reading;
  float min_v;
  float max_v;
} battery_stats_t;

battery_stats_t get_battery_stats();

// 설정
#define SERVO_PIN 14

// SG90 서보 설정 (50Hz, 13-bit 해상도 기준)
// 0도: 약 164 (0.5ms), 90도: 약 410 (1.25ms), 180도: 약 655 (2.0ms)
#define SERVO_MIN_DUTY 180
#define SERVO_MAX_DUTY 819
#define SERVO_45_DEGREE_DUTY \
  SERVO_MIN_DUTY + (70 * (SERVO_MAX_DUTY - SERVO_MIN_DUTY) / 180)

static int64_t last_motor_activity_time = 0;
static bool is_servo_active = true;
static bool is_on = false;

// 서보 모터 PWM 설정
void configure_servo() {
  ledc_timer_config_t ledc_timer = {.speed_mode = LEDC_LOW_SPEED_MODE,
                                    .timer_num = LEDC_TIMER_0,
                                    .duty_resolution = LEDC_TIMER_13_BIT,
                                    .freq_hz = 50,
                                    .clk_cfg = LEDC_AUTO_CLK};
  ledc_timer_config(&ledc_timer);

  ledc_channel_config_t ledc_channel = {
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .channel = LEDC_CHANNEL_0,
      .timer_sel = LEDC_TIMER_0,
      .intr_type = LEDC_INTR_DISABLE,
      .gpio_num = SERVO_PIN,
      .duty = SERVO_45_DEGREE_DUTY,  // 초기 위치 (OFF 상태)
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
extern const char _binary_style_css_start[];
extern const char _binary_style_css_end[];

// 토글 핸들러
esp_err_t toggle_handler(httpd_req_t* req) {
  is_on = !is_on;
  ESP_LOGI(TAG, "Servo %s", is_on ? "ON" : "OFF");
  is_servo_active = true;
  last_motor_activity_time = esp_timer_get_time();
  set_servo_angle(is_on ? SERVO_MIN_DUTY : SERVO_45_DEGREE_DUTY);
  httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

// 상태 핸들러
esp_err_t status_handler(httpd_req_t* req) {
  char resp[64];
  sprintf(resp, "{\"status\": \"%s\"}", is_on ? "ON" : "OFF");
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

httpd_handle_t server = NULL;

// ADC 설정
void configure_adc() {
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC_CHANNEL, atten);
  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF,
                           adc_chars);
}

// 배터리 잔량(%) 및 전압 반환
battery_stats_t get_battery_stats() {
  uint32_t adc_reading = 0;
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    adc_reading += adc1_get_raw(ADC_CHANNEL);
  }
  adc_reading /= NO_OF_SAMPLES;

  uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
  float battery_voltage = (float)voltage_mv * VOLTAGE_DIVIDER_RATIO / 1000.0f;

  // 3.7V LiPo 배터리 전압 범위 (3.2V ~ 4.2V)
  float min_v = 3.2f;
  float max_v = 4.2f;

  int percentage = (int)((battery_voltage - min_v) / (max_v - min_v) * 100.0f);

  if (percentage < 0) {
    percentage = 0;
  }
  if (percentage > 100) {
    percentage = 100;
  }
  ESP_LOGI(TAG,
           "Raw ADC: %ld, Voltage: %ldmV, Battery: %.2fV, Percentage: %d%%",
           adc_reading, voltage_mv, battery_voltage, percentage);

  battery_stats_t stats = {.percentage = percentage,
                           .voltage = battery_voltage,
                           .adc_reading = adc_reading,
                           .min_v = min_v,
                           .max_v = max_v};
  return stats;
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
    // 추가: Wi-Fi가 끊겼을 때 ON 상태였으면 OFF로 변경
    if (is_on) {
      ESP_LOGI(TAG, "Servo was ON, setting to OFF due to Wi-Fi disconnection.");
      is_on = false;
      set_servo_angle(SERVO_45_DEGREE_DUTY);  // OFF 상태로 변경
    }
    esp_wifi_connect();
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
    ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    start_webserver();
  }
}

// 서보 모터의 전력을 관리하는 태스크
void servo_power_management_task(void* pvParameters) {
  while (1) {
    if (is_servo_active &&
        (esp_timer_get_time() - last_motor_activity_time) > 2 * 1000 * 1000) {
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

  // ADC 설정
  configure_adc();
  ESP_LOGI(TAG, "ADC configured.");

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

  last_motor_activity_time = esp_timer_get_time();
  xTaskCreate(&servo_power_management_task, "servo_power_task", 2048, NULL, 5,
              NULL);
}