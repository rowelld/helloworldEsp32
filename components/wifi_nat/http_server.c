
#include "http_server.h"

#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <esp_wifi.h>
#include <sys/param.h>

#include "esp_netif.h"
#include "pages.h"
#include "wifi_nat_nvs.h"

static const char *TAG = "http_server";

esp_timer_handle_t restart_timer;

static void restart_timer_callback(void *arg) {
    ESP_LOGI(TAG, "Restarting now...");
    esp_restart();
}

esp_timer_create_args_t restart_timer_args = {
        .callback = &restart_timer_callback,
        .arg = (void *)0,
        .name = "restart_timer"
    };

static esp_err_t index_get_handler(httpd_req_t *req) {
    char *buf;
    size_t buf_len;
    esp_err_t err;

    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            if (strcmp(buf, "reset=Reboot") == 0) {
                esp_timer_start_once(restart_timer, 500000);
            }
            char param1[64];
            char param2[64];
            char param3[64];

            if (httpd_query_key_value(buf, "ap_ssid", param1, sizeof(param1)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => ap_ssid=%s", param1);
                preprocess_string(param1);
                if (httpd_query_key_value(buf, "ap_password", param2,
                                          sizeof(param2)) == ESP_OK) {
                    ESP_LOGI(TAG, "Found URL query parameter => ap_password=%s",
                             param2);
                    preprocess_string(param2);
                    int argc = 3;
                    char *argv[3];
                    argv[0] = "set_ap";
                    argv[1] = param1;
                    argv[2] = param2;
                    set_ap(argc, argv);
                    esp_timer_start_once(restart_timer, 500000);
                }
            }
            if (httpd_query_key_value(buf, "ssid", param1, sizeof(param1)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => ssid=%s", param1);
                preprocess_string(param1);
                if (httpd_query_key_value(buf, "password", param2,
                                          sizeof(param2)) == ESP_OK) {
                    ESP_LOGI(TAG, "Found URL query parameter => password=%s",
                             param2);
                    preprocess_string(param2);
                    int argc = 3;
                    char *argv[3];
                    argv[0] = "set_sta";
                    argv[1] = param1;
                    argv[2] = param2;
                    set_sta(argc, argv);
                    esp_timer_start_once(restart_timer, 500000);
                }
            }
            if (httpd_query_key_value(buf, "staticip", param1, sizeof(param1)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => staticip=%s",
                         param1);
                preprocess_string(param1);
                if (httpd_query_key_value(buf, "subnetmask", param2,
                                          sizeof(param2)) == ESP_OK) {
                    ESP_LOGI(TAG, "Found URL query parameter => subnetmask=%s",
                             param2);
                    preprocess_string(param2);
                    if (httpd_query_key_value(buf, "gateway", param3,
                                              sizeof(param3)) == ESP_OK) {
                        ESP_LOGI(TAG, "Found URL query parameter => gateway=%s",
                                 param3);
                        preprocess_string(param3);
                        int argc = 4;
                        char *argv[4];
                        argv[0] = "set_sta_static";
                        argv[1] = param1;
                        argv[2] = param2;
                        argv[3] = param3;
                        set_sta_static(argc, argv);
                        esp_timer_start_once(restart_timer, 500000);
                    }
                }
            }
        }
        free(buf);
    }

    const char *resp_str = (const char *)req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    return ESP_OK;
}

static httpd_uri_t indexp = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_get_handler,    
};

httpd_handle_t start_webserver(wifi_nat_config_t *config) {
    httpd_handle_t server = NULL;
    httpd_config_t hconfig = HTTPD_DEFAULT_CONFIG();

    const char *config_page_template = CONFIG_PAGE;
    char *config_page = malloc(strlen(config_page_template) + 512);
    sprintf(config_page, config_page_template, config->ap_ssid,
            config->ap_passwd, config->ssid, config->passwd, config->static_ip,
            config->subnet_mask, config->gateway_addr);
    indexp.user_ctx = config_page;

    esp_timer_create(&restart_timer_args, &restart_timer);

    ESP_LOGI(TAG, "Starting server on port: '%d'", hconfig.server_port);
    if (httpd_start(&server, &hconfig) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &indexp);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server) {
    // Stop web server.
    httpd_stop(server);
}