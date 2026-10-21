#include "SecureOperation.h"
#include "Crypto_Constant.h"
#include "Crypto_API.h"
#include "MemOperation.h"
#include "MemBackUp.h"
#include "RNG_API.h"
#include "NvmMap.h"

#if SEC_MEM_OPERATION
u8		isSecureCopy;
u8		isSecureCopyInv;

void SC_SetSecCopyFlag(u8 flag)
{
	isSecureCopy	= flag;
	isSecureCopyInv	= U8_INV(flag);
}

void SC_SecOpeInit(void)
{
	SC_SetSecCopyFlag(COMMON_COPY_FLAG);
}

u16 ecp_calSecCmpParamChecksum(ecpSecCmp_secParam_t * cmpSecParam)
{
	u8      X_A     tempbuff[10];

	PXS_W_U32(tempbuff, PMtMEM(cmpSecParam->ecpSecCmpSrc));
	PXS_W_U32(PM8_A_CONST(tempbuff,4), PMtMEM(cmpSecParam->ecpSecCmpDes));
	PXS_W_U16(PM8_A_CONST(tempbuff,8), cmpSecParam->ecpSecCmpLength);

	return Get_crc16(tempbuff, 10);
}

void ecpMem_SecureCompareParamInit(ecpSecCmp_secParam_t * cmpSecParam, PM8 Des, PM8 Src, PX16 secCmpInvRes,u16 length)
{
	cmpSecParam->ecpSecCmpDes 				= Des;
	cmpSecParam->ecpSecCmpSrc 				= Src;
	cmpSecParam->ecpSecCmpLength 			= length;
	cmpSecParam->ecpSecCmpFuncSignature 	= ECP_FUNC_SIGNATURE_INIT;
	cmpSecParam->ecpSecCmpInvertResult		= secCmpInvRes;
	cmpSecParam->ecpSecCmpParamChecksum 	= ecp_calSecCmpParamChecksum(cmpSecParam);
	return;
}

