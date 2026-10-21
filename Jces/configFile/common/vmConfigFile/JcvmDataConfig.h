/****************************************************
* Copyright (C), 2008-2015, Eastcompeace. Co., Ltd. 
* this code is auto generate by config gen tool
* build time: 2024-06-24 18:25:32
* build by:   liguiyu
******************************************************/ 
#ifndef JCVM_DATA_CONFIG_H_
#define JCVM_DATA_CONFIG_H_
#include "ecpDefine.h"
#include "memory.h"
#include "SysConfig.h"
#include "profile.h"

/*****************************************************************************
 variable in E2p . value can  be modified
 ****************************************************************************/

#define    VM_JC_CONFIG									1
#define    VM_GOP_CONFIG								1
#define    APP_PBOC_OR_EDEP_CONFIG						1
#define    APP_PBOC_CONFIG								1
#define    APP_EDEP_CONFIG								1
#define    APP_JT_FUNC_CONFIG							1
#define    APP_EID_CONFIG								1
#define    APP_DCEP_CONFIG                          	0
#define    APP_DCEP_ZZH_CONFIG							0
#define    APP_SELECT_BY_FILE_ID_CONFIG					0
#define    APP_ISDR_SPECIAL_CONFIG					    1
#define    APP_AMENDMENT_B_SPECIAL_CONFIG               1
#define    APP_UICC_FUNCTION                            1
#define    APP_BYD_CONFIG                               0
#define    VM_RESERVE_CONFIG							1
#define    VM_CARDINIT_CONFIG							1

#if CONFIG_IN_BUFFER
	extern u8 CONST vm_config[];
#endif


#define     VM_CONFIG_OFFSET_START          (0x00000200L)

                                               //EEPROM Address  tag  Length  default_Value
#if  VM_JC_CONFIG
#define    VM_OS_CONF                            0x0200        //FFFF   0x01   11
#define    VM_OS_LOGICAL_CHANNEL_NUMBER          0x0201        //1001   0x01   04
#define    VM_OS_DTR_SIZE                        0x0202        //FFFF   0x02   0400
#define    VM_WTX_RESPONSE_ERROR_CODE            0x0204        //FFFF   0x02   0000
#define    VM_RECEIVE_ERROR_CODE                 0x0206        //FFFF   0x02   0000
#define    VM_INT_ERROR_CODE                     0x0208        //FFFF   0x02   0000
#define    VM_HALT_SESSION_CODE                  0x020a        //FFFF   0x02   0000
#define    VM_E2P_CONFIG_MAP_BIT_TBL_SEGMENT     0x020c        //FFFF   0x01   1C
#define    VM_E2P_CONFIG_FREE_TBL_ENTRY          0x020d        //FFFF   0x01   48
#define    VM_E2P_CONFIG_RFU                     0x020e        //FFFF   0x01   01
#define    VM_CONFIG                             0x020f        //FFFF   0x01   12

#define    VM_JC_CONFIG_LENGTH                   0x0010
#endif

