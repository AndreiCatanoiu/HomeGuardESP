#ifndef __MQTT_COMM_H__
#define __MQTT_COMM_H__

#include "freertos/FreeRTOS.h"

#define COMM_MQTT_SERVER	"MQTT_SERVER"
#define COMM_MQTT_PORT		"MQTT_PORT"
#define COMM_MQTT_USER		"MQTT_USER"
#define COMM_MQTT_PASS		"MQTT_PASS"
#define COMM_MQTT_TOPIC		"MQTT_TOPIC"
#define COMM_MQTT_UP		"MQTT_UP"
#define COMM_MQTT_DOWN		"MQTT_DOWN"

void mqtt_app_init(void);
void mqtt_app_start(void);
void mqtt_app_stop(void);
void mqtt_app_send(char* data, size_t len);

uint8_t is_mqtt_connected();
uint8_t is_mqtt_subscribed();

#endif