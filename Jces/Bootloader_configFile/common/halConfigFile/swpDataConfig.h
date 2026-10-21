#ifndef __SWP_DATA_CONFIG_H_
#define __SWP_DATA_CONFIG_H_

#include "SysConfigProfile.h"
#include "SysConfigTarget.h"
#include "ecpDefine.h"
//#include "Swp_Hcp_Layer.h"

#if HAL__PLATFORM__CHIP_ARM
	extern void Image$$ER_JC_SEC_KEY$$Base;
#endif

#if (HAL__CONFIG_AREA__NEW)

#define			ROM_AREA_START								ROM_BASE
#define 		BOOT_KEY_DATA_AREA_SIZE						0x1f0
#define			COS_CONFIG_OFFSET_START						(0)

//bootOS keys
#if HAL__PLATFORM__SIMULATOR
#define			BOOT_CONFIG_KEY_BASE						(ROM_BASE + ROMSIZE + EEPROM_IMAGE_SIZE)
#else
#define 		BOOT_CONFIG_KEY_BASE                		((memref)&Image$$ER_JC_SEC_KEY$$Base)
#endif

//安全保护区域
#define			SEC_SEGMENT						  			(0)
#define    		SEC_MF_KEY_VALUE				  			(SEC_SEGMENT) 					  	  //+0x10       00112233445566778899AABBCCDDEEFF
#define     	PRIVATE_CMD_AUTH_KEY    		    (SEC_SEGMENT)
#define 		  SEC_MF_KEY_CNTR					  			(SEC_SEGMENT + 0X10)				  //+0x01	    06
#define 		  SEC_MF_KEY_FLAG					  			(SEC_SEGMENT + 0X11)				  //+0x01	    A3
#define     	CM_LIFECYCLE				      			(SEC_SEGMENT + 0X12)				  //+0x02	    01FE
#define 		  CM_AUTH_TRY_CNTR              	(SEC_SEGMENT + 0X14)				  //+0x01		00
#define     	ISD_CVM_PIN									    (SEC_SEGMENT + 0X15)				  //+0x0c		03030106313233343536FFFF //要使用ASCII码保存
#define			IOCONFIG_COLD_HISTBYTES						(ISD_CVM_PIN + 0xC)
#define			IOCONFIG_COLD_HISTLEN						(IOCONFIG_COLD_HISTBYTES + 0xF)
#define			IOCONFIG_COLD_IFBYTES						(IOCONFIG_COLD_HISTLEN + 0x1)
#define			IOCONFIG_COLD_IFLEN							(IOCONFIG_COLD_IFBYTES + 0xF)
#define			IOCONFIG_WARM_HISTBYTES					(IOCONFIG_COLD_IFLEN + 0x1)
#define			IOCONFIG_WARM_HISTLEN						(IOCONFIG_WARM_HISTBYTES + 0xF)
#define			IOCONFIG_WARM_IFBYTES						(IOCONFIG_WARM_HISTLEN + 0x1)
#define			IOCONFIG_WARM_IFLEN							(IOCONFIG_WARM_IFBYTES + 0xF)
#define			IOCONFIG_T0_CONFIG							(IOCONFIG_WARM_IFLEN + 0x1)
#define			CRYPTO_CONFIG								(IOCONFIG_T0_CONFIG + 0x1)
#define			E2P_CONFIG_TOP_ADDRESS						(CRYPTO_CONFIG + 0x1)                 //+ 0x01		00

#define 		keySetsLoaded					  			(E2P_CONFIG_TOP_ADDRESS + 0x4)                 //+ 0x01		00
#define     	registry_updateEnable        (keySetsLoaded + 0x1) 				  //+ 0x01		00

#define			DIVERSIFIED_DATA				  	 		(registry_updateEnable + 0x1)				  //0x0A BYTE

#define 		IOCONFIG_GUARDTIME_CB			  			(DIVERSIFIED_DATA + 0xA)  				  //+ 0x02			04FF

