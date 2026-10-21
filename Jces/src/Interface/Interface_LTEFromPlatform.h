#ifndef _INTERFACE_LTEFROMPLATFORM_H_
#define _INTERFACE_LTEFROMPLATFORM_H_

#include "ecpDefine.h"
#include "profile.h"

#define INVALID_ADDR              0xFFFFFFFF

#define GID1_TYPE           0//modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
#define MCCMNC_TYPE         1
#define ICCID_TYPE          2
#define PPR_TYPE            3
#define TYPE_PROFILE_CLASS  4
#define GID2_TYPE           5//added by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP

#define SMS_TYPE     0
#define HTTP_TYPE    1
#define CATTP_TYPE   2


#define MODE_PRIVATE 0x01
#define MODE_DEFAULT 0x02

#define ExternalAuth_Excuted               0x02


#define PRI_AUTH_ENABLE         0xFF   //私有鉴权方式打开
#define PRI_AUTH_DISABLE        0x5A    //私有鉴权方式关闭

extern u8  X_A G_DownLoadingProfileIndex;

/************************function define********************************************/
extern u8 IsUsimADF(u8 * aidBuf, u16 offset,u8 aidLen);

#if JAVA_CARD_UICC
extern memref GetFileSystemTotalSize(void);
#else
#define  GetFileSystemTotalSize()      0
#endif

extern u8 IF_GetDownloadProfileIndex(void);

extern void IF_PEGlobalVariableInit(void);

extern memref GetMFFileStart(void);

extern void IF_M2M_ISDP_Process84(u8 ssdId, u8 *buffer, u8 bufferLen, u8 *outPut);

extern void IF_M2M_ISDP_Process85(u8 ssdId, u8 *buffer, u8 bufferLen, u8 *outPut);

extern void IF_M2M_ISDP_Process86(u8 appid,memref buffer, u16 totallen, u8 *outPut);

extern void IF_Get_EUICCEID(u8 *identification);

extern void SetDownLoadingProfileMFOffset(memref mfOffset);

extern void SetDownLoadingProfilePOL1(u8 POL1);

extern memref GetMaxProfileMFEnd(void);

extern u8 IF_CustomerGetDataFromMetadata(u8 type,u8 *outPut);

extern u8 IF_CustomerCheckTestProfile(void);

extern u8 IF_UpdateConnectPara(u8 *inBuf,u16 inLen);

extern void IF_PEUpdateCATTPPersonalisation(u16 catTpMaxSduSize, u16 catTpMaxPduSize);

extern u8 IF_PEUpdateContactlessProtocolParameters(u8 *contactlessProtocolParameters);

extern void IF_PEUpdateRestrictParameter(u8 restrictParameter);

extern void IF_SetProfileCapacityInfo(u16 DIR);

extern void IF_SetDownLoadingProfileToValidState(void);

extern void IF_SetProfileAnalyzeMNOSD(void);

extern u8 IF_GetAnalyzeAppIdByAIDList(u8 *aidBuf,u8 aidLen);

extern void IF_SetAnalyzeExtraditeSDID(u8 appid);

extern void IF_SetAnalyzeLifecycle(u8 lifeCycle);

extern void IF_SetKeyCounter(u8* counter,u8 CouterLen);

extern void IF_InitKeyCounter(void);

//extern u16 IF_GetHttpTriggerParaBySDID(u8 sdId,u8 tag,u8 *dataBuffer,u16 dataBufferLen);

extern u8 IF_M2M_GetEnableISDPHttpConnectPara(u8 *para);

extern void NM_ProfileStoreData(void);
//extern void NM_ProfileStoreData(void);

//extern void NM_ProfileLoad(void);

//extern void NM_ProfilePutKey(void);

extern u8 IF_GPGetPkgIDByPkgAID(u8* aidBuf, u8 aidLen,u8 majorVersion, u8 minorVersion/*,u8 mode*/);

extern u16 IF_ExcuteProfileGPCommand(u8 excuteSdId);

extern u8 GetProfileIndexByISDPTar(u8* tarBuf);

extern memref GetIfMfCreated(u8 mode);

extern u8 IF_GetAppIdByAppRef(u16 appRef);

//extern u16 IF_GetRAMResWORDLen(void);

extern void IF_UpdateFileToMetaData(u8 dataLen,u8 * indata,u8 offset,u16 fileName);

extern u8 IF_ProfileChangeMechanism(void);

extern u8 IF_GetNotifitionType(void);

extern void IF_M2M_IncreaseNotificateNumerber(void);

extern void IF_SaveSCP80KeyValueBeforeUpdate(void);

extern void IF_M2M_DeleteProfilePE(u8 profileIndex);

//extern void IF_RSPReturnMainFormSTK(void);

extern void IF_GetCATTPPara(u8 size[2],u8 type);

extern u8 GPSSDExistCompleteSecureKey(u8 sdId,u8 scp_type);

extern u8 IF_M2M_ISDP_Process87(u8 appid, memref buffer, u16 totallen);

extern u8 IF_GetScp81SecurityLevel(void);

extern u8 IF_GetScp80SecurityLevel(void);

extern void IF_ChangeProfileInitPrivateFile(void);	//remove warning,dengyiwen,2022/9/27
#endif
