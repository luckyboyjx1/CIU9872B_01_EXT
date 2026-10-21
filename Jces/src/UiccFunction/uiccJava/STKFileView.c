/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCToolkitPublic.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2013-10-23

*Description :该文件主要用于存放实现SIM API/UICC API 文件系统中的一些公共函数
*
*History     :
	
author    time      version    description
xuyz   2015/10/30   V1.0       创建该文件
************************************************************************************************************************************************************************************************/
#include "STKFileView.h"
#include "fileSystem.h"
#include "fileAction.h"
#include "gsmpublic.h"
#include "ToolkitAPIPublic.h"
//#include "main.h"
//#include "ChannelManagement.h"
//#include "JCREMain.h"

#if UICC__FUNCTION__SIMAPI
#include "SIMToolkitConstant.h"
#include "UICCToolkitAPI.h"
#endif

#if UICC__FUNCTION__UICCAPI
#include "UICCToolkitConstants.h"
#endif

BYTE    xdata    G_ToolkitFileSystemMode; 	 //中间变量，在处理UICCAPI/SIMAPI的时候使用
WORD    xdata    G_ToolkitCurrentAdfName;	 //中间变量，在处理UICCAPI/SIMAPI的时候使用
WORD    xdata    G_ToolkitCurrentDfName;	 //中间变量，在处理UICCAPI/SIMAPI的时候使用
UWORD   xdata    G_ToolkitCurrentDfAddr;	 //中间变量，在处理UICCAPI/SIMAPI的时候使用
WORD    xdata    G_ToolkitCurrentEfName;	 //中间变量，在处理UICCAPI/SIMAPI的时候使用
UWORD   xdata    G_ToolkitCurrentEfAddr; 	 //中间变量，在处理UICCAPI/SIMAPI的时候使用
BYTE    xdata    G_ToolkitCurrentRecNum; 	 //中间变量，在处理UICCAPI/SIMAPI的时候使用


/*
*@function    :Toolkit_ComputeBinary
*
*@description :计算当前操作binary文件的权限 
*
*@param       :action
*@param       :fileOffset
*@param       :length
*
*@history     :TRUE/FALSE
*
*@author      :xuyz
*
*@history     :V1.0 create by xuyz@20131107
			  :V1.1 :modify by lgy@20140225
*/
BYTE Toolkit_ComputeBinary(BYTE action,jshort fileOffset,jshort length, jshort apiMode)
{
	BYTE idata fileType,fileStatus;
	WORD  idata spaceSize;
	UWORD xdata actualFileAddr;

	if(G_ToolkitCurrentEfAddr == FALSE)
	{
		UiccOrSimViewThrowException(NO_EF_SELECTED, apiMode);	  	//抛出异常 	NO_EF_SELECTED
		
		return FALSE;
	}

	fileType = GetFileHeadByte(G_ToolkitCurrentEfAddr,F_Type_Offset);

	if(fileType != TRANTYPE)
	{
		UiccOrSimViewThrowException(COMMAND_INCOMPATIBLE, apiMode);	 	//抛出异常 COMMAND_INCOMPATIBLE	
		
		return FALSE;
	}

	actualFileAddr = SearchFile(G_ToolkitCurrentDfAddr, G_ToolkitCurrentEfName);	//获取文件的实际地址，防止文件时映射文件，而修改到了映射文件的状态

	fileStatus	= GetFileHeadByte(actualFileAddr,F_Status_Offset);

	if((fileStatus&0x01) == FILE_INVALIDATE)//该处要注意有些文件可能 要求文件失效的时候也可以读取
	{
		if(apiMode == UICCAPI_Mode)
		{
#if UICC__FUNCTION__UICCAPI
		    UICCThrowUICCException(REF_DATA_INVALIDATED);	  	//抛出异常 REF_DATA_INVALIDATED
#endif
		    return FALSE;
		}
		else
		{
			if((fileStatus&0x40) == FILE_INVALIDATE)  //USIM文件系统的bit6表示失效后读写权限， 20130227 lj
			{
#if UICC__FUNCTION__SIMAPI
				 ThrowSIMViewException(INVALIDATION_STATUS_CONTRADICTION_SIM);	  //b1=0: invalidated; b3=0: not readable or updatable when invalidated
#endif
				 return FALSE;
			}
		}
	}

	spaceSize = GetFileHeadWord(G_ToolkitCurrentEfAddr,F_SpaceSize_Offset);	//获取透明文件的空间大小

	if((spaceSize < (fileOffset + length))||(fileOffset < 0))
	{
		UiccOrSimViewThrowException(OUT_OF_FILE_BOUNDARIES, apiMode);	  	//抛出异常 OUT_OF_FILE_BOUNDARIES
		
		return FALSE;
	}
	
    if(Toolkit_GetKeyStatus(G_ToolkitCurrentEfAddr,action, apiMode) == FALSE)
    {
		UiccOrSimViewThrowException(SECURITY_STATUS_NOT_SATISFIED, apiMode);   	//抛出异常 	SECURITY_STATUS_NOT_SATISFIED
		
	    return FALSE;
    }
	
	return TRUE;
}

