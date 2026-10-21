/************************************************************
  Copyright (C), 1988-1999, Eastcompeace. Co., Ltd.
  FileName: Crypto_SoftDES.h.
  implement DES compution with software
  Author: Lizumeng.
  Version : 1.0.
  Date: 2010/04/16.
  Description: This header file contains functions used to compute
               DES.
  History:
      <author>    <time>    <version>    <desc>
      Lizumeng   10/04/16     1.0     build this moudle
*************************************************************/
#ifndef SOFTWAREDES_H_
#define SOFTWAREDES_H_

#include "Sym_API.h"

#if WITH_SOFT_DES

//extern u8 g_KOREANSEED_Key[DES_BLOCKSIZE * 3];		//store the DES key buffer, the max size is triple DES KEYSIZE
/*****************************************************************************
* constants
*****************************************************************************/
 //encryption with DES
#define EN_DES 	MODE_ENCRYPT
//decryption with DES
#define DE_DES 	MODE_DECRYPT

/*****************************************************************************
* API
*****************************************************************************/
extern void SoftDes_loadDESKey(u8 keyByteLen, PX32 pExternKey);
extern void SoftDes_DoDes_Crypt(PX8 dataIn, PX8 dataOut, u8 en_deCrypt, PX32 pExternKey, u8 Key_type_len);
extern void SoftDes_HardwareDES(u8 dataOut[8], u8 dataIn[8], u8 pExternKey[8], u8 algType);
extern void SoftDes_Hardware3DES2Key(u8 dataOut[8], u8 dataIn[8], u8 pExternKey[16], u8 algType);
extern void SoftDes_Hardware3DES3Key(u8 dataOut[8], u8 dataIn[8], u8 pExternKey[24], u8 algType);
#define loadDESKey(keyByteLen, pExternKey)									SoftDes_loadDESKey(keyByteLen, pExternKey)
#define DoDes_Crypt(dataIn, dataOut, en_deCrypt, pExternKey, Key_type_len)	SoftDes_DoDes_Crypt(dataIn, dataOut, en_deCrypt, pExternKey, Key_type_len)
#define HardwareDES(dataOut, dataIn, pExternKey, algType)					SoftDes_HardwareDES(dataOut, dataIn, pExternKey, algType)
#define Hardware3DES2Key(dataOut, dataIn, pExternKey, algType)				SoftDes_Hardware3DES2Key(dataOut, dataIn, pExternKey, algType)
#define Hardware3DES3Key(dataOut, dataIn, pExternKey, algType)				SoftDes_Hardware3DES3Key(dataOut, dataIn, pExternKey, algType)
#else
extern void loadDESKey(u8 keyByteLen, PX32 pExternKey);
extern void DoDes_Crypt(PX8 dataIn, PX8 dataOut, u8 en_deCrypt, PX32 pExternKey, u8 Key_type_len);
//extern void DoDes_Crypt(PX32 dataIn, u8 dataInLen, PX32 keyBuffer, u8 keyByteLen, PX32 tICV,u8 option);
void HardwareDES(u8 dataOut[8], u8 dataIn[8], u8 pExternKey[8], u8 algType);
void Hardware3DES2Key(u8 dataOut[8], u8 dataIn[8], u8 pExternKey[16], u8 algType);
void Hardware3DES3Key(u8 dataOut[8], u8 dataIn[8], u8 pExternKey[24], u8 algType);
#endif  // WITH_SOFT_DES
#endif /* SOFTWAREDES_H_ */
