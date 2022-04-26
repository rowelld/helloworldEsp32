#include <string.h>
#include "mbedtls/sha256.h"
#include "mbedtls/md.h"

#include "nvs_flash.h"

#include "commands_nvs.h"

#define NVS_PARTITION "cfg_part"
#define NVS_PARTITION_NAMESPACE "command-cfg"
#define NVS_KEY_DEVICE_PASSWORD "pass_hash"

void commands_get_config(commands_config_t *config)
{
    esp_err_t err;
    size_t hash_size = 32;
    nvs_handle handle;

    err = nvs_open_from_partition(NVS_PARTITION, NVS_PARTITION_NAMESPACE, NVS_READONLY, &handle);

    err = nvs_get_blob(handle, NVS_KEY_DEVICE_PASSWORD, config->hash, &hash_size);

    nvs_close(handle);
}

void commands_set_password(const char *password)
{   
    uint8_t *hash = command_config.hash;
    mbedtls_sha256((uint8_t *)password, strlen(password), hash, 0);

    esp_err_t err;
    nvs_handle handle;

    err = nvs_open_from_partition(NVS_PARTITION, NVS_PARTITION_NAMESPACE, NVS_READWRITE, &handle);    

    err = nvs_set_blob(handle, NVS_KEY_DEVICE_PASSWORD, hash, sizeof(command_config.hash));    

    nvs_close(handle);
}