/*
*@function     :Toolkit_SetChannel
*
*@description  ：设置调用UICC API的toolkit app的当前df，ef信息
*
*@param        :fileAddr
*@param        :fileName
*@param        :option	:TRUE:是否需要获取当前DF/EF信息
*
*@return        :responseLgth
*
*@author       :xuyz
*
*@history      :V1.0 :create by xuyz@20131107
*              :V1.1 :modify by xuxiaojun@20190609 调整G_ToolkitCurrentRecNum赋值的位置，因为后续UICCToolkitCopyNativeContextToJavaContext使用该变量时并未区分ADF/EF   <个人esim测试脚本-移动测试版-明文\SE访问控制应用测试>
*/
BYTE Toolkit_SetChannel(UWORD fileAddr,WORD fileName,BYTE option, jshort apiMode)
{
  BYTE idata fileType;
	BYTE idata returnLgth;
	UWORD xdata backDFAddr,backEFAddr;
	WORD idata backDFName,backEFName;

	returnLgth = 0x00;    //初始化当前选中文件的响应值返回长度

   	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);
	
	G_ToolkitCurrentRecNum = 0x00; 

   	if( fileType == ADFTYPE )					//DF  ADF  MF
  	{ 
		G_ToolkitCurrentDfName = fileName;

		G_ToolkitCurrentDfAddr = fileAddr;
											   
		G_ToolkitCurrentEfName = FALSE;

		G_ToolkitCurrentEfAddr = FALSE;
   	
		fileType = GetFileHeadByte(fileAddr,F_DIRNum_offset);

		if(fileType != 0x00)
		{
			G_ToolkitCurrentAdfName = fileName; 
		}
	

		if(option == TRUE)
		{
			backDFAddr =  G_CurrentDfAddr[G_ChannelNum];

			backDFName =  G_CurrentDfName[G_ChannelNum];

			G_CurrentDfAddr[G_ChannelNum] = G_ToolkitCurrentDfAddr;

			G_CurrentDfName[G_ChannelNum] = G_ToolkitCurrentDfName;

			if(apiMode == UICCAPI_Mode)
			{
#if UICC__FUNCTION__UICCAPI
				returnLgth = BaseUsimGetDfInformation(tempBuf);
#endif
			}

			else
			{
#if UICC__FUNCTION__SIMAPI
				returnLgth = GetDfInformation(tempBuf+100);// 20140228 添加对DF信息处理	
#endif
			}
	
			G_CurrentDfAddr[G_ChannelNum] = backDFAddr;

			G_CurrentDfName[G_ChannelNum] = backDFName;
			
		}
  	}
  	else 
	{
   		G_ToolkitCurrentEfName = fileName;

   		G_ToolkitCurrentEfAddr = fileAddr;
		
#if UICC__FUNCTION__SIMAPI
		if((fileType == CYCTYPE)&&(apiMode == SIMAPI_Mode))
		{
			G_ToolkitCurrentRecNum = 1;
		}
#endif		
		if(option == TRUE)
		{
			backDFAddr =  G_CurrentDfAddr[G_ChannelNum];

			backEFAddr =  G_CurrentEfAddr[G_ChannelNum];

			backEFName =  G_CurrentEfName[G_ChannelNum];

			G_CurrentDfAddr[G_ChannelNum] = G_ToolkitCurrentDfAddr;

			G_CurrentEfName[G_ChannelNum] =	G_ToolkitCurrentEfName;

			G_CurrentEfAddr[G_ChannelNum] = G_ToolkitCurrentEfAddr;

			if(apiMode == UICCAPI_Mode)
			{
			   returnLgth = BaseUsimGetEfInformation(tempBuf);
			}
#if UICC__FUNCTION__SIMAPI
			else
			{
				returnLgth = GetEfInformation(tempBuf+100);	
			}

#endif			
			G_CurrentEfAddr[G_ChannelNum] =  backEFAddr;

			G_CurrentEfName[G_ChannelNum] = backEFName;	

			G_CurrentDfAddr[G_ChannelNum] = backDFAddr;
		}
	}
	return  returnLgth;
}
/*
**function    :ToolkitSelectByFileId
*
*@param       :fid
*
*@return      :fileAddr/FALSE
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20131111
*/
UWORD ToolkitSelectByFileId(jshort fileName)
{
	BYTE idata type;
	WORD idata  fileParentName;
	UWORD xdata fileAddr;

	if(G_ToolkitCurrentDfAddr == FALSE)
	{
		return FALSE;
	}
	if(fileName == 0x7FFF)
	{
		if((G_ToolkitFileSystemMode&0xA1) == UICCFileSystem)
		{
			return FALSE;
		}
	}

	if(fileName == 0x3F00)
	{
		if((G_ToolkitFileSystemMode&0xA1) == ADFFileSystem)
		{
			return FALSE;
		}
	}

  if((fileName == 0x7FFF)||(fileName == 0x3F00))
	{
	  fileName = ((fileName == DF_MF) ? DF_MF : G_ToolkitCurrentAdfName);
			
		fileAddr = SelectFile(E_FILE_START,fileName);              //是否为MF或当前应用的ADF 

		return fileAddr;
	}

	fileAddr = SelectFile(G_ToolkitCurrentDfAddr,fileName);     //是否是当前DF或当前DF下的文件

	if(fileAddr == FALSE)
	{
		fileParentName = GetFileHeadWord(G_ToolkitCurrentDfAddr, F_SecondParentName_Offset);

		if((fileParentName == 0x3F00)&&((G_ToolkitFileSystemMode&0xA1) == ADFFileSystem))    //当前选择的文件不能为某个ADF的父文件或ADF的兄弟文件
		{
			return FALSE;
		}
	
		fileAddr = GetParentOrMapFileAddr(G_ToolkitCurrentDfAddr,F_ParentName_Offset);  //是否为当前DF的父文件
	
		if(fileParentName != fileName)
		{
			fileAddr = SelectFile(fileAddr,fileName);
			
			if(fileAddr != FALSE)                                		//是否是当前文件的兄弟文件
			{
			    type = GetFileHeadByte(fileAddr,F_Type_Offset);
				
				if(type !=  DFTYPE)
				{
					fileAddr = FALSE;
				}
				else
				{
					type = GetFileHeadByte(fileAddr,F_DIRNum_offset);

					if( G_ToolkitFileSystemMode != SIMFileSystem)  //SIMFileSystem下可以访问卡上的所有文件
					{
						if(type != 0x00)	 //当前文件的兄弟文件不可能为ADF：这种情况的出现可能是由于访问的是MF/DF，而DF的兄弟文件为某个ADF ，对于Toolkit是禁止的
						{
							fileAddr = FALSE;
						}
					}
				}
			}
	    }
	}

	
	return fileAddr;	
}
/*
*@function    :Toolkit_GetAccessDomainInfo
*
*@description :获取当前MF or ADF的access Domain 信息
*
*@param        :offset
*@param        :domainBuffer
*@param        :arrNum   2F06/6F06记录号
*@param        :recNum   6F80/ appEntryIndex
*
*@return       :TRUE:找到合适的权限访问条件/FALSE：未找到
*
*@author       :xuyz
*
*@history      :V1.0 :create by xuyz@20131107
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
BYTE Toolkit_GetAccessDomainInfo(WORD offset, BYTE *domainBuffer, BYTE arrNum,BYTE recNum)
{
	BYTE idata i;
	UWORD xdata toolkitAppAddr;
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(recNum));
#else
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
	if(toolkitAppAddr == FALSE)
	{		
		return FALSE;
	}

	ReadLFBufForJavaAccess(toolkitAppAddr,recNum,offset,&tempBuf[0] ,sizeof(BYTE));
	
	ReadLFBufForJavaAccess(toolkitAppAddr,recNum,offset+1,&tempBuf[1] ,tempBuf[0]);

	for(i=1; i< tempBuf[0]; i+= 0x06)
	{
		if(tempBuf[i] == arrNum)
		{
			ecpMem_Copy(domainBuffer, &tempBuf[i+1], 0x05);

			return TRUE;
		}
	}

	return FALSE;
}

/*
*@function   :Toolkit_GetKeyStatus
*
*@param      :fileAddr
*@param      :action
*
*@return     :当前文件的访问权限:90/97/others
*
*@author    :xuyz
*
*@history   :V1.0 :create by xuyz@20131107
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
BYTE Toolkit_GetKeyStatus(UWORD fileAddr,BYTE action, jshort apiMode)
{
	BYTE  idata fileType;
	BYTE xdata accessDomain[0x05];  //对于MF下的文件系统，读取更新权限使用5个字节表示，创建权限使用5个字节表示
	BYTE idata recNum,arrNum,result;
	BYTE xdata tmp[0x04];
	UWORD xdata backCurrentDfAddr;
	WORD xdata backCurrentDFName,keyStatus;  //keyStatus为了支持APDU或权限/与 类型改为WORD
	UWORD xdata toolkitAppAddr;

	if(bRamBuffer[PROFILE_APDU_MODE] == TRUE) // add by chentao20240124为了提高COS兼容性，profile下载过程中不检查
	{
		return TRUE;
	}

#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{		
		return FALSE;
	}
#endif
	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

	if( fileType != ADFTYPE )
	{
		fileAddr = SearchFile(G_ToolkitCurrentDfAddr, G_ToolkitCurrentEfName);	

		if(fileAddr == FALSE)
		{
			return FALSE;
		}
	}

	arrNum = 0x00;	   //默认为访问MF下的文件

	if((G_ToolkitFileSystemMode&0xA1) == ADFFileSystem)
	{
		tempBuf[0] = GetCurrentADFAid(G_ToolkitCurrentAdfName,&tempBuf[1]);  //获取当前toolkit app所访问的ADF AID
	
	    arrNum = GetAIDNumber(&tempBuf[2],FALSE);
	
		if(arrNum == FALSE)
		{
			return FALSE;
		}
	}

	recNum = G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel];

	if(recNum == FALSE)  //权限文件不存在，则直接返回FALSE，无法操作文件
	{	
		return FALSE;
	}

	if(G_ToolkitFileSystemMode >= AdminADFFileSystem ) //当前要操作的是admin access权限
	{
		result =  Toolkit_GetAccessDomainInfo(AdminAccessLgth_OFFSET,accessDomain,arrNum,recNum);

		if(result == FALSE)
		{
			return FALSE;
		}
	}
	else
	{
		if(G_ToolkitFileSystemMode == ADFFileSystem)  //访问ADF下的文件
		{
			result =  Toolkit_GetAccessDomainInfo(ADFAccessDomainLgth_OFFSET,accessDomain,arrNum,recNum);

			if(result == FALSE)
			{
				return FALSE;
			}
		}
		else   //访问SIM API 下的文件系统或 uiccfilesystem
		{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
			toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(recNum));
#endif
			ReadLFBufForJavaAccess(toolkitAppAddr,recNum,MFAccessDomainLength_OFFSET,&accessDomain[0] ,sizeof(BYTE));

			ReadLFBufForJavaAccess(toolkitAppAddr,recNum,MFAccessDomainContent_OFFSET,&accessDomain[1] ,accessDomain[0]);
		}
	}

	//todo :先备份当前UICC APDU级的currentEF currentDF 等信息
	backCurrentDFName = G_CurrentDfName[G_ChannelNum];
	backCurrentDfAddr = G_CurrentDfAddr[G_ChannelNum];

	G_CurrentDfName[G_ChannelNum] = G_ToolkitCurrentDfName;
	G_CurrentDfAddr[G_ChannelNum] = G_ToolkitCurrentDfAddr;
		
	keyStatus = GetAcessPurview(fileAddr,action);

	G_CurrentDfName[G_ChannelNum] = backCurrentDFName;
	G_CurrentDfAddr[G_ChannelNum] = backCurrentDfAddr;
	
	if(keyStatus == FALSE)
	{
#if UICC__FUNCTION__SIMAPI
		if((apiMode == SIMAPI_Mode)&&(action ==A_IncreaseFile))
		{
			ThrowSIMViewException(FILE_INCONSISTENT_SIM);	//SIMViewEXCEPTION_FILE_INCONSISTENT
		}
#endif
		return FALSE;
	}

	if(accessDomain[0] == 0x00)
	{
		if(bRamBuffer[PROFILE_APDU_MODE] == TRUE) // add by chentao20220808 德国测试有profile 中APPlication的EA参数没有81tag，但是安装应用过程中processdata调用了UICCAPI的updateBinary
		{
			return TRUE;//测试用例：德国外场问题profile下载测试_RSP/profile下载删除-20220701
		}

		return FALSE;
	}
	else if(accessDomain[0] == 0x01)
	{
		if(accessDomain[1] == 0x00)   //full access	to fileSystem
		{
			if(keyStatus==0x0097)
			{
				return FALSE;
			}
			
			return TRUE;
		}
		else if(accessDomain[1] == 0xFF)  //No access to fileSystem
		{
			return FALSE;
		}
	}
	else if(accessDomain[0] == 0x04)
	{
		if(accessDomain[1] == 0x02)
		{
			if(keyStatus == 0x0090)  //always
			{
				return ValueBit(accessDomain[4],2);	
			}
			else if(keyStatus <= 0x0008)  //PIN1 :APP1 - APP8
			{
				return ValueBit(accessDomain[2],(keyStatus -1));
			}
			else if(((keyStatus&0xF000)==0xC000)||((keyStatus&0xF000)==0xD000))	  //OR  or AND 
			{
				ecpMem_Set(tmp, 0x00,sizeof(tmp));

				if((keyStatus&0x0001) == 0x0001)  //PIN1
				{
					tmp[0] = ValueBit(accessDomain[2],0);	
				}

				if((keyStatus&0x0002) == 0x0002)   //PIN2
				{
					tmp[1]  = ValueBit(accessDomain[4],3);	
				}

				if((keyStatus&0x0004) == 0x0004)  //ADM1
				{
					tmp[2]  = ValueBit(accessDomain[3],0);	
				}

				if((keyStatus&0x0008) == 0x0008)  //ADM2
				{
					tmp[3]  = ValueBit(accessDomain[3],1);	
				}

				for(result =0 ;result <4 ;result ++)
				{
					if(ValueBit(keyStatus,result))
					{
						if((keyStatus&0xF000)==0xC000)  //OR
						{
							if(tmp[result] == TRUE)
							{
								return TRUE;
							}
						}
						else 
						{
							if(tmp[result] == FALSE)
							{
								return FALSE;
							}
						}
					}
				}

				if((keyStatus&0xF000)==0xC000)  //OR
				{
					return FALSE;
					
				}
				else 
				{
					return TRUE;
				
				}

			}
			else if(((keyStatus&0x00F0)==0x0080)&&(keyStatus<0x008A))	   //pin2
			{
				return ValueBit(accessDomain[4],3);
			}   
			else if((keyStatus&0x0F) >= 0x0A)	//0A-0E:ADM1- ADM5  8A-8E: ADM6 -ADM10
			{
				if((keyStatus >= 0x0A)&&(keyStatus < 0x0F))	 // A-E:ADM1 ADM2 ADM3 ADM4 ADM5
				{
					return ValueBit(accessDomain[3],(keyStatus -0x0A));	
				}
				else if((keyStatus >= 0x8A)&&(keyStatus <= 0x8C))
				{
					return ValueBit(accessDomain[3],(keyStatus - 0x85));	
				}
				else  if((keyStatus >= 0x8D)&&(keyStatus <= 0x8E))
				{
					return ValueBit(accessDomain[4],keyStatus - 0x8D);	
				}			
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
	}
	else if(accessDomain[0] == 0x03)           //APDU access mechanism, 只用于 SIM API 模式
	{
		if(accessDomain[1] == 0x01)
		{
			if(keyStatus == 0x0090)  //always
			{
				return ValueBit(accessDomain[3],0);	 //Byte 2, b1
			}
			else if(keyStatus <= 0x0008)  //PIN1 :APP1 - APP8
			{
				return ValueBit(accessDomain[3],1); //Byte 2, b2
			}
			else if(((keyStatus&0x00F0)==0x0080)&&(keyStatus<0x008A))	   //pin2
			{
				return ValueBit(accessDomain[3],2); //Byte 2, b3
			}   
			else if((keyStatus&0x000F) >= 0x000A)	//0A-0E:ADM1- ADM5  8A-8E: ADM6 -ADM10
			{
				if((keyStatus >= 0x0A)&&(keyStatus <= 0x0D))	 // A-D:ADM1 ADM2 ADM3 ADM4 ,对应于SIM  ADM 0-3
				{
					return ValueBit(accessDomain[3],(keyStatus -0x06)); //Byte 2, b5-b8
				}
				else if(keyStatus == 0x0E) //E, ADM 5,对应于SIM  ADM 4
				{
					return ValueBit(accessDomain[2],0); //Byte 1, b1
				}
				else if((keyStatus >= 0x8A)&&(keyStatus <= 0x8E)) //8A-8E,ADM6 -ADM10,对应于SIM  ADM 5-9
				{
					return ValueBit(accessDomain[2],(keyStatus&0x0F)-0x09);	//Byte 1, b2-b6
				}	
			}
		}
	}	

	return  FALSE ;

}
/*
*@function    :UICCToolkit_computeRecord
*
*@description :uicc api操作EF文件时判断其权限,并返回当前记录号
*
*@param       :mode,
*@param       :recNumber
*@param       :recOffset
*@param       :action
*
*@return      :recNo/FALSE
*
*@author       :xuyz
*
*@history      :V1.0 :create by xuyz@20131109
			   :V1.1 :modify by lgy@20140225
*/
BYTE Toolkit_computeRecord(jshort recNumber,jbyte mode,jshort recOffset,jshort respLength,BYTE action, jshort apiMode)
{	
	RecordFileHead xdata  fileHead;
	UWORD xdata actualfileAddr;
	BYTE idata status;

	if(G_ToolkitCurrentEfAddr == FALSE)
	{
		UiccOrSimViewThrowException(NO_EF_SELECTED,apiMode);
		
		return FALSE;
	}

	if(recOffset < 0x00)
	{
		UiccOrSimViewThrowException(OUT_OF_RECORD_BOUNDARIES,apiMode);
				
		return FALSE;

	}

	if((mode <REC_ACC_MODE_NEXT)||(mode >REC_ACC_MODE_ABSOLUTE))
	{
		UiccOrSimViewThrowException(INVALID_MODE,apiMode);
		
		return FALSE;
	}

	ReadEEPromCache((BYTE xdata *)&fileHead.FileId,G_ToolkitCurrentEfAddr,sizeof(RecordFileHead));

	if(((fileHead.FileType&0x07) != LINTYPE)&&((fileHead.FileType&0x07) != CYCTYPE )) //线形记录文件或者循环记录文件
	{
		UiccOrSimViewThrowException(COMMAND_INCOMPATIBLE,apiMode);
		
		return FALSE;
	}
	
	actualfileAddr = SearchFile(G_ToolkitCurrentDfAddr, G_ToolkitCurrentEfName);
	
	status = GetFileHeadByte(actualfileAddr,F_Status_Offset);  //若当前操作的是映射文件，则访问映射文件本身的状态，而不是访问实体文件的状态
	
	if((status&0x01) == FILE_INVALIDATE)		//判断文件是否有效
	{
		if(apiMode == UICCAPI_Mode)
		{
#if UICC__FUNCTION__UICCAPI
		    //抛出异常 REF_DATA_INVALIDATED
		    UICCThrowUICCException(REF_DATA_INVALIDATED);
#endif
		}
		else
		{
			if((status&0x40) == FILE_INVALIDATE) //USIM文件系统的bit6表示失效后读写权限， 20130227 lj
			{
#if UICC__FUNCTION__SIMAPI
			  	ThrowSIMViewException(INVALIDATION_STATUS_CONTRADICTION_SIM);
#endif
			}
		}
		
		return FALSE;

	}
	
	if((recOffset + respLength) > fileHead.RecLength)
	{
		UiccOrSimViewThrowException(OUT_OF_RECORD_BOUNDARIES,apiMode);
		
		return FALSE;
	}

	if(((fileHead.FileType&0x37) == CYCTYPE)&&(mode != REC_ACC_MODE_PREVIOUS)&&(action == A_UpdateFile))
	{
		if(apiMode == UICCAPI_Mode)
		{
#if UICC__FUNCTION__UICCAPI
		    //抛出异常 COMMAND_INCOMPATIBLE
	    	UICCThrowUICCException(COMMAND_INCOMPATIBLE);
#endif
		}
		else
		{
#if UICC__FUNCTION__SIMAPI
			ThrowSIMViewException(INVALID_MODE_SIM);
#endif
		}
		
		return FALSE;
	}

	if( mode == REC_ACC_MODE_CURRENT)
	{
		if(recNumber < 0x00)
		{
			UiccOrSimViewThrowException(RECORD_NOT_FOUND, apiMode);
			
			return FALSE;
		}

		if(recNumber > fileHead.RecNumber)
		{
			UiccOrSimViewThrowException(RECORD_NOT_FOUND, apiMode);
						
			return FALSE;

		}
	}

	if(Toolkit_GetKeyStatus(G_ToolkitCurrentEfAddr,action, apiMode) == FALSE)
	{
		UiccOrSimViewThrowException(SECURITY_STATUS_NOT_SATISFIED, apiMode);
		
		return FALSE;
	}
	
	return TRUE;
	
}
