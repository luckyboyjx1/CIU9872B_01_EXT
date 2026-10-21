#ifndef __OPFUNCTION_H__
#define __OPFUNCTION_H__
#include "profile.h"
#include "Apdu.h"

#define CHAINING_CLEAR				0
#define CHAINING_PUTKEY		   		1
#define CHAINING_INSTALL		    2
#define CHAINING_DELETE		  		3
#define CHAINING_STOREDATA		    4

#define SCP_01            						0x01
#define SCP_02            						0x02
#define SCP_03            						0x03
#define SCP_80            						0x80
#define SCP_81            						0x81
#define SCP_10            						0x10
#define SCP_11            						0x11
#define SCP_FF									0xFF

#define BASE_KEY								0x00 // 该开关控制的两个函数generateStaticKeyScp03、generateStaticKey未被调用

#define CONSTANTS_S_ENC   						0x82
#define CONSTANTS_C_MAC   						0x01
#define CONSTANTS_R_MAC   						0x02
#define CONSTANTS_DEK     						0x81

#if GP__AMENDMENT__D
	#define R_ENC						0x20
#endif

/*
for Security Levels
---------------------------------------------
b8|b7|b6|b5|b4|b3|b2|b1|Description
  |  |	|  |  |  |  |1 |C_MAC
  |  |  |  |  |	 |	|0 |NO_C_MAC
  |  |  |  |  |	 |1 |  |C_DECRYPTION
  |  |  |  |  |	 |0 |  |NO_C_DECRYPTION
  |  |  |  |  |1 |  |  |ISO7816_C_MAC
  |  |  |  |  |0 |  |  |
  |  |  |  |1 |  | 	|  |ISO7816_AUTHENTICATED
  |  |  |  |0 |  | 	|  |
  |  |  |1 |  |  |	|  |R_MAC
  |  |  |0 |  |  |	|  |NO_R_MAC
  |  |1 |  |  |	 |	|  |R_ENCRYPTION
  |  |0 |  |  |	 |	|  |NO_R_DECRYPTION
  |1 |  |  |  |	 |	|  |
  |0 |  |  |  |	 |	|  |
1 |  |  |  |  |	 |	|  |AUTHENTICATED
0 |  |  |  |  |	 |	|  |NO_AUTHENTICATED
---------------------------------------------
*/


#define SCF_C_MAC_NULL								0x00
#define SCF_C_MAC_FLAG								0x01
#define SCF_DECRYPTION_FLAG             			0x02
#define SCF_C_MAC_DECRYPTION_FLAG             		0x03
#define SCF_R_MAC_FLAG             					0x10
#define SCF_ENCRYPTION_FLAG             			0x20
#define SCF_C_MAC_R_MAC_FLAG						0x11
#define SCF_R_MAC_ENCRYPTION_FLAG					0x30

// if the BeginRmacSession, or External Authentication contain RMAC flag, but in this command do not GenRmacResponse.
#define SCF_ZERO_APDU_RMAC_FLAG        				0x04
#define SCF_ZERO_APDU_RMAC_FLAG_MASK   				0xFB
#define SCF_ANY_AUTHENTICATED     				0x40

#define SCF_NO_AUTHENTICATED  						0x00
//SCF_AUTHENTICATED
#define SCF_AUTHENTICATED     						0x80
#define SCF_C_MAC             						0x81
#define SCF_DECRYPTION             					0x82
#define SCF_C_MAC_DECRYPTION        				0x83

#define SCF_80_AUTHENTICATED        				0x88

#define SCF_R_MAC             						0x90
#define SCF_C_MAC_R_MAC        						0x91
#define SCF_R_MAC_DECRYPTION        				0x92
#define SCF_C_MAC_R_MAC_DECRYPTION        			0x93

#define SCF_ENCRYPTION             					0xA0
#define SCF_C_MAC_ENCRYPTION        				0xA1
#define SCF_ENCRYPTION_DECRYPTION       			0xA2
#define SCF_C_MAC_DECRYPTION_ENCRYPTION        		0xA3

#define SCF_R_MAC_ENCRYPTION        				0xB0
#define SCF_C_MAC_R_MAC_ENCRYPTION        			0xB1
#define SCF_R_MAC_DECRYPTION_ENCRYPTION        		0xB2
#define SCF_C_MAC_R_MAC_DECRYPTION_ENCRYPTION		0xB3

