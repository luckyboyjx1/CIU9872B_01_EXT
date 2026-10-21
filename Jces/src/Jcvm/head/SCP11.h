/*
 * SCP11.h
 *
 *  Created on: 2021-7-9, refer to SCP10
 *      Author: xuxiaojun
 */

#ifndef SCP11_H_
#define SCP11_H_
#include "profile.h"
#include "bc_object.h"
#include "EllipseCurve_API.h"

#if GP__SCP__SCP11__FOR_MASK

//extern u8    X_A    tempBuf[512]; deleted by xuxiaojun@20220927: RAM优化

#define HOST_ID_MAX_LEN                            0x10 //Hoitld  的长度问题，没找到规范定义其长度
#define GROUP_ID_MAX_LEN                           0x10 //Card Group  ID - LV is the  length and  the  value field  of tag  '5F20 '  (subject identifier) in CERT.SD.ECKA
#define SIN_MAX_LEN                                0x10 //SIN - LV is the length and the value field of the Security Domain Provider Identification Number  of the SD
#define SDIN_MAX_LEN                               0x10 //SDIN - LV is the length and the value field of the Security Domain Image Number  of the SD (see   [GPCS] ) .

#define PERFORMSECURITYOPERATION_STATE_START       0x00
#define PERFORMSECURITYOPERATION_STATE_CONTINUE    0x01
#define PERFORMSECURITYOPERATION_STATE_END         0x02

/*--------------------------------------PERFORM SECURITY OPERATION指令跨指令时需要保存的相关信息-----------------------------------------*/
#define g_current_PerformSecurityOperation         g_current_SCP_cardChallenge //modify by xuxiaojun@20220927:g_current_SCP_cardChallenge是区分了逻辑通道信息的，可以复用

#define g_current_PerformSecurityOperation_state               PN8_A_U8(g_current_PerformSecurityOperation, 0x00)//表示当前执行的PERFORM SECURITY OPERATION的状态，分别为接收第一条，接收中间条，接收完全
#define g_current_PerformSecurityOperation_keyVersion          PN8_A_U8(g_current_PerformSecurityOperation, 0x01)//表示当前执行的PERFORM SECURITY OPERATION的第一条数据所指示的key version的值
#define g_current_PerformSecurityOperation_keyId               PN8_A_U8(g_current_PerformSecurityOperation, 0x02)//表示当前执行的PERFORM SECURITY OPERATION的第一条数据所指示的key id的值
#define g_current_PerformSecurityOperation_curCertCount        PN8_A_U8(g_current_PerformSecurityOperation, 0x03)//表示当前收到的指令是当前证书的第几条, 初始值为0，当收到证书最后一条时该值也清为0，表示下一个证书的开始
#define g_current_PerformSecurityOperation_certNum             PN8_A_U8(g_current_PerformSecurityOperation, 0x04)//表示当前收到的是第几个证书
#define PerformSecurityOperation_LENGTH           0x05

#define SCP11_A							0x01
#define SCP11_B							0x02
#define SCP11_C							0x03


#define SCP11_PK_MAX_LV_LEN					(2*MAX_ECC_KEY_BYTELEN + 8) //第一个L长度固定1字节，后续是7F49的内容

/*----------------------------------------存储某个scp11会话的信息-----------------------------------------------*/
#define Authorizations_under_SCP11c_MAX_LEN     0x50 ////coe证书的的BF20指令访问规则:	'BF20'  Var  Authorizations under SCP11c支持的最大能力

#define SCP11SessionPara_LENGTH           (0x17 + 2*MAX_ECC_KEY_BYTELEN + 2)

extern u8 G_SCP11SessionPara[SCP11SessionPara_LENGTH * MAX_LOGICAL_CHANNELS];   //存储某个scp11会话的信息
#define g_current_SCP11SessionPara  PN8_A_U8(G_SCP11SessionPara, SCP11SessionPara_LENGTH * selected_channel)

