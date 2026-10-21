/*
*@HISTORY       :V1.0: 创建源文件

*				 V1.1：修改函数processTimeoutInCloseWaitState，在清除链接记录时，需保留通道ID，因为在关闭通道会使用CATTPRecord.channelId @20111121 Version 1.7- >1.8
*/
#include  "typedef.h"
#include  "Uicc_main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include  "error.h"
#include  "arithmatic.h"
#include  "ceph.h"
//#include  "main.h"
#include  "stkalert.h"
#include  "FunctionSwitch.h"
#include "BIPModuleProcess.h"
#include "CatTp.h"
#include "ToolkitRFM.h"
#include "TimerModuleProces.h"
#include   "CompactManage.h"
#include  "ExpandedManage.h"
#include "Interface_LTEFromPlatform.h"
#include "Crypto_Constant.h"
//#include "uicc_m2M.h"

#if UICC__CDMA__SMS__DOWNLOAD
#include "cdma.h"
#endif 
#include "ToolkitAPIPublic.h"

#if M2M_EUICC
#include "uicc_M2M.h"
#endif

#if UICC__FUNCTION__CATTP


ConnectionRecord xdata CATTPRecord;

BYTE xdata G_CATTP_TimerIdentifier;
WORD xdata G_CurrentSDUForSendLength;        //当前要发送的SDU的长度

WORD xdata G_CATTPSndSEQ;                    //当前发送的SEQ
WORD xdata G_CurrentPDULength;  //当前收到的PDU的长度

BYTE xdata G_prePduNoData;   //1，前一个PDU无数据，0前一个PDU有数据
WORD xdata G_LASTSDU_NB;      //sdu 最后一块的SEQ
BYTE xdata G_LASTSDU_Flag;  //判断当前是否收到了最后一条SDU

BYTE xdata G_RetryTimers;  //CATTP链接过程中 每次发送APDU后将，此变量设置为3，如果3次超时，则关闭BIP通道

BYTE xdata G_CATTPOperation;  //该变量为TRUE时，表示进入到CATTP协议操作.初始化或释放连接后，清空该变量

BYTE xdata G_CATTPSndNextSeqNB;
BYTE xdata G_CATTPType;          //用来表示谁调用80EC,发openchannel时候读取参数用,发完openchannel还原
BYTE xdata G_PDUSize[2];
/*
*@description: CATTP变量初始化
*
*@function   : BIPModuleInit
*
*@param       :void
*
*@return      :void
*
*@author      :LCZ
*
*@hisotry     :V1.0 ：create @20190115
*/
void CATTPModuleInit(void)
{	
	clearCatTPVariable();
	
	G_CATTPSndSEQ = 0x00;

	ecpMem_Set((BYTE *)&CATTPRecord, 0x00 ,sizeof(CATTPRecord));
	
	G_prePduNoData = FALSE;
	
	G_CATTPOperation = FALSE ;
	
	G_CATTPSndNextSeqNB = FALSE;

	G_LASTSDU_NB = FALSE;      //sdu 最后一块的SEQ

	G_LASTSDU_Flag = FALSE;  //判断当前是否收到了SDU的最后一块
		
	G_PushCommand = FALSE;
	
//	G_refreshTag = FALSE;
	
	G_RetryTimers = 0;
	
 	G_CurrentPDULength = 0;
	 
	G_CATTP_TimerIdentifier = 0;

}
/*
*@description :SETUP EVENlist 主动式命令
*
*@param       :evenList:事件列表  LV结构
*
*@returns
*
*@history
*/
BYTE cephSetUpEvenListSingle(BYTE *evenList)
{
	BYTE idata result;
	
	InitProactiveCommand(CMD_SETUPEVENT,0x00,DEV_ME);

	RecData[0] = 0x19;
	
	RecData[1] = 0x00;

	if(evenList != NULL)
	{
		AddValue((BYTE xdata *)&RecData[1], &evenList[0]);
	}

	AddTLV((BYTE xdata *)&ResData[1], (BYTE xdata *)&RecData[0]);	
	
	result = SendProactiveCommand();

	return result;
}
/*
*@description :计算当前存储数据的CHECKSUM
*
*@param       :
*
*@return
*
*@history     :
*/
WORD CATTPCalCheckSumAddr(UWORD fileAddr,WORD dataLen,BYTE DataOffset)
{
	BYTE xdata dataIn[0x02];
	UWORD idata i,sum;
	
	sum = 0;
	
	for(i=0;i<dataLen;)
	{
		if(i == (dataLen-1))
		{
//				ReadTFBuf(fileAddr, i+DataOffset, &dataIn[0] , 0x01);//接收BIP数据时候前两字节是Len  发送Por时候则不是 所以添加偏移看需要计算CRC的偏移
				ReadEeprom(&dataIn[0], fileAddr+i+DataOffset, 0x01);
			
				dataIn[1] = 0x00;
		}
		else
		{
//				ReadTFBuf(fileAddr, i+DataOffset, &dataIn[0] , 0x02);//接收BIP数据时候前两字节是Len  发送Por时候则不是 所以添加偏移看需要计算CRC的偏移
				ReadEeprom(&dataIn[0], fileAddr+i+DataOffset, 0x02);
		}
		
		if(i == CATTP_CHECKSUM_OFFSET)
		{
			dataIn[0] = 0x00;
			dataIn[1] = 0x00;
		}
		
		sum += (dataIn[0] << 8) | dataIn[1];

		if(sum > 0xFFFF)
		{
		  sum +=1;
		  
		  sum &= 0xFFFF;
		}

		i+=2;
	  
	}
	
	return (WORD)(~sum);
}
/*
*@description:"Send Request" in "OPEN" state
*
*@param      :void
*
*@returns    :
*
*@history    :
*/
BYTE CATTPSendRequestInOpenState(BYTE *dataBuf)
{
	if(CATTPRecord.SND_NXT_SEQ_NB < (CATTPRecord.SND_UNA_PDU_SEQ_NB + CATTPRecord.SND_WIN_SIZE))   
	{
		//send ACK Data
		CATTPSendACKPDU(dataBuf);
		
		G_CATTPSndNextSeqNB = TRUE;

		/*
		根据102127 5.3.2.1,Each time a PDU requiring an acknowledgment (i.e. ACK with Data, SYN, NUL) is sent, the sequence number is
		incremented by one once this PDU has been sent (that is a post-increment).
		PDUs requiring no acknowledgment (i.e. ACK without data, ACK/EACK without data and RST) do not increment the
		sequence number.
		*/
		if(dataBuf != NULL)
		{		
			CATTPRecord.SND_NXT_SEQ_NB +=1;
		}
	}
	else
	{
		//insufficient resource now
	}
	CATTPRecord.status = CATTP_OPEN;
	
	return TRUE;
}
/*
*@description  :拼装CATTP头的一些共同部分
*
*@param        :
*
*@return
*
*@history      :
*/
void CATTPHeaderPDU(BYTE *dataBuf)
{
	dataBuf[0] = 0x00;
	
	dataBuf[1] = 0x00;
	
	dataBuf[3] = (CATTPRecord.srcPort >>8)&0xFF;
	
	dataBuf[4] = CATTPRecord.srcPort&0xFF;
	
	dataBuf[5] = (CATTPRecord.desPort>>8)&0xFF;
	
	dataBuf[6] = CATTPRecord.desPort&0xFF;
	
	dataBuf[15] = 0x00; //check sum1
	
	dataBuf[16] = 0x00; //check sum2

}

