
/************************************************************************************************************
* Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   TlsMainFlow.c
*
* Description: 该文件主要用于存放TLS 层算法相关的函数
*
* @see        :<rfc5246.txt.pdf>
*
* auther：     xxxx    xxxx.xx.xx
*
* history：    V1.0 :create @20180411 ,该文档主要存放用于处理TLS的主流程
				    
*
************************************************************************************************************/
#include "Scp81Connection.h"
#include "TlsMainFlow.h"
#include "FileAction.h"
#include "fileSystem.h"
#include "TlsAlgorithm.h"
#include "HttpProcess.h"
#include "Interface_AnnexB.h"
//#include "ToolkitRFM.h"
//#include "ToolkitApiPublic.h"
//#include "StkCatRuntimeEnviorment.h"
#include "BIPModuleProcess.h"
#include "Gsmpublic.h"
#include "ExpandedManage.h"
#include "CompactManage.h"
//#include "platform_api.h"
#include "JcvmConfig.h"
//#include "uicc_interface.h"
//#include "uicc_M2M.h"
#include "Interface_LTEFromBoot.h"
#include "stkhead.h"
#include "ToolkitRFM.h"
#if  COS_UPDATE_ONLINE_BY_NativeOS
#include "UpCos.h"
#endif

#if GP__AMENDMENT__B__FOR_MASK
TLSSecurityParameters xdata G_TLSSecurityParameters;

#if (!GP__AMENDMENT__B__TLS_MasterKey_IN_FILE)
BYTE xdata G_TLSMaterSecretBuf[TLS_MaterSecret_Len];
#endif

#if (!GP__AMENDMENT__B__TLS_SessionKey_IN_FILE)
BYTE xdata G_TLSSessionKeyBuf[TLS_SESSION_KEY_LEN]; // TLS_ClientRandom_Len + TLS_ServerRandom_Len
#endif                                              // Client_WirteMacKey + Server_WirteMacKey + Client_WirteKey + Server_WirteKey + Client_IV + Server_IV

#if (!GP__AMENDMENT__B__HAND_SHAKE_MSG_IN_FILE)
BYTE xdata G_HandShakeHashMD5Buf[MD_ICV_SIZE+2];							//当前已update进去的数据计算到的hash,根据hash算法计算方案，前两字节是blockNum，使用时定义数组SHA256_ICV_SIZE+2大小即可，中间计算过程不用做任何赋值，hash计算函数内部会处理
BYTE xdata G_HandShakeHashMD5RemainDataBuf[MD_BLOCK_SIZE+1];				//上一次update数据进去时的剩余的不足一块的数据的剩余长度，
BYTE xdata G_HandShakeHashSha256Buf[SHA256_ICV_SIZE+2];						//当前已update进去的数据计算到的hash,根据hash算法计算方案，前两字节是blockNum，使用时定义数组SHA256_ICV_SIZE+2大小即可，中间计算过程不用做任何赋值，hash计算函数内部会处理
BYTE xdata G_HandShakeHashSha256RemainDataBuf[SHA256_BLOCK_SIZE+1];			//上一次update数据进去时的剩余的不足一块的数据的剩余长度，
#endif

/*********************constant define 执行PRF算法时的一些lable的ASCII编码***************************************/
BYTE code LABEL_KEY_EXPANSION[] ={0x0D,0x6B,0x65,0x79,0x20,0x65,0x78,0x70,0x61,0x6E,0x73,0x69,0x6F,0x6E};  //"key expansion"

BYTE code LABEL_MASTER_SECRET[] ={0x0D,0x6D,0x61,0x73,0x74,0x65,0x72,0x20,0x73,0x65,0x63,0x72,0x65,0x74};  //"master secret" lv结构

BYTE code LABEL_CLIENT_FINISHED[] ={0x0F,0x63,0x6C,0x69,0x65,0x6E,0x74,0x20,0x66,0x69,0x6E,0x69,0x73,0x68,0x65,0x64};  //"client finished"

BYTE code LABEL_SERVER_FINISHED[] ={0x0F,0x73,0x65,0x72,0x76,0x65,0x72,0x20,0x66,0x69,0x6E,0x69,0x73,0x68,0x65,0x64};  //"server finished"

BYTE code Label_Tls_Extand[] = {0x09,0x01,0x00,0x00,0x05,0x00,0x01,0x00,0x01,0x01};	//tls扩展字段	最后一个字节可以使用http私有指令进行配置

/*
*@function   : TLSInitVariable
*
*@description: 初始化TLS相关变量，一般在成功打开通道后调用。后续将进行TLS 的send client hello
*
*@param      : void
*
*@return     : void
*
*@history    : V1.0 :created by xuyunzhen@20180411
*         
*/
void TLSInitVariable(void)
{			
	memset((BYTE *)&G_TLSSecurityParameters, 0x00, sizeof(G_TLSSecurityParameters));

	IF_AnnexB_HTTPRFM(HTTP_RFM_MODE_CLEAR);  //清空 http RFM备份文件
}

/*
*@function   : setTLSSecurityPara
*
*@description: 设置G_TLSSecurityParameters的状态
*
*@param      : BYTE RecordState   状态：取值为TLSSecurityParameters.tlsRecordState的状态变迁
*              BYTE tException    异常：TLS 层的状态字 可以用这个异常来表示TLS层的Alert Value
*              BYTE AlertLever    级别：Alert Protocol
*
*@return     : void
*
*@history    : V1.0 :created by xuyunzhen@20180411
*         
*/
void setTLSSecurityPara(BYTE RecordState,BYTE tException,BYTE AlertLever)
{
	G_TLSSecurityParameters.tlsRecordState = RecordState;
			
	G_TLSSecurityParameters.tlsException = tException;  
	
	G_TLSSecurityParameters.tlsAlertLever = AlertLever;
}

/*
*@function     :TLSPatchTypeAndVersion
*
*@description:  拼装以下结构中的ContentType和ProtocolVersion
*               struct {
*                    ContentType type;
*                    ProtocolVersion version;
*                    uint16 length;
*                    opaque fragment[TLSPlaintext.length];
*                } TLSPlaintext;
*
*@see          :<rfc5246TLS1.2.txt.pdf> 6.2.1.  Fragmentation
*
*@param        :BYTE type,  change_cipher_spec(20), alert(21), handshake(22), application_data(23)
*
*return        :void
*
*@history      :V1.0 :created by xuxiaojun@20201022
*/
void TLSPatchTypeAndVersion(BYTE type, BYTE* buf)
{
	buf[0] = type;		//ContentType type;
			
	SetWord(&buf[1], G_TLSSecurityParameters.tlsProtcolVersion);
}
/*
*@function     :TLSSendClientHelloMsg
*
*@description:  组装client Hello消息，将对应的数据放在BIP响应文件中，通过当前打开的biP通道发送出去,注意，如果要考虑重发的话，第二次的随机数从文件中出
*
*@see          :<rfc5246TLS1.2.txt.pdf> 6.2.1.  Fragmentation、  7.4.  Handshake Protocol、    7.4.1.2.  Client Hello
*
*@param        :void
*
*return        :void
*
*@history      :V1.0 :created by xuyunzhen @20180417
*@history      :V1.1 :modify by xuxiaojun@20201028 为了提高握手性能，握手数据不采用写文件方式，考虑到tempBuf比较大，握手数据一般不会超过该数组大小。
*
*@NOTE         :该函数会占用ResData[0,...]资源，所以使用时需要考虑ResData的大小
*/
//BYTE code G_random[]={0x59,0xA4,0x6D,0x1B,0xBE,0x40,0x30,0x8B,0x45,0x05,0x57,0xCA,0xC3,0x59,0x96,0x2F,0xFD,0xF9,0xA2,0x7D,0x72,0x62,0x51,0x86,0x5E,0xD4,0x3A,0x7D,0xB5,0x69,0xD8,0x50};  //AID设置成印度rjio测试卡需求
BYTE TLSSendClientHelloMsg(void)
{
	WORD idata off;
	BYTE idata len;
#if GP__AMENDMENT__B__TLS_SessionKey_IN_FILE
	UWORD xdata fileAddr;
#endif

	memset(tempBuf , 0x00, 0xFF);  	
			
	G_TLSSecurityParameters.tlsProtcolVersion = IF_GetSCP81TLSVersion(G_HTTPSStatus.TriggeredSD);
	
	TLSPatchTypeAndVersion(TLS_HANDSHAKE_PROTOCOL, &tempBuf[2]);
	
	tempBuf[7] = HandshakeType_ClientHello;	 //01  HandshakeType_ClientHello
	
	SetWord(&tempBuf[11], G_TLSSecurityParameters.tlsProtcolVersion);
	
	Random_Number_Generator_Get(&tempBuf[13], TLS_ClientRandom_Len);
	
#if GP__AMENDMENT__B__TLS_SessionKey_IN_FILE
	fileAddr = SelectPrivateTFFile(DF_TLS,EF_TLSKey);

	WriteTFBuf(fileAddr, TLS_ClientRandom_Offset, &tempBuf[13], TLS_ClientRandom_Len);  //将生成的client random缓存起来
#else
	memcpy(G_TLSSessionKeyBuf, &tempBuf[13], TLS_ClientRandom_Len);		//G_TLSSessionKeyBuf保存随机数，不写文件，加快速度
#endif
	
	off = 13 + TLS_ClientRandom_Len + 2; //47

	if (G_TLSSecurityParameters.tlsProtcolVersion == TLS_Version12)
	{
		memcpy(tempBuf+off, Label_Tls0303_CipherSuit, sizeof(Label_Tls0303_CipherSuit));
		
		off += sizeof(Label_Tls0303_CipherSuit);				
	}
	else
	{		
		memcpy(tempBuf+off, Label_Tls0301_CipherSuit, sizeof(Label_Tls0301_CipherSuit));
		
		off += sizeof(Label_Tls0301_CipherSuit);
	}
	
	memcpy(tempBuf+off, Label_Tls_Extand+1, sizeof(Label_Tls_Extand)-1);

	off += sizeof(Label_Tls_Extand)-1;

	tempBuf[off - 1] = JcvmConfig_GetTlsExtandVersion();    //移动联通需要是02， 电信是01
	
	len = (off - 2 - TLS_RecordHead_LENGTH - TLS_HandShakeHead_LENGTH);

	SetWord(&tempBuf[9], len);    //设置Client Hello字段的长度  ，ResData[6-8]存储uint24 length; ResData[6]默认为0 ，目前暂时不支持3字节的长度

	SetWord(&tempBuf[5], len + TLS_HandShakeHead_LENGTH);  //Handshake;消息的总长度  拼装进ResData[3-4]字节中

	SetWord(&tempBuf[0], len+TLS_HandShakeHead_LENGTH+TLS_RecordHead_LENGTH);  //record总长度

#if GP__AMENDMENT__B__HAND_SHAKE_MSG_IN_FILE		
	HttpsHandShakeHashUpdata(tempBuf+7, len+TLS_HandShakeHead_LENGTH, TLS_FIRST_BLOCK, NULL);
#else
	//先把clienthello数据缓存在G_HandShakeHashRemainDataBuf中，因为此时还不知道要采用ALG_SHA_1还是ALG_SHA_256， 目前clientHello数据不会越界G_HandShakeHashRemainDataBuf
	G_HandShakeHashSha256RemainDataBuf[0] = len+TLS_HandShakeHead_LENGTH;

	memcpy(G_HandShakeHashSha256RemainDataBuf+1, tempBuf+7,len+4); //这里需要注意G_HandShakeHashSha256RemainDataBuf是否会越界，这里用一个越界判断没有实际意义，因为越界之后，该方案已经不适用了
#endif
	return TRUE;
}

/*
*@function     :TLSSendClientKeyExchangeMsg
*
*@description:  组装ClientKeyExchange 消息，将对应的数据放在BIP 响应文件中，通过当前打开的biP通道发送出去,注意，如果要考虑重发的话，第二次的随机数从文件中出
*
*@see          :<rfc4279-PSK.txt.pdf>  psk_identity<0..2^16-1> 所以，固定了psk_identity的长度为2字节
*
*@param        :void
*
*return        :WORD  返回当前Client KeyExchange写入EF_BIPModule_Send文件中的有效数据个数
*
*@history      :V1.0 :created by xuyunzhen@20180417
*              :V1.1 :modify by xuxiaojun@20180530
*@NOTE         :由于需求不同，目前对于psk_identity数据的处理方式是，把85 tag下的所有数据全部一齐拼装，而没有对85 tag进行解析得到pskid。因为海外需求、电信需求、ESIM需求对于85 tag的定义都可能不一样，而且测试时，并没有对这串数据进行比较
*/
WORD TLSSendClientKeyExchangeMsg(WORD offSet)
{
	BYTE idata clientKeyExchangeBoyLen;
	WORD idata pskidLen, off;
	UWORD xdata triggerParaFileAddr;
 
	memset(tempBuf , 0x00, 0xFF); 
	
	triggerParaFileAddr = SelectPrivateLFFile(DF_TLS,EF_HTTPTriggerPara);  //A100/9106存储触发参数
	
	TLSPatchTypeAndVersion(TLS_HANDSHAKE_PROTOCOL, &tempBuf[2]);
	
	tempBuf[7] = HandshakeType_ClientKeyExchange;		    //16 client_key_exchange
	
	off = 11;
	
	pskidLen = 0;   //初始化为0,读1字节时可以避免出现大小端问题
	
	ReadLFBuf(triggerParaFileAddr, HTTP_TriggerPara_SecurityPara_Index, 2, (BYTE*)&clientKeyExchangeBoyLen, 1); //Security parameters tag(85) + Length + Length of PSK Identity + PSK Identity

	pskidLen = clientKeyExchangeBoyLen;   //这里借用clientKeyExchangeBoyLen是因为如果直接把1字节赋值给pskidLen时，假设当前值为0x4A，则pskidLen的值是0x4Axx	

	SetWord(&tempBuf[off], pskidLen);
		
	off += 2;                                              //pskid的V域
		
	ReadLFBuf(triggerParaFileAddr, HTTP_TriggerPara_SecurityPara_Index, 3, (BYTE*)&tempBuf[off], pskidLen); //lv结构，前2个字节是l
	
	clientKeyExchangeBoyLen = (2+pskidLen); 
	
	off += pskidLen;
	
	SetWord(&tempBuf[2+TLS_RecordHead_LENGTH+2], clientKeyExchangeBoyLen);  //设置Client Hello字段的长度  ，ResData[6-8]存储uint24 length; ResData[6]默认为0 ，目前暂时不支持3字节的长度
	
	SetWord(&tempBuf[2+3], clientKeyExchangeBoyLen+4);  //Handshake;消息的总长度  拼装进ResData[3-4]字节中
	
	SetWord(tempBuf, off-2);
	
	HttpsHandShakeHashUpdata(tempBuf+2+TLS_RecordHead_LENGTH, clientKeyExchangeBoyLen+4, TLS_OTHER_BLOCK, NULL);
	
	return off;
}
/*
*@function     :TLSClientUpMsgExistIV
*
*@description  :判断TLS的上行报文中,是否存在IV向量，判断向量是否存在，首先要根据版本号，再根据算法，再看changecipher的值

*@              根据6.2.3.2. CBC block cipher ,tls1.0 是不带IV 向量的 -> With block ciphers in CBC mode (Cipher Block Chaining) the initialization vector (IV) for the first record is generated with the other keys and secrets when the security parameters are set. The IV for subsequent records is the last ciphertext block from the previous record.
                                               TLS1.1 TLS1.2 中的 IV 是随机数
*
*@see          :<rfc5246TLS1.2.txt.pdf> 6.2.3.2 CBC Block Cipher    The Initialization Vector (IV) SHOULD be chosen at random, and MUST be unpredictable.    
*               <rfc4346TLS1.1.txt.pdf> 6.2.3.2 CBC Block Cipher    TLS 1.1 uses an explicit IV in order to prevent the attacks
*               <rfc2246TLS1.0.txt.pdf> 6.2.3.2 CBC Block Cipher    没有IV  
*

*
*@param        :void
*
*@return       :TRUE/FALSE
*
*@history      :created by xuxiaojun@20180508
*/
BYTE TLSClientUpMsgGnerateIV(void)
{
	if(G_TLSSecurityParameters.tlsProtcolVersion == TLS_Version10) 
	{
		return FALSE;
	}
	
	if(G_TLSSecurityParameters.cipherAlgorithm == ALG_NULL)
	{
		return FALSE;
	}
	
	if(G_TLSSecurityParameters.clientChangeCipher != TRUE)
	{
		return FALSE;
	}
	
	return TRUE;
}

