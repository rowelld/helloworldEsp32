/*
 * gsm_modem_mode.h
 *
 *  Created on: May 26, 2020
 *      Author: spestano@spinginemail.com
 */

#ifndef COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_MODEM_MODE
#define COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_MODEM_MODE

#include "gsm/gsm_includes.h"

uint8_t gsm_modem_mode_is_data(void);

gsmr_t gsm_modem_disable_flow_control(const gsm_api_cmd_evt_fn evt_fn, void *const evt_arg, 
    const uint32_t blocking);
    
gsmr_t gsm_modem_echo_off(const gsm_api_cmd_evt_fn evt_fn, void *const evt_arg, 
    const uint32_t blocking);

gsmr_t gsm_modem_dtr_1(const gsm_api_cmd_evt_fn evt_fn, void *const evt_arg, 
    const uint32_t blocking);
#endif /* COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_MODEM_MODE */
