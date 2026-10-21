/*****************************************************************************
 * @(#)mematom.c	1.20 03/08/13
 * Version 1.7
 *
 wang hui
 eastcompeace Technology Corporation

 History:
 Last modified on 07/07/30
 .Created
 Original authors: Alexandre Frey, Nicolas Regnault
 ****************************************************************************/
#ifndef _GLOBAL_VARIABLE_H__
#define _GLOBAL_VARIABLE_H__

/*#include "typejava.h"			   */


#include "Apdu.h"
#include "loader.h"
#include "target.h"
#include "memory.h"
#include "TableMgmntTypes.h"
#include "KeyManagerOld.h"

#include "ecpReadWrite.h"
#include "ecpOperation.h"
#include "JcvmDataConfig.h"
#include "SysConfigData.h"
#include "profile.h"
#if VM_WITH_DH
#include "Crypto_DH.h"
#endif
/***********************************************************************************
 * ATR.h
 ***********************************************************************************/
//T = 0
#define ATR_T0_DIRECT_TS							0x3B
#define ATR_T0_INVERSE_TS							0x3F

#define DEFAULT_PPS									0x01
#define DEFAULT_NO_PPS								0x00
#define DEFAULT_SLEEP_MODE							0x02
#define DEFAULT_NO_SLEEP_MODE						0x00
#define DEFAULT_T0									0x00
#define DEFAULT_T1									0x04
#define DEFAULT_DERECT_CONVENTION					0x00
#define DEFAULT_INVERSE_CONVENTION					0x08
#define DEFAULT_COLD_TCK_ABSENT						0x00
#define DEFAULT_COLD_TCK_PRESENT					0x10
#define DEFAULT_WARM_TCK_ABSENT						0x00
#define DEFAULT_WARM_TCK_PRESENT					0x20		  
#define GOTOPATCH_LENGTH								0x05
#define ATR_HISTORY_BYTE_OFF 			        		0x04

#define REGISTRY_UPDATA_ENABLE						0XFF
#define REGISTRY_UPDATA_DISABLE						0x55




#define REMOTE_APDU_RES_OFF_OFF						0x53

#define ROCESS_LEVEL_OTA    						0x02
// global variables


/****************************************************************************************************************************
***************************************************used for stack************************************************************
*****************************************************************************************************************************/
extern PX8		 									D_A 						JSP;
extern PX8 											D_A 						FP;
extern PX8											D_A							FPend;
extern u8											D_A 						FPsize;
extern PX8		 									D_A 						Top;
extern u32											X_A							VMStack[];//modify by jiangxiaohui@20220324 分层
extern u8											I_A							CardSecurityState;
/****************************************************************************************************************************/

extern PX8 										D_A 		BackTop;
extern PX8 										D_A 		BackFP;
/****************************************************************************************************************************
***************************************************used for VM***************************************************************
*****************************************************************************************************************************/ 
extern memref 										X_A 						cachedClassCompLoc;
extern memref 										X_A 						cachedMethodCompLoc;
extern memref 										X_A 						cachedFieldCompLoc;
extern PF8		 									D_A 						JPC;
extern u8 											I_A 						PCpkgID;
extern u16 											I_A 						cc;
extern u8 											D_A 						localsize;
extern u8 											D_A 						paramsize;
extern u8 											I_A 						selected_channel;
extern u8 											I_A 						lastVMPkg;
extern jref 										X_A 						tempReferenceIDGRT;
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for temp_ob_manager**************************************************
*****************************************************************************************************************************/
extern jref 										X_A 						returnedTempArrayId;
extern u8 											X_A 						noOfTemporaryObjects;
/****************************************************************************************************************************/


/****************************************************************************************************************************
***************************************************used for sd key management************************************************
*****************************************************************************************************************************/ 
extern u8 											X_A							g_bSDId[MAX_LOGICAL_CHANNELS];
extern u8											X_A							g_bISDId[MAX_LOGICAL_CHANNELS];
extern u8											X_A							g_bIsdCount;
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for opFunction*******************************************************
*****************************************************************************************************************************/
extern u8 											X_A							currKeySetVer[];
extern u8 											X_A							sesKey[16];
extern u8 											X_A							macKey[16];
extern u8 											X_A							deKey[16];
extern u8 											X_A							lastMAC[8];
extern u8 											X_A							cardChallenge[];
extern u8 											X_A							hostChallenge[8];
extern u8 											X_A							TEMPBuffer[];
//#define 											ATSBuffer  					TEMPBuffer
#define MAX_DATA_NUM					8

