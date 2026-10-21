
#include "crypto_impl.h"
#include "Crypto_Constant.h"
#include "Crypto_API.h"
#include "stack.h"
#include "ob_manager.h"
#include "object.h"
#include "UtilMethod.h"
#include "SysConfigTarget.h"
#include "memory.h"
#include "profile.h"
#include "SecureOperation.h"
#include "halCryptoConfig.h"
#include "Crypto_common_API.h"
#include "MemGetReuseRam.h"
#include "RNG_API.h"
#include "RSA_API.h"
#include "Sym_API.h"
#include "EllipseCurve_API.h"
#include "HASH_API.h"

#if !EMV_OPTIMIZATION_CODE
WEAK PM8 getArrayDataWithCheck(u16 outLen)
{
	PM8		D_A		outBuff;
	u16		D_A		outOffset;
	
	outOffset = popShort();
	outBuff = MEMtPM8( locateObjectInMemory( popReference() ) );
	
#if JCVM__VM__NULLOBJ_CHECK
    check_null((PF8)outBuff);
#endif
	
	check_bounds_interval((PF8)outBuff, outOffset, outLen);
	
	outBuff = PM8_A_U16(MEMtPM8(get_array_data((PF8)outBuff)), outOffset);
	
	return outBuff;
}
#endif //!EMV_OPTIMIZATION_CODE

PM8 getArrayDataWithoutCheck(void)
{
	PM8		D_A		outBuff;
	u16		D_A		outOffset;
	
	outOffset = popShort();
	outBuff = MEMtPM8( locateObjectInMemory( popReference() ) );
	
	outBuff = PM8_A_U16(MEMtPM8(get_array_data((PF8)outBuff)), outOffset);
	
	return outBuff;
}

PM8 getJavaObjectDataArray(void)
{
	return MEMtPM8( get_array_data( (PF8)MEMtPM8( locateObjectInMemory( popReference() ) ) ) );
}

#if !EMV_OPTIMIZATION_CODE
WEAK PM8 getJavaObjectDataRef(void)
{
	return (PM8)MEMtPM8( locateObjectInMemory( popReference() ) );
}
#endif //!EMV_OPTIMIZATION_CODE

PM8 GetArrayDataAndCheckBounds(PM8 inBuff, u16 inOffset, u16 inLen)
{
#if JCVM__VM__NULLOBJ_CHECK
    check_null((PF8)inBuff);
#endif
	
	check_bounds_interval((PF8)inBuff, inOffset, inLen);
	
	inBuff 			= PM8_A_U16(MEMtPM8(get_array_data((PF8)inBuff)), inOffset);
	
	return inBuff;
}
#if VM_WITH_RANDOMDATA
//---------------------------------------------
// lzm (2010-05-20)
//---------------------------------------------
void genRandomData(void)
{
#define ONE_TIME_RNG_GEN_NUM	0x08
u32		X_A		tmpBuff[ONE_TIME_RNG_GEN_NUM >> 2];
	PM8		D_A		outBuff;
//	PM8		D_A		seedBuff;
	PM8		X_A		outptr;
	u16		D_A		offset;
	u16		D_A		len;
	u8		D_A		mode;
	u8		X_A		executControl = 0;
	PATCH_JCVM_VM_CRYPTO_IMPL_001();	
	mode			= U16t8(popShort());  //(lzm modified according [MISA])
/*	seedBuff		= */MEMtPM8(locateObjectInMemory(popReference()));
	len 			= popShort();
	offset 			= popShort();
	outBuff 		= MEMtPM8(locateObjectInMemory(popReference()));

#if JCVM__VM__NULLOBJ_CHECK
	check_null((PF8)outBuff);
#endif

	check_bounds_interval((PF8)outBuff, offset, len);

	outptr 			= MEMtPM8(get_array_data((PF8)outBuff));

	if(0 == len)
	{
		executControl = 0xA5;
	}
	
	while (len > ONE_TIME_RNG_GEN_NUM)
	{
		SC_FUNCTION_EXC_INIT_STATUS();
		Random_number_generator((PX8)tmpBuff, ONE_TIME_RNG_GEN_NUM, mode);
		SC_FUNCTION_EXC_CHECK_STATUS();
		memCopyNoAtomic(PM8_A_U16(outptr, offset), MEMtPM8(tmpBuff), ONE_TIME_RNG_GEN_NUM);
		offset 		= U8_A_U16(ONE_TIME_RNG_GEN_NUM, offset);
		len 		= U16_S_U8(len , ONE_TIME_RNG_GEN_NUM);
		executControl = 0xA5;
	}

	if (len > 0)
	{
		SC_FUNCTION_EXC_INIT_STATUS();
		Random_number_generator((PX8)tmpBuff, (u8)len, mode);
		SC_FUNCTION_EXC_CHECK_STATUS();
		memCopyNoAtomic(PM8_A_U16(outptr, offset), MEMtPM8(tmpBuff), len);
		executControl = 0xA5;
	}

	if (executControl != 0xA5)
	{
		faultDetected();
	}
		
	pushShort(len);
}
#endif  // WITH_RANDOMDATA

#if VM_WITH_CRC
//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
void CRCInit(void)
{
	PM8		X_A		crcBuff;
	PM8		D_A		initialBuff;
	PM8		D_A		processState;
	u16		D_A		inLen;
	u8		D_A		algType;
	PATCH_JCVM_VM_CRYPTO_IMPL_002();		
	processState 	= getJavaObjectDataArray();
	
	crcBuff 		= getJavaObjectDataArray();
	
	inLen 			= popShort();
	
	initialBuff 	= getArrayDataWithCheck(inLen);
	
	algType 		= memReadU8(crcBuff);
	PM8_INC(crcBuff);
	
	if (ALG_ISO3309_CRC16 == algType)
	{
		if (0x02 != inLen)
		{
			pushShort(CRC_PROCESS_FAIL);
			return;
		}
	}
#if WITH_CRC_32
	else  // ALG_ISO3309_CRC32
	{
		if (0x04 != inLen)
		{
			pushShort(CRC_PROCESS_FAIL);
			return;
		}
	}
#endif //WITH_CRC_32	
	memCopyNoAtomic(crcBuff, initialBuff, inLen);
	memWriteU8NoAtomic(processState, CRC_INIT);
	pushShort(CRC_PROCESS_OK);
}

//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------

/*******************************************************************************
 * Function name: CRCUpdateAlg
 * Description: update CRC value
 * Input Parameters:  (precondition)
 *	PM8 inBuff	    address of buffer
 *	u16 inLen           length of buffer
 *      u8 algType	    ALG_ISO3309_CRC16 or ALG_ISO3309_CRC32
 *      u8 crcState         processing state
 *      PM8 crcBuff    initail value buffer
 * Output Parameters:  (postcondition)
 *		Checksum result
 * NOTE: none
 ********************************************************************************/
void CRCCalAlg(PM8 inBuff, u16 inLen, u8 algType, u8 crcState, PM8 crcBuff, PM8 outBuff)	
{
	u32		X_A	checkSum;
	u8		X_A mode;
	PATCH_JCVM_VM_CRYPTO_IMPL_003();		
	checkSum = 0x00;
	if(OS_JC2_MODE == OS_CONF3_JC2_JC3_MODE)
	{
		mode = CRC_MODE_JC2;
	}
	else
	{
		mode = CRC_MODE_JC3;
	}
	switch(algType)
	{
	case ALG_ISO3309_CRC16:
		if (CRC_CONSTRUCTED == crcState)
		{
			checkSum = 0x00000000;
		}
		else if (CRC_INIT == crcState)
		{
			checkSum = reflectShort(memReadU16(crcBuff));//修改处，添加reflectShort函数;
		}
		else  // CRC_UPDATE
		{
			checkSum = U16_XOR_U16(reflectShort(memReadU16(crcBuff)), 0xFFFF);
		}
		break;
#if WITH_CRC_32
	case ALG_ISO3309_CRC32:
		if (CRC_CONSTRUCTED == crcState)
		{
			checkSum = 0x00000000;
		}
		else if (CRC_INIT == crcState)
		{
			checkSum = reflectLong(memReadU32(crcBuff));
		}
		else  // CRC_UPDATE
		{
			if(CRC_MODE_JC2 == mode)
			{
				checkSum = U32_XOR_U32(reflectLong(memReadU32(crcBuff)), 0xFFFFFFFF);
			}
			else
			{
				checkSum = U32_XOR_U32(memReadU32(crcBuff), 0xFFFFFFFF);
			}
		}
		break;
#endif //WITH_CRC_32
	default:
		break;
	}
	
	if (isInE2P(inBuff))
	{
		memRefresh();
	}
	checkSum = CRCDoCal(inBuff, inLen, checkSum, algType, mode);

	if (ALG_ISO3309_CRC16 == algType)
	{
		memWriteU16NoAtomic(outBuff, U32t16(checkSum));
	}
#if WITH_CRC_32	
	else
	{
		memWriteU32NoAtomic(outBuff, checkSum);
	}
#endif //WITH_CRC_32	
	return;
}

void CRCUpdate(void)
{
	PM8		D_A		inBuff;
	PM8		D_A		crcBuff;
	PM8		D_A		processState;
	u16		D_A		inLen;
	u8		D_A		algType;
	u8		D_A		crcState;
	PATCH_JCVM_VM_CRYPTO_IMPL_004();	
	processState 	= getJavaObjectDataArray();
	crcBuff 		= getJavaObjectDataArray();
	inLen 			= popShort();
	inBuff			= getArrayDataWithCheck(inLen);

	crcState 		= memReadU8(processState);
	algType 		= memReadU8(crcBuff);
	PM8_INC(crcBuff);

	CRCCalAlg(inBuff, inLen, algType, crcState, crcBuff, crcBuff);

	memWriteU8NoAtomic(processState, CRC_UPDATE);
}

//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
void CRCDofinal(void)
{
	PM8		D_A		inBuff;
	PM8		D_A		outBuff;
	PM8		D_A		crcBuff;
	PM8		D_A		processState;
	u16		D_A		inLen;
	u8		D_A		retLen;
	u8		D_A		algType;
	u8		D_A		crcState;
	PATCH_JCVM_VM_CRYPTO_IMPL_005();	
	
	processState 	= getJavaObjectDataArray();
	
	crcBuff 		= getJavaObjectDataArray();
	algType 		= memReadU8(crcBuff);
	PM8_INC(crcBuff);
	
	if (ALG_ISO3309_CRC16 == algType)
	{
		retLen = 2;
	}
#if WITH_CRC_32	
	else
	{
		retLen = 4;
	}
#endif //WITH_CRC_32	
	outBuff 		= getArrayDataWithCheck(retLen);
	
	inLen 			= popShort();
	inBuff 			= getArrayDataWithCheck(inLen);

	crcState 		= memReadU8(processState);
	CRCCalAlg(inBuff, inLen, algType, crcState, crcBuff, outBuff);

	memWriteU8NoAtomic(processState, CRC_CONSTRUCTED);

	pushShort(retLen);
}
#endif  // WITH_CRC

#if VM_WITH_CHECKSUM
//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
void setKeyChecksum(void)
{
	PM8				keyBuff;
	u16 	X_A 	keyLen;

	keyLen 			= popShort();
	keyBuff 		= getJavaObjectDataRef();

	keyBuff 		= MEMtPM8(get_array_data((PF8)keyBuff));
	setKeyChecksumAlg(keyBuff, keyLen);
}
#endif  //WITH_CHECKSUM

#if VM_WITH_SYM
//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
void CipherUpdate(u8 symAlg)		SMALL_MODE
{
	PM8				inBuff;
	PM8				outBuff;
	PM8				tmpBuff;
	PM8				ICVBuff;
	PM8				keyBuff;
	PM8				processState;
	u16		D_A		inLen;
	u16		D_A		retLen;
	u16		D_A		keyBitLen;
	u16		D_A		outOffset;
	u16		D_A		byteNumInMultiBlockMask;
	u8		D_A		sbcState;
	u8		D_A		mode;
	PATCH_JCVM_VM_CRYPTO_IMPL_006();		
	processState 	= getJavaObjectDataArray();
	sbcState 		= memReadU8(processState);
	
	tmpBuff 		= getJavaObjectDataArray();
	PM8_INC(tmpBuff);
	mode 			= memReadU8(tmpBuff);
	PM8_INC(tmpBuff);
	
	ICVBuff 		= getJavaObjectDataRef();
	keyBitLen 		= popShort();
	keyBuff 		= getJavaObjectDataArray();
	
	outOffset 		= popShort();
	outBuff 		= getJavaObjectDataRef();
	
	inLen 			= popShort();
	inBuff 			= getArrayDataWithCheck(inLen);
	
	byteNumInMultiBlockMask = U16_INV(U16_S_U16(getSYMBlockSize(symAlg), 1));
	
	retLen 			= inLen;
	if (SBC_UPDATE == sbcState)
	{
		retLen 		+= (mode >> 4);
	}

	retLen 			= retLen & byteNumInMultiBlockMask;
	outBuff			= GetArrayDataAndCheckBounds(outBuff, outOffset, retLen);
	
	if (0x00 == inLen)
	{
		pushShort(0);
		return;
	}

	if (MEM_NULL != ICVBuff)
	{
		ICVBuff 		= MEMtPM8(get_array_data((PF8)ICVBuff));
	}

	if (isInE2P(keyBuff))
	{
		memRefresh();
	}

	SymCipherUpdate(inBuff, inLen, outBuff, keyBuff, keyBitLen, ICVBuff, tmpBuff, mode, sbcState, 0x00, symAlg);

	Sym_Wipekey();
	if ((SBC_INIT == sbcState) /*&& (0 != inLen)*/)
	{
		memWriteU8NoAtomic(processState, SBC_UPDATE);
	}
	pushShort(retLen);
}

