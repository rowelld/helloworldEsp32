# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES "")
set(COMPONENT_PRIV_REQUIRES "" "mfc2160" "wifi_nat" "cloud" "nvs_flash" "commands" "fota" "rs232" )

set(COMPONENT_SRCS "main.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()