//#include "FunImp.h"
#include "Boot.h"
#include "DataOperation.h"
#include "SysConfig.h"
#include "Apdu.h"
#include "Io_7816T0.h"
#include "7816Util.h"
#include "APDU.h"
#include "MemBackUp.h"
#include "BootLifeCycle.h"
#include "MemOperation.h"
#include "bootConfigProfile.h"
#include "Crypto_API.h"
#include "SysConfig.h"
#include "BootRam.h"
#include "Mifare.h"
#include "Io_14443TaL3.h"
#include "Patch.h"
#include "SysConfigTarget.h"
#include "bootConfigMethod.h"
#include "SecureOperation.h"
#include "SecureCrypto.h"
#include "SysHal.h"
#include "Crypto_common_API.h"
#include "io.h"
#include "chipHead.h"
//#include "JcvmConfig.h"
#include "SecureCrypto.h"
#include "SysConfigData.h"
//#include "JcvmDataConfig.h"
#include "Patch.h"
#include "AppAPI.h"
//#include "boot_callback.h"
#include "JcvmDataConfig.h"
#include "BootOSImpl.h"
#include "BootOS.h"
#include "chipDataConfig.h"
#include "CgmManager.h"

#include "euiccConfig.h"
#include "bootPlaformConfigProfile.h"
#if !BOOTLOADER_PRO
#include "MifareHal.h"
#include "SwpHal.h"
#include "keyGroup.h"
#include "SecurityDomainData.h"
#include "keySet.h"
#endif

#include "BL_Upgrade.h"

#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#include "FlashBackUp.h"
#include "NvmHal.h"	// remove warning,dengyiwen,2022/9/22
#include "PrivateCommand.h"  // remove warning,dengyiwen,2022/9/22

#if BOOT__BOOT_OS

#pragma BOOT_MAIN_RAM_START

u8				X_A g_securityState;
PN8				X_A SetBank0Base;
u8			  X_A g_privateCmdFlg;

BOOT_MAIN_RAM_END
static u16 CONST  SpecialTag[25] =
{
	0x0000,     /* 获取卡片的boot version*/	
	0x0001,    	/* CLK_CONFIG_CB  */
	0x0002,     /* CLK_CONFIG_CL  */
	0x0003,     /* CLK_CONFIG_CL_COM  */
	0x0004,    	/* CLK_CONFIG_RSA_KEYGEN  */
	0x0005, 	/* CLK_CONFIG_RSA_OPERATION */
	0x0008, 	/* CLK_CONFIG_EEPROM_OPERATION */
	0x0101, 	/* IOCONFIG_COLD_IFLEN */
	0x0102, 	/* IOCONFIG_COLD_IFBYTES*/
	0x0103, 	/* IOCONFIG_COLD_HISTLEN */
	0x0104,    	/* IOCONFIG_COLD_HISTBYTES  */
	0x0105,    	/* IOCONFIG_WARM_IFLEN */
	0x0106, 	/* IOCONFIG_WARM_IFBYTES */
	0x0107, 	/* IOCONFIG_WARM_IFBYTES */
	0x0108, 	/* IOCONFIG_WARM_HISTBYTES */
	0x010C, 	/* IOCONFIG_UART_WTX */
	0x0201, 	/* IOCONFIG_TA_USERUID */
	0x0202, 	/* IOCONFIG_TA_USERSAK_COMPLETE */	
	0x0203,  	/* IOCONFIG_TA_USERSAK_INCOMPLETE */
	0x0204,  	/* IOCONFIG_TA_USERATQ */	
	0x0205,  	/* IOCONFIG_TA_IFLEN */
	0x0206,  	/* IOCONFIG_TA_IFBYTES */
	0x0207, 	/* IOCONFIG_TA_HISTLEN */
	0x0208,  	/* IOCONFIG_TA_HISTBYTES */
	0x0209  	/* IOCONFIG_TA_WTX */	
};


#define	 STOREDATA		   0x01      /* - - - - - - - 1 */
#define	 GETDATA		   0x02      /* - - - - - - 1 - */
#define  BYTECONFIG        0x00      /* - - - - - 0 - - */
#define  BITCONFIG         0x04      /* - - - - - 1 - - */
#define  BITCONFIG_1       0x04      /* - - - - 0 1 - - */
#define  BITCONFIG_2       0x0C      /* - - - - 1 1 - - */
#define  FUNCTIONCONFIG    0x80

typedef u16 (*ConfigFunction)(u8 * buffer);

#define  CYCLE_READY       0x10      /* - - - 1 - - - - */
#define  CYCLE_INIT        0x20      /* - - 1 - - - - - */
#define  CYCLE_CREATION    0x40      /* - 1 - - - - - - */
#define  CYCLE_READY_INIT  0x30      /* - - 1 1 - - - - */
#define  CYCLE_ALL         0x70      /* - 1 1 1 - - - - */

typedef struct TagControl{
	u16 tagName;             //store/get data P1P2 tag 
	u8  property;            //Tag property contain bit/Byte tyep , life Cycle etc
	u8  length;              //when using bit to config, then this byte will be reused for bit maping.
	u16 offsetAddr;          //Tag offset Address
}ConfigTagList;

