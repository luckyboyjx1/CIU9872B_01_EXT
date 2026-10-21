/*
 * Crypto_API.h
 *
 *  Created on: 2014-2-28
 *      Author: jiangxiaohui
 */

#ifndef CRYPTO_API_H_
#define CRYPTO_API_H_

#include "Crypto_Constant.h"
#include "Crypto_common_API.h"
#include "halCryptoProfile.h"
#include "RNG_API.h"
#include "EllipseCurve_API.h"
#include "SecureCrypto.h"

//Card Specification v2.2 Version 1.0.1 ¨C Amendment E :4.5 Preloaded ECC Curve Parameters -> Table 4-3: Key Parameter Reference Values
//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 
#define ECCCure_P_256_KeyParaRef                0x00
#define ECCCure_P_384_KeyParaRef                0x01
#define ECCCure_P_521_KeyParaRef                0x02
#define ECCCure_brainpoolP256r1_KeyParaRef      0x03
#define ECCCure_brainpoolP256t1_KeyParaRef      0x04
#define ECCCure_brainpoolP384r1_KeyParaRef      0x05
#define ECCCure_brainpoolP384t1_KeyParaRef      0x06
#define ECCCure_brainpoolP512r1_KeyParaRef      0x07
#define ECCCure_brainpoolP512t1_KeyParaRef      0x08
#define ECCCure_RP256V1_KeyParaRefF             0x40

#if WITH_SYM

	#if WITH_DES
		extern void single_DESMAC(PX8 dataIn, PM8 keyBuff, PX8 dataOut, u16 length, PM8 ICVBuff);
		extern void Triple_DES(PX8 dataIn, PM8 keyBuff, PX8 dataOut, u16 length, PM8 ICVBuff, u8 mode, u16 flag);
		extern void HARDWARE_DES(PX8 dataIn, PM8 keypointer, PX8 dataOut, u8 modeEnDe);
	#else
		#define single_DESMAC( dataIn,  keyBuff,  dataOut,  length,  ICVBuff)
		#define Triple_DES( dataIn,  keyBuff,  dataOut,  length,  ICVBuff,  mode, flag)
		#define HARDWARE_DES(dataIn, keypointer, dataOut,  modeEnDe)
	#endif // WITH_DES


	#if WITH_AES
		extern void AESCryptForUICC(u8 *dataIn, u8 *dataOut, u8 *key, u8 KeyByteLen, u8 mode);
		extern void KeyExpansionForUICC(u8 *key, u8 KeyByteLen, u8 *keyExpansion, u8 mode);
	#else
	#endif // WITH_AES

	extern void SymCipherUpdate(PM8 inBuff, u16 inLen, PM8 outBuff, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 mode, u8 sbcState, u8 algType, u8 symAlg) SMALL_MODE;
	extern u16 SymCipherDofinal(PM8 inBuff, u16 inLen, PM8 outBuff, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 sbcState, u8 symAlg);
	extern void SymSignatureUpdate(PM8 inBuff, u16 inLen, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 sbcState, u8 symAlg) SMALL_MODE;
	extern u8 SymSignatureDofinal(PM8 inBuff, u16 inLen, PM8 outBuff, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 sbcState, u8 macLen, u8 symAlg) SMALL_MODE;
#else
	#define SymCipherUpdate( inBuff,  inLen,  outBuff,  keyBuff,  keyBitLen,  ICVBuff,  SYMBuff,  mode,  sbcState,  algType,  symAlg) 
	#define SymCipherDofinal( inBuff,  inLen,  outBuff,  keyBuff,  keyBitLen,  ICVBuff,  SYMBuff,  algType,  mode,  sbcState,  symAlg)					0
	#define SymSignatureUpdate( inBuff,  inLen,  keyBuff,  keyBitLen,  ICVBuff,  SYMBuff,  algType,  mode,  sbcState,  symAlg) 
	#define SymSignatureDofinal( inBuff,  inLen,  outBuff,  keyBuff,  keyBitLen,  ICVBuff,  SYMBuff,  algType,  mode,  sbcState,  macLen,  symAlg) 		0
#endif /* WITH_SYM */

