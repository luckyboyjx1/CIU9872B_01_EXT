//#include "core_cm0.h"
#include "SpiHal.h"
#include "DataOperation.h"
#include "NvmHal.h"
#include "chipHead.h"
#include "Io_SPI.h"
#include "bootPlaformConfigProfile.h"
//#include "dma.h"
//#include "Chip.h"
//#include "CIU9872B_01_V4_Hal_Iocontrol.h"

#if HAL__IO__SPI

//SPI_HandleTypeDef hSPIStVar;
DMA_HandleTypeDef hDMA_RxStVar,hDMA_TxStVar;

volatile u8 g_TxRxType;
u8		spiInitFlag = 0;

//extern volatile u16  Spi_PrivTxLength;


u8 SpiHal_TxRx_Buffer[SPI_TXRX_Buffer_Size];

	
static void SpiHal_ConfigIOMux(void)
{
	IOCTRLOD &= ((~BIT5) & (~BIT6) & (~BIT7) & (~BIT8));//set io5\6\7\8 as push-pull mode	
	
	IOCTRLDS &= ((~BIT5) & (~BIT6) & (~BIT7) & (~BIT8));//set io5\6\7\8 as small driver
	
	IOCTRLPUEN |= BIT5 ;//set io5 as pull up
	
	IOCTRLPDEN |= (BIT8 | BIT6 | BIT7) ;	//set io6\7\8 as pull down	根据比亚迪的要求，BIT8(MISO)初始化必须为低电平；其他引脚按照SLI97的方式处理，MOSI\CLK初始化为低电平，CS初始化为高电平
	
	IOCTRLMODE |= (3<<10 | 3<<12 | 3<<14 | 3<<16);//set io5\6\7\8 as Alternal function mode
	
	IOCTRLAF1 = (2<<20 | 2<<24 | 4<<28);	//set io5\6\7 as SPI_CS\SPI_CLK\SPI_MOSI
	
	IOCTRLAF2 = 4;	//set io8 as SPI_MISO
	
	IOCTRLCFGEN = 0x59AF6E01;	//enable config

}

/*
void SpiHal_RegInit(void)
{
	SPI0->SPICTRL3 &= ~0x10;  //disenable spi
	SPI0->SPICTRL0=0x80;      //SSN pull up int enble
	SPI0->SPICTRL1=(sysConfig_GetSPIMODE()&0x03)<<1;      //Mode 0 1 2 3
	//SPI0->SPICTRL1=0x06;      //Mode 0 1 2 3
	SPI0->SPICTRL2=0x88;      //default.
	SPI0->SPICTRL3 =0x40;     //enble ssn debouncce
	SPI0->SPICTRL3 |=0x0c;   ////clear rx tx fifo
	SPI0->SPICTRL3 |=0x300;  //clear ssn down and up flag;
	PAD->PADIO_PU_0 = 0x0;
	SPI0->SPICTRL1|=0xC0;     //open tx rx
}
*/

u8  ecpSpi_isProcess(void)
{
	if((SYSWKPCON>>31)&0x1)
	{
		return 1;	//检测到SSN下降沿，返回1，不进入睡眠
	}
	else
		return 0;	//没检测到SSN下降沿，返回0，进入睡眠
}

/**
  * @brief  init SPI DMA RX\TX channel param 
  * @param  None  
  * @return None
  */
void SpiHal_DMA_StrucInit(void)
{
	/*1.init DMA RX HANDLE*/
	hDMA_RxStVar.Init.Channel = SPI_RX_CHANNEL;	//DMA channel 0
	hDMA_RxStVar.Init.Priority = PRIORITY_FIX;	//DMA Channel priority = fix
	hDMA_RxStVar.Init.TransBeatType = TRANSBEAT_NORM;	//trans type = Normal
	hDMA_RxStVar.Init.TransModeType = TRANSMODE_NEED;	//trans mode = Need
	hDMA_RxStVar.Init.BlockSizeUnit = BLOCKSIZE_0;	//DMAC_CA0->BC = 0
	hDMA_RxStVar.Init.DataWidth = DATA_BYTE;	//set 1 block = 1 byte
	hDMA_RxStVar.Init.Direction = PERIPH_TO_MEMERY;	//
	/* init DMA RX channel para*/
	HAL_DMA_Init(&hDMA_RxStVar);
	
	/*2.init DMA TX HANDLE*/
	hDMA_TxStVar.Init.Channel = SPI_TX_CHANNEL;
	hDMA_TxStVar.Init.Priority = PRIORITY_FIX;
	hDMA_TxStVar.Init.TransBeatType = TRANSBEAT_NORM;
	hDMA_TxStVar.Init.TransModeType = TRANSMODE_NEED;
	hDMA_TxStVar.Init.BlockSizeUnit = BLOCKSIZE_0;
	hDMA_TxStVar.Init.DataWidth = DATA_BYTE;
	hDMA_TxStVar.Init.Direction = MEMERY_TO_PERIPH;

	/* init DMA TX channel para*/
	HAL_DMA_Init(&hDMA_TxStVar);
	
}
#if !HAL_IO_SPI_MASTER

