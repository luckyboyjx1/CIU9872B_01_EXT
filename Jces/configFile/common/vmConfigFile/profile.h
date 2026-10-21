/*****************************************************************************
 *profile.h
 *config the OS.
 *wang hui
 *eastcompeace Technology Corporation
 *Flags:
 *.
 *Note:
 *.
 *History:
 *2007,6,23	.Created
 *****************************************************************************/

#ifndef   __PROFILE_H__
#define   __PROFILE_H__

#include "JcvmPlatformProfile.h"
#include "SysConfig.h"
#include "appHeader.h"

/*=========================================================================== */
/*							bootloader  enable 								  */
/*=========================================================================== */
#define BOOTLOADER_PRO                  			0

//卡片支持的在线升级功能类型，以下2个开关可以全部为0，但是最多只能打开一个
#if HAL__PLATFORM__SIMULATOR
#define COS_UPDATE_ONLINE_BY_NativeOS						0  // 只能是0    simulator软仿没有BootLoader以及cosEntry，不支持升级，调试代码需要做特殊处理（目前没有做）
#define COS_UPDATE_ONLINE_MUTUAL								0  // 只能是0    simulator软仿没有BootLoader以及cosEntry，不支持升级，调试代码需要做特殊处理（目前没有做）
#else
#define COS_UPDATE_ONLINE_BY_NativeOS						0 // 0/1			仅支持通过NativeOS在线升级
#define COS_UPDATE_ONLINE_MUTUAL								0 // 0/1			支持COSA、COSB相互在线升级
#endif

#if COS_UPDATE_ONLINE_MUTUAL
#define UPCOS_COSA                      			1                    //UPCOS功能  A/B二选一
#define UPCOS_COSB                      			0
#endif

#define SUPPORT_COS_UPDATE						(COS_UPDATE_ONLINE_MUTUAL||COS_UPDATE_ONLINE_BY_NativeOS||BOOTLOADER)//支持cos升级（在线或离线）

/****************************************************************************************************************************************
 these macro  DON't CARE !!!!!
 these macro  DON't CARE !!!!!
 these macro  DON't CARE !!!!!
*****************************************************************************************************************************************/
//卡片类型  以下3个开关必须打开一个 且只能打开一个
#define    EUICC_ONLY                               0
#define    SE_ONLY                                  0
#define    SE_AND_EUICC                             1

//卡片支持的UICC通讯功能类型，以下2个开关可以全部为0，但是最多只能打开一个
#define    JAVA_CARD_UICC                           1  // 1 indicated support JAVA CARD UICC function
#define    NATIVE_CARD_UICC                         0  // 1 indicated support NATIVE CARD UICC function

#if (EUICC_ONLY || SE_AND_EUICC)
#define    RSP_EUICC                                1    // either 1(RSP) or 0(M2M), can't be same with M2M_EUICC
#define    M2M_EUICC                                0
#if M2M_EUICC
#define    M2M_ESx										1   //SGP02 V4.2 4.1.4 ESx (Device - eUICC) Interface Description
#else
#define    M2M_ESx										0
#endif

#define    RSP_SGP22_BF22                  		0   //add by xieyong@20220914 SGP22V2.4新增的tag先用宏隔开

#define    EUICC_FUNC_ASN1_CODING                   1    //ASN.1编码解析功能打开  支持EUICC时 该开关必须打开
#define    EUICC_FUNC_SCP03t						1    // =1 : support SCP03t  ,支持EUICC功能时打开

#define    PROFILE_GENERATE_CREATEFILE_MORE30       1    // 0 or 1

#define    UICC_NEW_FILESYSTEM                      0

#define    UICC__OPERATOR__CHINAMOBILE              0 // only be 0
#define    UICC__OPERATOR__CHINATELECOM             0 // only be 0
#define    UICC__OPERATOR__CHINAUNICOM              0 // only be 0
#define    PE_CHECK_UNKNOWTAG                       1  //检查PE中的非法tag ，不符合规范报错  ，不打开该开关 直接忽略（但是没有做长度偏移 可能会在其他地方出错 ，该情况无法知道出错的PEID）
#else //#if (EUICC_ONLY || SE_AND_EUICC)

#define    RSP_EUICC                                0    // only be 0
#define    M2M_EUICC                                0    // only be 0

#define    EUICC_FUNC_ASN1_CODING                   0    //ASN.1编码解析功能关闭
#define    EUICC_FUNC_SCP03t						0