/*
*@description  :拼装ACK PDU，并发送出去
*
*@param        :dataBuf:LV 结构   ;目前仅支持一个字节的ACK数据，
*
*@return
*
*@history      :
*/
BYTE CATTPSendACKPDU(BYTE *dataBuf)
{
	WORD idata checkSum;
	
	BYTE idata result,dataLgth;
	
//	G_CATTPSndNextSeqNB = FALSE;
	
	if(dataBuf == NULL)
	{
		dataLgth = 0x00;
	}
	else
	{
		dataLgth = dataBuf[0];
	}
	tempBuf[0] = 0x12 +dataLgth;   //PDU 长度
	
	tempBuf[1] = CATTP_HEADER_ACK_BIT|CATTP_VERSION;
	
// 	tempBuf[2] = 0x00;  //RFU
// 	
// 	tempBuf[3] = 0x00; //rfu
	
	CATTPHeaderPDU(&tempBuf[2]);
	
	tempBuf[4] = 0x12; //header length
	
// 	tempBuf[5] = (CATTPRecord.srcPort >>8)&0xFF;
// 	
// 	tempBuf[6] = CATTPRecord.srcPort&0xFF;
// 	
// 	tempBuf[7] = (CATTPRecord.desPort>>8)&0xFF;
// 	
// 	tempBuf[8] = CATTPRecord.desPort&0xFF;
	
	tempBuf[9] = 0x00;
	
	tempBuf[10] = dataLgth;
	
	tempBuf[11] = (CATTPRecord.SND_NXT_SEQ_NB >>8)&0xFF;
	
	tempBuf[12] = CATTPRecord.SND_NXT_SEQ_NB&0xFF;
	
	tempBuf[13] = (CATTPRecord.RCV_CUR_SEQ_NB >>8)&0xFF;
	
	tempBuf[14] = CATTPRecord.RCV_CUR_SEQ_NB&0xFF;
	
	tempBuf[15] = (CATTPRecord.RCV_WIN_SIZE>>8)&0xFF;
	
	tempBuf[16] = CATTPRecord.RCV_WIN_SIZE&0xFF;
	
// 	tempBuf[17] = 0x00; //check sum1
// 	
// 	tempBuf[18] = 0x00; //check sum2
	
	ecpMem_Copy(&tempBuf[19], &dataBuf[1],dataLgth);  //拷贝数据
	
	checkSum = CATTPCalCheckSum(&tempBuf[1],tempBuf[0]);
	
	tempBuf[17] = (checkSum>>8)&0xFF;
	
	tempBuf[18] = checkSum&0xFF;

	if((CATTPRecord.status == CATTP_SYN_SENT)||((CATTPRecord.status == CATTP_OPEN)&&(dataBuf != NULL)))	    //cattp链接未建立或者CATTP链接建立后，但是发给服务器的POR服务器端未处理
	{

		WriteVariable(VaribleTag_BackPDU,&tempBuf[0]);
	
		G_RetryTimers = 0x03;
	}
		
	result = cephSendDataSingle(0x01,CATTPRecord.channelId,&tempBuf[0]);

	return result;
}
/*
*@description:发送SYN PDU
*
*@function:	 CATTPSendSYNPDU
*
*@param   :	identification
*/
BYTE CATTPSendSYNPDU(BYTE *identification)
{
	WORD idata checksum;
	BYTE idata result;
	BYTE idata winsize;
	
	//拼装SYN-PDU
	
	tempBuf[1] = CATTP_HEADER_SYN_BIT|CATTP_VERSION; //SYN  bit
	
// 	tempBuf[2] = 0x00; //rfu1
// 	
// 	tempBuf[3] = 0x00; //rfu2
	
	CATTPHeaderPDU(&tempBuf[2]);
	
	tempBuf[4] = 22 + identification[0]+1;  //headerLength,
	
// 	tempBuf[5] = ( CATTPRecord.srcPort >>8)&0xFF;
// 	
// 	tempBuf[6] =  CATTPRecord.srcPort&0xFF;  //source port
// 		
// 	tempBuf[7] = (CATTPRecord.desPort >>8)&0xFF;
// 	
// 	tempBuf[8] = CATTPRecord.desPort&0xFF;  //des port
	
	tempBuf[9] = 0x00;
	
	tempBuf[10] = 0x00;  //datalength = 0
	
	tempBuf[11] =  (CATTPRecord.SND_INI_SEQ_NB >>8)&0xFF;
	
	tempBuf[12] =  CATTPRecord.SND_INI_SEQ_NB&0xFF;  //SEQ
	
	tempBuf[13] = 0x00;
	
	tempBuf[14] = 0x00;  //ACK non signification 
	
	tempBuf[15] = 0x00;
	
	winsize = (CATTPRecord.RCV_SDU_SIZE_MAX)/(CATTPRecord.RCV_PDU_SIZE_MAX) + (((CATTPRecord.RCV_SDU_SIZE_MAX)%(CATTPRecord.RCV_PDU_SIZE_MAX))?1:0);
	
	if(winsize <DEFAULT_WIN_SIZE)
	{
		winsize = DEFAULT_WIN_SIZE;
	}
	
	//tempBuf[16] = DEFAULT_WIN_SIZE;  //WIN SIZE non signification 
	
	tempBuf[16] = winsize;
	
// 	tempBuf[17] = 0x00;
// 	
// 	tempBuf[18] = 0x00;	//While computing the checksum, the checksum field itself shall be replaced with zeros.
	
	tempBuf[19] = (CATTPRecord.RCV_PDU_SIZE_MAX>>8)&0xFF;
	
	tempBuf[20] = CATTPRecord.RCV_PDU_SIZE_MAX&0xFF ;    //PDU SIZE
	
	tempBuf[21] = (CATTPRecord.RCV_SDU_SIZE_MAX>>8)&0xFF;
	
	tempBuf[22] = CATTPRecord.RCV_SDU_SIZE_MAX&0xFF ;    //SDU SIZE
	
	ecpMem_Copy(&tempBuf[23], identification,identification[0]+1); //identification
	
	checksum = CATTPCalCheckSum(&tempBuf[1],tempBuf[4] );
	
	tempBuf[17] = (checksum >>8)&0xFF;
	
	tempBuf[18] = checksum&0xFF;
	
	tempBuf[0] = tempBuf[4];  //SYN PDU length

	WriteVariable(VaribleTag_BackPDU,&tempBuf[0]);

	G_RetryTimers = 0x03;
	
	result =  cephSendDataSingle(0x01, CATTPRecord.channelId, &tempBuf[0]);
	
	return result;
}
/*
*@desription:拼装EACK发送
*
*@param
*
*@return
*
*@history
*/
BYTE CATTPSendEACKPDU(void)
{
	WORD idata checkSum;
	BYTE idata num,result;
	UWORD idata fileAddr;
	
	fileAddr = SelectPrivateTFFile(DF_CATTP,EF_EACKSEQ); //zhhj
	
	ReadTFBuf(fileAddr, 0x00, &num, sizeof(BYTE));
	

	tempBuf[1] = CATTP_HEADER_ACK_BIT|CATTP_HEADER_EACK_BIT|CATTP_VERSION;
	
// 	tempBuf[2] = 0x00;
// 	
// 	tempBuf[3] = 0x00; //RFU
	
	CATTPHeaderPDU(&tempBuf[2]);
	
	tempBuf[4] = 0x12 + num*2;
	
// 	tempBuf[5] = (CATTPRecord.srcPort >>8)&0xFF;
// 	
// 	tempBuf[6] = CATTPRecord.srcPort&0xFF;
// 	
// 	tempBuf[7] = (CATTPRecord.desPort>>8)&0xFF;
// 	
// 	tempBuf[8] = CATTPRecord.desPort&0xFF;
	
	tempBuf[9] = 0x00;
	
	tempBuf[10] = 0x00;
	
	tempBuf[11] = (CATTPRecord.SND_NXT_SEQ_NB >>8)&0xFF;
	
	tempBuf[12] = CATTPRecord.SND_NXT_SEQ_NB&0xFF;
	
	tempBuf[13] = (CATTPRecord.RCV_CUR_SEQ_NB >>8)&0xFF;
	
	tempBuf[14] = CATTPRecord.RCV_CUR_SEQ_NB&0xFF;
	
	tempBuf[15] = (CATTPRecord.RCV_WIN_SIZE>>8)&0xFF;
	
	tempBuf[16] = CATTPRecord.RCV_WIN_SIZE&0xFF;
	
// 	tempBuf[17] = 0x00; //check sum1
// 	
// 	tempBuf[18] = 0x00; //check sum2
	
	ReadTFBuf(fileAddr, 0x01,&tempBuf[19] ,num*2);
	
	checkSum = CATTPCalCheckSum(&tempBuf[1],tempBuf[4]);
	
	tempBuf[17] = (checkSum>>8)&0xFF;
	
	tempBuf[18] = checkSum&0xFF;
	
	tempBuf[0] = 18+num*2;
	
	result = cephSendDataSingle(0x01,CATTPRecord.channelId,&tempBuf[0]);
	
	return result;	
}
/*
*@description :拼装RST  PDU
*
*@param
*
*@return
*
*@history
* V1.1 CATTPSendRSTPDU和CATTPSendRST_ACKPDU函数合并,seq为0x00的时候是CATTPSendRST_ACKPDU,seq目前最小值为0x01
*/
BYTE CATTPSendRSTPDU(BYTE reason,WORD seq)
{
	BYTE idata result;
	WORD idata checkSum;
	BYTE idata winsize;
	
	if(seq !=0x00)
	{
		tempBuf[1] = CATTP_HEADER_RST_BIT|CATTP_VERSION;
		
		tempBuf[11] = (seq >> 8)&0xFF;
		
		tempBuf[12] = seq&0xFF;  //seq
		
		tempBuf[13] = 0x00;
		
		tempBuf[14] = 0x00; //ACK
		
		winsize = (CATTPRecord.RCV_SDU_SIZE_MAX)/(CATTPRecord.RCV_PDU_SIZE_MAX) + (((CATTPRecord.RCV_SDU_SIZE_MAX)%(CATTPRecord.RCV_PDU_SIZE_MAX))?1:0);
		
		if(winsize<DEFAULT_WIN_SIZE)
		{
			winsize = DEFAULT_WIN_SIZE;
		}
		
		//tempBuf[16] = DEFAULT_WIN_SIZE;  //WIN SIZE
		
		tempBuf[16] = winsize;
	}
	else
	{
		tempBuf[1] = CATTP_HEADER_RST_BIT|CATTP_HEADER_ACK_BIT|CATTP_VERSION;
		
		tempBuf[11] =0x00;
	
		tempBuf[12] = 0x00;  //seq
		
		tempBuf[13] = (CATTPRecord.CUR_PDU_SEQ_NB >>8)&0xFF;
		
		tempBuf[14] =  CATTPRecord.CUR_PDU_SEQ_NB &0xFF;; //ACK
		
		tempBuf[16] = 0x00; //win size
    }
// 	tempBuf[2] = 0x00; //RFU1
// 	
// 	tempBuf[3] = 0x00;  //RFU2
	
	CATTPHeaderPDU(&tempBuf[2]);
	
	tempBuf[4] = 0x13;  //header lgth
	
// 	tempBuf[5] = (CATTPRecord.srcPort >>8)&0xFF;
// 	
// 	tempBuf[6] = CATTPRecord.srcPort&0xFF;
// 	
// 	tempBuf[7] = (CATTPRecord.desPort>>8)&0xFF;
// 	
// 	tempBuf[8] = CATTPRecord.desPort&0xFF;
	
	tempBuf[9] = 0x00;
	
	tempBuf[10] = 0x00;
	
	tempBuf[15] = 0x00;
	
// 	tempBuf[17] = 0x00;
// 	
// 	tempBuf[18] = 0x00;
	
	tempBuf[19] = reason;
	
	checkSum = CATTPCalCheckSum(&tempBuf[1],tempBuf[4]);
	
	tempBuf[17] = (checkSum>>8)&0xFF;
	
	tempBuf[18] = checkSum&0xFF;
	
	tempBuf[0] = 19;
	
	result = cephSendDataSingle(0x01,CATTPRecord.channelId,&tempBuf[0]);
	
	return result;	
	
}
/*
*@description :判断接收到的pdu是否有效
*
*@param       :
*
*@returns     :TRUE:pdu 有效/FALSE :pdu 无效
*
*@history     :V1.1,modify bu xwh 20181010,修改源端口和目的端口比较的bug(&&修改成||)
*/
BYTE CATTPPduValid(void)
{
	BYTE idata headerLgth;
	UWORD  xdata bipAddr;
	 
	//bipAddr = SelectPrivateTFFile(DF_Toolkit, EF_BIPData);  //缓存BIP数据 zhhj
	
//	bipAddr = SelectPrivateTFFile(DF_BIPModule, EF_BIPModule_Receive);  //缓存BIP数据 zhhj
	bipAddr = BIPModule_Receive_ADDR_START;
	
//	ReadTFBuf(bipAddr,CATTP_HeaderLgth_OFFSET + CATTP_Len, &headerLgth,sizeof(BYTE));
	ReadEeprom(&headerLgth, bipAddr + CATTP_HeaderLgth_OFFSET + CATTP_Len, sizeof(BYTE));
	
//	ReadTFBuf(bipAddr,CATTP_TagField_OFFSET + CATTP_Len,&tempBuf[0],headerLgth);
	ReadEeprom(tempBuf, bipAddr + CATTP_TagField_OFFSET + CATTP_Len, headerLgth);
	
	CATTPRecord.CUR_PDU_FLAG = tempBuf[CATTP_TagField_OFFSET];
	
	CATTPRecord.CUR_PDU_VRS_NB = CATTPRecord.CUR_PDU_FLAG &0x03;
	
	if((CATTPRecord.CUR_PDU_FLAG&0x80) == CATTP_HEADER_SYN_BIT)   //SYN PDU
	{
		CATTPRecord.SYN_PDU_SIZE_MAX = (((WORD)tempBuf[CATTP_PDUMAX_OFFSET]) <<8)&0xFF00 +tempBuf[CATTP_PDUMAX_OFFSET+1];
		
		CATTPRecord.SYN_SDU_SIZE_MAX = (((WORD)tempBuf[CATTP_SDUMAX_OFFSET]) <<8)&0xFF00 +tempBuf[CATTP_SDUMAX_OFFSET+1];
		
	}
	
	if((tempBuf[CATTP_SrcPort_OFFSET]   != ((CATTPRecord.desPort>>8)&0xFF)) ||( tempBuf[CATTP_SrcPort_OFFSET+1] !=(CATTPRecord.desPort&0xFF)))  //接受到的PDU的原端口 应该等于发送出去的PDU的目的端口
	{
		return FALSE;
	}
	
	if((tempBuf[CATTP_DesPort_OFFSET]   != ((CATTPRecord.srcPort>>8)&0xFF)) ||( tempBuf[CATTP_DesPort_OFFSET+1] !=(CATTPRecord.srcPort&0xFF)))  //接受到的PDU的目的端口 应该等于发送出去的PDU的源端口
	{
		return FALSE;
	}
	
	CATTPRecord.CUR_PDU_SEQ_NB = (((WORD)tempBuf[CATTP_SEQNumber_OFFSET]) <<8)&0xFF00 |tempBuf[CATTP_SEQNumber_OFFSET+1];
	
	CATTPRecord.CUR_PDU_ACK_NB =  (((WORD)tempBuf[CATTP_ACKNumber_OFFSET]) <<8)&0xFF00 |tempBuf[CATTP_ACKNumber_OFFSET+1];
	
	CATTPRecord.CUR_PDU_WIN_SIZE =  (((WORD)tempBuf[CATTP_WINSIZE_OFFSET]) <<8)&0xFF00 |tempBuf[CATTP_WINSIZE_OFFSET+1];
	
	CATTPRecord.CUR_PDU_CHECKSUM =(((WORD)tempBuf[CATTP_CHECKSUM_OFFSET]) <<8)&0xFF00 |tempBuf[CATTP_CHECKSUM_OFFSET+1];
	
	CATTPRecord.CUR_PDU_HEADER_LENGTH = headerLgth;
	
	CATTPRecord.CUR_PDU_DATA_LENGTH = ((WORD)tempBuf[CATTP_DataLgth_OFFSET]<<8)&0xFF00 | tempBuf[CATTP_DataLgth_OFFSET +1];
	
	if(CATTPRecord.CUR_PDU_CHECKSUM  != CATTPCalCheckSumAddr(bipAddr,CATTPRecord.CUR_PDU_HEADER_LENGTH+CATTPRecord.CUR_PDU_DATA_LENGTH,CATTP_Len))
	{
		return FALSE;
	}
	
	if(G_CurrentPDULength != (CATTPRecord.CUR_PDU_HEADER_LENGTH+CATTPRecord.CUR_PDU_DATA_LENGTH))//PDU length as received from the lower layer does not match sum of header and data length;
	{
		return FALSE;
	}
	
	if(	G_CurrentPDULength >CATTPRecord.RCV_PDU_SIZE_MAX) //Size of PDU exceeds maximum acceptable PDU size (RCV_PDU_SIZE_MAX).
	{
		return FALSE;
	}
	
	return TRUE;
}
/*
*@description :在 SYN SENT状态下接受到PDU的处理流程
*
*@param       :
*
*@return	TRUE:不要再重新打开timer ，兼容脚本，没有实际意义/false：需要重新打开timer/	TimerClose ：需要关闭timer
*
*@history
*/
BYTE CATTPProcessPduArrivalInSynSent(void)
{
	BYTE idata winsize;     //为了防止 SDU 和PDU大小一样的情况 winsize为1  而测试时候下来的级联CUR_PDU_FLAG
	
	if(CATTPPduValid() == FALSE)
	{
		return TRUE;   
	}
	
	if((CATTPRecord.CUR_PDU_FLAG&0x10) == CATTP_HEADER_RST_BIT)  //RST SET
	{
		if((CATTPRecord.CUR_PDU_FLAG&0x40) == CATTP_HEADER_ACK_BIT)   //ACK set
		{
			CATTPRecord.status = CATTP_CLOSE;
			return TimerClose;
		}
		else
		{
			return TRUE;    
		}
	}
	else
	{
		CATTPCloseTimer();	   //数据收全，先关timer

		if((CATTPRecord.CUR_PDU_FLAG&0x80) == CATTP_HEADER_SYN_BIT)   //SYN set
		{
			if((CATTPRecord.CUR_PDU_FLAG&0x40) == CATTP_HEADER_ACK_BIT)   //ACK set
			{
				if(CATTPRecord.CUR_PDU_ACK_NB == CATTPRecord.SND_INI_SEQ_NB)
				{
					if(CATTPRecord.CUR_PDU_VRS_NB == CATTP_VERSION)  //Version right
					{
						if(CATTPRecord.SYN_PDU_SIZE_MAX >= MIN_PDU_LENGTH)
						{
							CATTPRecord.RCV_CUR_SEQ_NB = CATTPRecord.CUR_PDU_SEQ_NB;
							
							CATTPRecord.RCV_INI_SEQ_NB =  CATTPRecord.CUR_PDU_SEQ_NB;
							
							CATTPRecord.SND_WIN_SIZE = CATTPRecord.CUR_PDU_WIN_SIZE;
							
							CATTPRecord.SND_PDU_SIZE_MAX = CATTPRecord.SYN_PDU_SIZE_MAX;
							
							CATTPRecord.SND_SDU_SIZE_MAX = CATTPRecord.SYN_SDU_SIZE_MAX;
							
							CATTPRecord.SND_UNA_PDU_SEQ_NB = CATTPRecord.CUR_PDU_ACK_NB +1;
							
							winsize = (CATTPRecord.RCV_SDU_SIZE_MAX)/(CATTPRecord.RCV_PDU_SIZE_MAX) + (((CATTPRecord.RCV_SDU_SIZE_MAX)%(CATTPRecord.RCV_PDU_SIZE_MAX))?1:0);
							
							if(winsize < DEFAULT_WIN_SIZE)
							{
								winsize = DEFAULT_WIN_SIZE;
							}
							
							//CATTPRecord.RCV_WIN_SIZE = DEFAULT_WIN_SIZE;    //标准应由计算得到  允许接收到的PDU数目
							
							CATTPRecord.RCV_WIN_SIZE = winsize;
												

							//send ACK
							CATTPSendACKPDU(NULL);
							
							CATTPRecord.status = CATTP_OPEN; 
																		
							return FALSE;
							
						}
						else
						{
							CATTPSendRSTPDU(0x01, CATTPRecord.CUR_PDU_ACK_NB+1);
							
							CATTPRecord.status = CATTP_CLOSE;  //discard PDU //Send<RST> <SEQ=CUR_PDU_ACK_NB+1> <Reason Code=’01'>
							
							return FALSE; 
						}
						
					}
					else
					{
				
						CATTPSendRSTPDU(0x06, CATTPRecord.SND_NXT_SEQ_NB);
						
						CATTPRecord.status = CATTP_CLOSE;  //discard PDU  send RST  <SEQ= SND_NXT_SEQ_NB> reason = 06
						
						return FALSE;
					}
	
				}
				else
				{	
					CATTPRecord.status = CATTP_SYN_SENT;  //discard PDU
					return FALSE;
				}
			}
			else  //NO ack set
			{	
				//CATTPSendRST_ACKPDU(0x04);
				
				CATTPSendRSTPDU(0x04,0x00);
				
				CATTPRecord.status = CATTP_CLOSE;  //<Send RST ACK;reason = 04> discard PDU
				
				return FALSE;
			}
		}
		else	//SYN NOT SET
		{
				if((CATTPRecord.CUR_PDU_FLAG&0x40) == CATTP_HEADER_ACK_BIT)   //ACK set
				{
					if(CATTPRecord.CUR_PDU_ACK_NB == CATTPRecord.SND_INI_SEQ_NB)
					{
						CATTPSendRSTPDU(0x04,  CATTPRecord.CUR_PDU_ACK_NB + 1);
						
						CATTPRecord.status = CATTP_CLOSE;  //discard PDU  send RST  <SEQ= SND_NXT_SEQ_NB> reason = 06

						return FALSE;//	Send <RST>   <SEQ= CUR_PDU_ACK_NB + 1>  <reason=‘04’>  Deallocate connection record   Discard PDU
					}
					else
					{
						CATTPRecord.status = CATTP_SYN_SENT;  //discard PDU
						return FALSE;
					}
				}
				else
				{
					CATTPRecord.status = CATTP_SYN_SENT;  //discard PDU
					return FALSE;
				}
		}
	}
}
/*
*@description :将当前SEQ更新到OUT OF SEQUENCE列表中
*
*@param       
*
*@returns     
*
*@history   V1.1:modify by xwh 20181010，修改写入currentSeq的处理逻辑
*/
void CATTPUpdateListOutOfSequence(WORD currentSeq)
{
	WORD idata seq;
	BYTE idata num,i;
	UWORD idata fileAddr;
	
	fileAddr = SelectPrivateTFFile(DF_CATTP,EF_EACKSEQ);
	
	if(fileAddr == FALSE)
	{
		return ;
	}
	
	ReadTFBuf(fileAddr, 0x00, &num, sizeof(BYTE));
	
	for(i=1 ;i<=num; i++)
	{
		ReadTFBuf(fileAddr,i*2-1,(BYTE *)&seq,sizeof(WORD));
		
		if(currentSeq <= seq)
		{
			//WriteTFFileNonAtomic(fileAddr,2*(i+1)-1,fileAddr+sizeof(TranFileHead)+2*i-1,(num-i+1)*2);
			//从前往后拷贝，数据有可能重叠，不能调用WriteTFFileNonAtomic ，必须调用 VMCopyDataToAddr,by xwh 20201104
			VMCopyDataToAddr(fileAddr+sizeof(TranFileHead)+2*i+1,GetTFSize(fileAddr)-(2*i+1),fileAddr+sizeof(TranFileHead)+2*i-1,(num-i+1)*2,MODE_PROTECT_WRITE);
			
			WriteTFBuf(fileAddr,i*2-1, (BYTE *)&currentSeq,sizeof(WORD));
			
			num ++;
			
			SetTFConst(fileAddr, 0x00, num, sizeof(BYTE));
			
			return;
		}
	}
	
	WriteTFBuf(fileAddr, i*2-1, (BYTE*)&currentSeq,sizeof(WORD)); //语句执行到这里一定是 i = (num + 1),,将数据写到最后一个位置
	
	num ++;
			
	SetTFConst(fileAddr, 0x00, num, sizeof(BYTE));
}
/*
*@description:判断out of list sequence list empty
*
*@param
*
*@return
*/
BYTE CATTPOutOfListSequenceIsEmpty(void)
{
	BYTE idata num;
	UWORD xdata fileAddr;
	
	fileAddr = SelectPrivateTFFile(DF_CATTP,EF_EACKSEQ);
	
	if(fileAddr == FALSE)
	{
		return  FALSE;
	}
	
	ReadTFBuf(fileAddr, 0x00, &num, sizeof(BYTE));
	
// 	if(num == 0x00)
// 	{
// 		return FALSE;
// 	}
// 	else
// 	{
		return num;
// 	}
	
}
void CATTPClearFile(WORD fileID)
{
	
	UWORD xdata pduAddr;
	BYTE xdata tmp[0x04];
	
	pduAddr = SelectPrivateTFFile(DF_CATTP,fileID); //zhhj
	
	if(pduAddr == FALSE)
	{
		return  ;
	}

	ReadTFBuf(pduAddr, 0x00, tmp,0x04);

	if((tmp[0] == 0x00)&&(tmp[1] == 0x00)&&(tmp[2] == 0x00)&&(tmp[3] == 0x00))
	{
		return;
	}
	
	SetTFConst(pduAddr, 0x00, 0x00,0x04);
	
	
}
 /*
 *@description :判断当前pdu文件是否有数据需要处理
 *
 *@param       :void
 *
 *@return      :TRUE/FALSE
 *
 *@history
 */
