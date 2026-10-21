
/*
 * 	 GBAAuth
 *   
 */
#include <string.h> 
#include "typedef.h"
#include "Hard_api.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include "stkhead.h"
#include "Sha1.h"
#include "interface.h"


#ifdef GBA


/*
description: 对最后一块数据进行HAMC运算，并输出最终的结果.该函数的功能是RFC2014中的Appendix -- Sample Code的
*                                                         MD5Final(digest, &context);          finish up 1st pass
*                                                         MD5Init(&context);                   init context for 2nd pass
*                                                         MD5Update(&context, k_opad, 64);     start with outer pad
*                                                         MD5Update(&context, digest, 16);     then results of 1st hash
*                                                         MD5Final(digest, &context);          finish up 2nd pass
*@fucntion : HMACDoFinal
*
*@param    :BYTE *keyBuf     初始的密钥值，也可以用于存储最终的HMAC的KEY. 传入该参数时需要注意：传入数组的长度不小于64字节，否则会造成溢出
*@param    :BYTE keyLen      初始密钥的长度， The authentication key K can be of any length up to B the block length of the hash function
*@param    :BYTE *src        需要计算的最后数据的数据源
*@param    :BYTE srcLgth     需要计算的最后数据的数据长度
*@param    :BYTE *dest       最后计算出的HMAC值
*
*@return   : HMAC的长度
*
*@history    :V1.0 : created by xuxiaojun@20180411
*/
/*
BYTE HMACSha256DoFinal(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest, SHA_INFO * context)
{
	SHA256Update(context, src, srcLgth, 0, 0);
		
	SHA256Final(context, dest);

	HMACSha256Init(keyBuf, keyLen, 0x5C, context);

	SHA256Update(context, dest, HMAC_DIGEST_LENGTH_SHA256, 0, 0);

	SHA256Final(context, dest);
	
	return HMAC_DIGEST_LENGTH_SHA256;
}
*/


/*
*@description:对K进行初步操作，1：K XOR ipad 或K XOR opad，然后hash该值. 生成HMAC KEY，最终的HMAC KEY的长度为64字节, 规范RFC2014 <2. Definition of HMAC>中的step1和step2
*
*@function   :HMACCaulateK
*
*@See        :RFC2014 <2. Definition of HMAC>
*             The authentication key K can be of any length up to B the block length of the hash function
*             Applications that use keys longer than B bytes will first hash the key using H and then use the resultant L byte string as the actual key to HMAC*
*
*@param      :BYTE *kBuf    初始的密钥值，也可以用于存储最终的HMAC的KEY. 传入该参数时需要注意：传入数组的长度不小于64字节，否则会造成溢出
*@param      :BYTE kLen     初始密钥的长度， The authentication key K can be of any length up to B the block length of the hash function
*@param      :BYTE xorChar  取值为0x36或0x5C 参考RFC2014 <2. Definition of HMAC>
*@param      :BYTE alg      HMAC的基础HASH算法，取值为HMAC_MD5、HMAC_SHA256
*
*@return     :TRUE: 算法ALG支持/FALSE: 算法alg不支持 
*
*@history    :V1.0 : created by xuxiaojun@20180411
*/
/*
BYTE HMACSha256Init(BYTE* kBuf, BYTE kLen, BYTE xorChar, SHA_INFO * context)
{	
	BYTE xdata tempKey[64];
	BYTE idata i;
	
	i = 0;
	
	memset(tempKey, 0x00, sizeof(tempKey));
	
	if(kLen > 64)    // Applications that use keys longer than B bytes will first hash the key using H and then use the resultant L byte string as the actual key to HMAC*
	{
		SHA256DoFinalSingleCall(&tempKey[0], &kBuf[0], kLen,0,0);
	}
	else
	{
		memcpy(tempKey, kBuf, kLen);
	}
	
	for (i = 0; i < 64; i++)							//k 与 ipad(0x36) or opad(0x5C)异或		//for sha1: 0-19
	{
		tempKey[i] ^= xorChar;
	}
	
	SHA256Init(context);
			
	SHA256Update(context, tempKey, sizeof(tempKey), 0, 0) ;
	
	return TRUE;
}
*/

/*
 *@description :对数据进行HAMC运算，并输出最终的结果.
 *
 *@param       :digest :the result  buf,输出长度固定为0x20字节
 *@param       :sourcedata:待处理的数据缓冲区
 *@param       :length    :待处理的数据长度 
 *      
 * Add padding and return the message digest
 */
/* 
void HMACSha256DoFinalSingleCall(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest) 
{	
	SHA_INFO xdata context;
	
	HMACSha256Init(keyBuf, keyLen, 0x36, &context);	
	
	HMACSha256DoFinal(keyBuf,keyLen, src, srcLgth, dest, &context);	
}
*/

/*                                                                                     
 *@Function name :	 GBANAFAuthenticate for GBA security context (NAF Derivation Mode) 
 *					 
 *@Description:       Ks_ext_NAF = KDF(Ks, "gba-me", RAND, IMPI, NAF_Id) 
								= HMAC-SHA-256 (Key , S)
					 	Key = Ks = CK || IK
					 	S = FC||P0||L0||P1||L1||P2||L2||P3||L3
							FC = 0x01
							P0 = 0x6762612D6D65 ("gba-me")
							L0 = 0x06
							P1 = G_Auth_Rand
							L1 = length of G_Auth_Rand
							P2 = IMPI
							L2 = length of IMPI
							P3 = NAF_ID
							L3 = length of NAF_ID
  *
  *@Coder: 			 xuqs            
  *
  *@Status:			   
*/

