#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <stdint.h>

typedef struct {
  int percentage;
  float voltage;
  uint32_t adc_reading;
  float min_v;
  float max_v;
} battery_stats_t;

void configure_adc();
battery_stats_t get_battery_stats();

#endif // BATTERY_MONITOR_H