#if WITH_HASH
	extern void HASHUpdate(PM8 inBuff, u16 inLen, u8 mdState, PM8 mdBuff, PM8 tmpBuff, u8 algType);
	extern void HASHDoFinal(PM8 inBuff, u16 inLen, PM8 outBuff, u8 mdState, PM8 mdBuff, PM8 tmpBuff, u8 algType);
#else
	#define HASHUpdate( inBuff,  inLen,  mdState,  mdBuff,  tmpBuff,  algType)
	#define HASHDoFinal( inBuff,  inLen,  outBuff,  mdState,  mdBuff,  tmpBuff,  algType)
#endif /* WITH_HASH */

#if WITH_RSA
	extern u8 DoRSAOpertaion(u8 algType, u16 keyLen) SMALL_MODE;
	extern void RSAPreSign(PM8 inBuff, u16 inLen, PM8 tmpSrcBuff, u16 tmpBuffLen, PM8 mdBuff, PM8 tmpBuff, u8 algType, u16 keyLen,u16 sigLen)		SMALL_MODE;
	extern u8 RSAVerify(PM8 inBuff, u16 inLen, PM8 tmpSrcBuff, u16 tmpBuffLen, PM8 mdBuff, PM8 tmpBuff, u8 algType, u16 keyLen,u16 preHashFlag)		SMALL_MODE;
	extern void RSA_storeMEKey(PM8 pbExp, PM8 pbModulusN, u16 wExpLength, u16 wModulusNLength, u8 bRSAKeyType);
	extern void RSA_storeCRTKey(PM8 pbParmP, PM8 pbParmQ, PM8 pbParmDP1, PM8 pbParmDQ1, PM8 pbParmInvQ, u16 wModulusNLength, u16 wPrimeLength, u8 bRSAKeyType);
	extern void RSA_storeKEYGENCRTKey(PM8 pbParmP, PM8 pbParmQ, PM8 pbParmDP1, PM8 pbParmDQ1, PM8 pbParmInvQ, u16 primeLen)    SMALL_MODE;
	extern void RSA_storeKEYGENPriKey(PM8 pbModulusN, PM8 pbParmD, u16 primeLen);

	#define RSA_Get_OutData()							g_CryptoData.pbBuffer
	#define RSA_Get_OutDataLength()						g_CryptoData.wBufferLen
	#define RSA_GetRSAKeyType()							g_RSAKeyStore.bRSAKeyType
	#define RSA_GetModuluLength()						g_RSAKeyStore.wModulusNLength
	#define RSA_GetCRTParmLength()						g_RSAKeyStore.wPrimeLength
	#define RSA_Set_InData(inDataBuffer)				g_CryptoData.pbBuffer 	= inDataBuffer
	#define RSA_Set_InDataLength(inDataLength)			g_CryptoData.wBufferLen = inDataLength

	extern u16 RSA_CipherUpdate(PM8 inBuff, u16 inLen, PM8 tmpSrcBuff, u16 accOffset, u16 tmpMaxLen, u8 algType, u8 mode);
	extern u16 RSA_CipherDofinal(PM8 inBuff, u16 inLen, PM8 tmpSrcBuff, u16 accOffset, u8 algType, u8 mode, u8 rsaState);
	extern u16 RSA_Signature(PM8 inBuff, u16 inLen, PM8 sigBuff, PM8 tmpSrcBuff, PM8 mdBuff, PM8 tmpBuff, u16 accLen, u16 sigLen, u8 algType, u8 mode,u16 preHashFlag);
	/*-------------------------------------------------------------------------------------------------  */
	// RSA KEY GEN
	/*-------------------------------------------------------------------------------------------------	 */
	#if WITH_RSA_KEY_GEN
		extern void RSAKeyGenCore(u8 storeKeyType);
	#else
		#define RSAKeyGenCore(storeKeyType)
	#endif //WITH_RSA_KEY_GEN
	/*-------------------------------------------------------------------------------------------------  */
	// MR API
	/*-------------------------------------------------------------------------------------------------	 */
	#if WITH_SIGNMR
		extern u16 msgRecoverAlg(PM8 sigAndRecDataBuff, u16 sigLength);
		extern void signMRDofinalAlg(PM8 inBuff, u16 inLength, PM8 sigBuff, u8 algType);
		extern void signMRUpdateFinalAlg(PM8 inBuff, u16 inLength, PM8 sigBuff, u8 algType, PM8 tmpSrcBuff, u16 tmpBuffLen, PM8 mdBuff, PM8 tmpBuff);
		extern boolean signMRVerifyAlg(PM8 inBuff, u16 inLength, PM8 recoverBuff, u16 recoverDataLen, PM8 mdBuff, PM8 tmpBuff, u8 algType);
	#else
		#define msgRecoverAlg( sigAndRecDataBuff,  sigLength)																   0
		#define signMRDofinalAlg( inBuff,  inLength,  sigBuff,  algType)
		#define signMRUpdateFinalAlg( inBuff,  inLength,  sigBuff,  algType,  tmpSrcBuff,  tmpBuffLen,  mdBuff,  tmpBuff)
		#define signMRVerifyAlg( inBuff,  inLength,  recoverBuff,  recoverDataLen,  mdBuff,  tmpBuff,  algType)       		   0
	#endif	//  WITH_SIGNMR

