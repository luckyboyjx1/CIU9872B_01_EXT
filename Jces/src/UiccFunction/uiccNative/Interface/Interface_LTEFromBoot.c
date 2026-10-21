/************************************************************************************************************
 * Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
 * All rights reserved.
 *
 * File name:   hard_api.c
 *
 * Description: 该文件主要定义底层相关的功能函数
 *
 * auther：     xxxx    xxxx.xx.xx
 *
 * history：    V1.0
 *
 ************************************************************************************************************/
#include "typedef.h"
#include "Interface_LTEFromBoot.h"
#include "uicc_main.h"
#include "interface.h"
#include "gsmpublic.h"
#include "Crypto_Constant.h"
#include "Crypto_API.h"
#include "Stkhead.h"
#include "gsmpublic.h"
#include "NvmHal.h"
//#include "platform_api.h"
#include "nativemethod_impl.h"
#include "MemBackUp.h"
#include "JcvmDataConfig.h"


//BYTE xdata G_Bank;
//DWORD xdata G_CODE_SIZE;


void ReturnBootloader()
{
	ecpSysHal_ClearCard();
}

/*
BYTE SetEeprom(UWORD eepAddr, BYTE HEX,WORD length)
{
	nvmFillNoAtomic((BYTE *)eepAddr, HEX,  length);
	return TRUE;
}

BYTE WriteEeprom(UWORD LogEepAddr,BYTE * RAM_addr,WORD length)
{
	nvmCopyNoAtomic((BYTE *)LogEepAddr,RAM_addr,length);
	return TRUE;
}
*/
void ReadEeprom(BYTE * RAM_addr,UWORD sourceAddr,UWORD length)
{
	memCopyToRam(RAM_addr,(BYTE *)sourceAddr,length);
}

BYTE WriteEepromEX(UWORD eepAddr,BYTE * SourceAddr,WORD length)
{
	nvmCopyNoAtomic((BYTE *)eepAddr,SourceAddr,length);
	return TRUE;
}

/*
BYTE SecureMoveEeprom(UWORD desAddr,UWORD srcAddr, UWORD dataLen)
{
#define moveDateBuf  (tempBuf + 100)
	
	if(( dataLen == 0x00 ) || (srcAddr== desAddr ))
	{
		return FALSE;
	}
	
	SetMovDateBufSrcDstDataLen(moveDateBuf,srcAddr,desAddr,dataLen);

	WriteEepromEX(E_SECUREMOVE_START, moveDateBuf, SecureMoveStuct_LengthAll);	

	StartSecureMoveEeprom(0,0,0);
#undef moveDateBuf
	return TRUE;
}*/
#if (EUICC_ONLY||SE_AND_EUICC)
//void StartMoveEepromFast(void)
//{
//	BYTE  idata tag;
//	WORD  idata copyLgth,len;
//	UWORD xdata srcAddr,desAddr,dataLen;
//	UWORD xdata profileLength;
//	WORD  idata pages,i;

//	//注意，tempBuf 大小必须是512  
//#define moveDateBuffer  (tempBuf + 100)

//	CacheFlush();

//	memset(moveDateBuffer,0x00,SecureMoveStuct_LengthAll);

//	ReadEeprom(moveDateBuffer,E_SECUREMOVE_START,SecureMoveStuct_LengthAll);	

//	if(moveDateBuffer[SecureMoveEepromTag_Offset] != D_EEPMoveTag_FAST)
//	{
//		return;
//	}

//	srcAddr = BuildUWord(&moveDateBuffer[SecureMoveEepromSrcAddr_Offset]);
//	desAddr = BuildUWord(&moveDateBuffer[SecureMoveEepromDesAddr_Offset]);
//	dataLen = BuildUWord(&moveDateBuffer[SecureMoveEepromMoveLen_Offset]);
//		
//	if((dataLen == 0)||(srcAddr== desAddr))
//	{
//		return;
//	}
//	
//	tag = 0;
//	profileLength = srcAddr - desAddr;
//	pages = (profileLength/HAL_EEPAGE_SIZE);
//	profileLength = pages*HAL_EEPAGE_SIZE;       //整数页，至少是一页

//	while(dataLen >0)	
//	{
//			copyLgth = (HAL_EEPAGE_SIZE-GetPageOffset(desAddr));    //一页中的数据
//		
//			len = (dataLen >= copyLgth)?copyLgth:dataLen;      //本次要移动的数据长度
//		
//			if(tag != 0)
//			{
//				moveDateBuffer[SecureMoveEepromTag_Offset] = D_EEPMoveTag_FAST;