#define    UICC__OPERATOR__CHINAMOBILE              0
#define    UICC__OPERATOR__CHINATELECOM             0
#define    UICC__OPERATOR__CHINAUNICOM              0

#define    UICC__FUNCTION__2B80_AT_FILEEND  		    0    // 2B80 文件放在文件系统最后，记录数可以动态增加或减少,该开关只适用于SE_ONLY

#endif // #if (EUICC_ONLY || SE_AND_EUICC)

#define    UICC__OPERATOR__CHINATELECOM__4G        0

#if UICC__OPERATOR__CHINATELECOM__4G   
#define    UICC__CDMA_2G_NOT_SUPPORT               0
#define    UICC__GSM_2G_NOT_SUPPORT                0
#else
#define    UICC__CDMA_2G_NOT_SUPPORT               1
#define    UICC__GSM_2G_NOT_SUPPORT                0
#endif
#define UICC__FUNCTION__CATTP                      0    //代码量约 9K

#define UICC__AUTH__GBA                      	 	1
#define UICC__AUTH__XOR                             1
#define UICC__AUTH__GSM__V1V2V3                     1
#define UICC__AUTH__TUAK                            1
#define UICC__AUTH__LOCALKEY                        1
#define UICC__AUTH__MBMS                            1
#define UICC__AUTH__EAP                             1
#define UICC__AUTH__EPS                             1
#define UICC__AUTH__COUNTER                         1

#define UICC__ISIM                                  1

#define UICC__AUTH__5G__SUPI                      	1
#define UICC__AUTH__5G__SUSPEND                     0

#if UICC__OPERATOR__CHINAMOBILE
#define UICC__APP__WHITECARD              1
#define UICC__APP__LIVECARD               1
#define UICC__APP__UPLOADINFO             1
#define UICC__APP__SIMINFO                1
#define UICC__FUNCTION_CMCCAPI			      1
#else
#define UICC__APP__WHITECARD              0
#define UICC__APP__LIVECARD               0
#define UICC__APP__UPLOADINFO             0
#define UICC__APP__SIMINFO                0
#define UICC__FUNCTION_CMCCAPI			      0
#endif

#if JAVA_CARD_UICC
#define UICC__FUNCTION__SIMAPI                      1
#define UICC__FUNCTION__UICCAPI                     1
#define UICC__FUNCTION__USIMAPI                     1
#define UICC__FUNCTION__STKAPI						1
#define UICC__FUNCTION__CSIMAPI                     0  // 代码量约576字节, 另外对应的JAVA约380字节
#endif

#define UICC__FUNCTION__SINGLEDES					1
#define UICC__STKAPI_TOOLKITINFO_IN_HEAP			1
#if UICC__FUNCTION__SIMAPI
#define SIMAPI_SUPPORT_123EVENT         1 // only valid in SIM API
#else
#define SIMAPI_SUPPORT_123EVENT         0
#endif

#define VERIFYPINEVENT                         0

#if !BOOTLOADER_PRO
#define UICC__FUNCTION__BERTLVCmd					1
#else
 #define UICC__FUNCTION__BERTLVCmd					0
#endif

#define UICC__FUNCTION__DFLink  					1

#define UICC__FUNCTION__UniversalPin  					1

#define EUICC__FUNCTION__GUOMI_eSIM   				0   // 代码量约636字节
#define EUICC__FUNCTION_RedTea_Mobile				0

#define UICC_RFM_FILE_UPDATE_EVENT                  1  // 代码量约1212字节

#define UICC_SUPPORT_CHAGE_DELIVER_POR	          	0  //当deliver por长度超过一条，截断返回62F1的时候，是否要改变指令执行个数和响应个数相匹配

#define UICC_SUPPORT_POR_DELIVER_TO_SUBMIT	        0  //当deliver por长度超过一条 , 转换成 Submit 再发送

#define UICC_FUCTION_PRIVATEAUTH					0 //是否支持私有鉴权
#define GW_PRIVATE_GM_CRYPTO						0 //GW Private GM(SM1,SM4,SM7)

#define UICC_AB8C_ARR_PRIVATE                       1 //AB/8C格式存放在Af06私有文件

#define UICC_SUPPORT_RFMHANDLER_D0                  1//RFM中支持主动式命令,by xwh 20211117

#define UICC_USATTEST                               0

#define High_Update_InRAM_New                       0 // 高频更新的文件放在ram中，针对联通7FF0/6F5B文件的需求处理

