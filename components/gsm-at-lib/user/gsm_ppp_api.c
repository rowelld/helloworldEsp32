/*
 * gsm_ppp_api.c
 *
 *  Created on: May 26, 2020
 *      Author: spestano@spinginemail.com
 */
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "gsm/gsm.h"
#include "gsm/gsm_call.h"
#include "gsm/gsm_nvs.h"
#include "gsm/gsm_modem_mode.h"
#include "gsm/gsm_user_private.h"
#include "gsm/gsm_ppp_api.h"
#include "gsm/gsm_ppp_dial.h"
#include "gsm/gsm_network_status.h"

static gsm_config_t gsm_config = {0};
static SemaphoreHandle_t semph = NULL;

gsmr_t gsm_ppp_init(void)
{
    semph = xSemaphoreCreateMutex();
    gsmr_t err = gsm_config_get(&gsm_config);;
    printf("GSM APN=%s\n",gsm_config.apn);
    return err;
}

gsmr_t gsm_ppp_establish(void)
{
    uint32_t retry_count = 0;
    /* Check for network status at most 60 seconds */
    do {
        gsm_network_status(NULL, NULL, NULL, 1);
        /* Query every 1000 ms*/
        gsm_delay(1000);
    }while( (gsm_network_get_reg_status() != GSM_NETWORK_REG_STATUS_CONNECTED) &&
            retry_count++ < 60);

    /* Set APN */
    gsm_network_set_apn(gsm_config.apn, gsm_config.user, gsm_config.pass, NULL, NULL, 1);

    gsm_modem_disable_flow_control(NULL, NULL, 1);

    /* Give 1500 ms delay */
    gsm_delay(1500);
    if (gsm_ppp_dial( NULL, NULL, 1) != gsmOK) {
        printf("Failed to call\r\n");
    }

    return gsmOK;
}

gsmr_t gsm_ppp_re_establish(void)
{
    if (gsm_modem_mode_is_data()) {
        xSemaphoreTake(semph, portMAX_DELAY);
        /* Exit PPP online mode */
        gsm_ppp_dial_done_dtr(NULL, NULL, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        xSemaphoreGive(semph);
    }

    /* Disconnect PPP */
    gsm_ppp_terminate_call(NULL, NULL, 1);
    
    /* Re establish PPP */
    gsm_ppp_establish();
    return gsmERR;
}

gsmr_t gsm_ppp_exit_online(void)
{
    xSemaphoreTake(semph, portMAX_DELAY);
    gsmr_t err = gsm_ppp_dial_done_dtr(NULL, NULL, 1);
    xSemaphoreGive(semph);
    return gsmOK;
}

gsmr_t gsm_ppp_resume(void)
{
    return gsm_ppp_dial_resume(NULL, NULL, 1);
}

size_t gsm_ppp_write(const uint8_t* data, size_t len)
{
    size_t ret;
    xSemaphoreTake(semph, portMAX_DELAY);
    ret = gsm.ll.send_fn(data, len);
    xSemaphoreGive(semph);
    /* Access low level uart write function */
    return ret; 
}