#include "typedef.h"
#include "uicc_main.h"
#include "SIMToolkitConstant.h"
#include "gsmpublic.h"
#include "fileAction.h"
//#include "transaction.h"
#include "Interpreter.h"
//#include "VMAccessEeprom.h"
#include "filesystem.h"
#include "STKFileView.h"
#include "uicc_interface.h"
#include "ToolkitAPIPublic.h"
//#include "main.h"
//#include "ChannelManagement.h"
#include "platform_api.h"
#include "EUICCInterface.h"
#if UICC__FUNCTION__STKAPI
#include "SimAPi.h"
#include "UICCToolkitAPI.h"
#include "Supi.h"
#include "STKAlert.h"
#endif

#if UICC__FUNCTION__SIMAPI

//以下变量为解决SIMView的缺陷，对于每一个逻辑通道上都保存了一份G_ToolkitFileSystemMode等的备份，在使用的时候进行初始化，该方案不影响其他SIM Applet的上下文
BYTE   xdata 	G_SIMFileSystemMode[MAX_LOGICAL_CHANNELS];
BYTE   xdata    G_SIMCurrentRecNum[MAX_LOGICAL_CHANNELS];
WORD   xdata    G_SIMCurrentDfName[MAX_LOGICAL_CHANNELS];
WORD   xdata    G_SIMCurrentEfName[MAX_LOGICAL_CHANNELS];
UWORD  xdata    G_SIMCurrentDfAddr[MAX_LOGICAL_CHANNELS];
UWORD  xdata    G_SIMCurrentEfAddr[MAX_LOGICAL_CHANNELS];

