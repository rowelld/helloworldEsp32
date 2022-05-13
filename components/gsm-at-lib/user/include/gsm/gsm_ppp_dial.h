/*
 * gsm_ppp_dial.h
 *
 *  Created on: May 26, 2020
 *      Author: spestano@spinginemail.com
 */

#ifndef COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_PPP_DIAL_H_
#define COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_PPP_DIAL_H_
/* Get most important include files */
#include "gsm/gsm_includes.h"

gsmr_t gsm_ppp_dial(const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking);

gsmr_t gsm_ppp_dial_done(const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking);

gsmr_t gsm_ppp_dial_done_dtr(const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking);
gsmr_t gsm_ppp_dial_resume(const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking);

gsmr_t gsm_ppp_terminate_call(const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking);
        
gsmr_t gsm_ppp_send_data(const uint8_t* data, size_t len, const gsm_api_cmd_evt_fn evt_fn,
        void *const evt_arg, const uint32_t blocking);
#endif /* COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_PPP_DIAL_H_ */
