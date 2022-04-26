idf_component_register( SRCS "ota_nvs.c" "ota.c" 
                        INCLUDE_DIRS "."
                        REQUIRES ""
                        PRIV_REQUIRES "nvs_flash" "app_update" "esp_http_client" "cloud" "esp_https_ota"
                         )