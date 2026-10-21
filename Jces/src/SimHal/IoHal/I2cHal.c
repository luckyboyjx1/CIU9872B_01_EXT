


#include "I2cHal.h"
#include "DataOperation.h"
#include "NvmHal.h"
#include "chipHead.h"
#include "Io_I2C.h"
#include "bootPlaformConfigProfile.h"

#if HAL__IO__I2C 

ST_I2C g_stI2C;
volatile u8 g_GPIO_Event;

//====================================================
#if BOOTLOADER
void I2C_IRQHandlerProcess(void)
#else
__irq void I2C_IRQHandler(void)
#endif
{
    if(I2CSTS == STAT_SLAVE_SLAW)            
	/* Own SLA+W has been received,ACK has been returned */
    {
        I2CMSK |= Bit0_En;
        I2CCON |= I2C_AA_SET;                   /* Send ACK */
        I2CCON &= (I2C_SI_CLEAR&I2C_START_CLEAR&I2C_STOP_CLEAR);        /* Clear SI/STA/STOP */
//        I2CMSK |= Bit0_En;                      /* Mask I2C interrupt*/
        I2C_RxProcess(g_stI2C.g_commBuf, &(g_stI2C.wFrameLen));
        while(!((I2CSTS == STAT_RSTOP)));
        I2CCON |= I2C_AA_SET;                   /* Set ACK */
        I2CCON &= I2C_SI_CLEAR;                 /* Clear SI flag */
        I2CCON &= I2C_DISABLE;
		if((g_stI2C.wFrameLen==6)&&(g_stI2C.g_commBuf[1]==0xCF))
		{
			rtnBackup(I2C_REREST_FLAG);
			
		}
		else
		{
			I2C_ReceCOMMANDProcess(g_stI2C.wFrameLen);
		}
		
		
    }
	else if(I2CSTS==STAT_SLAVE_SLAR_ACK)                /* SLA+R received, ACK returned */
	{
		I2CMSK |= Bit0_En;
        I2CCON |= I2C_AA_SET;                   /* Send ACK */
        I2CCON &= (I2C_SI_CLEAR&I2C_START_CLEAR&I2C_STOP_CLEAR);        /* Clear SI/STA/STOP */
		
		I2C_TxProcess(g_stI2C.g_commBuf,g_stI2C.wFrameLen);
		I2C_TranmitFinshProcess();
		
	
	}
    else
    {
        I2CCON |= I2C_AA_SET;                   /* Send ACK */
        I2CCON &= (I2C_SI_CLEAR&I2C_START_CLEAR&I2C_STOP_CLEAR);        /* Clear SI/STA/STOP */
    }
}

//====================================================
//    I2C Slave Initialize
void I2C_S_Init(void)
{
    g_stI2C.event = 0;
    CLKIOCON |= 0x48; //Enable I2C and GPIO clk
    
    IOPULLCFG |= Bit5_En | Bit6_En | Bit3_En; //使能上拉
    IOPULLSEL |= Bit5_En | Bit6_En | Bit3_En; //使能强上拉

    INTIOCON |= Bit6_En;                    // Enable I2C interrupt
    INTIOCON &= Bit3_Dis;                   // Disable GPIO interrupt
	NVIC_ClearPendingIRQ(I2C_IRQn);
    NVIC_EnableIRQ(I2C_IRQn);
    
    IOSEL = (IOSEL & ~0x001F8000) | Bit16_En | Bit19_En;  // Switch IO to I2C mode
    I2CCON = (I2C_ENABLE|I2C_AA_SET);       /* Enable I2C interface,set ack,clear start,stop */
    I2CADDR = 0xA2;//g_pbComCfg[I2C_SLAVE_ADDR];        /* Set I2C slave address */
    I2CMSK &= Bit0_Dis;
}
//====================================================
void I2C_RxReady(void)
{
	I2CMSK &= Bit0_Dis;

}
//====================================================
// I2C Receive One Byte without ACK/NAK
u8 I2C_RxByte(u8 *data)
{
    while(!((I2CSTS == STAT_RDATA) || (I2CSTS == STAT_GCA_ACK)))
    {
        if(I2CSTS == STAT_RSTOP)
        {
            return RECEIVE_ERROR;
        }
    }
    *data = I2CDATA;                /* Receive data */
    I2CCON |= I2C_AA_SET;           /* Set ACK */
    I2CCON &= I2C_SI_CLEAR;         /* Clear SI flag */
    return RECEIVE_OK;
}

