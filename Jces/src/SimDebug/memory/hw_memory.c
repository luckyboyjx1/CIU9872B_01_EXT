/*
 * hw_memory.c
 *
 *  Created on: 2015年1月30日
 *      Author: mowenpan
 */

#include "stdio.h"
#include "hwConfig.h"
#include "cardAPI.h"
#include "hal.h"

#define HW_PAGE_SIZE		256


FILE * g_hw_outHexFile;



static void eepromInitByHex(FILE *initHexFile)
{
	u8 buf[HW_PAGE_SIZE];
	u32 size;
	u32 offset = 0;

	while (offset<HW_EEPROM_SIZE)
	{
		size = fread(buf, HW_PAGE_SIZE, 1, initHexFile);

		if (size > 0)
		{
			memcpy(EEPROMSTACK + offset, buf, HW_PAGE_SIZE);
			offset += HW_PAGE_SIZE;
		}
		else
		{
			break;
		}
	}
}


void hw_nvmWrite(PN8 desc, PM8 src, u16 length)
{
#if EN_EEPSAVE
	u32 offset;

	desc = pointToMemory(desc);

	offset = (u32)(desc - EEPROM_BASE);
	fseek(g_hw_outHexFile, offset, SEEK_SET);
	fwrite(src, length, 1, g_hw_outHexFile);
#endif
}

void hw_memoryInit(char *initHex, char *outHex)
{
	FILE *initHexFile;

	if (initHex == NULL) {
			initHexFile = NULL;
		} else {
			initHexFile = fopen(initHex, "rb+");
		}

		if (outHex == NULL) {
			outHex = "eeprom.txt";
		}

		if (initHexFile == NULL) {//设置则读，不设不读
			//hw_eepromInit();
		} else {
			eepromInitByHex(initHexFile);
		}
		//存在则写，不存在不写
		if (g_hw_outHexFile == NULL) {
			g_hw_outHexFile = fopen(outHex, "wb+");
			fwrite(&EEPROMSTACK[0], HW_EEPROM_SIZE, 1, g_hw_outHexFile);
			fflush(g_hw_outHexFile);
		}
}

void hw_saveEepromToHex()
{
	rewind(g_hw_outHexFile);
	fwrite(&EEPROMSTACK[0], HW_EEPROM_SIZE, 1, g_hw_outHexFile);
}
