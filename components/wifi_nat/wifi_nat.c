#include <esp_http_server.h>

#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "nvs.h"
#include "nvs_flash.h"

#if !IP_NAPT
#error "IP_NAPT must be defined"
#endif
#include "lwip/lwip_napt.h"

#include "http_server.h"
#include "wifi_nat.h"
#include "wifi_nat_nvs.h"

static EventGroupHandle_t wifi_event_group;
static wifi_nat_app_cb app_layer_cb;

const int WIFI_CONNECTED_BIT = BIT0;

static const char *TAG = "WIFI NAT";

#define JOIN_TIMEOUT_MS (2000)

uint16_t connect_count = 0;
bool ap_connect = false;
bool first_disconnect = true;
bool has_static_ip = false;

uint32_t my_ip;
uint32_t my_ap_ip;

struct portmap_table_entry portmap_tab[IP_PORTMAP_MAX];

esp_netif_t *wifiAP;
esp_netif_t *wifiSTA;

wifi_nat_config_t nat_config;

#if 0 
char *ssid = NULL;
char *passwd = NULL;
char *static_ip = NULL;
char *subnet_mask = NULL;
char *gateway_addr = NULL;
char *ap_ssid = NULL;
char *ap_passwd = NULL;
char *ap_ip = NULL;
#endif

esp_err_t apply_portmap_tab() {
    for (int i = 0; i < IP_PORTMAP_MAX; i++) {
        if (portmap_tab[i].valid) {            
            ip_portmap_add(portmap_tab[i].proto, my_ip, portmap_tab[i].mport,
                           portmap_tab[i].daddr, portmap_tab[i].dport);
        }
    }
    return ESP_OK;
}

esp_err_t delete_portmap_tab() {
    for (int i = 0; i < IP_PORTMAP_MAX; i++) {
        if (portmap_tab[i].valid) {
            ip_portmap_remove(portmap_tab[i].proto, portmap_tab[i].mport);
        }
    }
    return ESP_OK;
}

esp_err_t get_portmap_tab() {
    esp_err_t err;
    nvs_handle_t nvs;
    size_t len;

    err = nvs_open_from_partition("cfg_part", "wifi-cfg", NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_get_blob(nvs, "portmap_tab", NULL, &len);
    if (err == ESP_OK) {
        if (len != sizeof(portmap_tab)) {
            err = ESP_ERR_NVS_INVALID_LENGTH;
        } else {
            err = nvs_get_blob(nvs, "portmap_tab", portmap_tab, &len);
            if (err != ESP_OK) {
                memset(portmap_tab, 0, sizeof(portmap_tab));
            }
        }
    }
    nvs_close(nvs);

    return err;
}

static char *param_set_default(const char *def_val) {
    char *retval = malloc(strlen(def_val) + 1);
    strcpy(retval, def_val);
    return retval;
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
    esp_netif_dns_info_t dns;
    wifi_nat_event_t evt = {0};

    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            ap_connect = true;
            first_disconnect = true;

            ESP_LOGI(TAG, "got ip:" IPSTR,
                     IP2STR(&event->event_info.got_ip.ip_info.ip));
            my_ip = event->event_info.got_ip.ip_info.ip.addr;
            if (esp_netif_get_dns_info(wifiSTA, ESP_NETIF_DNS_MAIN, &dns) ==
                ESP_OK) {
                dhcps_dns_setserver((const ip_addr_t *)&dns.ip);
                ESP_LOGI(TAG, "set dns to:" IPSTR, IP2STR(&dns.ip.u_addr.ip4));
            }
            apply_portmap_tab();

            evt.id = EVT_CONNECT;
            app_layer_cb(&evt);

            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            // ESP_LOGI(TAG, "disconnected - retry to connect to the AP");
            ap_connect = false;
            if (!has_static_ip) {
                delete_portmap_tab();
            }
            esp_wifi_connect();

            if (first_disconnect) {
                evt.id = EVT_DISCONNECT;
                app_layer_cb(&evt);
                first_disconnect = false;
            }

            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;

        case SYSTEM_EVENT_AP_STACONNECTED:
            connect_count++;
            ESP_LOGI(TAG, "%d. station connected", connect_count);
            break;

        case SYSTEM_EVENT_AP_STADISCONNECTED:
            connect_count--;
            ESP_LOGI(TAG, "station disconnected -%d remain", connect_count);
            break;

        default:
            break;
    }
    return ESP_OK;
}

const int CONNECTED_BIT = BIT0;

