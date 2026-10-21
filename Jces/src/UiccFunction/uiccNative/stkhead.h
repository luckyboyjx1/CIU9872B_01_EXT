#ifndef    _STKHEAD_H   
#define	   _STKHEAD_H 
#include    "typedef.h"
#include    "uicc_main.h"

extern BYTE	 xdata  G_STKBusy;

extern WORD   xdata OTATempVarAllLength;

extern BYTE xdata G_FindCH;

#define DecToAsc(x)   x += (x < 0x0A) ? 0x30:0x37;


//#define    OTA_APP_TYPE        0x08
///////////////////////PageListInformation TYPE///////////////////

#define 	TYPE_App_Close			    0x04   
#define 	TYPE_Recommend_List			0x01
#define     TYPE_Catalog_List           0x10
#define 	TYPE_Search_Result			0x05
#define		TYPE_Group_List				0x06
#define		TYPE_GroupMember_List		0x07
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

//-------------------menu type-------------------------------------
#define		MenuType_OTA		0x04 			//OTA目录
#define		MenuType_SOLID	    0x06			//完全固化目录

//-----------------------------------------------------------------------
//descript of wib sub function
//-----------------------------------------------------------------------

#define TEMP_BUF_POS1			0xA0
#define TEMP_BUF_POS2			0xC0
//------------------------------------------------------------------------
extern BYTE  xdata G_StkStatus;  

extern BYTE  xdata G_StoreName;

extern UWORD xdata G_StkCommandAddr;

extern UWORD xdata G_AppStartAddr;

extern WORD  xdata G_VARIABLECONTROL; 

extern WORD  xdata CommandStartNum;

extern BYTE  xdata currentComNUM;

extern BYTE  xdata  G_SelectedNum;

extern BYTE  xdata  G_CounterType;

extern BYTE  xdata  G_FileNum;	

extern BYTE  xdata  G_LastNum;

extern BYTE xdata STKControl;


#define		D_PRLDisplayTag			0xB7

#define     BIPModule_DataAvaiable_Alert     0xE4
#define		BIPModule_Channelstatus_Alert			0xE5
#define     TimerModule_TimerExpiration_Alert         0xE6
#define 	D_SCP81TriggerTag					0xE7



//-------------------------------------------------------------------
//define the comamnd qualify of refresh command
//-------------------------------------------------------------------
#define     REFRESH_MODE_UICC_RESET                        0x04
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
#define	CMD_GETINPUT2				0x33
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
#define 	SMS_LIST_DISPLAY	0x0c     // for display sms list 

//---------------------------------------------------------------------
#define     VpSuccess           0x70
#define     VpFail              0x71
#define	    ContinuPerform  	0xF0
#define     ExitStkPerform      0xA0
#define     BackMove         	0xFB
#define		READVARIBLE      	0x90
#define     PackageLgth         0x8C     //0x8C=140 BYTE, if the UDL > PackageLgth then seperate it to concatenated packages
#define     POP_OK              0x00
#define     StkOk               0x00

#ifdef LOCALOTA
#define		D_NextPageItem		    0x4F
#define     D_PrePageItem           0x4B
#else
#define		D_NextPageItem		    0xFF
#define     D_PrePageItem           0xFE
#endif


#define		D_SPBNextPageItem		0xFF
#define		D_SPBPrePageItem		0xFE


#define     DEFAULTALPHABET     0x80
#define     BINARYDATA          0x81
#define     UCS2ENCODE          0x82

//-----------------------------Registry Bit Number--------------------------

#define		Function134Num						0x09
#define		YouXuanManYou						0x0A
#define		SwitchNumBefore					0x0B
#define		SwitchNumAfter						0x0C
#define		ReadAll								0x0D
#define		DoubleADN							0x0E
#define		DoubleSMS							0x0F
#define		DisablePIN2							0x10
#define		MobileOTA							0x11
#define		UnicomOTA							0x12
#define		NoOTARFMReport						0x13
#define		OTAOpenDisplay						0x15
#define		OTANew6FFE						0x16
#define		OpenPorject							0x17
#define		ZJBrandType							0x18
#define		JSBrandType							0x19
#define		OTASetUpCallExit						0x1A
#define		OTAMenuSequence					0x1B
#define		OTARegistrationWhenEntryRecommend	0x1C
#define		OTARecommendAvaiable 				0x1D
#define		OTAMenuManagementShowLastPage	0x1E


