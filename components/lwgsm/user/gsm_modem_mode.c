#include "gsm_modem_mode.h"
#include "gsm_user_private.h"
#include "lwgsm/lwgsm_mem.h"
#include "lwgsm/lwgsm_private.h"

lwgsmr_t gsm_modem_echo_off(const lwgsm_api_cmd_evt_fn evt_fn, void *const evt_arg, const uint32_t blocking) {

    LWGSM_MSG_VAR_DEFINE(msg);
    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_ATE0;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), gsmi_userinitiate_cmd, 500);
}