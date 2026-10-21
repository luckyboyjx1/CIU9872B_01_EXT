/************************************************************************************************
Copyright (C), 1988-2018, Eastcompeace. Co., Ltd.

File name   : SCP03Realize.c

Author:     : xyz  

Version:    : V1.0
              

Description :  该文件主要存放与SCP03相关的功能实现 ，其基本功能来自于规范<GPC_2.2_D_SCP03_v1.1.1.pdf>,&&支持EUICC?M2M   SCP02 4.1.3.2 Command coding for &4.1.3.3 Download and Installation of a Profile

************************************************************************************************/ 
#include "SCP03Realize.h"
#include "ASN1Operation.h"
#include "EUICCCustomer.h"
#include "string.h"
#include "EUICCProfile.h"
#include "Interpreter.h"
#include "ToolkitAPIPublic.h"  //为了引用到G_TARRecIndex  后续考虑封装成接口
#include "cardManager.h"
#include "EUICCMain.h"
#include "Crypto_Constant.h"
#include "opFunction.h"
#include "Interface_LTEFromPlatform.h"
#include "ToolkitRFM.h"
#include "object.h"
#include "keySet.h"
#include "euiccConfig.h"	//remove warning,dengyiwen,2022/9/21
#include "SEEUICCInterface.h"	//remove warning,dengyiwen,2022/9/27

#if  EUICC_FUNC_SCP03t
/*
*@funtion    :increaseICVCounter
*
*@description:将15字节的counter加1
*
*@param      :BYTE *icvCounter
*
*@return     :void
*
*@author     :xuyz
*
*@history    :V1.0 :@20190104
*/
void SCP03tIncreaseICVCounter(BYTE *icvCounter)
{
	BYTE idata i;
	
	i = 15;
	
	icvCounter[i]++;
	
	while(icvCounter[i] == 0) 
	{
		i--;
		
		if(i == 0) 
		{
			ecpMem_Set(icvCounter, 0x00, 0x10);
		}
		
		icvCounter[i]++;
	}
}

