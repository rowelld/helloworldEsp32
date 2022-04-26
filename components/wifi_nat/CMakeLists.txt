idf_component_register( SRCS "http_server.c" "wifi_nat.c" "wifi_nat_nvs.c" 
                        INCLUDE_DIRS "."
                        REQUIRES ""
                        PRIV_REQUIRES "nvs_flash" "wpa_supplicant" "esp_http_server" "console" "lwip")