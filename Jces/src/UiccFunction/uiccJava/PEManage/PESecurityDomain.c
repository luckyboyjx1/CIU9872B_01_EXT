
/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   PESecurityDomain.c
*
* Description: 该文件主要是对PE-SecurityDomain的所有处理
*
* auther：     zxx    20170623
*
* history：    V1.0
*
************************************************************************************************************/


#include  "typedef.h"
#include  "fileAction.h"
#include  "tlvHeader.h"
#include  "ProfileInterpreter.h"
#include "profileApi.h"
#include "gsmpublic.h"
#include "ProfileGPCommand.h"
#include  "VMAccessEeprom.h"
#include  "platform_api.h"
#include  "ob_manager.h"
#include  "SecurityDomainData.h"
#include  "keyGroup.h"
#include  "Uicc_interface.h"
#if (EUICC_ONLY||SE_AND_EUICC)
/*********************PE-SecurityDomain START*****************************/
	
////ApplicationInstance, -- see section 8.7.3
//BYTE code SDapplicationInstTag []= {0xA1};
////[APPLICATION 15] ApplicationIdentifier,
//BYTE code SDapplicationLoadPackageAIDTag []= {0xA1,0x4F};
////[APPLICATION 15] ApplicationIdentifier,
//BYTE code SDclassAIDTag []= {0xA1,0x4F};
//// [APPLICATION 15] ApplicationIdentifier,
//BYTE code SDinstanceAIDTag []= {0xA1,0x4F};
////[APPLICATION 15] ApplicationIdentifier OPTIONAL,
//BYTE code SDextraditeSecurityDomainAIDTag []= {0xA1,0x4F};
////[2] OCTET STRING,
//BYTE code SDapplicationPrivilegesTag []= {0xA1,0x82};
////[3] OCTET STRING (SIZE(1)) DEFAULT '07'H,
//BYTE code SDlifeCycleStateTag []= {0xA1,0x83};
///* Coding according to GP Life Cycle State.  */
////[PRIVATE 9] OCTET STRING,
//BYTE code SDapplicationSpecificParametersC9Tag []= {0xA1,0xC9};
///***************************************************************************************************/
//// [PRIVATE 15] ApplicationSystemParameters OPTIONAL,
//BYTE code SDsystemSpecificParametersTag []= {0xA1,0xEF};
////[PRIVATE 7] OCTET STRING OPTIONAL,
//BYTE code SDvolatileMemoryQuotaC7Tag []= {0xA1,0xEF,0xC7};
//[PRIVATE 8] OCTET STRING OPTIONAL,
//BYTE code nonVolatileMemoryQuotaC8Tag []= {0xA1,0xEF,0xC8};
//[PRIVATE 11] OCTET STRING OPTIONAL,
//BYTE code globalServiceParametersTag []= {0xA1,0xEF,0xCB};
//[PRIVATE 15] OCTET STRING OPTIONAL,
//BYTE code implicitSelectionParameterTag []= {0xA1,0xEF,0xCF};
//[PRIVATE 23] OCTET STRING OPTIONAL,
//BYTE code SDvolatileReservedMemoryTag []= {0xA1,0xEF,0xD7};
//[PRIVATE 24] OCTET STRING OPTIONAL,
//BYTE code SDnonVolatileReservedMemoryTag []= {0xA1,0xEF,0xD8};
//[PRIVATE 10] OCTET STRING OPTIONAL,
//BYTE code ts102226SIMFileAccessToolkitParameterTag []= {0xA1,0xEF,0xCA};//TODO
//[0] TS102226AdditionalContactlessParameters OPTIONAL,
//BYTE code ts102226AdditionalContactlessParametersTag []= {0xA1,0xEF,0xB0};//TODO

/* Parameters for contactless applications encoded according to TS 102 226 */
//OCTET STRING 
//BYTE code protocolParameterDataTag []= {0xA1,0xEF,0xB0,0x04};//TODO

/* Coded according to Contactless Protocol Parameters Structure as defined in GP Amd. C */
//[PRIVATE 25] OCTET STRING OPTIONAL, 
//BYTE code contactlessProtocolParametersTag []= {0xA1,0xEF,0xD9};
/* Coded according to User Interaction Parameters Structure as defined in GP Amd. C */
// [PRIVATE 26] OCTET STRING OPTIONAL  
//BYTE code SDuserInteractionContactlessParametersTag []= {0xA1,0xEF,0xDA};

/***************************************************************************************************/

	//[PRIVATE 10] UICCApplicationParameters OPTIONAL,
//BYTE code SDapplicationParametersTag []= {0xA1,0xEA};

////[0] OCTET STRING OPTIONAL,
//BYTE code SDuiccToolkitApplicationSpecificParametersFieldTag []= {0xA1,0xEA,0x80};
////[1] OCTET STRING OPTIONAL,
//BYTE code SDuiccAccessApplicationSpecificParametersFieldTag []= {0xA1,0xEA,0x81};
//// [2] OCTET STRING OPTIONAL
//BYTE code SDuiccAdministrativeAccessApplicationSpecificParametersFieldTag []= {0xA1,0xEA,0x82};

////SEQUENCE OF OCTET STRING OPTIONAL  LIST... ?TAG ????
//BYTE code processSDDataListTag []= {0xA1,0x30};

//BYTE code processSDDataTag []= {0xA1,0x30,0x04};

