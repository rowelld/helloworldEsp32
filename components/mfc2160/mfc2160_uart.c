#include "mfc2160_uart.h"

#include "driver/uart.h"
#include "esp_log.h"

#define TAG "mfc2160"
#define PACKET_READ_TICS 35

esp_err_t mfc2160_uart_init(void) {
    uart_config_t config = {
        .baud_rate = UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_2,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    if (uart_driver_install(UART_NUM, 1024, 0, 0, NULL, 0) != ESP_OK) {
        ESP_LOGE(TAG, "install uart driver failed.");
        return ESP_FAIL;
    }

    if (uart_param_config(UART_NUM, &config) != ESP_OK) {
        ESP_LOGE(TAG, "config uart error!");
        return ESP_FAIL;
    }

    if (uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE,
                     UART_PIN_NO_CHANGE) != ESP_OK) {
        ESP_LOGE(TAG, "config uart pin failed!");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t mfc2160_uart_deinit(void) {
    uart_driver_delete(UART_NUM);
    return ESP_OK;
}

esp_err_t mfc2160_uart_write_bytes(const char *bytes, size_t len) {
    esp_err_t err = uart_write_bytes(UART_NUM, bytes, len);
    return err;
}

size_t mfc2160_uart_read_bytes(uint8_t *buffer, size_t len) {
    return uart_read_bytes(UART_NUM, buffer, len, 100 / portTICK_PERIOD_MS);
}

size_t mfc2160_uart_get_rx_lenght(void) {
    size_t rx_len = 0;
    uart_get_buffered_data_len(UART_NUM, &rx_len);

    return rx_len;
}