#include "wifi.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "lwip/ip4_addr.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "mqtt_comm.h"
#include "esp_system.h"
#include "system_time.h"
#include "esp_http_server.h"
#include "dns_server.h" 

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include <settings.h>

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
	const char *ssid;
	const char *pass;
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

static void parse_form_data(char* buf, char* field_name, char* output, size_t output_size);
static void configure_button();
static bool check_for_ap_press();
static httpd_handle_t server = NULL;
static EventGroupHandle_t wifi_event_group;
static WifiDetails_t wifi_details;
static WifiAp_t APs[] = 
{
    (WifiAp_t){.ssid = s_settings.wifi_ssid , .pass = s_settings.wifi_pass },
};
static uint8_t APs_count;

const static char* TAG = "WIFI";
const static int CONNECTED_BIT = BIT0;
const static int DISCONNECTED_BIT = BIT1;

void configure_button(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << AP_BUTTON),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}

bool check_for_ap_press(void) {
    if (gpio_get_level(AP_BUTTON) == 0) {
        vTaskDelay(50 / portTICK_PERIOD_MS);
        if (gpio_get_level(AP_BUTTON) == 0) {
            ESP_LOGI(TAG, "Configuration button pressed → AP mode");
            return true;
        }
    }
    return false;
}

static void start_captive_dns(void) {
    dns_server_config_t cfg = DNS_SERVER_CONFIG_SINGLE(
        "*",           
        "WIFI_AP_DEF"
    );
    start_dns_server(&cfg);
    ESP_LOGI(TAG, "Captive DNS started");
}

static esp_err_t save_wifi_config_handler(httpd_req_t *req) {
    char buf[200];
    int ret, remaining = req->content_len;
    
    if (remaining > sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Content too long");
        return ESP_FAIL;
    }
    
    while (remaining > 0) {
        size_t chunk = (remaining < sizeof(buf)) ? remaining : sizeof(buf);
        ret = httpd_req_recv(req, buf, chunk);
        if (ret <= 0) {
            return ESP_FAIL;
        }
        remaining -= ret;
    }
    buf[req->content_len] = '\0';
    
    char ssid[33] = {0};
    char pass[65] = {0};
    
    parse_form_data(buf, "ssid", ssid, sizeof(ssid));
    parse_form_data(buf, "pass", pass, sizeof(pass));
    
    if (strlen(ssid) > 0) {
        settings_set(WIFI_SSID, ssid, strlen(ssid) + 1, true);
        settings_set(WIFI_PASS, pass, strlen(pass) + 1, true);
        
        const char* html = "<!DOCTYPE html><html><head>"
            "<title>Succes</title><meta http-equiv='refresh' content='5;url=/'>"
            "<style>body{font-family:Arial;text-align:center;padding-top:50px;}</style>"
            "</head><body>"
            "<h1>Configuration saved!</h1>"
            "<p>The device will restart and connect to the WiFi network.</p>"
            "</body></html>";
        
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, html, strlen(html));
        
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        esp_restart();
        return ESP_OK;
    }
    
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid parameters");
    return ESP_FAIL;
}

static esp_err_t cp_redirect_handler(httpd_req_t *req) {
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t handle_default(httpd_req_t *req) {
    ESP_LOGI(TAG, "Handling default URI");
    const char* html = "<html><body><h1>Captive Portal</h1></body></html>";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html, strlen(html));
    return ESP_OK;
}

static esp_err_t root_handler(httpd_req_t *req) 
{
    const char* html = "<!DOCTYPE html><html><head>"
        "<title>ESP32 WiFi Setup</title>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<style>body{font-family:Arial;margin:0;padding:20px;} "
        "form{max-width:400px;margin:0 auto;} "
        "input[type=text],input[type=password]{width:100%;padding:12px 20px;margin:8px 0;display:inline-block;border:1px solid #ccc;box-sizing:border-box;} "
        "button{background-color:#4CAF50;color:white;padding:14px 20px;margin:8px 0;border:none;cursor:pointer;width:100%;} "
        "button:hover{opacity:0.8;}</style>"
        "</head><body>"
        "<form action='/save-config' method='post'>"
        "<h2>WiFi Configuration</h2>"
        "<label for='ssid'><b>WiFi name (SSID)</b></label>"
        "<input type='text' placeholder='Enter SSID' name='ssid' required>"
        "<label for='pass'><b>Password</b></label>"
        "<input type='password' placeholder='Enter password' name='pass' required>"
        "<button type='submit'>Save and connect</button>"
        "</form>"
        "</body></html>";
        
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html, strlen(html));
    return ESP_OK;
}

