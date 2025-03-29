#include "decoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_comm.h"
#include "settings.h"

static const char *TAG = "DECODER";

Cmd_t *exec = NULL;
int exec_count = 0;
Cmd_t *query = NULL;
int query_count = 0;
void decode_settings(char *args)
{
    char *group = strtok(args, " ");
    char *tag   = strtok(NULL, " ");
    char *value = strtok(NULL, "");
    
    if (!group || !tag || !value) {
        ESP_LOGW(TAG,"Usage: settings [group] [tag] [value]\n");
        return;
    }
    
    if (strcmp(group, "wifi") == 0) {
        if (strcmp(tag, "ssid") == 0) {
            settings_set(WIFI_SSID, value, strlen(value), true);
            ESP_LOGI(TAG,"WiFi SSID set to: %s\n", s_settings.wifi_ssid);
        } else if (strcmp(tag, "pass") == 0) {
            settings_set(WIFI_PASS, value, strlen(value), true);
            ESP_LOGI(TAG,"WiFi PASS set to: %s\n", s_settings.wifi_pass);
        } else {
            ESP_LOGE(TAG,"Unknown WiFi tag: %s\n", tag);
        }
    }
    else if (strcmp(group, "mqtt") == 0) {
        if (strcmp(tag, "server") == 0) {
            settings_set(COMM_MQTT_SERVER, value, strlen(value), true);
            ESP_LOGI(TAG,"MQTT Server set to: %s\n", s_settings.mqtt_server);
        } else if (strcmp(tag, "port") == 0) {
            uint16_t port = (uint16_t)atoi(value);
            settings_set(COMM_MQTT_PORT, &port, sizeof(port), false);
            ESP_LOGI(TAG,"MQTT Port set to: %d\n", s_settings.mqtt_port);
        } else if (strcmp(tag, "user") == 0) {
            settings_set(COMM_MQTT_USER, value, strlen(value), true);
            ESP_LOGI(TAG,"MQTT User set to: %s\n", s_settings.mqtt_user);
        } else if (strcmp(tag, "pass") == 0) {
            settings_set(COMM_MQTT_PASS, value, strlen(value), true);
            ESP_LOGI(TAG,"MQTT Pass set to: %s\n", s_settings.mqtt_pass);
        } else if (strcmp(tag, "topic") == 0) {
            settings_set(COMM_MQTT_TOPIC, value, strlen(value), true);
            ESP_LOGI(TAG,"MQTT Topic set to: %s\n", s_settings.mqtt_topic);
        } else if (strcmp(tag, "up") == 0) {
            settings_set(COMM_MQTT_UP, value, strlen(value), true);
            ESP_LOGI(TAG,"MQTT Up set to: %s\n", s_settings.mqtt_up);
        } else if (strcmp(tag, "down") == 0) {
            settings_set(COMM_MQTT_DOWN, value, strlen(value), true);
            ESP_LOGI(TAG,"MQTT Down set to: %s\n", s_settings.mqtt_down);
        } else {
            ESP_LOGE(TAG,"Unknown MQTT tag: %s\n", tag);
        }
    }
    else if (strcmp(group, "sensor") == 0) {
        if (strcmp(tag, "id") == 0) {
            uint16_t id = (uint16_t)atoi(value);
            settings_set(KEY_SENSOR_ID, &id, sizeof(id), false);
            ESP_LOGI(TAG,"Sensor ID set to: %d\n", s_settings.sensor_id);
        } else if (strcmp(tag, "status") == 0) {
            if (isdigit((unsigned char)value[0])) {
                uint16_t status_val = (uint16_t)atoi(value);
                settings_set(KEY_SENSOR_STATUS, &status_val, sizeof(status_val), false);
            } else {
                settings_set(KEY_SENSOR_STATUS, value, strlen(value), true);
            }
            const char *status_str = (s_settings.status == SENSOR_STATUS_UP) ? "UP" :
                                      (s_settings.status == SENSOR_STATUS_DOWN) ? "DOWN" :
                                      (s_settings.status == SENSOR_STATUS_MAINTENANCE) ? "MAINTENANCE" : "UNKNOWN";
            ESP_LOGI(TAG,"Sensor Status set to: %s\n", status_str);
        } else {
            ESP_LOGE(TAG,"Unknown sensor tag: %s\n", tag);
        }
    }
    else {
        ESP_LOGE(TAG,"Unknown group: %s\n", group);
    }
}

