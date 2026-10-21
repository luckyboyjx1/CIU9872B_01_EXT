/*
 * hwConfig.h
 *
 *  Created on: 2015Äê1ÔÂ4ÈÕ
 *      Author: mowenpan
 */

#ifndef HWCONFIG_H_
#define HWCONFIG_H_


#include "ecpDefine.h"
#include "SysConfigTarget.h"  //TODO

#define HW_EEPROM_BASE								    EEPROM_BASE
#define HW_EEPROM_LIMIT                                	EEPROM_LIMIT
#define HW_EEPROM_SIZE                                	(EEPROM_LIMIT - EEPROM_BASE)

#define HW_ROM_BASE										ROM_BASE
#define	HW_ROM_LIMIT									ROM_LIMIT
#define	HW_ROM_SIZE									  ( ROM_LIMIT-ROM_BASE)

#define HW_RAM_BASE										RAM_BASE
#define	HW_RAM_LIMIT									RAM_LIMIT
#define	HW_RAM_SIZE									    (CHIP_RAM_LIMIT - CHIP_RAM_BASE)

#endif /* HWCONFIG_H_ */
