
/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   PEFileSystem.c
*
* Description: 该文件主要是对模版PE的处理
*
* auther：     zxx    20170623
*
* history：    V1.0
*
************************************************************************************************************/


#include  "PETemplateFileProcess.h"
#include  "typedef.h"
#include  "fileAction.h"
#include  "gsmpublic.h"
#include  "fileSystem.h"
#include "tlvHeader.h"
#include  "ProfileInterpreter.h"
#include "profileApi.h"
#include "Interpreter.h"
#include "uicc_main.h"
#include "Stkhead.h"
#include  "VMAccessEeprom.h"
#include "opFunction.h"

#if (EUICC_ONLY||SE_AND_EUICC)





/*
*@funtion    :processFileResult
*
*@description: 根据文件操作返回的结果，进行相应的处理
*
*@param      :BYTE result     --      文件操作处理结果
*
*@return     :BYTE   --  TRUE/FALSE
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20190401
*            :V1.1 :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*            :V1.2 :modify by chentao@20211012
*            */
BYTE processFileResult(BYTE result)
{

	switch(result)
	{
		case 0xFE:
			return TRUE;
		case E_InvalidRequest:

			appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);//Installing profile with PE-USIM before PE-MF, eUICC reports error.eUICC responses with PEStatus (5) invalid-request-format

			break;
		case E_PINMissing:

			appendErrorResponse(PIN_CODE_MISSING,G_PEControlContext.PE_identification);//Installing profile with PE-USIM before PE-MF, eUICC reports error.eUICC responses with PEStatus (5) invalid-request-format

			break;

		case E_FileSize:
			appendErrorResponse(NOT_ENOUGH_MEMORY,G_PEControlContext.PE_identification);//Installing profile with PE-USIM before PE-MF, eUICC reports error.eUICC responses with PEStatus (5) invalid-request-format
#if RSP_EUICC
			G_ProfileErrorReason = ErrorReason_installFailedDueToInsufficientMemoryForProfile;
#endif
			break;
		case E_NotSupport:
			if(ValueBit(G_PEControlContext.IsExistFlag2,Mandated) == TRUE)//如果是强制的
			{
				appendErrorResponse(FETURE_NOT_SUPPORTED,G_PEControlContext.PE_identification);
			}
			else
			{
				appendPEStatusInfo(FETURE_NOT_SUPPORTED,G_PEControlContext.PE_identification); //拼警告，profile继续下载 todo:  每个文件都会拼装一次警告，需要修改
				return TRUE;
			}
			break;
		default:
			appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
			break;
	}

	//FALSE/E_Parameter
	//appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);//Installing profile with PE-USIM before PE-MF, eUICC reports error.eUICC responses with PEStatus (5) invalid-request-format
					
	return FALSE;
}




/*
*@funtion    :createMFFile
*
*@description: 安装完ISD-P之后，就创建3F00文件
*
*@param      :void
*@return     :BYTE
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170707
*/
BYTE createMFFile(void)
{
	UWORD xdata fileAddr;
	
	fileAddr = SelectPrivateTFFile(DFProfileId,TemplateFileId + 1);//选择MF模版
	
	if(fileAddr == FALSE) //预置ISD_P通过APDU指令创建私有文件
	{
		return 0xFE;
	}
	
	initTlvInfo(&G_TemplateTlvInfo,fileAddr+sizeof(TranFileHead),GetFileHeadWord(fileAddr, F_SpaceSize_Offset));
	
	initFindTlv(&G_TemplateTlvInfo);
	
	findTLV(&G_TemplateTlvInfo,0xFF,0xFF,1);		
	
	VMReadBuf(tempBuf,G_TemplateTlvInfo.bufferAddr+G_TemplateTlvInfo.findValueOff,G_TemplateTlvInfo.findValueLen);

	return profileCreateMF(tempBuf,G_TemplateTlvInfo.findValueLen);
}


