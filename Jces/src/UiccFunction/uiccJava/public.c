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
			V1.10定义宏BERTLVCmd，将BER-TLV结构的处理放入该宏内 --20111027 by chenl
			V1.11 修改函数AccessCondition，对于RFM 操作而言，通过判断ArrName的bit0-bit4 来判断对应的RFM 权限（read update write deactive active）@20140727 by xuyz

      **************************************for UICC APDU*******************************************************
 			V1.12 修改函数GetAcessPurview，如果DF下存在6F06 ，则优先选择6F06 
				  修改函数SearchContaining，为兼容UICC API测试，searchString的值采取读E2的方式
				  修改函数GetAIDNumber，该函数中不要再使用 tempBUf做缓冲@20151201 by xuyz

			V1.13 修改函数GetAcessPurview，为兼容RFM权限配置，此处需要对ArrName进行转换@20151202 by xuyz

 */
#include  "typedef.h"
#include  "uicc_main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
#include  "gsmpublic.h"
#include  "uicc_interface.h"
#include  "stkhead.h"
#include  "T0ApduImpl.h"
#if UICC__FUNCTION__STKAPI
#include "ToolkitAPIPublic.h"
#endif
#include "profileApi.h"
#include "ProfileInterpreter.h"
#include "cardManager.h"
#include "Interface_LTEFromPlatform.h"
#include "Boot.h"
#include "BootLifeCycle.h"
#include "euiccConfig.h"	//remove warning,dengyiwen,2022/9/21
#include "CompactManage.h"	//remove warning,dengyiwen,2022/9/21
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
#if SE_ONLY
	BYTE idata result;
#endif
	ClearBit(G_SystemStatus,B_needCopy);

	if(ValueBit(G_SystemStatus,B_RfmPerform) == FALSE)
	{	
		GlobalReceiveData();  //接收数据后G_CommandStatus = DataReceived;
	}


#if SE_ONLY
	result = IF_ReceiveData();

	return result;
#else
	return 0;

#endif


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
		SmartLumingRead();	//优选漫游
#endif	
		
#endif		
	}	
	
	SetBit(G_SystemStatus,B_needCopy);

	if(ValueBit(G_SystemStatus,B_RfmPerform) == FALSE)
	{
		GlobalSendData();
	}
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
	BYTE  idata fileType/*,recNumberLgth[2],fileStatus*/;  //remove warning,dengyiwen,2022/9/22
   	UWORD xdata fileUseSpace,fileSpaceAddr;

	//在没有创建任何文件前，E_FILE_START 的值是0xFFFFFFFF，此时直接退出 
	if((fileAddr == INVALID_ADDR)||(fileAddr == FALSE))
	{
		return FALSE;
	}

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
			fileSpaceAddr = GetFileHeadUsedSize( fileAddr);	
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
//		else if(fileType == TRANTYPE)
//		{
//			tempParameter = GetFileHeadWord(fileAddr, F_SpaceSize_Offset);		//tempParameter as SpaceSize

//		 	fileSpaceAddr = tempParameter+sizeof(TranFileHead);
//		}
//		else if((fileType == LINTYPE)||(fileType == CYCTYPE))
//		{
//			ReadEeprom((BYTE  *)&recNumberLgth[0],fileAddr+F_RecNum_Offset,sizeof(recNumberLgth));
//		
//		        fileStatus = GetFileHeadByte(fileAddr,F_Status_Offset);
//				
//				fileSpaceAddr = (UWORD)(recNumberLgth[0]*recNumberLgth[1])+sizeof(RecordFileHead) ;	//fileSpaceAddr as Space
//				
//				if((fileType == CYCTYPE) && ((fileStatus & F_MAP_FILE) != F_MAP_FILE))//循环文件且不是映射文件，才需要+1
//				{
//					fileSpaceAddr ++;
//				}
//		}
		else if((fileType == TRANTYPE)||(fileType == LINTYPE)||(fileType == CYCTYPE))
		{
			fileSpaceAddr = GetFileTotalSize(fileAddr);
		}
		else
		{
			return FALSE;
		}
   	}

	return FALSE;
}

#if UICC__FUNCTION__DFLink

UWORD SelctLinkDF(UWORD fileAddr)
{
	BYTE idata i;
	WORD idata fileName;
	BYTE xdata dfLinkPath[0x08];

	if(UICC__FUNCTION__DFLink__Cfg)
	{
		ReadEEPromCache(dfLinkPath,fileAddr + F_KEY_Offset,0x08);
		
		if((dfLinkPath[0] == 0xFF)&&(dfLinkPath[1] == 0xFF))
		{
			return fileAddr;
		}
		else
		{
			fileAddr = E_FILE_START;
			
			for(i=0x00;i<8;i=i+2)
			{
				 fileName = (((WORD)dfLinkPath[i]<<8)&0xFF00) + dfLinkPath[i+1];

				 if(fileName == 0xFFFF)
				 {
					 break;
				 }

				 fileAddr = SelectFile(fileAddr,fileName);
			 }

			 return fileAddr;
		}
	}
	else
	{

		return fileAddr;

	}

}
#endif

