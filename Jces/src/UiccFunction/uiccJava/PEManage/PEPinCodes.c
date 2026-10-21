
/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   PEPinCodes.c
*
* Description: 该文件主要是对PE-PINCodes的所有处理
*
* auther：     zxx    20170623
*
* history：    V1.0
*
************************************************************************************************************/


#include  "typedef.h"
#include  "fileAction.h"
#include  "gsmpublic.h"
#include "tlvHeader.h"
#include  "ProfileInterpreter.h"
#include "profileApi.h"
#include "fileSystem.h"

#if (EUICC_ONLY||SE_AND_EUICC)
/*
*@function: PEProcessPINConfiguration
*
*@description: 处理PIN PE中的PINConfiguration模块
*
*@param       :void
*
*@return       :TRUE成功/FALSE:错误
*
*@author      :create by xuyz
*
*@hisroty    :V1.0 :create by  xuyz20170724
*/

/*
*@funtion    :IsExistLocalPinRef
*
*@description: All the Local PINs referenced by a pinStatusTemplateDO shall either be defined in a parent ADF or DF or created in a following PE-PINCodes.
*
*@param      :void
*   
*@return     :TRUE/FALSE
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20190225
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/ 
BYTE IsExistLocalPinRef(void)
{
	BYTE idata i,j,pinRef,recNum;
	UWORD xdata fileAddr,firstLevelDf;
	
	if(G_fileControlContext.LoadDFAddr == G_CreatingMFOffset)//MF/DF，则不需要判断
	{
		 return TRUE;
	}
	else
	{
		firstLevelDf = GetFirstLevelDf(G_fileControlContext.LoadDFAddr);

	    fileAddr = SelectFile(firstLevelDf,EF_PRIVATE_PIN);//选择ADF下面的localpin文件
	   
		if(fileAddr == FALSE)
		{
			return FALSE;
		}
	
		recNum = GetLFRecNumLgth(fileAddr,FileNum);
	
		
		for(i= 0x00;i<0x04;i++)
		{
			pinRef = GetFileHeadByte(firstLevelDf, F_ChvStatus_Offset+i);//ADF/DF Local PINs referenced by a pinStatusTemplateDO
	
			if((pinRef >= 0x81) && (pinRef <= 0x88))
			{
				for(j = 1;j<=recNum;j++)
				{
					ReadLFBuf(fileAddr,j,0,&tempBuf[0],1);
					if(pinRef == tempBuf[0])
					{
						break;
					}
				}
				if(j > recNum)//不存在
				{
					 return FALSE;
				}						
			}
					
		}
		return TRUE;				
	}
}



/*
*@funtion    :ISExistInPukList
*
*@description: In case a PUKKeyReferenceValue is defined the related PUKKeyReferenceValue shall exist within the PE-PUKCodes list.
*
*@param      :BYTE pukRef
*   
*@return     :TRUE/FALSE
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20190709
*/ 
BYTE ISExistInPukList(BYTE pukRef)
{
	UWORD idata fileAddr;
	BYTE  idata num,i,tag;
	
	fileAddr = SelectFile(G_CreatingMFOffset,DF_PROFILECFG);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	fileAddr = SelectFile(fileAddr,EF_PRIVATE_PUK);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	num = GetLFRecNumLgth(fileAddr,FileNum);
	
	for(i = 1; i <= num; i++)
	{
		ReadLFBuf(fileAddr,i,0,&tag,1);
		
		if(tag == pukRef)                          //第一字节是0xFF表示空记录
		{					
			return TRUE;	
		}
 		
	}
	return FALSE;
}




