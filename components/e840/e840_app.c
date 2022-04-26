
#include "e840_app.h"

#include <string.h>

#include "e840_uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static uint8_t run_flag = 0;
static unsigned char buffer[128];

#define TAG "e840_app"
#define delay_ms(__ms) vTaskDelay(pdMS_TO_TICKS(__ms))

static void e840_app_task(void *pData) {
    while (run_flag) {
        memset(buffer, 0, sizeof(buffer));

        size_t len = e840_uart_read_bytes((uint8_t *)buffer, sizeof(buffer));

        if (len > 0) {
            ESP_LOGI(TAG, "Scan Len %d", len);
            ESP_LOGI(TAG, "%.*s", len, buffer);
        }
    }
}

esp_err_t e840_app_init(void) {
    esp_err_t err = e840_uart_init();
    return err;
}

esp_err_t e840_app_start(void) {
    run_flag = 1;
    xTaskCreate(e840_app_task, "e840-task", 1024 * 4, NULL, 5, NULL);

    delay_ms(3000);

    e840_uart_write_bytes("+++", sizeof("+++"));

    e840_uart_write_bytes("AT+CPIN\r\n", sizeof("AT+CPIN\r\n"));

    return ESP_OK;
}

esp_err_t e840_app_deinit(void) {
    e840_uart_deinit();
    return ESP_OK;
}