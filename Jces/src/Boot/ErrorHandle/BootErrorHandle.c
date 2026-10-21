#include "BootErrorHandle.h"
#include "Io.h"
#include "BootRam.h"
#include "Apdu.h"
#include <setjmp.h>

#pragma BOOT_ERROR_HANDLE_RAM_START

static TYPE_JUP_BUF *  	X_A	bootErrorHandle_ErrorHan;

BOOT_ERROR_HANDLE_RAM_END

void bootErrorHandle_Init(void)
{
	 bootErrorHandle_ErrorHan	= (TYPE_JUP_BUF * )0;
}

void bootErrorHandle_SetLongJmp(TYPE_JUP_BUF * errorHandle)
{
	 bootErrorHandle_ErrorHan = (TYPE_JUP_BUF *)errorHandle;
}

void bootErrorHandle_ThrowError(u16 errorcode)
{
	if(0 == bootErrorHandle_ErrorHan)
	{
		while(1);
	}
			
	apdu_SuspendAllInterrupts();
	longjmp((* bootErrorHandle_ErrorHan), errorcode);
}

