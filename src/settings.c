#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_system.h"
#include "esp_log.h"
#include "settings.h"
#include "decoder.h"

static const char *TAG = "SETTINGS";
app_settings_t s_settings;

static sensor_status_t string_to_enum(const char *str) {
    if (strcmp(str, "UP") == 0) {
        return SENSOR_STATUS_UP;
    } else if (strcmp(str, "DOWN") == 0) {
        return SENSOR_STATUS_DOWN;
    } else if (strcmp(str, "MAINTENANCE") == 0) {
        return SENSOR_STATUS_MAINTENANCE;
    }
    return SENSOR_STATUS_DOWN; 
}

static void load_defaults(void)
{
    strncpy(s_settings.wifi_ssid, WIFI_SSID_DEFAULT, sizeof(s_settings.wifi_ssid));
    strncpy(s_settings.wifi_pass, WIFI_PASS_DEFAULT, sizeof(s_settings.wifi_pass));
    strncpy(s_settings.mqtt_server, MQTT_SERVER_DEFAULT, sizeof(s_settings.mqtt_server));
    s_settings.mqtt_port = MQTT_PORT_DEFAULT;
    strncpy(s_settings.mqtt_user, MQTT_USER_DEFAULT, sizeof(s_settings.mqtt_user));
    strncpy(s_settings.mqtt_pass, MQTT_PASS_DEFAULT, sizeof(s_settings.mqtt_pass));
    strncpy(s_settings.mqtt_topic, MQTT_TOPIC_DEFAULT, sizeof(s_settings.mqtt_topic));
    strncpy(s_settings.mqtt_up, MQTT_UP_DEFAULT, sizeof(s_settings.mqtt_up));
    strncpy(s_settings.mqtt_down, MQTT_DOWN_DEFAULT, sizeof(s_settings.mqtt_down));
    strncpy(s_settings.decoded_sensor_id, SENSOR_ID_DEFAULT, sizeof(s_settings.decoded_sensor_id));
    id_encoder_base64(s_settings.decoded_sensor_id, s_settings.encoded_sensor_id, sizeof(s_settings.encoded_sensor_id));
    s_settings.status = SENSOR_STATUS_DEFAULT;
}

void print_all_settings(void){
    ESP_LOGI(TAG, "Current settings:");
    ESP_LOGI(TAG, "WiFi SSID: %s", s_settings.wifi_ssid);
    ESP_LOGI(TAG, "WiFi PASS: %s", s_settings.wifi_pass);
    ESP_LOGI(TAG, "MQTT Server: %s", s_settings.mqtt_server);
    ESP_LOGI(TAG, "MQTT Port: %d", s_settings.mqtt_port);
    ESP_LOGI(TAG, "MQTT User: %s", s_settings.mqtt_user);
    ESP_LOGI(TAG, "MQTT Pass: %s", s_settings.mqtt_pass);
    ESP_LOGI(TAG, "MQTT Topic: %s", s_settings.mqtt_topic);
    ESP_LOGI(TAG, "MQTT Up: %s", s_settings.mqtt_up);
    ESP_LOGI(TAG, "MQTT Down: %s", s_settings.mqtt_down);
    ESP_LOGI(TAG, "Decoded sensor ID: %s", s_settings.decoded_sensor_id);
    ESP_LOGI(TAG, "Encoded sensor ID: %s", s_settings.encoded_sensor_id);
    const char *status_str = (s_settings.status == SENSOR_STATUS_UP) ? "UP" :
                              (s_settings.status == SENSOR_STATUS_DOWN) ? "DOWN" :
                              (s_settings.status == SENSOR_STATUS_MAINTENANCE) ? "MAINTENANCE" :
                              "UNKNOWN";
    ESP_LOGI(TAG, "Sensor STATUS: %s", status_str);
}

