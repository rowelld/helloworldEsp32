#ifndef COMPONENTS_WIFI_NAT_HTTP_SERVER
#define COMPONENTS_WIFI_NAT_HTTP_SERVER

#include <esp_http_server.h>
#include "wifi_nat.h"

httpd_handle_t start_webserver(wifi_nat_config_t *config);

void stop_webserver(httpd_handle_t server);

#endif /* COMPONENTS_WIFI_NAT_HTTP_SERVER */
