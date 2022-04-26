#include "mfc2160_command.h"

#include "esp_crc.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "mfc2160_codes.h"
#include "mfc2160_uart.h"

#define CHECK_BIT(var, pos) (((var) >> (pos)) & 1)

static const char *TAG = "command";

static mfc2160_app_result_t finger_status(void) {
    if (mfc2160_ps_get_image() == PS_NO_FINGER) {
        return IR_FINGERINVALID;
    }
    return IR_FINGERVALID;
}

mfc2160_ps_result_t genchar(uint8_t bufId) {
    if (mfc2160_ps_gen_char(bufId) == PS_OK) {
        return PS_OK;
    }

    return PS_COMM_ERR;
}

uint32_t check_finger_valid(void) {
    uint32_t time = 0;

    while (1) {
        if (finger_status() == IR_FINGERVALID) {
            printf("Please remove finger...\r\n");
            continue;
        }

        time = esp_timer_get_time() / 1000000;
        printf("Please put your finger on...\r\n");
        while (1) {
            if (finger_status() == IR_FINGERVALID) {
                goto exit;
            }
        }
    }

exit:
    return time;
}

mfc2160_ps_result_t mfc2160_ps_valid_template_num(uint16_t *validNum) {
    mfc2160_ps_result_t ret;

    uint8_t cmd_pkg[] = {0xef, 0x01, 0xff, 0xff, 0xff, 0xff,
                         0x01, 0x00, 0x03, 0x1d, 0x00, 0x00};

    uint16_t sum = 0;
    for (int i = 6; i < sizeof(cmd_pkg); i++) sum += cmd_pkg[i];
    cmd_pkg[sizeof(cmd_pkg) - 1] = sum;
    cmd_pkg[sizeof(cmd_pkg) - 2] = sum >> 8;

    if(!mfc2160_uart_write_bytes((char *)cmd_pkg, sizeof(cmd_pkg))) {
        ret = PS_COMM_ERR;
    }

    uint8_t ret_pkg[32];

    size_t len = mfc2160_uart_read_bytes((uint8_t *)&ret_pkg, sizeof(ret_pkg));

    if(len > 0) {
        esp_log_buffer_hex(TAG, ret_pkg, len);
        ret = ret_pkg[9];
        *validNum = ret_pkg[10] << 8 | ret_pkg[11];
    } else {
        ret = PS_COMM_ERR;
    }

    return ret;
}

mfc2160_ps_result_t mfc2160_ps_read_sys_para(void) {
    mfc2160_ps_result_t ret;
    static uint8_t buffer[] = {0xef, 0x01, 0xff, 0xff, 0xff, 0xff,
                               0x01, 0x00, 0x03, 0x0f, 0x00, 0x00};

    uint16_t sum = 0;

    for (uint8_t i = 6; i < sizeof(buffer); i++) {
        sum += buffer[i];
    }

    buffer[sizeof(buffer) - 1] = sum;
    buffer[sizeof(buffer) - 2] = sum >> 8;

    if (ESP_FAIL == mfc2160_uart_write_bytes((char *)buffer, sizeof(buffer))) {
        ESP_LOGE(TAG, "COMM Error!");
        return PS_COMM_ERR;
    }

    uint8_t ret_pkg[64];

    size_t len = mfc2160_uart_read_bytes((uint8_t *)&ret_pkg, sizeof(ret_pkg));

    vTaskDelay(pdMS_TO_TICKS(1000));

    if (len == 0) {
        ESP_LOGE(TAG, "len? %d", len);
        ret = PS_COMM_ERR;
    } else {
        
        esp_log_buffer_hex(TAG, ret_pkg, len);
        ret = ret_pkg[9];
    }

    return ret;
}

