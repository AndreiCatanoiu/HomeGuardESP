#include "ota_updates.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "settings.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "cJSON.h"

static const char* TAG = "ota_updates";
static const char* s_manifest_url = "andreicatanoiu.ro/HomeGuard/firmware/latest.json";
static const char* s_current_version = NULL;

void ota_init() {
    s_current_version = s_settings.firmware_version;
    ESP_LOGI(TAG, "OTA init: manifest=%s current=%s",
             s_manifest_url, s_current_version);
}

bool ota_check_and_perform(void) {
    if (!s_manifest_url || !s_current_version) {
        ESP_LOGE(TAG, "OTA not initialized!");
        return false;
    }

    esp_http_client_config_t http_cfg = {
        .url            = s_manifest_url,
        .timeout_ms     = 5000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&http_cfg);
    if (esp_http_client_perform(client) != ESP_OK) {
        int err = esp_http_client_get_errno(client);
        ESP_LOGW(TAG, "Manifest fetch failed: errno=%d", err);
        esp_http_client_cleanup(client);
        return false;
    }
    int len = esp_http_client_get_content_length(client);
    if (len <= 0) {
        ESP_LOGW(TAG, "Empty manifest (length=%d)", len);
        esp_http_client_cleanup(client);
        return false;
    }
    char *buf = malloc(len + 1);
    int r = esp_http_client_read(client, buf, len);
    buf[r < len ? r : len] = '\0';
    esp_http_client_cleanup(client);

    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (!root) {
        ESP_LOGW(TAG, "JSON parse error");
        return false;
    }
    cJSON *ver = cJSON_GetObjectItemCaseSensitive(root, "version");
    cJSON *url = cJSON_GetObjectItemCaseSensitive(root, "url");
    if (!cJSON_IsString(ver) || !cJSON_IsString(url)) {
        ESP_LOGW(TAG, "Missing fields in manifest");
        cJSON_Delete(root);
        return false;
    }

    if (strcmp(ver->valuestring, s_current_version) <= 0) {
        ESP_LOGI(TAG, "Firmware up to date (%s)", s_current_version);
        cJSON_Delete(root);
        return false;
    }
    ESP_LOGI(TAG, "Update available: %s -> %s",
             s_current_version, ver->valuestring);

    esp_http_client_config_t ota_http_cfg = {
        .url             = url->valuestring,
        .timeout_ms      = 60000,
        .transport_type  = HTTP_TRANSPORT_OVER_TCP,  
    };
    esp_https_ota_config_t ota_cfg = {
        .http_config            = &ota_http_cfg,
        .max_http_request_size  = 0,   
        .bulk_flash_erase       = false,
        .partial_http_download  = false,
        .http_client_init_cb    = NULL, 
    };

    esp_err_t ota_err = esp_https_ota(&ota_cfg);
    cJSON_Delete(root);

    if (ota_err == ESP_OK) {
        ESP_LOGI(TAG, "OTA ok, rebooting...");
        esp_restart();
        return true;
    } else {
        ESP_LOGE(TAG, "OTA failed: %s", esp_err_to_name(ota_err));
        return false;
    }
}