static const ConfigTagList configTaglist[]=
{
	{0x0000,  GETDATA|BYTECONFIG|CYCLE_ALL,   BOOT_VERSION_SIZE, 0x0000, },                  //boot version
	{0x0001,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, CLK_CONFIG_CB, },
	{0x0002,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, CLK_CONFIG_CL, } ,
	{0x0003,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, CLK_CONFIG_CL_COM, },  
	{0x0004,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, CLK_CONFIG_RSA_KEYGEN, },
	{0x0005,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, CLK_CONFIG_RSA_OPERATION, }	,
	{0x0006,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_READY, 0x04, E2P_CONFIG_TOP_ADDRESS, },	
	{0x0007,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_READY, 0x01, E2P_CONFIG_BACK_UP_PAGE_NUM, },
	{0x0008,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, CLK_CONFIG_CL_EEPROM_PROGRAM, },
	{0x0009,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_READY, 0x01, E2P_CONFIG_FLASH_PROTECT_PAGE_NUM, },
	{0x000A,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_READY, 0x01, MAPTABLE_PAGENUM, },
                                                                                                      
	{0x0080,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x00,	CRYPTO_CONFIG,},
	{0x0082,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x02,	CRYPTO_CONFIG,},	
	{0x0090,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x00,	BOOT_AREA_CONFIG,},           //BOOT_CONFIG_EXTENDED_LOGICAL_CHANNEL 
	{0x0091,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x01,	BOOT_AREA_CONFIG,},           //BOOT_CONFIG_SPECIAL_TAG_VALID_CHECK
	{0x00A0,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x00,	HAL_CONFIG,},                 //HAL_CONFIG_RF_EEP_PROGRAM_CLK

#if (HAL_ISO7816_T0_CONFIG || HAL_ISO7816_T1_CONFIG)
	{0x0101,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, IOCONFIG_COLD_IFLEN, },
	{0x0102,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x0F, IOCONFIG_COLD_IFBYTES, },
	{0x0103,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, IOCONFIG_COLD_HISTLEN, },
	{0x0104,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x0F, IOCONFIG_COLD_HISTBYTES, },
	{0x0105,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, IOCONFIG_WARM_IFLEN, },
	{0x0106,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x0F, IOCONFIG_WARM_IFBYTES, },
	{0x0107,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, IOCONFIG_WARM_HISTLEN, },
	{0x0108,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x0F, IOCONFIG_WARM_HISTBYTES, },
	{0x0109,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, IOCONFIG_TC2_VALUE, },
#if HAL_ISO7816_T1_CONFIG
	{0x010A,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, IOCONFIG_T1_TB_VALUE, },
	{0x010B,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, IOCONFIG_T1_TC_VALUE, },
#endif
	{0x010C,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x02, IOCONFIG_UART_WTX, },
	{0x010D,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x02, IOCONFIG_GUARDTIME_CB, },
	
	{0x0180,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x00,	IOCONFIG_UART_CONF,},
	{0x0181,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x01,	IOCONFIG_UART_CONF,},
	{0x0182,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x02,	IOCONFIG_UART_CONF,},
	{0x0183,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x03,	IOCONFIG_UART_CONF,},
	{0x0184,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x04,	IOCONFIG_UART_CONF,},
	{0x0185,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x05,	IOCONFIG_UART_CONF,},
	{0x0186,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x06,	IOCONFIG_UART_CONF,},
	{0x0187,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x07,	IOCONFIG_UART_CONF,},
	{0x0188,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x02,	CRYPTO_CONFIG,},
	{0x0189,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  0x03,   CRYPTO_CONFIG,},
	{0x018A,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  0x04,   CRYPTO_CONFIG,},
#endif
	
#if  HAL_ISO14443_TA_CONFIG
	{0x0201,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x0C, IOCONFIG_TA_USERUID, },
	{0x0202,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, IOCONFIG_TA_USERSAK_COMPLETE, },
	{0x0203,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, IOCONFIG_TA_USERSAK_INCOMPLETE, },
	{0x0204,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x02, IOCONFIG_TA_USERATQ, },
	{0x0205,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, IOCONFIG_TA_IFLEN, },
	{0x0206,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x0F, IOCONFIG_TA_IFBYTES, },
	{0x0207,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, IOCONFIG_TA_HISTLEN, },
	{0x0208,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x0F, IOCONFIG_TA_HISTBYTES, },
	{0x0209,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x03, IOCONFIG_TA_WTX, },
	
	{0x0280,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x04,	IOCONFIG_TA_CONF,},
	{0x0281,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x00,	IOCONFIG_TA_CONF,},
	{0x0282,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x01,	IOCONFIG_TA_CONF,},
	{0x0283,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x02,	IOCONFIG_TA_CONF,},
	{0x0284,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x03,	IOCONFIG_TA_CONF,},
	{0x0285,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x05,	IOCONFIG_TA_CONF,},
	{0x0286,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x06,	IOCONFIG_TA_CONF,},
	{0x0287,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x07,	IOCONFIG_TA_CONF,},
#endif
	
#if  HAL_MIFARE_CONFIG
	{0x0301,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x02, IOCONFIG_MIFARE_USERATQ, },
	{0x0302,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x0C, IOCONFIG_MIFARE_USERUID, },
	{0x0303,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, IOCONFIG_MIFARE_USERSAK_COMPLETE, },
	{0x0304,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,   0x01, IOCONFIG_MIFARE_USERSAK_INCOMPLETE, },

	{0x0380,  STOREDATA|GETDATA|BITCONFIG_2|CYCLE_ALL,	0x00,	IOCONFIG_MIFARE_CONF,},
	{0x0381,  STOREDATA|GETDATA|BITCONFIG_2|CYCLE_ALL,	0x00,	IOCONFIG_MIFARE_L3_CONF,},
	{0x0382,  STOREDATA|GETDATA|BITCONFIG_2|CYCLE_ALL,	0x02,	IOCONFIG_MIFARE_L3_CONF,},
	{0x0383,  STOREDATA|GETDATA|BITCONFIG_2|CYCLE_ALL,	0x04,	IOCONFIG_MIFARE_L3_CONF,},
	{0x0384,  STOREDATA|GETDATA|BITCONFIG_2|CYCLE_ALL,	0x06,	IOCONFIG_MIFARE_L3_CONF,},
#endif

#if  VM_JC_CONFIG
	{0x1000,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_READY,	0x01,	VM_OS_CONF},
	{0x1001,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_READY,	0x01,	VM_OS_LOGICAL_CHANNEL_NUMBER},
	{0x1002,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x01,	VM_CONFIG},
	{0x1003,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x01,	VM_E2P_CONFIG_RFU},
	{0x1080,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x03,	VM_OS_CONF}, //VM_OS_CONF bit3: OS_CONF3_TCK_ICC_MODE
	{0x1081,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x04,	VM_OS_CONF}, //VM_OS_CONF bit4: OS_CONF3_JC2_JC3_MODE
	{0x1082,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x07,	VM_OS_CONF}, //VM_OS_CONF bit7: OS_CONF7_TCK_BCTC_MODE
#endif

#if  VM_GOP_CONFIG
	{0x2001, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	VM_CVM_PIN},
	{0x2002, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	U16_A_U16(VM_CVM_PIN, 1)},
	{0x2003, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	U16_A_U16(VM_CVM_PIN, 2)},
	{0x2004, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	U16_A_U16(VM_CVM_PIN, 3)},
	{0x2005, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x08,	U16_A_U16(VM_CVM_PIN, 4)},
	
	{0x2006, STOREDATA|GETDATA|BYTECONFIG|CYCLE_READY_INIT,  	0x01,	VM_SCP_DEFAULT_KEY_VERSION},
	{0x2007, STOREDATA|GETDATA|BYTECONFIG|CYCLE_READY_INIT,  	0x34,	VM_KEYSETS},
	{0x2008, STOREDATA|GETDATA|BYTECONFIG|CYCLE_READY_INIT,  	0x01,	CM_AUTH_MAX_TRY_CNTR},
	{0x2009, STOREDATA|GETDATA|BYTECONFIG|CYCLE_READY_INIT,  	0x01,	CM_AID_LENGTH},
	{0x200A, STOREDATA|GETDATA|BYTECONFIG|CYCLE_READY_INIT,  	0x10,	CM_AID},
	
	//{0x200B, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	},
	{0x200C, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	CM_SCP_TYPE},
	{0x200D, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x01,	SCP_OPTION,},
	{0x200E, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x02,	CM_LAST_BASIC_PACKAGE},
	{0x200F, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x02,	CM_SD_RSA_TOKEN_BYTE_LEN},
	{0x2010, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x30,	CPLC_DATA_CONF},
	{0x2011, STOREDATA|GETDATA|BYTECONFIG|CYCLE_READY_INIT,  	0x01,	VM_ISD_COUNT},
	{0x2012, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	VM_TOKEN_KEY_VERSION},
	{0x2013, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	VM_TOKEN_KEY_ID},
	{0x2014, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	VM_RECEIPT_KEY_VERSION},
	{0x2015, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	VM_RECEIPT_KEY_ID},
	{0x2016, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	VM_DAP_KEY_VERSION},
	{0x2017, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	VM_DAP_KEY_ID},
	{0x2018, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	VM_MIN_SCP_KEY_VERSION},
	{0x2019, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	VM_MAX_SCP_KEY_VERSION},
	{0x201A, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	VM_MIN_SCP_KEY_ID},
	{0x201B, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	VM_MAX_SCP_KEY_ID},
	{0x201C, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	VM_SCP_KEY_ID_START_INDEX},
	{0x201D, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	VM_DEFAULT_APP_ID},
	
	{0x2020, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x03,	ISD_PRIV},
	
	{0x2080, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x00,	GP_CONFIG}, // GP_CONF0_INSTALL_FOR_LOAD_NO_HASH
	{0x2081, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x01,	GP_CONFIG}, // GP_CONF1_INSTALL_FOR_OLD_ICC
	{0x2082, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x02,	GP_CONFIG}, // GP_CONF2_ISD_SUPPORT_9F66_TAG
	{0x2083, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x03,	GP_CONFIG}, // GP_CONF3_PRO_SEC_RECEIVE
	{0x2084, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x04,	GP_CONFIG}, // GP_CONF4_INSTALL_FOR_EID
	{0x2085, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x05,	GP_CONFIG}, // GP_CONF5_REGISTER
	{0x2086, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x06,	GP_CONFIG}, // GP_CONF6_SSD_USE_ISD_DIVDATA
	{0x2087, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x07,	GP_CONFIG}, // GP_CONFG7_RFU
#if GP__CGM__FOR_MASK
	{0x208B, STOREDATA|GETDATA|FUNCTIONCONFIG|CYCLE_ALL,0x01,	0},//PMtMEM(CgmInit_EnableOrNot)
	{0x208D, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,	0x01,	VM_CONFIG},
	{0x208E, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x01,	CGM_SYSTEM_CONSUME_CONFIG},
#endif
	{0x2090, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x00,	GP_SECOND_CONFIG}, // GP_SECOND_CONFIG0_CONSTRUCT_AUTH_CODE
	{0x2091, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x01,	GP_SECOND_CONFIG}, // GP_SECOND_CONFIG1_SSD_PSEUDO_RANDOM
	{0x2092, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x02,	GP_SECOND_CONFIG}, // GP_SECOND_CONFIG2_SELECTAPP_FRAME_LC_DISMATCH_DATALEN
	{0x2093, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x03,	GP_SECOND_CONFIG}, // GP_SECOND_CONFIG4_00BF_TAG_ENABLE
	{0x2094, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x04,	GP_SECOND_CONFIG}, // GP_SECOND_CONFIG10_AID_FULLY_MATCH_ENABLE
	{0x2095, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x05,	GP_SECOND_CONFIG}, // GP_SECOND_CONFIG20_SELECT_FILE_BYID_ENABLE
	{0x2096, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x06,	GP_SECOND_CONFIG}, // GP_SECOND_CONFIG40_ABC_RUWEI
	{0x2097, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x07,	GP_SECOND_CONFIG}, // GP_SECOND_CONFIG80_GET_PLATFORM_INFORMATION

	{0x20A0, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x00,	GP_THIRD_CONFIG}, // GP_THIRD_CONFIG bit0: this is used for GP UICC Test
	{0x20A1, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x01,	GP_THIRD_CONFIG}, // GP_THIRD_CONFIG bit1: this is used for GP Mapping Guidelines Test.
	{0x20A2, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x02,	GP_THIRD_CONFIG}, // GP_THIRD_CONFIG bit2: this is used for BFC test.
	{0x20A3, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x03,	GP_THIRD_CONFIG}, // GP_THIRD_CONFIG bit3: this is used for CI test.
	{0x20A4, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x04,	GP_THIRD_CONFIG}, // GP_THIRD_CONFIG bit4: rfu
	{0x20A5, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x05,	GP_THIRD_CONFIG}, // GP_THIRD_CONFIG bit5: rfu
	{0x20A6, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x06,	GP_THIRD_CONFIG}, // GP_THIRD_CONFIG bit6: rfu
	{0x20A7, STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL,  	0x07,	GP_THIRD_CONFIG}, // GP_THIRD_CONFIG bit7: rfu
#endif
	
#if  VM_WITH_SELECT_BY_FILE_ID
#endif

	{0x3004, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,    0x01,	VGP_CONFIG,},

#if APP_PBOC_OR_EDEP_CONFIG
	{0x4005, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	ECP_PSE_INSTANCE_DIR_NUM},
#endif
	
#if  APP_PBOC_CONFIG
	{0x4001, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x02,	ECP_PBOC_INSTANCE_NVM_SIZE},
	{0x4002, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x02,	ECP_PPE_INSTANCE_NVM_SIZE},
	{0x4003, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x02,	ECP_PBOC_INSTANCE_RAM_SIZE},
	{0x4004, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	ECP_PBOC_INSTANCE_DIR_NUM},
	{0x4014, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x05,	ECP_PBOC_DEFAULT_CONFIG_AREA},
	{0x4015, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x07,	U16_A_U16(ECP_PBOC_DEFAULT_CONFIG_AREA, 5)},
	{0x4016, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x03,	U16_A_U16(ECP_PBOC_DEFAULT_CONFIG_AREA, 12)},
#endif
	
#if  APP_EDEP_CONFIG
	{0x4006, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x02,	ECP_EDEP_INSTANCE_NVM_SIZE},
	{0x4007, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	ECP_EDEP_INSTANCE_DIR_NUM},
	{0x4008, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	CONSTRUCT_AUTH_SEL},
	{0x4009, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x04,	CONSTRUCT_AUTH_CHIP_ADDR},
	{0x400A, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x09,	CONSTRUCT_AUTH_CODE},
	{0x400B, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	ECP_EDEP_USER_CFG_PRODUCT_TYPE},
	{0x400C, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	ECP_EDEP_USER_CFG_PRODUCT_TYPE_EX},
	{0x400D, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	ECP_EDEP_USER_CFG_XA_WALLET_ADVANCE},
	{0x400E, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x04,	ECP_EDEP_USER_TFT_VERSION},
	{0x400F, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x05,	ECP_EDEP_USER_CFG_RFU},
	{0x4010, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x10,	ECP_EDEP_MF_KEY_VALUE},
#endif
	
#if APP_JT_FUNC_CONFIG
	{0x4017, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x11,	ECP_EID_INSTANCE_NVM_SIZE},
#endif

#if  APP_EID_CONFIG
	{0x4011, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x02,	ECP_EID_INSTANCE_NVM_SIZE},
	{0x4012, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x02,	ECP_EID_INSTANCE_RAM_SIZE},
	{0x4013, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x0C,	ECP_EID_DEFAULT_CONFIG_AREA},	
#endif
#if  APP_DCEP_CONFIG
	{0x4031, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x02,	ECP_DCEP_INSTANCE_NVM_SIZE},
	{0x4033, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x02,	ECP_DCEP_INSTANCE_RAM_SIZE},
	{0x4032, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x05,	ECP_DCEP_DEFAULT_CONFIG_AREA},
	{0x4034, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	ECP_DCEP_APPLICATION_VERSION},
#endif
#if  APP_DCEP_ZZH_CONFIG
	{0x4041, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x02,	ECP_DCEP_ZZH_INSTANCE_NVM_SIZE},
	{0x4043, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x02,	ECP_DCEP_ZZH_INSTANCE_RAM_SIZE},
	{0x4042, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x05,	ECP_DCEP_ZZH_DEFAULT_CONFIG_AREA},
	{0x4044, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	ECP_DCEP_ZZH_APPLICATION_VERSION},
#endif


	
#if APP_AMENDMENT_B_SPECIAL_CONFIG
	{0x6001, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	TLS_EXTAND_VERSION},
	{0x6002, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	AMENDMENT_B_CONFIG},
	{0x6003, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	AMENDMENT_B_Trigger_SMS_POR},	//同步5g代码，增加一个字节配置https触发短信的apdu-por长度
#endif

#if APP_UICC_FUNCTION
	{0x5001, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x10,	E_SUSPEND_TOKEN},
	{0x5002, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x20,	E_TERMINAL_CAPBILITY},
	{0x5003, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x60,	E_SUSPENDSTATUS},
	{0x5004, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	OPERATOR},
	{0x5005, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	CONFIG_EUICC},
	{0x5006, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	CONFIG_UICC},
	{0x5007, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	E_CMCC_GETONLIESTATE},
	{0x5009, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	NFC_RECORD}, //used for UICC__OPERATOR__CHINAMOBILE; bit1表示NFC刷卡的状态，bit2表示是否需要指定AID的应用才能获取刷卡信息
	{0x5012, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	POLLSYSTEM_DURATION},
	{0x5013, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	PROCESS_PROACTIVE},
#endif
	{0x5008, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	REQUIREMENT_DEFAULT_SCP},
	{0x5010, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	CARD_OPTION},
	{0x5011,  STOREDATA|GETDATA|FUNCTIONCONFIG|CYCLE_ALL,			0xFF,	0},

	{0xFF21,  GETDATA|FUNCTIONCONFIG|CYCLE_ALL,			0xFF,	0},
	{0xFFC1,  GETDATA|FUNCTIONCONFIG|CYCLE_ALL,			0xFF,	0},


    {0xC015,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x01,	CHIP_IO_TYPE },
	{0xC017,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x010,	PATCH_ENKEY },
	{0xC018,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x01,	CHIP_SPI_MODE },
	{0xC019,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x01,	SYSCLK },
	{0xC100,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x01,	ENABLE_PRI_AUTH},
	{0xC101,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x01,	PRI_AUTH_APPAID_LEN},
	{0xC102,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x10,	PRI_AUTH_APPAID},
	{0xC103,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x01,	SSD_AUTH_COUNTER},
	{0xC020,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL, 0x00, CHIPHARDWARE},
	{0xC021,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL, 0x01, CHIPHARDWARE},
	{0xC022,  STOREDATA|GETDATA|BITCONFIG_1|CYCLE_ALL, 0x02, CHIPHARDWARE},
	{0xC023,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x01,	BL_UPADTE_ENMODE},
	{0xC024,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  0x01, CHIP_RESET_ENBLE},
	
#if BOOTLOADER
	//0xB000 ~ 0xB0ff用于cos状态下，修改BL的参数的Tag(BOOTLOADER_CONFIG_BASE)
	{0xB001,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x10,	BLMFKEY_BASE_ADDR_OFFSET},
	{0xB002,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x10,	BLENKEY_BASE_ADDR_OFFSET},
	{0xB003,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x01,	BLMODE_BASE_ADDR_OFFSET},
//	{0xB004,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x01,	BLSPIMODE_BASE_ADDR_OFFSET}, //BL_SPI_MODE
//	{0xB005,  STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,	0x10,	BLVERSION_BASE_ADDR_OFFSET},
#endif
#if APP_ISDR_SPECIAL_CONFIG
	{0x7001, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x01,	ISDR_AIDLENGTH},
	{0x7002, STOREDATA|GETDATA|BYTECONFIG|CYCLE_ALL,  	0x10,	ISDR_AIDVALUE},
#endif
	{0xFFFF, 0, 0,	0,},   //Tag Error
};