//---------------------------------------------
// lzm (2009-10-18)
// modify by liweiming@20210303 
// 根据“国密API-电信log脚本"
// 1.对SM2加长了加解密Buffer长度，支持到1K数据。
// 2.调用CipherDofinal和SignatureDofinal之后，修改状态为INIT提前，避免抛异常时无法修改。
//---------------------------------------------
void CipherDofinal(u8 symAlg)		SMALL_MODE
{
	PM8		D_A		inBuff;
	PM8		X_A		outBuff;
	PM8		D_A		ICVBuff;
	PM8		D_A		keyBuff;
	PM8		D_A		tmpBuff;
	PM8		D_A		processState;
	u16		D_A		inLen;
	u16		D_A		retLen;
	u16		D_A		outOffset;
	u16		D_A		keyBitLen;
	u16		D_A		totalLength;
	u8		D_A		mode;
	u8		D_A		algType;
	u8		D_A		sbcState;
	u8		D_A		blockSize;
	u8		D_A		blockSizeMask;
	u8 		D_A		algTypeMode;
	PATCH_JCVM_VM_CRYPTO_IMPL_007();	
	
	processState 	= getJavaObjectDataArray();
	tmpBuff 		= getJavaObjectDataArray();
	ICVBuff 		= getJavaObjectDataRef();
	
	keyBitLen 		= popShort();
	keyBuff 		= getJavaObjectDataArray();
	
	outOffset 		= popShort();
	outBuff 		= getJavaObjectDataRef();
	
	inLen 			= popShort();
	inBuff 			= getArrayDataWithCheck(inLen);
	
	sbcState 		= memReadU8(processState);
	algType 		= memReadU8(tmpBuff);
	PM8_INC(tmpBuff);
	mode 			= memReadU8(tmpBuff);
	PM8_INC(tmpBuff);

	blockSize 		= getSYMBlockSize(symAlg);
	blockSizeMask 	= (blockSize - 1);
	algTypeMode 	= getSYMalgType(symAlg, algType, CIPHER_MODE);
	memWriteU8NoAtomic(processState, SBC_INIT);//if dofinal,processState need to init

	if (MEM_NULL != ICVBuff)		 //此处SM4复用DES的
	{
		ICVBuff 	= MEMtPM8(get_array_data((PF8)ICVBuff));
	}
	totalLength 	= inLen;
	if (SBC_UPDATE == sbcState)	 // have done update
	{
		totalLength += (mode >> 4);
	}
	retLen			= totalLength;
	check_null(outBuff);//first check buffer is null,second check length

#if WITH_AEAD
	if( (SYM_MODE_AEAD_GCM == algTypeMode) || (SYM_MODE_AEAD_CCM == algTypeMode))
	{
		;
	}
	else
#endif
	{
		if( (SYM_MODE_NOPAD == algTypeMode) || (mode & MODE_DECRYPT))
		{
			if (totalLength & blockSizeMask)
			{
				if (PMtMEM(ICVBuff) != REF_NULL)
				{
					ecpMem_Set(ICVBuff, 0, blockSize);
				}
				throw_error(CRYPTOEXCEPT_ILLEGAL_USE);
			}
			if (SBC_UPDATE != sbcState)	 // have done update
			{
				if(0x00 == totalLength)
				{
					if (PMtMEM(ICVBuff) != REF_NULL)
					{
						ecpMem_Set(ICVBuff, 0, blockSize);
					}
					throw_error(CRYPTOEXCEPT_ILLEGAL_USE);
				}
			}
		}
		else if( (SYM_MODE_NONE == algTypeMode) || (totalLength & blockSizeMask) )
		{
			retLen = totalLength + (blockSize - (totalLength & blockSizeMask));
		}
		else if(SYM_MODE_ISO9797_M1 == algTypeMode)
		{
			if (SBC_UPDATE != sbcState)	 // have done update
			{
				if(0x00 == totalLength)
				{
					retLen = blockSize;
				}
			}
		}
	}

	outBuff			= GetArrayDataAndCheckBounds(outBuff, outOffset, retLen);

	if (isInE2P(keyBuff))
	{
		memRefresh();
	}

	inLen = SymCipherDofinal(inBuff, inLen, outBuff, keyBuff, keyBitLen, (PX8)ICVBuff, tmpBuff, algType, mode, sbcState, symAlg);

	Sym_Wipekey();
	if (PMtMEM(ICVBuff) != REF_NULL)
	{
#if WITH_AEAD
		if((SYM_MODE_AEAD_GCM != algTypeMode) && (SYM_MODE_AEAD_CCM != algTypeMode))
#endif //modify by huangzehua@20220819 AEAD 开关关闭时，测试AES脚本项目cbc iv=0时，IV没有初始化导致结果出错，需要初始化IV 数组
		{
			ecpMem_Set(ICVBuff, 0, blockSize);
		}
	}

	if (mode & MODE_DECRYPT)
	{
		if (inLen == WRONG_PADDING)
		{
			throw_error(CRYPTOEXCEPT_ILLEGAL_USE);
			return;
		}
		else
		{
			pushShort(retLen - inLen);
			return;
		}
	}
	else if(inLen != SBC_ENCRYPT_OK)
	{
		faultDetected();
	}
	pushShort(retLen);
}

//---------------------------------------------
// lzm (2009-10-18)
// The bit meaning of 'flag' is structed as follows:
// - - - -  x 0 0 1 DES algorithm
// - - - -  x 0 1 0 AES algorithm
// - - - -  x 1 0 0 KoreanSeed algorithm
// - - - -  0 x x x sign mode
// - - - -  1 x x x verify mode
//---------------------------------------------
void SignatureUpdate(u8 symAlg)		SMALL_MODE
{
	PM8				inBuff;
	PM8				tmpBuff;
	PM8				ICVBuff;
	PM8				keyBuff;
	PM8				processState;
	u16		D_A		inLen;
	u16		D_A		keyBitLen;
	u8		D_A		mode;
	u8		D_A		algType;
	u8		D_A		sbcState;
	PATCH_JCVM_VM_CRYPTO_IMPL_008();	
	processState 	= getJavaObjectDataArray();
	tmpBuff 		= getJavaObjectDataArray();
	ICVBuff 		= getJavaObjectDataArray();
	keyBitLen 		= popShort();
	keyBuff 		= getJavaObjectDataArray();
	inLen 			= popShort();
	inBuff			= getArrayDataWithCheck(inLen);

	if (0x00 == inLen)
	{
		return;
	}
	sbcState 		= memReadU8(processState);
	
	algType 		= memReadU8(tmpBuff);
	PM8_INC(tmpBuff);
	mode 			= memReadU8(tmpBuff);
	PM8_INC(tmpBuff);

	if (isInE2P(keyBuff))
	{
		memRefresh();
	}

	SymSignatureUpdate(inBuff, inLen, keyBuff, keyBitLen, (PX8)ICVBuff, tmpBuff, algType, mode, sbcState, symAlg);

	Sym_Wipekey();
	if ((SBC_INIT == sbcState) /*&& (inLen != 0)*/)
	{
		memWriteU8NoAtomic(processState, SBC_UPDATE);
	}
	incrementCodeFlowControlFlag();
}

//---------------------------------------------
// lzm (2009-10-18)
// The bit meaning of 'flag' is structed as follows:
// - - - -  x 0 0 1 DES algorithm
// - - - -  x 0 1 0 AES algorithm
// - - - -  x 1 0 0 KoreanSeed algorithm
// - - - -  0 x x x sign mode
// - - - -  1 x x x verify mode
// modify by liweiming@20210303 
// 根据“国密API-电信log脚本"
// 1.对SM2加长了加解密Buffer长度，支持到1K数据。
// 2.调用CipherDofinal和SignatureDofinal之后，修改状态为INIT提前，避免抛异常时无法修改。
//---------------------------------------------
void SignatureDofinal(u8 symAlg)		SMALL_MODE
{
	PM8				inBuff;
	PM8				outBuff;
	PM8				ICVBuff;
	PM8				keyBuff;
	PM8				tmpBuff;
	PM8				processState;
	u16		D_A		inLen;
	u16 	D_A		totalLength;
	u16		D_A		retLen;
	u16		D_A		outOffset;
	u16		D_A		keyBitLen;
	u8		D_A		signLen;
	u8		D_A		sbcState;
	u8		I_A		mode;
	u8		I_A		algType;
	u8		I_A		algTypeMode;
	u8		I_A		blockSize;
	u8		I_A		blockSizeMask;
	PATCH_JCVM_VM_CRYPTO_IMPL_009();	
	processState 	= getJavaObjectDataArray();
	tmpBuff 		= getJavaObjectDataArray();
	ICVBuff 		= getJavaObjectDataArray();
	keyBitLen 		= popShort();
	keyBuff 		= getJavaObjectDataArray();

	signLen 		= 0;
	if (symAlg & SBC_VERIFY)
	{
		signLen 	= U16t8(popShort());
	}

	outOffset 		= popShort();
	outBuff 		= getJavaObjectDataRef();
	
	inLen 			= popShort();
	inBuff			= getArrayDataWithCheck(inLen);

	sbcState 		= memReadU8(processState);
	algType 		= memReadU8(tmpBuff);
	PM8_INC(tmpBuff);
	mode 			= memReadU8(tmpBuff);
	PM8_INC(tmpBuff);


	totalLength 	= inLen;
	if (SBC_UPDATE == sbcState)	 // have done update
	{
		totalLength += (mode >> 4);
	}

	blockSize 		= getSYMBlockSize(symAlg);
	blockSizeMask 	= (blockSize - 1);

	retLen			= getSYMSinReturnLength(symAlg, algType, totalLength);

	outBuff			= GetArrayDataAndCheckBounds(outBuff, outOffset, retLen);

	algTypeMode 	= getSYMalgType(symAlg, algType, SIGNATURE_MODE);

	memWriteU8NoAtomic(processState, SBC_INIT);//if dofinal,processState need to init

	if( (SYM_MODE_NOPAD == algTypeMode) )
	{
		if (totalLength & blockSizeMask)
		{
			ecpMem_Set(ICVBuff, 0, blockSize);
			throw_error(CRYPTOEXCEPT_ILLEGAL_USE);
		}
		if (SBC_UPDATE != sbcState)	 // have done update
		{
			if(0x00 == totalLength)
			{
				ecpMem_Set(ICVBuff, 0, blockSize);
				throw_error(CRYPTOEXCEPT_ILLEGAL_USE);
			}
		}
	}
	if (symAlg & SBC_VERIFY)
	{
		if (signLen != retLen)
		{ // signature length is not correct, then return false.
			ecpMem_Set(ICVBuff, 0, blockSize);
			pushShort(SBC_VERIFY_FAIL);
			return;
		}
	}
	
	if (isInE2P(keyBuff))
	{
		memRefresh();
	}
	if(SYM_MODE_CMAC == algTypeMode)
	{
		if (symAlg & SBC_VERIFY)
		{
			retLen 	= SymCMACDofinal(inBuff, inLen, outBuff, keyBuff, keyBitLen, (PX8)ICVBuff, tmpBuff, algType, mode, sbcState, U16t8(retLen), symAlg);
		}
		else
		{
			SymCMACDofinal(inBuff, inLen, outBuff, keyBuff, keyBitLen, (PX8)ICVBuff, tmpBuff, algType, mode, sbcState, U16t8(retLen), symAlg);
		}
	}
	else
	{
		if (symAlg & SBC_VERIFY)
		{
			retLen 	= SymSignatureDofinal(inBuff, inLen, outBuff, keyBuff, keyBitLen, (PX8)ICVBuff, tmpBuff, algType, mode, sbcState, U16t8(retLen), symAlg);
		}
		else
		{
			SymSignatureDofinal(inBuff, inLen, outBuff, keyBuff, keyBitLen, (PX8)ICVBuff, tmpBuff, algType, mode, sbcState, U16t8(retLen), symAlg);
		}
	}
	Sym_Wipekey();

	ecpMem_Set(ICVBuff, 0, blockSize);
	incrementCodeFlowControlFlag();

	pushShort(retLen);
}


