idf_component_register( SRCS "cloud_api.c" "cloud_mqtt.c" "cloud_tcp.c" "cloud_nvs.c"
						INCLUDE_DIRS "."
						REQUIRES ""
						PRIV_REQUIRES "mqtt" "nvs_flash" )
