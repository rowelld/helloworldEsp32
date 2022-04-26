#include "mfc2160_search.h"

#include "mfc2160_codes.h"
#include "mfc2160_command.h"
#include "esp_log.h"

#define TAG "search"

static mfc2160_app_result_t search(uint16_t *uID) {
    if (genchar(1) != PS_OK) return IR_NOMATCH;

    uint16_t score = 0;
    if (mfc2160_ps_search(uID, &score) != PS_OK) {        
        return IR_NOMATCH;
    }

    ESP_LOGI("search", "%d", score);

    return IR_MATCHFINGER;
}

esp_err_t mfc2160_search_start(uint16_t *id) {
    esp_err_t ret = ESP_FAIL;
    uint8_t count = 0;
    uint16_t user_id;

    while (count < 3) {
        check_finger_valid();
        mfc2160_app_result_t search_res = search(&user_id);

        if (search_res == IR_MATCHFINGER) {
            // printf("Match OK. ID=%d\r\n", (uint16_t)id);
            ESP_LOGI(TAG, "Match OK. ID=%d", (uint16_t)user_id);
            *id = user_id;
            ret = ESP_OK;
            break;
        } else {
            printf("No Match\r\n");
        }
        count++;
    }

    return ret;
}