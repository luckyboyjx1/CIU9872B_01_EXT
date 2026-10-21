#include "CgmManager.h"
#include "contentManager.h"
#include "nativemethod_impl.h"
#include "profile.h"
#include "mask.h"
#include "ob_manager.h"
#include "bc_object.h"
#include "object.h"
#include "firewall.h"
#include "utilMethod.h"
#include "error.h"
#include "anomaly.h"
#include "global_variable.h"
#include "chn_manager.h"
#include "context.h"
#include "packageloc.h"
#include "mask.h"
#include "cardManager.h"
#include "MemBackUp.h"
#include "cvm.h"
#include "MemOperation.h"
#include "JcvmConfig.h"
#include "KeyLogicManager.h"
#include "NvmMap.h"
#include "FreeTable.h"
#include "OPELFUpgrade.h"
#include "SEEUICCInterface.h"
#include "ecpDefine.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#include "Interface_LTEFromPlatform.h"

/*****************************************************************************
 * File:  contentManager.c
 * Version 1.13

 wang hui modify
 eastcompeace Technology Corporation

 History:
 2007,11,14	.created
 Original author: wang hui
 *****************************************************************************/
/*****************************************************************************
 AID method
 modified by xuxiaojun@20210402 调整_newarrayObject的调用位置，减少cache提交的次数
 modified by jiangxiaohui@20210615: 性能优化，EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
 *****************************************************************************/
jref initAID(PN8 bArray, u16 offset, u8 length)		SMALL_MODE
{
	jref		D_A		theAID;
	jref		D_A		aid;
	//u8			D_A		objType; //remove warning,dengyiwen,2022/9/21
	PN8			D_A		theAidAddr;

	if (AIDLenInvalid(length))
		throw_error(SYSTEMEXCEPT_ILLEGAL_VALUE);

	theAID		= _newarrayObject(AType_Byte, length, BIT_SET_JCRE_ENTRY_Permanent);

	theAidAddr  = MEMtPN8(PFtMEM(PF8_A_CONST(MEMtPN8(locateObjectInMemory(theAID)), PA_DataArray_START))); // theAidAddr	= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(theAID))));

	nvmCopyAtomic(theAidAddr, PN8_A_U16(bArray, offset), U8t16(length) );

	aid	= newObject(AID_CLASS_OFFSET, JC_FRAME_WORK_PKG_ID, BIT_SET_JCRE_ENTRY_Permanent);

	theAidAddr = MEMtPN8(locateObjectInMemory(aid));

	store_object_S_filed(theAidAddr, AIDARRAY_INDEX_OF_AID, theAID);
	memRefresh();
	return aid;
}

boolean AID_equals_array(jref aid, PF8 bArray, u16 offset, u16 length)			SMALL_MODE
{
	u16			D_A		arrayLen;
	PF8			D_A		bArrayAddr;
	PF8			D_A		aidArray;

	aidArray 	= MEMtPF8(locateObjectInMemory(aid));
	arrayLen	= get_array_length(aidArray);
	if (length != arrayLen)
	{
		return false;
	}
	aidArray	= MEMtPF8(get_array_data(aidArray));
	bArrayAddr	= PF8_A_U16(bArray, offset);
	/*@ -type @*/return (memCompare(aidArray, bArrayAddr, length) == ECP_CMP_RESULT_EQUAL);/*@ +type @*/
}

boolean AID_partialEquals_array(jref aid, PF8 bArray, u16 offset, u16 length)	SMALL_MODE
{
	u16			D_A		arrayLen;
	PF8			D_A		bArrayAddr;
	PF8			D_A		aidArray;

	aidArray	= MEMtPF8(locateObjectInMemory(aid));
	arrayLen	= get_array_length(aidArray);
	if (length > arrayLen)
	{
		return false;
	}
	aidArray	= MEMtPN8(get_array_data(aidArray));
	bArrayAddr	= PF8_A_U16(bArray, offset);
	/*@ -type @*/return (memCompare(aidArray, bArrayAddr, length) == ECP_CMP_RESULT_EQUAL);/*@ +type @*/
}

void setPkgCountLocked(void)
{

	u8		D_A   pkgLockedCount;

	pkgLockedCount = nvmReadU8(MEMtPN8(PM_lockedPkgCount_JC));
	JcvmConfig_SetCmLastBasicPkg(U8t16(pkgLockedCount));

}

void setAppletCountLocked(void)
{
	u16		D_A   appLockedCount;

	appLockedCount = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
	JcvmConfig_SetCMappletLocketCount(appLockedCount);
}


void PackageMgrCommit()
{
	jref		D_A		g_newPackage;
	u8			D_A		g_newPackageIdentifier;

	g_newPackage					= nvmReadU16(MEMtPN8(PM_g_newPackage_JC));
	g_newPackageIdentifier			= nvmReadU8(MEMtPN8(PM_g_pkgIdInProcess_JC));

	if (g_newPackage != REF_NULL)
	{
		if (check_addr_null(Pkg_Tabel_Cash))
		{
			Pkg_Tabel_Object_Cash	= locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.pkgTable));
			Pkg_Tabel_Cash 			= get_array_data(MEMtPF8(Pkg_Tabel_Object_Cash));
		}
		nvmWriteU16Atomic(PN8_A_U16(MEMtPN8(Pkg_Tabel_Cash), U8_M_U8(g_newPackageIdentifier, SIZE_REF)), g_newPackage);
		//since the installation is successfully completed, reset this variable
		nvmWriteU8Atomic(MEMtPN8(PM_g_pkgIdInProcess_JC), ILLEGAL_ID);
		nvmWriteU16Atomic(MEMtPN8(PM_g_newPackage_JC), REF_NULL);
		nvmWriteU8Atomic(MEMtPN8(PM_lockedPkgCount_JC),U8_A_U8(nvmReadU8(MEMtPN8(PM_lockedPkgCount_JC)),1));
	}
	FreeTable_BufInit();
	tempConstantObjFree();
}
/*
 * modified by xuxiaojun@20210414 constantpool_component_Size小于0x100时用RAM来存储constantpool_component，可以减少分配与回收的次数，提升性能
 */
void tempConstantObjFree()
{

	constantpool_component_Addresses = nvmReadU32(MEMtPN8(PM_g_tempMemoryAddress_JC));
	//free temp Memory allocted when constantpool need
	if (constantpool_component_Addresses != ILLEGAL_ADDRESS) // 该函数不需要修改，因为PM_g_tempMemoryAddress_JC的初始值为ILLEGAL_ADDRESS
	{
		NvmManager_Free(U32t16(constantpool_component_Addresses), MAP_ENTRY_BLOCK_PCK_ID);
		nvmWriteU32Atomic(MEMtPN8(PM_g_tempMemoryAddress_JC), ILLEGAL_ADDRESS);
		nvmWriteU16Atomic(MEMtPN8(PM_g_tempMemorySize_JC), 0);
		constantpool_component_Addresses	= ILLEGAL_ADDRESS;
	}

	constantpool_component_Size			= 0;
}

// return the pkgEntry  find by index in the pkg_table
jref getPkgEntry(u8 index)			SMALL_MODE
{
	if (check_addr_null(Pkg_Tabel_Cash))
	{
		Pkg_Tabel_Object_Cash	= locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.pkgTable));

		check_null(MEMtPF8(Pkg_Tabel_Object_Cash));

		Pkg_Tabel_Cash			= get_array_data(MEMtPF8(Pkg_Tabel_Object_Cash));
	}
	check_bounds(MEMtPF8(Pkg_Tabel_Object_Cash), index, Header_ArrayType_Reference);

	return nvmReadU16(PN8_A_U16(MEMtPM8(Pkg_Tabel_Cash), U8_M_U8(index, SIZE_REF)));
}

