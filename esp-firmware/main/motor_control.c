#include "motor_control.h"

#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "MOTOR_CONTROL";

// 설정
#define SERVO_PIN 14

// SG90 서보 설정 (50Hz, 13-bit 해상도 기준)
#define SERVO_MIN_DUTY 180
#define SERVO_MAX_DUTY 819
#define SERVO_45_DEGREE_DUTY \
  SERVO_MIN_DUTY + (70 * (SERVO_MAX_DUTY - SERVO_MIN_DUTY) / 180)

static int64_t last_motor_activity_time = 0;
static bool is_servo_active = true;
static bool is_on = false;

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
  last_motor_activity_time = esp_timer_get_time();
}

static void set_servo_angle(uint32_t duty) {
  if (!is_servo_active) {
    // 서보가 비활성 상태이면 다시 활성화
    configure_servo();
  }
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
  ESP_LOGI(TAG, "Servo set to duty: %lu", duty);
  update_servo_activity();
}

void set_servo_angle_on() {
  set_servo_angle(SERVO_MIN_DUTY);
  is_on = true;
}

void set_servo_angle_off() {
  set_servo_angle(SERVO_45_DEGREE_DUTY);
  is_on = false;
}

void update_servo_activity() {
  is_servo_active = true;
  last_motor_activity_time = esp_timer_get_time();
}

void servo_power_management_task(void* pvParameters) {
  while (1) {
    if (is_servo_active &&
        (esp_timer_get_time() - last_motor_activity_time) > 2 * 1000 * 1000) {
      ESP_LOGI(TAG, "Detaching servo motor due to inactivity to save power.");
      ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
      is_servo_active = false;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

bool get_is_on() { return is_on; }

void set_is_on(bool status) { is_on = status; }
