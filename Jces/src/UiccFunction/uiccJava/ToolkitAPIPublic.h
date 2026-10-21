#ifndef _TOOLKITAPIPUBLIC_H_
#define _TOOLKITAPIPUBLIC_H_
#include "typedef.h"
#include "uicc_main.h"

#if UICC__FUNCTION__UICCAPI
#include "USATEnvelopeHandler.h"
#include "UICCToolkitApi.h"
#endif

#if UICC__FUNCTION__SIMAPI
#include "SimApi.h"
#endif

#define  UCS2       		0x80
#define  GSMCOMPRESSED 		0x00
#define  GSMDEFAULT			0x04

//**************************Poll Interval Constants ********************************
#define POLL_NO_DURATION                                        0
#define POLL_SYSTEM_DURATION                                    -1

#define     MAXVALUE_POLL_DURATION             (jshort)15300

extern WORD FindMinDuration(BYTE curretNum);

/***************EVENT DOWNLOAD***********************************************/

#define   MT_call              							0x00 
#define   Call_connected       							0x01
#define   Call_disconnected	   							0x02
#define   Location_status	   							0x03
#define   User_activity        							0x04
#define   Idle_screen_available							0x05
#define   Card_reader_status        					0x06
#define   Language_selection	    					0x07
#define   Browser_Termination							0x08
#define   Data_available            					0x09
#define   Channel_status	        					0x0A
#define   Access_Technology_Change_single               0x0B 
#define   Display_parameters_changed                    0x0C 
#define   Local_connection                              0x0D 
#define   Network_Search_Mode_Change                    0x0E 
#define   Browsing_status                               0x0F 
#define   Frames_Information_Change                     0x10 
//#define   reserved for 3GPP (I-WLAN Access Status)     0x11 
//#define   reserved for 3GPP (Network Rejection)        0x12 
#define   Network_Rejection                             0x12
#define   HCI_connectivity_event                        0x13 
#define   Access_Technology_Change_multiple             0x14 
//#define   reserved for 3GPP (CSG cell selection)       0x15 
#define   Contactless_state_request                     0x16 

//******************************** Events Constants for Java API ts102241********************************
#define EVENT_PROFILE_DOWNLOAD                                  1
//2
//3                                                     
//4
//5
//6
#define EVENT_MENU_SELECTION                                    7
#define EVENT_MENU_SELECTION_HELP_REQUEST                       8
#define EVENT_CALL_CONTROL_BY_NAA                               9
//10
#define EVENT_TIMER_EXPIRATION                                  11
#define EVENT_EVENT_DOWNLOAD_MT_CALL                            12
#define EVENT_EVENT_DOWNLOAD_CALL_CONNECTED                     13
#define EVENT_EVENT_DOWNLOAD_CALL_DISCONNECTED                  14
#define EVENT_EVENT_DOWNLOAD_LOCATION_STATUS                    15
#define EVENT_EVENT_DOWNLOAD_USER_ACTIVITY                      16
#define EVENT_EVENT_DOWNLOAD_IDLE_SCREEN_AVAILABLE              17
#define EVENT_EVENT_DOWNLOAD_CARD_READER_STATUS                 18
#define EVENT_STATUS_COMMAND                                    19
#define EVENT_EVENT_DOWNLOAD_LANGUAGE_SELECTION                 20
#define EVENT_EVENT_DOWNLOAD_BROWSER_TERMINATION                21
#define EVENT_EVENT_DOWNLOAD_DATA_AVAILABLE                     22
#define EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS                     23
//24
#define EVENT_EVENT_DOWNLOAD_ACCESS_TECHNOLOGY_CHANGE           25
#define EVENT_EVENT_DOWNLOAD_DISPLAY_PARAMETER_CHANGED          26
#define EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION                   27
#define EVENT_EVENT_DOWNLOAD_NETWORK_SEARCH_MODE_CHANGE         28
#define EVENT_EVENT_DOWNLOAD_BROWSING_STATUS                    29   
// event number 30 and 31 are reserved for 3GPP
#define EVENT_EVENT_DOWNLOAD_HCI_CONNECTIVITY                   32
// event number 33 is reserved for 3GPP
#define EVENT_EVENT_DOWNLOAD_FRAMES_INFORMATION_CHANGED         34
#define EVENT_EVENT_DOWNLOAD_CONTACTLESS_STATE_REQUEST          35
#define EVENT_PROACTIVE_HANDLER_AVAILABLE                       123
#define EVENT_EXTERNAL_FILE_UPDATE                              124
#define EVENT_REMOTE_FILE_UPDATE                                125
#define EVENT_APPLICATION_DESELECT                              126
#define EVENT_FIRST_COMMAND_AFTER_ATR                           127
#define EVENT_EVENT_DOWNLOAD_ACCESS_TECHNOLOGY_CHANGE_MULTIPLE  128
#define EVENT_MEMORY_FAILURE                                    129
#define EVENT_UNRECOGNIZED_ENVELOPE                             -1
//*******************************Events Constants for C COS***********************************
 /*
*         字节１　　　　　　　			 字节2				     　　字节３						       　　字节４					      　　字节５					   　　　 字节６                                   字节7
*　b7 b6 b5 b4 b3 b2 b1 b0       b15 b14 b13 b12 b11 b10 b9 b8    b23 b22 b21 b20 b19 b18 b17 b16    b31 b30 b29 b28 b27 b26 b25 b24      b39 b38 b37 b36 b35 b34 b33 b32 	  b47 b46 b45 b44 b43 b42 b41 b40   b55 b54 b53 b52 b51 b50 b49 b48
*
*/
#define  EVENT_PROFILE_DOWNLOAD_bit                     1 

