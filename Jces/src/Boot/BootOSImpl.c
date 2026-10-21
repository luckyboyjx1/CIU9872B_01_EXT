/*
 * BootOSImpl.c
 *
 *  Created on: 2014年11月18日
 *      Author: ZhangBo
 */


#include "BootOSImpl.h"
#include "BootOS.h"
#include "MemBackUp.h"
#include "Apdu.h"
#include "MemOperation.h"
#include "Crypto_API.h"
#include "SysConfigData.h"
#include "BootLifeCycle.h"
#include "MemGetReuseRam.h"
#include "BootAreaInitRam.h"
#include "SCPInterface.h"
#include "Io_Swp.h"
#include "Mifare.h"
#include "Boot.h"
#include "BootRegister.h"
//break the code architectural temporarily
#include "SCPDealData.h"
#include "bootPlaformConfigProfile.h"
#include "Crypto_Constant.h"

#if !BOOTLOADER_PRO
#include "euiccConfig.h"
#endif

#if BOOTLOADER
#include "FunImp.h"
#endif

#if GP__CGM__FOR_MASK
#include "CgmManager.h"
#endif


#if  GP__SCP__SCP02
static u8											g_bootSCPsessionKey[16];

void initBootSCPsessionKey(memref keyBuffer)
{
	memCopyNoAtomic(MEMtPM8(g_bootSCPsessionKey), MEMtPM8(keyBuffer), sizeof(g_bootSCPsessionKey));
}

void generateCardCryptogramSCP0102(PX8 firstChallenge, PX8 secondChallenge, PX8 derivationData)
{

	#define PADDING_REMAIN_LEN		0x07
	#define CHALLENGE_LEN			0x08
	#define PADDING_80				0x80
	#define icv derivationData

	PX8 tempPtr;

	tempPtr = MEMtPX8(MemGetReuseRam_GetTempArrBuffer(24));

	ecpMem_Copy(tempPtr, firstChallenge, CHALLENGE_LEN);
	tempPtr = PX8_A_CONST(tempPtr , CHALLENGE_LEN);

	ecpMem_Copy(tempPtr, secondChallenge, CHALLENGE_LEN);
	tempPtr = PX8_A_CONST(tempPtr , CHALLENGE_LEN);

	// do padding 0x80
	PX_W_U8(tempPtr, PADDING_80);
	tempPtr = PX8_A_CONST(tempPtr , 0x01);

	//do padding 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	ecpMem_Set(tempPtr, 00, PADDING_REMAIN_LEN);

	//clear ICV for DES CAL
	ecpMem_Set(icv, 00, DES_BLOCKSIZE);

	tempPtr = MEMtPX8(MemGetReuseRam_GetTempArrBuffer(24));
	initBootSCPsessionKey(PNtMEM(g_current_SCP_SES_KEY));
	Triple_DES(tempPtr, g_bootSCPsessionKey, tempPtr, 24, icv, CBC_Encrypt, SENSITIVE_DATA_MASKED);
}

#endif

#define	COLD_ATR_TAG		   0x0104
#define TO_MODIFY_COLDATR_FLAG 0

#define WARM_ATR_TGA		   0x0108
#define TO_MODIFY_WARMATR_FLAG 1

#define TO_MODIFY_HIS_ONLY	   0
#define TO_MODIFY_WHOLE_ATR    1

#if BOOT__CONFIGDATA__POWER_OFF_FOR_TEST
#define		HEAP_BOUND_TAG				0xFF21
#define		AUTH_TRY_CNTR_ADDRESS_TAG	0xFFC1
#define 	SET_CARDSTATE				0x5011

#if !NATIVE_CARD
void recordNewEEpromArea(memref arr)
{
	PX8 	p;

	p 							= MEMtPX8(arr);
	PR_W_U32(p, MapTable_GetMapTopAddress());

	p = PX8_A_U8(p, (u8)sizeof(memref));
	PR_W_U32(p, FreeTable_GetTopFreeNvm());
}
#endif

#if !BOOTLOADER_PRO
u16 getPrivateData(PX8 apduBuffer,u8 type)    SMALL_MODE
{
	u8 D_A version;
	u16 D_A valueTag;
//	u16 D_A sequenceCounter;
//	jref D_A tempKeyset;
	memref     counterAddr;

	valueTag = U16_OR_U16(U16_SHL_U8(apduBuffer[2], 8), apduBuffer[3]);
	switch(valueTag)
	{
#if !NATIVE_CARD
		case HEAP_BOUND_TAG:
			recordNewEEpromArea(PXtMEM(&apduBuffer[0]));
			PR_W_U32(&apduBuffer[8], FreeTable_GetTblAddr());
			apdu_ApduSetOutgoingAndSend(0,12);
			break;
		case AUTH_TRY_CNTR_ADDRESS_TAG:
			apdu_ApduSetIncomingAndReceive();
			version = apduBuffer[5];

			PXB_W_U16(&apduBuffer[4], getKeyCounter(&counterAddr, version));
			PXB_W_U32(&apduBuffer[0], PNtMEM(counterAddr));

			apdu_ApduSetOutgoingAndSend(0,6);

			break;
#if BOOTLOADER
		case  BL_LOAD_KEY_TAG:
			return LoaderBootloaderKey(apduBuffer);
			break;
#endif
#if GP__CGM__FOR_MASK
		case  CGM_INIT_ENABLE_TAG:
			CgmInit_EnableOrNot(apduBuffer, type);
			break;
#endif
#endif
		case  SET_CARDSTATE:
			JcvmConfig_SetSEState(apduBuffer);
			break;
		default:
			return BOOT_REFERENCED_DATA_NOT_FOUND;
	}
	return 0x9000;
}
#endif
#endif
#if BOOT__CONFIGDATA__SENCOND_ATR
u8 secondAtrOperate(PX8 commandBuffer, u8 operateType)
{
	if(STOREDATA == operateType)
	{
		if((commandBuffer[APDU_DATA_OFFSET + 1] > 0x1E) || (commandBuffer[APDU_DATA_OFFSET + 2] != 0x3B))
		{
			return FALSE;
		}
		//nvmCopyAtomic(OS_SECOND_ATR, &commandBuffer[APDU_DATA_OFFSET], commandBuffer[APDU_P3_OFFSET]);
		memCopyAtomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(OS_SECOND_ATR)), &commandBuffer[APDU_DATA_OFFSET], commandBuffer[APDU_P3_OFFSET]);
	}
	else
	{
		//ecpMem_Copy(&commandBuffer[APDU_DATA_OFFSET], OS_SECOND_ATR, commandBuffer[APDU_P3_OFFSET]);
		memCopyToRam(&commandBuffer[APDU_DATA_OFFSET], MEMtPN8(JcvmConfig_GetVmConfigAddr(OS_SECOND_ATR)), commandBuffer[APDU_P3_OFFSET]);
		apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, commandBuffer[APDU_P3_OFFSET]);
	}
	return TRUE;
}
#endif
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 CheckAtrInterfaceBytes

  Input parameter:   atr

  Output parameter:  b8: atr中有无tck字节;
                     b7-b1: InterfaceBytes末字节的偏移

  Description:  检查atr的接口字节

 -------------------------------------------------------------------------------------------------------------------*/
u8 CheckAtrInterfaceBytes(PR8 atr)
{
    u8		offset;
    u8		i;
    u8		tckTag;

    tckTag 	= 0;
	offset 	= 1;	//point to T0 byte

	while(1)
	{
		if(offset > 33 )
		{
			return false;	//ISO7816-3
		}

		if( U8_AND_U8(atr[offset], 0xf0) == 0)
		{
			break;
		}
		/*@ -type @*/
		i	=  (U8_AND_U8(atr[offset], 0x10) == 0x10) ? 1:0;
		i 	+= (U8_AND_U8(atr[offset], 0x20) == 0x20) ? 1:0;
		i 	+= (U8_AND_U8(atr[offset], 0x40) == 0x40) ? 1:0;
		/*@ +type @*/
		if(U8_AND_U8(atr[offset], 0x80) == 0x80)
		{
			U8_INC(i);

			offset	= U16t8(U8_A_U8(offset, i)); //point to TDi byte

			if(U8_AND_U8(atr[offset], 0x0f) != 0)
			{
				tckTag = 1;	//tck byte
			}
		}
		else
		{
			offset	= U16t8(U8_A_U8(offset, i));
			break;
		}
	} //offset now point to the end of interface bytes

	return tckTag ? U16t8(U8_A_U8(0x80, offset)) : offset;
}


/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 modifyAtrValue

  Input parameter:   coldOrWarm，要修改冷复位的ATR，还是热复位的ATR
					 hitoryOrWhole，修改对应ATR的历史字节，还是接口字节

  Description:       将目标位置的数据更改为：长度为length的buf数据

 -------------------------------------------------------------------------------------------------------------------*/
