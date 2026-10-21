/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCToolkitRegistery.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2013-11-05

*Description :该文件主要用于实现基于TS102241 所描述的interface FileView的功能 ，
*
*History     :
	
author    time      version    description
xuyz   2013/11/05   V1.0       创建该文件

xuyz   2013/11/11   V1.2       增加fileView对象的上下文在java层和native层的切换，
                               该部分代码的实现是基于函数 ToolkitCopyJavaContextToNativeContext	,ToolkitCopyNativeContextToJavaContext
************************************************************************************************************************************************************************************************/
#include    "fileAction.h"
#include    "gsmpublic.h"
#include    "hard_api.h"
#include    "fileSystem.h"
#include     "interface.h"
#include    "UICCToolkitAPI.h"
#include    "UICCToolkitConstants.h"
//#include    "transaction.h"
//#include   "Interpreter.h"
#include  "uicc_interface.h"
#include  "STKFileView.h"
#include  "Object.h"
#include  "FireWall.h"
#include  "ToolkitAPIPublic.h"
#include "platform_api.h"
#include "ToolkitRFM.h"
#include "EUICCInterface.h"
#include "ob_manager.h"
#if UICC__FUNCTION__UICCAPI
BYTE    xdata    G_ToolkitFileContextEvent;  //0x00:永久对象，0x01:clear on reset,0x02:clear on deselect,0x03空闲
/*
*@function    :Toolkit_GetADFName
*
*@param       :查找特定的AID在文件系统中的FID
*
*@param		  :void
*
*@return      :返回特定的ADF Aid在COS所保存的FID
*
*@author      :xuyz
*
*@history      :V1.0 :create by xuyz@20131223
*/
void UICCToolkit_GetADFName(void)
{
	UWORD xdata fileAddr;
	WORD idata fileName;
	
	jshort aidLength = PopOperandStackShort();
    
	jshort aidOffset = PopOperandStackShort();
    
	jref aidBuffer = PopOperandStackShort();

	check_bounds_interval_buffer(aidBuffer,aidOffset,aidLength);

	CopyArrayDataToDest(&tempBuf[100],aidBuffer,aidOffset,aidLength);

	fileAddr = SelectFileByName(E_FILE_START,&tempBuf[100],aidLength);
	
	if(fileAddr == FALSE)
	{	
		PushOperandStackShort(FALSE);
		return;
	}
	
	VMReadBuf((BYTE *)&fileName,fileAddr,sizeof(WORD));

	PushOperandStackShort((jshort)fileName);	
}
/*
*@function     :JVMImpl_ViewContextCheck
*
*@description  :
*
*
*@return       :void
*
*@author       :
*
*@history	   :
*
*@others       :getShareInterfaceObjecti的参数如下(NULL,0X01)
*/
void JVMImpl_ViewContextCheck(unsigned short orContext)
{
	PF8 X_A objAddr;

	objAddr = MEMtPF8(locateObjectInMemory(orContext));//获取对象的地址

	checkArrayRead(objAddr);	//该处主要用于判断如果访问的是一个CLEAR_ON_DESELECT对象，则访问该对象的应用必须是创建该对象的应用
}
/*
*@function      :ToolkitCopyJavaContextToNativeContext
*
*@description   :将当前fileView对象的上下文拷贝到native层进行后续数据操作 ,fileContext的数据长度为16字节
*
*@param         :objref_t toolkitJavaContext
*
*@return         :void
*
*@author        :xuyz
*
*@history       :V1.0 :create by xuyz
*
*@others     :假设当前COS从MF开始最多支持4级DF
*/
void UICCToolkitCopyJavaContextToNativeContext(void)
{
	UWORD xdata fileAddr;
	BYTE idata num,fileType,backSfi;
	WORD idata fileName;
	
	jbyte idata event =  PopOperandStackShort();
    
	jshort idata adfName  = PopOperandStackShort();
    
	jbyte idata accessMode  = PopOperandStackShort();
    
	jref idata fileContext = PopOperandStackShort();

	JVMImpl_ViewContextCheck(fileContext); //该处主要用于判断如果访问的是一个CLEAR_ON_DESELECT对象，则访问该对象的应用必须是创建该对象的应用

	CopyArrayDataToDest(tempBuf,fileContext,0,FileContextLength);

    G_ToolkitCurrentAdfName = FALSE; 

	G_ToolkitFileContextEvent = event;  //0x00:永久对象，0x01:clear on reset,0x02:clear on deselect,0x03空闲

	if((tempBuf[0] == 0x00)||(tempBuf[0] == 0xFF))	  //第一次调用该函数，则根据实际情况初始化当前df ，当前df为mf或某个adf
	{
		G_ToolkitFileSystemMode = accessMode;

		if((accessMode & 0xA1) == ADFFileSystem)
		{
			fileAddr = SelectFile(E_FILE_START,adfName);

			Toolkit_SetChannel(fileAddr,adfName,FALSE,UICCAPI_Mode);
		}
		else
		{
			Toolkit_SetChannel(E_FILE_START,0x3F00,FALSE, UICCAPI_Mode);
		}
	}
	else
	{
		G_ToolkitFileSystemMode = tempBuf[0];
	
		fileAddr = E_FILE_START;

		for(num = 0x00;num < tempBuf[2]; num += 2)  //toolkitapp最多支持4层路径  ，toolkit app存储路径的目的是为了防止其中某个app删除文件后，导致其他的app文件查找不到
		{
	   		 fileName = ((tempBuf[3+num]<<8)&0xFF00) + tempBuf[4+num];

			 backSfi = G_CurrentSfi;
            
			 fileAddr = SelectFile(fileAddr,fileName);
            
			 G_CurrentSfi = backSfi;
			 
			 if(fileAddr == FALSE)
			 {
				 break;
			 }
			else
			{
				Toolkit_SetChannel(fileAddr,fileName,FALSE, UICCAPI_Mode);

			    fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

				if(fileType != DFTYPE)  //若当前路径中的EF可以找到，则使用toolkit本身保存的记录号信息
				{
					G_ToolkitCurrentRecNum = tempBuf[1];   //恢复当前Toolkit记录号信息
				}
			}
		 }
	}
}

