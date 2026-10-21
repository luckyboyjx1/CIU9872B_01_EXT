#ifndef    _EUICCCUSTOMERPUBLIC_H   
#define	   _EUICCCUSTOMERPUBLIC_H 
#include   "typedef.h"
#include   "profile.h"

//#if RSP_EUICC

#define CIPKID_ForSignature             0
#define CIPKID_ForVerification          1




/*-------------------ASN1 TAG定义--------------------*/

#define   TAG_SVN         			                                  0x82
#define   TAG_ExtCardResource                                         0x84

#define   TAG_EuiccCiPKIdListForVerification                          0xA9
#define   TAG_EuiccCiPKIdListForSigning                               0xAA

#define   TAG_UiccCapability                                          0x85
#define   TAG_Ts102241Version                                         0x86
#define   TAG_GlobalplatformVersion                                   0x87
#define   TAG_RspCapability                                           0x88

#define   TAG_EuiccCategory                                           0x8B	//euiccCategory [11] INTEGER
#define   TAG_ForbiddenProfilePolicyRules                             0x99  //forbiddenProfilePolicyRules [25] PprIds OPTIONAL, –- Tag '99' 
#define   TAG_PPVersion                                               0x04  //Basic types,VersionType ::= OCTET STRING(SIZE(3))
#define   TAG_SasAcreditationNumber                                   0x0C  //UTF8String (SIZE(0..64))
#define   TAG_CertificationDataObject                                 0x8C  //certificationDataObject [12] CertificationDataObject OPTIONAL


#define   TAG_SearchCriteria                                          0xA0  //searchCriteria [0] CHOICE
#define   TAG_SearchCriteria_AID                                      0x4F
#define   TAG_SearchCriteria_ICCID                                    0x5A
#define   TAG_NickName                                                0x90
#define   TAG_ProfileClass                                            0x95

#define   TAG_ProfileInfo                                             0xE3

/************************Profile metaData结构体中各项数据的偏移定义******************************/
#define GET_PROFILE_INFO_TAG_LIST   0x00
#define GET_PROFILE_INFO_DEF_LIST   0x01

#define UNSPECIFIED_TAG_NUMBER      0x06

//typedef __packed struct
//{
//   BYTE profileState;
//   BYTE profilePolicyRules[4];  //TLV格式 tag为 99
//   BYTE profileClass[3];        //TLV格式，tag为95 ,默认值为950102
//   BYTE iccid[12];              //存储的是TLV ，初始化这个结构体初始化成0. Iccid ::= [APPLICATION 26] OCTET STRING (SIZE(10))
//   BYTE iconType[3];            //TLV
//   WORD iconRef;
//   WORD serviceProviderNameRef;
//   WORD profileNameRef;
//   WORD notificationConfigurationInfoRef;   //数据的存储结构为TLV, B6 L 30LV 30LV...
//   WORD dpProprietaryDataRef;
//   WORD profileOwerRef;
//   WORD profileNickNameRef;
//   WORD unspecifiedTag[UNSPECIFIED_TAG_NUMBER];
//}customerMetaDataStore;


//#define   MetaData_ProfileState_Offset                                0x00
//#define   MetaData_ProfilePolicyRules_Offset                          0x01
//#define   MetaData_ProfilePolicyRulesValue_Offset                     0x04
//#define   MetaData_ProfileClass_Offset                                0x05
//#define   MetaData_ProfleClassValue_Offset                            0x07
//#define   MetaData_Iccid_Offset                                       0x08
//#define   MetaData_IccidValue_Offset                                  0x0A
//#define   MetaData_IconType_Offset                                    0x14
//#define   MetaData_IconRef_Offset                                     0x17
//#define   MetaData_ServiceProviderNameRef_Offset                      0x19
//#define   MetaData_ProfileNameRef_Offset                              0x1B
//#define   MetaData_NotificationConfigurationInfoRef_Offset            0x1D
//#define   MetaData_DpProprietaryDataRef_Offset                        0x1F
//#define   MetaData_ProfileOwerRef_Offset                              0x21
//#define   MetaData_ProfileNickNameRef_Offset                          0x23  //数据存储结构是TLV,服务器下来的是什么，存储的就是什么
//#define   MetaData_UnspecifiedTagRef_Offset                           0x25

