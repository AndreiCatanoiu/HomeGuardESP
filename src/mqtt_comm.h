#ifndef __MQTT_COMM_H__
#define __MQTT_COMM_H__

#include "freertos/FreeRTOS.h"

#define COMM_MQTT_SERVER	"your_server"
#define COMM_MQTT_PORT		1883
#define COMM_MQTT_USER		"your_user_name"
#define COMM_MQTT_PASS		"your_user_password"
#define COMM_MQTT_TOPIC		"your_topic"
#define COMM_MQTT_UP		"up"
#define COMM_MQTT_DOWN		"down"

void mqtt_app_init(void);
void mqtt_app_start(void);
void mqtt_app_stop(void);
void mqtt_app_send(char* data, size_t len);

uint8_t is_mqtt_connected();
uint8_t is_mqtt_subscribed();

#endif