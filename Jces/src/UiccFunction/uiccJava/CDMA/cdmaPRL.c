
/*
*@HISTORY       :V1.0: 创建源文件
*				 V1.1：修改函数WritePRLData，兼容小端芯片					--by yangjian 2015.07.14
*/



#include <string.h>
#include "uicc_main.h"
#include "gsmpublic.h"
#include "fileSystem.h"
#include "stkhead.h"
#include "fileAction.h"
#include "ceph.h"
#include "uicc_error.h"
//#include "resume.h"
#include "CDMA.h"
//#include "PRL.h"
#include "ToolkitRfm.h"
#include "Stkhead.h"
//#include "CatTp.h"
#include "Stkalert.h"


#if (UICC__CDMA__CATTP__PRL||UICC__CDMA__SMS__PRL)
#include "CatTp.h"

//WORD xdata VersionNumber;
BYTE xdata G_EPRL_PROCING;
BYTE xdata G_PRL;     //是否PRL更新
#endif

#if UICC__CDMA__CATTP__PRL
BYTE xdata G_CATTPCurrentTotalNumber;

BYTE xdata G_CATTPPreNumber;

BYTE xdata G_CATTPTar[0x03];//存放CATTP中当前正在执行PRL更新的TAR值

BYTE code PRLSuccess[]={
	// 0   PRL更新成功   0   15
	16,0x00,0x50,0x00,0x52,0x00,0x4C,0x66,0xF4,0x65,0xB0,0x62,0x10,0x52,0x9F,0xFF,0x01,
};
BYTE code EPRLSuccess[]={
	// 0   EPRL更新成功!   0   16
	18,0x00,0x45,0x00,0x50,0x00,0x52,0x00,0x4C,0x66,0xF4,0x65,0xB0,0x62,0x10,0x52,0x9F,0xFF,0x01,
};
#endif
#if (UICC__CDMA__CATTP__PRL||UICC__CDMA__SMS__PRL)
/**
 * @brief 	calculate the CRC
 * 
 * @param 	iniData 
 * @param 	ptr: PRL string 
 * @param 	count 
 * 
 * @Returns	:
 * 
 * @author 	?
 * 
 * @history	tested by created by 
 */
 
WORD PRLcrc(WORD iniData, BYTE *ptr, WORD count)
{
	WORD idata crc, i, j;

	crc = iniData;

	for (j=0;j<(count);j++)
	{
		crc = crc ^ (unsigned short)*ptr++ << 8;

		for (i = 0; i < 8; ++i)
		{
			if (crc & 0x8000)
			{
				crc = crc << 1 ^ 0x1021;
			}
			else
			{
				crc = crc << 1;
			}
		}
	}

	return (crc & 0xFFFF);
}
#endif

#if UICC__CDMA__SMS__PRL
/**
 * @brief 	to see if the SMS is a PRL SMS
 * 
 * @param 	str: data string 
 * @param 	pos: data offset 
 * 
 * @Returns	:
 * 
 * @author 	?
 * 
 * @history	tested by 
 */