#if  VM_GOP_CONFIG
#define    GP_CONFIG                             0x0210        //FFFF   0x01   00
#define    GP_SECOND_CONFIG                      0x0211        //FFFF   0x01   09
#define    GP_THIRD_CONFIG                       0x0212        //FFFF   0x01   01
#define    CM_SCP_TYPE                           0x0213        //200C   0x01   02
#define    CM_AUTH_MAX_TRY_CNTR                  0x0214        //2008   0x01   0A
#define    CM_AID_LENGTH                         0x0215        //2009   0x01   08
#define    CM_AID                                0x0216        //200A   0x10   A000000003000000FFFFFFFFFFFFFFFF
#define    CM_LAST_BASIC_PACKAGE                 0x0226        //200E   0x02   000F
#define    CM_AppTableIndex                      0x0228        //FFFF   0x01   00
#define    CM_FileAid                            0x0229        //FFFF   0x02   2F00
#define    CM_SD_RSA_TOKEN_BYTE_LEN              0x022b        //200F   0x02   0080
#define    CPLC_DATA_CONF                        0x022d        //2010   0x30   FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
#define    VM_CM_AUTH_TRY_CNTR                   0x025d        //FFFF   0x01   00
#define    VM_CVM_PIN                            0x025e        //2001   0x0c   0303020312345FFFFFFFFFFF
#define    VM_KEYSETS                            0x026a        //2007   0x34   0000FF07404142434445464748494A4B4C4D4E4F404142434445464748494A4B4C4D4E4F404142434445464748494A4B4C4D4E4F
#define    VM_TOKEN_KEY_VERSION                  0x029e        //2012   0x01   70
#define    VM_TOKEN_KEY_ID                       0x029f        //2013   0x01   01
#define    VM_RECEIPT_KEY_VERSION                0x02a0        //2014   0x01   71
#define    VM_RECEIPT_KEY_ID                     0x02a1        //2015   0x01   01
#define    VM_DAP_KEY_VERSION                    0x02a2        //2016   0x01   73
#define    VM_DAP_KEY_ID                         0x02a3        //FFFF   0x01   01
#define    VM_SCP_DEFAULT_KEY_VERSION            0x02a4        //2006   0x01   FF
#define    VM_MIN_SCP_KEY_VERSION                0x02a5        //2018   0x01   01
#define    VM_MAX_SCP_KEY_VERSION                0x02a6        //2019   0x01   6F
#define    VM_MIN_SCP_KEY_ID                     0x02a7        //201A   0x01   01
#define    VM_MAX_SCP_KEY_ID                     0x02a8        //201B   0x01   03
#define    VM_SCP_KEY_ID_START_INDEX             0x02a9        //201C   0x01   01
#define    VM_ISD_COUNT                          0x02aa        //2011   0x01   01
#define    VM_DEFAULT_APP_ID                     0x02ab        //201D   0x01   01
#define    VM_KEYSETSLOADED                      0x02ac        //FFFF   0x01   00
#define    VM_REGISTRY_UPDATEENABLE              0x02ad        //FFFF   0x01   00

#define    VM_GOP_CONFIG_LENGTH                  0x009e
#endif

#if  APP_PBOC_OR_EDEP_CONFIG
#define    ECP_PBOC_EDEP_COMMON_AREA             0x02ae        //FFFF   0x06   000000000000
#define    ECP_PSE_INSTANCE_DIR_NUM              0x02b4        //FFFF   0x01   05

#define    APP_PBOC_OR_EDEP_CONFIG_LENGTH        0x0007
#endif

#if  APP_PBOC_CONFIG
#define    ECP_PBOC_INSTANCE_NVM_SIZE            0x02b5        //4001   0x02   2100
#define    ECP_PBOC_INSTANCE_RAM_SIZE            0x02b7        //4003   0x02   02E0
#define    ECP_PBOC_INSTANCE_DIR_NUM             0x02b9        //4004   0x01   23
#define    ECP_PBOC_DEFAULT_CONFIG_AREA          0x02ba        //FFFF   0x0f   841360000000000000000100000000
#define    ECP_PPE_INSTANCE_NVM_SIZE             0x02c9        //4002   0x02   0400

#define    APP_PBOC_CONFIG_LENGTH                0x0016
#endif

#if  APP_EDEP_CONFIG
#define    ECP_EDEP_MF_KEY_VALUE                 0x02cb        //FFFF   0x10   00112233445566778899aabbccddeeff
#define    ECP_EDEP_INSTANCE_NVM_SIZE            0x02db        //4006   0x02   2000
#define    ECP_EDEP_INSTANCE_DIR_NUM             0x02dd        //4007   0x01   24
#define    ECP_EDEP_USER_CFG_PRODUCT_TYPE        0x02de        //FFFF   0x01   00
#define    ECP_EDEP_USER_CFG_PRODUCT_TYPE_EX     0x02df        //FFFF   0x01   00
#define    ECP_EDEP_USER_CFG_XA_WALLET_ADVANCE   0x02e0        //FFFF   0x01   00
#define    ECP_EDEP_USER_TFT_VERSION             0x02e1        //FFFF   0x04   00000000
#define    ECP_EDEP_USER_CFG_RFU                 0x02e5        //FFFF   0x05   0000000000
#define    CONSTRUCT_AUTH_CODE                   0x02ea        //400A   0x09   000000000000000000
#define    CONSTRUCT_AUTH_SEL                    0x02f3        //4008   0x01   00
#define    CONSTRUCT_AUTH_CHIP_ADDR              0x02f4        //4009   0x04   00000000

