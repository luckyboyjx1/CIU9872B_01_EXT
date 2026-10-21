#ifndef    _ARITHMATIC_H_
#define    _ARITHMATIC_H_

#include "typedef.h"
#include "Crypto_Constant.h"
///----------------------------------
//		DES  definition
//-----------------------------------

//#define 	DECRYPT     	        0xFE
//#define 	ENCRYPT     			0xF1
//#define 	CBC         			0x8F
//#define 	ECB         			0x7F
//#define		ECB_Encrypt 			0x71
//#define		ECB_Decrypt 			0x7E
//#define		CBC_Encrypt 			0x81
//#define		CBC_Decrypt 			0x8E
//#define		CBC_Init_Encrypt		0x91
//#define		CBC_Init_Decrypt		0x9E
//#define   	CRC16_POLYNOMIAL 		0x1021 //   CRC_16校验方式的多项式.
//
////AES算法申明
//#define AES_ECB_DECRYPT	          0x01      //上半字节表示ECB/CBC，后半字节表示加密/解密
//#define AES_ECB_ENCRYPT           0x02
//#define AES_CBC_DECRYPT			  0x11
//#define AES_CBC_ENCRYPT           0x12
//
//
//#define		SigDES					0x00
//#define		TriDES					0x02


#define AES_ECB_DECRYPT	          ECB_Decrypt      
#define AES_ECB_ENCRYPT           ECB_Encrypt
#define AES_CBC_DECRYPT			  CBC_Decrypt
#define AES_CBC_ENCRYPT           CBC_Encrypt
/**************************************funtion define***********************************************************************/
extern void SingleDES(BYTE * pbSrcStr, BYTE keyContext[8],BYTE * pbTgtStr, BYTE bMod, WORD lgth);

extern void TripleDES(BYTE * pbSrcStr,BYTE keyContext[16],BYTE * pbTgtStr, BYTE bMod, WORD lgth, BYTE keyLength);

extern BYTE AESCal(BYTE * inputData, BYTE *outputData, WORD len, BYTE * keyBuf, BYTE keyLen, BYTE *ivData, BYTE mode );

#endif
