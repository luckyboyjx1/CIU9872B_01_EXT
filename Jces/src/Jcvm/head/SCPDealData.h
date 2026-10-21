/*
 * SCPDealData.h
 *
 *  Created on: 2013-9-24
 *      Author: jiangxiaohui
 */

#ifndef SCPDEALDATA_H_
#define SCPDEALDATA_H_

#include "profile.h"
#include "typejava.h"
#include "cryptoProfile.h"

#define DES_KEY_PADDING_OFF				0x03
#define AES_KEY_PADDING_OFF				0x04

#define IS_NEED_CHECK_INLENGTH_NO		0x00
#define IS_NEED_CHECK_INLENGTH_YES		0x01

//#define OPTION_ENCRYPTO					0x02
//#define OPTION_DECRYPTO					0x01

#define CRYPTO_PADDING_80				0x80

#define	TAG_81							0x81
#define	TAG_82							0x82

#if GP__SCP
	extern u8 add80Padding(PX8 SrcAddr, u8 len, u8 blockSize);
	extern u8 compareRamData(PX8 firstRamData, PX8 secondRamData, u8 len);
	extern u8 verifyMAC(PX8 apduBuf);
	extern u8 decryptCommandData(PX8 apduBuf);
	extern u16 decryptAndVerifyMAC(PX8 apduBuf, u8 bSecureLevel,u8 needDecryptCommandData,u8 needVerifyMAC);
	extern void RmacUpdate(PX8 SrcAddr, u16 length);
	extern void generateCardChallenge(PN8 keyData, u8 comeInOrder);
	extern u8 generateOrCompareCardCryptogram(u8 bOp );
	extern void generateSecureChannelNumber(void);
	extern void cipherSCP(PX8 dataIn, PX8 keyBuff, PX8 dataOut, u16 length, u8 * ICVBuffer, u8 mode, u8 blockSize, u8 isNeedDoPadding);
	extern u16 dealcipherData(PX8 inBuffer, PX8 keyBuff, PX8 outBuffer, u16 inlength, u8 mode, u8 blockSize);
	extern u16 cipherData(PX8 inBuffer, u16 inlength, jref theKey, u8 bOp, u8 isNeedCheckInlength);
	extern void getRSAKeyRef(u8 keyType, PN8 keySetObj, u8 keyId, PX8 keyAddress);
#else
	#define add80Padding(SrcAddr, len, blockSize)																				0
	#define compareRamData(firstRamData, secondRamData, len)																	0
	#define verifyMAC(apduBuf)																											0
	#define decryptCommandData(apduBuf)																								0
	#define decryptAndVerifyMAC(apduBuf, bSecureLevel, needDecryptCommandData, needVerifyMAC)											0
	#define RmacUpdate(SrcAddr, length)
	#define generateCardChallenge(keyData, comeInOrder)
	#define generateOrCompareCardCryptogram(bOp)																				0
	#define generateSecureChannelNumber()
	#define cipherSCP(dataIn, keyBuff, dataOut, length, ICVBuffer, mode, blockSize, isNeedDoPadding)
	#define dealcipherData(inBuffer, keyBuff, outBuffer, inlength, mode, blockSize)												0
	#define cipherData(inBuffer, inlength, theKey, bOp, isNeedCheckInlength)													0
	#define getRSAKeyRef(keyType, keySetObj, keyId)
#endif

#if (GP__SCP && VM_WITH_DES)
	extern u16 nativeDESSignatureDoFinalCal(PN8 keySetObj, u8 keyId, memref inBuff, u16 inLen, memref sigBuff, u16 sigLen, u8 algType, u8 mode, u8 flag);
	extern u16 nativeDESChiperUpdateCal(PN8 keySetObj, u8 keyId, memref inBuff, u16 inLen, memref sigBuff, PX8 ICVBuff, PX8 tmpBuff, u8 mode, u8 sbcState);
	extern u16 nativeDESChiperDoFinalCal(PN8 keySetObj, u8 keyId, memref inBuff, u16 inLen, memref sigBuff, PX8 ICVBuff, PX8 tmpBuff, u8 algType, u8 mode, u8 sbcState);
#else
	#define nativeDESSignatureDoFinalCal(keySetObj, keyId, inBuff, inLen, sigBuff, sigLen, algType, mode, flag)					0
	#define nativeDESChiperUpdateCal(keySetObj, keyId, inBuff, inLen, sigBuff, ICVBuff, tmpBuff, mode, sbcState)				0
	#define nativeDESChiperDoFinalCal(keySetObj, keyId, inBuff, inLen, sigBuff, ICVBuff, tmpBuff, algType, mode, sbcState)		0
#endif

#if (GP__SCP && VM_WITH_RSA)
	extern u16 nativeRSAChiperDoFinalCal(PN8 keySetObj, u8 keyType, u8 keyId, memref inBuff, u16 inLen, memref sigBuff, u16 sigLen, u8 algType, u8 mode);
	extern u16 nativeRSASignatureDoFinalCal(PN8 keySetObj, u8 keyType, u8 keyId, PN8 inBuff, u16 inLen, PN8 sigBuff, u16 sigLen, u8 algType, u8 mode);
#else
	#define nativeRSAChiperDoFinalCal(keySetObj, keyType, keyId, inBuff, inLen, sigBuff, sigLen, algType, mode)					0
	#define nativeRSASignatureDoFinalCal(keySetObj, keyType, keyId, inBuff, inLen, sigBuff, sigLen, algType, mode)				0
#endif

#if (GP__SCP && GP__KEY_MANAGER__ECC_KEY)
	extern u16 nativeECCSignatureDoFinalCal(PN8 keySetObj, u8 keyType, u8 keyId, PN8 inBuff, u16 inLen, PN8 sigBuff, u16 sigLen, u8 mode);
#else
	#define nativeECCSignatureDoFinalCal(keySetObj, keyType, keyId, inBuff, inLen, sigBuff, sigLen, mode)		                0
#endif

//CASD及SCP10共用的函数
#if	(GP__SCP && (GP__AMENDMENT__A__FOR_MASK | GP__SCP__SCP10__FOR_MASK))
	extern u8 	getValueOffByTag(PX8 inBuffer, u8 currentLenOff);
	extern u16	getNextTagOff(u8 * inBuffer, u8 currentTagOff, u8 tagType, u16 tagName, u8 IsNeedCheckTagName);
	extern u16	getLenByTag(PX8 inBuff, u8 inOffset);
	extern u8 	scp10_recoverKey(PX8 keySetObj, u8* inBuff, u8 inOffset,u8* outBuff, u8 outOffset);
#else
	#define getValueOffByTag(inBuffer, currentLenOff)																			0
	#define getNextTagOff(inBuffer, currentTagOff, tagType, tagName, IsNeedCheckTagName)										0
	#define getLenByTag(inBuff, inOffset)																						0
	#define scp10_recoverKey(keySetObj, inBuff, inOffset, outBuff, outOffset)													0
#endif

#endif /* SCPDEALDATA_H_ */