void secureStoreSensitiveData(void)
{
	u16     X_A     keyLen;
	PM8		X_A		keyDate;
	u16     X_A     inOffset;
	PM8		X_A		inBuff;

	keyLen					= popShort();
	keyDate					= getJavaObjectDataRef();
	
	inOffset				= popShort();
	inBuff					= getJavaObjectDataRef();

	SC_InOutSaveBack(inBuff, keyDate, inOffset, 0);
	
	inBuff					= GetArrayDataAndCheckBounds(inBuff, inOffset, keyLen);
	keyDate					= MEMtPM8(get_array_data((PF8)keyDate));
	
	SC_checkReturnCode(SC_SYMsetKey_Java(keyDate, inBuff, keyLen));
}

void secureStorePINData(void)
{
	u16     X_A     keyLen;
	u16     X_A     keyDesLen;
	PM8		X_A		keyDate;
	u16     X_A     inOffset;
	PM8		X_A		inBuff;
	PM8		X_A		tempBuff;

	tempBuff				= MEMtPM8(get_array_data(getJavaObjectDataRef()));
	keyDesLen				= popShort();
	keyDate					= getJavaObjectDataRef();

	inOffset				= popShort();
	inBuff					= getJavaObjectDataRef();


	SC_InOutSaveBack(inBuff, keyDate, inOffset, 0);
	
	inBuff					= GetArrayDataAndCheckBounds(inBuff, inOffset, keyDesLen);
	keyLen					= U16_S_U8(get_array_length(keyDate), 2);
	keyDate					= MEMtPM8(get_array_data((PF8)keyDate));

	SC_checkReturnCode(SC_storeSensitiveDataAPI(keyDate, keyLen, tempBuff, keyDesLen, inBuff));	
	memRefresh(); // 对 java decrementTriesRemaining() 的非原子操作及时提交
}

void secureGetSensitiveData(void)
{
	u16     X_A     keyLen;
	u16     X_A     outOffset;
	PM8		X_A		outBuff;
	PM8		X_A		keyDate;
	keyLen 			= popShort();
	outOffset 		= popShort();
	outBuff         = getJavaObjectDataRef();
	
	keyDate 	    = getJavaObjectDataRef();
	
	SC_InOutSaveBack(outBuff, keyDate, outOffset, 0);
	
	outBuff			= GetArrayDataAndCheckBounds(outBuff, outOffset, keyLen);
	keyDate 		= MEMtPM8(get_array_data((PF8)keyDate));
	
	SC_checkReturnCode(SC_SYMgetKey_Java(outBuff, keyDate, keyLen));
}

#if WITH_AEAD
//---------------------------------------------
/* jxh (2021-8-18)
* modify by dengyiwen@20221018: 不判断if (isInE2P(keyBuff))，直接执行memRefresh();
* modify by dengyiwen@20220927: 消除警告
* dengyiwen@20230111根据评审意见修改，数组边界需要检查
* modify by xuxiaojun@20230302: 比亚迪API中虽然说明了aadBuf为null时需要抛空指针异常，但是后续跟比亚迪的测试跟进时要求为null时忽略长度和偏移，算法继续执行，且这种方式也符合算法原理。
*                               对于JC305的标准API，传入时的数据是我们自己控制的,不会出现为null的情况。所以对于aadBuf的边界判断还原到EPX3.0.1,但是增加null时adataOffset与adataLen重置为0的操作，避免后续地址访问越界
*/
//---------------------------------------------
void AEADCipherInit(u8 symAlg)		SMALL_MODE
{
	PM8		D_A		nonceBuf;
	PM8		X_A		aadBuf;
	PM8		D_A		ICVBuff;
	PM8		D_A		keyBuff;
	PM8		D_A		tmpBuff;
	u16		D_A		nonceLen;
	u16		D_A		adataLen;
	u16		D_A		adataOffset;
	u16		D_A		messageLen;
	u16		D_A		tagSize;
	u16		D_A		keyBitLen;
	u8		D_A		mode;
	u8		D_A		algType;
	u8		D_A		blockSize;
	u8		D_A		blockSizeMask;
	u8 		D_A		algTypeMode;

	tmpBuff 		= getJavaObjectDataArray();
	ICVBuff 		= getJavaObjectDataRef();

	keyBitLen 		= popShort();
	keyBuff 		= getJavaObjectDataArray();

	tagSize 		= popShort();
	messageLen 		= popShort();
	adataLen 		= popShort();

	adataOffset		= popShort();
	aadBuf = MEMtPM8( locateObjectInMemory( popReference() ) );

	if (MEM_NULL != aadBuf)
	{
		check_bounds_interval((PF8)aadBuf, adataOffset, adataLen);

		aadBuf = PM8_A_U16(MEMtPM8(get_array_data((PF8)aadBuf)), adataOffset);
	}
	else
	{
		adataLen = 0x00;
	}

	nonceLen 		= popShort();
	nonceBuf 		= getArrayDataWithCheck(nonceLen);

	algType 		= memReadU8(tmpBuff);
	PM8_INC(tmpBuff);
	mode 			= memReadU8(tmpBuff);
	PM8_INC(tmpBuff);

	//blockSize 		= getSYMBlockSize(symAlg);		//not used

	if (MEM_NULL != ICVBuff)		 //此处SM4复用DES的
	{
		ICVBuff 	= MEMtPM8(get_array_data((PF8)ICVBuff));
	}

	memRefresh();

	nonceLen = SymCipherInit(nonceBuf, nonceLen, aadBuf, adataLen, messageLen, tagSize, keyBuff, keyBitLen, (PX8)ICVBuff, tmpBuff, algType, mode, symAlg);

	Sym_Wipekey();

	pushShort(nonceLen);
}


//---------------------------------------------
// jxh (2021-8-18)
// modify by dengyiwen@20220927: 消除警告
//---------------------------------------------
void AEADCipherTag(u8 symAlg)		SMALL_MODE
{
	PM8		D_A		tagBuf;
	PM8		D_A		ICVBuff;
	PM8		D_A		keyBuff;
	PM8		D_A		tmpBuff;
	u16		D_A		adataLen;
	u16		D_A		messageLen;
	u16		D_A		nonceLen;
	u16		D_A		tagSize;
	u16		D_A		keyBitLen;
	u8		D_A		mode;
	u8		D_A		algType;
	//u8		D_A		blockSize;

	tmpBuff 		= getJavaObjectDataArray();
	ICVBuff 		= getJavaObjectDataRef();

	keyBitLen 		= popShort();
	keyBuff 		= getJavaObjectDataArray();

	messageLen 		= popShort();
	nonceLen 		= popShort();
	adataLen 		= popShort();

	tagSize 		= popShort();
	tagBuf 			= getArrayDataWithCheck(tagSize);

	algType 		= memReadU8(tmpBuff);
	PM8_INC(tmpBuff);
	mode 			= memReadU8(tmpBuff);
	PM8_INC(tmpBuff);

	//blockSize 		= getSYMBlockSize(symAlg);		//not used

	if (MEM_NULL != ICVBuff)		 //此处SM4复用DES的
	{
		ICVBuff 	= MEMtPM8(get_array_data((PF8)ICVBuff));
	}


	if (isInE2P(keyBuff))
	{
		memRefresh();
	}
	tagSize = SymCipherTag(tagBuf, tagSize, adataLen, nonceLen, messageLen, keyBuff, keyBitLen, (PX8)ICVBuff, tmpBuff, algType, mode, symAlg);

	Sym_Wipekey();

	pushShort(tagSize);
}

#endif // WITH_AEAD
#endif // WITH_DES || WITH_AES

#if VM_WITH_RSA
//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
void storeRSAMEKeyRef(void)
{
	PM8				pbExp;
	PM8				pbModulusN;
	u16		X_A		wExpLength;
	u16		X_A		wModulusNLength;
	u8		X_A		bRSAKeyType;

	bRSAKeyType 	= U16t8(popShort());
	wExpLength 		= popShort();
	wModulusNLength	= popShort();
	pbModulusN 		= getJavaObjectDataArray();
	pbExp 			= PM8_A_U16(pbModulusN, wModulusNLength);

	RSA_storeMEKey(pbExp, pbModulusN, wExpLength, wModulusNLength, bRSAKeyType);
}

//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
void storeRSACrtKeyRef(void)
{
	PM8		D_A		pbParmP;
	PM8		D_A		pbParmQ;
	PM8		D_A		pbParmDP1;
	PM8		D_A		pbParmDQ1;
	PM8		D_A		pbParmInvQ;
	u16		D_A		wModulusNLength;
	u16		D_A		wPrimeLength; 

	/********************************************************************
	PL:PrimeLength(byte)
	-------------------------------------------------------------------------
	Member	|P		|Q		|Dp		|Dq		|InvQ	|CRC	|N		|E		|
	-------------------------------------------------------------------------
	Len		|PL		|PL		|PL		|PL		|PL		|2		|2PL	|4		|
	-------------------------------------------------------------------------
	********************************************************************/
	
	wModulusNLength = popShort() >> 3;
	wPrimeLength 	= popShort();
	pbParmP 		= getJavaObjectDataArray();
	pbParmQ 		= PM8_A_U16(pbParmP, wPrimeLength);
	pbParmDP1 		= PM8_A_U16(pbParmQ, wPrimeLength);
	pbParmDQ1 		= PM8_A_U16(pbParmDP1, wPrimeLength);
	pbParmInvQ 		= PM8_A_U16(pbParmDQ1, wPrimeLength);
	
	RSA_storeCRTKey(pbParmP, pbParmQ, pbParmDP1, pbParmDQ1, pbParmInvQ, wModulusNLength, wPrimeLength, TYPE_RSA_CRT_PRIVATE);
		// Clear cache
	memRefresh();
}

//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
void storeRSAKeyPair(void)
{
	PM8		D_A		pbParmP;
	PM8		D_A		pbParmQ;
	PM8		D_A		pbParmDP1;
	PM8		D_A		pbParmDQ1;
	PM8		D_A		pbParmInvQ;
	PM8		D_A		pbExp;
	PM8		D_A		pbModulusN;
	u16		D_A		wModulusNLength;
	u16		D_A		wPrimeLength;
	u16		D_A		wExpLength;
	u8		D_A		bRSAKeyType;
	PATCH_JCVM_VM_CRYPTO_IMPL_010();	
	bRSAKeyType = (u8)popShort();
	wModulusNLength = popShort();

	if (ALG_RSA_CRT == bRSAKeyType)
	{
		wPrimeLength 	= (u16)(wModulusNLength >> 1);
		pbParmP 		= getJavaObjectDataArray();
		pbParmQ 		= PM8_A_U16(pbParmP, wPrimeLength);
		pbParmDP1 		= PM8_A_U16(pbParmQ, wPrimeLength);
		pbParmDQ1 		= PM8_A_U16(pbParmDP1, wPrimeLength);
		pbParmInvQ 		= PM8_A_U16(pbParmDQ1, wPrimeLength);
		RSA_storeKEYGENCRTKey(pbParmP, pbParmQ, pbParmDP1, pbParmDQ1, pbParmInvQ, wPrimeLength);
	}
	else
	{
		// use g_RSAKeyStore.pbParmP to store the private key reference
		// the modulus address of private key
		pbParmP = getJavaObjectDataArray();
		pbParmQ = PM8_A_U16(pbParmP, wModulusNLength);
		RSA_storeKEYGENPriKey(pbParmP, pbParmQ, wModulusNLength);
	}

	wExpLength = popShort();

	pbModulusN 		= getJavaObjectDataArray();
	pbExp 			= PM8_A_U16(pbModulusN, wModulusNLength);

	RSA_storeMEKey(pbExp, pbModulusN, wExpLength, wModulusNLength, bRSAKeyType);
}

