/*
 * bootPlaformConfigProfile.h
 *
 *  Created on: 2014Äê11ÔÂ25ÈÕ
 *      Author: jiangxiaohui
 */

#ifndef __BOOT_PLATFORM_CONFIG_PROFILE_H_
#define __BOOT_PLATFORM_CONFIG_PROFILE_H_

/*=========================================================================== */
/*							boot 		function							  */
/*=========================================================================== */
#define	BOOT__IO__MIX_MODE							0
#define BOOTLOADER                      			0

/*=========================================================================== */
/*							function    feature								  */
/*=========================================================================== */
#define BOOT__MEMORY__VALUE_INIT_BY_SEGMENT			0
#define BOOT__MEMORY__FLASH_BACK_UP					1
#define	BOOT__MEMORY__FLASH_BACK_UP_4_BYTE_WRITE	1
#define BOOT__MEMORY__ATOMIC_BACK_UP				0
#define BOOT__MEMORY__NOATOMIC_BACK_UP				0
#define BOOT__MEMORY__CHIP_PAGE_PROTECT				0

#endif /* BOOTCONFIGPROFILE_H_ */