/********************storeMeta data中各元素的边界值的宏定义**************************/
#define   Default_DP_Address_MAX_LEN                                  0xFF
#define   Profile_NickName_MAX_LEN                                    0x100 // profileNickname [16] UTF8String (SIZE(0..64))  The size limits for UTF-8 strings apply to the number of UTF-8 characters, each coded on 1 to 4 bytes
#define   Common_Auth_CtxParams1_MAX_TLV_LEN                          0xFF  // AuthenticateServerRequest中ctxParams1的TLV的最大字节数，由于
#define   PrepareDownload_hashCc_Len                                  0x20  // hashCc Octet32 OPTIONAL, -- Hash of confirmation code
#define   InitialiseSecureChanne_hostId_MAX_LEN                       0x10  // hostId[4] OctetTo16 -- Host ID value / OctetTo16 ::= OCTET STRING (SIZE(1..16))
#define   ICCID_LEN                                                   0x0A  // ICCID的长度
#define   ProfileOwner_MAX_LEN                                        sizeof(tempBuf) // 是由于CUSTOMER_StoreMetadata函数中需要把profileOwner备份到tempBuf中
#define   NotificationAddress_MAX_LEN                                 0x80  // notificationAddress的L的最大值  GenerateNotificationMetadataTLV中会使用到，由于会借用tempBuf，所以这里对能力进行限制

/********************OperatorId subfield中各字段的定义*******************************/
#define  OperatorId_MCCMNC_TAG                                        0x80
#define  OperatorId_GID1_TAG                                          0x81
#define  OperatorId_GID2_TAG                                          0x82
/************************RSP中接口 Response 的TAG宏定义******************************/
#define TAG_LIST_NOTIFICATION									      0xBF28
#define TAG_SET_NICK_NAME										      0xBF29
#define TAG_RETRIEVE_NOTIFICATION_LIST							      0xBF2B
#define TAG_REMOVE_NOTIFICATIOV_FROM_LIST   	                      0xBF30
#define TAG_ENABLE_PROFILE                  	                      0xBF31
#define TAG_DISABLE_PROFILE                 	                      0xBF32
#define TAG_DELETE_PROFILE                 	                          0xBF33
#define TAG_EUICCMEMORYRESET                	                      0xBF34
#define TAG_SET_DEFAULT_DPADDR							              0xBF3F
#define TAG_GET_RAT                                                   0XBF43

/************************RSP中接口 Response 的Result 宏定义**************************/
#define RESULT_OK_       										      0x00
#define RESULT_UNDEFINED_ERROR              	                      0x7F
#define RESULT_ICCID_NOT_FOUND							              0x01
#define RESULT_NOTHINGTODELETE                                        0x01
#define RESULT_PROFILE_NotINDisabledState                             0x02
#define RESULT_PROFILE_NotINEnabledState                              0x02
#define RESULT_DISALLOWEDByPolicy                                     0x03
#define RESULT_WRONGProfileReenabling                                 0x04
#define RESULT_CATBUSY                                                0x05
#define RESULT_COMMANDERROR                                                0x07

/**************************CASE3 STATUSResponse*************************************/
#define PROFILE_NOT_FOUND                                             0x6A82
#define PROFILE_NOT_ALLOWED                                           0x6985
#define PROFILE_CATBUSY                                               0x9483 //0x9300 当成system异常被捕获时  写error reason时 会将高字节变为0，在Java层无法转为状态字9300

#define CUSTOMERCommand_ENABLEPROFILE                                 0x01
#define CUSTOMERCommand_DISABLEPROFILE                                0x02

/**********RSP中接口enableProfile/DisableProfile Response 的Result 宏定义************/
#define RESULT_ICCID_NOR_AID_NOT_FOUND                                0x01
#define RESULT_PROFILE_NOT_IN_ENABLED_OR_DISABLED_STATE               0x02
#define RESULT_DISALLOWED_BY_POLICY                                   0x03
#define RESULT_WRONG_PROFILE_REENABLING                               0x04
#define RESULT_CAT_BUSY                                               0x05


