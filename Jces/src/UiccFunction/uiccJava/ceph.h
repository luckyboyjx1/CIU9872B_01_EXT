#ifndef __CEPH_H_
#define __CEPH_H_
#include "typedef.h"
#include "profile.h"

#define  UCS2       		0x80
#define  GSMCOMPRESSED 		0x00
#define  GSMDEFAULT			0x04


#define	SmsTag				0x09

#define D_MaxNameLen				20		//短信群发：群组名称最大长度
#define D_SmsItemMaxLen				14		//短信群发：短信列表时Item项的最大显示长度
#define D_GroupNameLen		(D_MaxNameLen+2)

extern BYTE cephDisplayTextSingle(BYTE comamndQulify,BYTE dcs,BYTE * textString);

extern BYTE cephSetupCallSingle(BYTE commQualify, BYTE dcs,BYTE * alphaIdentifier1, BYTE * address,BYTE * alphaIdentifier2);

extern BYTE cephSendShortMessageSingle(BYTE commQualify,BYTE  * alphaIdentifier, BYTE * address, BYTE * smsTPDU);

extern BYTE cephSelectItemSingle(BYTE commQualify, BYTE dcs,BYTE * alphaIdentifier, BYTE * itemDataObjects,BYTE itemLgth);

extern BYTE cephGetInputSingle(BYTE commQualify,BYTE varId,BYTE dcs, BYTE  * textString, BYTE minLength, BYTE maxLength,BYTE * defaultString);

extern BYTE cephRefreshSingle(BYTE commQualify, BYTE * fileList);

extern BYTE cephSetupEventListSingle(BYTE  * eventList);

extern BYTE cephReceiveDataSingle(BYTE channelId, BYTE dcs, BYTE * alpha, BYTE dataLgth);

extern BYTE cephCloseChannelSingle(BYTE commQualify, BYTE channel);

extern BYTE cephSendDataSingle(BYTE commQualify, BYTE channel, BYTE* dataBuf );

extern BYTE cephGetChannelStatusSingle(BYTE varId );

extern BYTE cephOpenChannelSingle(BYTE commQualify, BYTE* address, BYTE bearerType, BYTE  * bearer, BYTE * bufSize, BYTE * portNum, BYTE tpType,BYTE *  AccessName);

extern BYTE cephOpenChannel(BYTE commQualify, BYTE dcs, BYTE* alphaIdentifier, BYTE* address, BYTE bearerType, BYTE* bearer, WORD bufSize, BYTE* subAddress, BYTE* Duration1, BYTE* Duration2,  BYTE* localAddress, BYTE* NAN,  BYTE* login, BYTE* passWord, BYTE* protocol, BYTE* remoteAddr );

extern BYTE cephGetInkeySingle(BYTE commQualify,BYTE varId, BYTE dcs, BYTE  * textString);

extern BYTE cephMoreTimeSingle(void);

extern BYTE cephPlayToneSingle( BYTE dcs,BYTE * alphaIdentifier, BYTE tone, BYTE timeUint, BYTE timeInterval);

extern BYTE cephPollIntervalSingle(BYTE cmdType,BYTE timeUint, BYTE timeInterval);

extern BYTE cephProLocalInforSingle(BYTE varibleId,BYTE commQualify);

extern BYTE cephSendUssd(BYTE dcsAlpha,BYTE  * alphaIdentifier,BYTE dcsUssd,BYTE * ussdString);

//extern BYTE cephLaunchBrowser(BYTE commQualify,BYTE dcsUrl,BYTE * urlString,BYTE CONST * alphaIdentifier);
extern BYTE cephLaunchBrowser(BYTE commQualify,BYTE * urlString,BYTE  * alphaIdentifier);

extern BYTE cephSendSsSingle(BYTE dcs,BYTE  * alphaIdentifier,BYTE  * ssString);