//				SetUWord(&moveDateBuffer[SecureMoveEepromSrcAddr_Offset],srcAddr);
//				SetUWord(&moveDateBuffer[SecureMoveEepromDesAddr_Offset],desAddr);
//				SetUWord(&moveDateBuffer[SecureMoveEepromMoveLen_Offset],dataLen);

//				WriteEepromEX(E_SECUREMOVE_START, moveDateBuffer, SecureMoveStuct_LengthPart);
//				CacheFlush();
//			}
//		
//			if((GetPageOffset(desAddr) == 0)&&(len == HAL_EEPAGE_SIZE))   //中间页，无断电保护写
//			{
//				CacheFlush();
//				pages = (dataLen > profileLength)?(profileLength/HAL_EEPAGE_SIZE):(dataLen/HAL_EEPAGE_SIZE);

//				for(i=0; i<pages; i++)
//				{
//					ecpNvm_WriteCache_NoCheck((BYTE *)(desAddr+i*HAL_EEPAGE_SIZE),(BYTE *)(srcAddr+i*HAL_EEPAGE_SIZE),HAL_EEPAGE_SIZE);
//				}

//				srcAddr += (pages*HAL_EEPAGE_SIZE);
//				desAddr += (pages*HAL_EEPAGE_SIZE);
//				dataLen -= (pages*HAL_EEPAGE_SIZE);  //剩余数据长度
//			}
//			else                                                     //首尾页，断电保护写
//			{
//				WriteEepromEX(desAddr, (BYTE *)srcAddr, len);

//				srcAddr += len;
//				desAddr += len;
//				dataLen -= len;  //剩余数据长度
//			}

//			tag = 1;
//	}

//	SetEepromEX(E_SECUREMOVE_START+SecureMoveEepromTag_Offset, 0xFF, sizeof(BYTE));
//	CacheFlush();
//#undef moveDateBuffer	
//	return ;
//}
#endif
/*
void SetMovDateBufSrcDstDataLen(BYTE * movDateBuf,UWORD srcAddr,UWORD desAddr,UWORD dataLen)
{
	memset(movDateBuf,0x00,SecureMoveStuct_LengthAll);
	movDateBuf[SecureMoveEepromTag_Offset] = D_EEPMoveTag;

	if(srcAddr < desAddr)
	{		
		SetUWord(&movDateBuf[SecureMoveEepromSrcAddr_Offset],srcAddr + dataLen);
		SetUWord(&movDateBuf[SecureMoveEepromDesAddr_Offset], desAddr + dataLen);		
	}
	else
	{
		SetUWord(&movDateBuf[SecureMoveEepromSrcAddr_Offset],srcAddr);
		SetUWord(&movDateBuf[SecureMoveEepromDesAddr_Offset], desAddr);
	}
	
	SetUWord(&movDateBuf[SecureMoveEepromMoveLen_Offset],dataLen);
}*/
/*
*@function     :StartSecureMoveEeprom
*
*@description  :带断电保护移动数据
*
*@see          :
*
*@param        :desAddr -> 目标地址
*@param        :srcAddr -> 源数据地址
*@param        :dataLen -> 移动数据长度
*
*@return       :void
*
*
*@author       :xwh@20181116
*
*@hisotry      :created by xwh@20181116
*              :增加(desAddr == 0)&&(srcAddr == 0)的判断，by xwh@20200923
*/

//void StartSecureMoveEeprom(UWORD desAddr,UWORD srcAddr, UWORD dataLen)
//{
//	BYTE    idata  len,moveType;
//	WORD  idata setLength,startOffset;
//#define moveDateBuf  (tempBuf + 100)

//	//memset(moveDateBuf,0x00,SecureMoveStuct_LengthAll);

//	if((desAddr == 0)&&(srcAddr == 0)&&(dataLen == 0))
//	{
//		ReadEeprom(moveDateBuf,E_SECUREMOVE_START,SecureMoveStuct_LengthAll);	
//#if (EUICC_ONLY||SE_AND_EUICC)
//		if(moveDateBuf[SecureMoveEepromTag_Offset] == D_EEPMoveTag_FAST)
//		{
//			StartMoveEepromFast();
//			return;
//		}
//#endif
//		if(moveDateBuf[SecureMoveEepromTag_Offset] != D_EEPMoveTag)
//		{
//			return;
//		}

