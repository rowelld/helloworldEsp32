#include "fota.h"

#include <string.h>

#include "cloud_api.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "fota_nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

#define BUFFSIZE 1024

static const char* TAG = "FOTA";

static char url[512] = {0};
static size_t url_len = 0;
static uint8_t fota_started = 0;

static char error_message[1024] = {0};
static int binary_file_length = 0;
static int firmware_size = 0;

extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

static esp_err_t validate_image_header(esp_app_desc_t* new_app_info) {
    if (new_app_info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
    }

    if (memcmp(new_app_info->version, running_app_info.version,
               sizeof(new_app_info->version)) == 0) {
        ESP_LOGW(TAG,
                 "Current running version is the same as a new. We will not "
                 "continue the update.");
        return ESP_FAIL;
    }

    return ESP_OK;
}

static esp_err_t _http_client_init_cb(esp_http_client_handle_t http_client) {
    esp_err_t err = ESP_OK;
    return err;
}

static void fota_task(void* data) {
    ESP_LOGI(TAG, "Starting FOTA");

    esp_err_t fota_finish_err = ESP_OK;
    ESP_LOGI(TAG, "url %s", url);

    esp_http_client_config_t config = {
        .url = url,
        .cert_pem = (char*)server_cert_pem_start,
        .timeout_ms = 5000,
        .keep_alive_enable = true,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
        .http_client_init_cb = _http_client_init_cb,
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed");
        vTaskDelete(NULL);
    }

    esp_app_desc_t app_desc;
    err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_https_ota_read_img_desc failed");
        goto ota_end;
    }
    err = validate_image_header(&app_desc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "image header verification failed");
        goto ota_end;
    }

    while (1) {
        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        ESP_LOGD(TAG, "Image bytes read: %d", esp_https_ota_get_image_len_read(https_ota_handle));
    }

    if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {
        ESP_LOGE(TAG, "Complete data was not received.");
    } else {
        fota_finish_err = esp_https_ota_finish(https_ota_handle);
        if ((err == ESP_OK) && (fota_finish_err == ESP_OK)) {
            ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting...");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            esp_restart();
        } else {
            if (fota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
                ESP_LOGE(TAG, "Image validation failed, image is corrupted");
            }
            ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed 0x%x", fota_finish_err);
            vTaskDelete(NULL);
        }
    }

ota_end:
    esp_https_ota_abort(https_ota_handle);
    ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed");
    vTaskDelete(NULL);
}

void fota_init(void) {
    fota_nvs_read_url(url, &url_len);
    ESP_LOGI(TAG, "len= %d URL=%s", url_len, url);
}

void fota_start(void) {
    ESP_LOGI(TAG, "Fota Started");

    if (xTaskCreate(fota_task, "fota-task", 1024 * 8, NULL, 4, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create fota task");
    }
}

void fota_set_url(const char* new_url) {
    strncpy(url, new_url, 255);
    fota_nvs_write_url(url, strlen(url));
}

const char* fota_get_url(void) { return url; }

const char* fota_get_version(void) {
    const esp_app_desc_t* desc = esp_ota_get_app_description();
    return desc->version;
}