/*
*@funtion    :PEProcessPINConfiguration
*
*@description: pinconfig SEQUENCE (SIZE (1..26))OF PINConfiguration,
*
*@param      :void
*   
*@return     :TRUE/FALSE
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20190709
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE PEProcessPINConfiguration(void)
{
	BYTE idata result,tag80,tag81;
	BYTE idata buf[2];	

	tag80 = 0;
	tag81 = 0;
	setVerticalFind(&G_PETlvInfo);
	
//	tempBuf[KEY_STATUS_OFF] = 0x07;//pinAttributes UInt8 DEFAULT 7, 
		
	
	while(1)
	{	
		
		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
			break;  //处理到最后一个tag
		}		
		tag80 = 0;
		tag81 = 0;

		if(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] == 0x30)
		{
			tempBuf[KEY_STATUS_OFF] = 0x07;
			
			tempBuf[KEY_LIMIT_OFF] = 0x33;  //十进制的51
			
			tempBuf[KEY_PUKREF_OFF] = 0;
			
			setVerticalFind(&G_PETlvInfo);		
			
			while(1)
			{

				result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
				
				if(result == FALSE)
				{					
					break;//处理到最后一个tag
				}	
									
				switch(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex])
				{
					case 0x80:  //keyReference
						
						VMReadBuf(buf,G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen); 
					
						if(G_PETlvInfo.findValueLen == 0x02)
						{
							tempBuf[KEY_ID_OFF] = buf[1];							
						}
						else if(G_PETlvInfo.findValueLen == 0x01)
						{
							tempBuf[KEY_ID_OFF] = buf[0];	
						}
						else
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
							return FALSE;
						}
						
						if(((tempBuf[KEY_ID_OFF] >= 1) && (tempBuf[KEY_ID_OFF] <= 0x08)) || ((tempBuf[KEY_ID_OFF] >= 10) && (tempBuf[KEY_ID_OFF] <=14))
							|| ((tempBuf[KEY_ID_OFF] >= 0x81) && (tempBuf[KEY_ID_OFF] <= 0x88)) ||((tempBuf[KEY_ID_OFF] >= 138) && (tempBuf[KEY_ID_OFF] <= 142)))
						{
						}
						else
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
							return FALSE;
						}
						tag80 = 1;	
						break; 
						
					case 0x81:  //pinValue OCTET STRING (SIZE (8))	
                        
                        if(G_PETlvInfo.findValueLen != 8)	
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
							return FALSE;
						}							
					
						ecpMem_Set(&tempBuf[KEY_CONTENT_OFF],0xFF,0x08);
					
						VMReadBuf(&tempBuf[KEY_CONTENT_OFF],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen); 
					    tag81 = 1;
						break;
					
					case 0x82:  //unblockingPINReference PUKKeyReferenceValue OPTIONAL						
					
					    VMReadBuf(buf,G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen); 
					
						if(G_PETlvInfo.findValueLen == 0x02)
						{
							tempBuf[KEY_PUKREF_OFF] = buf[1];
						}
						else if(G_PETlvInfo.findValueLen == 0x01)
						{
							tempBuf[KEY_PUKREF_OFF] = buf[0];
						}
						else
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
							return FALSE;
						}
						
						if(((tempBuf[KEY_PUKREF_OFF] >=1) && (tempBuf[KEY_PUKREF_OFF] <= 8)) || ((tempBuf[KEY_PUKREF_OFF] >= 0x81) && (tempBuf[KEY_PUKREF_OFF] <= 0x88)))
						{
						}
						else
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
							return FALSE;
						}
						
						if(ISExistInPukList(tempBuf[KEY_PUKREF_OFF]) == FALSE)//In case a PUKKeyReferenceValue is defined the related PUKKeyReferenceValue shall exist within the PE-PUKCodes list.
						{
							if(ValueBit(G_profileControlContext.IsCreateFlag,CREATE_PUK) == TRUE) // modify chentao20220510 PUK is create
							{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
							return FALSE;
							}
						}
						
						break;
					
					case 0x83: //pinAttributes UInt8 DEFAULT 7, 
						 
					    if(G_PETlvInfo.findValueLen != 1)	
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
							return FALSE;
						}
						
						VMReadBuf(&tempBuf[KEY_STATUS_OFF],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen); 
						if(tempBuf[KEY_STATUS_OFF] > 0x07)
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
							return FALSE;
						}
						break;
					
					case 0x84: //maxNumOfAttemps-retryNumLeft UInt8 DEFAULT 51
						VMReadBuf(buf,G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen); 
					
						if(G_PETlvInfo.findValueLen == 0x02)
						{
							tempBuf[KEY_LIMIT_OFF] = buf[1];
						}
						else if(G_PETlvInfo.findValueLen == 0x01)
						{
							tempBuf[KEY_LIMIT_OFF] = buf[0];
						}
						else
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
							return FALSE;
						}
						
						if((tempBuf[KEY_LIMIT_OFF] > 0x7F) && (G_PETlvInfo.findValueLen == 0x01))
						{
							if(MOBILE_Operator == OPERATOR_CHINATELECOM)//modify chentao@202200426
							{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
							return FALSE;
							}
						}
						break;
					default:
						appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
						return FALSE;
					
				}
			}
				
			if((tag80 == 0) || (tag81 == 0))
			{
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
			}
			
			setReturnFind(&G_PETlvInfo);
			
			if(checkPINPUKMaxRemain(tempBuf[KEY_LIMIT_OFF] ) == FALSE)
			{
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
			}
			
			if(updatePinData(&tempBuf[0], PIN_LEN) == FALSE)
			{
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
			}
			
		}
		else
		{
			appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
			return FALSE;
		}
	}

	if((tag80 == 0) || (tag81 == 0))
	{
		appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
		return FALSE;
	}
		
	setReturnFind(&G_PETlvInfo);
		
	return TRUE;
			
}
/*
*@funtion    :PINCodesPEProcess
*
*@description: 对PE-PINCodes进行解析处理
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
BYTE PINCodesPEProcess(void)
{
	BYTE idata result;
	UWORD xdata curAdfAddr;
	UWORD xdata curDFAddr;
	WORD idata adfId;

	result = FALSE;
	curAdfAddr = FALSE;
	curDFAddr = FALSE;
	
	if(ValueBit(G_PEControlContext.IsExistFlag2,PE_Complete) == FALSE)//不够一个PE，将整个数据都备份
	{
		backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr,  G_PETlvInfo.bufferLen);	
		
		return TRUE;
	}		
	
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
			case 0xA0:
				
				if(CheckTagOcc(G_PEControlContext.TagA0Occ++) == FALSE)
				{
					return FALSE;
				}
				
				if (PEHeaderAnalyze()== FALSE)  //处理PEHeader信息
				{
					return FALSE;
				}
				
				/*if((ValueBit(G_profileControlContext.IsCreateFlag,CREATE_MF) == FALSE) || (ValueBit(G_profileControlContext.IsCreateFlag,CREATE_PUK) == FALSE))
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
								
					return FALSE;
				}*/
				if(ValueBit(G_profileControlContext.IsCreateFlag,CREATE_MF) == FALSE) // modify chentao20220510 德国有profile时PUK在PIN之后需要兼容
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
								
					return FALSE;
				}
				if(ValueBit(G_profileControlContext.IsCreateFlag,CREATE_PUK) == FALSE)
				{
					if(MOBILE_Operator == OPERATOR_CHINATELECOM) // //add chentao20220510--兼容德国A3在A2 之前的profile，电信备案测试WQ_Offline_profile_Test\TC_1_1_7_ProfileFormat_PinBeforePUK_Erro
					{
						appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);

						return FALSE;
					}
				}
				
				if(G_fileControlContext.LoadDFAddr == G_CreatingMFOffset)
				{
					if(ValueBit(G_profileControlContext.IsCreateFlag,CREATE_GolbalPIN) == TRUE)
					{
						appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
						return FALSE;
					}			
				}
				else
				{
					if(ValueBit(G_profileControlContext.IsCreateFlag,CREATE_LocalPIN) == TRUE)
					{
						appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
								
						return FALSE;
					}
				}
				
				break;		
			
			case 0xA1:      //pinCodes pinconfig
				
				setVerticalFind(&G_PETlvInfo);
			
			
				while(1)
				{
					result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
					
					if(result == FALSE)
					{
						break;  //处理到最后一个tag
					}
			
					if(G_PETlvInfo.tagBuffer[1] == 0xA0)//pinconfig SEQUENCE (SIZE (1..26))OF PINConfiguration,
					{
						if(G_fileControlContext.LoadDFAddr != G_CreatingMFOffset)//该DF有可能是ADF，也有可能是7F10，也有可能是7F10/5F3C,pin文件需要创建在ADF/7F10下面,3F00不需要创建localpin文件
						{
							if(createSpecProfilePrivateFile(TRUE,0,0) == FALSE)
							{
								return FALSE;
							}
						}
						
						if(PEProcessPINConfiguration() == FALSE)
						{
							return FALSE;
						}							
					}
					else if(G_PETlvInfo.tagBuffer[1] == 0x81)//filePath OCTET STRING /* temporary File ID for ADF, coding according to section 8.3.5 */
					{
							if((G_PETlvInfo.findValueLen % 2) != 0)
							{
								appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification); //长度必须是偶数
								return FALSE;
							}
							
							if(G_PETlvInfo.findValueLen > (2*DF_Depth))
							{
								appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification); //长度不能超过DF的深度
								return FALSE;
							}
						
							curAdfAddr  = G_fileControlContext.LoadADFAddr; //备份当前的ADF地址
							
							curDFAddr = G_fileControlContext.LoadDFAddr;
						
							if(profileSelectByPath((BYTE *)(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff),G_PETlvInfo.findValueLen) == FALSE)//选择要映射的文件地址，地址设置为映射文件的地址
							{
								appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
								return FALSE;
							}								
							
							//获取映射文件的ADF地址										
							VMReadBuf((BYTE xdata *)&adfId,G_fileControlContext.LoadADFAddr,sizeof(WORD));	//获取映射的ADF ID

							G_fileControlContext.LoadDFAddr = curDFAddr;  //恢复为当前的DF地址
							
							G_fileControlContext.LoadADFAddr = curAdfAddr;//G_fileControlContext.LoadADFAddr需要恢复为之前的ADF，否则后面的鉴权文件不会创建到当前ADF下面，会创建在映射的ADF下面
							
							if(G_fileControlContext.LoadADFAddr != FALSE)
							{
								if(createSpecProfilePrivateFile(TRUE,1,adfId) == FALSE)//在当前ADF下面创建localpin文件，属性是映射文件
								{
									return FALSE;
								}
							}						

							break;
					}
				}
			
				setReturnFind(&G_PETlvInfo);
			
				break;		
			   			
			default:
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
			
		}
	}
	
	if(G_profileControlContext.ProfileVersion >= 0x22)//profile2.2规范才做此检查
	{
		if(IsExistLocalPinRef() == FALSE)
		{
			appendErrorResponse(PIN_CODE_MISSING,G_PEControlContext.PE_identification);
			return FALSE;
		}
	}
		
	return TRUE;
}

#endif
