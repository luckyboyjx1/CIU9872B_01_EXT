/*
 * SecureCrypto.h
 *
 *  Created on: 2014Äê11ÔÂ20ÈÕ
 *      Author: jiangxiaohui
 */

#ifndef SECURECRYPTO_H_
#define SECURECRYPTO_H_

#include "SecureOperation.h"
#include "Sym_API.h"

/* Error codes */
#define ECP_ERR_OK              0xA95C
#define ECP_ERR_FAILURE         0xCA8E

#define __RSA_SECURITY__				0
#define __SYM_DFA_PROTECT__			    0

#define SC_MAX_PIN_SIZE													128

#define DEFAULT_KEY_INDEX            									0
#define MAX_KEYS                     									3
#define MAX_DES_KEY_LENGTH           									16
#define MAX_KEY_SET_VERSIONS         									0x6f
#define MAX_KEY_SET_NUM         										7
#define	LEN_OF_KEYSET_ARRAY												(MAX_KEYS * MAX_DES_KEY_LENGTH)
#define	LEN_OF_SC_KEYSET_ARRAY											((MAX_KEYS * MAX_DES_KEY_LENGTH) + 2)
#define	LEN_OF_SC_MAX_DES_KEY											(MAX_DES_KEY_LENGTH + 2)


#define	SC_PKI_CRT_KEY_P_ENCRY											0x01
#define	SC_PKI_CRT_KEY_Q_ENCRY											0x02
#define	SC_PKI_CRT_KEY_DP_ENCRY											0x04
#define	SC_PKI_CRT_KEY_DQ_ENCRY											0x08
#define	SC_PKI_CRT_KEY_QINV_ENCRY										0x10

#define	SC_PKI_CRT_KEY_NEED_ENCRY_MASK									(SC_PKI_CRT_KEY_P_ENCRY | SC_PKI_CRT_KEY_Q_ENCRY | SC_PKI_CRT_KEY_DP_ENCRY | SC_PKI_CRT_KEY_DQ_ENCRY | SC_PKI_CRT_KEY_QINV_ENCRY)


#define	SC_PKI_CRT_KEY_P_MASK											0x01
#define	SC_PKI_CRT_KEY_Q_MASK											0x02
#define	SC_PKI_CRT_KEY_DP_MASK											0x04
#define	SC_PKI_CRT_KEY_DQ_MASK											0x08
#define	SC_PKI_CRT_KEY_QINV_MASK										0x10

#define	SC_PKI_ME_KEY_M_ENCRY											0x01
#define	SC_PKI_ME_KEY_D_ENCRY											0x02
#define	SC_PKI_ME_KEY_NEED_ENCRY_MASK									(0)


#define SC_PKI_IN_PLAIN_TEXT											0x11
#define SC_PKI_IN_ENCRY_TEXT											0x22
#define SC_PKI_IN_DEAL_TEXT												0x44

#define SC_PKI_IN_DEAL_AND_ENCRY_TEXT									(SC_PKI_IN_DEAL_TEXT | SC_PKI_IN_ENCRY_TEXT)
#define SC_PKI_IN_DEAL_AND_PLAIN_TEXT									(SC_PKI_IN_DEAL_TEXT | SC_PKI_IN_PLAIN_TEXT)
#define SC_PKI_IN_TEXT_MASK												(SC_PKI_IN_PLAIN_TEXT | SC_PKI_IN_ENCRY_TEXT | SC_PKI_IN_DEAL_TEXT)

#define	SC_INIT_GET_KEY													0x01

//-------------------------------------------------------------------------------------------------
// Define RSA Key Reference Store Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
	PM8 crtpbMaskIv;
	PM8 crtpbCrcValue;
	PM8 pbMaskIv;
	PM8 pbCrcValue;
} SC_RSAKeyRefStore;

//-------------------------------------------------------------------------------------------------
// Define RSA Key GEN Store Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
	PM8 pbMaskIv;
	PM8 pbCrcValue;
} SC_RSAKeyGenParam;

typedef struct
{
	u16 pChecksum;
	u16 qChecksum;
}RSACRTKeyParCheckSum;

