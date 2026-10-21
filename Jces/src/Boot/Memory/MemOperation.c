/*
 *
 *  Created on: 2014-2-17
 *      Author: yangfajun
 */

#include "DataOperation.h"
#include "SysConfigTarget.h"
#include "SysConfig.h"
#include "MemBackUpServe.h"
#include "MemOperationServe.h"
#include "MemOperation.h"
#include "MemNvmOperation.h"
#include "SysConfigTarget.h"
#include "SecureOperation.h"
#include "FlashBackUp.h"
#include "SecureCrypto.h"
#include "Crypto_Constant.h"

#if BOOT__MEMORY__OPERATION
/****************************************memory read********************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK u8  memReadU8(PM8 addr)
{
	u8 curReaded;
	if ((flush_flag) && check_in_cach(addr))
	{
		curReaded = stream_buffer[U16_AND_U16(PMtMEM(addr), EEPAGE_MASK)]; /*IN EEP BUT IN CATCH*/
	}
	else
	{
		curReaded = PM_R_U8(addr);
	}
	return curReaded;
}
#endif //!EMV_OPTIMIZATION_CODE

#if !EMV_OPTIMIZATION_CODE
WEAK u16 memReadU16(PM8 addr)
{
	u16 memReaded;
	memReaded = PM_R_U16(addr); 
	if (flush_flag)
	{
		if (U16_AND_U16(PMtMEM(addr), EEPAGE_MASK) != (u16)EEPAGE_MASK)	// no page crossing
		{
			if (check_in_cach(addr))
			{
				memReaded = PXB_R_U16(&stream_buffer[U16_AND_U16(PMtMEM(addr), EEPAGE_MASK)]);
			}
		}
		else
		{
			memReaded = U16_SHL_U8(memReadU8(addr), 8);
			memReaded = U16_OR_U16(memReaded, PMtMEM(memReadU8(PM8_A_CONST(addr, 1))));
		}
	}
	return memReaded;
}
#endif //!EMV_OPTIMIZATION_CODE

//#if !EMV_OPTIMIZATION_CODE
WEAK u32 memReadU32(PM8 addr)
{
	u32 tempReaded;
	tempReaded = PM_R_U32(addr);
	if (flush_flag)
	{
		if (U16_AND_U16(PMtMEM(addr), EEPAGE_MASK) < U16_S_U16(EEPAGE_SIZE, 3)) //no page crossing
		{
			if (check_in_cach(addr))
			{
				tempReaded = PXB_R_U32(&stream_buffer[U16_AND_U16(PMtMEM(addr), EEPAGE_MASK)]);
			}
		}
		else
		{
			tempReaded = U32_SHL_U8(memReadU16(addr), 16);
			tempReaded = U32_OR_U32(tempReaded, PMtMEM(memReadU16(PM8_A_U8(addr, 2))));
		}
	}

	return tempReaded;
}
//#endif //!EMV_OPTIMIZATION_CODE

#if !SEC_MEM_OPERATION
u8  memCompare(PM8 addr1, PM8 addr2, u16 length)
{
	PM8		I_A	s;
	PM8		I_A	t;
	s8		I_A	sb;
	s8		I_A	tb;
	//s8		I_A	value;
	u8		I_A	result;

	//value	= 0;

	result  = ECP_CMP_RESULT_EQUAL;
	s		= MEMtPM8(addr2);
	t		= MEMtPM8(addr1);

	while (length > 0)
	{
		U16_DEC(length);

		sb	= memReadU8(s);
		tb	= memReadU8(t);


		if (sb < tb)
		{
//			value	= 0xFF;
			sb  = ECP_CMP_RESULT_SMALLER;
		}
		else if (sb > tb)
		{
//			value	= 1;
			sb  = ECP_CMP_RESULT_GREATER;
		}
		else
		{
			sb	= ECP_CMP_RESULT_EQUAL;
		}

		if(ECP_CMP_RESULT_EQUAL == result)
		{
			result	= sb;
		}
		else
		{
			tb		= sb;  // only for delay
		}

		PM8_INC(s);
		PM8_INC(t);
	}
	return result;
}
#endif //!SEC_MEM_OPERATION


