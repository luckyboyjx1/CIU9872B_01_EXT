/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.5

 * MODEL: 	All model

 * RELEASE: 2008-12-18

 * AUTHER:	徐庆盛 徐云珍

 * HISTORY: V1.0 创建源文件

 			V1.1 添加函数CreateBERTLVFile,创建BER-TLV structure EF,文件类型为0x39

			V1.2 修改函数DeleteFile，ResizeFile，增加对BER-TLV structure EF的删除和修改大小(版本：1.40 -> 1.41)
			
			V1.3 增加在create file时写E2 错误时，返回9240错误状态字

			V1.4 修改CreateLinerFile、CreateTransFile、CreateBERTLVFile：增加TAG 0x8E，用于RFM更新权限的控制

			V1.5 增加创建EF文件时的初始化方式：增加‘C4’TAG，初始化整个文件的内容为长度（固定为1BYTE）后续的字节值 
			V1.6 修改CreateLinerFile、CreateTransFile：增加TAG 0x89，用于PBOC指示文件安全属性对应的记录号(适用于RFID-USIM卡)

			V1.7 修改CreateLinerFile：当文件空间大小不能被记录长度整除时，返回参数错误，文件创建失败

			v1.8 修改函数CreateEFFile，修改创建循环文件时，记录个数比实际要创建的记录个数多创建一条 20110722 by xuyz
			
			V1.9 修改函数ResizeFile,判断P3的长度是不是和后续接收的长度一致 @20120613 by  xuyz
			
			V1.10 修改函数InitRecordFile，当ptrToLen为null时 ，不能取其下标，否则可能出现异常@20140728 by xuyz
			
			V1.11 修改函数CreateFile，判断62tag的长度与P3 的长度是否吻合；
				  修改函数DeleteFile，若当前需要移动数据，则首先设置secureMover为D_EEPNoMoveTag，以便断电时标示数据还未开始移动 ；
				  修改函数ResizeFile ,对于线性文件，resize的文件大小一定得是记录长度的整数倍，卡片才允许执行
				  修改函数ResizeFile,,对于空间增加，需要清除新增 空间内容为全FF时，在resize标记清除后处理
				  修改函数ResizeFile,如果不需要移动空间，设置tag|= 0x01;若需要移动数据，则先设置secureMover为D_EEPNoMoveTag，以便断电时标示数据还未开始移动·20150512
				  
		   	V1.12  修改函数ResizeFile，增加子函数InitRecordFileEX，InitTransFileEX初始化EF，改为断电保护写；删除空间增加时，初始化新增空间为全FF的操作@20150513
			V1.13  修改函数CreateFile，当判断条件(P3 != (RecData[1] +2))返回状态字6A80    @20150515   by  wgl
			V1.14  修改函数CreateFile，改正创建映射文件时不能识别同一个DF下相同文件ID的错误   	 @20170925   by  yangjian
		
 */ 
#include "uicc_main.h"
#include "uicc_error.h"
#include "fileSystem.h"
#include "gsmpublic.h"
#include "fileAction.h"
//#include "hard_api.h"
#include "stkhead.h"
#include "eepromDefine.h"
#include <string.h>
#include <stdlib.h>
#include  "interface.h"
//#include "Transaction.h"
#include  "Interface_LTEFromBoot.h"
/*
*function    : modifyGrandParentSize
*
*@description:  从结构体ResizeAntiTearing中恢复父目录的文件大小信息
				
*
*@param      :ResizeAntiTearing * antiStruct  --  断电文件上下文结构体
*@param       BYTE mode			  --   	D_CreateFile/D_Resize/D_DeleteFile

*
*@return     :void  
*
*@history    :V1.0 by zhxx @20200609
*/



/*
*function    : modifyGrandParentSize2
*
*@description:  更新父目录文件大小信息到结构体ResizeAntiTearing中
				
*
*@param      :ResizeAntiTearing * antiStruct  --  断电文件上下文结构体
*@param       SDWORD headSize	   ---   要改变的文件头大小
*@param       SDWORD contentsize   ---    要改变的文件体大小
*@param       BYTE mode			  --   	D_CreateFile/D_Resize/D_DeleteFile

*
*@return     :BYTE  -- 文件层级
*
*@history    :V1.0 by zhxx @20200609
*/
#if NATIVE_CARD_UICC
BYTE modifyGrandParentSize2(SDWORD headSize,SDWORD contentsize,BYTE mode)
{
    UWORD xdata  parentfileaddr;
	BYTE idata i;
	SingleFileHead xdata  dfHeader;

	for(i=0; i < DF_Depth; i++)	  //更新各个父目录的文件大小信息
	{	
        parentfileaddr = (i == 0) ? G_CurrentDfAddr[G_ChannelNum] : (GetParentOrMapFileAddr(parentfileaddr,F_ParentName_Offset));

		ReadEeprom((BYTE *)&dfHeader.UsedContentSize,parentfileaddr + F_UsedContentSize_Offset,sizeof(SingleFileHead));
		
		dfHeader.UsedHeadSize += headSize;

		dfHeader.UsedContentSize += contentsize;
		
		if((i == 0) && (mode != D_Resize))//放在结构体SingleFileHead中，一次性写，加快创建文件的速度
		{ 
		   	if(mode == (D_CreateFile | DF))	
			{
				dfHeader.ChildDfNum ++;
			}
			else if(mode == (D_CreateFile | EF))	
			{
				dfHeader.ChildEfNum ++;
			}
			else if(mode == (D_DeleteFile | DF))	
			{
				dfHeader.ChildDfNum --;
			}
			else if(mode == (D_DeleteFile | EF))	
			{
				dfHeader.ChildEfNum --;
			}
			else
			{
				
			}
		}
		
		WriteEepromEX(parentfileaddr + F_UsedContentSize_Offset,(BYTE *)&dfHeader.UsedContentSize,sizeof(SingleFileHead));
		
		if(parentfileaddr == E_FILE_START)
		{
			break;
		}
		
	}
	
	return i;
}



