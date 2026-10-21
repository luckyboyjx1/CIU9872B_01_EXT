/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.0

 * MODEL: 	All model

 * RELEASE: 2008-12-18

 * AUTHER:	徐庆盛 徐云珍

 * HISTORY: V1.0 创建源文件
            V1.1 修改函数DeleteFile与ResizeFile,宏DF_ADF使用全局变量G_DF_ID_USIM替代,增加代码兼容性   by yangjian 2022.12.6

 	
		
 */ 
#include "uicc_main.h"
#include "resume.h"
#include "uicc_error.h"
#include "fileSystem.h"
#include "gsmpublic.h"
#include "fileAction.h"
#include "Interface_LTEFromBoot.h"
#include "stkhead.h"
#include "eepromDefine.h"
#include "interface.h"
#include <string.h>
#include <stdlib.h>
//#include "Transaction.h"
//#ifdef DELRESIZEFILE  

/*-------------------------------------------------------------------------------------------------------------------                                                                                       
* @Function     	    :BackDFNameForMoveFalsh
*  
* @Discription        : 备份当前DF name和父DF应用场景，比如RFM执行Resize等移动Flash的操作，可能会改变DF Address，从RFM等出来后需要重新选地址
*
* @See                :
*                                                                                                                                                
* @Parameter          :void                                                                  					                                                                                                             
*
* @Return             :void
*                                                                                                                                                        
* @History		        :V1.0 Creat by LCZ 20180726 
*
 -------------------------------------------------------------------------------------------------------------------*/  
void BackDFNameForMoveFalsh(WORD DFFileName[4])
{
	ReadEeprom((BYTE *)&DFFileName[0],G_CurrentDfAddr[G_ChannelNum]+ F_ParentName_Offset,sizeof(WORD)*2); 
		
	DFFileName[2] = G_CurrentDfName[G_ChannelNum];
	
	DFFileName[3] = G_CurrentEfName[G_ChannelNum];
	
	return;
}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       
* @Function     	    :ReloadDFOrEFAddr
*  
* @Discription        : 重新SetChannel DF EF地址，场景：RFM或者API中执行Resize等会改变DF或者EF地址，配合BackDFNameForMoveFalsh函数使用
*
* @See                :
*                                                                                                                                                
* @Parameter          :void                                                                  					                                                                                                             
*
* @Return             :void
*                                                                                                                                                        
* @History		        :V1.0 Creat by LCZ 20180726 
*
 -------------------------------------------------------------------------------------------------------------------*/  
void ReloadDFOrEFAddr(WORD fileName[4])
{
	UWORD xdata tmpAddr;
	
	tmpAddr = SearchFile(E_FILE_START,fileName[0]);	
	
	tmpAddr = SearchFile(tmpAddr,fileName[1]);					  	
				
	tmpAddr = SearchFile(tmpAddr,fileName[2]);				
	
	SetChannel(tmpAddr,fileName[2],FALSE);  //重新设置DF地址
	
	if(fileName[3] != 0x00)           //复位没有其它操作进行RFM操作时候，EF和EfAddr是False
	{
		tmpAddr = SelectFile(tmpAddr,fileName[3]);	

		SetChannel(tmpAddr,fileName[3],FALSE);  //重新设置EF的地址
	}
	
	return;
} 

#if !HEAD_BODY_TOGETHER		
  
extern void DeleteEFContent(UWORD  EFAddr);
extern BYTE DeleteDFContent(UWORD  DFAddr);


void ClearIndex(WORD  fileContentRef)
{
  if(fileContentRef != 0)
	{
		SetEepromEX(E_ADDR_INDEX_TABLE + (fileContentRef-1)*sizeof(UWORD), 0xFF, sizeof(UWORD));	
	}
}

BYTE ClearAddrIndex(UWORD  DFAddr)
{
	UWORD xdata fileUseSpace,fileAddr,fileSpaceAddr;
	
	WORD idata fileContentRef;
	
	BYTE  idata fileType;
	
	fileUseSpace = GetFileHeadUsedSize(DFAddr);
	
    fileUseSpace = fileUseSpace + DFAddr;

   	for(fileAddr = DFAddr+sizeof(FileSystemHead);fileAddr < fileUseSpace;fileAddr = fileAddr + fileSpaceAddr)//
   	{	
		fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

		if(fileType == ADFTYPE)
		{			
			fileContentRef = GetFileHeadWord(fileAddr,F_DFContent_Offset);

			fileSpaceAddr = GetFileHeadUsedSize(fileAddr);	
			
			if(fileContentRef != 0)
			{
				SetEepromEX(E_ADDR_INDEX_TABLE + (fileContentRef-1)*sizeof(UWORD), 0xFF, sizeof(UWORD));	
			}
			
			ClearAddrIndex(fileAddr);	
		}
		else if((fileType == TRANTYPE) || (fileType == LINTYPE)||(fileType == CYCTYPE))
		{					
            fileContentRef = GetFileHeadWord(fileAddr,F_Content_Offset);

			fileSpaceAddr = sizeof(EFileHead);

            if(fileContentRef != 0)
			{
				SetEepromEX(E_ADDR_INDEX_TABLE + (fileContentRef-1)*sizeof(UWORD), 0xFF, sizeof(UWORD));	
			}								

		}
		else
		{
			return FALSE;
		}
   	}

	return TRUE;
	
}



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 DeleteFile
  
  Command format:    00    E4    00   00   0x02
  
  Data    content:   File ID                                                       
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description: 在当前的ADF/DF下，删除DF/ADF/EF
  
  status:			     

 -------------------------------------------------------------------------------------------------------------------*/

