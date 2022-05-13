/**
 * \file            main.c
 * \brief           Main file
 */

/*
 * Copyright (c) 2019 Tilen MAJERLE
 *  
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of GSM-AT library.
 *
 * Before you start using WIN32 implementation with USB and VCP,
 * check gsm_ll_win32.c implementation and choose your COM port!
 */
#include "gsm/gsm.h"
#include "sim_manager.h"
#include "network_utils.h"

static gsmr_t gsm_callback_func(gsm_evt_t* evt);

/* Device info string array */
char dev_str[20];

/**
 * \brief           Program entry point
 */
int
main(void) {
    printf("Starting GSM application!\r\n");

    /* Initialize GSM with default callback function */
    if (gsm_init(gsm_callback_func, 1) != gsmOK) {
        printf("Cannot initialize GSM-AT Library\r\n");
    }

    /* Read information */

    /* Read device manufacturer */
    gsm_device_get_manufacturer(dev_str, sizeof(dev_str), NULL, NULL, 1);
    printf("Manuf: %s\r\n", dev_str);

    /* Read device model */
    gsm_device_get_model(dev_str, sizeof(dev_str), NULL, NULL, 1);
    printf("Model: %s\r\n", dev_str);
    
    /* Read device serial number */
    gsm_device_get_serial_number(dev_str, sizeof(dev_str), NULL, NULL, 1);
    printf("Serial: %s\r\n", dev_str);
    
    /* Read device revision */
    gsm_device_get_revision(dev_str, sizeof(dev_str), NULL, NULL, 1);
    printf("Revision: %s\r\n", dev_str);

    /*
     * Do not stop program here.
     * New threads were created for GSM processing
     */
    while (1) {
        gsm_delay(1000);
    }

    return 0;
}

/**
 * \brief           Event callback function for GSM stack
 * \param[in]       evt: Event information with data
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
static gsmr_t
gsm_callback_func(gsm_evt_t* evt) {
    switch (gsm_evt_get_type(evt)) {
        case GSM_EVT_INIT_FINISH: printf("Library initialized!\r\n"); break;
        /* Process and print registration change */
        case GSM_EVT_NETWORK_REG_CHANGED: network_utils_process_reg_change(evt); break;
        /* Process current network operator */
        case GSM_EVT_NETWORK_OPERATOR_CURRENT: network_utils_process_curr_operator(evt); break;
        /* Process signal strength */
        case GSM_EVT_SIGNAL_STRENGTH: network_utils_process_rssi(evt); break;

        /* Other user events here... */

        default: break;
    }
    return gsmOK;
}

