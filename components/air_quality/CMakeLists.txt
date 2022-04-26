idf_component_register( SRCS "air_quality_app.c"
    INCLUDE_DIRS "."
    REQUIRES ""
    REQUIRES "esp_idf_lib_helpers" "sht4x" "sgp40" "i2cdev"
)