//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
void RSACipherUpdate(void)
{
	PM8		X_A		tmpSrcBuff;
	PM8		D_A		inBuff;
	u16		D_A		inLen;
	u16		D_A		accOffset;	//when EMV_OPTIMIZATION_CODE set 0, accOffset is 0
	u16		D_A		tmpMaxLen;
	u16		D_A		retStatus;
	u8		D_A		algType;
	u8		D_A		mode;		//when EMV_OPTIMIZATION_CODE set 0, mode is 0

	PATCH_JCVM_VM_CRYPTO_IMPL_012();
	mode 			= U16t8(popShort());
	algType 		= U16t8(popShort());
	tmpMaxLen 		= popShort();
	accOffset 		= popShort();
	tmpSrcBuff 		= getJavaObjectDataRef();
	
	inLen 			= popShort();
	inBuff 			= getArrayDataWithCheck(inLen);

	tmpSrcBuff 		= PM8_A_U16(MEMtPM8(get_array_data((PF8)tmpSrcBuff)), accOffset);
	retStatus       = RSA_CIPHER_ILLEGAL_USE;
	
	retStatus  		= RSA_CipherUpdate(inBuff, inLen, tmpSrcBuff, accOffset, tmpMaxLen, algType, mode);

	pushShort(retStatus);
}


//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
void RSACipherDofinal(void)
{
	PM8		D_A		inBuff;
	PM8		X_A		outBuff;
	PM8		D_A		tmpSrcBuff;
	PM8		D_A		processState;
	u16		D_A		inLen;
	u16		D_A		outOffset;
	u16		D_A		accOffset;
	u16		D_A		retLen;
	u8		D_A		mode;
	u8		D_A		algType;
	u8		D_A		rsaState;
	
	PATCH_JCVM_VM_CRYPTO_IMPL_013();

	processState	= getJavaObjectDataArray();
	mode			= U16t8(popShort());
	algType			= U16t8(popShort());
	accOffset		= popShort();
	tmpSrcBuff		= getJavaObjectDataRef();
	outOffset		= popShort();
	outBuff			= getJavaObjectDataRef();
	
	inLen			= popShort();
	inBuff			= getArrayDataWithCheck(inLen);

	rsaState 		= memReadU8(processState);


	check_null((PF8)outBuff);
	check_bounds(outBuff, outOffset, Header_ArrayType_Byte);
	if (RSA_INIT != rsaState)
	{
		tmpSrcBuff 	= MEMtPM8(get_array_data((PF8)tmpSrcBuff));
	}

	if (RSA_PROCESS_OK == RSA_CipherDofinal(inBuff, inLen, tmpSrcBuff, accOffset, algType, mode, rsaState))
	{
		retLen =  RSA_Get_OutDataLength();

		outBuff  = GetArrayDataAndCheckBounds(outBuff, outOffset, retLen);
		memCopyNoAtomic(outBuff, (PM8)RSA_Get_OutData(), retLen);
		pushShort(retLen);
	}
	else
	{
		pushShort(RSA_CIPHER_ILLEGAL_USE);
	}
}


//---------------------------------------------
// lzm (2009-10-18)
// modify by dengyiwen@20220722: 增加Java card3.0.5内容
// modify by xuxiaojun@20221020: 增加sigBuff的边界检测，从EPX3.0.1合并过来
//---------------------------------------------
void RSASignatureDofinal(u8 mode)		SMALL_MODE
{
	PM8				inBuff;
	PM8				sigBuff;
	u16		D_A		inLen;
	u16		D_A		sigLen;
	u16		D_A		retLen;
	u8		D_A		algType;
	u16     D_A     preHashFlag;
	PATCH_JCVM_VM_CRYPTO_IMPL_014();	
	algType 		= U16t8(popShort());
	sigLen 			= popShort();
	
	retLen 			= RSA_GetModuluLength();
	sigBuff 		= getArrayDataWithCheck(retLen);
	
	inLen 			= popShort();
	inBuff 			= getArrayDataWithCheck(inLen);
	preHashFlag     = popShort();

	if (RSA_VERIFY == mode)
	{
		if (sigLen != retLen)
		{
			pushShort(false);
			return;
		}
	}

	incrementCodeFlowControlFlag();
	pushShort(RSA_Signature(inBuff, inLen, sigBuff,  NULL, NULL, NULL, 0, sigLen, algType, mode,preHashFlag));
}

//---------------------------------------------
// lzm (2009-10-18)
// modify by dengyiwen@20220722: 增加Java card3.0.5内容
//---------------------------------------------
void RSASignatureUpdateFinal(u8 mode)		SMALL_MODE
{
	PM8				inBuff;
	PM8				sigBuff;
	PM8				mdBuff;
	PM8				tmpBuff;
	PM8				tmpSrcBuff;
	u16		D_A		inLen;
	u16		D_A		sigLen;
	u16		D_A		retLen;
	u16		D_A		accLen;
	u8		D_A		algType;
	PATCH_JCVM_VM_CRYPTO_IMPL_015();	
	tmpBuff 		= getJavaObjectDataArray();
	mdBuff 			= getJavaObjectDataArray();
	accLen 			= popShort();
	tmpSrcBuff 		= getJavaObjectDataRef();
	algType 		= U16t8(popShort());
	
	sigLen 			= popShort();
	retLen 			= RSA_GetModuluLength();
	sigBuff 		= getArrayDataWithCheck(retLen);
	
	inLen 			= popShort();
	inBuff 			= getArrayDataWithCheck(inLen);

	if (RSA_VERIFY == mode)
	{
		if (sigLen != retLen)
		{
			pushShort(false);
			return;
		}
	}

	PM8_INC(mdBuff); // skip the algorithm type byte

	if (tmpSrcBuff)
	{
		tmpSrcBuff 	= MEMtPM8(get_array_data((PF8)tmpSrcBuff));
	}

	incrementCodeFlowControlFlag();
	pushShort(RSA_Signature(inBuff, inLen, sigBuff, tmpSrcBuff, mdBuff, tmpBuff, accLen,  sigLen, algType, mode,REF_NULL));
}

void secureSetPKIKey(void)
{
	PM8		X_A		inBuff;
	PM8		X_A		keyBuff;
	PX8		X_A		keytempbuff;
	u16     X_A     keyOff;
	u16     X_A     keyByteLen;
	u16     X_A     realKeyLen;
//	u8		X_A		keyMode;
	u8      D_A     keyType;
	u8      D_A     keyStatus;
	u8		D_A     status;
	u16		D_A		wModulusNLength;
	
	PATCH_JCVM_VM_CRYPTO_IMPL_016();		
	realKeyLen		= (u16)popShort();
	keyType 		= (u8)popShort();
	keyStatus 		= (u8)popShort();
	status			= (u8)popShort();	
	keyByteLen 		= (u16)popShort();
	keyOff 			= (u16)popShort();
	keyBuff 		= getJavaObjectDataArray();

	inBuff			= getArrayDataWithCheck(realKeyLen);

	wModulusNLength	= realKeyLen << 1;
//	keyMode			= SC_checkKeyIsNeedENCry(keyType, status);

	keytempbuff		= MEMtPX8(MemGetReuseRam_GetTempArrFromTopBase(MAX_RSA_MUD_BYTE_LEN));
	SC_checkReturnCode(SC_PKIsetKey_JAVA(keytempbuff, keyBuff, keyOff, inBuff, keyByteLen, keyType, realKeyLen, status ));

	if(TYPE_RSA_CRT_PRIVATE == keyType)
	{
		if(((status == SC_PKI_CRT_KEY_P_MASK) || (status == SC_PKI_CRT_KEY_Q_MASK)) && (keyStatus & (SC_PKI_CRT_KEY_P_MASK | SC_PKI_CRT_KEY_Q_MASK)) == (SC_PKI_CRT_KEY_P_MASK | SC_PKI_CRT_KEY_Q_MASK)) //  P + Q -> calcModuleN
		{
			wModulusNLength = calcModuleN(keyBuff, realKeyLen, IS_RSA_CRT_MODULAR_SAVE);
		}
		if(((status == SC_PKI_CRT_KEY_P_MASK) || (status == SC_PKI_CRT_KEY_DP_MASK)) && (keyStatus & (SC_PKI_CRT_KEY_P_MASK | SC_PKI_CRT_KEY_DP_MASK)) == (SC_PKI_CRT_KEY_P_MASK | SC_PKI_CRT_KEY_DP_MASK)) // p + dp -> getE
		{
			RSA_CalE(keyBuff, realKeyLen);
		}
		if(0x1F == keyStatus)  //set Pinv
		{
			SetKeyPinv(keyBuff, realKeyLen);
		}
	}
	
	pushShort(wModulusNLength << 3);
}


void secureGetPKIKey(void)
{
	PM8		X_A		outBuff;
	PM8		X_A		keyDate;
	PX8		X_A		keytempbuff;
	u16     D_A     keyByteLen;
	u16     D_A     keyOffset;
	u16     D_A     realKeyLen;
	u8      D_A     keyType;
	u8      D_A     keyStatus;
	PATCH_JCVM_VM_CRYPTO_IMPL_017();		
	realKeyLen		= (u16)popShort();
	keyType 		= (u8)popShort();
	keyStatus 		= (u8)popShort();
	keyByteLen 		= (u16)popShort();
	
	outBuff			= getArrayDataWithCheck(realKeyLen);
	
	keyOffset 		= (u16)popShort();
	keyDate 		= getJavaObjectDataArray();

	keytempbuff		= MEMtPX8(MemGetReuseRam_GetTempArrFromTopBase(MAX_RSA_MUD_BYTE_LEN));
	SC_checkReturnCode(SC_PKIgetKey_JAVA(keytempbuff, outBuff, keyDate, keyOffset, keyByteLen, keyType, realKeyLen, keyStatus));
	
//	ecpMem_Set(keytempbuff, MEMtPX8(g_TRNG_Cache.Data[SYM_RNG_KEY_OFFSET]), keyByteLen);
}

#if VM_WITH_SIGNMR
//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
void signMRDofinal(void)
{
	PM8				inBuff;
	PM8				sigBuff;
	u16 	X_A 	inLength;
	u16 	X_A 	retLen;
	u8 		X_A 	algType;
	
	if (TYPE_RSA_CRT_PRIVATE == RSA_GetRSAKeyType())
	{
		retLen 		= RSA_GetCRTParmLength() << 1;
	}
	else
	{
		retLen 		= RSA_GetModuluLength();
	}
	
	algType 		= U16t8(popShort());
	
	sigBuff 		= getArrayDataWithCheck(retLen);
	
	inLength 		= popShort();
	inBuff 			= getArrayDataWithCheck(inLength);

	if (isInE2P(inBuff))
	{
		memRefresh();
	}

	RSA_Set_InData(bTempArr);
//	g_CryptoData.pbBuffer = bTempArr;  // for hash result temp store, that is 20 bytes.

	signMRDofinalAlg(inBuff, inLength, sigBuff, algType);

	pushShort(retLen);
}

//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
void signMRUpdateFinal(void)
{
	PM8				inBuff;
	PM8				sigBuff;
	PM8				tmpBuff;
	PM8				mdBuff;
	PM8				tmpSrcBuff;
//	u16 	X_A 	sigOffset;
	u16 	X_A 	retLen;
	u16 	X_A 	accLen;
	u16 	X_A 	inLength;
	u8 		X_A 	algType;
	
	if (TYPE_RSA_CRT_PRIVATE == RSA_GetRSAKeyType())
	{
		retLen 		= RSA_GetCRTParmLength() << 1;
	}
	else
	{
		retLen 		= RSA_GetModuluLength();
	}
	
	tmpBuff 		= getJavaObjectDataArray();
	mdBuff 			= getJavaObjectDataArray();
	accLen 			= popShort();
	tmpSrcBuff 		= getJavaObjectDataArray();
	algType 		= (u8)popShort();
	
	sigBuff 		= getArrayDataWithCheck(retLen);
	
	inLength 		= popShort();
	inBuff 			= getArrayDataWithCheck(inLength);
	
	PM8_INC(mdBuff); // skip the algorithm type byte

	if (isInE2P(inBuff))
	{
		memRefresh();
	}
	RSA_Set_InData(bTempArr);
//	g_CryptoData.pbBuffer = bTempArr;  // for hash result temp store, that is 20 bytes.

	signMRUpdateFinalAlg(inBuff, inLength, sigBuff, algType, tmpSrcBuff, accLen, mdBuff, tmpBuff);

	pushShort(retLen);
}

//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
void msgRecover(void)
{
	PM8				sigAndRecDataBuff;
	u16 	X_A 	sigLength;
	u16 	X_A 	recMsgLen;

	sigLength 			= popShort();
	sigAndRecDataBuff 	= getArrayDataWithCheck(sigLength);

	RSA_Set_InData(bTempArr);
//	g_CryptoData.pbBuffer = bTempArr;  // for 2048 bits RSA computation, this buffer should be at least 256 bytes.

	recMsgLen 			= msgRecoverAlg(sigAndRecDataBuff, sigLength);
	pushShort(recMsgLen);
}

