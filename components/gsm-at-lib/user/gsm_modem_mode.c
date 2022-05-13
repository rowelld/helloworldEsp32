/*
 * gsm_modem_mode.c
 *
 *  Created on: May 26, 2020
 *      Author: spestano@spinginemail.com
 */


#include "gsm/gsm_modem_mode.h"
#include "gsm/gsm_private.h"
#include "gsm/gsm_mem.h"
#include "gsm/gsm_user_private.h"
uint8_t gsm_modem_mode_is_data(void)
{
    return gsm.mode;
}

gsmr_t gsm_modem_disable_flow_control(const gsm_api_cmd_evt_fn evt_fn, void *const evt_arg, 
    const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);
    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_IFC;
    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_user_initiate_cmd, 500);
}

gsmr_t gsm_modem_echo_off(const gsm_api_cmd_evt_fn evt_fn, void *const evt_arg, 
    const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);
    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_ATE0;
    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_user_initiate_cmd, 500);
}

gsmr_t gsm_modem_dtr_1(const gsm_api_cmd_evt_fn evt_fn, void *const evt_arg, 
    const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);
    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_USER_CMD_ATD_1;
    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_user_initiate_cmd, 500);   
}