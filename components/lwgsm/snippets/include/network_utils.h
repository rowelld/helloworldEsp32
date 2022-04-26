#ifndef COMPONENTS_LWGSM_SNIPPETS_INCLUDE_NETWORK_UTILS
#define COMPONENTS_LWGSM_SNIPPETS_INCLUDE_NETWORK_UTILS

#include <stdint.h>

#include "lwgsm/lwgsm.h"

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

void network_utils_process_reg_change(lwgsm_evt_t* evt);
void network_utils_proces_curr_oeprator(lwgsm_evt_t* evt);
void network_utils_process_rssi(lwgsm_evt_t* evt);

#ifdef __cplusplus
}
#endif /*  __cplusplus */
#endif /* COMPONENTS_LWGSM_SNIPPETS_INCLUDE_NETWORK_UTILS */