/*
*@function     :SCP03tGenICV
*
*@description  :对G_RSPSessionContext.icvCounter进行运算产生SCP03t的ICV 向量
*
@param        ：BYTE sessionKey[16]
*@param       : BYTE sessionKeyLen :加密密钥长度
*@param       : BYTE icvCounter: 待加密的counterValue
*@param       : BYTE outICV[16]
*@param       :BYTE mode : C_DECRYPTION/C_ENCRYTION
*
*@return      : 产生16字节的ICV 字段
*
*@author      :xuyz
*
*@history      :V1.0 create @201901013
*/
void SCP03tGenICV(BYTE *sessionKey, BYTE sessionKeyLen,BYTE *icvCounter, BYTE *outICV, BYTE mode)
{
	BYTE xdata tmpICVCounter[0x10];
	
	ecpMem_Copy(tmpICVCounter, icvCounter, 0x10);
	
	if(mode == C_ENCRYTION)
	{			
		tmpICVCounter[0] = 0x80;  //根据GP 附录D  6.2.7 得知 The padded counter block used for the generation of the ICV for command encryption shall also be used to generate the ICV for response encryption, however, with the following additional intermediate step: Before encryption, the most significant byte of this block shall be set to '80'.  
	}
#if (EUICC_ONLY||SE_AND_EUICC)
#if EUICC__FUNCTION__GUOMI_eSIM
	if(G_CertType == Cert_GUOMI)
	{
		//g_SM4NormalFlag = NormalSM4;
		
		SM4ECB(tmpICVCounter, 0x10, sessionKey ,  sessionKeyLen , MODE_ENCRYPT, (BYTE *)outICV);
		
		//g_SM4NormalFlag = NonnormalSM4;
	}
	else
#endif
#endif
	{
		AESECB(tmpICVCounter, 0x10, sessionKey ,  sessionKeyLen , MODE_ENCRYPT, (BYTE *)outICV);
	}
}
/*
*@function    : SCP03tEncryptionAndRMAC
*
*@description : 在scp03t模式下进行上行报文的加密和mac计算，此函数中返回的数据为86+L+V的数据
*
*@param       :BYTE *inData  :该数据不包括T :RAM
*@param       :BYTE inDataLen  :待加密的数据长度，inDataLen不能为0
*@param       :BYTE operationMode： SECURITYLEVEL_RMAC/SECURITYLEVEL_RENCRYPTION
*@param       :BYTE *outBuf： 输出buf要比输入buf多至少 ( padding(16字节) + mac(8字节)+T(1字节)+L(3字节) )  = 28字节:RAM
*@para        :   BYTE *macKey
*@param       :   BYTE macKeyLen
*@param       :   BYTE *initMacValue   ：固定为16字节
*@param       :   BYTE *encKey
*@param       :   BYTE encKeyLen
*@param       :   BYTE *icvCounter
*
*@return      :WORD outLen
*
*@author      :xuyz
*
*@history     :V1.0 :create @20190220
*/
#if M2M_EUICC
WORD SCP03tEncryptionAndRMAC(BYTE *inData, BYTE inDataLen, BYTE operationMode, BYTE *outBuf,BYTE *macKey,BYTE macKeyLen, BYTE *initMacValue, BYTE *encKey,BYTE encKeyLen,BYTE *icvCounter)
{
	WORD idata tmpLen;
	BYTE idata paddingLen,lenBytes;
	BYTE xdata tmpMacData[0x10];
	
	tmpLen = inDataLen;  //先给tmpLen赋值，后续可能不支持加密，仅支持计算mac
	
	if((operationMode&SECURITYLEVEL_RENCRYPTION) == SECURITYLEVEL_RENCRYPTION )  //对输入数据进行加密
	{
		SCP03tGenICV(encKey, encKeyLen,icvCounter, tmpMacData, C_ENCRYTION);  //6.2.7  APDU Response R -MAC and R-E NCRYPTION Generation  and Verification 
		
		ecpMem_Move(outBuf, inData, inDataLen);   //拷贝输入数据到outBuf缓存中，然后进行填充加密
			
		paddingLen = 0x10 - (inDataLen%0x10);  //2-2: 补位 8000.....
		
		ecpMem_Set(&outBuf[inDataLen], 0x00, paddingLen);
		
		outBuf[inDataLen] = 0x80;
		
		inDataLen += paddingLen;
		
#if EUICC__FUNCTION__GUOMI_eSIM
		if(G_CertType == Cert_GUOMI)
		{
//			g_SM4NormalFlag = NormalSM4;
			
			tmpLen = SM4CBC((BYTE *)outBuf, inDataLen, encKey,  encKeyLen, tmpMacData , 0x10 , MODE_ENCRYPT, (BYTE *)outBuf);
			
			//g_SM4NormalFlag = NonnormalSM4;
		}
		else
#endif
		{
			tmpLen = AESCBC((BYTE *)outBuf, inDataLen, encKey,  encKeyLen, tmpMacData , 0x10 , MODE_ENCRYPT, (BYTE *)outBuf);
		}
		
		if(tmpLen == FALSE)
		{
			return FALSE;
		}	
	}
	
	if((operationMode & SECURITYLEVEL_RMAC) == SECURITYLEVEL_RMAC ) 
	{
		paddingLen = 0;
		
		tmpMacData[paddingLen++] = 0x86;
		
		tmpLen += 0x08;  //86后面的L包括MAC的长度
		
		lenBytes = SetBerTlvLength(&tmpMacData[paddingLen],tmpLen);
		paddingLen += lenBytes;	
		
		tmpLen -= 0x08;  //还原实际待运算的数据长度
		
		ecpMem_Move(outBuf + paddingLen, outBuf, tmpLen);
		
		ecpMem_Copy(outBuf, tmpMacData, paddingLen );
		
		tmpLen += paddingLen;  //参与mac运算的数据长度

#if EUICC__FUNCTION__GUOMI_eSIM
		if(G_CertType == Cert_GUOMI)
		{
//			g_SM4NormalFlag = NormalSM4;
			
			Cmac_SM4(macKey, macKeyLen, (BYTE *)outBuf, tmpLen , initMacValue, 0x10 , tmpMacData);
			
//			g_SM4NormalFlag = NonnormalSM4;
		}
		else
#endif
		{
			Cmac(macKey, macKeyLen, (BYTE *)outBuf, tmpLen , initMacValue, 0x10 , tmpMacData);
		}
		
		ecpMem_Copy(&outBuf[tmpLen], tmpMacData, 0x08);
		
		tmpLen += 0x08;  //加上mac的实际长
	}
	
// #ifndef RSP_EUICC	   //个人版在收到加密数据后，ICV 就加1，设备版在发送响应报文之后，才ICV加1
// 	SCP03tIncreaseICVCounter(icvCounter);  
// #endif	
	return tmpLen;
}
#endif
/*
*@function    :   SCP03tDecryptAndCalMac(ASN1 TLV,tag为单字节)
*
*@description :   在SCP03t模式下进行报文的mac计算和解密操作,解密后的数据写回原位置处，并返回明文数据的总长度(即除去mac和填充数据以外的数据总长度)
*                该函数同时支持RSP版本和M2M 版本中对于SCP03t的管理
*
*@param       :   UWORD srcDataAddr   : TLV
*@param       :   UWORD outDataAddr,  :TLV  (该参数主要针对设备版 下行数据存在文件，为了将解密后数据放在EUICCTempDataBuf和个人版统一 方便PE动态解析 所以增加该变量)如果operationMode没有指示SECURITYLEVEL_CDECRYPTION的，表示该函数不需要解密，数据还是在srcDataAddr中，不会做E2拷贝
*@param       :   WORD srcDataLen     :the totalLen Of TLV
*@oaram       :   BYTE operationMode  :操作模式 SECURITYLEVEL_CMAC | SECURITYLEVEL_CDECRYPTION的各种组合
*@para        :   BYTE *macKey
*@param       :   BYTE macKeyLen
*@param       :   BYTE *initMacValue   ：固定为16字节
*@param       :   BYTE *encKey
*@param       :   BYTE encKeyLen
*@param       :   BYTE *icvCounter
*
*@return      :   WORD clearLen
*
*@author      :  xuyz
*
*@history     :  V1.0 @20190104
*/
WORD SCP03tDecryptAndCalMac(UWORD srcDataAddr,UWORD outDataAddr, WORD srcDataLen, BYTE operationMode,BYTE *macKey,BYTE macKeyLen, BYTE *initMacValue, BYTE *encKey,BYTE encKeyLen,BYTE *icvCounter)
{
	BYTE idata tmp,paddingLen,macLen,i;
	UWORD idata tmpLen;
	UWORD xdata commandDataAddr,tempOutAddr;
	BYTE xdata tmpMacData[0x10];
	
	tmp = ASN1_GetBERTlvLen(srcDataAddr + 1, (UWORD*)&tmpLen);
	
	if( srcDataLen != (tmpLen + tmp + 1) )   //TLV 的长度不匹配
	{
		return FALSE;
	}
	
	if(JcvmConfig_GetSEState() == CARD_OP_READY)
	{
		return srcDataLen;
	}
	
	SCP03tIncreaseICVCounter(icvCounter);
	
	macLen = 0;
	
	if((operationMode&SECURITYLEVEL_CMAC) == SECURITYLEVEL_CMAC)  //需要计算mac
	{
#if (EUICC_ONLY||SE_AND_EUICC)
#if EUICC__FUNCTION__GUOMI_eSIM
		if(G_CertType == Cert_GUOMI)
		{
//			g_SM4NormalFlag = NormalSM4;
			
			Cmac_SM4(macKey, macKeyLen, (BYTE *)srcDataAddr,srcDataLen -8, initMacValue, 0x10 , tmpMacData);
			
//			g_SM4NormalFlag = NonnormalSM4;
		}
		else
#endif
#endif
		{
			Cmac(macKey, macKeyLen, (BYTE *)srcDataAddr,srcDataLen -8, initMacValue, 0x10 , tmpMacData);
		}
		
		if(NvmCmp((BYTE *)(srcDataAddr+srcDataLen -8), tmpMacData, 0x08) != EQUAL)
		{
			return FALSE;
		}
			
		VMWriteBuf(PXtMEM(initMacValue),tmpMacData, 0x10 );  //GP附录D : At any point in time, the last 16 byte C- MAC  computed is part of the channel state and is ref erred to as the “MAC chaining value” further in this document. The first “MAC chaining value” is set to 16  bytes '00' (see section  6.2.3). This chaining (see  Figure  6- 3) ensures the card that all commands in a sequence have been received.
		
		tmpLen -= 8;  //密文数据长度
		
		macLen = 8;
		
	}
	
	commandDataAddr = srcDataAddr + tmp +1;  //point to the CommandData
	
	tempOutAddr = outDataAddr  + tmp +1; //解密后数据存储区跳过TL字段
	
	paddingLen = 0;
	
	if((operationMode&SECURITYLEVEL_CDECRYPTION) == SECURITYLEVEL_CDECRYPTION)  //需要解密数据
	{		
		SCP03tGenICV(encKey,encKeyLen, icvCounter, tmpMacData, C_DECRYPTION);  
#if (EUICC_ONLY||SE_AND_EUICC)
#if EUICC__FUNCTION__GUOMI_eSIM
		if(G_CertType == Cert_GUOMI)
		{
//			g_SM4NormalFlag = NormalSM4;
			
			tmpLen = SM4CBC((BYTE *)commandDataAddr, tmpLen, encKey,  encKeyLen, tmpMacData , 0x10 , MODE_DECRYPT, (BYTE *)tempOutAddr);
			
//			g_SM4NormalFlag = NonnormalSM4;
		}
		else
#endif
#endif
		{
			tmpLen = AESCBC((BYTE *)commandDataAddr, tmpLen, encKey,  encKeyLen, tmpMacData , 0x10 , MODE_DECRYPT, (BYTE *)tempOutAddr);
		}
		
		if(tmpLen == FALSE)
		{
			return FALSE;
		}
		
		VMReadBuf(tmpMacData, tempOutAddr +tmpLen- 0x10, 0x10 );  //读取最后16字节的填充字节，参考GP附录D
		
		for(i=0 ;i < 0x10; i++)   //GP 附录D  4.1.4  AES Padd ing  
		{
			if(tmpMacData[15-i] != 0x00)
			{
				if(tmpMacData[15-i] == 0x80)
				{
					paddingLen = i +1;
					
					break;
				}
				else
				{
					return FALSE;
				}
			}
	
		}
	
		VMReadBuf(tmpMacData,srcDataAddr,1+tmp);
		
		VMWriteBuf(outDataAddr,tmpMacData,1+tmp);//将TL拷贝到outAddr, 解密后结构为tag L + V明文
	}
	
// #if RSP_EUICC	   //个人版在收到加密数据后，ICV 就加1，设备版在发送响应报文之后，才ICV加1
// 	SCP03tIncreaseICVCounter(icvCounter);  //注意： profile的下载过程中 86 87 88都会进入该函数 //The encryption counter for ICV calculation is incremented each time a TLV with tag '86', '87'  or '88' is received. 
// #endif
	
	return (srcDataLen - paddingLen - macLen);
}