#define ILLEGL_BANK			MEMtPN8(0xFFFFFF)


static u16 boot_VerifyTransportKeyInternal(PX8 apduBuffer)    SMALL_MODE
{
	u8		D_A		count;
	u16		D_A		sw;
	PN8		D_A		bootKeyCounterBase;
	u8		X_A		tempKeyBuf[PLATFORM_ADM_KEY_LEN+1];
	u16  volatile   X_A     status;

	PATCH_BOOT_001();
	
	bootKeyCounterBase	= MEMtPN8(sysConfig_GetBootKeyCounterBase());
	count				= PN_R_U8(bootKeyCounterBase);

	if (count == 0)
	{
		g_securityState = SECURITY_STATE_ATTACK;
		return BOOT_MFKEY_VERIFY_FAILED_OVER; // blocked
	}
	U8_DEC(count);
	nvmWriteU8NoAtomic(bootKeyCounterBase, count);
	
	SC_deactiveNVMParityCheck();		
	SC_checkReturnCode(SC_storeSensitiveDataAPI(MEMtPM8(sysConfig_GetBootKeyValueBase()), PLATFORM_ADM_KEY_LEN, tempKeyBuf, PLATFORM_ADM_KEY_LEN, &apduBuffer[APDU_DATA_OFFSET]));
	SC_activeNVMParityCheck();
	memBeginTransaction(SINGLE_APDU_PROCESS);

	status = ECP_CMP_RESULT_NOT_EQUAL;
	status = SC_memCompareEqualWithDFA(tempKeyBuf, MEMtPM8(sysConfig_GetBootKeyValueBase()), SC_PLATFORM_ADM_KEY_LEN);
	
	if (status == ECP_CMP_RESULT_EQUAL)
	{
		if(~status != ~ECP_CMP_RESULT_EQUAL)
		{
			faultDetected();
		}
		
		// REVIEW: 最大尝试次数是否过多
		nvmWriteU8NoAtomic(bootKeyCounterBase, MAX_TRY_TIME);
		g_securityState 		= SECURITY_STATE_MFKEY_VERIFIED;
		sw = BOOT_NO_ERROR;
	}
	else
	{
		g_securityState = SECURITY_STATE_ATTACK;
		sw = U8_A_U16(count,0x6300);
	}
	memCommitTransaction();

	return sw;
}

static void boot_LoadPlatformAdmKeyInternal(PX8 apduBuffer)    SMALL_MODE
{
	PATCH_BOOT_002();
	memBeginTransaction(SINGLE_APDU_PROCESS);
	sysConfig_SetBootKeyCounter(MAX_TRY_TIME);

	SC_deactiveNVMParityCheck();

	SC_checkReturnCode(SC_storeSensitiveDataAPI(MEMtPM8(sysConfig_GetBootKeyValueBase()), PLATFORM_ADM_KEY_LEN, MEMtPM8(sysConfig_GetBootKeyValueBase()), PLATFORM_ADM_KEY_LEN, apduBuffer));
	
	SC_activeNVMParityCheck();
	memCommitTransaction();
}

static u16 boot_UnblockPrivateCommand(PX8 APDUBuffer)    SMALL_MODE
{
	PN8    D_A 	privteCmdAuthKeyBase;
	u8     D_A	pkey;
	u16    D_A  sw;
	PATCH_BOOT_003();
	privteCmdAuthKeyBase 	= MEMtPN8(sysConfig_GetPrivateCmdAuthKeyBase());
	pkey 					= PN_R_U8(PN8_A_U8(privteCmdAuthKeyBase, g_privateCmdFlg));

	sw 						= BOOT_CLA_NOT_SUPPORTED;
	
	apdu_ApduSetIncomingAndReceive();

	if ((g_privateCmdFlg == PRIVATE_CMD_AUTH_KEY_LEN) && (0 == APDUBuffer[APDU_P1_OFFSET]) && (0 == APDUBuffer[APDU_P2_OFFSET]))
	{
		if(SC_memCompare(MEMtPN8(&APDUBuffer[APDU_DATA_OFFSET]), privteCmdAuthKeyBase, PRIVATE_CMD_AUTH_KEY_LEN) == ECP_CMP_RESULT_EQUAL)
		{
			if (BootLifeCycle_IsBlockBootConfig(BLOCK_BIT_MASK) && !BootLifeCycle_IsBlockBootConfig(PRIVATE_COMMAND_DISABLE_MASK))
			{
				U8_INC(g_privateCmdFlg);
				g_securityState = SECURITY_STATE_ATTACK;
				sw = BOOT_NO_ERROR;
			}
		}
	}
	else
	{
		if ((APDUBuffer[APDU_P1_OFFSET] == pkey) && (0 == APDUBuffer[APDU_P2_OFFSET]))
		{
			U8_INC(g_privateCmdFlg);
		}
		else
		{
			g_privateCmdFlg = 0;
		}

	}

	if(BootLifeCycle_IsBlockBootConfig(BLOCK_BIT_MASK) && (0x00 == g_privateCmdFlg))
	{
		if(!BootLifeCycle_IsBlockBootConfig(PRIVATE_COMMAND_DISABLE_MASK))
		{
			memBeginTransaction(SINGLE_APDU_PROCESS);
			pkey			= PN_R_U8(MEMtPN8(sysConfig_GetBootKeyFlagBase()));
			U8_INC(pkey);
			sysConfig_SetBootKeyFlag(pkey);
			memCommitTransaction();
		}
	}
	return sw;
}



