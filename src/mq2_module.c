#include <stdio.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "mq2_module.h"

const static char *TAG = "MQ2";
const static char *MSG_SAFE = "Safe - Low gas concentration.";
const static char *MSG_CAUTION = "Caution - Moderate gas concentration.";
const static char *MSG_WARNING = "Warning - High gas concentration!";
const static char *MSG_DANGER = "Danger - Very high gas concentration! Immediate action required!";

static void mq2_digital_config()
{
    gpio_config_t io_conf = 
    {
        .pin_bit_mask = (1ULL << MQ2_SENSOR_DIGITAL_GPIO), 
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE 
    };

    gpio_config(&io_conf);
    ESP_LOGI(TAG, "Digital sensor configured on GPIO %d", MQ2_SENSOR_DIGITAL_GPIO);
}

static void mq2_analog_config()
{
    adc1_config_width(ADC_WIDTH_BIT_12); 
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); 
    ESP_LOGI(TAG, "Analog sensor configured on GPIO %d", MQ2_SENSOR_ANALOG_GPIO);
}

static void interpret_gas_level(int analog_value)
{
    
    if (analog_value < 1000)
    {
        ESP_LOGI(TAG, "Analog gas level: %d - %s", analog_value, MSG_SAFE);
    }
    else if (analog_value < 2000)
    {
        ESP_LOGI(TAG, "Analog gas level: %d - %s", analog_value, MSG_CAUTION);
    }
    else if (analog_value < 3000)
    {
        ESP_LOGW(TAG, "Analog gas level: %d - %s", analog_value, MSG_WARNING);
    }
    else
    {
        ESP_LOGE(TAG, "Analog gas level: %d - %s", analog_value, MSG_DANGER);
    }
}

void mq2_task()
{
    mq2_digital_config();
    mq2_analog_config();

    int digital_state;
    int analog_value;

    while (1)
    {
        digital_state = gpio_get_level(MQ2_SENSOR_DIGITAL_GPIO);
        if (digital_state == 0)
        {
            ESP_LOGI(TAG, "Gas detected (Digital)!");
        }
        else
        {
            ESP_LOGI(TAG, "No gas detected (Digital).");
        }

        analog_value = adc1_get_raw(ADC1_CHANNEL_6);
        interpret_gas_level(analog_value);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