//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
void signMRVerify(void)
{
	PM8				mdBuff;
	PM8				tmpBuff;
	PM8				inBuff;
	PM8				recoverBuff;
	u16 	X_A 	inLength;
//	u16 	X_A 	inOffset;
	u16 	X_A 	recoverDataLen;
//	u16 	X_A 	recoverBuffOffset;
	u8 		X_A 	algType;

	algType 		= (u8)popShort();
	tmpBuff 		= getJavaObjectDataArray();
	mdBuff 			= getJavaObjectDataArray();
	
	recoverDataLen 	= popShort();
	recoverBuff 	= getArrayDataWithCheck(recoverDataLen);
	
	inLength 		= popShort();
	inBuff 			= getArrayDataWithCheck(inLength);

	PM8_INC(mdBuff);

	if (isInE2P(inBuff))
	{
		memRefresh();
	}

	RSA_Set_InData(bTempArr);
//	g_CryptoData.pbBuffer = bTempArr;  // for hash result temp store, that is 20 bytes.

	if (MR_VERIFY_SUCCESS == signMRVerifyAlg(inBuff, inLength, recoverBuff, recoverDataLen, mdBuff, tmpBuff, algType))
	{
		pushShort(true);
	}
	else
	{
		pushShort(false);
	}
}

//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
/*
void signMRUpdateVerify(void)
{
	memref X_A inBuff, recoverBuff, tmpBuff, mdBuff;
	u16 X_A recoverDataLen, recoverBuffOffset, inLength, inOffset;
	u8 X_A algType;

	tmpBuff = getJavaObjectDataRef();
	mdBuff = getJavaObjectDataRef();
	algType = (u8)popShort();
	recoverDataLen = popShort();
	recoverBuffOffset = popShort();
	recoverBuff = getJavaObjectDataRef();
	inLength = popShort();
	inOffset = popShort();
	inBuff = getJavaObjectDataRef();

#if JCVM__VM__NULLOBJ_CHECK
	check_null((PF8)inBuff);
#endif

	check_bounds_interval((PF8)inBuff, inOffset, inLength);

	inBuff = PM8_A_U16(get_array_data((PF8)inBuff), inOffset);
	recoverBuff = PM8_A_U16(get_array_data((PF8)recoverBuff), recoverBuffOffset);


	tmpBuff = get_array_data((PF8)tmpBuff);
	mdBuff = get_array_data((PF8)mdBuff);

	if (isInE2P(inBuff))
	{
		memRefresh();
	}

	g_CryptoData.pbBuffer = bTempArr;  // for hash result temp store, that is 20 bytes.

	if (MR_VERIFY_SUCCESS == signMRUpdateVerifyAlg(inBuff, inLength, recoverBuff, recoverDataLen, algType,
		mdBuff, tmpBuff))
	{
		pushShort(true);
	}
	else
	{
		pushShort(false);
	}
}*/
#endif  // VM_WITH_SIGNMR
#endif	 // WITH_RSA



#if VM_WITH_HASH
//---------------------------------------------
// lzm (2009-10-18)
//---------------------------------------------
void MessageDigest(u1 isGuoMiAlg)
{
	PM8		D_A		inBuff;
	PM8		X_A		outBuff;
	PM8		D_A		mdBuff;
	PM8		D_A		tmpBuff;
	PM8		D_A		processState;
	u16		D_A		inLen;
	u8		D_A		retLen;
	u8		D_A		mdState;
	u8		D_A		algType;
	PATCH_JCVM_VM_CRYPTO_IMPL_018();		
	processState 	= getJavaObjectDataArray();
	mdState 		= memReadU8(processState);
	
	tmpBuff 		= getJavaObjectDataArray();
	
	mdBuff 			= getJavaObjectDataArray();
	algType 		= memReadU8(mdBuff);
	PM8_INC(mdBuff);
	
	retLen 			= 0;
	switch (algType)
	{
#if WITH_MD5
	case ALG_MD:
		retLen = MD_DIGEST_SIZE;
		break;
#endif
#if WITH_RIPEMD160
	case ALG_RIPEMD160:
		retLen = RIPEMD160_DIGEST_SIZE;
		break;
#endif
#if WITH_SHA1
	case ALG_SHA_1:
		retLen = SHA1_DIGEST_SIZE;
		break;
#endif
#if WITH_SHA224
	case ALG_SHA_224:
		retLen = SHA224_DIGEST_SIZE;
		break;
#endif
#if WITH_SHA256
	case ALG_SHA_256:
		retLen = SHA256_DIGEST_SIZE;
		break;
#endif
#if WITH_SHA384
	case ALG_SHA_384:
		retLen = SHA384_DIGEST_SIZE;
		break;
#endif
#if WITH_SHA512
	case ALG_SHA_512:
		retLen = SHA512_DIGEST_SIZE;
		break;
#endif
#if WITH_SM3
case ALG_SM3:
	retLen = SM3_DIGEST_SIZE;
	break;
#endif
    default:
			faultDetected();
	    break;
	}
	
	outBuff 		= getArrayDataWithCheck(retLen);
	
	inLen 			= popShort();
	inBuff 			= getArrayDataWithCheck(inLen);
	
	if (isInE2P(inBuff))
	{
		memRefresh();
	}

	HASHDoFinal(inBuff, inLen, outBuff, mdState, mdBuff, tmpBuff, algType);
	memWriteU8NoAtomic(processState, HASH_INIT);
	pushShort(retLen);
}

//---------------------------------------------
// lzm (2009-10-18)
// modify by liweimin@20210324: 优化算法性能，减少跨页和擦写NVM的次数, EPX_SUITE/CHIP/HED_CIU98M25/HED_CIU98M25_5G_SWP
//---------------------------------------------
void MDUpdate(u1 isGuoMiAlg)
{
	PM8		D_A		inBuff;
	PM8		D_A		mdBuff;
	PM8		D_A		tmpBuff;
	PM8		D_A		processState;
	u16		D_A		inLen;
	u8		D_A		mdState;
	u8		D_A		algType;
	PATCH_JCVM_VM_CRYPTO_IMPL_019();
	
	processState 	= getJavaObjectDataArray();
	tmpBuff 		= getJavaObjectDataArray();
	mdBuff 			= getJavaObjectDataArray();
	
	inLen 			= popShort();
	inBuff 			= getArrayDataWithCheck(inLen);

	if (0x00 == inLen)
	{
		return;
	}
	
	mdState 		= memReadU8(processState);
	algType 		= memReadU8(mdBuff);
	PM8_INC(mdBuff);

	if (isInE2P(inBuff))//modify by jiangxiaohui@20220630 去除M25上对HASH算法的优化，为提交cache不适用通用平台，会导致数据错误。
	{
		memRefresh();
	}
	HASHUpdate(inBuff, inLen, mdState, mdBuff, tmpBuff, algType);
	if (HASH_INIT == mdState)
	{
		memWriteU8NoAtomic(processState, HASH_UPDATE);
	}
}

#if VM_WITH_MD_INIT
//---------------------------------------------
// lzm (2010-05-20)
//---------------------------------------------
void MDInit(void)
{
	PM8		X_A		initialDigestBuff;
	PM8		X_A		digestedMsgLenBuff;
	PM8		X_A		mdBuff;
	PM8		X_A		processState;
	u16			X_A		initialDigestLength;
	u16			X_A		digestedMsgLenLength;
	boolean		X_A		retValue;
	u8		D_A		algType;

	processState			= getJavaObjectDataArray();
	mdBuff 					= getJavaObjectDataArray();
	
	digestedMsgLenLength	= popShort();
	digestedMsgLenBuff		= getArrayDataWithCheck(digestedMsgLenLength);
	
	initialDigestLength		= popShort();
	initialDigestBuff		= getArrayDataWithCheck(initialDigestLength);
	algType 				= memReadU8(mdBuff);
	PM8_INC(mdBuff);
	
	retValue = HASHInitAlg(initialDigestBuff, initialDigestLength, digestedMsgLenBuff, digestedMsgLenLength, mdBuff, algType);
	if (retValue == true)
	{
		memWriteU8NoAtomic(processState, HASH_UPDATE);
	}

	pushShort(retValue);
}
#endif	//WITH_MD_INIT
#endif  // WITH_MD5 || WITH_SHA1 || WITH_RIPEMD160 || WITH_SHA224 || WITH_SHA256 || WITH_SHA384 || WITH_SHA512

#if VM_WITH_ECC
//---------------------------------------------
// lzm (2010-05-20)
//---------------------------------------------
void storeECDomainParams(void)
{
    PM8	 paramArr;
//	u16 X_A cofactor;
    u8 X_A checkFlag;

    checkFlag = (u8)popShort();
    g_ECKeyStore.bParamStatus = (u8)popShort();
    g_ECKeyStore.b_HValue = (u8)popShort();
	g_ECKeyStore.b_RLength = (u8)popShort();
	g_ECKeyStore.b_PLength = (u8)popShort();
    paramArr = getJavaObjectDataRef();

    paramArr = (PM8)get_array_data((PF8)paramArr);
	PM8_INC(paramArr);  // skip the keyType byte
	g_ECKeyStore.pb_domainParams = (PM8)paramArr;

	memRefresh();
	if (checkFlag)
	{
		if ((g_ECKeyStore.bParamStatus & FP_DOMAIN_PARAM_MASK) >= FP_DOMAIN_PARAM_SETED)
		{
			pushShort(CheckECDomainParams());
		}
		else if(g_ECKeyStore.bParamStatus == DOMAIN_PARAM_UNSETED)
		{
			InitDefaultECDomainParams();
			pushShort(false);
		}
		else
		{
			pushShort(false);
		}
	}
	else
	{
		pushShort(true);
	}
}

//---------------------------------------------
// lzm (2010-05-20)
//---------------------------------------------
void storeECPubOrPrivKeyRef(void)
{
    memref X_A keyRef;
	u8 X_A checkFlag, keyType;
	u8 X_A status = EC_PROCESS_OK;
	
	checkFlag = (u8)popShort();
    keyType = (u8)popShort();
	keyRef = PNtMEM(getJavaObjectDataRef());

	memRefresh();

    switch(keyType)
    {
	case TYPE_EC_FP_PRIVATE:
    	g_ECKeyStore.pb_PrivateKeyS = MEMtPN8(get_array_data((PF8)keyRef));
    	if (checkFlag)
    	{
    		status = CheckECPrivateKey();
    	}
    	break;
    case TYPE_EC_FP_PUBLIC:
    	g_ECKeyStore.pb_PublicKeyQxy = MEMtPN8(get_array_data((PF8)keyRef));
    	if (checkFlag)
    	{
    		status = CheckECPublicKey();
    	}
    	break;
    }

    pushShort(status);
}

//---------------------------------------------
// lzm (2009-10-18)
// ECKeyGen(void)
//---------------------------------------------
void ECKeyGen(void)
{
	g_CryptoData.pbBuffer = bTempArr;
	GenECKeyPair();
}

//---------------------------------------------
// lzm (2009-10-18)
// modify by dengyiwen@20220722: 增加Java card3.0.5内容
//---------------------------------------------
void ECSignatureDofinal(u8 mode)
{
	memref X_A sigBuff, inBuff;
	u16 X_A inLen, inOffset, sigOffset, sigLen;
	u8 X_A algType;
	u16 X_A preHashFlag;

	algType = (u8)popShort();
	sigLen = popShort();
	sigOffset = popShort();
	sigBuff = PNtMEM(getJavaObjectDataRef());
	inLen = popShort();
	inOffset = popShort();
	inBuff = PNtMEM(getJavaObjectDataRef());
	preHashFlag = popShort();

#if JCVM__VM__NULLOBJ_CHECK
	check_null((PF8)inBuff);
	check_null((PF8)sigBuff);
#endif

	check_bounds_interval((PF8)inBuff, inOffset, inLen);

	if (ECC_SIGN == mode)
	{
		sigLen = (g_ECKeyStore.b_RLength << 1) + 6;
	}
	
	check_bounds_interval((PF8)sigBuff, sigOffset, sigLen);

	inBuff = PM8_A_U16(get_array_data((PF8)inBuff), inOffset);
	sigBuff = PM8_A_U16(get_array_data((PF8)sigBuff), sigOffset);

	g_CryptoData.pbBuffer = bTempArr;  // for 2048 bits RSA computation, this buffer should be at least 256 bytes.
	                                   // for 256 bits ECC computation, this buffer should be at least 65 bytes.

	if (isInE2P(inBuff))
	{
		memRefresh();
	}
	
	if (ECC_SIGN == mode)
	{
		ECCSign(MEMtPN8(inBuff), inLen, NULL, NULL, algType,preHashFlag);
		memCopyNoAtomic(MEMtPN8(sigBuff), (PM8)g_CryptoData.pbBuffer, g_CryptoData.wBufferLen);
		pushShort(g_CryptoData.wBufferLen);
	}
	else  // for verify
	{
		pushShort(ECVerify(MEMtPN8(inBuff), inLen, MEMtPN8(sigBuff), sigLen, NULL, NULL, algType,preHashFlag));
	}
}

