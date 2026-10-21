/*
 * CASDNativeMethod.c
 *
 *  Created on: 2013-12-12
 *      Author: jiangxiaohui
 */

#include "Crypto_impl.h"
#include "stddef.h"
#include "keySet.h"
#include "SCPInterface.h"
#include "casdStoreKey.h"
#include "scp10.h"
#include "global_variable.h"
#include "stack.h"
#include "CASDNativeMethod.h"
#include "OPCMCal.h"
#include "SCPDealData.h"
#if GP__AMENDMENT__A__FOR_MASK
//modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
u8 checkAndGetKeyType(PN8 keySetObj, u8 keyId)
{
	if (MEM_NULL != fm03_locateKeyType(keySetObj, keyId))
	{
		return keySet_getKeyType(keySetObj, keyId,THROW_ERROR);
	}
	return KEY_TYPE_NULL;
}

/**
 * (non-Javadoc)
 * @see org.globalplatform.Authority#init(byte)
 */
void CASD_init(u8 theMode, u8 isPk, u8 Scenario, jref CASDAppRef)
{

	PN8					sdDataObj;
	PN8					keySetObj;
	PN8 					keyLocation;
	jref sdDataRef;

	u8 keyIsInit;
	u8 keyType;
	keyIsInit   = false;
	
	sdDataRef = sdData_getSDdataRef(CASDAppRef);
	sdDataObj = MEMtPN8(locateObjectInMemory(sdDataRef));
	keySetObj = MEMtPN8(getKeySetObj(sdDataObj, CASD_KEY_VERSION));

	switch(isPk)
	{
		case ScenarioPK_Mask:
			switch(Scenario)
			{
				case Scenario1_Mask:
					if(MODE_SIGN == theMode)
					{
						// SKCASDAUTRsaKey // SKCASDAUTRsaKeySig
						// SKCASDAUTRsaPriKey // SKCASDAUTRsaKeySig
						keyType = checkAndGetKeyType(keySetObj, SK_CASD_AUT_KEY_ID);
						if( (KEY_TYPE_RSA_PIVATE == keyType) || (KEY_TYPE_RSA_CRT == keyType) )
						{
							keyIsInit = true;
						}
					}
					else if(MODE_KEY_RECOVERY == theMode)
					{
						// PKCAAUTRsaKey  // PKCAAUTRsaSignMR
						keyType = checkAndGetKeyType(keySetObj, PK_CA_AUT_KEY_ID);
						if(KEY_TYPE_RSA_PUBLIC == keyType)
						{
							keyIsInit = true;
						}
					}
				break;
				case Scenario2A_Mask:
					if(MODE_KEY_RECOVERY == theMode)
					{
						// PKCAAUTRsaKey  // PKCAAUTRsaSignMR
						keyType = checkAndGetKeyType(keySetObj, PK_CA_AUT_KEY_ID);
						if(KEY_TYPE_RSA_PUBLIC == keyType)
						{
							// SKCASDCTRsaKey  // SKCASDCTRsaCipher
							// SKCASDCTRsaPriKey  // SKCASDCTRsaCipher
							keyType = checkAndGetKeyType(keySetObj, SK_CASD_CT_KEY_ID);
							if( (KEY_TYPE_RSA_PIVATE == keyType) || (KEY_TYPE_RSA_CRT == keyType) )
							{
								keyIsInit = true;
							}
						}
					}
				break;
				case Scenario2B_Mask:
					if(MODE_KEY_RECOVERY == theMode)
					{
						// SKCASDCTRsaKey  // SKCASDCTRsaCipher
						// SKCASDCTRsaPriKey  // SKCASDCTRsaCipher
						keyType = checkAndGetKeyType(keySetObj, SK_CASD_CT_KEY_ID);
						if( (KEY_TYPE_RSA_PIVATE == keyType) || (KEY_TYPE_RSA_CRT == keyType) )
						{
							keyIsInit = true;
						}
					}

				break;
			}
		break;

		case ScenarioNPK_Mask:
			switch(Scenario)
			{
				case Scenario1_Mask:
					if(MODE_SIGN == theMode)
					{
						// KSCASDAUTDesKey  // KSCASDAUTDesSig
						keyType = checkAndGetKeyType(keySetObj, KS_CASD_AUT_KEY_ID);
						if(KEY_TYPE_SYMMETRIC == keyType)
						{
							keyIsInit = true;
						}
					}
					else if(MODE_KEY_RECOVERY == theMode)
					{
						// KSCASDAUTDesKey  //  KSCASDAUTDesSig
						keyType = checkAndGetKeyType(keySetObj, KS_CASD_AUT_KEY_ID);
						if(KEY_TYPE_SYMMETRIC == keyType)
						{
							// KSCASDCTDesKey  //  KSCASDCTDesCipher
							keyType = checkAndGetKeyType(keySetObj, KS_CASD_CT_KEY_ID);
							if(KEY_TYPE_SYMMETRIC == keyType)
							{
								keyIsInit = true;
							}
						}
					}
				break;
				default:
				break;
			}
		break;
	}
	if(false == keyIsInit)
	{
		throw_error(CRYPTOEXCEPT_ILLEGAL_VALUE);
	}
}