//		srcAddr = BuildUWord(&moveDateBuf[SecureMoveEepromSrcAddr_Offset]);
//		desAddr = BuildUWord(&moveDateBuf[SecureMoveEepromDesAddr_Offset]);
//		dataLen = BuildUWord(&moveDateBuf[SecureMoveEepromMoveLen_Offset]);
//	}
//	else
//	{
//		ReadEeprom(moveDateBuf+SecureMoveEepromWriteDesAddr_Offset,E_SECUREMOVE_START+SecureMoveEepromWriteDesAddr_Offset,SecureMoveStuct_LengthAll-SecureMoveStuct_LengthPart);	
//	}
//	
//	if(srcAddr < desAddr)
//	{
//		moveType = D_MoveBackward;			
//	}
//	else
//	{
//		moveType = D_MoveForward;
//	}

//	len = 0x00;

//	while(dataLen >0)
//	{
//		len = (dataLen >= D_EEMoveSize)?D_EEMoveSize:dataLen;

//		if(moveType == D_MoveForward)
//		{
//				ReadEeprom((BYTE *)&moveDateBuf[SecureMoveEepromDataBuf_Offset], srcAddr, len);
//		}
//		else
//		{
//				ReadEeprom((BYTE *)&moveDateBuf[SecureMoveEepromDataBuf_Offset], srcAddr-len, len);
//		}

//		moveDateBuf[SecureMoveEepromLen_Offset] = len;

//		if(len < D_EEMoveSize)
//		{
//				memset((BYTE *)&moveDateBuf[SecureMoveEepromDataBuf_Offset+len],0x00,D_EEMoveSize-len);
//		}

//		moveDateBuf[SecureMoveEepromTag_Offset] = D_EEPMoveTag;

//		SetUWord(&moveDateBuf[SecureMoveEepromSrcAddr_Offset],srcAddr);
//		SetUWord(&moveDateBuf[SecureMoveEepromDesAddr_Offset],desAddr);
//		SetUWord(&moveDateBuf[SecureMoveEepromMoveLen_Offset],dataLen);

//		WriteEepromEX(E_SECUREMOVE_START, moveDateBuf, SecureMoveStuct_LengthPart);

//		if(moveType == D_MoveForward)
//		{
//				WriteEepromEX(desAddr, &moveDateBuf[SecureMoveEepromDataBuf_Offset], len);
//				srcAddr = srcAddr + len;
//				desAddr = desAddr + len;
//		}
//		else
//		{
//				WriteEepromEX(desAddr-len, &moveDateBuf[SecureMoveEepromDataBuf_Offset], len);
//				srcAddr = srcAddr - len;
//				desAddr = desAddr - len;
//		}

//		dataLen -= len;
//	}

//	//fileType = 0;

//	//写文件头
//	if(moveDateBuf[SecureMoveEepromWriteDataLen_Offset] != 0)
//	{
//		desAddr = BuildUWord(&moveDateBuf[SecureMoveEepromWriteDesAddr_Offset]);
//		WriteEepromEX(desAddr, &moveDateBuf[SecureMoveEepromWriteDataBuf_Offset], moveDateBuf[SecureMoveEepromWriteDataLen_Offset]);

//		//fileType = GetFileHeadByte(desAddr,F_Type_Offset);
//	}

//	/*setLength = BuildWord(&moveDateBuf[SecureMoveEepromSetDataLen_Offset]);

//	if(setLength != 0)
//	{
//		desAddr = BuildUWord(&moveDateBuf[SecureMoveEepromSetDesAddr_Offset]);

//		if(fileType == CYCTYPE)
//		{
//			SetEepromEX(desAddr,0x01,0x01);
//			SetEepromEX(desAddr+1,moveDateBuf[SecureMoveEepromSetDataValue_Offset],setLength-1);
//		}
//		else
//		{
//			SetEepromEX(desAddr,moveDateBuf[SecureMoveEepromSetDataValue_Offset],setLength);
//		}
//	}*/

//	//初始化文件内容
//	if(moveDateBuf[SecureMoveEepromInitDataLen_Offset] != 0)
//	{
//		desAddr = BuildUWord(&moveDateBuf[SecureMoveEepromInitDesAddr_Offset]);
//		startOffset = BuildWord(&moveDateBuf[SecureMoveEepromInitStartOffset_Offset]);
//		InitTransOrRecordFile(desAddr,startOffset,&moveDateBuf[SecureMoveEepromInitDataLen_Offset],moveDateBuf[SecureMoveEepromInitHeRecordNo_Offset],NOT_PROFILE_MODE);
//	}
//	
//	SetEepromEX(E_SECUREMOVE_START+SecureMoveEepromTag_Offset, 0xFF, sizeof(BYTE));
//#undef moveDateBuf
//	return ;
//}

