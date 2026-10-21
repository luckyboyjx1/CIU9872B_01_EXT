/*
 * AppEdepApi.c
 *
 *  Created on: 2018Äê9ÔÂ6ÈÕ
 *      Author: jiangxiaohui
 */

#include "profile.h"

#if VM_WITH_EDEP

#include "Apdu.h"
#include "mask.h"
#include "JcvmConfig.h"
#include "utilMethod.h"
#include "bc_object.h"
#include "ecpPBOC_Main.h"
#include "AppAPI.h"
#include "EDEP_CmdInterface.h"
#include "ecpPBOC_Constants.h"
#include "ecpPublic_GlobalVar.h"
#include "anomaly.h"
#include "Mifare.h"


extern u1 	B_A		io_rfParamIsInit;

void JcvmConfig_GetConstructAuthChipBase(PX8 buf)
{
	ecpMem_Copy(buf, MEMtPM8(0x14000050), 9);
}

jref getAppletClassAid(u8 index)
{
		PN8			D_A			aidAddr;

	if (index != APP_NULL)
	{
		getAppTable();
		APP_Entry_Object_Cash	= locateObjectInMemory(nvmReadU16(PN8_A_U16(MEMtPN8(APP_Table_Cash), U8_M_U8(index, SIZE_REF))));
		aidAddr					= MEMtPN8(locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(APP_Entry_Object_Cash, classAID_INDEX)))));
		if (PNtMEM(aidAddr) != 0)
		{
			return nvmReadU16(MEMtPN8(get_object_field_ref(aidAddr, AIDARRAY_INDEX_OF_AID)));
		}
	}
	return APP_NULL;
}

void AppAPI_ConstructAuthCode(PF8 buf)     SMALL_MODE
{
	u8 Array[0x09];
	
	PATCH_APPEDEP_API_001();
	if(0x01 == JcvmConfig_GetConstructAuthSel())
	{
		memCopyAtomic(buf, MEMtPM8(JcvmConfig_GetConstructAuthCodeBase()), 9);
	}
	else
	{
		//memCopyAtomic(buf, MEMtPM8(JcvmConfig_GetConstructAuthChipBase()), 9);
		JcvmConfig_GetConstructAuthChipBase(Array);
		memCopyAtomic(buf, Array, 9);
	}
	
	return;
}


u8 PbocAPI_IsGetConstructAuthCode(PX8 apduBuf)    SMALL_MODE
{
	u8     D_A     result;

	result	= false;

	if (PXB_R_U8(&apduBuf[OFFSET_CLA]) == (0x80 | selected_channel) &&
			PXB_R_U8(&apduBuf[OFFSET_INS]) == 0xCA &&
			PXB_R_U8(&apduBuf[OFFSET_P1]) == 0x00 &&
			PXB_R_U8(&apduBuf[OFFSET_P2]) == 0x00 &&
			PXB_R_U8(&apduBuf[OFFSET_LC]) == 0x09)
	{
		PXB_W_U8(&bRamBuffer[SELECTING_APPLET_FLAG],0xFD);
		AppAPI_ConstructAuthCode(apduBuf);
		apdu_ApduSetOutgoingAndSend(0,9);
		result	= true;
	}

	return result;
}

u8 EdepAPI_IsSearchEdepAdf(void)
{
	u8 bState;

	PbocAPI_GetConfig(ECP_TYPE_EDEP_PRODUCT_S,(PX8)(&bState));
	if((ECP_TYPE_EDEP != g_PBOC_instType) && ((bState & 0x40) == 0x40))
	{

		bState = SearchEdepADF();
		if(bState != 0xFF)
		{
			PXB_W_U8(&bRamBuffer[SELECTING_APPLET_FLAG],bState);
			return true;
		}
	}
	return false;
}

u16 mifare_Derive4UidFrom7Uid(void)
{
	return 0;
}

void mifare_switchON0FF(u1 switchON)
{
	PN8	mifareConfigAddress;
	u8	mifareConfig;
	
	PATCH_BOOT_MIFARE_006();
	mifareConfigAddress		= MEMtPN8(sysConfig_GetMifareConfBase());
	mifareConfig			= PN_R_U8(mifareConfigAddress);
	if(switchON)
	{
		mifareConfig		= U8_OR_U8(mifareConfig, IOCONFIG_MIFARE_CONF_ON_MASK);
		mifare_config(0);
	}
	else
	{
		mifareConfig		= U8_AND_U8(mifareConfig, IOCONFIG_MIFARE_CONF_OFF_MASK);
		iso14443TaL3_Config();
	}

	ecpCiu_SetAtqa((phIso14443L3_control.atqa.b9b16.wholeByteH << 8) + phIso14443L3_control.atqa.b1b8.wholeByteL);
	ecpCiu_SetUID(phIso14443L3_control2.uid);
	ecpCiu_SetSakComplete(phIso14443L3_control.sakComplete);
	ecpCiu_SetSakInComplete(phIso14443L3_control.sakIncomplete);
	
	nvmWriteU8Atomic(mifareConfigAddress, mifareConfig);
	return;
}