mfc2160_ps_result_t mfc2160_ps_read_index_table(uint8_t *indexTable) {
    mfc2160_ps_result_t ret = PS_OK;
    uint8_t table_index = 0;

    for (int x = 0; x < 2; x++) {
        uint8_t buffer[] = {0xef, 0x01, 0xff, 0xff, 0xff, 0xff, 0x01,
                            0x00, 0x04, 0x1f, x,    0x00, 0x00};

        uint16_t sum = 0;
        for (uint8_t i = 6; i < sizeof(buffer); i++) {
            sum += buffer[i];
        }

        buffer[sizeof(buffer) - 1] = sum;
        buffer[sizeof(buffer) - 2] = sum >> 8;

        if (!mfc2160_uart_write_bytes((char *)buffer, sizeof(buffer))) {
            return PS_COMM_ERR;
        }

        uint8_t ret_pkg[64];

        size_t len =
            mfc2160_uart_read_bytes((uint8_t *)&ret_pkg, sizeof(ret_pkg));

        if (len == 0) {
            ret = PS_COMM_ERR;
        } else {
            // ESP_LOGI(TAG, "Index Table! %d", x);
            esp_log_buffer_hex(TAG, ret_pkg, len);
            ret = PS_OK;
            for (uint8_t i = 10; i < len - 2; i++) {
                for (uint8_t pos = 0; pos < 8; pos++) {
                    indexTable[table_index++] = CHECK_BIT(ret_pkg[i], pos);
                }
                if (table_index >= 100) break;
            }
        }
        // vTaskDelay(pdMS_TO_TICKS(500));
    }

    return ret;
}

mfc2160_ps_result_t mfc2160_ps_get_image(void) {
    mfc2160_ps_result_t ret;
    uint8_t cmd_pkg[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
                           0x01, 0x00, 0x03, 0x01, 0x00, 0x05};

    if (!mfc2160_uart_write_bytes((char *)cmd_pkg, sizeof(cmd_pkg))) {
        ret = PS_COMM_ERR;
    }

    uint8_t ret_pkg[64];

    uint8_t len = mfc2160_uart_read_bytes((uint8_t *)&ret_pkg, sizeof(ret_pkg));

    if (len == 0) {
        ret = PS_COMM_ERR;
    } else {
        // esp_log_buffer_hex(TAG, ret_pkg, len);
        ret = ret_pkg[9];
    }

    return ret;
}



mfc2160_ps_result_t mfc2160_ps_gen_char(uint8_t bufID) {
    mfc2160_ps_result_t ret;
    uint8_t cmd_pkg[] = {0xEF, 0x01, 0xFF, 0xFF,  0xFF, 0xFF, 0x01,
                         0x00, 0x04, 0x02, bufID, 0x00, 0x00};

    uint16_t sum = 0;
    for (int i = 6; i < sizeof(cmd_pkg); i++) {
        sum += cmd_pkg[i];
    }
    cmd_pkg[sizeof(cmd_pkg) - 1] = sum;
    cmd_pkg[sizeof(cmd_pkg) - 2] = sum >> 8;

    esp_log_buffer_hex(TAG, cmd_pkg, sizeof(cmd_pkg));

    if (!mfc2160_uart_write_bytes((char *)cmd_pkg, sizeof(cmd_pkg))) {
        ESP_LOGE(TAG, "genchar uart problem");
        ret = PS_COMM_ERR;
    }

    uint8_t ret_pkg[64];

    vTaskDelay(pdMS_TO_TICKS(300));

    uint8_t len = mfc2160_uart_read_bytes((uint8_t *)&ret_pkg, sizeof(ret_pkg));

    if (len == 0) {
        ret = PS_COMM_ERR;
    } else {
        // esp_log_buffer_hex(TAG, ret_pkg, len);
        ret = ret_pkg[9];
    }

    return ret;
}

mfc2160_ps_result_t mfc2160_ps_reg_model(void) {
    mfc2160_ps_result_t ret;
    uint8_t cmd_pkg[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
                         0x01, 0x00, 0x03, 0x05, 0x00, 0x9};

    if (!mfc2160_uart_write_bytes((char *)cmd_pkg, sizeof(cmd_pkg))) {
        ret = PS_COMM_ERR;
    }

    uint8_t ret_pkg[64];

    vTaskDelay(pdMS_TO_TICKS(500));

    uint8_t len = mfc2160_uart_read_bytes((uint8_t *)&ret_pkg, sizeof(ret_pkg));

    if (len == 0) {
        ESP_LOGE(TAG, "reg_model read: %d", len);
        ret = PS_COMM_ERR;
    } else {
        esp_log_buffer_hex(TAG, ret_pkg, len);
        ret = ret_pkg[9];
    }

    return ret;
}

