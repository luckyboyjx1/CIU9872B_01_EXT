/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.0

 * MODEL: 	All model

 * RELEASE:	2008-5-13

 * AUTHER:	徐庆盛

 * HISTORY: V1.0 创建源文件
		
			V1.1 江苏BIP远程文件管理格式修改为同本地OTABIP远程文件管理格式一致

			V1.2 修改函数OTARFMDecrypt，修改当下行数据若第n条恰好为8的整数倍，则第n+1条数据解密错误
			
			V1.3 修改函数OTARFMDecrypt,RFM的SPI固定为06 00,同时修改函数OTARFMExcute,当RFM MAC COUNTER计算错误后,不再返回RFM失败信息,直接返回9000@20130821 V1.8->1.9
			
			V1.4 修正RFM mac或counter出错后，未清除缓存文件直接返回，导致后续RFM不能正常执行的BUG --ZZJ @2013-9-2

*/
#include <string.h>
#include "stkhead.h"
#include "fileAction.h"
#include "gsmpublic.h"
#include "Authentication.h"
#include "OTA3Head.h"
#include "error.h"
#include "fileSystem.h"
#include "arithmatic.h"



#ifdef OTA
static code INSLIST rfm_list[]=
{
	{0xA4,SelectCommand},
	{0xD6,UpdateBinary},
	{0xDC,UpdateRecord},
	{0xA2,SeekCommand},
	{0x32,IncreaseCommand},
	{0x04,FileStatusHandler},
	{0x44,FileStatusHandler},
	{0xD4,ResizeFile},							//CLA in RFM is 0xA0
	{0xE0,CreateFile},              
	{0xE4,DeleteFile},
	{EOL,InsError},
};

/*-------------------------------------------------------------------------------------------------------//

  Function name:	RfmDataInfo

  Input parameter:	addr:		保存RFM数据的文件

  				groupIndex:	批次号

  				memberIndex:	非0:	数据序号
  							0	:	计算总数

 Output parameter:成功: 某批次数据总条数/某批次某条数据所在记录号

 				失败:	FALSE

 Description:       new

 -------------------------------------------------------------------------------------------------------------------*/

BYTE RfmDataInfo(UWORD addr,BYTE groupIndex, BYTE memberIndex)
{
	BYTE  idata recNum, i;
	BYTE  idata tmp[3];

	recNum = GetLFRecNumLgth(addr,FileNum);

	for(i = 1; i <= recNum; i ++)
	{
		ReadLFBuf(addr, i, 0 , &tmp[0], sizeof(tmp));

		if(tmp[0] == groupIndex)
		{
			if(memberIndex == 0)		//得到批次的总包数
			{
				return tmp[2];
			}

			if(tmp[1] == memberIndex)	//得到某批次某次序数据所在的记录号
			{
				return i;
			}
		}
	}

	return FALSE;
}


/*-------------------------------------------------------------------------------------------------------//

  Function name:	GetRfmCmd

  Input parameter:	*offset:当前记录中已读取的长度

				*logicIndex:当前记录号，逻辑号码

				fileAddr:RFM数据 解包后所在的文件

				rfmDataInfo:批次号、索引、总数

				mode :		0x00: 取APDU

							ox01:取操作数

 Output parameter:FALSE:失败

 				TRUE:成功，继续执行

 				0x02:到达填充位，命令结束

 Description: 		从RFM明文中取命令数据

 status:			new

 -------------------------------------------------------------------------------------------------------------------*/