#define g_current_SCP11SessionPara_sessionType                 g_current_SCP11SessionPara                        //区别scp11a/b/c
#define g_current_SCP11SessionPara_ocePkRef                    PN8_A_CONST(g_current_SCP11SessionPara, 0x01)     //ocePK 的索引，目前仅支持单字节的索引
#define g_current_SCP11SessionPara_oceSubjectIdLv              PN8_A_CONST(g_current_SCP11SessionPara, 0x02)     //coe证书的的subId: '5F20'   Subject Identifier 的LV
#define g_current_SCP11SessionPara_oceAuthorizationsLv         PN8_A_CONST(g_current_SCP11SessionPara, 0x13)     //coe证书的的BF20指令访问规则:	'BF20'  Var  Authorizations under SCP11c  的LV
//publicKey(LV)		ALG_EC_FP_KEY_521 是 02  时的公钥长度为0x85, 其中V是包含了非压缩格式的04再内，再增加一字节的L, modify by xuxiaojun@20220528  测试用例： SCP11c_1：  C_sm_STORE_DATA_Whitelist_SUCCESS_KID20h_521_a1_63_10
#define g_current_SCP11SessionPara_ocePKValueLV                PN8_A_CONST(g_current_SCP11SessionPara, 0x17) //((memref)&scpData) 不能复用scpData，因为其内部的scpSESKey等秘钥会保存有效数据

/*---------------------------------------------PERFORM SECURITY OPERATION指令中缓存证书的格式----------------------------------------------------------------*/
//证书的最大长度：BF20的长度 + 7F49公钥长度 + 5F37签名长度 + 其他可计算最大长度
#define MAX_CERTIFICATE_LEN   (Authorizations_under_SCP11c_MAX_LEN + MAX_ECC_KEY_BYTELEN + MAX_ECC_KEY_BYTELEN + 203)
#define TmpCert_LENGTH        (2 + MAX_CERTIFICATE_LEN)

#if GP__SCP__SCP11__CERT_IN_GLOBAL_RAM
//typedef  struct currentCert
//{
//	u16 offset;                      //表示当前缓存证书的偏移
//	u8  cert[MAX_CERTIFICATE_LEN];   //表示当前缓存的证书数据, 7F21 L V
//}TmpCert;	                     //PERFORM SECURITY OPERATION指令中

extern u8 G_TmpCertInRam[TmpCert_LENGTH * MAX_LOGICAL_CHANNELS];
#define g_current_SCP11_TmpCert  PN8_A_U8(G_TmpCertInRam, TmpCert_LENGTH*selected_channel)
#endif

extern PX8 G_TmpCert;
#define g_current_TmpCert_offset                PX8_A_CONST(G_TmpCert, 0x00)   //表示当前缓存证书的偏移
#define g_current_TmpCert_cert                  PX8_A_CONST(G_TmpCert, 0x02)   //表示当前缓存的证书数据, 7F21 L V

/***********************cert**********************************/
#define GPCERT_Tag							0x21	//0x7f21
#define GPCERT_CertSerNum_Tag				0x93
#define GPCERT_CAId_Tag						0x42
#define GPCERT_SubjectId_Tag					0x20	//0x5F20
#define GPCERT_KeyUsage_Tag					0x95
#define GPCERT_EffectiveDate_Tag				0x25	//0x5F25
#define GPCERT_ExpirationDate_Tag  				0x24	//0x5F24
#define GPCERT_CASDImageNum_Tag				0x45
#define GPCERT_DiscretionaryData53_Tag 		0x53
#define GPCERT_DiscretionaryData73_Tag			0x73
#define GPCERT_Authorizations_Tag				0x22	//0xBF20	 //定义为0x22，是为了和GPCERT_SubjectId_Tag区分开
#define GPCERT_PublicKey_Tag					0x49	//0x7f49
#define GPCERT_Signature_Tag					0x37	//0x5F37
#define GPCERT_PublicKey_VALUE_Tag			0xB0
#define GPCERT_PublicKey_REFERENCE_Tag		0xF0
#define GPCERTFICATE_KEY_USAGE_VERIFICATION 0x82
#define GPCERTFICATE_KEY_USAGE_KEYAGREEMENT 0x0080

