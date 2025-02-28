#ifndef __MQTT_COMM_H__
#define __MQTT_COMM_H__

#include "freertos/FreeRTOS.h"

void mqtt_app_init(void);
void mqtt_app_start(void);
void mqtt_app_stop(void);
void mqtt_app_send(char* data, size_t len, const char* mqtt_up);
void is_device_available(void *pvParameter);

uint8_t is_mqtt_connected();
uint8_t is_mqtt_subscribed();

#endif