#if !BOOTLOADER_PRO && !NATIVE_CARD
u16 modifyAtrValue(u8 coldOrWarm, u8 hitoryOrWhole, PR8 buf, u8 length)
{
	PN8		ATR[4];							//point to "len, if, len, his"
	u8 		tckTag;
	u8 		offset;
	u8 		value;

	tckTag = 0;

	//point the cold reset atr
	if(coldOrWarm == TO_MODIFY_COLDATR_FLAG)
	{
		ATR[0]	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_COLD_IFLEN));
		ATR[1] 	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_COLD_IFBYTES));
		ATR[2]	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_COLD_HISTLEN));
		ATR[3] 	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_COLD_HISTBYTES));
	}
	//point to the warm reset atr
	else if(coldOrWarm == TO_MODIFY_WARMATR_FLAG)
	{
		ATR[0]	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_WARM_IFLEN));
		ATR[1] 	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_WARM_IFBYTES));
		ATR[2]	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_WARM_HISTLEN));
		ATR[3] 	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_WARM_HISTBYTES));
	}
	else
	{
		return false;
	}

	//modify the history bytes and length
	if(hitoryOrWhole == TO_MODIFY_HIS_ONLY)
	{
		if(length > 0x0f)
		{
			return false;
		}
		nvmWriteU8Atomic(ATR[2],length);
		nvmCopyAtomic(ATR[3], buf, length);
	}
	//modify the whole atr
	else if(hitoryOrWhole == TO_MODIFY_WHOLE_ATR)
	{
		if(length > 30)
		{
			return false;
		}

		tckTag = CheckAtrInterfaceBytes(buf);
		if(!tckTag)
		{
			return false;
		}

		offset = U8_AND_U8(tckTag,0x7F);
		nvmWriteU8Atomic(ATR[0],offset);
		nvmCopyAtomic(ATR[1], MEMtPN8(PX8_A_U8(buf,1)), offset);
		nvmWriteU8Atomic(ATR[2], U8_AND_U8(buf[1], 0x0f));
		nvmCopyAtomic(ATR[3], PX8_A_U8(buf,offset+1), U8_AND_U8(buf[1], 0x0f));
	}
	else
	{
		return false;
	}

	//tck present or not; 7816-3
	tckTag = U8_AND_U8(tckTag, 0x80);
	if(hitoryOrWhole == TO_MODIFY_WHOLE_ATR)
	{
		if(coldOrWarm == TO_MODIFY_COLDATR_FLAG)
		{//enable cold tck
			tckTag = U8_SHR_U8(tckTag,3);
			value = PN_R_U8(MEMtPN8(sysConfig_GetUartConf()));
			value = U8_AND_U8(value ,0xef);
			value = U8_OR_U8(value, tckTag);
			nvmWriteU8Atomic(MEMtPN8(sysConfig_GetUartConf()), value);
		}
		else
		{//enable warm tck
			tckTag = U8_SHR_U8(tckTag, 2);
			value = PN_R_U8(MEMtPN8(sysConfig_GetUartConf()));
			value = U8_AND_U8(value, 0xdf);
			value = U8_OR_U8(value, tckTag);
			nvmWriteU8Atomic(MEMtPN8(sysConfig_GetUartConf()), value);
		}
	}
	return true;
}

u8 modifyAtr(PX8 commandBuffer,u8 operateType)
{
	u16 configName;
	u8 	coldOrWarm;

	configName = PXB_R_U16(&commandBuffer[APDU_P1_OFFSET]);

	if(commandBuffer[APDU_DATA_OFFSET] > 1)
	{
		return false;
	}
	/*@ -type @*/
	coldOrWarm = ((configName == COLD_ATR_TAG) ? TO_MODIFY_COLDATR_FLAG : TO_MODIFY_WARMATR_FLAG);
	/*@ +type @*/
	return U16t8(modifyAtrValue(coldOrWarm, commandBuffer[APDU_DATA_OFFSET], MEMtPX8(&commandBuffer[APDU_DATA_OFFSET + 1]), U8_S_U8(commandBuffer[APDU_P3_OFFSET], 1)));
}
#endif

#if	BOOT__CONFIGDATA__WRITE_FILE_ID
u8 writeDataOfSelectAppByFileId(PX8 commandBuffer, u8 operateType)
{
	PN8 			dFileIdAddr;
	u8 				dataLength;

	dFileIdAddr	= MEMtPN8(PN_R_U24(MEMtPN8(JcvmConfig_GetVmConfigAddr(CM_FileIdAddr))));
	dataLength	= commandBuffer[APDU_P3_OFFSET];

	if(dFileIdAddr >= MEMtPN8(COS_CONFIG_START) && dFileIdAddr < MEMtPN8(ROM_BASE))
	{
		if(operateType == STOREDATA)
		{
			nvmCopyAtomic(dFileIdAddr, &commandBuffer[APDU_DATA_OFFSET], dataLength);
		}
		else
		{
			ecpMem_Copy(&commandBuffer[APDU_DATA_OFFSET], dFileIdAddr, dataLength);
			apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, dataLength);
		}
		return true;
	}
	else
	{
		return false;
	}
}
#endif
#if	BOOT__CONFIGDATA__CARD_VERSION
u8 getCardPlatformVersion(PX8 commandBuffer, u8 operateType)
{
	u8 				dataLength;
	u8        cardVersionArray[8];
	if(operateType == GETDATA)
	{
		dataLength	= commandBuffer[APDU_P3_OFFSET];
		getCardVersion(MEMtPX8(&cardVersionArray[0]));
		ecpMem_Copy(&commandBuffer[APDU_DATA_OFFSET],MEMtPX8(&cardVersionArray[0]) , dataLength);
		apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, dataLength);
		return true;
	}
	return false;
}
#endif


#if HAL__CONFIG_AREA__NEW

#define	COLD_ATR_TAG		   0x0104
#define TO_MODIFY_COLDATR_FLAG 0

#define WARM_ATR_TGA		   0x0108
#define TO_MODIFY_WARMATR_FLAG 1

#define TO_MODIFY_HIS_ONLY	   0
#define TO_MODIFY_WHOLE_ATR    1
/***************************************************used for BOOT CONFIG***********************************************************/
static u8 											g_bootSCPcardChallenge[8];
static u8											g_bootSCPhostChallenge[8];
//static u8											g_bootSCPsessionKey[16];
static u8											g_bootSCPmacKey[16];
static u8											g_bootSCPdekKey[16];
static u8											g_bootSCPlastMAC[8];
static PN8											SetBank0Base;
u8 											        Boot_CurPrivilege;
u8 											        Boot_internal_verify_flag;
/***********************************************************************************/
//void generateCardCryptogramSCP0102(PX8 firstChallenge, PX8 secondChallenge, PX8 derivationData)
//{
//
//	#define PADDING_REMAIN_LEN		0x07
//	#define CHALLENGE_LEN			0x08
//	#define PADDING_80				0x80
//	#define icv derivationData
//
//	PX8 tempPtr;
//
//	tempPtr = MEMtPX8(MemGetReuseRam_GetTempArrBuffer(24));
//
//	ecpMem_Copy(tempPtr, firstChallenge, CHALLENGE_LEN);
//	tempPtr = PX8_A_CONST(tempPtr , CHALLENGE_LEN);
//
//	ecpMem_Copy(tempPtr, secondChallenge, CHALLENGE_LEN);
//	tempPtr = PX8_A_CONST(tempPtr , CHALLENGE_LEN);
//
//	// do padding 0x80
//	PX_W_U8(tempPtr, PADDING_80);
//	tempPtr = PX8_A_CONST(tempPtr , 0x01);
//
//	//do padding 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	ecpMem_Set(tempPtr, 00, PADDING_REMAIN_LEN);
//
//	//clear ICV for DES CAL
//	ecpMem_Set(icv, 00, DES_BLOCKSIZE);
//
//	tempPtr = MEMtPX8(MemGetReuseRam_GetTempArrBuffer(24));
//	initBootSCPsessionKey(PNtMEM(g_current_SCP_SES_KEY));
//	Triple_DES(tempPtr, g_bootSCPsessionKey, tempPtr, 24, icv, CBC_Encrypt, SENSITIVE_DATA_MASKED, SENSITIVE_DATA_MASKED);
//}
//
//void initBootSCPsessionKey(memref keyBuffer)
//{
//	memCopyNoAtomic(MEMtPM8(g_bootSCPsessionKey), MEMtPM8(keyBuffer), sizeof(g_bootSCPsessionKey));
//}

void initBootSecureGlobalVariable(void)
{
	Boot_CurPrivilege 			= 0;
	Boot_internal_verify_flag 	= 0;
	SetBank0Base 				= ILLEGL_BANK;
	
	//必须进行优化的代码，完全不符合层次要求。
	selected_channel			= 0;
	g_current_SCP_I_Option		= 0;
}

u16 boot_bootOSstart(void)
{
	u8 serialNumber[14] = {0x65,0x63,0x70,0x42,0x6F,0x6F,0x74,0x4F,0x53,0x73,0x74,0x61,0x72,0x74};
	
	if ((0xFA != APDUBuffer[APDU_P1_OFFSET]) || (0xDC != APDUBuffer[APDU_P2_OFFSET]) || (0x0E != APDUBuffer[APDU_P3_OFFSET]))
	{
		return BOOT_INS_NOT_SUPPORTED;
	}
	
	apdu_ApduSetIncomingAndReceive();
	
	if (ecpMem_Compare(MEMtPX8(&APDUBuffer[APDU_DATA_OFFSET]), serialNumber, 14) != 0)
	{
		return BOOT_INS_NOT_SUPPORTED;
	}
	
	Boot_internal_verify_flag = U8_OR_U8(Boot_internal_verify_flag, BOOT_BOOT_OS_START_MASK);

	return BOOT_NO_ERROR;
}

u16 boot_unwrap(u8 targetPrivilege)
{
	if(BOOT_IS_HAD_CURRENT_PRIV(targetPrivilege) || (BOOT_READY_STATE == boot_GetLifeCycle()) )
	{
		return boot_CheckIsAuthenticated();
	}
	return BOOT_INS_NOT_SUPPORTED;
}

u16 boot_SetKeyTryCounter()
{
	u16 sw;
	
	apdu_ApduSetIncomingAndReceive();

	sw = boot_unwrap(BOOT_PRIVILEGE_NM);

	if (BOOT_NO_ERROR != sw)
	{
		return sw;
	}

	nvmWriteU16Atomic(MEMtPN8(BOOT_KEY_TRY_COUNTER_BASE), APDUBuffer[APDU_P2_OFFSET]);

	return sw;
}