u8  memCompareEqual(PM8 addr1, PM8 addr2, u16 length)
{
	PM8		I_A	s;
	PM8		I_A	t;
	u8		I_A	result;
	u16     I_A	len;
	u16     I_A i;
	u32     X_A srcDWord;
	u32     X_A dstDWord;

	result  = ECP_CMP_RESULT_EQUAL;
	s		= MEMtPM8(addr2);
	t		= MEMtPM8(addr1);

	i = 0;
	len = length & 0x03;
	if (len != 0x00)
	{
		for (; i < len; i++)
		{
			if(memReadU8(MEMtPM8(U32_A_U16(t , i))) != memReadU8(MEMtPM8(U32_A_U16(s , i))))
			{
				result  = ECP_CMP_RESULT_NOT_EQUAL;
				return result;
			}
		}
	}
	
	for(; i < length; i +=4)
	{
		srcDWord = memReadU32(MEMtPM8(U32_A_U16(s,i)));

		dstDWord = memReadU32(MEMtPM8(U32_A_U16(t,i)));

		if(dstDWord != srcDWord)
		{
			result  = ECP_CMP_RESULT_NOT_EQUAL;
			break;
		}
	}

	return result;
}
/****************************************nvm read********************************************/

/*@ -type @*/
u8  memCompareSigned(PM8 addr1, PM8 addr2, u16 length)
{
	PM8		I_A	s;
	PM8		I_A	t;
	s8		I_A	sb;
	s8		I_A	tb;

	s		= MEMtPM8(addr1);
	t		= MEMtPM8(addr2);
	while (length > 0)
	{
		U16_DEC(length);
		sb	= memReadU8(s);
		tb	= memReadU8(t);
		if(sb > tb)
			return GREATER;
		else if(sb < tb)
			return LESS;
		PM8_INC(s);
		PM8_INC(t);
	}
	return EQUAL;
}

#if WITH_MEMCOMPARE  //for Java card 3.0.5 ¡£dengyiwen
u8 memCompareU16(PM8 addr1, PM8 addr2, u16 length)
{
	u8 i;
	s16 s;
	s16 t;
	for(i = 0;i < length * 2;i += 2)
	{
	s = U16tS16(memReadU16(addr1 + i));
	t = U16tS16(memReadU16(addr2 + i));
	  if(t < s)
	  {
	   	return GREATER;
	  }
	  else if(t > s)
	  {
		  return LESS;
	  }
	}
	return EQUAL;
}

u8 memCompareU32(PM8 addr1, PM8 addr2, u16 length)
{
	u8 i;
	s32 s;
	s32 t;
	for(i = 0;i < length * 4;i += 4)
	{
	s = U32tS32(memReadU32(addr1 + i));
	t = U32tS32(memReadU32(addr2 + i));
	  if(t < s)
	  {
	   	return GREATER;
	  }
	  else if(t > s)
	  {
		  return LESS;
	  }
	}
	return EQUAL;
}
#endif
/*@ +type @*/
u8 memCompareWithStatus(PM8 addr1, PM8 addr2, u16 length, u8 statusOK, u8 statusFail)
{

	if(ECP_CMP_RESULT_EQUAL == memCompare(addr1, addr2, length))
	{
		return statusOK;
	}
	else
	{
		return statusFail;
	}
}


/****************************************nvm write atomic********************************************/

void nvmWriteU8Atomic(PN8 dest, u8 value)
{
	if (isInProgress())
	{
		setWriteEepType(WRITEEP_TRANSACTION);
	}
	else
	{
		if(U32_AND_U32(PNtMEM(dest), NOR_EEPAGE_MASK) == PNtMEM(stream_page))
		{
			setWriteEepType(gBackUp_cacheType);
		}
		else
		{
			setWriteEepType(WRITEEP_ATOMIC);
		}
	}

	E2P_stream_open(dest);

	stream_buffer[stream_page_offset] = value;
	flush_flag = 1;
}

