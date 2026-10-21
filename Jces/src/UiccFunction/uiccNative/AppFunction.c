/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.0

 * MODEL: 	All model

 * RELEASE:	2008-5-13

 * AUTHER:	徐庆盛

 * HISTORY: V1.0 创建源文件
 	        V1.1 根据需求添加函数OTACaluteMac与singleDesMac
			V1.2 PendingMessageAlert修改为两个参数,EF_DownLoadFile改为EF_RFMDownLoadFile避免冲突   

*/
#include <string.h>
#include "stkhead.h"
#include "fileAction.h"
#include "gsmpublic.h"
#include "Authentication.h"
#include "error.h"
#include "fileSystem.h"
#include "arithmatic.h"
//#include "OTA3Head.h"



/*-------------------------------------------------------------------------------------------------------------------

  Function name :	PBOCAuth

  input parameter:   	BYTE key[16]

  			BYTE x[8]

  output parameter:	void

  description:       	The algorithm PBOC


  status:

 -------------------------------------------------------------------------------------------------------------------*/
void PBOCAuth(BYTE key[16],BYTE sourceKey[8])
{
	BYTE idata i;
	BYTE xdata nx[8];

	for(i=0;i<8;i++)
	{
		nx[i] = ~sourceKey[i];
	}

	TripleDES(sourceKey,key,sourceKey,ECB_Encrypt,8);

	TripleDES(nx,key,nx,ECB_Encrypt,8);

	memcpy(&key[0], &sourceKey[0], 8);

	memcpy(&key[8], &nx[0], 8);
}

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
	BYTE idata tmp[4];

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

  Function name:	SaveRFMDataSms

  input parameter:	ptr:UDL的偏移

  output parameter:	成功：	TRUE

					          失败：	返回FALSE

  author:			Zhuli

  status:
 -------------------------------------------------------------------------------------------------------------------*/
void SaveRFMDataSms(BYTE ptr)
{
	UWORD idata fileAddr;
	BYTE  idata recordNo;
	BYTE  idata result;

	fileAddr = SelectLFFile(DF_OTA, EF_RFMDownLoadFile);

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


/*-------------------------------------------------------------------------------------------------------//

  Function name:	singleDesMac
 
  Input parameter:	
 			        cpEnter:data

					cpKey  :key	

					cpResult:result

					nEnterLen: length of data

					mode:	 ECB_Encrypt/CBC_Encrypt
 
  Output parameter:     the number of tlv structure
 
  Description:          return the number that first tlv structure in input buffer 

  Status:		OK

 -------------------------------------------------------------------------------------------------------------------*/
void singleDesMac(BYTE  * dataBuf,BYTE  cpKey[8],BYTE  cpResult[8],WORD len,BYTE mode)
{	
	WORD idata i;
	BYTE idata j;
	BYTE xdata d[8];

	memset(cpResult, 0x00, 8);

	for(i=0; i<len; i+=8)
	{
		if ((i + 8) > len)						//最后一页
		{
			memset(d, 0x20, 8);

			memcpy(d, &dataBuf[i], len - i);
		}
		else
		{
			memcpy(d, &dataBuf[i], 8);
		}

		for(j=0; j<8; j++)
		{
			d[j] = d[j]^cpResult[j];
		}

		SingleDES(d, cpKey, cpResult, mode, 8);
	}	
}



/*--------------------------------------------------------------------------------------------------------------------
  Function name:	OTACaluteMac

  input parameter:	dataBuf：	数据

  					mac:		mac内容,如果(type==1)则把Mac的计算结果存入，否则比较Mac的内容

					key:		key的内容

					len:		计算数据的长度

					type:		0x01:计算MAC
							0x00:校验MAC

  output parameter:	TRUE/FALSE

  description:		计算并且比较Mac的内容

  author:			Zhuli

  Testor:			Caibo

  status:			OK
 -------------------------------------------------------------------------------------------------------------------*/

BYTE OTACaluteMac(BYTE * dataBuf, BYTE mac[4], BYTE key[16], WORD len, BYTE type)
{
  	BYTE xdata c[8];

  	singleDesMac(dataBuf,key,c,len,CBC_Encrypt);

	if (type == 0x00)
	{
		if(memcmp(&mac[0], &c[0], 4) == EQUAL)
		{
			return TRUE;
		}

		return FALSE;
	}

	memcpy(&mac[0], &c[0], 4);

	return TRUE;
}
