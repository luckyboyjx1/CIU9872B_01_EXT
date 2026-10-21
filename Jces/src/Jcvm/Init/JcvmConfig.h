#ifndef __BOOT_CONFIG_H_
#define __BOOT_CONFIG_H_

#include "DataOperation.h"
#include "profile.h"

#define		COS_JCVM_CODE_VERISON        0x04   //when Boot or Jcvm Code will be modified,this shall be added by one.it will compiled with code and it will showed in Sim or Usim ATR.

#define INS_INIT										0x72
#define INS_SET_LIFE_CYCLE_OF_BOOT						0x40
#define INS_SET_FILE_ID_SYS								0xE3
#define INS_CONFIG_APP									0xE4
#define INS_SET_ISD_DEFAULT_KEY_COUNTER    0xE5
#define INS_UPDATE_DEFAULT_UID							0x81

#define JCVM_ILLEGAL_CONFIG_DATA_INDEX					0xFFFF

#define JCVM_GET_CONFIG_DATA_OFFSET						0x00
#define JCVM_GET_CONFIG_DATA_LENGTH						0x01

#define JCVM_CONFIG_GROUP								0x10
#define GP_CONFIG_GROUP									0x20
#define FINANCE_CONFIG_GROUP							0x40

#define SYS_CONFIG_NO_ERROR								0x9000
#define SYS_CONFIG_SECURITY_STATUS_NOT_SATISFIED		0x6982
#define SYS_CONFIG_CLA_NOT_SUPPORTED					0x6E00
#define SYS_CONFIG_REFERENCED_DATA_NOT_FOUND			0x6A88

/*the configType of getConfig method*/
#define GET_LAST_BASIC_PACKAGE	  						0
#define CHECK_CARD_CREATED		  						1
#define SD_RSA_TOKEN_BYTE_LEN     						2
#define GET_CARD_STATE_INFO								5
#define INSTALL_FOR_LOAD_NO_HASH						6
#define INSTALL_FOR_OLD_ICC								7
#define SUPPORT_9F66_TAG								8
#define INSTALL_FOR_EID							    	9
#define PRO_SEC_NOT_RECEIVE								10
#define GP_REGISTER_ZHEJIANG_NONGXIN					11
#define GP_CONFIG_FOR_BFC							    12 // used for Specification name: GlobalPlatform Card Basic Financial Configuration
#define GET_STATUS_SUPPORT_SSD						    13
#define GET_MAX_RSA_MUD_BIT_LEN							14
#define GP_SCP02_PSEUDO_RANDOM							15
#define GP_GP_SPECIAL_TAG_00BF							16
#define GET_GUOMI_STANDARD								17
#define GP_ABC_RUWEI_TEST								18
#define	GP_OPTIMIZE_CREATE_CARDMANAGER					19
#define	GP_GET_PLATFORM_INFORMATION						20
#define GET_INIT_MASK_PACKAGE_NUMBER                    21
#define IS_CRYPTO_ALL_USE_RTR							22
#define	GET_LOCKED_APPLET_NUMBER                        23
#define SE_BE_0R_NOT                                    24
#define UICCCONFIG_TriggerToolkitApi                    25

#define GP_CONFIG_FOR_UICC_Configuration                26 //GP_THIRD_CONFIG bit0: this is used for GP UICC Test. Specification name: GlobalPlatform Card UICC Configuration
#define GP_CONFIG_FOR_MAPPING_TEST                      27 //GP_THIRD_CONFIG bit1: this is used for GP Mapping Guidelines Test. Specification name: GlobalPlatform Card Mapping Guidelines of Existing GP v2.1.1 Implementation on v2.2.1
#define GP_CONFIG_FOR_CI	     					    28 //GP_THIRD_CONFIG bit3: this is used for CI test. Specification name: GlobalPlatform Card Common Implementation Configuration
#define M2M_LocalManageProfile				     		29

#define	GP_INSTALL_FOR_LOAD_NO_HASH						0x01
#define	GP_INSTALL_FOR_OLD_ICC							0x02
#define	GP_ISD_SUPPORT_9F66_TAG							0x04
#define	GP_PROCESS_SECURITY_RECEIVE_JINDIAN				0x08
#define	GP_FOR_EID										0x10
#define	GP_REGISTER_FOR_ZHEJIANG_NONGXIN				0x20
#define	GP_FOR_BFC										0x80
#define OS_JC2_MODE		  	          					0x00
#define OS_JC3_MODE			  	          				0x10
#define OS_ISSUED_MODE		  	          				0x04
#define OS_TCK_MODE			  	          				0x00
#define OS_BCTC_MODE									0x80

void 	        JcvmConfig(void);
//extern u16  	getPrivateData(PN8 apduBuffer)    SMALL_MODE;
extern u16  	getConfig(u8 configType);
extern memref	JcvmConfig_GetVmConfigAddr(u32 offset) ;
u16             JcvmConfig_ConfigDataFunc(PX8 apduBuf, u8 funcType)  SMALL_MODE  ;

extern u16 JcvmConfig_UpdateIsdUid(PX8  apduBuffer);	//remove warning,dengyiwen,2022/9/26
//R
#define JcvmConfig_GetOsConfBase() 										U32_A_U32(VM_CONFIG_START, VM_OS_CONF)
#define JcvmConfig_GetOsConf()											PN_R_U8(MEMtPN8(JcvmConfig_GetOsConfBase()))
#define JcvmConfig_SetOsConf(config)									nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetOsConfBase()), config)

//R
#define JcvmConfig_GetOsLogChnNumBase()									U32_A_U32(VM_CONFIG_START, VM_OS_LOGICAL_CHANNEL_NUMBER)
#define JcvmConfig_GetOsLogChnNum() 									PN_R_U8(MEMtPN8(JcvmConfig_GetOsLogChnNumBase()))
#define JcvmConfig_SetOsLogChnNum(logChnNum)							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetOsLogChnNumBase()), logChnNum)
//R
#define JcvmConfig_GetOsDtrSizeBase()									U32_A_U32(VM_CONFIG_START, VM_OS_DTR_SIZE)
#define JcvmConfig_GetOsDtrSize() 										PN_R_U16(MEMtPN8(JcvmConfig_GetOsDtrSizeBase()))
#define JcvmConfig_SetOsDtrSize(dtrSize)								nvmWriteU16NoAtomic(MEMtPN8(JcvmConfig_GetOsDtrSizeBase()), dtrSize)

//R
#define JcvmConfig_GetMapBitTabSegBase()								U32_A_U32(VM_CONFIG_START, VM_E2P_CONFIG_MAP_BIT_TBL_SEGMENT)
#define JcvmConfig_GetMapBitTabSeg()									PN_R_U8(MEMtPN8(JcvmConfig_GetMapBitTabSegBase()))
#define JcvmConfig_SetMapBitTabSeg(MapBitTabSeg)						nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetMapBitTabSegBase()), MapBitTabSeg)

//R
#define JcvmConfig_GetFreeTabEntryBase()								U32_A_U32(VM_CONFIG_START, VM_E2P_CONFIG_FREE_TBL_ENTRY)
#define JcvmConfig_GetFreeTabEntry()									PN_R_U8(MEMtPN8(JcvmConfig_GetFreeTabEntryBase()))
#define JcvmConfig_SetFreeTabEntry(FreeTableNum)						nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetFreeTabEntryBase()), FreeTableNum)

//R
#define JcvmConfig_GetVmConfigBase(void)								U32_A_U32(VM_CONFIG_START, VM_CONFIG)
#define JcvmConfig_GetVmConfig(void)									PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigBase()))
#define JcvmConfig_SetVmConfig(vmConfig)								nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetVmConfigBase()), vmConfig)

//R
#define JcvmConfig_GetLoadConfigBase(void)								U32_A_U32(VM_CONFIG_START, VM_E2P_CONFIG_RFU)
#define JcvmConfig_GetLoadConfig(void)									PN_R_U8(MEMtPN8(JcvmConfig_GetLoadConfigBase()))
#define JcvmConfig_SetLoadConfig(loadConfig)							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetLoadConfigBase()), loadConfig)

#if  1
//R
#define JcvmConfig_GetGpConfBase() 										U32_A_U32(VM_CONFIG_START,GP_CONFIG)
#define JcvmConfig_GetGpConf()											PN_R_U8(MEMtPN8(JcvmConfig_GetGpConfBase()))
#define JcvmConfig_SetGpConf(configValue)								nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetGpConfBase()), configValue)

//=>新定义的配置项，没有地方使用
#define JcvmConfig_GetGpSecondConfBase() 								U32_A_U32(VM_CONFIG_START, GP_SECOND_CONFIG)
#define JcvmConfig_GetGpSecondConf() 									PN_R_U8(MEMtPN8(JcvmConfig_GetGpSecondConfBase()))
#define JcvmConfig_SetGpSecondConf(configValue)							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetGpSecondConfBase()), configValue)

//added by xuxiaojun@20220804 used for UL
#define JcvmConfig_GetGpThirdConfBase() 								U32_A_U32(VM_CONFIG_START, GP_THIRD_CONFIG)
#define JcvmConfig_GetGpThirdConf() 									PN_R_U8(MEMtPN8(JcvmConfig_GetGpThirdConfBase()))
#define JcvmConfig_SetGpThirdConf(configValue)							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetGpThirdConfBase()), configValue)

#define BIT_SUPPORT_SCP02_OR_SCP03 		0x01 //default:0	-- -- 0 indicates that support scp02, 1 indicates support scp03
#define BIT_SUPPORT_SCP11 				0x02 //default:0	-- -- 1 indicates that support scp11, 0 indicates not support scp11
#define BIT_SUPPORT_SCP80				0x04 //default:1	-- -- 1 indicates that support scp80, 0 indicates not support scp80
#define BIT_SUPPORT_SCP81				0x08 //default:1	-- -- 1 indicates that support scp81, 0 indicates not support scp81
//R W 
#define JcvmConfig_GetCmScpTypeBase()									U32_A_U32(VM_CONFIG_START, CM_SCP_TYPE)
#define JcvmConfig_GetCmScpType() 										PN_R_U8(MEMtPN8(JcvmConfig_GetCmScpTypeBase()))
#define JcvmConfig_SetCmScpType(scpType) 								nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetCmScpTypeBase()), scpType)

#define JcvmConfig_GetCmScpTypeIsSCP03									(U8_AND_U8(JcvmConfig_GetCmScpType() ,BIT_SUPPORT_SCP02_OR_SCP03))
#define JcvmConfig_GetCmScpTypeIsSupportSCP11							(U8_AND_U8(JcvmConfig_GetCmScpType() ,BIT_SUPPORT_SCP11))
#define JcvmConfig_GetCmScpTypeIsSupportSCP80							(U8_AND_U8(JcvmConfig_GetCmScpType() ,BIT_SUPPORT_SCP80))
#define JcvmConfig_GetCmScpTypeIsSupportSCP81							(U8_AND_U8(JcvmConfig_GetCmScpType() ,BIT_SUPPORT_SCP81))
//R
#define JcvmConfig_GetCmAuthMaxCountBase()								U32_A_U32(VM_CONFIG_START, CM_AUTH_MAX_TRY_CNTR)
#define JcvmConfig_GetCmAuthMaxCount()									PN_R_U8(MEMtPN8(JcvmConfig_GetCmAuthMaxCountBase()))
#define JcvmConfig_SetCmAuthMaxCount(cmAuthMaxCount)					nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetCmAuthMaxCountBase()), cmAuthMaxCount)

//R
#define JcvmConfig_GetISDRAIDLengthBase()								U32_A_U32(VM_CONFIG_START, ISDR_AIDLENGTH)
#define JcvmConfig_GetISDRAidLength() 									PN_R_U8(MEMtPN8(JcvmConfig_GetISDRAIDLengthBase()))
#define JcvmConfig_SetISDRAIDLength(AidLength)							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetISDRAIDLengthBase()), AidLength)

#define JcvmConfig_GetISDRAidBase()										(U32_A_U32(VM_CONFIG_START, ISDR_AIDVALUE))

#define JcvmConfig_GetCmAidLengthBase()									U32_A_U32(VM_CONFIG_START, CM_AID_LENGTH)
#define JcvmConfig_GetCmAidLength() 									PN_R_U8(MEMtPN8(JcvmConfig_GetCmAidLengthBase()))
#define JcvmConfig_SetCmAidLength(cmAidLength)							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetCmAidLengthBase()), cmAidLength)

//=>
#define JcvmConfig_GetCmAidBase()										(U32_A_U32(VM_CONFIG_START, CM_AID))

//R
#define JcvmConfig_GetCmLastBasicPkgBase()								U32_A_U32(VM_CONFIG_START, CM_LAST_BASIC_PACKAGE)
#define JcvmConfig_GetCmLastBasicPkg()									PN_R_U16(MEMtPN8(JcvmConfig_GetCmLastBasicPkgBase()))
#define JcvmConfig_SetCmLastBasicPkg(cmLastBasicPkg)					nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetCmLastBasicPkgBase()), cmLastBasicPkg)

