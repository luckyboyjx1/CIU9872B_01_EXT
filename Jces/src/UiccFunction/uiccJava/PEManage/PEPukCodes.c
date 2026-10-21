
/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   PEPukCodes.c
*
* Description: 该文件主要是对PE-PUKCodes的所有处理
*
* auther：     zxx    20170623
*
* history：    V1.0
*
************************************************************************************************************/



#include  "typedef.h"
#include  "fileAction.h"
#include  "gsmpublic.h"
#include  "tlvHeader.h"
#include  "ProfileInterpreter.h"
#include  "profileApi.h"
#include  "Interpreter.h"
#include  "uicc_main.h"

#if (EUICC_ONLY||SE_AND_EUICC)

/*
*@function :PEProcessPUKConfiguration
*
@description:解析Profile中的pukCodes 组件
*
*@param     :void
*
*@return    :void
*
*@author    :xuyz
*
*@history   :V1.0 :create by xuyz@20170725
*           :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/


BYTE PEProcessPUKConfiguration(void)
{
	BYTE idata result,tag80,tag81;
	BYTE idata buf[2];
	
	tempBuf[PUK_LIMIT_OFF] = 0xAA;   //maxNumOfAttemps-retryNumLeft UInt8 DEFAULT 170	
	
	tag80 = 0; 
	tag81 = 0;

	setVerticalFind(&G_PETlvInfo);
	
	while(1)
	{	
		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
			break;  //处理到最后一个tag
		}	
	
		switch(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex])
		{
			case 0x80:  //keyReference

				VMReadBuf(buf,G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen); 
			
				if(G_PETlvInfo.findValueLen == 0x02)
				{
					tempBuf[PUK_ID_OFF] = buf[1];
				}
				else if(G_PETlvInfo.findValueLen == 0x01)
				{
					tempBuf[PUK_ID_OFF] = buf[0];
				}
				else
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
				
				if(((tempBuf[PUK_ID_OFF] >=1) && (tempBuf[PUK_ID_OFF] <= 8)) || ((tempBuf[PUK_ID_OFF] >= 0x81) && (tempBuf[PUK_ID_OFF] <= 0x88)))
				{
				}
				else
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
				tag80 = 1;
				break; 
				
			case 0x81:  //pukValue OCTET STRING (SIZE (8))
				if(G_PETlvInfo.findValueLen != 8)	
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
				
				VMReadBuf(&tempBuf[PUK_VALUE_OFF],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen); 
			    tag81 = 1;
				break;
			
			case 0x82:  //maxNumOfAttemps-retryNumLeft
				VMReadBuf(buf,G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen); 
			
				if(G_PETlvInfo.findValueLen == 0x02)
				{
					tempBuf[PUK_LIMIT_OFF] = buf[1];
				}
				else if(G_PETlvInfo.findValueLen == 0x01)
				{
					tempBuf[PUK_LIMIT_OFF] = buf[0];
				}
				else
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
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
	
	if(checkPINPUKMaxRemain(tempBuf[PUK_LIMIT_OFF] ) == FALSE)
	{
		appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
		return FALSE;
	}

	setReturnFind(&G_PETlvInfo);
	
	if(updatePukData(&tempBuf[PUK_ID_OFF],PUK_LEN) == FALSE)
	{
		appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
		return FALSE;
	}
	
	return TRUE;
}
/*
*@funtion    :PUKCodesPEProcess
*
*@description: 对PE-PUKCodes进行解析处理
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
BYTE PUKCodesPEProcess(void)
{
	BYTE idata result;	
	
	//tagA0Occ = 0;	
	
	if(ValueBit(G_PEControlContext.IsExistFlag2,PE_Complete) == FALSE)//不够一个PE，将整个数据都备份
	{
		backData(EUICCBackPEBuf, G_PETlvInfo.bufferAddr, G_PETlvInfo.bufferLen);	
	
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
			
				if( (ValueBit(G_profileControlContext.IsCreateFlag,CREATE_PUK) == TRUE)|| (ValueBit(G_profileControlContext.IsCreateFlag,CREATE_MF) == FALSE))//Only one set of PUK codes exist in a Profile Package
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					
					return FALSE;
				}
			    SetBit(G_profileControlContext.IsCreateFlag,CREATE_PUK);
				break;
			
			case 0xA1:  //thePUKCodesPECodesTag
			
				setVerticalFind(&G_PETlvInfo);
			
				while(1)
				{
					result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
					
					if(result == FALSE)
					{
						break;  //处理到最后一个tag
					}
			
					if(G_PETlvInfo.tagBuffer[1] == 0x30)
					{
						if(PEProcessPUKConfiguration()== FALSE)
						{
							return FALSE;
						}
					}
					else
					{
						appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
						return FALSE;
					}
				}
			
				setReturnFind(&G_PETlvInfo);
			
				break;
			default:
				appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);//eUICC_M2M_additonal/压力测试/Profile_DownLoad_pressure_Test_03/Transaction 6:  下载第3个PE错误的profile测试     ,a3 PE里面包含A0  A1 A2 这3个子tag，按照规范
				/*
				 * PE-PUKCodes ::= SEQUENCE { puk-Header PEHeader, pukCodes SEQUENCE (SIZE (1..16))OF PUKConfiguration } 只有A0 A1 没有其他tag
				 */
				return FALSE;
				//break;
		}
	}

	
	return TRUE;
}

#endif
