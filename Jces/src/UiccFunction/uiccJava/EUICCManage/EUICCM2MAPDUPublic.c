/************************************************************************************************
Copyright (C), 1988-2003, Eastcompeace. Co., Ltd.

File name   : EUICCM2MAPDU.c

Author:     : xyz  

Version:    : V1.0

Description :  该文件主要存放M2M版本下 ES5 ES6 ES8接口的函数接口实现 

************************************************************************************************/ 
#include "EUICCM2MAPDUPublic.h"
#include "EUICCCustomerCertificate.h"
#include "EUICCMain.h"
#include "EUICCM2MAPDU.h"
#include "Interpreter.h"
#include "EUICCProfile.h"
#include "ASN1Operation.h"
#include "arithmatic.h"
#include "hard_api.h"
#include "uicc_main.h"
#include "Crypto_API.h"
#include "cardManager.h"
#include "keySet.h"
#include "object.h"
#include "dataManager.h"
#include "tagNode.h"
#include "SecurityDomainData.h"
#include "ToolKitAPIPublic.h"
#include "SEEUICCInterface.h"
#include "STKRemoteAppManagement.h"
#include "platform_api.h"   //warning  SHA256DoFinalSingleCall_Ex
#include "VMAccessEeprom.h" //warning  VMReadByte/VMReadUWORD
#include "opFunction.h"     //warning  getSdDataObjBySdid
#include "dataManager.h"    //warning  dataManager_storeData
#include "keyGroup.h"
#include "fm02_keyData.h"
#include "ProfileGPCommand.h"

#if M2M_EUICC
/*
*@function   :GenerateNotificationMessage
*
*@description:拼装通知数据
*
*@See        :数据参照 <SGP.02_v3.2_updated.pdf>Table 56: Data Format for Notification，ts_101220v110100p.pdf，ts_102223v150001p.pdf
*
*@param      :BYTE notifyType --- 输入参数,通知类型
*@param      :BYTE * IMEI --- 输入参数IMEI,结构为LV
*@param      :BYTE * outBuf --- 输出参数, 存放通知数据V
*
*@return     :BYTE outBuf中有效数据的长度, 最大不会超过53字节
*
*@RAM        :outBuf[0...]的格式为E1 L EID + Notification type + Notification  sequence number + ISD-P- AID + IMEI + MEID
*
*@author     :lx @20190102
*
*@history    :modify by lx @20190417
*/
BYTE GenerateNotificationMessage(BYTE notifyType,BYTE * IMEI,BYTE * outBuf)
{
	BYTE idata offset;
	UWORD xdata seqNumber;
	
	outBuf[0] = 0xE1; //eUICC notification tag. ‘E1’.To avoid conflicts with the values defined in ETSI TS 101 220 [2], a tag from the proprietary class is used.
	
	offset = 2; //outBuf[1]预留给通知长度,由于通知数据最大为53字节，所以这里预留1个字节
	
	outBuf[offset] = 0x4C; //EID的tag
	
	VMReadBuf(&outBuf[offset+1], EUICCEIDInfo_Addr, 0x11); //EID，LV结构，L的值为0x10
	
	offset += (outBuf[offset+1] + 2); //偏移到Notification type
	
	outBuf[offset] = 0x4D; //Notification type的tag
	
	outBuf[offset+1] = 0x01; //Notification type的length
	
	outBuf[offset+2] = notifyType; //Notification type
	
	offset += (outBuf[offset+1] + 2); //偏移到Notification sequence number
	
	outBuf[offset] = 0x4E; //Notification sequence number的tag
	
	outBuf[offset+1] = 0x02; //Notification sequence number的length
	
	seqNumber = VMReadUword(EUICCNotificationSeqNumber);
	
	SetWord(&outBuf[offset+2],seqNumber);
	
	offset += (outBuf[offset+1] + 2); //偏移到ISD-P- AID
	
	outBuf[offset] = 0x2F; //ISD-P- AID的tag
	
	outBuf[offset+1] = GetProfileAIDByProfileIndex(G_CurEnabledProfileIndex, &outBuf[offset+2]);//ISD-P- AID的length和value
	
	offset += (outBuf[offset+1] + 2); //偏移到IMEI	If both IMEI and MEID are retrieved, only one could be sent to limit overall message length.
	
	if(IMEI[0] != 0)
	{	
		outBuf[offset] = 0x14; //IMEI的tag	
		
		outBuf[offset+1] = IMEI[0]; //IMEI的length
		
		ecpMem_Copy(&outBuf[offset+2],&IMEI[1],IMEI[0]);

		offset += (outBuf[offset+1] + 2);
	}
	
	outBuf[1] = offset-2; //Length(EID+Notification type+Notification sequence number+ISDPAID+IMEI+MEID)							
	
	return offset;
}

/*
*@function   :CheckKeyEstablishment2DataAvarable
*
*@description:校验establishISDRKeySet/establishISDPKeySet过程第二条指令的3A02数据的合法性
*
*@param      :BYTE * dataBuf ,3A02的内容 
*
*@return     :BYTE dataLen  ,3A02的长度
*
*@author     :lgy@20181221
*
*@history    :V1.0
*/
BYTE CheckKeyEstablishment2DataAvarable(BYTE * dataBuf, BYTE totleLen)
{
	BYTE idata offset,ScenarioParameters;
	
	if(dataBuf[0] != 0xA6)
	{
		return FALSE;
	}
	
	if((dataBuf[1] + 2) != totleLen)
	{
		return FALSE;
	}
	
	offset = 2;
	
	if((dataBuf[offset] != 0x90)||(dataBuf[offset+1] != 0x02)||(dataBuf[offset+2] != 0x03))	//Scenario identifier ‘03’
	{
		return FALSE;
	}
	
	if((dataBuf[offset + 3]&0x08) == 0)//SGP02 V3.2   Table 83: Scenario Parameters
	{
		return FALSE;
	}
	
	ScenarioParameters = dataBuf[offset + 3];
	
	offset += 0x04;		//	Key Usage Qualifier
	
	if( (dataBuf[offset] != 0x95) || (dataBuf[offset+1] != 0x01) || ((dataBuf[offset+2] != 0x5C)&&(dataBuf[offset+2] != 0x10)) )	
	{
		return FALSE;
	}
	
	offset += 0x03;		//	指向Key Access
	
	if((dataBuf[offset] == 0x96)&&(dataBuf[offset+1] == 0x01))
	{
		if(dataBuf[offset+2] == 0xFF)//GP221 规范 Table 11-18: Key Access，‘FF’ Not available
		{
			return FALSE;
		}
		
		offset += 0x03;		//指向Key Type
	}
		
	if((dataBuf[offset] != 0x80)||(dataBuf[offset+1] != 0x01))
	{
		return FALSE;
	}
	
	offset += 0x03;		//指向Key Length
	
	if((dataBuf[offset] != 0x81)||(dataBuf[offset+1] != 0x01))
	{
		return FALSE;
	}
	
	offset += 0x03;		//Key Identifier
	
	if((dataBuf[offset] != 0x82)||(dataBuf[offset+1] != 0x01)) 
	{
		return FALSE;
	}
	
	if(dataBuf[offset+2] > 0x7F) // Key Identifier = '00' - '7F'，这里对于key id的判断，有疑问：本次密钥协商之后写入秘要的过程并没有用到这个字段，而且假设本次生成的是SCP03的key，其允许的key id是01，02，03，那此处的key id取值是什么？必须是01还是可以为01,02,03中的任何值？该字段如何被使用
	{
		return FALSE;
	}
	
	offset += 0x03;		//Key Version Number
	
	if((dataBuf[offset] != 0x83)||(dataBuf[offset+1] != 0x01))  
	{
		return FALSE;
	}
	
	if((dataBuf[offset+2] == 0x00) || (dataBuf[offset+2] > 0x7F)) // Key Version Number = '01' - '7F' 
	{
		return FALSE;
	}
	
	offset += 0x03;		//Initial value of sequence counter
	
	if((dataBuf[offset] != 0x91) || 
		((dataBuf[offset+1] != 0x00)&&(dataBuf[offset+1] != 0x02)&&(dataBuf[offset+1] != 0x03)&&(dataBuf[offset+1] != 0x05)&&(dataBuf[offset+1] != 0x08)))  // 这里是否要根据不同的协议类型来判断该字段的值是否正确
	{
		return FALSE;
	}
	
	offset += (dataBuf[offset+1] + 2);//Security Domain Image Number (SDIN)  ,OPTION
	
	if(dataBuf[offset] == 0x45)
	{
		offset += (dataBuf[offset+1] + 2);//HostID   ,Conditional
	}
	
	if(ScenarioParameters & 0x04)	//Include Host and Card ID in key derivation process 
	{
		if(dataBuf[offset] != 0x84)//HostID (shall only be present if scenario parameter b3 is set)
		{
			return FALSE;
		}
		
		offset += (dataBuf[offset+1] + 2);
	}
	
	if(totleLen != offset)
	{
		return FALSE;
	}
	
	return TRUE;
}
/*
*@function   :VerifyKeyEstablishment2Signature
*
*@description:校验establishISDRKeySet/establishISDPKeySet过程第二条指令的签名的合法性，
*
*@param      :BYTE * dataBuf 产生签名的源数数据(3A02 + 7F49)以及签名值(5F37)
*
*@return     :BYTE dataLen   待签名数据长度即3A02 + 7F49的总长度，dataBuf + dataLen后的数据为5F37（签名值）
*
*@RAM        :该函数中会借用temp[0...192]，仅在该函数内部有效
*
*@author     :lgy@20181221
*
*@history    :V1.0
*/
BYTE VerifyKeyEstablishment2Signature(BYTE * dataBuf,BYTE dataLen)
{
	BYTE idata offset;
	
	ecpMem_Copy(tempBuf,dataBuf + dataLen + 3,64);// 备份签名值 --- 5F37的内容 不包括tag
	
	offset = dataLen;
	
	dataBuf[offset++] = 0x00;
	
	dataBuf[offset++] = 0x85;	//0085 ----RC 
	
	dataBuf[offset++] = RC_Len;
	
	VMReadBuf(dataBuf + offset,EUICC_AuthenTmpBuf + TempSMSRCertRC_Off + 1,RC_Len);// 拼装establishISDRKeySet1产生的随机数
	
	//SHA256DoFinalSingleCall_Ex((UWORD)(dataBuf),offset + RC_Len ,tempBuf + 64);//用tempBuf + 128缓存哈希值		//无效代码， 校验签名函数内部会再次计算hash,jiangqingbin-20220825
	
	VMReadBuf(tempBuf + 128 ,EUICC_AuthenTmpBuf + TempSMSRCertPublickey_Off,64);

#if EUICC__FUNCTION__GUOMI_eSIM
	if(G_CertType == Cert_GUOMI)
	{
		if(VerifySignForSM2(tempBuf + 128, dataBuf, (offset + RC_Len), tempBuf) != TRUE)
		{
			return FALSE;
		}
	}
	else
#endif
	{
		if(VerifySignForECDSA(tempBuf + 128,64,VMReadByte(EUICC_AuthenTmpBuf + TempSMSRCertKeyParaRef_Off), dataBuf, (offset + RC_Len), tempBuf, tempBuf + 32,64,ALG_ECDSA_SHA_256) != TRUE)	//该函数在epx3.0中增加了一个传入参数
		{
			return FALSE;
		}
	}
	
	return TRUE;
	
}
/*
*
*@function    :ECKACaclSHS
*
*@description :计算SHS, Calculate ShS from ePK.SR.ECKA and SK.ECASD.ECKA.
*
*@param       :BYTE *shsBuf   输出   ， 输入数据存放在RecData中
*
*@return      :计算成功返回SHS的长度；计算失败返回FALSE
*
*@RAM         :tempBuf[0..63]存储ePK.SR.ECKA即0x7F49中的密钥值，tempBuf[64..95]存储SK.ECASD.ECKA, tempBuf[128..]给calcSHS用，从该函数返回时，tempBuf[0..shsLen]有效
*
*@test        :NO 
*
*@author      :lgy
*
*@history     :V1.0 :create @20181224
*/
BYTE ECKACaclSHS(BYTE* shsBuf)
{
	UWORD xdata ECASDCertAddr;
	BYTE idata keyPara,epublicKeyOff,shsLen;
	WORD idata euiccCertRef;
	
	euiccCertRef = memReadU16(MEMtPM8(M2M_EUICCEUMUICCCARef_Addr));
	
	if(euiccCertRef == REF_NULL)
	{
		return FALSE;
	}
	
	ECASDCertAddr = locateObjectInMemory(euiccCertRef);

	VMReadBuf(&tempBuf[64],M2MCAEUICCListGet_skEUICCArrayValueAddr(ECASDCertAddr) + 1,32);
	
	keyPara = VMReadByte(M2MCAEUICCListGet_parameterEUICCAddr(ECASDCertAddr));
	
	if(RecData[2] == 0x81)
	{
		epublicKeyOff = RecData[3] + 4;  //指向7F49 
	}
	else
	{
		epublicKeyOff = RecData[2] + 3;  //指向7F49 
	}
	
	epublicKeyOff += 4;//指向keyValue
	
	ecpMem_Copy(&tempBuf[0], &RecData[epublicKeyOff], 64);		//拷贝ePK.SR.ECKA到 tempBuf[0]中  固定 支持ECC256 Gx + Gy = 64字节
	
	if(checkPointOnEccCurve(&RecData[epublicKeyOff], 64, keyPara) == FALSE)
	{
		return FALSE;
	}
	
	shsLen = calcSHS(&tempBuf[0], 64,keyPara, &tempBuf[64], 32 , &tempBuf[128]);  //这里使用的时候要注意子函数调用时tempBuf[128..]是否会发生越界
	
	if(shsLen == FALSE)
	{
		return FALSE;
	}
	
	ecpMem_Move(shsBuf,&tempBuf[128],shsLen);
	
	return shsLen;  //64字节
	
}