//R
#define JcvmConfig_GetCmAppTabIndexBase()								U32_A_U32(VM_CONFIG_START, CM_AppTableIndex)
#define JcvmConfig_GetCmAppTabIndex()									PN_R_U8(MEMtPN8(JcvmConfig_GetCmAppTabIndexBase()))
#define JcvmConfig_SetCmAppTabIndex(cmAppTabIndex)						nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetCmAppTabIndexBase()), cmAppTabIndex)

//=>
#define JcvmConfig_GetCmFileAidBase()									U32_A_U32(VM_CONFIG_START, CM_FileAid)

//R
#define JcvmConfig_GetCmSdRsaTokenByteLenBase()							U32_A_U32(VM_CONFIG_START, CM_SD_RSA_TOKEN_BYTE_LEN)
#define JcvmConfig_GetCmSdRsaTokenByteLen() 							PN_R_U16(MEMtPN8(JcvmConfig_GetCmSdRsaTokenByteLenBase()))
#define JcvmConfig_SetCmSdRsaTokenByteLen(cmSdRsaTokenByteLen) 			nvmWriteU16NoAtomic(MEMtPN8(JcvmConfig_GetCmSdRsaTokenByteLenBase()), rsaTokenLen)

//=>
#define  PRODUCTION_LIFECYCLE_LENGTH     42   //Visa GlobalPlatform 2.1.1 Card Implementation Requirements v2.0.pdf:  Table 7 - Card Production Life Cycle Data
//public final static byte OFFSET_MODULE_FABRICATION = 18;
//public final static byte OFFSET_CARD_MANUFACTURER = 22;
//public final static byte OFFSET_PRE_PERSONALISER = 26;
//public final static byte OFFSET_PERSONALISER = 34;
#define JcvmConfig_GetCplcDataConfBase()								U32_A_U32(VM_CONFIG_START, CPLC_DATA_CONF)

#if  APP_BYD_CONFIG
#define JcvmConfig_GetSEIDBydBase()					         			U32_A_U32(VM_CONFIG_START, SEID_BYD)//added by xuxiaojun@20220621 比亚迪需求 seid
#endif

//R W
#define JcvmConfig_GetCmAuthTryCountBase()								U32_A_U32(VM_CONFIG_START, VM_CM_AUTH_TRY_CNTR)
#define JcvmConfig_GetCmAuthTryCount()									nvmReadU8(MEMtPN8(JcvmConfig_GetCmAuthTryCountBase()))
#define JcvmConfig_SetCmAuthTryCount(tryTime)							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetCmAuthTryCountBase()), tryTime)

//=>
#define JcvmConfig_GetCvmPinBase()										U32_A_U32(VM_CONFIG_START, VM_CVM_PIN)
#define JcvmConfig_GetCvmPin()
#define JcvmConfig_SetCvmPin(cvmPin)

//=>
#define JcvmConfig_GetKeySetsCounterBase() 								U32_A_U32(VM_CONFIG_START, VM_KEYSETS)
#define JcvmConfig_GetdefaultKeyBase()                                  U32_A_U32(JcvmConfig_GetKeySetsCounterBase(), 4)
#define JcvmConfig_SetKeySets(configValue)

//R
#define JcvmConfig_GetTokenKeyVersionBase()								U32_A_U32(VM_CONFIG_START, VM_TOKEN_KEY_VERSION)
#define JcvmConfig_GetTokenKeyVersion()									nvmReadU8(MEMtPN8(JcvmConfig_GetTokenKeyVersionBase()))
#define JcvmConfig_SetTokenKeyVersion(tokenKeyVersion)					nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetTokenKeyVersionBase()), tokenKeyVersion)

//R
#define JcvmConfig_GetTokenKeyIdBase()									U32_A_U32(VM_CONFIG_START, VM_TOKEN_KEY_ID)
#define JcvmConfig_GetTokenKeyId()										nvmReadU8(MEMtPN8(JcvmConfig_GetTokenKeyIdBase()))
#define JcvmConfig_SetTokenKeyId(tokenKeyId)							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetTokenKeyIdBase()), tokenKeyId)

//R
#define JcvmConfig_GetReceiptKeyVersionBase()							U32_A_U32(VM_CONFIG_START, VM_RECEIPT_KEY_VERSION)
#define JcvmConfig_GetReceiptKeyVersion()								nvmReadU8(MEMtPN8(JcvmConfig_GetReceiptKeyVersionBase()))
#define JcvmConfig_SetReceiptKeyVersion(receiptKeyVersion)				nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetReceiptKeyVersionBase()), receiptKeyVersion)

//R
#define JcvmConfig_GetReceiptKeyIdBase()								U32_A_U32(VM_CONFIG_START, VM_RECEIPT_KEY_ID)
#define JcvmConfig_GetReceiptKeyId()									nvmReadU8(MEMtPN8(JcvmConfig_GetReceiptKeyIdBase()))
#define JcvmConfig_SetReceiptKeyId(receiptKeyId)						nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetReceiptKeyIdBase()), receiptKeyId)

//R
#define JcvmConfig_GetDAPKeyVersionBase()								U32_A_U32(VM_CONFIG_START, VM_DAP_KEY_VERSION)
#define JcvmConfig_GetDAPKeyVersion()									nvmReadU8(MEMtPN8(JcvmConfig_GetDAPKeyVersionBase()))
#define JcvmConfig_SetDAPKeyVersion(dapKeyVersion)						nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetDAPKeyVersionBase()), dapKeyVersion)

//R
#define JcvmConfig_GetDAPKeyIdBase()									U32_A_U32(VM_CONFIG_START, VM_DAP_KEY_ID)
#define JcvmConfig_GetDAPKeyId()										nvmReadU8(MEMtPN8(JcvmConfig_GetDAPKeyIdBase()))
#define JcvmConfig_SetDAPKeyId(dapKeyId)								nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetDAPKeyIdBase()), dapKeyId)

//R
#define JcvmConfig_GetScpDefaultKeyVersionBase()						U32_A_U32(VM_CONFIG_START, VM_SCP_DEFAULT_KEY_VERSION)
#define JcvmConfig_GetScpDefaultKeyVersion()							nvmReadU8(MEMtPN8(JcvmConfig_GetScpDefaultKeyVersionBase()))
#define JcvmConfig_SetScpDefaultKeyVersion(scpDefaultKeyVersion)		nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetScpDefaultKeyVersionBase()), scpDefaultKeyVersion)

//R
#define JcvmConfig_GetMinSCPKeyVersionBase()							U32_A_U32(VM_CONFIG_START, VM_MIN_SCP_KEY_VERSION)
#define JcvmConfig_GetMinSCPKeyVersion()								nvmReadU8(MEMtPN8(JcvmConfig_GetMinSCPKeyVersionBase()))
#define JcvmConfig_SetMinSCPKeyVersion(minSCPKeyVersion)				nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetMinSCPKeyVersionBase()), minSCPKeyVersion)

//R
#define JcvmConfig_GetMaxSCPKeyVersionBase()							U32_A_U32(VM_CONFIG_START, VM_MAX_SCP_KEY_VERSION)
#define JcvmConfig_GetMaxSCPKeyVersion()								nvmReadU8(MEMtPN8(JcvmConfig_GetMaxSCPKeyVersionBase()))
#define JcvmConfig_SetMaxSCPKeyVersion(maxSCPKeyVersion)				nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetMaxSCPKeyVersionBase()), maxSCPKeyVersion)

//R
#define JcvmConfig_GetMinSCPKeyIdBase()									U32_A_U32(VM_CONFIG_START, VM_MIN_SCP_KEY_ID)
#define JcvmConfig_GetMinSCPKeyId()										nvmReadU8(MEMtPN8(JcvmConfig_GetMinSCPKeyIdBase()))
#define JcvmConfig_SetMinSCPKeyId(minSCPKeyId)							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetMinSCPKeyIdBase()), minSCPKeyId)

//R
#define JcvmConfig_GetMaxSCPKeyIdBase()									U32_A_U32(VM_CONFIG_START, VM_MAX_SCP_KEY_ID)
#define JcvmConfig_GetMaxSCPKeyId()										nvmReadU8(MEMtPN8(JcvmConfig_GetMaxSCPKeyIdBase()))
#define JcvmConfig_SetMaxSCPKeyId(maxSCPKeyId)							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetMaxSCPKeyIdBase()), maxSCPKeyId)

//R
#define JcvmConfig_GetSCPKeyIdStartIndexBase()							U32_A_U32(VM_CONFIG_START, VM_SCP_KEY_ID_START_INDEX)
#define JcvmConfig_GetSCPKeyIdStartIndex()								nvmReadU8(MEMtPN8(JcvmConfig_GetSCPKeyIdStartIndexBase()))
#define JcvmConfig_SetSCPKeyIdStartIndex(scpKeyIdStartIndex)			nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetSCPKeyIdStartIndexBase()), scpKeyIdStartIndex)

//R
#define JcvmConfig_GetIsdCountBase()									U32_A_U32(VM_CONFIG_START, VM_ISD_COUNT)
#define JcvmConfig_GetIsdCount()										PN_R_U8(MEMtPN8(JcvmConfig_GetIsdCountBase()))
#define JcvmConfig_SetIsdCount(isdCount)								nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetIsdCountBase()), isdCount)

//R
#define JcvmConfig_GetDefaultAppIDBase()								U32_A_U32(VM_CONFIG_START, VM_DEFAULT_APP_ID)
#define JcvmConfig_GetDefaultAppID()									PN_R_U8(MEMtPN8(JcvmConfig_GetDefaultAppIDBase()))
#define JcvmConfig_SetDefaultAppID(defaultAppID)						nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetDefaultAppIDBase()), defaultAppID)

//VM_KEYSETSLOADED

//R W
#define JcvmConfig_GetRegistryUpdateBase()								U32_A_U32(VM_CONFIG_START, VM_REGISTRY_UPDATEENABLE)
#define JcvmConfig_GetRegistryUpdate() 									nvmReadU8(MEMtPN8(JcvmConfig_GetRegistryUpdateBase()))
#define JcvmConfig_SetRegistryUpdate(flag) 								nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetRegistryUpdateBase()), flag)

//R
#define JcvmConfig_GetMAXFrameContextNUMBase()							U32_A_U32(VM_CONFIG_START, VM_MAX_FRAME_CONTEXT_NUM)
#define JcvmConfig_GetMAXFrameContextNUM()								PN_R_U8(MEMtPN8(JcvmConfig_GetMAXFrameContextNUMBase()))
#define JcvmConfig_setMAXFrameContextNUM(flag) 							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetMAXFrameContextNUMBase()), flag)

//R W
#define JcvmConfig_GetDiversifiedData1Base()							U32_A_U32(VM_CONFIG_START, DIVERSIFIED_DATA)

//R W
#define JcvmConfig_GetTmGuardtimeCBBase()							    U32_A_U32(VM_CONFIG_START, TM_GUARDTIME_CB)
#define JcvmConfig_GetTmGuardtimeCB() 									PN_R_U16(MEMtPN8(JcvmConfig_GetTmGuardtimeCBBase()))
#define JcvmConfig_SetTmGuardtimeCB(flag) 								nvmWriteU16NoAtomic(MEMtPN8(JcvmConfig_GetTmGuardtimeCBBase()), flag)

//R W
#define JcvmConfig_GetErrorHandleBase()									U32_A_U32(VM_CONFIG_START, ERROR_HANDLE)
#define JcvmConfig_GetErrorHandle() 									PN_R_U8(MEMtPN8(JcvmConfig_GetErrorHandleBase()))
#define JcvmConfig_SetErrorHandle(flag) 								nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetErrorHandleBase()), flag)

//R W
#define JcvmConfig_GetUsatTestBase()									U32_A_U32(VM_CONFIG_START, USAT_TEST)
#define JcvmConfig_GetUsatTest() 										PN_R_U8(MEMtPN8(JcvmConfig_GetUsatTestBase()))
#define JcvmConfig_SetUsatTest(flag) 									nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetUsatTestBase()), flag)

//R W
#define JcvmConfig_GetCGMsystemConsumeConfigBase()						U32_A_U32(VM_CONFIG_START, CGM_SYSTEM_CONSUME_CONFIG)
#define JcvmConfig_GetCGMsystemConsumeConfig() 							PN_R_U8(MEMtPN8(JcvmConfig_GetCGMsystemConsumeConfigBase()))
#define JcvmConfig_SetCGMsystemConsumeConfig(flag) 						nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetCGMsystemConsumeConfigBase()), flag)

//R W
#define JcvmConfig_GetAMDCEfcfgFlagBase()								U32_A_U32(VM_CONFIG_START, AMDC_EFCFG_FLAG)
#define JcvmConfig_GetAMDCEfcfgFlag() 									PN_R_U8(MEMtPN8(JcvmConfig_GetAMDCEfcfgFlagBase()))
#define JcvmConfig_SetAMDCEfcfgFlag(flag) 								nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetAMDCEfcfgFlagBase()), flag)

//R W
#define JcvmConfig_GetVMRFUBase()										U32_A_U32(VM_CONFIG_START, VM_RFU)
#define JcvmConfig_GetVMRFU() 											PN_R_U8(MEMtPN8(JcvmConfig_GetVMRFUBase()))
#define JcvmConfig_SetVMRFU(flag) 										nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetVMRFUBase()), flag)

//R W
#define JcvmConfig_GetUsimPersonBase()									U32_A_U32(VM_CONFIG_START, USIM_PERSO)
#define JcvmConfig_GetUsimPerson() 										PN_R_U8(MEMtPN8(JcvmConfig_GetUsimPersonBase()))
#define JcvmConfig_SetUsimPerson(flag) 									nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetUsimPersonBase()), flag)

//R W
#define JcvmConfig_GetOtaRemainSpaceBase()								U32_A_U32(VM_CONFIG_START, OTA_REMAIN_SPACE)
#define JcvmConfig_GetOtaRemainSpace() 									PN_R_U16(MEMtPN8(JcvmConfig_GetOtaRemainSpaceBase()))
#define JcvmConfig_SetOtaRemainSpace(flag) 								nvmWriteU16NoAtomic(MEMtPN8(JcvmConfig_GetOtaRemainSpaceBase()), flag)

//R W
#define JcvmConfig_GetVgpConfigBase()								    U32_A_U32(VM_CONFIG_START, VGP_CONFIG)
#define JcvmConfig_GetVgpConfig() 									    PN_R_U8(MEMtPN8(JcvmConfig_GetVgpConfigBase()))
#define JcvmConfig_SetVgpConfig(flag) 								    nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetVgpConfigBase()), flag)

//R W
#define JcvmConfig_GetCMappletLocketCountBase()							U32_A_U32(VM_CONFIG_START, CM_APPLET_LOCKED_COUNT)
#define JcvmConfig_GetCMappletLocketCount() 							PN_R_U16(MEMtPN8(JcvmConfig_GetCMappletLocketCountBase()))
#define JcvmConfig_SetCMappletLocketCount(flag) 						nvmWriteU16NoAtomic(MEMtPN8(JcvmConfig_GetCMappletLocketCountBase()), flag)

//R W
#define JcvmConfig_GetReqBeijingPersoBase()								U32_A_U32(VM_CONFIG_START, REQ_BEIJING_PERSO)
#define JcvmConfig_GetReqBeijingPerso() 								PN_R_U8(MEMtPN8(JcvmConfig_GetReqBeijingPersoBase()))
#define JcvmConfig_SetReqBeijingPerso(flag) 							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetReqBeijingPersoBase()), flag)

//R W
#define JcvmConfig_GetRfCommunicationSwitchBase()						U32_A_U32(VM_CONFIG_START, RF_COMMUNICATION_SWITCH)
#define JcvmConfig_GetRfCommunicationSwitch() 							PN_R_U8(MEMtPN8(JcvmConfig_GetRfCommunicationSwitchBase()))
#define JcvmConfig_SetRfCommunicationSwitch(flag) 						nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetRfCommunicationSwitchBase()), flag)

//R W
#define JcvmConfig_GetNoCurrentLimitBase()								U32_A_U32(VM_CONFIG_START, NO_CURRENT_LIMIT)
#define JcvmConfig_GetNoCurrentLimit() 									PN_R_U8(MEMtPN8(JcvmConfig_GetNoCurrentLimitBase()))
#define JcvmConfig_SetNoCurrentLimit(flag) 								nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetNoCurrentLimitBase()), flag)

//R W
#define JcvmConfig_GetAmendmentCTestBase()								U32_A_U32(VM_CONFIG_START, AMENDMENT_C_TEST)
#define JcvmConfig_GetAmendmentCTest() 									PN_R_U8(MEMtPN8(JcvmConfig_GetAmendmentCTestBase()))
#define JcvmConfig_SetAmendmentCTest(flag) 								nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetAmendmentCTestBase()), flag)

//R W
#define JcvmConfig_GetConfigTestBase()									U32_A_U32(VM_CONFIG_START, CONFIG_TEST)
#define JcvmConfig_GetConfigTest() 										PN_R_U8(MEMtPN8(JcvmConfig_GetConfigTestBase()))
#define JcvmConfig_SetConfigTest(flag) 									nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetConfigTestBase()), flag)

//R W
#define JcvmConfig_GetSupportSSDNestingBase()							U32_A_U32(VM_CONFIG_START, SUPPORT_SSD_NESTING)
#define JcvmConfig_GetSupportSSDNesting() 								PN_R_U8(MEMtPN8(JcvmConfig_GetSupportSSDNestingBase()))
#define JcvmConfig_SetSupportSSDNesting(flag) 							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetSupportSSDNestingBase()), flag)

//R W
#define JcvmConfig_GetScpOptionBase()							        U32_A_U32(VM_CONFIG_START, SCP_OPTION)
#define JcvmConfig_GetScpOption() 								        PN_R_U8(MEMtPN8(JcvmConfig_GetScpOptionBase()))
#define JcvmConfig_SetScpOption(flag) 							        nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetScpOptionBase()), flag)

//R W
#define JcvmConfig_GetGetDataScSwitchBase()							    U32_A_U32(VM_CONFIG_START, GETDATA_SC_SWITCH)
#define JcvmConfig_GetGetDataScSwitch() 								PN_R_U8(MEMtPN8(JcvmConfig_GetGetDataScSwitchBase()))
#define JcvmConfig_SetGetDataScSwitch(flag) 							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetGetDataScSwitchBase()), flag)

//R W
#define JcvmConfig_GetIsdPrivBase()							            U32_A_U32(VM_CONFIG_START, ISD_PRIV)
#define JcvmConfig_GetIsdPriv() 								        PN_R_U24(MEMtPN8(JcvmConfig_GetIsdPrivBase()))
#define JcvmConfig_SetIsdPriv(flag) 							        nvmWriteU24NoAtomic(MEMtPN8(JcvmConfig_GetIsdPrivBase()), flag)


//R W
#define JcvmConfig_GetStausSupportSSDBase()							    U32_A_U32(VM_CONFIG_START, VM_GETSTATUS_SUPPORT_SSD)
#define JcvmConfig_GetStausSupportSSD() 								PN_R_U8(MEMtPN8(JcvmConfig_GetStausSupportSSDBase()))
#define JcvmConfig_SetStausSupportSSD(flag) 							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetStausSupportSSDBase()), flag)

//R W
#define JcvmConfig_GetEnablePriAuthBase()							    U32_A_U32(VM_CONFIG_START, ENABLE_PRI_AUTH)
//#define JcvmConfig_GetEnablePriAuth()									PN_R_U8(MEMtPN8(JcvmConfig_GetEnablePriAuthBase()))
//#define JcvmConfig_SetEnablePriAuth(flag) 							 	nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetEnablePriAuthBase()), flag)
extern u8 JcvmConfig_GetEnablePriAuth(void);
extern void JcvmConfig_SetEnablePriAuth(u8 flag);


//R W
#define JcvmConfig_GetPriAuthAppAIDLenBase()							U32_A_U32(VM_CONFIG_START, PRI_AUTH_APPAID_LEN)
#define JcvmConfig_GetPriAuthAppAIDLen() 								PN_R_U8(MEMtPN8(JcvmConfig_GetPriAuthAppAIDLenBase()))
#define JcvmConfig_SetPriAuthAppAIDLen(flag) 							nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetPriAuthAppAIDLenBase()), flag)

//R W
#define JcvmConfig_GetSsdAuthCounterBase()							 	U32_A_U32(VM_CONFIG_START, SSD_AUTH_COUNTER)
#define JcvmConfig_GetSsdAuthCounter() 								 	PN_R_U8(MEMtPN8(JcvmConfig_GetSsdAuthCounterBase()))
#define JcvmConfig_SetSsdAuthCounter(flag) 							 	nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetSsdAuthCounterBase()), flag)

#define JcvmConfig_GetNFCRecordBase()							 	U32_A_U32(VM_CONFIG_START, NFC_RECORD)
#define JcvmConfig_GetNFCRecord() 									PN_R_U8(MEMtPN8(JcvmConfig_GetNFCRecordBase()))
#define JcvmConfig_SetNFCRecord(flag) 								nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetNFCRecordBase()), flag)

#define JcvmConfig_GetPollSystemDurationBase()							 	U32_A_U32(VM_CONFIG_START, POLLSYSTEM_DURATION)
#define JcvmConfig_GetPollSystemDuration() 									PN_R_U8(MEMtPN8(JcvmConfig_GetPollSystemDurationBase()))
#define JcvmConfig_SetPollSystemDuration(flag) 								nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetPollSystemDurationBase()), flag)

#define JcvmConfig_GetProcessProactiveBase()							 	U32_A_U32(VM_CONFIG_START, PROCESS_PROACTIVE)
#define JcvmConfig_GetProcessProactive() 									PN_R_U8(MEMtPN8(JcvmConfig_GetProcessProactiveBase()))
#define JcvmConfig_SetProcessProactive(flag) 								nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetProcessProactiveBase()), flag)

//R W
#define JcvmConfig_GetPriAuthAppAIDBase()							     U32_A_U32(VM_CONFIG_START, PRI_AUTH_APPAID)

//R W
#define JcvmConfig_GetVMReserveBaseBase()							    U32_A_U32(VM_CONFIG_START, VM_RESERVE_BASE)

#define JcvmConfig_GetOnLineStateBase()							        U32_A_U32(VM_CONFIG_START, E_CMCC_GETONLIESTATE)

#else

#define JcvmConfig_GetGpConfBase() 										0
#define JcvmConfig_GetGpConf() 											0
#define JcvmConfig_SetGpConf(configValue) 								0

#define JcvmConfig_GetGpSecondConfBase() 								0
#define JcvmConfig_GetGpSecondConf() 									0
#define JcvmConfig_SetGpSecondConf(configValue)							0

#define JcvmConfig_GetCmScpTypeBase() 									0
#define JcvmConfig_GetCmScpType() 										0
#define JcvmConfig_SetCmScpType(scpType) 								0
                                                                            
#define JcvmConfig_GetCmAuthMaxCountBase() 								0
#define JcvmConfig_GetCmAuthMaxCount() 									0
#define JcvmConfig_SetCmAuthMaxCount(cmAuthMaxCount) 					0
                                                                            
#define JcvmConfig_GetCmAidLengthBase() 								0
#define JcvmConfig_GetCmAidLength() 									0
#define JcvmConfig_SetCmAidLength(cmAidLength) 							0
                                                                            
//=>                                                                        
#define JcvmConfig_GetCmAidBase() 										0
                                                                            
#define JcvmConfig_GetCmLastBasicPkgBase() 								0
#define JcvmConfig_GetCmLastBasicPkg() 									0
#define JcvmConfig_SetCmLastBasicPkg(cmLastBasicPkg) 					0
                                                                            
#define JcvmConfig_GetCmAppTabIndexBase() 								0
#define JcvmConfig_GetCmAppTabIndex()									0
#define JcvmConfig_SetCmAppTabIndex(cmAppTabIndex) 						0
                                                                            
//=>                                                                        
#define JcvmConfig_GetCmFileAidBase() 									0
                                                                            
#define JcvmConfig_GetCmSdRsaTokenByteLenBase() 						0
#define JcvmConfig_GetCmSdRsaTokenByteLen() 							0
#define JcvmConfig_SetCmSdRsaTokenByteLen(cmSdRsaTokenByteLen) 			0

//=>                                                                        
#define JcvmConfig_GetCplcDataConfBase() 								0
                                                                            
#define JcvmConfig_GetCmAuthTryCountBase() 								0
#define JcvmConfig_GetCmAuthTryCount() 									0
#define JcvmConfig_SetCmAuthTryCount(tryTime) 							0
                                                                            
//=>                                                                        
#define JcvmConfig_GetCvmPinBase() 										0
#define JcvmConfig_GetCvmPin() 											0
#define JcvmConfig_SetCvmPin(cvmPin) 									0
                                                                            
//=>                                                                        
#define JcvmConfig_GetKeySetsBase() 									0
#define JcvmConfig_GetKeySets() 										0
#define JcvmConfig_SetKeySets(configValue) 								0
                                                                            
#define JcvmConfig_GetTokenKeyVersionBase() 							0
#define JcvmConfig_GetTokenKeyVersion() 								0
#define JcvmConfig_SetTokenKeyVersion(tokenKeyVersion) 					0
                                                                            
#define JcvmConfig_GetTokenKeyIdBase() 									0
#define JcvmConfig_GetTokenKeyId() 										0
#define JcvmConfig_SetTokenKeyId(tokenKeyId) 							0
                                                                            
#define JcvmConfig_GetReceiptKeyVersionBase() 							0
#define JcvmConfig_GetReceiptKeyVersion() 								0
#define JcvmConfig_SetReceiptKeyVersion(receiptKeyVersion) 				0
                                                                            