#define    APP_EDEP_CONFIG_LENGTH                0x002d
#endif

#if  APP_JT_FUNC_CONFIG
#define    ECP_JT_ECEP_LOCK_INFO                 0x02f8        //FFFF   0x01   00
#define    ECP_JT_FILE_NVM_REF_PBOC              0x02f9        //FFFF   0x02   0000
#define    ECP_JT_FILE_NVM_REF_EDEP              0x02fb        //FFFF   0x02   0000
#define    ECP_JT_FILE_NVM_SHARE_CUR_OFFSET      0x02fd        //FFFF   0x02   0000
#define    ECP_JT_FILE_NVM_SHARE_TLV_MAX_RECORDS 0x02ff        //FFFF   0x04   00000000
#define    ECP_JT_RFU                            0x0303        //FFFF   0x05   0000000000

#define    APP_JT_FUNC_CONFIG_LENGTH             0x0010
#endif

#if  APP_EID_CONFIG
#define    ECP_EID_INSTANCE_NVM_SIZE             0x0308        //FFFF   0x02   1E00
#define    ECP_EID_INSTANCE_RAM_SIZE             0x030a        //FFFF   0x02   02C0
#define    ECP_EID_DEFAULT_CONFIG_AREA           0x030c        //FFFF   0x0c   200000000000000000000000

#define    APP_EID_CONFIG_LENGTH                 0x0010
#endif

#if  APP_DCEP_CONFIG
#define    ECP_DCEP_INSTANCE_NVM_SIZE                          //4031   0x02   7400
#define    ECP_DCEP_INSTANCE_RAM_SIZE                          //4033   0x02   0812
#define    ECP_DCEP_DEFAULT_CONFIG_AREA                        //4032   0x05   0000000000
#define    ECP_DCEP_APPLICATION_VERSION                        //4034   0x01   01
#define    ECP_DCEP_RFU                                        //FFFF   0x02   0000

#define    APP_DCEP_CONFIG_LENGTH                0x0000
#endif

#if  APP_DCEP_ZZH_CONFIG
#define    ECP_DCEP_ZZH_INSTANCE_NVM_SIZE                      //4041   0x02   2000
#define    ECP_DCEP_ZZH_INSTANCE_RAM_SIZE                      //4043   0x02   02E0
#define    ECP_DCEP_ZZH_DEFAULT_CONFIG_AREA                    //4042   0x05   0000000000
#define    ECP_DCEP_ZZH_APPLICATION_VERSION                    //4044   0x01   01
#define    ECP_DCEP_ZZH_RFU                                    //FFFF   0x02   0000

#define    APP_DCEP_ZZH_CONFIG_LENGTH            0x0000
#endif

#if  APP_SELECT_BY_FILE_ID_CONFIG
#define    FILE_ID_SYS_MAP                                     //FFFF   0xdd   All(00)

#define    APP_SELECT_BY_FILE_ID_CONFIG_LENGTH   0x0000
#endif

#if  APP_ISDR_SPECIAL_CONFIG
#define    ISDR_AIDLENGTH                        0x0318        //7001   0x01   10
#define    ISDR_AIDVALUE                         0x0319        //7002   0x10   A0000005591010FFFFFFFF8900000100

#define    APP_ISDR_SPECIAL_CONFIG_LENGTH        0x0011
#endif

#if  APP_AMENDMENT_B_SPECIAL_CONFIG
#define    TLS_EXTAND_VERSION                    0x0329        //6001   0x01   02
#define    AMENDMENT_B_CONFIG                    0x032a        //6002   0x01   1D
#define    AMENDMENT_B_Trigger_SMS_POR           0x032b        //6003   0x01   03

#define    APP_AMENDMENT_B_SPECIAL_CONFIG_LENGTH 0x0003
#endif

