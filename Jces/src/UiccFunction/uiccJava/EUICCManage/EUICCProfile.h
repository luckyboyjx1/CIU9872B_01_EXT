#ifndef    _EUICCPROFILE_H   
#define	   _EUICCPROFILE_H 
#include    "typedef.h"

/************************function define******************************************/


extern BYTE GetProfileAppIdByAID(BYTE * aidValue, BYTE aidLen, BYTE profileIndex);

extern BYTE GetIdleProfileIndex(void);
extern BYTE GetValidProfileNumber(BYTE *outBuf);
extern BYTE GetProfileIndexByProfileClass(BYTE profileClass, BYTE * outBuf);
extern BYTE GetProfileIndexByICCID(BYTE *iccidBuf);
extern BYTE GetProfileIndexByAIDAndState(BYTE *aidBuf, BYTE aidLen,BYTE profiletValidTag);
extern BYTE GetProfileAIDByProfileIndex(BYTE profileIndex, BYTE *aidBuf);
extern BYTE GetProfilePOL1(BYTE profileIndex);
extern BYTE GetProfileState(BYTE profileIndex);
extern BYTE IsProfileValid(BYTE profileIndex);
extern BYTE GetSingleBYTEProfileMeta(BYTE profileIndex, WORD metaDataIndex,BYTE subOffset);
extern BYTE GetProfileMetadaTLV(BYTE profileIndex, WORD tlvoffset, BYTE *outBuf);
extern WORD GetProfileOwnerSubField(BYTE profileIndex,BYTE subFieldType, BYTE *outBuf,BYTE mode);
extern BYTE StoreProfileMetadataUnspecifiedTagRef(UWORD dataAddr, BYTE profileIndex);
extern BYTE StoreProfileMetadataRef(BYTE *dataBuf, WORD valueLen, WORD refOffset, BYTE profileIndex );
extern BYTE StoreProfileMetadataTLV(BYTE *dataBuf, BYTE valueLen, WORD tlvOffset, BYTE profileIndex );
//extern BYTE StoreProfileMetadataBYTE(BYTE dataBuf, WORD tlvOffset, BYTE profileIndex );
///extern void setProfileStateByProfileIndex(BYTE status,BYTE ProfileIndex);

extern UWORD GetStoreDataAddrBytProfileIndex(BYTE profileIndex);
extern UWORD GetKeyDataAddrBytProfileIndex(BYTE profileIndex,BYTE keyVersion);


extern void DeleteProfilePackageInfo(BYTE deleteProfileIndex, UWORD profileAppTableAddr);
extern void DeleteProfileAppletInfo(BYTE deleteProfileIndex, UWORD profileAppTableAddr, BYTE mode);
extern void DeleteProfileConfigInfo(BYTE deleteProfileIndex);
extern void DeleteProfileReleaseMetadata(BYTE deleteProfileIndex);
extern void DownloadOrDeleteProfileResume(void);
extern void DeleteProfileDataByProfileIndex(BYTE deleteProfileIndex, BYTE mode);
extern void DeleteProfileByProfileIndex(BYTE ProfileIndex);

extern void DeleteProfileGlobalServiceList(BYTE deleteProfileIndex);
extern void SetProfileInfoAfterLoadPE(BYTE profileIndex);

#define GetProfileIndexByAID(aidBuf,aidLen)            GetProfileIndexByAIDAndState(aidBuf,aidLen,PROFILE_VALIDFLAG)
#define GetDownloadingProfileIndexByAID(aidBuf,aidLen) GetProfileIndexByAIDAndState(aidBuf,aidLen,PROFILE_LOADINGFLAG)

extern WORD GetProfileEntryRef(BYTE profileIndex);
extern UWORD GetProfileEntryAddr(BYTE profileIndex);
extern UWORD GetObjectItemAddr(UWORD ObjectAddr,BYTE itemIndex);
extern UWORD GetObject_ArrayItemAddr(UWORD profileEntryAddr,BYTE itemIndex);
extern UWORD GetISDPObjectAddr(BYTE profileIndex);
extern UWORD GetProfileMetaDataAddr(BYTE profileIndex);
extern UWORD MakeProfileArrayObject(WORD newContext,WORD Len,WORD* OutArrayRef,BYTE profileIndex);
extern WORD getISDPISDRContext(BYTE pkgId,BYTE appId);
extern BYTE GetCertType(BYTE profileIndex);
extern void SetCertType(BYTE profileIndex, BYTE certType);
extern  UWORD GetProfileConnectivityParaAddr(BYTE profileIndex);
extern BYTE ProfileOwnerIsExist(BYTE profileIndex);