/*
*@function   :GetShareInfo
*
*@description:用于establishISDRKeySet/establishISDPKeySet过程第二步 根据下行crt获取shareInfo和提取key的一些信息
*             参考GP附录E 4.8章节
*
*@param      :BYTE* CRTInfo,	 输入数据，下行中的'A6'的值域 9002XXXX 9501XX 9601XX...
*
*@param      :BYTE  CRTInfoLen   输入数据长度
*
*@param      :BYTE *shareInfo    输出 key usage qualifier + Key Type + Key Lengh + RD + hostId LV + IIN-LV + CIN-LV
*
*@param      :BYTE *keyCounter   输出
*
*@param      :BYTE *keyInfo      输出 key usage qualifier + key type + key length + Key Access  + Key Identifier + Key Version Number + scenarioParam
*
*@return     :BYTE dataLen       shareInfo的长度							
*
*@author     :lgy@20181224    在该函数调用之前 已经对数据进行过合法性检查  这里直接使用
*
*@history    :V1.0
*/
#define    keyUsageQualifierOff   0
#define    keyTypeOff             1
#define    keyLengthOff           2
#define    keyAccessOff           3
#define    keyIdentifierOff       4
#define    keyVersionNumberOff    5 
#define    scenarioParamOff       6

BYTE GetShareInfo(BYTE* CRTInfo,BYTE CRTInfoLen,BYTE* shareInfo,BYTE* keyCounter,BYTE* keyInfo)
{
	BYTE idata ScenarioPara,offset,shareInfoOffset,SDINOffset;
	UWORD xdata sdDataObj;

	ecpMem_Set(keyInfo,0,7);

	shareInfoOffset = 0;

	SDINOffset = 0;

	ScenarioPara = CRTInfo[3];

	offset = 4;//指向Key Usage

	shareInfo[shareInfoOffset++] = CRTInfo[offset + 2];//key usage qualifier

	keyInfo[keyUsageQualifierOff] = CRTInfo[offset + 2];

	offset += 3;  //Key Access  confitional

	if(CRTInfo[offset] == 0x96)
	{
 		keyInfo[keyAccessOff] = CRTInfo[offset + 2];

		offset += 3;  //Key Type
	}

	shareInfo[shareInfoOffset++] = CRTInfo[offset + 2];//Key Type

	keyInfo[keyTypeOff] = CRTInfo[offset + 2];

	offset += 3;  //Key Length

	shareInfo[shareInfoOffset++] = CRTInfo[offset + 2];//Key Lengh

	keyInfo[keyLengthOff] = CRTInfo[offset + 2];

	offset += 3;  //Key Identifier

	keyInfo[keyIdentifierOff] = CRTInfo[offset + 2];

	offset += 3;  //Key Version Number

	keyInfo[keyVersionNumberOff] = CRTInfo[offset + 2];

	keyInfo[scenarioParamOff] = ScenarioPara;

	offset += 3;  //Initial value of sequence counter

	ecpMem_Copy(keyCounter,&CRTInfo[offset + 1],CRTInfo[offset + 1] + 1);	//  LV结构

	offset += (CRTInfo[offset + 1] + 2);// 指向Security Domain Image Number (SDIN) 或者HostId

	if(CRTInfo[offset] == 0x45)
	{
		SDINOffset = offset;

		offset += (CRTInfo[offset + 1] + 2); //HostID
	}

	if(ScenarioPara & ScenarioPara_IncludeDRFlag)
	{
		Random_number_generator(&shareInfo[shareInfoOffset],0x10,1);//GP_AdemntE有定义DR的长度 根据哈希长度确定RD长度 Table 4-19: Length of the Derivation Random

		VMWriteBuf(EUICC_AuthenTmpBuf + TempSMSRCertRC_Off + 1,&shareInfo[shareInfoOffset],0x10);//备份RD  ，复用RC区域  长度10字节

		shareInfoOffset += 0x10;
	}

	if(ScenarioPara & ScenarioPara_IncludeHostFlag)//add hostID
	{
		if(CRTInfo[offset] != 0x84)
		{
			throw_error(0x6A80);
		}

		ecpMem_Copy(&shareInfo[shareInfoOffset],&CRTInfo[offset + 1],CRTInfo[offset + 1] + 1);  	//hostId LV

		shareInfoOffset += (CRTInfo[offset + 1] + 1);

//		shareInfo[shareInfoOffset] = GetSDGeneraldata(0x42, &shareInfo[shareInfoOffset+1], ISDR_APPID);  	//IIN-LV
		sdDataObj = sdData_getsdDataObjByAppId(ISDR_APPID);//GetsdDataObjByProfileIndex(ILLEGAL_PROFILEINDEX, ISDR_APPID);
		shareInfo[shareInfoOffset] = GetSDGeneraldata(&shareInfo[shareInfoOffset+1], 0x42, sdDataObj, sizeof(shareInfo)-shareInfoOffset-1, 0x00);

		shareInfoOffset += (shareInfo[shareInfoOffset] + 1);// CIN-LV

		if(SDINOffset != 0)
		{
//			ecpMem_Copy(&shareInfo[shareInfoOffset],&CRTInfo[offset + 1],CRTInfo[offset + 1] + 1);
			ecpMem_Copy(&shareInfo[shareInfoOffset],&CRTInfo[SDINOffset + 1],CRTInfo[SDINOffset + 1] + 1);

			shareInfoOffset += (shareInfo[shareInfoOffset] + 1);//
		}
		else
		{
//			shareInfo[shareInfoOffset] = GetSDGeneraldata(0x45, &shareInfo[shareInfoOffset+1], ISDR_APPID);  	//
			sdDataObj = sdData_getsdDataObjByAppId(ISDR_APPID);//GetsdDataObjByProfileIndex(ILLEGAL_PROFILEINDEX, ISDR_APPID);
			shareInfo[shareInfoOffset] = GetSDGeneraldata(&shareInfo[shareInfoOffset+1], 0x45, sdDataObj, sizeof(shareInfo)-shareInfoOffset-1, 0x00);

			shareInfoOffset += (shareInfo[shareInfoOffset] + 1);
		}

	}
	else
	{
		if(CRTInfo[offset] == 0x84)//If bit 3 is set (“Include Host and Card ID in key derivation process”) and tag '84' (Host ID) is not present
									//within tag 'A6' (see  Table 4-15), then an error shall be returned. Similarly, if bit 3 is not set and tag '84'
									//(Host ID) is present within tag 'A6' (see Table 4-15), then an error shall be returned.
		{
			throw_error(0x6A80);
		}
	}

	return shareInfoOffset;
//	return 0; //暂时返回0 实现函数以后返回shareInfoOffset
}
/*
*@function   :DerivationScp03Key
*
*@description:用于establishISDRKeySet/establishISDPKeySet过程第二步 创建scp03密钥或scp80密钥
*            
*
*@param      :BYTE* keyInfo,	输入数据 key usage qualifier + key type + key length + Key Access  + Key Identifier + Key Version Number + scenarioParam
*
*@param      :BYTE  keyCounter   输入 密钥的counter值 LV结构
*
*@param      :BYTE *keyValue    输出  密钥值 长度为keyLen*4
*
*@param      :BYTE  keyLen      单只密钥长度（该函数需要put 1/3支secureChannelKey）
*
*@param      :BYTE  appID    
*
*@param      :BYTE profileIndex :存储某个profile下的密钥
*
*@return     :void				
*
*@author     :lgy@20181225    
*
*@history    :V1.0 modify by xuxiaojun@20190505 修改SCP03KeyFormatInfo.sequenceCounter的赋值方式，避免大小端问题：如000003被赋值成节00000300
*            :V1.1 modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
*/
BYTE DerivationScpKey(BYTE* keyInfo,BYTE* keyCounter,BYTE* keyValue,BYTE keyLen,BYTE appID)
{
	BYTE xdata back;
	BYTE idata keyOff, i, num;
	BYTE idata checkValue[4]={0x03, 0x00, 0x00, 0x00};
	WORD xdata result;
	PN8 sdDataObject;
	u16 keySetRef,keyGroupRef;
	PN8 keySetObj;

	if(IsSCP03Key(keyInfo[keyVersionNumberOff])) //SCP03
	{
		if((keyCounter[0] != 3)&&(keyCounter[0] != 0)) //附录D 6.2.2.1 章节 scp03 key的counter是3字节
		{						//在这里先做判断 不合法直接返回 避免分配空间以后再判断造成回收空间比较麻烦
			return FALSE;
		}
	}
	else if(IsSCP80Key(keyInfo[keyVersionNumberOff])) //SCP80
	{
		if((keyCounter[0] != 5)&&(keyCounter[0] != 0)) //附录D 6.2.2.1 章节 scp80 key的counter是5字节
		{						//在这里先做判断 不合法直接返回 避免分配空间以后再判断造成回收空间比较麻烦
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	if(keyInfo[keyUsageQualifierOff] == 0x5C) // (1 secure channel base key)
	{
		num = 1;
	}
	else
	{
		num = 3;
	}

	result = TRUE;

	sdDataObject = MEMtPN8(getSdDataObjBySdid(appID));

	back = bRamBuffer[PROFILE_APDU_MODE]; //备份bRamBuffer[PROFILE_APDU_MODE]

	bRamBuffer[PROFILE_APDU_MODE] = TRUE; // java层总入口恢复初始值

	APDUBuffer[OFFSET_P1] = 0x00; //默认当前版本的密钥不存在

	if(keyInfo[scenarioParamOff]&ScenarioPara_DoNotDeleteKeyFlag)	//If the bit for "Do not delete existing keys" is set, all other keys in the SD shall remain unchanged. Else all keys in the SD except the newly generated keyset shall be deleted
	{
		keySetRef = keySet_getKeySet(sdDataObject, keyInfo[keyVersionNumberOff]);

		if(keySetRef != REF_NULL) //当前版本的密钥已存在,更新密钥
		{
			keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));

			for(i=0; i<num; i++)
			{
				if(keyInfo[keyTypeOff] != keySet_getKeyType(keySetObj, U8_A_U8(keyInfo[keyIdentifierOff], i),THROW_ERROR))
				{
					return FALSE;//测试用例:HB_Offline_Test/5KeyEstablish指令异常测试/Tele_TC1_ISDR_KeyEstablishment_ERROR_01/Transaction 2/步骤2、ISD-R,EKRT2-1 协商生成已有KEY，KEY TYPE与之前不同
				}

				if(keyLen != keyData_getKeyDataById(keySetObj, U8_A_U8(keyInfo[keyIdentifierOff], i), keyInfo[keyTypeOff], MEM_NULL))
				{
					return FALSE; //测试用例:HB_Offline_Test/5KeyEstablish指令异常测试/Tele_TC1_ISDR_KeyEstablishment_ERROR_01/Transaction 2/步骤3、ISD-R,EKRT2-2 协商生成已有KEY，KEY LENGTH与之前不同
				}
			}

			APDUBuffer[OFFSET_P1] |= keyInfo[keyVersionNumberOff];
		}
	}
	else
	{
		keyGroupRef = sdData_getKeyGroupRef(sdDataObject, sdData_getScpType(sdDataObject), FALSE);
		if (REF_NULL != keyGroupRef)
		{
			keySet_deleteAllScKey(sdDataObject);
			bRamBuffer[GC_Requested] = 1;//打垃圾回收标记
		}

	}

	if(keyInfo[keyUsageQualifierOff] == 0x5C) // (1 secure channel base key)
	{
		APDUBuffer[OFFSET_P2] = keyInfo[keyIdentifierOff] & 0x7F;

		APDUBuffer[OFFSET_LC] = keyLen + 1 + 11; //1是版本号的len，11是单支KEY的keyType+keyLen+checkValue(LV)+keyUsageQualifier(LV)+keyAccess(LV)的长度
	}
	else
	{
		APDUBuffer[OFFSET_P2] = keyInfo[keyIdentifierOff] | 0x80;

		APDUBuffer[OFFSET_LC] = keyLen*3 + 1 + 33; //1是版本号的len，33是三支KEY的keyType+keyLen+checkValue(LV)+keyUsageQualifier(LV)+keyAccess(LV)的长度
	}

	APDUBuffer[OFFSET_CLA] = 0x80;
	APDUBuffer[OFFSET_INS] = 0xD8;
	APDUBuffer[OFFSET_CDATA] = keyInfo[keyVersionNumberOff]; //key版本号

	keyOff = U8_A_U8(OFFSET_CDATA, 1); //指向第一支key

	for(i=0; i<num; i++)
	{
		APDUBuffer[keyOff++] = 0xFF;

		APDUBuffer[keyOff++] = keyInfo[keyTypeOff]; //keyType

		if(keyInfo[keyTypeOff] == keyType_AES)
		{
			U8_INC(APDUBuffer[OFFSET_LC]);
			if((IsSCP80Key(keyInfo[keyVersionNumberOff])) &&
							(((num == 3)&&(i == 1)) || ((num == 1)&&(keyInfo[keyIdentifierOff] == 0x02)))) //0x01<=keyVersion<=0x0F 且keyType为AES时,在kid2加maclen
			{
				APDUBuffer[keyOff++] = keyLen+2; //keyType为AES时，有两个keyLength，第一个length包含maclen的长度
			}
			else
			{
				APDUBuffer[keyOff++] = keyLen+1; //keyType为AES时，有两个keyLength
			}
		}

		APDUBuffer[keyOff++] = keyLen; //keyLength

		ecpMem_Copy(&APDUBuffer[keyOff],keyValue+(keyLen*i),keyLen); //keyValue

		keyOff += keyLen;

		if((IsSCP80Key(keyInfo[keyVersionNumberOff])) && (keyInfo[keyTypeOff] == keyType_AES) &&
				(((num == 3)&&(i == 1)) || ((num == 1)&&(keyInfo[keyIdentifierOff] == 0x02)))) //0x01<=keyVersion<=0x0F 且keyType为AES时,在kid2加maclen
		{
			APDUBuffer[keyOff++] = 0x08; //macLength = 0x08，<Profile interoperability technical specification_V2.2_FINAL.pdf>中说明macLength[7] UInt8 DEFAULT 8

			U8_INC(APDUBuffer[OFFSET_LC]);
		}

		ecpMem_Copy(&APDUBuffer[keyOff],checkValue,4); //checkValueLength+checkValue

		keyOff += 4;

		APDUBuffer[keyOff++] = 0x01; //keyUsageQualifierLength

		if(((num == 3)&&(i == 0)) || ((num == 1)&&(keyInfo[keyIdentifierOff] == 0x01)))
		{
			APDUBuffer[keyOff++] = 0x14; //keyUsageQualifier
		}
		else if(((num == 3)&&(i == 1)) || ((num == 1)&&(keyInfo[keyIdentifierOff] == 0x02)))
		{
			APDUBuffer[keyOff++] = 0x18; //keyUsageQualifier
		}
		else if(((num == 3)&&(i == 2)) || ((num == 1)&&(keyInfo[keyIdentifierOff] == 0x03)))
		{
			APDUBuffer[keyOff++] = 0x48; //keyUsageQualifier
		}
		else
		{
			return FALSE;
		}

		APDUBuffer[keyOff++] = 0x01; //keyAccessLength

		APDUBuffer[keyOff++] = keyInfo[keyAccessOff]; //keyAccess
	}

	KM_InitPutKeySequence(APDUBuffer[OFFSET_P1]);

	ecpMem_Copy(&G_profileGPState.keyCounter[0], &keyCounter[0], keyCounter[0]+1); //预置G_profileGPState.keyCounter，PutKeys时在resetSequenceCounter中做setSequenceCounter

	if(SW_NO_ERROR != PutKeys(sdDataObject))//, PUT_KEYS_MODE_EN)) //putKey  //该函数去掉了一个传入参数，之前同步代码时没有同步修改M2M调用位置的修改 -jiangqingbin-20220825
	{
		result = FALSE;
	}

//	if(result == TRUE)
//	{
//		setKeyCounterForSCP80(appID, keyInfo[keyVersionNumberOff], keyCounter[0], &keyCounter[1]); //setKeyCounter
//	}

	ecpMem_Set(&G_profileGPState.keyCounter[0], 0, 0x06); //恢复G_profileGPState.keyCounter

	bRamBuffer[PROFILE_APDU_MODE] = back; //恢复bRamBuffer[PROFILE_APDU_MODE]

	return result;

}
/*
*@description: 计算收条
*
*@Function   : GenerateDESReceipt                                           
* 
*@See        : 数据源参照GP 附录E 4.8.1	   算法参照 GP221  C.5 Receipts  Symmetric cryptography using the receipt key, an ICV of binary zeroes and the signature method
				described in appendix B.1.2.2 - Single DES Plus Final Triple DES MAC across the receipt data.   
*                                                                                                                                     
*@Param:     : 	BYTE *keyBuf 		计算收条的key
*@param		 :	BYTE keyLen 		计算收条的key的长度
*@param		 :	BYTE * katBuf  		计算收条的数据
*@param		 :	BYTE katLen  		计算收条的数据的长度
*@param		 :	BYTE * receiptBuf    计算的收条存放位置
*				                                                                                                         
*@Return:      BYTE 计算出来的收条的长度
* 
*@author      :
*/ 
BYTE GenerateDESReceipt(BYTE * keyBuf, BYTE keyLen, BYTE * katBuf, BYTE katLen, BYTE * receiptBuf)
{
	BYTE idata i,caculateLen;
	
	BYTE xdata icv[8];
	
	ecpMem_Set(icv,0,8);
	
	caculateLen = (katLen/8)*8 + 8;	//Padding of the data is defined in appendix B.4 - DES Padding	数据后面需要填充80 xx xx ，总长度为8的整数倍 ，如果源数据是8的整数倍 需要填充8字节（80 + 7个00）
	
	katBuf[katLen] = 0x80;	//上层传入的是RecData 足够多缓存8字节  这里直接在后面填充数据
	
	ecpMem_Set(&katBuf[katLen + 1],0,caculateLen - katLen - 1);
	
	if(caculateLen>8)                                           //参考<<GPC-Spec_2.2_Amendment_A_1.0.1>> this signature is a MAC of the appropriate data according to appendix B122 - Single DES plus Final Triple DES.
	{
		for(i = 0 ; i < (caculateLen-8); i=i+8)
		{
			SingleDES(&katBuf[i], keyBuf, icv, CBC_Init_Encrypt, 8);
		}
	}
    
    TripleDES(&katBuf[caculateLen - 8], keyBuf, icv,  CBC_Init_Encrypt, 8, 0x10);
    
    ecpMem_Copy(receiptBuf,icv,8);
    
	return 0x08;
}
/*
*@description: 计算收条
*
*@Function   : EUICCGenReceipt                                           
* 
*@See        : 数据源参照GP 附录E 4.8.1	  DES key产生的收条 算法参照 GP221  C.5 Receipts  Symmetric cryptography using the receipt key, an ICV of binary zeroes and the signature method
				described in appendix B.1.2.2 - Single DES Plus Final Triple DES MAC across the receipt data.   
*                                                                                                                                     
*@Param:     : 	BYTE *keyBuf 		计算收条的key
*@param		 :	BYTE keyLen 		计算收条的key的长度
*@param      :   BYTE keyType        生成收条的算法由生成key的keytype决定，不应该固定用AES算法
*@param		 :	BYTE * katBuf  		计算收条的数据
*@param		 :	BYTE katLen  		计算收条的数据的长度
*@param		 :	BYTE * receiptBuf    计算的收条存放位置
*				                                                                                                         
*@Return:    :BYTE 计算出来的收条的长度
* 
*@author     :lgy@20181229
*
*@history    :V1.0
*/ 
BYTE EUICCGenReceipt(BYTE * keyBuf, BYTE keyLen, BYTE keyType, BYTE * katBuf, BYTE katLen, BYTE * receiptBuf)
{
	
	if(keyType == keyType_DES)	  //  参考gp221 11.1.8   DES – mode (ECB/CBC) implicitly known
	{
		return  GenerateDESReceipt(keyBuf, keyLen, katBuf, katLen, receiptBuf);
	}
	else if(keyType == keyType_AES) //AES (16, 24, or 32 long keys)
	{				
		return Cmac(keyBuf, keyLen, katBuf, katLen, NULL, 0, receiptBuf);
	}
	else
	{
		return FALSE;
	}
	
}
/*
*@description: 将ISD-P的生命周期转换为SGP02V3.2章节Table 1: ISD-P Coding States的形式
*
*@Function   : TransformISDPLifeCycle                                           
* 
*@See        :   由于我们的应用信息表未记录Profile下载完成后的状态，需要转换一下 ，profile下载以后涉及到激活失效删除等操作 直接在配置区修改 而不是在应用信息表里修改   
				
*                                                                                                                                     
*@param		 :	BYTE* aidBuf  ,LV结构 
*@param      :  BYTE profileIndex 
*				                                                                                                         
*@Return:     ：lifeCycle
* 
*@author      :lgy@20190424
*/ 
BYTE TransformISDPLifeCycle(/*BYTE* aidBuf,*/BYTE profileIndex)
{
	BYTE idata lifeCycle;
	UWORD xdata profileEntryAddr,profileAppTableAddr;
	UWORD xdata theAppEntry;

	profileEntryAddr = GetProfileEntryAddr(profileIndex);

	profileAppTableAddr = GetProfile_theAppTableValueAddr(profileEntryAddr);

	theAppEntry = locateObjectInMemory(PN_R_U16(U32_A_U16(profileAppTableAddr, U16_M_U16(ISDP_APPID, SIZE_REF))));

	lifeCycle = memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(theAppEntry), bCurrentState_INDEX)));

	if(lifeCycle == SSD_SELECTABLE)//如果Profile存在 其生命周期只能是07和0F ，07表示刚安装还未建立密钥 ，0F表示建立密钥以及后续的状态
	{
		return SSD_SELECTABLE;
	}
	else  //0F ,1F ,3F
	{
		if(VMReadByte(PROFILE_VALIDTAG_CFG + profileIndex) != PROFILE_VALIDFLAG)//还未下载成功Profile数据
		{
			return SSD_PERSONALIZED;
		}

		if(JcvmConfig_GetProfileEnableIndex() == profileIndex)
		{
			return PROFILE_ENABLE; //激活
		}
		else
		{
			return PROFILE_DISABLE; //失效
		}
	}

	return PROFILE_DISABLE; //失效
}
/*
*@description: Get profile attribute
*
*@Function   : M2M_GetProfileAttributes
*
*@See        :    4.1.1.5 eUICC Capability Audit ----Table 29: ISD-P Properties
*
*@param      :  BYTE profileIndex
*
*@Return:     ：ProfileAttributes
*
*@author      :chentao@20220929
*/
BYTE M2M_GetProfileAttributes(BYTE profileIndex)
{
	BYTE idata result;

	result = 0;

	if(profileIndex == JcvmConfig_GetM2MFallBackProfileIndex())
	{
		result = FallBack_Attribute;
	}
#if M2M_ESx
	if(profileIndex == JcvmConfig_GetM2MEmergencyProfileIndex())
	{
		result = Emergency_Attribute;
	}
	if(profileIndex == JcvmConfig_GetM2MTestProfileIndex()) //test profile
	{
		result = Test_Attribute;
	}
#endif
	return result;

}
/*
*@description: 拼装EUICC  GetStatus的5Ctag
*
*@Function   : EUICCCapabilityAuditPaddingTagList                                           
* 
*@See        : EUICCCapabilityAudit子函数        
*                                                                                                                                     
*@Param:     : 	BYTE* outBuf
*@param		 :	BYTE* aidBuf  ,LV结构 
*@param      :  BYTE* tagList , other search criteria  + 5C  或者5C
*@param      :  BYTE tagLen tagList的总长度 
*@param      :  BYTE profileIndex 为了方便获取53tag的数据 
*				                                                                                                         
*@Return:     ：Len 本次拼装的数据长度
* 
*@author      :modify by xuxiaojun@20190514 获取8F TAG信息时，都是获取ISD-P，由于ISD-P关联到ISD-R，且ISD-R不支持CGM，所以无论ISD-P是否支持CGM，都是获取其自身的值
*/ 
//BYTE EUICCCapabilityAuditPaddingTagList(BYTE* outBuf,BYTE* aidBuf,BYTE* tagList,BYTE tagLen,BYTE profileIndex)
//{
//	BYTE idata i,offset,has5CTag,profileLifeCycle;
//	BYTE idata appID;
//	UWORD xdata CGMMemory, ActualMemory, RemainMemory;
//
//	has5CTag = 0;
//
//	profileLifeCycle = TransformISDPLifeCycle(profileIndex);
//
//	for(i = 0;i < tagLen;)//按照GP221 11.4.3 如果有5C tag，响应中只能包含5C tag的数据 按照table11-36拼装数据 ，根据SGP02_V3.2 ，5C tag必须出现在输入数据中 ，那么Other search criteria怎么处理？
//	{
//		if((tagList[i] == 0x9F)&&(tagList[i+1] == 0x70)&&(tagList[i+2] == 0x01))//根据SGP02 V3.2 的2.2.1.3 ISD-P章节 对生命周期的描述
//		{
//			if(tagList[i+3] != profileLifeCycle)//(GetAppIdByAIDFromAIDList(&aidBuf[1],aidBuf[0])))
//			{
//				return 0;
//			}
//
//			i += 4;
//		}
//		else if((tagList[i] == 0x53)&&(tagList[i+1] == 0x01))
//		{
//
//			if(tagList[i+2] == 0x01)//查找设置了fallBack属性的profile
//			{
//				if(profileIndex != VMReadByte(M2MFallBackProfileIndex))
//				{
//					return 0;
//				}
//			}
//			else if(tagList[i+2] == 0x00)
//			{
//				if(profileIndex == VMReadByte(M2MFallBackProfileIndex))
//				{
//					return 0;
//				}
//			}
//			else
//			{
//				ThrowError(SW_WRONG_DATA);
//			}
//
//			i += 3;
//
//		}
//		else if(tagList[i] == 0x5C)
//		{
//			if((i + tagList[i+1] + 2) != tagLen) //总长度不匹配 i是5C之前的长度 tagList[i+1]是5C的内容的长度
//			{
//				ThrowError(SW_WRONG_DATA);
//			}
//
//			has5CTag = 1;
//
//			break;
//		}
//		else
//		{
//			ThrowError(SW_WRONG_DATA);//return 0;
//		}
//	}
//
//	i += 2;	//指向5C的内容
//
//	if((has5CTag == 0)||(tagList[i] != 0x4F))	// 在EUICC 设备版 要求5C tag必须存在 ,响应数据必须存在4F （根据5C拼装）
//	{
//		ThrowError(SW_WRONG_DATA);  //查询条件不满足
//	}
//
//	offset = 2;
// 	outBuf[0] = 0xE3;
//
//	appID = ILLEGAL_APPID;
//
//	CGMMemory = 0x00;
//
//	ActualMemory = 0x00;
//
//	RemainMemory = 0x00;
//
//	for(;i<tagLen;i++)
//	{
//		switch(tagList[i])
//		{
//			case 0x4F:
//				outBuf[offset++] = 0x4F;
//				ecpMem_Copy(&outBuf[offset],aidBuf,aidBuf[0]+1);
//				offset += (aidBuf[0] + 1);
//				break;
//
//			case 0x9F:
//				if(tagList[i+1] == 0x70)
//				{
//					outBuf[offset++] = 0x9F;
//					outBuf[offset++] = 0x70;
//					outBuf[offset++] = 0x01;
//					outBuf[offset++] = profileLifeCycle;
//
//					i++;
//				}
//
//				break;
//
//			case 0x53:
//				outBuf[offset++] = 0x53;
//				outBuf[offset++] = 0x01;
//
//				if(profileIndex == VMReadByte(M2MFallBackProfileIndex))
//				{
//					outBuf[offset++] = 0x01;
//				}
//				else
//				{
//					outBuf[offset++] = 0x00;
//				}
//				break;
//
//			case  0x8F: //Cumulative Granted Non- Volatile Memory 测试用例<eUICC_M2M_SGP_11_v3_3_Card_Compliance_Test_Suite_v2_2_0/TestSuite_BasicInitialState/TC_ES5_ECA_1_eUICCCapabilityAudit_SMS_07_Nominal_Case_Retrieve_ISD_P_with_Mem_Information>
//			{
//				 VMReadBuf((BYTE*)&CGMMemory, GetAddrByIndexAddr(SEUICC_GetAPPLETENTRYTABLERef(ISDP_APPID)) + SDletEntry_CGMVolatileMemory_offset + GPFlash_Mode *sizeof(UWORD),sizeof(UWORD));//CGM
//
//				 outBuf[offset++] = 0x8F;
//				 outBuf[offset++] = 4;
//				 outBuf[offset++] = (CGMMemory>>24)&0xFF;
//				 outBuf[offset++] = (CGMMemory>>16)&0xFF;
//				 outBuf[offset++] = (CGMMemory>>8)&0xFF;
//				 outBuf[offset++] = (CGMMemory)&0xFF;
//
//				 break;
//			}
//
//			case 0x91: //Cumulative Remaining Non- Volatile Memory
//				 appID = GPGetCGMSdId(ISDP_APPID,GPFlash_Mode); //获取具有CGM的父SD，从其本身开始搜索
//
//				 if(appID == ILLEGAL_APPID)
//				 {
//					RemainMemory = GPGetCardRemainMemory(GPFlash_Mode); //该分支没有支持CGM的父SD  返回卡片剩余空间
//				 }
//				 else
//				 {
//				 	 VMReadBuf((BYTE*)&CGMMemory, GetAddrByIndexAddr(SEUICC_GetAPPLETENTRYTABLERef(ISDP_APPID)) + SDletEntry_CGMVolatileMemory_offset + GPFlash_Mode *sizeof(UWORD),sizeof(UWORD));//CGM
//
//				 	 VMReadBuf((BYTE*)&ActualMemory, GetAddrByIndexAddr(SEUICC_GetAPPLETENTRYTABLERef(ISDP_APPID)) + SDletEntry_TotalActualVolatileMemory_offset + GPFlash_Mode *sizeof(UWORD),sizeof(UWORD));//ActualMemory
//
//					 RemainMemory = CGMMemory -  ActualMemory;
//				 }
//
//				 outBuf[offset++] = 0x91;
//				 outBuf[offset++] = 4;
//				 outBuf[offset++] = (RemainMemory>>24)&0xFF;
//				 outBuf[offset++] = (RemainMemory>>16)&0xFF;
//				 outBuf[offset++] = (RemainMemory>>8)&0xFF;
//				 outBuf[offset++] = (RemainMemory)&0xFF;
//				 break;
//
//			default:
//				ThrowError(SW_REFERENCED_DATA_NOT_FOUND);
//		}
//	}
//
// 	outBuf[1] = offset-2;
//
//	return offset;
	