//SCF_ANY_AUTHENTICATED
#define SCF_AUTHENTICATED_ANY     						0x40
#define SCF_C_MAC_ANY             						0x41
#define SCF_DECRYPTION_ANY             					0x42
#define SCF_C_MAC_DECRYPTION_ANY        				0x43

#define SCF_R_MAC_ANY             						0x50
#define SCF_C_MAC_R_MAC_ANY        						0x51
#define SCF_R_MAC_DECRYPTION_ANY        				0x52
#define SCF_C_MAC_R_MAC_DECRYPTION_ANY        			0x53

#define SCF_ENCRYPTION_ANY             					0x60
#define SCF_C_MAC_ENCRYPTION_ANY        				0x61
#define SCF_ENCRYPTION_DECRYPTION_ANY       			0x62
#define SCF_C_MAC_DECRYPTION_ENCRYPTION_ANY        		0x63

#define SCF_R_MAC_ENCRYPTION_ANY        				0x70
#define SCF_C_MAC_R_MAC_ENCRYPTION_ANY        			0x71
#define SCF_R_MAC_DECRYPTION_ENCRYPTION_ANY        		0x72
#define SCF_C_MAC_R_MAC_DECRYPTION_ENCRYPTION_ANY		0x73

#define DEMAND_AUTHENTICATE			0x00
#define IGNORE_AUTHENTICATE			0x01

#define ISO7816_NO_AUTHENTICATED                0x00
#define ISO7816_AUTHENTICATED                   0x08
#define ISO7816_C_MAC                           0x0C
/************第1字节权限对应bit位****************************/
#define			SECUREDOMAIN_PRIVI_TAG		        0x80
#define			DAP_VERIFICATION_PRIVILEGES_TAG		0xC0
#define			AUTHORIZED_PRIVILEGES_TAG			0x40
#define			TOKEN_PRIVILEGES_TAG				0x20
/************第3字节权限对应bit位****************************/
#define			RECEIPT_PRIVILEGES_TAG				0x80
#define			CIPHERED_LOAD_PRIVILEGES_TAG	    0x40
#define			GLOBAL_DELETE_PRIVILEGES_TAG	    0x10

#define			IS_APPLET_EXTRADTION_FLAG			0x01
#define			IS_FINAL_APPLICATION_FLAG			0x02

#define INS_OP_MANAGE_SECURITY_ENVIRONMENT 					0x22
#define INS_OP_PERFORM_SECURITY_OPERATION 					0x2A
#define INS_OP_INTERNAL_AUTHENTICATION 						0x88
#define INS_OP_GET_CHALLENGE								0x84
#define INS_OP_EXTERNAL_AUTHENTICATION						0x82

#define INS_OP_APPEND_RECORD  					(u8)0xE2
#define INS_OP_SELECT         					(u8)0xA4
#define INS_OP_DELETE         					(u8)0xE4
#define INS_OP_GET_DATA       					(u8)0xCA
#define INS_OP_GET_STATUS     					(u8)0xF2
#define INS_OP_INSTALL        					(u8)0xE6
#define INS_OP_LOAD           					(u8)0xE8
#define INS_OP_PIN_VERIFY 						(u8)0x20
#define INS_OP_PIN_CHANGE 						(u8)0x24
#define INS_OP_PIN_DISABLE						(u8)0x26
#define INS_OP_PIN_ENABLE 						(u8)0x28
#define INS_OP_PIN_UNBLOCK						(u8)0x2C
#define INS_OP_PUT_DATA       					(u8)0xDA
#define INS_OP_PUT_KEY        					(u8)0xD8
#define INS_OP_SET_STATUS     					(u8)0xF0
#define INS_OP_STORE_DATA     					(u8)0xE2
#define INS_OP_PERFORM_TRANSACTION     			(u8)0x12
#define INS_OP_ELF_UPGRADE     					(u8)0xEA
#define INS_INITIALIZE_UPDATE 					(u8)0x50
#define INS_VERIFY            					(u8)0x20
#define INS_EXTERNAL_AUTHENTICATION             (u8)0x82

#define CLA_OP_ISO7816_ENC						(u8)0x88
#define CLA_OP_ISO7816_MAC						(u8)0x8C

#define INS_OP_BEGIN_RMAC_SESSION               (u8)0x7A
#define INS_OP_END_RMAC_SESSION                 (u8)0x78

#define INS_OP_GENERATE_KEYPAIR					(u8)0x42
#define INS_OP_GET_RESPONSE						(u8)0xC0

#define INS_OP_EXTEND_OPEN_MIFARE               (u8)0xA6
#define INS_OP_EXTEND_CLOSE_MIFARE              (u8)0xA7


