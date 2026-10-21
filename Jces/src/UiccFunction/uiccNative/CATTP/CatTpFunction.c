/*
*@HISTORY       :V1.0: 创建源文件

*				 V1.1：修改函数processTimeoutInCloseWaitState，在清除链接记录时，需保留通道ID，因为在关闭通道会使用CATTPRecord.channelId   @20111121 Version 1.7- >1.8
				 V1.2: 修改CATTPOpenLink函数，在电信备案测试,identification数据段为0时,不需要添加ICCID	   							  by yangjian 2019.5.5
*/
#include  "typedef.h"
#include  "main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include  "error.h"
#include  "arithmatic.h"
#include  "ceph.h"
#include  "interface.h"
#include  "CATTP.h"
#include "TimerModuleProces.h"
#include "LDFile.h"
#include "BIPModuleProcess.h"
#include "Transaction.h"


#ifdef CATTP

ConnectionRecord xdata CATTPRecord;

BYTE xdata G_CattpTimerId;		//CATTP 业务开启的timerId

//BYTE xdata G_BipPrlStatus;
BYTE xdata G_CATBIPChannelId;	//本次接收到cattp数据的通道号
BYTE xdata G_CATBIPDataLgth;
BYTE xdata G_ChannelStatusID;	//本次接收到ChannelStatus事件的通道号

WORD xdata G_CurrentPDULength;  //当前收到的PDU的长度

BYTE xdata G_RefNumber;		//批次号
BYTE xdata G_prePduNoData;   //1，前一个PDU无数据，0前一个PDU有数据
BYTE xdata G_SDUARRIVAL;  //判断当前是否接收到一条完整的SDU,TRUE:SDU 已经被完整接收 FALSE/sdu未收全
WORD xdata G_LASTSDU_NB;      //sdu 最后一块的SEQ
BYTE xdata G_LASTSDU_Flag;  //判断当前是否收到了最后一条SDU

BYTE xdata G_RetryTimers;  //CATTP链接过程中 每次发送APDU后将，此变量设置为3，如果3次超时，则关闭BIP通道

BYTE xdata G_CATTPOperation;  //该变量为TRUE时，表示进入到CATTP协议操作.初始化或释放连接后，清空该变量