//以下五个开关不需要与运营商绑定，可以根据需求来决定。目前电信5G备案是全开。EUICC全开。其他情况全关。
#define UICC__AUTH__CDMA__CAVE                      1
#define UICC__AUTH__CDMA__MD5                       1
#define UICC__CDMA__SMS__DOWNLOAD              		1
#define UICC__CDMA__SMS__PRL                        1
#define UICC__CDMA__CATTP__PRL                      0
//config for DTR RTR function
#define JCVM__RAM__DTR__RTR__NEW                    0     //DTR RTR 新方案，只有中国电信平台需要打开，其他平台不用打开,by xwh 20220525


//config for Card features
#define JCVM__IO__SWP                      	 		BOOT__IO__SWP
#define JCVM__IO__MIFARE                            BOOT__IO__MIFARE
#define JCVM__IO__SPI                      	 		BOOT__IO__SPI
#define JCVM__IO__I2C                      	 		BOOT__IO__I2C

#define JCVM__MEMORY__TRANSIENT_MANAGER				1
#if JCVM__MEMORY__TRANSIENT_MANAGER
	#define JCVM__MEMORY__DTR_SPACE					1
	#define JCVM__MEMORY__TRAN_DEALLO				1
	#define JCVM__MEMORY__TRAN_ALLO					1
	#define JCVM__MEMORY__TRAN_MANAGER				1        //
#else
	#define JCVM__MEMORY__DTR_SPACE					0
	#define JCVM__MEMORY__TRAN_DEALLO				0
	#define JCVM__MEMORY__TRAN_ALLO					0
	#define JCVM__MEMORY__TRAN_MANAGER				0
#endif

#define	  JCVM__MEMORY__NVM_RESIZE              	1

#define	  JCVM__BOOTCOMMAND__LOCKED             	1
#define   JCVM__CONFIGDATA__CARD_VERSION   	        0

//config for JavaCard features
#define JCVM__VM__JAVACARDX_FRAMEWORK_UTIL			0        // =1 : support javacardx.framework.util
#define JCVM__VM__JAVACARDX_FRAMEWORK_UTIL_INTX  	0        // =1 : support javacardx.framework.util.intx
#define JCVM__VM__JAVACARDX_MATH                    0
#define JCVM__VM__RMI      							0        // =1 : support java.rmi

#define JCVM__VM__CHECK_STACK             			1        // =1 : Runtime checking for stack underflows
#define JCVM__VM__CHECK_STACK_POP					1
#define JCVM__VM__CHECK_LOCAL         				1	           /* check local variabel is in rang.	   */
#define JCVM__VM__FIRE_WALL	   						1        // =1 : support object owner check   config for JavaCard feature (Fire wall)
#define JCVM__VM__NULLOBJ_CHECK						1
#define JCVM__VM__NOT_TRUSTED             			1       /* Runtime checking of memory access addresses */

#define JCVM__IO__SWP_SELECT_BY_FILE_ID             0        // =1 : support select by file id command on contactless interface

#define VM_INTS_SUPPORTED							1

/****************************************************************************************************************************************
 these macros config for product!!!
 these macros config for product!!!
 these macros config for product!!!
*****************************************************************************************************************************************/

//config for Algorithm
/*
*	Followings three controlings means that the bad using of DES crypto and mac support or not ,
*	there exists three functions,invoked by scp module and Lte module,
*	if you want ot use it , you shall let it be true.
*	three functions are   single_DESMAC    ,    Triple_DES   ,    HARDWARE_DES
*	if you don't know these ,you shall search these functions , you can open it correctly.
*/
#define	 JCVM__CRYPTO__DES_SINGLE_MAC                   1
#define	 JCVM__CRYPTO__DES_TRIPLE_CIPHER                1
#define	 JCVM__CRYPTO__DES_SINGLE_CIPHER                1


//controls JavaCard Algorithm Api
//OverSea Product shall change them,according to actual Requirement.
/*=========================================================================== */
/*	security features														  */
/*=========================================================================== */
#define VM_WITH_RANDOMDATA 					1
#define VM_WITH_CRC							1
#define VM_WITH_ECC							1
#define VM_WITH_SIGNMR              		0
#define VM_WITH_HMAC                   		1
#define VM_WITH_DES     					1
#define VM_WITH_AES                    		1
#define VM_WITH_ECDSA                       1
#define VM_WITH_RSA     					0
#define VM_WITH_MD5     					1
#define VM_WITH_SHA1    					1
#define VM_WITH_SHA224                 		1
#define VM_WITH_SHA256                 		1
#define VM_WITH_SHA384                 		0
#define VM_WITH_SHA512                 		0
#define VM_WITH_RIPEMD160              		0
#define VM_WITH_KOREANSEED             		0
#define VM_WITH_CHECKSUM					0
#define VM_WITH_MD_INIT						1