static void start_captive_httpd(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 4096;
    
    config.max_open_sockets = 4;
    config.server_port = 80;
    config.lru_purge_enable = true;
    config.max_uri_handlers = 10;
    config.max_resp_headers = 16;
    config.recv_wait_timeout = 5;
    config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root = {
            .uri      = "/",
            .method   = HTTP_GET,
            .handler  = root_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &root);

        httpd_uri_t save = {
            .uri      = "/save-config",
            .method   = HTTP_POST,
            .handler  = save_wifi_config_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &save);

        httpd_uri_t favicon = {
            .uri      = "/favicon.ico",
            .method   = HTTP_GET,
            .handler  = cp_redirect_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &favicon);

        const char* paths[] = {
            "/hotspot-detect.html",
            "/success.html",
            "/generate_204",
            "/generate204",
            "/connecttest.txt",
            "/ncsi.txt",
            "/redirect",
            "/library/test/success.html",
            "/hotspot.html",
            "/canonical.html",
            "/mobile/status.php",
        };

        for (size_t i = 0; i < sizeof(paths)/sizeof(paths[0]); ++i) {
            httpd_uri_t cap = {
                .uri      = paths[i],
                .method   = HTTP_GET,
                .handler  = cp_redirect_handler,
                .user_ctx = NULL
            };
            httpd_register_uri_handler(server, &cap);
        }

        httpd_uri_t catchall = {
            .uri      = "/*",
            .method   = HTTP_GET,
            .handler  = handle_default,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &catchall);

        ESP_LOGI(TAG, "Captive portal HTTP server started");
    } else {
        ESP_LOGE(TAG, "Error starting HTTP server");
    }
}

void start_wifi_ap(void) {
    char ssid[150]; 
    snprintf(ssid, sizeof(ssid), "Device id: %s", s_settings.encoded_sensor_id);
    ssid[33] = '\0';
    
    wifi_config_t wifi_config = { 0 };
    strlcpy((char*)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid));
    strlcpy((char*)wifi_config.ap.password, "HomeGuard", sizeof(wifi_config.ap.password));
    wifi_config.ap.ssid_len      = strlen(ssid);
    wifi_config.ap.authmode      = WIFI_AUTH_WPA2_PSK;
    wifi_config.ap.max_connection= 4;
    wifi_config.ap.channel       = 1;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "AP mode started: %s", wifi_config.ap.ssid);

    start_captive_dns();
    start_captive_httpd();
}

