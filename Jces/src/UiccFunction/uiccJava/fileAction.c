/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
			V1.1 修改函数GetFileHeadByte，增加BER-TLV类型的判断（1.19 -> 1.20）
			V1.2 修改函数GetFileHeadByte，定义宏BERTLVCmd，将BER-TLV结构的处理放入该宏内 --20111027 by chenl
			v1.3 增加函数ReadTFBufExtend， 用于读取透明文件中超过256 字节的数据
			V1.4 修改函数FindTFAddr与FindLFAddr(定义宏LONG_LIFE_BAK_FILE时),判断有备份文件时才去找备份文件地址 

 */
#include  "typedef.h"
#include  "uicc_main.h"
//#include  "main.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include "fileAction.h"
#include "interface.h"
//#include "transaction.h"
//#include "VMAccessEeprom.h"
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	GetParentOrMapFileAddr                                                                                                                   
                                                                                                                                                
  input parameter:	UWORD fileAddr 
  
  					BYTE offset:	F_ParentName_Offset
									F_MapFile_Offset                                          
  					                                                                                                         
  output parameter:	UWORD fileAddr                                                                                                           
                                                                                                                                                
  description:		
  
  status:			OK         

 -------------------------------------------------------------------------------------------------------------------*/ 
UWORD GetParentOrMapFileAddr (UWORD fileAddr,BYTE offset)
{
	WORD xdata fileName[3];

	ReadEeprom((BYTE *)&fileName[0],fileAddr + offset,sizeof(fileName)); 

	fileAddr = SearchFile(E_FILE_START,fileName[0]);

	if(fileAddr != FALSE)
	{
		fileAddr = SearchFile(fileAddr,fileName[1]);					  	
	}

	if(offset == F_MapFile_Offset)
	{
		if(fileAddr != FALSE)
		{
			fileAddr = SearchFile(fileAddr,fileName[2]);					  	
		}
	}

	return fileAddr;
}


/*
 *@Function: getCycleFPointAddr
 *
 *@Descrip:  获取循环文件的文件体的地址
 *
 *@Param:    UWORD fileAddr --   文件地址
        
 *
 *@Return:   BYTE	cycleFPoint ---	  循环文件的记录指针
 *
 *@Author:   zhxx 20200221
 *
 */
UWORD getCycleFPointAddr(UWORD fileAddr)
{	
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	return GetHeFlashFileDataAddr(fileAddr);		
#else
	return (fileAddr+sizeof(RecordFileHead));
#endif	
}



