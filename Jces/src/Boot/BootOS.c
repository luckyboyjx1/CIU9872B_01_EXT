/*****************************************************************************
 TestMode.c

 only for test ,

 WangHui
 eastcompeace Technology Corporation

 History:
 2008,1,23	.Created
 *****************************************************************************/

#include "BootOS.h"
#include "BootOSImpl.h"
#include "sysConfigTarget.h"
#include "Apdu.h"
#include "MemBackUp.h"
#include "SysConfigData.h"
#include "MemOperation.h"
#include "Patch.h"
#include "mifare.h"
#include "BootAreaInitRam.h"
#include "jcvmDataConfig.h"
#include "opFunction.h"
#include "RamManager.h"
#include "CgmManager.h"
#include "SysHal.h"

#if HAL__CONFIG_AREA__NEW

static u8		X_A securityState;
static u8			g_privateCmdFlg;
/********************************** store/get data start*******************************************************/
u8 TagError(u8 *buffer, u8 type);
/*@ -disspecopt @*/
/*@ -type @*/
static  const ConfigTagList configTaglist[]=
{ 
// 	[00 00 - 00 FF] For SYS_CONFIG
#if 0
	{0x0000,  GETDATA|BYTECONFIG,						0x10,	bootOSVERSION,},
  // SWP platform not support                                                                                                          
	{0x0001,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	CLK_CONFIG_CB,},
	{0x0002,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	CLK_CONFIG_CL,},
	{0x0003,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	CLK_COM_CONFIG_CL,},
	{0x0004,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	CLK_COM_CONFIG_RSA1,},
	{0x0005,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	CLK_COM_CONFIG_RSA2,},
                                                                                                                                         
	// reserved for the future.                                                                                                            
	{0x0080,  STOREDATA|GETDATA|BITCONFIG,				0x01,	TagError,},
	{0x0081,  STOREDATA|GETDATA|BITCONFIG,				0x01,	TagError,},
	{0x0082,  STOREDATA|GETDATA|BITCONFIG,				0x01,	TagError,},
	{0x0006,  STOREDATA|GETDATA|BYTECONFIG,				0x04,	E2P_CONFIG_TOP_ADDRESS,},
#endif
	{0x0006,  GETDATA|BYTECONFIG,									0x04,	E2P_CONFIG_TOP_ADDRESS,},
	{0x0007,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	E2P_CONFIG_BACK_UP_PAGE_NUM,},
	{0x0008,  STOREDATA|GETDATA|BYTECONFIG,				0x02,	OS_DTR_SIZE,},
#if 0
	{0x0009,  STOREDATA|GETDATA|BYTECONFIG,				0x02,	NON_JCRE_RTR_SIZE,},
#endif
                                                                                                                    
//	[01 00 - 01 FF] For UART_CONFIG
#if 0  // SWP platform not support
	{0x010A,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	IOCONFIG_T1_TB_VALUE,},
	{0x010B,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	IOCONFIG_T1_TC_VALUE,},
	{0x010D,  STOREDATA|GETDATA|BYTECONFIG,				0x02,	TM_GUARDTIME_CB,},

	{0x0180,  STOREDATA|GETDATA|BITCONFIG,				0x01,	IOCONFIG_UART_CONF,},
	{0x0181,  STOREDATA|GETDATA|BITCONFIG,				0x02,	IOCONFIG_UART_CONF,},
	{0x0182,  STOREDATA|GETDATA|BITCONFIG,				0x04,	IOCONFIG_UART_CONF,},
	{0x0183,  STOREDATA|GETDATA|BITCONFIG,				0x05,	IOCONFIG_UART_CONF,},
	{0x0184,  STOREDATA|GETDATA|BITCONFIG,				0x10,	IOCONFIG_UART_CONF,},
	{0x0185,  STOREDATA|GETDATA|BITCONFIG,				0x20,	IOCONFIG_UART_CONF,},
	{0x0188,  STOREDATA|GETDATA|BITCONFIG,				0x40,	TagError,},
#endif
	{0x0101,  STOREDATA|GETDATA,						0x01,	IOCONFIG_COLD_HISTLEN,},
	{0x0102,  STOREDATA|GETDATA,						0x0F,	IOCONFIG_COLD_HISTBYTES,},
	{0x0103,  GETDATA,									0x01,	IOCONFIG_COLD_IFLEN,},
	{0x0104,  STOREDATA|FUNCTIONCONFIG,					0x1F,	PMtMEM(modifyAtr),},
	{0x0105,  STOREDATA|GETDATA,						0x01,	IOCONFIG_WARM_HISTLEN,},
	{0x0106,  STOREDATA|GETDATA,						0x0F,	IOCONFIG_WARM_HISTBYTES,},
	{0x0107,  GETDATA,									0x01,	IOCONFIG_WARM_IFLEN,},
	{0x0108,  STOREDATA|FUNCTIONCONFIG,					0x1f,	PMtMEM(modifyAtr),},
	{0x0109,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	IOCONFIG_TC2_VALUE,},
	{0x010C,  STOREDATA|GETDATA|BYTECONFIG,				0x02,	IOCONFIG_UART_WTX,},

//	[02 00 - 02 FF] For TA_CONFIG
#if 0  // SWP platform not support
	{0x0203,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	IOCONFIG_TA_USERSAK_INCOMPLETE,},
	{0x0205,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	IOCONFIG_TA_IFLEN,},
	{0x0206,  STOREDATA|GETDATA|BYTECONFIG,				0x0F,	IOCONFIG_TA_IFBYTES,},


	{0x0201,  STOREDATA|GETDATA|BYTECONFIG,				0x0B,	SWP_CONFIG_UID,},
	{0x0202,  STOREDATA|GETDATA|BYTECONFIG,				0x01,   SWP_CONFIG_SAK,},
	{0x0204,  STOREDATA|GETDATA|BYTECONFIG,				0x02,	SWP_CONFIG_ATQA,},
	{0x0207,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	APPLICATION_DATA_LEN,},
	{0x0208,  STOREDATA|GETDATA|BYTECONFIG,				0x0F,	APPLICATION_DATA,},
	{0x0209,  STOREDATA|GETDATA|BYTECONFIG,				0x05,	SWP_TYPEB_MODE_PUPI,},
	{0x020A,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	SWP_TYPEB_MODE_AFI,},
	{0x020B,  STOREDATA|GETDATA|BYTECONFIG,				0x04,	SWP_TYPEB_MODE_ATQB,},
	{0x020C,  STOREDATA|GETDATA|BYTECONFIG,				0x03,	DATARATE_MAX_TA,},
	{0x020D,  STOREDATA|GETDATA|BYTECONFIG,				0x03,	DATARATE_MAX_TB,},

	{0x0286,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	SWP_TYPEA_MODE_CID_SUPPORT,},
	{0x0287,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	RF_COMMUNICATION_SWITCH,},
	{0x0288,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	SWP_TYPEA_MODE_ENABLE,},
	{0x0289,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	SWP_TYPEB_MODE_ENABLE,},
#endif

//	[03 00 - 03 FF] For MIFARE_CONFIG
#if 0  // SWP platform not support
	{0x0301,  STOREDATA|GETDATA|BYTECONFIG,				0x02,	IOCONFIG_MIFARE_USERATQ,},
	{0x0302,  STOREDATA|GETDATA|BYTECONFIG,				0x0C,	IOCONFIG_MIFARE_USERUID,},
	{0x0303,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x0304,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x0305,  STOREDATA|GETDATA|BYTECONFIG,				0x0B,	TagError,},
	{0x0380,  STOREDATA|GETDATA|BYTECONFIG,				0x0B, 	TagError,},
	{0x0381,  STOREDATA|GETDATA|BYTECONFIG,				0x0B,	TagError,},
	{0x0382,  STOREDATA|GETDATA|BYTECONFIG,				0x0B,	TagError,},
	{0x0383,  STOREDATA|GETDATA|BYTECONFIG,				0x0B,	TagError,},
	{0x0384,  STOREDATA|GETDATA|BYTECONFIG,				0x0B,	TagError,},


	{0x0306,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	REQA_RESPONSE_DELAY,},
	{0x0307,  STOREDATA|GETDATA|BYTECONFIG,				0x6C,	clfModeIDConfig,},
	{0x0385,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	CLF_MODEL_ID_ENABLE,},
	{0x0386,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	CLF_MODEL_ID,},
#endif
//	[04 00 - 04 FF] For SWP_CONFIG

#if 0  // SWP platform not support
	{0x0401,  STOREDATA|GETDATA|BYTECONFIG,				0x03,	SWP_IDM_REGISTRY_VERSION_SW,},
	{0x0402,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	SWP_IDM_REGISTRY_HCI_VERSION,},
	{0x0403,  STOREDATA|GETDATA|BYTECONFIG,				0x03,	SWP_IDM_REGISTRY_VERSION_HARD,},
	{0x0404,  STOREDATA|GETDATA|BYTECONFIG,				0x08,	SWP_IDM_REGISTRY_VENDOR_NAME,},
	{0x0405,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	SWP_IDM_REGISTRY_MODEL_ID,},
	{0x0407,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	SWP_CLT_SUPPORT,},


	{0x0400,  STOREDATA|GETDATA|BYTECONFIG,				0x08,	SWP_SESSION_ID,},
	{0x0406,  STOREDATA|GETDATA|BYTECONFIG,				0x02,	SWP_LINK_REGISTRY_REC_ERROR,},
	{0x0408,  STOREDATA|GETDATA|BYTECONFIG,				0x10,	HIGHT_LAYER_RESPONSE,},
	{0x0409,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	SWP_FWI_SFGI,},
	{0x040A,  STOREDATA|GETDATA|BYTECONFIG,				0x06,	SWP_GATES_LIST,},
#endif

//	[10 00 - 1F FF] For JCVM_CONFIG
	{0x1001,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	OS_LOGICAL_CHANNEL_NUMBER,},
#if 0  // SWP platform not support
	{0x1000,  STOREDATA|GETDATA|BYTECONFIG,				0x08,	TagError,},
	{0x1080,  STOREDATA|GETDATA|BYTECONFIG,				0x08,	TagError,},
	{0x1081,  STOREDATA|GETDATA|BYTECONFIG,				0x08,	TagError,},
	{0x1082,  STOREDATA|GETDATA|BYTECONFIG,				0x08,	TagError,},
#endif

//	[20 00 - 2F FF] For GP_CONFIG
#if 0  // SWP platform not support
	{0x2000,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2006,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2007,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x200B,  STOREDATA|GETDATA|BYTECONFIG,				0x10,	TagError,},
	{0x200F,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	CM_SD_RSA_TOKEN_BYTE_LEN,},
	{0x2011,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2012,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2013,  STOREDATA|GETDATA|BYTECONFIG,				0x01, 	TagError,},
	{0x2014,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2015,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2016,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2017,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2018,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2019,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x201A,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x201B,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x201C,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2080,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2081,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2082,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2083,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2084,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2085,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x2086,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	GETDATA_SC_SWITCH,},
	{0x2087,  STOREDATA|GETDATA|BITCONFIG,				0x01,	CONFIG_TEST,},
	{0x208C,  STOREDATA|GETDATA|BITCONFIG,				0x04,	OS_CONF,},

#endif

#if 1
	{0x2001,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	ISD_CVM_PIN,},
	{0x2002,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	ISD_CVM_PIN + 1,},
	{0x2003,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	ISD_CVM_PIN + 2,},
	{0x2004,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	ISD_CVM_PIN + 3,},
	{0x2005,  STOREDATA|GETDATA|BYTECONFIG,				0x08,	ISD_CVM_PIN + 4,},
	{0x2008,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	CM_AUTH_MAX_TRY_CNTR,},
	{0x2009,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	CM_AID_LENGTH,},
	{0x200A,  STOREDATA|GETDATA|BYTECONFIG,				0x10,	CM_AID,},
	{0x200C,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	CM_SCP_TYPE,},
	{0x200D,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	SCP_OPTION,},
	{0x200E,  STOREDATA|GETDATA|BYTECONFIG,				0x02,	CM_LAST_BASIC_PACKAGE,},
#if  !GP__DATA_MANAGER__FOR_MASK
	{0x2010,  STOREDATA|GETDATA|BYTECONFIG,				0x30,	CPLC_DATA_CONF,},
#endif
	{0x201D,  STOREDATA|GETDATA|BYTECONFIG,				0x03,	ISD_PRIV,},
#endif
#if BOOT__CONFIGDATA__SENCOND_ATR
	{0x201E,  STOREDATA|GETDATA|FUNCTIONCONFIG,			0x20,	PMtMEM(secondAtrOperate),},
#endif
	{0x201F,  STOREDATA|GETDATA|BYTECONFIG,				0x0A,	DIVERSIFIED_DATA,},
	{0x2088,  STOREDATA|GETDATA|BITCONFIG,				0x02,	CONFIG_TEST,},
	{0x2089,  STOREDATA|GETDATA|BITCONFIG,				0x04,	CONFIG_TEST,},

	{0x208A,  STOREDATA|GETDATA|BITCONFIG,				0x01,	OS_CONF,},
#if GP__CGM__FOR_MASK
	{0x208B,  STOREDATA|GETDATA|FUNCTIONCONFIG,			0x01,	PMtMEM(CgmInit_EnableOrNot),},
	{0x208D,  STOREDATA|GETDATA|BITCONFIG,				0x08,	OS_CONF,},
	{0x2092,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	CGM_SYSTEM_CONSUME_CONFIG,},
#endif
	{0x208E,  STOREDATA|GETDATA|BITCONFIG,				0x10,	OS_CONF,},
	{0x208F,  STOREDATA|GETDATA|BITCONFIG,				0x20,	OS_CONF,},
	{0x2090,  STOREDATA|GETDATA|BITCONFIG,				0x40,	OS_CONF,},
	{0x2091,  STOREDATA|GETDATA|BITCONFIG,				0x80,	OS_CONF,},
	

	//	[30 00 - 3F FF] For APP_CONFIG
#if 0  // SWP platform not support
	{0x3000,  STOREDATA|GETDATA|BYTECONFIG, 			0x01,	TagError,},
	{0x3001,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x3002,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
	{0x3003,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	TagError,},
#endif
	
	{0x3004,  STOREDATA|GETDATA|BYTECONFIG,				0x01,	VGP_CONFIG,},
	{0x3005,  STOREDATA|GETDATA|BYTECONFIG,				0x02,	OTA_REMAIN_SPACE,},
	{0x3006,  STOREDATA|GETDATA|BYTECONFIG,				0x03,	CM_FileIdAddr,},
	{0x3007,  STOREDATA|GETDATA|FUNCTIONCONFIG,			0xFF,	PMtMEM(writeDataOfSelectAppByFileId),},

#if JCVM__IO__SWP
	{0x3092,  STOREDATA|GETDATA|BITCONFIG,				0x20,	IFX_SWITCH,},
#endif

#if BOOT__CONFIGDATA__POWER_OFF_FOR_TEST
	//for power off testing
	{0xFF21,  GETDATA|FUNCTIONCONFIG,					0xFF,	PMtMEM(getPrivateData),},
	{0xFFC1,  GETDATA|FUNCTIONCONFIG,					0xFF,	PMtMEM(getPrivateData),},
#endif
//	error:
	{ERROR_CONFIG,  STOREDATA|GETDATA|FUNCTIONCONFIG,	0xFF,	PMtMEM(TagError),},
};
/*@ +type @*/
/*@ +disspecopt @*/
u8 TagError(u8 *buffer, u8 type)
{
	return false;
}

u16 ConfigDataFunc(u16 tag, PN8 address, PX8 commandBuffer, u8 operateType, u8 operateLength, u8 bitMask)
{
	u8 				tagH;
	u8 				dataLength;
	u8 				result;
	u8 				storeOrGet;
	u8 				oldByteValue;
	ConfigFunction 	configFunction;

	tagH 			= U8_SHR_U8(tag, 8);
	dataLength 		= operateLength;
	result 			= true;

	if(INS_STORE_DATA == commandBuffer[APDU_INS_OFFSET])
	{
		storeOrGet	= STOREDATA;
	}
	else
	{
		storeOrGet	= GETDATA;
	}

	if(U8_AND_U8(operateType, FUNCTIONCONFIG) == FUNCTIONCONFIG)
	{/*@ -castfcnptr @*/
		/*@ -discast @*/
		configFunction 	= (ConfigFunction)address;
		result 			= configFunction(commandBuffer , storeOrGet);
		/*@ +discast @*/
	}/*@ +castfcnptr @*/
	else
	{
		if(storeOrGet == STOREDATA)
		{
			if(U8_AND_U8(operateType, BITCONFIG) == BITCONFIG)
			{
				oldByteValue	= PN_R_U8(address);
				if(commandBuffer[APDU_DATA_OFFSET] > 1)
				{
					return BOOT_DATA_INVALID;
				}
				if(0 == commandBuffer[APDU_DATA_OFFSET])
				{
					commandBuffer[APDU_DATA_OFFSET]		= U8_AND_U8(U8_INV(bitMask), oldByteValue);
				}
				else
				{
					commandBuffer[APDU_DATA_OFFSET]		= U8_OR_U8(bitMask, oldByteValue);
				}
				dataLength = 1;
			}

			nvmCopyAtomic(address, &commandBuffer[APDU_DATA_OFFSET], dataLength);
		}
		else
		{
			ecpMem_Copy(&commandBuffer[APDU_DATA_OFFSET], address, dataLength);
			if(U8_AND_U8(operateType, BITCONFIG) == BITCONFIG)
			{
				if(U8_AND_U8(commandBuffer[APDU_DATA_OFFSET], bitMask) == 0)
				{
					commandBuffer[APDU_DATA_OFFSET]		= 0;
				}
				else
				{
					commandBuffer[APDU_DATA_OFFSET]		= 1;
				}
				dataLength = 1;
			}
			apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, dataLength);
		}
	}
	if(result == TRUE)
	{
		return BOOT_NO_ERROR;
	}
	return BOOT_INS_NOT_SUPPORTED;
}