void DeleteFile(void)
{
	SingleFileHead xdata dfHeader;
	BYTE idata result,fileType,dirNum;
	WORD  idata fileName;
	UWORD idata fileAddr,moveBodyLen,fileContentRef;
	UWORD idata dataSize,fileContentSize,mfUsedSize,fileContentAddr,curMFBodyEnd;
	BYTE idata need_commit;
	
	need_commit = FALSE;
	
	dirNum = 0;
	
	if(P1P2P3Check(0x02,FALSE) == FALSE)
	{
		return ;
	}

	/*
	if((G_CurrentDfName[G_ChannelNum] == DF_MF) && (G_CurrentEfName[G_ChannelNum] == EF_ICCID))  			//对于标准的敏感数据文件,不允许进行DeleteFile操作
	{
	    SecuritySatisfied();
		return;
	}
	else if((G_CurrentDfName[G_ChannelNum] == DF_PHASE) && (G_CurrentEfName[G_ChannelNum] == EF_IMSI))
	{
	    SecuritySatisfied();
		return;
	}
	else if((G_CurrentDfName[G_ChannelNum] == DF_ADF) && (G_CurrentEfName[G_ChannelNum] == EF_IMSI))
	{
		SecuritySatisfied();
		return;     
	}
	*/
	
	ReceiveData();

	fileName = ((RecData[0]<<8)&0xFF00)+RecData[1];
	
	if((fileName == DF_MF)||(G_CurrentDfName[G_ChannelNum] == fileName))
	{
		TechnicalProblem();
		return;									
	}

	fileAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum],fileName);							//addr of file to delete 

	if(fileAddr == FALSE)
	{
		FileNotFound();
		return;			
	}
	result = GetFileHeadByte(fileAddr,F_Type_Offset);

	if(result == DFTYPE)
	{
		result = GetFileHeadByte(fileAddr,F_DIRNum_offset);

		if(result != 0x00)
		{
			if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)	//not perform delete ADF for RFM
			{
				TechnicalProblem();
				return;
			}
		}		
	}
	else
	{
		SetChannel(fileAddr,fileName,FALSE);      //设置EF当前EF
	}
	
	//SetChannel后再进行判断	
	if((G_CurrentDfName[G_ChannelNum] == DF_MF) && (G_CurrentEfName[G_ChannelNum] == EF_ICCID))  			//对于标准的敏感数据文件,不允许进行DeleteFile操作
	{
	    SecuritySatisfied();
		return;
	}
	else if((G_CurrentDfName[G_ChannelNum] == DF_PHASE) && (G_CurrentEfName[G_ChannelNum] == EF_IMSI))
	{
	    SecuritySatisfied();
		return;
	}
	else if((G_CurrentDfName[G_ChannelNum] == G_DF_ID_USIM) && (G_CurrentEfName[G_ChannelNum] == EF_IMSI))
	{
		SecuritySatisfied();
		return;     
	}
	
	
	result = AccessCondition(fileAddr,A_DeleteFile);						
	   
	if(result == FALSE)
	{
		SecuritySatisfied();
		return;
	}
	
	need_commit = IF_BeginTransaction();
	
	ReadEeprom((BYTE *)&dfHeader.UsedContentSize,G_CurrentDfAddr[G_ChannelNum]+F_UsedContentSize_Offset,sizeof(SingleFileHead));
	
	mfUsedSize = GetFileHeadUsedSize(E_FILE_START);	
	
	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

	fileContentSize = GetFileContentSize(fileAddr);	  //获取文件体总大小
	
	curMFBodyEnd = E_FileBodyStart - GetFileContentSize(E_FILE_START);
	
	if(fileType == DFTYPE)
	{					
		dirNum = GetFileHeadByte(fileAddr,F_DIRNum_offset);	   //如果删除ADF，需要删除2F00中的特定记录

		fileContentRef = GetFileHeadWord(fileAddr,F_DFContent_Offset);
		
		fileContentAddr = VMReadUword(E_ADDR_INDEX_TABLE + (fileContentRef-1)*sizeof(UWORD)); 

		dataSize = GetFileHeadUsedSize(fileAddr);

        DeleteDIRNum(dirNum);		

        modifyGrandParentSize2(-dataSize,-fileContentSize,(D_DeleteFile|DF));

		moveBodyLen = fileContentAddr - fileContentSize - curMFBodyEnd;	

        ClearAddrIndex(fileAddr);		

	}
	else																								
	{				
		
		fileContentRef = GetFileHeadWord(fileAddr,F_Content_Offset);
		
		fileContentAddr = VMReadUword(E_ADDR_INDEX_TABLE + (fileContentRef-1)*sizeof(UWORD)); 

		dataSize = sizeof(EFileHead);
		
		modifyGrandParentSize2(-dataSize,-fileContentSize,(D_DeleteFile|EF));

        moveBodyLen = fileContentAddr - curMFBodyEnd;		
	}

    ClearIndex(fileContentRef);		
							
    if((E_FILE_START + mfUsedSize) > (fileAddr + dataSize))  //need not move
	{
		MoveHugeEepromEXPrepare(fileAddr,fileAddr + dataSize, (E_FILE_START + mfUsedSize)-(fileAddr + dataSize), 0);															
	}	

	if(moveBodyLen > 0) //移动文件体
	{
		MoveHugeEepromEXPrepare(curMFBodyEnd + fileContentSize,curMFBodyEnd, moveBodyLen, 1);

        SyncAddrIndexTable(fileContentAddr,fileContentSize,D_DeleteFile);		
	}

	IF_CommitTransaction(need_commit);	
		
	MoveHugeEepromRunning();
	
	SetChannel(G_CurrentDfAddr[G_ChannelNum],G_CurrentDfName[G_ChannelNum],FALSE);

	CommandFinish(0x00);                                  							
}







/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ResizeFile
  
  Command format:    00    D4    00   00   0xXX
  
  Data    content:   FCP                                                        
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       在当前的APF/DF下，改变DF/ADF/EF的大小
  
  status:			VER 2007-07-17     

 -------------------------------------------------------------------------------------------------------------------*/  