static u16 BootVerifyTransportKey(PX8 apduBuffer)    SMALL_MODE
{
	PATCH_BOOT_004();
	apdu_ApduSetIncomingAndReceive();

	// p3 长度为0x10
	if (apduBuffer[APDU_P3_OFFSET] != 0x10)
	{
		return BOOT_WRONG_LENGTH;
	}

	return boot_VerifyTransportKeyInternal(apduBuffer);
}

static u16 loadPlatformAdmKey(PX8 apduBuffer)	SMALL_MODE
{
	u8		D_A  lc;
	u16 	D_A  sw;
	PATCH_BOOT_005();
	apdu_ApduSetIncomingAndReceive();

	lc	= apduBuffer[4];

	// p3 长度为0x10
	if ((0x10 != lc) && (0x20 != lc))
	{
		return BOOT_MFKEY_WRONG_LENGTH;
	}

	if(0x10 == lc)
	{
		if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
		{
			return BOOT_SECURITY_STATUS_NOT_SATISFIED;
		}

		boot_LoadPlatformAdmKeyInternal(&apduBuffer[APDU_DATA_OFFSET]);
	}
	else
	{
		sw	= boot_VerifyTransportKeyInternal(&apduBuffer[APDU_CLA_OFFSET]);

		if(BOOT_NO_ERROR != sw)
		{
			return sw;
		}
		else
		{
			boot_LoadPlatformAdmKeyInternal(&apduBuffer[U8_A_U8(APDU_DATA_OFFSET, PLATFORM_ADM_KEY_LEN)]);
		}
	}

	return BOOT_NO_ERROR;
}
// modify chentao@20230330  产业部需求：计算和校验CRC不输入地址和长度
static u16  attainAndVerifyMemoryCodeNew(PX8 apduBuffer,u8 ins)    SMALL_MODE
{
	u16			D_A		result;
	u8 			D_A		temp[2];
	PATCH_BOOT_006();
	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	
	result	= 0;
	if(apduBuffer[2] == 0)
	{
		result = Get_crc16((PN8)CODECRC_START, (u32)CODECRC_SIZE);
	}
	else if(apduBuffer[2] == 1)
	{
		result = Get_crc16((PN8)USEREEPROMCRC_START, (u32)USEREEPROMCRC_SIZE);
	}
	else if(apduBuffer[2] == 2)//add by chentao@20230529 calu CONFIGUECRC and BOOTLOADERCRC
	{
		result = Get_crc16((PN8)CONFIGUECRC_START, (u32)CONFIGUECRC_SIZE);
	}
	/*else if(apduBuffer[2] == 3)
	{
		result = Get_crc16((PN8)BOOTLOADERCRC_START, (u32)BOOTLOADERCRC_SIZE);
	}*/
	else
	{
		return BOOT_MIFARE_P1P2_NOT_MATCHED;
	}
	
	if(ins == INS_ATTAIN_MEMORY_CODE)
	{		
		PXB_W_U16(apduBuffer , result);
		apdu_ApduSetOutgoingAndSend(0,2);
	}
	else
	{
		apdu_ApduSetIncomingAndReceive();
		PXB_W_U16(temp, result);
		if( SC_memCompareEqualWithDFA(PX8_A_CONST(apduBuffer, 5), MEMtPM8(temp), 2) != ECP_CMP_RESULT_EQUAL)
		{
			return  BOOT_CRC_RESULT_COMPARE_FAILED;
		}
	}
	return BOOT_NO_ERROR;
}

static u16 attainAndVerifyMemoryCode(PX8 apduBuffer,u8 ins)    SMALL_MODE
{
	u32			D_A		length;
	u16			D_A		result;
	u16			D_A		offset;
	PN8			D_A		addr;
	u8 			D_A		temp[2];
	PATCH_BOOT_006();
	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
//	if( ILLEGL_BANK == SetBank0Base)
//	{
//		return BOOT_CONDITIONS_NOT_SATISFIED;
//	}


	result	= 0;
	if(0 == apduBuffer[APDU_P3_OFFSET])
	{
		return BOOT_WRONG_LENGTH;
	}
	offset	= U16_OR_U16(U16_SHL_U8(apduBuffer[2], 8), apduBuffer[3]);
	apdu_ApduSetIncomingAndReceive();
	addr	= PN8_A_U16(MEMtPN8(PXB_R_U32(PX8_A_CONST(apduBuffer, 5))) , offset);
	length = PXB_R_U32(PX8_A_CONST(apduBuffer, 9));
	
	//CRC分两段计算（数据段 + 高频更新文件段）跳过中间的备份区 by xwh 20211227
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	if(PNtMEM(addr + length) > HE_FLASH_BASE)
	{
		length = PNtMEM(BACK_UP_AREA_START - addr);
		result = Get_crc16(addr, length);
		
		addr = (PN8)HE_FLASH_BASE;
		length = HE_FLASH_SIZE;
#if !RUN_IN_SIMULATOR
		result = Get_crc16WithIV(result,addr, length);
#endif
	}
	else
#endif
	{
		result = Get_crc16(addr, length);
	}
	PXB_W_U16(temp, result);
	if(ins == INS_ATTAIN_MEMORY_CODE)
	{
		//*(u16 *)APDUBuffer = result;
		PXB_W_U16(apduBuffer , result);
		apdu_ApduSetOutgoingAndSend(0,2);
	}
	else
	{
	  	if( SC_memCompareEqualWithDFA(PX8_A_CONST(apduBuffer, 13), MEMtPM8(temp), 2) != ECP_CMP_RESULT_EQUAL)
	  	{
	    	return  BOOT_CRC_RESULT_COMPARE_FAILED;
	  	}
	}
	return BOOT_NO_ERROR;
}



//COSCRC calc four part:1: coscode 2: cos configure area 3:user area 4:bootloader area
// modify by xieyong@20220107 增加一卡一CRC功能
u8 coscrc_CalcVerify(u8 type)   
{
	//u32			D_A		length;   //remove warning,dengyiwen,2022/9/21
	u16			D_A		crcresult[3];
	u8   i;
	u8 crcverifyresult;
	
	crcverifyresult=0x00;//no error  bit0:code crc bit1:config crc bit2:usereeprom bit3:bootloader
#if (!RUN_IN_SIMULATOR)
	memRefresh();
	crcresult[0] = Get_crc16((PN8)CODECRC_START, (u32)CODECRC_SIZE);
	crcresult[1] = Get_crc16((PN8)CONFIGUECRC_START, (u32)CONFIGUECRC_SIZE);
	crcresult[2] = Get_crc16((PN8)USEREEPROMCRC_START, (u32)USEREEPROMCRC_SIZE);
	//crcresult[3] = Get_crc16((PN8)BOOTLOADERCRC_START, (u32)BOOTLOADERCRC_SIZE);
	
	if(type == COSCRC_CALC)
	{
		memCopyNoAtomic((u8*)(JC_CRC_SATRT), (u8*)crcresult, sizeof(crcresult));
	}
	else
	{
		//crcverifyresult=0x00;//no error  bit0:code crc bit1:config crc bit2:usereeprom bit3:bootloader

		for(i=0;i<3;i++)
		{
			if(nvmReadU16(PX8_A_CONST(JC_CRC_SATRT, i*2))  != PX_R_U16(&crcresult[i]))
			{
				crcverifyresult|=(1<<i);
			}
		}
		
	}
	memRefresh();
#endif
	return crcverifyresult;
}


static u16 boot_SetLifeCycle(PX8 APDUBuffer)    SMALL_MODE
{
	PATCH_BOOT_007();
	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	switch(APDUBuffer[APDU_P2_OFFSET])
	{
		case LIFE_CYCLE_INIT:
			if(BootLifeCycle_GetLifeCycle() != LIFE_CYCLE_READY)
			{
				return BOOT_SECURITY_STATUS_NOT_SATISFIED;
			}
			return BOOT_INS_NOT_SUPPORTED;
		case LIFE_CYCLE_BLOCK:
			if(BootLifeCycle_GetLifeCycle() < LIFE_CYCLE_INIT)
			{
				return BOOT_SECURITY_STATUS_NOT_SATISFIED;
			}
			BootLifeCycle_BlockBootConfig(BLOCK_BIT_MASK);
			return BOOT_NO_ERROR;
		default:
			return BOOT_CONDITIONS_NOT_SATISFIED;
	}
}

static u16 boot_GetLifeCycle(PX8 APDUBuffer)    SMALL_MODE
{
	PATCH_BOOT_008();
	
	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	
	APDUBuffer[0]	= BootLifeCycle_GetLifeCycle();
	
	if(BootLifeCycle_IsBlockBootConfig(BLOCK_BIT_MASK))
	{
		APDUBuffer[0]	= LIFE_CYCLE_BLOCK;
	}

	apdu_ApduSetOutgoingAndSend(0, 1);

	return BOOT_NO_ERROR;
}