//}

/*
*@function   :CheckSMSRAddressingParametersSMS_A2
*
*@description:UpdateSMSRAddressingParametersSMS的子函数，用于判断SMS的子TAG A2的合法性
*
*@param      :BYTE* buffer   LV格式: L 81LV 82LV
*
*@return     :0x0000 TAG合法 / 0x6A80 TAG非法
*
*@test       :09 810301020382020102(9000)
*             08 8103010203820100(6a80)
*             12 8103010203820B0102030405060708090A0B(9000)
*             09 810301020383020102(6A80)
*             0D 81030102038202010283020102(6A80)
*             05 8103010203(6A80)
*             04 82020102(6A80)
*
*@author     :xuxiaojun
*
*@history    :created by xuxiaojun@20181225
*未处理情况  :TAG重复出现时，没有抛错; 但是TAG内的数据长度非法时，抛错。
*/
WORD CheckSMSRAddressingParametersSMS_A2(BYTE* buffer)
{
	BYTE idata offset,i;
	BYTE idata tagExist;
	
	if(buffer[0] == 0x00)
	{
		return 0x6A80;
	}
	
	offset = 1;
	
	tagExist = 0x00;

	while(offset <= buffer[0])
	{  
		i = buffer[offset]&0x03;
		
		if((tagExist & i ) != 0x00)  //存在重复的 81 82 tag
		{
			return SW_WRONG_DATA;
		}
		
		switch(buffer[offset])
		{
			case 0x81:
			{
				tagExist |= 0x01;
				
				if(buffer[offset+1] != 3)
				{
					return SW_WRONG_DATA;
				}
				
				break;
			}
			case 0x82:
			{
				tagExist |= 0x02;
				
				if((buffer[offset+1] < 2) || (buffer[offset+1] > 12))
				{
					return SW_WRONG_DATA;
				}
				
				break;
			}			
			default:
				return SW_WRONG_DATA;			
		}
		
		offset += (buffer[offset+1] + 2);
	}
			
	if((tagExist & 0x03) != 0x03)  // 81TAG和82TAG必须存在
	{
		return SW_WRONG_DATA;
	}
	
	return 0x9000;
}
/*
*@function   :CheckSMSRAddressingParametersSMS
*
*@description:M2M_StoreData_UpdateSMSRAddressingParameters的子函数，用于检查SMSR Addressing Parameters的SMS参数的有效性
*
*@param      :BYTE* buffer   LV格式: L 81LV A2LV A2LV...
*
*@return     :0x0000 更新成功 / 0x6A80, 0x6A84, 0x6A88 更新失败
*
*@author     :xuxiaojun
*
*@history    :created by xuxiaojun@20181225
*未处理情况  :81TAG重复出现的情况没有处理；不止一个A2指向同一个ISD-P的情况没有处理;A2指向一个卡内不存在的ISD-P时也会将其存储起来
*/
WORD CheckSMSRAddressingParametersSMS(BYTE* buffer)
{
	BYTE idata offset;
	BYTE idata tagExist;
	WORD idata result;
		
	if(buffer[0] < 0x04) //A3存在时 子tag’81’必须存在 2-12 bytes SM-SR Platform Destination Address*
	{
		return SW_WRONG_DATA;
	}
	
	offset = 1;
	
	tagExist = 0x00;
	
	result = 0x9000;
	
	while((offset <= buffer[0]) && (result == 0x9000))
	{
		switch(buffer[offset])
		{
			case 0x81:
			{
				if((tagExist&0x01) == 0x01)  //存在重复的81  tag
				{
					return SW_WRONG_DATA;   
				}
				
				tagExist |= 0x01;
				
				if((buffer[offset+1] < 2) || (buffer[offset+1] > 12))
				{
					result = SW_WRONG_DATA;
				}
				
				break;
			}
			case 0xA2:
			{
				tagExist |= 0x02;
				
				result = CheckSMSRAddressingParametersSMS_A2((BYTE*)&buffer[offset+1]);  // 判断SMS-A2 tag的合法性
				
				break;
			}
			default:
				result = SW_WRONG_DATA;			
		}
		
		offset += (buffer[offset+1] + 2);
	}
	
	if(result != 0x9000)
	{
		return result;
	}
	
	if( (tagExist&0x01) != 0x01)  // 81 TAG必须存在
	{
		return SW_WRONG_DATA;
	}
	
	return 0x9000;
}
/*
*@function   :CheckSMSRAddressingParametersCATTP
*
*@description:M2M_StoreData_UpdateSMSRAddressingParameters的子函数，用于检查SMSR Addressing Parameters的BIP for CATTP参数的有效性
*
*@param      :BYTE* buffer   LV格式: L 81LV A2LV A2LV...
*
*@return     :0x0000 更新成功 / 0x6A80, 0x6A84, 0x6A88 更新失败
*
*@author     :xuxiaojun
*
*@history    :created by xuxiaojun@20181225
*未处理情况  :暂时未处理数据非法情况
*/
WORD CheckSMSRAddressingParametersCATTP(BYTE* buffer)
{
	BYTE idata offset;
	BYTE idata transportLevelTag,destinationAddressTag;
	
	//SGP02 V3.2 4.1.1.10  :The CR bit of the tags shall be set to zero.     ,102220 7.1.1.1 Single byte format (CR bit 描述)
	//102220  :'3C' or 'BC' UICC/terminal interface transport level tag    '3E' or 'BE' Other address (data destination address) tag
	
	if(buffer[0] < 12)//102223  8.59 UICC/terminal interface transport level  5字节 ， 8.58 Other Address 最少7字节
	{
		return SW_WRONG_DATA;
	}
	
	offset = 1;
	transportLevelTag = FALSE;
	destinationAddressTag = FALSE;
	
	for(; offset < buffer[0];)
	{
		if((buffer[offset] == 0x3C)&&(buffer[offset+1] == 0x03))
		{
// 			if((buffer[offset+2] == 0)||(buffer[offset+2] > 6))//102223 8.59  Coding of the Transport protocol type:  保留值 做兼容处理 不抛错
// 			{
// 				return SW_WRONG_DATA;
// 			}
			
			transportLevelTag = TRUE;
			
			offset += 5;//指向下一个tag
		}
		else if(buffer[offset] == 0x3E)//102223  8.58 Other Address
		{
			if((buffer[offset+2] == 0x21)&&(buffer[offset+1] == 0x05))//'21' = Ipv4 address; Address information in octet 4 to octet 7 contains the Ipv4 address
			{				
				offset += 7;
			}
			else if((buffer[offset+2] == 0x57)&&(buffer[offset+1] == 0x11))//'57' = Ipv6 address; the Address information in octet 4 to octet 19 contains the Ipv6 address.
			{
				offset += 0x12;
			}
			else
			{
				return SW_WRONG_DATA;
			}
			
			destinationAddressTag = TRUE;
		}
		else 	//	A4 tag 里面只能有3E 和3C tag
		{
			return SW_WRONG_DATA;
		}
		
	}
	
	if((transportLevelTag == FALSE)||(destinationAddressTag == FALSE))
	{
		return SW_WRONG_DATA;
	}
	
	return 0x9000;
}
/*
*@function   :CheckSMSRAddressingParametersDNS_A2
*
*@description:CheckSMSRAddressingParametersDNS的子函数，用于判断DNS中A2 TAG的合法性
*
*@param      :BYTE* buffer   LV格式: L 3ELV 82LV ... /  L BELV 82LV ...
*
*@return     :0x9000 更新成功 / 0x6A80,
*
*@author     :xuxiaojun
*
*@history    :created by xuxiaojun@20181225
*未处理情况  :目前没有实现DNS，所以该函数直接返回0x00
*/
WORD CheckSMSRAddressingParametersDNS_A2(BYTE* buffer)
{
	return 0x9000;
}
/*
*@function   :CheckSMSRAddressingParametersDNS
*
*@description:M2M_StoreData_UpdateSMSRAddressingParameters的子函数，用于检查SMSR Addressing Parameters的DNS参数的有效性
*
*@param      :BYTE* buffer   LV格式: L 81LV A2LV A3LV...
*
*@return     :0x9000 更新成功 / 0x6A80, 0x6A84, 0x6A88 更新失败
*
*@author     :xuxiaojun
*
*@history    :created by xuxiaojun@20181225
*未处理情况  :目前COS不支持DNS,所以这里仅仅将数据存储起来
*/
WORD CheckSMSRAddressingParametersDNS(BYTE* buffer)
{
	BYTE idata offset;
	BYTE idata tagExist;
	WORD idata result;
	
	//If the SM-SR uses Tag ‘A5’ with a length of zero then neither of the fields above are required to be present.

	if((buffer[0] < 0x05) && (buffer[0] != 0)) //’81’ 1-n  ,‘A2’  0-n,2个TLV都是强制tag ，至少5字节  //  modify by chentao-20221010 A5 length 可以是0
	{
		return SW_WRONG_DATA;
	}
	
	offset = 1;
	
	tagExist = 0x00;
	
	result = 0x9000;
	
	while((offset <= buffer[0]) && (result == 0x9000))
	{
		switch(buffer[offset])
		{
			case 0x81:
			{
				tagExist |= 0x01;
				
				if(buffer[offset+1] < 1)   // 81的长度为1-n
				{
					result = SW_WRONG_DATA;
				}
				
				break;
			}
			case 0xA2:
			{
				tagExist |= 0x02;
				
				result = CheckSMSRAddressingParametersDNS_A2((BYTE*)&buffer[offset+1]);  // 判断SMS-A2 tag的合法性
				
				break;
			}
			case 0xA3:                     // A3的长度为1-n
			{
				tagExist |= 0x04;
				
				if(buffer[offset+1] < 1)
				{
					result = SW_WRONG_DATA;
				}
				
				break;
			}
			default:
				result = SW_WRONG_DATA;			
		}
		
		offset += (buffer[offset+1] + 2);
	}
	
	if(result != 0x9000)
	{
		return result;
	}
	
	if((tagExist&0x03) != 0x03)  // 81和A2 TAG必须存在
	{
		return SW_WRONG_DATA;
	}
	
	return 0x9000;
}
/*
*@function   :FinaliseISDRhandover_DeleteKey
*
*@description:M2M_DeleteCommand_FinaliseISDRhandover的子函数，用于删除ISD-R密钥中除buffer中指定密钥之外的其他密钥. 进入该函数时，buffer中的所有数据都是有效的
*@see        :Delete all keys except those in the key ranges indicated in the command parameters.这里应该是一个事务，要么所有的key全部删除，要么一个都不删除		
*
*@param      :BYTE* buffer     需要被保留的密钥，其格式参考<SGP.02_v3.2_updated.pdf> Table 49: Delete [card content] Command Data Field : F203XXXXXX F203XXXXXX
*@param      :BYTE bufLen      BYTE* buffer中有效数据长度，其值为5 or 10
*
*@return     :TRUE 删除成功；FALSE 删除失败
*
*@author     :xuxiaojun
*
*@history    :created by xuxiaojun@20181226
*注意        :这里是否需要考虑TOKEN等没有keyCompleteTag字段的密钥？
*/
void FinaliseISDRhandover_DeleteKey(BYTE* buffer, BYTE bufLen)
{
	UWORD xdata sdDataObj, keyGroupObj, keySetObj, curKeyVersion;
	BYTE idata transactionTag, i;
	WORD idata keyGroupRef, keySetRef;
	BYTE idata offset;
	PN8 keyLocation;
	u16 keyDataLen;
	jref nextKeyDataRef;


	sdDataObj = sdData_getsdDataObjByAppId(ISDR_APPID);

	keyGroupRef = sdData_getFirstKeyGroupRef(MEMtPN8(sdDataObj));

	transactionTag = IF_BeginTransaction();

	while(REF_NULL != keyGroupRef)
	{
		keyGroupObj = locateObjectInMemory(keyGroupRef);

		keySetRef = keyGroup_getKeySetRef(MEMtPN8(keyGroupObj));

		while(REF_NULL != keySetRef)
		{
			keySetObj = locateObjectInMemory(keySetRef);

			offset =0;

			curKeyVersion = keySet_getKeyVersion(MEMtPN8(keySetObj));

			if(curKeyVersion == buffer[2])
			{
				offset = 3;//指向保留的第一个kid
			}
			else if((bufLen == 10) && (curKeyVersion == RecData[7]))
			{
				offset = 8;
			}
			else
			{
				//todo nothing
			}

			if(offset != 0)//该版本的密钥需要被保留 ，判断需要保留的KID
			{
				for(i = KEY_ID_01; i <= KEY_ID_03; i++)
				{
					if((i < RecData[offset]) || (i > RecData[offset+1]))
					{
						keyLocation = MEMtPN8(FM03_locateKeyId(keySetObj, i, TAG_GET_KEY, &nextKeyDataRef, &keyDataLen));
						if(MEM_NULL == keyLocation)
						{
							continue;
						}

						keySet_deleteScKey(MEMtPN8(sdDataObj), curKeyVersion, i); //清除不需要保留的key

					}
				}
			}
			else
			{
				keySet_deleteScKey(MEMtPN8(sdDataObj), curKeyVersion, KEY_ID_INIT_VALUE);
			}

			keySetRef = keySet_getNextKeySetRef(keySetObj);
		}

		keyGroupRef = keyGroup_getNextKeyGroupRef(MEMtPN8(keyGroupObj));
	}

	IF_CommitTransaction(transactionTag);
}
/*
*@function   : MasterDelete_CheckB6Tag
*
*@description: Master Delete  操作中检查B6数据的合法性
*
*@param      :BYTE * dataOffset   //指向B6中数据的起始
*@param      :BYTE b6Len
*
*@return     :void
*
*@authro      :xuyz
*
*@history     :V1.0 :create @20181227
*/
void MasterDelete_CheckB6Tag(BYTE * dataOffset, BYTE b6Len)
{
	BYTE idata tagFlag;
	BYTE idata tagLen,lenLen,dataLen;
	BYTE idata offset;
	
	tagFlag = 0;
	
	for(offset =0; offset < b6Len; )
	{
		tagLen = 1; 
		
		switch(dataOffset[offset])
		{
			case 0x42:
				if((tagFlag&0x01) == 0x01)   //tag重复了
				{
					throw_error(SW_WRONG_DATA);
				}
				tagFlag |= 0x01;
							
				break;
				
			case 0x45:
				if((tagFlag&0x02) == 0x02)   //tag重复了
				{
					throw_error(SW_WRONG_DATA);
				}
				tagFlag |= 0x02;
							
				break;
			
			case 0x5F:
				if(dataOffset[offset +1 ] != 0x20)
				{
					throw_error(SW_WRONG_DATA);
				}
				
				if((tagFlag&0x04) == 0x04)
				{
					throw_error(SW_WRONG_DATA);
				}
				
				tagLen = 2;
				
				tagFlag |= 0x04;
				
				break;
				
			case 0x93:
				
				if((tagFlag&0x08) == 0x08)
				{
					throw_error(SW_WRONG_DATA);
				}
				
				tagFlag |= 0x08;
			
				break;
				
			default:
				
				throw_error(SW_WRONG_DATA);
		}
		
		if(dataOffset[offset + tagLen] < 0x80)
		{
			lenLen = 1;
			
			dataLen = dataOffset[offset + tagLen];
		}
		else
		{
			lenLen = 2;
			
			dataLen = dataOffset[offset + tagLen + 1];
		}
		
		offset += dataLen + lenLen + tagLen;
		
	}
	
	if((tagFlag&0x0F) != 0x0F)    //Mandaroy tag 不存在
	{
		throw_error(SW_WRONG_DATA);
	}
}