u1 mifare_switchStatus(void)
{
	return (IOCONFIG_MIFARE_CONF_ON_MASK == sysConfig_IsMifareOs());
}

u16 nativeMethods_readSecurityCodeDataNative(void)
{
	jref	D_A  bufRef;
	PF8		D_A  buf;
	u16		D_A  offset;
	
	PATCH_JCVM_VM_NATIVEMETHOD_IMPL_010();	
	
	offset		= popShort();
	bufRef		= popReference();
		
	buf			= MEMtPF8(locateObjectInMemory(bufRef));
	if(MEMREF_NULL == PFtMEM(buf))
	{
		throw_error(NULL_POINTER_EXCEPTION);
	}
	check_bounds_interval(buf, offset, 9);
	
	buf			= MEMtPF8(get_array_data(buf));
	buf			= PF8_A_U16(buf, offset);

	AppAPI_ConstructAuthCode(buf);

	return 0x00;
}

void io_rfParamInit()
{
	if((false == io_rfParamIsInit) && (APDU_PROTOCOL_TA != io_protocolType))
	{
		iso14443Ta_Config();
		io_rfParamIsInit = true;
	}
}

u8 io_GetSakComplete(void)
{
	io_rfParamInit();
	return iso14443Ta_GetSakComplete();
}

u8 io_GetSakIncomplete(void)
{
	io_rfParamInit();
	return iso14443Ta_GetSakIncomplete();
}

u8 io_GetUid(PX8 uidAry)
{
	io_rfParamInit();
	return iso14443Ta_GetUid(uidAry);
}

u16 io_GetAtqa(void)
{
	io_rfParamInit();
	return iso14443Ta_GetAtqa();
}


void iso14443TaL3_SetSakComplete(u8 sakComplete)
{
	PN8	taConfigAddress;
	u8	taConfig;
	
	PATCH_BOOT_APDU_IO14443TAL3_011();
	
	phIso14443L3_control.sakComplete = sakComplete;

	taConfigAddress	= MEMtPN8(sysConfig_GetTaConfBase());
	taConfig		= PN_R_U8(sysConfig_GetTaConfBase());
	taConfig		= U8_OR_U8(taConfig, IOCONFIG_TA_CONF_USERSAK_COMPLETE_MASK);

//	memBeginTransaction(SINGLE_APDU_PROCESS);

	nvmWriteU8Atomic(taConfigAddress, taConfig);
	nvmWriteU8Atomic(MEMtPN8(sysConfig_GetTaUsersakCompleteBase()), sakComplete);

	ecpCiu_SetSakComplete(sakComplete);
//	memCommitTransaction();
}

u8 iso14443TaL3_GetSakIncomplete(void)
{
	PATCH_BOOT_APDU_IO14443TAL3_012();
	return phIso14443L3_control.sakIncomplete;
}

void iso14443TaL3_SetSakIncomplete(u8 sakIncomplete)
{
	PN8	taConfigAddress;
	u8	taConfig;

	PATCH_BOOT_APDU_IO14443TAL3_013();
	
	phIso14443L3_control.sakIncomplete = sakIncomplete;

	taConfigAddress	= MEMtPN8(sysConfig_GetTaConfBase());
	taConfig		= PN_R_U8(sysConfig_GetTaConfBase());
	taConfig		= U8_OR_U8(taConfig, IOCONFIG_TA_CONF_USERSAK_INCOMPLETE_MASK);

//	memBeginTransaction(SINGLE_APDU_PROCESS);

	nvmWriteU8Atomic(taConfigAddress, taConfig);
	nvmWriteU8Atomic(MEMtPN8(sysConfig_GetTaUsersakInCompleteBase()), sakIncomplete);

	ecpCiu_SetSakInComplete(sakIncomplete);
	
//	memCommitTransaction();
}

