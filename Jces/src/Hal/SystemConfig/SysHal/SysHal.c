#include "SysHal.h"
#include "SysConfig.h"
#include "chipHead.h"
#include "NvmHal.h"
#include "Io_SPI.h"
#include "Io_I2C.h"
#include "Io.h"
#include "chipHead.h"
#include "MixMode.h"
#include "UartHal.h" 

#include "system_mhcard.h"
#if HAL__SYSTEM_CONFIG


volatile u32 g_3BSent;
volatile u32 g_coldRst;

//modify by xuxiaojun@20221108: HAL__IO__SPI和HAL__IO__I2C关闭时编译不过问题
void reset_Init()
{
	//0x20000054 = 1 : firmware sent 0x3B
	g_3BSent = *(volatile uint32_t *)0x20000054;
	g_coldRst = *(volatile uint32_t *)0x500030F0;
	
	
	if (g_coldRst == 1)
	{
		g_Iso7816Reset = UART_STATUS_INIT;
	}
	
    /*
        3BWrote | BGT_EN   |
              X |      1   | 0x3B is not in SCI fifo. now the RST pin is high
              1 |      0   | 0x3B is in SCI fifo, is transmitting. we shall wait for send over.
              0 |      0   | 0x3B is not in SCI fifo. now the RST pin maybe low                               
    */
	if ((SCI->CTRL & SCI_CTRL_BGT_INT_EN) == 0
        && g_3BSent == 1)
	{
		SCI->BGTCTRL &= ~SCI_BGTCTRL_BGT_EN;//disable bgt
        SCI->BGTDATA = 0x0C;
        SCI->INTIO1 = SCI_INTIO_BGT_INT;//clear bgt int

        SCI->BGTCTRL |= SCI_BGTCTRL_BGT_EN;
        while(0 == (SCI->INTIO1 & SCI_INTIO_BGT_INT));
		g_Iso7816Reset = UART_STATUS_COLDRESET;
	}

#if HAL__IO__I2C	
	I2CResetFlag = I2C_STATUS_INIT;
	g_I2cIsReceiveApdu = 0;	
#endif
	
#if HAL__IO__SPI	
    SpiResetFlag = SPI_STATUS_INIT;
	g_SpiIsReceiveApdu = 0;
#endif
}

u8 reset_ReInit(u8 resetStauts)
{
	u8     D_A    isDeselect;
	isDeselect  = 0;
//	__enable_irq();  
	if(resetStauts != 0)
	{
		switch(resetStauts)
		{
			case SPI_REREST_FLAG : 
				 //SpiResetFlag = 1;
				break;
			case CT_REREST_FLAG : 
				 //SpiResetFlag = 1;
				break;
			case CL_REREST_FLAG : 
				 isDeselect = 1;
				break;
			case VM_REREST_FLAG : 
		#if (HAL__IO__I2C || HAL__IO__SPI)
//				if(ProtocolWorkFlag & (_SPI_WORK_FLAG|_I2C_WORK_FLAG))
//				{
//					SpiResetFlag = 1;
//					I2CResetFlag = 1;
//				}
		#endif
				break;
			case I2C_REREST_FLAG : 
				 //I2CResetFlag = 1;
				break;
			default:
				break;
				
		}
	}
	return isDeselect;
}

void ecpSysHal_Init()
{
	TrngCacheInit();
}

u8 ecpSysHal_GetCpuPower(void)
{

	return CONTACT_BASE_POWER;
}

u8 ecpSysHal_IsWarmReset(void)
{
	if (g_coldRst == 1) //cold reset
    {
		g_coldRst = 0;
        return 0;
    }
	else
	{
		return 1;
	}
}

u8 ecpSysHal_IsDirectConversion(void)
{
	return 1;
}

void ecpSysHal_SetCpuClk(u32 clk)
{
	SYSCFG->CLK_CFG = (SYSCFG->CLK_CFG & ~SYSCTRL_HCLK_SEL_Mask) | (clk << SYSCTRL_HCLK_SEL_Pos);
}

u16 ecpSysHal_GetCpuClk(void)
{
	return 0;
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
#define CHIP_IDDATA_LEN          						0x20
	u8  result	= false ;
	switch((infoType & 0x0F))
	{
		case 0x01:
				buf[4] = 4;
				memcpy(&buf[5], (u8*)0x1FFFFE20, 4);
				result	= true ;
			break;
			
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
//		case 0x08:
//				buf[4] = CHIP_IDDATA_LEN;
//				HED_GetFactoryCode(&buf[5]);
//				//memcpy(MEMtPX8(&buf[5]), (u8*)(&pIfxMailboxArea->ifxMbxShadowPage2.cimData), CHIP_IDDATA_LEN);
//				result	= true ;
//			break;
		default:
			break;
	}

	return result;
}
__attribute__( ( section( "seInfoDataArea" ) ) )

u8 CONST seInfo[JC_SE_INFO_SIZE]  = {
	0x00
};
__attribute__( ( section( "CRCDataArea" ) ) )

u8 CONST CRCData[JC_CRC_SIZE]  = {
	0x00
};

u8 ecpSysHal_SeInfoFun(PX8 buf, u16 desOff,u16 datalen ,u8 infoType)
{
#define READ_DATA       0x00
#define WRITE_DATA      0x01
#define CLEAR_DATA      0xFF
	u8 result;
	
	result=0x00;
	if (desOff + datalen > JC_SE_INFO_SIZE)
	{
		
		return 0x01;
	}
	switch((infoType))
	{
		case READ_DATA:
			memCopyAtomic((u8*)buf, (u8*)(JC_SE_INFO_SATRT + desOff), datalen);
			result = 0x00;
			break;
		case WRITE_DATA:
			memCopyAtomic((u8*)(JC_SE_INFO_SATRT + desOff), (u8*)buf, datalen);
			result = 0x00;	
			break;
		case CLEAR_DATA:
			memFillAtomic((u8*)JC_SE_INFO_SATRT, 0x00 , JC_SE_INFO_SIZE);
			result = 0x00;
			break;
		
		default:
			
			break;
	}
	return result; 
	
}

u8 ecpSysHal_SuspendAllInterrupts(void)
{
	//__disable_irq();
	return 0;
}

u8 ecpSysHal_ResumeAllInterrupts(void)
{
//	__enable_irq();
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
	u32 		u32addr;
	u32 		BOOT_PARAM_VALID_VALUE ;
	u8   		X_A   	ramBuffer[HAL_EEPAGE_SIZE];
	PN8			align_des;
	u16			ramBufferoffset;
	u8			result;
	
	u32addr = 0x0087FFFC;
	BOOT_PARAM_VALID_VALUE = 0x5A5AA5A5;
	
	align_des 			= MEMtPN8(U32_AND_U32(u32addr, NOR_HAL_EEPAGE_MASK));
	ramBufferoffset				= U16_AND_U16(u32addr, HAL_EEPAGE_MASK);
	
	ecpMem_Copy(ramBuffer, align_des, HAL_EEPAGE_SIZE);
	ecpMem_Copy(PX8_A_U16(MEMtPX8(ramBuffer), ramBufferoffset), MEMtPN8(&BOOT_PARAM_VALID_VALUE), 4);
	
	erasePageWithCheck(align_des);
	result = ProgPageWithCheck(ramBuffer, align_des);
	if (result == CHECK_WRITE_NOK)
	{
		while(1);
	}		
	
	return 0;
}

void ecpSysHal_SystemReset(u8 resetFlag)
{
//	rtnBackup(resetFlag);
	
}

#endif
