#include "SysConfig.h"
#include "SysHal.h"
#include "SysConfigData.h"
#include "SysConfigHalRam.h"
#include "SysConfigProfile.h"
#include "SysConfigTarget.h"

#if HAL__SYSTEM_CONFIG

#pragma SYS_CONFIG_RAM_START

u8		X_A		r_memEepromBuffer[EEPROM_BUFFER_SIZE + TEMP_ARR_ALIGN_SIZE] ;
u8		X_A		bTempArr[TEMP_ARR_BUF_SIZE];

SYS_CONFIG_RAM_END

void sysConfig_Init(void)
{
	ecpSysHal_Init();
}

#if HAL__SYSTEM_CONFIG

//u32 sysConfig_GetRsaKeyOperation()
//{
//	PN8  D_A  rsaOperationAddr;
//	u32  D_A  result;
//	rsaOperationAddr = MEMtPN8(&sysConfigData[CLK_CONFIG_RSA_OPERATION]);
//	switch(CLK_CONFIG_RSA_OPERATION_LENGTH)
//	{
//	    case 1:
//	    	result = PN_R_U8(rsaOperationAddr);
//		    break;
//	    case 2:
//	    	result = PN_R_U16(rsaOperationAddr);
//		    break;
//	    case 3:
//	    case 4:
//	    	result = PN_R_U32(rsaOperationAddr);
//		    break;
//	    default:
//	    	result = 0;
//		    break;
//	}
//	return  result;
//}


//u32 sysConfig_GetRsaKeyGenClk()
//{
//	PN8  D_A  rsaAddr;
//	u32  D_A  result;
//	rsaAddr = MEMtPN8(&sysConfigData[CLK_CONFIG_RSA_KEYGEN]);
//	switch(CLK_CONFIG_RSA_KEYGEN_LENGTH)
//	{
//	    case 1:
//	    	result = PN_R_U8(rsaAddr);
//		    break;
//	    case 2:
//	    	result = PN_R_U16(rsaAddr);
//		    break;
//	    case 3:
//	    case 4:
//	    	result = PN_R_U32(rsaAddr);
//		    break;
//	    default:
//	    	result = 0;
//		    break;
//	}
//	return  result;
//}

u32 sysConfig_GetCpuClkCb(void)
{
	PN8  D_A  clkAddr;
	u32  D_A  result;
	clkAddr = MEMtPN8(sysConfig_GetClkConfigCbBase());
	switch(CLK_CONFIG_CB_LENGTH)
	{
	    case 1:
	    	result = PN_R_U8(clkAddr);
		    break;
	    case 2:
	    	result = PN_R_U16(clkAddr);
		    break;
	    case 3:
	    case 4:
	    	result = PN_R_U32(clkAddr);
		    break;
	    default:
	    	result = 0;
		    break;
	}
	return  result;
}

u32 sysConfig_GetCpuClkRf(void)
{
	PN8  D_A  clkrfAddr;
	u32  D_A  result;
	clkrfAddr = MEMtPN8(sysConfig_GetClkConfigClBase());
	switch(CLK_CONFIG_CL_LENGTH)
	{
	    case 1:
	    	result = PN_R_U8(clkrfAddr);
		    break;
	    case 2:
	    	result = PN_R_U16(clkrfAddr);
		    break;
	    case 3:
	    case 4:
	    	result = PN_R_U32(clkrfAddr);
		    break;
	    default:
	    	result = 0;
		    break;
	}
	return  result;
}

u32 sysConfig_GetCpuClkRfCom(void)
{
	PN8  D_A  clkRfComAddr;
	u32  D_A  result;
	clkRfComAddr = MEMtPN8(sysConfig_GetClkConfigClComBase());
	switch(CLK_CONFIG_CL_COM_LENGTH)
	{
	    case 1:
	    	result = PN_R_U8(clkRfComAddr);
		    break;
	    case 2:
	    	result = PN_R_U16(clkRfComAddr);
		    break;
	    case 3:
	    case 4:
	    	result = PN_R_U32(clkRfComAddr);
		    break;
	    default:
	    	result = 0;
		    break;
	}
	return  result;
}
//u32 sysConfig_GetRfEeProgramClk(void)
//{
//	PN8  I_A  clkRfE2pAddr;
//	u32  I_A  result;
//	clkRfE2pAddr = MEMtPN8(sysConfig_GetClkConfigClEepromProgramBase());
//	switch(CLK_CONFIG_EEPROM_OPERATION_LENGTH)
//	{
//	    case 1:
//	    	result = PN_R_U8(clkRfE2pAddr);
//		    break;
//	    case 2:
//	    	result = PN_R_U16(clkRfE2pAddr);
//		    break;
//	    case 3:
//	    case 4:
//	    	result = PN_R_U32(clkRfE2pAddr);
//		    break;
//	    default:
//	    	result = 0;
//		    break;
//	}
//	return  result;
//}
#endif



#endif


