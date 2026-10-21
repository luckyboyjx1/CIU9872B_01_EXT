/*
 * main.c
 *
 *  Created on: 2014Äê3ÔÂ17ÈÕ
 *      Author: zousonghua
 */

#include "hal.h"
#include "JcvmInit.h"
#include "Jcvm_Nvm.h"
#include "SysConfigTarget.h"
#include "mask.h"
#include "JcvmDataConfig.h"
#include "SysConfigData.h"
#include "target.h"
//extern  u8 BootOS_SecurityArea[BOOT_KEY_DATA_AREA_SIZE];
//void romInit()
//{
//	//ROM mask
//	memcpy(&ROMSTACK[ROM_MASK_OFFSET], rommask, ROMSIZE);
//	memcpy(&ROMSTACK[ROM_MASK_OFFSET + ROMSIZE], staticinit, EEPROM_IMAGE_SIZE);
//	//memcpy(&ROMSTACK[ROM_MASK_OFFSET + ROMSIZE + EEPROM_IMAGE_SIZE], BootOS_SecurityArea, BOOT_KEY_DATA_AREA_SIZE);
//}

void ramInit()
{
	memset(RAMSTACK, 0, CHIP_RAM_LIMIT-CHIP_RAM_BASE);
}

//void eepConfigInit()
//{
//	u32 StartAddr = 0;
//
//	//eep Config data
//	memcpy(&EEPROMSTACK[EEPROM_CONFIG_OFFSET], sysConfig, HAL_BOOT_CONFIG_END);
//	StartAddr = &EEPROMSTACK[EEPROM_CONFIG_OFFSET + HAL_BOOT_CONFIG_END + BOOT_CONFIG_RESERVE_SIZE];
//#if GP__AMENDMENT__C__FOR_MASK
//	//ADM C config
//	memcpy(StartAddr, bEECFG1, EECFG_SIZE);
//	memcpy((StartAddr + EECFG_SIZE), bEECFG2, EECFG_SIZE);
//	StartAddr += 0x500;
//#endif
//	memcpy(StartAddr, vm_config, (CARDINIT_CONFIG_BASE - VM_CONFIG_OFFSET_START));
//}

//void ramReset()
//{
//	memset(RAMSTACK, 0, RAM_LIMIT-RAM_BASE);
//}

#ifdef AUTO_MAKE
#include <windows.h>
long handleExceptionFilter(){
	return 1;
}
#define ErrorFilter() SetUnhandledExceptionFilter(handleExceptionFilter);
#else
#define ErrorFilter()
#endif

 int main(int argc, char* argv[]) {
	ErrorFilter()
	initCallback();
//	hw_eepromInit = eepConfigInit;
	hw_init(argc,argv);
	romInit();
	hw_setResetHandler();
	ramInit();
	os_init();
	return(-1);
}