#define    	D_UnicomRFMSucessTag	    0xC0


#define		D_RFMSucessTag			    0xA3



//BER-TLV tags in ME to SIM direction

#define TLV_SMSPPDOWNLOAD						0xD1
#define TLV_CELLBROADCASTDOWNLOAD				0xD2
#define TLV_MENUSELECTION						0xD3
#define TLV_CALLCONTROL							0xD4
#define TLV_MOSHORTMESSAGECONTROL				0xD5
#define TLV_EVENTDOWNLOAD						0xD6
#define TLV_TIMEREXPIRATION						0xD7
#define TLV_RESERVED_DF							0xDF

//SIM-TLV tags in SIM to ME direction
#define TLV_PROACTIVESIMCOMMAND					0xD0
#define TLV_CR								    0x80
#define TLV_INVERT_CR							0x7F
#define TLV_COMMANDDETAIL						0x81
#define TLV_DEVICEIDENTITY						0x82
#define TLV_RESULT							    0x83
#define TLV_DURATION							0x84
#define TLV_ALPHAIDENTITFIER					0x85
#define TLV_ADDRESS								0x86
#define TLV_CAPABILITYCONFIGURATIONPARAMETER	0x87
#define TLV_SUBADDRESS							0x88
#define TLV_SSSTRING							0x89
#define TLV_USSDSTRING							0x8A
#define TLV_SMSTPDU								0x8B
#define TLV_CELLBROADCASTPAGE					0x8C
#define TLV_TEXTSTRING							0x8D
#define TLV_TONE								0x8E
#define TLV_ITEM								0x8F
#define TLV_ITEMIDENTIFIER						0x90
#define TLV_RESPONSELENGTH						0x91
#define TLV_FILELIST							0x92
#define TLV_LOCATIONINFOMRATION					0x93
#define TLV_IMEI								0x94
#define TLV_HELPREQUEST							0x95
#define TLV_NETWORKMEASUREMENTRESULT			0x96
#define TLV_DEFAULTTEXT							0x97
#define TLV_ITEMNEXTACTIONINDICATOR				0x98
#define TLV_EVENTLIST							0x99
#define TLV_CAUSE								0x9A
#define TLV_LOCATIONSTATUS						0x9B
#define TLV_TRANSACTIONIDENTIFIER				0x9C
#define TLV_BCCHCHANNELLIST						0x9D
#define TLV_ICONIDENTIFIER						0x9E
#define TLV_ITEMICONIDENTIFIER					0x9F
#define TLV_CARDREADERSTATUS					0xA0
#define TLV_CARDATR								0xA1
#define TLV_CAPDU								0xA2
#define TLV_RAPDU								0xA3
#define TLV_TIMERIDENTIFIER						0xA4
#define TLV_TIMERVALUE							0xA5
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
#define TLV_URL									0xB1
#define TLV_BEARER								0xB2
#define TLV_PROVISIONINGFILEREFERENCE			0xB3
#define TLV_BROWSERTERMINATIONCAUSE				0xB4
#define TLV_BEARERDESCRIPTION					0xB5
#define TLV_CHANNELDATA							0xB6
#define TLV_CHANNELDATALENGTH					0xB7
#define TLV_CHANNELSTATUS						0xB8
#define TLV_BUFFERSIZE							0xB9
#define TLV_CARDREADERIDENTIFIER				0xDA
#define TLV_RESERVED_3B							0xDB
#define TLV_SIMMEINTERFACETRANSPORT				0xBC
#define TLV_NETWORKACCESSNAME                   0xC7
#define TLV_RESERVED_3D							0xDD
#define TLV_OTHERADDRESS						0xDE
#define TLV_RESERVED_60							0x60
#define TLV_RESERVED_61							0x61

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

//Result
#define CMD_Success								0x00
#define CMD_SUCCESS							    0x00
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


//ADN PLUGIN
#define     ADNITEMNUM			0x08			//the rec num displayed in each page
#define     ADNITEMFULL         (ADNITEMNUM+2)
//RDI PLUGIN
#define     PPFUCSEPARATOR		0x2C           //the list separator of PPDownload information for Zone Location