/*
*@function   :ToolkitCopyNativeContextToJavaContext
*
*@description:当前某个fileView对象的上下文改变后，在UICC API函数执行完后，改变其上下文
*
*@param      :fileContext
*
*@return     :void
*
*@author     :xuyz
*
*@history    :V1.0 :create by xuyz@20131111
*
*@others     :假设当前COS从MF开始最多支持4级DF，
*/

void UICCToolkitCopyNativeContextToJavaContext(void)
{
	BYTE  xdata filePath[0x0A];
	BYTE idata filePathLgth;
	WORD idata parentName,secondeParentName;

	jref idata fileContext = PopOperandStackShort();
	
	tempBuf[0] = G_ToolkitFileSystemMode;
    
	tempBuf[1] = G_ToolkitCurrentRecNum;

	filePathLgth = 0x00;

	if(G_ToolkitCurrentEfAddr != FALSE )
	{
		filePath[0x0A-filePathLgth-2] = (G_ToolkitCurrentEfName>>0x08)&0xFF;
		filePath[0x0A-filePathLgth-1] = G_ToolkitCurrentEfName&0xFF;

		filePathLgth += 0x02;
	}
	
	filePath[0x0A-filePathLgth-2] = (G_ToolkitCurrentDfName>>0x08)&0xFF;
	filePath[0x0A-filePathLgth-1] = G_ToolkitCurrentDfName&0xFF;

	filePathLgth += 0x02;

	if(G_ToolkitCurrentDfName != 0x3F00)   //有上级DF
	{
		secondeParentName = GetFileHeadWord(G_ToolkitCurrentDfAddr, F_SecondParentName_Offset); 
	
		filePath[0x0A-filePathLgth-2] = (secondeParentName>>0x08)&0xFF;
		filePath[0x0A-filePathLgth-1] = secondeParentName&0xFF;
	
		filePathLgth += 0x02;
	
		if(secondeParentName != 0x3F00)	   //假设目前最大支持4级DF
		{
			parentName = GetFileHeadWord(G_ToolkitCurrentDfAddr, F_ParentName_Offset);	 
			
			filePath[0x0A-filePathLgth-2] = (parentName>>0x08)&0xFF;
			filePath[0x0A-filePathLgth-1] = parentName&0xFF;
		
			filePathLgth += 0x02; 

			if(parentName != 0x3F00)
			{
				if((G_ToolkitFileSystemMode&0xA1) == ADFFileSystem)//ADF文件系统	UICC测试API4-AFV-DLTF在ADF下建3级DF1级EF，超出COS处理能力。为测过脚本，将ADF做特殊处理。如DF目录大于4级，将最上层目录设为当前ADF
				{
					if(parentName == G_ToolkitCurrentAdfName)
					{
						filePath[0x0A-filePathLgth-2] = 0x3F;
						filePath[0x0A-filePathLgth-1] = 0x00;
					}
					else
					{
						filePath[0x0A-filePathLgth-2] = (G_ToolkitCurrentAdfName>>0x08)&0xFF;
						filePath[0x0A-filePathLgth-1] = G_ToolkitCurrentAdfName&0xFF;
					}
				}
				else   //UICC文件系统或SIM 文件系统
				{
					filePath[0x0A-filePathLgth-2] = 0x3F;
					filePath[0x0A-filePathLgth-1] = 0x00;
				}
				
					filePathLgth += 0x02; 
			}
		}
	}

	tempBuf[2] = filePathLgth;

	ecpMem_Copy(&tempBuf[3], &filePath[0x0A-filePathLgth],filePathLgth);	    //拷贝filePath到tempBuf，更新到fileView对象中去

	CopySrcDataToArray(tempBuf,fileContext,0,FileContextLength);	
}
/**
*@function    :UICCToolkit_selectPataByteSFI
*
*@description : This method selects a file by SFI in the current directory of the FileView.<br> The file context associated with the FileView object is changed after  successful execution.
*               This SELECT method allows to update the current file without handling the Select Response.
*
*@param       :sfi  is the Short File Identifier of the file to be selected.
*
*@return      :void
*
*@exception   :UICCException  in case of error
*              FILE_NOT_FOUND if a file with the corresponding SFI is not found or if the sfi parameter is invalid.
*              MEMORY_PROBLEM
*              INTERNAL_ERROR
*              CONDITIONS_OF_USE_NOT_SATISFIED
*
*@author       :xuyz
*
*@history      :V1.0:create by xuyz@20131107
*/
void UICCToolkit_selectPataByteSFI(void) 
{
	UWORD xdata fileAddr;
	BYTE idata fileType;
	WORD idata fileName;

   	jbyte idata sfi = PopOperandStackShort();

	if((sfi < 0x01)||(sfi >0x1E))
	{
		UICCThrowUICCException(FILE_NOT_FOUND);
	}

	if(G_ToolkitCurrentDfAddr == FALSE)
	{
		UICCThrowUICCException(FILE_NOT_FOUND);
	}

	fileAddr = SelectFileByName(G_ToolkitCurrentDfAddr,(BYTE*)&sfi,0x00);							//Modify at 2007-02-07

	if(fileAddr == FALSE)															//Add at 2007-02-07
	{
	 	UICCThrowUICCException(FILE_NOT_FOUND);
	}
			
	ReadEeprom((BYTE idata *)&fileName,fileAddr,sizeof(WORD));

	fileType = GetFileHeadByte(fileAddr,F_Status_Offset);

	if((fileType&F_MAP_FILE) == F_MAP_FILE)
	{
		fileAddr = GetParentOrMapFileAddr(fileAddr, F_MapFile_Offset);			
	}

	if(fileAddr == FALSE)  
	{
		UICCThrowUICCException(FILE_NOT_FOUND);
	}

	
	if(IF_ProcessNoShareableFile(fileAddr,fileName,0x01,UICCAPI_Mode) == FALSE)	//Api_1_Cont要求当选中的文件为非共享文件时，则抛出异常 FILE_NOT_FOUND
	{
		UICCThrowUICCException(FILE_NOT_FOUND);
	}

	Toolkit_SetChannel(fileAddr,fileName,FALSE, UICCAPI_Mode);
}
/**
*@function    :UICCToolkit_searchRecord
*
*@description :This method searches a pattern in the current linear fixed/cyclic EF.
*             <p>
*             Notes:If patOffset or pattLength or respOffset or respLengthparameter is negative an ArrayIndexOutOfBoundsException exception is thrown.
*                   If pattOffset+pattLength is greater than patt.length, the length of the patt array a ArrayIndexOutOfBoundsException exception is thrown and no search is performed.
*                   If respOffset+respLength is greater than response.length, the length of the response array a ArrayIndexOutOfBoundsException exception is thrown and no search is performed.
* 
*@param       :mode                                is the search mode, according to TS 102.221 (no type information).
*@param       :recordNum                           is the record number to start the search, 0 indicates from the current record
*@param       :searchIndication                    indication of the search mode for the enhanced search
*@param       :patt                                is the reference to the byte array containing the search pattern.
*@param       :pattOffset                          is the offset of the search pattern in the byte array.
*@param       :pattLength                          is the length of the search pattern.
*@param       :response                            reference to response array to hold the record numbers
*@param       :respOffset                          is the offset into the response array to store the record numbers
*@param       :respLength                          - is the maximum number of elements to be copied into the response array.
*              If the respLength is greater than the number of records found, the whole response is copied into the response buffer and the number of elements copied is returned by the method.
*              If the respLength is smaller than the number of records found, the first respLength record numbers are copied into the response buffer and respLength is returned by the method.
*
*@return      :number of array elements copied into the response array. Returns 0 if no record containing containing the pattern was found.
*
*@exception   NullPointerException            if patt is null
*@exception   ArrayIndexOutOfBoundsException  if searching would cause access of data outside array bounds
*@exception  UICCException                   in case of error
*             INTERNAL_ERROR
*             OUT_OF_RECORD_BOUNDARIESif pattLengthis greater than the record size
*             COMMAND_INCOMPATIBLE
*             SECURITY_STATUS_NOT_SATISFIED
*             REF_DATA_INVALIDATED
*             NO_EF_SELECTED
*             RECORD_NOT_FOUND: record with number recordNum not found in file
*             INVALID_MODE
*           COMMAND_NOT_ALLOWED  if the referenced file is terminated
*
*@author     :xuyz
*
*@history    :V1.0 :create by xuyz@20131108
*            :V1.1 :modify by xuxiaojun@20191009 白盒测试调整search的赋值位置
*
*@others     :patt为byte型数组
              response  为short型数组
*/
void UICCToolkit_searchRecord(void)
{
	RecordFileHead xdata  cyclicEf;
	signed char idata step;
	BYTE idata offset, tag,search[0x02];
	BYTE idata recNumStart,i;	
	DWORD xdata pattBuffer;
	UWORD idata backCurrentEFAddr;
	BYTE idata backSearch;
	jshort * resBuffer;
	UWORD xdata actualfileAddr;
	BYTE idata status;

   jshort idata respLength = PopOperandStackShort();
   jshort idata respOffset = PopOperandStackShort();
   jref idata response = PopOperandStackShort();	           //response 为short型数组
   jshort idata pattLength = PopOperandStackShort();
   jshort idata pattOffset = PopOperandStackShort();
   jref  idata patt = PopOperandStackShort();	            //patt为byte型数组
   jshort idata searchIndication = PopOperandStackShort();
   jshort idata recordNum =  PopOperandStackShort();
   jbyte idata mode = PopOperandStackShort();

   check_bounds_interval_buffer(patt,pattOffset,pattLength);

   check_bounds_interval_buffer(response,respOffset,respLength);

   if( (mode != SIMPLE_SEARCH_START_FORWARD)&&(mode != SIMPLE_SEARCH_START_BACKWARD)&&(mode != ENHANCED_SEARCH))
   {
   	   UICCThrowUICCException(INVALID_MODE);
   }

   if(recordNum < 0x00)
   {
   	   UICCThrowUICCException(RECORD_NOT_FOUND);
   }

	if(G_ToolkitCurrentEfAddr == FALSE)
	{
		UICCThrowUICCException(NO_EF_SELECTED);
	} 

	VMReadBuf((BYTE xdata *)&cyclicEf.FileId,G_ToolkitCurrentEfAddr,sizeof(RecordFileHead));
	
	if(((cyclicEf.FileType&0x07) != LINTYPE)&&((cyclicEf.FileType&0x07) != CYCTYPE )) //线形记录文件或者循环记录文件
	{
		UICCThrowUICCException(COMMAND_INCOMPATIBLE);
	}
	actualfileAddr  = SearchFile(G_ToolkitCurrentDfAddr, G_ToolkitCurrentEfName);

	status	= GetFileHeadByte(actualfileAddr,F_Status_Offset);
	
	if((status&0x01) == FILE_INVALIDATE)		//判断文件是否有效
	{
		UICCThrowUICCException(REF_DATA_INVALIDATED);
	}
	
	if(recordNum > cyclicEf.RecNumber)
	{
		UICCThrowUICCException(RECORD_NOT_FOUND);
	}

	if(pattLength > cyclicEf.RecLength)
	{
		UICCThrowUICCException(OUT_OF_RECORD_BOUNDARIES);	
	}

	if(Toolkit_GetKeyStatus(G_ToolkitCurrentEfAddr,A_ReadFile,UICCAPI_Mode) == FALSE)
	{
		UICCThrowUICCException(SECURITY_STATUS_NOT_SATISFIED);
	}


	step = 1;                                               //默认为forward
	
	offset = 0x00;											//默认是0偏移            

	tag = FALSE;											//默认是不使用first occurrence方式
	
	search[0] = (searchIndication>>8)&0xFF;
        
	search[1] = searchIndication&0xFF;

	if((mode&0x06) == 0x04)									//SIMPLE MODE
	{
		if(recordNum == 0x00)
		{
			if(G_ToolkitCurrentRecNum == 0x00)
			{
				UICCThrowUICCException(RECORD_NOT_FOUND);
			}

			recNumStart = G_ToolkitCurrentRecNum;

		}
		else
		{
			recNumStart = recordNum;	
		}

		if((mode&0x01) == 0x01)								//backward
		{
			step = -1;
		}
	}
	else
	{
		if((search[0]&0x08) == 0x08)  //After First Occurrence search
		{
			tag = TRUE;
		}
		else
		{
			if((search[1] + pattLength) > cyclicEf.RecLength)
			{
				UICCThrowUICCException(OUT_OF_RECORD_BOUNDARIES);	
			}

			offset = search[1];
		}

		if((search[0]&0x02) == 0x00)						//ABSOLUTE
		{
			if(recordNum == 0x00)
			{
				if(G_ToolkitCurrentRecNum == 0x00)
				{
					UICCThrowUICCException(RECORD_NOT_FOUND);
				}

				recNumStart = G_ToolkitCurrentRecNum;

			}
			else   
			{
				recNumStart = recordNum;
			}

			if((search[0]&0x01) == 0x01)					//backward
			{
				step = -1;
			}			
		}
		else
		{
			if((search[0]&0x03) == 0x03)                   //backward
			{
				step = -1;
			}
			if(G_ToolkitCurrentRecNum == FALSE)		//未指定
			{
				if(step == 0x01)                              //forward
				{
					recNumStart = 0x01;
				}
				else
				{
					recNumStart = cyclicEf.RecNumber;
				}
			}
			else                                                     //CF
			{
				if((cyclicEf.FileType&0x07) == LINTYPE)
				{
					recNumStart = G_ToolkitCurrentRecNum + step;  //search函数保证了 G_CurrentRecNum[G_ChannelNum] == fileHead.RecNumber||0x01时的错误返回
					if((recNumStart > cyclicEf.RecNumber) || (recNumStart == 0))  // add by xuWh@20230518  增加异常判断
					{
						UICCThrowUICCException(RECORD_NOT_FOUND);
					}
				}
				else
				{
					recNumStart = G_ToolkitCurrentRecNum;
					
					if(step == 0x01)
					{
						recNumStart = ((recNumStart == (cyclicEf.RecNumber))?0x01 : (recNumStart +1));	
					}
					else
					{
						recNumStart = ((recNumStart == 0x01)?(cyclicEf.RecNumber) : (recNumStart-1));
					}
				}
				
			}
			
		}
	}

	pattBuffer = GetArrayBufferAddress(patt) + pattOffset;

	backCurrentEFAddr = G_CurrentEfAddr[G_ChannelNum] ;
    
	G_CurrentEfAddr[G_ChannelNum] = G_ToolkitCurrentEfAddr;

	backSearch = RecData[1];
    
	RecData[1] = search[1]; 

	tag = SearchContaining(step,recNumStart,offset,(BYTE *)pattBuffer,pattLength,tag);

	G_CurrentEfAddr[G_ChannelNum] = backCurrentEFAddr;
    
	RecData[1] = backSearch;

	if(tag == 0x00)
	{
		PushOperandStackShort(0x00);
		return;	
	}

	G_ToolkitCurrentRecNum = tempBuf[0];

   	resBuffer = ((jshort*)GetArrayBufferAddress(response)) + respOffset; 

	tag = ((tag > respLength)?respLength:tag);

	for(i=0; i<tag; i++)
	{
		search[0] = 0x00;
        
		search[1] = tempBuf[i];
		
		VMWriteBuf((UWORD)(resBuffer+i),(BYTE *)search,0x02); //resBuffer是上层API传入，可能为RAM，可能为E2
	}	
	PushOperandStackShort(tag);
}
#endif
