#ifndef COMPONENTS_FOTA_FOTA
#define COMPONENTS_FOTA_FOTA

void fota_init(void);

void fota_start(void);

void fota_set_url(const char* url);

const char* fota_get_url(void);

const char* fota_get_version(void);

#endif /* COMPONENTS_FOTA_FOTA */
