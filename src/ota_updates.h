#ifndef OTA_UPDATES_H
#define OTA_UPDATES_H

#include <stdbool.h>

void ota_init();
bool ota_check_and_perform(void);

#endif