/*
*@function     :TLSServerDownMsgExistIV
*
*@description:  判断TLS的下行报文中,是否存在IV向量，判断向量是否存在，前提是已经进行了changespec的协商、当前已经是加密算法
*@              根据6.2.3.2. CBC block cipher ,tls1.0 是不带IV 向量的 -> With block ciphers in CBC mode (Cipher Block Chaining) the initialization vector (IV) for the first record is generated with the other keys and secrets when the security parameters are set. The IV for subsequent records is the last ciphertext block from the previous record.
                                               TLS1.1 TLS1.2 中的 IV 是随机数
*
*@see          :<rfc5246TLS1.2.txt.pdf> 6.2.3.2 CBC Block Cipher    The Initialization Vector (IV) SHOULD be chosen at random, and MUST be unpredictable.    
*               <rfc4346TLS1.1.txt.pdf> 6.2.3.2 CBC Block Cipher    TLS 1.1 uses an explicit IV in order to prevent the attacks
*               <rfc2246TLS1.0.txt.pdf> 6.2.3.2 CBC Block Cipher    没有IV 
*
*@param         :void
*
*@return       :TRUE/FALSE
*
*@history      :created by xuxiaojun@20180508
*/
BYTE TLSServerDownMsgExistIV(void)
{
	if(G_TLSSecurityParameters.tlsProtcolVersion == TLS_Version10) 
	{
		return FALSE;
	}
	
	return TRUE;
}

/*
*@function     :TLSSendAlertMsg
*
*@description:  组装Alert消息，将对应的数据放在BIP响应文件中，通过当前打开的biP通道发送出去
*
*@see          :
*
*@param        :void
*
*return        :void
*
*@history      :V1.0 :created by xuyunzhen@20180509
*               V1.1 :modify by xuxiaojun@20180625  如果是握手成功之前发送的alert不需要加密；如果是握手成功之后的alert需要加密 <印尼Reliance电信AmendmentB测试脚本-20180614/RealNetTest_15> 通过G_TLSSecurityParameters.serverSeqNum==0来判断握手没有成功
*
*@NOTE         :该函数会占用ResData[0,...]资源，所以使用时需要考虑ResData的大小
*               该函数会借用RecData[0,...]资源，RecData[0,...,33]存放加密KEY; RecData[40,...,56]存放IV，目前IV最长为16字节
*/
void TLSSendAlertMsg(void)
{
	BYTE idata ivLength,paddingLen;
	WORD idata off;
	BYTE xdata HashWorkSpace[HASH_BLOCK_SIZE + HASH_BLOCK_SIZE + HASH_ICV_SIZE + 8];

	TLSPatchTypeAndVersion(TLS_ALERT_PROTOCOL, &ResData[0]);
	
	ResData[3] = 0x00;
	
	ResData[4] = 0x02;                                                  //alert的明文数据长度是2
	
	off = 5;
	
	ivLength = 0x00;
	
	if((G_TLSSecurityParameters.clientChangeCipher == 1) && (G_TLSSecurityParameters.serverChangeCipher == 1))                       //用clientSeqNum表示是否发送了clientFinish，用例中用clientFinish发送之后就发送密文alert，在此之前发送明文alert  <印尼Reliance电信AmendmentB测试脚本-20180614/RealNetTest_15>期望发送明文， <印尼Reliance电信AmendmentB测试脚本-20180614/RealNetTest_16>期望发送密文
	{
		if(TLSClientUpMsgGnerateIV() == TRUE) 
		{
			ivLength = G_TLSSecurityParameters.blockLength;
			
			Random_Number_Generator_Get(&ResData[off], ivLength);      //产生IV 向量
		}
	}
	
	off += ivLength;
	
	ResData[off] = G_TLSSecurityParameters.tlsAlertLever;        //alert lever
	
	off++;
	
	ResData[off] = G_TLSSecurityParameters.tlsException;         //alert value
	
	off++;                                                       //偏移到mac位置
	
	if((G_TLSSecurityParameters.clientChangeCipher == 1) && (G_TLSSecurityParameters.serverChangeCipher == 1))               //用clientSeqNum表示是否发送了clientFinish，用例中用clientFinish发送之后就发送密文alert，在此之前发送明文alert  <印尼Reliance电信AmendmentB测试脚本-20180614/RealNetTest_15>期望发送明文， <印尼Reliance电信AmendmentB测试脚本-20180614/RealNetTest_16>期望发送密文
	{
		if(G_TLSSecurityParameters.macAlgorithm != ALG_HMAC_NULL)
		{
			TLSGetSessionKey(Client_WirteMacKey_Value_Off, &RecData[0]); //获取MAC_write_key 存放在RecData[0]开始的位置 ，LV结构
		
			TLSCalMsgMAC(ClientUpMsg_Mode,TLS_ONLY_BLOCK,&RecData[0],&ResData[0],off,ivLength,&ResData[off], HashWorkSpace); //IV不参与MAC计算
			
			off += G_TLSSecurityParameters.macLength;                    //point to the off of the uint8 padding[GenericBlockCipher.padding_length]
		
			paddingLen = TLSGetPendingLen(off - TLS_RecordHead_LENGTH - ivLength);   //补位的长度, paddingLen包括1字节的padding_length
		
			memset(&ResData[off], paddingLen-1, paddingLen);
			
			off += paddingLen;   //偏移到GenericBlockCipher结构体的最末尾

			TLSGetSessionKey(Client_WirteKey_Value_Off, &RecData[0]);	//获取加密 key ： LV 结构	
			
			if(G_TLSSecurityParameters.tlsProtcolVersion == TLS_Version10) //获取初始向量, <rfc2246_TLS1.0.txt.pdf> initialization vector (IV) for the first record is generated with the other keys and secrets when the security parameters are set. The IV for subsequent records is the last ciphertext block from the previous record.也就是函数TLSKeyBlockCalculation生成的IV
			{
				TLSGetSessionKey(Client_IV_Value_Off, &RecData[0x40]);  
				
				memmove(&RecData[0x40], &RecData[0x41], RecData[0x40]);
			}
			else
			{
				memcpy(&RecData[0x40], &ResData[5], G_TLSSecurityParameters.blockLength); 
			}
		
			if (G_TLSSecurityParameters.cipherAlgorithm != ALG_NULL)
			{
				TLSEncryDecryMsg(&RecData[0], &RecData[0x40],&ResData[TLS_RecordHead_LENGTH+ivLength],off-TLS_RecordHead_LENGTH-ivLength,TLS_EncryMsg_Mode,TLS_ONLY_BLOCK);  //从content开始加密，加密数据包括CONTENT ,MAC,PAdding
			}
			
//			if(G_TLSSecurityParameters.tlsProtcolVersion == TLS_Version10) //对于TLS 1.0 ，把IV 向量保存起来供下次加解密使用
//			{
//				TLSWriteSessionKey(Client_IV_Value_Off,&ResData[off - G_TLSSecurityParameters.blockLength]);
//			}
		}
		
		G_TLSSecurityParameters.clientSeqNum ++;   //客户端序列号加1
	}
	
	SetWord(&ResData[3], off - TLS_RecordHead_LENGTH);  //设置长度
	
//	fileAddr = SelectPrivateTFFile(DF_BIPModule,EF_BIPModule_Send);
// 	fileAddr = BIPModule_Send_ADDR_START;

//	memmove(&ResData[2],&ResData[0],off);
		
//	WriteTFFileWord(fileAddr, off);
	G_BIPModule_Send_Length = off;
	
//	WriteTFBuf(fileAddr, 2, ResData+2, off); 
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
	ecpMem_Copy(BIPModule_Send_ADDR_START+2, ResData,off); // 警告的长度，不会越界
#else
	WriteEepromWithoutProtect((UWORD)(BIPModule_Send_ADDR_START+2), ResData,off); // 警告的长度，不会越界
#endif
}