#else  //WITH_RSA
	#define DoRSAOpertaion( algType,  keyLen) 																		0
	#define RSAPreSign( inBuff,  inLen,  tmpSrcBuff,  tmpBuffLen,  mdBuff,  tmpBuff,  algType,  keyLen)		
	#define RSAVerify( inBuff,  inLen,  tmpSrcBuff,  tmpBuffLen,  mdBuff,  tmpBuff,  algType,  keyLen)				0
	#define RSA_storeMEKey( pbExp,  pbModulusN,  wExpLength,  wModulusNLength,  bRSAKeyType)
	#define RSA_storeCRTKey( pbParmP,  pbParmQ,  pbParmDP1,  pbParmDQ1,  pbParmInvQ,  wModulusNLength,  wPrimeLength,  bRSAKeyType)
	#define RSA_storeKEYGENCRTKey( pbParmP,  pbParmQ,  pbParmDP1,  pbParmDQ1,  pbParmInvQ ,primeLen)
	#define RSA_storeKEYGENPriKey( pbModulusN,  pbParmD, primeLen);
	#define RSA_GetModuluLength()				0
	#define RSA_GetCRTParmLength()				0
	#define RSA_Set_InData( inDataBuffer)
	#define RSA_Set_InDataLength( inDataLength)
	#define RSA_Get_OutData()					0
	#define RSA_Get_OutDataLength()				0
	#define RSA_GetRSAKeyType()					0
	#define RSAKeyGenCore(storeKeyType)
	#define RSA_CipherUpdate(inBuff, inLen, tmpSrcBuff, accOffset, tmpMaxLen, algType, mode)				  0
	#define RSA_CipherDofinal(inBuff, inLen, tmpSrcBuff, accOffset, algType, mode, rsaState)		  0
	#define RSA_Signature(inBuff, inLen, sigBuff, tmpSrcBuff, mdBuff, tmpBuff, accLen, sigLen, algType, mode) 0
#endif	//  WITH_RSA

#if WITH_SM2			  	
	#define SM2_storePubKey(sm2PubKey)									g_SM2KeyStore.publicKeyBuffer = sm2PubKey
	#define SM2_storePriKey(sm2PriKey)									g_SM2KeyStore.privteKeyBuffer = sm2PriKey
	#define SM2_getPubKey()												g_SM2KeyStore.publicKeyBuffer
	#define SM2_getPriKey()												g_SM2KeyStore.privteKeyBuffer	
	extern void SM2_ECP_GetZa(PM8 IDa, u8 ID_len, PM8 publicKeyBuffer, PM8 ZaOutBuff);
	extern void SM2_ECP_GetE(PM8 inBuff, u16 inLen, PM8 ZA, PX8 EOutBuff);			
	extern void SM2_ECP_KDF(PX8 Z, u16 klen, PX8 outbuff) SMALL_MODE;
	extern void SM2_ECP_KDF2(PX8 Z, u16 zlen, u16 klen, PX8 outbuff) SMALL_MODE;
	extern u16 SM2_Signature(PM8 inBuff, u16 inLen, PM8 sigBuff, PM8 Za, PM8 mdBuff, PM8 tmpBuff,  u8 mode);
	extern u16 SM2_SigVerifyWithNoSM3(PM8 inBuff, u16 inLen, PM8 sigBuff, u16 sigLen, PM8 tmpSrcBuff, u16 accOffset, u8 mode, u8 sm2State);
	extern u16 SM2_CipherDofinal(PM8 inBuff, u16 inLen,  PM8 outBuff, PM8 tmpSrcBuff, u16 accOffset, u8 mode, u8 SM2State);
	extern void SM2_KeyExchange(PM8 Pa,PM8 Pb,PM8 tmpPubOfA,PM8 tmpPubOfB,PM8 priOfA,PM8 tmpPriOfA,PM8 IDa,PM8 IDb,u8 IdBytesLen_A,u8 IdBytesLen_B,PM8 outBuffer,u8 exchangeKeyLen,u8 bMode);
