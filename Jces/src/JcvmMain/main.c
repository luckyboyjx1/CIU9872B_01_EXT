
#include "JcvmInit.h"
#include "chipHead.h"
#include "patch.h"
#include "FunEntryConfig.h"
#if BOOTLOADER
//__attribute__( ( section( "bootRamArea" ) ) )
#pragma  arm section rwdata "bootRamArea",zidata = "bootRamArea"   //don't genarate data in hex file
FunctionReadEntry  RealEntry;
#endif
int main()
{
//	PATCH_MAIN_001();	
//	clkInit(0,0);
//	__disable_irq();      //THD89 Only        //THD89 clkInit
//	patch_initBeforAts();		
//	DeselectFlag = 0;
//	IsColdResetFlag = 1;   // No register mark the reset type so define a flag
//	//BackupPC();
//	
//	__enable_irq();  
	os_init();
	return 0;
}