void ResizeFile(void)
 {
 	BYTE  * ptrToValue;
 	BYTE    idata tag1,tag2;
	BYTE 	idata pos, tmp, fileStatus, result, i;
	BYTE 	idata recNum[sizeof(UWORD)];
	WORD 	idata parId1,fileId;
	UWORD	xdata oldfileSize,newFileSize;
	UWORD  xdata dataLen;
	UWORD  xdata  currentEfHeadAddr,flashUsedAddr,fileContentAddr,destAddr,modifyEfParAddr,tmpAddr,modifyEfAddr,srcAddr,fileAddr;
	BYTE  idata resizeMapFlag;
	WORD  idata fileName[4];
	EFileHead xdata tmpEFFileHead;
	BYTE idata need_commit;
	BYTE idata index;
	 
	 
	need_commit = FALSE;

	if(P1P2Check() ==FALSE)
	{
		return;
	}

	
	ReceiveData();

	flashUsedAddr =  (E_FileBodyStart - GetFileContentSize(E_FILE_START));// GetFileBodyEndAddr();

	
	memset((BYTE *)&recNum[0], 0x00, sizeof(recNum));

	fileStatus = 0x00;                   //初始化状态字节  

	pos = FindTag((BYTE *)&RecData[0], 0x62,  P3);

	if(pos == FALSE)
	{
		IncorrentParameter();														 //Send 6A 80
		return;
	}

	if((RecData[pos]+2) != P3)
	{
		IncorrentParameter();														 //Send 6A 80
		return;
	}
	
	pos++;	

	ptrToValue = &RecData[pos];

	pos = P3 - pos;
	
	tmp = FindTag((BYTE *)&ptrToValue[0], 0x83, pos);

	if(tmp == FALSE)
	{
		IncorrentParameter();														 //Send 6A 80
		return;
	}


	fileId = (((WORD)ptrToValue[tmp+1]<<8)&0xFF00) + ptrToValue[tmp+2];

	if(fileId == 0x3F00)		//暂时不考虑MF的空间变更
	{
		IncorrentParameter();													//Send 6A 80
		return;
	}


	tag1 = FindTag((BYTE *)&ptrToValue[0],  0x80,  pos);					//变更EF空间

	tag2= FindTag((BYTE *)&ptrToValue[0], 0x81, pos);				        //变更DF空间

	if((tag1 == FALSE)&&(tag2 != FALSE))  //变更DF空间，由于是动态文件系统，不需要考虑
	{
	    if(SelectFile(G_CurrentDfAddr[G_ChannelNum], fileId) == FALSE)
		{
		 	FileNotFound();								//Send 6A 82
			return;
		}
		else
		{     
			  ConditionSatisfied();					//ResizeFile规范定义：动态文件系统,DF resize if not allowed, the error status SW1/SW2="6985'
			  //CommandFinish(0x00);
		     return;
		}	  
	}
	else	                          //变更透明文件，线形文件或BERTLV Reserved Size
	{
	  	parId1 = G_CurrentEfName[G_ChannelNum];

		tmp = tag1;

		//result = sizeof(WORD);
	}

	if(fileId != parId1)
	{
		tmpAddr = SelectFile(G_CurrentDfAddr[G_ChannelNum], fileId);	

		if(tmpAddr == FALSE)
		{
			FileNotFound();								//Send 6A 82
			return;
		}
		else
		{
			SetChannel(tmpAddr, fileId, FALSE);
		}  
	}
	
	//SetChannel后再进行判断
	if((G_CurrentDfName[G_ChannelNum] == DF_MF) && (G_CurrentEfName[G_ChannelNum] == EF_ICCID))  			//对于标准的敏感数据文件,不允许进行ResizeFile操作
	{
	    SecuritySatisfied();
		return;
	}
	else if((G_CurrentDfName[G_ChannelNum] == DF_PHASE) && (G_CurrentEfName[G_ChannelNum] == EF_IMSI))
	{
	    SecuritySatisfied();
		return;
	}
	else if((G_CurrentDfName[G_ChannelNum] == G_DF_ID_USIM) && (G_CurrentEfName[G_ChannelNum] == EF_IMSI))
	{
		SecuritySatisfied();
		return;     
	}
	//获取文件FileSize，兼容大小端的情况,by xwh 20220519
	if(ptrToValue[tmp] == 1)
	{
		newFileSize = ptrToValue[tmp+1];
	}
	else if(ptrToValue[tmp] == 2)
	{
		newFileSize = BuildWord(&ptrToValue[tmp+1]);
	}
	else//80tag的长度不能超过2
	{
		ReferencedDataInvalid();					//Send 6A 80
		return;
	}

//	result = sizeof(UWORD) - 1;					//对于DF空间的变更，只支持最大4字节的空间表示方法

//	for(i=ptrToValue[tmp]; i>0; i--)			//由于UWORD的字长定义决定最大的DF的空间
//	{
//		recNum[result] = ptrToValue[tmp+i];

//		result--;
//	}


//#ifdef LIB_HAL_LittleEndian     //变更线性文件，透明文件或者bertlv文件
//	if(sizeof(UWORD) == 0x04)
//	{
//		newFileSize = ((((unsigned long)(recNum[sizeof(UWORD) - 4]))<<24)&0xFF000000)+((((unsigned long)(recNum[sizeof(UWORD) - 3]))<<16)&0x00FF0000)+((recNum[sizeof(UWORD) - 2]<<8)&0x0000FF00)+recNum[sizeof(UWORD) - 1];
//	}
//	else if(sizeof(UWORD) == 0x02)
//	{
//	   newFileSize = ((recNum[sizeof(UWORD) - 2]<<8)&0xFF00)+recNum[sizeof(UWORD) - 1];
//	}
//#else
//	memcpy((BYTE *)&newFileSize, &recNum[0], sizeof(recNum));	  //对于DF，记录文件和二进制文件，新空间的内容一律按照文件的实际空间表示，对于ber-tlv文件，该大小为文件的保留大小
//#endif

	
	currentEfHeadAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);  //获取文件的文件头地址

	fileStatus = GetFileHeadByte(currentEfHeadAddr,F_Status_Offset);  //获取文件的状态
	
	resizeMapFlag = 0;  //判断文件是否为映射文件

	if((fileStatus & F_MAP_FILE) == F_MAP_FILE)	//如果是映射文件
	{
		ReadEeprom((BYTE *)&fileName[0],currentEfHeadAddr + F_MapFile_Offset,sizeof(WORD)*3); 

		tmpAddr = SearchFile(E_FILE_START,fileName[0]);
		
		if(tmpAddr != FALSE)
		{
			tmpAddr = SearchFile(tmpAddr,fileName[1]);					  	
		}
				
		modifyEfParAddr = tmpAddr;  //保存当前待resize的DF地址
		
		if(tmpAddr != FALSE)
		{
			tmpAddr = SearchFile(tmpAddr,fileName[2]);				
		}       
		
		if(tmpAddr == FALSE)
		{
			return ;
		}
		
		modifyEfAddr = tmpAddr;	 //保存当前待resize的EF地址
		
		fileAddr = tmpAddr;
		
		resizeMapFlag = TRUE;
		
		BackDFNameForMoveFalsh(fileName);

	    G_CurrentDfAddr[G_ChannelNum] = modifyEfParAddr; //modifyGrandParentSize2中会使用
		
	}
	else
	{
		fileAddr = G_CurrentEfAddr[G_ChannelNum];
		
		modifyEfAddr = G_CurrentEfAddr[G_ChannelNum];
		
		modifyEfParAddr = G_CurrentDfAddr[G_ChannelNum];
	}



	fileStatus = GetFileHeadByte(currentEfHeadAddr,F_Type_Offset);   //获取文件的类型

	if(fileStatus == LINTYPE) //线性文件
	{
		ReadEeprom((BYTE  *)&recNum[0],modifyEfAddr + F_RecNum_Offset, sizeof(recNum));

		oldfileSize = recNum[0] * recNum[1];		//记录文件的空间

		if((newFileSize % recNum[1]) != 0x00)	 //新的记录大小不是记录长度的整倍数
		{
			IncorrentParameter();
		    return;
		}
		if((newFileSize / recNum[1]) >254 )	 //记录个数最大254
		{
			IncorrentParameter();
		    return; 
		}
	}
	else if(fileStatus == TRANTYPE)
	{
		oldfileSize = GetTFSize(modifyEfAddr);
	}
	else //This command allows to modify the memory space allocated to the MF, a DF/ADF, a transparent file or a linear fixed  file or a BER-TLV structured EF under the current directory (MF, DF/ADF). This command shall not be allowed for a cyclic file.
	{
	     FileStructureError();
		 return;
	}
	
	result = AccessCondition(fileAddr, A_ResizeFile);

	if(result == FALSE)      //安全状态不满足
	{
		SecuritySatisfied();
		return;
	}

	if(newFileSize == oldfileSize)  //空间没有变化
	{
		CommandFinish(0x00);
		return;
	}
	
	need_commit = IF_BeginTransaction();

	index = GetFileHeadByte(fileAddr, F_Pointer_Offset); //判断是否是强更新文件
	
	if(index!=0x00)															//为强更新文件，不支持Resize操作
	{
			SecuritySatisfied();
		  return;
	}
	
	if(oldfileSize > newFileSize)						//减小空间
	{																								
		if(fileStatus == LINTYPE)
		{
			if(newFileSize < recNum[1])		//至少有一条记录空间，记录的空间不能缩减至0
			{
				 IncorrentParameter();
		         return;
			}		
		}
  		
	}
	else //if(oldfileSize < newFileSize)		//增大空间
	{
		if((newFileSize-oldfileSize)> GetNVMAvailMemory())        //增大空间是否满足可用的空间
		{
				NoEnoughSpace();
			  return;
		}

	}


	
	
	ReadEeprom((BYTE *)&tmpEFFileHead, fileAddr,sizeof(EFileHead));

	if(fileStatus == LINTYPE)
	{
		recNum[0] = newFileSize / recNum[1];		//计算新的记录条数
		
		tmpEFFileHead.efSize.recInfo[0] = recNum[0];
	}
	else
	{		
		parId1 = newFileSize;
			
		tmpEFFileHead.efSize.SpaceSize = parId1;			
	}


    srcAddr	= flashUsedAddr;

	destAddr  =  flashUsedAddr - (newFileSize - oldfileSize);

	fileContentAddr =  getFileContentAddr(fileAddr);//文件内容的起始地址


	if(oldfileSize > newFileSize) //空间减小
	{
		dataLen = fileContentAddr + newFileSize - flashUsedAddr;
	}
	else //空间增大
	{
		dataLen = fileContentAddr + oldfileSize - flashUsedAddr;
	}
	
	MoveHugeEepromEXPrepare(destAddr,srcAddr,dataLen,0);			//处理数据体的安全移动
 
  SyncAddrIndexTable(fileContentAddr,oldfileSize - newFileSize,D_Resize); // 需要放在E_FlashUsedAddr更新之后

	if(fileStatus == LINTYPE)   //修改EF的文件头信息
	{
		WriteEepromEX(fileAddr+F_RecNum_Offset, (BYTE  *)&recNum[0], sizeof(BYTE));
	}
	else
	{		
		WriteEepromEX(fileAddr+F_SpaceSize_Offset, (BYTE  *)&parId1, sizeof(WORD));	
	}

	
	i = modifyGrandParentSize2(0,newFileSize - oldfileSize, D_Resize);
	
	if(i == DF_Depth)  //超过最大层级，报错
	{
		IF_AbortTransaction(need_commit);
		
		IncorrentParameter();
		
		return;
	}	

    IF_CommitTransaction(need_commit);
	
	MoveHugeEepromRunning();



  /*if(mode == 0x01)
	{
		memcpy(ptrToValue, (BYTE*)fcpData, pos);//对于通过adminfileview方式的resize
	}*/			

	if(oldfileSize < newFileSize)     //增加EF空间
	{
		tmp = FindTag((BYTE xdata *)&ptrToValue[0],0xA5,pos);			//处理文件的初始值字段，不带断电保护

		if(tmp == FALSE)
		{
			tmp = FindTag((BYTE xdata *)&ptrToValue[0],0x85,pos);
		}

		if(tmp != FALSE)
	 	{
			if(fileStatus == LINTYPE)
			{
				InitRecordFile(fileContentAddr+oldfileSize-newFileSize+oldfileSize,&ptrToValue[tmp],(newFileSize-oldfileSize)/recNum[1],recNum[1]);
			}
			else  if(fileStatus == TRANTYPE)
			{
				InitTransFile(fileContentAddr+oldfileSize-newFileSize+oldfileSize,&ptrToValue[tmp],newFileSize-oldfileSize,0x00); //这里传进去的inex为0x00  后面IF_InitFileResize做轮询模式的初始化
			}
			else
			{
			}
		}
	}

	
	if(resizeMapFlag == TRUE)
	{		
		ReloadDFOrEFAddr(fileName);
	}	
	
	CommandFinish(0x00); 
}
#endif


