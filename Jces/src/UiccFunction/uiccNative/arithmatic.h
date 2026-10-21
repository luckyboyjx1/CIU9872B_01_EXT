#ifndef    _ARITHMATIC_H_
#define    _ARITHMATIC_H_
#include "typedef.h"
///----------------------------------
//		DES  definition
//-----------------------------------



#define 	DECRYPT     	0xFE
#define 	ENCRYPT     	0xF1
#define 	CBC         	0x8F
#define 	ECB         	0x7F
//#define		ECB_Encrypt 	0x71
//#define		ECB_Decrypt 	0x7E
//#define		CBC_Encrypt 	0x81
//#define		CBC_Decrypt 	0x8E
//#define		CBC_Init_Encrypt		0x91
//#define		CBC_Init_Decrypt		0x9E
#define   	CRC16_POLYNOMIAL 		0x1021 //   CRC_16校验方式的多项式.

#define		SigDES	0x00
#define		TriDES	0x02


#define    TripleDES2Key    0x00
#define    TripleDES3Key    0x01


//#define SingleDES(pbSrcStr,keyContext,pbTgtStr,bMod,lgth)   DESCalc(pbSrcStr,keyContext,pbTgtStr,bMod,lgth,SigDES);
//#define TripleDES(pbSrcStr,keyContext,pbTgtStr,bMod,lgth)	DESCalc(pbSrcStr,keyContext,pbTgtStr,bMod,lgth,TriDES);

extern void DESCalc(BYTE * pbSrcStr,BYTE keyContext[16],BYTE * pbTgtStr, BYTE bMod, WORD lgth, BYTE DESMode);

extern void SingleDES(BYTE * pbSrcStr, BYTE keyContext[8],BYTE * pbTgtStr, BYTE bMod, WORD lgth);

extern void TripleDES(BYTE * pbSrcStr,BYTE keyContext[16],BYTE * pbTgtStr, BYTE bMod, WORD lgth, BYTE keyLength);

extern WORD  CRC16Calc(BYTE *pucData,BYTE nBytes);

#endif