u16 boot_ExternalAuthenticatePreOperator(void)
{
	u16 		D_A		sequenceCounter;
	u8			D_A		p1;
	u8			D_A		p2;
	u8			D_A		auth_Trycntr;
	u8			D_A		lifeCycle;
	u8			D_A		tempKeybuff[BOOT_KEY_STRUCT_LEN];
	PM8 		D_A		bootConfigKey;

	p1			= APDUBuffer[APDU_P1_OFFSET];
	p2			= APDUBuffer[APDU_P2_OFFSET];
	lifeCycle = boot_GetLifeCycle();
	
	if (BOOT_LOCKED_STATE == lifeCycle)
	{
		if (!U8_AND_U8(Boot_internal_verify_flag, BOOT_BOOT_OS_START_MASK))
		{
			return BOOT_INS_NOT_SUPPORTED;
		}
	}	
	
	apdu_ApduSetIncomingAndReceive();
	Boot_internal_verify_flag = BOOT_BOOT_OS_START_MASK;
	
	//锁卡状态下，解锁权限的获取必须同时先获取其他三个权限。
	if ((BOOT_LOCKED_STATE == lifeCycle) && (BOOT_PRIVILEGE_UL == p1))
	{
		if (U8_AND_U8(Boot_CurPrivilege, BOOT_PRIVILEGE_UL_MASK) != BOOT_PRIVILEGE_UL_MASK)
		{
			return BOOT_INS_NOT_SUPPORTED;
		}
	}
	 //CMRE_READY 状态如果密钥不初始化更新，就会在boot_VerifyCheckValue中检查密钥的合法性。
	if ((BOOT_INITIALIZED_STATE == lifeCycle) || (BOOT_TERMINATED_STATE == lifeCycle))
	{
		return BOOT_INS_NOT_SUPPORTED;
	}

	if (p2 != 0)
	{
		return BOOT_INCORRECT_P1P2;
	}
	if (APDUBuffer[APDU_P3_OFFSET] != 0x08)
	{
		return BOOT_WRONG_LENGTH;
	}

	bootConfigKey = MEMtPM8(boot_GetKeyAddress(p1));
	if(0 == bootConfigKey)
	{
		return BOOT_INS_NOT_SUPPORTED;
	}

	//check checkvalue
	if (BOOT_KEY_VALUE_OK != boot_VerifyCheckValue(bootConfigKey, BOOT_VERIFY_KEY_CHECK_VALUE))
	{
		return BOOT_INS_NOT_SUPPORTED;
	}

	//decrypto the bootConfigKey and store the result into tempKeybuff;
	boot_DecrytoKey(bootConfigKey,MEMtPX8(tempKeybuff));
	auth_Trycntr = boot_GetTryCTR(MEMtPM8(tempKeybuff));
	if ((nvmReadU8(MEMtPN8(BOOT_KEY_TRY_COUNTER_BASE))) < auth_Trycntr)
	{
		return BOOT_INS_NOT_SUPPORTED;
	}

	// cardChallenge
	Random_number_generator(MEMtPX8(g_bootSCPcardChallenge), 8, 1);

	// hostChallenge
	ecpMem_Copy(g_bootSCPhostChallenge, &APDUBuffer[5], 8);

	sequenceCounter = boot_GetKeyCTR(MEMtPM8(tempKeybuff));
	g_bootSCPcardChallenge[0]= U16t8(U16_SHR_U8(sequenceCounter,8));
	g_bootSCPcardChallenge[1]= U8_AND_U8(U16t8(sequenceCounter),0xFF);

	// generate session key (KDCenc, KDCmac)
	boot_GenerateSessionKey(MEMtPN8(tempKeybuff), sequenceCounter);

	//option = 0 : copy CardCryptogram to apdubuffer, option = 2 : used for boot
	//generateCardCryptogram(hostChallenge, cardChallenge, 2);
	generateCardCryptogramSCP0102(MEMtPX8(g_bootSCPhostChallenge), MEMtPX8(g_bootSCPcardChallenge), PX8_A_CONST(APDUBuffer, 13));

	ecpMem_Copy(PX8_A_CONST(APDUBuffer, 5), g_bootSCPcardChallenge, 0x08);

	boot_SetTryCTR(bootConfigKey);
	Boot_internal_verify_flag = U8_OR_U8(BOOT_INTERNAL_VERIFY_OK, p1) ;

	apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, 16);
	return BOOT_NO_ERROR;
}

u16 boot_VerifyExternalAuthenticate(void)
{
	u8			D_A		p1;

	p1		= APDUBuffer[APDU_P1_OFFSET]; // Retrieve the encryption type

	if (U8_AND_U8(Boot_internal_verify_flag, BOOT_INTERNAL_VERIFY_OK) != BOOT_INTERNAL_VERIFY_OK)
	{
		return BOOT_INS_NOT_SUPPORTED;
	}

	if ((p1 != U8_AND_U8(Boot_internal_verify_flag, BOOT_PRIVILEGE_MASK)))
	{
		return BOOT_INCORRECT_P1P2;
	}
	Boot_internal_verify_flag = BOOT_BOOT_OS_START_MASK;

	apdu_ApduSetIncomingAndReceive();

	if (APDUBuffer[APDU_P3_OFFSET] == 0x10)
	{
		g_current_SCP_I_Option = U8_OR_U8(g_current_SCP_I_Option, SC0102_ICV_FOR_CMAC);
		
		if (boot_verifyMAC() == 0)
		{
			//g_current_SCP_I_Option = scpOptionBak;
			return BOOT_CONDITIONS_NOT_SATISFIED;
		}
		//g_current_SCP_I_Option = scpOptionBak;
		//Compare HostCryptogram
		//option = 0 : copy CardCryptogram to apdubuffer, option = 1 : compare HostCryptogram
		generateCardCryptogramSCP0102(MEMtPX8(g_bootSCPcardChallenge), MEMtPX8(g_bootSCPhostChallenge), PX8_A_CONST(APDUBuffer, 50));
		
		if (ecpMem_Compare(PX8_A_CONST(APDUBuffer, 50), PX8_A_CONST(APDUBuffer, 5), 8) != 0)
		{
			return BOOT_AUTHENTICATION_FAILED;
		}
		boot_SetSecuritychannel(p1);
	}
	else
	{
		return BOOT_WRONG_LENGTH;
	}
	return BOOT_NO_ERROR;
}

void boot_CalMACSCP02(PX8 srcAddress, u8 len, PX8 keyBuff, PM8 ICVBuff)
{
	//u8 blockNum,len,length;
#define ONE_BLOCK 			0x01

	u8 		X_A		blockNum;
	u16		X_A		length;
	u16		X_A		tempI;
	PN8 			tempPtr;
	u8		X_A		temArry[DES_BLOCKSIZE];

	ecpMem_Set(temArry, CLEARED_VALUE, DES_BLOCKSIZE);

	//todo:change,padding error
	blockNum = add80Padding(srcAddress, len, DES_BLOCKSIZE);
	length   = U8t16 (U8_SHL_U8(blockNum, 3));

	tempI   = U16_S_U8(length , DES_BLOCKSIZE);
	tempPtr = srcAddress;

	memCopyToRam(MEMtPX8(temArry), ICVBuff, DES_BLOCKSIZE);
	if (ONE_BLOCK != blockNum)
	{
		single_DESMAC(tempPtr, keyBuff, TEMPBuffer, tempI, temArry);
	}

	tempPtr = PX8_A_U8(srcAddress , tempI);
	Triple_DES(tempPtr, keyBuff, MEMtPX8(TEMPBuffer), U8t16(DES_BLOCKSIZE), temArry, U32t8(CBC_Encrypt), SENSITIVE_DATA_MASKED);
	//E2P_fill_array((memref)ICVBuff, 8, 0x00);
	memCopyNoAtomic(ICVBuff, MEMtPX8(temArry), DES_BLOCKSIZE);
	memRefresh();
}

void boot_decryptCommandDataSCP0102(u8 dataLen)
{
	//REVIEW
	PX8				 tempPtr;
	u8		X_A   	 ICV[DES_BLOCKSIZE];

	tempPtr = PX8_A_U16(APDUBuffer , OFFSET_CDATA);
	ecpMem_Set(MEMtPX8(ICV), CLEARED_VALUE, DES_BLOCKSIZE);

	Triple_DES(tempPtr, MEMtPN8(g_bootSCPsessionKey), tempPtr, U8t16(dataLen), MEMtPX8(ICV), CBC_Decrypt, SENSITIVE_DATA_MASKED);
}

/*
 *
 *	Decrypy command data in APDU buffer
 *
 * */