#if  NATIVE_CARD_UICC		
#ifdef SECURE_CREATE
/*
 *@description:创建EF文件
 *
 *@param:      fileMaxSize :the parent file Max  size
 *@param:      fileUseSize :the parent file Used size
 *@param       fileName    :current EF id
 *
 *@Returns     the need file size of creating A EF file/E_Parameter - error return
 *
 *@history     create by xyz at 090721 16:29
*/

UWORD CreateEFFileSECURE(UWORD fileUseSize,WORD fileName)
{
	BYTE idata pos,result,i,updatetag;
	BYTE idata fileType,shortFileId,arrNum,arrName,fileStatus;
	BYTE  * ptrToValue;
	UWORD  idata fileSize;
	UWORD xdata fileNeedSize,allocateAddr,mfUsedSize,datalen,updateAddr,bodyMovelen,fileBodyEnd;
	UWORD xdata mapFileAddr;
	EFileHead xdata efFileHead;
	BYTE idata need_commit;
	BYTE xdata   HighUpdateCfgInfo[High_Update_ConfigForFID];

	BYTE idata 	 UpdateConfigIndex;			
	
	need_commit = FALSE;
	
	allocateAddr = FALSE;
	
	bodyMovelen = 0;
	
	datalen = 0;

	pos = 1 + FindTag(&RecData[0],0x62,P3);

	ptrToValue = &RecData[pos];

	pos = P3 - pos;
	
	result = FindTag(&ptrToValue[0],0x82,pos);							//File Type	
	
	fileType = ptrToValue[result+1];

	result = FindTag(&ptrToValue[0],0x8A,pos);                          //fileStatus

	if(result == FALSE)
	{
		return E_Parameter;		
	}

	memset((BYTE *)&efFileHead, 0x00, sizeof(efFileHead));
	
	fileStatus = ptrToValue[result+1]&0x0F;

	result = FindTag(&ptrToValue[0],0x8D,pos);							//Expand for read and updata enable when invalid

	if(result != FALSE)
	{
		if((ptrToValue[result+1]&0x04) == 0x04)
		{
			fileStatus |= F_READ_UPDATA_ENBALE;	
		}		
	}
	
	result = FindTag(&ptrToValue[0],0x8B,pos);									//Referenced mode

	if((result != FALSE)&&(ptrToValue[result]==0x03))
	{
		arrNum = ptrToValue[result+3];

		arrName = ptrToValue[result+1];
	}
	else 
	{
		arrNum = FindArrNum(&ptrToValue[0],pos);	//该函数内部会使用到tempBuf	,因此要注意到buf不能使用tempBuf，会冲突
	
		if(arrNum == FALSE)
		{
			arrNum = FindExtandArr(&ptrToValue[0],pos);		//AB格式
				
			if (arrNum == FALSE)
			{
				return E_Parameter;
			}
		}
		
		arrName = (BYTE)((EF_ARR>>8)&0xFF);
	}
	
	arrName = ((arrName == 0x2F)?F_ARRNAME_2F:F_ARRNAME_6F); //modify @20140727 ,bit7为0表示arrName 是2F ，为1表示arrName；bit0-bit6表示EF 文件对应的RFM 权限
	
	result = FindTag(&ptrToValue[0],0x88,pos);									//short file id

  	if(result == FALSE)
	{
		shortFileId = fileName&0x1F;			//5 least signification bits in the FID used as SFI

		if(shortFileId == 0x1F)	//SFI should in range from 0x01 to 0x1E,add at 2007-02-07
		{
			shortFileId = 0x00;			
		}
	}
	else
	{
		if(ptrToValue[result] != 0x00)
		{
			shortFileId= (ptrToValue[result+1]>> 3)&0x1F;
		}
		else
		{
			shortFileId = 0x00;									//Not support SFI
		}
	}

	result = FindTag(&ptrToValue[0],0x80,pos);					//File Size

	if(result == FALSE)
	{
	 	return E_Parameter;
	}
	
	updatetag = FALSE;         //默认不是强更新文件

	updatetag  = FindTag(&ptrToValue[0],0x91,pos);					//File Size
	
	if((updatetag != FALSE) && (ptrToValue[updatetag] != 0x00))
	{
		updatetag = ptrToValue[updatetag+1];        //备份个数
		
		if(IF_SupportHighUpdate(updatetag) == FALSE)     //带了9101xx  但是不支持这个功能当做普通文件处理
		{
			updatetag = FALSE;
		}
		
		if(updatetag == 0x01)   //配置一个 没必要
		{
			return E_Parameter; 
		}
		
		if(((fileType&0x37) != TRANTYPE) && ((updatetag & Hige_UpdateWithRAM) == Hige_UpdateWithRAM)) //强更新RAM 支持透明文件
		{
			return E_Parameter;
		}

	}
	else
	{
		updatetag = FALSE;
	}
	
	UpdateConfigIndex = 0;
	

	fileSize = (((WORD)ptrToValue[result+1] << 8) & 0xFF00) | ptrToValue[result+2];
	
/*
对于特定的敏感数据文件或已定义‘93’tag的文件,设置为不能读或映射
*/
#if UICC_CreatFile_93Tag
	if(((G_CurrentDfName[G_ChannelNum] == DF_PHASE)&&(fileName == EF_KI)) || ((G_CurrentDfName[G_ChannelNum] == DF_MF)&&(fileName == EF_AuthCFG)) 			//对于KI等特殊文件,默认为不可读,不可映射
		|| ((G_CurrentDfName[G_ChannelNum] == DF_MF)&&(fileName == EF_KEY))||((G_CurrentDfName[G_ChannelNum] == DF_MF)&&(fileName == EF_Single4G_KI))
		||((G_CurrentDfName[G_ChannelNum] == DF_DCS1800)&&(fileName == EF_KI)))
	{
			shortFileId |= File_MapAndRead;		
	}
	else
	{
		result = FindTag(&ptrToValue[0],Tag_File_MapAndRead,pos);							//Expand for File Map and File Read

		if(result != FALSE)
		{
			if(ptrToValue[result] != 0x01)
			{
				return E_Parameter;
			}
			if(ptrToValue[result+1] == 0x01)				  		//仅为0x01时有效
			{
				shortFileId |= File_MapAndRead;
			}
			else
			{
				shortFileId &= 0x7F;
			}
		}

	}
#endif	

	result = FindTag(&ptrToValue[0],0x8F,pos);							//Expand for FileMapping   

	if(result != FALSE)
	{
		if(ptrToValue[result]==0x06)
		{
			efFileHead.MapFile[0] = (((WORD)ptrToValue[result+1]<<8)&0xFF00) | ptrToValue[result+2];
            
			efFileHead.MapFile[1] = (((WORD)ptrToValue[result+3]<<8)&0xFF00) | ptrToValue[result+4];

			efFileHead.MapFile[2] = (((WORD)ptrToValue[result+5]<<8)&0xFF00) | ptrToValue[result+6];
		}
		else if(ptrToValue[result] == 0x04)
		{
			efFileHead.MapFile[0] = (((WORD)ptrToValue[result+1]<<8)&0xFF00) | ptrToValue[result+2];
        
			efFileHead.MapFile[1] = efFileHead.MapFile[0];
        
			efFileHead.MapFile[2] = (((WORD)ptrToValue[result+3]<<8)&0xFF00) | ptrToValue[result+4];
		}
		else if(ptrToValue[result] == 0x02)
		{
			efFileHead.MapFile[0] = 0x3F00;
        
			efFileHead.MapFile[1] = 0x3F00;
        
			efFileHead.MapFile[2] = (((WORD)ptrToValue[result+1]<<8)&0xFF00) | ptrToValue[result+2];
		}
		else	//for baihe-test
		{
			return E_Parameter;	
		}
		
/*
对于特定的敏感数据文件或已定义‘93’tag的文件,设置为不能读或映射
*/
#if UICC_CreatFile_93Tag	
		if(((efFileHead.MapFile[1] == DF_PHASE)&&(efFileHead.MapFile[2] == EF_KI)) || ((efFileHead.MapFile[1] == DF_MF)&&(efFileHead.MapFile[2] == EF_AuthCFG)) 		//KI文件6F1B(3F00/2E1B),鉴权OPC参数文件2E00,PIN文件2E10等不允许被映射
			|| ((efFileHead.MapFile[1] == DF_MF)&&(efFileHead.MapFile[2] == EF_KEY))||((efFileHead.MapFile[1] == DF_MF)&&(efFileHead.MapFile[2] == EF_Single4G_KI))
		||((efFileHead.MapFile[1] == DF_DCS1800)&&(efFileHead.MapFile[2] == EF_KI)))
		{
			return E_Parameter;
		}
		

		mapFileAddr = SearchFile(E_FILE_START,efFileHead.MapFile[0]);						   //电信5G平台代码复用

		if(mapFileAddr != FALSE)
		{
			mapFileAddr = SearchFile(mapFileAddr,efFileHead.MapFile[1]);					  	
		}
		
		if(mapFileAddr != FALSE)
		{
			mapFileAddr = SearchFile(mapFileAddr,efFileHead.MapFile[2]);					  	
		}
		
		
		if(mapFileAddr != FALSE)
		{		
			ReadEeprom((BYTE *)&result,mapFileAddr + F_MapAndRead_Offset,sizeof(BYTE));   //带有93Tag,值为1,则该文件不允许被映射
	
			if((result&File_MapAndRead) == File_MapAndRead)
			{
				return E_Parameter;
			}
		}
#endif		

		fileStatus = fileStatus|F_MAP_FILE;

		fileSize = 0x0000;

	    if(sizeof(EFileHead) >  GetNVMAvailMemory()) 	//映射文件只判断文件头空间是否足够,修改缺陷279
		{
			return E_FileSize;
		}

	}
	else
	{
		fileStatus = fileStatus&(~F_MAP_FILE);
         
		efFileHead.UpdateConfignum = 0x00; //默认不是强更新文件	

		if((fileType&0x37) == TRANTYPE)  //透明文件
		{
			efFileHead.efSize.SpaceSize = fileSize;			
		
		}
		else 
		{
			result = FindTag(&ptrToValue[0],0x82,pos);

			efFileHead.efSize.recInfo[1] = ptrToValue[result+4];						//Record length in FCP is WORD
			
			if(efFileHead.efSize.recInfo[1] == 0x00)   //长度为0 也报错
			{
				return E_Parameter;
			}
	
			if ((fileSize%efFileHead.efSize.recInfo[1]) != 0x00)
			{
				return E_Parameter;
			}
			
			if(((fileType&0x37) == CYCTYPE) && (efFileHead.efSize.recInfo[1] == 0xFF)) //循环文件大小最大 254
			{
				return E_Parameter;
			}
			
			if((fileSize/efFileHead.efSize.recInfo[1]) > 254)
			{
				return E_Parameter;
			}

			efFileHead.efSize.recInfo[0] = (BYTE)((fileSize/efFileHead.efSize.recInfo[1])&0x00FF);	
			
			if((fileType&0x37) == CYCTYPE)  
			{
				if(efFileHead.efSize.recInfo[0] == 0xFF )   //不允许创建255条的循环记录文件
				{
					return E_Parameter;
				}
								
				fileSize = efFileHead.efSize.recInfo[0] * efFileHead.efSize.recInfo[1] + 1;   //循环文件的实际大小,记录指针放在文件体的第一个字节

			}
					
		}

			if((fileSize + sizeof(EFileHead)) >  GetNVMAvailMemory()) //文件头和文件体的大小一起判断
			{
			   return E_FileSize;
			}

			allocateAddr = 	SecureDataStore(fileName,(WORD*)&efFileHead.FileContentRef);

      if(allocateAddr == FALSE)
			{
			    allocateAddr = VMHeapAlloc(fileSize, (WORD*)&efFileHead.FileContentRef); //文件体分配的地址
			}
			else
			{
				fileType |= 0x80;
				
				fileSize = 0;
				
			}

		
			if(allocateAddr == ADDR_IS_NULL)  //空间不够
			{
				return E_FileSize;
			}						
			
	
			if(updatetag != FALSE)     //强更新文件
			{
					updateAddr = GetAddrFormHigh_Update_START(fileSize,&efFileHead.UpdateConfignum,updatetag,fileType,&HighUpdateCfgInfo[0]);
				
				  	if(updateAddr != ADDR_IS_NULL)
					{
						UpdateConfigIndex = efFileHead.UpdateConfignum;			
					}
					else
					{
						return E_FileSize;
					}
			}				
	}
	
	need_commit = IF_BeginTransaction(); //开启事务	 

	if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)           
	{
		arrName |= 0x7F;       //RFM创建的文件拥有RFM 的任何权限	
	}
	else
	{
		result = FindTag(&ptrToValue[0],0x8E,pos);				//03.48 RFM权限

		if((result == FALSE))                     //TAG不存在 默认开启所有RFM权限
		{			
			arrName |= 0x7F;             //和测试部 产业部讨论 不带8E tag时默认开启全部权限，如果需要关闭或指定某些权限根据需求配置 
		}
		else if((ptrToValue[result] == 0))		            //TAG长度为0 关闭所有RFM权限
		{
			arrName &= F_RFM_PERMIT_NONE;
		}
		else
		{
			arrName |= (ptrToValue[result+1]&0x7F);    //arrName的高bit被用来定义是6F还是2F ，低7bit被用来	扩展RFM 权限
		}
	}
  
	
	/*
	result = FindTag(&ptrToValue[0],0x8E,pos);				//RFM权限	
	
	if(result == FALSE)										//如果0x8E不存在，则默认允许RFM
	{
		fileStatus |= F_RFM_PERMIT_UNICOM;					
	}
	else
	{		
		if((ptrToValue[result+1] & F_RFM_PERMIT_UNICOM) != 0x00)	
		{
			fileStatus |= (ptrToValue[result+1] & F_RFM_PERMIT_UNICOM);
		}
		else
		{
			fileStatus &= 0xCF;
		}			
	}
	*/

	fileNeedSize = sizeof(efFileHead);//文件头所需大小

    mfUsedSize = GetFileHeadUsedSize(E_FILE_START);

	efFileHead.FileId = fileName;
	
	efFileHead.FileType = fileType;
	
	efFileHead.FileStatus = fileStatus;
	
	efFileHead.ArrNum = arrNum;

	efFileHead.ArrName = arrName;

	efFileHead.ShortFileId = shortFileId;
  
	if((fileType&0x3F) != 0x39)  //非 BER-TLV文件
	{
		result = FindTag(&ptrToValue[0],0xA5,pos);
	
		if(result == FALSE)
		{
			result = FindTag(&ptrToValue[0],0x85,pos);
		}
		
		if(result == FALSE)
		{
			ptrToValue = NULL;	
		}
		else
		{
			ptrToValue = &ptrToValue[result];	
		}
	}	
	
	
	
	datalen = E_FILE_START + mfUsedSize - (G_CurrentDfAddr[G_ChannelNum] + fileUseSize); //判断要创建的文件头是否需要移动
	
	if(datalen > 0)
	{
		MoveHugeEepromEXPrepare(G_CurrentDfAddr[G_ChannelNum]+ fileUseSize + fileNeedSize,	G_CurrentDfAddr[G_ChannelNum] + fileUseSize,datalen,0);	

		WriteEepromEX(E_CREATEPROTECT, (BYTE*)&efFileHead.FileId, sizeof(efFileHead));
		
		MoveHugeEepromEXPrepare(G_CurrentDfAddr[G_ChannelNum] + fileUseSize, E_CREATEPROTECT, sizeof(efFileHead), 1);
	}
	else
	{
		WriteEepromEX(G_CurrentDfAddr[G_ChannelNum] + fileUseSize, (BYTE*)&efFileHead.FileId, sizeof(efFileHead));
	}
		
	
	
	fileBodyEnd =  E_FileBodyStart -  GetFileContentSize(E_FILE_START);//GetFileBodyEndAddr();
	
	if((updatetag == FALSE) && ((fileType & 0x80) == 0))     //非强更新文件,且非敏感数据区,且非映射文件
	{
		if((fileStatus&F_MAP_FILE) != F_MAP_FILE)
		{
			bodyMovelen = (getFileContentAddr(G_CurrentDfAddr[G_ChannelNum]) - GetFileContentSize(G_CurrentDfAddr[G_ChannelNum])) - fileBodyEnd; //判断要创建的文件头是否需要移动
		}		
	}
	
	if(bodyMovelen > 0)//强更新不需要移动
	{
		MoveHugeEepromEXPrepare(fileBodyEnd-fileSize, fileBodyEnd,bodyMovelen,2);
		
		SyncAddrIndexTable(allocateAddr + fileSize, -fileSize,D_CreateFile);		
		
	}
	
	if((updatetag & Hige_UpdateWithEep) == 0x00) //普通文件  或者强更新在RAM的文件
	{
		updateAddr = allocateAddr;//强更新在RAM，或者普通文件使用allocateAddr，强更新在eeprom使用updateAddr，后面统一使用updateAddr
	}
	
	if(((fileStatus&F_MAP_FILE) != F_MAP_FILE) && ((fileType&0x37) == CYCTYPE)) //not map file,and 循环文件的话，需要将循环指针先备份起来，断电时恢复
	{
		 SetEepromEX(E_CREATEPROTECT+sizeof(FileSystemHead), 1, 1);	

		 MoveHugeEepromEXPrepare(updateAddr, E_CREATEPROTECT+sizeof(FileSystemHead), 1, 3);		
	}
	
	if((fileStatus&F_MAP_FILE) != F_MAP_FILE)//非映射文件才会有文件体
	{
		VMWriteUword((E_ADDR_INDEX_TABLE + ( efFileHead.FileContentRef-1)*sizeof(UWORD)),updateAddr);	
	}	
			

	i = modifyGrandParentSize2(fileNeedSize,fileSize,(D_CreateFile|EF));//修改各父目录属性信息
	
	if(i == DF_Depth)  //COS 最大支持5级DF
	{
        IF_AbortTransaction(need_commit);
		
		return E_Parameter;
	}	
	
	IF_CommitTransaction(need_commit);  //提交事务
	
	MoveHugeEepromRunning(); //移动内容

	if((fileStatus&F_MAP_FILE) != F_MAP_FILE)			//forbid init map file
	{						
		if((fileType&0x37) == TRANTYPE)
		{
			InitTransFile(updateAddr,ptrToValue,efFileHead.efSize.SpaceSize,updatetag);  //初始化文件体信息
		}
		else
		{
		   	if((fileType&0x37) == CYCTYPE) 
			{
			    SetEepromEX(updateAddr,1,1);//初始化循环文件的记录指针字节
				InitRecordFile(updateAddr+1,ptrToValue,efFileHead.efSize.recInfo[0],efFileHead.efSize.recInfo[1]);
			}
			else
			{
				InitRecordFile(updateAddr,ptrToValue,efFileHead.efSize.recInfo[0],efFileHead.efSize.recInfo[1]);
			}		

		}

		if(updatetag!=0x00)
		{
		    IF_Init_High_Update_Config(UpdateConfigIndex,HighUpdateCfgInfo);
		}
	    
	}    
		
	return fileNeedSize;	
}



