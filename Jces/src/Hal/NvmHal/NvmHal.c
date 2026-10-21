#include "NvmHal.h"
#include "chipHead.h"
#include "FunImp.h"

#include "mhcard_flash.h"

#if HAL__MEMORY__OPERATION


#define HEFLASH_NOERROR         0x00000000
#define HEFLASH_FAIL            0xFFFFFFFF
#define HEFLASH_FAILALREADY     0xFFBBAADD
#define HEFLASH_CONDITIONERR    0xFFFF55AA

void Reset_Handler(void);

void ecpNvm_FlashBackUp_RollBackPage()
{
//	u32 result;
//	
//	result = 0;
//	
//	//在所有的操作flash之前调用openWrSwitch3();
//	openWrSwitch3();
//	
//	//芯片自带的页保护回滚操作(存在操作Flash操作)
//	result = HED_HEFlash_Init();
//	
//	if(result != 0x00)
//	{
//		//Maybe it means HEFlash become invalid . Restart IC agagin will be best method.
//		MACRO_Init_St();
//		Reset_Handler(); 
//	}
//	
//	//在调用库之后关闭closeWrSwitch234，再开openWrSwitch2()
//	//实际是为下一条指令或下一次操作flash准备
//	closeWrSwitch234();
//	openWrSwitch2();
}

void erasePageWithCheck(PN8 eew_Des)
{
	u8 isFinish;
	u32 result;
	
	if ((PNtMEM(eew_Des) & HAL_EEPAGE_MASK) != 0)
	{
		while(1);
	}
	
	FCU->RO = 0x00000000;
	FCU->RO0_512B = 0x00000000;
	FCU->RO1_512B = 0x00000000;
	FLASH_ProtectConfig(DISABLE);
	
	isFinish = 0;
	do
	{
		
		result = FLASH_ErasePage(PNtMEM(eew_Des), FLASH_ERASE_MODE_PAGE_SMART);

		if(CHECK_WRITE_NOK == ecpNvm_CheckFill(eew_Des, 0xFF, HAL_EEPAGE_SIZE))
		{
			isFinish++;
		}
		
		if(result == FLASH_COMPLETE)
		{
			return ;
		}
		else if(result == FLASH_BUSY)
		{
			isFinish++;
		}
		else
		{
			//进入此条件存在以下两种情况
			//1. result == HEFLASH_FAILALREADY(HEFLASH_FAIL之后继续执行) 或 HEFLASH_CONDITIONERR(地址错误)
			//2. 出现不认识的返回值
			while(1);
		}
		
		if(isFinish == 0x02)
		{
			while(1);
		}
	}while(1);
}
//STATUS HARDWEAR_E2P_Fill(PN8 desAddr, UINT8 value, UINT32 Size)
//{
//	return CHECK_WRITE_OK;
//}
//u8 HARDWEAR_E2P_Write(PN8 desAddr, PN8 srcAddr, UINT32 Size)
//{
//	return	CHECK_WRITE_OK;
//}

u8 ProgPartWithCheck(PN8 ram_Src, PN8 eew_Des, u16 eew_length)
{
	u32 res;
	
	//aligne 4bytes
    if (((PNtMEM(eew_Des) & 3) != 0) || ((eew_length & 3) != 0))
	{
		while(1);
	}
	
	FCU->RO = 0x00000000;
    FCU->RO0_512B = 0x00000000;
    FCU->RO1_512B = 0x00000000;
    FLASH_ProtectConfig(DISABLE);

	res = FLASH_ProgramBuf((u32)eew_Des, ram_Src, eew_length, FLASH_PROG_MODE_SMART2);
	if (res != FLASH_COMPLETE)
	{
		return CHECK_WRITE_NOK;
	}
	
	return ecpNvm_CheckWrite(MEMtPN8(eew_Des), MEMtPN8(ram_Src), eew_length);
}


u8 ProgPageWithCheck(PN8 ram_Src, PN8 eew_Des)
{
	u32 res;
	
	if ((PNtMEM(eew_Des) & HAL_EEPAGE_MASK) != 0)
	{
		while(1);
	}
	
    FCU->RO = 0x00000000;
    FCU->RO0_512B = 0x00000000;
    FCU->RO1_512B = 0x00000000;
	FLASH_ProtectConfig(DISABLE);

	res = FLASH_ProgramBuf((u32)eew_Des, ram_Src, HAL_EEPAGE_SIZE, FLASH_PROG_MODE_SMART2);
	if (res != FLASH_COMPLETE)
	{
		return CHECK_WRITE_NOK;
	}
	
	return ecpNvm_CheckWrite(MEMtPN8(eew_Des), MEMtPN8(ram_Src), HAL_EEPAGE_SIZE);
}
void ecpNvm_WriteNoCache(PN8 eew_Des, PM8 eew_Src, u16 eew_length)
{	
	PN8			align_des;
	u16			offset;
	u16			length;
	u8			tempResult;
	u8   		ramBuffer[HAL_EEPAGE_SIZE];
	PM8			Src;	
	//PN8			Des;
	memref X_A tempAddr;
	u16 X_A tempLength;
	
	//Des					= eew_Des; //not used
	Src					= eew_Src;
	length			= eew_length;
	
	if(!isInE2P(eew_Des)&&!isInOTP(eew_Des))
	{
		if(0 == sysConfig_GetEePatchSize())
		{
			while(1);
		}
	}

	align_des 			= MEMtPN8(U32_AND_U32(PNtMEM(eew_Des), NOR_HAL_EEPAGE_MASK));
	ecpMem_Copy(ramBuffer, align_des, HAL_EEPAGE_SIZE);
	

	offset				= U16_AND_U16(PMtMEM(eew_Des), HAL_EEPAGE_MASK);
	ecpMem_Copy(PX8_A_U16(MEMtPX8(ramBuffer), offset), Src, length);
	
		tempAddr    = (u32)eew_Des;
	tempLength  = eew_length;
	tempLength  = U8_A_U16(U32t8(U32_AND_U32(tempAddr, 0x0F)), tempLength);
	tempLength  = U16_AND_U16(U8_A_U16(0x0F, tempLength), 0xFFFFFFF0);
	eew_Des	= (u8*)U32_AND_U32(tempAddr, 0xFFFFFFF0);
	eew_length= tempLength;
	
	offset = eew_Des -  align_des;


	tempResult = ProgPartWithCheck(PX8_A_U16(MEMtPX8(ramBuffer), offset), eew_Des, eew_length);
	
	while(CHECK_WRITE_NOK == tempResult)
	{
		erasePageWithCheck(align_des);
		tempResult = ProgPageWithCheck(ramBuffer, align_des);
	}
	
	return;
}

