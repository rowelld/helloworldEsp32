/*
 * gsm_user_int.c
 *
 *  Created on: May 26, 2020
 *      Author: spestano@spinginemail.com
 */


#include "gsm/gsm_private.h"
#include "gsm/gsm_user_private.h"
#include "gsm/gsm_call.h"
#include "gsm/gsm_mem.h"


 /**
 * \brief           Function to initialize every AT command.
 * \note            Never call this function directly. Set as initialization function for command and use `msg->fn(msg)`
 * \param[in]       msg: Pointer to \ref gsm_msg_t with data
 * \return          Member of \ref gsmr_t enumeration
 */
gsmr_t
gsmi_user_initiate_cmd(gsm_msg_t* msg) {
    switch((uint32_t)CMD_GET_CUR()) {
        case GSM_CMD_PPP:
            gsm.ll.send_fn("ATD", strlen("ATD"));
            gsm.ll.send_fn("*99#", strlen("*99#"));
            gsm.ll.send_fn("\r\n", strlen("\r\n"));
            break;
        case GSM_CMD_CREG_GET:
            gsm.ll.send_fn("AT+CREG?\r\n", strlen("AT+CREG?\r\n"));
            break;
        case GSM_CMD_PPP_EXIT:
            gsm.mode = 0;
            gsm.ll.send_fn("+++", strlen("+++"));
            gsm_delay(1000);
            break;
        case GSM_USER_CMD_CGDCONT:
            gsm.ll.send_fn("AT+CGDCONT=1,\"IP\",", strlen("AT+CGDCONT=1,\"IP\",")); 
            gsm.ll.send_fn("\"",1);
            gsm.ll.send_fn(msg->msg.network_attach.apn, strlen(msg->msg.network_attach.apn));
            gsm.ll.send_fn("\"",1);
            gsm.ll.send_fn("\r\n", strlen("\r\n"));
            break;
        case GSM_USER_CMD_PPP_DATA:
            if (gsm.mode) {
                gsm.ll.send_fn(msg->msg.ppp_data.data, msg->msg.ppp_data.len);
            }
            gsm_sys_sem_release(&gsm.sem_sync);
            break;
        case GSM_USER_CMD_ATD_1:
            gsm.ll.send_fn("AT&D1\r\n", strlen("AT&D1\r\n"));
            break;
        case GSM_CMD_IFC:
            gsm.ll.send_fn("AT+IFC=0,0\r\n", strlen("AT+IFC=0,0\r\n"));
            break;
        case GSM_USER_CMD_QURCCFG:
            gsm.ll.send_fn("AT+QURCCFG=\"urcport\",\"uart1\"\r\n", strlen("AT+QURCCFG=\"urcport\",\"uart1\"\r\n"));
            break;
        case GSM_USER_CMD_QURCRST:
            gsm.ll.send_fn("AT+QRIR\r\n", strlen("AT+QRIR\r\n"));
            break;
        case GSM_USER_CMD_QRICFG:
            gsm.ll.send_fn("AT+QCFG=\"urc/ri/ring\",\"pulse\",500\r\n", strlen("AT+QCFG=\"urc/ri/ring\",\"pulse\",500\r\n"));
            break;
        case GSM_USER_CMD_PPP_EXIT:
            gsm.mode = 0;
            // TCA6416AWriteOutputPin(TCA6416A_P02, 0);
            // gsm_delay(500);
            // TCA6416AWriteOutputPin(TCA6416A_P02, 1);
            // gsm_delay(1000);
            break;
        case GSM_CMD_ATH:
            gsm.ll.send_fn("ATH\r\n", strlen("ATH\r\n"));
            break;
        case GSM_CMD_ATO:
            gsm.ll.send_fn("ATO\r\n", strlen("ATO\r\n"));
            break;
        case GSM_CMD_ATE0:
            gsm.ll.send_fn("ATE0\r\n", strlen("ATE1\r\n"));
            break;
        case GSM_CMD_ATE1:
            gsm.ll.send_fn("ATE1\r\n", strlen("ATE1\r\n"));
            break;
        case GSM_CMD_CMGL:
            gsm.ll.send_fn("AT+CMGL=\"REC UNREAD\"\r\n",strlen("AT+CMGL=\"REC UNREAD\"\r\n"));
            break;
        default:
            return gsmERR;
    }

    return gsmOK;
}
