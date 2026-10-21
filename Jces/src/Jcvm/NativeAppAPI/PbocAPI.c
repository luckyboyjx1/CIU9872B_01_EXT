/*
 * PbocAPI.c
 *
 *  Created on: 2014年6月6日
 *      Author: linguoming1
 */

#include "PbocAPI.h"

#if VM_WITH_PBOC_OR_EDEP

#include "opFunction.h"
#include "memOperation.h"
#include "Apdu.h"
#include "mask.h"
#include "JcvmConfig.h"
#include "utilMethod.h"
#include "bc_object.h"
#include "ecpPBOC_Main.h"
#include "AppAPI.h"
#if VM_WITH_EDEP
#include "EDEP_CmdInterface.h"
#endif
#include "anomaly.h"

extern u16     X_A     gBackUp_curOffset;

#if VM_WITH_JT_FUNC
void AppAPI_GET_SHARE_TLV_MAX_RECORDS(PF8 buf)     SMALL_MODE
{
	memCopyAtomic(buf, MEMtPM8(JcvmConfig_Get_SHARE_TLV_MAX_RECORDSBase()), 4);
	return;
}
#endif

//void AppAPI_DealBeforThrowExce(PX16 errorcode)
//{
//	natAppComEntry.NaAppAPI_DealBeforThrowExce(errorcode);
//	return;
//}

void PbocAPI_Init(void)
{
	g_isPBOCselected	= 0;
}

void PbocAPI_DealBeforThrowExce(PX16 errorcode)
{
	PATCH_PBOC_BACKUP_006();
	
	if (PBOC_TRUE == g_isPBOCselected)
	{
#if VM_WITH_EDEP		
		// 复位交易状态机到select状态
		if(ECP_TYPE_EDEP == g_PBOC_instType)
		{
			resetEDEPVar_select();
		}
#endif
#if VM_WITH_PBOC_AND_EDEP		
		else 
#endif
#if VM_WITH_PBOC		
		if(ECP_TYPE_PBOC == g_PBOC_instType)
		{
			resetVar_select();
			g_flagUpdate = 0xFF/*QPBOC_CAPP_ABORT_FLAG*/;
			abortTransactionByJCRE();
			errorcode[0] = SYSTEMEXCEPT_NO_RESOURCE;
		}
#endif		
	}
	
	return;
}

