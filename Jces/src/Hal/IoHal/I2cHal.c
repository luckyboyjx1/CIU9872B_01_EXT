


#include "I2cHal.h"
#include "DataOperation.h"
#include "NvmHal.h"
#include "chipHead.h"
#include "Io_I2C.h"
#include "bootPlaformConfigProfile.h"

#if HAL__IO__I2C 

/**
 * @brief DMACH SFR definition
 */
typedef struct 
{
  __IO	unsigned int 		DMAC_CA                ;  /*!< DMAC_CA        	register,	Address offset: 0x0 ~ 0x3 */
  __IO	unsigned int 		DMAC_CB                ;  /*!< DMAC_CB        	register,	Address offset: 0x4 ~ 0x7 */
  __IO	unsigned int 		DMAC_SA                ;  /*!< DMAC_SA        	register,	Address offset: 0x8 ~ 0xb */
  __IO	unsigned int 		DMAC_DA                ;  /*!< DMAC_DA        	register,	Address offset: 0xc ~ 0xf */
} DMACH_SFRTypeDef;
#define DMACH_BASE  (DMA_BA+0x10)
#define DMACH_STEP  (0x10)

#define I2CHAL_TYPE_NONE	0x00
#define I2CHAL_TYPE_RX      0x10
#define I2CHAL_TYPE_TX      0x20

#define I2C_TXRX_Buffer_Size	263
u8 I2CHal_TxRx_Buffer[I2C_TXRX_Buffer_Size];
u16 I2CHal_TxRx_Length;
volatile static u8 g_i2cTxRxType;

u8 i2cInitFlag;

//====================================================


void I2CHal_enableInterrupt(void)
{
    //*Interrupt
    NVIC_ICPR = ((1<<IRQ_I2C) | (0x01 << IRQ_DMA));           //I2C DMA 中断
    NVIC_ISER |= (0x01 << IRQ_I2C)| (0x01 << IRQ_DMA);        //使能 I2C DMA 中断
}


u8 I2CHal_isBusy(void)
{
	if ((I2CSR & (1 << SR_BusBusy)))
	{
		//receive start signal
		return 1;
	}
	
	//receive stop signal
	return 0;
}

/**
  * @brief  使能“Force SCL Low”或者“NAK”，用于Slave模式时向Master端表征slave busy
  * @param  BusyMode:0表示“Force SCL Low方式”;1表示“NAK方式”.
  * @return NONE
  */
void I2CHal_Slave_SetNak(void)
{
    SET_REG_BIT(I2CCTR0,NAK_SET);/*enable send NAK*/
	SET_REG_BIT(I2CCTR1,tx_auto_nak);
}

/**
  * @brief  失效“Force SCL Low”和“NAK”功能,SLave模式下准备向Master端发送数据
  * @param  BusyMode:0表示“Force SCL Low方式”;1表示“NAK方式”.
  * @return NONE
  */
void I2CHal_Slave_ClrNak(void)
{
	CLEAR_REG_BIT(I2CCTR0,NAK_SET);/*close send nak*/
	CLEAR_REG_BIT(I2CCTR1,tx_auto_nak);
}