/****************************************************************************************************************************/	

/****************************************************************************************************************************
***************************************************used for loader***********************************************************
*****************************************************************************************************************************/
extern u8 											X_A							g_LoaderState;
extern u8 											X_A							g_previous_cap_tag;
extern u8 											X_A							g_cap_tag;
extern u8 											X_A							g_capFlags;
extern u16 											X_A							g_dataSize;
extern u16 											X_A							g_dataOffset;
extern u16 											X_A							t_Component_length;
extern u16 											I_A							g_offset;
extern u16 											I_A							g_max;
extern u32 											I_A							g_componentSize;
extern u8 											I_A      					g_lastBlock;
extern u8 											I_A							g_capMinor;
extern u8 											I_A							g_capMajor;
extern u8 											I_A							g_pkgMinor;
extern u8 											I_A							g_pkgMajor;
extern u8 											I_A							g_savedBytesCount;
extern u8 											X_A							g_savedBytes[3];
extern u1											B_A							g_more;
extern u1											B_A							g_bytesLeft;
extern u1											B_A							g_needCheckHash;
extern u1											B_A							g_isResolveStaticField;
extern u16											X_A							g_loadComplete;
extern u8 											X_A							g_appletAID[16];
extern u8 											X_A							f_pkgIDs[16];
extern u16 											X_A							g_staticReferenceCount;
extern u16 											X_A							class_component_Size;
extern u16 											X_A							method_component_Size;
extern u16 											X_A							staticfield_component_Size;
extern u16 											X_A							export_component_Size;
extern u16 											X_A							constantpool_component_Size;
extern u8											X_A			                CONSTANTPOOL_COMPONENT[0x100];
extern memref 										X_A							class_component_Addresses;
extern memref 										X_A							method_component_Addresses;
extern memref 										X_A							staticfield_component_Addresses;
extern memref 										X_A							export_component_Addresses;
extern memref 										X_A							constantpool_component_Addresses;
extern memref                                       X_A                         G_PkgComponentAddrStartAddr;
extern memref                                       X_A                         G_PkgComponentAddrEndAddr;

extern u16 											X_A							class_component_offset;
extern u16											X_A							method_component_offset;
extern u16 											X_A							export_component_offset;
extern u16 											X_A							constantpool_componentoffset;
extern u8 											X_A 						f_currentState;
extern u8 											X_A 						g_aidCount;
extern u8 											X_A							g_aidLength;
extern u8 											X_A 						g_aidOffset;
extern u8 											X_A 						g_icvsize;    // merged from /COS/EPX/EPX/v3.0/DEVELOP@20220722
extern u8 											X_A 						g_icvbuf[16]; // merged from /COS/EPX/EPX/v3.0/DEVELOP@20220722

/*@ -namechecks @*/
extern u8 											X_A 						m_count;
extern u8 											X_A 						m_base;
/*@ +namechecks @*/
extern u8 											X_A 						k_count;
extern u8 											X_A 						k_base;
#define 											COMPONENT_MAX  				13
#define 											g_buffer 					APDUBuffer
#define 											g_OPbTemp					class_component_Addresses
#define 											g_OPbTempDAP				method_component_Addresses
#define 											g_E2C4tag					m_count
#define 											g_E2Size					g_aidCount
#define 											g_SigDataSize				g_capMinor
#define 											g_resolvResult				k_count
#define 											g_sigDataOffset				g_aidOffset
#define 											g_C4LenOffset				g_aidOffset
#define 											g_icvoffset					g_aidOffset// merged from /COS/EPX/EPX/v3.0/DEVELOP@20220722
#define 											g_pkgNameLength				m_count
#define 											g_nameOffset 				g_aidOffset
#define 											g_pkgName					g_staticReferenceCount
#define 											g_pkgNameBuffer				class_component_Addresses
#define 											g_backupCC					export_component_Size
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for WatchExeptionISR*************************************************
*****************************************************************************************************************************/
//extern u8											X_A							patchRamEntry[48];
//extern u16											X_A							patchEntrySel;
//extern u8 											X_A 						savedA;
//extern u32 											X_A							savedEPTR;
//extern u32											X_A							patchTempAddress;
//extern 												B_A							patchEEP;
//extern 												B_A							patchServiceSel;
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for patch************************************************************
*****************************************************************************************************************************/
//extern PatchTable *									X_A							PatchTableEntry;
//extern u16 											X_A							PatchTableNumber;
//extern u8 											X_A							patchRamArea[PATCH_RAM_SIZE];
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for GC***************************************************************
*****************************************************************************************************************************/
extern u1											B_A							deletingPackage;
extern u8 											X_A							packageDeletedcontext;
extern u8											X_A							packageDeletedID;
extern u8											X_A							appletDeletion;