// find the pkg by the aid
u8 findPkgID(PF8 aid, u16 offset, u16 length, u8 isCheckVersion)		SMALL_MODE
{
	u8		D_A		pkgID, f_major1, f_minor1;
	jref	D_A		pkgAID, pkgEntry;
	PF8		D_A		EntryAddr;
	PF8		D_A		pkgStatus;

	pkgStatus		= MEMtPF8(get_array_data(MEMtPF8(locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.pkgStatus)))));

	for (pkgID = 0; pkgID < ON_CARD_PKG_MAX; U8_INC(pkgID))
	{
		if (memReadU8(PF8_A_U8(pkgStatus, pkgID)) != 0)
			continue;

		pkgEntry	= getPkgEntry(pkgID);
		if (pkgEntry != REF_NULL)
		{
			EntryAddr 	= MEMtPF8(locateObjectInMemory(pkgEntry));
			pkgAID		= getPkgAID(pkgEntry);
			if (isCheckVersion != 0)
			{
				f_major1 = memReadU8(get_object_field_ref(EntryAddr, pkgMajor_INDEX));
				f_minor1 = memReadU8(get_object_field_ref(EntryAddr, pkgMinor_INDEX));
				if ((U8_SHR_U8(isCheckVersion , 4) != f_major1) || (U8_AND_U8(isCheckVersion , 0x0F) > f_minor1))
				{
					continue;
				}
			}
			if (AID_equals_array(pkgAID, aid, offset, length))
			{
				return pkgID;
			}
		}
	}
	return ILLEGAL_ID;
}

u16 getPkgContext(u8 pkgID)		SMALL_MODE
{
	u8		D_A		i;
	PN8		D_A		packageContextTable;

		packageContextTable		= MEMtPN8(locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.packageContextTable)));

		packageContextTable		= MEMtPN8(get_array_data(packageContextTable));

		for (i = 0; i < ON_CARD_APPLET_PKG_MAX; U8_INC(i))
		{
			if ( memReadU8(PN8_A_U8(packageContextTable, i)) == pkgID)
			{
				return U8t16(i);
			}
		}


	return 0xFFFF;
}

u8 getPkgIdByContext(u8 pkgContext)    SMALL_MODE
{
	u8       D_A    i;

	PN8      D_A    packageContextTable;

	packageContextTable = MEMtPN8(locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.packageContextTable)));

	packageContextTable = MEMtPN8(get_array_data(packageContextTable));

	i = PN_R_U8(PN8_A_U8(packageContextTable, pkgContext));

	return i;
}
/*
*merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP to support GP__AMENDMENT__H__FOR_MASK
*/
void addAppletPackage(u8 pkgID)		SMALL_MODE
{
	u8		D_A		i;
	PN8		D_A		packageContextTable;

	packageContextTable		= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.packageContextTable)))));
	//将升级过程中删除的旧包上下文表中的位置保留赋值给新包
#if GP__AMENDMENT__H__FOR_MASK
    if( PN_R_U8(upGradeStatus_INDEX)> UPGRADE_STATS_COMPLETED)
    {
    	if(PN_R_U8(upGradepkgPkgID_INDEX) == pkgID)
		{
				if (memReadU8(PN8_A_U8(packageContextTable, PN_R_U8(upGradepkgContext_INDEX))) == ILLEGAL_ID)
				{
					nvmWriteU8Atomic(PN8_A_U8(packageContextTable, PN_R_U8(upGradepkgContext_INDEX)), pkgID);
					nvmWriteU8Atomic(G_CurExecuEnvironment.appletPkgCount, U16t8(U8_A_U8(nvmReadU8(G_CurExecuEnvironment.appletPkgCount), 1)));
					return;
				}
    	}
		else
		{
			throw_error(SW_CONDITIONS_NOT_SATISFIED);//应用升级过程中，新下载的包与应用升级无关
		}
    }
    
#endif
	for (i = 0; i < ON_CARD_APPLET_PKG_MAX; U8_INC(i))
	{
		if (memReadU8(PN8_A_U8(packageContextTable, i)) == ILLEGAL_ID)
		{
			nvmWriteU8Atomic(PN8_A_U8(packageContextTable, i), pkgID);
			nvmWriteU8Atomic(G_CurExecuEnvironment.appletPkgCount, U16t8(U8_A_U8(nvmReadU8(G_CurExecuEnvironment.appletPkgCount), 1)));//该行代码不能被注释掉，即使放开对于native 应用也没有影响 modify by jiangqingbin@20220622
			return;
		}
	}
	return;
}
/*
* modified by xuxiaojun@20210402 调整_newarrayObject的调用位置，减少cache提交的次数
*/
void setImportInfo(PF8 bArray, u8 offset, u8 count)	SMALL_MODE
{
	jref	D_A		newArray;
	PF8		D_A		peAddr;

	newArray	= _newarrayObject(AType_Byte, count, BIT_NEED_NOTHING);

	peAddr = PF8_A_CONST(MEMtPF8(locateObjectInMemory(newArray)), PA_DataArray_START); //peAddr	= MEMtPF8(get_array_data(MEMtPF8(locateObjectInMemory(newArray))));

	nvmCopyNoAtomic(peAddr, PF8_A_U8(bArray, offset), count);

	peAddr		= MEMtPF8(locateObjectInMemory(nvmReadU16(MEMtPN8(PM_g_newPackage_JC)))); // newArray 与 PM_g_newPackage_JC不在同一页，所有前面的_newarrayObject不提交CACHE，走到这里时会自动提交

	nvmWriteU16Atomic(MEMtPN8(get_object_field_ref(peAddr, importedPackages_INDEX)), newArray);	  //4 wzq: nvmWriteU16Atomic -> nvmWriteU16Atomic
	nvmWriteU8Atomic(MEMtPN8(get_object_field_ref(peAddr, importCount_INDEX)), count);// 10
	// PM_g_newPackage_JC的importedPackages_INDEX 与 newArray 不在同一页，所以会自动提交CACHE
}

void releaseStaticFeild(u16 appContext)		SMALL_MODE
{
	jref		D_A		pkgEntry;
	u16			D_A		staicRefNumber;
	PF8			D_A		pkgEntryAddr;
	PF8			D_A		staticFeild;
	PF8			D_A		PkgContexTable;
	u8			D_A		pkgID;
	jref		D_A		objectId;
	u16			D_A		index;

	if(appContext == INVALID_CONTEXT)
	{
		return;
	}

	PkgContexTable	= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.packageContextTable)))));
	pkgID			= memReadU8(PN8_A_U16(PkgContexTable, U16_AND_U16(U16_SHR_U8(appContext, 8), 0x00FF)));
	if (pkgID > ON_CARD_PKG_MAX)
	{
		return;	// invaild id
	}
	pkgEntry		= getPkgEntry(pkgID);
	pkgEntryAddr	= MEMtPN8(locateObjectInMemory(pkgEntry));
	staicRefNumber	= nvmReadU16(MEMtPN8(get_object_field_ref(pkgEntryAddr, pkgStaticReferenceCount_INDEX)));
	staticFeild		= MEMtPN8(getPackageComponentLocation(pkgID, COMPONENT_STATICFIELD));


	// search the static reference component in this pkg,
	//if has ref point to a object belong to this applet ,removed it.
	for (index = 0; index < staicRefNumber; U16_INC(index))
	{
		objectId = nvmReadU16(staticFeild);

		if ((objectId > 0) && (objectId < INVALID_ID))
		{
			if (nvmReadU16(PN8_A_CONST(MEMtPN8(locateObjectInMemory(objectId)), OBJ_HDR_Context)) == appContext)
			{
				nvmFillAtomic(staticFeild, 0, sizeof(jref));
			}
		}
		staticFeild	= PN8_A_CONST(staticFeild, sizeof(jref));
	}
}

/*****************************************************************************
 PkgEntry  method
modified by xuxiaojun@20210402 调整objAddr数据的写顺序为低地址到高地址
 *****************************************************************************/
jref newPackageEntry(jref aid, u8 major, u8 minor, u8 capMinor)	SMALL_MODE
{
	jref		D_A		objref;
	PN8			D_A		objAddr;

	objref		= newObject(PackageEntry_CLASS_OFFSET, IMPL_PKG_ID, BIT_NEED_REFRESH);
	objAddr 	= MEMtPN8(locateObjectInMemory(objref));

	store_object_B_filed(objAddr, sdId_INDEX, PXB_R_U8(&bRamBuffer[INSTALL_APPLET_SDID])); // 0
	store_object_S_filed(objAddr, pkgAID_INDEX, aid); // 3
	if(capMinor == 0x02)
	{
		store_object_S_filed(objAddr, pkgName_INDEX, g_pkgName); // 6
	}
	store_object_B_filed(objAddr, pkgMinor_INDEX, minor); // 7
	store_object_B_filed(objAddr, pkgMajor_INDEX, major); // 8
	if(capMinor == 0x02)
	{
		store_object_B_filed(objAddr, pkgNamelength_INDEX, g_pkgNameLength); // 11
	}
	return objref;
}

