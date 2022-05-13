/**
 * \file            gsm_ll_template.c
 * \brief           Low-level communication with GSM device template
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
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 *
 * Modified:
 *
 * Added PPP support
 */
#include "system/gsm_ll.h"
#include "gsm/gsm.h"
#include "gsm/gsm_private.h"
#include "gsm/gsm_mem.h"
#include "gsm/gsm_input.h"
#include "system/gsm_sys.h"
#include "gsm/gsm_user_private.h"

#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define GSM_CFG_UART_NUM                UART_NUM_1
#define GSM_CFG_UART_TX_PIN             27
#define GSM_CFG_UART_RX_PIN             26
#define GSM_CFG_UART_RX_BUFF_SIZE       4096
#define GSM_CFG_UART_EVENT_QUEUE_SIZE   50
#define GSM_CFG_UART_POWER_KEY          4
#define GSM_CFG_UART_RST                5
#define GSM_CFG_UART_POWER_ON           23

#define GSM_CFG_LL_TASK_SIZE                 2048*5
#define GSM_CFG_LL_TASK_PRIORITY             1
#define MIN_POST_IDLE                        10
#define MIN_PRE_IDLE                         10
#define MIN_PATTERN_INTERVAL                 10000

#define ESP_MODEM_LINE_BUFFER_SIZE (GSM_CFG_UART_RX_BUFF_SIZE / 2)
#define ESP_MODEM_EVENT_QUEUE_SIZE (16)

static gsm_sys_mbox_t gsm_mbox;
static gsm_sys_thread_t thread;
static uint8_t initialized = 0;
static uint8_t gsm_buffer[0x1000];
static const char* TAG = "gsm_uart";

/**
 * \brief UART Event Task Entry
 * \param[in] param: Task parameter
 */
static void uart_event_task_entry(void *param) {

    uart_event_t event;
    while (1) {
        if (xQueueReceive(gsm_mbox, &event, portMAX_DELAY)) {
            size_t len = 0;
            switch (event.type) {
                case UART_DATA: {
                    len = uart_read_bytes( GSM_CFG_UART_NUM,
                            gsm_buffer, GSM_CFG_UART_RX_BUFF_SIZE, 50);
                    if (len) {
                        
                        /* Make sure we dont miss pp
                        p packets check flags 0x7e */
                        if ( gsm.mode || ((gsm_buffer[0] == 0x7e) && (gsm_buffer[len - 1] == 0x7e))) {
                            gsm.evt.evt.ppp_rcv.data = gsm_buffer;
                            gsm.evt.evt.ppp_rcv.len = len;
                            gsmi_send_cb(GSM_EVT_PPP_RECV);
                            
                        } else {
                             ESP_LOG_BUFFER_HEXDUMP("<<gsm", gsm_buffer, len, ESP_LOG_INFO);
                            if (CMD_IS_CUR(GSM_CMD_PPP_EXIT)) {
                                /* The message receive is not OK response, send it to ppp rcv*/
                                if (strncmp((char*)gsm_buffer, "\r\nOK\r\n", 6)) {
                                    gsm.evt.evt.ppp_rcv.data = gsm_buffer;
                                    gsm.evt.evt.ppp_rcv.len = len;
                                    gsmi_send_cb(GSM_EVT_PPP_RECV);
                                    break;
                                }
                            }
                            /* Try to remove new line from start of response */
                            if (len > 2 && gsm_buffer[0] == '\r' && gsm_buffer[1] == '\n') {
                                gsm_input_process(gsm_buffer + 2, len - 2);
                            } else {
                                gsm_input_process(gsm_buffer, len);
                            }
                        }
                    }
                    break;
                }
                case UART_FIFO_OVF:
                    ESP_LOGW(TAG,"HW FIFO Overflow\n");
                    uart_flush_input(GSM_CFG_UART_NUM);
                    xQueueReset(gsm_mbox);
                    break;

                case UART_BUFFER_FULL:
                    ESP_LOGW(TAG,"Ring Buffer Full");
                    uart_flush_input(GSM_CFG_UART_NUM);
                    xQueueReset(gsm_mbox);
                    break;

                case UART_BREAK:
                    ESP_LOGW(TAG,"Rx Break\n");
                    break;

                case UART_PARITY_ERR:
                    ESP_LOGW(TAG,"Parity Error\n");
                    break;

                case UART_FRAME_ERR:
                    ESP_LOGW(TAG,"Frame Error\n");
                    break;

                case UART_PATTERN_DET:
                    break;

                default:
                    ESP_LOGW(TAG,"unknown uart event type: %d\n", event.type);
                    break;
            }

        }

    }
    vTaskDelete(NULL);
}



/**
 * \brief           Send data to GSM device, function called from GSM stack when we have data to send
 * \param[in]       data: Pointer to data to send
 * \param[in]       len: Number of bytes to send
 * \return          Number of bytes sent
 */