/*
*@funtion    :GetLastDFAddr
*
*@description: 获取下载的profile中最后一个DF地址
*
*@param      :
*
*@return     :UWORD  --获取下载的profile中最后一个DF地址
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20200701
*/
UWORD GetLastDFAddr()
{

	//WORD  idata tempParameter;
	BYTE  idata fileType;
	//BYTE  idata recNumberLgth[2];	//remove warning,dengyiwen,2022/9/27
   	UWORD xdata fileUseSpace,fileSpaceAddr,fileAddr,lastDFAddr;

   	fileAddr = G_CreatingMFOffset;

	fileUseSpace = GetFileHeadUsedSize(fileAddr);

    fileUseSpace = fileUseSpace + G_CreatingMFOffset;

    lastDFAddr = FALSE;

   	for(fileAddr = fileAddr+sizeof(FileSystemHead);fileAddr < fileUseSpace;fileAddr = fileAddr + fileSpaceAddr)
   	{

		fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

		if(fileType == ADFTYPE)
		{
			fileSpaceAddr = GetFileHeadUsedSize( fileAddr);

			lastDFAddr = fileAddr;
		}
//		else if(fileType == TRANTYPE)
//		{
//			tempParameter = GetFileHeadWord(fileAddr, F_SpaceSize_Offset);		//tempParameter as SpaceSize

//		 	fileSpaceAddr = tempParameter+sizeof(TranFileHead);
//		}
//		else if((fileType == LINTYPE)||(fileType == CYCTYPE))
//		{
//			ReadEeprom((BYTE  *)&recNumberLgth[0],fileAddr+F_RecNum_Offset,sizeof(recNumberLgth));

//			fileSpaceAddr = (UWORD)(recNumberLgth[0]*recNumberLgth[1])+sizeof(RecordFileHead) ;	//fileSpaceAddr as Space
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

	return lastDFAddr;
}
/*
 *@Function: getCycleFPoint
 *
 *@Descrip:  获取循环文件的记录指针
 *
 *@Param:    UWORD fileAddr --   文件地址
        
 *
 *@Return:   BYTE	cycleFPoint ---	  循环文件的记录指针
 *
 *@Author:   zhxx 20200221
 *
 */
BYTE getCycleFPoint(UWORD fileAddr)
{
   BYTE   idata cycleFPoint;
   UWORD  xdata fileAddrdes;
	
   fileAddrdes = getCycleFPointAddr(fileAddr);   //获取循环文件的 文件体地址

   cycleFPoint = ReadEECacheByte(fileAddrdes); //读取第一个字节  

   return cycleFPoint;
}

#if (EUICC_ONLY || SE_AND_EUICC)
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

  Function name :	 GetFileHeadByte
  
  Input parameter:   UWORD fileAddr
  					 BYTE operationType
					    					                                                                                                         
  Output parameter:  
                                                                                                                                                
  Description:       

  Coder:             CAIBO               Tester:   
  
  Status:	

*            modify by yjr@20211223 最高bit位用来表示文件即不允许被映射，也不允许被读 

 ------------------------------------------------------------------- ------------------------------------------------*/
BYTE GetFileHeadByte(UWORD fileAddr,BYTE operationType)
{
	BYTE idata type;

	type = ReadEECacheByte(fileAddr + operationType);

	if(operationType == F_Type_Offset)
	{
		if((type&FILETYPE)==0x38)
		{
			return DFTYPE;
		}
#if UICC__FUNCTION__BERTLVCmd
		else if((type&FILETYPE)== BERTLVTYPE)   //BER-TLV file
		{
			return 0x39;
		}
        #endif
		else
		{
		   return (type&0x07);
		}
	}
	
	if(operationType == F_ShordFid_Offset)
	{
		return (type&0x7F);//最高bit位用来表示文件即不允许被映射，也不允许被读 by yjr@20211223
	}

	return type;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetFileHeadUsedSize
  
  Input parameter:   UWORD fileAddr
  					 BYTE operationType
					    					                                                                                                         
  Output parameter:  
                                                                                                                                                
  Description:       

  Coder:             xiaoyh               Tester:   
  
  Status:			     

 ------------------------------------------------------------------- ------------------------------------------------*/

UWORD GetFileHeadUsedSize(UWORD fileAddr)
{
	  UWORD xdata usedSize;
	  
	  VMReadBuf((BYTE xdata *)&usedSize,fileAddr + F_UsedSize_Offset,sizeof(UWORD));

	  return usedSize;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetFileHeadWord
  
  Input parameter:   UWORD fileAddr
  					 BYTE operationType
					    					                                                                                                         
  Output parameter:  
                                                                                                                                                
  Description:       

  Coder:             chenl               Tester:   
  
  Status:			     

 ------------------------------------------------------------------- ------------------------------------------------*/

WORD GetFileHeadWord(UWORD fileAddr,BYTE operationType)
{
	WORD xdata type;
	
	ReadEeprom((BYTE xdata *)&type,fileAddr + operationType,sizeof(WORD));
	
	return type;		
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 FindLFAddr
  
  Input parameter:   UWORD fileAddr, BYTE index, BYTE offset, BYTE len    					                                                                                                         

  Output parameter:  addr /FALSE
                                                                                                                                                
  Description:       根据所给条件找到LF文件的偏移地址
  
  Status:			     

 -------------------------------------------------------------------------------------------------------------------*/ 
UWORD FindLFAddr(UWORD fileAddr, BYTE index, BYTE offset, BYTE len)
{
	BYTE  idata fileRecNumLgth[2];
	BYTE idata fileType;
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadEeprom((BYTE idata *)&fileRecNumLgth[0],fileAddr+F_RecNum_Offset,sizeof(fileRecNumLgth));

	ReadEeprom((BYTE *)&fileType,fileAddr+F_Type_Offset,sizeof(BYTE));

	if ((index == 0) || (index > fileRecNumLgth[0]) ||((offset + len) > fileRecNumLgth[1]))
	{
		return FALSE;
	}

	fileAddr = fileAddr +((index - 1)*(fileRecNumLgth[1]))+sizeof(RecordFileHead) + offset;

	if((fileType&0x37) == CYCTYPE)	//
	{
		fileAddr += 1;
	}

	return fileAddr;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SetLFConst
  
  Input parameter:   UWORD fileAddr, BYTE index, BYTE offset, BYTE  dataContext,BYTE lgth

  Output parameter:  TRUE/FALSE
                                                                                                                                                
  Description:       设置线形文件的一个记录的一定长度的值

  Coder:             CAIBO               Tester:
  
  status:			 OK	     

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE SetLFConst(UWORD fileAddr, BYTE index, BYTE offset, BYTE  dataContext,BYTE lgth)
{
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	if(IF_IsHeFlashFile(fileAddr))
	{
		return UpdateFileInHeFlash(fileAddr,index,offset,dataContext,NULL,lgth);		
	}
#endif
	fileAddr = FindLFAddr(fileAddr, index, offset, lgth);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	return SetEepromAtomic(fileAddr,dataContext,lgth);	 
} 

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 WriteLFBuf
  
  Input parameter:   UWORD fileAddr, BYTE index, BYTE offset, BYTE * buf, BYTE len

  Output parameter:  TRUE/FALSE
                                                                                                                                                
  Description:       Write data into a linear fixed record

  Coder:             CAIBO               Tester:
  
  Status:			 OK    

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE WriteLFBuf(UWORD fileAddr, BYTE index, BYTE offset, BYTE * buf, BYTE len)
{
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	if(IF_IsHeFlashFile(fileAddr))
	{
		return UpdateFileInHeFlash(fileAddr,index,offset,0,buf,len);		
	}
#endif
	fileAddr = FindLFAddr(fileAddr, index, offset, len);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	return WriteEepromAtomic(fileAddr,&buf[0],len);	 
}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ReadLFBuf
  
  Input parameter:   UWORD fileAddr, BYTE index, BYTE offset, BYTE * buf, BYTE len

  Output parameter:  TRUE/FALSE
                                                                                                                                              
  Description:       Read data from a linear fixed record

  Coder:             CAIBO               Tester:
  
  Status:			     

 -------------------------------------------------------------------------------------------------------------------*/    

BYTE ReadLFBuf(UWORD fileAddr, BYTE index, BYTE offset, BYTE * buf, BYTE len)
{
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	WORD  idata efId;
	BYTE  idata recNo,fileType;
	BYTE  idata fileRecNumLgth[2];

	if(IF_IsHeFlashFile(fileAddr))
	{
		fileType = GetFileHeadByte(fileAddr, F_Type_Offset);
		
		ReadEeprom((BYTE idata *)&fileRecNumLgth[0],fileAddr+F_RecNum_Offset,sizeof(fileRecNumLgth));

		if ((index == 0) || (index > fileRecNumLgth[0]) ||((offset + len) > fileRecNumLgth[1]))
		{
			return FALSE;
		}
	
		ReadEeprom((BYTE *)&efId,fileAddr,sizeof(WORD));	
		recNo = GetHeFlashRecordNoByFileId(GetFileHeadByte(fileAddr, F_Pointer_Offset),efId);

		if(recNo == FALSE)
		{
			return FALSE;
		}

		//fileAddr = (HE_FLASH_BASE+(recNo-1)*PFPageSize-EEPROM_BASE)+((index - 1)*(fileRecNumLgth[1]))+offset;
		fileAddr = (HE_FLASH_BASE+(recNo-1)*PFPageSize)+((index - 1)*(fileRecNumLgth[1]))+offset;

		if(fileType == CYCTYPE)	//循环文件，需要将记录指针字节偏移过去
		{
		   	fileAddr ++;
		}
	}
	else
#endif
	{
	fileAddr = FindLFAddr(fileAddr, index, offset, len);	
	}

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	ReadEeprom(&buf[0],fileAddr,len);
    
	return TRUE ;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 FindTFAddr
  
  Input parameter:   UWORD fileAddr, WORD offset, BYTE lgth

  Output parameter:  BYTE Addr/FALSE
                                                                                                                                                
  Description:       根据所给条件找到TF文件的偏移地址
  
  Status:			     

 -------------------------------------------------------------------------------------------------------------------*/
UWORD FindTFAddr(UWORD fileAddr, WORD offset, WORD lgth)
{
	WORD  idata fileSize;
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	WORD  idata efId;
	BYTE  idata recNo;	
#endif	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	fileSize = GetFileHeadWord(fileAddr,F_SpaceSize_Offset);

	if ((offset+lgth)> fileSize) 
	{
		return FALSE;
	}
	//考虑强更新文件的情况,by xwh 20220623
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	if(IF_IsHeFlashFile(fileAddr))
	{			
		ReadEeprom((BYTE *)&efId,fileAddr,sizeof(WORD));	
		recNo = GetHeFlashRecordNoByFileId(GetFileHeadByte(fileAddr, F_Pointer_Offset),efId);

		if(recNo == FALSE)
		{
			return FALSE;
		}

		//fileAddr = (HE_FLASH_BASE+(recNo-1)*PFPageSize-EEPROM_BASE)+offset;
		fileAddr = (HE_FLASH_BASE+(recNo-1)*PFPageSize)+offset;
	}
	else
#endif
	{		
		fileAddr = fileAddr + sizeof(TranFileHead) + offset;
	}
	return fileAddr;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SetTFContent
  
  Input parameter:   UWORD fileAddr, WORD offset, BYTE ch

  Output parameter:  BYTE TRUE/FALSE
                                                                                                                                                
  Description:       Write a Byte into a TF
  
  Status:			 OK    

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE SetTFConst(UWORD fileAddr, WORD offset, BYTE ch,WORD lgth)
{
#if High_Update_InRAM_New
	if(IF_UpdateWithRAM_New(fileAddr)!= FALSE)
	{
		return IF_UpdateOrReadWithRAM_New(fileAddr,offset, (BYTE *)&ch, lgth,Hige_SetRAM_New);
	}
	
#endif
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	if(IF_IsHeFlashFile(fileAddr))
	{
		return UpdateFileInHeFlash(fileAddr,(offset>>8)&0xFF,offset&0xFF,ch,NULL,lgth);		
	}
#endif
	fileAddr = FindTFAddr(fileAddr, offset, lgth);	

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	return SetEepromAtomic(fileAddr,ch,lgth);	 
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 WriteTFBuf
  
  input parameter:   UWORD fileAddr, WORD offset, BYTE * buf, BYTE len

  output parameter:  TRUE/FASLE
                                                                                                                                                
  description:       Write Data into a transparent file
  
  status:			     

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE WriteTFBuf(UWORD fileAddr, WORD offset, BYTE * buf, WORD len)
{
#if High_Update_InRAM_New
	if(IF_UpdateWithRAM_New(fileAddr)!= FALSE)
	{
		return IF_UpdateOrReadWithRAM_New(fileAddr,offset, buf, len,Hige_UpdateRAM_New);
	}
	
#endif
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	if(IF_IsHeFlashFile(fileAddr))
	{
		return UpdateFileInHeFlash(fileAddr,(offset>>8)&0xFF,offset&0xFF,0,buf,len);		
	}
#endif
	fileAddr = FindTFAddr(fileAddr, offset, len);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	return WriteEepromAtomic(fileAddr,&buf[0],len);	 
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ReadTFBuf
  
  input parameter:   UWORD fileAddr, WORD offset, BYTE * buf, BYTE len

  output parameter:  TRUE/FASLE
                                                                                                                                                
  description:       Read Data From a transparent file
  
  status:			     

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE ReadTFBuf(UWORD fileAddr, WORD offset, BYTE * buf, WORD len)
{
//#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
//	WORD  idata efId;
//	BYTE  idata recNo;
//	WORD  idata fileSize;

#if High_Update_InRAM_New
	if(IF_UpdateWithRAM_New(fileAddr)!= FALSE)
	{
		return IF_UpdateOrReadWithRAM_New(fileAddr,offset, buf, len,Hige_ReadRAM_New);
	}
	
#endif
//	if(IF_IsHeFlashFile(fileAddr))
//	{
//		fileSize = GetFileHeadWord(fileAddr,F_SpaceSize_Offset);

//		if ((offset+len)> fileSize) 
//		{
//			return FALSE;
//		}
//	
//		ReadEeprom((BYTE *)&efId,fileAddr,sizeof(WORD));	
//		recNo = GetHeFlashRecordNoByFileId(GetFileHeadByte(fileAddr, F_Pointer_Offset),efId);

//		if(recNo == FALSE)
//		{
//			return FALSE;
//		}

//		//fileAddr = (HE_FLASH_BASE+(recNo-1)*PFPageSize-EEPROM_BASE)+offset;
//		fileAddr = (HE_FLASH_BASE+(recNo-1)*PFPageSize)+offset;
//	}
//	else
//#endif
	{
		fileAddr = FindTFAddr(fileAddr, offset, len);
	}

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
    
    ReadEeprom(&buf[0],fileAddr,len);
	
	return TRUE ;	
}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetTFSize
  
  input parameter:   UWORD fileAddr

  output parameter:  Get file size of a transparent file
                                                                                                                                                
  description:       
  
  status:			OK		     

 -------------------------------------------------------------------------------------------------------------------*/    
WORD GetTFSize(UWORD fileAddr)
{
	return GetFileHeadWord(fileAddr, F_SpaceSize_Offset);
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SelectLFFile
  
  input parameter:   WORD dfName, DF file id

					 WORD efName, EF file id    					                                                                                                         

  output parameter:  addr /FALSE
                                                                                                                                                
  description:       在dfName下查找文件efName,如果找到,返回文件的地址

  Coder:		xiaoyh			Tester:   
  
  status:			     

 ------------------------------------------------------------------- ------------------------------------------------*/    
UWORD SelectLFFile(WORD dfName, WORD efName)
{
	UWORD xdata fileAddr;

	fileAddr = SelectFile(E_FILE_START,dfName) ;

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	fileAddr = SelectFile(fileAddr, efName);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	return fileAddr;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 WriteTFBuf
  
  input parameter:   UWORD fileAddr, WORD offset, BYTE * buf, BYTE len

  output parameter:  TRUE/FASLE
                                                                                                                                                
  description:       Write Data into a transparent file
  
  status:			     

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE WriteTFFileNonAtomic(UWORD fileAddr, WORD offset, UWORD srcAddr, WORD len)
{
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	if(IF_IsHeFlashFile(fileAddr))
	{
		return UpdateFileInHeFlash(fileAddr,(offset>>8)&0xFF,offset&0xFF,0,(BYTE *)srcAddr,len);		
	}
#endif	
	
	fileAddr = FindTFAddr(fileAddr, offset, len);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	return WriteEepromNonAtomic(fileAddr,(BYTE *)srcAddr,len);	   
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	GetParentOrMapFileAddr                                                                                                                   
                                                                                                                                                
  input parameter:	UWORD fileAddr 
  
  					BYTE offset:	F_ParentName_Offset
									F_MapFile_Offset                                          
  					                                                                                                         
  output parameter:	UWORD fileAddr                                                                                                           
                                                                                                                                                
  description:		
  
  status:			OK         

 -------------------------------------------------------------------------------------------------------------------*/ 
UWORD GetPrivateParentOrMapFileAddr (UWORD fileAddr,BYTE offset)
{
	WORD xdata fileName[3];

	ReadEeprom((BYTE *)&fileName[0],fileAddr + offset,sizeof(fileName)); 

	fileAddr = SearchFile(ADDR_PRIVATE_FILE_START,fileName[0]);

	if(fileAddr != FALSE)
	{
		fileAddr = SearchFile(fileAddr,fileName[1]);					  	
	}

	if(offset == F_MapFile_Offset)
	{
		if(fileAddr != FALSE)
		{
			fileAddr = SearchFile(fileAddr,fileName[2]);					  	
		}
	}

	return fileAddr;
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
UWORD SelectPrivateFile(UWORD  fileAddr,WORD fileName)
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
		//#if UICC__FUNCTION__DFLink
		
	 	// return SelctLinkDF(fileAddr);
		//#else
		 return fileAddr;
		//#endif
	}

	//G_CurrentSfi = GetFileHeadByte(fileAddr,F_ShordFid_Offset);					//Add at 2007-02-07

	fileType = GetFileHeadByte(fileAddr,F_Status_Offset);

	if((fileType&F_MAP_FILE) == F_MAP_FILE)
	{
		fileAddr = GetPrivateParentOrMapFileAddr(fileAddr, F_MapFile_Offset);			
	}

	return fileAddr ;
	
}
UWORD SelectPrivateLFFile(WORD dfName, WORD efName)
{
	UWORD xdata fileAddr;

	fileAddr = SelectPrivateFile(ADDR_PRIVATE_FILE_START,dfName) ;

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	fileAddr = SelectPrivateFile(fileAddr, efName);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	return fileAddr;
}
#if (EUICC_ONLY||SE_AND_EUICC)
/*@Function:    SelectProcessingProfileLFFile
 *
 *@Descrip:     选择当前正在下载的Profile（不是当前enable的Profile）中的文件
 *
 *@See:	        ts_102222
 *
 *@Param:       dfName   ---> DF ID
 *@param        efName   ---> EF ID
 *
 *@Return:      void
 *
 *@Author:      XWH 20170724
 *
 *@History:     V1.0---Create by XWH 20170724
 */
UWORD SelectProcessingProfileLFFile(WORD dfName, WORD efName)
{
	UWORD xdata fileAddr;

	fileAddr = SelectFile(G_CreatingMFOffset,dfName) ;

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	fileAddr = SelectFile(fileAddr, efName);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	return fileAddr;
}

/*@Function:    SelectLFFileWithMfStart                                           
 * 
 *@Descrip:     选择指定mfStart 下的文件
 *
 *@See:	        ts_102222
 *                                                                                                                                     
 *@Param:       dfName   ---> DF ID
 *@param        efName   ---> EF ID
 *				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                            
 * 
 *@Author:      XWH 20170724
 *  
 *@History:     V1.0---Create by XWH 20190318             
 */ 

UWORD SelectLFFileWithMfStart(UWORD mfStart,WORD dfName, WORD efName)
{
	UWORD xdata fileAddr;
	
	fileAddr = SelectFile(mfStart,dfName) ;

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	fileAddr = SelectFile(fileAddr, efName);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	return fileAddr;
}
#endif
/*
*@description:清空文件
*
*@param      :recNum:记录号
*
*@return     :void
*
*@history    :create by xyz@100513 8:59
*/
void ClearFileConst(UWORD fileAddr,BYTE hex)
{
	WORD  idata fileSize;

	if(fileAddr == FALSE)
	{
		return;
	}
	
	fileSize = GetFileTotalSize(fileAddr);

	SetEepromNonAtomic(fileAddr+ sizeof(TranFileHead),hex,fileSize-sizeof(TranFileHead));
}

#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
BYTE IF_IsHeFlashFile(UWORD addr)
{
	BYTE  idata num;

	num = GetFileHeadByte(addr, F_Pointer_Offset);
	
	if((num>=1)&&(num<=MaxNum_HeFlash_Files))
	{	
	    return TRUE;		
	}

	return FALSE;		
}
BYTE IF_UpdateCycleRecPointer(UWORD addr)
{
	BYTE  idata fileType;

	fileType = GetFileHeadByte(addr, F_Type_Offset);
       
	if((P2 != ABSOLUTE_CURRENT) && ((fileType & 0x07) == CYCTYPE)&&(IF_IsHeFlashFile(addr) == TRUE))//只有循环文件才会更新循环记录指针
	{	
	    return TRUE;		
	}

	 return FALSE;		
}
UWORD GetHeFlashFileDataAddr(UWORD addr)
{
	BYTE  idata num;
	WORD idata efId;

	if(IF_IsHeFlashFile(addr))
	{
		ReadEeprom((BYTE *)&efId,addr,sizeof(WORD));	
		num = GetHeFlashRecordNoByFileId(GetFileHeadByte(addr, F_Pointer_Offset),efId);
	
		//return (HE_FLASH_BASE+(num-1)*PFPageSize-EEPROM_BASE);	
		return (HE_FLASH_BASE+(num-1)*PFPageSize);			
	}
	else
	{
		return (addr+sizeof(RecordFileHead));
	}
}

BYTE IsFileNameInvalid(WORD fileName)
{
	return (((fileName == 0x0000)||(fileName == 0xFFFF))?TRUE:FALSE);
}


BYTE IsBufferInitValue(BYTE * buffer,BYTE initValue)
{
	WORD  idata i;

	for(i=0; i<PFPageSize; i++)
	{
		if(buffer[i] != initValue)	
		{
			return FALSE;
		}
	}

	return TRUE;
}

//modify by xuweihua 20211117,支持创建14个强更新文件
//文件分两组，文件1~7 创建在前8条记录, 文件8-14 创建在后8条记录
void SetSerchStartAndEnd(BYTE fileNo,BYTE * serchStart,BYTE * serchEnd)
{
	/*
	//文件1 ~ 4 分配在记录 1 ~ 5
	if(fileNo <= 4)
	{
		*serchStart = 1;
		*serchEnd = 5;
	}
	//文件5 ~ 8 分配在记录 6 ~ 10
	else if(fileNo <= 8)
	{
		*serchStart = 6;
		*serchEnd = 10;
	}
	//文件9 ~ 13 分配在记录 11 ~ 16
	else
	{
		*serchStart = 11;
		*serchEnd = 16;
	}
	*/
	//文件1 ~ 7 分配在记录 1 ~ 8
	if(fileNo <= 7)
	{
		*serchStart = 1;
		*serchEnd = 8;
	}
	//文件8 ~ 14 分配在记录 9 ~ 16
	else //if(fileNo <= 14)
	{
		*serchStart = 9;
		*serchEnd = 16;
	}
}
/*
*@function      :GetHeFlashUsableRecordNo
*
*@description :在HE FLASH区查找一条可用的记录(无文件占用)
*
*@param       : fileNo : 强更新文件序号，1 ~ 13 (按照创建文件顺序确定)               

*@return       :num :可用的记录号(0x01 ~ 0x10)
*
*@author       :xwh 20200930
*
*@history      :create by xwh 20200930
*/
BYTE GetHeFlashUsableRecordNo(BYTE fileNo)
{
	BYTE  idata matchNum,i,loop,curFileNo;
	BYTE  xdata recordNo[MaxNum_HeFlash_Group],fileNoBuf[MaxNum_HeFlash_Group];	
	WORD xdata fileEfID[MaxNum_HeFlash_Group];
	UWORD xdata counter[MaxNum_HeFlash_Group];
	WORD idata checkCrc,calcCrc,curEfName;
	UWORD idata curCounter;
	BYTE  idata serchStart,serchEnd;
	BYTE  idata RamBuffer[PFPageSize];

	matchNum = 0x00;

	SetSerchStartAndEnd(fileNo,&serchStart,&serchEnd);

	for(i=serchStart; i<=serchEnd; i++)
	{
		//该记录号已经确定有文件，因此不需要再检查，提高遍历效率
		//if(i == skipFileNo)
		//{
		//	continue;
		//}
	
		ecpMem_Copy(RamBuffer, (unsigned char *)(HE_FLASH_BASE+(i-1)*PFPageSize), PFPageSize); //read the record

		checkCrc = BuildWord(&RamBuffer[RamBuffer_CrcOffset]);
		curFileNo = RamBuffer[RamBuffer_FileNoOffset];
		curEfName = BuildWord(&RamBuffer[RamBuffer_EfNameOffset]);
		curCounter = BuildUWord(&RamBuffer[RamBuffer_CounterOffset]);

        calcCrc = Hardware_CalCRC((u8 *)&RamBuffer[0],PFPageSize-2);

		//没有文件占用的记录
		if((checkCrc != calcCrc)||(IsFileNameInvalid(curEfName)))
		{
			return i;
		}
		//已被文件占用的记录
		else
		{
			for(loop=0; loop<matchNum; loop++)
			{
				//找到重复的记录，counter 值小的记录是空记录
				if((curFileNo == fileNoBuf[loop])&&(curEfName == fileEfID[loop]))
				{
					return ((curCounter > counter[loop])?recordNo[loop]:i);
				}
			}

			//没有找到重复的记录，将当前记录信息存入数组中
			recordNo[matchNum] = i;
			fileNoBuf[matchNum] = curFileNo;
			fileEfID[matchNum] = curEfName;
			counter[matchNum] = curCounter;
			matchNum++;								
		}
	}

	return FALSE;	
}

/*
*@function      :GetHeFlashRecordNoByFileId
*
*@description :在HE FLASH区查找fileId 指定的文件占用的记录(如果有几条匹配的记录，选择counter 值最大的记录)
*
*@param       : fileId : ef 文件ID

*@return       :num :可用的记录号(0x01 ~ 0x10)
*
*@author       :xwh 20200930
*
*@history      :create by xwh 20200930
*/
BYTE GetHeFlashRecordNoByFileId(BYTE fileNo,WORD fileId)
{
	BYTE  idata matchNum,i,findRecordNo,curFileNo;
	BYTE  xdata recordNo[16];
	UWORD xdata recordCounter[16];
	WORD idata checkCrc,calcCrc,curEfName;
	UWORD idata maxCounter;
	BYTE  idata serchStart,serchEnd;
	BYTE  idata RamBuffer[PFPageSize];
	
	matchNum = 0x00;

	SetSerchStartAndEnd(fileNo,&serchStart,&serchEnd);

	for(i=serchStart; i<=serchEnd; i++)
	{
		ecpMem_Copy(RamBuffer, (unsigned char *)(HE_FLASH_BASE+(i-1)*PFPageSize), PFPageSize); //read the record
		
		checkCrc = BuildWord(&RamBuffer[RamBuffer_CrcOffset]);
		
        calcCrc = Hardware_CalCRC((u8 *)&RamBuffer[0],PFPageSize-2);

		curFileNo = RamBuffer[RamBuffer_FileNoOffset];
		curEfName = BuildWord(&RamBuffer[RamBuffer_EfNameOffset]);

		//文件ID 匹配的记录
		if((checkCrc == calcCrc)&&(curFileNo == fileNo)&&(curEfName==fileId))
		{
			recordNo[matchNum] = i;
			recordCounter[matchNum] = BuildUWord(&RamBuffer[RamBuffer_CounterOffset]);
			matchNum ++;
		}
	}

	//没找到可用记录，这种情况不允许出现
	if(matchNum == 0)
	{
		//在弱写的情况下可能出现文件体找不到的情况，这种情况下找到一条空闲记录，重新初始化文件体，避免文件丢失的情况, by xwh 20210914
		findRecordNo = GetHeFlashUsableRecordNo(fileNo);

		if(findRecordNo == FALSE)
		{
			//return FALSE;
			//如果出现文件被全部占满的情况，随机指定一条记录重新初始化文件体, by xwh 20210914
			Random_Number_Generator_Get(&RamBuffer[0],1);
			/*
			//文件1 ~ 4 分配在记录 1 ~ 5
			if(fileNo <= 4)
			{
				findRecordNo = ((RamBuffer[0]%5)+1);
			}
			//文件5 ~ 8 分配在记录 6 ~ 10
			else if(fileNo <= 8)
			{
				findRecordNo = ((RamBuffer[0]%5)+6);
			}
			//文件9 ~ 13 分配在记录 11 ~ 16
			else
			{
				findRecordNo = ((RamBuffer[0]%6)+11);
			}
			*/
			//文件1 ~ 7 分配在记录 1 ~ 8
			if(fileNo <= 7)
			{
				findRecordNo = ((RamBuffer[0]%8)+1);
			}
			//文件5 ~ 8 分配在记录 9 ~ 16
			else //if(fileNo <= 14)
			{
				findRecordNo = ((RamBuffer[0]%8)+9);
			}			
		}

		ecpMem_Set(RamBuffer, 0xFF, PFPageSize); //set init value to "FF"

		SetUWord(&RamBuffer[RamBuffer_CounterOffset],0x00000000);        //set the init counter to 0			
		SetWord(&RamBuffer[RamBuffer_EfNameOffset],fileId);  
		RamBuffer[RamBuffer_FileNoOffset] = fileNo;

		calcCrc = Hardware_CalCRC((u8 *)&RamBuffer[0],PFPageSize-2);    
		SetWord(&RamBuffer[RamBuffer_CrcOffset],calcCrc);  
			
		WriteHEFlashPage(HE_FLASH_BASE+(findRecordNo-1)*PFPageSize,RamBuffer);
	
		return findRecordNo;
	}

	findRecordNo = recordNo[0];
	maxCounter = recordCounter[0];

	for(i=1; i<matchNum; i++)
	{
		if(recordCounter[i] > maxCounter)
		{
			findRecordNo = recordNo[i];
			maxCounter = recordCounter[i];
		}
	}

	return findRecordNo;
}

BYTE UpdateFileInHeFlash(UWORD fileAddr,BYTE para1,BYTE para2,BYTE hex,BYTE * buf,BYTE len)
{
	BYTE  idata fileType,fileNo;
	BYTE  idata fileRecNumLgth[2],index;
	WORD idata offset;
	WORD  idata fileSize;
	UWORD idata curCounter;
	WORD  idata efId,calcCrc;
	BYTE  idata recNo,dstRecNo;	
	BYTE  idata RamBuffer[PFPageSize];
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);  
	fileNo = GetFileHeadByte(fileAddr, F_Pointer_Offset);
	
	index = 0;

	if(fileType == TRANTYPE)
	{
		offset = para1*0x100+para2;
	
		fileSize = GetFileHeadWord(fileAddr,F_SpaceSize_Offset);

		if ((offset+len)> fileSize) 
		{
			return FALSE;
		}
	}
	else
	{
		index = para1;
		offset = para2;
	
		ReadEeprom(&fileRecNumLgth[0],fileAddr+F_RecNum_Offset,sizeof(fileRecNumLgth));

		if((index == 0) || (index > fileRecNumLgth[0]) ||((offset + len) > fileRecNumLgth[1]))
		{
			return FALSE;
		}

		offset +=((index-1)*fileRecNumLgth[1]);

		//循环文件，需要将记录指针字节偏移过去
		if(fileType == CYCTYPE)
		{
			offset ++;
		}
	}
	
	ReadEeprom((BYTE *)&efId,fileAddr,sizeof(WORD));	
	recNo = GetHeFlashRecordNoByFileId(fileNo,efId);

	dstRecNo = GetHeFlashUsableRecordNo(fileNo);

	//if((recNo == FALSE)||(dstRecNo == FALSE))
	//{
	//	return FALSE;
	//}

	if(recNo == FALSE)
	{
		return FALSE;
	}

	if(dstRecNo == FALSE)
	{
		//return FALSE;
		//如果出现文件被全部占满的情况，随机指定一条记录使用, by xwh 20210914
		Random_Number_Generator_Get(&RamBuffer[0],1);
		/*
		//文件1 ~ 4 分配在记录 1 ~ 5
		if(fileNo <= 4)
		{
			dstRecNo = ((RamBuffer[0]%5)+1);
		}
		//文件5 ~ 8 分配在记录 6 ~ 10
		else if(fileNo <= 8)
		{
			dstRecNo = ((RamBuffer[0]%5)+6);
		}
		//文件9 ~ 13 分配在记录 11 ~ 16
		else
		{
			dstRecNo = ((RamBuffer[0]%6)+11);
		}
		*/
		//文件1 ~ 7 分配在记录 1 ~ 8
		if(fileNo <= 7)
		{
			dstRecNo = ((RamBuffer[0]%8)+1);
		}
		//文件8 ~ 14 分配在记录 9 ~ 16
		else //if(fileNo <= 14)
		{
			dstRecNo = ((RamBuffer[0]%8)+9);
		}		
	}
	ecpMem_Copy(RamBuffer,(unsigned char *)(HE_FLASH_BASE+(recNo-1)*PFPageSize),PFPageSize);

	//更新循环文件记录指针
	if((fileType == CYCTYPE)&&(G_UpdateCycleRecPointer == TRUE))
	{
		RamBuffer[0] = index;
	}

	if(buf == NULL)
	{
		ecpMem_Set(RamBuffer+offset,hex,len);
	}
	else
	{
		ecpMem_Copy(RamBuffer+offset,&buf[0],len);
	}

	curCounter = BuildUWord(&RamBuffer[RamBuffer_CounterOffset]);
	curCounter ++;
	SetUWord(&RamBuffer[RamBuffer_CounterOffset],curCounter);

	calcCrc = Hardware_CalCRC((u8 *)&RamBuffer[0],PFPageSize-2);

	SetWord(&RamBuffer[RamBuffer_CrcOffset],calcCrc);

	return WriteHEFlashPage(HE_FLASH_BASE+(dstRecNo-1)*PFPageSize,RamBuffer);	
}
/*
*@function      :CheckHeFlashRecordResumeSub
*
*@description   :上电时检查HE_FLASH_BASE区，将弱写记录清除或者实写
*
*@param         : none          

*@return        :none
*
*@author        :xwh 20200930
*
*@history       :create by xwh 20200930
*/
void CheckHeFlashRecordResumeSub(BYTE fileNo)
{
	BYTE  idata matchNum,i,loop,curFileNo;
	BYTE  xdata recordNo[MaxNum_HeFlash_Group],fileNoBuf[MaxNum_HeFlash_Group];	
	WORD xdata fileEfID[MaxNum_HeFlash_Group];
	UWORD xdata counter[MaxNum_HeFlash_Group];
	WORD idata checkCrc,calcCrc,curEfName;
	UWORD idata curCounter;
	BYTE  idata serchStart,serchEnd;
	BYTE  idata RamBuffer[PFPageSize];
		
	SetSerchStartAndEnd(fileNo,&serchStart,&serchEnd);

	//CRC 错误的，清除
	matchNum = 0;
	
	for(i=serchStart; i<=serchEnd; i++)
	{
		ecpMem_Copy(RamBuffer, (unsigned char *)(HE_FLASH_BASE+(i-1)*PFPageSize), PFPageSize); //read the record

		//记录的值是全00 (初始值)
		if(IsBufferInitValue(RamBuffer,0x00))
		{
			continue;
		}

		checkCrc = BuildWord(&RamBuffer[RamBuffer_CrcOffset]);
		curFileNo = RamBuffer[RamBuffer_FileNoOffset];
		curEfName = BuildWord(&RamBuffer[RamBuffer_EfNameOffset]);
		curCounter = BuildUWord(&RamBuffer[RamBuffer_CounterOffset]);
		
		calcCrc = Hardware_CalCRC((u8 *)&RamBuffer[0],PFPageSize-2);
        
		//CRC 错误的记录
		if(checkCrc != calcCrc)
		{			
			ecpMem_Set(RamBuffer,0x00, PFPageSize);
			WriteHEFlashPage(HE_FLASH_BASE+(i-1)*PFPageSize,RamBuffer);					
		}
		else
		{
			//有效的文件ID		
			for(loop=0; loop<matchNum; loop++)
			{
				//找到重复的记录，将counter 值大的记录实写
				if((curFileNo == fileNoBuf[loop])&&(curEfName == fileEfID[loop]))
				{
					if(curCounter > counter[loop])
					{
						WriteHEFlashPage(HE_FLASH_BASE+(i-1)*PFPageSize,RamBuffer);									
					}
					else
					{
						ecpMem_Copy(RamBuffer, (unsigned char *)(HE_FLASH_BASE+(recordNo[loop]-1)*PFPageSize), PFPageSize); //read the record
						WriteHEFlashPage(HE_FLASH_BASE+(recordNo[loop]-1)*PFPageSize,RamBuffer);		
					}

					return;						
				}
			}
								
			//将当前记录信息存入数组中
			recordNo[matchNum] = i;
			fileNoBuf[matchNum] = curFileNo;
			fileEfID[matchNum] = curEfName;
			counter[matchNum] = curCounter;
			matchNum++;
		}
		
	}				
}
/*
*@function      :CheckHeFlashRecordResume
*
*@description   :上电时检查HE_FLASH_BASE区，将弱写记录清除或者实写
*
*@param         : none          

*@return        :none
*
*@author        :xwh 20200930
*
*@history       :create by xwh 20200930
*/
void CheckHeFlashRecordResume(void)
{
	BYTE  idata num;
	BYTE  idata fileNo;
	
	for(num=1; num<=2; num++)
	{		
		if(num == 1)
		{
			fileNo = 1;	
		}		
		else
		{
			fileNo = 8;	
		}

		CheckHeFlashRecordResumeSub(fileNo);			
	}			
}
#endif
#if High_Update_InRAM_New
/**
 * @brief 		InitUpdateInRAMVariable 
 * 
 * @param      
 *
 * @Returns   	
 *
 * @author 		LCZ
 *
 * @history 	created by LCZ 20200321
 */
void InitUpdateInRAMVariable(void)
{
#if High_Update_InRAM_New	
	ecpMem_Set(G_High_Update_InRAM_new, 0x00,sizeof(G_High_Update_InRAM_new));
#endif		
}
/**
 * @brief 		IF_UpdateWithRAM 判断强更新文件是否在RAM的 好判断时候执行写操作
 * 
 * @param       UWORD fileAddr  文件体地址
 *
 * @Returns   	TRUE  RAM/ FALSE E
 *
 * @author 		LCZ
 *
 * @history 	created by LCZ 20200321
 */
BYTE IF_UpdateWithRAM_New(UWORD fileAddr)
{
	BYTE idata updatetag;
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	updatetag = GetFileHeadByte(fileAddr, F_Pointer_Offset);
	
	if((updatetag & Hige_UpdateWithRAM_New) ==Hige_UpdateWithRAM_New)
	{
		return updatetag;
	}

    return FALSE;	
}
/**
 * @brief 		IF_UpdateOrReadWithRAM 强更新在RAM的文件的读写
 * 
 * @param       UWORD fileAddr  文件体地址
 *
 * @Returns   	TRUE  已经执行完读写/ FALSE  不是强更新在RAM里面/0x02 强更新RAM里面 但是执行出错
 *
 * @author 		LCZ
 *
 * @history 	created by LCZ 20200321
 */
BYTE IF_UpdateOrReadWithRAM_New(UWORD fileAddr,WORD offset, BYTE * buf, WORD len,BYTE mode)
{
	WORD  idata fileSize;
	BYTE  idata updatetag;
	BYTE  idata indexoffset;
	
	fileSize = GetFileHeadWord(fileAddr,F_SpaceSize_Offset);

	if ((offset+len)> fileSize) //出错
	{
		return FALSE;
	}
	
	updatetag = GetFileHeadByte(fileAddr, F_Pointer_Offset);
	
	indexoffset = FindTag(&G_High_Update_InRAM_new[0],updatetag,sizeof(G_High_Update_InRAM_new)); //(updatetag & Hige_UpdateWithEep)  bit7 代表强更新  其余位 是索引
	
	if((indexoffset == FALSE) )   //第一次读写 
	{		
		for(;indexoffset<sizeof(G_High_Update_InRAM_new);) //缓存TLV格式(T 为index)
		{
			if((G_High_Update_InRAM_new[indexoffset] == 0x00)&&(G_High_Update_InRAM_new[indexoffset+1] == 0x00))
			{
				break;     //空闲的位置
			}
			else
			{
				indexoffset += (G_High_Update_InRAM_new[indexoffset+1] +2);
			}
		}
		
		G_High_Update_InRAM_new[indexoffset]   =  updatetag; //G_High_Update_RAM 大小应该是足够的  创建文件的时候做判断
		
		G_High_Update_InRAM_new[indexoffset+1] =  (fileSize & 0xFF);
		
// 		if(checkCacheAddr(fileAddr+sizeof(TranFileHead),fileSize) == FALSE) //整个文件读到RAM里面
		{
			ReadEeprom(&G_High_Update_InRAM_new[indexoffset+2],fileAddr+sizeof(TranFileHead),fileSize);
		}
// 		else
// 		{
// 			ReadEEPromCache(&G_High_Update_RAM[offset+2],fileAddr+sizeof(TranFileHead),fileSize);
// 		}
		
		indexoffset ++ ;//执行L的位置  和 FindTag  匹配
	}

	
	if(mode == Hige_ReadRAM_New)
	{
		ecpMem_Copy(buf, &G_High_Update_InRAM_new[indexoffset+1+offset], len);
	}
	else if(mode == Hige_SetRAM_New)
	{
		ecpMem_Set(&G_High_Update_InRAM_new[indexoffset+1+offset], buf[0], len);
	}
	else
	{
		ecpMem_Copy(&G_High_Update_InRAM_new[indexoffset+1+offset], buf, len);
	}

	return TRUE;

}
#endif
