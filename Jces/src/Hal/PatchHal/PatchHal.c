#include "PatchHal.h"
#include "SysConfigData.h"
#include "SysConfigProfile.h"
#include "SysConfigHalRam.h"
#include "chipHead.h"
#include "stack.h"
#include "error.h"
#include "ecpDefine.h"
#include "nativemethod_impl.h"
#include "ob_manager.h"
#include "object.h"
#include "NvmHal.h"

#if HAL__PATCH
#if GW_PRIVATE_GM_CRYPTO
__attribute__((section("SM1_ENTRY")))
void Alg_SM1_Process_JumpArea(unsigned char enc_sel, unsigned char mode, int round_sel, unsigned char sk_sel, unsigned char *SK, unsigned char *MK, unsigned char *IV, unsigned int len, unsigned char *input, unsigned char *output)
{
		Alg_SM1_Process(enc_sel, mode, round_sel, sk_sel, SK, MK, IV, len, input, output);
}
__attribute__((section("SM4_ENTRY")))
void Alg_SM4_Process_JumpArea(unsigned char mode,unsigned char sel,unsigned char *iv,unsigned char *mi,unsigned char *mo,unsigned char *key,unsigned int len)
{
		Alg_SM4_Process(mode, sel, iv, mi, mo, key, len);
}
__attribute__((section("SM7_ENTRY")))
void Alg_SM7_Process_JumpArea(unsigned int mode,unsigned int sel,unsigned char *iv,unsigned char *mi,unsigned char *mo,unsigned char *key,unsigned int len)
{
		Alg_SM7_Process(mode, sel, iv, mi, mo, key, len);
}
__attribute__((section("VERSION_ENTRY")))
unsigned char Alg_GetVersion_JumpArea(unsigned char* versionTLV)
{
	return Alg_GetVersion(versionTLV);
}

__attribute__((section("patchfunctionlist")))
FunctionEntry CONST FunctionList={
				Alg_SM7_Process_JumpArea,
				Alg_SM4_Process_JumpArea,
				Alg_SM1_Process_JumpArea,	
				Alg_GetVersion_JumpArea,
};

#endif

#pragma SYS_PATCH_RAM_START

SYS_PATCH_RAM_END

void ecpPatch_initBeforAts(void)
{

}

void ecpPatch_initAfterAts(void)
{

}

void ecpPatch_BackupOperation(PX8 tempData1,PX8 tempData2)
{
}

void ecpPatch_RecoveryOperation(u16 tempData1,u16 tempData2)
{
}

PM8 FlagHandle(u16 flag)  	 //该函数实现对补丁号的查找
{
#if HAL_PATCH_NATIVE
	PM8 		patch_addr;
	PM8 		PC_pointer;
	u8 			i;
	
	PC_pointer 	= NULL;
	patch_addr 	= MEMtPM8(sysConfig_GetDefaultPatchTableAddrBase());
	for(i = 0; i < DEFAULT_PATCH_TABLE_NUM; i++)
	{
		if(PC_R_U16(patch_addr) == flag)
		{
	       	PC_pointer = MEMtPM8(PC_R_U32(PM8_A_U8 (patch_addr , 2)));
			PC_pointer = MEMtPM8((memref)PC_pointer | 0x01);
			return PC_pointer;
		}
		patch_addr += 6;	
	}

	if(EXTERNAL_PATCH_TABLE_EXIST == sysConfig_GetexternalPatchTableFlg())
	{
		patch_addr = (PM8)sysConfig_GetexternalPatchTableAddr();
		for(i = 0; i < sysConfig_GetexternalPatchTableNum(); i++)
		{
			if(PC_R_U16(patch_addr) == flag)
			{
				PC_pointer = MEMtPM8(PC_R_U32(PM8_A_U8 (patch_addr , 2)));
				return PC_pointer;
			}
			patch_addr += 6;
		}	
	}
	
 	if(PC_pointer == NULL)
 	{
		
	}
	return PC_pointer;
#endif //HAL_PATCH_NATIVE
}