void ecpNvm_WriteCache(PN8 eew_Des, PM8 eew_Src, u16 eew_length)
{
	u8   		X_A   	ramBuffer[HAL_EEPAGE_SIZE];
	PN8			align_des;
	u16			ramBufferoffset;
	u8			result;
#if SUPPORT_COS_UPDATE
	if(!COS_UPDATE_COSLoading)		//该变量表示cos升级调用该擦写接口，不做判断
#endif
	{
		if(!isInE2P(eew_Des)&&!isInOTP(eew_Des))
		{
			if(0 == sysConfig_GetEePatchSize())
			{
				while(1);
			}
		}
	}

	align_des 			= MEMtPN8(U32_AND_U32(PNtMEM(eew_Des), NOR_HAL_EEPAGE_MASK));
	ramBufferoffset				= U16_AND_U16(PMtMEM(eew_Des), HAL_EEPAGE_MASK);
	
	ecpMem_Copy(ramBuffer, align_des, HAL_EEPAGE_SIZE);
	ecpMem_Copy(PX8_A_U16(MEMtPX8(ramBuffer), ramBufferoffset), eew_Src, eew_length);
		
	do
	{
		erasePageWithCheck(align_des);
		result = ProgPageWithCheck(ramBuffer, align_des);						
	}while(CHECK_WRITE_NOK == result);	

	return;
}
void ecpNvm_WriteCache_NoCheck(PN8 eew_Des, PM8 eew_Src, u16 eew_length)
{
	u8   		X_A   		ramBuffer[HAL_EEPAGE_SIZE];
	u8			result;

	PATCH_HAL_NVMHAL_003();
	
	if(!isInE2P(eew_Des)&&!isInOTP(eew_Des))
	{
		if(0 == sysConfig_GetEePatchSize())
		{
			while(1);
		}
	}

	ecpMem_Copy(ramBuffer, eew_Src, HAL_EEPAGE_SIZE);
	
	do
	{
		erasePageWithCheck(eew_Des);
		result = ProgPageWithCheck(ramBuffer, eew_Des);
	}while(CHECK_WRITE_NOK == result);	

	return;
}

void ecpNvm_WriteErase(PN8 eew_Des, PM8 eew_Src, u16 eew_length)
{
	ecpNvm_WriteCache(eew_Des, eew_Src, eew_length);
}

void ecpNvm_ErasePage(PN8 eew_Des)
{
	if(!isInE2P(eew_Des)&&!isInOTP(eew_Des))
	{
		while(1);
	}
	
	erasePageWithCheck(MEMtPN8(U32_AND_U32(PNtMEM(eew_Des), NOR_HAL_EEPAGE_MASK)));
	return;
}

void ecpNvm_EraseBlock(PN8 eew_Des)
{
	u8 			i;
	if(!isInE2P(eew_Des)&&!isInOTP(eew_Des))
	{
		while(1);
	}
	
	for(i = 0; i < ERASEBLOCKSIZE; i++)
	{
		ecpNvm_ErasePage((PN8)(U32_A_U16(eew_Des, U8_M_U16(i, HAL_EEPAGE_SIZE))));
	}

	return;
}

void ecpNvm_WriteOnly(PN8 eew_Des, PM8 eew_Src, u16 eew_length)
{
	if(!isInE2P(eew_Des)&&!isInOTP(eew_Des))
	{
		while(1);
	}
	ecpNvm_WriteNoCache(eew_Des, eew_Src, eew_length);
	return;
}

void ecpNvm_WriteCommit(void)
{
	return;
}

u8 ecpNvm_CheckWrite(PN8 eew_Des, PM8 eew_Src, u16 checkLength)
{	
	if (0 != ecpMem_Compare(eew_Des, eew_Src, checkLength))
	{
		return	CHECK_WRITE_NOK;
	}
	
	return CHECK_WRITE_OK;
}

u8 ecpNvm_CheckFill(PN8 eew_Des, u8 value, u16 checkLength)
{
	u8			result;
    
	result  = CHECK_WRITE_OK;
	if (checkLength)
    {
        do
        {
            checkLength--;
            if(PN_R_U8(eew_Des) != value)
			{
				result  = CHECK_WRITE_NOK;
				break;
			}
			PN8_INC(eew_Des);
       }while (checkLength);
    }
    return result;
}

u8	 ecpNvm_IsInProgress(void)
{
	return 0;
}

void ecpNvm_WaitForProgress(void)
{
	return;
}

#endif
