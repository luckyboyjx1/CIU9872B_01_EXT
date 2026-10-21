/************************************************************************************************************
* Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   TlsAlgorithm.c
*
* Description: 该文件主要用于存放TLS 层算法相关的函数
*
* @see        :<rfc5246.txt.pdf>TLS1.2  <RFC4346.pdf> TLS1.1
*
* auther：     xxxx    xxxx.xx.xx
*
* history：    V1.0 :create @20180411 ,该文档主要存放TLS算法相关的函数
				    
*
************************************************************************************************************/
#include "TlsAlgorithm.h"
#include "TlsMainFlow.h"
#include "HttpProcess.h"
#include "CardManager.h"
#include "Crypto_Constant.h"
#include "Crypto_API.h"
#include "T0ApduImpl.h"
//#include "platform_api.h"
#include "FileAction.h"
#include "fileSystem.h"
#include "Interface_AnnexB.h"
#include "stkhead.h"
#if COS_UPDATE_ONLINE_BY_NativeOS
#include "FunEntryConfig.h"
#include "UpCos.h"
#endif

#if GP__AMENDMENT__B__FOR_MASK

//HMACAlgorithm xdata G_HMACAlgorithm;

/*
*@funtion    :TLSP_HASH
*
*@description:P_HASH算法，主要用于计算SecurityParameters.mac_key_length，产生verify_data等
*
*@see        :<rfc4346TLS1.1.txt.pdf> 5.  HMAC and the Pseudorandom Function
*                                     PRF(secret, label, seed) = P_MD5(S1, label + seed) XOR  P_SHA-1(S2, label + seed);
*             <rfc5246TLS1.2.txt.pdf> 5.  HMAC and the Pseudorandom Function
*                                     PRF(secret, label, seed) = P_<hash>(secret, label + seed)  A(0)=label+seed; A(i) = HMAC_hash(secret, A(i-1)) 
*                                     P_hash(secret, lable+seed) = HMAC_hash(secret, A(1) + lable+seed) +
                                                                   HMAC_hash(secret, A(2) + lable+seed) +
                                                                   HMAC_hash(secret, A(3) + lable+seed) + ...
*
*@param      :BYTE alg       : 当前HMAC的hash算法
*@param      :BYTE *secret   : 密钥 
*@param      :BYTE  secretLen: 密钥长度
*@param      :BYTE * label   ：LV 结构
*@param      :BYTE * seed    : LV 结构
*@param      :BYTE * out     :期望输出的数据缓存: out缓存的长度为 1如果outlen不是hmacLen的整数倍，则为(outLen/hmacLen +1)*hmacLen，否则为outlen
*@param      :BYTE outLen    :期望输出的数据长度
*            
*@return     :void
*
*@history    :V1.0 : created by xuyunzhen @20180411 
			  NOTE: 1:本函数占用了tempBuf的最后HMAC_DIGEST_LENGTH_MAX(32)字节，即如果tempBuf是256 ，则占用了223-255；如果tempBuf是512 ，则占用了479-511
			       2: out缓存的长度为 1如果outlen不是hmacLen的整数倍，则为(outLen/hmacLen +1)*hmacLen，否则为outlen
				     
*/				
void TLSP_HASH(BYTE alg, BYTE * secret, BYTE secretLen, BYTE  * label, BYTE * seed, BYTE * out, BYTE outLen)				
{
//	MD5_CTX  xdata md5_Context;
//	SHA1_CTX xdata sha1Context;
//	SHA_INFO xdata sha256Context;

	u8		X_A			tmpBuff[HASH_BLOCK_SIZE];
	u8		X_A			mdBuff[HASH_ICV_SIZE + 4];
	u8		X_A			hMacGenKeyBuffer[HASH_BLOCK_SIZE + 4];

	BYTE     idata hashLen, hmacLen;
	WORD     idata hmacDataOff;  	
	
	hashLen = 0;
	
	hmacDataOff = sizeof(tempBuf) - HMAC_DIGEST_LENGTH_MAX; 
	
	HMACInit(secret, secretLen, tmpBuff, mdBuff, hMacGenKeyBuffer, alg);
	HMACUpdate(MEMtPM8(&label[1]), label[0], tmpBuff, mdBuff, alg);
	hmacLen =  HMACDofinal(MEMtPM8(&seed[1]), seed[0], MEMtPM8(&tempBuf[hmacDataOff]), mdBuff, tmpBuff, hMacGenKeyBuffer, alg);
	do
	{
		//计算HMAC_hash(secret, A(i) + seed) , seed = label + seed
		HMACInit(secret, secretLen, tmpBuff, mdBuff, hMacGenKeyBuffer, alg);
		HMACUpdate(MEMtPM8(&tempBuf[hmacDataOff]), hmacLen, tmpBuff, mdBuff, alg);
		HMACUpdate(MEMtPM8(&label[1]), label[0], tmpBuff, mdBuff, alg);
		hmacLen =  HMACDofinal(MEMtPM8(&seed[1]), seed[0], MEMtPM8(&out[hashLen]), mdBuff, tmpBuff, hMacGenKeyBuffer, alg);

		hashLen += hmacLen;
		if (hashLen < outLen)
		{
			GeneralHMAC(secret, secretLen, MEMtPM8(&tempBuf[hmacDataOff]), hmacLen, MEMtPM8(&tempBuf[hmacDataOff]), hmacLen, alg);
			//计算A(i) = HMAC_hash(secret, A(i-1)) tempBuf[hmacDataOff]存放A[i-1]
		}

	}while(hashLen < outLen);
}