/*
*function    : GetFileContentSize
*
*@description:  获取文件内容的长度
				
*
*@param      :UWORD fileAddr --文件头地址

*
*@return     :UWORD  -- 文件内容长度
*
*@history    :V1.0 by zhxx @20200609
*/
UWORD GetFileContentSize(UWORD fileAddr)
{
	BYTE  idata fileType;
	UWORD xdata fileContentSize;
	BYTE  idata fileRecNumLgth[2];

	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);	//

	if(fileType == ADFTYPE)//DF
	{
		fileContentSize = GetFileHeadUword( fileAddr,F_UsedContentSize_Offset);
	}
	else if(fileType == TRANTYPE)  //
	{
		fileContentSize =  GetTFSize(fileAddr);
	}
	else if((fileType == LINTYPE)||(fileType == CYCTYPE)) //
	{
     	ReadEeprom((BYTE *)&fileRecNumLgth[0],fileAddr+F_RecNum_Offset,sizeof(fileRecNumLgth));

     	fileContentSize = (fileRecNumLgth[0]*fileRecNumLgth[1]) ;	//fileSpaceAddr as Space

		if(fileType == CYCTYPE)	//
		{
		   fileContentSize ++;
		}
	}
	else
	{
		return FALSE;
	}

	return fileContentSize;
}



/*
*function    : GetFileBodyEndAddr
*
*@description:  获取文件体的结束地址
				
*
*@param      :void

*
*@return     :void
*
*@history    :V1.0 by zhxx @20200609
*/
UWORD GetFileBodyEndAddr()
{
	 UWORD xdata mfBodyEndAddr,mfContentLen,fileBodyStart;

	 fileBodyStart = E_FileBodyStart;

	 mfContentLen = GetFileContentSize(E_FILE_START);

	 mfBodyEndAddr = (fileBodyStart - mfContentLen);

	 return  mfBodyEndAddr;
}
#endif

/*
*function    : GetAvailNVMMemory
*
*@description:  获取eeprom可用空间大小
				
*
*@param      :void

*
*@return     :UWORD  -- eeprom可用空间
*
*@history    :V1.0 by zhxx @20200227
*/
UWORD  GetNVMAvailMemory(void)
{
	 #if !HEAD_BODY_TOGETHER	
    UWORD xdata mfHeaderEndAddr,spareSpace,mfBodyEndAddr;

	mfHeaderEndAddr = GetFileHeaderEndAddr();

    mfBodyEndAddr = E_FileBodyStart -  GetFileContentSize(E_FILE_START);//GetFileBodyEndAddr();

	spareSpace =  mfBodyEndAddr - mfHeaderEndAddr;

	return 	spareSpace;
	#else
	return 0;
	#endif

}


 #if !HEAD_BODY_TOGETHER
/*
*function    : GetFileHeaderEndAddr
*
*@description:  获取文件头区域的结束地址 
				
*
*@param      :void

*
*@return     :文件头的结束地址 
*
*@history    :V1.0 by zhxx @20200219
*/
UWORD GetFileHeaderEndAddr(void)
{
	UWORD xdata mfUsedSize;  
	
	ReadEeprom((BYTE *)&mfUsedSize,E_FILE_START+F_UsedHeadSize_Offset,sizeof(UWORD));	   //文件头的结束地址即MF起始地址+已经使用的地址
	
	return E_FILE_START + mfUsedSize;
}




/*
*function : GetAndModifyFreeSpaceSize
*
*@description:  
*
*@param      :UWORD size 需要分配的空间
*@param      :
*@param      :
*
*@return     :UWORD addr 分配的地址/FALSE 空间不足
*
*@history    : added by zhxx@20200219
*/
UWORD GetAllocatedAddr(UWORD assignSize)
{	
	UWORD xdata flashUsedAddr,dfContentLen,curDFBodyStart;
	WORD idata 	fileContentRef;

	dfContentLen = GetFileContentSize(G_CurrentDfAddr[G_ChannelNum]);

	fileContentRef = GetFileHeadWord(G_CurrentDfAddr[G_ChannelNum],F_DFContent_Offset);

	curDFBodyStart = VMReadUword(E_ADDR_INDEX_TABLE + (fileContentRef-1)*sizeof(UWORD));
	
	flashUsedAddr = curDFBodyStart - dfContentLen;	  //对象已经分配到的地址


	flashUsedAddr -= assignSize;
			
	return flashUsedAddr;
}

/*
 *@Function: FindFreeIndexFromAddrIndexTable
 *
 *@Descrip:	 在ADDR_INDEX_TABLE中找一个空闲的索引，若找不到 则返回REF_NULL 表示本次未找到合适的索引
 *
 *@See:		地址索引表的索引同时当成对象引用使用，有效索引从1~0xFFFF
 *
 *@Param:    void
 *
 *@Return:   WORD-返回分配的内存空间地址在ADDR_INDEX_TABLE中的索引号（index/0x0000）
 *
 *@Author:   added by zhxx@20200219
 *
 *@History:  V1.0 BY zhxx
			 V1.1  MF和HEAP共用 两头压缩 不支持地址索引表扩展  仅支持MAX_INDEX_NUM条地址
 */
WORD FindFreeIndexFromAddrIndexTable(void)
{
	WORD idata index;
	UWORD xdata addrValue;
        	
	for(index=0; index < MAX_INDEX_NUM; index++)	//从地址表中查找空的索引
	{
		ReadEeprom((BYTE*)&addrValue,E_ADDR_INDEX_TABLE + index * sizeof(UWORD), sizeof(UWORD));
		
		if(addrValue == 0xFFFFFFFF)	//找到空闲的索引
		{
			return (index + 1);
		}
	}
	
	return REF_NULL;
	
}



