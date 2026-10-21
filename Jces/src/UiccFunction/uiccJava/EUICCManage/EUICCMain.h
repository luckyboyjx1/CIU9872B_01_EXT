#ifndef    _EUICCMAIN_H_
#define	   _EUICCMAIN_H_
#include    "typedef.h"
#include   "SysConfigTarget.h"
#include   "profile.h"

/*******************************Communication Interface Access Define************************************************/
//#define Communication_Global_ON   0x80
//#define Communication_Global_OFF  0x00
/******************appType**********************************************/

//#define  StandaloneApplication_Type   0xFF
//#define  HeadApplication_Type         0xFE	   //代码约定： 当一个应用是成员应用时，则一定属于其HeadApplication TLV 所指定的组

/**********************AppletAID_List_TABLE***********************************/
//#define  List_LenInTable            0x12
//#define  AidLen_offset              0x00
//#define  AidValue_offset            0x01
//#define  appType_offset             0x11    //others:表示该app的head appid，0xFF：表示独立应用，0xfe表示head应用

//#define  CAPCOMPONENT_HeaderTag             1


/*************************groupList define****************************************************************/
//#define  MaxNum_InCRELAuthTable            0x0A
/*
*每个表有10条记录，每条记录有 18字节
格式如下
/——————————————————————————————----——————————————————-|
_AID length___|____AID Value___|______tag_____________|
*/

//#define  Member_LenInTable         0x12
//#define  Member_TableSize          (MaxNum_InCRELAuthTable*Member_LenInTable)


//#define  Member_AIDLen_Offset      0x00
//#define  Member_AIDValue_Offset    0x01
//#define  Member_TAG_offset         0x11    //others:表示是成员的appId，0xFF：表示不是成员

/**********************CREL List Table***********************************/
//#define CREL_LenInTable            Member_LenInTable		  //因为该函数IsInGroupCRELList ，同时比较了Auth List和crel  list,所以这里两个表的长度设置成一样的
//#define CREL_TableSize             Member_TableSize

//#define GarbageCollector_TRUE   0xA5 //垃圾回收标记
//#define GarbageCollector_FALSE  0xFF
/*******************SecureDomainKey Format****************************************************/
//#define  MaxNumber_KeyVersion           0x6F    //GP 2.2.1 :PutKey指令规范version的范围是 01- 7F;Mapping guidelines 6.9.1 :Verison范围是01-6F
//#define  KeyAllComplete_Tag             0x07    //当前版本的kid1 kid2 kid3已经全部更新成功

//#define  keyType_Format1                0x01
//#define  keyType_Format2                0x02

#define  keyType_DES                    0x80
//#define  keyType_AES                    0x88
//#define  keyType_TLS                    0x85

//#define  Offset_keyVersion        0x00
//#define  Offset_nextKeyAddrRef    0x01
//#define  Offset_keySize    		  0x03
//#define  Offset_keyType           0x04
//#define  Offset_keyCompleteTag    0x07
//#define  Offset_sequenceCounter	  0x08
//#define  Offset_keyValue          0x0A
//#define  Offset_keyUsage          0x3A
//#define  Offset_keyAccess         0x3D


/*************************************Applet Privileges*************************************************************/
//#define PRIVILEGE_FIRSTBYTE_SD                       0x80//privilege第一个字节的各bit位含义，参见GP11.1.2 wys
//#define PRIVILEGE_FIRSTBYTE_DAP                      0x40
//#define PRIVILEGE_FIRSTBYTE_DELEGATEDMANAGEMENT      0x20
//#define PRIVILEGE_FIRSTBYTE_CARDLOCK                 0x10
//#define PRIVILEGE_FIRSTBYTE_CARDTERMINATE            0x08
//#define PRIVILEGE_FIRSTBYTE_CARDRESET                0x04
//#define PRIVILEGE_FIRSTBYTE_CVM                      0x02
//#define PRIVILEGE_FIRSTBYTE_MANDATEDDAP              0x01
//#define PRIVILEGE_SECONDBYTE_TRUSTEDPATH             0x80
//#define PRIVILEGE_SECONDBYTE_AUTHORIZEDMANAGEMENT    0x40
//#define PRIVILEGE_SECONDBYTE_TOKENMANAGEMENT         0x20
//#define PRIVILEGE_SECONDBYTE_GLOBALDELETE            0x10
//#define PRIVILEGE_SECONDBYTE_GLOBALLOCK              0x08
//#define PRIVILEGE_SECONDBYTE_GLOBALREGISTRY          0x04
//#define PRIVILEGE_SECONDBYTE_FINALAPPLICATION        0x02
//#define PRIVILEGE_SECONDBYTE_GLOBALSERVICE           0x01

