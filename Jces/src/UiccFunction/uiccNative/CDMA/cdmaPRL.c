
/*
*@HISTORY       :V1.0: 创建源文件
*				 V1.1：修改函数WritePRLData，兼容小端芯片					--by yangjian 2015.07.14
*/



#include <string.h>
#include "uicc_main.h"
#include "gsmpublic.h"
#include "fileSystem.h"
#include "cdma.h"
#include "stkhead.h"
#include "fileAction.h"
#include "ceph.h"
#include "uicc_error.h"
//#include "resume.h"
#include "CDMA.h"
//#include "Transaction.h"
#include "interface.h"



#ifdef CDMA
WORD xdata VersionNumber;
BYTE xdata G_EPRL_PROCING;


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

/**
 * @brief 	store the first PRL data to the file 6FFF
 * 
 * @param 	receiveSms: data string
 * @param 	fileHeadAddr:  
 * 
 * @Returns	:0xFF(last num)/0x00(error)/(0x01-0x0f) already stored num
 * 
 * @author 	XWH
 * 
 * @history	tested by 
 */
BYTE storePRLData(BYTE  * receiveSms, UWORD fileHeadAddr)
{
	BYTE idata CNum, TNum, ReceiveNum;
	BYTE idata PRLLen, PRLTag, i;
	WORD  idata fileSize;
	UWORD xdata fileDataAddr,fileBodyAddr;
	PRLSMSFILEHEAD xdata PRLSmsFileHead;
	// current num
	CNum = (receiveSms[0]&0xF0) >> 4;

	ReceiveNum = (receiveSms[0]&0xF0) >> 4;

	TNum = receiveSms[0]&0x0F;	                                               // total num


	fileSize = GetTFSize(fileHeadAddr);

	//ReadEeprom((BYTE *)&fileSize, fileHeadAddr-sizeof(TranFileHead)+F_MSize_Offset, sizeof(WORD)); //retrieve the length of the whole file, don't include the file head

    fileBodyAddr = getFileContentAddr(fileHeadAddr);


	ReadEeprom((BYTE *)&PRLSmsFileHead, fileBodyAddr, sizeof(PRLSMSFILEHEAD));

	if ((PRLSmsFileHead.StoreNum != 0) && (PRLSmsFileHead.TotleNum == PRLSmsFileHead.StoreNum))
	{
		return IS_LAST; 														// receive all
	}

	if ((fileSize-sizeof(PRLSMSFILEHEAD)-PRLSmsFileHead.UsedSpace) < (WORD)(receiveSms[3]+4))
	{
		return FALSE;
	}

	fileDataAddr = fileBodyAddr + sizeof(PRLSMSFILEHEAD);


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
	if (CNum ==0x01)
	{
		VersionNumber = ((receiveSms[6]<<8)&0xFF00) + receiveSms[7]	;
	}

	fileDataAddr = fileBodyAddr + sizeof(PRLSMSFILEHEAD)+PRLSmsFileHead.UsedSpace;

	

	receiveSms[0] = CNum ;

	WriteEepromEX(fileDataAddr, &receiveSms[0], receiveSms[3] +4);                   // store the PRL data(TLV)

	PRLSmsFileHead.TotleNum = TNum ;

	PRLSmsFileHead.StoreNum = PRLSmsFileHead.StoreNum + 1 ;                                                     // set the store num

	PRLSmsFileHead.UsedSpace = PRLSmsFileHead.UsedSpace + receiveSms[3] + 4;          //add the usedsize

	WriteEepromEX(fileBodyAddr, (BYTE *)&PRLSmsFileHead, sizeof(PRLSMSFILEHEAD));
	

	if (PRLSmsFileHead.TotleNum == PRLSmsFileHead.StoreNum)
	{
		return IS_LAST;
	}

	else return (ReceiveNum);
}


/**
 * @brief 	read out  the PRL data to the buffer ResData
 * 
 * @param 	fileHeadAddr: 文件体地址 
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

						memmove((BYTE *)&resultOut[0], (BYTE *)&resultOut[PRLLen-2], sizeof(WORD));
					}
					else
					{
						CRCResult = PRLcrc(0xFFFF, &resultOut[0], PRLLen);
					}
				}
				else if (PRLTag == PRLNum)
				{
					CRCResult = PRLcrc(CRCResult, &resultOut[0], PRLLen-2);

					memmove((BYTE *)&resultOut[0], (BYTE *)&resultOut[PRLLen-2], sizeof(WORD));
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
 * @history 	created by 
 */
