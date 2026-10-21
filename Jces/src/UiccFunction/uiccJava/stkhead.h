#ifndef    _STKHEAD_H   
#define	   _STKHEAD_H 
#include    "typedef.h"
#include    "uicc_main.h"

extern BYTE  xdata G_StkStatus;  

extern BYTE  xdata G_StoreName;

extern UWORD xdata G_StkCommandAddr;

extern UWORD xdata G_AppStartAddr;

extern WORD  xdata G_VARIABLECONTROL; 

extern BYTE  xdata currentComNUM;


#define	OFFSET_NULL_VARIABLE        	0xFFFF
//------------------------------------------------------------------
//define the command type of proactive command
//------------------------------------------------------------------
#define	CMD_REFRESH					0x01
#define	CMD_SETUPEVENT      			0x05
#define	CMD_SETUPCALL				0x10
#define	CMD_SENDSS           			0x11
#define	CMD_SENDUSSD				0x12
#define	CMD_SENDSMS				0x13
#define	CMD_SENDDTMF				0x14    
#define	CMD_LAUNCHBROWSER    		0x15
#define	CMD_PLAYTONE				0x20
#define	CMD_DISPLAY					0x21
#define	CMD_GETINKEY				0x22
#define	CMD_GETINPUT				0x23
#define	CMD_SELECTITEM   			0x24
#define	CMD_SETUPMENU				0x25
#define	CMD_PLINFO					0x26
#define	CMD_TIMERMANAGE				0x27
#define	CMD_SUIMT					0x28
#define	CMD_OPENCHANNEL			0x40
#define	CMD_CLOSECHANNEL			0x41
#define	CMD_RECEIVEDATA			0x42
#define	CMD_SENDDATA				0x43
#define	CMD_GETCHANNELSTATUS		0x44
//-------------------------------------------------------------------
//define the comamnd qualify of proactive command
//-------------------------------------------------------------------
#define	   	Q_SETUPMENU			0x00
#define     Q_SENDSS            0x00
#define    	Q_SENDUSSD			0x00
#define    	Q_SENDSMS			0x01
#define     Q_SENDSMS_NOTPACKED 0x00
#define    	Q_PLAYTONE			0x00
#define     Q_SETUPCALL         0x00
#define	   	Q_GETINPUT      	0x01
#define    	Q_DISPLAY			0x81                    
#define     Q_DISPLAY_CLEAR     0x01
#define    	Q_SELECTITEM   		0x00
#define    	Q_SUIMT				0x00
#define     Q_SETUPEVENT        0x00
#define     Q_SENDDTMF		    0x00
#define     DISPLAYERROR        0x01
#define     PLUGINPROMPT        0x02
#define     RPALGORSHA1         0x01
//--------------------------------------------------------------------
//define   DEVICE IDENTITIES OF PROACTIVE COMMAND
//--------------------------------------------------------------------
#define	   	DEV_KEYPAD			0x01
#define	   	DEV_DISPLAY			0x02
#define    	DEV_EARPIECE		0x03
#define	   	DEV_SIM				0x81
#define	   	DEV_ME				0x82
#define	   	DEV_NETWORD			0x83
//---------------------------------------------------------------------
#define		MAX_LENGTH        	0xFD

#define     PackageLgth         	0x8C  


#define  	ContinuPerform  	    	0xF0
#define  	BackMove					0xFB
#define     ExitStkPerform      		0xFC
#define     ExitStkPerformNoGoonexit 	0xF8


//-------------------------------------------------------------------
//define the comamnd qualify of refresh command
//-------------------------------------------------------------------
#define     REFRESH_MODE_UICC_RESET                        0x04
#define     REFRESH_MODE_PROFILE_STATE_CHANGE              0x09
///////////////////////PageListInformation TYPE///////////////////

