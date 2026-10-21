#ifndef __BOOTLOADER_FUNIMP_H__
#define	__BOOTLOADER_FUNIMP_H__

#include "ecpDefine.h"
#include "Boot.h"
#include "FunEntryConfig.h"
#include "bootPlaformConfigProfile.h"

#if !HAL__PLATFORM__SIMULATOR
#if SUPPORT_COS_UPDATE
extern u8     COS_UPDATE_COSLoading;

extern void Image$$ER_FUNENTRYCONFIG$$Base;
//extern void Image$$ER_FUNENTRYCONFIG$$Limit;
extern void Image$$ER_FUNBOOTENTRYCONFIG$$Base;
//extern void Image$$ER_FUNBOOTENTRYCONFIG$$Limit;

//extern void Image$$ER_COSB_BOOTLOADER_CODE$$Limit;
extern void Image$$ER_BOOTLOADER_CODE$$Base;

//extern void Image$$ER_BOOTLOADER_PARA$$Base;
extern void Image$$ER_BOOTLOADER_HAL_CONFIG_DATA$$Base;
//extern void Image$$ER_PATCH_START$$Base;
extern void Image$$ER_COS_CODE$$Base;
extern void Image$$ER_COSA_BACKENTRY$$Base;
extern void Image$$ER_COSB_BACKENTRY$$Base;

#define APDU_CLA_OFFSET    						0
#define APDU_INS_OFFSET								1
#define APDU_P1_OFFSET								2
#define APDU_P2_OFFSET     						3
#define APDU_P3_OFFSET     						4
#define APDU_DATA_OFFSET   						5
#define APDU_HEADER_LENGTH						5
#define APDU_EXT_CDATA_OFFSET					7

#define BL_LOAD_KEY_TAG								0xC104

#define     CHIP_CONFIG_START										((u32)&Image$$ER_FUNENTRYCONFIG$$Base)
//#define     CHIP_CONFIG_END										((u32)&Image$$ER_FUNENTRYCONFIG$$Limit)
#define     BOOT_CONFIG_START										((u32)&Image$$ER_FUNBOOTENTRYCONFIG$$Base)
//#define     BOOT_CONFIG_LIMIT										((u32)&Image$$ER_FUNBOOTENTRYCONFIG$$Limit)
//#define     BOOT_LAODER_LIMIT										((u32)&Image$$ER_COSB_BOOTLOADER_CODE$$Limit)

//#define     BOOT_PARA_START										((u32)&Image$$ER_BOOTLOADER_PARA$$Base)
#define     BOOT_LOADER_START									((u32)&Image$$ER_BOOTLOADER_CODE$$Base)
//#define     BOOT_LOADER_END										((u32)&Image$$ER_COSA_BACKENTRY$$Base)
#define     COSA_DATA_START										((u32)&Image$$ER_COS_CODE$$Base)

#if COS_UPDATE_ONLINE_MUTUAL
#define     COSA_BACKENTRY         						((u32)&Image$$ER_COSA_BACKENTRY$$Base)	
#define     COSB_BACKENTRY         						((u32)&Image$$ER_COSB_BACKENTRY$$Base)
#endif

#define 	BOOTLOADER_CONFIG_START						((u32)&Image$$ER_BOOTLOADER_HAL_CONFIG_DATA$$Base)
#define     BOOTLOADER_END         						0x129000		//BootLoader end


#define INS_CODE_BLOCK_SELECT										0xD0
#define INS_DOWNLOAD														0xD7
#define INS_DOWNLOAD_CHECK_CRC  								0x33
#define INS_SWITCH_ECP_LOADER										0xE9
#define INS_GET_OS_INFO         								0xBF
#define INS_LOAD_BL_KEY													0x25

#define INS_UPCOS_PRIVATE					0xD2//UPCOS_ONLINE ÀΩ”–÷∏¡Ó

extern  u8		X_A g_securityState;
extern  PN8		X_A SetBank0Base;
extern u16 BootLoader_Get_crc16(PM8 pu1CRC, u32 u1CRCLength, u16 ValCrc);
extern u16 switchToEcpLoader(u8 mode);
extern u16 WriteAPP_EEPROM(PX8 apduBuffer);
extern u16 LoaderBlockSelect(PX8 apduBuffer);
extern u16 CheckCrcValue(PX8 apduBuffer);
extern u16 GetOsInfo(PX8 apduBuffer);
extern u16 LoaderBootloaderKey(PX8 apduBuffer);
extern void BootLoader_Cpy(PX8 Des, PM8 src, u16 Length);
extern void initBootLoaderVar(void);
extern void IF_SetUPCOSFileSystemAddr(void);

#else

#if NATIVE_CARD_UICC
#define initBootLoaderVar()					0
#endif

#endif
#endif
#endif