#define MORE_CERTIFICATE     0x80
#define LAST_CERTIFICATE     0x00

#define NOT_CHECK_KEYUSAGE	1
#define CHECK_KEYUSAGE 		2

#define MORE_COMMAND         0x80
#define LAST_COMMAND         0x00

/*********************** SCP parameters **********************************/
#define SCP11_PARAMETER_VARIANT_MASK 0x03
#define SCP11_PARAMETER_VARIANT_B    0x00
#define SCP11_PARAMETER_VARIANT_A    0x01
#define SCP11_PARAMETER_VARIANT_RFU  0x02
#define SCP11_PARAMETER_VARIANT_C    0x03

#define SCP11_PARAMETER_HOSTID_MASK           0x04
#define SCP11_PARAMETER_HOSTID_INCLUDE        0x04
#define SCP11_PARAMETER_HOSTID_NOT_INCLUDE    0x00

/***************************scp11 key id*******************************/
#define KEY_ID_FOR_PK_CA_KLOC_ECDSA               0x10 // KID '10' for PK.CA-KLOC.ECDSA
#define KEY_ID_FOR_SK_SD_ECKA_FOR_SCP11a          0x11
#define KEY_ID_FOR_STATIC_DEK_FOR_SCP11a          0x12
#define KEY_ID_FOR_SK_SD_ECKA_FOR_SCP11b          0x13
#define KEY_ID_FOR_STATIC_DEK_FOR_SCP11b          0x14
#define KEY_ID_FOR_SK_SD_ECKA_FOR_SCP11c          0x15
#define KEY_ID_FOR_STATIC_DEK_FOR_SCP11c          0x16

/***************************scp11 option “i” *******************************/
#define SCP11_OPTION_MASK                        0x7F
#define SCP11A_SUPPORTED                         0x01
#define SCP11B_SUPPORTED                         0x02
#define PERSISTENTLY_STORES_PK_OCE               0x04
#define CERT_CHAIN_SUPPORTED                     0x08
#define SCP11C_SUPPORTED                         0x10
#define BF20_SUPPORTED                           0x20
#define SECURE_MESSAGE_S16_MODE                  0x40

/***************************scp11*******************************/
#define SCP11_PK_CA_KLOC_ECDSA_PCK_ID				   EASTCOMPEACE_OPENPLATFORM_ID
#define newSCP11_PK_CA_KLOC_ECDSA()					   newObject(SPC11_PK_CA_KLOC_ECDSA_OFFSET,SCP11_PK_CA_KLOC_ECDSA_PCK_ID, BIT_NEED_REFRESH)

#define SCP11_SK_SD_ECKA_PCK_ID				           EASTCOMPEACE_OPENPLATFORM_ID
#define newSCP11_SK_SD_ECKA()					       newObject(SPC11_SK_SD_ECKA_OFFSET,SCP11_SK_SD_ECKA_PCK_ID, BIT_NEED_REFRESH)

#define SCP11_CERT_OCE_ECKA_PCK_ID				       EASTCOMPEACE_OPENPLATFORM_ID
#define newSCP11_CERT_OCE_ECKA()	    		       newObject(SPC11_CERT_OCE_ECKA_OFFSET,SCP11_CERT_OCE_ECKA_PCK_ID, BIT_NEED_REFRESH)

#define SCP11_PK_CA_KLOC_ECDSA_getKeyVersion(pkCAObj)               memReadU8(get_object_field_ref(pkCAObj, PK_CA_keyVersion_Index))
#define SCP11_PK_CA_KLOC_ECDSA_setKeyVersion(pkCAObj, ver)          memWriteU8Atomic(get_object_field_ref(pkCAObj, PK_CA_keyVersion_Index), ver)

#define SCP11_PK_CA_KLOC_ECDSA_getKeyId(pkCAObj)                    memReadU8(get_object_field_ref(pkCAObj, PK_CA_keyId_Index))
#define SCP11_PK_CA_KLOC_ECDSA_setKeyId(pkCAObj, id)                memWriteU8Atomic(get_object_field_ref(pkCAObj, PK_CA_keyId_Index), id)