///***************************************************************************************************/
////SEQUENCE OF KeyObject OPTIONAL, -- see section 8.6.3
//BYTE code theSecurityDomainPEKeyObjectTag []= {0xA2};
////LIST... ?TAG ????
//BYTE code KeyObjectListTag []= {0xA2,0x30};
//// [21] OCTET STRING (SIZE (1)),
//BYTE code keyUsageQualifierTag []= {0xA2,0x30,0x95};
////[22] OCTET STRING (SIZE (1)) DEFAULT '00'H,
//BYTE code keyAccessTag []= {0xA2,0x30,0x96};
////[2] OCTET STRING (SIZE (1)),
//BYTE code keyIdentifierTag []= {0xA2,0x30,0x82};
////[3] OCTET STRING (SIZE (1)),
//BYTE code keyVersionNumberTag []= {0xA2,0x30,0x83};
//// [5] OCTET STRING OPTIONAL,
//BYTE code keyCounterValueTag []= {0xA2,0x30,0x85}; //TODO
////LIST... ?TAG ????
//BYTE code keyCompontentsListTag []= {0xA2,0x30,0x30};
////LIST... ?TAG ????
//BYTE code keyCompontentsTag []= {0xA2,0x30,0x30,0x30}; 
//// [0] OCTET STRING, 
//BYTE code keyTypeTag []= {0xA2,0x30,0x30,0x30,0x80};
//// [6] OCTET STRING,
//BYTE code keyDataTag []= {0xA2,0x30,0x30,0x30,0x86};
//// [7] UInt8 DEFAULT 8
//BYTE code macLengthTag []= {0xA2,0x30,0x30,0x30,0x87};//TODO

//// SEQUENCE OF OCTET STRING OPTIONAL -- see section 8.6.4	
////LIST... ?TAG ????
//BYTE code theSecurityDomainPEsdPersoDataTag []= {0xA3};



//putkey指令的格式
//form1
// New version number(1)
// Key type(1)
// Length of key or key component data(1~3)
// Key or key component data value(1~n)
// Length of key check value (1)
// Key check value(0~n)

//form2
//New version number(1)
//Key type(2)
//Length of key or key component data(1~3)
//Key or key component data value(1~n)
//Length of key check value (1)
//Key check value(0~n)

//Length of key usage qualifier(1)
//Key usage qualifier(0~n)
//Length of key access(1)
//key access(0~n)

//注：当存在mac时，Key or key component data value的格式如下：
//Length of the key in bytes (1)
//Ciphered key (16/32)
//Length of the MAC in bytes(1)

/*
*@funtion    :IF_SupportSomeKeyType
*
*@description: 是否支持传进的keytype
*
*@param      :BYTE KeyType
*
*@return     :BYTE   TRUE/FALSE
*
*@history    :V1.0 :create by xieyong@20240617
*/
BYTE IF_SupportSomeKeyType(BYTE KeyType)
{
	BYTE idata flag;

	flag = 0;

#if !GP__KEY_MANAGER__PUT_RSA_KEY
	if((KeyType >= 0xA0) && (KeyType <= 0xA8))   // A0 - A8
	{
		flag = 1;
	}
#endif

	if(flag == 1)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/*
*@funtion    :makePutKey
*
*@description: 根据GP2.2.1规范，拼装标准的putKey指令到APDUBuffer中
*
*@param      :BYTE * keyTmp: tmp[0] as keyUsageQualifier, tmp[1] as keyAccess, tmp[2] as keyIdentifier, tmp[3] as keyVersionNumber/最大三组
*@param      :WORD   keyCompontentLen 总长度
*@param      :BYTE  *keyCompontentData ：类型为TYPE KEYValueLen KeyValue ... ... 
*
*@return     :WORD   statusword
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*/
WORD makePutKey(BYTE keyId,BYTE keyVersion,WORD keyCompontentLen, BYTE  *keyCompontentData)
{
	WORD idata curOff;
	
	if(keyCompontentLen == 0)
	{
		return 0x9000;
	}
	
	if(keyVersion == 0)//modify by zhxx@20230413,德国商用测试：1、BugList_23_14，keyversion是0希望下载成功
	{
		return 0x9000;
	}
	
	curOff = 0;
	
	RecData[curOff++] = keyVersion;

	ecpMem_Copy(&RecData[curOff], &keyCompontentData[0], keyCompontentLen);  //拷贝数据

#if !GP__KEY_MANAGER__PUT_RSA_KEY
	if(IF_SupportSomeKeyType(RecData[1]) == FALSE)
	{
		return 0x9000;
	}
#endif
		
	curOff += keyCompontentLen;	
	
	CLA = (0x80 | selected_channel);
	
	INS = 0xD8;
	
	P1 = 0x00;  //GP221: A value of'00' indicates that a new key or group of keys is being added
	
	P2 = keyId; //tmp[2] as keyIdentifier
	
	P3 = curOff;   //拼装的APDU 数据长度
	
	//macLength: For AES KID keys, indicates the length of the MAC in bytes as defined in TS 102 226 [102 226]. This value shall be ignored for other key types
	
	return IF_ExcuteProfileGPCommand(G_profileGPState.peInstallAppId);
}



/*
*@funtion    :CorrectKeyId
*
*@description: 德国商用profile测试，BugList_23_11，第一支key和第三支key的keyIdentifier都是01希望下载成功，代码修改，兼容这种出错情况，将相同的keyid纠正之后再次执行putkey指令
*
*@param      :WORD sw: 执行putkey指令返回的状态字
*@param      :BYTE keyId ：执行putkey指令的keyid
*@param      :BYTE keyVersion：执行putkey指令的keyVersion
*
*@return     :WORD   sw
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20230417
*/
WORD CorrectKeyId(WORD sw,BYTE keyId,BYTE keyVersion)
{
	WORD sendLength,sdRef;
	BYTE idata correctKeyId,scpType,i;
	WORD  keySetRef;
	BYTE xdata tempData[KEY_INFO_LENGTH_FORMAT2*3 + 4*3];//最大长度为每支key的长度*KEY_INFO_LENGTH_FORMAT2 + 扩展字节，存储获取的keyinfo，C004xxxxxxxx/C00Axx......
	BYTE idata c0Offset[3];//记录三支key在获取的keyinfo中，长度的偏移
	PN8	sdDataObj,keySetObj;
	u32  D_A  appEntryAddr;

	ecpMem_Set(c0Offset,0,sizeof(c0Offset));


	if((sw == 0x6A80)&&(IsSCP02Key(keyVersion) || IsSCP80Key(keyVersion)))
	{
		scpType = getSCPTypeByKeyVersion(keyVersion);//获取scp类型02/80

		appEntryAddr = getEntryByAppId(G_profileGPState.peInstallAppId);

		if(appEntryAddr == ADDR_IS_NULL)
		{
			return sw;
		}

		sdRef = memReadU16(MEMtPM8(get_object_field_ref(appEntryAddr, theApplet_INDEX)));

		sdDataObj = MEMtPN8(locateObjectInMemory(sdData_getSDdataRef(sdRef)));

		//sendLength	= sdData_getKeyInfo(tempBuf, 250, sdDataObj, &context);//C004  /  C00A

		keySetRef = keyGroup_getKeySet(sdDataObj, scpType, keyVersion);

		if(keySetRef == REF_NULL)
		{
			return sw;
		}

		keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));

		sendLength = writeKeyInfo(tempData, sizeof(tempData), keySetObj, 0);//存储获取的keyinfo，C004xxxxxxxx/C00Axx......

		if(sendLength == 0)
		{
			return sw;
		}
		else
		{

			for(i = 0; i < 3 ; i++)
			{
				c0Offset[i] = FindTagOffset(tempData,0xC0,sendLength,i+1);

				if(c0Offset[i] == FALSE)
				{
					break;
				}
			}

			if(i == 2)//卡上已存在2支key，纠正正在put的第三支key的keyid,eg:1,2,2、1,2,4改为1.2.3;
			{
				correctKeyId = 6 - (tempData[c0Offset[0]+1]+tempData[c0Offset[1]+1]);

				P2 = correctKeyId; //将错误的keyid纠正之后再执行一遍put key操作

				return IF_ExcuteProfileGPCommand(G_profileGPState.peInstallAppId);
			}
			else//其它情况，抛原来的异常
			{
				return sw;
			}
		}
	}
	
	return sw;
	
}



