#ifndef _QUICKCONFIRM_H_
#define _QUICKCONFIRM_H_
#include "ecpDefine.h"
#define EF_DateOfProduction	0x2F52
#define EF_SM2PublicKey		0x2F53
#define EF_Uploadconfig		0x8F00

#define OFFSET_CNTR	0
#define OFFSET_TPDA	5

extern void DisplayCardInfo(void);
extern void UploadCardInfo(void);
extern u8 GetSEID(u8 sdId, u8 tag, u8 * SEID);

#endif
