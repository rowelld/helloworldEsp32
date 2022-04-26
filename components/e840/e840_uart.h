#ifndef COMPONENTS_E840_E840_UART
#define COMPONENTS_E840_E840_UART

#include <stdint.h>
#include "driver/gpio.h"

#define UART_BAUDRATE       115200
#define UART_NUM            UART_NUM_1
#define TX_PIN              25
#define RX_PIN              26

esp_err_t e840_uart_init(void);

esp_err_t e840_uart_deinit(void);

esp_err_t e840_uart_write_bytes(const char *bytes, size_t len);

size_t e840_uart_read_bytes(uint8_t *buffer, size_t len);

size_t e840_uart_get_rx_length(void);

#endif /* COMPONENTS_E840_E840_UART */