/*
*@function  : KeyObjectSequenceAnalyze
*
*@description: 分析SD 中的KeyList 组件
*
*               KeyObject::= SEQUENCE 
*                { 
*                  keyUsageQualifier [21] OCTET STRING (SIZE (1)), 
*                  keyAccess [22] OCTET STRING (SIZE (1)) DEFAULT '00'H, 
*                  keyIdentifier [2] OCTET STRING (SIZE (1)), 
*                  keyVersionNumber [3] OCTET STRING (SIZE (1)), 
*                  keyCounterValue [5] OCTET STRING OPTIONAL, 
*                  keyCompontents SEQUENCE OF SEQUENCE 
*                      { keyType [0] OCTET STRING, 
*                        keyData [6] OCTET STRING, 
*                        macLength[7] UInt8 DEFAULT 8 
*                     }
*             }
*@参考规范    :《eUICC Profile Package Interoperable Format Technical Specification v2.0.pdf》 8.6.3
*              :Note: 按照ASN.1规范，SEQUENCE是一个有序集合，不考虑数据乱序过来的情况       
*
*@param       :void
*
*@rerurn      :void
*
*@author      :xuyz
*
*@history     :V1.0:create by xuyz@20170711
*             :V1.1:modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE KeyObjectSequenceAnalyze(void)
{
	BYTE idata result,i;
	BYTE xdata tmp[0x09];  //tmp[0] as keyUsageQualifier, tmp[1] as keyAccess, tmp[2] as keyIdentifier, tmp[3] as keyVersionNumber
	BYTE xdata tBuf[0x07];  //保存keyType 或KeyValue长度的临时buf
	BYTE idata totalKeyLen,keyDataLen;
	BYTE idata occTimes;//用于表示keyComponent objects出现的次数
	BYTE idata macLen;
	BYTE xdata tmpTag[0x04];
	WORD idata sw;
	
	if(G_PETlvInfo.levelIndex == 0)
	{
		setVerticalFind(&G_PETlvInfo);
	}
	
	ecpMem_Set(tmp, 0x00, sizeof(tmp));
	ecpMem_Set(tmpTag, 0x00, sizeof(tmpTag));
	
	totalKeyLen = 0;
	
	while(1)
	{			

		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
			if(G_PETlvInfo.findNextLen == 0)
			{
				break;
			}	
			else
			{
				backData( EUICCBackPEBuf,G_PETlvInfo.bufferAddr + G_PETlvInfo.findNextOff, G_PETlvInfo.bufferLen-G_PETlvInfo.findNextOff);	
				return 2;  //处理到最后一个tag
			}				
		}		
			
		if(G_PETlvInfo.isCompleteBuffer[G_PETlvInfo.levelIndex] == FALSE)//处理的tag不完整
		{
			backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr + G_PETlvInfo.findTagOff,  G_PETlvInfo.bufferLen-G_PETlvInfo.findTagOff);		
			return 2;
		}
		
		G_profileControlContext.TotalLen += G_PETlvInfo.findTLVLen;
		
		if(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] == 0x30)   //第一个 keyList 结构体
		{
				setVerticalFind(&G_PETlvInfo);       // TLV嵌套，设置垂直查找信息
			
			    IF_InitKeyCounter();
			
				while(1)
				{
					result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
					if(result == FALSE)
					{
						break;
					}										
					
					switch(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex])
					{
						case 0x95:    //keyUsageQualifier
						{	
							if((G_PETlvInfo.findValueLen != 0x01) && (G_PETlvInfo.findValueLen != 0x02))//keyUsageQualifier [21] OCTET STRING (SIZE (1..2))
							{
								appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
								return FALSE;
							}
							tmp[0] = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
							tmpTag[0] = 1;
							break;
						}
						case 0x96:  //keyAccess
						{	
							if(G_PETlvInfo.findValueLen != 0x01)//keyAccess [22] OCTET STRING (SIZE (1)) DEFAULT '00'H,
							{
								appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
								return FALSE;
							}
							tmp[1] = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
							break;
						}
						case 0x82:  //keyIdentifier
						{
							if(G_PETlvInfo.findValueLen != 0x01)//keyIdentifier [2] OCTET STRING (SIZE (1)),
							{
								appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
								return FALSE;
							}
							tmp[2] = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
							tmpTag[2] = 1;
							break;
						}
						case 0x83:  //keyVersionNumber
						{
							if(G_PETlvInfo.findValueLen != 0x01)//keyVersionNumber [3] OCTET STRING (SIZE (1)),
							{
								appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
								return FALSE;
							}
							tmp[3] = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);

							if(tmpTag[2] == 0)
							{
								appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
								return FALSE;
							}
							tmpTag[3] = 1;
							break;
						}
						case 0x85:  //keyCounterValue  ,NOTE: This field may be ignored for the keys used by the protocols SCP02, SCP03 or SCP80 that mandate an initial counter to be set to the default value right after their initialization.
						{
							if(G_PETlvInfo.findValueLen > 5)//SCP02 keyset the default value is 0000h, for SCP03 it is 000000h, for SCP80 it is 0000000000).
							{
								appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
								return FALSE;
							}
							
							VMReadBuf(&tmp[4],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff,G_PETlvInfo.findValueLen);	
							
							IF_SetKeyCounter(&tmp[4],G_PETlvInfo.findValueLen);
							
							break;
						}
						case 0x30:  //keyCompontents:  keyType [0] OCTET STRING,  keyData [6] OCTET STRING,  macLength[7] UInt8 DEFAULT 8 
						{										
							setVerticalFind(&G_PETlvInfo); 
							
							occTimes = 0;
							
							if(IsSCP03Key(tmp[3]))
							{
							}
							else if(IsSCP81Key(tmp[3]))
							{
							}
							else if((IsSCP02Key(tmp[3])) || ((tmp[0] != 0)&& (!((tmp[3] >= 0x70) && (tmp[3] <= 0x75)))))   //scp 02 密钥/keyUsageQualifier存在的情况下按照Form2进行存储,RASToken key先按照格式1进行存储
							{
								tempBuf[totalKeyLen ++] = 0xFF;
							}
							else
							{
								//白盒测试添加
							}
							tmpTag[1] = 1;
							while (1)
							{	
								result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
								
								if(result == FALSE)
								{
									break;
								}
								switch(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex])
								{
									case 0x30:
									{
										occTimes++;
										
										macLen = AES_DEFAULT_MAC_LENGTH;//macLength[7] UInt8 DEFAULT 8
										
										if(G_PETlvInfo.findValueLen < 6) //error：不存在86的情况
										{
											appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					
								            return FALSE;
										}
										
										VMReadBuf(&tBuf[0],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff,7);
										
										if((tBuf[0] != 0x80) || (tBuf[2] == 0x83))
										{
											appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					
								            return FALSE;
										}
										
										if((occTimes > 1) && (!((tBuf[2] >= 0xA0) && (tBuf[2] <= 0xA8))))//TestPurpose: 8.2.2.3.  Check number of keyComponent objects This test shall check if keyComponent is assigned just once per key(针对对称密钥：eg：DES,AES)
										{
											appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				
											return FALSE;
										}
										
										if( tBuf[4] < 0x80) 
										{
											keyDataLen = tBuf[4] ;
											
											result = 5;  //读取数据的偏移
										}
										else if(tBuf[4] == 0x81) 
										{
											keyDataLen = tBuf[5] ;
											
											result = 6;  //读取数据的偏移
										}
										else
										{
											appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
											return FALSE;//ERROR
										}
										
										tempBuf[totalKeyLen] = tBuf[2];  //保存keyType
										
										if((tBuf[2] == 0x88) || (tBuf[2] == 0x85))//AES key,eg:格式：88 11 10 11223344556677889910111213141516 || PSK key,eg:格式：85 21 20 F0C0FAAC0EF1364A3E5EB4229CF797A3752CD0C8277844576B3E05D505A03F21
										{
											//The field "length of the MAC" shall be present if "ciphered key" contains an AES key with key identifier '02' and key version '01' to '0F' or '11'
											if((IsSCP80Key(tmp[3]))&&(tBuf[2] == 0x88) && (tmp[2] == 2))//For AES KID keys, indicates the length of the MAC in bytes as defined in TS 102 226 [102 226]. This value shall be ignored by the eUICC for other key types.
											{
												tempBuf[totalKeyLen+1] = keyDataLen+2; 
											}
											else
											{
												tempBuf[totalKeyLen+1] = keyDataLen+1; 
											}
											
											tempBuf[totalKeyLen+2] = keyDataLen; 											
										
										    totalKeyLen += 3; 
										}
										else
										{
											tempBuf[totalKeyLen+1] = keyDataLen;  
										
										    totalKeyLen += 2; 
										}
																		
										//ReadEeprom(&tempBuf[totalKeyLen],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff+result,G_PETlvInfo.findValueLen-result);
										
										//totalKeyLen += G_PETlvInfo.findValueLen-result;
										//modify by zhhj 20180607  存在86后面还有87的情况，所以使用keyDataLen 
                                        //例子 3028 950138 960100 820101 830120 301A 3018 800180 8610 404142434445464748494A4B4C4D4E4F 870108
                                        VMReadBuf(&tempBuf[totalKeyLen],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff+result,keyDataLen);
										
										totalKeyLen += keyDataLen;										
										
										if(G_PETlvInfo.findValueLen-result - keyDataLen > 0)//存在87tag
										{
											if((G_PETlvInfo.findValueLen-result - keyDataLen) != 3)//macLength[7] UInt8 DEFAULT 8  
											{
												appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
												return FALSE;//ERROR
											}
											if(VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff+result+keyDataLen) != 0x87)//tag正确性检查
											{
												appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
												return FALSE;//ERROR
											}
											
											macLen = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff+result+keyDataLen+2);
										}
										
										//The field "length of the MAC" shall be present if "ciphered key" contains an AES key with key identifier '02' and key version '01' to '0F' or '11'
										if((IsSCP80Key(tmp[3]))&&(tBuf[2] == 0x88) && (tmp[2] == 2))
										{
										   tempBuf[totalKeyLen ++] = macLen;
										}
										                                        
										break;
									}
									default:
									{
										appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
										return FALSE;
									}
								}
							}
							
							if(tempBuf[0] == 0xA1)   //GP命令中RSAToken的checkValue为 00
							{
								tempBuf[totalKeyLen++] = 0x00;
							}
							else
							{
								tempBuf[totalKeyLen] = 0x03; //checkValueLen
										
								ecpMem_Set(&tempBuf[totalKeyLen+1], 0x00, 0x03 );
								
								totalKeyLen += 4;
							}
							
							if(IsSCP03Key(tmp[3]))
							{
							}
							else if(IsSCP81Key(tmp[3]))
							{
							}
							else if((IsSCP02Key(tmp[3])) || ((tmp[0] != 0)&& (!((tmp[3] >= 0x70) && (tmp[3] <= 0x75)))))   //scp 02 密钥/keyUsageQualifier存在的情况下按照Form2进行存储,RASToken key先按照格式1进行存储
							{
								tempBuf[totalKeyLen ++] = 0x01;
								
								tempBuf[totalKeyLen ++] = tmp[0];
								
								tempBuf[totalKeyLen ++] = 0x01;
								
								tempBuf[totalKeyLen ++] = tmp[1];							
							}		 
							else
							{
								//白盒测试添加
							}
													
							sw = makePutKey(tmp[2],tmp[3], totalKeyLen, tempBuf);
							
							sw = CorrectKeyId(sw,tmp[2],tmp[3]);
							
							if(processStatusWord(sw,FALSE) == FALSE)
							{
								return FALSE;
							}	
										
							ecpMem_Set(tmp, 0x00, sizeof(tmp));
						
							totalKeyLen = 0;
							
							setReturnFind(&G_PETlvInfo);

							break;
						}
						default:
							appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
							return FALSE;				
					}				
				}
				for(i = 0; i<sizeof(tmpTag); i++)	
				{
					if(tmpTag[i] == 0)
					{
						appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
						return FALSE;	
					}
				}	
				setReturnFind(&G_PETlvInfo);  //回退到上一级
						
		}
		else  //error tag
		{
			appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
			return FALSE;
		}
	}
	
	setReturnFind(&G_PETlvInfo);
	
	 return TRUE;

}

WORD makeInstallForExtradition(BYTE* instanceAID, BYTE* extraditeSDAID)
{
	WORD idata curOff ;
	WORD idata status;

	ecpMem_Copy(RecData, extraditeSDAID, extraditeSDAID[0]+1); // Security Domain AID

	curOff = (extraditeSDAID[0]+1);

	RecData[curOff++] = 0x00; // Length  of data

	ecpMem_Copy(&RecData[curOff], instanceAID, instanceAID[0]+1); // Application  or Executable Load File  AID

	curOff += (instanceAID[0]+1);

	RecData[curOff++] = 0x00; // Length

	RecData[curOff++] = 0x00; // Length of Extradition Parameters field

	RecData[curOff++] = 0x00; // Length of Extradition  Token

	CLA = (0x80 | selected_channel);

	INS = 0xE6;

	P1 = 0x10;

	P2 = 0x00;

	if(curOff < 0x100)
	{
		P3 = curOff;
	}
	else//暂时不支持大于0xFF的安装指令
	{
		return 0x6985;
	}

	status = IF_ExcuteProfileGPCommand(MNOSD_APPID);

	return status;

}


/*
*@funtion    :processSDAdminSessionPara85
*
*@description: 当0070长度超过0xFF时，对内部0x85的子tag进行解析，拼装成0xA5的指令形式进行存储
*
*@param      :void
*
*@return     :BYTE    --TRUE:执行成功
*                     --FALSE:执行失败
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20220803
*/
BYTE processSDAdminSessionPara85(void)
{
	TlvInfo xdata temp85Tlv;
	BYTE idata result,lenNum0070,lenNum85,temp,tempLenByte,temp1;
	UWORD xdata length;
	WORD idata sw;

	tempLenByte = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff+2);

	//The DGI shall be coded on two byes in binary format, followed by a length indicator coded as defined in GP Systems Scripting Language Specification v1.1.0, annex B.
	//
	if(tempLenByte < 0xFF)//Length < 255,one byte, eg: 0070 D1 85 820113[84LV 85LV 86LV 89LV]
	{
		lenNum0070 = 1;
	}
	else //Length >= 255,Three bytes with a leading 0xFF, eg: 0070 FF0117 85 820113[84LV 85LV 86LV 89LV]
	{
		lenNum0070 = 3;
	}


	temp = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff + 2 + lenNum0070);//point to 85 tag

	if((temp != 0x85) && (temp != 0xF5))//如果不是0x85或者0xF5 tag，返回不处理,http/dns
	{
		return TRUE;
	}