u16 configDispatcher(PX8 commandBuffer)
{
	u8		i;
	u16 	configName;
	u8 		commandDataLength;
	u8 		operateType;

	configName 			= PXB_R_U16(&commandBuffer[APDU_P1_OFFSET]);

	if(INS_STORE_DATA == commandBuffer[APDU_INS_OFFSET])
	{
		operateType		= STOREDATA;
	}
	else
	{
		operateType		= GETDATA;
	}

	if(operateType == STOREDATA)
	{
		apdu_ApduSetIncomingAndReceive();
		if(boot_unwrap(BOOT_PRIVILEGE_CM) != BOOT_NO_ERROR)
		{
			return BOOT_SECURITY_STATUS_NOT_SATISFIED;
		}	
	}
	else
	{
		if (!(BOOT_IS_HAD_CURRENT_PRIV(BOOT_PRIVILEGE_CM) || (BOOT_READY_STATE == boot_GetLifeCycle())))
		{
			return BOOT_CONDITIONS_NOT_SATISFIED;
		}
	}
	
	//After decrypt, commandDataLength may have been changed.
	commandDataLength = commandBuffer[APDU_P3_OFFSET];
	
	for(i = 0; ; U8_INC(i))
	{
	    if(((configTaglist[i].tagName == configName) && (U8_AND_U8(configTaglist[i].configType, operateType) == operateType))
			|| (configTaglist[i].tagName == ERROR_CONFIG))
	    {
				if(configTaglist[i].configLength < commandDataLength)
				{
						return BOOT_WRONG_LENGTH;
				}
				return ConfigDataFunc(configTaglist[i].tagName, MEMtPN8(JcvmConfig_GetVmConfigAddr(configTaglist[i].configAddress)), commandBuffer, configTaglist[i].configType, commandDataLength, configTaglist[i].configLength);
	    }
	}
	return BOOT_INCORRECT_P1P2;

	
}

