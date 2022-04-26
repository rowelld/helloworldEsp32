#ifndef COMPONENTS_FOTA_FOTA_NVS
#define COMPONENTS_FOTA_FOTA_NVS

#include <stdlib.h>

void fota_nvs_read_url(char* url, size_t* len);

void fota_nvs_write_url(const char* url, size_t len);

#endif /* COMPONENTS_FOTA_FOTA_NVS */