u16 PbocAPI_GetConfig(u8 configType, PX8 pConf)    SMALL_MODE
{
	u16     D_A    configValue = 0;
//	u8 defaultConfigureArea[15] = {0x80,0x13,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	PATCH_PBOC_API_001();
	switch (configType)
	{
#if VM_WITH_PBOC
		case ECP_TYPE_PBOC_INSTANCE_NVM_SIZE:
			configValue		= JcvmConfig_GetPbocEepSize();
			break;
		case ECP_TYPE_PSE_INSTANCE_NVM_SIZE:
		case ECP_TYPE_PPSE_INSTANCE_NVM_SIZE:
			configValue		= JcvmConfig_GetPpeEepSize();
			break;
		case ECP_TYPE_PBOC_INSTANCE_RAM_SIZE:
			configValue		= JcvmConfig_GetPbocRamSize();
			break;
		case ECP_TYPE_PBOC_DEFAULT_CONFIGE_AREA:
			ecpMem_Copy(pConf, MEMtPM8(JcvmConfig_GetPbocDefaultBase()), 0x0F);
			break;
		case ECP_TYPE_PPSE_PSE_DIR_NUM:
			configValue = JcvmConfig_GetPseInstanceDirNum();  //TODO:
			break;
		case ECP_TYPE_PBOC_DIR_NUM:
			configValue = JcvmConfig_GetPbocInstanceDirNum();  //TODO:
			break;
#endif
#if VM_WITH_EDEP
		case ECP_TYPE_CONSTRUCT_AUTH_CODE:
			AppAPI_ConstructAuthCode(pConf);
			break;
		case ECP_TYPE_EDEP_INSTANCE_NVM_SIZE:
			configValue = JcvmConfig_GetEdepEepSize();
			break;
		case ECP_TYPE_EDEP_DIR_NUM:
			configValue = JcvmConfig_GetEdepInstanceDirNum();
			break;
		case ECP_TYPE_EDEP_PRODUCT_TYPE:
			configValue = JcvmConfig_GetEdepInstanceProductType();
			break;
		case ECP_TYPE_EDEP_PRODUCT_TYPE_EX:
			configValue = JcvmConfig_GetEdepInstanceProductTypeEx();
			break;
		case ECP_TYPE_EDEP_XA_WALLET_ADVANCE:
			configValue = JcvmConfig_GetEdepInstanceXaWalletAdvance();
			break;
		case ECP_TYPE_EDEP_TFT_VERSION:
			ecpMem_Copy(pConf, MEMtPM8(JcvmConfig_GetEdepInstanceTftVersionBase()), ECP_EDEP_USER_CFG_YCT_VERSION_BYTENUM);
			break;
		case ECP_TYPE_EDEP_RFU:
			ecpMem_Copy(pConf, MEMtPM8(JcvmConfig_GetEdepRfuBase()), ECP_EDEP_USER_CFG_RFU_BYTENUM);
			break;
		case ECP_TYPE_EDEP_PRODUCT_S:
			ecpMem_Copy(pConf, MEMtPM8(JcvmConfig_GetEdepRfuBase()), ECP_EDEP_USER_CFG_PRODUCT_TYPE_BYTENUM);
			break;
#endif

#if (VM_WITH_PBOC && VM_WITH_EDEP && VM_WITH_JT_FUNC)
		case ECP_TYPE_JT_ECEP_LOCK_INFO:
			configValue = JcvmConfig_GetJTECEPLockInfo();
			break;
		case ECP_TYPE_JT_FILE_REF_EDEP:
			configValue = JcvmConfig_GetJTFileRefEDEP();
			break;
		case ECP_TYPE_JT_FILE_ADDR_EDEP:
			configValue = JcvmConfig_GetJTFileAddrEDEP();
			break;
		case ECP_TYPE_JT_SHARE_TLV_MAX_RECORDS:
			AppAPI_GET_SHARE_TLV_MAX_RECORDS(pConf);
			break;
#if 0
		case ECP_TYPE_JT_FILE_ADDR_1E:
			configValue = JcvmConfig_GetJTFileAddr1E();
			break;
		case ECP_TYPE_JT_AMOUNT_ADDR:
			configValue = JcvmConfig_GetJTAmountAddr();
			break;
		case ECP_TYPE_JT_RFU:
			ecpMem_Copy(pConf, MEMtPM8(JcvmConfig_GetJTRFUBase()), ECP_JT_RFU_BYTENUM);
			break;

#endif
#endif

#if VM_WITH_EID
		case ECP_TYPE_EID_INSTANCE_NVM_SIZE:
			configValue = JcvmConfig_GetEIDEepSize();
			break;
		case ECP_TYPE_EID_INSTANCE_RAM_SIZE:
			configValue = JcvmConfig_GetEIDRamSize();
			break;
		case ECP_TYPE_EID_DEFAULT_CONFIGE_AREA:
			ecpMem_Copy(pConf, MEMtPM8(JcvmConfig_GetEIDDefaultBase()), 0x0C);
			break;
#endif
		default:
			break;
	}

	return configValue;
}

u8 PbocAPI_CallPboc(PX8 apduBuf)
{
	
	PATCH_PBOC_API_003();
	if (PBOC_TRUE == g_isPBOCselected)
		{
			if ( (APDUBuffer[OFFSET_INS] != INS_OP_SELECT)
	#if (VM_WITH_EDEP || VM_WITH_EID)
					|| ( (INS_OP_SELECT == APDUBuffer[OFFSET_INS]) &&  ((ECP_TYPE_EDEP == g_PBOC_instType) || (ECP_TYPE_EID == g_PBOC_instType)) )
					)
			{
				return ecpPBOCMain(PBOC_FALSE);
			}
	#else
				)	
			{
				ecpPBOCMain();
			}
	#endif
		}
	return false;
}

