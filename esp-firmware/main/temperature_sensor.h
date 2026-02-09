#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include "onewire_bus.h"

void temperature_sensor_init();
float get_temperature_celsius();
void ds18b20_read_task(void *pvParameters);


#endif // TEMPERATURE_SENSOR_H
