
/*
  V1.0 创建文件
  V1.1 修改函数LDFileCheckCounter，LDCleanCounter，LDFileMuliPPHanlder，由一条counter改为多条counter    
       修改函数LDFileSMSPoR,改为从LDFile_RFMPerso中取RFM批次号												by yangjian 20180720



*/



#include <string.h>
#include  "typedef.h"
#include  "uicc_main.h"
//#include  "resume.h"
#include "stkhead.h"
#include "fileSystem.h"
#include "fileAction.h"
#include "gsmpublic.h"
#include "error.h"
#include "interface.h"
#include "gsmpublic.h"
#include "arithmatic.h"
#include "LDFile.h"
#include "ceph.h"
//#include "Transaction.h"
#include "CompactManage.h"
#ifdef CATTP
#include "CATTP.h"
#endif
#if GP__AMENDMENT__B__FOR_MASK
#include "HttpProcess.h"
#endif


BYTE xdata G_OneByteTest[3];											//for test
WORD xdata G_TwoByteTest[3];
UWORD xdata G_FourByteTest;

#ifdef LDFile

BYTE  xdata G_LDFileDeliver;		//标志是否返回9FXX/9EXX/61XX
BYTE  xdata G_LDFileDeliverMuli;	//deliver是否级联
BYTE  xdata G_Source_Type;      //区分数据源 CATTP/Http/SMS
WORD  xdata G_CurrentRemoteResponseLgth;	//https的下行数据执行完apdu指令后的响应数据长度,长度赋值0xFFFF 作为Http触发短信时候特殊使用
UWORD xdata G_CurrentRFMEfAddr; //用来保存102226读取数据过长时候的地址
BYTE  xdata G_RFMTypeStatus;		//设置当前的RFM操作状态 
BYTE  xdata G_RFMAllReceived;				//表示已收完一个批次的RFM短信 ，准备处理
BYTE  xdata G_FormatAndType;				//数据的格式  压缩/AA/AE
BYTE  xdata G_TARRecIndex[3];				//同步tpx/epx平台的G_TARRecIndex, G_TARRecIndex[0]: tarType(native 都是RFM);G_TARRecIndex[1]:指示当前的文件系统;G_TARRecIndex[2]：RFU
BYTE  xdata G_CurrentComNUM;  			//保存当前RFM执行的APDU 个数
WORD  xdata G_CurrentCommandOffset;  //当前APDU 指令在文件中的偏移
BYTE xdata  G_PORTag;               //判断当前是否需要发送POR

/*
*@description :执行短信模式的CATTP打开链接
*
*@function :CATTPExcuteCommand
*
*@param     :void
*
*@return     :TRUE/FALSE 返回值无意义
*
*@author     :xuyz
*
*@history    :V1.0:create @20160819
*/
BYTE CATTPExcuteCommand(void)																 //here,commandbuffer is the same with global var ResData[]
{
  BYTE  xdata firstCpl[0x03];
  UWORD xdata fileAddr;
  WORD  xdata cpLgth;	
	WORD  xdata ptr;

	fileAddr = SelectTFFile(DF_STK, LDFile_RelaxFile);	//not inclue the file head 	   缓存解密后的RFM数据

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
										
	ReadTFBuf(fileAddr, 0, &firstCpl[0], 0x02);         //更新 CPL

	cpLgth = (firstCpl[0]<<0x08)| firstCpl[1];
	
	SW1 = 0x90;

	SetBit(G_SystemStatus,B_RfmPerform);                //需要增加， PushCommand

	ptr = G_CurrentCommandOffset;
	
	while((ptr < (cpLgth+2)) && ((SW1 == 0x90)||(SW1 == 0x9F)) )
    {
		ReadTFBuf(fileAddr,ptr,&G_APDU[0],0x05); //读取 CLA INS P1 P2 P3 
		
		if( CLA == INS)   //遇到填充字节
		{
			break;
		}
		
		if((ptr+P3)>(cpLgth+2)) //本条指令取不完整
		{
			return Cipher_Error;
		}

		ptr = ptr + 5 ;

		if(INS == 0xEC)
		{
			ReadTFBuf(fileAddr,ptr,&RecData[0],P3);
		
		    ptr = ptr + P3 ;
#ifdef CATTP			
			PushCommand();
#endif			
		}
		else
		{
			break;
		}
 
   }

   ClearBit(G_SystemStatus,B_RfmPerform);       //需要增加， PushCommand

#ifdef CATTP
   if(G_PushCommand == TRUE)
	{

		CATTPOpenLink();

		G_PushCommand = FALSE;
	}
#endif	
	return POR_OK;
 
}


/*-------------------------------------------------------------------------------------------------------//

  Function name:	DesMacFunction
 
  Input parameter:	
 			        cpEnter:data

					cpKey  :key	

					cpResult:result

					nEnterLen: length of data
 
  Output parameter:     the number of tlv structure
 
  Description:          return the number that first tlv structure in input buffer 

  Status:		OK

 -------------------------------------------------------------------------------------------------------------------*/

void DesMacFunction(UWORD eepromAddr,BYTE * cpKey,BYTE cpResult[8],WORD nEnterLen,BYTE mode)
{
	WORD idata i,padNum;	
	BYTE idata j;
	BYTE xdata cpEnter[8];
	
	padNum = nEnterLen % 8 ;	

	if(padNum != 0x00)					           		 // if the data length is not N*8 byte.then must pad some data. 
	{
		SetEepromEX(eepromAddr+nEnterLen,0x00,(8-padNum));			

		nEnterLen = 8*(nEnterLen/8) + 8;
	}	

	memset(cpResult,0x00,0x08);

	for(i=0x00;i < nEnterLen;i=i+0x08)
	{
		ReadEeprom((BYTE *)&cpEnter[0],eepromAddr+i,8);

		for(j=0;j<0x08;j++)
		{
		  	cpEnter[j]=cpEnter[j] ^ cpResult[j];
	    } 

		if(mode == TRIPLEDES)    
		{
			TripleDES(&cpEnter[0],&cpKey[0],&cpResult[0],ECB_Encrypt,0x08,16);
		}
		else
		{
			SingleDES(&cpEnter[0],&cpKey[0],&cpResult[0],ECB_Encrypt,0x08); 
		}
	}	
}

/*
*@description: 获取CATTP 指定的密钥
*
*@param
*
*@description
*
@history
*/
BYTE CATGetRFMKey(BYTE index ,BYTE *key,BYTE mode)
{
	BYTE idata result;
	UWORD xdata fileAddr;
	
	fileAddr = SelectLFFile(DF_STK,LDFile_KeyFile);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	index = index*2;

	if(mode == MODE_KIC)
	{
		index = index-1;
	}

	result = ReadLFBuf(fileAddr,index,0x00,key,0x10);
	
	return result;	
}

/*-------------------------------------------------------------------------------------------------------//

  Function name:	ChipheRe
 
  Input parameter:	Spi
  					Kic
					pointer
					result_pointer
					mode：ENCRYPT/DECRYPT
					lgth
 			       
  Output parameter: the buffer of the key
 
  Description:      加/解密   

  status: 			need test
  
  author :          XuWeiHua      

 -------------------------------------------------------------------------------------------------------------------*/
BYTE CATTPChipheRe(BYTE kic,BYTE * pointer,BYTE * result_pointer,BYTE mode,WORD lgth)
{
	BYTE idata result;
  BYTE idata keyId;
  BYTE idata decryptType;
	BYTE xdata keyVlaue[0x10];

  if( (kic&0x03) != 0x01)                           //that is , KIC(2-1bit)=01, DES 
  {
	 	return Securit_Error;
  }

 	keyId = ((kic&0xF0)>>0x04);                       //get the keyID

	result =CATGetRFMKey(keyId,&keyVlaue[0],MODE_KIC);               //获取解密key   

	if(result == FALSE)
	{
		return Securit_Error; 
	}

  	decryptType = kic &0x0C;                          //get the KIC bit(4-3),that is ,mode

 	switch (decryptType)
	{
		case 0:
			SingleDES(&pointer[0],&keyVlaue[0x00],&result_pointer[0],CBC&mode,lgth);
			break;
		case 4:
			TripleDES(&pointer[0],&keyVlaue[0x00],&result_pointer[0],CBC&mode,lgth,16);          //now, the same with case 4
		  break;
		case 8:	
			TripleDES(&pointer[0],&keyVlaue[0x00],&result_pointer[0],CBC&mode,lgth,24);          //now, the same with case 4
			break;
		default:
			SingleDES(&pointer[0],&keyVlaue[0x00],&result_pointer[0],ECB&mode,lgth);
			break;
	}

   return POP_OK;                                                     		//REQUEST COMPAREING THE DATA	
}


