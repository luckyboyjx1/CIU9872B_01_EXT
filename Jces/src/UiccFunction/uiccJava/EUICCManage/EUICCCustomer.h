#ifndef    _EUICCCUSTOMER_H   
#define	   _EUICCCUSTOMER_H 
#include    "typedef.h"
#include "uicc_main.h"

/**********************macro define*************************************/
#define RSPSessionState_NULL                  0x00
#define RSPSessionState_Init                  0x01   //该状态为执行完 GetEUICCChallenge 后的会话状态
#define RSPSessionState_Auth                  0x02   //该状态为执行完 AuthenticateServer 后的会话状态
#define RSPSessionState_Prepare               0x03   //该状态为执行完 PrepareDownload 后的会话状态
#define RSPSessionState_InitiaSecureChannel   0x04   //该状态为执行完 InitialiseSecureChannel 后的会话状态
#define RSPSessionState_ConfigureISDP         0x05   //该状态为执行完 ConfigureISDP 后的会话状态
#define RSPSessionState_PrepareStoreMetadata  0x06   //该状态为 准备storedata的状态
#define RSPSessionState_StoreMetadata         0x07   //该状态为执行完 StoreMetadata 后的会话状态
#define RSPSessionState_ReplaceSessionKeys    0x08   //该状态为执行完  ReplaceSessionKeys 后的状态
#define RSPSessionState_PreparePEDownload     0x09    //该状态为收到 PE 的tag和lenth时的状态
#define RSPSessionState_LoadProfileElements   0x0A   //该状态为执行 profile安装时的状态

#define ServerAddressMaxLen                   0x43	 //ServerAddress的最大长度，实网测试实际来的地址长度不定，所以用宏定义, 由于在RSPSessionContext结构体中被使用，所以其值+1要求是4的整数倍
#define TransactionIDMaxLen                   0x10   //transactionId的最大长度, TransactionId ::= OCTET STRING (SIZE(1..16))

#define ProfileNameMaxLen					  0x100  //profileName的最大长度, profileName [18] UTF8String (SIZE(0..64))
#define ServiceProviderNameMaxLen			  0x80   //serviceProviderName的最大长度, serviceProviderName [17] UTF8String (SIZE(0..32))
#define NotificationAddressMaxLen			  102    //拼装通知时，有ICCID并且seqNumber最大的情况下留给notificationAddress的最大长度
#define IconMaxLen							  1024	 //icon的最大长度


typedef __packed struct
{
  UWORD remainSequence;  //登记该序列的剩余长度,主要用于profile下载过程中登记序列的剩余总长度 ,如 storeMedata 序列 profilePE 序列
  BYTE sessionSate;
  BYTE errorResult;            //保存当前profile下载过程中出现错误时的ErrorReason in ErrorResult，初始化0, 目前的实现该值只对LoadPE过程有效。其他的过程不需要更改
  BYTE downloadProfileIndex;   //保存当前下载的profile的index
  BYTE transactionIdLen;
  BYTE channllengeTransactionId[TransactionIDMaxLen];  
  BYTE serverAddress[ServerAddressMaxLen+1];  //LV 结构，用于存储在AuthenticateServer 指令中的 serverAddress， 在 拼装 installResult中使用，暂时缓存在RAM 中
  BYTE encKey[0x10];
  BYTE macKey[0x10];
  BYTE initMacValue[0x10];
  BYTE icvCounter[0x10];
  WORD clearDataLen;     //存储当前明文数据长度，主要用于storeMetaData中节约E2使用
  BYTE channelNum;       //当前会话所有的逻辑通道号
}RSPSessionContext;

extern RSPSessionContext xdata G_RSPSessionContext;

typedef __packed struct
{
  BYTE blockNum;       //多条时的blockNum
  WORD storeDataLen ;  //当前已收到的命令长度
	
}CustomerStoreData;

extern CustomerStoreData xdata G_CustomerStoreDataApdu;

/*****************struct info*************************************************/
/*
typedef __packed struct
{
	WORD nextPPARRef;   
	BYTE currentPPARSize;
    BYTE PPARValue[];    // 格式为30LV，eg: 30 1D 80020640(ppr) A1133011800392F928810447534D418204525350FF(operatorID) 82020780(UserConsent)
}RATTableList;     //该结构体不会在代码中使用，只会起一个结构参考作用.预置RAT时是按照实际大小分配空间的    
*/

//#define RATTableList_nextPPARRef_Offset      0x00
//#define RATTableList_currentPPARSize_Offset  0x02
//#define RATTableList_PPARValue_Offset        0x03