//---------------------------------------------
// lzm (2009-10-18)
// modify by dengyiwen@20220722: 增加Java card3.0.5内容
//---------------------------------------------
void ECSignatureUpdateFinal(void)
{
	memref X_A sigBuff, inBuff, tmpBuff, mdBuff;
	u16 X_A inLen, inOffset, sigOffset, sigLen;
	u8 X_A algType;

	tmpBuff = PNtMEM(getJavaObjectDataRef());
	mdBuff = PNtMEM(getJavaObjectDataRef());
	algType = (u8)popShort();
//	sigLen = popShort();
	popShort();
	sigOffset = popShort();
	sigBuff = PNtMEM(getJavaObjectDataRef());
	inLen = popShort();
	inOffset = popShort();
	inBuff = PNtMEM(getJavaObjectDataRef());

#if JCVM__VM__NULLOBJ_CHECK
	check_null((PF8)inBuff);
	check_null((PF8)sigBuff);
#endif

	check_bounds_interval((PF8)inBuff, inOffset, inLen);
	sigLen = (g_ECKeyStore.b_RLength << 1) + 6;
	check_bounds_interval((PF8)sigBuff, sigOffset, sigLen);

	inBuff = PM8_A_U16(get_array_data((PF8)inBuff), inOffset);
	sigBuff = PM8_A_U16(get_array_data((PF8)sigBuff), sigOffset);

	tmpBuff = get_array_data((PF8)tmpBuff);
	mdBuff = get_array_data((PF8)mdBuff);
	PM8_INC(mdBuff); // skip the algorithm type byte

	g_CryptoData.pbBuffer = bTempArr;  // for 2048 bits RSA computation, this buffer should be at least 256 bytes.
	                                   // for 256 bits ECC computation, this buffer should be at least 65 bytes.

	if (isInE2P(inBuff))
	{
		memRefresh();
	}

	ECCSign(MEMtPN8(inBuff), inLen, MEMtPN8(mdBuff), MEMtPN8(tmpBuff), algType,REF_NULL);
	memCopyNoAtomic(MEMtPN8(sigBuff), (PM8)g_CryptoData.pbBuffer, g_CryptoData.wBufferLen);
	pushShort(g_CryptoData.wBufferLen);
}

//---------------------------------------------
// lzm (2009-10-18)
// modify by dengyiwen@20220722: 增加Java card3.0.5内容
//---------------------------------------------
void ECVerifyUpdateFinal(void)
{
	memref X_A sigBuff, inBuff, tmpBuff, mdBuff;
	u16 X_A inLen, inOffset, sigOffset, sigLen;
	u8 X_A algType;

	tmpBuff = PNtMEM(getJavaObjectDataRef());
	mdBuff = PNtMEM(getJavaObjectDataRef());
	algType = (u8)popShort();
	sigLen = popShort();
	sigOffset = popShort();
	sigBuff = PNtMEM(getJavaObjectDataRef());
	inLen = popShort();
	inOffset = popShort();
	inBuff = PNtMEM(getJavaObjectDataRef());

#if JCVM__VM__NULLOBJ_CHECK
	check_null((PF8)inBuff);
	check_null((PF8)sigBuff);
#endif

	check_bounds_interval((PF8)inBuff, inOffset, inLen);
	check_bounds_interval((PF8)sigBuff, sigOffset, sigLen);

	inBuff = PM8_A_U16(get_array_data((PF8)inBuff), inOffset);
	sigBuff = PM8_A_U16(get_array_data((PF8)sigBuff), sigOffset);

	tmpBuff = get_array_data((PF8)tmpBuff);
	mdBuff = get_array_data((PF8)mdBuff);
	PM8_INC(mdBuff);

	g_CryptoData.pbBuffer = bTempArr;  // for 2048 bits RSA computation, this buffer should be at least 256 bytes.
	                                   // for 256 bits ECC computation, this buffer should be at least 65 bytes.

	if (isInE2P(inBuff))
	{
		memRefresh();
	}

	pushShort(ECVerify(MEMtPN8(inBuff), inLen, MEMtPN8(sigBuff), sigLen, MEMtPN8(mdBuff), MEMtPN8(tmpBuff), algType,REF_NULL));
}

//---------------------------------------------
// lzm (2009-10-18)
// ECDH(void)
//---------------------------------------------
void ECDH(void)
{
	memref X_A publicKeyAddr, secretAddr;
	u16 X_A secretOffset, publicKeyLen, publicKeyOffset, secretLen;
	u8 X_A ECDHType;

	ECDHType = (u8)popShort();
	secretOffset = popShort();
	secretAddr = PNtMEM(getJavaObjectDataRef());
	publicKeyLen = popShort();
	publicKeyOffset = popShort();
	publicKeyAddr = PNtMEM(getJavaObjectDataRef());

#if JCVM__VM__NULLOBJ_CHECK
    check_null((PF8)secretAddr);
	check_null((PF8)publicKeyAddr);
#endif

    check_bounds_interval((PF8)publicKeyAddr, publicKeyOffset, publicKeyLen);

	if ((ECDHType == ALG_EC_SVDP_DH) || (ECDHType == ALG_EC_SVDP_DHC))
	{
		secretLen = SHA1_DIGEST_SIZE;
	}
	else
	{
		secretLen = g_ECKeyStore.b_PLength;
	}
	check_bounds_interval((PF8)secretAddr, secretOffset, secretLen);

	secretAddr = PM8_A_U16(get_array_data((PF8)secretAddr), secretOffset);
	publicKeyAddr = PM8_A_U16(get_array_data((PF8)publicKeyAddr), publicKeyOffset);

	if (isInE2P(publicKeyAddr))
	{
		memRefresh();
	}

	g_CryptoData.pbBuffer = bTempArr;  // for 2048 bits RSA computation, this buffer should be at least 256 bytes.
	                                   // for 256 bits ECC computation, this buffer should be at least 65 bytes.

    if ((publicKeyLen == (1 + (g_ECKeyStore.b_PLength << 1))) && (memReadU8(MEMtPM8(publicKeyAddr)) == 0x04))
    {
	    g_ECKeyStore.pb_PublicKeyQxy = MEMtPN8(publicKeyAddr);

		if (CheckECPublicKey() == EC_KEY_CHECK_OK)
		{
			ECDiffieHellman(ECDHType);
			memCopyNoAtomic(MEMtPN8(secretAddr), g_CryptoData.pbBuffer, secretLen);
			pushShort(secretLen);
	    }
		else
		{
			pushShort(EC_ILLEGAL_VALUE);
		}
	}
	else
	{
		pushShort(EC_ILLEGAL_VALUE);
	}
}


//---------------------------------------------
// lzm (2009-10-18)
// ECDH(void)
//modify by dengyiwen@20230111根据评审意见修改，此为byd私有API所用
//---------------------------------------------
#if VM_WITH_ECCMATH
void ECCMathX(void)
{
#define 	ALG_ECC_MATH_X_MODE_POINGT_ADD				0x01
#define 	ALG_ECC_MATH_X_MODE_POINGT_SUB				0x02
#define 	ALG_ECC_MATH_X_MODE_POINGT_MUL				0x03
	PM8 X_A pointP;
	PM8 X_A pointQ;
	PM8 X_A resultAddr;
	u16 X_A resultLen, pointQLen, pointPLen;
	u16 result;
	u8 X_A mode;

	mode = (u8)popShort();

	resultLen 		= 1 + (g_ECKeyStore.b_PLength << 1);
	if(ALG_ECC_MATH_X_MODE_POINGT_MUL == mode)
	{
		pointQLen 	= g_ECKeyStore.b_PLength;
	}
	else
	{
		pointQLen 	= 1 + (g_ECKeyStore.b_PLength << 1);
	}
	pointPLen 		= 1 + (g_ECKeyStore.b_PLength << 1);


	resultAddr 		= getArrayDataWithCheck(resultLen);
	pointQ 			= getArrayDataWithCheck(pointQLen);
	pointP 			= getArrayDataWithCheck(pointPLen);

	if (isInE2P(pointQ) || isInE2P(pointP))
	{
		memRefresh();
	}
	result = EC_ILLEGAL_VALUE;
	g_CryptoData.pbBuffer = bTempArr;  // for 2048 bits RSA computation, this buffer should be at least 256 bytes.
	                                   // for 256 bits ECC computation, this buffer should be at least 65 bytes.
	g_ECKeyStore.pb_PublicKeyQxy = MEMtPN8(pointP);
    if( (memReadU8(MEMtPM8(pointP)) == 0x04) && (CheckECPublicKey() == EC_KEY_CHECK_OK) )
    {
    	PM8_INC(pointP);
		if(ALG_ECC_MATH_X_MODE_POINGT_MUL == mode)
		{
			g_ECKeyStore.pb_PrivateKeyS = pointQ;
			if (EC_KEY_CHECK_OK == CheckECPrivateKey())
			{
				if(EC_PROCESS_OK == ecpUniversalAMulPointXY(pointQ, pointP, g_CryptoData.pbBuffer, g_ECKeyStore.pb_domainParams))
				{
					result = resultLen;
				}
			}
		}
		else
		{
			g_ECKeyStore.pb_PublicKeyQxy = MEMtPN8(pointQ);
			if( (memReadU8(MEMtPM8(pointQ)) == 0x04) && (CheckECPublicKey() == EC_KEY_CHECK_OK))
			{
				PM8_INC(pointQ);
// todo 涉及到算法底层，先注释掉
				if(ALG_ECC_MATH_X_MODE_POINGT_ADD == mode)
				{
					// todo
					if(EC_PROCESS_OK == ECPointAdd(pointP, pointQ, g_CryptoData.pbBuffer))
					{
						result = resultLen;
					}
				}
				else
				{
					if(EC_PROCESS_OK == ECPointSub(pointP, pointQ, g_CryptoData.pbBuffer))
					{
						result = resultLen;
					}
				}
			}
		}
	}
	if(EC_ILLEGAL_VALUE !=result)
	{
		memWriteU8NoAtomic(MEMtPN8(resultAddr), 0x04);
		PM8_INC(resultAddr);
		memCopyNoAtomic(MEMtPN8(resultAddr), g_CryptoData.pbBuffer, g_ECKeyStore.b_PLength << 1);
	}
    pushShort(result);
}
#endif  //VM_WITH_ECCMATH
#endif  // WITH_ECC

#if VM_WITH_HMAC
//---------------------------------------------
// lzm (2009-10-18)
// HMAC_Init
//---------------------------------------------
void HMAC_Init(void)
{
	PM8				hMacKeyArr;
	PM8				hMacMDBuffer;
	PM8				hMacTempBuffer;
	PM8				hMacGenKeyBuffer;
	u16 	X_A 	keyByteLen;
	u8 		X_A 	algType;

	hMacGenKeyBuffer 	= getJavaObjectDataRef();
	hMacMDBuffer 		= getJavaObjectDataRef();
	hMacTempBuffer 		= getJavaObjectDataRef();
	keyByteLen 			= popShort();
	hMacKeyArr 			= getJavaObjectDataRef();

#if JCVM__VM__NULLOBJ_CHECK
	check_null((PF8)hMacKeyArr);
	check_null((PF8)hMacMDBuffer);
	check_null((PF8)hMacTempBuffer);
	check_null((PF8)hMacGenKeyBuffer);
#endif

	/* check array bounds */
	check_bounds_interval((PF8)hMacKeyArr, 0, keyByteLen);

	hMacKeyArr 			= MEMtPM8(get_array_data((PF8)hMacKeyArr));
	hMacTempBuffer 		= MEMtPM8(get_array_data((PF8)hMacTempBuffer));
	hMacGenKeyBuffer 	= MEMtPM8(get_array_data((PF8)hMacGenKeyBuffer));

	hMacMDBuffer 		= MEMtPM8(get_array_data((PF8)hMacMDBuffer));
	algType 			= memReadU8(hMacMDBuffer);
	PM8_INC(hMacMDBuffer);

	if (isInE2P(hMacKeyArr))
	{
		memRefresh();
	}

	HMACInit(hMacKeyArr, keyByteLen, hMacTempBuffer, hMacMDBuffer, hMacGenKeyBuffer, algType);
}

