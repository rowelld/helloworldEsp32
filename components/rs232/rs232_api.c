#include "rs232_api.h"

#include <string.h>

#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "cloud_api.h"

#define RS232_UART_TX_PIN 1
#define RS232_UART_RX_PIN 3
#define RS232_UART_NUM 0
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

static SemaphoreHandle_t semph;
static QueueHandle_t uart0_queue;

static const char* TAG = "RS232";

static uint8_t rs232_mode = 0;
static uint8_t dtmp[RD_BUF_SIZE];

static void rs232_task_rx(void* pdata) {
    (void)pdata;
    TickType_t xTicksToWait = pdMS_TO_TICKS(10000);
    TimeOut_t xTimeOut;
    while (1) {
        uart_event_t event = {0};
        vTaskSetTimeOutState(&xTimeOut);
        if (xQueueReceive(uart0_queue, (void*)&event, pdMS_TO_TICKS(10000))) {
            memset(dtmp, 0, RD_BUF_SIZE);
            ESP_LOGI(TAG, "uart[%d] event", RS232_UART_NUM);
            switch (event.type) {
                case UART_DATA:
                    ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                    uart_read_bytes(RS232_UART_NUM, dtmp, event.size, 50);
                    ESP_LOG_BUFFER_HEX(TAG, dtmp, event.size);
                    break;

                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    uart_flush_input(RS232_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;

                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    uart_flush_input(RS232_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;

                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
}

static void rs232_init_uart(void) {
    uart_config_t uart_config = {.baud_rate = 115200,
                                 .data_bits = UART_DATA_8_BITS,
                                 .parity = UART_PARITY_DISABLE,
                                 .stop_bits = UART_STOP_BITS_1,
                                 .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_param_config(RS232_UART_NUM, &uart_config);

    uart_set_pin(RS232_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(RS232_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20,
                        &uart0_queue, 0);

    xTaskCreate(rs232_task_rx, "uart_event_task", 3072, NULL, 2, NULL);
}

void rs232_api_init(void) { rs232_init_uart(); }

int8_t rs232_api_set_mode(uint8_t mode) {

    if (mode <= RS232_MODE_FINGERPRINT) {
        rs232_mode = mode;
        return ESP_OK;
    }

    return ESP_FAIL;
}

uint8_t rs232_api_get_mode(void) { return rs232_mode; }

void rs232_api_write(const uint8_t* data, size_t len) {
    // cloud_api_send( "[W] => " +data, len + sizeof("[W] => "));
    uart_write_bytes(RS232_UART_NUM, (char*)data, len);
}