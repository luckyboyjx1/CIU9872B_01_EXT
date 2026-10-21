#include "typedef.h"
#include "uicc_main.h"
#include "Gsmpublic.h"
#include "fileAction.h"
#include "fileSystem.h"
#include "profileInterpreter.h"
#include "Interpreter.h"
#include "Interface_LTEFromPlatform.h"
#include "Interface_PlatformFromLTE.h"
#include "EUICCCustomerPublic.h"
#include "ProfileGPCommand.h"
#include "EUICCMain.h"
#include "profileApi.h"
#include "cardManager.h"
#include "Stkhead.h"
#include "Platform_api.h"
#include "FreeTable.h"
#include "ProfileInterpreter.h"
#include "cdma.h"		//remove warning,dengyiwen,2022/9/27


#if (EUICC_ONLY||SE_AND_EUICC)
BYTE  xdata G_DownLoadingProfileIndex; //当前下载的Profile的index

/*
*@funtion    :checkPINPUKMaxRemain
*
*@description: 判断pin和puk的max，remain值是否符合条件
*
*@param      :BYTE maxRetry
*             
*@return     :TRUE/FALSE
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20190719
*/
BYTE checkPINPUKMaxRemain(BYTE maxRemain)
{
	BYTE idata maxRetry,remainRetry;
	
	maxRetry = 0;
	remainRetry = 0;
	
	maxRetry = (maxRemain >> 4) & 0x0F;
	remainRetry = maxRemain & 0x0F;
	
	if(maxRetry < remainRetry)
	{
		return FALSE;
	}
	
	return TRUE;
}



/*
*@funtion    :createSpecProfilePrivateFile
*
*@description: 根据私有文件的模版，创建特定的私有文件
*
*@param      :void
*             
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zhxx@20190429
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE createSpecProfilePrivateFile(BYTE localPin,BYTE num,WORD adfId)
{	
	BYTE idata result,adfTag,pos,dataLen;
	UWORD xdata fileAddr,backFileAddr,firstLevelDF;
	WORD idata fileId;
	TlvInfo xdata tempInfo;
	
	fileAddr = FALSE;
	
	adfTag = 0;
	
	if(localPin == FALSE)
	{
		if(PESearchAIDName((BYTE *)&USIMPartial[0]) == TRUE)//ADF USIM
		{
			adfTag = 1;
		}
		else if(PESearchAIDName((BYTE *)&ISIMPartial[0]) == TRUE)//ADF ISIM
		{
			adfTag = 2;
		}
		else if(PESearchAIDName((BYTE *)&CSIMPartial[0]) == TRUE)//ADF CSIM
		{
			adfTag = 3;
		}
		else //如果是普通的ADF，不创建私有文件
		{
			return TRUE;
		}
	}//local pin为TRUE时，ADF可以是任意的
	else
	{
		firstLevelDF = GetFirstLevelDf(G_fileControlContext.LoadDFAddr);
	}
	
	fileAddr = SelectPrivateTFFile(DFProfileId,SpecPrivateTemplateFileId);	//选择特定的私有文件模板

    if(fileAddr == FALSE)
	{
		return FALSE;
	}	

	initTlvInfo(&tempInfo,fileAddr+sizeof(TranFileHead),GetTFSize(fileAddr));//bufferAddr直接指向模版文件的文件体地址		

	initFindTlv(&tempInfo);
	
	backFileAddr = G_fileControlContext.LoadDFAddr;
	G_fileControlContext.LoadDFAddr = (localPin == TRUE) ? firstLevelDF : G_fileControlContext.LoadADFAddr;


	do{
		result = findTLV(&tempInfo,0xFF,0xFF,1);
		
		if(result == FALSE) 
		{
			break;
		}
		
		VMReadBuf(tempBuf,tempInfo.bufferAddr+tempInfo.findValueOff,tempInfo.findValueLen);
		
		dataLen = tempInfo.findValueLen;
		
		if((((tempInfo.tagBuffer[0] == 0xA0) || //所有的ADF下面都需要创建
		  ((tempInfo.tagBuffer[0] == 0xA1) && ((adfTag == 1) || (adfTag == 2))) || //USIM和ISIM下面需要创建
		  ((tempInfo.tagBuffer[0] == 0xA2) && (adfTag == 1)) || //只有USIM下面需要创建
		  ((tempInfo.tagBuffer[0] == 0xA3) && (adfTag == 3))) && (localPin == 0)) ||//只有CSIM下面需要创建
		  ((tempInfo.tagBuffer[0] == 0xA5) && (localPin == 1))) //只有local下面需要创建
		{
            if(num != 0)
			{
				SetWord(tempBuf+tempInfo.findValueLen,0x8f04);
				SetWord(tempBuf+tempInfo.findValueLen+2,adfId);
				
				pos = FindTag(tempBuf,0x83,tempInfo.findValueLen);
				
				if(pos != FALSE)
				{
					fileId = BuildWord(&tempBuf[pos+1]);
					dataLen += 6;
					if(((fileId == EF_KI) || (fileId == EF_AuthMaxCounter)) || ((num >= 3) && (fileId == EF_PRIVATE_AUTHCFG)) || ((num >= 4) && (fileId == EF_SQNCheck)) || ((num == 1) && (fileId == EF_PRIVATE_PIN)))
					{
					   SetWord(tempBuf+tempInfo.findValueLen+4,fileId);
					}
					else
					{
						dataLen -= 6;
					}
				}
								
			}	
			
	        result = profileCreateFile(tempBuf,dataLen,PROFILE_MODE);//profile模式创建，eSIM_Test_additional/During_Load_Profile_break_in_update_file_Test_01
						
			if(processFileResult(result) == FALSE)
			{
				return FALSE;
			}			
		}
		
		if((G_fileControlContext.LoadEFName == EF_PRIVATE_AUTHCFG) && (tempInfo.tagBuffer[0] == 0xD6) && (localPin == FALSE))//更新二进制文件
		{
			profileUpdateBinary(BuildWord(&tempBuf[0]),tempBuf+2,tempInfo.findValueLen-2);
		}		
		
	}while(result);	

	G_fileControlContext.LoadDFAddr = backFileAddr;

   return TRUE;	
}


/*
*@funtion    :createPrivateFile
*
*@description: 根据私有文件的模版，创建私有文件
*
*@param      :void
*             
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE createProfilePrivateFile()
{
	UWORD xdata fileAddr;
	BYTE idata result,fileResult;
	
	fileAddr = SelectPrivateTFFile(DFProfileId,PrivateTemplateFileId);//选择私有文件模版
	
	if(fileAddr == FALSE) //预置ISD_P通过APDU指令创建私有文件
	{
		return 0xFE; 
	}
	
	initTlvInfo(&G_TemplateTlvInfo,fileAddr+sizeof(TranFileHead),GetFileHeadWord(fileAddr, F_SpaceSize_Offset));
	
	initFindTlv(&G_TemplateTlvInfo);
	
	G_fileControlContext.CurrentFileFileRecLen = 0;
	
	do{
		result = findTLV(&G_TemplateTlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE) 
		{
			return 0xFE;
		}
		
		VMReadBuf(tempBuf,G_TemplateTlvInfo.bufferAddr+G_TemplateTlvInfo.findValueOff,G_TemplateTlvInfo.findValueLen);
		
		if(G_TemplateTlvInfo.tagBuffer[0] == 0xD6)  //更新二进制文件
		{
			profileUpdateBinary(BuildWord(&tempBuf[0]),tempBuf+2,G_TemplateTlvInfo.findValueLen-2);
		}
		else if(G_TemplateTlvInfo.tagBuffer[0] == 0xA4)//根据路径选择文件
		{
			profileSelectByPath(tempBuf,G_TemplateTlvInfo.findValueLen);
		}
		else if(G_TemplateTlvInfo.tagBuffer[0] == 0xDC)  //更新记录文件
		{
			profileUpdateRecord(tempBuf[0],tempBuf[1],tempBuf+2,G_TemplateTlvInfo.findValueLen-2);
		}
		else if(G_TemplateTlvInfo.tagBuffer[0] == 0x62)//创建文件
		{
			fileResult = profileCreateFile(tempBuf,G_TemplateTlvInfo.findValueLen,PROFILE_MODE);//eSIM_Test_additional/During_Load_Profile_break_in_update_file_Test_01
			
			if(processFileResult(fileResult) == FALSE)
			{
				return FALSE;
			}
		}
		else
		{
			return 0xFE;
		}
		
	}while(TRUE);
}
/********************************new function for uiccapi and UICC APDU*********************************************************************/