#if  APP_UICC_FUNCTION
#define    E_SUSPEND_TOKEN                       0x032c        //5001   0x10   All(FF)
#define    E_TERMINAL_CAPBILITY                  0x033c        //5002   0x20   All(FF)
#define    E_SUSPENDSTATUS                       0x035c        //5003   0x5c   All(FF)
#define    OPERATOR                              0x03b8        //5004   0x01   47
#define    CONFIG_EUICC                          0x03b9        //5005   0x01   FA
#define    CONFIG_UICC                           0x03ba        //5006   0x01   10
#define    E_CMCC_GETONLIESTATE                  0x03bb        //5007   0x01   FF
#define    NFC_RECORD                            0x03bc        //5009   0x01   00
#define    POLLSYSTEM_DURATION                   0x03bd        //5012   0x01   10
#define    PROCESS_PROACTIVE                     0x03be        //5013   0x01   01

#define    APP_UICC_FUNCTION_LENGTH              0x0093
#endif

#if  APP_BYD_CONFIG
#define    SEID_BYD                                            //8001   0x10   All(FF)

#define    APP_BYD_CONFIG_LENGTH                 0x0000
#endif

#if  VM_RESERVE_CONFIG
#define    DIVERSIFIED_DATA                      0x03bf        //FFFF   0x0b   All(00)
#define    TM_GUARDTIME_CB                       0x03ca        //FFFF   0x02   04FF
#define    ERROR_HANDLE                          0x03cc        //FFFF   0x01   00
#define    USAT_TEST                             0x03cd        //FFFF   0x01   00
#define    CGM_SYSTEM_CONSUME_CONFIG             0x03ce        //FFFF   0x01   00
#define    AMDC_EFCFG_FLAG                       0x03cf        //FFFF   0x01   01
#define    VM_RFU                                0x03d0        //FFFF   0x01   00
#define    USIM_PERSO                            0x03d1        //FFFF   0x01   00
#define    OTA_REMAIN_SPACE                      0x03d2        //FFFF   0x02   3C00
#define    VGP_CONFIG                            0x03d4        //3004   0x01   30
#define    CM_APPLET_LOCKED_COUNT                0x03d5        //FFFF   0x02   0000
#define    REQ_BEIJING_PERSO                     0x03d7        //FFFF   0x01   01
#define    RF_COMMUNICATION_SWITCH               0x03d8        //FFFF   0x01   01
#define    NO_CURRENT_LIMIT                      0x03d9        //FFFF   0x01   00
#define    AMENDMENT_C_TEST                      0x03da        //FFFF   0x01   00
#define    CONFIG_TEST                           0x03db        //FFFF   0x01   00
#define    SUPPORT_SSD_NESTING                   0x03dc        //FFFF   0x01   00
#define    SCP_OPTION                            0x03dd        //200D   0x01   55
#define    GETDATA_SC_SWITCH                     0x03de        //FFFF   0x01   00
#define    ISD_PRIV                              0x03df        //2020   0x03   82FD80
#define    OS_CONF                               0x03e2        //208D   0x01   5A
#define    VM_GETSTATUS_SUPPORT_SSD              0x03e3        //FFFF   0x01   01
#define    ENABLE_PRI_AUTH                       0x03e4        //C100   0x01   00
#define    PRI_AUTH_APPAID_LEN                   0x03e5        //C101   0x01   06
#define    PRI_AUTH_APPAID                       0x03e6        //C102   0x10   112233445566FFFFFFFFFFFFFFFFFFFF
#define    SSD_AUTH_COUNTER                      0x03f6        //C103   0x01   FF
#define    REQUIREMENT_DEFAULT_SCP               0x03f7        //5008   0x01   00
#define    MAPTABLE_PAGENUM                      0x03f8        //FFFF   0x01   15
#define    VM_EE_PATCH_SIZE                      0x03f9        //FFFF   0x04   All(00)
#define    CARD_OPTION                           0x03fd        //5010   0x01   01
#define    VM_RESERVE_BASE                       0x03fe        //FFFF   0x02   All(FF)

#define    VM_RESERVE_CONFIG_LENGTH              0x0041
#endif

#if  VM_CARDINIT_CONFIG
#define    CARDINIT_CONFIG_BASE                  0x0400        //FFFF   0x00   All(FF)

#define    VM_CARDINIT_CONFIG_LENGTH             ((JCES_CONFIG_END - CURRENT_EXCEPTION_LIST + 0xff) & 0xFFFFFF00)
#endif


#define     VM_CONFIG_OFFSET_END          (0x000004ffL)

