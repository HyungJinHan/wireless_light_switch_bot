#include "temperature_sensor.h"

#include "ds18b20.h"
#include "onewire_bus.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "TEMP_SENSOR";

#define ONEWIRE_BUS_GPIO 26

static float temperature_celsius = -127.0f;  // DS18B20 초기값
static onewire_bus_handle_t bus_handle;
static ds18b20_device_handle_t ds18b20_handle = NULL;

void temperature_sensor_init() {
  onewire_bus_config_t bus_config = {
      .bus_gpio_num = ONEWIRE_BUS_GPIO,
  };
  onewire_bus_rmt_config_t rmt_config = {
      .max_rx_bytes = 10,
  };
  ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus_handle));
  ESP_LOGI(TAG, "1-Wire bus initialized");

  vTaskDelay(pdMS_TO_TICKS(100));  // 버스 안정화를 위한 딜레이

  onewire_device_iter_handle_t iter = NULL;
  onewire_device_t onewire_dev;
  esp_err_t result = ESP_OK;

  ESP_ERROR_CHECK(onewire_new_device_iter(bus_handle, &iter));
  ESP_LOGI(TAG, "Device iterator created");

  if (onewire_device_iter_get_next(iter, &onewire_dev) == ESP_OK) {
    ds18b20_config_t config = {};
    result = ds18b20_new_device_from_enumeration(&onewire_dev, &config,
                                                 &ds18b20_handle);
    if (result != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create DS18B20 device: %s",
               esp_err_to_name(result));
    }
  } else {
    ESP_LOGE(TAG, "No 1-Wire devices found on GPIO %d", ONEWIRE_BUS_GPIO);
  }

  onewire_del_device_iter(iter);

  if (ds18b20_handle == NULL) {
    ESP_LOGE(TAG, "DS18B20 not found, stopping init.");
    return;
  }

  ESP_LOGI(TAG, "DS18B20 device initialized");
  xTaskCreate(&ds18b20_read_task, "ds18b20_read_task", 2048, NULL, 5, NULL);
}

void ds18b20_read_task(void* pvParameters) {
  if (ds18b20_handle == NULL) {
    ESP_LOGE(TAG, "DS18B20 handle is not initialized. Deleting task.");
    vTaskDelete(NULL);
    return;
  }

  while (1) {
    esp_err_t conv_result = ds18b20_trigger_temperature_conversion(ds18b20_handle);
    if (conv_result != ESP_OK) {
      ESP_LOGE(TAG, "Failed to trigger temperature conversion: %s", esp_err_to_name(conv_result));
      vTaskDelay(pdMS_TO_TICKS(5000));
      continue;
    }
    vTaskDelay(pdMS_TO_TICKS(800));

    float temp;
    esp_err_t result = ds18b20_get_temperature(ds18b20_handle, &temp);
    if (result == ESP_OK) {
      temperature_celsius = temp;
      ESP_LOGI(TAG, "Temperature: %.2f°C", temperature_celsius);
    } else {
      ESP_LOGE(TAG, "Failed to read temperature. Error: %s",
               esp_err_to_name(result));
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

float get_temperature_celsius() { return temperature_celsius; }