/*
*@Function: VMHeapAlloc
*
*@Descrip:	 存储空间的动态分配
*
*@See:		从FlashUsedAddr处分配allocSize 大小的空间，并在ADDR_INDEX_TABLE表中找一个空闲位置将分配的地址写入，
*		    返回ADDR_INDEX_TABLE中的索引号，空间不足或找不到空闲索引则返回0x0000；
*
*@Param:   size  :分配的存储空间的大小
*@param    index :分配的有效的地址引用
*
*@Return:   UWORD-返回分配的内存空间 or ADDR_IS_NULL /index[0] = 0   
*
*@Author:   added by  zhxx 20200219
*
*@History:  
*/
UWORD VMHeapAlloc(UWORD size,WORD* index)
{
	WORD idata indexInAddrTable;
	UWORD xdata newAllocAddr,assignSize;

	assignSize = size;
	
	indexInAddrTable = FindFreeIndexFromAddrIndexTable(); //先找索引表再分配地址，找索引时可能会动态扩展导致可分配空间发生变化 
	
	if(indexInAddrTable == REF_NULL)
	{
		return ADDR_IS_NULL;
	}
	
	index[0]  = indexInAddrTable;
	
	newAllocAddr = GetAllocatedAddr(assignSize);
	
	if(newAllocAddr == FALSE)//当前可用空间小于需要分配的空间大小
	{
		return ADDR_IS_NULL;
	}
	
	return newAllocAddr;
}
#endif

/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	CompareExpandValue                                                                                                                
                                                                                                                                                
  Input parameter: BYTE * ptrToResult(LV 结构)
                                                                                              
  Output parameter: FALSE/RefNum                                                                                                      
                                                                                                                                                
  Description:    比较文件EF_ARR 中的Expand value ，没有相同的值则添加记录，返回结果

  Coder:			ZXL         Tester:   

  Current status:      

-------------------------------------------------------------------------------------------------------------*/
BYTE CompareExpandValueInFile(BYTE * ptrToResult)
{
	BYTE    idata recLenNum;
	BYTE    idata i,flag;
	UWORD 	xdata arrFileAddr;
	
	arrFileAddr = SelectLFFile(DF_MF,EF_ARR);

	if(arrFileAddr == FALSE)
	{
		return FALSE;
	}
	recLenNum = GetLFRecNumLgth(arrFileAddr,FileLgth);

	if(recLenNum < ptrToResult[0])					
	{
		return FALSE;

	}

	recLenNum = GetLFRecNumLgth(arrFileAddr,FileNum);

	flag = 0xFF;  //该标志即标记当前已经查找到空记录,又标记着空记录的记录号

	for(i=0; i<recLenNum; i++)
	{
		ReadLFBuf(arrFileAddr, i+1, 0x00, (BYTE *)&ptrToResult[ptrToResult[0] + 1], ptrToResult[0]);		//Reeplace  RecData Buf with ptrToResult Buf

		if(memcmp((BYTE *)&ptrToResult[ptrToResult[0] + 1],&ptrToResult[1],ptrToResult[0]) == EQUAL)		
		{
			return (i+1);
		}

		if((ptrToResult[ptrToResult[0] + 1]  == 0xFF)&&(flag == 0xFF))  //保留查找到第一条为空的记录，若在文件中没有找到相同的记录，则将新信息写入该条记录中
		{			
			flag = i;
		}
	}

	if(flag == 0xFF)	    // 没有找到空记录 
	{
		return FALSE;
	}
	else
	{
		WriteLFBuf(arrFileAddr, flag+1,0, &ptrToResult[1],ptrToResult[0]);
	
		return (flag+1);
	}
}

/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	ActionOfBit                                                                                                                
                                                                                                                                                
  Input parameter: BYTE bitPos
                                                                                              
  Output parameter: FALSE/SC_DO                                                                                                      
                                                                                                                                                
  Description:      

  Coder:			ZXL         Tester:   

  Current status:      

-------------------------------------------------------------------------------------------------------------*/

BYTE ActionOfBit(BYTE bitPos)
{
	return 1<<(bitPos-1);	  //bit1: A_ReadFile,bit2:A_UpdateFile,bit3:A_CreateDF,bit4:A_DeactivateFile,bit5 :A_ActivateFile,bit6:A_TerminateFile,bit7: A_DeleteFile
}

/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	SecurityConditionByte                                                                                                                
                                                                                                                                                
  Input parameter: 	BYTE scByte
                                                                                              
  Output parameter: FALSE/SC_DO                                                                                                      
                                                                                                                                                
  Description:      

  Coder:			ZXL         Tester:   

  Current status:      

-------------------------------------------------------------------------------------------------------------*/
BYTE SecurityConditionByte(BYTE scByte)
{
	switch(scByte)
	{
		case 0x00: return 0x90;				//ALW

		case 0xFF: return 0x97;				//NEV

		case 0x10: return 0x01;				//PIN

		case 0x90: return 0x0A;				//ADM

		default: return FALSE;
	}
}

/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	CompactToExpand                                                                                                                
                                                                                                                                                
  Input parameter: BYTE * ptrToLen : point to L of the Security attribute TLV(Tag = '8C')
  					
				BYTE * resultBuf
                                                                                              
  Output parameter: TRUE/FALSE                                                                                                      
                                                                                                                                                
  Description:      转换Compact格式为Referenced格式，不支持OR template

  Coder:			ZXL         Tester:   

  Current status:      