//其它参数区，注意当SEC_SEGMENT区域内的空间大小发生变化，GENERAL_SEGMENT也可能需要更新
#define			GENERAL_SEGMENT								(IOCONFIG_GUARDTIME_CB + 0x2)                //

#define 		OTHER_CONFIG	  				  			(GENERAL_SEGMENT + 0X0)              //+ 0x01			00
#define 		OTHER_CONFIG_LVDD1				  			((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(OTHER_CONFIG)))) & 0x40)
#define			ERROR_HANDLE								(GENERAL_SEGMENT + 0X01)             //+ 0x01			00
#define         USAT_TEST                                   (GENERAL_SEGMENT + 0x02)
#define         CGM_SYSTEM_CONSUME_CONFIG					(GENERAL_SEGMENT + 0x03)    //CGM统计系统消耗的配置开关，00为系统消耗由SSD本身扣除，01为系统消耗由ISD扣除
#define         AMDC_EFCFG_FLAG                             (GENERAL_SEGMENT + 0x04)
#define         CLF_MODEL_ID_ENABLE                         (GENERAL_SEGMENT + 0x05)
#define     	E2P_CONFIG_BACK_UP_PAGE_NUM			  		(GENERAL_SEGMENT + 0X06)	//+ 0x01      32(0X20)
#define			E2P_CONFIG_MAP_BIT_TBL_SEGMENT				(GENERAL_SEGMENT + 0X07)	//+ 0x01			8 (8*128=1024 byte)
#define			E2P_CONFIG_FREE_TBL_ENTRY					(GENERAL_SEGMENT + 0X08)	//+	0x01			72  //实际是72+8=80项
#define			E2P_CONFIG_RFU								(GENERAL_SEGMENT + 0X09)	//+	0x01			00
//用户可配置参数
//通讯协议部分
#define 		IOCONFIG_UART_WTX				  			(GENERAL_SEGMENT + 0X0A)   			 //+ 0x02			1F40

#define         USIM_PERSO                                  (memref)(IOCONFIG_UART_WTX + 0x2)	 //1B
#define         OTA_REMAIN_SPACE                            (memref)(USIM_PERSO + 0x1)	 //2B

#define 		IOCONFIG_UART_CONF				  			(OTA_REMAIN_SPACE + 0x2)  			 //+ 0x01   		01
#define 		IOCONFIG_UART_PPSENABLE		  	  			((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_UART_CONF)))) & 0x01)
#define 		IOCONFIG_UART_SLEEPMODE		  	  			((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_UART_CONF)))) & 0x02)
#define         IOCONFIG_UART_DEFAULT_PROTOCOL	  			((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_UART_CONF)))) & 0x04)
#define         IOCONFIG_UART_INVERSE_CONVENTION  			((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_UART_CONF)))) & 0x08)
#define         IOCONFIG_UART_COLD_TCK_PRESENT	  			((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_UART_CONF)))) & 0x10)
#define         IOCONFIG_UART_WARM_TCK_PRESENT	  			((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(IOCONFIG_UART_CONF)))) & 0x20)
#define 		IOCONFIG_TC2_VALUE				  			(IOCONFIG_UART_CONF + 0x1)             //+ 0x01			0A

// 系统功能部分
#define         OS_CONF										(IOCONFIG_TC2_VALUE + 0x1)			 //+ 0x01		    //测移动的所有脚本: 0f/07
#define 		    OS_CGM_CLOSE   			  	  			    ((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(OS_CONF)))) & 0x02)
#define			    OS_C7C8_NOCHECK								((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(OS_CONF)))) & 0x08)						//测tck,pboc时，nocheck,0x1f
#define			    OS_C4_NOCHECK								((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(OS_CONF)))) & 0x10)
#define         OS_NOATOMIC_BACK_UP                         ((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(OS_CONF)))) & 0x20)          //非原子是否备份。0 不备份，1 备份。
#define			    OS_SELECT_BY_FIleID							((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(OS_CONF)))) & 0x80)
#define			OS_LOGICAL_CHANNEL_NUMBER					(OS_CONF + 0x1)  			 //+ 0x01			01
#define			OS_DTR_SIZE									(OS_LOGICAL_CHANNEL_NUMBER + 0x1)  			 //+ 0x02			0400

