#ifndef _TOOLKITRFM_H_
#define _TOOLKITRFM_H_

#include "typedef.h"
#include "platform_api.h"
#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT
#define     POR_DELIVER_MULTI  0x80    //电信测试
extern BYTE  xdata G_Deliver_T0_Submit;
extern WORD  xdata G_PorAdditionalLen;
extern UWORD xdata G_Back_EnvelopHanderAddr;//GP补充测试脚本支持SCP02和KeyManage版20170309/Bug_List_07_01/步骤5 在STK中的deliver转Submit会导致发送第一条0B和后面的submit使用的G_EnvelopHanderAddr地址不一样
#endif
/**************************ToolkitFindOrClearSpecifiedRFMSMS mode 参数模式**********************************/
#define MODE_HEADER                0x80     //查找Header
#define MODE_FIND_NOUSE        0x00	 //查找空记录
#define MODE_FIND_RECNO        0x01	 //查找指定记录号
#define MODE_FIND_TOTAL        0x02	 //查找指定记录短信总条数
#define MODE_CLEAR_RECORD   0x03

#if SE_ONLY
#define TAR2_SIMFile_System         0x01   //B00011: SIM file System
#define TAR2_UICCFile_System        0x02   //B00002: UICC shared File System
#define TAR2_USIMFile_System        0x03   //B00020  USIM fileSystem  移动USIM TAR
#define TAR2_CSIMFile_System        0x04   //B00021  CSIM fileSystem  移动USIM TAR
#define TAR2_PRL_System               0x05   //BF01FF  PRL TAR    //目前ppdownload里面的PRL TAR只用来PUSH 来通道 当做RFM TAR来处理
#define TAR2_EPRL_System             0x06   //BF02FF  EPRL TAR
#define TAR2_OPLMN_System         0x07   //BF03FF  OPLMN TAR
#define TAR2_PKCS_System            0x08   //B001FF  PKCS TAR  路径0x7F50开始
#endif

//Config UICC bit 位定义
//目前只使用了bit 0/1/7 ,其他bit 后续可以使用
#define  BIT_UICCConfig_RollBAckWhenNoService    0   //No Service时Roll-Back 时机控制
#define  BIT_UICCConfig_ClearSessionWhen83       1   //83 时是否清session
#define  BIT_UICCConfig_Setupeventlist		     2   //开机是否发送setupeventlist  和之前产业化平台保持一致，修改这个为2 modify by chentao@20211014
#define  BIT_UICCConfig_TriggerToolkitApi        3   //default:0 -- 1 success trigger Toolkit when getShareableInterfaceObject return 0; 0 fail
#define  BIT_UICCConfig_Por_Counter_Clear        4   //default:1 -- 1 need clear por counter when error; 0 need not
#define  BIT_UICCConfig_Prl_Not_DisplayInfo      5   //default:0 -- 0 display info"PRL update sucess"; 1  not display info"PRL update sucess"
#define  BIT_PEID_CHECK                          6   //add by chentao@20220421 Germany profile has  the same PE id
#define  BIT_UICCConfig_IfRemoveSMSHeaderBytes   7   //是否要移除SMS header里面的“【东信和平】”字段（仅用于实网测试）

//No Service时Roll-Back 时机控制
#define ROLLBACK_AFTER_STATUS        0x00    // No Service时，等待10个Status 再Roll-Back (默认)
#define ROLLBACK_AT_ONCE             0x01    // No Service时，立刻Roll-Back


#define NO_SETUPEVENTLIST              0x01    // 开机不发送
#define SUPPORT_SETUPEVENTLIST         0x00    // 开机发送

//83 时是否清session
#define CLEAR_SESSION_WHEN83         0x00    // 83 时清session
#define DONOT_CLEAR_SESSION_WHEN83   0x01    // 83 时不清session

//Toolkit_ResponsePack中errorCode != POR_OK时，是否把counter清掉，默认值为清
#define POR_COUNTER_CLEAR             0x01
#define POR_COUNTER_CLEAR_NOT     0x00

//是否要移除SMS header里面的“【东信和平】”字段（仅用于实网测试）
#define DONOT_REMOVE_SMSHEADER_BYTES 0x00    // 不移除SMS header里面的“【东信和平】”字段（仅用于实网测试）
#define REMOVE_SMSHEADER_BYTES       0x01    // 移除SMS header里面的“【东信和平】”字段（仅用于实网测试）

