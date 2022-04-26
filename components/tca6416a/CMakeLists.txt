idf_component_register( SRCS "TCA6416A_i2c.c" "TCA6416A.c"
                        INCLUDE_DIRS "." 
                        PRIV_REQUIRES "i2cdev" "esp_idf_lib_helpers")