/*--------------------------------------------------------------------------------------------------------------------                                                                                       
  Function name:	IsRFMReceiveAll
  
  input parameter:	fileAddr   ：文件地址

  					packageNum ：批次

  output parameter:	收满所有返回TRUE，否则返回FALSE
                                                                                                                                         
  description:		把数据按照TLV的结构存入文件，文件的开始是数据类型（TLV）包括了批次、已收短信数目、短信总数、文件已用空间
  					
					以后的结构是按照短信的索引作为Tag存放的TLV结构

  author:			Zhuli

  Test:				Caibo
  
  status:		    OK	
 -------------------------------------------------------------------------------------------------------------------*/
//BYTE CATTPIsRFMReceiveAll(UWORD fileAddr, BYTE packageNum,BYTE tag)
BYTE IsRFMReceiveAll(UWORD fileAddr, BYTE packageNum,BYTE tag)
{	
	BYTE  idata i, recNum, totalNum;

	recNum = LDFileRecordFindSub(packageNum, 1,tag);	                           //寻找第一条或者单条	的记录

	if(recNum != FALSE)
	{
		if(tag != LDFileMuliProgess)
		{
			return TRUE;	
		}
	}
	else
	{
		return FALSE;
	}
	
	ReadLFBuf(fileAddr, recNum, 0x02, &totalNum, sizeof(BYTE));
	
	if(totalNum == 0x00)
	{
		return FALSE;
	}	

	for(i=1; i<=totalNum; i++)
	{
		recNum = LDFileRecordFindSub(packageNum, i,tag);                       //判断是否每一条都收到

		if(recNum == FALSE)
		{
			return FALSE;
		}
	}

	return TRUE;	
}


/*
*@Function name:	StoreDataSms
*  
*@input parameter:	bufdata:  UDL开始
*  					        refNumber ：批次号
*  					        seqNumber：索引
*  					        totalNumber ：短信总数
*                   tag:标记 
*
*@return          TRUE:成功/false：失败/FF 存满
*
*@author:			XuWeiHua
*
*@history
*/
//添加tag这个参数  
BYTE StoreDataSms(BYTE * bufdata,BYTE refNumber,BYTE seqNumber,BYTE totalNumber,BYTE tag)
{	
	UWORD xdata fileAddr;
	BYTE  idata recordNo,pos;
	BYTE  idata result,len;
	BYTE  idata recNum,i;
	BYTE  xdata tem[4];
			
	fileAddr = SelectLFFile(DF_STK, LDFile_DownloadFileMuli);  //下行缓存 5E12	

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

    ReadEeprom((BYTE *)&recNum, fileAddr+F_RecNum_Offset, 0x01);	//读取5E12 记录总数	

	recordNo = FALSE;

	for(i=1;i<=recNum;i++)
	{
		ReadLFBuf(fileAddr,i,0,&tem[0],sizeof(tem));

		if((tem[0] == refNumber)&&(tem[1] == seqNumber)&&(tem[2] == totalNumber)&&(totalNumber != 0x01))   //对于级联的下行，收到同一条，抛弃。
		{
			return TRUE;           
		}

		if(((tem[0] == 0x00)&&(tem[1] == 0x00)&&(tem[2] == 0x00)&&(tem[3] == 0x00)) || ((tem[0] == 0xFF)&&(tem[1] == 0xFF)&&(tem[2] == 0xFF)&&(tem[3] == 0xFF)))
		{
			if(recordNo == FALSE)
			{
				recordNo = i;

				break;
			}
		}
	}	

	if(recordNo == FALSE)
	{		
		for(i=1; i<=recNum; i++)
		{
			ReadLFBuf(fileAddr, i, 0x00, &tem[0], sizeof(BYTE));

			if(tem[0] == refNumber)
			{
				continue;
			}	
			
			result = IsRFMReceiveAll(fileAddr, tem[0],tag);

			if(result == FALSE)
			{
				SetLFConst(fileAddr,i, 0x00, 0x00, 0x04);

				recordNo = i;   

				break;	
			}
		}

		if(recordNo == FALSE)          //Is all record is used,clear the first record
		{
			recordNo = 1;   	
		}

	}

	if(bufdata[1] < 2)
	{
		return FALSE;
	}

	len = 3 + 1 + bufdata[0] - (bufdata[1] + 1);                  // length of refNumber,seqNumber,totalNumber,L,CPL to data end   -（02 70 00三个字节）

	pos = bufdata[1] - 2;

	bufdata[pos] = refNumber;                                 // reference number

	bufdata[pos+1] = seqNumber;	                              // sequence number

	bufdata[pos+2] = totalNumber;                             // total number

	bufdata[pos+3] = len - 4;                                 // user data len	

	result = WriteLFBuf(fileAddr, recordNo, 0x00, (BYTE *)&bufdata[pos], len);

	if(result == FALSE)
	{
		return FALSE;
	}

	if(totalNumber == 0x01)	                              // 已收满
	{
		PendingMessageAlert(tag,0x00);	
		return 0xFF;
	}
  
	result = IsRFMReceiveAll(fileAddr,refNumber,tag);

	if(result == TRUE)
	{
		PendingMessageAlert(tag,0x00);	
		return 0xFF;
	}
    else
	{
		return TRUE;
	}	
}



/**
 * @brief	查找上行缓存数据起始位置
 *
 * @param	fileAddr:缓存文件地址
 *			type:0x00->压缩型；0x01->扩展型
 *
 * @return	起始偏移
 * 
 * @author	PL
 * 
 * @history	created at 20121029
 */
BYTE LDFileFindBegain(UWORD fileAddr, BYTE type)
{	
	BYTE idata i, j;
	
	if (type != 0x00)		//找数据起始地址(扩展型)
	{
		for (i = 0x00; i < 0x03; i++)	//复用Mac[0]，表示tag位置
		{
			ReadTFBuf(fileAddr, i, (BYTE *)&j, 0x01);
			
			if (j == TAG_ResponseScriptingTemplateDefinite)		//找到tag
			{
				break;
			}
		}
		
		if (i == 0x03)		//数据有误
		{
			return FALSE;
		}
	}
	else		//压缩型
	{
		i = 0x04;
	}
	
	return i;	
}

/**
 * @brief	下行短信清除
 *
 * @param	IELa:级联短信批次
 *			
 * 
 * @author	PL
 * 
 * @history	created at 20121029
 */
void LDFileClearSMSDownload(BYTE IELa)
{
	UWORD xdata fileAddr;
	BYTE idata index, sumNum;
	BYTE xdata tmp[3];
	
	fileAddr = SelectTFFile(DF_OTA, LDFile_DownloadFileMuli);
		
	ReadEeprom((BYTE *)&sumNum, (UWORD)(fileAddr+F_RecNum_Offset), 0x01);	//读取记录总数
		
	for (index = 0x01; index <= sumNum; index++)
	{
		ReadLFBuf(fileAddr, index, 0x00, (BYTE *)&tmp, 0x03);		//读取批次号
			
		if ((tmp[0] == IELa)&&(tmp[2]!=0x01))		//找到此批次中的短信                     只是针对级联下行清除
		{
			SetLFConst(fileAddr, index, 0x00, 0x00, 0x04);			//清除短信（懒惰删除）
		}
	}		
}

/**
 * @brief	CPL长度检查
 *
 * @param	dataBuf:CPL数据
 *			type:0x00->检测CPL长度;0xXX->拼装CPL(取值为dataBuf长度，另：若取值为0xFF，则表示长度固定为两个字节)
 *
 * @return	CPL长度
 * 
 * @author	PL
 * 
 * @history	created at 20121029
 */
BYTE LDFileCPLCheck(BYTE * dataBuf, BYTE type)
{
	BYTE idata result;
	
	if (type != 0xFF)		//BER情况
	{
			for (result = 0; result < type; result++)	//剔除前面的0
			{
				if (dataBuf[result] != 0x00)
				{
					break;
				}
			}
			
			if (result == type)
			{
				return FALSE;
			}
			
			type = type - result;	//有效数据长度
			
			if (type == 0x01)
			{
				if (dataBuf[result] < 0x80)		//单字节情况
				{
					memcpy((BYTE *)dataBuf, (BYTE *)&dataBuf[result], 0x01);
					
					return  0x01;
				}
				else		//81情况
				{
					memcpy((BYTE *)&dataBuf[1], (BYTE *)&dataBuf[result], 0x01);
					
					dataBuf[0] = 0x81;
					
					return 0x02;
				}
			}
			else	//多字节情况
			{
				memmove((BYTE *)&dataBuf[1], (BYTE *)&dataBuf[result], type);   //由memcpy 改为memmove  20160831
				
				dataBuf[0] = 0x80 + type;
				
				return  ((dataBuf[0]&0x0F)+1);
			}
	}
	else
	{
		return 0x02;
	}	
}

