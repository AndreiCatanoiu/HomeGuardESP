#include <stdio.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mq2_module.h"
#include "pir_module.h"
#include "wifi.h"
#include "mqtt_comm.h"

void system_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret); 
}

void app_main()
{
    system_init();
    xTaskCreate(&wifi_task, "wifi_task", 4096, NULL, 5, NULL);
    while (!is_wifi_connected())
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    mqtt_app_send("Hello, World!", 13);
    
    bool use_mq2 = true;
    if (use_mq2)
    {
        xTaskCreate(&mq2_task, "mq2_task", 2048, NULL, 5, NULL);
    }
    else
    {
        xTaskCreate(&pir_task, "pir_task", 2048, NULL, 5, NULL);
    }
}