#define			VGP_CONFIG									(OS_DTR_SIZE + 0x2)         	 //+ 0x01
#define			    IS_VGP_CONFIG								(PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(VGP_CONFIG))))
#define         CONFIG_VGP                                  (U8_AND_U8(IS_VGP_CONFIG ,0x01))
#define         CONFIG_SECURE_CHANNLE_SERVICE_GP221         (U8_AND_U8(IS_VGP_CONFIG ,0x02)) //GPSystem.getSecureChannel()的开关,1表示按照GP221，0表示非GP221
#define         CONFIG_GP221                                (U8_AND_U8(IS_VGP_CONFIG ,0x10))
#define         CONFIG_CLENTRY_ALL_CREATE                   (U8_AND_U8(IS_VGP_CONFIG ,0x20))
// GP功能部分
#define     CM_SCP_TYPE					      			(VGP_CONFIG + 0x1)  			 //+ 0x01			02
#define 		CM_AUTH_MAX_TRY_CNTR			  			(CM_SCP_TYPE + 0x1)  			 //+ 0x01			0A
#define			CM_AID_LENGTH								(CM_AUTH_MAX_TRY_CNTR + 0x1)    		 //+ 0x01			08
#define			CM_AID										(CM_AID_LENGTH + 0x1)  			 //+ 0x10			A000000003000000
#define			CM_LAST_BASIC_PACKAGE					    (CM_AID + 0x10)  			 //+ 0x02			0026
#define			CM_FileIdRECNUM								(CM_LAST_BASIC_PACKAGE + 0x2)  			 //+ 0x01			00
#define			CM_FileIdAddr								(CM_FileIdRECNUM + 0x1)  			 //+ 0x02			2F00
#define 		CM_SD_RSA_TOKEN_BYTE_LEN					(CM_FileIdAddr + 0x2)  			 //+ 0x01			80

#define			REQ_BEIJING_PERSO                               (CM_SD_RSA_TOKEN_BYTE_LEN + 0x1)
#define			    CSNEnble							   ((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(REQ_BEIJING_PERSO)))) & 0x01)
#define			    UIDSEQ							  	   ((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(REQ_BEIJING_PERSO)))) & 0x02)
#define 		RF_COMMUNICATION_SWITCH		           		(REQ_BEIJING_PERSO + 0x1)
#define     NO_CURRENT_LIMIT                            (RF_COMMUNICATION_SWITCH + 0x1)

#define         AMENDMENT_C_TEST                            (NO_CURRENT_LIMIT + 0x01)                       //1B

#define 		CONFIG_TEST									(AMENDMENT_C_TEST + 0x1)					   //1B		 RFU1
#define			SCP80_TEST			              	        ((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(CONFIG_TEST)))) & 0x01)  //做成可配置，测试电信时需要菜单置0 测scp80不需要菜单置1
#define         GALITT_TEST                                 ((PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(CONFIG_TEST)))) & 0x02)

#define 		SUPPORT_SSD_NESTING							(CONFIG_TEST + 0x1)					   //1B		 SSD support nesting or not, 1: support,0: not support
#define 		SCP_OPTION									(SUPPORT_SSD_NESTING + 0x1)					   //1B		 i = 0x15/0x55/0x35/0x75
#define 		GETDATA_SC_SWITCH							(SCP_OPTION + 0x1)					   //1B		 GET DATA mandating SC  ,1: on; 0: off

//#define			SCP											(PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(CM_SCP_TYPE))))
//#define			OPTION										(PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(SCP_OPTION))))
//#define			GET_DAT_SC									(PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(GETDATA_SC_SWITCH))))

#define			ISD_PRIV									(GETDATA_SC_SWITCH + 0x1)						//3B

#define         ECP_SEGMENT_END                             (ISD_PRIV + 0x3)                        //


#define         OS_SECOND_ATR                        (ECP_SEGMENT_END)


#define			HAL_CONFIG_END						(SEC_SEGMENT + 0xC0)

#endif

#endif