#define VM_WITH_GUOMI                  		0
#if  VM_WITH_GUOMI
#define VM_WITH_SM1			        		1
#define VM_WITH_SM2			        		0   // 代码量约1600字节
#define VM_WITH_SM3			        		1
#define VM_WITH_SM4			        		1
#define VM_WITH_SM7			        		0
#define VM_WITH_SSF33			        	0
#endif

#define VM_WITH_SYM							(VM_WITH_DES || VM_WITH_AES || VM_WITH_KOREANSEED || VM_WITH_SM1 || VM_WITH_SM4 || VM_WITH_SSF33)
#define VM_WITH_HASH						(VM_WITH_MD5 || VM_WITH_SHA1 || VM_WITH_RIPEMD160 || VM_WITH_SHA224 || VM_WITH_SHA256 || VM_WITH_SHA384 || VM_WITH_SHA512 || VM_WITH_SM3)


/*===========================================================================*/
/*	re-triming config														 */
/*===========================================================================*/
#define VM_NONEXISTENT_WITH_EDEP			0		//没有JAVA CARD VM，只有EDEP应用.
#define VM_WITH_PBOC                   		0
#define VM_WITH_EDEP                   		0
#define VM_WITH_PBOC_OR_EDEP                (VM_WITH_PBOC || VM_WITH_EDEP)
#define VM_WITH_PBOC_AND_EDEP               (VM_WITH_PBOC && VM_WITH_EDEP)
#define VM_WITH_PBOC_API                    0

#define VM_WITH_JT_FUNC						0		//交通部中交金卡需求，欲开此开关，须保证VM_WITH_PBOC，VM_WITH_EDEP置1
#define VM_WITH_EID							0
#define VM_WITH_EID2						0		// EID2.0+

#define VM_WITH_DCEP   0     //DCEP 币串应用APP
#define VM_WITH_DCEP_ZZH   0 //DCEP 准账户应用  APP
#define VM_WITH_SELECT_BY_FILE_ID      		0
#define VM_WITH_APP_CONFIG					0 //PBOC APP transfer cap config
#define VM_WITH_CONFIG_FILE					(VM_WITH_SELECT_BY_FILE_ID)

#define CONSTANT_POOL_COMPONENT_IN_RAM      1 //constantpool_component_Size小于0x100时用RAM来存储constantpool_component，可以减少分配与回收的次数，提升性能
/*===========================================================================*/
/*	One must not be allowed to change the following Switch					 */
/*===========================================================================*/
#ifndef VM_WITH_DEBUG
	#define VM_WITH_DEBUG					0
#endif

#define EMV_PAYPASS							0			//paypass native API

#define GPC_2_3_1                           0           //support GlobalPlatform Technology Card Specification Version 2.3.1, 对应JAVA层的public final static boolean GPC_2_3_1
#define GP_COMMAND_CHAINING                 1           //support 11.1.5.1  GlobalPlatform Command Chaining
#define GPC_CONFI_V2                        0           //support GPC_Common_Implementation_Configuration_v2.0

//GP function
///brief: config for data manager
#define GP__DATA_MANAGER__FOR_MASK					1        // =1 : support Data Manager

///brief: config for key manager
#define GP__KEY_MANAGER            					1        // =1 : support Key Manager
#define GP__KEY_MANAGER__FORMAT2    				1        // =1 : use private store form for key
#define GP__KEY_MANAGER__FORMAT3    				1        // =1 : use private store form for key

#if GP__KEY_MANAGER
#define GP__KEY_MANAGER__DELETE_KEY           		1        // =1 : support delete key command
#define GP__KEY_MANAGER__STORE_KEY            		0        // =1 : support store data key to put key, 代码量约1004字节
#define GP__KEY_MANAGER__PUT_RSA_KEY                0
#define	GP__KEY_MANAGER__ECC_KEY                    0        // =1 :support put/store EccKey，代码量约1980字节
#else
#define GP__KEY_MANAGER__DELETE_KEY           		0
#define GP__KEY_MANAGER__STORE_KEY            		0
#define GP__KEY_MANAGER__PUT_RSA_KEY                0
#define GP__KEY_MANAGER__ECC_KEY                    0
#endif


