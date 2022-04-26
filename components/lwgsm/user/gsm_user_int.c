#include "gsm_user_private.h"
#include "lwgsm/lwgsm_call.h"
#include "lwgsm/lwgsm_mem.h"
#include "lwgsm/lwgsm_private.h"

lwgsmr_t gsmi_userinitiate_cmd(lwgsm_msg_t* msg) {
    switch ((uint32_t)CMD_GET_CUR()) {
        case LWGSM_CMD_PPP:
            lwgsm.ll.send_fn("ATD", strlen("ATD"));
            lwgsm.ll.send_fn("*99#", strlen("*99#"));
            lwgsm.ll.send_fn("\r\n", strlen("\r\n"));
            break;
        case LWGSM_CMD_CREG_GET:
            lwgsm.ll.send_fn("AT+CREG?\r\n", strlen("AT+CREG?\r\n"));
            break;
        case LWGSM_USER_CMD_PPP_EXIT:
            // lwgsm.mode = 0;
            lwgsm.ll.send_fn("+++", strlen("+++"));
            // gsm_delay(1000);
            lwgsm_delay(1000);
            break;
        case LWGSM_USER_CMD_CGDCONT:
            lwgsm.ll.send_fn("AT+CGDCONT=1,\"IP\",", strlen("AT+CGDCONT=1,\"IP\","));
            lwgsm.ll.send_fn("\"", 1);
            lwgsm.ll.send_fn(msg->msg.network_attach.apn, strlen(msg->msg.network_attach.apn));
            lwgsm.ll.send_fn("\"", 1);
            lwgsm.ll.send_fn("\r\n", strlen("\r\n"));
            break;
        case LWGSM_USER_CMD_ATD_1:
            lwgsm.ll.send_fn("AT&D1\r\n", strlen("AT&D1\r\n"));
            break;
        case LWGSM_CMD_IFC:
            lwgsm.ll.send_fn("AT+IFC=0,0\r\n", strlen("AT+IFC=0,0\r\n"));
            break;
        case LWGSM_USER_CMD_QURCCFG:
            lwgsm.ll.send_fn("AT+QURCCFG=\"urcport\",\"uart1\"\r\n", strlen("AT+QURCCFG=\"urcport\",\"uart1\"\r\n"));
            break;
        case LWGSM_USER_CMD_QURCRST:
            lwgsm.ll.send_fn("AT+QRIR\r\n", strlen("AT+QRIR\r\n"));
            break;
        case LWGSM_USER_CMD_QRICFG:
            lwgsm.ll.send_fn("AT+QCFG=\"urc/ri/ring\",\"pulse\",500\r\n", strlen("AT+QCFG=\"urc/ri/ring\",\"pulse\",500\r\n"));
            break;
        default:
            return lwgsmERR;            
    }
    return lwgsmOK;
}