BYTE GetRfmCmd(BYTE * offset,BYTE * logicIndex,UWORD fileAddr,BYTE rfmDataInfo[3],BYTE mode)
{
	BYTE idata lgth;
	BYTE idata recIndex;
	BYTE idata remainL;
	BYTE idata nextL;
	BYTE idata dateLength;
	BYTE xdata *ptrToCmdBuf;

	if(mode == 0x00)				//取APDU
	{
		ptrToCmdBuf = G_APDU;
		dateLength = 5;
	}
	else							//取数据
	{
		ptrToCmdBuf = RecData;
		dateLength = P3;
	}

	recIndex = RfmDataInfo(fileAddr,rfmDataInfo[0], logicIndex[0]);

	ReadLFBuf(fileAddr , recIndex ,3,&lgth, sizeof(BYTE));

	remainL = lgth - offset[0];				//本记录剩余的长度

	if(remainL >= dateLength)				//本记录可取完整命令数据
	{
		ReadLFBuf(fileAddr , recIndex ,4+offset[0],ptrToCmdBuf, dateLength);

		offset[0] += dateLength;

		if((mode == 0x00)&&(ptrToCmdBuf[0] == ptrToCmdBuf[1]))		//2008-03-19 CLA==INS 遇到填充字节
		{
			return 0x02;				//取APDU到达填充字节
		}
	}
	else if(logicIndex[0] == rfmDataInfo[2])	//本条无法取完整且到达最后一条记录
	{
		if(mode == 0x01)
		{
			return FALSE;					//命令错误
		}
	}
	else									//命令跨记录
	{
		nextL = dateLength - remainL;

		ReadLFBuf(fileAddr , recIndex ,4+offset[0],ptrToCmdBuf, remainL);

L_get_next_rfm:

		logicIndex[0] ++;

		recIndex = RfmDataInfo(fileAddr,rfmDataInfo[0],logicIndex[0]);

		ReadLFBuf(fileAddr , recIndex ,3,&lgth, sizeof(BYTE));

		if(nextL > lgth)
		{
			if(logicIndex[0] == rfmDataInfo[2])			//已到达最后一 条
			{
				return FALSE;					//命令错误
			}
			else							//多条连接完成
			{
				ReadLFBuf(fileAddr , recIndex ,4,&ptrToCmdBuf[remainL], lgth);

				remainL = remainL + lgth;			//offset in databuf

				nextL = nextL - lgth;

				goto L_get_next_rfm;
			}
		}

		ReadLFBuf(fileAddr , recIndex ,4,&ptrToCmdBuf[remainL], nextL);

		offset[0] = nextL;						//命令连接完成

		if((mode == 0x00)&&(ptrToCmdBuf[0] == ptrToCmdBuf[1]))	//填充字节长度超过5，且分在两个记录中
		{
			return 0x02;						//取APDU到达填充字节
		}
	}

	return TRUE;
}


/*-------------------------------------------------------------------------------------------------------//

  Function name:	ExcuteCommand

  Input parameter:	dataAddr:RFM数据 解包后所在的文件

				rfmDataInfo[3]:批次号、索引、总数

				fillLength:MAC填充字节

 Output parameter:FALSE:失败

 				TRUE:成功

 Description: 		执行RFM命令

 status:			new

 -------------------------------------------------------------------------------------------------------------------*/

