#include "battery_monitor.h"

#include <stdlib.h>

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

static const char* TAG = "BATTERY_MONITOR";

// ADC 설정
#define ADC_CHANNEL ADC_CHANNEL_6  // GPIO 34
#define NO_OF_SAMPLES 64           // Multisampling
#define VOLTAGE_DIVIDER_RATIO 2.0  // 10k + 10k voltage divider, so ratio is 2

static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t cali_handle = NULL;
static const adc_atten_t atten = ADC_ATTEN_DB_12;

void configure_adc() {
  // ADC 유닛 초기화
  adc_oneshot_unit_init_cfg_t init_config1 = {.unit_id = ADC_UNIT_1,
                                              .ulp_mode = ADC_ULP_MODE_DISABLE};
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

  // ADC 채널 설정
  adc_oneshot_chan_cfg_t config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = atten,
  };
  ESP_ERROR_CHECK(
      adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL, &config));

  // ADC 캘리브레이션 설정
  adc_cali_line_fitting_config_t cali_config = {
      .unit_id = ADC_UNIT_1,
      .atten = atten,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
  };
  esp_err_t ret =
      adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ADC Calibration failed.");
  }
}

battery_stats_t get_battery_stats() {
  uint32_t adc_reading = 0;
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    int raw;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL, &raw));
    adc_reading += raw;
  }
  adc_reading /= NO_OF_SAMPLES;

  int voltage_mv = 0;
  if (cali_handle) {
    ESP_ERROR_CHECK(
        adc_cali_raw_to_voltage(cali_handle, adc_reading, &voltage_mv));
  }

  float battery_voltage = (float)voltage_mv * VOLTAGE_DIVIDER_RATIO / 1000.0f;

  float min_v = 3.2f;
  float max_v = 4.2f;

  int percentage = (int)((battery_voltage - min_v) / (max_v - min_v) * 100.0f);

  if (percentage < 0) {
    percentage = 0;
  }
  if (percentage > 100) {
    percentage = 100;
  }
  ESP_LOGI(TAG, "Raw ADC: %ld, Voltage: %dmV, Battery: %.2fV, Percentage: %d%%",
           adc_reading, voltage_mv, battery_voltage, percentage);

  battery_stats_t stats = {.percentage = percentage,
                           .voltage = battery_voltage,
                           .adc_reading = adc_reading,
                           .min_v = min_v,
                           .max_v = max_v};
  return stats;
}
