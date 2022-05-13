/*
 * gsm_network_status.c
 *
 *  Created on: May 26, 2020
 *      Author: spestano@spinginemail.com
 */
#include "gsm/gsm_user_private.h"
#include "gsm/gsm_network_status.h"
#include "gsm/gsm_mem.h"
#include "gsm/gsm_ppp_api.h"
#include "gsm/gsm_modem_mode.h"

gsmr_t gsm_network_status(gsm_network_reg_status_t* status, const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);


    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CREG_GET;
    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_user_initiate_cmd, 10000);
}

gsmr_t gsm_network_set_apn(const char* apn, const char* user, const char* password, 
        const gsm_api_cmd_evt_fn evt_fn, void *const evt_arg, const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);
    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_USER_CMD_CGDCONT;
    GSM_MSG_VAR_REF(msg).msg.network_attach.apn = apn;
    GSM_MSG_VAR_REF(msg).msg.network_attach.user = user;
    GSM_MSG_VAR_REF(msg).msg.network_attach.pass = password;
    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_user_initiate_cmd, 500);
}

int16_t gsm_network_rssi_get(void)
{
    int16_t rssi = 0;
    if (gsm_modem_mode_is_data())
    {
        if (gsm_ppp_exit_online() == gsmOK) {
            gsm_network_rssi(&rssi, NULL, NULL, 1);
            gsm_ppp_resume();
        }
    }
    else
    {
        gsm_network_rssi(&rssi, NULL, NULL, 1);
    }
    return rssi;
}