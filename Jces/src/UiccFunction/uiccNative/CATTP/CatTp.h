#ifndef _CATTP_h_
#define _CATTP_h_
#include "typedef.h"

 //********************************	 /
 #define TimerClose  0xFE
 /************************************/
#define  CSIM_ADF_TAR  0x00

#define		D_TmpBufStart			232

#define    TRIPLEDES            0x01
#define    SINGLEDES            0x02

#define     EF_CATTPDuanDian       0x7E01   //4字节，第一字节保存写数据tag ，后续字节保存tar值   
#define 	EF_CATTP               0x7E04  // 记录文件,两条,每条长度为FF ,保存BIP 和CAT-TP数据，初始化为全FF
//#define 	EF_TIMER               0x7E05  //透明文件，初始化为0102030405060708,记录长度为8个字节,存放timer, http业务也需要使用到timer机制，所以该文件移动到LDfile.h中
#define     EF_PDUFILE             0x7E06  //存储当前PDU中的数据字段 ,大小为一条SDU的最大值,初始化为0，存放数据格式UsedSpace(WORD),SEQ1(WORD),LEN1(WORD),DATA1 ,SEQ2(WORD),LEN2(WORD),DATA.....。每次开机或数据处理完毕，该文件清0
#define     EF_CATTPPar            0x7E07  //CATTP 配置文件
#define     EF_EACKSEQ			   0x7E08  //存放OUT OF SEQ，LV结构LBYTE，V  word+word+word ,已按序排列，初始为0
//#define     EF_CATKEY              0x7E09 //存放5组密钥
#define     EF_CATTPSave           0x7E0A  //缓存当前收到的PRL数据totalUsedSpace index1 usedspace data1 index2 usedspace data2...
//#define   	EF_CATTPDownLoadFile   0x7E02  //缓存RFM数据
#define 	EF_CATTPTempFile	   0x7E03 //缓存解密后的RFM数据

//#define		EF_CATTPKeyFile			0x5E01	//存放5组密钥 key文件(线性)


 #define    VaribleTag_BackPDU     0xC2
/**************7E07 Offset********************************/
#define CardCode_Offset          0x00
#define TypeVersion_Offset       0x01
#define IPAddr_Offset            0x02
#define CATTPPort_Offset         0x08
#define TransTypePort_Offset     0x0A
#define BIPPort_Offset            0x0B
#define NetAccessName_Offset      0x0D
#define IdentificationData_Offset 0x14
//***********************************************************************
extern BYTE xdata G_CATTPBackResponse[0x10];

extern BYTE xdata G_EventTag;

//extern BYTE xdata G_BipPrlStatus;

extern BYTE xdata G_CattpTimerId;    //timer identifier管理（01 -08）

extern BYTE xdata G_CATBIPChannelId;

extern BYTE xdata G_CATBIPDataLgth;

extern BYTE xdata G_ChannelStatusID;

extern BYTE xdata G_RefNumber;		//批次号

//extern WORD xdata  G_CurrentCommandOffset;

//extern BYTE xdata  G_PORDeliverSubmit;

extern BYTE xdata G_PushCommand;

//extern BYTE xdata  G_CurrentComNUM;  

//extern BYTE xdata  G_PORTag;

extern WORD xdata G_CurrentPDULength;  //当前一条PDU的长度

extern BYTE xdata G_prePduNoData;  //记录上一个PDU有无数据

extern BYTE xdata G_SDUARRIVAL; 

extern WORD xdata G_LASTSDU_NB;

extern BYTE xdata G_LASTSDU_Flag;  //判断当前是否收到了最后一条SDU;

extern  BYTE xdata G_RetryTimers;  //CATTP链接过程中 每次发送APDU后将，此变量设置为3，如果3次超时，则关闭BIP通道

extern BYTE xdata G_MOMode;

extern BYTE xdata G_CATTPOperation;

extern BYTE xdata G_CATTPSndNextSeqNB;

extern BYTE xdata G_CATTPCurrentTotalNumber;

extern BYTE xdata G_CATTPPreNumber;

extern BYTE xdata G_CATTPTar[0x03];//存放CATTP中当前正在执行PRL更新的TAR值

#define	 	B_meSurportLocationStatus  0
#define 	B_meSurportDataAvailable   1
#define		B_meSurportChannelStatus   2
#define     B_meSurportOpenChannel     3
#define     B_meSurportCloseChannel    4
#define     B_meSurportReceiveData     5
#define     B_meSurportSendData        6
#define     B_meSurportGetChannelStatus   7


#define    POR_OK  			0x00   //etsi 102 225 table 5
#define    CC_Failed 		0x01
#define    CNTR_LOW     	0x02
#define    CNTR_HIGH    	0x03
#define    CNTR_BLOCKED 	0x04
#define    Cipher_Error 	0x05
#define    Securit_Error 	0x06
#define    More_Time        0x08
#define    TAR_Unkown   	0x09

