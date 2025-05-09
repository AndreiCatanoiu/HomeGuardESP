#include "ota_updates.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "settings.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "cJSON.h"

static const char* TAG = "OTA";
static const char* s_manifest_url = "https://andreicatanoiu.ro/HomeGuard/firmware/latest.json";
static const char* s_current_version = NULL;
char ver_copy[10];

extern const uint8_t server_cert_pem_start[] asm("_binary_fullchain_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_fullchain_pem_end");

void ota_init() 
{
    s_current_version = s_settings.firmware_version;
    ESP_LOGI(TAG, "OTA init: manifest=%s current=%s",
            s_manifest_url, s_current_version);
}

static char* download_json(const char* url) 
{
    ESP_LOGI(TAG, "Downloading JSON from %s", url);
    
    esp_http_client_config_t config = 
    {
		.url = (url != NULL ? url : s_manifest_url),
        .timeout_ms = 5000,
        .keep_alive_enable = true,
		.cert_pem = (char *)server_cert_pem_start,
        .skip_cert_common_name_check = true,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return NULL;
    }
    
    int content_length = esp_http_client_fetch_headers(client);
    int status_code = esp_http_client_get_status_code(client);
    
    ESP_LOGI(TAG, "HTTP Status = %d, content_length = %d", status_code, content_length);
    
    if (status_code != 200) 
    {
        ESP_LOGE(TAG, "HTTP request failed with status code %d", status_code);
        esp_http_client_cleanup(client);
        return NULL;
    }
    
    char *buffer = NULL;
    int total_read = 0;
    
    if (content_length > 0) 
    {
        buffer = malloc(content_length + 1);
        if (!buffer) 
        {
            ESP_LOGE(TAG, "Failed to allocate memory for response buffer");
            esp_http_client_cleanup(client);
            return NULL;
        }
        
        int read_len;
        while (total_read < content_length) 
        {
            read_len = esp_http_client_read(client, buffer + total_read, content_length - total_read);
            if (read_len <= 0) 
            {
                ESP_LOGW(TAG, "Error reading data, read_len=%d", read_len);
                break;
            }
            total_read += read_len;
            ESP_LOGI(TAG, "Read %d bytes, total now: %d/%d", read_len, total_read, content_length);
        }
        buffer[total_read] = '\0';
    } 
    else 
    {
        const int chunk_size = 512;
        buffer = malloc(chunk_size);
        if (!buffer) 
        {
            ESP_LOGE(TAG, "Failed to allocate memory for response buffer");
            esp_http_client_cleanup(client);
            return NULL;
        }
        
        int buffer_size = chunk_size;
        int read_len;
        
        while (1) 
        {
            if (total_read + chunk_size > buffer_size) 
            {
                buffer_size += chunk_size;
                buffer = realloc(buffer, buffer_size);
                if (!buffer) 
                {
                    ESP_LOGE(TAG, "Failed to reallocate memory");
                    esp_http_client_cleanup(client);
                    return NULL;
                }
            }
            
            read_len = esp_http_client_read(client, buffer + total_read, chunk_size - 1);
            if (read_len <= 0) 
            {
                break;
            }
            
            total_read += read_len;
        }
        
        if (total_read > 0) 
        {
            buffer = realloc(buffer, total_read + 1);
            buffer[total_read] = '\0';
        } 
        else 
        {
            free(buffer);
            buffer = NULL;
        }
    }
    
    esp_http_client_cleanup(client);
    
    if (total_read == 0) 
    {
        ESP_LOGE(TAG, "No data read from server");
        free(buffer);
        return NULL;
    }
    
    ESP_LOGI(TAG, "Successfully read %d bytes: %s", total_read, buffer);
    return buffer;
}

bool ota_check_and_perform(void) 
{
    if (!s_manifest_url || !s_current_version) 
    {
        ESP_LOGE(TAG, "OTA not initialized!");
        return false;
    }
    
    char *json_data = download_json(s_manifest_url);
    if (!json_data) 
    {
        ESP_LOGE(TAG, "Failed to download manifest");
        return false;
    }
    
    cJSON *root = cJSON_Parse(json_data);
    free(json_data);  
    
    if (!root) 
    {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) 
        {
            ESP_LOGE(TAG, "JSON parse error near: %.20s", error_ptr);
        } 
        else 
        {
            ESP_LOGE(TAG, "JSON parse error");
        }
        return false;
    }
    
    cJSON *ver_item = cJSON_GetObjectItemCaseSensitive(root, "version");
    cJSON *url_item = cJSON_GetObjectItemCaseSensitive(root, "url");
    
    if (!cJSON_IsString(ver_item) || !cJSON_IsString(url_item)) 
    {
        ESP_LOGE(TAG, "Invalid manifest format: version or url missing");
        cJSON_Delete(root);
        return false;
    }
    
    const char *new_version = ver_item->valuestring;
    const char *update_url_original = url_item->valuestring;
    
    char *http_url = NULL;
    const char *update_url = update_url_original;
    
    if (strncmp(update_url_original, "https://", 8) == 0) 
    {
        http_url = malloc(strlen(update_url_original) + 1);
        if (http_url) 
        {
            strcpy(http_url, "http://");
            strcat(http_url, update_url_original + 8);
            update_url = http_url;
            ESP_LOGI(TAG, "Converted HTTPS URL to HTTP: %s", update_url);
        }
    }
    
    ESP_LOGI(TAG, "Current version: %s, Available version: %s", s_current_version, new_version);
    strcpy(ver_copy, new_version);
    
    if (strcmp(new_version, s_current_version) <= 0) 
    {
        ESP_LOGI(TAG, "Firmware is up to date");
        cJSON_Delete(root);
        if (http_url) free(http_url);
        return false;
    }
    
    ESP_LOGI(TAG, "Update available: %s -> %s", s_current_version, new_version);
    ESP_LOGI(TAG, "Update URL: %s", update_url);
    
    esp_http_client_config_t ota_config = 
    {
        .url = update_url,
        .timeout_ms = 60000,
        .keep_alive_enable = true,
        .cert_pem = (char *)server_cert_pem_start,
        .skip_cert_common_name_check = false,
    };
    
    esp_https_ota_config_t ota_cfg = 
    {
        .http_config = &ota_config,
    };
    
    ESP_LOGI(TAG, "Starting OTA update...");
    esp_err_t err = esp_https_ota(&ota_cfg);
    
    cJSON_Delete(root);
    if (http_url) free(http_url);
    
    if (err == ESP_OK) 
    {
        ESP_LOGI(TAG, "OTA update successful! Restarting...");
        settings_set(KEY_SENSOR_FIRMWARE_VERSION, ver_copy, strlen(ver_copy) + 1 , true);
        return true;
    } 
    else 
    {
        ESP_LOGE(TAG, "OTA update failed: %s", esp_err_to_name(err));
        return false;
    }
}