WORD AESECB(BYTE *in,WORD inlen, BYTE *key,BYTE keylen,BYTE chipherMode,BYTE *out)
{
	BYTE idata tmpLen;

	tmpLen = SymCipherDofinal(MEMtPM8(in), inlen, MEMtPM8(out), MEMtPM8(key), U8_M_U8(keylen,8), NULL, NULL, ALG_AES_BLOCK_128_ECB_NOPAD, U8_OR_U8(SYM_ECB_MODE, chipherMode), 0x00, ALG_AES);

	if (tmpLen == WRONG_PADDING)
	{
		return FALSE;
	}

	return inlen;
}

WORD AESCBC(BYTE *in, WORD inlen, BYTE *key, BYTE keylen, BYTE *iv, BYTE ivlen, BYTE chipherMode, BYTE *out)
{
	BYTE idata tmpLen;

	tmpLen = SymCipherDofinal(MEMtPM8(in), inlen, MEMtPM8(out), MEMtPM8(key), U8_M_U8(keylen,8), MEMtPM8(iv), NULL, ALG_AES_BLOCK_128_CBC_NOPAD, U8_OR_U8(SYM_CBC_MODE, chipherMode), 0x00, ALG_AES);

	if (tmpLen == WRONG_PADDING)
	{
		return FALSE;
	}

	return inlen;
}