#if CLEAR_OS
static u16 clearCard(PX8 apduBuffer)    SMALL_MODE
{

//	u8 configSelector;
	PATCH_BOOT_009();
	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
		return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}

	if(((apduBuffer[3] != 0) && (apduBuffer[3] != 0x01)) || (apduBuffer[2] != 0))
	{
		return BOOT_THIS_P1P2_NOT_SUPPORTED;
	}
	if(apduBuffer[4] != 0)
	{
		return BOOT_WRONG_LENGTH;
	}
	if (apduBuffer[3] == 0x00)
	{
		//configSelector = 0xFF;  // 0xff: FlashLoader reactivate
	}
	else if (apduBuffer[3] == 0x01)
	{
//			configSelector = 0x80;  // 0x80: FlashLoader deactivate
		return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}  
  
//	//擦除掉PFL后，添加此处检查防止直接E0E6直接废卡
//	if(sysConfig_GetNvmTop() != 0x100BAC00)
//	{
//		return BOOT_SECURITY_STATUS_NOT_SATISFIED;
//	}
	ecpSysHal_ClearCard();
    return BOOT_NO_ERROR;
}
#endif

static u16 BlockSelect(PX8 apduBuffer)    SMALL_MODE
{
	PATCH_BOOT_010();
	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}

	if (apduBuffer[4] != 0)
	{
		return BOOT_WRONG_LENGTH;
	}
	//SetBank0Base = APDUBuffer[3] * 0x010000 + 0XC00000;
	SetBank0Base = MEMtPN8(U8_M_U32(apduBuffer[3] , 0x010000));
	SetBank0Base = MEMtPN8(U32_A_U32(SetBank0Base , EEPROM_BASE));
	
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	if (PNtMEM(SetBank0Base) >= EEPROM_LIMIT_HE)	
#else		
	if (PNtMEM(SetBank0Base) >= EEPROM_LIMIT)
#endif	
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}

	return BOOT_NO_ERROR;
}


static u16 clearAllEeprom(PN8 nvmBottom)    SMALL_MODE
{
	u32			D_A		remainSize;
	PN8			D_A		addr;

	PATCH_BOOT_011();
	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}

	patch_reInit();



	addr = nvmBottom;
	remainSize = U32_S_U32(sysConfig_GetNvmTop(), PNtMEM(nvmBottom));

    fillEepromAccordingToPlatform(MEMtPN8(addr), NVM_FILL_CHARACTER, remainSize);
    // add by chentao@20220930,从RSP_LAST_OPERATIONAL_PROFILE 往后的字节0是有意义的，默认值应该为0xFF
	MemNvmOperation_Fill(MEMtPN8(COS_CONFIG_START + RSP_LAST_OPERATIONAL_PROFILE), 0xFF, (JCES_CONFIG_END - RSP_LAST_OPERATIONAL_PROFILE));
	
	FlashBackUp_clearFlagArea();
	
	BootLifeCycle_SetLifeCycle(LIFE_CYCLE_READY);
	BootLifeCycle_UnblockBootConfig(BLOCK_BIT_MASK);
	
#if GP__AMENDMENT__C__FOR_MASK
	
	clearProtocolData();
#endif
	
#if HAL__IO__MIFARE
	ecpMifareHal_Release();
#endif
	
#if HAL__IO__SWP
	SwpHal_ClearSessionID();
#endif

	return BOOT_NO_ERROR;
}

static u16 boot_ChipConfigFun(PX8 apduBuffer)    SMALL_MODE
{
	u16		D_A sw;
	PATCH_BOOT_012();
	sw	= BOOT_SECURITY_STATUS_NOT_SATISFIED;
	
	if(SECURITY_STATE_MFKEY_VERIFIED == SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
		if(0 != apduBuffer[APDU_P3_OFFSET])
		{
			apdu_ApduSetIncomingAndReceive();
		}
		
		if(true == sysConfig_ChipConfigFun(apduBuffer, apduBuffer[APDU_P1_OFFSET]))
		{
			if(0 != apduBuffer[APDU_P3_OFFSET])
			{
				apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, apduBuffer[APDU_P3_OFFSET]);
			}
			sw	= BOOT_NO_ERROR;
		}
	}
	return sw;
}

static u16 Read_EEPROM(PX8 apduBuffer)     SMALL_MODE
{
	u16			D_A		Lr;
	u16			D_A		offset;
	PN8			D_A		addr;
	PATCH_BOOT_013();
	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	if( SetBank0Base == ILLEGL_BANK)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}
	
	offset	= U16_OR_U16(U16_SHL_U8(apduBuffer[2], 8), apduBuffer[3]);
	addr	= PN8_A_U16(SetBank0Base , offset);
	
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	if (PNtMEM(addr) > EEPROM_LIMIT_HE)	
#else		
	if (PNtMEM(addr) > EEPROM_LIMIT)
#endif	
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}

	Lr = apduBuffer[4];

	if(Lr == 0)
	{
		Lr = 256;
	}

	ecpMem_Copy(apduBuffer, addr, Lr);

	apdu_ApduSetOutgoingAndSend(0,Lr);

	return BOOT_NO_ERROR;
}

static u16 Write_EEPROM(PX8 apduBuffer)    SMALL_MODE
{
	u16			D_A			offset;
	PN8			D_A			addr;

	PATCH_BOOT_014();
	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	if( SetBank0Base == ILLEGL_BANK)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}

	offset	= U16_OR_U16(U16_SHL_U8(apduBuffer[2], 8), apduBuffer[3]);
	addr	= PN8_A_U16(SetBank0Base, offset);

	apdu_ApduSetIncomingAndReceive();

//	nvmCopyAtomic(addr, PX8_A_CONST(apduBuffer, 5), apduBuffer[4]);

	MemNvmOperation_WriteErase(addr, PX8_A_CONST(apduBuffer, 5), apduBuffer[4]);
	
	return BOOT_NO_ERROR;
}

static u16 Write_PFL(PX8 apduBuffer)    SMALL_MODE
{
	u16			D_A			offset;
	u32			D_A			addr;
	u16 					bLen;
	
	PATCH_BOOT_014();
	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}

	if(apduBuffer[APDU_P3_OFFSET] !=0xFF)
	{
		return BOOT_WRONG_LENGTH;
	}
	else
	{
		bLen = 0x100;
	}
	
	//当前PFL大小为15K，直接固定地址写入，每条指令0x100字节
	addr = 0x100C7400;
	offset	= U8_M_U16(apduBuffer[APDU_P1_OFFSET], bLen);
	addr	= U32_A_U16(addr, offset);

	apdu_ApduSetIncomingAndReceive();

	apduBuffer[APDU_P3_OFFSET] = apduBuffer[APDU_P2_OFFSET];

	MemNvmOperation_WriteErase((PN8)addr, PX8_A_CONST(apduBuffer, 4), bLen);
	
	return BOOT_NO_ERROR;
}

u16 Write_EEPROM_Page(PX8 apduBuffer)
{
	u16			D_A			offset;
	PN8 					desAddr;
	PX8 					srcAddr;
	u16 					bLen;

	if(g_securityState == SECURITY_STATE_ATTACK )
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	if( SetBank0Base == ILLEGL_BANK)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}
	
	if(apduBuffer[APDU_P3_OFFSET] !=0xFF)
	{
//		bLen = U8_A_U8(apduBuffer[APDU_P3_OFFSET], 1);
		return BOOT_WRONG_LENGTH;
	}
	else
	{
		bLen = 0x100;
	}
	
	offset	= U16_SHL_U8(U8t16(apduBuffer[APDU_P1_OFFSET]), 8);
	desAddr	= PN8_A_U16(SetBank0Base, offset);
	
	if(!(isInE2P(desAddr) && isInE2P(PN8_A_U16(desAddr,U16_S_U16(bLen,1)))))
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;// only write/read  eep,other address is invalid.
	}
	
	apdu_ApduSetIncomingAndReceive();	

	apduBuffer[APDU_P3_OFFSET] = apduBuffer[APDU_P2_OFFSET];
	
	if(bLen < HAL_EEPAGE_SIZE)
	{
		ecpMem_Copy(&stream_buffer[stream_page_offset], PX8_A_CONST(apduBuffer, APDU_P3_OFFSET), bLen);
		stream_page_offset += bLen;
		if( stream_page_offset == HAL_EEPAGE_SIZE)
		{
			ecpNvm_WriteCache_NoCheck(MEMtPN8(PNtMEM(desAddr) & NOR_EEPAGE_MASK), stream_buffer, HAL_EEPAGE_SIZE);
			stream_page_offset = 0;
		}
	}
	else
	{
		srcAddr	= PX8_A_CONST(apduBuffer, APDU_P3_OFFSET);
		while(bLen != 0)
		{
			ecpNvm_WriteCache_NoCheck(desAddr, srcAddr, HAL_EEPAGE_SIZE);
			desAddr += HAL_EEPAGE_SIZE;
			srcAddr += HAL_EEPAGE_SIZE;
			bLen -= HAL_EEPAGE_SIZE;
		}
	}
	
 	return BOOT_NO_ERROR;
}

#if !BOOTLOADER_PRO	
static u16 FLASH_Write_EEPROM(PX8 apduBuffer)    SMALL_MODE
{
	u16			D_A			offset;
	PN8			D_A			addr;

	PATCH_BOOT_014();
	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	if( SetBank0Base == ILLEGL_BANK)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}

	offset	= U16_OR_U16(U16_SHL_U8(apduBuffer[2], 8), apduBuffer[3]);
	addr	= PN8_A_U16(SetBank0Base, offset);

	apdu_ApduSetIncomingAndReceive();
	if(addr	>=	BACK_UP_AREA_START)
	{
		SC_deactiveEDC();
	}
	//nvmCopyAtomic(addr, PX8_A_CONST(apduBuffer, 5), apduBuffer[4]);
	//memRefresh();
	//E2P_FLASH_stream_flush();
	MemNvmOperation_WriteOnly(addr, PX8_A_CONST(apduBuffer, 5), apduBuffer[4]);
	
	if(addr	>=	BACK_UP_AREA_START)
	{
		SC_activeEDC();
	}
	return BOOT_NO_ERROR;
}