// return the pkg AID
jref getPkgAID(jref PkgEntry)
{
	PN8			D_A		objaddr;
	jref		D_A		value;

	objaddr 	= MEMtPN8(locateObjectInMemory(PkgEntry));

	check_null(objaddr);

	value 		= PN_R_U16(MEMtPN8(get_object_field_ref(objaddr, pkgAID_INDEX)));
	objaddr 	= MEMtPN8(locateObjectInMemory(value));



	check_null(objaddr);

	value 		= PN_R_U16(MEMtPN8(get_object_field_ref(objaddr, AIDARRAY_INDEX_OF_AID)));
	return (value);
}

/*****************************************************************************
 PkgEntry  method
 *****************************************************************************/
//jref newAppTableEntry()
//{
//	jref		X_A		objref;
//
//	objref = newObject(AppTableEntry_CLASS_OFFSET, IMPL_PKG_ID, BIT_SET_JCRE_ENTRY_Permanent);
//	return objref;
//}

/*****************************************************************************
 AppletMgr  method
 *****************************************************************************/
u8 manageAppFlag(u8 appID,u8 appFlagManageType)		SMALL_MODE
{
	u8			D_A		index;
	u8 			D_A		flagBitMapIndex;
	u8 			D_A		appCount;
	u32 		D_A		temp;
	PN8 		D_A		tempAddress;

	#define	iAppIdFlagAddress	tempAddress
	#define iMultiFlagAddress	tempAddress
	#define iExtraFlagAddress	tempAddress

	temp			= 0;
	tempAddress 	= MEMtPN8(0);
	index			= U8_D_U8(appID, APP_FLAG_SIZE);
	flagBitMapIndex = U8_R_U8(appID, APP_FLAG_SIZE);

	switch(appFlagManageType)
	{
		case CLR_ID_FLAG:
		case SET_ID_FLAG:
			iAppIdFlagAddress 	= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.AppIdFlag)))));
			iAppIdFlagAddress	= PN8_A_U16(iAppIdFlagAddress, U8_M_U8(index, SIZE_U32));
			temp				= nvmReadU32(iAppIdFlagAddress);

			if(appFlagManageType == CLR_ID_FLAG)
			{
				temp = U32_XOR_U32(temp, U32_SHL_U8(0x00000001, flagBitMapIndex));
			}
			else
			{
				temp = U32_OR_U32(temp, U32_SHL_U8(0x00000001, flagBitMapIndex));
			}
			break;

		case GET_APP_COUNT:
		{
			PN8			D_A			theAppTable;
			PN8			D_A			entry;
			u16			D_A			i;
			u16			D_A			bFreeEntryIndex;
			u8			D_A			bAppID;

			bFreeEntryIndex		= nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
			theAppTable			= MEMtPN8(getAppTable());
			appCount = 0;
			for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
			{
				entry			= MEMtPN8(locateObjectInMemory(nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(i, SIZE_REF)))));
				bAppID			= memReadU8(get_object_field_ref(entry, bAppID_INDEX));

				/*if ((bAppID == 0) || (bAppID == UICC_APPID))//预置应用不计数
				{
					continue;
				}*/
				if(bAppID == APP_NULL)
				{
					continue;
				}
				U8_INC(appCount);
			}
			return appCount;
		}
		case CLR_MULTI_FLAG:
		case SET_MULTI_FLAG:
		case CHECK_MULTI_FLAG:
			iMultiFlagAddress	= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.MutiFlag)))));
			iMultiFlagAddress	= PN8_A_U16(iMultiFlagAddress, U8_M_U8(index, SIZE_U32));
			temp				= nvmReadU32(iMultiFlagAddress);
			if(appFlagManageType == CLR_MULTI_FLAG)
			{
				temp = U32_AND_U32(temp, ~U32_SHL_U8(0x00000001, flagBitMapIndex));
			}
			else if(appFlagManageType == SET_MULTI_FLAG)
			{
				temp = U32_OR_U32(temp, U32_SHL_U8(0x00000001, flagBitMapIndex));
			}
			else
			{
				if(U32_AND_U32(temp, U32_SHL_U8(1, flagBitMapIndex)))
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			break;

		case SET_EXTRA_FLAG:
		case CLR_EXTRA_FLAG:
		case CHECK_EXTRA_FLG:
			iExtraFlagAddress	= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.ExtraditonFlag)))));
			iExtraFlagAddress	= PN8_A_U16(iExtraFlagAddress, U8_M_U8(index, SIZE_U32));
			temp				= nvmReadU32(iExtraFlagAddress);

			if(appFlagManageType == SET_EXTRA_FLAG)
			{
				temp = U32_OR_U32(temp, U32_SHL_U8(U8t32(U8_AND_U8(1, PXB_R_U8(&bRamBuffer[INSTALL_APPLET_EXTRADTIONFLAG]))), flagBitMapIndex));
			}
			else if(appFlagManageType == CLR_EXTRA_FLAG)
			{
				temp = U32_AND_U32(temp, U32_INV(U32_SHL_U8(0x00000001, flagBitMapIndex)));
			}
			else
			{
				if(U32_AND_U32(temp, U32_SHL_U8(1, flagBitMapIndex)))
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			break;

		default:
			break;
	}

	nvmWriteU32Atomic(tempAddress,temp);
	return 0;
}

u8 getNewID()
{
	u8			D_A		index;
	u8			D_A     start;
	u8			D_A		flagBitMapIndex;
	u8			D_A		flagBitMapBase;
	u32			D_A		tmpID;
	PF8			D_A		iAppIdFlagAddress;

	iAppIdFlagAddress = MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.AppIdFlag)))));

//	if(bRamBuffer[IS_JCRE] == 0x01) //TODO appid 	取负值表示JCRE CONTEXT
//	{
//		start = 4;
//	}
//	else
	{
		start = 0;
	}

	for (index = start; index < (APP_FLAG_END/APP_FLAG_SIZE); U8_INC(index))	//fetch the next element of array iAppIdFlag
	{
		tmpID			= PN_R_U32(PN8_A_U16(iAppIdFlagAddress, U8_M_U8(index, SIZE_U32)));

		flagBitMapBase	= U16t8(U8_M_U8(index, APP_FLAG_SIZE));

		for(flagBitMapIndex = 0;flagBitMapIndex < APP_FLAG_SIZE; U8_INC(flagBitMapIndex))	//find a free bit of the current element
		{
			if (U32_AND_U32(tmpID, U32_SHL_U8(0x00000001, flagBitMapIndex)) == 0)
			{
				return U16t8(U8_A_U8(flagBitMapBase, flagBitMapIndex));	//return the New ID
			}
		}
	}
	return APP_NULL;
}

/*
 * modified by xuxiaojun@20210402 调整_newarrayObject的调用位置，减少cache提交的次数
 * modified by jiangxiaohui@20210615: 性能优化，EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
 * modify by chentao@20211022 MNOSD 需要拷贝权限
 */
