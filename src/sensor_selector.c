#include <stdio.h>
#include "pir_module.h"
#include "mq2_module.h"
#include "sensor_selector.h"
#include "mqtt_comm.h"

const char *TAG = "SENSOR_SELECTOR";

void sensor_selector_task(void *pvParameter)
{
    uint16_t sensor_id = *((uint16_t *)pvParameter);
    if (sensor_id % 2 == 1)
    {
        pir_config();
    }
    else
    {
        mq2_config();
    }

    while (1)
    {
        if (sensor_id % 2 == 1)
        {
            pir_process_data();
        }
        else
        {
            mq2_process_data();
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