#define JcvmConfig_GetReceiptKeyIdBase() 								0
#define JcvmConfig_GetReceiptKeyId() 									0
#define JcvmConfig_SetReceiptKeyId(receiptKeyId) 						0
                                                                            
#define JcvmConfig_GetDAPKeyVersionBase() 								0
#define JcvmConfig_GetDAPKeyVersion() 									0
#define JcvmConfig_SetDAPKeyVersion(dapKeyVersion) 						0
                                                                            
#define JcvmConfig_GetDAPKeyIdBase() 									0
#define JcvmConfig_GetDAPKeyId() 										0
#define JcvmConfig_SetDAPKeyId(dapKeyId) 								0
                                                                            
#define JcvmConfig_GetScpDefaultKeyVersionBase() 						0
#define JcvmConfig_GetScpDefaultKeyVersion() 							0
#define JcvmConfig_SetScpDefaultKeyVersion(scpDefaultKeyVersion) 		0
                                                                            
#define JcvmConfig_GetMinSCPKeyVersionBase() 							0
#define JcvmConfig_GetMinSCPKeyVersion() 								0
#define JcvmConfig_SetMinSCPKeyVersion(minSCPKeyVersion) 				0
                                                                            
#define JcvmConfig_GetMaxSCPKeyVersionBase() 							0
#define JcvmConfig_GetMaxSCPKeyVersion() 								0
#define JcvmConfig_SetMaxSCPKeyVersion(maxSCPKeyVersion) 				0
                                                                            
#define JcvmConfig_GetMinSCPKeyIdBase() 								0
#define JcvmConfig_GetMinSCPKeyId() 									0
#define JcvmConfig_SetMinSCPKeyId(minSCPKeyId) 							0
                                                                            
#define JcvmConfig_GetMaxSCPKeyIdBase() 								0
#define JcvmConfig_GetMaxSCPKeyId() 									0
#define JcvmConfig_SetMaxSCPKeyId(maxSCPKeyId) 							0
                                                                            
#define JcvmConfig_GetSCPKeyIdStartIndexBase() 							0
#define JcvmConfig_GetSCPKeyIdStartIndex() 								0
#define JcvmConfig_SetSCPKeyIdStartIndex(scpKeyIdStartIndex) 			0
                                                                            
#define JcvmConfig_GetIsdCountBase() 									0
#define JcvmConfig_GetIsdCount() 										0
#define JcvmConfig_SetIsdCount(isdCount) 								0
                                                                            
#define JcvmConfig_GetDefaultAppIDBase() 								0
#define JcvmConfig_GetDefaultAppID() 									0
#define JcvmConfig_SetDefaultAppID(defaultAppID) 						0
                                                                            
//VM_KEYSETSLOADED                                                          
                                                                            
#define JcvmConfig_GetRegistryUpdateBase() 								0
#define JcvmConfig_GetRegistryUpdate() 									0
#define JcvmConfig_SetRegistryUpdate(flag) 								0
                                                                            
#define JcvmConfig_GetMAXFrameContextNUMBase() 							0
#define JcvmConfig_GetMAXFrameContextNUM() 								0
#define JcvmConfig_setMAXFrameContextNUM(flag) 							0

//R W
#define JcvmConfig_GetDiversifiedData1Base()							0
#define JcvmConfig_GetDiversifiedData1() 								0
#define JcvmConfig_SetDiversifiedData1(flag) 							0

//R W
#define JcvmConfig_GetTmGuardtimeCBBase()							    0
#define JcvmConfig_GetTmGuardtimeCB() 									0
#define JcvmConfig_SetTmGuardtimeCB(flag) 								0

//R W
#define JcvmConfig_GetErrorHandleBase()									0
#define JcvmConfig_GetErrorHandle() 									0
#define JcvmConfig_SetErrorHandle(flag) 								0

//R W
#define JcvmConfig_GetUsatTestBase()									0
#define JcvmConfig_GetUsatTest() 										0
#define JcvmConfig_SetUsatTest(flag) 									0

//R W
#define JcvmConfig_GetCGMsystemConsumeConfigBase()						0
#define JcvmConfig_GetCGMsystemConsumeConfig() 							0
#define JcvmConfig_SetCGMsystemConsumeConfig(flag) 						0

//R W
#define JcvmConfig_GetAMDCEfcfgFlagBase()								0
#define JcvmConfig_GetAMDCEfcfgFlag() 									0
#define JcvmConfig_SetAMDCEfcfgFlag(flag) 								0

//R W
#define JcvmConfig_GetRFUBase()											0
#define JcvmConfig_GetRFU() 											0
#define JcvmConfig_SetRFU(flag) 										0

//R W
#define JcvmConfig_GetUsimPersonBase()									0
#define JcvmConfig_GetUsimPerson() 										0
#define JcvmConfig_SetUsimPerson(flag) 									0

//R W
#define JcvmConfig_GetOtaRemainSpaceBase()								0
#define JcvmConfig_GetOtaRemainSpace() 									0
#define JcvmConfig_SetOtaRemainSpace(flag) 								0

//R W
#define JcvmConfig_GetVgpConfigBase()								    0
#define JcvmConfig_GetVgpConfig() 									    0
#define JcvmConfig_SetVgpConfig(flag) 								    0

//R W
#define JcvmConfig_GetCMappletLocketCountBase()							0
#define JcvmConfig_GetCMappletLocketCount() 							0
#define JcvmConfig_SetCMappletLocketCount(flag) 						0

//R W
#define JcvmConfig_GetReqBeijingPersoBase()								0
#define JcvmConfig_GetReqBeijingPerso() 								0
#define JcvmConfig_SetReqBeijingPerso(flag) 							0

//R W
#define JcvmConfig_GetRfCommunicationSwitchBase()						0
#define JcvmConfig_GetRfCommunicationSwitch() 							0
#define JcvmConfig_SetRfCommunicationSwitch(flag) 						0

//R W
#define JcvmConfig_GetNoCurrentLimitBase()								0
#define JcvmConfig_GetNoCurrentLimit() 									0
#define JcvmConfig_SetNoCurrentLimit(flag) 								0

//R W
#define JcvmConfig_GetAmendmentCTestBase()								0
#define JcvmConfig_GetAmendmentCTest() 									0
#define JcvmConfig_SetAmendmentCTest(flag) 								0

//R W
#define JcvmConfig_GetConfigTestBase()									0
#define JcvmConfig_GetConfigTest() 										0
#define JcvmConfig_SetConfigTest(flag) 									0

//R W
#define JcvmConfig_GetSupportSSDNestingBase()							0
#define JcvmConfig_GetSupportSSDNesting() 								0
#define JcvmConfig_SetSupportSSDNesting(flag) 							0

//R W
#define JcvmConfig_GetScpOptionBase()							        0
#define JcvmConfig_GetScpOption() 								        0
#define JcvmConfig_SetScpOption(flag) 							        0

//R W
#define JcvmConfig_GetGetDataScSwitchBase()							    0
#define JcvmConfig_GetGetDataScSwitch() 								0
#define JcvmConfig_SetGetDataScSwitch(flag) 							0

//R W
#define JcvmConfig_GetIsdPrivBase()							            0
#define JcvmConfig_GetIsdPriv() 								        0
#define JcvmConfig_SetIsdPriv(flag) 							        0

//R W
#define JcvmConfig_GetVMReserveBaseBase()							    0
#endif /*VM_GOP*/

#if  VM_WITH_SELECT_BY_FILE_ID
//=>
#define JcvmConfig_GetFileSysIdMapBase()	 							U32_A_U32(VM_CONFIG_START, FILE_ID_SYS_MAP)

#define JcvmConfig_GetFileSysIdMapSize()								FILE_ID_SYS_MAP_SIZE
#else
#define JcvmConfig_GetFileSysIdMapBase()	 							0
#define JcvmConfig_GetFileSysIdMapSize()	 							0
#endif 	/*VM_WITH_SELECT_BY_FILE_ID*/

#if SE_ONLY
//R W
#define JcvmConfig_GetCurrentExceptionListBase()						U32_A_U32(VM_CONFIG_START, CURRENT_EXCEPTION_LIST)
#else
extern u32 JcvmConfig_GetCurrentExceptionListBase(void);
#endif
#define JcvmConfig_GetCurrentExceptionList() 							nvmReadU32(MEMtPN8(JcvmConfig_GetCurrentExceptionListBase()))
#define JcvmConfig_SetCurrentExceptionList(flag) 						nvmWriteU32Atomic(MEMtPN8(JcvmConfig_GetCurrentExceptionListBase()),flag)//bug 381

#if SE_ONLY
#define JcvmConfig_GetDtpBase() 										U32_A_U32(VM_CONFIG_START, dTP)
#define JcvmConfig_GetMaxDtpBase()										U32_A_U32(VM_CONFIG_START, max_dTP)
#define JcvmConfig_GetRtpBase()											U32_A_U32(VM_CONFIG_START, rTP)
#else
extern u32 JcvmConfig_GetDtpBase(void);
extern u32 JcvmConfig_GetMaxDtpBase(void);
extern u32 JcvmConfig_GetRtpBase(void);
#endif
#define JcvmConfig_GetDtp() 											nvmReadU32(MEMtPN8(JcvmConfig_GetDtpBase()))
#define JcvmConfig_SetDtp(flag) 										nvmWriteU32Atomic(MEMtPN8(JcvmConfig_GetDtpBase()),flag)

#define JcvmConfig_GetMaxDtp()											nvmReadU16(MEMtPN8(JcvmConfig_GetMaxDtpBase()))
#define JcvmConfig_SetMaxDtp(flag) 										nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetMaxDtpBase()),flag)

#define JcvmConfig_GetRtp() 											nvmReadU16(MEMtPN8(JcvmConfig_GetRtpBase()))
#define JcvmConfig_SetRtp(flag) 										nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetRtpBase()),flag)
//=>
//R W 
#define JcvmConfig_GetRtpObjectDeletedBase() 							U32_A_U32(VM_CONFIG_START, rTP_OBJECT_DELETED)
#define JcvmConfig_GetRtpObjectDeleted() 								nvmReadU8(MEMtPN8(JcvmConfig_GetRtpObjectDeletedBase()))
#define JcvmConfig_SetRtpObjectDeleted(flag) 							nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetRtpObjectDeletedBase()),flag)

//=>
#define JcvmConfig_GetDtpObjectDeletedBase() 							U32_A_U32(VM_CONFIG_START, dTP_OBJECT_DELETED)
#define JcvmConfig_GetDtpObjectDeleted() 								nvmReadU32(MEMtPN8(JcvmConfig_GetDtpObjectDeletedBase()))
#define JcvmConfig_SetDtpObjectDeleted(flag) 							nvmWriteU32Atomic(MEMtPN8(JcvmConfig_GetDtpObjectDeletedBase()),flag)

//R W
#define JcvmConfig_GetApduBufferRefBase() 								U32_A_U32(VM_CONFIG_START, APDU_BUFFER)
#define JcvmConfig_GetApduBufferRef() 									nvmReadU16(MEMtPN8(JcvmConfig_GetApduBufferRefBase()))
#define JcvmConfig_SetApduBufferRef(flag) 								nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetApduBufferRefBase()),flag)

//R W 
#define JcvmConfig_GetShortBufferRefBase()								U32_A_U32(VM_CONFIG_START, SHORT_BUFFER)
#define JcvmConfig_GetShortBufferRef() 									nvmReadU16(MEMtPN8(JcvmConfig_GetShortBufferRefBase()))
#define JcvmConfig_SetShortBufferRef(flag) 								nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetShortBufferRefBase()),flag)


#define JcvmConfig_GetCacheBufferRefBase()								U32_A_U32(VM_CONFIG_START, CACHE_BUFFER)
#define JcvmConfig_GetCacheBufferRef() 									nvmReadU16(MEMtPN8(JcvmConfig_GetCacheBufferRefBase()))
#define JcvmConfig_SetCacheBufferRef(flag) 								nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetCacheBufferRefBase()),flag)


//R W 
#define JcvmConfig_GetRamBufferRefBase()								U32_A_U32(VM_CONFIG_START, RAM_BUFFER)
#define JcvmConfig_GetRamBufferRef() 									nvmReadU16(MEMtPN8(JcvmConfig_GetRamBufferRefBase()))
#define JcvmConfig_SetRamBufferRef(flag) 								nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetRamBufferRefBase()),flag)

//R W 
#define JcvmConfig_GetMaxEntryCountInMapBase()							U32_A_U32(VM_CONFIG_START, MAX_ENTRY_COUNT_IN_MAP)
#define JcvmConfig_GetMaxEntryCountInMap()								nvmReadU16(MEMtPN8(JcvmConfig_GetMaxEntryCountInMapBase()))
#define JcvmConfig_SetMaxEntryCountInMap(maxEntryCount) 				nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetMaxEntryCountInMapBase()),maxEntryCount)