jref addEntry(jref classaid, boolean zIsInst)		SMALL_MODE
{
	jref 			D_A		entry;
	u8 				D_A		newID;
	memref 			D_A		entryAddr, theAppTable;
	jref 			D_A		bPrivRef;
	jref 			D_A		bGlbSvcNameRef;
	//jref 			D_A		bAppProIdRef; //remove warning,dengyiwen,2022/9/21
	jref 			D_A		cgmRef;
	u16				D_A		bFreeEntryIndex;
	u16				D_A		cgmArrySize;

	/*@ -disspecopt @*/bFreeEntryIndex 	= nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);/*@ +disspecopt @*/

	entry = REF_NULL;

	bGlbSvcNameRef = REF_NULL;

	bPrivRef = REF_NULL;
	cgmRef  = REF_NULL;

	if (bFreeEntryIndex < APP_END)
	{
		newID = getNewID();

#if (EUICC_ONLY||SE_AND_EUICC)//卡片初始化时 上下文是JCRE （这里将上下文切到应用），实例化应用时在注册方法里是应用的上下文
		if((newID >= UICC_APPID)&&(newID <= ECASD_APPID))
		{
			cc = U16_OR_U16(U16_SHL_U8(newID,8),newID);//(newID <<8)|newID;
		}
		if(bRamBuffer[PROFILE_INSTALL_MNOSD_FLAG] == TRUE)// entry 顺序：MNOSD  UICC ISDR ECASD ISDP ，ISDR UICC ECASD 的模块和应用entry用同一个
		{
			newID = MNOSD_APPID;
		}
#elif JAVA_CARD_UICC
		if(/*(zIsInst == true) ||*/ (newID == UICC_APPID))
		{
			cc = UICC_CONTEXT;
		}
#endif

#if (EUICC_ONLY||SE_AND_EUICC)
		if ((zIsInst == true) || (newID <= ECASD_APPID)) //entry 顺序：MNOSD UICC ISDR  ECASD ISDP ，ISDR UICC ECASD 的模块和应用entry用同一个
#elif JAVA_CARD_UICC
		if ((zIsInst == true) || (newID <= UICC_APPID))// card manager ,UICC
#else
		if ((zIsInst == true) || (newID <= ISD_APPID))// card manager
#endif
		{
			manageAppFlag(newID, SET_ID_FLAG);

			bPrivRef = _newarrayObject(AType_Byte, PRIVMAXBYTES, BIT_NEED_NOTHING); //不是实例的话，不需要权限

			nvmCopyNoAtomic(PF8_A_CONST(MEMtPF8(locateObjectInMemory(bPrivRef)), SIZE_PRIMITIVE_ARRAY_HEADER), PM8_A_U16(bRamBuffer, INSTALL_APPLET_PRIV), PRIVMAXBYTES);

#if	GP__CGM__FOR_APPPARACHECK
			if (newID != ISD_APPID)
			{
				cgmArrySize = getCGMarraySizeImpl(bRamBuffer[SET_INSTALL_PARA]);

				if(0 != cgmArrySize)
				{
					cgmRef = _newarrayObject(AType_Byte, cgmArrySize, BIT_NEED_NOTHING); //不是实例的话，不需要权限
				}

			}
#endif
		}

#if (EUICC_ONLY||SE_AND_EUICC)
		if ((zIsInst == true) || ((newID >= UICC_APPID)&&(newID <= ECASD_APPID)))// classEntry 不添加globalservice成员，和应用共用的classErntry才需要添加
#elif JAVA_CARD_UICC
		if ((zIsInst == true) || (newID <= UICC_APPID)) //ISD and  UICCAPP
#else
		if ((zIsInst == true) || (newID <= ISD_APPID))// ISD
#endif
		{
			bGlbSvcNameRef = _newarrayObject(AType_Byte, MAX_GLOBALSERVICE_NAME_NUMBER, BIT_SET_JCRE_ENTRY_Permanent);
		}

		entry = newObject(AppTableEntry_CLASS_OFFSET, IMPL_PKG_ID, BIT_SET_JCRE_ENTRY_Permanent); // entry = newAppTableEntry(); // 把entry设置成永久入口点对象，这里不提交CACHE是因为函数结束时会调用memRefresh

		entryAddr = locateObjectInMemory(entry);
//
//		set_JCRE_entry(MEMtPN8(entryAddr), false);// 主动提交cache

#if GP__CGM__FOR_APPPARACHECK
		//modify by wuronghua, entryAddr 变量在后面会复用，导致entry取值错误
		g_appEntryAddr = entryAddr;
#endif

#if (EUICC_ONLY||SE_AND_EUICC)
		if ((zIsInst == true) || (newID <= ECASD_APPID)) //entry 顺序：MNOSD UICC ISDR  ECASD ISDP ，ISDR UICC ECASD 的模块和应用entry用同一个
#elif JAVA_CARD_UICC
		if ((zIsInst == true) || (newID <= UICC_APPID))// card manager ,UICC
#else
		if ((zIsInst == true) || (newID <= ISD_APPID))// card manager
#endif
		{
		/*@ -discast @*/
			store_object_B_filed(MEMtPF8(entryAddr), bAppID_INDEX, (jbyte)newID); //
		/*@ +discast @*/
		}
		else
		{
			store_object_B_filed(MEMtPF8(entryAddr), bAppID_INDEX, APP_NULL); // 1
		}
		if(REF_NULL != bPrivRef)
		{
			store_object_B_filed(MEMtPF8(entryAddr), binstallprivlen_INDEX, bRamBuffer[MANDATE_SD_ID]);		// 6
		}
		if(REF_NULL != cgmRef)
		{
			store_object_B_filed(MEMtPF8(entryAddr), CGMflag_INDEX, bRamBuffer[SET_INSTALL_PARA]);		// 7
		}
			
		store_object_S_filed(MEMtPF8(entryAddr), classAID_INDEX, U16tS16(classaid)); // 16

		store_object_S_filed(MEMtPF8(entryAddr), bPriv_INDEX, U16tS16(bPrivRef)); // 18
		
		if(REF_NULL != cgmRef)
		{
			store_object_S_filed(MEMtPF8(entryAddr), CGMarray_INDEX, U16tS16(cgmRef)); 	//19
		}

//		bAppProIdRef = _newarrayObject(AType_Byte, APPPROIDLEN, true);
//		store_object_S_filed(MEMtPF8(entryAddr), bAppProId_INDEX, U16tS16(bAppProIdRef));
//
//		set_JCRE_entry(MEMtPN8(locateObjectInMemory(bAppProIdRef)));
#if (EUICC_ONLY||SE_AND_EUICC)
		if ((zIsInst == true) || ((newID >= UICC_APPID)&&(newID <= ECASD_APPID)))// classEntry 不添加globalservice成员，和应用共用的classErntry才需要添加
#elif JAVA_CARD_UICC
		if ((zIsInst == true) || (newID <= UICC_APPID)) //ISD and  UICCAPP
#else
		if ((zIsInst == true) || (newID <= ISD_APPID))// card manager
#endif
		{
			store_object_S_filed(MEMtPF8(entryAddr), bGlbSvcName_INDEX, U16tS16(bGlbSvcNameRef)); // 20
		}

		theAppTable = getAppTable();
#if (EUICC_ONLY||SE_AND_EUICC)
		if(((bRamBuffer[PROFILE_APDU_MODE] == TRUE)&&(newID == ISDP_APPID))
			||(bRamBuffer[PROFILE_INSTALL_MNOSD_FLAG] == TRUE))//安装MNOSD或者ISDP时 替换原entry的值,支持se+euicc时 ，se的cardmanager也需要替换
		{
			nvmWriteU16Atomic(MEMtPN8(U32_A_U32(theAppTable, U32_M_U32(U8t32(newID), SIZE_REF))), U16tS16(entry));
		}
		else
#endif
		{
			nvmWriteU16Atomic(MEMtPN8(U32_A_U32(theAppTable, U32_M_U32(U16t32(bFreeEntryIndex), SIZE_REF))), U16tS16(entry));
			/*@ -disspecopt @*/
			memWriteU16Atomic(G_CurExecuEnvironment.bFreeEntryIndex, U16_A_U8(bFreeEntryIndex, 1));
			/*@ +disspecopt @*/
		}

#if JAVA_CARD_UICC
		if(/*(zIsInst == true) ||*/ (newID == UICC_APPID))
		{
			cc = JCRE_CONTEXT;
		}
#endif
		memRefresh(); // 主动提交cache
	}
	else
	{
		throw_error(SYSTEMEXCEPT_NO_RESOURCE);
	}
	return entry;
}
#if GP__AMENDMENT__C__FOR_MASK
/**
*added by jiangxiaohui@20210615: 性能优化，EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
*supportContactLessProtocolType_INDEX恢复成0xFFFF,by wuronghua 20211125
*/
jref addClEntry(u8 appletType)		SMALL_MODE
{
	jref 			D_A		entry;
	memref 			D_A		entryAddr;
	jref 			D_A		assignedProtocolTypeOrder;

	entry = REF_NULL;

	assignedProtocolTypeOrder = _newarrayObject(AType_Byte, 4, BIT_NEED_NOTHING); //不是实例的话，不需要权限
	memWriteU32NoAtomic(PF8_A_CONST(MEMtPF8(locateObjectInMemory(assignedProtocolTypeOrder)), SIZE_PRIMITIVE_ARRAY_HEADER), 0x03818284);
	
	entry = newObject(CLRegistryEntry_CLASS_OFFSET, IMPL_AMDC_PKG_ID, BIT_SET_JCRE_ENTRY_Permanent); // entry = newAppTableEntry(); // 把entry设置成永久入口点对象，这里不提交CACHE是因为函数结束时会调用memRefresh
	entryAddr = locateObjectInMemory(entry);
	
	store_object_B_filed(MEMtPF8(entryAddr), applicationDisplay_INDEX, U16tS16(0xFFFF)); // 3
	store_object_B_filed(MEMtPF8(entryAddr), AFI_INDEX, U16tS16(0xFFFF)); // 4
	store_object_B_filed(MEMtPF8(entryAddr), appletType_INDEX, U16tS16(appletType)); // 5
	store_object_B_filed(MEMtPF8(entryAddr), supportContactLessProtocolType_INDEX, U16tS16(0xFFFF)); // 10
	store_object_S_filed(MEMtPF8(entryAddr), CommuInterfAccRestriction_INDEX, U16tS16(0x100)); // 11
	store_object_S_filed(MEMtPF8(entryAddr), continuousProcessingTimeout_INDEX, U16tS16(0x01)); // 14
	
	store_object_S_filed(MEMtPF8(entryAddr), AssignedProtocolTypeOrder_INDEX, U16tS16(assignedProtocolTypeOrder)); // 27
	memRefresh(); // 主动提交cache
	return entry;
}
#endif
u16 getEntryByInstAID(u16 classaid)
{
	PN8			D_A			theAppTable;
	PN8			D_A			entry;
	u32			D_A			AidAdd1;
	u32			D_A			AidAdd2;
	u16			D_A			i;
	u16			D_A			bFreeEntryIndex;
	u8			D_A			AidLen1;
	u8			D_A			AidLen2;
//	u8        X_A       aidBuffer[0x11];
	

	bFreeEntryIndex		= nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
	theAppTable			= MEMtPN8(getAppTable());

	if(REF_NULL != classaid)
	{
		AidAdd2 = locateObjectInMemory(classaid);
		checkPreviousContextAccess(MEMtPF8(AidAdd2), 0);
		AidAdd2 = locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(AidAdd2), AIDARRAY_INDEX_OF_AID))));
		AidLen2 = U16t8(get_array_length(MEMtPF8(AidAdd2)));
		AidAdd2 = get_array_data(MEMtPF8(AidAdd2));
		