//模版文件为2F7X,X为模版的最后一个字节
//ATTR---0：CreatedByDefault    1：NotCreatedByDefault
/*
*@funtion    :LoadTemplate
*
*@description: 用模版方式创建文件时，载入模版
*
*@param      : BYTE * startTag    --起始tag
*              BYTE *curTag       --当前tag
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE LoadTemplate(BYTE tempId)
{
	UWORD xdata fileAddr;
	
	tempId = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff+5);
	
	fileAddr = SelectPrivateTFFile(DFProfileId,TemplateFileId + tempId);

	if(G_profileControlContext.ProfileVersion < 0x22) //profile V 2.1  modify by chentao@20211103 兼容果通2.0版本的profile
	{
		if((tempId == MF_TEMPLATE_LAST_ID) ||(tempId == TELECOM_TEMPLATE_LAST_ID) || (tempId == PHONEBOOK_TEMPLATE_LAST_ID)  )
		{
			fileAddr = SelectPrivateTFFile(DFProfileId,(TemplateFileId_2_1 + tempId));
		}
	}

	
	if(fileAddr == FALSE)
	{
		if(ValueBit(G_PEControlContext.IsExistFlag2,Mandated) == TRUE)
		{
			return FALSE;
		}
		appendPEStatusInfo(TEMPLATE_NOT_SUPPORTED,G_PEControlContext.PE_identification);//拼装警告信息，继续执行
		return TRUE;
	}

	
	initTlvInfo(&G_TemplateTlvInfo,fileAddr+sizeof(TranFileHead),GetTFSize(fileAddr));//bufferAddr直接指向模版文件的文件体地址

	G_TempLen = 0;
	return TRUE;
}




/*
*@funtion    :createTemplateSubFunc
*
*@description: 提取createTemplateBeforeTagFile的公共函数
*
*@param      : BYTE i    --要查找的tag
*@return     :BYTE:TRUE/FALSE
*
*@author     :zxx
*
*@history    :V1.0 :create by zhxx@20190630
*            :V1.1 :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE createTemplateSubFunc(BYTE i)
{
	BYTE idata result;
	
	if(findTLV(&G_TemplateTlvInfo,i,0xFF,1))
	{


		VMReadBuf(tempBuf,G_TemplateTlvInfo.bufferAddr+G_TemplateTlvInfo.findValueOff,G_TemplateTlvInfo.findValueLen);
	
		result = profileCreateFile(tempBuf,G_TemplateTlvInfo.findValueLen,PROFILE_MODE_TEMPLATE);
		
		if(processFileResult(result) == FALSE)
		{
			return FALSE;
		}		
	}
    
    return TRUE;	
}


/*
*@funtion    :createTemplateBeforeTagFile
*
*@description: 依次创建模版文件中curTag之前的文件
*
*@param      : BYTE * startTag    --起始tag
*              BYTE *curTag       --当前tag
*@return     :BYTE:TRUE/FALSE
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE createTemplateBeforeTagFile(BYTE * startTag,BYTE curTag)//依次创建模版文件中curTag之前的文件
{
	
	BYTE idata i;

	if(G_fileControlContext.LoadDFAddr == FALSE)
	{
		appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
		return FALSE;
	}
	
	initFindTlv(&G_TemplateTlvInfo);
	
	if((curTag >= 0xA2) && (curTag <= 0xBE))
	{
		for(i = *startTag;i<curTag;i++)
		{
			if(createTemplateSubFunc(i) == FALSE)
			{
				return FALSE;
			}		
		}
	}
	else
	{
		if(*startTag >= 0x1F)
		{
			for(i = *startTag;i<curTag;i++)
			{
				if(createTemplateSubFunc(i) == FALSE)
				{
					return FALSE;
				}		
			}
		}
		else
		{
			for(i = *startTag;i<=0xBE;i++)
			{
				if(createTemplateSubFunc(i) == FALSE)
				{
					return FALSE;
				}		
			}
			for(i = 0x1F;i < curTag;i++)
			{
				if(createTemplateSubFunc(i) == FALSE)
				{
					return FALSE;
				}			
			}		
		}
		
	}
	
	if(curTag == 0xBE)
	{
		*startTag = 0x1F;
	}
	else
	{
		*startTag = curTag+1;
	}
	
	return TRUE;
}


/*
*@funtion    :createTemplateAfterTagFile
*
*@description: 依次创建模版文件中curTag之后的文件
*
*@param      : BYTE * startTag    --起始tag
*@return     :BYTE: TRUE/FALSE
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE createTemplateAfterTagFile(BYTE curTag)//依次创建模版文件中curTag之后的文件
{
	BYTE idata i,fileResult;
	SWORD idata result;
	

	if(G_fileControlContext.LoadDFAddr == FALSE)
	{
		appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
		return FALSE;
	}

	initFindTlv(&G_TemplateTlvInfo);
	
	if(curTag == 0xBE)  //0xBE是单字节tag的最大值，超过0xBE时，tag就变成两个字节的tag了，我们的做法是只存放第二个字节的序号值，即序号值从0x1F开始累加
	{
		i = 0x1F;
	}
	else
	{
		i = curTag+1;
	}
	
	for(;;i++)
	{
		if(i == 0xBF)//0xBE之后需要查找的是0x1F
		{
			i = 0x1F;
		}
		
		result = findTLV(&G_TemplateTlvInfo,i,0xFF,1);	
		
		if(result == FALSE)
		{
			 break;
		}
		
		VMReadBuf(tempBuf,G_TemplateTlvInfo.bufferAddr+G_TemplateTlvInfo.findValueOff,G_TemplateTlvInfo.findValueLen);

		fileResult = profileCreateFile(tempBuf,G_TemplateTlvInfo.findValueLen,PROFILE_MODE_TEMPLATE);
		
		if(processFileResult(fileResult) == FALSE)
		{
			return FALSE;
		}
	}
	return TRUE;
}
/*
*@description : 查找当前USIM /CSIM的AID Name
*
*@function    :PESearchAIDName
*
*@param        :BYTE *pidrx
*
*@return        :void
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20180206
*             :V1.1 :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/

BYTE code USIMPartialAID1[] = {0x10, 0xA0,0x00,0x00,0x00,0x87,0x10,0x02,0xFF,0x86,0xFF,0x03,0x89,0xFF,0xFF,0xFF,0xFF}; 

BYTE  PESearchAIDName(BYTE *pidrx)
{
	BYTE idata dirNum;
	BYTE xdata aidCmpBuffer[0x12];
	UWORD xdata fileAddr;
	
	if(G_fileControlContext.LoadADFAddr == FALSE)
	{
		return FALSE;
	}
	
	dirNum = GetFileHeadByte(G_fileControlContext.LoadADFAddr, F_DIRNum_offset);
	
	if(dirNum != 0)
	{
		fileAddr = SelectFile(G_CreatingMFOffset, G_profileControlContext.CURLOADDIR);

		if(fileAddr == FALSE)
		{
			 return FALSE;
		}
		
		ReadLFBuf(fileAddr,dirNum,0x02,&aidCmpBuffer[0],0x12);  //tempBuf[0]- 0x4F,tempBuf[1]-L tempBuf[2]-tempBuf[0x12] : V
		
		if(ecpMem_Compare(&aidCmpBuffer[2],&pidrx[1],pidrx[0]) == EQUAL)
		{
			return TRUE;
		}
	}
	return FALSE;
	
}


/*
*@description:2F00文件中 增加记录
*
*@param      :recNum:记录号
*
*@return     :void
*
*@history    :create by xyz@100513 8:59
*            :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE WriteDIRNum(BYTE *aidBuf)
{
	BYTE idata 	recNum,num;	
	UWORD xdata fileAddr;
	BYTE idata aidCmpBuffer[2];
	BYTE xdata tempData[0x14];

	fileAddr = SelectFile(G_CreatingMFOffset, G_profileControlContext.CURLOADDIR);

	if(fileAddr == FALSE)
	{
		 return 0;
	}
	recNum = GetLFRecNumLgth(fileAddr,FileNum);		

	for(num=0x01;num<=recNum;num++)
	{
		ReadLFBuf(fileAddr,num,0x02,&aidCmpBuffer[0],0x2);  //tempBuf[0]- 0x4F,tempBuf[1]-L tempBuf[2]-tempBuf[0x12] : V

		if(aidCmpBuffer[0] == 0xFF)
		{
			tempData[0] = 0x61;
			tempData[1] = aidBuf[0]+2;
			tempData[2] = 0x4F;
			ecpMem_Copy(tempData+3,&aidBuf[0],aidBuf[0]+1);
			
			WriteLFBuf(fileAddr,num,0,tempData,aidBuf[0]+4);
			
			return num;
		}		
	}
	return 0;	
}


/*
*@function   :SetProfileChannel
*
*@description:设置正在下载的profile的fileAddr和fileName
*
*@param       :UWORD fileAddr   DF/EF的地址
@param       :WORD fileName     DF/EF的文件名
*
*@return       :BYTE    true/false
*
*@auuthor      :zhxx
*
*@history      :V1.0 :create by zhxx@20190328
*              :V1.1 :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void SetProfileChannel(UWORD fileAddr,WORD fileName)
{
	BYTE idata fileType;


	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);


   	if(fileType == ADFTYPE )					//DF  ADF  MF
  	{ 		
   		G_fileControlContext.LoadDFName = fileName;

		G_fileControlContext.LoadDFAddr = fileAddr;

		G_fileControlContext.LoadEFAddr = FALSE;
		
		if(GetFileHeadByte(G_fileControlContext.LoadDFAddr,F_DIRNum_offset) != 0)
		{
			G_fileControlContext.LoadADFAddr = G_fileControlContext.LoadDFAddr;
		}
  	}
  	else 
	{
  		G_fileControlContext.LoadEFAddr = fileAddr;

  		G_fileControlContext.LoadEFName = fileName;
	}
}


/*
*@funtion    :PEFileSystemProcess
*
*@description: 对PE-Using Template进行解析处理
*
*@param      :NONE
*             
*@return     :BYTE    --TRUE:继续执行
*                     --FALSE:终止执行，直接返回
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*                V1.1  : modify by xwh 20170711,优化程序执行流程，提高执行效率
*                V1.2  :modify by xwh 20170711,优化程序执行流程，收到62 后直接创建文件，下一级的处理不再需要
*/
extern BYTE code USIMPartial[8] ;//= {0x07, 0xA0,0x00,0x00,0x00,0x87,0x10,0x02};  //101220  附录E

