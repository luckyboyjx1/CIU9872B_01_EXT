


#include "SpiHal.h"
#include "DataOperation.h"
#include "NvmHal.h"
#include "chipHead.h"
#include "Io_SPI.h"
#include "bootPlaformConfigProfile.h"

#if HAL__IO__SPI


u8  SPI_RxBuffer[SPI_BUFFER_SIZE]  __attribute__((at(0x20008780)));
u8  SPI_TxBuffer[SPI_BUFFER_SIZE]  __attribute__((at(0x20008980))); 
u8  SPI_CachBuffer[SPI_BUFFER_SIZE];

u8    SPIMasterEn;
u8    RxStartFlag;
u8    SpiRxFinishFlag;
u8    SpiTxStartFlag;
u8    RxCount=0;
u8    RxRealCount=0;
u16   RealDatalength=0;
/*
 *  Copyright (c) 2012, Beijing Tongfang Microelectroics Co., Ltd.
 *  All rights reserved.
 * 
 *  FileName: SPI.c
 *  SCFID: 
 *  Feature: SPI process of PICC
 *  Version: V0.1
 * 
 *  History: 
 *    2017-09-10 by GaoY
 *      1. Original version 0.1
 * */

/**
  *@brief	SPI master init.
  *@param	spiMode:SPI mode select,SQI SDI or normal mode.
  *@param	clkMode:Define clock phase plarity. 
  *@retval	None.  				 
  */
	
void ecpSpi_MasterInit(u8 clkMode)
{
	SPIMasterEn = 1;
	CLKIOCON |= 0x04;					/* Enable SPI clock */

	SPIMSK |= 0x01;					/* Mask BOVER interrupt */
	SPIDMAMSK = 0x03;					/* Mask BOVER OVER DMA interrupt */	
	SPICON = 0x00;  					/* Clear first */
	SPICON = (0xD0|clkMode);			/* Master mode,enable,clk defines clock、phase 、plarity */
	IOSEL =0x49200;						/*SPI mode*/
}

void SPI_Reset(void)
{
    SPIMSK = 0xFF;                      //mask all irq of single byte mode 
    RSTIOCON &= Bit2_Dis;           //reset SPI
    while((RSTIOCON & Bit2_En)!= Bit2_En); //wait for resetting over
    SPICON = 0;
	
    SPICON |= sysConfig_GetSPIMODE();                     // Set CPHA & COPL
    SPICON |= Bit7_En;              // SPI start to work
    //SPIRESUMEMSK = 0;                   //enable awake
	    //SPIRESUMEMSK = 0;                   //enable awake
    GPIORESUMEEN = Bit3_En;
}
/**
  *@brief	SPI slave init.
  *@param	clkMode:Define clock phase plarity. 
  *@retval	None.  				 
  */
void ecpSpi_SlaveInit(u8 clkMode)
{
	 SPIMasterEn = 0;
//	CLKIOCON |= 0x04;					/* Enable SPI clock */
//	
//	IOSEL0 =0x49200;						/*SPI mode*/
//	SPIMSK |= 0x05;					    /* Mask BOVER OVER interrupt */
//	SPIDMAMSK = 0x00;					/* Mask BOVER  DMA interrupt,OPEN DMA OVER interrupt */			
////	SPIRESUMEMSK &=  Bit0_Dis;		    /* Enable slave mode ssn wakeup function */
//	SPICON = (0x80|clkMode);  			/* Slave mode,enable,clkMode defines clock、phase 、plarity */
	  CLKIOCON |= (Bit2_En | Bit3_En);    //  Enable SPI/GPIO CLK
//    INTIOCON |= Bit2_En;                    //  Enable SPI interrupt
//    INTIOCON |= Bit3_En;                   //  Enable GPIO interrupt
//		NVIC_ClearPendingIRQ(SPI_IRQn);
//	  NVIC_EnableIRQ(SPI_IRQn);

    //clear buf
    ecpMem_Set(SPI_RxBuffer,0x00,SPI_BUFFER_SIZE);
    ecpMem_Set(SPI_TxBuffer,0x00,SPI_BUFFER_SIZE);

    //Switch IO to MISO,MOSI,SSN,SPICLK mode
    //IOSEL0 = (Bit19_En | Bit18_En | Bit16_En | Bit15_En |Bit13_En| Bit12_En | Bit10_En| Bit9_En);    //    Switch IO to SPI mode, leave SSN to be a GPIO
    GPIOCFG0 &= Bit3_Dis;//write 0 GPIO3 edge interrupt
		GPIOCFG1 |= Bit3_En;
//		NVIC_ClearPendingIRQ(GPIO_IRQn);
//	  NVIC_EnableIRQ(GPIO_IRQn);
//		GPIOINTEN |= Bit3_En;
		IOSEL =0x49200;						/*SPI mode*/
		
    //DMA mode
    SPI_Reset();
	  RxStartFlag = 0;
	  SpiTxStartFlag=0;
}

