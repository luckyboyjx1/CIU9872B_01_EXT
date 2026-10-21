#ifndef ANNEXB_INTERFACE_H
#define ANNEXB_INTERFACE_H
#include "ecpDefine.h"
#include "profile.h"
#include "typedef.h"
/*********************************IF_AnnexB_StartTimerpublic的模式**************/
#define mode_AfterRestartHandset 1
#define mode_AfterUnNormalEnd    2


extern u16 IF_AnnexB_HongKong_GetAppResponseLength(u8 mode);
extern void IF_AnnexB_ProcessRemoteAppendDataForPor(u8 mode);
extern void IF_AnnexB_StartTimerpublic(u8 mode);

//extern void IF_AnnexB_StartTimerAfterRestartHandset(void);
//extern void IF_AnnexB_StartTimerAfterUnNormalEnd(void);

//以下宏定义值参考5G SimCard\DOC\03 需求规范\移动5G卡技术要求\BIP增强处理技术要求_20200529.pdf
#define  NOTGPAnnexBTritag      0xFF		//HTTP 触发标志初始化为0xff, 标识当前sms不是http触发sms
#define  GPAnnexBTritag      	0x00		//表示是http的触发短信并且正常解析参数
#define  TLSSessionHasExist     0x01		//当前有tls session在执行 
#define  TLSSessionHasClose	    0x02		//tls session已关闭
#define  GPAnnexBTriParaError   0x03		//参数错误


#define ComNUM_CLEAR		0
#define ComNUM_INCREASE		1
#define ComNUM_DECREASE		2

#define IF_AnnexB_StartTimerAfterRestartHandset() IF_AnnexB_StartTimerpublic(mode_AfterRestartHandset)
#define IF_AnnexB_StartTimerAfterUnNormalEnd()    IF_AnnexB_StartTimerpublic(mode_AfterUnNormalEnd)

#if UICC__OPERATOR__CHINATELECOM
extern u8 const Label_Tls0301_CipherSuit[7];
extern u8 const Label_Tls0303_CipherSuit[7];
#else
extern u8 const Label_Tls0301_CipherSuit[7];
extern u8 const Label_Tls0303_CipherSuit[11];
#endif
extern u8 IF_HTTPS_HandShake(void);
extern void HttpTimerExpiration(void);
extern void IF_AnnexB_RetryHttpSessionByTimerExpiration(u8 timerExpirationId);
extern void IF_AnnexB_StartHttpSessionByAPI(void);
extern void IF_AnnexB_INDIORJIO_Special(s16* value);
extern void IF_AnnexB_HTTPRFM(u8 mode);
extern u8 IF_AnnexB_PendRefreshCommand(u8 commandType);
extern void IF_AnnexB_TLSVariableInit(void);
extern u8 IF_AnnexB_ChinaMobileNotSuppotRFMmode(u8 * tarAppAID, u8 appIdLen);
extern void NM_GetAppletRefByAppId(void);
extern u16 IF_GetSCP81TLSVersion(u8 sdId);
extern u8 IF_AnnexB_GetOpenChannelParaForHttp(u8 *para);
extern void IF_HTTPDataAvaiable(u8 channelDataLen, u16 offset);
extern u8 IF_AnnexB_HTTP_SecondSession_Start(void);
extern u8 IF_GP_GPAnnexB_TriggerSMS(memref fileAddr, u16 commandOffset,u16 cplLen);
extern u8 IF_GP_GPAnnexB_TriggerSMS_Check(memref fileAddr, u16 commandOffset, u16 DataLen);
extern u8 IF_AnnexB_ProcessRFMRAMAPDUPublic(u8 currentComNUM,u16 resultLength,u8 isRAM);
extern u8 IF_AnnexB_ProcessRAMRFMAPDU(u8 currentComNUM, u16 resultLength);
extern void IF_AnnexB_GetHttpsCompactAPDURes(void);
extern u8 IF_AnnexB_ProcessExtendRFMRAMAPDU(u16 additionalLen,u8 isRAM);
extern void IF_AnnexB_SaveHttpsCompactAPDURes(void);
extern u8 IF_AnnexB_GPStoreSDDNS(void);
extern void IF_AnnexB_CommandExecuteError(void);
extern u8 IF_AnnexB_ChannelStatus(void);
extern void IF_AnnexB_GETTargetedADF3G(u8* src);
extern u8 IF_AnnexB_GETTargetedADF2G(u8* src);
extern void IF_EAPTAR(u8 *tarBuf);
extern u8 IF_EAPADF(void);
extern u8 HTTPPendingSessionTrigger(void);
extern u16 IF_GetHttpTriggerParaBySDID(u8 sdId,u8 tag,u8 *dataBuffer,u16 dataBufferLen);
extern BYTE IF_CheckConnectionParameters(BYTE * ConnectionParameters, BYTE length);

//***********HTTPAdministration 接口对应的函数************************//
extern void NM_GPRequestHTTPAdministrationSession(void);
extern void NM_getCurrentHttpStatus(void);
extern void NM_getHttpAPPID(void);

extern void IF_ProcessRemoteMoveFalshForHTTP(memref SrcfileAddr,u16 Offset, u16 cplLen);
extern BYTE IF_HttpTeleUsimModeCheck(void);
// extern u8 IF_RfmApduModeCheck(void);  该函数被移动到CompactManage.c中
extern void clearHttpRetryFlag(void);
extern u8 IF_HttpOpenChannelTagCompalete(u8 *srcBuf, u8 scrLen);
extern u8 IF_OpenChannel_Retry( u8 result);
extern void IF_AnnexB_ChangeComNUM(u8 mode);
extern void IF_AnnexB_CloseCommandSession(void);
extern BYTE IF_AnnexB_getCommandSessionChannelNum(u8 sdId);
extern BYTE IF_AnnexB_SupportNewCommandSession(void);
extern void HTTPSecondSessionParaHold(memref fileAddr, u16 commandOffset,u16 cplLen, u8 mode, u8 triggerAPPID);

extern BYTE IF_UPCOS_HTTPSAnalyseTriggerPara(BYTE * triggerBuf);
extern void IF_UPCOS_PendingHTTPAlert(void);
extern void IF_UPCOSCreaBipLinkByStatus(void);
//文件 "Interface_Hmac.h" 的宏此处为了编译过临时存放
#define ALG_HMAC_NULL               0
//#define ALG_HMAC_MD5	            1
//#define ALG_HMAC_SHA1             2
//#define ALG_HMAC_SHA256	        3

#define HMAC_DIGEST_LENGTH_MD5      16
//#define DIGEST_LENGTH				SHA1_DIGEST_SIZE
//#define DIGEST_LENGTH_256			SHA256_DIGEST_SIZE

#define HMAC_DIGEST_LENGTH_MAX      32  //???°×?′óμ?hmac?a?ahashê?3??asha256 32 ×??ú

#endif