#define SCP11_PK_CA_KLOC_ECDSA_getInternalCounter(pkCAObj)          memReadU16(get_object_field_ref(pkCAObj, PK_CA_internalCounter_Index))
#define SCP11_PK_CA_KLOC_ECDSA_setInternalCounter(pkCAObj, coun)    memWriteU16Atomic(get_object_field_ref(pkCAObj, PK_CA_internalCounter_Index), coun)

#define SCP11_PK_CA_KLOC_ECDSA_getWhitelistField(pkCAObj)           get_object_field_ref(pkCAObj, PK_CA_Whitelist_Index)
#define SCP11_PK_CA_KLOC_ECDSA_getWhitelistRef(pkCAObj)             memReadU16(SCP11_PK_CA_KLOC_ECDSA_getWhitelistField(pkCAObj))
#define SCP11_PK_CA_KLOC_ECDSA_getWhitelistDataObj(pkCAObj)         locateObjectInMemory(SCP11_PK_CA_KLOC_ECDSA_getWhitelistRef(pkCAObj))
#define SCP11_PK_CA_KLOC_ECDSA_setWhitelistRef(pkCAObj, ref)        memWriteU16Atomic(SCP11_PK_CA_KLOC_ECDSA_getWhitelistField(pkCAObj), ref)

#define SCP11_PK_CA_KLOC_ECDSA_getCAKLOCIdentifierField(pkCAObj)    get_object_field_ref(pkCAObj, PK_CA_CA_KLOC_Identifier_Index)
#define SCP11_PK_CA_KLOC_ECDSA_getCAKLOCIdentifierRef(pkCAObj)      memReadU16(SCP11_PK_CA_KLOC_ECDSA_getCAKLOCIdentifierField(pkCAObj))
#define SCP11_PK_CA_KLOC_ECDSA_getCAKLOCIdentifierDataObj(pkCAObj)  locateObjectInMemory(SCP11_PK_CA_KLOC_ECDSA_getCAKLOCIdentifierRef(pkCAObj))
#define SCP11_PK_CA_KLOC_ECDSA_setCAKLOCIdentifierRef(pkCAObj, ref) memWriteU16Atomic(SCP11_PK_CA_KLOC_ECDSA_getCAKLOCIdentifierField(pkCAObj), ref)

#define SCP11_PK_CA_KLOC_ECDSA_getNextPKCAField(pkCAObj)            get_object_field_ref(pkCAObj, PK_CA_nextPK_CA_KLOC_ECDSA_Index)
#define SCP11_PK_CA_KLOC_ECDSA_getNextPKCARef(pkCAObj)              memReadU16(SCP11_PK_CA_KLOC_ECDSA_getNextPKCAField(pkCAObj))
#define SCP11_PK_CA_KLOC_ECDSA_setNextPKCARef(pkCAObj, ref)         memWriteU16Atomic(SCP11_PK_CA_KLOC_ECDSA_getNextPKCAField(pkCAObj), ref)

#define SCP11_CERT_OCE_ECKA_getPkValueOfOCEField(certObj)           get_object_field_ref(certObj, PK_OCE_pkValueOfOCE_Index)
#define SCP11_CERT_OCE_ECKA_getPkValueOfOCERef(certObj)             memReadU16(SCP11_CERT_OCE_ECKA_getPkValueOfOCEField(certObj))
#define SCP11_CERT_OCE_ECKA_getPkValueOfOCEDataObj(certObj)         locateObjectInMemory(SCP11_CERT_OCE_ECKA_getPkValueOfOCERef(certObj))
#define SCP11_CERT_OCE_ECKA_setPkValueOfOCERef(certObj, ref)        memWriteU16Atomic(SCP11_CERT_OCE_ECKA_getPkValueOfOCEField(certObj), ref)

