#pragma once
#include <stdint.h>

#define WIFI_SSID_DEFAULT    "TotalRom_2.4GHz"
#define WIFI_PASS_DEFAULT    "totalrom"

#define MQTT_SERVER_DEFAULT  "catanoiulicenta.ddns.net"
#define MQTT_PORT_DEFAULT    1883
#define MQTT_USER_DEFAULT    "admin"
#define MQTT_PASS_DEFAULT    "admin"
#define MQTT_TOPIC_DEFAULT   "senzor/licenta/andrei/catanoiu/type"
#define MQTT_UP_DEFAULT      "up"
#define MQTT_DOWN_DEFAULT    "down"

#define SENSOR_ID_DEFAULT    120

#define NVS_NAMESPACE "app_settings"

#define WIFI_SSID         "wifi_ssid"
#define WIFI_PASS         "wifi_pass"

#define COMM_MQTT_SERVER  "mqtt_server"
#define COMM_MQTT_PORT    "mqtt_port"
#define COMM_MQTT_USER    "mqtt_user"
#define COMM_MQTT_PASS    "mqtt_pass"
#define COMM_MQTT_TOPIC   "mqtt_topic"
#define COMM_MQTT_UP      "mqtt_up"
#define COMM_MQTT_DOWN    "mqtt_down"

#define KEY_SENSOR_ID    "sensor_id"

typedef struct {
    char wifi_ssid[32];
    char wifi_pass[32];
    char mqtt_server[100];
    uint16_t mqtt_port;
    char mqtt_user[32];
    char mqtt_pass[100];
    char mqtt_topic[150];
    char mqtt_up[32];
    char mqtt_down[32];
    uint16_t sensor_id;
} app_settings_t;

extern app_settings_t s_settings;

void settings_init(void);
void settings_save(void);
void settings_erase_all(void);
app_settings_t* settings_get(void);