u8 boot_decryptCommandData(void)
{
	u8 		X_A		dataLen;
	u8 		X_A		bOK;
	u8 		X_A		blockSize;
	u8 		X_A		blockMask;
	u8 		X_A		bLe;
	PX8 			tempPtr;
	u8		X_A		mac[DES_BLOCKSIZE];

	bOK = OK;
	blockSize = 0;
	blockMask = 0;

	dataLen = APDUBuffer[OFFSET_LC];

	bLe = U8_A_U8(dataLen, OFFSET_CDATA);
	bLe = APDUBuffer[bLe];

	if( 0x00 == dataLen )
	{
	}
	else
	{
		{
			blockSize = DES_BLOCKSIZE;
			blockMask = DES_BLOCKSIZE_MASK;
		}

		// second deCrypto commandData.
		if (dataLen >= blockSize || dataLen == 0x03)//修改处，原代码为(dataLen >= blockSize)
		{
			switch(g_current_SCP_Type)
			{
				case 0x02:
					if(dataLen == DES_BLOCKSIZE)
						return OK;
					if (U8_AND_U8(dataLen, blockMask) != 0)
					{
						bOK = NOK; // SW_WRONG_LENGTH;
					}
					dataLen = U8_S_U8(dataLen, blockSize);
					tempPtr = PX8_A_U16(APDUBuffer , OFFSET_CDATA);
					tempPtr = PX8_A_U8(tempPtr , dataLen);
					ecpMem_Copy(MEMtPX8(mac), tempPtr, blockSize);
					boot_decryptCommandDataSCP0102(dataLen);
					break;
				default:
					break;
			}

		}

		// third dePadding deCryptoed commandData.
		{
			tempPtr = PX8_A_U16(APDUBuffer , OFFSET_LC);
			tempPtr = PX8_A_U16(tempPtr , dataLen);

			while(( 0x00 == PX_R_U8(tempPtr)) && ( 0x00 != dataLen ) )
			{
				PX8_DEC(tempPtr);
				U8_DEC(dataLen);
			}

			if( 0x80 == PX_R_U8(tempPtr))
			{
				if(dataLen != 0)
				{
					U8_DEC(dataLen);
				}
				else
				{
					bOK = NOK;
				}
			}
			else
			{
				bOK = NOK;//wrong decrypted data
				if(0 == APDUBuffer[OFFSET_LC] || APDUBuffer[OFFSET_CDATA] == APDUBuffer[U8_A_U8(OFFSET_CDATA, 2)]) //修改处
					bOK = OK;
			}

			//update APDULc

			switch(g_current_SCP_Type)
			{
				case 0x02:
					APDUBuffer[OFFSET_LC] = U8_A_U8(dataLen, MAC_LENGTH);
					// forth recover mac to APDUBuffer
					dataLen = U8_A_U8(dataLen, OFFSET_CDATA);
					tempPtr = PX8_A_U16(APDUBuffer , dataLen);
					ecpMem_Copy(tempPtr, MEMtPX8(mac), MAC_LENGTH);
					break;
				default:
					break;
			}

		}
	}

	return bOK;
}


u8 boot_verifyMAC(void)
{
	#define SCP_02 0x02
	u8 		X_A		savaForAPDUCLA;
	u8 		X_A		bOK;
	u8 		X_A		len;
	u8 		X_A		bLc;
	u8    	X_A    	outTempBuf[8];

	bOK 			= NOK;
	bLc 			= APDUBuffer[OFFSET_LC];

	if (bLc < 8)
	{
		bOK = OTHER;
	}
	else
	{
		//the logic channel number is not used in verifyMac
		savaForAPDUCLA = APDUBuffer[OFFSET_CLA];
		APDUBuffer[OFFSET_CLA] = U8_AND_U8(APDUBuffer[OFFSET_CLA], 0xFC);

		// offset to MAC : APDUHEAD(5 Byte) + DATALEN - 8
		len = U8_S_U8(bLc, MACLENGTH_SUB_APDUHEADSIZE);

		ecpMem_Set(MEMtPX8(TEMPBuffer), CLEARED_VALUE, 16);

		//copy MAC to temp buffer
		ecpMem_Copy(PX8_A_U16(MEMtPX8(TEMPBuffer) , 0x10), PX8_A_U8(APDUBuffer , len), MAC_LENGTH);

		switch(g_current_SCP_Type)
		{
		case SCP_02 ://todo:change
			boot_CalMACSCP02(APDUBuffer, len, MEMtPM8(g_bootSCPmacKey), MEMtPM8(g_bootSCPlastMAC));
				break;

			default:
				break;
		}

		//after verifyMac ,then recover the logic channel number
		APDUBuffer[OFFSET_CLA] = savaForAPDUCLA;

		//compare MAC,OK equals to 1 in our platform,but memCompare is 0 when the compared data is the same(zm)
		//bOK = (ECP_CMP_RESULT_EQUAL==memCompare(MEMtPX8((PX8_A_U16(TEMPBuffer , 0x10))), MEMtPM8(TEMPBuffer), MAC_LENGTH)? OK : NOK);
		if(memCompare(MEMtPM8((PX8_A_U16(MEMtPX8(TEMPBuffer) , 0x10))), MEMtPM8(TEMPBuffer), MAC_LENGTH) == ECP_CMP_RESULT_EQUAL)
		{
			bOK		=	OK;
		}
		else
		{
			bOK		= 	NOK;
		}

		//after compare MAC ,Clear the TEMPBuffer.
		ecpMem_Set(TEMPBuffer, CLEARED_VALUE, MAC_LENGTH);

		if ( SCP_02 == g_current_SCP_Type )
		{
			//update MAC if needed
			if( SC0102_ICV_FOR_CMAC == U8_AND_U8(g_current_SCP_I_Option, SC0102_ICV_FOR_CMAC) )
			{
				single_DESMAC(MEMtPX8(g_bootSCPlastMAC), MEMtPM8(g_bootSCPmacKey), outTempBuf, U8t16(8), MEMtPM8(TEMPBuffer));
				memCopyNoAtomic(MEMtPN8(g_bootSCPlastMAC), outTempBuf, MAC_LENGTH);
			}
		}

		{
#if GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK
			APDUBuffer[OFFSET_CLA] = (u8)(APDUBuffer[OFFSET_CLA] & 0xF0) ;
#endif
			APDUBuffer[OFFSET_LC] = U8_S_U8(APDUBuffer[OFFSET_LC], MAC_LENGTH);
		}

	}

	return bOK;
}


u16  boot_UpdateKey(void)
{
	u16			D_A		errorCode;
	u16			D_A		lifeCycle;
	PM8 				keybase;
	u16         		maskCodeIndex;
	PX8         		tempKeybuff;
	u8          		tembuff[2];
	u8          		maskcode;
	u8          		i;
	u8          		checkValue;
	u8			D_A		p1;

	
	lifeCycle = U8t16(boot_GetLifeCycle());
	p1 = APDUBuffer[APDU_P1_OFFSET];

	apdu_ApduSetIncomingAndReceive();

	if ((Boot_CurPrivilege > BOOT_PRIVILEGE_INIT) || (lifeCycle == BOOT_READY_STATE))
	{
		keybase = MEMtPM8(boot_GetKeyAddress(p1));
		if (0 == keybase)
		{
			return BOOT_INS_NOT_SUPPORTED;
		}

		if (U8_AND_U8(Boot_CurPrivilege, p1) == p1)
		{
			errorCode = boot_CheckIsAuthenticated();
			if (BOOT_NO_ERROR != errorCode)
			{
				return errorCode;
			}
		}
		else
		{
			if (lifeCycle != U32t16(BOOT_READY_STATE))
			{
				return BOOT_INS_NOT_SUPPORTED;
			}
		}

		tempKeybuff 	= MEMtPX8(&APDUBuffer[APDU_DATA_OFFSET]);
		ecpMem_Set(MEMtPX8(&tempKeybuff[BOOT_SINGL_KEY_LEN]), 0, 3);
		Random_number_generator(MEMtPX8(tembuff), 2 , 1);
		maskCodeIndex 	= PXB_R_U16(tembuff);
		maskcode 		=  boot_GetMaskCodeFromIndex(maskCodeIndex);
		for (i = 0; i < BOOT_CRYPT_DATA_LEN; U8_INC(i))
		{
			tempKeybuff[i] = U8_XOR_U8(tempKeybuff[i], maskcode);
		}

		checkValue 											= boot_CalcCheckValue(tempKeybuff, BOOT_CRYPT_DATA_LEN);
		tempKeybuff[BOOT_CHECKVALUE_INDEX] 					= checkValue;
		tempKeybuff[BOOT_MASKCODEINDEX_INDEX] 				= U16t8(U16_SHR_U8(maskCodeIndex, 0x08));
		tempKeybuff[U8_A_U8(BOOT_MASKCODEINDEX_INDEX, 1)] 	= U16t8(U16_AND_U16(maskCodeIndex, 0xFF));
		nvmCopyAtomic(keybase, tempKeybuff, BOOT_KEY_STRUCT_LEN);
		memRefresh();
		return BOOT_NO_ERROR;
	}
	else
	{
		return BOOT_INS_NOT_SUPPORTED;
	}

}

// Decrypy command data in APDU buffer
u16 boot_EncryptCommandData(PX8 inBuff, u16 inLen, PX8 keyBuff)
{
	u8		D_A		tempLen;
	u8		D_A		icv[8];

	ecpMem_Set(icv, 0, 8);
	tempLen = U16t8(U16_AND_U16(inLen, 7));

	if (tempLen != 0)
	{
		tempLen 	= U8_S_U8(8, tempLen);
		ecpMem_Set(PX8_A_U16(inBuff, inLen), 0, tempLen);
		PX_W_U8(PX8_A_U16(inBuff, inLen), 0x80);
		inLen 		=  U16_A_U8(inLen, tempLen);
	}

	Triple_DES(inBuff, keyBuff, inBuff, inLen, icv, CBC_Encrypt, SENSITIVE_DATA_MASKED);
	return inLen;
}

u16 boot_CheckIsAuthenticated (void)
{
	u16		D_A		result;
	u8 				scpOptionBak;
	
	result				= BOOT_NO_ERROR;
	scpOptionBak 		= g_current_SCP_I_Option;

	if (Boot_CurPrivilege == BOOT_PRIVILEGE_INIT)
	{
		return result;
	}
	
	if (boot_decryptCommandData() != 1)
	{
		result = BOOT_INS_NOT_SUPPORTED;
	}

	if (result == BOOT_NO_ERROR)
	{
		g_current_SCP_I_Option = U8_A_U8(g_current_SCP_I_Option, SC0102_ICV_FOR_CMAC);
		if (boot_verifyMAC() != 1)
		{ //APDUBuffer[OFFSET_LC] < 8
			closeSecuritychannel(SC_TERMINATED);
			result = BOOT_INS_NOT_SUPPORTED;
		}
		g_current_SCP_I_Option = scpOptionBak;
	}
	else
	{
		result = BOOT_INS_NOT_SUPPORTED;
	}
	return result;
}

