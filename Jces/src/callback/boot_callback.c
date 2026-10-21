/*
 * hal_callback.c
 *
 *  Created on: 2016Äê4ÔÂ10ÈÕ
 *      Author: huangzhigao
 */


#include "hal_callback.h"
#include "Io_14443TaL3.h"
#include "ecpDefine.h"

__attribute__((weak))   PN8 X_A	MAP;

__attribute__((weak))  u16 getPrivateData(PN8 apduBuffer,u8 type)
{
	return 0x9000;
}


__attribute__((weak)) PN8 GetUsedAddrNO(void)
{
	return NULL;
}


__attribute__((weak)) u16 MapTable_GetTopIndex(void)
{
	return 0;
}

__attribute__((weak)) memref JcvmConfig_GetSaveTransFlagAreaBase(void)
{
	return 0;
}

void initBootCallback(){
	halInitJavaBuffer();
#if HAL__IO__ISO14443_TA
	PhIso14443L3AtqaB1b8WholeByteL = &(phIso14443L3_control.atqa.b1b8.wholeByteL);
#endif
}



