/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   SCP03_Arithmetic.c
*
* Description: 该文件主要定义了SCP03所涉及的算法
*
* auther：     dingzy    20170616
*
* history：    V1.0
*
************************************************************************************************************/
#include "typedef.h"
#include "hard_api.h"
#include  "AES.h"



BYTE xdata G_CacheBuffer[34];// IV :L+ 16 + L + cacheData

BYTE xdata G_CacheLast;

BYTE xdata G_AesCipherMode;

BYTE xdata G_AesCipherAlg;

BYTE xdata G_AesCipherPadding;

BYTE xdata G_CalcCMacState;

BYTE xdata G_BlockSize;

BYTE xdata G_MainKey[CMAC_MAINKEY_SIZE_MAX];

BYTE xdata G_SubKey1[CMAC_BLOCKSIZE_MAX];

BYTE xdata G_SubKey2[CMAC_BLOCKSIZE_MAX];

BYTE xdata G_ChainingCi[CMAC_BLOCKSIZE_MAX];

#define AES_CIPHER_LEN 16

#define AES_CACHE_L_OFF (AES_CIPHER_LEN+1)


#if 1	//aes加解算法在 TlsAlgorithm.c 中实现

void XorCalc(BYTE * src, BYTE * dest, BYTE lgth)
{
	BYTE i;
	
	for (i=0;i<lgth;i++)
	{
		dest[i] = src[i] ^ dest[i];
	}
}

/*
*@description :对上下行报文进行加解密，传进来的数据已是block的整数倍，该函数内部不需要考虑加密算法为null的情况，由函数调用者在外部判断，只有当加密算法不为null时才调用该函数
*             :对于一条完整的记录来说加密前的明文和解密后的密文，其数据格式是下面结构体中去除IV之后的。
*              struct {
*                   opaque IV[SecurityParameters.record_iv_length];
*                   block-ciphered struct {
*                       opaque content[TLSCompressed.length];
*                       opaque MAC[SecurityParameters.mac_length];
*                       uint8 padding[GenericBlockCipher.padding_length];
*                       uint8 padding_length;
*                   };
*               } GenericBlockCipher;
*@function    :TLSEncryDecryMsg
*
*@param       :BYTE *keyBuf  		:key值
*@param       :BYTE keyLen   		:key 长度
*@param       :BYTE * ivBuf 		:iv值
*@param       :BYTE ivLen			:iv长度
*@param       :BYTE chipherMode 	:ALG_ENCRYPT/ALG_DECRYPT
*@param       :BYTE alg：			:ALG_AES_CBC/ALG_AES_ECB
*@param       :BYTE padding：		:如果加解密数据不是一次加密长度的倍数时, 补齐长度的数据值
*
*@return      :void
*
*@history     :created xxx
*@history     :modify by jiangqingbin@20211130  修改keyBuf 和 ivBuf 不再是LV格式，增加传入参数表示两者长度	-jiangqingbin-20211130
*             
*/
void aesCipherInit(BYTE * keyBuf, BYTE keyLen, BYTE * ivBuf, BYTE ivLen, BYTE chipherMode, BYTE alg, BYTE padding)
{	
	AESKeyInit(keyBuf, keyLen);

	G_AesCipherMode = chipherMode;
	
	G_AesCipherAlg = alg;
	
	G_AesCipherPadding = padding;
	
	if (ivBuf != NULL)
	{
		G_CacheBuffer[0] = ivLen;
	
		memcpy(&G_CacheBuffer[1], &ivBuf[0], ivLen);
	}
	else
	{
		G_CacheBuffer[0] = 0x10;
		
		memset(&G_CacheBuffer[1], 0, 16);
	}
	
	G_CacheLast = FALSE;
	
	G_CacheBuffer[AES_CIPHER_LEN + 1] = 0;
}