void FlashOperation_Erase(PN8 dest, u16 length)
{
	if(((U32_R_U16(dest, HAL_EEPAGE_SIZE) != 0) || (U32_R_U16(length, HAL_EEPAGE_SIZE) != 0)) && (length != 0))
	{
		bootErrorHandle_ThrowError(0x9007);
	}
	
	while(length > 0)
	{
		if((U32_R_U16(dest , U8_M_U16(4 , HAL_EEPAGE_SIZE)) == 0) && (length >= U8_M_U16(4 , HAL_EEPAGE_SIZE)))
		{
			ecpNvm_EraseBlock(dest);
			length 	= U16_S_U16(length,U8_M_U16(4 , HAL_EEPAGE_SIZE));
			dest 	= PN8_A_U16(dest, U8_M_U16(4 , HAL_EEPAGE_SIZE));
		}
		else
		{
			ecpNvm_ErasePage(dest);
			length  = U16_S_U16(length, HAL_EEPAGE_SIZE);
			dest 	= PN8_A_U16(dest, HAL_EEPAGE_SIZE);
		}
	}
	
	return ;
}

static u16 FLASH_ERASE_EEPROM(PX8 apduBuffer)    SMALL_MODE
{
	u16			D_A			offset;
	PN8			D_A			addr;

	PATCH_BOOT_014();
	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	if( SetBank0Base == ILLEGL_BANK)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}

	offset	= U16_OR_U16(U16_SHL_U8(apduBuffer[2], 8), apduBuffer[3]);
	addr	= PN8_A_U16(SetBank0Base, offset);

	apdu_ApduSetIncomingAndReceive();
	if(addr	>=	BACK_UP_AREA_START)
	{
		SC_deactiveEDC();
	}
	//nvmCopyAtomic(addr, PX8_A_CONST(apduBuffer, 5), apduBuffer[4]);
	//memRefresh();
	//MemNvmOperation_Write_Direct(addr, PX8_A_CONST(apduBuffer, 5), apduBuffer[4]);
	FlashOperation_Erase(addr, FLASH_PAGE_SIZE);
	if(addr	>=	BACK_UP_AREA_START)
	{
		SC_activeEDC();
	}
	return BOOT_NO_ERROR;
}
#endif

static u16 CardIssue(PX8 apduBuffer)    SMALL_MODE
{
	// p3 长度为0x10
	PATCH_BOOT_015();
	if (apduBuffer[APDU_P3_OFFSET] != 0x00 || apduBuffer[APDU_P2_OFFSET] != 0)
	{
		return BOOT_WRONG_LENGTH;
	}
	
	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	if(BootLifeCycle_GetLifeCycle() < LIFE_CYCLE_INIT)
	{
		return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	
	BootLifeCycle_BlockBootConfig(BLOCK_BIT_MASK);

	return BOOT_NO_ERROR;
}

#if JAVA_CARD_UICC 
u16 ChangeSDInfo(PX8 ApduBuffer)
{
	jref keyGroupRef;
	jref keySetRef;
	PN8		sdDataObj;

//	keyGroupRef = ADDR_IS_NULL;
//	keySetRef = ADDR_IS_NULL;

	apdu_ApduSetIncomingAndReceive();

	sdDataObj		=	MEMtPN8(getSdDataObjBySdid(ISD_APPID));

	keyGroupRef = sdData_getKeyGroupRef(sdDataObj, SCP_02, FALSE);

	//if(keyGroupRef != ADDR_IS_NULL)
	{
		keySetRef = keyGroup_getKeySetRef(MEMtPF8(locateObjectInMemory(keyGroupRef)));
		keySet_setSequenceCounter(MEMtPN8(locateObjectInMemory(keySetRef)), 0x02, MEMtPR8(&ApduBuffer[APDU_DATA_OFFSET]));
	}
	return BOOT_NO_ERROR;

}
#endif

#if HAL_PATCH_NATIVE
static u16 boot_dowloadPatch(PX8 apdubuffer)    SMALL_MODE
{
	PATCH_BOOT_016();
  if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}

	if(U8_AND_U8(BootLifeCycle_GetLifeCycle() , LIFE_CYCLE_INIT) != LIFE_CYCLE_INIT)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}
    
    return patch_download(apdubuffer);
}
#endif

static u16 boot_breakTestPageErase(PX8 apdubuffer)    SMALL_MODE
{
	u16			D_A		offset;
	PN8			D_A		addr;

	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	
	if (apdubuffer[APDU_P2_OFFSET] != 0x00)
	{
		return BOOT_THIS_P1P2_NOT_SUPPORTED;
	}

	if (apdubuffer[APDU_P3_OFFSET] != 0)
	{
		return BOOT_WRONG_LENGTH;
	}

	if( SetBank0Base == ILLEGL_BANK)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}
	
	offset	= U16_OR_U16(U16_SHL_U8(apdubuffer[APDU_P1_OFFSET], 8), apdubuffer[APDU_P2_OFFSET]);
	addr	= PN8_A_U16(SetBank0Base , offset);

	MemNvmOperation_Erase(addr, EEPAGE_SIZE);
	
	return BOOT_NO_ERROR;
}

static u16 boot_breakTestPageProg(PX8 apdubuffer)    SMALL_MODE
{
	u16			D_A		offset;
	PN8			D_A		addr;

	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}

	if( SetBank0Base == ILLEGL_BANK)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}
	
	if ( (apdubuffer[APDU_P3_OFFSET] % 4) != 0)
	{
		return BOOT_WRONG_LENGTH;
	}
	
	offset	= U16_OR_U16(U16_SHL_U8(apdubuffer[APDU_P1_OFFSET], 8), apdubuffer[APDU_P2_OFFSET]);
	addr	= PN8_A_U16(SetBank0Base , offset);

	apdu_ApduSetIncomingAndReceive();
	
	MemNvmOperation_WriteOnly(addr, &apdubuffer[APDU_DATA_OFFSET], apdubuffer[APDU_P3_OFFSET]);
	
	return BOOT_NO_ERROR;
}

memref boot_GetConfigBase(u8 tagH, u8 tagL)
{
	memref  D_A    addr;
	PATCH_BOOT_017();
//	addr	= sysConfig_GetSysConfigBase(tagL);
	
	if((tagH == 0x00) && (tagL == 0x00))
	{
		addr	= PFtMEM(&bootVersion[0]);
	}
#if BOOTLOADER
	else if(tagH == 0xB0) //0xB000 ~ 0XB0FF用于在cos状态下配置BootLoader参数
	{
		//extern void Image$$ER_BOOTLOADER_PARA$$Base;
		addr	= BOOTLOADER_CONFIG_START;
	}
#endif
	else
	{
		addr	= COS_CONFIG_START;
	}
	
	return addr;
}
static u16 boot_ValidStoreGetDataCmd(PX8 apduBuf)
{
	u8		D_A		count;
	u16		D_A		sw;
	PN8		D_A		bootSpecialTagAuthCounterBase;
	u8		X_A		tempKeyBuf[8];
	
	if(BOOT_CONFIG_SPECIAL_TAG_VALID_CHECK)
	{
		bootSpecialTagAuthCounterBase	= MEMtPN8(sysConfig_GetSpecialTagAuthCounterBase());
		count				= PN_R_U8(bootSpecialTagAuthCounterBase);

		if ((count == 0)||(0x08 != apduBuf[APDU_P3_OFFSET]))
		{
			g_securityState = SECURITY_STATE_ATTACK;
			return BOOT_MFKEY_VERIFY_FAILED_OVER; // blocked
		}

		SC_checkReturnCode(SC_SYMgetKey_C(MEMtPM8(&tempKeyBuf[0]), MEMtPN8(sysConfig_GetSpecialTagKeyBase()), 8, SENSITIVE_DATA_NOT_MASK));
		memBeginTransaction(SINGLE_APDU_PROCESS);

		if (SC_memCompare(MEMtPM8(&apduBuf[APDU_DATA_OFFSET]), tempKeyBuf, 8) == ECP_CMP_RESULT_EQUAL)
		{
				nvmWriteU8NoAtomic(bootSpecialTagAuthCounterBase, SPECIAL_EEPROM_TAG_MAX_TRY_TIMES);
				g_securityState 		= SECURITY_STATE_MFKEY_VERIFIED;
				sw = BOOT_NO_ERROR;
		}
		else
		{
				U8_DEC(count);
				nvmWriteU8NoAtomic(bootSpecialTagAuthCounterBase, count);
				g_securityState = SECURITY_STATE_ATTACK;
				sw = U8_A_U16(count,0x6300);
		}
		memCommitTransaction();
	}
	else
	{
		g_securityState = SECURITY_STATE_ATTACK;
		return BOOT_CLA_NOT_SUPPORTED; // blocked
	}
	
	return sw;
}
static u8 boot_SpecialEepromConfig(u16 tag)
{
	u8		D_A		loop;
	u8		D_A		counter;
	
	counter = (sizeof(SpecialTag))/2;
	for(loop = 0; loop < counter;U8_INC(loop))
	{
		if(tag == SpecialTag[loop])
		{
			return true;
		}
	}
	return false;	
}
u16 boot_ModifySpecialEepromTagConfig(PX8 apduBuf)
{

	u8		D_A 	isSpecialTag;
	u8		X_A		funcType;
	u16		X_A 	TagBytes;
	u16		X_A 	sw;

	u16		X_A 	p1;
	u16		X_A 	p2;
	
    p1 = U16_SHL_U8(apduBuf[APDU_P1_OFFSET], 8); //(zlh_modify)
	p2 = U8_AND_U8(apduBuf[APDU_P2_OFFSET], 0xff);
	
	funcType 	= apduBuf[APDU_INS_OFFSET];
	
	TagBytes	= U16_OR_U16(p1, p2);
//	TagBytes	= U16_OR_U16(U16_SHL_U8(apduBuf[APDU_P1_OFFSET], 8), U8_AND_U8(apduBuf[APDU_P2_OFFSET], 0xff));

//	if(INS_STORE_DATA == funcType)
//	{
//		apdu_ApduSetIncomingAndReceive();
//	}

	if((INS_STORE_DATA == funcType) && (0xFFFF == TagBytes))
	{
		apdu_ApduSetIncomingAndReceive();
		sw = boot_ValidStoreGetDataCmd(apduBuf);
	}
	else
	{
		isSpecialTag = boot_SpecialEepromConfig(TagBytes);
		if(1 == isSpecialTag)
		{

			sw = configDispatcher(apduBuf,funcType);
		}
		else
		{
			sw = BOOT_CLA_NOT_SUPPORTED;
		}
	}
	return sw;
	
}