#define P1_OP_CARD_STATUS    					0x80
#define P1_OP_APPLET_STATUS  					0x40
#define P1_OP_FILE_STATUS    					0x20
#define P1_OP_APPLET_FILE_STATUS  				0x10
#define P1_OP_INSTALL_FOR_LOAD    				0x02
#define P1_OP_INSTALL_FOR_INSTALL                     0x04
#define P1_OP_INSTALL_FOR_MAKE_SELECTABLE             0x08
#define P1_OP_INSTALL_FOR_INSTALL_AND_MAKE_SELECTABLE 0x0C
#define P1_OP_INSTALL_FOR_EXTRADITION 0x10
#define P1_OP_INSTALL_FOR_PERSONALIZE 0x20
#define	P1_OP_INSTALL_FOR_REGISTRY_UPDATE 0x40

//ELF UPGRADE merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP to support GP__AMENDMENT__H__FOR_MASK
#define P1_OP_ELF_UPGRADE_START     0x01
#define P1_OP_ELF_UPGRADE_RESUME    0x02
#define P1_OP_ELF_UPGRADE_RECOVERY  0x03
#define P1_OP_ELF_UPGRADE_ABORT     0x04
#define P1_OP_ELF_UPGRADE_STATUS    0x08

#define CARD_RECOG_DAT_64		0x64
//get data tag
#define	CARD_WALLETAPPSEID		     (u16)0x00D1       //硬件唯一标识
#define CARD_WALLETMANAGEMENTINFO    (u16)0xDF30   //硬件钱包管理信息
#define	CARD_ISSUER_DAT				 (u16)0x0045
#define CARD_ISSUER_NUM        (u16)0x0042         //卡发行商编号
#define CARD_ID                (u16)0x0044         //卡唯一标识
#define CARD_DATA              (u16)0x0066         //卡数据
#define CARD_REAL_RESOURCE	   (u16)0xEF20		   //实际卡资源
//#define CARD_RESOURCE_DATA   (u16)0xFF20         //卡资源数据
#define CARD_SEID              (u16)0x0011			//SEID wangwenbo
#define MENU_PARAMETER         (u16)0xFF1F         //菜单信息 GSM TS0348
#define CARD_E2E_SPACE         (u16)0xFF20         //E2P空间 GSM TS0348
#define CARD_RESOURCE_DATA     (u16)0xFF21         //卡资源数据 ETSI TS102226
#define KEY_INFO               (u16)0x00E0         //当前安全域的密钥信息
#define DEFALUT_KEY_INFO (u16)0x00C1
#define CARD_KEYVERSION        (u16)0x00E2         //卡商密钥信息
#define ACTUAL_FREE_ROOM       (u16)0xEF20         
#define SIM_VERSION            (u16)0x2F10         //SIM卡的版本信息，SE上不做要求
#define SE_VER_INFO            (u16)0x2F12         //SE版本信息
#define SE_ID_BYD              (u16)0x0011         //比亚迪需求SEID长度为24字节，前16字节为分散因子信息(SEID_BYD)，后8字节为随机数信息(随机生成)，这个TAG，是我们自己定义的

#define SE_CSL                 (u16)0x00D3          //当前踩燃栋
#define KEY_SEQUENCE           (u16)0x00C0         //安全域默认密钥的计数器
//#define OTA_COUNTER            (u16)0x00C2         //OTA3引擎计数器，SE上不做要求
#define CARD_APP_INFO          (u16)0x2F00         //安全域剩余空间及已安装应用信息
#define CAED_APP_INFO1			(u16)0x2F01
#define SSD_DATA               (u16)0x0072         //辅助安全域数据

#define OP_TLV_ADMIN_SESSION_PARA (u16)0x0085      //AMD B tag
#define OP_TLV_DNS_Resolver       (u16)0x00F5      //AMD B tag

#define VGP_KEY_SEQUENCE	   (u16)0x00C1
#define DIVERSIFIEDATA	   (u16)0x00CF
u8 find_write_Tag(memref addr, u8 tag, u8 scpType, u8 option);
#if SCP_SWITCH
//u16 setScpProtocol(void) small;
#endif

#define SD_TYPE                   									  0
#define IS_ALLOWED_EXTRADITON   										  1   //not used
#define SD_DAP_KEY_VERSION        										2
#define SD_SCP                    										3
#define SD_MAX_AUTH_ATTEMPT      									    4
#define SD_FREE_AUTH_ATTEMPT      										5