/*
*@description:初始化通道的SIM API信息
*
@param       :channelId:0xFF，时，要初始化所有的通道SIMView信息
*@param      :initValue: 0x00：初始化为无效地址/ELSE:初始化为3F00等起始地址
*
*@return     :void
*
*@history
*/
void Toolkit_InitSIMView(BYTE channelId, BYTE initValue)
{
	BYTE idata startChannel,endChannel,i;

	if(channelId == 0xFF)
	{
		startChannel = 0;
        
		endChannel = MAX_LOGICAL_CHANNELS;
	}
	else
	{
		startChannel = channelId;
        
		endChannel = channelId+1;
	}

	for(i = startChannel; i < endChannel; i++)
	{
        G_SIMFileSystemMode[i] = SIMFileSystem;  //对于SIM API ,可访问所有的DF
        G_SIMCurrentEfName[i] = FALSE;
        G_SIMCurrentEfAddr[i] = FALSE;
        G_SIMCurrentRecNum[i] = FALSE;
    
        if(initValue == 0xFF)
        {
            G_SIMCurrentDfName[i] = DF_MF;
            G_SIMCurrentDfAddr[i] =	E_FILE_START;
        }
        else
        {
            G_SIMCurrentDfName[i] = FALSE;
            G_SIMCurrentDfAddr[i] =	FALSE;
        }	
	}
}
/*
*@description:将通道上的SIMView上下文拷贝到中间变量里，以进行后续处理
*
*@function   :Toolkit_SIMViewCopyFileContext
*
*@param      :void
*
*@return     :void
*
*@author     :xuyz
*
*@history	 :V1.0:创建源函数
*/
void  Toolkit_SIMViewCopyFileContext(void)
{
	G_ToolkitFileSystemMode = G_SIMFileSystemMode[G_CurSelectedChannel];

	G_ToolkitCurrentRecNum = G_SIMCurrentRecNum[G_CurSelectedChannel];

	G_ToolkitCurrentDfName = G_SIMCurrentDfName[G_CurSelectedChannel];

	G_ToolkitCurrentDfAddr = G_SIMCurrentDfAddr[G_CurSelectedChannel];
	
	G_ToolkitCurrentEfName = G_SIMCurrentEfName[G_CurSelectedChannel]; 

	G_ToolkitCurrentEfAddr = G_SIMCurrentEfAddr[G_CurSelectedChannel];
}
/*
*@desription:保存当前通道的SIMView FileContext,以支持多通道通过SIMAPI UICCAPI访问文件
*
*@function  : Toolkit_SIMViewRestoreFileContext
*
*@param     :void
*
*@return    :void
*
*@author    :xuyz
*
*@history   :V1.0:创建源函数
*/
void Toolkit_SIMViewRestoreFileContext(void)
{
	G_SIMFileSystemMode[G_CurSelectedChannel] = G_ToolkitFileSystemMode ;

	G_SIMCurrentRecNum[G_CurSelectedChannel] = G_ToolkitCurrentRecNum;

	G_SIMCurrentDfName[G_CurSelectedChannel] = G_ToolkitCurrentDfName;

	G_SIMCurrentDfAddr[G_CurSelectedChannel] = G_ToolkitCurrentDfAddr;
	
	G_SIMCurrentEfName[G_CurSelectedChannel] = G_ToolkitCurrentEfName; 

	G_SIMCurrentEfAddr[G_CurSelectedChannel] = G_ToolkitCurrentEfAddr;	
}
#define SELECT_PARA_4      0x01
#define SELECT_PARA_SHORT  0x02
void Toolkit_selectParaPublic(BYTE mode)
{
	BYTE idata responseLgth;
	UWORD xdata fileAddr/*,fcpData*/;
	BYTE  idata backCurrentSfi;
	jshort  idata fcpLength;
	jshort idata fcpOffset;
	jref idata fcp;
	jshort idata fid;
	jshort idata apiMode;
	
	fcp = REF_NULL;
	fcpOffset = 0x00;
	fcpLength = 0x00;
	
	apiMode = PopOperandStackShort();
	
	if(mode == SELECT_PARA_4)
	{
		fcpLength = PopOperandStackShort();
		fcpOffset = PopOperandStackShort();
		fcp = PopOperandStackShort();
	}
	
	fid = PopOperandStackShort();

	if(mode == SELECT_PARA_4)
	{
		check_bounds_interval_buffer(fcp,fcpOffset,fcpLength);
	}

	backCurrentSfi  = G_CurrentSfi;	

	fileAddr = ToolkitSelectByFileId(fid);
			
	G_CurrentSfi = 	backCurrentSfi;
	
	if(fileAddr == FALSE)	
	{	
		UiccOrSimViewThrowException(FILE_NOT_FOUND_SIM, apiMode);
		
		return;
	}

	if((apiMode == UICCAPI_Mode)&&(fid == 0x7FFF))
	{
		fid = G_ToolkitCurrentAdfName;
	}

   	if(IF_ProcessNoShareableFile(fileAddr,fid,0x01,apiMode) == FALSE) ////Api_1_Cont要求当选中的文件为非共享文件时，则抛出异常 FILE_NOT_FOUND
	{
		UiccOrSimViewThrowException(FILE_NOT_FOUND_SIM, apiMode);
		
		return;
	}

	if(mode == SELECT_PARA_4)
	{
		responseLgth = Toolkit_SetChannel(fileAddr,fid,TRUE, apiMode);

		responseLgth = (responseLgth<fcpLength)?responseLgth:fcpLength;

		if(apiMode == UICCAPI_Mode)
		{
			CopySrcDataToArray(tempBuf,fcp,fcpOffset,responseLgth);	
		}
		else
		{
			CopySrcDataToArray(tempBuf+100,fcp,fcpOffset,responseLgth);				
		}

		PushOperandStackShort(responseLgth);
	}
	else
	{
		Toolkit_SetChannel(fileAddr,fid,FALSE, apiMode);
	}
}
// ------------------------------- API methods ----------------------------
/**
* SELECT command as defined in GSM 11.11 standard.<br>
* By default, the MF is selected at the beginning of each applet activation
* (current file = MF). This method selects a file of the GSM file system.
* The file search starts at the current DF of the applet according to
* the file search method described in GSM 11.11 specification. The current
* DF or current EF and the current record pointer of the calling applet will
* be changed after successful execution.
*
* <p>
* Notes:<ul>
* <li><em>If </em><code>fciOffset</code><em> or </em><code>fciLength</code><em> parameter is negative an </em><code>ArrayIndexOutOfBoundsException</code>
* <em> exception is thrown and no read is performed.</em>
* <li><em>If </em><code>fciOffset+fciLength</code><em> is greater than </em><code>fci.length</code><em>, the length
* of the </em><code>fci</code><em> array an </em><code>ArrayIndexOutOfBoundsException</code><em> exception is thrown
* and no status request is performed.</em> 
* </ul> 
*
* @See   sim.impl.SIMAccess.select(short, byte[], short, short)
*
* @param fid is the File Identifier of the file to be selected.
* @param fci is the reference to the target byte array for FCI (File Control
*        Information) of current file, coding is according to GSM 11.11.
*        If <code>fci</code> is <code>null</code> the <code>NullPointerException</code> is thrown.
* @param fciOffset is the offset in the <code>fci</code> buffer for the response data.
* @param fciLength is the length of the required data in the <code>fci</code> byte array. 
*        If the <code>fciLength</code> is greater than the length of the response, the whole 
*	      response is copied into the <code>fci</code> buffer and the length of the response 
*        is returned by the method. If the <code>fcilength</code> is smaller than the length 
*	      of the response, the first part of the response is copied into the <code>fci</code>
*        buffer and the <code>fciLength</code> is returned by the method.
*
* @return length of the data which have been written in the <code>fci</code> buffer
*         (cannot be greater than <code>fciLength</code> parameter)
*
* @exception NullPointerException if <code>fci</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if operation would cause access of data outside 
*         array bounds.
* @exception SIMViewException in case of error<ul>
*          <li><code>FILE_NOT_FOUND</code>
*          <li><code>MEMORY_PROBLEM</code>
*          <li><code>INTERNAL_ERROR</code></ul>
*/
void Toolkit_SelectFCI(void)
{
	Toolkit_selectParaPublic(SELECT_PARA_4);
}
/**
* @See: sim.impl.SIMAccess.select(short fid)
*       uicc.access.FileView.select(short fid)
*
* SELECT command as defined in GSM 11.11 standard.<br>
* This SELECT method allows to update the current file without handling the
* Select Response.
* By default, the MF is selected at the beginning of each applet activation
* (current file = MF). This method selects a file of the common GSM file
* system. The file search starts at the current DF of the applet according
* to the file search method described in GSM 11.11 specification. The current
* DF or current EF and the current record pointer of the calling applet will
* be changed after successful execution.
*
* @param fid is the File Identifier of the file to be selected.
*
* @exception SIMViewException in case of error <ul>
*     		<li><code>FILE_NOT_FOUND</code>
*          <li><code>MEMORY_PROBLEM</code>
*     		<li><code>INTERNAL_ERROR</code></ul>
*/
void Toolkit_SelectById(void)
{
	Toolkit_selectParaPublic(SELECT_PARA_SHORT);
}
/**
* STATUS command as defined in GSM 11.11 standard.<br>
* This method returns the FCI (File Control Information) of the current DF
* (or MF) of the calling applet.
*
* <p>
* Notes:<ul>
* <li><em>If </em><code>fciOffset</code><em> or </em><code>fciLength</code><em> parameter is negative an </em><code>ArrayIndexOutOfBoundsException</code>
* <em> exception is thrown and no read is performed.</em>
* <li><em>If </em><code>fciOffset+fciLength</code><em> is greater than </em><code>fci.length</code><em>, the length
* of the </em><code>fci</code><em> array an </em><code>ArrayIndexOutOfBoundsException</code><em> exception is thrown
* and no status request is performed.</em> 
* </ul> 
*
* @See   sim.impl.SIMAccess.status(byte[], short, short)
* @param fci is the reference to the target byte array for FCI (File Control
*        Information) of current DF (or MF), coding is according to GSM 11.11.
*        If <code>fci</code> is <code>null</code> the <code>NullPointerException</code> is thrown.
* @param fciOffset is the offset in the <code>fci</code> buffer for the response data.
* @param fciLength is the length of the required data in the <code>fci</code> byte array.
*	      If the <code>fciLength</code> is greater than the length of the response, the whole 
*	      response is copied into the <code>fci</code> buffer and the length of the response 
*        is returned by the method. If the <code>fcilength</code> is smaller than the length 
*	      of the response, the first part of the response is copied into the <code>fci</code>
*        buffer and the <code>fciLength</code> is returned by the method.
*
* @return length of the data which have been written in the <code>fci</code> buffer
*         (cannot be greater than <code>fciLength</code> parameter)
*
* @exception NullPointerException if <code>fci</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if operation would cause access of data outside 
*         array bounds.
* @exception SIMViewException in case of error <ul>
*          <li><code>MEMORY_PROBLEM</code>
*      	<li><code>INTERNAL_ERROR</code></ul>
*/

