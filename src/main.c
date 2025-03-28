#include <stdio.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sensor_selector.h"
#include "wifi.h"
#include "mqtt_comm.h"
#include "settings.h"

void system_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret); 
    settings_init();
}

void app_main()
{
    system_init();
    xTaskCreate(&wifi_task, "wifi_task", 4096, NULL, 5, NULL);
    xTaskCreate(&sensor_selector_task, "sensor_selector_task", 4096, &(s_settings.sensor_id), 5, NULL);
    xTaskCreate(&is_device_available, "is_device_available", 4096, NULL, 5, NULL);

    sensor_status_t new_status = SENSOR_STATUS_MAINTENANCE;
    settings_set(KEY_SENSOR_STATUS, &new_status, sizeof(new_status), false);

}