void nativeMethod_sepatch_init(void)
{
	u16		D_A		bOff;
	u16	  patchLength;
	u32 		X_A 		totalPatchSize;
	u32						src;
	u8			D_A			TransactionStarted;	
	
	bOff		= popShort();

	patchLength = bOff;
	
	TransactionStarted = 0;
	
  // patchStartAddress 8 byte align
	patchLength				= U16_A_U8(patchLength, 7);
	patchLength				= U16_AND_U16(patchLength, 0xFFF8);
	totalPatchSize			= sysConfig_GetEePatchSize();
	
	if ( totalPatchSize + patchLength > MAX_PATCH_EEP_SIZE)
	{
		throw_error(PATCH_MEMORY_NOT_ENOUGH);
	}
	if (!isInProgress())
	{
		beginTransaction(SINGLE_APDU_PROCESS);
		TransactionStarted = 1;
	}
	//src								= 	(u32)(&(Image$$ER_PATCH_START$$Base)) + memReadU32(MEMtPM8(EE_PATCH_SIZE));
	src								= 	(u32)(&(Image$$ER_PATCH_START$$Base)) + totalPatchSize;
	if(0x00 != patchLength)
	{		
		//totalPatchSize 			= U32_A_U16(memReadU32(MEMtPM8(EE_PATCH_SIZE)), patchLength);
		totalPatchSize 			= U32_A_U16(totalPatchSize, patchLength);
	}
	else	
	{		
		//totalPatchSize 			= memReadU32(MEMtPM8(EE_PATCH_SIZE));
		//totalPatchSize 			= 0;
		throw_error(PATCH_WRONG_LENGTH);
	}

	//memWriteU32Atomic(MEMtPM8(EE_PATCH_SIZE),totalPatchSize);
	sysConfig_SetEePatchSize(totalPatchSize);

	if (TransactionStarted)
	{
		commitTransaction();
	}

	pushInt(src);
}

void nativeMethod_sepatch_download(void)
{
	u16		D_A		bOff;
	PM8  D_A bArray;	
	u32		D_A patchAddr;
	u8    D_A   calcarry[0x100];
	u8	D_A  des2key[0x10];
	u8  D_A ICV[8];// = {0};
	u8			D_A			TransactionStarted;	
	
	bOff		= popShort();
	
	bArray		= (PM8)MEMtPM8( locateObjectInMemory( popReference() ) );	
	
	TransactionStarted = 0;
	
	if(MEMREF_NULL != (memref)(bArray))
	{
		bArray			= MEMtPM8(get_array_data(bArray));
	
		if (bArray[APDU_P3_OFFSET] < 0x04)
		{
			throw_error(PATCH_MEMORY_NOT_ENOUGH);
		}
		
		if(((bOff -4)%0x08)!=0x00)
		{
			throw_error(PATCH_DATA_ERROR);			
		}
	
		memcpy(des2key, MEMtPM8(sysConfig_GetPatchEnkeyBase()), 0x10);
			
		patchAddr = ((bArray[OFFSET_ADDR_PATCH] << 24) | (bArray[OFFSET_ADDR_PATCH+1] << 16) | (bArray[OFFSET_ADDR_PATCH+2] << 8) | (bArray[OFFSET_ADDR_PATCH+3]));

		Triple_DES(&bArray[OFFSET_DATA_PATCH], des2key, calcarry, bOff-4, ICV, ECB_Decrypt,SENSITIVE_DATA_NOT_MASK);
		
		if (!isInProgress())
		{
			beginTransaction(SINGLE_APDU_PROCESS);
			
			TransactionStarted = 1;
		}		

//	nvmCopyNoAtomic(MEMtPN8(patchAddr), MEMtPM8(&calcarry[0]), bOff-4);
//	MEM_write_array_NonAtomic(patchAddr, (memref)calcarry, bOff-4);
		memCopyAtomic(MEMtPM8(patchAddr), MEMtPM8((memref)calcarry), bOff-4);
		if (TransactionStarted)
		{
			commitTransaction();
		}	   
	}
	else
	{
		throw_error(NULL_POINTER_EXCEPTION);
	}
}


