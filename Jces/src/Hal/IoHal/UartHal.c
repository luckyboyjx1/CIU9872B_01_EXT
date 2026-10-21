#include "UartHal.h"
#include "chipHead.h"
#include "Io.h"

#include "mhcard_iso7816.h"
#include "system_mhcard.h"

#if HAL__PLATFORM__SIMULATOR
	#include <uart/UartPort.h>
#endif
#if HAL__IO__ISO7816_T0

//#define		NO_PROCESS_616C			0
//#define		PROCESS_61				1
//#define		PROCESS_6C				2

//u8	X_A	process616c;
//u8	X_A	process616cLength;

//#pragma DATA("bootRamArea")
//volatile u8 g_Iso7816Reset;   

//#pragma DATA()
/** @brief  Table with Fi values see ISO 7816-3 
 **
 ** @detail
 **    	all possible Fi values see ISO 7816-3
 **		if a value = 0xFFFF => this place is reserved for future use (RFU)
 **/
static u16 CONST  uartHal_FiTable[16] =
{ 0xFFFF, // 0000
		372, // 0001
		0xFFFF, // 0010
		0xFFFF, // 0011
		0xFFFF, // 0100
		0xFFFF, // 0101
		0xFFFF, // 0110
		0xFFFF, // 0111   RFU = reserved for future use
		0xFFFF, // 1000   RFU
		512, // 1001
		0xFFFF, // 1010
		0xFFFF, // 1011
		0xFFFF, // 1100
		0xFFFF, // 1101
		0xFFFF, // 1110   RFU
		0xFFFF // 1111   RFU
		};
/** @brief  Table with Di values see ISO 7816-3 
 **
 ** @detail
 **   all possible Di values see ISO 7816-3 (DI:0111=64 comes from ETSI spec)
 **   if a value = 0xFF => this place is reserved for future use (RFU)
 **/
static  u8 CONST uartHal_DiTable[16] =
{ 0xFF, // 0000   RFU = reserved for future use
		1, // 0001
		2, // 0010
		4, // 0011
		8, // 0100
		16, // 0101
		32, // 0110
		64, // 0111
		12, // 1000
		0xFF, // 1001
		0xFF, // 1010   RFU
		0xFF, // 1011   RFU
		0xFF, // 1100   RFU
		0xFF, // 1101   RFU
		0xFF, // 1110   RFU
		0xFF // 1111   RFU
		};

extern volatile u32 g_3BSent;
		
__attribute__( ( section( "System_Section" ) ) )
volatile u8 g_Iso7816Reset; 

void ecpUart_Init(void)
{
	
    NVIC_DisableIRQ(SCIS_IRQn);
    /*the SCI module clock default open, in contact mode*/
    
//    SRAM->u32_3BSent = 0;
    /*
        3BWrote | BGT_EN   |
              X |      1   | 0x3B is not in SCI fifo. now the RST pin is high
              1 |      0   | 0x3B is in SCI fifo, is transmitting. we shall wait for send over.
              0 |      0   | 0x3B is not in SCI fifo. now the RST pin maybe low                               
    */
//    if ((SCI->CTRL & SCI_CTRL_BGT_INT_EN) == 0
//        /*&& SRAM->u32_3BWrote == 1*/)
//    {
////        SRAM->u32_3BSent = 1;
//        //Wait for 12 ETUS for 0x3b transmission to complete
//        SCI->BGTCTRL &= ~SCI_BGTCTRL_BGT_EN;//disable bgt
//        SCI->BGTDATA = 0x0C;
//        SCI->INTIO1 = SCI_INTIO_BGT_INT;//clear bgt int
//        SCI->BGTCTRL |= SCI_BGTCTRL_BGT_EN;
//        while(0 == (SCI->INTIO1 & SCI_INTIO_BGT_INT));
//    }
    
    SYSCTRL_ClockEnable(SYSCTRL_Periph_ISO7816, ENABLE);
	SYSCTRL_ClockEnable(SYSCTRL_Periph_GPIO, ENABLE);
	
	SYSCTRL_SoftReset(SYSCTRL_GPIO_RESET);
    SYSCTRL_SoftReset(SYSCTRL_ISO7816_RESET);
	
	SCI->MODHW = (SCI->MODHW & ~SCI_MODHW_RETRY_SEL_Msk) | SCI_MODHW_RETRY_5;
	SCI->CTRL |= SCI_CTRL_RETRY_EN;
	
	//set fi/di = 372
	SCI->ETUDATA = 372;
    
	SCI->CTRL |= (SCI_CTRL_TX_FIFO_CLR | SCI_CTRL_RX_FIFO_CLR);
	
    //BWT is the minimum delay between the leading edge of the last character of the block received
	//by the card and the leading edge of the first character of the next block transmitted by the card 0x1F is mean the BGT is 0x1F etu
	SCI->BGTDATA = 0x05;
	
	//CWT is the maximum delay between the leading edges of two consecutive characters in the block
	//0xA14 is mean the CWT is 0xA14 etu
	SCI->INTIO1 = SCI_INTIO_ALL;
	
    //SCI_ITConfig(SCI_CTRL_RX_INT_EN | SCI_CTRL_BGT_INT_EN | SCI_CTRL_EXTRST_INT_EN, ENABLE);
	//SCI_ITConfig(SCI_CTRL_EXTRST_INT_EN, ENABLE);
	SCI->CTRL |= SCI_CTRL_RX_INT_EN | SCI_CTRL_EXTRST_INT_EN | SCI_CTRL_BGT_INT_EN;
	
	GPIO->MODE_H = (GPIO->MODE_H & 0xFFFFFF00) | 0x0D;
    
    //SYSCFG->WAKE_SYS_CTRL |= SYSCTRL_WAKE_SYS_CTRL_ISO7816_Wk_EN;
    
    if ((GPIO->IOD >> 8) & 0x10)//RST is high
    {
        //SCI_BgtCwtEnable(SCI_BGTCTRL_BGT_EN, ENABLE); //enable bgt for send 0x3B
		SCI->BGTCTRL |= SCI_BGTCTRL_BGT_EN;
    }
    NVIC_EnableIRQ(SCIS_IRQn);
}

