idf_component_register(
    SRCS "mfc2160_search.c" "mfc2160_enrollment.c" "mfc2160_app.c" "mfc2160_uart.c" "mfc2160_command.c"
    INCLUDE_DIRS . 
    REQUIRES ""
    PRIV_REQUIRES "cloud" 
)
