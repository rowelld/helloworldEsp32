#include <string.h>
#include <ctype.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "mbedtls/sha256.h"
#include "mbedtls/md.h"

#include "commands.h"
#include "commands_nvs.h"
#include "commands_parser.h"
#include "commands_list.h"

#define BUFFER_SIZE 128

typedef size_t (*commands_parser_cb)(char *in, char *out, void *arg);

typedef enum
{
    COMMANDS_OK = 0,
    COMMANDS_FAIL = -1,
    COMMANDS_AUTH_FAIL = -2,
} commands_err_t;

typedef struct
{
    char buffer[BUFFER_SIZE];
    commands_user_cb response_callback;
} command_evt_t;

static QueueHandle_t command_queue;
extern commands_config_t commands_config = {0};
static char response_buffer[1024];

static uint8_t compare_sha256(const char *password)
{
    uint8_t hash[32];
    mbedtls_sha256((uint8_t *)password, strlen(password), hash, 0);
    
    // int res = memcmp(hash, command_config.hash, 32);
    // esp_log_buffer_hex("hash-1", hash, 32);
    // esp_log_buffer_hex("hash-2", command_config.hash, 32);
    // ESP_LOGI("command", "comparison result %d", res);

    return (memcmp(hash, command_config.hash, 32) == 0) ? 1 : 0;
}

static size_t process_command(char *in, char *out)
{
    commands_err_t err = COMMANDS_FAIL;
    size_t len = 0;
    uint8_t param_start = 0;
    uint8_t cmd_start_pos = 2;

    char command_name[15] = {0};

    /* Check if its a valid command */
    if ('a' == tolower(in[0]) && ('t' == tolower(in[1])))
    {
        for (int i = 2; i < strlen(in); i++)
        {
            /* Find the start of parameter */
            if ((in[i] == '=') && !param_start)
            {
                /* Save the start of parameters offset */
                param_start = i;
            }

            /* End of line reached */
            if ((in[i] == '\0') || (in[i] == '\r') || (in[i] == '\n'))
            {
                in[i] = '\0';
                break;
            }
        }
        /* Proceed only if we have parameter start */
        if (param_start)
        {
            /* Copy the command name to our buffer */
            size_t command_name_len = &in[param_start] - &in[cmd_start_pos];
            strncpy(command_name, &in[cmd_start_pos], command_name_len);

            /* Get the hash index of the command from db */
            // uint16_t index = at_search(command_name);
            ESP_LOGI("commands-app", "Command Name: %s", command_name);
            commands_parser_cb parser = commands_list_search(command_name);

            /* If command is present in db */
            if (parser)
            {
                uint8_t pass_start_pos = param_start + 1;
                char *pass_ptr, *save_ptr;
                pass_ptr = strtok_r(&in[pass_start_pos], ",", &save_ptr);
                /* The first parameter is always the password */
                ESP_LOGI("commands-app", "pass %s", pass_ptr);

                if (pass_ptr && compare_sha256(pass_ptr))
                {
                    /* Authenticate command */
                    // char* param_ptr = strtok_r(NULL, ",", &save_ptr);
                    len = parser(save_ptr, out, NULL);
                    err = len ? COMMANDS_OK : COMMANDS_FAIL;
                }
                else
                {
                    err = COMMANDS_AUTH_FAIL;
                }
            }
            else
            {
                ESP_LOGE("commands", "no parser for %s", command_name);
            }
        }
        if (err == COMMANDS_FAIL)
        {
            len = sprintf(out, "ERROR");
        }
        else if (err == COMMANDS_AUTH_FAIL)
        {
            len = sprintf(out, "AUTH FAILED");
        }
    }

    return len;
}

static void commands_task(void *data)
{
    (void)data;
    while (1)
    {
        command_evt_t *event;
        if (xQueueReceive(command_queue, &event, portMAX_DELAY) == pdPASS)
        {
            size_t len = process_command((char *)event->buffer, response_buffer);

            if (event->response_callback && len)
            {
                event->response_callback(response_buffer, len);
            }

            free(event);
        }
    }
}

void commands_init(void)
{
    commands_get_config(&command_config);
    // esp_log_buffer_hex("commands", command_config.hash, 32);
    commands_list_init();
    command_queue = xQueueCreate(10, sizeof(void *));

    xTaskCreate(commands_task, "commands-task", 3096, NULL, 3, NULL);
}

void commands_process(const char *commands, size_t len, commands_user_cb rsp_callback)
{
    command_evt_t *event = (command_evt_t *)malloc(sizeof(command_evt_t));
    configASSERT(event != NULL);

    if (event)
    {
        size_t copy_length = len >= BUFFER_SIZE ? BUFFER_SIZE - 1 : len;
        memset(event->buffer, 0, sizeof(event->buffer));
        memcpy(event->buffer, commands, copy_length);
        event->response_callback = rsp_callback;

        if (xQueueSendToBack(command_queue, &event, pdMS_TO_TICKS(100)) != pdPASS)
        {
            free(event);
        }
    }
}