/*
 * gsm_user_private.h
 *
 *  Created on: May 26, 2020
 *      Author: spestano@spinginemail.com
 */

#ifndef COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_USER_PRIVATE
#define COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_USER_PRIVATE

#include "gsm/gsm_includes.h"
#include "gsm/gsm_private.h"

enum {
    GSM_USER_CMD_START = GSM_CMD_END,
    GSM_USER_CMD_CGDCONT,
    GSM_USER_CMD_END,
    GSM_USER_CMD_QURCCFG,
    GSM_USER_CMD_QURCRST,
    GSM_USER_CMD_QRICFG,
    GSM_USER_CMD_PPP_DATA,
    GSM_USER_CMD_PPP_EXIT,
    GSM_USER_CMD_ATD_1,
};
 /**
 * \brief           Function to initialize every AT command.
 * \note            Never call this function directly. Set as initialization function for command and use `msg->fn(msg)`
 * \param[in]       msg: Pointer to \ref gsm_msg_t with data
 * \return          Member of \ref gsmr_t enumeration
 */
gsmr_t
gsmi_user_initiate_cmd(gsm_msg_t* msg);

#endif /* COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_USER_PRIVATE */