# if  VM_WITH_SM4
WORD SM4ECB(BYTE *in,WORD inlen, BYTE *key,BYTE keylen,BYTE chipherMode,BYTE *out)
{
	BYTE idata tmpLen;

	tmpLen = SymCipherDofinal(MEMtPM8(in), inlen, MEMtPM8(out), MEMtPM8(key), U8_M_U8(keylen,8), NULL, NULL, ALG_SM4_ECB_NOPAD_NEW, U8_OR_U8(SYM_ECB_MODE, chipherMode), 0x00, ALG_SM4);

	if (tmpLen == WRONG_PADDING)
	{
		return FALSE;
	}

	return inlen;
}

WORD SM4CBC(BYTE *in, WORD inlen, BYTE *key, BYTE keylen, BYTE *iv, BYTE ivlen, BYTE chipherMode, BYTE *out)
{
	BYTE idata tmpLen;

	tmpLen = SymCipherDofinal(MEMtPM8(in), inlen, MEMtPM8(out), MEMtPM8(key), U8_M_U8(keylen,8), MEMtPM8(iv), NULL, ALG_SM4_CBC_NOPAD_NEW, U8_OR_U8(SYM_CBC_MODE, chipherMode), 0x00, ALG_SM4);

	if (tmpLen == WRONG_PADDING)
	{
		return FALSE;
	}

	return inlen;
}