u8 PbocAPI_CallPbocNative(u16 fun_index)    SMALL_MODE
{
	u8		D_A    isCallPbocNative;
#if	(_PBOC_WRITE_PAGE_ALINE_ && ECP_PBOC_SECURITY)
	PBOC_U8    PBOC_XDATA_AREA   NVMNeedOffset;
#endif
	
	PATCH_PBOC_API_002();

	isCallPbocNative	= true;

	switch(fun_index)
	{
		case ecpPBOCinstallParaProcess_NM:
			pushInt(ecpPBOCinstallParaProcess());
			break;
		case ecpInformSelect_NM:
			pushShort(ecpPBOCInformSelect());
			break;
		case ecpPBOCMain_NM:
#if (VM_WITH_EDEP || VM_WITH_EID)
			ecpPBOCMain(PBOC_TRUE);
#else
			ecpPBOCMain();
#endif
			break;
		case ecpInformDeselect_NM:
			ecpPBOCInformDeselect(true);
			break;
#if ECP_PBOC_SECURITY
		case ecpPBOCInformKill_NM:
			g_PBOC_instType = (PBOC_U8)popShort();
			#if ARM_CHIP
			//	g_PBOC_instRAMBuff = (PBOC_U8_XD_PTR)get_array_data(MEMtPF8(locateObjectInMemory(popReference())));
			//#else
				popReference();
			#endif	
			g_instNvmBuffRef = popReference();
			g_PBOC_instNVMBuff = (PBOC_U8_NV_PTR)get_array_data(MEMtPF8(locateObjectInMemory(g_instNvmBuffRef)));
			
			ReadEE_InBootArea(ECP_PBOC_PAGE_ALINE_INFO_OFFSET, 1, &NVMNeedOffset);//读取页对齐偏移
			g_PBOC_instNVMBuff = g_PBOC_instNVMBuff + NVMNeedOffset;
		
			g_app_eeprom_End = ecpGetPBOCInstUserSpaceEND();
		  g_readBackup = QEX_READ_DEST_EEPROM_DATA;
			KillApplication();
			break;
#endif
#if VM_WITH_EDEP	
		case readSecurityCodeDataNative_NM:
			pushShort(nativeMethods_readSecurityCodeDataNative());
		break;	
#endif	
		default:
			isCallPbocNative	= false;
			break;
	}

	return isCallPbocNative;
}


#if VM_WITH_EDEP || VM_WITH_PBOC
u8 PbocAPI_IsNativeAppSelected(void)
{
	if(g_isPBOCselected == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}
#endif

#if  VM_WITH_PBOC ||VM_WITH_EDEP ||VM_WITH_EID
void PbocAPI_DeselectNativeApp(void)
{
	g_PBOC_instType = 0;
}
#endif

#define  Owner_PIN  0
#define  pin_data  1
#define dataToProtectChks 1
#define  shared_Pin  0x11


memref GetObjAddrFromField(memref objectAddr, u8 oFieldId)
{
	jref objRef;
	
	PATCH_PBOC_API_004();
	if(objectAddr ==  REF_NULL)
	{
		return REF_NULL;
	}
	
	objRef = memReadU16((PM8)get_object_field_ref(objectAddr, oFieldId));

	if(objRef ==  REF_NULL)
	{
		return REF_NULL;
	}

	return locateObjectInMemory(objRef);
}


memref getpinvalueAddr()
{
	memref sharedPinObject,OwnerPINAddr;

	PATCH_PBOC_API_005();
	sharedPinObject =  GetObjAddrFromField(JcvmConfig_GetAdvanceAddr(),shared_Pin);

	if(sharedPinObject != REF_NULL)
		{
			OwnerPINAddr = GetObjAddrFromField(sharedPinObject, Owner_PIN);
			if(OwnerPINAddr != REF_NULL)
			{
				return get_array_data((PF8)GetObjAddrFromField(OwnerPINAddr, pinValue_INDEX));
			}
		}

	return REF_NULL;
}

memref getPTLPTCAddr()
{
	memref sharedPinObject,PAPTCPTLAddr;
	
	PATCH_PBOC_API_006();
	sharedPinObject =  GetObjAddrFromField(JcvmConfig_GetAdvanceAddr(),shared_Pin);

	if(sharedPinObject != REF_NULL)
	{
		PAPTCPTLAddr = GetObjAddrFromField(sharedPinObject, pin_data);
		if(PAPTCPTLAddr != REF_NULL)
		{
			return get_array_data((PF8)GetObjAddrFromField(PAPTCPTLAddr, dataToProtectChks)) + 4;
		}

	}

	return REF_NULL;
}

#define  ATCOFF            0x01
memref getATCAddr(void)
{
	PATCH_PBOC_API_007();
	//return GetObjAddrFromField(JcvmConfig_GetAdvanceAddr(),ATCOFF);
	return get_object_field_ref(JcvmConfig_GetAdvanceAddr(),ATCOFF);
}

#endif //VM_WITH_PBOC_OR_EDEP