/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               
  Function name :	 GetLastDFHeadAddr                                                                                                                   
                                                                                                                                                
  input parameter:  void
                   					                                                                                                   
  output parameter:  last DFHeader Addr                                                                                                       
                                                                                                                                                
  description:     get last DFHeader Addr   
					 
  Current status:    Code OK!  Test OK!    
-------------------------------------------------------------------------------------------------------------*/
UWORD GetLastDFHeadAddr(UWORD currentDFAddr)
{
	UWORD xdata lastDFHeadAddr;
	BYTE  idata fileType;
   	UWORD xdata fileUseSpace,fileSpaceAddr,fileAddr;
	
	lastDFHeadAddr = currentDFAddr;

	fileUseSpace = GetFileHeadUsedSize(currentDFAddr);
	
    fileUseSpace = fileUseSpace + currentDFAddr;

   	for(fileAddr = currentDFAddr+sizeof(FileSystemHead);fileAddr < fileUseSpace;fileAddr = fileAddr + fileSpaceAddr)
   	{  	
		fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

		if(fileType == ADFTYPE)
		{
			fileSpaceAddr = GetFileHeadUsedSize(fileAddr);

            lastDFHeadAddr = fileAddr;		
		}
		else if((fileType == TRANTYPE) || (fileType == LINTYPE)||(fileType == CYCTYPE))
		{		
			fileSpaceAddr = sizeof(EFileHead) ;	//fileSpaceAddr as Space
		}
		else
		{
			return lastDFHeadAddr;
		}
   	}

	return lastDFHeadAddr;	
	
}



