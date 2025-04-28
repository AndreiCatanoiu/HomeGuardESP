#pragma once
#include <stdint.h>
#include "esp_err.h"

typedef enum 
{
    SENSOR_STATUS_DOWN=0,
    SENSOR_STATUS_UP,
    SENSOR_STATUS_MAINTENANCE,
} sensor_status_t;

#define WIFI_SSID_DEFAULT    "TotalRom_2.4GHz"
#define WIFI_PASS_DEFAULT    "totalrom"

#define MQTT_SERVER_DEFAULT  "www.andreicatanoiu.ro"
#define MQTT_PORT_DEFAULT    1883
#define MQTT_USER_DEFAULT    "admin"
#define MQTT_PASS_DEFAULT    "admin"
#define MQTT_TOPIC_DEFAULT   "senzor/licenta/andrei/catanoiu"
#define MQTT_UP_DEFAULT      "up"
#define MQTT_DOWN_DEFAULT    "down"

#define SENSOR_ID_DEFAULT    "SenzorGaz-1"
#define SENSOR_ID_ENCODED_DEFAULT    ""
#define SENSOR_STATUS_DEFAULT  SENSOR_STATUS_UP
#define SENSOR_FIRMWARE_VERSION  "1.0.0"

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
#define KEY_SENSOR_ID_ENCODED "encoded_id"
#define KEY_SENSOR_STATUS "sensor_status"
#define KEY_SENSOR_FIRMWARE_VERSION "firmware_vs"

typedef struct 
{
    char wifi_ssid[32];
    char wifi_pass[32];
    char mqtt_server[100];
    uint16_t mqtt_port;
    char mqtt_user[32];
    char mqtt_pass[100];
    char mqtt_topic[150];
    char mqtt_up[32];
    char mqtt_down[32];
    char encoded_sensor_id[100];
    char decoded_sensor_id[100];
    sensor_status_t status;
    char firmware_version[64];
} app_settings_t;

extern app_settings_t s_settings;

void settings_init(void);
void settings_save(void);
esp_err_t wifi_ap_set(const char *ssid, const char *password);
void print_all_settings(void);
esp_err_t settings_set(const char *key, void *value, size_t size, bool is_string);
void settings_reset(void);