#include "CVM.h"
#include "stack.h"
#include "global_variable.h"
#include "ob_manager.h"
#include "object.h"
#include "object.h"
#include "utilMethod.h"
#include "SecureOperation.h"
#include "MemBackUp.h"

#if GP__CVM__FOR_MASK

#define SET_TRY_LIMIT		1
#define DEC_TRY_REMAIN		2
#define GET_TRY_REMAIN		3
#define SET_TRY_REMAIN		4
#define RESET_TRY_REAMIN	5
#define GET_PIN_FORMAT		6
#define DISABLE_PIN			7
#define ENABLE_PIN			8
#define CHECK_PIN_ACTIVE	9

/*****************************************************************************
 * File:  CVM.c
 * Version 1.0

 wang hui modify
 eastcompeace Technology Corporation

 History:
 2011,2,25	.created
 Original author: wang hui
 *****************************************************************************/
 static void decrementTriesRemain(PM8 arrayRef)
{
	u8		D_A		triesRemain;
	triesRemain = nvmReadU8(MEMtPN8(PM8_A_U8(arrayRef, CVM_TRY_REMAIND_OFFSET)));
	if (triesRemain == 0 ) {
		return ;
	}

	U8_DEC(triesRemain);
	nvmWriteU8NoAtomic(MEMtPN8(PM8_A_U8(arrayRef, CVM_TRY_REMAIND_OFFSET)), triesRemain);
//	for(i = 0;i < 3; U8_INC(i))
//	{
//		nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(CVM_TRY_REMAIND)), triesRemain);
//		memRefresh();
//		if(PN_R_U8(JcvmConfig_GetVmConfigAddr(CVM_TRY_REMAIND)) == triesRemain)
//		{
//			return;
//		}
//	}
//	while(1);
}


u8 checkPinValue()
{
	u8		D_A		triesRemain;
	u16		D_A		length;
	PM8		D_A		arrayRef;
	PM8		D_A		pinValueAddr;
	u8		D_A		result;
#define PIN_CHECK_OK			0x5A
#define PIN_CHECK_FAIL			0xA5
	result 			= 0;
	length			= popShort();
	arrayRef		= MEMtPM8(getJavaObjectDataArray());
	pinValueAddr	= getArrayDataWithCheck(length);
	
	triesRemain 	= nvmReadU8(MEMtPN8(PM8_A_U8(arrayRef, CVM_TRY_REMAIND_OFFSET)));
	
	if(triesRemain > 0)
	{
		decrementTriesRemain(arrayRef);
	
		if(length == U8t16(memReadU8(MEMtPM8(PM8_A_U8(arrayRef, CVM_PIN_LENGTH_OFFSET)))))
		{
			if(ECP_CMP_RESULT_EQUAL == SC_memCompareEqualWithDFA(pinValueAddr, MEMtPM8(PM8_A_U8(arrayRef, CVM_PIN_VALUE_OFFSET)), length))
			{
				triesRemain		= nvmReadU8(MEMtPN8(PM8_A_U8(arrayRef, CVM_TRY_MAX_OFFSET)));
				nvmWriteU8NoAtomic(MEMtPN8(PM8_A_U8(arrayRef, CVM_TRY_REMAIND_OFFSET)), triesRemain);
				memRefresh();
				result =  1;
			}
		}
	}
	
	return result;
}