BYTE xdata G_CATTPSndNextSeqNB;

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
		AddVlaue((BYTE xdata *)&RecData[1], &evenList[0]);
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
WORD CATTPCalCheckSumAddr(UWORD fileAddr,WORD dataLen)
{
	BYTE idata dataIn[0x02];
	UWORD xdata i,sum;
	BYTE idata pading;
	
	sum = 0;
	pading =0;
	
	if((dataLen % 2) !=0)
	{
		pading =1;
		dataLen -=1;
	}
	
	for(i=0;i<dataLen;)
	{
		ReadTFBuf(fileAddr, i, &dataIn[0] , 0x02);
		
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
	
	if(pading)
	{
		ReadTFBuf(fileAddr, i, &dataIn[0] , pading);
			
		sum += (dataIn[0] << 8) & 0xff00;
		
		if(sum > 0xFFFF)
		{
			sum +=1;
			
			sum &= 0xFFFF;
		}

	}
	
	sum &= 0xFFFF;
	
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
*@description  :拼装PDU共同部分
*
*@param        :
*
*@return
*
*@history      :
*/
void CATTPSendCOMMOMPDU(void)
{

	tempBuf[2] = 0x00;  //RFU
	
	tempBuf[3] = 0x00; //rfu

	
	tempBuf[5] = (CATTPRecord.srcPort >>8)&0xFF;
	
	tempBuf[6] = CATTPRecord.srcPort&0xFF;
	
	tempBuf[7] = (CATTPRecord.desPort>>8)&0xFF;
	
	tempBuf[8] = CATTPRecord.desPort&0xFF;
	
	tempBuf[9] = 0x00;

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
	
//	tempBuf[2] = 0x00;  //RFU
//	
//	tempBuf[3] = 0x00; //rfu
	
	tempBuf[4] = 0x12; //header length
	
//	tempBuf[5] = (CATTPRecord.srcPort >>8)&0xFF;
//	
//	tempBuf[6] = CATTPRecord.srcPort&0xFF;
//	
//	tempBuf[7] = (CATTPRecord.desPort>>8)&0xFF;
//	
//	tempBuf[8] = CATTPRecord.desPort&0xFF;
//	
//	tempBuf[9] = 0x00;
	
	CATTPSendCOMMOMPDU();   //20160704  代码共同部分
	
	tempBuf[10] = dataLgth;
	
	tempBuf[11] = (CATTPRecord.SND_NXT_SEQ_NB >>8)&0xFF;
	
	tempBuf[12] = CATTPRecord.SND_NXT_SEQ_NB&0xFF;
	
	tempBuf[13] = (CATTPRecord.RCV_CUR_SEQ_NB >>8)&0xFF;
	
	tempBuf[14] = CATTPRecord.RCV_CUR_SEQ_NB&0xFF;
	
	tempBuf[15] = (CATTPRecord.RCV_WIN_SIZE>>8)&0xFF;
	
	tempBuf[16] = CATTPRecord.RCV_WIN_SIZE&0xFF;
	
	tempBuf[17] = 0x00; //check sum1
	
	tempBuf[18] = 0x00; //check sum2
	
	if(dataBuf != NULL)
	{
		memcpy(&tempBuf[19], &dataBuf[1],dataLgth);  //拷贝数据
	}
	
	checkSum = CATTPCalCheckSum(&tempBuf[1],tempBuf[0]);
	
	tempBuf[17] = (checkSum>>8)&0xFF;
	
	tempBuf[18] = checkSum&0xFF;

	if((CATTPRecord.status == CATTP_SYN_SENT)||(CATTPRecord.status == CATTP_OPEN)&&(dataBuf != NULL))	    //cattp链接未建立或者CATTP链接建立后，但是发给服务器的POR服务器端未处理
	{

		WriteVariable(VaribleTag_BackPDU,&tempBuf[0]);
	
		G_RetryTimers = 0x03;
	}
		
	result = cephSendDataSingle(0x01, G_BIPModuleStatus.channelNum,&tempBuf[0]);


	return result;


	
}
/*

*/
BYTE CATTPSendSYNPDU(BYTE *identification)
{
	WORD xdata checksum;
	BYTE idata result;
	
	//拼装SYN-PDU
	
	tempBuf[1] = CATTP_HEADER_SYN_BIT|CATTP_VERSION; //SYN  bit
	
//	tempBuf[2] = 0x00; //rfu1
//	
//	tempBuf[3] = 0x00; //rfu2
	
	tempBuf[4] = 22 + identification[0]+1;  //headerLength,
	
//	tempBuf[5] = ( CATTPRecord.srcPort >>8)&0xFF;
//	
//	tempBuf[6] =  CATTPRecord.srcPort&0xFF;  //source port
//		
//	tempBuf[7] = (CATTPRecord.desPort >>8)&0xFF;
//	
//	tempBuf[8] = CATTPRecord.desPort&0xFF;  //des port
//	
//	tempBuf[9] = 0x00;

	CATTPSendCOMMOMPDU();   //20160704  代码共同部分
	
	tempBuf[10] = 0x00;  //datalength = 0
	
	tempBuf[11] =  (CATTPRecord.SND_INI_SEQ_NB >>8)&0xFF;
	
	tempBuf[12] =  CATTPRecord.SND_INI_SEQ_NB&0xFF;  //SEQ
	
	tempBuf[13] = 0x00;
	
	tempBuf[14] = 0x00;  //ACK non signification 
	
	tempBuf[15] = 0x00;
	
	tempBuf[16] = DEFAULT_WIN_SIZE;  //WIN SIZE non signification 
	
	tempBuf[17] = 0x00;
	
	tempBuf[18] = 0x00;	//While computing the checksum, the checksum field itself shall be replaced with zeros.
	
	tempBuf[19] = (CATTPRecord.RCV_PDU_SIZE_MAX>>8)&0xFF;
	
	tempBuf[20] = CATTPRecord.RCV_PDU_SIZE_MAX&0xFF ;    //PDU SIZE
	
	tempBuf[21] = (CATTPRecord.RCV_SDU_SIZE_MAX>>8)&0xFF;
	
	tempBuf[22] = CATTPRecord.RCV_SDU_SIZE_MAX&0xFF ;    //SDU SIZE
	
	memcpy(&tempBuf[23], identification,identification[0]+1); //identification
	
	checksum = CATTPCalCheckSum(&tempBuf[1],tempBuf[4] );
	
	tempBuf[17] = (checksum >>8)&0xFF;
	
	tempBuf[18] = checksum&0xFF;
	
	tempBuf[0] = tempBuf[4];  //SYN PDU length

	WriteVariable(VaribleTag_BackPDU,&tempBuf[0]);

	G_RetryTimers = 0x03;
	
	result =  cephSendDataSingle(0x01, G_CATBIPChannelId, &tempBuf[0]);
	
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
	WORD xdata checkSum;
	BYTE idata num,result;
	UWORD xdata fileAddr;
	
	fileAddr = SelectTFFile(DF_OTA,EF_EACKSEQ);
	
	ReadTFBuf(fileAddr, 0x00, &num, sizeof(BYTE));
	

	tempBuf[1] = CATTP_HEADER_ACK_BIT|CATTP_HEADER_EACK_BIT|CATTP_VERSION;
	
//	tempBuf[2] = 0x00;
//	
//	tempBuf[3] = 0x00; //RFU
	
	tempBuf[4] = 0x12 + num*2;
	
//	tempBuf[5] = (CATTPRecord.srcPort >>8)&0xFF;
//	
//	tempBuf[6] = CATTPRecord.srcPort&0xFF;
//	
//	tempBuf[7] = (CATTPRecord.desPort>>8)&0xFF;
//	
//	tempBuf[8] = CATTPRecord.desPort&0xFF;
//	
//	tempBuf[9] = 0x00;

	CATTPSendCOMMOMPDU();   //20160704  代码共同部分
	
	tempBuf[10] = 0x00;
	
	tempBuf[11] = (CATTPRecord.SND_NXT_SEQ_NB >>8)&0xFF;
	
	tempBuf[12] = CATTPRecord.SND_NXT_SEQ_NB&0xFF;
	
	tempBuf[13] = (CATTPRecord.RCV_CUR_SEQ_NB >>8)&0xFF;
	
	tempBuf[14] = CATTPRecord.RCV_CUR_SEQ_NB&0xFF;
	
	tempBuf[15] = (CATTPRecord.RCV_WIN_SIZE>>8)&0xFF;
	
	tempBuf[16] = CATTPRecord.RCV_WIN_SIZE&0xFF;
	
	tempBuf[17] = 0x00; //check sum1
	
	tempBuf[18] = 0x00; //check sum2
	
	ReadTFBuf(fileAddr, 0x01,&tempBuf[19] ,num*2);
	
	checkSum = CATTPCalCheckSum(&tempBuf[1],tempBuf[4]);
	
	tempBuf[17] = (checkSum>>8)&0xFF;
	
	tempBuf[18] = checkSum&0xFF;
	
	result = cephSendDataSingle(0x01, G_BIPModuleStatus.channelNum,&tempBuf[0]);
	
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
*/
BYTE CATTPSendRSTPDU(BYTE reason,WORD seq)
{
	BYTE idata result;
	WORD idata checkSum;
	
	tempBuf[1] = CATTP_HEADER_RST_BIT|CATTP_VERSION;
	
//	tempBuf[2] = 0x00; //RFU1
//	
//	tempBuf[3] = 0x00;  //RFU2
	
	tempBuf[4] = 0x13;  //header lgth
	
//	tempBuf[5] = (CATTPRecord.srcPort >>8)&0xFF;
//	
//	tempBuf[6] = CATTPRecord.srcPort&0xFF;
//	
//	tempBuf[7] = (CATTPRecord.desPort>>8)&0xFF;
//	
//	tempBuf[8] = CATTPRecord.desPort&0xFF;
//	
//	tempBuf[9] = 0x00;

	CATTPSendCOMMOMPDU();   //20160704  代码共同部分
	
	tempBuf[10] = 0x00;
	
	tempBuf[11] = (seq >> 8)&0xFF;
	
	tempBuf[12] = seq&0xFF;  //seq
	
	tempBuf[13] = 0x00;
	
	tempBuf[14] = 0x00; //ACK
	
	tempBuf[15] = 0x00;
	
	tempBuf[16] = DEFAULT_WIN_SIZE;  //WIN SIZE
	
	tempBuf[17] = 0x00;
	
	tempBuf[18] = 0x00;
	
	tempBuf[19] = reason;
	
	checkSum = CATTPCalCheckSum(&tempBuf[1],tempBuf[4]);
	
	tempBuf[17] = (checkSum>>8)&0xFF;
	
	tempBuf[18] = checkSum&0xFF;
	
	result = cephSendDataSingle(0x01, G_BIPModuleStatus.channelNum,&tempBuf[0]);
	
	return result;	
	
}
/*
*@description :拼装RST ACK  PDU
*
*@param
*
*@return
*
*@history
*/
BYTE CATTPSendRST_ACKPDU(BYTE reason)
{
	BYTE idata result;
	WORD idata checkSum;
	
	tempBuf[1] = CATTP_HEADER_RST_BIT|CATTP_HEADER_ACK_BIT|CATTP_VERSION;
	
//	tempBuf[2] = 0x00; //RFU1
//	
//	tempBuf[3] = 0x00;  //RFU2
	
	tempBuf[4] = 0x13;  //header lgth
	
//	tempBuf[5] = (CATTPRecord.srcPort >>8)&0xFF;
//	
//	tempBuf[6] = CATTPRecord.srcPort&0xFF;
//	
//	tempBuf[7] = (CATTPRecord.desPort>>8)&0xFF;
//	
//	tempBuf[8] = CATTPRecord.desPort&0xFF;
//	
//	tempBuf[9] = 0x00;

	CATTPSendCOMMOMPDU();   //20160704  代码共同部分
	
	tempBuf[10] = 0x00;  //data lgth
	
	tempBuf[11] =0x00;
	
	tempBuf[12] = 0x00;  //seq
	
	tempBuf[13] = (CATTPRecord.CUR_PDU_SEQ_NB >>8)&0xFF;
	
	tempBuf[14] =  CATTPRecord.CUR_PDU_SEQ_NB &0xFF;; //ACK
	
	tempBuf[15] = 0x00;
	
	tempBuf[16] = 0x00; //win size
	
	tempBuf[17] = 0x00;
	
	tempBuf[18] = 0x00;
	
	tempBuf[19] = reason;
	
	checkSum = CATTPCalCheckSum(&tempBuf[1],tempBuf[4]);
	
	tempBuf[17] = (checkSum>>8)&0xFF;
	
	tempBuf[18] = checkSum&0xFF;
	
	result = cephSendDataSingle(0x01, G_BIPModuleStatus.channelNum,&tempBuf[0]);
	
	return result;	
	
}
/*
*@description :判断接收到的pdu是否有效
*
*@param       :
*
*@returns     :TRUE:pdu 有效/FALSE :pdu 无效
*
*@history
*/
BYTE CATTPPduValid(void)
{
	BYTE idata headerLgth;
	UWORD  xdata bipAddr;
	
	//bipAddr = SelectTFFile(DF_OTA, 0x6EAF);  //缓存BIP数据
	//ReadTFBuf(bipAddr,CATTP_HeaderLgth_OFFSET, &headerLgth,sizeof(BYTE));	
	//ReadTFBuf(bipAddr,CATTP_TagField_OFFSET,&tempBuf[0],headerLgth);
	
	bipAddr = BIPModule_Receive_ADDR_START;
	
	ReadEeprom(&headerLgth,bipAddr+CATTP_HeaderLgth_OFFSET,sizeof(BYTE));
	
	ReadEeprom(&tempBuf[0],bipAddr+CATTP_TagField_OFFSET,headerLgth);
	
	CATTPRecord.CUR_PDU_FLAG = tempBuf[CATTP_TagField_OFFSET];
	
	CATTPRecord.CUR_PDU_VRS_NB = CATTPRecord.CUR_PDU_FLAG &0x03;
	
	if((CATTPRecord.CUR_PDU_FLAG&0x80) == CATTP_HEADER_SYN_BIT)   //SYN PDU
	{
		CATTPRecord.SYN_PDU_SIZE_MAX = (((WORD)tempBuf[CATTP_PDUMAX_OFFSET]) <<8)&0xFF00 +tempBuf[CATTP_PDUMAX_OFFSET+1];
		
		CATTPRecord.SYN_SDU_SIZE_MAX = (((WORD)tempBuf[CATTP_SDUMAX_OFFSET]) <<8)&0xFF00 +tempBuf[CATTP_SDUMAX_OFFSET+1];
		
	}
	
	if((tempBuf[CATTP_SrcPort_OFFSET]   != (CATTPRecord.desPort>>8)&0xFF) &&( tempBuf[CATTP_SrcPort_OFFSET+1] !=(CATTPRecord.desPort&0xFF)))  //接受到的PDU的原端口 应该等于发送出去的PDU的目的端口
	{
		return FALSE;
	}
	
	if((tempBuf[CATTP_DesPort_OFFSET]   != (CATTPRecord.srcPort>>8)&0xFF) &&( tempBuf[CATTP_DesPort_OFFSET+1] !=(CATTPRecord.srcPort&0xFF)))  //接受到的PDU的目的端口 应该等于发送出去的PDU的源端口
	{
		return FALSE;
	}
	
	CATTPRecord.CUR_PDU_SEQ_NB = (((WORD)tempBuf[CATTP_SEQNumber_OFFSET]) <<8)&0xFF00 |tempBuf[CATTP_SEQNumber_OFFSET+1];
	
	CATTPRecord.CUR_PDU_ACK_NB =  (((WORD)tempBuf[CATTP_ACKNumber_OFFSET]) <<8)&0xFF00 |tempBuf[CATTP_ACKNumber_OFFSET+1];
	
	CATTPRecord.CUR_PDU_WIN_SIZE =  (((WORD)tempBuf[CATTP_WINSIZE_OFFSET]) <<8)&0xFF00 |tempBuf[CATTP_WINSIZE_OFFSET+1];
	
	CATTPRecord.CUR_PDU_CHECKSUM =(((WORD)tempBuf[CATTP_CHECKSUM_OFFSET]) <<8)&0xFF00 |tempBuf[CATTP_CHECKSUM_OFFSET+1];
	
	CATTPRecord.CUR_PDU_HEADER_LENGTH = headerLgth;
	
	CATTPRecord.CUR_PDU_DATA_LENGTH = ((WORD)tempBuf[CATTP_DataLgth_OFFSET]<<8)&0xFF00 | tempBuf[CATTP_DataLgth_OFFSET +1];
	
	if(CATTPRecord.CUR_PDU_CHECKSUM  != CATTPCalCheckSumAddr(bipAddr,CATTPRecord.CUR_PDU_HEADER_LENGTH+CATTPRecord.CUR_PDU_DATA_LENGTH))
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
*@return
*
*@history
*/
BYTE CATTPProcessPduArrivalInSynSent(void)
{
	if(CATTPPduValid() == FALSE)
	{
		return FALSE;
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
			return FALSE;
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
							
							CATTPRecord.RCV_WIN_SIZE = DEFAULT_WIN_SIZE;    //标准应由计算得到

							//send ACK
							CATTPSendACKPDU(NULL);
							
							CATTPRecord.status = CATTP_OPEN; 
																		
							return TRUE;
							
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
				CATTPSendRST_ACKPDU(0x04);
				
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
*@history  ：V1.1 :修改功能，修改序列号更新错误的问题
*/
void CATTPUpdateListOutOfSequence(WORD currentSeq)
{
	WORD idata seq;
	BYTE idata num,i,offset;
	UWORD idata fileAddr;
	
	fileAddr = SelectTFFile(DF_OTA,EF_EACKSEQ);
	
	ReadTFBuf(fileAddr, 0x00, &num, sizeof(BYTE));
	
	for(i=1 ;i<=num; i++)
	{
		ReadTFBuf(fileAddr,i*2-1,(BYTE *)&seq,sizeof(WORD));
		
		if(currentSeq <= seq)
		{		
			break;  //记录下当前待插入的位置i
		}
	}
	tempBuf[0] = num;
	
	ReadTFBuf(fileAddr,0x01,&tempBuf[1],(i-1)*2);
	
	offset = 1+(i-1)*2;  //point to 写入的位置
	
#ifdef LittleEndian
	tempBuf[offset ++] = currentSeq&0xFF;   //对于小端 芯片 ，高字节在前 ，低字节在后 
			
	tempBuf[offset ++] = (currentSeq >> 8)&0xFF;
#else
	tempBuf[offset ++] = (currentSeq >> 8)&0xFF;  //对于大端 芯片，高字节在后 ，低字节在前
			
	tempBuf[offset ++] = currentSeq&0xFF;
#endif	
	
	ReadTFBuf(fileAddr,i*2-1,&tempBuf[offset],((num+1)-i)*2);  //读取剩余的数据
	
	tempBuf[0] ++;
	
	WriteTFBuf(fileAddr,0x00,&tempBuf[0], tempBuf[0]*2 +1);	
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
	
	fileAddr = SelectTFFile(DF_OTA,EF_EACKSEQ);
	
	ReadTFBuf(fileAddr, 0x00, &num, sizeof(BYTE));
	
	if(num == 0x00)
	{
		return FALSE;
	}
	else
	{
		return num;
	}
	
}
void CATTPClearFile(WORD fileID)
{
	
	UWORD xdata pduAddr;
	BYTE xdata tmp[0x04];     
	
	pduAddr = SelectTFFile(DF_OTA,fileID);

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
	WORD xdata usedSpace;
	UWORD xdata pduAddr;
	
	pduAddr = SelectTFFile(DF_OTA,LDFile_RelaxFile);               //原7E06改为-->5E03
	
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
	WORD idata dataOffset,dataLgth,actual,offset,usedSpace;
	UWORD  xdata bipAddr,pdufileAddr;
	
//	pdufileAddr = SelectTFFile(DF_OTA,EF_PDUFILE);	//

	pdufileAddr = SelectTFFile(DF_OTA,LDFile_RelaxFile);    //原7E06————>5E03
	
	ReadTFBuf(pdufileAddr, 0x00, (BYTE *)&usedSpace,sizeof(WORD));
		
	dataOffset = CATTPRecord.CUR_PDU_HEADER_LENGTH;  //获取PDU 数据起始偏移
	
	dataLgth = CATTPRecord.CUR_PDU_DATA_LENGTH;
	
	offset = usedSpace+2;
	
	WriteTFBuf(pdufileAddr,offset ,(BYTE *)&CATTPRecord.CUR_PDU_SEQ_NB,sizeof(WORD));  //更新当前SEQ
	
	WriteTFBuf(pdufileAddr,offset+2,(BYTE *)&dataLgth,sizeof(WORD));
	
	offset +=4;
	
	//bipAddr = SelectTFFile(DF_OTA, 0x6EAF);  //缓存BIP的数据
	bipAddr = BIPModule_Receive_ADDR_START;								//for test
	
	while(dataLgth >0)
	{
		if ( dataLgth > 0xFF )
		{
			actual = 0xFF;
		}
		else
		{
			actual = dataLgth;
		}
		
		//ReadTFBuf(bipAddr,dataOffset, &tempBuf[0], actual);
		ReadEeprom(&tempBuf[0],bipAddr+dataOffset,actual);
		
		WriteTFBuf(pdufileAddr,offset,&tempBuf[0] ,actual);
		
		dataOffset  += actual;
		
		offset += actual;
		
		dataLgth -= actual;		
	}
	
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
	UWORD xdata fileAddr;
	
	
	currentSEQ = CATTPRecord.CUR_PDU_SEQ_NB;
	
	max = 0x00;
	
	fileAddr = SelectTFFile(DF_OTA,EF_EACKSEQ);
	
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
*@history     :V1.1:修改代码偏移错误的问题
*/

void CATTPRemoveSequenceNumber(void)
{
	WORD idata curSeq;
	BYTE idata num,i,offset;
	UWORD xdata fileAddr;
	
	fileAddr = SelectTFFile(DF_OTA,EF_EACKSEQ);
	
	ReadTFBuf(fileAddr, 0x00, &num, sizeof(BYTE));
	
	offset = 1;
	
	tempBuf[0] = 0;
	
	for(i =1 ;i <=num; i++)
	{
		ReadTFBuf(fileAddr, i*2-1 ,(BYTE *)&curSeq,sizeof(WORD));
		
		if(curSeq <= CATTPRecord.RCV_CUR_SEQ_NB)
		{
		}
		else  //挑选出不能被移除的代码，然后保存起来
		{			
#ifdef LittleEndian
			tempBuf[offset ++] = curSeq&0xFF;   //对于小端 芯片 ，高字节在前 ，低字节在后 
			
			tempBuf[offset ++] = (curSeq >> 8)&0xFF;
#else
			tempBuf[offset ++] = (curSeq >> 8)&0xFF;  //对于大端 芯片，高字节在后 ，低字节在前
			
			tempBuf[offset ++] = curSeq&0xFF;
#endif	
			
			tempBuf[0] ++;
		}
	}
	
	WriteTFBuf(fileAddr,0x00,&tempBuf[0], tempBuf[0]*2 +1);
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
	if(0)    	
	{ 
		//Send <RST>
		//<SEQ=SND_NXT_SEQ_NB>
		//<reason=‘05’>
		//Deallocate connection record
		CATTPSendRSTPDU(0x05,CATTPRecord.SND_NXT_SEQ_NB);  //retransmission counter > maximum of retries

		//signal "connection reset"
		//To Upper Layer

		CATTPRecord.status = CATTP_CLOSE_WAIT;
		
		Timeout_Events_Process();
	}
	else
	{

	}
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
	memset((BYTE *)&CATTPRecord, 0x00,sizeof(CATTPRecord));
	
	G_CATTPOperation = FALSE;
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
	
	channelId = G_BIPModuleStatus.channelNum;   //此处备份channelId，后续关闭通道时，还会使用到CATTPRecord.channelId

	clearCatTPRecord();
	
	G_BIPModuleStatus.channelNum = channelId;
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
	DWORD xdata tem;
	
	if(CATTPPduValid() == FALSE)  //PDU invaid ---> discard PDU
	{
		return FALSE;
	}
	
	tem	= (DWORD)CATTPRecord.RCV_CUR_SEQ_NB+(DWORD)CATTPRecord.RCV_WIN_SIZE+1;		//数据有可能超出2个字节表示的范围，强制转换为4个字节保存
	
	//CATTPRecord.RCV_CUR_SEQ_NB +1 == CATTPRecord.CUR_PDU_SEQ_NB  FFFF +1 == 0000	?
	//if(((CATTPRecord.RCV_CUR_SEQ_NB < CATTPRecord.CUR_PDU_SEQ_NB)||((G_prePduNoData == 0x01)&&(CATTPRecord.RCV_CUR_SEQ_NB == CATTPRecord.CUR_PDU_SEQ_NB)))&&(CATTPRecord.CUR_PDU_SEQ_NB <= (CATTPRecord.RCV_CUR_SEQ_NB+CATTPRecord.RCV_WIN_SIZE+1)))  //对于只有无 segment的PDU，RCV_CUR_SEQ_NB<CUR_PDU_SEQ_NB,
//	if(((CATTPRecord.RCV_CUR_SEQ_NB < CATTPRecord.CUR_PDU_SEQ_NB)||((G_prePduNoData == 0x01)&&(CATTPRecord.RCV_CUR_SEQ_NB == CATTPRecord.CUR_PDU_SEQ_NB)))&&(CATTPRecord.CUR_PDU_SEQ_NB <= tem))  //对于只有无 segment的PDU，RCV_CUR_SEQ_NB<CUR_PDU_SEQ_NB,
	if(((CATTPRecord.RCV_CUR_SEQ_NB < CATTPRecord.CUR_PDU_SEQ_NB)||((WORD)(CATTPRecord.RCV_CUR_SEQ_NB +1) == CATTPRecord.CUR_PDU_SEQ_NB)||((G_prePduNoData == 0x01)&&(CATTPRecord.RCV_CUR_SEQ_NB == CATTPRecord.CUR_PDU_SEQ_NB)))&&(CATTPRecord.CUR_PDU_SEQ_NB <= tem))  //对于只有无 segment的PDU，RCV_CUR_SEQ_NB<CUR_PDU_SEQ_NB,
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
					CATTPSendRSTPDU(0x04,CATTPRecord.CUR_PDU_ACK_NB +1);  //send RST,reason =04,SEQ = CUR_PDU_ACK_NB+1    FFFF+1  ？
				}
				else
				{
					CATTPSendRST_ACKPDU(0x04);   //send RST ACK,,SEQ = no significant reason =04 ACK = CUR_PDU_SEQ_NB WIN = 0
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
							if( (CATTPRecord.SND_UNA_PDU_SEQ_NB -1+CATTPRecord.SND_WIN_SIZE) <(CATTPRecord.CUR_PDU_ACK_NB+CATTPRecord.CUR_PDU_WIN_SIZE))    //会超过两字节  ？
							{
								CATTPRecord.SND_WIN_SIZE = CATTPRecord.CUR_PDU_WIN_SIZE;
							}
							else
							{
								CATTPRecord.SND_WIN_SIZE = CATTPRecord.SND_UNA_PDU_SEQ_NB -1+CATTPRecord.SND_WIN_SIZE -CATTPRecord.CUR_PDU_ACK_NB;	       //会超过两字节  ？
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
				//	if(CATTPRecord.CUR_PDU_SEQ_NB <= ((DWORD)CATTPRecord.RCV_CUR_SEQ_NB + (DWORD)CATTPRecord.RCV_WIN_SIZE))
					if(CATTPRecord.CUR_PDU_SEQ_NB <= (tem-1))
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
					CATTPRecord.RCV_WIN_SIZE = DEFAULT_WIN_SIZE;
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
					G_LASTSDU_NB = CATTPRecord.CUR_PDU_SEQ_NB;  //保存最后一个PDU的序列号,没有后续数据则是最后一个

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
					PendingMessageAlert(D_CATTPProcessSDU,0x00);
							
					CATTPRecord.status = CATTP_OPEN;	
					
					return 0x02;
				}
				else
				{
					CATTPSendACKPDU(NULL);
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

		cephMoreTimeSingleTelecom();
		
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
// BYTE isDestPortInUse(void)   //该注释代码注意不要删掉，此处可以提示逻辑问题
// {
// 	return FALSE;
// }
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
// 				if(isDestPortInUse())  //默认为FALSE，，所以此处逻辑不会走进来，相关代码删掉
// 				{
// 					CATTPSendRST_ACKPDU(0x02);
// 				}
// 				else
				{
					CATTPSendRST_ACKPDU(0x03);
				}
				
			}
			else  //SYN NOT SET
			{
				CATTPSendRST_ACKPDU(0x04);
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
// 			result = CATTPProcessPduArrivalInClose();	//不会进来？
			break;
		case CATTP_LISTEN:
			break;
		case CATTP_OPEN:
			result = CATTPProcessPduArrivalInOpen();
			break;
		case CATTP_SYN_SENT:
			result= CATTPProcessPduArrivalInSynSent();
			break;
		case CATTP_SYN_RCVD:
			break;
		case CATTP_CLOSE_WAIT:
			break;
		default:
			break;
	}
	

	if(CATTPRecord.status ==  CATTP_CLOSE)	 //对于RST短信，先做setupEvenlist，再关timer，最后close channel
	{
		tempBuf[1] = 0x01;
		
		tempBuf[2] = 0x03;
			
		cephSetUpEvenListSingle(&tempBuf[1]);

		if(result == TimerClose)
		{
			CATTPCloseTimer();	   //数据收全，先关timer			
		}

		//关闭通道 ，关闭定时器，注销事件
		//cephCloseChannelSingle(0x00, CATTPRecord.channelId);
		BIPModuleProcessStatus();
		
		//CATTPClearFile(EF_PDUFILE);  //清空缓存数据
		CATTPClearFile(LDFile_RelaxFile);
		CATTPClearFile(EF_CATTPSave);

		return CMD_Success;
		
	}
			
	if(result != 0x02)	  //丢弃该短信 ，重新开启timer	;若SDU已经收全，则在全部收全时去startTimer
	{
	    CATTPStartTimer(0x00);
	}
	return result;
}
/**
 * @brief 		处理通道状态下行事件函数
 *
 * @Returns   	TRUE:成功 / FALSE:失败
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @090910 15:37
 */
//BYTE ChannelStatus(BYTE pos)		//D60B 99010A 8202 8281  (pos位置) B802 0105
//{

//	if((RecData[pos] & 0x7F) != 0x38)
//	{
//		return FALSE;
//	}
//	if(RecData[pos+1] != 0x02)
//	{
//		return FALSE;
//	}

//	if((RecData[pos+2]&0x80) != 0x00)
//	{
//		return FALSE;      
//	}

//	G_ChannelStatusID  = RecData[pos+2]&0x07;

//	if(G_ChannelStatusID == 0x00)
//	{
//		return FALSE;
//	}
//	
//	PendingMessageAlert(D_ChannelStatus,0x00);

//	return TRUE;
//}
/*
*@description :接受BIP数据
*
*@param       :void
*
*@returns     :
*
*@history     :
*/
BYTE BIPDataAvaiable(void)
{
	UWORD xdata bipAddr;
	WORD xdata offset;
	BYTE idata pos, length;
		
	/*
	bipAddr = SelectTFFile(DF_OTA, 0x6EAF);  //缓存BIP数据		//STKPerform
	
	if(bipAddr == FALSE)
	{
		return FALSE;
	}
	*/
	
	bipAddr = BIPModule_Receive_ADDR_START;						//for test
	
	G_CurrentPDULength = 0x00;
		
	offset = 0x00;
	
	while(TRUE)		   												//此处定义一次接收200bytes，循环接收
	{
		length = (G_CATBIPDataLgth > 0xED) ? 0xED : G_CATBIPDataLgth;

		pos = cephReceiveDataSingle(G_CATBIPChannelId,0x00,NULL,length);					

		if (pos != FALSE)
		{
			if((offset+ RecData[pos]) > 0x600) 						//只能接受1422的BIP数据
			{
				return FALSE;
			}
			
			WriteEeprom(bipAddr+offset,&RecData[pos+1],RecData[pos]);										//for test
			//WriteTFBuf(bipAddr, offset, &RecData[pos+1], RecData[pos]);

			//WriteEeprom(bipSendFileAddr + bipSendFileOff, &tempBuf[0], off);

			offset += RecData[pos];
			
			G_CurrentPDULength += RecData[pos];  //备份当前PDU LENGTH ，需检查PDULEGNTH 是否等于header lgth+data lgth
		}

		pos = FindTag((BYTE *)&RecData[0],TLV_CHANNELDATALENGTH,P3);		//表示还有多少数据未接收？

		if ((pos == FALSE)||(RecData[pos+1] == 0x00))
		{
			break;
		}		
		else
		{
			G_CATBIPDataLgth = RecData[pos+1];
		}
	}

   	if(G_BIPModuleStatus.channelNum  == G_CATBIPChannelId)  //该数据并不是之前打开的通道里的数据
	{					
		return CATTPPduArrivalEvents();
	} 

	return FALSE;
}
/**
 * @brief 		CATTP处理通道数据下行事件函数
 *
 * @Returns   	TRUE:成功 / FALSE:失败
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @080610 15:37
 */
//BYTE CATDataAvailable(BYTE pos)
//{
//	if((RecData[pos] & 0x7F) != 0x38)
//	{
//		return FALSE;
//	}

//	if(RecData[pos+1] != 0x02)
//	{
//		return FALSE;
//	}

//	if((RecData[pos+2]&0x80) != 0x80)
//	{
//		return FALSE;
//	}

//	G_CATBIPChannelId = RecData[pos+2]&0x07;

//	if(G_CATBIPChannelId == 0x00)
//	{
//		return FALSE;
//	}

//	if((RecData[pos+4] & 0x7F) != 0x37)
//	{
//		return FALSE;
//	}

//	if(RecData[pos+5] != 0x01)
//	{
//		return FALSE;
//	}

//	G_CATBIPDataLgth = RecData[pos+6];		//point to data length;

//	G_EventTag = TRUE;

//	PendingMessageAlert(D_CATTPDataAvaiable,0x00);

//	return TRUE;
//}
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
	BYTE idata i,transportType,flag,result;
	BYTE idata identiTagExist;
	UWORD xdata fileAddr;
	BYTE xdata bipport[0x02],cattpPort[0x02],SDUSize[0x02],bufSize[0x02];
	BYTE xdata ipAddr[0x08];
	BYTE xdata netAccess[0x10];
	BYTE xdata identification[0x20];

	identiTagExist = FALSE;						//默认为不存在
	
	fileAddr = SelectLFFile(DF_OTA,EF_CATTP);	  //当前下发了PushCommand，但是CATTP链接已经打开，则不需要执行该命令

	if(G_CATTPOperation == TRUE)
	{
		SetLFConst(fileAddr, 0x01,0x00,0x00,sizeof(BYTE));

    	SetLFConst(fileAddr, 0x02,0x00,0x00,sizeof(BYTE));	 //清除存放BIP信息，CATTP信息的文件

		return;
	}	
	
	ReadLFBuf(fileAddr, 0x01,0x00,&tempBuf[0],0x01);  //获取数据长度
	
	ReadLFBuf(fileAddr, 0x01,0x01,&tempBuf[1],tempBuf[0]);

	netAccess[0] = 0x00;

	ipAddr[0] = 0x00;
		
	i = 1;

	flag = FALSE;
	
	while(i< tempBuf[0])
	{
		switch(tempBuf[i]&0x7F)
		{
			case 0x35:	//bear description tag
			{					
				break;
			}
			case 0x3C:   //interface transport level
			{			
				transportType = tempBuf[i+2];
	
				bipport[0]  = tempBuf[i +3];  
				bipport[1]  = tempBuf[i +4];  

				flag = TRUE;
				
				break;
			}
			case 0x3e:  //other adress ,ipadress
			{
				memcpy(ipAddr,&tempBuf[i+1],tempBuf[i+1]+1);
											
				break;
			}
			case 0x47: //netAcess name
			{
				memcpy(netAccess, &tempBuf[i+1],tempBuf[i+1]+1); //LV 结构
								
				break;
			}
			default:
				break;
		}
		
		i += tempBuf[i+1]+2;            	
	}

	if((ipAddr[0] == 0x00)||(flag == FALSE)) //IP地址或IP端口号不存在，则返回错误
	{
		SetLFConst(fileAddr, 0x01,0x00,0x00,sizeof(BYTE));

    	SetLFConst(fileAddr, 0x02,0x00,0x00,sizeof(BYTE));	 //清除存放BIP信息，CATTP信息的文件

		return;	
	}

	ReadLFBuf(fileAddr, 0x02,0x00,&tempBuf[0],0x01);  //获取数据长度
	
	ReadLFBuf(fileAddr, 0x02,0x01,&tempBuf[1],tempBuf[0]);
	
	identification[0] = 0x00;

//	CATTPClearFile(EF_PDUFILE);
    CATTPClearFile(LDFile_RelaxFile);   //添加清除5E03头，打开bip通道，5E03用来存放pdu,存放前要清除前面的长度字节等。
		
	bufSize[0x00] = 0x02;
	
	bufSize[0x01] = 0x00;  //buffer size
		
	SDUSize[0x00] = 0x04;
	
	SDUSize[0x01] = 0x00;  //默认max sdu length =MAX_SDU_SIZE
	
	i = 1;

	flag = FALSE; // 表示PUSH命令中目的端口号是否存在

	
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

				bufSize[0x00] =tempBuf[i +2];
	
				bufSize[0x01] = tempBuf[i +3];  //buffer size
				
				break;
			}
			case 0x36: //channel data = identification data
			{
				memcpy(identification,&tempBuf[i+1] ,tempBuf[i+1]+1);
				identiTagExist = TRUE;									//表示存在该Tag
				break;
			}
			default:
				break;
		}
		
		i += tempBuf[i+1]+2;
	}

	SetLFConst(fileAddr, 0x01,0x00,0x00,0x01);

  SetLFConst(fileAddr, 0x02,0x00,0x00,0x01);	 //清除存放BIP信息，CATTP信息的文件
  //----------------------------------------开通道，事件列表--------------------------------------------------------------
  if(netAccess[0] == 0x00)
	{
		result = cephOpenChannelSingle(0x01, &ipAddr[0], 0x03,NULL, bufSize, bipport, transportType,NULL);
	}
	else
	{
		result = cephOpenChannelSingle(0x01, &ipAddr[0], 0x03,NULL, bufSize, bipport, transportType,netAccess);
	}

	
	if(GetResponseResult()== CMD_Success)
	{
		G_CATBIPChannelId =  result;
		
		G_BIPModuleStatus.channelNum = result;					//for test
	}
	else
	{
		if(netAccess[0] == 0x00)
		{
			result = cephOpenChannelSingle(0x01, &ipAddr[0], 0x03,NULL, bufSize, bipport, transportType,NULL);
		}
		else
		{
			result = cephOpenChannelSingle(0x01, &ipAddr[0], 0x03,NULL, bufSize, bipport, transportType,netAccess);
		}
		
		if(GetResponseResult() != CMD_Success)
		{
			G_CATBIPChannelId =  FALSE;
			
			return;
		}
			
		G_CATBIPChannelId =  result;
		
		G_BIPModuleStatus.channelNum = result;					//for test
		
	}
	
	tempBuf[0] = 0x03;	

	tempBuf[1] = 0x03;
	
	tempBuf[2] = 0x09;  //Data available;
	
	tempBuf[3] = 0x0A;  //Channel status;
	
	cephSetUpEvenListSingle(&tempBuf[0]);  //Setup EvenList		
   //----------------进行CATTP链接----------------------------------------------------

	clearCatTPRecord();

	G_BIPModuleStatus.channelNum = G_CATBIPChannelId;
	
	G_BIPModuleStatus.channelDataType = ChannelDataType_CATTP;
	
	if(flag == FALSE)  //return  error :Destport not specified 
	{
		CATTPRecord.status = CATTP_CLOSE;
		
		return;
	}
	CATTPRecord.RCV_SDU_SIZE_MAX = (((WORD)SDUSize[0x00]) <<8)&0xFF00 + SDUSize[0x01];
	
	if(CATTPRecord.RCV_SDU_SIZE_MAX > MAX_SDU_SIZE)  //TS102 226有说明，当SDU大小超过卡片所容许的SDU 大小时，使用卡片提供的
	{
		CATTPRecord.RCV_SDU_SIZE_MAX = MAX_SDU_SIZE;
	}
	
	CATTPRecord.desPort = (((WORD)cattpPort[0x00]) <<8)&0xFF00 | cattpPort[0x01];
	
	G_CATTPOperation = TRUE;  //开始CATTP连接操作

	if(identiTagExist == FALSE)			 //表示该Tag不存在时,要添加ICCID (电信备案测试)
	{
		fileAddr =SelectTFFile(DF_MF,EF_ICCID);
	
		identification[0x00] = 0x0A;
	
		ReadTFBuf(fileAddr,0x00,&identification[0x01], 0x0A);  //SYNPDU 默认identification data 
	}
	
	CATTPActiveOpenRequest(identification);	
	//------------------------------------------------------------------------------------------------------

	CATTPStartTimer(0x00);
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
	UWORD xdata i,sum;
	BYTE idata pading;
	
	sum = 0;
	pading =0;
	
	if((dataLen % 2) !=0)
	{
		pading =1;
		dataLen -=1;
	}
	
	for(i=0;i<dataLen;)
	{
      sum += (dataIn[i] << 8) | dataIn[i+1];
      
	  if(sum > 0xFFFF)
	  {
		  sum +=1;
		  
		  sum &= 0xFFFF;
	  }
	  
	  i+=2;
	  
	}
	
	if(pading)
	{
		sum += (dataIn[i] << 8) & 0xff00;
		
		if(sum > 0xFFFF)
		{
			sum +=1;
			
			sum &= 0xFFFF;
		}

	}
	
	sum &= 0xFFFF;
	
	return (WORD)(~sum);
}
/*
@description:发送CATTP 打开请求
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
	
	CATTPRecord.RCV_PDU_SIZE_MAX = MAX_PDU_SIZE;  //Update RCV_PDU_SIZE_MAX,RCV_SDU_SIZE_MAX, from Open request parameters
	
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
*description :关闭timer，即释放timrId,发送主动式命令
*
*@function   :CATTPCloseTimer
*
*@patam      :void
*
*@returns    :void
*
*@author     :create by xuyz
*
*@history    :1.0:Create @20160819 
*/

void CATTPCloseTimer(void)
{	
	TimerModuleReaseTimerId(&G_CattpTimerId);
	
	TimerManagementProactive(Close_Timer,G_CattpTimerId,NULL);	
	
	G_CattpTimerId = 0xFF;  //无效的 timer identifier	
}


/*
*@description:Timer Management 之start timer 
*
*@param      : type=0x00,start timer and more timer;type =0x01 ,start timer only
*
*@returns     :proactive result
*
*@history    :
*/
BYTE CATTPStartTimer(BYTE type)
{
	BYTE xdata timerValue[0x04];
	BYTE idata result;
		
	G_CattpTimerId = TimerModuleAllocTimerId();
	
	if(G_CattpTimerId == 0x00)  //无可使用的timer identifier
	{
		return FALSE;
	}
	
	timerValue[0x00] = 0x03;		   //根据规范定时器时间设置为30s
	timerValue[0x01] = 0x00;
	timerValue[0x02] = 0x00;
	timerValue[0x03] = 0x03;
	
	result = TimerManagementProactive(Start_Timer,G_CattpTimerId,timerValue);
	
	if(type == 0x00)
	{
  	 if(result != CMD_Success)
	   {
		  return result;
	   }
	
	   if(G_EventTag == TRUE)						
	   {
			G_EventTag = FALSE;
			return result;
		}

	   result = cephMoreTimeSingleTelecom();
	}

	return result;
	
}


/*
*@function   :IF_CATTP_RetryCattpsessionByTimerExpiration
*
*@description:上一次的CATTP会话异常终止之后，收到timer expiration之后再开启会话，累计尝试次数为
*
*@See        :   
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0  created by xxj @20180826         
*/
void IF_CATTP_RetryCattpsessionByTimerExpiration(BYTE timerExpirationId)
{
	if (G_EventTimerExpirationId == G_CattpTimerId)	
	{
		tempBuf[0] =ReadVarible(VaribleTag_BackPDU,&tempBuf[1]);

		if(tempBuf[0] != 0x00)
		{
			G_RetryTimers --; 

			if(G_RetryTimers == FALSE)
			{
				TimerModuleReaseTimerId(&G_CattpTimerId);

				tempBuf[1] = 0x01;  //Location status  TS102 223 p8.25

				tempBuf[2] = 0x03;  
					
				cephSetUpEvenListSingle(&tempBuf[1]);
		
				//cephCloseChannelSingle(0x00, G_CATBIPChannelId);    //3次没收到服务器的响应，则关闭通道
				BIPModuleProcessStatus();

				clearCatTPRecord();

				ClearVariable(VaribleTag_BackPDU);

				return;
			}

			cephSendDataSingle(0x01,G_CATBIPChannelId,&tempBuf[0]);

			TimerModuleReaseTimerId(&G_CattpTimerId);;

			CATTPStartTimer(0x00);
		}
	}
}

#endif