UWORD GetsdDataObjByProfileIndex(BYTE profileIndex, BYTE sdId)
{
	UWORD    xdata     profileEntry,appTableAddr,theAppEntry;
	WORD     idata     index, sdRef;

	profileEntry = GetProfileEntryAddr(profileIndex);

	appTableAddr = GetProfile_theAppTableValueAddr(profileEntry);

	for(index=0;index<APP_END;index++)
	{
		theAppEntry = locateObjectInMemory(PN_R_U16(U32_A_U16(appTableAddr, U16_M_U16(index, SIZE_REF))));

		if(PN_R_U8(PFtMEM(get_object_field_ref(MEMtPF8(theAppEntry), bAppID_INDEX))) == sdId)
		{
			sdRef = (PN_R_U16(PFtMEM(get_object_field_ref(MEMtPF8(theAppEntry), theApplet_INDEX))));

			return locateObjectInMemory(sdData_getSDdataRef(sdRef));
		}
	}

	return ADDR_IS_NULL;
}



UWORD GetKeyDataAddr(UWORD sdDataObj,BYTE keyVersion)
{
	WORD idata keySetRef;

	keySetRef = keySet_getKeySet(MEMtPN8(sdDataObj), keyVersion);

	if(REF_NULL == keySetRef)
	{
		return ADDR_IS_NULL;
	}

	return locateObjectInMemory(keySetRef);
}
/*
*@function :MasterDelete_VerifyDeleteToken
*
*@param    :BYTE deleteProfileIndex
*			BYTE len            ----  tokenSource的长度
*			BYTE* tokenSource   ----  计算Token值的数据源, 并没有进行Padding，其格式为 P1 + P2 + Length + Data field
*			BYTE offsetToken    ----  Token的长度域在RecData中的偏移
*    		BYTE *b6Data        ----  B6的数据域
*           BYTE b6Len          ----  B6的有效数据长度
*
*@return   :TRUE/FALSE
*
*@see      :Delete Token, calculated as defined in GlobalPlatform Card Specification Amendment D [10] , provided by the SM-DP
*
*@authro   :xuyz
*
*@history  :

*/
BYTE code RID_ISDR[] = {0xA0,0x00,0x00,0x05,0x59};

