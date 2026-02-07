#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdbool.h>

void configure_servo();
void set_servo_angle_on();
void set_servo_angle_off();
void update_servo_activity();
void servo_power_management_task(void* pvParameters);
bool get_is_on();
void set_is_on(bool status);

#endif // MOTOR_CONTROL_H
