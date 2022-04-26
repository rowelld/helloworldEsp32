#ifndef COMPONENTS_COMMANDS_COMMANDS_PARSER
#define COMPONENTS_COMMANDS_COMMANDS_PARSER

#include <stdint.h>

size_t command_parser_restart(char *param, char *out, void *arg);

size_t command_parser_cloud(char *param, char *out, void *arg);

size_t command_parser_security(char *param, char *out, void *arg);

size_t command_parser_enroll(char *param, char *out, void *arg);

size_t command_parser_search(char *param, char *out, void *arg);

size_t command_parser_remove(char *param, char *out, void *arg);

size_t command_parser_cancel(char *param, char *out, void *arg);

size_t command_parser_ota_start(char *param, char *out, void *arg);

size_t command_parser_ota_url(char *param, char *out, void *arg);

#endif /* COMPONENTS_COMMANDS_COMMANDS_PARSER */