/**
 * @brief	寻找下行缓存文件可用记录（子函数）
 *
 * @param	fileAddr:文件地址
 *			refNumber:批次号
 *			num:第几条（级联），单条或者打开CATTP的下行，则为总数 01
 *
 * @return	检查结果
 * 
 * @author	PL
 * 
 * @history	created at 20121029
 */
 BYTE LDFileRecordFindSub(BYTE refNumber, BYTE num,BYTE tag)	       //参数fileAddr不要，改为函数内选择文件5E12操作，此函数只用来 查找5E12特定记录是否存在 20160818
 {	 
 	BYTE idata index, sumNum;
 	BYTE xdata IELa[4];
	UWORD xdata fileAddr;

	fileAddr = SelectTFFile(DF_OTA, LDFile_DownloadFileMuli);
 
 	ReadEeprom((BYTE *)&sumNum, (UWORD)(fileAddr+F_RecNum_Offset), 0x01);	//读取记录总数
 
 	for (index = 0x01; index <= sumNum; index++)
	{
		ReadLFBuf(fileAddr, index, 0x00, (BYTE *)IELa, 0x04);		//读取批次，序列号，总数，长度

		if((tag ==LDFileSingleProgess)&&(IELa[0] == 0xFF) && (IELa[2] == 0x01) && (IELa[3] != 0x00))   //批次  FF 单条
		{
			return index;	
		}
		else if((tag ==D_RFMSucessTag)&&(IELa[0] == 0xFE) && (IELa[2] == 0x01) && (IELa[3] != 0x00)) //批次  FE   打开CATTP
		{
		 	return index;
		}
		else if((IELa[0] == refNumber) && (IELa[1] == num) &&(IELa[2] != 0x01) && (IELa[3] != 0x00)&& (tag ==LDFileMuliProgess))		//找到记录  级联
		{
			return index;
		}
		else
		{
			
		}	
	}
	
	return FALSE;
 }

/**
 * @brief	远程文件管理下行counter检查
 *
 * @param	counter:下行的counter
 *
 * @return	检查结果
 * 
 * @author	PL
 * 
 * @history	created at 20121029
 */
BYTE LDFileCheckCounter(BYTE counter[5], BYTE firstSPI,BYTE counterId)
{	
	UWORD xdata fileAddr;
	BYTE xdata counterIn[5];
	BYTE idata mode;
	
	mode = firstSPI & 0x18;		//获取SPI第一字节的bit4,bit5
	
	if((mode & 0x10) != 0x00)	//检查counter
	{
		fileAddr = SelectLFFile(DF_OTA, LDFile_CounterFile);
		
		if (fileAddr == FALSE)		//暂时返回安全错误
		{
			return UnSecurityError;
		}
		
		ReadLFBuf(fileAddr, counterId,0, (BYTE *)counterIn, 0x05);		//读取卡内counter
		
		IncreaseCounter((BYTE *)counterIn, 0x05);		//counter自增
		
		if (mode == 0x18)		//下行counter必须只比卡内多1
		{			
			if (memcmp_p((BYTE *)counter, (BYTE *)counterIn, 0x05) > EQUAL)
			{
				return CNTRHigh;
			}
		}
		
		if (memcmp_p((BYTE *)counter, (BYTE *)counterIn, 0x05) < EQUAL)	//下行counter必须比卡内counter大
		{
			return CNTRLow;
		}
				
		WriteLFBuf(fileAddr,counterId, 0x00,(BYTE *)&counter[0],0x05);	//写入新counter
	}
	
	//不检查counter
	
	return FALSE;	

} 



/**
 * @brief	远程文件管理下行SPI检查
 *
 * @return	检查结果
 * 
 * @author	PL
 * 
 * @history	created at 20121029
 */
BYTE LDFileSPICheck(BYTE firstSPI)
{	
	BYTE idata check1, check2;
	
	check1 = firstSPI & 0x18;			//检查counter安全级别
	
	check2 = LDFILEMLSD & 0x18;
	
	if (check1 < check2)
	{
		return OtherError;
	}
	
	check1 = firstSPI & 0x04;			//检查数据加密安全级别
	
	check2 = LDFILEMLSD & 0x04;
	
	if (check1 < check2)
	{
		return OtherError;
	}
	
	check1 = firstSPI & 0x03;			//检查校验算法安全级别
	
	check2 = LDFILEMLSD & 0x03;		
	
	if (check1 < check2)
	{
		return OtherError;
	}
	
	return FALSE;	
}


/**
 * @brief	下行短信处理
 *
 * @param	tag:短信类型
 *
 * @return	处理结果
 * 
 * @author	PL
 * 
 * @history	created at 20121029
 */
