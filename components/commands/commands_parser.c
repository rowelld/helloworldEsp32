#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include <string.h>
#include <stdio.h>

#include "esp_log.h"
#include "esp_system.h"

#include "cloud_api.h"
#include "commands_nvs.h"
#include "commands_parser.h"
#include "mfc2160_app.h"
#include "mfc2160_command.h"
#include "fota.h"

#define SUCCESS_STR "OK"
#define ERROR_STR "ERROR"
#define MAX_SCAN_LIST 10

#define TAG "command-parser"

typedef struct
{
    uint8_t used;
    char name[23];
    uint8_t bda[6];
} scan_db_t;

static TimerHandle_t xRestartTimer = NULL;
// static scan_db_t scan_db[MAX_SCAN_LIST];

static void restart_timer_cb(void *data)
{
    esp_restart();
}


size_t command_parser_restart(char *param, char *out, void *arg)
{
    (void)param;

    xRestartTimer = xTimerCreate("restart-tmr", pdMS_TO_TICKS(5000), pdFALSE, NULL, restart_timer_cb);
    if (xRestartTimer)
    {
        xTimerStart(xRestartTimer, 0);
    }

    return sprintf(out, SUCCESS_STR);
}

size_t command_parser_cloud(char *param, char *out, void *arg)
{
    if (!param) {
        cloud_config_t config;
        cloud_api_get_config(&config);
        return sprintf(out, "+CLOUD:%d,%s,%d\r\nOK", config.conntype, config.host, config.port);
    }

    char *token[3], *save_ptr;
    token[0] = strtok_r(param, ",", &save_ptr);
    token[1] = strtok_r(NULL, ",", &save_ptr);
    token[2] = strtok_r(NULL, ",", &save_ptr);

    if (token[0] && token[1] && token[2])
    {
        cloud_config_t config;
        config.conntype = strtoul(token[0], NULL, 10);
        strcpy(config.host, token[1]);
        config.port = strtoul(token[2], NULL, 10);
        cloud_api_set_config(&config);

        xRestartTimer = xTimerCreate("restart-tmr", pdMS_TO_TICKS(5000), pdFALSE, NULL, restart_timer_cb);
        if (xRestartTimer) {
            xTimerStart(xRestartTimer, 0);
        }
        return sprintf(out, SUCCESS_STR);
    }

    return sprintf(out, ERROR_STR);
}

size_t command_parser_security(char *param, char *out, void *arg)
{
    if (!param) {
        return sprintf(out, ERROR_STR);
    }
    commands_set_password(param);
    return sprintf(out, SUCCESS_STR);
}

size_t command_parser_enroll(char *param, char *out, void *arg) {
    uint8_t id = 0;
    esp_err_t ret = mfc2160_app_enrollment(&id);
    if (ret == ESP_OK) {
        return sprintf(out, "OK\r\n%d", id);
    } else {
        return sprintf(out, ERROR_STR);
    }       
}

size_t command_parser_search(char *param, char *out, void *arg) {    
    uint16_t id = 0;
    esp_err_t ret = mfc2160_app_search(&id);

    if(ret != ESP_OK) {
        return sprintf(out, ERROR_STR);
    } else {        
        return sprintf(out,  "OK\r\n%d", id);
    }    
}

size_t command_parser_remove(char *param, char *out, void *arg) {
    uint8_t uID = strtoul(param, NULL, 10);

    if (mfc2160_app_empty_single(uID) == ESP_OK)
        return sprintf(out, SUCCESS_STR);
    else
        return sprintf(out, ERROR_STR);
}

size_t command_parser_cancel(char *param, char *out, void *arg) {

    if (mfc2160_ps_cancel() != PS_OK) {
        return sprintf(out, ERROR_STR);
    }

    return sprintf(out, SUCCESS_STR);
}

size_t command_parser_ota_start(char *param, char *out, void *arg) {
    if(!param) {
        fota_start();
        return sprintf(out, SUCCESS_STR);
    }

    return sprintf(out, "ERROR");
}

size_t command_parser_ota_url(char *param, char *out, void *arg) {
    if (!param) {
        return sprintf(out, "+OTAURL:%s", fota_get_url());
    }
    fota_set_url(param);

    return sprintf(out, SUCCESS_STR);
}