/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               
  Function name :	 CreateDF                                                                                                                   
                                                                                                                                                
  input parameter: 
                     fileMaxSize:  the parent file max size

                     fileUseSize:  the parent file used size
					                                                                                                   
  output parameter:  the file will used space/false                                                                                                          
                                                                                                                                                
  description:       compare the space and write df file head
					 
  Current status:    Code OK!  Test OK!    
-------------------------------------------------------------------------------------------------------------*/
UWORD CreateDFFileSECURE(UWORD fileUseSize,WORD fileName)
{
	FileSystemHead xdata dfFileHead;
	UWORD xdata fileSpareSize,mfUsedSize,datalen,allocateAddr,lastDFHeadAddr;
	BYTE  idata result,pos,i,j;
	BYTE  * ptrToValue;
	BYTE idata need_commit;
	
	need_commit = FALSE;
		
	if(sizeof(FileSystemHead) > GetNVMAvailMemory())
	{
		return E_FileSize;
	}

	memset((BYTE *)&dfFileHead, 0x00, sizeof(FileSystemHead));

	dfFileHead.FileId = fileName;

	pos = 1 + FindTag(&RecData[0],0x62,P3);

	ptrToValue = &RecData[pos];

	pos = P3 - pos;

	result = FindTag(&ptrToValue[0],0x82,pos);

	dfFileHead.FileType = ptrToValue[result+1]&0xF8;

	result = FindTag(&ptrToValue[0],0x8A,pos);

	if(result == FALSE)
	{
		return E_Parameter;		
	}
	
	result = FindTag(&ptrToValue[0],0x81,pos);

	if(result == FALSE)
	{
	 	return E_Parameter;
	}	
	
	
	lastDFHeadAddr = GetLastDFHeadAddr(G_CurrentDfAddr[G_ChannelNum]);
	
	if(GetFileHeadUsedSize(lastDFHeadAddr) == sizeof(FileSystemHead))//not support create two null DF continuously
	{
		return E_Parameter;
	}	
	
	dfFileHead.UsedHeadSize = sizeof(FileSystemHead);

	dfFileHead.ParentName[1] = G_CurrentDfName[G_ChannelNum];

	dfFileHead.ParentName[0] = GetFileHeadWord(G_CurrentDfAddr[G_ChannelNum], F_SecondParentName_Offset); 		

	if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)
	{
		result = FindTag(&ptrToValue[0],0x84,pos);	//AID mode 远程文件管理中，卡片不允许create file创建ADF	

		if(result != FALSE)
		{
			return 	E_Parameter;
		}	
	}
	
	result = FindTag(&ptrToValue[0],0x8B,pos);									//Referenced mode
	
	if((result != FALSE)&&(ptrToValue[result]==0x03))
	{
		  dfFileHead.ArrNum = ptrToValue[result+3];

	    dfFileHead.ArrName = ptrToValue[result+1];
	}
	else
	{
		result = FindArrNum(&ptrToValue[0],pos);	 	//Compact mode for RFM	 	//该函数内部会使用到tempBuf	,因此要注意到buf不能使用tempBuf，会冲突
	
		if(result == FALSE)
		{
			result = FindExtandArr(&ptrToValue[0],pos);		//AB格式
				
			if (result == FALSE)
			{
				return E_Parameter;
			}
		}

		dfFileHead.ArrNum = result;

		dfFileHead.ArrName = (BYTE)((EF_ARR>>8)&0xFF);
	}
	
	
	dfFileHead.ArrName = (dfFileHead.ArrName == 0x2F?F_ARRNAME_2F:F_ARRNAME_6F);  //mofiby @20140727 修改该字节的函数，bit7表示对应的arrName是2F还是6F，bit0-bit6保留作为EF 文件的RFM 权限
	
	result = FindTag(&ptrToValue[0],0xC6,pos);

	if(result == FALSE)
	{
		return E_Parameter;
	}

	for(j=0x01;ptrToValue[result+j] != 0x83;j = j+3){;}

	for(i=0x00;i<0x04;i++)
	{
		if((j+(i*3))< ptrToValue[result])
		{
			dfFileHead.ChvStatus[i] = ptrToValue[result+j+(i*3)+2];
		}
		else
		{
		   dfFileHead.ChvStatus[i] = 0xFF;
		}
	}

	result = FindTag(&ptrToValue[0],0x84,pos);								//AID mode								