/*
 *@description:创建 Profile MF
 *
 *@param:      currentDfAddr :当前DF地址
 *@param:      fileName :     FID
 *@param       buf    :FCP
 *@param       len    :FCP  长度
 *
 *@Returns     the need file size of creating A DF file/E_Parameter - error return
 *
 *@history     create by xyz at 090721 16:29
 			   modify by zzj at 131214	,本次修改主要为为了支持UICC API中的创建文件
*/
BYTE profileCreateMF(BYTE *fcp,BYTE fcpLen)
{
	BYTE  idata result;
	UWORD xdata profileOffset;
	FileSystemHead xdata  mfFileHead;
	BYTE  idata i,j;
	
	profileOffset = GetMaxProfileMFEnd();
	
	//私有文件区没有创建，报错
	//默认文件区没有创建，报错
	//地址已到达应用区，报错
#if RSP_EUICC
	if((GetIfMfCreated(MODE_PRIVATE) == INVALID_ADDR)||(GetIfMfCreated(MODE_DEFAULT) == INVALID_ADDR)||(AllocSpaceEnough(sizeof(FileSystemHead)) == FALSE))
	{
		return  E_FileSize;
	}
#else
	if((GetIfMfCreated(MODE_PRIVATE) == INVALID_ADDR)||(AllocSpaceEnough(sizeof(FileSystemHead)) == FALSE))
	{
		return  E_FileSize;
	}
#endif
	
	ecpMem_Set((BYTE*)&mfFileHead, 0x00, sizeof(mfFileHead));//初始化FillByte，避免出现随机值导致CRC不一致情况,by chentao@20240415  //bug531
	
	mfFileHead.FileId = DF_MF;

	mfFileHead.FileType = 0x78 ;							//MF as sharable

	result = FindTag(fcp,0x8B,fcpLen);									//Referenced mode

	if(result != FALSE)
	{
		if(fcp[result]==0x03)
		{
			mfFileHead.ArrNum = fcp[result+3];
		}
		else
		{
			mfFileHead.ArrNum = fcp[result+1];
		}		
	}
	else
	{
//		mfFileHead.ArrNum = IF_SetMFArrNum();								//MF Arr Num ;因UICC API测试用例要求所有的DF权限都是ALWAYS，测试脚本Api_1_Fvw_Incr case16 pdf描述的是文件创建于DFtest下，实际我们的文件创建在了MF下，此处需确认,此处将UICC_API 批处理中的的第7条记录权限改成ALWAYS,实际发卡时不准这样修改，
		return E_Parameter;
	}
	
	mfFileHead.ArrName = F_ARRNAME_2F;  //modify @20140727  bit7为0表示当前的arrName为2F，bit0-bit6
	
	mfFileHead.DIRNum = 0x00;
	
	//mfFileHead.MaxSize = GetHeapBase() - profileOffset; //将所有空间分配给MF，PE end后再设置成实际大小
	
	//if(mfFileHead.MaxSize <= sizeof(FileSystemHead))                 //重复判断
	//{
	//	return  E_FileSize;
	//}
	
	mfFileHead.LocalPinIndex = 1; //bug454 added by zhangxiaoxiao@20221202： 创建3F00时，3f00头，这个变量没赋初值
	
	mfFileHead.MaxSize = sizeof(FileSystemHead);                     //MaxSize与UsedSize相等

	mfFileHead.UsedSize = sizeof(FileSystemHead);

	mfFileHead.ChildDfNum = 0x01;

	mfFileHead.ChildEfNum = 0x00;

	mfFileHead.ParentName[0] = DF_MF;

	mfFileHead.ParentName[1] = DF_MF;
	
	result = FindTag(fcp,0xC6,fcpLen);

	if(result != FALSE)
	{		
		for(j=0x01;j<fcp[result];j = j+3) // modify by chentao@20240530 83 tag 只在C6范围内找，否则在83不存在的情况会死循环   例： 00E000003A 6238 82027821 83027F60 840FA0000000871002FF47F001890000108A0105AB0F80015F90008001209700840188970081020000 C6059000950108
		{
			if(fcp[result+j] == 0x83) //找到第一个83退出
			{
				break;
			}
		}

		for(i=0x00;i<0x04;i++)
		{
			if((j+(i*3))< fcp[result])
			{
				mfFileHead.ChvStatus[i] = fcp[result+j+(i*3)+2];
			}
			else
			{
			   mfFileHead.ChvStatus[i] = 0xFF;
			}
		}
	}
	else
	{
		mfFileHead.ChvStatus[0] = 0x01;
			
		mfFileHead.ChvStatus[1] = 0x02;//MF下面只有global pin

		mfFileHead.ChvStatus[2] = 0x0A;
		
		mfFileHead.ChvStatus[3] = 0x0B;
	}
	
	mfFileHead.Rfu = 0xFF;

	mfFileHead.KeyLmit = 0x0A;

	result = FindTag(&fcp[0],0x8E,fcpLen);                             //DF key

	if (result != FALSE)
	{
		ecpMem_Copy((BYTE *)&mfFileHead.MF_Key[0], &fcp[result+1], 0x08);
	}
	else
	{
		ecpMem_Set((BYTE *)&mfFileHead.MF_Key[0], 0xFF, 0x08);
	}
	
	result = WritePageCacheEX(profileOffset,(BYTE *)&mfFileHead.FileId,sizeof(FileSystemHead));          //write mf head to the file system start of eeprom 

	if(result == FALSE)
	{
		return FALSE;
	}
	
	G_CreatingMFOffset = profileOffset;
	
	SetProfileChannel(profileOffset,DF_MF);	

	//将当前正在下载的Profile 的mfOffset 写入PROFILE_MFOFFSET_CFG 
	SetDownLoadingProfileMFOffset(G_CreatingMFOffset);
	
	return 0xFE;
}


/*@Function:    profileModifyMFInfo                                           
 * 
 *@Descrip:     profile解析调用的Resize File API
 *
 *@See:	        ts_102222
 *                                                                                                                                     
 *@Param:       fcp      ---> FCP template,结构参考TS102222
 *@param        fcpLen   ---> FCP template length
 *				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                            
 * 
 *@Author:      XWH 20170616
 *  
 *@History:     V1.0---Create by XWH 20170616     
 *              V1.1---Modify by XWH 20170731,增加对8B 和C6 tag的处理
 *              V1.2---modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
 */ 