BYTE MasterDelete_VerifyDeleteToken(BYTE deleteProfileIndex, BYTE len, BYTE* tokenSource,  BYTE offsetToken,BYTE *b6Data,  BYTE b6Len)
{
	BYTE idata tag45Len, tag42Len, tag5F20len;
	WORD idata para42Off,para45Off,para5F20Off;
	BYTE xdata tempKey[0x10];
	UWORD xdata sdDataObj, keySetObj;
	BYTE idata bKeyType;

	para42Off = ASN1_FindTagTLV((UWORD)b6Data , b6Len, 0x42);        // 暂时不支持超过1字节的长度

	para45Off = ASN1_FindTagTLV((UWORD)b6Data , b6Len, 0x45);        // 暂时不支持超过1字节的长度

	para5F20Off = ASN1_FindTagTLV((UWORD)b6Data , b6Len, 0x5F20);    // 暂时不支持超过1字节的长度

	sdDataObj = GetsdDataObjByProfileIndex(deleteProfileIndex, ISDP_APPID);

	tag42Len = GetSDGeneraldata(tempBuf, 0x42, sdDataObj, sizeof(tempBuf), 0x00);

	if((FALSE == tag42Len) || (tag42Len != b6Data[para42Off+1]) || (0 != NvmCmp(&b6Data[para42Off + 2],tempBuf,tag42Len)))
	{
		throw_error(SW_WRONG_DATA);
	}

	tag45Len = GetSDGeneraldata(tempBuf, 0x45, sdDataObj, sizeof(tempBuf), 0x00);

	if((FALSE == tag45Len) || (tag45Len != b6Data[para45Off+1]) || (0 != NvmCmp(&b6Data[para45Off + 2],tempBuf,tag45Len)))
	{
		throw_error(SW_WRONG_DATA);
	}

	tag5F20len = GetSDGeneraldata(tempBuf, 0x5F20, sdDataObj, sizeof(tempBuf), 0x00);

	if(tag5F20len == FALSE)   // If the value of tag 5F20 is not set by the SM-DP, the default value shall be the value of the RID of ISD-P defined in section 2.2.3.
	{
		if((b6Data[para5F20Off + 2] != 5) || (0 != ecpMem_Compare(&b6Data[para5F20Off + 3],(BYTE *)RID_ISDR,5)))
		{
			throw_error(SW_WRONG_DATA);
		}

		ecpMem_Copy(&b6Data[para5F20Off + 3], (BYTE *)RID_ISDR,5);
	}
	else
	{
		if((tag5F20len != b6Data[para5F20Off + 2]) || (0 != NvmCmp(&b6Data[para5F20Off + 3],tempBuf,tag5F20len)))
		{
			throw_error(SW_WRONG_DATA);
		}
	}

	keySetObj = GetKeyDataAddr(sdDataObj, TOKEN_KEY_VERSION);

	if(ADDR_IS_NULL == keySetObj)
	{
		return FALSE;         //如果当前sdID没有put token key应该如果操作
	}

	bKeyType = keySet_getKeyType(MEMtPN8(keySetObj), CONTENT_KEY_ID,THROW_ERROR);

	if(bKeyType == keyType_AES) //<SGP02> This function shall use the ISD-P token verification key(AES key with key version number ‘70’ and key identifier ‘01’
	{
		if(0x10 != RecData[offsetToken])  // <GPC_2.2_D_SCP03_v1.1.1.pdf> 8.2  Tokens for AES:   The length of the token shall be 16 bytes.
		{
			throw_error(SW_WRONG_DATA);
		}

		bKeyType = keyData_getKeyDataById(MEMtPN8(keySetObj), KEY_ID_01, keyType_AES, tempKey);

		if((bKeyType != 0x10) || (FALSE == Cmac(tempKey, 0x10, tokenSource, len, tempBuf, 0x00, tempKey))) //初始向量传入NULL，测试用例：M2M版UL套件《TC_ES5_MD_1_MasterDelete_SMS_01_Nominal_Case_17_00_38》
		{
			return FALSE;
		}

		if(ecpMem_Compare(tempKey, &RecData[offsetToken+1], 0x10) == EQUAL)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	
return TRUE;
}
/*
*@function :IsISDP_R_Tar
*
*@descreption: 检查当前需要比较的TAR是不是ISD-P的TAR
			
*
*@param    :BYTE* buf  ,传入的是3字节TAR
*
*@return   :TRUE/FALSE
*
*@see      :
*
*@authro   :lgy
*
*@history  :V1.0@20190429
*          :V1.1  modify by xuxiaojun@20190505 增加buf是否与ECASD的TAR冲突的判断 <WQ_Offline_Function_Test/TC_TAR_Allocation_Not_ISDR_ECASD_ISDP_TARS_Error_01>
*          :V1.2  mpdify by lgy@20200727 去掉是否与ECASD ISDR的TAR冲突的判断，单独封装函数 ，修改该函数返回值 返回TAR的profileIndex
*/
BYTE IsISDP_Tar(BYTE* buf, BYTE mode)
{
	BYTE idata i,j;
	BYTE xdata isdpTar[10];
	UWORD      X_A    sdObj;
	jref       X_A    SDdataRef;
	memref 	   X_A    sdDataObj,sdTarBuffer;
	
	for(i = 0; i < PROFILE_MAX_NUMBER; i++)
	{
		if(VMReadByte(PROFILE_VALIDTAG_CFG + i) == PROFILE_INVALIDFLAG)
		{
			continue;
		}
		
		if((mode == INSTALL_CHECK_ISDPTAR)&&(i == bRamBuffer[LOADING_PROFILE_INDEX]))//安装时 还没创建ISDP对象
		{
			continue;
		}

		sdObj = GetISDPObjectAddr(i);

		SDdataRef 	= memReadU16(get_object_field_ref(MEMtPF8(sdObj), sdData_INDEX));//sdData_getSdDataRef(sdObj);

		if (REF_NULL == SDdataRef)
		{
			continue;
		}

		sdDataObj = locateObjectInMemory(SDdataRef);

		sdTarBuffer = sdData_getSDTarbufferAddr((PX8)sdDataObj);

		if(sdTarBuffer == ADDR_IS_NULL)//没有TAR
		{
			continue;
		}

		VMReadBuf(&isdpTar[0],sdTarBuffer + sdTarNumber_offset,sizeof(isdpTar));
		for(j = 0;j < isdpTar[0];j++)//比较TAR值是否有相同的 ，TAR已存在 不允许安装
		{
			if(ecpMem_Compare(&isdpTar[1+j*3],&buf[0],3) == EQUAL)
			{
				return i;
			}
		}
	}		

	return ILLEGAL_PROFILEINDEX;
}
/*检查TAR是否与ISDR ECASD的tar一致*/
BYTE IsISDR_ECASD_Tar(BYTE* buf)
{
	if((buf[0] == 0x00)&&(buf[1] == 0x00)&&(buf[2] == 0x01))//ISD_R_TAR = [00 00 01]
	{
		return TRUE;
	}

	if((buf[0] == 0x00)&&(buf[1] == 0x00)&&(buf[2] == 0x02))//ECASD TAR = [00 00 02]  <SGP.02_v3.2_updated.pdf> ‘00 00 02’ ECASD Application. Used as the TAR
	{
		return TRUE;
	}

	return FALSE;
}
/*
*@function :ProcessISDPTarInfo
*
*@descreption: 保存ISD_P的TAR和MSL ，如果安装参数中没有EA /CA ，TAR值由AID的13~15字节决定 ，
				MSL按照 102226 8.2.1.3.2.4.1 MSL Parameter   的Minimum SPI1处理
*
*@param    :BYTE* buf  ,传入的是G_APDUBuffer
*
*@return   :void 
*
*@see      :
*
*@authro   :lgy
*
*@history  :V1.0
*@history  :V1.1 by lgy  @20200727  ISDP的TAR存在SecurityDomainData 与普通SD一致 ，TAR不存在EA CA参数中时  使用AID中的特定字节作为TAR

*/
WORD ProcessISDPTarInfo(BYTE * buf,BYTE ToolkitAppSpecificParaOffset,BYTE ToolkitAppSpecificParaLgth,BYTE  tagFlag)
{
	BYTE idata executableModuleOffset,instanceAidOffset;
	BYTE idata tagNum;
	BYTE idata miniSpi,tmpLen;
	BYTE xdata AID_TAR[3];

	executableModuleOffset =  APDUBuffer[5]+6;  //point to Length of Executable Module AID

	instanceAidOffset = executableModuleOffset + APDUBuffer[executableModuleOffset]+1;	  //point to Length of Application AID

	instanceAidOffset++;

	ecpMem_Copy(&AID_TAR,&APDUBuffer[instanceAidOffset + 12],3);

	ecpMem_Set(tempBuf, 0x00, sizeof(tagRAMAPDUTarsInfo));  //清空特定文件的某一记录//ecpMem_Set(&tempBuf[0],0, PROFILE_ISDPTAR_TableRecSize); // tar num 1字节 + 最多9字节tar + msl 1字节

	tempBuf[sdAppId_offset] = ISDP_APPID;

	if((tagFlag&0x02) == 0x02) //先处理CA tag中Length of Access Domain field ，使其和EA tag中的数据对齐
	{
		tmpLen = buf[ToolkitAppSpecificParaOffset ++];  //point to Length of Access Domain field

		ToolkitAppSpecificParaOffset += tmpLen;  //point to Priority level of the Toolkit application instance

		ToolkitAppSpecificParaLgth -= (tmpLen + 1);  //CA tag中除AccessDomain外的剩余数据长度
	}
																		//CA & EA都不存在  用AID的13~15字节作为tar ，MSL设为0
	if( (ToolkitAppSpecificParaLgth >= 12)&&((tagFlag &0x03) != 0x00) ) //102226 8.2.1.3.2.2.1章节中 数据格式，至少存在1个tar的情况下 需要满足该条件
	{
		tmpLen = buf[ToolkitAppSpecificParaOffset +3];//Maximum number of menu

		ToolkitAppSpecificParaOffset += (4 + 2*tmpLen);//point to  Maximum number of channels for this application instance

		if(ToolkitAppSpecificParaLgth >= (2*tmpLen + 12))
		{
			ToolkitAppSpecificParaLgth -= (4 + 2*tmpLen);

			ToolkitAppSpecificParaOffset++;  //Length of Minimum Security Level field
			ToolkitAppSpecificParaLgth--;

			if((buf[ToolkitAppSpecificParaOffset] != 2)||(buf[ToolkitAppSpecificParaOffset+1] != 0x01))
			{
				return 0x6A80;
			}

			miniSpi = buf[ToolkitAppSpecificParaOffset+2]; //缓存spi值 1个字节

			ToolkitAppSpecificParaOffset += 3; //Length of TAR Value(s) field
			ToolkitAppSpecificParaLgth -= 3;

			if(buf[ToolkitAppSpecificParaOffset] != 0)//TAR 个数不为0
			{
				if(buf[ToolkitAppSpecificParaOffset] > 3)//最多支持3个tar
				{
					return (0x6A80);
				}
				
				ecpMem_Copy(&tempBuf[sdTarNumber_offset],&buf[ToolkitAppSpecificParaOffset],buf[ToolkitAppSpecificParaOffset]*3 + 1);
			}
			
		}
	}
	
	if(tempBuf[sdTarNumber_offset] == 0)
	{
		tempBuf[sdTarNumber_offset] = 1; // 用AID的13~15字节作为tar ，MSL设为0
		
		ecpMem_Copy(&tempBuf[sdTarValue_offset],&AID_TAR[0],3);	//拷贝AID的13-15字节作为tar ，AID一定是16字节
	}
	
	for(tagNum = 0;tagNum < tempBuf[sdTarNumber_offset];tagNum++)//检查TAR值是否与其他ISD_P重复 ，同时也不能和ISD_R的TAR重复
	{
		if(IsISDP_Tar(&tempBuf[sdTarValue_offset + tagNum*3],INSTALL_CHECK_ISDPTAR) != ILLEGAL_PROFILEINDEX)
		{
			return (0x6A80);
		}
		
		if(IsISDR_ECASD_Tar(&tempBuf[sdTarValue_offset + tagNum*3]) != FALSE)
		{
			return (0x6A80);
		}

	}
	
	tempBuf[sdMslValue_Offset] = miniSpi;
	
	SaveSDTarTemp(tempBuf);//VMWriteBuf(PROFILE_ISDPTAR_TABLE + profileIndex*PROFILE_ISDPTAR_TableRecSize,tempBuf,PROFILE_ISDPTAR_TableRecSize);

	return 0x9000;
}

/*
*@function   : StoreGeneraldataForISD_P
*
*@description:处理GP 指令中的Store data命令,仅针对ISD-P，目前只支持42 45 5F20这3个Tag ,这3个tag在MasterDelete的时候会使用于Token计算
*
*@param      :BYTE profileIndex
*
*@return     :BYTE  TRUE/FALSE 执行指令成功/失败
*
*@author     :lgy
*
*@history    :V1.0:create by lgy@20190426	
*            :V1.1:modify by xuxiaojun@20190430  增加附录B参数的存储
*            :V1.2:modify by lgy@20200722  该函数只给ISDP使用 去掉一个参数
*/
void StoreGeneraldataForISD_P(BYTE profileIndex)
{
	WORD idata sdRef;
	PN8				sdDataObj	;
	UWORD    xdata     profileEntry,appTableAddr,theAppEntry;

	if(((P1&0x80) != 0x80)&&(P2 != 0x00))
	{
		throw_error(SW_WRONG_P1P2);
	}

	if(((RecData[0] == 0x00)&&(RecData[1] == 0x70))||(RecData[0] == 0x85)||(RecData[0] == 0xA5)||(RecData[0] == 0x42)||(RecData[0] == 0x45)||((RecData[0] == 0x5F)&&(RecData[1] == 0x20))) //处理SD相关数据
	{

		profileEntry = GetProfileEntryAddr(profileIndex);

		appTableAddr = GetProfile_theAppTableValueAddr(profileEntry);

		theAppEntry = locateObjectInMemory(PN_R_U16(U32_A_U16(appTableAddr, U16_M_U16(ISDP_APPID, SIZE_REF))));

		sdRef = PN_R_U16(PFtMEM(get_object_field_ref(MEMtPF8(theAppEntry), theApplet_INDEX)));

		sdDataObj  = MEMtPN8(locateObjectInMemory(sdData_getSDdataRef(sdRef)));

		dataManager_storeData(sdDataObj);
	}
	else
	{
		throw_error(SW_WRONG_DATA);
	}
}
#if M2M_ESx
/*
*@function   : M2MLocalEnableEmergencyProfile
*
*@description: This function is used by the Device to locally enable the Emergency Profile.
*@: 			The eUICC SHALL NOT send notifications to the SM-SR.
*@: 			The Emergency Profile SHOULD remain enabled even after a restart of the Device.
*@: 			It is up to the Device to disable the Emergency Profile.
*
*@param      :
*
*@return     result
*@return     success (0),
*@return     errorProfileRef (8), -- Emergency/Test Profile does not exists
*@return     errorAlreadyEnabled (9), -- Emergency/Test Profile is already enabled
*@return     errorAlreadyDisabled (10), -- Emergency/Test Profile is already disabled
*@return     errorForbidden (12), -- Transition is forbidden for the Test Profile
*@return     undefinedError (127):
*
*@author     :chentao
*
*@history    :V1.0 :create @chentoa20220928
*/
BYTE M2MLocalEnableEmergencyProfile(void)
{
	BYTE idata  profileIndex, transactionTag;

	//A Profile with the Emergency Profile Attribute set exists on the eUICC.
	profileIndex = JcvmConfig_GetM2MEmergencyProfileIndex();

	if(profileIndex == ILLEGAL_PROFILEINDEX)//Profile with the Emergency Profile Attribute is not exist
	{
		return  Error_ProfileRef;
	}
	//The Profile with the Emergency Profile Attribute set is not already enabled
	if(GetProfileState(profileIndex) == PROFILE_STATE_ENABLED)
	{
		return  Error_AlreadyEnabled;
	}

    // TODO：当前激活的profile POL 不能 disable 是否需要检查？？暂时忽略

	transactionTag = IF_BeginTransaction();

	//JcvmConfig_SetM2MFallBackDisabledNoAllowProfileIndex(ILLEGAL_PROFILEINDEX);   // M2MFallBackDisabledNoAllowProfileIndex,对于该字段而言，如果M2MFallBackDisabledNoAllowProfileIndex 有有效数据，则执行到这里，该字段要被清空成FF ，如果该字段本来就没数据，那么其值本来就是FF

	JcvmConfig_SetM2MLastDisableProfileIndex(G_CurEnabledProfileIndex);               //M2MLastDisableProfileIndex

	//JcvmConfig_SetM2MNotificationType(NOTIFICATION_RROFILE_CHANGE_SUCCEEDED);

	JcvmConfig_SetUpdataEnableProfileIndex(profileIndex);//M2MUpdateEnableProfileIndex

	JcvmConfig_SetUpdateChangeProfileInitPrivateFileIndex(PROFILE_PrivateFileInit);

	IF_M2M_SetRefreshFlag();

	IF_CommitTransaction(transactionTag);

	return Succsss;
}

/*
*@function   : M2MLocalDisableEmergencyProfile
*
*@description: This function is used by the Device to locally disable the Emergency Profile and enable the previously enabled Profile.
*
*@param      :
*
*@return     result
*@return     success (0),
*@return     errorProfileRef (8), -- Emergency/Test Profile does not exists
*@return     errorAlreadyEnabled (9), -- Emergency/Test Profile is already enabled
*@return     errorAlreadyDisabled (10), -- Emergency/Test Profile is already disabled
*@return     errorForbidden (12), -- Transition is forbidden for the Test Profile
*@return     undefinedError (127):
*
*@author     :chentao
*
*@history    :V1.0 :create @chentoa20220928
*/
BYTE M2MLocalDisableEmergencyProfile(void)
{
	BYTE idata  profileIndex, transactionTag;

	//A Profile with the Emergency Profile Attribute set exists on the eUICC.
	profileIndex = JcvmConfig_GetM2MEmergencyProfileIndex();

	if(profileIndex == ILLEGAL_PROFILEINDEX)//Profile with the Emergency Profile Attribute is not exist
	{
		return  Error_ProfileRef;
	}
	//The Profile with the Emergency Profile Attribute set is enabled
	if(GetProfileState(profileIndex) == PROFILE_STATE_DISABLED)
	{
		return  Error_AlreadyDisabled;
	}
	 // TODO：当前激活的profile POL 不能 disable 是否需要检查？？暂时忽略

	transactionTag = IF_BeginTransaction();

	//JcvmConfig_SetM2MFallBackDisabledNoAllowProfileIndex(ILLEGAL_PROFILEINDEX); // M2MFallBackDisabledNoAllowProfileIndex,对于该字段而言，如果M2MFallBackDisabledNoAllowProfileIndex 有有效数据，则执行到这里，该字段要被清空成FF ，如果该字段本来就没数据，那么其值本来就是FF(主动disableProfile，这个字段应该要清成FF)

	JcvmConfig_SetM2MNotificationType(NOTIFICATION_FIRST_PROFILE_CHANGE_AFTER_EMERGERCY_PROFILE_DISABLE);//  NOTIFICATION_FIRST_PROFILE_CHANGE_AFTER_EMERGERCY_PROFILE_DISABLE M2MNotificationType

	JcvmConfig_SetUpdataEnableProfileIndex(JcvmConfig_GetM2MLastDisableProfileIndex());//enable the previously enabled Profile

	JcvmConfig_SetUpdateChangeProfileInitPrivateFileIndex(PROFILE_PrivateFileInit);

	IF_M2M_SetRefreshFlag();

	IF_CommitTransaction(transactionTag);

	return Succsss;
}

/*
*@function   : M2MLocalEnableTestProfile
*
*@description: This function is used by the Device to locally enable the Test Profile
*
*@param      :
*
*@return     result
*@return     success (0),
*@return     errorProfileRef (8), -- Emergency/Test Profile does not exists
*@return     errorAlreadyEnabled (9), -- Emergency/Test Profile is already enabled
*@return     errorAlreadyDisabled (10), -- Emergency/Test Profile is already disabled
*@return     errorForbidden (12), -- Transition is forbidden for the Test Profile
*@return     undefinedError (127):
*
*@author     :chentao
*
*@history    :V1.0 :create @chentoa20221009
*/
BYTE M2MLocalEnableTestProfile(void)
{
	BYTE idata  testProfileIndex, transactionTag;

	//A Test Profile exists on the eUICC
	testProfileIndex = JcvmConfig_GetM2MTestProfileIndex();

	if(testProfileIndex == ILLEGAL_PROFILEINDEX)//Test Profile does not exists
	{
		return  Error_ProfileRef;
	}
	//The Test Profile is not already enabled
	if(GetProfileState(testProfileIndex) == PROFILE_STATE_ENABLED)
	{
		return  Error_AlreadyEnabled;
	}

	// TODO：当前激活的profile POL 不能 disable 是否需要检查？？暂时忽略

	transactionTag = IF_BeginTransaction();

	//JcvmConfig_SetM2MFallBackDisabledNoAllowProfileIndex(ILLEGAL_PROFILEINDEX);   // M2MFallBackDisabledNoAllowProfileIndex,对于该字段而言，如果M2MFallBackDisabledNoAllowProfileIndex 有有效数据，则执行到这里，该字段要被清空成FF ，如果该字段本来就没数据，那么其值本来就是FF

	JcvmConfig_SetM2MLastDisableProfileIndex(G_CurEnabledProfileIndex);               //M2MLastDisableProfileIndex

	//JcvmConfig_SetM2MNotificationType(NOTIFICATION_RROFILE_CHANGE_SUCCEEDED);

	JcvmConfig_SetUpdataEnableProfileIndex(testProfileIndex);//M2MUpdateEnableProfileIndex

	JcvmConfig_SetUpdateChangeProfileInitPrivateFileIndex(PROFILE_PrivateFileInit);

	IF_M2M_SetRefreshFlag();

	IF_CommitTransaction(transactionTag);

	return Succsss;
}
/*
*@function   : M2MLocalDisableTestProfile
*
*@description: This function is used by the Device to locally disable the Test Profile and enable the previously enabled Profile
*
*@param      :
*
*@return     result
*@return     success (0),
*@return     errorProfileRef (8), -- Emergency/Test Profile does not exists
*@return     errorAlreadyEnabled (9), -- Emergency/Test Profile is already enabled
*@return     errorAlreadyDisabled (10), -- Emergency/Test Profile is already disabled
*@return     errorForbidden (12), -- Transition is forbidden for the Test Profile
*@return     undefinedError (127):
*
*@author     :chentao
*
*@history    :V1.0 :create @chentoa20221009
*/
BYTE M2MLocalDisableTestProfile(void)
{
	BYTE idata  testProfileIndex, transactionTag;

	//A Test Profile exists on the eUICC.
	testProfileIndex = JcvmConfig_GetM2MEmergencyProfileIndex();

	if(testProfileIndex == ILLEGAL_PROFILEINDEX)//Profile with the Emergency Profile Attribute is not exist
	{
		return  Error_ProfileRef;
	}
	//The Test Profile is enabled
	if(GetProfileState(testProfileIndex) == PROFILE_STATE_DISABLED)
	{
		return  Error_AlreadyDisabled;
	}
	 // TODO：当前激活的profile POL 不能 disable 是否需要检查？？暂时忽略

	transactionTag = IF_BeginTransaction();

	//JcvmConfig_SetM2MFallBackDisabledNoAllowProfileIndex(ILLEGAL_PROFILEINDEX); // M2MFallBackDisabledNoAllowProfileIndex,对于该字段而言，如果M2MFallBackDisabledNoAllowProfileIndex 有有效数据，则执行到这里，该字段要被清空成FF ，如果该字段本来就没数据，那么其值本来就是FF(主动disableProfile，这个字段应该要清成FF)

	JcvmConfig_SetM2MNotificationType(NOTIFICATION_FIRST_PROFILE_CHANGE_AFTER_TEST_PROFILE_DISABLE);//  NOTIFICATION_FIRST_PROFILE_CHANGE_AFTER_EMERGERCY_PROFILE_DISABLE M2MNotificationType

	JcvmConfig_SetUpdataEnableProfileIndex(JcvmConfig_GetM2MLastDisableProfileIndex());//enable the previously enabled Profile

	JcvmConfig_SetUpdateChangeProfileInitPrivateFileIndex(PROFILE_PrivateFileInit);

	IF_M2M_SetRefreshFlag();

	IF_CommitTransaction(transactionTag);

	return Succsss;
}
#endif  //end of M2M_ESx
#endif //end of #if M2M_EUICC