/*
*@function     :TLSSendClientFinishedMsg
*
*@description:  组装ClientFinished 消息，将对应的数据放在BIP 响应文件中，通过当前打开的biP通道发送出去,注意，如果要考虑重发的话，第二次的随机数从文件中出
				The Finished message is the first one protected with the just negotiated algorithms, keys, and secrets
*
*@see          :
*
*@param        :WORD offSet  当前需要写入EF_BIPModule_Send文件的有效起始偏移
*
*return        :void
*
*@history      :V1.0 :created by xuyunzhen@20180509
*
*@NOTE         :该函数会占用ResData[0,...]资源，所以使用时需要考虑ResData的大小
*               该函数会借用RecData[0,...]资源，RecData[0,...,33]存放加密KEY; RecData[40,...,56]存放IV，目前IV最长为16字节
*/
WORD TLSSendClientFinishedMsg(WORD offSet)
{
	WORD idata off;
	BYTE idata ivLength,paddingLen;
	BYTE xdata HashWorkSpace[HASH_BLOCK_SIZE + HASH_BLOCK_SIZE + HASH_ICV_SIZE + 8];

	memset(&ResData[0] , 0x00, sizeof(ResData));  
	
	TLSPatchTypeAndVersion(TLS_HANDSHAKE_PROTOCOL, &ResData[2]);
	
	off = 7;
	
	ivLength = 0x00;

	if(TLSClientUpMsgGnerateIV() == TRUE) 
	{
		ivLength = G_TLSSecurityParameters.blockLength;
		
		Random_Number_Generator_Get(&ResData[off], ivLength);   //产生IV 向量
	}
	
	off += ivLength;
	
	ResData[off] = HandshakeType_ClientFinished;	
	
	ResData[off+3] = TLSGetFinishedVerifyData(&ResData[off+4],  (BYTE *)&LABEL_CLIENT_FINISHED[0]); 
	
	HttpsHandShakeHashUpdata(ResData+off, 0x10, TLS_OTHER_BLOCK, NULL);
	
	off += (ResData[off+3] + TLS_HandShakeHead_LENGTH);    //point to the data off of MAC
	
	TLSGetSessionKey(Client_WirteMacKey_Value_Off, &RecData[0]);  //获取MAC_write_key 存放在RecData[0]开始的位置 ，LV结构
	
	ResData[2+TLS_RecordHead_LENGTH-1] = 0x10;   //先设置TLSCompressed.length的值进行mac运算, 0x10 = verify data的长度 + TLS_HandShakeHead_LENGTH
	
	TLSCalMsgMAC(ClientUpMsg_Mode, TLS_ONLY_BLOCK, &RecData[0], &ResData[2], off-2, ivLength, &ResData[off], HashWorkSpace); //IV不参与MAC计算
	
	off += G_TLSSecurityParameters.macLength;    //point to the off of the uint8 padding[GenericBlockCipher.padding_length]
	
	if(G_TLSSecurityParameters.cipherAlgorithm != ALG_NULL)
	{	
		paddingLen = TLSGetPendingLen(off - TLS_RecordHead_LENGTH - ivLength-2);   //补位的长度, paddingLen包括1字节的padding_length
	
		memset(&ResData[off], paddingLen-1, paddingLen);
		
		off += paddingLen;   //偏移到GenericBlockCipher结构体的最末尾

		TLSGetSessionKey(Client_WirteKey_Value_Off, &RecData[0]);	//获取加密 key ： LV 结构	
		
		if(G_TLSSecurityParameters.tlsProtcolVersion == TLS_Version10) //获取初始向量, <rfc2246_TLS1.0.txt.pdf> initialization vector (IV) for the first record is generated with the other keys and secrets when the security parameters are set. The IV for subsequent records is the last ciphertext block from the previous record.也就是函数TLSKeyBlockCalculation生成的IV
		{
			TLSGetSessionKey(Client_IV_Value_Off, &RecData[0x40]);  
			
			memmove(&RecData[0x40], &RecData[0x41], RecData[0x40]);
		}
		else
		{
			memcpy(&RecData[0x40], &ResData[2+5], G_TLSSecurityParameters.blockLength); 
		}
	
		TLSEncryDecryMsg(&RecData[0], &RecData[0x40],&ResData[2+TLS_RecordHead_LENGTH+ivLength],off-TLS_RecordHead_LENGTH-ivLength-2,TLS_EncryMsg_Mode,TLS_ONLY_BLOCK);  //从content开始加密，加密数据包括CONTENT ,MAC,PAdding
			
		if (G_TLSSecurityParameters.tlsProtcolVersion == TLS_Version10) //对于TLS 1.0 ，把IV 向量保存起来供下次加解密使用
		{
			TLSWriteSessionKey(Client_IV_Value_Off, ResData+off-G_TLSSecurityParameters.blockLength);
		}
	}
	
	G_TLSSecurityParameters.clientSeqNum++;   //客户端序列号加1

	SetWord(ResData+2+3, off - TLS_RecordHead_LENGTH-2);	 //设置长度
	
	SetWord(ResData, off-2);	 //设置长度
	
	memcpy(tempBuf, ResData, off);
	
	return off;
}
/*
*@description:  组装TLSSendChangeCipherSpecMsg 消息，将对应的数据放在BIP 响应文件中，通过当前打开的biP通道发送出去,注意，如果要考虑重发的话，第二次的随机数从文件中出
*
*@see          :
*
*@function     :TLSSendChangeCipherSpecMsg
*
*@param        :WORD offSet  当前需要写入EF_BIPModule_Send的有效起始位置
*
*return        :WORD  返回当前Change Cipher写入EF_BIPModule_Send中的有效数据的长度
*
*@author       :xuyz
*
*@history      :V1.0 :create @
*/
WORD TLSSendChangeCipherSpecMsg(WORD offSet)
{
	BYTE idata off;
		
	TLSCalMasterSecret();   //在changecipher之后得出master
	
	TLSKeyBlockCalculation(); //在changecipher之后分散出密钥，因为Finished命令需要使用加密参数
	
	G_TLSSecurityParameters.clientChangeCipher = TRUE;	
		
	TLSPatchTypeAndVersion(TLS_CHANGE_CIPHER_SPECS_PROTOCOL, &tempBuf[2]);
	
	off = 5;
	
	tempBuf[off++] = 0x00;
	
	tempBuf[off++] = 0x01;   //uint16 length of TLSPlaintext,  长度为1
		
	tempBuf[off++] = 0x01;		//enum { change_cipher_spec(1), (255) } type
	
	SetWord(tempBuf, off-2);

	return off;
}
/*
*@function     :TLSAnalyzeCipherSuit
*
*@description  :对serverhello中的ciphersuits进行分析，并对G_TLSSecurityParameters 中涉及算法相关的全局变量进行赋值操作
*
*@see          :<rfc5246TLS1.2.txt.pdf> A.5 The Cipher Suite     Page 84
*
*@param        :WORD cipherSuite   ： 服务器下发的changecipher里的值
*
*@return       :void
*
*@shitory      :V1.0 :created by xuyunzhen @20180417
*/
void TLSAnalyzeCipherSuit(WORD cipherSuite)
{	
	switch (cipherSuite)
	{
		case TLS_PSK_WITH_NULL_SHA:				//002C
			
			G_TLSSecurityParameters.cipherAlgorithm = ALG_NULL;
		
			G_TLSSecurityParameters.enckeyLength = 0;
		
		    G_TLSSecurityParameters.blockLength = 0;
		
			G_TLSSecurityParameters.macAlgorithm = ALG_HMAC_SHA1;
		
			G_TLSSecurityParameters.macKeyLength = SHA1_DIGEST_SIZE;
		
			G_TLSSecurityParameters.macLength = SHA1_DIGEST_SIZE;
		
			break;
		
		case TLS_PSK_WITH_3DES_EDE_CBC_SHA:       //008B   //<rfc5246TLS1.2.txt.pdf> Page 84
			
			G_TLSSecurityParameters.cipherAlgorithm = ALG_DES_CBC;
			
			G_TLSSecurityParameters.enckeyLength = 24;
			
			G_TLSSecurityParameters.blockLength = 8;
		
			G_TLSSecurityParameters.macAlgorithm = ALG_HMAC_SHA1;
		
			G_TLSSecurityParameters.macKeyLength = SHA1_DIGEST_SIZE;
		
			G_TLSSecurityParameters.macLength = SHA1_DIGEST_SIZE;
			
			break;
		
		case TLS_PSK_WITH_AES_128_CBC_SHA: 		//008C
			
			G_TLSSecurityParameters.cipherAlgorithm = ALG_AES_CBC;

			G_TLSSecurityParameters.enckeyLength = 16;
			
			G_TLSSecurityParameters.blockLength = 16;
		
			G_TLSSecurityParameters.macAlgorithm = ALG_HMAC_SHA1;
		
			G_TLSSecurityParameters.macKeyLength = SHA1_DIGEST_SIZE;
		
			G_TLSSecurityParameters.macLength = SHA1_DIGEST_SIZE;
			
			break;
			
		case TLS_PSK_WITH_NULL_SHA256: 		//00B0
			
			G_TLSSecurityParameters.cipherAlgorithm = ALG_NULL;
			
			G_TLSSecurityParameters.enckeyLength = 0;
		
		    G_TLSSecurityParameters.blockLength = 0;
			
			G_TLSSecurityParameters.macAlgorithm = ALG_HMAC_SHA_256;
		
			G_TLSSecurityParameters.macKeyLength = SHA256_DIGEST_SIZE;
		
			G_TLSSecurityParameters.macLength = SHA256_DIGEST_SIZE;
		
			break;
		
		case TLS_PSK_WITH_AES_128_CBC_SHA256:		//00AE
			
			G_TLSSecurityParameters.cipherAlgorithm = ALG_AES_CBC;
			
			G_TLSSecurityParameters.enckeyLength = 16;
		
		    G_TLSSecurityParameters.blockLength = 16;
			
			G_TLSSecurityParameters.macAlgorithm = ALG_HMAC_SHA_256;
		
			G_TLSSecurityParameters.macKeyLength = SHA256_DIGEST_SIZE;
		
			G_TLSSecurityParameters.macLength = SHA256_DIGEST_SIZE;
		
			break;
		
// 		case TLS_PSK_WITH_AES_128_GCM_SHA256:		//00A8
// 			
// 			G_TLSSecurityParameters.cipherAlgorithm = ALG_AES_GCM;
// 			
// 			G_TLSSecurityParameters.enckeyLength = 16;
// 		
// 		    G_TLSSecurityParameters.blockLength = 16;
// 			
// 			G_TLSSecurityParameters.macAlgorithm = ALG_HMAC_SHA_256;
// 		
// 			G_TLSSecurityParameters.macKeyLength = SHA256_DIGEST_SIZE;
// 		
// 			G_TLSSecurityParameters.macLength = SHA256_DIGEST_SIZE;
// 			
// 			break;
		
		default:
			
			G_TLSSecurityParameters.cipherAlgorithm = ALG_NULL;
			
			G_TLSSecurityParameters.enckeyLength = 0;
		
		    G_TLSSecurityParameters.blockLength = 0;
			
			G_TLSSecurityParameters.macAlgorithm = ALG_HMAC_NULL;
		
			G_TLSSecurityParameters.macKeyLength = 0;
		
			G_TLSSecurityParameters.macLength = 0;
		
			break;			
		
	}
	G_TLSSecurityParameters.cipherSuite = cipherSuite;
	return;
	
}
/*
*@description:处理TLS层接收到的ServerHello消息，主要用来协商TLS层的安全参数
*
*@see        :RFC5246.pdf 7.4.1.3.  Server Hello
*
*@function   :TLSProcessServerHello
*
*@param      :BYTE *severHelloBuf    服务器下来的serverHello的数据
*@param      :WORD serverHelloLen    服务器下来的serverHello的数据的长度
*
*@return     :TRUE:数据解析成功/FALSE:数据解析失败
*
@hsitory     :V1.0 created by xuyunzhen@20180412
*/
BYTE TLSProcessServerHello(BYTE *severHelloBuf, WORD serverHelloLen)
{
	WORD idata off;
	BYTE idata cipherLen, cipherIndex;
#if GP__AMENDMENT__B__TLS_SessionKey_IN_FILE
	UWORD xdata fileAddr;
#endif
		
	if(BuildWord(severHelloBuf) > G_TLSSecurityParameters.tlsProtcolVersion)  //下行服务器中的TLS 版本号，大于CLIENT Hello中的SD支持的版本号则报错返回
	{
		setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_PROTOCOL_VERSION, ALERT_LEVER_FATAL);// 0246
		
		return FALSE;
	}
	
	off = 2;
	
#if GP__AMENDMENT__B__TLS_SessionKey_IN_FILE
	fileAddr = SelectPrivateTFFile(DF_TLS,EF_TLSKey);

	WriteTFBuf(fileAddr, TLS_ServerRandom_Offset, &severHelloBuf[off], TLS_ServerRandom_Len);  //将server random缓存起来
#else
	memcpy(G_TLSSessionKeyBuf+TLS_ClientRandom_Len, &severHelloBuf[off], TLS_ServerRandom_Len);		//tempBuf保存随机数，不写文件，加快速度
#endif
	off += TLS_ServerRandom_Len;
	
	off += (1 + severHelloBuf[off]);  //跳过 SessionID session_id; 字段, point to the CipherSuite cipher_suite;
		
	if (G_TLSSecurityParameters.tlsProtcolVersion == TLS_Version12)
	{
		cipherLen = sizeof(Label_Tls0303_CipherSuit)-1;
		
		memcpy(ResData, Label_Tls0303_CipherSuit+1, cipherLen);
	}
	else
	{
		cipherLen = sizeof(Label_Tls0301_CipherSuit)-1;
		
		memcpy(ResData, Label_Tls0301_CipherSuit+1, cipherLen);
	}	

	cipherIndex = 0;
	
	while(cipherIndex < cipherLen)
	{
		if (memcmp(ResData+cipherIndex, &severHelloBuf[off], 2) == EQUAL)
		{
			break;
		}
		
		cipherIndex += 2;
	}
	
	G_TLSSecurityParameters.tlsProtcolVersion = BuildWord(&severHelloBuf[0]);
	
	if(cipherIndex >= cipherLen)  //下行的加密算法卡片不支持
	{
		setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_ILLEGAL_PARAMETER, ALERT_LEVER_FATAL);// 022F
		
		return FALSE;
	}	
		
	TLSAnalyzeCipherSuit(BuildWord(&severHelloBuf[off])); //通过server中的cipher进行mac iv等变量的配置
	
	off += 2;		//point to CompressionMethod;
	
	G_TLSSecurityParameters.TLSEachLength = off;
	
	if (severHelloBuf[off] != 0x00)		//CompressionMethod
	{
		return FALSE;
	}
	
	off++;				//point to expandtion;
	
	off++;				//point to expandtion length
	
	if (severHelloBuf[off] == 0x05)		//目前支持值max_fragment_length(1)的扩展格式，长度(LV)是5字节
	{
		off += 2;
		
		if (severHelloBuf[off] == 0x01)		//目前支持值max_fragment_length(1)的扩展格式
		{
			off += 2;
			
			if (severHelloBuf[off] == 0x01)			//1字节长度
			{
				off ++;
				
				switch (severHelloBuf[off])
				{
					case 1:
						G_TLSSecurityParameters.TLSEachLength = 512;		//2^9;		//
						break;
					
					case 2:
						G_TLSSecurityParameters.TLSEachLength = 1024;		//2^10;
						break;
					
					case 3:	
						G_TLSSecurityParameters.TLSEachLength = 2048;		//2^11;
						break;
					
					default:						
						return FALSE;	
				}
#if (!GP__AMENDMENT__B__HAND_SHAKE_MSG_IN_FILE)
				HttpsHandShakeHashUpdata(NULL, 0, TLS_FIRST_BLOCK, NULL);
#endif
				return TRUE;
			}
		}
	}	

	return FALSE;
}
/*
*@description:将当前期望的tlsRecordState转换成期望的HandshakeType msg_type
*
*@see        :RFC5246.pdf 7.4.  Handshake Protocol
*
*@function   :TLSTransRecordStateToHandShakeType
*
*@param      :BYTE tlsRecordState    当前握手协议中期望达到的状态
*
*@return     :BYTE HandshakeType     当前握手协议中期望达到的状态对应的HandshakeType msg_type
*
*@history    :V1.0 Created by xuyunzhen@20180412
*/
BYTE TLSTransRecordStateToHandShakeType(BYTE tlsRecordState)
{
	BYTE idata handShakeType;
	
	handShakeType = FALSE;
	
	switch(tlsRecordState)
	{
		case TLS_RECEIVE_SERVER_HELLO_STATE:
			
			handShakeType = HandshakeType_ServerHello;
			break;
		
		case TLS_RECEIVE_SERVER_KEY_EXCHANGE_STATE:
		
			handShakeType = HandshakeType_ServerKeyExchange;
			break;
		
		case TLS_RECEIVE_SERVER_HELLO_DONE_STATE:
			
			handShakeType = HandshakeType_ServerHelloDone;
		
			break;
			
		case TLS_RECEIVE_SERVER_FINISHED_STATE:
			
			handShakeType = HandshakeType_ServerFinished;
		
			break;
		
		default: //白盒测试添加
			break;
	}
	
	return handShakeType;
}
/*
*@description: 对serverfinished数据进行解密和验证mac，如果成功，则将verfirydata 移动到buf[0]的位置，同时返回verifydatalen的长度
*
*@function    :TLSVerifyServerFinishedMsg
*
*@param       :BYTE *msgBuf:  输入数据，数据内容包括TLS_RecordHead+ IV +CONTEN+MAC+PADDING，是密文形式
*@param       :BYTE msgLen: LEN(TLS_RecordHead_LENGTH + IV +CONTEN+MAC+PADDING)
*
*@return       :BYTE verifyDataLen 即校验数据的长度
*
*@history     	:creat by jiangqb@20190714	
*				:modify by jiangqb @20191122 去除没必要的用于冗余代码
*				:modify by jiangqb@20200707 根据华哥建议优化代码，不用写文件操作，直接在tempBuf中解密
*/
BYTE TLSVerifyServerFinishedMsg(BYTE *msgBuf, WORD msgLen)
{
	BYTE idata ivLength, paddingLen, contentLen;
	
	//UWORD xdata fileAddr;
	BYTE xdata HashWorkSpace[HASH_BLOCK_SIZE + HASH_BLOCK_SIZE + HASH_ICV_SIZE + 8];
		
	ivLength = 0;
	
	paddingLen = 0;
	
	if(G_TLSSecurityParameters.serverChangeCipher != TRUE) //服务器端未协商，默认明文传输
	{
		if(msgBuf[TLS_RecordHead_LENGTH + ivLength] != HandshakeType_ServerFinished)   //收到的不是server finished命令
		{
			return FALSE;
		}	
	}
	else   //服务器端协商加密模式，后续数据采用加密、mac方式
	{
		if(G_TLSSecurityParameters.cipherAlgorithm != ALG_NULL)
		{
			if(TLSServerDownMsgExistIV() == TRUE)  // 下行报文存在IV向量
			{
				ivLength = G_TLSSecurityParameters.blockLength;
				
				memcpy(&ResData[0xC0], &msgBuf[TLS_RecordHead_LENGTH], ivLength);  //拷贝初始向量，V 结构
			}
			else
			{
				TLSGetSessionKey(Server_IV_Value_Off, &ResData[0xC0]);   //1.0的向量从IV 中获取
				
				memmove(&ResData[0xC0], &ResData[0xC1], ResData[0xC0]);
				
				TLSWriteSessionKey(Server_IV_Value_Off, &msgBuf[msgLen-G_TLSSecurityParameters.blockLength]);  //TLS1.0 :The IV for subsequent records is the last ciphertext block from the previous record
			}
			
			TLSGetSessionKey(Server_WirteKey_Value_Off, &ResData[0x80]);	//获取解密 key ： LV 结构	
		
			TLSEncryDecryMsg(&ResData[0x80], &ResData[0xC0],&msgBuf[TLS_RecordHead_LENGTH+ivLength], msgLen-ivLength-TLS_RecordHead_LENGTH, TLS_DecryMsg_Mode, TLS_ONLY_BLOCK);  //对数据进行解密
			
			paddingLen = msgBuf[msgLen-1]+1;  //获取补位的数据长度
		}
		
		if(msgBuf[TLS_RecordHead_LENGTH + ivLength] != HandshakeType_ServerFinished)   //收到的不是server finished命令
		{
			return FALSE;
		}
		
		TLSGetSessionKey(Server_WirteMacKey_Value_Off, &ResData[0x40]);	//获取mac key ： LV 结构	
		
		contentLen = msgLen - TLS_RecordHead_LENGTH - ivLength- paddingLen - G_TLSSecurityParameters.macLength;

		SetWord(&msgBuf[3], contentLen);  //设置参与mac运算的content 数据长度
		
		TLSCalMsgMAC(ServerDownMsg_Mode, TLS_ONLY_BLOCK, &ResData[0x40], msgBuf, contentLen + ivLength+ TLS_RecordHead_LENGTH, ivLength, &ResData[0xC0], HashWorkSpace);  //0XC0存放计算出来的mac数据
		
		if (memcmp(&msgBuf[msgLen - paddingLen - G_TLSSecurityParameters.macLength], &ResData[0xC0], G_TLSSecurityParameters.macLength) != EQUAL)		//compare mac
		{
			return FALSE;
		}
	}
	
	paddingLen = msgBuf[TLS_RecordHead_LENGTH + ivLength+3];  		//paddingLen 复用
	
	memcpy(ResData, &msgBuf[TLS_RecordHead_LENGTH + ivLength+4], paddingLen);
	
	return paddingLen;
}
/*
*@description :处理TLS 握手阶段的接收到的 tls消息,卡端此时肯定存在一个或多个完整的记录, 本函数只能在握手协议期间使用
*
*@see          :<rfc5246TLS1.2.txt.pdf> 6.2.1.  Fragmentation、  7.4.  Handshake Protocol、    7.4.1.2.  Client Hello
*
*@function    :TLSProcessRecieveHandShakeMsg
*
*@param       :void
*
*@return      :void
*
*@history     	:V1.0 created by xuyunzhen @20180417
*             	:V1.1 modify by xuweihua@20180719  将借用了tempBuf循环写文件的地方用WriteEepromNonAtomic来替换
*				:modify by jiangqb @20191122 去除没必要的用于冗余代码, 修改serverhello和serverhellodown一起来的时候的判断位置的bug
*               :modify by jiangqb @20210609 修改recordLen含义被复用导致的错误，下载电信profile后，HTTPS1.2补充脚本中TLS_PSK_Handshake_06
*/