BYTE code MFNotSupportTag[] ={0xA5,0x88,0x80,0x84};
BYTE code MFNeedSupportTag[] ={0xC6,0x8B,0x82,0x83};
BYTE profileModifyMFInfo(BYTE * fcp,WORD fcpLen)
{
	BYTE  idata result;
	BYTE  idata i,j;
	FileSystemHead xdata  mfFileHead;

	VMReadBuf((BYTE *)&mfFileHead.FileId,G_CreatingMFOffset,sizeof(FileSystemHead));   
	if(ValueBit(G_profileControlContext.IsCreateFlag,CREATE_MF) == TRUE) //重复创建3F00 testcase01-3
	{
		return FALSE;
	}

	for(i=0x00;i<sizeof(MFNotSupportTag);i++)
	{
		result = FindTag(fcp,MFNotSupportTag[i],fcpLen);
		if((ADF_SUPPORT_SFI) && (MFNotSupportTag[i] == 0x88)) // add by chentao@20220610 兼容德国profile下有88tag
		{
			continue;
		}

		if(result != FALSE)
		{
			return FALSE;
		}
	}

	for(i=0x00;i<sizeof(MFNeedSupportTag);i++)
	{
		result = FindTag(fcp,MFNeedSupportTag[i],fcpLen);
		if(result == FALSE)
		{
			return FALSE;
		}
	}

	result = FindTag(fcp,0x8B,fcpLen);									//Referenced mode

	if(result != FALSE)
	{
		if(fcp[result]==0x03)
		{
			mfFileHead.ArrNum = fcp[result+3];
		}
		else
		{
			mfFileHead.ArrNum = fcp[result+1];
		}		
	}


	result = FindTag(fcp,0xC6,fcpLen);

	if(result != FALSE)
	{		
		for(j=0x01;j<fcp[result] ;j = j+3) // modify by chentao@20240530 83 tag 只在C6范围内找，否则在83不存在的情况会死循环   例： 00E000003A 6238 82027821 83027F60 840FA0000000871002FF47F001890000108A0105AB0F80015F90008001209700840188970081020000 C6059000950108
		{
			if(fcp[result+j] == 0x83) //找到第一个83退出
			{
				break;
			}
		}

		for(i=0x00;i<0x04;i++)
		{
			if((j+(i*3))< fcp[result])
			{
				mfFileHead.ChvStatus[i] = fcp[result+j+(i*3)+2];
			}
			else
			{
			   mfFileHead.ChvStatus[i] = 0xFF;
			}
		}
	}

	result = FindTag(fcp,0x83,fcpLen);
	if(result != FALSE)
	{
		if(fcp[result] != 0x02 )
		{
			return FALSE;
		}
	}

	result = FindTag(fcp,0x82,fcpLen);
	if(result != FALSE)
	{
		if((fcp[result] != 0x02) && (fcp[result] != 0x04))
		{
			return FALSE;
		}
	}
	WritePageCacheEX(G_CreatingMFOffset,(BYTE *)&mfFileHead.FileId,sizeof(FileSystemHead)); 

	SetProfileChannel(G_CreatingMFOffset,DF_MF);	
	return TRUE;
}


/*@Function:    profileCreateFile                                           
 * 
 *@Descrip:     profile解析调用的Create File API
 *
 *@See:	        ts_102222
 *                                                                                                                                     
 *@Param:       fcp      ---> FCP template,结构参考TS102222
 *@param        fcpLen   ---> FCP template length
 *				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                            
 * 
 *@Author:      XWH 20170616
 *  
 *@History:     V1.0---Create by XWH 20170616    
 *              V1.1---modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP         
 */ 
BYTE  profileCreateFile(BYTE * fcp,WORD fcpLen,BYTE type)
{
	BYTE  idata pos;
	BYTE  idata filetype;
	WORD  idata fileName;
	UWORD idata fileAddr,fileMaxSize,UsedSize;
	
	if(fcpLen == 0)  
	{
		return 0xFE;	
	}
	
	pos = FindTag(fcp,0x83,fcpLen);              //File ID
	
	if((pos == FALSE) || (fcp[pos] != 0x02))
	{
		return E_Parameter;
	}

	fileName = ByteToWord(fcp[pos+1],fcp[pos+2]);
	
	//if(fileName == 0x3F00)                       //File ID = 3F00 表示这是一个Profile创建文件的开始
	//{
	//	 return profileCreateMF(fcp,fcpLen);
		
	//}
	
#if RSP_EUICC
	if(fileName == EF_CLASSES) //EFENV-CLASSES SHALL never be present in any Profile Package, however if present, it SHALL be ignored by the eUICC when installing the Profile.
	{
		G_fileControlContext.LoadEFName = fileName;
		return 0xFE;
	}
	
	pos = FindTag(fcp,0x84,fcpLen);              //获取USIM或者CSIM的PE_identification，用于PEEnd时mccmnc检查不匹配时使用

	if(pos != FALSE)
	{
		if(ecpMem_Compare(&USIMPartial[1],&fcp[pos+1],USIMPartial[0]) == 0x00)
		{
			G_USIM_identification = G_PEControlContext.PE_identification;
		}
		else if(ecpMem_Compare(&CSIMPartial[1],&fcp[pos+1],CSIMPartial[0]) == 0x00)
		{
			G_CSIM_identification = G_PEControlContext.PE_identification;
		}
		else
		{

		}
	}
#endif

	fileAddr = SearchFile(G_fileControlContext.LoadDFAddr,fileName);

	if(fileAddr != FALSE)
	{
		SetProfileChannel(fileAddr,fileName);
		//return FALSE;
		return 0xFE;  //返回值暂定，无任何实质意义
	}

	if((PROFILE_MODE_GENERIC == type) || (PROFILE_MODE_TEMPLATE == type))
	{
		if(ValueBit(G_profileControlContext.IsCreateFlag,CREATE_MF) == FALSE) //error：第一个创建的不是3F00 HB TestCase01_9
		{
			return E_InvalidRequest;
		}
	}

	pos = FindTag(fcp,0x82,fcpLen);              //File Type
	
	if(pos == FALSE)
	{
		return E_Parameter;
	}
	
	if( (fcp[pos] != 0x02) && (fcp[pos] != 0x04))
	{
		return E_Parameter;
	}

	/*if(((fcp[pos+1] & 0xF0) != 0x70) && ((fcp[pos+1] & 0xF0) != 0x40) && ((fcp[pos+1] & 0xF0) != 0x00))
	{
		return E_Parameter;
	}*/

	if(((fcp[pos+1] & FILETYPE) != ADFTYPE) && ((fcp[pos+1] & FILETYPE) != 0x39) && ((fcp[pos+1] & 0x37) != TRANTYPE)
				&& ((fcp[pos+1] & 0x37) != LINTYPE) && ((fcp[pos+1] & 0x37) != CYCTYPE))  // modify by chentao @20230114 合V2.1代码   log_2022_12_29_明文profile    7F40文件
	{
		return E_Parameter;
	}

	
	filetype = fcp[pos+1]&FILETYPE;

  UsedSize = GetFileHeadUsedSize(G_fileControlContext.LoadDFAddr);

	fileAddr = G_fileControlContext.LoadDFAddr+UsedSize;							//back for set directory

	if(filetype== ADFTYPE)										//create df
	{
		if(fileName == 0xFFFF)
		{
			return E_Parameter;
		}
		fileMaxSize = commonCreateDFFile(G_fileControlContext.LoadDFAddr,fileName,fcp,fcpLen,type);
		ClearBit(G_profileControlContext.IsCreateFlag,CREATE_LocalPIN);
	}
	else if((filetype== TRANTYPE) || (filetype== LINTYPE) || (filetype== CYCTYPE) || (filetype == 0x39))
	{
		fileMaxSize = commonCreateEFFile(G_fileControlContext.LoadDFAddr,fileName,fcp,fcpLen,type);
	}
	else//文件类型错误，需要抛错，HB_Offline_Test/14-PROFILE PE数据异常测试/Profile_FulSeg004f
	{
        return  FALSE;
	}

	if(fileMaxSize == E_FileNotSupportWaring)   //modify by chentao@20211111  为了防止每一个文件拼一次警告，返回0xFE，只在END拼一次警告
	{
		//appendPEStatusInfo(FETURE_NOT_SUPPORTED,G_PEControlContext.PE_identification); //拼警告，profile继续下载
		SetBit(G_profileControlContext.IsCreateFlag,CREATE_FileNotSupportWaring);
		G_Tmp_identification = G_PEControlContext.PE_identification;
		return 0xFE;
	}
			
	if(fileMaxSize <= E_NotSupport)
	{
		return fileMaxSize;
	}

	SetProfileChannel(fileAddr,fileName);

	return 0xFE;
}