#if (EUICC_ONLY||SE_AND_EUICC)
BYTE GetMaxLocalPinIndex(void)
{
	    WORD  idata tempParameter;
		BYTE  idata fileType/*,recNumberLgth[2]*/,maxLocalPinIndex,localPinIndex;
	   	UWORD xdata fileUseSpace,fileSpaceAddr,fileAddr;
        
		// modify by zhangxiaoxiao@20221204:			
    	//1、创建3F00时，文件头的LocalPinIndex赋初值1
        //2、ReadPinStatus时，遍历localpin，需要遍历文件系统的整个DF，判断LocalPinIndex是否为0，localpin存在于DF/ADF下面
	   	maxLocalPinIndex = 1;

	   	fileAddr = E_FILE_START;

		fileUseSpace = GetFileHeadUsedSize(fileAddr);

	    fileUseSpace = fileUseSpace + fileAddr;

	   	for(fileAddr = fileAddr+sizeof(FileSystemHead);fileAddr < fileUseSpace;fileAddr = fileAddr + fileSpaceAddr)
	   	{

			fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

			if(fileType == ADFTYPE)
			{
				fileSpaceAddr = GetFileHeadUsedSize( fileAddr);

				localPinIndex = GetFileHeadByte(fileAddr,F_LocalPinIndex_offset);

				if(localPinIndex > maxLocalPinIndex)
				{
					maxLocalPinIndex = localPinIndex;
				}
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
//			else if(fileType == TRANTYPE)
//			{
//				tempParameter = GetFileHeadWord(fileAddr, F_SpaceSize_Offset);		//tempParameter as SpaceSize

//			 	fileSpaceAddr = tempParameter+sizeof(TranFileHead);
//			}
//			else if((fileType == LINTYPE)||(fileType == CYCTYPE))
//			{
//				ReadEeprom((BYTE  *)&recNumberLgth[0],fileAddr+F_RecNum_Offset,sizeof(recNumberLgth));

//				fileSpaceAddr = (UWORD)(recNumberLgth[0]*recNumberLgth[1])+sizeof(RecordFileHead) ;	//fileSpaceAddr as Space
//			}
			else if((fileType == TRANTYPE)||(fileType == LINTYPE)||(fileType == CYCTYPE))
			{
				fileSpaceAddr = GetFileTotalSize(fileAddr);
			}
			else
			{
				return FALSE;
			}
	   	}


		return (maxLocalPinIndex+1);
}
#endif
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

#if SE_ONLY
	if(fileName == 0x7F21)
	{
		fileName = 0x7F20;
	}
#else
	if(fileName == 0x7F21)// modify by chentao@20221025 尼泊尔profile中USIM ADF的FID是7F21,之前的处理是选择7F21直接选择7F20，但是KI私有文件在7F21目录下，在7F20下没有导致鉴权失败，
  {
		 if(SearchFile(fileAddr,fileName)== FALSE)  //modify by chentao@20221025现修改为现在7F21失败的情况下在选择7F20
		 {
			 fileName = 0x7F20;
		 }
  }
#endif


	fileAddr = SearchFile(fileAddr,fileName);

	if(fileAddr == FALSE)
	{
	 	return FALSE;
	}

	fileType  = GetFileHeadByte(fileAddr,F_Type_Offset);

	if(fileType == ADFTYPE)
	{
		#if UICC__FUNCTION__DFLink
		
	 	 return SelctLinkDF(fileAddr);
		#else
		 return fileAddr;
		#endif
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
UWORD SelectFileByName(UWORD fileAddr,BYTE * name,BYTE lgth)
{
	BYTE  idata fileType/*,recNumberLgth[2],fileStatus*/;  //remove warning,dengyiwen,2022/9/22
   	WORD  idata fileSize;
	BYTE  idata fileShortId;
	UWORD xdata fileUseSpace,fileSpaceAddr;
	BYTE xdata aidBuf[0x11];

	//在没有创建任何文件前，E_FILE_START 的值是0xFFFFFFFF，此时直接退出 
	if((fileAddr == INVALID_ADDR)||(fileAddr == FALSE))
	{
		return FALSE;
	}
   		
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
				 GetAIDNameInFile(fileShortId,&aidBuf[0]);

				 if((ecpMem_Compare(&aidBuf[1],name,lgth) == EQUAL)&&(aidBuf[0] >= lgth))
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
			fileSpaceAddr = GetFileHeadUsedSize( fileAddr);
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
//		else if(fileType == TRANTYPE)
//		{
//			fileSize = GetFileHeadWord(fileAddr, F_SpaceSize_Offset);
//		    
//			fileSpaceAddr = fileSize+sizeof(TranFileHead);
//		}
//		else if((fileType == LINTYPE)||(fileType == CYCTYPE))
//		{
//			ReadEEPromCache((BYTE idata *)&recNumberLgth[0],fileAddr+F_RecNum_Offset,sizeof(recNumberLgth));

//				fileStatus = GetFileHeadByte(fileAddr,F_Status_Offset);
//				
//				fileSpaceAddr = (recNumberLgth[0]*recNumberLgth[1])+sizeof(RecordFileHead) ;
//				
//				if((fileType == CYCTYPE) && ((fileStatus & F_MAP_FILE) != F_MAP_FILE))//循环文件且不是映射文件，才需要+1
//				{
//					fileSpaceAddr++;
//				}	
//		}
		else if((fileType == TRANTYPE)||(fileType == LINTYPE)||(fileType == CYCTYPE))
		{
			fileSpaceAddr = GetFileTotalSize(fileAddr);
		}
		else
		{
			return FALSE;
		}

   	}

	return FALSE;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SelectFileByShortId                                                                                                                   
                                                                                                                                                
  Input parameter:   fileaddr(the start addr of df,mf,adf)

					 shortName(df,ef,name)                                                                  
  					                                                                                                         
  Output parameter:  fileAddr/FALSE                                                                                                               
                                                                                                                                                
  Description:       在当前给出的DF或ADF下查找文件,如果存在返回文件的起始地址,对映射文件返回实体地址.
  
  Coder:		     CAIBO        Tester:     

  Test Status:         

 -------------------------------------------------------------------------------------------------------------------*/    
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

	ReadEeprom((BYTE idata *)&fileName,addr,sizeof(WORD));

	G_CurrentSfi = GetFileHeadByte(addr,F_ShordFid_Offset);					
	
	fileType = GetFileHeadByte(addr,F_Status_Offset);

	if((fileType&F_MAP_FILE) == F_MAP_FILE)
	{
		addr = GetParentOrMapFileAddr(addr, F_MapFile_Offset);			
	}
	
	IF_ProcessNoShareableFile(addr,fileName,0x00,NULLAPI_Mode);

	SetChannel(addr,fileName, FALSE);

	return addr ;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       
  Function name:	LocateRecordInFile
  
  input parameter:	fileAddr:	EF文件的地址
					tag:		寻找的TAG

  output parameter:	成功：记录号
					失败：FALSE
                                                                                                                                                
  description:		在TV结构的记录文件中，根据tag定位记录号

  Test:				Caibo
  
  status:			OK			
 -------------------------------------------------------------------------------------------------------------------*/
BYTE LocateRecordInFile(UWORD fileAddr, BYTE tag)
{
	BYTE idata i, tmp;
	BYTE idata recNum;

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	for(i = 0x00; i < recNum; i ++)
	{
		ReadLFBuf(fileAddr, i+1, 0, &tmp, 1);

		if(tmp == tag)
		{
			return (i+1);
		}
	}

	return FALSE;
}



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
	fileAddr = FALSE;

#if SE_ONLY

	fileAddr = SelectLFFile(DF_PROFILECFG,EF_PRIVATE_PIN);

#else
	if((keyId >= 0x81) && (keyId <= 0x88))//0x8x的pin从相应的ADF文件下面的pin文件读，否则从MF下面的pin文件里面读，8A~8E仍然从MF下面获取
	{
		if(G_CurrentDfName[G_ChannelNum] == 0x3F00)     // modify by xieyong@20211216  清除、设置、获取7F10下的localPIN状态做特殊处理
		{
			fileAddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],DF_PROFILECFG);
		}
		else
		{
			fileAddr = GetFirstLevelDf(G_CurrentDfAddr[G_ChannelNum]);
		}

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

  Function name :	 ClearLocalPinStatus                                                                                                                   
                                                                                                                                                
  input parameter:   fileAddr    当前ADF地址
					
					 *pinstatus  对应的LocalPin
  output parameter:  void                                                                                                              
                                                                                                                                                
  description:       判断当前LocalPin是否失效,如果没有失效,则清空Pin状态,用于SelectCommand重选ADF时候使用

	*@history    :V1.0 :create by LCZ 20210408

 -------------------------------------------------------------------------------------------------------------------*/ 

void ClearLocalPinStatus(UWORD fileAddr,BYTE *pinstatus)
{
	BYTE  idata i,j,reclen;
	UWORD xdata fileAddrlocalpin;
	KeyStructer xdata chvStuct;
	
	fileAddrlocalpin =  SelectFile(fileAddr,EF_PRIVATE_PIN);
	
	if(fileAddrlocalpin != FALSE)
	{
		reclen = GetLFRecNumLgth(fileAddrlocalpin,FileLgth);
		
		for(i= 0x81;i<=0x88;i++)
		{
			j = LocateRecordInFile(fileAddrlocalpin,i);	 //查找特定类型的PIN

			if(j != FALSE)
			{
				ReadLFBuf(fileAddrlocalpin,j,0,(BYTE xdata *)&chvStuct.KeyId,reclen);
				
				if((chvStuct.KeyStatus&0x01) == PIN_ENABLED)//设置MF下面每个ADF下的 local pin1-8的状态
				{
					ClearBit(*pinstatus,(i&0x0F)-0x01);
				}
			}
		}		
	}
		
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       
  Function name :	 ReadPinStatus                                                                                                                   
                                                                                                                                                
  input parameter:   void                                           
  					                                                                                                         
  output parameter:  void                                                                                                              
                                                                                                                                                
  description:       set the pin status to the paramater
  
* @history           :modify by zhxx@20221202，方案变动,DF下面也可以有localpin，对于localpin，应该遍历3F00下面的每个DF，判断F_LocalPinIndex_offset是否存在

  status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/    

void ReadPinStatus(void)
{ 
    BYTE idata status,i,fileType,localPinIndex,RecNumLgth;
	KeyStructer xdata chvStuct;
	BYTE idata j;
	UWORD xdata fileAddr = FALSE;
	UWORD xdata pinAddr;
	UWORD xdata fileUseSpace,fileSpaceAddr;
	WORD idata tempParameter;

	G_ChvStatus = 0x00;           //PIN1

#if SE_ONLY
	G_KeyStatus = 0x00;           //PIN2
#else
	G_AdmStatus = 0x00;
	ecpMem_Set(G_LocalPinStatus,0x00,sizeof(G_LocalPinStatus));
#endif
#if UICC__FUNCTION__UniversalPin	
	status = ReadKey(0x11,(BYTE xdata *)&chvStuct.KeyId);

	if(status == TRUE)
	{
		if((chvStuct.KeyStatus&0x01)==PIN_DISABLED)//设置Universal pin的状态
		{
			SetBit(G_UniversalPinStatus,0);
		}	
	}
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


#if SE_ONLY


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
	
	fileAddr = E_FILE_START;

	fileUseSpace = GetFileHeadUsedSize(fileAddr);

	fileUseSpace = fileUseSpace + fileAddr;
	
	for(fileAddr = fileAddr+sizeof(FileSystemHead);fileAddr < fileUseSpace;fileAddr = fileAddr + fileSpaceAddr)//遍历所有文件，找到F_LocalPinIndex_offset不为0的DF
	{
		fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

		if(fileType == ADFTYPE)
		{
			 fileSpaceAddr = GetFileHeadUsedSize( fileAddr);

			 localPinIndex = GetFileHeadByte(fileAddr,F_LocalPinIndex_offset);//获取localPinIndex

			 if((localPinIndex >= 1) && (localPinIndex <= sizeof(G_LocalPinStatus)))//判断localPinIndex在有效范围内
			 {
				pinAddr =  SelectFile(fileAddr,EF_PRIVATE_PIN);//获取当前DF下面是否有EF_PRIVATE_PIN文件

				if(pinAddr != FALSE)//如果存在local pin文件
				{
				  RecNumLgth = GetLFRecNumLgth(pinAddr,FileLgth);//获取pin文件记录长度

				  for(i= 0x81; i <= 0x88; i++)//遍历localpin  0x81~0x88
				  {
					j = LocateRecordInFile(pinAddr,i);	 //查找特定类型的PIN所在文件记录

					if(j != FALSE)
					{
						ReadLFBuf(pinAddr,j,0,(BYTE xdata *)&chvStuct.KeyId,RecNumLgth);

						if((chvStuct.KeyStatus&0x01) == PIN_DISABLED)//设置MF下面每个ADF下的 local pin1的状态
						{
							SetBit(G_LocalPinStatus[localPinIndex-1],(i&0x0F)-0x01);
						}
					}
				  }
				}
			 }
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
		else if((fileType == TRANTYPE)||(fileType == LINTYPE)||(fileType == CYCTYPE))
		{
			fileSpaceAddr = GetFileTotalSize(fileAddr);
		}
		else
		{
			 break;
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
			 V1.1 : 修改keyStatus的类型声明改为WORD， 由于G_AdmStatus的类型被改为WORD,此需求来源是移动NFC2.0 SWP卡要求AC 应用下文件的更新权限为ADM10 @20170211
			  V1.2:LCZ 为支持Global Pin 1-8/Local Pin 81-88  G_ChvStatus/G_LocalPinStatus 从bit0开始使用-之前都是bit1开始使用。参数keyRefense为01-08 则要减1使用,G_AdmStatus不改,bit位够用
*/
BYTE ConfigureKeyStatus(WORD keyRefense,BYTE mode)
{
	WORD idata  keyStatus;
	BYTE idata recNum,result;
	UWORD xdata fileAddr;
	BYTE xdata  tmpStatus[8];  //目前或/与权限支持 PIN1 PIN2 ADM1 ADM2  ADM3 ADM4，需要扩展的话 该数组相应的扩大  增加LocalPIn1/2  大小增加2

	fileAddr = FALSE;
	
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
#if UICC__FUNCTION__UniversalPin	
	else if(keyRefense == 0x11)                 //Universal PIN
	{
		keyStatus = G_UniversalPinStatus;
		keyRefense = 0x00;                   //use bit 0
		
		if(mode == SetStatusMode)	
		{
			SetBit(G_UniversalPinStatus,keyRefense);
		}
		else if(mode == ClearStatusMode)
		{
			ClearBit(G_UniversalPinStatus,keyRefense);	
		}
		else   //白盒测试需要，代码量不变
		{
			;
		}  
	}	
#endif
	else if(((keyRefense&0x0080)==0x0080)&&(keyRefense<0x008A))
	{

#if SE_ONLY
	   keyStatus = G_KeyStatus;        //PIN2

		keyRefense = keyRefense & 0x7F;

		keyRefense -= 0x01;

		if(mode == SetStatusMode)
		{
			SetBit(G_KeyStatus,keyRefense);
		}
		else if(mode == ClearStatusMode)
		{
			ClearBit(G_KeyStatus,keyRefense);
		}
		else   //
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
		// modify by zhangxiaoxiao@20221204:			
		//1、创建3F00时，文件头的LocalPinIndex赋初值1
        //2、ReadPinStatus时，遍历localpin，需要遍历文件系统的整个DF，判断LocalPinIndex是否为0，localpin存在于DF/ADF下面
		if((recNum >= 1) && (recNum <= sizeof(G_LocalPinStatus)))
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
	else if(((keyRefense & 0xF000) == 0xC000) || ((keyRefense & 0xF000) == 0xD000))//OR  or AND
	{
		ecpMem_Set(tmpStatus, 0x00,sizeof(tmpStatus));

		if((keyRefense&0x0001)==0x0001)   //PIN1
		{
			tmpStatus[0]=ValueBit(G_ChvStatus,0);
		}
		if((keyRefense&0x0002)==0x0002)    //PIN2
		{
			tmpStatus[1]=ValueBit(G_ChvStatus,1);
		}
		
		if(((keyRefense&0x0040)==0x0040) || ((keyRefense&0x0080)==0x0080))    //Localpin1/2
		{
#if SE_ONLY
			if((keyRefense&0x0040)==0x0040)
			{

			   tmpStatus[6]=ValueBit(G_KeyStatus,0);//local pin1

			}
			else
			{
				tmpStatus[7]=ValueBit(G_KeyStatus,1);//local pin2
			}
#else
			fileAddr = GetFirstLevelDf(G_CurrentDfAddr[G_ChannelNum]);

			if(fileAddr == FALSE)
			{
				return TRUE;
			}

			recNum = GetFileHeadByte(fileAddr,F_LocalPinIndex_offset);
			
			// modify by zhangxiaoxiao@20221204:			
			//1、创建3F00时，文件头的LocalPinIndex赋初值1
            //2、ReadPinStatus时，遍历localpin，需要遍历文件系统的整个DF，判断LocalPinIndex是否为0，localpin存在于DF/ADF下面
			if((recNum >= 1) && (recNum <= sizeof(G_LocalPinStatus)))
			{
				keyStatus = G_LocalPinStatus[recNum-1];
			}
			else
			{
				return TRUE;
			}
			
			if((keyRefense&0x0040)==0x0040)
			{
			
			   tmpStatus[6]=ValueBit(keyStatus,0);//Local Pin1
				
			}
			else
			{
				tmpStatus[7]=ValueBit(keyStatus,1);//Local Pin2
			}
#endif
		}
		if((keyRefense&0x0004)==0x0004)    //ADM1
		{
			tmpStatus[2]=ValueBit(G_AdmStatus,1);
		}
		if((keyRefense&0x0008)==0x0008)    //ADM2
		{
			tmpStatus[3]=ValueBit(G_AdmStatus,2);
		}
		if((keyRefense&0x0010)==0x0010)     //ADM3
		{
			tmpStatus[4]=ValueBit(G_AdmStatus,3);
		}
		if((keyRefense&0x0020)==0x0020)    //ADM4
		{
			tmpStatus[5]=ValueBit(G_AdmStatus,4);
		}
		
		for(result=0x00;result<sizeof(tmpStatus);result++)
		{
			if(ValueBit(keyRefense,result))
			{
				if((keyRefense&0xF000)==0xC000)      //或权限
				{
					if(tmpStatus[result] == TRUE)
					{
						return TRUE;
					}
					
				}
				else
				{
					if(tmpStatus[result] == FALSE)
					{
						return FALSE;
					}
				}
			}
		}
		
		if((keyRefense&0xF000)==0xC000)  //OR
		{
			return FALSE;
					
		}
		else 
		{
			return TRUE;
				
		}
	}
	else
	{
		keyStatus = G_AdmStatus;       //ADM1-ADM10

		keyRefense = ((keyRefense&0x0080) == 0x0080) ? ((keyRefense&0x000F)-0x04) : (keyRefense-0x09);

		if(mode == SetStatusMode)	
		{
			SetBit(G_AdmStatus,keyRefense);
		}
		else if(mode == ClearStatusMode)
		{
			ClearBit(G_AdmStatus,keyRefense);	
		}
		else   //白盒测试需要，代码量不变
		{
			;
		}  
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
BYTE  GetKeyRefenseStatus(WORD keyRefense)
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
*           modify by xuweihua@20211106 AB/8C规则存储在私有文件2700/AF06，增加宏开关UICC_AB8C_ARR_PRIVATE
*/
WORD GetAcessPurview(UWORD fileAddr,BYTE action)
{
	BYTE  idata arrNum,fileLgth,tempPosition;
	UWORD xdata arrFileAddr;
	BYTE idata arrName,i;
	UWORD xdata addr;
	WORD idata parName,keyStatus;     //keyStatus为了支持或/与权限 返回状态字改为WORD类型

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

	
	if(arrFileAddr == FALSE)            //若文件arr不存在，则默认权限为always
	{
		return 0x90;
	}
	else
	{
		arrNum = GetFileHeadByte(fileAddr, F_ArrNum_Offset);

		//2F06FF和2F06FE两条记录专门给私有文件使用，权限为全部NEV和ADM, by xwh 20190321
		if(arrNum == ARR_RECNUM_NEV)
		{
			return 0x97;
		}
		else if(arrNum == ARR_RECNUM_ADM)
		{
			return 0x0A;
		}
		
#if UICC_AB8C_ARR_PRIVATE
		if((arrNum & A_PRIVATEARR_REC_FLAG) == A_PRIVATEARR_REC_FLAG)
		{
			fileLgth = GetPrivateArrInfoToExpand(arrNum,tempBuf,tempBuf+MaxUseLen);
		}
		else
#endif
		{
			fileLgth = GetLFRecNumLgth(arrFileAddr,FileLgth);

			if(ReadLFBuf(arrFileAddr,arrNum,0,&tempBuf[0],fileLgth) == FALSE)//记录不存在 则返回FALSE
			{
				return FALSE;
			}
		}
	}
	
	//for(arrNum =0x00; ((tempBuf[arrNum] != 0xFF) && (arrNum < fileLgth)); arrNum = arrNum + 5+ tempPosition)
	//判断条件位置换一下，防止越界访问
	for(arrNum =0x00; ( (arrNum < fileLgth)&&(tempBuf[arrNum] != 0xFF)); arrNum = arrNum + 5+ tempPosition)
	{
		if((tempBuf[arrNum]!=0x80)&&(tempBuf[arrNum]!=0x84)&&(tempBuf[arrNum+1]!=0x01))
		{
			return FALSE;
		}

		tempPosition = tempBuf[arrNum+4];
		
		//if((((tempBuf[arrNum+2]&action) == action)&&(tempBuf[arrNum] == 0x80)&&(action != A_IncreaseFile)&&(action != A_ResizeFile))||((tempBuf[arrNum]==0x84)&&((action == A_IncreaseFile)||(action == A_ResizeFile))))								//Find the 
		if((((tempBuf[arrNum+2]&action) == action)&&(tempBuf[arrNum] == 0x80)&&(action != A_IncreaseFile)&&(action != A_ResizeFile))||((tempBuf[arrNum]==0x84)&&((action == A_IncreaseFile)||(action == A_ResizeFile))&&(tempBuf[arrNum+2]==action)))								//Find the 
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

					return tempBuf[arrNum +4+3];
				}
// 				case 0x84:                   //Increase  OR RESIZE命令特定参数
// 				 {
// 					if((tempBuf[arrNum+5]== action)&&((tempBuf[arrNum+8])==0x83))
// 					{
// 					    return tempBuf[arrNum+10];
// 					}
// 					else 
// 					{
// 					    return FALSE;
// 					}					
// 				 } 
				case 0xA0:                            //OR template   ,暂时仅支持PIN1 PIN2 ADM1 ADM2的组合，用返回值的低4bit表示
				case 0xAF:                            //AND template	暂时仅支持PIN1 PIN2 ADM1 ADM2的组合，用返回值的低4bit表示
				{
					if(tempBuf[arrNum+3] == 0xA0)
					{
						keyStatus = 0xC000;
					}
					else
					{
						keyStatus = 0xD000;
					}

					for(i=0x00;i<tempPosition;)
					{
					 	if((tempBuf[arrNum+7+i]==0x83)&&(tempBuf[arrNum+8+i]==0x01))
						{
							if(tempBuf[arrNum+9+i] == 0x01)
							{
								keyStatus |= 0x0001;			 //PIN1
							}
							else  if(tempBuf[arrNum+9+i] == 0x02)
							{
								keyStatus |= 0x0002;			 //PIN2
							}
							else  if(tempBuf[arrNum+9+i] == 0x0A)
							{
								keyStatus |= 0x0004;		    //adm1
							}
							else  if(tempBuf[arrNum+9+i] == 0x0B)
							{
								keyStatus |= 0x0008;		      //ADM2
							}
							else  if(tempBuf[arrNum+9+i] == 0x0C)
							{
								keyStatus |= 0x0010;		      //ADM3
							}
							else  if(tempBuf[arrNum+9+i] == 0x0D)
							{
								keyStatus |= 0x0020;		      //ADM4
							}
							else  if(tempBuf[arrNum+9+i] == 0x81)
							{
								keyStatus |= 0x0040;		      //LocalPin1
							}
							else  if(tempBuf[arrNum+9+i] == 0x82)
							{
								keyStatus |= 0x0080;		      //LocalPin2
							}
							else
							{
								//	APDU暂时不支持 0xA0、0xAF ; Toolkit 支持 0xA0、0xAF模板
							}
						}
						i += tempBuf[arrNum+6+i]+2;
					}
					return keyStatus;
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
*             modify by yanjiru@20171017:  将DeleteFile和ResizeFile分开判断，否则如果该两个权限都配置了，执行DeleteFile时，ResizeFile也执行
*             modify by yanjiru@20211229:  创建文件时，带有92TAG,值为1，则该文件不允许被读，不允许被映射
*             modify by xwh@20220106  KI文件,鉴权opc等参数文件 pin/puk文件,不允许被read
*/
BYTE AccessCondition(UWORD fileAddr,BYTE action)
{
#if UICC__OPERATOR__CHINATELECOM__4G
	BYTE  idata result;
#endif
	BYTE  idata fileType,actionStatus,tag;
	WORD  idata keyStatus;            //为了支持或/与权限，keyStatus扩展为WORd类型

	//return TRUE;
	//ReadEeprom(&result,CARD_LIFE_CYCLE,sizeof(BYTE));
	
    //if((result&0x0F) != 0x0F)          //不是锁卡状态，不用校验权限
    //{
	//	return TRUE;
    //}

	if((JcvmConfig_GetSEState() == CARD_OP_READY)&&(BootLifeCycle_IsBlockBootConfig(BLOCK_BIT_MASK)==0))
	{
		return TRUE; 
	}
	//中国电信4G 使用2F08 文件控制RFM 权限, by xwh 20210224
#if UICC__OPERATOR__CHINATELECOM__4G
	result = IF_AccessCondition(fileAddr,action);

	if(result != 0xFF)
	{
		return result;
	}	
#endif
	
	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

	if( fileType != ADFTYPE )
	{
		fileAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);

		if(fileAddr == FALSE)
		{
			return FALSE;
		}
	}
	
	if(action == A_ReadFile)
	{		
		tag = ReadEECacheByte(fileAddr + F_MapAndRead_Offset);//创建文件时，带有92TAG,值为1，则改文件不允许被读，不允许被映射by yjr@20211223,此处不能调用GetFileHeadByte
	
		if((tag&File_MapAndRead) == File_MapAndRead)
		{
			return FALSE;
		}
		
		//KI文件,鉴权opc等参数文件 pin/puk文件,不允许被read
		if(((G_CurrentEfName[G_ChannelNum] == EF_KI)&&((G_CurrentDfName[G_ChannelNum] == G_DF_ID_USIM)
#if UICC__AUTH__CDMA__CAVE			
			||(G_CurrentDfName[G_ChannelNum] == G_DF_ID_CSIM)
#endif
#if UICC__ISIM		
		||(G_CurrentDfName[G_ChannelNum] == G_DF_ID_ISIM)
#endif		
		)) 
			|| ((G_CurrentEfName[G_ChannelNum] == EF_PRIVATE_AUTHCFG)&&((G_CurrentDfName[G_ChannelNum] == G_DF_ID_USIM)
#if UICC__ISIM		
		||(G_CurrentDfName[G_ChannelNum] == G_DF_ID_ISIM)
#endif		
		)) 
			|| (((G_CurrentEfName[G_ChannelNum] == EF_PRIVATE_PUK)||(G_CurrentEfName[G_ChannelNum] == EF_PRIVATE_PIN))&&(G_CurrentDfName[G_ChannelNum] == DF_PROFILECFG))
		    || ((G_CurrentEfName[G_ChannelNum] == EF_Ki_TPX)&&(G_CurrentDfName[G_ChannelNum] == DF_PHASE))
		    || ((G_CurrentEfName[G_ChannelNum] == EF_AuthCFG_TPX)&&(G_CurrentDfName[G_ChannelNum] == DF_MF)))
		{
			return FALSE;
		}
	}	
	
	if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)	//RFM权限
	{
		return RFMAccessContion(fileAddr,action);
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
		//actionStatus = ValueBit(G_SystemStatus,B_cmk);		 //校验MF key
		actionStatus = (SECURITY_STATE_MFKEY_VERIFIED == g_securityState);
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

		if((dataPointer[i]&0x7F) == (tag&0x7F))
		{
			return ((i&0xFF)+1+ch);			
		}
	
		i =i + dataPointer[i+1+ch]+2+ch;
	}

	return FALSE;
}

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

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	SearchStarting

  input parameter:	recordNo:记录号

					maxRecnum:记录数

					step:步长,有符号类型
  					
  output parameter:  TRUE/FALSE

  description:       find a record starting with the given pattern for 2G

  status:			 OK

 -------------------------------------------------------------------------------------------------------------------*/

BYTE SearchStarting(BYTE recordNo, BYTE maxRecnum, SBYTE step)
{	
	for(; (recordNo > 0) && (recordNo <= maxRecnum) ; recordNo = recordNo+step)
	{
		ReadLFBuf(G_CurrentEfAddr[G_ChannelNum], recordNo, 0, (BYTE * )&RecData[P3],  P3);

		if(ecpMem_Compare(&RecData[0], &RecData[P3], P3) == EQUAL)
		{
			return recordNo;
		}
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
BYTE OffsetAfterFirstOccurrence(BYTE physicalNo,BYTE value,BYTE recLen)
{
	BYTE idata pos;
	BYTE idata tmp;

	for(pos=0x00 ; pos<recLen ; )
	{
		ReadLFBuf(G_CurrentEfAddr[G_ChannelNum],physicalNo,pos,&tmp,sizeof(BYTE));
		
		pos ++;

		if(tmp == value)
		{
			return pos;
		}	
	}

	return FALSE;
}
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
  
  modify:           该函数会在SeekCommand函数中调用，调用时传入的searchString为局部变量RAM
                    该函数会在UICCToolkit_searchRecord函数中调用，调用时传入的searchString参数是JAVA层uicc.framework.UICCFileViewImpl.searchRecord传递过来的，可能是RAM，可能是E2

@history        :V1.2为了提高seek速度 ，开通临时变量srcTmp  dstTmp增强数组@20180105
                                                                           
																		

 -------------------------------------------------------------------------------------------------------------------*/    


#define CMP_SEARCHLEN  0x10
BYTE SearchContaining(SBYTE step,BYTE recordNo,BYTE offsetValue,BYTE * searchString,BYTE length,BYTE needAfterFirstOccurrence)
{
	BYTE idata j,k;
	BYTE idata result;
	BYTE idata recLen;
	BYTE idata offset;
	BYTE idata physicalNo;
	BYTE idata tag,RecIndicator;
	BYTE idata validRecNumber;
	BYTE idata compareNum,compareLen;
	
	BYTE xdata bitRecord[0x20];  //最多支持256条记录，实际记录只有255 ，，每个bit表示一个record
	BYTE idata arrayIndex,bitNumber,backRecordNo;
	

	BYTE xdata dstTmp[CMP_SEARCHLEN];
	
	RecordFileHead xdata fileHead;

	ReadEEPromCache((BYTE xdata *)&fileHead.FileId,G_CurrentEfAddr[G_ChannelNum],sizeof(RecordFileHead));
	
	RecIndicator = getCycleFPoint(G_CurrentEfAddr[G_ChannelNum]);

	validRecNumber =  fileHead.RecNumber;

	recLen = GetLFRecNumLgth(G_CurrentEfAddr[G_ChannelNum],FileLgth);	

	ecpMem_Set(tempBuf, 0x00, sizeof(tempBuf));
	
	compareNum = length/CMP_SEARCHLEN;  
		
	if((length&(CMP_SEARCHLEN - 1)) != 0)
	{
		compareNum ++;
	}
	
	backRecordNo = recordNo;
	
	ecpMem_Set(bitRecord, 0x00, sizeof(bitRecord));

	for(result = 0x00; (recordNo > 0) && (recordNo <=  validRecNumber) ; recordNo = recordNo+step)
	{
		if((fileHead.FileType&0x07) == CYCTYPE)
		{
			physicalNo = ((RecIndicator+recordNo-1) >  fileHead.RecNumber?((RecIndicator+recordNo-1)- fileHead.RecNumber):(RecIndicator+recordNo-1));	
		}
		else
		{
			physicalNo = recordNo;
		}

		if(needAfterFirstOccurrence == TRUE)
		{
			offset = OffsetAfterFirstOccurrence(physicalNo,RecData[1],recLen);
		
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
		
		tag = ReadLFBuf(G_CurrentEfAddr[G_ChannelNum],physicalNo,offset,&tempBuf[0],recLen - offset) ;  //一次性读出记录中的剩余数据

		if(tag == FALSE)
		{
			break;
		}
		
		for(j = 0; j < (recLen - offset); j++) 
		{
			if((recLen - offset - j) < length)   //记录的剩余数据长度如果比待比较的数据长度小，则不再继续比较 
			{
				break;
			}
			
			for(k=0; k < compareNum ; k++)                        
			{  
				if(k == (compareNum - 1))  //最后一条的待读取的记录长度
				{
					compareLen = length - k *CMP_SEARCHLEN;
				}
				else
				{
					compareLen = CMP_SEARCHLEN;
				}	
				
				//ReadEeprom((BYTE *)&dstTmp[0],(UWORD)searchString+k*CMP_SEARCHLEN,compareLen);
				
				VMReadBuf((BYTE *)&dstTmp[0],(UWORD)searchString+k*CMP_SEARCHLEN,compareLen);
						
				if(ecpMem_Compare(&tempBuf[j+k*CMP_SEARCHLEN], &dstTmp[0], compareLen ) != EQUAL)
				{
					break;
				}                                                                                                          
			}  

			if(k == compareNum)
			{
				arrayIndex = (recordNo-1)/8; //数组的第n个元素	，从0开始, 当objRef/8为双字节时，arrayIndex如果定义为byte型，则会被截断

				bitNumber = (recordNo-1) %8; //数组第n个元素的第n个bit	 ，从0开始	
				
				SetBit(bitRecord[arrayIndex], bitNumber);
				
				result++;

				break;
			} 
		}			
		
  	}	
	
	ecpMem_Set(tempBuf, 0x00, sizeof(tempBuf));
	
	tag = 0;
	
	for(; (backRecordNo > 0) && (backRecordNo <=  validRecNumber) ; backRecordNo = backRecordNo+step)
	{
		arrayIndex = (backRecordNo-1)/8; //数组的第n个元素	，从0开始, 当objRef/8为双字节时，arrayIndex如果定义为byte型，则会被截断

		bitNumber = (backRecordNo-1) %8; //数组第n个元素的第n个bit	 ，从0开始	
		
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
/*************************FDN 同步的函数 ****************************************/
/*
*@function: FileLifeCycleHandle
*
*@description:  失效或激活某个df下的Ef文件 
*
*@param       :dfName
*@param       :efName
*@param       :action:激活or失效
×
×@return      :void
*
*@author      :xuyz
*
*@history     :
*/
void FileLifeCycleHandle(WORD dfName,WORD efName,BYTE action)
{
	UWORD xdata fileAddr;
    BYTE  idata efStatus;
	UWORD xdata CurrentEfHeadAddr;	

//	fileAddr =  SelectTFFile(dfName,efName);
	
	fileAddr = SelectFile(E_FILE_START,dfName);

 	if( fileAddr == FALSE)
   	{
    	return ;
	}
	
	CurrentEfHeadAddr = SearchFile(fileAddr,efName);
	
 	if( CurrentEfHeadAddr == FALSE)
   	{
    	return ;
	}

	efStatus = GetFileHeadByte(CurrentEfHeadAddr, F_Status_Offset);

 	if(action == FILE_REINVALIDATE)
	{
	   efStatus |= 0x01;
	}
	else
	{
	   efStatus &= 0xfe;
	}

    SetEepromEX(CurrentEfHeadAddr + F_Status_Offset,efStatus,sizeof(BYTE));

	return ;
}
#if (!UICC__GSM_2G_NOT_SUPPORT)
/*
*@function :UsimFdnSYNCtoSIM

*@description: 将USIM 下的FDN 状态同步到SIM 下
*
*@param      :fdnStatus:TRUE失效ADN 文件 /FALSE:激活ADN 文件
*
*@return     :void
*
*@author     :
*/
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
	
	fileStatus = 0x00;
	
  fileAddr = SelectTFFile(DF_PHASE,EF_SST);  //2G EF_SST 读服务状态

	if(fileAddr != FALSE)
	{
		ReadTFBuf(fileAddr, 0, (BYTE  *)&fileStatus, sizeof(BYTE));
	}

	if((fileStatus&0x3C) != 0x3C)// ADN  FDN 未同时分配激活	 ,不同步
	{
		return ;
	}

	FileLifeCycleHandle(DF_TELECOM,EF_ADN,(fdnStatus==TRUE)?FILE_INVALIDATE:FILE_REINVALIDATE);	 //失效文件

}
/*
*@function   :IsFdnServiceActiveInSIM
*
*@description:判断FDN服务是激活还是失效 
		          GSM 11.11: 
							If ADN is not activated, service n°3 is enabled. If ADN is
							activated, the ME checks the response data of EFADN. If EFADN is invalidated, service n°3 is enabled. In all other cases
							service n°3 is disabled
*
*@param       :void
*
*@return       :false:FDN服务无效/TRUE:fdn服务有效 
×
×@author       ：
*/
BYTE IsFdnServiceActiveInSIM(void)
{
	UWORD xdata fileAddr,adnAddr;
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
	
//	adnAddr =  SelectTFFile(DF_TELECOM,EF_ADN);
	
	fileAddr = SelectFile(E_FILE_START,DF_TELECOM);
	
	if(fileAddr == FALSE)
	{
		return FALSE;	
	}
	
	adnAddr = SearchFile(fileAddr,EF_ADN);
	
	if(adnAddr != FALSE)
	{
		 efStatus = GetFileHeadByte(adnAddr, F_Status_Offset);
		
		 if((efStatus & 0x01) == FILE_REINVALIDATE )          //ADN文件激活
		 {
		 	return FALSE;	 
		 }
	}
	
	return TRUE;

}
/*
*@function    : IsFdnServiceActiveInUSIM
*
*@description :判断FDN 服务在 USIM 中是否有效 
*
*@param       :void
*
*@return      :TRUE:FDN 服务启用/FALSE： FDN服务失效
*
*@author       :xuyz
*
*@history     :
*/
BYTE IsFdnServiceActiveInUSIM(void)
{
	UWORD xdata ustAddr,estAddr;
	BYTE  idata USTStatus,ESTStatus;

	USTStatus = 0x00;
	ESTStatus = 0x00;

	ustAddr =  SelectTFFile(G_DF_ID_USIM,EF_UST);  //读取3G下服务列表

	if(ustAddr !=  FALSE)
	{
		ReadTFBuf(ustAddr, 0, (BYTE idata *)&USTStatus, sizeof(BYTE));	 

		if((USTStatus&0x02)!=0x02)	 //未分配
		{
			return FALSE;
		}
		estAddr =  SelectTFFile(G_DF_ID_USIM,EF_EST);	   // 3G 下FDN服务状态

		if(estAddr != FALSE) 
		{
			ReadTFBuf(estAddr, 0, (BYTE idata *)&ESTStatus, sizeof(BYTE));

			if((ESTStatus&0x01)==0x01)
			{
				return TRUE;
			}			
		}		
	}

	return FALSE;
}

/*
*@function : SimFdnSYNCtoUSIM
*
*@parameter ：fdnStatus ：FALSE:fdn失效/true：fdn激活
*
*@return    :void
*
*@author
*/
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
 #endif
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 FdnProgcess                                                                                                                   
                                                                                                                                                
  input parameter:   void                                           
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       set the ef information to the gsm buffer
  
  status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/    
void FdnProgcess(void)
{
#if (!UICC__GSM_2G_NOT_SUPPORT)
	UWORD xdata fileAddr;
    BYTE  idata efStatus;

	fileAddr = SelectTFFile(DF_PHASE,0x6F38);

    if(fileAddr == FALSE)
	{
        return;
	}
	
 	ReadTFBuf(fileAddr,0,(BYTE idata *)&efStatus,sizeof(BYTE));

    if((efStatus & 0x30) != 0x30)//fdn active and allowed
	{
         return;
	}

 	if((efStatus & 0x0C) == 0x0C)//adn active and allowed
    { 
		fileAddr = SelectFile(E_FILE_START,DF_TELECOM);
		
		if( fileAddr == FALSE)
		{
			return;
		}
		
   		fileAddr = SearchFile(fileAddr,EF_ADN);

        if(fileAddr == FALSE)
		{
        	return;
		}
		efStatus = GetFileHeadByte(fileAddr, F_Status_Offset);

    	if((efStatus & 0x01) == FILE_REINVALIDATE )
		{
     		return;
		}
 	}
	
	fileAddr = SelectFile(E_FILE_START,DF_PHASE);
	
	if( fileAddr == FALSE)
	{
 		return;
	}
	
 	fileAddr =  SearchFile(fileAddr,EF_IMSI);

 	if( fileAddr == FALSE)
   	{
    	return;
	}
 
	efStatus = GetFileHeadByte(fileAddr, F_Status_Offset);
 
    SetEepromEX(fileAddr + F_Status_Offset,efStatus & 0xFE,sizeof(BYTE));
	
	fileAddr = SelectFile(E_FILE_START,DF_PHASE);
	
	if( fileAddr == FALSE)
	{
 		return;
	}

 	fileAddr =  SearchFile(fileAddr,0x6F7E);

 	if( fileAddr == FALSE)
	{
 		return;
	}
	efStatus = GetFileHeadByte(fileAddr, F_Status_Offset);

 	SetEepromEX(fileAddr + F_Status_Offset,efStatus & 0xFE,sizeof(BYTE));
#endif	
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
	if(G_CurrentDfAddr[G_ChannelNum] == FALSE)
	{
		return;
	}

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
	BYTE xdata tmp[4];
	UWORD xdata fileAddr;

	fileAddr = SelectLFFile(DF_MF,G_LTEDirFileID);

	if(fileAddr == FALSE)
	{
		 return;
	}

	ReadLFBuf(fileAddr,recordNum,0x00,tmp,sizeof(tmp));
	
	if((tmp[0] != 0x61)||(tmp[2] != 0x4F)||(tmp[3] > 0x10))
	{
		aidBuf[0] = FALSE;
		
		return;
	}
	
	aidBuf[0] = tmp[3];
	
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
 *            该函数同时供JAVA虚拟机和APDU使用，调用函数为Toolkit_GetKeyStatus、CreateDFFile、UICCNativeImpl_ProcessRAMAccessDomain
*/
BYTE GetAIDNumber(BYTE *aidBuf, BYTE profileType)
{
	BYTE idata 	recNum,num;
	BYTE xdata aidCmpBuffer[0x12];
	UWORD xdata fileAddr;
	
	fileAddr = FALSE;

	if(profileType == FALSE)
	{
		fileAddr = SelectLFFile(DF_MF,G_LTEDirFileID);
	}
#if (EUICC_ONLY||SE_AND_EUICC)
	else
	{
		fileAddr = SelectFile(G_CreatingMFOffset,G_profileControlContext.CURLOADDIR) ;
	}
#endif
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

		if(aidCmpBuffer[1] == aidBuf[0])
		{
			if(ecpMem_Compare(&aidCmpBuffer[2],&aidBuf[1],aidCmpBuffer[1]) == 0x00)
			{
				return (num+1);
			}
		}
	}	 

	return FALSE;	  
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

	fileAddr = SelectFile(E_FILE_START,G_LTEDirFileID);

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
		
		if(ecpMem_Compare(&aidCmpBuffer[2],&aidBuf[1],aidBuf[0]) == 0x00)
		{
			return (num+1);
		}
		
	}	 

	return FALSE;	  
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

		fileSpaceSize = tempParameter+sizeof(TranFileHead);
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
		
			fileSpaceSize = (UWORD)(recNumberLgth[0]*recNumberLgth[1])+sizeof(RecordFileHead) ;	//fileSpaceAddr as Space

			if((fileType == CYCTYPE) && ((fileStatus & F_MAP_FILE) != F_MAP_FILE))//
			{
				fileSpaceSize++; //
			}
		}
	}
#if UICC__FUNCTION__BERTLVCmd
	else if(fileType == BERTLVTYPE)                                       //BER-TLV
	{
		 tempParameter = GetFileHeadWord(fileAddr, F_SpaceSize_Offset);

		 fileSpaceSize = tempParameter+sizeof(BERTLVFileHead);

		 tempParameter = GetFileHeadWord(fileAddr, F_BERTLV_ReservedSize);

		 fileSpaceSize = tempParameter +  fileSpaceSize;
	}
#endif
	else
	{
		return FALSE;
	}
	
	return fileSpaceSize;
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
	
	for(fileAddr = fileAddr+sizeof(FileSystemHead);fileAddr < fileUseSpace;fileAddr = fileAddr + fileSpaceAddr)  // 遍历MF下的文件
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
		}
		
		fileSpaceAddr = GetFileTotalSize(fileAddr);
		
		if(fileSpaceAddr == FALSE)
		{
			return FALSE;
		}
   	}
	
	return FALSE;
}
/*
*@description:??2F00???????
*
*@param      :recNum:???
*
*@return     :void
*
*@history    :create by xyz@100513 8:59
*/
void DeleteDIRNum(BYTE recNum)
{
	UWORD xdata fileAddr;

	 fileAddr = SelectLFFile(DF_MF,G_LTEDirFileID);

	if(fileAddr == FALSE)
	{
		 return ;
	}

	SetLFConst(fileAddr,recNum,0x00,0xFF,GetLFRecNumLgth(fileAddr,FileLgth));
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetBerTlvLength
  
  input parameter:   void
   
  output parameter:

  description:
  					 
  
  history:	 modify by chentao @20220627 eUICC_M2M_SGP_11_v3_3_Card_Compliance_Test_Suite_v2_2_0\TC_SAR_5_SCP03T_ErrorManagement_01_Error_Case_Incorrect_Length_in_Init_update.js
			84,85,86 tag 的长度特殊处理,如果不特殊处理的话在GetBerTlvLength函数抛了异常，不能走到SCP03T层，无法返回9F44状态字
 -------------------------------------------------------------------------------------------------------------------*/    
UWORD GetBerTlvLength(BYTE * buffer,WORD * lenByte)
{
	UWORD  idata length; 
	BYTE  idata off;

	off = 0;
	length = 0;

	if(buffer[0] > 0x83)//HB_Offline_Test/5KeyEstablish指令异常测试/Tele_TC9_ISDR_KeyEstablishment_ERROR_01/步骤10、ISD-R,EstablishISDRKeySet cmd1 cert len error
	{
		ThrowError(0x6A80);
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
*@description:  dataOrder 从小到大, 冒泡排序
*
*@param      :recNum:记录号
*
*@return     :void
*
*@history    :create by jiangqb@20200420
*/
void dataOrder(BYTE size,BYTE *Buf)
{
	BYTE idata i,j,temp;
		
	for(i=0; i<size; i++)
	{
		for (j=0; j < size-1-i; j++)
		{
			if (Buf[j] > Buf[j+1])
			{
				temp = Buf[j];
				
				Buf[j] = Buf[j+1];
				
				Buf[j+1] = temp;
			}
		}
		
	}	
}