/********************************** store/get data end*******************************************************/


u16 bootOsCommand()
{
	u8			D_A		bIns_;
	u16 		D_A 	sw = BOOT_NO_ERROR;
	bIns_		= APDUBuffer[APDU_INS_OFFSET];
	
	selected_channel = 0;
	g_current_SCP_Type = SCP_02;

	switch (APDUBuffer[APDU_INS_OFFSET])
	{
	case INS_VERIFY_TANSPORTKEY:
			//sw = BootVerifyTransportKey(APDUBuffer);
			sw = BOOT_INS_NOT_SUPPORTED;
			break;
		case BOOT_INS_START_BOOT_OS:
			sw = boot_bootOSstart();
			break;
		case BOOT_INS_EXTERNAL_AUTHENTICATE_PRE_OPERATOR:
			sw = boot_ExternalAuthenticatePreOperator();
			break;
		case BOOT_INS_EXTERNAL_AUTHENTICATE:
			sw = boot_VerifyExternalAuthenticate();
			break;
		case BOOT_INS_UPDAKEY:
			sw = boot_UpdateKey();
			break;
		case BOOT_INS_SET_KEY_TRYCTR:
			sw = boot_SetKeyTryCounter();
			break;
		case BOOT_INS_SET_SYSTEM_STATE:
			sw = boot_SetCardState();
			break;

		case INS_STORE_DATA:
		case INS_GET_DATA:
			sw = configDispatcher(APDUBuffer);
			break;
		case INS_GET_CRC:
		case INS_GET_CRC_NEW:
		case INS_VERIFY_CRC:
		case INS_VERIFY_CRC_NEW:
			sw = attainAndVerifyMemoryCode(APDUBuffer);
			break;
		
		case INS_BOOT:
			sw = clearAllEeprom(APDUBuffer);
			break;
		case INS_INIT:
			sw = initCardData();
			break;
#if	BOOT__IO__MIFARE
		case INS_MANAGER_MIFARE:
			sw = mifare_Function(APDUBuffer);
			break;
#endif
		case INS_CLEAR_OS:
		case INS_CLEAR_OS_NEW:
			sw = clearCard();
			break;
		case INS_BLOCK_SELECT:
			sw = BlockSelect(APDUBuffer);
			break;
		case INS_READ:
			sw = Read_EEPROM(APDUBuffer);	
			break;
		case INS_WRITE:
			sw = Write_EEPROM(APDUBuffer);
			break;
		case INS_WRITE_PAGE:
			sw = Write_EEPROM_Page(APDUBuffer);			
			break;

			/*only mobile*/
		//case INS_INIT_DESFIRE:
		//	sw = IF_Initialize_Defire();
		//	break;
		case INS_PATCH:
		case INS_SET_FILE_ID_SYS:
		case INS_CONFIG_APP:
		case INS_CONFIG_CHIP:
		case INS_DERIVE4UID_FROM7UID:
		default:
			//selected_channel = curlogicalChannelBack;
			sw = BOOT_INS_NOT_SUPPORTED;
			break;
	}
	return sw;
}