void nativeMethod_sepatch_InstallandRegister(void)
{
	u16		D_A		bOff, patchlen, patchcrc, calccrc;
	PM8  bArray;	
	u32		patchAddr,patchdataAddr;
	u8		D_A			TransactionStarted;
	
	bOff		= popShort();
	
	bArray		= (PM8)MEMtPM8( locateObjectInMemory( popReference() ) );	
	
	TransactionStarted = 0;
	
	
	if(MEMREF_NULL != (memref)(bArray))
	{
		bArray			= MEMtPM8(get_array_data(bArray));		
		
		if (bArray[APDU_P3_OFFSET] != 0x14)
		{
			throw_error(PATCH_MEMORY_NOT_ENOUGH);
		}
		
//	patchdataAddr = ((bArray[OFFSET_PATCH_DATA_ADDR] << 24) | (bArray[OFFSET_PATCH_DATA_ADDR + 1] << 16) | (bArray[OFFSET_PATCH_DATA_ADDR + 2] << 8) | (bArray[OFFSET_PATCH_DATA_ADDR + 3]));
		
		patchdataAddr = ((bArray[OFFSET_PATCH_DATA_ADDR])|(bArray[OFFSET_PATCH_DATA_ADDR+1]<<8)|(bArray[OFFSET_PATCH_DATA_ADDR+2]<<16)|(bArray[OFFSET_PATCH_DATA_ADDR+3])<<24);
		
		patchlen = ((bArray[OFFSET_PATCH_LENGTH ] << 8) | (bArray[OFFSET_PATCH_LENGTH + 1]));
		
		patchcrc = ((bArray[OFFSET_PATCH_CRC ] << 8) | (bArray[OFFSET_PATCH_CRC + 1]));	
		
		calccrc = Get_crc16((PN8)(patchdataAddr-1), patchlen);
		
		if(calccrc != patchcrc)
		{
			throw_error(PATCH_DATA_ERROR);			
		}
		
		patchAddr = ((bArray[OFFSET_ADDR_PATCH] << 24) | (bArray[OFFSET_ADDR_PATCH + 1] << 16) | (bArray[OFFSET_ADDR_PATCH + 2] << 8) | (bArray[OFFSET_ADDR_PATCH + 3]));

//	MEM_write_array_NonAtomic(patchAddr, (memref)(&bArray[OFFSET_DATA_PATCH]), bOff-4);	
		
#if BIG_ENDIAN
		bArray[OFFSET_PATCH_DATA_ADDR]   = patchdataAddr>>24;
		bArray[OFFSET_PATCH_DATA_ADDR+1] = patchdataAddr>>16;
		bArray[OFFSET_PATCH_DATA_ADDR+2] = patchdataAddr>> 8;
		bArray[OFFSET_PATCH_DATA_ADDR+3] = patchdataAddr&0xff;
#endif
//modify by	xieyong@20220408  修改打补丁之后执行指令失败的问题;听辉哥建议修改代码,当补丁目标函数地址未4字节对齐时,需填充2字节,以便找到正确的地址执行
		if(patchAddr%4)
		{
			if (!isInProgress())
			{
				beginTransaction(SINGLE_APDU_PROCESS);
				TransactionStarted = 1;
			}
			
			nvmWriteU16Atomic(MEMtPM8(patchAddr),0x0046);
			
			patchAddr += 0x02;
		}

		nvmCopyAtomic(MEMtPM8(patchAddr), MEMtPM8((memref)(&bArray[OFFSET_DATA_PATCH])), bOff-4);
		
		if(patchAddr%4)
		{
			if(TransactionStarted)
			{
				commitTransaction();
			}	 
		}

	}
	else
	{
		throw_error(NULL_POINTER_EXCEPTION);
	}
	
}