BYTE autoWriteEeprom(UWORD eepAddr,BYTE * SourceAddr,WORD size,BYTE mode)
{
	//统一使用非原子写,by xwh 20200505
	nvmCopyNoAtomic((BYTE *)eepAddr,SourceAddr,size);

	return TRUE;
}

BYTE autoWriteTFBuf(UWORD fileAddr, WORD offset, BYTE * buf, BYTE len,BYTE mode)
{
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

	return autoWriteEeprom(fileAddr,&buf[0],len,mode);
}

BYTE autoWriteLFBuf(UWORD fileAddr, BYTE index, BYTE offset, BYTE * buf, BYTE len,BYTE mode)
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

	return autoWriteEeprom(fileAddr,&buf[0],len,mode);
}

/*@Function:    profileUpdateBinary                                           
 * 
 *@Descrip:     profile解析调用的UpdateBinary API
 *
 *@See:	        ts_102222
 *                                                                                                                                     
 *@Param:       off          ---> 更新二进制文件的偏移量
 *@param        dataBuffer   ---> 源数据
 *@param        dataLen      ---> 源数据长度
 *				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                            
 * 
 *@Author:      XWH 20170616
 *  
 *@History:     V1.0---Create by XWH 20170616         
 *              V1.1---modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP    
 */ 
BYTE  profileUpdateBinary(WORD off,BYTE* dataBuffer,WORD dataLen)
{
	while(dataLen > 0xFF)
	{
		VMReadBuf(tempBuf,(UWORD)dataBuffer,0xFF);
		autoWriteTFBuf(G_fileControlContext.LoadEFAddr,off,tempBuf,0xFF,PROFILE_MODE);
		dataLen -= 0xFF;
		off += 0xFF;
		dataBuffer += 0xFF;
	}	
    VMReadBuf(tempBuf,(UWORD)dataBuffer,dataLen);	
	return autoWriteTFBuf(G_fileControlContext.LoadEFAddr,off,tempBuf,dataLen,PROFILE_MODE);
	
}

/*@Function:    profileUpdateRecord                                           
 * 
 *@Descrip:     profile解析调用的UpdateRecord API
 *
 *@See:	        ts_102222
 *  
 *@Param:       recNum       --->  文件记录号
 *@Param:       off          ---> 更新二进制文件的偏移量
 *@param        dataBuffer   ---> 源数据
 *@param        dataLen      ---> 源数据长度
 *				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                            
 * 
 *@Author:      XWH 20170616
 *  
 *@History:     V1.0---Create by XWH 20170616             
 *              V1.1---modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
 */ 
BYTE profileUpdateRecord(BYTE recNum,WORD off,BYTE* dataBuffer,WORD dataLen)
{
	WORD offset;
	
	offset = G_fileControlContext.UpdateOffset; //G_UpdateOffset 保持不变
	if(G_fileControlContext.CurrentFileFileRecLen > 0)
	{
		while((off + dataLen) > G_fileControlContext.CurrentFileFileRecLen)
		{	
			VMReadBuf(tempBuf,(UWORD)dataBuffer,(G_fileControlContext.CurrentFileFileRecLen-off));
			autoWriteLFBuf(G_fileControlContext.LoadEFAddr,recNum,off,tempBuf,(G_fileControlContext.CurrentFileFileRecLen-off),PROFILE_MODE);
			dataLen -= (G_fileControlContext.CurrentFileFileRecLen-off);
			
			dataBuffer += (G_fileControlContext.CurrentFileFileRecLen-off);
			
			offset += (G_fileControlContext.CurrentFileFileRecLen-off);
			recNum = (offset / G_fileControlContext.CurrentFileFileRecLen)+1;
			off = offset % G_fileControlContext.CurrentFileFileRecLen;
		}
		
	}	
	 VMReadBuf(tempBuf,(UWORD)dataBuffer,dataLen);
	 return autoWriteLFBuf(G_fileControlContext.LoadEFAddr,recNum,off,tempBuf,dataLen,PROFILE_MODE);
}

/*@Function:    profileSelectByPath                                           
 * 
 *@Descrip:     profile解析调用的SelectByPath API
 *
 *@See:	        ts_102222
 *                                                                                                                                     
 *@Param:       path    ---> 文件路径（不包括3F00），例如7F106F3A
 *@param        pathLen ---> 文件路径长度
 *				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                            
 * 
 *@Author:      XWH 20170616
 *  
 *@History:     V1.0---Create by XWH 20170616   
 *              V1.1---Modify by XWH 20170727,修改函数，支持path 是FLASH 地址的情况
 *              V1.2---modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
 */ 
BYTE profileSelectByPath(BYTE* path,WORD pathLen)
{	
	WORD  idata i;
	WORD  idata fileName;
	UWORD idata fileAddr;
	
	fileAddr = FALSE;
	
	if(pathLen == 0) //若pathLen为0，表示选择MF
	{
		SetProfileChannel(G_CreatingMFOffset,DF_MF);	
		return TRUE;
	}
	
	for(i=0; i < pathLen; i+=2)
	{
		fileName = VMReadShort((UWORD)(path+i));
		
		if(i == 0)
		{
			fileAddr = SelectFile(G_CreatingMFOffset,fileName);
		}
		else
		{
			fileAddr = SelectFile(G_fileControlContext.LoadDFAddr,fileName);
		}

		if(fileAddr == FALSE)
		{
			return FALSE;
		}
		
		SetProfileChannel(fileAddr,fileName);	
	}
			
	return TRUE;;
}



/*@Function:    updatePukData                                           
 * 
 *@Descrip:     profile解析调用写PUK数据
 *
 *@See:	        ts_102222
 *                                                                                                                                     
 *@Param:       pukData    ---> PUK数据，格式见<profile解析模块设计文档.docx>
 *@param        len        ---> PUK数据长度
 *				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                            
 * 
 *@Author:      XWH 20170616
 *  
 *@History:     V1.0---Create by XWH 20170616             
 */ 
BYTE updatePukData(BYTE * pukData,BYTE pukLen)
{
	UWORD idata fileAddr;
	BYTE  idata num,i,tag;
	
	fileAddr = SelectFile(G_CreatingMFOffset,DF_PROFILECFG);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	fileAddr = SelectFile(fileAddr,EF_PRIVATE_PUK);
	
	num = GetLFRecNumLgth(fileAddr,FileNum);
	
	for(i = 1; i <= num; i++)
	{
		ReadLFBuf(fileAddr,i,0,&tag,1);
		
		if(tag == 0xFF)                          //第一字节是0xFF表示空记录
		{		
			WriteLFBuf(fileAddr,i,0,pukData,pukLen);
			
			return TRUE;	
		}
        else
		{
			if(tag == pukData[0])//存在相同的pukRef
			{
				return FALSE;
			}
		}			
	}
	return FALSE; //pukCodes SEQUENCE (SIZE (1..16))OF PUKConfiguration,若所有的记录都被占用了，则抛错
}


