#include "wifi.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "lwip/ip4_addr.h"
#include "lwip/dns.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "mqtt_comm.h"
#include "esp_system.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

typedef enum
{
	INVALID       = 0,
	CONNECTING,
	CONNECTED,
	DISCONNECTING,
	DISCONNECTED
} WIFI_STATES;

typedef struct 
{
	char *ssid;
	char *pass;
}WifiAp_t;

typedef struct
{
	WIFI_STATES state;
	uint32_t down_timestamp;
	WifiAp_t *ap;
	uint8_t ip[4];
	uint8_t mac[6];
	uint8_t public_ip[4];
	uint8_t conn_err;
} WifiDetails_t;

static EventGroupHandle_t wifi_event_group;
static WifiDetails_t wifi_details;
// List of access points to connect to
static WifiAp_t APs[] = 
{
    (WifiAp_t){.ssid = "Your SSID", .pass = "Your Password" },
    (WifiAp_t){.ssid = "TotalRom_2.4G", .pass = "12345678" },
    (WifiAp_t){.ssid = "TotalRom_5G", .pass = "12345678" },
};
static uint8_t APs_count;

const static char* TAG = "WIFI";
const static int CONNECTED_BIT = BIT0;
const static int DISCONNECTED_BIT = BIT1;

static void wifi_event_handler(int32_t event_id)
{
	switch (event_id)
	{
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
            break;
        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            xEventGroupSetBits(wifi_event_group, DISCONNECTED_BIT);
            break;
        default:
            ESP_LOGW(TAG, "WIFI_EVENT_STA_UNKNOWN %ld", event_id);
            break;
	}
}

static void ip_event_handler(int32_t event_id, void* event_data)
{
	switch (event_id)
	{
        case IP_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
            memcpy(wifi_details.ip, &event->ip_info.ip, sizeof(wifi_details.ip));
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            xEventGroupClearBits(wifi_event_group, DISCONNECTED_BIT);
            break;
        default:
            ESP_LOGW(TAG, "IP_EVENT_STA_UNKNOWN %ld", event_id);
            break;
	}
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	if (event_base == WIFI_EVENT)
	{
		ESP_LOGI(TAG, "event_handler WIFI_EVENT");
		wifi_event_handler(event_id);
	}
	else if (event_base == IP_EVENT)
	{
		ESP_LOGI(TAG, "event_handler IP_EVENT");
		ip_event_handler(event_id, event_data);
	}
	else
	{
		ESP_LOGW(TAG, "event_handler UNKNOWN %ld", event_id);
	}
}

void wifi_init_sta(void)
{
    APs_count = sizeof(APs) / sizeof(APs[0]);

    ESP_LOGI(TAG, "Init %d access points finished.", APs_count);

    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = { 0 };
    snprintf((char *)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), "%s", APs[0].ssid);
    snprintf((char *)wifi_config.sta.password, sizeof(wifi_config.sta.password), "%s", APs[0].pass);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Init finished.");
}

static void wifi_connect(const char* ssid, const char* password)
{
	wifi_config_t wifi_config = { 0 };
	snprintf((char*)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), "%s", ssid);
	snprintf((char*)wifi_config.sta.password, sizeof(wifi_config.sta.password), "%s", password);
	ESP_LOGI(TAG, "Connecting to %s %s ...", wifi_config.sta.ssid, wifi_config.sta.password);
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	esp_err_t err = esp_wifi_connect();
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Connecting to %s, err %d", wifi_config.sta.ssid, err);
		wifi_details.conn_err++;
	}
	else
	{
		wifi_details.conn_err = 0;
	}
	if (wifi_details.conn_err >= 5)
		esp_restart();
}

void wifi_task_events(WIFI_STATES new_state)
{
	if (wifi_details.state == new_state)
	{
		ESP_LOGI(TAG, "Same state received %d", new_state);
		return;
	}

	switch (new_state)
	{
        case CONNECTING:
            ESP_LOGI(TAG, "Wifi connecting");
            wifi_details.state = new_state;
            break;
        case CONNECTED:
            ESP_LOGI(TAG, "Wifi connected");
            //mqtt_app_start();
            wifi_details.state = new_state;
            break;
        case DISCONNECTING:
            ESP_LOGI(TAG, "Wifi disconnecting");
            wifi_details.down_timestamp = esp_log_timestamp();
            wifi_details.state = new_state;
            break;
        case DISCONNECTED:
            ESP_LOGI(TAG, "Wifi disconnected");
            wifi_details.state = new_state;
            break;
        default:
            ESP_LOGI(TAG, "Unknown state received %d", new_state);
            break;
	}
}