void nvmWriteU16Atomic(PN8 dest, u16 value)
{
	if (isInProgress() || U16_AND_U16(PNtMEM(dest), EEPAGE_MASK) == (u16)EEPAGE_MASK)
	{
		setWriteEepType(WRITEEP_TRANSACTION);
		if (!isInProgress())
		{
			openBackUpPageNumStart();
		}
	}
	else
	{
		if(U32_AND_U32(PNtMEM(dest), NOR_EEPAGE_MASK) == PNtMEM(stream_page))
		{
			setWriteEepType(gBackUp_cacheType);
		}
		else
		{
			setWriteEepType(WRITEEP_ATOMIC);
		}
	}

	E2P_stream_open(dest);

	stream_buffer[stream_page_offset]	= U16t8(U16_SHR_U8(value, 8));

	if (stream_page_offset == (u16)EEPAGE_MASK)
	{
		flush_flag = 1;
		E2P_stream_open(PN8_A_CONST(dest, 1));
	}
	else
	{
		U16_INC(stream_page_offset);
	}

	stream_buffer[stream_page_offset]	= U16t8(value);
	flush_flag							= 1;

	if (!isInProgress() && (WRITEEP_TRANSACTION == getWriteEepType()))
	{
		memCommitTransaction();
	}
}

void nvmWriteU32Atomic(PN8 dest, u32 value)
{
	u8 X_A temp[SIZE_U32];
	PX_W_U32(temp, value);
	nvmCopyAtomic(dest, MEMtPN8(temp), SIZE_U32);
}

u16 nvmCopyAtomic(PN8 dest, PM8 src, u16 length)
{
	u8	   flag;
	flag = false;
	 // in transaction;
	if (isInProgress() || (U16_S_U16(EEPAGE_SIZE, U16_AND_U16(PNtMEM(dest), EEPAGE_MASK)) < length))
	{
		setWriteEepType(WRITEEP_TRANSACTION);
		if (!isInProgress())
		{
			openBackUpPageNumStart();
			flag = 1;
		}
	}
	else
	{
		if(U32_AND_U32(PNtMEM(dest), NOR_EEPAGE_MASK) == PNtMEM(stream_page))
		{
			setWriteEepType(gBackUp_cacheType);
		}
		else
		{
			setWriteEepType(WRITEEP_ATOMIC);
		}
	}

	E2P_stream_open(dest);

	E2P_stream_write_array(src, length);

	if (flag)
	{
		memCommitTransaction();
	}
	return ECP_ERR_OK;
}

void nvmFillAtomic(PN8 dest, u8 value, u16 length)
{
	u8	flag = false;

	if (isInProgress()	|| (U16_S_U16(EEPAGE_SIZE, U16_AND_U16(PNtMEM(dest), EEPAGE_MASK)) < length))
	{
		setWriteEepType(WRITEEP_TRANSACTION);
		if (!isInProgress())
		{
			openBackUpPageNumStart();
			flag = 1;
		}
	}
	else
	{
		if(U32_AND_U32(PNtMEM(dest), NOR_EEPAGE_MASK) == PNtMEM(stream_page))
		{
			setWriteEepType(gBackUp_cacheType);
		}
		else
		{
			setWriteEepType(WRITEEP_ATOMIC);
		}
	}

	E2P_stream_open(dest);

	E2P_stream_fill_array(length, value);

	if (flag)
	{
		memCommitTransaction();
	}
}

/****************************************nvm write nonatomic********************************************/



/****************************************nvm write lost********************************************/




/****************************************mem write atomic********************************************/

void memWriteU8Atomic(PM8 dest, u8 value)
{
	if (isInE2P(dest))
	{
		nvmWriteU8Atomic(dest, value);
	}
	else
	{
		PX_W_U8(dest, value);
	}
}

void memWriteU16Atomic(PM8 dest, u16 value)
{
	if (isInE2P(dest))
	{
		nvmWriteU16Atomic(dest, value);
	}
	else
	{
		PX_W_U16(dest, value);
	}
}

