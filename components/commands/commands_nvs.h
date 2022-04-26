#ifndef COMPONENTS_COMMANDS_COMMANDS_NVS
#define COMPONENTS_COMMANDS_COMMANDS_NVS

#include <stdint.h>

typedef struct command_config
{
    uint8_t hash[32];    
} commands_config_t;

commands_config_t command_config;

void commands_get_config(commands_config_t* config);

void commands_set_password(const char* password);

#endif /* COMPONENTS_COMMANDS_COMMANDS_NVS */