#define  EVENT_FORMATTED_SMS_PP_ENV_bit                 2    //reserved by 3GPP
#define  EVENT_FORMATTED_SMS_PP_UPD_bit                 3    //reserved by 3GPP              
#define  EVENT_UNFORMATTED_SMS_PP_ENV_bit               4    //reserved by 3GPP
#define  EVENT_UNFORMATTED_SMS_PP_UPD_bit               5    //reserved by 3GPP
#define  EVENT_UNFORMATTED_SMS_CB_bit                   6    //reserved by 3GPP   
      
#define  EVENT_MENU_SELECTION_bit                       7                  
#define  EVENT_MENU_SELECTION_HELP_REQUEST_bit          8   
#define  EVENT_CALL_CONTROL_BY_SIM_bit                  9 

#define  EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM_bit      10     //reserved by 3GPP   
 
#define  EVENT_TIMER_EXPIRATION_bit                     11
#define  EVENT_EVENT_DOWNLOAD_MT_CALL_bit               12
#define  EVENT_EVENT_DOWNLOAD_CALL_CONNECTED_bit        13
#define  EVENT_EVENT_DOWNLOAD_CALL_DISCONNECTED_bit     14
#define  EVENT_EVENT_DOWNLOAD_LOCATION_STATUS_bit       15                         
#define  EVENT_EVENT_DOWNLOAD_USER_ACTIVITY_bit         16
#define  EVENT_EVENT_DOWNLOAD_IDLE_SCREEN_AVAILABLE_bit 17
#define  EVENT_EVENT_DOWNLOAD_CARD_READER_STATUS_bit    18       
#define  EVENT_STATUS_COMMAND_bit                       19                             
#define  EVENT_EVENT_DOWNLOAD_LANGUAGE_SELECTION_bit    20                             
#define  EVENT_EVENT_DOWNLOAD_BROWSER_TERMINATION_bit   21       
#define  EVENT_EVENT_DOWNLOAD_DATA_AVAILABLE_bit     	22                            
#define  EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS_bit        23       
                     
#define  EVENT_FORMATTED_SMS_CB_bit                     24   //reserved by 3GPP
#define  EVENT_FIRST_COMMAND_AFTER_SELECT_bit          44/** Event : First Command after GSM application selection = 127*///该事件同UICC EVENT_FIRST_COMMAND_AFTER_ATR_bit事件

#define  EVENT_EVENT_DOWNLOAD_ACCESS_TECHNOLOGY_CHANGE_bit          25
#define  EVENT_EVENT_DOWNLOAD_DISPLAY_PARAMETER_CHANGED_bit         26
#define  EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION_bit                  27
#define  EVENT_EVENT_DOWNLOAD_NETWORK_SEARCH_MODE_CHANGE_bit        28
#define  EVENT_EVENT_DOWNLOAD_BROWSING_STATUS_bit                   29


#define    EVENT_EVENT_DOWNLOAD_IWLAN_ACCESS_STATUS_bit                30	   //30  reserved by 3GPP
#define    EVENT_EVENT_DOWNLOAD_NETWORK_REJECTION_bit                  31	   //31  reserved by 3GPP

#define   EVENT_EVENT_DOWNLOAD_HCI_CONNECTIVITY_bit                 32

#define    EVENT_EVENT_DOWNLOAD_CSG_CELL_SELECTION_bit                 33	   //33  reserved by 3GPP

#define   EVENT_EVENT_DOWNLOAD_FRAMES_INFORMATION_CHANGED_bit       34
#define   EVENT_EVENT_DOWNLOAD_CONTACTLESS_STATE_REQUEST_bit        35


#define   EVENT_EVENT_DOWNLOAD_IMS_REGISTRATION_bit               36	  //  reserved by 3GPP
#define   EVENT_EVENT_DOWNLOAD_INCOMING_IMS_DATA_bit              37	  //  reserved by 3GPP
#define   EVENT_FORMATTED_USSD_bit                                38	  //  reserved by 3GPP
#define   EVENT_UNFORMATTED_USSD_bit                              39	  //  reserved by 3GPP