static void url_decode(char* dst, const char* src) {
    char a, b;
    while (*src) {
        if ((*src == '%') && ((a = src[1]) && (b = src[2])) && 
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a'-'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            
            if (b >= 'a') b -= 'a'-'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            
            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}

static void parse_form_data(char* buf, char* field_name, char* output, size_t output_size) {
    char param_name[64];
    snprintf(param_name, sizeof(param_name), "%s=", field_name);
    
    char* start = strstr(buf, param_name);
    if (!start) {
        output[0] = '\0';
        return;
    }
    
    start += strlen(param_name);
    char* end = strchr(start, '&');
    if (!end) {
        end = start + strlen(start);
    }
    
    size_t param_len = end - start;
    if (param_len >= output_size) {
        param_len = output_size - 1;
    }
    
    memcpy(output, start, param_len);
    output[param_len] = '\0';
    url_decode(output, output);
}

void check_for_ap_mode_request(void) 
{
    const gpio_num_t CONFIG_BUTTON_PIN = AP_BUTTON;  
    
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << CONFIG_BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    if (gpio_get_level(CONFIG_BUTTON_PIN) == 0) {
        vTaskDelay(50 / portTICK_PERIOD_MS);
        
        if (gpio_get_level(CONFIG_BUTTON_PIN) == 0) {
            ESP_LOGI(TAG, "Configuration button pressed, switches to AP mode");
            start_wifi_ap();
        }
    }
}

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
            
            time_setup();
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
    static esp_event_handler_instance_t inst_any_id, inst_got_ip;

    ESP_ERROR_CHECK( esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID,
        &event_handler, NULL,
        &inst_any_id
        ) );

    ESP_ERROR_CHECK( esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP,
        &event_handler, NULL,
        &inst_got_ip
        ) );
    
    if (wifi_event_group == NULL) {
        wifi_event_group = xEventGroupCreate();
        configASSERT(wifi_event_group);
    }

    xEventGroupClearBits(wifi_event_group, CONNECTED_BIT | DISCONNECTED_BIT);

    wifi_config_t wifi_config = { 0 };
    snprintf((char*)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid),
             "%s", s_settings.wifi_ssid);
    snprintf((char*)wifi_config.sta.password, sizeof(wifi_config.sta.password),
             "%s", s_settings.wifi_pass);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "STA mode started, connecting to %s", wifi_config.sta.ssid);
}

static void wifi_connect(const char* ssid, const char* password)
{
	wifi_config_t wifi_config = { 0 };
	snprintf((char*)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), "%s", ssid);
	snprintf((char*)wifi_config.sta.password, sizeof(wifi_config.sta.password), "%s", password);
	ESP_LOGI(TAG, "Connecting to %s ...", wifi_config.sta.ssid);
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
            mqtt_app_start();
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

void wifi_task(void *pvParameters) {
    ESP_LOGI(TAG, "WiFi task pornit");
    esp_efuse_mac_get_default(wifi_details.mac);

    configure_button();

    bool has_credentials = (strlen(s_settings.wifi_ssid) > 0);
    if (!has_credentials) {
        start_wifi_ap();
        for(;;) vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    wifi_init_sta();
    wifi_task_events(CONNECTING);

    EventBits_t bits;
    uint8_t selected_ap = 0;

    while (1) {
        if (check_for_ap_press()) {
            esp_wifi_stop();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            start_wifi_ap();
            vTaskDelete(NULL);
        }

        switch (wifi_details.state) {
            case CONNECTING:
                xEventGroupClearBits(wifi_event_group, CONNECTED_BIT | DISCONNECTED_BIT);
                wifi_connect(APs[selected_ap].ssid, APs[selected_ap].pass);
                wifi_details.ap = &APs[selected_ap];
                bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | DISCONNECTED_BIT,
                                           pdFALSE, pdFALSE, 10000 / portTICK_PERIOD_MS);
                if (bits & CONNECTED_BIT) wifi_task_events(CONNECTED);
                else if (bits & DISCONNECTED_BIT) {
                    wifi_details.conn_err++;
                    if (wifi_details.conn_err >= 3) {
                        start_wifi_ap(); 
                        vTaskDelete(NULL);
                    }
                    selected_ap = (selected_ap + 1) % APs_count;
                }
                break;
            case CONNECTED:
                bits = xEventGroupGetBits(wifi_event_group);
                if (bits & DISCONNECTED_BIT) wifi_task_events(DISCONNECTING);
                break;
            case DISCONNECTING:
                if (esp_log_timestamp() - wifi_details.down_timestamp <= 60000) {
                    mqtt_app_stop();
                    esp_wifi_connect();
                    bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | DISCONNECTED_BIT,
                                               pdFALSE, pdFALSE, 10000 / portTICK_PERIOD_MS);
                    if (bits & CONNECTED_BIT) wifi_task_events(CONNECTED);
                } else {
                    wifi_task_events(DISCONNECTED);
                }
                break;
            case DISCONNECTED:
                wifi_task_events(CONNECTING);
                break;
            default:
                ESP_LOGE(TAG, "Unknown state %d", wifi_details.state);
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