-------------------------------------------------------------------------------------------------------------*/
static BYTE code AccessContion[] = {0xA4,0x06,0x83,0x01,0xFF,0x95,0x01,0x08};

BYTE CompactToExpand(BYTE * ptrToLen,BYTE * resultBuf)
{
	BYTE idata i,j,k,f;
	BYTE idata tmpResult,sameResult;

	if((ptrToLen[0]&0x80) == 0x80)
	{
		return FALSE;						// 不支持proprietary
	}

	for(k=1,f=0,j=7;j>0;j--)
	{
		if(ValueBit(ptrToLen[0],(j-1)) == TRUE)      //AM  byte
		{
			tmpResult = SecurityConditionByte(ptrToLen[1+f]);

			if(tmpResult == FALSE)
			{
			 	return FALSE;
			}

			f++;

			for(sameResult=FALSE,i=1;(f>1)&&(i<k);)
			{
				if(resultBuf[i+3] == 0xA4)         //权限不为NEVER,ALW
				{
					if(resultBuf[i+7] == tmpResult)
					{
						sameResult = TRUE;
	                    
						break;
					}
					else
					{
						i = i+11;
					}
				}
				else
				{
					if(resultBuf[i+3] == tmpResult)
					{
						sameResult = TRUE;
	                    
						break;
					}
					else
					{
						i = i+5;
					}
				}
			}


			if(sameResult == TRUE)
			{
				resultBuf[i+2] = resultBuf[i+2]+ActionOfBit(j);
					
			}
			else
			{
				resultBuf[k] = 0x80;
                
				resultBuf[k+1] = 0x01;
                
				resultBuf[k+2] = ActionOfBit(j);

				if((tmpResult == 0x90)||(tmpResult == 0x97))
				{
					resultBuf[k+3] = tmpResult;
                    
					resultBuf[k+4] = 0x00;
					
					k = k+5;	
				}
				else
				{
					memcpy_G(&resultBuf[k+3], (BYTE CONST *)&AccessContion[0], sizeof(AccessContion));

					resultBuf[k+7] = tmpResult;
					
					k = k+11;					
				}
			}
		}
	}					

	resultBuf[0] = k-1;		//???

	return TRUE;		
} 

/*
 *@description:  对于compact format模式，找出其在0x2F06文件中所处的记录号；若未找到，则在0x2F06找到空记录，写入，返回空记录号
 *
 *@param      :  ptrToValue:TLVTLV...数据
 *@param      :  pos:                当前数据的长度
 *
 *@returns    :  0x2F06中的记录号
 *
 *@history    :  create by xyz at 090804 15:03 
*/
BYTE FindArrNum(BYTE *ptrToValue,BYTE pos)
{
	BYTE idata result;

	result = FindTag(&ptrToValue[0],0x8C,pos);									//Compact mode for RFM		

	if(result == FALSE)
	{
		return 	FALSE;	
	}
	
	result = CompactToExpand((BYTE *)&ptrToValue[result+1],(BYTE * )&tempBuf[0]);

	if(result == FALSE)
	{
		return 	FALSE;
	}

	result = CompareExpandValueInFile((BYTE * )&tempBuf[0]);

	if(result == FALSE)
	{
		return FALSE; 
	}
	
	return result;
}



/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	InitPattern                                                                                                                
                                                                                                                                                
  Input parameter: 	BYTE * ptrToLen : point to NULL or L of the TLV(Tag = 'A5')
  					
					BYTE * pos
                                                                                              
  Output parameter: 0xFF- 全部置0xFF/0xC1-直接填充/0xC2-按块重复/0xC3-只初始化第一条记录 /0xC4-将文件体初始化成后续字节                                                                                                    
                                                                                                                                                
  Description:      Init pattern for created ef

  Coder:			ZXL         Tester:   Caibo

  Current status:	modify at 2007-06-25       

-------------------------------------------------------------------------------------------------------------*/
BYTE InitPattern(BYTE * ptrToLen,BYTE * pos)
{
	BYTE idata i;

	if(ptrToLen != NULL)
	{
		for(i = 1;i <= 0x04; i++)
		{
			pos[0] = FindTag(&ptrToLen[1],(0xC0+i),ptrToLen[0]);

			if((pos[0] != FALSE)&&(ptrToLen[pos[0]+1] != 0x00))
			{
			 	return (0xC0+i);		
			}
		}
	
	}
	return 0xFF;                                               //全部置FF         							
}

/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	InitTransFile                                                                                                                   
                                                                                                                                                
  Input parameter: 	BYTE * ptrToLen : point to NULL or L of the TLV(Tag = 'A5')
  					
					UWORD fileBodyAddr
					
					WORD bodySize
					                                                                                                   
  Output parameter: void                                                                                                      
                                                                                                                                                
  Description:      Init the craeted TF

  Coder:			ZXL         Tester:   Caibo

  Current status:   modify order at 2007-06-25  
  V1.1               添加强更新文件tag updatetag,如果是强更新文件 并且采用轮询方式的话 初始化时候 要加上 TAG+crc 20200309-LCZ

