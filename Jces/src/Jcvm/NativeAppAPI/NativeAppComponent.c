/*
 * NativeAppComponent.c
 *
 *  Created on: 2018年6月29日
 *      Author: jiangxiaohui
 */

#include "NativeAppComponent.h"
//#include "ecpPBOC_Typedefine.h"
#include "AppAPI.h"
#include "JcvmConfig.h"
#include "JcvmDataConfig.h"
#include "Apdu.h"
#include "CreateJavaBuffer.h"

#if VM_WITH_PBOC_API

__attribute__( ( section( "natAppComEntry_data" ) ) )
NativeAppComEntry	CONST	natAppComEntry	={
	PbocAPI_Init,
	PbocAPI_DealBeforThrowExce,
	PbocAPI_GetConfig,
	PbocAPI_CallPboc,
	PbocAPI_CallPbocNative,
	PbocAPI_IsGetConstructAuthCode,
	PbocAPI_IsNativeAppSelected,
	PbocAPI_DeselectNativeApp,
	EdepAPI_IsSearchEdepAdf,
	AppAPI_SelectByFileId,
	AppAPI_ConfigApp
};


// 指向平台层大缓存，复用bTempArr
//PBOC_U8_XD_PTR_XD  g_ecpTempArrInGP;

//The instance NVM area pointer.
u8 *  g_PBOC_instNVMBuff;

//The instance type that being selected.
u8 X_A g_PBOC_instType;                 

//The bit flag to indicate whether PBOC instance has been selected.
u1  g_isPBOCselected;
#if _CRYPTODATA_IN_TRANSACTION_
PBOC_U8  bHaveCryptoKey;
#endif
u16 g_instNvmBuffRef;


WEAK void PbocAPI_Init(void)
{
//	g_PBOC_instRAMBuff[0] = 0;
	g_PBOC_instNVMBuff = 0;
	g_PBOC_instType = 0;
	g_isPBOCselected = 0;
	g_instNvmBuffRef = 0;
	return;
}

WEAK void PbocAPI_DealBeforThrowExce(PX16 errorcode)
{	
	return;
}

WEAK u16 PbocAPI_GetConfig(u8 configType, PX8 pConf)    SMALL_MODE
{
	return 0;
}

WEAK u8 PbocAPI_CallPboc(PX8 apduBuf)
{
	return false;
}

WEAK u8 PbocAPI_CallPbocNative(u16 fun_index)    SMALL_MODE
{
	return false;
}

WEAK u8 PbocAPI_IsGetConstructAuthCode(PX8 apduBuf)    SMALL_MODE
{
	return false;
}

WEAK u8 PbocAPI_IsNativeAppSelected(void)
{
	return false;
}

WEAK void PbocAPI_DeselectNativeApp(void)
{
	return;
}

WEAK u8 EdepAPI_IsSearchEdepAdf(void)
{
	return false;
}

//__attribute__((weak)) u8 AppAPI_SelectByFileId(PX8 apduBuffer, PX8 errorCode)
//{
//	if((apduBuffer[APDU_P1_OFFSET] == 00) && (apduBuffer[APDU_P2_OFFSET] == 00) && (apduBuffer[APDU_P3_OFFSET] == 0x02))
//	{
//		if(GP_SECOND_CONFIG20_SELECT_FILE_BYID_ENABLE)
//		{
//			return true;
//		}
//		else
//		{
//			return false;
//		}
//
//	}
//	else
//	{
//		return false;
//	}
//}
WEAK u16 AppAPI_ConfigApp(PX8 apduBuffer)
{
	return 0x6D00;
}
#endif