BYTE LDFileMuliPPHanlder(BYTE refNumber,BYTE tag)
{	
	UWORD xdata fileAddr, fileAddr2,fileBodyAddr2;
	BYTE idata index, result, PCNTR;
	BYTE xdata IELa[3], CPL[4], counter[5],key[0x10];
	BYTE xdata mac[8];		//暂不支持DS情况
	BYTE xdata sourceHead[8];	//CHL-TAR（暂时认为CHL为1个字节）
	BYTE idata counterId;
	BYTE idata processResult;
	BYTE idata i,ccLgth,k;
	WORD idata decryLen,offset,dataOffset,dataLen;

	PCNTR = 0x00;

	processResult = POR_OK; //表示结果正确

	fileAddr = SelectLFFile(DF_OTA, LDFile_DownloadFileMuli);    //下行保存文件 5E12   不判断 fileAddr == FALSE      if fileAddr == FALSE 不会进来

	fileAddr2 = SelectTFFile(DF_OTA, LDFile_RelaxFile);		//选择执行缓存文件 5E03
			
	if (fileAddr2 == FALSE)
	{		
		return OtherError;
	}

	fileBodyAddr2 = getFileContentAddr(fileAddr2);		   //得到文件体地址

	index = LDFileRecordFindSub(refNumber, 0x01,tag);	  //查找第一条记录的位置

	if(index == FALSE)
	{
		return FALSE;	
	}
	
	ReadLFBuf(fileAddr, index, 0x00, (BYTE *)IELa, 0x03);	//获取级联头信息(批次、索引 总数、)
	
	
	ReadLFBuf(fileAddr, index, 6, (BYTE *)&sourceHead[0], 0x08);	//获取CHL-TAR  		6字节=批次 + 索引 +总数+长度+CPL（暂时认为2字节）


	if(((sourceHead[3]>>4)&0x0F)!=0x00)									//使用了KIC-Key
	{
		counterId = (sourceHead[3]>>4)&0x0F;	
	}
	else
	{
		if(((sourceHead[4]>>4)&0x0F)!=0x00)							//使用了KID-Key
		{
			counterId = (sourceHead[4]>>4)&0x0F;	
		}	
	    else													   //KIC-Key与KID-Key均不使用
		{
			counterId = 0x01; 
		}
	}
	ccLgth = sourceHead[0] - 0x0D;		//保存Mac Len

	G_CurrentCommandOffset = 0x03 + sourceHead[0];   //指向第一条APDU指令偏移   CPL 暂时认为 2字节 +CHL 本身长度  =3
			
	result = LDFileSPICheck(sourceHead[1]);		//安全级别检测
	
	if (result != FALSE)		
	{
		if(processResult == POR_OK)
		{
			processResult = OtherError;	                    //记录错误，程序继续往下执行
		}
	}

	//if(((sourceHead[3]&0x0C) == 0x00)||((sourceHead[4]&0x0C) == 0x00)) //kid kid不支持单des算法
	//kid kid不支持单des算法
	//Ciphering前提下判断加密模式不是DES in CBC mode和DES in ECB mode
	//Cryptographic和Checksum Digital Signature前提下判断MAC模式不是DES in CBC mode	    
	if(((((sourceHead[3]&0x0C) == 0x00)||((sourceHead[3]&0x0C) == 0x0C))&&((sourceHead[1]&0x04) == 0x04))     
	||(((sourceHead[4]&0x0C) == 0x00)&&((sourceHead[1]&0x03) != 0x00)&&((sourceHead[1]&0x03) != 0x01)))         
	{
		//LDFileClearSMSDownload(IELa[0], tag);		//清除此批次的下行短信缓存		
		//return UnSecurityError;	//标记返回安全级别不符
		if(processResult == POR_OK)
		{
			processResult = UnSecurityError;	                    //记录错误，程序继续往下执行
		}
	}

	dataOffset = 0x00;

	for(i=0x01;i<=IELa[2];i++)	                            //读取5E12批次数据到5E03
	{
		ReadLFBuf(fileAddr, index, 0x03, (BYTE *)RecData, 0x01);	//获取此条有用数据长度

		result = 4;	        

		ReadLFBuf(fileAddr, index, result, (BYTE *)&RecData[1], RecData[0]);	//获取此条有用数据

		WriteTFBuf(fileAddr2,dataOffset,(BYTE *)&RecData[1] ,RecData[0]);	   //数据写到5E03

		dataOffset += RecData[0];                                                //	dataOffset的长度包括了 CPL自身的长度

		if (IELa[1] == IELa[2])		//此批次已全部处理完
		{
			break;
		}

		IELa[1]++;		//偏向下一条

		index = LDFileRecordFindSub(IELa[0], IELa[1],tag);	//找到下一条的记录号   i
		
		if (index == FALSE)
		{

			if(processResult == POR_OK)
		    {
			   processResult = CipheringError;	                    //记录错误，程序继续往下执行	  等到数据解密后才返回错误信息
		    }
		}
		
	}			  
				
	if(tag != LDFileMuliProgess)
	{
		  SetLFConst(fileAddr, index, 0x00, 0x00, 4);						//清除此单条FF和打开cattp  FE 这两条下行缓存	
	}
	
	decryLen = dataOffset -10; //dataOffset- 2 =CPL   解密从CNTR开始，CHL + SPI+ KIC+ KID +TAR =8
		
	if ((sourceHead[1] & 0x04) != 0x00)		//数据短信解密
	{
		if((decryLen & 0x07) != 0x00)    //需要解密的数据长度  为8的整数倍
		{
			if(processResult == POR_OK)
			{
				processResult = CipheringError;	 //记录错误，程序继续往下执行
			}	
		}

		offset = 10;	  //待解密数据的起始偏移，从CNTR开始 CNTR PCNTR CC DATA	  8+2

		k = 0;
		
		while(decryLen>0)
		{
			dataLen = (decryLen <= 240)?decryLen:240;   //每次最大读取240字节进行解密
			
			ReadTFBuf(fileAddr2,offset,&RecData[0], dataLen);

			if((sourceHead[3] & 0x0C) != 0x0C)
			{
				if(k == 0)                          //解密第一块的时候  保存最后八个字节，这八个字节等到解密第二个块才会用到第一块的最后八个字节。同时第二块的最后八个字节也要为第三块解密，保存最后的八个字节。
				{
					memcpy(&key[0],&RecData[dataLen-8], 0x08);  //保存解密前的最后8字节数据	    
				}
				else
				{
					memcpy(&key[8],&RecData[dataLen-8], 0x08);  //保存解密前的最后8字节数据	
				}
				
			}
			
			result = CATTPChipheRe(sourceHead[3],&RecData[0],&tempBuf[0],DECRYPT,dataLen); //从CNTR开始解密

			if(result != POP_OK)
			{

				if(processResult == POR_OK)
		    	{
			   		processResult = CipheringError;	                    //记录错误，程序继续往下执行
		    	}
	
			}

			if(k == 0x00)
		 	{
		 		k ++;

				PCNTR = tempBuf[5];
		 	}
		 	else
		 	{
		 		if((sourceHead[3] & 0x0C) != 0x0C)
		 		{
			 		for(i = 0; i <8;i++)
					{
						tempBuf[i] = tempBuf[i]^key[i];
					}		 
					 
					memcpy((BYTE  *)&key[0], (BYTE   *)&key[8], 8);
				}
			}

			WriteTFBuf(fileAddr2,offset,&tempBuf[0], dataLen);

			offset += dataLen;
			
			decryLen -=	dataLen;
		}
		  
		
	}

	ReadTFBuf(fileAddr2,10, &counter[0],5);  //读取counter      CPL长度暂时认为 2	  CHL + SPI+ KIC+ KID +TAR =8

	ReadTFBuf(fileAddr2,SMS_Head_CplToPcntr, &mac[0], ccLgth);  //读取mac

	ReadTFBuf(fileAddr2, 0x00,&tempBuf[0],SMS_Head_CplToPcntr);	 //CPL CHL SPI KIC KID TAR CONTR PCNTR = 16

	WriteTFBuf(fileAddr2, ccLgth,&tempBuf[0],SMS_Head_CplToPcntr);  //将CPL后移ccLgth个字节计算mac

	if(processResult!=POR_OK)                        //已经解密  移动数据  可以提取到上行需要的返回数据
	{
		return processResult;
	}


	if((sourceHead[1]&0x03) == 0x02)	                   //spi
	{
    	result = CATGetRFMKey(((sourceHead[4]&0xF0)>>0x04),&key[0],MODE_KID);                 //获取计算mac key   ，mac计算从CPL开始，不包括CC数据
		
		if(result == FALSE)
		{
			return Securit_Error;
		}

	  if((sourceHead[4] & 0x03) == 0x01)		          //kid
		{
			//todo： 以下第一行仿真器出错 -jiangqingbin-20211129
			if((sourceHead[4] & 0x0C) == 0x00)				
			{			 
				DesMacFunction(fileBodyAddr2+ccLgth,&key[0x00],tempBuf,dataOffset-ccLgth,SINGLEDES);  //dataOffset-8 = CPL- CClength +2   CPL本身的字节长度包括在dataOffset,计算MAC从CPL开始					   	
			}		
			else if(((sourceHead[4]&0x0C)==0x04)||((sourceHead[4]&0x0C)==0x08))
			{			    
				DesMacFunction(fileBodyAddr2+ccLgth,&key[0x00],tempBuf,dataOffset-ccLgth,TRIPLEDES);

			}
			else
			{
			   return Securit_Error;
			}
		}

		if(memcmp(&tempBuf[0],&mac[0],ccLgth) != EQUAL)
		{
			return CC_Failed;		 
		}
	}

	result = LDFileCheckCounter((BYTE *)counter, sourceHead[1],counterId);			//检查counter				
		
	if (result != FALSE)				
	{		
		return result;	//标记counter错误
	}
	
	dataOffset -= PCNTR;	//剔除填充位

	dataOffset -= 2 ;      //减去CPL长度   dataOffset包括了 CPL本身的长度
	
	CPL[0] = (BYTE)((dataOffset >> 0x08) & 0xFF);	//添加长度
	
	CPL[1] = (BYTE)(dataOffset & 0xFF);
	
	WriteTFBuf(fileAddr2, 0x00, (BYTE *)CPL, 0x02);		//写入缓存文件前端   保存数据的长度
	
//	LDFileClearSMSDownload(IELa[0], tag);		//清除此批次的下行短信缓存
	return PoROK;	//标记可以执行	
}
/**
 * @brief	拼装PoR
 *
 * @param	dataBuf:附加返回数据,LV
 *			headBuf:头数据，LV
 *			outBuf:输出数据,LV
 *			sum:总条数（为0x01时，认为非级联）
 *			num:级联中序号（不级联情况使用0x01）
 *
 * @return	完成情况 
 *
 * @author	PL
 * 
 * @history	created at 20121029，暂不支持级联情况
 */
BYTE LDFileSMSPoR(BYTE * dataBuf, BYTE * headBuf, BYTE * outBuf, BYTE sum, BYTE num)
{	
	UWORD xdata fileAddr;
	
	if (sum > 0x01)		//级联
	{
		fileAddr = SelectTFFile(DF_OTA, LDFile_RFMPerso);		 		
		
		if (fileAddr == FALSE)
		{
			return FALSE;
		}
		
		if (num == 0x01)	//起始短信
		{
			outBuf[0] = 0x08;
			
			outBuf[1] = 0x07;
			
			outBuf[7] = 0x71;
			
			outBuf[8] = 0x00;
		}
		else
		{
			outBuf[0] = 0x06;
			
			outBuf[1] = 0x05;
		}
		
		outBuf[2] = 0x00;
			
		outBuf[3] = 0x03;
		
	
		ReadTFBuf(fileAddr, 0x00, (BYTE *)&outBuf[4], 0x01);
	
		if (sum == num)			//此次批次全部发完，批次更改
		{
			SetTFConst(fileAddr, 0x00, (outBuf[4]+1), 0x01);
		}
		
		outBuf[5] = sum;		//总条数
		
		outBuf[6] = num;		//条号
	}
	else		//不级联	   TS-101181/电信需求白皮书  15.4.1.2数据短信响应头格式
	{
		outBuf[0] = 0x03;
		
		outBuf[1] = 0x02;
		
		outBuf[2] = 0x71;
		
		outBuf[3] = 0x00;
	}
	
	if (headBuf != NULL)		//添加PoR头
	{
		AddVlaue((BYTE *)outBuf, (BYTE *)headBuf);
	}
	
	if (dataBuf != NULL)		//添加附加数据
	{
		AddVlaue((BYTE *)outBuf, (BYTE *)dataBuf);
	}
		
	return TRUE;
}