//---------------------------------------------
// lzm (2009-10-18)
// HMAC_Update
//---------------------------------------------
void HMAC_Update(void)
{
	PM8				inBuff;
	PM8				hMacMDBuffer;
	PM8				hMacTempBuffer;
	u16 	X_A 	inOffset;
	u16 	X_A 	inLength;
	u8 		X_A 	algType;

	hMacMDBuffer 	= getJavaObjectDataRef();
	hMacTempBuffer 	= getJavaObjectDataRef();
	inLength 		= popShort();
	inOffset 		= popShort();
	inBuff 			= getJavaObjectDataRef();

#if JCVM__VM__NULLOBJ_CHECK
	check_null((PF8)inBuff);
	check_null((PF8)hMacMDBuffer);
	check_null((PF8)hMacTempBuffer);
#endif

	check_bounds_interval((PF8)inBuff, inOffset, inLength);

	hMacTempBuffer 	= MEMtPM8(get_array_data((PF8)hMacTempBuffer));

	hMacMDBuffer 	= MEMtPM8(get_array_data((PF8)hMacMDBuffer));
	algType 		= memReadU8(hMacMDBuffer);
	PM8_INC(hMacMDBuffer);

	inBuff 			= PM8_A_U16(MEMtPM8(get_array_data((PF8)inBuff)), inOffset);

	if (isInE2P(inBuff))
	{
		memRefresh();
	}

	HMACUpdate(inBuff, inLength, hMacTempBuffer, hMacMDBuffer, algType);
}


u8 getHMACLen(u8 algType)
{
	u8		D_A 	retLen;

	retLen 			= 0;
	switch (algType)
	{
	#if WITH_MD5
		case ALG_MD:
		case ALG_HMAC_MD5:	
			retLen = MD_DIGEST_SIZE;
			break;
	#endif
	#if WITH_RIPEMD160
		case ALG_RIPEMD160:
		case ALG_HMAC_RIPEMD160:
			retLen = RIPEMD160_DIGEST_SIZE;
			break;
	#endif
	#if WITH_SHA1
		case ALG_SHA_1:
		case ALG_HMAC_SHA1:
			retLen = SHA1_DIGEST_SIZE;
			break;
	#endif
	#if WITH_SHA224
		case ALG_SHA_224:
			retLen = SHA224_DIGEST_SIZE;
			break;
	#endif
	#if WITH_SHA256
		case ALG_SHA_256:
		case ALG_HMAC_SHA_256:
			retLen = SHA256_DIGEST_SIZE;
			break;
	#endif
	#if WITH_SHA384
		case ALG_SHA_384:
		case ALG_HMAC_SHA_384:
			retLen = SHA384_DIGEST_SIZE;
			break;
	#endif
	#if WITH_SHA512
		case ALG_SHA_512:
		case ALG_HMAC_SHA_512:
			retLen = SHA512_DIGEST_SIZE;
			break;
	#endif
	#if WITH_SM3
		case ALG_SM3:
		case ALG_HMAC_SM3:
			retLen = SM3_DIGEST_SIZE;
		break;
	#endif

		default:
			faultDetected();
			break;
		}

	return retLen;

}


//---------------------------------------------
// lzm (2009-10-18)
// HMAC_Dofinal
//---------------------------------------------
void HMAC_Dofinal(void)
{
	PM8				inBuff;
	PM8				outBuff;
	PM8				hMacMDBuffer;
	PM8				hMacTempBuffer;
	PM8				hMacGenKeyBuffer;
	u16 	X_A 	inLength;
	u16 	X_A 	inOffset;
	u16 	X_A 	outOffset;
	u8 		X_A 	retLen;
	u8 		X_A 	algType;

	hMacGenKeyBuffer 	= getJavaObjectDataRef();
	hMacMDBuffer 		= getJavaObjectDataRef();
	hMacTempBuffer 		= getJavaObjectDataRef();
	outOffset 			= popShort();
	outBuff 			= getJavaObjectDataRef();
	inLength 			= popShort();
	inOffset 			= popShort();
	inBuff 				= getJavaObjectDataRef();

#if JCVM__VM__NULLOBJ_CHECK
	check_null((PF8)inBuff);
	check_null((PF8)outBuff);
	check_null((PF8)hMacMDBuffer);
	check_null((PF8)hMacTempBuffer);
	check_null((PF8)hMacGenKeyBuffer);
#endif

	check_bounds_interval((PF8)inBuff, inOffset, inLength);

	hMacTempBuffer 		= MEMtPM8(get_array_data((PF8)hMacTempBuffer));
	hMacGenKeyBuffer 	= MEMtPM8(get_array_data((PF8)hMacGenKeyBuffer));

	hMacMDBuffer 		= MEMtPM8(get_array_data((PF8)hMacMDBuffer));
	algType 			= memReadU8(hMacMDBuffer);
	PM8_INC(hMacMDBuffer);
	
	retLen = getHMACLen(algType);
	check_bounds_interval((PF8)outBuff, outOffset, retLen);

	inBuff 				= PM8_A_U16(MEMtPM8(get_array_data((PF8)inBuff)), inOffset);
	outBuff 			= PM8_A_U16(MEMtPM8(get_array_data((PF8)outBuff)), outOffset);

	if (isInE2P(inBuff))
	{
		memRefresh();
	}

	retLen = HMACDofinal(inBuff, inLength, outBuff, hMacMDBuffer, hMacTempBuffer, hMacGenKeyBuffer, algType);
	pushShort(retLen);
}

//---------------------------------------------
// lzm (2009-10-18)
// HMAC_Verify();
//---------------------------------------------
void HMAC_Verify(void)
{
	PM8				inBuff;
	PM8				outBuff;
	PM8				hMacMDBuffer;
	PM8				hMacTempBuffer;
	PM8				hMacGenKeyBuffer;
	PX8				tmpData;
	u16 	X_A 	inLength;
	u16 	X_A 	inOffset;
	u16 	X_A 	outLength;
	u16 	X_A 	outOffset;
	u8 		X_A 	retLen;
	u8 		X_A 	tmpLen;
	u8 		X_A 	algType;

	hMacGenKeyBuffer 	= getJavaObjectDataRef();
	hMacMDBuffer 		= getJavaObjectDataRef();
	hMacTempBuffer 		= getJavaObjectDataRef();
	outLength 			= popShort();
	outOffset 			= popShort();
	outBuff 			= getJavaObjectDataRef();
	inLength 			= popShort();
	inOffset 			= popShort();
	inBuff 				= getJavaObjectDataRef();

#if JCVM__VM__NULLOBJ_CHECK
	check_null((PF8)inBuff);
	check_null((PF8)outBuff);
	check_null((PF8)hMacMDBuffer);
	check_null((PF8)hMacTempBuffer);
	check_null((PF8)hMacGenKeyBuffer);
#endif

	check_bounds_interval((PF8)inBuff, inOffset, inLength);
	check_bounds_interval((PF8)outBuff, outOffset, outLength);

	hMacTempBuffer 		= MEMtPM8(get_array_data((PF8)hMacTempBuffer));
	hMacGenKeyBuffer 	= MEMtPM8(get_array_data((PF8)hMacGenKeyBuffer));

	hMacMDBuffer 		= MEMtPM8(get_array_data((PF8)hMacMDBuffer));
	algType 			= memReadU8(hMacMDBuffer);
	PM8_INC(hMacMDBuffer);
	
	retLen = getHMACLen(algType);
	check_bounds_interval((PF8)outBuff, outOffset, retLen);

	inBuff 				= PM8_A_U16(MEMtPM8(get_array_data((PF8)inBuff)), inOffset);
	outBuff 			= PM8_A_U16(MEMtPM8(get_array_data((PF8)outBuff)), outOffset);

	if (isInE2P(inBuff))
	{
		memRefresh();
	}

	tmpData 			= bTempArr;

	retLen 				= HMACDofinal(inBuff, inLength, tmpData, hMacMDBuffer, hMacTempBuffer, hMacGenKeyBuffer, algType);

	if (outLength == retLen)// modify by dengyiwen@20220810: 修改ecckey错误和AEAD算法数组过大错误
	{
		memRefresh();

		//if (*(u8 *)&outBuff == 0)
		 tmpLen 		= SC_memCompareEqualWithDFA( tmpData, outBuff,  outLength);

		if (tmpLen == ECP_CMP_RESULT_EQUAL)
		{
			pushShort(true);
		}
		else
		{
			pushShort(false);
		}
	}
	else
	{
		pushShort(false);
	}
}

void GeneralHMACNM()
{
	PM8				keyAddr;
	PM8				indataAddr;
	PM8				outdataAddr;
	PM8 			key;
	PM8 			indata;
	PM8 			outdata;
	u16 	X_A		keylen;
	u16 	X_A		indatalen;
	u16 	X_A		returnLen;
	u8 		X_A		outdatalen;
	u8 		X_A		type;

	type 			=  (u8)popShort();
	outdatalen 		=  (u8)popShort();
	outdataAddr 	= getJavaObjectDataRef();
    indatalen 		= popShort();
    indataAddr 		= getJavaObjectDataRef();
	keylen 			= popShort();
 	keyAddr			= getJavaObjectDataRef();

#if JCVM__VM__NULLOBJ_CHECK
    check_null((PF8)keyAddr);
    check_null((PF8)indataAddr);
    check_null((PF8)outdataAddr);
#endif

	key 			= MEMtPM8(get_array_data((PF8)(PF8)keyAddr));
	indata 			= MEMtPM8(get_array_data((PF8)(PF8)indataAddr));
	outdata 		= MEMtPM8(get_array_data((PF8)(PF8)outdataAddr));

	returnLen 		= GeneralHMAC(key, keylen, indata, indatalen, outdata, outdatalen, type);
	pushShort(returnLen);
}

#endif // WITH_HMAC

