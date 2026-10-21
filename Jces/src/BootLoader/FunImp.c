#include "FunImp.h"
#include "profile.h"
#include "Crypto_Constant.h"
#include "SecureCrypto.h"
#include "MemOperation.h"
#include "bootPlaformConfigProfile.h"
#include "NvmHal.h"
#include "BootLifeCycle.h"
#include "uicc.h"
#if (BOOTLOADER_PRO&&COS_UPDATE_ONLINE_BY_NativeOS)
#include "UpCos.h"
#endif
#if SUPPORT_COS_UPDATE
u8     COS_UPDATE_COSLoading;
#endif
#if BOOTLOADER
u16 														crcValue;
u8  														downloaderSuccess;
//u8															ConvertBlSeuccess;
#endif
#if BOOTLOADER||COS_UPDATE_ONLINE_BY_NativeOS
void initBootLoaderVar(void)
{
#if BOOTLOADER
	crcValue					= 0;
	downloaderSuccess	=	0;
//	ConvertBlSeuccess = 0;
	COS_UPDATE_COSLoading = FALSE;
#endif
	
#if (BOOTLOADER_PRO&&COS_UPDATE_ONLINE_BY_NativeOS)
	InitUpCOSVar();
#endif

}
#endif
#if BOOTLOADER||COS_UPDATE_ONLINE_BY_NativeOS
u16 BootLoader_Get_crc16(PM8 pu1CRC, u32 u1CRCLength, u16 ValCrc)
{
	u16 crc;
	u8 i;
	crc = ValCrc;
	while (u1CRCLength != 0)
	{
		U32_DEC(u1CRCLength);
		//			crc = crc ^ (u16) *pu1CRC++ << 8;
		crc = U16_XOR_U16(crc, PM_R_U8(pu1CRC) << 8);
		PM8_INC(pu1CRC);
		i = 8;
		do
		{
			if (crc & 0x8000)
			{
				crc = U16_XOR_U16(crc << 1, 0x1021);
			}
			else
			{
				crc = crc << 1;
			}
		} while (--i);
	}
	return (crc);
}
#endif
#if (BOOTLOADER||COS_UPDATE_ONLINE_BY_NativeOS)&&(!BOOTLOADER_PRO)  
u16 switchToEcpLoader(u8 mode)
{
	PN32			D_A			addrCheck;
	PN32			D_A			addr;
	u32				D_A			osEntry;	
	u32       D_A     mfAddr,privateFileAddr;
	
	if((g_securityState == SECURITY_STATE_ATTACK)&&(mode == 1))
	{
		return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	//涉及到文件系统的变量，需要有UICC宏管控，该宏的选择还需要确定下 魏平姣@20221207
#if !COS_UPDATE_ONLINE_MUTUAL && JAVA_CARD_UICC//  BOOTLOADER +COS_UPDATE_ONLINE_MUTUAL时 不需要用配置区的文件系统地址（并且该情况不存在这个配置区） COS_UPDATE_ONLINE_BY_NativeOS
	//JcvmConfig_SetFileSystemStart(GetMFFileStart());
	
	if(BootLifeCycle_GetLifeCycle() <= LIFE_CYCLE_INIT)//兼容未做个人化 直接下载BOOTLOADER COS 
	{
		 mfAddr = 0;
		privateFileAddr = 0;
	}
	else
	{
		mfAddr = GetMFFileStart();
		privateFileAddr = ADDR_PRIVATE_FILE_START;
	}
	
	UPCOSConfig_SetUPCOSFileSystemAddr(mfAddr);
	UPCOSConfig_SetUPCOSPrivateFileSystemAddr(privateFileAddr);
#endif	
	addr = (PN32)BOOT_CONFIG_START;

	addrCheck = (PN32)((u8 *)addr + sizeof(FunctionReadEntry));
	osEntry = sizeof(FunctionReadEntry) >> 2;
	for(; osEntry != 0 ; osEntry--)
	{
		if((*addr == 0)||(*addr == 0xFFFFFFFF)|| (*addr != *addrCheck))
		{
			return BOOT_CONDITIONS_NOT_SATISFIED;
		}
		addr ++;
		addrCheck ++;
	}
	
	addr = (PN32)&FuncEntry;
	
	COS_UPDATE_COSLoading = TRUE;//该变量表示COS升级正在写CODE区，升级完以后置为FALSE 不允许更改CODE区

	fillEeprom(MEMtPN8((memref)addr), 0x00 , EEPAGE_SIZE);
	
	COS_UPDATE_COSLoading = FALSE;//该变量表示COS升级正在写CODE区，升级完以后置为FALSE 不允许更改CODE区

//	ConvertBlSeuccess = 1;
	return BOOT_NO_ERROR;
}
#endif

#if BOOTLOADER
u16 WriteAPP_EEPROM(PX8 apduBuffer)
{
	u16			D_A			offset;
	PN8			D_A			addr;
	PN32			D_A			addr_1;
	PN32			D_A			addr_2;
	u32				D_A			osEntry;
//	u8    		D_A   		calcarry[0x100];
	u8			D_A  		des2key[0x10];
	u8  		D_A 		ICV[8];
	PM8         D_A         ptr;
	u16         D_A         dataLen;
	//u8			X_A			tempResult;  //remove warning,dengyiwen,2022/9/21

	if(g_securityState == SECURITY_STATE_ATTACK)
	{
		return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	
//	if(1 != ConvertBlSeuccess)
//	{
//		return BOOT_CONDITIONS_NOT_SATISFIED;
//	}
	
	if( (PNtMEM(SetBank0Base) & 0xFF000000) != JC_EEPROM_BASE )
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}
	
	if(0xFF != apduBuffer[APDU_P3_OFFSET])
	{
		return BOOT_WRONG_LENGTH; //未补齐一页
	}
	
	offset	= U16_SHL_U8(apduBuffer[APDU_P1_OFFSET], 8);
	addr	= PN8_A_U16(SetBank0Base, offset);
	
	apdu_ApduSetIncomingAndReceive(); 

	apduBuffer[APDU_P3_OFFSET] = apduBuffer[APDU_P2_OFFSET];
	dataLen = 0x100;
	
#if (0x200 == HAL_EEPAGE_SIZE)
	ptr = MEMtPX8(&stream_buffer[(u32)addr&0x100]);
#else
	ptr = MEMtPX8(&stream_buffer[0]);
#endif
	
	if(*(MEMtPM8(DOWNLOAD_EN_MODE)) == 0x01)
	{
		memset(ptr, 0x00, 0x100);
		memcpy(MEMtPM8(des2key), MEMtPM8(DOWNLOAD_KEY_BASE), DownLoad_ENKEY_LEN);
		Triple_DES(PX8_A_CONST(apduBuffer, APDU_P3_OFFSET), des2key, ptr, dataLen, ICV, ECB_Decrypt, SENSITIVE_DATA_NOT_MASK);
	}
	else
	{
		memcpy(ptr, PX8_A_CONST(apduBuffer, APDU_P3_OFFSET), dataLen);
	}

	if((((u32)addr & 0xFFFFFF00) != CHIP_CONFIG_START) && (((u32)addr & 0xFFFFFF00) != BOOT_CONFIG_START) )
	{
		crcValue = BootLoader_Get_crc16(ptr, dataLen, crcValue);
	}
#if (BOOTLOADER_PRO||UPCOS_COSB)
	if(!(((CHIP_CONFIG_START <= ((u32)addr))&&((u32)addr<BOOT_CONFIG_START)) 		
			|| ((COSA_DATA_START <= (u32)addr) && ((u32)addr < CHIP_CONFIG_START))))
	{
		return BOOT_DATA_INVALID;
	}
#else
	if(!(((BOOT_CONFIG_START <= ((u32)addr))&&((u32)addr<BOOTLOADER_CONFIG_START)) 
			|| ((BOOT_LOADER_START <= (u32)addr) && ((u32)addr < BOOTLOADER_END))))
	{
		return BOOT_DATA_INVALID;
	}
#endif

	//if((((u32)addr & 0xFFFFFF00) == BOOT_CONFIG_START)||(((u32)addr & 0xFFFFFF00) == CHIP_CONFIG_START))
	if(((CHIP_CONFIG_START <= ((u32)addr))&&((u32)addr<BOOT_CONFIG_START)) || ((BOOT_CONFIG_START <= ((u32)addr))&&((u32)addr<BOOTLOADER_CONFIG_START)))
	{
		if (downloaderSuccess != 1)
		{
			 return BOOT_CONDITIONS_NOT_SATISFIED;
		}
		if((((u32)addr & 0xFFFFFF00) == BOOT_CONFIG_START)||(((u32)addr & 0xFFFFFF00) == CHIP_CONFIG_START))
		{
			addr_1 = (PN32)((u8 *)ptr);
			addr_2 = (PN32)((u8 *)ptr + sizeof(FunctionReadEntry));
			osEntry = sizeof(FunctionReadEntry) >> 2;
			for(; osEntry != 0 ; osEntry--)
			{
				if(((u32)*addr_1 == 0)||((u32)*addr_1 == 0xFFFFFFFF)|| ((u32)*addr_1 != (u32)*addr_2))
				{
					return BOOT_CONDITIONS_NOT_SATISFIED;
				}
				addr_1 ++;
				addr_2 ++;
			}
		}
	}
	
	COS_UPDATE_COSLoading = TRUE;//该变量表示COS升级正在写CODE区，升级完以后置为FALSE 不允许更改CODE区
	
#if (0x200 == HAL_EEPAGE_SIZE)	
	if((u32)addr & 0x100)
#endif
	{		
		do
		{
			addr= (PN8)((u32)addr&NOR_HAL_EEPAGE_MASK);			//由于上面的限定条件，addr为一页的后半页起始地址才开启擦写，一次擦写一整页，但此时传入擦写接口的addr必须为页起始地址，否则会产生跨页
			ecpNvm_WriteErase( (PN8)((u32)addr),stream_buffer,HAL_EEPAGE_SIZE);
		}while(CHECK_WRITE_NOK == ecpNvm_CheckWrite(addr, stream_buffer, HAL_EEPAGE_SIZE));	
	}

	COS_UPDATE_COSLoading = FALSE;//该变量表示COS升级正在写CODE区，升级完以后置为FALSE 不允许更改CODE区
	
	return BOOT_NO_ERROR;
}

u16 CheckCrcValue(PX8 apduBuffer)
{
	if(g_securityState == SECURITY_STATE_ATTACK)
	{
		return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	
	apdu_ApduSetIncomingAndReceive();
	
	if (crcValue != U16_OR_U16(U16_SHL_U8(apduBuffer[APDU_DATA_OFFSET], 8), apduBuffer[APDU_DATA_OFFSET + 1]))
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}
	
	crcValue = 0;
	downloaderSuccess = 1;

	return BOOT_NO_ERROR;
}

u16 LoaderBlockSelect(PX8 apduBuffer)
{
	u32 baseAddr;

	if(g_securityState == SECURITY_STATE_ATTACK)
	{
		return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}

	baseAddr = 0;
	baseAddr = (apduBuffer[APDU_P1_OFFSET] << 24 | apduBuffer[APDU_P2_OFFSET] << 16);
	
	if (apduBuffer[APDU_P3_OFFSET] != 0)
	{
		return BOOT_WRONG_LENGTH;
	}

	SetBank0Base = MEMtPN8(baseAddr);

	return BOOT_NO_ERROR;
}
u16 GetOsInfo(PX8 apduBuffer)
{
	ecpMem_Copy(MEMtPX8(&apduBuffer[APDU_DATA_OFFSET]), &bootVersion[0], BOOT_VERSION_SIZE);
	
  apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, BOOT_VERSION_SIZE);
	
	return BOOT_NO_ERROR;
}

u16 LoaderBootloaderKey(PX8 apduBuffer)
{
	u8		lc;
	
	if(g_securityState == SECURITY_STATE_ATTACK)
	{
		return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	
	apdu_ApduSetIncomingAndReceive(); 
	
	lc	= apduBuffer[APDU_P3_OFFSET];

	// p3 长度为0x10
	if (DownLoad_ENKEY_LEN != lc)
	{
		return BOOT_MFKEY_WRONG_LENGTH;
	}
	
	ecpNvm_WriteErase(MEMtPM8(DOWNLOAD_KEY_BASE), &apduBuffer[APDU_DATA_OFFSET], DownLoad_ENKEY_LEN);
	return BOOT_NO_ERROR;
}
#endif
#if COS_UPDATE_ONLINE_BY_NativeOS
void IF_SetUPCOSFileSystemAddr(void)
{
		UPCOSConfig_SetUPCOSFileSystemAddr(0x0000);
}

#endif

__attribute__((section("BL_FUN_CPY")))
void BootLoader_Cpy(PX8 Des, PM8 src, u16 Length)
{
		while(Length)
		{
			PX_W_U8(Des , PM_R_U8(src));
			PX8_INC(Des);
			PM8_INC(src);
			U16_DEC(Length);
		}
}