mfc2160_ps_result_t mfc2160_ps_load_char(uint8_t bufId, uint8_t uID) {
    uint8_t cmd_pkg[] = {0xef, 0x01, 0xff,  0xff,     0xff, 0xff, 0x01, 0x00,
                         0x06, 0x07, bufId, uID >> 8, uID,  0x00, 0x00};

    uint16_t sum = 0;
    for (int i = 6; i < sizeof(cmd_pkg); i++) sum += cmd_pkg[i];
    cmd_pkg[sizeof(cmd_pkg) - 1] = sum;
    cmd_pkg[sizeof(cmd_pkg) - 2] = sum >> 8;

    if (!mfc2160_uart_write_bytes((char *)cmd_pkg, sizeof(cmd_pkg))) {
        return PS_COMM_ERR;
    }
    uint8_t ret_pkg[32];

    uint8_t len = mfc2160_uart_read_bytes((uint8_t *)&ret_pkg, sizeof(ret_pkg));

    if (len == 0) {
        return PS_COMM_ERR;
    }

    return PS_OK;
}

mfc2160_ps_result_t mfc2160_ps_store_char(uint8_t uId) {
    mfc2160_ps_result_t res;
    uint8_t cmd_pkg[] = {0xEF, 0x01, 0xFF, 0xFF,     0xFF, 0xFF, 0x01, 0x00,
                         0x06, 0x06, 0x01, uId >> 8, uId,  0x00, 0x00};

    uint16_t sum = 0;
    for (int i = 6; i < sizeof(cmd_pkg); i++) {
        sum += cmd_pkg[i];
    }
    cmd_pkg[sizeof(cmd_pkg) - 1] = sum;
    cmd_pkg[sizeof(cmd_pkg) - 2] = sum >> 8;

    if (!mfc2160_uart_write_bytes((char *)cmd_pkg, sizeof(cmd_pkg))) {
        res = PS_COMM_ERR;
    }
    uint8_t ret_pkg[64];

    vTaskDelay(pdMS_TO_TICKS(500));

    uint8_t len = mfc2160_uart_read_bytes((uint8_t *)&ret_pkg, sizeof(ret_pkg));

    if (len == 0) {
        res = PS_COMM_ERR;
    } else {
        esp_log_buffer_hex(TAG, ret_pkg, len);
        res = ret_pkg[9];
    }
    return res;
}

mfc2160_ps_result_t mfc2160_ps_cancel(void) {
    mfc2160_ps_result_t res;
    
    static uint8_t cmd_pkg[] = {0xef, 0x01, 0xff, 0xff, 0xff, 0xff,
                                0x01, 0x00, 0x03, 0x30, 0x00, 0x34};

    if (!mfc2160_uart_write_bytes((char *)cmd_pkg, sizeof(cmd_pkg))) {
        res = PS_COMM_ERR;
    }

    uint8_t ret_pkg[64];

    uint8_t len = mfc2160_uart_read_bytes((uint8_t *)&ret_pkg, sizeof(ret_pkg));

    if (len > 0) {
        esp_log_buffer_hex(TAG, ret_pkg, len);
        res = ret_pkg[9];
    } else {
        res = PS_COMM_ERR;
    }

    return res;
}

mfc2160_ps_result_t mfc2160_ps_search(uint16_t *uId, uint16_t *score) {
    uint16_t startID = 0x00, endID = 0x63;
    uint8_t cmd_pkg[] = {0xEF,    0x01,       0xFF,  0xFF, 0xFF, 0xFF,
                         0x01,    0x00,       0x08,  0x04, 0x01, startID >> 8,
                         startID, endID >> 8, endID, 0x00, 0x00};

    uint16_t sum = 0;
    for (int i = 6; i < sizeof(cmd_pkg); i++) sum += cmd_pkg[i];
    cmd_pkg[sizeof(cmd_pkg) - 1] = sum;
    cmd_pkg[sizeof(cmd_pkg) - 2] = sum >> 8;

    if (!mfc2160_uart_write_bytes((char *)cmd_pkg, sizeof(cmd_pkg))) {
        ESP_LOGE(TAG, "uart_write_bytes");
        return PS_COMM_ERR;
    }

    uint8_t ret_pkg[64];

    vTaskDelay(pdMS_TO_TICKS(500));

    uint8_t len = mfc2160_uart_read_bytes((uint8_t *)&ret_pkg, sizeof(ret_pkg));

    if (len > 0) {
        esp_log_buffer_hex(TAG, ret_pkg, len);
        *uId = ret_pkg[10] << 8 | ret_pkg[11];
        *score = ret_pkg[12] << 8 | ret_pkg[13];

        return ret_pkg[9];
    } else {
        return PS_COMM_ERR;
    }

    return PS_OK;
}