#define 											defferredObjectTableSize 	m_count
#define 											deferredObjectCount         m_base
#define 											deferredObjectTableOverflow k_count

#define 											deferredObjectTable         staticfield_component_Addresses
#define 											bmp                         method_component_Addresses	
//#define 											appletDeletionContexts	    class_component_Addresses 
#define 											firstBmpEnd     	        class_component_Addresses // modify by jiangxiaohui@20220804:GC  对象标记 firstBmpEnd 需要记录，后面mark会用到，firstBmpEnd 互用32位loader过程中的变量
#define 											firstBmp                    method_component_Addresses
#define 											secondBmp		  	        APDUBuffer
#define 											mapTableTag				    g_savedBytesCount
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for contentManager***************************************************
*****************************************************************************************************************************/
extern memref 										X_A 						Pkg_Tabel_Cash;
extern memref 										X_A 						Pkg_Tabel_Object_Cash;
extern memref 										X_A 						APP_Table_Cash;
extern memref 										X_A 						APP_Table_Object_Cash;
extern memref 										X_A 						APP_Entry_Object_Cash;

#define 											APPLET_PRE_STATUS          	0
#define 											APPLET_CURRENT_STATUS     	1
#define 											APPLET_PRIVELEGE           	2
#define 											APPLET_CONTEXT            	3

/****************************************************************************************************************************
***************************************************used for DataManager*************************************************************
*****************************************************************************************************************************/
extern u8	 										X_A				handle_2F00Tag[MAX_LOGICAL_CHANNELS] ;//dataManager handle 2F00 linked.
extern u8	 										X_A				handle_2F00TagLink[MAX_LOGICAL_CHANNELS];
extern u8 											X_A				g_moreCommands;
extern u8 											X_A				currKeySetVer[MAX_LOGICAL_CHANNELS];
extern u8 											X_A				moreDataFlag[MAX_LOGICAL_CHANNELS];
extern u8											X_A				g_ExtraPkgOff[MAX_LOGICAL_CHANNELS];
extern u8											X_A				g_ExtraAppOff[MAX_LOGICAL_CHANNELS];
extern u8    										X_A				g_PkgOff[MAX_LOGICAL_CHANNELS];
extern u8											X_A				g_AppOff[MAX_LOGICAL_CHANNELS];
extern u8											X_A				g_SdOff[MAX_LOGICAL_CHANNELS];
extern u16											X_A				g_CommandOffset;
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for nativemethod_impl************************************************
*****************************************************************************************************************************/
extern memref 										X_A							wFreeEESize;

//extern u8											X_A			isVGPconfig; 
//extern u8											X_A			isGPconfig221;
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for memory***********************************************************
*****************************************************************************************************************************/
extern u1											B_A							e2p_allocation_flag;
extern u32             								X_A      					g_maxEntryAdd;
extern u32             								X_A      					g_usedBlockAdd;
//extern PX8											X_A			 				r_memEepromBuffer;
extern u16 											X_A							dtrSize;
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for map table bit map************************************************
*****************************************************************************************************************************/
extern u1											B_A 						isFirstPage;
extern u8  											X_A  						compactTimer;
extern u32											X_A							NVM_FREE_TABLE_START;
#define  											bitMapCount   				g_offset
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for logical channel**************************************************
*****************************************************************************************************************************/
extern u8  											X_A							logicalChannelNums;
extern u8	  		 								X_A							protocol_type;
/****************************************************************************************************************************/



/****************************************************************************************************************************
***************************************************used for testmode************************************************************
*****************************************************************************************************************************/
//#define 											SetBank0Base  				class_component_Addresses
/****************************************************************************************************************************/


