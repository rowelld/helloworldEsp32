idf_component_register(SRCS "pppos_app.c"
                        INCLUDE_DIRS "."
                        REQUIRES "" 
                        PRIV_REQUIRES "gsm-at-lib" "lwip")