#ifndef  SingleSIM
	if(result != FALSE)
	{
		fileSpareSize = SelectFileByName(G_CurrentDfAddr[G_ChannelNum],(BYTE *)&ptrToValue[result+1],ptrToValue[result]);

		if(fileSpareSize != FALSE)
		{
			return E_DfNameExist;
		}

		dfFileHead.DIRNum = GetAIDNumber(&ptrToValue[result]);
	}
	else
#endif
	{
	    dfFileHead.DIRNum = 0x00;
	}
	
	need_commit = IF_BeginTransaction();	
	
	mfUsedSize = GetFileHeadUsedSize(E_FILE_START);
		
	
	allocateAddr = VMHeapAlloc(0, (WORD*)&dfFileHead.FileContentRef);
	
	if(allocateAddr == ADDR_IS_NULL)  //空间不够
	{
		return E_FileSize;
	}
	
	
			
	VMWriteUword((E_ADDR_INDEX_TABLE + (dfFileHead.FileContentRef-1)*sizeof(UWORD)),allocateAddr);		

	datalen = E_FILE_START + mfUsedSize - (G_CurrentDfAddr[G_ChannelNum] + fileUseSize); //判断要创建的文件是否在MF的最末尾
	
	if(datalen > 0)
	{
		MoveHugeEepromEXPrepare(G_CurrentDfAddr[G_ChannelNum]+ fileUseSize + sizeof(FileSystemHead), G_CurrentDfAddr[G_ChannelNum] + fileUseSize,datalen,0);
		
		WriteEepromEX(E_CREATEPROTECT, (BYTE *)&dfFileHead.FileId,sizeof(FileSystemHead));
		
		MoveHugeEepromEXPrepare(G_CurrentDfAddr[G_ChannelNum]+ fileUseSize, E_CREATEPROTECT,sizeof(FileSystemHead),1);
	}
	else
	{
		WriteEepromEX(G_CurrentDfAddr[G_ChannelNum]+ fileUseSize, (BYTE *)&dfFileHead.FileId,sizeof(FileSystemHead));
	}
			
	
	i = modifyGrandParentSize2(sizeof(FileSystemHead),0,(D_CreateFile| DF));
	
	if(i >= (DF_Depth-1) )  //cos最大支持3级DF
	{
		IF_AbortTransaction(need_commit);
		
		return E_Parameter;
	}
	
	IF_CommitTransaction(need_commit);
	
	MoveHugeEepromRunning();
		
	return fileUseSize;
}
#endif

#endif



//#endif