/*@Function:    updatePinData                                           
 * 
 *@Descrip:     profile解析调用写PUK数据
 *
 *@See:	        ts_102222
 *                                                                                                                                     
 *@Param:       pukData    ---> PUK数据，格式见<profile解析模块设计文档.docx>
 *@param        len        ---> PUK数据长度
 *				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                            
 * 
 *@Author:      XWH 20170616
 *  
 *@History:     V1.0---Create by XWH 20170616 
 *              V1.1---Modify by XWH 20170801,PUK 默认次数设置成10 次
 *              V1.2---modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
                V1.3---modify by zhxx@20220419,同步EPX产业化/INFENION_SLM97/Branch/huawei代码 */
BYTE updatePinData(BYTE* pinBuf,BYTE pinBufLen)
{
	UWORD idata fileAddrPin,firstLevelDf;
	BYTE  idata recNum,tag,i;
	WORD  idata fileName;
	UWORD idata fileAddrPinMf;
	BYTE  idata num; 
	
	
//	if((((pinBuf[0] & 0x80) == 0x80) && (G_fileControlContext.LoadADFAddr == FALSE)) || (((pinBuf[0] & 0x80) == 0) && (G_fileControlContext.LoadADFAddr != FALSE)))//当前ADF地址为空，但是更新的是localpin的Ref，或者当前ADF不为空，更新的是globalpinRef，都要报错
//	{
//		return FALSE;
//	}
	
	if(G_fileControlContext.LoadDFAddr == G_CreatingMFOffset)//3F00/B100/1B02
	//if(G_LoadADfAddr == FALSE)//3F00/B100/1B02
	{		
		SetBit(G_profileControlContext.IsCreateFlag,CREATE_GolbalPIN); //global pin
		fileAddrPin = SelectFile(G_CreatingMFOffset,DF_PROFILECFG);
		if(fileAddrPin == FALSE)
		{
			return FALSE;
		}
		
		fileAddrPin = SelectFile(fileAddrPin,EF_PRIVATE_PIN);
	}
	else
	{
		SetBit(G_profileControlContext.IsCreateFlag,CREATE_LocalPIN);

		firstLevelDf = GetFirstLevelDf(G_fileControlContext.LoadDFAddr);

		fileAddrPin = SelectFile(firstLevelDf,EF_PRIVATE_PIN);//ADF/1B02

		fileName = GetFileHeadWord(firstLevelDf,F_Name_Offset);

		if(fileName == 0x7F10)// add by chentao@20211216 修改红茶移动profile下载失败的bug--profile中MF没有 PIN PE，7F10下有PIN PE
		{
			fileAddrPinMf = SelectFile(G_CreatingMFOffset,DF_PROFILECFG);
			fileAddrPinMf = SelectFile(fileAddrPinMf,EF_PRIVATE_PIN);

			if(fileAddrPinMf != FALSE)
			{
				recNum = GetLFRecNumLgth(fileAddrPinMf,FileNum);

				num = LocateRecordInFile(fileAddrPinMf,pinBuf[0]);	 //查找特定类型的PIN

				if(num == FALSE)
				{
					for(i = 1; i <= recNum; i++)
					{
						ReadLFBuf(fileAddrPinMf,i,0,&num,1);

						if(num == 0xFF)//空记录
						{
							SetBit(G_profileControlContext.IsCreateFlag,CREATE_GolbalPIN); //global pin
							WriteLFBuf(fileAddrPinMf,i,0,pinBuf,pinBufLen);
							break;
						}
					}
				}
			}
		}
	}
	
		
	if(fileAddrPin == FALSE)
	{
		return FALSE;
	}
		
	recNum = GetLFRecNumLgth(fileAddrPin,FileNum);

	for(i = 1; i <= recNum; i++)
	{
		ReadLFBuf(fileAddrPin,i,0,&tag,1);
		
		if(tag == 0xFF)//空记录
		{
			WriteLFBuf(fileAddrPin,i,0,pinBuf,pinBufLen);
		
		   return TRUE;
		}
		else
		{
			if(tag == pinBuf[0])//存在相同的PINRef
			{
				return FALSE;
			}
		}					
				
	}	
    
	return FALSE;  //pinconfig SEQUENCE (SIZE (1..26))OF PINConfiguration,将PIN文件记录定义为26条，若所有的记录都被占用了，则返回FALSE
}




//#define outBuffer    (tempBuf + 200)

/**
*modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void setHRPD (BYTE *kiBuf, BYTE len)
{
	UWORD idata fileAddr;
	fileAddr = SelectFile(G_fileControlContext.LoadADFAddr,EF_CHAP_SS);
	
	if(fileAddr == FALSE)
	{
		return;
	}
	
	kiBuf[0] = kiBuf[2] >> 3;
	
	ShiftDataUncoil(&kiBuf[2], len, &kiBuf[1], 5);          //去掉头5位，尾3位
	
	WriteTFBuf(fileAddr,0x00,&kiBuf[0],kiBuf[0]+1);		
}



//为了简化处理，该函数只处理了一个CHAP SS(因为目前下载的电信Profile只有一个CHAP SS)
/**
*modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void setCHAP (BYTE *kiBuf, WORD len)
{
	UWORD idata fileAddr;
	WORD idata fileOff;
	BYTE  idata naiNum; 
	
	fileAddr = SelectFile(G_fileControlContext.LoadADFAddr,EF_CHAP_SS_TABLE);
	 
	if (fileAddr == FALSE)
	{
		return;
	}
	
	//ecpMem_Copy(&outBuffer[2], kiBuf, len);			//把所有数据复制到 outBuffer[2] 分析, 预留两字节是LV
	
	naiNum = kiBuf[2] >> 4; 
	fileOff = 0;	
	kiBuf[2+len] = 0;	
	
	ShiftDataUncoil(&kiBuf[2], len+1, &kiBuf[2], 4);        //TODO: ShiftDataUncoil存在bug, 偏移的最后一位存在bug，所以需要最后一位的下一位置0，偏移数组是共len+1的长度才正常
	
	while (naiNum--)
	{	
		kiBuf[0] = kiBuf[2] >> 4;			//tag
		
		ShiftDataUncoil(&kiBuf[2], len, &kiBuf[2], 4); 		
		
		kiBuf[1] = kiBuf[2] >> 3;			//length
		
		ShiftDataUncoil(&kiBuf[2], len, &kiBuf[2], 5);

		WriteTFBuf(fileAddr, fileOff, &kiBuf[0], kiBuf[1]+2);	
		
		fileOff += kiBuf[1]+2;
		
		ecpMem_Move(&kiBuf[2], &kiBuf[2+kiBuf[1]], len);
	}	
}

//为了简化处理，该函数对每一个 MN_AAA和MN_HA都当作 16 字节处理
/**
*modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void setMobileIpSs(BYTE *kiBuf, WORD len)
{
	UWORD idata fileAddrHA,fileAddrAAA;
	WORD idata fileAddrAAAOff, fileAddrHAOff;
	BYTE idata naiNum;
	
		
	fileAddrAAA = SelectFile(G_fileControlContext.LoadADFAddr, MN_AAA_SS_TABLE);
	
	fileAddrHA = SelectFile(G_fileControlContext.LoadADFAddr, MN_HA_SS_TABLE); 	        //6E80(HPRD 模式鉴权密钥文件)
	
	if((fileAddrHA == FALSE)||(fileAddrAAA == FALSE))
	{
		return;
	}
	
	//ecpMem_Copy(&outBuffer[2], kiBuf, len);			//把所有数据复制到 outBuffer[2] 分析, 预留两字节是LV
	
	naiNum = kiBuf[2] >> 4; 
	fileAddrAAAOff = 0;	
	fileAddrHAOff = 0;
	kiBuf[2+len] = 0;	
	
	ShiftDataUncoil(&kiBuf[2], len+1, &kiBuf[2], 4);        //TODO: ShiftDataUncoil存在bug, 偏移的最后一位存在bug，所以需要最后一位的下一位置0，偏移数组是共len+1的长度才正常
	
	while (naiNum--)
	{	
		kiBuf[0] = kiBuf[2] >> 4;				//tag	
		
		ShiftDataUncoil(&kiBuf[2], len, &kiBuf[2], 4); 		
		
		kiBuf[1] = kiBuf[2] >> 3;				//length
		
		ShiftDataUncoil(&kiBuf[2], len, &kiBuf[2], 5);

		WriteTFBuf(fileAddrAAA, fileAddrAAAOff, &kiBuf[0], kiBuf[1]+2);	
		
		fileAddrAAAOff += kiBuf[1]+2;
		
		//以上是MN-AAA_SS处理， 以下是MN-HA_SS处理
		ecpMem_Move(&kiBuf[2], &kiBuf[2+kiBuf[1]], len);
		
		kiBuf[1] = kiBuf[2] >> 3;				//length
		
		ShiftDataUncoil(&kiBuf[2], len, &kiBuf[2], 5);
		
		WriteTFBuf(fileAddrHA, fileAddrHAOff, &kiBuf[0], kiBuf[1]+2);

		fileAddrHAOff += kiBuf[1]+2;
		
		ecpMem_Move(&kiBuf[2], &kiBuf[2+kiBuf[1]], len);
	}		
}

#define MobileIpSsReadLen         32
/*
*@function   : setMobileIpSsAddr的子函数
*
*@description:
*
*@see        : <eUICC Profile Package Interoperable Format Technical Specification v2.0.pdf>  8.4.3
*
*@param      :kiBufAddr 传入数据地址
*@para       :outPutoffset  tempBuf的偏移
*@param      :len       传入数据的长度
*
*@return     :void
*
*@author     :LCZ

*@history    :V1.0 :create by LCZ 2019-7-18
*/
void MobileIpSsBitToBYTE(UWORD kiBufAddr, WORD outPutoffset,BYTE len)
{
	SBYTE idata j;
	WORD idata i;
	BYTE xdata buf[MobileIpSsReadLen];
	
	VMReadBuf(buf,kiBufAddr,sizeof(buf));
	
	for(i = 0x00;i<sizeof(buf);i++)
	{
		for(j= 0x07;j>=0;j--)
		{
			if(ValueBit(buf[i],j))
			{
				tempBuf[outPutoffset++] = 0x31;
			}
			else
			{
				tempBuf[outPutoffset++] = 0x30;
			}
		}
	}
}
/*setMobileIpSsAddr的子函数*/
void MobileIpSsBYTEToBit(BYTE *input,BYTE *output,BYTE Len)
{
	BYTE idata i,j;
	
	if(Len <0x08)
	{
		output[0] = 0x00;
		
		for(i=0x00;i<Len;i++)
		{
			if(input[i] == 0x31)
			{
				SetBit(output[0],Len-i-1);
			}		 
		}
	}
	else //只能是8的整数倍
	{
		for(j=0x00;j<Len;j+=8)
		{
			output[j/8] = 0x00;
			for(i=0x00;i<0x08;i++)
			{
				if(input[i+j] == 0x31)
				{
					SetBit(output[j/8],7-i);
				}
			}
		}
	}
}

