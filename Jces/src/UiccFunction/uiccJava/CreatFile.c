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
			V1.9 定义BERTLVCmd，将BER-TLV结构的处理放入该宏内 --20111027 by chenl
			V1.10 :修改函数CreateDFFile ，修改ArrName，此字节表示由2F或者6F修改为此字节的bit 7表示：bit7 = 0表示2F,bit7 = 1表示6F,bit0-bit4表示EF文件的RFM权限（read update write deactive active）	@20140727 xuyz
			       修改函数CreateEFFile(，修改对arrName的处理，其低4bit用于保存当前EF 的RFM权限 @20140727 modify by xuyz
			
			V1.11 修改函数DeleteFile,ResizeFile 修改断电失败问题 ；修改Resize APDU 指令过程中的所有访问 E2操作都是断电 保护写操作 @20150513

			********************new for uiccapi***************************************************************************
			V1.12 由于UICC API也支持创建文件，为简化代码，将Create  file\DeleteFile\ResizeFile命令进行优化并支持文件大小扩展功能@20151117
			V1.13 修改函数CreateFile，对于UICC API屏蔽掉验证 ValueBit(G_SystemStatus,B_cmk)  @20151201 by  xuyz
			V1.14 修改函数CreateDFFile、 CreateEFFile，修改securemove时的tag处理@20151203 by xuyz
			V1.15 修改函数CreateEFFile,增加91tag表示是否存在备份文件       @20190808 by yangjian
			
 */ 
#include "uicc_main.h"
//#include  "main.h"
//#include "resume.h"
#include "fileSystem.h"
#include "gsmpublic.h"
#include "fileAction.h"
#include "uicc_interface.h"
#include "stkhead.h"
#include "Stkalert.h"
#include "Supi.h"
#include <string.h>
//#include <stdlib.h>
#include <uicc_error.h>
#include "ProfileInterpreter.h"
#include "profileApi.h"
#include "EUICCMain.h"
#include "Interface_LTEFromPlatform.h"
#include "cardManager.h"
#include "uicc_main.h"
#include "EUICCInterface.h"
#include "VMAccessEeprom.h"
#include  "CompactManage.h"
#include "STKFileView.h"
#include "ToolkitAPIPublic.h"
#include "euiccConfig.h"	//remove warning,dengyiwen,2022/9/21
#include "PETemplateFileProcess.h"  //remove warning,dengyiwen,2022/9/21

void ModifyFileSize(NewResizeAntiTearing  protectDo,UWORD modifyLen)
{
	BYTE idata i;

	for(i = 0; i< DF_Depth ; i++)
	{
		if(protectDo.ParentAddr[i] == FALSE)
		{
			break;
		}

		VMWriteBuf(protectDo.ParentAddr[i]+F_UsedSize_Offset, (BYTE *)&protectDo.NewUsedSize[i], sizeof(UWORD));	//写各父目录的已用空间字段
		
		if(modifyLen > 0)
		{
			VMWriteBuf(protectDo.ParentAddr[i]+F_MSize_Offset, (BYTE *)&protectDo.NewMaxSize[i], sizeof(UWORD));	//写各父目录的已用空间字段
		}
	}
}

BYTE PreModifyFileSize(NewResizeAntiTearing * protectDo,UWORD currentDfAddr,s32 modifySize,BYTE mode,UWORD modifyLen)
{

	BYTE idata i;
	UWORD xdata parentfileaddr,parentfilesize;

	for(i=0; i < DF_Depth; i++)
	{
		if(i == 0)
		{
			parentfileaddr = currentDfAddr;
		}
		else
		{
			parentfileaddr = GetParentOrMapFileAddr(parentfileaddr,F_ParentName_Offset);
		}

		parentfilesize = GetFileHeadUsedSize(parentfileaddr);

		protectDo->ParentAddr[i]  = parentfileaddr;

		protectDo->NewUsedSize[i] = modifySize + parentfilesize;
		
		if(modifyLen > 0)
		{
			protectDo->NewMaxSize[i] = modifySize + parentfilesize + modifyLen;
		}

#if (EUICC_ONLY||SE_AND_EUICC)
		if(((parentfileaddr == E_FILE_START) && (mode == NOT_PROFILE_MODE)) || ((mode != NOT_PROFILE_MODE)&&(parentfileaddr == G_CreatingMFOffset)))
#else
		if(parentfileaddr == E_FILE_START)
#endif
		{
			break;
		}
	}

	return i;
}


/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	GetMFUsedSize                                                                                                                
                                                                                                                                                
  Input parameter: BYTE * ptrToResult(LV 结构)
                                                                                              
  Output parameter: FALSE/RefNum                                                                                                      
                                                                                                                                                
  Description:    比较文件EF_ARR 中的Expand value ，没有相同的值则添加记录，返回结果

  Coder:			ZXL         Tester:   

  Current status:      

-------------------------------------------------------------------------------------------------------------*/
UWORD GetMFUsedSize(void)
{
	FileIdType xdata fileHead;
	UWORD xdata MFUsedSize;
	
	MFUsedSize = 0;

	if((E_FILE_START == 0)||(E_FILE_START == 0Xffffffff))
	{
		return 0;
	}

	ReadEEPromCache((BYTE *)&fileHead,E_FILE_START,sizeof(fileHead));
	
	if((fileHead.FileId == DF_MF)&&((fileHead.FileType&MFTYPE) == MFTYPE))  //MF′??ú
	{												
		MFUsedSize = GetFileHeadUsedSize(E_FILE_START);
	}

	return MFUsedSize;
}
/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	CompareExpandValue                                                                                                                
                                                                                                                                                
  Input parameter: BYTE * ptrToResult(LV 结构)
                                                                                              
  Output parameter: FALSE/RefNum                                                                                                      
                                                                                                                                                
  Description:    比较文件EF_ARR 中的Expand value ，没有相同的值则添加记录，返回结果

  Coder:			ZXL         Tester:   

  Current status:  

  History:
				  V1.0 create file
				  V1.1 AB/8C格式存储在DF_Toolkit_P/EF_ARR_PRIVATE文件中，并使用ArrNum最高2bit是1来区分
				  V1.2 没有找到空记录 ，返回0xFE，文件权限固定为ADM

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
		ReadLFBuf(arrFileAddr, i+1, 0x00, (BYTE xdata *)&ptrToResult[ptrToResult[0] + 1], ptrToResult[0]);		//Reeplace  RecData Buf with ptrToResult Buf

		if(ecpMem_Compare((BYTE xdata *)&ptrToResult[ptrToResult[0] + 1],&ptrToResult[1],ptrToResult[0]) == EQUAL)
		{
			return (i+1);
		}

		if((ptrToResult[ptrToResult[0] + 1]  == 0xFF)&&(flag == 0xFF))  //保留查找到第一条为空的记录，若在文件中没有找到相同的记录，则将新信息写入该条记录中
		{			
			flag = i;
		}
	}

	if(flag == 0xFF)	    // 没有找到空记录 ，返回0xFE，文件权限固定为ADM
	{
		return ARR_RECNUM_ADM;
	}
	else
	{
		WriteLFBuf(arrFileAddr, flag+1,0, &ptrToResult[1],ptrToResult[0]);
	
		return (flag+1);
	}
}

#if UICC_AB8C_ARR_PRIVATE
/*
*created by xuweihua@20211106 AB/8C规则存储在私有文件2700/AF06，增加宏开关UICC_AB8C_ARR_PRIVATE
*/
BYTE CompareExpandValueInFilePrivateArr(BYTE * ptrToResult)
{
	BYTE  idata recLenNum,recLenLgth;
	BYTE  idata i,flag;
	UWORD xdata arrFileAddr;
	WORD  idata pos;
	
	arrFileAddr = SelectLFFile(DF_Toolkit_P,EF_ARR_PRIVATE);

	if(arrFileAddr == FALSE)
	{
		return FALSE;
	}
	
	recLenLgth = GetLFRecNumLgth(arrFileAddr,FileLgth);

	if(recLenLgth < (ptrToResult[1]+2))			
	{
		return FALSE;
	}

	recLenNum = GetLFRecNumLgth(arrFileAddr,FileNum);

	flag = 0xFF;  //该标志即标记当前已经查找到空记录,又标记着空记录的记录号

	pos = TEMP_BUF_SIZE-MaxUseLen;

	for(i=1; i<=recLenNum; i++)
	{				
		ReadLFBuf(arrFileAddr, i, 0, (BYTE xdata *)&tempBuf[pos], recLenLgth);		//Reeplace  RecData Buf with ptrToResult Buf

		if((tempBuf[pos] ==  ptrToResult[0])&&(tempBuf[pos+1] ==  ptrToResult[1])&&(ecpMem_Compare((BYTE xdata *)&tempBuf[pos+2],&ptrToResult[2],ptrToResult[1]) == EQUAL))
		{
			//ArrNum最高2bit是1表示存储在EF_ARR_PRIVATE文件中
			return (A_PRIVATEARR_REC_FLAG|i);
		}

		if((tempBuf[pos]  == 0xFF)&&(flag == 0xFF))  //保留查找到第一条为空的记录，若在文件中没有找到相同的记录，则将新信息写入该条记录中
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
		WriteLFBuf(arrFileAddr, flag,0, &ptrToResult[0],ptrToResult[1]+2);
	
		//ArrNum最高2bit是1表示存储在EF_ARR_PRIVATE文件中
		return (A_PRIVATEARR_REC_FLAG|flag);	
	}
}
#endif
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
  history: modify by xuweihua@20211106 AB/8C规则存储在私有文件2700/AF06，增加宏开关UICC_AB8C_ARR_PRIVATE
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
					ecpMem_Copy(&resultBuf[k+3], (BYTE CONST *)&AccessContion[0], sizeof(AccessContion));

					resultBuf[k+7] = tmpResult;
					
					k = k+11;					
				}
			}
		}
	}					

	resultBuf[0] = k-1;		//???

	if(resultBuf[0] >= MaxUseLen)
	{
		return FALSE;
	}
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
 *               modify by xuweihua@20211106 AB/8C规则存储在私有文件2700/AF06，增加宏开关UICC_AB8C_ARR_PRIVATE
