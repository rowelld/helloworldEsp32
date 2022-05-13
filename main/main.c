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
#include "esp_sntp.h"
#include "esp_system.h"
#include "fota.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "mfc2160_app.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "rs232_api.h"
#include "wifi_nat.h"

// #include "lwgsm/lwgsm.h"

#include "gsm/gsm_device_info.h"
#include "gsm/gsm_modem_mode.h"
#include "gsm/gsm_network_status.h"
#include "gsm/gsm_ppp_api.h"
#include "gsm/gsm_qctl_sms.h"
#include "gsm/gsm_urc.h"
#include "network_utils.h"

static const char *TAG = "main-app";

char serial[32] = {0};
uint8_t chipId[6] = {0};

static SemaphoreHandle_t connectivity_sync;
static gsm_sms_entry_t sms_entry;
static size_t sms_entry_cnt = 0;
static uint8_t sms_pos = 0;
static uint8_t start_msg = 0;

static void wifi_nat_cb(wifi_nat_event_t *evt);
static void cloud_cb(cloud_event_t *evt);
static void connect_to_cloud(void);
static void remote_command_cloud_response_wrapper(const char *out, size_t len);
static gsmr_t gsm_callback_func(gsm_evt_t *evt);
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
    if (len > 1) cloud_api_send_rc_response((uint8_t *)out, len);
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

static gsmr_t gsm_callback_func(gsm_evt_t *evt) {
    switch (gsm_evt_get_type(evt)) {
        case GSM_EVT_INIT_FINISH:
            printf("Library initialized!\r\n");
            break;

        case GSM_EVT_NETWORK_REG_CHANGED:
            network_utils_process_reg_change(evt);
            if (gsm_network_get_reg_status() ==
                GSM_NETWORK_REG_STATUS_CONNECTED) {
                xSemaphoreGive(connectivity_sync);
            }
            break;

        /* Process current network operator */
        case GSM_EVT_NETWORK_OPERATOR_CURRENT:
            network_utils_process_curr_operator(evt);
            break;
        /* Process signal strength */
        case GSM_EVT_SIGNAL_STRENGTH:
            network_utils_process_rssi(evt);
            break;
        case GSM_EVT_RESET:
            printf("GSM RESET DONE\r\n");
            break;
        case GSM_EVT_SIM_STATE_CHANGED:
            printf("Sim state changed %d\r\n", evt->evt.cpin.state);
            break;
        case GSM_EVT_DATA_MODE:
            printf("GSM in data mode\r\n");
            break;
        case GSM_EVT_COMMAND_MODE:
            printf("GSM in command mode\r\n");
            break;
        case GSM_EVT_UNKNOWN_REPLY:
            printf("GSM UKNOWN REPLY\r\n");
            // if (strstr(evt->evt.unkown_rsp.msg, "RDY") ||
            // strstr(evt->evt.unkown_rsp.msg, "READY")) {
            // xSemaphoreGive(connectivity_sync);
            // }
            break;
        case GSM_EVT_PPP_RECV:
            // pppos_app_receive((uint8_t*)evt->evt.ppp_rcv.data,
            // evt->evt.ppp_rcv.len);
            break;
        /* Other user events here... */
        case GSM_EVT_SMS_READY: { /* SMS is ready notification from device */
            printf("SIM device SMS service is ready!\r\n");
            xSemaphoreGive(connectivity_sync);
            break;
        }
        case GSM_EVT_SMS_RECV: { /* New SMS received indicator */
            gsmr_t res;

            printf("New SMS received!\r\n"); /* Notify user */

            /* Try to read SMS */
            res = gsm_sms_read(gsm_evt_sms_recv_get_mem(evt),
                               gsm_evt_sms_recv_get_pos(evt), &sms_entry, 1,
                               NULL, NULL, 0);
            if (res == gsmOK) {
                printf("SMS read in progress!\r\n");
            } else {
                printf("Cannot start SMS read procedure!\r\n");
            }
            break;
        }
        case GSM_EVT_SMS_READ: { /* SMS read event */
            gsm_sms_entry_t *entry = gsm_evt_sms_read_get_entry(evt);
            if (gsm_evt_sms_read_get_result(evt) == gsmOK && entry != NULL) {
                /* Print SMS data */
                printf("SMS read. From: %s, content: %s\r\n", entry->number,
                       entry->data);

                /* Try to send SMS back */
                if (gsm_sms_send(entry->number, entry->data, NULL, NULL, 0) ==
                    gsmOK) {
                    printf("SMS send in progress!\r\n");
                } else {
                    printf("Cannot start SMS send procedure!\r\n");
                }

                /* Delete SMS from device memory */
                gsm_sms_delete(entry->mem, entry->pos, NULL, NULL, 0);
            }
            break;
        }
        case GSM_EVT_SMS_SEND: { /* SMS send event */
            if (gsm_evt_sms_send_get_result(evt) == gsmOK) {
                printf("SMS has been successfully sent!\r\n");
            } else {
                printf("SMS has not been sent successfully!\r\n");
            }
            break;
        }
        case GSM_EVT_SMS_DELETE: {
            if (gsm_evt_sms_delete_get_result(evt) == gsmOK) {
                printf("SMS deleted, memory position: %d\r\n",
                       (int)gsm_evt_sms_delete_get_pos(evt));
            } else {
                printf("SMS delete operation failed!\r\n");
            }
            break;
        }
        default:
            break;
    }
    return gsmOK;
}

#if 0
static lwgsmr_t lwgsm_callback_func(lwgsm_evt_t *evt) {
    switch (evt->type) {
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

    connectivity_sync = xSemaphoreCreateBinary();

#if 1
    esp_efuse_mac_get_default(chipId);
    sprintf(serial, "%d%d%d%d%d%d", chipId[0], chipId[1], chipId[2], chipId[3],
            chipId[4], chipId[5]);

    printf("Serial ID: %s\r\n", serial);

    char ssid_name[23] = {0};
    sprintf(ssid_name, "ESP32_%.*s", 6, serial + 8);
    ESP_LOGI(TAG, "%s", ssid_name);

    initialize_wifi_nat(wifi_nat_cb, ssid_name);
#endif

    commands_init();

#if 0

    // rs232_api_init();

    if (gsm_init(gsm_callback_func, 1) != gsmOK) {
        printf("cannot initialized GSM-AT library\r\n");
    }

    if (gsm_device_is_present()) {
        xSemaphoreTake(connectivity_sync, pdMS_TO_TICKS(15000));
        gsm_modem_echo_off(NULL, NULL, 1);
        vTaskDelay(pdMS_TO_TICKS(2000));

        gsm_device_get_serial_number(serial, sizeof(serial), NULL, NULL, 1);
        serial[15] = 0;
        printf("Serial: %s\r\n", serial);
        if (strlen(serial) < 15) {
            ESP_LOGE(TAG, "Serial Error %s", serial);
            abort();
        }

        gsm_modem_dtr_1(NULL, NULL, 1);
        gsm_sms_enable(NULL, NULL, 1);
        gsm_sms_delete_all(GSM_SMS_STATUS_ALL, NULL, NULL, 1);

        gsm_urc_ri_pulse(NULL, NULL, 1);
        gsm_urc_enable_uart(NULL, NULL, 1);

        gsm_ppp_init();
        gsm_ppp_establish();
    }

    xSemaphoreTake(connectivity_sync, pdMS_TO_TICKS(15000));

#endif

    xTaskCreate(system_info_task, "system-info", 2048, NULL, 3, NULL);

    err = mfc2160_app_init();

    // ESP_LOGI(TAG, "mfc-2160 %d", err);
}
