#ifndef COMPONENTS_MFC2160_MFC2160_UART
#define COMPONENTS_MFC2160_MFC2160_UART

#include <stdint.h>
#include "driver/gpio.h"

#define UART_BAUDRATE 57600
#define UART_NUM UART_NUM_1
#define TX_PIN 25 //GPIO_NUM_19 //25
#define RX_PIN 26 //GPIO_NUM_18 //26
// #define VDD_PIN GPIO_NUM_13 

esp_err_t mfc2160_uart_init(void);

esp_err_t mfc2160_uart_deinit(void);

esp_err_t mfc2160_uart_write_bytes(const char *bytes, size_t len);

size_t mfc2160_uart_read_bytes(uint8_t *buffer, size_t len);

size_t mfc2160_uart_get_rx_lenght(void);

#endif /* COMPONENTS_MFC2160_MFC2160_UART */