void TLSProcessRecieveHandShakeMsg(void)
{
	WORD idata curDataOff, recordLen,remainDataLen,handShakeLen;
	BYTE xdata tmp[5],handShakeHead[4], verifyData[12];
	BYTE idata verifyDataLen;
	
	remainDataLen = BuildWord(tempBuf);
	
	curDataOff = 2;  //数据的起始偏移, LLV，从V开始保存是本次收到的bip数据
	
	while(remainDataLen > 0)
	{
		if(remainDataLen > TLS_RecordHead_LENGTH)
		{
			memcpy(tmp, tempBuf+curDataOff, TLS_RecordHead_LENGTH);

			recordLen = BuildWord(&tmp[3]);
		}
		else
		{
			G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;

			return;
		}

		if(remainDataLen < (recordLen + TLS_RecordHead_LENGTH))
		{
			G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;

			return;
		}
		
		if(G_TLSSecurityParameters.tlsRecordState == TLS_RECEIVE_SERVER_FINISHED_STATE)      //将要处理server finished msg, 是密文，单独处理
		{
			if(remainDataLen != (recordLen + TLS_RecordHead_LENGTH))                         //解析到ServerFinshed时后续还有数据,报错，关通道
			{
				G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;  

				return;
			}

			verifyDataLen = TLSVerifyServerFinishedMsg(tempBuf+curDataOff, recordLen + TLS_RecordHead_LENGTH);		//解密
			
			if(verifyDataLen == FALSE)
			{
				G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;                 //在安全参数协商完成之前就收到了changeCiper消息，则关闭通道报错
		
				return;
			}
			else
			{
				TLSGetFinishedVerifyData(verifyData, (BYTE *)&LABEL_SERVER_FINISHED[0]); //计算本次握手信息的verfify data
					
				if(TLS_VerifyData_Len!= verifyDataLen)
				{
					G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;  //<rfc5246TLS1.2.txt.pdf>  It is a fatal error if a Finished message is not preceded by a ChangeCipherSpec message at the appropriate point in the handshake
					
					return;
				}
				
				if(memcmp(verifyData, ResData, TLS_VerifyData_Len) != EQUAL)		//比较 verifyData
				{
					G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;
					
					return;
				}
			}	

			G_TLSSecurityParameters.serverSeqNum++;				//服务器端序列号+1
			
			memset(ResData, 0x00, sizeof(ResData));				//此处清空ResData是因为在拼装上行数据时，ResData有可能作为来源数据
			
			G_TLSSecurityParameters.tlsRecordState  = TLS_SEND_APPLICATION_DATA_STATE; //握手成功之后，发送http第一次请求	
		}
		else
		{				
			switch(tmp[0])  //contentType: change_cipher_spec(20), alert(21), handshake(22), application_data(23), (255)
			{
				case TLS_APPLICATION_DATA_PROTOCOL:  //0x17;	在握手期间出现了应用数据，关闭通道报错
				{
					G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;
					
					return;
				}
				case TLS_CHANGE_CIPHER_SPECS_PROTOCOL: //0x14;	服务器下发ChangeCipherSpec指令必须是在客户端发送了Finished之后，且未收到服务器下发的Finished之前
				{
					if(G_TLSSecurityParameters.tlsRecordState < TLS_SEND_CLIENT_FINISHED_STATE) //The ChangeCipherSpec message is sent during thehandshake after the security parameters have been agreed upon, but before the verifying Finished message is sent
					{
						G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;                 //在安全参数协商完成之前就收到了changeCiper消息，则关闭通道报错
					
						return;
					}
					
					if(recordLen != 0x01)  //<rfc5246TLS1.2.txt.pdf> The message consists of a single byte of value 1
					{
						G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;      //运changeCiper消息错误
					
						return;
					}
					
					G_TLSSecurityParameters.serverChangeCipher = tempBuf[curDataOff + TLS_RecordHead_LENGTH];
											
					G_TLSSecurityParameters.tlsRecordState = TLS_RECEIVE_SERVER_FINISHED_STATE;
					
					break;
				}
				case TLS_ALERT_PROTOCOL:           //0x15;         //在握手期间收到 alert消息上发相同的警告信息(电信备案测试用例：中国电信HTTPS1.2/TLS_Alert_protocol/TLS_PSK_Alert_01/02/03)
				{
					setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, tempBuf[curDataOff+TLS_RecordHead_LENGTH+1], tempBuf[curDataOff+TLS_RecordHead_LENGTH]);

					return;
		
				}
				case TLS_HANDSHAKE_PROTOCOL:  //收到的是握手信息
				{
					memcpy(handShakeHead, tempBuf+2+TLS_RecordHead_LENGTH, TLS_HandShakeHead_LENGTH);
					
					if(handShakeHead[0] == HandshakeType_HelloRequest)  //helloRequest消息忽略， <rfc5246TLS1.2.txt.pdf>  All of the data from all messages in this handshake (not including any HelloRequest messages)
					{
						break;
					}

					//16,14,17,15
					if(handShakeHead[0] != TLSTransRecordStateToHandShakeType(G_TLSSecurityParameters.tlsRecordState))  //收到的信息handShake消息顺序与期望的不对
					{	
						if ((handShakeHead[0] == HandshakeType_ServerHelloDone) && (G_TLSSecurityParameters.tlsRecordState == TLS_RECEIVE_SERVER_KEY_EXCHANGE_STATE))
						{
							//支持当期望是 ServerKeyExchange(0C), 实际上来的数据是 ServerHelloDone(0E)
						}
						else
						{
							G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;  //暂时处理成关闭通道
						
							return;
						}
					}
					
					if(handShakeHead[0] == HandshakeType_ServerHello)
					{			
						if(TLSProcessServerHello(tempBuf+2+TLS_RecordHead_LENGTH+TLS_HandShakeHead_LENGTH, BuildWord(&handShakeHead[2])) == FALSE)  //解析下行的serverhello数据
						{
							G_TLSSecurityParameters.tlsRecordState = TLS_SEND_ALERT_DATA_STATE;  //暂时处理成关闭通道
						
							return;
						}

						G_TLSSecurityParameters.tlsRecordState = TLS_RECEIVE_SERVER_KEY_EXCHANGE_STATE;
						
						handShakeLen = BuildWord(&handShakeHead[2])+TLS_HandShakeHead_LENGTH;		//不包含tls头长度
						
						HttpsHandShakeHashUpdata(tempBuf+2+TLS_RecordHead_LENGTH, handShakeLen, TLS_OTHER_BLOCK, NULL);
						
						curDataOff += (TLS_RecordHead_LENGTH + handShakeLen);
						
						//处理serverHello, ServerKeyExchange, serverHelloDown同时下来的情况, ServerKeyExchange和serverHelloDown可以带或不带TLS_RecordHead(16030xxxxx)的5字节头, 把ServerKeyExchange和serverHelloDown数据往前移，下一个循环进入case ServerKeyExchange或者serverHelloDown处理
						if ((handShakeLen+TLS_RecordHead_LENGTH) < remainDataLen)
						{			
							//没有tls头
							if ((tempBuf[curDataOff] == HandshakeType_ServerKeyExchange) || (tempBuf[curDataOff] == HandshakeType_ServerHelloDone))		
							{
								memmove(tempBuf+2+TLS_RecordHead_LENGTH, tempBuf+curDataOff, remainDataLen-(curDataOff-2));
								
								remainDataLen -= handShakeLen;	

								SetWord(&tempBuf[2+TLS_RecordHead_LENGTH-2], remainDataLen-TLS_RecordHead_LENGTH ); 		//此处+2再-2是方便理解逻辑
								
								curDataOff = 2;
								
								continue;
							}	
							//有tls头	
							else if ((tempBuf[curDataOff] == TLS_HANDSHAKE_PROTOCOL) && (G_TLSSecurityParameters.tlsProtcolVersion == BuildWord(tempBuf+curDataOff+1)))
							{
								memmove(tempBuf+2, tempBuf+curDataOff, remainDataLen-(curDataOff-2));
								
								remainDataLen -= (handShakeLen + TLS_RecordHead_LENGTH);	
								
								curDataOff = 2;
								
								continue;
							}
							else 
							{	
								setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_HANDSHAKE_FAILURE, ALERT_LEVER_FATAL); // 0228
							}
						}					
					}
					else if (handShakeHead[0] == HandshakeType_ServerKeyExchange)
					{
						G_TLSSecurityParameters.tlsRecordState = TLS_RECEIVE_SERVER_HELLO_DONE_STATE;
						
						handShakeLen = TLS_HandShakeHead_LENGTH + BuildWord(&handShakeHead[2]);
						
						HttpsHandShakeHashUpdata(tempBuf+2+TLS_RecordHead_LENGTH, handShakeLen, TLS_OTHER_BLOCK, NULL);
						
						curDataOff += (TLS_RecordHead_LENGTH + handShakeLen);
						
						if ((handShakeLen+TLS_RecordHead_LENGTH) < remainDataLen)
						{
							if (tempBuf[curDataOff] == HandshakeType_ServerHelloDone)	
							{
								memmove(tempBuf+2+TLS_RecordHead_LENGTH, tempBuf+curDataOff, remainDataLen-(curDataOff-2));
								
								remainDataLen -= handShakeLen;	

								SetWord(&tempBuf[2+TLS_RecordHead_LENGTH-2], remainDataLen-TLS_RecordHead_LENGTH ); 		//此处+2再-2是方便理解逻辑
								
								curDataOff = 2;
								
								continue;
							}	
							else if ((tempBuf[curDataOff] == TLS_HANDSHAKE_PROTOCOL) && (G_TLSSecurityParameters.tlsProtcolVersion == BuildWord(tempBuf+curDataOff+1)))
							{
								memmove(tempBuf+2, tempBuf+curDataOff, remainDataLen-(curDataOff-2));
								
								remainDataLen -= (handShakeLen + TLS_RecordHead_LENGTH);	
								
								curDataOff = 2;
								
								continue;
							}
							else 
							{	
								setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_HANDSHAKE_FAILURE, ALERT_LEVER_FATAL);
							}
						}
					}
					else if(handShakeHead[0] == HandshakeType_ServerHelloDone)
					{	
						if(remainDataLen != 9)                         //解析到ServerHelloDone时后续还有数据,报错，关通道
						{
							G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;  //在安全参数协商完成之前就收到了changeCiper消息，则关闭通道报错				
						}
						else
						{
#if M2M_EUICC
							//isdr的时候，只支持tls1.2模式，《eUICC_M2M_SGP_11_v3_3_Card_Compliance_Test_Suite_v2_2_0---TestSuite_BasicInitialState---TC_SAR_4_HTTPSRestrictions_01_Nominal_Case_TLS_1_2_only_supported_by_ISDR》
							if ((G_HTTPSStatus.TriggeredSD == ISDR_APPID) && (G_TLSSecurityParameters.tlsProtcolVersion != TLS_Version12))
							{
								G_TLSSecurityParameters.tlsProtcolVersion = TLS_Version12;		//isdr的tls版本要求是tls1.2-0303

								setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_PROTOCOL_VERSION, ALERT_LEVER_FATAL);

								break;
							}
#endif
							HttpsHandShakeHashUpdata(handShakeHead, 4, TLS_OTHER_BLOCK, NULL);
							
							G_TLSSecurityParameters.tlsRecordState = TLS_SEND_CLIENT_KEY_EXCHANGE_STATE;	  //设置当前状态为ClientExchange状态，下面拼装ClientKeyExchange信息				
						}
					}	
					else
					{
						setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_HANDSHAKE_FAILURE, ALERT_LEVER_FATAL);// 0228
					}
					
					return;	
				}
				
				default:
					return;
			}
		}
		
		curDataOff += (recordLen + TLS_RecordHead_LENGTH);

		remainDataLen -= (recordLen + TLS_RecordHead_LENGTH);				
	}
	
	return;
}
/*
*@description: 对tls层的record 数据，主要是application数据进行解密 和计算mac操作，解密后的数据更新在http command层. 
*              根据实现方案，该函数仅对一条record进行解析
*              该函数传入的是一条完整的record记录，可能只包含http头，可能包含http头+半个http体，也可能包含1个http头+1个http体，也可能包含1个http头+1个http体+半个http头等各种情况，可以从http明文的任何位置断开
**@see        :
*	struct {
*          opaque IV[SecurityParameters.record_iv_length];
*          block-ciphered struct {
*              opaque content[TLSCompressed.length];
*              opaque MAC[SecurityParameters.mac_length];
*              uint8 padding[GenericBlockCipher.padding_length];
*              uint8 padding_length;
*          };
*      } GenericBlockCipher;

*    struct {
*          ContentType type;
*          ProtocolVersion version;
*          uint16 length;
*          opaque fragment[TLSPlaintext.length];
*      } TLSPlaintext;
*
*    MAC(MAC_write_key, seq_num +
*                            TLSCompressed.type +
*                            TLSCompressed.version +
*                            TLSCompressed.length +
*                            TLSCompressed.fragment);
*
*@function   :TLSApplicationDataDecryVerifyMac
*
*@param      :UWORD fileAddr, 存放record密文数据的文件地址,从 ContentType type;开始 type[0] + version[1,2] + len[3,4] + IV + CONTENT + MAC + PADDING
*@param      :WORD recordLen, record密文数据的长度，包括record head + body(密文)
*
*@return     :BYTE commandStatus ： TLS_RECORD_MACERROR/TLS_RECORD_OK or others
*
*@history    :created by xuxiaojun@20180516
*            :modify by xuxiaojun@20180604   由于MAC校验必须等解密完成之后才可以进行，而解密的话，是完全借用的，如果数据超过tempBuf的长度，则会发生越界。且考虑到目前的方案，httpHead文件的大小不管怎么设置都有缺陷，所以，该文件就用于存储解密后的明文数据，待全部解密完成后，再对数据进行解析
*
*NOTE        :该函数内部借用tempBuf时要特别小心，因为该函数可能会使用AES算法，而AES算法最底层会借用ResData
*             该函数内部借用RecData[0x00...0x1F]存放enc key
*                           RecData[0x20...0x3F]存放IV/MAC，因为IV和MAC不会同时使用
*                           RecData[0x40...0x5F]备份下一次的IV
*                           RecData[0x60...]用于数据临时存储
*/
void TLSApplicationDataDecryVerifyMac(UWORD fileAddr, WORD curDataOff, WORD recordLen)
{
	UWORD xdata httpsHeadAddr;
	WORD idata httpsHeadLen, cipherLen, contentLen, offsetFileAddr, currLen, offsetHeadFile, recDataLen;
	BYTE idata ivLength, paddingLen, blockNum, tmp[2];
	BYTE xdata HashWorkSpace[HASH_BLOCK_SIZE + HASH_BLOCK_SIZE + HASH_ICV_SIZE + 8];
	
	ivLength = 0;
	
	paddingLen = 0;
	
	contentLen = 0;
	
	memset(tmp, 0x00, sizeof(tmp));

	recDataLen = APDU_BUFFER_SIZE - 5 - 0x60;  //RecData中可以被借用来临时进行数据存储的大小

//	httpsHeadAddr = SelectPrivateTFFile(DF_BIPModule, EF_BIPModule_Send); 
	httpsHeadAddr = (UWORD)BIPModule_Send_ADDR_START;
	
	if(G_TLSSecurityParameters.cipherAlgorithm != ALG_NULL)          //由于TLSGetSessionKey内部会使用到tempBuf，所以将该函数的调用提前，对获取解密KEY和MAC KEY
	{
		if (((recordLen - TLS_RecordHead_LENGTH)%G_TLSSecurityParameters.blockLength) != 0)		//收到的数据不是 cbc 加密数据块长度的整数倍
		{
			setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_DECRYPT_ERROR, ALERT_LEVER_FATAL); // 0233

			return;
		}
		
		TLSGetSessionKey(Server_WirteKey_Value_Off, &RecData[0x00]); //获取解密 key ： LV 结构	
		
		if(TLSServerDownMsgExistIV() == FALSE)                       // 下行报文不存在IV向量
		{
			TLSGetSessionKey(Server_IV_Value_Off, &RecData[0x20]);   //1.0的向量从IV 中获取

			memmove(&RecData[0x20], &RecData[0x21], RecData[0x20]);			
		}
		else
		{
			ivLength = G_TLSSecurityParameters.blockLength;

//			ReadTFBuf(fileAddr, curDataOff+TLS_RecordHead_LENGTH, &RecData[0x20], ivLength); //拷贝初始向量，V 结构
			ReadEeprom(&RecData[0x20], fileAddr + curDataOff+TLS_RecordHead_LENGTH, ivLength);
		}
	}	
	
	offsetFileAddr = curDataOff;
	