//#define PRIVILEGE_RECEIPT_GENERATION             	 0x80
//#define PRIVILEGE_CIPHERED_LOAD_FILE_DATA_BLOCK    	 0x40
//#define PRIVILEGE_CONTACTLESS_ACTIVATION           	 0x20
//#define PRIVILEGE_CONTACTLESS_SELF_ACTIVATION      	 0x10

//#define PRIVILEGE_SECONDBYTE_ILLEGALCHECK   0x73//privilege的第二个字节的bit7、6、5、2、1不能置1，该宏定义用于判断
//#define PRIVILEGE_SECONDBYTE_DEFAULT        0x80//在privilege只有一个字节的情况下，第二个字节使用该默认值(GP 6.6.2)
//#define PRIVILEGE_THIRDBYTE_DEFAULT         0x00//在privilege只有一个字节的情况下，第三个字节使用该默认值(GP 6.6.2)

/***************************GP Install指令************************/
//#define P1_INSTALLFORLOAD                  0x02
//#define P1_INSTALLFORINSTALL               0x04
//#define P1_INSTALLFORMAKESELECTABLE        0x08
#define P1_INSTALLFORINSTALLANDSELECTABLE  0x0C
#define P1_INSTALLFOREXTRADITION           0x10
#define P1_INSTALLFORPERSONALIZATION       0x20
//#define P1_INSTALLFORREGISTYUPDATE         0x40
//

#define  keyType_AES                    0x88
#define  keyType_SM4                    0x19 //源于《面向消费电子设备的SIM卡远程配置国家商用密码算法技术要求（征求意见稿）V2.pdf》中提到的国密接口规范

/********************Current Security Level*************************/
#define NO_SECURITYLEVEL                    0x00
#define SECURITYLEVEL_CMAC                  0x01
#define SECURITYLEVEL_CDECRYPTION           0x02
#define SECURITYLEVEL_RMAC_BEGINSESSION     0x04
#define SECURITYLEVEL_RMAC_BEGINSESSIONONLY 0x08
#define SECURITYLEVEL_RMAC                  0x10
#define SECURITYLEVEL_RENCRYPTION	        0x20
#define SECURITYLEVEL_AUTHENTICATED         0x80
#define SECURITYLEVEL_ANYAUTHENTICATED	    0x40

#define GPRam_Mode        0x00
#define GPFlash_Mode      0x01

//#define CONTACTLESS_PROTOCOL_PROFILE_NUMBER    0x06
//#define  List_LenInTable                       0x12

//#define CardMemory_Mode    0x01
//#define AppMemory_Mode     0x02

//#define  CAPCOMPONENT_InvalidTag            0xFF
//#define  CAPCOMPONENT_BeginTag              0xFE


#define  NOTIFICATION_MAX_NUMBER               0x0A  //卡片最大支持的notification的数目


//#define ISD_APPID          				 0x00											//去警告，防止重复定义，该宏已经在contentManager.h 中定义， 本文件仅定义euicc相关
#define MNOSD_APPID                      ISD_APPID
#define ISDR_APPID                       0x02       //ISD-R应用的 Appid
//#define UICC_APPID                       0x01//GetUICCAppId()//0x01       //USIM应用的APPID	//去警告，防止重复定义，该宏已经在contentManager.h 中定义， 本文件仅定义euicc相关
#define ECASD_APPID						 0x03       //ECASD的APPID
#define ISDP_APPID                       0x04//SE_APP_INSTALLED_MAX
//#define ILLEGAL_APPID                    0xFF												//去警告，防止重复定义，该宏已经在contentManager.h 中定义， 本文件仅定义euicc相关
//#define ILLEGAL_PKGID                    0xFF												//去警告，防止重复定义，该宏已经在contentManager.h 中定义， 本文件仅定义euicc相关


