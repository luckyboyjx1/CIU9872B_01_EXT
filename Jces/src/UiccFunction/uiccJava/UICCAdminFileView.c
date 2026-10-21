/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCAdminFileView.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2013-11-12

*Description :该文件主要用于实现基于TS102241 所描述的interface AdminFileView的功能 ，
*
*History     :
	
author    time      version    description
ZZJ   2013/11/12   V1.0       创建该文件
************************************************************************************************************************************************************************************************/
#include    "hard_api.h"
//#include    "resume.h"
#include    "fileAction.h"
#include    "fileSystem.h"
#include    "gsmpublic.h"
#include     "interface.h"
#include    "UICCToolkitAPI.h"
#include    "UICCToolkitConstants.h"
//#include    "transaction.h"
#include    "Interpreter.h"
#include  "uicc_interface.h"
//#include  "ChannelManagement.h"
#include  "STKFileView.h"
#include "ToolkitAPIPublic.h"
#include "platform_api.h"
#if UICC__FUNCTION__UICCAPI
/*
*@function    :GetCurentFileRoute
*
*@description :获取当前DF路径
*
*@param       :curentFileContext：用于保存chanel 0~3 当前DF路径
*
*@return      :void
*
*@author      :ZZJ
*
*@history     :V1.0:create by ZZJ@20131113
*/
void GetCurentFileRoute(WORD curentFileContext[4][3])
{
	BYTE idata i;

	for(i=0;i<4;i++)
	{
		if(getChannelState(i) == CHANNEL_CLOSED)   //i所指示的通道是关闭状态
		{
			continue;
		}
		
		if(G_CurrentDfAddr[i] == 0)
		{
			continue;
		}

		ReadEeprom((BYTE *)&curentFileContext[i][0],G_CurrentDfAddr[i] + F_ParentName_Offset,sizeof(WORD)*2);	   

		ecpMem_Copy((BYTE *)&curentFileContext[i][2], (BYTE*)&G_CurrentDfName[i],sizeof(WORD));
	}			
}
/*
*@function    :ResetCurentFileContext
*
*@description :恢复chanel 0~3 文件目录上下文
*
*@param       :curentFileContext：chanel 0~3 当前DF路径信息
*
*@return      :void
*
*@author      :ZZJ
*
*@history     :V1.0:create by ZZJ@20131113
*/
void ResetCurentFileContext(WORD curentFileContext[4][3])
{
	BYTE idata i,j,backSfi;
	UWORD xdata fileAddr;

	for(i=0;i<4;i++)
	{
		if(getChannelState(i) == CHANNEL_CLOSED)   //i所指示的通道是关闭状态
		{
			continue;
		}
		fileAddr = E_FILE_START;

		for(j=0;j<3;j++)
		{
			backSfi = G_CurrentSfi;
            
			fileAddr = SelectFile(fileAddr,curentFileContext[i][j]);
            
			G_CurrentSfi = backSfi;

			if(fileAddr == FALSE)
			{
				 SetChannel(fileAddr,curentFileContext[i][j-1],FALSE);						 //若找不到DF，则以其父目录为当前DF
			}
			else
			{
				G_CurrentDfAddr[i] = fileAddr;
			}
		}

		G_CurrentEfAddr[i] = SelectFile(G_CurrentDfAddr[i], G_CurrentEfName[i]);		 //DF存在，则找EF，若EF存在，则维持原状，否则SetChannel
		
		if(fileAddr == FALSE)
		{
			SetChannel(fileAddr,G_CurrentDfAddr[i],FALSE);
		}
	}
}


