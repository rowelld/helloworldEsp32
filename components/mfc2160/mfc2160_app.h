#ifndef COMPONENTS_MFC2160_MFC2160_APP
#define COMPONENTS_MFC2160_MFC2160_APP

#include "esp_err.h"

esp_err_t mfc2160_app_init(void);

esp_err_t mfc2160_app_deinit(void);

esp_err_t mfc2160_app_enrollment(uint8_t *uid);

esp_err_t mfc2160_app_empty_all(void);

esp_err_t mfc2160_app_empty_single(uint8_t indx);

esp_err_t mfc2160_app_search(uint16_t *id);

#endif /* COMPONENTS_MFC2160_MFC2160_APP */