//R W
#define JcvmConfig_GetUsedAddrNoBase() 									U32_A_U32(VM_CONFIG_START, USED_ADDR_NO)
#define JcvmConfig_GetUsedAddrNo() 										nvmReadU32(MEMtPN8(U32_A_U32(VM_CONFIG_START, USED_ADDR_NO)))
#define JcvmConfig_SetUsedAddrNo(flag) 									nvmWriteU32Atomic(MEMtPN8(U32_A_U32(VM_CONFIG_START, USED_ADDR_NO)),flag)

//R W 
#define JcvmConfig_GetJcreEntryEndBase() 								U32_A_U32(VM_CONFIG_START, JCRE_ENTRY_END)
#define JcvmConfig_GetJcreEntryEnd() 									PN_R_U16(MEMtPN8(JcvmConfig_GetJcreEntryEndBase()))
#define JcvmConfig_SetJcreEntryEnd(flag) 								nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetJcreEntryEndBase()),flag)

//R W 
#define JcvmConfig_GetVmGcRequestStateBase() 							U32_A_U32(VM_CONFIG_START, VM_GC_REQUESTED_STAET)
#define JcvmConfig_GetVmGcRequestState()  								nvmReadU8(MEMtPN8(JcvmConfig_GetVmGcRequestStateBase()))
#define JcvmConfig_SetVmGcRequestState(flag) 							nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetVmGcRequestStateBase()),flag)

//R W 
#define JcvmConfig_GetUsedAddrNOBAKBase()								U32_A_U32(VM_CONFIG_START, USED_ADDR_NO_BAK)
#define JcvmConfig_GetUsedAddrNOBAK() 									PN_R_U32(MEMtPN8(JcvmConfig_GetUsedAddrNOBAKBase()))
#define JcvmConfig_SetUsedAddrNoBAK(usedAddrNo)							nvmWriteU32Atomic(MEMtPN8(JcvmConfig_GetUsedAddrNOBAKBase()),usedAddrNo)

//R W 
#define JcvmConfig_GetOptimizeCardmanagerBase()							U32_A_U32(VM_CONFIG_START, OPTIMIZE_CARDMANAGER_CONFIG)
#define JcvmConfig_GetOptimizeCardmanager()								nvmReadU8(MEMtPN8(JcvmConfig_GetOptimizeCardmanagerBase()))
#define JcvmConfig_SetOptimizeCardmanager(tempValue)					nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetOptimizeCardmanagerBase()),tempValue)

//R W 
#define JcvmConfig_GetAdvanceAddrBase()									U32_A_U32(VM_CONFIG_START, ADVANCEOBJECT_ADDR)
#define JcvmConfig_GetAdvanceAddr()										nvmReadU32(MEMtPN8(JcvmConfig_GetAdvanceAddrBase()))
#define JcvmConfig_SetAdvanceAddr(tempValue)							nvmWriteU32Atomic(MEMtPN8(JcvmConfig_GetAdvanceAddrBase()),tempValue)

//R W
//#define JcvmConfig_GetFileSystemStartBase()								U32_A_U32(VM_CONFIG_START, FILE_SYSTEM_START)//该项配置暂时不使用了
//#define JcvmConfig_GetFileSystemStart()									nvmReadU32(MEMtPN8(JcvmConfig_GetFileSystemStartBase()))
//#define JcvmConfig_SetFileSystemStart(tempValue)						nvmWriteU32Atomic(MEMtPN8(JcvmConfig_GetFileSystemStartBase()),tempValue)


#if (VM_WITH_PBOC || VM_WITH_EDEP)
//=>该配置项未使用
#define JcvmConfig_GetPbocEdepCommonAreaBase()	 						U32_A_U32(VM_CONFIG_START, ECP_PBOC_EDEP_COMMON_AREA) 

//R
#define JcvmConfig_GetPseInstanceDirNumBase()							U32_A_U32(VM_CONFIG_START, ECP_PSE_INSTANCE_DIR_NUM) 
#define JcvmConfig_GetPseInstanceDirNum()								PN_R_U8(MEMtPN8(JcvmConfig_GetPseInstanceDirNumBase()))
#define JcvmConfig_SetPseInstanceDirNum(DirNum)							nvmWriteU32Atomic(MEMtPN8(JcvmConfig_GetPseInstanceDirNumBase()),DirNum)
#else
#define JcvmConfig_GetPbocEdepCommonAreaBase()	 						0 

#define JcvmConfig_GetPseInstanceDirNumBase()							0 
#define JcvmConfig_GetPseInstanceDirNum()								0
#define JcvmConfig_SetPseInstanceDirNum(DirNum)							0

#endif  /*VM_WITH_PBOC || VM_WITH_EDEP*/

#if  VM_WITH_PBOC
//R
#define JcvmConfig_GetPbocEepSizeBase()									U32_A_U32(VM_CONFIG_START, ECP_PBOC_INSTANCE_NVM_SIZE) 
#define JcvmConfig_GetPbocEepSize()	 									PN_R_U16(MEMtPN8(JcvmConfig_GetPbocEepSizeBase()))
#define JcvmConfig_SetPbocEepSize(PbocEepSize)							nvmWriteU32Atomic(MEMtPN8(JcvmConfig_GetPbocEepSizeBase()),PbocEepSize)

//R
#define JcvmConfig_GetPbocRamSizeBase()									U32_A_U32(VM_CONFIG_START, ECP_PBOC_INSTANCE_RAM_SIZE)
#define JcvmConfig_GetPbocRamSize()										PN_R_U16(MEMtPN8(JcvmConfig_GetPbocRamSizeBase())) 
#define JcvmConfig_SetPbocRamSize(PbocRamSize)							nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetPbocRamSizeBase()),PbocRamSize)

//R
#define JcvmConfig_GetPbocInstanceDirNumBase()							U32_A_U32(VM_CONFIG_START, ECP_PBOC_INSTANCE_DIR_NUM)
#define JcvmConfig_GetPbocInstanceDirNum()								PN_R_U8(MEMtPN8(JcvmConfig_GetPbocInstanceDirNumBase())) 
#define JcvmConfig_SetPbocInstanceDirNum(PbocInstanceDirNum)			nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetPbocInstanceDirNumBase()),PbocInstanceDirNum)

// NO R W 
#define JcvmConfig_GetPbocDefaultBase()	 								U32_A_U32(VM_CONFIG_START, ECP_PBOC_DEFAULT_CONFIG_AREA)
#define JcvmConfig_GetPbocDefault()										PN_R_U8(MEMtPN8(JcvmConfig_GetPbocDefaultBase())) 
#define JcvmConfig_SetPbocDefault(PbocDefault)							nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetPbocDefaultBase()),PbocDefault)

//R 
#define JcvmConfig_GetPpeEepSizeBase()	 								U32_A_U32(VM_CONFIG_START, ECP_PPE_INSTANCE_NVM_SIZE)
#define JcvmConfig_GetPpeEepSize()	 									PN_R_U16(MEMtPN8(JcvmConfig_GetPpeEepSizeBase())) 
#define JcvmConfig_SetPpeEepSize(PpeEepSize)							nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetPpeEepSizeBase()),PpeEepSize)

#else
#define JcvmConfig_GetPbocEepSizeBase()									0
#define JcvmConfig_GetPbocEepSize()	 									0
#define JcvmConfig_SetPbocEepSize(PbocEepSize)							0

#define JcvmConfig_GetPbocRamSizeBase()									0
#define JcvmConfig_GetPbocRamSize()										0
#define JcvmConfig_SetPbocRamSize(PbocRamSize)							0

#define JcvmConfig_GetPbocInstanceDirNumBase()							0
#define JcvmConfig_GetPbocInstanceDirNum()								0
#define JcvmConfig_SetPbocInstanceDirNum(PbocInstanceDirNum)			0

#define JcvmConfig_GetPbocDefaultBase()	 								0
#define JcvmConfig_GetPbocDefault()										0
#define JcvmConfig_SetPbocDefault(PbocDefault)							0

#define JcvmConfig_GetPpeEepSizeBase()	 								0
#define JcvmConfig_GetPpeEepSize()	 									0
#define JcvmConfig_SetPpeEepSize(PpeEepSize)							0
#endif /*VM_WITH_PBOC*/


#if  VM_WITH_EDEP
//NO R W
#define JcvmConfig_GetEdepMFKeyBase()	 								U32_A_U32(VM_CONFIG_START, ECP_EDEP_MF_KEY_VALUE)
#define JcvmConfig_GetEdepMFKey()		 								PN_R_U16(MEMtPN8(JcvmConfig_GetEdepMFKeyBase())) 
#define JcvmConfig_SetEdepMFKey(EdepMFKey)								nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetEdepMFKeyBase()),EdepMFKey)

//R
#define JcvmConfig_GetEdepEepSizeBase()									U32_A_U32(VM_CONFIG_START, ECP_EDEP_INSTANCE_NVM_SIZE)
#define JcvmConfig_GetEdepEepSize()										PN_R_U16(MEMtPN8(JcvmConfig_GetEdepEepSizeBase())) 
#define JcvmConfig_SetEdepEepSize(EdepEepSize)							nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetEdepMFKeyBase()),EdepEepSize)

//R
#define JcvmConfig_GetEdepInstanceDirNumBase()							U32_A_U32(VM_CONFIG_START, ECP_EDEP_INSTANCE_DIR_NUM)
#define JcvmConfig_GetEdepInstanceDirNum()								PN_R_U8(MEMtPN8(JcvmConfig_GetEdepInstanceDirNumBase()))
#define JcvmConfig_SetEdepInstanceDirNum(InstanceDirNum)				nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetEdepInstanceDirNumBase()),InstanceDirNum) 

//R
#define JcvmConfig_GetEdepInstanceProductTypeBase()						U32_A_U32(VM_CONFIG_START, ECP_EDEP_USER_CFG_PRODUCT_TYPE)
#define JcvmConfig_GetEdepInstanceProductType()							PN_R_U8(MEMtPN8(JcvmConfig_GetEdepInstanceProductTypeBase()))
#define JcvmConfig_SetEdepInstanceProductType(InstanceProductType)		nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetEdepInstanceProductTypeBase()),InstanceProductType)  

//R
#define JcvmConfig_GetEdepInstanceProductTypeExBase()					U32_A_U32(VM_CONFIG_START, ECP_EDEP_USER_CFG_PRODUCT_TYPE_EX) 
#define JcvmConfig_GetEdepInstanceProductTypeEx()						PN_R_U8(MEMtPN8(JcvmConfig_GetEdepInstanceProductTypeExBase()))
#define JcvmConfig_SetEdepInstanceProductTypeEx(ProductTypeEx)			nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetEdepInstanceProductTypeExBase()),ProductTypeEx)  

//R
#define JcvmConfig_GetEdepInstanceXaWalletAdvanceBasse()				U32_A_U32(VM_CONFIG_START, ECP_EDEP_USER_CFG_XA_WALLET_ADVANCE)
#define JcvmConfig_GetEdepInstanceXaWalletAdvance()						PN_R_U8(MEMtPN8(JcvmConfig_GetEdepInstanceXaWalletAdvanceBasse()))
#define JcvmConfig_SetEdepInstanceXaWalletAdvance(XaWalletAdvance)		nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetEdepInstanceXaWalletAdvanceBasse()),XaWalletAdvance)  

//=>
#define JcvmConfig_GetEdepInstanceTftVersionBase() 						U32_A_U32(VM_CONFIG_START, ECP_EDEP_USER_TFT_VERSION) 
#define JcvmConfig_GetEdepInstanceTftVersion()						PN_R_U8(MEMtPN8(JcvmConfig_GetEdepInstanceTftVersionBase()))
//=>
#define JcvmConfig_GetEdepRfuBase()	 									U32_A_U32(VM_CONFIG_START, ECP_EDEP_USER_CFG_RFU) 
#define JcvmConfig_GetEdepRfu()                                         PN_R_U8(MEMtPN8(JcvmConfig_GetEdepRfuBase()))
//=>
#define JcvmConfig_GetConstructAuthCodeBase()	 						U32_A_U32(VM_CONFIG_START, CONSTRUCT_AUTH_CODE)

//R
#define JcvmConfig_GetConstructAuthSelBase()							U32_A_U32(VM_CONFIG_START, CONSTRUCT_AUTH_SEL)
#define JcvmConfig_GetConstructAuthSel()								PN_R_U8(MEMtPN8(JcvmConfig_GetConstructAuthSelBase())) 
#define JcvmConfig_SetConstructAuthSel(ConstructAuthSel)				nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetConstructAuthSelBase()),ConstructAuthSel)

//NO R W 
#define JcvmConfig_GetConstructChipAddrBase()							U32_A_U32(VM_CONFIG_START, CONSTRUCT_AUTH_CHIP_ADDR)
#define JcvmConfig_GetConstructChipAddr()								PN_R_U32(MEMtPN8(JcvmConfig_GetConstructChipAddrBase())) 
#define JcvmConfig_SetConstructChipAddr(ConstructChipAddr)				nvmWriteU32Atomic(MEMtPN8(JcvmConfig_GetConstructChipAddrBase()),ConstructChipAddr)