/**
 * Generates the signature of all/last input data.
 * A call to this method resets this Authority interface to the state it was
 * in when previously initialized via a call to init().
 * That is, the object is reset and available to sign another message.
 * The input and output buffer may overlap and shall be global arrays.
 *
 * @param inBuff the input buffer of data to be signed
 * @param inOffset the offset in input buffer at which the signature starts
 * @param inLength the u8 length to sign
 * @param sigBuff the output buffer to store signature data
 * @param sigOffset the offset into sigBuff at which to begin signature generation
 *
 * @return the number of u8s of signature output in sigBuff
 *
 * @throws CryptoException with the following reason codes:
 * <li><code>INVALID_INIT</code> if this Authority interface is not initialized or
 * initialized in <code>MODE_KEY_RECOVERY</code> mode.</li>
 * <li><code>ILLEGAL_USE</code> if this Authority algorithm does not pad the message and
 * the message is not block aligned.</li>
 * @throws SecurityException if the inBuff or sigBuff are not global array.
 */

u16 CASD_sign(PN8 inBuff, u16 inOffset, u16 inLength, PN8 sigBuff, u16 sigOffset, jref CASDAppRef)
{
	PN8				sdDataObj;
	PN8				keySetObj;
	u8 keyType;
	u32 keyAddress;
	
	u16 keyu8Len = 0;
	
	u16 padlen;
	u16 inRemainOff;
	u16 outRemainOff;
	u16 remainDataLen;
	
	sdDataObj = MEMtPN8(locateObjectInMemory(sdData_getSDdataRef(CASDAppRef)));
	keySetObj = MEMtPN8(getKeySetObj(sdDataObj, CASD_KEY_VERSION));

//	if ((zInitState == false) || (bMode == MODE_KEY_RECOVERY)) {
//		throw_error(INVALID_INIT);
//	}

	// SK.CASD.AUT
	if(inLength > 0x80)
	{/**
		 *
		 * 		sigBuff[sigOffset] = SKCASDAUTRsaKeyu8Len;
		 * 		sigBuff[sigOffset + SKCASDAUTRsaKeyu8Len + 1] = remainDataLen
		 *
		 * 		return SKCASDAUTRsaKeyu8Len + remainDataLen + 2
		 *
		 * 		ISO9796 padding format: EB = 6A or 4A or 4B | BB...BB | BA | Data + 'BC' |
		 *
		 */
		keyu8Len = 0;
		keyType = checkAndGetKeyType(keySetObj, SK_CASD_AUT_KEY_ID);

		if(KEY_TYPE_RSA_PIVATE == keyType)
		{
			keyu8Len = keyData_getKeyDataById(keySetObj, SK_CASD_AUT_KEY_ID, RSA_N_DGI_TYPE, MEMtPN32(&keyAddress));
		}
		else if(KEY_TYPE_RSA_CRT == keyType)
		{
			keyu8Len = keyData_getKeyDataById(keySetObj, SK_CASD_AUT_KEY_ID, RSA_CRT_P_DGI_TYPE, MEMtPN32(&keyAddress));
			keyu8Len = U16_SHL_U8(keyu8Len , 1);
			//keyu8Len = 0x80;
		}
		else
		{
			throw_error(CRYPTOEXCEPT_UNINITIALIZED_KEY);
		}

		padlen = U16_S_U8(keyu8Len , 22);
		inRemainOff = U16_A_U16(inOffset , U16_A_U8(inLength , 24));
		outRemainOff = U16_A_U16(sigOffset , U16_A_U8(keyu8Len , 1));
		remainDataLen = U16_S_U16(inLength , padlen);

		//   move
		memCopyAtomic( (&inBuff[inRemainOff]), (&inBuff[U16_A_U16(inOffset , padlen)]), 23);
		//arrayCopyNonAtomic(inBuff, (u16)( inOffset + padlen ), inBuff, inRemainOff, (u16)23 );

		sigBuff[sigOffset] = U16t8(nativeRSASignatureDoFinalCal(keySetObj, keyType, SK_CASD_AUT_KEY_ID, (&inBuff[inOffset]), inLength, (&sigBuff[U16_A_U8(sigOffset , 1)]), 0x00, ALG_RSA_SHA_ISO9796, MODE_SIGN));
		//sigBuff[sigOffset] = (u8)SKCASDAUTRsaKeySig.sign(inBuff, inOffset, inLength, sigBuff, (u16)(sigOffset + 1));

		memCopyAtomic((&sigBuff[U16_A_U8(outRemainOff , 24)]), (&inBuff[U16_A_U8(inOffset , U16_A_U8(keyu8Len , 1))]), (U16_S_U8(remainDataLen , 23)));
		//arrayCopyNonAtomic(inBuff, (u16)(inOffset + keyu8Len + (u16)1 ), sigBuff, (u16)(outRemainOff + (u16)24 ), (u16)(remainDataLen - (u16)23) );

		memCopyAtomic( (&sigBuff[U16_A_U8(outRemainOff , 1)]), (&inBuff[inRemainOff]), 23);
		//arrayCopyNonAtomic(inBuff, inRemainOff , sigBuff, (u16)(outRemainOff + (u16)1), (u16)23 );

		sigBuff[outRemainOff] = U16t8(remainDataLen);

		return U16_A_U16(U16_AND_U16(U8t16(sigBuff[sigOffset]) , 0x00FF) , U16_A_U8(remainDataLen , 2));
	}
	//	KS.CASD.AUT
	else
	{
		sigBuff[sigOffset] = U16t8(nativeDESSignatureDoFinalCal(keySetObj, KS_CASD_AUT_KEY_ID, PXtMEM(&inBuff[inOffset]), inLength, PXtMEM(&sigBuff[U16_A_U8(sigOffset , 1)]), 0x00, ALG_DES_MAC8_ISO9797_1_M2_ALG3, MODE_SIGN, U8_OR_U8(ALG_DES , SBC_SIGN) ));
		//sigBuff[sigOffset] = (u8)KSCASDAUTDesSig.sign(inBuff, inOffset, inLength, sigBuff, (u16)(sigOffset + 1));
		return U8t16(U8_A_U8(sigBuff[sigOffset] , 1));
	}

}

