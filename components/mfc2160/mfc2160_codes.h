#ifndef COMPONENTS_MFC2160_MFC2160_CODES
#define COMPONENTS_MFC2160_MFC2160_CODES

typedef enum {
    IR_ACCEPT,
    IR_MOVEFINGERMORE,
    IR_DONTMOVEFINGERMORE,
    IR_END,
    IR_TOODRY,
    IR_TOOWET,
    IR_DISORDER,
	IR_FINGERVALID,
	IR_FINGERINVALID,
	IR_MATCHFINGER,
	IR_NOMATCH,
    IR_FAIL    =0x100,
    IR_CREATEFAIL,
    IR_RUNFAIL,
    IR_FINALIZEFAIL,
    IR_TPLSZIEOVERMAX,
	IR_GENCHARFAIL,
} mfc2160_app_result_t;

#endif /* COMPONENTS_MFC2160_MFC2160_CODES */