WORD aesCipherUpdate(BYTE * src, WORD srcLgth, BYTE * dest)
{	
	BYTE idata padNum;
	
	BYTE idata srcOff = 0;
	
	BYTE idata destOff = 0;
	
	padNum = AES_CIPHER_LEN - G_CacheBuffer[AES_CACHE_L_OFF];
	
	while (srcLgth > 0)
	{
		memcpy(&G_CacheBuffer[AES_CACHE_L_OFF + 1 + G_CacheBuffer[AES_CACHE_L_OFF]], &src[srcOff], padNum);
		
		if (padNum == 0)
		{
			srcLgth = 0;
		}	
		else
		{
			srcLgth -= padNum;
		}
		
		srcOff += padNum;
		
		if (G_AesCipherMode == ALG_ENCRYPT)																		//encrypt,   xor then encry
		{
			if (G_AesCipherAlg == ALG_AES_CBC)
			{
				XorCalc	(&G_CacheBuffer[1], &G_CacheBuffer[AES_CACHE_L_OFF+1], AES_CIPHER_LEN);
			}
			
			CoreCalcAES(&G_CacheBuffer[AES_CACHE_L_OFF+1],&dest[destOff], ALG_ENCRYPT);
			
			memcpy(&G_CacheBuffer[1], &dest[destOff], AES_CIPHER_LEN);
		}
		else			//(G_AesCipherMode == ALG_DECRYPT)													//decrypt,	 decry then xor
		{			
			CoreCalcAES(&G_CacheBuffer[AES_CACHE_L_OFF+1],&dest[destOff], ALG_DECRYPT);
	
			if (G_AesCipherAlg == ALG_AES_CBC)
			{
				XorCalc	(&G_CacheBuffer[1], &dest[destOff], AES_CIPHER_LEN);		

				memcpy(&G_CacheBuffer[1], &G_CacheBuffer[AES_CACHE_L_OFF+1], AES_CIPHER_LEN);
				
			}
		}
		
		destOff += AES_CIPHER_LEN;
		
		G_CacheBuffer[AES_CACHE_L_OFF] = 0;
		
		if (srcLgth >= AES_CIPHER_LEN)
		{
			padNum = AES_CIPHER_LEN;
		}
		else if (srcLgth > 0)
		{
			if (G_CacheLast == TRUE)
			{
				memcpy(&G_CacheBuffer[AES_CACHE_L_OFF+1], &src[srcOff], srcLgth);
				
				memset(&G_CacheBuffer[AES_CACHE_L_OFF + 1 +  srcLgth], G_AesCipherPadding, AES_CIPHER_LEN -  srcLgth);
				
				G_CacheBuffer[AES_CACHE_L_OFF] = AES_CIPHER_LEN;		
				
				padNum = 0;
			}
			else
			{
				G_CacheBuffer[AES_CACHE_L_OFF] = srcLgth;
			
				memcpy(&G_CacheBuffer[AES_CACHE_L_OFF+1], &src[srcOff], srcLgth);
				
				break;
			}
		}
	}
	
	return destOff;
}

WORD aesCipherDoFinal(BYTE * src, WORD srcLgth, BYTE * dest)
{
	BYTE idata destOff;
	
	G_CacheLast = TRUE;

	destOff = aesCipherUpdate(&src[0], srcLgth, &dest[0]);
	
	return destOff;
	
}

#endif

#if 0	//以下是关于cmac的计算，暂时用不到

/*
*@funtion    :moveOneBitLeft
*
*@description:将 src 的bit序列，左移 1 bit
*
*@param      :BYTE *src    -- 源数据
*             BYTE srcLen  -- 源数据长度
*             BYTE *result -- 移位结果
*
*@return     :void
*
*@author     :dingzy
*
*@history    :V1.0 :create by dingzy@20170616
*/
void moveOneBitLeft(BYTE *src, BYTE srcLen, BYTE *result)
{
	BYTE idata i;
	
	for(i=0; i<(srcLen-1); i++)
	{
		result[i] = (src[i]<<1) | (src[i+1]>>7);
	}
	
	result[srcLen-1] = src[srcLen-1]<<1;
	
	return;
}

/*
*@funtion    :xorBlock
*
*@description:块 异或 运算，每块16字节, block2 = block1 ^ block2
*
*@param      :block1 -- 数据块1
*             block2 -- 计算结果，保存在bolck2
*
*@return     :void
*
*@author     :dingzy
*
*@history    :V1.0 :create by dingzy@20170616
*/
void xorBlock(BYTE *block1, BYTE *block2, BYTE blockSize)
{
	BYTE idata i;
	
	for(i=0; i<blockSize; i++)
	{
		block2[i] ^= block1[i];
	}
	
	return;
}