/**
  *@brief	SPI receive one byte.
  *@param	None. 
  *@retval	None.
  */
u8 ecpSpi_RxByte(void)
{
	if(SPIMasterEn)
	{
		SPIDAT = 0xFF;					/* As master,send data 0xFF to start clock. */
	}
	while(!(SPISTS&0x01));			/* Wait BOVER be set, receive over. */
	return SPIDAT;						/* Return data and clear flag. */
}

/**
  *@brief	SPI transmit one byte.
  *@param	data:Data to transmit. 
  *@retval	None.
  */
void ecpSpi_TxByte(u8 data)
{
	SPIDAT = data;		 				/* Send data */
	while(!(SPISTS&0x01));			/* Wait BOVER be set, transmit over. */ 
	data = SPIDAT;						/* Clear flag. */
}
//#ifdef SPIDMAEN
/**
  *@brief	SPI DMA receive,working in half-duplex.
  *@param	buf:Receive data buffer. 
  *@param	len:Data length.
  *@retval	None.
  */
void ecpSpi_enableInterrupt(void)
{
	INTIOCON |= 0x04;
	NVIC_ClearPendingIRQ(SPI_IRQn);
	NVIC_EnableIRQ(SPI_IRQn);

}

void ecpSpi_disableInterrupt(void)
{
	INTIOCON &= ~0x04;
	NVIC_ClearPendingIRQ(SPI_IRQn);
	NVIC_DisableIRQ(SPI_IRQn);

}
void ecpSpiSSN_GPIOEnableInterrupt(void)
{
		GPIOINTEN |= Bit3_En;
	  INTIOCON |= Bit3_En;                   //  Enable GPIO interrupt
	  NVIC_ClearPendingIRQ(GPIO_IRQn);
	  NVIC_EnableIRQ(GPIO_IRQn);
		
	

}

void ecpSpi_DmaRx(u16 len)
{
//	u16 i;
//	for(i=0;i<2500;i++)
//	{
//		;
//	}
	
	//SPIDMASTS |= 0x01;				/* Clear flag. */
	    //clear buf
  ecpMem_Set(SPI_RxBuffer,0xff,SPI_BUFFER_SIZE);
	
 	SPIDMALEN  = len;				    /* Length of data to be received. */
	SPIDMARADR = (u32)(SPI_RxBuffer); 	
	//SPIDMAMSK = 0x02;	//开启DMA OVER中断
	while(GPIODAT_B3 == 0);     //只有在没有通信时，才开启
	SPIDMACON = 5; 
//	SPIDMACON &= 0xfffffffd;				/* Receive only. */	 
//  SPIDMACON |= 0x04;
//	SPIDMACON |= 0x01;				/* Start DMA receive. */
	RxStartFlag = 1;
	ecpSpiSSN_GPIOEnableInterrupt();
	//ecpSpi_enableInterrupt( );

//	while(!(SPIDMASTS & 0x01)); 		/* Waiting for DMA operation accomplish. */
//	SPIDMASTS |= 0x01;				/* Clear flag. */
//	memCopyToRam(buf, SPI_RxBuffer, len);
}


/**
  *@brief	SPI DMA transmit,working in half-duplex.
  *@param	buf:Transmit data buffer. 
  *@param	len:Data length.
  *@retval	None.
  */
