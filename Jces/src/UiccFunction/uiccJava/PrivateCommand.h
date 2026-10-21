#ifndef _PRIVATECOMMAND_H_
#define _PRIVATECOMMAND_H_
#include "profile.h"
#include "typedef.h"
#define OFFSET_UPDATEAUTHDATA_ALGOID     0
#define OFFSET_UPDATEAUTHDATA_KI         1
#define OFFSET_UPDATEAUTHDATA_OPC        17
#define OFFSET_UPDATEAUTHDATA_R          33
#define OFFSET_UPDATEAUTHDATA_C          113
#define OFFSET_UPDATEAUTHDATA_L          118
#define OFFSET_UPDATEAUTHDATA_DELTA      124
#define OFFSET_UPDATEAUTHDATA_RESLEN     125

extern void  SetDefaultApp(void);
extern BYTE HandlePrivateCmds(void);
extern void SetCardLifeCycle(void);
extern void SetStoreData(void);
extern void PersoReadEeprom(void);
extern void RemainHeapSize(void);
extern void InitAddrIndexTable(void);


#if (EUICC_ONLY || SE_AND_EUICC)
extern void UpdatePriveFile(void);
#else
extern void UpdateFile(void);
#endif

extern WORD SetCOSVersionData(BYTE * apduBuffer);
extern u16 getSessionkey(BYTE * apduBuf);
#endif