void MobileIpSsChangeOffset(UWORD kiBufAddr,WORD *tempoffset,WORD * offset)
{
	ecpMem_Move(tempBuf,&tempBuf[*offset],*tempoffset-*offset); //
	
	MobileIpSsBitToBYTE(kiBufAddr,*tempoffset-*offset,MobileIpSsReadLen);

	*tempoffset = (*tempoffset-*offset) + MobileIpSsReadLen*8;
	
	*offset = 0x00;
	
	return;
}
	
/*
*@function   : setMobileIpSsAddr
*
*@description:处理CMDA 85 tag数据比较大的时候 ,同时tempBuf要求为512大小
*
*@see        : <eUICC Profile Package Interoperable Format Technical Specification v2.0.pdf>  8.4.3
*
*@param      :kiBufAddr 传入数据地址
*@param      :len       传入数据的长度
*
*@return     :void
*
*@author     :LCZ

*@history    :V1.0 :create by LCZ 2019-7-18
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void setMobileIpSsAddr(UWORD kiBufAddr, WORD len)
{
	UWORD idata fileAddrHA,fileAddrAAA;
	WORD idata fileAddrAAAOff, fileAddrHAOff,offset,tempoffset;
	BYTE idata naiNum,i;
	BYTE idata resultBuf[33]; //缓存结果 TLV结构 L最大值为31
	
		
	fileAddrAAA = SelectFile(G_fileControlContext.LoadADFAddr, MN_AAA_SS_TABLE);
	
	fileAddrHA = SelectFile(G_fileControlContext.LoadADFAddr, MN_HA_SS_TABLE); 	        //6E80(HPRD 模式鉴权密钥文件)
	
	if((fileAddrHA == FALSE)||(fileAddrAAA == FALSE))
	{
		return;
	}
	
	tempoffset = 0x00;
	
	MobileIpSsBitToBYTE(kiBufAddr,tempoffset,MobileIpSsReadLen);

	tempoffset = MobileIpSsReadLen*8; //使用了tempBuf这么多位置
	
	kiBufAddr +=MobileIpSsReadLen; //源地址偏MobileIpSsReadLen
	
	fileAddrAAAOff = 0;	//目的地址偏移
	fileAddrHAOff = 0;  //目的地址偏移
	
	MobileIpSsBYTEToBit(tempBuf,&naiNum,0x04);     //获取 NUM_NAI
	
	offset = 0x04;                                 //下一个数据的偏移
	
	while(naiNum--)
	{
		if((tempoffset - offset)<0x04)
		{
			MobileIpSsChangeOffset(kiBufAddr,&tempoffset,&offset);
			
			kiBufAddr += MobileIpSsReadLen;
		}
		
		MobileIpSsBYTEToBit(&tempBuf[offset],&resultBuf[0],0x04);      //NAI_ENTRY_INDEX
		
		offset += 0x04;
		
		for(i= 0x00;i<2;i++)
		{
			if((tempoffset - offset)<0x05)
			{
				MobileIpSsChangeOffset(kiBufAddr,&tempoffset,&offset);
				
				kiBufAddr += MobileIpSsReadLen;
			}
			
			MobileIpSsBYTEToBit(&tempBuf[offset],&resultBuf[0x01],0x05);      //MN-AAA_SS_LENGTH  or  MN-HA_SS_LENGTH
				
			offset += 0x05;
			
			if((tempoffset - offset)<(resultBuf[0x01]*8)) //
			{
				MobileIpSsChangeOffset(kiBufAddr,&tempoffset,&offset);
				
				kiBufAddr += MobileIpSsReadLen;
			}
			
			MobileIpSsBYTEToBit(&tempBuf[offset],&resultBuf[0x02],resultBuf[0x01]*8);      //MN-AAA_SS  or MN-HA_SS
				
			offset += resultBuf[0x01]*8;
			
			if(i == 0x00)
			{
				WriteTFBuf(fileAddrAAA, fileAddrAAAOff, &resultBuf[0], resultBuf[1]+2);
				
				fileAddrAAAOff += resultBuf[1]+2;
			}
			else
			{
				WriteTFBuf(fileAddrHA, fileAddrHAOff, &resultBuf[0], resultBuf[1]+2);
				
				fileAddrHAOff += resultBuf[1]+2;
			}
		}

	}
	
}

/*
*@function   : setAuthCounterMax
*
*@description:处理PEAKA 组件中的authCounterMax 字段
*
*@see        : <eUICC Profile Package Interoperable Format Technical Specification v2.0.pdf>  8.4.2
*
*@param      :counter
*@param      :counterLen
*
*@return     :void
*
*@author     :zdm

*@history    :V1.0 :create by zdm@20190115
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void setAuthCounterMax(BYTE* authCounter,BYTE len)
{
	UWORD  fileAddr;
	
	if(G_fileControlContext.LoadADFAddr != FALSE)
	{
		fileAddr = SelectFile(G_fileControlContext.LoadADFAddr,EF_AuthMaxCounter);//私有模板里面需要创建1B1D 文件
	
		if(fileAddr == FALSE)
		{
			return;
		}
			
		SetTFConst(fileAddr,0x00,0,1);  
		
		WriteTFBuf(fileAddr,0x01,&authCounter[0],len);	
	}	
}



/*
*@function   : setSqnInit
*
*@description:处理PEAKA 组件中的setSqnInit 字段
*
*@see        : <eUICC Profile Package Interoperable Format Technical Specification v2.2.pdf>  8.4.2
*
*@param      :BYTE* sqnValue --   sqn的值
*@param      :BYTE len       --   sqn的长度
*@param      :BYTE index     --   sqn的索引
*
*@return     :void
*
*@author     :zhxx

*@history    :V1.0 :create by zhxx@20190408
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void setSqnInit(BYTE* sqnValue,BYTE len,BYTE index)
{
	UWORD  fileAddr;
	
	if(G_fileControlContext.LoadADFAddr != FALSE)
	{
		fileAddr = SelectFile(G_fileControlContext.LoadADFAddr,EF_SQNCheck);//选择EF_1B11
	
		if(fileAddr == FALSE)
		{
			return;
		}
			
		WriteLFBuf(fileAddr,index,0,&sqnValue[0],len);	
	}	
}



/*@Function:    setAuthCfg                                           
 * 
 *@Descrip:     设置sqnOptions字段： 
 *
 *@See:	        ts_102222
 *                                                                                                                                     
 *@Param:       sqnOption 

 *				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                            
 * 
 *@Author:      XWH 20170616
 *  
 *@History:     V1.0---Create by XWH 20170616  
 *              V1.1---modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP           
 */ 