-------------------------------------------------------------------------------------------------------------*/
void InitTransFile(UWORD fileBodyAddr,BYTE * ptrToLen,WORD bodySize,BYTE updatetag)
{
	BYTE idata pos;
	BYTE idata fillPattern;
	BYTE idata length;
	WORD idata spareSzie;	
	

	if(bodySize == 0)
	{
		return;
	}

	fillPattern = InitPattern(ptrToLen,&pos);

	if(fillPattern == 0xC1) 
	{	
		length = (ptrToLen[pos+1] > bodySize)?(BYTE)(bodySize&0x00FF):(ptrToLen[pos+1]-1);
			
		WriteEeprom(fileBodyAddr,(BYTE *)&ptrToLen[pos+2],length);
			
		SetEeprom((fileBodyAddr+length),ptrToLen[pos+2+length],bodySize-length);							 
	}
	else if(fillPattern == 0xC2) 
	{			
		for(spareSzie = bodySize;spareSzie>0;spareSzie =spareSzie- length)
		{
			length = (spareSzie>ptrToLen[pos+1])?ptrToLen[pos+1]:(BYTE)(spareSzie&0x00FF);

			WriteEeprom(fileBodyAddr+(bodySize-spareSzie),(BYTE *)&ptrToLen[pos+2],length);								
		}
	}
	else if(fillPattern == 0xC4)
	{		
		SetEeprom(fileBodyAddr, ptrToLen[pos+2], bodySize);  //文件体初始化成ptrToLen[pos+2]
	}
	else			      
	{		 
		SetEeprom(fileBodyAddr,0xFF,bodySize);				
	}

	if(updatetag)
	{
			IF_Init_High_Update_Area(fileBodyAddr,bodySize,updatetag);
	}

}

/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	InitRecordFile                                                                                                                   
                                                                                                                                                
  Input parameter: 	UWORD fileBodyAddr
  
  					BYTE * ptrToLen
					
					BYTE recNum
					
					BYTE recLen
					                                                                                                   
  Output parameter: void                                                                                                      
                                                                                                                                                
  Description:		C1模式:TS102.222 chapter6.3.2.2.2
				C2模式:TS102.222 chapter6.3.2.2.2
				C3模式:第一条记录初始化为初始值，其余记录全F；
				0xFF        :默认全F

  Coder:			ZXL     Tester: Caibo

  Current status:	new 2007-06-25    

-------------------------------------------------------------------------------------------------------------*/
void InitRecordFile(UWORD fileBodyAddr,BYTE * ptrToLen,BYTE recNum, BYTE recLen)
{
	BYTE idata pos;
	BYTE idata i;
	BYTE idata fillPattern;
	BYTE idata length;
	BYTE idata spareSzieInOneRecord;	
	WORD  idata actual;
	
	if((recNum == 0) || (recLen == 0))
	{
		return;
	}
	actual = recNum*recLen;		    //计算记录文件的实际占用空间

	fillPattern = InitPattern(ptrToLen,&pos);
	
	if(fillPattern != 0xFF)
	{
		length = (ptrToLen[pos+1] > recLen)?recLen:(ptrToLen[pos+1]-1);	//为防止某些芯片对空指针取下标时 ，芯片异常，这里增加判断
	}

	if(fillPattern == 0xC1)
	{		
		for(i = 0x00;i < recNum; i++)
		{
			WriteEeprom(fileBodyAddr+(i*recLen),(BYTE *)&ptrToLen[pos+2],length);
				
			SetEeprom(fileBodyAddr+(i*recLen)+length,ptrToLen[pos+2+length],recLen-length);
		}		
	}
	else if(fillPattern == 0xC2)
	{		
		for(i = 0x00;i < recNum; i++)
		{
			for(spareSzieInOneRecord = recLen;spareSzieInOneRecord>0;spareSzieInOneRecord -= length)
			{
				length = (spareSzieInOneRecord > ptrToLen[pos+1] ) ? ptrToLen[pos+1]:spareSzieInOneRecord;

				WriteEeprom(fileBodyAddr+(i*recLen)+(recLen-spareSzieInOneRecord),(BYTE *)&ptrToLen[pos+2],length);	
			}
		}	
	}	
	else if(fillPattern == 0xC4)
	{	
		SetEeprom(fileBodyAddr, ptrToLen[pos+2], actual);  //设置成ptrToLen[pos+2]
	}
	else
	{		
		SetEeprom(fileBodyAddr, 0xFF, actual);  //清空文件体

		if(fillPattern == 0xC3)
		{
			if(length < recLen)
			{
				length ++;
			}

			WriteEeprom(fileBodyAddr,(BYTE *)&ptrToLen[pos+2],length);
		}
	}
}


#if NATIVE_CARD_UICC
#ifndef SECURE_CREATE
/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               
  Function name :	 CreateDF                                                                                                                   
                                                                                                                                                
  input parameter: 
                     fileMaxSize:  the parent file max size

                     fileUseSize:  the parent file used size
					                                                                                                   
  output parameter:  the file will used space/false                                                                                                          
                                                                                                                                                
  description:       compare the space and write df file head
					 
  Current status:    Code OK!  Test OK!    
-------------------------------------------------------------------------------------------------------------*/

UWORD CreateDFFile(UWORD fileUseSize,WORD fileName)
{
	FileSystemHead xdata dfFileHead;
	UWORD xdata fileSpareSize,mfUsedSize,datalen,allocateAddr;
	BYTE  idata result,pos,i,j;
	BYTE  * ptrToValue;
		
	if(sizeof(FileSystemHead) > GetNVMAvailMemory())
	{
		return E_FileSize;
	}

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
	

	dfFileHead.UsedContentSize = 0;

	dfFileHead.UsedHeadSize = sizeof(FileSystemHead);

	dfFileHead.ChildDfNum = 0x00;

	dfFileHead.ChildEfNum = 0x00;	

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
	
	allocateAddr = VMHeapAlloc(0, (WORD*)&dfFileHead.FileContentRef);
	
	VMWriteUword((E_ADDR_INDEX_TABLE + (dfFileHead.FileContentRef-1)*sizeof(UWORD)),allocateAddr);

    mfUsedSize = GetFileHeadUsedSize(E_FILE_START);

	datalen = E_FILE_START + mfUsedSize - (G_CurrentDfAddr[G_ChannelNum] + fileUseSize); //判断要创建的文件是否在MF的最末尾


	if(datalen > 0)	//如果不在MF最末尾，而是中间插入一个文件，需要从当前DF的usedsize开始，往后挪动此次所需size个长度
	{
	    return 	E_FileSize;
	} 
	
    
	result = WriteEepromEX(G_CurrentDfAddr[G_ChannelNum]+fileUseSize,(BYTE *)&dfFileHead.FileId,sizeof(FileSystemHead));//将DF文件头写入文件头区
	
	if(result == FALSE)
	{
		return FALSE;
	}

	i = modifyGrandParentSize2(sizeof(FileSystemHead),0,(D_CreateFile| DF));
	
	if(i >= (DF_Depth-1) )  //cos最大支持3级DF
	{		
		return E_Parameter;
	}
		
	return fileUseSize;
}
#endif


