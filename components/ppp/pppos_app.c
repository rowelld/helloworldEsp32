#include "pppos_app.h"

#include "gsm/gsm_ppp_api.h"
#include "gsm/gsm_modem_mode.h"
#include "pppos_app.h"
#include "esp_log.h"
#include "tcpip_adapter.h"
#include "netif/ppp/ppp.h"
#include "netif/ppp/pppapi.h"
#include "netif/ppp/pppos.h"

static ppp_pcb* ppp;
static struct netif netif;
static const char* TAG = "PPPOS";
static const pppos_app_cb_t* ppp_app_cb;

void pppos_app_init(const pppos_app_cb_t* interface) {}

void pppos_app_connect(void) {}

void ppppos_app_disconnect(void) {}

void pppos_app_receive(uint8_t* data, size_t len) {}