void BootOsMain(void)
{
	u16		sw;
	u8		currLifeCycle;
	ppApdu_Command_t     D_A    cmd;
	
	sw				= BOOT_NO_ERROR;
	apdu_GetCommand(&cmd);
	APDUBuffer = cmd->pData;
	g_privateCmdFlg = 0;

	initBootSecureGlobalVariable();
	
	while (1)
	{
		
		currLifeCycle = boot_GetLifeCycle();
		if (BOOT_TERMINATED_STATE == currLifeCycle)
		{
			break;
		}

		/*to be compatible with old private commands that have been provided to the ChinaTelecom/ChinaMoble/UNICOM */
		if ((APDUBuffer[APDU_CLA_OFFSET] == 0xE0) && (boot_IsSupportPrevission() == BOOT_SUPPORT_COMPONENT_PRIVATE_COMMAND))
		{
			sw = ProcessRemoteComponentCmd(APDUBuffer);
		}
		else if (APDUBuffer[APDU_CLA_OFFSET] == 0xE4)
		{
			sw = bootOsCommand();
			if ((BOOT_NO_ERROR != sw) && (BOOT_LOCKED_STATE == currLifeCycle))
			{
				break;
			}
		}
		else
		{
			if(LIFE_CYCLE_INIT <= BootLifeCycle_GetLifeCycle())
			{
				break;
			}
			sw				= BOOT_CONDITIONS_NOT_SATISFIED;
		}
			
		memRefresh();
		
		apdu_SendStatusAndReceiveCommand(sw) ;
  } //end of while
}
/*
5 - 21: (transport key length) and (transport key version)
22: CardManager default key version
23: Is VGPconfig
24 - :CardManager AID
*/
u16 BootVerifyTransportKey(PX8 apduBuffer)
{
	u8		D_A		count;
	u16		D_A		sw;
	PN8		D_A		bootKeyCounterBase;

	apdu_ApduSetIncomingAndReceive();

	// p3 长度为0x10
	if (apduBuffer[APDU_P3_OFFSET] != 0x10)
	{
		return BOOT_WRONG_LENGTH;
	}

	bootKeyCounterBase	= MEMtPN8(sysConfig_GetBootKeyCounterBase());
	count				= PN_R_U8(bootKeyCounterBase);

	if (count == 0)
	{
		securityState = SECURITY_STATE_ATTACK;
		return BOOT_MFKEY_VERIFY_FAILED_OVER; // blocked
	}

	memBeginTransaction(SINGLE_APDU_PROCESS);

	if (memCompare(MEMtPM8(&apduBuffer[APDU_DATA_OFFSET]), MEMtPM8(sysConfig_GetBootKeyValueBase()), PLATFORM_ADM_KEY_LEN) == ECP_CMP_RESULT_EQUAL)
	{
		// REVIEW: 最大尝试次数是否过多
		nvmWriteU8NoAtomic(bootKeyCounterBase, MAX_TRY_TIME);
		securityState = SECURITY_STATE_MFKEY_VERIFIED;
		sw = BOOT_NO_ERROR;
	}
	else
	{
		U8_DEC(count);
		nvmWriteU8NoAtomic(bootKeyCounterBase, count);
		securityState = SECURITY_STATE_ATTACK;
		sw = U8_A_U16(count,0x6300);
	}
	memCommitTransaction();

	return sw;
}