u8 iso14443TaL3_GetUid(PX8 uidAry)
{
	u8 length;
	
	PATCH_BOOT_APDU_IO14443TAL3_014();
	
	ecpMem_Copy(MEMtPX8(&uidAry[0]), MEMtPX8(&phIso14443L3_control2.uid[0]), 4);
	length = 4;
	if(phIso14443L3_control2.uid[0] == 0x88)
	{
		ecpMem_Copy(MEMtPX8(&uidAry[4]), &phIso14443L3_control2.uid[5], 4);
		length = U8_A_U8(length, 4);

		if(phIso14443L3_control2.uid[5] == 0x88)
		{
			ecpMem_Copy(MEMtPX8(&uidAry[8]), &phIso14443L3_control2.uid[10], 4);
			length = U8_A_U8(length, 4);
		}
	}

	return length;
}

void iso14443TaL3_SetUid(PM8 uidAry)
{
	PN8	taConfigAddress;
	u8	taConfig;
	
	PATCH_BOOT_APDU_IO14443TAL3_015();
	
	taConfigAddress	= MEMtPN8(sysConfig_GetTaConfBase());
	taConfig		= PN_R_U8(sysConfig_GetTaConfBase());
	taConfig		= U8_OR_U8(taConfig, IOCONFIG_TA_CONF_USERUID_MASK);

//	memBeginTransaction(SINGLE_APDU_PROCESS);

	nvmWriteU8Atomic(taConfigAddress, taConfig);
	nvmCopyAtomic(MEMtPN8(sysConfig_GetTAUIDBase()), MEMtPM8(&uidAry[0]), 4);
	ecpMem_Copy(&phIso14443L3_control2.uid[0], &uidAry[0], 4);
	phIso14443L3_control2.uid[4]	= U8_XOR_U8(phIso14443L3_control2.uid[0], U8_XOR_U8(phIso14443L3_control2.uid[1], U8_XOR_U8(phIso14443L3_control2.uid[2], phIso14443L3_control2.uid[3])));
	if(phIso14443L3_control2.uid[0] == 0x88)
	{
		nvmCopyAtomic(MEMtPN8(U32_A_U8(sysConfig_GetTAUIDBase(), 4L)), &uidAry[4], 4);
		ecpMem_Copy(&phIso14443L3_control2.uid[5], &uidAry[4], 4);
		//	BCC is calculated for the UID part two.
		phIso14443L3_control2.uid[9]	= U8_XOR_U8(phIso14443L3_control2.uid[5], U8_XOR_U8(phIso14443L3_control2.uid[6], U8_XOR_U8(phIso14443L3_control2.uid[7], phIso14443L3_control2.uid[8])));
		if(phIso14443L3_control2.uid[5] == 0x88)
		{
			nvmCopyAtomic(MEMtPN8(U32_A_U8(sysConfig_GetTAUIDBase(), 8L)), &uidAry[8], 4);
			ecpMem_Copy(&phIso14443L3_control2.uid[10], &uidAry[8], 4);
			//	BCC is calculated for the UID part three
			phIso14443L3_control2.uid[14]	= U8_XOR_U8(phIso14443L3_control2.uid[10], U8_XOR_U8(phIso14443L3_control2.uid[11], U8_XOR_U8(phIso14443L3_control2.uid[12], phIso14443L3_control2.uid[13])));
		}
	}
	ecpCiu_SetUID( phIso14443L3_control2.uid );
//	memCommitTransaction();
}

u16  iso14443TaL3_GetAtqa(void)
{
	PATCH_BOOT_APDU_IO14443TAL3_016();
    return U16_OR_U16(U16_SHL_U8(phIso14443L3_control.atqa.b9b16.wholeByteH,8),phIso14443L3_control.atqa.b1b8.wholeByteL);
}

void iso14443TaL3_SetAtqa(PM8 atqaArray)
{
	PN8	taConfigAddress;
	u8	taConfig;
	u16 curAtqa;
	
	PATCH_BOOT_APDU_IO14443TAL3_017();
	
    curAtqa = PM_R_U16(atqaArray);
	phIso14443L3_control.atqa.b1b8.wholeByteL = U16t8(curAtqa);
	phIso14443L3_control.atqa.b9b16.wholeByteH = U16_SHR_U8(curAtqa,8);
	taConfigAddress	= MEMtPN8(sysConfig_GetTaConfBase());
	taConfig		= PN_R_U8(sysConfig_GetTaConfBase());
	taConfig		= U8_OR_U8(taConfig, IOCONFIG_TA_CONF_USERATQ_MASK);
//	memBeginTransaction(SINGLE_APDU_PROCESS);
	nvmWriteU8Atomic(taConfigAddress, taConfig);
	nvmWriteU16Atomic(MEMtPN8(sysConfig_GetTaUserATQBase()), curAtqa);
//	memCommitTransaction();
	
	ecpCiu_SetAtqa(curAtqa);
	return;
}


#endif