BYTE code gbame[] = {0x67,0x62,0x61,0x2D,0x6D,0x65};	//gba-me

BYTE GBANAFAuthenticate(void)
{
	
	BYTE idata pos,index,len;	
	UWORD xdata fileAddrbp,fileAddrKs;
	

	
	fileAddrbp = SelectTFFile(DF_ADF, EF_GBABP_USIM);
		
	index = 0x01;
	

	if(fileAddrbp == FALSE)
	{
		return FALSE;
	}
			
	UpdateNAFId(&RecData[1]);	
	
	tempBuf[0] = 0x01;												//FC
	
	memcpy(&tempBuf[1], (BYTE *)gbame, sizeof(gbame));				//P0

	tempBuf[1+sizeof(gbame)] = 0x00;								//L0
	tempBuf[2+sizeof(gbame)] = sizeof(gbame);	
	
	pos = 1+2+sizeof(gbame);										//point to P1

	//fileAddr = SelectTFFile(G_ISIMADFFID, EF_GBABP);			
	
	ReadTFBuf(fileAddrbp, 0x01, &tempBuf[pos], 16);					//读取6FD5中的RAND值

	tempBuf[pos+16] = 0x00;						//L1
	
	tempBuf[1+pos+16] = 16;
	
	pos += (2+16);									//point to P2

	memcpy(&tempBuf[pos],&RecData[3+RecData[1]],RecData[2+RecData[1]]);
	
	len = RecData[2+RecData[1]];
	
	tempBuf[pos+len] = 0x00;									//L2
	tempBuf[1+pos+len] = len;							
	
	pos += (2+len);											//point to P3
	memcpy(&tempBuf[pos], &RecData[2], RecData[1]);					//P3

	tempBuf[pos+RecData[1]] = 0x00;									//L3
	tempBuf[1+pos+RecData[1]] = RecData[1];
	
	pos += (2+RecData[1]);
	
	fileAddrKs = SelectLFFile(DF_UICCLTE_PRIVATE,EF_GBA_KS);
	
	ReadLFBuf(fileAddrKs,index,0x00,&RecData[0x80],0x20);	
		
	//HMAC_SHA256(&RecData[0x80], 0x20, &tempBuf[0], pos, &RecData[2]);//HMAC-SHA-256 (Key , S)	
	HMACSha256DoFinalSingleCall(&RecData[0x80],0x20,&tempBuf[0],pos,&RecData[2]);
	
	RecData[0] = 0xDB;												//"Successful GBA operation									

	RecData[1] = 0x20;												//Length of Ks_ext_NAF
		
	return 2+RecData[1];
}



/**
 * @brief 		更新EF_GBANL
 *
 * @Param		buf	:NAF_ID(LV)
 *
 * @author 		xuqs
 *
 * @history 	created by xuqs, @140401 20:01
 */
BYTE UpdateNAFId(BYTE * buf)
{
	UWORD xdata fileAddr, fileAddr1;
	BYTE idata recNum;
	BYTE idata i, tag;
	
	fileAddr = SelectTFFile(DF_ADF, EF_GBANL_USIM);
		
	fileAddr1 = SelectTFFile(DF_ADF, EF_GBABP_USIM);
	
	if ((fileAddr == FALSE)||(fileAddr1 == FALSE))
	{
		return FALSE;
	}

	tempBuf[0] = 0x80;										//NAF_ID tag
	
	memcpy(&tempBuf[1], &buf[0], buf[0]+1);
	
	tempBuf[buf[0]+2] = 0x81;								//B-TID tag
	
	ReadTFBuf(fileAddr1, 0x11, &tempBuf[buf[0]+3], sizeof(BYTE));
	
	//if ((tempBuf[buf[0]+3] == 0xFF) ||(tempBuf[buf[0]+3] == 0x00))						//如果终端没有更新EF_GBABP，则不对EF_GBANL的B-TID数据域进行更新
	if (tempBuf[buf[0]+3] == 0xFF)	
	{
		tempBuf[buf[0]+3] = 0x00;
	}
	
	ReadTFBuf(fileAddr1, 0x12, &tempBuf[buf[0]+4], tempBuf[buf[0]+3]);
		
	
	for(i = 0x01; i<= GetLFRecNumLgth(fileAddr,FileNum); i++)
	{
		ReadLFBuf(fileAddr, i, 0x00 , &tag, sizeof(BYTE));
		
		if (tag != 0x80)
		{ 			
			
 			WriteLFBuf(fileAddr, i, 0x00, &tempBuf[0], (buf[0]+4+tempBuf[buf[0]+3])); 									
			
			WriteEeprom(fileAddr + F_Pointer_Offset, &i, sizeof(BYTE));	//目前暂时使用RecIndicator来标记最近更新的记录号

		}
	}
	
	if (i > GetLFRecNumLgth(fileAddr,FileNum))
	{
		recNum = GetFileHeadByte(fileAddr, F_Pointer_Offset);			//目前暂时使用RecIndicator来标记最近更新的记录号
		
		if ((recNum+1) > GetLFRecNumLgth(fileAddr,FileNum))				//计算最早更新的记录号
		{
			recNum = 0x01;
		}
		else
		{
			recNum ++;
		}								
 			
 		WriteLFBuf(fileAddr, recNum, 0x00, &tempBuf[0], (buf[0]+4+tempBuf[buf[0]+3])); 
				
		WriteEeprom(fileAddr + F_Pointer_Offset, &recNum, sizeof(BYTE));
	}
	
	return TRUE;
}








#endif