#define SD_TYPE_NOT_ALLOW_DELETE        						  6
#define SD_TYPE_APPLET_CANNOT_DELETE    						  7
#define SD_TYPE_NOT_ALLOW_APPLET_INSTALL_FROM_ISD 		8
#define SD_TYPE_NOT_ALLOW_APPLET_LOAD_FROM_SD    		 	9
#define SD_TYPE_NOT_ALLOW_LOCK_FROM_ISD               10
#define SD_TYPE_ALLOW_APPLET_DELETE_FROM_SSD          11   
#define SD_TYPE_RECEIPT_GENERATION_FOR_SSD            12


#define SD_ALLOW_PACKAGE_RECEIVE_EXTRADITION_FROM_SD  13
#define SD_ALLOW_APPLET_RECEIVE_EXTRADITION_FROM_SD   14

#define SD_ALLOW_PACKAGE_EXTRADITION_FROM_SD          15
#define SD_ALLOW_APPLET_EXTRADITION_FROM_SD           16
#define SD_ALLOW_DELETION_FORM_SD					 						17

#define SD_SEARCH_UNDER_AMSD_TOKENVERIFICATION_SD     18
#define SD_SEARCH_UNDER_AMSD_RECEIPTGENERATION_SD     19

#if GP__CGM__FOR_APPPARACHECK
#define SD_MANAGE_RAM_SIZE         										20
#define SD_MANAGE_NVM_SIZE          									21
#define SD_CGM_REMAIN_RAM_SIZE                        22
#define SD_CGM_REMAIN_NVM_SIZE                        23
#define SD_CGM_RAM_SIZE							                  24
#define SD_CGM_NVM_SIZE							                  25
#define SD_REMAIN_NVM_CHECK                           26
#define SD_REMAIN_RAM_CHECK                           27
#define SD_CGM_RAM_SIZE_90                            28
#define SD_CGM_NVM_SIZE_8F                            29
#endif
#define NO_TOKENORRECEIPT_SD                          0xEE
//用于C9tag下828387编码的各种情况区分    高5个bit有效，低3bit RFU
#define	SD_IS_ISD_MASK                  0x30
#define SD_IS_AMSD_MASK				    0xF0
#define SD_IS_DMSD_UNDER_AMSD_MASK		0x08
#define SD_HAS_AMSD_ANCESTOR_MASK       0xD0
#define SD_HAS_AMSD_HIERARCHY_MASK		0xD0

#define SD_IS_ISD                       0x20
#define SD_IS_AMSD						0xF0
#define SD_IS_DMSD_UNDER_AMSD			0x08
#define SD_HAS_AMSD_ANCESTOR			0x80
#define SD_HAS_AMSD_HIERARCHY			0xC0

#define INFO_TYPE_GSM_COUNTER       0
#define INFO_TYPE_DAP_KEY_ARRAY     1
#define INFO_TYPE_PAG_SIZE          2

#define SD_RECEIPT_GENERATION                   0x01   //1: generation receipt  0: no
#define SD_NOT_ALLOW_DELETE                     0x02   //1: not allow delete    0: allow delete
#define SD_NOT_ALLOW_APPLET_DELETE_FROM_ISD     0x04
#define SD_NOT_ALLOW_APPLET_INSTALL_FROM_ISD    0x08
#define SD_NOT_ALLOW_APPLET_LOAD_FROM_SD        0x10
#define SD_NOT_ALLOW_LOCK_FROM_ISD              0x20   //1: not allow           0: allow
#define SD_TYPE_FIX_SIZE                        0x80

//
//  b0 b1  for receive extradition
//  b3 b2  for allow extradition out
//  b5 b4  for applet delete
#define SD_NOT_ALLOW            0x00
#define SD_ALLOW_FROM_ISD       0x01
#define SD_ALLOW_FROM_SSD       0x02

#define SD_RECEIVE_EXTRADITION         0x03
#define SD_APPLET_EXTRDITION_OUT       0x0C
#define SD_APPLET_DELETE               0x30

#define SD_INSTALL                     0x00
#define SD_DELETE                      0x00

#define DEMAND_AUTHENTICATE			0x00
#define IGNORE_AUTHENTICATE			0x01

#define ANTI_AUTH_ATTACK_LIMIT          0x0F


// used for the bit define in variable of g_BeginEnd_R_MAC 
#define BEGIN_RMAC_ISSUED_MASK    0x01
#define ALL_RESPONSE_WITH_RMAC_MASK    0x02
#define GET_CURRENT_RMAC_MASK    0x04
#define GET_CURRENT_RMAC_AND_END_MASK    0x08