/****************************************************************************************************************************
***************************************************used for protocol parameter***********************************************
*****************************************************************************************************************************/
#define 											ourPow(pow) 				U16_SHL_U8(2, U8_S_U8(pow, 1))
/****************************************************************************************************************************/


		
/****************************************************************************************************************************
***************************************************used for apdu_native******************************************************
*****************************************************************************************************************************/
#if HAL__PLATFORM__SIMULATOR
extern PX8		 									I_A 						bRamBuffer;
extern PX8		 									I_A 						APDUBuffer;
extern PX8		 									I_A 						cacheBuffer; // initCacheBuffer
extern PX16											I_A	 						bShortBuffer;
//extern PX8											X_A							bTempArr;
#else
extern u8		X_A					DtTrRamAlign[/*DT_RT_RAM_ALIGN_DWORD*/];
//extern u8		X_A					g_PBOC_instRAMBuff[/*RTR_TOP - (u16)(RAMBase) - SHORT_BUFFER_SIZE * 2 - RAM_BUFFER_SIZE - APDU_BUFFER_SIZE - DT_RT_RAM_ALIGN_DWORD*/];
extern u16		X_A					bShortBuffer[/*SHORT_BUFFER_SIZE*/];
extern u8		X_A					bRamBuffer[/*RAM_BUFFER_SIZE*/];
extern u8		X_A					APDUBuffer[/*APDU_BUFFER_SIZE*/];
extern u8		X_A					cacheBuffer[/*APDU_BUFFER_SIZE*/];
//extern u8		X_A					javaExceStaticFeild[/*RTR_BASE - RTR_TOP*/];
#endif


/****************************************************************************************************************************/


/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for HMAC********************************************************
*****************************************************************************************************************************/
#if VM_WITH_HMAC
	extern u8 										X_A							g_HmacDigestSize;
	extern u8 										X_A							g_HmacBlockSize;
	extern u8 										X_A							g_HashType;
	extern u8 										X_A							g_HmackKeyLen;
	extern PR8										X_A							g_HmacKey;
	extern PR8										X_A							g_HmacComputeKey;
#endif
/****************************************************************************************************************************/

/****************************************************************************************************************************/
#if VM_WITH_DH
extern DHKeyRefStore g_DHKeyStore;
extern const u8 DHdefaultG[1];
#endif
/****************************************************************************************************************************
***************************************************used for timer interrupt**************************************************
*****************************************************************************************************************************/
extern u8 											X_A							InterruptRegisterPro[13];
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for pboc*************************************************************
*****************************************************************************************************************************/
//extern												B_A							isPBOCselected;
//extern u8											X_A					g_AppType;
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for timer opfunction*************************************************
*****************************************************************************************************************************/
extern u8											CONST						rData1[3]; 				
extern u8											CONST						rData2[4];				
//extern u8											CONST						defaultKey[16];
extern u8											CONST						defaultKey_32Bytes[32];
extern u8											CONST						cardIndentify[16];		
extern u8											CONST 						defaultProtocolConfig;
/****************************************************************************************************************************/

//CGM统计系统消耗的配置开关，00为系统消耗由SSD本身扣除，01为系统消耗由ISD扣除
//#define												CGM_SYSTEM_CONSUME_CONFIG	(OS_SECOND_ATR + 0x20)
extern u16											X_A							g_CCInLoadOrInst;

extern u16											X_A							g_keepRSystem;
extern u16											X_A							g_keepRInstall;
extern memref										X_A							g_keepNInstall;
extern memref										X_A						    g_keepNSystem;

//#define	 g_CCInLoadOrInst					   g_staticReferenceCount
/*#define	 g_keepNSystem	                       staticfield_component_Addresses
#define  g_keepRSystem						   export_component_offset
#define	 g_keepNInstall						   constantpool_component_Addresses
#define	 g_keepRInstall						   constantpool_componentoffset*/
/****************************************************************************************************************************
***************************************************used for timer loader*****************************************************
*****************************************************************************************************************************/
    extern u8											CONST						bLoadOrderArr[13];
    extern ComponentEntry								CONST 		 				f_linkers[13];
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************used for brambuffer, should is same with define in Constants.java*********************************
*****************************************************************************************************************************/
#define SELECTING_APPLET_FLAG      	     		0
#define PROCESS_METHOD_FLAG           	     	1
#define DAP_VARIFY_TIME    		     			2
#define SET_INSTALL_PARA			     		3	//only used in insatllforinstall
#define	HASH_VERIFY_FLAG						3	//only used in installforload and load
#define SELECT_STATE							3	//only used in select
#define MANDATE_SD_ID      		     			4
#define INSTALLER_STATE_INDEX                	5
#define CURRENT_SECURITY_LOGICAL_NUMBER     	6
#define ISD_SECURITY_LEVEL            	     	7
#define SD_REQUESTED_SECURITY_LEVEL          	8
#define BLOCK_NUMBER        	    	     	9
#define INSTALL_FOR_LOAD    	    	     	10
#define GC_Requested    	    	     		11

