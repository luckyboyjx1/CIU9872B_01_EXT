#ifndef _CATTP_h_
#define _CATTP_h_
#include "typedef.h"

//********************************	 /
#define TimerClose  0xFE
//////////////文件系统//////////////

#define   VaribleTag_BackPDU          0xC2 //当数据太长时，分条缓存
//#define   VaribleTag_BackPDUData2     0xC3
//#define   VaribleTag_BackPDUData3     0xC4
#define   CATTPPOR          0xC5 //CATTP级联POR时候打标志

extern WORD xdata G_CATTPSndSEQ;  //当前发送的PDU的SEQ 

extern BYTE xdata G_CATTP_TimerIdentifier;    //timer identifier管理（01 -08）

extern WORD xdata G_CurrentSDUForSendLength;        //当前要发送的SDU的长度

extern BYTE xdata G_PDUSize[2];

/**************7E07 Offset********************************/
//#define CardCode_Offset          0x00
//#define TypeVersion_Offset       0x01
//#define IPAddr_Offset            0x02
//#define CATTPPort_Offset         0x08
//#define TransTypePort_Offset     0x0A
//#define BIPPort_Offset            0x0B
//#define NetAccessName_Offset      0x0D
//#define IdentificationData_Offset 0x14
//***********************************************************************
//define the state fo cat_tp
#define 	CATTP_CLOSE     	0x00
#define 	CATTP_LISTEN     	0x01
#define 	CATTP_OPEN       	0x02
#define	 	CATTP_SYN_SENT   	0x03
#define 	CATTP_SYN_RCVD   	0x04
#define 	CATTP_CLOSE_WAIT 	0x05

#define     CATTP_ACTIVE        0x01
#define     CATTP_PASSIVE       0x00

#define     CATTP_VERSION       0x00  //当前仅支持版本号00

#define     MAX_PDU_SIZE        0x200 //max PDU length在open channel主动式命令中通过terminal response返回
#define     MAX_SDU_SIZE        0x500 //电信研究院collis测试时，scenario_11_CATTP_30_ab_77的			Secure packet (SDU) length: 04 45
#define     DEFAULT_WIN_SIZE    0x03  //sdu size/pdu size



#define 	DEFAULT_LOCAL_PORT 		0x0500
#define 	DEFAULT_INI_SEQ_NB 		0x0001


//bit flag of the CAT_TP header
#define CATTP_HEADER_SYN_BIT  0x80
#define CATTP_HEADER_ACK_BIT  0x40
#define CATTP_HEADER_EACK_BIT 0x20
#define CATTP_HEADER_RST_BIT  0x10
#define CATTP_HEADER_NUL_BIT  0x08
#define CATTP_HEADER_SEG_BIT  0x04

#define 	CATTP_Len  		            0x02
//0x6EAF (EF_BIPData) offset
#define 	CATTP_TagField_OFFSET  		0x00
#define 	CATTP_HeaderLgth_OFFSET 	0x03
#define 	CATTP_SrcPort_OFFSET		0x04
#define 	CATTP_DesPort_OFFSET		0x06
#define 	CATTP_DataLgth_OFFSET		0x08
#define 	CATTP_SEQNumber_OFFSET		0x0A
#define 	CATTP_ACKNumber_OFFSET		0x0C
#define 	CATTP_WINSIZE_OFFSET		0x0E
#define 	CATTP_CHECKSUM_OFFSET		0x10
#define 	CATTP_PDUMAX_OFFSET         0x12
#define 	CATTP_SDUMAX_OFFSET         0x14

#define   CATTP_DATALGTH_OFFSET        0x12


#define 	MIN_PDU_LENGTH 				0x17

#define    D_CATTPProcessSDU	0xD2

#define 	ISDR_MODE 				0x00

#define   SD_MODE						0x01

extern BYTE xdata G_PushCommand;

extern WORD xdata G_CurrentPDULength;  //当前一条PDU的长度

extern BYTE xdata G_prePduNoData;  //记录上一个PDU有无数据


extern WORD xdata G_LASTSDU_NB;

extern BYTE xdata G_LASTSDU_Flag;  //判断当前是否收到了最后一条SDU;

extern BYTE xdata G_RetryTimers;  //CATTP链接过程中 每次发送APDU后将，此变量设置为3，如果3次超时，则关闭BIP通道