void boot_SetLifeCycleValue(u8 lifeCylcle)
{
	nvmWriteU8Atomic(MEMtPN8(BOOT_LIFECYCLE_BASE), lifeCylcle);
}

void boot_SetSecuritychannel(u8 privilege)
{
	PN8 				 keybase;
	u8 					 keyCTR;
	u8 					 checkValue;
	u8 			    	 maskcode;
	u16 		    	 maskcodeIndex;
	u8 					 tempkeybuff[BOOT_CRYPT_DATA_LEN];

	Boot_CurPrivilege	= U8_OR_U8(Boot_CurPrivilege, privilege);
	keybase 			= MEMtPN8(boot_GetKeyAddress(privilege));
	maskcodeIndex 		= nvmReadU16(PN8_A_CONST(keybase,BOOT_MASKCODEINDEX_INDEX));
	maskcode 			=  boot_GetMaskCodeFromIndex(maskcodeIndex);
	
	keyCTR 				= U16t8(boot_GetKeyCTR(MEMtPM8(keybase)));
	keyCTR 				= U8_XOR_U8(keyCTR, maskcode);
	U8_INC(keyCTR);
	keyCTR 				= U8_XOR_U8(keyCTR, maskcode);

	nvmWriteU8Atomic(PN8_A_CONST(keybase,BOOT_KEYCTR_INDEX), keyCTR);
	nvmWriteU8Atomic(PN8_A_CONST(keybase,BOOT_TRYCTR_INDEX), maskcode);

	memCopyToRam(MEMtPX8(tempkeybuff), MEMtPM8(keybase), BOOT_CRYPT_DATA_LEN);
	tempkeybuff[BOOT_KEYCTR_INDEX] = keyCTR;/*cause keyCTR haven't be wrote to keybase*/
	tempkeybuff[BOOT_TRYCTR_INDEX] = maskcode;

	checkValue 			= boot_CalcCheckValue(MEMtPX8(tempkeybuff), BOOT_CRYPT_DATA_LEN);
	nvmWriteU8Atomic(PN8_A_CONST(keybase,BOOT_CHECKVALUE_INDEX), checkValue);
}

void boot_SetTryCTR(PM8 bootkeybase)
{
	u8         		auth_Trycntr;
	u8         		checkValue;
	u8 			    maskcode;
	u16 		    maskcodeIndex;
	u8         		tempKeybuff[BOOT_KEY_STRUCT_LEN];


	memCopyToRam(MEMtPX8(tempKeybuff), MEMtPM8(bootkeybase), BOOT_KEY_STRUCT_LEN);
	maskcodeIndex 					= PXB_R_U16(PX8_A_CONST(tempKeybuff, BOOT_MASKCODEINDEX_INDEX));
	maskcode 						=  boot_GetMaskCodeFromIndex(maskcodeIndex);


	auth_Trycntr 					= boot_GetTryCTR(MEMtPM8(tempKeybuff));
	auth_Trycntr 					= U8_XOR_U8(auth_Trycntr, maskcode);
	U8_INC(auth_Trycntr);
	auth_Trycntr 					= U8_XOR_U8(auth_Trycntr, maskcode);

	nvmWriteU8Atomic(PM8_A_CONST(bootkeybase, BOOT_TRYCTR_INDEX), auth_Trycntr);
	tempKeybuff[BOOT_TRYCTR_INDEX] 	= auth_Trycntr;
	checkValue 						= boot_CalcCheckValue(MEMtPX8(tempKeybuff), BOOT_CRYPT_DATA_LEN);
	nvmWriteU8Atomic(PM8_A_CONST(bootkeybase, BOOT_CHECKVALUE_INDEX), checkValue);

	memRefresh();
}

u8 boot_setSupportOldVersionOrComponent(u8 type)
{
	PN8		desAddress;
	u8		curLifeCycle;

	desAddress 		= MEMtPN8(BOOT_COMMAND_OLD_COMPONENT_BASE);
	curLifeCycle 	= boot_GetLifeCycle();

	switch(type)
	{
		case BOOT_SUPPORT_COMPONENT_PRIVATE_COMMAND:
			if (!((BOOT_IS_HAD_CURRENT_PRIV(BOOT_PRIVILEGE_UL)) || (BOOT_READY_STATE == curLifeCycle)))
			{
				return false;
			}
			break;
		case BOOT_NOT_SUPPORT_COMPONENT_PRIVATE_COMMAND:
			if (BOOT_LOCKED_STATE == curLifeCycle)
			{
				return false;
			}
			break;
		default:
			return false;
	}
	memWriteU8Atomic(desAddress, type);
	return true;
}

u16 boot_SetCardState()
{
	u16			D_A		errorCode;
	u8			D_A		curLifeCycle;
	u8			D_A		p1;
	u8                  keyType;
	PM8 				bootConfigKey;
	
	p1				= APDUBuffer[APDU_P1_OFFSET];
	errorCode 		= BOOT_NO_ERROR;
	curLifeCycle 	= boot_GetLifeCycle();
	
	if (boot_setSupportOldVersionOrComponent(p1))
	{
		return errorCode;
	}

	apdu_ApduSetIncomingAndReceive();
	errorCode = boot_CheckIsAuthenticated();
	if (BOOT_NO_ERROR != errorCode)
	{
		return errorCode;
	}

	switch (curLifeCycle)
	{
		case BOOT_READY_STATE:
			if (BOOT_INITIALIZED_STATE == p1)
			{
				keyType = BOOT_PRIVILEGE_CM;
				for (p1 = 0; p1 < 4; U8_INC(p1))
				{
					bootConfigKey = MEMtPM8(boot_GetKeyAddress(keyType));
					if (BOOT_KEY_VALUE_OK != boot_VerifyCheckValue(bootConfigKey, BOOT_VERIFY_KEY_INIT_VALUE))
					{
						return BOOT_INS_NOT_SUPPORTED;
					}
					keyType = U8_SHL_U8(keyType, 1);
				}
				boot_SetLifeCycleValue(BOOT_INITIALIZED_STATE);
			}
			else if (BOOT_TERMINATED_STATE == p1)
			{
				boot_SetLifeCycleValue(p1);
			}
			else
			{
				return BOOT_INS_NOT_SUPPORTED;
			}
			break;
		case BOOT_INITIALIZED_STATE:
			if ((BOOT_SECURED_STATE == p1) || (BOOT_TERMINATED_STATE == p1))
			{

				boot_SetLifeCycleValue(p1);
			}
			else
			{
				return BOOT_INS_NOT_SUPPORTED;
			}
			break;
		case BOOT_SECURED_STATE:
			if ((BOOT_LOCKED_STATE == p1) || (BOOT_TERMINATED_STATE == p1))
			{
			
				boot_SetLifeCycleValue(p1);
				Boot_CurPrivilege = BOOT_PRIVILEGE_INIT;
			}
			else
			{
				return BOOT_INS_NOT_SUPPORTED;
			}
			break;
		case BOOT_LOCKED_STATE:
			if(BOOT_IS_HAD_CURRENT_PRIV(BOOT_PRIVILEGE_UL) && ((BOOT_SECURED_STATE == p1) || (BOOT_TERMINATED_STATE == p1)))
			{
				boot_SetLifeCycleValue(p1);
			}
			else
			{
				return BOOT_INS_NOT_SUPPORTED;
			}
			break;
		case BOOT_TERMINATED_STATE:
			return BOOT_INS_NOT_SUPPORTED;
		default:
			return BOOT_INS_NOT_SUPPORTED;
	}

	return errorCode;
}

u8 boot_GetLifeCycle(void)
{//MEMtPN8(JcvmConfig_GetVmConfigAddr(configTaglist[BOOT_LIFECYCLE_BASE].configAddress))
	return nvmReadU8(MEMtPN8(BOOT_LIFECYCLE_BASE));
}

u8 boot_GetTryCTR (PM8 bootkeybase)
{
	return   memReadU8(PM8_A_CONST(bootkeybase,BOOT_TRYCTR_INDEX));
}


u16 boot_GetKeyCTR (PM8 bootkeybase)
{
	return   U8t16(memReadU8(MEMtPN8(PM8_A_CONST(bootkeybase,BOOT_KEYCTR_INDEX))));
}

u8 boot_GetMaskCodeFromIndex(u16 index)
{
	//return  MEM_read_u8(U32_A_U16(BOOT_CONFIG_MASKCODE_BASE,index));
	return BOOT_MASKCODE;
}
/*
 * 注：如要获取解锁权限，必须具有其他所有权限，
 * */

memref boot_GetKeyAddress(u8 privilege)
{
	memref keyBase;

	keyBase = 0;

	switch (privilege)
	{
		case BOOT_PRIVILEGE_CM:
			keyBase = BOOT_CM_KEY_BASE;
			break;
		case BOOT_PRIVILEGE_PM:
			keyBase = BOOT_PM_KEY_BASE;
			break;
		case BOOT_PRIVILEGE_NM:
			keyBase = BOOT_NM_KEY_BASE;
			break;
		case BOOT_PRIVILEGE_UL:
			keyBase = BOOT_UL_KEY_BASE;
			break;
		default:
			break;
	}
	return keyBase;
}