#else
#define JcvmConfig_GetEdepMFKeyBase()	 								0
#define JcvmConfig_GetEdepMFKey()		 								0
#define JcvmConfig_SetEdepMFKey(EdepMFKey)								0

#define JcvmConfig_GetEdepEepSizeBase()									0
#define JcvmConfig_GetEdepEepSize()										0
#define JcvmConfig_SetEdepEepSize(EdepEepSize)							0

#define JcvmConfig_GetEdepInstanceDirNumBase()							0
#define JcvmConfig_GetEdepInstanceDirNum()								0
#define JcvmConfig_SetEdepInstanceDirNum(InstanceDirNum)				0

#define JcvmConfig_GetEdepInstanceProductTypeBase()						0
#define JcvmConfig_GetEdepInstanceProductType()							0
#define JcvmConfig_SetEdepInstanceProductType(InstanceProductType)		0

#define JcvmConfig_GetEdepInstanceProductTypeExBase()					0
#define JcvmConfig_GetEdepInstanceProductTypeEx()						0
#define JcvmConfig_SetEdepInstanceProductTypeEx(ProductTypeEx)			0

#define JcvmConfig_GetEdepInstanceXaWalletAdvanceBasse()				0
#define JcvmConfig_GetEdepInstanceXaWalletAdvance()						0
#define JcvmConfig_SetEdepInstanceXaWalletAdvance(XaWalletAdvance)		0

#define JcvmConfig_GetEdepInstanceTftVersionBase() 						0

#define JcvmConfig_GetEdepRfuBase()	 									0

#define JcvmConfig_GetConstructAuthCodeBase()	 						0

#define JcvmConfig_GetConstructAuthSelBase()							0
#define JcvmConfig_GetConstructAuthSel()								0
#define JcvmConfig_SetConstructAuthSel(ConstructAuthSel)				0

#define JcvmConfig_GetConstructChipAddrBase()							0
#define JcvmConfig_GetConstructChipAddr()								0
#define JcvmConfig_SetConstructChipAddr(ConstructChipAddr)				0

#endif  /*VM_WITH_EDEP*/

#if  (VM_WITH_PBOC && VM_WITH_EDEP && VM_WITH_JT_FUNC)  //函数调用已检查
//R W 没有地方调用
#define JcvmConfig_GetJTFileRefPBOCBase()								U32_A_U32(VM_CONFIG_START, ECP_JT_FILE_NVM_REF_PBOC)
#define JcvmConfig_GetJTFileRefPBOC()									nvmReadU16(MEMtPN8(JcvmConfig_GetJTFileRefPBOCBase()))
#define JcvmConfig_SetJTFileRefPBOC(addrInfo)	 						nvmCopyAtomic(MEMtPN8(JcvmConfig_GetJTFileRefPBOCBase()), addrInfo, 2)

//R W 
#define JcvmConfig_GetJTFileRefEDEPBase()								U32_A_U32(VM_CONFIG_START, ECP_JT_FILE_NVM_REF_EDEP)
#define JcvmConfig_GetJTFileRefEDEP()	 								nvmReadU16(MEMtPN8(JcvmConfig_GetJTFileRefEDEPBase()))
#define JcvmConfig_SetJTFileRefEDEP(addrInfo)	 						nvmCopyAtomic(MEMtPN8(JcvmConfig_GetJTFileRefEDEPBase()), addrInfo, 2)

//R W 
#define JcvmConfig_GetJTFileAddrEDEPBase()								U32_A_U32(VM_CONFIG_START, ECP_JT_FILE_NVM_SHARE_CUR_OFFSET)
#define JcvmConfig_GetJTFileAddrEDEP()	 								nvmReadU16(MEMtPN8(JcvmConfig_GetJTFileAddrEDEPBase()))
#define JcvmConfig_SetJTFileAddrEDEP(addrInfo)	 						nvmCopyAtomic(MEMtPN8(JcvmConfig_GetJTFileAddrEDEPBase()), addrInfo, 2)

//NO R W 
#define JcvmConfig_Get_SHARE_TLV_MAX_RECORDSBase()	 					U32_A_U32(VM_CONFIG_START, ECP_JT_FILE_NVM_SHARE_TLV_MAX_RECORDS)
#define JcvmConfig_Get_SHARE_TLV_MAX_RECORDS()	 						nvmReadU32(MEMtPN8(JcvmConfig_GetJTFileAddrEDEPBase()))
#define JcvmConfig_Set_SHARE_TLV_MAX_RECORDS(addrInfo)	 				nvmCopyAtomic(MEMtPN8(JcvmConfig_GetJTFileAddrEDEPBase()), addrInfo, 4)

//W 没有地方调用
#define JcvmConfig_GetJTAmountAddrBase()								U32_A_U32(VM_CONFIG_START, ECP_JT_RFU)
#define JcvmConfig_GetJTAmountAddr()									PN_R_U16(MEMtPN8(JcvmConfig_GetJTAmountAddrBase()))
#define JcvmConfig_SetJTAmountAddr(addrInfo)							nvmCopyAtomic(MEMtPN8(JcvmConfig_GetJTAmountAddrBase(), addrInfo, 2))

//R W 
#define JcvmConfig_GetJTECEPLockInfoBase() 								U32_A_U32(VM_CONFIG_START, ECP_JT_ECEP_LOCK_INFO)
#define JcvmConfig_GetJTECEPLockInfo() 									nvmReadU8(MEMtPN8(JcvmConfig_GetJTECEPLockInfoBase()))
#define JcvmConfig_SetJTECEPLockInfo(lockInfo)	 						nvmWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetJTECEPLockInfoBase()), lockInfo)

//无此配置项
#define JcvmConfig_GetJTFileAddr1EBase()								U32_A_U32(VM_CONFIG_START, ECP_JT_FILE_NVM_REF_1E)
#define JcvmConfig_GetJTFileAddr1E()									PN_R_U16(MEMtPN8(JcvmConfig_GetJTFileAddr1EBase()))
#define JcvmConfig_SetJTFileAddr1E(addrInfo)							nvmCopyAtomic(MEMtPN8(JcvmConfig_GetJTFileAddr1EBase(), addrInfo, 2))

//W
#define JcvmConfig_GetJTRFUBase()										U32_A_U32(VM_CONFIG_START, ECP_JT_RFU)
#define JcvmConfig_GetJTRFU()											PN_R_U16(MEMtPN8(JcvmConfig_GetJTRFUBase()))
#define JcvmConfig_SetJTRFU(info)										nvmCopyAtomic(MEMtPN8(JcvmConfig_GetJTRFUBase, info, 2))

#else
#define JcvmConfig_GetJTFileRefPBOCBase()								0
#define JcvmConfig_GetJTFileRefPBOC()									0
#define JcvmConfig_SetJTFileRefPBOC(addrInfo)	 						0

#define JcvmConfig_GetJTFileRefEDEPBase()								0
#define JcvmConfig_GetJTFileRefEDEP()	 								0
#define JcvmConfig_SetJTFileRefEDEP(addrInfo)	 						0

#define JcvmConfig_GetJTFileAddrEDEPBase()								0
#define JcvmConfig_GetJTFileAddrEDEP()	 								0
#define JcvmConfig_SetJTFileAddrEDEP(addrInfo)	 						0

#define JcvmConfig_Get_SHARE_TLV_MAX_RECORDSBase()	 					0
#define JcvmConfig_Get_SHARE_TLV_MAX_RECORDS()	 						0
#define JcvmConfig_Set_SHARE_TLV_MAX_RECORDS(addrInfo)	 				0

#define JcvmConfig_GetJTAmountAddrBase()								0
#define JcvmConfig_GetJTAmountAddr()									0
#define JcvmConfig_SetJTAmountAddr(addrInfo)							0

#define JcvmConfig_GetJTECEPLockInfoBase() 								0
#define JcvmConfig_GetJTECEPLockInfo() 									0
#define JcvmConfig_SetJTECEPLockInfo(lockInfo)	 						0

#define JcvmConfig_GetJTFileAddr1EBase()								0
#define JcvmConfig_GetJTFileAddr1E()									0
#define JcvmConfig_SetJTFileAddr1E(addrInfo)							0

#define JcvmConfig_GetJTRFUBase()										0
#define JcvmConfig_GetJTRFU()											0
#define JcvmConfig_SetJTRFU(info)										0

#endif  /* (VM_WITH_PBOC && VM_WITH_EDEP && VM_WITH_JT_FUNC)*/

#if  VM_WITH_EID //函数调用已检查
//R 
#define JcvmConfig_GetEIDEepSizeBase()									U32_A_U32(VM_CONFIG_START, ECP_EID_INSTANCE_NVM_SIZE)
#define JcvmConfig_GetEIDEepSize()										PN_R_U16(MEMtPN8(JcvmConfig_GetEIDEepSizeBase()))
#define JcvmConfig_SetEIDEepSize(EIDEepSize)							nvmWriteU32Atomic(MEMtPN8(JcvmConfig_GetEIDEepSizeBase()),EIDEepSize)

//R 
#define JcvmConfig_GetEIDRamSizeBase()									U32_A_U32(VM_CONFIG_START, ECP_EID_INSTANCE_RAM_SIZE)
#define JcvmConfig_GetEIDRamSize()										PN_R_U16(MEMtPN8(JcvmConfig_GetEIDRamSizeBase()))
#define JcvmConfig_SetEIDRamSize(EIDRamSize)							nvmWriteU32Atomic(MEMtPN8(JcvmConfig_GetEIDRamSizeBase()),EIDRamSize)

//NO R W 
#define JcvmConfig_GetEIDDefaultBase()									U32_A_U32(VM_CONFIG_START, ECP_EID_DEFAULT_CONFIG_AREA)
#define JcvmConfig_GetEIDDefault()										PN_R_U16(MEMtPN8(JcvmConfig_GetEIDDefaultBase()))
#define JcvmConfig_SetEIDDefault(EIDDefault)							nvmWriteU32Atomic(MEMtPN8(JcvmConfig_GetEIDDefaultBase()),EIDDefault)

#else
#define JcvmConfig_GetEIDEepSizeBase()									0
#define JcvmConfig_GetEIDEepSize()										0
#define JcvmConfig_SetEIDEepSize(EIDEepSize)							0

#define JcvmConfig_GetEIDRamSizeBase()									0
#define JcvmConfig_GetEIDRamSize()										0
#define JcvmConfig_SetEIDRamSize(EIDRamSize)							0

#define JcvmConfig_GetEIDDefaultBase()									0
#define JcvmConfig_GetEIDDefault()										0
#define JcvmConfig_SetEIDDefault(EIDDefault)							0

#endif /*VM_WITH_EID*/


#define JcvmConfig_GetE_SUSPEND_TOKENBase()									U32_A_U32(VM_CONFIG_START, E_SUSPEND_TOKEN)
#define JcvmConfig_GetE_TERMINAL_CAPBILITYBase()							U32_A_U32(VM_CONFIG_START, E_TERMINAL_CAPBILITY)
#define JcvmConfig_GetE_SUSPENDSTATUSBase()									U32_A_U32(VM_CONFIG_START, E_SUSPENDSTATUS)


#define JcvmConfig_GetFunctionSwitchBase()									U32_A_U32(VM_CONFIG_START, CONFIG_EUICC)
#define JcvmConfig_GetFunctionSwitchOriginal()								nvmReadU8(MEMtPN8(JcvmConfig_GetFunctionSwitchBase()))
#define JcvmConfig_SetFunctionSwitch(version)				                nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetFunctionSwitchBase()),version)

#define	EUICC__REUSE_OTPK				  	  								U8_AND_U8(JcvmConfig_GetFunctionSwitchOriginal(), 0x80)
#define	UICC__FUNCTION__DFLink__Cfg				  	  				        U8_AND_U8(JcvmConfig_GetFunctionSwitchOriginal(), 0x40)
#define	EUICC__SAVE_SESSIONKEY				  	  						    U8_AND_U8(JcvmConfig_GetFunctionSwitchOriginal(), 0x20)
#define MNO_SELF_EXTRADITE                                					U8_AND_U8(JcvmConfig_GetFunctionSwitchOriginal(), 0x10)
#define ADF_SUPPORT_SFI                                   					U8_AND_U8(JcvmConfig_GetFunctionSwitchOriginal(), 0x08)
#define RedTea_Mobile                                   					U8_AND_U8(JcvmConfig_GetFunctionSwitchOriginal(), 0x04)
#define UICC__FUNCTION__BERTLV      										U8_AND_U8(JcvmConfig_GetFunctionSwitchOriginal(), 0x02)//与UICC__FUNCTION__BERTLV配置互换，与产业化其它平台保持一致
#define Germany_Mobile                                   					U8_AND_U8(JcvmConfig_GetFunctionSwitchOriginal(), 0x01)

#define JcvmConfig_GetOPeratorBase()										U32_A_U32(VM_CONFIG_START, OPERATOR)
#define JcvmConfig_GetOPerat()								   			nvmReadU8(MEMtPN8(JcvmConfig_GetOPeratorBase()))
#define JcvmConfig_SetOPerator(version)				                		nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetOPeratorBase()),version)

