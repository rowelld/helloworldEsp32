#ifndef COMPONENTS_RS232_RS232_API
#define COMPONENTS_RS232_RS232_API

#include <stdint.h>
#include <stdlib.h>

#define RS232_MODE_DEFAULT      (uint8_t)0
#define RS232_MODE_TRANSPARENT  (uint8_t)1
#define RS232_MODE_FINGERPRINT  (uint8_t)2

void rs232_api_init(void);

int8_t rs232_api_set_mode(uint8_t mode);

uint8_t rs232_api_get_mode(void);

void rs232_api_write(const uint8_t* data, size_t len);

#endif /* COMPONENTS_RS232_RS232_API */
