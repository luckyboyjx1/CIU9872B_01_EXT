#ifndef    _CSIMAPIIMPL_H
#define	   _CSIMAPIIMPL_H
#include "string.h"
#include "typedef.h"

#define TAG_CSIM_SMS_TPDU     0xC8

#define BTAG_CSIM_MESSAGE_ID    0x00

#define BTAG_CSIM_MESSAGE_DISPLAY_MODE   0x0F

#define BTAG_CSIM_USER_DATA    0x01

extern WORD CSIMGetSecureDataLenOffset(BYTE mode);
extern void CSIMGetSecuredDataLength(void);
extern void CSIMGetSecuredDataOffset(void);
extern void CSIMGetUserDataLength(void);
extern void CSIMFindTLVUnderBearerData(void);

#endif
