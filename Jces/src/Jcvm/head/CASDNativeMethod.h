/*
 * CASDNativeMethod.h
 *
 *  Created on: 2013-12-16
 *      Author: jiangxiaohui
 */

#ifndef CASDNATIVEMETHOD_H_
#define CASDNATIVEMETHOD_H_

#include "Crypto_impl.h"
#include "stddef.h"
#include "keySet.h"
#include "SCPInterface.h"
#include "casdStoreKey.h"
#include "scp10.h"
#include "global_variable.h"
#include "stack.h"
#include "CASDNativeMethod.h"


#define 	CASD_INIT              	 	0x01
#define 	CASD_SIGN               	0x02
#define 	CASD_UPDATE               	0x04
#define 	CASD_RECOVER               	0x08

    /*****for amd A transient buff mask*********/
#define	 Scenario1_Mask 		0x01
#define	 Scenario2A_Mask		0x02
#define	 Scenario2B_Mask		0x04
#define	 ScenarioPK_Mask		0x01
#define	 ScenarioNPK_Mask		0x00

#define MODE_KEY_RECOVERY 		0x02


/*
 *
 *
 *
 * SKCASDAUTRsaPriKey // SKCASDAUTRsaKeySig
 * SKCASDAUTRsaKey // SKCASDAUTRsaKeySig
 * PKCAAUTRsaKey  // PKCAAUTRsaSignMR
 * PKCAAUTRsaKey  //  PKCAAUTRsaSignMR
 * SKCASDCTRsaKey  // SKCASDCTRsaCipher
 * SKCASDCTRsaPriKey  // SKCASDCTRsaCipher
 * SKCASDCTRsaKey  // SKCASDCTRsaCipher
 * SKCASDCTRsaPriKey  // SKCASDCTRsaCipher
 * KSCASDAUTDesKey  // KSCASDAUTDesSig
 * KSCASDAUTDesKey  //  KSCASDAUTDesSig
 * KSCASDCTDesKey  //  KSCASDCTDesCipher
 *
 * private DESKey KSCASDAUTDesKey;
 * private DESKey KSCASDCTDesKey;
 * private Signature KSCASDAUTDesSig;
 * private Cipher KSCASDCTDesCipher;
 * private SignatureMessageRecovery PKCAAUTRsaSignMR;
 * private RSAPublicKey PKCAAUTRsaKey;//用于验证CERT.AP.CT
 *
 * private Signature SKCASDAUTRsaKeySig;//用于以下签名
 * private RSAPrivateCrtKey SKCASDAUTRsaKey;//用于签名加密的RGK
 * private RSAPrivateKey SKCASDAUTRsaPriKey;//用于SF格式签名加密的RGK
 * private RSAPrivateCrtKey SKCASDCTRsaKey;//crt格式
 * private RSAPrivateKey SKCASDCTRsaPriKey;//SF格式
 * private Cipher SKCASDCTRsaCipher;
 *
 * */


/*
 *
 * Key identifier '01' is reserved for PK.CA.AUT
 * Key identifier '02' is reserved for SK.CASD.AUT (PK) and KS.CASD.AUT (Non - PK)
 * Key identifier '03' is reserved for SK.CASD.CT (PK) and KS.CASD.CT (Non- PK)
 *
 * */
// RSA key type
#define  PK_CA_AUT_KEY_ID		KEY_ID_01
// PKCAAUTRsaKey  //  PKCAAUTRsaSignMR

#define  SK_CASD_AUT_KEY_ID		KEY_ID_02
// SKCASDAUTRsaPriKey // SKCASDAUTRsaKeySig
// SKCASDAUTRsaKey // SKCASDAUTRsaKeySig

#define  SK_CASD_CT_KEY_ID		KEY_ID_03
//	 SKCASDCTRsaKey  // SKCASDCTRsaCipher
//	 SKCASDCTRsaPriKey  // SKCASDCTRsaCipher

// DES key type
#define  KS_CASD_AUT_KEY_ID		KEY_ID_02
//	 KSCASDAUTDesKey  //  KSCASDAUTDesSig

#if UICC1_0
#define  KS_CASD_CT_KEY_ID		KEY_ID_01
#else
#define  KS_CASD_CT_KEY_ID		KEY_ID_03
#endif //UICC1_0
//	 KSCASDCTDesKey  //  KSCASDCTDesCipher


#define KEY_TYPE_NULL		                           0x00
#if GP__AMENDMENT__A__FOR_MASK
	extern u8 checkAndGetKeyType(PN8 keySetObj, u8 keyId);
	extern void CASD_init(u8 theMode, u8 isPk, u8 Scenario, jref CASDAppRef);
	extern u16 CASD_sign(u8 * inBuff, u16 inOffset, u16 inLength, u8 * sigBuff, u16 sigOffset, jref CASDAppRef);
	extern void CASD_update(u8 * inBuff, u16 inOffset, u16 inLength, jref CASDAppRef);
	extern u16 CASD_recoverKey(u8 * inBuff, u16 inOffset, u16 inLength, u8 * outBuff, u16 outOffset, jref CASDAppRef);
	extern void CASDAPINativeMethod(void);
#else
	#define  checkAndGetKeyType( keySetObj,  keyId)                                              0
	#define  CASD_init( theMode,  isPk,  Scenario,  CASDAppRef)
	#define  CASD_sign(  inBuff,  inOffset,  inLength,   sigBuff,  sigOffset,  CASDAppRef)       0
	#define  CASD_update(  inBuff,  inOffset,  inLength,  CASDAppRef)
	#define  CASD_recoverKey(inBuff, inOffset,  inLength,outBuff,  outOffset,  CASDAppRef)       0
	#define  CASDAPINativeMethod()
#endif

#endif /* CASDNATIVEMETHOD_H_ */
