#include "network_utils.h"
#include "lwgsm/lwgsm.h"

static int16_t rssi;

void network_utils_process_reg_change(lwgsm_evt_t* evt) {
    lwgsm_network_reg_status_t stat;

    stat = lwgsm_network_get_reg_status();

    printf("Network registration status changed. New Status is: ");
    switch (stat)
    {
    case LWGSM_NETWORK_REG_STATUS_CONNECTED:
        printf("Connected to home network\r\n");
        break;

    case LWGSM_NETWORK_REG_STATUS_CONNECTED_ROAMING:
        printf("Connected to network and roaming\r\n");
        break;

    case LWGSM_NETWORK_REG_STATUS_SEARCHING:
        printf("Searching for network!\r\n");
        break;

    case LWGSM_NETWORK_REG_STATUS_SIM_ERR:
        printf("SIM CARD Error!\r\n");
        break;
    
    default:
        printf("other\r\n");
        break;
    }
    LWGSM_UNUSED(evt);
}

void network_utils_proces_curr_oeprator(lwgsm_evt_t* evt) {
    const lwgsm_operator_curr_t* o;
    o = lwgsm_evt_network_operator_get_current(evt);
    if(o != NULL) {
        switch (o->format)
        {
        case LWGSM_OPERATOR_FORMAT_LONG_NAME:
            printf("Operator long name: %s \r\n", o->data.long_name);
            break;
        
        case LWGSM_OPERATOR_FORMAT_SHORT_NAME:
            printf("Operator short name: %s \r\n", o->data.short_name);
            break;

        case LWGSM_OPERATOR_FORMAT_NUMBER:
            printf("Operator number: %d \r\n", o->data.num);
            break;
        
        default:
            break;
        }
    }
    lwgsm_network_rssi(&rssi, NULL, NULL, 0);
}

void network_utils_process_rssi(lwgsm_evt_t* evt) {
    int16_t rssi;
    rssi = lwgsm_evt_signal_strength_get_rssi(evt);

    printf("Network operator RSSI: %d dBm \r\n", (int)rssi);
}