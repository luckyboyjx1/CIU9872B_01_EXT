/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   PECDMAParameter.c
*
* Description: 该文件主要是对PE-CDMAParameter的所有处理
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
#include  "profileApi.h"
#include  "Gsmpublic.h"

#if (EUICC_ONLY||SE_AND_EUICC)

/*********************PE-CDMAParameter START*****************************/

/* A-Key for CAVE Authentication (OCTET STRING SIZE(8)), */ 
//BYTE code theCDMAParameterPEAuthKeyTag []= {0x81};
///* 
//Optional value for ssd
//Bytes 1..8: value if shared secret data A 
//Bytes 9..16: value if shared secret data B 
// OCTET STRING (SIZE (16)) OPTIONAL, 
//*/ 
//BYTE code theCDMAParameterPESSDTag []= {0x82};
///* 
//	Shared Secrets for HRPD access authentication 
//	Includes the shared secret data. Length is variable 
//	OCTET STRING (SIZE (8..255)) OPTIONAL, 
//*/ 
//BYTE code theCDMAParameterPEhrpdAccessAuthenticationDataTag []= {0x83};
///* 
//Parameters for simple IP authentication 
//The shared secrets will be provided as a list of TLVs 
//For each NAI-entry-index which will be provided within CHAP command the  
//shared secret will be coded as follows: 
//Tag: NAI-entry-index, length, Shared secret 
//up to 16 shared secret values may be provided 
//OCTET STRING (SIZE (10..4112)) OPTIONAL, 
//*/ 
//BYTE code theCDMAParameterPEsimpleIPAuthenticationData []= {0x84};
//	/* 
//	Parameters mobile IP authentication 
//	The shared secrets will be provided as a list of Tag Value pairs 
//	For each NAI-entry-index which will be provided within MN-HA command the 
//	necessary values will be coded as follows: 
//	Tag: NAI-entry-index, length of MN-AAA-SS, MN-AAA-SS value, length of
//	MN-HA-SS, MN-HA-SS value 
//	up to 16 entries may be provided 
//	OCTET STRING (SIZE (19..8208)) OPTIONAL
//	*/ 
//BYTE code theCDMAParameterPEmobileIPAuthenticationData []= {0x85};
/*********************PE-CDMAParameter END*****************************/


/*
*@funtion    :CDMAParameterPEProcess
*
*@description: 对PE-CDMAParameter进行解析处理
*
*@param      :
*             
*@return     :BYTE    --TRUE:继续执行
*                     --FALSE:终止执行，直接返回
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE CDMAParameterPEProcess(void)
{
	BYTE  idata result;
	
	
	if(ValueBit(G_PEControlContext.IsExistFlag2,PE_Complete) == FALSE)//不够一个PE，将整个数据都备份
	{
		backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr,  G_PETlvInfo.bufferLen);	
		
		return TRUE;
	}	
	
	
	if(PESearchAIDName((BYTE *)&CSIMPartial[0]) == FALSE)//PE-CSIMParameters may be provided in the context of ADF_CSIM
	{
		appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
		return FALSE;
	}
	
	if(createSpecProfilePrivateFile(FALSE,0,0) == FALSE)
	{
		return FALSE;
	}
	
	while(1)
	{
		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
			break;
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
							
				//SetBit(G_CurLoadLTEAUTH,B_CAVE);
				break;

			case 0x81:  //authenticationKey OCTET STRING (SIZE(8)),	
				
			    if(G_PETlvInfo.findValueLen != 8)
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				    return FALSE;
				}
				
				VMReadBuf(&tempBuf[0],  G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen);			
				setKI(PROFILEDOWNMODE,&tempBuf[0], G_PETlvInfo.findValueLen);
		
				break;	

			
			case 0x82:  //ssd OCTET STRING (SIZE (16)) OPTIONAL	
				 if(G_PETlvInfo.findValueLen != 16)
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				    return FALSE;
				}
				break;
			

			case 0x83: // hrpdAccessAuthenticationData OCTET STRING (SIZE (2..32)) OPTIONAL,
				if((G_PETlvInfo.findValueLen < 2) || (G_PETlvInfo.findValueLen > 32))
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				    return FALSE;
				}
				
				VMReadBuf(tempBuf+2,G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff,G_PETlvInfo.findValueLen);
						
				setHRPD(tempBuf, G_PETlvInfo.findValueLen);	
		
				break;
			case 0x84: //simpleIPAuthenticationData OCTET STRING (SIZE (3..483)) OPTIONAL
			   
				if((G_PETlvInfo.findValueLen < 3) || (G_PETlvInfo.findValueLen > 483))
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				    return FALSE;
				}			
			
				VMReadBuf(&tempBuf[2],  G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen);				
				setCHAP(&tempBuf[0], G_PETlvInfo.findValueLen);	

				break;
			case 0x85: //mobileIPAuthenticationData OCTET STRING (SIZE (12..957)) OPTIONAL 
			
				if((G_PETlvInfo.findValueLen < 12) || (G_PETlvInfo.findValueLen > 957))//tempBuf的最大可用长度为减去3，前两个字节存放T和L，最后一个字节用来设置0，移位时使用
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				    return FALSE;
				}

				if(G_PETlvInfo.findValueLen > ((sizeof(tempBuf)-3)))
				{
					setMobileIpSsAddr(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff,G_PETlvInfo.findValueLen);
				}
				else
				{
					VMReadBuf(&tempBuf[2],  G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen);				
					setMobileIpSs(&tempBuf[0], G_PETlvInfo.findValueLen);
				}					

				break;
			
			default:
				appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
				return FALSE;
		}
		
	}
	
	return TRUE;

}

#endif

