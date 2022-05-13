#ifndef COMPONENTS_PPP_PPPOS_APP
#define COMPONENTS_PPP_PPPOS_APP

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    void (*connected)(void* data);
    void (*disconnected)(void* data);
} pppos_app_cb_t;

void pppos_app_init(const pppos_app_cb_t* interface);

void pppos_app_connect(void);

void ppppos_app_disconnect(void);

void pppos_app_receive(uint8_t* data, size_t len);

#endif /* COMPONENTS_PPP_PPPOS_APP */