//    I2C Slave Initialize
void I2CHal_SlaveInit(u8 addr)
{
    DMACH_SFRTypeDef *pRxDMACH,*pTxDMACH;
    u32 tmpDmaCAx,tmpDmaCBx;
	
	//sysConfig_SetCpuClkCb();

	//*Clock
    SYSCLKEN |= (1 << I2CEN) | (1 << DMAEN) ;   //使能 DMA I2C 时钟
    
    //*IO
    IOCTRLMODE |= (0x03 << 8)| (0x03 << 6);     //iO模式选择 AF
    IOCTRLAF1 &= ~((0x0f<<12) | (0x0f<<16));    //AF=0 io3:I2C_SCL io4:I2C_SDA
    IOCTRLOD |= (0x01 << 3)| (0x01 << 4);       //开漏模式
    IOCTRLDS &= ~((0x01 << 3)| (0x01 << 4));    //弱区动
    IOCTRLPDEN &= ~((0x01 << 3)| (0x01 << 4));  //断开下拉电阻
    IOCTRLPUEN |= (0x01 << 3)| (0x01 << 4);     //上拉电阻
    IOAGFEN |= (0x01 << 1);                     //开启IIC模拟滤毛刺功能
    IOAGFCFG |= (0x10 << 2);                    //配置滤毛刺 82ns
    IOCTRLCFGEN = 0x59AF6E01;                   //io配置项使能
	I2CCTR1 = (1 << no_setnak_stop_int);        //BUSY时不产生中断
     
    //*DMA
    pRxDMACH = (DMACH_SFRTypeDef*)(DMACH_STEP*(I2C_RX_CHANNEL)+DMACH_BASE);         //获取 DMA_RX 寄存器
    tmpDmaCAx = 0;
	tmpDmaCAx = ((TRANSBEAT_NORM << BT)|(BLOCKSIZE_0 << BC));                       //传输类型:Normal | 每个DMA传输数据的数量 
    tmpDmaCBx = 0;
	tmpDmaCBx = ((TRANSMODE_NEED << MS)|(DATA_BYTE << TW));                         //传输模式:NEED | 传输位宽:字节
    /*fix src addr, inc Dst addr*/
	tmpDmaCBx |= (1<<FS);
    /*config trigger req,DREQ高电平 或 上升沿*/
    tmpDmaCAx |= (0x0E<<IS);
    pRxDMACH->DMAC_CA = tmpDmaCAx;
	pRxDMACH->DMAC_CB = tmpDmaCBx;
    
    pTxDMACH = (DMACH_SFRTypeDef*)(DMACH_STEP*(I2C_TX_CHANNEL)+DMACH_BASE);         //获取 DMA_TX 寄存器
    tmpDmaCAx = 0;
	tmpDmaCAx = ((TRANSBEAT_NORM << BT)|(BLOCKSIZE_0 << BC));                       //传输类型:Normal | 每个DMA传输数据的数量 
    tmpDmaCBx = 0;
	tmpDmaCBx = ((TRANSMODE_NEED << MS)|(DATA_BYTE << TW));                         //传输模式:NEED | 传输位宽:字节
    /*fix dst addr, inc src addr*/
    tmpDmaCBx |= (1<<FD);
    /*config trigger req,DREQ高电平 或 上升沿*/
    tmpDmaCAx |= (0x0E<<IS);
    pTxDMACH->DMAC_CA = tmpDmaCAx;
	pTxDMACH->DMAC_CB = tmpDmaCBx;
    
    //*I2C
    I2CADDR0 = (addr << 1) | 1;                                                     //7bit 地址
    I2CADDRMASK = 0x00;                                                             //屏蔽地址位:全部对比
    I2CCTR0 = (1<<INT_EN);                                                          //使能I2C中断
    I2CINTSR = 0;                                                                   //清除中断状态
    I2CINTMSK = 0x7f;                                                               //设置中断屏蔽
    
    I2CHal_enableInterrupt();
    g_i2cTxRxType = I2CHAL_TYPE_NONE;
    
	i2cInitFlag = 1;
}

void I2C_Enable(void)
{

    SYSCLKEN |= (1 << I2CEN);               //使能 I2C 时钟
    I2CSRST |= (1 << SOFT_RST);             //软复位
    NVIC_ICPR = (1<<IRQ_I2C);               //清除挂起I2C 中断
    NVIC_ISER = (1<<IRQ_I2C);               //使能 I2C中断
}

void I2C_Disable(void)
{

    SYSCLKEN |= (1 << I2CEN);       //使能I2C 时钟
    I2CSRST |= (1 << SOFT_RST);     //软复位
    SYSCLKCLR |= (1 << I2CEN);      //清除I2C 时钟
    NVIC_ICER = (1<<IRQ_I2C);       //清除 I2C中断
    NVIC_ICPR = (1<<IRQ_I2C);       //清除挂起I2C 中断
}

void I2CHal_rxReady(u16 length)
{
	__IO unsigned int tmpSFR;
	DMACH_SFRTypeDef *pRxDMACH;
    
	pRxDMACH = (DMACH_SFRTypeDef*)(DMACH_STEP*(I2C_RX_CHANNEL)+DMACH_BASE);
	
	/*disable Chnnel x*/
	__HAL_CLOSE_DMA_CHX(pRxDMACH->DMAC_CA);

    __HAL_CLEAR_I2CRX_FIFO();

	/*config src addr*/
	pRxDMACH->DMAC_SA =  (unsigned int)I2CRXDATAADDR;
    pRxDMACH->DMAC_DA = (u32)I2CHal_TxRx_Buffer;
	tmpSFR = pRxDMACH->DMAC_CA;
	tmpSFR &= (~0xFFFF);
	
	/*transfer data len =  BlockCount×BlockSize×TW，本项目配置BlockSize=1
	  refer BlockSizeUnit(BlockSize = BlockSizeUnit+1) 
	  and DataWidth(TW = 1,2,4) define*/
	tmpSFR |= I2C_TXRX_Buffer_Size;
	pRxDMACH->DMAC_CA = tmpSFR;

	/*open total tuel*/
	if(REG_BIT_IS_CLEAR(DMAC_CR, DE))
    {
        DMAC_CR |= (1ul<<DE);
    }
	/*open ChannelX tuel*/
	__HAL_OPEN_DMA_CHX(pRxDMACH->DMAC_CA);
	
    /*set DMA Rx level =0byte;*/
    I2CDMARDLR = 0x00;
    
    __HAL_I2C_ENDMA_RXLEVEL();

    g_i2cTxRxType = I2CHAL_TYPE_RX;

    I2CHal_Slave_ClrNak();
}

