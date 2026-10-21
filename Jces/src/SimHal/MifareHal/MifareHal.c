#include "MifareHal.h"
#include "chipHead.h"

#if HAL__IO__MIFARE

u8 ecpMifare_EnablePer(u8 activeMDF, u8 activeMFP)
{
	return 0;
}

void ecpMifare_Enable(void)
{

}

u8 ecpMifare_Active(PX8 mifarePara)
{
	return 0;
}

u8 ecpMifare_GetRats0(void)
{
	return 0;
}

u8 ecpMifare_GetRats1(void)
{
	return 0;
}

u16 ecpMifare_GetAtqa(void)
{
	return 0;
}

u8 ecpMifare_GetSakIncomplete(void)
{
	return 0;
}

u8 ecpMifare_GetSakcomplete(void)
{
	return 0;
}

u8 ecpMifare_Read(u8 blockNum, PM8 EncKey)
{
	return 0;
}

u8 ecpMifare_Write(u8 blockNum, PM8 EncKey)
{
	return 0;
}

#endif