/**
  *@brief	SPI slave init.
  *@param	clkMode:Define clock phase plarity. 
  *@retval	None.  				 
  */
void SpiHal_SlaveInit(void)
{
	u8 mode;
	//sysConfig_SetCpuClkCb();
	
	SpiHal_ConfigIOMux();	//config io

	__HAL_OPEN_SSPI_CLKEN();	//enable spi 
	
	/*Init SPI IP accord to Configuration Structure */
	SPICON = 0;
	
	mode = sysConfig_GetChipSpiMode();
	SPICON |= (mode&0x03);
	
	/*set TxChOn=1*/
	//需要先打开TX 通道，否则在主主设备发送第一条APDU命令后，会一直读取PCB（间隔100微秒左右），如果SE在准备好数据后，
	//再打开TX 通道，此时主设备可能已经读取了几位的数据，SE 再把DMA 的数据放入FIFO，此时会产生错位，导致数据出错
	//先打开TX 通道不会影响发送数据，只是先让发送模块工作起来，按照字节的每一位发送出去，正确的计数。
	__HAL_SPI_OPEN_TxChannel();
	
	/*init work mode*/
	/*Clear interrupt Flag*/
	__HAL_DISABLE_SSPI_INTEN();
	__HAL_CLR_SSPI_INTFLAG();
	__HAL_CLR_SPI_PENDIRQ();
		
	__HAL_SPI_DONE_INTEN();
	__HAL_ENABLE_SPI_IRQEN();
	
	spiInitFlag = 1;
	
	SpiHal_DMA_StrucInit();

}


//length is retransmission length
void SpiHal_rxReady(u16 length)
{
	//hSPIStVar.RxDataSize = length;
	HAL_StatusTypeDef t_retStatus;

	//20230210李磊：为防止上条指令在处理中主设备又发送来一条指令而导致时序错乱，修改为在接收到一条APDU指令后将中断关闭，下次准备接收或发送前打开中断(刘奇新协议栈测试脚本SPI047)
	__HAL_ENABLE_SPI_IRQEN();
	/*close Tx channel*/
//	if(REG_BIT_IS_SET(SPICHCON, TxChOn))
//	{
//		__HAL_SPI_CLOSE_TxChannel();
//	}
	/*disable DMA TX*/
	__HAL_SPI_CLOSEDMA_TXLVL();	
	__HAL_SPI_CLOSEDMA_RXLVL();
	
	/*clear rx\tx fifo*/
	__HAL_SPI_CLR_RXTXFIFO();	

	g_TxRxType = SPIHAL_TYPE_RX;

	/*Start DMA Rx transfer*/
	HAL_DMA_Start(&hDMA_RxStVar, (u32)SpiHal_TxRx_Buffer,(unsigned int)SPIRXDATAADDR, SPI_TXRX_Buffer_Size);
	//HAL_DMA_Start(&hDMA_TxStVar,(unsigned int)SPITXDATAADDR, (u32)SpiHal_TxRx_Buffer, length);	//for GW2.0 protocol resend
	
	/*set DMA Rx level =0byte;*/
	SPIDMARDLR = 0x00;
	
	/*set RxChOn=1*/
	__HAL_SPI_OPEN_RxChannel();
	
	/*set DMA Rx Enable*/
	__HAL_SPI_ENDMA_RXLEVEL();
	
}