void ecpSpi_DmaTx(u8* buf,u16 len)
{
	ecpMem_Set(SPI_TxBuffer,0xff,SPI_BUFFER_SIZE);
	memCopyToRam(SPI_TxBuffer, buf, len);
	while(GPIODAT_B3 == 0);     //只有在没有通信时，才开启
	SPICON &= Bit7_Dis;
  SPICON |= Bit7_En;          // SPI start to work
	SPIDMALEN  = len;					/* Length of data to be transmitted. */
	SPIDMAMSK = 0x02;	//屏蔽DMA BOVER中断
	SPIDMATADR = (u32)(SPI_TxBuffer);
	SPIDMACON = 3; 
//	SPIDMACON &= 0xfffffffb; 				/* Transmit only. */			
//	SPIDMACON |= 0x02;
//	SPIDMACON |= 0x01;				/* Start DMA transmit. */
	ecpSpi_enableInterrupt( );
	SpiTxStartFlag=1;
	
//	while(!(SPIDMASTS & 0x01));	    /* Waiting for DMA operation accomplish. */
//	SPIDMASTS |= 0x01;				/* Clear flag. */
}

/**
  *@brief	SPI DMA transfer.
  *@param	bufTx:Transmit data buffer. 
  *@param	bufRx:Receive data buffer. 
  *@param	len:Data length.
  *@retval	None.
  */
void ecpSpi_DmaTxRx(u8* bufTx,u8* bufRx,u16 len)
{
	SPIDMALEN  = len;					/* Length of data to be transmitted. */
	SPIDMATADR = (u32)(bufTx);
	SPIDMARADR = (u32)(bufRx);
	SPIDMACON |= 0x04;				
	SPIDMACON |= 0x02;
	SPIDMACON |= 0x01;				/* Start DMA transmit. */
	while(!(SPIDMASTS & 0x01));	    /* Waiting for DMA operation accomplish. */
	SPIDMASTS |= 0x01;				/* Clear flag. */
}


void ecpSpi_stopDMATransmit(void)
{
   SPIDMACON &= U16_XOR_U16(0xff,0x01);						/* Stop DMA transmit. */

}
//#endif

/**
  *@brief	SPI receive multi bytes.
  *@param	buf:Receive data buffer. 
  *@param	len:Data length.
  *@retval	None.
  */
void ecpSpi_RxNBytes(u8 * buf,u16 len)
{     
//#ifdef SPIDMAEN
//	ecpSpi_DmaRx(buf,len);
//#else
	u16	i;
		
	for(i=0;i<len;i++)
	{	
		*(buf+i) = ecpSpi_RxByte();
	}
	
//#endif
}

/**
  *@brief	SPI transmit multi bytes.
  *@param	buf:Transmit data buffer. 
  *@param	len:Data length.
  *@retval	None.
  */
void ecpSpi_TxNBytes(u8* buf,u16 len)
{ 	
//#ifdef SPIDMAEN	
//		ecpSpi_DmaTx(buf,len);
//#else
	u16	i;

	for(i=0;i<len;i++)
	{	
		ecpSpi_TxByte(*(buf+i));
	}
	
//#endif
}

void SPI_ResetChip(void)
{
	RSTIOCON 			&= ~(1<<2);	//对SPI进行复位，写0
		

}
u8 SPI_isSoftReset(void)
{
	while(!(RSTIOCON & (1<<2)))	//SPI完成复位，是否为1
	{
		return 1;
	}
	return 0;
	
}