/* (non-Javadoc)
 * @see org.globalplatform.Authority#update(u8 *, u16, u16)
 */
void CASD_update(PN8 inBuff, u16 inOffset, u16 inLength, jref CASDAppRef)
{
	// TODO Auto-generated method stub

}




/**
 * (non-Javadoc)
 * @see org.globalplatform.Authority#recoverKey(u8 *, u16, u16, u8 *, u16)
 */
u16 CASD_recoverKey(PN8 inBuff, u16 inOffset, u16 inLength, PN8 outBuff, u16 outOffset, jref CASDAppRef)
{
//	if ((zInitState == false) || (bMode == MODE_SIGN)) {
//		throw_error(INVALID_INIT);
//	}

	u8 						tmpLen;
	u8 						bKeyType;
	u16 					tag00B8LenOff;
	u16 					tag80B8LenOff;
	u16 					tag00AELenOff;
	u16 					retLength = 0;
	u16 					certificateTag;
	PN8						sdDataObj;
	PN8						keySetObj;
	boolean 				result;

	result 					= false;
	certificateTag 			= PX_R_U16(&inBuff[inOffset]);
	
	sdDataObj = MEMtPN8(locateObjectInMemory(sdData_getSDdataRef(CASDAppRef)));
	keySetObj = MEMtPN8(getKeySetObj(sdDataObj, CASD_KEY_VERSION));

	switch (certificateTag) {
	case 0x7F21: // Non-self descriptive certificate, that is just sign the value field.
		retLength = U8t16(scp10_recoverKey(keySetObj, inBuff, inOffset, outBuff, outOffset));
		break;

	case 0x00B8: // DGI '00B8' '80B8' '00AE'
		// Pull Mode using Non-PK scheme: process the KS.AP.CT verification and decryption belonging to AP.
//		if ((KSCASDAUTDesKey.isInitialized() == false) || (KSCASDCTDesKey.isInitialized() == false)) {
//			throw_error(UNINITIALIZED_KEY);
//		}

		tag00B8LenOff = U16_A_U8(inOffset, SIZE_OF_TWOBYTETAG_TAG);  // offset for the length after Tag 00B8.
		tag80B8LenOff = U16_A_U8(tag00B8LenOff , U8_A_U8(inBuff[tag00B8LenOff] , 3)); // offset for the length after Tag 80B8.
		tag00AELenOff = U16_A_U8(tag80B8LenOff , U8_A_U8(inBuff[tag80B8LenOff] , 3)); // offset for the length after Tag 00AE.
	
		// verify the signature that has been signed using the
		tmpLen = U8_A_U8(inBuff[tag00AELenOff] , 3); // the length of the tag 00AE (including tag and L, V).
		tmpLen = U16t8(U16_S_U8(inLength , tmpLen)); // the length of tag 00B8 and 80B8 (including tag and L, V).
		//KSCASDAUTDesSig.init(KSCASDAUTDesKey, Signature.MODE_VERIFY);
		result = nativeDESSignatureDoFinalCal(keySetObj, KS_CASD_AUT_KEY_ID, PNtMEM(&inBuff[inOffset]), tmpLen, PNtMEM(&inBuff[U16_A_U8(tag00AELenOff , 3)]), 8, ALG_DES_MAC8_ISO9797_1_M2_ALG3, MODE_VERIFY, U8_OR_U8(ALG_DES , SBC_VERIFY) );
		//result = KSCASDAUTDesSig.verify(inBuff, inOffset, tmpLen, inBuff, (u16)(tag00AELenOff + 3), (u16)8);
		if (result == true) {
			// decrypt the KS.AP.CT using KS.CASD.CT
			//KSCASDCTDesCipher.init(KSCASDCTDesKey, Cipher.MODE_DECRYPT);
			retLength = nativeDESChiperDoFinalCal(keySetObj, KS_CASD_CT_KEY_ID, PNtMEM(&inBuff[U16_A_U8(tag80B8LenOff , 1)]), 16, PNtMEM(&outBuff[outOffset]), MEM_NULL, MEM_NULL, ALG_DES_ECB_NOPAD, MODE_DECRYPT, SBC_INIT);
			//retLength = KSCASDCTDesCipher.doFinal(inBuff, (u16)(tag80B8LenOff + 1), (u16)16, outBuff, outOffset);
		}
		break;

	case U32t16(0x8010): // DGI '8010'
		// Push Mode A/B: this DGI contains the encrypted (using SK.CASD.CT) secure channel keys for AP.
//			if (SKCASDCTRsaKey.isInitialized() == false) {
//				throw_error(UNINITIALIZED_KEY);
//			}

		inOffset = U16_A_U8(inOffset , 2); // offset for the length of encrypted secure channel keys.
		if (inBuff[inOffset] == 0x81) {
			U16_INC(inOffset);
			tmpLen = U16t8(U16_AND_U16(U8t16(inBuff[inOffset]) , 0x00FF));
			U16_INC(inOffset); // offset for the cipher.
		} else {
			tmpLen = U16t8(U16_AND_U16(U8t16(inBuff[inOffset]) , 0x00FF));
			U16_INC(inOffset); // offset for the cipher.
		}

		// decrypt the secure channel keys.
		//SKCASDCTRsaCipher.init(SKCASDCTRsaKey, Cipher.MODE_DECRYPT);
		bKeyType = checkAndGetKeyType(keySetObj, SK_CASD_CT_KEY_ID);
		retLength = nativeRSAChiperDoFinalCal(keySetObj, bKeyType, SK_CASD_CT_KEY_ID, PNtMEM(&inBuff[inOffset]), tmpLen, PNtMEM(&outBuff[outOffset]), 0x00, ALG_RSA_PKCS1, MODE_DECRYPT);
		//retLength = SKCASDCTRsaCipher.doFinal(inBuff, inOffset, tmpLen, outBuff, outOffset);
		break;
	default:
		break;
	}

	return retLength;

}