#define RB_128BITS     0x87       //AES,b=128。TDEA,b=64
/******************for  RAM APDU*******************************************************************************************/
extern BYTE    xdata    G_RAMSessionSdId;
//extern BYTE    xdata    G_RAMGetStatus[19];

//extern WORD xdata G_CurContext; //
#define  G_CurContext      cc
/****************************************DELETE TAG OR INSTALL  TAG*******************************************************************/
#define IsDeleting_Tag                               0x01
#define IsInstalling_Tag                             0x02
#define IsRegisted_Tag                               0x04
#define IsProcessing_Tag                             0x08


//#define ILLEGAL_APPID                      0xFF	//去警告，防止重复定义，该宏已调整到contentManager.h 中定义
//#define ILLEGAL_PKGID                      0xFF	//去警告，防止重复定义，该宏已调整到contentManager.h 中定义


//#define ADDR_IS_NULL                       0x00000000


#define  CAPCOMPONENT_InvalidTag            0xFF
#define  CAPCOMPONENT_BeginTag              0xFE

/********************GetStatusTmpBuf define*************************/
#define NEXT_MATCH_INDEX                    0x00
#define LAST_P1                             0x01
#define LAST_AID_LENGTH                     0x02
#define LAST_AID_DATA                       0x03

extern WORD xdata G_CommandResponseLength;

extern BYTE xdata G_JCREMainProcessMode;

/**************SCP80 Format********************************************************************************************/
typedef __packed struct
{
	//BYTE keyVersion;        //Mapping_guidelines :6.9.1:For all Security Domains, these keys will be in the Key Version Number range '01' to '0F'.
	WORD nextKeyAddrRef;      //下一个版本的key的地址	,为了WORD 对齐,把nextKeyAddrRef 放前面,by xwh 20191104
	BYTE keyVersion;        //Mapping_guidelines :6.9.1:For all Security Domains, these keys will be in the Key Version Number range '01' to '0F'.
	BYTE keyType[0x03];           //密钥类型 的keyType，不支持FF模式,分别表示KIC KID KIK的算法标示，3个字段的算法标示不一定相同
	BYTE keyCompleteTag;    //当该值为0x07时，表示该版本的3支密钥已经完全更新成功： bit0=1：表示KID1 bit1=1表示kid2 bit2 =1：表示kid3
	BYTE keyCounter[0x05];
	BYTE keyLength[0x03];  //keyLength[0]表示KIC的长度 	   keyLength[1]表示KID的长度  keyLength[2]表示KIK的长度
	BYTE keyValue[0x60];   //最大支持32字节的AES算法，其他支持8 字节
}SCP80KeyFormat;     //SCP80密钥

//#define Offset_keyVersion           0x00
//#define Offset_nextKeyAddrRef       0x01 
//#define Offset_nextKeyAddrRef       0x00
//#define Offset_keyVersion           0x02
//#define Offset_keyType              0x03
//#define Offset_keyCompleteTag       0x06
#define Offset_keyCounter	        0x07
#define offset_scpKeyLength         0x0C
#define offset_scpKeyValue          0x0F

#define TYPE_SMS_ENV_MULTI      0x00	 //Envelope   多条信息
#define TYPE_SMS_ENV_SINGLE     0x01	 //Envelope   单条信息
#define TYPE_SMS_UPD_MULTI      0x80	 //Update SMS 多条信息
#define TYPE_SMS_UPD_SINGLE     0x81	 //Update SMS 单条信息

#define TYPE_RFMProactiveCommandTag     0xF1	 //主动式命令

//#define HEADER_MODE                       0x01   //header
//#define SMSDATA_MODE                      0x02   //data

#define TAG_SMSPP_DOWNLOAD     0xD1
#define TAG_CB_DOWNLOAD        0xD2


#define  ToolkitCBFormattedEvent           0x02
#define  ToolkitCBUnFormattedEvent         0x03

#define  HttpDataMidBlock                  0x00   //
#define  HttpDataEndBlock                  0x01   //

#define MODE_FIND_NOUSE        0x00	 //查找空记录
#define MODE_FIND_RECNO        0x01	 //查找指定记录号
#define MODE_FIND_TOTAL        0x02	 //查找指定记录短信总条数


/****************TarType define*******************************************/
#define TarType_Error                        0x00    //TarError
#define TarType_ToolkitApplet                0x01	 //该tar值用于触发应用
#define TarType_RAM                          0x02	 //RAM
#define TarType_RFM                          0x03  //RFM
#define TarType_WIB                          0x04  //WIB
#define TarType_ISDR                         0x05  //ISD_R
#define TarType_ISDP                         0x06  //ISD_P
#define TarType_CATTP                        0x07  //CATTP  