#define   EVENT_PROACTIVE_HANDLER_AVAILABLE_bit                     40
#define   EVENT_EXTERNAL_FILE_UPDATE_bit                            41
#define   EVENT_REMOTE_FILE_UPDATE_bit                              42
#define   EVENT_APPLICATION_DESELECT_bit                            43

#define  EVENT_FIRST_COMMAND_AFTER_ATR_bit                       44/** Event : First Command after ATR = 127*/

#define  EVENT_EVENT_DOWNLOAD_ACCESS_TECHNOLOGY_CHANGE_MULTIPLE_bit 45
#define  EVENT_MEMORY_FAILURE_bit                                   46
  
#define  EVENT_UNRECOGNIZED_ENVELOPE_bit                0 /** Event : Unrecognized Envelope = -1 */


#define  EVENT_InitValue                                       0
#define  EVENT_Private_SCP81Connection_EventObserved           0xFF

/*------------以下为G_ToolkitSystemProactiveFlag变量bit位定义-----------*/
//根据规范描述： 6.7.1.2
//At the beginning of a CAT session, the CAT Runtime Environment shall send a SET UP EVENT LIST system proactive command, if at least one of the EVENT_EVENT_DOWNLOAD_* events is registered by a selectable Toolkit applet.
//During a CAT session the CAT Runtime Environment shall send a SET UP EVENT LIST system proactive command whenever the registered event list is changed.


//6.7.1.1 SET UP MENU
//At the beginning of a CAT session, the CAT Runtime Environment shall send a SET UP MENU system proactive command, if at least one menu entry is registered and enabled by a selectable Toolkit applet.
//During a CAT session the CAT Runtime Environment shall send a SET UP MENU system proactive command whenever a menu entry is modified, added or removed or the EFSUME file under the DFTELECOM file is updated as defined in TS 102 222 [8].

#define Bit_SetUpMenu                      0   //该bit位表示在会话中，是否发送setupmenu主动式命令
#define Bit_SetUpEventList                 1   //该bit位表示在会话中，是否上发setupEventList主动式命令
#define Bit_PollOff                  	   2    //modifyby xuweihua  将Bit_PollOff和Bit_PollInterval互换，这样如果应用先请求polloff在请求pollInterval，则会先发polloff，在发pollInterval，外场测试反馈
#define Bit_PollInterval                   3
#define Bit_PowerOnSetupMenu               4    //该bit位表示在开机会话开始时是否要发送setupmenu主动式命令
#define Bit_PowerOnSetupEventList          5    //该bit位表示开机时是否要上发setupEventList主动式命令
#define Bit_PowerOnPollInterval            6    //该bit位表示开机时是否要上发setupEventList主动式命令
#define Bit_Pending                        7					 //该位表示有系统主动式指令在ResData等待Fetch，该字节在收到TerminalResponse以后清除

/**************************File Define Begin*************************************************/

#define   EF_EVENTListConfig           0x6F76
#define   EF_ChinaMobileTar            0x6F77
#define   EF_RAMAPDUConfig             0x6F79    //配置文件，主要和EF_RAMAPDUTars一起使用，暂时是2字节

#define   EF_PrivateConfig             0x4E03   //字节，目前第一字节没用，第二字节表示POR重发次数，第三字节管理上行批次，如果该字节为FF，表示从0开始，每次加1；该文件要么不存在，否则一定要正确配置，比如重发次数不能为0或FF
#define   EF_POR_HEADER                0x4E04  //对于不同的tar值，可以通过该文件配置不同的上行参数如TPOA PID DCS等	，当该文件不存在时，则使用下行数据中的参数发送上行
#define   EF_RFMRAM_MSL                0x4E8A  //该文件结构为TLVTLV(MSLID、length、MSLLevel) 结构，初始化为全FF,FF表示数据结束，因此不能存在MSLID为全FF的情况，MSLI从01开始，因为4E05文件中00时表示处理他自己的msl

#define	  EF_RFMRAM_COUNTER            0x4E09  //线性文件,海外RAM,RFMCOUNTER文件

/*************************以下为EF_RFM_TAR 4E05文件的结构描述××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××*/

typedef __packed struct tagRFMAPDUTars
{
	BYTE mslConfig; //00：表示所有的RFM 使用固定的SPI进行比较 .else :使用MSL记录文件中的msl值  
	BYTE counterConfig;  //00:tar 与counter对应， else：counter和 key对应
	BYTE counter[5];
	BYTE tarNum;
	BYTE tar[9];  //可根据记录个数进行扩展
}tagRFMAPDUTarsInfo;

#define RFM_mslConfig_offset      0x00
#define RFM_counterConfig_offset  0x01
#define RFM_counter_offset        0x02
#define RFM_tar_num_offset        0x07
#define RFM_tar_offset            0x08