u8 boot_CalcCheckValue (PX8 src, u16 keyLen)
{
	u8			D_A		checkvalue;
	u16 		D_A		i;
	PX8					temp;


	temp 		= src;
	checkvalue 	= 0;

	for(i=0; i < keyLen; U16_INC(i))
	{
		checkvalue = U8_XOR_U8(checkvalue, PX_R_U8(temp));
		PX8_INC(temp);
	}
	return checkvalue;
}

u8 boot_VerifyCheckValue(PM8 bootkey, u8 mode)
{
	u8		    D_A		checkvalue;
	u8 			X_A		tempkeybuff[BOOT_KEY_STRUCT_LEN];
	u8 			D_A		tempkey[BOOT_CRYPT_DATA_LEN];

	memCopyToRam(MEMtPX8(tempkeybuff), bootkey, BOOT_KEY_STRUCT_LEN);
	checkvalue			= PX_R_U8(PX8_A_CONST(tempkeybuff,BOOT_CHECKVALUE_INDEX));

	if (BOOT_VERIFY_KEY_CHECK_VALUE == mode)
	{
		if(checkvalue != boot_CalcCheckValue(MEMtPX8(tempkeybuff), BOOT_CRYPT_DATA_LEN))
		{
			return BOOT_KEY_VALUE_FAIL;
		}
	}
	else
	{
		ecpMem_Set(tempkey, NVM_DEFAULT_VALUE, BOOT_CRYPT_DATA_LEN);
		if (ecpMem_Compare(tempkey, tempkeybuff, BOOT_CRYPT_DATA_LEN) == 0)
		{
			return BOOT_KEY_VALUE_FAIL;
		}
	}
	return BOOT_KEY_VALUE_OK;
}

void boot_DecrytoKey(PM8 cipherKey, PX8 outbuff)
{
	u8 					 maskcode;
	u8 					 i;
	u16 				 maskcodeIndex;
	PX8					 temp;

	memCopyToRam(outbuff, cipherKey, BOOT_KEY_STRUCT_LEN);
	maskcodeIndex 		= PXB_R_U16(PX8_A_CONST(outbuff,BOOT_MASKCODEINDEX_INDEX));
	maskcode 			= boot_GetMaskCodeFromIndex(maskcodeIndex);
	temp 				= outbuff;

	for (i=0; i<BOOT_CRYPT_DATA_LEN; U8_INC(i))
	{
		PX_W_U8(temp, U8_XOR_U8(PX_R_U8(temp), maskcode));
		PX8_INC(temp);
	}
}

// Generate Session/MAC Key
void boot_GenerateSessionKey(PN8 staticKey, u16 sequenceCounter)		SMALL_MODE
{
	PX8				X_A		pt;
	u8				D_A		icv[8];
	u8				X_A		bTemp[16];

	ecpMem_Set(bTemp, 0, 16);
	PXB_W_U8(&bTemp[2],HIGH(sequenceCounter));
	PXB_W_U8(&bTemp[3],LOW(sequenceCounter));

	// scp02void cons
	bTemp[0] 	= 0x01;
	bTemp[1]	= CONSTANTS_S_ENC;
	ecpMem_Set(icv, 0x00, 0x08);
	pt			= g_bootSCPsessionKey;

	Triple_DES(MEMtPX8(bTemp), MEMtPM8(staticKey), pt, 16, icv, CBC_Encrypt, SENSITIVE_DATA_MASKED);
	memCopyNoAtomic(MEMtPM8(g_current_SCP_SES_KEY), pt, 16);//modify by jiangxiaohui@20220324 
	
	bTemp[1]	= CONSTANTS_C_MAC;
	ecpMem_Set(icv, 0x00, 0x08);
	pt			= g_bootSCPmacKey;

	Triple_DES(MEMtPX8(bTemp), MEMtPM8(staticKey), pt, 16, icv, CBC_Encrypt, SENSITIVE_DATA_MASKED);

	bTemp[1]	= CONSTANTS_DEK;
	ecpMem_Set(icv, 0x00, 0x08);
	pt			= g_bootSCPdekKey;

	Triple_DES(MEMtPX8(bTemp), MEMtPM8(staticKey), pt, 16, icv, CBC_Encrypt, SENSITIVE_DATA_MASKED);

	ecpMem_Set(g_bootSCPlastMAC, 0, 8);
}

u8 boot_IsSupportPrevission(void)
{
	return nvmReadU8(MEMtPN8(BOOT_COMMAND_OLD_COMPONENT_BASE));
}

/*
Parameter:
	keyType: e.g:ki, Akey
  keyBuffer: to store the network key from this buffer
	keyBufferOffset: the offset of the keyBuffer
  keyLength: length of the key

function: set the key to the real storage address.
*/
void bootSetKey(u8 keyType, PX8 keyBuffer, u8 keyBufferOffset, u8 keylength)
{
		u8 						keyTmpType;
		u8 						maskcode;
		u8 						checkValue;
		u8						tembuff[3];									//to store maskCodeIndex(2bytes) + checkValue(1byte)
		u16 					i;
		u16						keyLen;
		u16						maskCodeIndex;
		PX8						tempKeybuff;
		PN8 					tmpAddress;

		if(NVM_DEFAULT_VALUE == keyType)
		{
			throw_error(BOOT_DATA_INVALID);
		}

		keyLen					= U8t16(keylength);
		tempKeybuff 			= PX8_A_U8(keyBuffer, keyBufferOffset);
		Random_number_generator(MEMtPX8(tembuff), 2 , 1);
		maskCodeIndex 			= PXB_R_U16(tembuff);
		maskcode 				=  boot_GetMaskCodeFromIndex(maskCodeIndex);
		for (i = 0; i < keyLen; U16_INC(i))
		{
			tempKeybuff[i]		= U8_XOR_U8(tempKeybuff[i], maskcode);
		}

		checkValue 				= boot_CalcCheckValue(tempKeybuff, keyLen);
		tembuff[2] 				= checkValue;
		//find the address that can be used
		tmpAddress 				= MEMtPN8(BOOT_COMMON_KEY_BASE);
		do
		{
			keyTmpType 			= nvmReadU8(tmpAddress);
			i 					= nvmReadU16(PN8_A_CONST(tmpAddress, 1));
			
			if(PN8_A_U16(tmpAddress, U16_A_U8(keyLen, 6)) > MEMtPN8(BOOT_CONFIG_KEY_END))
			{
				throw_error(BOOT_NO_RESOURCE);
			}
			if ((keyTmpType == keyType) || ((keyTmpType == NVM_DEFAULT_VALUE) && (i == NVM_DEFAULT_VALUE_TWO_BYTE)))
			{
				nvmWriteU8Atomic(tmpAddress, keyType);
				nvmWriteU16Atomic(PM8_A_CONST(tmpAddress, 1), U16_A_U8(keyLen, 3));
				nvmCopyAtomic(PN8_A_CONST(tmpAddress, 3), tempKeybuff, keyLen);
				nvmCopyAtomic(PN8_A_CONST(tmpAddress, U16_A_U8(keyLen, 3)), tembuff, 3);
				memRefresh();
				break;
			}
			tmpAddress	= PN8_A_U8(tmpAddress, i);
			tmpAddress	= PN8_A_CONST(tmpAddress, 3);								//calc type(1byte) + len of key(2byte)
		}while (tmpAddress < MEMtPN8(BOOT_CONFIG_KEY_END));
}

/*
Parameter:
	keyType: e.g:ki, Akey
  keyBuffer: the buffer to be stored for the network key
	keyBufferOffset: the offset of the keyBuffer
  keyLength: length of the key

function: get the key from the real storage address.
*/
#define  MAX_BOOT_KEY_LENGTH         32
u8 bootGetKey(u8 keyType, PX8 keyBuffer, u8 keyBufferOffset)
{
	u8 			find;
	u8 			keyTmpType;
	u8 			checkValue;
	u8 			maskcode;
	u16 		i;
	u16 		tempKeyLen;
	u16 		maskCodeIndex;
	PX8		 	tempKeybuff;
	PN8			tmpAddress;
	u8          storeKeyInf[U8_A_U8(MAX_BOOT_KEY_LENGTH, 3)];

	find 			= 0;
	//find the address that can be used
	tmpAddress 		= MEMtPN8(BOOT_COMMON_KEY_BASE);
	do
	{
		keyTmpType 	= nvmReadU8(tmpAddress);
		tempKeyLen 	= nvmReadU16(PM8_A_CONST(tmpAddress, 1));
		
		if(NVM_DEFAULT_VALUE == keyTmpType)
		{
			return 2;
		}

		if (keyTmpType == keyType)
		{
			memCopyToRam(MEMtPX8(storeKeyInf), PM8_A_U8(tmpAddress, 3), tempKeyLen);
			find = 1;
			break;
		}
		tmpAddress	= PN8_A_U16(tmpAddress, tempKeyLen);
		tmpAddress	= PN8_A_CONST(tmpAddress, 3);								//calc type(1byte) + len of key(2byte)
	}while (tmpAddress < MEMtPN8(BOOT_CONFIG_KEY_END));

	if (0 == find)
	{
		return 2;
	}
	
	checkValue 		= storeKeyInf[U16_S_U8(tempKeyLen, 1)];
	tempKeyLen 		= U16_S_U8(tempKeyLen, 3);
	if (checkValue != boot_CalcCheckValue(MEMtPX8(storeKeyInf), tempKeyLen))
	{
		return 1;
	}
	
	maskCodeIndex 	= PXB_R_U16(PX8_A_U16(MEMtPX8(storeKeyInf), tempKeyLen));

	maskcode 		=  BOOT_MASKCODE;

	for (i = 0; i < tempKeyLen; U16_INC(i))
	{
		tempKeybuff[i] = U16t8(U16_XOR_U16(U8t16(tempKeybuff[i]), U8t16(maskcode)));
	}
	tempKeybuff 	= PX8_A_U8(keyBuffer, keyBufferOffset);
	ecpMem_Copy(tempKeybuff,MEMtPX8(storeKeyInf),tempKeyLen);
	return 0;
}