void memWriteU32Atomic(PM8 dest, u32 value)
{
	if (isInE2P(dest))
	{
		nvmWriteU32Atomic(dest, value);
	}
	else
	{
		PX_W_U32(dest, value);
	}
}

u16 memCopyAtomic(PM8 dest, PM8 src, u16 length)
{
	u16    retStatus;
	retStatus = ECP_ERR_FAILURE;
	if (isInE2P(dest))
	{
		retStatus = nvmCopyAtomic(dest, src, length);
	}
	else
	{
		retStatus = memCopyToRam(dest, src, length);
	}
	return retStatus;
}

void memFillAtomic(PM8 dest, u8 value, u16 length)
{
	if (isInE2P(dest))
	{
		nvmFillAtomic(dest, value, length);
	}
	else
	{
		memFillToRam(MEMtPX8(dest), value, length);
	}
}

/****************************************mem write noatomic********************************************/

void memWriteU8NoAtomic(PM8 dest, u8 value)
{
	if (isInE2P(dest))
	{
		nvmWriteU8NoAtomic(dest, value);
	}
	else
	{
		PX_W_U8(dest, value);
	}
}

void memWriteU16NoAtomic(PM8 dest, u16 value)
{
	if (isInE2P(dest))
	{
		nvmWriteU16NoAtomic(dest, value);
	}
	else
	{
		PX_W_U16(dest, value);
	}
}

void memWriteU32NoAtomic(PM8 dest, u32 value)
{
	if (isInE2P(dest))
	{
		nvmWriteU32NoAtomic(dest, value);
	}
	else
	{
		PX_W_U32(dest, value);
	}
}

u16 memCopyNoAtomic(PM8 dest, PM8 src, u16 length)
{
	u16    retStatus;
	retStatus = ECP_ERR_FAILURE;
	if (isInE2P(dest))
	{
		retStatus = nvmCopyNoAtomic(dest, src, length);
	}
	else
	{
		retStatus = memCopyToRam(dest, src, length);
	}
	return retStatus;
}

void memFillNoAtomic(PM8 dest, u8 value, u16 length)
{
	u8 rngValue[1];
	if (isInE2P(dest))
	{
		nvmFillNoAtomic(dest, value, length);
	}
	else
	{
		//SC_checkRandamReturnCode(Random_number_generator(MEMtPX8(rngValue), 1 , ALG_SECURE_RANDOM));
		//memFillToRam(dest, rngValue[0], length);
		memFillToRam(dest, value, length);
	}
}

/****************************************mem write lost********************************************/





void writeEeprom(PN8 dest, PM8 src, u16 length)
{
	u8	X_A		tempArray[EEPAGE_SIZE];
	
#if BOOT__MEMORY__BACK_UP
	//if flash support,when compacting, we will protect the first and the last page if dest is not backup address.
	if(dest	< BACK_UP_AREA_START)
	{
		FlashBackUp_PageProtect(PNtMEM(dest), tempArray);
		FlashBackUp_PageProtect(PNtMEM(PN8_A_U16(dest, length)), tempArray);
		MemNvmOperation_WriteErase(dest, src, length);
		FlashBackUp_CommitFlashFlag();
	}
	else
	#endif
	{
		SC_deactiveEDC();
		MemNvmOperation_WriteErase(dest, src, length);
		SC_activeEDC();
	}
}

/********************************mem write no cache, no back up*****************************************/
void nvmCopyNoCache(PN8 dest, PM8 src, u16 length)
{
	MemNvmOperation_WriteErase(dest, src, length);
}

void nvmFillNoCache(PN8 dest, u8 value, u16 length)
{
	MemNvmOperation_Fill(dest, value, length);
}

void nvmClearNoCache(PN8 dest, u16 length)
{
	MemNvmOperation_Erase(dest, length);
}

void memStreamClr(void)
{
	E2P_stream_clear();
}


#endif //BOOT__MEMORY__OPERATION