void nativeMethod_sepatch_updatelib(void)
{
	u16		D_A		bOff;
	u16		D_A		calccrc;
	u16		D_A		patchcrc;
	PM8  D_A bArray;	
	u32		D_A patchAddr;
	u8    D_A   calcarry[0x100];
	u8	D_A  des2key[0x10];
	u8  D_A ICV[8];// = {0};
	u8			tempResult;
	
	tempResult = CHECK_WRITE_NOK;
	bOff		= popShort();
	
	bArray		= (PM8)MEMtPM8( locateObjectInMemory( popReference() ) );	
	
	if(MEMREF_NULL != (memref)(bArray))
	{
		bArray			= MEMtPM8(get_array_data(bArray));
	
		if (bArray[APDU_P3_OFFSET] < 0x06)
		{
			throw_error(PATCH_MEMORY_NOT_ENOUGH);
		}
		
		if(((bOff -6)%0x08)!=0x00)
		{
			throw_error(PATCH_DATA_ERROR);			
		}
	
		memcpy(des2key, MEMtPM8(sysConfig_GetPatchEnkeyBase()), 0x10);
			
		patchAddr = ((bArray[OFFSET_ADDR_PATCH] << 24) | (bArray[OFFSET_ADDR_PATCH+1] << 16) | (bArray[OFFSET_ADDR_PATCH+2] << 8) | (bArray[OFFSET_ADDR_PATCH+3]));

		Triple_DES(&bArray[OFFSET_DATA_PATCH_LIB], des2key, calcarry, bOff-6, ICV, ECB_Decrypt,SENSITIVE_DATA_NOT_MASK);
		
		calccrc = Get_crc16((PN8)(calcarry), bOff-6);
		
		patchcrc = bArray[OFFSET_ADDR_PATCH+4]<<8|bArray[OFFSET_ADDR_PATCH+5];
		
		if(calccrc != patchcrc)
		{
			throw_error(PATCH_DATA_ERROR);			
		}

//		ecpNvm_WriteErase((u8*)patchAddr,(u8*)(memref)calcarry,bOff-6);		
		do
		{
			tempResult = ProgPartWithCheck((u8*)(memref)calcarry, (u8*)patchAddr, bOff-6);
		}while(CHECK_WRITE_NOK == tempResult);
	}
	else
	{
		throw_error(NULL_POINTER_EXCEPTION);
	}
	
}


void nativeMethod_decrypto(void)
{
	u16		D_A		srcbOff;
	u16		D_A		dsetbOff;
	u16		D_A		len;
	PM8 	D_A   srcbArray;
	PM8   D_A   destbArray;
	u8	D_A  des2key[0x10];
	u8  D_A ICV[8];// = {0};

	len				 = popShort();
	dsetbOff   = popShort();
	destbArray = (PM8)MEMtPM8( locateObjectInMemory( popReference() ) );
	srcbOff    = popShort();
	srcbArray  = (PM8)MEMtPM8( locateObjectInMemory( popReference() ) );
	if((len%8)!=0)
	{
		throw_error(PATCH_DATA_ERROR);
	}
	if((MEMREF_NULL != (memref)(destbArray))||(MEMREF_NULL != (memref)(srcbArray)))
	{
		destbArray			= MEMtPM8(get_array_data(destbArray));
		srcbArray			= MEMtPM8(get_array_data(srcbArray));
		ecpMem_Copy(des2key, MEMtPM8(sysConfig_GetPatchEnkeyBase()),0x10);
		Triple_DES(&srcbArray[srcbOff], des2key, &destbArray[dsetbOff], len, ICV, ECB_Decrypt ,SENSITIVE_DATA_NOT_MASK);
	}
	else
	{
		throw_error(NULL_POINTER_EXCEPTION);
	}
}
#endif
