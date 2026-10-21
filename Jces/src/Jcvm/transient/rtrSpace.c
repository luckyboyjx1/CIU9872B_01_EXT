/*
 * rtrSpace.c
 *
 *  Created on: 2020?6?5?
 *      Author: weipingjiao
 */

#include "rtrSpace.h"
//#include "ecpDefine.h"
#include "profile.h"
#include "euiccConfig.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 


PX8     		X_A                         g_rtrStartAddress[MAX_RTR_SPACE_CHANNELS];
//u8  			X_A							g_rChannelSpaceId[MAX_RTR_SPACE_CHANNELS];
//u8				X_A							g_rtrSpaceInfo[MAX_RTR_SPACE_CHANNELS];
//
///************************************************************************************************************************************************
//*************************************************************************************************************************************************/
//#define		dtrSpace_getRtrSpaceInfoEntry(index)						PX8_A_U16(MEMtPX8(g_rtrSpaceInfo), U8_M_U8(index, DTR_SPACE_INFO_ENTRY_SIZE))
//
//#define		dtrSpace_getRtrSpaceStatus(entry)							PX_R_U8(PX8_A_CONST(entry, DTR_SPACE_INFO_USED_OFFSET))
//#define		dtrSpace_setRtrSpaceStatus(entry,status)					PX_W_U8(PX8_A_CONST(entry, DTR_SPACE_INFO_USED_OFFSET), status)
//
//#define		dtrSpace_getRtrSpaceContext(entry)							PX_R_U16(PX8_A_CONST(entry, DTR_SPACE_INFO_CTX_OFFSET))
//#define		dtrSpace_setRtrSpaceContext(entry,context)					PX_W_U16(PX8_A_CONST(entry, DTR_SPACE_INFO_CTX_OFFSET), context)
///************************************************************************************************************************************************
//*************************************************************************************************************************************************/

void rtrSpace_init(void)
{
	ecpMem_Set(g_rtrStartAddress, 0x00, sizeof(g_rtrStartAddress));
//	ecpMem_Set(g_channelSpaceId, 0x00, MAX_LOGICAL_CHANNELS);
//	ecpMem_Set(g_dtrSpaceInfo, 0x00, DTR_SPACE_INFO_TATOL_SIZE);
}

//u8 rtrSpace_getDtrSpaceId(u8 channelId)
//{
//	return g_channelSpaceId[channelId];
//}

//void rtrSpace_setChannelSpaceId(u8 channelId, u8 spaceId)
//{
//	if (spaceId >= MAX_RTR_SPACE_CHANNELS) {
//        fatal_error(INVALID_DTR_SPACEID_ERROR);
//    }
//	g_channelSpaceId[channelId]		=  spaceId;
//}

memref rtrSpace_getRtrSpaceBase(u8 spaceId)
{
	return PXtMEM(g_rtrStartAddress[spaceId]);
}

memref rtrSpace_getRtrSpaceEntry(u8 spaceId)
{
	return PXtMEM(&g_rtrStartAddress[spaceId]);
}

//memref tranManager_getRtrPtrEntry(u8 profileId)
//{
//	return PNtMEM(PN8_A_U16(MEMtPN8(JcvmConfig_GetDtpBase()), U8_M_U8(SIZE_U16, ctxId)));
//}

u8	RtrSpace_searchExistRtrID(void)
{
#if (EUICC_ONLY||SE_AND_EUICC)
	u8  D_A	 RtrSpaceID;
	
	RtrSpaceID = 0;
	
	if(Euicc_getCurExecProfile() == ILLEGAL_PROFILEINDEX)
	{
		RtrSpaceID = SE_SPACE_ID;
	}
	else if(Euicc_getCurExecProfile() == G_CurEnabledProfileIndex)
	{
		RtrSpaceID = PROFILE_ENABLE_SPACE_ID;
	}
	else
	{
		RtrSpaceID = PROFILE_EXECUTION_SPACE_ID;
	}
	return RtrSpaceID;
#else
	return	SE_SPACE_ID;
#endif
}

// API 只 返回 prifile 的 RTR space
u8	RtrSpace_searchProfileRtrID(void)
{
#if (EUICC_ONLY||SE_AND_EUICC)
	u8  D_A	 RtrSpaceID;
	
	RtrSpaceID = 0;
	
	if(Euicc_getCurExecProfile() == ILLEGAL_PROFILEINDEX)
	{
		RtrSpaceID = PROFILE_EXECUTION_SPACE_ID;
	}
	else if(Euicc_getCurExecProfile() == G_CurEnabledProfileIndex)
	{
		RtrSpaceID = PROFILE_ENABLE_SPACE_ID;
	}
	else
	{
		RtrSpaceID = PROFILE_EXECUTION_SPACE_ID;
	}

	return RtrSpaceID;
#else
	return SE_SPACE_ID; //modify by jiangxiaohui@20220324 分层
#endif
}
