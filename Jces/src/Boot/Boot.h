#ifndef __BOOT_H_
#define __BOOT_H_

#include "DataOperation.h"
#include "bootConfigProfile.h"
#include "SecureOperation.h"

#define INS_PRIVATE_CMD						0XE0
#define INS_VERIFY_TANSPORTKEY   			0X10
#define INS_VERIFY_MF_KEY          			0X20
#define INS_LOAD_MF_KEY            			0X24
#define INS_GET_CRC_NEW						0X30
#define INS_VERIFY_CRC_NEW					0X32
#define INS_SET_BOOT_LIFE_CYCLE				0X40
#define INS_GET_BOOT_LIFE_CYCLE				0X42
#define INS_CLEAR_OS_NEW					0X44
#define INS_PATCH			    			0X51
#define INS_PRIVATE_CMD_CALLBACK			0X55
#define INS_BLOCK_SELECT         			0X70
#define INS_BOOT			    			0X71
#define INS_CONFIG_CHIP						0X73
#define INS_MIFARE_FUN_NEW					0X80
#define INS_READ                			0XB0
#define INS_WRITE							0XD6
#define INS_PFL_WRITE							0XD9
#define INS_STORE_DATA						0XE2
#define INS_GET_DATA						0XCA
#define INS_GET_SESSIONKEY					0XCB

#define INS_FLASH_ERASE_DATA				0XF5
#define INS_FLASH_WRITE_DATA				0XF6

#define INS_DERIVE4UID_FROM7UID 			0x3F
#define INS_GET_CRC              			0X50
#define INS_NXPP6_MIFAREFUNC				0x52
#define INS_VERIFY_CRC           			0X54
#define INS_ISSUE							0X56
#define INS_CLEAR_OS          				0xE6
#define INS_WRITE_PAGE						0xE8

#define INS_BREAK_TEST_PAGE_ERASE           0xD4
#define INS_BREAK_TEST_PAGE_PROG            0xD5

#define INS_ATTAIN_MEMORY_CODE  			0X01
#define INS_VERIFY_MEMORY_CODE  			0X02
#define INS_MEMABORT_TRANSACTION			0xF1

#define INS_SE_INFO							0xDE
#define INS_UPCOS_PRIVATE					0xD2//UPCOS私有指令


#define PRIVATE_INS_VERIFYRELOADMFKEY       0xF6    //校验或者重载MF key
#define PRIVATE_INS_VERIFYRELOADMANAGERKEY  0xF8    //校验或者重载Manager key
#define PRIVATE_INS_UPDATEAUTHDATA          0xFA    //更新鉴权参数
#define PRIVATE_INS_SETCOSVERSIONDATA		0xF9
#define PRIVATE_INS_SetRedTeaRootAESKEY     0xFB    //私有指令更新红茶移动根密钥 执行前提:校验MFkey 更新格式LV(V密钥值)

#define SECURITY_STATE_MFKEY_VERIFIED       ECP_SEC_CHECK_STATE_1
#define SECURITY_STATE_ATTACK   			ECP_SEC_CHECK_STATE_2
#define KEY_CREATED  						0xA3
#define MAX_TRY_TIME 						0x06
#define SPECIAL_EEPROM_TAG_MAX_TRY_TIMES	0x08
#define PLATFORM_ADM_KEY_LEN  				0x10
#define TRANSPORT_KEY_LEN					0x10
#define PRIVATE_CMD_AUTH_KEY_LEN			0x08
#define UNBLOCK_BOOT_COMMAND				0x09

#define BOOT_NO_ERROR						0x9000
#define BOOT_WRONG_LENGTH					0x6700
#define BOOT_MFKEY_WRONG_LENGTH				0x6710
#define BOOT_SECURITY_STATUS_NOT_SATISFIED  0x6982	
#define BOOT_MFKEY_VERIFY_FAILED_OVER       0x6983 
#define BOOT_CONDITIONS_NOT_SATISFIED	    0x6985
#define BOOT_MIFARE_P1P2_NOT_MATCHED        0x6A80
#define BOOT_BLOCK_SELECT_INVALID_P2        0x6A82 
#define BOOT_PATCH_NUMBER_WRONG             0x6A84
#define BOOT_MIFARE_WRONG_P1                0x6A86
#define BOOT_CRC_RESULT_COMPARE_FAILED      0x6A87
#define BOOT_THIS_P1P2_NOT_SUPPORTED	    0x6B00
#define BOOT_INS_NOT_SUPPORTED				0x6D00
#define BOOT_CLA_NOT_SUPPORTED              0x6E00
#define BOOT_REFERENCED_DATA_NOT_FOUND		0x6A88

#define BOOT_NO_RESOURCE					0x9005
#define BOOT_DATA_INVALID               	0x6984

#define MAX_CLEAR   						0x8000
#define NVM_FILL_CHARACTER 					0x00

#define NVM_DEFAULT_VALUE					0xFF
#define NVM_DEFAULT_VALUE_TWO_BYTE			0xFFFF
#define BOOT_READY_STATE            		NVM_DEFAULT_VALUE
#define BOOT_INITIALIZED_STATE				0x01
#define BOOT_SECURED_STATE					0x02
#define BOOT_LOCKED_STATE					0x03
#define BOOT_TERMINATED_STATE				0x04

// command type :
#define	ERROR_CONFIG						0xFFFF

#define	NOT_BOOT_MODE					    0x00

#define GET_LTE_PRODUCT_VERSION             0x3000
#define GET_CHIP_AND_JCES_VERSION           0x0000

#define COSCRC_CALC  0x00
#define COSCRC_VERIFY  0x01

extern u16 configDispatcher(PX8 apduBuf, u8 funcType)    SMALL_MODE;
extern	u8		X_A g_privateCmdFlg;
#if BOOT__BOOT_OS

	extern u8	boot_GetSecurityState(void);
	extern void bootInit(u8 isDeselect)    SMALL_MODE;
	extern void BootOsMain(void);
	extern u8 coscrc_CalcVerify(u8 type);

#else
	
	#define boot_GetSecurityState()										0
	#define bootInit(isDeselect)
	#define BootOsMain()
	#define coscrc_CalcVerify(type)        0
	
#endif

void initBootSCPsessionKey(memref keyBuffer);

#endif
