#include <string.h>

#include "VL53L1X_api.h"
#include "vl53l1_platform.h"
#include "VL53L1X_calibration.h"

#include "esp_log.h"

#include "vl53l1.h"

#define TAG "vl53l1"

static uint8_t run_flag = 0;
static vl53l1_app_cb user_callbacks = NULL;

static int center[2] = {167, 231}; /* these are the spad center of the 2 8*16 zones */
static int Zone = 0;
static int PplCounter = 0;
static uint8_t isInside = 0;

int16_t max_threshold;

#if 0
static int process_people_counting_data(int16_t distance, uint8_t zone)
{
    static int PathTrack[] = {0, 0, 0, 0};
    static int PathTrackFillingSize = 1;
    static int LeftPreviousStatus = NOBODY;
    static int RightPreviousStatus = NOBODY;
    static int PeopleCount = 0;

    int CurrentZoneStatus = NOBODY;
    int AllZoneCurrentStatus = 0;
    int AneventHasOccured = 0;

    if (distance < max_threshold)
    {
        CurrentZoneStatus = SOMEONE;
    }

    if (zone == LEFT)
    {
        if (CurrentZoneStatus != LeftPreviousStatus)
        {
            AneventHasOccured = 1;
            if (CurrentZoneStatus == SOMEONE)
            {
                AllZoneCurrentStatus += 1;
            }

            if (RightPreviousStatus == SOMEONE)
            {
                AllZoneCurrentStatus += 2;
            }
            LeftPreviousStatus = CurrentZoneStatus;
        }
    }
    else
    {
        if (CurrentZoneStatus != RightPreviousStatus)
        {
            AneventHasOccured = 1;
            if (CurrentZoneStatus == SOMEONE)
            {
                AllZoneCurrentStatus += 2;
            }
            if (LeftPreviousStatus == SOMEONE)
            {
                AllZoneCurrentStatus += 1;
            }

            RightPreviousStatus = CurrentZoneStatus;
        }
    }

    if (AneventHasOccured)
    {
        if (PathTrackFillingSize < 4)
        {
            PathTrackFillingSize++;
        }

        if ((LeftPreviousStatus == NOBODY) && (RightPreviousStatus == NOBODY))
        {
            if (PathTrackFillingSize == 4)
            {
                ESP_LOGI(TAG, "%d, %d, %d, %d", PathTrack[0], PathTrack[1], PathTrack[2], PathTrack[3]);

                if ((PathTrack[1] == 1) && (PathTrack[2] == 3) && (PathTrack[3] == 2))
                {
                    PeopleCount++;
                }
                else if ((PathTrack[1] == 2) && (PathTrack[2] == 3) && (PathTrack[3] == 1))
                {
                    PeopleCount--;
                }
            }
            PathTrackFillingSize = 1;
        }
        else
        {
            PathTrack[PathTrackFillingSize - 1] = AllZoneCurrentStatus;
        }
    }

    return (PeopleCount);
}


static void vl53l1x_app_task(void *arg)
{
    uint16_t Distance;
    uint8_t RangeStatus;
    uint8_t dataReady;

    uint8_t status = 0;

    while (run_flag)
    {
        while (dataReady == 0)
        {
            status = VL53L1X_CheckForDataReady(dev, &dataReady);
            vTaskDelay(2 / portTICK_RATE_MS);
        }

        dataReady = 0;
        status += VL53L1X_GetRangeStatus(dev, &RangeStatus);
        status += VL53L1X_GetDistance(dev, &Distance);
        status += VL53L1X_ClearInterrupt(dev);

        if (status != 0)
        {
            ESP_LOGE(TAG, "Error in operating the device.");
        }

// #ifdef PEOPLE_COUNTING_MODE
#if 0
        status = VL53L1X_SetROICenter(dev, center[Zone]);
        if (status != 0)
        {
            ESP_LOGE(TAG, "Error in changing the center of the ROI.\n");
        }

        PplCounter = process_people_counting_data(Distance, Zone);

        Zone++;
        Zone = Zone % 2;

        ESP_LOGI(TAG, "Zone: %d, RangeStatus: %d, Distance: %d, Count: %d", Zone, RangeStatus, Distance, PplCounter);
        ESP_LOGI(TAG, "Distance: %d", Distance);
// #else
#endif

        if (Distance <= max_threshold && isInside == 0)
        {
            isInside = 1;
            vl53l1_event_t event = {0};
            event.id = EVT_THRESHOLD_INSIDE;
            event.distance = Distance;
            user_callbacks(&event);
        }
        else if (Distance > max_threshold && isInside == 1)
        {
            isInside = 0;
            vl53l1_event_t event = {0};
            event.id = EVT_THRESHOLD_OUTSIDE;
            event.distance = Distance;
            user_callbacks(&event);
            
        }


        vTaskDelay(250 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
    VL53L1X_StopRanging(dev);
}

#endif

void vl53l1_stop_app(void)
{
    run_flag = 0;
}

void vl53l1_start_app(void)
{
    run_flag = 1;
    // xTaskCreate(vl53l1x_app_task, "vl53l1-task", 2048, NULL, 5, NULL);
}

uint8_t vl53l1_init(void)
{
    i2c_dev_t dev;
    memset(&dev, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(VL53L1X_InitDesc(&dev, 0x29, 0, 27, 14));

    uint8_t bytedata, status, sensorState = 0;

    status = VL53L1_RdByte(&dev, 0x010F, &bytedata);
    ESP_LOGI(TAG, "VL53L1 Model ID: %X", bytedata);

    while (sensorState == 0)
    {
        status = VL53L1X_BootState(&dev, &sensorState);
        vTaskDelay(200 / portTICK_RATE_MS);
    }

    // gpio_set_direction(GPIO_NUM_32, GPIO_MODE_OUTPUT);
    // gpio_set_level(GPIO_NUM_32, 0);

    // max_threshold = threshold;
    // user_callbacks = app_cb;

    // dev = 0x29;

    // // gpio_set_level(GPIO_NUM_32, 1);

    // uint8_t byteData, sensorState = 0;
    // uint8_t status;
    // uint16_t wordData;

    // status = VL53L1_RdByte(dev, 0x010F, &byteData);
    // // ESP_LOGI(TAG, "Vl53L1 Model ID: %X", byteData);
    // status = VL53L1_RdByte(dev, 0x0110, &byteData);
    // // ESP_LOGI(TAG, "Vl53L1 Module Type: %X", byteData);
    // status = VL53L1_RdWord(dev, 0x010F, &wordData);
    // // ESP_LOGI(TAG, "Vl53L1: %X", wordData);

    // while (sensorState == 0)
    // {
    //     status = VL53L1X_BootState(dev, &sensorState);
    //     vTaskDelay(200 / portTICK_RATE_MS);
    // }

    // // ESP_LOGI(TAG, "Chip booted");

    // status = VL53L1X_SensorInit(dev);
    // // ESP_LOGI(TAG, "Sensor Init %d", status);
    // status += VL53L1X_SetDistanceMode(dev, 1);
    // // ESP_LOGI(TAG, "Set Distance Mode %d", status);
    // status += VL53L1X_SetTimingBudgetInMs(dev, 33);
    // // ESP_LOGI(TAG, "Set Timing Budget %d", status);
    // status = VL53L1X_SetInterMeasurementInMs(dev, 100);

    // if (status != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Error in Initialization or configuration of the device");
    // }

    // VL53L1X_StartRanging(dev);

    return 0;
}