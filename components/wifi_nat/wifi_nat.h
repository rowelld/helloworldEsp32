#ifndef COMPONENTS_WIFI_NAT_WIFI_NAT
#define COMPONENTS_WIFI_NAT_WIFI_NAT

#include <stdio.h>
#include <string.h>

#define DEFAULT_AP_IP "192.168.4.1"
#define DEFAULT_DNS "8.8.8.8"

struct portmap_table_entry {
    uint32_t daddr;
    uint16_t mport;
    uint16_t dport;
    uint8_t proto;
    uint8_t valid;
};

typedef struct {
    char* ssid;
    char* passwd;
    char* static_ip;
    char* subnet_mask;
    char* gateway_addr;
    char* ap_ssid;
    char* ap_passwd;
    char* ap_ip;
} wifi_nat_config_t;

typedef enum {
    EVT_CONNECT,
    EVT_DISCONNECT,
    EVT_FAIL,
} wifi_nat_event_id_t;

typedef struct wifi_nat_event {
    wifi_nat_event_id_t id;
} wifi_nat_event_t;

typedef void (*wifi_nat_app_cb)(wifi_nat_event_t* evt);

esp_err_t initialize_wifi_nat(wifi_nat_app_cb app_cb, char* name);

#endif /* COMPONENTS_WIFI_NAT_WIFI_NAT */