#if 0
/**
 * @brief	Sumbit方式发送上行PoR
 *
 * @param	dataBuf:用户数据及长度
 * 
 * @author	PL
 * 
 * @history	created at 20121029
 */
void LDFileSMSSubmit(BYTE * dataBuf)
{
	BYTE xdata address[16];
//	UWORD xdata fileAddr;
	BYTE idata i,tmp;
	
	memset(RecData, 0x00, sizeof(RecData));

		
	RecData[0] = ReadVarible(LDFileTPOATag, (BYTE *)&RecData[1]);	//读取TPOA
	
	/*
	if (RecData[2] != 0x81)		//增加81
	{
		if (RecData[2] != 0x91)
		{
			RecData[0]++;
			
			memmove((BYTE *)&RecData[3], (BYTE *)&RecData[2], (RecData[0]-2));
		}
		
		RecData[2] = 0x81;
	}
	*/
	
	for (i = 0x03; i <= RecData[0]; i++)	//高地4位翻转
	{
		tmp = RecData[i] & 0x0F;
		
		RecData[i] = (RecData[i] >> 0x04) & 0x0F;
		
		RecData[i] = ((tmp << 0x04) & 0xF0) | RecData[i];
	}
	
//	ClearVariable(LDFileTPOATag);		
	
/*	
	fileAddr = SelectLFFile(DF_OTA, LDFile_TPOAFile);	//读取服务器地址
	
	if (fileAddr == FALSE)
	{
		return;
	}
	
	ReadLFBuf(fileAddr, 0x01, 0x00, (BYTE *)RecData, 0x01);		//读取TPDA
	
	ReadLFBuf(fileAddr, 0x01, 0x01, (BYTE *)&RecData[1], RecData[0]);
*/
	
	RecData[++RecData[0]] = 0x7F;		//PID
	
	RecData[++RecData[0]] = 0xF6;		//DCS
	
	RecData[0]++;
	
	memcpy((BYTE *)&RecData[RecData[0]], (BYTE *)&dataBuf[0], (dataBuf[0] + 1));	//UDL+UD
	
	RecData[0] += dataBuf[0];
	
	memcpy(tempBuf, RecData, (RecData[0] + 1));
	
	if (FALSE != LoadSCAddress((BYTE *)&address[0]))	                   	// 读短信中心号码
	{
		//commQualify=0x80表示“the  CHARi  field  in  the  User  Data  Subparameter  contains  a  User  Data  Header”
		cephSendShortMessageSingle(0x80, NULL, (BYTE *)address, (BYTE *)tempBuf);	
	}
	else
	{
		//commQualify=0x80表示“the  CHARi  field  in  the  User  Data  Subparameter  contains  a  User  Data  Header”
		cephSendShortMessageSingle(0x80, NULL, NULL, (BYTE *)tempBuf);		//发送上行短信
	}
}
#endif
/**
 * @brief	deliver方式发送上行PoR
 *
 * @param	dataBuf:用户数据及长度
 *			errorCode:错误编码
 * 
 * @author	PL
 * 
 * @history	created at 20121029
 */
void LDFileSMSDeliver(BYTE * dataBuf, BYTE errorCode)
{
	memcpy((BYTE *)&RecData[0],(BYTE *)&dataBuf[1],dataBuf[0]);
	
	if(G_CommandMode == Command_Mode_2G)	//SW1设置
	{
		SW1 = (errorCode == 0x00)?0x9F:0x9E;
		SW2 = dataBuf[0];		//SW2	
	}
	else
	{
		if(errorCode == 0x00)
		{
			SW1 = 0x61;
			SW2 = dataBuf[0];		//SW2	
		}
		else                        //按照102221,C.1.7,POR deliver error 应该在00C20000XX后返回“6200”，
		{
			SW1 = 0x62;
			SW2 = 0x00;		
		}
	}

	//SW2 = dataBuf[0];		//SW2	
	
	G_LDFileDeliver = TRUE;	

	G_CurrentReponse[G_ChannelNum] = dataBuf[0];	//数据长度

	return ;
}



BYTE JiLianSMSNum(WORD extraLen)
{
	BYTE idata smsNum;
	
	if(extraLen<125)
	{
		smsNum = 0x01;		
	}
	else
	{
		smsNum = ((extraLen-119)/(140-6))+2;		//此时至少两条	
	}

	return smsNum;
		

 
}

/**
 * @brief	远程文件管理PoR处理过程
 *
 * @param	errorData:类型
 * 
 * @author	PL
 * 
 * @history	created at 20121029
 */
 void LDFilePoR(BYTE errorTag)
{
	BYTE idata backLen;
	BYTE idata secondSPI,kind, type, sum, num; 
	UWORD xdata fileAddr, fileAddr2;
	BYTE xdata headBuf[141];
	WORD xdata len, result;
	
	fileAddr2 = SelectTFFile(DF_OTA, LDFile_RelaxFile);	//选择执行缓存文件 5E03
	
	if (fileAddr2 == FALSE)
	{
		return;
	}
	
	ReadTFBuf(fileAddr2, G_CurrentCommandOffset-12, (BYTE *)&secondSPI, 0x01);	//读取SPI第二个字节
	
	ReadTFBuf(fileAddr2, G_CurrentCommandOffset-8, (BYTE *)&kind, 0x01);	//读取TAR第二字节
	
	if ((secondSPI & 0x03) != 0x01)			//PoR存在检查
	{
		if (((secondSPI & 0x03) != 0x02) || (errorTag == PoROK))
		{			
			return;
		}
	}
	
	fileAddr = SelectTFFile(DF_OTA, LDFile_UpFile);		//选择上行缓存文件		//0x5E20
	
	if (fileAddr == FALSE)
	{		
		return;
	}
	
	ReadTFBuf(fileAddr, RFM_RES_LEN_OFF, (BYTE *)&headBuf, 0x02);		//获取附加数据长度	    5E20   RPL RHL TAR COUNTR PCNTR STATUS + 08  mac  最大长度
	
	len = BuildWord(&headBuf[0]);	//(WORD)((headBuf[0] << 0x08) & 0xFF00) | (headBuf[1] & 0xFF);

	sum = JiLianSMSNum(len);
	
	if((secondSPI & 0x20) == 0x00)		//若为SMS-Deliver模式,sum长度固定为1,修改缺陷219
	{
		sum = 0x01;
	}

	type = ((len != 0x00) ? 0x01 : 0x00);
	
	len += 25;		//指向数据尾21+4   21(RPL +RHL +TAR +COUNTR + PCNTR +TAG +STATUS +CC(8))   4(RFU RFU L1  L2)
	
	result = LDFileSMSRPS(fileAddr2, (BYTE *)&headBuf[0], errorTag, type);	//拼装头数据(第一条)  result=140 则表示下一条数据的偏移位置
	
	if (result == FALSE)
	{		
		return;
	}
	
	num = 0x01;	
	
	if ( FALSE == LDFileSMSPoR(NULL, (BYTE *)headBuf, tempBuf, sum, num) )		//拼装上行PoR
	{
		return;
	}

	LDCleanUpdateFile();		//拼装完成后,清空上行缓存文件
	
	if(((secondSPI & 0x20) == 0x00)&&(tempBuf[0]>0x80))		 		//若为SMS-Deliver模式,tempBuf[0]最大长度取0x80(0x8C-TPDA最大长度0x0C),tempBuf[5]长度也要相应改变,修改缺陷219
	{
		  tempBuf[0] = 0x80;
		  tempBuf[5] = tempBuf[0]-5;
		  tempBuf[5+13] = 0x62;		 	//POR返回的状态字由9000改为62F1,表示数据被截断,偏移13(RHL +TAR +COUNTR + PCNTR +TAG +STATUS+RFMComNUM+SW1)
		  tempBuf[5+13+1] = 0xF1;		
	}
	
	if(tempBuf[0]>0x8C)
	{
		backLen = tempBuf[0] - 0x8C;  	//表示下一次读取2700/5E20文件要向前移多少个字节
			
		result = result - backLen;	   //得到新的result读取位置
		
		tempBuf[0] = 0x8C;		 		//单条短信最多取0x8C个字节发送
		
	}
	
	if ((secondSPI & 0x20) != 0x00)		//SMS-Submit
	{		
		LDFileSMSSubmit((BYTE *)tempBuf);
	}
	else				//SMS-Deliver	    目前也是 140个字节，只回复一条
	{

		LDFileSMSDeliver((BYTE *)tempBuf, errorTag);
		
		return;
	}
	
	if (result != 0xFF)
	{
		while (num != sum)
		{
			num++;
			
			if ((len - result) > 140)	//获取读取数据长度
			{
				headBuf[0] = 140;
			}
			else
			{
				headBuf[0] = len - result;
			}
			
			ReadTFBuf(fileAddr, result, (BYTE *)&headBuf[1], headBuf[0]);	//读数据
			
			if ( FALSE == LDFileSMSPoR(NULL, (BYTE *)headBuf, tempBuf, sum, num) )		//拼装上行PoR
			{
				return;
			}
			
			backLen = 0x00;						//初始为0
			
			if(tempBuf[0]>0x8C)
			{
				backLen = tempBuf[0] - 0x8C;  	//表示下一次读取2700/5E20文件要向前移多少个字节
					
				tempBuf[0] = 0x8C;		 		//单条短信最多取0x8C个字节发送
			}

			LDFileSMSSubmit((BYTE *)tempBuf);		//发送上行
			
			//result += headBuf[0];		//向下偏移
			result = result + headBuf[0]- backLen;		//向下偏移


		}
	}
	
	ClearVariable(LDFileTPOATag);	  //上行短信发送完成后清除变量

	G_CurrentCommandOffset =  FALSE;   //上行发送完之后清除 偏移变量
}
/**
 * @brief	上行数据处理
 *
 * @param	fileAddr:执行缓存文件地址
 *			dataBuf:结果数据（级联时表示第一条）(LV)
 *			errorTag:错误编码
 *			mode:0x00->不带附加数据;0x01->带附加数据
 *
 * @return	下一条指向位置(不带附加数据时，只表示是否处理完成)
 * 
 * @author	PL
 * 
 * @history	created at 20121029
 */
