/**
 * \file            lwgsm_ll_esp32.c
 * \brief           Low-level communication with GSM device template
 */

/*
 * Copyright (c) 2020 Tilen MAJERLE
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
 * This file is part of LwGSM - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.0
 */
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "lwgsm/lwgsm.h"
#include "lwgsm/lwgsm_input.h"
#include "lwgsm/lwgsm_mem.h"
#include "lwgsm/lwgsm_private.h"
#include "system/lwgsm_ll.h"

#define GSM_CFG_UART_NUM UART_NUM_1
#define GSM_CFG_UART_TX_PIN 27
#define GSM_CFG_UART_RX_PIN 26
#define GSM_CFG_POWER_KEY_PIN 4
#define GSM_CFG_RST_PIN 5
#define GSM_CFG_POWER_ON_PIN 23
#define GSM_CFG_UART_RX_BUFF_SIZE 4096
#define GSM_CFG_UART_EVENT_QUEUE_SIZE 50

#define GSM_CFG_LL_TASK_SIZE 2048 * 5
#define GSM_CFG_LL_TASK_PRIORITY 1
#define MIN_POST_IDLE 10
#define MIN_PRE_IDLE 10
#define MIN_PATTERN_INTERVAL 10000

#define ESP_MODEM_LINE_BUFFER_SIZE (GSM_CFG_UART_RX_BUFF_SIZE / 2)
#define ESP_MODEM_EVENT_QUEUE_SIZE (16)

static lwgsm_sys_mbox_t gsm_mbox;
static lwgsm_sys_thread_t thread;
static uint8_t initialized = 0;
static uint8_t gsm_buffer[0x1000];
static const char* TAG = "lwgsm_uart";

/**
 * \brief           Send data to GSM device, function called from GSM stack when
 * we have data to send \param[in]       data: Pointer to data to send
 * \param[in]       len: Number of bytes to send
 * \return          Number of bytes sent
 */
static size_t send_data(const void* data, size_t len) {
    if (data) {
        uart_write_bytes(GSM_CFG_UART_NUM, data, len);
        ESP_LOG_BUFFER_HEXDUMP(TAG, data, len, ESP_LOG_INFO);
    }

    return len;
}

static void uart_event_task_entry(void* param) {
    uart_event_t event;
    while (1) {
        if (xQueueReceive(gsm_mbox, &event, portMAX_DELAY)) {
            size_t len = 0;
            switch (event.type) {
                case UART_DATA: {
                    len = uart_read_bytes(GSM_CFG_UART_NUM, gsm_buffer,
                                          GSM_CFG_UART_RX_BUFF_SIZE, 50);

                    if (len) {
                    }
                }
                /* code */
                break;

                default:
                    break;
            }
        }
    }
    vTaskDelete(NULL);
}

static void configure_uart(uint32_t baudrate) {
    uart_config_t uart_config = {0};

    uart_config.baud_rate = baudrate;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.rx_flow_ctrl_thresh = 0;
    // uart_config.use_ref_tick = 0;

    ESP_ERROR_CHECK(uart_param_config(GSM_CFG_UART_NUM, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(GSM_CFG_UART_NUM, GSM_CFG_UART_TX_PIN,
                                 GSM_CFG_UART_RX_PIN, UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(
        uart_driver_install(GSM_CFG_UART_NUM, GSM_CFG_UART_RX_BUFF_SIZE, 0,
                            GSM_CFG_UART_EVENT_QUEUE_SIZE, &gsm_mbox, 0));
    lwgsm_sys_thread_create(&thread, "lwgsm_uart", uart_event_task_entry, NULL,
                            GSM_CFG_LL_TASK_SIZE, LWGSM_SYS_THREAD_PRIO + 1);
}

/**
 * \brief           Callback function called from initialization process
 *
 * \note            This function may be called multiple times if AT baudrate is
 * changed from application. It is important that every configuration except AT
 * baudrate is configured only once!
 *
 * \note            This function may be called from different threads in GSM
 * stack when using OS. When \ref LWGSM_CFG_INPUT_USE_PROCESS is set to 1, this
 * function may be called from user UART thread.
 *
 * \param[in,out]   ll: Pointer to \ref lwgsm_ll_t structure to fill data for
 * communication functions \return          lwgsmOK on success, member of \ref
 * lwgsmr_t enumeration otherwise
 */
lwgsmr_t lwgsm_ll_init(lwgsm_ll_t* ll) {
#if !LWGSM_CFG_MEM_CUSTOM
    /* Step 1: Configure memory for dynamic allocations */
    static uint8_t memory[0x10000]; /* Create memory for dynamic allocations
                                       with specific size */

    /*
     * Create region(s) of memory.
     * If device has internal/external memory available,
     * multiple memories may be used
     */
    lwgsm_mem_region_t mem_regions[] = {{memory, sizeof(memory)}};
    if (!initialized) {
        lwgsm_mem_assignmemory(
            mem_regions,
            LWGSM_ARRAYSIZE(mem_regions)); /* Assign memory for allocations to
                                              GSM library */
    }
#endif /* !LWGSM_CFG_MEM_CUSTOM */

#ifdef GSM_CFG_RST_PIN
    gpio_set_direction(GSM_CFG_RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(GSM_CFG_RST_PIN, 1);
#endif

    gpio_set_direction(GSM_CFG_POWER_KEY_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(GSM_CFG_POWER_ON_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(GSM_CFG_POWER_ON_PIN, 1);

    gpio_set_level(GSM_CFG_POWER_KEY_PIN, 1);
    lwgsm_delay(100);
    gpio_set_level(GSM_CFG_POWER_KEY_PIN, 0);
    lwgsm_delay(1000);
    gpio_set_level(GSM_CFG_POWER_KEY_PIN, 1);

    /* Step 2: Set AT port send function to use when we have data to transmit */
    if (!initialized) {
        ll->send_fn = send_data; /* Set callback function to send data */
    }

    /* Step 3: Configure AT port to be able to send/receive data to/from GSM
     * device */
    configure_uart(ll->uart.baudrate); /* Initialize UART for communication */
    lwgsm_delay(500);
    initialized = 1;

    lwgsm.m.sms.ready = 1;
    lwgsmi_send_cb(LWGSM_EVT_SMS_READY);
    lwgsm.m.sms.enabled = 1;
    lwgsm.evt.evt.sms_enable.status = lwgsm.m.sms.enabled ? lwgsmOK : lwgsmERR;
    lwgsmi_send_cb(LWGSM_EVT_SMS_ENABLE);
     return lwgsmOK;
}

/**
 * \brief           Callback function to de-init low-level communication part
 * \param[in,out]   ll: Pointer to \ref lwgsm_ll_t structure to fill data for
 * communication functions \return          \ref lwgsmOK on success, member of
 * \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t lwgsm_ll_deinit(lwgsm_ll_t* ll) {
    initialized = 0; /* Clear initialized flag */
    return lwgsmOK;
}