#if VM_WITH_SM2
u8 CONST defaltID[16]	= {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
/*
void storeSM2UserIDref(void)
{
  SM2_IDStore.IDlen = popShort();
  SM2_IDStore.userID = (get_array_data((PF8)getJavaObjectDataRef()));

  if(SM2_IDStore.IDlen  ==0)
  {
  	SM2_IDStore.IDlen = IDDEFAULT_LEN;
    memCopyNoAtomic(SM2_IDStore.userID, defaltID, SM2_IDStore.IDlen);
  }
}
*/
// modify by liweimin@20210324: 优化算法性能，减少跨页和擦写NVM的次数, EPX_SUITE/CHIP/HED_CIU98M25/HED_CIU98M25_5G_SWP
void storeSM2KeyRef(void)
{
	u8		X_A		sm2KeyType;
	PM8				sm2KeyAddr;

	sm2KeyType		= U16t8(popShort());
	sm2KeyAddr		= getJavaObjectDataArray();

	if(!GUOMI_STANDARD)
	{
		if (TYPE_SM2_PRIVATE == sm2KeyType)
		{
			SM2_storePriKey(sm2KeyAddr);
		}
		else // if(TYPE_SM2_PUBLIC == g_SM2KeyStore.SM2KeyType)
		{
			SM2_storePubKey(sm2KeyAddr);
		}
	}
	else
	{
		if (TYPE_SM2_PRIVATE_NEW == sm2KeyType)
		{
			SM2_storePriKey(sm2KeyAddr);
		}
		else // if(TYPE_SM2_PUBLIC == g_SM2KeyStore.SM2KeyType)
		{
			SM2_storePubKey(sm2KeyAddr);
		}
		
	}
			
	// Clear cache
	memRefresh();//modify by jiangxiaohui@20220630 去除M25上对HASH算法的优化，为提交cache不适用通用平台，会导致数据错误。
}

void genSM2KeyPair(void)
{ 
	PX8				pubkey;
	PX8				prikey;
	PM8				sm2PubKey;
	PM8				sm2PriKey;

	
	prikey 			= MEMtPX8(MemGetReuseRam_GetTempArrBuffer(TEMP_ARR_BUF_SIZE));
	pubkey 			= PX8_A_U16(prikey, 32);  

	sm2PubKey 		= getJavaObjectDataArray();
	sm2PriKey 		= getJavaObjectDataArray();
	SM2_storePubKey(sm2PubKey);
	SM2_storePriKey(sm2PriKey);

	SM2_ECP_GetKey(prikey, pubkey, true);
}
void computeZa(void)
{
	PX8		X_A		tempZa;
	PX8		X_A		tempID;
	PM8		X_A		tempPublicKey;
	
	PM8		X_A		prt_Za;
	PM8		D_A		userID;
	PM8		D_A		privateKey;
	PM8		D_A		pubKey;
	u16		D_A		IDoffset;
	u8		D_A		IDlength;
	
	

	tempZa 			= MEMtPX8(MemGetReuseRam_GetTempArrBuffer(TEMP_ARR_BUF_SIZE));
	tempID 			= PX8_A_U8(tempZa, 32);  
	tempPublicKey 	= PX8_A_U8(tempID, 32);  
	
	prt_Za 			= getJavaObjectDataArray();
	privateKey 		= getJavaObjectDataRef();
	pubKey 			= getJavaObjectDataRef();
	IDlength 		= U16t8(popShort());
	IDoffset 		= popShort();
	userID 			= getJavaObjectDataRef();

/*
#if JCVM__VM__NULLOBJ_CHECK
	check_null((PF8)prt_Za);
#endif
*/

	//如果没传公钥，则自己生成公钥
	if (NULL != pubKey)
	{
		tempPublicKey = MEMtPM8(get_array_data((PF8)pubKey));
		tempPublicKey++;  // skip the Tag 0x04.
	}
	else
	{
		privateKey	= MEMtPM8(get_array_data((PF8)privateKey));
		// Clear cache
		memRefresh();
		SM2_ECP_GenKeyPair(privateKey, tempPublicKey);
	}

	if (IDlength == 0)
	{
		memCopyToRam(tempID, MEMtPM8(defaltID), 16);
		IDlength 	= 16;
	}
	else
	{
		userID 		= PM8_A_U16(MEMtPM8(get_array_data((PF8)userID)), IDoffset);
	    memCopyToRam(tempID, userID, IDlength);
	}
    SM2_ECP_GetZa(tempID, IDlength, tempPublicKey, tempZa);

	memCopyNoAtomic(prt_Za, tempZa, SM3_DIGEST_SIZE);
}

void SM2SignatureUpdateFinal(u8 mode)		SMALL_MODE
{
	PM8				inBuff;
	PM8				sigBuff;
	PM8				mdBuff;
	PM8				tmpBuff;
	u16		D_A		inLen;
	u16		D_A		inOffset;
	u16		D_A		sigLen;
	u16		D_A		sigOffset;

	tmpBuff 		= getJavaObjectDataRef();
	mdBuff 			= getJavaObjectDataRef();
	sigLen 			= popShort();
	sigOffset 		= popShort();
	sigBuff 		= getJavaObjectDataRef();
	inLen 			= popShort();
	inOffset 		= popShort();
	inBuff 			= getJavaObjectDataRef();

#if JCVM__VM__NULLOBJ_CHECK
	check_null((PF8)inBuff);
	check_null((PF8)sigBuff);
#endif

	check_bounds_interval((PF8)inBuff, inOffset, inLen);
	
	if (SM2_VERIFY == mode)
	{
		if (SM2_PUBLICKEYLEN != sigLen)
		{
			pushShort(false);
			return;
		}
	}
	else //if(SM2_SIGN == mode)
	{
		sigLen = 64;	
	}
	check_bounds_interval((PF8)sigBuff, sigOffset, sigLen);

	inBuff 			= PM8_A_U16(MEMtPM8(get_array_data((PF8)inBuff)), inOffset);
	sigBuff 		= PM8_A_U16(MEMtPM8(get_array_data((PF8)sigBuff)), sigOffset);

	tmpBuff 		= MEMtPM8(get_array_data((PF8)tmpBuff));
	mdBuff 			= MEMtPM8(get_array_data((PF8)mdBuff));
	PM8_INC(mdBuff); // skip the algorithm type byte

	pushShort(SM2_Signature(inBuff, inLen,  sigBuff, NULL, mdBuff, tmpBuff, mode));
}



void SM2SignatureDofinal(u8 mode)		SMALL_MODE			 
{
	PM8				Za;
	PM8				inBuff;
	PM8				sigBuff;
	u16		D_A		inLen;
	u16		D_A		sigLen;

	Za 				= getJavaObjectDataArray();

	sigLen 			= popShort();
	if (SM2_SIGN == mode)
	{
		sigLen 		= 64;
	}

	sigBuff 		= getArrayDataWithCheck(sigLen);
	
	inLen 			= popShort();
	
	inBuff 			= getArrayDataWithCheck(inLen);

	pushShort(SM2_Signature(inBuff, inLen,  sigBuff, Za, NULL, NULL, mode));
}


void SM2Sig_VerifyWithNoSM3(u8 mode)		SMALL_MODE
{
	PM8				inBuff;
	PM8				sigBuff;
	PM8				tmpSrcBuff;
	PM8				processState;

	u16		D_A		inLen;
	u16		D_A		inOffset;
	u16		D_A		sigLen;
	u16		D_A		sigOffset;
	u8		D_A		sm2State;
	u8		D_A		accOffset;

    sigLen = 0x00;
	processState 	= getJavaObjectDataRef();
	accOffset 		= U16t8(popShort());
	tmpSrcBuff 		= getJavaObjectDataRef();
	if (SM2_VERIFY == mode)
	{
		sigLen = popShort(); 	
	}
	sigOffset 		= popShort();
	sigBuff 		= getJavaObjectDataRef();
	inLen 			= popShort();
	inOffset 		= popShort();
	inBuff 			= getJavaObjectDataRef();

#if JCVM__VM__NULLOBJ_CHECK
	check_null((PF8)inBuff);
	check_null((PF8)sigBuff);
#endif

	//outBuff2 = outBuff; // check the array when copying the result later.
	check_bounds_interval((PF8)inBuff, inOffset, inLen);
	if (SM2_SIGN == mode)
	{
		sigLen 		= 64;
	}
	check_bounds_interval((PF8)sigBuff, sigOffset, sigLen);
	
	inBuff 			= PM8_A_U16(MEMtPM8(get_array_data((PF8)inBuff)), inOffset);
	sigBuff 		= PM8_A_U16(MEMtPM8(get_array_data((PF8)sigBuff)), sigOffset);
	
	processState 	= MEMtPM8(get_array_data((PF8)processState));
	sm2State 		= memReadU8(processState);


	if (SM2_INIT != sm2State)
	{
		tmpSrcBuff 	= MEMtPM8(get_array_data((PF8)tmpSrcBuff));
	}
	
	pushShort(SM2_SigVerifyWithNoSM3(inBuff, inLen, sigBuff, sigLen, tmpSrcBuff, accOffset, mode, sm2State));
}


void SM2CipherDofinal(void)
{
	PM8		D_A		inBuff;
	PM8		X_A		outBuff;
	PM8		D_A		tmpSrcBuff;
	PM8		D_A		processState;

	u16		D_A		inLen;
	u16		D_A		inOffset;
	u16		D_A		outLength;
	u16		D_A		accOffset;
	u8		D_A		mode;
	u8		D_A		SM2State;
	u8		X_A		SM2ChiperPaddingLength;

	processState 	= getJavaObjectDataArray();
	mode 			= U16t8(popShort());
	popShort();  // the param algType is not used in this function!
	accOffset 		= popShort();
	tmpSrcBuff 		= getJavaObjectDataRef();
	
	inLen 			= popShort();
	inBuff 			= getArrayDataWithCheck(inLen);
	
	if(!GUOMI_STANDARD)
	{
		SM2ChiperPaddingLength = SM2_CIPHER_PADDING_LEN + 1;
	}
	else
	{
		SM2ChiperPaddingLength = SM2_CIPHER_PADDING_LEN;
	}
	
	SM2State 		= memReadU8(processState);

	if (SM2_INIT == SM2State)
	{
		inOffset 	= inLen;  // calculate the total input length.
	}
	else
	{
		tmpSrcBuff 	= MEMtPM8(get_array_data((PF8)tmpSrcBuff));
		inOffset 	= inLen + accOffset;  // calculate the total input length.
	}
	
	if (SM2ENCRYPT == mode)
	{
		outLength	= inOffset + SM2ChiperPaddingLength;
	}
	else  // decrypt
	{
		outLength	= inOffset - SM2ChiperPaddingLength;
	}
	
	outBuff 		= getArrayDataWithCheck(outLength);
	
	pushShort(SM2_CipherDofinal(inBuff, inLen, outBuff, tmpSrcBuff, accOffset, mode, SM2State));
}

/*
*@function: NM_SM2KeyExchange
*
*@description:用户A和B密钥交换，产生一个秘密共享密钥
*
*@param      :u16 fixPubKey_A    用户A的公钥的引用
*@param      :u16 fixPriKey_A	  用户A的私钥的引用
*@param      :u16 tempPubKey_A_X 用户A的临时公钥的X部分的引用
*@param      :u16 tempPriKey_A	  用户A的临时私钥的引用
*@param      :u16 fixPubKey_B    用户B的公钥的引用
*@param      :u16 tempPubKey_B   用户B的临时公钥的引用
*@param      :u16 userId_A       用户A的用户Id的引用
*@param      :u16 userId_B       用户B的用户ID的引用
*@param      :u16 outBuffer      输出数组的引用
*@param      :u16 outOffset
*@param      :u8 exchangeKeyLen 交换的密钥长度
*
*@return     : void
*
*@author     :注：输入的数据均为Lv结构，存在数组中
*/
void SM2KeyExchange(void)
{
	PM8 X_A fixPubKey_A;
	PM8 X_A fixPriKey_A;
	PM8 X_A tempPubKey_A_X;
	PM8 X_A tempPriKey_A;
	PM8 X_A fixPubKey_B;
	PM8 X_A tempPubKey_B;
	PM8 X_A userId_A;
	PM8 X_A userId_B;
	PM8 X_A outBuffer;
	u8 D_A userIdLen_A,userIdLen_B,exchangeKeyLen;
	u8 D_A bMode;

	bMode = U16t8(popShort());

	exchangeKeyLen = U16t8(popShort());	//需要交换的密钥长度

	outBuffer = getArrayDataWithCheck(exchangeKeyLen);

	userId_B = getJavaObjectDataArray();

	userId_A = getJavaObjectDataArray();

	tempPubKey_B = getJavaObjectDataArray();	//临时公钥 B

	fixPubKey_B = getJavaObjectDataArray();   //固定公钥 B

	tempPriKey_A = getJavaObjectDataArray();	 //临时私钥 A

	tempPubKey_A_X = getJavaObjectDataArray();	  //临时公钥 A X

	fixPriKey_A = getJavaObjectDataArray();	     //固定私钥 A

	fixPubKey_A = getJavaObjectDataArray();	     //固定公钥 A

	userIdLen_A = memReadU8(userId_A);

	userIdLen_B = memReadU8(userId_B);

	SM2_KeyExchange(fixPubKey_A+1, fixPubKey_B+1,tempPubKey_A_X+1,tempPubKey_B+1,fixPriKey_A+1,tempPriKey_A+1,userId_A+1,userId_B+1,userIdLen_A,userIdLen_B,outBuffer,exchangeKeyLen,bMode);

}

void SM2CalPublicByPrivate(void)
{
	PM8				sm2PubKey;
	PM8				sm2PriKey;
	u8 		X_A		priKeyBuf[SM2_PRIVEYLEN];
	u8 		X_A		pubKeyBuf[SM2_PUBLICKEYLEN];

	sm2PriKey = getJavaObjectDataArray();
	memCopyNoAtomic(priKeyBuf, sm2PriKey, SM2_PRIVEYLEN);

	sm2PubKey = getArrayDataWithCheck(SM2_PUBLICKEYLEN);

	SM2_ECP_GenKeyPair(priKeyBuf, pubKeyBuf);

	memCopyNoAtomic(sm2PubKey, pubKeyBuf, SM2_PUBLICKEYLEN);
}
// modify by liweimin@20210324: 优化算法性能，减少跨页和擦写NVM的次数, EPX_SUITE/CHIP/HED_CIU98M25/HED_CIU98M25_5G_SWP
void SM2ComputeZ()
{
	PM8 X_A         zBuffer;
	PM8	X_A			sm2PubKey;
	PM8	X_A			userId;
	u16 D_A         userIdLen;

	zBuffer = getArrayDataWithCheck(32);

	sm2PubKey = getArrayDataWithCheck(64);

	userIdLen = U16t8(popShort());	//需要交换的密钥长度

	userId =  getArrayDataWithCheck(userIdLen);	   		 //存储用户身份标示引用

	memRefresh();//modify by jiangxiaohui@20220630 去除M25上对HASH算法的优化，为提交cache不适用通用平台，会导致数据错误。

	SM2_ECP_GetZa(userId, userIdLen, sm2PubKey, zBuffer);
}
#endif //WITH_SM2

/*@Function:    GenECCKeyPair
 *
 *@Descrip:     获取ecc类型非对称公私钥
 *
 *@See:
 *
 *@Param:      u8 * pkBuf:	 	公钥值 (outPut)
 *			   u8 * skBuf： 		私钥值 (outPut)
 *			   u8 keyRef:		椭圆参数的索引 (inPut)，目前只支持00(NIST P-256)和03(brainpoolP256r1)，即默认公钥长64字节，私钥32字节
 *
 *@Return:     u8: TRUE or FALSE
 */
u8 GenECCKeyPair(u8 * pkBuf, u8 * skBuf, u8 keyRef)
{
	setECDomainParams(keyRef);

	setECPubOrPrivKeyRef(TYPE_EC_FP_PRIVATE, skBuf, NULL);

	setECPubOrPrivKeyRef(TYPE_EC_FP_PUBLIC, NULL, pkBuf);

	ECKeyGen();

	return TRUE;
}

