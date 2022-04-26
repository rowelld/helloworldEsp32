#ifndef COMPONENTS_CLOUD_CLOUD_NVS
#define COMPONENTS_CLOUD_CLOUD_NVS

#include <stdint.h>
#include "cloud_api.h"

int16_t cloud_nvs_get(cloud_config_t* config);
void cloud_nvs_set_host(const char* host);
void cloud_nvs_set_port(uint16_t port);
void cloud_nvs_set_conntype(uint8_t conntype);

#endif /* COMPONENTS_CLOUD_CLOUD_NVS */