//	ReadTFBuf(fileAddr, offsetFileAddr, &RecData[0x60], TLS_RecordHead_LENGTH);   //读取TLS记录的前 5字节 ， ContentType type; ProtocolVersion version; uint16 length;
	ReadEeprom(&RecData[0x60], fileAddr + offsetFileAddr, TLS_RecordHead_LENGTH);
	
	cipherLen = BuildWord(&RecData[0x63]);                                        //iv+密文数据长度
	
	offsetFileAddr += TLS_RecordHead_LENGTH;                //偏移到IV
	
	offsetFileAddr += ivLength;                                            //偏移到content
	
	currLen = recDataLen;
	
	if(G_TLSSecurityParameters.cipherAlgorithm != ALG_NULL)
	{
		currLen = (recDataLen/G_TLSSecurityParameters.blockLength)*G_TLSSecurityParameters.blockLength; 	//一次解密传入的数据长度
	}
	
//	httpsHeadLen = ReadTFFileWord(httpsHeadAddr);		//可能有上一次未解析完的数据，所以先获取上一次的有效数据个数，这次的数据接着上一次的数据存储
	httpsHeadLen = G_BIPModule_Send_Length;
	
	offsetHeadFile = (0x02 + httpsHeadLen);
	
	cipherLen -= ivLength;	
	
	while(cipherLen > 0)          //对record数据进行解密，得到的明文写入EF_HTTPSHeadSave文件                  
	{
		if(cipherLen <= currLen)
		{
			currLen = cipherLen;
			
			paddingLen = TRUE;    //借用paddingLen来表示当前是不是最后一块，用于获取padding值
		}
		
//		ReadTFBuf(fileAddr, offsetFileAddr, &RecData[0x60], currLen);
		ReadEeprom(&RecData[0x60], fileAddr + offsetFileAddr, currLen);
		
		if(G_TLSSecurityParameters.cipherAlgorithm != ALG_NULL)
		{
			if(paddingLen == TRUE)
			{
				memcpy(&RecData[0x40], &RecData[0x60+currLen - G_TLSSecurityParameters.blockLength], G_TLSSecurityParameters.blockLength);  //备份下一次的初始向量
			}
	
			TLSEncryDecryMsg(&RecData[0x00], &RecData[0x20],&RecData[0x60], currLen,  TLS_DecryMsg_Mode, 0);    //解密时，上一次的明文作为下一次解密的IV
		}
			
//		WriteTFBuf(httpsHeadAddr, offsetHeadFile, &RecData[0x60], currLen);                                 //解密后的数据写到EF_BIPModule_Send 文件中，没有去掉MAC和PADDING
		if((offsetHeadFile + currLen) <= BIPModule_Send_SIZE)
		{
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
			ecpMem_Copy(httpsHeadAddr +  offsetHeadFile, &RecData[0x60], currLen);
#else
			WriteEepromWithoutProtect(httpsHeadAddr +  offsetHeadFile, &RecData[0x60], currLen); 
#endif
		}
			
		offsetFileAddr += currLen;
			
		offsetHeadFile += currLen;
		
		if(paddingLen == TRUE) //当前是最后一块
		{
			if(G_TLSSecurityParameters.cipherAlgorithm != ALG_NULL)
			{
				paddingLen = RecData[0x60+currLen-1]+1;
				
				if(G_TLSSecurityParameters.tlsProtcolVersion == TLS_Version10)       //对于TLS 1.0 ，把IV 向量保存起来供下次加解密使用
				{
					TLSWriteSessionKey(Server_IV_Value_Off, &RecData[0x40]);         //TLS1.0 :The IV for subsequent records is the last ciphertext block from the previous record
				}
			}
			else
			{
				paddingLen = 0;
			}
			
			contentLen = recordLen - TLS_RecordHead_LENGTH - ivLength - paddingLen - G_TLSSecurityParameters.macLength;   //本次解密得到的实际的明文数据的长度
		}
		
		cipherLen -= currLen;		
	}
		
	TLSGetSessionKey(Server_WirteMacKey_Value_Off, &RecData[0x20]);	 //获取mac key ： LV 结构，该函数内部会借用tempBuf
	
//	ReadTFBuf(fileAddr, curDataOff, &RecData[0x60], TLS_RecordHead_LENGTH);   //读取TLS记录的前 5字节 ， ContentType type; ProtocolVersion version; uint16 length;
	ReadEeprom(&RecData[0x60], fileAddr + curDataOff, TLS_RecordHead_LENGTH);
	
	SetWord(&RecData[0x63], contentLen);                                //设置参与mac运算的content 数据长度
	
	blockNum = TLS_FIRST_BLOCK;			                             //初始化为第一块
	
	TLSCalMsgMAC(ServerDownMsg_Mode, TLS_FIRST_BLOCK, &RecData[0x20], &RecData[0x60], TLS_RecordHead_LENGTH, 0x00, &RecData[0x00], HashWorkSpace);         //对明文数据算MAC， 第一次时需要把前面的5字节头加上
	
	offsetHeadFile = (0x02 + httpsHeadLen);
	
	httpsHeadLen += contentLen;
	