#if (GP__AMENDMENT__B__FOR_MASK ||GP__AMENDMENT__B__DNS_Resolution)
	lenNum85 = GetBERTlvLen((UWORD)(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff+2+lenNum0070+1), &length);//point to 85 len

	initTlvInfo(&temp85Tlv,G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff+2+lenNum0070+1+lenNum85,length);//point to 85 value

	initFindTlv(&temp85Tlv);

	do{//循环查找子tag，拼装成8xE20000L [A5L [84LV]]结构
		result = findTLV(&temp85Tlv,0xFF,0xFF,1);

		if(result == FALSE)
		{
			break;
		}

		if(temp == 0x85)//http
		{
			RecData[0] = 0xA5;
		}
		else//dns
		{
			RecData[0] = 0xF4;
		}
		

		if(temp85Tlv.findTLVLen <= 0x7F)
		{
			RecData[1] = temp85Tlv.findTLVLen;
			temp1 = 2;
		}
		else
		{
			RecData[1] = 0x81;
			RecData[2] = temp85Tlv.findTLVLen;
			temp1 = 3;
		}

		VMReadBuf(RecData+temp1,temp85Tlv.bufferAddr+temp85Tlv.findTagOff,temp85Tlv.findTLVLen);

		CLA = (0x80 | selected_channel);

		INS = 0xE2;  //INS of STORE DATA

		P1 = 0x80;   //Last block

		P2 = 0x00;

		P3 = temp85Tlv.findTLVLen + temp1;	    //8.6.4: Only the content of the STORE DATA commands will be provided (excluding CLA, INS, P1, P2, Lc).

		sw = IF_ExcuteProfileGPCommand(G_profileGPState.peInstallAppId); //8.6.4 Since there is no limitation in terms of content length for within the sdPersoData parameter, the complete DGI structure for the SD personalisation shall be sent in one complete byte array. Each DGI shall be provided in its own sdPersoData record. Only standardised DGIs,

		if(processStatusWord(sw,FALSE) == FALSE)
		{
			return FALSE;
		}

	}while(result);