#else
	#define SM2_storePubKey(sm2PubKey)
	#define SM2_storePriKey(sm2PriKey)
	#define SM2_getPubKey()																				  0
	#define SM2_getPriKey()																				  0	
	#define SM2_ECP_GetZa(IDa, ID_len, publicKeyBuffer, ZaOutBuff)
	#define SM2_ECP_GetE(inBuff, inLen, ZA, EOutBuff)			
	#define SM2_ECP_KDF(Z, klen, outbuff)
	#define SM2_Signature(inBuff, inLen,  sigBuff,  Za,  mdBuff,  tmpBuff,   mode)          			  0
	#define SM2_SigVerifyWithNoSM3(inBuff, inLen, sigBuff, sigLen, tmpSrcBuff, accOffset, mode, sm2State) 0
	#define SM2_CipherDofinal( inBuff,  inLen,  outBuff, tmpSrcBuff, accOffset, mode, SM2State)           0
	#define SM2_KeyExchange(Pa,Pb,tmpPubOfA,tmpPubOfB,priOfA,tmpPriOfA,IDa,IDb,IdBytesLen_A,IdBytesLen_B,outBuffer,exchangeKeyLen,bMode)  0
#endif

#if WITH_SOFT_SM2
	extern void SM2_ECP_GenKeyPair(PM8 privateKey, PX8 tempPublicKey);
	extern void SM2_ECP_GetKey(PX8 prikey, PX8 publickey, u8 isgenkeypair);
	extern void SM2_ECP_Sign(PX8 E_Buff, PX8 privteKeyBuffer, PM8 rs);
	extern u8 SM2_ECP_Verify(PM8 E, PM8 publicKey, PM8 rs) SMALL_MODE;
	extern void SM2_ECP_Encrypt(PX8 inBuff, u16 inLen, PM8 publickeyBuff, PM8 outBuff);
	extern u8 SM2_ECP_Decrypt(PM8 inBuff, u16 inLen, PM8 privatekeybuff, PM8 outBuff);
#endif // WITH_SOFT_SM2	

#if WITH_HMAC
	extern void HMACInit(PM8 inputKey, u16 inputKeyLen, PM8 tmpBuff, PM8 mdBuff, PM8 hMacGenKeyBuffer, u8 hmacType);
	extern void HMACUpdate(PM8 inputData, u16 inputDataLen, PM8 tmpBuff, PM8 mdBuff, u8 hmacType);
	extern u8 HMACDofinal(PM8 inputData, u16 inputDataLen, PM8 outputData, PM8 mdBuff, PM8 tmpBuff, PM8 hMacGenKeyBuffer, u8 algType);
	extern u8 GeneralHMAC(PM8 inputKey, u16 inputKeyLen, PM8 inputData, u16 inputDataLen,PM8 outputData, u8 outputDataLen, u8 hmacType);
#else
	#define HMACInit( inputKey,  inputKeyLen,  tmpBuff,  mdBuff,  hMacGenKeyBuffer,  hmacType)
	#define HMACUpdate( inputData,  inputDataLen,  tmpBuff,  mdBuff,  hmacType)
	#define HMACDofinal( inputData,  inputDataLen,  outputData,  mdBuff,  tmpBuff,  hMacGenKeyBuffer,  algType)			0
	#define GeneralHMAC( inputKey,  inputKeyLen,  inputData,  inputDataLen, outputData,  outputDataLen,  hmacType)		0
