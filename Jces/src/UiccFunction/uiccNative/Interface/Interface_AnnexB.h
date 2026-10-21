#ifndef ANNEXB_INTERFACE_H
#define ANNEXB_INTERFACE_H
#include "typedef.h"
/*********************************IF_AnnexB_StartTimerpublic的模式**************/
#define mode_AfterRestartHandset 1
#define mode_AfterUnNormalEnd    2

extern WORD IF_AnnexB_HongKong_GetAppResponseLength(BYTE mode);
extern void IF_AnnexB_ProcessRemoteAppendDataForPor(BYTE mode);
extern void IF_AnnexB_StartTimerpublic(BYTE mode);

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
extern BYTE code Label_Tls0301_CipherSuit[7];
extern BYTE code Label_Tls0303_CipherSuit[7];
#else
extern BYTE code Label_Tls0301_CipherSuit[7];
extern BYTE code Label_Tls0303_CipherSuit[11];
#endif

extern void HttpTimerExpiration(void);
extern void IF_AnnexB_RetryHttpSessionByTimerExpiration(BYTE timerExpirationId);
extern void IF_AnnexB_StartHttpSessionByAPI(void);
extern void IF_AnnexB_INDIORJIO_Special(SWORD* value);
extern void IF_AnnexB_HTTPRFM(BYTE mode);
extern BYTE IF_AnnexB_PendRefreshCommand(BYTE commandType);
extern void IF_AnnexB_TLSVariableInit(void);
extern BYTE IF_AnnexB_ChinaMobileNotSuppotRFMmode(BYTE * tarAppAID, BYTE appIdLen);
extern WORD IF_GetSCP81TLSVersion(BYTE sdId);
extern BYTE IF_AnnexB_GetOpenChannelParaForHttp(BYTE *para);
extern void IF_HTTPDataAvaiable(BYTE channelDataLen, WORD offset);
extern BYTE IF_AnnexB_HTTP_SecondSession_Start(void);
extern BYTE IF_GP_GPAnnexB_TriggerSMS(UWORD fileAddr, WORD commandOffset,WORD cplLen);
extern BYTE IF_GP_GPAnnexB_TriggerSMS_Check(UWORD fileAddr, WORD commandOffset, WORD DataLen);
extern BYTE IF_AnnexB_ProcessRFMRAMAPDUPublic(BYTE currentComNUM,WORD resultLength,BYTE isRAM);
extern BYTE IF_AnnexB_ProcessRAMRFMAPDU(BYTE currentComNUM, WORD resultLength);
extern void IF_AnnexB_GetHttpsCompactAPDURes(void);
extern BYTE IF_AnnexB_ProcessExtendRFMRAMAPDU(WORD additionalLen,BYTE isRAM);
extern void IF_AnnexB_SaveHttpsCompactAPDURes(void);
extern BYTE IF_AnnexB_GPStoreSDDNS(void);
extern void IF_AnnexB_CommandExecuteError(void);
extern BYTE IF_AnnexB_ChannelStatus(void);
extern void IF_AnnexB_GETTargetedADF3G(BYTE* src);
extern BYTE IF_AnnexB_GETTargetedADF2G(BYTE* src);
extern void IF_EAPTAR(BYTE *tarBuf);
extern BYTE IF_EAPADF(void);
extern BYTE HTTPPendingSessionTrigger(void);
extern WORD IF_GetHttpTriggerParaBySDID(BYTE sdId,BYTE tag,BYTE *dataBuffer,WORD dataBufferLen);

//***********HTTPAdministration 接口对应的函数************************//
extern void NM_GPRequestHTTPAdministrationSession(void);
extern void NM_getCurrentHttpStatus(void);
extern void NM_getHttpAPPID(void);

extern void IF_ProcessRemoteMoveFalshForHTTP(UWORD SrcfileAddr,WORD Offset, WORD cplLen);
extern BYTE IF_RfmApduModeCheck(void);
extern void clearHttpRetryFlag(void);
extern BYTE IF_HttpOpenChannelTagCompalete(BYTE *srcBuf, BYTE scrLen);
extern void IF_AnnexB_ChangeComNUM(BYTE mode);
extern BYTE IF_OpenChannel_Retry( BYTE result);
extern BYTE IF_HTTPS_HandShake(void);
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