BYTE CATTPPduBufferIsEmpty(void)
{
	WORD idata usedSpace;
	UWORD xdata pduAddr;
	
	pduAddr = SelectPrivateTFFile(DF_CATTP,EF_PDUFILE); //zhhj
	
	if(pduAddr == FALSE)
	{
		return  FALSE;
	}
	
	ReadTFBuf(pduAddr, 0x00, (BYTE *)&usedSpace,sizeof(WORD));
	
	
	if(usedSpace < 0x03)   //判断当前是否有数据需要处理
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
/*
*@description:将当前收到的PDU数据判断合法性之后，存在文件EF_PDUFILE中,从CPL开始存储
*
*@param 
*
*@returns
*
*@history
*/
void CATTPCopyDataToBuffers(void)
{
	WORD idata dataOffset,dataLgth,offset,usedSpace;
	UWORD  xdata bipAddr,pdufileAddr;
	
	pdufileAddr = SelectPrivateTFFile(DF_CATTP,EF_PDUFILE); //zhhj
	
	if(pdufileAddr == FALSE)
	{
		return  ;
	}
	
	ReadTFBuf(pdufileAddr, 0x00, (BYTE *)&usedSpace,sizeof(WORD));
		
	dataOffset = CATTPRecord.CUR_PDU_HEADER_LENGTH+CATTP_Len;  //获取PDU 数据起始偏移  CATTP_Len为 L
	
	dataLgth = CATTPRecord.CUR_PDU_DATA_LENGTH;
	
	offset = usedSpace+2;
	
	WriteTFBuf(pdufileAddr,offset ,(BYTE *)&CATTPRecord.CUR_PDU_SEQ_NB,sizeof(WORD));  //更新当前SEQ
	
	WriteTFBuf(pdufileAddr,offset+2,(BYTE *)&dataLgth,sizeof(WORD));
	
	offset +=4;
	
//	bipAddr = SelectPrivateTFFile(DF_BIPModule,EF_BIPModule_Receive);  //缓存BIP的数据 zhhj
	bipAddr = BIPModule_Receive_ADDR_START;	
//	while(dataLgth >0)
//	{
//		if ( dataLgth > 0xFF )
//		{
//			actual = 0xFF;
//		}
//		else
//		{
//			actual = dataLgth;
//		}
//
//		ReadTFBuf(bipAddr,dataOffset, &tempBuf[0], actual);
//
//		WriteTFBuf(pdufileAddr,offset,&tempBuf[0] ,actual);
//
//		dataOffset  += actual;
//
//		offset += actual;
//
//		dataLgth -= actual;
//	}

	//数据不重叠，可以调用WriteTFFileNonAtomic
	//WriteTFFileNonAtomic(pdufileAddr,offset,bipAddr+sizeof(TranFileHead)+dataOffset,dataLgth);
	WriteTFFileNonAtomic(pdufileAddr,offset,bipAddr+dataOffset,dataLgth);
	
	usedSpace += CATTPRecord.CUR_PDU_DATA_LENGTH +4;
	
	WriteTFBuf(pdufileAddr ,0x00, (BYTE *)&usedSpace, sizeof(WORD));
}
/*
*@description :when received PDU in sequence
*
*@param
*
*@returns
*
*@history
*/
WORD CATTPGetMaxSEQ(void)
{
	WORD idata currentSEQ,max,seq;
	BYTE idata num,i,offset;
	UWORD idata fileAddr;
	
	
	currentSEQ = CATTPRecord.CUR_PDU_SEQ_NB;
	
	max = 0x00;
	
	fileAddr = SelectPrivateTFFile(DF_CATTP,EF_EACKSEQ);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	ReadTFBuf(fileAddr, 0x00, &num, sizeof(BYTE));
	
	offset = 0x01;  //距离当前CUR_PDU_SEQ_NB的偏移，只有连续偏移的数据才在in sequence
	
	while(1)
	{
		for(i = 0 ;i< num; i++)
		{
			ReadTFBuf(fileAddr, i*2+1 ,(BYTE *)&seq,sizeof(WORD));
			
			if((seq -currentSEQ) == offset)
			{
				max = seq;
				
				offset ++;
			}
		}
		
		if(i == num)
		{
			break;
		}
	}
	
	if(max >currentSEQ )
	{
		return max;
	}
	else
	{
		return currentSEQ;	
	}
}
/*
*@description : remove from the array -out- of sequence those have a sequence number <=RCV_CUR_SEQ_NB
*
*@param
*
*@returns
*
*@history
*/
void CATTPRemoveSequenceNumber(void)
{
	WORD idata curSeq,curNum;
	BYTE idata num,i;
	UWORD idata fileAddr;
	
	fileAddr = SelectPrivateTFFile(DF_CATTP,EF_EACKSEQ);
	
	ReadTFBuf(fileAddr, 0x00, &num, sizeof(BYTE));
	
	curNum = num;
	
	ReadTFBuf(fileAddr, 0x01, &tempBuf[1], num*2);         //读取数据到tempBuf里面，整理好后一次写回 减少写文件次数
	
	for(i =1 ;i <=curNum;)
	{
		curSeq = BuildWord(&tempBuf[i*2-1]);
		
		if(curSeq <= CATTPRecord.RCV_CUR_SEQ_NB)
		{
			//MoveTFContent(fileAddr,(i+1)*2-1 ,i*2-1,(curNum-i)*2);
			
			ecpMem_Move(&tempBuf[(i+1)*2-1],&tempBuf[i*2-1],(curNum-i)*2);
			
			num --;
			curNum --;
		}
		else
		{
			i++;
		}
	}
	
	tempBuf[0] = num;
	
	WriteTFBuf(fileAddr, 0x00, &tempBuf[0], num*2+1);
}
/*
*@description :处理其他状态下的延时
*
*@param
*
*@return
*
*@hisotry
*/
void processTimeoutInOtherState(void)
{
// 	if(0)    	
// 	{ 
// 		//Send <RST>
// 		//<SEQ=SND_NXT_SEQ_NB>
// 		//<reason=‘05’>
// 		//Deallocate connection record
// 		CATTPSendRSTPDU(0x05,CATTPRecord.SND_NXT_SEQ_NB);  //retransmission counter > maximum of retries

// 		//signal "connection reset"
// 		//To Upper Layer

// 		CATTPRecord.status = CATTP_CLOSE_WAIT;
// 		
// 		Timeout_Events_Process();
// 	}
// 	else
// 	{

// 	}
	return;  //没用 白盒测试 先屏蔽
}
/*
*@description：清空当前Connection Record
*
*@param      : void
*
*@returns:   :void
*
*@hisotry
*/
void clearCatTPRecord(void)
{	
	ecpMem_Set((BYTE *)&CATTPRecord, 0x00,sizeof(CATTPRecord));
	
	G_CATTPOperation = FALSE;	
}

void clearCatTPVariable(void)
{
	ClearVariable(VaribleTag_BackPDU);

//	ClearVariable(VaribleTag_BackPDUData2);

//	ClearVariable(VaribleTag_BackPDUData3);
}
/*
*@description :处理在CLOSE WAIT状态下的延时
*
*@param
*
*@return
*
*@history
*/
void processTimeoutInCloseWaitState(void)
{
	BYTE idata channelId;
	
	CATTPRecord.status = CATTP_CLOSE;
	
	channelId = CATTPRecord.channelId;   //此处备份channelId，后续关闭通道时，还会使用到CATTPRecord.channelId

	clearCatTPRecord();
	
	CATTPRecord.channelId = channelId;
}
/*
*@description :超时事件处理
*/
void Timeout_Events_Process(void)
{
	switch(CATTPRecord.status)
	{
	case CATTP_CLOSE_WAIT:
		processTimeoutInCloseWaitState();
		break;
	case CATTP_OPEN:
	case CATTP_SYN_SENT:
	case CATTP_SYN_RCVD:
		processTimeoutInOtherState();
		break;
	default : 			  //此处为了白盒测试添加
		break;
	}
}
/*
*@description :在 open 状态下处理接受到的PDU 事件 :figure 25      5.4.2.2
*
*@param       :
*
*@returns   
*
*@history
*/
BYTE CATTPProcessPduArrivalInOpen(void)
{
	BYTE idata winsize;


	if(CATTPPduValid() == FALSE)  //PDU invaid ---> discard PDU
	{
		return FALSE;
	}

	if(((CATTPRecord.RCV_CUR_SEQ_NB < CATTPRecord.CUR_PDU_SEQ_NB)||((G_prePduNoData == 0x01)&&(CATTPRecord.RCV_CUR_SEQ_NB == CATTPRecord.CUR_PDU_SEQ_NB)))&&(CATTPRecord.CUR_PDU_SEQ_NB <= (CATTPRecord.RCV_CUR_SEQ_NB+CATTPRecord.RCV_WIN_SIZE+1)))  //对于只有无 segment的PDU，RCV_CUR_SEQ_NB<CUR_PDU_SEQ_NB,
	{
		if((CATTPRecord.CUR_PDU_FLAG&CATTP_HEADER_RST_BIT) == CATTP_HEADER_RST_BIT) //RST SET,start timer()--->CLOSE WAIT
		{
			//start timer ,connection reset;
			
			CATTPRecord.status = CATTP_CLOSE_WAIT;
			
			Timeout_Events_Process();
			
			return TimerClose;
		}
		else
		{
			CATTPCloseTimer();	   //数据收全，先关timer

			if((CATTPRecord.CUR_PDU_FLAG&CATTP_HEADER_SYN_BIT) == CATTP_HEADER_SYN_BIT)  //SYN SET
			{
				if((CATTPRecord.CUR_PDU_FLAG&CATTP_HEADER_ACK_BIT) == CATTP_HEADER_ACK_BIT)   //ACK set
				{
					CATTPSendRSTPDU(0x04,CATTPRecord.CUR_PDU_ACK_NB +1);  //send RST,reason =04,SEQ = CUR_PDU_ACK_NB+1
				}
				else
				{
					//CATTPSendRST_ACKPDU(0x04);   //send RST ACK,,SEQ = no significant reason =04 ACK = CUR_PDU_SEQ_NB WIN = 0
					
					CATTPSendRSTPDU(0x04,0x00);
				}
						
				CATTPRecord.status = CATTP_CLOSE;  //discard PDU ,coonection reset;
				
				return FALSE;
			}
			else  //SYN NOT SET
			{
				if((CATTPRecord.CUR_PDU_FLAG&CATTP_HEADER_ACK_BIT) == CATTP_HEADER_ACK_BIT)   //ACK set
				{
					if((CATTPRecord.SND_UNA_PDU_SEQ_NB-1) == CATTPRecord.CUR_PDU_ACK_NB)
					{
						if(CATTPRecord.SND_WIN_SIZE < CATTPRecord.CUR_PDU_WIN_SIZE)
						{
							CATTPRecord.SND_WIN_SIZE = CATTPRecord.CUR_PDU_WIN_SIZE;
						}
					}
					else
					{
						if((CATTPRecord.SND_UNA_PDU_SEQ_NB <= CATTPRecord.CUR_PDU_ACK_NB)&&(CATTPRecord.CUR_PDU_ACK_NB < CATTPRecord.SND_NXT_SEQ_NB))
						{
							if( (CATTPRecord.SND_UNA_PDU_SEQ_NB -1+CATTPRecord.SND_WIN_SIZE) <(CATTPRecord.CUR_PDU_ACK_NB+CATTPRecord.CUR_PDU_WIN_SIZE))
							{
								CATTPRecord.SND_WIN_SIZE = CATTPRecord.CUR_PDU_WIN_SIZE;
							}
							else
							{
								CATTPRecord.SND_WIN_SIZE = CATTPRecord.SND_UNA_PDU_SEQ_NB -1+CATTPRecord.SND_WIN_SIZE -CATTPRecord.CUR_PDU_ACK_NB;
							}
							
							CATTPRecord.SND_UNA_PDU_SEQ_NB = CATTPRecord.CUR_PDU_ACK_NB+1;
							
							//flush acknowledge PDUs
						}
					}
				}
				
				if((CATTPRecord.CUR_PDU_FLAG&CATTP_HEADER_EACK_BIT) == CATTP_HEADER_EACK_BIT)   //EACK set
				{
					//flush E-acknowledge PDUs
				}
				
				if(CATTPRecord.CUR_PDU_DATA_LENGTH != 0x00) //DATA PDU
				{
					if(CATTPRecord.CUR_PDU_SEQ_NB <= (CATTPRecord.RCV_CUR_SEQ_NB + CATTPRecord.RCV_WIN_SIZE))
					{

						CATTPCopyDataToBuffers();
					}
					else
					{
						CATTPSendACKPDU(NULL);               	//send <ACK> 
						
						CATTPRecord.status = CATTP_OPEN;
						
						return TRUE;
					}
				}
				else
				{
					if((CATTPRecord.CUR_PDU_FLAG&CATTP_HEADER_NUL_BIT) != CATTP_HEADER_NUL_BIT)  //NULL NOT SET
					{
						
						if((G_CurrentSDUForSendLength !=0x00)&&(G_CATTPSndSEQ == CATTPRecord.CUR_PDU_ACK_NB))     //CATTP级联POR时候打上标志   CATTPResponseRAMPor这里面只是发送一条PDU，即推出，剩余的PDU在收到空ACK确认后继续发送
						{
							PendingMessageAlert(CATTPPOR,0x00); //打标记
						}
						
						CATTPRecord.status = CATTP_OPEN;  //discard PDU	
						return FALSE;
					}
				}
				
				//(CATTPRecord.RCV_CUR_SEQ_NB+1)  修改为 (WORD)(CATTPRecord.RCV_CUR_SEQ_NB+1))  当 RCV_CUR_SEQ_NB为FFFF时候 +1 期望和CUR_PDU_SEQ_NB 0000相等CTCC_Test_工程集-3GPP-RFM/远程文件管理测试/BIP_Test_SequenceNumber
				if((CATTPRecord.CUR_PDU_SEQ_NB == (WORD)(CATTPRecord.RCV_CUR_SEQ_NB+1))||((G_prePduNoData == TRUE)&&(CATTPRecord.CUR_PDU_SEQ_NB==CATTPRecord.RCV_CUR_SEQ_NB))) //received PDU in sequence
				{
					if(CATTPOutOfListSequenceIsEmpty() == FALSE)
					{
						CATTPRecord.RCV_CUR_SEQ_NB = CATTPRecord.CUR_PDU_SEQ_NB;
					}
					else
					{
						CATTPRecord.RCV_CUR_SEQ_NB = CATTPGetMaxSEQ();
						
						CATTPRemoveSequenceNumber();
					}
				}
				else //update list out of sequences PDUs
				{
					CATTPUpdateListOutOfSequence(CATTPRecord.CUR_PDU_SEQ_NB);
				}
						
				
				if((CATTPRecord.CUR_PDU_FLAG&CATTP_HEADER_SEG_BIT) == CATTP_HEADER_SEG_BIT) //recalculate RCV_WIN_SIZE
				{
					CATTPRecord.RCV_WIN_SIZE -= 1;
				}
				else
				{
					winsize = (CATTPRecord.RCV_SDU_SIZE_MAX)/(CATTPRecord.RCV_PDU_SIZE_MAX) + (((CATTPRecord.RCV_SDU_SIZE_MAX)%(CATTPRecord.RCV_PDU_SIZE_MAX))?1:0);
					
					if(winsize <DEFAULT_WIN_SIZE)
					{
						winsize = DEFAULT_WIN_SIZE;
					}
					//CATTPRecord.RCV_WIN_SIZE = DEFAULT_WIN_SIZE;
					CATTPRecord.RCV_WIN_SIZE =winsize;
				}
								
				if((CATTPRecord.CUR_PDU_DATA_LENGTH != 0x00) ||((CATTPRecord.CUR_PDU_FLAG&CATTP_HEADER_NUL_BIT) == CATTP_HEADER_NUL_BIT))  //NUL PDU .ACK PDU+DATA SEQ加1
				{
					G_prePduNoData = FALSE;  //该PDU带数据，下一条PDU的SEQ应比当前条的SEQ 多1
				}
				else
				{
					G_prePduNoData = TRUE;  //此条PDU无数据，则下一条PDU的SEQ与当前条的SEQ相同，
				}
				
				if((CATTPRecord.CUR_PDU_FLAG&CATTP_HEADER_SEG_BIT) != CATTP_HEADER_SEG_BIT)  //SDU not COMPELETE,还有后续PDU待接收
				{
					G_LASTSDU_NB = CATTPRecord.CUR_PDU_SEQ_NB;  //保存最后一个PDU的序列号

					G_LASTSDU_Flag = TRUE;
				} 
				
				if(CATTPOutOfListSequenceIsEmpty() == FALSE ) //Send ACK
				{
					if(CATTPRecord.RCV_CUR_SEQ_NB < G_LASTSDU_NB)
					{
						CATTPSendACKPDU(NULL);

						return TRUE;
					}
				}
				else  
				{
					CATTPSendEACKPDU();  //send ACK+EACK

					return TRUE;
				}



				if((CATTPRecord.RCV_CUR_SEQ_NB >= G_LASTSDU_NB)&&(CATTPPduBufferIsEmpty() == FALSE)&&(G_LASTSDU_Flag == TRUE))  //一条完整的SDU 接收成功,并且当前有数据需要处理
				{
					PendingMessageAlert(D_CATTPProcessSDU,0);
							
					CATTPRecord.status = CATTP_OPEN;	
					
					return 0x02;
				}
				else
				{
					CATTPSendACKPDU(NULL);
					
					if(!SE_ONLY)//和EPX2.0一致，M2M/HB_Offline_Test/8安全性测试/Tele_TC13_Full_Profile_Installation_Process_TEST_01	//同步epx2.1产业化中的修改到此处-jiangiqngbin-20220809
					{
						cephMoreTimeSingle();
					}

//					cephMoreTimeSingle();//退出函数外面开启timer
					
//					CATTPStartTimer();
				}
					
				CATTPRecord.status = CATTP_OPEN;	
				
				return TRUE;		
			}
		}
	}
	else
	{	

		if(G_CATTPSndNextSeqNB == TRUE)
		{
// 根据测试部脚本修改			CATTPRecord.SND_NXT_SEQ_NB -= 1;  //若卡片前一次成功发送ACKwithDATA，SND_NXT_SEQ_NB已加1，当卡片收到重发数据时，发送的ACK PDU中的SND_NXT_SEQ_NB应与之前的ACKwithDATA的SND_NXT_SEQ_NB一致
			
			G_CATTPSndNextSeqNB = FALSE;
		}	    
		
		CATTPSendACKPDU(NULL);  //send ACK,discard PDU

		CATTPRecord.status = CATTP_OPEN;

		cephMoreTimeSingle();                    
		
		return 0x02;	
	}
}

/*
*@description
*
*@param
*
*@return
*
*@history
*/
BYTE isDestPortInUse(void)
{
	return FALSE;
}
/*
*@description :在CLOSE状态下执行收到的PDU
*
*@param
*
*@returns
*
*@history
*/
BYTE  CATTPProcessPduArrivalInClose(void)
{
	if(CATTPPduValid() == FALSE)  //PDU invaid ---> discard PDU
	{
		return FALSE;
	}
	
	if((CATTPRecord.CUR_PDU_FLAG&CATTP_HEADER_RST_BIT) == CATTP_HEADER_RST_BIT)   //RST set
	{
		CATTPRecord.status =CATTP_CLOSE;
		return TimerClose;   //discard PDU, CLOSE
	}
	else
	{
		if((CATTPRecord.CUR_PDU_FLAG&CATTP_HEADER_ACK_BIT) == CATTP_HEADER_ACK_BIT)   //ACK  set
		{
			CATTPSendRSTPDU(0x04 ,CATTPRecord.CUR_PDU_ACK_NB +1); 	
		}
		else
		{
			if((CATTPRecord.CUR_PDU_FLAG&CATTP_HEADER_SYN_BIT) == CATTP_HEADER_SYN_BIT)   //SYN  set
			{
				if(isDestPortInUse())
				{
					//CATTPSendRST_ACKPDU(0x02);
					CATTPSendRSTPDU(0x02,0x00);
				}
				else
				{
					//CATTPSendRST_ACKPDU(0x03);
					CATTPSendRSTPDU(0x03,0x00);
				}
				
			}
			else  //SYN NOT SET
			{
				//CATTPSendRST_ACKPDU(0x04);
				CATTPSendRSTPDU(0x04,0x00);
			}
		}
		
		CATTPRecord.status =CATTP_CLOSE;
		
		return FALSE;
	}	
}
/*
*@description :处理接受到的PDU 
*
*@param       :void
*
*@returns    
*
*@history  
*/
BYTE CATTPPduArrivalEvents(void)
{

	BYTE idata result;

	result = FALSE; 
		
	switch(CATTPRecord.status)
	{
		case CATTP_CLOSE:
			result = CATTPProcessPduArrivalInClose();
			break;
		case CATTP_LISTEN:
			break;
		case CATTP_OPEN:
			result = CATTPProcessPduArrivalInOpen();  //中国联通NFC-SWP测试工程集测试脚本20160321\GP2.2.1_UICC_ComplianceTests(联通版)-20141112\测试脚本\uicc_configuation_test.uce\SECURED\SDTreeExt_Secured
			break;
		case CATTP_SYN_SENT:
			result= CATTPProcessPduArrivalInSynSent();

			CATTPPushPOR(CATTPRecord.status);
		
			if((result == FALSE)&&(CATTPRecord.status != CATTP_CLOSE))	   //脚本要求，不影响
			{
				if(!SE_ONLY)//和EPX2.0一致，M2M/HB_Offline_Test/8安全性测试/Tele_TC13_Full_Profile_Installation_Process_TEST_01
				{
				   CATTPStartTimer(); //中国联通NFC-SWP测试工程集测试脚本20160321\GP2.2.1_UICC_ComplianceTests(联通版)-20141112\测试脚本\uicc_configuation_test.uce\SECURED\SDTreeExt_Secured\scenario_00_CATTP
				}
			}
	
			break;
		case CATTP_SYN_RCVD:
			break;
		case CATTP_CLOSE_WAIT:
			break;
		default: //白盒测试添加
			break;
	}
	

	if(CATTPRecord.status ==  CATTP_CLOSE)	 //按照联通脚本要求，对于RST短信，先关timer，再close channel，最后setupEvenlist(9900),by xwh 20160303
	{		
		//注销事件
		cephSetUpEvenListSingle(NULL);
	
		//先关timer		
		if(result == TimerClose)
		{
			CATTPCloseTimer();	   
		}			
		
		//关闭通道
		cephCloseChannelSingle(0x00,CATTPRecord.channelId);
		
//		G_CATBIPChannelId = FALSE; // 通道关闭后，把该值清掉 <scenario_01_CATTP_30_6d_04>
				
		CATTPClearFile(EF_PDUFILE);  //清空缓存数据
		
#if UICC__CDMA__CATTP__PRL
		CATTPClearFile(EF_CATTPSave);  //PRL缓存文件
#endif
		BIPModuleInit();
#if M2M_EUICC
		if(G_RefreshAfterCloseBIP == TRUE) //有refresh需要在cattp关闭后发送  CATTP--eUICC_M2M_SGP_11_v3_3_Card_Compliance_Test_Suite_v2_2_0/TC_ES5_DISP_2_DisableProfile_CAT_TP_01_Nominal_Case
		{
			G_RefreshAfterCloseBIP = FALSE;
			
			sendRefreshComm();			
		}
#endif
		return CMD_Success;
		
	}

	if(SE_ONLY)
	{
		if(result != 0x02)
		{
			CATTPStartTimer();
		}
	}
	//根据规范TS102127： To detect missing PDUs, the sending CAT_TP entity shall use a retransmission timer for each SYN, NUL and data PDU transmitted
	//CATTPPduArrivalEvents 调用的函数中，不会发送SYN NUL 和data PDU，所以timer关闭之后，无需再次打开timer

	return result;
}


/*
*@function   :CATTPSaveEnvPara
*
*@description: 保存开通CATTP时的参数信息，包括SPI KIC KID TAR COUNTER等。该信息位于EF_CATTP的第一条记录，所以该函数一定要在调用CATTPOpenLink之后再调用
*
*@param      :extendType : 0x00 compactType/ 0x81: extend AA/ 0x80: extend AE
*
*@return     :void
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20170828
*/
#define CATTP_PushType_Offset   0x00
#define CATTP_TarPara_Offset    0x01
#define CATTP_SPIPara_Offset    0x04
#define CATTP_SMSCPara_Offset   0x10
#define CATTP_TPOAPara_Offset   0x20
#define CATTP_CMDNumber_Offset  0x30
void CATTPSavePushPara(BYTE extendType)
{
	UWORD xdata fileAddr;
	BYTE idata  tag,tpoaLen;
	
// #if UICC__CDMA__SMS__DOWNLOAD
// 	UWORD xdata cdmaHeader;
// 	cdmaHeader = 0;
// #endif
	fileAddr = SelectPrivateLFFile(DF_CATTP,EF_CATTPPUSH);	 
	
	if(fileAddr == FALSE)
	{
		return;
	}

	tempBuf[CATTP_PushType_Offset] = extendType;   //TS102226_v12版本，表示AA 或AE tag/ 00 表示压缩格式
	
	ecpMem_Copy(&tempBuf[CATTP_TarPara_Offset], G_TARRecIndex, 0x03);
	
	ReadTFBuf(G_EnvelopHanderAddr,G_formatedSPIOffset,&tempBuf[CATTP_SPIPara_Offset], 0x0C);	    //SPI1:spi[0]   SPI2:spi[1] ，KIC KID	TAR(3bytes) COUNTR(5bytes)
		
	ReadTFBuf(G_EnvelopHanderAddr,0x04,&tag,sizeof(BYTE));   //Address TAG			 			
	
	if((tag&0x7F)==0x06)
	{
		ReadTFBuf(G_EnvelopHanderAddr,0x05,&tempBuf[CATTP_SMSCPara_Offset] ,0x10);   //Read SMSC
	}
	else
	{
		tempBuf[CATTP_SMSCPara_Offset] = 0;		 //预留0x10字节保存SMSC地址
	}	

// #if UICC__CDMA__SMS__DOWNLOAD
// 	if((G_IsCdmaSms == TRUE)&&(G_cdmaRecNum !=FALSE))
// 	{
// 		cdmaHeader = SelectPrivateLFFile(DF_Toolkit,EF_CSMS_Head);
// 			
// 		ReadLFBuf(cdmaHeader,G_cdmaRecNum,RFM_GSM_Originating_Address + 1,&tpoaLen ,sizeof(BYTE));  //直接到tpoa字符个数
// 			
// 	}
// 	else
// #endif
	{			
		ReadTFBuf(G_EnvelopHanderAddr,G_formatedTPOAOffset,&tpoaLen ,sizeof(BYTE));   //获取TPOA字符个数	：
	}
	
	tpoaLen = 2+ (tpoaLen/2) +(tpoaLen&0x01);   //tpoa length TPOA字符个数+ TPOA类型 +TPOA 	

// #if UICC__CDMA__SMS__DOWNLOAD
// 	if((G_IsCdmaSms == TRUE)&&(G_cdmaRecNum !=FALSE))
// 	{
// 		ReadLFBuf(cdmaHeader,G_cdmaRecNum,RFM_GSM_Originating_Address + 1,&tempBuf[CATTP_TPOAPara_Offset + 1] ,tpoaLen);	
// 	}
// 	else
// #endif
	{
		ReadTFBuf(G_EnvelopHanderAddr,G_formatedTPOAOffset,&tempBuf[CATTP_TPOAPara_Offset + 1] ,tpoaLen);   //获取TPOA字符个数	
	}	
	
	tempBuf[CATTP_TPOAPara_Offset ] = tpoaLen;
	
	tempBuf[CATTP_CMDNumber_Offset] = G_CurrentComNUM;

	WriteLFBuf(fileAddr,0x01,0,tempBuf,GetLFRecNumLgth(fileAddr, FileLgth));
		
}
/*
*@function: CATTPPushPOR
*
*@description:判断在收到PUSH交互流程结束后，时候要上发POR
*
*@param      :cattpStatus: 当前CATTP 的状态
*
*@return      :void
*
*@author     :xuyz
*
*@history    :V1.0 :create by xuyz@20170828
*/
void CATTPPushPOR(BYTE cattpStatus)
{
	UWORD xdata fileAddr;
	BYTE  xdata spi[0x0C],cc[0x10];
	BYTE  idata pcntr,cclength,blockSize,totalLen,UDLLgth,additionalLen;
	BYTE idata tpoaLen,offset,cmdNumber,cmdType;
	
	fileAddr = SelectPrivateLFFile(DF_CATTP,EF_CATTPPUSH);	  
	
	if(fileAddr == FALSE)
	{
		return;
	}
	
	
	ReadLFBuf(fileAddr,0X01,CATTP_SPIPara_Offset,&spi[0], 0x0C);	    //SPI1:spi[0]   SPI2:spi[1] ，KIC KID	TAR(3bytes) COUNTR(5bytes)
																																 
	if(((spi[1]&0x03) == 0x00) ||((spi[1]&0x03) == 0x03))  // No PoR reply to the Sending Entity (SE) OR  Reserved
	{
		return;
	}
	
	if(((spi[1]&0x02) == 0x02) && (cattpStatus != CATTP_CLOSE))  // PoR required only when an error has occurred
	{
		return;
	}
	
	ReadLFBuf(fileAddr,0X01,CATTP_TarPara_Offset,&G_TARRecIndex[0], 0x03);
	
	cclength = GetCcLength((spi[1]>>2)&0x03,0);	

	if(cclength == 0xFF)
	{
		return; 
	}
	
	ReadLFBuf(fileAddr,0x01,CATTP_PushType_Offset,&cmdType, 0x01);
	
	ReadLFBuf(fileAddr,0x01,CATTP_CMDNumber_Offset,&cmdNumber, 0x01);
	
	if(cmdType == FormatType_Compat)  //压缩格式
	{
		RecData[0] = cmdNumber;
		RecData[1] = 0x90;
		RecData[2] = 0x00;
		additionalLen = 3;
		
	}
	else if(cmdType == FormatType_ExtendAA)  //AA格式
	{
		RecData[0] = 0xAB;
		RecData[1] = 4*cmdNumber + 3;
		RecData[2] = 0x80;
		RecData[3] = 0x01;
		RecData[4] = cmdNumber;
		
		for(pcntr = 0 ; pcntr < cmdNumber; pcntr ++)
		{
			RecData[5 + pcntr*4] = 0x23;
			RecData[6 + pcntr*4] = 0x02;
			RecData[7 + pcntr*4] = 0x90;
			RecData[8 + pcntr*4] = 0x00;
		}
		additionalLen = RecData[1] + 2;
	}
	else
	{
		RecData[0] = 0xAE;
		RecData[1] = 0x80;
		
		for(pcntr = 0 ; pcntr < cmdNumber; pcntr ++)
		{
			RecData[2 + pcntr*4] = 0x23;
			RecData[3 + pcntr*4] = 0x02;
			RecData[4 + pcntr*4] = 0x90;
			RecData[5 + pcntr*4] = 0x00;
		}
		
		RecData[2 + cmdNumber*4] = 0x00;
		RecData[3 + cmdNumber*4] = 0x00;
		
		additionalLen =  4*cmdNumber + 4;  //参考TS102226 V12版本的规范进行相应数据的拼装
	}
	
	pcntr = 0x00;	
	
	blockSize = 0x08;
	
	if((spi[1]&0x10) == 0x10)	 //PoR response shall be ciphered	 
	{
		if((spi[2] & 0x02) == 0x02)
		{
			blockSize = 0x10;
		}
		
		pcntr = blockSize-((cclength+7+additionalLen)%blockSize);	 //cclength+additionalLen+7 = CNTR(5bytes) PCNTR(1bytes)  StatusCode(1bytes) +cc+addtional
		
		if(pcntr == blockSize)
		{
			pcntr = 0;
		}
	}
	
	totalLen = 0x00;
	tempBuf[0] = 0x02;
	tempBuf[1] = 0x71;
	tempBuf[2] = 0x00;  // 计算mac的数据参考规范TS23.048 chapter 6.4,另外TS31115 4.5章节也明确规定027100参与mac计算	

	UDLLgth = 1+3+5+1+1+cclength+additionalLen+pcntr; //CHL 1 + TAR 3 + CNTR 5 + PCNTR 1 + Status Code 1 + CC cclength + padding pctr
	tempBuf[3] = (UDLLgth>>8)&0xFF;	     //CPL high
	tempBuf[4] = UDLLgth&0xFF;           //CPL low
	totalLen += 2;

	tempBuf[5] = 0x0A+cclength;          //CHL
	ecpMem_Copy(&tempBuf[6],&spi[4],0x08);	 //TAR CNTR
	
	totalLen +=9;

	tempBuf[14] = pcntr;                 //PCNTR			
	tempBuf[15] = 0x00;             //statusCode
	totalLen +=2;
	
    ecpMem_Copy(&tempBuf[16], RecData, additionalLen);
	
	totalLen += additionalLen ;
	
	ecpMem_Set(&tempBuf[16+additionalLen],0x00,pcntr);
	
	if((spi[2] & 0x02) == 0x02)
	{
		tempBuf[16+additionalLen] = 0x80;
	}
	
	totalLen += pcntr;   //计算CC/RC/DS的数据长度
	
	if(cclength != 0x00)
	{							
		Toolkit_CalPORMac(spi[1],spi[3],cc,cclength,&tempBuf[0],totalLen+3); //这里total+3是因为计算totalLen时没有加上前三个字节的027100
				
		ecpMem_Move(&tempBuf[16+cclength],&tempBuf[16],totalLen-13);
		ecpMem_Copy(&tempBuf[16],cc,cclength);
		totalLen += cclength;
	}
	
	if((spi[1]&0x10)==0x10)
	{
		ToolkitDeChipherRe(spi[2],&tempBuf[9],&tempBuf[9],MODE_ENCRYPT,totalLen-6,FIRST_LAST_SMS);  //从CNTR开始加密, 这里totalLen-6之后如果不是8的倍数，则后面不会执行加密操作，所以抛异常
	}
	
	if((spi[1]&0x20) == 0x20)  //SMS -SUBMIT
	{
		InitProactiveCommand(CMD_SENDSHORTMESSAGE,0x00,DEV_NETWORD);
		
		RecData[0] = TLV_ADDRESS;
		RecData[1] = 0xFF; 
		
		ReadLFBuf(fileAddr,0X01,CATTP_SMSCPara_Offset,&RecData[1], 0x10);
		
		if(RecData[1]<=11)	     //SMSC长度最大11字节
		{
			AddTLV(&ResData[1],&RecData[0]); 
		}
		
		RecData[0] = 0x8B;
		RecData[1] = 0x00;		
		RecData[2] = 0x41; //TP_MTI-TP_SRR
		RecData[3] = 0x00; //TP_MR
		
		tpoaLen = ReadLFFileValue1ByteLen(fileAddr,0x01,CATTP_TPOAPara_Offset,&RecData[4]);
	 
		offset = 4+ tpoaLen;
				
		RecData[offset] = 0x00;  //PID, 根据GSM 03.40  9.2.3.9, Note that for the straightforward case of simple MS-to-SC short message transfer the Protocol Identifier is set to the value 0
		RecData[offset+1] = 0xF6;  //DCS												
		offset += 2;
		
		RecData[offset] = 3+totalLen;  //UDL	
		RecData[offset+1] = 0x02;	
		RecData[offset+2] = 0x71;	
		RecData[offset+3] = 0x00;						
		offset += 4;

		ecpMem_Copy(&RecData[offset],&tempBuf[3],totalLen);
		offset += totalLen;
		
		RecData[1] = offset - 2;
		
		AddTLV(&ResData[1], &RecData[0]);
		
// #if UICC__CDMA__SMS__DOWNLOAD				
// 		TransSMSFomat(NO_SMSC);
// #endif		
		SendProactiveCommand();
	
	}
	else
	{
		 //目前EUICC规范: SGP.02_v3.1.pdf 2.4.3 章节 SPI2 shall be set to: ‘00’: no PoR (this value shall only be used for the notification described in section 3.15.1),  or to ‘39’: PoR with CC and encryption.
	}
	
}
/*
*@description:L在收到PushCommand之后进行CATTP链接
 *
 *@param 
 *
 *@return
 *
 *@author
 *
 *@history
*/
void CATTPOpenLink(void)
{
	BYTE idata i,flag,result;
	UWORD xdata fileAddr;
	BYTE xdata cattpPort[0x02],SDUSize[0x02];
	BYTE xdata identification[0x20];

	
	BYTE idata b36Tag,b39tag;
	
//	ecpMem_Set(&SDUSize[0],0x00,0x02);
	
	SDUSize[0x00] = ((MAX_SDU_SIZE>>8)&0xFF);
	
	SDUSize[0x01] = (MAX_SDU_SIZE&0xFF);  //buffer size
	
	fileAddr = SelectPrivateLFFile(DF_CATTP,EF_CATTPPUSH);	

  if(fileAddr == FALSE)
	{
		 return ;
	}  

	if(G_CATTPOperation == TRUE) //当前下发了PushCommand，但是CATTP链接已经打开，则不需要执行操作
	{
		SetLFConst(fileAddr, 0x01,0x00,0x00,sizeof(BYTE));

        SetLFConst(fileAddr, 0x02,0x00,0x00,sizeof(BYTE));	 //清除存放BIP信息，CATTP信息的文件

		return;
	}	
	
	result = BIPModuleOpenChannel(ChannelDataType_CATTP);
	
	if(result != CMD_Success)
	{
		result = BIPModuleOpenChannel(ChannelDataType_CATTP);
		
		if(result != CMD_Success)
		{
			return;
		}
	}

	tempBuf[0] = ReadLFFileValue1ByteLen(fileAddr,0x02,0x00,&tempBuf[1]);
	
	identification[0] = 0x00;

	CATTPClearFile(EF_PDUFILE);
	
	i = 1;

	flag = FALSE; // 表示PUSH命令中目的端口号是否存在

	b36Tag=FALSE;
	
	b39tag = FALSE; //默认 SDU SIZE从触发短信获取,如果没有带的话 从Profile获取,再没有则使用默认值
	while(i < tempBuf[0])
	{
		switch(tempBuf[i]&0x7F)
		{
			case 0x3C:   //interface transport level,for CATTP ,porttype无意义
			{			
				cattpPort[0x00]  = tempBuf[i +3];  
				
				cattpPort[0x01]  = tempBuf[i +4]; 
				
				flag = TRUE;  //目的端口号存在
				
				break;
			}
			case 0x39:  //buffer size  =MAX SDU SIZE
			{
				SDUSize[0x00] = tempBuf[i +2]; 
				SDUSize[0x01] = tempBuf[i +3]; 

				b39tag = TRUE;
				break;
			}
			case 0x36: //channel data = identification data,当该字段不存在时，identification data 为ICCID  8.53 Channel data
			{
				b36Tag=TRUE;
				ecpMem_Copy(identification,&tempBuf[i+1] ,tempBuf[i+1]+1);
				
				break;
			}
			default: //白盒测试添加
				break;
		}
		
		i += tempBuf[i+1]+2;
	}
	
	tempBuf[0] = 0x02;	
	
	tempBuf[1] = 0x09;  //Data available;
	
	tempBuf[2] = 0x0A;  //Channel status;
	
	cephSetUpEvenListSingle(&tempBuf[0]);  //Setup EvenList		

   //----------------进行CATTP链接----------------------------------------------------

	clearCatTPRecord();

	CATTPRecord.channelId =	G_BIPModuleStatus.channelNum;
	
	if(flag == FALSE)  //return  error :Destport not specified 
	{
		CATTPRecord.status = CATTP_CLOSE;
		
		return;
	}
#if (EUICC_ONLY||SE_AND_EUICC)
	if(b39tag == FALSE)
	{
		IF_GetCATTPPara(SDUSize,0x00);  //获取PDU Size
	}
#endif
	CATTPRecord.RCV_SDU_SIZE_MAX = (((WORD)SDUSize[0x00]) <<8)&0xFF00 + SDUSize[0x01];
	
	if(CATTPRecord.RCV_SDU_SIZE_MAX > MAX_SDU_SIZE)  //TS102 226有说明，当SDU大小超过卡片所容许的SDU 大小时，使用卡片提供的
	{
		CATTPRecord.RCV_SDU_SIZE_MAX = MAX_SDU_SIZE;
	}
	
	CATTPRecord.desPort = ((((WORD)cattpPort[0x00]) <<8)&0xFF00) | cattpPort[0x01];
	
	G_CATTPOperation = TRUE;  //开始CATTP连接操作
//	if(G_CATTPType == JCREMain_ISDRRAMMode)
//	{
		CATTPRecord.RCV_PDU_SIZE_MAX = (((WORD)G_PDUSize[0x00]) <<8)&0xFF00 | G_PDUSize[0x01];;  //Update RCV_PDU_SIZE_MAX,RCV_SDU_SIZE_MAX, from Open request parameters
//	}
//	else
//	{
//		CATTPRecord.RCV_PDU_SIZE_MAX = MAX_PDU_SIZE;
//	}
	if(MOBILE_Operator == OPERATOR_CHINATELECOM)  //add by chentao@20211020  中国电信特殊需求
	{

		if(CATTPRecord.RCV_PDU_SIZE_MAX >MAX_PDU_SIZE)  //大于电信的需求0x200  则SendData时候发送0x200，Openchannel时候用push指令里面的大小bufsize
		{
			CATTPRecord.RCV_PDU_SIZE_MAX = MAX_PDU_SIZE;
		}
	}
	// 电信 PUSH中identificatin长度为0, 此时SYN中的identification长度也应该为0
	//36tag不存在，identification长度为0，发ICCID
	//36tag存在，identification长度为0，发0
	if((identification[0] == 0x00) && (b36Tag==FALSE))
	{
#if (EUICC_ONLY||SE_AND_EUICC)
		if(G_CATTPType == JCREMain_ISDRRAMMode) //ISDR 打开链接时候发送EID
		{
			IF_Get_EUICCEID(identification);
		}
		else
#endif
		{
			fileAddr =SelectTFFile(DF_MF,EF_ICCID);
		
			identification[0x00] = 0x0A;
		
			ReadTFBuf(fileAddr,0x00,&identification[0x01], 0x0A);  //SYNPDU 默认identification data 			
			
		}
	}
	
	G_CATTPType = TarType_Error;
	
	CATTPActiveOpenRequest(identification);	
	//------------------------------------------------------------------------------------------------------
	CATTPStartTimer();
}
/*
*@Description: 计算当前数据的check sum值
*
*@param      : dataIn: V
*@param      : dataLen : 数据长度
*
*@return     :WORD :checksum
*/
WORD CATTPCalCheckSum(BYTE *dataIn,WORD dataLen)
{
	UWORD idata i,sum;
	
	sum = 0;
	
	for(i=0;i<dataLen;)
	{
			if(i == (dataLen-1))
			{
					sum += (dataIn[i] << 8) & 0xff00;
			}
			else
			{
				sum += (dataIn[i] << 8) | dataIn[i+1];
			}
      
	  if(sum > 0xFFFF)
	  {
		  sum +=1;
		  
		  sum &= 0xFFFF;
	  }
	  
	  i+=2;
	  
	}
	
	return (WORD)(~sum);
}
/*
@description:
*
*@param
*
*@returns
*
*@history:
*/
void CATTPActiveOpenRequest(BYTE *identification)  //lv
{
	BYTE idata result;
	
	CATTPRecord.isActive = CATTP_ACTIVE; 
	
	CATTPRecord.SND_INI_SEQ_NB =  DEFAULT_INI_SEQ_NB;  //figure 11 TS102 127
	
	CATTPRecord.SND_NXT_SEQ_NB = CATTPRecord.SND_INI_SEQ_NB +1;
	
	CATTPRecord.SND_UNA_PDU_SEQ_NB = CATTPRecord.SND_INI_SEQ_NB;

	
	CATTPRecord.srcPort =  DEFAULT_LOCAL_PORT ; //分配一个本地端口号
		
	result = CATTPSendSYNPDU(identification);
	
	if(result != CMD_Success)
	{
		CATTPRecord.status = CATTP_CLOSE;
		
		return ;
	}	
	
	CATTPRecord.status = CATTP_SYN_SENT;
}
/*
*@description:释放timer
*
*@function   : CATTPRleaseTimer
*
*@param      :void
*
*@return     :void
*/
void CATTPRleaseTimer(void)
{
// 	BYTE idata i,timer;
// 	UWORD idata fileAddr;
// 	
// 	fileAddr = SelectPrivateTFFile(DF_Toolkit, EF_TIMER);
// 	
// 	for(i= 0; i<GetTFSize(fileAddr); i++)
// 	{
// 		ReadTFBuf(fileAddr, i ,&timer,sizeof(BYTE));
// 		
// 		if((timer&0x0F) == TimerIdentifier)  
// 		{			
// 			SetTFConst(fileAddr, i ,timer&0x0F,sizeof(BYTE));
// 			
// 			break;
// 		}
// 	
// 	}
	
	TimerModuleReaseTimerId(&G_CATTP_TimerIdentifier);
}

/*
*description :关闭timer，即释放timrId
*
*@patam      :
*
*@returns    :
*
*@history    : V1.0 --- Creat 
*                   V1.1---Modify by xwh ,如果当前没有开启的Timer，那么不用关闭Timer
*/
BYTE CATTPCloseTimer(void)
{
// 	BYTE idata i,timer;
// 	UWORD idata fileAddr;

	if(G_CATTP_TimerIdentifier == FALSE)
	{
		return FALSE;                                                             //如果当前没有开启的Timer，那么不用关闭Timer
	}
	
	TimerManagementProactive(Close_Timer,G_CATTP_TimerIdentifier,NULL);
	
	TimerModuleReaseTimerId(&G_CATTP_TimerIdentifier); // G_CATTP_TimerIdentifier = 0xFF;  //无效的 timer identifier
	
	return TRUE;
	
}
/*
*@description:Timer Management 之start timer
*
*@param  
*
*@returns  
*
*@history    :
*/
BYTE code cattptimervalue[] = {0x03,0x00,0x00,0x03};
BYTE CATTPStartTimer(void)
{
//	BYTE xdata timerValue[0x04];
	BYTE idata result;
// 	BYTE idata i,timer;
// 	UWORD idata fileAddr;
// 	
// 	fileAddr = SelectPrivateTFFile(DF_Toolkit, EF_TIMER);
// 	
// 	for(i= 0; i<GetTFSize(fileAddr); i++)
// 	{
// 		ReadTFBuf(fileAddr, i ,&timer,sizeof(BYTE));
// 		
// 		if((timer&0x80) == 0x00)
// 		{
// 			TimerIdentifier = timer;     //每次开启timer ，使用不同的timer identifier
// 			
// 			SetTFConst(fileAddr, i ,timer|0x80,sizeof(BYTE));
// 			
// 			break;
// 		}
// 	}
// 	
// 	if(i == 0x08)  //无可使用的timer identifier
// 	{
// 		return FALSE;
// 	}
// 	
	result = TimerModuleAllocTimerId();  //向系统申请一个timer
	
	if(result == FALSE)
	{
		return FALSE;
	}
	
	G_CATTP_TimerIdentifier = result;
	
// 	timerValue[0x00] = 0x03;
// 	timerValue[0x01] = 0x00;				//根据规范，定时器时间设置为30s
// 	timerValue[0x02] = 0x00;
// 	timerValue[0x03] = 0x03;
	
	result = TimerManagementProactive(Start_Timer,G_CATTP_TimerIdentifier,(BYTE *)&cattptimervalue[0]);
	

	if(result != CMD_Success)
	{
		return result;
	}
	
	if(G_BIPDataAvaiableEvent != TRUE)
	{
		result = cephMoreTimeSingle();
	}
	
	return result;
	
}

BYTE CATTPStartTimerOnly(void)
{
	//BYTE xdata timerValue[0x04];
	BYTE idata result;
	
// 	BYTE idata i,timer;
// 	UWORD idata fileAddr;
// 	
// 	fileAddr = SelectPrivateTFFile(DF_Toolkit, EF_TIMER);
// 	
// 	for(i= 0; i<GetTFSize(fileAddr); i++)
// 	{
// 		ReadTFBuf(fileAddr, i ,&timer,sizeof(BYTE));
// 		
// 		if((timer&0x80) == 0x00)
// 		{
// 			TimerIdentifier = timer;     //每次开启timer ，使用不同的timer identifier
// 			
// 			SetTFConst(fileAddr, i ,timer|0x80,sizeof(BYTE));
// 			
// 			break;
// 		}
// 	}
// 	
// 	if(i == 0x08)  //无可使用的timer identifier
// 	{
// 		return FALSE;
// 	}

	result = TimerModuleAllocTimerId();  //向系统申请一个timer
	
	if(result == FALSE)
	{
		return FALSE;
	}
	
	G_CATTP_TimerIdentifier = result;
	
// 	timerValue[0x00] = 0x03;	
// 	timerValue[0x01] = 0x00;					//根据规范，定时器时间设置为30s
// 	timerValue[0x02] = 0x00;
// 	timerValue[0x03] = 0x03;	
	
	result = TimerManagementProactive(Start_Timer,G_CATTP_TimerIdentifier,(BYTE *)&cattptimervalue[0]);
	
	return result;	
}
#endif