BYTE WritePRLData(UWORD writeFileAddr, BYTE * userData, WORD fileSize)
{
	WORD  idata usedSize, Offset;
	UWORD xdata fileHeadAddr;
	UWORD xdata fileDataAddr;
	BYTE  idata i, j, PRLLen, PRLTag, PRLNum;
	BYTE  idata tem[2];
	

	//fileHeadAddr = SelectTFFile(DF_CDMA, EF_PRL_SMS);
	fileHeadAddr = SelectTFFile(G_DF_ID_CSIM, EF_PRL_SMS);

	if (fileHeadAddr == FALSE)
	{
		return FALSE ;
	}

	fileHeadAddr = getFileContentAddr(fileHeadAddr);		  //得到文件体地址赋值给fileHeadAddr

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

	memmove(SMSstr+pos+len, SMSstr+pos+length, ll);

	memcpy(SMSstr+pos, &UserData[0x00], len);

	pos = pos + len;

	pos = pos + ll;

	SMSstr[upos+1] = len;

	//SMSstr[bpos+1] = pos - bpos-2;
	SMSstr[bpos+1] = SMSstr[bpos+1] - length + len;    //修改BearData length计算方式, by xwh 20190830

	SMSstr[1] = pos -2;

	memset(SMSstr+pos, 0, 255-pos);
	

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

	PendingMessageAlert(D_PRLDisplayTag,0x00);
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
	BYTE  idata  len;

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

	STKPerform = TRUE;
}



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
	UWORD xdata  fileHeadAddr, smsFileAddr,fileBodyAddr;
	BYTE  xdata  smsBuf[50];

	//smsFileAddr = SelectLFFile(DF_CDMA, EF_SMS);
	smsFileAddr = SelectLFFile(G_DF_ID_CSIM, EF_SMS);

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
	fileHeadAddr = SelectTFFile(G_DF_ID_CSIM, EF_PRL_SMS);

	if (fileHeadAddr == FALSE)
	{
		return FALSE ;
	}

	fileBodyAddr = getFileContentAddr(fileHeadAddr);				//得到文件体地址

	//fileHeadAddr = fileHeadAddr + sizeof(TranFileHead);		//去掉此句,改为直接使用文件头



	SNum = storePRLData(&str[pos], fileHeadAddr);		   //此处fileHeadAddr为文件头地址

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

		//ReadEeprom((BYTE *)&TNum, fileHeadAddr, 1);
		ReadEeprom((BYTE *)&TNum, fileBodyAddr, 1);

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
			memcpy(&smsBuf[prllen+3], (BYTE *)EPRLInfoProc, messageLen);
		}
		else
		{
			messageLen = sizeof(PRLInfoProc);
			memcpy(&smsBuf[prllen+3], (BYTE *)PRLInfoProc, messageLen);
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
				memmove(&str[pos],&str[pos+3],moveLen);	

				memset(&str[pos+moveLen], 0x00, 0x03);
		
				str[beaDtaLenOffset] -= 3;	
				
				/*
				temLen = str[beaDtaLenOffset];
				
				if(str[beaDtaLenOffset+temLen-2] == 0x06)
				{
					str[beaDtaLenOffset] -= 3;		   //长度不包括0601XX的部分
				}
				*/
				
				
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
			
			memcpy(&smsBuf[0x03], (BYTE *)EPRLInfoSucc, messageLen);
		}
		else
		{
			messageLen = sizeof(PRLInfoSucc);
			
			memcpy(&smsBuf[0x03], (BYTE *)PRLInfoSucc, messageLen);
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
			memmove(&str[pos],&str[pos+3],moveLen);
			
			memset(&str[pos+moveLen], 0x00, 0x03);
			
			str[beaDtaLenOffset] -= 3;

			/*
			temLen = str[beaDtaLenOffset];
				
			if(str[beaDtaLenOffset+temLen-2] == 0x06)
			{
				str[beaDtaLenOffset] -= 3;		   //长度不包括0601XX的部分
			}
			*/
	
			str[1] -= 3;
		}
//end			

		WriteLFBuf(smsFileAddr, recNo, 0, str, len);
	}

    crcResult = PRLcrcFromEeprom(fileBodyAddr, &RecData[0]);
	

	Addr = (RecData[0] << 8) | (RecData[1]);

	if ( crcResult != Addr)                                                  // the CRC calculation is incorrect
	{
		if(G_EPRL_PROCING == TRUE)
		{
			messageLen = sizeof(EPRLInfoError);
			
			memcpy(&smsBuf[0x03], (BYTE *)EPRLInfoError, messageLen);

		}
		else
		{
			messageLen = sizeof(PRLInfoError);
			
			memcpy(&smsBuf[0x03], (BYTE *)PRLInfoError, messageLen);
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

		SetEepromEX(fileBodyAddr, 0x00, sizeof(PRLSMSFILEHEAD));

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

	Addr = getFileContentAddr(Addr);

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
		
		memcpy(&smsBuf[0x03], (BYTE *)EPRLInfoSucc, messageLen);
	}
	else
	{
		messageLen = sizeof(PRLInfoSucc);
		
		memcpy(&smsBuf[0x03], (BYTE *)PRLInfoSucc, messageLen);
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

	SetEepromEX(fileBodyAddr, 0x00, sizeof(PRLSMSFILEHEAD));
	
	return TRUE;
}

#endif