////////////////////////////////////////////////////////////////////////////////
#define ALL_AID         0x01     //联通 菜单使用次数通知  上发所有已经下载菜单的
#define DEL_AID 		0x02	 //联通 菜单使用次数通知  上发要删除的菜单
///////////////////////////////////////////////////////////////////////////////
#define AidCntINSERT 	0x01
#define AidCntDELETE 	0x02
#define AidCntINCREASE	0x03
#define AidCntCLEAR	 	0x04



#define		D_SetUpMenuTag			0xA2

#define ADDR_IS_NULL         		DATA_BASE_LOGIC  //地址是空的
#define  DATA_BASE_LOGIC	      ((UWORD)0x00000000)  //数据区逻辑地址起始
//------------------------------------------------------------------------





#define BuildWord(buf)    ((WORD)(((WORD)(buf)[0]<<8)|(buf)[1]))
#define SetWord(buf,value)	do{(buf)[0]=((value)>>8)&0xFF; (buf)[1]=(value)&0xFF; }while(0)

extern void SetWordToBuffer(WORD len,BYTE * buf);
extern UWORD ByteBufToUWORD(BYTE *buf);
extern void UWORDToByteBuf(UWORD value, BYTE *buf);


extern WORD CallContralByUSIM(void);

extern BYTE EventDownload(void);

extern void SetUpMenu(void);

extern 	BYTE GetItemIDinEFMenu( BYTE * output, BYTE type);

extern BYTE GetItemIDinEFMenuTelecom(UWORD fileAddr, BYTE * output);

extern BYTE MenuSelection(void);


extern void PP_Download_Hanlder(void);




extern void USSDDataDownload(void);

#define Mode_Pending	0x01
#define Mode_Clearing	0x02

//extern void PendingMessageAlert(BYTE ch);

extern void PendingMessageAlert(BYTE ch,BYTE refNumber);

extern void ClearingMessageAlert(BYTE ch);

extern void ProcessPendingMessageAlert(void);

extern BYTE SendProactiveCommand(void);

extern BYTE GetResponseResult(void);

extern BYTE GetResResult(void);

extern BYTE GetResponseItemIdentifier(void);

extern void DisplayWarningMessage(void);

extern void InitProactiveCommand(BYTE commandType,BYTE commandQlify,BYTE desDevice);

extern BYTE AddTLV(BYTE *TLV_AIM,BYTE  * TLV_OBJECT);

extern BYTE AddT_L_V(BYTE * TLV_AIM, BYTE T, BYTE L, BYTE* Value);

extern BYTE AddProDataVarilbe(UWORD alpha_addr,BYTE structure_tag,BYTE item,BYTE language);

extern BYTE ClearVariableBuffer(BYTE mode) ;

extern BYTE AddValue(BYTE *TLV_AIM, BYTE * LV_OBJECT);

extern WORD LeachVarible(UWORD bytecodeAddr,UWORD EepromAddr,BYTE * tempbuf);

extern BYTE GetRegistryBitStatus(BYTE bitNum);

//----------------------------------------------------------------------------------------------------
extern void  InitStack(void) ;

extern void  PushStack(UWORD stackAddr);

extern UWORD PopStack(void);

extern void  InitVariable(void);

extern WORD GetVaribleAddr(BYTE variablename);

extern BYTE WriteVariable(BYTE variablename,BYTE * buffer);

extern BYTE ClearVariable(BYTE varId);

extern BYTE ReadVarible(BYTE variablename,BYTE * buffer);

extern BYTE InternalConvertVarID(BYTE editBufIndex);

//------------------------	TransFormat.c	编码转换	----------------------------------------------------------------------------
#define	HextoASCii	0x01
#define	BCDtoASCii	0x02
#define	ASCiitoHex	0x03
#define	ASCiitoBCD	0x04

#define TYPE_NORMAL 0x05
#define TYPE_USSD	0x06

extern BYTE CharacterPack(BYTE * pDataStr, BYTE Lgth, BYTE type);

extern void CharacterUnPack(BYTE  * pDataStr,BYTE  * pResultStr,BYTE Lgth, BYTE fillBits);

extern BYTE AsciiToUCS2(BYTE * dataBuffer,BYTE * resultBuffer);

extern BYTE UCS2ToAscii(BYTE * src, BYTE * dest);

extern BYTE ADNToAscii(BYTE * input, BYTE * output);

extern BYTE AsciiToADN(BYTE * input, BYTE * output);