#define 	TYPE_App_Name   		    0x02   //	0x22
#define 	TYPE_Menu_Dele  	    	0x08   //	0x28
#define 	TYPE_App_Close			    0x04   //	0x24
#define 	TYPE_Download_List			0x15
#define 	TYPE_Recommend_List			0x01
#define     TYPE_Catalog_List           0x10
#define 	TYPE_Search_Result			0x05
#define 	TYPE_Adn_List				0x00
#define		TYPE_Group_List				0x06
#define		TYPE_GroupMember_List		0x07
#define 	TYPE_Sms_List				0x03
#define		TYPE_Receiver_List			0x09
#define		TYPE_LMSJ_Recommend_List 	0x0A
#define		TYPE_LMSJ_Collection_List 	0x0B
#define		TYPE_FetionApp_Name			0x0C
#define		TYPE_PBookStart_List		0x0D
#define		TYPE_PBookHide_List			0x0E
#define		TYPE_PBookAll_List			0x0F
#define		TYPE_PBookAllClear_List		0x11
#define		TYPE_PBook_List				0x12
#define		TYPE_Signture_List			0x13
#define     TYPE_GroupStart_List        0x14
#define     TYPE_Catalog_Dele           0x16


#define G_SimImformationMenuId     0x4E      //移动sim卡信息  二级菜单
#define D_NativeMeun    0xC0      //移动本地菜单 开始标示符
#define D_PrePageItem  0xBE
#define D_NextPageItem  0xBF   //


//////////////////////////STK Alert Tag Definition/////////////////////////////////
#define		D_OTANoFreeSizeTag			0x03		//OTA提示下载空间不足
#define		D_OTAUserRegistTag			0x04		//OTA提示用户注册成功
#define		D_DownListUpdateTag			0x0F		//OTA提示下载列表已经更新
#define		D_DownListDeleteTag			0x10		//OTA提示下载列表已经清空
#define		D_PushListClearTag			0x11		//OTA提示Push列表已经清空
#define		D_OTAPushUpdateTag			0x12		//OTA提示Push类别已经更新
#define		D_InterDownSucessTag		0xA0		//OTA交互应用下载成功
#define		D_OTASynchronizTag			0xA1		//OTA信息同步操作
#define		D_SetUpMenuTag				0xA2		//重新建立STK菜单操作
#define		D_RFMSucessTag			    0xA3		//OTARFM接收完成
#define		D_WuYouADNBackUpTag			0xA5		//无忧电话本备份操作
#define		D_WuYouSmsBackupTag			0xA6		//无忧短信备份操作
#define		D_WuYouADNRecoverTag		0xA7		//无忧电话本恢复操作
#define		D_BrandChangeConfirmTag		0xA8		//更新成功上行确认信息
#define		D_OpenDisplayTag			0xA9		//显示开机问候语
#define		D_ZJBrand_Confirm			0xAA		//浙江品牌协议身份同步应答确认
#define		D_OTARegisterTag			0xAB		//OTA上发注册信息操作
#define		D_BrandUpdateTag			0xAC		//品牌更新命令上发操作
#define		D_DataDownSucessTag			0xAD		//OTA应用下载完成
#define		D_LocalInforTag				0xAE		//位置服务上行操作
#define		D_MenuSearchTag				0xAF		//OTA菜单搜索命令上发操作
#define		D_OTABigDataResultTag		0xB0		//OTA大数据接受结果上发操作
#define		D_WuYouBigDataResultTag		0xB1		//无忧大数据接受结果上发操作
#define		D_WuYouFreeSpaceTag			0xB2		//无忧剩余空间提示
#define		D_RefreshTag				0xB3		//Refresh命令操作
#define		D_WIBSucessTag				0xB4		//WIB下行成功
#define		D_WIBRFMProactiveCommandTag	0xB5		//RFM中存在主动式命令的操作
#define		D_WuYouRegeisterTag			0x21		//OTA提示无忧已经注册成功
#define		D_BipPushOpen				0xB6		//BIP open
#define		D_BipPushClose				0xB7		//BIP close
#define		D_EventProcess				0xB8		//event need process
#define		D_TimerExpirationProcess	0xB9		//Timer Expiration need process
#define		D_CloseChannelTag			0xBA
#define		D_DataAvaiable				0xBB
#define		D_EventDataDownload			0xBC
#define		D_OTASetUpEventList			0xCD
#define		D_GDOTAEventRun				0xBE       
#define		D_ErrorDisplayTag  		    0xC0        //Display error Info
#define		D_BrandRegisterTag			0x2B		//上发品牌注册信息操作
#define		D_OTAAppDelNoticeTag	    0xC4        //远程业务删除确认标记
#define     D_DirectoryNoticeTag        0xC5        //远程目录管理确认标记
#define     D_OTAAppOpenCloseTag        0xC6        //应用开通，关闭确认信息
#define		D_PopUpAppliction		    0xC7        //弹出应用
#define     D_UpdateDowloadConfirm      0xC8        //MT应用下载和更新确认
#define     D_PushListConfirm           0xC9        //最新推荐列表确认
#define     D_DownListUpdateConfirm     0xCA         //下载列表更新确认
#define     D_DisableEnbleAppConfirm    0xCB        //应用启用/禁用
#define     D_UpdateIPSmsNumber         0xBD        //远程更新IP地址或短信接入号
#define     D_DisplayMenuManagement     0xCE       //远程目录管理提示信息
#define		D_ChannelStatus				0xE5
#define     D_PRLDisplayTag			    0xE8
#if COS_UPDATE_ONLINE_MUTUAL
#define    D_UPCOS_ACTIVEREMIND        0xE9
#define    D_UPCOS_VERSIONAFFIRM       0xEA
#endif
//-------------SUPI -----------//