#define DataType_NULL  0x00
#define DataType_RFM   0x01
#define DataType_RAM   0x02

#define SourceType_NULL  0x00
#define SourceType_SMS   0x01
#define SourceType_CATTP 0x02
#define SourceType_HTTPS 0x03

//通过4E05/6F1A取当前tar msl 相关参数时使用
#define TARType_Index			0x01
#define Counter_Index		    0x02
#define MSL_Index				0x03
#define RFMBasePath_Index       0x04
#define WIB_SC_Index		    0x05

#define TYPE_KEY_KIC           0x01	 //KIC key type
#define TYPE_KEY_KID           0x02	 //KID key type
#define TYPE_KEY_KIK           0x03	 //KIK key type

#define MODE_KEY_KIC           TYPE_KEY_KIC//0x01	 //KIC key type
#define MODE_KEY_KID           TYPE_KEY_KID//0x02	 //KID key type
#define MODE_KIC_KID           0x83	 //KIC+KID

#define MSL_NO_SECURITY           0x80	 //No security
#define MSL_MINIMUN_SPI           0x01	 //Minimum SPI
#define MSL_MATCH_SPIKICKID       0x8E	 //Matching SPI/KIC/KID
#define MSL_MINISPI_MATCHKICKID   0x8F	 //Minimum SPI& Matching KIC/KID
#define MSL_MATCH_SPI1SPI2KICKID  0x9E	 //Matching SPI1/SPI2/KIC/KID


#define POP_OK           	                    0x00
#define ERROR_CHECKSUM   	                    0x01 
#define ERROR_CNTR_LOW   	                    0x02
#define ERROR_CNTR_HIGH  	                    0x03
#define ERROR_CNTR_BLCED 	                    0x04
#define ERROR_CIPHERING  	                    0x05
#define ERROR_SECURY     	                    0x06
#define ERROR_MEMERY     	                    0x07
#define ERROR_TIME       	                    0x08
#define ERROR_FILE       	                    0x0A

#define		KEYTYPE_DES3DES 	0x80
#define		KEYTYPE_3DES 	    0x81
#define		KEYTYPE_3DESCBC 	0x82
#define		KEYTYPE_DESECB 	    0x83
#define		KEYTYPE_DESCBC 	    0x84

#define 	TAG_NumberCommandTLV                   0x80
#define 	TAG_ClearResponseFile                  0xAA
#define 	TAG_BadFormat                          0x90
#define 	TAG_ImmediateActionResponse            0x81
#define 	TAG_ScriptChainingResponse             0x83
#define 	TAG_NumberCommandTLV                   0x80


#define		KEYITEM_LENGTH 	0x1A 

#define		PORMODE_COMMON 	            0x01  
#define		PORMODE_EVERESPONSE 	    0x02
#define     PORMODE_EXTEND              0x03   //扩展模式拼装POR数据
#define     PORMODE_AUTO_EVERESPONSE    0x04   //自动重发envelop 数据，主要用于在send或processtoolkit之前将当前的envelop response数据发送出去
#define		PORMODE_RESEND              0x80   //表示POR发送失败是否需要重发，与 PORMODE_COMMON	和 PORMODE_EVERESPONSE 可同时设置
#define		PORMODE_ISDR                0x40   //ISDR 非安全错误需要返回加密带mac的POR

#define     POR_DATA_MAX_SUBMIT		140
#define     POR_DATA_MAX_DELIVER	0xC8

#define HTTP_RFM_MODE_READ                         0x01           
#define HTTP_RFM_MODE_WRITE                        0x02
#define HTTP_RFM_MODE_CLEAR                        0x03

#define D_TmpBufStart 232
#define D_TmpBufStartWithAES 208


#define     SMS_MODE_FIRST          0x01
#define     SMS_MODE_RESEND         0x02
//更新当前会话使用的SCP80时候,发送POR时候要使用更新前的key，发送POR后使用新的key,所以在执行RAM操作的时候把更新前的key暂时保存到这个POR文件,执行完POR清空
//#define  SCP80Keyoffset            0x60   //一般来说 POR文件会>0x60+33 +33  存储格式TLV 00(Kic)/01(kid)
//#define  SCP80Keyoffset            (BIPModule_Send_SIZE - 0x80)  //使用BIPModule_Send_ADDR_START 最后一段作为保存旧的key 的地址, modify by xuweihua@20210625: 优化空间,EPX产业化/THC_THD89/Branch/EPX_THC_Mobile5G/Develop
//*************************************************
#define Fitst_SMS					1
#define FOLLOW_SMS					2
#define LAST_SMS					4
#define FIRST_LAST_SMS				5
#define FIRST_FOLLOW_SMS			3