void setAuthCfg(BYTE sqnOption) 
{
		BYTE  idata tmpData,AlgorithmID;
	
	    AlgorithmID = getAlgorithmID(G_fileControlContext.LoadADFAddr);
	
		//SQN Delta (1: not used, 0: used) ,此时把2E00里面的deltaC字节设置成FF(不使用delta)
		if(sqnOption&0x04)
		{
			tmpData = 0xFF;
			
			if(AlgorithmID == 1)//milenage
			{
				 WriteAuthCfgData(OFFSET_AUTHCFG_N,&tmpData,1);		
			}
			else if(AlgorithmID == 2)//TUAK
			{
				 WriteAuthCfgData(OFFSET_TUAK_AUTHCFG_N,&tmpData,1);	
			}
			else
			{
				
			}					   
		}
		
		if((sqnOption&0x08) == 0x08) //SQN Age Limit (1: not used, 0: used),将配置文件中的L设置为0
		{
			ecpMem_Set(&tempBuf[0],0,6);
			//setSqnAgeLimit(&tempBuf[0], 6);
			if(AlgorithmID == 1)//milenage
			{
				WriteAuthCfgData(OFFSET_AUTHCFG_L,&tempBuf[0],6); 
			}
			else if(AlgorithmID == 2)//TUAK
			{
				WriteAuthCfgData(OFFSET_TUAK_AUTHCFG_L,&tempBuf[0],6); 
			}
			else
			{
				
			}	
		}
		
			
		//bit1:Anonymity Key (AK) (1: not used, 0: used)
        //bit2:SQN wrap around (1: not allowed, 0: allowed)
        //In case SQN wrap around is allowed it means that SQN verification will be disabled if the respective SEQ value has reached the maximum value 07FFFFFFFFFF
		switch(sqnOption&0x03)
		{
			case 0:
				tmpData = 5;
				break;
			case 1:
				tmpData = 4;
				break;
			case 2:
				tmpData = 1;
				break;
			//case 3:
			default:
				tmpData = 0;
				break;
		}		
		
		WriteAuthCfgData(OFFSET_AUTHCFG_OPCFG,&tmpData,1);	//TUAK和milenage的配置偏移都是0
						
}

/*
*@function   : setAlgorithmOptions
*
*@description: 处理PEAKA 中的algorithm-Options 字段: bit1-3:  RES size (0: 32 bits ,1: 64 bits, 2:128 bits, 3: 256 bits)1, bit4-6 :MAC-A and MAC-S size (0: 64 bits ,1: 128 bits, 2: 256 bits)2 bit7:CK and IK size (0: 128 bits ,1: 256 bits)3 bit 8: RFU
*
*@param      :BYTE setAlgorithmOptions
*
*@return     : 处理MILEGNE 和TUAK的参数
*
*@author      :zdm
*
*@hisroty     :V1.0 :create by zdm@20190115
*             :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void setAlgorithmOptions(BYTE algorithmOptions)
{
	BYTE xdata buf[13];
	BYTE idata algorithmID;
	
	//sqnDelta OCTET STRING (SIZE(6)) DEFAULT '000010000000'H, /* ignored in case of usim-test-algorithm */ 
	//sqnAgeLimit OCTET STRING (SIZE(6)) DEFAULT '000010000000'H, /* ignored in case of usim-test-algorithm */
	BYTE code ind_l_n[] = {0x05, 0x00,0x00,0x10,0x00,0x00,0x00,0x1c};  //IND,L,N的默认配置
	
	algorithmID = getAlgorithmID(G_fileControlContext.LoadADFAddr);
	
	buf[0] = (algorithmOptions&0x07);//低3个bit(b1b2b3)用来表示resLen
	
	switch(buf[0])
	{
		case 0:
			buf[0] = 4;
			break;
		case 1:
			buf[0] = 8;
			break;
		case 2:
			buf[0] = 16;
			break;
		case 3:
			buf[0] = 32;
			break;	
		default:
			if(algorithmID == 1) //MILENGE 算法,当前LOAD的PROFILE的算法变量
			{	
				buf[0] = 8;
			}
			else
			{
				buf[0] = 0;
			}			
			break;	
	}
	if(algorithmID == 2) //TUAK
	{
		buf[1] = (algorithmOptions&0x38);//(b4b5b6)用来表示macASlen
		switch(buf[1] )
		{
			case 0:
				buf[3]  = 8;
				break;
			case 0x08:
				buf[3]  = 16;
				break;
			case 0x10:
				buf[3]  = 32;
				break;	
			default:
				buf[3]  = 0;
			break;		
		}
		
		if((algorithmOptions & 0x40) == 0x40)  //CK and IK size (0: 128 bits ,1: 256 bits)3
		{
			buf[1] = 0x20;
			buf[2] = 0x20;
		}
		else
		{
			buf[1] = 0x10;
			buf[2] = 0x10;
		}
		
		buf[4] = 0x01;  //KIITA numberOfKeccak   //numberOfKeccak UInt8 DEFAULT 1 
			
		ecpMem_Copy(&buf[5],&ind_l_n[0],8);
		
		WriteAuthCfgData(OFFSET_TUAK_AUTHCFG_RESSIZE,&buf[0],sizeof(buf));  
	}
	else
	{
		WriteAuthCfgData(OFFSET_AUTHCFG_RESLEN,&buf[0],1);  //milenge 算法
	}
		
    
}

/*@Function:    setSqnDelta                                           
 * 
 *@Descrip:     设置用于计算△＝2^n的n的值
 *
 *@See:	        ts_102222
 *                                                                                                                                     
 *@Param:       xoring     ---> C数据
 *@param        len        ---> C数据长度
 *				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                            
 * 
 *@Author:      XWH 20170616
 *  
 *@History:     V1.0---Create by XWH 20170616             
 */ 