void  Toolkit_Status(void)
{
	BYTE idata  totalLen;								//表示DF信息的总长度
	WORD idata backDFName;
	UWORD xdata backDFAddr;	
	jref idata arrayref; 
	jshort idata dataOffset; 
	jshort idata dataLength;
	jshort idata apiMode;
		
	apiMode = PopOperandStackShort();
	
	dataLength = PopOperandStackShort();
    
	dataOffset = PopOperandStackShort();
    
	arrayref = PopOperandStackShort();

	check_bounds_interval_buffer(arrayref, dataOffset, dataLength);	  //异常：ArrayIndexOutOfBoundsException

	backDFName = G_CurrentDfName[G_ChannelNum];	 

	backDFAddr = G_CurrentDfAddr[G_ChannelNum];

	G_CurrentDfName[G_ChannelNum] = G_ToolkitCurrentDfName;

	G_CurrentDfAddr[G_ChannelNum] = G_ToolkitCurrentDfAddr;
	
	if(apiMode == UICCAPI_Mode)
	{
		totalLen = BaseUsimGetDfInformation(tempBuf+100);
	}
	else
	{
		totalLen = GetDfInformation(tempBuf+100);   //此处要使用Toolkit目录
	}

	G_CurrentDfName[G_ChannelNum] = backDFName;

	G_CurrentDfAddr[G_ChannelNum] = backDFAddr;

 
	if(dataLength >=totalLen)
	{
		dataLength = totalLen;
	}
	
	CopySrcDataToArray(tempBuf+100,arrayref,dataOffset,dataLength);
 
	PushOperandStackShort(dataLength);  
}
/*@Function:  Toolkit_ReadUpdateBinaryPublic                                            
 * 
 *@Descrip:   读或者更新binary文件
 *
 *@See:	       
 *                                                                                                                                     
 *@Param:     mode ---> A_ReadFile or A_WriteFile    
 *
 *@Return:    void                                                                                                                                                                                                                                                   
 * 
 *@Author:    xwh
 *  
 *@History:   V1.0, copy by xwh 20180322  
 *
 *            V1.1, modify by xwh 20180809,拷贝文件不再需要分段多次拷贝，直接调用WriteEepromAtomic拷贝                      
 */
void Toolkit_ReadUpdateBinaryPublic(BYTE mode)
{
	BYTE idata result;
	jshort idata fileOffset; 
	jref idata arrayref; 
	jshort idata  respOffset; 
	jshort idata respLength;
    jshort idata apiMode;	
	UWORD xdata arrayDataAddr, fileDataAddr;
#if UICC__AUTH__5G__SUPI
	BYTE idata tmp[4],tmp1[4];//根据31.102规范4F0A文件大小只有4个字节，不存在大于TEMP_BUF_SIZE的情况
#endif	
	apiMode = PopOperandStackShort();
	
	respLength =PopOperandStackShort();
    
	respOffset =PopOperandStackShort();
    
	arrayref =PopOperandStackShort();
    
	fileOffset =PopOperandStackShort();

	arrayDataAddr = GetValidArrayDataAddr(arrayref,respOffset,respLength); //check_bounds_interval_buffer(arrayref, respOffset, respLength);
	
	result = Toolkit_ComputeBinary(mode,fileOffset,respLength, apiMode);

	if(result == FALSE)
	{
		if(mode == A_ReadFile)
		{
			PushOperandStackShort(FALSE);
		}
		return;
	}

	fileDataAddr = FindTFAddr(G_ToolkitCurrentEfAddr, fileOffset, respLength);

	if(mode == A_ReadFile)
	{	
		VMCopyDataToAddr(arrayDataAddr,0x00,fileDataAddr,respLength,MODE_PROTECT_WRITE);
		
		PushOperandStackShort(respOffset+respLength);
			
		return;
	}
	else // UpdateBinary
	{

#if UICC__AUTH__5G__SUPI	
		if((G_ToolkitCurrentDfName == DF_5GS)&&(G_ToolkitCurrentEfName ==EF_ROUTING_INDICATOR)) //API更新5FC0/4F0A的特殊处理
		{
			ReadTFBuf(G_ToolkitCurrentEfAddr, 0x00, (BYTE xdata *)&tmp[0], 0x04);
		}
#endif
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH		
		//当文件为强更新文件时，要用WriteTFBuf更新，才能保证更新到正确的地址,by xwh 20220623
		if(IF_IsHeFlashFile(G_ToolkitCurrentEfAddr))
		{
			VMReadBuf(tempBuf+0x100, arrayDataAddr, respLength);
			WriteTFBuf(G_ToolkitCurrentEfAddr, fileOffset, tempBuf+0x100, respLength);
		}
		else
#endif			
		{
			VMCopyDataToAddr(fileDataAddr,0x00,arrayDataAddr,respLength,MODE_PROTECT_WRITE);
		}

		if((apiMode == UICCAPI_Mode)&&(G_ToolkitCurrentEfName == 0x6F54)&&(G_ToolkitCurrentDfName == 0x7F10)) //UICC API:测试用例Cre_Pcs_Spco---CRRN3: During a CAT session the CAT Runtime Environment shall send a SET UP MENU system proactive command whenever the EFSUME file under the DFTELECOM file is updated as defined in TS 102 222 [7].
		{
			SetG_ToolkitSystemProactiveFlag(Bit_SetUpMenu);
		}
#if UICC__AUTH__5G__SUPI
		if((G_ToolkitCurrentDfName == DF_5GS)&&(G_ToolkitCurrentEfName ==EF_ROUTING_INDICATOR))
		{
			ReadTFBuf(G_ToolkitCurrentEfAddr, 0x00, (BYTE xdata *)&tmp1[0], 0x04);
			if((ecpMem_Compare(&tmp[0], &tmp1[0], 0x04) != EQUAL))
			{
				PendingMessageAlert(D_RefreshUpdateRoutingID,0);
			}
		}
#endif			
		return;
	}
}
/**
* READ BINARY command as defined in GSM 11.11 standard.<br>
* This method reads the data bytes of the current transparent EF of the
* calling applet.
*
* <p>
* Notes:<ul>
* <li><em>If </em><code>respOffset</code><em> or </em><code>respLength</code><em> parameter is negative a </em><code>ArrayIndexOutOfBoundsException</code>
* <em> exception is thrown and no read is performed.</em>
* <li><em>If </em><code>respOffset+respLength</code><em>is greater than </em><code>resp.length</code><em>, the length
* of the </em><code>resp</code><em> array a </em><code>ArrayIndexOutOfBoundsException</code><em> exception is thrown
* and no select is performed.</em> 
* </ul> 
*
* @See    sim.impl.SIMAccess.readBinary(short, byte[], short, short)
* @param fileOffset is the offset in the source transparent file.
* @param resp is the reference to the response byte array for read data.
* @param respOffset is the offset in the response byte array.
* @param respLength is the number of bytes to read.
*
* @return <code>respOffset+respLength</code>
*
* @exception NullPointerException if <code>resp</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if reading would cause access of data outside array bounds
* @exception SIMViewException in case of error <ul>
*      	<li><code>NO_EF_SELECTED</code>
*      	<li><code>FILE_INCONSISTENT</code>
*      	<li><code>AC_NOT_FULFILLED</code>
*      	<li><code>INVALIDATION_STATUS_CONTRADICTION</code>
*      	<li><code>OUT_OF_FILE_BOUNDARIES</code>
*          <li><code>MEMORY_PROBLEM</code>
*      	<li><code>INTERNAL_ERROR</code></ul>
*
* modify by xuxiaojun@20171115: 修改respLength的长度大于TEMP_BUF_SIZE时越界的BUG。E:\Java-ChinaMobile3GPPCos-Optimize\others\test\SIM Plus安全解决方案20171013\test_4_6
*/
void Toolkit_ReadBinary(void)
{
	Toolkit_ReadUpdateBinaryPublic(A_ReadFile);	
}

