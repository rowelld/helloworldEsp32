#include "gsm/gsm_qctl_sms.h"
#include "gsm/gsm_private.h"
#include "gsm/gsm_mem.h"
#include "gsm/gsm_user_private.h"

gsmr_t gsm_sms_get_unread(const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking)
{

    GSM_MSG_VAR_DEFINE(msg);
    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CMGL;
    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_user_initiate_cmd, 10000);
}