void CASDAPINativeMethod(void)
{
	u8 					X_A status;
	u8 					X_A theMode;
	u16 				X_A sigOffset;
	u16 				X_A inLength;
	u16 				X_A inOffset;
	u16 				X_A retLen;
	jref 				X_A CASDAppRef;
	memref 				X_A inBuff;
	memref 				X_A sigBuff;

#define theMode sigOffset
#define isPk inLength
#define Scenario inOffset	

	CASDAppRef 			= popReference();
	status 				= U16t8(popShort());
	sigOffset 			= popShort();
	sigBuff 			= locateObjectInMemory(popReference());
	inLength	 		= popShort();
	inOffset 			= popShort();
	inBuff 				= locateObjectInMemory(popReference());



	if(CASD_INIT != status)
	{
		check_null(MEMtPF8(inBuff));

		check_bounds_interval(MEMtPF8(inBuff), inOffset, inLength);
		//inBuff = (memref)PX8_A_U16(get_array_data((u8 *)inBuff), inOffset);
		inBuff = get_array_data(MEMtPF8(inBuff));
		
		if (isInE2P(inBuff))
		{
			memRefresh();
		}
		if(CASD_UPDATE != status)
		{
			/*
			if (TYPE_RSA_CRT_PRIVATE == g_RSAKeyStore.bRSAKeyType)
			{
				retLen = g_RSAKeyStore.wPrimeLength << 1;
			}
			else
			{
				retLen = g_RSAKeyStore.wModulusNLength;
			}
			*/

			check_null(MEMtPF8(sigBuff));

			//check_bounds_interval(sigBuff, sigOffset, retLen);
			//sigBuff = (memref)PX8_A_U16(get_array_data((u8 *)sigBuff), sigOffset);
			sigBuff = PFtMEM(get_array_data(MEMtPF8(sigBuff)));
		}
	}

	switch(status)
	{	
		case CASD_INIT:
			CASD_init(U16t8(theMode), U16t8(isPk), U16t8(Scenario), CASDAppRef);
		break;
		case CASD_SIGN:
			retLen = CASD_sign(MEMtPN8(inBuff), inOffset, inLength, MEMtPN8(sigBuff), sigOffset, CASDAppRef);
		break;
		case CASD_UPDATE:
			CASD_update(MEMtPN8(inBuff), inOffset, inLength, CASDAppRef);
		break;
		case CASD_RECOVER:
			retLen = CASD_recoverKey(MEMtPN8(inBuff), inOffset, inLength, MEMtPN8(sigBuff), sigOffset, CASDAppRef);
		break;
	}

	pushShort(retLen);
}
#endif