/*
*@function    :UICCToolkit_createFile
*
*@See         :uicc.framework.UICCAdminFileViewImpl.createFile(ViewHandler)
*
*@description :This method creates a new file under the current DF or ADF
*
*@param       :objref_t:FCP value
*
*@return      :void
*
* @exception  NullPointerException            if viewHandler is null
* @exception  UICCException                   in case of error
*             <li>MEMORY_PROBLEM
*             <li>INTERNAL_ERROR>
*             <li>SECURITY_STATUS_NOT_SATISFIED
*             <li>COMMAND_NOT_ALLOWED  if the referenced file is terminated
* @exception  AdminException                   in case of error
*             <li>FILE_ALREADY_EXISTS
*             <li>INCORRECT_PARAMETERS
*             <li>NOT_ENOUGH_MEMORY_SPACE
*             <li>DF_NAME_ALREADY_EXISTS
*
*@author      :ZZJ
*
*@history     :V1.0:create by ZZJ@20131113
*/
void UICCToolkit_createFile(void)
{
	BYTE  idata  result,fileType;
    WORD  idata  fileName;
	UWORD xdata  fileAddr;
	DWORD xdata  fcpData;
	
	jshort idata len = PopOperandStackShort();
    
	jref idata fcp = PopOperandStackShort();
	
	fcpData = GetArrayBufferAddress(fcp) ;  //这个是JAVA层传入，其取值为fcp = UICCPlatform.getTheVolatileByteArray();所以这里fcpData为RAM

	ecpMem_Copy(tempBuf, (BYTE*)fcpData, len);

	result = FindTag(tempBuf,0x83,len);
  
  	if(result == FALSE)
  	{
		UICCThrowAdminException(INCORRECT_PARAMETERS );
  	}
  	
  	fileName = ((tempBuf[result+1] << 8) & 0xFF00) + (tempBuf[result+2]&0x00FF) ;
  
  	fileAddr = SelectFile(G_ToolkitCurrentDfAddr,fileName) ;
        
  	if( fileAddr != FALSE )                                        						
  	{
		UICCThrowAdminException(FILE_ALREADY_EXISTS);
  	}

	result = FindTag(tempBuf,0x82,len);              //根据要创建文件的类型判断是否满足操作权限
  	
	if(result == FALSE)
	{
		UICCThrowAdminException(INCORRECT_PARAMETERS);
	}
    
    fileType =  tempBuf[result+1];

   	if((fileType&FILETYPE)== ADFTYPE)										//create df
  	{
  	   	result = Toolkit_GetKeyStatus(G_ToolkitCurrentDfAddr,A_CreateDF,UICCAPI_Mode);
  	}
  	else if(((fileType&FILETYPE)== 0x39)||((fileType&0x37) == TRANTYPE)||((fileType&0x37) == LINTYPE)||((fileType&0x37) == CYCTYPE))
  	{
  		result = Toolkit_GetKeyStatus(G_ToolkitCurrentDfAddr,A_CreateEF,UICCAPI_Mode);
  	}
  	else
  	{
  		UICCThrowAdminException(INCORRECT_PARAMETERS);
  	}

	if (result == FALSE)
	{
		UICCThrowUICCException(SECURITY_STATUS_NOT_SATISFIED);
	}
			
	ecpMem_Copy(tempBuf, (BYTE*)fcpData, len);

	fileAddr = createfile_sub(G_ToolkitCurrentDfAddr,tempBuf,len,fileName,fileType&FILETYPE);

	if(fileAddr == FALSE)
	{
		UICCThrowUICCException(MEMORY_PROBLEM); 
	}

	if((fileAddr == E_Parameter) || (fileAddr == E_NotSupport) || (fileAddr == E_InvalidRequest))//add by chentao 配合创建DF不支持88tag修改
	{
		UICCThrowAdminException(INCORRECT_PARAMETERS);
	}
	
	if(fileAddr == E_FileSize)
	{
		UICCThrowAdminException(NOT_ENOUGH_MEMORY_SPACE); 
	}
 
	if(fileAddr == E_DfNameExist)
	{
		UICCThrowAdminException(DF_NAME_ALREADY_EXISTS);
	}
	
	if(fileAddr == E_ConditionNotSat)
	{
		UICCThrowAdminException(CONDITIONS_OF_USE_NOT_SATISFIED);
	}

	IF_ProcessNoShareableFile(fileAddr,fileName,0x01,UICCAPI_Mode);

	Toolkit_SetChannel(fileAddr,fileName,FALSE,UICCAPI_Mode);

	return;    
}