u16 clearCard(void)
{
	apdu_ApduSetIncomingAndReceive();
	if(boot_unwrap(BOOT_PRIVILEGE_PM) != BOOT_NO_ERROR)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}
	
	ecpSysHal_ClearCard();
  return BOOT_NO_ERROR;
}

u16 clearAllEeprom(PX8 apduBuffer)
{
	u32			D_A		remainSize;
	PN8			D_A		addr;

	apdu_ApduSetIncomingAndReceive();

	
	if(boot_unwrap(BOOT_PRIVILEGE_CM) != BOOT_NO_ERROR)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}

	addr =  MEMtPN8(E2PBase);
	remainSize 	= U32_S_U32(PN_R_U32(MEMtPN8(sysConfig_GetNvmTopBase())),  PNtMEM(addr));
/*
	remainSize 	= U32_S_U32(PN_R_U32(MEMtPN8(sysConfig_GetNvmTopBase())), PNtMEM(nvmBottom));
	addr 		= nvmBottom;
	*/
// 	while(remainSize > MAX_CLEAR)
// 	{
// 		fillEeprom(MEMtPN8(addr), NVM_FILL_CHARACTER, MAX_CLEAR);
// 		addr = MEMtPN8(PN8_A_CONST(addr, MAX_CLEAR));
// 		remainSize = U32_S_U16(remainSize, MAX_CLEAR);
// 	}

// 	fillEeprom(MEMtPN8(addr), NVM_FILL_CHARACTER, U32t16(remainSize));
	while(remainSize > MAX_CLEAR)
	{
		nvmClearNoCache(MEMtPN8(addr), MAX_CLEAR);
		addr = MEMtPN8(PN8_A_CONST(addr, MAX_CLEAR));
		remainSize = U32_S_U16(remainSize, MAX_CLEAR);
	}
	nvmClearNoCache(MEMtPN8(addr), U32t16(remainSize));

	BootLifeCycle_SetLifeCycle(LIFE_CYCLE_READY);

	IF_SetUPCOSFileSystemAddr();
	
	//release mifare card
	Mifare_ReleaseCard();
	
	swp_ResetSwpConfigData();

	return BOOT_NO_ERROR;
}

u16 Read_EEPROM(PX8 apduBuffer)
{
	u16			D_A		Lr;
	u16			D_A		offset;
	PN8			D_A		addr;

	if( SetBank0Base == ILLEGL_BANK)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}

	Lr = U8t16(apduBuffer[APDU_P3_OFFSET]);

	if(Lr == 0)
	{
		Lr = 256;
	}
	
	addr = PN8_A_U16(SetBank0Base, U16_OR_U16(U16_SHL_U8(U8t16(apduBuffer[OFFSET_P1]), 8), U8t16(apduBuffer[OFFSET_P2])));

	if ( isInRom(addr) || isInRom(PN8_A_U16(addr,U16_S_U16(Lr, 1))) )
	{
		return 0x6985;// can't read mask
	}

// 	if (!(BOOT_IS_HAD_CURRENT_PRIV(BOOT_PRIVILEGE_NM) || (BOOT_READY_STATE == boot_GetLifeCycle())))
// 	{
// 		return BOOT_COMMAND_NOT_ALLOWED;
// 	}

	offset	= U16_OR_U16(U16_SHL_U8(U8t16(apduBuffer[APDU_P1_OFFSET]), 8), apduBuffer[APDU_P2_OFFSET]);
	addr	= PN8_A_U16(SetBank0Base , offset);

	ecpMem_Copy(apduBuffer, addr, Lr);

	if (Boot_CurPrivilege > BOOT_PRIVILEGE_INIT)
	{
		//after the authentication, the length that to be read must be multiple of 8 bytes.
		Lr = boot_EncryptCommandData(apduBuffer, Lr, g_bootSCPsessionKey);
	}

	apdu_ApduSetOutgoingAndSend(0,Lr);

	return BOOT_NO_ERROR;
}

u16 attainAndVerifyMemoryCode(PX8 apduBuffer)
{

	u32			D_A		length;
	u16			D_A		result;
	u16			D_A		offset;
	PN8			D_A		addr;
	u8 			D_A		temp[2];
	u8 			D_A		ins;

	ins = apduBuffer[APDU_P1_OFFSET];
	if( ILLEGL_BANK == SetBank0Base)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}

	result	= 0;
	offset	= U16_OR_U16(U16_SHL_U8(U8t16(apduBuffer[2]), 8), apduBuffer[3]);
	addr	= PN8_A_U16(SetBank0Base , offset);

	apdu_ApduSetIncomingAndReceive();

	if(boot_unwrap(BOOT_PRIVILEGE_CM) != BOOT_NO_ERROR)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}

	length = PXB_R_U32(PX8_A_CONST(apduBuffer, 5));

	result = Get_crc16(addr, length);
	PXB_W_U16(temp, result);
	if((INS_GET_CRC_NEW == apduBuffer[APDU_INS_OFFSET]) || (INS_GET_CRC == apduBuffer[APDU_INS_OFFSET]))
	{
		PXB_W_U16(apduBuffer , result);
		apdu_ApduSetOutgoingAndSend(0,2);
	}
	else
	{
	  	if( memCompare(PX8_A_CONST(apduBuffer, 9), MEMtPM8(temp), 2) !=ECP_CMP_RESULT_EQUAL)
	  	{
	    	return  BOOT_CRC_RESULT_COMPARE_FAILED;
	  	}
	}

	return BOOT_NO_ERROR;

}

u16 Write_EEPROM(PX8 apduBuffer)
{
	u16			D_A			offset;
	PN8			D_A			addr;

	if( SetBank0Base == ILLEGL_BANK)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}
	offset	= U16_OR_U16(U16_SHL_U8(U8t16(apduBuffer[2]), 8), apduBuffer[3]);
	addr	= PN8_A_U16(SetBank0Base, offset);

	if (isInRom(addr) || isInRom(PN8_A_U16(addr,U16_S_U16(U8t16(apduBuffer[APDU_P3_OFFSET]), 1))))
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;// can't write mask
	}


	apdu_ApduSetIncomingAndReceive();
	if(boot_unwrap(BOOT_PRIVILEGE_NM) != BOOT_NO_ERROR)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}
	nvmCopyLost(addr, PX8_A_CONST(apduBuffer, 5), apduBuffer[4]);
	//nvmCopyNoAtomic(addr, PX8_A_CONST(apduBuffer, 5), apduBuffer[4]);
	
	return BOOT_NO_ERROR;
}

u16 Write_EEPROM_Page(PX8 apduBuffer)
{
	u16			D_A			offset;
	PN8 					addr;
	u32 					bLen;

	//offset	= U16_OR_U16(U16_SHL_U8(apduBuffer[2], 8), apduBuffer[3]);
	offset	= U16_SHL_U8(U8t16(apduBuffer[APDU_P1_OFFSET]), 8);
	addr	= PN8_A_U16(SetBank0Base, offset);

	if (BOOT_READY_STATE != boot_GetLifeCycle())
	{
		return BOOT_INS_NOT_SUPPORTED;
	}

	apdu_ApduSetIncomingAndReceive();

	if(apduBuffer[APDU_P3_OFFSET] !=0xFF)
	{
		bLen = U32_A_U32(U8t32(apduBuffer[APDU_P3_OFFSET]), 1);
	}
	else
	{
		bLen = 0x100;
	}
	
	if (isInRom(addr) || isInRom(PN8_A_U16(addr,U16_S_U16(U32t16(bLen),1))))
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;// can't write mask
	}

	apduBuffer[APDU_P3_OFFSET] = apduBuffer[APDU_P2_OFFSET];

	nvmCopyLost(addr, PX8_A_CONST(apduBuffer, 4), bLen);
	
 	return BOOT_NO_ERROR;
}

u16 BlockSelect(u8 * apduBuffer)
{
	if (apduBuffer[APDU_P2_OFFSET] > 0x0E)//(ROM_AREA_START-COS_CONFIG_START)/0x010000
	{
		return BOOT_BLOCK_SELECT_INVALID_P2;
	}

	apdu_ApduSetIncomingAndReceive();
	if(boot_unwrap(U8_OR_U8(BOOT_PRIVILEGE_CM, BOOT_PRIVILEGE_NM)) != BOOT_NO_ERROR)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}	
	if (apduBuffer[OFFSET_LC] != 0)
	{
		return 0x6700;
	}
	SetBank0Base = MEMtPN8(U8_M_U32(apduBuffer[3] , 0x010000));
	SetBank0Base = MEMtPN8(PM8_A_U32(SetBank0Base , EEPROM_BASE));

	return BOOT_NO_ERROR;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 CheckAtrInterfaceBytes                                                                                                                   
                                                                                                                                                
  Input parameter:   atr                                                                  
  					                                                                                                         
  Output parameter:  b8: atr中有无tck字节; 
                     b7-b1: InterfaceBytes末字节的偏移 
  
  Description:  检查atr的接口字节                                                                                                       

 -------------------------------------------------------------------------------------------------------------------*/    
