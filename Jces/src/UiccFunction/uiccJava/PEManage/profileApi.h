#ifndef    _profileApi_h_
#define    _profileApi_h_
#include   "typedef.h"
#include    "tlvHeader.h"
#include  "Interface_LTEFromPlatform.h"

#if(1)
//暂时define******************************************************************

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

//WORD xdata G_LTEDirFileID; //当前激活的PROFILE的DIRFILEID,通常是2F00
extern WORD xdata G_LTEDirFileID; //当前激活的PROFILE的DIRFILEID,通常是2F00
//*********************************************************************************

#endif


#define POL_BITS        								0x07
#define PROFILEDOWNMODE                                0x00
#define NORMALMODE                                     0x01


//0x2F62 File_Offset
#define SEQ_NUMBER_OFFSET	0x0
#define Install_OFFSET	0x2
#define BF37_OFF					0x3
#define BF37_LEN_OFF1			0x5
#define BF37_LEN_OFF2			0x6


#define OFFSET_AUTHCFG_OPCFG             0x00
#define OFFSET_AUTHCFG_OPC               (OFFSET_AUTHCFG_OPCFG + 0x01)
#define OFFSET_AUTHCFG_R                 (OFFSET_AUTHCFG_OPC + 0x10)
#define OFFSET_AUTHCFG_C                 (OFFSET_AUTHCFG_R + 0x05)
#define OFFSET_AUTHCFG_IND               (OFFSET_AUTHCFG_C + 0x50)
#define OFFSET_AUTHCFG_L                 (OFFSET_AUTHCFG_IND + 0x01)
#define OFFSET_AUTHCFG_N                 (OFFSET_AUTHCFG_L + 0x06)
#define OFFSET_AUTHCFG_RESLEN            (OFFSET_AUTHCFG_N + 0x01)

//zhhj TUAK 
#define OFFSET_TUAK_AUTHCFG_TOPCMODE         0x00
#define OFFSET_TUAK_AUTHCFG_TOPC             (OFFSET_TUAK_AUTHCFG_TOPCMODE+0x01)
#define OFFSET_TUAK_AUTHCFG_KISIZE           (OFFSET_TUAK_AUTHCFG_TOPC+0x20)
#define OFFSET_TUAK_AUTHCFG_RESSIZE          (OFFSET_TUAK_AUTHCFG_KISIZE+0x01)
#define OFFSET_TUAK_AUTHCFG_CKLEN            (OFFSET_TUAK_AUTHCFG_RESSIZE+0x01)
#define OFFSET_TUAK_AUTHCFG_IKLEN            (OFFSET_TUAK_AUTHCFG_CKLEN+0x01)
#define OFFSET_TUAK_AUTHCFG_MACLEN           (OFFSET_TUAK_AUTHCFG_IKLEN+0x01)
#define OFFSET_TUAK_AUTHCFG_KIITA            (OFFSET_TUAK_AUTHCFG_MACLEN+0x01)
#define OFFSET_TUAK_AUTHCFG_IND              (OFFSET_TUAK_AUTHCFG_KIITA+0x01)
#define OFFSET_TUAK_AUTHCFG_L                (OFFSET_TUAK_AUTHCFG_IND+0x01)
#define OFFSET_TUAK_AUTHCFG_N                (OFFSET_TUAK_AUTHCFG_L+0x06)

/*---------------------function define------------------------------------------------*/

extern BYTE FindTagOffset(BYTE * dataPointer,BYTE tag,BYTE lgth,BYTE occ);


/*****FILE PE******/
extern BYTE profileModifyMFInfo(BYTE* fcp,WORD fcpLen);

extern BYTE profileCreateFile(BYTE* fcp,WORD fcpLen,BYTE type);

extern BYTE profileCreateMF(BYTE *fcp,BYTE fcpLen);

extern BYTE profileUpdateBinary(WORD off,BYTE* dataBuffer,WORD dataLen);

extern BYTE profileUpdateRecord(BYTE recNum,WORD off,BYTE* dataBuffer,WORD dataLen);

extern BYTE profileSelectByPath(BYTE* path,WORD pathLen);

/*****PUK PE******/

