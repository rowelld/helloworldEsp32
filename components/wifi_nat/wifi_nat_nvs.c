#include "wifi_nat_nvs.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "argtable3/argtable3.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/ip4_addr.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#if !IP_NAPT
#error "IP_NAPT must be defined"
#endif
#include "lwip/lwip_napt.h"

#define NVS_PARTITION "cfg_part"
#define NVS_PARTITION_NAMESPACE "wifi-cfg"

static const char *TAG = "wifi_nat_nvs";

esp_err_t get_config_param_str(char *name, char **param) {
    nvs_handle_t nvs;

    esp_err_t err = nvs_open_from_partition(
        NVS_PARTITION, NVS_PARTITION_NAMESPACE, NVS_READONLY, &nvs);
    if (err == ESP_OK) {
        size_t len;
        if ((err = nvs_get_str(nvs, name, NULL, &len)) == ESP_OK) {
            *param = (char *)malloc(len);
            err = nvs_get_str(nvs, name, *param, &len);
            // ESP_LOGI(TAG, "%s %s", name, *param);
        } else {
            return err;
        }
        nvs_close(nvs);
    } else {
        return err;
    }

    return ESP_OK;
}

void preprocess_string(char *str) {
    char *p, *q;

    for (p = q = str; *p != 0; p++) {
        if (*(p) == '%' && *(p + 1) != 0 && *(p + 2) != 0) {
            // quoted hex
            uint8_t a;
            p++;
            if (*p <= '9')
                a = *p - '0';
            else
                a = toupper((unsigned char)*p) - 'A' + 10;
            a <<= 4;
            p++;
            if (*p <= '9')
                a += *p - '0';
            else
                a += toupper((unsigned char)*p) - 'A' + 10;
            *q++ = a;
        } else if (*(p) == '+') {
            *q++ = ' ';
        } else {
            *q++ = *p;
        }
    }
    *q = '\0';
}

static struct {
    struct arg_str *ssid;
    struct arg_str *password;
    struct arg_end *end;
} set_ap_args;

int set_ap(int argc, char **argv) {
    esp_err_t err;
    nvs_handle_t nvs;

    int nerrors = arg_parse(argc, argv, (void **)&set_ap_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, set_ap_args.end, argv[0]);
        return 1;
    }

    preprocess_string((char *)set_ap_args.ssid->sval[0]);
    preprocess_string((char *)set_ap_args.password->sval[0]);

    if (strlen(set_ap_args.password->sval[0]) < 8) {
        printf("AP will be open (no passwd needed). \n");
    }

    err = nvs_open_from_partition(NVS_PARTITION, NVS_PARTITION_NAMESPACE,
                                  NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_str(nvs, "ap_ssid", set_ap_args.ssid->sval[0]);
    if (err == ESP_OK) {
        err = nvs_set_str(nvs, "ap_passwd", set_ap_args.password->sval[0]);
        if (err == ESP_OK) {
            err = nvs_commit(nvs);
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "AP settings %s/%s stored.",
                         set_ap_args.ssid->sval[0],
                         set_ap_args.password->sval[0]);
            }
        }
    }
    nvs_close(nvs);

    return err;
}

static struct {
    struct arg_str *ssid;
    struct arg_str *password;
    struct arg_end *end;
} set_sta_arg;

int set_sta(int argc, char **argv) {
    esp_err_t err;
    nvs_handle_t nvs;

    int nerrors = arg_parse(argc, argv, (void **)&set_sta_arg);
    if (nerrors != 0) {
        arg_print_errors(stderr, set_sta_arg.end, argv[0]);
        return 1;
    }

    preprocess_string((char *)set_sta_arg.ssid->sval[0]);
    preprocess_string((char *)set_sta_arg.password->sval[0]);

    err = nvs_open_from_partition(NVS_PARTITION, NVS_PARTITION_NAMESPACE,
                                  NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_str(nvs, "ssid", set_sta_arg.ssid->sval[0]);
    if (err == ESP_OK) {
        err = nvs_set_str(nvs, "passwd", set_sta_arg.password->sval[0]);
        if (err == ESP_OK) {
            err = nvs_commit(nvs);
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "STA settings %s/%s stored.",
                         set_sta_arg.ssid->sval[0],
                         set_sta_arg.password->sval[0]);
            }
        }
    }
    nvs_close(nvs);
    return err;
}