void ecpUart_T0Init(void)
{
	
}


void ecpUart_SendDirectTs(void)
{
	return;
}

void ecpUart_SendinverseTs(void)
{
	return;
}


void ecpUart_TsDelay(void)
{
	while((g_Iso7816Reset != UART_STATUS_WARMRESET) && (g_Iso7816Reset != UART_STATUS_COLDRESET))
	{
		
	}
}

void ecpUart_AtrDelay(void)
{
	
}

u8 ecpUart_DiscoverError(void)
{
	return false;
}


void ecpUart_SendRemaindAtr(PX8 atrBuf, u8 atrLen, u8 type)    SMALL_MODE
{
	ecpUart_TxMulBytes(atrBuf, atrLen, SEND_ATR);
}

u8 ecpUart_CheckFiDi(u8 FiDi)
{
	u8 Fi;
	u8 Di;
	
	
	Fi 			= U8_AND_U8(U8_SHR_U8(FiDi, 4), 0x0F);
	Di 			= U8_AND_U8(FiDi, 0x0F); 

	if(0xffff == uartHal_FiTable[Fi])
	{
		return 0;
	}
	
	if(0xff == uartHal_DiTable[Di])
	{
		return 0;
	}
	
	if((uartHal_FiTable[Fi] % uartHal_DiTable[Di]) != 0x00)
	{
		return 0;
	}
	
	if((Fi<2)||(9==Fi))
	{
		if((Fi<2)&&((0==Di) || ((Di>0x03)&&(Di<0x08)) || (Di>8)))
		{
			return 0;
		}
		if((9==Fi)&&((0==Di)||(Di>7)))
		{
			return 0;
		}		
		return 1;
	}
	else 
	{
		return 0;
	}

}

void ecpUart_SetFiDi(u8 FiDi)
{
	u8 Fi;
	u8 Di;	
	Fi 			= U8_AND_U8(U8_SHR_U8(FiDi, 4), 0x0F);
	Di 			= U8_AND_U8(FiDi, 0x0F); 
	
	SCI->ETUDATA = uartHal_FiTable[Fi] / uartHal_DiTable[Di];
	return ;
}

/*
void ecpUart_SetRecieveMode(void)
{

}
*/

void ecpUart_RxByte(PX8  buf)
{
	while (!(SCI->STAT & SCI_STAT_RX_FIFO_NONEMPTY));
	
    *buf = SCI->BUFHW;
}

//mixmode  the result of no dma of interface 7816 return always 1,others need look up the dma data register 
u8 ecpUart_getRxLen(void)
{
			return 1; 
}

void ecpUart_RxMulBytes(PR8 buf, u16 len, u8 flag)
{
	u16 i;
	
    for (i = 0; i < len; i++)
    {
        while (!(SCI->STAT & SCI_STAT_RX_FIFO_NONEMPTY));
        ((u8 *)buf)[i] = SCI->BUFHW;
    }
	SCI->INTIO1 = SCI_INTIO_RX_INT;		// must accroding to demo
}

/*
void ecpUart_SetTransmitMode(void)
{

}

void ecpUart_TxByte(PX8 byteData)
{

}
*/

void ecpUart_TxMulBytes(PR8 buf, u16 len, u8 flag)	SMALL_MODE
{
	uint32_t i;
    SCI->INTIO1 = SCI_INTIO_TX_INT;			// clear the interrupt flag of the TX
    for (i = 0; i < len; i++)
    {
        while (SCI->STAT & SCI_STAT_TX_FIFO_FULL);
        SCI->BUFHW = ((u8 *)buf)[i];
    }
	
	while(0 == (SCI->INTIO1 & SCI_INTIO_TX_INT));
}

