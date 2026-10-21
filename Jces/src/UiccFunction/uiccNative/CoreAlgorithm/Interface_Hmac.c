/************************************************************************************************************
* Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   HAMC.c
*
* Description: 该文件主要用于存放HMAC相关的函数
*
* @see        :<rfc2104(HMAC Keyed-Hashing for Message Authentication).pdf>
*
* auther：     xxxx    xxxx.xx.xx
*
* history：    V1.0 :create @20180411 ,目前支持HMAC_MD5 HMAC_SHA1 HMAC_SHA256
*
************************************************************************************************************/
#include "Interface_Hmac.h"
//#include "Sha1.h"
#include "Md5.h"
#include  <string.h>



#if HMAC_MD5

#if UICC_DIGEST_MD5 

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
*
*@history    :V1.0 : created by xuxiaojun@20180411
*/
BYTE HMACMD5Init(BYTE* kBuf, BYTE kLen, BYTE xorChar, MD5_CTX * context)
{
	BYTE xdata tempKey[64];
	BYTE idata i;
	
	i = 0;
	
	memset(tempKey, 0x00, sizeof(tempKey));
	
	if(kLen > 64)    // Applications that use keys longer than B bytes will first hash the key using H and then use the resultant L byte string as the actual key to HMAC*
	{
		MD5DoFinalSingleCall(&tempKey[0], &kBuf[0], kLen, 0, 0);
	}
	else
	{
		memcpy(tempKey, kBuf, kLen);
	}
	
	for (i = 0; i < 64; i++)							//k 与 ipad(0x36) or opad(0x5C)异或		//for sha1: 0-19
	{
		tempKey[i] ^= xorChar;
	}
	
	MD5Init(context);
			
	MD5Update(context, tempKey, sizeof(tempKey), 0, 0) ;
	
	return TRUE;
}
/*
*@description: 进行K XOR ipad ，并对扩展后的key进行hash运算. 该函数的功能是RFC2014中的Appendix -- Sample Code的
*                                                            k_ipad计算
*                                                            MD5Init(&context); init context for 1st pass
*                                                            MD5Update(&context, k_ipad, 64); start with inner pad
*
*@fucntion : HMACInit
*
*@param      :BYTE *kBuf    初始的密钥值，也可以用于存储最终的HMAC的KEY. 传入该参数时需要注意：传入数组的长度不小于64字节，否则会造成溢出
*@param      :BYTE kLen     初始密钥的长度， The authentication key K can be of any length up to B the block length of the hash function
*@param      :BYTE alg      HMAC的基础HASH算法，取值为HMAC_MD5、ALG_HMAC_SHA1、HMAC_SHA256
*
*@return   : TRUE: 算法ALG支持/FALSE: 算法alg不支持 
*
*@history    :V1.0 : created by xuxiaojun@20180411
*/
 BYTE HMACMD5Init(BYTE *kBuf, BYTE kLength, MD5_CTX *context)
 {
 	return HMACMD5CaulateK(kBuf, kLength, 0x36, context);
 }

/*
description: 对中间数据进行hmac运算, 该函数的功能是RFC2014中的Appendix -- Sample Code的
*                                    MD5Update(&context, text, text_len);  then text of datagram
*
*@fucntion : HMACUpdate
*
*@param    : BYTE *src     需要计算的中间数据的数据源
*@param    : BYTE srcLgth  需要计算的中间数据的长度
*@param    : BYTE alg      HMAC的基础HASH算法，取值为HMAC_MD5、ALG_HMAC_SHA1、HMAC_SHA256
*
*@return   : void
*
*@history    :V1.0 : created by xuxiaojun@20180411
*/
void HMACMD5Update(BYTE * src, WORD srcLgth, MD5_CTX * context)
{
	MD5Update(context, src, srcLgth, 0, 0) ;
}

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
BYTE HMACMD5DoFinal(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest, MD5_CTX * context)
{
	MD5Update(context, src, srcLgth, 0, 0);
		
	MD5Final(context, dest);

	HMACMD5Init(keyBuf, keyLen, 0x5C, context);

	MD5Update(context, dest, HMAC_DIGEST_LENGTH_MD5, 0, 0);

	MD5Final(context, dest);
	
	return HMAC_DIGEST_LENGTH_MD5;
}

/*
 *@description :对数据进行HAMC运算，并输出最终的结果.
 *
 *@param       :digest :the result  buf,输出长度固定为16字节
 *@param       :sourcedata:待处理的数据缓冲区
 *@param       :length    :待处理的数据长度 
 *      
 * Add padding and return the message digest
 */