#define		D_RefreshUpdateRoutingID				0xE7
//-------------------menu type-------------------------------------
#define		MenuType_OTA		0x04 			//OTA目录
#define		MenuType_SOLID	    0x06			//完全固化目录


//////////////////////////STK Alert Tag Definition/////////////////////////////////

#define		D_BipPushOpen				0xB6		//BIP open
#define		D_BipPushClose				0xB7		//BIP close
#define		D_EventProcess				0xB8		//event need process
#define		D_TimerExpirationProcess	0xB9		//Timer Expiration need process
#define		D_CloseChannelTag			0xBA
#define		D_DataAvaiable				0xBB
#define		D_EventDataDownload			0xBC
#define		D_GDOTAEventRun				0xBE       
#define		D_RFMDataAvaiable			0xCF
#define     D_RFMChannelStatus          0xD0

#define  	BIPModule_DataAvaiable_Alert        0x40
#define   	BIPModule_Channelstatus_Alert       0x41

#define 	D_SCP81TriggerTag                   0x43   
#define 	D_SCP81TimerTag						0x44
#define 	D_SCP81ReceiveTag					0x45

//BER-TLV tags in ME to SIM direction

#define TLV_SMSPPDOWNLOAD						0xD1
#define TLV_CELLBROADCASTDOWNLOAD				0xD2
#define TLV_MENUSELECTION						0xD3
#define TLV_CALLCONTROL							0xD4
#define TLV_MOSHORTMESSAGECONTROL				0xD5
#define TLV_EVENTDOWNLOAD						0xD6
#define TLV_TIMEREXPIRATION						0xD7
#define TLV_RESERVED_DF							0xDF

