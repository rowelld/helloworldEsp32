#include "esp_log.h"
#include "nvs_flash.h"

#include "cloud_nvs.h"

#define NVS_PARTITION "cfg_part"
#define NVS_PARTITION_NAMESPACE "cloud-cfg"
#define NVS_KEY_HOST "host"
#define NVS_KEY_PORT "port"
#define NVS_KEY_CONNTYPE "conntype"

int16_t cloud_nvs_get(cloud_config_t* config) {
    esp_err_t err;
    nvs_handle handle;
    size_t str_len = sizeof(config->host) - 1;
    
    err = nvs_open_from_partition(NVS_PARTITION, NVS_PARTITION_NAMESPACE, NVS_READONLY, &handle);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    err = nvs_get_str(handle, NVS_KEY_HOST, config->host, &str_len);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    err = nvs_get_u16(handle, NVS_KEY_PORT, &config->port);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    err = nvs_get_u8(handle, NVS_KEY_CONNTYPE, &config->conntype);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    nvs_close(handle);

    return err;
}

void cloud_nvs_set_host(const char* host) {
    esp_err_t err;
    nvs_handle handle;
    
    err = nvs_open_from_partition(NVS_PARTITION, NVS_PARTITION_NAMESPACE, NVS_READWRITE, &handle);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    err = nvs_set_str(handle, NVS_KEY_HOST, host);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    nvs_commit(handle);
    nvs_close(handle);
}

void cloud_nvs_set_port(uint16_t port) {
    esp_err_t err;
    nvs_handle handle;
    
    err = nvs_open_from_partition(NVS_PARTITION, NVS_PARTITION_NAMESPACE, NVS_READWRITE, &handle);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    err = nvs_set_u16(handle, NVS_KEY_PORT, port);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    nvs_commit(handle);
    nvs_close(handle);
}

void cloud_nvs_set_conntype(uint8_t conntype) {
    esp_err_t err;
    nvs_handle handle;
    
    err = nvs_open_from_partition(NVS_PARTITION, NVS_PARTITION_NAMESPACE, NVS_READWRITE, &handle);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    err = nvs_set_u8(handle, NVS_KEY_CONNTYPE, conntype);
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);

    nvs_commit(handle);
    nvs_close(handle);
}