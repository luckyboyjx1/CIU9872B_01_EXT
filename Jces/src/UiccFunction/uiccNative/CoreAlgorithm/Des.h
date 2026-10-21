#ifndef    _DES_H_
#define    _DES_H_

#include "typedef.h"

///----------------------------------
//		DES  definition
//-----------------------------------

#define 	DECRYPT     	        0xFE
#define 	ENCRYPT     			0xF1
#define 	CBC         			0x8F
#define 	ECB         			0x7F
#define		ECB_Encrypt 			0x71
#define		ECB_Decrypt 			0x7E
#define		CBC_Encrypt 			0x81
#define		CBC_Decrypt 			0x8E
#define		CBC_Init_Encrypt		0x91
#define		CBC_Init_Decrypt		0x9E
#define   	CRC16_POLYNOMIAL 		0x1021 //   CRC_16校验方式的多项式.

#define		SigDESLGTH					0x08
#define		TriDES2KEYLGTH				0x10
#define		TriDES3KEYLGTH				0x18

#ifdef HARDWARE_DES
	#define DESCalc(pbSrcStr,keyContext,pbTgtStr, bMod,lgth, keyLength) HardWareDESCalc(pbSrcStr,keyContext,pbTgtStr, bMod,lgth, keyLength)
#else
	#define DESCalc(pbSrcStr,keyContext,pbTgtStr, bMod,lgth, keyLength) SoftDESCalc(pbSrcStr,keyContext,pbTgtStr, bMod,lgth, keyLength)
#endif

extern  void SoftDESCalc(BYTE * pbSrcStr,BYTE keyContext[16],BYTE * pbTgtStr, BYTE bMod, WORD lgth, BYTE keyLength);
extern	void SingleDES(BYTE * pbSrcStr, BYTE keyContext[8],BYTE * pbTgtStr, BYTE bMod, WORD lgth);
extern	void TripleDES(BYTE * pbSrcStr,BYTE keyContext[24],BYTE * pbTgtStr, BYTE bMod, WORD lgth, BYTE keyLength);	

/**************************************funtion define***********************************************************************/

#endif
