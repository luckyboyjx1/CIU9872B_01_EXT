#include "BootLifeCycle.h"
#include "MemBackUp.h"
#include "Apdu.h"
#include "MemOperation.h"
#include "SysConfig.h"
#include "MemOperationServe.h"
#include "boot.h"
#if BOOT__LIFE_CYCLE

u8 BootLifeCycle_GetLifeCycle(void)
{
	u8		D_A		lifeCycle;
	PN8				bootLifeCycleBase;
	
	PATCH_BOOT_LIFECYCLE_001();
	
#if NATIVE_CARD_UICC							 //modify by yangjian: native UICC card,return default Value:LIFE_CYCLE_CREATE
	 return LIFE_CYCLE_CREATE;
#else	
	
	bootLifeCycleBase	= MEMtPN8(sysConfig_GetBootLifeCycleBase());
	lifeCycle			= PN_R_U8(bootLifeCycleBase);

	if(U8_AND_U8(lifeCycle, PN_R_U8(PN8_A_CONST(bootLifeCycleBase, 1))) != 0)
	{
		faultDetected();
	}

	return lifeCycle;
#endif	
}

void BootLifeCycle_SetLifeCycle(u8 lifeCycleValue)    SMALL_MODE
{
	u8		D_A		i;
	u8		D_A		TransactionStarted;
	PN8		D_A		bootLifeCycleBase;
	
	PATCH_BOOT_LIFECYCLE_002();
	
	TransactionStarted	= 0;
	bootLifeCycleBase	= MEMtPN8(sysConfig_GetBootLifeCycleBase());

	if (!isInProgress())
	{
		memBeginTransaction(SINGLE_APDU_PROCESS);
		TransactionStarted = 1;
	}

	for(i = 0;i < 10; U8_INC(i))
	{
		nvmWriteU8Atomic(MEMtPN8(bootLifeCycleBase), lifeCycleValue);
		nvmWriteU8Atomic(MEMtPN8(PN8_A_CONST(bootLifeCycleBase, 1)), U32t8(U8_INV(lifeCycleValue)));
		memRefresh();

		if((PN_R_U8(bootLifeCycleBase) == lifeCycleValue) && (PN_R_U8(PN8_A_CONST(bootLifeCycleBase, 1)) == U32t8(U8_INV(lifeCycleValue))))
		{
			if (TransactionStarted)
			{
				memCommitTransaction();
			}
			return;
		}
	}

	memAbortTransaction(ABORT_NOT_ON_RESET);
	apdu_SuspendAllInterrupts();
	while(1);
}

void BootLifeCycle_BlockBootConfig(u8 configBit)
{
	PN8		D_A addr;
	
	addr	= MEMtPN8(sysConfig_GetBootKeyFlagBase());
	
	nvmWriteU8Atomic(addr, U8_OR_U8(PN_R_U8(addr), configBit));
	
	coscrc_CalcVerify(COSCRC_CALC);
}

void BootLifeCycle_UnblockBootConfig(u8 configBit)
{
	PN8		D_A addr;
	
	addr	= MEMtPN8(sysConfig_GetBootKeyFlagBase());
	
	nvmWriteU8Atomic(addr, U8_AND_U8(PN_R_U8(addr), U8_INV(configBit)));	
	
}

u8 BootLifeCycle_IsBlockBootConfig(u8 configBit)
{
	u8		D_A result;
	
	if(U8_AND_U8(sysConfig_GetBootKeyFlag(), configBit))
	{
		result	= true;
	}
	else
	{
		result	= false;
	}
	
	return result;
}

u8 BootLifeCycle_IsCardInit(void)
{
	u8	D_A	tempLifeCycle;

	tempLifeCycle = BootLifeCycle_GetLifeCycle();

	if(tempLifeCycle  < LIFE_CYCLE_INIT)
	{
		tempLifeCycle = 0;
	}
	else
	{
		tempLifeCycle = 1;
	}
	return tempLifeCycle;
}
#endif