#define TAG_ClearRamExtendRes      0x01
#define TAG_ResponseRamExtendRes   0x02
#define TAG_ComNumRamExtendRes	   0x03   

/************以下为6F79 EF_RAMAPDUConfig的文件描述，该文件需要与EF_RAMAPDUTars配套使用*****************************************/
#define counterConfig_offset    0x00  //00:RAMtar使用唯一的counter；  else：使用scp80 key的counter
#define isd_mslConfig_offset    0x01  //00：使用sd自己的msl ，others ：使用MSL记录文件中的msl值	,
#define ssd_mslConfig_offset    0x02  //00：使用sd自己的msl ，others ：使用MSL记录文件中的msl值	,所有的ssd使用同一配置
/************以下为6F7B EF_RAMAPDUTars文件描述**********************************************/ 
/*
*@appId     :安全域的appID
*@counter[5]:5字节counter，在处理RAM时，根据配置信息，决定是否用统一的counter还是使用相关key的counter
*@msl[3]    :下行数据中的msl，其中MSL[0] ：LENGTH ，MSL[1] ：MSL Parameter 01 or 00, MSL[2] :Minimum SPI1
*@TarLength :i*3 ,目前每个sd最多支持3个tar
*@Tar[0-8]  :3 个Tars值

*该文件需要将ISD的初始化值预置进去，第一条记录内容必须配置成ISD的信息，如下：
00 0000000000 01 020100  01 000000  ;01表示MSL的判断采用MSL配置文件中的第1条
*/
typedef __packed struct tagRAMAPDUTars
{
	BYTE appId;   
	BYTE mslValue;  //该文件中的MSL值
	BYTE tarNumber;  //tar长度，最长为9字节，表示支持3个tar
	BYTE tar[9]; 
}tagRAMAPDUTarsInfo;

#define sdAppId_offset     0x00
#define sdMslValue_Offset  0x01
#define sdTarNumber_offset 0x02
#define sdTarValue_offset  0x03

#define  SD_TAR_INFO_LEN   0x0C

/*------------以下为6F80中字段偏移定义-----------*/
//note: 针对AccessDomain描述,若某个AccessDomain的长度为0或查找不到特定的arrNum(arrNum可代表MF或某个ADF),则认为当前toolkit不拥有对相应MF或ADF的访问管理权限
#define EventList_Length     7
typedef __packed struct tagToolitAppInfo
{
	BYTE AppletStatus;
	BYTE AppId;
	BYTE trigerMode;
	BYTE EventList[7];
	BYTE MaxTimerCount;
	BYTE AllocateTimerCount;
	BYTE MaxChannelCount;
	BYTE AllocateChannelCount;
	BYTE AppPriority;
	WORD InstallSequence;
	BYTE MFAccessDomainLength;  //主要指MF下的文件访问权限，不可以访问ADF下的文件
	BYTE MFAccessDomain[4];
	BYTE MaxEntryTextLen;
	BYTE MSLLength;
	BYTE MSLCoding[0x02]; //MSLCoding[0] :MSL Parameter	,MSLCoding[1]: MSL Data
	BYTE TARLength;
	BYTE TARValues[15];  // 暂时假定一个应用支持5个TARs
	BYTE MaxServiceCount;
	BYTE AllocateService;
	BYTE CurrentService;  //当前是否执行了主动式命令DECLARE SERVICE (add)
	WORD PollDuration;
	BYTE ADFAccessDomainLgth;  //保存ADF下的文件访问权限
	BYTE ADFAccessDomain[30];  //暂假定没有toolkit最多可以访问卡片上的5个ADF，每个ADF的权限信息用6个字节表示，first byte：2F00记录号，second byte:lgth,,lastBytes:AccessDomainBytes&Access Domain Parameter
	BYTE AdminAccessLgth;
	BYTE AdminAccessDomain[36]; //暂假定Admin包括MF下的和5个ADF下的权限，MF的权限记录号用00表示,即若当前查找不到arrNum = 0x00,即认为不拥有MF下文件的管理权限
	BYTE MaxEntryCount;
	BYTE MenuEntryPosition[20];      //目前支持10个Java Toolkit应用菜单，此处扩大可增加支持菜单个数，需要修改6F80文件大小。
}ToolitAppInfo;

#define APPLET_UNLOCKED_SELECTABLE      0x81	//AppletStatus

#define APP_UNLOCK_BIT                  0       //bit0==1表示UNLOCK
#define APP_SELECTABLE_BIT              7       //bit7==1表示SELECTABLE

#define  JCREMain_INITMode             		0x00	//该模式为上电初始化模式，上电会初始化为该值；用于判断热复位时是否需要根据JCRE规范做相关的操作，如果G_JCREMainProcessMode == JCREMain_INITMode，表示卡片还未进入java层 不需要做JCRE规范描述的相关操作 直接退出热复位处理
#define  JCREMain_APDUMode             		0x01
#define  JCREMain_UICCAPIMode          		0x02
#define  JCREMain_SIMAPIMode           		0x03
#define  JCREMain_RAMMode              		0x04
#define  JCREMain_ISDRRAMMode               0x05    //TarType_ISDR
#define  JCREMain_ISDPRAMMode               0x06    //TarType_ISDP