*/
BYTE FindArrNum(BYTE *ptrToValue,BYTE pos)
{
	BYTE idata result;

	result = FindTag(&ptrToValue[0],0x8C,pos);									//Compact mode for RFM		

	if(result == FALSE)
	{
		return 	FALSE;	
	}
#if UICC_AB8C_ARR_PRIVATE
	result = CompareExpandValueInFilePrivateArr(&ptrToValue[result-1]);
#else
	result = CompactToExpand((BYTE *)&ptrToValue[result+1],(BYTE * )&tempBuf[TEMP_BUF_SIZE-MaxUseLen]);

	if(result == FALSE)
	{
		return 	FALSE;
	}

	result = CompareExpandValueInFile((BYTE * )&tempBuf[TEMP_BUF_SIZE-MaxUseLen]);

	if(result == FALSE)
	{
		return FALSE; 
	}
#endif	
	return result;
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
				 modify by xuweihua@20211106 AB/8C规则存储在私有文件2700/AF06，增加宏开关UICC_AB8C_ARR_PRIVATE
*/
BYTE FindExtandArr(BYTE *ptrToValue,BYTE pos)
{
	BYTE idata result;

	result = FindTag(&ptrToValue[0],0xAB, pos);								

	if(result == FALSE)
	{
		return 	FALSE;	
	}
#if UICC_AB8C_ARR_PRIVATE
	result = CompareExpandValueInFilePrivateArr(&ptrToValue[result-1]);
#else	
	if((ptrToValue[result])>= MaxUseLen )
	{
		return FALSE;
	}
		
	ecpMem_Copy(tempBuf+(TEMP_BUF_SIZE-MaxUseLen),&ptrToValue[result],ptrToValue[result]+1);
	
	result = CompareExpandValueInFile((BYTE * )&tempBuf[TEMP_BUF_SIZE-MaxUseLen]);
#endif	
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

	if((ptrToLen != NULL)&&(ptrToLen[0] != 0xFF))
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
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	InitTransFile                                                                                                                   
                                                                                                                                                
  Input parameter: 	BYTE * ptrToLen : point to NULL or L of the TLV(Tag = 'A5')
  					
					UWORD fileBodyAddr
					
					WORD bodySize
					                                                                                                   
  Output parameter: void                                                                                                      
                                                                                                                                                
  Description:      Init the craeted TF

  Coder:			ZXL         Tester:   Caibo

  Current status:   modify order at 2007-06-25  
  history: modify by xuweihua@20211106 memcpy修改为ecpMem_Copy

-------------------------------------------------------------------------------------------------------------*/
void InitTransFileRAM(BYTE * RamBuffer,BYTE * ptrToLen,WORD bodySize)
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
			
		ecpMem_Copy(RamBuffer,(BYTE *)&ptrToLen[pos+2],length);			
		ecpMem_Set(RamBuffer+length,ptrToLen[pos+2+length],bodySize-length);
	}
	else if(fillPattern == 0xC2) 
	{			
		for(spareSzie = bodySize;spareSzie>0;spareSzie =spareSzie- length)
		{
			length = (spareSzie>ptrToLen[pos+1])?ptrToLen[pos+1]:(BYTE)(spareSzie&0x00FF);
			ecpMem_Copy(RamBuffer+(bodySize-spareSzie),(BYTE *)&ptrToLen[pos+2],length);								
		}
	}
	else if(fillPattern == 0xC4)
	{		
		ecpMem_Set(RamBuffer, ptrToLen[pos+2], bodySize);  //文件体初始化成ptrToLen[pos+2]
	}
	else			      
	{		 
		ecpMem_Set(RamBuffer,0xFF,bodySize);
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
  history: modify by xuweihua@20211106 memcpy修改为ecpMem_Copy
-------------------------------------------------------------------------------------------------------------*/
void InitRecordFileRAM(BYTE * RamBuffer,BYTE * ptrToLen,BYTE recNum, BYTE recLen)
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

	length = 0;   //intial value

	if(ptrToLen != NULL)
       {
		length = (ptrToLen[pos+1] > recLen)?recLen:(ptrToLen[pos+1]-1);	
	}

	if(fillPattern == 0xC1)
	{		
		for(i = 0x00;i < recNum; i++)
		{
			ecpMem_Copy(RamBuffer+(i*recLen),(BYTE *)&ptrToLen[pos+2],length);
				
			ecpMem_Set(RamBuffer+(i*recLen)+length,ptrToLen[pos+2+length],recLen-length);
		}		
	}
	else if(fillPattern == 0xC2)
	{		
		for(i = 0x00;i < recNum; i++)
		{
			for(spareSzieInOneRecord = recLen;spareSzieInOneRecord>0;spareSzieInOneRecord -= length)
			{
				length = (spareSzieInOneRecord > ptrToLen[pos+1] ) ? ptrToLen[pos+1]:spareSzieInOneRecord;

				ecpMem_Copy(RamBuffer+(i*recLen)+(recLen-spareSzieInOneRecord),(BYTE *)&ptrToLen[pos+2],length);	
			}
		}	
	}	
	else if(fillPattern == 0xC4)
	{	
		ecpMem_Set(RamBuffer, ptrToLen[pos+2], actual);  //设置成ptrToLen[pos+2]
	}
	else
	{		
		ecpMem_Set(RamBuffer, 0xFF, actual);  //清空文件体

		if(fillPattern == 0xC3)
		{
			if(length < recLen)
			{
				length ++;
			}

			ecpMem_Copy(RamBuffer,(BYTE *)&ptrToLen[pos+2],length);
		}
	}		
}
#endif // UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
/*
*@function : InitTransOrRecordFile
*
*@description  :初始化透明文件或者记录文件
*
*@param   :fileBodyAddr :记录文件体的地址
*@param    :prTolen:初始化数据
*@param    :recNum:记录个数
*@param    :recLen:记录长度
*
*@author
*
*@history: V1.0 create by xwh 
*          V1.1 增加参数startOffset，在Resize文件时，初始化文件只初始化新增加的空间，不改动原来的文件内容,by xwh 20201123
*/
void InitTransOrRecordFile(UWORD fileAddr,WORD startOffset,BYTE * initBuf,BYTE heRecNo,BYTE mode)
{
	BYTE  idata fileType;
	//BYTE idata recNum;  //remove warning,dengyiwen,2022/9/22
	WORD idata fileSize;
	BYTE  idata fileRecNumLgth[2];
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	WORD  idata efId,calcCrc;
	BYTE  idata fileNo;
	BYTE  idata RamBuffer[PFPageSize];
#endif

	fileSize = 0;
	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);
	
	if((fileType&0x37) == TRANTYPE)
	{
		fileSize = GetFileHeadWord(fileAddr,F_SpaceSize_Offset);		
	}
	else
	{
		ReadEeprom((BYTE idata *)&fileRecNumLgth[0],fileAddr+F_RecNum_Offset,sizeof(fileRecNumLgth));		
	}

#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	if(IF_IsHeFlashFile(fileAddr))
	{
		fileNo = GetFileHeadByte(fileAddr, F_Pointer_Offset);
		ReadEeprom((BYTE *)&efId,fileAddr,sizeof(WORD));	
		
	   	ecpMem_Copy(RamBuffer, (unsigned char *)(HE_FLASH_BASE+(heRecNo-1)*PFPageSize), PFPageSize); //read the record

		SetUWord(&RamBuffer[RamBuffer_CounterOffset],0x00000000);        //set the init counter to 0			
		SetWord(&RamBuffer[RamBuffer_EfNameOffset],efId);  
		RamBuffer[RamBuffer_FileNoOffset] = fileNo;

		if((fileType&0x37) == TRANTYPE)
		{
			InitTransFileRAM(RamBuffer,initBuf,fileSize);
		}
		else if((fileType&0x37) == LINTYPE)
		{
			InitRecordFileRAM(RamBuffer,initBuf,fileRecNumLgth[0],fileRecNumLgth[1]);
		}
		else if((fileType&0x37) == CYCTYPE)
		{
			if(startOffset == 0x00)
			{
				ecpMem_Set(RamBuffer,0x01,0x01);//初始化循环文件的循环指针字节
			}
			InitRecordFileRAM(RamBuffer+1,initBuf,fileRecNumLgth[0],fileRecNumLgth[1]);
		}

		calcCrc = Hardware_CalCRC((u8 *)&RamBuffer[0],PFPageSize-2);    

		SetWord(&RamBuffer[RamBuffer_CrcOffset],calcCrc);  
		
		WriteHEFlashPage(HE_FLASH_BASE+(heRecNo-1)*PFPageSize,RamBuffer);
		
		return;
	}
#endif
	
	if(fileType == TRANTYPE)
	{
		InitTransFileEX(fileAddr+sizeof(TranFileHead)+startOffset,initBuf,fileSize-startOffset,mode);
	}
#if UICC__FUNCTION__BERTLVCmd
	else if(fileType == BERTLVTYPE)  //BER-TLV
	{
		nvmFillNoAtomic(MEMtPN8(fileAddr+sizeof(BERTLVFileHead)+startOffset),0xFF,GetTFSize(fileAddr)-startOffset);	   //init the maxsize is '0xFF'
	}
#endif	
	else
	{
		 if((fileType&0x37) == CYCTYPE)
		 {
		 	 if(startOffset == 0x00)
		 	 {
			 	SetEepromEX(fileAddr+sizeof(RecordFileHead),1,1);
		 	 }
		 	 if(fileRecNumLgth[1] != 0)//记录长度不是0才做以下操作，UICC API测试，创建循环文件，filesize是0的测试
		 	 {
			    InitRecordFileEX(fileAddr+sizeof(RecordFileHead)+1+startOffset,initBuf,fileRecNumLgth[0]-(startOffset/fileRecNumLgth[1]),fileRecNumLgth[1],mode);
		 	 }
		 }
		 else
		 {
			 if(fileRecNumLgth[1] != 0)
			 {
			   InitRecordFileEX(fileAddr+sizeof(RecordFileHead)+startOffset,initBuf,fileRecNumLgth[0]-(startOffset/fileRecNumLgth[1]),fileRecNumLgth[1],mode);
			 }
		 }
	}
}
/*
 *@description:创建文件子函数
 *
 *@param:      currentDfAddr :当前DF地址
 *@param       buf    :FCP
 *@param       len    :FCP  长度
 *@param       fileName    :要创建的文件FileID
 *@param       filetype    :要创建的文件类型
 *
 *@Returns     创建结果
 *
 *@history     create by zzj at 0131214 16:29
 *           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
 *                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
UWORD createfile_sub(UWORD currentDfAddr,BYTE* buf,BYTE len,WORD fileName,BYTE filetype)
{
	//UWORD xdata dfUsedSize;  //remove warning,dengyiwen,2022/9/22
	UWORD xdata  fileMaxSize;
  	
	//dfUsedSize = GetFileHeadUsedSize(currentDfAddr);  //remove warning,dengyiwen,2022/9/22

   	if(filetype== ADFTYPE)										//create df
  	{
  		  fileMaxSize = commonCreateDFFile(currentDfAddr,fileName,buf,len,NOT_PROFILE_MODE);
  	}
  	else
  	{
  		  fileMaxSize = commonCreateEFFile(currentDfAddr,fileName,buf,len,NOT_PROFILE_MODE);
  	}
		
	//if(fileMaxSize <= E_NotSupport)
	//{
	//	  return fileMaxSize;
	//}

	//return (currentDfAddr+ dfUsedSize);
	return fileMaxSize;
}
/*
*@Function name :	 CreateFile
*
*@Command format:    00    E0    00   00   XX
*
*@Data    content:   FCP
*
*@input parameter:   void
*
*@output parameter:  void
*
*@description:       在当前的APF/DF下，建立DF/ADF/EF
*
*@status:			 加入RFM
*/
void CreateFile(void)
{
	BYTE  idata  result,pos,tag;
	UWORD xdata  fileAddr;
	WORD  idata  fileName;

	if(P1P2Check() ==FALSE)
	{
		return ;
	}

// 	if(VMReadByte(CARD_LIFE_CYCLE) >= CARD_SECURED)         //Mapping_guideLine: For the Issuer Security Domain, if the card Life Cycle State is CARD_LOCKED, a response of '6985' is returned
// 	{
// 		if(ValueBit(G_SystemStatus,B_cmk) == FALSE)	    			  //暂时屏蔽，脚本需要在未校验mfkey状态下直接用指令创建文件
// 		{															  //Api_2_Tkr_Rgfes_Bss_Bsb 直接创建文件 ,为兼容修改，可以将在UICCAPI 中createfile文件时，可以将MFKey的状态置上
// 			SecuritySatisfied();
// 																					   //send 98 04/69 82
// 			return;
// 		}		
// 	}

	ReceiveData();                                   								//receive data from ME
	                               								//receive data from ME
	if(P3 != (RecData[1] +2)) //根据测试脚本增加对P3长度的判断。
	{
		IncorrentParameter();					//Send 6A 80
		return;
	}

	pos = FindTag(&RecData[0],0x62,P3);

	if(pos == FALSE)
	{
		IncorrentParameter();														 //Send 6A 80
		return;
	}

	pos ++;

	result = FindTag(&RecData[pos],0x83,P3-pos);

  	if(result == FALSE)
  	{
		IncorrentParameter();
  		return;
  	}

	fileName = ByteToWord(RecData[result+1+pos],RecData[result+2+pos]) ;

 // 	fileAddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],fileName) ; //如果创建的文件是映射文件,在主体文件没建立的时候，导致会重复创建相同ID的EF。
	
	fileAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum],fileName) ;
	
  	if( fileAddr != FALSE )
  	{
		FileExist();
  		return ;
  	}

	tag = FindTag(&RecData[pos],0x82,P3-pos);              //根据要创建文件的类型判断是否满足操作权限

	if(tag == FALSE)
	{
		IncorrentParameter();														 //Send 6A 80
		return;
	}

   	if((RecData[tag+1+pos]&FILETYPE)== ADFTYPE)										//create df
  	{
  	   	result = AccessCondition(G_CurrentDfAddr[G_ChannelNum],A_CreateDF);
  	}
  	else if(((RecData[tag+1+pos]&FILETYPE)== 0x39)||((RecData[tag+1+pos]&0x37) == TRANTYPE)||((RecData[tag+1+pos]&0x37) == LINTYPE)||((RecData[tag+1+pos]&0x37) == CYCTYPE))
  	{
  		result = AccessCondition(G_CurrentDfAddr[G_ChannelNum],A_CreateEF);
  	}
  	else
  	{
  		IncorrentParameter();
		return;
  	}
	if (result == FALSE)
	{
		SecuritySatisfied();
		return;
	}

	fileAddr = createfile_sub(G_CurrentDfAddr[G_ChannelNum],(BYTE*)&RecData[pos],P3-pos,fileName,RecData[tag+1+pos]&FILETYPE);

	if(fileAddr == FALSE)
	{
		MemoryError();
		return;
	}

	if((fileAddr == E_Parameter) || (fileAddr == E_NotSupport) || (fileAddr == E_InvalidRequest))//add by chentao 配合创建DF不支持88tag修改
	{
		IncorrentParameter();
		return;
	}

	if(fileAddr == E_FileSize)
	{
		NoEnoughSpace();
		return;
	}

	if(fileAddr == E_DfNameExist)
	{
		DfNameExist();
		return;
	}

	if(fileAddr == E_ConditionNotSat)
	{
		ConditionSatisfied();
		return;
	}

	if(ValueBit(G_SystemStatus, B_RfmPerform) == TRUE)   //CreateFile 只有在RFM APDU 和普通 APDU 的情况下才会进入，而对于普通的UICC APDU，不需要保存非共享文件信息，因为此时在公司生产阶段，保存的非共享信息没意义
	{
		IF_ProcessNoShareableFile(fileAddr,fileName,0x00,NULLAPI_Mode);                                                                                       
	}

	SetChannel(fileAddr,fileName,FALSE);

	CommandFinish(0x00);                                  									//send 90 00
}


/*
*@function    :DeleteFile_sub
*
*@description :文件删除子函数
*
*@param       :antiStruct 待删除文件数据特征，其中 ParentAddr/FileAddr/FileId在调用本函数前需要被初始化
*
*@return      :void
*
*@author      :ZZJ
*
*@history     :V1.0:create by ZZJ@20131113
*/
BYTE DeleteFile_sub(UWORD fileAddr, UWORD CurrentDfAddr)
{
	SingleFileHead xdata dfHeader;
	BYTE idata /*result,*/fileType,fileStatus,i;
	UWORD xdata dataSize, mfUsedSize;
	NewResizeAntiTearing xdata protectDo;
	UWORD				xdata dataLen;

#if (EUICC_ONLY||SE_AND_EUICC)
	BYTE xdata temp[PROFILE_MAX_NUMBER];
	UWORD xdata currentMFOffset,currentMFUsedSize;
#endif

	BYTE idata TransactionStarted,secureMoveStarted;	

	TransactionStarted = FALSE;
	secureMoveStarted = FALSE;

	ecpMem_Set((BYTE *)&protectDo, 0x00, sizeof(NewResizeAntiTearing));

	ReadEEPromCache((BYTE *)&dfHeader.UsedSize,CurrentDfAddr+F_UsedSize_Offset,sizeof(SingleFileHead));	

	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

	if(fileType == DFTYPE)
	{
		dataSize = GetFileHeadUsedSize(fileAddr);  //获取DF 的已用空间，该已用空间就是DF的实际占用空间

		dfHeader.ChildDfNum --;
		
		protectDo.FileInfo[2] = GetFileHeadByte(fileAddr,F_DIRNum_offset);	    // 如果删除的是ADF,则同时删除2F00中特定记录
	}
	else
	{
		fileStatus =  GetFileHeadByte(fileAddr,F_Status_Offset);

		if((fileStatus&F_MAP_FILE) != F_MAP_FILE)			//not map file
		{
			dataSize = GetFileTotalSize(fileAddr);
		}
		else
		{
			dataSize = sizeof(TranFileHead);	 //sizeof(TranFileHead) = sizeof(RecordFileHead)
		}
	    
		dfHeader.ChildEfNum --;
	}

	
	i = PreModifyFileSize(&protectDo,CurrentDfAddr, - dataSize,NOT_PROFILE_MODE,0);
	
	if(i == DF_Depth)  //COS 最大支持5级DF，由数组protectDo.ParentAddr[0]决定
	{
		return E_Parameter;
	}
	
	mfUsedSize = GetMFUsedSize();

	dataLen = (E_FILE_START + mfUsedSize)-(fileAddr + dataSize);
	

#if (EUICC_ONLY||SE_AND_EUICC)

	protectDo.ProfileNum = 0;

	VMReadBuf(temp,PROFILE_VALIDTAG_CFG,PROFILE_MAX_NUMBER);    //获取当前所有profile的状态
		
	for(i= 0; i< PROFILE_MAX_NUMBER ; i++)
	{		  
		if(temp[i] != PROFILE_INVALIDFLAG)
		{
			currentMFOffset = VMReadUword(PROFILE_MFOFFSET_CFG + i*sizeof(UWORD)); 
			
			if((currentMFOffset > E_FILE_START)&&(currentMFOffset != 0xFFFFFFFF))
			{
				protectDo.ProfileIndex[protectDo.ProfileNum] = i;
				
				protectDo.ProfileNewMFOffset[protectDo.ProfileNum] = currentMFOffset - dataSize;
								
				currentMFUsedSize = GetFileHeadUsedSize(currentMFOffset);
				
				dataLen += currentMFUsedSize;
				
				protectDo.ProfileNum++;   //找到较大的那个MF 的偏移
			}
		}
	}
#endif
	
    if(dataLen != 0x00)	   //need not move
	{
		secureMoveStarted = TRUE;
		
		//如果事务已开启，又要移动数据，不允许这种操作，报错,by xwh 20200707
		if(isInProgress())
		{
			return E_ConditionNotSat;
		}
	}

	TransactionStarted = IF_BeginTransaction();
    
	if(protectDo.FileInfo[2] != FALSE)  
	{
		DeleteDIRNum(protectDo.FileInfo[2]);	
	}

	if(secureMoveStarted == TRUE)
	{
#define moveDateBuf  (tempBuf + 100)
		SetMovDateBufSrcDstDataLen(moveDateBuf,fileAddr+ dataSize,fileAddr,dataLen);

		VMWriteBuf(E_SECUREMOVE_START, moveDateBuf, SecureMoveStuct_LengthAll);	
#undef moveDateBuf		
	}

	ModifyFileSize(protectDo,0);

#if (EUICC_ONLY||SE_AND_EUICC)
	for(i = 0 ; i < protectDo.ProfileNum; i++)
	{		  
		  VMWriteUword(PROFILE_MFOFFSET_CFG + protectDo.ProfileIndex[i]*sizeof(UWORD),protectDo.ProfileNewMFOffset[i]);
	}
#endif	
	
	IF_CommitTransaction(TransactionStarted);

	if(secureMoveStarted == TRUE)
	{
		StartSecureMoveEeprom(0,0,0);
	}
	
	ReselectPrivateFile();//删除文件时，可能导致私有文件的全局变量地址发生变化，需要对全局变量进行重新赋值

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
	BYTE idata result;
	WORD  idata fileName;
	UWORD xdata fileAddr;

	if(P1P2P3Check(0x02,FALSE) ==FALSE)
	{
		return ;
	}

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
		if(IF_ProcessNoShareableFile(fileAddr,fileName, 0x02,NULLAPI_Mode) == FALSE)  //该文件是非共享文件,且已被其他通道占用
		{
			ConditionSatisfied();	 				// 69 85
			return;
		}
		SetChannel(fileAddr,fileName,FALSE);      //设置EF当前EF
	}
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	//强更新文件不允许delete
	if(IF_IsHeFlashFile(fileAddr))
	{
		ConditionSatisfied();
		return;
	}
