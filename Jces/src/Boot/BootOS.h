#ifndef __BOOTOS_H__
#define  __BOOTOS_H__

#include "DataOperation.h"
#include "SysConfigProfile.h"

#if HAL__CONFIG_AREA__NEW

#define INS_CLEAR_OS									0XE6
#define INS_MANAGE_MIFARE								0X0E
#define INS_GET_CARDINDENTIFY   						0X0F
#define INS_VERIFYMFK           						0X20
#define INS_READ                						0xB0
#define INS_WRITE										0XD6
#define INS_GET_DATA									0xCA
#define INS_BOOT			    						0X71
#define INS_INIT										0x72
#define INS_ISSUE										0X56
#define INS_GETCRC              						0X50
#define INS_VERIFYCRC           						0X54
#define INS_CHANGE_ATR          						0xF2
#define INS_PATCH			    						0X51
#define INS_SEVERINFO									0X4E
#define INS_PUTHEXKEY           						0XD7
#define INS_ATTAIN_MEMORY_CODE  						0X01
#define INS_VERIFY_MEMORY_CODE  						0X02
#define INS_USIM_PERSO_END      						0x44
#define INS_WRITE_OTA_BYTECODE  						0x30
#define INS_MANAGER_MIFARE      						0xE7
#define SET_JPC_DEBUG 									0x8F
#define INS_STORE_DATA									0XE2
#define INS_WRITE_PAGE									0xE8

#define INS_INIT_DESFIRE		0x7D
 #define PRIVATE_CMD_AUTH_KEY_LEN						0x08
 #define UNBLOCK_BOOT_COMMAND							0x09

#define GET_CONFIG_DATA_OFFSET							0x00
#define GET_CONFIG_DATA_LENGTH							0x01

#define JCVM_ILLEGAL_CONFIG_DATA_INDEX					0xFF
#define JCVM_GET_CONFIG_DATA_OFFSET						0x00
#define JCVM_GET_CONFIG_DATA_LENGTH						0x01

#define SYS_CONFIG_GROUP								0x00
#define UART_CONFIG_GROUP								0x01
#define TA_CONFIG_GROUP									0x02
#define MIFARE_CONFIG_GROUP								0x03

#define SWP_CONFIG_GROUP                                0x04  //97

#define JCVM_CONFIG_GROUP								0x10
#define GP_CONFIG_GROUP									0x20
#define APP_CONFIG_GROUP								0x30
#define FINANCE_CONFIG_GROUP							0x40

#define INS_PRIVATE_CMD_CALLBACK						0X55
#define INS_SET_LIFE_CYCLE_OF_BOOT						0x40
#define INS_SET_BOOT_LIFE_CYCLE							0X40
#define INS_GET_BOOT_LIFE_CYCLE							0X42
#define INS_CLEAR_OS_NEW								0X44
#define INS_SET_FILE_ID_SYS								0xE3
#define INS_CONFIG_APP									0xE4

#define INS_GET_CRC_NEW									0X30
#define INS_GET_CRC              						0X50
#define INS_VERIFY_CRC           						0X54
#define INS_VERIFY_CRC_NEW								0X32
#define INS_CONFIG_CHIP									0X73
#define INS_NXPP6_MIFAREFUNC							0x52
#define INS_MIFARE_FUN_NEW								0X80
#define INS_DERIVE4UID_FROM7UID 						0x3F
#define INS_VERIFY_TANSPORTKEY							0X10
#define INS_LOAD_MF_KEY									0X24
#define INS_BLOCK_SELECT								0X70

#define BOOT_NO_ERROR									0x9000
#define BOOT_WRONG_LENGTH								0x6700
#define BOOT_INS_NOT_SUPPORTED							0x6D00
#define BOOT_CONDITIONS_NOT_SATISFIED	    			0x6985
#define BOOT_SECURITY_STATUS_NOT_SATISFIED  			0x6982
#define	BOOT_DATA_INVALID								0x6984
#define BOOT_INCORRECT_P1P2 	              			0x6A86
#define BOOT_AUTHENTICATION_FAILED              		0x6300
#define BOOT_COMMAND_NOT_ALLOWED	      				0x6986
#define BOOT_CRC_RESULT_COMPARE_FAILED      			0x6A87
#define BOOT_BLOCK_SELECT_INVALID_P2        			0x6A82
#define BOOT_MFKEY_VERIFY_FAILED_OVER       			0x6983
#define BOOT_REFERENCED_DATA_NOT_FOUND					0x6A88
#define BOOT_CLA_NOT_SUPPORTED              			0x6E00
#define BOOT_NO_RESOURCE								0x9005

#define LIFE_CYCLE_READY								0x20
#define LIFE_CYCLE_INIT									0x40
#define LIFE_CYCLE_CREATE		  						0x60
#define LIFE_CYCLE_BLOCK								0x80
#define LIFE_CYCLE_MUTE		 	  						0xA0

#define ILLEGL_BANK										MEMtPN8(0xFFFFFF)

// config by invoke the function
#define	FUNCTIONCONFIG									0x20

// config by access the address
#define BYTECONFIG										0x00
#define	BITCONFIG										0x01
#define	COMPARELENGTH									0x02

// command type :
#define	STOREDATA										0x80
#define	GETDATA											0x40
#define	ERROR_CONFIG									0xFFFF



typedef struct tag{
	u16 tagName;  //store/get data P1P2 tag 
	
	u8 configType; 
	
	//u16 configLength;
	u8 configLength; //when using bit to config, then this byte will be reused for bit maping.
	
	//void (*func)(u8 * buffer, u8 type);
	u32 configAddress;
}ConfigTagList;

typedef u8 (*ConfigFunction)(u8 * buffer, u8 type);


#define	IS_CARD_RUN_FIRST_TIME	(PN_R_U8(CM_LIFECYCLE) == NVM_FILL_CHARACTER)
u16	 BootVerifyTransportKey(PX8 apduBuffer);
void BootOsMain(void);
u8	 modifyAtr(PX8 commandBuffer,u8 operateType);
u8	 CheckAtrInterfaceBytes(PR8 atr);
u16	 modifyAtrValue(u8 coldOrWarm, u8 hitoryOrWhole, PR8 buf, u8 length);
void resetSwpProtocol(void);
void InitATR(void);
u8	 BootLifeCycle_GetLifeCycle(void);
u8	 writeDataOfSelectAppByFileId(PX8 commandBuffer, u8 operateType);
void bootInit(u8 isDeselect)    SMALL_MODE;
void BootOs_Resume(void);
#endif

extern u16 initCardData(PX8  apduBuffer);


#endif



