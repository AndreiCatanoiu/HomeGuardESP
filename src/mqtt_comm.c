/*
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "MQTT";
static bool mqtt_connected = false;
static bool mqtt_subscribed = false;

static esp_mqtt_client_handle_t client;
static char mqtt_topic_up[100];
static char mqtt_topic_down[100];

static void log_error_if_nonzero(const char *message, int error_code)
{
	if (error_code != 0)
	{
		ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
	}
}

uint8_t mqtt_isconnected() 
{
	return mqtt_connected;
}

uint8_t mqtt_issubscribed() 
{
	if (mqtt_isconnected())
		return mqtt_subscribed;
	return false;
}

void mqtt_app_send(char* data, size_t len)
{
	if (mqtt_isconnected()==true)
	{
		int msg_id = esp_mqtt_client_publish(client, mqtt_topic_up, data, len, 0, 0);
		ESP_LOGD(TAG, "sent publish successful, msg_id=%d", msg_id);
	}
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
	esp_mqtt_event_handle_t event = event_data;
	esp_mqtt_client_handle_t client = event->client;
	int msg_id;
	switch ((esp_mqtt_event_id_t)event_id)
	{
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            mqtt_connected = true;
            msg_id = esp_mqtt_client_subscribe(client, mqtt_topic_down, 0);

            char server_inform[200];
            size_t server_inform_len = snprintf(server_inform, sizeof(server_inform), "Firmware: %s", getCompileDateTime());
            mqtt_app_send(server_inform, server_inform_len);
            

            ESP_LOGI(TAG, "Sent subscribe: topic=%s msg_id=%d", mqtt_topic_down, msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            mqtt_subscribed = false;
            mqtt_connected = true;
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            mqtt_subscribed = true;
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            mqtt_subscribed = false;
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGD(TAG, "New MQTT data: topic: %.*s", event->topic_len, event->topic);
            event->data[event->data_len] = 0;	
            decode(event->data, event->data_len, WIFI_MQTT);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
            {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
	}
}

void mqtt_app_init(void)
{
	if (client != NULL)
		return;

	char* mqtt_topic = (char*)settings_get_str(0xff, COMM_SETTING_STR, COMM_MQTT_TOPIC, 0);
	char* mqtt_up = (char*)settings_get_str(0xff, COMM_SETTING_STR, COMM_MQTT_UP, 0);
	char* mqtt_down = (char*)settings_get_str(0xff, COMM_SETTING_STR, COMM_MQTT_DOWN, 0);
	sprintf(mqtt_topic_down, "%s/%s/%s", mqtt_topic, mqtt_down,get_wifi_mac_string());
	sprintf(mqtt_topic_up, "%s/%s/%s", mqtt_topic, mqtt_up,get_wifi_mac_string());

	char mqtt_uri[100];
	snprintf(mqtt_uri, sizeof(mqtt_uri), "mqtt://%s", (char*)settings_get_str(0xff, COMM_SETTING_STR, COMM_MQTT_SERVER, 0));

	
	esp_mqtt_client_config_t mqtt_cfg = {
		.uri = mqtt_uri,
		.port = *(uint16_t*)settings_get_str(0x2, COMM_SETTING_STR, COMM_MQTT_PORT, 0),
		.username = (char*)settings_get_str(0xff, COMM_SETTING_STR, COMM_MQTT_USER, 0),
		.password = (char*)settings_get_str(0xff, COMM_SETTING_STR, COMM_MQTT_PASS, 0),
		.skip_cert_common_name_check = false,
	};

	client = esp_mqtt_client_init(&mqtt_cfg);
	esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
}

void mqtt_app_start(void)
{
	mqtt_app_init();
	esp_mqtt_client_start(client);
}

void mqtt_app_stop(void)
{
	esp_mqtt_client_stop(client);
}
*/