/*
*@description:  实现TLS中所使用的PRF算法，其中TLS1.0、TLS1.1与TLS1.2中的PRF 算法不一致，具体参考rfc4346TLS.txt.pdf 和 rfc5246TLS1.2.txt.pdf 的secrion 5 章节
*               TLS1.0和TLS1.1中算法： PRF(secret, label, seed) = P_MD5(S1, label + seed) XOR P_SHA-1(S2, label + seed);
*               TLS1.2中算法：         PRF(secret, label, seed) = P_<hash>(secret, label + seed)
*
*@function   :TLSPRF 
*
*@param      :BYTE *secret   : 密钥 
*@param      :BYTE  secretLen: 密钥长度
*@param      :BYTE * label   ：LV 结构
*@param      :BYTE * seed    : LV 结构
*@param      :BYTE * out     : 期望输出的数据缓存， 输出数据可以占用tempBuf[0]
*@param      :BYTE outLen    : 期望输出的数据长度
*
*@return     :void
*
*@note       :为支持TLS1.0、TLS1.1、TLS1.2所有调用TLF的地方，该函数借用了tempBuf[0-400].作为中间缓存数据，后续如果要移植到小资源的芯片，将相关的中间运算结果可以暂时存放于某个临时文件中
              因此输入数据不可以使用tempBuf作为输入缓存
*
*@history    :V1.0 :created by xuyunzhen @20180411
*/
void TLSPRF(BYTE * secret,BYTE secretLen, BYTE * label, BYTE * seed, BYTE * out, BYTE outLen)	
{
	BYTE idata s2FirstByteOff;
	BYTE idata halfSecretLen;
	
	if(G_TLSSecurityParameters.tlsProtcolVersion < TLS_Version12)  //TLS1.0 或 TLS1.1 <rfc4346TLS1.1.txt.pdf> 5. HMAC and the Pseudorandom Function
	{
		if((secretLen&0x01) == 0x00)   // secretLen为偶数
		{
			halfSecretLen = secretLen / 2 ;
		}
		else  // secretLen为奇数
		{
			halfSecretLen = (secretLen / 2) + 1;
		}
		
		s2FirstByteOff = secretLen / 2;
		
		TLSP_HASH(ALG_HMAC_MD5,  &secret[0], halfSecretLen, &label[0], &seed[0], &tempBuf[0], outLen); //最大与TLS1.0 1.1 来说，嗲用PRF最大长度为136字节，规范rfc4346TLS.txt.pdf 6.3. Key Calculation 章节处 . the most material is AES_256_CBC_SHA. It requires 2 x 32 byte keys, 2 x 20 byte MAC secrets, and 2 x 16 byte Initialization Vectors, for a total of 136 bytes of key material.
		
		TLSP_HASH(ALG_HMAC_SHA1, &secret[s2FirstByteOff], halfSecretLen, &label[0], &seed[0], &tempBuf[outLen], outLen);  
		
		for(halfSecretLen =0 ; halfSecretLen < outLen; halfSecretLen ++)  //借用halfSecretLen 作为循环变量的缓存, outlen的最大长度为128字节
		{
			out[halfSecretLen] = tempBuf[halfSecretLen] ^ tempBuf[outLen+ halfSecretLen];
		}
	}
	else     //TLS 1.2
	{
		TLSP_HASH(ALG_HMAC_SHA_256, secret, secretLen, label, seed, &tempBuf[0], outLen);  //对于TLS1.2 来说，最大的数据长度为计算rfc5246TLS1.2.txt.pdf  6.3.  Key Calculation 6 组key时的运算，最大需要192 字节(包括两组IV向量)
		
		memmove(&out[0], &tempBuf[0], outLen);		
	}
}

/*
*@description:按照TLS1.0 TLS1.1 TLS1.2进行mac key  encry_key  iv等数据的提取
*
*@note        :计算key所需要的输入数据master_secret, client_random, server_random已缓存在特定的文件里，从对应文件获取即可
               key_block = PRF(SecurityParameters.master_secret, "key expansion", SecurityParameters.server_random + SecurityParameters.client_random);

				Note:对于 TLS1.1 1.2 ，不需要产生IV 向量，对于TLS1.0 ，需要产生IV，用作下次加密
*
*@see        :<rfc5246TLS1.2.txt.pdf>  6.3. Key Calculation
*             <rfc4346TLS1.1.txt.pdf>  6.3. Key Calculation
			  <rfc2246TLS1.0.txt.pdf>  6.3. Key Calculation
*
*@function   :TLSKeyBlockCalculation
*
*@param      :void
*
*@return     :void
*
*@history    :1.0 :created by xuyunzhen @20180411
*
*/
void TLSKeyBlockCalculation(void)
{
	BYTE idata keyCalLen,ivLen;  //根据ciper算法，mac算法等计算当前keyCal需要的长度
	BYTE xdata masterKeyBuf[48];  //暂时用来缓存master Secret,后期由于资源问题，要考虑调整该全局变量的使用
	BYTE xdata randomBuf[65]; //暂时用来缓存random ,后期由于资源问题，要考虑调整该全局变量的使用

#if (GP__AMENDMENT__B__TLS_SessionKey_IN_FILE || GP__AMENDMENT__B__TLS_MasterKey_IN_FILE)
	UWORD xdata fileAddr;
#endif
	if(G_TLSSecurityParameters.tlsProtcolVersion == TLS_Version10) //With block ciphers in CBC mode (Cipher Block Chaining) the initialization vector (IV) for the first record is generated with the other keys and secrets when the security parameters are set. The IV for subsequent records is the last ciphertext block from  the previous record
	{
		ivLen = G_TLSSecurityParameters.blockLength ;
	}
	else   //对于TLS1.1 ，没有IV key的产生，对于TLS1.2 Currently, the client_write_IV and server_write_IV are only generated for implicit nonce techniques as described in Section 3.2.1 of [AEAD]
	{
		ivLen = 0 ;
	}

	keyCalLen = (G_TLSSecurityParameters.enckeyLength + ivLen + G_TLSSecurityParameters.macKeyLength) * 2;

	randomBuf[0] = TLS_ServerRandom_Len + TLS_ClientRandom_Len;

#if GP__AMENDMENT__B__TLS_SessionKey_IN_FILE
//	fileAddr =  SelectPrivateTFFile(DF_TLS,EF_HTTPSCommandData);  // 选择TLS 握手文件，该文件缓存了maser-scret，random等信息
	fileAddr = (UWORD)HTTP_CommandData_ADDR_START;

//	ReadTFBuf(fileAddr,TLS_MasterSecret_Offset, masterKeyBuf , TLS_MaterSecret_Len); //从缓存文件中读取48字节的masterKey
	ReadEeprom(masterKeyBuf, fileAddr+TLS_MasterSecret_Offset, TLS_MaterSecret_Len)

	fileAddr =  SelectPrivateTFFile(DF_TLS,EF_TLSKey);

	ReadTFBuf(fileAddr,TLS_ServerRandom_Offset, &randomBuf[1] , randomBuf[0]);  //读取serverRandom + ClientRandom，一共64字节

	TLSPRF(masterKeyBuf, TLS_MaterSecret_Len, (BYTE *)&LABEL_KEY_EXPANSION[0], randomBuf, &tempBuf[0], keyCalLen);

	WriteTFBuf(fileAddr,Client_WirteMacKey_Value_Off,&tempBuf[0], G_TLSSecurityParameters.macKeyLength);

	keyCalLen = G_TLSSecurityParameters.macKeyLength;   //复用该变量表示在tempBuf中的偏移

	WriteTFBuf(fileAddr,Server_WirteMacKey_Value_Off,&tempBuf[keyCalLen], G_TLSSecurityParameters.macKeyLength);

	keyCalLen += G_TLSSecurityParameters.macKeyLength;

	WriteTFBuf(fileAddr,Client_WirteKey_Value_Off,&tempBuf[keyCalLen], G_TLSSecurityParameters.enckeyLength);

	keyCalLen += G_TLSSecurityParameters.enckeyLength;

	WriteTFBuf(fileAddr,Server_WirteKey_Value_Off,&tempBuf[keyCalLen], G_TLSSecurityParameters.enckeyLength);

	keyCalLen += G_TLSSecurityParameters.enckeyLength;

	if(ivLen != 0)
	{
		WriteTFBuf(fileAddr,Client_IV_Value_Off,&tempBuf[keyCalLen], ivLen);

		keyCalLen += ivLen;

		WriteTFBuf(fileAddr,Server_IV_Value_Off,&tempBuf[keyCalLen], ivLen);
	}

	tempBuf[0] = G_TLSSecurityParameters.macKeyLength;

	tempBuf[1] = G_TLSSecurityParameters.macKeyLength;

	tempBuf[2] = G_TLSSecurityParameters.enckeyLength;

	tempBuf[3] = G_TLSSecurityParameters.enckeyLength;

	tempBuf[4] = ivLen;

	tempBuf[5] = ivLen;

	WriteTFBuf(fileAddr,Tls_KeyLen_Off,&tempBuf[0], 0x06);  //更新本次会话中6只key的长度
#else

			
	memcpy(randomBuf+1, G_TLSSessionKeyBuf+TLS_ClientRandom_Len, TLS_ServerRandom_Len);		//G_TLSSessionKeyBuf
	
	memcpy(randomBuf+1+TLS_ServerRandom_Len, G_TLSSessionKeyBuf, TLS_ClientRandom_Len);

	#if GP__AMENDMENT__B__TLS_MasterKey_IN_FILE
//	fileAddr =  SelectPrivateTFFile(DF_TLS,EF_HTTPSCommandData);
	fileAddr = (UWORD)HTTP_CommandData_ADDR_START;

//	ReadTFBuf(fileAddr,TLS_MasterSecret_Offset, masterKeyBuf , TLS_MaterSecret_Len); //从缓存文件中读取48字节的masterKey
	ReadEeprom(masterKeyBuf, fileAddr+TLS_MasterSecret_Offset, TLS_MaterSecret_Len);
	#else
	memcpy(masterKeyBuf, G_TLSMaterSecretBuf, sizeof(masterKeyBuf));		//masterKeyBuf
	#endif

	TLSPRF(masterKeyBuf, TLS_MaterSecret_Len, (BYTE *)&LABEL_KEY_EXPANSION[0], randomBuf, tempBuf, keyCalLen);
	
	memcpy(G_TLSSessionKeyBuf, tempBuf, sizeof(G_TLSSessionKeyBuf));
#endif
}

