/*
 *
 *  Created on: 2014-2-19
 *      Author: yangfajun
 */

#include "NvmHal.h"
#include "MemNvmOperation.h"
#include "MemBackUpServe.h"
#include "BootErrorHandle.h"
#include "BootLifeCycle.h"
#include "FlashBackUp.h"
#include "anomaly.h"

#if BOOT__MEMORY__OPERATION

#define	MOST_CHECK_TIME			2

u8 MemNvmOperation_WriteImpl(PN8 dest, PM8 src, u16 length, u8 type)
{
	u16     X_A     checkLength;
	u16	    X_A     chunk;
	u8		X_A     time;
	
	while(length > 0)
	{
		time	= 0;
		chunk 	= U16_S_U16(HAL_EEPAGE_SIZE, U16_AND_U16(U32t16(PMtMEM(dest)), HAL_EEPAGE_MASK));
        
		if(length > chunk)
		{
			checkLength     = chunk;
		}
		else
		{
			checkLength      = length;
		}
		
        do
        {
			if(time >= MOST_CHECK_TIME)
			{
				bootErrorHandle_ThrowError(SYSTEMEXCEPT_WRITE_MEM_FAIL);
			}
			
			switch(type)
			{
				case WRITE_ONLY:
					ecpNvm_WriteOnly(dest, src, checkLength);
//					src 	= PM8_A_U16(src, checkLength);
					break;
				case WRITE_ERASE:
					ecpNvm_WriteErase(dest, src, checkLength);
//					src 	= PM8_A_U16(src, checkLength);
					break;
				case FILL_ERASE:
					ecpNvm_WriteErase(dest, src, checkLength);
					break;
				default:
					bootErrorHandle_ThrowError(SYSTEMEXCEPT_WRITE_MEM_FAIL);
					break;
			}
            ecpNvm_WriteCommit();
			
			U8_INC(time);
        }while(CHECK_WRITE_NOK == ecpNvm_CheckWrite(dest, src, checkLength));
		
		if((length > chunk) && (type != FILL_ERASE))
		{
			src 	= PM8_A_U16(src, checkLength);
		}
		
		dest 	= PN8_A_U16(dest, checkLength);
		length 	= U16_S_U16(length, checkLength);
	}

	return 0;
}

u8 MemNvmOperation_Erase(PN8 dest, u16 length)
{
	if(((U32_R_U16(PNtMEM(dest), HAL_EEPAGE_SIZE) != 0) || (U32_R_U16(U16t32(length), HAL_EEPAGE_SIZE) != 0)) && (length != 0))
	{
		bootErrorHandle_ThrowError(SYSTEMEXCEPT_WRITE_MEM_FAIL);
	}
	
	while(length > 0)
	{
//		if((U32_R_U16(PNtMEM(dest) , U8_M_U16(ERASEBLOCKSIZE , HAL_EEPAGE_SIZE)) == 0) && (length >= U32t16(U8_M_U16(ERASEBLOCKSIZE , HAL_EEPAGE_SIZE))))
//		{
//			ecpNvm_EraseBlock(dest);
//			length 	= U16_S_U16(length,U8_M_U16(ERASEBLOCKSIZE , HAL_EEPAGE_SIZE));
//			dest 	= PN8_A_U16(dest, U8_M_U16(ERASEBLOCKSIZE , HAL_EEPAGE_SIZE));
//		}
//		else
		{
			ecpNvm_ErasePage(dest);
			length  = U16_S_U16(length, HAL_EEPAGE_SIZE);
			dest 	= PN8_A_U16(dest, HAL_EEPAGE_SIZE);
		}
	}
	
	return 0;
}

u8 MemNvmOperation_WriteOnly(PN8 dest, PM8 src, u16 length)
{
	return MemNvmOperation_WriteImpl(dest, src, length, WRITE_ONLY);
}

u8 MemNvmOperation_WriteErase(PN8 dest, PM8 src, u16 length)
{
	return MemNvmOperation_WriteImpl(dest, src, length, WRITE_ERASE);
}

u8 MemNvmOperation_Fill(PN8 dest, u8 value, u16 length)
{
	PX8		X_A 	tmpCachebuffer;
	
	tmpCachebuffer	= MEMtPX8(sysConfig_GetEepromBuffer(HAL_EEPAGE_SIZE));

	if(length > HAL_EEPAGE_SIZE)
	{
		ecpMem_Set(tmpCachebuffer, value, HAL_EEPAGE_SIZE);
	}
	else
	{
		ecpMem_Set(tmpCachebuffer, value, length);
	}

	return MemNvmOperation_WriteImpl(dest, tmpCachebuffer, length, FILL_ERASE);
}

#endif
