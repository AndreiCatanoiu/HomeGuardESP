#include "system_time.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "wifi.h"
#include <stdlib.h>
#include <string.h>

#define MIN_ACCEPTED_YEAR    2025
#define MIN_ACCEPTED_MONTH   2
#define MIN_ACCEPTED_DAY     21

static const char *TAG = "SYSTEM_TIME";

typedef enum 
{
    TIME_INVALID,
    TIME_RECOVERED,
    TIME_EXTERN,
    TIME_SYNCED
} TIME_STATE;

static TIME_STATE time_state = TIME_INVALID;

static time_t now;
static struct tm timeinfo;
static char time_now_str[64];

static void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

static void initialize_sntp(void)
{
    static uint8_t init = 0;
    if (init)
        return;
    init = 1;
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
	sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();
}

static uint8_t is_time_valid(void)
{
    return (timeinfo.tm_year >= (MIN_ACCEPTED_YEAR - 1900));
}

void time_setup(void)
{
    setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4", 1);
    tzset();

    time(&now);
    localtime_r(&now, &timeinfo);

    if (is_time_valid())
    {
        char recovery[50];
        strftime(recovery, sizeof(recovery), "%b %d %Y %H:%M:%S", &timeinfo);
        ESP_LOGI(TAG, "Time recovered: %s", recovery);
        time_state = TIME_RECOVERED;
    }

    while (!is_wifi_connected())
    {
		vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
	initialize_sntp();

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(time_now_str, sizeof(time_now_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
    ESP_LOGI(TAG, "Current time is %s", time_now_str);
}

void time_update(void)
{
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(time_now_str, sizeof(time_now_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
    ESP_LOGI(TAG, "Updated time is %s", time_now_str);
}

uint8_t is_time_set(void)
{
    return (time_state != TIME_INVALID);
}

uint64_t get_sys_time(uint8_t units)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000) / units;
}

void seconds_to_time(uint32_t seconds, char *time_out, size_t time_out_size)
{
    if (time_out == NULL)
        return;
    time_t sec = seconds;
    struct tm seconds_struct;
    localtime_r(&sec, &seconds_struct);
    strftime(time_out, time_out_size, "%Y-%m-%d %H:%M:%S", &seconds_struct);
}

char* actual_time_string() 
{
    time_t now = time(NULL);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    static char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return time_str;
}