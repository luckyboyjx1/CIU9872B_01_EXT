#ifndef _UICC_M2M_H_
#define _UICC_M2M_H

#include "typedef.h"
#include "profile.h"

#define     LEN_IMEI								0x08

//#define DecToAsc(x)   x += (x < 0x0A) ? 0x30:0x37;	移到uicc_main.h中，否则非M2M开关时找不到DecToAsc的定义



/************************function define********************************************/
#define NOTIFICATION_ONLY_SMS			      0x20 //only SMS
#define NOTIFICATION_ONLY_HTTP			     0x40 
#define NOTIFICATION_SMS_HTTP			      0x60 
#define NOTIFICATION_HTTP_SMS			      0x80


#define NOTIFICATION_TYPE_SMS			      0x00 // SMS
#define NOTIFICATION_TYPE_HTTP		      0x80 // HTTPS

#define   LocationStatus_Alert                          0x60

#define Tag_ResendNotificationSMS  	                    0xF0

extern BYTE xdata G_Handle_Notification_Tag;              //开始处理通知 (一次开机过程中只处理一次)
extern BYTE  xdata G_NotificationSmsRetryTime;           //通知SMS重试次数
extern BYTE  xdata G_NotificationTimerId;                //发送通知SMS成功后开启的timer
//extern BYTE  xdata G_NotificationConfir;				//用于发送通知确认por时， 判断当前是发送通知确认的por，获取isdp配置的tppid, tpdcs
extern BYTE  xdata G_IsSendHttpsNotification;            //正在通过https发送通知	
extern BYTE xdata G_RefreshAfterCloseBIP;            //需要在关闭https 通道后再发送refresh
extern BYTE xdata G_IMEI_FOR_NOTIFICATION[9];       //存储发送通知需要的IMEI
extern BYTE xdata G_STATUSNUM_FOR_NOTIFICATION; //STATUS个数，10次以后发送通知
extern BYTE xdata G_STATUSNUM_FOR_REFRESH;      //STATUS个数，1次以后重发refresh

#define LocationStatus_Normal                   0x00
#define LocationStatus_Limited_Service          0x01
#define LocationStatus_NO_Service     			0x02
#define LocationStatus_10_Status     			0x03    //没有收到LocationStatus，收到10个status command
#define LocationStatus_Limited_AND_NOService_10_Status	0x04
#define LocationStatus_NULL		    			0xFF	//初始值， 

extern BYTE BuildNotificationBySMS(void);
extern void LocationStatusProcess(void);
extern void HandleDefaultNotification(BYTE locationStatus);
extern void sendRefreshComm(void);
extern void startNotificationTimer(BYTE notiTytpe);
//extern void ProcessResendNotification(void);
extern void IF_ProcessResendNotification(BYTE timerExpirationId);

extern void GetIsdpSmsc(BYTE * smscBuffer);
extern void GetIsdpPidDcs(BYTE piddcs[2]);

extern BYTE SendNotificationBySMS(void);
extern BYTE SendNotificationByHTTPS(void);

extern BYTE IF_GetNotificationProtocol(void);
extern BYTE HexStrToAscStr(BYTE * hexBuf, BYTE * acsiiBuf, BYTE dataLgth);//EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
extern BYTE IF_StartTimerForHttpsNotification(void);
extern BYTE checkDataFieleIsISDP(void);
#endif
