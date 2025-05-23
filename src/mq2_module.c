#include <stdio.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "mq2_module.h"
#include "string.h"
#include "mqtt_comm.h"
#include "system_time.h"
#include "settings.h"

const static char *TAG = "MQ2";

static uint16_t SAFE_LEVEL = 1000;
static uint16_t CAUTION_LEVEL = 2000;
static uint16_t WARNING_LEVEL = 3000;

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

void mq2_config()
{
    mq2_digital_config();
    mq2_analog_config();
}

static void interpret_gas_level(int analog_value)
{
    if (analog_value < SAFE_LEVEL)
    {
        ESP_LOGI(TAG, "Analog gas level: %d - %s", analog_value, MSG_SAFE);
    }
    else if (analog_value < CAUTION_LEVEL)
    {
        ESP_LOGI(TAG, "Analog gas level: %d - %s", analog_value, MSG_CAUTION);
    }
    else if (analog_value < WARNING_LEVEL)
    {
        char* timestr = actual_time_string();
        char msg_warning[200] ;
        sprintf(msg_warning, "Analog gas level: %d - %s \n%s", analog_value, MSG_WARNING, timestr);
        ESP_LOGW(TAG, "%s", msg_warning);
        sprintf(msg_warning, "%s", MSG_WARNING);
        mqtt_app_send(msg_warning,strlen(msg_warning),"alerts");
    }
    else
    {
        char* timestr = actual_time_string();
        char msg_danger[200] ;
        sprintf(msg_danger, "Analog gas level: %d - %s \n%s", analog_value, MSG_DANGER, timestr);
        ESP_LOGE(TAG, "%s", msg_danger);
        sprintf(msg_danger, "%s", MSG_DANGER);
        mqtt_app_send(msg_danger,strlen(msg_danger),"alerts");
    }
}

void mq2_process_data()
{
    int digital_state;
    int analog_value;

    digital_state = gpio_get_level(MQ2_SENSOR_DIGITAL_GPIO);
    if (digital_state == 0)
    {
        ESP_LOGW(TAG, "Gas detected!");
    }
    else
    {
        ESP_LOGI(TAG, "No gas detected.");
    }

    analog_value = adc1_get_raw(ADC1_CHANNEL_6);
    interpret_gas_level(analog_value);
}