u16 loadPlatformAdmKey(u8 * apduBuffer)
{

	apdu_ApduSetIncomingAndReceive();

	// p3 长度为0x10
	if (apduBuffer[APDU_P3_OFFSET] != PLATFORM_ADM_KEY_LEN)
	{
		return 0x6710;
	}

	if(securityState == SECURITY_STATE_ATTACK)
	{
		return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}

	memBeginTransaction(SINGLE_APDU_PROCESS);
	nvmWriteU8Atomic(MEMtPM8(sysConfig_GetBootKeyFlagBase()), KEY_CREATED);
	nvmWriteU8Atomic(MEMtPM8(sysConfig_GetBootKeyCounterBase()), MAX_TRY_TIME);
	nvmCopyAtomic(MEMtPM8(sysConfig_GetBootKeyValueBase()), apduBuffer, PLATFORM_ADM_KEY_LEN);
	memCommitTransaction();

	return BOOT_NO_ERROR;
}

u8 boot_GetSecurityState(void)
{
	return securityState;
}

void bootInit(u8 isDeselect)    SMALL_MODE
{
	sysConfig_Init();

	apdu_Init(isDeselect);
	
	patch_initAfterAts();
	
	memStreamClr();

	apdu_SendStatusAndReceiveCommand(APDU_NO_STATUS);

	ecpSysHal_InitVar();
	
	bootErrorHandle_Init();

	memInitTransaction();
	memAbortTransaction(ABORT_ON_RESET);

	securityState	= 0;
	initBootSecureGlobalVariable();
	g_privateCmdFlg = 0;
}