//	WriteTFFileWord(httpsHeadAddr,httpsHeadLen);
	G_BIPModule_Send_Length = httpsHeadLen;
	
	currLen = recDataLen;                      //计算MAC时输入数据不需要是块的整数倍
	
	while(contentLen > 0)                      //对明文数据计算MAC
	{
		blockNum = TLS_OTHER_BLOCK;	
		
		if(contentLen <= currLen)              //最后一块
		{
			currLen = contentLen;	

			blockNum |= TLS_LAST_BLOCK;        //这里用&=是为了表示该数据既是第一块也是最后一块的情况	 
		}
		
//		ReadTFBuf(httpsHeadAddr, offsetHeadFile, &RecData[0x60], currLen); // 后续数据
		ReadEeprom( &RecData[0x60], httpsHeadAddr + offsetHeadFile, currLen);
		
		TLSCalMsgMAC(ServerDownMsg_Mode, blockNum, &RecData[0x20], &RecData[0x60], currLen, 0, &RecData[0x00], HashWorkSpace); //对明文数据算MAC， 第一次时需要把前面的5字节头加上
		
		if((blockNum & TLS_LAST_BLOCK) == TLS_LAST_BLOCK)
		{
//			ReadTFBuf(httpsHeadAddr, offsetHeadFile+currLen, &RecData[0x60], G_TLSSecurityParameters.macLength); // 读MAC
			ReadEeprom( &RecData[0x60], httpsHeadAddr + offsetHeadFile+currLen,  G_TLSSecurityParameters.macLength);
			
			if(memcmp(&RecData[0x60],&RecData[0x00], G_TLSSecurityParameters.macLength) != EQUAL)		    //compare mac	
			{
				setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_BAD_RECORD_MAC, ALERT_LEVER_FATAL); // 0214
				
				return;
			}
		}
		offsetHeadFile += currLen;
		
		contentLen -= currLen;
	}
	
	G_TLSSecurityParameters.tlsException = TLS_RECORD_OK;
	
	G_TLSSecurityParameters.serverSeqNum++;				//服务器端序列号+1
}
/*
*@description: TLS层处理收到的Application Data,注意，此时握手协议已经全部完成，该功能暂时不考虑握手协议的重传机制
               该函数的主要作用是解密TLS层的数据，并将解密后的数据写入HTTP层，供HTTPS层去处理
*
*@function   : TLSProcessRecieveApplicationDataMsg
*
*@param       :WORD offset bipReceive文件中接收到并保存了的数据的总长度  //该参数为标记，表示当前是否是C2的D6事件data available的最后一条，该值为0则表示当前数据已经接收完成，否则需要继续接收
*
*@return      :void
*
*@author      :
*
*@history     :created by xuxiaojun
*             :modify by xuxiaojun@20180615    根据<印尼Reliance电信AmendmentB测试脚本-20180614> T_028_HTTP_Seg_RAM_Chunked,指令不完整时需要继续接收
*             :modify by xuxiaojun@20180719    将借用了tempBuf循环写文件的地方用WriteEepromNonAtomic来替换
*             :modify by xuxiaojun@20180809    将该函数中对EVENT_PROACTIVE_HANDLER_AVAILABLE_bit(123)事件的处理移动到HTTP会话关闭之后。因为如果放在这里的话，就会在HTTP会话过程中要求发送主动式指令，印度rjio的refresh应用注册了123事件，然后发送refresh指令
*             :modify by jiangqb@20191120	   增加在解析https头解析出错时，把当前数据状态置为在eep中的状态
*/
BYTE TLSProcessRecieveApplicationDataMsg(BYTE channelDataLen, WORD offset)
{
	UWORD xdata bipReceiveAddr, httpsCommandAddr;
	WORD idata curDataOff, recordLen, remainDataLen, reWriteDataOff, bipSize;
	BYTE xdata tmp[5];// sessionState;

//	bipReceiveAddr = SelectPrivateTFFile(DF_BIPModule,EF_BIPModule_Receive);
	bipReceiveAddr = (UWORD)BIPModule_Receive_ADDR_START;
	
//	bipSize = GetTFSize(bipReceiveAddr);
	bipSize = BIPModule_Receive_SIZE;
	
//	remainDataLen = ReadTFFileWord(bipReceiveAddr);  //保存本次接收到的bip的数据长度
	remainDataLen = G_BIPModule_Receive_Length;
	
	curDataOff = 2;	
	
//	httpsCommandAddr = SelectPrivateTFFile(DF_TLS,EF_HTTPSCommandData);
	httpsCommandAddr = (UWORD)HTTP_CommandData_ADDR_START;
		
	G_HTTPSStatus.httpException = HTTP_OK;
	
	while(remainDataLen > TLS_RecordHead_LENGTH)
	{		
//		ReadTFBuf(bipReceiveAddr,curDataOff, &tmp[0], TLS_RecordHead_LENGTH);  //读取TLS记录的前 5字节 ， ContentType type; ProtocolVersion version; uint16 length;
		ReadEeprom(&tmp[0],bipReceiveAddr + curDataOff,TLS_RecordHead_LENGTH);
			
		recordLen = BuildWord(&tmp[3]);  
		
		if(recordLen == 0)  //只有TLS头，没有TLS体  电信AmendmentB测试脚本-20150618-C网&&G网/T_016_HTTP_NoSeg_RAM_FatalError
		{
			setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE,TLS_DECRYPT_ERROR,ALERT_LEVER_FATAL); // 0233
			
			return FALSE;
		}
		
		if(remainDataLen < (recordLen + TLS_RecordHead_LENGTH))  //剩余数据长度不满足一个record，不够解析则退出等待接收下一次，同时将未处理的数据移动到BIPReceive文件的起始，同时修改有效数据大小
		{
			if (remainDataLen == offset)		//数据全部没有处理
			{
				if (channelDataLen == 0)		//通道内无数据待接收, 当前数据全部不能处理，即是一个不完整的tls-record
				{
					return TRUE;		//用例TC_CV_3_ISDPNotEnabled_02_Nominal_HTTPS_Applet_Selectable_Only_on_Enabled_Profile中出现record内容是17030304C0，而第一次D6只下行0x200+的数据，此处要返回bip继续接收
				}
				else if ((remainDataLen+channelDataLen+2) > bipSize)	//防止bip能力不足接收数据，当前record又不是完整数据的情况下导致的死循环，报错返回
				{
					 //处理之前bipReceive的有效数据长度与处理之后的bipReceive的有效长度一致，说明bipReceive能力不够，无法接收更多的数据，此时要抛异常(数据溢出)
					setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_RECORD_OVERFLOW, ALERT_LEVER_FATAL); // 0216
				
					return FALSE;
				}
			}
			break;	//有完整的record被解密校验解析，跳出循环去226模块执行
		}
		
		if(tmp[0] != TLS_APPLICATION_DATA_PROTOCOL)   //收到的数据不是application  命令数据，终止会话
		{
			G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;
			
			return FALSE;
		}
		
		TLSApplicationDataDecryVerifyMac(bipReceiveAddr, curDataOff, recordLen+TLS_RecordHead_LENGTH);   //TLS层加解密， 每次解密一个tlsRecord
		
		if(G_TLSSecurityParameters.tlsException == TLS_RECORD_OK)
		{
			HTTPSAnalyseHead();         //MAC校验通过之后，对下行数据进行解析，可能会出现HTTP的某些异常
			
			if(G_HTTPSStatus.httpException != HTTP_OK)
			{
				if((G_HTTPSStatus.httpException == HTTP_HEAD_ERROR) || (G_HTTPSStatus.httpException == HTTP_BODY_ERROR))
				{
					//RFM模式时没有指定的app
					if ((G_HTTPSStatus.httpCommandType == HTTP_Command_Type_RFM) && ((G_HTTPSStatus.httpHeadExit & HTTP_HEAD_TARGETED_APP_EXIST) != HTTP_HEAD_TARGETED_APP_EXIST))
					{									
						setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_CLOSE_NOTIFY, ALERT_LEVER_WARNING);		//0100				//移动测试脚本---T_003_HTTP_NonSeg_RFM_NonNominal_InconsistantHeadAndBody
					}
					else
					{
		               //联通第三项测试中，天谕脚本期望在https头出错时统一报错0100 (用例： 联通测试第三项/国密卡测试数据https/机制测试\HTTP POST Response of Remote Administration Server/Testscript_N001,  X-Admin-Protocol和Content-Type的值不对时) (参考规范： rfc5246.pdf 7.2章节)
						setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_CLOSE_NOTIFY, ALERT_LEVER_WARNING);// 0100	
					}				
					
					return FALSE;
				}
		
				if(G_HTTPSStatus.httpException == HTTP_SESSION_FINISH)   //服务器下来204指示HTTP会话正常结束，需要发送警告
				{	
					if ((G_HTTPSStatus.httpContentType == HTTP_ContentType_LVMode) && (G_HTTPSStatus.contentLen.contentLength == 0))
					{
						setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE,TLS_CLOSE_NOTIFY,ALERT_LEVER_WARNING);// 0100
														
						return FALSE;
					}
					else if ((G_HTTPSStatus.httpHeadExit & HTTP_HEAD_URI_EXIST) == HTTP_HEAD_URI_EXIST)		//存在next uri 字段
					{
						G_HTTPSStatus.httpException = HTTP_Continue;
						
						return TRUE;
					}
					
					setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE,TLS_CLOSE_NOTIFY,ALERT_LEVER_WARNING);// 0100
														
					return FALSE;
				}
				
				if((G_HTTPSStatus.httpException == HTTP_UNKNOWN_APP_ERROR) || (G_HTTPSStatus.httpException == HTTP_NOT_SD_ERROR))  //这种情况下给服务器发送正常的17响应，通过HEADID_XAdminScript_Status进行提示，但是需要把BIPReceive清空
				{
//					WriteTFFileWord(bipReceiveAddr,0);
					G_BIPModule_Receive_Length = 0x00;
					
					return TRUE;
				}				
			}
		}
		else
		{
			G_TLSSecurityParameters.tlsRecordState = TLS_SEND_ALERT_DATA_STATE;
			
			return FALSE;
		}
		
		curDataOff += (recordLen+TLS_RecordHead_LENGTH);
		
		remainDataLen -= (recordLen+TLS_RecordHead_LENGTH);
	}
	
	//以下到d0循环都是偏移最后一块不完整的tls17record到bip接收文件的最前面
//	WriteTFFileWord(bipReceiveAddr,remainDataLen);//修改bipReceive有效数据长度
	G_BIPModule_Receive_Length = remainDataLen;
	
	reWriteDataOff = 2;
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
	if((bipSize-reWriteDataOff) < remainDataLen)
	{
		setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_CLOSE_NOTIFY, ALERT_LEVER_FATAL); //0200 文件大小不够
		
		return FALSE;
	}
	ecpMem_Move(bipReceiveAddr + reWriteDataOff,bipReceiveAddr+ curDataOff,remainDataLen);
#else
	if(FALSE == VMCopyDataToAddr(bipReceiveAddr , reWriteDataOff,bipSize-reWriteDataOff,bipReceiveAddr, curDataOff,remainDataLen,MODE_NO_PROTECT_WRITE,0,0))// modify by xuweihua@20220514 同时兼容配置区地址、文件头体分开、文件头体连接这三种情况下数据的移动
	{
		setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_CLOSE_NOTIFY, ALERT_LEVER_FATAL); //0200 文件大小不够
		
		return FALSE;
	}	
#endif
	
	//一次C2下来的所有record解析完成，获取到的command被处理
	//todo ： 进入业务逻辑的处理， 当前的command已经被写在A100/9103文件中，先判断一条指令是否完整，如果完整的话，则交给对应的RAM/RFM处理；如果不完整的话，则返回，等待接收下一条数据			
//	recordLen = ReadTFFileWord(httpsCommandAddr); //获取命令长度
	recordLen = G_HTTP_CommandData_Length;

#if GP__AMENDMENT__B__createSCP81Connection_FOR_MASK
	if (G_HTTPSStatus.HTTPTriggerMode == HTTPTriggeredBySCP81Connection)
	{
		if(G_HTTPSStatus.httpDataStatus == HTTP_DataType_END)		//scp81触发api的情况下不用经过226模块的命令分发
		{
			G_SCP81Connection.scp81CopyDataRemainLen = recordLen;
			
			Scp81ConnectionReport(SCP81_RECEIVE_DATA);	
		}
		return TRUE;
	}	
