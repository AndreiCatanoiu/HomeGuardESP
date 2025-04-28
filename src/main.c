#include <stdio.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sensor_selector.h"
#include "esp_wifi.h"
#include "wifi.h"
#include "mqtt_comm.h"
#include "settings.h"
#include "decoder.h"
#include "ota_updates.h"

void system_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret); 

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    settings_init();
    decoder_init();
}

void app_main()
{
    system_init();
    
    xTaskCreate(&command_task, "command_task", 4096, NULL, 5, NULL);
    xTaskCreate(&wifi_task, "wifi_task", 4096, NULL, 5, NULL);
    while (!is_wifi_connected()) 
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    ota_init();
    ota_check_and_perform();

    xTaskCreate(&sensor_selector_task, "sensor_selector_task", 4096, &(s_settings.decoded_sensor_id), 5, NULL);
    xTaskCreate(&is_device_available, "is_device_available", 4096, NULL, 5, NULL);
}