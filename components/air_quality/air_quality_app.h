#ifndef COMPONENTS_AIR_QUALITY_AIR_QUALITY_APP
#define COMPONENTS_AIR_QUALITY_AIR_QUALITY_APP

#include <stdint.h>

#include "esp_err.h"

#define SDA_GPIO 27
#define SCL_GPIO 14

extern int32_t SGP40_VOC;
extern float SHT4X_HUM;
extern float SHT4X_TEMP;

typedef struct 
{
    int32_t voc_index;
    int32_t relative_humidity;
    int32_t temperature;
} air_quality_sensor_t;

esp_err_t air_quality_init(void);

esp_err_t air_quality_get_data(air_quality_sensor_t *sensor_data);

#endif /* COMPONENTS_AIR_QUALITY_AIR_QUALITY_APP */
