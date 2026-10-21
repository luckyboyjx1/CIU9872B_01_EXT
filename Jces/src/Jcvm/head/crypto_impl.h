#ifndef CRYPTO_IMPL_H
#define CRYPTO_IMPL_H

#include "Crypto_Constant.h"
#include "Crypto_API.h"
#include "profile.h"


#define 	SYM_OTHER_MODE										0x00
#define 	SYM_NOPAD_MODE										0x01
#define 	SYM_9797M1_MODE										0x02

#if VM_WITH_SYM
	extern void CipherUpdate(u8 flag)					SMALL_MODE;
	extern void CipherDofinal(u8 flag)					SMALL_MODE;
	extern void SignatureUpdate(u8 flag)				SMALL_MODE;
	extern void SignatureDofinal(u8 flag)				SMALL_MODE;
	extern void secureStoreSensitiveData(void);
	extern void secureGetSensitiveData(void);
#else
	#define   CipherUpdate( flag)			
	#define   CipherDofinal( flag)			
	#define   SignatureUpdate( flag)			
	#define   SignatureDofinal( flag)
	#define   secureStoreSensitiveData()
	#define   secureGetSBCKey()
#endif



#if VM_WITH_RANDOMDATA
	extern void genRandomData(void);
#else
	#define  genRandomData()
#endif



#if VM_WITH_RSA
	extern void storeRSAMEKeyRef(void);
	extern void storeRSACrtKeyRef(void);
	extern void storeRSAKeyPair(void);
	extern void RSACipherUpdate(void);
	extern void RSACipherDofinal(void);
	extern void RSASignatureDofinal(u8 mode)			SMALL_MODE;
	extern void RSASignatureUpdateFinal(u8 mode)		SMALL_MODE;
	extern void calcModuleLengthAndExponent(void);
	extern void secureSetPKIKey(void);
	extern void secureGetPKIKey(void);
//	extern void SetKeyPinv(void);
#else
	#define  storeRSAMEKeyRef()
	#define  storeRSACrtKeyRef()
	#define  storeRSAKeyPair()
	#define  RSACipherUpdate()
	#define  RSACipherDofinal()
	#define  RSASignatureDofinal( mode)		
	#define  RSASignatureUpdateFinal( mode)
	#define	 calcModuleLengthAndExponent()
	#define  secureSetPKIKey()
	#define  secureGetPKIKey()
//	#define  SetKeyPinv()
#endif




#if VM_WITH_HASH
	extern void MessageDigest(u1 isGuoMiAlg);
	extern void MDUpdate(u1 isGuoMiAlg);
	extern void MDResetNM(void);
	extern void MDInit(void);
#else
	#define  MessageDigest( isGuoMiAlg)
	#define  MDUpdate( isGuoMiAlg)
	#define  MDResetNM()
	#define  MDInit()
#endif


#if VM_WITH_CRC
	extern void CRCInit(void);
	extern void CRCUpdate(void);
	extern void CRCDofinal(void);
	extern void setKeyChecksum(void);
#else
	#define  CRCInit()
	#define  CRCUpdate()
	#define  CRCDofinal()
	#define  setKeyChecksum()
#endif


#if VM_WITH_ECC
	extern void storeECDomainParams(void);
	extern void storeECPubOrPrivKeyRef(void);
	extern void ECKeyGen(void);
	extern void ECSignatureDofinal(u8 mode);
	extern void ECSignatureUpdateFinal(void);
	extern void ECVerifyUpdateFinal(void);
	extern void ECDH(void);
#if VM_WITH_ECCMATH
	extern void ECCMathX(void);
#endif
#else
	#define  storeECDomainParams()
	#define  storeECPubOrPrivKeyRef()
	#define  ECKeyGen()
	#define  ECSignatureDofinal( mode)
	#define  ECSignatureUpdateFinal()
	#define  ECVerifyUpdateFinal()
	#define  ECDH()
	#define  ECCMathX()
#endif


#if VM_WITH_SM2
	extern void computeZa(void);
	extern void genSM2KeyPair(void);
	extern void storeSM2KeyRef(void);
	extern void SM2CipherDofinal(void);
	extern void SM2SignatureUpdateFinal(u8 mode)	SMALL_MODE;
	extern void SM2SignatureDofinal(u8 mode)		SMALL_MODE;
	extern void SM2Sig_VerifyWithNoSM3(u8 mode)		SMALL_MODE;
	extern void SM2KeyExchange(void);
	extern void SM2CalPublicByPrivate(void);
	extern void SM2ComputeZ(void);
	extern u8 CONST defaltID[16];
#else
	#define  computeZa()
	#define  genSM2KeyPair()
	#define  storeSM2KeyRef()
	#define  SM2CipherDofinal()
	#define  SM2SignatureUpdateFinal(mode)
	#define  SM2SignatureDofinal(mode)
	#define  SM2Sig_VerifyWithNoSM3(mode)
#endif //WITH_SM2


#if VM_WITH_SIGNMR
	extern void signMRDofinal(void);
	extern void signMRUpdateFinal(void);
	extern void msgRecover(void);
	extern void signMRVerify(void);
//	extern void signMRUpdateVerify(void);
#else
	#define  signMRDofinal()
	#define  signMRUpdateFinal()
	#define  msgRecover()
	#define  signMRVerify()
//	#define  signMRUpdateVerify()
#endif


#if VM_WITH_HMAC
	extern void HMAC_Init(void);
	extern void HMAC_Update(void);
	extern void HMAC_Dofinal(void);
	extern void HMAC_Verify(void);
#else
	#define  HMAC_Init(void)
	#define  HMAC_Update(void)
	#define  HMAC_Dofinal(void)
	#define  HMAC_Verify(void)
#endif


#if JCVM__VM__JAVACARDX_MATH
	extern	void isBCDNumberPreProcessor(void);
	extern	void conversionPreProcessor(u8 toFormat);
	extern	void setMaxBigNumberValue(void);
	extern	void initBigNumber(void);
	extern	void mathHelper(u8 operation);
	extern	void checkForValidBNSize(void);
	extern	void compareBigNumbers(void);
	extern	void toBytes(void);
	extern	void getByteLength(void);
#else
	#define  conversionPreProcessor( toFormat)
	#define  isBCDNumberPreProcessor()
	#define  setMaxBigNumberValue()
	#define  initBigNumber()
	#define  mathHelper( operation)
	#define  checkForValidBNSize()
	#define  compareBigNumbers()
	#define  toBytes()
	#define  getByteLength()
#endif
extern PM8 getArrayDataWithCheck(u16 outLen);
extern PM8 getArrayDataWithoutCheck(void);
extern PM8 getJavaObjectDataArray(void);
extern PM8 getJavaObjectDataRef(void);
extern PM8 GetArrayDataAndCheckBounds(PM8 inBuff, u16 inOffset, u16 inLen);
extern void secureStorePINData(void);

extern u8 GenECCKeyPair(u8 * pkBuf, u8 * skBuf, u8 keyRef);

extern void AEADCipherInit(u8 symAlg);  //remove warning,dengyiwen,2022/9/22
extern void AEADCipherTag(u8 symAlg);  //remove warning,dengyiwen,2022/9/22
#endif