typedef struct
{
	PM8 sensitiveData;
	PM8 inBuff;
	u32 Len;
	u8 checksum;
} SC_SensitiveDataParaStruct;

#define SENSITIVE_DATA_MASKED	         0xA953
#define SENSITIVE_DATA_MASK              0x5A7A
#define SENSITIVE_DATA_NOT_MASK          0xA5A7
#define SENSITIVE_DATA_MASK_NOT_CHECK    0x95A9

#define GEN_RANDOM_NUMBER_AGAIN		1
#define GEN_RANDOM_NUMBER_FINISH	0

typedef struct
{
	PM8 sensitiveData;
	PM8 inBuff;
	u32 Len;
	u8 checksum;
} SC_symSetKeyParaStruct;

typedef struct
{
	PM8 sensitiveData;
	PM8 outBuff;
	u16 Len;
	u16 dataChecksum;
	u8 checksum;
} SC_symGetKeyParaStruct;

typedef struct
{
	PM8 outBufBack;
	PM8 sensitiveDataBack;
	u16 outBufOffBack;
	u16 sensitiveDataOffBack;
} SC_SaveBack;

#if WITH_SECURE_DATA_STORAGE

extern RSACRTKeyParCheckSum		X_A		g_RSACRTKeyParCheckSum;
extern SC_SaveBack				X_A		scDataSaveBack;

#define SC_checkRandamReturnCode(retrunCode) {													\
												if(ECP_ERR_RNG_GETRANDOM_OK != retrunCode)		\
												{												\
													faultDetected();							\
												}												\
											 }

#define SC_checkReturnCode(retrunCode) 		 {										\
												if(ECP_ERR_OK != retrunCode)		\
												{									\
													faultDetected();				\
												}									\
											 }


extern	void SC_RNGCacheFresh(void);
extern	u8 SC_RandomCheck(PM8 outptrBak, u16 len);
extern	void SC_DealSensitiveDataMask(PX32 dataPtr, u8 loopCtr);
extern	void SC_DealDESSensitiveData(Sym_LoadKey_Param_t * myLoadKeyParam, u16 flag);
											
extern	void SC_InOutSaveBack(PM8 outBuf, PM8 sensitiveData, u16 outBufOff, u16 sensitiveDataOff);
extern	void SC_InSaveBack(PM8 sensitiveData, u16 sensitiveDataOff);
extern	void SC_InOutCheckBack(PM8 outBuff, PM8 sensitiveData);
extern	void SC_InCheckBack(PM8 sensitiveData);
extern	u16 SC_SYMgetKey_C(PM8 outBuffer, PM8 keyData, u16 length, u16 flag);
extern	u16 SC_SYMgetKey_Java(PM8 outBuffer, PM8 keyData, u16 length);
extern	u16 SC_SYMsetKey_Java(PM8 keyData, PM8 inBuffer, u16 length);
extern	u16 SC_SYMsetKey_C(PM8 keyData, PM8 inBuffer, u16 length);
extern	u16 SC_storeSensitiveDataAPI(PM8 sensitiveData, u16 sensitiveDataLength, PM8 sensitiveDesData, u16 sensitiveDesDatalength, PM8 inBuffer);
extern	u16 SC_MemCopyAPI_JAVA(PM8 t, PM8 s, u16 length, u8 copyType);
extern	u16 SC_MemCopyAPI(PM8 t, PM8 s, u16 length, u8 copyType);

