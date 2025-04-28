#ifndef __WIFI_H__
#define __WIFI_H__

#include "freertos/FreeRTOS.h"
#include "lwip/ip_addr.h"
#include "esp_http_server.h"
#include "settings.h"
#define AP_BUTTON GPIO_NUM_0

void wifi_task(void *pvParameters);

bool is_wifi_connected();

uint8_t *get_wifi_ip();
char *get_wifi_ip_string();

uint8_t *get_wifi_broadcast_ip();

uint8_t *get_wifi_public_ip();
char *get_wifi_public_ip_string();

uint8_t *get_wifi_mac();
char *get_wifi_mac_string();

int8_t get_wifi_rssi();
uint8_t get_wifi_signal_strength();

void get_wifi_details();

#endif