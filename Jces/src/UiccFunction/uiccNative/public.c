/*

 * Copyright (c) 2005,eastcompeace

 * All rights reserved.

 * MODEL: 	ALL

 * version  V1.4

 * Desin    蔡勃

 * Coder:	蔡勃

 * HISTORY: V1.0 创建源文件 
		    V1.1 修改AccessCondition()，当执行权限为NEVER时，转换为校验DFKEY权限
			V1.2 函数SearchFile中增加BER-TLV文件的判断
			V1.3 函数SelectFileByName中增加BER-TLV文件的寻址
			V1.4 修改ReceiveData():增加接口函数IF_BeforeReceiveData()；
			V1.5 修改AccessCondition()：增加RFM权限的判断
			V1.6 修改函数SearchContaining，减少该函数的入参个数
			V1.7 修改函数SearchFile，在查找文件时，若当前地址为EF时，若当前地址所指向的文件并不是待查找的文件，则直接退出，不再进行后续查找
			v1.8 修改AccessCondition,修改当RFM 时，对于DF文件来说，权限总是返回TRUE@20110617
			V1.9 修改函数SearchContaining，增加对循环文件的特殊处理 @20110722 by xuyz
			V1.10 修改函数GetAIDNameInFile，增加判断AID数据的有效性@20111220
			V1.11 修改函数AccessCondition，将NEV权限文件的读操作由验证MF Key更改为验证Super Key @20141110 by xuyz
			V1.12 修改函数FdnProgcess,FileLifeCycleHandle,IsFdnServiceActiveInSIM,支持FDN过程中操作的文件是映射文件       @20170814 by yangjian
			V1.13 修改函数SearchContaining,加快seek指令的执行速度			    @20180115 by xyz
			V1.14 修改函数SelectFileByName，该函数中不再使用tempBuf,而使用定义的BYTE  xdata buf[17]		     @20180713 by yangjian
			V1.15 修改函数SearchContaining,使得Seek指令执行速度接近恒宝卡片    						@20180809 by yangjian
			V1.16 修改函数AccessCondition,在此函数中添加IF_AccessCondition,兼容中国电信平台     	@20190114 by yangjian
			V1.17 修改函数UsimFdnSYNCtoSIM,IsFdnServiceActiveInUSIM与SimFdnSYNCtoUSIM,宏DF_ADF使用全局变量G_DF_ID_USIM替代,增加代码兼容性     @20221206 by yangjian

 */
#include  "typedef.h"
#include  "uicc_main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include  "interface.h"
#include  "stkhead.h"
//#include "Transaction.h"
#include "Interface_LTEFromBoot.h"
#include "uicc_error.h"
#include    "ToolkitRFM.h"

BYTE xdata G_FindCH;

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ReceiveData                                                                                                                   
                                                                                                                                                
   Input parameter:   void                                                                  
  					                                                                                                         
   Output parameter:  void                                                                                                               
                                                                                                                                                
   Description:       收据数据到I/O 缓冲区从ME

   Coder:             CAIBO               Tester:   
  
   Status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/    