/*******Card Init  Area  Config*******/
#define 		CURRENT_EXCEPTION_LIST   					((CARDINIT_CONFIG_BASE + EEPAGE_MASK) & NOR_EEPAGE_MASK)   	//pageAlign
/* Top of E2PROM Heap */
#define 		E2P_TOP										(CURRENT_EXCEPTION_LIST + SIZE_MEMREF)   //+4
/* (real simple) Clear_on_deselect Transient Ram free pointers, 1 per group,
 NUM_GROUPS in all where 'x' is the group number index */
#define 		dTP                     					(E2P_TOP + SIZE_MEMREF)	           //+8
/* Variable to keep track of the maximum Clear_on_deselect
 Transient Ram amount used within the system */
#define 		max_dTP                 					(dTP + (SIZE_U16 * NUM_GROUPS))		 //+88
/* (real simple) Clear_on_Reset Transient Ram free pointer */
#define 		rTP                     					(max_dTP + SIZE_U16)               //+8A
/* flag indicating compaction of rtp transient space needed */
#define 		rTP_OBJECT_DELETED 							(rTP + SIZE_U16)					           //+8C
/* mask indicating which dtp transient space needs compaction */
#define 		dTP_OBJECT_DELETED 							(rTP_OBJECT_DELETED + SIZE_U8)	     //+8D
/*
 * Logical Channels RAM segments
 */
/* Clear On Reset segment start address */
#define 		RAM_RTR_BASE							  	(dTP_OBJECT_DELETED + dTP_NUM)    //+95
#define 		APDU_BUFFER 								(RAM_RTR_BASE + SIZE_MEMREF)        //+99
#define 		SHORT_BUFFER 								(APDU_BUFFER + SIZE_MEMREF)         //+9D
#define 		RAM_BUFFER 									(SHORT_BUFFER + SIZE_MEMREF)        //+A1
#define 		CACHE_BUFFER 								(RAM_BUFFER + SIZE_MEMREF)
/*MAP table and free table*/
#define 		MAX_ENTRY_COUNT_IN_MAP						(CACHE_BUFFER + SIZE_MEMREF)	 //+A5
#define 		USED_ADDR_NO								((MAX_ENTRY_COUNT_IN_MAP + SIZE_U16 + SIZE_U32) & NOR_EECELL_MASK)//(MAX_ENTRY_COUNT_IN_MAP  + SIZE_U32)  //////对齐  A8
#define 		JCRE_ENTRY_END			                    (USED_ADDR_NO + SIZE_MEMREF)   //AC
#define			NON_JCRE_RTR_SIZE							(JCRE_ENTRY_END + SIZE_MEMREF) //B0

#define 		VM_GC_REQUESTED_STAET						(NON_JCRE_RTR_SIZE+SIZE_U16)   //B2
//EEPROM area used for card java heap
#define 		E2P_HEAP_BASE			        			(VM_GC_REQUESTED_STAET + SIZE_U8)  //B3

#define  		USED_ADDR_NO_BAK							(E2P_HEAP_BASE + SIZE_MEMREF)     //B4
//#define			EEP_PATCH_SIZE								(USED_ADDR_NO_BAK + SIZE_MEMREF)
#define			OPTIMIZE_CARDMANAGER_CONFIG					(USED_ADDR_NO_BAK + SIZE_MEMREF)	//B8

#define			ADVANCEOBJECT_ADDR					        (OPTIMIZE_CARDMANAGER_CONFIG + SIZE_U8)		//BC

#define			FILE_SYSTEM_START					        (ADVANCEOBJECT_ADDR + SIZE_MEMREF)//BD
#define			RSP_LAST_OPERATIONAL_PROFILE				(FILE_SYSTEM_START + SIZE_U16)//C1   对应EUICCLastOperationaProfile_Addr
#define			PROFILE_ENABLEINDEX_CFG						(RSP_LAST_OPERATIONAL_PROFILE + SIZE_U8)			//C3
#define			UPDATA_ENABLE_PROFILE_INDEX					(PROFILE_ENABLEINDEX_CFG + SIZE_U8)		//C4
#define			UPDATA_DISABLE_PROFILE_INDEX				(UPDATA_ENABLE_PROFILE_INDEX + SIZE_U8)		//C5
#define			UPDATE_CHANGE_PROFILE_INIT_PRIFILE_INDEX    (UPDATA_DISABLE_PROFILE_INDEX + SIZE_U8)			//C6

