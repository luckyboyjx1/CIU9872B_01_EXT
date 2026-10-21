#ifndef    _EUICCPERSONALAPDU_H   
#define	   _EUICCPERSONALAPDU_H 
#include    "typedef.h"

/******************macro    define********************************************/
#define PERSONALTAG_RAT          0x01
#define PERSONALTAG_DEFAULTADDR  0x02
#define PERSONALTAG_RELEASECERT  0x03
#define PERSONALTAG_CALCERTCC    0x04
#define PERSONALTAG_EUICCINFO2   0x05

#define PERSONALTAG_PKCI       0xA0
#define PERSONALTAG_PKIDCI     0xA1
#define PERSONALTAG_CERTCI     0xA2
#define PERSONALTAG_CERTEUM    0xA3
#define PERSONALTAG_CERTEUICC  0xA4
#define PERSONALTAG_SKEUICC    0xA5
#define PERSONALTAG_HTTP_MASTERKEY    0xB0
#define PERSONALTAG_HTTP_CONFIG   	  0xB1

extern WORD xdata G_CAWrite;  //当前正在个人化的证书已经写入的数据长度
extern WORD xdata G_CALength; //当前正在个人化的证书的总长度


#define  PersonalProfile_State_NULL             0x00
#define  PersonalProfile_State_ISDP             0x01
#define  PersonalProfile_State_Personal         0x02
#define  PersonalProfile_State_DownloadProfile  0x03
#define  PersonalProfile_State_Enable           0x04

extern BYTE xdata G_PersonalProfileState;
extern BYTE xdata G_PersonalProfileIndex;

/******************function define********************************************/
extern void EUICCPersonalCACIData(void);
extern void EUICCPersonalCAEUMAndEUICCData(void);
extern void M2M_EUICCPersonalEUICCData(void);
//extern void EUICCPersonalRATData(void);
//extern void EUICCPersonalSMDPDefaultAddress(BYTE *buffer, BYTE mode);

//extern void EUICCRealseCERTData(void);
extern void EUICCCalCERTCheckSum(void);
extern void M2M_EUICCPersonalPKIDData(void);
extern void NM_EUICCPersonal(void);
extern void NM_PersonalProfileCommand(void);
extern void NM_M2M_ValidIns(void);
extern void NM_ECASDPersonal(void);
#endif