mfc2160_ps_result_t mfc2160_ps_empty(void) {
    uint8_t cmd_pkg[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
                         0x01, 0x00, 0x03, 0x0D, 0x00, 0x11};

    if (!mfc2160_uart_write_bytes((char *)cmd_pkg, sizeof(cmd_pkg))) {
        return PS_COMM_ERR;
    }

    uint8_t ret_pkg[64];

    uint8_t len = mfc2160_uart_read_bytes((uint8_t *)ret_pkg, sizeof(ret_pkg));

    if (len > 0) {
        esp_log_buffer_hex(TAG, ret_pkg, len);
        return ret_pkg[9];
    } else {
        return PS_COMM_ERR;
    }

    return PS_OK;
}

mfc2160_ps_result_t mfc2160_ps_del_char(uint8_t uId) {
    int delCnt = 1;
    uint8_t cmd_pkg[] = {0xEF,        0x01,   0xFF, 0xFF, 0xFF,     0xFF,
                         0x01,        0x00,   0x07, 0x0C, uId >> 8, uId,
                         delCnt >> 8, delCnt, 0x00, 0x00};

    uint16_t sum = 0;
    for (int i = 6; i < sizeof(cmd_pkg); i++) sum += cmd_pkg[i];
    cmd_pkg[sizeof(cmd_pkg) - 1] = sum;
    cmd_pkg[sizeof(cmd_pkg) - 2] = sum >> 8;

    if (!mfc2160_uart_write_bytes((char *)cmd_pkg, sizeof(cmd_pkg))) {
        return PS_COMM_ERR;
    }

    uint8_t ret_pkg[64];
    uint8_t len = mfc2160_uart_read_bytes((uint8_t *)ret_pkg, sizeof(ret_pkg));

    if (len > 0) {
        esp_log_buffer_hex(TAG, ret_pkg, len);
        return ret_pkg[9];
    } else {
        return PS_COMM_ERR;
    }

    return PS_OK;
}

mfc2160_ps_result_t mfc2160_ps_up_char(uint8_t buffID) {
    uint8_t cmd_pkg[] = {0xEF, 0x01, 0xFF, 0xFF,   0xFF, 0xFF, 0x01,
                         0x00, 0x04, 0x08, buffID, 0x00, 0x00};

    uint16_t sum = 0;
    for (int i = 6; i < sizeof(cmd_pkg); i++) sum += cmd_pkg[i];
    cmd_pkg[sizeof(cmd_pkg) - 1] = sum;
    cmd_pkg[sizeof(cmd_pkg) - 2] = sum >> 8;

    if (!mfc2160_uart_write_bytes((char *)cmd_pkg, sizeof(cmd_pkg))) {
        return PS_COMM_ERR;
    }

    uint8_t ret_pkg[12];
    uint8_t feature_pkg[43];
    uint8_t len = mfc2160_uart_read_bytes((uint8_t *)ret_pkg, sizeof(ret_pkg));
    ESP_LOGI(TAG, "len %d", len);

    if (len > 0) {
        esp_log_buffer_hex(TAG, ret_pkg, len);

        len = mfc2160_uart_read_bytes((uint8_t *)feature_pkg,
                                      sizeof(feature_pkg));
        uint8_t count = 1;

        while (len > 0) {
            ESP_LOGI(TAG, "Feature len: %d Count: %d", len, count++);
            esp_log_buffer_hex(TAG, feature_pkg, len);
            len = mfc2160_uart_read_bytes((uint8_t *)feature_pkg,
                                          sizeof(feature_pkg));
        }

        return ret_pkg[9];
    } else {
        return PS_COMM_ERR;
    }

    return PS_OK;
}