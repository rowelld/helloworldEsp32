
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "sht4x.h"
#include "sgp40.h"
#include "air_quality_app.h"

static const char *TAG = "sgp40-example";

#define MUTEX_TIMEOUT_MS 50
#define MUTEX_LOCK() xSemaphoreTake(mutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS))
#define MUTEX_UNLOCK() xSemaphoreGive(mutex)

static air_quality_sensor_t cur_data;
static SemaphoreHandle_t mutex;
static uint8_t run_flag = 0;
static uint8_t data_ready = 0;

static const char *voc_index_name(int32_t voc_index)
{
    if (voc_index <= 0)
        return "INVALID VOC INDEX";
    else if (voc_index <= 10)
        return "unbelievable clean";
    else if (voc_index <= 30)
        return "extremely clean";
    else if (voc_index <= 50)
        return "higly clean";
    else if (voc_index <= 70)
        return "very clean";
    else if (voc_index <= 90)
        return "clean";
    else if (voc_index <= 120)
        return "normal";
    else if (voc_index <= 150)
        return "moderately polluted";
    else if (voc_index <= 200)
        return "higly polluted";
    else if (voc_index <= 300)
        return "extremely polluted";

    return "RUN!";
}

void sgp40_task(void *pvParams)
{
    sht4x_t sht;
    sgp40_t sgp;

    memset(&sht, 0, sizeof(sht));
    ESP_ERROR_CHECK(sht4x_init_desc(&sht, 0, SDA_GPIO, SCL_GPIO));
    ESP_ERROR_CHECK(sht4x_init(&sht));
    ESP_LOGI(TAG, "Humidity sensor Initialized.");

    memset(&sgp, 0, sizeof(sgp));
    ESP_ERROR_CHECK(sgp40_init_desc(&sgp, 0, SDA_GPIO, SCL_GPIO));
    ESP_ERROR_CHECK(sgp40_init(&sgp));
    ESP_LOGI(TAG, "SGP40 initialized. Serial 0x%04x%04x%04x", sgp.serial[0], sgp.serial[1], sgp.serial[2]);

    vTaskDelay(pdMS_TO_TICKS(250));

    TickType_t last_wakeup = xTaskGetTickCount();

    while (run_flag)
    {
        float temperature, humidity;
        int32_t voc_index;

        if (sht4x_measure(&sht, &temperature, &humidity) == ESP_OK)
        {
            
            if (sgp40_measure_voc(&sgp, humidity, temperature, &voc_index) == ESP_OK)
            {
                ESP_LOGI(TAG, "%.2f °C, %.2f %%, VOC index: %d, Air is [%s]",
                         temperature, humidity, voc_index, voc_index_name(voc_index));

                cur_data.relative_humidity = humidity;
                cur_data.voc_index = voc_index;
                cur_data.temperature = temperature;

                SGP40_VOC = voc_index;
                SHT4X_TEMP = temperature * 0.001f;
                SHT4X_HUM = humidity * 0.001f;
                data_ready = 1;
            }
        }

        vTaskDelayUntil(&last_wakeup, pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
}

esp_err_t air_quality_init(void)
{
    run_flag = 1;
    memset(&cur_data, 0, sizeof(cur_data));
    mutex = xSemaphoreCreateMutex();
    configASSERT(mutex != NULL);

    xTaskCreate(sgp40_task, "sgp40", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL);

    return ESP_OK;
}

esp_err_t air_quality_get_data(air_quality_sensor_t *sensor_data)
{
    if (data_ready) {
        MUTEX_LOCK();        
        memcpy(&sensor_data, &cur_data, sizeof(air_quality_sensor_t));
        MUTEX_UNLOCK();
        return ESP_OK;
    }

    return ESP_FAIL;
}