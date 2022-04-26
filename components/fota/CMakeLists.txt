idf_component_register( SRCS "fota_nvs.c" "fota.c" 
                        INCLUDE_DIRS "."
                        REQUIRES ""
                        PRIV_REQUIRES "nvs_flash" "app_update" "esp_http_client" "cloud" "esp_https_ota"
                        EMBED_TXTFILES "certs/ca_cert.pem" )