/**
* UPDATE BINARY command as defined in GSM 11.11 standard.<br>
* This method updates the data bytes of the current transparent EF of
* the calling applet.
*
* <p>
* Notes:<ul>
* <li><em>If </em><code>dataOffset</code><em> or </em><code>dataLength</code><em> parameter is negative a </em><code>ArrayIndexOutOfBoundsException</code>
* <em> exception is thrown and no update is performed.</em>
* <li><em>If </em><code>dataOffset+dataLength</code><em> is greater than </em><code>data.length</code><em>, the length
* of the </em><code>data</code><em> array a </em><code>ArrayIndexOutOfBoundsException</code><em> exception is thrown
* and no update is performed.</em> 
* </ul> 
* @See    sim.impl.SIMAccess.updateBinary(short, byte[], short, short)
* @param fileOffset is the offset in the destination transparent file.
* @param data is the reference to the source byte array for data to update.
* @param dataOffset is the offset in the source byte array.
* @param dataLength is the number of bytes to update.
*
* @exception NullPointerException if <code>data</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if updating would cause access of data outside array bounds
* @exception SIMViewException in case of error<ul>
*      	<li><code>NO_EF_SELECTED</code>
*      	<li><code>FILE_INCONSISTENT</code>
*      	<li><code>AC_NOT_FULFILLED</code>
*      	<li><code>INVALIDATION_STATUS_CONTRADICTION</code>
*      	<li><code>OUT_OF_FILE_BOUNDARIES</code>
*          <li><code>MEMORY_PROBLEM</code>
*      	<li><code>INTERNAL_ERROR</code></ul>
* 
* history:  modify by xuxiaojun@20171115: 修改dataLength的长度大于TEMP_BUF_SIZE时越界的BUG。E:\Java-ChinaMobile3GPPCos-Optimize\others\test\SIM Plus安全解决方案20171013\test_4_6
*/
void Toolkit_UpdateBinary(void)
{
	Toolkit_ReadUpdateBinaryPublic(A_UpdateFile);
}


/*@Function:    ToolkitSearch.  Toolkit_SeekCommand的子函数                                            
 * 
 *@Descrip:     在ToolkitApplet中查找记录
 *
 *@See:	        ts_143019v050600p.pdf    sim.impl.SIMAccess.seek(byte, byte[], short, short)
 *                                                                                                                                     
 *@Param:       recordNo   ---起始记录号
 *              maxRecnum  ---最大记录号
 *              step       ---步长
 *              dataBuff   ---目标buffer
 *              pattLength ---长度
 *				                                                                                                         
 *@Return:      recordNo/FALSE                                                                                                                                                                                                                                                              
 * 
 *@Author:      XUPEI 20130610
 *  
 *@History:     V0.0---Create by XUPEI 20130610           
 */
BYTE ToolkitSearch(BYTE recordNo,BYTE maxRecnum,signed step,BYTE *dataBuff,WORD pattLength)
{
	for(; (recordNo > 0) && (recordNo <= maxRecnum) ; recordNo = recordNo+step)
	{
		ReadLFBuf(G_ToolkitCurrentEfAddr, recordNo, 0, &tempBuf[0], pattLength);

		if(NvmCmp(&tempBuf[0],&dataBuff[0],pattLength) == EQUAL)
		{
			return recordNo;
		}
	}

	return FALSE;	
}