static u16 SeInfo(PX8 apduBuffer)
{

	u16			D_A			offset;
	u16			D_A			lr;
	u8          result;
	u8          InfoType;
	
#if (!RUN_IN_SIMULATOR)
	lr = apduBuffer[APDU_P3_OFFSET];

	if (apduBuffer[APDU_P1_OFFSET] != 0x00)
	{
		if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
		{
			return BOOT_SECURITY_STATUS_NOT_SATISFIED;
		}
	}
	else
	{
		if(apduBuffer[APDU_P3_OFFSET] == 0x00)
		{
			lr = 256;
		}
	}
	
	offset	= (u16)apduBuffer[APDU_P2_OFFSET];
	InfoType = apduBuffer[APDU_P1_OFFSET];

	if (InfoType == 0x01)
	{
		//write data
		apdu_ApduSetIncomingAndReceive();
		result=sysConfig_SeInfoFun(&apduBuffer[APDU_DATA_OFFSET], offset,lr ,InfoType);
	}
	else if(InfoType == 0x00)//read data
	{
		result=sysConfig_SeInfoFun(&apduBuffer[0], offset,lr ,InfoType);
	}
	else
	{
		return BOOT_THIS_P1P2_NOT_SUPPORTED;
	}
	if ( result == 0x01)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}
	
	else
	{
		if (InfoType == 0x00)
		{
			//read data
			apdu_ApduSetOutgoingAndSend(0, lr);
		}
		return BOOT_NO_ERROR;
	
	}
#else
	return BOOT_NO_ERROR;
#endif
}


#pragma SAVE_OT
#pragma NO_OT
u16 configDispatcher(PX8 apduBuf, u8 funcType)    SMALL_MODE
{
	u8		D_A		tagH;
	u8		D_A		tagL;
	u8		D_A		oldValue;
	u8		D_A		newValue;
	u8		D_A		isStoreData;
	u8		D_A		legalLength;
	u8		D_A		lifeCycle;
	u8      D_A     tagProperty;
	u8      D_A     bitOffset;
	u8      D_A     bitMask;
	u16   D_A     sw;
	u16     D_A     i;
//	u16     D_A     theTag;
	u16		D_A		dataLength;
	PN8		D_A		configDataAddress;
//	ConfigFunction 	configFunction;

	PATCH_BOOT_018();
	

//    sw = 0;
	lifeCycle	= BootLifeCycle_GetLifeCycle();

	if(INS_STORE_DATA == funcType)
	{
		isStoreData		= true;
		apdu_ApduSetIncomingAndReceive();
	}
	else
	{
		isStoreData		= false;
	}
	
	tagH		= apduBuf[APDU_P1_OFFSET];//zlh_modify  change position
	tagL		= apduBuf[APDU_P2_OFFSET];
	dataLength	= apduBuf[APDU_P3_OFFSET];
	//privilege check
	if( (g_securityState == SECURITY_STATE_ATTACK) && (U16_OR_U16(U16_SHL_U16(tagH, 8), tagL) !=  0x00))
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}
	
	//find Tag config Control information by loop
	for(i = 0; ; U8_INC(i))
	{
		if( configTaglist[i].tagName == U16_OR_U16(U16_SHL_U16(tagH, 8), tagL) )
		{
			tagProperty		  = configTaglist[i].property;
			configDataAddress = PN8_A_U16(MEMtPN8(boot_GetConfigBase(tagH, tagL)), configTaglist[i].offsetAddr);
			break;
		}
		if( configTaglist[i].tagName == 0xFFFF)
		{
			return BOOT_INS_NOT_SUPPORTED;         //not find
		}
	}

	//tag cycle check
	if( U8_AND_U8(tagProperty, CYCLE_ALL) != CYCLE_ALL )
	{
		if( ((lifeCycle == LIFE_CYCLE_READY) && (U8_AND_U8(tagProperty, CYCLE_READY) != CYCLE_READY )) \
			||((lifeCycle == LIFE_CYCLE_INIT) && (U8_AND_U8(tagProperty, CYCLE_INIT) != CYCLE_INIT )) \
			||((lifeCycle == LIFE_CYCLE_CREATE) && (U8_AND_U8(tagProperty, CYCLE_CREATION) != CYCLE_CREATION ))  )
		{
			return BOOT_CONDITIONS_NOT_SATISFIED;
		}
	}
	if(true == isStoreData)
	{
		if(U8_AND_U8(tagProperty, STOREDATA) != STOREDATA) 
		{
			return BOOT_INS_NOT_SUPPORTED;         //not find
		}
	}
	//bit config check;
	if(U8_AND_U8(tagProperty, BITCONFIG) == BITCONFIG)   
	{
		 bitMask   = 0x01;
		 bitOffset = configTaglist[i].length;
		 oldValue  = PN_R_U8(configDataAddress);
		
		 if(U8_AND_U8(tagProperty, BITCONFIG_2) == BITCONFIG_2)
		 {
			  bitMask   = 0x03;
		 }
		 if(true == isStoreData)
		 {
			 	if(U8_AND_U8(tagProperty, STOREDATA) != STOREDATA) 
				{
					return BOOT_INS_NOT_SUPPORTED;         //not find
				}
			 
			 oldValue = U8_AND_U8(oldValue, U8_INV(U8_SHL_U8(bitMask,bitOffset)));
			 newValue = U8_AND_U8(bitMask, apduBuf[APDU_DATA_OFFSET]);
			 newValue = U8_OR_U8(oldValue, U8_SHL_U8(newValue, bitOffset));
		 }
		 else
		 {
			 oldValue	= U8_AND_U8(oldValue, U8_SHL_U8(bitMask,bitOffset));
			 newValue	= U8_SHR_U8(oldValue, bitOffset); 
		 }
		 apduBuf[APDU_DATA_OFFSET]	= newValue;
		 legalLength = 0x01;
	}
	else if(U8_AND_U8(tagProperty, FUNCTIONCONFIG) == FUNCTIONCONFIG)
	{
//		configFunction 	= (ConfigFunction)(PMtMEM(getPrivateData));
#if !BOOTLOADER_PRO		
		sw = getPrivateData(apduBuf,funcType);
#else
		sw = 0x9000;
#endif
		return sw;
//		return configFunction(apduBuf);
	}/*@ +discast @*//*@ +castfcnptr @*/
	else //byte config check;
	{
			legalLength = configTaglist[i].length;
	}
	
    //check length
	if(dataLength > legalLength)
	{
		return BOOT_WRONG_LENGTH;
	}

	if(true == isStoreData)
	{
		nvmCopyAtomic(configDataAddress, MEMtPM8(&apduBuf[APDU_DATA_OFFSET]), dataLength);
	}
	else
	{
		if( apduBuf[APDU_P2_OFFSET] < 0x80)	     
		{
			memCopyToRam(MEMtPX8(&apduBuf[APDU_DATA_OFFSET]),configDataAddress, legalLength);
		}
		apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, legalLength);
	}

	return BOOT_NO_ERROR;
}
#pragma RESTORE_OT

u8 boot_GetSecurityState(void)
{
	return g_securityState;
}
#if !BOOTLOADER_PRO	
extern u16			X_A									gFlash_FlagBlockOffset;
extern u16			X_A									gFlash_BackUpFlagOffset;
u16 test_memAbortTransaction()
{
	PM8 addr;
	u16 i;
	u32 commitFlag;

	addr	= PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE));
	for(i = 0; i < EEPAGE_SIZE; i += FLASH_FLAG_INFO_SIZE)
	{
		commitFlag = memReadU32(addr + i);
		if(commitFlag != 0xAAAAAAAA)
		{
			gFlash_FlagBlockOffset = i;
			gFlash_BackUpFlagOffset = gFlash_FlagBlockOffset + 3*FLASH_FLAG_INFO_SIZE;
			break;
		}
	}
	memAbortTransaction(ABORT_NOT_ON_RESET);
	return 0x9000;
}
#endif
void bootInit(u8 isDeselect)    SMALL_MODE
{
    sysConfig_Init();

	apdu_Init(isDeselect);
	
	patch_initAfterAts();
	
	memStreamClr();
	
	apdu_SendStatusAndReceiveCommand(APDU_NO_STATUS);
	
	bootErrorHandle_Init();

	memInitTransaction();
	memAbortTransaction(ABORT_ON_RESET);

	g_securityState	= SECURITY_STATE_ATTACK;
	SetBank0Base = ILLEGL_BANK;
	g_privateCmdFlg = 0;
	
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	openWrSwitch();
	HED_HEFlash_Init();
	closeWrSwitch();
#endif
}
/*******************************************************************************
 * Function name: derive4UidFrom7Uid
 * Description: This function is used to derive 4 byte NUID out of 7 byte UID,
 *              and the process method is defined in 3.3.2 of AN10927 (MIFARE
 *              and handling of UIDs) Rev.3.0, 4 August 2011, 190730(NXP).
 * Input Parameters:   (precondition)
 *		None.
 * Output Parameters:  (postcondition)
 *		The result 4 byte NUID is stored in the corresponding configure area.
 * NOTE:
 ********************************************************************************/