#endif //WITH_HMAC

#if WITH_CRC
	extern void CRCCalAlg(PM8 inBuff, u16 inLen, u8 algType, u8 crcState, PM8 crcBuff, PM8 outBuff)	;
	extern u32 CRCDoCal(PM8 inBuff, u16 inLen, u32 initValue, u8 algType, u8 mode);
	extern void setKeyChecksumAlg(PM8 keyBuff, u16 keyLen);
#else
	#define  CRCCalAlg( inBuff,  inLen,  algType,  crcState,  crcBuff,  outBuff)
	#define  CRCDoCal( inBuff,  inLen,  initValue,  algType,  mode)						0
	#define  setKeyChecksumAlg( keyBuff,  keyLen)
#endif  //end WITH_CRC

#define CBC_Init_Decrypt		(CBC_Decrypt | 0x10)
#define CBC_Init_Encrypt		(CBC_Encrypt | 0x10)
extern u8 I_A G_AESKeyByteLen; //AESkey的长度
//extern void AESCryptForUICC(u8 *dataIn, u8 *dataOut, u8 *key, u8 KeyByteLen, u8 mode);
extern void AESEncryptForUICC(u8 *dataIn, u8 *dataOut, u8 *keyExpansion);
extern void AESDecryptForUICC(u8 *dataIn, u8 *dataOut, u8 *keyExpansion);
extern void KeyExpansion(u8 *key, u8 *keyExpansion);
extern u8 CheckAesKeyLength(u8 keyLength);
extern u8 setECDomainParams(u16 keyRef); //merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 修改参数类型为u16
extern void setECPubOrPrivKeyRef(u8 keyType, u8 *privateKey, u8 *publicKey);
extern u8 checkPointOnEccCurve(u8 *publicKey, u8 publicKeyLen, u8 keyRef);
#if WITH_ECC
extern ECKeyRefStore g_ECKeyStore;
#endif
extern void CmacAlgorithmCal(PM8  inBuff,u16  inLen,PM8  outBuff, PX8  keyBuff, u16  keyBitLen,PX8  IcvBuf,PX8  subFirstKeyBuf,PX8  subSecondKeyBuf,u8  mode,u8  symAlg,u8  cmacLastBlock);
extern void SingleDESForUICC(u8 * pbSrcStr, u8 keyContext[8], u8 * pbTgtStr, u8 bMod, u16 lgth);
extern void TripleDESForUICC(u8 * pbSrcStr,u8 keyContext[16],u8 * pbTgtStr, u8 bMod, u16 lgth, u8 keyLength);
extern void symCmacAlgorithmCal(PM8 inBuff1, u16 inLen1,PM8 inBuff2, u16 inLen2,PM8 outBuff, PM8 keyBuff, u16 keyBitLen,PM8 IcvBuf, u8 mode, u8 symAlg);

//extern void SHA1Final( unsigned char * digest, unsigned char* sourcedata,unsigned int length, memref fileAddr, u8 mode);
//extern void MD5DoFinalSingleCall(unsigned char* digest /*[16]*/, unsigned char* sourcedata,unsigned int length, u8 mode,memref fileAddr);
extern void SHA1Final( PM8 digest, PM8 sourcedata,u16 length, PM8 fileAddr, u8 mode);
extern void SHA256DoFinalSingleCall(PM8 digest, PM8 sourcedata,u16 length, u8 mode, PM8 fileAddr) ;
extern void MD5DoFinalSingleCall(PM8 digest /*[16]*/, PM8 sourcedata,u16 length, u8 mode,PM8 fileAddr);
//#if WITH_SM4
//	extern void SM4CmacAlgorithmCal(PM8 inBuff1, u16 inLen1,PM8 inBuff2, u16 inLen2,PM8 outBuff, PM8 keyBuff, u16 keyBitLen,PM8 IcvBuf, u8 mode, u8 symAlg);
//#else
//	#define SM4CmacAlgorithmCal(inBuff1, inLen1, inBuff2, inLen2, outBuff,  keyBuff, keyBitLen, IcvBuf, mode, symAlg)
//#endif
#endif /* CRYPTO_API_H_ */
