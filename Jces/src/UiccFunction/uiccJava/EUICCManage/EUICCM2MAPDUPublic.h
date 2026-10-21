#ifndef    _EUICCM2MAPDUPUBLIC_H_
#define    _EUICCM2MAPDUPUBLIC_H_
#include   "typedef.h"
#include   "profile.h"

#if M2M_EUICC
/*************************function define********************************/
#define POL1_NO_POL1_ACTIVE      0x00
#define POL1_DISABLE_NOT_ALLOWED 0x01
#define POL1_DELETED_NOT_ALLOWED 0x02
#define POL1_DELETED_MANDATORY   0x04

#define NOTIFICATION_NON                                0x00  //无通知需要发送 每次发送完通知以后清成该值
#define NOTIFICATION_FIRST_NETWORD_ATTACHMENT           0x01
#define NOTIFICATION_RROFILE_CHANGE_SUCCEEDED           0x02
#define NOTIFICATION_RROFILE_CHANGE_FAILED_AND_ROLLBACK 0x03
#define NOTIFICATION_RROFILE_CHANGE_AFTER_FALLBACK      0x05
#define NOTIFICATION_FIRST_TIME                         0xFF

#if M2M_ESx
#define NOTIFICATION_FIRST_PROFILE_CHANGE_AFTER_EMERGERCY_PROFILE_DISABLE                    0x06
#define NOTIFICATION_FIRST_PROFILE_CHANGE_AFTER_TEST_PROFILE_DISABLE                         0x07
#endif

//通知重发次数
#define NOTIFICATION_RETRY_TIMES                        0x03


/***************************Scenario Parameters Bit******************/
#define  ScenarioPara_DoNotDeleteKeyFlag  0x01
#define  ScenarioPara_IncludeDRFlag       0x02
#define  ScenarioPara_IncludeHostFlag     0x04
#define  ScenarioPara_CertificateVerFlag  0x08

#define	FallBack_Attribute				0x01
#define	Emergency_Attribute				0x02
#define	Test_Attribute					0x04

#if M2M_ESx
#define		Succsss                     0
#define	    Error_ProfileRef 			8//Emergency/Test Profile does not exists
#define	    Error_AlreadyEnabled 		9 //Emergency/Test Profile is already enabled
#define	    Error_AlreadyDisabled 		10 //Emergency/Test Profile is already disabled
#define	    Error_Forbidden 			12//Transition is forbidden for the Test Profile
#define	    Error_undefine 				127
#endif

extern BYTE GenerateNotificationMessage(BYTE notifyType,BYTE * IMEI,BYTE * outBuf);
extern BYTE CheckKeyEstablishment2DataAvarable(BYTE * dataBuf, BYTE totleLen);
extern BYTE VerifyKeyEstablishment2Signature(BYTE * dataBuf,BYTE dataLen);
extern BYTE ECKACaclSHS(BYTE* shsBuf);
extern BYTE GetShareInfo(BYTE* CRTInfo,BYTE CRTInfoLen,BYTE* shareInfo,BYTE* keyCounter,BYTE* keyInfo);
extern BYTE DerivationScpKey(BYTE* keyInfo,BYTE* keyCounter,BYTE* keyValue,BYTE keyLen,BYTE appID);
//extern BYTE DerivationScp80Key(BYTE* keyInfo,BYTE* keyCounter,BYTE* keyValue,BYTE keyLen,BYTE appID,BYTE profileIndex);
extern BYTE GenerateDESReceipt(BYTE * keyBuf, BYTE keyLen, BYTE * katBuf, BYTE katLen, BYTE * receiptBuf);
extern BYTE EUICCGenReceipt(BYTE * keyBuf, BYTE keyLen, BYTE keyType, BYTE * katBuf, BYTE katLen, BYTE * receiptBuf);
extern BYTE TransformISDPLifeCycle(/*BYTE* aidBuf,*/BYTE profileIndex);
extern BYTE EUICCCapabilityAuditPaddingTagList(BYTE* outBuf,BYTE* aidBuf,BYTE* tagList,BYTE tagLen,BYTE profileIndex);

extern WORD CheckSMSRAddressingParametersSMS(BYTE* buffer);
extern WORD CheckSMSRAddressingParametersCATTP(BYTE* buffer);
extern WORD CheckSMSRAddressingParametersDNS(BYTE* buffer);
extern void FinaliseISDRhandover_DeleteKey(BYTE* buffer, BYTE bufLen);

extern void MasterDelete_CheckB6Tag(BYTE * dataOffset, BYTE b6Len);
extern BYTE MasterDelete_VerifyDeleteToken(BYTE deleteProfileIndex, BYTE len, BYTE* tokenSource,  BYTE offsetToken,BYTE *b6Data,  BYTE b6Len);
extern BYTE IsISDP_Tar(BYTE* buf,BYTE mode);
extern BYTE IsISDR_ECASD_Tar(BYTE* buf);
extern WORD ProcessISDPTarInfo(BYTE * buf,BYTE ToolkitAppSpecificParaOffset,BYTE ToolkitAppSpecificParaLgth,BYTE  tagFlag);
extern void StoreGeneraldataForISD_P(BYTE profileIndex);
extern WORD GPPutAESTokenReceiptKey(BYTE profileIndex);

extern UWORD GetsdDataObjByProfileIndex(BYTE profileIndex, BYTE sdId);

#if M2M_ESx
extern BYTE M2MLocalEnableEmergencyProfile(void);
extern BYTE M2MLocalDisableEmergencyProfile(void);
extern BYTE M2MLocalEnableTestProfile(void);
extern BYTE M2MLocalDisableTestProfile(void);
extern BYTE M2M_GetProfileAttributes(BYTE profileIndex);
#endif

#endif

#endif