//		aidBuffer[0] = AidLen2;
//		VMReadBuf(&aidBuffer[1],AidAdd2,AidLen2);

//		if((ecpMem_Compare(&USIMPartial[1],&aidBuffer[1],USIMPartial[0]) == 0) && (GetAIDNumber(aidBuffer,FALSE) != FALSE))//如果serverAID是ADF USIM 的aid，则转换成cos内部的UICCApplication的appid
//		{
//			return nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(UICC_APPID, SIZE_REF)));
//		}
#if UICC__OPERATOR__CHINAMOBILE
		if(IsUsimADF((u8*)AidAdd2, 0, AidLen2)== TRUE) //如果serverAID是ADF USIM 的aid，则转换成cos内部的UICCApplication的appid
		{
			return nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(getIndexByID(GetAppIdByAIDFromAIDList((u8*)GBAAppletAid,sizeof(GBAAppletAid))), SIZE_REF)));

			//return nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(UICC_APPID, SIZE_REF)));
		}
#endif
		for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
		{
			//if (0x00 != AidAdd1)  //not set value may be
			//{
				entry	= MEMtPN8(locateObjectInMemory(nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(i, SIZE_REF)))));
				AidAdd1	= locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(entry), theAID_INDEX))));

				//when aid is 0; check it immediately
				if(AidAdd1 == 0){
					continue;
				}

				AidAdd1 = locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(AidAdd1), AIDARRAY_INDEX_OF_AID))));
				AidLen1 = U16t8(get_array_length(MEMtPF8(AidAdd1)));
				AidAdd1 = get_array_data(MEMtPF8(AidAdd1));

				if((AidLen2 == AidLen1) && (memCompare(MEMtPM8(AidAdd1),MEMtPM8(AidAdd2),AidLen1) == ECP_CMP_RESULT_EQUAL))
				{
					return nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(i, SIZE_REF)));
				}
			//}
		}
	}
	return REF_NULL;
}

/**
*added by chentao@EPX产业化\STM_ST33G1M2\DEVELOP
*/
u16 SearchEntryByInstAID(u16 classaid)
{
	PN8			D_A			theAppTable;
	PN8			D_A			entry;
	u32			D_A			AidAdd1;
	u32			D_A			AidAdd2;
	u16			D_A			i;
	u16			D_A			bFreeEntryIndex;
	u8			D_A			AidLen1;
	u8			D_A			AidLen2;
	u8                    profileIndex;
	memref      X_A         profileEntryAddr;
	u16         X_A         ref;
#if (SE_AND_EUICC)

	bFreeEntryIndex		= nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
	theAppTable			= MEMtPN8(getAppTable());

	if(REF_NULL != classaid)
	{
		AidAdd2 = locateObjectInMemory(classaid);
		checkPreviousContextAccess(MEMtPF8(AidAdd2), 0);
		AidAdd2 = locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(AidAdd2), AIDARRAY_INDEX_OF_AID))));
		AidLen2 = U16t8(get_array_length(MEMtPF8(AidAdd2)));
		AidAdd2 = get_array_data(MEMtPF8(AidAdd2));

		/*for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
		{
			//if (0x00 != AidAdd1)  //not set value may be
			//{
				entry	= MEMtPN8(locateObjectInMemory(nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(i, SIZE_REF)))));
				AidAdd1	= locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(entry), theAID_INDEX))));

				//when aid is 0; check it immediately
				if(AidAdd1 == 0){
					continue;
				}

				AidAdd1 = locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(AidAdd1), AIDARRAY_INDEX_OF_AID))));
				AidLen1 = U16t8(get_array_length(MEMtPF8(AidAdd1)));
				AidAdd1 = get_array_data(MEMtPF8(AidAdd1));

				if((AidLen2 == AidLen1) && (memCompare(MEMtPM8(AidAdd1),MEMtPM8(AidAdd2),AidLen1) == ECP_CMP_RESULT_EQUAL))
				{
					return nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(i, SIZE_REF)));
				}
				//}
		}*/
		ref = getEntryByInstAID(classaid);//在当前环境下已找到相同的REF 直接返回 ，找不到继续按规则查找

		if(ref !=  REF_NULL)
		{
			return ref;
		}

		if(G_CurExecutingProfileIndex == ILLEGAL_PROFILEINDEX)//访问SE,SE和profile下所有的应用都需遍历，相同AID的返回。
		{
			for(profileIndex=0;profileIndex < PROFILE_MAX_NUMBER;profileIndex++)
			{
				profileEntryAddr = GetProfileEntryAddr(profileIndex);

				if(profileEntryAddr == ADDR_IS_NULL)
				{
					continue;
				}
				bFreeEntryIndex		= nvmReadU16(MEMtPF8(GetProfile_bFreeEntryIndexAddr(profileEntryAddr)));
				theAppTable			= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(nvmReadU16( MEMtPF8(GetProfile_theAppTableAddr(profileEntryAddr)))))));
//				for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
//				{
//					//if (0x00 != AidAdd1)  //not set value may be
//					//{
//						entry	= MEMtPN8(locateObjectInMemory(nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(i, SIZE_REF)))));
//						AidAdd1	= locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(entry), theAID_INDEX))));
//
//						//when aid is 0; check it immediately
//						if(AidAdd1 == 0){
//							continue;
//						}
//
//						AidAdd1 = locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(AidAdd1), AIDARRAY_INDEX_OF_AID))));
//						AidLen1 = U16t8(get_array_length(MEMtPF8(AidAdd1)));
//						AidAdd1 = get_array_data(MEMtPF8(AidAdd1));
//
//						if((AidLen2 == AidLen1) && (memCompare(MEMtPM8(AidAdd1),MEMtPM8(AidAdd2),AidLen1) == ECP_CMP_RESULT_EQUAL))
//						{
//							return nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(i, SIZE_REF)));
//						}
//					//}
//				}
				ref = SearchEntryFromSpecAppTableByInstAID(AidAdd2,AidLen2,theAppTable,bFreeEntryIndex);
				if(ref !=  REF_NULL)
				{
					return ref;
				}

			}


		}
		else//profile 只需遍历当前profile+SE下的应用
		{
			bFreeEntryIndex		= nvmReadU16(MEMtPN8(AM_bFreeEntryIndex_JC));
			theAppTable			= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(nvmReadU16( MEMtPN8(AM_theAppTable_JC))))));
