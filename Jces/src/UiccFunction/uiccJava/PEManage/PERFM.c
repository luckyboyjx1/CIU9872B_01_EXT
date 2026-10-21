
/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   PERFM.c
*
* Description: 该文件主要是对PE-RFM的所有处理
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
#include  "ToolkitAPIPublic.h"
#include  "ToolkitRFM.h"
#include  "profileApi.h"
#include  "gsmpublic.h"
#include  "profileInterpreter.h"
#include  "fileSystem.h"
#include  "STKRemoteAppManagement.h"
#include  "VMAccessEeprom.h"
#include  "opFunction.h"
#include  "SEEUICCInterface.h"
#if (EUICC_ONLY||SE_AND_EUICC)
/*
*@function   : PESaveRFMData
*
*@description: 在处理PE RFM 数据时，将数据解析完成后存储在2800/4E05文件中
*
*@param       :BYTE * recBuf 记录数据缓存空间,4E05的结构已改: TarNum 1字节，TarValue(15字节) MSL(1字节) UICCACCESSDomain(5字节) UICCAdminAccessDomain(5字节) AID(17字节) adfDomain(5字节) ADFAdminDomain(5字节)
*
*@return      :TRUE/FALSE 数据存储成功or失败
*
8@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20170723
*/
BYTE PESaveRFMData(BYTE * recBuf)
{
	UWORD xdata fileAddr;
	BYTE idata i,num,recLgth,tarNum,m,n;
	
	fileAddr = SelectFile(G_CreatingMFOffset,DF_Toolkit_P);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	fileAddr = SelectFile(fileAddr,EF_RFM_TAR);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	num = GetLFRecNumLgth(fileAddr,FileNum);

	recLgth = GetLFRecNumLgth(fileAddr,FileLgth);
	
	for(i=1;i<= num;i++) //检查TAR是否重复/instance AID 是否重复
	{
		ReadLFBuf(fileAddr,i,0x00,&tarNum,sizeof(BYTE));//TAR是否重复	
		
		if((tarNum != 0x00)&&(tarNum != 0xFF))  
		{
			ReadLFBuf(fileAddr, i,0x01, tempBuf, tarNum*3);
			for(m =0; m< (tarNum*3); m+= 3)
			{
				for(n =1; n< (recBuf[0]*3); n+= 3)
				{
					if(ecpMem_Compare(&tempBuf[m],&recBuf[n],3) == EQUAL)
					{
						return FALSE;
					}
				}
			}


			ReadLFBuf(fileAddr,i,PEAID_Offset,tempBuf,0x11);//	instance AID是否重复
			if((tempBuf[0] == recBuf[PEAID_Offset]) && (ecpMem_Compare(&tempBuf[1],&recBuf[PEAID_Offset+1],tempBuf[0]) == 0))
			{
				return FALSE;
			}			
		}
		else
		{
			break;
		}	
		
	}
	
	if(i <= num)
	{
		WriteLFBuf(fileAddr, i,0x00, recBuf, recLgth);//写空记录
			
		return TRUE;
	}
	
	return FALSE;
}




/*
*@funtion    :RFMPEProcess
*
*@description: 对PE-RFM进行解析处理
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
*/