#define SCP11_CERT_OCE_ECKA_getTag_5F20Field(certObj)               get_object_field_ref(certObj, PK_OCE_tag_5F20_Index)
#define SCP11_CERT_OCE_ECKA_getTag_5F20Ref(certObj)                 memReadU16(SCP11_CERT_OCE_ECKA_getTag_5F20Field(certObj))
#define SCP11_CERT_OCE_ECKA_getTag_5F20DataObj(certObj)             locateObjectInMemory(SCP11_CERT_OCE_ECKA_getTag_5F20Ref(certObj))
#define SCP11_CERT_OCE_ECKA_setTag_5F20Ref(certObj, ref)            memWriteU16Atomic(SCP11_CERT_OCE_ECKA_getTag_5F20Field(certObj), ref)

#define SCP11_SK_SD_ECKA_getKeyVersion(skSDObj)                     memReadU8(get_object_field_ref(skSDObj, SK_ECKA_keyVersion_INDEX))
#define SCP11_SK_SD_ECKA_setKeyVersion(skSDObj, ver)              memWriteU8Atomic(get_object_field_ref(skSDObj, SK_ECKA_keyVersion_INDEX), ver)

#define SCP11_SK_SD_ECKA_getKeyId(skSDObj)                          memReadU8(get_object_field_ref(skSDObj, SK_ECKA_keyId_INDEX))
#define SCP11_SK_SD_ECKA_setKeyId(skSDObj, kid)                           memWriteU8Atomic(get_object_field_ref(skSDObj, SK_ECKA_keyId_INDEX), kid)

#define SCP11_SK_SD_ECKA_getECKACertificateField(skSDObj)           get_object_field_ref(skSDObj, SK_ECKA_ECKACertificate_INDEX)
#define SCP11_SK_SD_ECKA_getECKACertificateRef(skSDObj)             memReadU16(SCP11_SK_SD_ECKA_getECKACertificateField(skSDObj))
#define SCP11_SK_SD_ECKA_getECKACertificateDataObj(skSDObj)         locateObjectInMemory(SCP11_SK_SD_ECKA_getECKACertificateRef(skSDObj))
#define SCP11_SK_SD_ECKA_setECKACertificateRef(skSDObj, ref)        nvmWriteU16Atomic(SCP11_SK_SD_ECKA_getECKACertificateField(skSDObj), ref)

#define SCP11_SK_SD_ECKA_getNextSKField(skSDObj)                    get_object_field_ref(skSDObj, nextSK_SD_ECKA_INDEX)
#define SCP11_SK_SD_ECKA_getNextSKRef(skSDObj)                      memReadU16(SCP11_SK_SD_ECKA_getNextSKField(skSDObj))
#define SCP11_SK_SD_ECKA_setNextSKRef(skSDObj, ref)                 nvmWriteU16Atomic(SCP11_SK_SD_ECKA_getNextSKField(skSDObj), ref)

#define sdData_getFirstSCP11_PK_CA_KLOC_ECDSAField(sdDataObj)       get_object_field_ref(sdDataObj,firstSCP11_PK_CA_KLOC_ECDSA_INDEX)
#define sdData_getFirstSCP11_PK_CA_KLOC_ECDSARef(sdDataObj)		    nvmReadU16(sdData_getFirstSCP11_PK_CA_KLOC_ECDSAField(sdDataObj)) // modify by xuxiaojun@20220625 需要考虑cache
#define	sdData_setFirstSCP11_PK_CA_KLOC_ECDSARef(sdDataObj, ref)    nvmWriteU16Atomic(sdData_getFirstSCP11_PK_CA_KLOC_ECDSAField(sdDataObj), ref)

#define sdData_getFirstSCP11_SK_SD_ECKAField(sdDataObj)             get_object_field_ref(sdDataObj,firstSCP11_SK_SD_ECKA_INDEX)
#define sdData_getFirstSCP11_SK_SD_ECKARef(sdDataObj)		        nvmReadU16(sdData_getFirstSCP11_SK_SD_ECKAField(sdDataObj)) // modify by xuxiaojun@20220625 需要考虑cache
#define	sdData_setFirstSCP11_SK_SD_ECKARef(sdDataObj, ref)          nvmWriteU16Atomic(sdData_getFirstSCP11_SK_SD_ECKAField(sdDataObj), ref)

