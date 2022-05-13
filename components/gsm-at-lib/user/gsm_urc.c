#include "gsm/gsm_urc.h"
#include "gsm/gsm_private.h"
#include "gsm/gsm_user_private.h"
#include "gsm/gsm_mem.h"

gsmr_t gsm_urc_enable_uart(const gsm_api_cmd_evt_fn evt_fn, void *const evt_arg, 
    const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);
    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_USER_CMD_QURCCFG;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_user_initiate_cmd, 10000);

}

gsmr_t gsm_urc_reset(const gsm_api_cmd_evt_fn evt_fn, void *const evt_arg, 
    const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);
    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_USER_CMD_QURCRST;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_user_initiate_cmd, 10000);

}

gsmr_t gsm_urc_ri_pulse(const gsm_api_cmd_evt_fn evt_fn, void *const evt_arg, 
    const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);
    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_USER_CMD_QRICFG;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_user_initiate_cmd, 10000);

}