/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>

#include "cloud_api.h"
#include "commands.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mfc2160_app.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "wifi_nat.h"
#include "fota.h"
#include "mfc2160_app.h"
#include "rs232_api.h"

// #include "lwgsm/lwgsm.h"

static const char *TAG = "main-app";

char serial[32] = {0};
uint8_t chipId[6] = {0};

static uint8_t start_msg = 0;
static void wifi_nat_cb(wifi_nat_event_t *evt);
static void cloud_cb(cloud_event_t *evt);
static void connect_to_cloud(void);
static void remote_command_cloud_response_wrapper(const char *out, size_t len);
// static lwgsmr_t lwgsm_callback_func(lwgsm_evt_t *evt);
static void system_info_task(void *pdata);


static void system_info_task(void *pdata) {
    (void)pdata;
    TickType_t last_wake_time = xTaskGetTickCount();

    while (1) {
        last_wake_time = xTaskGetTickCount();
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(30000));
        uint32_t ram_free = esp_get_free_heap_size();
        uint32_t iram_free = heap_caps_get_free_size(MALLOC_CAP_32BIT);
        uint32_t uptime = esp_timer_get_time() / 1000000;

        ESP_LOGI(TAG, "Ram: %u \tRam32: %u \tUptime: %u \tseconds", ram_free,
                 iram_free, uptime);
    }

    vTaskDelete(NULL);
}

static void remote_command_cloud_response_wrapper(const char *out, size_t len) {
    cloud_api_send_rc_response((uint8_t *)out, len);
}

static void connect_to_cloud(void) {
    cloud_api_init(cloud_cb);
    cloud_api_set_mqtt_id(serial);

    cloud_ret_t cloud_ret = cloud_api_connect();

    if (cloud_ret == CLOUD_RET_OK) {
        ESP_LOGI(TAG, "Cloud Connected");
    } else if (cloud_ret == CLOUD_RET_UNKNOWN_PROTOCOL) {
        ESP_LOGE(TAG, "Cloud Unknown protocl");
    }
}

static void cloud_cb(cloud_event_t *evt) {
    switch (evt->id) {
        case CLOUD_EVT_CONNECT:
            ESP_LOGI(TAG, "CLOUD_EVT_CONNECT");
            if (!start_msg) {
                char msg[64] = {0};
                fota_init();
                const char *ver = fota_get_version();
                ESP_LOGI(TAG, "Current Version: %s", ver);
                size_t len = sprintf(msg, "+BOOT:v%s", ver);
                cloud_api_send_rc_response((uint8_t *)msg, len);

                start_msg = 1;
            }

            break;

        case CLOUD_EVT_DISCONNECT:
            ESP_LOGI(TAG, "CLOUD_EVT_DISCONNECT");
            break;

        case CLOUD_EVT_RECONNECT:
            ESP_LOGI(TAG, "CLOUD_EVT_RECONNECT");
            break;

        case CLOUD_EVT_DATA_MQTT:
            ESP_LOGI(TAG, "CLOUD_EVT_DATA_MQTT");
            ESP_LOGI(TAG, "Topic=%.*s", evt->evt.data.mqtt.topic_len,
                     evt->evt.data.mqtt.topic);
            esp_log_buffer_hex(TAG, evt->evt.data.mqtt.data,
                               evt->evt.data.mqtt.len);

            commands_process((char *)evt->evt.data.mqtt.data,
                             evt->evt.data.mqtt.len,
                             remote_command_cloud_response_wrapper);
            break;

        default:
            ESP_LOGI(TAG, "UNKOWN_CLOUD_EVENT %d", evt->id);
            break;
    }
}

static void wifi_nat_cb(wifi_nat_event_t *evt) {
    switch (evt->id) {
        case EVT_CONNECT:
            connect_to_cloud();
            break;

        case EVT_DISCONNECT:
            cloud_api_disconnect();
            cloud_api_deinit();
            break;

        default:
            break;
    }
}

#if 0
static lwgsmr_t lwgsm_callback_func(lwgsm_evt_t *evt) {
    switch (lwgsm_evt_type_t(evt)) {
        case LWGSM_EVT_INIT_FINISH:
            printf("GSM Library initialized!\r\n");
            break;

        default:
            break;
    }
    return lwgsmOK;
}
#endif

void app_main(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(nvs_flash_init_partition("cfg_part"));

    esp_efuse_mac_get_default(chipId);
    sprintf(serial, "%d%d%d%d%d%d", chipId[0], chipId[1], chipId[2], chipId[3],
            chipId[4], chipId[5]);

    printf("Serial ID: %s\r\n", serial);

    char ssid_name[23] = {0};
    sprintf(ssid_name, "ESP32_%.*s", 6, serial + 8);
    ESP_LOGI(TAG, "%s", ssid_name);

    initialize_wifi_nat(wifi_nat_cb, ssid_name);

#if 0
    lwgsm_init(lwgsm_callback_func, 1);

    if (lwgsm_device_is_present()) {
        lwgsm_device_get_serial_number(serial, sizeof(serial), NULL, NULL, 1);
    } else {
        char ssid_name[23] = {0};
        sprintf(ssid_name, "ESP32_%.*s", 6, serial + 8);
        ESP_LOGI(TAG, "%s", ssid_name);

        initialize_wifi_nat(wifi_nat_cb, ssid_name);
    }
#endif

    commands_init();

    // rs232_api_init();

    xTaskCreate(system_info_task, "system-info", 2048, NULL, 3, NULL);

    err = mfc2160_app_init();

    ESP_LOGI(TAG, "mfc-2160 %d", err);
 
}