typedef __packed struct
{
	BYTE status;     //OPEN,CLOSED ,LISTEN,CLOSE-WAIT
	//BYTE channelId;		//cattp通道开启时的bip通道号,该值整合到bip全局变量的通道号中：G_BIPModuleStatus.channelNum
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
#define     MAX_SDU_SIZE        0x400
#define 	DEFAULT_WIN_SIZE		0x03  //sdu size/pdu size



#define 	DEFAULT_LOCAL_PORT 		0x0500
#define 	DEFAULT_INI_SEQ_NB 		0x0001


//bit flag of the CAT_TP header
#define CATTP_HEADER_SYN_BIT  0x80
#define CATTP_HEADER_ACK_BIT  0x40
#define CATTP_HEADER_EACK_BIT 0x20
#define CATTP_HEADER_RST_BIT  0x10
#define CATTP_HEADER_NUL_BIT  0x08
#define CATTP_HEADER_SEG_BIT  0x04


#define 	CATTP_TagField_OFFSET  		0x00
#define 	CATTP_HeaderLgth_OFFSET 	0x03
#define     CATTP_SrcPort_OFFSET		0x04
#define     CATTP_DesPort_OFFSET		0x06
#define     CATTP_DataLgth_OFFSET		0x08
#define     CATTP_SEQNumber_OFFSET		0x0A
#define     CATTP_ACKNumber_OFFSET		0x0C
#define     CATTP_WINSIZE_OFFSET		0x0E
#define     CATTP_CHECKSUM_OFFSET		0x10
#define     CATTP_PDUMAX_OFFSET         0x12
#define     CATTP_SDUMAX_OFFSET         0x14

#define    CATTP_DATALGTH_OFFSET        0x12


#define 	MIN_PDU_LENGTH 				0x17

//#define    D_BIPPushOpen    	0xD0
//#define    D_CATPushOpen    	0xD1
#define    D_CATTPProcessSDU	0xD2

#define MODE_KIC     0x00
#define MODE_KID     0x01
//-----------------------function define--------------------
//extern void PRLMenu(void);

extern BYTE BIPPRLSPICheck(BYTE firstSPI,BYTE secondSPI);

extern void CATTPResponseRFMPor(BYTE result);

extern BYTE CATDataAvailable(BYTE pos);

extern void CATTPOpenLink(void);

//extern BYTE CATTPIsRFMReceiveAll(UWORD fileAddr, BYTE packageNum,BYTE tag);

//extern BYTE StoreDataSms(BYTE * bufdata,BYTE refNumber,BYTE seqNumber,BYTE totalNumber,BYTE tag);

//extern BYTE LocalRFMTmpRecord(BYTE packageNum, BYTE index);

//extern void RFMDownLoadExcute(void);

//extern BYTE RFMDecryptANDCheck(BYTE refNumber);

//extern BYTE CATTPExcuteCommand(void);

//extern BYTE ReadDataSms(BYTE * bufData,BYTE refNumber,BYTE seqNumber);

extern BYTE ExcuteCommand(BYTE mode) ;

extern void PushCommand(void);

//extern void BipPushOpen(void);

extern void CATTPOpenRequest(void);

extern WORD CATTPCalCheckSum(BYTE *buf,WORD length);


extern BYTE CATTPStartTimer(BYTE type);

extern void CATTPCloseTimer(void);

extern void CATTPRleaseTimer(void);

extern BYTE cephSetUpEvenListSingle(BYTE *evenList);

extern BYTE BIPDataAvaiable(void);

extern BYTE CATTPPduArrivalEvents(void);

extern BYTE CATTPProcessPduArrivalInSynSent(void);

extern BYTE CATTPProcessPduArrivalInOpen(void);

extern BYTE CATTPProcessPduInSynSent(void);

extern WORD CATTPCalCheckSumAddr(UWORD fileAddr,WORD dataLen);

extern BYTE CATTPSendACKPDU(BYTE *dataBuf);

extern void CATTPSendCOMMOMPDU(void);

//extern BYTE CATTPSendMoRequest(void);

extern BYTE CATTPSendRequestInOpenState(BYTE *dataBuf);

extern BYTE CATTPProcessPduInOpen(void);

extern BYTE CATTPPduValid(void);

extern void CATTPUpdateListOutOfSequence(WORD currentSeq);

extern BYTE CATTPSendEACKPDU(void);

extern BYTE CATTPSendRSTPDU(BYTE reason,WORD seq);

extern BYTE CATTPSendRST_ACKPDU(BYTE reason);

extern void CATTPRemoveSequenceNumber(void);

//extern BYTE  CATTPProcessPduArrivalInClose(void);

extern BYTE isDestPortInUse(void);

extern BYTE CATTPProcessSDUData(void);

//extern void DesMacFunction(UWORD eepromAddr,BYTE * cpKey,BYTE cpResult[8],WORD nEnterLen,BYTE mode);

extern void Timeout_Events_Process(void);

//extern void CATTPOpen(void);

extern void clearCatTPRecord(void);

extern void CATTPActiveOpenRequest(BYTE *identification) ;

extern void TimerExpiration(void);

//extern BYTE CATGetRFMKey(BYTE index ,BYTE *key,BYTE mode );

extern BYTE ChannelStatus(BYTE pos);

//extern BYTE CATTPChipheRe(BYTE kic,BYTE * pointer,BYTE * result_pointer,BYTE mode,WORD lgth);

extern void CATTPClearFile(WORD fileID);

extern void ResumePrlData(void);

//extern BYTE CATTPExcuteResumePRLCommand(void);

//----------------------------------------------------------

extern BYTE code CATTPFileList[];

extern BYTE code EPRLSuccess[];

extern BYTE code PRLSuccess[];

extern void IF_CATTP_RetryCattpsessionByTimerExpiration(BYTE timerExpirationId);
#endif
