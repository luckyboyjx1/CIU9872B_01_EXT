/*
 * Crypto_Soft_SM4.h
 *
 *  Created on: 2014Äê4ÔÂ10ÈÕ
 *      Author: jiangxiaohui
 */

#ifndef CRYPTO_SOFT_SM4_H_
#define CRYPTO_SOFT_SM4_H_


#include "Sym_API.h"

#if WITH_SOFT_SM4


/***********************************************************************************
 * SM4 API
 ***********************************************************************************/
extern void SoftSM4_DoSM4_Crypt(PX8 dataIn, PX8 dataOut, u8 en_deCrypt, PX32 pExternKey, u8 KeyByteLen);
extern void SoftSM4_SM4KeyExpand(u16 keyBitLen, PX32 pExternKey, u8 cryptoMode);

#define SM4KeyExpand(keyBitLen, pExternKey, cryptoMode)               		SoftSM4_SM4KeyExpand(keyBitLen, pExternKey, cryptoMode)
#define DoSM4_Crypt(dataIn, dataOut, en_deCrypt, pExternKey, KeyByteLen)    SoftSM4_DoSM4_Crypt(dataIn, dataOut, en_deCrypt, pExternKey, KeyByteLen)
#else
extern void SM4KeyExpand(u16 keyBitLen, PX32 pExternKey, u8 cryptoMode);
//extern void DoSM4_Crypt(PX8 dataIn, PX8 dataOut, u8 en_deCrypt, PX32 pExternKey, u8 KeyByteLen);
void DoSM4_Crypt(PX32 dataIn, u8 dataInLen, PX32 keyBuffer, u8 keyByteLen, PX32 tICV,u8 option);

#endif /* WITH_SOFT_SM4 */

#endif /* CRYPTO_SOFT_SM4_H_ */