//#define  JCREMain_ProfileInstallMode        0x07   //
#define  NULLAPI_Mode           0xFF
#define  SIMAPI_Mode            JCREMain_SIMAPIMode
#define  UICCAPI_Mode           JCREMain_UICCAPIMode

#define AppletStatus_OFFSET                 0                                
#define AppId_OFFSET                       (AppletStatus_OFFSET + 1)   
#define TrigerMode_OFFSET                  (AppId_OFFSET + 1)  //使用APPID获取对应数据，比如AID data时，要注意如果在installforinstall中调用了SIMAPI此时应用并未写到注册表中，注册表中的信息是获取不到的
#define EventList_OFFSET          		   (TrigerMode_OFFSET +1)
#define MaxTimerCount_OFFSET               (EventList_OFFSET + EventList_Length)           
#define AllocateTimerCount_OFFSET          (MaxTimerCount_OFFSET+1)       
#define MaxChannelCount_OFFSET             (AllocateTimerCount_OFFSET + 1)    
#define AllocateChannelCount_OFFSET        (MaxChannelCount_OFFSET + 1)      
#define AppPriority_OFFSET                 (AllocateChannelCount_OFFSET + 1)  
#define InstallSequence_OFFSET             (AppPriority_OFFSET + 1)          
#define MFAccessDomainLength_OFFSET        (InstallSequence_OFFSET + 2)      
#define MFAccessDomainContent_OFFSET       (MFAccessDomainLength_OFFSET + 1)   
#define MaxEntryTextLen_OFFSET             (MFAccessDomainContent_OFFSET + 4)   
#define MSLLen_OFFSET                      (MaxEntryTextLen_OFFSET + 1)      
#define MSLCoding_OFFSET                   (MSLLen_OFFSET + 1) 
#define TARLength_OFFSET                   (MSLCoding_OFFSET + 2)
#define TARValues_OFFSET  			       (TARLength_OFFSET + 1)
#define MaxServiceCount_OFFSET             (TARValues_OFFSET + 15)
#define AllocateService_OFFSET             (MaxServiceCount_OFFSET + 1)
#define CurrentService_OFFSET			   (AllocateService_OFFSET + 1)
#define PollDuration_OFFSET				   (CurrentService_OFFSET + 1)
#define ADFAccessDomainLgth_OFFSET         (PollDuration_OFFSET + 2)
#define ADFAccessDomainValue_OFFSET        (ADFAccessDomainLgth_OFFSET + 1)
#define AdminAccessLgth_OFFSET             (ADFAccessDomainValue_OFFSET + 30)
#define AdminAccessDomainValue_OFFSET      (AdminAccessLgth_OFFSET+1)
#define MaxEntryCount_OFFSET               (AdminAccessDomainValue_OFFSET + 36)    
#define MenuEntryPosition_OFFSET           (MaxEntryCount_OFFSET + 1)

#define ToolkitApp_Item_Size_No_MenuEntry                  0x74      //不包括MenuEntry 信息的长度
#define ToolkitApp_Info_Size                  0x88                //6F80内容的长度，在heap 中分配
#define ToolkitApp_MenuEntry_Size             0x36            //6F81内容的长度，在heap 中分配
/*------------以下为6F81中字段偏移定义-----------*/

typedef __packed struct tagToolitEntryInfo
{
	BYTE EntryStatus;	    //bit0-bit7:bit1为1：表示Menu_ENABLE，为0表示Menu_DISABLE ； bit6为1表示selectable, 0表示其他；bit7为1：表示Menu_UNLOCKED，为0表示	Menu_LOCKED
	BYTE AppRecNum;			//6F80中的记录号
	BYTE ItemIdentifier;
	BYTE nextAction;
	BYTE helpSupport;       //TRUE or false
	BYTE IconIdentifier[2];
	BYTE IconQulifier;
	BYTE MenuEntryTextLen;
	BYTE MenuEntryText[40];
	BYTE TextAttribute[4]; //初始化默认值为  00 00 03 90 
}ToolkitMenuEntryInfo;


#define Menu_UNLOCKED  0x80
#define Menu_SELECTABLE 0x40
#define Menu_LOCKED    0x00
#define Menu_DISABLE   0x00
#define Menu_ENABLE    0x01