/*
*@funtion    :subKey
*
*@description:计算 subKey
*            :标准SP_800-38B.pdf  6.1 章节
*
*@param      :BYTE *K -- 原始key, L V 结构
*             BYTE blockSize -- 块大小， 128bits | 64bits （即16bytes | 8bytes）
*
*@return     :void
*
*@author     :dingzy
*
*@history    :V1.0 :create by dingzy@20170616
*/
void subKey(BYTE *K, BYTE blockSize)
{	
	BYTE *L;
	BYTE idata Rb;
	
	//1-将K存入全局变量
	memcpy(G_MainKey, K, K[0]+1);
	
	//2-调用 AES_ECB 加密函数，使用 K 对 initBlock(L) 进行加密，得到 L计算 L，L的长度和 blockSize有关	
	L = G_SubKey2;//L 复用 SUBKEY_K2 空间
	
	memset(L, 0x00, blockSize);
	
	aesCipherInit(K, NULL, ALG_ENCRYPT, ALG_AES_ECB, 0x00);
	
	aesCipherDoFinal(L, blockSize, L);
	
	//3-根据 blockSize，选择Rb
	if(blockSize == CMAC_BLOCKSIZE_128BITS)
	{
		Rb = RB_128BITS;
	}
	else if(blockSize == CMAC_BLOCKSIZE_64BITS)
	{
		Rb = RB_64BITS;
	}
	else
	{
		return;
	}

	//4-计算 K1	
	moveOneBitLeft(L, blockSize, G_SubKey1);
	
	if((L[0]&0x80) == 0x80)
	{		
		G_SubKey1[blockSize-1] ^= Rb;
	}
	
	//5-计算 K2
	moveOneBitLeft(G_SubKey1, blockSize, G_SubKey2);
	
	if((G_SubKey1[0]&0x80) == 0x80)
	{		
		G_SubKey2[blockSize-1] ^= Rb;
	}
	
	return;
}


/*
*@funtion    :calcCMacInit
*
*@description:计算 subkey, 并初始化CMAC
*            :标准SP_800-38B.pdf  6.1 章节
*
*@param      :BYTE *K -- 原始key, L V 结构
*             BYTE *icv -- 初始值,算AES-CMAC时icv为NULL
*             BYTE blockSize -- 块大小， 128bits | 64bits （即16bytes | 8bytes）
*
*@return     :void
*
*@author     :dingzy
*
*@history    :V1.0 :create by dingzy@20170616
*/
BYTE calcCMacInit(BYTE *K, BYTE *icv, BYTE blockSize)
{
	//2-计算subKey
	subKey(K, blockSize);
	
	//3-设置控制变量
	G_CalcCMacState = CALC_CMAC_STATE_BEGIN;
	G_BlockSize = blockSize;	  			
	
	aesCipherInit(G_MainKey, icv, ALG_ENCRYPT, ALG_AES_CBC, 0x00);
	
	return TRUE;
}

/*
*@funtion    :calcCMacSegment
*
*@description:计算MAC
*            :标准SP_800-38B.pdf  6.2 章节
*
*@param      :BYTE *M  -- 消息
*             WORD MLen
*             BYTE *T  -- 计算得到的 16 字节 MAC值
*             BYTE isLastSegment -- 	是否是最后一段 segment： TRUE-是， FALSE-否
*
*@return     :void
*
*@author     :dingzy
*
*@history    :V1.0 :create by dingzy@20170615
*/
BYTE calcCMacSegment(BYTE *M, WORD MLen, BYTE *T, BYTE isLastSegment)
{		
	BYTE idata n;
	BYTE idata posi;
	BYTE idata modM;
	BYTE xdata lastBlock[CMAC_BLOCKSIZE_MAX];
	
	n = MLen / G_BlockSize;
	
	if (isLastSegment == TRUE)
	{
		memset(lastBlock, 0x00, sizeof(lastBlock));
	
		modM = MLen % G_BlockSize;
		
		if ((modM != 0x00) || (MLen == 0))//not a complete block
		{
			//3-1 补位 800000......
			posi = MLen - modM;
			
			memcpy(lastBlock, &M[posi], modM);
				
			lastBlock[modM] = 0x80;
			
			MLen -= modM;
			
			//问题1：如果是最后一segment，则AES_CBC加密，T将会被冲掉，T至少和M等长
			aesCipherUpdate(M, MLen, M);	 						
			
			XorCalc	(&G_SubKey2[0], &lastBlock[0], CMAC_BLOCKSIZE_MAX);
			
			aesCipherDoFinal(lastBlock, CMAC_BLOCKSIZE_MAX, T);
			
			return TRUE;
		}
		else
		{
			XorCalc	(&G_SubKey1[0], &M[(n-1)*G_BlockSize], G_BlockSize);
		}
	}

	if (MLen>0)
	{
		aesCipherUpdate(M, MLen, M);				
	}
	
	if (isLastSegment == TRUE)
	{
		memcpy(T, &M[(n-1)*G_BlockSize], G_BlockSize);
	}

	return TRUE;
}


#endif