void HMACMD5DoFinalSingleCall(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest) 
{	
	MD5_CTX xdata context;
	
	HMACMD5Init(keyBuf, keyLen, 0x36, &context);	
	
	HMACMD5DoFinal(keyBuf,keyLen, src, srcLgth, dest, &context);	
}
#endif

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
BYTE HMACSha1Init(BYTE* kBuf, BYTE kLen, BYTE xorChar, SHA1_CTX * context)
{
	BYTE xdata tempKey[64];
	BYTE idata i;
	
	i = 0;
	
	memset(tempKey, 0x00, sizeof(tempKey));
	
	if(kLen > 64)    // Applications that use keys longer than B bytes will first hash the key using H and then use the resultant L byte string as the actual key to HMAC*
	{
		SHA1DoFinalSingleCall(&tempKey[0], &kBuf[0], kLen, 0, 0);
	}
	else
	{
		memcpy(tempKey, kBuf, kLen);
	}
	
	for (i = 0; i < 64; i++)							//k 与 ipad(0x36) or opad(0x5C)异或		//for sha1: 0-19
	{
		tempKey[i] ^= xorChar;
	}
	
	SHA1Init(context);
			
	SHA1Update(context, tempKey, sizeof(tempKey), 0, 0) ;
	
	return TRUE;
}
/*
*@description: 进行K XOR ipad ，并对扩展后的key进行hash运算. 该函数的功能是RFC2014中的Appendix -- Sample Code的
*                                                            k_ipad计算
*                                                            MD5Init(&context); init context for 1st pass
*                                                            MD5Update(&context, k_ipad, 64); start with inner pad
*
*@fucntion : HMACInit
*
*@param      :BYTE *kBuf    初始的密钥值，也可以用于存储最终的HMAC的KEY. 传入该参数时需要注意：传入数组的长度不小于64字节，否则会造成溢出
*@param      :BYTE kLen     初始密钥的长度， The authentication key K can be of any length up to B the block length of the hash function
*@param      :BYTE alg      HMAC的基础HASH算法，取值为HMAC_MD5、ALG_HMAC_SHA1、HMAC_SHA256
*
*@return   : TRUE: 算法ALG支持/FALSE: 算法alg不支持 
*
*@history    :V1.0 : created by xuxiaojun@20180411
*/
 BYTE HMACSha1Init(BYTE *kBuf, BYTE kLength, SHA1_CTX *context)
 {
 	return HMACSha1CaulateK(kBuf, kLength, 0x36, context);
 }

/*
description: 对中间数据进行hmac运算, 该函数的功能是RFC2014中的Appendix -- Sample Code的
*                                    MD5Update(&context, text, text_len);  then text of datagram
*
*@fucntion : HMACUpdate
*
*@param    : BYTE *src     需要计算的中间数据的数据源
*@param    : BYTE srcLgth  需要计算的中间数据的长度
*@param    : BYTE alg      HMAC的基础HASH算法，取值为HMAC_MD5、ALG_HMAC_SHA1、HMAC_SHA256
*
*@return   : void
*
*@history    :V1.0 : created by xuxiaojun@20180411
*/
void HMACSha1Update(BYTE * src, WORD srcLgth, SHA1_CTX * context)
{
	SHA1Update(context, src, srcLgth, 0, 0);
}

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
BYTE HMACSha1DoFinal(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest, SHA1_CTX * context)
{
	SHA1Update(context, src, srcLgth, 0, 0);
		
	SHA1Final(context, dest);

	HMACSha1Init(keyBuf, keyLen, 0x5C, context);

	SHA1Update(context, dest, HMAC_DIGEST_LENGTH_SHA1, 0, 0);

	SHA1Final(context, dest);
	
	return HMAC_DIGEST_LENGTH_SHA1;
}

/*
 *@description :对数据进行HAMC运算，并输出最终的结果.
 *
 *@param       :digest :the result  buf,输出长度固定为20字节
 *@param       :sourcedata:待处理的数据缓冲区
 *@param       :length    :待处理的数据长度 
 *      
 * Add padding and return the message digest
 */
void HMACSha1DoFinalSingleCall(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest) 
{	
	SHA1_CTX xdata context;
	
	HMACSha1Init(keyBuf, keyLen, 0x36, &context);	
	
	HMACSha1DoFinal(keyBuf,keyLen, src, srcLgth, dest, &context);		
}
#endif