extern BYTE code CSIMPartial[8] ;//= {0x07, 0xA0,0x00,0x00,0x03,0x43,0x10,0x02};  //101220  附录E 

extern BYTE code ISIMPartial[8] ;//= {0x07, 0xA0,0x00,0x00,0x00,0x87,0x10,0x04};  //101220  附录E

/**
*modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*modify V1.3  : modify by zhangxiaoxiao @20220531 修改更新文件大小超过缓存文件导致profile下载失败的bug
*modify :modify by zhxx@20220629 HB_Offline_Profile_Test/14-PROFILE PE数据异常测试/Profile_FulSeg00b3：B9(CSIM)下面没有A6、AE(CSIM模板强制性文件)所以抛异常
*/
BYTE PEFileSystemProcess(BYTE tempId)
{
	BYTE  idata result,pos,fileResult,updateResult;
	WORD idata tempParameter;
	BYTE idata tempRec,recNum;
	
	
//	if((ValueBit(G_PEControlContext.IsExistFlag2,PE_Complete) == FALSE) && (ValueBit(G_PEControlContext.IsExistFlag2,OccurFlag) == FALSE) && (G_PETlvInfo.bufferLen < 0x0F))//来的数据不是完整的，且前面的数据太短，直接备份，返回，TY_OfflineProfileFormatTest/HEAD\异常测试/HEAD_E01很短，但是是完整的，期望不保存，继续执行
//	{
//		backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr,  G_PETlvInfo.bufferLen);	
//		
//		return TRUE;
//	}

	tempRec = 0;

	recNum = 0;


	result = TemplatePreProcess(tempId);

	if( result == FALSE)
	{
		appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
		return FALSE;
	}
	else if(result == 0xFF)
	{
		return TRUE;
	}

	result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
	

    while(result)
	{
		if((G_PETlvInfo.levelIndex)==0)
		{
			
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
				case Tag_thePEHeader:
					
					if(CheckTagOcc(G_PEControlContext.TagA0Occ++) == FALSE)
					{
						return FALSE;
					}
				
					if (PEHeaderAnalyze()== FALSE)  //处理PEHeader信息
					{
						return FALSE;
					}
					
					break;

				case Tag_theTemplateID:
					if(NvmCmp((BYTE*)(G_PETlvInfo.bufferAddr + G_PETlvInfo.findValueOff),(BYTE*)G_Common_Template_ID,sizeof(G_Common_Template_ID)) != 0)
					{
						appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
				
					  return FALSE;
					}
					
					if(LoadTemplate(tempId) == FALSE)
					{
						appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);

						return FALSE;
					}
					G_PEControlContext.StartTag = 0xA2;
					G_PEControlContext.CurTag = 0xA2;
					break;

				default:
					//G_profileControlContext.TotalLen = 0;
					if (G_PETlvInfo.tagBuffer[0] >= (Tag_thePEFile&0x1F))        //A2 ~ Ax
					{

						setVerticalFind(&G_PETlvInfo);
						G_profileControlContext.TotalLen = 0;//modify by zhangxiaoxiao @20220531 修改更新文件大小超过缓存文件导致profile下载失败的bug
			
						G_PEControlContext.CurTag = G_PETlvInfo.tagBuffer[0];
								
						G_TempLen = 0;

						if(tempId == CSIM_TEMPLATE_LAST_ID)
						{
							if(G_PETlvInfo.tagBuffer[0] == 0xA6)
							{
								SetBit(G_PEControlContext.IsExistFlag3,A6Tag);
							}

							if(G_PETlvInfo.tagBuffer[0] == 0xAE)
							{
								SetBit(G_PEControlContext.IsExistFlag3,AETag);
							}
						}
					
						//created by default file system template
						if((tempId == MF_TEMPLATE_LAST_ID) || (tempId == USIM_TEMPLATE_LAST_ID) || (tempId == ISIM_TEMPLATE_LAST_ID) || (tempId == CSIM_TEMPLATE_LAST_ID))//CreatedByDefault
						{
							if(createTemplateBeforeTagFile(&G_PEControlContext.StartTag,G_PEControlContext.CurTag) == FALSE)//依次创建模版文件中curTag之前的文件
							{
								return FALSE;
							}
						}
						
						initFindTlv(&G_TemplateTlvInfo);
						
						findTLV(&G_TemplateTlvInfo,G_PEControlContext.CurTag,0xFF,1);
						
						if(appendFindValue(&G_TemplateTlvInfo,FromValue) == FALSE)//将模版的数据追加到临时buffer中，用于修改
						{
							return FALSE;
						}
						
						makeDescript();
						G_fileControlContext.UpdateOffset = 0;
						
						ClearBit(G_PEControlContext.IsExistFlag2,FileDescriptorTag);
						
						G_PEControlContext.FileDescriptorTimes = 0;
						
						if(G_PETlvInfo.findValueLen == 0)
						{
							pos = profileCreateFile(tempBuf,G_TempLen,PROFILE_MODE_TEMPLATE);
							
							if(processFileResult(pos) == FALSE)
							{
								return FALSE;
							}
						}
                                              						
					}
			}
		}
		else if((G_PETlvInfo.levelIndex)==1)
		{
			
			if(G_PETlvInfo.tagBuffer[0] >= (Tag_thePEFile&0x1F))
			{
				
				if(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] != Tag_thefillFileContent) //modify by zhangxiaoxiao @20220531 修改更新文件大小超过缓存文件导致profile下载失败的bug
				{
					if(G_PETlvInfo.isCompleteBuffer[G_PETlvInfo.levelIndex] == FALSE)//处理的tag不完整
					{
						backData( EUICCBackPEBuf, G_PETlvInfo.bufferAddr + G_PETlvInfo.findTagOff,G_PETlvInfo.bufferLen-G_PETlvInfo.findTagOff);

						return TRUE;
					}
				}

				if(G_PETlvInfo.isCompleteBuffer[G_PETlvInfo.levelIndex] == TRUE)  //modify by zhangxiaoxiao @20220531 修改更新文件大小超过缓存文件导致profile下载失败的bug
				{
					G_profileControlContext.TotalLen += G_PETlvInfo.findTLVLen;
				}
				else
				{
					G_profileControlContext.TotalLen += (G_PETlvInfo.bufferLen-G_PETlvInfo.findTagOff);
				}


				if(G_PETlvInfo.tagBuffer[1] == Tag_thefileDescriptor)
				{
					G_TempLen = 0;
					
					if(appendFindValue(&G_TemplateTlvInfo,FromValue) == FALSE)//将模版的数据追加到临时buffer中，用于修改
					{
						return FALSE; 
					}
					
					G_fileControlContext.UpdateOffset = 0;
					
					if( ValueBit(G_PEControlContext.IsExistFlag2,FileDescriptorTag) == TRUE)//Ax下面有多个A1，但是又没有fileid的情况
					{
						tempRec = FindTag(tempBuf,0x83,G_TempLen);
						
						if(tempRec != FALSE)
						{
							tempParameter = BuildWord(tempBuf+tempRec+1);//此时文件ID根据出现的次数依次相加
							tempParameter += G_PEControlContext.FileDescriptorTimes;
							SetWord(tempBuf+tempRec+1,tempParameter);
						}
					}
					
					SetBit(G_PEControlContext.IsExistFlag2,FileDescriptorTag);
					G_PEControlContext.FileDescriptorTimes ++;
				}
								
				//G_profileControlContext.TotalLen += G_PETlvInfo.findTLVLen;
				
				switch(G_PETlvInfo.tagBuffer[1])
				{
					case Tag_thefileDoNotCreate:
						break;

					case Tag_thefileDescriptor:							
					 								
						setVerticalFind(&G_PETlvInfo);	
						
						if(replaceTLVFromTLVList() == FALSE)
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
							return FALSE;
						}
						
						if(G_PETlvInfo.tagBuffer[0] == 0xA2) //创建USIM, ISIM,CSIM ADF
						{
							if((tempId == USIM_TEMPLATE_LAST_ID) || (tempId == ISIM_TEMPLATE_LAST_ID) || (tempId == CSIM_TEMPLATE_LAST_ID))
							{
								pos = FindTag(tempBuf,0x84,G_TempLen);//创建USIM, ISIM,CSIM ADF 缺少84
								if((pos == FALSE) ||(tempBuf[pos] == 0))
								{
									appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
									return FALSE;
								}

								pos = FindTag(tempBuf,0xC6,G_TempLen);//创建USIM, ISIM,CSIM ADF 缺少C6
								if(pos == FALSE)
								{
									appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
									return FALSE;
								}
							}
						}

						pos = FindTag(tempBuf,Tag_fId,G_TempLen);

						if((G_PETlvInfo.tagBuffer[0] == 0xA5) && (tempId == MF_TEMPLATE_LAST_ID)) //创建2F00
						{
							G_profileControlContext.CURLOADDIR = BuildWord(tempBuf+pos+1);
						}
						

						if((pos != FALSE)&&(BuildWord(tempBuf+pos+1) == 0x3F00))
						{
							if( profileModifyMFInfo(tempBuf,G_TempLen) == FALSE)//resize上一个File中的文件，即resize MF文件，因为MF文件在安装ISD-P的时候已经创建
							{
								appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
								return FALSE;
							}
							SetBit(G_profileControlContext.IsCreateFlag,CREATE_MF);
						}
						else
						{
							pos = FindTag(tempBuf,0x84,G_TempLen);
							
							if(pos != FALSE)
							{
								tempRec = GetAIDNumber((tempBuf+pos),TRUE); //2F00 里面没有的ADF，主动添加进去
								if(tempRec == FALSE) //为了应付中国电信甜瑜离线脚本测试，CSIM ADF在2F00里面没有导致选择CSIM失败的问题
								{
									recNum = WriteDIRNum(tempBuf+pos);
								}								
							}
							
														
							pos = FindTag(tempBuf,0x82,G_TempLen);              //File Type

							pos = (tempBuf[pos+1]&FILETYPE); 
													
							if(( ValueBit(G_PEControlContext.IsExistFlag2,FirstDFFlag) == TRUE)&&(pos == ADFTYPE))   //如果已经创建了模板DF ，则对于模板中其他的DF路径都位于模板DF之下
							{
								SetProfileChannel(G_fileControlContext.FirstDFAddr,G_fileControlContext.FirstDFName);
							}

							tempRec = profileCreateFile(tempBuf,G_TempLen,PROFILE_MODE_TEMPLATE);
							
							if(processFileResult(tempRec) == FALSE)
							{
								return FALSE;
							}
							
							
							if( ValueBit(G_PEControlContext.IsExistFlag2,FirstDFFlag) == FALSE)
							{
								if((ValueBit(G_PEControlContext.IsExistFlag2,DFFlag) == TRUE)&&(pos == ADFTYPE))  //如果当前模板是DF 模板，则保存第一个df的路径信息
								{		
									SetBit(G_PEControlContext.IsExistFlag2,FirstDFFlag);
									
									G_fileControlContext.FirstDFAddr = G_fileControlContext.LoadDFAddr;
									
									G_fileControlContext.FirstDFName = G_fileControlContext.LoadDFName;
									
									if(recNum != 0)
									{										
										VMSetBuf(G_fileControlContext.FirstDFAddr + F_DIRNum_offset,recNum,sizeof(BYTE));
										recNum = 0;
									}
								}
							}
						
						}
								
						G_TempLen = 0;									

						break;

					case Tag_thefillFileOffset:		

                        if(G_PETlvInfo.findValueLen > 2)
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					
							return FALSE;
						}							
			
					    if(G_PETlvInfo.findValueLen == 2)
						{
					    	G_fileControlContext.UpdateOffset += VMReadShort(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
						}
						else
						{
							G_fileControlContext.UpdateOffset += VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
						}

						break;

					case Tag_thefillFileContent://若不存在Tag_thefileDescriptor，则需要先创建，再更新

						if(G_TempLen > 0)
						{
							G_TempLen = 0;
							if(appendFindValue(&G_TemplateTlvInfo,FromValue) == FALSE)//将模版的数据追加到临时buffer中，用于修改
							{
								return FALSE;
							}
						}
						
						fileResult = profileCreateFile(tempBuf,G_TempLen,PROFILE_MODE_TEMPLATE);
					
						if(processFileResult(fileResult) == FALSE)
						{
							return FALSE;
						}
						
						G_TempLen = 0;						
						updateResult = makeUpdate();					
						if((updateResult & 0x0F) == FALSE)//数据不匹配错误/数据更新错误
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
#if RSP_EUICC
							if((updateResult & 0x80) == 0x80)
							{
								G_ProfileErrorReason = ErrorReason_installFailedDueToDataMismatch;
							}							
#endif
							return FALSE;
						}	

						ClearBit(G_PEControlContext.IsExistFlag2,ContentRequired);

						break;

					default: //白盒测试添加
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
		else
		{
			// 白盒测试添加
		}
				
		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if((result==FALSE) && (G_PETlvInfo.findNextLen > 0))
		{
			backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr + G_PETlvInfo.findNextOff,  G_PETlvInfo.bufferLen-G_PETlvInfo.findNextOff);			
			return TRUE;
		}
		
		while (result==FALSE)
		{
			if(setReturnFind(&G_PETlvInfo) == FALSE)
			{
				break;
			}
			result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
			
			if((result==FALSE) && (G_PETlvInfo.findNextLen > 0))
			{
				backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr + G_PETlvInfo.findNextOff,  G_PETlvInfo.bufferLen-G_PETlvInfo.findNextOff);	
				
				return TRUE;
			}
		}

	}   

	if(G_PETlvInfo.findNextLen > 0)
	{
		backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr + G_PETlvInfo.findNextOff,  G_PETlvInfo.bufferLen-G_PETlvInfo.findNextOff);	
		
		return TRUE;
	}
	
	
	if(TemplatePEEndProcess(tempId) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

#endif