//--------------------------------------------------------------------------------------
//SIM-TLV tags in SIM to ME direction
#define TLV_PROACTIVESIMCOMMAND					0xD0
#define TLV_CR								    0x80
#define TLV_INVERT_CR							0x7F
#define TLV_COMMANDDETAIL						0x81
#define TLV_DEVICEIDENTITY						0x82
#define TLV_RESULT							    0x83
//#define TLV_DURATION							0x84
//#define TLV_ALPHAIDENTITFIER					0x85
#define TLV_ADDRESS								0x86
//#define TLV_CAPABILITYCONFIGURATIONPARAMETER	0x87
#define TLV_SUBADDRESS							0x88
#define TLV_SSSTRING							0x89
//#define TLV_USSDSTRING							0x8A
#define TLV_SMSTPDU								0x8B
#define TLV_CELLBROADCASTPAGE					0x8C
#define TLV_TEXTSTRING							0x8D
//#define TLV_TONE								0x8E
#define TLV_ITEM								0x8F
#define TLV_ITEMIDENTIFIER						0x90
//#define TLV_RESPONSELENGTH						0x91
//#define TLV_FILELIST							0x92
#define TLV_LOCATIONINFOMRATION					0x93
#define TLV_IMEI								0x94
#define TLV_HELPREQUEST							0x95
#define TLV_NETWORKMEASUREMENTRESULT			0x96
//#define TLV_DEFAULTTEXT							0x97
#define TLV_ITEMNEXTACTIONINDICATOR				0x98
//#define TLV_EVENTLIST							0x99
#define TLV_CAUSE								0x9A
#define TLV_LOCATIONSTATUS						0x9B
#define TLV_TRANSACTIONIDENTIFIER				0x9C
#define TLV_BCCHCHANNELLIST						0x9D
//#define TLV_ICONIDENTIFIER						0x9E
//#define TLV_ITEMICONIDENTIFIER					0x9F
#define TLV_CARDREADERSTATUS					0xA0
#define TLV_CARDATR								0xA1
#define TLV_CAPDU								0xA2
#define TLV_RAPDU								0xA3
//#define TLV_TIMERIDENTIFIER						0xA4
//#define TLV_TIMERVALUE							0xA5
#define TLV_DATETIMEANDTIMEZONE					0xA6
#define TLV_CALLCONTROLREQUESTACTION			0xA7
#define TLV_ATCOMMAND							0xA8
#define TLV_ATRESPONSE							0xA9
#define TLV_BCREPEATINDICATOR					0xAA
#define TLV_IMMEDIATERESPONSE					0xAB
#define TLV_DTMFSTRING							0xAC
#define TLV_LANGUAGE							0xAD
#define TLV_TIMINGADVANCE						0xAE
#define TLV_RESERVED_2F							0xAF
#define TLV_BROWSERIDENTITY						0xB0
//#define TLV_URL									0xB1
//#define TLV_BEARER								0xB2
//#define TLV_PROVISIONINGFILEREFERENCE			0xB3
#define TLV_BROWSERTERMINATIONCAUSE				0xB4
#define TLV_BEARERDESCRIPTION					0xB5
#define TLV_CHANNELDATA							0xB6
#define TLV_CHANNELDATALENGTH					0xB7
#define TLV_CHANNELSTATUS						0xB8
#define TLV_BUFFERSIZE							0xB9
#define TLV_TERMINTERTRAN						0xBC

#define TLV_ACCESSTECHNOLOGY					0xBF
#define TLV_CARDREADERIDENTIFIER				0xDA
#define TLV_RESERVED_3B							0xDB
#define TLV_SIMMEINTERFACETRANSPORT				0xBC
#define TLV_NETWORKACCESSNAME                   0xC7
#define TLV_RESERVED_3D							0xDD
#define TLV_OTHERADDRESS						0xDE
#define TLV_RESERVED_60							0x60
#define TLV_RESERVED_61							0x61



//#define TLV_NETWORKACCESSNAME					0xD7
//#define TLV_PLMNWACTLIST						0x72

#define TLV_DURATION							0x84
#define TLV_ALPHAIDENTITFIER					0x85
#define TLV_CAPABILITYCONFIGURATIONPARAMETER	0x87
#define TLV_USSDSTRING							0x8A
#define TLV_TONE								0x8E
#define TLV_RESPONSELENGTH						0x91
#define TLV_FILELIST							0x92
#define TLV_DEFAULTTEXT							0x97
#define TLV_EVENTLIST							0x99
#define TLV_ICONIDENTIFIER						0x9E
#define TLV_ITEMICONIDENTIFIER					0x9F
#define TLV_TIMERIDENTIFIER						0xA4
#define TLV_TIMERVALUE							0xA5
#define TLV_URL									0xB1
#define TLV_BEARER								0xB2
#define TLV_PROVISIONINGFILEREFERENCE			0xB3


