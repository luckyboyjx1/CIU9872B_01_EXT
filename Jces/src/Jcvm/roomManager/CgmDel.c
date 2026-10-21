/*
 * cgmDelTime.c
 *
 *  Created on: 2015Äê10ÔÂ21ÈÕ
 *      Author: liuhanren
 */
#include "CgmManager.h"

#if GP__CGM__FOR_MASK

void cgmDel_App(u8 appId)
{
	u8 				    cgmFlag;
	u8 					bsdId;
	u32 				maxSDram;
	u16					context;
	u32					consumeRam;
	u32 				maxSDnvm;
	u32					consumeNvm;
	PN8 				cgmArrayAddr;

	maxSDram 			= 0;
	maxSDnvm 			= 0;

	context				= U16_SHL_U8(U8t16(CgmField_getPkgID(MEMtPF8(getSdEntryObj(appId)))), 8);
	context				= U16_OR_U16(context, U8t16(appId));
	CgmField_getEntryInfo(context);
	cgmFlag 			= CgmField_getCGMFlag(MEMtPF8(APP_Entry_Object_Cash));
	cgmArrayAddr 		= MEMtPN8(CgmField_getArrayObj(MEMtPF8(APP_Entry_Object_Cash)));

	consumeNvm			= g_UsedNVM;
	consumeRam			= g_UsedRAM;
	if(U8_AND_U8(cgmFlag, HAS82_MASK))
	{
		maxSDram = CgmField_getMax82(cgmArrayAddr, cgmFlag);
		#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
		maxSDram += CgmManager_getMax1of2(g_sD7Value, consumeRam);
		#endif
	}
	else
	{
		maxSDram = CgmManager_getMax1of2(g_sD7Value, consumeRam);
	}

	if(U8_AND_U8(cgmFlag, HAS83_MASK))
	{
		maxSDnvm = CgmField_getMax83(cgmArrayAddr, cgmFlag);
		#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
		maxSDnvm = U32_A_U32(maxSDnvm, CgmManager_getMax1of2(g_sD8Value, g_sUsedNVM));
		#endif
	}
	else
	{
		maxSDnvm = CgmManager_getMax1of2(g_sD8Value, consumeNvm);
	}

	bsdId = getAppAssociatedSdId(MEMtPF8(APP_Entry_Object_Cash));

	if(appId == bsdId)
	{
		bsdId = APP_FIRST;
	}
	CgmManager_manageSdMenSize(bsdId, maxSDram, maxSDnvm, SD_INCREASE_SIZE, false);
	#if CHINA_UMICOM_SPECIAL_MANAGE
	if(cgmFlag & HAS83_MASK)
	{
		CgmManager_manageSdMenSize(APP_FIRST, 0, CHINA_UNICOM_ISD_CONSUME_FOR_SSD, SD_INCREASE_SIZE, false);
	}
	#endif

	return;
}

void cgmDel_Pkg(u8 pkgSdId,u8 pkgId)
{
	PN8 				pkgEntryAddr;
	u32 				pkgConsumeNvm;
	#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
	pkgInfoc6c7c8 * INFOPtr;
	u16 c6;
	u16 c7;
	u16 c8;
	#endif

	pkgEntryAddr = MEMtPN8(locateObjectInMemory(getPkgEntry(pkgId)));
	pkgConsumeNvm = PX_R_U32(CgmManager_getPkgUsedNvmObj(pkgEntryAddr));

#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
	INFOPtr = (pkgInfoc6c7c8 *)getPkgInfoAddr(pkgId);
	c6 = getRecord_LoadC6(INFOPtr);
	c7 = getRecord_LoadC7(INFOPtr);
	c8 = getRecord_LoadC8(INFOPtr);
	if(pkgConsumeNvm < (u32)(c6 + c8))
	{
		pkgConsumeNvm = c6 + c8;
	}
	CgmManager_SetSdMenInfo(pkgSdId, c7, pkgConsumeNvm, SD_INCREASE_SIZE);
	SetRecordOfMemNull(INFOPtr);
#else
	CgmManager_SetSdMenInfo(pkgSdId, 0, pkgConsumeNvm, SD_INCREASE_SIZE);
#endif
	return;
}

//void cgmDel_Key(PN8 keySetObjectPa)
//{
//		u8						appId;
//	    u16						ccOfKeySet;
//	    u16						keyDataRef;
//		u32						keyObjectSize;
//		u32						keyDataObjectSize;
//		PN8     				keySetObject;
//		PN8						keyDataObject;
//		PN8						keyDataHand;
//
//		keySetObject  			= keySetObjectPa;
//		ccOfKeySet				= get_object_owner(keySetObject);
//		appId					= U16t8(U16_AND_U16(ccOfKeySet, APPLET_MASK)) ;
//		keyObjectSize			= CgmField_AlignSize(getObjectSize(keySetObject));
//
//		keyDataHand     		= MEMtPN8(get_object_field_ref(keySetObject, keyData_INDEX));
//		keyDataRef				= memReadU16(keyDataHand);
//		if(!keyDataRef)
//		{
//			return;
//		}
//
//		keyDataObject			= MEMtPN8(locateObjectInMemory(keyDataRef));
//		keyDataObjectSize       = CgmField_AlignSize(getObjectSize(keyDataObject));
//
//		CgmManager_manageSdMenSize(appId, 0, U32_A_U32(keyObjectSize, keyDataObjectSize),SD_INCREASE_SIZE, false);
//		MapTable_SetEntryStatus(keyDataRef, MAP_TABLE_ENTRY_STATUS_AVAIL);
//		MapTable_Commit();
//}
#endif