BYTE ExcuteCommand(UWORD dataAddr, BYTE rfmDataInfo[3])
{
  	UWORD xdata RFMDFAddr,RFMEFAddr;
	WORD idata RFMDFName,RFMEFName;
	WORD idata cmdLength;
	BYTE idata RFMCurrentRecord;
	BYTE idata curOffset,logicIndex,result,num;
	BYTE idata CommandMode;

	RFMDFAddr = G_CurrentDfAddr[G_ChannelNum];                        //备份当前路径

  	RFMEFAddr = G_CurrentEfAddr[G_ChannelNum];

  	RFMDFName = G_CurrentDfName[G_ChannelNum];

  	RFMEFName = G_CurrentEfName[G_ChannelNum];

  	RFMCurrentRecord = G_CurrentRecNum[G_ChannelNum];

	CommandMode = G_CommandMode;                 //备份当前模式，因为远程命令使用AO的class

	SetBit(G_SystemStatus,B_RfmPerform);

	SW1 = 0x90;							//初始条件

	G_CommandMode = Command_Mode_2G;    //A0 默认为2G模式

	logicIndex = 1;

	curOffset = 20;					//去掉第一条内的20字节无效数据

	num = RfmDataInfo(dataAddr,rfmDataInfo[0],1);

	ReadLFBuf(dataAddr, num, 4, &RecData[0], 2);

	cmdLength = (((BYTE)RecData[0]<<8)&0xFF00) + RecData[1] - 22;		//Secured Data总长度，可能包含填充字节

	num = 10;

	while(((SW1 == 0x90)||(SW1 == 0x9F)) && (cmdLength > 4))
	{
		result = GetRfmCmd(&curOffset,&logicIndex,dataAddr,rfmDataInfo,0);		//取APDU

		if(result == TRUE)
		{
			cmdLength = cmdLength-5;
		}
		else if(result == 0x02)
		{
			break;
		}
		else
		{
			goto L_RfmExcuteFaile;
		}

		if(P3 != 0)
		{
			result = GetRfmCmd(&curOffset,&logicIndex,dataAddr,rfmDataInfo,1);		//取操作数

			if(result == TRUE)
			{
				cmdLength = (cmdLength>P3)?(cmdLength-P3):0;
			}
			else
			{
L_RfmExcuteFaile:

				num = 10;					//执行错误

				break;
			}
		}

		if(INS == 0xE0)
		{
			SetBit(G_SystemStatus,B_cmk);
		}

		for(num = 0 ; ; num ++)			//执行RFM命令
		{
			if((rfm_list[num].ins == INS)||(rfm_list[num].ins == EOL))
			{
				rfm_list[num].func();

				break;
			}
		}

		//dataAddr = SelectLFFile(DF_OTA, EF_DownLoadFile);	 		//执行完一次命令后,重新定位文件


	}

	ClearBit(G_SystemStatus,B_cmk);

	ClearBit(G_SystemStatus,B_RfmPerform);

  	G_CurrentDfAddr[G_ChannelNum] = RFMDFAddr  ;				//需要完成路径恢复后才可退出

  	G_CurrentEfAddr[G_ChannelNum] = RFMEFAddr  ;

  	G_CurrentDfName[G_ChannelNum] = RFMDFName  ;

  	G_CurrentEfName[G_ChannelNum] = RFMEFName  ;

  	G_CurrentRecNum[G_ChannelNum] = RFMCurrentRecord ;

	G_CommandMode = CommandMode;

	G_CurrentReponse[G_ChannelNum] = 0x00;

	if ((num == 10) || ((SW1 != 0x90)&&(SW1 != 0x9F)))           		//add by cui, ref 03.48 p15
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/*--------------------------------------------------------------------------------------------------------------------

  Function name:	IsRFMReceiveAll

  input parameter:	fileAddr

  				packageNum

  output parameter:收满所有返回TRUE，否则返回FALSE

  description:		检查RFM数据文件内的某一包是否收完全

  author:			Zhuli

  Test:			Caibo

  status:		    OK
 -------------------------------------------------------------------------------------------------------------------*/

BYTE IsRFMReceiveAll(UWORD fileAddr, BYTE packageNum)
{
	BYTE  idata i, total;

	total = RfmDataInfo(fileAddr, packageNum, 0);				//计算总包数

	if(total == 0)
	{
		return FALSE;
	}

	for(i = 1; i <= total; i ++)						//检查每包是否收到
	{
		if(RfmDataInfo(fileAddr,packageNum,i) == FALSE)
		{
			return FALSE;
		}
	}

	return TRUE;
}
/*--------------------------------------------------------------------------------------------------------------------

  Function name:	ClearRFMTmpPackage

  Input parameter:	fileAddr:存储RFM缓冲数据的文件地址
  					pageNum:批次

  Output parameter:	空记录的记录号

  Description:		清空指定批次的所有记录

  Author:			Zhuli

  Test:				Caibo

  Status:			OK
 -------------------------------------------------------------------------------------------------------------------*/
void ClearRFMTmpPackage(UWORD fileAddr, BYTE pageNum)
{
	BYTE  idata tmp, i,recNum;

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	for(i = 1; i <= recNum; i ++)
	{
		ReadLFBuf(fileAddr, i, 0x00, &tmp, sizeof(BYTE));

		if(tmp == pageNum)
		{
			SetLFConst(fileAddr, i, 0x00, 0x00, 0x04);
		}
	}
}

/*--------------------------------------------------------------------------------------------------------------------

  Function name:	ClearRFMTmpFile

  input parameter:	fileAddr
                    curtPackage : 短信批次号

  output parameter:	void


  description:		在文件中删除批次号与curtPackage不相同的一个包

  author:			Zhuli

  Test:				Caibo

  status:			OK
 -------------------------------------------------------------------------------------------------------------------*/
void ClearRFMTmpFile(UWORD fileAddr, BYTE curtPackage)
{
	BYTE idata i, recNum,packageIndex;

	recNum = GetLFRecNumLgth(fileAddr, FileNum);

	for(i = 1; i <= recNum; i ++)
	{
		ReadLFBuf(fileAddr, i, 0, &packageIndex, sizeof(BYTE));

		if((packageIndex != curtPackage)&&(IsRFMReceiveAll(fileAddr,packageIndex) == FALSE))
		{
			ClearRFMTmpPackage(fileAddr,packageIndex);

			return;
		}
	}
}
/*--------------------------------------------------------------------------------------------------------------------

  Function name:	UsableRecordInFile

  input parameter:	在RFM数据文件找空记录

  output parameter:	成功：	返回记录号

					失败：	返回FALSE

  description:

  author:			Zhuli

  status:
 -------------------------------------------------------------------------------------------------------------------*/
BYTE UsableRecordInFile(UWORD fileAddr)
{
	BYTE idata recNum, i;
	BYTE xdata tmp[4];

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	for(i=1; i<=recNum; i++)
	{
		ReadLFBuf(fileAddr, i, 0x00, &tmp[0], sizeof(tmp));

		if((tmp[0] == 0x00)&&(tmp[1] == 0x00)&&(tmp[2] == 0x00)&&(tmp[3] == 0x00))
		{
			return i;
		}
	}

	return FALSE;
}

/*--------------------------------------------------------------------------------------------------------------------

  Function name:	SaveRFMDataSms

  input parameter:	ptr:UDL的偏移

  output parameter:	成功：	TRUE

					失败：	返回FALSE

  description:		保存下载的RFM数据短信
  					根据EF_DownLoadFile是否有空记录，来确定是否保存该条短信。
  					如果没有空记录，则清除批次不一样的没有收齐的其中一包中的短信。
					如果没有空记录，并且没有未收齐的批次，则放弃该条短信。

  author:			Zhuli

  status:
 -------------------------------------------------------------------------------------------------------------------*/
void SaveRFMDataSms(BYTE ptr)
{
	UWORD xdata fileAddr;
	BYTE  idata recordNo;
	BYTE  idata result;

	fileAddr = SelectLFFile(DF_OTA, EF_DownLoadFile);

	if(fileAddr == FALSE)
	{
		return;											
	}

	recordNo = UsableRecordInFile(fileAddr);				//是否有空的记录

	if(recordNo == FALSE)							//没有空的记录
	{
		ClearRFMTmpFile(fileAddr,RecData[ptr+4]);			//清除批次不一样的没有收齐的其中一包中的短信

		recordNo = UsableRecordInFile(fileAddr);			//是否有空的记录

		if(recordNo == FALSE)						//短信都已经收满
		{
		 	return;							//放弃当前的短信
		}
	}

	if(RecData[ptr+1] < 2)
	{
		return;
	}

	tempBuf[0] = 3 + RecData[ptr] - RecData[ptr+1];				//存储数据的总长度:(短信批次、索引、短信总数)+(UDL＋1)-(UDHL＋1)

	memcpy(&tempBuf[1], &RecData[ptr+4], 3);

	tempBuf[4] = tempBuf[0] - 4;						//用户数据长度

	ptr = RecData[ptr+1] - 2 + ptr;

	memcpy(&RecData[ptr], &tempBuf[1],4);

	result = WriteLFBuf(fileAddr, recordNo, 0x00,&RecData[ptr], tempBuf[0]);

	if(result == FALSE)
	{
	 	return;
	}

	result = IsRFMReceiveAll(fileAddr,tempBuf[1]);

	if(result == TRUE)
	{
		PendingMessageAlert(D_RFMSucessTag,0x00);
	}
}
/*--------------------------------------------------------------------------------------------------------------------

  Function name:	SaveBigRFMDataSms

  input parameter:	ptr:UDL的偏移

  output parameter:	成功：	TRUE

					失败：	返回FALSE

  description:		保存通过BIP模式下载的单条RFM数据短信

  author:			Zhuli

  status:
 -------------------------------------------------------------------------------------------------------------------*/
#ifdef LOCALOTA

BYTE xdata BIPRfmIndex;

//本地OTA 中BIP方式的远程文件更新

void SaveBIPRFMDataSms(BYTE ptr)
{
	BYTE  idata recordNo;
	BYTE  idata result,length,i,lastLength;
	UWORD xdata fileAddr,actual;
	BYTE  idata start;

	ReadTFBuf(G_BipDataAddr, 0x00, &RecData[0],0xFF);

	fileAddr = SelectLFFile(DF_OTA, EF_DownLoadFile);

	if(fileAddr == FALSE)
	{
		return;
	}

	recordNo = UsableRecordInFile(fileAddr);				//是否有空的记录

	if(recordNo == FALSE)							        //没有空的记录
	{
		ClearRFMTmpFile(fileAddr,RecData[ptr+5]);		//清除批次不一样的没有收齐的其中一包中的短信

		recordNo = UsableRecordInFile(fileAddr);			//是否有空的记录

		if(recordNo == FALSE)						       //短信都已经收满
		{
		 	return;							        //放弃当前的短信
		}
	}

	if(RecData[ptr+2] < 0x02)             //point to UDHL
	{
		return;
	}

	actual = ( ((RecData[ptr]<<8)&0xFF00) | (RecData[ptr+1]&0x00FF) ) - (RecData[ptr+2]+1);  //待存储数据的实际长度
	
	if(actual > 0x400)      //实际传输的字节数超过了 BIP最大能传输的字节数1024bytes
	{
		return;
	}	

	if(BIPRfmIndex == 0xFF)
	{
		BIPRfmIndex = 0x01;   //批次从1-FF循坏
	}

	tempBuf[1] = BIPRfmIndex;         //短信批次

	BIPRfmIndex ++;

	tempBuf[3] = actual / 220 ;   //假定每条记录只存储220bytes的有效数据

	if((actual % 220) != 0)  //短信总数
	{
		tempBuf[3] = tempBuf[3] + 1;
	}	

	i = 1;                             //短信起始索引

	lastLength = 0;    

	while(actual > 0)
	{
		if ( actual > 220 )
		{
			length = 220;
		}
		else
		{
			length = actual;
		}

		tempBuf[0] = length + 4;		

		tempBuf[2] = i;                      //短信索引

		tempBuf[4] = length;                 //TPDU字段的数据长度

		start = 5+(i-1)*lastLength;

	
		ReadTFBuf(G_BipDataAddr, start, &RecData[0],0xFF);
			
		memcpy_G(&tempBuf[5],&RecData[0],length);	
	
		recordNo = UsableRecordInFile(fileAddr);	//是否有空的记录

		if(recordNo == FALSE)						    
		{
		 	return;							        //放弃当前的短信
		}

		result = WriteLFBuf(fileAddr, recordNo, 0x00,&tempBuf[1], tempBuf[0]);

		if(result == FALSE)
		{
	 		return;
		}

		lastLength = length;

		actual = actual - length;

		i ++; 
	}
	
	PendingMessageAlert(D_RFMSucessTag,0x00);
}

#endif
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	OTARFMDecrypt

  input parameter:	fileAddr:RFM下行数据文件

					packageNum:批次号

  output parameter:  TRUE/FALSE

  description:		在保存RFM下行数据的文件解包RFM数据,并检验MAC
					注意:该文件的大小需要满足最大可能的解包长度

  status:			OK
-------------------------------------------------------------------------------------------------------------------*/
BYTE OTARFMDecrypt(UWORD fileAddr, BYTE packageNum)
{
	BYTE  idata result;
	BYTE  idata lgth;						//一次解包处理的字节长度
	BYTE  idata lastLgth;					//一次解包后剩余的不足8字节的余数
	BYTE  idata i;
	BYTE  idata total;
	BYTE  idata recIndex;
	BYTE  xdata mac[4], key[16], cc[8];

	total = RfmDataInfo(fileAddr,packageNum,0);

	if(total == 0)
	{
		return FALSE;
	}

	for(i = 1; i <= total; i ++)
	{
		recIndex = RfmDataInfo(fileAddr,packageNum, i);

		ReadLFBuf(fileAddr, recIndex, 0x03, (BYTE *)&RecData[0], sizeof(BYTE));

		if((RecData[0] == 0x00)||(RecData[0] == 0xFF))
		{
			return FALSE;
		}

		ReadLFBuf(fileAddr, recIndex, 0x04, (BYTE *)&RecData[1], RecData[0]);

		if(i == 0x01)
		{
			if((RecData[4] != 0x06)||(RecData[5] != 0x00))  //spi 的值为 06 00
			{
				return FALSE;
			}
			
			if( G_RFMTag == 0xB0 ) //OTA
			{
				result = OTAGetKey((BYTE *)&RecData[21], &key[0], D_RFMKey);		//offset 21指向随机数
			}
			else if( G_RFMTag == 0xE0  )	//OTP
			{
				result = OTAGetKey((BYTE *)&RecData[21], &key[0], D_OTPRfmKey);
			}
			else	//for baihe-test
			{
				return FALSE;
			}

			if(result == FALSE)
			{
				return FALSE;
			}
				

			memcpy((BYTE *)&RecData[D_TmpBufStart+16], (BYTE *)&RecData[21], 4);	//保存随机数(未加密)

			memmove((BYTE *)&RecData[21], (BYTE *)&RecData[25], RecData[0]-24);	//前移随机数后续的数据

			lgth = RecData[0] - 14;									//DES加密数据不包含CPL,CHL,SPI,KIC,KID,TAR,随机数

			lastLgth = lgth &0x07;

			if(lastLgth != 0x00)
			{
				lgth -= lastLgth;

				memcpy((BYTE *)&RecData[D_TmpBufStart+8], (BYTE *)&RecData[lgth + 11], lastLgth);		//保存本次剩余数据
			}

			memcpy(&cc[0], (BYTE *)&RecData[lgth+3], 8);		//本次的最后一个8字节块

			TripleDES((BYTE *)&RecData[11], &key[0], (BYTE *)&RecData[11], CBC_Decrypt, lgth);			//从CNTR开始解密

			memcpy(&mac[0], (BYTE *)&RecData[17], 4);		//得到解密后的MAC

			memcpy((BYTE *)&RecData[17], (BYTE *)&RecData[D_TmpBufStart+16], 4);	//随机数替换MAC,用于计算MAC

			RecData[0] = lgth + 10;

			WriteLFBuf(fileAddr, recIndex,3, (BYTE *)&RecData[0], RecData[0]+1);		//LV格式记录解密后的文件
		}
		else
		{
			lgth = RecData[0];

			if(lastLgth != 0)										//上次有余数
			{
				lgth += lastLgth;									//本次数据的长度加上上次剩余数据

				memmove((BYTE *)&RecData[lastLgth+1], (BYTE *)&RecData[1], RecData[0]); //将上次剩余的未解密的数据和该次数据拼在一起。						

				memcpy((BYTE *)&RecData[1], (BYTE *)&RecData[D_TmpBufStart+8], lastLgth);
			}

			lastLgth = (lgth &0x07);

			if(lastLgth != 0)
			{							
				lgth -= lastLgth;
				
				memcpy((BYTE *)&RecData[D_TmpBufStart+8], (BYTE *)&RecData[lgth+1], lastLgth);
			}

			memcpy((BYTE *)&RecData[D_TmpBufStart+16], (BYTE *)&RecData[lgth-7], 8);	//将此次解密的原始数据的后8个字节放入BUFFER中。

			TripleDES((BYTE *)&RecData[1], &key[0], (BYTE *)&RecData[0], CBC_Decrypt, lgth);
			
			for(result=0; result<8; result ++)									//解密后的头8个字节要做异或操作。
			{			
				RecData[result] = RecData[result] ^ cc[result];
			}
			
			memcpy(&cc[0], (BYTE *)&RecData[D_TmpBufStart+16], 8);	//将此次解密的原始数据的后8个字节放入BUFFER中
		
			SetLFConst(fileAddr, recIndex, 3, lgth, sizeof(BYTE));				//%%%需要和后条合并一次写入
			
			WriteLFBuf(fileAddr, recIndex,4+2, (BYTE *)&RecData[0x00], lgth);	//空2字节给调整8字节整数倍使用
		}
	}

	OtaRfmMac(fileAddr, &key[0], &cc[0], total, packageNum);

	if(memcmp(&mac[0], &cc[0], 4) != EQUAL)
	{
		return FALSE;
	}

	recIndex = RfmDataInfo(fileAddr,packageNum, 1);		//取解包的第一包数据所在记录号

	ReadLFBuf(fileAddr, recIndex, 4 + 10, &cc[0], 5);	//取计数器

	result = OTAVarifyCNTR(&cc[0], D_CNTRB, FALSE);

	if(result == FALSE)					//较验Counter
	{
		return FALSE;
	}

	OTAVarifyCNTR(&cc[0], D_CNTRB, result);			//根据Counter B较验结果进行更新/不更新

	return TRUE;
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTARFMExcute

  input parameter:   void

  output parameter:	 FALSE/TRUE

  description:       Process OTA2 pending message

  author:			 Zhuli

  status:			 OK

  Testor:			Caibo

 -------------------------------------------------------------------------------------------------------------------*/
BYTE OTARFMExcute(void)
{
	UWORD xdata fileAddr;
	BYTE  idata  i, result,recNum;
	BYTE  xdata rfmDataInfo[3];

	fileAddr = SelectLFFile(DF_OTA, EF_DownLoadFile);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	recNum = GetLFRecNumLgth(fileAddr, FileNum);

	for(i = 1; i <= recNum; i ++)		//找任意一个收全的批次
	{
		ReadLFBuf(fileAddr, i, 0, (BYTE *)&rfmDataInfo[0], 3);

		if(IsRFMReceiveAll(fileAddr, rfmDataInfo[0]) == TRUE)
		{
			break;
		}
	}

	if(i > recNum)							//没有收全的批次
	{
		return FALSE;
	}

	result = OTARFMDecrypt(fileAddr , rfmDataInfo[0]);
	
	if(result == FALSE)  //MAC  or COUNTER error
	{
		ClearRFMTmpPackage(fileAddr, rfmDataInfo[0]);
		
		return FALSE;
	}

	if(result == TRUE)
	{
		result = ExcuteCommand(fileAddr, &rfmDataInfo[0]);		//new
	}

	//fileAddr = SelectLFFile(DF_OTA, EF_DownLoadFile);				//RFM操作后,重新选文件

	ClearRFMTmpPackage(fileAddr, rfmDataInfo[0]);

	result = OTASendRFMResult(result, rfmDataInfo[0]);

	return result;
}

#endif
