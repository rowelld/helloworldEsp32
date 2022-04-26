#include "mfc2160_enrollment.h"

#include <string.h>

#include "esp_log.h"
#include "mfc2160_codes.h"
#include "mfc2160_command.h"

uint8_t userID;

#define TAG "enrollment"

static void update_UID(void) {
    uint8_t indexTable[100] = {0};
    mfc2160_ps_read_index_table(indexTable);
    esp_log_buffer_hex(TAG, indexTable, sizeof(indexTable));

    for (int i = 0; i < sizeof(indexTable); i++) {
        if (indexTable[i] == 0) {
            userID = i;
            break;
        }
    }
}

static mfc2160_app_result_t imd_enroll(uint16_t uid, uint8_t bFirst) {
    ESP_LOGI(TAG, "Enroll %d", uid);
    static uint8_t enrollCnt = 0;
    if (bFirst) {
        enrollCnt = 0;
    }

    uint8_t bufID = enrollCnt + 1;
    if (genchar(bufID) != PS_OK) {
        ESP_LOGE(TAG, "genchar failed");
        return IR_GENCHARFAIL;
    }

    enrollCnt++;
    ESP_LOGI(TAG, "enroll count: %d", enrollCnt);

    if (enrollCnt == 5) {
        if (mfc2160_ps_reg_model() != PS_OK) {
            ESP_LOGE(TAG, "reg_model failed...");
            return IR_FAIL;
        }

        if (mfc2160_ps_store_char(uid) != PS_OK) {
            ESP_LOGE(TAG, "store_char failed...");
            return IR_FAIL;
        }

        return IR_END;
    }
    return IR_ACCEPT;
}



esp_err_t mfc2160_enrollment_start(uint8_t *id) {
    esp_err_t ret = ESP_OK;
    update_UID();

    ESP_LOGI(TAG, "UID is %d", userID);

    for (int i = 0;; i++) {
        printf("Fingerprint sampling progress: %d \r\n", i + 1);

        check_finger_valid();
        mfc2160_app_result_t iR = imd_enroll(userID, i == 0 ? 1 : 0);

        if (iR == IR_END) {
            printf("Fingerprint enroll success! %d\r\n", userID);
            ret = ESP_FAIL;
            *id = userID;
            break;
        }
        if (iR == IR_FAIL) {
            printf("FingerPrint enroll fail! \r\n");
            ret = ESP_FAIL;
            break;
        }
    }

    update_UID();
    return ret;
}