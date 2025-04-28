#include <stdio.h>
#include <string.h>
#include "pir_module.h"
#include "mq2_module.h"
#include "sensor_selector.h"
#include "mqtt_comm.h"
#include "settings.h"
#include "esp_log.h"
#include "decoder.h"

const char *TAG = "SENSOR_SELECTOR";

void sensor_selector_task(void *pvParameter)
{
    const char *sensor_id = (const char *)pvParameter;
    if (strstr(sensor_id, "PIR") != NULL)
    {
        pir_config();
    }
    else if (strstr(sensor_id, "Gaz") != NULL)
    {
        mq2_config();
    }
    else
    {
        ESP_LOGW(TAG, "Unknown sensor type in sensor id: %s", sensor_id);
    }

    while (1)
    {
        if (s_settings.status == SENSOR_STATUS_UP)
        {
            if (strstr(sensor_id, "PIR") != NULL)
            {
                pir_process_data();
            }
            else if (strstr(sensor_id, "Gaz") != NULL)
            {
                mq2_process_data();
            }
            else
            {
                ESP_LOGW(TAG, "Unknown sensor type in sensor id: %s", sensor_id);
            }
        }
        else if (s_settings.status == SENSOR_STATUS_DOWN)
        {
            ESP_LOGE(TAG, "The sensor is down");
        }
        else if (s_settings.status == SENSOR_STATUS_MAINTENANCE)
        {
            ESP_LOGW(TAG, "The sensor is in maintenance");
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