void settings_init(void)
{
    bool changes = false;
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle; using default values", esp_err_to_name(err));
        load_defaults();
        return;
    }

    size_t required_size = sizeof(s_settings.wifi_ssid);
    err = nvs_get_str(handle, WIFI_SSID, s_settings.wifi_ssid, &required_size);
    if (err != ESP_OK) 
    {
        ESP_LOGW(TAG, "Key %s not found; using default value", WIFI_SSID);
        strncpy(s_settings.wifi_ssid, WIFI_SSID_DEFAULT, sizeof(s_settings.wifi_ssid));
        changes = true;
    }

    required_size = sizeof(s_settings.wifi_pass);
    err = nvs_get_str(handle, WIFI_PASS, s_settings.wifi_pass, &required_size);
    if (err != ESP_OK) 
    {
        ESP_LOGW(TAG, "Key %s not found; using default value", WIFI_PASS);
        strncpy(s_settings.wifi_pass, WIFI_PASS_DEFAULT, sizeof(s_settings.wifi_pass));
        changes = true;
    }

    required_size = sizeof(s_settings.mqtt_server);
    err = nvs_get_str(handle, COMM_MQTT_SERVER, s_settings.mqtt_server, &required_size);
    if (err != ESP_OK) 
    {
        ESP_LOGW(TAG, "Key %s not found; using default value", COMM_MQTT_SERVER);
        strncpy(s_settings.mqtt_server, MQTT_SERVER_DEFAULT, sizeof(s_settings.mqtt_server));
        changes = true;
    }

    err = nvs_get_u16(handle, COMM_MQTT_PORT, &s_settings.mqtt_port);
    if (err != ESP_OK) 
    {
        ESP_LOGW(TAG, "Key %s not found; using default value", COMM_MQTT_PORT);
        s_settings.mqtt_port = MQTT_PORT_DEFAULT;
        changes = true;
    }

    required_size = sizeof(s_settings.mqtt_user);
    err = nvs_get_str(handle, COMM_MQTT_USER, s_settings.mqtt_user, &required_size);
    if (err != ESP_OK) 
    {
        ESP_LOGW(TAG, "Key %s not found; using default value", COMM_MQTT_USER);
        strncpy(s_settings.mqtt_user, MQTT_USER_DEFAULT, sizeof(s_settings.mqtt_user));
        changes = true;
    }

    required_size = sizeof(s_settings.mqtt_pass);
    err = nvs_get_str(handle, COMM_MQTT_PASS, s_settings.mqtt_pass, &required_size);
    if (err != ESP_OK) 
    {
        ESP_LOGW(TAG, "Key %s not found; using default value", COMM_MQTT_PASS);
        strncpy(s_settings.mqtt_pass, MQTT_PASS_DEFAULT, sizeof(s_settings.mqtt_pass));
        changes = true;
    }

    required_size = sizeof(s_settings.mqtt_topic);
    err = nvs_get_str(handle, COMM_MQTT_TOPIC, s_settings.mqtt_topic, &required_size);
    if (err != ESP_OK) 
    {
        ESP_LOGW(TAG, "Key %s not found; using default value", COMM_MQTT_TOPIC);
        strncpy(s_settings.mqtt_topic, MQTT_TOPIC_DEFAULT, sizeof(s_settings.mqtt_topic));
        changes = true;
    }

    required_size = sizeof(s_settings.mqtt_up);
    err = nvs_get_str(handle, COMM_MQTT_UP, s_settings.mqtt_up, &required_size);
    if (err != ESP_OK) 
    {
        ESP_LOGW(TAG, "Key %s not found; using default value", COMM_MQTT_UP);
        strncpy(s_settings.mqtt_up, MQTT_UP_DEFAULT, sizeof(s_settings.mqtt_up));
        changes = true;
    }

    required_size = sizeof(s_settings.mqtt_down);
    err = nvs_get_str(handle, COMM_MQTT_DOWN, s_settings.mqtt_down, &required_size);
    if (err != ESP_OK) 
    {
        ESP_LOGW(TAG, "Key %s not found; using default value", COMM_MQTT_DOWN);
        strncpy(s_settings.mqtt_down, MQTT_DOWN_DEFAULT, sizeof(s_settings.mqtt_down));
        changes = true;
    }

    err = nvs_get_u16(handle, KEY_SENSOR_ID, &s_settings.decoded_sensor_id);
    if (err != ESP_OK) 
    {
        ESP_LOGW(TAG, "Key %s not found; using default value", KEY_SENSOR_ID);
        strncpy(s_settings.decoded_sensor_id, SENSOR_ID_DEFAULT, sizeof(s_settings.decoded_sensor_id));
        id_encoder_base64(s_settings.decoded_sensor_id, s_settings.encoded_sensor_id, sizeof(s_settings.encoded_sensor_id));
        changes = true;
    }

    size_t status_size = sizeof(s_settings.status);
    err = nvs_get_u16(handle, KEY_SENSOR_STATUS, &s_settings.status);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Key %s not found; using default value", KEY_SENSOR_STATUS);
        s_settings.status = SENSOR_STATUS_DEFAULT;
        changes = true;
    }

    print_all_settings();
    nvs_close(handle);
    
    if (changes)
        settings_save();
}