BYTE ReceiveData(void)                                                                  //接收数据到RecData缓冲区中，最多个
{
	ClearBit(G_SystemStatus,B_needCopy);

	if(ValueBit(G_SystemStatus,B_RfmPerform) == FALSE)
	{
		if(P3 == 0x00)
		{
			return FALSE;
		}
		
		GlobalReceiveData();  //接收数据后G_CommandStatus = DataReceived;
	}	
	
	return TRUE;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SendData                                                                                                                   
                                                                                                                                                
  Input parameter:   void                                                                  
  					                                                                                                         
  Output parameter:  void
  
  Description:       I/O 缓冲区发送到ME                                                                                                           
                                                                                                                                                
  Coder:             CAIBO               Tester: 
  
  Status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/    
void SendData(void)                                                                       //发送RecData缓冲区中的数据?
{
	if(INS == 0xB0)
	{
#if (SE_ONLY && UICC__OPERATOR__CHINAMOBILE)
		
#if !NOSMARTLUMING
		//SmartLumingRead();	//优选漫游
#endif	
		
#endif		
	}	
	
	SetBit(G_SystemStatus,B_needCopy);

	if(ValueBit(G_SystemStatus,B_RfmPerform) == FALSE)
	{
		GlobalSendData();
	}
}



/*
*@Function: 读取一个 BYTE
*
*@Descrip:  VM统一使用该接口，这里可以读取RAM或FLASH中的数据，因此，传入地址必须是物理地址
*
*@See:
*
*@Param:    srcAddr-读取的数据在FLASH或RAM中的物理地址
*
*@Return:   BYTE-读取一个字节的值
*
*@Author:   zzj 2014-4-23
*
*@History:  V0.0--
*/
BYTE VMReadByte(UWORD srcAddr)
{
	BYTE idata value;
	
	ReadEeprom(&value, srcAddr, 1);

	return value;
}


/*
*@Function:读地址
*
*@Descrip: 涉及大小端问题，用于读取指定物理地址的UWORD类型数据(通常是地址数据)
*
*@See:
*
*@Param:    srcAddr-待读取数据的目标物理地址
*
*@Return:   UWORD-读取的4字节数据
*
*@Author:   zhxx 2020-2-19
*
*@History:  V0.0--
*/
UWORD VMReadUword(UWORD srcAddr)
{
	BYTE xdata value[4];
	UWORD xdata destValue;

	ReadEeprom(value, srcAddr, 4);

	destValue = value[0];
	destValue = (destValue<<8) + value[1];
	destValue = (destValue<<8) + value[2];
	destValue = (destValue<<8) + value[3];
	
	return(destValue);
}




/*
*@Function:写地址
*
*@Descrip:  涉及大小端问题，用于写入指定物理地址的UWORD类型数据（通常是地址数据）
*
*@See:
*
*@Param:    destAddr-目标物理地址
*           value-要写入的UWORD类型数据
*
*@Return:   void
*
*@Author:   zhxx 2020-2-19
*
*@History:  V0.0--
*/
void VMWriteUword(UWORD destAddr, UWORD value)
{
	BYTE xdata tmp[4];

	tmp[0] = ((value>>24)&0xFF);

	tmp[1] = ((value>>16)&0xFF);

	tmp[2] = ((value>>8)&0xFF);

	tmp[3] = (value&0xFF);

	//SetBit(G_COMMANDPATH,B_ACCESSEX); //从ACCESSEX中进入擦写函数，为了保证擦写函数执行的指令路径

	//WriteEepromEX(destAddr,tmp,4); 
	VMWriteBuf(destAddr,tmp,4); 
}


UWORD GetSpaceFileAddr(UWORD fileAddr)
{
	BYTE  idata fileType;
	UWORD xdata fileSpaceAddr;

	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

	if(fileType == ADFTYPE)
	{
		fileSpaceAddr = GetFileHeadUsedSize(fileAddr);
	}
	else if((fileType == TRANTYPE) || (fileType == LINTYPE)||(fileType == CYCTYPE))
	{
	
		fileSpaceAddr = sizeof(EFileHead) ;	//fileSpaceAddr as Space
	}
	else
	{
		return FALSE;
	}

	return fileSpaceAddr;
}


/*
*@description: 根据文件类型,获取文件所占空间大小
*
*@function    :GetFileTotalSize
*
*@param        :UWORD fileAddr
*
*@return      LfileTotalSize
*
**@author    : create by xuyz
*
*@history    :V1.0 :create by 
*/
UWORD GetFileTotalSize(UWORD fileAddr)
{
	UWORD xdata fileSpaceSize;
	WORD  idata tempParameter;
	BYTE  idata fileType,recNumberLgth[2],fileStatus;
	
	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

	if(fileType == ADFTYPE)
	{
		fileSpaceSize = GetFileHeadUsedSize(fileAddr);	
	}
	else if(fileType == TRANTYPE)
	{
		#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
			if(IF_IsHeFlashFile(fileAddr))
			{
				tempParameter = 0x00;    //强更新文件体在强更新区域
			}
			else
		#endif	
			{	
				tempParameter = GetTFSize(fileAddr);		//tempParameter as SpaceSize
			}

			fileSpaceSize = tempParameter+sizeof(EFileHead);
	}
	else if((fileType == LINTYPE)||(fileType == CYCTYPE))
	{
		ReadEeprom((BYTE  *)&recNumberLgth[0],fileAddr+F_RecNum_Offset,sizeof(recNumberLgth));
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
		if(IF_IsHeFlashFile(fileAddr))
		{
			fileSpaceSize = sizeof(RecordFileHead) ;
		}
		else
#endif
		{
			fileStatus = GetFileHeadByte(fileAddr,F_Status_Offset);
		
			fileSpaceSize = (UWORD)(recNumberLgth[0]*recNumberLgth[1])+sizeof(EFileHead) ;	//fileSpaceAddr as Space

			if((fileType == CYCTYPE) && ((fileStatus & F_MAP_FILE) != F_MAP_FILE))//
			{
				fileSpaceSize++; //
			}
		}
	}
	else
	{
		return FALSE;
	}
	
	return fileSpaceSize;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SearchFile                                                                                                                   
                                                                                                                                                
  Input parameter:   fileAddr(the start addr of df,mf,adf)

					 filename(df,ef,name)                                                                  
  					                                                                                                         
  Output parameter:  fileAddr/FALSE                                                                                                               
                                                                                                                                                
  Description:       在当前给出的DF或ADF下查找文件,如果存在返回文件的起始地址,对fileAddr不做条件判断,不判断文件映射.
  
  Coder:		     CAIBO        Tester:     

  Test Status:         

 -------------------------------------------------------------------------------------------------------------------*/    
UWORD SearchFile(UWORD  fileAddr,WORD fileName)
{
	WORD  idata tempParameter;
	BYTE  idata fileType;
   	UWORD xdata fileUseSpace,fileSpaceAddr;

	ReadEeprom((BYTE *)&tempParameter,fileAddr,sizeof(WORD));									//select df/mf self

	if(tempParameter == fileName)																	//tempParameter as FileId
	{
	 	return fileAddr;
	}

	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);   //若当前地址所指向的文件并不是待查找的文件，则只有在当前地址所指向的文件为DF时，才继续往下查找，否则直接退出
	
	if(fileType != DFTYPE)
	{
		return FALSE;
	}
	fileUseSpace = GetFileHeadUsedSize(fileAddr);
	
    fileUseSpace = fileUseSpace + fileAddr;

	fileName=IF_SelectFile(fileName);

   	for(fileAddr = fileAddr+sizeof(FileSystemHead);fileAddr < fileUseSpace;fileAddr = fileAddr + fileSpaceAddr)
   	{
        ReadEeprom((BYTE *)&tempParameter,fileAddr,sizeof(WORD));

		if(tempParameter == fileName)																//tempParameter as FileId
		{
			return fileAddr;
		}
  	
		fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

		if(fileType == ADFTYPE)
		{
			fileSpaceAddr = GetFileHeadUsedSize(fileAddr);	
		}
#if UICC__FUNCTION__BERTLVCmd
		else if(fileType == 0x39)                                       //BER-TLV
		{
			 tempParameter = GetFileHeadWord(fileAddr, F_SpaceSize_Offset);	

			 fileSpaceAddr = tempParameter+sizeof(BERTLVFileHead);

			 tempParameter = GetFileHeadWord(fileAddr, F_BERTLV_ReservedSize);

			 fileSpaceAddr = tempParameter +  fileSpaceAddr;
		}
#endif
		else if((fileType == TRANTYPE) || (fileType == LINTYPE)||(fileType == CYCTYPE))
		{
#if 	HEAD_BODY_TOGETHER
			fileSpaceAddr = GetFileTotalSize(fileAddr);
#else
			fileSpaceAddr = sizeof(EFileHead) ;	//fileSpaceAddr as Space
#endif			
			
			
			
		}
		else
		{
			return FALSE;
		}
   	}

	return FALSE;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SelectFile                                                                                                                   
                                                                                                                                                
  Input parameter:   fileAddr(the start addr of df,mf,adf)

					 filename(df,ef,name)                                                                  
  					                                                                                                         
  Output parameter:  fileAddr/FALSE                                                                                                               
                                                                                                                                                
  Description:       在当前给出的DF或ADF下查找文件,如果存在返回文件的起始地址,对于映射文件返回实体地址.
  
  Coder:		     CAIBO        Tester:     

  Test Status:         

 -------------------------------------------------------------------------------------------------------------------*/ 
UWORD SelectFile(UWORD  fileAddr,WORD fileName)
{
	BYTE  idata fileType;

	if(fileName == 0x7F21)
    {
        fileName = 0x7F20;
    }
		

	fileAddr = SearchFile(fileAddr,fileName);

	if(fileAddr == FALSE)
	{
	 	return FALSE;
	}

	fileType  = GetFileHeadByte(fileAddr,F_Type_Offset);

	if(fileType == ADFTYPE)
	{
	 	return fileAddr;
	}

	G_CurrentSfi = GetFileHeadByte(fileAddr,F_ShordFid_Offset);					//Add at 2007-02-07

	fileType = GetFileHeadByte(fileAddr,F_Status_Offset);

	if((fileType&F_MAP_FILE) == F_MAP_FILE)
	{
		fileAddr = GetParentOrMapFileAddr(fileAddr, F_MapFile_Offset);			
	}

	return fileAddr ;
	
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SelectFileByName                                                                                                                   
                                                                                                                                                
  Input parameter:   fileaddr(the start addr of df,mf,adf)

					 filename(df,ef,name)                                                                  
  					                                                                                                         
  Output parameter:  fileAddr/FALSE                                                                                                               
                                                                                                                                                
  Description:       在当前给出的DF或ADF下查找文件,如果存在返回文件的起始地址,对fileAddr不做条件判断.
  
  Coder:		     CAIBO        Tester:     

  Test Status:         

 -------------------------------------------------------------------------------------------------------------------*/    
#ifndef SingleSIM
UWORD SelectFileByName(UWORD fileAddr,BYTE * name,BYTE lgth)
{
	BYTE  idata fileType;
	BYTE  xdata buf[17];
	BYTE  idata fileShortId;
	UWORD xdata fileUseSpace,fileSpaceAddr;

   		
	fileUseSpace = GetFileHeadUsedSize(fileAddr);
	
	fileUseSpace = fileUseSpace+ fileAddr;

   	for(fileAddr = fileAddr + sizeof(FileSystemHead);fileAddr < fileUseSpace;fileAddr = fileAddr + fileSpaceAddr)					 
   	{	 
		fileShortId = GetFileHeadByte(fileAddr, F_ShordFid_Offset);

		fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

		if(fileType == ADFTYPE)	
		{
			if((lgth != 0x00)&&(fileShortId != 0x00))     //查找AID name
			{
				 GetAIDNameInFile(fileShortId,&buf[0]);

				  if((memcmp(&buf[1],&name[0],lgth) == EQUAL)&&(buf[0] >= lgth))
				 {
				  	return fileAddr;
				 }	
			}
		}
		else 
		{
			if(lgth == 0x00)	     //通过短文件标示查找文件
			{
				if(fileShortId == name[0])
				 {
				 	return fileAddr;
				 }	
			}
		}  
		if(fileType == ADFTYPE)
		{
			fileSpaceAddr = GetFileHeadUsedSize(fileAddr);
		}
#if UICC__FUNCTION__BERTLVCmd
		else if(fileType == 0x39)                  //BER-TLV structure
		{
			 fileSize = GetFileHeadWord(fileAddr, F_SpaceSize_Offset);      //max size	

			 fileSpaceAddr = fileSize+sizeof(BERTLVFileHead);

			 fileSize = GetFileHeadWord(fileAddr, F_BERTLV_ReservedSize);		//reserverd size

			 fileSpaceAddr = fileSize +  fileSpaceAddr;
		}
#endif
		else if((fileType == TRANTYPE) || (fileType == LINTYPE)||(fileType == CYCTYPE))
		{			
			#if 	HEAD_BODY_TOGETHER
			fileSpaceAddr = GetFileTotalSize(fileAddr);
#else
			fileSpaceAddr = sizeof(EFileHead) ;	//fileSpaceAddr as Space
#endif			
			
		}
		else
		{
			return FALSE;
		}

   	}

	return FALSE;
}
#endif
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SelectFileByShortId                                                                                                                   
                                                                                                                                                
  Input parameter:   fileaddr(the start addr of df,mf,adf)

					 shortName(df,ef,name)                                                                  
  					                                                                                                         
  Output parameter:  fileAddr/FALSE                                                                                                               
                                                                                                                                                
  Description:       在当前给出的DF或ADF下查找文件,如果存在返回文件的起始地址,对映射文件返回实体地址.
  
  Coder:		     CAIBO        Tester:     

  Test Status:         

 -------------------------------------------------------------------------------------------------------------------*/    
#ifndef SingleSIM
UWORD SelectFileByShortId(UWORD  fileAddr,BYTE shortName)
{
	BYTE	idata	fileType;
	UWORD	idata	addr; 
	WORD     idata  fileName;

	addr = SelectFileByName(fileAddr,&shortName,0x00);							//Modify at 2007-02-07

	if(addr == FALSE)															//Add at 2007-02-07
	{
	 	return FALSE;
	}

	ReadEeprom((BYTE *)&fileName,addr,sizeof(WORD));

	G_CurrentSfi = GetFileHeadByte(addr,F_ShordFid_Offset);					
	
	fileType = GetFileHeadByte(addr,F_Status_Offset);

	if((fileType&F_MAP_FILE) == F_MAP_FILE)
	{
		addr = GetParentOrMapFileAddr(addr, F_MapFile_Offset);			
	}

	SetChannel(addr,fileName, FALSE);

	return addr ;
}
#endif


#if !NATIVE_CARD_UICC
/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   ReadOrWritePukInfo
*
* input parameter:  BYTE pukRef,BYTE * buf,BYTE mode
*
*output parameter:TRUE/FALSE
*
* Description: 该文件主要是读取或者设置puk的值
*
* auther：     zhxx    20190613
*
* history：    V1.0
*
************************************************************************************************************/
BYTE ReadOrWritePukInfo(BYTE pukRef,BYTE * buf,BYTE mode)
{
	BYTE  idata recNum,i,recLen;
	UWORD xdata fileAddr;
	BYTE idata tempData;
	fileAddr = FALSE;
	
	fileAddr = SelectLFFile(DF_PROFILECFG,EF_PRIVATE_PUK);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	recNum = GetLFRecNumLgth(fileAddr,FileNum);
	recLen = GetLFRecNumLgth(fileAddr,FileLgth);
	
	for(i = 1 ; i <= recNum; i++)
	{
		ReadLFBuf(fileAddr,i,0,&tempData,1);
		if(tempData == pukRef)
		{
			if(mode == Read_Key)
			{
				return ReadLFBuf(fileAddr,i,0,&buf[0],recLen);
			}
			else
			{
				return WriteLFBuf(fileAddr,i,0,&buf[0],recLen);
			}
		}
	}
	return FALSE;
}
#endif


#if (!NATIVE_CARD_UICC)
/*
*@funtion    :GetFirstLevelDf
*
*@description: 获取当前DF的第一层级的DF地址：eg：传入5F3A地址，返回7F10地址
*
*@param      :
*
*@return     :UWORD  -- 获取当前DF的第一层级的DF地址
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20200701
*/
UWORD GetFirstLevelDf(UWORD currentDfAddr)
{
	UWORD xdata firstLevelDf,parentfileaddr;
	BYTE idata i;

	if(currentDfAddr == FALSE)
	{
		return FALSE;
	}

	firstLevelDf = currentDfAddr;

	for(i=0; i < DF_Depth; i++)
	{
		parentfileaddr = GetParentOrMapFileAddr(firstLevelDf,F_ParentName_Offset);

		if(parentfileaddr == E_FILE_START)
		{
			break;
		}
		else
		{
			firstLevelDf = parentfileaddr;
		}
	}

	return firstLevelDf;

}
#endif

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ReadOrWriteKey                                                                                                                   
                                                                                                                                                
  input parameter:   TRUE/FALSE                                            
  					                                                                                                         
  output parameter:  BYTE keyId
  					 BYTE * buf                                                                                                           
                     BYTE mode: Read_Key
					 			Write_Key                                                                                                                           
  description:
  
  author:			 ChenL       
  
  status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/
BYTE ReadOrWriteKey(BYTE keyId, BYTE * buf, BYTE mode)
{
  	BYTE  idata num;
	UWORD xdata fileAddr;

	#if NATIVE_CARD_UICC
  fileAddr =  SelectLFFile(DF_MF,EF_KEY);
	#else
	if((keyId >= 0x81) && (keyId <= 0x88))//0x8x的pin从相应的ADF文件下面的pin文件读，否则从MF下面的pin文件里面读，8A~8E仍然从MF下面获取
	{
		fileAddr = GetFirstLevelDf(G_CurrentDfAddr[G_ChannelNum]);

		fileAddr =  SelectFile(fileAddr,EF_PRIVATE_PIN);

	}
	else
	{
		fileAddr =  SelectLFFile(DF_PROFILECFG,EF_PRIVATE_PIN);
	}
	#endif

	if(fileAddr == FALSE)
	{
	 	return FALSE;
	}

	num = LocateRecordInFile(fileAddr,keyId);	 //查找特定类型的PIN

	if(num == FALSE)
	{
		return FALSE;
	}

	if(mode == Read_Key)
	{
		ReadLFBuf(fileAddr,num,0,&buf[0],GetLFRecNumLgth(fileAddr,FileLgth));
	}
	else
	{
		WriteLFBuf(fileAddr,num,0,buf,GetLFRecNumLgth(fileAddr,FileLgth));
	}
	return TRUE;
}



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ReadKey                                                                                                                   
                                                                                                                                                
  input parameter:   TRUE/FALSE                                            
  					                                                                                                         
  output parameter:  BYTE keyId,BYTE * buf                                                                                                           
                                                                                                                                                
  description:       
  
  status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/
BYTE ReadKey(BYTE keyId,BYTE * buf)
{
	BYTE idata result;

	result = ReadOrWriteKey(keyId, buf, Read_Key);

	return result;	  
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 WriteKey                                                                                                                   
                                                                                                                                                
  input parameter:   TRUE/FALSE                                            
  					                                                                                                         
  output parameter:  BYTE keyId,BYTE * buf                                                                                                           
                                                                                                                                                
  description:       
  
  status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/
BYTE WriteKey(BYTE keyId,BYTE * buf)
{
	BYTE idata result;

	result = ReadOrWriteKey(keyId, buf, Write_Key);

	return result;	 
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ReadPinStatus                                                                                                                   
                                                                                                                                                
  input parameter:   void                                           
  					                                                                                                         
  output parameter:  void                                                                                                              
                                                                                                                                                
  description:       set the pin status to the paramater
  
  status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/    

void ReadPinStatus(void)
{ 
    BYTE idata status;
	KeyStructer xdata chvStuct;
	
	BYTE idata i,recNum,num;
	UWORD xdata fileAddr = FALSE,adfFileAddr,pinAddr,j;
	BYTE xdata aidCmpBuffer[0x12];


	G_ChvStatus = 0x00;           //PIN1
	
	//G_AdmStatus = 0x00;         //同步TPX 5G代码，不清掉G_AdmStatus,否则reloadkey不能执行

#if NATIVE_CARD_UICC
	G_KeyStatus = 0;
#else
	memset(G_LocalPinStatus,0x00,sizeof(G_LocalPinStatus));
#endif
	
    for(i = 0x01;i<=0x08;i++)//Global Pin 1-8
	{
		status = ReadKey(i,(BYTE xdata *)&chvStuct.KeyId);

		if(status == TRUE)
		{
			if((chvStuct.KeyStatus&0x01)==PIN_DISABLED)//设置glocal pin1的状态
			{
				SetBit(G_ChvStatus,i-1);//G_ChvStatus Bit0对应pin1
			}	
		}
	}


#if NATIVE_CARD_UICC


    for(i = 0x81;i<=0x88;i++)//Global Pin 1-8
	{
		status = ReadKey(i,(BYTE xdata *)&chvStuct.KeyId);

		if(status == TRUE)
		{
			if((chvStuct.KeyStatus&0x01)==PIN_DISABLED)//设置glocal pin1的状态
			{
				SetBit(G_KeyStatus,(i&0x7F)-1);//G_ChvStatus Bit0对应pin1
			}
		}
	}

#else
	
	fileAddr = SelectLFFile(DF_MF,0x2F00);//遍历0x2F00文件
	
	if(fileAddr != FALSE)
	{
		recNum = GetLFRecNumLgth(fileAddr,FileNum);
		
		for(num=0x00; num < recNum; num++)
		{
			ReadLFBuf(fileAddr,num+1,0x02,&aidCmpBuffer[0],0x12);  //tempBuf[0]- 0x4F,tempBuf[1]-L tempBuf[2]-tempBuf[0x12] : V

			if(aidCmpBuffer[0] == 0x4F)
			{
				adfFileAddr = SelectFileByName(E_FILE_START,&aidCmpBuffer[2],aidCmpBuffer[1]);//遍历每个ADF文件
				
				if(adfFileAddr != FALSE)
				{
					pinAddr =  SelectFile(adfFileAddr,EF_PRIVATE_PIN);
				
					if(pinAddr != FALSE)
					{
						for(i= 0x81;i<=0x88;i++)
						{
							j = LocateRecordInFile(pinAddr,i);	 //查找特定类型的PIN

							if(j != FALSE)
							{
								ReadLFBuf(pinAddr,j,0,(BYTE xdata *)&chvStuct.KeyId,GetLFRecNumLgth(pinAddr,FileLgth));
								
								if((chvStuct.KeyStatus&0x01) == PIN_DISABLED)//设置MF下面每个ADF下的 local pin1的状态
								{
									SetBit(G_LocalPinStatus[num],(i&0x0F)-0x01);
								}
							}
						}
					}
				}			
			}					
		}	
	}

#endif

	for(i = 0x0A;i<=0x0E;i++)//
	{
		status = ReadKey(i,(BYTE xdata *)&chvStuct.KeyId);

		if(status == TRUE)
		{
			if((chvStuct.KeyStatus&0x01) == PIN_DISABLED)
			{
				SetBit(G_AdmStatus,i-9); //这里要和ConfigureKeyStatus使用对应
			}	
		}
	}
	
	for(i = 0x8a;i<=0x8E;i++)
	{
		status = ReadKey(i,(BYTE xdata *)&chvStuct.KeyId);

		if(status == TRUE)
		{
			if((chvStuct.KeyStatus&0x01) == PIN_DISABLED)
			{
				SetBit(G_AdmStatus,(i & 0x0F)-0x04); //这里要和ConfigureKeyStatus使用对应
			}	
		}
	}
}
/*
*@description:清除、设置、获取当前key的校验状态
*
*@param      :keyType:指示当前key的类型
*@param      :mode :0x00-设置当前key为已校验
*					0x01-清除当前key的校验状态
					0x02-获取当前key的状态
*returns 
*
*@history   :create by xyz @100526
*/
BYTE ConfigureKeyStatus(BYTE keyRefense,BYTE mode)
{
	BYTE idata  keyStatus;
	#if !NATIVE_CARD_UICC	
	UWORD xdata fileAddr;
	BYTE idata recNum,result;
	
	fileAddr = FALSE;
	#endif

    if(keyRefense < 0x0A)                 //PIN1
	{
		keyStatus = G_ChvStatus;
		
		keyRefense -= 0x01;//PIN1对应G_ChvStatus bit0
		
		if(mode == SetStatusMode)	
		{
			SetBit(G_ChvStatus,keyRefense);
		}
		else if(mode == ClearStatusMode)
		{
			ClearBit(G_ChvStatus,keyRefense);	
		}
		else //白盒测试需要,代码量不变
		{
			;
		} 
	}
	else if(((keyRefense&0x80)==0x80)&&(keyRefense<0x8A))   
	{
		#if NATIVE_CARD_UICC
		keyStatus = G_KeyStatus;        //PIN2
		
		keyRefense = keyRefense & 0x7F;

		if(mode == SetStatusMode)	
		{
			SetBit(G_KeyStatus,keyRefense);
		}
		else if(mode == ClearStatusMode)
		{
			ClearBit(G_KeyStatus,keyRefense);	
		}
		else   //白盒测试需要，代码量不变
		{
			;
		} 
		#else
		
		fileAddr = GetFirstLevelDf(G_CurrentDfAddr[G_ChannelNum]);

		if(fileAddr == FALSE)
		{
			return FALSE;
		}

		recNum = GetFileHeadByte(fileAddr,F_LocalPinIndex_offset);
		
		if(recNum != 0)
		{
			keyStatus = G_LocalPinStatus[recNum-1];
		}
		else
		{
			return FALSE;
		}
		
		keyRefense = keyRefense & 0x7F;
		keyRefense -= 0x01; //Local Pin1对应G_LocalPinStatus Bit0

		if(mode == SetStatusMode)	
		{
			SetBit(G_LocalPinStatus[recNum-1],keyRefense);
		}
		else if(mode == ClearStatusMode)
		{
			ClearBit(G_LocalPinStatus[recNum-1],keyRefense);	
		}
		else   //白盒测试需要，代码量不变
		{
			;
		} 
		#endif
	}
	else
	{
		keyStatus = G_AdmStatus;       //ADM1-ADM10

		keyRefense = ((keyRefense&0x80) == 0x80) ? ((keyRefense&0x0F)-0x04) : (keyRefense-0x09);

		if(mode == SetStatusMode)	
		{
			SetBit(G_AdmStatus,keyRefense);
		}
		else if(mode == ClearStatusMode)
		{
			ClearBit(G_AdmStatus,keyRefense);	
		}
		else	//for baihe-test
		{;}
	}
	
	if(mode == GetStatusMode)
	{
		return ValueBit(keyStatus,keyRefense);
	}
	return TRUE;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetKeyRefenseStatus                                                                                                                   
                                                                                                                                                
  Input parameter:   keyRefense
                              
  					                                                                                                         
  Output parameter:  TRUE/FALSE                                                                                                               
                                                                                                                                                
  description:       获取当前KEY的效验状态
  
  status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/  
BYTE  GetKeyRefenseStatus(BYTE keyRefense)
{
	return 	ConfigureKeyStatus(keyRefense,GetStatusMode);
}

/*
*@Description:获取当前操作的操作权限
*
*@param       :fileAddr: 文件地址
*@param       :action  :  操作类型
*
*@returns    :
*
*@history:  create by xyz@100628 14:45
*/
BYTE GetAcessPurview(UWORD fileAddr,BYTE action)
{
	BYTE  idata arrNum,fileLgth,tempPosition;
	UWORD xdata arrFileAddr;
	
	#if !NATIVE_CARD_UICC
		BYTE idata arrName,i;
	UWORD xdata addr;
	WORD idata parName,keyStatus;
	#endif
	
	
	#if NATIVE_CARD_UICC
	  arrFileAddr = SelectLFFile(DF_MF,EF_ARR);
	#else
	arrName = GetFileHeadByte(fileAddr,F_ArrName_Offset);

	arrName =  (((arrName&F_ARRNAME_6F)==F_ARRNAME_6F)?0x6F:0x2F);

	 if((G_CurrentDfName[G_ChannelNum] == DF_MF)||(arrName == 0x2F)) //如果当前df文件是3F00 或 文件中配置的Arr是2F，则权限从3F00/2F06获取
    {
         arrFileAddr = SelectLFFile(DF_MF,EF_ARR);
    }
	  else 
    {
        if(GetFileHeadByte(fileAddr,F_Type_Offset) == DFTYPE)
				{
					addr = GetParentOrMapFileAddr(fileAddr,F_ParentName_Offset);
				}
				else
				{
					addr = G_CurrentDfAddr[G_ChannelNum];
				}					


        while(1)
        {
					 arrFileAddr = SelectFile(addr,0x6F06) ;   //选择当前df下的6F06

					 if(arrFileAddr != FALSE)
					 {
						break;
					 }
            parName = GetFileHeadWord(addr, F_SecondParentName_Offset); 

            addr = GetParentOrMapFileAddr(addr,F_ParentName_Offset);  //如果当前DF下不存在6F06 文件，则继续向上查找父DF的权限文件

						if(parName == DF_MF)	  //DF下的6F06 不存在，且DF的父DF为MF，则选择文件3F00/2F06
            {
									arrFileAddr = SelectLFFile(DF_MF,EF_ARR);  //3F00/2F06  必须创建
                 break ;
            }
        }
    }
	#endif
	
	
	
	if(arrFileAddr == FALSE)            //若文件arr不存在，则默认权限为always
	{
		return 0x90;
	}
	else
	{
		arrNum = GetFileHeadByte(fileAddr, F_ArrNum_Offset);

		fileLgth = GetLFRecNumLgth(arrFileAddr,FileLgth);

		ReadLFBuf(arrFileAddr,arrNum,0,&tempBuf[0],fileLgth);
	}
	
	for(arrNum =0x00; ((arrNum < fileLgth) && (tempBuf[arrNum] != 0xFF)); arrNum = arrNum + 5+ tempPosition)
	{
		if((tempBuf[arrNum]!=0x80)&&(tempBuf[arrNum]!=0x84)&&(tempBuf[arrNum+1]!=0x01))
		{
			return FALSE;
		}

		tempPosition = tempBuf[arrNum+4];
		
		if((((tempBuf[arrNum+2]&action) == action)&&(tempBuf[arrNum] == 0x80)&&(action != A_IncreaseFile))||((tempBuf[arrNum]==0x84)&&((action == A_IncreaseFile)||(action == A_ResizeFile))))								//Find the 
		//if((((tempBuf[arrNum+2]&action) == action)&&(tempBuf[arrNum] == 0x80)&&(action != A_IncreaseFile)&&(action != A_ResizeFile))||((tempBuf[arrNum]==0x84)&&((action == A_IncreaseFile)||(action == A_ResizeFile))&&(tempBuf[arrNum+2]==action)))								//Find the 		
		{			
			switch(tempBuf[arrNum+3])
			{
			 	case 0x90:
				case 0x97:
				{
					return tempBuf[arrNum+3];
				}
				case 0xA4:
				{
					if((tempBuf[arrNum+5]!=0x83)&&(tempBuf[arrNum+6]!=0x01))
					{
						return FALSE;
					}

					if((action == A_IncreaseFile)||(action == A_ResizeFile))
					{
						if(tempBuf[arrNum+2]==action)
						{
							return tempBuf[arrNum +4+3];
						}	
					}
					else
					{
						return tempBuf[arrNum +4+3];	
					}

					break;


				}	
				 case 0x84:                   //Increase命令特定参数
				 {
					//if((tempBuf[arrNum+5]==0x32)&&(tempBuf[arrNum+8])==0x83)
					if((tempBuf[arrNum+5]==A_IncreaseFile)&&(tempBuf[arrNum+8])==0x83)
					{
					    return tempBuf[arrNum+10];
					}
					else 
					{
					    return FALSE;
					}
										
				 }  
				
				default:
				{
				 	return  FALSE;
				}
			}
		}
	}  

	return FALSE;
}








/*
*@description: 比较当前文件的操作权限是否允许执行此操作
*
*@param      :fileaddr:the start address of ef file head
*@param	:action : ef's operation 具体定义见main.h中文件操作的定义
*
*@returns    :TRUE/FALSE
*
*@history   : create by xyz@100628 15:06
*/

BYTE AccessCondition(UWORD fileAddr,BYTE action)
{
	BYTE  idata result,fileType,keyStatus,actionStatus;
#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)
	result = IF_AccessCondition(fileAddr,action);

	if(result != 0xFF)
	{
		return result;
	}
#endif
	result = IF_CreateFileAccessCondition(fileAddr,action);

	if(result == TRUE)
	{
		return TRUE;	
	}
	
	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);
	
	if( fileType != ADFTYPE )
	{
		fileAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);

		if(fileAddr == FALSE)
		{
			return FALSE;
		}
	}

#if UICC_CreatFile_93Tag
	if(action == A_ReadFile)
	{		
		ReadEeprom((BYTE *)&result,fileAddr + F_MapAndRead_Offset,sizeof(BYTE));   //创建文件时,带有93Tag,值为1,则该文件不允许被读,不允许被映射,此处不能调用GetFileHeadByte
	
		if((result&File_MapAndRead) == File_MapAndRead)
		{
			return FALSE;
		}
	}
#endif	
	
	if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)	//RFM权限
	{
		if(fileType == ADFTYPE)
		{
			return TRUE;
		}
		else
		{
				/*
				Bit 0 ：Read/Search    
				Bit 1 ：Update
				Bit 2 ：Delete
				Bit 3 ：Deactivate
				Bit 4 ：Activate
				Bit 5 ：Increase
				Bit 6 ：Resize 权限 ，如果EF文件时通过RFM 创建，则EF文件自动获取了RFM 下的Delete/Resize权限
				*/
				keyStatus = GetFileHeadByte(fileAddr, F_ArrName_Offset);  //modify @20140727，bit7此时表示arrName，在这里bit0-bit6有用
				
				if(action == A_ResizeFile)
				{
					return ((keyStatus&0x40) == 0x40);  //bit6 :表示FRM的resize操作	 		                                  
				}
				else if(action == A_DeleteFile)
				{
					return ((keyStatus&0x04) == 0x04);////bit3 :表示FRM的DELETE操作
				}
				else if(action == A_IncreaseFile)
				{
					return ((keyStatus&0x20) == 0x20);  //bit5 :表示FRM的increase操作	 		                                   
				}
				else
				{
					return ((keyStatus&action) == action);	 //A_ActivateFile = 0x10   	                                    
																									//A_DeactivateFile = 0x08	   			                                    
						                                     //A_UpdateFile	= 0x02 
						                                     //A_ReadFile= 0x01 		
				}
			}	
		
	}
	
	keyStatus = GetAcessPurview(fileAddr,action);
	
	if(keyStatus == FALSE)
	{
		return FALSE;
	}
	
	if(keyStatus == 0x90)
	{
		actionStatus = TRUE;
	}
	else if(keyStatus == 0x97)
	{
		if(action == A_ReadFile)
		{

			if(G_SuperKeyAccess == SuperKeyAccess_PassVerify)	   //验证Super Key @20141110 by xuyz
			{
				actionStatus = TRUE;
			}
			else
			{
				actionStatus = FALSE;
			}

		}
		else
		{
			actionStatus = ValueBit(G_SystemStatus,B_cmk);		 //校验MF key
		}
	}
	else
	{
		actionStatus = GetKeyRefenseStatus(keyStatus);
	}
	
	return actionStatus;
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 FindTag

  input parameter:   dataPointer

  					 tag

  output parameter:  The offset of the tag that is fineded on the tag/FALSE

  description:       Search the tag in the dataPointer, if success return the position of the length,

                     else return FALSE

  Code status:        OK

  test status:        OK

 -------------------------------------------------------------------------------------------------------------------*/

BYTE FindTag(BYTE * dataPointer,BYTE tag,BYTE lgth)
{
	WORD idata i,dataLgth;

	dataLgth = 0x0000 + lgth;

	for(i = 0; i < dataLgth; )
	{
		G_FindCH = 0x00;

		if(dataPointer[i+1] == 0x81)
		{
			G_FindCH = 0x01;
		}

		if((dataPointer[i]&0x7F) == (tag&0x7F))
		{
			return ((i&0xFF)+1+G_FindCH);			
		}
	
		i =i + dataPointer[i+1+G_FindCH]+2+G_FindCH;
	}

	return FALSE;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 OffsetAfterFirstOccurrence                                                                                                                
                                                                                                                                                
  Input parameter:  physicalNo: 记录在文件中的物理记录号

  				 value: 用于判断的值

  				 recLen: 记录的长度           
  					                                                                                                         
  Output parameter: 符合条件的字节其后的位置的偏移量                                                                                       
                                                                                                                                                
  Description:       在一条记录中，找到第一次出现value的位置
  
  Status:		

 -------------------------------------------------------------------------------------------------------------------*/    
#ifndef SingleSIM
BYTE OffsetAfterFirstOccurrence(UWORD fileContentAddr,BYTE physicalNo,BYTE value,BYTE recLen)
{
	BYTE idata pos;
	BYTE idata tmp;

	for(pos=0x00 ; pos<recLen ; )
	{	
#if HEAD_BODY_TOGETHER	
		ReadLFBuf(G_CurrentEfAddr[G_ChannelNum],physicalNo,pos,&tmp,sizeof(BYTE));
#else
		ReadEeprom(&tmp,fileContentAddr+(physicalNo-1)*recLen+pos,sizeof(BYTE));
#endif		
		
		
		pos ++;

		if(tmp == value)
		{
			return pos;
		}	
	}

	return FALSE;
}
#endif
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	SearchStarting

  input parameter:	recordNo:记录号

					maxRecnum:记录数

					step:步长,有符号类型
  					
  output parameter:  TRUE/FALSE

  description:       find a record starting with the given pattern for 2G

  status:			 OK

 -------------------------------------------------------------------------------------------------------------------*/

BYTE  SearchStarting(BYTE recordNo, BYTE maxRecnum, signed step)
{	
	for(; (recordNo > 0) && (recordNo <= maxRecnum) ; recordNo = recordNo+step)
	{
		ReadLFBuf(G_CurrentEfAddr[G_ChannelNum], recordNo, 0, (BYTE * )&RecData[P3],  P3);

		if(memcmp(&RecData[0], &RecData[P3], P3) == EQUAL)
		{
			return recordNo;
		}
	}

	return FALSE;
}


#ifndef SingleSIM
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	SearchContaining                                                                                                             
                                                                                                                                                
  Input parameter:	step:步长,有符号类型
  
  					recordNo:查找起始记录号
					
					offsetValue: 指定的第一次出现的值，只在需要从第一次出现位置的查找模式下有效
					
					searchString:对比的string的起始地址
					
					length:	对比的string的长度
					
					needAfterFirstOccurrence:需要从第一次出现value的偏移处查找

				  					                                                                                                         
  Output parameter:  符合条件的记录的个数                                                                                                           
                                                                                                                                                
  Description:		find a record containing with the given pattern   for 3G  
  
  Status:			 new         

 -------------------------------------------------------------------------------------------------------------------*/ 
BYTE SearchContaining(signed char step,BYTE recordNo,BYTE offsetValue,BYTE * searchString,BYTE length,BYTE needAfterFirstOccurrence)
{
	BYTE idata j,k;
	BYTE idata result;
	BYTE idata recLen;
	BYTE idata offset;
	BYTE idata physicalNo;
	BYTE idata tag;
	BYTE idata validRecNumber,RecIndicator;
	UWORD xdata fileContentAddr;
	
	BYTE xdata bitRecord[0x20];  //最多支持256条记录，实际记录只有255 ，，每个bit表示一个record
	BYTE idata arrayIndex,bitNumber,backRecordNo;
	
	EFileHead xdata fileHead;

	if(G_CurrentEfAddr[G_ChannelNum] == FALSE)
	{
		return FALSE;	
	}	
	
	
	ReadEeprom((BYTE *)&fileHead.FileId,G_CurrentEfAddr[G_ChannelNum],sizeof(EFileHead));
	
	#if HEAD_BODY_TOGETHER
	   RecIndicator = getCycleFPoint(G_CurrentEfAddr[G_ChannelNum]);
	#else
	   fileContentAddr = getFileContentAddr(G_CurrentEfAddr[G_ChannelNum]);		
	
		if((fileHead.FileType&0x07) == CYCTYPE)  
		{
			RecIndicator = getCycleFPoint(G_CurrentEfAddr[G_ChannelNum]);
			
				fileContentAddr++;
		}
	#endif
	
	
	
	validRecNumber =  fileHead.efSize.recInfo[0];

	recLen = GetLFRecNumLgth(G_CurrentEfAddr[G_ChannelNum],FileLgth);	

	memset(tempBuf, 0x00, sizeof(tempBuf));  
	
	backRecordNo = recordNo;
	
	memset(bitRecord, 0x00, sizeof(bitRecord));

	for(result = 0x00; (recordNo > 0) && (recordNo <=  validRecNumber) ; recordNo = recordNo+step)
	{
		if((fileHead.FileType&0x07) == CYCTYPE)
		{
			physicalNo = ((RecIndicator+recordNo-1) >  fileHead.efSize.recInfo[0]?((RecIndicator+recordNo-1)- fileHead.efSize.recInfo[0]):(RecIndicator+recordNo-1));	
		}
		else
		{
			physicalNo = recordNo;
		}

		if(needAfterFirstOccurrence == TRUE)
		{
			#if HEAD_BODY_TOGETHER
			offset = OffsetAfterFirstOccurrence(0,physicalNo,RecData[1],recLen);		   //Search增强模式搜索循环文件bug修改，缺陷编号195
			#else
			offset = OffsetAfterFirstOccurrence(fileContentAddr,physicalNo,RecData[1],recLen);		   //Search增强模式搜索循环文件bug修改，缺陷编号195
			#endif
			
		
			if((offset == 0x00) || ((offset+length-1) > recLen))
			{
				continue;
			}
			//offset = offset - 1;//中国联通6-5基本指令测试的问题，该模式需要从查找到的第一个位置后面一个偏移开始，该段代码一定要注释掉
		}
		else
		{
			offset = offsetValue;
		}
		
		#if HEAD_BODY_TOGETHER
		tag = ReadLFBuf(G_CurrentEfAddr[G_ChannelNum],physicalNo,offset,&tempBuf[0],recLen - offset) ;  //一次性读出记录中的剩余数据

			if(tag == FALSE)
			{
				break;
			}
		#else
			ReadEeprom(&tempBuf[0],fileContentAddr+((physicalNo - 1)*recLen) + offset,recLen - offset);
		#endif
	
		 
		k = 0;
		
		for(j = 0; j < (recLen - offset); j++) 
		{
			if((recLen - offset - j) < length)   //记录的剩余数据长度如果比待比较的数据长度小，则不再继续比较 
			{
				break;
			}
			
			
			if(memcmp(&tempBuf[j], &searchString[0], length) == EQUAL)	
			{
				k = TRUE;
				break;
			} 
			
							 
			
		}
		
			
		if(k == TRUE)
		{
			arrayIndex = (recordNo-1)>>3; //数组的第n个元素	，从0开始, 当objRef/8为双字节时，arrayIndex如果定义为byte型，则会被截断
	
			bitNumber = (recordNo-1)&0x07; //数组第n个元素的第n个bit	 ，从0开始	
			
			SetBit(bitRecord[arrayIndex], bitNumber);
			
			result++;
		} 			
		
  	}	
	
	memset(tempBuf, 0x00, sizeof(tempBuf));  
	
	tag = 0;

	
	for(; (backRecordNo > 0) && (backRecordNo <=  validRecNumber) ; backRecordNo = backRecordNo+step)
	{
		arrayIndex = (backRecordNo-1)>>3; //数组的第n个元素	，从0开始, 当objRef/8为双字节时，arrayIndex如果定义为byte型，则会被截断

		bitNumber = (backRecordNo-1) & 0x07; //数组第n个元素的第n个bit	 ，从0开始	
		
		if(ValueBit(bitRecord[arrayIndex], bitNumber) == TRUE)
		{
			tempBuf[tag ++]  = backRecordNo;
		}
	}
	
	
	if(tag != result)   //该语句原则上不可能进来
	{
		return FALSE;
	}

	return result;
}
#endif


//#ifdef NEWFDN
/*
FDN BDN过程中 对文件激活和失效的子函数

参量 action: FILE_REINVALIDATE/FILE_INVALIDATE	代表激活/失效 文件
*/
void FileLifeCycleHandle(WORD dfName,WORD efName,BYTE action)
{
	//UWORD xdata fileAddr;
	UWORD xdata dfAddr;
	UWORD xdata currentEfHeadAddr;
    BYTE  idata efStatus;	

    dfAddr =  SelectFile(E_FILE_START,dfName);	 	//得到对应的DF地址

	currentEfHeadAddr =  SearchFile(dfAddr,efName);

 	if( currentEfHeadAddr == FALSE)
   	{
    	return ;
	}

	efStatus = GetFileHeadByte(currentEfHeadAddr, F_Status_Offset);

 	if(action == FILE_REINVALIDATE)
	{
	   efStatus |= 0x01;
	}
	else
	{
	   efStatus &= 0xfe;
	}

  SetEepromEX(currentEfHeadAddr + F_Status_Offset,efStatus,sizeof(BYTE));

	return ;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 UsimFdnSYNC                                                                                                                   
                                                                                                                                                
  input parameter:   void                                           
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       
  
  status:			          

 -------------------------------------------------------------------------------------------------------------------*/
void UsimFdnSYNCtoSIM(BYTE fdnStatus)
{
	UWORD xdata fileAddr;
    BYTE  idata fileStatus;

	//USIM同步到SIM，需要先判断USIM下FDN分配
	fileAddr =  SelectTFFile(G_DF_ID_USIM,EF_UST);  //读取3G下服务列表

	if(fileAddr == FALSE)
	{
		return;
	}
	
	ReadTFBuf(fileAddr, 0, (BYTE *)&fileStatus, sizeof(BYTE));	 

	if((fileStatus&0x02)!=0x02)	 //未分配
	{
		return;
	}
	
   	fileAddr = SelectTFFile(DF_PHASE,EF_SST);  //2G EF_SST 读服务状态

	if(fileAddr != FALSE)
	{
		ReadTFBuf(fileAddr, 0, (BYTE *)&fileStatus, sizeof(BYTE));
	}

	if((fileStatus&0x3C) != 0x3C)// ADN  FDN 未同时分配激活	 ,不同步
	{
		return ;
	}

	FileLifeCycleHandle(DF_TELECOM,EF_ADN,(fdnStatus==TRUE)?FILE_INVALIDATE:FILE_REINVALIDATE);	 //失效文件

}
BYTE IsFdnServiceActiveInSIM(void)
{
 	UWORD xdata fileAddr;
	UWORD xdata currentEfHeadAddr;
	UWORD xdata telecomDfAddr;
    BYTE  idata fileStatus,efStatus;

	fileAddr = SelectTFFile(DF_PHASE,EF_SST);  //2G EF_SST 读服务状态

	if(fileAddr == FALSE)
	{
		return FALSE;	
	}
	
	ReadTFBuf(fileAddr, 0, (BYTE *)&fileStatus, sizeof(BYTE));	//读服务状态
	
	if((fileStatus&0x30) != 0x30)	 //fdn is not activated
	{
		return FALSE;
	}
	
	if((fileStatus&0x0C) != 0x0C)	 //adn is not activated
	{
	 	return TRUE;
	}
	
	telecomDfAddr =  SelectFile(E_FILE_START,DF_TELECOM);		//得到DF:3F00/7F10的文件地址

	currentEfHeadAddr =  SearchFile(telecomDfAddr,EF_ADN);
	
	if(currentEfHeadAddr != FALSE)
	{
		 efStatus = GetFileHeadByte(currentEfHeadAddr, F_Status_Offset);
		
		 if((efStatus & 0x01) == FILE_REINVALIDATE )          //ADN文件激活
		 {
		 	return FALSE;	 
		 }
	}
	
	return TRUE;

}

BYTE IsFdnServiceActiveInUSIM(void)
{
	UWORD xdata ustAddr,estAddr;
    BYTE  idata USTStatus,ESTStatus;

	USTStatus = 0x00;
	ESTStatus = 0x00;

	ustAddr =  SelectTFFile(G_DF_ID_USIM,EF_UST);  //读取3G下服务列表

	if(ustAddr!= FALSE)
	{
		ReadTFBuf(ustAddr, 0, (BYTE *)&USTStatus, sizeof(BYTE));	 

		if((USTStatus&0x02)!=0x02)	 //未分配
		{
			return FALSE;
		}
		estAddr =  SelectTFFile(G_DF_ID_USIM,EF_EST);	   // 3G 下FDN服务状态

		if(estAddr != FALSE) 
		{
			ReadTFBuf(estAddr, 0, (BYTE *)&ESTStatus, sizeof(BYTE));

			if((ESTStatus&0x01)==0x01)
			{
				return TRUE;
			}			
		}		
	}

	return FALSE;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SimFdnSYNC                                                                                                                   
                                                                                                                                                
  input parameter:   void                                           
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       
  
  status:			          

 -------------------------------------------------------------------------------------------------------------------*/
 void SimFdnSYNCtoUSIM(BYTE fdnStatus)
 {
 	UWORD xdata fileAddr,estAddr;
    BYTE  idata fileStatus;

	//SIM同步到USIM，需要先判断SIM下FDN分配并激活
	fileAddr = SelectTFFile(DF_PHASE,EF_SST);  //2G EF_SST 读服务状态

	if(fileAddr== FALSE)
	{
		return;
	}

	ReadTFBuf(fileAddr, 0, (BYTE *)&fileStatus, sizeof(BYTE));	//读服务状态
	
	if((fileStatus&0x30) != 0x30)	 //fdn is not activated
	{	
		return ;
	}
	
	fileAddr = SelectTFFile(G_DF_ID_USIM,EF_SST);  //3G EF_SST 读服务状态

	if(fileAddr != FALSE)
	{
		ReadTFBuf(fileAddr, 0, (BYTE *)&fileStatus, sizeof(BYTE));	//读USIM下FDN服务是否分配

		if((fileStatus&0x02) == 0x02)                       //USIM下FDN分配
		{
		    estAddr =  SelectTFFile(G_DF_ID_USIM,EF_EST);	   

			if(estAddr!= FALSE)
			{
				ReadTFBuf(estAddr, 0, (BYTE *)&fileStatus, sizeof(BYTE));	 // 读USIM下FDN服务状态	 ，根据SIM下FDN服务状态修改USIM下相应的FDN服务状态值

				if(fdnStatus == FALSE)
				{
					fileStatus &= 0xfe;
				}
				else
				{
					fileStatus |= 0x01;
				}

				WriteTFBuf(estAddr, 0, (BYTE *)&fileStatus, sizeof(BYTE));
			}
		}
	}
 
 }
//#endif


/*

*/
BYTE fdnInvalidate(WORD dfId,WORD efID)
{
	UWORD xdata currentEfHeadAddr;
	BYTE idata efStatus;
	UWORD xdata dfAddr;

	if(dfId == DF_CDMA)
	{
	   dfAddr = SelectFile(E_FILE_START,DF_CDMA);	
	}
	else
	{
	   dfAddr = SelectFile(E_FILE_START,DF_PHASE);
	}	

	if(dfAddr == FALSE)
	{
		return FALSE;
	}

	currentEfHeadAddr =  SearchFile(dfAddr,efID);

 	if( currentEfHeadAddr == FALSE)
   	{
    	return FALSE;
	}
	
	/*
	efFileAddr = SelectTFFile(dfId, efID);
	
	if(efFileAddr ==FALSE)
	{
		return FALSE;
	}
	*/
	
    ReadEeprom((BYTE  *)&efStatus,currentEfHeadAddr + F_Status_Offset,sizeof(BYTE));

    SetEepromEX(currentEfHeadAddr + F_Status_Offset,efStatus&0xFE,sizeof(BYTE));
    
    return TRUE;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 FdnProgcess                                                                                                                   
                                                                                                                                                
  input parameter:   void                                           
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       set the ef information to the gsm buffer
  
  status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/    
void FdnProgcess(BYTE mode)
{
	 UWORD xdata efFileAddr;
     BYTE  idata efStatus;
	 UWORD xdata currentEfHeadAddr; 
	 UWORD xdata telecomDfAddr;

	 if(mode ==CDMAFDN)
	 {
		efFileAddr = SelectTFFile(DF_CDMA, EF_CDMASST);
	 }
	 else
	 {
		efFileAddr = SelectTFFile(0x7F20,0x6F38);
	 }
	 
	 ReadTFBuf(efFileAddr, 0x00, (BYTE  *)&efStatus, sizeof(BYTE));

	 if((efStatus & 0x30) != 0x30)//fdn active and allowed
    {
         return;
	}

	telecomDfAddr = SelectFile(E_FILE_START,DF_TELECOM);		//得到3F00/7F10的文件地址

 	if((efStatus & 0x0C) == 0x0C)//adn active and allowed
    {
 		currentEfHeadAddr =  SearchFile(telecomDfAddr,EF_ADN);

        if(currentEfHeadAddr == FALSE)
		{
        	return;
		}
		
		efStatus = GetFileHeadByte(currentEfHeadAddr, F_Status_Offset);

    	if((efStatus & 0x01) == FILE_REINVALIDATE )
		{
     		return;
		}
		
		/*
 		ReadEeprom((BYTE  *)&efStatus, efFileAddr+F_Status_Offset, sizeof(BYTE));

    	if((efStatus & 0x01) == FILE_REINVALIDATE )
		{
     		return;
		}
		*/
 	}

	if(mode == CDMAFDN)
	{
		fdnInvalidate(DF_CDMA,EF_IMSIM);
		fdnInvalidate(DF_CDMA,EF_IMSIT);
		fdnInvalidate(DF_CDMA,EF_TMSI);
	}
	else
	{
		fdnInvalidate(0x7F20,0x6F07);
		fdnInvalidate(0x7F20,0x6F7E);
	} 


	
/*	
	UWORD xdata fileAddr;
	UWORD xdata currentEfHeadAddr; 
	UWORD xdata telecomDfAddr,phaseDfAddr;
    BYTE  idata efStatus;

	fileAddr = SelectTFFile(DF_PHASE,0x6F38);

    if(fileAddr == FALSE)
	{
        return;
	}
	
 	ReadTFBuf(fileAddr,0,(BYTE *)&efStatus,sizeof(BYTE));

    if((efStatus & 0x30) != 0x30)//fdn active and allowed
	{
         return;
	}

	telecomDfAddr = SelectFile(E_FILE_START,DF_TELECOM);		//得到3F00/7F10的文件地址

 	if((efStatus & 0x0C) == 0x0C)//adn active and allowed
    { 
   		
		currentEfHeadAddr =  SearchFile(telecomDfAddr,EF_ADN);

        if(currentEfHeadAddr == FALSE)
		{
        	return;
		}

		efStatus = GetFileHeadByte(currentEfHeadAddr, F_Status_Offset);

    	if((efStatus & 0x01) == FILE_REINVALIDATE )
		{
     		return;
		}
 	}

	phaseDfAddr = SelectFile(E_FILE_START,DF_PHASE);		//得到3F00/7F20的文件地址

	currentEfHeadAddr =  SearchFile(phaseDfAddr,EF_IMSI);

 	if( currentEfHeadAddr == FALSE)
   	{
    	return;
	}
 
	efStatus = GetFileHeadByte(currentEfHeadAddr, F_Status_Offset);
 
    AccessEepromEX(currentEfHeadAddr + F_Status_Offset,NULL,(efStatus&0xFE),sizeof(BYTE),(EEP_Set)|(EEP_NoSec));

	currentEfHeadAddr =  SearchFile(phaseDfAddr,0x6F7E);

 	if( currentEfHeadAddr == FALSE)
	{
 		return;
	}

	efStatus = GetFileHeadByte(currentEfHeadAddr, F_Status_Offset);

 	SetEepromEX(currentEfHeadAddr + F_Status_Offset,efStatus & 0xFE,sizeof(BYTE));
	*/
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 BackFilePath                                                      
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       备份文件路径信息
  
  status:			 修改为只记录文件ID，不记录地址 
 -------------------------------------------------------------------------------------------------------------------*/   

void BackFilePath(PathInfo * ptrToFilePath)
{
	ReadEeprom((BYTE *)&(ptrToFilePath->fileId[0]),G_CurrentDfAddr[G_ChannelNum]+F_SecondParentName_Offset,sizeof(WORD));	//Back 2nd ParentName

  	ptrToFilePath->fileId[1] = G_CurrentDfName[G_ChannelNum];

  	ptrToFilePath->fileId[2] = G_CurrentEfName[G_ChannelNum];

  	if(ptrToFilePath->fileId[2] != FALSE)
	{
		ptrToFilePath->recPointer = G_CurrentRecNum[G_ChannelNum];
		
		ptrToFilePath->sfi = G_CurrentSfi;		
	}
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ResumeFilePath                                                      
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       恢复文件路径信息
  
  status:			 修改为只记录文件ID，不记录地址 
 -------------------------------------------------------------------------------------------------------------------*/   

BYTE ResumeFilePath(PathInfo * ptrToFilePath)
{
	UWORD xdata fileAddr;
	BYTE  idata i;

	fileAddr = E_FILE_START;
	
	for(i=0;(ptrToFilePath->fileId[i] != FALSE)&&(i < 0x03); i++)
	{
		fileAddr = SelectFile(fileAddr,ptrToFilePath->fileId[i]);									 //not inclue the file head 

	   	if(fileAddr == FALSE)
		{
			return FALSE;
		}

		SetChannel(fileAddr,ptrToFilePath->fileId[i],FALSE);
	}
	
	if(i == 0x03)
	{
		G_CurrentRecNum[G_ChannelNum] = ptrToFilePath->recPointer;
		
		G_CurrentSfi = ptrToFilePath->sfi;			
	}
	
	return TRUE;	
}

/*
*@description:通过2F00中的记录号，获得某AID的DF name ，DF length
*
*@param      :BYTE recordNum: 获取2F00里指定记录的的AID,及AID,length
*@param      :BYTE *aidBuf    : 存放获取的AID name LV  aidBuf[0]-AIDName length	V-AIDName
*
*@returns    :TRUE/FALSE
*
*@history    :create by xyz@100510 17:54
*/
void GetAIDNameInFile(BYTE recordNum,BYTE *aidBuf)
{
	BYTE idata tmp;
	UWORD xdata fileAddr;

	fileAddr = SelectLFFile(DF_MF,0x2F00);

	if(fileAddr == FALSE)
	{
		 return;
	}

	ReadLFBuf(fileAddr,recordNum,0x00,&tmp,sizeof(BYTE));
	
	if(tmp != 0x61)
	{
		aidBuf[0] = FALSE;
		
		return;
	}
	ReadLFBuf(fileAddr,recordNum,0x02,&tmp,sizeof(BYTE));
	
	if(tmp != 0x4F)
	{
		aidBuf[0] = FALSE;
		
		return;
	}

	ReadLFBuf(fileAddr,recordNum,0x03,&aidBuf[0],sizeof(BYTE));

	if(aidBuf[0] > 0x10)	   //aid长度无效 
	{
		aidBuf[0] = FALSE;
		
		return;
	}

	ReadLFBuf(fileAddr,recordNum,0x04,&aidBuf[1],aidBuf[0]);
}
/*
*@description : 通过AID name 查找在文件2F00中是否存在记录与之相等 ,返回记录号
*
*@param       :BYTE  *aidBuf LV
*
*@returns     :记录号 /FALSE
*
*@history     :create by xyz @100510 17:54
*/
BYTE GetAIDNumber(BYTE *aidBuf)
{
	BYTE idata 	recNum,num;
	UWORD xdata fileAddr;
	BYTE xdata localAID[0x15];

	fileAddr = SelectLFFile(DF_MF,0x2F00);

	if(fileAddr == FALSE)
	{
		 return FALSE;
	}
	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	for(num=0x00;num<recNum;num++)
	{
		ReadLFBuf(fileAddr,num+1,0x02,&localAID[0],0x12);  //tempBuf[0]- 0x4F,tempBuf[1]-L tempBuf[2]-tempBuf[0x12] : V

		if(localAID[0] != 0x4F)
		{
			continue;
		}

		if(localAID[1] == aidBuf[0])
		{
			if(memcmp(&localAID[2],&aidBuf[1],localAID[1]) == 0x00)
			{
				return (num+1);
			}
		}
	}
}

/*
*@description:删除2F00文件中特定记录
*
*@param      :recNum:记录号
*
*@return     :void
*
*@history    :create by xyz@100513 8:59
*/
void DeleteDIRNum(BYTE recNum)
{
	UWORD xdata fileAddr;

	 fileAddr = SelectLFFile(DF_MF,0x2F00);

	if(fileAddr == FALSE)
	{
		 return ;
	}

	SetLFConst(fileAddr,recNum,0x00,0xFF,GetLFRecNumLgth(fileAddr,FileLgth));
}

/*
*@description : 通过AID name 查找在文件2F00中是否存在记录与之部分bufen相等 ,返回记录号
*
*@param       :BYTE  *PartialAidBuf
*@param       :BYTE mode:  full_match/partitial_match
*
*@returns     :记录号 /FALSE
*
*@history     :create by xyz @100510 17:54
 *            
*/
BYTE GetAIDNumberForMode(BYTE *aidBuf, BYTE mode)
{
	BYTE idata 	recNum,num;
	BYTE xdata aidCmpBuffer[0x12];
	UWORD xdata fileAddr;

	fileAddr = SelectFile(E_FILE_START,0x2F00);

	if(fileAddr == FALSE)
	{
		 return FALSE;
	}
	
	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	for(num=0x00;num<recNum;num++)
	{
		ReadLFBuf(fileAddr,num+1,0x02,&aidCmpBuffer[0],0x12);  //tempBuf[0]- 0x4F,tempBuf[1]-L tempBuf[2]-tempBuf[0x12] : V

		if(aidCmpBuffer[0] != 0x4F)
		{
			continue;
		}

		if(mode == AID_FullMatch)
		{
			if( aidCmpBuffer[1] != aidBuf[0] )  //全匹配
			{
				continue;
			}
		}
		else
		{
			if( aidCmpBuffer[1] < aidBuf[0] )  //部分匹配
			{
				continue;
			}
		}
		
		if(memcmp(&aidCmpBuffer[2],&aidBuf[1],aidBuf[0]) == EQUAL)
		{
			return (num+1);
		}
		
	}	 

	return FALSE;	  
}


/*
*@function   : GetADFFID
*
*@description:  获取ADF 的FID信息
*
*@param       :BYTE *rid 5字节
*@param       :BYTE *3gppcode: 2字节
*
*@return      FID
*
*@hisotry    :V1.0 :create by@20171020
*/
WORD GetADFFID(BYTE *PartialAidBuf)
{
	UWORD xdata fileAddr;
	UWORD xdata fileUseSpace,fileSpaceAddr;
	BYTE  idata fileType;
	WORD idata adfFid;
	BYTE idata dirNum,currentDirNum;
	
	dirNum = GetAIDNumberByPartialAID(PartialAidBuf);
	
	if(dirNum == FALSE)  //未找到部分匹配的AID
	{
		return FALSE;
	}
	
	fileUseSpace = GetFileHeadUsedSize(E_FILE_START);
	
  fileUseSpace = fileUseSpace + E_FILE_START;

	fileAddr = E_FILE_START;
	
  for(fileAddr = fileAddr+sizeof(FileSystemHead);fileAddr < fileUseSpace;fileAddr = fileAddr + fileSpaceAddr)
  {   
		fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

		if(fileType == ADFTYPE)
		{
			currentDirNum = GetFileHeadByte(fileAddr,F_DIRNum_offset);
						
			if(currentDirNum == dirNum)  //返回查找到的ADF 的FID
			{
				adfFid = GetFileHeadWord(fileAddr,F_Name_Offset);
				
				return adfFid;
			}
			else
			{
				fileSpaceAddr = GetFileHeadUsedSize(fileAddr);
			}			
		}
		else if((fileType == TRANTYPE) || (fileType == LINTYPE)||(fileType == CYCTYPE))
		{
			fileSpaceAddr = sizeof(EFileHead) ;	//fileSpaceAddr as Space		
		}

  }
	
	return FALSE;
}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetBerTlvLength
  
  input parameter:   void
   
  output parameter:

  description:
  					 
  
  status:			     

 -------------------------------------------------------------------------------------------------------------------*/    
UWORD GetBerTlvLength(BYTE * buffer,WORD * lenByte)
{
		WORD  idata length;
		BYTE  idata off;
	
		if(buffer[0] > 0x83)//HB_Offline_Test/5KeyEstablish指令异常测试/Tele_TC9_ISDR_KeyEstablishment_ERROR_01/步骤10、ISD-R,EstablishISDRKeySet cmd1 cert len error
		{
			//ThrowError(0x6A80);
			length = buffer[0];		//当传入数据是非BERTLV时，这样复制更具有兼容性
				
			off = 2;
		}
		else if(buffer[0] == 0x83)
		{
			length = (((buffer[1]<<16)&0xFF0000)|((buffer[2]<<8)&0xFF00)|buffer[3]);
			off = 5;
		}
		else if(buffer[0] == 0x82)
		{
			length =  ((((WORD)(buffer[1]) << 8) & 0xFF00) | ((WORD)(buffer[2]) & 0x00FF));
				
			off = 4;
		}
		else if(buffer[0] == 0x81)
		{
			length = buffer[1];
				
			off = 3;
		}
		else
		{
			length = buffer[0];
				
			off = 2;
		}
		
		if(lenByte != NULL)
		{
			*lenByte += off;
		}
		
		return length;
}



/*
*@function     :ASN1_GetBERTlvLen
*
*@description  :获取BER编码规则的TLV中，L域占用字节数，以及L的值即value占用的字节数
*
*@see          :
*
*@param        :UWORD srcAddr -- 输入参数，指向 BER TLV编码Len的起始即Tag后第一字节，可能是 xx/81 xx /82 xxxx / 83 xxxxxx
*@param        :UWORD* len    -- 输出参数，BER TLV结构中的Len值
*
*@return       :BYTE          -- 返回L域占用的字节数
*
*@RAM Resources:
*
*@hisotry      :V1.0 created by lgy@20181113
*              :V1.1 modify by xuxiaojun@20190326  eUICC test RSP 20190301\TY_Offline_RSP_ManageCmdTest \ Download_260K_size_profile 要求下载260K的profile，传入的长度为83 029696
*/
BYTE GetBERTlvLen(UWORD srcAddr,UWORD* len)
{
	BYTE xdata temp[4];
	WORD idata lenNum;

	lenNum = 0x00;

	VMReadBuf(&temp[0],srcAddr,4); //len最大可能为4字节  83 xx xx xx

	*len = GetBerTlvLength(temp,&lenNum);

	return (lenNum-1);
}

#if UICC__FUNCTION__RFM
/*
*@function     :keyData_getRFMKey
*
*@description  :获取指定的keyVersion和keyID的key值
*
*@see          :
*
*@param        :BYTE keyversion 	-- 输入参数，指定的keyVersion， 支持01-0f， 40-4f
*@param        :BYTE keyID    		-- 输入参数，指定的keyID，		支持kic，kid
*@param        :BYTE *keyValueBuf	-- 输出参数，keyValue
*
*@return       :BYTE          -- 返回key的长度，不存在该key时返回FALSE
*
*@hisotry      :V1.0 created by jiangqingbin@20220317
*/			
//key存储格式：(L TLVTLV...)	0036 01 22 10 D9BE655005BEB775324D53DEA699F13A 10 52DABF691AFD0E2B540CA6A09DEE3BF9 40 11 10 887F658FD4BAD493887F658FD4BAD493 
BYTE keyData_getRFMKey(BYTE wantkeyVersion, BYTE keyID, BYTE * keyValueBuf)
{
	//在nvm地址 RFM_KEY_ADDR 中查找指定key
	WORD xdata allKeyLen;
	WORD xdata offSet;
	BYTE xdata keyLen, subKeyLen;
	BYTE xdata keyVersion;
	
	keyLen = 0;
	subKeyLen = 0;
	
	ReadEeprom((BYTE *)&allKeyLen, RFM_KEY_ADDR, 2);			
		
	offSet = 2;
	
	while (offSet < allKeyLen)
	{
		keyVersion = VMReadByte(RFM_KEY_ADDR + offSet);
		
		offSet ++;		//point to keyLen
		
		keyLen = VMReadByte(RFM_KEY_ADDR + offSet);
		
		offSet ++;		//point to first key keylen
		
		if (keyVersion == wantkeyVersion)
		{
			subKeyLen = VMReadByte(RFM_KEY_ADDR+offSet);
			
			offSet ++;	//point to first key keyValue
			
			if (keyID == MODE_KIC)		//KIC
			{		
				if (keyValueBuf != NULL)	//检查scp81key的psk是否存在是会传入NULL
				{
					ReadEeprom(keyValueBuf, RFM_KEY_ADDR+offSet, subKeyLen);	
				}
			}
			else if (keyID == MODE_KID)//KID
			{
				if (keyLen == (subKeyLen + 1))		//该版本的key值存在一支key， 默认是KIC, 即不存在KID
				{
					subKeyLen = 0;
				}
				
				offSet += subKeyLen;	//point to second key keylen
				
				subKeyLen = VMReadByte(RFM_KEY_ADDR+offSet);
				
				offSet ++;				//point to second key keyValue
				
				if (keyValueBuf != NULL)
				{
					ReadEeprom(keyValueBuf, RFM_KEY_ADDR+offSet, subKeyLen);
				}					
			}
			else	//不支持其他keyID
			{
				subKeyLen = 0;
			}
			
			return subKeyLen;
		}
		
		offSet += keyLen;		//point to next keyVersion
	}
	
	return 0;
}
#endif

#if UICC__FUNCTION__RFM
/*
*@function     :putRFMKey
*
*@description  :写入RFMKey， 目前仅支持scp80 和 scp81 key
*
*@see          :
*
*@param        : void
*
*@hisotry      :V1.0 created by jiangqingbin@20220318
*/
//key store format:	0036 01 22 10 D9BE655005BEB775324D53DEA699F13A 10 52DABF691AFD0E2B540CA6A09DEE3BF9 40 11 10 887F658FD4BAD493887F658FD4BAD493 
//putkey format:	C0EA04002D  01 22 10 D9BE655005BEB775324D53DEA699F13A 10 52DABF691AFD0E2B540CA6A09DEE3BF9 (9000)
void putRFMKey(void)
{
	WORD xdata allKeyLen;
	WORD xdata offSet;
	BYTE xdata keyLen;
	BYTE xdata keyVersion;
	
	if (P3 == 0x00)		//清空所有key
	{
		SetEeprom(RFM_KEY_ADDR, 0x00, 2); 
		
		return;
	}

	keyLen = 0;
	offSet = 0;
	
	ReadEeprom((BYTE *)&allKeyLen, RFM_KEY_ADDR, 2);

	offSet += 2;
	
	while (offSet < allKeyLen)
	{
		keyVersion = VMReadByte(RFM_KEY_ADDR + offSet);
		
		offSet ++;		//point to keyLen
		
		keyLen = VMReadByte(RFM_KEY_ADDR + offSet);
		
		offSet ++;		//point to first key keyValue
		
		if (keyVersion == RecData[0])		//替换原有的keyVerion的key, 替换和被替换的key总长度必须一样
		{
			if (keyLen != RecData[1])	
			{
				IncorrentParameter();
					
				return;
			}
					
			WriteEeprom(RFM_KEY_ADDR+offSet, &RecData[2], P3-2);
			
			return;
		}
			
		offSet += keyLen;		//point to next keyVersion	
	}
	
	//以下是新加key
	
	if ((allKeyLen+P3) > (RFM_KEY_ALL_LEN-2))		//空间不足
	{
		NoEnoughSpace();
					
		return;
	}
	
	allKeyLen += P3;
	
	WriteEeprom(RFM_KEY_ADDR, (BYTE *)&allKeyLen, 2);	
	
	WriteEeprom(RFM_KEY_ADDR+offSet, &RecData[0], P3);	
}
#endif
/*
*@description: FindTagFullMatch用于查找完整的tag,比如00/80表示不同的含义等tag
*
  input parameter:   dataPointer

  					 tag

  output parameter:  The offset of the tag that is fineded on the tag/FALSE

  description:       Search the tag in the dataPointer, if success return the position of the length,

                     else return FALSE
*
*@history   :LCZ 20201123
*/
BYTE FindTagFullMatch(BYTE * dataPointer,BYTE tag,BYTE lgth)
{
	BYTE idata ch;
	WORD idata i,dataLgth;

	dataLgth = 0x0000 + lgth;

	for(i = 0; i < dataLgth; )
	{
		ch = 0x00;

		if(dataPointer[i+1] == 0x81)
		{
			ch = 0x01;
		}

		if(dataPointer[i] == tag)
		{
			return ((i&0xFF)+1+ch);			
		}
	
		i =i + dataPointer[i+1+ch]+2+ch;
	}

	return FALSE;
}

BYTE GetAssoiatedSDByAppId(BYTE appId)
{
	return ISD_APPID;
}
/*@Function:   ReadLFFileValue                                            
 * 
 *@Descrip:    从LF文件读取内容
 *
 *@See:	       
 *                                                                                                                                     
 *@Param:      fileAddr      
 *              						                                                                                                  
 *@Return:     BYTE                                                                                                                                                                                                                                                              
 * 
 *@Author:     xwh
 *  
 *@History:    V0.0---Create by xwh 20180801
 *                        
 */
BYTE ReadLFFileValue1ByteLen(UWORD fileAddr,BYTE index,BYTE offset,BYTE * buf)
{
	BYTE idata parameterLen;

	ReadLFBuf(fileAddr, index, offset, (BYTE*)&parameterLen, 1); //取Host的长度
			
	ReadLFBuf(fileAddr, index, offset+1, &buf[0], parameterLen); //从文件读取Host的值,长度大于0xFF的暂时不做处理

	return parameterLen;
}

/*@Function:    SetBerTlvLength                                           
 * 
 *@Descrip:     根据SPI KIC KID 的值获取CC 长度
 *
 *@See:	        ts_143019v050600p.pdf,TS 23.048
 *                                                                                                                                     
 *@Param:        spi0  --- SPI第一字节，与msl进行比较
 *@param        :*buf: Minimum SPI1
 *				                                                                                                         
 *@Return:      TRUE(检查通过)/FALSE（检查不通过）                                                                                                                                                                                                                                                                
 * 
 *@Author:      XWH 20131018
 *  
 *@History:     V0.0---Create by XWH 20131018             
 */ 
BYTE SetBerTlvLength(BYTE * buf,WORD totalLen)
{
	BYTE  idata offset;

	offset = 0;

	if(totalLen > 0xFF)
	{
		buf[offset++] = 0x82;

		buf[offset++] =  (totalLen>>8)&0xFF;

		buf[offset++]  = totalLen&0xFF;
	}
	else if(totalLen > 0x7F)
	{
		buf[offset++] = 0x81;

		buf[offset++] = totalLen&0xFF;
	}
	else
	{
		buf[offset++] = totalLen&0xFF;           //指向数据体
	}

	return offset;
}
/*
*@description: 计算当前CPL长度,兼容固定为2字节和BERTLV的情况
*
@param       :CPL1
*
@return      :cplLength	 :支持81 82,若CPL固定为2字节,则暂时支持0001-0EFF数据长度的编码,否则考虑当前CPL为BER-tlv编码方式
*
*@author      :create by xuyz
*
*@history   :v1.0 create by xuyz
*/
BYTE ToolkitGetCPLLength(BYTE CPL1)
{
	if(CPL1 < 0x0F)	
	{
		return 0x02;
	}
	else if(CPL1 <0x80)
	{
		return 0x01;
	}
	else 
	{
		return ((CPL1&0x0F)+1);
	}
}