//#define RATTableListStruct_SIZE              RATTableList_PPARValue_Offset   // RATTableList中除了PPARValue之外的字节数
/*
typedef __packed struct
{
	WORD nextNotificationRef;
	UWORD seqNumber;
	BYTE profileManagementOperation;
	BYTE notificationType;               // 记录当前条通知的类型是final result还是other
	WORD euiccEumRef;                    // 只有通知类型为OtherSignedNotification时该字段才有效
	WORD notificationDataSize;
}NotificationListHead;
*/
/*
typedef __packed struct
{
	WORD nextNotificationRef; 
	UWORD seqNumber;
	BYTE profileManagementOperation;
    BYTE notificationType; 
	WORD notificationDataSize;     //通知数据所占的实际空间
	BYTE notificationData[];    如果是ProfileInstallationResult，数据存储格式为BF37 L V; 如果是OtherSignedNotification，数据存储格式为30 L V
}NotificationList;   //该结构体不会在代码中使用，只会起一个结构参考作用
*/

//#define NotificationList_nextNotificationRef_Offset            0x00
//#define NotificationList_seqNumber_Offset                      0x02
//#define NotificationList_profileManagementOperation_Offset     0x06
//#define NotificationList_notificationType_Offset               0x07
//#define NotificationList_euiccEumRef_Offset                    0x08
//#define NotificationList_notificationDataSize_Offset           0x0A            //这个地方的写入和读取需要注意大小端问题
//#define NotificationList_notificationData_Offset               0x0C

//#define NotificationListStruct_SIZE                            sizeof(NotificationListHead)//不包含BYTE notificationData[]字段长度
#define NotificationMaxSIZE                                    0x110           //每次分配通知时，都按这个最大值来分配，包含了NotificationListHead在内

#define  GetNotificationListItemAddr                     GetObjectItemAddr
#define  GetNotificationList_ArrayItemAddr               GetObject_ArrayItemAddr

#define  GetNotificationList_nextNotificationRefAddr(NotificationAddr)				GetNotificationListItemAddr(NotificationAddr,nextNotificationRef_INDEX)
#define  GetNotificationList_seqNumberAddr(NotificationAddr)						GetNotificationListItemAddr(NotificationAddr,seqNumber_INDEX)
#define  GetNotificationList_profileManagementOperationAddr(NotificationAddr)		GetNotificationListItemAddr(NotificationAddr,profileManagementOperation_INDEX)
#define  GetNotificationList_notificationTypeAddr(NotificationAddr)					GetNotificationListItemAddr(NotificationAddr,notificationType_INDEX)
#define  GetNotificationList_euiccEumRefAddr(NotificationAddr)						GetNotificationListItemAddr(NotificationAddr,euiccEumRef_INDEX)
#define  GetNotificationList_notificationDataSizeAddr(NotificationAddr)				GetNotificationListItemAddr(NotificationAddr,notificationDataSize_INDEX)
#define  GetNotificationList_notificationDataAddr(NotificationAddr)					GetNotificationListItemAddr(NotificationAddr,notificationData_INDEX)

#define  GetNotificationList_notificationData_ArrayAddr(NotificationAddr)			GetNotificationList_ArrayItemAddr(NotificationAddr,notificationData_INDEX)
/*
NotificationEvent ::= BIT STRING { 
  notificationInstall(0), 
  notificationEnable(1), 
  notificationDisable(2), 
  notificationDelete(3) 
}
*/
#define NOTIFICATION_EVENT_Install                              0x80
#define NOTIFICATION_EVENT_ENABLE                               0x40
#define NOTIFICATION_EVENT_DISABLE                              0x20
#define NOTIFICATION_EVENT_DELETE                               0x10

#define NOTIFICATION_TYPE_FINALRESULT                           0x01
#define NOTIFICATION_TYPE_OTHER                                 0x02

#define TAG_INITIALMAC                                          0x80
#define TAG_PPKENC                                              0x81
#define TAG_PPKCMAC                                             0x82

#define TAG_ICCID                                               0x5A
#define TAG_SERTVICEPROVIDERNAME                                0x91
#define TAG_PROFILENAME                                         0x92
#define TAG_ICONTYPE                                            0x93
#define TAG_ICON                                                0x94
#define TAG_PROFILECLASS                                        0x95
#define TAG_NOTIIFICATIONCONFIG                                 0xB6
#define TAG_PROFILEOWNER                                        0xB7
#define TAG_PROFILEPOLICYRULES                                  0x99
#if RSP_SGP22_BF22
#define TAG_SERVICESPECIFICDATASTOREDINEUICC                    0x22
#define TAG_SERVICESPECIFICDATANOTSTOREDINEUICC                 0x23
#endif
 