#define			M2M_FALLBACK_PROFILE_INDEX            		(UPDATE_CHANGE_PROFILE_INIT_PRIFILE_INDEX + SIZE_U8)	    //C7 对应 M2MFallBackProfileIndex
#define			M2M_FALLBACK_NOALLOWDISABLE_PROFILEINDEX   	(M2M_FALLBACK_PROFILE_INDEX + SIZE_U8)   //C8 对应M2MFallBackDisabledNoAllowProfileIndex
#define			M2M_LASTDISABLE_PROFILEINDEX       			(M2M_FALLBACK_NOALLOWDISABLE_PROFILEINDEX + SIZE_U8)// C9 对应 M2MLastDisableProfileIndex
#define			M2M_NOTIFICATION_TYPE       				(M2M_LASTDISABLE_PROFILEINDEX + SIZE_U8)//CA 对应M2MNotificationType
#if M2M_ESx
#define			M2M_EMERGENCY_PROFILE_INDEX       			(M2M_NOTIFICATION_TYPE + SIZE_U8)//CB 对应M2M_Emergency Profile
#define			M2M_TEST_PROFILE_INDEX       				(M2M_EMERGENCY_PROFILE_INDEX + SIZE_U8)//CC 对应M2M_test Profile
#define			JCES_CONFIG_END								(M2M_TEST_PROFILE_INDEX + SIZE_U8) //CD
#else
#define			JCES_CONFIG_END								(M2M_NOTIFICATION_TYPE + SIZE_U8) //CC
#endif

#define			E2PBase										(VM_CONFIG_START + CURRENT_EXCEPTION_LIST)
#define 		PBOC_BAK_AREA_ADDR						   	getBackupStart()

//#define			SAVE_TRANS_FLAG_AREA						(E2P_IMAGE_START - 0x10)

	//===========================================================================
	//  JAVA  VM Addr config
	//  If you change the value of E2PBase or ROMBase, be sure to update mask.cfg
	//  accordingly. The Mask Generator depends on these values to correctly link
	//  the pre-installed packages.
	//===========================================================================
	//#define E2PBase  											OS_EEP_BASE //start of config area
//#if	JCES_CONFIG_END < SAVE_TRANS_FLAG_AREA
	#define		E2PLimit               						sysConfig_GetNvmTop()
//#else
//	#define 	E2PLimit									"ERROR"
//#endif
#if SE_ONLY
#define 		CONFIG_END_ALIGN							(memref)((CARDINIT_CONFIG_BASE +  VM_CARDINIT_CONFIG_LENGTH + 0xFF) & 0xFFFFFF00)
#else

#define 		EUICC_CONFIG_BASE							(CARDINIT_CONFIG_BASE +  VM_CARDINIT_CONFIG_LENGTH)
#define 		EUICC_CURRENT_EXCEPTION_LIST				(EUICC_CONFIG_BASE)														// 00
#define 		EUICC_dTP									(EUICC_CURRENT_EXCEPTION_LIST +  SIZE_MEMREF * PROFILE_MAX_NUMBER)		// 80
#define 		EUICC_dTP_back								(EUICC_dTP + (SIZE_U16 * NUM_GROUPS) * PROFILE_MAX_NUMBER)				// 1480
#define 		EUICC_max_dTP								(EUICC_dTP_back + (SIZE_U16 * NUM_GROUPS))		 						// 1520
#define 		EUICC_max_dTP_back							(EUICC_max_dTP + (SIZE_U16 * PROFILE_MAX_NUMBER))		 				// 1560
#define 		EUICC_rTP									(EUICC_max_dTP_back + SIZE_U16)		 									// 1562
#define 		EUICC_rTP_back								(EUICC_rTP + SIZE_U16 * PROFILE_MAX_NUMBER)								// 15A2
#define 		EUICC_rTP_ProfileInitial_back				(EUICC_rTP_back + SIZE_U16)												// 15A4
#define 		EUICC_CONFIG_END							(EUICC_rTP_ProfileInitial_back +  SIZE_U16)									// 15A6

#define 		CONFIG_END_ALIGN							(memref)((EUICC_CONFIG_END + 0xFF) & 0xFFFFFF00)

#endif

#endif




