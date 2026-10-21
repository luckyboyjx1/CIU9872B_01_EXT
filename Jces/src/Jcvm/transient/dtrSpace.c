#include "RamManager.h"
#include "tranManager.h"
#include "dtrSpace.h"
#include "anomaly.h"
#include "JcvmRam.h"
#include "SEEUICCInterface.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if JCVM__MEMORY__DTR_SPACE

PX8     		X_A                         g_dtrStartAddress[MAX_LOGICAL_CHANNELS];
u8  			X_A							g_channelSpaceId[MAX_LOGICAL_CHANNELS];
u8				X_A							g_dtrSpaceInfo[DTR_SPACE_INFO_TATOL_SIZE];

/************************************************************************************************************************************************
*************************************************************************************************************************************************/
#define		dtrSpace_getDtrSpaceInfoEntry(index)						PX8_A_U16(MEMtPX8(g_dtrSpaceInfo), U8_M_U8(index, DTR_SPACE_INFO_ENTRY_SIZE))

#define		dtrSpace_getDtrSpaceStatus(entry)							PX_R_U8(PX8_A_CONST(entry, DTR_SPACE_INFO_USED_OFFSET))
#define		dtrSpace_setDtrSpaceStatus(entry,status)					PX_W_U8(PX8_A_CONST(entry, DTR_SPACE_INFO_USED_OFFSET), status)

#define		dtrSpace_getDtrSpaceContext(entry)							PX_R_U16(PX8_A_CONST(entry, DTR_SPACE_INFO_CTX_OFFSET))
#define		dtrSpace_setDtrSpaceContext(entry,context)					PX_W_U16(PX8_A_CONST(entry, DTR_SPACE_INFO_CTX_OFFSET), context)

#define		dtrSpace_getDtrSpaceProfileId(entry)						PX_R_U8(PX8_A_CONST(entry, DTR_SPACE_INFO_PROFILE_OFFSET))
#define		dtrSpace_setDtrSpaceProfileId(entry,profileId)				PX_W_U8(PX8_A_CONST(entry, DTR_SPACE_INFO_PROFILE_OFFSET), profileId)

/************************************************************************************************************************************************
*************************************************************************************************************************************************/
void dtrSpace_init(void)
{
	ecpMem_Set(g_dtrStartAddress, 0x00, sizeof(g_dtrStartAddress));
	ecpMem_Set(g_channelSpaceId, 0x00, MAX_LOGICAL_CHANNELS);
	ecpMem_Set(g_dtrSpaceInfo, 0x00, DTR_SPACE_INFO_TATOL_SIZE);
}

u8 dtrSpace_getDtrSpaceId(u8 channelId) 
{
	return g_channelSpaceId[channelId];
}

void dtrSpace_setChannelSpaceId(u8 channelId, u8 spaceId) 
{
	if (spaceId >= logicalChannelNums) {
        fatal_error(INVALID_DTR_SPACEID_ERROR);
    }
	g_channelSpaceId[channelId]		=  spaceId;
}

memref dtrSpace_getDtrSpaceBase(u8 spaceId)
{
	return PXtMEM(g_dtrStartAddress[spaceId]);
}

memref dtrSpace_getDtrSpaceEntry(u8 spaceId)
{
	return PXtMEM(&g_dtrStartAddress[spaceId]);
}

memref tranManager_getDtrPtrEntry(u8 ctxId)
{
	return PNtMEM(PN8_A_U16(MEMtPN8(JcvmConfig_GetDtpBase()), U8_M_U8(SIZE_U16, ctxId)));
}
/**
 * modify by xuxiaojun@20220513 根据分层代码 第一次代码评审(20220415).xlsx 将G_CurExecutingProfileIndex宏定义为0xFF
 */
u8 dtrSpace_assignDtrSpace(u16 contextId, u1 isMultiSelected)
{	
	u8				X_A		spaceId;
	u8				X_A		ctxId;
	u16				X_A		dtrAvailable;
	
	ctxId		= GET_PKGID(contextId);
	spaceId		= INVALID_SPACE_ID;
#if JCVM__RAM__DTR__RTR__NEW	
	dtrAvailable = U32t16(tranAllocate_dtrAvailable(ctxId));
#else	
	dtrAvailable = U32t16(tranAllocate_dtrAvailable());
#endif
	if(true == isMultiSelected)
	{
		spaceId			= dtrSpace_searchExistDtrCtx(U16_AND_U16(contextId, GROUP_MASK));
	}
	else if (dtrAvailable < tranMananger_getDtpByCtxId(ctxId))
	{
		spaceId = INVALID_SPACE_ID;
	}
	else
	{
		for (spaceId = 0; spaceId < logicalChannelNums; U8_INC(spaceId))
		{
			if (DTR_AVAILABLE == dtrSpace_getDtrSpaceStatus(dtrSpace_getDtrSpaceInfoEntry(spaceId)))
			{

				if(NO_MEMORY_AVAILABLE == RamManager_alloc(MEMtPX8(dtrSpace_getDtrSpaceEntry(spaceId)), tranMananger_getDtpByCtxId(ctxId), false) )
				{
					spaceId		= INVALID_SPACE_ID;
					break;
				}
				dtrSpace_setDtrSpaceStatus(dtrSpace_getDtrSpaceInfoEntry(spaceId), DTR_INUSE);
				dtrSpace_setDtrSpaceContext(dtrSpace_getDtrSpaceInfoEntry(spaceId), U16_AND_U16(contextId, GROUP_MASK));
				dtrSpace_setDtrSpaceProfileId(dtrSpace_getDtrSpaceInfoEntry(spaceId), G_CurExecutingProfileIndex);
				break;
			}
		}
	}
	return spaceId;
}

u8	dtrSpace_searchExistDtrCtx(u16 contextId)
{
	u8 		spaceId;
	PX8		dtrSpaceInfoEntry;
	
	spaceId		= 0;
	while(spaceId < MAX_LOGICAL_CHANNELS)
	{
		dtrSpaceInfoEntry = MEMtPX8(dtrSpace_getDtrSpaceInfoEntry(spaceId));
		if((dtrSpace_getDtrSpaceStatus(dtrSpaceInfoEntry) == DTR_INUSE) && (dtrSpace_getDtrSpaceContext(dtrSpaceInfoEntry) == contextId))
		{
		#if (EUICC_ONLY||SE_AND_EUICC)
			if(Euicc_getCurExecProfile() == dtrSpace_getDtrSpaceProfileId(dtrSpaceInfoEntry))
		#endif
			{
				break;
			}
		}
		U8_INC(spaceId);
	}

	if(MAX_LOGICAL_CHANNELS == spaceId)
	{
		spaceId		= INVALID_SPACE_ID;
	}
	
	return spaceId;
}

void dtrSpace_closeDtrSpace(u8 spaceId, u16 contextId)
{
	dtrSpace_setDtrSpaceStatus(dtrSpace_getDtrSpaceInfoEntry(spaceId), DTR_AVAILABLE);
	dtrSpace_setDtrSpaceContext(dtrSpace_getDtrSpaceInfoEntry(spaceId), NULL_CONTEXT);
#if (EUICC_ONLY||SE_AND_EUICC)
	dtrSpace_setDtrSpaceProfileId(dtrSpace_getDtrSpaceInfoEntry(spaceId), G_CurEnabledProfileIndex);
#endif
	RamManager_free(MEMtPX8(dtrSpace_getDtrSpaceEntry(spaceId)));
}
#endif