u16 ecpMem_SecureCompare(ecpSecCmp_secParam_t * cmpSecParam)
{
	memref  	X_A   	src;
	memref  	X_A   	des;
	u16 		X_A		length;
	u16 		X_A		compareResult;
	u8			X_A		xorResult;
	u8			X_A		minusResult;
	u16 		X_A		rndXorStarIndex;
	u16 		X_A		rndMinusStarIndex;
	u16 		X_A		signaturebackup;
	u16 		X_A		signature;
	u16 		X_A		i;
	u16 		X_A		j;
	u16			X_A		k;
	u16			X_A		tempK1;
	u16         X_A     tempK2;
	u16 		X_A		in;
	u8			X_A		srcTmpi;
	u8			X_A		desTmpi;
	u8			X_A		srcTmpj;
	u8			X_A		desTmpj;
	u8			X_A		srcTmpk;
	u8			X_A		desTmpk;
	u32		    X_A		randomDword;
	u16		    X_A		retStatus;
	
	PATCH_BOOT_SECUREOPERATION_001();
	

	des 				= (memref)cmpSecParam->ecpSecCmpDes;
	src 				= (memref)cmpSecParam->ecpSecCmpSrc;
	length 				= cmpSecParam->ecpSecCmpLength;
	xorResult 			= (u8)XOR_INIT_EQU;
	minusResult 		= MINUS_INIT_EQU;
	compareResult 		= ECP_CMP_RESULT_NOT_EQUAL;
	signature 			= cmpSecParam->ecpSecCmpFuncSignature;
	signaturebackup 	= signature;
	
	retStatus = ECP_ERR_RNG_GETRANDOM_FAILURE;
	retStatus = getDataFromRngCache((PX8)&randomDword, 0x04);
	if (retStatus != ECP_ERR_RNG_GETRANDOM_OK)
	{
		faultDetected();
	}

	in 					= 0;

	if(length > 0)
	{
		rndXorStarIndex   = randomDword >> 16;
		rndMinusStarIndex = randomDword & 0xFFFF;
		rndXorStarIndex 	= U16_R_U16(rndXorStarIndex , length);
		rndMinusStarIndex 	= U16_R_U16(rndMinusStarIndex , length);
		i 					= rndXorStarIndex;
		j 					= rndMinusStarIndex;
		k 					= length;

		do
		{
			U16_INC(i);
			if(i == length)
			{
				i = 0;
			}

			if(j == 0)
			{
				j = (length - 1);
			}
			else
			{
				U16_DEC(j);
			}
			
			U16_DEC(k);
			
			srcTmpi = memReadU8(MEMtPM8(U32_A_U16(src,i)));
			srcTmpj = memReadU8(MEMtPM8(U32_A_U16(src,j)));
			srcTmpk = memReadU8(MEMtPM8(U32_A_U16(src,k)));

			desTmpi = memReadU8(MEMtPM8(U32_A_U16(des,i)));
			desTmpj = memReadU8(MEMtPM8(U32_A_U16(des,j)));
			desTmpk = memReadU8(MEMtPM8(U32_A_U16(des,k)));
			
			xorResult 		= U8_AND_U8 (xorResult,U8_INV(U8_XOR_U8(desTmpi , srcTmpi)));
			minusResult 	= U8_OR_U8(U8_S_U8(desTmpj, srcTmpj),minusResult);
			
			//resist timing attack
			compareResult	= U8_S_U8(desTmpk, srcTmpk);
			if(0 != compareResult)
			{
				tempK1		= k;
			}
			
			if(compareResult == 0)
			{
				tempK2		= k;
			}
			
			U16_INC(in);
		}while((i != rndXorStarIndex) && (j != rndMinusStarIndex) && (in != length));
	}
	//check comparison is not skipped
	if(in == length)
	{
		signature 		+= ECP_COMPARE_FINISH;
	}

	//double check comparison result
	if((xorResult == (u8)XOR_INIT_EQU && minusResult == MINUS_INIT_EQU)
	   || (xorResult != (u8)XOR_INIT_EQU && minusResult != MINUS_INIT_EQU)
	  )
	 {
	 	signature 		+= ECP_COMPARE_CHECK;
	 }
	 else
	 {
		 faultDetected();
	 }

	 //check all the above steps performed
	 if(signature == U16_A_U16(U16_A_U16(signaturebackup ,ECP_COMPARE_FINISH), ECP_COMPARE_CHECK))
	 {
	 	if(xorResult == (u8)XOR_INIT_EQU)
	 	{
	 		PX_W_U16(cmpSecParam->ecpSecCmpInvertResult, U16_INV(ECP_CMP_RESULT_EQUAL));
	 	}
	 	else
	 	{
	 		PX_W_U16(cmpSecParam->ecpSecCmpInvertResult, U16_INV(ECP_CMP_RESULT_NOT_EQUAL));
	 	}

	 	signature 		+= ECP_RETURN_INVERT_COMPARE_RESULT;

	 	if(MINUS_INIT_EQU == minusResult)
	 	{
	 		compareResult = ECP_CMP_RESULT_EQUAL;
	 	}
	 	else
	 	{
			srcTmpi = memReadU8(MEMtPM8(U32_A_U16(src,tempK1)));
			desTmpi = memReadU8(MEMtPM8(U32_A_U16(des,tempK1)));
			if(U8_XOR_U8(U8_AND_U8(srcTmpi, 0x80), U8_AND_U8(desTmpi, 0x80)) == 0x00) //同为正或负
			{
				if(srcTmpi > desTmpi)
				{
					compareResult = ECP_CMP_RESULT_GREATER;
					//by zhangrufieng
					PX_W_U16(cmpSecParam->ecpSecCmpInvertResult, U16_INV(ECP_CMP_RESULT_GREATER));
				}
				else
				{
					compareResult = ECP_CMP_RESULT_SMALLER;
					//by zhangrufieng
					PX_W_U16(cmpSecParam->ecpSecCmpInvertResult, U16_INV(ECP_CMP_RESULT_SMALLER));
				}
			}
			else
			{
				if(U8_AND_U8(desTmpi, 0x80) > 0)
				{
					compareResult = ECP_CMP_RESULT_GREATER;
					//by zhangrufieng
					PX_W_U16(cmpSecParam->ecpSecCmpInvertResult, U16_INV(ECP_CMP_RESULT_GREATER));
				}
				else
				{
					compareResult = ECP_CMP_RESULT_SMALLER;
					//by zhangrufieng
					PX_W_U16(cmpSecParam->ecpSecCmpInvertResult, U16_INV(ECP_CMP_RESULT_SMALLER));
				}
			}
	 	}
	 	signature 		+= ECP_RETURN_COMPARE_RESULT;
	 }
	 else
	 {
		 faultDetected();
	 }
	 
	 //check parameter is not modified
	if((cmpSecParam->ecpSecCmpParamChecksum != ecp_calSecCmpParamChecksum(cmpSecParam))
	  || (cmpSecParam->ecpSecCmpSrc != MEMtPM8(src)) || (length != cmpSecParam->ecpSecCmpLength))
	{
		faultDetected();
	}
	
	cmpSecParam->ecpSecCmpFuncSignature = signature;
	 return compareResult;
}



