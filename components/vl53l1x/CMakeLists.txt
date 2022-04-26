idf_component_register( SRCS "vl53l1x_app.c" "vl53l1x.c"
                        INCLUDE_DIRS "."
                        REQUIRES "" 
                        PRIV_REQUIRES "i2cdev" "log" "esp_idf_lib_helpers")