u8 CheckAtrInterfaceBytes(PR8 atr)
{
    u8		offset;
    u8		i;
    u8		tckTag;

    tckTag 	= 0;
	offset 	= 1;	//point to T0 byte

	while(1)
	{
		if(offset > 33 )
		{
			return false;	//ISO7816-3
		}

		if( U8_AND_U8(atr[offset], 0xf0) == 0)
		{
			break;
		}
/*@ -type @*/
		i	=  (U8_AND_U8(atr[offset], 0x10) == 0x10) ? 1:0;
/*@ +type @*/
		i 	=  U8_A_U8(i, (U8_AND_U8(atr[offset], 0x20) == 0x20) ? 1:0);
		i 	=  U8_A_U8(i, (U8_AND_U8(atr[offset], 0x40) == 0x40) ? 1:0);

		if(U8_AND_U8(atr[offset], 0x80) == 0x80)
		{
			U8_INC(i);

			offset	= U8_A_U8(offset, i); //point to TDi byte

			if(U8_AND_U8(atr[offset], 0x0f) != 0)
			{
				tckTag = 1;	//tck byte
			}
		}
		else
		{
			offset	= U8_A_U8(offset, i);
			break;
		}
	} //offset now point to the end of interface bytes

	return tckTag ? U8_A_U8(0x80, offset) : offset;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 modifyAtrValue                                                                                                                   
                                                                                                                                                
  Input parameter:   coldOrWarm，要修改冷复位的ATR，还是热复位的ATR
					 hitoryOrWhole，修改对应ATR的历史字节，还是接口字节

  Description:       将目标位置的数据更改为：长度为length的buf数据

 -------------------------------------------------------------------------------------------------------------------*/   
u16 modifyAtrValue(u8 coldOrWarm, u8 hitoryOrWhole, PR8 buf, u8 length)
{
	PN8		ATR[4];							//point to "len, if, len, his"
	u8 		tckTag;
	u8 		offset;
	u8 		value;
	
	//point the cold reset atr
	if(coldOrWarm == TO_MODIFY_COLDATR_FLAG)
	{
		ATR[0]	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_COLD_IFLEN));
		ATR[1] 	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_COLD_IFBYTES));
		ATR[2]	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_COLD_HISTLEN));
		ATR[3] 	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_COLD_HISTBYTES));
	}
	//point to the warm reset atr
	else if(coldOrWarm == TO_MODIFY_WARMATR_FLAG)
	{
		ATR[0]	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_WARM_IFLEN));
		ATR[1] 	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_WARM_IFBYTES));
		ATR[2]	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_WARM_HISTLEN));
		ATR[3] 	= MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_WARM_HISTBYTES));
	}
	else
	{
		return FALSE;
	}

	//modify the history bytes and length
	if(hitoryOrWhole == TO_MODIFY_HIS_ONLY)
	{
		if(length > 0x0f)
		{
			return FALSE;
		}
		nvmWriteU8Atomic(ATR[2],length);
		nvmCopyAtomic(ATR[3], buf, length);
	}
	//modify the whole atr
	else if(hitoryOrWhole == TO_MODIFY_WHOLE_ATR)
	{
		if(length > 30)
		{
			return FALSE;
		}

		tckTag = CheckAtrInterfaceBytes(buf);
		if(!tckTag)
		{
			return FALSE;
		}

		offset = U8_AND_U8(tckTag,0x7F);
		nvmWriteU8Atomic(ATR[0],offset);
		nvmCopyAtomic(ATR[1], MEMtPN8(PX8_A_U8(buf,1)), offset);
		nvmWriteU8Atomic(ATR[2], U8_AND_U8(buf[1], 0x0f));
		nvmCopyAtomic(ATR[3], PX8_A_U8(buf,U8_A_U8(offset, 1)), U8_AND_U8(buf[1], 0x0f));
	}
	else
	{
		return FALSE;
	}

	//tck present or not; 7816-3
	tckTag = U8_AND_U8(tckTag, 0x80);
	if(hitoryOrWhole == TO_MODIFY_WHOLE_ATR)
	{
		if(coldOrWarm == TO_MODIFY_COLDATR_FLAG)
		{//enable cold tck
			tckTag = U8_SHR_U8(tckTag,3);
			value = PN_R_U8(MEMtPN8(sysConfig_GetUartConf()));
			value = U8_AND_U8(value ,0xef);
			value = U8_OR_U8(value, tckTag);
			nvmWriteU8Atomic(MEMtPN8(sysConfig_GetUartConf()), value);
		}
		else
		{//enable warm tck
			tckTag = U8_SHR_U8(tckTag, 2);
			value = PN_R_U8(MEMtPN8(sysConfig_GetUartConf()));
			value = U8_AND_U8(value, 0xdf);
			value = U8_OR_U8(value, tckTag);
			nvmWriteU8Atomic(MEMtPN8(sysConfig_GetUartConf()), value);
		}
	}
	
	return true;
}

u8 modifyAtr(PX8 commandBuffer,u8 operateType)
{
	u16 configName;
	u8 	coldOrWarm;

	configName = PXB_R_U16(&commandBuffer[APDU_P1_OFFSET]);

	if(commandBuffer[APDU_DATA_OFFSET] > 1)
	{
		return false;
	}

	if(configName == COLD_ATR_TAG)
	{
		coldOrWarm = TO_MODIFY_COLDATR_FLAG;
	}
	else
	{
		coldOrWarm = TO_MODIFY_WARMATR_FLAG;
	}

	return U16t8(modifyAtrValue(coldOrWarm, commandBuffer[APDU_DATA_OFFSET], MEMtPX8(&commandBuffer[U8_A_U8(APDU_DATA_OFFSET, 1)]), U8_S_U8(commandBuffer[APDU_P3_OFFSET], 1)));
}

u8 writeDataOfSelectAppByFileId(PX8 commandBuffer, u8 operateType)
{
	PN8				modeIdAddr;
	PN8 			dFileIdAddr;
	u8 				dataLength;

	modeIdAddr	= MEMtPN8(PN_R_U24(MEMtPN8(JcvmConfig_GetVmConfigAddr(CM_FileIdAddr))));
	//dFileIdAddr = 0x00000000 + modeIdAddr;
	dFileIdAddr = PN8_A_U32(modeIdAddr, 0x00000000);
	dataLength	= commandBuffer[APDU_P3_OFFSET];
	
	if(dFileIdAddr >= MEMtPN8(COS_CONFIG_START) && dFileIdAddr < MEMtPN8(ROM_BASE))
	{
		if(operateType == STOREDATA) 
		{
			nvmCopyAtomic(dFileIdAddr, &commandBuffer[APDU_DATA_OFFSET], dataLength);
		}
		else 
		{
			ecpMem_Copy(&commandBuffer[APDU_DATA_OFFSET], dFileIdAddr, dataLength);
			apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, dataLength);
		}
		return true;
	}
	else
	{
		return false;
	}
}

#if BOOT__CONFIGDATA__POWER_OFF_FOR_TEST
#define		HEAP_BOUND_TAG				0xFF21
#define		AUTH_TRY_CNTR_ADDRESS_TAG	0xFFC1

u16 getPrivateData(PX8 apduBuffer, u8 operateType)    SMALL_MODE
{
	u8 D_A version;
	u16 D_A valueTag;
	u16 D_A sequenceCounter;
	//jref D_A tempKeyset;

	if (!(BOOT_IS_HAD_CURRENT_PRIV(BOOT_PRIVILEGE_NM) || (BOOT_READY_STATE == boot_GetLifeCycle())))
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}

	valueTag = U16_OR_U16(U16_SHL_U8(U8t16(apduBuffer[2]), 8), apduBuffer[3]);
	switch(valueTag)
	{
		case HEAP_BOUND_TAG:
			//recordNewEEpromArea();
			//PXB_W_U32(&apduBuffer[0], U32_S_U8(g_maxEntryAdd, 4));
			//PXB_W_U32(&apduBuffer[4], g_usedBlockAdd);
			PXB_W_U32(&apduBuffer[8], NVM_FREE_TABLE_START);
			apdu_ApduSetOutgoingAndSend(0,12);
			break;
		case AUTH_TRY_CNTR_ADDRESS_TAG:
			apdu_ApduSetIncomingAndReceive();
			version = apduBuffer[5];
			//tempKeyset = getKeySet(version);
			//PXB_W_U32(&apduBuffer[0], PNtMEM(keySet_GetSequenceCounterField(MEMtPN8(locateObjectInMemory(tempKeyset)))));
			//sequenceCounter = keySet_getSequenceCounter(MEMtPN8(locateObjectInMemory(tempKeyset)), 0x00);
			PXB_W_U16(&apduBuffer[4], sequenceCounter);
			apdu_ApduSetOutgoingAndSend(0,6);
			break;
		default:
			return BOOT_REFERENCED_DATA_NOT_FOUND;
	}
	return true;
}
#endif
#if BOOT__CONFIGDATA__SENCOND_ATR
u8 secondAtrOperate(PX8 commandBuffer, u8 operateType)
{
	if(STOREDATA == operateType)
	{
		if((commandBuffer[U8_A_U8(APDU_DATA_OFFSET, 1)] > 0x1E) || (commandBuffer[U8_A_U8(APDU_DATA_OFFSET, 2)] != 0x3B))
		{
			return false;
		}
		//nvmCopyAtomic(OS_SECOND_ATR, &commandBuffer[APDU_DATA_OFFSET], commandBuffer[APDU_P3_OFFSET]);
		memCopyAtomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(OS_SECOND_ATR)), &commandBuffer[APDU_DATA_OFFSET], commandBuffer[APDU_P3_OFFSET]);
	}
	else 
	{
		//ecpMem_Copy(&commandBuffer[APDU_DATA_OFFSET], OS_SECOND_ATR, commandBuffer[APDU_P3_OFFSET]);
		memCopyToRam(&commandBuffer[APDU_DATA_OFFSET], MEMtPN8(JcvmConfig_GetVmConfigAddr(OS_SECOND_ATR)), commandBuffer[APDU_P3_OFFSET]);
		apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, commandBuffer[APDU_P3_OFFSET]);
	}
	return true;
}
#endif

#endif