extern BYTE xdata G_CATTPOperation;

extern BYTE xdata G_CATTPSndNextSeqNB;

extern BYTE xdata G_CATTPType;

typedef __packed struct
{
	BYTE status;     //OPEN,CLOSED ,LISTEN,CLOSE-WAIT
	BYTE channelId;
	BYTE isActive;   //active request or passive request
	WORD desPort;
	WORD srcPort;
	
	
	WORD SND_INI_SEQ_NB;  //Variables for CAT_TP sending activity
	WORD SND_NXT_SEQ_NB;
	WORD SND_UNA_PDU_SEQ_NB;
	WORD SND_PDU_SIZE_MAX;
	WORD SND_SDU_SIZE_MAX;
	WORD SND_WIN_SIZE;
	
	WORD RCV_PDU_SIZE_MAX; //Variables for CAT_TP receiving activity
	WORD RCV_SDU_SIZE_MAX;
	WORD RCV_CUR_SEQ_NB;
	WORD RCV_INI_SEQ_NB;
	WORD RCV_WIN_SIZE;
	
	WORD SYN_PDU_SIZE_MAX;  //SUN PDU
	WORD SYN_SDU_SIZE_MAX;
		
	BYTE CUR_PDU_FLAG;   //Variables from current PDU
	BYTE CUR_PDU_VRS_NB;
	BYTE CUR_PDU_HEADER_LENGTH;
	WORD CUR_PDU_DATA_LENGTH;
	WORD CUR_PDU_SEQ_NB;
	WORD CUR_PDU_ACK_NB;
	WORD CUR_PDU_WIN_SIZE;
	WORD CUR_PDU_CHECKSUM;
} ConnectionRecord; 

extern ConnectionRecord xdata CATTPRecord;

//-----------------------function define--------------------

extern void CATTPSavePushPara(BYTE extendType);

extern void CATTPPushPOR(BYTE cattpStatus);

extern void CATTPOpenLink(void);

extern void PushCommand(void);

extern WORD CATTPCalCheckSum(BYTE *buf,WORD length);

extern BYTE CATTPStartTimer(void);

extern BYTE CATTPStartTimerOnly(void);

extern BYTE CATTPCloseTimer(void);

extern void CATTPRleaseTimer(void);

extern BYTE cephSetUpEvenListSingle(BYTE *evenList);

extern BYTE CATTPPduArrivalEvents(void);

extern BYTE CATTPProcessPduArrivalInSynSent(void);

extern BYTE CATTPProcessPduArrivalInOpen(void);

extern BYTE CATTPProcessPduInSynSent(void);

extern WORD CATTPCalCheckSumAddr(UWORD fileAddr,WORD dataLen,BYTE DataOffset);

extern void CATTPModuleInit(void);

extern BYTE CATTPSendACKPDU(BYTE *dataBuf);

extern BYTE CATTPSendRequestInOpenState(BYTE *dataBuf);

extern void CATTPHeaderPDU(BYTE *dataBuf);

extern void IF_SendPDU(void);

extern BYTE IF_CATTPcephMoreTime(void);

extern BYTE CATTPPduValid(void);

extern void CATTPUpdateListOutOfSequence(WORD currentSeq);

extern BYTE CATTPSendEACKPDU(void);

extern BYTE CATTPSendRSTPDU(BYTE reason,WORD seq);

extern BYTE CATTPSendRST_ACKPDU(BYTE reason);

extern void CATTPRemoveSequenceNumber(void);

extern BYTE  CATTPProcessPduArrivalInClose(void);

extern BYTE isDestPortInUse(void);

extern BYTE CATTPProcessSDUData(void);

extern void Timeout_Events_Process(void);

extern void clearCatTPRecord(void);

extern void clearCatTPVariable(void);

extern void CATTPActiveOpenRequest(BYTE *identification) ;

extern void TimerExpiration(void);

extern void CATTPClearFile(WORD fileID);

extern void  ClearCATTPBuf(void);

extern void CATTPSendRetry(void);

extern void CATTPSendMoreData(UWORD PorAddr,WORD totalLen, BYTE channelId);

extern void CATTPModuleInit(void);

extern void InitCattpVariable(void);

extern BYTE IF_SendPor(void);
//----------------------------------------------------------

#endif