/*
*@function    :UICCToolkit_deleteFile
*
*@See         :uicc.framework.UICCAdminFileViewImpl.deleteFile(short)
*
*@description :This method initiates the deletion of an EF immediately under the current DF
*
*@param       :fid: File ID
*
*@return      :void
*
*@throws      :UICCException - in case of error
*               FILE_NOT_FOUND 
*               MEMORY_PROBLEM 
*               INTERNAL_ERROR 
*               SECURITY_STATUS_NOT_SATISFIED 
*             javacard.framework.TransactionException - if the operation would cause the commit capacity to be exceeded
*
*@author      :ZZJ
*
*@history     :V1.0:create by ZZJ@20131113
*/
void UICCToolkit_deleteFile(void)
{	
	BYTE idata result;
	UWORD xdata fileAddr;
	WORD xdata curentFileContext[4][3];

	jshort idata fileName =  PopOperandStackShort();
	
	if((fileName == DF_MF)||(G_ToolkitCurrentDfName == fileName))
	{
		UICCThrowUICCException(FILE_NOT_FOUND);								
	}

	fileAddr = SearchFile(G_ToolkitCurrentDfAddr,fileName);							//addr of file to delete 

	if(fileAddr == FALSE)
	{
		UICCThrowUICCException(FILE_NOT_FOUND);
	}
	result = GetFileHeadByte(fileAddr,F_Type_Offset);

	if(result != DFTYPE)
	{
		if(IF_ProcessNoShareableFile(fileAddr,fileName, 0x03, UICCAPI_Mode) == FALSE)  ////Api_1_Contòa?óμ±???Dμ????t?a·?12?í???tê±￡??ò?×3?òì3￡ FILE_NOT_FOUND//??3yμ±?°í¨μàμ?·?12?íef ?·??D??￠￡???fileViewéè??D?μ?EF DF???t???°￡?ê×?è?D??D?éè??μ?EFê?·??a·?12?í???t￡??òê?·?ò?±?????í¨μà?ò???ó?ù·??ê
		{
			UICCThrowUICCException(FILE_NOT_FOUND);
		}
		Toolkit_SetChannel(fileAddr,fileName,FALSE,UICCAPI_Mode);      //éè??EFμ±?°EF		
	}
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	//强更新文件不允许delete
	if(IF_IsHeFlashFile(fileAddr))
	{
		UICCThrowAdminException(CONDITIONS_OF_USE_NOT_SATISFIED); 
	}
#endif
	if(Toolkit_GetKeyStatus(fileAddr,A_DeleteFile,UICCAPI_Mode) == FALSE)
	{
		UICCThrowUICCException(SECURITY_STATUS_NOT_SATISFIED);
	}

	GetCurentFileRoute(curentFileContext);						

	result = DeleteFile_sub(fileAddr,G_ToolkitCurrentDfAddr);
	
	if(result == FALSE)
	{
		UICCThrowUICCException(MEMORY_PROBLEM );
	}

	if(result == E_ConditionNotSat)
	{
		UICCThrowAdminException(CONDITIONS_OF_USE_NOT_SATISFIED);
	}

	Toolkit_SetChannel(G_ToolkitCurrentDfAddr,G_ToolkitCurrentDfName,FALSE,UICCAPI_Mode);

	IF_ProcessNoShareableFileListAfterDeleteFile();

  	ResetCurentFileContext(curentFileContext);

	return; 			
}
/**
*@function      : UICCToolkit_resizeFile
*
*@See           : uicc.framework.UICCAdminFileViewImpl.resizeFile(ViewHandler)
*
*@description   :This method resizes a file under the current DF or ADF, as described in TS 102 222. If the current file of the AdminFileView is a BER-TLV structured EF and Tag '86' (Maximum File Size) is present within the proprietary TLV ( tag 'A5') in the FCP data object then this method operates in Mode 0 as defined in TS 102 222.
*
*@param         : fcp 
*@param         :length
                 Note:fcp,length的数据来源于ViewHandler
*                ViewHandler object containing the value part of the FCP data object for the  RESIZE command, as described in TS 102 222.
*
*@exception  NullPointerException            if viewHandler is null
*@exception  UICCException                   in case of error
*            MEMORY_PROBLEM
*            COMMAND_INCOMPATIBLE
*            SECURITY_STATUS_NOT_SATISFIED
*            REF_DATA_INVALIDATED
*            FILE_NOT_FOUND
*            INTERNAL_ERROR
*            COMMAND_NOT_ALLOWED  if the referenced file is terminated
*@exception  AdminException                   in case of error<ul>
*            CONDITIONS_OF_USE_NOT_SATISFIED
*            NOT_ENOUGH_MEMORY_SPACE
*            INCORRECT_PARAMETERS
*           
*@exception  :TransactionException if the operation would cause the commit capacity to be exceeded.
*
*@author     :xuyz
*
*@history    :V1.0 :create by xuyz@20131115
*/