extern BYTE updatePukData(BYTE * pukData,BYTE pukLen);
/*****PIN PE******/

extern BYTE updatePinData(BYTE* pinBuf,BYTE pinBufLen);


/*****AKAParameter PE*******/

extern void setAuthCounterMax(BYTE * authCounter,BYTE len);

extern void setSqnInit(BYTE* sqnValue,BYTE len,BYTE index);

extern void setAuthCfg(BYTE sqnOption);

extern void setAlgorithmOptions(BYTE algorithmOptions);

extern void setSqnDelta(BYTE* sqnDelta,BYTE len,BYTE algorithmID);

extern void setSqnAgeLimit(BYTE* sqnAgeLimit,BYTE len);

extern BYTE setKI(BYTE type,BYTE *kiBuf, BYTE len);

extern void setHRPD (BYTE *kiBuf, BYTE len);

extern void setCHAP (BYTE *kiBuf, WORD len);

extern void setMobileIpSs(BYTE *kiBuf, WORD len);

extern void setMobileIpSsAddr(UWORD kiBufAddr, WORD len);

extern void WriteAuthCfgData(BYTE offset,BYTE * value,BYTE len);

extern BYTE createSpecProfilePrivateFile(BYTE localPin,BYTE num,WORD adfId);

extern BYTE checkPINPUKMaxRemain(BYTE maxRemain);
//zhhj

extern void setTuakAuthCounterMax(BYTE * authCounter,BYTE len);

extern BYTE getAlgorithmID(UWORD fileAddr);
/**************interface for ISD_R*********/

extern BYTE getProfileIndexByISDPid(BYTE ISDP_appid);

extern WORD deleteProfileComonent(BYTE ISDPAppid,BYTE profileindex);

/************function define In file  PEGenericFileManagement.c******************/
extern void makeDescript(void);


extern BYTE  PESearchAIDName(BYTE *pidrx);

extern BYTE WriteDIRNum(BYTE *aidBuf);





extern void PEGPGlobalVariableInit(void);
extern UWORD commonCreateDFFile(UWORD currentDfAddr,WORD fileName,BYTE *buf,BYTE len,BYTE mode);
extern UWORD commonCreateEFFile(UWORD currentDfAddr,WORD fileName,BYTE *buf,BYTE len,BYTE mode);
//extern WORD GetADFFID(BYTE *PartialAidBuf);
//extern UWORD GetFileHeadUsedSize(UWORD fileAddr);
extern UWORD SelectPrivateLFFile(WORD dfName, WORD efName);
extern BYTE AllocSpaceEnough(UWORD assignSize);
//extern void ShiftDataUncoil(BYTE *input,WORD len,BYTE *output,BYTE pos);

extern BYTE autoWriteEeprom(UWORD eepAddr,BYTE * SourceAddr,WORD size,BYTE mode);
extern BYTE autoSetEeprom(UWORD eepAddr,BYTE hex,WORD size,BYTE mode);
extern BYTE getMapFileArrNum(WORD mapFile0,WORD mapFile1,WORD mapFile2);

extern BYTE ProfileIndexIsValid(BYTE profileIndex);

//GetIfMfCreated //OK
//GetMaxProfileMFEnd //OK
//SetDownLoadingProfileMFOffset //OK
//SetDownLoadingProfilePOL1 //OK
//IF_Customer_IMSISatisfyCondition //OK
//IF_CustomerCheckTestProfile //OK
//IF_PEUpdateRestrictParameter //OK
//IF_SetProfileCapacityInfo //OK
//IF_SetAnalyzeExtraditeSDID //OK
//IF_SetAnalyzeLifecycle //OK
//IF_SetKeyCounter //OK
//IF_InitKeyCounter //OK

//IF_ExcuteProfileGPCommand
//IF_GPGetPkgIDByPkgAID //OK
//IF_GetAnalyzeAppIdByAIDList //OK

//IF_GetDownloadProfileIndex
//IF_CustomerGetDataFromMetadata
//IF_PEUpdateCATTPPersonalisation
//IF_PEUpdateContactlessProtocolParameters
//IF_SetDownLoadingProfileToValidState


#endif