#define TempCIPKIdToBeUsed_Off    0x00 //该字段在profile安装成功后，需存储到profile配置信息中，供后续产生enable disable delete通知使用，LV 结构
#define TempeuiccSignature1_Off   0x40  //该字段在AuthServer时产生，在prepareDowanlod时会使用一次 67字节
#define TempPKDPpb_Off       	  0x90  //DPpb & DPDSAuth()的公钥值 ，包括曲线参数和PKValue ，结构为1字节曲线参数 + 64字节公钥 共65字节，DPpb在prepareDowanlod时提取保存，在InitialiseSecureChannel使用
#define TempCertDPAuthAuthID      0x90 //LV 结构（与TempPKDPpb_Off共用）  在AuthServer时 将CERT.DPauth.ECDSA的authorityKeyIdentifier存在该区域，PrepareDownLoad时候和CERT.DPpb.ECDSA的authorityKeyIdentifier比较是否属于一条CI链


#define TempSIMOID_Off            0xD1  //LV 结构  在AuthServer时产生，在prepareDowanlod比较，产生临时key以后再存放在EUICC_OpKeyBuf中



#define TempSMSRCertPublickey_Off   0x00  //设备版使用  该字段存储ISD-R/ISD-P建立会话密钥过程中 第一步下发的SM-SR证书中的公钥值 64字节
#define TempSMSRCertKeyParaRef_Off  0x40  //设备版使用  该字段存储ISD-R/ISD-P建立会话密钥过程中 第一步下发的SM-SR证书中的公钥参数类型 ，1字节
#define TempSMSRCertRC_Off          0x41  //设备版使用  存储ISD-R/ISD-P建立会话密钥过程中 第一步返回的随机数 LV结构 随机数16/32字节

#define EUICC_OpKeyPara_Off        1
#define EUICC_OpKeyPKLen_Off       2
#define EUICC_OpKeyPK_Off          3
#define EUICC_OpKeySKLen_Off       67
#define EUICC_OpKeySK_Off          68
#define EUICC_OpKeyOID_Off         100 //指向OID Len字段



#if (EUICC_ONLY||SE_AND_EUICC)

//EUICC配置区定义  约10K 
#define  EUICCBackPEBuf           (EUICC_CFG_START)   //profile动态解析备份不完整tlv数据区
#define  EUICCBackPEBufSize       (0x400)

#define	 EUICCTempDataBuf		  (EUICCBackPEBuf + EUICCBackPEBufSize)
#define  EUICCTempDataBufSize     (0x1000)  // RSP_EUICC测试_PPR规则测试\PPR_Rules_Test_Nominal_TSN06.html RetrieveNotificationsListRequest的数据很长
                                            //连续下载删除profile时，通知的个数可能会达到0x10个，那么retruveNotification时，需要发送的通知长度超过EUICCTempDataBufSize则会无法发送
#define EUICC_AuthenTmpBuf        (EUICCTempDataBuf + EUICCTempDataBufSize)
#define EUICC_AuthenTmpSize       0x200   //euiccCiPKIdToBeUsed + euiccSignature1.字段+ PK.DPpb.ECDSA字段(04 +PkValue) + SIMOID字段

#define  EUICC_SessionKey			(EUICC_AuthenTmpBuf + EUICC_AuthenTmpSize)  //S-ENC LV  S-MAC LV
#define  EUICC_SessionKeyLen		0x40

#define EUICC_ICCIDAndAID         EUICC_SessionKey + EUICC_SessionKeyLen
#define EUICC_ICCIDAndAIDLen			0x25