static size_t
send_data(const void* data, size_t len) {
    /* Implement send function here */
    if (data) {
        uart_write_bytes(GSM_CFG_UART_NUM, data, len);
        if (!gsm.mode) {
            ESP_LOG_BUFFER_HEXDUMP(TAG, data, len, ESP_LOG_INFO);
        }
    }
    return len;                                 /* Return number of bytes actually sent to AT port */
}

/**
 * \brief           Configure UART using DMA for receive in double buffer mode and IDLE line detection
 */
static void
configure_uart(uint32_t baudrate) {
    uart_config_t uart_config = { 0 };

    /* Initialize UART configurations */
    uart_config.baud_rate = baudrate;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.rx_flow_ctrl_thresh = 0;
    uart_config.use_ref_tick = 0;

    /* Set UART configuration */
    ESP_ERROR_CHECK(uart_param_config(GSM_CFG_UART_NUM, &uart_config));

    /* Set UART pin assignment */
    ESP_ERROR_CHECK(
            uart_set_pin(GSM_CFG_UART_NUM, GSM_CFG_UART_TX_PIN, GSM_CFG_UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    /* Install UART driver and get event queue used inside driver */
    ESP_ERROR_CHECK(
            uart_driver_install(GSM_CFG_UART_NUM, GSM_CFG_UART_RX_BUFF_SIZE, 0, GSM_CFG_UART_EVENT_QUEUE_SIZE, &gsm_mbox, 0));

    gsm_sys_thread_create(&thread, "gsm_uart", uart_event_task_entry, NULL, GSM_CFG_LL_TASK_SIZE, GSM_SYS_THREAD_PRIO+1);

}

/**
 * \brief           Callback function called from initialization process
 *
 * \note            This function may be called multiple times if AT baudrate is changed from application.
 *                  It is important that every configuration except AT baudrate is configured only once!
 *
 * \note            This function may be called from different threads in GSM stack when using OS.
 *                  When \ref GSM_CFG_INPUT_USE_PROCESS is set to 1, this function may be called from user UART thread.
 *
 * \param[in,out]   ll: Pointer to \ref gsm_ll_t structure to fill data for communication functions
 * \return          gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_ll_init(gsm_ll_t* ll) {
#if !GSM_CFG_MEM_CUSTOM
    /* Step 1: Configure memory for dynamic allocations */
    static uint8_t memory[0x10000];             /* Create memory for dynamic allocations with specific size */

    /*
     * Create region(s) of memory.
     * If device has internal/external memory available,
     * multiple memories may be used
     */
    gsm_mem_region_t mem_regions[] = {
        { memory, sizeof(memory) }
    };
    if (!initialized) {
        gsm_mem_assignmemory(mem_regions, GSM_ARRAYSIZE(mem_regions));  /* Assign memory for allocations to GSM library */
    }
#endif /* !GSM_CFG_MEM_CUSTOM */
    gpio_set_direction(GSM_CFG_UART_RST, GPIO_MODE_OUTPUT);    
    gpio_set_level(GSM_CFG_UART_RST, 1);

    gpio_set_direction(GSM_CFG_UART_POWER_KEY, GPIO_MODE_OUTPUT);
    gpio_set_direction(GSM_CFG_UART_POWER_ON, GPIO_MODE_OUTPUT);

    gpio_set_level(GSM_CFG_UART_POWER_ON, 1);

    gpio_set_level(GSM_CFG_UART_POWER_KEY, 1);
    gsm_delay(100);
    gpio_set_level(GSM_CFG_UART_POWER_KEY, 0);
    gsm_delay(1000);
    gpio_set_level(GSM_CFG_UART_POWER_KEY, 1);

    /* Step 2: Set AT port send function to use when we have data to transmit */
    if (!initialized) {
        ll->send_fn = send_data;                /* Set callback function to send data */
    }

    /* Step 3: Configure AT port to be able to send/receive data to/from GSM device */
    configure_uart(ll->uart.baudrate);          /* Initialize UART for communication */
    gsm_delay(500);
    initialized = 1;

    /* Initialize SMS manually */
    gsm.m.sms.ready = 1;                /* SMS ready flag */
    gsmi_send_cb(GSM_EVT_SMS_READY);    /* Send SMS ready event */
    gsm.m.sms.enabled = 1;              /* Set enabled status */
    gsm.evt.evt.sms_enable.status = gsm.m.sms.enabled ? gsmOK : gsmERR;
    gsmi_send_cb(GSM_EVT_SMS_ENABLE);   /* Send to user */
    return gsmOK;
}

/**
 * \brief           Callback function to de-init low-level communication part
 * \param[in,out]   ll: Pointer to \ref gsm_ll_t structure to fill data for communication functions
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_ll_deinit(gsm_ll_t* ll) {
    initialized = 0;                            /* Clear initialized flag */
    return gsmOK;
}
