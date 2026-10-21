#ifndef ANNEX_B_ADMINSESSIONPARA_H
#define ANNEX_B_ADMINSESSIONPARA_H
#include "typedef.h"
#include "ecpDefine.h"

extern void GPStoreSDAdminSessionPara(PN8 sdDataObj, BYTE * dataBuffer);
extern WORD GPGetSDAdminSessionPara(PN8 sdDataObj, WORD tag, BYTE * dataBuffer);
//extern BYTE IF_AnnexB_GPStoreDataPara(void);
extern BYTE GetSDAdminSessionParaTagOffet(BYTE tag);
extern BYTE GPStoreSDAdminSessionParaSub(PN8 sdDataObj, BYTE tag, BYTE * dataBuffer);
extern void GPStoreSDAdminSessionPara85(PN8 sdDataObj, BYTE * dataBuffer);
//extern WORD GPGetSDAdminSessionPara85(PN8 sdDataObj, BYTE * dataBuffer);
//extern WORD GPGetSDAdminSessionParaA5(PN8 sdDataObj, BYTE * dataBuffer);

extern WORD GPGetSDAdminSessionPara85orA5(PN8 sdDataObj, BYTE * dataBuffer,BYTE mode);
#define GPGetSDAdminSessionPara85(sdDataObj, dataBuffer)  GPGetSDAdminSessionPara85orA5(sdDataObj, dataBuffer,0x85)
#define GPGetSDAdminSessionParaA5(sdDataObj, dataBuffer)  GPGetSDAdminSessionPara85orA5(sdDataObj, dataBuffer,0xA5)

extern void StoreInstallforInstallSDAdmSessionParameter(BYTE AdmSessParaOffset, WORD* tagRef);
extern WORD GPGetSDTriggerParaBySDID(BYTE sdID, BYTE tag, BYTE *dataBuffer,WORD dataBufferLen);

#endif



