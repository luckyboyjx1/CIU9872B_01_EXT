#include "profile.h"
#include "JcvmConfig.h"
#include "APDU.h"
#include "SysConfig.h"
#include "Boot.h"
#include "BootLifeCycle.h"
#include "memory.h"
#include "global_variable.h"
#include "JcvmRam.h"
#include "JcvmDataConfig.h"
#include "MemOperation.h"
#include "reset.h"
#include "cardManager.h"
#include "ob_manager.h"
#include "PbocAPI.h"
#include "AppAPI.h"
#include "keySet.h"
#include "jcvmConfigMethod.h"
#include "SEEUICCInterface.h"
#include "CLProtocolMngt.h"
#include "object.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 


#if HAL__MEMORY__RAM_MAP_IN_C51
#pragma JCVM_CONFIG_RAM_START
#endif

u16 initCardData(PX8  apduBuffer)    SMALL_MODE
{
	PATCH_JCVM_INIT_JCVMCONFIG_001();
    if(INS_INIT == apduBuffer[APDU_INS_OFFSET])
	{
		if (SECURITY_STATE_ATTACK == boot_GetSecurityState()) 
		{
		    return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	    }
		if(LIFE_CYCLE_READY != BootLifeCycle_GetLifeCycle())
		{
			return BOOT_SECURITY_STATUS_NOT_SATISFIED;
		}
	}
#if (EUICC_ONLY||SE_AND_EUICC)
	G_CurExecutingProfileIndex = ILLEGAL_PROFILEINDEX;
	ExecuEnvironmentInit(G_CurExecutingProfileIndex);
#endif
	InitJCRE1();
	InitJCRE2();

	loadSysPkg();
	BootLifeCycle_SetLifeCycle(LIFE_CYCLE_INIT);
	JcvmConfig_SetCMappletLocketCount(0x0);
	#if JAVA_CARD_UICC
	InitCardConfigArea();
	#endif
	return BOOT_NO_ERROR;
}

u16 getConfig(u8 configType) {
	u16 configValue;

	configValue = 0;
	switch (configType)
	{
		case GET_LAST_BASIC_PACKAGE:
			configValue = U8t16(JcvmConfig_GetCmLastBasicPkg());
			break;
		case GET_INIT_MASK_PACKAGE_NUMBER:
			configValue = U8t16(NPACKAGES);
			break;
		case GET_LOCKED_APPLET_NUMBER:
			configValue = JcvmConfig_GetCMappletLocketCount();
			break;
		case CHECK_CARD_CREATED:
			if (BootLifeCycle_GetLifeCycle() < LIFE_CYCLE_CREATE)
			{
				configValue = 0;
			}
			else
			{
				configValue = 1;
			}
			break;
#if !GP__VERSION__FOR_MINI_MASK
		case SD_RSA_TOKEN_BYTE_LEN:
			configValue = JcvmConfig_GetCmSdRsaTokenByteLen();
			break;
		case INSTALL_FOR_LOAD_NO_HASH://测试用例：gpBFCCompliance208:   ccm_APDU_GP211_load_SEC_12_dapMissing_1mandatedDapVerif
			if (GP_CONF0_INSTALL_FOR_LOAD_NO_HASH != GP_INSTALL_FOR_LOAD_NO_HASH)
			{
				configValue = 0;
			}
			else
			{
				configValue = 1;
			}
			break;
		case INSTALL_FOR_OLD_ICC:
				if (GP_CONF1_INSTALL_FOR_OLD_ICC != GP_INSTALL_FOR_OLD_ICC)
				{
					configValue = 0;
				} else
				{
					configValue = 1;
				}
			break;
		case SUPPORT_9F66_TAG:
				if(GP_CONF2_ISD_SUPPORT_9F66_TAG != GP_ISD_SUPPORT_9F66_TAG)
				{
					configValue = 0;
				}
				else
				{
					configValue = 1;
				}
				break;

		case INSTALL_FOR_EID:
			if (GP_CONF4_INSTALL_FOR_EID != GP_FOR_EID)
			{
				configValue = 0;
			}
			else
			{
				configValue = 1;
			}
			break;

		case PRO_SEC_NOT_RECEIVE:
			if(GP_CONF3_PRO_SEC_RECEIVE != GP_PROCESS_SECURITY_RECEIVE_JINDIAN)
			{
				configValue = 0;
			}
			else
			{
				configValue = 1;
		}
			break;

		case GP_REGISTER_ZHEJIANG_NONGXIN:
			if(GP_CONF5_REGISTER != GP_REGISTER_FOR_ZHEJIANG_NONGXIN)
			{
				configValue = 0;
			}
			else
			{
				configValue = 1;
			}
			break;
		case GET_STATUS_SUPPORT_SSD:
			if (U8_AND_U8(JcvmConfig_GetStausSupportSSD(), 0x01) == 0x01)
			{
				configValue = 1;
			}
			else
			{
				configValue = 0;
			}
			break;
#endif
		case IS_CRYPTO_ALL_USE_RTR:
			if(CONFIG_CRYPTO_ALL_USE_RTR != 0x04)
			{
				configValue = 0;
			}
			else
			{
				configValue = 1;
			}
			break;
		case SE_BE_0R_NOT:
			if(CARD_SE_BE_OR_NOT)//modify by liguiyu@20220111 if (U8_AND_U8(JcvmConfig_GetSeBeOrNot(), 0x01) == 0x01)
			{
				configValue = 1;
			}
			else
			{
				configValue = 0;
			}
			break;
		case UICCCONFIG_TriggerToolkitApi:
	#if JAVA_CARD_UICC
			configValue = ValueBit(JcvmConfig_GetConfigUicc(), BIT_UICCConfig_TriggerToolkitApi);
	#endif
			break;
		case GP_CONFIG_FOR_UICC_Configuration:
			if(CONFIG_GP_FOR_UICC_Configuration)
			{
				configValue = 1;
			}
			else
			{
				configValue = 0;
			}
			break;
		case GP_CONFIG_FOR_MAPPING_TEST:
			if(CONFIG_GP_FOR_MAPPING_TEST)
			{
				configValue = 1;
			}
			else
			{
				configValue = 0;
			}
			break;
		case GP_CONFIG_FOR_BFC:
			if(CONFIG_GP_FOR_BFC)
			{
				configValue = 1;
			}
			else
			{
				configValue = 0;
			}
			break;
		case GP_CONFIG_FOR_CI:
			if(CONFIG_GP_FOR_CI)
			{
				configValue = 1;
			}
			else
			{
				configValue = 0;
			}
			break;
		case M2M_LocalManageProfile: // added by chentao@20221201: 根据20221130 合陈涛代码时代码评审.xlsx 修改，避免JAVA层使用开关
#if M2M_ESx
			configValue = 1;
#else
			configValue = 0;
#endif
			break;
		default:
			break;
	}

	return configValue;
}