u8 updatePinValue()
{
	u8   transactionStarted;
	u8		D_A		triesRemain;
	u16		D_A		format;
	u16		D_A		length;
	PM8		D_A		arrayRef;
	PM8		D_A		pinValueAddr;
	u8		D_A		curResult;

	curResult		= 0;
	format			= popShort();
	length			= popShort();
	arrayRef		= MEMtPM8(getJavaObjectDataArray());
	pinValueAddr	= getArrayDataWithCheck(length);
	if(length <= GLOBAL_PIN_SIZE)
	{
		triesRemain	= nvmReadU8( PM8_A_U8(arrayRef, CVM_TRY_MAX_OFFSET) );

		if (!isInProgress())
		{
			begin_transaction(SINGLE_APDU_PROCESS);
			transactionStarted = 1;
		}/*end if*/

		nvmWriteU8Atomic(MEMtPN8(PM8_A_U8(arrayRef, CVM_PIN_FORMAT_OFFSET)), U16t8(format));
		nvmCopyAtomic(MEMtPN8(PM8_A_U8(arrayRef, CVM_PIN_VALUE_OFFSET)), pinValueAddr, length);
		nvmWriteU8Atomic(MEMtPN8(PM8_A_U8(arrayRef, CVM_TRY_REMAIND_OFFSET)), triesRemain);
		nvmWriteU8Atomic(MEMtPN8(PM8_A_U8(arrayRef, CVM_PIN_LENGTH_OFFSET)), U16t8(length));

		if (transactionStarted)
		{
			commit_transaction();
		}/*end if*/
	
	    curResult = 1;
	}
		return curResult;

}
//merged from EPX\v3.0\DEVELOP@20220722
u8 accessPinInfo()
{
	u16		D_A		value;
	u16		D_A		funType;
	PM8		D_A		arrayRef;
	u8		D_A		curResult;

	value		= popShort();
	funType		= popShort();
	arrayRef	= MEMtPM8(get_array_data(MEMtPM8(locateObjectInMemory(popReference()))));
    curResult	= 0;

	switch(funType)
	{	
		case SET_TRY_LIMIT:
			//begin_transaction(SINGLE_APDU_PROCESS);
			nvmWriteU8NoAtomic(MEMtPN8(PM8_A_U8(arrayRef, CVM_TRY_MAX_OFFSET)), U16t8(value));
			nvmWriteU8NoAtomic(MEMtPN8(PM8_A_U8(arrayRef, CVM_TRY_REMAIND_OFFSET)), U16t8(value));
			//commit_transaction();
			memRefresh();
			curResult = 0;
			break;

		case DEC_TRY_REMAIN:
			decrementTriesRemain(arrayRef);
			curResult = 0;
			break;

		case GET_TRY_REMAIN:
			curResult = nvmReadU8(PM8_A_U8(arrayRef, CVM_TRY_REMAIND_OFFSET));
			break;

		case SET_TRY_REMAIN:
			//Operations performed by this interface shall be independent of, and notinterfere with, any transaction in progress (e.g. if the verify(byte[], short, byte, byte)method is invoked from 
		    //within a transaction and this transaction is aborted,then the try counter is not revert to its original value).
			//try remaind ???-¡Á¨®D'
			//Collise GP API-> "gpapi_testApplet_98_CVM_resetAndUnblockState_success_from_invalidSubm_abortedTrx_de_98_22"
			nvmWriteU8NoAtomic(MEMtPN8(PM8_A_U8(arrayRef, CVM_TRY_REMAIND_OFFSET)), U16t8(value));
			memRefresh();
			curResult = 0;
			break;

		case RESET_TRY_REAMIN:
			nvmWriteU8NoAtomic(MEMtPN8(PM8_A_U8(arrayRef, CVM_TRY_REMAIND_OFFSET)), nvmReadU8(MEMtPN8(PM8_A_U8(arrayRef, CVM_TRY_MAX_OFFSET))));
			memRefresh();
			curResult = 0;
			break;

		case GET_PIN_FORMAT:
			curResult = nvmReadU8(MEMtPN8(PM8_A_U8(arrayRef, CVM_PIN_FORMAT_OFFSET)));
			break;
		
		case DISABLE_PIN:
			nvmWriteU8Atomic(MEMtPN8(PM8_A_U8(arrayRef, CVM_PIN_FORMAT_OFFSET)), (U8_OR_U8(nvmReadU8(MEMtPN8(PM8_A_U8(arrayRef, CVM_PIN_FORMAT_OFFSET))), 0x80)));
			curResult = 1;
			break;
		
		case ENABLE_PIN:
			nvmWriteU8Atomic(MEMtPN8(PM8_A_U8(arrayRef, CVM_PIN_FORMAT_OFFSET)), (U8_AND_U8(nvmReadU8(MEMtPN8(PM8_A_U8(arrayRef, CVM_PIN_FORMAT_OFFSET))), 0x7F)));
			curResult = 1;
			break;
		
		case CHECK_PIN_ACTIVE:
			if(U8_AND_U8(nvmReadU8(MEMtPN8(PM8_A_U8(arrayRef, CVM_PIN_FORMAT_OFFSET))), 0x80) == 0x80)
			{
				curResult = 0;
			}
			else
			{
				curResult = 1;
			}
			break;
			
		default:
			break;
	}


	
	return curResult;
}

void getIsdGlobalPinData(void)
{
	u8		D_A    cvmId;
	PN8		D_A    pbBuffer;

	cvmId = U32t8(popShort());
	pbBuffer = MEMtPX8(getJavaObjectDataArray());

	if(cvmId != 0x11)
	{
		pbBuffer[0] = 0x03;
		pbBuffer[1] = 0x03;
		pbBuffer[2] = 0x01;
		pbBuffer[3] = 0x08;
		ecpMem_Set(MEMtPN8(&pbBuffer[4]), 0x31, 0x08);
	}
	else
	{
		memCopyToRam(MEMtPX8(&pbBuffer[0]),MEMtPN8(JcvmConfig_GetCvmPinBase()),GLOBAL_PIN_DATA_SIZE);
	}
}
#endif