#if  GP__KEY_MANAGER__STORE_KEY
#define  GP__KEY_MANAGER__STORE_DGI_KEY           0       // =1 : support store data key to put  DGI key
#else
#define  GP__KEY_MANAGER__STORE_DGI_KEY            0
#endif
///brief: config for Token
#define	GP__TOKEN__FOR_MASK							1        // =1 : support Token , if this macro is 1, need GP__KEY_MANAGER is set 1
///brief: config for DAP
#define	GP__DAP__FOR_MASK							1        // =1 : support DAP,    if this macro is 1, need GP__KEY_MANAGER is set 1
///brief: config for Receipt
#define	GP__RECEIPT__FOR_MASK						1        // =1 : support RECIPT, if this macro is 1, need GP__KEY_MANAGER is set 1
///brief: config for HCI API
#define GP__HCI_API__FOR_MASK                    	BOOT__IO__HCIAPI        // =1 : support HCI API, if this macro is 1, need JCVM__IO__SWP is set 1
///brief: config for CGM
#define GP__CGM__FOR_MASK                  			0         // =1 : support CGM	//注意该开关开启时需要同步配置 OS_CGM_CLOSE 为1， 否则会导致删除profile空间回收大小不对，这个CGM为啥存在两个开关还不能同时开启？
#define GP__CGM__FOR_APPPARACHECK                1         // =1 : support CGM	//注意该开关开启时需要同步配置 OS_CGM_CLOSE 为1， 否则会导致删除profile空间回收大小不对，这个CGM为啥存在两个开关还不能同时开启？

///brief: config for scp
#define GP__SCP										1       // =1 : support SCP, if this macro is 0, follow macros have not meaning
#define GP__SCP__API__FOR_MASK                      1

#if (GP__SCP__API__FOR_MASK && JAVA_CARD_UICC)
#define GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP8x   1
#else
#define GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP8x   0
#endif

#define GP__SCP__SCP01								    0        // =1 : support SCP01
#define GP__SCP__SCP01_RMAC							    0 		 // =1 : support RMAC in SCP01
#define GP__SCP__SCP02								    1        // =1 : support SCP02
#define GP__SCP__SCP02_RMAC							    1		 // =1 : support RMAC in SCP02
#define GP__SCP__SCP03_RMAC							    1		 // =1 : support RMAC in SCP03
#define GP__SCP__SCP10__FOR_MASK            			0        // =1 : support SCP10
#define GP__SCP__SCP11__FOR_MASK                    0        // =1 : support SCP11

#if GP__SCP__SCP11__FOR_MASK
#define GP__SCP__SCP11__A	                                0        // =1 : support SCP11A
#define GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP11     1
#define GP__SCP__SCP11__C_AUTHORIZATIONS                    0        // =1: support Authorizations under SCP11c; else not support
#define GP__SCP__SCP11__CERT_IN_GLOBAL_RAM                  0        // =1: support cert in global RAM; else cert in SSD'S DTR
#else
#define GP__SCP__SCP11__A	                              0
#define GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP11   0
#define GP__SCP__SCP11__C_AUTHORIZATIONS                  0
#define GP__SCP__SCP11__CERT_IN_GLOBAL_RAM                0        
#endif
#define GP__AMENDMENT__A__FOR_MASK					0        // =1 : support Amendment A
#define	GP__AMENDMENT__B__FOR_MASK					1        // =1 : support Amendment B,  代码量约 25K
#define GP__AMENDMENT__C__FOR_MASK                 	0        // =1 : support Amendment C
#define GP__AMENDMENT__D							0        // =1 : support Amendment D (SCP03) EUICC_M2M需要打开，代码量约为2176字节
#define GP__AMENDMENT__H__FOR_MASK                 	0        // =1 : support Amendment H