//-------------------------------------------------------------------------------------
//Type of command and next action indicator
#define CMD_REFRESH								0x01
#define CMD_MORETIME							0x02
#define CMD_POLLINTERVAL						0x03
#define CMD_POLLINGOFF							0x04
#define CMD_SETUPEVENTLIST						0x05
#define CMD_SETUPCALL							0x10
#define CMD_SENDSS								0x11
#define CMD_SENDUSSD							0x12
#define CMD_SENDSHORTMESSAGE					0x13
#define CMD_SENDDTMF							0x14
#define CMD_LAUNCHBROWSER						0x15
#define CMD_PLAYTONE							0x20
#define CMD_DISPLAYTEXT							0x21
#define CMD_GETINKEY							0x22
#define CMD_GETINPUT							0x23
#define CMD_SELECTITEM							0x24
#define CMD_SETUPMENU							0x25
#define CMD_PROVIDELOCALINFORMATION				0x26
#define CMD_TIMERMANAGEMENT						0x27
#define CMD_SETUPIDLEMODELTEXT					0x28
#define CMD_PERFORMCARDAPDU						0x30
#define CMD_POWERONCARD							0x31
#define CMD_POWEROFFCARD						0x32
#define CMD_GETREADERSTATUS						0x33
#define CMD_RUNATCOMMAND						0x34
#define CMD_LANGUAGENOTIFICATION				0x35
#define CMD_OPENCHANNEL							0x40
#define CMD_CLOSECHANNEL						0x41
#define CMD_RECEIVEDATA							0x42
#define CMD_SENDDATA							0x43
#define CMD_GETCHANNELSTATUS					0x44
#define CMD_RESERVED_60							0x60
#define CMD_ENDOFPROACTIVESESSION				0x81

/*****************************Other CMD**********************************************/
#define   CMD_DECLARE_SERVICE	                 0x47


//Result
#define CMD_Success								0x00
#define CMD_PARTIALCOMPREHENSION				0x01
#define CMD_MISSINGINFORMATION					0x02
#define CMD_REFRESHWITHADDITIONALEFREAD			0x03
#define CMD_SUCCESSWITHOUTICON					0x04
#define CMD_SUCCESSWITHCALLCONTROLATSIM			0x05
#define CMD_SUCCESSWITHLIMITEDSERVICE			0x06
#define CMD_PERFORMEDWITHMODIFICATION			0x07
#define CMD_USERTERMINATESESSION				0x10
#define CMD_BACKWARDMOVE						0x11
#define CMD_NORESPONSE							0x12
#define CMD_REQUESTHELPINFO						0x13
#define CMD_TRANSACTIONTERMINATED				0x14

#define CMD_MEUNABLEPROCESS						0x20
#define CMD_NETWORKUNABLE						0x21
#define CMD_USERNOTACCEPT						0x22
#define CMD_USERCLEARDOWN						0x23
#define CMD_ACTIONCONTRADICTION					0x24
#define CMD_TEMPORARYPROBLEM					0x25
#define CMD_GENERICERROR						0x26

#define CMD_BEYONDCAPABILITIES					0x30
#define CMD_UNKNOWNCOMMAND						0x31
#define CMD_UNKNOWNDATA							0x32
#define CMD_UMKNOWNNUMBER						0x33
#define CMD_SSRETURNERROR						0x34
#define CMD_SMSRPERROR							0x35
#define CMD_MISSINGVALUE						0x36
#define CMD_USSDERROR							0x37
#define CMD_MULTIPLECARDERROR					0x38
#define CMD_PERMANENTPROBLEM					0x39
#define CMD_PROTOCOLERROR						0x3A


//device
#define DEVICE_KEYPAD							0x01
#define DEVICE_DISPLAY							0x02
#define DEVICE_EARPIECE							0x03
#define DEVICE_CARDREADER00						0x10
#define DEVICE_CARDREADER01						0x11
#define DEVICE_CARDREADER02						0x12
#define DEVICE_CARDREADER03						0x13
#define DEVICE_CARDREADER04						0x14
#define DEVICE_CARDREADER05						0x15
#define DEVICE_CARDREADER06						0x16
#define DEVICE_CARDREADER07						0x17
#define DEVICE_CHANNEL1							0x21
#define DEVICE_CHANNEL2							0x22
#define DEVICE_CHANNEL3							0x23
#define DEVICE_CHANNEL4							0x24
#define DEVICE_CHANNEL5							0x25
#define DEVICE_CHANNEL6							0x26
#define DEVICE_CHANNEL7							0x27
#define DEVICE_SIM								0x81
#define DEVICE_ME								0x82
#define DEVICE_NETWORK							0x83