void ecpUart_EnterSleepMode(void)
{
	//check sci rx fifo is not empty
	if (SCI->STAT & (SCI_STAT_RX_FIFO_NONEMPTY | SCI_STAT_TX_FIFO_NONEMPTY))
	{
		return;
	}
	
	NVIC_ClearPendingIRQ(GPIO_IRQn);
    NVIC_ClearPendingIRQ(SCIS_IRQn);

	SYSCFG->WAKE_SYS_CTRL |= SYSCTRL_WAKE_SYS_CTRL_ISO7816_Wk_EN;

    FCU->CFG |= BIT(15);//flash deepsleep
    SYSCFG->SYS_CON |= BIT(31);//flash deep sleep
    
    //SSC->TRM_CR1 = *(volatile uint32_t *)(0x03000020); //if softreset the trimcode will not -10 so need reload
    
    SCB->SCR = (SCB->SCR & ~(SCB_SCR_SLEEPDEEP_Msk|SCB_SCR_SLEEPONEXIT_Msk))| SCB_SCR_SLEEPDEEP_Msk;
    
    if (SYSCFG->CBCL_FLAG & SYSCTRL_CBCL_FLAG_CB_FLG)
    {
//        GPIO->CON = 0x5555;
//        GPIO->MODE_H = 0x0D;
//        GPIO->MODE_L = 0x00;
        
        SYSCTRL_LowPowerConfig(LP_RAM_MODE, SleepState_DeepSleep);
    }
    
    SYSCTRL_LowPowerConfig(LP_ROM_MODE, SleepState_ShutDown);
	
	//set rst pin wakeup in sleep mode
	GPIO->WAKEUP_EN |= 0x10;
	
	SCB->SCR |= BIT(2);
	
	__wfi();
}

void ecpUart_GuardTime(void)
{
	SCI->BGTDATA = sysConfig_GetGuardTime();
	SCI->BGTCTRL |= SCI_BGTCTRL_BGT_EN;	
	while ((SCI->STAT & SCI_STAT_BGT) == 0);
	SCI->STAT &= ~SCI_STAT_BGT;
	SCI->BGTCTRL &= ~SCI_BGTCTRL_BGT_EN;	
	return ;	
}

void ecpUart_StopTx(void)
{
	
}

u8 ecpUart_TxRxRunning(void)
{
	if (((SCI->STAT & SCI_STAT_RX_FIFO_NONEMPTY) == 0) && ((SCI->STAT & SCI_STAT_TX_FIFO_NONEMPTY) == 1))
	{
		return 0;
	}
//	//接收空闲、接收FIFO为空、发送空闲
//	//if(((SCISSR & SCISSR_RB) == 0) && ((SCIRXFCR & SCIRXFCR_rx_empty) == 0))
//	if(((SLCxx_UART->IOSTAT & IOSTAT_RBUSY) == 0x00) && ((SLCxx_UART->IOSTAT & IOSTAT_TBUSY) == 0x00))
//	{
//		return 0;
//	}
	return 1;
}
boolean ecpUart_IsReset(void)
{
	if((UART_STATUS_COLDRESET == g_Iso7816Reset) || (UART_STATUS_WARMRESET == g_Iso7816Reset))
	{
		//IFX_Imm_InterfaceActivationInfo &= ~IMM_IAI_ISO7816_ACTIVATED;
		//g_Iso7816Reset = UART_STATUS_INIT;
		return true;
	}
	else if (SCI->INTIO1 & SCI_INTIO_EXTRST_INT)
	{
		return true;
	}
	
	return false;
}

void ecpUart_ClearFifo(void)
{
//	u8 buf;
//	while((SCISSR&(1<<3))==1)
//	{
//		
//	}
//	while((SCIRXFCR&(1<<1))==0)
//	{
//		buf = (unsigned char)SCISBUF;
//	}
//	SCIRXFCR |=  0X01;
//    //clear 7816 buf data
////     ecpMem_Set(g_uartCmd,0x00,ISO7816UTIL_HEADER_LENGTH);
//    
    
}



void ecpUart_ClearResetFlag(void)
{
    g_Iso7816Reset = UART_STATUS_NONERESET;
}

boolean ecpUart_IsRisingState(void)
{
	return true;
}

void ecpUart_CloseInterrupt(void)
{

}

void ecpUart_OpenInterrupt(void)
{

}

