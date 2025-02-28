#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stddef.h>

void time_setup(void);
void time_update(void);
uint8_t is_time_set(void);
uint64_t get_sys_time(uint8_t units);
void seconds_to_time(uint32_t seconds, char *time_out, size_t time_out_size);
char* actual_time_string(void);

#endif 