//void MoveEepromResume(void)
//{
//	BYTE    idata  len;
//	UWORD xdata srcAddr,desAddr,dataLen;

//#define moveDateBuf  (tempBuf + 100)

//	//memset(moveDateBuf,0x00,SecureMoveStuct_LengthAll);

//	ReadEeprom(moveDateBuf,E_SECUREMOVE_START,SecureMoveStuct_LengthPart);	
//#if (EUICC_ONLY||SE_AND_EUICC)
//	if(moveDateBuf[SecureMoveEepromTag_Offset] == D_EEPMoveTag_FAST)
//	{
//		StartMoveEepromFast();
//		return;
//	}
//#endif
//	if(moveDateBuf[SecureMoveEepromTag_Offset] != D_EEPMoveTag)
//	{
//		return;
//	}

//	srcAddr = BuildUWord(&moveDateBuf[SecureMoveEepromSrcAddr_Offset]);
//	desAddr = BuildUWord(&moveDateBuf[SecureMoveEepromDesAddr_Offset]);
//	dataLen = BuildUWord(&moveDateBuf[SecureMoveEepromMoveLen_Offset]);

//	len = moveDateBuf[SecureMoveEepromLen_Offset];

//	if(len != 0x00)
//	{
//			if(srcAddr > desAddr)	//数据由后向前移动（由高地址向低地址移动）
//			{
//					WriteEepromEX(desAddr, &moveDateBuf[SecureMoveEepromDataBuf_Offset], len);	//始终恢复保护区内的数据到实际的Eeprom地址
//					srcAddr = srcAddr + len;
//					desAddr = desAddr + len;
//			}
//			else
//			{
//					WriteEepromEX(desAddr-len, &moveDateBuf[SecureMoveEepromDataBuf_Offset], len);
//					srcAddr = srcAddr - len;
//					desAddr = desAddr - len;
//			}
//			
//			dataLen = dataLen -len;
//	}
//	
//	StartSecureMoveEeprom(desAddr,srcAddr,dataLen);
//#undef moveDateBuf	
//}

BYTE SetEepromEX(UWORD eepAddr,BYTE Hex,WORD length)
{
	nvmFillNoAtomic((BYTE *)eepAddr, Hex,  length);
	return TRUE;
}

BYTE ReadEECacheByte(UWORD logicStartAddr)
{
	BYTE  idata tag;

	ReadEeprom(&tag,logicStartAddr,sizeof(BYTE));

	return tag;
}

BYTE SetEepromAtomic(UWORD logDstEEPAddr,BYTE hex,WORD size)
{
	nvmFillAtomic((BYTE *)logDstEEPAddr,hex,size);
	return TRUE;
}

BYTE WriteEepromAtomic(UWORD logDstEEPAddr,BYTE * RAMaddr,WORD size)
{
	nvmCopyAtomic((BYTE *)logDstEEPAddr, RAMaddr, size);
	return TRUE;
}

BYTE WriteEepromNonAtomic(UWORD logDstEEPAddr,BYTE * ramAddr,WORD size)
{
	nvmCopyNoAtomic((BYTE *)logDstEEPAddr,ramAddr,size);
	return TRUE;
}

void VMReadBuf(BYTE * destRam, UWORD srcAddr, UWORD length)//VMReadBytes
{
	memCopyToRam(destRam,(BYTE *)srcAddr,length);
//	ReadEeprom(destRam, srcAddr, length);
}

BYTE WriteEepromWithoutProtect(UWORD destAddr, BYTE *srcData, WORD length)
{
	nvmCopyLost(MEMtPN8(destAddr), MEMtPN8(srcData), length);
	return TRUE;
}

BYTE VMWriteBuf(UWORD destAddr, BYTE *srcData, WORD length)
{
	memCopyAtomic((BYTE *)destAddr,srcData,length);

	return TRUE;
}




BYTE IF_BeginTransaction(void)
{
	if (!isInProgress())
	{
		memBeginTransaction(0);
		return TRUE;
	}
	
	return FALSE;

}


