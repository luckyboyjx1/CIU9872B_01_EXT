/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   PENonStandard.c
*
* Description: 该文件主要是对PE-NonStandard的所有处理
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

#if (EUICC_ONLY||SE_AND_EUICC)
/*
*@funtion    :NonStandardPEProcess
*
*@description: 对PE-NonStandard进行解析处理
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
BYTE NonStandardPEProcess(void)
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

				if(ValueBit(G_PEControlContext.IsExistFlag2,Mandated) == TRUE)//TC_8_2_9_2_Error_when_installing_mandatory_PE_NonStandard
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}

				break;
			case 0x81:  //theNonStandardPEissuerIDTag
				
				break;
			
			case 0x82:  //theNonStandardPEcontentTag
				
				break;
			
			default:
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
		}
	}
	
	return TRUE;
}

#endif
