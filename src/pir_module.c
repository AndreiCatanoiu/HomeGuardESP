#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "pir_module.h"
#include "string.h" 
#include "mqtt_comm.h"

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
        char* message = "Motion detected!";
        ESP_LOGW(TAG,"%s",message);
        mqtt_app_send(message,strlen(message));
    }
    else
    {
        ESP_LOGI(TAG, "No motion detected.");
    }
}