#if GP__AMENDMENT__B__FOR_MASK
#define GP__AMENDMENT__B__createSCP81Connection_FOR_MASK   1 // =1 : support uicc.connection.createSCP81Connection
#define GP__AMENDMENT__B__requestHTTPAdministrationSession 1 // =1 : support org.globalplatform.HTTPAdministration
#define GP__AMENDMENT__B__SUPOORT_TWO_SESSIONS             1 // =1 : support
#define GP__AMENDMENT__B__SUPPORT_SESSION_RETRY            1 // =1 : openchannel fail and session breakdown will go into retey policy; else do not support retry policy
#define GP__AMENDMENT__B__POST_WITH_SPACE_AND_CRLF         0 // =1 : support post with space and CRLF
#define GP__AMENDMENT__B__TLS_SessionKey_IN_FILE           0 // =1 : TLS SessionKey in EF_TLSKEY if ram is not enough; else TLS SessionKey in ram
#define GP__AMENDMENT__B__TLS_MasterKey_IN_FILE            0 // =1 : TLS MasterKey in EF_TLSKEY if ram is not enough; else TLS MasterKey in ram
#define GP__AMENDMENT__B__HAND_SHAKE_MSG_IN_FILE           0 // =1 : HandShakeMsg in EF_HTTPSCommandData if ram is not enough; else HandShakeMsg in ram
#define GP__AMENDMENT__B__DNS_Resolution                   1 // =1 : support DNS
#else
#define GP__AMENDMENT__B__createSCP81Connection_FOR_MASK   0
#define GP__AMENDMENT__B__requestHTTPAdministrationSession 0
#define GP__AMENDMENT__B__SUPOORT_TWO_SESSIONS             0
#define GP__AMENDMENT__B__SUPPORT_SESSION_RETRY            0
#define GP__AMENDMENT__B__POST_WITH_SPACE_AND_CRLF         0
#define GP__AMENDMENT__B__TLS_SessionKey_IN_FILE           0
#define GP__AMENDMENT__B__TLS_MasterKey_IN_FILE            0
#define GP__AMENDMENT__B__HAND_SHAKE_MSG_IN_FILE           0
#define GP__AMENDMENT__B__DNS_Resolution                  0
#endif

#define TIMERMODULE                                        1 // =1 : support timer

#if TIMERMODULE
#define TIMERMODULE_TIMERID_IN_RAM                         1 // =1 : clear timer id when reset
#define TIMERMODULE_TIMERID_IN_FILE                        0 // =1 : clear timer id when uicc.framework.releaseTimer executed
#else
#define TIMERMODULE_TIMERID_IN_RAM                         0
#define TIMERMODULE_TIMERID_IN_FILE                        0
#endif

///brief: config for CVM
#define GP__CVM__FOR_MASK                           1        // =1 : support CVM function

/*
 * GP API Version: defauLt version is GP API 1.5
 */
#define GP__GP_API__FOR_V1_5_MASK                   0        // =1 : support GP API V1.5
#define GP__GP_API__FOR_V1_6_MASK                   1        // =1 : support GP API V1.6
#define GP__GP_API__FOR_V1_7_MASK                   0       // =1 : support GP API V1.7, 与JAVA层GP__GP_API__FOR_V1_7_MASK保持一致

#define	GP__NATIVE_METHOD__FOR_LTE       			      0        // =1 : support LTE private native function
#define GP__RAM_INIT__FOR_LTE						            0        // =1 : support LTE parameter initialize



#define	GP__VERSION__FOR_MINI_MASK					        0        // =1 : GP Optimize version

//	国内运营商配置
#define	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK	  0        // =1 : support China Mobile SWP 1.0 requirement
#define GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK   0        // =1 : support China Mobile SWP 2.0 requirement
#define GP__VERSION__FOR_CHINA_TELECOM_MASK         0        // =1 : support China TeleCom SWP requirement
#define	GP__VERSION__FOR_CHINA_UNICOM_MASK			    0        // =1 : support China Unicom SWPrequirem
#define GP__VERSION__FOR_IRAN                       0


#if	LTE__OTA
#define  JCVM__CONFIGDATA__LTE_OTA					1
#else
#define  JCVM__CONFIGDATA__LTE_OTA					0
#endif

#define     UICC__GUOWANG__AUTH_SPECIAL_CONFIG      0       //国网鉴权特殊需求

#define    EUICC_FUNC_eSE_INTERFACE                 1  //eSE 接口


#define  PROFILE_MAX_NUMBER                    0x0A  //卡片最大支持的profile的数目, 由于标记disable操作时使用PROFILE_MAX_NUMBER+1，所以该宏定义最大支持的个数0xFD

#define WITH_JC305                            0

#if WITH_JC305
#define SENSITIVE_RESULT_SUPPORT            1
#define SENSITIVE_ARRAY_SUPPORT             1
#define VM_WITH_DH                          1
#endif

#define RSP_NewFunction                  0   //add by xieyong@20220914 SGP22V2.4新增的tag先用宏隔开

#endif


