#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

#include "lwip/apps/sntp.h"
#include "stdint.h"

typedef enum
{
	TIME_MICROSECONDS = 1,
	TIME_MILISECONDS  = 1000,
	TIME_SECONDS      = 1000000,
	TIME_MINUTES      = 60000000
}SysTimeUnits_t;

time_t now;
struct tm timeinfo;
char time_now_str[100];

void time_setup();
void time_extern_sync(time_t epoch);
void time_update();
uint8_t is_time_set();
void seconds_to_time(uint32_t seconds, char * time_out, size_t time_out_size);
uint64_t get_sys_time(SysTimeUnits_t units);

#endif