//			for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
//			{
//				//if (0x00 != AidAdd1)  //not set value may be
//				//{
//					entry	= MEMtPN8(locateObjectInMemory(nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(i, SIZE_REF)))));
//					AidAdd1	= locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(entry), theAID_INDEX))));
//
//					//when aid is 0; check it immediately
//					if(AidAdd1 == 0){
//						continue;
//					}
//
//					AidAdd1 = locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(AidAdd1), AIDARRAY_INDEX_OF_AID))));
//					AidLen1 = U16t8(get_array_length(MEMtPF8(AidAdd1)));
//					AidAdd1 = get_array_data(MEMtPF8(AidAdd1));
//
//					if((AidLen2 == AidLen1) && (memCompare(MEMtPM8(AidAdd1),MEMtPM8(AidAdd2),AidLen1) == ECP_CMP_RESULT_EQUAL))
//					{
//						return nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(i, SIZE_REF)));
//					}
//				//}
//			}

			return SearchEntryFromSpecAppTableByInstAID(AidAdd2,AidLen2,theAppTable,bFreeEntryIndex);
		}


	}
#endif
	return REF_NULL;
}
/*@ -disspecopt @*/
jref addAppletInfo(jref aid, u8 bContext, u16 installoff)		SMALL_MODE
{
	jref		D_A		Entry;
	PN8			D_A		EntryAddr;

	Entry		= addEntry(aid, false);
	if (Entry != U32t16(REF_NULL))
	{
		EntryAddr	= MEMtPN8(locateObjectInMemory(Entry));
		store_object_B_filed(EntryAddr, bsdId_INDEX, g_bISDId[selected_channel]);//0
		store_object_B_filed(EntryAddr, theContext_INDEX, bContext);            // 2
		store_object_B_filed(EntryAddr, bCurrentState_INDEX, APP_STATE_LOADED); // 3
		store_object_S_filed(EntryAddr, installMethodAddr_INDEX, installoff);   // 23, 这个有没有必要声明成public
	}
	else
	{
		throw_error(SYSTEMEXCEPT_NO_RESOURCE);
	}
	return Entry;
}

/*
void appMgrcommit()
{

	PN8			D_A		theAppTable;
	PN8			D_A		entry;
	u16			D_A		i, bFreeEntryIndex;
	u8			D_A		bCurrentState;

	bFreeEntryIndex		= nvmReadU16(MEMtPN8(AM_bFreeEntryIndex_JC));
	theAppTable			= MEMtPN8(getAppTable());
	// mark all other applets in the same package as successfully loaded.
	for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
	{
		entry			= MEMtPN8(locateObjectInMemory(nvmReadU16(PN8_A_U16(theAppTable, U8_M_U8(i, SIZE_REF)))));
		bCurrentState	= PN_R_U8(MEMtPN8(get_object_field_ref(entry, bCurrentState_INDEX)));
		if (bCurrentState == APP_STATE_PRELOAD)
		{
			//store_object_B_filed(entry,bCurrentState_INDEX,APP_STATE_LOADED);
			memWriteU8NoAtomic((PM8)get_object_field_ref(entry, bCurrentState_INDEX), APP_STATE_LOADED);
		}
	}
}*/

memref getAppTable()
{
	if (check_addr_null(APP_Table_Cash))
	{
//		APP_Table_Object_Cash	= locateObjectInMemory(nvmReadU16(MEMtPN8(AM_theAppTable_JC)));
//		APP_Table_Object_Cash	= locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.AppTable));
		APP_Table_Cash			= get_array_data(MEMtPN8(locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.AppTable))));
	}
	return APP_Table_Cash;
}

u16 getIndexByID(u8 AppID)	SMALL_MODE
{

	PN8			D_A			theAppTable;
	PN8			D_A			entry;
	u16			D_A			i;
	u16			D_A			bFreeEntryIndex;
	u8			D_A			bAppID;

	bFreeEntryIndex		= nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
	theAppTable			= MEMtPN8(getAppTable());
	// mark all other applets in the same package as successfully loaded.
	for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
	{
		entry			= MEMtPN8(locateObjectInMemory(nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(i, SIZE_REF)))));
		bAppID			= memReadU8(get_object_field_ref(entry, bAppID_INDEX));
		if (bAppID == AppID)
		{
			return i;
		}
	}
	return 0xFFFF;
}
/*@ +disspecopt @*/
u16 getAppletFiled(u16 index,u8 getType)
{
	memref			D_A			aidAddr;
	jref			D_A			bPrivRef;
	memref			D_A			bPrivArray;
	u8 				D_A			bFirstPrivilageByte;

	if (index != APP_INDEX_NULL)
	{
		getAppTable();
		APP_Entry_Object_Cash = locateObjectInMemory(PN_R_U16(U32_A_U16(APP_Table_Cash, U16_M_U16(index, SIZE_REF))));
		aidAddr = locateObjectInMemory(PN_R_U16(PFtMEM(get_object_field_ref(MEMtPF8(APP_Entry_Object_Cash), theAID_INDEX))));

		//if((REF_NULL != aidAddr) && (REF_ADDRESS_NULL != aidAddr))
		if(REF_NULL != aidAddr)
		{
			switch(getType)
			{
			case GET_AID_ARRAY:
				return PN_R_U16(PFtMEM(get_object_field_ref(aidAddr, AIDARRAY_INDEX_OF_AID)));

			case GET_MEM_MANAGE:

			  break;

			case GET_SDID:
				return 	U8t16(memReadU8(MEMtPN8(get_object_field_ref(MEMtPF8(APP_Entry_Object_Cash), bsdId_INDEX))));

			case GET_FirstSDID:
				bPrivRef = memReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(APP_Entry_Object_Cash), bPriv_INDEX)));
				bPrivArray = get_array_data(MEMtPF8(locateObjectInMemory(bPrivRef)));
				bFirstPrivilageByte = memReadU8(MEMtPM8(bPrivArray));
				if(U8_AND_U8(bFirstPrivilageByte, 0x80) == 0x80)
				{
					return 	U8t16(memReadU8(MEMtPN8(get_object_field_ref(MEMtPF8(APP_Entry_Object_Cash), bAppID_INDEX))));
				}
				else
				{
					return 	U8t16(memReadU8(MEMtPN8(get_object_field_ref(MEMtPF8(APP_Entry_Object_Cash), bsdId_INDEX))));
				}
			}
		}
	}
	return  APP_INDEX_NULL;
}
/**
*added by chenttao@EPX产业化\STM_ST33G1M2\DEVELOP
*/
#if (EUICC_ONLY||SE_AND_EUICC)
void getISDRAID(void)
{
	u8		D_A    aidLength;
	PN8		D_A    pbBuffer;

	pbBuffer	= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(popReference()))));
	aidLength	= JcvmConfig_GetISDRAidLength();
	PXB_W_U8(pbBuffer, aidLength);
	ecpMem_Copy(MEMtPN8(&pbBuffer[1]), MEMtPN8(JcvmConfig_GetISDRAidBase()), aidLength);
}
#endif
void getManagerAID(void)
{
	u8		D_A    aidLength;
	PN8		D_A    pbBuffer;

	pbBuffer	= MEMtPN8(getJavaObjectDataArray());
	aidLength	= JcvmConfig_GetCmAidLength();
	PXB_W_U8(pbBuffer, aidLength);
	ecpMem_Copy(MEMtPN8(&pbBuffer[1]), MEMtPN8(JcvmConfig_GetCmAidBase()), aidLength);
}

#if LTE__OTA
u8 getAppletInformation(u8 appId, u8 type)
{

	u8		D_A		bAppID;
	u8		D_A		bCurrentStatus;
	u8		D_A		bPreStatus;
	u8		D_A		bPrivelege;
	u8		D_A		bContext;
	u16		D_A		j;
	u16		D_A		bFreeEntryIndex;
	memref  D_A		theAppTable;
	memref  D_A		entry;
	
	bFreeEntryIndex = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
    theAppTable = getAppTable();
	for (j = APP_FIRST; j < bFreeEntryIndex; j++)
    {
		entry = locateObjectInMemory(PN_R_U16(theAppTable + j * SIZE_REF));
		bAppID = memReadU8(MEMtPM8(get_object_field_ref(entry, bAppID_INDEX)));
		if(bAppID == appId)
        {
        	bCurrentStatus = memReadU8(MEMtPM8(get_object_field_ref(entry, bCurrentState_INDEX)));
        	bPreStatus = memReadU8(MEMtPM8(get_object_field_ref(entry, bPreviousState_INDEX)));
        	bPrivelege = memReadU8(MEMtPM8(get_object_field_ref(entry, bPriv_INDEX)));
        	bContext = memReadU8(MEMtPM8(get_object_field_ref(entry, theContext_INDEX)));
			switch(type)
			{
				case APPLET_PRE_STATUS:
					return bPreStatus;
				case APPLET_CURRENT_STATUS:
					return bCurrentStatus;
				case APPLET_PRIVELEGE:
					return bPrivelege;
				case APPLET_CONTEXT:
					return bContext;
				default:
					//return -1;
					return 0xFF;
			}
        }
    }
	return 0;
}
#endif