#define APDU_DATAFIELD_IS_NOT_RECEIVED			0xA5
#define APDU_DATAFIELD_IS_RECEIVED				0x5A

#define GENERATE_CARD_CRYPRTOGRAM             	0x00
#define COMPARE_CARD_CRYPRTOGRAM              	0x01

#define CalRMAC_Ture 							0x01
#define CalRMAC_False 							0x00

#define IS_NEED_DO_PADDING_YES					0xA5
#define IS_NEED_DO_PADDING_NO					0x5A

#if GP__AMENDMENT__D
	#define AES_KEY_TYPE						0x88
#endif
#define DES_KEY_TYPE							0x80

#define AIDLenInvalid(len) ((len < 5)||(len > 16))

//#if GP__SCP__SCP11__C_AUTHORIZATIONS //merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
#define BF20_INSTALL_OR_LOAD_AID_TAG			0x8A
#define BF20_CREAT_APP_PRIVILEGE_TAG			0xC5
#define BF20_MAX_VOLATILE_CGM_TAG				0x82
#define BF20_MAX_NON_VOLATILE_CGM_TAG			0x83
#define BF20_DGI_LIST_STOREDATA_TAG				0x8B
#define BF20_TLV_LIST_STOREDATA_TAG				0x8C
#define BF20_DELETE_APP_ELF_AID_TAG				0x8E
#define BF20_UPGRADED_ELF_AID_TAG				0x8F
//#endif
#if GP__SCP__SCP11__FOR_MASK
extern u16 check_SCP11cOPCommand(void);
#endif

extern u16  StoreReceivedCommandAPDU(void);
extern u16  checkOPCommand(void);
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
extern void managerSDLifeCycle(void);
#endif
extern memref getCurentAidAddrss(PX8 aidLength);
extern u16 check_Op_Command1(void);
extern u16 check_Op_Command2(void);
extern void storeOption(void);
extern void SCPAPIImplement(void);
extern void setNoRmac(void); 
extern void setATRHistBytesNM(void);

#if GPC_2_3_1
extern u8 getATRHistBytesNM(void);

#if BOOT__IO__SWP
extern u8 getUIDorPUPIBytesNM(void);
#endif

#endif

#if !GP__VERSION__FOR_MINI_MASK
		extern void Get_CPLC_Data(void);
		extern void Put_CPLC_Data(void);
		extern void dealC9Tag(void);
#else
		#define Get_CPLC_Data()
		#define Put_CPLC_Data()
		#define dealC9Tag()
#endif
#if GP__SCP
    extern void SCPAPIImplement(void);
#else
		#define SCPAPIImplement()
#endif
#if !GP__VERSION__FOR_MINI_MASK
extern void isActiveApp(u16 appId);
#else
#define isActiveApp(appId)
#endif

extern void initSDKeyMap(u8 AppId, u8 sTagOff, u8 bC9Len);
extern void deleteSDKeyMap(u8 sdId);
extern void deleteSD(u8 sdId);
extern u32 getInfo(u8 sdId, u8 type,u16 cgmRam , u32 cgmNvm);
extern void getSDInfo(void);
extern void getPackageSize(void);
extern void managepackageSize(void);
extern void getPagSize(u8 index, u8 * buf);
extern u16 putData(void);
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
extern u16 getAppInfoCMS2AC(u8 sdId, u8 * buf, u16 * dataLen);
#else
extern u16 getAppInfo(u8 sdId, u8 * buf, u16 * dataLen);
#endif
extern u8 getAppletNum(void);
extern u8 iSetSDPersonalizeState(u8 bSdid);
extern void manageKeyMapOfSD(void);
extern	memref   getSdDataObjBySdid(u8 bSdid);
extern u8 getCurrentSDId(void);
extern 	void OPCMGenData(void);
#if (GP__TOKEN__FOR_MASK | GP__DAP__FOR_MASK | GP__RECEIPT__FOR_MASK)
    extern	void OPGetKeySetStatus(void);
#else
    #define OPGetKeySetStatus(void)
#endif
extern	void OPCMVerifyImplement(void);
extern u8 check_FinalApplicationPrivi(u8  sdId);
#endif
		
extern u8 GetAppIdByAIDFromAIDList(u8 *aidValue,u8 aidLen);		
extern u8 GPSSDExitCompleteProtolKey(u8 sdId);		