u16 SpiHal_txData(u8* txData, u16 txLength)
{
	 HAL_StatusTypeDef t_retStatus;
	
	ecpMem_Copy(SpiHal_TxRx_Buffer,txData,txLength);
	
	//20230210李磊：为防止上条指令在处理中主设备又发送来一条指令而导致时序错乱，修改为在接收到一条APDU指令后将中断关闭，下次准备接收或发送前打开中断(刘奇新协议栈测试脚本SPI047)
	__HAL_ENABLE_SPI_IRQEN();

	/*close Rx channel*/
	if(REG_BIT_IS_SET(SPICHCON, RxChOn))
	{
		__HAL_SPI_CLOSE_RxChannel();
	}
	/*disable DMA RX*/
	__HAL_SPI_CLOSEDMA_RXLVL();
	
	/*clear rx\tx fifo*/
	//__HAL_SPI_CLR_RXTXFIFO();
	
	g_TxRxType = SPIHAL_TYPE_TX; 

	/*Start DMA Tx transfer*/
	t_retStatus = HAL_DMA_Start(&hDMA_TxStVar, (unsigned int)SPITXDATAADDR, (u32)SpiHal_TxRx_Buffer, txLength);

	/*set DMA Tx level =15byte;*/
	SPIDMATDLR = 0x0F;
	
	/*set TxChOn=1*/
	__HAL_SPI_OPEN_TxChannel();
	
	/*set DMA Tx Enable*/
	__HAL_SPI_ENDMA_TXLEVEL();
	
	return HAL_OK;
}

void SpiHal_StopTxData(void)
{
	//HAL_DMA_Start(&hDMA_TxStVar, (unsigned int)SPITXDATAADDR, (unsigned int)SpiHal_TxRx_Buffer, 0);
		
	//20230210李磊：为防止上条指令在处理中主设备又发送来一条指令而导致时序错乱，修改为在接收到一条APDU指令后将中断关闭，下次准备接收或发送前打开中断(刘奇新协议栈测试脚本SPI047)
	__HAL_DISABLE_SPI_IRQEN();
		
	//__HAL_SPI_CLOSE_TxChannel();
	__HAL_SPI_CLOSEDMA_TXLVL();
	
	g_TxRxType = SPIHAL_TYPE_NONE;
}

void SpiHal_StopRxData(void)
{
	//20230210李磊：为防止上条指令在处理中主设备又发送来一条指令而导致时序错乱，修改为在接收到一条APDU指令后将中断关闭，下次准备接收或发送前打开中断(刘奇新协议栈测试脚本SPI047)
	__HAL_DISABLE_SPI_IRQEN();
	
	__HAL_SPI_CLOSE_RxChannel();
	__HAL_SPI_CLOSEDMA_RXLVL();
	
	g_TxRxType = SPIHAL_TYPE_NONE;
}




/**
  *@brief	start spi interrupt send
  *@param	buf:Transmit data buffer. 
  *@param	len:Data length.
  *@retval	None.
  */

void SPI_IRQHandlerProcess(void)
{
	u16 len = 0;
	if(REG_BIT_IS_CLEAR(SPIIS, SPI_DONE))
	{
		return;
	}

	if(spiInitFlag == 1)		//full speed
	{
		//调试发现IIC和SPI的提高时钟频率不能放在对应的初始化中，不然会影响到7816的低功耗发送ATR   
		sysConfig_SetCpuClkCb();

		spiInitFlag = 0;
	}

	__HAL_CLR_SPI_DONEFLAG();//clear int flag
	SYSWKPCON |= BIT31;	//clear SPIWKP_FLAG

	if( REG_BIT_IS_SET(SPICHCON, RxChOn) && (g_TxRxType == SPIHAL_TYPE_RX))
	{
		//__HAL_WAIT_SSPI_RXFIFO_NULL();
		
		__HAL_CLR_SPI_DMARx_STA();
		len = SPI_TXRX_Buffer_Size - SPI_DMARx_TC;
		
		Spi_RxDataFinish(SpiHal_TxRx_Buffer,len);		
		
		return;
	}
	else if(REG_BIT_IS_SET(SPICHCON, TxChOn) && (g_TxRxType == SPIHAL_TYPE_TX))
	{
		/*wait transfer finish with status no error*/
		while( (DMAC_CB1 & 0x070000) != 0x50000 ) ;
		
		/*clear transfer status*/
		__HAL_CLR_SPI_DMATx_STA();
		if(SpiResetFlag == SPI_STATUS_WARMRESET)
		{
			SpiResetFlag = SPI_STATUS_NONERESET;
			ecpSysHal_SystemReset(SPI_REREST_FLAG);
		}
		
		Spi_RxDataReady();
	}
	else  //if( g_TxRxType == SPIHAL_TYPE_NONE )
	{
		__HAL_CLR_SPI_DMATx_STA();
		//__HAL_SPI_CLR_RXTXFIFO();
		 
		/* start RX DMA */
		//HAL_DMA_Start(&hDMA_RxStVar, (u32)SpiHal_TxRx_Buffer,(unsigned int)SPIRXDATAADDR, SPI_TXRX_Buffer_Size);
		Spi_RxDataReady();
	}
	
}