//extern	void SC_storeMEKey(PM8 DesAdrr, u16 DesOffs, u8 MeKeyType);
//extern	void SC_storeCRTKey(PM8 pbParmInvQ, u16 wPrimeLength);
extern	u8 	 SC_checkKeyIsNeedENCry(u8 keyType, u8 keyStatus);
extern	void SC_PKIGetAllKey(PX8 outBuffer, u8 keyType);
extern	u16 SC_SYMsetKey(SC_symSetKeyParaStruct symSetKeyPara);
extern	u16 SC_SYMgetKey(SC_symGetKeyParaStruct symGetKeyPara);
extern	u16 SC_PKIsetKey(PX8 keytempbuff, PM8 keyData, u16 keyOffs, PM8 inBuffer, u16 keyLen, u8 keyType, u16 realKeyLen, u8 mode);
extern  u16 SC_PKIsetKey_JAVA(PX8 keytempbuff, PM8 keyData, u16 keyOffs, PM8 inBuffer, u16 keyLen, u8 keyType, u16 realKeyLen, u8 keyStatus);
extern	u16 SC_PKIgetKey(PX8 keytempbuff, PM8 outBuffer, PM8 keyData, u16 keyOffs, u16 keyLen, u8 keyType, u16 realKeyLen, u8 mode);
extern  u16 SC_PKIgetCRTKey_C(PM8 keyData, u16 keyLen);
extern	u16 SC_PKIgetKey_JAVA(PX8 keytempbuff, PM8 outBuffer, PM8 keyData, u16 keyOffs, u16 keyLen, u8 keyType, u16 realKeyLen, u8 keyStatus);
extern	u16 SC_KEYSETsetKey(PM8 keyData, u16 keyOffs, PM8 inBuffer);
extern  u16 SC_KEYSETgetKey(PM8 keyData, PM8 outBuffer);
extern  u16 SC_storeSensitiveData(SC_SensitiveDataParaStruct SensitivePara, SC_SensitiveDataParaStruct SensitiveDesPara);
//extern void RSA_storeCRTKeyAddSecurityParam(PM8 pbModular, PM8 pbParmExponent, u16 wExpLength);
extern u16 SC_getSensitiveData(SC_SensitiveDataParaStruct SensitivePara);
extern u16 SC_getSensitiveDataAPI(PM8 sensitiveData, u16 sensitiveDataLength, PM8 outBuffer);
extern void SC_KEYSETUnMask(PM8 keyData, PM8 outBuffer);
#else

#define  SC_RNGCacheFresh()
#define  SC_RandomCheck(outptrBak, len)													GEN_RANDOM_NUMBER_FINISH
#define  SC_checkRandamReturnCode(retrunCode)											retrunCode
#define  SC_checkReturnCode(retrunCode)													retrunCode
#define	 SC_DealSensitiveDataMask(dataPtr, loopCtr)
#define	 SC_DealDESSensitiveData(myLoadKeyParam, flag)     DealDESSensitiveData(myLoadKeyParam, flag)

#define	 SC_InOutSaveBack(outBuf, sensitiveData, outBufOff, sensitiveDataOff)
#define	 SC_InSaveBack(sensitiveData, sensitiveDataOff)
#define	 SC_InOutCheckBack(outBuff, sensitiveData)
#define	 SC_InCheckBack(sensitiveData)
#define	 SC_SYMgetKey_C(outBuffer, keyData, length, flag)								memCopyAtomic(outBuffer, keyData, length)
#define	 SC_SYMgetKey_Java(outBuffer, keyData, length)									memCopyAtomic(outBuffer, keyData, length)
#define	 SC_SYMsetKey_Java(keyData, inBuffer, length)									memCopyAtomic(keyData, inBuffer, length)
#define	 SC_SYMsetKey_C(keyData, inBuffer, length)									memCopyAtomic(keyData, inBuffer, length)

#define  SC_storeSensitiveDataAPI(sensitiveData, sensitiveDataLength, sensitiveDesData, sensitiveDesDatalength, inBuffer) memCopyAtomic(sensitiveDesData, inBuffer, sensitiveDesDatalength)
#define  SC_getSensitiveDataAPI(sensitiveData, sensitiveDataLength, outBuffer)  memCopyAtomic(outBuffer, sensitiveData, sensitiveDesDatalength)