#if WITH_RF_DERIVE_UID
u16 derive4UidFrom7Uid(void)
{
	u8		X_A			tempUidBuffer[4];
	PATCH_BOOT_021();
	
	if(SECURITY_STATE_MFKEY_VERIFIED != SC_StateCheck(g_securityState, U8_INV(g_securityState), ECP_SECURE_CHECK_INIT))
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	
	ecpCiu_derive4UidFrom7Uid(tempUidBuffer);
	memCopyNoAtomic(MEMtPM8(sysConfig_GetTAUIDBase()), tempUidBuffer, 4);  // store the 4 byte NUID to the configure area.
	return 0x9000;
}
#endif

u16 bootConfig(PX8 ApduBuffer, PN8 nvmBottom, u8 isBootMode)    SMALL_MODE
{
	u8 apdu_ins;
	u16	   D_A  	sw;

	PATCH_BOOT_023();
	
handleBootCommand:
	
	apdu_ins = ApduBuffer[APDU_INS_OFFSET];
	sw	= 0x6E00;

	if (BootLifeCycle_IsBlockBootConfig(BLOCK_BIT_MASK) && (UNBLOCK_BOOT_COMMAND != g_privateCmdFlg))
	{
		if (apdu_ins == INS_PRIVATE_CMD_CALLBACK)
		{
			sw = boot_UnblockPrivateCommand(ApduBuffer);
		}
		else if((apdu_ins == INS_STORE_DATA) || (apdu_ins == INS_GET_DATA))
		{
			sw = boot_ModifySpecialEepromTagConfig(ApduBuffer);
		}
		else
		{
			sw = BOOT_CLA_NOT_SUPPORTED;
		}
	}
	else
	{
		//double check the block status
		if (g_privateCmdFlg == 0xA7)
		{
			sw = BOOT_CLA_NOT_SUPPORTED;
		}
		else
		{
		
			switch (apdu_ins)
			{
				case INS_VERIFY_TANSPORTKEY:
				case INS_VERIFY_MF_KEY:
					sw = BootVerifyTransportKey(ApduBuffer);
					break;
				case INS_LOAD_MF_KEY:
					sw = loadPlatformAdmKey(ApduBuffer);
					break;
				case INS_GET_CRC_NEW:
					sw = attainAndVerifyMemoryCodeNew(ApduBuffer, INS_ATTAIN_MEMORY_CODE);
					break;
				case INS_GET_CRC:
					sw = attainAndVerifyMemoryCode(ApduBuffer, INS_ATTAIN_MEMORY_CODE);
					break;
				case INS_VERIFY_CRC_NEW:
					sw = attainAndVerifyMemoryCodeNew(ApduBuffer, INS_VERIFY_MEMORY_CODE);
					break;
				case INS_VERIFY_CRC:
					sw = attainAndVerifyMemoryCode(ApduBuffer, INS_VERIFY_MEMORY_CODE);
					break;
		#if !BOOTLOADER_PRO 
				case INS_SET_BOOT_LIFE_CYCLE:
					sw = boot_SetLifeCycle(ApduBuffer);
					break;
				case INS_GET_BOOT_LIFE_CYCLE:
					sw = boot_GetLifeCycle(ApduBuffer);
					break;
		#endif
		#if CLEAR_OS
				case INS_CLEAR_OS:
				case INS_CLEAR_OS_NEW:
					sw = clearCard(ApduBuffer);
					break;
		#endif
		#if HAL_PATCH_NATIVE
				case INS_PATCH:
					sw = boot_dowloadPatch(ApduBuffer);
					break;
		#endif
				case INS_BLOCK_SELECT:
					sw = BlockSelect(ApduBuffer);
					break;
		#if !BOOTLOADER_PRO
				case INS_BOOT:
					sw = clearAllEeprom(nvmBottom);
					break;
		#endif
				case INS_CONFIG_CHIP:
					sw = boot_ChipConfigFun(ApduBuffer);
					break;
		#if BOOT__IO__MIFARE
				case INS_NXPP6_MIFAREFUNC:
				case INS_MIFARE_FUN_NEW:
					sw = mifare_Function(ApduBuffer);
					break;
		#endif
				case INS_READ:
					sw = Read_EEPROM(ApduBuffer);
					break;
				case INS_WRITE:
					sw = Write_EEPROM(ApduBuffer);
					break;
				case INS_PFL_WRITE:
					sw = Write_PFL(ApduBuffer);
					break;
		#if !BOOTLOADER_PRO 
				case INS_ISSUE:
					sw = CardIssue(ApduBuffer);
					break;
		#endif
		#if JAVA_CARD_UICC 
				case INS_SET_ISD_DEFAULT_KEY_COUNTER:
					sw = ChangeSDInfo(ApduBuffer);
					break;
		#endif
				case INS_STORE_DATA:
					sw = configDispatcher(ApduBuffer, INS_STORE_DATA);
					break;
				case INS_GET_DATA:
					sw = configDispatcher(ApduBuffer, INS_GET_DATA);
					break;
		#if !BOOTLOADER_PRO	
				case INS_FLASH_ERASE_DATA:
					sw = FLASH_ERASE_EEPROM(ApduBuffer);
					break;
				case INS_FLASH_WRITE_DATA:
					sw = FLASH_Write_EEPROM(ApduBuffer);
					break;
		#endif
	#if WITH_RF_DERIVE_UID
				case INS_DERIVE4UID_FROM7UID:
					sw = derive4UidFrom7Uid();
					break;
	#endif
				case INS_WRITE_PAGE:
					sw = Write_EEPROM_Page(ApduBuffer);
					break;
	#if BOOTLOADER
				case INS_CODE_BLOCK_SELECT:
					sw = LoaderBlockSelect(ApduBuffer);
					break;
				case INS_DOWNLOAD:
					sw = WriteAPP_EEPROM(ApduBuffer);
					break;
				case INS_DOWNLOAD_CHECK_CRC:
					sw = CheckCrcValue(ApduBuffer);
					break;
#if !BOOTLOADER_PRO				
				case INS_SWITCH_ECP_LOADER:
					sw = switchToEcpLoader(1);
					break;
#endif				
	#endif
#if (SUPPORT_COS_UPDATE&&(!BOOTLOADER_PRO))
			   case INS_UPCOS_PRIVATE:
				   sw = UPCOSPrivateCommand(ApduBuffer);
				   break;
	#endif
				
	#if UICC_FUCTION_PRIVATEAUTH
				//case PRIVATE_INS_VERIFYRELOADMFKEY:  这个需求已经从国网需求中去掉了 by chentao 
				case PRIVATE_INS_VERIFYRELOADMANAGERKEY:
					sw = ReloadVerifyManagenerKey(ApduBuffer);
					break;				
				case PRIVATE_INS_UPDATEAUTHDATA:
					sw = UpdateAuthData(ApduBuffer);
					break;
	#endif
				
#if  EUICC__FUNCTION_RedTea_Mobile
				case PRIVATE_INS_SetRedTeaRootAESKEY:
					sw = SetRedTeaRootAESKEY(ApduBuffer);
					break;
#endif
#if JAVA_CARD_UICC
				case PRIVATE_INS_SETCOSVERSIONDATA:
					sw = SetCOSVersionData(ApduBuffer);
					break;
#endif					
#if (EUICC_ONLY || SE_AND_EUICC)
				case INS_GET_SESSIONKEY:
					sw = getSessionkey(ApduBuffer);
					break;
				
				case INS_SE_INFO:
					sw = SeInfo(ApduBuffer);
					break;
#endif				
				case INS_BREAK_TEST_PAGE_ERASE:
					sw = boot_breakTestPageErase(ApduBuffer);
					break;
				case INS_BREAK_TEST_PAGE_PROG:
					sw = boot_breakTestPageProg(ApduBuffer);
					break;
			#if !BOOTLOADER_PRO			
				case INS_MEMABORT_TRANSACTION:
					sw = test_memAbortTransaction();
					break;
			#endif
				default:
					sw = BOOT_INS_NOT_SUPPORTED;
					break;
			}

			if(isBootMode)
			{
				apdu_SendStatusAndReceiveCommand(sw);
				goto handleBootCommand;
			}
		}
	}

	return sw;
}

void BootOsMain(void)
{
	u16 D_A sw;
	
	PM8 apduBuffer;
	
	ppApdu_Command_t     D_A    cmd;
	
	PATCH_JCVM_INIT_JCVMCONFIG_003();
	
	sw = 0x9000;
	apdu_GetCommand(&cmd);
	//APDUBuffer = cmd->pData;
	apduBuffer = cmd->pData;
	
	while (1)
	{
		if (apduBuffer[APDU_CLA_OFFSET] == INS_PRIVATE_CMD)
		{
			sw = bootConfig(apduBuffer, MEMtPN8(E2PBase), NOT_BOOT_MODE);

			if ((BOOT_INS_NOT_SUPPORTED == sw) || (INS_BOOT == apduBuffer[APDU_INS_OFFSET]))
			{
				switch (apduBuffer[APDU_INS_OFFSET])
				{
				#if !NATIVE_CARD
					case INS_BOOT:
						sw = AppAPI_ClearConfigFile(sw);
					    break;
					case INS_INIT:
					case INS_SET_LIFE_CYCLE_OF_BOOT:
						sw = initCardData(apduBuffer);
						break;
					case INS_SET_FILE_ID_SYS:
						sw = AppAPI_WriteConfigFile(apduBuffer);
						break;
				#if !BOOTLOADER_PRO
					case INS_UPDATE_DEFAULT_UID: // 产业化要用
						sw = JcvmConfig_UpdateIsdUid(apduBuffer);
						break;
				#endif
//					case INS_CONFIG_APP:
//						sw = natAppComEntry.NaAppAPI_ConfigApp(apduBuffer);
//						break;
				#endif
					default:
						break;
				}
			}

		}
		else
		{
			#if NATIVE_CARD_UICC	
			//NATIVE UICC不需要执行EPX私有指令,by xwh 20220325
			break;
			#else
			if (BootLifeCycle_IsCardInit())
			{
				break;
			}
			else
			{
				sw = BOOT_CLA_NOT_SUPPORTED;
			}
			#endif
		}
		apdu_SendStatusAndReceiveCommand(sw);
	}
}

#endif