void Toolkit_ReadUpdateRecordPublic(BYTE rwMode)
{
	BYTE idata tmp,phyNum;
	BYTE idata tag,fileType,transActioinFlag;
	jshort idata recNumber;
	jbyte idata mode;
	jshort idata  recOffset;
	jref idata arrayref;
	jshort idata respOffset; 
	jshort idata respLength;
	jshort idata apiMode;
	//UWORD xdata arrayDataAddr,fileDataAddr;
		 
	apiMode = PopOperandStackShort();
	respLength = PopOperandStackShort(); 
	respOffset = PopOperandStackShort(); 
	arrayref = PopOperandStackShort();
	recOffset = PopOperandStackShort();
	mode = PopOperandStackShort();
	recNumber = PopOperandStackShort(); 
	
	//arrayDataAddr = GetValidArrayDataAddr(arrayref,respOffset,respLength); //check_bounds_interval_buffer(arrayref, respOffset, respLength);
	check_bounds_interval_buffer(arrayref, respOffset, respLength);
                                                            //send 
    tmp = Toolkit_computeRecord(recNumber, mode, recOffset, respLength, rwMode, apiMode);

	if(tmp == FALSE )
	{
		if(rwMode == A_ReadFile)
		{
			PushOperandStackShort(FALSE);
		}
		return;
	}
	
	phyNum =  GetPhyNum(rwMode,G_ToolkitCurrentEfAddr,mode,recNumber,&G_ToolkitCurrentRecNum);

	if(phyNum == FALSE)
	{
		UiccOrSimViewThrowException(RECORD_NUMBER_NOT_AVAILABLE_SIM, apiMode);
		return;
	}

//	fileDataAddr = FindLFAddr(G_ToolkitCurrentEfAddr, phyNum, recOffset, respLength);	
//	
//	if(fileDataAddr == FALSE)
//	{
//		if(rwMode == A_ReadFile)
//		{
//			PushOperandStackShort(FALSE);
//		}
//		return;
//	}
	
	if(rwMode == A_ReadFile)
	{
		//VMCopyDataToAddr(arrayDataAddr,0x00,fileDataAddr,respLength);
		ReadLFBuf(G_ToolkitCurrentEfAddr, phyNum,recOffset, tempBuf, respLength);
		CopySrcDataToArray(tempBuf,arrayref,respOffset,respLength);
		
		PushOperandStackShort(respOffset+respLength);
	}
	else
	{
		CopyArrayDataToDest(tempBuf,arrayref,respOffset,respLength);
		
		fileType = GetFileHeadByte(G_ToolkitCurrentEfAddr, F_Type_Offset);//获取文件类型

		transActioinFlag = FALSE;

		if((mode != REC_ACC_MODE_CURRENT) && ((fileType & 0x07) == CYCTYPE))//只有循环文件才会更新记录指针字节
		{
			transActioinFlag = IF_BeginTransaction();
		}
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
		G_UpdateCycleRecPointer = IF_UpdateCycleRecPointer(G_ToolkitCurrentEfAddr);	
#endif
		//tag = VMCopyDataToAddr(fileDataAddr,0x00,arrayDataAddr,respLength);
		tag = WriteLFBuf(G_ToolkitCurrentEfAddr,phyNum, recOffset, tempBuf, respLength);
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
		if(G_UpdateCycleRecPointer == FALSE)
#endif	
		{
			if((mode != REC_ACC_MODE_CURRENT) && ((fileType & 0x07) == CYCTYPE))//只有循环文件才会更新记录指针字节
			{
				VMSetBuf(getCycleFPointAddr(G_ToolkitCurrentEfAddr),phyNum,1);
			}
		}	
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
		G_UpdateCycleRecPointer = FALSE;
#endif		
		if(tag == FALSE)
		{
			IF_AbortTransaction(transActioinFlag);
			
			UiccOrSimViewThrowException(MEMORY_PROBLEM_SIM, apiMode);	//SIMViewEXCEPTION_MEMORY_PROBLEM
			return;
		} 
		
		IF_CommitTransaction(transActioinFlag);
	}
}
/**
* READ RECORD command as defined in GSM 11.11 standard.<br>
* This method reads the data bytes of the current linear fixed/cyclic EF
* of the calling applet. The current record pointer can be changed due to
* the choosen mode.
*     
* <p>
* Notes:<ul>
* <li><em>If </em><code>respOffset</code><em> or </em><code>respLength</code><em> parameter is negative a </em><code>ArrayIndexOutOfBoundsException</code>
* <em> exception is thrown and no read is performed.</em>
* <li><em>If </em><code>respOffset+respLength</code><em> is greater than </em><code>resp.length</code><em>, the length
* of the </em><code>resp</code><em> array a </em><code>ArrayIndexOutOfBoundsException</code><em> exception is thrown
* and no select is performed.</em> 
* </ul> 
*
* @param recNumber is the record number.
* @param mode is the mode for reading record, according to GSM 11.11.
*        If <code>mode</code> is <code>REC_ACC_MODE_NEXT</code> and the record pointer is at the last record 
*        the <code>RECORD_NUMBER_NOT_AVAILABLE SIMViewException</code> shall be thrown.
*        If <code>mode</code> is <code>REC_ACC_MODE_PREVIOUS</code> and the record pointer is at the first record,
*        the <code>RECORD_NUMBER_NOT_AVAILABLE SIMViewException</code> shall be thrown.
* @param recOffset is the offset in the record for the data to read.
* @param resp is the reference to the response byte array for read data.
* @param respOffset is the offset in the response byte array.
* @param respLength is the number of bytes to read.
*
* @return <code>respOffset+respLength</code>
*
* @exception NullPointerException if <code>resp</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if reading would cause access of data outside array bounds
* @exception SIMViewException in case of error <ul>
*      	<li><code>NO_EF_SELECTED</code>
*      	<li><code>FILE_INCONSISTENT</code>
*      	<li><code>AC_NOT_FULFILLED</code>
*      	<li><code>INVALIDATION_STATUS_CONTRADICTION</code>
*      	<li><code>OUT_OF_RECORD_BOUNDARIES</code>
*      	<li><code>RECORD_NUMBER_NOT_AVAILABLE</code>
*      	<li><code>INVALID_MODE</code>
*          <li><code>MEMORY_PROBLEM</code>
*      	<li><code>INTERNAL_ERROR</code></ul>
*/
void Toolkit_ReadRecord(void)
{
	Toolkit_ReadUpdateRecordPublic(A_ReadFile);
}
/**
* UPDATE RECORD command as defined in GSM 11.11 standard.<br>
* This method updates the data bytes of the current linear fixed/cyclic EF
* of the calling applet. The current record pointer can be changed due to
* the choosen mode.
*
* <p>
* Notes:<ul>
* <li><em>If </em><code>dataOffset</code><em> or </em><code>dataLength</code><em> parameter is negative a </em><code>ArrayIndexOutOfBoundsException</code>
* <em> exception is thrown and no select is performed.</em>
* <li><em>If </em><code>dataOffset+dataLength</code><em> is greater than </em><code>data.length</code><em>, the length
* of the </em><code>data</code><em> array a </em><code>ArrayIndexOutOfBoundsException</code><em> exception is thrown
* and no update is performed.</em> 
* </ul> 
*
* @param recNumber is the record number.
* @param mode is the mode for updating record, according to GSM 11.11.
*        If <code>mode</code> is <code>REC_ACC_MODE_NEXT</code> and the record pointer is at the last record 
*        the <code>RECORD_NUMBER_NOT_AVAILABLE SIMViewException</code> shall be thrown.
*        If <code>mode</code> is <code>REC_ACC_MODE_PREVIOUS</code> and the record pointer is at the first record,
*        the <code>RECORD_NUMBER_NOT_AVAILABLE SIMViewException</code> shall be thrown.
* @param recOffset is the offset in the record for the data to update.
* @param data is the reference to the source byte array for data to update.
* @param dataOffset is the offset in the source byte array.
* @param dataLength is the number of bytes to update.
*
* @exception NullPointerException if <code>data</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if updating would cause access of data outside array bounds
* @exception SIMViewException in case of error<ul>
*      	<li><code>NO_EF_SELECTED</code>
*      	<li><code>FILE_INCONSISTENT</code>
*      	<li><code>AC_NOT_FULFILLED</code>
*      	<li><code>INVALIDATION_STATUS_CONTRADICTION</code>
*      	<li><code>OUT_OF_RECORD_BOUNDARIES</code>
*      	<li><code>RECORD_NUMBER_NOT_AVAILABLE</code>
*      	<li><code>INVALID_MODE</code>
*          <li><code>MEMORY_PROBLEM</code>
*      	<li><code>INTERNAL_ERROR</code></ul>
*/

void Toolkit_UpdateRecord(void)
{
	Toolkit_ReadUpdateRecordPublic(A_UpdateFile);	
}