/***********************************SendFileExtendBackData struct define *********************************************/
typedef __packed struct
{
	UWORD backAddr; 	    //备份当前需要发送的第一部分数据的起始地址，这个地址有可能是EuiccTempBuf,有可能是某条通知的数据域起始地址
	WORD  euiccCertRef;     //当前需要发送的数据中包含的EUICC证书的地址的引用，如果不包含证书 为REF_NULL
	WORD  eumCertRef;       //当前需要发送的数据中包含的eum证书的地址的引用，如果不包含证书 为REF_NULL
	WORD  backOffset;	    //指向当前正在发送的数据的偏移，如果是证书 表示的是相对于证书内容的偏移（证书存储结构：L（2字节总长度）+证书TLV）
	WORD  len;		        //当前正在发送的数据的剩余长度
	UWORD  totalLen;			//剩余待发送数据的总长度//modify by xieyong
	BYTE  curretnSendStep;  //0表示正在发送第一部分数据（backAddr） 1表示正在发送eumCertAddr，2表示正在发送euiccCertAddr中的数据  
	BYTE  currentChannel;   //当前需要取数据的通道，这个变量主要用于在接收到一条新的指令时 判断要不要清空备份信息，如果备份数据的通道来了一条非C0指令，其取响应过程被打断
	BYTE  notifiyNum;		//表示当前需要发送的通知的个数
	//WORD  notifiy[NOTIFICATION_MAX_NUMBER];  //表示当前需要发送的通知的引用
	WORD  notifiy[NOTIFICATION_MAX_NUMBER];  //表示当前需要发送的通知的引用
}SendFileExtendBackData; 

extern SendFileExtendBackData xdata G_SendFileExtendBackDataStruct;

#define SendStep_BackAddr              0 //从backAddr发数据
#define SendStep_EUICCCert             1 //从EUICC证书中发数据
#define SendStep_EUMCert               2 //从EUM证书中发数据
//#define SendStep_Notify                3 //从通知中发数据，如果通知没有发送完 会将通知地址写到SendStep_BackAddr中的备份地址


/**********************function define**********************************/
extern void CUSTOMER_SendSingleBYTEResult(WORD ResponseTag, BYTE result);

extern BYTE GetEuiccInfo_ExtCardResource(BYTE * outBuf);
extern void CUSTOMER_SendSingleBYEResult(WORD ResponseTag, BYTE result);



extern void CUSTOMER_GetEuiccConfiguredAddresses(void);
extern void CUSTOMER_SetDefaultDpAddress(void);
extern void CUSTOMER_GetEUICCInfo(void);
extern void CUSTOMER_ListNotification(void);
extern void CUSTOMER_RetrieveNotificationsList(void);
extern void CUSTOMER_RemoveNotificationFromList(void);
extern void CUSTOMER_LoadCRL(void);
extern void CUSTOMER_GetProfilesInfo(void);
extern void CUSTOMER_EnableProfile(void);
extern void CUSTOMER_DisableProfile(void);
extern void CUSTOMER_DeleteProfile(void);
extern void CUSTOMER_eUICCMemoryReset(void);
extern void CUSTOMER_GetEID(void);
extern void CUSTOMER_SetNickname(void);


extern void CUSTOMER_GetEUICCChallenge(void);
extern void CUSTOMER_AuthenticateServer(void);
extern void CUSTOMER_PrepareDownload(void);
extern void CUSTOMER_CancelSession(void);
extern void CUSTOMER_InitialiseSecureChannel(void);
extern void CUSTOMER_ConfigureISDP(void);
extern void CUSTOMER_StoreMetadata(void);
extern void CUSTOMER_ReplaceSessionKeys(void);
extern void CUSTOMER_LoadCRL(void);

extern void CUSTOMER_PrepareStoreMetadata(void);

extern void CUSTOMER_PrepareLoadPE(void);
extern void CUSTOMER_LoadProfilePE(void);
extern void CUSTOMER_ReceiveErrorAPDU(void);
extern BYTE CUSTOMER_ReceiveStoreData(void);


extern void NM_CUSTOMERISDRStoreData(void);
extern void NM_GetISDRSelectOtherResponse(void);
extern void NM_ISDRInstallISDP(void);

extern void CUSTOMER_UpdateMetadata(void);
extern void NM_CUSTOMERISDR_GetResponse(void);
extern void ISDR_GetResponse(void);
extern void CUSTOMER_Getsessionkey(void);
extern void CUSTOMER_GetProfileInfo(void);


//extern BYTE IsUiccApp(BYTE contextId);
#endif
