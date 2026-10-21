/*
 * ecpDefine.c
 *	define how to translate point to memory and translate memory to point in simalator
 *  Created on: 2014Äê2ÔÂ28ÈÕ
 *      Author: zousonghua
 */

#include "ecpDefine.h"
#include "hal.h"
#include "hwConfig.h"
#include "SysConfig.h"

#ifdef HW_ALONE
#define isInRam(address)							0
#define isInE2P(address)							0
#define isInRom(address)							0
#define isValidAddress(addr) 			            0

u16 htoj16(u16 value)
{
	return ((LOW(value) << 8) | HIGH(value));
}
u32 htoj32(u32 x)
{
	return (((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) | ((x & 0x00ff0000) >> 8) | ((x & 0xff000000) >> 24));
}

#endif
PF8 memoryToPoint(u32 address)
{
	if(isInRam(address))
	{
		return (PF8) (&RAMSTACK[(address-CHIP_RAM_BASE)]);
	}
	if(isInRom(address))
	{
		return (PF8)(&ROMSTACK[(address-ROM_BASE)]);
	}
	if(isInE2P(address))
	{
		return (PF8)(&EEPROMSTACK[(address-EEPROM_BASE)]);
	}

	return (PF8) address;
}

u32 pointToMemory(PF8 point)
{
	if((point>=&RAMSTACK[0]) && (point <=&RAMSTACK[HW_RAM_SIZE ]))
	{
		return (u32)(&RAMSTACK[HW_RAM_SIZE ] - point);
	}
	if((point>=&ROMSTACK[0]) && (point <=&ROMSTACK[HW_ROM_SIZE ]))
	{
		return (u32)(&ROMSTACK[HW_ROM_SIZE ] - point);
	}
	if((point>=&EEPROMSTACK[0]) && (point <=&EEPROMSTACK[HW_EEPROM_SIZE ]))
	{
		return (u32)(&EEPROMSTACK[HW_EEPROM_SIZE ] - point);
	}

	return (u32) point;
}


