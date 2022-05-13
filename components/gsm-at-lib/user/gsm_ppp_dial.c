/*
 * gsm_ppp_dial.c
 *
 *  Created on: May 26, 2020
 *      Author: spestano@spinginemail.com
 */
#include "gsm/gsm_user_private.h"
#include "gsm/gsm_call.h"
#include "gsm/gsm_mem.h"

#include "gsm/gsm_ppp_dial.h"
/**
 * \brief           PPP dial
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       number: Phone number to call, including country code starting with `+` sign
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t gsm_ppp_dial(const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);
    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_PPP;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_user_initiate_cmd, 10000);
}


gsmr_t gsm_ppp_dial_done(const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);
    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_PPP_EXIT;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_user_initiate_cmd, 1000);
}

gsmr_t gsm_ppp_dial_done_dtr(const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);
    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_USER_CMD_PPP_EXIT;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_user_initiate_cmd, 1000);
}


gsmr_t gsm_ppp_dial_resume(const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);
    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_ATO;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_user_initiate_cmd, 1000);
}

gsmr_t gsm_ppp_terminate_call(const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);
    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_ATH;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_user_initiate_cmd, 10000);

}

gsmr_t gsm_ppp_send_data(const uint8_t* data, size_t len, const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking)
{

    return 0;     
}