#ifndef SECURE_CREATE
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

UWORD CreateEFFile(UWORD fileUseSize,WORD fileName)
{
	BYTE idata pos,result,i,updatetag;
	BYTE idata fileType,shortFileId,arrNum,arrName,fileStatus;
	BYTE  * ptrToValue;
	WORD  idata fileSize;
	UWORD xdata fileNeedSize,allocateAddr,mfUsedSize,datalen,updateAddr;
	WORD xdata mapFile[3];
    EFileHead xdata efFileHead;
	BYTE xdata   HighUpdateCfgInfo[High_Update_ConfigForFID];
	BYTE idata 	 UpdateConfigIndex;	

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

	result = FindTag(&ptrToValue[0],0x8F,pos);							//Expand for FileMapping

	if(result != FALSE)
	{
		if(ptrToValue[result]==0x06)
		{
			mapFile[0] = (((WORD)ptrToValue[result+1]<<8)&0xFF00) | ptrToValue[result+2];
            
			mapFile[1] = (((WORD)ptrToValue[result+3]<<8)&0xFF00) | ptrToValue[result+4];

			mapFile[2] = (((WORD)ptrToValue[result+5]<<8)&0xFF00) | ptrToValue[result+6];
		}
		else if(ptrToValue[result] == 0x04)
		{
			mapFile[0] = (((WORD)ptrToValue[result+1]<<8)&0xFF00) | ptrToValue[result+2];
            
			mapFile[1] = mapFile[0];
            
			mapFile[2] = (((WORD)ptrToValue[result+3]<<8)&0xFF00) | ptrToValue[result+4];
		}
		else if(ptrToValue[result] == 0x02)
		{
			mapFile[0] = 0x3F00;
            
			mapFile[1] = 0x3F00;
            
			mapFile[2] = (((WORD)ptrToValue[result+1]<<8)&0xFF00) | ptrToValue[result+2];
		}
		else	//for baihe-test
		{
			return E_Parameter;	
		}

		fileStatus = fileStatus|F_MAP_FILE;

		if((fileType&0x37) == TRANTYPE)
		{
	        efFileHead.efSize.SpaceSize = 0; 			
	    }
		else 
		{
			efFileHead.efSize.recInfo[0] = 0x00;
			
			efFileHead.efSize.recInfo[1] = 0x00;
						
			fileSize = 0x0000;
		}

		if(sizeof(EFileHead) >  GetNVMAvailMemory()) 	//映射文件只判断文件头空间是否足够
		{
			return E_FileSize;
		}

		efFileHead.UpdateConfignum = 0x00; //映射文件 强更新标志登记在实体文件

		efFileHead.FileContentRef =  REF_NULL;//映射文件不需要分配实际的对象空间
	}
	else
	{
		mapFile[0] = 0x0000;

		mapFile[1] = 0x0000;

		mapFile[2] = 0x0000;

		fileStatus = fileStatus&(~F_MAP_FILE);

		if((fileType&0x37) == TRANTYPE)  //透明文件
		{
			efFileHead.efSize.SpaceSize = fileSize;	
			
		}
		else 
		{
			result = FindTag(&ptrToValue[0],0x82,pos);

			efFileHead.efSize.recInfo[1] = ptrToValue[result+4];						//Record length in FCP is WORD
			
			efFileHead.UpdateConfignum = 0x00;//默认不是强更新文件
			
			if(	efFileHead.efSize.recInfo[1] == 0x00)   //长度为0 也报错
			{
				return E_Parameter;
			}
	
			if ((fileSize%	efFileHead.efSize.recInfo[1]) != 0x00)
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

			efFileHead.efSize.recInfo[0] = (BYTE)((fileSize/	efFileHead.efSize.recInfo[1])&0x00FF);	
			
			if((fileType&0x37) == CYCTYPE)  //对于循环文件来说，多创建一条记录以实现循环文件的断电保护，added @20110721 by xuyz
			{
				if(	efFileHead.efSize.recInfo[0] == 0xFF )   //不允许创建255条的循环记录文件
				{
					return E_Parameter;
				}
				
				fileSize = 	efFileHead.efSize.recInfo[0] * 	efFileHead.efSize.recInfo[1] + 1;   //循环文件的实际大小,记录指针放在文件体的第一个字节

			}
		}			

		allocateAddr = 	SecureDataStore(fileName,(WORD*)&efFileHead.FileContentRef);

            if(allocateAddr == FALSE)
			{
			    allocateAddr = VMHeapAlloc(fileSize, (WORD*)&efFileHead.FileContentRef); //文件体分配的地址
			}
			else
			{
				fileSize = 0;  //复用fileType的最高bit表示是否存储在安全存储区
			}

			
			if(allocateAddr == ADDR_IS_NULL)
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
	
	if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)           
	{
		arrName |= 0x7F;       //RFM创建的文件拥有RFM 的任何权限	
	}
	else
	{
		result = FindTag(&ptrToValue[0],0x8E,pos);				

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


	fileNeedSize = sizeof(efFileHead);//文件头所需大小

    mfUsedSize = GetFileHeadUsedSize(E_FILE_START);

	datalen = E_FILE_START + mfUsedSize - (G_CurrentDfAddr[G_ChannelNum] + fileUseSize); //判断要创建的文件是否在MF的最末尾

	if(datalen > 0)
	{
		return 	E_FileSize;
	}

	
	efFileHead.FileId = fileName;
		
	efFileHead.FileType = fileType;
	
	efFileHead.FileStatus = fileStatus;
	
	efFileHead.ArrNum = arrNum;

	efFileHead.ArrName = arrName;

	efFileHead.ShortFileId = shortFileId;

	efFileHead.MapFile[0] = mapFile[0];
	
	efFileHead.MapFile[1] = mapFile[1];
	
	efFileHead.MapFile[2] = mapFile[2];
	
	result = WriteEepromEX(G_CurrentDfAddr[G_ChannelNum]+fileUseSize,(BYTE *)&efFileHead.FileId,sizeof(efFileHead));

	if(result == FALSE)
	{
		return FALSE;
	}

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

	if((fileStatus&F_MAP_FILE) != F_MAP_FILE)			//forbid init map file
	{		
		if((updatetag & Hige_UpdateWithEep) == 0x00) //普通文件  或者强更新在RAM的文件
		{
			updateAddr = allocateAddr;
		}
		
		if((fileType&0x37) == TRANTYPE)
		{
			InitTransFile(updateAddr,ptrToValue,efFileHead.efSize.SpaceSize,updatetag);  //初始化文件体信息
			VMWriteUword((E_ADDR_INDEX_TABLE + (efFileHead.FileContentRef-1)*sizeof(UWORD)),updateAddr);	   //将分配的地址写入相应索引的地址索引表中			
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
			
			VMWriteUword((E_ADDR_INDEX_TABLE + (efFileHead.FileContentRef-1)*sizeof(UWORD)),updateAddr);	   //将分配的地址写入相应索引的地址索引表中		

		}							
		
		if(updatetag!=0x00)
		{			 
			IF_Init_High_Update_Config(UpdateConfigIndex,HighUpdateCfgInfo); //强更新在RAM的 配置区也是需要修改的 
		}
			    
	}


	i = modifyGrandParentSize2(fileNeedSize,fileSize,(D_CreateFile|EF));//修改各父目录属性信息
	
	if(i == DF_Depth)  //COS 最大支持5级DF
	{		
		return E_Parameter;
	}
		
	return fileNeedSize;	
}
#endif
#endif
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 CreateFile
  
  Command format:    00    E0    00   00   XX
  
  Data    content:   FCP                                                        
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       在当前的APF/DF下，建立DF/ADF/EF
  
  status:			 加入RFM    

 -------------------------------------------------------------------------------------------------------------------*/    
#if NATIVE_CARD_UICC
void CreateFile(void)
{
	UWORD xdata  fileAddr;
	UWORD xdata  fileMaxSize,UsedSize;
	WORD  idata  fileName;
	BYTE  idata  result,pos,i,Len,tagLen;


	if((CLA!=0xA0)&&((CLA&0xF0)!=0x00))						//CLA可以为0xA0或为0x0X(不可以为0x80)
	{
		ClaError();
		return;
	}
	
		
	if(P1P2Check() ==FALSE)
	{
		return ;
	}
 
	ReceiveData();                                   								//receive data from ME
  
	pos = FindTag(&RecData[0],0x62,P3);

	if(pos == FALSE)
	{
		IncorrentParameter();														 //Send 6A 80
		return;
	}
	
	
	if(P3 != (RecData[1] +2))
	{
		IncorrentParameter();					//Send 6A 80
		return;
	}

	
	Len = RecData[pos] + 2;               //数据长度
	
	tagLen = 0x00;
	
	for(i=0x03;i<P3;i += 2)          //第一个tag长度开始,i下标
	{	
		tagLen = RecData[i] + 2 + tagLen;
		
     	i +=  RecData[i];
    }
	
	
	if((tagLen + 2)!=Len)
	{
		 P3Error();			
		 return;
    }
	
	pos ++;

	result = FindTag(&RecData[pos],0x83,P3 -pos);

	if(result == FALSE)
	{
		IncorrentParameter();					//Send 6A 80
		return;
	}
	
	fileName = (((WORD)RecData[result + pos + 1] << 8) & 0xFF00) + ((WORD)RecData[result + pos +2]&0x00FF) ;

	fileAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum],fileName);//查找要创建的文件是否存在
      
	if( fileAddr != FALSE )                                        						
	{
		FileExist();							//Send 6A 89
		return;
	}
	
	result = FindTag(&RecData[pos],0x82,P3 -pos);
	
	if(result == FALSE)
	{
		IncorrentParameter();														 	//Send 6A 80
		return;
	}

	UsedSize = GetFileHeadUsedSize(G_CurrentDfAddr[G_ChannelNum]);	//获取当前DF的已经使用空间

	fileAddr = G_CurrentDfAddr[G_ChannelNum] + UsedSize; //获取将要创建的文件地址

 	if((RecData[result+1+pos]&FILETYPE)== ADFTYPE)										//create df
	{
	   	result = AccessCondition(G_CurrentDfAddr[G_ChannelNum],A_CreateDF);

		if(result == FALSE)
		{
			SecuritySatisfied();
			return;
		}     

		fileMaxSize = IF_CreateDFFile(UsedSize,fileName);
	

	}
	else if(((RecData[result+1+pos]&FILETYPE)== 0x39)||((RecData[result+1+pos]&0x37) == TRANTYPE)||((RecData[result+1+pos]&0x37) == LINTYPE)||((RecData[result+1+pos]&0x37) == CYCTYPE))
	{
		result = AccessCondition(G_CurrentDfAddr[G_ChannelNum],A_CreateEF);

		if(result == FALSE)
		{
			SecuritySatisfied();
			return;
		}

		fileMaxSize = IF_CreateEFFile(UsedSize,fileName);

	}
	else
	{
	    fileMaxSize =  E_Parameter;
	}

	if(fileMaxSize == FALSE)
	{
		MemoryError();
		return;
	}
	
	if(fileMaxSize == E_Parameter)
	{
		IncorrentParameter();
		return;
	}
	
	if(fileMaxSize == E_FileSize)
	{
		NoEnoughSpace();
		return;
	}
 
	if(fileMaxSize == E_DfNameExist)
	{
		DfNameExist();
		return;
	}


	SetChannel(fileAddr,fileName,FALSE);
	
	CommandFinish(0x00);                                  									//send 90 00
}
#endif




