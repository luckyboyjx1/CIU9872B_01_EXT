#include "SysHal.h"
#include "SysConfig.h"
#include "chipHead.h"
#include "NvmHal.h"
#include "Apdu.h"
#include "SysPlatformConfigProfile.h"

#if HAL__SYSTEM_CONFIG

void reset_Init()
{
	return;
}

u8 reset_ReInit(u8 resetStauts)
{
	u8     D_A    isDeselect;
		isDeselect  = 0;
//		__enable_irq();
//		if(resetStauts != 0)
//		{
//			switch(resetStauts)
//			{
//				case SPI_REREST_FLAG :
//					 SpiResetFlag = 1;
//					break;
//				case CT_REREST_FLAG :
//					 //SpiResetFlag = 1;
//					break;
//				case CL_REREST_FLAG :
//					 isDeselect = 1;
//					break;
//				case VM_REREST_FLAG :
//					if(ProtocolWorkFlag & (_SPI_WORK_FLAG|_I2C_WORK_FLAG))
//					{
//						SpiResetFlag = 1;
//						I2CResetFlag = 1;
//					}
//					break;
//				case I2C_REREST_FLAG :
//					 I2CResetFlag = 1;
//					break;
//				default:
//					break;
//
//			}
//		}
		return isDeselect;
}

void ecpSysHal_Init()
{
}

u8 ecpSysHal_GetCpuPower(void)
{
	return 0;
}

u8 ecpSysHal_IsWarmReset(void)
{ 
	return 0;
}

u8 ecpSysHal_IsDirectConversion(void)
{
	return 1;
}

void ecpSysHal_SetCpuClk(u32 clk)
{

}

u16 ecpSysHal_GetCpuClk(void)
{
	return 0;
}

void ecpSysHal_RecCpuClk(void)
{
	
}

void ecpSysHal_SetIntBeforeRfWtx(void)
{
	return;
}

void ecpSysHal_SetIntAfterRfWtx(void)
{
	return;
}

u8 ecpSysHal_ChipConfigFun(PX8 buf, u8 infoType)    SMALL_MODE
{	
	u8  result	= false ;
	switch((infoType & 0x0F))
	{
		case 0x0F:
			switch(buf[3])
			{
				case 0x00:
					{
						buf[4] = 4;
						buf[5] = U32_SHR_U8(EEPROM_BASE & 0xFF000000, 24);
						buf[6] = U32_SHR_U8(EEPROM_BASE & 0x00FF0000, 16);
						buf[7] = U32_SHR_U8(EEPROM_BASE & 0x0000FF00, 8);
						buf[8] = U32t8(EEPROM_BASE & 0x000000FF);
						result	= true ;
					}
					break;
				case 0x01:
					{
						buf[4] = 1;
						buf[5] = sysConfig_GetBackUpPageNum();
						result	= true ;
					}
					break;
				case 0x02:
					{
						buf[4] = 2;
						buf[5] = U16_SHR_U8(EEPAGE_SIZE, 8);
						buf[6] = U16t8(EEPAGE_SIZE& 0x00FF);
						result	= true ;
					}
					break;
				default: break;
			}
			break;
		default:
			break;
	}

	return result;
}
u8 ecpSysHal_SuspendAllInterrupts(void)
{
	return 0;
}

u8 ecpSysHal_ResumeAllInterrupts(void)
{
	return 0;
}

u8 ecpSysHal_AssignProtocolPriority(void)
{
	return 0;
}

void ecpSysHal_ManageNvmRepair(void)
{

}

void ecpSysHal_ManageSetClockConfig(u8 type)
{

}

u8 ecpSysHal_ClearCard(void)
{
	return 0;
}
#endif