void BootOs_Resume(void)
{
	initMemBackUpGloableVar();
	NvmManager_Init(MEMtPN8(U32_A_U32(PFtMEM(staticinit), U32_S_U32(EEPROM_IMAGE_SIZE, U32_M_U32(MASK_MAP_ENTRY_COUNT, MAP_ENTRY_SIZE))))
/*@ -disspecopt @*/		,MEMtPN8(U32_S_U32(MASK_MAP_START, U32_M_U32(MASK_MAP_ENTRY_COUNT, MAP_ENTRY_SIZE))) /*@ +disspecopt @*/
						,MASK_MAP_ENTRY_COUNT, MEMtPN8(getBackupStart()));
	NvmManager_Boot(MASK_MAP_ENTRY_COUNT, MEMtPN8(getBackupStart()));
	RamManager_init();
}

/*to be compatible with old private commands that have been provided to the ChinaTelecom/ChinaMoble/UNICOM */
#define SAK_ADDRESS (u16)0x0468
#define ATQA_ADDRESS (u16)0x0469
#define SWP_SESIONID_ADDRESS (u16)0x0400
u16 ProcessRemoteComponentCmd(u8 * apduBuffer)
{

	u8 bIns  = apduBuffer[OFFSET_INS];
	u8 bResLen = apduBuffer[OFFSET_LC];
	u16 addr  = (((u16)apduBuffer[OFFSET_P1] << 8) | (u16)apduBuffer[OFFSET_P2]);
	u16  sw = BOOT_NO_ERROR ;
	u8* bTmpAddr;

	if(apduBuffer[OFFSET_CLA] != 0xE0)
	{
		return BOOT_CONDITIONS_NOT_SATISFIED;
	}

	switch(bIns)
	{
		case 0xD6:
		{
			if(securityState != SECURITY_STATE_MFKEY_VERIFIED)
			{
				return BOOT_CONDITIONS_NOT_SATISFIED;
			}
			apdu_ApduSetIncomingAndReceive();

			if((SAK_ADDRESS == addr)&&(bResLen == 1))
			{
				bTmpAddr = MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_SAK));
			}
			else if((ATQA_ADDRESS == addr)&&(bResLen == 2))
			{
				bTmpAddr = MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_ATQA));
			}
			else if((SWP_SESIONID_ADDRESS == addr)&&(bResLen == 8))
			{
				bTmpAddr =  MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_SESSION_ID));
			}
			else
			{
				return  BOOT_WRONG_LENGTH;
			}

			nvmCopyAtomic(bTmpAddr, (apduBuffer+ 5), bResLen);

			break;
		}
		case 0xB0:
		{
			if(securityState != SECURITY_STATE_MFKEY_VERIFIED)
			{
				return BOOT_CONDITIONS_NOT_SATISFIED;
			}
			if((SAK_ADDRESS == addr)&&(bResLen == 1))
			{
				bTmpAddr = MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_SAK));
			}
			else if((ATQA_ADDRESS == addr)&&(bResLen == 2))
			{
				bTmpAddr = MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_ATQA));;
			}
			else
			{
				return  BOOT_WRONG_LENGTH;
			}

			ecpMem_Copy(apduBuffer, bTmpAddr, bResLen);
			apdu_ApduSetOutgoingAndSend(0,bResLen);
			break;
		}
		case 0x70:
		{
			if((apduBuffer[OFFSET_P1] != 00) ||(apduBuffer[OFFSET_P2] != 0) || (apduBuffer[OFFSET_LC] != 0))
				return  BOOT_CONDITIONS_NOT_SATISFIED;
			break;
		}
		case 0x10:
		case 0x20:
			sw = BootVerifyTransportKey(APDUBuffer);
			break;
		case 0x24:
			sw = loadPlatformAdmKey(APDUBuffer);
			break;
		default:
			return  BOOT_CONDITIONS_NOT_SATISFIED;
	}

	return sw;
}

#endif