//====================================================
//    I2C Receive Process for Packge Data
//    The format of package is
//    SOF | CLS INS  P1 P2 LEN1 LEN2| ... DATA | LRC
//    55  | XX  XX   XX XX XX   XX  | ... ZZ   | YY
u8 I2C_RxProcess(u8* RxBuf, u16 * length)
{
    u16 temp;
    //u8 frameData[4];
	temp=0;
    if((I2C_RxByte(RxBuf) == RECEIVE_OK))
	{
		temp++;
	}
    while(!((I2CSTS == STAT_RSTOP)))
	{
		if(I2C_RxByte(RxBuf + temp) == RECEIVE_OK)
		{
			temp++;
		}
	
	} 
	length[0] = temp;

    g_stI2C.event = IS_I2C_RX;

    return RECEIVE_OK;

}

//====================================================
//    I2C Send One Byte with checking ACK/NAK
u8 I2C_TxByte(u8 data)
{
    I2CDATA = data;
    I2CCON &= I2C_SI_CLEAR;                 /* Clear SI flag */
    while(1)
    {
        if(I2CSTS == STAT_RACK)             /* Byte sent, ACK received */
        {
            return SEND_OK;
        }
    }
}
//============================================================

u8 I2C_TxReady(u8* TxBuf, u16 TxLen)
{
	//u32 i;
    //for (i = 0; i < 0x0002000; i++); 
	//memset(g_stI2C.g_commBuf, 0, I2C_BUFFER_SIZE);
	memcpy(g_stI2C.g_commBuf, TxBuf, TxLen);
	g_stI2C.wFrameLen=TxLen;
    IOSEL = (IOSEL & ~0x0001F8000) | Bit16_En | Bit19_En;  // Switch IO to I2C mode
    I2CCON |= I2C_ENABLE;
    I2CCON |= I2C_AA_SET;                   /* Set ACK */
    I2CCON &= I2C_SI_CLEAR;                 /* Clear SI flag */

	I2CMSK &= Bit0_Dis;
	
	return 0;
}
//====================================================
//    I2C Send Process for package data
//    The format of package is

u8 I2C_TxProcess(u8* TxBuf, u16 TxLen)
{
//  u8 checksum = 0;
    u16 temp;
	u16 RemainlastBitlen;
//    u32 i;
//    for (i = 0; i < 0x0002000; i++); 

//    IOSEL = (IOSEL & ~0x0001F8000) | Bit16_En | Bit19_En;  // Switch IO to I2C mode
//    I2CCON |= I2C_ENABLE;
//    I2CCON |= I2C_AA_SET;                   /* Set ACK */
//    I2CCON &= I2C_SI_CLEAR;                 /* Clear SI flag */

//    //拉低一下 IO 通知主端可以接收了。
//    NTF_Out();
//    NTF_Clr();
//    for (i = 0; i < 0x00020; i++);//
//    NTF_Set();
//    NTF_In();

//    while(1)
//    {
//        if(I2CSTS==STAT_SLAVE_SLAR_ACK)                /* SLA+R received, ACK returned */
//            break;
//    }

//    I2C_TxByte(0xAA);
//    I2C_TxByte(TxLen >> 8);
//    I2C_TxByte(TxLen);
//    for(temp = 0; temp < TxLen; temp ++)
//    {
//        I2C_TxByte(TxBuf[temp]);
//        checksum ^= TxBuf[temp];
//    }
//    //send checksum
//    I2CDATA = checksum;
	RemainlastBitlen=TxLen-1;
	for(temp = 0;temp < RemainlastBitlen;temp++)
	{
		I2C_TxByte(TxBuf[temp]);
	}
	
	I2CDATA = TxBuf[RemainlastBitlen];
	I2CCON |= I2C_AA_SET;                   /* Set ACK */
    I2CCON &= I2C_SI_CLEAR;                 /* Clear SI flag */
    while(1)
    {

        if(I2CSTS == STAT_SLAVE_RNACK)      /* Byte sent, NACK received */
        {
            I2CCON |= I2C_AA_SET;           /* Set ACK */
            I2CCON &= I2C_SI_CLEAR;         /* Clear SI flag */
            break;
        }
		
		if(I2CSTS == STAT_RACK)             /* Byte sent, ACK received */
        {
            I2CDATA = 0x00;
			I2CCON |= I2C_AA_SET;                   /* Set ACK */
			I2CCON &= I2C_SI_CLEAR;                 /* Clear SI flag */
        }
    }
    return 0;
}







#endif




