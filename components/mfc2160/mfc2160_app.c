#include "mfc2160_app.h"

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mfc2160_command.h"
#include "mfc2160_enrollment.h"
#include "mfc2160_search.h"
#include "mfc2160_uart.h"

#define TAG "mfc2160_app"

esp_err_t mfc2160_app_init(void) {
    esp_err_t err = mfc2160_uart_init();
    ESP_LOGI(TAG, "err? %d", err);

    mfc2160_ps_result_t res = mfc2160_ps_read_sys_para();   
    
    if (res != PS_OK) {
        ESP_LOGE(TAG, "res? %d", res);
        err = ESP_FAIL;
    }

    // mfc2160_ps_empty();
    // uint16_t validNum;
    // mfc2160_ps_valid_template_num(&validNum);
    // ESP_LOGI(TAG, "Valid Num: %d", validNum);

    return err;
}

esp_err_t mfc2160_app_deinit(void) {
    mfc2160_uart_deinit();
    return ESP_OK;
}

esp_err_t mfc2160_app_enrollment(uint8_t *uid) {
    uint8_t id;
    esp_err_t ret = mfc2160_enrollment_start(&id);
    *uid = id;
    return ret;
}

esp_err_t mfc2160_app_empty_all(void) {
    return (mfc2160_ps_empty() == PS_OK) ? ESP_OK : ESP_FAIL;
}

esp_err_t mfc2160_app_empty_single(uint8_t indx) {
    return (mfc2160_ps_del_char(indx) == PS_OK) ? ESP_OK : ESP_FAIL;
}

esp_err_t mfc2160_app_search(uint16_t *uid) {    
    esp_err_t ret = mfc2160_search_start(uid);
    return ret;
}