BYTE RFMPEProcess(void)
{
	BYTE idata result,appID,tag81,i,recNo;
	BYTE idata aidTimes,A0tagTimes,uiccDomainTimes,tarNum,accessPara;
	BYTE xdata aidTar[0x04];
	WORD idata offset;
	BYTE xdata tembuf[0x03];
		
	if(ValueBit(G_PEControlContext.IsExistFlag2,PE_Complete) == FALSE)//不够一个PE，将整个数据都备份
	{
		backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr,  G_PETlvInfo.bufferLen);	
	
		return TRUE;
	}

	appID = MNOSD_APPID;
	
	aidTimes = 0;
	
	A0tagTimes = 0;
	
	uiccDomainTimes = 0;
	
	tarNum = 0;
	
	offset = 0;
	
	accessPara = 0;
	
	ecpMem_Set(aidTar, 0x00, sizeof(aidTar));
	
	ecpMem_Set(RecData, 0x00, 200);
	
	ecpMem_Set(tembuf, 0x00, sizeof(tembuf));
	
	tag81 = 0;
		
	while(1)
	{
		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
			break;  //处理到最后一个tag
		}
		
		
		if(G_PETlvInfo.tagBuffer[0] != 0xA0)
		{
			if( G_PEControlContext.TagA0Occ == 0)
			{
				appendErrorResponse(INVALID_PARAMETER,0);
				return FALSE;
			}	
		}
		
		switch(G_PETlvInfo.tagBuffer[0])
		{
			case 0xA0:  //process sd-Header
			{	
				if(A0tagTimes == 0x00)
				{
					if(CheckTagOcc(G_PEControlContext.TagA0Occ++) == FALSE)
					{
						return FALSE;
					}
					
					if (PEHeaderAnalyze()== FALSE)  //处理PEHeader信息
					{
						return FALSE;
					}
					
					
					if(G_profileControlContext.IsCreateMNOSD == 0)
					{
						appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);//Installing profile with PE-RFM before PE-SecurityDomain, eUICC reports error. eUICC responses with PEStatus (5) invalid-request-format or PEStatus (6) invalid-parameter
					
					   return FALSE;
					}				
				}
				else if(A0tagTimes == 0x01)  //tarList [0] SEQUENCE OF OCTET STRING (SIZE(3)) OPTIONAL,
				{
					setVerticalFind(&G_PETlvInfo);
					
					while(1)
					{
						result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
						
						if(result == FALSE)
						{
							break;  //处理到最后一个tag
						}
						if(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] == 0x04)  
						{						
							VMReadBuf(&RecData[PETARValue_Offset + tarNum*3],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff,0x03);  
														
							recNo = ToolkitfindSpecifiedTARValue(&RecData[PETARValue_Offset + tarNum*3]);//同一个profile中RFM的tar不能和应用实例的tar相同
							
							if(recNo != FALSE)
							{
								appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
								return FALSE;
							}
							else
							{
								recNo = ToolkitFindSDTars(&RecData[PETARValue_Offset + tarNum*3]);//同一个profile中RFM的tar不能和SSD的tar相同
								
								if(recNo != ILLEGAL_APPID)
								{
									appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
									return FALSE;
								}

#if SE_AND_EUICC
								G_CurExecutingProfileIndex = ILLEGAL_PROFILEINDEX;//查找是否与SE区的SD TAR重复
								ExecuEnvironmentInit(G_CurExecutingProfileIndex);

								recNo = ToolkitFindSDTars(&RecData[PETARValue_Offset + tarNum*3]);//profile中RFM的tar不能和SE的SSD的tar相同

								G_CurExecutingProfileIndex = bRamBuffer[LOADING_PROFILE_INDEX];
								ExecuEnvironmentInit(G_CurExecutingProfileIndex);

								if(recNo != ILLEGAL_APPID)
								{
									appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
									return FALSE;
								}
#endif
							}
									
							tarNum ++;		
                            
							if(tarNum > 5)//tar的个数超过文件最大限制5，抛错
							{
								appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
								return FALSE;
							}
 							
						}
						else
						{
							appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);//不识别的tag，抛错
							return FALSE;
						}
					}
					
					setReturnFind(&G_PETlvInfo);							
				}
				else
				{
					//error 
				}
				
				A0tagTimes ++;
			
				break;
			}
			case 0x4F: 
				
				if(aidTimes == 0x00)   //instanceAID [APPLICATION 15] ApplicationIdentifier
				{
					if(AIDLenInvalid(G_PETlvInfo.findValueLen))
					{
						appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
						return FALSE;
					}
					if(G_PETlvInfo.findValueLen >= 15)
					{
						aidTar[0] = TRUE;  //标识RFM AID 存在TAR字段
						
						VMReadBuf(&aidTar[1],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff+12,0x03);  
					}	
					RecData[PEAID_Offset] = G_PETlvInfo.findValueLen;
					VMReadBuf(&RecData[PEAID_Offset+1],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff,G_PETlvInfo.findValueLen); 
										
					if(IF_GPGetPkgIDByPkgAID(&RecData[PEAID_Offset+1],G_PETlvInfo.findValueLen,0,0/*,0*/) != ILLEGAL_PKGID)//同一个profile中不能存在包的应用包的aid与RFM实例aid相同
					{
						appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
						return FALSE;
					}
					
					if(IF_GetAnalyzeAppIdByAIDList(&RecData[PEAID_Offset+1],G_PETlvInfo.findValueLen) != ILLEGAL_APPID)//同一个profile中不能存在应用实例的aid与rfm实例aid相同
					{
						appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
						return FALSE;
					}
				}
				else if(aidTimes == 0x01)  //securityDomainAID [APPLICATION 15] ApplicationIdentifier OPTIONAL,
				{                   
					appID = IF_GetAnalyzeAppIdByAIDList((BYTE *)(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff),G_PETlvInfo.findValueLen);					
					
					if(appID == ILLEGAL_APPID) //关联的SD不存在
					{
						appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
						return FALSE;
					}
					
				}
				else
				{
					//报错
				}
				aidTimes ++;
				
				break;
			case 0x81:   //minimumSecurityLevel [1] OCTET STRING (SIZE (1)),
				
				if(tarNum == 0x00)  //In case tarList is provided the TAR values are taken from this list. tarList shall include at least one TAR if available. In case tarList is not available the TAR value defined within bytes 13-15 of the instanceAID is used.
				{
					
					if(aidTar[0] == TRUE)//即使TAR不存在，也不抛错
					{
						ecpMem_Copy(&RecData[PETARValue_Offset],&aidTar[1],0x03);
		
					    tarNum ++;
					}			
					
				}
				
				if(G_PETlvInfo.findValueLen != 1)//msl的长度不为1，抛错
				{
					appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					return FALSE;
				}
				
				RecData[PETarMSL_Offset] = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
				tag81 = 1;
				break;
			
			case 0x04:  
				
			    if((G_PETlvInfo.findValueLen != 1) && (G_PETlvInfo.findValueLen != 4))//长度判断
				{
					appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					return FALSE;
				}
				
				accessPara = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
				
				if(((G_PETlvInfo.findValueLen == 1) && ((accessPara != 0) && (accessPara != 0xFF))) || ((G_PETlvInfo.findValueLen == 4) && (accessPara != 2)))
				{
					appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					return FALSE;
				}
			
				if(uiccDomainTimes == 0x00)   //uiccAccessDomain OCTET STRING,
				{
					offset = PEUICCAccessDomain_Offset;			
				}
				else if(uiccDomainTimes == 0x01)  //uiccAdminAccessDomain OCTET STRING,
				{
					offset = PEUICCAdminAccessDomain_Offset;					
				}
				else
				{
					
				}
				
				RecData[offset] = G_PETlvInfo.findValueLen;  //LV 结构
					
				VMReadBuf(&RecData[offset +1],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff,G_PETlvInfo.findValueLen);    //读取uicc accessDomain字段
					
				uiccDomainTimes ++;
			
				break;
				
			case 0x30:    //adfRFMAccess ADFRFMAccess OPTIONAL
				
				setVerticalFind(&G_PETlvInfo);
			
				while(1)
				{
					result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
					
					if(result == FALSE)
					{
						break;  //处理到最后一个tag
					}
					if(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] ==  0x80)   //adfAID ApplicationIdentifier
					{
                 
						if(AIDLenInvalid(G_PETlvInfo.findValueLen))  //aid长度非法
						{
							appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
							return FALSE;
						}
						
						RecData[PEADFAid_Offset] = G_PETlvInfo.findValueLen;  //LV 结构
					
						VMReadBuf(&RecData[PEADFAid_Offset +1],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff,G_PETlvInfo.findValueLen);    //读取uicc accessDomain字段			
			
						if(GetAIDNumber(&RecData[PEADFAid_Offset],TRUE) == FALSE)
						{
							//ERROR  此处需要判断当前profile是否存在该ADF AID，如果不存在则报错
							appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
							return FALSE;	
						}
						
						if(ecpMem_Compare(&RecData[PEAID_Offset],&RecData[PEADFAid_Offset],G_PETlvInfo.findValueLen+1) == 0)//RFM的instanceAID和ADF aid不能相同
						{
							appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
							return FALSE;
						}
						
						if(IF_GPGetPkgIDByPkgAID(&RecData[PEADFAid_Offset+1],G_PETlvInfo.findValueLen,0,0/*,0*/) != ILLEGAL_PKGID)//同一个profile中不能存在包的应用包的aid与adf aid相同
						{
							appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
							return FALSE;
						}
						
						if(IF_GetAnalyzeAppIdByAIDList(&RecData[PEADFAid_Offset+1],G_PETlvInfo.findValueLen) != ILLEGAL_APPID)//同一个profile中不能存在应用实例的aid与rfm adf aid相同
						{
							appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
							return FALSE;
						}

						tembuf[0] = 1;
					}
					else 
					{
						
						if((G_PETlvInfo.findValueLen != 1) && (G_PETlvInfo.findValueLen != 4))//长度判断
						{
							appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
							return FALSE;
						}
						
						if(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] ==  0x81) //adfAccessDomain OCTET STRING,		
						{
							offset = PEAIDAccessDomain_Offst;
							tembuf[1] = 1;
						}
						else if(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] ==  0x82)  //adfAdminAccessDomain OCTET STRING
						{
							offset = PEAIDAdminAccessDomain_Offset;
							tembuf[2] = 1;
						}
						else
						{
							appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
							return FALSE;	
						}
						
						RecData[offset] = G_PETlvInfo.findValueLen;  //LV 结构
					
						VMReadBuf(&RecData[offset +1],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff,G_PETlvInfo.findValueLen);    //读取uicc accessDomain字段			
			
					}
						
				}	
				for(i =0; i < sizeof(tembuf); i++)
				{
					if(tembuf[i] == 0)
					{
						appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
						return FALSE;	
					}
				}
				setReturnFind(&G_PETlvInfo);	
				
				break;
				
			default:
					appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					return FALSE;	
		}
	}
	
	
	if((A0tagTimes < 1) || (aidTimes == 0) || (uiccDomainTimes < 2) || (tag81 == 0)) //TAR list 可选，TAR可以从AID中获取
	{
		appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
		return FALSE;	
	}
	
	RecData[PETARNum_Offset] = tarNum;
	RecData[PEsecurityDomainID_Offset] = appID; 
	
	if(PESaveRFMData(RecData) == FALSE) //4E05的记录数据
	{
		appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
		return FALSE;
	}
	return TRUE;
}

#endif
