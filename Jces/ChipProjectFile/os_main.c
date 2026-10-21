
#include "global_variable.h"
#include "FunEntryConfig.h"
#include "ecpOperation.h"
#include "FunImp.h"

//#pragma arm section rwdata="os_main_Section",zidata="os_main_Section"

#pragma CODE("os_main_Section")

#define BOOT_OS_ENTRY			0xA59A75A7


//void RstInit(void)
//{
//#if BOOTLOADER
//	PN32			D_A			addrCheck;
//	PN32			D_A			addr;
//	u32				D_A			osEntry;
//
//	#define BOOT_OS_ENTRY			0xA59A75A7
//
//	addr = (PN32)CHIP_CONFIG_START;
//
//	addrCheck = (PN32)((u8 *)addr + sizeof(RealEntry));
//
//	osEntry = sizeof(RealEntry) >> 2;
//	for(; osEntry != 0 ; osEntry--)
//	{
//		if((*addr == 0)||(*addr == 0xFFFFFFFF)|| (*addr != *addrCheck))
//		{
//			osEntry = BOOT_OS_ENTRY;
//			break;
//		}
//		addr ++;
//		addrCheck ++;
//	}
//
//	if( osEntry == BOOT_OS_ENTRY)
//	{
//		BootLoader_Cpy(MEMtPX8(&RealEntry), MEMtPM8(BOOT_CONFIG_START), sizeof(RealEntry));
//	}
//	else
//	{
//		BootLoader_Cpy(MEMtPX8(&RealEntry), MEMtPM8(CHIP_CONFIG_START), sizeof(RealEntry));
//	}
//
//	RealEntry.RstInitFunc();
//#else
//	RstInitProcess();
//#endif
//}

int os_main() 
//int main() 
{
	PN32			D_A			addr;
	u32				D_A			off;
#if BOOTLOADER	
	PN32			D_A			addrCheck;

	u32				D_A			osEntry;	
	
	
	addr = (PN32)CHIP_CONFIG_START;
	
	addrCheck = (PN32)((u8 *)addr + sizeof(RealEntry));	
	
	osEntry = sizeof(RealEntry) >> 2;
	for(; osEntry != 0 ; osEntry--)
	{
		if((*addr == 0)||(*addr == 0xFFFFFFFF)|| (*addr != *addrCheck))
		{
			osEntry = BOOT_OS_ENTRY;
			break;
		}
		addr ++;
		addrCheck ++;
	}
	
	if( osEntry == BOOT_OS_ENTRY)
	{
		BootLoader_Cpy(MEMtPX8(&RealEntry), MEMtPM8(BOOT_CONFIG_START), sizeof(RealEntry));
	}
	else
	{
		BootLoader_Cpy(MEMtPX8(&RealEntry), MEMtPM8(CHIP_CONFIG_START), sizeof(RealEntry));
	}

	RealEntry.resetInit();

	//clear ram
//	addr = (u32*)COS_RAM_DATA_START;
//	for(off=0; off< (COS_RAM_DATA_SIZE / 4); off++)
//	{
//		*(addr + off) = 0x00;
//	}

	RealEntry.appMain();

//	if( osEntry == BOOT_OS_ENTRY)
//	{
//		FuncBootEntry.C_MainAPI();
//	}
//	else
//	{
//		FuncEntry.C_MainAPI();
//	}
#else
//	PN32			D_A			addr;
//	u32				D_A			off;

//	//clear ram
//	addr = (u32*)(RAM_GLOABSS_START + 0x100);
//	for(off=0; off< (RAM_GLOABSS_SIZE / 4); off++)
//	{
//		*(addr + off) = 0x00;
//	}
//	
	resetInit_Process();

	//appMain_Process();

	__main();
#endif
	return 0;
}

#pragma CODE()