static struct {
    struct arg_str *static_ip;
    struct arg_str *subnet_mask;
    struct arg_str *gateway_addr;
    struct arg_end *end;
} set_sta_static_arg;

int set_sta_static(int argc, char **argv) {
    esp_err_t err;
    nvs_handle_t nvs;

    int nerrors = arg_parse(argc, argv, (void **)&set_sta_static_arg);
    if (nerrors != 0) {
        arg_print_errors(stderr, set_sta_static_arg.end, argv[0]);
        return 1;
    }

    preprocess_string((char *)set_sta_static_arg.static_ip->sval[0]);
    preprocess_string((char *)set_sta_static_arg.subnet_mask->sval[0]);
    preprocess_string((char *)set_sta_static_arg.gateway_addr->sval[0]);

    err = nvs_open_from_partition(NVS_PARTITION, NVS_PARTITION_NAMESPACE, NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_str(nvs, "static_ip", set_sta_static_arg.static_ip->sval[0]);
    if (err == ESP_OK) {
        err = nvs_set_str(nvs, "subnet_mask",
                          set_sta_static_arg.subnet_mask->sval[0]);
        if (err == ESP_OK) {
            err = nvs_set_str(nvs, "gateway_addr",
                              set_sta_static_arg.gateway_addr->sval[0]);
            if (err == ESP_OK) {
                err = nvs_commit(nvs);
                if (err == ESP_OK) {
                    ESP_LOGI(TAG, "STA Static IP settings %s/%s/%s stored.",
                             set_sta_static_arg.static_ip->sval[0],
                             set_sta_static_arg.subnet_mask->sval[0],
                             set_sta_static_arg.gateway_addr->sval[0]);
                }
            }
        }
    }
    nvs_close(nvs);
    return err;
}

static void register_set_sta(void) {
    set_sta_arg.ssid = arg_str1(NULL, NULL, "<ssid>", "SSID");
    set_sta_arg.password = arg_str1(NULL, NULL, "<passwd>", "Password");
    set_sta_arg.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "set_sta",
        .help = "Set SSID and password of the STA interface",
        .hint = NULL,
        .func = &set_sta,
        .argtable = &set_sta_arg};
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static void register_set_sta_static(void) {
    set_sta_static_arg.static_ip = arg_str1(NULL, NULL, "<ip>", "IP");
    set_sta_static_arg.subnet_mask =
        arg_str1(NULL, NULL, "<subnet>", "Subnet Mask");
    set_sta_static_arg.gateway_addr =
        arg_str1(NULL, NULL, "<gw>", "Gateway Address");
    set_sta_static_arg.end = arg_end(3);

    const esp_console_cmd_t cmd = {
        .command = "set_sta_static",
        .help = "Set Static IP for the STA interface",
        .hint = NULL,
        .func = &set_sta_static,
        .argtable = &set_sta_static_arg};
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static void register_set_ap(void) {
    set_ap_args.ssid = arg_str1(NULL, NULL, "<ssid>", "SSID of AP");
    set_ap_args.password = arg_str1(NULL, NULL, "<passwd>", "Password of AP");
    set_ap_args.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "set_ap",
        .help = "Set SSID and password of the SoftAP",
        .hint = NULL,
        .func = &set_ap,
        .argtable = &set_ap_args};
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

void register_router(void) {
    register_set_sta();
    register_set_sta_static();
    register_set_ap();
}