BYTE LDFileSMSRPS(UWORD fileAddr, BYTE * dataBuf,BYTE tag, BYTE mode)
{
	UWORD xdata fileAddr2,fileBodyAddr2;
	BYTE xdata headBuf[49], infor[11], len[4],key[0x10];
	WORD xdata length,decryLen,dataLen,moveoffset,offset;
	BYTE idata result, pCntr,k,i;
	BYTE idata outcome;
                                                     //  0	1    2  3-5 6-10
	ReadTFBuf(fileAddr, G_CurrentCommandOffset-12, (BYTE *)infor, 11);	//SPI_2,KIC,KID,TAR,COUNTER	    G_CurrentCommandOffset	指向5E03数据其实地址  -12  指向SPI_2

	fileAddr2 = SelectTFFile(DF_OTA, LDFile_UpFile);	//选择结果缓存文件

	fileBodyAddr2 = getFileContentAddr(fileAddr2);
	
	if (fileAddr2 == FALSE)
	{
		return FALSE;
	}
			
	if (mode == 0x01)
	{
		ReadTFBuf(fileAddr2, RFM_RES_LEN_OFF, (BYTE *)&len[1], 0x02);		//获取附加数据长度	  RPL + RHL +TAR +COUNTR + PCNTR +STATUS +CCLength(留8字节) +RFU +RFU
		
		length = BuildWord(&len[1]); //(WORD)(((len[1] << 0x08) & 0xFF00) | (len[2] & 0xFF));

		if ((infor[0]&0x20) == 0x00)	             //执行ＲＦＭ 压缩格式时候（扩展格式在执行下行时候已经截取数据） deliver  响应过长，只发送一条，后续数据抛弃处理，发送完返回9000，不再返回61XX，根据测试部 陈科，连续C0取数据影响手机兼容性 
		{
			if (length >0x77)
			{
				length = 0x77;

				SetWord((BYTE *)&len[1],length);

				WriteTFBuf(fileAddr2, RFM_RES_LEN_OFF, (BYTE *)&len[1], 2);		//长度写回
			}		    
		}
		
		SetTFConst(fileAddr2, (length+4)+21, 0x00, 0x08);	//数据结尾填充8字节0，方便以后使用
		
		if (infor[4] == 0x01)		//扩展型另类处理，因响应数据里存储的RFU RFU L1 L2 V 结构，此处将扩展型的响应tag添加进来，变成RFU RFU tag L v  or   rfu tag L1 L2 V    or   TAG 82 L1 L2 V
		{
			len[0] = LDFileCPLCheck((BYTE *)&len[1], 0x02);	//BER变换
			
			length = length + len[0] + 1;    //1  tag
			
			headBuf[0] = TAG_ResponseScriptingTemplateDefinite;		//扩展型TAG,复用headBuf
			
			memcpy((BYTE *)&headBuf[1], (BYTE *)&len[1], len[0]);	//填入长度
			
			WriteTFBuf(fileAddr2, (3-len[0])+21, (BYTE *)headBuf, (len[0]+1));		//填入附加数据变换    3-len[0]  偏离两个 RFU 字节的大小

			moveoffset = 3-len[0];  //前面的 RPL RHL TAR COUNTR PCNTR STATUS （此时还没有mac字节）  
		}
		else
		{
			moveoffset = 2+2;          //不是扩展型，则覆盖前面的两个 RFU字节  + 2  L1 L2	
		}
	}
	else
	{
		length = 0;
	}
	
	if(tag == PoROK)
	{
		headBuf[0] = infor[0] & 0x0C;		//MAC包含字节数,复用headBuf ; CC = 8 ,RC= 4 暂时不支持DS  b4b3=11 DS
	}
	else
	{
		headBuf[0] = 0x00;
	}
	
	if (headBuf[0] > 0x08)
	{
		return FALSE;
	}
	
	length += (headBuf[0] + 11);		//从RHL开始长度.11字节包含:RHL TAR CNTR PCNTR Statuscode
	
	if ((infor[0]&0x10) != 0x00)		//POR是否有加密
	{
		if(((length-4)& 0x07) == 0x00)	   //加密数据从CNTR开始,4表示减去RHL TAR的长度
		{
			pCntr = 0x00;
		}
		else
		{
			pCntr = 8 - ((length-4)& 0x07);	//PCNTR,加密数据从CNTR开始,4表示减去RHL TAR的长度,计算补位
		}
	}
	else
	{
		pCntr = 0x00;
	}
	
	length += pCntr;		//添加补充字节长度
	
	len[1] = (BYTE)(length >> 0x08) & 0xFF;	//RPL
	
	len[2] = (BYTE)(length & 0xFF);
	
	len[0] = LDFileCPLCheck((BYTE *)&len[1], 0xFF);	//BER变换02
	
	memcpy((BYTE *)&dataBuf[1], (BYTE *)&len[1], len[0]);	//导入RPL
	
	result = len[0] + 1;
	
	dataBuf[result++] = headBuf[0] + 10;		//导入RHL
	
	
	memcpy((BYTE *)&dataBuf[result], (BYTE *)&infor[3], 0x08);	//导入TAR,COUNTER
	
	result += 0x08;
	
	dataBuf[result++] = pCntr;		//导入PCNTR
	
	dataBuf[result++] = tag;		//导入返回状态
	
	dataBuf[0] = result - 1;

	WriteTFBuf(fileAddr2, moveoffset+8, (BYTE *)&dataBuf[1], dataBuf[0]);		//把 RPL RHL TAR CNTR PCNTR Statuscode	 //前面的 RPL RHL TAR COUNTR PCNTR STATUS （此时还没有mac字节）  需要移动的长度   8  预留的mac字节长度 写到5E20 和数据一起算mac

	if((tag == PoROK)&&(headBuf[0] == 0x08))	  
	{
	   outcome = CATGetRFMKey(((infor[2]&0xF0)>>0x04),&key[0],MODE_KID);

	   if(outcome == FALSE)
	   {
	   		return FALSE;               	
	   }

	   if((infor[2] & 0x03) == 0x01)                          			//DES
       { 
			if((infor[2]&0x0C)==0x00)
			{			    		
				DesMacFunction(fileBodyAddr2+moveoffset,&key[0x00],(BYTE *)&headBuf[1],length+len[0],SINGLEDES);	   	
            	
			}
			else if(((infor[2]&0x0C)==0x04)||((infor[2]&0x0C)==0x08))
			{			    
				DesMacFunction(fileBodyAddr2+moveoffset,&key[0x00],(BYTE *)&headBuf[1],length+len[0],TRIPLEDES);
			}
			else
			{
			    return FALSE;     //return 
            }
	   }
		
	}
	else		//POR不OK的时候，上行都是明文POR 
	{
		headBuf[0] = 0x00;
	}
		
	memcpy((BYTE *)&dataBuf[result], (BYTE *)&headBuf[1], headBuf[0]);		//导入MAC
	
	result += headBuf[0];	//偏移增加
	
	dataBuf[0] += headBuf[0];	//总长度增加
	
	length += len[0];	//计算加上RPL后的总长度

	if(headBuf[0]  == 0x08)       //mac = 8 bytes  CC
	{
		offset = 0x00+ moveoffset;	
	}
	else if(headBuf[0]  == 0x04)    //mac = 4 bytes  RC 
	{
		offset = 0x04 + moveoffset ;
	}
	else                            //mac = 0 byte   POR不OK的时候，上行都是明文POR
	{
		 offset =0x08 + moveoffset;
	}
	
	WriteTFBuf(fileAddr2, offset, (BYTE *)&dataBuf[1], dataBuf[0]);		//把 RPL RHL TAR CNTR PCNTR Statuscode  CC 	  写到5E20 和数据一起

	moveoffset =  offset ;//保留偏移长度 

	if ((infor[0]&0x10) != 0x00)		//有加密
	{
		length -= (4+len[0]);	//需加密数据长度，减去RPL RHL TAR的长度
		
		decryLen = length;

		k=0;

		offset += 6;       // sizeof(RPL RHL TAR) =6

		while(decryLen>0)
		{
			dataLen=(decryLen <= 48)?decryLen:48;   //每次最大读取240字节进行解密

			ReadTFBuf(fileAddr2,offset,&tempBuf[0], dataLen);

//			if(k == 0x00)
//			{
//				memset((BYTE *)&headBuf[1],0x00,0x08);							
//			}

			if(((infor[2]&0x0C)!=0x0C)&&(k != 0x00))          //第一次与0 异或 不需要，第二块开始与第一块最后八字节异或
			{
				for(i=0;i<8;i++)
				{
					tempBuf[i] = tempBuf[i]^headBuf[i+1];
			    }
			}
			
			outcome = CATTPChipheRe(infor[2],&tempBuf[0],&tempBuf[0],ENCRYPT,dataLen); //从CNTR开始加密

			WriteTFBuf(fileAddr2,offset,&tempBuf[0], dataLen);       //加密数据写回
			
			memcpy(&headBuf[1],&tempBuf[dataLen-8], 0x08);  //保存加密后的最后8字节数据	

			offset += dataLen;

			decryLen -= dataLen;
			
			k++;						
		}

		
		length += (4+len[0]);	//添加不需加密数据长度
	}
	
	if (mode == 0x01)		//添加附加数据
	{

			
		if (length > 140)
		{
			ReadTFBuf(fileAddr2, moveoffset+dataBuf[0], (BYTE *)&dataBuf[result], (140-dataBuf[0]));	//读数据    moveoffset数据开始位置	   +1跳过长度字节
			result = moveoffset + 140;	//指向下一次读取的位置
			
			dataBuf[0] = 140;		//此次返回数据长度，如果存在多条短信，则在本函数外进行拼装，返回值(FF or 140)用于判断当前是否为多条短信
		}
		else
		{
			ReadTFBuf(fileAddr2, moveoffset+dataBuf[0], (BYTE *)&dataBuf[result], (length-dataBuf[0]));	//读数据
			dataBuf[0] = length;		//此次返回数据长度
		
			result = 0xFF;			//说明数据已经处理完
		}
	}
	else
	{
		result = 0xFF;
	}
	
	return result;
}
/**
 * @brief	远程文件管理标记处理函数
 *
 * @param	tag:标记类型
 * 
 * @author	PL
 * 
 * @history	created at 20121029
 */