void settings_save(void)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS for writing", esp_err_to_name(err));
        return;
    }

    err = nvs_set_str(handle, WIFI_SSID, s_settings.wifi_ssid);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving WiFi SSID");
    
    err = nvs_set_str(handle, WIFI_PASS, s_settings.wifi_pass);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving WiFi PASS");
    
    err = nvs_set_str(handle, COMM_MQTT_SERVER, s_settings.mqtt_server);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MQTT SERVER");
    
    err = nvs_set_u16(handle, COMM_MQTT_PORT, s_settings.mqtt_port);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MQTT PORT");
    
    err = nvs_set_str(handle, COMM_MQTT_USER, s_settings.mqtt_user);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MQTT USER");
    
    err = nvs_set_str(handle, COMM_MQTT_PASS, s_settings.mqtt_pass);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MQTT PASS");
    
    err = nvs_set_str(handle, COMM_MQTT_TOPIC, s_settings.mqtt_topic);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MQTT TOPIC");
    
    err = nvs_set_str(handle, COMM_MQTT_UP, s_settings.mqtt_up);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MQTT UP");

    err = nvs_set_str(handle, COMM_MQTT_DOWN, s_settings.mqtt_down);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MQTT DOWN");
    
    err = nvs_set_u16(handle, KEY_SENSOR_ID, s_settings.decoded_sensor_id);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SENSOR ID");
    
    err = nvs_set_u16(handle, KEY_SENSOR_STATUS, s_settings.status);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving sensor status");

    err = nvs_commit(handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error committing NVS");
    } else 
    {
        ESP_LOGI(TAG, "Settings saved to flash. Restarting ESP...");
    }
    nvs_close(handle);
    esp_restart();
}

esp_err_t settings_set(const char *key, void *value, size_t size, bool is_string)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS for writing", esp_err_to_name(err));
        return err;
    }

    if (is_string)
        err = nvs_set_str(handle, key, (char *)value);
    else
        err = nvs_set_blob(handle, key, value, size);

    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error (%s) setting key %s", esp_err_to_name(err), key);
        nvs_close(handle);
        return err;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error committing NVS");
        nvs_close(handle);
        return err;
    }
    nvs_close(handle);

    if (strcmp(key, WIFI_SSID) == 0) 
    {
        strncpy(s_settings.wifi_ssid, (char *)value, sizeof(s_settings.wifi_ssid));
    } 
    else if (strcmp(key, WIFI_PASS) == 0) 
    {
        strncpy(s_settings.wifi_pass, (char *)value, sizeof(s_settings.wifi_pass));
    } 
    else if (strcmp(key, COMM_MQTT_SERVER) == 0) 
    {
        strncpy(s_settings.mqtt_server, (char *)value, sizeof(s_settings.mqtt_server));
    } 
    else if (strcmp(key, COMM_MQTT_PORT) == 0) 
    {
        s_settings.mqtt_port = *(uint16_t *)value;
    } 
    else if (strcmp(key, COMM_MQTT_USER) == 0) 
    {
        strncpy(s_settings.mqtt_user, (char *)value, sizeof(s_settings.mqtt_user));
    } 
    else if (strcmp(key, COMM_MQTT_PASS) == 0) 
    {
        strncpy(s_settings.mqtt_pass, (char *)value, sizeof(s_settings.mqtt_pass));
    } 
    else if (strcmp(key, COMM_MQTT_TOPIC) == 0) 
    {
        strncpy(s_settings.mqtt_topic, (char *)value, sizeof(s_settings.mqtt_topic));
    } 
    else if (strcmp(key, COMM_MQTT_UP) == 0) 
    {
        strncpy(s_settings.mqtt_up, (char *)value, sizeof(s_settings.mqtt_up));
    } 
    else if (strcmp(key, COMM_MQTT_DOWN) == 0) 
    {
        strncpy(s_settings.mqtt_down, (char *)value, sizeof(s_settings.mqtt_down));
    } 
    else if (strcmp(key, KEY_SENSOR_ID) == 0) 
    {
        strncpy(s_settings.decoded_sensor_id, (char *)value, sizeof(s_settings.decoded_sensor_id));
        id_encoder_base64(s_settings.decoded_sensor_id, s_settings.encoded_sensor_id, sizeof(s_settings.encoded_sensor_id));
    }
    else if (strcmp(key, KEY_SENSOR_STATUS) == 0) 
    {
        if (is_string) 
        {
            s_settings.status = string_to_enum((char *)value);
        } 
        else 
        {
            s_settings.status = *((sensor_status_t *)value);
        }
    }
    
    ESP_LOGI(TAG, "Setting %s updated successfully", key);
    settings_save();

    return ESP_OK;
}