#define  GetProfileEntryItemAddr              GetObjectItemAddr
#define  GetProfile_bFreeEntryIndexAddr(profileEntryAddr)         GetProfileEntryItemAddr(profileEntryAddr,Profile_bFreeEntryIndex_INDEX)
#define  GetProfile_theAppTableAddr(profileEntryAddr)             GetProfileEntryItemAddr(profileEntryAddr,Profile_theAppTable_INDEX)
#define  GetProfile_iAppIdFlagAddr(profileEntryAddr)              GetProfileEntryItemAddr(profileEntryAddr,Profile_iAppIdFlag_INDEX)
#define  GetProfile_iMutiFlagAddr(profileEntryAddr)               GetProfileEntryItemAddr(profileEntryAddr,Profile_iMutiFlag_INDEX)
#define  GetProfile_iExtraditonFlagAddr(profileEntryAddr)         GetProfileEntryItemAddr(profileEntryAddr,Profile_iExtraditonFlag_INDEX)
#define  GetProfile_pkgTableAddr(profileEntryAddr)                GetProfileEntryItemAddr(profileEntryAddr,profile_PkgTable_INDEX)
#define  GetProfile_pkgStatusAddr(profileEntryAddr)               GetProfileEntryItemAddr(profileEntryAddr,profile_pkgStatus_INDEX)
#define  GetProfile_pkgLocationTableAddr(profileEntryAddr)        GetProfileEntryItemAddr(profileEntryAddr,Profile_pkgLocationTable_INDEX)
#define  GetProfile_packageContextTableAddr(profileEntryAddr)     GetProfileEntryItemAddr(profileEntryAddr,Profile_packageContextTable_INDEX)
#define  GetProfile_appletPkgCountAddr(profileEntryAddr)          GetProfileEntryItemAddr(profileEntryAddr,Profile_appletPkgCount_INDEX)
#define  GetProfile_defaultAppIDAddr(profileEntryAddr)     	      GetProfileEntryItemAddr(profileEntryAddr,Profile_defaultAppID_INDEX)
#define  GetProfile_bCardStateAddr(profileEntryAddr)              GetProfileEntryItemAddr(profileEntryAddr,Profile_bCardState_INDEX)
//#define  GetProfile_PkIdRefAddr(profileEntryAddr)                 GetProfileEntryItemAddr(profileEntryAddr,Profile_PkIdRef_INDEX)
//#define  GetProfile_LTEAblityAddr(profileEntryAddr)               GetProfileEntryItemAddr(profileEntryAddr,Profile_LTEAblity_INDEX)
//#define  GetProfile_M2M_POL1Addr(profileEntryAddr)                GetProfileEntryItemAddr(profileEntryAddr,M2M_POL1_INDEX)

#define  GetProfileEntry_ArrayItemAddr      GetObject_ArrayItemAddr
#define  GetProfile_theAppTableValueAddr(profileEntryAddr)         GetProfileEntry_ArrayItemAddr(profileEntryAddr,Profile_theAppTable_INDEX)
#define  GetProfile_iAppIdFlagValueAddr(profileEntryAddr)          GetProfileEntry_ArrayItemAddr(profileEntryAddr,Profile_iAppIdFlag_INDEX)
#define  GetProfile_iMutiFlagValueAddr(profileEntryAddr)           GetProfileEntry_ArrayItemAddr(profileEntryAddr,Profile_iMutiFlag_INDEX)
#define  GetProfile_iExtraditonFlagValueAddr(profileEntryAddr)     GetProfileEntry_ArrayItemAddr(profileEntryAddr,Profile_iExtraditonFlag_INDEX)
#define  GetProfile_pkgTableValueAddr(profileEntryAddr)            GetProfileEntry_ArrayItemAddr(profileEntryAddr,profile_PkgTable_INDEX)
#define  GetProfile_pkgStatusValueAddr(profileEntryAddr)           GetProfileEntry_ArrayItemAddr(profileEntryAddr,profile_pkgStatus_INDEX)
#define  GetProfile_pkgLocationTableValueAddr(profileEntryAddr)    GetProfileEntry_ArrayItemAddr(profileEntryAddr,Profile_pkgLocationTable_INDEX)
#define  GetProfile_packageContextTableValueAddr(profileEntryAddr) GetProfileEntry_ArrayItemAddr(profileEntryAddr,Profile_packageContextTable_INDEX)
//#define  GetProfile_PkIdRefValueAddr(profileEntryAddr)             GetProfileEntry_ArrayItemAddr(profileEntryAddr,Profile_PkIdRef_INDEX)

/*以下4项内容放在ISDP里面*/
#define  GetProfile_PkIdRefAddr(ISDPObjectAddr)                 GetObjectItemAddr(ISDPObjectAddr,Profile_PkIdRef_INDEX)
#define  GetProfile_LTEAblityAddr(ISDPObjectAddr)               GetObjectItemAddr(ISDPObjectAddr,Profile_LTEAblity_INDEX)
#define  GetProfile_M2M_POL1Addr(ISDPObjectAddr)                GetObjectItemAddr(ISDPObjectAddr,M2M_POL1_INDEX)
#define  GetProfile_M2M_ConnectivityParaAddr(ISDPObjectAddr)    GetObjectItemAddr(ISDPObjectAddr,M2M_ConnectivityPara_INDEX)
#define  GetProfile_CertTypeAddr(ISDPObjectAddr)                GetObjectItemAddr(ISDPObjectAddr,CertType_INDEX)

