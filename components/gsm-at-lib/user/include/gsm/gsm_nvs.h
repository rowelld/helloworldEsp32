#ifndef COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_NVS
#define COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_NVS

#include "gsm/gsm.h"
typedef struct {
    char apn[32];
    char user[32];
    char pass[32];
}gsm_config_t;

gsmr_t gsm_config_get(gsm_config_t* config);

gsmr_t gsm_config_set(const char* apn, const char* user, const char* pass);

#endif /* COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_NVS */