u8 getAssociateSDId(u8 AppID)
{
	memref		D_A		theAppTable;
	memref		D_A		entry;
	u16			D_A		bFreeEntryIndex;
	u16			D_A		j;
	u8			D_A		bAppID;
	u8			D_A		sdID;

	sdID = 0xFF;

	/*@ -disspecopt @*/bFreeEntryIndex = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);/*@ +disspecopt @*/
	theAppTable = getAppTable();

	for (j = APP_FIRST; j < bFreeEntryIndex; U16_INC(j))
	{
		entry = locateObjectInMemory(PN_R_U16(U32_A_U16(theAppTable, U16_M_U16(j, SIZE_REF))));
		bAppID = nvmReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(entry), bAppID_INDEX)));
		if(bAppID == AppID)
		{
			sdID = nvmReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(entry), bsdId_INDEX)));
			if(sdID != AppID)
			return sdID;
		}
	}
	return 0xFF;
}

memref getSdEntryObj(u8 appId)
{
	u8				appIndex;
	memref			entryAdd;

	appIndex		= U16t8(getIndexByID(appId));
	if(0xFF == appIndex)
	{
		entryAdd	=  MEM_NULL;
	}
	else
	{
		entryAdd	= locateObjectInMemory(memReadU16(MEMtPM8(U32_A_U16(APP_Table_Cash,U8_M_U16(SIZE_REF,appIndex)))));
	}

	return 	entryAdd;
}

#if GP__AMENDMENT__C__FOR_MASK
u8 isHaveVolatilePriority(memref theEntryObjectAddr)
{
	u8      volatilePriority;
	memref  clAppEntryAddr;


	clAppEntryAddr = locateObjectInMemory(PN_R_U16(get_object_field_ref(theEntryObjectAddr, clAppEntry_INDEX)));

	if((REF_NULL != clAppEntryAddr) && (MEM_NULL != clAppEntryAddr) )
	{
		volatilePriority = PN_R_U8(get_object_field_ref(MEMtPF8(clAppEntryAddr), VolatilePriority_INDEX));
		if(volatilePriority)
		{
			return 1;
		}
	}

	return 0;
}
#endif
/*
 * 按照优先级根据 bStartIndex 查找下一个 Entry
 * 输入 : AppTable 的index -1,0,1,2,3... (-1表示获取最高优先级的项)
 * 输出 : AppTable 的index -1,0,1,2,3... (-1表示没有找到)
 * 异常情况 : 当bStartIndex有效但对应项为空时则顺序查找顺序查找无 VolatilePriority 的项
 */
u16 GetNextAppEntryIdxByPriority(u16 bStartIndex)
{
	u16 		D_A			bFreeEntryIndex;
	bFreeEntryIndex = PF_R_U16(G_CurExecuEnvironment.bFreeEntryIndex);
	getAppTable();
	
	if(bStartIndex == APP_INDEX_NULL) //查找最高优先级
	{
#if GP__AMENDMENT__C__FOR_MASK
		if(bRamBuffer[AMDC_VOLATILE_PRIORITY_TAG] == NEED_VOLATILE_AMENDMENTC_SEARCH)
		{
			bRamBuffer[AMDC_VOLATILE_PRIORITY_TAG] = VOLATILE_AMENDMENTC_APP_EXIST ;
		}
#endif
		bStartIndex = APP_FIRST;
	}
	else
	{
		U16_INC(bStartIndex);
	}
#if GP__AMENDMENT__C__FOR_MASK
	if(bRamBuffer[AMDC_VOLATILE_PRIORITY_TAG] == VOLATILE_AMENDMENTC_APP_EXIST)
	{
		return getVolatileApplet_AmendmentC(bStartIndex, SEARCH_NEXT);
	}
#endif
	if(bStartIndex >= bFreeEntryIndex)
	{
		return APP_INDEX_NULL;
	}
	else
	{
         return bStartIndex ;
	}
}

#if GP__AMENDMENT__C__FOR_MASK
u16 getVolatileApplet_AmendmentC(u16 bStartIndex, s8 direction)
{
	memref 		D_A			theEntryObjectAddr;
	u16 		D_A			bFreeEntryIndex;
	u16			D_A			i;
	bFreeEntryIndex = PF_R_U16(G_CurExecuEnvironment.bFreeEntryIndex);

	for (i = bStartIndex; i < bFreeEntryIndex && i != APP_INDEX_NULL; ) //先顺序查找有 VolatilePriority 的项
	{
		theEntryObjectAddr = locateObjectInMemory(PN_R_U16(U32_A_U16(APP_Table_Cash, U16_M_U16(i, SIZE_REF))));

		if (isHaveVolatilePriority(theEntryObjectAddr))
		{
			return i;
		}

		if ((APP_FIRST == i) && 0 > direction)
		{ // end for "previous" searching, and it can search next keeping using volatile
			return APP_INDEX_NULL;
		}

		if (direction > 0)
		{
			U16_INC(i);
		}
		else
		{
			U16_DEC(i);
		}
	}

	bRamBuffer[AMDC_VOLATILE_PRIORITY_TAG] = NEED_VOLATILE_AMENDMENTC_SEARCH  ;
	return  APP_FIRST ;
}
#endif

/**
 * select previous
 * lowest -> non -> highest -> voliatile
 */
u16 GetPreviousAppEntryIdxByPriority(u16 bStartIndex)
{
#if GP__AMENDMENT__C__FOR_MASK
	u16 		D_A			bFreeEntryIndex;
	u16			D_A			backup_bStartIndex;
#endif

#if GP__AMENDMENT__C__FOR_MASK
	backup_bStartIndex								= bStartIndex;
#endif
	getAppTable();

	if(APP_FIRST 	!= bStartIndex)
	{
		U16_DEC(bStartIndex);
	}
#if GP__AMENDMENT__C__FOR_MASK

	//next已经探索完volatile,previous向前搜索完成匹配应用,才能开始临时优先搜索,
	if((bRamBuffer[ AMDC_VOLATILE_PRIORITY_TAG ] 	   == NEED_VOLATILE_AMENDMENTC_SEARCH))
	{
		if(APP_FIRST == backup_bStartIndex)
		{
			bFreeEntryIndex 	= PF_R_U16(G_CurExecuEnvironment.bFreeEntryIndex);
			U16_DEC(bFreeEntryIndex);
			bStartIndex 						     = bFreeEntryIndex;
			bRamBuffer[ AMDC_VOLATILE_PRIORITY_TAG ] = VOLATILE_AMENDMENTC_APP_EXIST ;
			bStartIndex 						     = getVolatileApplet_AmendmentC(bStartIndex, SEARCH_PREVIOUS);
		}
	}

	//next没搜索完volatile,或者previous当前已经搜索到volatile,则向前继续搜完即止
	else if(bRamBuffer[ AMDC_VOLATILE_PRIORITY_TAG ] 	== VOLATILE_AMENDMENTC_APP_EXIST)
	{
		bStartIndex 							   =  getVolatileApplet_AmendmentC(bStartIndex, SEARCH_PREVIOUS);
	}

	//如果不需要搜索临时优先级应用，且已经探索到头，应当结束
	else if(APP_FIRST == backup_bStartIndex)
	{
		bStartIndex 							   = APP_INDEX_NULL;
	}
#else
	else
	{
		bStartIndex 							   = APP_INDEX_NULL;
	}
#endif
	return bStartIndex;

}


u16 GetAppEntryIdxByPriority(u16 bStartIndex, s8 direction)
{
	if(direction < 0)
	{
		return GetPreviousAppEntryIdxByPriority(bStartIndex);
	}
	else
	{
		return GetNextAppEntryIdxByPriority(bStartIndex);
	}
}

