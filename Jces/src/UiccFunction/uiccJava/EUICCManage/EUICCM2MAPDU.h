#ifndef    _EUICCM2MAPDU_H_
#define    _EUICCM2MAPDU_H_
#include   "typedef.h"
#include   "profile.h"

/*************************tag define************************/
#if M2M_EUICC
#define      TAG_RC      0x85
#define      RC_Len      0x10

#define     ISDR_KeySet      0x00
#define     ISDP_KeySet      0x80

#define     M2M_KeySet1      0x01
#define     M2M_KeySet2      0x02

extern    BYTE xdata G_M2MKeySetState;
extern    BYTE xdata G_M2MStoreData[2];

/*************************function define********************************/
extern void M2M_DeleteCommand_ProfileDelete(BYTE deleteProfileindex);
extern void M2M_DeleteCommand_MasterDelete(BYTE deleteProfileindex);
extern void M2M_DeleteCommand_FinaliseISDRhandover(void);
extern BYTE M2M_DeleteCommand(void);

extern BYTE M2M_CreateISDP(void);

extern void M2M_StoreData_HandleNotificationConfirmation (void);
extern void M2M_StoreData_UpdateConnectivityParameters(BYTE profileIndex);
extern void M2M_StoreConnectivityParameters(BYTE profileIndex, BYTE *dataBuf, BYTE dataLen);
extern void M2M_StoreData_EstablishISDRKeySet1(BYTE keyType);
extern void M2M_StoreData_EstablishISDRKeySet2(BYTE keyType);
extern void M2M_StoreData_UpdateSMSRAddressingParameters(void);
extern void M2M_StoreData_SetFallbackAttribute(void);
extern void M2M_StoreData_ProfileDisable(void);
extern void M2M_StoreData_ProfileEnable(void);

extern BYTE M2M_ISDRStoreData(void);
extern BYTE M2M_ISDPStoreData(void);

extern void M2M_StoreData_UpdatePOL1ByMNO(void);

extern BYTE M2M_InstallForPersonalization(void);


extern void M2M_ISDRRAMCommand(void);
extern void M2M_ISDPRAMCommand(void);
#endif

//extern void NM_ECASDGetData(void);
//extern void NM_ECASDPersonal(void);
#if M2M_ESx
extern void NM_M2MISDPStoreDataPersonal(void);
extern BYTE M2MLocalEnvelopCommand(void);
extern void M2M_StoreData_Set_Emergency_Profile_Attribute(void);
#endif

//extern void NM_M2MISDRStoreDataPersonal(void);
#endif


