/*
 * gsm_network_status.h
 *
 *  Created on: May 26, 2020
 *      Author: spestano@spinginemail.com
 */

#ifndef COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_NETWORK_STATUS_H_
#define COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_NETWORK_STATUS_H_

/* Get most important include files */
#include "gsm/gsm_includes.h"

gsmr_t gsm_network_status(gsm_network_reg_status_t* status, const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking);

gsmr_t gsm_network_set_apn(const char* apn, const char* user, const char* password, 
        const gsm_api_cmd_evt_fn evt_fn, void *const evt_arg, const uint32_t blocking);

int16_t gsm_network_rssi_get(void);
#endif /* COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_NETWORK_STATUS_H_ */