#define INSTALL_APPLET_SDID              		12
#define INSTALL_APPLET_PRIV              		13
/*
#define INSTALL_APPLET_EXTRADTIONFLAG    		14
#define APP_IN_PROCESS                     		15
#define INSTALLTRANSACTIONFLAG           		16

#define TEMP_ARRAY_OFF    	    	     		17
#define LOAD_PKG_LEN_OFF    	    	     	17// THIS  used as a temparray
#define LOAD_PKG_AID_OFF    	    	     	18// THIS  used as a temparray
#define HASHBUFER_OFF    	    	     		37// THIS  used STORE hash block when install for load
*/

#define  INSTALL_APPLET_EXTRADTIONFLAG            (INSTALL_APPLET_PRIV + 3)                        //16
#define  APP_IN_PROCESS                           (INSTALL_APPLET_EXTRADTIONFLAG + 1)              //17  
#define  INSTALL_TRANSACTION_FLAG                 (APP_IN_PROCESS + 1)                             //18 modify by xuxiaojun@20220330 the same name with Constants.java
#define  LOAD_PKG_LEN_OFF                         (INSTALL_TRANSACTION_FLAG + 1)                   //19
#define  LOAD_PKG_AID_OFF                         (LOAD_PKG_LEN_OFF + 1) 			               //20

#define  STORE_DATA_SEQUNENCE					  (LOAD_PKG_AID_OFF + 19)						   //39 modify by xuxiaojun@20220330 the same name with Constants.java
#define  HASHBUFER_OFF                            (STORE_DATA_SEQUNENCE + 1)                          		//40
#define  CLINTERFACE_SWITCH_OFF					  (HASHBUFER_OFF + 40)		 					   //80

#define PRE_SEL_CHANNEL							    80
#define CHECK_SCP81_AUTHED                          81
#define IS_JCRE                                     82
#define SELECTING_APPLET_INDEX      	     		83

#define PROCESS_LEVEL                               84
#define TAR											85// 3

// define for proactive handler
#define APDU_SESSION_LEVEL					  		88 //1 byte, todo: unused
#define FIRST_CMD_ON_BASIC_CHNL					    89 //1 byte, todo: unused
#define REMOTE_SCP_TYPE     						90 //1 byte, todo: used for uicc
#define REMOTE_SCP_KEY_VERSION         				91 //1 byte, todo: used for uicc
#define REMOTE_SCP_SDID          					92 //1 byte, todo: used for uicc
#define INSTALL_SD_NOT_AUTO_PERSONALIZED			93 
#define PIN_CONVERT_OFF							  	94  //8 bytes,todo: unused
#define INSTALL_FOR_PERSONERLIZE                	103 //used in Java lagyer for storing app id when install [for personerlize]. 按顺序排
#define INSTALL_FOR_LOAD_BY_C2    					104 //1 byte, unused
#define COMMAND_HEAD_RECEIVED						105 //1 byte, unused
#define IS_RESTORE									106
#define Old_Channel                             	107 //1 byte, unused
#define PUTRSATOKENA1                           	108 //1 byte, unused
#define GET_STATUS_APPAID                       	109 //16 bytes, unused

//由于GET_STATUS_APPAID未使用，考虑复用于缓存 REGISTRY_UPDATE_CGM_NVM_FLAG 
#define REGISTRY_UPDATE_CGM_NVM_FLAG               (GET_STATUS_APPAID)	   //109 1byte
#define REGISTRY_UPDATE_CGM_NVM_VALUE              (REGISTRY_UPDATE_CGM_NVM_FLAG + 1)      //110 4byte

#define REGISTRY_UPDATE_CGM_RAM_FLAG               (REGISTRY_UPDATE_CGM_NVM_VALUE + 4)     //114 1byte，复用，用于临时保存注册表更新时的CGM值
#define REGISTRY_UPDATE_CGM_RAM_VALUE              REGISTRY_UPDATE_CGM_RAM_FLAG + 1        //115 4byte