BYTE IsPRLSMS(BYTE *str, WORD pos)
{
	BYTE idata len;

	if ((str[0] == 0xD1)&&(str[1] == 0x81))
	{
		len = str[2];
	}
	else
	{
		len = str[1];
	}

	while (str[pos] != 0x0F)    //在BEARER的TLV中，找到 信息显示模式 TLV
	{
		pos = pos + str[pos+1]+2;

		if ( pos > (len+1))
		{
			return FALSE;
		}
	}

	//增加对EPRL(D1,F1)的处理，by xwh 20150419
	G_EPRL_PROCING = FALSE;

	if ((str[pos+2] == 0xD0) || (str[pos+2] == 0xF0) || (str[pos+2] == 0xD1) || (str[pos+2] == 0xF1)) 	//保留字段是PRL
	{
		if((str[pos+2] == 0xD1)||(str[pos+2] == 0xF1))
		{
			G_EPRL_PROCING = TRUE;
		}

		str[pos+2] = 0x00;
	
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
#endif
#if (UICC__CDMA__CATTP__PRL||UICC__CDMA__SMS__PRL)
/**
 * @brief 	store the first PRL data to the file 6FFF
 * 
 * @param 	receiveSms: data string
 * @param 	fileHeadAddr: EF_6FFF 
 * 
 * @Returns	:0xFF(last num)/0x00(error)/(0x01-0x0f) already stored num
 * 
 * @author 	XWH
 * 
 * @history	tested by 
 *            V0.1---modify by xwh 20220816,优化空间，EF_PRL_SMS文件去掉，复用EF_CATTPSave文件后半部分
 */
BYTE storePRLData(BYTE  * receiveSms, UWORD fileHeadAddr,WORD fileSize)
{
	BYTE idata CNum, TNum, ReceiveNum;
	BYTE idata PRLLen, PRLTag, i;
	//WORD  idata fileSize;
	UWORD xdata fileDataAddr;
	PRLSMSFILEHEAD xdata PRLSmsFileHead;
	// current num
	CNum = (receiveSms[0]&0xF0) >> 4;

	ReceiveNum = (receiveSms[0]&0xF0) >> 4;

	TNum = receiveSms[0]&0x0F;	                                               // total num

	//fileSize用参数传入，不用这里读取
	//ReadEeprom((BYTE *)&fileSize, fileHeadAddr-sizeof(TranFileHead)+F_SpaceSize_Offset, sizeof(WORD)); //retrieve the length of the whole file, don't include the file head

	ReadEeprom((BYTE *)&PRLSmsFileHead, fileHeadAddr, sizeof(PRLSMSFILEHEAD));

	if ((PRLSmsFileHead.StoreNum != 0) && (PRLSmsFileHead.TotleNum == PRLSmsFileHead.StoreNum))
	{
		return IS_LAST; 														// receive all
	}

	if ((fileSize-sizeof(PRLSMSFILEHEAD)-PRLSmsFileHead.UsedSpace) < (WORD)(receiveSms[3]+4))
	{
		return FALSE;
	}

	fileDataAddr = fileHeadAddr + sizeof(PRLSMSFILEHEAD);

	if (TNum == PRLSmsFileHead.TotleNum)
	{
		for (i=0; i<PRLSmsFileHead.StoreNum; i++)
		{
			ReadEeprom((BYTE *)&PRLTag, fileDataAddr, sizeof(BYTE));

			ReadEeprom((BYTE *)&PRLLen, fileDataAddr+3, sizeof(BYTE));

			if (PRLTag == CNum)
			{
				if (receiveSms[3] == PRLLen) 		// if size EQUAL
				{
					receiveSms[0] = CNum ;
					WriteEepromEX(fileDataAddr, (BYTE *)&receiveSms[0], PRLLen+4); 				// replace the old one with the new one
				}

				return (ReceiveNum);
			}

			fileDataAddr = fileDataAddr + PRLLen + 4 ;
		}
	}

	if (TNum != PRLSmsFileHead.TotleNum)                                     // other pached data(the first)
	{
		if (CNum == 0x01)
		{
			PRLSmsFileHead.StoreNum = 0 ;

			PRLSmsFileHead.UsedSpace = 0 ;
		}
		else if (PRLSmsFileHead.StoreNum != 0x00)
		{
			//return(PRLSmsFileHead.StoreNum);
			return (ReceiveNum);
		}

	}
//	if (CNum ==0x01)
//	{
//		VersionNumber = ((receiveSms[6]<<8)&0xFF00) + receiveSms[7]	;
//	}

	fileDataAddr = fileHeadAddr + sizeof(PRLSMSFILEHEAD)+PRLSmsFileHead.UsedSpace;

	receiveSms[0] = CNum ;

	WriteEepromEX(fileDataAddr, &receiveSms[0], receiveSms[3] +4);                   // store the PRL data(TLV)

	PRLSmsFileHead.TotleNum = TNum ;

	PRLSmsFileHead.StoreNum = PRLSmsFileHead.StoreNum + 1 ;                                                     // set the store num

	PRLSmsFileHead.UsedSpace = PRLSmsFileHead.UsedSpace + receiveSms[3] + 4;          //add the usedsize

	WriteEepromEX(fileHeadAddr, (BYTE *)&PRLSmsFileHead, sizeof(PRLSMSFILEHEAD));

	if (PRLSmsFileHead.TotleNum == PRLSmsFileHead.StoreNum)
	{
		return IS_LAST;
	}

	else return (ReceiveNum);
}


/**
 * @brief 	read out  the PRL data to the buffer ResData
 * 
 * @param 	fileHeadAddr 
 * @param 	resultOut 
 * 
 * @Returns	:
 * 
 * @author 	?
 * 
 * @history 	created by XWH
 */
WORD PRLcrcFromEeprom(UWORD fileHeadAddr, BYTE * resultOut)
{
	WORD idata CRCResult, nextoffset;
	BYTE idata i, j, PRLLen, PRLTag, PRLNum, offset[2];
	UWORD xdata fileDataAddr;

	ReadEeprom((BYTE *)&PRLNum, fileHeadAddr+1, 1);
	nextoffset = 0;
	offset[0]= 0;
	offset[1]= 0;
	CRCResult = 0x00; //去掉警告

	for (i=0; i<PRLNum;i++)
	{
		fileDataAddr = fileHeadAddr + sizeof(PRLSMSFILEHEAD);

		for (j=0;j<PRLNum;j++)
		{
			ReadEeprom((BYTE *)&PRLTag, fileDataAddr, 1);                        // CNum

			ReadEeprom((BYTE *)&PRLLen, fileDataAddr+3, 1);                      // PRLLen

			if (PRLTag == (i+1))                                                     // find the CNum
			{
				ReadEeprom((BYTE *)&offset[0], fileDataAddr+1, 2); //读取每条记录的偏移

	 			if (nextoffset!=((WORD)(offset[0]<<8)+offset[1]))
				{
					return FALSE;
				}
				
				nextoffset += PRLLen;
				
				ReadEeprom((BYTE *)&resultOut[0], fileDataAddr+4, PRLLen);            // PRL data

				if (PRLTag == 0x01)
				{
					if (PRLTag == PRLNum)
					{
						CRCResult = PRLcrc(0xFFFF, &resultOut[0], PRLLen-2);

						ecpMem_Move((BYTE *)&resultOut[0], (BYTE *)&resultOut[PRLLen-2], sizeof(WORD));
					}
					else
					{
						CRCResult = PRLcrc(0xFFFF, &resultOut[0], PRLLen);
					}
				}
				else if (PRLTag == PRLNum)
				{
					CRCResult = PRLcrc(CRCResult, &resultOut[0], PRLLen-2);

					ecpMem_Move((BYTE *)&resultOut[0], (BYTE *)&resultOut[PRLLen-2], sizeof(WORD));
				}
				else
				{
					CRCResult = PRLcrc(CRCResult, &resultOut[0], PRLLen);
				}

				break;
			}

			fileDataAddr = fileDataAddr + PRLLen + 4;
		}

		if (j == PRLNum)																//if not find the number of sms
		{
			return FALSE;
		}

	}

	CRCResult = ~CRCResult;

	return CRCResult;
}

/**
 * @brief 	read out  the PRL data to the buffer ResData
 * 
 * @param 	writeFileAddr 
 * @param 	tempBuf 
 * @param 	fileSize 
 * 
 * @Returns	:
 * 
 * @author 	?
 * 
 * @history 	created by xwh
 *            V0.1---modify by xwh 优化空间，EF_PRL_SMS文件去掉，复用EF_CATTPSave文件后半部分
 *            V0.2---modify by xwh 20220816, 增加对usedSize大小的判断，防止越界
 */
BYTE WritePRLData(UWORD writeFileAddr, BYTE * userData, WORD fileSize)
{
	WORD  idata usedSize, Offset;
	UWORD xdata fileHeadAddr;
	UWORD xdata fileDataAddr;
	BYTE  idata i, j, PRLLen, PRLTag, PRLNum;
	BYTE  idata tem[2];
	

	//fileHeadAddr = SelectTFFile(DF_CDMA, EF_PRL_SMS);
#if UICC__CDMA__PRL__REDUCE_SIZE
	fileHeadAddr = SelectPrivateTFFile(DF_CATTP,EF_CATTPSave);
	
	if (fileHeadAddr == FALSE)
	{
		return FALSE ;
	}
	
	usedSize = GetTFSize(fileHeadAddr);
	
	if(usedSize < PRL_SMS_SIZE)
	{
		return FALSE ;
	}
	
	usedSize = usedSize - PRL_SMS_SIZE;
	
	fileHeadAddr = fileHeadAddr + sizeof(TranFileHead) + usedSize;
#else	
	fileHeadAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum], EF_PRL_SMS);

	if (fileHeadAddr == FALSE)
	{
		return FALSE ;
	}

	fileHeadAddr = fileHeadAddr + sizeof(TranFileHead);
#endif
	ReadEeprom((BYTE *)&PRLNum, fileHeadAddr+1, 1);

	usedSize = 0x00;

	for (i=0; i<PRLNum; i++)
	{
		fileDataAddr = fileHeadAddr + sizeof(PRLSMSFILEHEAD);

		for (j=0; j<PRLNum; j++)
		{
			ReadEeprom((BYTE *)&PRLTag, fileDataAddr, 1);

			ReadEeprom((BYTE *)&tem[0], fileDataAddr+1, sizeof(WORD));

	 		Offset=(((WORD)tem[0]<<8)+tem[1]);		  			//因为前面的offset值以大端放式写入，固要以大端方式恢复 
							  
			ReadEeprom((BYTE *)&PRLLen, fileDataAddr+3, 1);

			if (PRLTag ==(i+1))                                                     // find the CNum
			{
				ReadEeprom((BYTE *)&userData[0x00], fileDataAddr+4, PRLLen);

				if ((Offset + PRLLen) > fileSize)
				{
					return FALSE;
				}

				WriteEepromEX(writeFileAddr+Offset, (BYTE *)&userData[0x00], PRLLen);

				usedSize += PRLLen; // add by WG 20070510 -- 级连PRL, 长度为两个版本的PRL总和

				break;
			}

			fileDataAddr = fileDataAddr + PRLLen + 4;
		}

		if (j == PRLNum)																//if not find the number of sms
		{
			return FALSE;
		}

	}

	userData[0] = (usedSize&0xFF00)>>8;

	userData[1] = usedSize&0x00FF;

	return TRUE;
}