//#define GetOPerator() 														(JcvmConfig_GetOPeratorer() & 0x07)
#define	MOBILE_Operator				  	  							    	  U8_AND_U8(JcvmConfig_GetOPerat(), 0x07)

//同步2.1配置，避免配置冲突
#define	UICC__SET_UNIVELSALPIN_USE				  	  				  		U8_AND_U8(JcvmConfig_GetOPerat(), 0x08)
#define	UICC__DONOT_ENABLEPIN_WHEN_BLOCK				  	      			U8_AND_U8(JcvmConfig_GetOPerat(), 0x10)
#define	O_RESP_OFFSET				  	                               		U8_AND_U8(JcvmConfig_GetOPerat(), 0x20)
#define	YANG_CHENG_TONG_CFG				  	                                U8_AND_U8(JcvmConfig_GetOPerat(), 0x40)
#define Resend_Notification_Immediately                          			U8_AND_U8(JcvmConfig_GetOPerat(), 0x80) // 1 立即重发，默认值
#define Check_GID1GID2                          							U8_AND_U8(JcvmConfig_GetOPerat(), 0x40) // 1检查是否匹配，默认值(RSP)

#define JcvmConfig_GetConfigUiccBase()										U32_A_U32(VM_CONFIG_START, CONFIG_UICC)
#define JcvmConfig_GetConfigUicc()								    		nvmReadU8(MEMtPN8(JcvmConfig_GetConfigUiccBase()))
#define JcvmConfig_SetConfigUicc(version)				                	nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetConfigUiccBase()),version)

//https私有指令配置的可选功能开关, GPAnnexB_TLS_EXTEND_VERSION 字节的各个bit含义
#define BIT_GP__AMENDMENT__B__DNS_Version									4
#define BIT_GP__AMENDMENT__B__SUPPORT_SESSION_RETRY							5

#define JcvmConfig_GetTlsExtandVersionBase()							    U32_A_U32(VM_CONFIG_START, TLS_EXTAND_VERSION)
#define JcvmConfig_GetTlsExtandVersion()								    nvmReadU8(MEMtPN8(JcvmConfig_GetTlsExtandVersionBase()))
#define JcvmConfig_SetTlsExtandVersion(version)				                nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetTlsExtandVersionBase()),version) 

#define JcvmConfig_GetHttpsOpenChannelRetryTimes()                         ((0xF0 & (JcvmConfig_GetAmendmentBTriggerPor())) >> 4)

#define BIT_GP__AMENDMENT__B__Optional_Tag                                   0 //default:0 -- 1 indicates that Optional Tag should return; 0 indicates should not return
#define BIT_GP__AMENDMENT__B__ConnectionPara_With_ISD                        1 //default:0 -- 1 indicates that connection parameter should search up to ISD; 0 indicates not consider ISD
#define BIT_GP__AMENDMENT__B__ConnectionPara_With_TriggeredSD                2 //default:1 -- 1 indicates that connection parameter should search up to triggered SD; 0 indicates not consider triggered SD
#define BIT_GP__AMENDMENT__B__Channel_Status                                 3 //default:1 -- 1 indicates that send close channel immediately; 0 indicates that send close channel after alert
#define BIT_GP__AMENDMENT__B__Open_Flag                                      4 //default:1 -- 1 indicates that https session open since successfully sended clientHello, 0 indicates that send https session open since successfully processed trigger message
#define BIT_GP__AMENDMENT__B__Content_Type                                   5 //default:1 -- 1 indicates that use HTTP_ContentType_LVMode in the send; 0 indicates HTTP_ContentType_ChunkedMode
#define BIT_GP__AMENDMENT__B__OpenChannel_With_3X_Retry                      6 //default:0 -- 1 indicates that when openchannel response with 3x need to retry openchannel by retry policy; 0 indicates not need
#define BIT_GP__AMENDMENT__B__Bip_ReceiveData_ProactiveCommand_B7_Transfer   7 //default:0 -- 1 indicates that when the value of B7 tag in CMD_RECEIVEDATA ProactiveCommand response greater than 0xED, need transfer to 0xED; 0 indicates need not to transfer

#define JcvmConfig_GetAmendmentBConfigBase()							    U32_A_U32(VM_CONFIG_START, AMENDMENT_B_CONFIG)
#define JcvmConfig_GetAmendmentBConfig()								    nvmReadU8(MEMtPN8(JcvmConfig_GetAmendmentBConfigBase()))
#define JcvmConfig_SetAmendmentBConfig(version)				                nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetAmendmentBConfigBase()),version)

#define JcvmConfig_GetAmendmentBTriggerPorBase()							U32_A_U32(VM_CONFIG_START, AMENDMENT_B_Trigger_SMS_POR)
#define JcvmConfig_GetAmendmentBTriggerPor()								nvmReadU8(MEMtPN8(JcvmConfig_GetAmendmentBTriggerPorBase()))
#define JcvmConfig_SetAmendmentBTriggerPor(version)				            nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetAmendmentBTriggerPorBase()),version)

#define JcvmConfig_GetRSPLastOperationaProfileBase()						U32_A_U32(VM_CONFIG_START, RSP_LAST_OPERATIONAL_PROFILE)
#define JcvmConfig_GetRSPLastOperationaProfile()							nvmReadU8(MEMtPN8(JcvmConfig_GetRSPLastOperationaProfileBase()))
#define JcvmConfig_SetRSPLastOperationaProfile(index)				        nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetRSPLastOperationaProfileBase()),index)

#define JcvmConfig_GetProfileEnableIndexBase()							    U32_A_U32(VM_CONFIG_START, PROFILE_ENABLEINDEX_CFG)
#define JcvmConfig_GetProfileEnableIndex()								    nvmReadU8(MEMtPN8(JcvmConfig_GetProfileEnableIndexBase()))
#define JcvmConfig_SetProfileEnableIndex(index)				                nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetProfileEnableIndexBase()),index)

#define JcvmConfig_GetUpdataEnableProfileIndexBase()					    U32_A_U32(VM_CONFIG_START, UPDATA_ENABLE_PROFILE_INDEX)
#define JcvmConfig_GetUpdataEnableProfileIndex()						    nvmReadU8(MEMtPN8(JcvmConfig_GetUpdataEnableProfileIndexBase()))
#define JcvmConfig_SetUpdataEnableProfileIndex(index)				        nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetUpdataEnableProfileIndexBase()),index)

#define JcvmConfig_GetUpdataDisableProfileIndexBase()					    U32_A_U32(VM_CONFIG_START, UPDATA_DISABLE_PROFILE_INDEX)
#define JcvmConfig_GetUpdataDisableProfileIndex()						    nvmReadU8(MEMtPN8(JcvmConfig_GetUpdataDisableProfileIndexBase()))
#define JcvmConfig_SetUpdataDisableProfileIndex(index)				        nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetUpdataDisableProfileIndexBase()),index)

#define JcvmConfig_GetUpdateChangeProfileInitPrivateFileIndexBase()		    U32_A_U32(VM_CONFIG_START, UPDATE_CHANGE_PROFILE_INIT_PRIFILE_INDEX)
#define JcvmConfig_GetUpdateChangeProfileInitPrivateFileIndex()			    nvmReadU8(MEMtPN8(JcvmConfig_GetUpdateChangeProfileInitPrivateFileIndexBase()))
#define JcvmConfig_SetUpdateChangeProfileInitPrivateFileIndex(index)	    nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetUpdateChangeProfileInitPrivateFileIndexBase()),index)

#define JcvmConfig_GetM2MFallBackProfileIndexBase()		    				U32_A_U32(VM_CONFIG_START, M2M_FALLBACK_PROFILE_INDEX)
#define JcvmConfig_GetM2MFallBackProfileIndex()								nvmReadU8(MEMtPN8(JcvmConfig_GetM2MFallBackProfileIndexBase()))
#define JcvmConfig_SetM2MFallBackProfileIndex(index)						nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetM2MFallBackProfileIndexBase()),index)

#define JcvmConfig_GetM2MFallBackDisabledNoAllowProfileIndexBase()		    U32_A_U32(VM_CONFIG_START, M2M_FALLBACK_NOALLOWDISABLE_PROFILEINDEX)
#define JcvmConfig_GetM2MFallBackDisabledNoAllowProfileIndex()				nvmReadU8(MEMtPN8(JcvmConfig_GetM2MFallBackDisabledNoAllowProfileIndexBase()))
#define JcvmConfig_SetM2MFallBackDisabledNoAllowProfileIndex(index)			nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetM2MFallBackDisabledNoAllowProfileIndexBase()),index)

#define JcvmConfig_GetM2MLastDisableProfileIndexBase()		    			U32_A_U32(VM_CONFIG_START, M2M_LASTDISABLE_PROFILEINDEX)
#define JcvmConfig_GetM2MLastDisableProfileIndex()							nvmReadU8(MEMtPN8(JcvmConfig_GetM2MLastDisableProfileIndexBase()))
#define JcvmConfig_SetM2MLastDisableProfileIndex(index)						nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetM2MLastDisableProfileIndexBase()),index)

#define JcvmConfig_GetM2MNotificationTypeBase()		    					U32_A_U32(VM_CONFIG_START, M2M_NOTIFICATION_TYPE)
#define JcvmConfig_GetM2MNotificationType()									nvmReadU8(MEMtPN8(JcvmConfig_GetM2MNotificationTypeBase()))
#define JcvmConfig_SetM2MNotificationType(Type)								nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetM2MNotificationTypeBase()),Type)

#if M2M_ESx  //SGP02 V4.2 4.1.4 ESx (Device - eUICC) Interface Description
#define JcvmConfig_GetM2MEmergencyProfileIndexBase()		    			U32_A_U32(VM_CONFIG_START, M2M_EMERGENCY_PROFILE_INDEX)
#define JcvmConfig_GetM2MEmergencyProfileIndex()							nvmReadU8(MEMtPN8(JcvmConfig_GetM2MEmergencyProfileIndexBase()))
#define JcvmConfig_SetM2MEmergencyProfileIndex(index)						nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetM2MEmergencyProfileIndexBase()),index)

#define JcvmConfig_GetM2MTestProfileIndexBase()		    					U32_A_U32(VM_CONFIG_START, M2M_TEST_PROFILE_INDEX)
#define JcvmConfig_GetM2MTestProfileIndex()									nvmReadU8(MEMtPN8(JcvmConfig_GetM2MTestProfileIndexBase()))
#define JcvmConfig_SetM2MTestProfileIndex(index)							nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetM2MTestProfileIndexBase()),index)
#endif

#define JcvmConfig_GetEUICCCurrentExecptionListBase(profileindex)			PNtMEM(PN8_A_U16(U32_A_U32(VM_CONFIG_START, EUICC_CURRENT_EXCEPTION_LIST), U8_M_U16(profileindex, SIZE_MEMREF)))
//#define JcvmConfig_GetEUICCCurrentExecptionList(profileindex)				nvmReadU32(JcvmConfig_GetEUICCCurrentExecptionListBase(profileindex))
//#define JcvmConfig_SetEUICCCurrentExecptionList(profileindex, value)		nvmWriteU32Atomic(MEMtPN8(JcvmConfig_GetEUICCCurrentExecptionListBase(profileindex)),value)

#define JcvmConfig_GetEUICCDtpBase(profileindex)							PNtMEM(PN8_A_U16(U32_A_U32(VM_CONFIG_START, EUICC_dTP), U8_M_U16(profileindex, (SIZE_U16 * NUM_GROUPS))))
#define JcvmConfig_GetEUICCDtpBackBase()									U32_A_U32(VM_CONFIG_START, EUICC_dTP_back)

#define JcvmConfig_GetEUICCMaxDtpBase(profileindex)							PNtMEM(PN8_A_U16(U32_A_U32(VM_CONFIG_START, EUICC_max_dTP), U8_M_U16(profileindex, SIZE_U16)))
//#define JcvmConfig_GetEUICCMaxDtp(profileindex)								nvmReadU16(MEMtPN8(JcvmConfig_GetEUICCMaxDtpBase(profileindex)))
//#define JcvmConfig_SetEUICCMaxDtp(profileindex,value)						nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetEUICCMaxDtpBase(profileindex)),value)

#define JcvmConfig_GetEUICCMaxDtpBackBase()									U32_A_U32(VM_CONFIG_START, EUICC_max_dTP_back)
#define JcvmConfig_GetEUICCMaxDtpBack()										nvmReadU16(MEMtPN8(JcvmConfig_GetEUICCMaxDtpBackBase()))
#define JcvmConfig_SetEUICCMaxDtpBack(value)								nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetEUICCMaxDtpBackBase()),value)

#define JcvmConfig_GetEUICCRtpBase(profileindex)							PNtMEM(PN8_A_U16(U32_A_U32(VM_CONFIG_START, EUICC_rTP), U8_M_U16(profileindex, SIZE_U16)))
//#define JcvmConfig_GetEUICCRtp(profileindex)								nvmReadU16(MEMtPN8(JcvmConfig_GetEUICCRtpBase(profileindex)))
//#define JcvmConfig_SetEUICCRtp(profileindex,value)							nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetEUICCRtpBase(profileindex)),value)

