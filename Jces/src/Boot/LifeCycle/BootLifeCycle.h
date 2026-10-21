#ifndef __BOOT_LIFE_CYCLE_H_
#define __BOOT_LIFE_CYCLE_H_

#include "DataOperation.h"
#include "bootConfigProfile.h"

#define LIFE_CYCLE_READY			0x20
#define LIFE_CYCLE_INIT				0x40
#define LIFE_CYCLE_CREATE		  	0x60
#define LIFE_CYCLE_BLOCK			0x80
#define LIFE_CYCLE_MUTE		 	  	0xA0


#define  BLOCK_BIT_MASK						0x80
#define  PRIVATE_COMMAND_DISABLE_MASK		0x40

#if BOOT__LIFE_CYCLE
	extern  u8	 BootLifeCycle_GetLifeCycle(void);
	extern  void BootLifeCycle_SetLifeCycle(u8 lifeCycleValue)    SMALL_MODE;
	extern  void BootLifeCycle_BlockBootConfig(u8 configBit);
	extern  void BootLifeCycle_UnblockBootConfig(u8 configBit);
	extern	u8   BootLifeCycle_IsBlockBootConfig(u8 configBit);
	extern  u8 	 BootLifeCycle_IsCardInit(void);
#else
	#define  BootLifeCycle_GetLifeCycle()							0
	#define  BootLifeCycle_SetLifeCycle(lifeCycleValue)
	#define  BootLifeCycle_BlockBootConfig(configBit)
	#define  BootLifeCycle_UnblockBootConfig(configBit)
	#define	 BootLifeCycle_IsBlockBootConfig(configBit)				0
	#define  BootLifeCycle_IsCardInit()								0
#endif

#endif