/**
 * @brief       modify gsm buffer's data to prl update sucess/(n/m)
 * 
 * @param 	SMSstr 
 * @param 	UserData 
 * @param 	len 
 * @param 	savebit 
 * 
 * @author 	XWH
 * 
 * @history 	created by XWH
 */
void UpdateSMS(BYTE * SMSstr, BYTE * UserData, BYTE len, BYTE savebit)
{
	BYTE idata pos, length, ll;
	BYTE idata bpos, upos;

	UserData[0x00] = 0x20 ;

	ShiftDataPlus(&UserData[0x02], len, &UserData[0x00], 5);

	if (savebit < 5)
	{
		len =len + 1;                                 // new user data length
	}

	pos = 3;

	while ( SMSstr[pos] != 0x08 )
	{
		pos = pos + SMSstr[pos+1]+2;

		if (pos > (SMSstr[1]+1))
		{
			return   ;
		}
	}

	bpos = pos;

	pos = pos + 2;

	while ( SMSstr[pos] != 0x01 )
	{
		pos = pos + SMSstr[pos+1]+2;

		if (pos > (SMSstr[1]+1))
		{
			return   ;
		}
	}

	upos = pos;

	length = SMSstr[pos+1];                                // old user data length

	pos = pos + 2;

	ll = SMSstr[1] - length - pos + 2;

	ecpMem_Move(SMSstr+pos+len, SMSstr+pos+length, ll);

	ecpMem_Copy(SMSstr+pos, &UserData[0x00], len);

	pos = pos + len;

	pos = pos + ll;

	SMSstr[upos+1] = len;

	//SMSstr[bpos+1] = pos - bpos-2;
	SMSstr[bpos+1] = SMSstr[bpos+1] - length + len;    //修改BearData length计算方式, by xwh 20190830

	SMSstr[1] = pos -2;

	ecpMem_Set(SMSstr+pos, 0, 255-pos);
	

}

/**
 * @brief      	show prompt message
 * 
 * @param	UserData 
 * @param 	len 
 * 
 * @author 	
 * 
 * @history 	created by 
 */

void WritePRLDisplayText(BYTE * UserData)
{
	
	WriteVariable(TAG_PRL_DISPLAYSTR,UserData);

	PendingMessageAlert(D_PRLDisplayTag,0);
}


