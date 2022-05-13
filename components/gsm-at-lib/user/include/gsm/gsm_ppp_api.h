/*
 * gsm_ppp_api.h
 *
 *  Created on: May 26, 2020
 *      Author: spestano@spinginemail.com
 */

#ifndef COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_PPP_API_H_
#define COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_PPP_API_H_

#include "gsm/gsm_includes.h"
gsmr_t gsm_ppp_init(void);

gsmr_t gsm_ppp_establish(void);

gsmr_t gsm_ppp_re_establish(void);

gsmr_t gsm_ppp_exit_online(void);

gsmr_t gsm_ppp_resume(void);

size_t gsm_ppp_write(const uint8_t* data, size_t len);
#endif /* COMPONENTS_GSM_AT_LIB_USER_INCLUDE_GSM_GSM_PPP_API_H_ */
