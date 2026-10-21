#ifndef __APP_API_H_
#define __APP_API_H_

#include "DataOperation.h"
#include "profile.h"
#include "NativeAppComponent.h"

extern NativeAppComEntry	CONST	natAppComEntry;
void AppAPI_ConstructAuthCode(PF8 buf)                    SMALL_MODE;
#if ECP_PBOC_SECURITY
	extern PBOC_U8 ReadEE_InBootArea(PBOC_U16 offs_boot, PBOC_U8 size_data, PBOC_U8_P p_data);
#endif
#if VM_WITH_PBOC_OR_EDEP
	extern void AppAPI_DealBeforThrowExce(PX16 errorcode);
#else
	#define AppAPI_DealBeforThrowExce(errorcode)
#endif

#if JCVM__CONFIG_FILE
	extern u16  AppAPI_WriteConfigFile(PX8 apduBuffer)    SMALL_MODE;
	extern u16 AppAPI_ClearConfigFile(u16 sw)             SMALL_MODE;
#else
	#define	AppAPI_WriteConfigFile(apduBuffer)						0x6D00
	#define	AppAPI_ClearConfigFile(sw)								sw
#endif

#if JCVM__IO__SWP_SELECT_BY_FILE_ID
	extern void AppAPI_initFileIdSys(void);
	extern void AppAPI_ClearFileIdSys(void);
#else
	#define	AppAPI_initFileIdSys()
	#define AppAPI_ClearFileIdSys()
#endif
extern u8 	AppAPI_SelectByFileId(PX8 apduBuffer, PX8 errorCode);

#if VM_WITH_JT_FUNC
extern	void AppAPI_GET_SHARE_TLV_MAX_RECORDS(PF8 buf);
#endif

//#if JCVM__APP_CONFIG
    extern	u16 AppAPI_ConfigApp(PX8 apduBuffer)    SMALL_MODE;
//#else
//	#define AppAPI_ConfigApp(apduBuffer)							0x6D00
//#endif

#endif