/**
 * @brief       show prompt message
 * 
 * @author	
 * 
 * @history 	created by 
 */
void ShowPRLInfo(void)
{
	//UWORD xdata  fileAddr;
	BYTE  idata  len;
/*
	fileAddr = SelectTFFile(DF_CDMA, 0x6FFE);

	ReadTFBuf(fileAddr, 0, &len, sizeof(BYTE));

	if ((len == 0x00) || (len >= 51))
	{
		return;
	}

	ReadTFBuf(fileAddr, sizeof(BYTE), &RecData[3], len);
*/
	len = ReadVarible(TAG_PRL_DISPLAYSTR,&RecData[3]);
	
	ClearVariable(TAG_PRL_DISPLAYSTR);
	
	if ((len == 0x00) || (len >= 51))
	{
		return;
	}

	InitProactiveCommand(CMD_DISPLAYTEXT, Q_DISPLAY_CLEAR, DEV_DISPLAY);

	RecData[0] = 0x8D;

	RecData[1] = len + 1 ;

	RecData[2] = 0x08;

	AddTLV(&ResData[1], &RecData[0]);

	//STKPerform[0] = TRUE;
	SendProactiveCommand();

	return;
}

#endif

#if UICC__CDMA__SMS__PRL
BYTE code  PRLInfoSucc[] = {0x00, 0x50, 0x00, 0x52, 0x00, 0x4C, 0x66, 0xF4, 0x65, 0xB0, 0x62, 0x10, 0x52, 0x9F};
BYTE code  PRLInfoProc[] = {0x59, 0x04, 0x74, 0x06, 0x4E, 0x2D, 0x76, 0x84, 0x00, 0x50, 0x00, 0x52, 0x00, 0x4C, 0x66, 0xF4, 0x65, 0xB0};
BYTE code  PRLInfoError[] = {0x00, 0x50, 0x00, 0x52, 0x00, 0x4C, 0x66, 0xF4, 0x65, 0xB0, 0x51, 0xFA, 0x95, 0x19, 0xFF, 0x0C, 0x80, 0x54, 0x7C, 0xFB, 0x5B, 0xA2, 0x62, 0x37, 0x67, 0x0D, 0x52, 0xA1};

BYTE code  EPRLInfoSucc[] = {0x00,0x45,0x00,0x50,0x00,0x52,0x00,0x4C,0x66,0xF4,0x65,0xB0,0x62,0x10,0x52,0x9F};
BYTE code  EPRLInfoProc[] = {0x59,0x04,0x74,0x06,0x4E,0x2D,0x76,0x84,0x00,0x45,0x00,0x50,0x00,0x52,0x00,0x4C,0x66,0xF4,0x65,0xB0};
BYTE code  EPRLInfoError[] = {0x00,0x45,0x00,0x50,0x00,0x52,0x00,0x4C,0x66,0xF4,0x65,0xB0,0x51,0xFA,0x95,0x19,0xFF,0x0C,0x80,0x54,0x7C,0xFB,0x5B,0xA2,0x62,0x37,0x67,0x0D,0x52,0xA1};

/**
 * @brief       update the PLMNList
 * 
 * @param 	str 
 * @param 	recNo 
 * 
 * @Returns	TPDA
 * 
 * @author 	
 * 
 * @history 	created by 
 *            V0.1---modify by xwh 优化空间，EF_PRL_SMS文件去掉，复用EF_CATTPSave文件后半部分
 *            V0.2---modify by xwh 20220816, 增加对usedSize大小的判断，防止越界
 */
BYTE UpdatePLMNList(BYTE * str, BYTE recNo)
{
	BYTE  idata  messageLen, i, copyTimes, tag;
	BYTE  idata  TNum, SNum, pos;
	BYTE  idata  IsPRL, status;
	BYTE  idata  moveLen, beaDtaLenOffset;	//added by tl
	WORD  xdata  crcResult, ClearLength;
	WORD  xdata  datalen, len, prllen;
	UWORD xdata  Addr;
	UWORD xdata  fileHeadAddr, smsFileAddr;
	BYTE  xdata  smsBuf[50];
	WORD  idata  usedSize;
#if(!UICC__CDMA_2G_NOT_SUPPORT)
	smsFileAddr = SelectLFFile(DF_CDMA, EF_SMS);
#else
	smsFileAddr = SelectLFFile(G_DF_ID_CSIM, EF_SMS);
#endif
	if (smsFileAddr == FALSE)
	{
		return FALSE;
	}

	len = GetLFRecNumLgth(smsFileAddr, FileLgth);                               // length of record

	tag = RecData[0];

	if (tag == 0xD1)
	{
		pos = 0;                                                                // encoding+length
	}
	else
	{
		pos = 3;

		while ( str[pos] != 0x08 )
		{
			pos = pos + str[pos+1]+2;

			if (pos > (str[1]+1))
			{
				return  FALSE ;
			}
		}
	
		pos = pos + 2;
		
		IsPRL = IsPRLSMS(str, pos);
		
		if ( IsPRL == FALSE )
		{
			return FALSE;
		}

		while ( str[pos] != 0x01 )
		{
			pos = pos + str[pos+1]+2;

			if (pos > (str[1]+1))
			{
				return  FALSE ;
			}
		}

		pos = pos +2;	
	}

	pos = pos + 2;															//pos pointer to CNum/TNum

	//fileHeadAddr = SelectTFFile(DF_CDMA, EF_PRL_SMS);
#if UICC__CDMA__PRL__REDUCE_SIZE
	fileHeadAddr = SelectPrivateTFFile(DF_CATTP,EF_CATTPSave);
	
	if (fileHeadAddr == FALSE)
	{
		return FALSE ;
	}
	
	usedSize = GetTFSize(fileHeadAddr);
	
	if(usedSize < PRL_SMS_SIZE)
	{
		return FALSE;
	}
	
	usedSize = usedSize - PRL_SMS_SIZE;
	
	fileHeadAddr = fileHeadAddr + sizeof(TranFileHead) + usedSize;
	
	usedSize = PRL_SMS_SIZE;
#else		
	fileHeadAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum], EF_PRL_SMS);

	if (fileHeadAddr == FALSE)
	{
		return FALSE ;
	}
	
	usedSize = GetTFSize(fileHeadAddr);

	fileHeadAddr = fileHeadAddr + sizeof(TranFileHead);
