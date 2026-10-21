/*****************************************************************************
 * @(#)mematom.c	1.20 03/08/13
 * Version 1.7
 *
 lin guo ming
 eastcompeace Technology Corporation

 History:
 Last modified on 07/07/30
 .Created
 Original authors: lin guo ming
 ****************************************************************************/
#include "tranManager.h"
#include "RamManager.h"
#include "tranAllocate.h"
#include "BootLifeCycle.h"
#include "ob_manager.h"
#include "object.h"
#include "Apdu.h"
#include "nativemethod_impl.h"
#include "JcvmRam.h"
#include "rtrSpace.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if JCVM__MEMORY__TRAN_MANAGER
/*******************************************global variable used in RamManager module************************************************************
*************************************************************************************************************************************************/
PX8				X_A							g_rtrStart;
transient_cache	X_A							g_transientCache;
/************************************************************************************************************************************************
*************************************************************************************************************************************************/
void tranMananger_initInfo(void)
{
	u8	X_A	i;

	for (i = 0; i < NUM_GROUPS; U8_INC(i))
	{
		nvmWriteU16NoAtomic(PN8_A_U16(MEMtPN8(JcvmConfig_GetDtpBase()), U8_M_U8(i, SIZE_U16)), 0);
	}

	JcvmConfig_SetMaxDtp(0);
	JcvmConfig_SetRtp(0);
	JcvmConfig_SetRtpObjectDeleted(0);
	nvmFillNoAtomic(MEMtPN8(JcvmConfig_GetDtpObjectDeletedBase()), 0x00, dTP_NUM);
}
//modify by weipingjiao@20230711
void tranMananger_initSERtp(void)
{
//	dtrSpace_init();
//	rtrSpace_init();

//	//g_rtrStart		= 0;
//	ecpMem_Set(MEMtPX8(&g_transientCache), 0x00, sizeof(g_transientCache));

	RamManager_alloc(MEMtPX8(rtrSpace_getRtrSpaceEntry(SE_SPACE_ID)), tranMananger_getSERtp(), false);

}
void tranMananger_initProfileRtp(void)
{
//	dtrSpace_init();
//	rtrSpace_init();

//	//g_rtrStart		= 0;
//	ecpMem_Set(MEMtPX8(&g_transientCache), 0x00, sizeof(g_transientCache));


#if (EUICC_ONLY||SE_AND_EUICC)
	if(ILLEGAL_PROFILEINDEX != G_CurEnabledProfileIndex)
	{
		RamManager_alloc(MEMtPX8(rtrSpace_getRtrSpaceEntry(PROFILE_ENABLE_SPACE_ID)), tranMananger_getProfileRtp(G_CurEnabledProfileIndex), false);
	}
#endif
}

void tranMananger_initGv(void)
{
	dtrSpace_init();
	rtrSpace_init();

	//g_rtrStart		= 0;
	ecpMem_Set(MEMtPX8(&g_transientCache), 0x00, sizeof(g_transientCache));

//	RamManager_alloc(MEMtPX8(rtrSpace_getRtrSpaceEntry(SE_SPACE_ID)), tranMananger_getSERtp(), false);
//#if (EUICC_ONLY||SE_AND_EUICC)
//	if(ILLEGAL_PROFILEINDEX != G_CurEnabledProfileIndex)
//	{
//		RamManager_alloc(MEMtPX8(rtrSpace_getRtrSpaceEntry(PROFILE_ENABLE_SPACE_ID)), tranMananger_getProfileRtp(G_CurEnabledProfileIndex), false);
//	}
//#endif
}

void tranMananger_commitCache(void)
{
	if((g_transientCache.Flag != 0) && isInProgress()){
		memRefresh();

		if(U8_AND_U8(g_transientCache.Flag, DTP_FLAG))
		{
		 	nvmWriteU16Atomic(MEMtPN8(g_transientCache.DTR_Addr), g_transientCache.DTP_Ptr);
			g_transientCache.DTR_Addr = 0;
		}

		if(U8_AND_U8(g_transientCache.Flag, MAXDTR_FLAG))
		{
			JcvmConfig_SetMaxDtp(g_transientCache.MaxDTR_Ptr);
		}

		if(U8_AND_U8(g_transientCache.Flag, RTR_FLAG))
		{
			JcvmConfig_SetRtp(g_transientCache.RTR_Ptr);
		}
	}

	g_transientCache.Flag = 0;
}

void tranMananger_abortCache(void)
{
	if (U8_AND_U8(g_transientCache.Flag, RTR_FLAG))
	{
		if(g_transientCache.RTR_Ptr != JcvmConfig_GetRtp())
		{
			RamManager_reAlloc(MEMtPX8(rtrSpace_getRtrSpaceEntry(RtrSpace_searchExistRtrID())), JcvmConfig_GetRtp());
		}
	}

	g_transientCache.Flag = 0;

}

u16 tranMananger_getRtp(void)
{
	u16   tempRtp;

	if (U8_AND_U8(g_transientCache.Flag, RTR_FLAG))
	{
		tempRtp = g_transientCache.RTR_Ptr;			
	}
	else
	{
		tempRtp = JcvmConfig_GetRtp();
	}
	return tempRtp;
}

u16 tranMananger_getRtrBase(void)
{
	u8   D_A	  spaceId;
	spaceId  	= RtrSpace_searchExistRtrID();
	//result = RamManager_reAlloc(MEMtPX8(dtrSpace_getRtrSpaceEntry(spaceId)),temPtr);
	return U32t16(PXtMEM(rtrSpace_getRtrSpaceBase(spaceId)));
}