#define INSTALL_FOR_LOAD_TO_SDID                	125
#define CONTANTLESS_DO_RESET                    	126  //1 byte, unused  该变量在C层设置值，再到Java层判断做响应处理后清除
#define TagExist  			                    	127
#define OFF_APID  			                  		128 //8 bytes
#define LIFECYCLETRANSITION							136
#define MAXSYMMETRICKEYCNT							137 //1 byte, unused 
#define AUTHFAILCNT									138 //1 byte, unused
#define MAX_LOGICAL_CHANNEL_NUMBER        			139  //最大逻辑通道个数, todo: what is the different from MAX_LOGICAL_CHANNELS in bootConfigProfile.h ?
#define INSTALLFORPERSON_SELECT_CHANNEL         	140
#define AMDC_VOLATILE_PRIORITY_TAG			        141 //1 byte, used for GP__AMENDMENT__C__FOR_MASK

//	TODO:复用hash buffer，如果空间不够用，需要考虑申请另外的空间来存放
#define  OTHER_SEARCH_CRITERIA_LENGTH             (AMDC_VOLATILE_PRIORITY_TAG + 1)//142, 1 byte, used for get status
#define  OTHER_SEARCH_CRITERIA_OFF                (OTHER_SEARCH_CRITERIA_LENGTH + 1)//143, 30 bytes, used for get status, 使用时没有对边界进行限制，可能会越界
#define  TAGLIST_LENGTH                           (OTHER_SEARCH_CRITERIA_OFF + 30)//173, 1 byte
#define  TAGLIST_OFF                              (TAGLIST_LENGTH + 1)//174, 1 byte

#define  TEMP_ARRAY_OFF                         180 //8 bytes, used for global pin, 使用时没有对边界进行限制，可能会越界

#define IS_LOAD_IN_PROGRESS	                     188 //1 byte
#define IS_UICC_JUMP_OFF	                     189 //1 byte, used for uicc
#define  LOADING_PROFILE_INDEX                   190 //1 byte, 正在下载的profileIndex
#define  PROFILE_APDU_MODE                       191 //1 byte, PROFILE下载过程中的下载 安装指令 不需要算mac 校验安全级别等
#define  PROFILE_INSTALL_MNOSD_FLAG              192 //1 byte, 表示当前正在安装MNOSD
#define  SD_TAR_INFO_OFF						 193 //12 bytes  ,用于安装过程中缓存SD 的TAR信息 0x0c字节

#define  GP_GETSTATUS_P1                        205 //1 byte, get stutus指令的P1值备份
#define  RF_FIRST_SELECT_CMD_FLAG                206 //1 byte, 表示非7816接口第一条选择命令是否已接收到

//#define  SWP_HCI_SERVICE_ID                     207 //1 byte
//#define  SWP_HCI_EVENT_TYPE                       208 //1 byte
//#define  SWP_HCI_CALLBACK_RESULT                      209 //1 byte

#define  RAM_BUFFER_END                        212 // 大小设置为WORD的整数倍
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for bShortBuffer*****************************************************
*****************************************************************************************************************************/
#define INSTALLER_EXCEPTION 					0
#define VM_EXCEPTION     					    1
#define LOAD_C4_VALUE_H							2 // C4总长度  高字节
#define LOAD_C4_VALUE_L        					3 // C4总长度  低字节
#define STATUS_FLAG_OFF        					4 // use MAX_LOGICAL_CHANNELS elements, one element for every logical channel

#if JAVA_CARD_UICC
#define REMOTE_APDU_RES_LEN_OFF    				(STATUS_FLAG_OFF + MAX_LOGICAL_CHANNELS) //yangfajun，8个逻辑通道各占一个字节MAX_LOGICAL_CHANNELS
#define CATRE_ERH_POST_LEN_OFF					(REMOTE_APDU_RES_LEN_OFF + 1)
#define REMOTE_PKT_CMD_LEN_OFF    				(CATRE_ERH_POST_LEN_OFF + 1)
#define REMOTE_PKT_RES_LEN_OFF					(REMOTE_PKT_CMD_LEN_OFF + 1)
#define REMOTE_PROCESS_RES_LEN					(REMOTE_PKT_RES_LEN_OFF + 1)
#define SHORT_BUFFER_END                        (REMOTE_PROCESS_RES_LEN + 1)
#else
#define SHORT_BUFFER_END                       (STATUS_FLAG_OFF + MAX_LOGICAL_CHANNELS)
#endif

/****************************************************************************************************************************/
		

/****************************************************************************************************************************
***************************************************used for SCP*****************************************************
*****************************************************************************************************************************/