void InitTransFileEX(UWORD fileBodyAddr,BYTE * ptrToLen,WORD bodySize)
{
	BYTE idata pos;
	BYTE idata fillPattern;
	BYTE idata length,len;
	WORD idata spareSzie;
	WORD idata remainLen;

	if(bodySize == 0)
	{
		return;
	}

	fillPattern = InitPattern(ptrToLen,&pos);

	if(fillPattern == 0xC1) 
	{	
		length = (ptrToLen[pos+1] > bodySize)?(BYTE)(bodySize&0x00FF):(ptrToLen[pos+1]-1);
			
		WriteEepromEX(fileBodyAddr,(BYTE *)&ptrToLen[pos+2],length);

		remainLen = bodySize-length;

		fileBodyAddr += length;

		while(remainLen > 0)
		{
			len = (remainLen > 255)?255:remainLen;

			SetEepromEX(fileBodyAddr,ptrToLen[pos+2+length],len);

			remainLen = remainLen - len;

			fileBodyAddr += len;
		}					 
	}
	else if(fillPattern == 0xC2) 
	{			
		for(spareSzie = bodySize;spareSzie>0;spareSzie =spareSzie- length)
		{
			length = (spareSzie>ptrToLen[pos+1])?ptrToLen[pos+1]:(BYTE)(spareSzie&0x00FF);

			WriteEepromEX(fileBodyAddr+(bodySize-spareSzie),(BYTE *)&ptrToLen[pos+2],length);								
		}
	}

}

