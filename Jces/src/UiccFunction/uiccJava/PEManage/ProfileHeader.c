
/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   ProfileHeader.c
*
* Description: 该文件主要是ProfileHeader的处理
*
* auther：     zxx    20170623
*
* history：    V1.0
*
************************************************************************************************************/
#include  "typedef.h"
#include "tlvHeader.h"
#include  "ProfileInterpreter.h"
#include  "profileApi.h"
#include  "uicc_main.h"
#include "Interface_PlatformFromLTE.h"
#include "Interface_LTEFromPlatform.h"
#include "ToolkitRFM.h"
#include "STKFileView.h"
#include "VMAccessEeprom.h"
#include  "Gsmpublic.h"
#include "opFunction.h"

#if (EUICC_ONLY||SE_AND_EUICC)
/*
*@funtion    :ProfileHeaderProcess
*
*@description: 对ProfileHeader进行解析处理
*
*@param      :
*             
*@return     :BYTE      --TRUE:继续执行
	*                     --FALSE:终止执行，直接返回
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/

BYTE CheckTagOcc(BYTE idata tag)
{	
	if(tag >= 1)
	{
		appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
		return FALSE;
	}
	return TRUE;
}

BYTE ProfileHeaderProcess(void)
{
	BYTE idata result,i,fileResult;
	BYTE idata majorVersion,minorVersion;
	BYTE idata tag80Occ, tag81Occ,tag82Occ,tag83Occ,tagA5Occ,tagA6Occ,tagAIDpos,tagVersionPos;
#if M2M_EUICC
	BYTE idata tag84Occ;
	tag84Occ = 0;
#endif
	
	if(ValueBit(G_PEControlContext.IsExistFlag2,PE_Complete) == FALSE)//不够一个PE，将整个数据都备份
	{
		backData(EUICCBackPEBuf, G_PETlvInfo.bufferAddr, G_PETlvInfo.bufferLen);	

		return TRUE;
	}
	
	G_DownLoadingProfileIndex = IF_GetDownloadProfileIndex();	
	
	//创建私有文件移动到profile解析过程中	
	fileResult = createMFFile();
	if(processFileResult(fileResult) == FALSE)
	{
		return FALSE;
	}
	

	G_CurMFFileStart = G_CreatingMFOffset;

	fileResult = createProfilePrivateFile();
	if(fileResult == FALSE)
	{
		return FALSE;
	}	
	
	tag80Occ = 0;
	tag81Occ = 0;
	tag82Occ = 0;
	tag83Occ = 0;
	tagA5Occ = 0;
	tagA6Occ = 0;
	
#if M2M_EUICC
//Java成员默认值是0  不需要重复设置	SetDownLoadingProfilePOL1(0);//If this variable is not supplied in the Profile Package, its value shall be set to all 0 by the eUICC.
#endif
	
	while(1)
	{
		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
			break;  //处理到最后一个tag
		}
		
		switch(G_PETlvInfo.tagBuffer[0])
		{
			case 0x80:  //major-version
			{	
				if(CheckTagOcc(tag80Occ++) == FALSE)
				{
					return FALSE;
				}
				
				if(G_PETlvInfo.findValueLen != 1)//major-version UInt8, -- set to 2 for this version of the specification
				{
					appendErrorResponse(UNSUPPORTED_PROFILE_VERSION,0);
				
					return FALSE;
				}
				
				majorVersion = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
			
				//eUICC_Response_type_E01  profile 1.0
				//Profile_FulSeg0001  profile 3.0
				if(majorVersion !=2)//If the major-version indicated by the Profile is not supported by the eUICC (e.g. if it is an earlier or an older version), the eUICC shall return an error unsupported-profile-version and stop the processing of the Profile.
				{
					appendErrorResponse(UNSUPPORTED_PROFILE_VERSION,0);
				
					return FALSE;
				}
				
				break;
			}	
			case 0x81: // minor-version				
				if(CheckTagOcc(tag81Occ++) == FALSE)
				{
					return FALSE;
				}
				
				if(G_PETlvInfo.findValueLen != 1)//minor-version UInt8, -- set to 2 for this version of the specification
				{
					appendErrorResponse(UNSUPPORTED_PROFILE_VERSION,0);
				
					return FALSE;
				}
				minorVersion = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);

				G_profileControlContext.ProfileVersion =( ((majorVersion << 4) & 0xF0) + minorVersion);
				break;
			case 0x82: //profileType				
				if(CheckTagOcc(tag82Occ++) == FALSE)
				{
					return FALSE;
				}
				break;
			case 0x83: //iccid	
				if(G_PETlvInfo.findValueLen != 0x0A)
				{
					appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					
					return FALSE;
				}
				if(CheckTagOcc(tag83Occ++) == FALSE)
				{
					return FALSE;
				}
				break;
			case 0x84: //pol	
				
		#if M2M_EUICC
				if(CheckTagOcc(tag84Occ++) == FALSE)
				{
					return FALSE;
				}
 
				majorVersion = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);  //复用majorVersion 变量，读出当前profile的POL	
				if(majorVersion > 4)
				{
					appendErrorResponse(BAD_VALUES,0);
					return FALSE;
				}
					
				SetDownLoadingProfilePOL1(majorVersion); //调用平台层函数
		#endif
				break;
			case 0xA5://eUICC-Mandatory-services
			{					
				setVerticalFind(&G_PETlvInfo);
				
				if(CheckTagOcc(tagA5Occ++) == FALSE)
				{
					return FALSE;
				}
			
				while(1)
				{
					result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
					
					if(result == FALSE)
					{
						break;  //处理到最后一个tag
					}
					
					if(G_PETlvInfo.findValueLen != 0)//NULL
					{
						appendErrorResponse(INVALID_PARAMETER,0);
						return FALSE;
					}

					//add by chentao@20211122 中国电信离线脚本测试--pofile结构测试库V2.3.1\\data_09_eUICC_Mandatory_services_05\\data_09_eUICC_Mandatory_services_05---A505 9500 00 9600
					if((G_PETlvInfo.tagBuffer[1] < HEADER_SERVICE_CONTACTLESS) || (G_PETlvInfo.tagBuffer[1] > HEADER_SERVICE_PROFILE_B_P256))
					{
						appendErrorResponse(INVALID_REQUEST_FORMAT,0);
						return FALSE;
					}
					// modify by chentao@20211111  电信SWP国密RSP备案测试要求不能支持CATTP和BERTLV 8.2.2.1 & 8.2.3.4
					if(MOBILE_Operator == OPERATOR_CHINATELECOM)//support EAP
					{
						if((G_PETlvInfo.tagBuffer[1] == 0x8C) || (G_PETlvInfo.tagBuffer[1] == 0x8A) || (G_PETlvInfo.tagBuffer[1] == 0x8D) || (G_PETlvInfo.tagBuffer[1] == 0x8E) || (G_PETlvInfo.tagBuffer[1] == 0x8F)
								|| (G_PETlvInfo.tagBuffer[1] == HEADER_SERVICE_CATTP) )//multos/EAP/multiple-usim/multiple-isim/multiple-csim
						{
							appendErrorResponse(FETURE_NOT_SUPPORTED,0);
							return FALSE;
						}
					}
					else
						{//Germany Test:support EAP,20220414 
						
						if((G_PETlvInfo.tagBuffer[1] == 0x8C) || (G_PETlvInfo.tagBuffer[1] == 0x8D) || (G_PETlvInfo.tagBuffer[1] == 0x8E) || (G_PETlvInfo.tagBuffer[1] == 0x8F) )//multos/multiple-usim/multiple-isim/multiple-csim,support eap
						{
							appendErrorResponse(FETURE_NOT_SUPPORTED,0);
							return FALSE;
						}
					}
					if(G_PETlvInfo.tagBuffer[1] == 0x92)
					{
						if(!UICC__FUNCTION__BERTLV)
						{
							appendErrorResponse(FETURE_NOT_SUPPORTED,0);
							return FALSE;
						}
					}

					if(G_PETlvInfo.tagBuffer[1] == 0x93)
					{
						if(!UICC__FUNCTION__DFLink__Cfg)
						{
							appendErrorResponse(FETURE_NOT_SUPPORTED,0);
							return FALSE;
						}
					}
					//记录所支持的ServicesList
					i = (G_PETlvInfo.tagBuffer[1]-0x80)/0x08;        //复用i
					result = G_PETlvInfo.tagBuffer[1]%0x08; //复用result,记录置1的位置

					SetBit(G_PEControlContext.SUPPORTED_ServicesList[i],result);


					/*else if((G_PETlvInfo.tagBuffer[1] >= 0x80) && (G_PETlvInfo.tagBuffer[1] <= 0x94))
					{
						
					}
					else
					{
						appendErrorResponse(INVALID_PARAMETER,0);
						return FALSE;
					}*/
										
				}		
				setReturnFind(&G_PETlvInfo);
				
				break;
			}
			case 0xA6: //eUICC-Mandatory-GFSTEList
			{
				setVerticalFind(&G_PETlvInfo);
				
				if(CheckTagOcc(tagA6Occ++) == FALSE)
				{
					return FALSE;
				}
			
				while(1)
				{
					result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
					

					if(result == FALSE)
					{
						break;  //处理到最后一个tag
					}
					
					switch(G_PETlvInfo.tagBuffer[1])
					{
						case 0x06:
						{
							if(G_PETlvInfo.findValueLen != 6)
							{
								appendErrorResponse(TEMPLATE_NOT_SUPPORTED,0);
								return FALSE;
							}
							
							i = VMReadByte(G_PETlvInfo.bufferAddr + G_PETlvInfo.findValueOff+5);
							if(!((NvmCmp((BYTE*)(G_PETlvInfo.bufferAddr + G_PETlvInfo.findValueOff),(BYTE *)&G_Common_Template_ID[0],5) == 0) && ((i >= 1) && (i <= 0x0E))))
							{
								appendErrorResponse(TEMPLATE_NOT_SUPPORTED,G_PEControlContext.PE_identification);
				
								return FALSE;
							}

							break;
						}
						default:  //白盒测试添加
							break;
					}
					
				}			
				setReturnFind(&G_PETlvInfo);
				
				break;
			}
			case 0x87://connectivityParameters
		#if M2M_EUICC
			    VMReadBuf(&tempBuf[0],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff,G_PETlvInfo.findValueLen);
			    IF_UpdateConnectPara(tempBuf,G_PETlvInfo.findValueLen);
			
			    setVerticalFind(&G_PETlvInfo);//connectivityParameters中非法tag的处理
			    while(1)
				{
					result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);					
					
					if(result == FALSE)
					{
						break;  //处理到最后一个tag
					}
					
					switch(G_PETlvInfo.tagBuffer[1])
					{
						case 0xA0:
					    case 0xA1:
						case 0xA2:
							break;
						default:
							appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
							return FALSE;
					}
				}
				setReturnFind(&G_PETlvInfo);							    										
		#endif		
				break;
			case 0xA8://eUICC-Mandatory-AIDs  V2.2
				setVerticalFind(&G_PETlvInfo);
			
			    while(1)
				{
					result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
					

					if(result == FALSE)
					{
						break;  //处理到最后一个tag
					}
					
					switch(G_PETlvInfo.tagBuffer[1])
					{
						case 0x30:
						{
							if(G_PETlvInfo.findValueLen > 0x16)//maxAIDLen(0x10)+versionLen(2)+taglen(4)
							{
								appendErrorResponse(INVALID_REQUEST_FORMAT,0);
								return FALSE;
							}

                            VMReadBuf(&tempBuf[0],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff,G_PETlvInfo.findValueLen);	
							
							tagAIDpos = FindTag(tempBuf,0x80,G_PETlvInfo.findValueLen);//查找aid ApplicationIdentifier
							
							if((tagAIDpos == FALSE) || (AIDLenInvalid(tempBuf[tagAIDpos])))//不存在aid tag 或aid的长度不符合要求
							{
								appendErrorResponse(INVALID_REQUEST_FORMAT,0);
								return FALSE;
							}
							
							tagVersionPos = FindTag(tempBuf,0x81,G_PETlvInfo.findValueLen);//查找version
							
							if((tagVersionPos == FALSE) || (tempBuf[tagVersionPos] != 2))//version不存在
							{
								appendErrorResponse(INVALID_REQUEST_FORMAT,0);
								return FALSE;
							}
							
							/*The eUICC-Mandatory-AIDs list the AIDs and version of the library packages that are required for the Profile Package to be installed correctly.
							When an AID is present in the Profile header and not known by the eUICC, 
							the installation of the Profile Package shall be aborted with the status code lib-not-supported.
							When the version is not compatible with the versions supported by the eUICC, the installation of the Profile Package shall also be aborted by the eUICC with
							the status code lib-not-supported.*/
							if(IF_GPGetPkgIDByPkgAID(&tempBuf[tagAIDpos+1],tempBuf[tagAIDpos],tempBuf[tagVersionPos+1],tempBuf[tagVersionPos+2]/*,1*/) == ILLEGAL_PKGID)
							{
								appendErrorResponse(LIB_NOT_SUPPORT,0);
								return FALSE;
							}
							
							break;
						}
						default:  //白盒测试添加
							break;
					}
					
				}	
			
			    setReturnFind(&G_PETlvInfo);
				
				break;
			default:
				appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
				return FALSE;
			
			
		}
	}
	
	if((tag80Occ == 0) || (tag81Occ == 0) || (tag83Occ == 0)|| (tagA5Occ == 0) || (tagA6Occ == 0))
	{
		appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
		return FALSE;
	}		
	//copy ISDP info

	return TRUE;
}

#endif