#if	BOOTLOADER
void SPI_IRQHandlerProcess(void)
#else
__irq void SPI_IRQHandler(void)
#endif
{
	
	
/*	if(SPIDMASTS&0x02)
	{
		SPIDMASTS = 0xff;
		if(RxStartFlag)
		{
			
			if((SPI_RxBuffer[RxCount]==SPI_RBLOCK_PCB_COMMAND_BLOCK)||(SPI_RxBuffer[RxCount]==SPI_SBLOCK_PCB_COMMAND_BLOCK))
			{
				RxRealCount++;
				
				if(RxRealCount == 6)
				{
					memCopyToRam(SPI_CachBuffer, &SPI_RxBuffer[RxCount], RxRealCount);
					RxStartFlag=0;
					ecpSpi_stopDMATransmit();
					SPI_ReceCOMMANDProcess(RxRealCount);
					RxCount = 0;
					RxRealCount = 0;
				}
				
				
			}
			else if(SPI_RxBuffer[RxCount] == SPI_IBLOCK_PCB_COMMAND_BLOCK)
			{
				RxRealCount++;
	
				if(RxRealCount ==7)
				{
					RealDatalength=U8_A_U8(U8_A_U8(SPI_RxBuffer[RxCount+SPI_HEADER_LC_OFFSET],SPI_IBLOCK_HEADER_LENTH),SPI_LRC_LENTH);
					RealDatalength=RealDatalength+RxCount;

				}
				if(RealDatalength== (u16)SPIDMARPT)
				{
					memCopyToRam(SPI_CachBuffer, &SPI_RxBuffer[RxCount], RxRealCount);	
					RxStartFlag=0;
					ecpSpi_stopDMATransmit();
					SPI_ReceCOMMANDProcess(RxRealCount);
					RxCount = 0;
					RxRealCount = 0;
					RealDatalength=0;
					
				}
				
			
			}
			else
			{
				
				RxCount++;
			}
			
//			if((SPI_RxBuffer[SPI_HEADER_PCB_OFFSET] == SPI_RBLOCK_PCB_COMMAND_BLOCK)||(SPI_RxBuffer[SPI_HEADER_PCB_OFFSET] == SPI_SBLOCK_PCB_COMMAND_BLOCK))
//			{
//				RxCount++;
//				if(RxCount == 6)
//				{
//					memCopyToRam(SPI_CachBuffer, SPI_RxBuffer, RxCount);
//					
//					
//					RxStartFlag=0;
//					ecpSpi_stopDMATransmit();
//					SPI_ReceCOMMANDProcess(RxCount);
//					RxCount = 0;
//				}
//			}
//			if(SPI_RxBuffer[SPI_HEADER_PCB_OFFSET] == SPI_IBLOCK_PCB_COMMAND_BLOCK)
//			{
//				RxCount++;
//				//RealDatalength=U8_A_U8(U8_A_U8(SPI_ComBuffer[SPI_HEADER_LC_OFFSET],SPI_IBLOCK_HEADER_LENTH),SPI_LRC_LENTH);
//				if(RxCount > 7)
//				{
//					RealDatalength=U8_A_U8(U8_A_U8(SPI_RxBuffer[SPI_HEADER_LC_OFFSET],SPI_IBLOCK_HEADER_LENTH),SPI_LRC_LENTH);
//					if(RealDatalength== (u16)SPIDMARPT)
//					{
//						memCopyToRam(SPI_CachBuffer, SPI_RxBuffer, RxCount);
//						
//						
//						RxStartFlag=0;
//						ecpSpi_stopDMATransmit();
//						SPI_ReceCOMMANDProcess(RxCount);
//						RxCount = 0;
//						
//					}
//				}
//			}
			
		}		
	}
*/
	if(SPIDMASTS&0x01)
	{
		SPIDMASTS = 0xff;
		if(SpiTxStartFlag)
		{
			INTIOCON &= ~0x04;
			
			SpiTxStartFlag = 0;
			SPI_TranmitFinshProcess();
		}
		
	}
	
	
}

#if	BOOTLOADER
void GPIO_IRQHandlerProcess(void)
#else
void GPIO_IRQHandler(void)
#endif
{
	u16 datalength;
	
	  GPIOINTEN &= Bit3_Dis;
		if(GPIOINTSTS&0x08)
		{
			GPIOINTSTS |=0x08;
			SPIDMASTS = SPIDMASTS;      //clear spi all
			if(RxStartFlag)
			{
				 if(SPI_RxBuffer[SPI_HEADER_PCB_OFFSET] == SPI_IBLOCK_PCB_COMMAND_BLOCK)
				 {
						datalength=(u16)SPIDMARPT;
					 	if(datalength==6)
						{
							if((SPI_RxBuffer[1]==0xCF)&&(SPI_RxBuffer[2]==0x00))
							{
								//SPI_ResetChip();
								rtnBackup(SPI_REREST_FLAG);
							}	
						}
						memCopyToRam(SPI_CachBuffer, &SPI_RxBuffer[0], datalength);
						RxStartFlag=0;
						ecpSpi_stopDMATransmit();

						SPI_ReceCOMMANDProcess(datalength);
				 }
				 else
				 {
						RxStartFlag=0;
						ecpSpi_stopDMATransmit();
					  SPI_Reset();
					  ecpSpi_DmaRx(SPI_BUFFER_SIZE);						
					 
				 }
			
			}
		
		}
	
	


}






#endif




