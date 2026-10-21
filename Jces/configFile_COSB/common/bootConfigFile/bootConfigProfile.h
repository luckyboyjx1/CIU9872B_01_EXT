/*
 * bootConfigProfile.h
 *
 *  Created on: 2014年11月25日
 *      Author: jiangxiaohui
 */

#ifndef BOOTCONFIGPROFILE_H_
#define BOOTCONFIGPROFILE_H_

#include "SysConfigProfile.h"
#include "bootPlaformConfigProfile.h"

/* INTEGER support code enabled if '1' */
#define MAP_ADDRESS_NEED_DEAL							1
#define BOOT_7816_SEND_STATUS_FLAG						PH_ISO7816L4_FLAG_BYPASSPARSER
#define NVM_FILL_CHARACTER 								0x00
/*=========================================================================== */
/*							boot 		function							  */
/*=========================================================================== */
#define	BOOT__IO__APDU_API							1
#define	BOOT__IO__APDU								1
#define BOOT__IO__APDU_EXTENDED_LENGTH				0
#define BOOT__IO__PROTOCOL							1
#define BOOT__IO__ISO7816							1
#define BOOT__IO__MIFARE							HAL__IO__MIFARE
#define BOOT__IO__SWP								HAL__IO__SWP
#define BOOT__IO__SPI								HAL__IO__SPI
#define BOOT__IO__HCIAPI							HAL__IO__SWP
#define BOOT__IO__SPI_I2C_PROTOCOL_A				HAL__IO__SPI

#define BOOT__LIFE_CYCLE							1
#define	BOOT__BOOT_OS								1
#define BOOT__CHANNEL__GET_RESPONSE_DATA			0
#define BOOT__CHANNEL__EXTEND_LOGICAL_CHANNEL		0
#define BOOT__MEMORY__FOUR_BYTE_ALIGN				0 //We should lock this switch when used resize
#define BOOT__MEMORY__BACK_UP						1
#define	BOOT__MEMORY__FREE_TABLE					1

#define BOOT__MEMORY__NVM_MANAGER					1
#if BOOT__MEMORY__NVM_MANAGER
	#define	BOOT__MEMORY__NVM_MANAGER_NVM_ALLOCATE		1
	#define BOOT__MEMORY__NVM_MANAGER_NVM_COMPACT		1
	#define BOOT__MEMORY__NVM_MANAGER_NVM_DEALLOCATE	1
	#define BOOT__MEMORY__NVM_MANAGER_MAP_TABLE			1
#else
	#define	BOOT__MEMORY__NVM_MANAGER_NVM_ALLOCATE		0
	#define BOOT__MEMORY__NVM_MANAGER_NVM_COMPACT		0
	#define BOOT__MEMORY__NVM_MANAGER_NVM_DEALLOCATE	0
	#define BOOT__MEMORY__NVM_MANAGER_MAP_TABLE			0
#endif

#if BOOT__MEMORY__NVM_MANAGER
#define BOOT__MEMORY__NVM_RESIZE						1
#else
#define BOOT__MEMORY__NVM_RESIZE						0
#endif

#define BOOT__MEMORY__RAM_MANAGER						1

#define BOOT__MEMORY__OPERATION							1
/*=========================================================================== */
/*							function    feature								  */
/*=========================================================================== */
#define	BOOT__SECURITY__KEY_DATA_SAFE					1
#define BOOT__CONFIGDATA__POWER_OFF_FOR_TEST            1        // =1 : support power off test requirement
#define BOOT__CONFIGDATA__SENCOND_ATR       			0        // =1 : support personal ATR for produc
#define BOOT__CONFIGDATA__WRITE_FILE_ID           		0
#define	BOOT__BOOTCOMMAND__LOCKED						1

#define	BOOT__CONFIGDATA__CARD_VERSION		        	0

#define ISO_BASIC_CHANNEL_NUM                       0
#define ISO_MAX_LOGICAL_CHANNELS					4	// ISO interface used max channel num   需要跟Java层定义的宏对应：ISO_MAX_LOGICAL_CHANNELS

#define NFC_BASIC_CHANNEL_NUM						(ISO_BASIC_CHANNEL_NUM + ISO_MAX_LOGICAL_CHANNELS)	// swp interface basic channel num
#define NFC_MAX_LOGICAL_CHANNELS					0	// SWP interface used max channel num   需要跟Java层定义的宏对应：NFC_MAX_LOGICAL_CHANNELS

#define SPI_BASIC_CHANNEL_NUM						4//(NFC_BASIC_CHANNEL_NUM + NFC_MAX_LOGICAL_CHANNELS)	// spi interface basic channel num
#define SPI_MAX_LOGICAL_CHANNELS					4	// spi interface used max channel num   需要跟Java层定义的宏对应：SPI_MAX_LOGICAL_CHANNELS

#define MAX_LOGICAL_CHANNELS   						(NFC_MAX_LOGICAL_CHANNELS + ISO_MAX_LOGICAL_CHANNELS + SPI_MAX_LOGICAL_CHANNELS)

#endif /* BOOTCONFIGPROFILE_H_ */