//-----------------------------stkSubFunction.c-------------------------------------------
extern signed char memcmp_p(BYTE * buf1,BYTE *buf2,BYTE lgth);

extern BYTE SendProactiveCommand(void);

extern BYTE GetResponseResult(void);

extern BYTE GetResResult(void);

extern BYTE GetResponseItemIdentifier(void);

extern void DisplayWarningMessage(void);

extern void InitProactiveCommand(BYTE commandType,BYTE commandQlify,BYTE desDevice);

extern BYTE AddTLV(BYTE *TLV_AIM,BYTE  * TLV_OBJECT);

extern BYTE AddT_L_V(BYTE * TLV_AIM, BYTE T, BYTE L, BYTE* Value);

//---------------------------------------variable.c-------------------------------------------------------------

extern void InitVariable(void);

extern WORD GetVaribleAddr(BYTE variablename,BYTE * len);

extern BYTE WriteVariable(BYTE variablename,BYTE * buffer);

extern BYTE ClearVariable(BYTE varId);

extern BYTE ReadVarible(BYTE variablename,BYTE * buffer);

//------------------------	TransFormat.c	编码转换	----------------------------------------------------------------------------
#define	HextoASCii	0x01
#define	BCDtoASCii	0x02
#define	ASCiitoHex	0x03
#define	ASCiitoBCD	0x04

#define TYPE_NORMAL 0x05
#define TYPE_USSD	0x06


extern void CharacterUnPack(BYTE  * pDataStr,BYTE  * pResultStr,BYTE Lgth, BYTE fillBits);

extern void AsciiToUCS2(BYTE * dataBuffer,BYTE * resultBuffer);

extern void UCS2ToAscii(BYTE * src, BYTE * dest);

extern BYTE ADNToAscii(BYTE * input, BYTE * output);

extern BYTE AsciiToADN(BYTE * input, BYTE * output);

extern BYTE IntToChar(BYTE i, BYTE mode);

extern BYTE CharToInt(BYTE i, BYTE mode);

extern BYTE IntToString(BYTE * lv_src, BYTE * lv_dst);

extern BYTE StringToInt(BYTE * lv_src, BYTE * lv_dst);

extern BYTE HexToUCS2(BYTE src, BYTE * lv_dst);

extern void TenToSixteen(BYTE * lvString, BYTE * outPut);

extern void NumToHEX(DWORD src,BYTE *buf);

extern void reverse(BYTE * buf);

extern BYTE IsASCIIString( BYTE* buf);

//------------------------	stkSunFunction.c	------------------------------------------------------------------
extern WORD GetTagPosInTLVFile(UWORD fileAddr, BYTE tag);

extern BYTE TransPhoneAdnFormat(BYTE * input);

extern BYTE GetValueInTLVFile(UWORD fileAddr, BYTE tag, BYTE * buf);

extern BYTE GetLabelStr(BYTE index, BYTE * buf);

extern BYTE AddLabelToProCommand(BYTE index,BYTE tag,BYTE item,BYTE * buf);

extern void GetResponseTextString(BYTE commQualify,BYTE * buf,BYTE maxLength);

extern BYTE ReadSmsData(BYTE * dataBuf, BYTE timeTag, BYTE numTag, BYTE dataTag); 

extern BYTE GetADNAlphaLen(BYTE * dataBuffer,BYTE maxLength);

extern void OTASequenceMenu(BYTE IsCanSee);

extern void SortApplicationID(BYTE * list,BYTE mode);

extern void IncreaseCounter(BYTE *cntr,BYTE counterLen);

extern BYTE AddValue(BYTE * LV_AIM,BYTE * LV_OBJECT);
//----------------------------------------------------------
#define toDWORD(x)		((DWORD)(x))

#define toUWORD(x)		((UWORD)(x))

#define toWORD(x)		((WORD)(x))

#define toBYTE(x)		((BYTE)(x))

#define toULong(x)		((unsigned long)(x))

#define toUChar(x)		((unsigned char)(x))

#define toUShort(x)		((unsigned short )(x))

#define toPointer(x)	((void *)(x))
//----------------------------------------------------------

#endif