void ecpUart_SetRxLength(u16 rxlen)
{
//	u16 tmpLen;

//	if(rxlen == 0x00)
//	{
//		return;
//	}
//	
//	if(rxlen > 5)
//	{
//		tmpLen = 5;
//	}
//	else
//	{
//		tmpLen = rxlen;
//	}
//	
//	//先全部清除
//	SCIRXFCR &= 0xF0FF;
//	//设置water_level
//	SCIRXFCR |= (tmpLen << 8);

//	//等待设置成功
//	while((SCIRXFCR & (tmpLen << 8)) != (tmpLen << 8));
	
	return;
}
void ecpUart_EnRxIrq(void)
{

}



#endif  //HAL__IO__ISO7816_T0

#if HAL__IO__ISO7816_T1

u16 ecpUartCalT1Crc(PX8 buf, u16 length)
{
	return 0;
}

void ecpUartT1_SetCrcCal(void)
{

}

void ecpUart_T1Init(void)
{

}

u16 ecpUART_RxBuffer(PR8 pData, u16 pDataSize, PX16 fullness, u16 cwt, u8 isEnterSleepMode)
{
	return 0;
}

u16 ecpUart_TxBuffer(PR8 pData, u16 pDataSize, u16 fullness, u8 txOptions)
{
	return 0;
}
#endif


//#if BOOTLOADER
//void Uart_RxIrqHandlerProcess(void)
//#else
void Uart_RxIrqHandlerProcess(void)
//#endif
{
	MixMode_UartReceive();
	
	SCI->INTIO1 |= SCI_INTIO_RX_INT;           //Safe code.
}

//#if BOOTLOADER
//void Uart_RstIrqHandlerProcess(void)
//#else
void Uart_RstIrqHandlerProcess(void)
//#endif
{   	
	//set fi/di = 372
	SCI->ETUDATA = 372;
    
    //SCI_ClearFifo(SCI_CTRL_TX_FIFO_CLR | SCI_CTRL_RX_FIFO_CLR);
	SCI->CTRL |= (SCI_CTRL_TX_FIFO_CLR | SCI_CTRL_RX_FIFO_CLR);
	
	g_Iso7816Reset = UART_STATUS_WARMRESET;
	
	SCI->INTIO1 |= SCI_INTIO_EXTRST_INT;//clear int.
	
	if (g_3BSent == 1)
	{
		//for card
		SYSCFG->RST_EN = SYSCTRL_GLB_RESET;
	}
	else
	{
		//for emulator
		rtnBackup(CT_REREST_FLAG);
	}
}

boolean ecpUart_WarnReset(void)
{
	if(g_Iso7816Reset == UART_STATUS_WARMRESET)
	{
			return true;
	}
	return false;
}

void ecpUart_ClrRxIrqStatus(void)
{

}

u8 wtxTimer_Tunning(void)
{
    return 0;
}

void GPIO_IRQHandler(void)
{

}

void UART_IRQHandler(void)
{

    if (SCI->INTIO1 & SCI_INTIO_EXTRST_INT
		&& SCI->CTRL & SCI_CTRL_EXTRST_INT_EN)
    {
		Uart_RstIrqHandlerProcess();	
    }	
	else if (SCI->INTIO1 & SCI_INTIO_TX_FIFO_EMPTY_INT
        && SCI->CTRL & SCI_CTRL_TX_FIFO_EMPTY_INT_EN)
    {	
        SCI->INTIO1 |= SCI_INTIO_TX_FIFO_EMPTY_INT;//clear int.
    }
    else if (SCI->INTIO1 & SCI_INTIO_TX_INT
        && SCI->CTRL & SCI_CTRL_TX_INT_EN)
    {
        SCI->INTIO1 |= SCI_INTIO_TX_INT;//clear int.        
    }
	else if (SCI->INTIO1 & SCI_INTIO_RX_INT 
        && SCI->CTRL & SCI_CTRL_RX_INT_EN)
    {
		//Uart_RxIrqHandlerProcess();
		
		SCI->INTIO1 |= SCI_INTIO_RX_INT;
    }
	else if (SCI->INTIO1 & SCI_INTIO_CWT_INT
		&& SCI->CTRL & SCI_CTRL_CWT_INT_EN)
	{
		io_WtxRequest();
		SCI->INTIO1 |= SCI_INTIO_CWT_INT;  
	}
    else if (SCI->INTIO1 & SCI_INTIO_BGT_INT
             && SCI->CTRL & SCI_CTRL_BGT_INT_EN)
    {
        if ( (g_Iso7816Reset == UART_STATUS_INIT) || (g_Iso7816Reset == UART_STATUS_NONERESET) )
        {
            SCI->BUFHW = 0x3B;
            g_Iso7816Reset = UART_STATUS_COLDRESET;
            SCI->INTIO1 = SCI_INTIO_BGT_INT;
        }
		
		SCI->CTRL &= ~SCI_CTRL_BGT_INT_EN;
	}
	
    NVIC_ClearPendingIRQ(SCIS_IRQn);
}

