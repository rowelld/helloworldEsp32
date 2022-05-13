#include "esp_log.h"
#include "nvs_flash.h"
#include "gsm/gsm_nvs.h"

#define NVS_PARTITION "cfg_part"
#define NVS_PARTITION_NAMESPACE "gsm-cfg"
#define NVS_KEY_APN "apn"
#define NVS_KEY_USER "user"
#define NVS_KEY_PASS "pass"

gsmr_t gsm_config_get(gsm_config_t* config)
{
    esp_err_t err;
    nvs_handle handle;
    size_t buffer_size = 32;

    err = nvs_open_from_partition(NVS_PARTITION, NVS_PARTITION_NAMESPACE, NVS_READONLY, &handle);

    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    err = nvs_get_str(handle, NVS_KEY_APN, NULL, &buffer_size);
    err = nvs_get_str(handle, NVS_KEY_APN, config->apn, &buffer_size);

    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    err = nvs_get_str(handle, NVS_KEY_USER,  config->user, &buffer_size);

    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    err = nvs_get_str(handle, NVS_KEY_PASS,  config->pass, &buffer_size);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    nvs_close(handle);

    ESP_LOGI(__FUNCTION__, "APN: %s", config->apn);
    ESP_LOGI(__FUNCTION__, "APN Username: %s", config->user);
    ESP_LOGI(__FUNCTION__, "APN Password: %s", config->pass);
    return gsmOK;
}

gsmr_t gsm_config_set(const char* apn, const char* user, const char* pass)
{
    esp_err_t err;
    nvs_handle handle;
   
    err = nvs_open_from_partition(NVS_PARTITION, NVS_PARTITION_NAMESPACE, NVS_READWRITE, &handle);

    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    err = nvs_set_str(handle, NVS_KEY_APN, apn);

    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    err = nvs_set_str(handle, NVS_KEY_USER, user);

    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    err = nvs_set_str(handle, NVS_KEY_PASS, pass);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    nvs_close(handle);

    return gsmOK;
}