BYTE Cmac_SM4(BYTE *key, BYTE keylen, BYTE *M, WORD MLen, BYTE * headMsg, BYTE headMsgLen, BYTE *T)
{
	BYTE xdata IVBuf[SM4_BLOCKSIZE];

	ecpMem_Set(IVBuf, 0x00, SM4_BLOCKSIZE);    //初始向量是Zero block

	symCmacAlgorithmCal(MEMtPM8(headMsg), headMsgLen, MEMtPM8(M), MLen, MEMtPM8(T), MEMtPM8(key), U8_M_U8(keylen,8), MEMtPM8(IVBuf), CBC_MAC_Encrypt, ALG_SM4);

	return SM4_BLOCKSIZE;
}
#endif

#endif
/*
*@funtion    :SCP03CalcAuthenticationCryptogram
*
*@description:
*
*@param      :BYTE *sessionKeySMAC -- 过程密钥 S-MAC
*             BYTE *hostChallenge
*             BYTE *cardChallenge
*             BYTE *cardCryptogram -- 生成的卡片密文 , 16字节，卡外使用8字节即可
*
*@return     :void
*
*@author     :xuyz
*
*@history    :V1.0 :creat额@20190218
*/
//#if M2M_EUICC
//void SCP03CalcAuthenticationCryptogram(BYTE *sessionKeySMAC,BYTE derivationContant, BYTE *hostChallenge, BYTE *cardChallenge, BYTE *cardCryptogram)
//{
//	BYTE xdata dataDerivation[0x20];  //12(label”) +1(“separation indicator) +2(L)+1(i)+8(cardChallenge) + 8(hostChallenge)
//
//	//1-组装 dataDerivation
//	ecpMem_Set(dataDerivation, 0x00, sizeof(dataDerivation));
//
//	dataDerivation[11] = derivationContant;//derivation constant，卡片密文标识： 0x00
//
//	dataDerivation[13] = 0x00;//2 byte integer “L” , 卡片密文长度 0x0040
//	dataDerivation[14] = 0x40;
//
//	dataDerivation[15] = 0x01;//"i",
//
//	ecpMem_Copy(&dataDerivation[16], hostChallenge, 0x08);//host challenge (8 bytes)
//	ecpMem_Copy(&dataDerivation[24], cardChallenge, 0x08);//card challenge (8 bytes)
//
//	//一次PRF输出存放在cardCryptogram ，函数外部取8字节即可
//#if EUICC__FUNCTION__GUOMI_eSIM
//	if(G_CertType == Cert_GUOMI)
//	{
////		g_SM4NormalFlag = NormalSM4;
//
//		Cmac_SM4(&sessionKeySMAC[1], sessionKeySMAC[0], dataDerivation, 0x20, NULL,0,dataDerivation); //复用dataDerivation的前16字节做mac存储。
//
////		g_SM4NormalFlag = NonnormalSM4;
//	}
//	else
//#endif
//	{
//		Cmac(&sessionKeySMAC[1], sessionKeySMAC[0], dataDerivation, 0x20, NULL,0,dataDerivation); //复用dataDerivation的前16字节做mac存储。
//	}
//
//	ecpMem_Copy(cardCryptogram, dataDerivation, 0x08); //提取16字节MAC值的前8字节，作为cardCryptogram
//
//	return;
//}
//#endif
/*
*@funtion    :SCP03_GenSessionKey
*
*@see        :GP 附录D : 6.2.2.1 :The “context” parameter shall be set to the concatenation of the host challenge (8 bytes) and the card challenge (8 bytes).
                         4.1.5 Data Derivation Scheme
*
*@description:计算过程密钥，SCPO3标准中，会从静态密钥集中，分散出过程密钥
*
*@param      :BYTE *staticKey -- 原始密钥（可能为： Key-ENC、Key-MAC）LV结构
*             BYTE derivationConstant -- 分散过程密钥的类型（可能为：S-ENC：0x04， S-MAC：0x06， S-RMAC：0x07）
*             BYTE *hostChallenge
*             BYTE *cardChallenge
*             BYTE *sessionKey
*
*@return     :void
*
*@author     :xuyz
*
*@history    :V1.0 :create by xuyz@20180328
              v1.01 zhhj@20180413
*/
//#if M2M_EUICC
//void SCP03GenSessionKey(BYTE *staticKey, BYTE derivationConstant, BYTE *hostChallenge, BYTE *cardChallenge, BYTE *sessionKey)
//{
//	BYTE xdata dataDerivation[0x20];  //12(label”) +1(“separation indicator) +2(L)+1(i)+8(cardChallenge) + 8(hostChallenge)
//
//	ecpMem_Set(dataDerivation, 0x00, sizeof(dataDerivation));  //1-组装 dataDerivation
//
//	dataDerivation[11] = derivationConstant;//derivation constant
//
//	if(staticKey[0] == 0x20)  //GP附录D : 6.2.2.1: The length of the session keys shall be reflected in the parameter “L” (i.e. '0080' for AES-128 keys, '00C0' for AES-192 keys and '0100' for AES-256 keys).
//	{
//		dataDerivation[13] = 0x01;
//
//		dataDerivation[14] = 0x00;
//	}
//	else if(staticKey[0] == 0x18)
//	{
//		dataDerivation[13] = 0x00;
//
//		dataDerivation[14] = 0xC0;
//	}
//	else
//	{
//		dataDerivation[13] = 0x00;
//
//		dataDerivation[14] = 0x80;
//	}
//
//	dataDerivation[15] = 0x01;//GP附录D: 4.1.5 Data Derivation Scheme A 1 byte counter “i” as specified in the KDF (which may take the values '01' or '02'; value '02' is used when “L” takes the values '00C0' and '0100', i.e. when the PRF of the KDF is to be called twice to generate enough derived data).
//
//	ecpMem_Copy(&dataDerivation[16], hostChallenge, 0x08);//host challenge (8 bytes)
//
//	ecpMem_Copy(&dataDerivation[24], cardChallenge, 0x08);//card challenge (8 bytes)
//
//#if EUICC__FUNCTION__GUOMI_eSIM
//	if(G_CertType == Cert_GUOMI)
//	{
////		g_SM4NormalFlag = NormalSM4;
//
//		Cmac_SM4(&staticKey[1], staticKey[0], dataDerivation, 0x20, NULL,0, sessionKey);  // First PRF 产生1-16字节的输出
//
////		g_SM4NormalFlag = NonnormalSM4;
//	}
//	else
//#endif
//	{
//		Cmac(&staticKey[1], staticKey[0], dataDerivation, 0x20, NULL,0, sessionKey);  // First PRF 产生1-16字节的输出
//	}
//
//	if(staticKey[0] > 0x10)   // 此处的代码可能有点问题，因为程序暂时未考虑超过16字节的AES key的处理，所以此处暂时没有考虑代码的正确性
//	{
//		//which may take the values  ' 01'  or ' 02' ; value ' 02'   is used when “L” takes the values ' 00C0'   and  ' 0100' , i.e. when the PRF of the KDF is to be called twice to generate enough derived data
//
//		dataDerivation[15] = 0x02;//"i", 该值为: 0x01 / 0x02
//
//#if EUICC__FUNCTION__GUOMI_eSIM
//		if(G_CertType == Cert_GUOMI)
//		{
////			g_SM4NormalFlag = NormalSM4;
//
//			Cmac_SM4(&staticKey[1], staticKey[0], dataDerivation, 0x20, NULL,0,sessionKey+0x10);  //second PRF 获取剩余的密钥长度
//
////			g_SM4NormalFlag = NonnormalSM4;
//		}
//		else
//#endif
//		{
//			Cmac(&staticKey[1], staticKey[0], dataDerivation, 0x20, NULL,0,sessionKey+0x10);  //second PRF 获取剩余的密钥长度
//		}
//	}
//}
//#endif

