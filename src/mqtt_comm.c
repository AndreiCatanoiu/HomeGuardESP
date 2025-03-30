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
#include "settings.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "mqtt_comm.h"
#include "settings.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "decoder.h"

static const char *TAG = "MQTT";
static bool mqtt_connected = false;
static bool mqtt_subscribed = false;

static esp_mqtt_client_handle_t client;
static char mqtt_topic_down[300];

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

void mqtt_app_send(char* data, size_t len, const char* mqtt_up)
{
	if (mqtt_isconnected()==true)
	{
        char mqtt_topic_up[300];
        sprintf(mqtt_topic_up, "%s/%s/%s", s_settings.mqtt_topic, s_settings.encoded_sensor_id,mqtt_up);
		int msg_id = esp_mqtt_client_publish(client, mqtt_topic_up, data, len, 0, 0);
		ESP_LOGD(TAG, "sent publish successful, msg_id=%d", msg_id);
	}
}

void is_device_available(void *pvParameter)
{
    while (1)
    {
        static char availity[16];
        if (s_settings.status == SENSOR_STATUS_UP)
        {   
            ESP_LOGI(TAG, "Device is available");
            strcpy(availity,"alive");
        }
        else if (s_settings.status == SENSOR_STATUS_MAINTENANCE)
        {
            ESP_LOGI(TAG, "Device is in maintenance mode");
            strcpy(availity,"maintenance");
        }
        mqtt_app_send(availity, strlen(availity)+1, "availity");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
	esp_mqtt_event_handle_t event = event_data;
	esp_mqtt_client_handle_t client = event->client;
	int msg_id;
	switch ((esp_mqtt_event_id_t)event_id)
	{
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            mqtt_connected = true;
            msg_id = esp_mqtt_client_subscribe(client, mqtt_topic_down, 0);

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
            ESP_LOGD(TAG, "New MQTT data: data: %.*s", event->data_len, event->data);
            decoder_process_command(event->data);
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

	char mqtt_topic[250];
    snprintf(mqtt_topic ,sizeof(mqtt_topic),"%s/%s", s_settings.mqtt_topic, s_settings.encoded_sensor_id);
	char* mqtt_down = s_settings.mqtt_down;
	sprintf(mqtt_topic_down, "%s/%s", mqtt_topic, mqtt_down);

	char mqtt_uri[150];
	snprintf(mqtt_uri, sizeof(mqtt_uri), "mqtt://%s", s_settings.mqtt_server);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = mqtt_uri,                
                .port = s_settings.mqtt_port,         
            },
        },
        .credentials = {
            .username = s_settings.mqtt_user,
            .authentication = {
                .password = s_settings.mqtt_pass,
            }
        }
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