/*
*@description : 从DF_TLS/EF_TLSKey (A100/9101)文件中 获取TLS握手过程中生成的write MAC secret write key iv 等数据
*
*@see         ： no   A100/9101文件
*
*@function    :TLSGetSessionKey
*
*@param       :BYTE keyFileDataOff: 取值为 Client_WirteMacKey_Value_Off /Server_WirteMacKey_Value_Off/Client_WirteKey_Value_Off/Server_WirteKey_Value_Off/Client_IV_Value_Off/Server_IV_Value_Off等中间的一个
*@param       :BYTE * outBuf      : LV结构，最大为33字节

*@return      :void
*
*@history     :created by xuxiaojun@20180510
*@NOTE        :该函数内部会借用tempBuf[0...5]，所以调用该函数时需要注意数据别被冲掉
*             :定义一个6字节的局部数组，避免复用tempBuf, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void TLSGetSessionKey(BYTE keyFileDataOff,BYTE *outBuf)
{		
#if GP__AMENDMENT__B__TLS_SessionKey_IN_FILE
	UWORD xdata fileAddr;
	BYTE xdata tmp[6];

	memset(tmp, 0x00, sizeof(tmp));

	fileAddr =  SelectPrivateTFFile(DF_TLS,EF_TLSKey);

	ReadTFBuf(fileAddr,Tls_KeyLen_Off, tmp , 0x06);  //获取A100/9101文件中的6组KEY的长度

	switch(keyFileDataOff)
	{
		case Client_WirteMacKey_Value_Off:
			outBuf[0] = tmp[0];
			break;

		case Server_WirteMacKey_Value_Off:
			outBuf[0] = tmp[1];
			break;

		case Client_WirteKey_Value_Off:
			outBuf[0] = tmp[2];
			break;

		case Server_WirteKey_Value_Off:
			outBuf[0] = tmp[3];
			break;

		case Client_IV_Value_Off:
			outBuf[0] = tmp[4];
			break;

		case Server_IV_Value_Off:
			outBuf[0] = tmp[5];
			break;
	}

	ReadTFBuf(fileAddr,keyFileDataOff, &outBuf[1] , outBuf[0]);
#else
	switch(keyFileDataOff)
	{
		case Client_WirteMacKey_Value_Off:		
			outBuf[0] = G_TLSSecurityParameters.macKeyLength;
		
			memcpy(outBuf+1, G_TLSSessionKeyBuf, outBuf[0]);
			break;
		
		case Server_WirteMacKey_Value_Off:
			outBuf[0] = G_TLSSecurityParameters.macKeyLength;
		
			memcpy(outBuf+1, G_TLSSessionKeyBuf+G_TLSSecurityParameters.macKeyLength, outBuf[0]);
			break;
		
		case Client_WirteKey_Value_Off:
			outBuf[0] = G_TLSSecurityParameters.enckeyLength;
		
			memcpy(outBuf+1, G_TLSSessionKeyBuf+2*G_TLSSecurityParameters.macKeyLength, outBuf[0]);
			break;
		
		case Server_WirteKey_Value_Off:
			outBuf[0] = G_TLSSecurityParameters.enckeyLength;
		
			memcpy(outBuf+1, G_TLSSessionKeyBuf+2*G_TLSSecurityParameters.macKeyLength+G_TLSSecurityParameters.enckeyLength, outBuf[0]);
			break;
		
		case Client_IV_Value_Off:
			outBuf[0] = G_TLSSecurityParameters.blockLength;
		
			memcpy(outBuf+1, G_TLSSessionKeyBuf+2*G_TLSSecurityParameters.macKeyLength+2*G_TLSSecurityParameters.enckeyLength, outBuf[0]);
			break;
		
		case Server_IV_Value_Off:
			outBuf[0] = G_TLSSecurityParameters.blockLength;
		
			memcpy(outBuf+1, G_TLSSessionKeyBuf+2*G_TLSSecurityParameters.macKeyLength+2*G_TLSSecurityParameters.enckeyLength+G_TLSSecurityParameters.blockLength, outBuf[0]);
			break;
		default: 			  //此处为了白盒测试添加
			break;
	}
#endif
}
/*
*@description:更新session key TLS握手过程中生成的write MAC secret write key iv 等数据. 长度不需要更新，直接写值即可
*
*@function   :TLSWriteSessionKey
*
*@param      :BYTE keyFileDataOff: 取值为 Client_WirteMacKey_Value_Off /Server_WirteMacKey_Value_Off/Client_WirteKey_Value_Off/Server_WirteKey_Value_Off/Client_IV_Value_Off/Server_IV_Value_Off等中间的一个
*@param      :BYTE *inBuf        : V结构，最大为32字节，有效数据长度由keyFileDataOff在文件中的len决定
*
*@return     :void
*
*@history    :created by xuxiaojun@20180510
*/
void TLSWriteSessionKey(BYTE keyFileDataOff,BYTE *inBuf)
{	
#if GP__AMENDMENT__B__TLS_SessionKey_IN_FILE
	UWORD xdata fileAddr;
	BYTE idata keyLen;

	keyLen = 0;

	fileAddr =  SelectPrivateTFFile(DF_TLS,EF_TLSKey);

	ReadTFBuf(fileAddr,Tls_KeyLen_Off, tempBuf , 0x06);  //获取A100/9101文件中的6组KEY的长度

	switch(keyFileDataOff)
	{
		case Client_WirteMacKey_Value_Off:
			keyLen = tempBuf[0];
			break;

		case Server_WirteMacKey_Value_Off:
			keyLen = tempBuf[1];
			break;

		case Client_WirteKey_Value_Off:
			keyLen = tempBuf[2];
			break;

		case Server_WirteKey_Value_Off:
			keyLen = tempBuf[3];
			break;

		case Client_IV_Value_Off:
			keyLen = tempBuf[4];
			break;

		case Server_IV_Value_Off:
			keyLen = tempBuf[5];
			break;
	}

	WriteTFBuf(fileAddr, keyFileDataOff, inBuf, keyLen);
#else
	switch(keyFileDataOff)
	{
		case Client_WirteMacKey_Value_Off:		
			memcpy(G_TLSSessionKeyBuf, inBuf, G_TLSSecurityParameters.macKeyLength);
			break;
		
		case Server_WirteMacKey_Value_Off:
			memcpy(G_TLSSessionKeyBuf+G_TLSSecurityParameters.macKeyLength, inBuf, G_TLSSecurityParameters.macKeyLength);
			break;
		
		case Client_WirteKey_Value_Off:
			memcpy(G_TLSSessionKeyBuf+2*G_TLSSecurityParameters.macKeyLength, inBuf, G_TLSSecurityParameters.enckeyLength);
			break;
		
		case Server_WirteKey_Value_Off:
			memcpy(G_TLSSessionKeyBuf+2*G_TLSSecurityParameters.macKeyLength+G_TLSSecurityParameters.enckeyLength, inBuf, G_TLSSecurityParameters.enckeyLength);
			break;
		
		case Client_IV_Value_Off:
			memcpy(G_TLSSessionKeyBuf+2*G_TLSSecurityParameters.macKeyLength+2*G_TLSSecurityParameters.enckeyLength, inBuf, G_TLSSecurityParameters.blockLength);
			break;
		
		case Server_IV_Value_Off:
			memcpy(G_TLSSessionKeyBuf+2*G_TLSSecurityParameters.macKeyLength+2*G_TLSSecurityParameters.enckeyLength+G_TLSSecurityParameters.blockLength, inBuf, G_TLSSecurityParameters.blockLength);
			break;
		default: 			  //此处为了白盒测试添加
			break;
	}
#endif
}
/*
×@description  ：通过pre_master_secret计算master_secret
*
*@see          : <rfc4346TLS1.1.txt.pdf>:  8.1.  Computing the Master Secret master_secret = PRF(pre_master_secret, "master secret", ClientHello.random + ServerHello.random)[0..47];
*                <rfc5246TLS1.2.txt.pdf>:  8.1.  Computing the Master Secret master_secret = PRF(pre_master_secret, "master secret", ClientHello.random + ServerHello.random)[0..47];
*
*@function     :TLSCalMasterSecret
*
*@param        :void
*
*@return       :void
*
*@history      :V1.0 create by xuyunzhen @20180412
*
*@NOTE         :本函数的实现占用了tempBuf[128- (128+TLS_MaterSecret_Len)] ,tempBuf[0-(TLS_MaterSecret_Len)] tempBuf[200- (200+TLS_MaterSecret_Len)]
*/
void TLSCalMasterSecret(void)
{
	BYTE xdata randomBuf[65]; //暂时用来缓存random ,后期由于资源问题，要考虑调整该全局变量的使用
	BYTE idata preMasterSecretLen;
#if (GP__AMENDMENT__B__TLS_SessionKey_IN_FILE || GP__AMENDMENT__B__TLS_MasterKey_IN_FILE)
	UWORD xdata fileAddr;
#endif

	preMasterSecretLen = TLSGetPreMasterSecret(&tempBuf[128]);  //计算 preMasterSecretLen
	
	randomBuf[0] = TLS_ServerRandom_Len + TLS_ClientRandom_Len; 
	
#if GP__AMENDMENT__B__TLS_SessionKey_IN_FILE
	fileAddr =  SelectPrivateTFFile(DF_TLS,EF_TLSKey);

	ReadTFBuf(fileAddr, TLS_ClientRandom_Offset, &randomBuf[1], TLS_ClientRandom_Len);  //先读取client random

	ReadTFBuf(fileAddr, TLS_ServerRandom_Offset, &randomBuf[1 + TLS_ClientRandom_Len], TLS_ServerRandom_Len);  //再读取server random
#else
	memcpy(randomBuf+1, G_TLSSessionKeyBuf, TLS_ClientRandom_Len);
	
	memcpy(randomBuf+1+TLS_ClientRandom_Len, G_TLSSessionKeyBuf+TLS_ClientRandom_Len, TLS_ServerRandom_Len);
#endif

	TLSPRF(&tempBuf[128], preMasterSecretLen, (BYTE *)&LABEL_MASTER_SECRET[0], randomBuf, &tempBuf[0], TLS_MaterSecret_Len); //进行masrer_secret计算

#if GP__AMENDMENT__B__TLS_MasterKey_IN_FILE
//	fileAddr =  SelectPrivateTFFile(DF_TLS,EF_HTTPSCommandData);
	fileAddr = (UWORD)HTTP_CommandData_ADDR_START;

//	WriteTFBuf(fileAddr,TLS_MasterSecret_Offset, &tempBuf[0], TLS_MaterSecret_Len);
	WriteEepromWithoutProtect(fileAddr+TLS_MasterSecret_Offset, &tempBuf[0], TLS_MaterSecret_Len);
#else
	memcpy(G_TLSMaterSecretBuf ,tempBuf, TLS_MaterSecret_Len);
#endif
}