#endif
	SNum = storePRLData(&str[pos], fileHeadAddr,usedSize);

	if (SNum == FALSE)
	{
		return FALSE;
	}
	else if (SNum != IS_LAST)
	{
		prllen = 0x00;

		if (SNum > 0x09)
		{
			smsBuf[0x03] = 0x00;
			smsBuf[0x04] = ((SNum/10)&0x0F) | 0x30;
			smsBuf[0x05] = 0x00;
			smsBuf[0x06] = ((SNum%10)&0x0F) | 0x30;
			prllen +=4;
		}
		else
		{
			smsBuf[0x03]= 0x00;

			smsBuf[0x04] = (SNum&0x0F) | 0x30;

			prllen +=2;
		}
		smsBuf[prllen+0x03]= 0x00;

		smsBuf[prllen+0x04] = 0x2F;
		// '/'
		prllen += 2;

		smsBuf[prllen+0x03]= 0x00;

		ReadEeprom((BYTE *)&TNum, fileHeadAddr, 1);

		if (TNum>0x09)
		{
			smsBuf[prllen+0x04] = ((TNum/10)&0x0F)| 0x30;
			smsBuf[prllen+0x05] = 0x00;
			smsBuf[prllen+0x06] = ((TNum%10)&0x0F)| 0x30;
			prllen += 4;
		}
		else
		{
			smsBuf[prllen+0x04] = (TNum&0x0F) | 0x30;
			prllen += 2;

		}

		if(G_EPRL_PROCING == TRUE)
		{
			messageLen = sizeof(EPRLInfoProc);
			ecpMem_Copy(&smsBuf[prllen+3], (BYTE *)EPRLInfoProc, messageLen);
		}
		else
		{
			messageLen = sizeof(PRLInfoProc);
			ecpMem_Copy(&smsBuf[prllen+3], (BYTE *)PRLInfoProc, messageLen);
		}
		messageLen += prllen;

		smsBuf[0x02] = messageLen/2;

		if (tag == 0xD1)
		{			
			smsBuf[0x02] = messageLen;
			WritePRLDisplayText(&smsBuf[0x02]);                                         // show message "PRL Update In Progress X/Y"
		}
		else
		{
			UpdateSMS(str, &smsBuf[0x00], messageLen+1, 0);

//add		
			pos = 3;
	
			while ( str[pos] != 0x08 )
			{
				pos = pos + str[pos+1]+2;
	
				if (pos > (str[1]+1))
				{
					return  FALSE ;
				}
			}
		
			beaDtaLenOffset = pos + 1;		//used for deleate the display mode			

			pos = pos + 2;

			while (str[pos] != 0x0F)    //在BEARER的TLV中，找到 信息显示模式 TLV
			{
				pos = pos + str[pos+1]+2;
		
				if ( pos > (str[1]+1))
				{
					return FALSE;
				}
			}

			//moveLen = str[beaDtaLenOffset] - (pos + 2 - beaDtaLenOffset);	
			moveLen = str[1] + 2 - (pos+3);				
			
			if (str[pos+2] == 0x00) 	//保留字段是PRL
			{
				ecpMem_Move(&str[pos],&str[pos+3],moveLen);

				ecpMem_Set(&str[pos+moveLen], 0x00, 0x03);
		
				str[beaDtaLenOffset] -= 3;
				
				str[1] -= 3;
			}
//end	
		}

		return FALSE;
	}

	if (tag != 0xD1)
	{
		if(G_EPRL_PROCING == TRUE)
		{
			messageLen = sizeof(EPRLInfoSucc);
			
			ecpMem_Copy(&smsBuf[0x03], (BYTE *)EPRLInfoSucc, messageLen);
		}
		else
		{
			messageLen = sizeof(PRLInfoSucc);
			
			ecpMem_Copy(&smsBuf[0x03], (BYTE *)PRLInfoSucc, messageLen);
		}
		smsBuf[0x02] = messageLen/2;

		UpdateSMS(str, &smsBuf[0x00], messageLen+1, 0);
//add		
		pos = 3;

		while ( str[pos] != 0x08 )
		{
			pos = pos + str[pos+1]+2;

			if (pos > (str[1]+1))
			{
				return  FALSE ;
			}
		}

		beaDtaLenOffset = pos + 1;		//used for deleate the display mode
			
		pos = pos + 2;

		while (str[pos] != 0x0F)    //在BEARER的TLV中，找到 信息显示模式 TLV
		{
			pos = pos + str[pos+1]+2;
	
			if ( pos > (str[1]+1))
			{
				return FALSE;
			}
		}

		//moveLen = str[beaDtaLenOffset] - (pos + 2 - beaDtaLenOffset);		
		moveLen = str[1] + 2 - (pos+3);	

		if (str[pos+2] == 0x00) 	//保留字段是PRL
		{
			ecpMem_Move(&str[pos],&str[pos+3],moveLen);
			
			ecpMem_Set(&str[pos+moveLen], 0x00, 0x03);
			
			str[beaDtaLenOffset] -= 3;
			
			str[1] -= 3;
		}
//end			

		WriteLFBuf(smsFileAddr, recNo, 0, str, len);
	}

	crcResult = PRLcrcFromEeprom(fileHeadAddr, &RecData[0]);

	Addr = (RecData[0] << 8) | (RecData[1]);

	if ( crcResult != Addr)                                                  // the CRC calculation is incorrect
	{
		if(G_EPRL_PROCING == TRUE)
		{
			messageLen = sizeof(EPRLInfoError);
			
			ecpMem_Copy(&smsBuf[0x03], (BYTE *)EPRLInfoError, messageLen);

		}
		else
		{
			messageLen = sizeof(PRLInfoError);
			
			ecpMem_Copy(&smsBuf[0x03], (BYTE *)PRLInfoError, messageLen);
		}

		smsBuf[0x02] = messageLen/2;

		if (tag == 0xD1)
		{
			smsBuf[0x02] = messageLen;
			WritePRLDisplayText(&smsBuf[0x02]);                          // show message "PRL Update Error, Contact Custcomer Services"
		}
		else
		{
			ReadLFBuf(smsFileAddr, recNo, 0, str, len);

			UpdateSMS(str, &smsBuf[0x00], messageLen+1, 0);
		}

		SetEepromEX(fileHeadAddr, 0x00, sizeof(PRLSMSFILEHEAD));

		return FALSE;
	}

	while (crcResult != 0x00)
	{
		crcResult --;

		if (crcResult == 0x788)
		{
			crcResult = 0x00;
		}
	}
		  
	//Addr = SelectTFFile(DF_CDMA, EF_PRL_UPDATEFILE);
	//更新当前DF或者ADF下的6F30文件，by xwh 20150419
	//增加对EPRL(D1)的处理，by xwh 20150419
	if(G_EPRL_PROCING == TRUE)
	{
		Addr = SelectTFFile(G_CurrentDfName[G_ChannelNum], 0x6F5A);
	}
	else
	{
		Addr = SelectTFFile(G_CurrentDfName[G_ChannelNum], EF_PRL_UPDATEFILE);

	}

	if (Addr == FALSE)
	{
		return FALSE ;
	}

	ReadEeprom((BYTE  *)&crcResult, Addr+F_SpaceSize_Offset, sizeof(WORD));

	Addr = Addr + sizeof(TranFileHead);

	status = WritePRLData(Addr, (BYTE  *)&RecData[0x00], crcResult);

	if (status == FALSE)
	{
		return FALSE;
	}

	datalen = (RecData[0]<<8) | (RecData[1]);

	if (datalen > crcResult)
	{
		return FALSE;
	}

	ClearLength = crcResult - datalen;

	copyTimes = ClearLength/(0xF0);

	for (i=0;i<copyTimes;i++)
	{
		SetEepromEX(Addr+datalen+i*0xF0, 0x00, 0xF0); 	                         // copy the PRL data to the file EF 6F30
	}

	SetEepromEX(Addr+datalen+copyTimes*0xF0, 0x00, ClearLength%(0xF0)); 	//copy the rest data

	if(G_EPRL_PROCING == TRUE)
	{
		messageLen = sizeof(EPRLInfoSucc);
		
		ecpMem_Copy(&smsBuf[0x03], (BYTE *)EPRLInfoSucc, messageLen);
	}
	else
	{
		messageLen = sizeof(PRLInfoSucc);
		
		ecpMem_Copy(&smsBuf[0x03], (BYTE *)PRLInfoSucc, messageLen);
	}

	smsBuf[0x02] = messageLen/2;

	if (tag == 0xD1)
	{
		smsBuf[2] = messageLen;
		WritePRLDisplayText(&smsBuf[0x02]);                                       // show message "PRL Update Error, Contact Custcomer Services"
	}
	else
	{
		ReadLFBuf(smsFileAddr, recNo, 0, str, len);
	}

	SetEepromEX(fileHeadAddr, 0x00, sizeof(PRLSMSFILEHEAD));
	
	return TRUE;
}
#endif
#if UICC__CDMA__CATTP__PRL
/*
*@description:
*
*@param
*
*@index
*/
WORD CATTPDataIsReceived(BYTE index)
{
	UWORD xdata fileAddr;
	BYTE  idata i;
	WORD idata usedSpace,totalUsedSpace,offset;
	
	fileAddr = SelectPrivateTFFile(DF_CATTP,EF_CATTPSave);
	
	if(fileAddr == FALSE)  //缓存文件出错
	{
		return FALSE;
	}
	
	ReadTFBuf(fileAddr, 0x00, (BYTE *)&totalUsedSpace,sizeof(WORD));
	
	for(offset = 0x02;offset < totalUsedSpace; offset +=usedSpace+3)
	{
		ReadTFBuf(fileAddr,offset,(BYTE*)&i,sizeof(BYTE));
		
		if(i == index)
		{
			return offset;
		}
		
		ReadTFBuf(fileAddr,offset+1,(BYTE*)&usedSpace,sizeof(WORD));
		
	}
	
	return FALSE;
}
/*
*@description
*
*@param
*
*@history
*/
BYTE CATTPSavePRLData(UWORD dataAddr, WORD dataOffset,WORD len,BYTE index)
{
	UWORD xdata fileAddr;
	WORD idata offset,usedSpace,dataLen;
	
	fileAddr = SelectPrivateTFFile(DF_CATTP,EF_CATTPSave);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	dataLen = len;  //备份数据长度
	
	ReadTFBuf(fileAddr, 0x00, (BYTE *)&usedSpace,sizeof(WORD));
	
	WriteTFBuf(fileAddr,usedSpace+2,&index, sizeof(BYTE));	              //写 序号
	
	WriteTFBuf(fileAddr,usedSpace+3,(BYTE *)&dataLen, sizeof(WORD));	  
	
	offset = 0x05;
	
// 	while(	dataLen > 0)
// 	{
// 		saveLen = (dataLen <= 240)?dataLen:240;  //每次读取240字节进行保存
// 		
// 		ReadTFBuf(dataAddr,dataOffset,&tempBuf[0], saveLen);

// 		WriteTFBuf(fileAddr,usedSpace+offset,&tempBuf[0], saveLen);
// 		
// 		offset += saveLen;
// 		
// 		dataOffset += saveLen;

// 		dataLen -= saveLen;
// 	}

	//数据不重叠，可以调用WriteTFFileNonAtomic
	WriteTFFileNonAtomic(fileAddr,usedSpace+offset,dataAddr+sizeof(TranFileHead)+dataOffset,dataLen);
	
	usedSpace += len+3;  //总长度变长
	
	WriteTFBuf(fileAddr, 0x00, (BYTE *)&usedSpace,sizeof(WORD));
	
	return TRUE;
}
/*
*@description:将缓存中EF_CATTPSave的数据更新到PRL 文件中，主要用于PRL 处理和PRL恢复的过程
*
*@function   :UpdatePRLData
*
*@param      :UWORD prlAddr
*@param      :UWORD fileAddr
*
*@return     :void
*
*@author     :xuyz
*
*@history    :V1.0:create @20160818
*/
void UpdatePRLData(UWORD prlAddr, UWORD fileAddr)
{
	
	WORD idata offset,sduLen,actual,dataSize,fileSize;
	BYTE idata i,initialValue;
	
	initialValue = 0x00;
	
	if(G_CATTPTar[1] == 0x03)  //0xBF 01 FF，更新7F25下的6F30/  0xBF 02 FF，更新CSIM ADF下的6F5A  / 0xBF 03 FF，更新USIM ADF下的6F61
	{
		initialValue = 0xFF;
	}
	
	fileSize = GetTFSize(prlAddr);
	
	dataSize = 0x00;
	
	for(i=1; i<= G_CATTPCurrentTotalNumber; i++)
	{
		offset = CATTPDataIsReceived(i);
		
		if(offset == FALSE)
		{
			return;
		}
		
		ReadTFBuf(fileAddr, offset+1, (BYTE*)&sduLen,sizeof(WORD));
		
		offset += 0x03  ;//point to数据
		
// 		while(sduLen >0)
// 		{
// 			actual = (sduLen > 255)?255:sduLen;
// 			
// 			ReadTFBuf(fileAddr,offset, &tempBuf[0], actual);
// 			
// 			offset += actual;
// 			
// 			WriteTFBuf(prlAddr,dataSize,&tempBuf[0] ,actual);
// 			
// 			dataSize  += actual;		
// 			
// 			sduLen -= actual;		
// 		}

		//数据不重叠，可以调用WriteTFFileNonAtomic
		WriteTFFileNonAtomic(prlAddr,dataSize,fileAddr+sizeof(TranFileHead)+offset,sduLen);
		
		dataSize += sduLen;
	
	}
	
	if(dataSize <fileSize)  //数据未写完，则将剩余数据清空 成文件的初始值
	{
		sduLen = fileSize - dataSize;
		
		while(sduLen >0)
		{
			actual = (sduLen > 255)?255:sduLen;
			
			SetTFConst(prlAddr,dataSize,initialValue ,actual);  //断电写，防止数据错误
		
			dataSize  += actual;	
		
			sduLen -= actual;		
		}
	}

}
/*
*@descriptoon:
*
*@param
*
*@history
*/
BYTE CATTPDataIsReceivedAll(void)
{
	BYTE idata i;
	
	for(i=1 ;i <= G_CATTPCurrentTotalNumber; i++)
	{
		if(CATTPDataIsReceived(i) != FALSE)  //存在第i条PRL数据
		{
		}
		else
		{
			return FALSE;
		}
	}
	
	return TRUE;
}
/*
*@description:在PRL执行或恢复过程中，根据tar值，获取不同的PRL文件地址
*
*@function  :GetPRLAddr
*
*@param     :void
*
*@return     :PrlAddr
*
*@author    :xuyz
*
*@history   :V1.0:create 1.0@20160818
*/
UWORD GetPRLAddr(void)
{
	UWORD xdata fileAddr;
	
	if(G_CATTPTar[1] == 0x01)  //更新的是7F25下的6F30 
	{
#if(!UICC__CDMA_2G_NOT_SUPPORT)	
		fileAddr = SelectTFFile(DF_CDMA,EF_PRL_UPDATEFILE);
#else
		fileAddr = SelectTFFile(G_DF_ID_CSIM,EF_PRL_UPDATEFILE);
#endif
	}
	else if(G_CATTPTar[1] == 0x02)  //更新的是CSIM ADF 下的6F5A
	{
		fileAddr = SelectTFFile(G_DF_ID_CSIM,0x6F5A);
	}
	else   //更新的是USIM ADF下的6F61
	{
		fileAddr = SelectTFFile(G_DF_ID_USIM,0x6F61);
	} 
	
	return fileAddr;
}
/*
*@description:PRL数据收全后，更新ＰＲＬ数据　
*
*@param
*
*@history
*/
BYTE CATTPExcutePRLCommand(void)
{
	BYTE idata i;
	WORD idata offset,sduLen,dataSize,fileSize;
	UWORD xdata prlAddr,fileAddr,tagAddr;
	
	prlAddr = GetPRLAddr();
	
	if(prlAddr == FALSE)
	{
		return CipheringError;
	}
	fileAddr =SelectPrivateTFFile(DF_CATTP,EF_CATTPSave);  //该文件执行到这里是一定存在的否则前面更新缓存文件也会出错，因此此处不判断文件是否存在的有效性
	
	if(fileAddr == FALSE)
	{
		return CipheringError;
	}
	
	dataSize = 0x00;

	for(i=1; i<= G_CATTPCurrentTotalNumber; i++)
	{
		offset = CATTPDataIsReceived(i);
		
		if(offset == FALSE)
		{
			return CipheringError;
		}
		
		ReadTFBuf(fileAddr, offset+1, (BYTE*)&sduLen,sizeof(WORD));

		dataSize += sduLen;
	}

	fileSize = GetTFSize(prlAddr);  //下行数据中的PRL数据超过了当前文件的总大小

	if(dataSize > fileSize)
	{
		return 	CipheringError;
	}

	tagAddr =SelectPrivateTFFile(DF_CATTP,EF_CATTPDuanDian);
	
	if(tagAddr == FALSE)
	{
		return CipheringError;
	}
	
	tempBuf[0] = 0x01;
	 
	tempBuf[1] = G_CATTPCurrentTotalNumber;
	 
	ecpMem_Copy(&tempBuf[2] ,&G_CATTPTar[0], 0x03);
	 
	WriteTFBuf(tagAddr, 0x00,tempBuf,0x05); //将断电恢复tag更新进指定文件

	UpdatePRLData(prlAddr,fileAddr);  //更新PRL数据到PRL文件中
	
  SetTFConst(tagAddr, 0x00, 0x00,0x05);
    
	return POR_OK;
	
}
#if UICC__CDMA__PRL__REDUCE_SIZE
void SMSClearFileData(void)
{
	UWORD idata fileAddr;
	WORD  idata usedSize;
	
	fileAddr =SelectPrivateTFFile(DF_CATTP,EF_CATTPSave);
	
	if(fileAddr == FALSE)
	{
		return;
	}
	
	usedSize = GetTFSize(fileAddr);
	usedSize = usedSize - PRL_SMS_SIZE;
	
	fileAddr = fileAddr + sizeof(TranFileHead) + usedSize;
	
	SetEepromEX(fileAddr, 0x00, sizeof(PRLSMSFILEHEAD));
}
#endif
/*
*@description:在开机时判断之前是否在进行CATTP PRL更新操作，如果是，且断电了，则重新进行更新
*
*
*/