#define MenuEntryStatus_OFFSET       0                              
#define AppRecNum_OFFSET             (MenuEntryStatus_OFFSET + 1) 
#define ItemIdentifier_OFFSET        (MenuEntryStatus_OFFSET + 2)       
#define NextAction_OFFSET            (MenuEntryStatus_OFFSET + 3)      
#define HelpSupported_OFFSET 	     (MenuEntryStatus_OFFSET + 4)         
#define IconIdentifier_OFFSET        (MenuEntryStatus_OFFSET + 5)      
#define IconQulifier_OFFSET          (MenuEntryStatus_OFFSET + 7)      
#define MenuEntryTextLen_OFFSET      (MenuEntryStatus_OFFSET + 8)        
#define MenuEntryText_OFFSET         (MenuEntryStatus_OFFSET + 9)  
#define TextAttribute_OFFSET         (MenuEntryStatus_OFFSET + 49)     
/*------------以下为6F84中字段偏移定义-----------*/
//该文件初始化为全0，清空时也清成0	 ,代码处理时，按照记录长度的一半进行计算的
typedef __packed struct ToolkitFileListInfo
{
	BYTE extralFileNumber;
	BYTE extralFileList[20];
	BYTE remoteFileNumber;  
	BYTE remoteFileList[20];
}ToolkitFileListInfo;


/**************************File Define END***************************************************/

/**************************Global Variables Define Begin*************************************/
extern  UWORD   xdata     G_EnvelopHanderAddr;        
extern  BYTE     xdata    G_EnvelopeTag;    //用于保存当前的Envelope BER-TLV tag ，C2指令的第一字节 
extern  BYTE     xdata    G_ToolkitSystemProactiveFlag;
//extern  BYTE    xdata    G_PowerOnProfielDone;  //判断当前是否已经 执行了开机的主动式命令seupmenu setupeVENTLIST POLL INTERVAL
extern  WORD    xdata    G_TrigerToolkitRef;    //保存待触发toolkit应用的ref
extern  BYTE    xdata    G_ToolkitTar[0x03];  //缓存当前format_CB,format_update format_env的tar值
extern BYTE    xdata    G_SPIKICKID[4];
extern BYTE    xdata G_CcLengthForAES;
extern  BYTE    xdata    G_TARRecIndex[3];
extern  BYTE    xdata    G_RFMRAMKeyversion;//缓存当前RAM短信使用的RAM密钥版本
extern  BYTE    xdata    G_formatedSPIOffset;    //缓存当前format ENV Data 的spi在6F90中的偏移
extern  BYTE    xdata    G_formatedTPOAOffset;   //缓存当前format ENV Data 的TPOA 在6F90中的偏移
extern  BYTE    xdata    G_CurrentRFMResponse;       //通过getresponse需要获取的当前POR的数据长度
//extern  BYTE   xdata     G_VolatileByteArrayIsCalled; //用于标记当前指令是否调用了getTheVolatileByteArray API
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
extern  WORD    xdata    G_ToolkitAppletInfoRef;
extern  BYTE    xdata    G_ToolkitAppIndex;//正在安装的Toolkit应用的apptabelEntry Index
#endif
extern  WORD    xdata    G_SD_MenuInfoRef;

#define  SD_MENU_INFO_LEN   0x16
/**************************Global Variables Define END***************************************/

/**************************Others Variables Define Begin*************************************/
#define TimerSum	   0x08

#define  FINDNULL_MODE  0x01
#define  COUNT_MODE     0x02

#define	APDUMode 0x00
#define	RAMMode  0x01

#define	MODE_SECURE_OFFSET   0x00
#define	MODE_SECURE_LENGTH   0x01
#define	MODE_MESSAGE_OFFSET  0x02
#define	MODE_MESSAGE_LENGTH  0x03

#define  AppAddr_Mode      0x00
#define	 ExtralFile_Mode   0x25	   //#define   EVENT_EXTERNAL_FILE_UPDATE_bit        37
#define  RemoteFile_Mode   0x26	   //#define   EVENT_REMOTE_FILE_UPDATE_bit          38


/*************FORMATTED SMS-CB MID*****************************************/
#define  CBS_FORMATTED_MIDMIN   0x1080
#define  CBS_FORMATTED_MIDMAX   0x109F

/*********************************************************************************************/
#define Index_EnvelopeHandler          0x00
#define Index_EnveResHandler           0x01
#define Index_ProactiveHandler         0x02
#define Index_ProResHandler            0x03
#define Index_BERTLVEditHandler        0x04

typedef __packed struct tagToolkitHandler
{
	BYTE * HandlerBuffer;	        //指向TLVList存储区的指针				
	WORD   HandlerBufferSize;       //TLVList存储区总空间大小                                
	WORD   TLVListLength;           //已存储的TLVList的大小
	WORD   CurrentTLVLength;        //当前选中的TLV的length
	WORD   CurrentTLVValueOffset;   //当前选中的TLV的Value的偏移量,如果当前没有选中的TLV，CurrentTLVValueOffset的值为0xFFFF;    
	BYTE   HandlerAvailable;        //表示当前Handler是否有效，1有效，0无效    
}ToolkitHandler;

extern ToolkitHandler ToolkitHandlerList[5];