void IF_CommitTransaction(BYTE need_commit)
{
  if(need_commit == TRUE)
	{
		memCommitTransaction();
	}		
}

/*@Function:IF_AbortTransaction
 *
 *@Descrip :Abort事务
 *
 *@See     :
 *
 *@Param   :BYTE tag --- Abort的表示，TURE表示需要Abort / FALSE表示不需要Abort
 *
 *@Return  :void
 *
 *@History :create by LGY/2017.11.17
 */
void abortTransaction(void)
{
		memAbortTransaction(ABORT_NOT_ON_RESET);	
}

/*@Function:IF_AbortTransaction
 *
 *@Descrip :Abort事务
 *
 *@See     :
 *
 *@Param   :BYTE tag --- Abort的表示，TURE表示需要Abort / FALSE表示不需要Abort
 *
 *@Return  :void
 *
 *@History :create by LGY/2017.11.17
 */
void IF_AbortTransaction(BYTE tag)
{
	if(tag == TRUE)
	{
		abortTransaction();
	}
}

/*dst < src  return 1 ,dst > src  return -1*/
SBYTE NvmCmp(BYTE * dst,BYTE * src,WORD len)
{
	WORD idata i;
	SBYTE* destAddr; // 这里需要有符号的比较，不要去警告
	SBYTE* srcAddr;

	memRefresh();
	destAddr = memoryToPoint((memref)(dst));
	srcAddr =  memoryToPoint((memref)(src));

	for(i=0;i<len;i++)
	{
		 if (*(destAddr+i) < *(srcAddr+i))
		 {
			  return 1;
		 }
		 else if (*(destAddr+i) > *(srcAddr+i))
		 {
			  return -1;
		  }
	}

	return 0;
}

BYTE VMWriteEepromOrRam(BYTE* destAddr, BYTE* srcAddr, WORD length)
{
	memCopyAtomic(destAddr, srcAddr, length);
	return TRUE;
}
SWORD VMReadShort(UWORD srcAddr)
{
	return (SWORD)memReadU16((PM8)srcAddr);
}

BYTE WritePageCacheEX(UWORD logEEPAddr,BYTE * RAMaddr,WORD Length)
{
    memCopyNoAtomic((PM8)logEEPAddr, (PM8)RAMaddr, Length);

	return TRUE;
}

void VMSetBuf(UWORD destAddr, BYTE hex, WORD length)
{
	memFillAtomic((BYTE *)destAddr, hex,length);
}

//modify by xuweihua-20000514, ???????, ?????????????????
BYTE VMCopyDataToAddr(UWORD destAddr,WORD destOffset,WORD maxSize,UWORD srcAddr,WORD srcOffset,UWORD copyLen, BYTE mode,BYTE isDstFile,BYTE isSrcFile)
{
	WORD  idata onetimeLen;
	BYTE idata result;
	
	if((maxSize != 0x00)&&(copyLen > maxSize))
	{
		return FALSE;
	}
	
	if(isDstFile)
	{
		destAddr = FindTFAddr(destAddr, destOffset, copyLen);
		
		if(destAddr == FALSE)
		{
			return FALSE;
		}
	}
	else
	{
		destAddr = destAddr + destOffset;
	}
	
	if(isSrcFile)
	{		
		srcAddr = FindTFAddr(srcAddr, srcOffset, copyLen);
		
		if(srcAddr == FALSE)
		{
			return FALSE;
		}
	}
	else
	{		
		srcAddr = srcAddr + srcOffset;
	}

	//从前向后拷贝并且数据重叠，为了防止源数据被冲掉，必须改变拷贝的顺序
	if((srcAddr < destAddr)&&((srcAddr + copyLen - 1)>=destAddr))
	{
		destAddr += copyLen;
        srcAddr += copyLen;

		while(copyLen > 0)
		{
			onetimeLen = ((copyLen>0x100)?0x100:copyLen);

			VMReadBuf(tempBuf+0x100, srcAddr -onetimeLen, onetimeLen);

			if(mode == MODE_NO_PROTECT_WRITE)
			{
				result = WriteEepromWithoutProtect(destAddr -onetimeLen, tempBuf+0x100, onetimeLen);
			}
			else
			{
				result = VMWriteBuf(destAddr -onetimeLen, tempBuf+0x100, onetimeLen);
			}

			if(result == FALSE)
			{
				return FALSE;
			}

			srcAddr -= onetimeLen;
			destAddr -=onetimeLen;
			copyLen -= onetimeLen;
		}
	}
	else
	{
		while(copyLen > 0)
		{
			onetimeLen = ((copyLen>0x100)?0x100:copyLen);

			VMReadBuf(tempBuf+0x100, srcAddr, onetimeLen);

			if(mode == MODE_NO_PROTECT_WRITE)
			{
				result = WriteEepromWithoutProtect(destAddr, tempBuf+0x100, onetimeLen);
			}
			else
			{
				result = VMWriteBuf(destAddr, tempBuf+0x100, onetimeLen);
			}
			if(result == FALSE)
			{
				return FALSE;
			}
			srcAddr += onetimeLen;
			destAddr += onetimeLen;
			copyLen -= onetimeLen;
		}
	}

	return TRUE;
}

