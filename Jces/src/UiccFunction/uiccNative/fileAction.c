/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
			V1.1 修改函数GetFileHeadByte，增加BER-TLV类型的判断（1.19 -> 1.20）

 */
#include  "typedef.h"
#include  "uicc_main.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include "fileAction.h"
#include  "interface.h"
//#include "Transaction.h"
#include  "eepromdefine.h"
#include "Interface_LTEFromBoot.h"



UWORD getCycleFPointAddr(UWORD fileAddr)
{	
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	return GetHeFlashFileDataAddr(fileAddr);		
#else
	getFileContentAddr(fileAddr);
#endif	
}



/*
 *@Function: getCycleFPoint
 *
 *@Descrip:  根据循环文件的指针号，获取循环文件的实际记录指针     
 *
 *@Param:    BYTE pointNum --   循环文件指针号
        
 *
 *@Return:   BYTE	cycleFPoint ---	  循环文件的实际记录指针 
 *
 *@Author:   zhxx 20200221
 *
 */
BYTE getCycleFPoint(UWORD fileAddr)
{
   BYTE idata cycleFPoint;
 
#if !HEAD_BODY_TOGETHER	
   ReadEeprom(&cycleFPoint,getFileContentAddr(fileAddr),1);
#else
	 cycleFPoint = ReadEECacheByte(getCycleFPointAddr(fileAddr)); 
#endif

   return cycleFPoint;
}


/*
 *@Function: GetFileHeadUsedSize
 *
 *@Descrip:  获取DF文件的usedsize     
 *
 *@Param:    UWORD fileAddr --   DF文件地址
        
 *
 *@Return:   UWORD	usedSize --   DF文件的usedsize
 *
 *@Author:   zhxx 20200226
 *
 */
UWORD GetFileHeadUsedSize(UWORD fileAddr)
{
	  UWORD xdata usedSize;
	
#if	!HEAD_BODY_TOGETHER
	  
	  ReadEeprom((BYTE xdata *)&usedSize,fileAddr + F_UsedHeadSize_Offset,sizeof(UWORD));
#else
	   VMReadBuf((BYTE xdata *)&usedSize,fileAddr + F_UsedSize_Offset,sizeof(UWORD));
	#endif

	  return usedSize;
}
 /*
 *@Function: getFileContentAddr
 *
 *@Descrip:  根据文件头的地址获取文件体的地址
 *
 *@Param:    UWORD  fileAddr	 -- 文件头地址


 *
 *@Return:   destValue
 *
 *@Author:   zhxx 20200219
 *
 */
