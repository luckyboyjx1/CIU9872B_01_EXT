/*
 * cmgLoadTime.c
 *
 *  Created on: 2015Äê10ÔÂ21ÈÕ
 *      Author: liuhanren
 */

#include "CgmLoad.h"
#include "CgmManager.h"

#if GP__CGM__FOR_MASK
void CgmLoad_ConsumeCumulate(u32 pSize, u8 cgm_mask)
{
	if(U8_AND_U8(cgm_mask, HAS82_MASK))
	{
		g_UsedRAM = U32_A_U32(g_UsedRAM,pSize);
	}
	else
	{
		if(g_systemConsumeConfig != 0x00)
		{
			pSize = U32_S_U32(pSize, MAP_ENTRY_SIZE);
		}

		g_UsedNVM = U32_A_U32(g_UsedNVM,pSize);
	}
}



void CgmLoad_End(u16 entryIndex)
{
	u8						sdId;
	u8  					pkgId;
	u32						consmueNvm;
	PN8				 		pkgEntryAddr;
	PN8						pkgUsedNvmAddr;

	consmueNvm	 		   = g_UsedNVM;
	pkgId 				   = findPkgID(bRamBuffer, LOAD_PKG_AID_OFF, bRamBuffer[LOAD_PKG_LEN_OFF], 0);
	pkgEntryAddr		   = MEMtPN8(locateObjectInMemory(getPkgEntry(pkgId)));
	pkgUsedNvmAddr 		   = CgmManager_getPkgUsedNvmObj(pkgEntryAddr);


	nvmWriteU32Atomic(pkgUsedNvmAddr, consmueNvm);

	sdId 				   = CgmManager_getAppIdFromEntry(entryIndex);
	CgmManager_SetSdMenInfo(sdId, 0, consmueNvm, SD_DECREASE_SIZE);
	return;
}
#endif
#if GP__CGM__FOR_APPPARACHECK
void CgmLoad_error(void)
{
    bRamBuffer[INSTALLER_STATE_INDEX] = INSTALLER_STATE_ERROR;
	bRamBuffer[INSTALL_FOR_LOAD] =U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], U8_INV(U8_SHL_U8(1, selected_channel)));
}

void CgmLoad_LimitCheck(u8 appID)
{
	u8				error;
	u16				message;

	error			= false;

	if(CgmField_getSpaceInfo(appID, SD_REMAIN_RAM_CHECK) == 1)
	{
		error		= true;
		message		= SYSTEMEXCEPT_NO_TRANSIENT_SPACE;
	}
	if(CgmField_getSpaceInfo(appID, SD_REMAIN_NVM_CHECK) == 1)
	{
		error		= true;
		message		= SYSTEMEXCEPT_NO_RESOURCE;
	}

	if(error)
	{
		CgmLoad_error() ;
		CgmManager_error(message);
	}
}
#endif