void LDFileProgess(BYTE tag)
{
	BYTE idata result, recNum;
	BYTE idata i;
	UWORD xdata fileAddr;
	BYTE  xdata rfmDataInfo[3];
	BYTE idata isTriHttp;
	
	isTriHttp = NOTGPAnnexBTritag;
	
	G_RFMAllReceived = FALSE;		//处理前恢复标记字节
	
	fileAddr = SelectLFFile(DF_OTA, LDFile_DownloadFileMuli);        //下行缓存5E12

	if (fileAddr == FALSE)
	{
		return;
	}

	ReadEeprom((BYTE *)&recNum, (UWORD)(fileAddr+F_RecNum_Offset), 0x01);	//读取5E12 记录总数
	
	for(result = FALSE,i = 1;i <= recNum;i++)                            //找一个收全的批次
	{
		ReadLFBuf(fileAddr,i,0,(BYTE *)&rfmDataInfo[0],3);

		if(IsRFMReceiveAll(fileAddr,rfmDataInfo[0],tag) == TRUE)
		{
			result = TRUE;
			break;
		}
	}

	if(result == FALSE)
	{
		return;	
	}

	result = LDFileMuliPPHanlder(rfmDataInfo[0],tag);		//头处理
//=====================不在解密里面清除下行？级联的情况在解密之后或者解密出错退出后清除批次号、索引、序列号、长度；单条或者打开CATTP的下行，则解密过程中读取数据后，清除
    if(tag == LDFileMuliProgess)
	{
    	LDFileClearSMSDownload(rfmDataInfo[0]);	   //解密之后清除下行，不管解密成功或者失败
	}

	if (result != PoROK)
	{
		if((tag!=0xA3)
#if GP__AMENDMENT__B__FOR_MASK	
		&&(G_HTTPSStatus.HTTPTriggerMode == FALSE)
#endif
		)						//非CATTP,非HTTP
		{
			LDFilePoR(result);		//错误PoR
		}			
		return;
	}

	if(tag == 0xA3)	                           // CATTP 的 tag
	{
		result = CATTPExcuteCommand();
		return;
	}	
	
	G_Source_Type = SourceType_SMS;
		
	fileAddr = SelectTFFile(DF_OTA,LDFile_RelaxFile);  
	
	RemoteAPDUProcessFunction(fileAddr, G_CurrentCommandOffset-SMS_Head_CplToPcntr, 0x00, RemoteDataEndBlock, &isTriHttp); 
	
//	ReadLFBuf(fileAddr, i, 12,(BYTE *)&type,1);	        //读取指令类型
//	
//	if (type == 0x01)		//扩展型执行
//	{
//		ExtendedRFMExcuteCommand();
//	}
//	else		//压缩型执行
//	{
//		CompressCMDProgress();
//	}
	
#ifdef CATTP	
	if(G_PushCommand == TRUE)
	{
		G_PushCommand = FALSE;

		G_TARRecIndex[1] = FALSE;

		GPRAMRFMExtendResponsePublic(TAG_ClearResponseFile);

		CATTPOpenLink();	

		return;
	}
#endif	

	G_TARRecIndex[1] = FALSE;
	
	LDFilePoR(result);		//发送上行

	GPRAMRFMExtendResponsePublic(TAG_ClearResponseFile);  //发送上行之后 接着清除上行
	
	if(G_EarlyResponseTag == 0x11)  //当前响应为early response且存在refresh指令
	{
		cephRefreshSingle(0x04);
	}
	
	G_TARRecIndex[1] = FALSE;		//清除LDFile标记
	
}
/*
*@description:  清除RFM数据counter
*
*@param		 : void
*
*@return	 :void
*
*@history	 :V1.0:create by xuyz@20150427
*/
void LDCleanCounter(void)
{		
	BYTE idata recNum,i;
	UWORD xdata fileAddr;
	
	fileAddr = SelectLFFile(DF_OTA, LDFile_CounterFile);	
	
	if(fileAddr == FALSE)
	{
		CommandFinish(0x00);
		return;
	}

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	for(i=1;i<=recNum;i++)
	{
		SetLFConst(fileAddr,i,0x00,0x00,5);	
	}

	CommandFinish(0x00);
	

	
}


/*
 *
 *
*/
void LDCleanUpdateFile(void)
{		
	//WORD idata fileSize;
	UWORD xdata fileAddr;

	fileAddr = SelectTFFile(DF_OTA, LDFile_UpFile);		//选择上行缓存文件
							 
	//fileSize = GetTFSize(fileAddr);	   	//确保该文件有效数据长度少于256bytes

	SetTFConst(fileAddr,0x00,0x00,0x30);
		
}

/*
*@description:检查当前tar值是不是RFM tar
*
*@function   :IsRFMTar
*
*@param      :BYTE *tar
*
*@return     :TRUE:是 RFM tar/FALSE:不是 RFM TAR
*
×@author
*/
BYTE code RFMTars[] = {    0xB0,0x00,0x02,   //共享文件 压缩型 		//TAR3_SHARE_FILE	0x02
                           0xB0,0x00,0x11,							//TAR3_SIM			0x11
						   0xB0,0x00,0x20,   //CSIM ADF 压缩型		//TAR3_CSIM_ADF		0x20
                           0xB0,0x00,0x21,   //USIM ADF 压缩型		//TAR3_USIM_ADF		0x21
                      //   0xB0,0x00,0x22,   //ISIM ADF 压缩型
						   0xB0,0x01,0x02,   //共享文件 压缩型		//TAR3_SHARE_FILE	0x02
						   0xB0,0x01,0x11, 							//TAR3_SIM			0x11
						   0xB0,0x01,0x20,   //CSIM ADF 压缩型		//TAR3_CSIM_ADF		0x20
                           0xB0,0x01,0x21,   //USIM ADF 压缩型		//TAR3_USIM_ADF		0x21
                     //    0xB0,0x00,0x22,   //ISIM ADF 压缩型，暂不支持ISIM
                           };