u16 GetImplicitSelectedAppletIdNM()
{
	u8 ioInterface;
	u8 logicalChannel;
	ioInterface		= U16t8(popShort());
	logicalChannel	= U16t8(popShort());
	return GetImplicitSelectedAppletId(ioInterface, logicalChannel);
}
/**
 * 在当前逻辑通道和当前IO通道上选择默认应用
 * b8  b7  b6  b5  b4  b3  b2  b1  Meaning                                    
	1   -   -   -   -   -   -   -   Contactless I/O                 
	-   1   -   -   -   -   -   -   Contact I/O                     
	-   -   X   -   -   -   -   -   RFU                             
	-   -   -   X   X   X   X   X   Logical channel number (0 to 19)
 * @param ioInterface
 * @param logicalChannel
 * @return appletIndex
 */
u16 GetImplicitSelectedAppletId(u8 ioInterface,u8 logicalChannel)
{
	u16	 		D_A			resetSelect;
	u8 			D_A			implicitPara;
	u8 			D_A			ioimplicitSelect;
//    u8 			D_A			lcimplicitSelect;
	u16 		D_A			i;
	u32 		D_A			theEntryObjectAddr;
	u8 			D_A			io7816implicitSelect_;
	u8 			D_A			ioSwpimplicitSelect_;
	u16 		D_A			bPrivRef ;
	memref 		D_A			bPrivArray ;
	u8 			D_A			bFirstPrivilageByte;
#if HIGH_FALUT_INJECTION_RESIST
	u8 			D_A			bSecondPrivilageByte;
	u8 			D_A			bThirdPrivilageByte ;
	u8 			D_A			binstallprivlen;
#endif

#if (EUICC_ONLY||SE_AND_EUICC)
//	u8          D_A         bakCurExecutingProfileIndex;
	u8          D_A         channelProfileIndex;
#endif

	resetSelect = APP_INDEX_NULL;

	implicitPara = ((ioInterface == 0)? U32t8(0x40) : U32t8(0x80));

#if (EUICC_ONLY||SE_AND_EUICC)
	channelProfileIndex = getChannelProfileIndex(logicalChannel);

//	bakCurExecutingProfileIndex = G_CurExecutingProfileIndex;

	if(G_CurExecutingProfileIndex != channelProfileIndex)
	{
		G_CurExecutingProfileIndex = channelProfileIndex;

		ExecuEnvironmentInit(G_CurExecutingProfileIndex);
	}
#endif
	getAppTable();

//=========================================================================================

	ioimplicitSelect = 0;

	
//=========================================================================================
	for (i = GetNextAppEntryIdxByPriority(0xFFFF); i != 0xFFFF; i = GetNextAppEntryIdxByPriority(i))
	{
		theEntryObjectAddr = locateObjectInMemory(memReadU16(MEMtPM8(U32_A_U16(APP_Table_Cash, U16_M_U16(i, SIZE_REF)))));
		
		if(memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(theEntryObjectAddr), bAppID_INDEX))) == APP_NULL)
		{
			continue;
		}

		io7816implicitSelect_ = memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(theEntryObjectAddr), iso7816ImplicitSelection_INDEX)));
		ioSwpimplicitSelect_ = memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(theEntryObjectAddr), swpImplicitSelection_INDEX)));


		#if HIGH_FALUT_INJECTION_RESIST
		bSecondPrivilageByte	= memReadU8(MEMtPM8(U32_A_U8(bPrivArray, 1)));
		bThirdPrivilageByte		= memReadU8(MEMtPM8(U32_A_U8(bPrivArray, 2)));
		binstallprivlen = memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(theEntryObjectAddr), binstallprivlen_INDEX)));
		#endif

		if (implicitPara == 0x40)
		{
			ioimplicitSelect = io7816implicitSelect_;
		}
		else
		{
			ioimplicitSelect = ioSwpimplicitSelect_;
		}

		if ( U8_AND_U8(ioimplicitSelect, U8_SHL_U8(1, logicalChannel)) != 0 )
		{
			resetSelect = i;
			break;
		}


#if HIGH_FALUT_INJECTION_RESIST
		//checkPrivIntegrity();
		if((bFirstPrivilageByte ^ bSecondPrivilageByte ^ bThirdPrivilageByte ^ binstallprivlen) != bPrivCheckSum)
		{
			faultDetected();
		}
#endif
	}
	
	if(resetSelect != APP_INDEX_NULL) {
		u32 theEntryObjectAddr = locateObjectInMemory(memReadU16(MEMtPM8(U32_A_U16(APP_Table_Cash, U16_M_U16(resetSelect, SIZE_REF)))));
/*@ -type @*/
/*@ -discast @*/
		u8 bCurrentState = (jbyte)memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(theEntryObjectAddr), bCurrentState_INDEX)));
/*@ +discast @*/
/*@ +type @*/
		bPrivRef = memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(theEntryObjectAddr), bPriv_INDEX)));
		bPrivArray = get_array_data(MEMtPF8(locateObjectInMemory(bPrivRef)));
		bFirstPrivilageByte = memReadU8(MEMtPM8(bPrivArray));
		if(U8_AND_U8(bFirstPrivilageByte, 0x01) == U32t8(0x01)) {
    		if(bCurrentState == U32t8(0xFF)
    			|| bCurrentState == U32t8(0x7F)) {
    			resetSelect = APP_FIRST;	//select ISD 对应的AppletIndex
    		}
		} else {
			if(U8_AND_U8(bCurrentState,U32t8(0x80)) == U32t8(0x80)) {
				resetSelect = APP_FIRST;	//select ISD 对应的AppletIndex
			}
		}
	}

//	#if (EUICC_ONLY||SE_AND_EUICC) //隐式选择（无需调deselect） 选中后保持G_CurExecutingProfileIndex为当前被选中应用的区域
//	if(G_CurExecutingProfileIndex != bakCurExecutingProfileIndex)
//	{
//		G_CurExecutingProfileIndex = bakCurExecutingProfileIndex;
//
//		ExecuEnvironmentInit(G_CurExecutingProfileIndex);
//	}
//	#endif

	return resetSelect;
}

/*
*@function     :GetGBAAppletEntry
*
*@description  :获取GBAApplet的Entry。该函数用于UICC应用的接口getShareableInterfaceObject获取GBA应用信息
*
*@param        :
*
*@return       :GBAApplet的Entry
*
*@history      :V1.0 created by lx@20211019
*/
void GetGBAAppletEntry(void)
{
#if UICC__OPERATOR__CHINAMOBILE
	pushReference(nvmReadU16(PN8_A_U16(MEMtPN8(getAppTable()), U16_M_U16(getIndexByID(GetAppIdByAIDFromAIDList((u8*)GBAAppletAid,sizeof(GBAAppletAid))), SIZE_REF)))); //测试用例：V2X安全应用测试库
#else
	pushReference(REF_NULL);
#endif
}
#if RSP_EUICC
/*
*@function     : SaveICCIDAndAID
*
*@description  :德国特殊需求：SE区和profile区的应用AID重复的情况，忽略profile中的应用，同时记录冲突profile的ICCID和AID
				
*
*@param       
*
*@return       
*
*@RAM Resources:
*
*@hisotry      :V1.0 created by chentao@20230220
*              
*/	
void SaveICCIDAndAID()
{
	u8						AidLen,offset;
	u16 					AidObj,off;
	memref 					AidAdd;
	u8                      tmp[0x25];

	AidObj =  popShort();

	if(Germany_Mobile)
	{
		ecpMem_Set(tmp,0x00,sizeof(tmp));

		offset = 0x01;

		tmp[offset++] = 0xE3;
		offset++;                        //偏移过保存信息的长度

		GetProfileMetadaTLV(IF_GetDownloadProfileIndex(),MetaDataIccid_INDEX,tmp+offset);
		offset += 0x0C;

		if(AidObj != REF_NULL)
		{
			AidAdd = locateObjectInMemory(AidObj);
			AidAdd = locateObjectInMemory(memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(AidAdd), AIDARRAY_INDEX_OF_AID))));
			AidLen = U16t8(get_array_length(MEMtPF8(AidAdd)));
			off = 0;
		}

		AidAdd = U32_A_U16(get_array_data(MEMtPF8(AidAdd)),off);

		tmp[offset++] = 0x4F;    //  AID tag
		tmp[offset++] = AidLen;

		VMReadBuf(tmp+offset, (UWORD)AidAdd, AidLen);
		offset += AidLen;

		tmp[0] = offset-1;  //总信息长度
		tmp[2] = offset-3;  //profile信息长度

		VMWriteBuf(EUICC_ICCIDAndAID, (u8*)tmp, offset);
	}
	else
	{
		//ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}

	return;
}
#endif