#define EUICC_OpKeyBuf            (EUICC_ICCIDAndAID + EUICC_ICCIDAndAIDLen)  //该密钥在init SccureChannel 的时候使用 使用后需要被清掉
#define EUICC_OpKeySize            140    //totalLen(1字节) +pkref + pkLen(1字节) +  publicValue(64字节) + skLen(1字节) + skValue(32字节) + smOIDLen(1字节) + smOIDValue(最大41字节长)


#define PROFIEL_CGF_START                           (EUICC_OpKeyBuf + EUICC_OpKeySize)
#define  PROFILE_VALIDTAG_CFG                       (PROFIEL_CGF_START) /*      profile的状态 有效、无效、正在下载、正在删除*/
#define  PROFILE_CATTPPara                         (PROFILE_VALIDTAG_CFG + PROFILE_MAX_NUMBER)              //该字段用于存储profile中的8.6.7  CAT_TP personalisation ，前两个字节是catTpMaxSduSize ，后两个字节是 catTpMaxPduSize. 该参数存储时用的是VMWriteShort，所以读的时候注意
#define  PROFILE_MFOFFSET_CFG                      (PROFILE_CATTPPara + PROFILE_MAX_NUMBER*sizeof(UWORD))

#define  ProfileConfigInfo_SIZE                    ((PROFILE_MFOFFSET_CFG + PROFILE_MAX_NUMBER*sizeof(UWORD))  - PROFIEL_CGF_START)

#define  EUICC_CFG_END                            (PROFILE_VALIDTAG_CFG + ProfileConfigInfo_SIZE)
#else

#define EUICC_CFG_END                              EUICC_CFG_START
#endif


/***********************macro define**************************************/
#define ILLEGAL_PROFILEINDEX                    0xFF  //非法的profile索引

#define PROFILE_INVALIDFLAG                     0xFF  //profile不存在,初始化时，所有的profile状态更新成该值
#define PROFILE_VALIDFLAG                       0x01  //profile 有效 (下载完成，PE-END执行后设置成该标记)
#define PROFILE_LOADINGFLAG                     0x02  //profile正在下载中，创建ISD-P后更新profile状态为此状态，供后续使用
#define PROFILE_DELETELAG                     	0x03  //profile 删除


#define NoSynSE_Profile                         0xFF   //当前profile不需要同步
#define NeedSynSE_Profile                       0x00   //profile需要同步SE

#define ModeSyn_NoCheckTag                      0x00
#define ModeSyn_CheckTag                        0x01


#define PROFILE_STATE_DISABLED                  0x00
#define PROFILE_STATE_ENABLED                   0x01

#define PROFILE_PrivateFileInit                 0x01
#define PROFILE_PrivateFileNoInit               0xFF

#define PROFILE_STATE_AUTH_NOT_UPDATE           0xFF
#define PROFILE_STATE_AUTH_UPDATED              0x88



/***********************global variable define****************************/
extern BYTE  xdata G_HttpExecutingProfileIndex;
extern BYTE  xdata G_RAMBackExecutingProfileIndex;
extern BYTE  xdata G_CurEnabledProfileIndex;  //当前激活的profile index ，RSP和M2M版按照实际值在开机或切换profile后设置

/***********************function define***********************************/
extern void EUICCInitGobalVariable(void);
extern void InitCardConfigArea(void);
extern UWORD  GetArrayDataAddrByRefAddr(UWORD RefAddr);


#define  EUICCEIDInfo_Addr      GetArrayDataAddrByRefAddr(EID_Array_JC)
#define  EUICCEIDLVLEN          0x11          //EID的LV长度
#define  EUICCECASD66LV_Addr    GetArrayDataAddrByRefAddr(ECASD_StoreDataBuf_JC)

#define  EUICCCARef_Addr                           FirstCICert_JC           //2字节的引用用于存储第一个CA证书的引用，
#define  EUICCEUMUICCCARef_Addr					   FirstEUMAndEuiccCert_JC //RSP版
#define  M2M_EUICCEUMUICCCARef_Addr	               FirstM2MCAEuiccCert_JC