#endif	
	
	if(recordLen == 0) //recordLen == 0有两种情况，第一种是当前接收到的数据仅仅是头，还没有解析到数据体，这种情况需要返回继续接收数据
		               //                          第二种是数据体已经解析完毕，指令执行完成。本次下来的数据仅仅是数据的结束标记300D0A0D0A，这种情况下需要返回，并且发送POR
	{
		if(G_HTTPSStatus.httpDataStatus != HTTP_DataType_END)  //返回继续接收数据
		{
			return FALSE;
		}
	}
	
	if (G_HTTPSStatus.IsHTTPCommandExecueError == POR_OK)
	{
		G_Data_Type = G_HTTPSStatus.httpCommandType;	//区分 ram/rfm, native平台不需要,	该变量在RFMRAM函数中用于区分rfm/ram, 此处在native平台也需要赋值
		
		G_Source_Type = SourceType_HTTPS;
		
#if JAVA_CARD_UICC		
		G_RFMRAMKeyversion = IF_GetScp81KeyVersionNum();	//用于后续判断安全级别，native平台不需要
#endif
						
#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)
		G_HTTPSStatus.IsHTTPCommandExecueError = RemoteAPDUProcessFunction(httpsCommandAddr, 2, recordLen, G_HTTPSStatus.httpDataStatus,NULL);
#endif		
		G_Source_Type = SourceType_NULL;

#if JAVA_CARD_UICC
		G_SpecialRFM = FALSE;							//RFM的特殊aid标志， native没有该需求
#endif
	}
	else
	{		
//		ClearFileFirstWord(DF_TLS, EF_HTTPSCommandData);	
		G_HTTP_CommandData_Length = 0x00;
		
		return TRUE;	
	}
	
	if ((G_HTTPSStatus.httpHeadExit&HTTP_HEAD_URI_EXIST) != HTTP_HEAD_URI_EXIST)		//下行数据没有带next uri， 代表会话结束
	{
		if (G_HTTPSStatus.httpDataStatus == HTTP_DataType_END)
		{
			setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_CLOSE_NOTIFY, ALERT_LEVER_WARNING);//0100 用例 用例名称:  T_032_HTTP_ProtocolCheck_POSTResponse_NoXAdminNextURI
		
			return FALSE;
		}		
	}
	
	return TRUE;
}
/*
*@description: 组装TLS Application Protcol层的数据，并通过BIP 通道发送出去
*              考虑到缓存原因，TLS 层的opaque fragment[TLSPlaintext.length]数据已存放在EF_BIPModule_Send 文件中，但是不包括mac iv padding数据等
*@see        :
*	struct {
*          opaque IV[SecurityParameters.record_iv_length];
*          block-ciphered struct {
*              opaque content[TLSCompressed.length];
*              opaque MAC[SecurityParameters.mac_length];
*              uint8 padding[GenericBlockCipher.padding_length];
*              uint8 padding_length;
*          };
*      } GenericBlockCipher;

*    struct {
*          ContentType type;
*          ProtocolVersion version;
*          uint16 length;
*          opaque fragment[TLSPlaintext.length];
*      } TLSPlaintext;
*
*    MAC(MAC_write_key, seq_num +
*                            TLSCompressed.type +
*                            TLSCompressed.version +
*                            TLSCompressed.length +
*                            TLSCompressed.fragment);
*							
*@function   : TLSPatchApplicationOutData
*
*@param      : void
*
*@return     : void
*
*@author     : lx 20180516
*
*@history    :modify by xuxiaojun@20180531
*@NOTE       :该函数中会使用RecData[0x00...0x1f] 用来临时存储Client_WirteMacKey_Value_Off
*                           RecData[0x20...0x3f] 用来存储Client_WirteKey_Value_Off
*                           RecData[0x40...0x5F] 用于存储MAC值
*                           RecData[0x60...]     临时存储数据，其作用范围仅仅是该函数内部
*             该函数中使用tempBuf要特别小心，因为该函数可能会调用AES算法，而AES算法最底层借用了tempBuf
*/
BYTE TLSPatchApplicationOutData(void)
{
	BYTE idata ivLgth, paddingLen, blockNum;
	WORD idata httpDataLen, httpDataOff, srcDataDestOff, currLen, offset, recDataLen, allHttpDataDestOff, bipDataOff;
	UWORD xdata bipSendFileAddr, httpDataFileAddr;			
	BYTE xdata HashWorkSpace[HASH_BLOCK_SIZE + HASH_BLOCK_SIZE + HASH_ICV_SIZE + 8];

	ivLgth = 0;
	
	currLen = 0;
	
	paddingLen = 0;
		
	bipDataOff = 2;
	
//	bipSendFileAddr = SelectPrivateTFFile(DF_BIPModule,EF_BIPModule_Send); //密文响应数据
	bipSendFileAddr = (UWORD)BIPModule_Send_ADDR_START; //密文响应数据
	
//	httpDataFileAddr = SelectPrivateTFFile(DF_BIPModule,EF_BIPModule_Receive); // 明文响应数据
	httpDataFileAddr = (UWORD)BIPModule_Receive_ADDR_START; // 明文响应数据
	
	if(TLSClientUpMsgGnerateIV() == TRUE)  
	{
		ivLgth = G_TLSSecurityParameters.blockLength;
	}

	srcDataDestOff = 2 + TLS_RecordHead_LENGTH + ivLgth;		//2+5+16=0X17
	
	allHttpDataDestOff = 2 + TLS_RecordHead_LENGTH + ivLgth + G_BIPModule_Receive_Length;			//要加密的数据到该偏移为止		//2+5+16 + 0x266 = 27E
	
	TLSGetSessionKey(Client_WirteMacKey_Value_Off, &RecData[0]);					//mac key, RecData[0]是mackey的长度

	TLSGetSessionKey(Client_WirteKey_Value_Off, &RecData[0x22]);
	
	httpDataOff = 2 + TLS_RecordHead_LENGTH + ivLgth;                               //获取srcData在文件中content的起始偏移，2表示长度占用的字节数
		
	while (srcDataDestOff < allHttpDataDestOff)
	{		
		if ((allHttpDataDestOff - srcDataDestOff) < G_TLSSecurityParameters.TLSEachLength)		//剩余的要加密的数据长度小于  G_TLSSecurityParameters.TLSEachLength
		{
			httpDataLen = allHttpDataDestOff - srcDataDestOff;
		}
		else
		{
			httpDataLen = G_TLSSecurityParameters.TLSEachLength;
		}	
		
		srcDataDestOff += httpDataLen;
	
		paddingLen = TLSGetPendingLen(httpDataLen + G_TLSSecurityParameters.macLength); 			//计算出填充字节的个数		0X220->0X10

		recDataLen = APDU_BUFFER_SIZE - 5 - 0x60 - G_TLSSecurityParameters.macLength - paddingLen;  //RecData中可以被借用来临时进行数据存储的大小
		
		TLSPatchTypeAndVersion(TLS_APPLICATION_DATA_PROTOCOL, &RecData[0x60]);
	
		SetWord(&RecData[0x63], httpDataLen);                                              // 拼装TLS头  ProtocolVersion
	
		offset = 0x65;                                                                     //偏移到fragment[TLSPlaintext.length]
	
		if(TLSClientUpMsgGnerateIV() == TRUE)                                           //判断上行是否存在IV
		{				
			Random_Number_Generator_Get(&RecData[offset], ivLgth);                      //产生IV 向量
		}
			
		offset += ivLgth;                                                               //偏移到content[TLSCompressed.length]                
		
		blockNum = TLS_FIRST_BLOCK;			//初始化为第一块

		TLSCalMsgMAC(ClientUpMsg_Mode, TLS_FIRST_BLOCK, &RecData[0], &RecData[0x60], offset-0x60, ivLgth, &RecData[0x40], HashWorkSpace);         //对明文数据算MAC， 第一次时需要把前面的5字节头加上
	
		SetWord(&RecData[0x63], ivLgth + httpDataLen + G_TLSSecurityParameters.macLength + paddingLen);  //最后密文数据的长度，由于参数MAC计算的数据长度是明文数据长度，所以不能将这一步提到计算MAC之前做
			
//		if (!WriteTFBuf(bipSendFileAddr, bipDataOff, &RecData[0x60], TLS_RecordHead_LENGTH + ivLgth)) //修改TLS头和初始向量，到这时TLS中的长度已经是对后续长度的正确表示了，EF_BIPModule_Send文件大小不够
//		{
//			return FALSE;		//写文件失败
//		}
		if((bipDataOff + TLS_RecordHead_LENGTH + ivLgth) <= BIPModule_Send_SIZE)
		{
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
			ecpMem_Copy(bipSendFileAddr + bipDataOff , &RecData[0x60],TLS_RecordHead_LENGTH + ivLgth);
#else
			WriteEepromWithoutProtect(bipSendFileAddr + bipDataOff , &RecData[0x60],TLS_RecordHead_LENGTH + ivLgth);
#endif
		}
		
		bipDataOff += TLS_RecordHead_LENGTH + ivLgth;
		
		currLen = recDataLen;
	
		if(G_TLSSecurityParameters.cipherAlgorithm != ALG_NULL)
		{
			currLen = (recDataLen/G_TLSSecurityParameters.blockLength)*G_TLSSecurityParameters.blockLength - G_TLSSecurityParameters.blockLength; 	   //一次计算传入的数据的长度，TLSEncryDecryMsg函数中要求传入数据长度必须是块的整数倍

			if(TLSClientUpMsgGnerateIV() == FALSE)                                           //判断上行是否存在IV
			{
				TLSGetSessionKey(Client_IV_Value_Off, &RecData[offset]);                    //获取1.0的初始向量，用于加密的IV
					
				memmove(&RecData[offset], &RecData[offset+1], RecData[offset]);
			}
			
			memmove(&RecData[0x60], &RecData[0x65], G_TLSSecurityParameters.blockLength);         //后续用tempBuf[0...G_TLSSecurityParameters.blockLength-1]来存放IV	
		}
	
		//while(httpDataLen > 0)                      //对content计算MAC并加密，此时tls头已经计算了MAC，所以后面只需要对data计算MAC和加密，考虑到TLSEncryDecryMsg需要传IV进去，所以用tempBuf[0..ivLgth-1]表示初始向量
		while (httpDataOff < srcDataDestOff)
		{
			blockNum = TLS_OTHER_BLOCK;	
		
			if(httpDataLen <= currLen)              //最后一块
			{
				currLen = httpDataLen;	

				blockNum |= TLS_LAST_BLOCK; //这里用&=是为了表示该数据既是第一块也是最后一块的情况	 
			}

			httpDataLen -= currLen;
			
//			ReadTFBuf(httpDataFileAddr, httpDataOff, &RecData[0x60+G_TLSSecurityParameters.blockLength], currLen); // 读取原数据
			ReadEeprom(&RecData[0x60+G_TLSSecurityParameters.blockLength],httpDataFileAddr + httpDataOff,currLen);
			
			httpDataOff += currLen;			//http明文数据实际读取偏移
		
			TLSCalMsgMAC(ClientUpMsg_Mode, blockNum, &RecData[0], &RecData[0x60+G_TLSSecurityParameters.blockLength], currLen, 0, &RecData[0x40], HashWorkSpace); //对明文数据算MAC， 第一次时需要把前面的5字节头加上
		
			if((blockNum & TLS_LAST_BLOCK) == TLS_LAST_BLOCK)
			{
				memcpy(&RecData[0x60+G_TLSSecurityParameters.blockLength+currLen], &RecData[0x40], G_TLSSecurityParameters.macLength);           //将MAC写入缓存, 参与加密
				
				memset(&RecData[0x60+G_TLSSecurityParameters.blockLength+currLen+G_TLSSecurityParameters.macLength], paddingLen-1, paddingLen); //将padding写入缓存，参与加密
				
				currLen += (G_TLSSecurityParameters.macLength + paddingLen);
			}
		
			if(G_TLSSecurityParameters.cipherAlgorithm != ALG_NULL)
			{
				TLSEncryDecryMsg(&RecData[0x22], &RecData[0x60],  &RecData[0x60+G_TLSSecurityParameters.blockLength], currLen, TLS_EncryMsg_Mode, blockNum);
			}
		
//			if(!WriteTFBuf(bipSendFileAddr, bipDataOff, &RecData[0x60+G_TLSSecurityParameters.blockLength], currLen))  //写入加密后的数据
//			{
//				return FALSE;		//写文件失败
//			}
			if((bipDataOff + currLen) <= BIPModule_Send_SIZE)
			{
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
				ecpMem_Copy(bipSendFileAddr + bipDataOff ,  &RecData[0x60+G_TLSSecurityParameters.blockLength],currLen);
#else
				WriteEepromWithoutProtect(bipSendFileAddr + bipDataOff ,  &RecData[0x60+G_TLSSecurityParameters.blockLength],currLen);
#endif
			}

			if((blockNum & TLS_LAST_BLOCK) == TLS_LAST_BLOCK) 
			{					
				if ((G_TLSSecurityParameters.tlsProtcolVersion == TLS_Version10) && (G_TLSSecurityParameters.cipherAlgorithm != ALG_NULL)) //对于TLS 1.0 ，把IV 向量保存起来供下次加解密使用
				{
					TLSWriteSessionKey(Client_IV_Value_Off,&RecData[0x60+currLen]);
				}
			}
			else
			{
				if(G_TLSSecurityParameters.cipherAlgorithm != ALG_NULL)
				{
					memmove(&RecData[0x60], &RecData[0x60+currLen], G_TLSSecurityParameters.blockLength);  //后续用tempBuf[0...ivLgth-1]来存放IV，将上一次计算的密文的后ivLgth做为下一次的IV
				}
			}
			
			bipDataOff += currLen;			//写入偏移
		}

		G_TLSSecurityParameters.clientSeqNum++;
	}
	
//	WriteTFFileWord(bipSendFileAddr, bipDataOff-2);
	G_BIPModule_Send_Length = bipDataOff-2;
	
//	ClearFileFirstWord(DF_BIPModule,EF_BIPModule_Receive);		//清空缓存的要发送的http明文数据，避免在下次接收数据的时候判断成已存在数据
	G_BIPModule_Receive_Length = 0x00; 
	
	
	return TRUE;
}

/*
*@description :保存本次上行数据的URI到文件的第六条记录, 用于后面的可能的重连
*
*@see         :
*
*@function    :saveNextUriForRetry
*
*@param       :void
*
*@return      :void
*
*@history     V1.0 :created by jiangqb @20191122
*				v1.1: modify by jiangqingbin@20220505 该函数的意义就是为了保存nextUri用于重试，当不支持重试时，不需要执行本函数
*/
void saveNextUriForRetry()
{
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY
	WORD idata pskidLen;
	UWORD xdata fileTriggerAddr;
	
	if (G_TLSSecurityParameters.clientSeqNum > 1)
	{
		fileTriggerAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPTriggerPara);
		
		pskidLen = ReadLFFileValue(fileTriggerAddr, HTTP_TriggerPara_NextURIPara_Index, tempBuf);
		
		WriteLFFileValue(fileTriggerAddr, HTTP_TriggerPara_URIPara_Index, tempBuf, pskidLen);			//把第七条记录的数据复制到第六条
	}
#endif
}
				
				
				