//#define SC_storeMEKey(DesAdrr, DesOffs, MeKeyType)
//#define SC_storeCRTKey(pbParmInvQ, wPrimeLength)
#define	SC_checkKeyIsNeedENCry(keyType, keyStatus)										0x00
#define SC_PKIGetAllKey(outBuffer, keyType)
#define SC_SYMsetKey(storeSensitiveDataPara)                                   			(memCopyAtomic(storeSensitiveDataPara.sensitiveData, storeSensitiveDataPara.inBuff, storeSensitiveDataPara.Len),ECP_ERR_OK)
//#define SC_getSensitiveData(getSensitiveDataPara)                                       (memCopyAtomic(getSensitiveDataPara.outBuff, getSensitiveDataPara.sensitiveData, getSensitiveDataPara.Len),ECP_ERR_OK)
//#define SC_PKIsetKey(keytempbuff, keyData, keyOffs, inBuffer, keyLen, keyType, realKeyLen, mode)	(memCopyAtomic(PM8_A_U16(keyData, keyOffs), inBuffer, realKeyLen),ECP_ERR_OK)
#define SC_PKIsetKey(keytempbuff, keyData, keyOffs, inBuffer, keyLen, keyType, realKeyLen, mode)	(memCopyAtomic(PM8_A_U16(keyData, keyOffs), inBuffer, realKeyLen),ECP_ERR_OK)
#define SC_PKIsetKey_JAVA(keytempbuff, keyData, keyOffs, inBuffer, keyLen, keyType, realKeyLen, mode)	(memCopyAtomic(PM8_A_U16(keyData, keyOffs), inBuffer, realKeyLen),ECP_ERR_OK)

#define SC_PKIgetKey(keytempbuff, outBuffer, keyData, keyOffs, keyLen, keyType, realKeyLen, mode)	memCopyAtomic(outBuffer, PM8_A_U16(keyData, keyOffs), realKeyLen)
#define SC_PKIgetKey_JAVA(keytempbuff, outBuffer, keyData, keyOffs, keyLen, keyType, realKeyLen, mode)	memCopyAtomic(outBuffer, PM8_A_U16(keyData, keyOffs), realKeyLen)


#define SC_KEYSETsetKey(keyData, keyOffs, inBuffer)										memCopyAtomic(PM8_A_U8(keyData, keyOffs), inBuffer, MAX_DES_KEY_LENGTH)
#define SC_KEYSETgetKey(keyData, outBuffer)												memCopyAtomic(outBuffer, keyData, LEN_OF_KEYSET_ARRAY)
#define SC_KEYSETUnMask(keyData, outBuffer)												memCopyAtomic(outBuffer, keyData, LEN_OF_KEYSET_ARRAY)

//#define RSA_storeCRTKeyAddSecurityParam(pbModular, pbParmExponent, wExpLength)

#endif	// WITH_SECURE_DATA_STORAGE


#if SEC_MEM_OPERATION
extern	void SC_MemCopyRam2RamAPI(PX8 t, PM8 s, u16 length);
#define SC_MemCopyMem2RamAPI(t, s, length)												SC_checkReturnCode(SC_MemCopyAPI(t, s, length, ECP_COPY_TYPE_MEM_2_RAM))
//#define SC_MemCopyRom2RamAPI(t, s, length)												SC_checkReturnCode(SC_MemCopyAPI(t, s, length, ECP_COPY_TYPE_ROM_2_RAM))
#define SC_MemCopyAtomicAPI(t, s, length)												SC_checkReturnCode(SC_MemCopyAPI(t, s, length, ECP_COPY_TYPE_ATOMIC))
#define SC_MemCopyNoAtomicAPI(t, s, length)												SC_checkReturnCode(SC_MemCopyAPI(t, s, length, ECP_COPY_TYPE_NOATOMIC))

#else

#define	 SC_MemCopyRam2RamAPI(t, s, length)												ecpMem_Move(t, s, length)
#define	 SC_MemCopyMem2RamAPI(t, s, length)												memCopyToRam(t, s, length)
//#define	 SC_MemCopyRom2RamAPI(t, s, length)												ecpMem_Copy(t, s, length)
#define	 SC_MemCopyAtomicAPI(t, s, length)												memCopyAtomic(t, s, length)
#define	 SC_MemCopyNoAtomicAPI(t, s, length)											memCopyNoAtomic(t, s, length)

#endif
#endif /* SECURECRYPTO_H_ */