#define  EUICCNotificationSeqNumber                EUICCNotificationSeqNumber_JC
#define  EUICCRATRef_Addr                          RATTableRef_JC //指向第一个RAT TABLE的引用
#define  EUICCSMDPDefaultAddress_RefInitial_Addr   EUICCSMDPDefaultAddress_RefInitial_JC
#define  EUICCSMDPDefaultAddress_Ref_Addr          EUICCSMDPDefaultAddress_Ref_JC
#define  EUICCRootSMDSAddress_Ref_Addr             EUICCRootSMDSAddress_Ref_JC
#define  EUICCNotificationsRef_Addr                NotifycationRef_JC

#define  EUICCINFO2_SECTION1                         GetArrayDataAddrByRefAddr(EUICCINFO2_SECTION1_Ref_JC)// LV结构 固定1 + 15字节,初始值为810302010082030202008303040200
#define  EUICCINFO2_SECTION1_LEN                     15
#define  EUICCINFO2_SECTION2                        GetArrayDataAddrByRefAddr(EUICCINFO2_SECTION2_Ref_JC)  // 2字节引用, L V结构 1+ 19字节
#define  EUICCINFO2_SECTION2_MAX_LEN                 25//modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP, 最好是做成配置
#define  EUICCINFO2_CATEGORY                         (EUICCINFO2_CATEGORY_JC)             //int型静态域, L V结构
#define  EUICCINFO2_CATEGORY_LEN                     3
#define  EUICCINFO2_SECTION3                         GetArrayDataAddrByRefAddr(EUICCINFO2_SECTION3_Ref_JC)


//__align(4) SHA1_CTX  xdata G_LoadSha1Context;
//extern SHA1_CTX  xdata G_LoadSha1Context;

//extern BYTE xdata G_ComponetCurrentStep;  //当前组件处理到第几步?
//extern BYTE xdata G_CapMinorVersion;	//缓存当前正在下载的包的次版本号
//extern BYTE xdata G_LoadTmpBufOffset;  //有效数据在G_LoadTmpBuf中的偏移
//extern UWORD xdata G_LoadComponentRemainLen;   //当前正在处理的组件的剩余长度
//extern BYTE xdata G_LoadRecOffset;  //当前正在处理的APDU数据的偏移
//extern BYTE xdata G_LoadTmpBuf[20];  //处理组件过程中用于缓存一些临时数据
//extern UWORD xdata G_C4D4Len;     //TAG C4 D4的长度	 //中国移动研究院NFC2.0测试脚本/GP功能补充测试_20150507_en/东信	/apdu命令要求/load指令测试/6.1.2.2.11  检查C4/D4所标识的数据长度和后续TLv的数据长度时候一致
//extern BYTE xdata G_CapComponetTag;  //当前正在处理的组件的tag
//extern BYTE xdata G_LoadHashBuffer[0x15];  //LV结构，存储当前在load 时需要计算的hash值
//extern BYTE xdata G_CapTagInvalid;   //缓存当前正在处理的tag是有效tag还是非法tag。中国移动tck302要求过滤掉代码中的非法tag
//extern BYTE xdata G_SelectPriorityTable[TOTAL_APP_INSTALLED_MAX+1];	 //用于存放选择优先级，存储内容为appid ；结构如下L	Volatile app1	Volatile app2	...	永久app1	永久app2 ……    L-1代表有效的应用个数

extern BYTE xdata G_EUICCResponse[72]; //LV结构 ，最多拼装5个警告  ，按错误且带警告的情况开设数组长度 （13*5  + 13（最后一个错误码）+ 6（30xx + a0xx + 8100）+ 1(总L)）  EUICC profile下载时用于缓存PEStatus的全局数组，不使用Resdata，使用Resdata会冲掉0通道上的主动式指令

#if EUICC__FUNCTION__GUOMI_eSIM
	extern BYTE xdata G_CertType; //用于判断当前证书是国际证书还是国密证书，国际证书为0，国密证书为1，该变量初始值为0，默认为国际证书
#endif

#define Cert_ECC	0x00
#define Cert_GUOMI	0x01

#define  G_CapComponetTag  g_cap_tag

#define GetObjectHeaderAddr   locateObjectInMemory//GetAddrByTableIndex
#endif
