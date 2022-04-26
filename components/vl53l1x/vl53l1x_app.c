#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "vl53l1x_app.h"

#define SDA_GPIO 27
#define SCL_GPIO 14

static const char *TAG = "vl53l1-app";

void vl53l1_task(void *pvParams)
{
    vl53l1x_t vl53l1;

    esp_err_t err;

    memset(&vl53l1, 0, sizeof(vl53l1));
    err = vl53l1x_init_desc(&vl53l1, 0, SDA_GPIO, SCL_GPIO);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "SUCCESS! %d", err);
        // ESP_ERROR_CHECK_WITHOUT_ABORT(vl53l1x_init(&vl53l1));
    } else {
        ESP_LOGE(TAG, "Init Error! %d", err);
    }

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    vTaskDelete(NULL);
}

esp_err_t vl53l1x_init_app(void)
{

    xTaskCreate(vl53l1_task, "vl53l1-task", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL);

    return ESP_OK;
}