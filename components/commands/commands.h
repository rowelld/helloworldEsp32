#ifndef COMPONENTS_COMMANDS_COMMANDS
#define COMPONENTS_COMMANDS_COMMANDS

#include <stdint.h>
#include <stdlib.h>

typedef void(*commands_user_cb)(const char* rsp, size_t len);

void commands_init(void);

void commands_process(const char* commands, size_t len, commands_user_cb rsp_callback);
#endif /* COMPONENTS_COMMANDS_COMMANDS */