/**********************************PprIds中宏定义************************************/
#define PPRIDS_pprUpdateControl                                       0x80   // pprUpdateControl(0), -- defines how to update PPRs via ES6
#define PPRIDS_ppr1                                                   0x40   // ppr1(1), -- Indicator for PPR1 'Disabling of this Profile is not allowed'
#define PPRIDS_ppr2                                                   0x20   // ppr2(2), -- Indicator for PPR2 'Deletion of this Profile is not allowed'

/**********************************Profile Class宏定义*******************************/
#define PROFILE_CLASS_TEST                                            0x00   // Function (ES10c): GetProfilesInfo   ProfileClass ::= INTEGER {test(0), provisioning(1), operational(2)}
#define PROFILE_CLASS_PROVISIONING                                    0x01
#define PROFILE_CLASS_OPERATIONAL                                     0x02

/**********************************Profile State宏定义*******************************/
#define PROFILE_STATE_ENABLED                                         0x01   // Function (ES10c): GetProfilesInfo  ProfileState ::= INTEGER {disabled(0), enabled(1)} 
#define PROFILE_STATE_DISABLED                                        0x00

#define NOTIFICATION_INSTALL                                          0x80 //bitstring(0)
#define NOTIFICATION_ENABLE                                           0x40 //bitstring(1)
#define NOTIFICATION_DISABLE                                          0x20 //bitstring(2)
#define NOTIFICATION_DELETE                                           0x10 //bitstring(3)

/*****************************Profile install result macro define***************************/
#define BPPCommandId_INITIALISESECURECHANNEL                          0x00
#define BPPCommandId_CONFIGUREISDP                                    0x01
#define BPPCommandId_STOREMETADATA                                    0x02
#define BPPCommandId_STOREMETADATA2                                   0x03
#define BPPCommandId_REPLACESESSIONKEYS                               0x04
#define BPPCommandId_LOADPROFILEELEMENTS                              0x05


#define ErrorReason_incorrectInputValues                              0x01
#define ErrorReason_invalidSignature                                  0x02
#define ErrorReason_invalidTransactionId                              0x03
#define ErrorReason_unsupportedCrtValues                              0x04
#define ErrorReason_unsupportedRemoteOperationType                    0x05
#define ErrorReason_unsupportedProfileClass                           0x06
#define ErrorReason_scp03tStructureError                              0x07
#define ErrorReason_scp03tSecurityError                               0x08
#define ErrorReason_installFailedDueToIccidAlreadyExistsOnEuicc       0x09
#define ErrorReason_installFailedDueToInsufficientMemoryForProfile    0x0A
#define ErrorReason_installFailedDueToInterruption                    0x0B
#define ErrorReason_installFailedDueToPEProcessingError               0x0C
#define ErrorReason_installFailedDueToDataMismatch                    0x0D
#define ErrorReason_testProfileInstallFailedDueToInvalidNaaKey        0x0E
#define ErrorReason_pprNotAllowed                                     0x0F
#define ErrorReason_installFailedDueToUnknownError                    0x7F

///**********************prepare download中的DownloadErrorCode***********************/
//#define ErrorCode_invalidCertificate                                  0x01
//#define ErrorCode_invalidSignature                                    0x02
//#define ErrorCode_unsupportedCurve                                    0x03
#define ErrorCode_noSessionContext                                    0x04

#define DownloadErrorCode_invalidTransactionId                        0x05
#define DownloadErrorCode_undefinedError                              0x7F


//#define RESPONSE_NoError                                              0x00

/************************AuthenticateErrorCode***********************************/
#define AuthenticateError_invalidCertificate                          0x01
#define AuthenticateError_invalidSignature                            0x02
#define AuthenticateError_unsupportedCurve                            0x03
#define AuthenticateError_noSessionContext                            0x04

#define AuthenticateError_invalidOid                                  0x05
#define AuthenticateError_euiccChallengeMismatch                      0x06
#define AuthenticateError_ciPKUnknown                                 0x07
#define AuthenticateError_undefinedError                              0x7F

/*****************************************生成通知时的宏定义 **********************************/
#define NOTIFICATION_HEAPALLOC_MODE_PRE                               0x00
#define NOTIFICATION_HEAPALLOC_MODE_NOR                               0x01