#endif
	result = AccessCondition(fileAddr,A_DeleteFile);

	if(result == FALSE)
	{
		SecuritySatisfied();
		return;
	}


	result = DeleteFile_sub(fileAddr, G_CurrentDfAddr[G_ChannelNum]);

	if(result == FALSE)
	{
		MemoryError();		//Send 65 81
		return;
	}

	if(result == E_ConditionNotSat)
	{
		ConditionSatisfied();
		return;
	}

	SetChannel(G_CurrentDfAddr[G_ChannelNum],G_CurrentDfName[G_ChannelNum],FALSE);

	IF_ProcessNoShareableFileListAfterDeleteFile();

	CommandFinish(0x00);
}
/*
*@function:
*
*@description: @description  :resize结束后，需要需要断电保护写更新文件初始值
*
*@param   :fileBodyAddr: 文件体地址
*@param   :ptrTolen:初始化数据体
*@param   :bodySize:透明文件大小
*
*@return  :void
*
*@author  :
*
*@history:modify by chentao@20210722 EPX产业化/INFENION_SLM97
*/
void InitTransFileEX(UWORD fileBodyAddr,BYTE * ptrToLen,WORD bodySize,BYTE mode)
{
	BYTE idata pos;
	BYTE idata fillPattern;
	BYTE idata length;
	WORD idata spareSzie;
	WORD idata remainLen;

	if(bodySize == 0)
	{
		return;
	}
	//remove warning,dengyiwen,2022/9/22
	//life = JcvmConfig_GetSEState();//VMReadByte(CARD_LIFE_CYCLE);

	fillPattern = InitPattern(ptrToLen,&pos);

	if(fillPattern == 0xC1) 
	{
#if (EUICC_ONLY||SE_AND_EUICC)
		ClearBit(G_PEControlContext.IsExistFlag2,ContentRequired);
#endif
		length = (ptrToLen[pos+1] > bodySize)?(BYTE)(bodySize&0x00FF):(ptrToLen[pos+1]-1);
#if (EUICC_ONLY||SE_AND_EUICC)
		if((mode & PROFILE_MODE) == PROFILE_MODE)
		{
			autoWriteEeprom(fileBodyAddr,(BYTE *)&ptrToLen[pos+2],length,PROFILE_MODE);
			
			autoSetEeprom(fileBodyAddr+length,ptrToLen[pos+2+length],bodySize-length,PROFILE_MODE);
			return;
		}		
#endif
		
		WriteEepromEX(fileBodyAddr,(BYTE *)&ptrToLen[pos+2],length);

		remainLen = bodySize-length;

		if(remainLen > 0)
		{
			SetEepromEX(fileBodyAddr+length,ptrToLen[pos+2+length],remainLen);
		}
	}
	else if(fillPattern == 0xC2) 
	{
#if (EUICC_ONLY||SE_AND_EUICC)
		ClearBit(G_PEControlContext.IsExistFlag2,ContentRequired);
#endif
		for(spareSzie = bodySize;spareSzie>0;spareSzie =spareSzie- length)
		{
			length = (spareSzie>ptrToLen[pos+1])?ptrToLen[pos+1]:(BYTE)(spareSzie&0x00FF);
#if (EUICC_ONLY||SE_AND_EUICC)
			if(mode != NOT_PROFILE_MODE)
			{
				autoWriteEeprom(fileBodyAddr+(bodySize-spareSzie),(BYTE *)&ptrToLen[pos+2],length,PROFILE_MODE);
				continue;
			}
#endif
			WriteEepromEX(fileBodyAddr+(bodySize-spareSzie),(BYTE *)&ptrToLen[pos+2],length);
		}
	}
	else
	{
#if (EUICC_ONLY||SE_AND_EUICC)
		if(mode != NOT_PROFILE_MODE)
		{
			autoSetEeprom(fileBodyAddr,0xFF,bodySize,PROFILE_MODE);	
			return;
		}	
#endif

		if(fillPattern == 0xC4)
		{
			SetEeprom(fileBodyAddr, ptrToLen[pos+2], bodySize);  //设置成ptrToLen[pos+2]
		}
		else
		{
		   SetEepromEX(fileBodyAddr,0xFF,bodySize);
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
*@history: modify by xuxiaojun@20201201 代码还原，该函数中不能将文件初始值全部写成0xFF，否则文件的初始值与创建的批处理里面不一致
*          modify by chentao@20210722 EPX产业化/INFENION_SLM97
*/
void InitRecordFileEX(UWORD fileBodyAddr,BYTE * ptrToLen,BYTE recNum, BYTE recLen,BYTE mode)
{
	BYTE idata pos,life;
	BYTE idata i;
	BYTE idata fillPattern;
	BYTE idata length;
	BYTE idata spareSzieInOneRecord;	
	WORD  idata remainLen;
	WORD  idata actual;
	
	if((recNum == 0) || (recLen == 0))
	{
		return;
	}
	
	life = JcvmConfig_GetSEState();//VMReadByte(CARD_LIFE_CYCLE);
	
	actual = recNum*recLen;		    //计算记录文件的实际占用空间
	
	length = 0;   //intial value

	fillPattern = InitPattern(ptrToLen,&pos);
	
	if(fillPattern != 0xFF)
	{
		length = (ptrToLen[pos+1] > recLen)?recLen:(ptrToLen[pos+1]-1);	//为防止某些芯片对空指针取下标时 ，芯片异常，这里增加判断
	}

	if(fillPattern == 0xC1)
	{
#if (EUICC_ONLY||SE_AND_EUICC)
		ClearBit(G_PEControlContext.IsExistFlag2,ContentRequired);
#endif
		for(i = 0x00;i < recNum; i++)
		{
#if (EUICC_ONLY||SE_AND_EUICC)
			if((mode & PROFILE_MODE) == PROFILE_MODE)
			{
				autoWriteEeprom(fileBodyAddr+(i*recLen),(BYTE *)&ptrToLen[pos+2],length,PROFILE_MODE);
				
				autoSetEeprom(fileBodyAddr+(i*recLen)+length,ptrToLen[pos+2+length],recLen-length,PROFILE_MODE);
				continue;
			}		
#endif
			
			WriteEepromEX(fileBodyAddr+(i*recLen),(BYTE *)&ptrToLen[pos+2],length);		
			SetEepromEX(fileBodyAddr+(i*recLen)+length,ptrToLen[pos+2+length],recLen-length);			
		}		
	}
	else if(fillPattern == 0xC2)
	{
#if (EUICC_ONLY||SE_AND_EUICC)
		ClearBit(G_PEControlContext.IsExistFlag2,ContentRequired);
#endif
		for(i = 0x00;i < recNum; i++)
		{
			for(spareSzieInOneRecord = recLen;spareSzieInOneRecord>0;spareSzieInOneRecord -= length)
			{
				length = (spareSzieInOneRecord > ptrToLen[pos+1] ) ? ptrToLen[pos+1]:spareSzieInOneRecord;
#if (EUICC_ONLY||SE_AND_EUICC)
				if(mode != NOT_PROFILE_MODE)
				{
					autoWriteEeprom(fileBodyAddr+(i*recLen)+(recLen-spareSzieInOneRecord),(BYTE *)&ptrToLen[pos+2],length,PROFILE_MODE);
					continue;
				}
#endif
				WriteEepromEX(fileBodyAddr+(i*recLen)+(recLen-spareSzieInOneRecord),(BYTE *)&ptrToLen[pos+2],length);			
			}
		}	
	}
	else
	{
#if (EUICC_ONLY||SE_AND_EUICC)
		if(mode != NOT_PROFILE_MODE)
		{
			if(fillPattern == 0xC4)
			{	
				autoSetEeprom(fileBodyAddr, ptrToLen[pos+2], actual,PROFILE_MODE);  //设置成ptrToLen[pos+2]
			}
			else
			{		
				autoSetEeprom(fileBodyAddr, 0xFF, actual,PROFILE_MODE);  //清空文件体

				if(fillPattern == 0xC3)
				{
					if(length < recLen)
					{
						length ++;
					}

					autoWriteEeprom(fileBodyAddr,(BYTE *)&ptrToLen[pos+2],length,PROFILE_MODE);
				}
			}
			return;
		}	
#endif
		if(life > CARD_OP_READY)
		{
			remainLen = recNum*recLen;	
			
			while(remainLen > 0)
			{
				length = (remainLen > 255)?255:remainLen;

				SetEepromEX(fileBodyAddr,0xFF,length);

				remainLen = remainLen - length;

				fileBodyAddr += length;
			}	
		}
		else
		{
			if(fillPattern == 0xC4)
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
	}	
}
/*
*function     :IF_AccessConditionResizeFile
*
*@description :判断当前fileAddr所指示的文件是否具有resizefile的权限
*
*@param       :fileAddr
*@param       :mode : 0x00 UICC APDU /0x01:ToolkitApp
*
*@return      :TRUE/FALSE
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20131115
*/
BYTE IF_AccessConditionResizeFile(UWORD fileAddr,BYTE mode)
{
	BYTE idata result;

	result = FALSE;
    
	if(mode == 0x00)
	{
		result = AccessCondition(fileAddr, A_ResizeFile);		//暂时考虑权限和Delete的一致	
	}
#if UICC__FUNCTION__UICCAPI
	else		    //对于API模式resize文件，程序执行到这里时，权限已经验证过了
	{
		return TRUE;
	}
#endif

	return result;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       
* @Function     	    :BackDFNameForMoveFalsh
*  
* @Discription        :备份当前DF name和DF的父DF  应用场景：比如RFM执行Resize等移动Falsh的操作，可能会改变DF Address，从RFM等出来后需要从新选址
*  					备份各个通道文件信息以及toolkit应用的文件信息
*  					G_EnvelopHanderAddr 不需要备份，因为私有文件不会被移动
* @See                :
*                                                                                                                                                
* @Parameter          :void                                                                  					                                                                                                             
*
* @Return             :void
*                                                                                                                                                        
* @History		        :V1.0 Creat by LCZ 20180726 
* 						V1.1 modify by chentao 20230310 新增备份非0 通道的文件信息和toolkit应用的文件信息
*
 -------------------------------------------------------------------------------------------------------------------*/  
BYTE BackDFNameForMoveFalsh(WORD DFFileName[PROFILEDELETE_BACKUPFILESYSTEM])
{
	// ChainingValue   G_SIMCurrentDfName G_SIMCurrentEfName
	BYTE idata channelNum;
  //4*MAX_LOGICAL_CHANNELS

	ecpMem_Set(DFFileName,0,PROFILEDELETE_BACKUPFILESYSTEM*sizeof(WORD));

	for(channelNum = 0; channelNum < 4;channelNum++)
	{
		if(G_CurrentDfAddr[channelNum] != FALSE)
		{
			VMReadBuf((BYTE *)&DFFileName[channelNum*4],G_CurrentDfAddr[channelNum]+ F_ParentName_Offset,sizeof(WORD)*2);

			DFFileName[channelNum*4+2] = G_CurrentDfName[channelNum];

			DFFileName[channelNum*4+3] = G_CurrentEfName[channelNum];
		}
	}

	if(G_ToolkitCurrentDfName != FALSE) //toolkit Applet, 这里也得区分通道
	{
		VMReadBuf((BYTE *)&DFFileName[4*4],G_ToolkitCurrentDfAddr + F_ParentName_Offset,sizeof(WORD)*2);

		DFFileName[4*4+2] = G_ToolkitCurrentDfName;

		DFFileName[4*4+3] = G_ToolkitCurrentEfName;
	}


	return TRUE;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       
* @Function     	    :ReloadDFOrEFAddr
*  
* @Discription        :从新SetChannel DF EF地址,场景:RFM或者API中执行Resize等会改变DF或者EF地址，需要重新选址，配合BackDFNameForMoveFalsh函数使用
*
* @See                :
*                                                                                                                                                
* @Parameter          :void                                                                  					                                                                                                             
*
* @Return             :void
*                                                                                                                                                        
* @History		        :V1.0 Creat by LCZ 20180726 
* 						V1.1 modify by chentao 20230310 新增还原非0 通道的文件信息和toolkit应用的文件信息
*
 -------------------------------------------------------------------------------------------------------------------*/  
void ReloadDFOrEFAddr(WORD fileName[PROFILEDELETE_BACKUPFILESYSTEM])
{
	UWORD xdata tmpAddr;
	BYTE idata backChannelNum, channelNum;
	
	backChannelNum = G_ChannelNum;

	for(channelNum = 0;channelNum<4;channelNum++)
	{
		G_ChannelNum = channelNum;

		if(fileName[channelNum*4] != FALSE)
		{
			tmpAddr = SearchFile(E_FILE_START,fileName[channelNum*4]);

			tmpAddr = SearchFile(tmpAddr,fileName[channelNum*4+1]);

			if(fileName[channelNum*4+2] != FALSE)
			{
				tmpAddr = SearchFile(tmpAddr,fileName[channelNum*4+2]);

				SetChannel(tmpAddr,fileName[channelNum*4+2],FALSE);  //重新设置DF的地址

				if(fileName[channelNum*4+3] != 0x00)           //复位没有其他操作进行RFM操作时候 EF 和EfAddr是False
				{
					tmpAddr = SelectFile(tmpAddr,fileName[channelNum*4+3]);

					SetChannel(tmpAddr,fileName[channelNum*4+3],FALSE);  //重新设置EF的地址
				}
			}

		}
	}
		if(fileName[4*4] != FALSE)
		{
			tmpAddr = SearchFile(E_FILE_START,fileName[4*4]);

			if(tmpAddr != FALSE)
			{
				tmpAddr = SearchFile(tmpAddr,fileName[4*4+1]);

				if((fileName[4*4+2] != 0x00) != FALSE)
				{
					tmpAddr = SearchFile(tmpAddr,fileName[4*4+2]);

					Toolkit_SetChannel(tmpAddr,fileName[4*4+2],FALSE,UICCAPI_Mode);

					if(fileName[4*4+3] != 0x00)
					{
						tmpAddr = SelectFile(tmpAddr,fileName[4*4+3]);
						Toolkit_SetChannel(tmpAddr,fileName[4*4+3],FALSE,UICCAPI_Mode);
					}
				}
			}
		}


	G_ChannelNum = backChannelNum;
	
	return;
}
/*
 *@Function: AllocSpaceNotEnough
 *
 *@Descrip:	 从startAddr处开始遍历 ，找到第一个有效块或者无效块 （高地址往低地址倒过来遍历heap）
 *          
 *@See：    startAddr - 1为当前块的最高地址 
 *			 
 *@Return:   void
 *
 *@Author:   xwh 20180502
 *
 *@History:  V1.0---creat by xwh
 *
 *                V1.1---修改判断条件，通过GetMaxProfileMFEnd 判断空间是否足够分配, by xwh 20190311
 */
BYTE AllocSpaceEnough(UWORD assignSize)
{
	if((assignSize >= GetHeapBase()) || ((GetMaxProfileMFEnd() + assignSize) > GetHeapBase())) //地址空间不足，此处判断要避免溢出的情况
	{
		return FALSE;
	}
	
	return TRUE;
}
/*
*@function      :publicResizeFile
*
*@description   :该函数主要用于处理UICC APDU级以及Tollkit APP级的resizefile的处理
*
*@param         :&ptrToValue
*@param         :pos
*@param         :mode :UICC APDU: 0x00/ToolkitApp:0x01
*
*@return        :StatusSW1SW2
*
*@author        :xuyz
*
*@history       :V1.0 :create by xuyz@20131115
*               :V1.1 :modify by jiangxiaohui@20210426 EPX/EPX_SUITE/CHIP/THC_THD89/RELEASE/RSP_TEST_RELEASE_20210426
*               :V1.2 :修改计算move长度的bug,modifyEfParAddr替换成modifyEfAddr,by xwh 20211110
*/
WORD publicResizeFile(BYTE * ptrToValue,BYTE pos,BYTE mode,UWORD fcpData)
{
	BYTE 				idata tmp, fileStatus, result, i;
	BYTE 				xdata recNum[2];
	WORD 				idata parId1;
	UWORD 			    xdata oldfileSize;
	NewResizeAntiTearing	xdata protectDo;
	UWORD				xdata dataLen;
	RecordFileHead xdata tmpRecordFileHead;
	TranFileHead xdata tmpTranFileHead;
	UWORD xdata newFileSize;
	UWORD xdata mfUsedsize;

	UWORD  xdata remainLen;
	UWORD xdata  CurrentEfHeadAddr;
	UWORD xdata modifyEfAddr,modifyEfParAddr,tmpAddr;
	WORD  idata fileName[PROFILEDELETE_BACKUPFILESYSTEM];        //前3备份 DFname   最后备份EF name
	BYTE idata resizeMapFlag;

#if (EUICC_ONLY||SE_AND_EUICC)
	BYTE xdata temp[PROFILE_MAX_NUMBER];
	UWORD xdata currentMFOffset,currentMFUsedSize;
#endif

	BYTE idata TransactionStarted,secureMoveStarted;

	tmpAddr = FALSE;
	TransactionStarted = FALSE;
	secureMoveStarted = FALSE;	
	
	mfUsedsize = GetMFUsedSize();

	result = FindTag(&ptrToValue[0],0x80,pos);					//File Size

	if(result == FALSE)
	{
		return E_Parameter;
	}
	
	if(ptrToValue[result] == 0x01)				  //UICCAPI Api_2_Tkr_RgfeSo 测到 800103 ，需要支持01长度的fileSize
	{
		newFileSize = ptrToValue[result+1];
	}
	else if(ptrToValue[result] == 0x02)
	{
		newFileSize = BuildWord(&ptrToValue[result+1]);//((buf[result+1] << 8) & 0xFF00) | buf[result+2];
	}
	else
	{
		return 0x6984;
	}
	
	ecpMem_Set((BYTE *)&protectDo, 0x00, sizeof(NewResizeAntiTearing));

	CurrentEfHeadAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);  //获取EF的实际地址

	fileStatus = GetFileHeadByte(CurrentEfHeadAddr,F_Status_Offset);  //判断EF是否为映射文件
	
	resizeMapFlag = 0;  //当前resize的是否是map文件

	if((fileStatus & F_MAP_FILE) == F_MAP_FILE)
	{
		VMReadBuf((BYTE *)&fileName[0],CurrentEfHeadAddr + F_MapFile_Offset,sizeof(WORD)*3); 

		tmpAddr = SearchFile(E_FILE_START,fileName[0]);
		
		if(tmpAddr != FALSE)
		{
			tmpAddr = SearchFile(tmpAddr,fileName[1]);					  	
		}
				
		modifyEfParAddr = tmpAddr;  //保存当前待RESIZE文件的DF地址
		
		if(tmpAddr != FALSE)
		{
			tmpAddr = SearchFile(tmpAddr,fileName[2]);				
		}       
		
		if(tmpAddr == FALSE)
		{
			return 0x6982;
		}
		
		modifyEfAddr = tmpAddr;
		
		resizeMapFlag = TRUE;
		
		BackDFNameForMoveFalsh(fileName);
		
	}
	else
	{
		modifyEfAddr = G_CurrentEfAddr[G_ChannelNum];
		
		modifyEfParAddr = G_CurrentDfAddr[G_ChannelNum];
	}
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	//强更新文件不允许resize
	if(IF_IsHeFlashFile(modifyEfAddr))
	{
		return 0x6985;
	}
#endif	

#if High_Update_InRAM_New
	//强更新RAM文件不允许resize.
	if(IF_UpdateWithRAM_New(modifyEfAddr)!= FALSE)
	{
		return 0x6985;
	}
#endif

	fileStatus = GetFileHeadByte(CurrentEfHeadAddr,F_Type_Offset);   //若当前地址所指向的文件并不是待查找的文件，则只有在当前地址所指向的文件为DF时，才继续往下查找，否则直接退出

	if(fileStatus == LINTYPE)
	{
		VMReadBuf((BYTE  *)&recNum[0],modifyEfAddr + F_RecNum_Offset, sizeof(recNum));

		oldfileSize = recNum[0] * recNum[1];		//记录文件需要计算文件的实际空间
	}
	else if(fileStatus == TRANTYPE)
	{
		oldfileSize = GetTFSize(modifyEfAddr);
	}
	else
	{
		if(G_CommandMode == Command_Mode_3G)
		{
			return 0x6981;
		}
		else
		{
			return 0x9408;
		}
	}
	
	result = IF_AccessConditionResizeFile(modifyEfAddr, mode);

	if(result == FALSE)      //原先直接返回状态字6982，和E4等指令在没有校验权限的情况下返回状态字不一致（A0D400000A/A0E4000002），故修改
	{
		if(G_CommandMode == Command_Mode_3G)
		{
			return 0x6982;
		}
		else
		{
			return 0x9804;
		}
	}

	if(newFileSize == oldfileSize)  //空间没有变化
	{
		return 0x9000;
	}
	

	i = PreModifyFileSize(&protectDo,modifyEfParAddr,-oldfileSize + newFileSize,NOT_PROFILE_MODE,0);
	
	if(i == DF_Depth)  //COS 最大支持5级DF，由数组protectDo.ParentAddr[0]决定
	{
		return 0x6A80;
	}
	
	if(fileStatus == LINTYPE)
	{
		if((newFileSize % recNum[1]) != 0x00)	 //新的纪录大小不是记录长度的整数倍
		{
			return 0x6A80;
		}
		
		if((newFileSize / recNum[1])>254) //102221 8.2.2.2  记录文件 记录个数最大254 长度 255 循环文件 记录个数254  大小254  文件记录的大小应该在创建时候判断
		{
			return 0x6A80;
		}
	}
	
	if(oldfileSize > newFileSize)						//减少空间
	{																									//对于DF的空间缩减只需要判断当前DF的剩余空间
		if(fileStatus == LINTYPE)
		{
			if(newFileSize < recNum[1])		//至少有一条记录的空间,记录文件的空间不能够缩减至0
			{
				return 0x6A80;
			}		
		}
  		
	}
	else                                                                      	//增加空间
	{
		if(AllocSpaceEnough(newFileSize-oldfileSize) == FALSE)        //卡片空间不够移动
		{
			return 0x6A84;
		}
	}

	if(fileStatus == LINTYPE)
	{
		recNum[0] = newFileSize / recNum[1];		//计算新的记录数
		
		VMReadBuf((BYTE *)&tmpRecordFileHead,modifyEfAddr,sizeof(RecordFileHead));
		
		tmpRecordFileHead.RecNumber = recNum[0];
		
		ecpMem_Copy(&protectDo.FileInfo[0], (BYTE *)&tmpRecordFileHead,sizeof(tmpRecordFileHead));

	}
	else
	{		
		parId1 = newFileSize;
		
		VMReadBuf((BYTE *)&tmpTranFileHead, modifyEfAddr,sizeof(tmpTranFileHead));
		
		tmpTranFileHead.SpaceSize = parId1;
		
		ecpMem_Copy(&protectDo.FileInfo[0], (BYTE *)&tmpTranFileHead,sizeof(tmpTranFileHead));
	
	}
	   
	if(newFileSize  > oldfileSize) //如果是增加EF的空间
	{
		tmp = FindTag((BYTE *)&ptrToValue[0],0xA5,pos);			//处理文件的初始值字段,没有断电保护
		
		if(tmp == FALSE)
		{
			tmp = FindTag((BYTE *)&ptrToValue[0],0x85,pos);
		}

		if((tmp == FALSE)||(ptrToValue[tmp] > 0x2F))
	 	{	
			protectDo.FileInfo[0x10] = 0xFF;  //不去初始化空间，原始内容保持不变
		}
		else
		{
			ecpMem_Copy(&protectDo.FileInfo[0x10],&ptrToValue[tmp] , ptrToValue[tmp]+1);
		}

	}
	
	oldfileSize = oldfileSize + sizeof(TranFileHead);		//对于EF的已用空间需要计算文件头的长度

	newFileSize = newFileSize + sizeof(TranFileHead);
		
	dataLen = E_FILE_START + mfUsedsize - modifyEfAddr - oldfileSize;      //计算需要进行移动的数据长度
	
#if (EUICC_ONLY||SE_AND_EUICC)

	protectDo.ProfileNum = 0;

	VMReadBuf(temp,PROFILE_VALIDTAG_CFG,PROFILE_MAX_NUMBER);    //获取当前所有profile的状态
		
	for(i= 0; i< PROFILE_MAX_NUMBER ; i++)
	{		  
		if(temp[i] != PROFILE_INVALIDFLAG)
		{
			currentMFOffset = VMReadUword(PROFILE_MFOFFSET_CFG + i*sizeof(UWORD)); 
			
			if((currentMFOffset > E_FILE_START)&&(currentMFOffset != 0xFFFFFFFF))
			{
				protectDo.ProfileIndex[protectDo.ProfileNum] = i;
				
				protectDo.ProfileNewMFOffset[protectDo.ProfileNum] = currentMFOffset + newFileSize - oldfileSize;
								
				currentMFUsedSize = GetFileHeadUsedSize(currentMFOffset);
				
				dataLen += currentMFUsedSize;
				
				protectDo.ProfileNum++;   //找到较大的那个MF 的偏移
			}
		}
	}
#endif	

	if(dataLen != 0x00)							//如果EF或者DF下面还有其它文件
	{
		secureMoveStarted = TRUE;
		
		//如果事务已开启，又要移动数据，不允许这种操作，报错,by xwh 20200707
		if(isInProgress())
		{
			return 0x6985;
		}
	}
	
	TransactionStarted = IF_BeginTransaction();

	if(secureMoveStarted == TRUE)
	{
#define moveDateBuf  (tempBuf + 100)
		SetMovDateBufSrcDstDataLen(moveDateBuf,modifyEfAddr + oldfileSize,modifyEfAddr + newFileSize,dataLen);
		
		if(newFileSize  > oldfileSize) //如果是增加EF的空间
		{
			//空间增加，擦除新空间中原有Eeprom数据为FF	
			//空间增加，有初始化字段，初始化增加的空间
			SetUWord(&moveDateBuf[SecureMoveEepromInitDesAddr_Offset],modifyEfAddr);
			SetWord(&moveDateBuf[SecureMoveEepromInitStartOffset_Offset],oldfileSize-sizeof(TranFileHead));
			ecpMem_Copy(&moveDateBuf[SecureMoveEepromInitDataLen_Offset], (BYTE *)&protectDo.FileInfo[0x10],0x30);			
		}

		VMWriteBuf(E_SECUREMOVE_START, moveDateBuf, SecureMoveStuct_LengthAll);	
#undef moveDateBuf
	}

	if(fileStatus == LINTYPE)   //修改EF 的文件头信息
	{
		VMWriteBuf(modifyEfAddr+F_RecNum_Offset, (BYTE  *)&recNum[0], sizeof(BYTE));
	}
	else
	{		
		VMWriteBuf(modifyEfAddr+F_SpaceSize_Offset, (BYTE  *)&parId1, sizeof(WORD));
	}
	
	ModifyFileSize(protectDo,0);

	if(secureMoveStarted == FALSE)	
	{
		if(newFileSize  > oldfileSize) //如果是增加EF的空间
		{
			remainLen = newFileSize - oldfileSize;		
			SetEepromNonAtomic(modifyEfAddr + oldfileSize,0xFF,remainLen);

			if((protectDo.FileInfo[0x10] != 0xFF)&&(protectDo.FileInfo[0x10] != 0x00))
			{
				InitTransOrRecordFile(modifyEfAddr,oldfileSize-sizeof(TranFileHead),&protectDo.FileInfo[0x10],0,NOT_PROFILE_MODE);
			}
		}
	}

#if (EUICC_ONLY||SE_AND_EUICC)
	for(i = 0 ; i < protectDo.ProfileNum; i++)
	{		  
		  VMWriteUword(PROFILE_MFOFFSET_CFG + protectDo.ProfileIndex[i]*sizeof(UWORD),protectDo.ProfileNewMFOffset[i]);
	}
#endif	
	
	IF_CommitTransaction(TransactionStarted);

	if(secureMoveStarted == TRUE)
	{
		StartSecureMoveEeprom(0,0,0);
	}
	
	if(resizeMapFlag == TRUE)
	{
		ReloadDFOrEFAddr(fileName);
	}	

	return 0x9000;
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
 	BYTE idata pos,tmp,tag1,tag2;
	BYTE  * ptrToValue;
	WORD idata fileId, parId1,statusSW1SW2;
	UWORD xdata fileAddr;
	 
	fileAddr = 0x00000000;//lj 20140224 去掉编译警告

 	if(P1P2Check() ==FALSE)
	{
		return ;
	}

	ReceiveData();

	pos = FindTag((BYTE xdata *)&RecData[0], 0x62,  P3);

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

	tmp = FindTag((BYTE xdata *)&ptrToValue[0], 0x83, pos);

	if(tmp == FALSE)
	{
		IncorrentParameter();														 //Send 6A 80
		return;
	}

	fileId = (((WORD)ptrToValue[tmp+1]<<8)&0xFF00) + ptrToValue[tmp+2];

	if(fileId == 0x3F00)		//暂时不考虑MF的空间变更
	{
		ConditionSatisfied();											//删除MF规范有描述返回6985 测试用例 eSIM产品（模板）自动化测试--PE检测--7.PE - RFM-UICC Tar
		return;
	}

	tag1 = FindTag((BYTE xdata *)&ptrToValue[0],  0x80,  pos);					//变更EF空间

	tag2= FindTag((BYTE xdata *)&ptrToValue[0], 0x81, pos);				        //变更DF空间

	if((tag1 == FALSE)&&(tag2 != FALSE))  //变更DF空间
	{
		if(SelectFile(G_CurrentDfAddr[G_ChannelNum], fileId) == FALSE)
		{
			FileNotFound();								//Send 6A 82
			return;
		}
		else
		{
			 //CommandFinish(0x00);
			ConditionSatisfied();

			 return;
		}
	}
	else	                          //变更透明文件，线形文件或BERTLV Reserved Size
	{
	  	parId1 = G_CurrentEfName[G_ChannelNum];
	}

	if(fileId != parId1)
	{
		fileAddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],fileId);

		if(fileAddr == FALSE)
		{
			FileNotFound();								//Send 6A 82

			return;
		}
		else
		{
			if(IF_ProcessNoShareableFile(fileAddr,fileId, 0x02,NULLAPI_Mode) == FALSE)  //该文件是非共享文件,且已被其他通道占用
			{
				ConditionSatisfied();	 				// 69 85

				return;
			}
			SetChannel(fileAddr, fileId, FALSE);
		}
	}

	
	statusSW1SW2 = publicResizeFile(ptrToValue, pos, 0x00,FALSE);

	if(statusSW1SW2 == 0x9000)
	{
		NoShareableFileIsValidAndSetBit(fileAddr,fileId,0x00,NULLAPI_Mode);	//若当前文件为非共享文件，则设置其信息

#if UICC__AUTH__5G__SUPI
		if((ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)&&(G_CurrentDfName[G_ChannelNum] == DF_5GS)&&(G_CurrentEfName[G_ChannelNum] ==EF_ROUTING_INDICATOR)) //远程更新5FC0/4F0A的特殊处理
		{			
			PendingMessageAlert(D_RefreshUpdateRoutingID,0);
		}
#endif
		CommandFinish(0x00);
	}
	else
	{
		GlobalSetStatus(statusSW1SW2);
	}
}

/********************************new function for uiccapi and UICC APDU*********************************************************************/
/*
@description:   目前仅支持同一层级的DF 的映射关系，判断当前要创建的DF和映射的DF是不是同一层级的DF 不是则报错

*/

#if UICC__FUNCTION__DFLink
BYTE DFLinkIsSameLevel(BYTE *linkPath,UWORD currentDfAddr)
{
	BYTE idata cmpLen;
	BYTE xdata currentDFPath[0x08];
	WORD idata parentName[0x02],curDfName;;

	if(UICC__FUNCTION__DFLink__Cfg)
	{
		ecpMem_Set(currentDFPath, 0xFF ,sizeof(currentDFPath));
		
		ecpMem_Set(parentName, 0xFF ,sizeof(parentName));
		
		parentName[1] = GetFileHeadWord(currentDfAddr, F_Name_Offset);

		parentName[0] = GetFileHeadWord(currentDfAddr, F_SecondParentName_Offset);
		
		VMReadBuf((BYTE *)&curDfName,currentDfAddr+F_Name_Offset,sizeof(WORD));
		
		if(curDfName == 0x3F00)
		{
			currentDFPath[0] = 0x3F;
			currentDFPath[1] = 0x00;

			cmpLen = 2;
		}
		else if( parentName[1] == 0x3F00)
		{
			currentDFPath[0] = 0x3F;
			currentDFPath[1] = 0x00;

			currentDFPath[2] = (curDfName >>8 )&0xFF;
			currentDFPath[3] = curDfName &0xFF;


			cmpLen = 4;
		}
		else if( parentName[0] == 0x3F00)
		{
			currentDFPath[0] = 0x3F;
			currentDFPath[1] = 0x00;

			currentDFPath[2] = (parentName[1] >>8 )&0xFF;
			currentDFPath[3] = parentName[1] &0xFF;

			currentDFPath[4] = (curDfName >>8 )&0xFF;
			currentDFPath[5] = curDfName &0xFF;

			cmpLen = 6;
		}
		else
		{
			currentDFPath[0] = 0x3F;
			currentDFPath[1] = 0x00;

			currentDFPath[2] = (parentName[0] >>8 )&0xFF;
			currentDFPath[3] = parentName[0]&0xFF;

			currentDFPath[4] = (parentName[1] >>8 )&0xFF;
			currentDFPath[5] = parentName[1] &0xFF;

			currentDFPath[6] = (curDfName >>8 )&0xFF;
			currentDFPath[7] = curDfName &0xFF;

			cmpLen = 8;
		}
		
		if((linkPath[0] == 0x3F)&&(linkPath[1] == 0x00))
		{

		}
		else
		{
			ecpMem_Move(&linkPath[2], &linkPath[0], 0x06);

			linkPath[0] = 0x3F;
			linkPath[1] = 0x00;
		}
		
		if(ecpMem_Compare(linkPath,currentDFPath, cmpLen) == EQUAL)
		{
			return TRUE;
		}
		else  //不是同一层级的
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}
#endif


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 AddChildDfOrEfNum
  
  input parameter:   WORD dfName, DF file id

					 WORD efName, EF file id    					                                                                                                         

  output parameter:  addr /FALSE
                                                                                                                                                
  description:       在dfName下查找文件efName,如果找到,返回文件的地址

  Coder:		xiaoyh			Tester:   
  
  status:			     

 ------------------------------------------------------------------- ------------------------------------------------*/    
void AddChildDfOrEfNum(UWORD dfAddr,BYTE offset,BYTE mode)
{
	BYTE  idata num;
	
	num = VMReadByte(dfAddr+offset);//读取父DF下的df/ef数目
		
	num ++;
		
	VMWriteBuf(dfAddr+offset,&num ,sizeof(BYTE)); //更新父DF下的df/ef数目
}

#if (EUICC_ONLY||SE_AND_EUICC)
/*
*@funtion    :IsExistGlobalPin
*
*@description: All the Global PINs referenced by a pinStatusTemplateDO shall be defined in the "PIN Context" of the MF.
*
*@param      :BYTE pinRef 
*   
*@return     :TRUE/FALSE
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20190225
*            :V1.1 :modify by chentao@EPX产业化\INFENION_SLM97\Develop
*/  
BYTE IsExistGlobalPin(BYTE pinRef)
{
	UWORD xdata fileAddr;
	BYTE idata recNum,i;
	fileAddr = SelectFile(G_CreatingMFOffset,DF_PROFILECFG);
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	fileAddr = SelectFile(fileAddr,EF_PRIVATE_PIN);//选择MF下的pin文件，Global PINs referenced by a pinStatusTemplateDO 是否存在于该文件中
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	recNum = GetLFRecNumLgth(fileAddr,FileNum);
	
	if(((pinRef >= 1) && (pinRef <= 8)) || ((pinRef >= 0x0A) && (pinRef <= 0x0E))|| ((pinRef >= 0x8A) && (pinRef <= 0x8E)))
	{
		for(i = 1; i <= recNum;i++)
		{
		  ReadLFBuf(fileAddr,i,0,&tempBuf[0],1);//读取该条记录的pinref
		  if(pinRef == tempBuf[0])
		  {
			  return TRUE;
		  }
		}
		return FALSE;

	}
	else
	{
	   return TRUE;
	}
}
#endif
/**
*modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                             用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*modify by chentao@20210722 EPX产业化/INFENION_SLM97
*modify by xuweihua@20210929 创建文件制定权限可以使用8B/AB/8C三种格式之一，8B的长度必须是03
*modify by zhxx@20220419 ，HB_Offline_Profile_Test/Profile_FulSeg0046,when create df,should not exist 0xa5 tag
*/ 
BYTE code DFNeedSupportTag[] ={0xC6,0x82,0x83};
#if SE_ONLY
BYTE code DFNotSupportTag[] ={0x88,0x80};//102222 ,create df has a5
#else
BYTE code DFNotSupportTag[] ={0x88,0x80,0xA5};//HB_Offline_Profile_Test/Profile_FulSeg0046,when create df,should not exist 0xa5 tag
#endif
UWORD commonCreateDFFile(UWORD currentDfAddr,WORD fileName,BYTE *buf,BYTE len,BYTE mode)
{
	FileSystemHead xdata dfFileHead;
	BYTE  idata result,i,j;
	UWORD xdata parentfileaddr;
	NewResizeAntiTearing xdata protectDo;
	UWORD xdata dataLen;
	UWORD xdata currentDfUsedSize,mfUsedsize;

#if UICC__FUNCTION__2B80_AT_FILEEND		//remove warning,dengyiwen,2022/9/27
	UWORD xdata toolkitAddr;
	UWORD xdata bb00DfUsedSize;
#endif
	
#if (EUICC_ONLY||SE_AND_EUICC)
	UWORD xdata currentMFOffset,currentMFUsedSize;
	BYTE xdata temp[PROFILE_MAX_NUMBER];
#if UICC__FUNCTION__DFLink
	UWORD xdata fileAddr;
#endif
#endif

	BYTE idata mappingDfTag;
	BYTE idata TransactionStarted,secureMoveStarted;	
	
	mappingDfTag = FALSE;
	TransactionStarted = FALSE;
	secureMoveStarted = FALSE;

	//add by chentao 补上合PE解析优化代码没合全的代码
	for(i=0x00;i<sizeof(DFNotSupportTag);i++)  //出现错误的tag
	{
		result = FindTag(&buf[0],DFNotSupportTag[i],len);
		if((ADF_SUPPORT_SFI) && (DFNotSupportTag[i] == 0x88))
		{
			continue;
		}
		if(result != FALSE)
		{
			return E_InvalidRequest;  //mofify by chentao @20211125
		}
	}

	for(i=0x00;i<sizeof(DFNeedSupportTag);i++) //缺少必要的tag
	{
		result = FindTag(&buf[0],DFNeedSupportTag[i],len);
		if(result == FALSE)
		{
			if(DFNeedSupportTag[i] == 0xC6)
			{
				result = FindTag(&buf[0],0x8F,len);  //不是映射文件且没有0xc6
				if(result == FALSE)
				{
					return E_InvalidRequest;
				}
			}
			else
			{
				return E_InvalidRequest;
			}
		}
	}

	for(i=0x00;i<sizeof(DFNeedSupportTag);i++)
	{
		result = FindTagOffset(&buf[0],DFNeedSupportTag[i],len,2);	//必要的tag出现多次
		if(result != FALSE)
		{
			return E_Parameter;
		}
	}
#if (EUICC_ONLY||SE_AND_EUICC)
	if((mode & PROFILE_MODE) == PROFILE_MODE)//profile创建模式
	{		
		mfUsedsize = GetFileHeadUsedSize(G_CreatingMFOffset);
		
		if((G_CreatingMFOffset + mfUsedsize + sizeof(FileSystemHead)) > GetHeapBase())
		{
			return E_FileSize;
		}
	}
	else
#endif
	{
		mfUsedsize = GetMFUsedSize();
		
		if(AllocSpaceEnough(sizeof(FileSystemHead)) == FALSE)
		{
			return E_FileSize;
		}
	}
		
	
//	if((mode == PROFILE_MODE) && (FindTag(&buf[0],0x80,len) != FALSE))//efFileSize Mandatory for EF file types Not allowed for DF files and EF link files 
//	{
//		return E_Parameter;
//	}

	// 中国电信5g 测试，rfm/func/cmd/uicc/T_CreateFile_Error_PORSubmit 创建DF 时，有88 tag 应返回6A80
	// modify by chentao@20211125前面已经判断了
	
		if(!ADF_SUPPORT_SFI)
		{
			result = FindTag(&buf[0],0x88,len);									//short file id

			if(result != FALSE)
			{
				return E_Parameter;
			}
		}
 	
	//dfFileHead.FillByte = 0x00;
	ecpMem_Set((BYTE*)&dfFileHead, 0x00, sizeof(dfFileHead));//初始化FillByte，避免出现CRC不一致的情况,by chentao@20240415  //bug531
	
	dfFileHead.Rfu = 0xFF;
	
	dfFileHead.KeyLmit = 0x0A;
	
	ecpMem_Set((BYTE *)&dfFileHead.MF_Key[0], 0xFF, 0x08);
	
	result = FindTag(&buf[0],0x8F,len);                             //DF Link

    if (result != FALSE)
    {
  #if UICC__FUNCTION__DFLink
    	if(UICC__FUNCTION__DFLink__Cfg)
    	{
			if(buf[result] > 0x08)
			{
				return E_Parameter;
			}
		
			if(buf[result] == 0x08)
			{
				if((buf[result+1] == 0x3F)&&(buf[result+2] == 0x00))
				{
			
				}
				else
				{
					return E_Parameter;
				}
				i = 3;
			}
			else
			{
				i = 1;
			}
#if (EUICC_ONLY||SE_AND_EUICC)
			if((mode & PROFILE_MODE) == PROFILE_MODE)  //modify by chentao@20211012
			{
				fileAddr = G_CreatingMFOffset;

				for(; i<buf[result];i+=2)
				{
					fileAddr = SelectFile(fileAddr, BuildWord(&buf[result+i]));

					if(fileAddr == FALSE)
					{
						return E_Parameter;
					}
				}
				VMReadBuf((BYTE *)&dfFileHead.FileId,fileAddr,sizeof(FileSystemHead));
			}
#endif
			ecpMem_Copy((BYTE *)&dfFileHead.MF_Key[0], &buf[result+1], buf[result]);
	
			if(DFLinkIsSameLevel(&dfFileHead.MF_Key[0], currentDfAddr) == FALSE)
			{
				return E_Parameter;
			}
			
			ecpMem_Copy((BYTE *)&dfFileHead.MF_Key[0], &buf[result+1], buf[result]);
			
			mappingDfTag = TRUE;
    	}
    	else
	#endif
    	{
		    if(ValueBit(G_PEControlContext.SUPPORTED_ServicesList[2],dfLink_SUPPORTED) == FALSE)  //ServicesList 不支持 DF link
			{
	    		if(ValueBit(G_PEControlContext.IsExistFlag2,Mandated))  //如果是mandatory PE 则报错
				{
					return E_NotSupport;
				}
				else                                                   //否则报警告
				{
					return E_FileNotSupportWaring;
				}
			}
			//return E_NotSupport;
    	}

    }

	dfFileHead.FileId = fileName;

	result = FindTag(&buf[0],0x82,len);

	dfFileHead.FileType = buf[result+1]&0xF8;

	dfFileHead.MaxSize = sizeof(FileSystemHead);
	dfFileHead.UsedSize = sizeof(FileSystemHead);

	dfFileHead.ChildDfNum = 0x00;

	dfFileHead.ChildEfNum = 0x00;

	dfFileHead.LocalPinIndex = 0;

	dfFileHead.ParentName[1] = GetFileHeadWord(currentDfAddr, F_Name_Offset);

	dfFileHead.ParentName[0] = GetFileHeadWord(currentDfAddr, F_SecondParentName_Offset);

#if (EUICC_ONLY||SE_AND_EUICC)
	if(mode != NOT_PROFILE_MODE)
	{
		dfFileHead.ArrName = 0x2F;

	    dfFileHead.ArrNum = 0x01;
	}
	else
#endif
	{
		if((ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)&&(FindTag(&buf[0],0x84,len)!=FALSE))
		{
			return 	E_Parameter;                                                                        //AID mode 远程文件管理中，卡片不允许create file创建ADF
		}
	}

	//8B/AB/8C模式三选一
	//8B长度必须是3
	result = FindTag(&buf[0],0x8B,len);									//Referenced mode

	if(result != FALSE)
	{
		if(buf[result] != 0x03)
		{
			return E_Parameter;
		}
#if UICC_AB8C_ARR_PRIVATE
		if((buf[result+3] > STANDARD_ARR_MAX_NUMBER)&&(buf[result+3] != ARR_RECNUM_ADM)&&(buf[result+3] != ARR_RECNUM_NEV))//arrNum的最高2个bit位为1时,指示AB/8C访问规则，因此8B规则创建文件时的记录条数不能大于0xBF
		{
			return E_Parameter;
		}
#endif			
		dfFileHead.ArrNum = buf[result+3];
	    dfFileHead.ArrName = buf[result+1];
	}
	//else if((ValueBit(G_SystemStatus,B_RfmPerform) == TRUE) && (mode != PROFILE_MODE))
	//电信5G测试要求本地创建文件支持AB模式,by xwh 20210508
	else
	{
		result = FindArrNum(&buf[0],len);	 	//Compact mode for RFM	 	//该函数内部会使用到tempBuf	,因此要注意到buf不能使用tempBuf，会冲突
	
		if(result == FALSE)
		{
			result = FindExtandArr(&buf[0],len);		//AB格式
				
			if (result == FALSE)
			{
				return E_Parameter;
			}
		}

		dfFileHead.ArrNum = result;

		dfFileHead.ArrName = (BYTE)((EF_ARR>>8)&0xFF);
	}
	
	dfFileHead.ArrName = ((dfFileHead.ArrName == 0x2F)?F_ARRNAME_2F:F_ARRNAME_6F);
	
	result = FindTag(&buf[0],0xC6,len);

	if(result != FALSE)
	{
			for(j=0x01;j<buf[result] ;j = j+3) // modify by chentao@20240530 83 tag 只在C6范围内找，否则在83不存在的情况会死循环   例： 00E000003A 6238 82027821 83027F60 840FA0000000871002FF47F001890000108A0105AB0F80015F90008001209700840188970081020000 C6059000950108
			{
				if(buf[result+j] == 0x83) //找到第一个83退出
				{
					break;
				}

			}

			for(i=0x00;i<0x04;i++)
			{
				if((j+(i*3))< buf[result])
				{
					dfFileHead.ChvStatus[i] = buf[result+j+(i*3)+2];
#if (EUICC_ONLY||SE_AND_EUICC)
            // modify by zhangxiaoxiao@20221204:			
			//1、创建3F00时，文件头的LocalPinIndex赋初值1
            //2、ReadPinStatus时，遍历localpin，需要遍历文件系统的整个DF，判断LocalPinIndex是否为0，localpin存在于DF/ADF下面
					if(((dfFileHead.ChvStatus[i] >= 0x81) && (dfFileHead.ChvStatus[i] <= 0x88)) && 
						(((mode != NOT_PROFILE_MODE)&&(currentDfAddr == G_CreatingMFOffset)) ||((mode==NOT_PROFILE_MODE)&&(currentDfAddr == E_FILE_START))))//第一层级的DF才需要有LocalPinIndex
					{
						  dfFileHead.LocalPinIndex = GetMaxLocalPinIndex();
						
							if(dfFileHead.LocalPinIndex > sizeof(G_LocalPinStatus))
							{
								return  E_FileSize;
							}
					}

					if((mode != NOT_PROFILE_MODE) && (G_profileControlContext.ProfileVersion >= 0x22) && (fileName != DF_PROFILECFG) && (fileName != DF_Toolkit_P))//profile创建模式,profile2.2规范才做此检查
					{
						//All the Global PINs referenced by a pinStatusTemplateDO shall be defined in the "PIN Context" of the MF.
					   if(IsExistGlobalPin(dfFileHead.ChvStatus[i]) == FALSE)
					   {
					   //return E_PINMissing;
					   dfFileHead.ChvStatus[i] = 0xFF;
					   }
					}
#endif
				}

				else
				{
					 dfFileHead.ChvStatus[i] = 0xFF;
				}
			}

	}
	else// pinStatusTemplateDO Not allowed for EF files Mandatory for DF/ADF files
	{
		if(mappingDfTag == FALSE)//没有c6 tag，且不是映射DF，则抛错TC_8_2_3_8_Creating_a_DF_with_dfLink_when_eUICC_supports_dfLink
		{
			//return 	E_Parameter;
			dfFileHead.LocalPinIndex = 0;
			dfFileHead.ChvStatus[0] = 0x01;
			dfFileHead.ChvStatus[1] = 0xFF;
			dfFileHead.ChvStatus[2] = 0xFF;
			dfFileHead.ChvStatus[3] = 0xFF;
		}		
	}
	
	
	result = FindTag(&buf[0],0x84,len);								//AID mode

	if(result != FALSE)
	{
#if (EUICC_ONLY||SE_AND_EUICC)
		if(mode != NOT_PROFILE_MODE)
		{			
				dfFileHead.DIRNum = GetAIDNumber(&buf[result],TRUE);
		}
		else
#endif
		{
			parentfileaddr = SelectFileByName(currentDfAddr,(BYTE *)&buf[result+1],buf[result]);

			if(parentfileaddr != FALSE)
			{
				return E_DfNameExist;
			}

			dfFileHead.DIRNum = GetAIDNumber(&buf[result],FALSE);
		}

        dfFileHead.Rfu =  PN_R_U8(MEMtPN8(JcvmConfig_GetEnablePriAuthBase()));	//设置ADF的算法配置字节为默认值	
	}
	else
	{
	    dfFileHead.DIRNum = 0x00;
	}

	ecpMem_Set((BYTE *)&protectDo, 0x00, sizeof(NewResizeAntiTearing));
	
	i = PreModifyFileSize(&protectDo,currentDfAddr,sizeof(FileSystemHead),mode,0);
	
	if(i >= (DF_Depth-1) )  //COS 最大支持3级DF，由数组protectDo.ParentAddr[0]决定,当  i = 3时，表示当前DF上已经有4级父 DF了，当前DF一旦创建成功 就是第4 级 df,所以此处需DF失败
	{
		return E_Parameter;
	}	
	
	currentDfUsedSize = GetFileHeadUsedSize(currentDfAddr);
#if UICC__FUNCTION__2B80_AT_FILEEND
	//在MF 下创建文件，如果DF_Toolkit_P_Ex/EF_ToolkitApp已经创建，那么要把他们往后移动，保证EF_ToolkitApp是最后一个文件, by xwh 20210710
	if(currentDfAddr == E_FILE_START)
	{		
		toolkitAddr = SearchFile(E_FILE_START,DF_Toolkit_P_Ex);

		if(toolkitAddr != FALSE)
		{
			bb00DfUsedSize = GetFileHeadUsedSize(toolkitAddr);
			
			toolkitAddr = SearchFile(toolkitAddr,EF_ToolkitApp);

			if(toolkitAddr != FALSE)
			{
				currentDfUsedSize -= bb00DfUsedSize;
			}
		}
	}
#endif	

#if (EUICC_ONLY||SE_AND_EUICC)
	if(mode != NOT_PROFILE_MODE)//profile创建模式
	{
		dataLen = G_CreatingMFOffset + mfUsedsize - (currentDfAddr+ currentDfUsedSize);
	}
	else
#endif
	{
		dataLen = E_FILE_START + mfUsedsize - (currentDfAddr+ currentDfUsedSize);
		
#if (EUICC_ONLY||SE_AND_EUICC)

		protectDo.ProfileNum = 0;

		VMReadBuf(temp,PROFILE_VALIDTAG_CFG,PROFILE_MAX_NUMBER);    //获取当前所有profile的状态
		
		for(i= 0; i< PROFILE_MAX_NUMBER ; i++)
		{		  
			if(temp[i] != PROFILE_INVALIDFLAG)
			{
				currentMFOffset = VMReadUword(PROFILE_MFOFFSET_CFG + i*sizeof(UWORD)); 
				
				if((currentMFOffset > E_FILE_START)&&(currentMFOffset != 0xFFFFFFFF))
				{
					protectDo.ProfileIndex[protectDo.ProfileNum] = i;
					
					protectDo.ProfileNewMFOffset[protectDo.ProfileNum] = currentMFOffset+sizeof(FileSystemHead);
										
					currentMFUsedSize = GetFileHeadUsedSize(currentMFOffset);
					
					dataLen += currentMFUsedSize;
					
					protectDo.ProfileNum++;   //找到较大的那个MF 的偏移
				}
			}
		}
#endif
	}
	
	
	if(dataLen != 0) //当前待创建的文件位于现有文件系统的末尾，此时不需要移动数据，只要在空间足够的情况下，创建文件即可
	{
		secureMoveStarted = TRUE;
		
		//如果事务已开启，又要移动数据，不允许这种操作，报错,by xwh 20200707
		if(isInProgress())
		{
			return E_ConditionNotSat;
		}
	}

	ecpMem_Copy(&protectDo.FileInfo[0], (BYTE  *)&dfFileHead.FileId,sizeof(FileSystemHead));
	
	TransactionStarted = IF_BeginTransaction();

	//if(mode == NOT_PROFILE_MODE)
	//{
		//WriteEepromEX(E_STRUCTURE_START, (BYTE xdata *)&protectDo, sizeof(NewResizeAntiTearing));		//写事务性断电保护
	//}
	
	//if((protectDo.Tag&0x01) == 0x00)
	if(secureMoveStarted == TRUE)
	{
		//result = SecureMoveEeprom( currentDfAddr+ currentDfUsedSize + sizeof(FileSystemHead), currentDfAddr + currentDfUsedSize,dataLen);		
		
		//if(result == FALSE)
		//{
		//	return FALSE;
		//}
#define moveDateBuf  (tempBuf + 100)
		SetMovDateBufSrcDstDataLen(moveDateBuf,currentDfAddr + currentDfUsedSize,currentDfAddr+ currentDfUsedSize + sizeof(FileSystemHead),dataLen);

		SetUWord(&moveDateBuf[SecureMoveEepromWriteDesAddr_Offset],currentDfAddr+currentDfUsedSize);
		moveDateBuf[SecureMoveEepromWriteDataLen_Offset] = sizeof(FileSystemHead);		
		ecpMem_Copy(&moveDateBuf[SecureMoveEepromWriteDataBuf_Offset], (BYTE  *)&dfFileHead.FileId,sizeof(FileSystemHead));

		VMWriteBuf(E_SECUREMOVE_START, moveDateBuf, SecureMoveStuct_LengthAll);	
#undef moveDateBuf	
	}

	if(secureMoveStarted == FALSE)
	{
		result = VMWriteBuf(currentDfAddr+currentDfUsedSize, (BYTE *)&dfFileHead.FileId,sizeof(FileSystemHead)); 
		
		if(result == FALSE)
		{
			return FALSE;
		}
	}

	AddChildDfOrEfNum(currentDfAddr,F_ChildDFNum_Offset,mode);  //更新父DF下的DF数目
	
	ModifyFileSize(protectDo,0);
	
#if (EUICC_ONLY||SE_AND_EUICC)	
	for(i = 0 ; i < protectDo.ProfileNum; i++)
	{		  
		  VMWriteUword(PROFILE_MFOFFSET_CFG + protectDo.ProfileIndex[i]*sizeof(UWORD),protectDo.ProfileNewMFOffset[i]);
	}
#endif	


	IF_CommitTransaction(TransactionStarted);

	if(secureMoveStarted == TRUE)
	{
		StartSecureMoveEeprom(0,0,0);
	}
	
	//return dfFileHead.UsedSize;
	return (currentDfAddr+currentDfUsedSize);
	
}

/**
*V1.0 create file
*V1.1:modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                              用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*V1.2:modify by chentao@20210721 EPX产业化/INFENION_SLM97
*V1.3:modify by xuweihua@20210929 创建文件制定权限可以使用8B/AB/8C三种格式之一，8B的长度必须是03
*V1.4:modify by xuweihua@20211117 支持创建14 个强更新文件  
*V1.5:modify by xuweihua@20211229 不允许创建文件映射到Ki/OPC/PIN/PUK文件，支持创建文件时指定文件不允许读和映射  
:V1.6 创建文件时，检查短文件标识是否已经存在, 缺陷382，联通备案脚本023_UICC_CreateFile有测到，modify by zhxx 20220510
*/
BYTE code EFNotSupportTag[] ={0xC6,0x84};
BYTE code EFNeedSupportTag[] ={0x80,0x82,0x83};

UWORD commonCreateEFFile(UWORD currentDfAddr,WORD fileName,BYTE *buf,BYTE len,BYTE mode)
{
	BYTE idata result,i,tag8F, result1;
	BYTE idata fileType,shortFileId,arrNum,arrName,fileStatus;
	UWORD  idata fileSize;
	UWORD idata fileNeedSize,modifyLen;
	WORD xdata mapFile[3];
	TranFileHead xdata transEfFileHead;
	RecordFileHead xdata recordEfFileHead;
#if UICC__FUNCTION__BERTLVCmd
	BERTLVFileHead xdata  bertlvFileHead;
	UWORD  idata bertlvfileSize;
#endif
	UWORD idata mapFileAddr;

	NewResizeAntiTearing	xdata protectDo;
	UWORD	xdata dataLen,currentDfUsedSize,mfUsedsize;
 
#if (EUICC_ONLY||SE_AND_EUICC)
	UWORD	xdata  currentMFOffset,currentMFUsedSize,currentDfMaxsize;
    BYTE xdata temp[PROFILE_MAX_NUMBER];
#endif

	BYTE idata TransactionStarted,secureMoveStarted;
    BYTE idata heRecordNo; 
#if UICC__FUNCTION__2B80_AT_FILEEND		//remove warning,dengyiwen,2022/9/27
	UWORD idata toolkitAddr;
	UWORD idata bb00DfUsedSize;
#endif
#if   High_Update_InRAM_New
	BYTE xdata   update_ram_new,new_allot_size,update_new_buf[2];
#endif
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	BYTE idata num,heFlashTag,pos;			
	WORD  idata calcCrc;
	BYTE  idata fileNo;		

	heFlashTag = FALSE;
	//num = 0xFF;
	fileNo = 0x00;	
#endif
#if   High_Update_InRAM_New
	update_ram_new = 0x00; //强更新index  或上0x80
	
	new_allot_size = 0x00;//已经分配的强更新ram size.
	
	ecpMem_Set(update_new_buf,0x00,sizeof(update_new_buf));
#endif
 	heRecordNo = 0x00;
	fileSize = 0;
	TransactionStarted = FALSE;
	secureMoveStarted = FALSE;
	arrNum = 0xFF;
	modifyLen = 0;
	
	for(i=0x00;i<sizeof(EFNotSupportTag);i++)  //出现错误的tag
	{
		result = FindTag(&buf[0],EFNotSupportTag[i],len);
		if(result != FALSE)
		{
			return E_InvalidRequest;
		}
	}

	for(i=0x00;i<sizeof(EFNeedSupportTag);i++) //缺少必要的tag
	{
		result = FindTag(&buf[0],EFNeedSupportTag[i],len);
		if(EFNeedSupportTag[i] == 0x80)
		{
			if(result == FALSE)// 没有80
			{
				result = FindTag(&buf[0],0x8F,len);
				if(result == FALSE)//不是映射文件
				{
					return E_InvalidRequest; // modify by chentao@20211119  中国电信离线脚本测试--pofile结构测试库V2.3.1--data\03-PE-EAP--data_03_PE_EAP_04  缺少80 tag 期望返回INVALID_REQUEST_FORMAT
				}
			}
			else // 有80
			{
				result = FindTag(&buf[0],0x8F,len);
				if(result != FALSE) //是映射文件
				{
					if(mode == PROFILE_MODE_TEMPLATE)
					{
						//guotong9_23lod_Fail_Test01
						//return FALSE;
					}

				}
			}

		}
		else if(EFNeedSupportTag[i] == 0x82)
		{
			if(result == FALSE)// 没有82
			{
				result = FindTag(&buf[0],0x8F,len);
				if(result != FALSE)
				{
					if(mode == PROFILE_MODE_GENERIC )
					{
						return E_InvalidRequest;
					}
				}
			}
			else // 有82
			{
				result = FindTag(&buf[0],0x8F,len);

				if(result != FALSE)
				{
					if(mode == PROFILE_MODE_TEMPLATE)
					{
						//return FALSE;
					}
				}

			}
		}
		else
		{
			if(result == FALSE)
			{
				return E_InvalidRequest;
			}
		}
	}
	
	for(i=0x00;i<sizeof(EFNeedSupportTag);i++)
	{
		result = FindTagOffset(&buf[0],EFNeedSupportTag[i],len,2);	//必要的tag出现多次
		if(result != FALSE)
		{
			return E_Parameter;
		}
	}
#if (EUICC_ONLY||SE_AND_EUICC)
	if(mode == PROFILE_MODE_TEMPLATE)
	{
		if( contentRequiredCheck() == FALSE)
		{
			return FALSE;
		}
	}
#endif

//	if(mode == PROFILE_MODE)
//	{
//		result = FindTag(&buf[0],0xC6,len);
//	
//		if(result != FALSE)// pinStatusTemplateDO Not allowed for EF files Mandatory for DF/ADF files
//		{
//			return E_Parameter;
//		}	
//	}
		
	result = FindTagOffset(&buf[0],0x02,len,1);	

	if(result != FALSE)
	{
		return E_Parameter;
	}

	result = FindTag(&buf[0],0x82,len);							//File Type

	 if((buf[result] != 0x02) && (buf[result] != 0x04))
	 {
		 return E_Parameter;
	 }

	fileType = buf[result+1];
	
	if( buf[result+2] != 0x21) 
	{
		return E_Parameter;
	}
	
#if (EUICC_ONLY||SE_AND_EUICC)
// add by xieyong@20211105 在profile header中不到BERTLV 服务，实际有文件是BERTLV文件的情况下，profile能下载成功但是需要拼装警告
	//modify by chentao @20220318 profile header no bertlv & card  not support bertlv	
	if((fileType&0x3F) == 0x39)  //BER-TLV
	{
		if(ValueBit(G_PEControlContext.SUPPORTED_ServicesList[2],bertlv_SUPPORTED) == FALSE)  //ServicesList 不支持 BER-TLV
		{
			if(!UICC__FUNCTION__BERTLV)
			{
				if(ValueBit(G_PEControlContext.IsExistFlag2,Mandated))  //If "mandated" is set in the corresponding PE header, the installation of the Profile shall be aborted by the eUICC
				{
					return E_NotSupport;
				}
				else  // the eUICC shall send a status code set to "feature-not-supported" without any "additional-information" and the installation shall continue without creating the BER-TLV file.   
				{
					return E_FileNotSupportWaring;
				}
			}
		}
	}
#else
	if((fileType&0x3F) == BERTLVTYPE)  //BER-TLV
	{
		if(!UICC__FUNCTION__BERTLV)
		{
			return E_NotSupport;
		}
	}
#endif

	result = FindTag(&buf[0],0x8A,len);                          //fileStatus
    if(result == FALSE)
	{
		fileStatus = 0x05;
	}
	else
	{
		if( buf[result] != 1)
		{
			return E_Parameter;
		}
		fileStatus = buf[result+1]&0x0F;
		if(fileStatus == 0x02)
		{
			return E_Parameter;
		}
	}

	//'A5' or '85', Tag proprietary, application depndent
	result = FindTag(&buf[0],0xA5,len);
	
	if(result == FALSE)
	{
		result = FindTag(&buf[0],0x85,len);
	}

	if(result != FALSE)
	{		
		result1 = FindTag(&buf[result+1],0xc0,buf[result]);							//Expand for read and updata enable when invalid

		if(result1 != FALSE)
		{
			if((buf[result+1+result1+1]&0x40) == 0x40)
			{
				fileStatus |= F_READ_UPDATA_ENBALE;
			}
		}
	}

	//8B/AB/8C模式三选一
	//8B长度必须是3
	result = FindTag(&buf[0],0x8B,len);									//Referenced mode
	
	if((result != FALSE)&&(buf[result]==0x03))
	{
#if UICC_AB8C_ARR_PRIVATE
		if((buf[result+3] > STANDARD_ARR_MAX_NUMBER)&&(buf[result+3] != ARR_RECNUM_ADM)&&(buf[result+3] != ARR_RECNUM_NEV))//arrNum的最高2个bit位为1时,指示AB/8C访问规则，因此8B规则创建文件时的记录条数不能大于0xBF
		{
			return E_Parameter;
		}
#endif			
		arrNum = buf[result+3];
		arrName = buf[result+1];
	}
	//else if((ValueBit(G_SystemStatus,B_RfmPerform) == TRUE) && (mode == NOT_PROFILE_MODE))                 //arrNum  arrName
	//电信5G测试要求本地创建文件支持AB模式,by xwh 20210508
	else
	{
		arrNum = FindArrNum(&buf[0],len);	//该函数内部会使用到tempBuf	,因此要注意到buf不能使用tempBuf，会冲突
	
		if(arrNum == FALSE)
		{
			arrNum = FindExtandArr(&buf[0],len);		//AB格式
				
			if (arrNum == FALSE)
			{
				return E_Parameter;
			}	
		}
		
		arrName = (BYTE)((EF_ARR>>8)&0xFF);
	}
	
	arrName = ((arrName == 0x2F)?F_ARRNAME_2F:F_ARRNAME_6F);

	result = FindTag(&buf[0],Tag_fShortID,len);									//short file 	id

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
		if(buf[result] > 1)   //shortEFID [8] OCTET STRING (SIZE (0..1)) OPTIONAL,
		{
			return FALSE;
		}

		if(buf[result] != 0x00)
		{
			shortFileId= (buf[result+1]>> 3)&0x1F;
			if(shortFileId > 0x1E) //(SFI) is coded as 5 bits valued in the range from 1 to 30.
			{
				if((mode == PROFILE_MODE) && (MOBILE_Operator != OPERATOR_CHINATELECOM))	//add by chentao@20231020 为了提高COS兼容性，在profile下载过程中如果SFI设置不合法，则不设置SFI
				{
					shortFileId = 0x00;
				}
				else
				{
					return FALSE;
				}
			}
		}
		else
		{
			shortFileId = 0x00;									//Not support SFI
		}
	}
	
  	if(mode == NOT_PROFILE_MODE)//标准的创建文件，才需要检查88tag是否重复,eSIM_Test_additional/During_Load_Profile_break_in_update_file_Test_01
  	{
		if(shortFileId != 0)//when create ef, is exist the same shortFileId
		{
			if(SelectFileByName(G_CurrentDfAddr[G_ChannelNum],&shortFileId,0x00) != FALSE)
			{
				return E_Parameter;
			}
		}
  	}

  	result = FindTag(&buf[0],0x80,len);					//File Size

	tag8F = FindTag(&buf[0],0x8F,len);	   //Expand for FileMapping
	
  	if(result != FALSE)
  	{
		if(buf[result] == 0x01)				  //UICCAPI Api_2_Tkr_RgfeSo 测到 800103 ，需要支持01长度的fileSize
		{
			fileSize = buf[result+1];
		}
		else if(buf[result] == 0x02)
		{
			fileSize = BuildWord(&buf[result+1]);
		}
		else
		{
			return E_Parameter;
		}
	}
	if((ValueBit(G_SystemStatus,B_RfmPerform) == TRUE) && (mode == NOT_PROFILE_MODE))
	{
		fileStatus = (fileStatus &0xF0) | 0x05;//远程创建文件，文件状态默认为05
	}
	if((fileType&0x37) == TRANTYPE) //默认不是强更新文件
	{
		transEfFileHead.UpdateConfignum = 0x00; 	
	}
	else
	{
		recordEfFileHead.UpdateConfignum = 0x00;  
	}	
#if	High_Update_InRAM_New
	update_ram_new = FindTag(&buf[0],0x91,len);				//tag-910180
	if((update_ram_new != FALSE)&&(buf[update_ram_new] != 0x00)&&(buf[update_ram_new+1] == 0x80))
	{
		if((fileType&0x37) != TRANTYPE) //目前只支持透明文件
		{
			return E_Parameter;
		}
		
		ReadEeprom(&update_ram_new,Config_High_Update_InRAM_num,0x01);//强更新ram 个数
		
		update_ram_new++;
		
		update_ram_new |= Hige_UpdateWithRAM_New;//index ++
		
		ReadEeprom(&new_allot_size,Config_High_Update_InRAM_Size,0x01);//强更新ram 个数
		
		if((new_allot_size + 2 + fileSize) > sizeof(G_High_Update_InRAM_new)) //缓存在 G_High_Update_InRAM_new是以 TLV格式 T(update_ram_new)L(fileSize)V(文件体)
		{
			return E_FileSize;
		}
		
		new_allot_size += (2 + fileSize);//已经使用的大小
		
		update_new_buf[0] = update_ram_new;
		update_new_buf[1] = new_allot_size;
		
	}
	else
	{
		update_ram_new = 0x00; //防止强更新RAM和High_Update_In_HEFlash  91 tag冲突赋值
	}
	
	if(update_ram_new != FALSE)
	{
		transEfFileHead.UpdateConfignum = update_ram_new;
	}
#endif
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH	
	pos = FindTag(&buf[0],0x91,len);				//tag-91定义是否存在备份文件

	if((pos != FALSE)&&(buf[pos] != 0x00)&&(buf[pos+1] != 0x80))									   //该tag存在时,文件头fileType字节的最高位置为1,表示存在备份文件(该Tag设置为9100)
	{
		//强更新文件不能是映射文件，在文件映射的情况下要把实体文件创建成强更新文件		
		if(tag8F != FALSE)
		{
			return E_Parameter;
		}
	
		ReadEeprom(&fileNo,ADDR_FILENO_HEFLASH,0x01);
		
		fileNo++;
		
		//最多允许创建14个强更新文件
		if((fileNo == 0x00)||(fileNo > MaxNum_HeFlash_Files))
		{
			return E_Parameter;
		}

		heRecordNo = GetHeFlashUsableRecordNo(fileNo);

		if((heRecordNo == FALSE)||(heRecordNo> Num_HeFlash_Pages))
		{
			return E_Parameter;
		}

		if((fileType&0x37) == TRANTYPE)
		{
			transEfFileHead.UpdateConfignum = fileNo;
		}
		else
		{
			recordEfFileHead.UpdateConfignum = fileNo;
		}	

		heFlashTag = TRUE;  //表示是强更新文件，文件体放在HE_FLASH区
	}
#endif
	
	result = FindTag(&buf[0],0x93,len);							//Expand for File Map and File Read

	if(result != FALSE)
	{
		if(buf[result] != 0x01)
		{
			return E_Parameter;
		}
		if(buf[result+1] == 0x01)
		{
			shortFileId |= File_MapAndRead;
		}
		else
		{
			shortFileId &= 0x7F;
		}
	}
	
#if UICC__FUNCTION__BERTLVCmd
    bertlvfileSize=fileSize;
#endif

	result = FindTag(&buf[0],0x8F,len);
	if(result != FALSE)
	{
//		if((mode == PROFILE_MODE) && (tag80 != FALSE))// efFileSize Mandatory for EF file types Not allowed for DF files and EF link files
//		{
//			return E_Parameter;
//		}
		
		fileSize = 0x0000;		//20141217 ligy
		
		if(buf[result]==0x08)
		{
			mapFile[0] = BuildWord(&buf[result+3]);

			mapFile[1] = BuildWord(&buf[result+5]);

			mapFile[2] = BuildWord(&buf[result+7]);
		}
		else if(buf[result]==0x06)
		{
			mapFile[0] = BuildWord(&buf[result+1]);

			mapFile[1] = BuildWord(&buf[result+3]);

			mapFile[2] = BuildWord(&buf[result+5]);
		}
		else if(buf[result] == 0x04)
		{  
			mapFile[0] = BuildWord(&buf[result+1]);

			mapFile[1] = mapFile[0];

			mapFile[2] = BuildWord(&buf[result+3]);
#if (EUICC_ONLY||SE_AND_EUICC)
			/*if((mode == PROFILE_MODE) && (ValueBit(G_IsExistFlag1,Attr8BTag)==FALSE))
			{
				arrNum = getMapFileArrNum(mapFile[0],mapFile[1],mapFile[2]);
			}*/
#endif
		}
		else if(buf[result] == 0x02)
		{
			mapFile[0] = 0x3F00;

			mapFile[1] = 0x3F00;

			mapFile[2] = BuildWord(&buf[result+1]);
		}
		else //8F数据长度错误，报错
		{
			return E_Parameter;
		}
		
		if(mode == NOT_PROFILE_MODE)//modify by zhxx@20230131,个人化有预置profile时，测试eSIM_BugList/8-1会有问题，该profile中会对NAA进行映射，所以下载profile的过程中不做判断，兼容性更好一些
		{
			//KI文件,鉴权opc等参数文件 pin/puk文件,不允许被映射
			if(((mapFile[2] == EF_KI)&&((mapFile[1] == G_DF_ID_USIM)
	#if UICC__AUTH__CDMA__CAVE
				||(mapFile[1] == G_DF_ID_CSIM)
	#endif
	#if UICC__ISIM
			||(mapFile[1] == G_DF_ID_ISIM)
	#endif
			))
				|| ((mapFile[2] == EF_PRIVATE_AUTHCFG)&&((mapFile[1] == G_DF_ID_USIM)
	#if UICC__ISIM
			||(mapFile[1] == G_DF_ID_ISIM)
	#endif
			))
				|| (((mapFile[2] == EF_PRIVATE_PUK)||(mapFile[2] == EF_PRIVATE_PIN))&&(mapFile[1] == DF_PROFILECFG))
			|| ((mapFile[2] == EF_Ki_TPX)&&(mapFile[1] == DF_PHASE)) // modify by xuweihua@20220107
				|| ((mapFile[2] == EF_AuthCFG_TPX)&&(mapFile[1] == DF_MF)))
			{
				return E_Parameter;
			}
	#if (EUICC_ONLY||SE_AND_EUICC)
			if(mode != NOT_PROFILE_MODE)
			{
				mapFileAddr = SearchFile(G_CreatingMFOffset,mapFile[0]);
			}
			else
	#endif
			{
				mapFileAddr = SearchFile(E_FILE_START,mapFile[0]);
			}
	
			if(mapFileAddr != FALSE)
			{
				mapFileAddr = SearchFile(mapFileAddr,mapFile[1]);
			}

			if(mapFileAddr != FALSE)
			{
				mapFileAddr = SearchFile(mapFileAddr,mapFile[2]);
			}


			if(mapFileAddr != FALSE)
			{
				result = ReadEECacheByte(mapFileAddr + F_MapAndRead_Offset);//创建文件时，带有92TAG,值为1，则改文件不允许被读，不允许被映射，此处不能调用GetFileHeadByte
		
				if((result&File_MapAndRead) == File_MapAndRead)
				{
					return E_Parameter;
				}
			}

		}

		fileStatus = fileStatus|F_MAP_FILE;

		if((fileType&0x37) == TRANTYPE)
		{
	    	transEfFileHead.SpaceSize = 0x0000;
	    }
#if UICC__FUNCTION__BERTLVCmd
		else if((fileType&0x3F) == BERTLVTYPE)  //BER-TLV
		{
			bertlvFileHead.ReservedSize = 0x0000;
		}
#endif
	    else
	    {
			recordEfFileHead.RecLength = 0x00;

			recordEfFileHead.RecNumber = 0x00;

			fileSize = 0x0000;
		}
	}
	else
	{

		if(mode != NOT_PROFILE_MODE)
		{
			if(FindTag(&buf[0],0x92,len) != FALSE)//不是映射文件，且有92tag，必须有文件内容
			{
#if (EUICC_ONLY||SE_AND_EUICC)
				SetBit(G_PEControlContext.IsExistFlag2,ContentRequired);
#endif
			}
		}

		if(fileSize == 0)
		{
			#if SE_ONLY
			    //return E_Parameter; //modify by zhxx@20220816,代码评审修改，不影响SE的卡片
			#else//EUICC 卡
				
				if(mode == NOT_PROFILE_MODE)//非profile模式创建文件
				{
					return E_Parameter; 
				}
				else//下载profile过程中创建文件
				{
					if(MOBILE_Operator == OPERATOR_CHINATELECOM)//modify by zhxx@20220630-HB_Offline_Test_02/14-PROFILE PE数据异常测试/Profile_FulSeg0053
					{
						return E_Parameter;
					}
					else//modify chentao@20220610 为了兼容德国profile 文件大小为0 的profile能下载成功
					{
						
					}
				}
			#endif
			
			
		}
		mapFile[0] = 0x0000;

		mapFile[1] = 0x0000;

		mapFile[2] = 0x0000;

		fileStatus = fileStatus&(~F_MAP_FILE);

		if((fileType&0x37) == TRANTYPE)  //透明文件
		{
			transEfFileHead.SpaceSize = fileSize;
		}
#if UICC__FUNCTION__BERTLVCmd
		else if((fileType&0x3F) == BERTLVTYPE)  //BER-TLV
		{
			bertlvFileHead.ReservedSize = fileSize;
		}
#endif
		else
		{
			result = FindTag(&buf[0],0x82,len);

			recordEfFileHead.RecLength = buf[result+4];						//Record length in FCP is WORD
			if(recordEfFileHead.RecLength == 0x00)   //长度为0  也报错
			{
				if(buf[result+3] != 0)//如果记录长度为0x100等，抛错
				{
					return E_Parameter;
				}
				
				recordEfFileHead.RecNumber = 0;//联通UICCAPI测试，api_1_fvw_incr，会创建记录长度是0的文件
			}
			else
			{
				if ((fileSize%recordEfFileHead.RecLength) != 0x00)
				{
					return E_Parameter;
				}
			
				if(((fileType&0x37) == CYCTYPE) && (recordEfFileHead.RecLength == 0xFF)) //记录文件大小最大255  循环文件  254
				{
					return E_Parameter;
				}
			
				if((fileSize/recordEfFileHead.RecLength) > 254)
				{
					return E_Parameter;
				}

				recordEfFileHead.RecNumber = (BYTE)((fileSize/recordEfFileHead.RecLength)&0x00FF);
			}			
			if((fileType&0x37) == CYCTYPE)  //对于循环文件来说，多创建一条记录以实现循环文件的断电保护，added @20110721 by xuyz
			{
				if(recordEfFileHead.RecNumber == 0xFF )   //不允许创建255条的循环记录文件
				{
					return E_Parameter;
				}

				fileSize = recordEfFileHead.RecNumber * recordEfFileHead.RecLength + 1;   //循环文件的实际大小
			}
		}
	}
#if UICC__FUNCTION__BERTLVCmd
	if((fileType&0x3F) == BERTLVTYPE)  //BER-TLV
	{
		result = FindTag(&buf[0],0xA5,len);                             //tag 'A5' is  mandatory for BER-TLV structure EFs

		if(result == FALSE)
		{
			//return E_Parameter;

			bertlvFileHead.FileDetails = 0x01;

			bertlvFileHead.MaxSize = 0x0000;
		}
		else
		{
		len =  buf[result];

		buf = &buf[result+1];

		result = FindTag(&buf[0],0x84,len);               //tag '84' is mandatory for BER-TLV structure EFs

		if(result == FALSE)
		{
			//return E_Parameter;
			bertlvFileHead.FileDetails = 0x01;
		}
		else
		{
			bertlvFileHead.FileDetails = buf[result+1];
		}

		result = FindTag(&buf[0],0x86,len);               //tag '86' is optional for BER-TLV structure EFs ,if no present, all available memory of the UICC may be allocated to the file

		if(result == FALSE)
		{
			//return E_Parameter;
			//bertlvFileHead.MaxSize = fileSpareSize - bertlvFileHead.ReservedSize-sizeof(bertlvFileHead);
			bertlvFileHead.MaxSize = 0;
		}
		else
		{
			if((fileStatus&F_MAP_FILE) != F_MAP_FILE)									//forbid init map file
			{
					if(buf[result] == 0x01)				  
					{
						bertlvFileHead.MaxSize = buf[result+1];
					}
					else if(buf[result] == 0x02)
					{
						bertlvFileHead.MaxSize = BuildWord(&buf[result+1]);
					}
					else
					{
						return E_Parameter;
					}
				}
				else
				{
					bertlvFileHead.MaxSize = 0x0000;
				}
			}
		}
		if(bertlvFileHead.MaxSize == 0)//modify by chentao@20231021 8_2_3_28
		{
			bertlvFileHead.MaxSize = bertlvfileSize;
		}

		fileNeedSize = bertlvFileHead.ReservedSize + bertlvFileHead.MaxSize+sizeof(BERTLVFileHead);
	}
	else
	{
		fileNeedSize = fileSize + sizeof(TranFileHead);    //sizeof(TranFileHead) == sizeof(recordEfFileHead)
	}
#else
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH	
		if(heFlashTag == TRUE)
		{
			if((fileSize + Num_HeFlashPage_ManageBytes) > PFPageSize) //4// 4字节是页是否已占用标记
			{
				return E_Parameter;
			}

			fileNeedSize = sizeof(TranFileHead);    //强更新文件体都备份到 强更新区域
		}
		else
#endif		
		{
			fileNeedSize = fileSize + sizeof(TranFileHead);    //sizeof(TranFileHead) == sizeof(recordEfFileHead)
		}
#endif
	
	currentDfUsedSize = GetFileHeadUsedSize(currentDfAddr);
#if (EUICC_ONLY||SE_AND_EUICC)
	if((ValueBit(G_SystemStatus,B_RfmPerform) == TRUE) && (mode == NOT_PROFILE_MODE))
	{
		fileStatus = (fileStatus &0xF0) | 0x05;//远程创建文件，文件状态默认为05
	}
#else
	
	if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)           
	{	
		arrName |= 0x7F;       //RFM创建的文件拥有RFM 的任何权限
		
		fileStatus = (fileStatus &0xF0) | 0x05;//远程创建文件，文件状态默认为05
	}
	else
	{
		result = FindTag(&buf[0],0x8E,len);				//03.48 RFM权限

		if(result == FALSE)                     //TAG不存在
		{			
			arrName |= 0x7F; 	                      //和测试部 产业部讨论 不带8E tag时默认开启全部权限，如果需要关闭或指定某些权限根据需求配置 
		}
		else if(buf[result] == 0)              // TAG长度为0
		{
			arrName &= F_RFM_PERMIT_NONE;	          //8E TAG长度为0，默认RFM权限全部关闭
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
			Bit 6 ：Resize
			*/
			arrName |= (buf[result+1]&0x7F);    //arrName的高bit被用来定义是6F还是2F ，低7bit被用来	扩展RFM 权限
		}
	}
#endif	
	if((fileType&0x37) == TRANTYPE)
	{
		transEfFileHead.FileId = fileName;

		transEfFileHead.FileType = fileType;

		transEfFileHead.FileStatus = fileStatus;

		transEfFileHead.ArrNum = arrNum;

		transEfFileHead.ArrName = arrName;

		transEfFileHead.ShortFileId = shortFileId;

		//transEfFileHead.Rfu = 0xFF;

		transEfFileHead.MapFile[0] = mapFile[0];

		transEfFileHead.MapFile[1] = mapFile[1];

		transEfFileHead.MapFile[2] = mapFile[2];
	}
#if UICC__FUNCTION__BERTLVCmd
	else if((fileType&0x3F) == BERTLVTYPE)  //BER-TLV
	{
		bertlvFileHead.FileId = fileName;

		bertlvFileHead.FileType = fileType;

		bertlvFileHead.FileStatus = fileStatus;

		bertlvFileHead.ArrNum = arrNum;

		bertlvFileHead.ArrName = arrName;

		bertlvFileHead.ShortFileId = shortFileId;

		bertlvFileHead.MapFile[0] = mapFile[0];

		bertlvFileHead.MapFile[1] = mapFile[1];

		bertlvFileHead.MapFile[2] = mapFile[2];

		bertlvFileHead.UsedSize	  = 0x0000;
	}
#endif
	else
	{
		recordEfFileHead.FileId = fileName;

		recordEfFileHead.FileType = fileType;

		recordEfFileHead.FileStatus = fileStatus;

		recordEfFileHead.ArrNum = arrNum;

		recordEfFileHead.ArrName = arrName;

		recordEfFileHead.ShortFileId = shortFileId;

		//recordEfFileHead.RecIndicator = 0x01;

		recordEfFileHead.MapFile[0] = mapFile[0];

		recordEfFileHead.MapFile[1] = mapFile[1];

		recordEfFileHead.MapFile[2] = mapFile[2];
	}

	ecpMem_Set((BYTE *)&protectDo, 0x00, sizeof(NewResizeAntiTearing));

	if((fileType&0x3F) != BERTLVTYPE)  //非 BER-TLV文件
	{
		result = FindTag(&buf[0],0xA5,len);
		
		if(result == FALSE)
		{
			result = FindTag(&buf[0],0x85,len);
		}

		if(result == FALSE)
		{
			buf = NULL;
		}
		else
		{
			buf = &buf[result];
		}
		
		if((buf == NULL)||(buf[0] > 0x2F)||((fileStatus&F_MAP_FILE) == F_MAP_FILE))          //buf此时从数据长度开始
		{
			protectDo.FileInfo[0x10] = 0xFF;
		}
		else
		{
			ecpMem_Copy(&protectDo.FileInfo[0x10],&buf[0], buf[0]+1);
		}
	}
	else
	{
		protectDo.FileInfo[0x10] = 0xFF;
	}
	

#if (EUICC_ONLY||SE_AND_EUICC)
	if(mode != NOT_PROFILE_MODE)//profile创建模式
	{
		mfUsedsize = GetFileHeadUsedSize(G_CreatingMFOffset);
		
		if((fileNeedSize >= GetHeapBase()) || ((G_CreatingMFOffset + mfUsedsize + fileNeedSize) > GetHeapBase())) // 剩余空间不够
		{
			return E_FileSize;
		}
	}
	else
#endif
	{
		mfUsedsize = GetMFUsedSize();
		
		if(AllocSpaceEnough(fileNeedSize) == FALSE)
		{
			return E_FileSize;
		}		
	}
#if UICC__FUNCTION__2B80_AT_FILEEND
	//在MF 下创建文件，如果DF_Toolkit_P_Ex/EF_ToolkitApp已经创建，那么要把他们往后移动，保证EF_ToolkitApp是最后一个文件, by xwh 20210710
	if(currentDfAddr == E_FILE_START)
	{		
		toolkitAddr = SearchFile(E_FILE_START,DF_Toolkit_P_Ex);

		if(toolkitAddr != FALSE)
		{
			bb00DfUsedSize = GetFileHeadUsedSize(toolkitAddr);
			
			toolkitAddr = SearchFile(toolkitAddr,EF_ToolkitApp);

			if(toolkitAddr != FALSE)
			{
				currentDfUsedSize -= bb00DfUsedSize;
			}
		}
	}
#endif
#if (EUICC_ONLY||SE_AND_EUICC)
	if(mode != NOT_PROFILE_MODE)//profile创建模式
	{
		if((G_CreatingMFOffset + mfUsedsize) >= (currentDfAddr+ currentDfUsedSize))
		{
			dataLen = G_CreatingMFOffset + mfUsedsize - (currentDfAddr+ currentDfUsedSize);
		}
		else
		{
			faultDetected(); // 出现这种情况时，卡片已经坏掉了，就不继续往下走
		}
	}
	else
#endif
	{
		dataLen = E_FILE_START + mfUsedsize - (currentDfAddr+ currentDfUsedSize);
#if (EUICC_ONLY||SE_AND_EUICC)

		protectDo.ProfileNum = 0;

		VMReadBuf(temp,PROFILE_VALIDTAG_CFG,PROFILE_MAX_NUMBER);    //获取当前所有profile的状态
		
		for(i= 0; i< PROFILE_MAX_NUMBER ; i++)
		{		  
			if(temp[i] != PROFILE_INVALIDFLAG)
			{
				currentMFOffset = VMReadUword(PROFILE_MFOFFSET_CFG + i*sizeof(UWORD)); 
				
				if((currentMFOffset > E_FILE_START)&&(currentMFOffset != 0xFFFFFFFF))
				{
					protectDo.ProfileIndex[protectDo.ProfileNum] = i;
					
					protectDo.ProfileNewMFOffset[protectDo.ProfileNum] = currentMFOffset+fileNeedSize;
					
					currentMFUsedSize = GetFileHeadUsedSize(currentMFOffset);
					
					dataLen += currentMFUsedSize;
					
					protectDo.ProfileNum++;   //找到较大的那个MF 的偏移
				}
			}
		}
#endif		
	}
	//mfUsedsize = GetFileHeadUsedSize(currentDfAddr);

	VMReadBuf((BYTE xdata *)&currentDfMaxsize,currentDfAddr + F_MSize_Offset,sizeof(UWORD));
		
	if(dataLen == 0)  //1、当前待创建的文件位于现有文件系统的末尾，此时不需要移动数据，只要在空间足够的情况下，创建文件即可2、当前DF的最大空间大于已使用空间，也不需要移动
	{
		//protectDo.Tag  |= 0x01;			//No Need Move
	}
	else//modify by zhxx@20230328,解决下载proifle过程中创建文件时，需要移动文件速度慢的问题，方案：将一条指令中的多次移动合并到一次移动
	{
		if(mode == PROFILE_MODE_GENERIC)//下载profile中创建文件需要移动的话，需要特殊处理,只有通用方式创建文件文件系统才会有移动的情况
		{
			if((currentDfMaxsize <= currentDfUsedSize)) //下载profile过程中， 当前DF的最大空间大于已使用空间，不需要移动数据
			{
				modifyLen = PreAnalyseCreateFile();//第一个需要移动的文件，向后面预解析，看有没有需要移动的文件，合并在一起移动
				
				if((G_CreatingMFOffset+mfUsedsize+fileNeedSize+modifyLen) > GetHeapBase()) // add by chentao@20240301
				{
					return E_FileSize;
				}

				secureMoveStarted = TRUE;//注意，如果在预解析时，profile后面数据比较短，导致返回的modifyLen是0时，此次创建文件系统也是需要移动的
			}
			/*(else//后面的文件已经预留了最大空间，不需要再移动
			{
				protectDo.Tag  |= 0x01;			//No Need Move
			}*/
		}
		else//本地或者其它方式创建文件需要挪动的话，直接移动
		{  
			secureMoveStarted = TRUE;//非profile mode
//如果事务已开启，又要移动数据，不允许这种操作，报错,by xwh 20200707
			if(isInProgress())
			{
				return E_ConditionNotSat;
			}
		}

	}
	i = PreModifyFileSize(&protectDo,currentDfAddr,fileNeedSize,mode,modifyLen);
	
	if(i == DF_Depth)  //COS 最大支持5级DF，由数组protectDo.ParentAddr[0]决定
	{
		return E_Parameter;
	}

	if((fileType&0x37) == TRANTYPE)   //创建文件时，将文件初始化内容备份到数组中，当初始化内容过长时，则断电后默认将文件初始化成全F
	{
		ecpMem_Copy(&protectDo.FileInfo[0], (BYTE  *)&transEfFileHead.FileId,sizeof(TranFileHead));				
	}
#if UICC__FUNCTION__BERTLVCmd
	else if((fileType&0x3F) == BERTLVTYPE)
	{
		ecpMem_Copy(&protectDo.FileInfo[0], (BYTE  *)&bertlvFileHead.FileId,sizeof(BERTLVFileHead));	
	}
#endif
	else
	{
		ecpMem_Copy(&protectDo.FileInfo[0], (BYTE  *)&recordEfFileHead.FileId,sizeof(RecordFileHead));
	}	
	
	if(secureMoveStarted == TRUE)
	{
		//SetEepromEX(E_SECUREMOVE_START+MoveEepAntiTeaingTag_Offset, D_EEPNoMoveTag, sizeof(BYTE));    //éè??±ê??	

		//IF_CommitTransaction(isInProgress());
		//如果事务已开启，又要移动数据，不允许这种操作，报错,by xwh 20200707
		if(isInProgress())
		{
			return E_ConditionNotSat;
		}
	}
	TransactionStarted = IF_BeginTransaction();

	if(secureMoveStarted == TRUE)
	{
#define moveDateBuf  (tempBuf + 100)
		SetMovDateBufSrcDstDataLen(moveDateBuf,currentDfAddr + currentDfUsedSize,currentDfAddr+ currentDfUsedSize + fileNeedSize + modifyLen,dataLen);

		SetUWord(&moveDateBuf[SecureMoveEepromWriteDesAddr_Offset],currentDfAddr+currentDfUsedSize);

		if((fileType&0x37) == TRANTYPE)
		{
			moveDateBuf[SecureMoveEepromWriteDataLen_Offset] = sizeof(TranFileHead);
			ecpMem_Copy(&moveDateBuf[SecureMoveEepromWriteDataBuf_Offset], (BYTE *)&transEfFileHead.FileId,sizeof(TranFileHead));
		}
#if UICC__FUNCTION__BERTLVCmd
		else if((fileType&0x3F) == 0x39)
		{
			moveDateBuf[SecureMoveEepromWriteDataLen_Offset] = sizeof(BERTLVFileHead);
			ecpMem_Copy(&moveDateBuf[SecureMoveEepromWriteDataBuf_Offset], (BYTE *)&bertlvFileHead.FileId,sizeof(BERTLVFileHead));
		}
#endif
		else
		{
			moveDateBuf[SecureMoveEepromWriteDataLen_Offset] = sizeof(recordEfFileHead);
			ecpMem_Copy(&moveDateBuf[SecureMoveEepromWriteDataBuf_Offset], (BYTE *)&recordEfFileHead.FileId,sizeof(recordEfFileHead));
		}

		//映射文件，不需要初始化
		if((fileStatus&F_MAP_FILE) != F_MAP_FILE)			
		{
			SetUWord(&moveDateBuf[SecureMoveEepromInitDesAddr_Offset],currentDfAddr+currentDfUsedSize);
			moveDateBuf[SecureMoveEepromInitHeRecordNo_Offset] = heRecordNo;
			SetWord(&moveDateBuf[SecureMoveEepromInitStartOffset_Offset],0x00);
			ecpMem_Copy(&moveDateBuf[SecureMoveEepromInitDataLen_Offset], (BYTE *)&protectDo.FileInfo[0x10],0x30);
		}
						
		VMWriteBuf(E_SECUREMOVE_START, moveDateBuf, SecureMoveStuct_LengthAll);	
#undef moveDateBuf
	}

	if(secureMoveStarted == FALSE)
	{
		if((fileType&0x37) == TRANTYPE)
		{
			result = VMWriteBuf(currentDfAddr+currentDfUsedSize,(BYTE *)&transEfFileHead.FileId,sizeof(TranFileHead));
		}
#if UICC__FUNCTION__BERTLVCmd
		else if((fileType&0x3F) == BERTLVTYPE)
		{
			result = VMWriteBuf(currentDfAddr+currentDfUsedSize,(BYTE *)&bertlvFileHead.FileId,sizeof(BERTLVFileHead));
		}
#endif
		else
		{
			result = VMWriteBuf(currentDfAddr+currentDfUsedSize,(BYTE *)&recordEfFileHead.FileId,sizeof(recordEfFileHead));
		}
		
		if(result == FALSE)
		{
			return FALSE;
		}
	}

	AddChildDfOrEfNum(currentDfAddr,F_ChildEFNum_Offset,mode);  //更新父DF下的EF数目
	
	ModifyFileSize(protectDo,modifyLen);

#if (EUICC_ONLY||SE_AND_EUICC)
	for(i = 0 ; i < protectDo.ProfileNum; i++)
	{
		 VMWriteUword(PROFILE_MFOFFSET_CFG + protectDo.ProfileIndex[i]*sizeof(UWORD),protectDo.ProfileNewMFOffset[i]);
	}
#endif	
		
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	if(heFlashTag == TRUE)
	{
		SetEepromEX(ADDR_FILENO_HEFLASH,fileNo,0x01);
	}
#endif	
#if High_Update_InRAM_New	
	if(update_ram_new != FALSE)
	{
	   WriteEepromEX(Config_High_Update_InRAM_num, update_new_buf, sizeof(update_new_buf));		//更新index /size.
	}
#endif
	IF_CommitTransaction(TransactionStarted);

	if(secureMoveStarted == TRUE)
	{
		StartSecureMoveEeprom(0,0,0);
	}
	else
	{
		if((fileStatus&F_MAP_FILE) != F_MAP_FILE)	
		{		
			InitTransOrRecordFile(currentDfAddr+currentDfUsedSize,0x00,buf,heRecordNo,mode);			
		}
	}
	
	//return fileNeedSize;
	return (currentDfAddr+currentDfUsedSize);
}
#if UICC__FUNCTION__2B80_AT_FILEEND
/*
*@Function name :	 AddToolkitAppRecord
*
*@Deiscription:    在EF_ToolkitApp 文件记录不足时，增加一条记录
*                        删除应用时，如果删除的是EF_ToolkitApp 文件最后一条记录，则将记录数减一
*
*@Data    content:  
*
*@input parameter:   recordNo: 0(增加记录)/x(删除记录)
*
*@output parameter:  Record No/TRUE/FALSE
*
*
*@author:		create by xwh 20210710    2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
BYTE AddOrDeleteToolkitAppRecord(BYTE recordNo)
{
	UWORD idata toolkitAppAddr,dfAddr;
	BYTE  idata toolkitAppRecNum,toolkitAppRecLen,TransactionStarted;
	UWORD idata currentDfUsedSize;

	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)   //文件系统未创建时，执行此函数无意义,如初始化isd会进入到该函数中
	{
		return FALSE;
	}

	toolkitAppRecNum = GetLFRecNumLgth(toolkitAppAddr, FileNum);
	toolkitAppRecLen = GetLFRecNumLgth(toolkitAppAddr, FileLgth);

	if(recordNo == 0)
	{
		//预留一条记录的空闲空间，防止越界
		if(AllocSpaceEnough(2*toolkitAppRecLen) == FALSE)
		{
			return FALSE;
		}
	}
	else
	{
		if((recordNo != toolkitAppRecNum)||(toolkitAppRecNum == 1))
		{
			return FALSE;
		}
	}

	TransactionStarted = IF_BeginTransaction();

	if(recordNo == 0)
	{
		toolkitAppRecNum ++;
	}
	else
	{
		toolkitAppRecNum --;
	}
	
	VMWriteBuf(toolkitAppAddr+F_RecNum_Offset, (BYTE  *)&toolkitAppRecNum, sizeof(BYTE));

	dfAddr = SelectFile(E_FILE_START,DF_Toolkit_P_Ex) ;

	currentDfUsedSize = GetFileHeadUsedSize(dfAddr);

	if(recordNo == 0)
	{
		currentDfUsedSize += toolkitAppRecLen;
	}
	else
	{
		currentDfUsedSize -= toolkitAppRecLen;
	}
	
	VMWriteBuf(dfAddr+F_UsedSize_Offset, (BYTE *)&currentDfUsedSize, sizeof(UWORD));	//写各父目录的已用空间字段

	currentDfUsedSize = GetFileHeadUsedSize(E_FILE_START);

	if(recordNo == 0)
	{
		currentDfUsedSize += toolkitAppRecLen;
	}
	else
	{
		currentDfUsedSize -= toolkitAppRecLen;
	}
	
	VMWriteBuf(E_FILE_START+F_UsedSize_Offset, (BYTE *)&currentDfUsedSize, sizeof(UWORD));	//写各父目录的已用空间字段

	//增加记录时，将新增加的记录初始化成全FF， by xwh 20210713
	if(recordNo == 0)
	{
		SetLFConst(toolkitAppAddr,toolkitAppRecNum,0x00,0xFF,toolkitAppRecLen);
	}

	IF_CommitTransaction(TransactionStarted);

	return toolkitAppRecNum;
}
#endif