//====================================================
//    I2C Send One Byte with checking ACK/NAK
u16 I2CHal_txData(u8* txData, u16 txLength)
{
	__IO unsigned int tmpSFR;
	DMACH_SFRTypeDef *pTxDMACH;
    
    __HAL_CLEAR_I2CTX_FIFO();
    I2CHal_TxRx_Length = txLength;
    ecpMem_Copy(I2CHal_TxRx_Buffer,txData,txLength);

	pTxDMACH = (DMACH_SFRTypeDef*)(DMACH_STEP*(I2C_TX_CHANNEL)+DMACH_BASE);
	
	/*disable Chnnel x*/
	__HAL_CLOSE_DMA_CHX(pTxDMACH->DMAC_CA);

	/*config src addr*/
	pTxDMACH->DMAC_SA =  (u32)I2CHal_TxRx_Buffer;
    pTxDMACH->DMAC_DA = (unsigned int)I2CTXDATAADDR;
	tmpSFR = pTxDMACH->DMAC_CA;
	tmpSFR &= (~0xFFFF);
	
	/*transfer data len =  BlockCount×BlockSize×TW，本项目配置BlockSize=1
	  refer BlockSizeUnit(BlockSize = BlockSizeUnit+1) 
	  and DataWidth(TW = 1,2,4) define*/
	tmpSFR |= (txLength & 0xffff);
	pTxDMACH->DMAC_CA = tmpSFR;

	/*open total tuel*/
	if(REG_BIT_IS_CLEAR(DMAC_CR, DE))
    {
        DMAC_CR |= (1ul<<DE);
    }
    
    while(REG_BIT_IS_SET(I2CSR,SR_TIP));
    while(REG_BIT_IS_CLEAR(I2CSR,SR_TIP));
    
	/*open ChannelX tuel*/
	__HAL_OPEN_DMA_CHX(pTxDMACH->DMAC_CA);
	
    /*set DMA Tx level =8byte;*/
    I2CDMATDLR = 0x07;
    I2CDMACR |= (1 << I2C_TDMAE);
    
    g_i2cTxRxType = I2CHAL_TYPE_TX;

    I2CHal_Slave_ClrNak();
	
	return HAL_OK;
}


void I2CHal_StopTxData(void)
{
    I2CHal_Slave_SetNak();
	
	g_i2cTxRxType = I2CHAL_TYPE_NONE;
    
}

void I2CHal_StopRxData(void)
{
    I2CHal_Slave_SetNak();
	
	g_i2cTxRxType = I2CHAL_TYPE_NONE;
}


void I2C_IRQHandlerProcess(void)
{
	u16 recv_count = 0;
    u8 dmaStatus = 0;

	if(i2cInitFlag == 1)		//full speed
	{
		//调试发现IIC和SPI的提高时钟频率不能放在对应的初始化中，不然会影响到7816的低功耗发送ATR   
		sysConfig_SetCpuClkCb();

		i2cInitFlag = 0;
	}

    //	/* 清除I2C中断标志位*/
	__HAL_CLR_I2C_STOPFLAG();//clear Stop int flag
    
	if( REG_BIT_IS_CLEAR(I2CSR,SLAVE_RW) && (g_i2cTxRxType == I2CHAL_TYPE_RX)) 
	{		
        //等待fifo清空
        //while(__HAL_I2C_RX_FIFO_NUM > 0);

        /*清除I2C DMA通道接收状态标志，用户在清状态前，可以判断下实际状态，并进行相应的处理，本处没有判断*/
        dmaStatus = (DMAC_CB2>>SS) & 7; 
        while((dmaStatus != 0) &&(dmaStatus != 5));
        __HAL_CLR_I2C_DMARx_STA();
        /*通过DMA通道,I2C实际接收数据赋值给RxDataSize变量*/
		recv_count = I2C_TXRX_Buffer_Size - I2C_DMARx_TC;
        I2CHal_Slave_SetNak(); //set busy

        I2C_RxDataFinish(I2CHal_TxRx_Buffer, recv_count);
	}
	else if(REG_BIT_IS_SET(I2CSR, SLAVE_RW) && (g_i2cTxRxType == I2CHAL_TYPE_TX))
	{
        //while(__HAL_I2C_TX_FIFO_NUM > 0); //等待fifo清空

        dmaStatus = (DMAC_CB3 >> SS) & 7;       
        while((dmaStatus != 5));
        //	/*清除I2C DMA通道接收状态标志，用户在清状态前，可以判断下实际状态，并进行相应的处理，本处没有判断*/
        __HAL_CLR_I2C_DMATx_STA();

        I2C_RxDataReady();
	}
	
	else
	{
        //	/*清除I2C DMA通道接收状态标志，用户在清状态前，可以判断下实际状态，并进行相应的处理，本处没有判断*/
        __HAL_CLR_I2C_DMARx_STA();
        __HAL_CLR_I2C_DMATx_STA();

        I2C_RxDataReady();
	}
    
	
}

#endif




