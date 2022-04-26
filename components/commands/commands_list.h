#ifndef COMPONENTS_COMMANDS_COMMANDS_LIST
#define COMPONENTS_COMMANDS_COMMANDS_LIST

#include <stdint.h>
#include <stdlib.h>

typedef size_t (*commands_parser_cb)(char *in, char *out, void *arg);

void commands_list_init(void);

commands_parser_cb commands_list_search(const char *command);

#endif /* COMPONENTS_COMMANDS_COMMANDS_LIST */
