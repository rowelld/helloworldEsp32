idf_component_register(
    SRCS "tca95x5.c"
    INCLUDE_DIRS "."
    REQUIRES "i2cdev" "esp_idf_lib_helpers"
)
