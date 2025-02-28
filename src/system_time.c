/*#include "system_time.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "wifi.h"

#define MIN_ACCEPTED_YEAR	2024
#define MIN_ACCEPTED_MONTH	3
#define MIN_ACCEPTED_DAY	4

static const char *TAG = "TIME";

typedef enum
{
	TIME_INVALID,
	TIME_RECOVERED,
	TIME_EXTERN,
	TIME_SYNCED
}TIME_STATE;

TIME_STATE time_state = TIME_INVALID;

static uint8_t is_time_valid()
{
	const uint8_t year_ok = timeinfo.tm_year >= (MIN_ACCEPTED_YEAR - 1900);
	return year_ok;
}

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
	sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
	esp_sntp_init();
}

void time_setup()
{
	time(&now);
	localtime_r(&now, &timeinfo);
	
	if (is_time_valid()) 
	{
		char recovery[50];
		strftime(recovery, sizeof(recovery), "%b %d %Y %H:%M:%S", &timeinfo);
		ESP_LOGI(TAG, "Time recovered: %s", recovery);
		time_state = TIME_RECOVERED;
	}

	setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4", 1);
	tzset();
	ESP_LOGI(TAG, "Actual time GMT+2:\n");

	localtime_r(&now, &timeinfo);
	strftime(time_now_str, sizeof(time_now_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
	ESP_LOGI(TAG, "Time is %s", time_now_str);
}

void time_extern_sync(time_t epoch)
{
	if (time_state >= TIME_EXTERN)
		return;
	struct timeval tv;
	tv.tv_sec = epoch;
	tv.tv_usec = 0;
	settimeofday(&tv, NULL);
	ESP_LOGI(TAG, "Time is synchronized from custom code");
	sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
	
	time(&now);
	localtime_r(&now, &timeinfo);
	strftime(time_now_str, sizeof(time_now_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
	ESP_LOGI(TAG, "Time Extern is %s", time_now_str);
	time_state = TIME_EXTERN;
}

static void obtain_time(void)
{
	if (time_state >= TIME_SYNCED)
		return;
	if (!is_wifi_connected())
	{
		ESP_LOGI(TAG, "Waiting for wifi to connect... ");
		return;
	}
	
	initialize_sntp();

	if (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) 
	{
		ESP_LOGI(TAG, "Waiting for system time to be set... ");
		return;
	}
	time(&now);
	localtime_r(&now, &timeinfo);
	strftime(time_now_str, sizeof(time_now_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
	ESP_LOGI(TAG, "Time SNTP is %s", time_now_str);
	time_state = TIME_SYNCED;
}

void time_update()
{
	obtain_time();
	time(&now);
	localtime_r(&now, &timeinfo);
	strftime(time_now_str, sizeof(time_now_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
}

uint8_t is_time_set()
{
	return time_state != TIME_INVALID;
}

uint64_t get_sys_time(SysTimeUnits_t units)
{
	struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000) / units;
}

void seconds_to_time(uint32_t seconds, char * time_out, size_t time_out_size)
{
	if (time_out == NULL)
		return;
	struct tm seconds_struct;
	localtime_r((time_t*)&seconds, &seconds_struct);
	strftime(time_out, time_out_size, "%Y-%m-%d %H:%M:%S", &seconds_struct);
}
*/