void wifi_init(const char *ssid, const char *passwd, const char *static_ip,
               const char *subnet_mask, const char *gateway_addr,
               const char *ap_ssid, const char *ap_passwd, const char *ap_ip) {
    ip_addr_t dnsserver;

    wifi_event_group = xEventGroupCreate();

    esp_netif_init();
    has_static_ip = true;
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifiAP = esp_netif_create_default_wifi_ap();
    wifiSTA = esp_netif_create_default_wifi_sta();

    tcpip_adapter_ip_info_t ipInfo_sta;
    if ((strlen(ssid) > 0) && (strlen(static_ip) > 0) &&
        (strlen(subnet_mask) > 0) && (strlen(gateway_addr) > 0)) {
        has_static_ip = true;
        my_ip = ipInfo_sta.ip.addr = ipaddr_addr(static_ip);
        ipInfo_sta.gw.addr = ipaddr_addr(gateway_addr);
        ipInfo_sta.netmask.addr = ipaddr_addr(subnet_mask);
        tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
        tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo_sta);
        apply_portmap_tab();
    }

    my_ap_ip = ipaddr_addr(ap_ip);

    esp_netif_ip_info_t ipInfo_ap;
    ipInfo_ap.ip.addr = my_ap_ip;
    ipInfo_ap.gw.addr = my_ap_ip;
    IP4_ADDR(&ipInfo_ap.netmask, 255, 255, 255, 0);
    esp_netif_dhcps_stop(wifiAP);
    esp_netif_set_ip_info(wifiAP, &ipInfo_ap);
    esp_netif_dhcps_start(wifiAP);

    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {0};
    wifi_config_t ap_config = {.ap = {
                                   .channel = 1,
                                   .authmode = WIFI_AUTH_WPA2_PSK,
                                   .ssid_hidden = 0,
                                   .max_connection = 10,
                                   .beacon_interval = 100,
                               }};

    strlcpy((char *)ap_config.sta.ssid, ap_ssid, sizeof(ap_config.sta.ssid));
    if (strlen(ap_passwd) < 8) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    } else {
        strlcpy((char *)ap_config.sta.password, ap_passwd,
                sizeof(ap_config.sta.password));
    }

    if (strlen(ssid) > 0) {
        strlcpy((char *)wifi_config.sta.ssid, ssid,
                sizeof(wifi_config.sta.ssid));
        strlcpy((char *)wifi_config.sta.password, passwd,
                sizeof(wifi_config.sta.password));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));
    } else {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));
    }

    dhcps_offer_t dhcps_dns_value = OFFER_DNS;
    dhcps_set_option_info(6, &dhcps_dns_value, sizeof(dhcps_dns_value));

    dnsserver.u_addr.ip4.addr = ipaddr_addr(DEFAULT_DNS);
    dnsserver.type = IPADDR_TYPE_V4;
    dhcps_dns_setserver(&dnsserver);

    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, pdFALSE, pdTRUE,
                        JOIN_TIMEOUT_MS / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(esp_wifi_start());

    if (strlen(ssid) > 0) {
        ESP_LOGI(TAG, "wifi_init_apsta finished.");
        ESP_LOGI(TAG, "connect to ap SSID: %s ", ssid);
    } else {
        ESP_LOGI(TAG, "wifi_init_ap with default finished.");
    }
}

esp_err_t initialize_wifi_nat(wifi_nat_app_cb app_cb, char* name) {
    app_layer_cb = app_cb;

    get_config_param_str("ssid", &nat_config.ssid);
    if (nat_config.ssid == NULL) {
        nat_config.ssid = param_set_default("");
    }
    get_config_param_str("passwd", &nat_config.passwd);
    if (nat_config.passwd == NULL) {
        nat_config.passwd = param_set_default("");
    }
    get_config_param_str("static_ip", &nat_config.static_ip);
    if (nat_config.static_ip == NULL) {
        nat_config.static_ip = param_set_default("");
    }
    get_config_param_str("subnet_mask", &nat_config.subnet_mask);
    if (nat_config.subnet_mask == NULL) {
        nat_config.subnet_mask = param_set_default("");
    }
    get_config_param_str("gateway_addr", &nat_config.gateway_addr);
    if (nat_config.gateway_addr == NULL) {
        nat_config.gateway_addr = param_set_default("");
    }
    get_config_param_str("ap_ssid", &nat_config.ap_ssid);
    if (nat_config.ap_ssid == NULL) {
        nat_config.ap_ssid = param_set_default(name);
    }
    get_config_param_str("ap_passwd", &nat_config.ap_passwd);
    if (nat_config.ap_passwd == NULL) {
        nat_config.ap_passwd = param_set_default("admin1234");
    }
    get_config_param_str("ap_ip", &nat_config.ap_ip);
    if (nat_config.ap_ip == NULL) {
        nat_config.ap_ip = param_set_default(DEFAULT_AP_IP);
    }

    get_portmap_tab();

    // setup wifi
    wifi_init(nat_config.ssid, nat_config.passwd, nat_config.static_ip,
              nat_config.subnet_mask, nat_config.gateway_addr,
              nat_config.ap_ssid, nat_config.ap_passwd, nat_config.ap_ip);

    ip_napt_enable(my_ap_ip, 1);
    ESP_LOGI(TAG, "NAT is enabled");

    char *lock = NULL;
    get_config_param_str("lock", &lock);
    if (lock == NULL) {
        lock = param_set_default("0");
    }
    if (strcmp(lock, "0") == 0) {
        ESP_LOGI(TAG, "starting config web server.");
        start_webserver(&nat_config);
    }
    free(lock);

    register_router();

    return ESP_OK;
}