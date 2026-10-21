#include "NvmHal.h"
#include "chipHead.h"
#include "Sysconfig.h"

#if HAL__MEMORY__OPERATION


void erasePageWithCheck(PN8 eew_Des)
{	
	u8								isFinish;
	u8								result;
	if(!isInE2P(eew_Des))
	{
		while(1);
	}
	isFinish = 0;
	do
	{
		ecpMem_Set(MEMtPN8(eew_Des), 0xFF, HAL_EEPAGE_SIZE);
		result						= ecpNvm_CheckFill(eew_Des, 0xFF, HAL_EEPAGE_SIZE);
		if(CHECK_WRITE_OK == result)
		{
			return;
		}
		else if(CHECK_WRITE_NOK == result)
		{
			isFinish ++;
		}
		else
		{
			while(1);
		}
		if(isFinish == 0x02)
		{
			while(1);
		}
	}while(1);
}

u8 ProgPartWithCheck(PN8 ram_Src, PN8 eew_Des, u16 eew_length)
{
	if(!isInE2P(eew_Des))
	{
		while(1);
	}
	ecpMem_Copy(MEMtPN8(eew_Des), MEMtPN8(ram_Src), eew_length);
	return ecpNvm_CheckWrite(MEMtPN8(eew_Des), MEMtPN8(ram_Src), eew_length);
}
u8 ProgPageWithCheck(PN8 ram_Src, PN8 eew_Des)
{	
	if(!isInE2P(eew_Des))
	{
		while(1);
	}
	ecpMem_Copy(MEMtPN8(eew_Des), MEMtPN8(ram_Src), HAL_EEPAGE_SIZE);
	return ecpNvm_CheckWrite(MEMtPN8(eew_Des), MEMtPN8(ram_Src), HAL_EEPAGE_SIZE);
}
void ecpNvm_WriteNoCache(PN8 eew_Des, PM8 eew_Src, u16 eew_length)
{	
	PN8			N_A		Des;
	PN8			N_A		align_des;
	PM8			M_A		Src;
	u16			X_A		length;
	u8			X_A		offset;
	u8			X_A		tempResult;
	u8   		X_A   	ramBuffer[HAL_EEPAGE_SIZE];
	Des					= eew_Des;
	Src					= eew_Src;
	length				= eew_length;

	align_des 			= MEMtPN8(U32_AND_U32(PNtMEM(eew_Des), NOR_HAL_EEPAGE_MASK));
	ecpMem_Copy(ramBuffer, align_des, HAL_EEPAGE_SIZE);
	
	offset				= U8_AND_U8(PMtMEM(eew_Des), HAL_EEPAGE_MASK);
	ecpMem_Copy(PX8_A_U8(MEMtPX8(ramBuffer), offset), Src, length);
	
//	offset				= U8_AND_U8(PMtMEM(eew_Des), NOR_HAL_BLOCK_MASK);
//	length				+= U16_AND_U16(PMtMEM(eew_Des), HAL_BLOCK_MASK);
//	length				= U16_AND_U16(U16_A_U16(length, HAL_BLOCK_MASK), NOR_HAL_BLOCK_MASK);
//	Des					= MEMtPN8(U32_AND_U32(PNtMEM(eew_Des), NOR_HAL_BLOCK_MASK));
	
	tempResult = ProgPartWithCheck(PX8_A_U8(MEMtPX8(ramBuffer), offset), Des, length);

	while(CHECK_WRITE_NOK == tempResult)
	{
		erasePageWithCheck(align_des);
		tempResult = ProgPageWithCheck(ramBuffer, align_des);
	}
}

void ecpNvm_WriteCache(PN8 eew_Des, PM8 eew_Src, u16 eew_length)
{
	PN8			N_A 	align_des;
	PM8			M_A		Src;
	u16			X_A		length;
	u8			X_A		offset;
	u8			X_A		tempResult;
	u8   		X_A   	ramBuffer[HAL_EEPAGE_SIZE];
	
	Src					= eew_Src;
	length				= eew_length;

	align_des			= MEMtPN8(U32_AND_U32(PNtMEM(eew_Des), NOR_HAL_EEPAGE_MASK));
	offset				= U8_AND_U8(PMtMEM(eew_Des), HAL_EEPAGE_MASK);
	
	ecpMem_Copy(ramBuffer, align_des, HAL_EEPAGE_SIZE);
	ecpMem_Copy(PX8_A_U8(MEMtPX8(ramBuffer), offset), Src, length);
	
	do
	{
		erasePageWithCheck(align_des);
		tempResult 		= ProgPageWithCheck(ramBuffer, align_des);
	}while(CHECK_WRITE_NOK == tempResult);	

	return;
}
void ecpNvm_WriteCache_NoCheck(PN8 eew_Des, PM8 eew_Src, u16 eew_length)
{
	u8			X_A		tempResult;
	u8   		X_A   	ramBuffer[HAL_EEPAGE_SIZE];
	
	PATCH_HAL_NVMHAL_002();
	ecpMem_Copy(ramBuffer, eew_Src, HAL_EEPAGE_SIZE);
	
	do
	{
		erasePageWithCheck(eew_Des);
		tempResult 		= ProgPageWithCheck(ramBuffer, eew_Des);
	}while(CHECK_WRITE_NOK == tempResult);	

	return;
}

void ecpNvm_WriteErase(PN8 eew_Des, PM8 eew_Src, u16 eew_length)
{
	ecpNvm_WriteNoCache(eew_Des, eew_Src, eew_length);
}

void ecpNvm_ErasePage(PN8 eew_Des)
{
	erasePageWithCheck(MEMtPN8(U32_AND_U32(PNtMEM(eew_Des), NOR_HAL_EEPAGE_MASK)));
	return;
}

void ecpNvm_EraseBlock(PN8 eew_Des)
{
	u8			X_A		i;
	PN8			N_A 	align_des;
	align_des			= MEMtPN8(U32_AND_U32(PNtMEM(eew_Des), NOR_HAL_EEPAGE_MASK));
	for(i = 0; i < ERASEBLOCKSIZE;)
	{
		ecpNvm_ErasePage(align_des);
		i 				+= HAL_EEPAGE_SIZE;
		align_des 		+= HAL_EEPAGE_SIZE;
	}

	return;
}

void ecpNvm_WriteOnly(PN8 eew_Des, PM8 eew_Src, u16 eew_length)
{
	memmove(memoryToPoint((memref)(eew_Des)), memoryToPoint((memref)(eew_Src)), eew_length);
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