//----------------USAT TEST----------------------//
#if  UICC_USATTEST
extern void USATGetInkey(void);
extern void USATMoreTime(void);
extern void USATPollInterval(void);
extern void USATPollingOff(void);
extern BYTE MTCallEvent(BYTE pos);
extern BYTE CallConnected(BYTE pos);
extern BYTE CallDisConnect(BYTE pos);
extern void USATMTCallEvent(void);
extern void USATCallContral(void);
extern BYTE LocationEvent(BYTE pos);
extern void USATLocationEvent(void);
extern void USATCallDisConnected(void);
extern void USATCallConnected(void);
extern void USATUserActiveEvent(void);
extern BYTE UserActiveEvent(void);
extern void USATIdleSrcEvent(void);
extern BYTE IdleSrcEvent(void);
extern void USATBrowserTerminateEvent(void);
extern BYTE BrowserTerminateEvent(void);
extern void ChannelStatusEvent(void);
extern void USATChannelStatusEvent(void);
extern void USATDataAvalEvent(void);
extern void USATChannelStatus(void);
extern void USATSetupMenuAndENV(void);
extern void USATSetupEventList(void);
extern void USATTimerManagement(void);
extern void USATReceiveData(void);

extern void USATDisplayText(void);
extern void USATGetInPut(void);
extern void USATPlayTone(void);
extern void USATRefresh(void);
extern void USATSendMessage(void);

extern void USATSetSpcPrjItem(void);
extern void USATSelectItem(void);
extern void USATSendSS(void);
extern void USATSendUSSD(void);
extern void USATSetUpCall(void);
extern void USATProvideLociInfo(void);

extern void USATLaunchBrowser(void);
extern void USATOpenChannel(void);
extern void USATCloseChannel(void);
extern void USATSendData(void);

extern void USATDataAvailable(BYTE pos);

extern BYTE cephReceiveDataSingle2(BYTE channleId, BYTE dcs, BYTE * alpha, BYTE * receiveData);

extern void USATTestApp(BYTE menuId);

extern BYTE xdata G_USATEventContral;

extern BYTE code scadrr1[];
extern BYTE code scadrr2[];
extern BYTE code scadrr3[];
extern BYTE code scadrr4[];
extern BYTE code scadrr5[];
extern BYTE code scadrr6[];
extern BYTE code scadrr7[];
extern BYTE code scadrr8[];
extern BYTE code scadrr9[];
extern BYTE code scadrr10[];
extern BYTE code scadrr11[];
extern BYTE code scadrr12[];

extern BYTE code data7_21[];
extern BYTE code mthj[];
extern BYTE code eventdisplay[];
extern BYTE code EventlistSrting[];
extern BYTE code mthjylj[];
extern BYTE code mohjylj[];
extern BYTE code hjcx1[];
extern BYTE code hjcx2[];
extern BYTE code hjcx3[];
extern BYTE code locationeventlist[];
extern BYTE code useractive[];
extern BYTE code idlesrc[];
extern BYTE code browser[];
extern BYTE code channeldroplink[];
extern BYTE code channelstatusdis[];

#define D_USATTEST_DataAvailable		0x75
#define D_USATTEST_MTCallEvent_1		0x76
#define D_USATTEST_MTCallEvent_2		0x77
#define D_USATTEST_MTCallEvent_3		0x78
#define D_USATTEST_CallConnected_1		0x79
#define D_USATTEST_CallConnected_2		0x7A
#define D_USATTEST_CallConnected_3		0x7B
#define D_USATTEST_CallConnected_4		0x7C
#define D_USATTEST_CallConnected_5		0x7D
#define D_USATTEST_CallConnected_6		0x7E
#define D_USATTEST_CallConnected_7		0x7F
#define D_USATTEST_CallDisConnect_1		0x8D   //原先值0x80 和 短信TYPE_SMS_UPD_MULTI值冲突 修改为  0x8D
#define D_USATTEST_CallDisConnect_2		0x8E   //原先值0x81 和 短信TYPE_SMS_UPD_SINGLE值冲突 修改为 0x8E
#define D_USATTEST_CallDisConnect_3		0x82
#define D_USATTEST_CallDisConnect_4		0x83
#define D_USATTEST_CallDisConnect_5		0x84
#define D_USATTEST_USATLocationEvent 	0x85
#define D_USATTEST_LocationEvent		0x86
#define D_USATTEST_UserActiveEvent		0x87
#define D_USATTEST_IdleSrcEvent			0x88
#define D_USATTEST_BrowserTerminateEvent	0x89
#define D_USATTEST_ChannelStatusEvent_1	0x8A
#define D_USATTEST_ChannelStatusEvent_2	0x8B
#define D_USATTEST_ChannelStatusEvent_3	0x8C


#endif


#endif