/*
*@descrition :获取PSK的值，PSK的值可以根据85tag指示的key version和key id来获取，也可以根据ICCID和master key来分散得到
*
*@see        :<GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf> Security Parameters
*             <TLS key derivation.pdf> 根据ICCID和master key来分散得到
*
*@function   :TLSGetPSK
*
*@param      :BYTE* psk     获取到的psk的值
*
*@return     :BYTE          psk的长度
*
*@history    :created by xuyunzhen@20180413
*/
BYTE TLSGetPSK(BYTE* psk)
{
	UWORD xdata FileAddr/*, keyAddr*/;
	BYTE idata keyversion, pskLen;  //这里有一个奇怪的现象，如果把pskLen去掉，当前用到pskLen的地方用len代替的话，编译出来的代码会比使用pskLen的大8字节
	WORD idata offset/*, keyAddrRef*/;
	
	FileAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPTriggerPara);  //A100/9106存储触发参数

	if(FALSE == FileAddr)
	{
		return 0;
	}
	
	ReadLFBuf(FileAddr, HTTP_TriggerPara_SecurityPara_Index, 2, &pskLen, sizeof(BYTE));
	
	offset = (2 + 1 + pskLen + 1);
	
	ReadLFBuf(FileAddr, HTTP_TriggerPara_SecurityPara_Index, offset, (BYTE*)&keyversion, sizeof(BYTE));
	
	pskLen = 0;
	
	if (IsSCP81Key(keyversion))
	{
#if !JAVA_CARD_UICC
	#if COS_UPDATE_ONLINE_BY_NativeOS
		pskLen  = IF_UPCOS_GetPSK(psk, keyversion,G_HTTPSStatus.TriggeredSD);
		
		if(pskLen != 0)// pskLen = 0,继续查找其他业务的PSK key;
		{
			return pskLen;
		}
#endif
#if UICC__FUNCTION__RFM		
		pskLen = keyData_getRFMKey(keyversion, 0x01, psk);				
#endif		
#else
		pskLen = keyData_getKeyData(G_HTTPSStatus.TriggeredSD, keyversion, 0x01, PMtMEM(psk)); //假设PSK的kid被转换成了1

		while (pskLen == 0)	//G_HTTPSStatus.TriggeredSD
		{
			G_HTTPSStatus.TriggeredSD = getAssociateSDId(G_HTTPSStatus.TriggeredSD);	//只会返回两种值，返回不同的G_HTTPSStatus.TriggeredSD时表示上一层关联安全域的sdid，返回0xFF表示当前G_HTTPSStatus.TriggeredSD是自关联

			if (G_HTTPSStatus.TriggeredSD == ILLEGAL_APPID)
			{
				break;
			}

			pskLen = keyData_getKeyData(G_HTTPSStatus.TriggeredSD, keyversion, 0x01, PMtMEM(psk));
		}

		//参考LTE2.0， 在https握手获取到正确的key时， 对当前的scp81通道进行赋值RAM模式，后续用于判断当前下行指令是否是安全可靠指令
		if (pskLen > 0)
		{
			bRamBuffer[REMOTE_SCP_SDID] = G_HTTPSStatus.TriggeredSD;
		}
#endif
		return pskLen;
	}
	else
	{
		return 0;
	}
}
/*
*@descrition :获取pre_master_secret, 目前卡片支持的算法都是PSK 相关的算法
*
*@see        :<rfc4279-PSK.txt.pdf>   2.  PSK Key Exchange Algorithm 
*             The premaster secret is formed as follows: if the PSK is N octets long, concatenate a uint16 with the value N, N zero octets, a second  uint16 with the value N, and the PSK itself.
*             假设N的值为32字节即PSK的长度为32字节，则premaster secret为0x0020 00000000....00 0020 PSK
*@function   :TLSGetPreMasterSecret
*
*@param      :BYTE *preMasterSecretBuf: 存放preMasterSecretBuf的缓存地址
*
*@return     :BYTE preMasterSecretLen  长度为2字节的psk的长度+psk长度字节的00+2字节的psk长度+psk
*
*@history    :created by xuyunzhen@20180413
*/
BYTE TLSGetPreMasterSecret(BYTE *preMasterSecretBuf)
{
	BYTE idata pskLen;

	pskLen = TLSGetPSK(preMasterSecretBuf);
	
	//modify by jiangqingbin@20220518 此处调整到解析参数时调用TLSGetPSK(MULL)进行判断期望的scp81key是否存在,后续需要统一到所有平台，以下判断key存在代码可以注释
//	if(pskLen == 0)
//	{
//#if NATIVE_CARD_UICC
//		IncorrentParameter();	//ThrowError(SW_WRONG_DATA);
//		
//		return FALSE;
//#else
//		ThrowError(SW_WRONG_DATA);	
//#endif	
//	}
	
	memmove((BYTE *)&preMasterSecretBuf[pskLen+4], (BYTE *)&preMasterSecretBuf[0], pskLen);
	
	memset((BYTE *)&preMasterSecretBuf[0], 0x00, pskLen+4);
		
	preMasterSecretBuf[1]= pskLen;  //omit : preMasterSecretBuf[0] = 0x00;
	
	preMasterSecretBuf[pskLen + 3]= pskLen; //omit : preMasterSecretBuf[pskLen +2] = 0x00;   对后半段的数据长度进行赋值
	
	return (pskLen*2+4);
}