/*UWORD GetHeapBase(void)
{
	return FreeTable_GetTopFreeNvm();//gFreeTable_lastFreeBlockTop;
}*/

#if !JAVA_CARD
/*void HMACSha256DoFinalSingleCall(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest)
{
	BYTE idata tmpBuff[SHA256_BLOCK_SIZE+1];
	BYTE idata mdBuff[SHA256_ICV_SIZE+2];
//	BYTE idata hMacGenKeyBuffer[keyLen+1];
	BYTE idata hMacGenKeyBuffer[HASH_BLOCK_SIZE + 4];
	
	HMACInit(MEMtPM8(keyBuf), U8t16(keyLen), MEMtPM8(tmpBuff), MEMtPM8(mdBuff), MEMtPM8(hMacGenKeyBuffer), ALG_HMAC_SHA_256);

	HMACDofinal(MEMtPM8(src), srcLgth, MEMtPM8(dest), MEMtPM8(mdBuff), MEMtPM8(tmpBuff), MEMtPM8(hMacGenKeyBuffer), ALG_HMAC_SHA_256);
}*/
#endif
void SetEepromNonAtomic(UWORD logDstEEPAddr,BYTE Value,UWORD size)
{
	nvmFillNoAtomic(MEMtPN8(logDstEEPAddr),Value,size);
}

/*
*@function     :IsUiccApp
*
*@description  :If is UICC app
*
*@see          :
*
*@param        :NULL
*
*@return       :TRUE:is UICC app/FALSE:not UICC app
*
*
*@author       :xwh@20220519
*
*@hisotry      :created by xwh@20220519
*              
*/
BYTE IsUiccApp(BYTE  channelNum)
{
#if JAVA_CARD_UICC	
	if(U8_AND_U8(U16t8(getChannelContext(channelNum)), 0xFF)==UICC_APPID)	
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
#else
	return TRUE;
#endif
}


void setChannelNum(BYTE cla)
{
	G_ChannelNum = (cla&0x03);
}

#if BOOT__IO__SWP
void TerminalCapabilityProcess(BYTE * buf)
{
	BYTE  idata pos;

	if(buf[0] == 0xA9)	                        //Terminal capability tag 
	{
		pos = FindTag(&buf[2],0x82,buf[1]);	    //Additional interface support

		if(pos != FALSE)
		{
			if((buf[2+pos+1]&0x01)==0x00)	    //UICC-CLF interface not supported
			{
				swp_PullDownManagementS2();
			}
		}
	}
	
}
#endif




#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
BYTE WriteHEFlashPage(UWORD updateAddr,BYTE * RamBuffer)
{
	BYTE  idata i,result;
	
	if(updateAddr == FALSE)
	{
		return FALSE;
	}
	
	for(i=0;i<5;i++)
	{	
#ifdef HESEC
		openWrSwitch();		
		result = HED_HEFlashEWPage_SEC(updateAddr,(UWORD *)RamBuffer);
		closeWrSwitch();
#else	
		openWrSwitch();
		result = HED_HEFlashEWPage(updateAddr,(UWORD *)RamBuffer);
		closeWrSwitch();
#endif			
		if(result != 0)
		{
			continue;
		}
		else
		{
			break;
		}		
	}
	
	if(i >= 5)
	{
		return FALSE;
	}

	return TRUE;
}
WORD Hardware_CalCRC(BYTE * ptr, UWORD count)
{
	WORD  idata crc,calcCrc;
	
	crc = HED_CRC16(0xFFFF,count,ptr, 0x01);
	
	calcCrc = ((crc<<8)&0xFF00)|((crc>>8)&0xFF);
	 
	return calcCrc;   
}
#endif