void UICCToolkit_resizeFile(void)
{
	DWORD xdata fcpData;
	UWORD xdata fileAddr;
	BYTE idata tmp,tag1,tag2;
	WORD idata fileId,parId1;
	BYTE idata backSfi,fileStatus;
	WORD idata statusSW1SW2;

	UWORD xdata actualFileAddr;
	//UWORD xdata backCurrentEfAddr,backCurrentDfAddr;
	//WORD idata  backEfName, backDfName;
	WORD  xdata filename[PROFILEDELETE_BACKUPFILESYSTEM];  //?3??DF name  4?? EF name
	
	jshort idata length = PopOperandStackShort();
    
	objref_t idata fcp  = PopOperandStackShort();

	fcpData = GetArrayBufferAddress(fcp) ;  //这个是JAVA层传入，其取值为fcp = UICCPlatform.getTheVolatileByteArray();所以这里fcpData为RAM

	ecpMem_Copy(tempBuf, (BYTE*)fcpData, length);

    tmp = FindTag(&tempBuf[0], 0x83, length);

	if(tmp == FALSE)
	{
		UICCThrowAdminException(INCORRECT_PARAMETERS);
	}

	fileId = (((WORD)tempBuf[tmp+1]<<8)&0xFF00) + tempBuf[tmp+2];

	if(fileId == 0x3F00)		//暂时不考虑MF的空间变更
	{
		UICCThrowAdminException(INCORRECT_PARAMETERS);
	}

	tag1 = FindTag((BYTE *)&tempBuf[0],  0x80,  length);					//变更EF空间

	tag2= FindTag((BYTE *)&tempBuf[0], 0x81, length);				        //变更DF空间

	if((tag1 == FALSE)&&(tag2 != FALSE))  //变更DF空间
	{	
		parId1 = G_ToolkitCurrentDfName;	

	}
	else	                          //变更透明文件，线形文件或BERTLV Reserved Size
	{
		parId1 = G_ToolkitCurrentEfName;
	}

	if(fileId != parId1)
	{
		backSfi = G_CurrentSfi;
		fileAddr = SelectFile(G_ToolkitCurrentDfAddr, fileId);
		G_CurrentSfi = 	backSfi;
	
		if(!fileAddr)
		{
			UICCThrowUICCException(FILE_NOT_FOUND);
		}
		else
		{	
			if(IF_ProcessNoShareableFile(fileAddr,fileId, 0x03,UICCAPI_Mode) == FALSE)  //该文件是非共享文件,且已被其他通道占用	。//Api_1_Cont要求当选中的文件为非共享文件时，则抛出异常 FILE_NOT_FOUND
			{
				UICCThrowUICCException(FILE_NOT_FOUND);
			}	
			Toolkit_SetChannel(fileAddr, fileId, FALSE,UICCAPI_Mode);			
		}  
	}

	if(tag1 != FALSE)  //当前变更的是EF的空间
	{
		actualFileAddr = SearchFile(G_ToolkitCurrentDfAddr, G_ToolkitCurrentEfName);	//获取文件的实际地址，防止文件时映射文件，而修改到了映射文件的状态
		
		fileStatus = GetFileHeadByte(actualFileAddr,F_Status_Offset);

		if((fileStatus&0x01) == 0x00)  // 当前文件失效
		{
			UICCThrowUICCException(REF_DATA_INVALIDATED);
		}

		tmp = Toolkit_GetKeyStatus(G_ToolkitCurrentEfAddr, A_ResizeFile,UICCAPI_Mode);  //因为publicResizeFile使用了变量tempBuf，若在publicResizeFile统一验证权限，则有可能导致tempBuf中的数据被冲掉，所以将resize权限的验证提前放到这里	
	}
	else  //变更DF空间
	{
		tmp = Toolkit_GetKeyStatus(G_ToolkitCurrentDfAddr, A_ResizeFile,UICCAPI_Mode);
	}
	if(tmp == FALSE)
	{
		UICCThrowUICCException(SECURITY_STATUS_NOT_SATISFIED);
	}
    //todo :先备份当前UICC APDU级的currentEF currentDF 等信息
	//backCurrentEfAddr = G_CurrentEfAddr[G_ChannelNum];
	//backEfName        = G_CurrentEfName[G_ChannelNum];
	
	//backCurrentDfAddr = G_CurrentDfAddr[G_ChannelNum];
	//backDfName        = G_CurrentDfName[G_ChannelNum];
	ecpMem_Set(filename,0x00,sizeof(filename));
	tmp = BackDFNameForMoveFalsh(filename);

	G_CurrentEfAddr[G_ChannelNum] = G_ToolkitCurrentEfAddr;
	G_CurrentEfName[G_ChannelNum] = G_ToolkitCurrentEfName;
	G_CurrentDfAddr[G_ChannelNum] = G_ToolkitCurrentDfAddr;
	G_CurrentDfName[G_ChannelNum] = G_ToolkitCurrentDfName;
	
	ecpMem_Copy(tempBuf, (BYTE*)fcpData, length);
 
	statusSW1SW2 = publicResizeFile(&tempBuf[0],length,0x01,fcpData); 

	//G_CurrentEfAddr[G_ChannelNum] = backCurrentEfAddr;
	//G_CurrentEfName[G_ChannelNum] = backEfName;
	
	//G_CurrentDfAddr[G_ChannelNum] = backCurrentDfAddr;
	//G_CurrentDfName[G_ChannelNum] = backDfName;
	if(tmp == TRUE)
	{
		ReloadDFOrEFAddr(filename);
	}
	
	//todo :检查由于resize文件会不会导致当前某个通道的current信息发生变化

	if(statusSW1SW2 == 0x6A80)
	{
		UICCThrowUICCException(INCORRECT_PARAMETERS);
	}
	else if(statusSW1SW2 == 0x6A82)
	{
		UICCThrowUICCException(FILE_NOT_FOUND);
	}
	else if(statusSW1SW2 == 0x6982)
	{
		UICCThrowUICCException(SECURITY_STATUS_NOT_SATISFIED);
	}
	else if(statusSW1SW2 == 0x6981)
	{
		UICCThrowUICCException(COMMAND_INCOMPATIBLE);
	}
	else if(statusSW1SW2 == 0x6581)
	{
		UICCThrowUICCException(MEMORY_PROBLEM);
	}
	else if(statusSW1SW2 == 0x6A84)
	{
		UICCThrowAdminException(NOT_ENOUGH_MEMORY_SPACE);
	}
	else if(statusSW1SW2 == 0x6985)
	{
		UICCThrowAdminException(CONDITIONS_OF_USE_NOT_SATISFIED);
	}

   	if(tag1 != FALSE)  	 //当前变更的是EF的空间
	{
		NoShareableFileIsValidAndSetBit(G_ToolkitCurrentEfAddr,fileId,0x01,UICCAPI_Mode);	//若当前文件为非共享文件，则设置其信息
	}
}	
#endif

	