/*该结构体用于规定javatoolkit应用执行时需维护的临时数据*/
typedef __packed struct tagToolkitContext
{
	BYTE CurrentApprecNum[MAX_LOGICAL_CHANNELS];	//处理非共享文件时，需要使用到该变量，该变量的多通道是为了支持：应用可以多通道处理不同类型的fileView，这里的应用可以是toolkit应用也可以是Java Applet
	SBYTE CurrentEvent[MAX_LOGICAL_CHANNELS];  //为支持对于普通A4选择Toolkit应用可以进行文件处理，但是不能进行processToolkit处理(将event初始化成FALSE,在gethandler会抛异常)；此处event按照通道
	//BYTE CurrentTrigeMode[MAX_LOGICAL_CHANNELS];    //支持一个通道触发Toolkit应用，同时另一通道进行SIMAccess 等操作，两个通道之间的操作相互没有影响
}ToolkitContext;

extern ToolkitContext G_ToolkitContext;

extern  BYTE    xdata    G_PostTag;
//extern  WORD    xdata    G_CUR_DURATION;
extern  UWORD   xdata    G_PollIntervalRecNum;
extern  BYTE    xdata    G_TriggerAppNumber;  //当前event触发的toolkit app的个数
extern  BYTE    xdata    G_ISOBusyExceptionFlag;  //判断当前是否要向终端发送由toolkit app抛出的9300异常
/********************fileSystem Mode************************************************/

#define    ADFFileSystem        0xA0	   //ADFFileSystem :访问ADF下的DF或EF
#define    UICCFileSystem       0xA1	   //仅可访问MF下的DF 和EF
#define    AdminADFFileSystem 	0xA2		//ADF文件系统的管理权限
#define    AdminUICCFileSystem  0xA3       //UICC文件系统的管理权限

/***************record mode***********************************************/
#define REC_ACC_MODE_CURRENT    0x04
#define REC_ACC_MODE_ABSOLUTE   0x04
#define REC_ACC_MODE_NEXT       0x02
#define REC_ACC_MODE_PREVIOUS   0x03

/**************************UICCToolkitPublic.c**********************************************/
extern void SetG_ToolkitSystemProactiveFlag(u16 bit);

extern BYTE PostAfterReceiveCommandHeader(void);

extern BYTE UICCToolkitFindSpecifiedAID(BYTE *aidBuf, BYTE aidLgth);

extern BYTE GetToolkitEntryAppNumByAppId(BYTE appID);

extern BYTE GetCurrentToolkitEntryAppNum(void);

extern  BYTE ToolkitFindTLVPublic(BYTE handlerIndex,BYTE tag,BYTE occurrence, BYTE apiMode);

extern  BYTE Toolkit_getFirstByteOfSpecifiedTag(BYTE handlerIndex, BYTE tag, BYTE apiMode);

extern  BYTE Toolkit_getValueBytePublic(BYTE handlerIndex,WORD offset, BYTE apiMode);

extern void handler_check_currenttlv(BYTE handlerIndex,BYTE apiMode);

extern void handler_check_available(BYTE handlerIndex, BYTE apiMode);

extern void  ClearHandlerCurrentTLV(BYTE handlerIndex);

extern BYTE ToolKitFindAppRecordNumById(BYTE menuId);
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
extern BYTE SearchFileList(BYTE* filelist, BYTE* blkNum);
#endif
extern BYTE  ToolkitfindSpecifiedTARValue(BYTE *tar);

extern void UICCToolkitInit(void);

extern WORD JcvmProcessRemoteAppManageAPDU(void);

extern void ThrowSIMViewException(jshort reason);

extern void ThrowToolkitException(jshort reason);

extern DWORD GetArrayBufferAddress(jref refrence);

extern void ThrowToolkitException(jshort reason);

extern void UICCThrowToolkitException(jshort reason);

extern void UICCThrowUICCException(jshort reason);

extern void UICCThrowAdminException(jshort reason);

extern void UICCThrowSystemException(jshort reason);

extern void ThrowAPDUException(jshort reason);

extern void check_bounds_interval_buffer(WORD objref,SWORD offset,SWORD length)	;

extern UWORD GetValidArrayDataAddr(WORD arrayRef, SWORD soffset,SWORD  sLength);

extern BYTE CallToolkitApplet(BYTE recNum,SBYTE event,BYTE trigerMode);

extern WORD ToolkitSubGetSecureDataLenOffset(BYTE mode);

extern WORD Toolkit_subTPUDLOffset(void);

extern void ToolkitAvaiableOfEnvelopResponse(BYTE apiMode);

extern BYTE Toolkit_publicProfileCheck1(jshort index, BYTE apiMode);

extern void check_profile_available(BYTE apiMode);

extern BYTE ProactivecmdPermission(void);

extern BYTE SetToolkitChannelServiceIdStatus(BYTE Id,  BYTE mode,WORD offset);