/***********************************************funtion define **********************************/
extern BYTE ProfileChangeDisable_GernerateNotifyAndSetState(BYTE targetProfileIndex, BYTE mode);
extern void ProfileChangeEnable_GernerateNotifyAndSetState(BYTE targetProfileIndex, BYTE targetProfileClass, BYTE mode);
extern BYTE Notification_GetValidNum(UWORD* addr, UWORD* addr2);

extern void Notification_GenerateOtherOperation(BYTE operation, BYTE profileIndex);
extern BYTE Notification_GenerateProfileInstallResult(BYTE bppCommandId, BYTE result, BYTE * iccid);

extern WORD GetEuiccCiPKIdList(BYTE* OutBufAddr, WORD freeLen, BYTE mode);
extern WORD GetProfileInfoDataByTagList(UWORD outAddr, WORD freeLen, BYTE* tagList,BYTE profileIndex, BYTE mode);
extern BYTE GetForbiddenProfilePolicyRules(BYTE* buf);

extern BYTE PPRsVerification(BYTE ppr, BYTE  *OperatorId, BYTE OperatorIdLen);
extern BYTE StoreMetaData_verifyProfileOwner(BYTE  *OperatorId, BYTE OperatorIdLen);
extern BYTE StoreMetaData_ProcessEachMetaData88(void);

extern BYTE PrepareDownload_VerifySmdpSigned2(UWORD dataAddr, BYTE *dataBuf);
extern BYTE AuthenPrepareDownload_VerifyDpSignature(BYTE *certificateAddr,WORD certificateLen, UWORD signatureBufAddr ,BYTE signatureBufLen,UWORD signatureValueAddr, BYTE signatureValueLen , BYTE mode);

extern BYTE InitialiseSecureChannel_VerifySmdpSign(UWORD dataStartAddr,WORD smdpSignOff);
extern BYTE InitialiseSecureChannel_CalShS(UWORD smdpOtpkAddr, BYTE *outBuf);

extern BYTE AuthenticateServer_CheckServerSigned1(UWORD dataAddr, BYTE *serverChallangeBuf);
extern BYTE AuthenticateServer_CheckEuiccCiPKIdToBeUsed(UWORD dataAddr, UWORD * UICCCertifiIndex);
extern void AuthenticateServerPrepareDownload_SendResponseError(UWORD dataAddr, BYTE errorCode,WORD startTag);

extern WORD AuthenticateServer_SendResponseSuccess(UWORD ctxParams1Addr, BYTE ctxParams1Len,BYTE *serverChallange, UWORD caListAddr);

extern WORD AuthenticateServer_CalEuiccSignature1(UWORD signDataAddr, WORD signDataLen, UWORD euiccListAddr, BYTE *sigoutBuf);

extern BYTE PrepareDownload_GenerateEuiccSigned2(UWORD dataAddr,BYTE* OutBuf,WORD tagOffset[4]);
extern BYTE PrepareDownload_GenerateEuiccSignature2(BYTE* dataBuf,BYTE dataLen ,BYTE* OutBuf);
extern WORD GetEuiccInfo2SubFunc(UWORD OutBuf, WORD freeLen);

extern BYTE SetDefaultDpAddress_MemoryReset(BYTE *dpAddress, BYTE dpAddressLen);
extern void CancelSessionSendErrorResult(BYTE errorCode);
extern BYTE PaddingCancelSessionResponse(BYTE cancelSessionReason);

extern BYTE GetFinalResultTLVLen(BYTE result, BYTE bppCommandId, BYTE* simaResponse);
extern void ResetTagList(BYTE* tagList,BYTE *FormatTaglist);
//extern void EUICC_SendSingleChannelResponse(WORD sendLen);
extern UWORD Notification_HeapAlloc(WORD notificationDataSize ,BYTE operation, WORD euiccEumref, BYTE notificationType, BYTE mode);
extern void CloseChannelAfterProfileChange(BYTE refreshFlag);
extern void SetSendFileExtendInfo(UWORD bufferAddr, WORD len);
extern void IF_ClearSendFileExtendInfo(void);
extern void RSP_ClearSession(void);
extern void RSP_abortLoadingSession(void);
extern void RSP_abortLoadingSessionThrowError(u16 error);
extern void RSP_InitGobalVariable(void);
#endif

//#endif

