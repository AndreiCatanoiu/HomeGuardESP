#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "pir_module.h"

const static char *TAG = "PIR";

static void pir_config()
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
}
    
void pir_task()
{
    pir_config();
    int pir_state = gpio_get_level(PIR_SENSOR_GPIO);

    ESP_LOGI(TAG, "Motion detection initialized on GPIO %d", PIR_SENSOR_GPIO);
    while (1)
    {
        if (pir_state == 1)
        {
            ESP_LOGW(TAG, "Motion detected!");
        }
        else
        {
            ESP_LOGI(TAG, "No motion detected.");
        }
        
        pir_state = gpio_get_level(PIR_SENSOR_GPIO);
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
}