BYTE code timerId[8] = {0x01,0x02,0x03,0x04,0x005,0x06,0x07,0x08};
void ResumePrlData(void)
{
	UWORD xdata prlAddr,fileAddr,tagAddr;
	
// 	tagAddr = SelectTFFile(DF_STK, EF_TIMER);	  //先清空timer

// 	 if(tagAddr != FALSE)
// 	 {
// 	 	ReadTFBuf(tagAddr,0x00,&tempBuf[0x10],0x08);

// 		if(ecpMem_Compare((BYTE *)&timerId[0],&tempBuf[0x10], 0x08) != EQUAL)
// 		{
// 	 		WriteTFBuf(tagAddr,0x00,(BYTE *)&timerId[0],0x08);
// 		}
//    }

	
	tagAddr =SelectPrivateTFFile(DF_CATTP,EF_CATTPDuanDian);
	
	if(tagAddr == FALSE)
	{
#if UICC__CDMA__PRL__REDUCE_SIZE
		SMSClearFileData();
#endif
		return ;
	}
	
	ReadTFBuf(tagAddr, 0x00,tempBuf,0x05);  //从PRL恢复区读取恢复tag
	
	if(tempBuf[0] == 0x00)   //tag无效，则清除当前的PRL缓存区域
	{
		ecpMem_Set(G_CATTPTar, 0x00,0x03);
		
		G_CATTPCurrentTotalNumber = FALSE;
#if UICC__FUNCTION__CATTP
		CATTPClearFile(EF_CATTPSave);
#endif
#if UICC__CDMA__PRL__REDUCE_SIZE
		SMSClearFileData();
#endif
		return;
	}
	 
  G_CATTPCurrentTotalNumber = tempBuf[1] ;
	 
	ecpMem_Copy(&G_CATTPTar[0], &tempBuf[2] , 0x03);
	 
	prlAddr = GetPRLAddr();
	
	if(prlAddr == FALSE)
	{
		return;
	}
	
	fileAddr =SelectPrivateTFFile(DF_CATTP,EF_CATTPSave);
	
	if(fileAddr == FALSE)
	{
		return;
	}
	
	UpdatePRLData(prlAddr,fileAddr);

  SetTFConst(tagAddr, 0x00, 0x00,0x05);
    
  ecpMem_Set(G_CATTPTar, 0x00,0x03);
    
	G_CATTPCurrentTotalNumber = FALSE;
	
#if UICC__CDMA__PRL__REDUCE_SIZE
	SMSClearFileData();
#endif

}

BYTE BIPPRLSPICheck(BYTE firstSPI,BYTE secondSPI)
{	
	BYTE idata check1, check2;
	
	check1 = firstSPI & 0x03;			//检查校验算法安全级别(目前只判断这个安全级别必须02或以上)
	
	check2 = 0x02;
	
	if (check1 < check2)
	{
		return FALSE;
	}

	check1 = secondSPI & 0x03;			//检查POR返回级别
	
	check2 = 0x01;
	
	if (check1 < check2)
	{
		return FALSE;
	}

	return TRUE;
		
}
#endif