extern void checkStkBusy(BYTE apiMode);

extern WORD Toolkit_subGetOtherDataLength(BYTE tag);

extern jshort Toolkit_copyTagDataToDstBuffer(BYTE tag,BYTE offset);

extern jbyte Toolkit_BaseCompareDstBuffer(jbyte handlerIndex, jshort valueOffset, objref_t compareBuffer, jshort compareOffset, jshort compareLength,BYTE mode,BYTE apiMode);

extern void Toolkit_copyHandlerBufferDataToDstBuffer(BYTE handlerIndex,WORD handlerOffset,objref_t dstBuffer,WORD dstOffset,WORD copyLength);

extern jshort Toolkit_BaseCopyDataToDstBuffer(jbyte handlerIndex, jshort valueOffset, objref_t dstBuffer, jshort dstOffset, jshort dstLength,BYTE mode, BYTE apiMode);

/**************************Functions Define END*********************************************/
extern void CopySrcDataToArray(BYTE* srcbuff,jref arrayref,jshort offset,jshort length);

extern void CopyArrayDataToDest(BYTE * destaddr,jref arrayref,jshort offset,jshort length);

/***********************************ToolkitRegistryPublic**********************************************************/
extern BYTE ToolkitEventRegistered(BYTE localEvent)	 ;

extern void releaseTimerAndService(BYTE serviceTimeId,WORD offset,BYTE eventBit,BYTE reason, BYTE apiMode);

extern void allocateTimerAndService(BYTE serviceTimeId,WORD offset,BYTE eventBit,BYTE reason, BYTE apiMode);

extern BYTE findUnallocatedTimerIdAndServiceId(WORD offset);

extern BYTE ToolkitFindSpecifiedMenuId(BYTE menuId);

extern BYTE ToolkitFindSpecifiedMenuIdForInstall(BYTE menuId);

extern void AllocateTimerPublic(void);

extern void AllocateTimerPublic(void);

extern void ChangeMenuEntryPublic(void);

extern void setMenuStatusPublic(BYTE menuStatus);

extern void GetPollIntervalPublic(void);

extern void InitMenuEntryPublic(void);

extern void ReleaseTimerPublic(void) ;

extern void RequestPollIntervalPublic(void);

extern void InitDisplayGetInkeyGetInput(BYTE mode);

/************************************************************************/
extern BYTE GetBufBit(BYTE *buf, BYTE bit);

extern void SetBufBit(BYTE *buf, BYTE bit);

extern void ClearBufBit(BYTE *buf, BYTE bit);

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP

#define    MODE_EXTERNAL_FILE_LIST     0
#define    MODE_REMOTE_FILE_LIST     1
extern WORD AddEFToolkitAppInfo(WORD newCC,BYTE* srcData,WORD arrayLen);
extern UWORD GetToolkitInfoObj(BYTE beFreeIndex);
extern UWORD GetToolkitAppInfoArrayAddr(UWORD toolkitInfoObj);
extern u32 GetCurrentToolkitEntryArr(void);
extern UWORD GetFirstToolkitEventFileListObj(UWORD toolkitInfoObj,BYTE mode);
extern UWORD GetToolkitEventFileListArr(UWORD ToolkitEventFileListObj);
extern UWORD GetNextToolkitEventFileListObj(UWORD ToolkitEventFileListObj);
extern BYTE SearchFileList(BYTE* filelist, BYTE index, BYTE mode);
extern void SetToolkitEventFileListObj(UWORD toolkitInfoObj,u16 ref,BYTE mode);
extern void clearToolkitEventFileList(UWORD toolkitInfoObj,PN8 fileList,BYTE mode);
#endif


#define ToolkitGetEventBit	    GetBufBit
#define ToolkitSetEventBit	    SetBufBit
#define ToolkitClearEventBit	ClearBufBit
//*********************ToolkitException Constants ********************************
#define HANDLER_OVERFLOW                          1
#define HANDLER_NOT_AVAILABLE                     2
#define UNAVAILABLE_ELEMENT                       3
#define MENU_ENTRY_NOT_FOUND                      4
#define REGISTRY_ERROR                            5
#define EVENT_NOT_SUPPORTED                       6
#define EVENT_ALREADY_REGISTERED                  7
#define OUT_OF_TLV_BOUNDARIES                     8
#define TERMINAL_PROFILE_NOT_AVAILABLE            9
#define ALLOWED_LENGTH_EXCEEDED                   10
#define NO_TIMER_AVAILABLE                        11
#define INVALID_TIMER_ID                          12
#define EVENT_NOT_ALLOWED                         13
#define BAD_INPUT_PARAMETER                       14
#define COMMAND_NOT_ALLOWED                       15
#define NO_SERVICE_ID_AVAILABLE                   16
#define INVALID_SERVICE_ID                        17
#define TAR_NOT_DEFINED                           18
 
#endif