#if HMAC_SHA256
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
/*
*@description: 进行K XOR ipad ，并对扩展后的key进行hash运算. 该函数的功能是RFC2014中的Appendix -- Sample Code的
*                                                            k_ipad计算
*                                                            MD5Init(&context); init context for 1st pass
*                                                            MD5Update(&context, k_ipad, 64); start with inner pad
*
*@fucntion : HMACInit
*
*@param      :BYTE *kBuf    初始的密钥值，也可以用于存储最终的HMAC的KEY. 传入该参数时需要注意：传入数组的长度不小于64字节，否则会造成溢出
*@param      :BYTE kLen     初始密钥的长度， The authentication key K can be of any length up to B the block length of the hash function
*@param      :BYTE alg      HMAC的基础HASH算法，取值为HMAC_MD5、ALG_HMAC_SHA1、HMAC_SHA256
*
*@return   : TRUE: 算法ALG支持/FALSE: 算法alg不支持 
*
*@history    :V1.0 : created by xuxiaojun@20180411
*/
// BYTE HMACSha256Init(BYTE *kBuf, BYTE kLength, SHA_INFO *context)
// {
// 	return HMACSha256CaulateK(kBuf, kLength, 0x36, context);
// }

/*
description: 对中间数据进行hmac运算, 该函数的功能是RFC2014中的Appendix -- Sample Code的
*                                    MD5Update(&context, text, text_len);  then text of datagram
*
*@fucntion : HMACUpdate
*
*@param    : BYTE *src     需要计算的中间数据的数据源
*@param    : BYTE srcLgth  需要计算的中间数据的长度
*@param    : BYTE alg      HMAC的基础HASH算法，取值为HMAC_MD5、ALG_HMAC_SHA1、HMAC_SHA256
*
*@return   : void
*
*@history    :V1.0 : created by xuxiaojun@20180411
*/
void HMACSha256Update(BYTE * src, WORD srcLgth, SHA_INFO * context)
{
	SHA256Update(context, src, srcLgth, 0, 0);
}

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
BYTE HMACSha256DoFinal(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest, SHA_INFO * context)
{
	SHA256Update(context, src, srcLgth, 0, 0);
		
	SHA256Final(context, dest);

	HMACSha256Init(keyBuf, keyLen, 0x5C, context);

	SHA256Update(context, dest, HMAC_DIGEST_LENGTH_SHA256, 0, 0);

	SHA256Final(context, dest);
	
	return HMAC_DIGEST_LENGTH_SHA256;
}

/*
 *@description :对数据进行HAMC运算，并输出最终的结果.
 *
 *@param       :digest :the result  buf,输出长度固定为0x20字节
 *@param       :sourcedata:待处理的数据缓冲区
 *@param       :length    :待处理的数据长度 
 *      
 * Add padding and return the message digest
 */
#if JAVA_CARD
void HMACSha256DoFinalSingleCall(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest) 
{	
	SHA_INFO xdata context;
	
	HMACSha256Init(keyBuf, keyLen, 0x36, &context);	
	
	HMACSha256DoFinal(keyBuf,keyLen, src, srcLgth, dest, &context);	
}
#endif


/*

*/
BYTE HMACSha256DoFinalInEEprom(BYTE * keyBuf,BYTE keyLen, UWORD fileAddr, WORD srcLgth, BYTE * dest, SHA_INFO * context)
{
	SHA256Update(context, NULL, srcLgth, 0x01, fileAddr);
		
	SHA256Final(context, dest);

	HMACSha256Init(keyBuf, keyLen, 0x5C, context);

	//SHA256Update(context, NULL, HMAC_DIGEST_LENGTH_SHA256, 0x01, fileAddr);
	SHA256Update(context, dest, HMAC_DIGEST_LENGTH_SHA256, 0x00, 0x00);

	SHA256Final(context, dest);
	
	return HMAC_DIGEST_LENGTH_SHA256;

}

/*

*/
void HMACSha256DoFinalSingleCallInEEprom(BYTE * keyBuf,BYTE keyLen, UWORD fileAddr, WORD srcLgth, BYTE * dest) 
{	
	SHA_INFO xdata context;
	
	HMACSha256Init(keyBuf, keyLen, 0x36, &context);	

	HMACSha256DoFinalInEEprom(keyBuf,keyLen, fileAddr, srcLgth, dest, &context);

	/*
	SHA256Update(context, src, srcLgth, 0, 0);
		
	SHA256Final(context, dest);

	HMACSha256Init(keyBuf, keyLen, 0x5C, context);

	SHA256Update(context, dest, HMAC_DIGEST_LENGTH_SHA256, 0, 0);

	SHA256Final(context, dest);
	
	return HMAC_DIGEST_LENGTH_SHA256;
	*/
	
	//HMACSha256DoFinal(keyBuf,keyLen, src, srcLgth, dest, &context);		
}

#endif