#define sdData_getSCP11_certField(sdDataObj)                get_object_field_ref(sdDataObj, cert_INDEX)
#define sdData_getSCP11_certRef(sdDataObj)                  memReadU16(sdData_getSCP11_certField(sdDataObj))
#define sdData_setSCP11_certRef(sdDataObj, ref)             memWriteU16Atomic(sdData_getSCP11_certField(sdDataObj), ref)

#define sdData_getSCP11_PerformSecurityOperationField(sdDataObj)             get_object_field_ref(sdDataObj, SCP11_PerformSecurityOperation_Index)
#define sdData_getSCP11_PerformSecurityOperation(sdDataObj)                  memReadU16(sdData_getSCP11_PerformSecurityOperationField(sdDataObj))
#define sdData_setSCP11_PerformSecurityOperation(sdDataObj, ref)             memWriteU16Atomic(sdData_getSCP11_PerformSecurityOperationField(sdDataObj), ref)

#define sdData_getSCP11_SessionParaField(sdDataObj)             get_object_field_ref(sdDataObj, SCP11_SessionPara_Index)
#define sdData_getSCP11_SessionPara(sdDataObj)                  memReadU16(sdData_getSCP11_SessionParaField(sdDataObj))
#define sdData_setSCP11_SessionPara(sdDataObj, ref)             memWriteU16Atomic(sdData_getSCP11_SessionParaField(sdDataObj), ref)

#if (!GP__SCP__SCP11__CERT_IN_GLOBAL_RAM)
#define sdData_getSCP11_TmpCertField(sdDataObj)             get_object_field_ref(sdDataObj, SCP11_TmpCert_Index)
#define sdData_getSCP11_TmpCert(sdDataObj)                  memReadU16(sdData_getSCP11_TmpCertField(sdDataObj))
#define sdData_setSCP11_TmpCert(sdDataObj, ref)             memWriteU16Atomic(sdData_getSCP11_TmpCertField(sdDataObj), ref)
#endif

#define MODE_CREATE_SCP11_SESSIONPARA   0x00
#define MODE_GET_SCP11_SESSIONPARA      0x01
extern void sdData_getSCP11SessionPara(PN8 sdDataObj, u8 mode);