extern BYTE IntToChar(BYTE i, BYTE mode);

extern BYTE CharToInt(BYTE i, BYTE mode);


extern BYTE IntToString(BYTE * lv_src, BYTE * lv_dst);

extern BYTE StringToInt(BYTE * lv_src, BYTE * lv_dst);

extern BYTE HexToUCS2(BYTE src, BYTE * lv_dst);

extern BYTE TenToSixteen(BYTE * lvString, BYTE * outPut);

extern BYTE SixteenToTen(WORD src, BYTE * buf);

//extern BYTE HttpSixteenToTen(WORD src, BYTE * buf);

extern void NumToHEX(DWORD src,BYTE *buf);

extern void reverse(BYTE * buf);

extern BYTE IsASCIIString( BYTE* buf);

extern BYTE keyData_getRFMKey(BYTE wantkeyVersion, BYTE keyID, BYTE * keyValueBuf);

//------------------------	stkSunFunction.c	------------------------------------------------------------------

extern BYTE LoadSCAddress(BYTE * buf);

extern BYTE LocateRecordInFile(UWORD fileAddr, BYTE tag);

extern WORD GetTagPosInTLVFile(UWORD fileAddr, BYTE tag);

extern BYTE TransPhoneAdnFormat(BYTE * input);

extern BYTE GetBCDCount(BYTE * buf, BYTE len);

extern BYTE GetValueInTLVFile(UWORD fileAddr, BYTE tag, BYTE * buf);

extern BYTE GetLabelStr(BYTE index, BYTE * buf);

extern BYTE AddLabelToProCommand(BYTE index,BYTE tag,BYTE item,BYTE * buf);

extern void GetResponseTextString(BYTE commQualify,BYTE * buf,BYTE maxLength);

extern BYTE ReadADNname(UWORD adnAddr, BYTE recIndex,BYTE * dataBuf);

extern BYTE UnicomReadADNname(UWORD adnAddr, BYTE recIndex,BYTE * dataBuf);

extern BYTE ReadSmsData(BYTE * dataBuf, BYTE timeTag, BYTE numTag, BYTE dataTag); 

extern BYTE GetSmsData(BYTE * dataBuf, BYTE dataTag);

extern void GetSMSAlpha(BYTE * buf,BYTE alphaLen);

extern void UnicomGetSMSAlpha(BYTE * buf,BYTE alphaLen);

extern BYTE GetADNAlphaLen(BYTE * dataBuffer,BYTE maxLength);

extern void OTASequenceMenu(BYTE IsCanSee);

extern void SortApplicationID(BYTE * list,BYTE mode);

extern void SetUpEventList(void);

extern void IncreaseCounter(BYTE *cntr,BYTE counterLen);

//-------------------------一卡多号------------------------------------
extern BYTE SelectNumber(void);

extern BYTE TDNameModify(BYTE groupId,BYTE * buf,BYTE dcs);

extern BYTE NameModify(BYTE groupId,BYTE textId,BYTE dcs,BYTE len);

extern BYTE SwitchActivePhone(BYTE phoneNum,BYTE mode);

extern void DoubleNumber(void);



//-----------------------------位置服务--------------------------------
extern BYTE LocalInfoPPDownload(BYTE pos);

extern void LocalInfoHandler(void);

//----------------------------短信签名---------------------------------
extern BYTE SignManage(void);

extern BYTE GetSignatureConfigByte(BYTE offset);

extern BYTE SetSignatureConfigByte(BYTE offset, BYTE status);

extern BYTE SelectItemSigne(void);

extern BYTE AddSignatureRecord(BYTE varId, BYTE index, BYTE mode);

extern BYTE AddSMSSignature(BYTE dcs, BYTE tag);

extern void AddDisplaySMS(BYTE *dcs);

extern void PendingMessageAlert(BYTE ch,BYTE refNumber);

extern void ProcessPendingMessageAlert(void);

//----------------------------------------------------------
#define toDWORD(x)		((DWORD)(x))

#define toUWORD(x)		((UWORD)(x))

#define toWORD(x)		((WORD)(x))

#define toBYTE(x)		((BYTE)(x))


#define toULong(x)		((unsigned long)(x))
#define toUChar(x)		((unsigned char)(x))
#define toUShort(x)		((unsigned short )(x))

#define toPointer(x)		((void *)(x))
//----------------------------------------------------------
#endif