void tranMananger_setRtp(u16 newRtp)
{
	g_transientCache.RTR_Ptr	= newRtp;
	g_transientCache.Flag		= U8_OR_U8(g_transientCache.Flag, RTR_FLAG);
}

u16 tranMananger_getMaxDtp(void)
{
	u16   D_A    maxDtp;

	if (U8_AND_U8(g_transientCache.Flag, MAXDTR_FLAG))
	{
		maxDtp = g_transientCache.MaxDTR_Ptr;			
	}
	else
	{
		maxDtp = JcvmConfig_GetMaxDtp();
	}
	return maxDtp;
}

void tranMananger_setMaxDtp(u16 newMaxDtp)
{
	g_transientCache.MaxDTR_Ptr		= newMaxDtp;
	g_transientCache.Flag			= U8_OR_U8(g_transientCache.Flag, MAXDTR_FLAG);
}

void tranMananger_setDtpByDtrAddress(PN8 dtrAddrss, u16 newDtp)
{
	if (g_transientCache.DTR_Addr == PNtMEM(dtrAddrss))
	{
		g_transientCache.DTP_Ptr		= newDtp;
	}			
	else
	{
		if(U8_AND_U8(g_transientCache.Flag, DTP_FLAG))
		{
		 	nvmWriteU16Atomic(MEMtPN8(g_transientCache.DTR_Addr), g_transientCache.DTP_Ptr);
		}
		g_transientCache.DTP_Ptr		= newDtp;
		g_transientCache.DTR_Addr		= PNtMEM(dtrAddrss);
		g_transientCache.Flag			= U8_OR_U8(g_transientCache.Flag, DTP_FLAG);
	}
}

u1 tranMananger_IsTransient(PN8 entry_addr)
{
	u1 result;
	u8 obType;

	obType = nvmReadU8(MEMtPN8(entry_addr));
	if (U8_AND_U8(obType, HEADER_JCRE_TransObj_IsTrans) == HEADER_JCRE_TransObj_IsTrans)
	{
		result	= true;
	}
	else
	{
		result	= false;
	}

	return result;
}

void tranMananger_setDtpByCtxId(u8 ctxId, u16 newDtp)
{
	tranMananger_setDtpByDtrAddress(MEMtPN8(tranManager_getDtrPtrEntry(ctxId)), newDtp);
}

u16	tranMananger_getDtpByDtrAddress(PN8 dtrAddrss)
{
	u16   curDtrAddr;

	if (g_transientCache.DTR_Addr == PNtMEM(dtrAddrss))
	{
		curDtrAddr = g_transientCache.DTP_Ptr;			
	}
	else
	{
		curDtrAddr = nvmReadU16(dtrAddrss);		
	}
	return curDtrAddr;
}

u16 tranMananger_getDtpByCtxId(u8 ctxId)
{
	return tranMananger_getDtpByDtrAddress(MEMtPN8(tranManager_getDtrPtrEntry(ctxId)));
}

void tranMananger_setTransientObjectDeleted(u16 context)		SMALL_MODE
{
	u8		D_A 		APPID;
	u8		D_A			i;
	u8		D_A			j;
	u8		D_A			dtrMask;

	if (context == 0)
	{
		JcvmConfig_SetRtpObjectDeleted(1);
	}
	else
	{
		APPID		= GET_PKGID(context);
		i			= U8_R_U8(APPID, 8);
		j			= U8_SHR_U8(APPID, 3);
		dtrMask		= nvmReadU8(PN8_A_U8(MEMtPN8(JcvmConfig_GetDtpObjectDeletedBase()), j));
		/* 0x0001 is used as mask bit to set dtrMask for correct context */
		nvmWriteU8Atomic(PN8_A_U8(MEMtPN8(JcvmConfig_GetDtpObjectDeletedBase()), j), U8_OR_U8(U8_SHL_U8(1, i), dtrMask));
	}
	return;
}

void tranMananger_getGlobalBuf(void)
{
	ppApdu_Response_t	response;

	if(BootLifeCycle_GetLifeCycle() >= LIFE_CYCLE_CREATE)
	{
	    #if HAL__PLATFORM__SIMULATOR
//		bRamBuffer		= MEMtPX8(get_array_data(MEMtPF8(locateObjectInMemory(JcvmConfig_GetRamBufferRef()))));
//		bShortBuffer	= MEMtPX16(get_array_data(MEMtPF8(locateObjectInMemory(JcvmConfig_GetShortBufferRef()))));
//		APDUBuffer		= MEMtPX8(get_array_data(MEMtPF8(locateObjectInMemory(JcvmConfig_GetApduBufferRef()))));
//		cacheBuffer		= MEMtPX8(get_array_data(MEMtPF8(locateObjectInMemory(JcvmConfig_GetCacheBufferRef()))));
		#endif
		
		//读取GC Request 标志
		bRamBuffer[GC_Requested] = nvmReadU8(MEMtPN8(JcvmConfig_GetVmGcRequestStateBase()));

		apdu_FetchComand(APDUBuffer); // added by jiangxiaohui@20220324 以前的为什么注释掉？现在又为什么放开
		/*@ -discast @*/
		/*@ -castfcnptr @*/
		apdu_InitSetOutGoingCallBack((SETOUTGOING_CALLBACK *)&SetRemoteProcessRes);
		/*@ +castfcnptr @*/
		/*@ +discast @*/
        apdu_SetCommand(APDUBuffer, APDU_BUFFER_SIZE);
	}
    else
    {
    	apdu_GetResponse(&response);
//		#if HAL__PLATFORM__SIMULATOR
//        APDUBuffer 		= response->pData;
//		#endif
	}
}
#endif