#define  GetProfile_PkIdRefValueAddr(ISDPObjectAddr)            	 GetObject_ArrayItemAddr(ISDPObjectAddr,Profile_PkIdRef_INDEX)
#define  GetProfile_M2M_ConnectivityParaValueAddr(ISDPObjectAddr)    GetObject_ArrayItemAddr(ISDPObjectAddr,M2M_ConnectivityPara_INDEX)


#define  GetMetaDataItemAddr                     GetObjectItemAddr
#define  GetMetaData_profileStateAddr(MetaDataAddr)	                       GetMetaDataItemAddr(MetaDataAddr,MetaDataProfileState_INDEX)
#define  GetMetaData_profilePolicyRulesAddr(MetaDataAddr)                  GetMetaDataItemAddr(MetaDataAddr,MetaDataProfilePolicyRules_INDEX)
#define  GetMetaData_profileClassAddr(MetaDataAddr)                        GetMetaDataItemAddr(MetaDataAddr,MetaDataProfileClass_INDEX)
#define  GetMetaData_iccidAddr(MetaDataAddr)                               GetMetaDataItemAddr(MetaDataAddr,MetaDataIccid_INDEX)
#define  GetMetaData_iconTypeAddr(MetaDataAddr)                            GetMetaDataItemAddr(MetaDataAddr,MetaDataIconType_INDEX)
#define  GetMetaData_iconRefAddr(MetaDataAddr)                             GetMetaDataItemAddr(MetaDataAddr,MetaDataIconRef_INDEX)
#define  GetMetaData_serviceProviderNameRefAddr(MetaDataAddr)              GetMetaDataItemAddr(MetaDataAddr,MetaDataServiceProviderNameRef_INDEX)
#define  GetMetaData_profileNameRefAddr(MetaDataAddr)                      GetMetaDataItemAddr(MetaDataAddr,MetaDataProfileNameRef_INDEX)
#define  GetMetaData_notificationConfigurationInfoRefAddr(MetaDataAddr)    GetMetaDataItemAddr(MetaDataAddr,MetaDataNotificationConfigurationInfoRef_INDEX)
#define  GetMetaData_dpProprietaryDataRefAddr(MetaDataAddr)                GetMetaDataItemAddr(MetaDataAddr,MetaDataDpProprietaryDataRef_INDEX)
#define  GetMetaData_profileOwerRefAddr(MetaDataAddr)                      GetMetaDataItemAddr(MetaDataAddr,MetaDataProfileOwerRef_INDEX)
#define  GetMetaData_profileNickNameRefAddr(MetaDataAddr)                  GetMetaDataItemAddr(MetaDataAddr,MetaDataProfileNickNameRef_INDEX)
#define  GetMetaData_unspecifiedTagAddr(MetaDataAddr)                      GetMetaDataItemAddr(MetaDataAddr,MetaDataUnspecifiedTag_INDEX)

#define  GetMetaData_ArrayItemAddr     GetObject_ArrayItemAddr
#define  GetMetaData_iconRefValueAddr(MetaDataAddr)                              GetMetaData_ArrayItemAddr(MetaDataAddr,MetaDataIconRef_INDEX)
#define  GetMetaData_serviceProviderNameRefValueAddr(MetaDataAddr)               GetMetaData_ArrayItemAddr(MetaDataAddr,MetaDataServiceProviderNameRef_INDEX)
#define  GetMetaData_profileNameRefValueAddr(MetaDataAddr)                       GetMetaData_ArrayItemAddr(MetaDataAddr,MetaDataProfileNameRef_INDEX)
#define  GetMetaData_notificationConfigurationInfoRefValueAddr(MetaDataAddr)     GetMetaData_ArrayItemAddr(MetaDataAddr,MetaDataNotificationConfigurationInfoRef_INDEX)
#define  GetMetaData_dpProprietaryDataRefValueAddr(MetaDataAddr)                 GetMetaData_ArrayItemAddr(MetaDataAddr,MetaDataDpProprietaryDataRef_INDEX)
#define  GetMetaData_profileOwerRefValueAddr(MetaDataAddr)                       GetMetaData_ArrayItemAddr(MetaDataAddr,MetaDataProfileOwerRef_INDEX)
#define  GetMetaData_profileNickNameRefValueAddr(MetaDataAddr)                   GetMetaData_ArrayItemAddr(MetaDataAddr,MetaDataProfileNickNameRef_INDEX)
#define  GetMetaData_unspecifiedTagValueAddr(MetaDataAddr)                       GetMetaData_ArrayItemAddr(MetaDataAddr,MetaDataUnspecifiedTag_INDEX)

#endif