void wifi_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Task started!");
    esp_efuse_mac_get_default(wifi_details.mac);
    wifi_init_sta();
    
    wifi_task_events(CONNECTING);
    EventBits_t bits = 0;
    uint8_t selected_ap = 0;

    while (1)
    {
        switch (wifi_details.state)
        {
            case CONNECTING:
                xEventGroupClearBits(wifi_event_group, CONNECTED_BIT | DISCONNECTED_BIT);
                wifi_connect(APs[selected_ap].ssid, APs[selected_ap].pass);
                wifi_details.ap = &APs[selected_ap];
                
                bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | DISCONNECTED_BIT, pdFALSE, pdFALSE, 10 * 1000 / portTICK_PERIOD_MS);

                if (bits & CONNECTED_BIT)
                {
                    wifi_task_events(CONNECTED); 
                }
                else if (bits & DISCONNECTED_BIT)
                {
                    selected_ap++;
                    selected_ap %= APs_count;
                }
                break;

            case CONNECTED:
                bits = xEventGroupGetBits(wifi_event_group);
                if (bits & DISCONNECTED_BIT)
                {
                    wifi_task_events(DISCONNECTING); 
                }
                break;

            case DISCONNECTING:
                if (esp_log_timestamp() - wifi_details.down_timestamp <= 60 * 1000)
                {
                    ESP_LOGI(TAG, "Wifi disconnected, reconnecting ...");
                    xEventGroupClearBits(wifi_event_group, CONNECTED_BIT | DISCONNECTED_BIT);
                    esp_wifi_connect();
                    bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | DISCONNECTED_BIT, pdFALSE, pdFALSE, 10 * 1000 / portTICK_PERIOD_MS);
                    if (bits & CONNECTED_BIT)
                    {
                        wifi_task_events(CONNECTED);
                    }
                }
                else
                {
                    wifi_task_events(DISCONNECTED);
                }
                break;

            case DISCONNECTED:
                wifi_task_events(CONNECTING);
                break;

            default:
                ESP_LOGE(TAG, "Task went wrong!");
                break;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


bool is_wifi_connected()
{
	return wifi_details.state == CONNECTED;
}

uint8_t *get_wifi_ip()
{
	return wifi_details.ip;
}

char *get_wifi_ip_string()
{
	static char ip[17];
	snprintf(ip, sizeof(ip), "%s", ip4addr_ntoa((ip4_addr_t*)wifi_details.ip));
	return ip;
}

uint8_t *get_wifi_broadcast_ip()
{
	static uint8_t broadcast_ip[4];
	memcpy(broadcast_ip, wifi_details.ip, 4);
	broadcast_ip[3] = 0xFF;
	ESP_LOGD("Test", "%x %x", *(int*)wifi_details.ip, *(int*)broadcast_ip);
	return broadcast_ip;
}

uint8_t *get_wifi_public_ip()
{
	return wifi_details.public_ip;
}

char *get_wifi_public_ip_string()
{
	static char ip[17];
	snprintf(ip, sizeof(ip), "%s", ip4addr_ntoa((ip4_addr_t*)wifi_details.public_ip));
	return ip;
}

uint8_t *get_wifi_mac()
{
	return wifi_details.mac;
}

char *get_wifi_mac_string()
{
	static char mac_str[20];
	uint8_t *mac_bin = wifi_details.mac;
	snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac_bin[0], mac_bin[1], mac_bin[2], mac_bin[3], mac_bin[4], mac_bin[5]);
	return mac_str;
}

int8_t get_wifi_rssi()
{
	wifi_ap_record_t wifi_info;
	esp_wifi_sta_get_ap_info(&wifi_info);
	return wifi_info.rssi;
}

uint8_t get_wifi_signal_strength()
{
	const int8_t rssi = get_wifi_rssi();
	if (rssi <= -80) 
		return 0;
	else if (rssi >= -30) 
		return 100;
	return 100 - (rssi + 30) * 2 * -1;
}

void get_wifi_details()
{
	ESP_LOGI(TAG, "\t\t -------- %s --------", TAG);
	ESP_LOGI(TAG, "State:   \t%d", wifi_details.state);
	ESP_LOGI(TAG, "DownT:   \t%ld", wifi_details.down_timestamp);
	ESP_LOGI(TAG, "SSID:    \t%s", wifi_details.ap->ssid);
	ESP_LOGI(TAG, "MAC:     \t%s", get_wifi_mac_string());
	ESP_LOGI(TAG, "wan IP:  \t%s", get_wifi_public_ip_string());
	ESP_LOGI(TAG, "lan IP:  \t%s", get_wifi_ip_string());
	ESP_LOGI(TAG, "RSSI:    \t%d", get_wifi_rssi());
	ESP_LOGI(TAG, "Err:		\t%d", wifi_details.conn_err);
}