/*
*@description: 获取TLS 握手过程中的Finished中的verify_data数据，用于发送finished 时所用，也可以用于收到finished时校验
*
*@see        : <rfc4346TLS1.1.txt.pdf> 7.4.9.  Finished  verify_data = PRF(master_secret, finished_label, MD5(handshake_messages) + SHA-1(handshake_messages)) [0..11];
*              <rfc5246TLS1.2.txt.pdf>   7.4.9.  Finished  verify_data = PRF(master_secret, finished_label, Hash(handshake_messages))[0..verify_data_length-1];
*              struct {
*                  opaque verify_data[12];
*              } Finished;
*
@function    : TLSGetFinishedVerifyData
*
*@param      : BYTE *verifyData: 存放verifyData数据的缓存，该数据是属于输出
*@param      : BYTE * label    : For Finished messages sent by the client, the string "client finished".  
*                                For Finished messages sent by the server, the string "server finished".
*
*return      : verifyDataLen
*
*@history    : V1.0 :created by xuyunzhen@20180412
*
*@NOTE       :本函数的实现占用了tempBuf[0x41- (0x41+HMAC_DIGEST_LENGTH_MD5(16)+HMAC_DIGEST_LENGTH_SHA1(20))]用于存储hashValue,tempBuf[0x40]是hashValueLen
*                               tempBuf[0x80, 0x80+TLS_MaterSecret_Len(48)]使用时注意是否会越界. 该函数中的tempBuf的有效作用范围仅限于函数内部
*/
BYTE TLSGetFinishedVerifyData(BYTE *verifyData,  BYTE * label)
{		
#if GP__AMENDMENT__B__TLS_MasterKey_IN_FILE
	UWORD xdata fileAddr;
#endif

	if ((G_TLSSecurityParameters.clientChangeCipher == TRUE) && (G_TLSSecurityParameters.serverChangeCipher == FALSE))
	{
		tempBuf[0x40] = HttpsHandShakeHashUpdata(NULL, 0, TLS_LAST_BLOCK, &tempBuf[0x41]);	//clientfinish
	}
	else
	{
		tempBuf[0x40] = HttpsHandShakeHashUpdata(NULL, 0, TLS_ONLY_BLOCK, &tempBuf[0x41]);	//serverfinish
	}

#if GP__AMENDMENT__B__TLS_MasterKey_IN_FILE
//	fileAddr =  SelectPrivateTFFile(DF_TLS,EF_HTTPSCommandData);
       fileAddr = (UWORD)HTTP_CommandData_ADDR_START;

//	ReadTFBuf(fileAddr,TLS_MasterSecret_Offset, &tempBuf[0x80] , TLS_MaterSecret_Len);
	ReadEeprom(&tempBuf[0x80], fileAddr+TLS_MasterSecret_Offset, TLS_MaterSecret_Len);
#else
	memcpy(tempBuf+0x80, G_TLSMaterSecretBuf, TLS_MaterSecret_Len);
#endif
	TLSPRF(&tempBuf[0x80], TLS_MaterSecret_Len, (BYTE *)&label[0], &tempBuf[0x40], &verifyData[0], TLS_VerifyData_Len); //进行verify data的计算
	
	return TLS_VerifyData_Len;
}
/*
*@funtion     :TLSCalMsgMAC 
*
*@descrioption:计算TLS 层的MAC 值，针对上行和下行数据,计算mac，mac的运算 参考
*
*@see         :MAC(MAC_write_key, seq_num +
                            TLSCompressed.type +
                            TLSCompressed.version +
                            TLSCompressed.length +
                            TLSCompressed.fragment);
*
*@param       :BYTE msgMode    : ClientUpMsg_Mode/ServerDownMsg_Mode
*@param       :BYTE blockMode  : TLS_ONLY_BLOCK/TLS_LAST_BLOCK/TLS_FIRST_BLOCK
*@param       :BYTE *macKeyBuf : LV 结构
*@param       :BYTE *dataBuf   : 用于计算mac的数据起始位置，当blockMode&TLS_FIRST_BLOCK == TLS_FIRST_BLOCK时，其数据格式为type+version+length+fragment，fragment可能会包括iv+content+padding
*                                该函数内部不需要考虑IV是否存在，其由函数调用者通过ommitLen来进行控制
*@param       :BYTE dataLen    : 计算mac的总数据长度
*@param       :BYTE ommitLen   : 待忽略的数据长度，此处固定从 dataBuf[TLS_RecordHead_LENGTH] 开始忽略ommitLen 长的数据,忽略初始向量的长度。
*                                该值只有在blockMode&TLS_FIRST_BLOCK == TLS_FIRST_BLOCK时才有效，其他值是该参数无意义
×@param       :BYTE *macBuf    : 计算得到的mac值, mac的长度为G_TLSSecurityParameters.macLength
*
*@param       :BYTE *HashWorkSpace    :tmpBuff、mdBuff和hMacGenKeyBuffer的结合，用作HMAC计算的中间变量
*
*@return      :void
*
*@history     :created by xuxiaojun@20180509
*/
void TLSCalMsgMAC(BYTE msgMode, BYTE blockMode, BYTE *macKeyBuf, BYTE *dataBuf, WORD dataLen, BYTE ommitLen, BYTE *macBuf, BYTE * HashWorkSpace)
{
	BYTE xdata seq[8];  // sequence numbers are 64 bits long
	BYTE idata offset;

	PX8 tmpBuff;
	PX8 mdBuff;
	PX8 hMacGenKeyBuffer;

	offset 				= 0;
	tmpBuff 			= HashWorkSpace;
	mdBuff  			= PX8_A_U16(tmpBuff, HASH_BLOCK_SIZE);
	hMacGenKeyBuffer  	= PX8_A_U16(mdBuff, (HASH_ICV_SIZE + 4) );

	if((blockMode & TLS_FIRST_BLOCK) == TLS_FIRST_BLOCK)    //当前是第一块
	{
		memset(seq, 0x00, 0x08);
		if(msgMode == ServerDownMsg_Mode)
		{
			SetWord(&seq[6], G_TLSSecurityParameters.serverSeqNum);
		}

		if(msgMode == ClientUpMsg_Mode)
		{
			SetWord(&seq[6], G_TLSSecurityParameters.clientSeqNum);
		}

		HMACInit(MEMtPM8(&macKeyBuf[1]), macKeyBuf[0], tmpBuff, mdBuff, hMacGenKeyBuffer, G_TLSSecurityParameters.macAlgorithm);
		HMACUpdate(seq, 0x08, tmpBuff,mdBuff, G_TLSSecurityParameters.macAlgorithm);
		HMACUpdate(dataBuf, TLS_RecordHead_LENGTH, tmpBuff, mdBuff, G_TLSSecurityParameters.macAlgorithm);
				
		dataLen -= (TLS_RecordHead_LENGTH + ommitLen);  //剩余需要计算的数据长度
		
		offset += (TLS_RecordHead_LENGTH + ommitLen);   //剩余需要计算的数据起始位置偏移
	} 
	
	if((blockMode & TLS_LAST_BLOCK) == TLS_LAST_BLOCK)  //当前是最后一块
	{
		HMACDofinal(MEMtPM8(&dataBuf[offset]), dataLen, macBuf, mdBuff, tmpBuff, hMacGenKeyBuffer, G_TLSSecurityParameters.macAlgorithm);
	}
	else    //当前是中间块
	{
		HMACUpdate(MEMtPM8(&dataBuf[offset]), dataLen, tmpBuff, mdBuff, G_TLSSecurityParameters.macAlgorithm);
	}
}
/*
*@function   :  TLSGetPendingLen
*
*@description:  TLS计算上行数据的补位长度
                 如果数据长度为 81 ，blockLen为8 ，则补位 长度为7 ，则 uint8 padding[GenericBlockCipher.padding_length] = 6个6 padding_length，此函数返回值为7即可
*
*@see        : <rfc5246TLS1.2.txt.pdf> 6.2.3.2.  CBC Block Cipher  
*             the content length (TLSCompressed.length) is 61 bytes, and the MAC length is 20 bytes, then the length before padding is 82 bytes
              uint8 padding[GenericBlockCipher.padding_length];
              uint8 padding_length;
*
*@param      :WORD lgth, 该数据的格式为Type + Version + len + data[0,...,len-1] + mac[0,...,maclen-1]中去掉前三项的数据的长度
*
*@return     :BYTE paddingLen   返回的长度是指数据域padding[0,...,padding_len-1] + padding_len 的总长度，填充时，实际的填充数据为padding_len-1
*
*@history    :created by xuxiaojun@20180509
*/
BYTE TLSGetPendingLen(WORD length)
{	
	if(G_TLSSecurityParameters.cipherAlgorithm != ALG_NULL)
	{
		return G_TLSSecurityParameters.blockLength - (length%G_TLSSecurityParameters.blockLength); 
	}
	
	return 0;  //对于不需要加密的算法，长度返回为0
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
*@param       :BYTE *keyBuf  : LV 结构
*@param       :BYTE *IVBuf   : V结构，其长度为G_TLSSecurityParameters.blockLength
*@param       :BYTE *dataBuf : 需要被执行加密/解密操作的原数据，同时也是存放输出结果的数组
*@param       :WORD dataLgth : dataBuf中有效数据的长度，是block的整数倍
*@param       :BYTE operMode : TLS_EncryMsg_Mode/TLS_DecryMsg_Mode
*@param       :BYTE blockMode：TLS_ONLY_BLOCK/TLS_LAST_BLOCK/TLS_FIRST_BLOCK， 按照目前的实现方案，这个参数没有用到
*
*@return      :void
*
*@history     :created by xuxiaojun@20180511
*             
*@Note        :该函数在被调用时，需要考虑IV的变化. 该函数支持的算法在TLSAnalyzeCipherSuit中确定
*/
void TLSEncryDecryMsg(BYTE *keyBuf, BYTE *IVBuf,  BYTE *dataBuf,WORD dataLgth, BYTE operMode, BYTE blockMode)
{
	if((dataLgth%G_TLSSecurityParameters.blockLength)!=0)
	{
		return;
	}
	switch(G_TLSSecurityParameters.cipherAlgorithm)
	{
		case ALG_DES_CBC:	                  // 这种情况下，密钥长度为enckeyLength=24， blockLength=8 字节数
		{
			if(operMode == TLS_EncryMsg_Mode)
			{
				operMode = CBC_Init_Encrypt;  //这里将TLS层的算法宏转换成底层DES的算法宏
			}
			
			if(operMode == TLS_DecryMsg_Mode)
			{
				operMode = CBC_Init_Decrypt;       //这里将TLS层的算法宏转换成底层AES的算法宏
			}
			
			DESCBC(dataBuf, dataLgth, &keyBuf[1], keyBuf[0], IVBuf, G_TLSSecurityParameters.blockLength, operMode, dataBuf);
			
			break;
		}
					
		case ALG_AES_CBC:                     // 这种情况下，密钥长度为enckeyLength = 16; blockLength = 16 字节数
		{
			if(operMode == TLS_EncryMsg_Mode)
			{
				operMode = MODE_ENCRYPT;      //这里将TLS层的算法宏转换成底层AES的算法宏
			}
			
			if(operMode == TLS_DecryMsg_Mode)
			{
				operMode = MODE_DECRYPT;      //这里将TLS层的算法宏转换成底层AES的算法宏
			}
			
			//AESCBC(dataBuf, dataLgth, &keyBuf[1], keyBuf[0], IVBuf, G_TLSSecurityParameters.blockLength, operMode, dataBuf);  //对于dataLen长度超过一个block时，该函数内部会进行处理，所以这里不需要考虑循环
			SymCipherDofinal(MEMtPM8(dataBuf), dataLgth, MEMtPM8(dataBuf), MEMtPM8(&keyBuf[1]), U8_M_U8(keyBuf[0],8), MEMtPM8(IVBuf), NULL, ALG_AES_BLOCK_128_CBC_NOPAD, U8_OR_U8(SYM_CBC_MODE, operMode), 0x00, ALG_AES);

			return;
		}
		default:
			return;
	}
}

/*
*@function   : DESCBC
*
*@description: CBC模式的DES 
*
*@param      : BYTE *in              输入（明文|密文） 
*              BYTE inlen            输入的长度							 
*              BYTE *key             密钥
*              BYTE keylen           密钥的长度	
*              BYTE *iv              初始向量
*              BYTE ivlen            初始向量的长度
*              BYTE chipherMode      加密：MODE_ENCRYPT 解密：MODE_DECRYPT
*              BYTE *out             输出（密文|明文）
*@return     : BYTE                  0 失败 其他：成功，输出的长度
*
*@history    : created by xuxiaojun@20180512

*/
WORD DESCBC(BYTE *in, WORD inlen, BYTE *key, BYTE keylen, BYTE *iv, BYTE ivlen, BYTE chipherMode, BYTE *out) 
{
	if(ivlen != 8)
	{
		return 0;
	}
	
	if(((inlen & 0x07) != 0x00) || (inlen == 0x00))		// 长度不是 8 的整数倍
	{
		return 0;
	}

	chipherMode = U8_OR_U8(chipherMode, CBC_Mode);

	if(0x08 == keylen)
	{
		chipherMode = U8_OR_U8(chipherMode, 0x40);
	}
	else if(0x18 == keylen)
	{
		chipherMode = U8_OR_U8(chipherMode, 0x80);
	}

	Triple_DES(MEMtPX8(in), MEMtPM8(key), MEMtPX8(out), inlen, MEMtPM8(iv), chipherMode, SENSITIVE_DATA_MASKED);
	
	return inlen;
}




/*
*@description:https握手过程中的握手数据计算hash
*@See
*@function   :HttpsHandShakeHashUpdata
*
*@param      :BYTE *hashShakeBuf	本次计算hash的数据
*@param      :BYTE hashShakeBufLen  本次计算hash的数据的长度
*@param      :BYTE blockNum			计算hash的模式， 目前支持 sha1 和 sha256
*@param      :BYTE *outBuf			输出buf
*
*@return     :计算得到的hash组合的长度
*
*@history    :creat by Jiangqb@20200721
*			 :modify by jiangqb@20211129-从epx2.1移植到native平台, 由于native 平台底层不支持动态hash256算法，即调用 SHA256Final是会清空算法内部缓存，无法多次SHA256Final，
*			所以修改为开启开关GP__AMENDMENT__B__HAND_SHAKE_MSG_IN_FILE把 握手数据全部写入 HTTP_CommandData_ADDR_START缓存, 并复用G_HTTPSStatus.HttpChunkYetReadLen 表示保存的握手数据总长度。
*/
BYTE HttpsHandShakeHashUpdata(BYTE *hashShakeBuf, BYTE hashShakeBufLen, BYTE blockNum, BYTE *outBuf)
{
#if !GP__AMENDMENT__B__HAND_SHAKE_MSG_IN_FILE	
	BYTE xdata HashMD5Buf[MD_ICV_SIZE+2];
	BYTE xdata HashMD5RemainDataBuf[MD_BLOCK_SIZE+1];
	BYTE xdata HashSha256Buf[SHA256_ICV_SIZE+2];
	BYTE xdata HashSha256RemainDataBuf[SHA256_BLOCK_SIZE+1];
#endif
	
	if (blockNum == TLS_FIRST_BLOCK)	//clientHello
	{
#if GP__AMENDMENT__B__HAND_SHAKE_MSG_IN_FILE	
		G_HTTPSStatus.HttpChunkYetReadLen = TLS_MaterSecret_Len;//openchannel后G_HTTPSStatus.HttpChunkYetReadLen清零, 偏移TLS_MaterSecret_Len是因为前面的长度TLS_MaterSecret_Len可能用于保存materkey	
		
		WriteEeprom((UWORD)(HTTP_CommandData_ADDR_START+G_HTTPSStatus.HttpChunkYetReadLen), hashShakeBuf, hashShakeBufLen);
			
		G_HTTPSStatus.HttpChunkYetReadLen += hashShakeBufLen;	
#else
		//临时使用RecData， 因为clienthello的数据还不知道用sha1还是sha256，数据在clienthello的时候存储在了G_HandShakeHashRemainDataBuf中
		memcpy(RecData, G_HandShakeHashSha256RemainDataBuf, G_HandShakeHashSha256RemainDataBuf[0]+1);

		if(G_TLSSecurityParameters.tlsProtcolVersion < TLS_Version12)
		{
			HASHUpdate(NULL, 0, HASH_INIT, G_HandShakeHashMD5Buf, G_HandShakeHashMD5RemainDataBuf, ALG_MD);
			HASHUpdate(NULL, 0, HASH_INIT, G_HandShakeHashSha256Buf, G_HandShakeHashSha256RemainDataBuf, ALG_SHA_1);

			HASHUpdate(RecData+1, RecData[0], HASH_UPDATE, G_HandShakeHashMD5Buf, G_HandShakeHashMD5RemainDataBuf, ALG_MD);
			HASHUpdate(RecData+1, RecData[0], HASH_UPDATE, G_HandShakeHashSha256Buf, G_HandShakeHashSha256RemainDataBuf, ALG_SHA_1);
		}
		else
		{
			HASHUpdate(NULL, 0, HASH_INIT, G_HandShakeHashSha256Buf, G_HandShakeHashSha256RemainDataBuf, ALG_SHA_256);

			HASHUpdate(RecData+1, RecData[0], HASH_UPDATE, G_HandShakeHashSha256Buf, G_HandShakeHashSha256RemainDataBuf, ALG_SHA_256);
		}
#endif	
	}
	else if (blockNum == TLS_OTHER_BLOCK)	//中间块
	{
#if GP__AMENDMENT__B__HAND_SHAKE_MSG_IN_FILE	
		WriteEeprom((UWORD)(HTTP_CommandData_ADDR_START+G_HTTPSStatus.HttpChunkYetReadLen), hashShakeBuf, hashShakeBufLen);
			
		G_HTTPSStatus.HttpChunkYetReadLen += hashShakeBufLen;
#else	
		if(G_TLSSecurityParameters.tlsProtcolVersion < TLS_Version12)
		{
			HASHUpdate(hashShakeBuf, hashShakeBufLen, HASH_UPDATE, G_HandShakeHashMD5Buf, G_HandShakeHashMD5RemainDataBuf, ALG_MD);
			HASHUpdate(hashShakeBuf, hashShakeBufLen, HASH_UPDATE, G_HandShakeHashSha256Buf, G_HandShakeHashSha256RemainDataBuf, ALG_SHA_1);
		}
		else			
		{
			HASHUpdate(hashShakeBuf, hashShakeBufLen, HASH_UPDATE, G_HandShakeHashSha256Buf, G_HandShakeHashSha256RemainDataBuf, ALG_SHA_256);	
		}
#endif
	}
	else if (blockNum == TLS_LAST_BLOCK)	//	clientFinish/serverFinish
	{
#if GP__AMENDMENT__B__HAND_SHAKE_MSG_IN_FILE	
		SHA256DoFinalSingleCall(outBuf, NULL, G_HTTPSStatus.HttpChunkYetReadLen-TLS_MaterSecret_Len, 1, (UWORD)(HTTP_CommandData_ADDR_START+TLS_MaterSecret_Len));

		return HMAC_DIGEST_LENGTH_SHA256;
#else
		memcpy(HashSha256Buf, G_HandShakeHashSha256Buf, sizeof(HashSha256Buf));
		memcpy(HashSha256RemainDataBuf, G_HandShakeHashSha256RemainDataBuf, sizeof(HashSha256RemainDataBuf));

		if(G_TLSSecurityParameters.tlsProtcolVersion < TLS_Version12)
		{
			memcpy(HashMD5Buf, G_HandShakeHashMD5Buf, sizeof(HashMD5Buf));
			memcpy(HashMD5RemainDataBuf, G_HandShakeHashMD5RemainDataBuf, sizeof(HashMD5RemainDataBuf));

			HASHDoFinal(NULL, 0, outBuf, HASH_UPDATE, HashMD5Buf, HashMD5RemainDataBuf, ALG_MD);
			HASHDoFinal(NULL, 0, outBuf+MD_DIGEST_SIZE, HASH_UPDATE, HashSha256Buf, HashSha256RemainDataBuf, ALG_SHA_1);

			return (MD_DIGEST_SIZE+SHA1_DIGEST_SIZE);
		}
		else
		{
			HASHDoFinal(NULL, 0, outBuf, HASH_UPDATE, HashSha256Buf, HashSha256RemainDataBuf, ALG_SHA_256);		
			
			return SHA256_DIGEST_SIZE;
		}
#endif
	}
	else if (blockNum == TLS_ONLY_BLOCK)	//serverFinish
	{
#if GP__AMENDMENT__B__HAND_SHAKE_MSG_IN_FILE	
		SHA256DoFinalSingleCall(outBuf, NULL, G_HTTPSStatus.HttpChunkYetReadLen-TLS_MaterSecret_Len, 1, (UWORD)(HTTP_CommandData_ADDR_START+TLS_MaterSecret_Len));
		
		return SHA256_DIGEST_SIZE;
#else
		if(G_TLSSecurityParameters.tlsProtcolVersion < TLS_Version12)
		{
			HASHDoFinal(NULL, 0, outBuf, HASH_UPDATE, G_HandShakeHashMD5Buf, G_HandShakeHashMD5RemainDataBuf, ALG_MD);
			HASHDoFinal(NULL, 0, outBuf+MD_DIGEST_SIZE, HASH_UPDATE, G_HandShakeHashSha256Buf, G_HandShakeHashSha256RemainDataBuf, ALG_SHA_1);

			return (MD_DIGEST_SIZE+SHA1_DIGEST_SIZE);
		}
		else
		{
			HASHDoFinal(NULL, 0, outBuf, HASH_UPDATE, G_HandShakeHashSha256Buf, G_HandShakeHashSha256RemainDataBuf, ALG_SHA_256);

			return SHA256_DIGEST_SIZE;
		}
#endif
	}
	else
	{
		return 0;
	}
	return 0;
}
#endif

