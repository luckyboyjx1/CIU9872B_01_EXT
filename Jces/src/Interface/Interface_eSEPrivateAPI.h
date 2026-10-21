#ifndef _INTERFACE_ESEPRIVATEAPI_H
#define _INTERFACE_ESEPRIVATEAPI_H
#include "ecpDefine.h"
#include "bootPlaformConfigProfile.h"

#define eSE_Private_TagFirstByte   0xDF

#ifdef  COS_Update
extern u8 X_A G_RNG_SE[8];
extern u8 X_A G_RNG_Sever[8];
extern u8 X_A G_eSE_MAC_IV[0x10];
#endif
extern void CallEseInterfaceFunc(u8 tag);
extern void ProcessError(void);

extern void CUSTOMERAPI_GetSEInfo(void);
#ifdef  COS_Update
extern void CUSTOMERAPI_severRandom(void);
extern void CUSTOMERAPI_ReturnBootLoader(void);
extern void CUSTOMERAPI_UpdateFirmware(void);
extern void CUSTOMERAPI_VerifyFupCRC(void);

extern void eSEPrivateInit(void);
#endif
#endif