extern void SCP11PerformSecurityOperation_InitGlobalVar(void);
extern u8 SCP11AnalyzeA6_83(PX8 lvBuffer, PX8 keyVersion, PX8 keyId);
extern u8 SCP11AnalyzeA6_42(PX8 lvBuffer, PX8 Identifier);
extern u8 SCP11Analyze92(PX8 lvBuffer, PX16 counter);
extern u8 SCP11Analyze70(PX8 lvBuffer);
extern u16 SCP11StoreWhitelist(PN8 sdDataObj, PX8 whitelist, u8 keyVersion, u8 keyId);
extern void SCP11StoreIdentifier(PN8 sdDataObj, PX8 Identifier,u8 keyVersion, u8 keyId);
extern void SCP11InitKey(PN8 sdDataObj, u8 oldKeyVersion, u8 keyVersion, u8 keyId, u8 keyType, u8 status);
extern u8 SCP11GetKIDandKVNByCAID(PN8 sdDataObj, PX8 buffer, u16 bufSize);
extern jref SCP11GetPKCARefByKeyVerAndKeyId(PN8	sdDataObj, u8 keyVersion, u8 keyId);
extern u8 SCP11AnalyzeBF21(PX8 lvBuffer, PX8 certificate);
extern void SCP11StoreECKACert(PN8 sdDataObj, PX8 certificate, u8 keyVersion, u8 keyId, u16 currStoreLen);
extern u16 SCP11GetECKACert(PN8 sdDataObj, PX8 buf, u16 bufSize, jref *certRef, u16 certDataOffSet);
extern jref GetDekRefByKeyVerAndKeyId(PN8 sdDataObj, u8 keyVersion, u8 keyId);
extern jref GetSkValueRefByKeyVerAndKeyId(PN8 sdDataObj, u8 keyVersion, u8 keyId);
extern memref  GetSkValueAddrByKeyVerAndKeyId(PN8 sdDataObj, u8 keyVersion, u8 keyId);
extern u16  GetSkValueByKeyVerAndKeyId(PN8 sdDataObj, u8 keyVersion, u8 keyId, PX8 skValue);
extern jref GetECKACertRefByKeyVerAndKeyId(PN8	sdDataObj, u8 keyVersion, u8 keyId);
extern memref GetECKACertAddrByKeyVerAndKeyId(PN8 sdDataObj, u8 keyVersion, u8 keyId);
extern u8 GetECKACertSubValueByKeyVerAndKeyId(PN8 sdDataObj, u8 keyVersion, u8 keyId, u8 wantTag, PX8 bufLv, u8 bufSize);
extern memref GetSkObjByKeyVerAndKeyId(PN8 sdDataObj, u8 keyVersion, u8 keyId);
extern void SCP11SetSessionKey(PX8 keyBuf, u8 allKeyLen, u8 perKeylen,PX8 lastMacBuf);
extern u16 SCP11SetAuthenticatedLevel(PN8 sdDataObj, u8 keyUsage, u8 scp11ScpPara);
extern u8 SCP11PerformSecurityOperation_P1Valid(u8 p1, u16 certLen);
extern u16 SCP11PerformSecurityOperation_P2Valid(PN8 sdDataObj, u8 p2, PX8 keyUsage, u8 mode);
extern u16 SCP11PerformSecurityOperation(u8 APDUDataFieldIsReceived);
extern u8 SCP11MutualAuthenticate_A6Valid(PX8 a6Buf, PX8 ScpPara, PX8 keyUsage, PX8 keyType, PX8 keyLen, PX8 hostID);
extern u8 SCP11MutualAuthenticate_5F49Valid(PX8 dataBuf, PX8 ePKBuf);
extern u8 SCP11GenerateShSssOrShSes(PX8 pkBuf, PX8 skBuf, PX8 ShSxx);
extern u8 SCP11GenerateSharedInfo(PX8 sharedInfo, u8 keyUsage, u8 keyType, u8 keyLen, PX8 hostID, PX8 cardGroupIdLv);
extern u8 SCP11GenerateReceipt(PX8 inputDataForReceipt, PX8 receipt);
extern u16 SCP11MutualAuthenticate(u8 APDUDataFieldIsReceived);
extern u8 GPCertificate_GetField(PM8 certificateAddr,u16 certificateLen, u8 wantTag, PM8 dataBuf, u16 bufSize);
extern void SCP11AnalyzeStoreData(PN8 sdDataObj);
extern u16 SCP11PersistentlyStore5F20AndOcePk(PN8 sdDataObj,u8 scpOptions);

extern u16 GPCertificate_ValidOfCSN(PN8 pkcaObj, PM8 certBuf, u16 totleLen);
extern u16 GPCertificate_ValidOfSignature(PM8 certBuf, u16 totleLen, PX8 pKValue, u8 pKRef);
extern u16 check5F20AndOcePk(PN8 sdDataObj, u8 scpOptions,u8 scp11ScpPara);
extern u16 SCP11GetPersistently5F20AndOcePk(PN8 sdDataObj);
extern u16 GPCertificate_ValidOfPK(PM8 certBuf, u16 totleLen);
extern u16 GPCertificate_Valid(PN8 pkcaObj, PM8 certBuf, u16 totleLen, PX8 pKExtracted);
extern u8 BF20_GetField(u8 wantTag, PM8 dataBufLv, u16 bufSize, u8 tagOccur);
extern u8 checkSCP11cSTOREDATATagInBF20(u16 tag, u8 isDgi);
extern u8 SCP11_decryptDataValid(void);
extern void SCP11DeleteKey(PN8 sdDataObj, u8 keyVersion, u8 keyId);
#endif

#endif /* SCP11_H_ */