/*
*@description :处理TLS 流程，从握手到后续处理application data之间所有的
*
*@see         :RFC5246.pdf
*
*@function    :TLSHandProcess
*
*@param       :BYTE channelDataLen   该值为0表示数据接收完成,该参数对于TLS_RECEIVE_APPLICATION_DATA_STATE状态有效
*@param       :WORD offset           该参数对于TLS_RECEIVE_APPLICATION_DATA_STATE状态有效,用于表示本次接收数据的存储位置在文件中的偏移
*
*@return      :void
*
*@history     V1.0 :created by xuyunzhen @20180416
*NOTE         V1.1 :modfiy by xuxiaojun@20180803  G_HTTPSendRefresh的值在这里不能被清掉，因为refresh指令发送不成功的话，还需要在status指令处理过程中继续发送. 参考其他卡商的log，将refresh retry放在status指令中《20180724161512_LYF LS 5013_Command send for SPN update during call_After Call disconnected SIM refreshed and SPN update successfully_Other Vendor Card.txt》
*             之前曾考虑过，在该函数中重复发送refresh指令直到成功，但是这样可能会导致发送的次数太多影响其他的处理，所以就在这里设置一个标记，等到卡片空闲时发送STATUS指令时再重发
*             V1.2 :modify by xuxiaojun@20180803  增加retry相关功能。在每次HTTP响应上发之后清空G_HTTPSStatus.httpCommandType的值，因为会出现先发RAM，再发RFM的情况。需求来自印度rjio。先下发RAM指令，然后回POR，再下发RFM，再回POR
*             V1.3 :modify by xuxiaojun@20180809  删除HTTP会话结束时对refresh主动式指令的特殊处理。原因是将123事件触发移动到HTTP会话结束之后，可以直接触发rjio的refresh应用的123事件，发出refresh主动式指令。虽然发送refresh的位置在表现上是一样的，但是不需要在status中重新发送，避免对ACTIVATION应用产生影响 
*             V1.3 :modify by jiangqb@20191122	优化代码
*             V1.4 :modify by xuxiaojun@20200620 调整代码与NM_scp81ConnectionClose处理相匹配
*/
void TLSHandProcess(BYTE channelDataLen, WORD offset)
{
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY	
	UWORD xdata SessionFileAddr;
	
	SessionFileAddr = SelectPrivateTFFile(DF_TLS, EF_TLSsessionData);
#endif	
#if !SE_ONLY
	if(G_HttpExecutingProfileIndex != G_CurExecutingProfileIndex)
	{
		G_CurExecutingProfileIndex = G_HttpExecutingProfileIndex;
		ExecuEnvironmentInit(G_CurExecutingProfileIndex);
	}
#endif
	while(TRUE)
	{
		switch (G_TLSSecurityParameters.tlsRecordState & TLS_RECORD_STATE_MASK)  //tlsRecordState的最高位表示当前HTTP会话是否正常终止，低7位表示其状态
		{
			case TLS_SEND_CLIENT_HELLO_STATE:   //1  拼装 Client hello" 消息，并发送出去
			{				
				if (!TLSSendClientHelloMsg())
				{
					G_TLSSecurityParameters.tlsRecordState = TLS_SEND_ALERT_DATA_STATE;						
					break;
				}
				
				if (CMD_Success != BIPModuleSendResponseData(DataInRam, tempBuf))  //发送数据不成功
				{
					G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY					
                    SetBit(G_TLSSecurityParameters.tlsRecordState, BIT_RETRY_WITH_START_TIMER); //TestRetry02
#endif					
					break;					
				}
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY					
				//clienthello发送成功，https会话才正式开启, 测试移动脚本中在openchannel后还没有发clienthello就复位时，上电后不期望https重发，所以把打文件标记从openchannel中移到发送clienthello成功后 用例;UICC_SUSPEND-->3GSUS_UICC_6212			
				if(ValueBit(JcvmConfig_GetAmendmentBConfig(), BIT_GP__AMENDMENT__B__Open_Flag))
				{
					SetTFConst(SessionFileAddr, TLS_OpenCloseNormal_Offset, Https_Session_Running, 0x01);
				}
#endif					
				G_TLSSecurityParameters.tlsRecordState = TLS_RECEIVE_SERVER_HELLO_STATE;
				
				return;
			}
			case TLS_RECEIVE_SERVER_HELLO_STATE:
			case TLS_RECEIVE_SERVER_KEY_EXCHANGE_STATE:
			case TLS_RECEIVE_SERVER_HELLO_DONE_STATE:
			{
				TLSProcessRecieveHandShakeMsg();
				
				if((G_TLSSecurityParameters.tlsRecordState == TLS_SEND_CLIENT_KEY_EXCHANGE_STATE) 
					|| (G_TLSSecurityParameters.tlsRecordState == TLS_END_STATE)
				|| (G_TLSSecurityParameters.tlsRecordState == TLS_SEND_ALERT_DATA_STATE))
				{
					break;
				}
				
				return;
				
			}
			case TLS_SEND_CLIENT_KEY_EXCHANGE_STATE:  //此处要考虑数据一起发送
			{
				TLSSendClientKeyExchangeMsg(0); 
								
				BIPModuleSendResponseData(DataInRam, tempBuf);
				
				TLSSendChangeCipherSpecMsg(0);  //拼装[ChangeCipherSpec]发送出去

				BIPModuleSendResponseData(DataInRam, tempBuf);
				
				TLSSendClientFinishedMsg(0); //拼装 Finished 
				
				if (BIPModuleSendResponseData(DataInRam, tempBuf))  //发送数据不成功
				{
					G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;
					
//					SetBit(G_TLSSecurityParameters.tlsRecordState, BIT_RETRY_WITHOUT_START_TIMER);//G_TLSSecurityParameters.tlsRecordState |= 0x40;
					
					break;	
				}
				
				G_TLSSecurityParameters.tlsRecordState = TLS_RECEIVE_SERVER_CHANGE_CIPHER_STATE;
				
				return ;
				
			}
			case TLS_RECEIVE_SERVER_CHANGE_CIPHER_STATE:
			case TLS_RECEIVE_SERVER_FINISHED_STATE:
			{
				TLSProcessRecieveHandShakeMsg();
				
				if((G_TLSSecurityParameters.tlsRecordState  == TLS_SEND_APPLICATION_DATA_STATE)|| 
					(G_TLSSecurityParameters.tlsRecordState == TLS_END_STATE)|| 
					 (G_TLSSecurityParameters.tlsRecordState == TLS_SEND_ALERT_DATA_STATE))
				{
#if GP__AMENDMENT__B__createSCP81Connection_FOR_MASK
					if ((G_TLSSecurityParameters.tlsRecordState == TLS_SEND_APPLICATION_DATA_STATE) && (G_HTTPSStatus.HTTPTriggerMode == HTTPTriggeredBySCP81Connection))//握手通过后
					{
						Scp81ConnectionReport(SCP81_CONNECTION_ESTABLISHED);
						
						G_TLSSecurityParameters.tlsRecordState = TLS_RECEIVE_APPLICATION_DATA_STATE;
						
						return;
					}
#endif
					
					// 如果是HTTPTriggeredBySCP81Connection建立会话失败，会走到TLS_END_STATE才发送通知，但是通知的状态却是不对的
					
					break;
				}
				
				return;
				
			}
			case TLS_RECEIVE_APPLICATION_DATA_STATE:
			{
				if(TRUE == TLSProcessRecieveApplicationDataMsg(channelDataLen, offset))
				{
					if((channelDataLen != 0) || ((G_HTTPSStatus.httpDataStatus != HTTP_DataType_END)&&(G_HTTPSStatus.httpException == HTTP_OK)))
					{
						return;
					}
					
					//此情况是app调用了NM_scp81ConnectionClose接口，把channelDataType清空了	 //HTTPTriggeredBySCP81模式的发送数据不走本循环，由应用调用NM_scp81ConnectionSend接口发送
					if ((G_BIPModuleStatus.channelDataType == ChannelDataType_NULL) || (G_HTTPSStatus.HTTPTriggerMode == HTTPTriggeredBySCP81Connection))		
					{						
						return;
					}
					
					G_TLSSecurityParameters.tlsRecordState = TLS_SEND_APPLICATION_DATA_STATE;
	
					G_HTTPSStatus.httpHeadExit = 0;

					break;
				}

				if((G_TLSSecurityParameters.tlsRecordState == TLS_END_STATE) || (G_TLSSecurityParameters.tlsRecordState == TLS_SEND_ALERT_DATA_STATE))
				{
					break;
				}

				return;	
			}
			case TLS_SEND_APPLICATION_DATA_STATE:  
			{
				saveNextUriForRetry();

				if (HttpPatchPostResponseData())  //组装HTTP层的post响应数据，组装后的数据放于EF_BIPModule_Send文件中
				{
					if (TLSPatchApplicationOutData())  //组装TLS 层的应用层数据，并通过bip通道将数据发送出去
					{
						if (BIPModuleSendResponseData(DataInEep, ADDR_IS_NULL))  //发送数据不成功			//DataInEep
						{
							G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;

							//联通第三项测试中，期望在发送数据失败后发送timer进行重试	(用例： 联通测试第三项/国密卡测试数据https/机制测试\RetryPolicy/Testscript_N003)	
							//通用测试脚本，期望发送失败发送timer进行重试	(用例： 通用AmendmentB测试/RAM/： T_010_HTTP_NoSeg_RAM_Retry_Twice和T_014_HTTP_NoSeg_RAM_Retry_SendFail)
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY
							SetBit(G_TLSSecurityParameters.tlsRecordState, BIT_RETRY_WITH_START_TIMER);
#endif							
							break;
						}
					}
					else
					{
						setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, HTTP_SESSION_FINISH, ALERT_LEVER_FATAL); // 0205
						break;
					}
				}
				else
				{
					setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, HTTP_SESSION_FINISH, ALERT_LEVER_FATAL);// 0205
					break;
				}
#if JAVA_CARD_UICC
				if(G_HTTPSStatus.httpCommandType == HTTP_Command_Type_RFM)
				{
					IF_TriggerRemoteFileUpdate();
				}
#endif				
				G_HTTPSStatus.IsHTTPCommandExecueError = POR_OK;
				
				HttpProtocolInit();
				
				G_TLSSecurityParameters.tlsRecordState = TLS_RECEIVE_APPLICATION_DATA_STATE;
				
				return; 				
			}	

			case TLS_SEND_ALERT_DATA_STATE:  //B  给服务器发alert信息
			{
				TLSSendAlertMsg();
				
				BIPModuleSendResponseData(DataInEep, ADDR_IS_NULL); 	
					
				if((G_TLSSecurityParameters.tlsAlertLever == ALERT_LEVER_FATAL) || (G_TLSSecurityParameters.tlsException == TLS_CLOSE_NOTIFY))  // Alert messages   with a level of fatal result in the immediate termination of the   connection.
				{
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY
					if(ValueBit(G_TLSSecurityParameters.tlsRecordState, BIT_RETRY_WITH_START_TIMER))//if((G_TLSSecurityParameters.tlsRecordState&0x80) == 0x80)
					{
						G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;  //这种情况下，下次需要重新开通道，在开通道时会清空BIP的收发文件

						SetBit(G_TLSSecurityParameters.tlsRecordState, BIT_RETRY_WITH_START_TIMER);
					}
					else
#endif
					{
						//ClearFileFirstWord(DF_Toolkit, EF_RAMExtendResponse);			//清空响应数据
						
						G_TLSSecurityParameters.tlsRecordState = TLS_END_STATE;
					}
					
					break;
				}
				else  //在这种情况下，不需要关闭通道，所以需要清空BIP的收发文件
				{
//					ClearFileFirstWord(DF_BIPModule,EF_BIPModule_Send);
					
					
//					ClearFileFirstWord(DF_BIPModule,EF_BIPModule_Receive);
					G_BIPModule_Receive_Length = 0x00; 
					G_BIPModule_Send_Length = 0x00;
								
					G_TLSSecurityParameters.tlsRecordState = TLS_RECEIVE_APPLICATION_DATA_STATE;
					
					return;
				}
			}	
			
			case TLS_END_STATE:
			{					
				BIPModuleProcessStatus(); //调用BIP模块的关通道函数
#if !SE_ONLY
				G_HttpExecutingProfileIndex = G_CurEnabledProfileIndex;
#endif
#if JAVA_CARD_UICC					
				bRamBuffer[REMOTE_SCP_SDID] = ILLEGAL_APPID;		//scp81通道关闭后对该值进行清空
#endif				
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY
				if(ValueBit(G_TLSSecurityParameters.tlsRecordState, BIT_RETRY_WITH_START_TIMER))  //表示http会话过程中下来的channel status终止会话，此时需要start timer， 这种情况下是否要通知应用？如果重试之后还是没有成功，此时该如何通知应用？
				{
#if M2M_EUICC
					//https发通知，数据发送不成功，则直接关通道然后开启timer，发通知的timer机制独立于原来的timer机制，参考例子《HB_Offline_Test--7通知流程及连接参数更新测试--Tele_TC1_UCSN1_TC1_UCSN2》
					if(IF_StartTimerForHttpsNotification() == TRUE)
					{
						return;
					}
#endif

					if(G_HTTPSStatus.RetryConterRemain > 0)
					{
						InitHttpVariable();							//用例名称:  T_010_HTTP_NoSeg_RAM_Retry_Twice -- undefined 用例中，二次触发https，此处 InitHttpVariable 必须在 IF_AnnexB_StartTimerAfterUnNormalEnd前;
						
						IF_AnnexB_StartTimerAfterUnNormalEnd();

						return;
					}
					else    //重试次数被耗尽
					{
						G_HTTPSStatus.httpException = HTTP_RETRY_EXHAUSTED; //HttpSessionReport(HTTP_RETRY_EXHAUSTED);  //通知当前触发HTTP会话的应用
					}
				}										
#endif
				HttpSessionReport(G_HTTPSStatus.httpException);  //通知当前触发HTTP会话的应用

#if GP__AMENDMENT__B__createSCP81Connection_FOR_MASK				
				if(G_TLSSecurityParameters.serverSeqNum < 1) // if(IF_HTTPS_HandShake()) 握手过程引发了关通道
				{
					Scp81ConnectionReport(SCP81_CONNECTION_SETUP_FAILED);
				}
				else
				{
					if(G_SCP81Connection.scp81ConnectionStatus == SCP81_CONNECTION_CLOSED)
					{
						Scp81ConnectionReport(SCP81_CONNECTION_CLOSED);
					}
					else
					{
						Scp81ConnectionReport(SCP81_COMMUNICATION_ERROR);
					}
				}

#endif				
				InitHttpVariable(); // include G_HTTPSStatus.HTTPTriggerMode = HTTPTriggeredNull;
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY					
				SetTFConst(SessionFileAddr, TLS_OpenCloseNormal_Offset, Https_Session_End, 0x01);  	//这里清标记的动作不能提前到if((G_TLSSecurityParameters.tlsRecordState&0x80) == 0x80)之前，同因为《20180727112012_LYF LS 4510_Power OFF During BIP Session then Power ON after some Time_Error_10K Applet not downloaded_Activation Card.txt》用例测试HTTP过程中关闭手机，此时也地收到channel status要求关闭通道的指令，而且开机之后要求发送timer manage
#endif				
				//refresh应用注册了123事件，当使用123事件触发应用时，refresh应用通过调用ToolKit_init/Toolkit_appendArray/Toolkit_Send发送refresh主动式指令。但是123事件只能触发一次，那后续的refresh retry如何处罚呢？
				//JavaToolkitProcessProactiveHandlerEvent 在IF_ProcessSystemProactiveCommand 中调用
				//JavaToolkitProcessProactiveHandlerEvent();

#if M2M_EUICC
				if(G_RefreshAfterCloseBIP == TRUE) //有refresh需要在http关闭后发送
				{
					G_RefreshAfterCloseBIP = FALSE;

					sendRefreshComm();
				}
#endif
				HttpProtocolInit();       //清除HTTPS层相关变量
				
				TLSInitVariable();        //清除TLS相关的全局变量
				
				if(TRUE == IF_AnnexB_HTTP_SecondSession_Start())	//触发第二个http会话,表示第二次HTTP会话的触发参数已经全部解析完成，可以将触发参数的长度清为0
				{
					HTTPSessionStart();
				}
#if  COS_UPDATE_ONLINE_BY_NativeOS	
				IF_UPCOS_ExpandHandle_ForCloseHttpChannel();	
#endif				
				return;
			}
			default:
				return;							
		}
	}
}
#endif