BYTE IsRFMTar(BYTE *tar)
{
	BYTE idata i;

	for(i = 0; i< sizeof(RFMTars); i +=3)  
	{
		if((tar[0] == RFMTars[i])&&(tar[1] == RFMTars[i+1])&&(tar[2] == RFMTars[i+2]))
		{
			G_TARRecIndex[1] = tar[2];

			return TRUE;
		}
	}
	
	return FALSE;
}


/*
*@description:获取当前执行文件的路径
*
*@param      :buf
*
*@return
*
*@history
*/
BYTE RFMGetCurrentEFPath(BYTE * currentPath)
{
	WORD xdata fileName[0x02];
	
	memset(currentPath, 0xFF, 0x08);
	
	if(G_CurrentDfName[G_ChannelNum] != DF_MF)  //当前访问的EF的父目录不是MF，继续查找上级目录
	{
		 ReadEeprom((BYTE *)&fileName,G_CurrentDfAddr[G_ChannelNum] + F_ParentName_Offset,2*sizeof(WORD));
		
		 if(fileName[1] != DF_MF)  //三级目录全适用
		 {
		 	currentPath[0] = (fileName[0] >>8)&0xFF;
		 	
		 	currentPath[1] = fileName[0]&0xFF;
		 	
		 	currentPath[2] = (fileName[1] >>8)&0xFF;
		 	
		 	currentPath[3] = fileName[1]&0xFF;
		 	
		 	currentPath[4] = (G_CurrentDfName[G_ChannelNum] >>8)&0xFF;
		 	
		 	currentPath[5] = G_CurrentDfName[G_ChannelNum]&0xFF;
		 	
		 	currentPath[6] = (G_CurrentEfName[G_ChannelNum]>>8)&0xFF;
		 	
		 	currentPath[7] = G_CurrentEfName[G_ChannelNum]&0xFF;
		 	
		 	return 0x06;
		 	
		 }
		 else
		 {
		 	currentPath[0] = (fileName[1] >>8)&0xFF;
		 	
		 	currentPath[1] = fileName[1]&0xFF;
		 	
		 	currentPath[2] = (G_CurrentDfName[G_ChannelNum] >>8)&0xFF;
		 	
		 	currentPath[3] = G_CurrentDfName[G_ChannelNum]&0xFF;
		 	
		 	currentPath[4] = (G_CurrentEfName[G_ChannelNum]>>8)&0xFF;
		 	
		 	currentPath[5] = G_CurrentEfName[G_ChannelNum]&0xFF;
		 	
		 	return 0x04;	 
		 	
		 }		
	}
	else
	{
 	 	currentPath[0] = (G_CurrentDfName[G_ChannelNum] >>8)&0xFF;
	 	
	 	currentPath[1] = G_CurrentDfName[G_ChannelNum]&0xFF;
	 	
	 	currentPath[2] = (G_CurrentEfName[G_ChannelNum]>>8)&0xFF;
	 	
	 	currentPath[3] = G_CurrentEfName[G_ChannelNum]&0xFF;	
	 	
	 	return 0x02;
	}
	
}


/*
*@description: 判断当前正在执行的文件操作是否满足EFFCL文件中所规定的权限
*
*@param      ：efaddr ，action
*
*@return     ：TRUE：允许执行操作/FALSE：权限不满足
*
*@history
*/
BYTE RFMAccessCondition(UWORD fileAddr,BYTE action)
{	
	WORD xdata fileName;
	BYTE idata fileType,i;
	BYTE xdata currentPath[0x08];  //最多支持3级目录，EF FCL文件初始化记录全ff
	UWORD xdata FCLfileAddr;
	BYTE xdata name[0x09];
	
	//G_RFMFileIn2F08 = FALSE;											//执行前,默认该文件没有在2F08中有配置
	
	memset((BYTE *)currentPath, 0xFF, sizeof(currentPath));
	
	i = RFMGetCurrentEFPath(currentPath);
	
	//if(G_CompactExtendedTag == TAR3_SHARE_FILE)  //共享文件系统只能访问MF，MF下的所有DF和EF文件，应用开始运行时，默认当前路径为MF
	if ((G_TARRecIndex[1] == TAR3_SHARE_FILE)		//共享文件
		||(G_TARRecIndex[1] == TAR3_SIM)
	 )
	{
		fileName = ((WORD)currentPath[2]<<8)+currentPath[3];
		
		if((fileName == G_DF_ID_CSIM)||(fileName == G_DF_ID_USIM))  // 当前文件为CSIM / USIM / ISIM
		{
			//G_RFMFileIn2F08 = TRUE;							  	//此情况可认为在3F00/2F08中有配置
			return FALSE;
		}
	}
	
	if(G_CurrentEfName[G_ChannelNum] == FALSE)  //对于删除文件而言，当前拼装的数据经过上述组装可能只是3F000000等结构
	{
		ReadEeprom((BYTE *)&fileName,fileAddr,sizeof(WORD));
		
		currentPath[i] = (fileName>>8)&0xFF;
	 	
	 	currentPath[i+1] = fileName&0xFF;	
	}
	

	if(((action == A_CreateDF)||(action == A_CreateEF))&&(INS == 0xE0))  //创建文件时，不需要在此验证权限
	{
		
		fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

		if( fileType == ADFTYPE )
		{
			//G_RFMFileIn2F08 = TRUE;							  	//此情况可认为在3F00/2F08中有配置
			return TRUE;
		}
	}
	
	FCLfileAddr = SelectLFFile(DF_MF,EF_FCL);
	
	if(FCLfileAddr == FALSE)
	{
		return TRUE;
	}
	
	if(action == A_ReadFile)
	{
		action = Action_ReadBit;
	}
	else if(action == A_UpdateFile)
	{
		action = Action_UpdateBit;
	}
	else if(action == A_DeleteFile)
	{
		action = Action_DeleteBit;		
	}
	else if(action == A_ResizeFile)
	{
		action = Action_ResizeBit;		
	}
	else if(action == A_DeactivateFile)
	{
		action = Action_DeactiveBit;
	}
	else if(action == A_ActivateFile)
	{
		action = Action_ActiveBit;
	}
	
	for(i = 0; i <GetLFRecNumLgth(FCLfileAddr, FileNum) ; i++)
	{
		ReadLFBuf(FCLfileAddr,i+1, 0x00, (BYTE *)name, 0x09);  //读取文件路径ID  
		
		fileName = ((WORD)name[0]<<8)+name[1];
		
		if(fileName != DF_MF)   //卡端获取文件的权限都从MF开始，为防止FCL中某些文件的起始路径不是0x3F00，在此处添加MF一级目录
		{
			memmove(&name[2] ,&name[0],0x06);
			
			name[0] = 0x3F;
			
			name[1] = 0x00;
		}
		
		if(memcmp((BYTE *)name,(BYTE *)currentPath,0x08) == EQUAL)  //在FCL文件中找到当前正在被操作的文件
		{		
			//G_RFMFileIn2F08 = TRUE;							  	//找到该文件在3F00/2F08中有配置
			return ValueBit(name[0x08], action);
		}
	}
	
	//return FALSE;  //在FCL列表中，未找到相关文件,规范0815版本指令FCL列表中的文件为不可被远程操作的文件，对于不在FCL列表中的文件是否允许远程操作，需求未详细说明，暂时不允许RFM操作不在FCL列表中的文件，待需求明确后修改
	return TRUE;	//不在FCL列表中的文件，都默认允许RFM
}

/*
*@funtion    :ProcessRemoteCheckRAPDU 
*
*@description:检查APDU的完整性
*
*@param      :cplLen	--	数据长度
*
*@return     :TRUE or FALSE
*
*@author     :LCZ
*
*@history    :V1.0 LCZ @20181213      create
*/
BYTE ProcessRemoteCheckCAPDU(WORD cplLen)
{
//	if(G_Data_Type == DataType_RFM)
//	{
		if ((P3 != 0x00) && ((INS != 0xB0) && (INS != 0xB2) && (INS != 0xC0)))	//case3,case4
		{
			
			if((cplLen) < (P3+5))
			{
				return FALSE;
			}
		}
//	}
//	else
//	{
//		if( ((INS != 0xC0)&&(INS!=0xCA))||((INS == 0xCA)&&(P1==0xFF)&&(P2==0x1F)))        //GET DATA只有FF1F才接收数据		
//		{	
//			if((cplLen) < (P3+5))	  	//数据长度小于一条指令，退出
//			{
//				return FALSE;
//			}	
//		}
//	}
	
	return TRUE;
}

#endif 
