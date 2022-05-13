#ifndef COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_QCTL_SMS
#define COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_QCTL_SMS

#include "gsm/gsm_includes.h"

gsmr_t gsm_sms_get_unread(const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking);
#endif /* COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_QCTL_SMS */