#define CARD_CHALLENGE_LENGTH						0x08
#define HOST_CHALLENGE_LENGTH						0x08
#define CARD_CHALLENGE_LENGTH_OFF					0x03
#define HOST_CHALLENGE_LENGTH_OFF					0x03
#define RMAC_TEMP_LENGTH							0x08
#define MAC_LENGTH									0x08

extern u8 		X_A							g_SCP_Type[MAX_LOGICAL_CHANNELS];
extern u8 		X_A							g_SCP_I_Option[MAX_LOGICAL_CHANNELS];

#if GP__AMENDMENT__D
	#define SESSION_KEY_LENGTH 					0x20  // scp02/scp03 session key的最大长度
	#define SESSION_KEY_LENGTH_HALF				0x10
	#define SESSION_KEY_OFFSET 					0x05
#else
	#define SESSION_KEY_LENGTH 					0x10
	#define SESSION_KEY_LENGTH_HALF				0x08
	#define SESSION_KEY_OFFSET 					0x04
#endif

#define 		LAST_MAC_LENGTH				0x10
#define 		LAST_MAC_LENGTH_HALF		0x08
#define 		LAST_MAC_LENGTH_OFFSET		0x04

#define			CURRENT_MAC_LENGTH			0x08
#define			CURRENT_MAC_LENGTH_OFFSET	0x03

extern u8 		X_A						g_SCP_rmacTemp[RMAC_TEMP_LENGTH];
extern u16 		X_A						g_SCP_seqCounter[MAX_LOGICAL_CHANNELS];

#if GP__AMENDMENT__D
extern u8 		X_A 					SCP_sesKeyLen[MAX_LOGICAL_CHANNELS];  // g_current_SCP_SES_KEY对应的长度, SCP03协议中会话密钥长度可能为0x10,0x18,0x20。 测试用例704 GP UICC2 Compliance203：sm_APDU_initializeUpdate_success_secureChannelSessionAlreadyOpened_otherLc_SSD_scp03_24B_de_3b_58
extern u8 		X_A 					SCP_rencKeyLen[MAX_LOGICAL_CHANNELS];
extern u8 		X_A 					SCP_dekKeyLen[MAX_LOGICAL_CHANNELS];  // g_current_SCP_DEK_KEY对应的长度
extern u8 		X_A 					SCP_rdekKeyLen[MAX_LOGICAL_CHANNELS];
extern u8 		X_A 					SCP_macKeyLen[MAX_LOGICAL_CHANNELS];  // g_current_SCP_MAC_KEY对应的长度
extern u8 		X_A 					SCP_rmacKeyLen[MAX_LOGICAL_CHANNELS]; // g_current_SCP_RMAC_KEY对应的长度,目前来看，g_SCP_rmacKeyLen的值与g_SCP_macKeyLen的值一致
#define 		g_SCP_sesKeyLen         SCP_sesKeyLen[selected_channel] // g_current_SCP_SES_KEY对应的长度
#define			g_SCP_rencKeyLen        SCP_rencKeyLen[selected_channel]
#define			g_SCP_dekKeyLen         SCP_dekKeyLen[selected_channel]   // g_current_SCP_DEK_KEY对应的长度
#define 		g_SCP_rdekKeyLen        SCP_rdekKeyLen[selected_channel]
#define 		g_SCP_macKeyLen         SCP_macKeyLen[selected_channel]// g_current_SCP_MAC_KEY对应的长度
#define 		g_SCP_rmacKeyLen        SCP_rmacKeyLen[selected_channel]// g_current_SCP_RMAC_KEY对应的长度,目前来看，g_SCP_rmacKeyLen的值与g_SCP_macKeyLen的值一致
																		//remove warning,dengyiwen,2022/9/27 数组索引不能等于MAX_LOGICAL_CHANNELS将 MAX_LOGICAL_CHANNELS改成selected_channel，与上面定义保持一致，是当前选择的逻辑通道上的数据。
#else
extern u8 		X_A 					g_SCP_sesKeyLen;  // g_current_SCP_SES_KEY对应的长度
extern u8 		X_A 					g_SCP_rencKeyLen;
extern u8 		X_A 					g_SCP_dekKeyLen;  // g_current_SCP_DEK_KEY对应的长度
extern u8 		X_A 					g_SCP_rdekKeyLen;
extern u8 		X_A 					g_SCP_macKeyLen;  // g_current_SCP_MAC_KEY对应的长度
extern u8 		X_A 					g_SCP_rmacKeyLen; // g_current_SCP_RMAC_KEY对应的长度,目前来看，g_SCP_rmacKeyLen的值与g_SCP_macKeyLen的值一致
#endif