//#define JcvmConfig_GetEUICCRtpBackBase()									U32_A_U32(VM_CONFIG_START, EUICC_rTP_back)
//#define JcvmConfig_GetEUICCRtpBack()										nvmReadU16(MEMtPN8(JcvmConfig_GetEUICCRtpBackBase()))
//#define JcvmConfig_SetEUICCRtpBack(value)									nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetEUICCRtpBackBase()),value)

#define JcvmConfig_GetEUICCRTP_ProfileInitialBackBase()						U32_A_U32(VM_CONFIG_START, EUICC_rTP_ProfileInitial_back)
#define JcvmConfig_GetEUICCRTP_ProfileInitialBack()							nvmReadU16(MEMtPN8(JcvmConfig_GetEUICCRTP_ProfileInitialBackBase()))
#define JcvmConfig_SetEUICCRTP_ProfileInitialBack(value)					nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetEUICCRTP_ProfileInitialBackBase()),value)


#define JcvmConfig_GetCardOptionBase()					                	U32_A_U32(VM_CONFIG_START, CARD_OPTION)
#define JcvmConfig_GetCardOption()						    				nvmReadU8(MEMtPN8(JcvmConfig_GetCardOptionBase()))
#define JcvmConfig_SetCardOption(value)				       					nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetCardOptionBase()),value)
//#define UICC__FUNCTION__BERTLV                            U8_AND_U8(JcvmConfig_GetCardOption(), 0x02)
#define M2M_SEND_NOTIFICATION_AFTER_AUTH_SUCCESS	U8_AND_U8(JcvmConfig_GetCardOption(), 0x02)  //香港联调，配置鉴权成功是否发送通知
#define M2M_SMSC_FROM_MTSMS											U8_AND_U8(JcvmConfig_GetCardOption(), 0x04)  //MTS招标测试特殊需求：M2M版本发送MOSMS的短信中心取自下行
#define TaiwuZhilian_Support 												U8_AND_U8(JcvmConfig_GetCardOption(), 0x08)  //是否支持泰物智联需求
#define SUPPORT_NOTIFICATION_FIRST_TIME										U8_AND_U8(JcvmConfig_GetCardOption(), 0x10) // 配置是否发送开机通知

#define CARD_SE_BE_OR_NOT                                                   U8_AND_U8(JcvmConfig_GetCardOption(), 0x01)  //CARD_OPTION bit0
#define DEMAND_AUTHENTICATE_OP_READY                                        U8_AND_U8(JcvmConfig_GetCardOption(), 0x02)  //CARD_OPTION bit1 表示卡片处于CARD_OP_READY状态时，执行put key/store data 指令也需要鉴权
#define SELECTED_KEY_APPID                                                  U8_AND_U8(JcvmConfig_GetCardOption(), 0x04)  //CARD_OPTION bit2 表示SD2使用SD1的key在通道0建立安全通道后，则SD1不能在其他通道建立安全通道
#define AUTH_V1																U8_AND_U8(JcvmConfig_GetCardOption(), 0x08)
#define AUTH_V2																U8_AND_U8(JcvmConfig_GetCardOption(), 0x20)
#define AUTH_V3																U8_AND_U8(JcvmConfig_GetCardOption(), 0x40)
#define SUPPORT_V1V2V3														(AUTH_V1 || AUTH_V2 || AUTH_V3)
#define CARD_TESTMODE_OR_NOT                                                U8_AND_U8(JcvmConfig_GetCardOption(), 0x80)  //CARD_OPTION bit7 该比特主要用来控制ISD是否允许执行私有指令（目前控制一卡一CRC指令，外场测试不能执行）


#define Requirement_Default_SCP_NO                                          0x00 // if '81' is not existing in install parameter, then cos does not add default scp
#define Requirement_Default_SCP_ALL                                         0x01
#define Requirement_Default_SCP_SE                                          0x02
#define Requirement_Default_SCP_PROFILE                                     0x03

#define JcvmConfig_GetRequirement_Default_SCP_ConfigureBase()		       	U32_A_U32(VM_CONFIG_START, REQUIREMENT_DEFAULT_SCP)
#define JcvmConfig_GetGetRequirement_Default_SCP_Configure()		    	nvmReadU8(MEMtPN8(JcvmConfig_GetRequirement_Default_SCP_ConfigureBase()))
#define JcvmConfig_SetGetRequirement_Default_SCP_Configure(value)      		nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetRequirement_Default_SCP_ConfigureBase()),value)

#define sysConfig_GetMaptable_numBase() 						            U32_A_U32(COS_CONFIG_START,MAPTABLE_PAGENUM)
#define sysConfig_GetMaptable_num()                            	     	 	PN_R_U8(MEMtPN32(sysConfig_GetMaptable_numBase()))
#define sysConfig_SetMaptable_num(value)                       	     		nvmWriteU8Atomic(MEMtPN8(sysConfig_GetMaptable_numBase()),value)

#define	OS_CONF0_VGPS				  	  							U8_AND_U8(JcvmConfig_GetOsConf(), 0x01) //VM_OS_CONF bit0  配置为VGP，该配置在金融平台有用，在EPX平台未使用
#define	OS_CONF1_SCP_SWITCH			  	  							U8_AND_U8(JcvmConfig_GetOsConf(), 0x02) //VM_OS_CONF bit1
#define	OS_CONF2_ISSUED_MODE		  	  							U8_AND_U8(JcvmConfig_GetOsConf(), 0x04) //VM_OS_CONF bit2
#define	OS_CONF3_TCK_ICC_MODE										U8_AND_U8(JcvmConfig_GetOsConf(), 0x08) //VM_OS_CONF bit3 该值与CONFIG_GP_FOR_MAPPING_TEST不能同时置为1
#define	OS_CONF3_JC2_JC3_MODE										U8_AND_U8(JcvmConfig_GetOsConf(), 0x10) //VM_OS_CONF bit4 this is used for our OS is JavaCard2 or JavaCard3
#define	OS_CONF7_TCK_BCTC_MODE										U8_AND_U8(JcvmConfig_GetOsConf(), 0x80) //VM_OS_CONF bit7 this is used for bctc


//#define OS_CGM_CLOSE   			  	  			                    U8_AND_U8(JcvmConfig_GetVmConfig(), 0x01)		// JcvmConfig_GetVmConfig()
#define OS_CGM_CLOSE   			  	  			                    U8_AND_U8(JcvmConfig_GetVmConfig(), 0x02)//VM_CONFIG bit1:
#define	OS_C7C8_NOCHECK								                U8_AND_U8(JcvmConfig_GetVmConfig(), 0x08)	//测tck,pboc时，nocheck,0x1f
#define	OS_C4_NOCHECK								                U8_AND_U8(JcvmConfig_GetVmConfig(), 0x10)
#define OS_NOATOMIC_BACK_UP                                         U8_AND_U8(JcvmConfig_GetVmConfig(), 0x20)  //非原子是否备份。0 不备份，1 备份。
#define	OS_SELECT_BY_FIleID							                U8_AND_U8(JcvmConfig_GetVmConfig(), 0x80)

#define	GP_CONF0_INSTALL_FOR_LOAD_NO_HASH							U8_AND_U8(JcvmConfig_GetGpConf(), 0x01)  //this is used for in the installforload ,in this command if the SDAID is the CardMananger but not have Block HASH, and the installed a(more) MDAP SD.
#define	GP_CONF1_INSTALL_FOR_OLD_ICC								U8_AND_U8(JcvmConfig_GetGpConf(), 0x02)  //this is used for old ICC test
#define	GP_CONF2_ISD_SUPPORT_9F66_TAG								U8_AND_U8(JcvmConfig_GetGpConf(), 0x04)  //this is used for isd is support 9F66 tag
#define	GP_CONF3_PRO_SEC_RECEIVE									U8_AND_U8(JcvmConfig_GetGpConf(), 0x08)  //this is used for process security receive data
#define GP_CONF4_INSTALL_FOR_EID									U8_AND_U8(JcvmConfig_GetGpConf(), 0x10)  //
#define GP_CONF5_REGISTER											U8_AND_U8(JcvmConfig_GetGpConf(), 0x20)  //
#define GP_CONF6_SSD_USE_ISD_DIVDATA								U8_AND_U8(JcvmConfig_GetGpConf(), 0x40)  //this is used for isd is support 9F66 tag


#define GP_SECOND_CONFIG0_CONSTRUCT_AUTH_CODE      					U8_AND_U8(JcvmConfig_GetGpSecondConf(), 0x01)//住建部认证码开关
#define GP_SECOND_CONFIG1_SSD_PSEUDO_RANDOM         				U8_AND_U8(JcvmConfig_GetGpSecondConf(), 0x02)//SSD伪随机数生成KEY
#define GP_SECOND_CONFIG2_SELECTAPP_FRAME_LC_DISMATCH_DATALEN       U8_AND_U8(JcvmConfig_GetGpSecondConf(), 0x04)//非接选择ISD时LC与数据长度不匹配时返回错误状态字
#define GP_SECOND_CONFIG4_00BF_TAG_ENABLE      						U8_AND_U8(JcvmConfig_GetGpSecondConf(), 0x08)//对00BF TAG预制开关
#define GP_SECOND_CONFIG10_AID_FULLY_MATCH_ENABLE  					U8_AND_U8(JcvmConfig_GetGpSecondConf(), 0x10)//Select App完全匹配AID使能开关，默认关闭
#define GP_SECOND_CONFIG20_SELECT_FILE_BYID_ENABLE  				U8_AND_U8(JcvmConfig_GetGpSecondConf(), 0x20)//通过应用ID选择应用使能开关，默认关闭
#define GP_SECOND_CONFIG40_ABC_RUWEI  								U8_AND_U8(JcvmConfig_GetGpSecondConf(), 0x40)//农行入围测试脚本
#define GP_SECOND_CONFIG80_GET_PLATFORM_INFORMATION					U8_AND_U8(JcvmConfig_GetGpSecondConf(), 0x80)//获取平台相关版本信息

#define CONFIG_GP_FOR_UICC_Configuration               (U8_AND_U8(JcvmConfig_GetGpThirdConf() ,0x01)) //GP_THIRD_CONFIG bit0: this is used for GP UICC Test. Specification name: GlobalPlatform Card UICC Configuration
#define CONFIG_GP_FOR_MAPPING_TEST                     (U8_AND_U8(JcvmConfig_GetGpThirdConf() ,0x02)) //GP_THIRD_CONFIG bit1: this is used for GP Mapping Guidelines Test. Specification name: GlobalPlatform Card Mapping Guidelines of Existing GP v2.1.1 Implementation on v2.2.1
#define CONFIG_GP_FOR_BFC	     					   (U8_AND_U8(JcvmConfig_GetGpThirdConf(), 0x04)) //GP_THIRD_CONFIG bit2: this is used for BFC test. Specification name: GlobalPlatform Card Basic Financial Configuration
#define CONFIG_GP_FOR_CI	     					   (U8_AND_U8(JcvmConfig_GetGpThirdConf(), 0x08)) //GP_THIRD_CONFIG bit3: this is used for CI test. Specification name: GlobalPlatform Card Common Implementation Configuration

//#define	   GET_DAT_SC	                   JcvmConfig_GetGetDataScSwitch()
//#define	   SCP							   JcvmConfig_GetCmScpType()
//#define	   OPTION						   JcvmConfig_GetScpOption()

/**修复BIT位含义问题： 与JAVA层checkIsVGPconfig的bit位含义需要保持一致**/
#define CONFIG_VGP                                  (U8_AND_U8(JcvmConfig_GetVgpConfig() ,0x01)) //VGP_CONFIG bit0: JAVA层使用该bit位。 但是还有个配置OS_CONF0_VGPS
#define CONFIG_SECURE_CHANNLE_SERVICE_GP221         (U8_AND_U8(JcvmConfig_GetVgpConfig() ,0x02)) //VGP_CONFIG bit1: GPSystem.getSecureChannel()的开关,1表示按照GP221，0表示非GP221
#define	CONFIG_CRYPTO_ALL_USE_RTR	                (U8_AND_U8(JcvmConfig_GetVgpConfig() ,0x04)) //VGP_CONFIG bit2: JAVA 算法 API中是否使用RTR
#define CONFIG_GP221                                (U8_AND_U8(JcvmConfig_GetVgpConfig() ,0x10)) //VGP_CONFIG bit4:
#define CONFIG_CLENTRY_ALL_CREATE                   (U8_AND_U8(JcvmConfig_GetVgpConfig() ,0x20)) //VGP_CONFIG bit5:
#define OS_CONF4_C9_45_SUPPORT                      (U8_AND_U8(JcvmConfig_GetVgpConfig() ,0x40)) //VGP_CONFIG bit6: this is used for if it is intended that the Security Domain be capable of accepting extradition requests, this field contains 'C90145'



#endif