#endif

    return TRUE;

}


/*
*@funtion    :SecurityDomainPEProcess
*
*@description: 对PE-SecurityDomain进行解析处理
*
*@param      :
*             
*@return     :BYTE    --TRUE:继续执行
*                     --FALSE:终止执行，直接返回
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
              V1.2  modify by chentao 20220426  德国profile在安装SD 时候 A2即putkey时断开，导致 自引渡数据被清除导致profile下载失败，所以把局部变量改成全局变量
*/
BYTE SecurityDomainPEProcess(void)
{
	BYTE idata result=TRUE,temp,restrictParameter;
	WORD idata sw,offset;
	WORD idata catTpMaxSduSize,catTpMaxPduSize;

	//BYTE idata needExtradite;
	
	restrictParameter = 0;
	
	catTpMaxSduSize = 0;
	
	catTpMaxPduSize = 0;
	
	//ecpMem_Set(instanceAID, 0x00, 0x11);

	//ecpMem_Set(extraditeSDAID, 0x00, 0x11);

	//needExtradite = FALSE;

	if((ValueBit(G_PEControlContext.IsExistFlag2,OccurFlag) == FALSE) && (G_PETlvInfo.bufferLen < 0x0F))//第一次进来，且前面的数据太短，直接备份，返回
	{
		backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr,  G_PETlvInfo.bufferLen);	
		
		return TRUE;
	}
	
	if(ValueBit(G_PEControlContext.IsExistFlag2,OccurFlag) == TRUE)
	{
		temp = VMReadByte(G_PETlvInfo.bufferAddr);
	
		if(temp == 0x30)//恰好在值的结束处断开
		{
			G_PETlvInfo.levelIndex = 1;
		}
	}
	else
	{
		SetBit(G_PEControlContext.IsExistFlag2,OccurFlag);
		ClearBit(G_PEControlContext.IsExistFlag3,NeedExtradite);
		ecpMem_Set(instanceAID,0,0x11);
		ecpMem_Set(extraditeSDAID,0,0x11);
	}
  	
    if(G_PETlvInfo.levelIndex == 1)
	{
		//G_PETlvInfo.lenBuffer[0] = G_PETlvInfo.findNextLen-G_PETlvInfo.findTLVLen;
		if(ValueBit(G_PEControlContext.IsExistFlag2,PE_Complete) == TRUE)
		{
			G_PETlvInfo.lenBuffer[0] = G_PETlvInfo.findNextLen-G_PETlvInfo.valueLenBuffer[0]+G_profileControlContext.TotalLen;
			G_PETlvInfo.offBuffer[0] = G_PETlvInfo.valueLenBuffer[0]-G_profileControlContext.TotalLen;
			G_PETlvInfo.findNextLen = G_PETlvInfo.valueLenBuffer[0]-G_profileControlContext.TotalLen;
		}
		else
		{
			G_PETlvInfo.findNextLen = G_PETlvInfo.bufferLen;
		}
		
	} 	
	
	while(1)
	{
		if(G_PETlvInfo.levelIndex == 0)
		{
			result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		}
			
		if(result == FALSE)
		{
			if(G_PETlvInfo.findNextLen > 0)
			{
				backData(EUICCBackPEBuf, G_PETlvInfo.bufferAddr + G_PETlvInfo.findNextOff, G_PETlvInfo.bufferLen-G_PETlvInfo.findNextOff);	
				return TRUE;
			}
			else
			{
				break;
			}		
		}
		
		if((G_PETlvInfo.tagBuffer[0] >= 0xA0) && (G_PETlvInfo.tagBuffer[0] <= 0xA5))//tag合法的情况下备份数据，tag不合法 无法二次进入SecurityDomainPEProcess，处理异常时无法确定PEID  // eUICC_M2M_additonal/压力测试/Profile_DownLoad_pressure_Test_03
		{
			if((G_PETlvInfo.tagBuffer[0] != 0xA2) && (G_PETlvInfo.isCompleteBuffer[G_PETlvInfo.levelIndex] == FALSE))
			{
				backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr + G_PETlvInfo.findTagOff,  G_PETlvInfo.bufferLen-G_PETlvInfo.findTagOff);

				return TRUE;
			}
		}
		if(G_PETlvInfo.tagBuffer[0] != 0xA0)
		{
			if(G_PEControlContext.TagA0Occ == 0)
			{
				appendErrorResponse(INVALID_PARAMETER,0);
				return FALSE;
			}
		}
		
		switch(G_PETlvInfo.tagBuffer[0])
		{
			case 0xA0:  //process sd-Header
			{				
				if(CheckTagOcc(G_PEControlContext.TagA0Occ++) == FALSE)
				{
					return FALSE;
				}
				
				if (PEHeaderAnalyze()== FALSE)  //处理PEHeader信息
				{
					return FALSE;
				}
				
				if(ValueBit(G_profileControlContext.IsCreateFlag,CREATE_GolbalPIN) == FALSE)
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}		
				
				break;
			}
			case 0xA1:  //process instance ApplicationInstance,PE-SecurityDomain instance ApplicationInstance TAG A1 is mandatory
			{			
				if(ApplicationInstanceAnalyze(PESD_MODE) == FALSE)
				{
					return FALSE;
				}
				break;
			}
			case 0xA2: //keyList SEQUENCE OF KeyObject OPTIONAL, -- see section 8.6.3
			{
				if(G_PETlvInfo.levelIndex == 0)
				{
					G_profileControlContext.TotalLen = 0;
				}
				temp = KeyObjectSequenceAnalyze();
				if(temp == FALSE)
				{
					return FALSE;
				}
				else if(temp == 2)
				{
					return TRUE;
				}
				SetBit(G_PEControlContext.IsExistFlag2,SD_A2);
				break;
			}
			case 0xA3: //sdPersoData SEQUENCE OF OCTET STRING OPTIONAL -- see section 8.6.4
			{				
				setVerticalFind(&G_PETlvInfo); // TLV嵌套，设置垂直查找信息
			
				while (1)
				{	
					result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
					
					if(result == FALSE)
					{
						break;
					}
					switch(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex])
					{
						case 0x04:
						{							
							if(G_PETlvInfo.findValueLen > 0xff)//超过一条APDU的长度
							{
								if(processSDAdminSessionPara85() == FALSE)
								{
									appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
									return FALSE;
								}

								break;

							}
							
							VMReadBuf(RecData,(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff),G_PETlvInfo.findValueLen);

							//Any DGI shall be coded on two bytes in binary format, followed by a length indicator coded as defined in [Scripting Lang] Annex B.
							if(G_PETlvInfo.findValueLen != (3 + RecData[2]))//eUICC_Profile_PE_boundary/Each_DGI_shall_be_provided_in_its_own_sdPersoData_record_test_01  eg:04 16 0070084206010203040506 0070084506060504030201中0070应该放在两个04下面
							{
								appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
								return FALSE;
							}

							CLA = (0x80 | selected_channel);
	
							INS = 0xE2;  //INS of STORE DATA
							
							P1 = 0x80;   //Last block
							
							P2 = 0x00;  
							
							P3 = G_PETlvInfo.findValueLen;	    //8.6.4: Only the content of the STORE DATA commands will be provided (excluding CLA, INS, P1, P2, Lc).                                            
									
							sw = IF_ExcuteProfileGPCommand(G_profileGPState.peInstallAppId); //8.6.4 Since there is no limitation in terms of content length for within the sdPersoData parameter, the complete DGI structure for the SD personalisation shall be sent in one complete byte array. Each DGI shall be provided in its own sdPersoData record. Only standardised DGIs,
				
							if(processStatusWord(sw,FALSE) == FALSE)
							{
								return FALSE;
							}
							
							break;
						}
						default:
						{
							appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
							return FALSE;
						}
					}
				}
				
				setReturnFind(&G_PETlvInfo);
				
				break;
			 
			}
			case 0xA4://openPersoData SEQUENCE 
				
			    if(G_profileControlContext.IsCreateMNOSD > 0)//Only the PE-SecurityDomain that instantiates the MNO-SD may include the openPersoData parameter, this parameter is forbidden for the other Security Domains
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification); //modify by chentao@20211122 中国电信离线脚本测试--pofile结构测试库V2.3.1\\data_05_OPENpersonalization\\ddata_05_OPENpersonalization_05  期望返回INVALID_REQUEST_FORMAT
					return FALSE;
				}
			
				setVerticalFind(&G_PETlvInfo); // TLV嵌套，设置垂直查找信息
			
				offset = 0 ;

				while (1)
				{	
					result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
					
					if(result == FALSE)
					{
						break;
					}
					switch(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex])
					{
						case 0xD9://SD -PE :restrictParameter [PRIVATE 25]  , GP 221  :Restrict parameter:0xD9
						{
							if(G_PETlvInfo.findValueLen != 1)
							{
								appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
							    return FALSE;
							}

							//add by chentao@20211122 中国电信离线脚本测试--pofile结构测试库V2.3.1\\data_05_OPENpersonalization\\data_05_OPENpersonalization_02 restrictParameter 是80期望下载失败
							// GP23 table 11-53 中bit8 是RFU
							restrictParameter = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
							if(U8_AND_U8(restrictParameter, 0x80)== 0x80)
							{
								appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
								return FALSE;
							}

							tempBuf[offset++] = 0xD9;
							tempBuf[offset++] = 0x01;
							tempBuf[offset++] = restrictParameter;
							//IF_PEUpdateRestrictParameter(restrictParameter);
							
							break;
						}
						case 0x04://Contactless Protocol Parameters 0x04
							if(G_PETlvInfo.findTLVLen-1 > sizeof(tempBuf))
							{
								appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
							    return FALSE;
							}
							tempBuf[offset++] = 0xA0;//contactlessProtocolParameters
							VMReadBuf(&tempBuf[offset],(UWORD)(G_PETlvInfo.bufferAddr+G_PETlvInfo.findTagOff+1), G_PETlvInfo.findTLVLen-1);//传入LV
							offset += (G_PETlvInfo.findTLVLen-1);
//							VMReadBuf(&tempBuf[0],(UWORD)(G_PETlvInfo.bufferAddr+G_PETlvInfo.findTagOff+1), G_PETlvInfo.findTLVLen-1);//传入LV
//							if(IF_PEUpdateContactlessProtocolParameters(tempBuf) == FALSE)
//							{
//								appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
//								return FALSE;
//							}
							break;
						default:
						{
							appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
							return FALSE;
						}
					}
				}

				if(makeInstallForRegistryUpdate(tempBuf,offset) != 0x9000)
				{
					appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					return FALSE;
				}
				setReturnFind(&G_PETlvInfo);			
					
				break;
			case 0xA5://catTpParameters SEQUENCE
				
				if(G_profileControlContext.IsCreateMNOSD > 0)//Only the PE-SecurityDomain that instantiates the MNO-SD may include the catTpParameters parameter: these parameters are forbidden for the other Security Domains.
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);//modify by chentao@20211118 中国电信离线脚本测试--pofile结构测试库V2.3.1\\data\06-CatTpParameters字段\\03-SD带有catTpParameters字段 期望返回INVALID_REQUEST_FORMAT
					return FALSE;
				}

				setVerticalFind(&G_PETlvInfo); 
				
				while (1)
				{	
					result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
					
					if(result == FALSE)
					{
						break;
					}
					switch(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex])
					{
						case 0x80: //catTpMaxSduSize UInt16
						{
							if(G_PETlvInfo.findValueLen == 1)
							{								
								catTpMaxSduSize = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
							}
							else if(G_PETlvInfo.findValueLen == 2)
							{
								catTpMaxSduSize = VMReadShort(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
							}
							else
							{
								appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
							    return FALSE;
							}
																				
							break;
						}
						case 0x81://catTpMaxPduSize UInt16
							//modify by chentao@20211122 中国电信离线脚本测试--pofile结构测试库V2.3.1\\ddata_06_CatTpParameters\\data_06_CatTpParameters_04   CatTpParameters字段参数的顺序与规范不一致期望下载失败
							if(catTpMaxSduSize == 0)
							{
								appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
								return FALSE;
							}

							if(G_PETlvInfo.findValueLen == 1)
							{								
								catTpMaxPduSize = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
							}
							else if(G_PETlvInfo.findValueLen == 2)
							{
								catTpMaxPduSize = VMReadShort(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
							}
							else
							{
								appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
							    return FALSE;
							}
																				
							break;
						default:
						{
							appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
							return FALSE;
						}
					}
				}
				
				IF_PEUpdateCATTPPersonalisation(catTpMaxSduSize,catTpMaxPduSize);
							
				setReturnFind(&G_PETlvInfo);	
				
				break;
							
			default: //白盒测试添加
				appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification); // eUICC_M2M_additonal/压力测试/Profile_DownLoad_pressure_Test_03   PE里的子tag超出规范定义 （PE里有A6tag）
				return FALSE;
							
		}
	}
	
	
	if(ValueBit(G_PEControlContext.IsExistFlag3,NeedExtradite) == TRUE)
	{
		sw = makeInstallForExtradition(instanceAID, extraditeSDAID);
		if(processStatusWord(sw,FALSE) == FALSE)
		{
			return FALSE;
		}
	}

	if(ValueBit(G_PEControlContext.IsExistFlag2,PE_Complete) == TRUE)//一个完整的PE执行结束
	{
		/*if((G_profileControlContext.IsCreateMNOSD == 0) && ( ValueBit(G_PEControlContext.IsExistFlag2,SD_A2) == 0))// modify by chentao@20230302 德国测试有profile MNOSD没有A2tag希望下载成功，为了COS兼容性去掉这个异常检查
		{
			appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
			return FALSE;
		}*/
		
		G_profileControlContext.IsCreateMNOSD++;
	}
		

	return TRUE;
}

#endif