/*
*@function : InitRecordFileEX
*
*@description  :resize结束后，需要需要断电保护写更新文件初始值
*
*@param   :fileBodyAddr :记录文件体的地址
*@param    :prTolen:初始化数据
*@param    :recNum:记录个数
*@param    :recLen:记录长度
*
*@author
*
*@history:
*/
void InitRecordFileEX(UWORD fileBodyAddr,BYTE * ptrToLen,BYTE recNum, BYTE recLen)
{
	BYTE idata pos;
	BYTE idata i;
	BYTE idata fillPattern;
	BYTE idata length;
	BYTE idata spareSzieInOneRecord;	
	//WORD  idata actual;
	
	if((recNum == 0) || (recLen == 0))
	{
		return;
	}


	fillPattern = InitPattern(ptrToLen,&pos);
	
	if(fillPattern != 0xFF)
	{
		length = (ptrToLen[pos+1] > recLen)?recLen:(ptrToLen[pos+1]-1);	//为防止某些芯片对空指针取下标时 ，芯片异常，这里增加判断
	}

	if(fillPattern == 0xC1)
	{		
		for(i = 0x00;i < recNum; i++)
		{
			WriteEepromEX(fileBodyAddr+(i*recLen),(BYTE *)&ptrToLen[pos+2],length);
				
			SetEepromEX(fileBodyAddr+(i*recLen)+length,ptrToLen[pos+2+length],recLen-length);
		}		
	}
	else if(fillPattern == 0xC2)
	{		
		for(i = 0x00;i < recNum; i++)
		{
			for(spareSzieInOneRecord = recLen;spareSzieInOneRecord>0;spareSzieInOneRecord -= length)
			{
				length = (spareSzieInOneRecord > ptrToLen[pos+1] ) ? ptrToLen[pos+1]:spareSzieInOneRecord;

				WriteEepromEX(fileBodyAddr+(i*recLen)+(recLen-spareSzieInOneRecord),(BYTE *)&ptrToLen[pos+2],length);	
			}
		}	
	}	
	
}


/*
*@description:   v1.0:对于Security attributes 扩展模式(0xAB)模式，cos中没有对应扩展格式的机制，此处是测试5G-SIMCard需求的远程http-rfm创建文件中采用了AB模式，只做简单的处理，
 *				 判断扩展格式中的访问权限对应2F06中的访问权限，进行强制赋值，可能会有问题，后续其他项目不建议使用本函数,不保证正确性                
				 v1.1:modify by zhxx：之前的处理有问题，应该把AB的内容写到一条空记录，返回该记录号，或者返回和2F06文件中相匹配的记录号，5G联通5-2外场测试会远程创建4F09文件，使用AB格式，然后删除4F09文件
 *
 *@param      :  ptrToValue:TLVTLV...数据
 *@param      :  pos:                当前数据的长度
 *
 *@returns    :  0x2F06中的记录号
 *
 *@history    :  create by jiangqb at 20190715
                 modify by zhangxx at 20200821
*/
BYTE FindExtandArr(BYTE *ptrToValue,BYTE pos)
{
	BYTE idata result;

	result = FindTag(&ptrToValue[0],0xAB, pos);								

	if(result == FALSE)
	{
		return 	FALSE;	
	}
	
	memcpy(&tempBuf[0x50],&ptrToValue[result],ptrToValue[result]+1);
	
	result = CompareExpandValueInFile((BYTE * )&tempBuf[0x50]);
	
	return result;
}