/*************Response Status Code*****************************************/
#define  POR_OK                 0x00
#define  CCFailed               0x01
#define  CNTRLow                0x02
#define  CNTRHigh               0x03
#define  CNTRBlocked            0x04
#define  CipheringError         0x05
#define  RFMSecurityError  		0x06
#define  InsufficientMemory     0x07
#define  RFMNeedMoreTime        0x08
#define  TARUnknown             0x09
#define  RFMSecurityLevel       0x0A
#define  GPAnnexBAPDUError      0x0B		//https特殊错误，GPAnnexB的宏没有开启
//#define  NOTGPAnnexBTritag      0x0C		//不是https的触发tag

//*************************************************************************************************************************
extern BYTE xdata G_CurrentComNUM;               //保存当前RFM执行的APDU 个数

#if UICC_SUPPORT_CHAGE_DELIVER_POR
extern BYTE xdata G_BakCommandNumber;
#endif

extern WORD xdata G_RfmProactiveCommand_offset;	 //保存RFM中的主动式命令的起始(D0字节)

extern BYTE    xdata    G_FormatAndType;				//当前数据格式的模式
extern WORD    xdata    G_CurrentRemoteResponseLgth;	//https的下行数据执行完apdu指令后的响应数据长度
extern BYTE xdata G_NewSCPKey;     // add chentao 更新SCP80key后，发响应值需要用更新前的key，发完响应后用新的key
extern UWORD xdata G_CurrentRFMEfAddr; //用来保存102226读取数据过长时候的地址
extern BYTE xdata G_Source_Type;      //区分数据源 CATTP/Http/SMS
extern BYTE xdata G_SpecialRFM;
extern BYTE xdata  G_Data_Type;       //区分数据格式 压缩/AA/AE等
extern BYTE  xdata G_LAST_REFNUMBER;
extern BYTE  xdata G_86errorNum;
extern WORD  xdata CommandStartNum;
/****************************************function define*******************************************************************/
extern BYTE ToolkitSaveFirstFomatData(BYTE udPos,BYTE eventType,BYTE *tpoa);

extern BYTE ToolkitSaveRFMHead(BYTE refrence, BYTE eventID,BYTE udposOffset,BYTE smsType,BYTE *tpoa);

//extern BYTE ToolkitFindSpecifiedHeaderNum( BYTE mode, BYTE refrence,BYTE smsType,BYTE *tpoa);

//extern BYTE ToolkitFindSpecifiedRFMSMS(BYTE mode ,BYTE refrence,BYTE index,BYTE smsType,BYTE *tpoa);
extern BYTE ToolkitFindOrClearSpecifiedRFMSMS(BYTE mode ,BYTE refrence,BYTE index,BYTE smsType,BYTE *tpoa);

#define ToolkitFindSpecifiedHeaderNum(mode,refrence,smsType,tpoa)  ToolkitFindOrClearSpecifiedRFMSMS(mode|MODE_HEADER,refrence,0x00,smsType,tpoa)
#define ToolkitFindSpecifiedRFMSMS(mode,refrence,index,smsType,tpoa)  ToolkitFindOrClearSpecifiedRFMSMS(mode,refrence,index,smsType,tpoa)
#define ToolkitClearRFMSms(mode,refrence,smsType ,tpoa)  ToolkitFindOrClearSpecifiedRFMSMS(mode,refrence,0x00,smsType ,tpoa)

extern BYTE ToolkitSaveRFMSMS(BYTE udPos,BYTE recNo,BYTE smsType,BYTE *tpoa);

extern void ToolkitClearPackage(BYTE diffcurrentRefrence,BYTE smsType,BYTE *tpoa);

//extern void ToolkitClearRFMSms(BYTE refrence,BYTE smsType,BYTE mode, BYTE * tpoa);

extern void ToolkitRFMExcute(BYTE smsType,BYTE refrence);

extern BYTE ToolkitRFMSMSReceivedAll(BYTE refrence,BYTE smsType,BYTE *tpoa);

extern WORD ToolkitPatchUnformatENVSMS(BYTE refrence, BYTE headerNo,BYTE smsType,BYTE *tpoa);

extern WORD ToolkitExcuteFormattedRFMSMS(BYTE refrence,BYTE headerNo,BYTE smsType,BYTE *tpoa);