void setSqnDelta(BYTE * sqnDelta,BYTE len,BYTE algorithmID)
{
	BYTE idata integer,deci;
	BYTE idata i,tmpData;
	
	for(i = 0; i<6; i++)
	{
		if(sqnDelta[i] != 0x00)
		{
			break;
		}
	}
	
	if(i == 6)
	{
		//error,△ 一定为非0值
	}
	
	integer = 5 - i;
	
	integer = integer << 3;
	
	for(deci=0 ; deci < 8 ; deci++ )
	{
		if(ValueBit(sqnDelta[i] ,deci) == TRUE)
		{
			break;
		}
	}
	
	tmpData = integer | deci;
	
	if(algorithmID == 1)
	{
		WriteAuthCfgData(OFFSET_AUTHCFG_N,&tmpData,1);	
	}
	else if(algorithmID == 2)
	{
		WriteAuthCfgData(OFFSET_TUAK_AUTHCFG_N,&tmpData,1);	
	}
	else
	{
		
	}
			
	
}

/*@Function:    setSqnAgeLimit                                           
 * 
 *@Descrip:     设置L的内容
 *
 *@See:	        ts_102222
 *                                                                                                                                     
 *@Param:       sqnAgeLimit     ---> C数据
 *@param        len        ---> C数据长度
 *				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                            
 * 
 *@Author:      XWH 20170616
 *  
 *@History:     V1.0---Create by XWH 20170616             
 */ 
/*
void setSqnAgeLimit(BYTE * sqnAgeLimit,BYTE len) 
{
	WriteAuthCfgData(OFFSET_AUTHCFG_L,sqnAgeLimit,len);	
}*/

/*
*@function   : setTuakSqnOption
*
*@description: 处理PEAKA 中的SqnOption 字段:
                 bit1:  Anonymity Key (AK) (1: not used, 0: used)
                 bit2 : SQN wrap around (1: not allowed, 0: allowed)
                            In case SQN wrap around is allowed it means that SQN verification 
                            will be disabled if the respective SEQ value has reached the 
                            maximum value 07FFFFFFFFFF
                 bit3: SQN Delta (1: not used, 0: used)
                 bit4: SQN Age Limit (1: not used, 0: used)

*/

/*@Function:    getMapFileArrNum                                           
 * 
 *@Descrip:     取mapfile的arrnum
 *
 *@See:	        
 * 
*@Param:        mapFile0     
 *@Param:       mapFile1      
 *@param        mapFile2     
 *				                                                                                                         
 *@Return:      BYTE                                                                                                                                                                                                                                                            
 * 
 *@Author:      zhhj 20180705
 *  
 *@History:     V1.0---Create by zhhj 20180705             
 */ 
BYTE getMapFileArrNum(WORD mapFile0,WORD mapFile1,WORD mapFile2)
{
		BYTE idata arrNum;
		UWORD xdata mapfileAddr;
			
		//zhhj 获得实体文件的arrnum,替换为映射文件的
			
		mapfileAddr = SearchFile(G_CreatingMFOffset,mapFile0);
		if(mapfileAddr == FALSE)
		{
			return FALSE;
		}
		
		mapfileAddr = SearchFile(mapfileAddr,mapFile1);					  	
		
		if(mapfileAddr == FALSE)
		{
			return FALSE;					  	
		}
		
		mapfileAddr = SearchFile(mapfileAddr,mapFile2);

		if(mapfileAddr == FALSE) //17.HB_Offline_Profile_Test/14-PROFILE PE数据异常测试/Profile_FulSeg0058
		{
			return FALSE;
		}
		
		arrNum= GetFileHeadByte(mapfileAddr,F_ArrNum_Offset);
		
		return arrNum;
}



/*@Function:    WriteAuthCfgData                                           
 * 
 *@Descrip:     写EF_PRIVATE_AUTHCFG文件内容
 *
 *@See:	        ts_102222
 * 
*@Param:        offset     ---> 写EF_PRIVATE_AUTHCFG文件的偏移
 *@Param:       value      ---> 要写的数据
 *@param        len        ---> 数据长度
 *				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                            
 * 
 *@Author:      XWH 20170616
 *  
 *@History:     V1.0---Create by XWH 20170616            
 *              V1.1---modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP 
 */ 
void WriteAuthCfgData(BYTE offset,BYTE * value,BYTE len)
{
	UWORD idata fileAddr;

    if(G_fileControlContext.LoadADFAddr != FALSE)
	{
		fileAddr = SelectFile(G_fileControlContext.LoadADFAddr,EF_PRIVATE_AUTHCFG);//选择EF_1B10
	
		if(fileAddr == FALSE)
		{
			return;
		}
			
		WriteTFBuf(fileAddr,offset,&value[0],len);	
	}	
}

BYTE autoSetEeprom(UWORD eepAddr,BYTE hex,WORD size,BYTE mode)
{
	nvmFillNoAtomic((BYTE *)eepAddr, hex,  size);
	return TRUE;
}
#endif
BYTE FindTagOffset(BYTE * dataPointer,BYTE tag,BYTE lgth,BYTE occ)
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
			occ--;
			if(occ == 0)
			{
				return ((i&0xFF)+1+ch);		
			}				
		}
	
		i =i + dataPointer[i+1+ch]+2+ch;
	}

	return FALSE;
}

/*
*@function: setKI
*
*@description:处理PEAKA组件中的字段AlgoParameter的key OCTET STRING, 数据，目前将其认为是ki
*
*@param      :BYTE *kiBuf
*@param      :BYTE len
*
*@return      :void
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20170726
*             :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE setKI(BYTE type,BYTE *kiBuf, BYTE len)
{

	UWORD idata fileAddr;
	BYTE idata algorithmID;

#if (EUICC_ONLY||SE_AND_EUICC)
	if(type == PROFILEDOWNMODE)// download profile
	{
		fileAddr = SelectFile(G_fileControlContext.LoadADFAddr,EF_KI);
		algorithmID = getAlgorithmID(G_fileControlContext.LoadADFAddr);
		//如果是TUAK，将KI长度写入	还要把len写2D00的第34字节
    // TOPCMode(1字节) + TOPC(32字节) + KISize(1字节) + ResSize(1字节) + CKSize(1)+IKSize(1字节) + MACSize(1字节)+KIiTA(1字节)
		if(algorithmID == 2)
		{
		   WriteAuthCfgData(OFFSET_TUAK_AUTHCFG_KISIZE,&len,1);
		}
	}
	else
#endif
	{
		fileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum],EF_KI);

	}

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	return WriteLFBuf(fileAddr,1,0x00,&kiBuf[0],len);

}

/*
*@function   : getAlgorithmID
*
*@description:根据传进来的ADF地址，获取该ADF下面所使用的算法
*
*@see        : <eUICC Profile Package Interoperable Format Technical Specification v2.2.pdf>  8.4.2
*
*@param      :UWORD fileAddr
*
*@return     :BYTE AlgorithmID
*
*@author     :zhxx

*@history    :V1.0 :create by zhxx@20190408
*/
BYTE getAlgorithmID(UWORD fileAddr)
{
	return GetFileHeadByte(fileAddr,F_AlgID_Offset);
}
/*
*@function   : ProfileIndexIsValid
*
*@description: 判断profileindex是否合法
*
*@see        :
*
*@param      :
*
*@return
*
*@author     :chentao

*@history    :V1.0 :create by chentao@20230210
*/
BYTE ProfileIndexIsValid(BYTE profileIndex)
{
	if((profileIndex >= 0) && (profileIndex < PROFILE_MAX_NUMBER))
	{
		return TRUE;
	}

	return FALSE;
}