/*
void ecpSpi_Reset(void)
{
	// soft reset
	RMU->RMU_ALGRST |= (1<<9);
}
*/



//#if	BOOTLOADER
//void GPIO_IRQHandlerProcess(void)
//#else
//void GPIO_IRQHandler(void)
//#endif
//{
//	//nothing
//}

#else
/// spi master mode
u8 g_spi_master_set_ssn;
u8 g_sendFinish;
u8 g_first_receive;
u8 g_SpiWakeUpTimeUs;

void init_GlobalVar()
{
	g_spi_master_set_ssn = 0;
	g_sendFinish = 0;
	g_first_receive = 1;
	g_SpiWakeUpTimeUs = 100;
}

void DelayUs(uint32_t us)
{
    uint32_t i ,j;
    for(i=0;i<us;i++)
    {
        for(j=0;j<7;j++)
        {
        }            
    }
}

/**
  *@brief	SPI master init.
  *@param	spiMode:SPI mode select,SQI SDI or normal mode.
  *@param	clkMode:Define clock phase plarity. 
  *@retval	None.  				 
  */
void ecpSpi_MasterInit()
{
	u8 mode;
	sysConfig_SetCpuClkCb();	//set spi clock 
	
	SpiHal_ConfigIOMux();	//config io

	__HAL_OPEN_SSPI_CLKEN();	//enable spi 	
	
	mode = sysConfig_GetChipSpiMode();
	
	/*Init SPI IP accord to Configuration Structure */
	SPICON = 0;
	
	SPICON |= 0<<MSB;					//set to MSB
	SPICON |= 1<<MASTER_SLAVE;			//set to master
	SPICON |= (mode&0x03);				//set mode
	
	SPICHCON = 0;
	SPICHCON |= 0x04;		//set ssn high
	SPICHCON |= 0x02;		//set Force CS out as spi_cs control by software
	
	/*init work mode*/
	/*Clear interrupt Flag*/
	__HAL_DISABLE_SSPI_INTEN();
	__HAL_CLR_SSPI_INTFLAG();
	__HAL_CLR_SPI_PENDIRQ();
		
	/*enable spi interrupt*/
	__HAL_SPI_DONE_INTEN();
	__HAL_ENABLE_SPI_IRQEN();
	
	
	SpiHal_DMA_StrucInit();
	
	init_GlobalVar();
	
}

u8 getTransmitFinishFlag()
{
	return g_sendFinish;
}