memref JcvmConfig_GetVmConfigAddr(u32 offset)    SMALL_MODE
{
	memref    D_A    addr;

	if (CARDINIT_CONFIG_BASE > offset)
	{
		addr	= PNtMEM(VM_CONFIG_START + offset);
		return addr;
	}

	return offset;
}
/*增加私有指令E081000000，用于在镜像做卡后将UID更新成芯片实际的UID,by wuronghua 20211230*/
u16 JcvmConfig_UpdateIsdUid(PX8  apduBuffer)    SMALL_MODE
{
#if HAL__IO__SWP
	u32 isdEntryAddr;
	u32 clEntryAddr;
	u32 typeAParaObjAddr;
	memref arrayAddr;
	u8  uidData[6];
	u8  writeLen;
	u16 ProtocolDataRef;
	
	if (SECURITY_STATE_ATTACK == boot_GetSecurityState()) 
	{
		return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	if(LIFE_CYCLE_INIT > BootLifeCycle_GetLifeCycle())
	{
		return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	
	//get isd entry
	isdEntryAddr = getSdEntryObj(0x00);

	clEntryAddr = locateObjectInMemory(PN_R_U16(get_object_field_ref(isdEntryAddr, clAppEntry_INDEX)));

	if((REF_NULL != clEntryAddr) && (MEM_NULL != clEntryAddr) )
	{
		typeAParaObjAddr = locateObjectInMemory(PN_R_U16(get_object_field_ref(MEMtPF8(clEntryAddr), typeAPara_INDEX)));
		
		//arrayAddr = GetDataFromProtoObj(typeAParaObjAddr);
		if (typeAParaObjAddr != MEM_NULL)
		{
			ProtocolDataRef = PX_R_U16((u8 *)get_object_field_ref(typeAParaObjAddr, ProtocolData_INDEX));
			arrayAddr = get_array_data(MEMtPF8(locateObjectInMemory(ProtocolDataRef)));
			
			if (arrayAddr != MEM_NULL)
			{
				//set chip uid
				uidData[1] = SwpHal_GetUid(&uidData[2]);
				
				if (memReadU8(MEMtPM8(arrayAddr + UID_TLV)) == 0x80)	//TAG
				{
					//if (src[SWP_TYPEA_UID_LEN] != 0x00)
					{	
						uidData[0] = uidData[1] + 1;
						memCopyNoAtomic(MEMtPM8(arrayAddr + UID_TLV + 1), uidData, uidData[0] + 1);
						
						//set config
						sysConfig_SetIoconfigSwpTypeAUidLen(uidData[1]);
						sysConfig_SetIoconfigSwpTypeAUidValue(&uidData[2], 4);
						
						//set current para
						#if GP__AMENDMENT__C__FOR_MASK
						UpdateCurrentCLParaUid(&uidData[2], 4);
						#endif
						
						memRefresh();
						
						#if HAL__IO__MIFARE
						//set mifare uid
						ecpMifareHal_UpdateBlock0();
						#endif
						
						return BOOT_NO_ERROR;
					}
				}
				
				
			}
		}
	}
#endif
	return BOOT_REFERENCED_DATA_NOT_FOUND;
}