extern BYTE CheckRAMMinimumSecurityLevel(BYTE spi);

extern BYTE ToolkitGetCPLLength(BYTE CPL1);

extern BYTE ToolkitDeChipherRe(BYTE kic,BYTE * pointer,BYTE * result_pointer,BYTE mode,WORD lgth, BYTE last);

extern WORD ToolkitProcessHeader(BYTE headerNo,WORD udlLength,BYTE mode);

extern BYTE ToolkitCalculateMac(UWORD eeAddr,BYTE spi,BYTE kic,BYTE kid,BYTE cplLength,BYTE ccLength,WORD length,BYTE * resultBuffer);

extern unsigned long lGenCRC32( unsigned long lOldCRC,BYTE ByteVal );

extern BYTE IncreatCNTR(BYTE cntr[5]);

extern void ToolkitProcessRFMRAMWIBApp(WORD cplOffset);

extern void Toolkit_ResponsePack(BYTE errorCode,BYTE mode,WORD additionalLen, BYTE httpTriStatus);//modify by jiangqingbin-20220303, 增加一个传入参数，表示http短信触发结果，不再使用原来的复用SW1，SW2的方式 

extern BYTE Toolkit_CalPORMac(BYTE spi,BYTE kid,BYTE *macBuf,BYTE macLength,BYTE * buffer,WORD length);

extern void ToolkitClearAllData(BYTE smsType, BYTE reference,BYTE *tpoa);

extern BYTE ToolkitEnvelopIsSubmitTar(BYTE firstNo);

extern void GetEnvelopFirstRecTar(BYTE firstRecNo, BYTE *spi);

extern void ToolkitCBExcute(WORD SN,WORD MID,BYTE pageByte);

extern BYTE ToolkitProcessSMSUpdate(void);

extern BYTE CheckTar(BYTE *inputtar, BYTE *tarType);

extern BYTE  GPRAMAPDUExtendResponse(BYTE tag);

extern void  ProcessRAMAPDU(UWORD dataFileAddr,WORD commandOffset, WORD cplLen);

extern BYTE CheckMinimumSecurityLevel(BYTE spi,BYTE *miniSPIBuf);

extern void CB_Download_Hanlder(void);

extern WORD NativeProcessRemoteAppManageAPDU(void);

extern void ToolkitClearDifTotalData(BYTE reference, BYTE total, BYTE *tpoa);

extern WORD RemoteAPDUProcessForHttp(UWORD fileAddr,WORD commandOffset, WORD cplLen, BYTE block);

extern BYTE ProcessExtendAPDUForHttp(UWORD fileAddr,WORD commandOffset, WORD cplLen, BYTE block);

extern BYTE ProcessCompactAPDUForHttp(UWORD fileAddr,WORD commandOffset, WORD cplLen, BYTE block);

extern BYTE BackDFNameForMoveFalsh(WORD DFFileName[4]);

extern BYTE HttpProcessRAMinit(void);

extern void HttpDataMoveForward(UWORD SrcfileAddr,WORD Offset, WORD cplLen, BYTE mode);

extern BYTE RemoteApduResultForHttp(UWORD fileAddr,WORD commandOffset,WORD dataLen,WORD ReMainLen,BYTE RemoteResult);

extern void ReloadDFOrEFAddr(WORD fileName[4]);

extern BYTE ProcessRemoteCheckCAPDU(WORD cplLen);

extern WORD getCurrentAdfName(void);

extern void HttpApduPor(void);

extern BYTE  HTTPRAMRFMExtendResponse(BYTE tag);

extern BYTE GetBlockNumber(WORD length,WORD size);

extern BYTE GetSCP80KeyType(BYTE keyinfo,BYTE keytype,BYTE keyDESorAES);

extern BYTE GetAESMACLen(BYTE keyinfo,BYTE keytype);

extern void AESDeChipher(UWORD srcDstAddr,BYTE srcDstOff, BYTE aesMod, WORD lgth,BYTE kic);

extern BYTE GetCcLength(BYTE spi1,BYTE kid);

extern BYTE ToolkitDeChipherEe(BYTE kic,UWORD srcDstAddr,WORD srcDstOff,BYTE mode,WORD lgth);

extern BYTE ToolkitFindRFMCSMS_Head(BYTE mode ,BYTE refrence,BYTE total,BYTE smsType,BYTE *tpoa);

extern BYTE CheckSPIRuleForPor(BYTE * spikickid);

extern void RFMHandlerD0(BYTE num);
#endif