//u16 ecp_calSecCpyParamChecksum(ecpSecCpy_secParam_t * cpySecParam)
//{
//	u8      X_A     tempbuff[10];

//	PX_W_U32(tempbuff, PMtMEM(cpySecParam->ecpSecCpySrc));
//	PX_W_U32(PM8_A_CONST(tempbuff,4), PMtMEM(cpySecParam->ecpSecCpyDes));
//	PX_W_U16(PM8_A_CONST(tempbuff,8), cpySecParam->ecpSecCpyLength);

//	return Get_crc16(tempbuff, 10);
//}

//void ecpMem_SecCpyParamParamInit(ecpSecCpy_secParam_t * cpySecParam, PM8 Des, PM8 Src, u16 length)
//{
//	cpySecParam->ecpSecCpyDes = Des;
//	cpySecParam->ecpSecCpySrc = Src;
//	cpySecParam->ecpSecCpyLength = length;
//	cpySecParam->ecpSecCpyParamChecksum = ecp_calSecCpyParamChecksum(cpySecParam);
//	return;
//}

u16 SC_MemCopyRam2Ram(SC_SecCpyStruct secCpyPara)
{
	u8    X_A    confusionBuff[8];
	PX8          rngBuff;
	PX8          confusionDest;
	PX8          confusionSrc;
	u16   X_A    confusionRLen;
	u8    X_A    confusionFLen;
	PX8          dest;
	PX8          src;
	u16          length;
	u16				retStatus;
	retStatus		= ECP_ERR_FAILURE;
	
	PATCH_BOOT_MEMORY_MEMOPERATIONSERVE_004();
	
	rngBuff = (PX8)g_TRNG_Cache.Data;
	confusionDest = confusionBuff;
	confusionSrc  = &rngBuff[4];
	/* rngBuff[0] : the random byte to decide first round copy confusion
	 * rngBuff[1] : the random byte to decide second round copy confusion
	 * rngBuff[2] : the random byte to divide real copy length
	 * rngBuff[3] : the random byte to divide fake copy length
	 */
	
	SC_parameterIntegrityCheck(secCpyPara);
	
	dest   = secCpyPara.secCpyDes;
	src    = secCpyPara.secCpySrc;
	length = secCpyPara.secCpyLength;
	
	if (secCpyPara.secCpyLength != 0)
	{
		if((dest > src) && (dest < PX8_A_U16(src, length)))
		{
			confusionRLen = U16_R_U16(rngBuff[2], length);
			confusionFLen = U8_AND_U8(rngBuff[3], 0x07);
			if(U8_AND_U8(rngBuff[0], 0x11) == 0x11)
			{
				ecpMem_Move(PX8_A_U8(dest, confusionRLen), PX8_A_U8(src, confusionRLen), (s32)U16_S_U16(length, confusionRLen));
				
				ecpMem_Move(confusionDest, confusionSrc, confusionFLen);
				confusionDest = PX8_A_U8(confusionDest, confusionFLen);
				confusionSrc  = PX8_A_U8(confusionSrc, confusionFLen);
			}
			else
			{
				ecpMem_Move(confusionDest, confusionSrc, confusionFLen);
				confusionDest = PX8_A_U8(confusionDest, confusionFLen);
				confusionSrc  = PX8_A_U8(confusionSrc, confusionFLen);
				
				ecpMem_Move(PX8_A_U8(dest, confusionRLen), PX8_A_U8(src, confusionRLen), (s32)U16_S_U16(length, confusionRLen));
			}
			
			confusionFLen = U8_S_U8(8, confusionFLen);
			if(U8_AND_U8(rngBuff[1], 0x44) == 0x44)
			{
				ecpMem_Move(dest, src, (s32)confusionRLen);
				ecpMem_Move(confusionDest, confusionSrc, confusionFLen);
			}
			else
			{
				ecpMem_Move(confusionDest, confusionSrc, confusionFLen);
				ecpMem_Move(dest, src, (s32)confusionRLen);
			}
		}
		else
		{
			confusionRLen = U16_R_U16(rngBuff[2], length);
			confusionFLen = U8_AND_U8(rngBuff[3], 0x07);
			if(U8_AND_U8(rngBuff[0], 0x11) == 0x11)
			{
				ecpMem_Move(dest, src, (s32)confusionRLen);
				dest = PX8_A_U16(dest, confusionRLen);
				src  = PX8_A_U16(src, confusionRLen);
				
				ecpMem_Move(confusionDest, confusionSrc, confusionFLen);
				confusionDest = PX8_A_U8(confusionDest, confusionFLen);
				confusionSrc  = PX8_A_U8(confusionSrc, confusionFLen);
			}
			else
			{
				ecpMem_Move(confusionDest, confusionSrc, confusionFLen);
				confusionDest = PX8_A_U8(confusionDest, confusionFLen);
				confusionSrc  = PX8_A_U8(confusionSrc, confusionFLen);
				
				ecpMem_Move(dest, src, (s32)confusionRLen);
				dest = PX8_A_U16(dest, confusionRLen);
				src  = PX8_A_U16(src, confusionRLen);
			}
			
			confusionRLen = U16_S_U16(length, confusionRLen);
			confusionFLen = U8_S_U8(8, confusionFLen);
			if(U8_AND_U8(rngBuff[1], 0x44) == 0x44)
			{
				ecpMem_Move(dest, src, (s32)confusionRLen);
				ecpMem_Move(confusionDest, confusionSrc, confusionFLen);
			}
			else
			{
				ecpMem_Move(confusionDest, confusionSrc, confusionFLen);
				ecpMem_Move(dest, src, (s32)confusionRLen);
			}
		}
		
		// 通过补比较RNG value 已确保前面的流程被正常执行过。
		if(0x00 == ecpMem_Compare(confusionBuff, &rngBuff[4], 8))
		{
			// double check user confusionFLen 
			if(U8_AND_U8(rngBuff[3], 0x07) + confusionFLen == 8)
			{
				retStatus = ECP_ERR_OK;
			}
			else
			{
				faultDetected();
			}
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (0 != length)
		{
			faultDetected();
		}
		else
		{
				retStatus = ECP_ERR_OK;
		}
	}
	
	if (secCpyPara.secCpyLength != length)
	{
		faultDetected();
	}
	
	return retStatus;
}
u16 SC_MemCopyRom2Ram(SC_SecCpyStruct secCpyPara)
{
	u8    X_A    confusionBuff[8];
	PX8          rngBuff;
	PX8          confusionDest;
	PM8          confusionSrc;
	u16   X_A    confusionRLen;
	u8    X_A    confusionFLen;
	PX8          dest;
	PM8          src;
	u16          length;
	
	PATCH_BOOT_MEMORY_MEMOPERATIONSERVE_004();
	
	rngBuff = (PX8)g_TRNG_Cache.Data;
	confusionDest = confusionBuff;
	confusionSrc  = &rngBuff[4];
	/* rngBuff[0] : the random byte to decide first round copy confusion
	 * rngBuff[1] : the random byte to decide second round copy confusion
	 * rngBuff[2] : the random byte to divide real copy length
	 * rngBuff[3] : the random byte to divide fake copy length
	 */
	
	SC_parameterIntegrityCheck(secCpyPara);
	
	dest   = secCpyPara.secCpyDes;
	src    = secCpyPara.secCpySrc;
	length = secCpyPara.secCpyLength;
	
	if (secCpyPara.secCpyLength != 0)
	{
		confusionRLen = U16_R_U16(rngBuff[2], length);
		confusionFLen = U8_AND_U8(rngBuff[3], 0x07);
		if(U8_AND_U8(rngBuff[0], 0x11) == 0x11)
		{
			ecpMem_Copy(dest, src, (s32)confusionRLen);
			dest = PX8_A_U16(dest, confusionRLen);
			src  = PM8_A_U16(src, confusionRLen);
			
			ecpMem_Copy(confusionDest, confusionSrc, confusionFLen);
			confusionDest = PX8_A_U8(confusionDest, confusionFLen);
			confusionSrc  = PM8_A_U8(confusionSrc, confusionFLen);
		}
		else
		{
			ecpMem_Copy(confusionDest, confusionSrc, confusionFLen);
			confusionDest = PX8_A_U8(confusionDest, confusionFLen);
			confusionSrc  = PM8_A_U8(confusionSrc, confusionFLen);
			
			ecpMem_Copy(dest, src, (s32)confusionRLen);
			dest = PX8_A_U16(dest, confusionRLen);
			src  = PM8_A_U16(src, confusionRLen);
		}
		
		confusionRLen = U16_S_U16(length, confusionRLen);
		confusionFLen = U8_S_U8(8, confusionFLen);
		if(U8_AND_U8(rngBuff[1], 0x44) == 0x44)
		{
			ecpMem_Copy(dest, src, (s32)confusionRLen);
			ecpMem_Copy(confusionDest, confusionSrc, confusionFLen);
		}
		else
		{
			ecpMem_Copy(confusionDest, confusionSrc, confusionFLen);
			ecpMem_Copy(dest, src, (s32)confusionRLen);
		}
	}
	
	if (secCpyPara.secCpyLength != length)
	{
		faultDetected();
	}
	return ECP_ERR_OK;
}

u16 SC_MemCopy(SC_SecCpyStruct secCpyPara, u8 type)
{
	u16		 				i;
	u16                     len;
	u16		        		retStatus;
	
	retStatus		= ECP_ERR_FAILURE;

	SC_SetSecCopyFlag(SECURE_COPY_FLAG);
	
	SC_parameterIntegrityCheck(secCpyPara);
	
	if(ECP_COPY_TYPE_MEM_2_RAM == type)
	{
		memCopyToRam(secCpyPara.secCpyDes, secCpyPara.secCpySrc, secCpyPara.secCpyLength);
	}
	else if(ECP_COPY_TYPE_ATOMIC == type)
	{
		memCopyAtomic(secCpyPara.secCpyDes, secCpyPara.secCpySrc, secCpyPara.secCpyLength);
	}
	else
	{
		memCopyNoAtomic(secCpyPara.secCpyDes, secCpyPara.secCpySrc, secCpyPara.secCpyLength);
	}

	//check the copy is success  
	if(
		//如果copy动作存在重叠区，不进行比较
		((PMtMEM(secCpyPara.secCpySrc) < PMtMEM(secCpyPara.secCpyDes)) && (PMtMEM(PM8_A_U16(secCpyPara.secCpySrc, secCpyPara.secCpyLength)) >= PMtMEM(secCpyPara.secCpyDes))) ||
		((PMtMEM(secCpyPara.secCpyDes) < PMtMEM(secCpyPara.secCpySrc)) && (PMtMEM(PM8_A_U16(secCpyPara.secCpyDes, secCpyPara.secCpyLength)) >= PMtMEM(secCpyPara.secCpySrc))) || 
		//如果是 RAM_2_RAM, 数据极大可能为明文敏感数据，此处不进行比较防止侧信道的攻击，在SC_MemCopyRam2Ram中保证RAM拷贝被执行。
		( (ECP_COPY_TYPE_MEM_2_RAM == type) && (!isInE2P(secCpyPara.secCpySrc))) 
	  ) 
	{
		i = secCpyPara.secCpyLength;
	}
	else
	{
		i = 0;
		len = secCpyPara.secCpyLength & 3;
		if (len != 0x00)
		{
			for (; i < len; U16_INC(i))
			{
				if(memReadU8(MEMtPM8(U32_A_U16(secCpyPara.secCpyDes, i))) != memReadU8(MEMtPM8(U32_A_U16(secCpyPara.secCpySrc, i))))
				{
					faultDetected();
				}
			}
		}
		
		for(; i < secCpyPara.secCpyLength; i = U16_A_U8(i, 4))
		{
			if(memReadU32(MEMtPM8(U32_A_U16(secCpyPara.secCpyDes , i))) != memReadU32(MEMtPM8(U32_A_U16(secCpyPara.secCpySrc, i))))
			{
				faultDetected();
			}
		}
	}
	
	//guarantee the copy check performed
	if(i != secCpyPara.secCpyLength)
	{
		faultDetected();
	}
	else
	{
		retStatus = ECP_ERR_OK;
	}
	
	SC_parameterIntegrityCheck(secCpyPara);
	
	if(SC_CheckSceCopyFlag())
	{
		if(SECURE_COPY_FLAG == isSecureCopy)
		{
			SC_SetSecCopyFlag(COMMON_COPY_FLAG);
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		faultDetected();
	}
	
	return retStatus;
}

u8 SC_memCompare(PM8 Des, PM8 Src, u16 length)
{
	ecpSecCmp_secParam_t	X_A			myCmpSecParam;
	u8						X_A			compareResult;
	u16						X_A			invertCompareResult;

	ecpMem_SecureCompareParamInit(&myCmpSecParam, Des, Src, &invertCompareResult, length);
	compareResult 		= ecpMem_SecureCompare(&myCmpSecParam);
	if (myCmpSecParam.ecpSecCmpFuncSignature != ECP_SECURE_COMPARE_PERFORM)
	{
		faultDetected();
	}
	
	compareResult		= SC_StateCheck(compareResult, PX_R_U16((PX8)&invertCompareResult), ECP_SECURE_CHECK_INIT);

	return compareResult;
}

u8 SC_StateCheck(u8 state, u8 inverseState, u16 initCtrFlow)
{
	PATCH_BOOT_SECUREOPERATION_003();
	switch(state)
	{
		case ECP_SEC_CHECK_STATE_1:
			if(inverseState != U8_INV(ECP_SEC_CHECK_STATE_1))
			{
				faultDetected();
			}
			initCtrFlow	+= ECP_SECURE_CHECK_SIGNATURE;
			break;
		case ECP_SEC_CHECK_STATE_2:
			if(inverseState != U8_INV(ECP_SEC_CHECK_STATE_2))
			{
				faultDetected();
			}
			initCtrFlow	+= ECP_SECURE_CHECK_SIGNATURE;
			break;
		case ECP_SEC_CHECK_STATE_3:
			if(inverseState != U8_INV(ECP_SEC_CHECK_STATE_3))
			{
				faultDetected();
			}
			initCtrFlow	+= ECP_SECURE_CHECK_SIGNATURE;
			break;
		case ECP_SEC_CHECK_STATE_4:
			if(inverseState != U8_INV(ECP_SEC_CHECK_STATE_4))
			{
				faultDetected();
			}
			initCtrFlow	+= ECP_SECURE_CHECK_SIGNATURE;
			break;
		default:
			faultDetected();
			break;
	}
	
	//check secure state check performed
	if(initCtrFlow != ECP_SECURE_CHECK_PERFORM)
	{
		faultDetected();
	}
	
	return state;
}

u8 SC_memCompareEqualWithDFA(PM8 Des, PM8 Src, u16 length)
{
	ecpSecCmp_secParam_t	X_A			cmpSecParam;
	u16						X_A			invertCompareResult;
	memref  	X_A   	src;
	memref  	X_A   	des;
	u16 		X_A		len;
	u16 		X_A		compareResult;
	u32			X_A		xorResult;
	u32			X_A		minusResult;
	u16 		X_A		signaturebackup;
	u16 		X_A		signature;
	u16 		X_A		i;
	u16 		X_A		j;

	u32			X_A		srcTmpi;
	u32			X_A		desTmpi;

	u32			X_A		srcTmpj;
	u32			X_A		desTmpj;

	
	PATCH_BOOT_SECUREOPERATION_001();
	
	ecpMem_SecureCompareParamInit(&cmpSecParam, Des, Src, &invertCompareResult, length);

	des 				= (memref)cmpSecParam.ecpSecCmpDes;
	src 				= (memref)cmpSecParam.ecpSecCmpSrc;
	length 				= cmpSecParam.ecpSecCmpLength;
	xorResult 			= XOR_INIT_EQU;
	minusResult 		= MINUS_INIT_EQU;
	compareResult 		= ECP_CMP_RESULT_NOT_EQUAL;
	signature 			= cmpSecParam.ecpSecCmpFuncSignature;
	signaturebackup 	= signature;
	
	i 					= 0;
	j 					= 0;

	len = length & 0x03; //len = length % 4;
	if (len != 0x00)
	{
		for (; i<len; i++,j++)
		{
			srcTmpi = memReadU8(MEMtPM8(U32_A_U16(src,i)));
			srcTmpj = memReadU8(MEMtPM8(U32_A_U16(src,j)));
			
			desTmpi = memReadU8(MEMtPM8(U32_A_U16(des,i)));
			desTmpj = memReadU8(MEMtPM8(U32_A_U16(des,j)));
			
 			xorResult 		= U32_AND_U32(xorResult,U32_INV(U32_XOR_U32(desTmpi , srcTmpi)));
			minusResult 	= U32_OR_U32(U32_S_U32(desTmpj, srcTmpj), minusResult);
		}
	}
	
	
	if (length >= 4)
	{
		j = length - 4;
		do
		{
			srcTmpi = memReadU32(MEMtPM8(U32_A_U16(src,i)));
			srcTmpj = memReadU32(MEMtPM8(U32_A_U16(src,j)));
			
			desTmpi = memReadU32(MEMtPM8(U32_A_U16(des,i)));
			desTmpj = memReadU32(MEMtPM8(U32_A_U16(des,j)));
			
			xorResult 		= U32_AND_U32(xorResult,U32_INV(U32_XOR_U32(desTmpi , srcTmpi)));
			minusResult 	= U32_OR_U32(U32_S_U32(desTmpj, srcTmpj),minusResult);
			
			i = U16_A_U8(i, 4);
			j = j - 4;
		}while( (i != (length )) );
	}
	//check comparison is not skipped
	if(i == length)
	{
		signature 		+= ECP_COMPARE_FINISH;
	}

	//double check comparison result
	if((xorResult == XOR_INIT_EQU && minusResult == MINUS_INIT_EQU)
	   || (xorResult != XOR_INIT_EQU && minusResult != MINUS_INIT_EQU)
	  )
	 {
	 	signature 		+= ECP_COMPARE_CHECK;
	 }
	 else
	 {
		 faultDetected();
	 }

	 //check all the above steps performed
	 if(signature == U16_A_U16(U16_A_U16(signaturebackup ,ECP_COMPARE_FINISH), ECP_COMPARE_CHECK))
	 {
	 	if(xorResult == XOR_INIT_EQU)
	 	{
	 		PX_W_U16(cmpSecParam.ecpSecCmpInvertResult, U16_INV(ECP_CMP_RESULT_EQUAL));
	 	}
	 	else
	 	{
	 		PX_W_U16(cmpSecParam.ecpSecCmpInvertResult, U16_INV(ECP_CMP_RESULT_NOT_EQUAL));
	 	}

	 	signature 		+= ECP_RETURN_INVERT_COMPARE_RESULT;

	 	if(MINUS_INIT_EQU == minusResult)
	 	{
	 		compareResult = ECP_CMP_RESULT_EQUAL;
	 	}
	 	else
	 	{
			compareResult = ECP_CMP_RESULT_NOT_EQUAL;
	 	}
	 	signature 		+= ECP_RETURN_COMPARE_RESULT;
	 }
	 else
	 {
		 faultDetected();
	 }
	 
	 //check parameter is not modified
	if((cmpSecParam.ecpSecCmpParamChecksum != ecp_calSecCmpParamChecksum(&cmpSecParam))
	  || (cmpSecParam.ecpSecCmpSrc != MEMtPM8(src)) || (length != cmpSecParam.ecpSecCmpLength))
	{
		faultDetected();
	}
	
	cmpSecParam.ecpSecCmpFuncSignature = signature;

	if (cmpSecParam.ecpSecCmpFuncSignature != ECP_SECURE_COMPARE_PERFORM)
	{
		faultDetected();
	}
	
	compareResult		= SC_StateCheck(compareResult, PX_R_U16((PX8)&invertCompareResult), ECP_SECURE_CHECK_INIT);

	return compareResult;
}


#endif //SEC_MEM_OPERATION



#if BOOT__SECURITY__KEY_DATA_SAFE
#define	SEC_DATA_MASKCODE									0xED

#define SEC_DATA_AREA_BASE    								BOOT_CONFIG_KEY_BASE

#define	SET_KEY												0x00
#define	GET_KEY												0x01

#define	SEC_DATA_BLOCK_OK									0
#define SEC_DATA_BLOCK_ILLEGAL								1

#define GET_SEC_DATA_LEN_ADDR(a)							(a)
#define	GET_SEC_DATA_CHECK_VALUE_ADDR(a)					PN8_A_U8(a, SEC_DATA_LEN_SIZE)
#define GET_SEC_DATA_VALUE_ADDR(a)							PN8_A_U8(GET_SEC_DATA_CHECK_VALUE_ADDR(a), SEC_DATA_CHECK_VALUE_SIZE)

#define SecKeyBlock_GetLength(a)							PN_R_U16(GET_SEC_DATA_LEN_ADDR(a))
#define SecKeyBlock_GetCheckValue(a)						PN_R_U8(GET_SEC_DATA_CHECK_VALUE_ADDR(a))

#define SecKeyBlock_SetLength(a, v)							memWriteU16NoAtomic(GET_SEC_DATA_LEN_ADDR(a), v)
#define SecKeyBlock_SetCheckValue(a, v)						memWriteU8NoAtomic(GET_SEC_DATA_CHECK_VALUE_ADDR(a), v)

u8 SO_secSetGetImpl(PX8 dataBuf, u16 dataOffset, u16 dataLength, PM8 secDataArea, u8 opeType)
{
	u8		result;
	PM8		srcDataBuf;
	PM8		desDataBuf;
	PX8		calDataCheckValueBuf;
	u8		dataBlockCheckValue;
	u8		newDataCheckValue;
	u8		maskDataValue;
	u16		i;

	result = 0;
	if(SET_KEY == opeType)
	{
		srcDataBuf				= MEMtPM8(PX8_A_U16(dataBuf, dataOffset));
		desDataBuf				= MEMtPM8(GET_SEC_DATA_VALUE_ADDR(secDataArea));
		calDataCheckValueBuf	= srcDataBuf;
	}
	else
	{
		//dataLength				= SecKeyBlock_GetLength(secDataArea);
		srcDataBuf				= MEMtPM8(GET_SEC_DATA_VALUE_ADDR(secDataArea));
		desDataBuf				= MEMtPM8(PX8_A_U16(dataBuf, dataOffset));
		calDataCheckValueBuf	= desDataBuf;
	}

	//encrypt when store data, decrypt when get data
	for(i = 0; i < dataLength; U16_INC(i))
	{
		maskDataValue		= U8_XOR_U8(PM_R_U8(PM8_A_U16(srcDataBuf, i)), SEC_DATA_MASKCODE);
		memWriteU8Atomic(PM8_A_U16(desDataBuf, i), maskDataValue);
	}
	//get data check value
	newDataCheckValue		= 0;
	for(i = 0; i < dataLength; U16_INC(i))
	{
		newDataCheckValue 	= U8_XOR_U8(newDataCheckValue, PX_R_U8(calDataCheckValueBuf));
		PX8_INC(calDataCheckValueBuf);
	}

	if(SET_KEY == opeType)
	{
		//SecKeyBlock_SetLength(secDataArea, dataLength);
		SecKeyBlock_SetCheckValue(secDataArea, newDataCheckValue);
	}
	else
	{
		dataBlockCheckValue	= SecKeyBlock_GetCheckValue(secDataArea);
		if(dataBlockCheckValue != newDataCheckValue)
		{
			result	= SEC_DATA_BLOCK_ILLEGAL;
		}
	}

	return result;
}

u16 SO_calSecDataAreaLen(u16 dataLen)
{
	u16		len;

	len		= U32t16(U16_A_U16(SEC_DATA_HEADER_SIZE, dataLen));

	return len;
}

u16 SO_storeSecData(PX8 inBuffer, u8 inBufferOffset, u8 datalength, PM8 secDataArea)
{
	u16 secDataAreaRef;

	secDataAreaRef = 0;

	if(MEM_NULL == secDataArea)
	{
		secDataAreaRef	= NvmManager_Allocate(U16t32(SO_calSecDataAreaLen(U8t16(datalength))), MAP_ENTRY_BLOCK_PCK_ID);
		secDataArea		= MEMtPM8(MapTable_GetEntityContent(secDataAreaRef));
	}

	SO_secSetGetImpl(inBuffer , inBufferOffset, datalength, secDataArea, SET_KEY);

	return secDataAreaRef;
}

u8 SO_getSecDataByAddr(PX8 outBuffer, u8 outBufferOffset, u8 dataLength, PM8 secDataArea)
{
	return SO_secSetGetImpl(outBuffer , outBufferOffset, dataLength, secDataArea, GET_KEY);
}

u8 SO_getSecDataByRef(PX8 outBuffer, u8 outBufferOffset, u8 dataLength, u16 ref)
{
	return SO_getSecDataByAddr(outBuffer, outBufferOffset, dataLength, MEMtPM8(MapTable_GetEntityContent(ref)));
}
#endif //BOOT__SECURITY__KEY_DATA_SAFE