/**
* SEEK command as defined in GSM 11.11 standard.<br>
* This method seeks a pattern in the current linear fixed EF of the calling
* applet.
*
* <p>
* Notes:<ul>
* <li><em>If </em><code>pattOffset</code><em> or </em><code>pattLength</code><em> parameter is negative a </em><code>ArrayIndexOutOfBoundsException</code>
* <em> exception is thrown and no seek is performed.</em>
* <li><em>If </em><code>pattOffset+pattLength</code><em> is greater than </em><code>patt.length</code><em>, the length
* of the </em><code>patt</code><em> array a </em><code>ArrayIndexOutOfBoundsException</code><em> exception is thrown
* and no seek is performed.</em> 
* </ul> 
*
* @param mode is the seek mode, according to GSM 11.11 (no type information).
*        If <code>mode</code> is <code>SEEK_FROM_NEXT_FORWARD</code> and the record pointer is at the last record 
*        the <code>PATTERN_NOT_FOUND SIMViewException</code> shall be thrown.
*        If <code>mode</code> is <code>SEEK_FROM_PREVIOUS_BACKWARD</code> and the record pointer is at the first record,
*        the <code>PATTERN_NOT_FOUND SIMViewException</code> shall be thrown.
* @param patt is the reference to the byte array containing the seek pattern.
* @param pattOffset is the offset of the seek pattern in the byte array.
* @param pattLength is the length of the seek pattern.
*        If <code>pattLength</code> is greater than the current record size than 
*        the <code>OUT_OF_RECORD_BOUNDARIES SIMViewException</code> shall be thrown.
*        If <code>pattLength</code> is <code>zero</code> than <code>PATTERN_NOT_FOUND SIMViewException</code> shall be thrown.
*
* @return record number if pattern found
*
* @exception NullPointerException if <code>patt</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if seeking would cause access of data outside array bounds
* @exception SIMViewException in case of error<ul>
*      	<li><code>NO_EF_SELECTED</code>
*      	<li><code>PATTERN_NOT_FOUND</code>
*      	<li><code>FILE_INCONSISTENT</code>
*      	<li><code>AC_NOT_FULFILLED</code>
*      	<li><code>INVALIDATION_STATUS_CONTRADICTION</code>
*      	<li><code>INVALID_MODE</code>
*      	<li><code>OUT_OF_RECORD_BOUNDARIES</code>
*          <li><code>MEMORY_PROBLEM</code>
*      	<li><code>INTERNAL_ERROR</code></ul>
*/
void Toolkit_SeekCommand(void)
{
	BYTE idata theRecord;
	BYTE * dataBuff;
	RecordFileHead xdata fileHead;
	UWORD xdata actualfileAddr;
	BYTE idata status;
	
	jbyte idata mode; 
    
	jref idata arrayref; 
    
	jshort idata pattOffset; 
    
	jshort idata pattLength;
	
	 
	pattLength = PopOperandStackShort();
    
	pattOffset = PopOperandStackShort();
    
	arrayref = PopOperandStackShort();
    
	mode = PopOperandStackShort();

	dataBuff = (BYTE *)GetValidArrayDataAddr(arrayref, pattOffset, pattLength);//(GetArrayBufferAddress(arrayref) + pattOffset);					//数据起始地址

	if(G_ToolkitCurrentEfAddr == FALSE)
	{
		ThrowSIMViewException(NO_EF_SELECTED_SIM);	//SIMViewEXCEPTION_NO_EF_SELECTED
		
		return;
	}

	ReadEeprom((BYTE *)&fileHead,G_ToolkitCurrentEfAddr,sizeof(RecordFileHead));

	if(((fileHead.FileType&0x07) != LINTYPE)/*&&((fileHead.FileType&0x07) != CYCTYPE )*/) //SIMAPI中只允许线性文件做seek 测试用例 api1_svw_seekb_bbs
	{
		ThrowSIMViewException(FILE_INCONSISTENT_SIM);	//SIMViewEXCEPTION_FILE_INCONSISTENT
		
		return;
	}                                                          
    
	actualfileAddr  = SearchFile(G_ToolkitCurrentDfAddr, G_ToolkitCurrentEfName);

	status	= GetFileHeadByte(actualfileAddr,F_Status_Offset);
	
	if((status&0x01) == FILE_INVALIDATE)
	{
		ThrowSIMViewException(INVALIDATION_STATUS_CONTRADICTION_SIM);	//SIMViewEXCEPTION_INVALIDATION_STATUS_CONTRADICTION
		
		return;
	}    
	
	if(fileHead.RecLength < pattLength)   
	{
		ThrowSIMViewException(OUT_OF_RECORD_BOUNDARIES_SIM); //SIMViewEXCEPTION_OUT_OF_RECORD_BOUNDARIES
		
		return;
	}                                                                                                                  
 
	if(Toolkit_GetKeyStatus(G_ToolkitCurrentEfAddr,A_ReadFile,SIMAPI_Mode) == FALSE)
	{
		ThrowSIMViewException(AC_NOT_FULFILLED_SIM);	//SIMViewEXCEPTION_AC_NOT_FULFILLED
		
		return;
	}

	if(((mode<0)||(mode>3)) == TRUE)
	{
		ThrowSIMViewException(INVALID_MODE_SIM);
		
		return;
	}

	switch(mode)
	{
		case FRM_BGN_FRD :
		{ 
			theRecord = ToolkitSearch(1, fileHead.RecNumber,1,dataBuff,pattLength);
		
			break;
		}
		case FRM_LST_BFD : 
		{
			theRecord = ToolkitSearch(fileHead.RecNumber,fileHead.RecNumber, (-1),dataBuff,pattLength);
			
			break;
		}
		case FRM_CUR_FRD : 
		{
			if(G_ToolkitCurrentRecNum == 0x00)
			{
				theRecord = ToolkitSearch(0x01,fileHead.RecNumber,1,dataBuff,pattLength);				
			}
			else
			{
				theRecord = ToolkitSearch(G_ToolkitCurrentRecNum+0x1,fileHead.RecNumber,1,dataBuff,pattLength);
			}
		
			break;
		}
		default : 
		{
			if(G_ToolkitCurrentRecNum == 0x00)	
			{
				theRecord = ToolkitSearch(fileHead.RecNumber,fileHead.RecNumber, (-1),dataBuff,pattLength);
			}
			else
			{
				theRecord = ToolkitSearch(G_ToolkitCurrentRecNum-0x1,fileHead.RecNumber, (-1),dataBuff,pattLength);
			}
			
			break;
		}
	}	

	if(theRecord == FALSE)					//找不到相应记录，返回异常				
	{
		ThrowSIMViewException(PATTERN_NOT_FOUND_SIM);	//SIMViewEXCEPTION_PATTERN_NOT_FOUND	

		return;		
	}			

	G_ToolkitCurrentRecNum = theRecord;

	PushOperandStackShort((jshort)theRecord);			                                                 
}

