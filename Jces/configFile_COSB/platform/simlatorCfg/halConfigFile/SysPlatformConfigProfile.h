/*****************************************************************************
 SysPlatformConfigProfile.h

 config the OS.

 wang hui
 eastcompeace Technology Corporation
 Flags:
 .
 Note:
 .

 History:
 2007,6,23	.Created
 *****************************************************************************/

#ifndef   __SYS_PLATFORM_CONFIG_PROFILE_H__
#define   __SYS_PLATFORM_CONFIG_PROFILE_H__

/*=========================================================================== */
/*=========================================================================== */
#define HAL__PLATFORM__CHIP_ARM						0
#define HAL__PLATFORM__SIMULATOR					1
#define HAL__JCES_CHIP__FOR_TEST					0
//#define HAL__UNIT__FOR_DEBUG								0
#define RAM_COMPACT_BUFFER_IN_LOCAL					1

/*=========================================================================== */
/*							APDU , IO features								  */
/*=========================================================================== */
#define	HAL__IO__ISO7816_UART						1
#define HAL__IO__ISO7816_T0							1
#define HAL__IO__ISO7816_T1							0
#define HAL__IO__ISO14443_TA						1
#define	HAL__IO__MIFARE								0
#define HAL__IO__SPI 								0//(sysConfig_GetPROTOCLTYPE())
#define HAL__IO__I2C 								0//(!sysConfig_GetPROTOCLTYPE())
#define HAL__IO__SWP							    0
#define HAL__IO__SWP__LIB                           0

#define HAL__IO__TIMER								1
#define HAL__IO__TIMER_MODE                         0
/*=========================================================================== */
/*							MEM OPERATION features							  */
/*=========================================================================== */
#define HAL__MEMORY__POINTER_24_BIT					0
#define U8_2_U32_INVERSE				1

/*=========================================================================== */
/*							PATCH features									  */
/*=========================================================================== */
#define HAL__PATCH									0

/*=========================================================================== */
/*							SECURE SAFE CRYPTO								  */
/*=========================================================================== */

#define	LIB_HASH_WORKSPACE_SIZE						0x00
#define	LIB_SYM_WORKSPACE_SIZE						0x100

#endif
