#ifndef    _DNS_H_
#define    _DNS_H_
#include   "typedef.h"
#include "ecpDefine.h"
/*********************macro define*****************************/

#define UICC_STOP                            0x01
#define RETRY_POLICY                         0x02
#define OK_Empty_RAS_IP_List_Cache           0x03
#define OK_Non_Empty_RAS_IP_List_Cache       0x04
#define OK_WITH_DNS_IP_List_Cache_Empty      0x05
#define OK_WITH_DNS_IP_List_Cache_Non_Empty  0x06
#define NOK_WITH_No_Suitable_Answer          0x07
#define DNS_QUERIES                          0x08

/*********************fetchingDNSIPList的模式定义*****************************/
#define MODE_INIT                            0x01
#define MODE_Timer_Expiration                0x02

/*********************Security Domain DNS Resolution Parameters*****************************/
#define D0_OFFSET_DNS_D0_D3_buffer           0x00 // D0LV, 3 bytes, Force DNS Resolution
#define D1_OFFSET_DNS_D0_D3_buffer           0x03 // D1LV, 9 bytes, DNS-From-ME Retry Policy Parameters
#define D2_OFFSET_DNS_D0_D3_buffer           0x0C // D2LV, 9 bytes, DNS IP Retry Policy Parameters
#define D3_OFFSET_DNS_D0_D3_buffer           0x15 // D3LV, 9 bytes, RAS IP Retry Policy Parameters
#define LENGTH_OF_DNS_D0_D3_buffer           0x1E

#define TAG_D0_IN_DNS_Resolution_Parameters  0xD0
#define TAG_D1_IN_DNS_Resolution_Parameters  0xD1
#define TAG_D2_IN_DNS_Resolution_Parameters  0xD2
#define TAG_D3_IN_DNS_Resolution_Parameters  0xD3
#define TAG_D6_IN_DNS_Resolution_Parameters  0xD6
#define TAG_D8_IN_DNS_Resolution_Parameters  0xD8
#define TAG_F7_IN_DNS_Resolution_Parameters  0xF7
#define TAG_F9_IN_DNS_Resolution_Parameters  0xF9
#define TAG_FA_IN_DNS_Resolution_Parameters  0xFA
// Gemalto DNS Resolver v1.1.pdf 里面的tag，用于表示RAS IP所占权重，即优先级
#define TAG_D8_IN_DNS_Resolution_Parameters  0xD8


#define DNS_TriggerPara_Fully_Qualified_Domain_Name 0xD6
#define DNS_TriggerPara_Connection_Parameters  		0xFA

#define DNS_PARA_D1_D2_D3_PER_TLV_LENGTH           0x09		//DNS Resolution Parameters 中的D1,D2,D3都是TLV总长度 0x09


/*****************global variable define************************/
typedef __packed struct dnsByMEConnectStatus
{
	UWORD ipAddress;   //当前建立连接的DNS IP Address
	WORD tryCounter;
	BYTE timerId;		  //DNS重试时分配的timerid
	BYTE resume;	      //该值为1标识当前是重试操作,初始值为0
} DNSByMEConnectStatus;

extern DNSByMEConnectStatus xdata G_DNSByMEConnectStatus;

typedef __packed struct dnsIPConnectStatus
{
	UWORD ipAddress;   //当前建立连接的DNS IP Address
	WORD tryCounter;
	BYTE timerId;		  //DNS重试时分配的timerid；初始值为0x00 (timer的正常范围是1-8)
	BYTE resume;	      //该值为1标识当前是重试操作,初始值为0
} DNSIPConnectStatus;

extern DNSIPConnectStatus xdata G_DNSIPConnectStatus;

typedef __packed struct rasIPConnectStatus
{
	UWORD ipAddress;   //当前建立连接的RAS IP Address
	WORD tryCounter;
	BYTE timerId;		  //RAS重试时分配的timerid；初始值为0x00 (timer的正常范围是1-8)
	BYTE resume;	      //该值为1标识当前是重试操作,初始值为0
} RASIPConnectStatus;

extern RASIPConnectStatus xdata G_RASIPConnectStatus;

#define DNSResolutionProcedureState_NULL                               0x00
#define DNSResolutionProcedureState_DNS1                               0x01
//#define DNSResolutionProcedureState_DNS2                               0x02
#define DNSResolutionProcedureState_CONNECT_DNS_IP                     0x03
#define DNSResolutionProcedureState_Fetch_DNS_IP_List_from_ME          0x04
#define DNSResolutionProcedureState_CONNECT_RAS_IP                     0x05
#define DNSResolutionProcedureState_ESTABLISH_TLS                      0x06
#define DNSResolutionProcedureState_HTTP_EXCHANGE                      0x07
#define DNSResolutionProcedureState_END                                0x08
#define DNSResolutionProcedureState_DNS_QUERIES                        0x09

typedef __packed struct dnsResolution
{
	BYTE curState;  // 表示当前在哪个步骤下执行DNS / RAS / TLS / HTTP Exchange的哪个状态下
    BYTE curResult; // 表示当前步骤执行的结果
    BYTE required;
    BYTE performed;
}DNSResolutionProcedureState;
extern DNSResolutionProcedureState xdata G_DNSResolutionProcedureState;


/*****************function define**********************************/
extern void DNSResolutionInitVariable(void);
extern void DNSByMEInitVariable(void);
extern void DNSInitVariable(void);
extern void RASInitVariable(void);
extern void AdministrationSessionInitVariable(void);
extern WORD InitializeDNSIPTryCounter(void);
extern WORD InitializeDNSFromMETryCounter(void);
extern WORD InitializeRASIPTryCounter(void);
extern void DNSResolution_Main(void);
extern BYTE findRASIPAddressInConnectionPara(void);
extern BYTE GetOpenChannelParaForDNS(BYTE *para);
extern BYTE GetOpenChannelParaForRAS(BYTE *para);
extern void DNSConnect_StartTimer(void);
extern void DNS_RetryDNSSessionByTimerExpiration(BYTE timerExpirationId);
extern void RASConnect_StartTimer(void);
extern void RAS_RetryHttpSessionByTimerExpiration(BYTE timerExpirationId);
extern void DNSByME_StartTimer(void);
extern void DNSByME_RetryFetchingDNSIPListSessionByTimerExpiration(BYTE timerExpirationId);
extern u8 DNSResolution_ConnectDNS(void);
extern BYTE getValueofForceDNSResolution(void);
extern WORD InitializeSessionTryCounter(void);
extern BYTE isIPCorrect(BYTE *dataBuffer, WORD subTagLen);
extern BYTE DNSResolutionParaCheck(BYTE dnsTag);
extern BYTE GPStoreSDDNSResolutionParaSub(PN8 sdDataObj, BYTE tag, BYTE * dataBuffer);
extern void GPStoreSDDNSResolutionPara(PN8 sdDataObj, BYTE * dataBuffer);
extern WORD GPGetSDDNSResolutioParaF4(PN8 sdDataObj, BYTE * dataBuffer);
extern WORD GPGetSDDNSResolutioParaF5(PN8 sdDataObj, BYTE * dataBuffer);
extern WORD GPGetSDDNSResolutionPara(PN8 sdDataObj, WORD tag, BYTE * dataBuffer);
#endif