/**
* INCREASE command as defined in GSM 11.11 standard.<br>
* This method increases the current cyclic EF record of the calling applet.
* The response buffer will only contain the value of the increased record.
*
* <p>
* Notes:<ul>
* <li><em>If </em><code>incrOffset</code><em> or </em><code>respOffset</code><em> parameter is negative an </em><code>ArrayIndexOutOfBoundsException</code>
* <em> exception is thrown and no increase is performed.</em>
* <li><em>If </em><code>incrOffset</code><em> is greater than </em><code>incr.length</code><em>, the length
* of the </em><code>incr</code><em> array an </em><code>ArrayIndexOutOfBoundsException</code><em> exception is thrown
* and no increase is performed.</em> 
* <li><em>If </em><code>respOffset</code><em> is greater than </em><code>resp.length</code><em>, the length
* of the </em><code>resp</code><em> array an </em><code>ArrayIndexOutOfBoundsException</code><em> exception is thrown
* and no increase is performed.</em> 
* </ul> 
*
* @param incr is the reference to the source byte array, containing the
*        value to add, on 3 bytes.
* @param incrOffset is the offset in the source byte array.
* @param resp is the reference to the response byte array for new record value.
*        If <code>resp</code> buffer is smaller than the record size, the <code>ArrayIndexOutOfBoundsException</code> is thrown.
*        If <code>resp</code> buffer is bigger than the record size, the <code>resp</code> buffer is filled
*        with the record value and left justified
* @param respOffset is the offset in the response byte array.
*
* @return length of the valid data in the <code>resp</code> buffer
*      (cannot be greater than the record size)
*
* @exception NullPointerException if <code>incr</code> or <code>resp</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if increasing would cause access of data outside array bounds
* @exception SIMViewException in case of error<ul>
*      	<li><code>NO_EF_SELECTED</code>
*      	<li><code>FILE_INCONSISTENT</code>
*      	<li><code>AC_NOT_FULFILLED</code>
*      	<li><code>INVALIDATION_STATUS_CONTRADICTION</code>
*      	<li><code>MAX_VALUE_REACHED</code>
*          <li><code>MEMORY_PROBLEM</code>
*      	<li><code>INTERNAL_ERROR</code></ul>
*/

