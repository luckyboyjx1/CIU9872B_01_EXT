 /************************************************************************************************************
 * Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
 * All rights reserved.
 *
 * File name:   arithmatic.c
 *
 * Description: 该文件主要实现与应用相关的算法接口，如des  3des等
 *
 * auther：     xxxx    xxxx.xx.xx
 *
 * history：    V1.0
 *
 ************************************************************************************************************/
#include "typedef.h"
#include "arithmatic.h"
 //#include "hard_api.h"
 //#include "VMAccessEeprom.h"
#include "Crypto_API.h"
#include "Crypto_Constant.h"
#include "MemOperation.h"

/*-------------------------------------------------------------------------------------------------------//

  Function name:	SingleDES

 Input parameter:
 			        pbSrcStr:源数据(若有初始值，则可用参数pbTgtStr传递，用参数bMod来区分)

				keyContext  :主密钥

				pbTgtStr:结果数据(若有初始值，则可用此参数传递，用参数bMod来区分)

				bMod: 	ECB_Encrypt     : 	ECB	加密
						ECB_Decrypt    :	ECB	解密
						CBC_Encrypt     :    无初始值CCB加密
						CBC_Decrypt  	:	无初始值CCB解密
						CBC_Init_Encrypt	:有初始值时，初始值已以参数方式传递的CBC加密
						CBC_Init_Decrypt	:有初始值时，初始值已以参数方式传递	的CBC解密

				lgth:源数据长度

  Output parameter:     void

  Description:          单DES算法。
  				注意:若是CBC_Encrypt模式而且有初始值，则可在函数调用之前用初始值与源数据的第一个BLOCK
  				作一次异或后再传递给pbSrcStr;若是CBC_Decrypt模式，则可在做完3DES算法后用初始值与pbTgtStr的第
  				一个BLOCK作一次异或，得到的数据才是结果数据

  Example:		data: 			7897875668685365
				des-key  : 		7890847689465788
				des-ECB加密: 	6C39BBF88D955E3D
				des-ECB解密:	3279102C0E5C7BE1

				data : 			789787566868536528276634596754467865475640574334
				des-key  : 		7890847689465788
				des-CBC加密: 	6C39BBF88D955E3D8280767E287A159822FEEE23CF4E0AC7
				des-CBC解密:	3279102C0E5C7BE16C977287D5649322281F75FD79869ACD


  Status:		OK

 -------------------------------------------------------------------------------------------------------------------*/
void SingleDES(BYTE * pbSrcStr, BYTE keyContext[8],BYTE * pbTgtStr, BYTE bMod, WORD lgth)
{
	SingleDESForUICC(pbSrcStr, keyContext, pbTgtStr, bMod, lgth);
}

/*-------------------------------------------------------------------------------------------------------//

  Function name:	TripleDES

  Input parameter:
 			        pbSrcStr:源数据(若有初始值，则可用参数pbTgtStr传递，用参数bMod来区分)

				keyContext  :主密钥

				pbTgtStr:结果数据(若有初始值，则可用此参数传递，用参数bMod来区分)

				bMod: 	ECB_Encrypt     : 	ECB	加密
						ECB_Decrypt    :	ECB	解密
						CBC_Encrypt     :    无初始值CCB加密
						CBC_Decrypt  	:	无初始值CCB解密
						CBC_Init_Encrypt	:有初始值时，初始值已以参数方式传递的CBC加密
						CBC_Init_Decrypt	:有初始值时，初始值已以参数方式传递	的CBC解密

				lgth:源数据长度

  Output parameter:     void

  Description:          3DES算法。
  				注意:若是CBC_Encrypt模式而且有初始值，则可在函数调用之前用初始值与源数据的第一个BLOCK
  				作一次异或后再传递给pbSrcStr;若是CBC_Decrypt模式，则可在做完3DES算法后用初始值与pbTgtStr的第
  				一个BLOCK作一次异或，得到的数据才是结果数据

  Example:		data: 			789787566868536528276634596754467865475640574334
				3des-key  : 	26777978045645893542467496856409
				3DES-ECB加密: 	F8AFFBC2AE4EB17734B97F68D2CB9A2C1163959669DC44BA
				3DES-ECB解密: 	81DDE3AAE0ADDD59BA276AFBACB0AFD79F2CAEA219653975
				3DES-CBC加密：	F8AFFBC2AE4EB177CB5B0E3EAF841CDF532F5745172E0787
				3DES-CBC解密：	81DDE3AAE0ADDD59C2B0EDADC4D8FCB2B70BC89640026D33

  Status:		OK

 -------------------------------------------------------------------------------------------------------------------*/
void TripleDES(BYTE * pbSrcStr,BYTE keyContext[16],BYTE * pbTgtStr, BYTE bMod, WORD lgth, BYTE keyLength)
{
	TripleDESForUICC(pbSrcStr, keyContext, pbTgtStr, bMod, lgth, keyLength);
}

/*
*@description: 采用AES进行运算
*
*@function   : AESCal
*
*@param       :BYTE *inputData
*@param        BYTE * outputData
*@param        WORD  len
*@param       :BYTE * keyBuf
*@param       :BYTE keyLen
*@param       :BYTE *ivData
*@param       :BYTE mode:AES_ECB_DECRYPT/AES_ECB_ENCRYPT/AES_CBC_DECRYPT/AES_CBC_ENCRYPT
*
8@return       :TRUE/FALSE

*/
BYTE AESCal(BYTE * inputData, BYTE *outputData, WORD len, BYTE * keyBuf, BYTE keyLen, BYTE *ivData, BYTE mode )
{
	BYTE xdata ivBuffer[16];

	memCopyToRam(ivBuffer, ivData, 16);

	SymCipherDofinal(MEMtPM8(inputData), len, MEMtPM8(outputData), MEMtPM8(keyBuf), U8_M_U8(keyLen,8), MEMtPM8(ivBuffer), NULL, ALG_AES_BLOCK_128_CBC_NOPAD, mode, 0x00, ALG_AES);
	return TRUE;
}

