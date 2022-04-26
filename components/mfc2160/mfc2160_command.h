#ifndef COMPONENTS_MFC2160_MFC2160_COMMAND
#define COMPONENTS_MFC2160_MFC2160_COMMAND

#include <stdint.h>
#include <string.h>
#include "esp_err.h"

typedef enum {
    PS_OK                = 0x00,
    PS_COMM_ERR          = 0x01,
    PS_NO_FINGER         = 0x02,
    PS_GET_IMG_ERR       = 0x03,
    PS_FP_TOO_DRY        = 0x04,
    PS_FP_TOO_WET        = 0x05,
    PS_FP_DISORDER       = 0x06,
    PS_LITTLE_FEATURE    = 0x07,
    PS_NOT_MATCH         = 0x08,
    PS_NOT_SEARCHED      = 0x09,
    PS_MERGE_ERR         = 0x0a,
    PS_ADDRESS_OVER      = 0x0b,
    PS_READ_ERR          = 0x0c,
    PS_UP_TEMP_ERR       = 0x0d,
    PS_RECV_ERR          = 0x0e,
    PS_UP_IMG_ERR        = 0x0f,
    PS_DEL_TEMP_ERR      = 0x10,
    PS_CLEAR_TEMP_ERR    = 0x11,
    PS_SLEEP_ERR         = 0x12,
    PS_INVALID_PASSWORD  = 0x13,
    PS_RESET_ERR         = 0x14,
    PS_INVALID_IMAGE     = 0x15,
    PS_HANGOVER_UNREMOVE = 0X17,
} mfc2160_ps_result_t;

typedef struct {
    uint16_t header;
    uint32_t chip_address;
    uint8_t flag;
    uint16_t len;
    uint8_t instruction_code;
} __attribute__((packed)) mfc2160_instruction_t;

uint32_t check_finger_valid(void);

mfc2160_ps_result_t mfc2160_ps_read_sys_para(void);

mfc2160_ps_result_t mfc2160_ps_read_index_table(uint8_t *indexTable);

mfc2160_ps_result_t mfc2160_ps_get_image(void);

mfc2160_ps_result_t mfc2160_ps_gen_char(uint8_t bufID);

mfc2160_ps_result_t mfc2160_ps_reg_model(void);

mfc2160_ps_result_t mfc2160_ps_store_char(uint8_t uId);

mfc2160_ps_result_t mfc2160_ps_cancel(void);

mfc2160_ps_result_t mfc2160_ps_search(uint16_t *uId, uint16_t *score);

mfc2160_ps_result_t genchar(uint8_t bufId);

mfc2160_ps_result_t mfc2160_ps_empty(void);

mfc2160_ps_result_t mfc2160_ps_del_char(uint8_t uId);

mfc2160_ps_result_t mfc2160_ps_up_char(uint8_t buffID);

mfc2160_ps_result_t mfc2160_ps_load_char(uint8_t bufId, uint8_t uID);

mfc2160_ps_result_t mfc2160_ps_valid_template_num(uint16_t *validN);

#endif /* COMPONENTS_MFC2160_MFC2160_COMMAND */
