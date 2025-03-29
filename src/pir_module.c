#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "pir_module.h"
#include "string.h" 
#include "mqtt_comm.h"
#include "system_time.h"
#include "settings.h"

const static char *TAG = "PIR";

void pir_config()
{    
    gpio_config_t io_conf = 
    {
        .pin_bit_mask = (1ULL << PIR_SENSOR_GPIO), 
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE 
    };
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "Motion detection initialized on GPIO %d", PIR_SENSOR_GPIO);
}
    
void pir_process_data()
{
    int pir_state = gpio_get_level(PIR_SENSOR_GPIO);
    
    if (pir_state == 1)
    {
        char* timestr = actual_time_string();
        char msg_motion[200] ;
        sprintf(msg_motion, "Motion detected! \n%s", timestr);
        ESP_LOGE(TAG, "%s", msg_motion);
        mqtt_app_send(msg_motion,strlen(msg_motion),"alerts");
    }
    else
    {
        ESP_LOGI(TAG, "No motion detected.");
    }
}