/* Master-Slave */
/************************************************* 
  Function:    SpiMasterHal_txData(u8 *txData, u16 txLength)
  Description:  		
  Calls:              
  Input: none
  Output:none
  Return:
  Others: 
*************************************************/ 
u16 SpiMasterHal_txData(u8 *txData, u16 txLength, u8 isPullUpSsn)
{
	 HAL_StatusTypeDef t_retStatus;
	
	/*wait spi free*/
	//while(__HAL_SPI_BUSY_STA != 0);
	g_sendFinish = 0;
	
	g_spi_master_set_ssn = isPullUpSsn;
	
	ecpMem_Copy(SpiHal_TxRx_Buffer,txData,txLength);

	/*close Rx channel*/
	if(REG_BIT_IS_SET(SPICHCON, RxChOn))
	{
		__HAL_SPI_CLOSE_RxChannel();
	}
	/*disable DMA RX*/
	__HAL_SPI_CLOSEDMA_RXLVL();
	
	/*clear rx\tx fifo*/
	__HAL_SPI_CLR_RXTXFIFO();
	
	/*enable SPI_DONE interrupt*/
	__HAL_SPI_DONE_INTEN();
	__HAL_ENABLE_SPI_IRQEN();
	
	g_TxRxType = SPIHAL_TYPE_TX; 

	/*Start DMA Tx transfer*/
	t_retStatus = HAL_DMA_Start(&hDMA_TxStVar, (unsigned int)SPITXDATAADDR, (u32)SpiHal_TxRx_Buffer, txLength);

	/*set DMA Tx level =15byte;*/
	SPIDMATDLR = 0x0F;
	
	/*set TxChOn=1*/
	__HAL_SPI_OPEN_TxChannel();
	
	/*set DMA Tx Enable*/
	__HAL_SPI_ENDMA_TXLEVEL();
	
	/*set tx len*/
	SPICHCON |=  ((txLength*8)&0xFFFF)<<TxRx_len;
	
	/*pull down ssn*/
	SPICHCON &= 0xFFFFFFFB;
	
	/*wakeup time*/
	DelayUs(g_SpiWakeUpTimeUs);
	//for(u16 i=0; i<1000; i++);

	/*start spi transmit,start clock*/
	SPISTR = 1;
	
	return HAL_OK;

}
/************************************************* 
  Function:     ecpSpi_MasterReceiveNBytes(unsigned char *DataAddr, unsigned int DataLen)
  Description:  		
  Calls:              
  Input: none
  Output:none
  Return:
  Others: 
*************************************************/ 
void SpiMasterHal_rxData(u8* rxData, u16 DataLen, u8 isPullUpSsn)
{
	HAL_StatusTypeDef t_retStatus;
	unsigned int t_dataRecept;

	/*close Tx channel*/
	if(REG_BIT_IS_SET(SPICHCON, TxChOn))
	{
		__HAL_SPI_CLOSE_TxChannel();
	}
	/*disable DMA TX*/
	__HAL_SPI_CLOSEDMA_TXLVL();	
	
	__HAL_DISABLE_SPI_IRQEN();
	__HAL_DISABLE_SSPI_INTEN();
	
	/*clear rx\tx fifo*/
	__HAL_SPI_CLR_RXTXFIFO();	

	g_TxRxType = SPIHAL_TYPE_RX;
	
//	/*set RxChOn=1*/
	__HAL_SPI_OPEN_RxChannel();
	
	SPICHCON |=  ((DataLen*8)&0xFFFF)<<TxRx_len;		//set rx len
	
	SPICHCON &= 0xFFFFFFFB;			//set cs low
	
	if(g_first_receive)
	{
		DelayUs(g_SpiWakeUpTimeUs);
		//for(u16 i=0; i<1000; i++);		//wake up slave delay
	}
		
	SPISTR = 1;		//start spi transmit
	
	t_dataRecept = 0;

	__HAL_WAIT_IF_SSPI_RXFIFO_NULL();
	
	while(t_dataRecept<DataLen)
	{
		while((__HAL_SPI_RX_FIFO_NUM == 0) && (__HAL_SPI_BUSY_STA != 0));
		
		rxData[t_dataRecept++] = SPIRXDATA;

		if(__HAL_SPI_RXTX_DONE_STA == 1)
			break;
	}
	
	while(__HAL_SPI_RXTX_DONE_STA == 0);
	
	__HAL_SPI_CLR_RXTXFIFO();//clr fifo	
	
	SPICHCON = 0x02;	//clear SPICHCON except Force CS out
	g_first_receive = 0;	

	if(isPullUpSsn)
	{
		SPICHCON |= 0x04;		//pull up ssn
		g_first_receive = 1;	
	}
	
}



#if	BOOTLOADER
void SPI_IRQHandlerProcess(void)
#else
__irq void SPI_IRQHandler(void)
#endif
{
	u16 len = 0;
	if(REG_BIT_IS_CLEAR(SPIIS, SPI_DONE))
	{
		return;
	}

	__HAL_CLR_SPI_DONEFLAG();//clear int flag
	SYSWKPCON |= BIT31;	//clear SPIWKP_FLAG

	if( REG_BIT_IS_SET(SPICHCON, RxChOn) && (g_TxRxType == SPIHAL_TYPE_RX))
	{
		SPICHCON = 0x02;	//clear SPICHCON except Force CS out
	
		if(g_spi_master_set_ssn)
		{
			SPICHCON |= 0x04; 	//pull up ssn
		}
		
		//__HAL_CLR_SPI_DMARx_STA();
		len = SPI_TXRX_Buffer_Size - SPI_DMARx_TC;
		
		//Spi_RxDataFinish(SpiHal_TxRx_Buffer,len);		
		
		return;
	}
	else if(REG_BIT_IS_SET(SPICHCON, TxChOn) && (g_TxRxType == SPIHAL_TYPE_TX))
	{
		
		SPICHCON = 0x02; 	//clear SPICHCON except Force CS out
		if(g_spi_master_set_ssn)
		{
			SPICHCON |= 0x04;	//pull up ssn
		}
		
		/*clear transfer status*/
		__HAL_CLR_SPI_DMATx_STA();
		
		g_sendFinish = 1;

		return;
	}
	else  //if( g_TxRxType == SPIHAL_TYPE_NONE )
	{
		SPICHCON = 0x02; //clear SPICHCON except Force CS out
		__HAL_CLR_SPI_DMATx_STA();
		
		return;
	}
	
}


#endif


#endif




