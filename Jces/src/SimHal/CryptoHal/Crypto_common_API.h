/*
 * Crypto_common_API.h
 *
 *  Created on: 2014Äê3ÔÂ17ÈÕ
 *      Author: jiangxiaohui
 */
#include "ecpDefine.h"
#include "halCryptoConfig.h"

#ifndef CRYPTO_COMMON_API_H_
#define CRYPTO_COMMON_API_H_



#define BIG_DATA_PADDING_FROM_TOP		  			0x00
#define BIG_DATA_PADDING_FROM_BOT		   			0x80	
#define BIG_DATA_PADDING_ALIGN_TWO_BYTE	  			0x02
#define BIG_DATA_PADDING_ALIGN_FOUR_BYTE			0x04
#define BIG_DATA_PADDING_ALIGN_EIGHT_BYTE			0x08
#define BIG_DATA_PADDING_ALIGN_SIXTH_BYTE			0x10	

#define BIG_DATA_PADDING_FROM_MASK		   			0x80   
#define BIG_DATA_PADDING_ALIGN_MASK					0x7F


/**
DES sign update
1. loadkey
2. keymask
3. getkey
4. updateBoot
5. updateVM

DES sign dofinal
1. loadkey
2. keymask
3. getkey
4. dofinalBoot
5. dofianlVM

DES sign verify
1. loadkey
2. keymask
3. getkey
4. dofianlVM

RSA sign dofinal
1. calculate
2. keymask(2)
3. SC_PKIGetAllKey
4. preSign
5. dofinalBoot
6. dofianlVM

RSA sign verify
1. compare
2. calculate
3. keymask(2)
4. SC_PKIGetAllKey
5. dofinalBoot
6. dofianlVM
**/


extern void BigEndian2LittleEndian(PX8 InOutBuf, u16 inputLen);
extern void dataReversal(PM8 InOutBuf, u16 inputLen);
extern u8 checkSrcIsAllZero(PX8 src, u8 count);
extern void ecpAXorB(PX8 pointA, PX8 pointB, PX8 pointC, u16 count);
extern void ecpAXorConst(PX8 pointA, u8 secValue, u16 lenght);
extern u8 reflectChar(u8 inNum);
extern u16 reflectShort(u16 inNum);
extern u32 reflectLong(u32 inNum);
extern u16 Get_crc16(PM8 pu1CRC, u32 u1CRCLength);
extern u16 Get_crc32(PX32 pu1CRC, u32 u1CRCLength);
//extern u8 ecpIsRngInProgress(void);
//extern u8 ecpRngSuspend(void);
//extern void ecpRngRestart(u8 RNGprocess);
//extern u8 ecpIsDesInProgress(void);
//extern u8 ecpDesSuspend(void);
//extern void ecpDesRestart(u8 DESprocess);
//extern u8 ecpIsAesInProgress(void);
//extern u8 ecpAesSuspend(void);
//extern void ecpAesRestart(u8 AESprocess);
extern u8 ecpIsRsaInProgress(void);
extern u8 ecpRsaSuspend(void);
extern void ecpRsaRestart(u8 RSAprocess);
extern void clearCryptoSpace(void);
	#define ecpIsRngInProgress(void)		0
	#define ecpRngSuspend(void)				0
	#define ecpRngRestart(RNGprocess)
	#define ecpIsDesInProgress(void)		0
	#define ecpDesSuspend(void)				0
	#define ecpDesRestart( DESprocess)
	#define ecpIsAesInProgress(void)		0
	#define ecpAesSuspend(void)				0
	#define ecpAesRestart( AESprocess)
//	#define ecpIsRsaInProgress(void)		0
//	#define ecpRsaSuspend(void)				0
//	#define ecpRsaRestart( RSAprocess)
extern u16 BigData_Mul(PX8 result,PM8 d1_Addr,u16 d1_len, PM8 d2_Addr, u16 d2_len ,u8 type);
extern u16 BigData_Add(PX8 result,PM8 d1_Addr,u16 d1_len, PM8 d2_Addr, u16 d2_len ,u8 type);
extern u16 BigData_Sub(PX8 result,PM8 d1_Addr,u16 d1_len, PM8 d2_Addr, u16 d2_len ,u8 type);
extern void random_delay(void);
extern void ecpAMulBModC(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint);
extern u8 ecpAXorGetChecksum(PX8 pointA, u8 length);
#if WITH_RSA

#define RSA_CRT_ME_KEY_GEN					0xA5
#define RSA_CRT_ME_KEY_CAL					0x5A
	
#define RSA_CRT_CALE_OK						0xA5
#define RSA_CRT_CALE_NOK					0x5A
	
#if	RSA_CRT_MODULAR_SAVE
	#define	IS_RSA_CRT_MODULAR_SAVE			TRUE
	#define E_INDEX							1
#else 
	#define	IS_RSA_CRT_MODULAR_SAVE			FALSE
	#define E_INDEX							0
#endif /*RSA_CRT_MODULAR_SAVE*/

extern u16 calcModuleN(PM8 keyBuff, u16 wPrimeLength, u8 mode);
extern void SetKeyPinv(PM8 keyBuff, u16 realKeyLen);
	
#if	(RSA_CRT_MODULAR_SAVE || RSA_CRT_E_SAVE)
	extern void RSA_storeKey_CRTNE(PM8 pbParmInvQ, u16 primeLen, u8 mode);
#else
	#define RSA_storeKey_CRTNE(pbParmInvQ, primeLen, mode)
#endif 

#if RSA_CRT_E_SAVE
	extern void RSA_CalE(PM8 keyBuff, u16 primeLen);
	extern void RSA_storeKey_CRTE(PM8 pbExp, u16 pbExpLen, u8 mode);
	extern u8 RSA_CalE_Hal(PM8 pBuff, PM8 qBuff, PM8 DpBuff, PM8 DqBuff, PX8 EBuff, u16 primeLen);
#else 
	#define RSA_CalE(keyBuff, realKeyLen)
	#define RSA_storeKey_CRTE(pbExp, pbExpLen, mode)
	#define RSA_CalE_Hal(pBuff, qBuff, DpBuff, DqBuff, EBuff, primeLen);
#endif /*RSA_CRT_E_SAVE*/

#if	RSA_CRT_MODULAR_SAVE
	extern void RSA_storeKey_CRTN(PM8 pbModulusN, u16 wModulusNLength, u8 mode);
#else
	#define RSA_storeKey_CRTN(pbModulusN, wModulusNLength, mode)
#endif 

#endif /* WITH_RSA */
#endif /* CRYPTO_COMMON_API_H_ */