void Toolkit_increase(void)
{
	RecordFileHead xdata cyclicEf;
	BYTE idata result,fistNum,secondNum,transActioinFlag;
	WORD idata totalValue;
	BYTE idata middleValue;
	jshort idata incrLength;
	jshort idata apiMode;
	
	jref idata incr; 
	jshort idata incrOffset; 
	jref idata resp; 
	jshort idata respOffset;
	UWORD xdata actualfileAddr;
	BYTE idata status,RecIndicator;
		
	apiMode = PopOperandStackShort();
	respOffset = PopOperandStackShort();
	resp = PopOperandStackShort(); 
	incrLength = PopOperandStackShort();
	incrOffset = PopOperandStackShort(); 
	incr = PopOperandStackShort(); 

	check_bounds_interval_buffer(incr, incrOffset, incrLength);	  //异常：ArrayIndexOutOfBoundsException

	check_bounds_interval_buffer(resp,respOffset,0);
	
	if(incrLength > 0x7F) 
	{		
		UiccOrSimViewThrowException(FILE_INCONSISTENT_SIM, apiMode);   //抛出异常 ArrayIndexOutOfBoundsException
	}

	if(G_ToolkitCurrentEfAddr == FALSE )
    {
		UiccOrSimViewThrowException(NO_EF_SELECTED_SIM, apiMode);	//SIMViewEXCEPTION_NO_EF_SELECTED
		
		return;
	}                                                     		//send 94 00

	ReadEEPromCache((BYTE idata *)&cyclicEf.FileId,G_ToolkitCurrentEfAddr,sizeof(RecordFileHead));
	
	RecIndicator = getCycleFPoint(G_ToolkitCurrentEfAddr);

	check_bounds_interval_buffer(resp, respOffset, cyclicEf.RecLength);	  //异常：ArrayIndexOutOfBoundsException

	if((cyclicEf.FileType&0x07) != CYCTYPE)					
    {								
		UiccOrSimViewThrowException(FILE_INCONSISTENT_SIM, apiMode);	//SIMViewEXCEPTION_FILE_INCONSISTENT
		
		return;
	}                                                     									

	actualfileAddr  = SearchFile(G_ToolkitCurrentDfAddr, G_ToolkitCurrentEfName);

	status	= GetFileHeadByte(actualfileAddr,F_Status_Offset);
	
  	if((status&0x01)==FILE_INVALIDATE)
  	{ 
		UiccOrSimViewThrowException(INVALIDATION_STATUS_CONTRADICTION_SIM, apiMode);	
		
		return;
	}    
	                 
    if(cyclicEf.RecNumber == 0)                          //this cyclic record is no record
	{
		UiccOrSimViewThrowException(RECORD_NUMBER_NOT_AVAILABLE_SIM, apiMode);           //抛出异常 RECORD_NOT_FOUND
		
		return;
	}
	
	if(Toolkit_GetKeyStatus(G_ToolkitCurrentEfAddr,A_IncreaseFile,apiMode) == FALSE)   //访问条件不满足
	{
		UiccOrSimViewThrowException(AC_NOT_FULFILLED_SIM, apiMode);	//SIMViewEXCEPTION_AC_NOT_FULFILLED
		
		return;
	}                                                          									

	ReadEEPromCache((BYTE xdata *)&tempBuf[0],getCycleFPointAddr(G_ToolkitCurrentEfAddr) + (RecIndicator-1)*cyclicEf.RecLength + 1,cyclicEf.RecLength);//读取当前记录指针所指记录值

	if(incrLength != 0x00)
	{
		CopyArrayDataToDest((BYTE xdata *)&tempBuf[0x100],incr,incrOffset,incrLength);

		fistNum = cyclicEf.RecLength<=incrLength?cyclicEf.RecLength:incrLength;

		secondNum = cyclicEf.RecLength>incrLength?cyclicEf.RecLength:incrLength;

		middleValue = 0x00;

		for(result = 1;result <= fistNum; result++)
		{   
			totalValue = middleValue + tempBuf[cyclicEf.RecLength-result] + tempBuf[0x100-result+incrLength] ;

			tempBuf[cyclicEf.RecLength-result] = 0xFF & totalValue;

			middleValue = (totalValue >> 8) & 0x01;
		}

		if(cyclicEf.RecLength<incrLength)										//record lgth < P3
		{
			for(result = fistNum;result < secondNum;result++) 
			{
				totalValue = middleValue  +  tempBuf[0x100+secondNum - 1 - result ] ;

				tempBuf[0x100+secondNum - 1 - result ] = 0xFF & totalValue;

				if(tempBuf[0x100+secondNum - 1 - result ]>0x00)				//max value reached
				{
					UiccOrSimViewThrowException(OUT_OF_RECORD_BOUNDARIES_SIM, apiMode); //SIMViewEXCEPTION_OUT_OF_RECORD_BOUNDARIES
					
					return;
				}

				middleValue = (totalValue >> 8) & 0x01;
			}
		}
		else															//record lgth >= P3
		{
			for(result = fistNum;result < secondNum;result++) 
			{
				totalValue = middleValue  +  tempBuf[secondNum - 1 - result ] ;

				tempBuf[secondNum - 1 - result ] = 0xFF & totalValue;

				middleValue = (totalValue >> 8) & 0x01;
			}
		} 
	 
		if(middleValue == 0x01)
		{												
			UiccOrSimViewThrowException(MAX_VALUE_REACHED_SIM, apiMode);
			
			return;
		} 

		CopySrcDataToArray((BYTE xdata *)&tempBuf[0x00],resp,respOffset,cyclicEf.RecLength);
	}

	if(RecIndicator == 1)
	{
		RecIndicator = cyclicEf.RecNumber;
	}
	else
	{
		RecIndicator = RecIndicator - 1;
	}

	G_ToolkitCurrentRecNum = 1;							//Add at 2007-11-14
	
	transActioinFlag = IF_BeginTransaction();
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	G_UpdateCycleRecPointer = IF_UpdateCycleRecPointer(G_ToolkitCurrentEfAddr);	
#endif	
	result = WriteLFBuf(G_ToolkitCurrentEfAddr,RecIndicator,0x00,(BYTE xdata *)&tempBuf[0],cyclicEf.RecLength);
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	if(G_UpdateCycleRecPointer == FALSE)
#endif
	{	
		VMSetBuf(getCycleFPointAddr(G_ToolkitCurrentEfAddr),RecIndicator,1);
	}	
	if(result == FALSE)
	{
		IF_AbortTransaction(transActioinFlag);
		
		UiccOrSimViewThrowException(MEMORY_PROBLEM_SIM, apiMode);         //抛出异常MEMORY_PROBLEM
		
		return;
	}
	
	IF_CommitTransaction(transActioinFlag);
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	G_UpdateCycleRecPointer = FALSE;
#endif
	PushOperandStackShort(cyclicEf.RecLength);
}
/*
*@description:  处理SIMToolkit的文件状态公共函数
*
*@function   :  ToolkitFileStatusHandlerPublic
*
*@param      : BYTE actionMode
*@param      : BYTE Status
*
*@return      :void
*
*@author      : create by xuyz
*/
void ToolkitFileStatusHandlerPublic(BYTE actionMode, BYTE fileStatus)
{
	BYTE idata status,writeTag;
	UWORD xdata actualFileAddr;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();

	if(G_ToolkitCurrentEfAddr == FALSE)
	{
		UiccOrSimViewThrowException(NO_EF_SELECTED_SIM, apiMode);	//SIMViewEXCEPTION_NO_EF_SELECTED
		
		return;
	}

	if(Toolkit_GetKeyStatus(G_ToolkitCurrentEfAddr,actionMode,apiMode) == FALSE)
	{
		UiccOrSimViewThrowException(AC_NOT_FULFILLED_SIM, apiMode);	//SIMViewEXCEPTION_AC_NOT_FULFILLED
		
		return;
	}

	actualFileAddr = SearchFile(G_ToolkitCurrentDfAddr, G_ToolkitCurrentEfName);	//获取文件的实际地址，防止文件时映射文件，而修改到了映射文件的状态
	
	status = GetFileHeadByte(actualFileAddr, F_Status_Offset);

	if((status&0x01) == fileStatus)
	{
		if(apiMode == SIMAPI_Mode)
		{
			UiccOrSimViewThrowException(INVALIDATION_STATUS_CONTRADICTION_SIM, apiMode);	//SIMViewEXCEPTION_INVALIDATION_STATUS_CONTRADICTION
		}
		
		return; //uicc api时 根据java api描述 * No exception shall be thrown when deactivating an already deactivated EF. 
	} 
    
	if(actionMode == A_DeactivateFile)
	{
		writeTag = SetEepromAtomic(actualFileAddr+F_Status_Offset,status&0xFE,sizeof(BYTE));  // 确认 事务是否已经开启 
	}
	else // A_ActivateFile
	{
		writeTag = SetEepromAtomic(actualFileAddr+F_Status_Offset,status|0x01,sizeof(BYTE));
	}
        
	if(writeTag == FALSE)
	{
		UiccOrSimViewThrowException(MEMORY_PROBLEM_SIM, apiMode);	//SIMViewEXCEPTION_MEMORY_PROBLEM
		
		return;
	}	
}
/**
* INVALIDATE command as defined in GSM 11.11 standard.<br>
* This method invalidates the currently selected EF of the calling applet.
*
* @exception SIMViewException in case of error<ul>
*      	<li><code>NO_EF_SELECTED</code>
*      	<li><code>AC_NOT_FULFILLED</code>
*      	<li><code>INVALIDATION_STATUS_CONTRADICTION</code>
*          <li><code>MEMORY_PROBLEM</code>
*      	<li><code>INTERNAL_ERROR</code></ul>
*/
void Toolkit_Invalidate(void)
{
	ToolkitFileStatusHandlerPublic(A_DeactivateFile, FILE_INVALIDATE);		
}

/**
* REHABILITATE command as defined in GSM 11.11 standard.<br>
* This method rehabilitates the currently selected EF of the calling applet.
*
* @exception SIMViewException in case of error <ul>
*      	<li><code>NO_EF_SELECTED</code>
*      	<li><code>AC_NOT_FULFILLED</code>
*      	<li><code>INVALIDATION_STATUS_CONTRADICTION</code>
*          <li><code>MEMORY_PROBLEM</code>
*      	<li><code>INTERNAL_ERROR</code></ul>
*/
void Toolkit_Rehabilitate(void)
{
	ToolkitFileStatusHandlerPublic(A_ActivateFile, FILE_REINVALIDATE);		
}

#endif



