#include <stdio.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "mq2_module.h"

const static char *TAG = "MQ2";

// Configurare pentru senzorul digital (MQ2)
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
    ESP_LOGI(TAG, "MQ2 Digital Sensor configured on GPIO %d", MQ2_SENSOR_DIGITAL_GPIO);
}

// Configurare pentru senzorul analogic (MQ2)
static void mq2_analog_config()
{
    adc1_config_width(ADC_WIDTH_BIT_12); // Rezoluție de 12 biți
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // GPIO 34 = ADC1_CHANNEL_6
    ESP_LOGI(TAG, "MQ2 Analog Sensor configured on GPIO %d", MQ2_SENSOR_ANALOG_GPIO);
}

// Interpretarea valorilor analogice (scală)
static const char* interpret_gas_level(int analog_value)
{
    if (analog_value < 1000)
    {
        return "Safe - Low gas concentration.";
    }
    else if (analog_value < 2000)
    {
        return "Caution - Moderate gas concentration.";
    }
    else if (analog_value < 3000)
    {
        return "Warning - High gas concentration!";
    }
    else
    {
        return "Danger - Very high gas concentration! Immediate action required!";
    }
}

// Task pentru MQ2 (Digital și Analogic)
void mq2_task()
{
    mq2_digital_config();
    mq2_analog_config();

    int digital_state;
    int analog_value;

    while (1)
    {
        // Citire digitală
        digital_state = gpio_get_level(MQ2_SENSOR_DIGITAL_GPIO);
        if (digital_state == 0)
        {
            ESP_LOGI(TAG, "Gas detected (Digital)!");
        }
        else
        {
            ESP_LOGI(TAG, "No gas detected (Digital).");
        }

        // Citire analogică
        analog_value = adc1_get_raw(ADC1_CHANNEL_6);
        const char* gas_level_description = interpret_gas_level(analog_value);
        ESP_LOGI(TAG, "Analog gas level: %d - %s", analog_value, gas_level_description);

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay de 1 secundă
    }
}