extern u8       X_A                     g_SCP_SCP03t_Flag;//标识是否是scp03t ，是的话在8482里面需要将数据源转成TLV格式

#define SCP_KEY_LENGTH					((SESSION_KEY_LENGTH * MAX_LOGICAL_CHANNELS))
#define SCP_CARDCHALLENGE_LENGTH		(CARD_CHALLENGE_LENGTH * MAX_LOGICAL_CHANNELS)
#define SCP_HOSTCHALLENGE_LENGTH		(HOST_CHALLENGE_LENGTH * MAX_LOGICAL_CHANNELS)
#define SCP_LASTMAC_LENGTH				(LAST_MAC_LENGTH * MAX_LOGICAL_CHANNELS)
#define SCP_CURRENTMAC_LENGTH			(MAC_LENGTH * MAX_LOGICAL_CHANNELS)
#define SCP_BACKMAC_LENGTH				(MAC_LENGTH * MAX_LOGICAL_CHANNELS)

#define SCP_DATA_START      			SCP_DATA_AREA_START //U32_S_U16(NvmManager_GetTopNvm(), SCP_DATA_LENGTH)merger from EPX\v2.1\BRANCH\wuronghua\FM151M @20220301: SCP_DATA_START定义成sysConfig_GetNvmTop，而如果sct文件修改后不一定能保证SCP_DATA_START和sysConfig_GetNvmTop一定对应(缺陷系统编号365)



#define g_current_SCP_I_Option  			g_SCP_I_Option[selected_channel]
#define g_current_SCP_Type  				g_SCP_Type[selected_channel]
#define g_current_SCP_seqCounter  			g_SCP_seqCounter[selected_channel]


//#if GP__AMENDMENT__D
	#define SCP03_ICV_COUNTER_LENGTH			0x10
	#define SCP03_ICV_COUNTER_LASTOFF			0x0F
	//顺序计数器，用于计算（C-decrypt/R-decrypt）ICV
	extern u8 		X_A					g_SCP_ICV_Counter[SCP03_ICV_COUNTER_LENGTH];
//#endif

extern u8 		X_A					g_SCP_appEnc;
extern u8 		X_A					g_SCP_appREnc;
extern u8 		X_A					g_SCP_appRmac;

#if GP__SCP__SCP10__FOR_MASK
	/* The bit of sesKeyFlag indicates whether the corresponding session key is set:
	 *  b7    b6    b5    b4     b3    b2    b1    b0
	 *  rfu CR-macIV  R-enc C-enc  C-dek R-dek R-mac C-mac
	 * '0' means not; '1' means yes.
	 */
extern u8 sesKeyFlag;
#endif

extern u8 		X_A					g_SCP_BeginEnd_R_MAC[MAX_LOGICAL_CHANNELS];
extern u16 		X_A 				g_SCP_SequnceCount[MAX_LOGICAL_CHANNELS];    // only used for begin rmac command


/****************************************************************************************************************************
***************************************************used for KeyLogicManager*****************************************************
*****************************************************************************************************************************/

extern u8 											X_A			g_isSetDefaultKey;
extern u16											X_A			g_EccKeyParaRef; //F0 的值
extern u8                                           X_A			g_EccKeyParaLen; //B2-B7的长度
extern u8                                           X_A         g_PutEccKey;     //当前的put key指令在执行put ecc key
extern u8											X_A			g_markEccKey;//bit0 - F0, bit 1 -B0/B1, bit2~bit7 - B2~B7
extern u8											X_A			g_storeEccKeyMode;
extern u8											X_A			g_storeDGIKeyFlag;
extern u8											X_A			g_keyDataOffest;
extern u8											X_A			g_keyUsageValueOffest;
extern u8    										X_A			g_keyAccessValueOffest;
extern u8											X_A			g_PutKeyId;
extern u8											X_A			g_isNewKey;
extern u8    										X_A			g_putKeyCommands;
extern u16											X_A			g_PutKeyRemaindLen;
extern u16											X_A			g_PutkeyDataLen;
extern u16											X_A			g_KeepRefForSplit;
extern u16											X_A			g_putKeyCurrState;
extern u16											X_A			g_KeepLenForUsageOrAccess;
#endif

