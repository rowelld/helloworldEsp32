#ifndef COMPONENTS_WIFI_NAT_WIFI_NAT_NVS
#define COMPONENTS_WIFI_NAT_WIFI_NAT_NVS

#include "esp_err.h"

esp_err_t get_config_param_str(char* name, char** param);

void register_router(void);

void preprocess_string(char* str);

int set_ap(int argc, char** argv);

int set_sta(int argc, char** argv);

int set_sta_static(int argc, char** argv);

#endif /* COMPONENTS_WIFI_NAT_WIFI_NAT_NVS */
