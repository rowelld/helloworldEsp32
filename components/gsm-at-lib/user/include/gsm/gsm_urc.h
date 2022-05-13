#ifndef COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_URC
#define COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_URC

#include "gsm/gsm_includes.h"

gsmr_t gsm_urc_enable_uart(const gsm_api_cmd_evt_fn evt_fn, void *const evt_arg, 
    const uint32_t blocking);
gsmr_t gsm_urc_reset(const gsm_api_cmd_evt_fn evt_fn, void *const evt_arg, 
    const uint32_t blocking);
gsmr_t gsm_urc_ri_pulse(const gsm_api_cmd_evt_fn evt_fn, void *const evt_arg, 
    const uint32_t blocking);
#endif /* COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_URC */