void query_settings(char *args)
{
    char *group = strtok(args, " ");
    char *tag   = strtok(NULL, " ");
    
    char query_mqtt_msg[200] ;

    if (!group || !tag) {
        ESP_LOGW(TAG,"Usage: settings [group] [tag]\n");
        return;
    }
    
    if (strcmp(group, "wifi") == 0) {
        if (strcmp(tag, "ssid") == 0) {
            sprintf(query_mqtt_msg, "WiFi SSID: %s\n", s_settings.wifi_ssid);
            ESP_LOGI(TAG,"%s", query_mqtt_msg);
        } else if (strcmp(tag, "pass") == 0) {
            sprintf(query_mqtt_msg, "WiFi PASS: %s\n", s_settings.wifi_pass);
            ESP_LOGI(TAG,"%s", query_mqtt_msg);
        } else {
            sprintf(query_mqtt_msg, "Unknown WiFi tag: %s\n", tag);
            ESP_LOGE(TAG,"%s", query_mqtt_msg);
        }
    }
    else if (strcmp(group, "mqtt") == 0) {
        if (strcmp(tag, "server") == 0) {
            sprintf(query_mqtt_msg, "MQTT Server: %s\n", s_settings.mqtt_server);
            ESP_LOGI(TAG,"%s", query_mqtt_msg);
        } else if (strcmp(tag, "port") == 0) {
            sprintf(query_mqtt_msg, "MQTT Port: %d\n", s_settings.mqtt_port);
            ESP_LOGI(TAG,"%s", query_mqtt_msg);
        } else if (strcmp(tag, "user") == 0) {
            sprintf(query_mqtt_msg, "MQTT User: %s\n", s_settings.mqtt_user);
            ESP_LOGI(TAG,"%s", query_mqtt_msg);
        } else if (strcmp(tag, "pass") == 0) {
            sprintf(query_mqtt_msg, "MQTT Pass: %s\n", s_settings.mqtt_pass);
            ESP_LOGI(TAG,"%s", query_mqtt_msg);
        } else if (strcmp(tag, "topic") == 0) {
            sprintf(query_mqtt_msg, "MQTT Topic: %s\n", s_settings.mqtt_topic);
            ESP_LOGI(TAG,"%s", query_mqtt_msg);
        } else if (strcmp(tag, "up") == 0) {
            sprintf(query_mqtt_msg, "MQTT Up: %s\n", s_settings.mqtt_up);
            ESP_LOGI(TAG,"%s", query_mqtt_msg);
        } else if (strcmp(tag, "down") == 0) {
            sprintf(query_mqtt_msg, "MQTT Down: %s\n", s_settings.mqtt_down);
            ESP_LOGI(TAG,"%s", query_mqtt_msg);
        } else {
            sprintf(query_mqtt_msg,"Unknown MQTT tag: %s\n", tag);
            ESP_LOGI(TAG,"%s", query_mqtt_msg);
        }
    }
    else if (strcmp(group, "sensor") == 0) {
        if (strcmp(tag, "id") == 0) {
            sprintf(query_mqtt_msg, "Sensor ID: %d\n", s_settings.sensor_id);
            ESP_LOGI(TAG,"%s", query_mqtt_msg);
        } else if (strcmp(tag, "status") == 0) {
            const char *status_str = (s_settings.status == SENSOR_STATUS_UP) ? "UP" :
                                      (s_settings.status == SENSOR_STATUS_DOWN) ? "DOWN" :
                                      (s_settings.status == SENSOR_STATUS_MAINTENANCE) ? "MAINTENANCE" : "UNKNOWN";
            sprintf(query_mqtt_msg, "Sensor Status: %s\n", status_str);
            ESP_LOGI(TAG,"%s", query_mqtt_msg);
        } else {
            sprintf(query_mqtt_msg,"Unknown sensor tag: %s\n", tag);
            ESP_LOGI(TAG,"%s", query_mqtt_msg);
        }
    }
    else {
        sprintf(query_mqtt_msg, "Unknown group: %s\n", group);
        ESP_LOGI(TAG,"%s", query_mqtt_msg);
    }

    mqtt_app_send(query_mqtt_msg,strlen(query_mqtt_msg),"query");
}

void decoder_process_command(const char *cmd)
{
    if (!cmd || strlen(cmd) == 0) {
        return;
    }

    char buffer[256];
    strncpy(buffer, cmd, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    if (buffer[0] == '#')
    {
        char *command = buffer + 1;
        char *cmd_name = strtok(command, " ");
        char *args = strtok(NULL, "");
        if (!cmd_name) {
            ESP_LOGW(TAG,"Invalid exec command.\n");
            return;
        }
        for (int i = 0; i < exec_count; i++) {
            if (strcmp(exec[i].name, cmd_name) == 0) {
                exec[i].function(args);
                return;
            }
        }
        ESP_LOGE(TAG,"Unknown exec command: %s\n", cmd_name);
    }
    else if (buffer[0] == '?') {
        char *command = buffer + 1;
        char *cmd_name = strtok(command, " ");
        char *args = strtok(NULL, "");
        if (!cmd_name) {
            ESP_LOGE(TAG,"Invalid query command.\n");
            return;
        }
        for (int i = 0; i < query_count; i++) {
            if (strcmp(query[i].name, cmd_name) == 0) {
                query[i].function(args);
                return;
            }
        }
        ESP_LOGE(TAG,"Unknown query command: %s\n", cmd_name);
    }
    else {
        ESP_LOGE(TAG,"Unknown command format: %s\n", buffer);
    }
}
void decoder_init(void)
{
    static Cmd_t exec_commands[] = {
        { 
            .name = "settings", 
            .params = "[group] [tag] [value]",
            .info = "Set a new value. Available groups: wifi (ssid, pass), mqtt (server, port, user, pass, topic, up, down), sensor (id, status)",
            .function = decode_settings 
        },
    };
    exec = exec_commands;
    exec_count = sizeof(exec_commands) / sizeof(exec_commands[0]);

    static Cmd_t query_commands[] = {
        { 
            .name = "settings", 
            .params = "[group] [tag]",
            .info = "Display the current value. Available groups: wifi (ssid, pass), mqtt (server, port, user, pass, topic, up, down), sensor (id, status)",
            .function = query_settings 
        },
    };
    query = query_commands;
    query_count = sizeof(query_commands) / sizeof(query_commands[0]);
}

void command_task(void *pvParameters)
{
    char lineBuffer[256];
    int idx = 0;

    memset(lineBuffer, 0, sizeof(lineBuffer)); 

    while (1)
    {
        int ch = getchar(); 

        if (ch != EOF) 
        {
            fflush(stdout);

            if (ch == '\n' || ch == '\r')
            {
                if (idx > 0)
                {
                    lineBuffer[idx] = '\0';
                    printf("%s",lineBuffer);
                    decoder_process_command(lineBuffer); 
                    idx = 0; 
                }
            }
            else if (idx < sizeof(lineBuffer) - 1)
            {
                lineBuffer[idx++] = ch;
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}