UWORD getFileContentAddr(UWORD  fileAddr)
{
	WORD  idata FileContentRef;	
	BYTE idata fileType;
	
	UWORD xdata destValue;
	
#if HEAD_BODY_TOGETHER
	destValue  = fileAddr + sizeof(EFileHead);
#else
	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

	if(fileType == DFTYPE)
	{
	    ReadEeprom((BYTE *)&FileContentRef,fileAddr+F_DFContent_Offset,sizeof(WORD));	//读取文件头中文件体的引用
	}
	else
	{
	  	ReadEeprom((BYTE *)&FileContentRef,fileAddr+F_Content_Offset,sizeof(WORD));	//读取文件头中文件体的引用
	}
	
	if(FileContentRef == REF_NULL)
	{
	   return FALSE;
	}
		
	destValue = VMReadUword(E_ADDR_INDEX_TABLE + (FileContentRef-1)*sizeof(UWORD));	 //根据引用获取相应的地址

	if(destValue == 0xFFFFFFFF)
	{
	    return FALSE;
	}
		
  if(fileType!=DFTYPE)
	{		 
		destValue = IF_GetHigtUpdateAddr(destValue,fileAddr);
	}
#endif
	
	return destValue;
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
UWORD GetParentOrMapFileAddr (UWORD fileAddr,BYTE offset)
{
	WORD  idata fileName[3];

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

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetFileHeadByte
  
  Input parameter:   UWORD fileAddr
  					 BYTE operationType
					    					                                                                                                         
  Output parameter:  
                                                                                                                                                
  Description:       

  Coder:             CAIBO               Tester:   
  
  Status:			     

 ------------------------------------------------------------------- ------------------------------------------------*/
BYTE GetFileHeadByte(UWORD fileAddr,BYTE operationType)
{
  BYTE idata type;

  ReadEeprom((BYTE *)&type,fileAddr + operationType,sizeof(BYTE));

  if(operationType == F_Type_Offset)
  {
	  	if((type&FILETYPE)==0x38)
	  	{
			return DFTYPE;
	  	}
#if UICC__FUNCTION__BERTLVCmd
		else if((type&FILETYPE)== 0x39)   //BER-TLV file
		{
			return 0x39;
		}
        #endif
	  	else
	  	{
	   	   return (type&0x07);
	  	}
   }
   
   
#if UICC_CreatFile_93Tag   							//增加使用'93'tag定义不能被读和映射的文件
   if(operationType == F_ShordFid_Offset)
   {
		return (type&0x7F);  //最高bit位用来表示文件即不允许被映射,也不允许被读
   }
#endif   

   return type;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetFileHeadUword
  
  Input parameter:   UWORD fileAddr
  					 BYTE operationType
					    					                                                                                                         
  Output parameter:  
                                                                                                                                                
  Description:       

  Coder:             xiaoyh               Tester:   
  
  Status:			     

 ------------------------------------------------------------------- ------------------------------------------------*/

UWORD GetFileHeadUword(UWORD fileAddr,BYTE operationType)
{
	  UWORD xdata buf;
	  
	  ReadEeprom((BYTE *)&buf,fileAddr + operationType,sizeof(UWORD));

	  return buf;
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
	WORD idata type;
	
	ReadEeprom((BYTE *)&type,fileAddr + operationType,sizeof(WORD));
	
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
	BYTE  idata fileRecNumLgth[2],fileType;
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadEeprom((BYTE *)&fileRecNumLgth[0],fileAddr+F_RecNum_Offset,sizeof(fileRecNumLgth));

	ReadEeprom((BYTE *)&fileType,fileAddr+F_Type_Offset,sizeof(BYTE));

	if ((index == 0) || (index > fileRecNumLgth[0]) ||((offset + len) > fileRecNumLgth[1]))
	{
		return FALSE;
	}

	#if HEAD_BODY_TOGETHER
		fileAddr = fileAddr +((index - 1)*(fileRecNumLgth[1]))+sizeof(EFileHead) + offset;
	#else
	  fileAddr = getFileContentAddr(fileAddr) +((index - 1)*(fileRecNumLgth[1]))+ offset;
	#endif
	
	

	if((fileType&0x37) == CYCTYPE)	//循环文件，需要将记录指针字节偏移过去
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
	fileAddr = FindLFAddr(fileAddr, index, offset, lgth);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	//SetEepromEX(fileAddr,dataContext,lgth);
	return SetEepromAtomic(fileAddr,dataContext,lgth);	 

	return TRUE ;
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
	fileAddr = FindLFAddr(fileAddr, index, offset, len);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	//return WriteEepromEX(fileAddr,&buf[0],len);
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
	fileAddr = FindLFAddr(fileAddr, index, offset, len);

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
UWORD FindTFAddr(UWORD fileAddr, WORD offset, BYTE lgth)
{
	WORD  idata fileSize;
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	fileSize = GetFileHeadWord(fileAddr,F_SpaceSize_Offset);

	if ((offset+lgth)> fileSize) 
	{
		return FALSE;
	}

	#if HEAD_BODY_TOGETHER
	  fileAddr = fileAddr + sizeof(EFileHead) + offset;
	#else
	  fileAddr = getFileContentAddr(fileAddr) + offset;
	#endif
	

	return fileAddr;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SetTFContent
  
  Input parameter:   UWORD fileAddr, WORD offset, BYTE ch

  Output parameter:  BYTE TRUE/FALSE
                                                                                                                                                
  Description:       Write a Byte into a TF
  
  Status:			 OK    

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE SetTFConst(UWORD fileAddr, WORD offset, BYTE ch,BYTE lgth)
{
	BYTE idata isSmallblock;
	BYTE idata mode,buf[1];
	UWORD headAddr;
	
	buf[0] = ch;
	
	mode = IF_UpdateOrReadWithRAM(fileAddr,offset,buf,lgth,Hige_Set_mode);
	
	if(mode != 0x02)
	{
		return mode;
	}
	
	headAddr = fileAddr;
	
	isSmallblock = IF_TFUpdate_Smallblock(fileAddr);  //不能移动此函数位置  fileAddr现在是文件头地址
	
	fileAddr = FindTFAddr(fileAddr, offset, lgth);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	if(isSmallblock == TRUE) //如果是轮询方式的 更新
	{
		return IF_Update_SmallBlock( fileAddr,headAddr,buf,offset,lgth,Hige_Set_mode);
	}

	//SetEepromEX(fileAddr,ch,lgth);
	return SetEepromAtomic(fileAddr,ch,lgth);	 

	return TRUE ;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 WriteTFBuf
  
  input parameter:   UWORD fileAddr, WORD offset, BYTE * buf, BYTE len

  output parameter:  TRUE/FASLE
                                                                                                                                                
  description:       Write Data into a transparent file
  
  status:			     

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE WriteTFBuf(UWORD fileAddr, WORD offset, BYTE * buf, BYTE len)
{
	BYTE idata isSmallblock;
	BYTE idata mode;
	UWORD xdata headAddr;

	mode = IF_UpdateOrReadWithRAM(fileAddr,offset,buf,len,Hige_Update_mode);
	
	if(mode != 0x02)
	{
		return mode;
	}

	headAddr = fileAddr;
	
	isSmallblock = IF_TFUpdate_Smallblock(fileAddr);  //不能移动此函数位置  fileAddr现在是文件头地址

	
	fileAddr = FindTFAddr(fileAddr, offset, len);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	if(isSmallblock == TRUE) //如果是轮询方式的 更新
	{
		return IF_Update_SmallBlock(fileAddr,headAddr,buf,offset,len,Hige_Update_mode);
	}

	//return WriteEepromEX(fileAddr,&buf[0],len);	
	return WriteEepromAtomic(fileAddr,&buf[0],len);	 
	


}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ReadTFBuf
  
  input parameter:   UWORD fileAddr, WORD offset, BYTE * buf, BYTE len

  output parameter:  TRUE/FASLE
                                                                                                                                                
  description:       Read Data From a transparent file
  
  status:			     

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE ReadTFBuf(UWORD fileAddr, WORD offset, BYTE * buf, BYTE len)
{
	BYTE idata mode;
	
	mode = IF_UpdateOrReadWithRAM(fileAddr,offset,buf,len,Hige_Read_mode);
	
	if(mode != 0x02)
	{
		return mode;
	}

	fileAddr = FindTFAddr(fileAddr, offset, len);

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
	UWORD idata fileAddr;

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
#if !NATIVE_CARD_UICC
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
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SelectPrivateLFFile
  
  input parameter:   WORD dfName, DF file id

					 WORD efName, EF file id    					                                                                                                         

  output parameter:  addr /FALSE
                                                                                                                                                
  description:       在dfName下查找文件efName,如果找到,返回文件的地址

  Coder:		xiaoyh			Tester:   
  
  status:			     

 ------------------------------------------------------------------- ------------------------------------------------*/    
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
#endif

/*@Function:   ReadLFFileValue                                            
 * 
 *@Descrip:    ??¨?LF???t?¨￠¨¨??¨2¨¨Y
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
WORD ReadLFFileValue(UWORD fileAddr,BYTE index,BYTE * buf)
{
	WORD idata parameterLen;

	ReadLFBuf(fileAddr, index, 0, &buf[0], 2); //
	
	parameterLen = BuildWord(buf);
			
	ReadLFBuf(fileAddr, index, 2, &buf[0], parameterLen); //

	return parameterLen;
}




/*@Function:   WriteLFFileValue                                            
 * 
 *@Descrip:    写入L(2BYTE)V格式数据到记录文件的指定记录
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
 *				V1.1---modify by jiangqb 20191128-增加函数返回值,写入成功返回TRUE, 写文件越界，写入失败时返回false
 *                        
 */
BYTE WriteLFFileValue(UWORD fileAddr,BYTE index,BYTE * buf,WORD length)
{
	BYTE idata temp[2];
	
	SetWord(temp, length);
	
	WriteLFBuf(fileAddr, index, 0, temp, 2);
	
	if (!WriteLFBuf(fileAddr, index, 2, buf, length))
	{
		return FALSE;
	}
	
	return TRUE;
}


/*------------------------------------------------------------------------------------------------------------     
  Function name :	ReadTFFileWord                                                                                                                   
                                                                                                                                                
  input parameter:   void                                                                    
  					                                                                                                         
  output parameter:  void                                                                                                                 
                                                                                                                                                
  description:       This function manage the bookmark
  
  status: 			need test
  
  author :          XuWeiHua      
 --------------------------------------------------------------------------------------------------------------*/
WORD ReadTFFileWord(UWORD fileAddr)
{
	WORD  idata len;

	ReadTFBuf(fileAddr, 0x00, (BYTE *)&len, sizeof(WORD));  //??¨¨?bipreceive?D¨?DD?ì¨oy?Y|ì???¨oy?ê?3?¨o???￥?a0

	return len;
}

/*------------------------------------------------------------------------------------------------------------                                                                                                                                               
  Function name :	WriteTFFileWord                                                                                                                   
                                                                                                                                                
  input parameter:   void                                                                    
  					                                                                                                         
  output parameter:  void                                                                                                                 
                                                                                                                                                
  description:       This function manage the bookmark
  
  status: 			need test
  
  author :          XuWeiHua      
 --------------------------------------------------------------------------------------------------------------*/
void WriteTFFileWord(UWORD fileAddr,WORD len)
{
	WORD  idata writeLen;

	writeLen = len;

	WriteTFBuf(fileAddr, 0x00, (BYTE*)&writeLen, 2);
}



