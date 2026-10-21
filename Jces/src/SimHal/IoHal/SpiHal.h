#ifndef __SpiHal_H__
#define	__SpiHal_H__

#include "DataOperation.h"
#include "ecpDefine.h"
#include "SysConfigProfile.h"
/*=================================================================*
 * Types:
 *=================================================================*/

/*=================================================================*
 * Defines:
 *=================================================================*/
 
 
#if HAL__IO__SPI

#define    Bit0_En        (0x1    )
#define    Bit1_En        (0x1<<1 )
#define    Bit2_En        (0x1<<2 )
#define    Bit3_En        (0x1<<3 )
#define    Bit4_En        (0x1<<4 )
#define    Bit5_En        (0x1<<5 )
#define    Bit6_En        (0x1<<6 )
#define    Bit7_En        (0x1<<7 )
#define    Bit8_En        (0x1<<8 )
#define    Bit9_En        (0x1<<9 )
#define    Bit10_En    (0x1<<10)
#define    Bit11_En    (0x1<<11)
#define    Bit12_En    (0x1<<12)
#define    Bit13_En    (0x1<<13)
#define    Bit14_En    (0x1<<14)
#define    Bit15_En    (0x1<<15)
#define    Bit16_En    (0x1<<16)
#define    Bit17_En    (0x1<<17)
#define    Bit18_En    (0x1<<18)
#define    Bit19_En    (0x1<<19)
#define    Bit20_En    (0x1<<20)
#define    Bit21_En    (0x1<<21)
#define    Bit22_En    (0x1<<22)
#define    Bit23_En    (0x1<<23)
#define    Bit24_En    (0x1<<24)
#define    Bit25_En    (0x1<<25)
#define    Bit26_En    (0x1<<26)
#define    Bit27_En    (0x1<<27)
#define    Bit28_En    (0x1<<28)
#define    Bit29_En    (0x1<<29)
#define    Bit30_En    (0x1<<30)
#define    Bit31_En    (0x1<<31)

#define    Bit0_Dis    (~(0x1)    )
#define    Bit1_Dis    (~(0x1<<1) )
#define    Bit2_Dis    (~(0x1<<2) )
#define    Bit3_Dis    (~(0x1<<3) )
#define    Bit4_Dis    (~(0x1<<4) )
#define    Bit5_Dis    (~(0x1<<5) )
#define    Bit6_Dis    (~(0x1<<6) )
#define    Bit7_Dis    (~(0x1<<7) )
#define    Bit8_Dis    (~(0x1<<8) )
#define    Bit9_Dis    (~(0x1<<9) )
#define    Bit10_Dis    (~(0x1<<10))
#define    Bit11_Dis    (~(0x1<<11))
#define    Bit12_Dis    (~(0x1<<12))
#define    Bit13_Dis    (~(0x1<<13))
#define    Bit14_Dis    (~(0x1<<14))
#define    Bit15_Dis    (~(0x1<<15))
#define    Bit16_Dis    (~(0x1<<16))
#define    Bit17_Dis    (~(0x1<<17))
#define    Bit18_Dis    (~(0x1<<18))
#define    Bit19_Dis    (~(0x1<<19))
#define    Bit20_Dis    (~(0x1<<20))
#define    Bit21_Dis    (~(0x1<<21))
#define    Bit22_Dis    (~(0x1<<22))
#define    Bit23_Dis    (~(0x1<<23))
#define    Bit24_Dis    (~(0x1<<24))
#define    Bit25_Dis    (~(0x1<<25))
#define    Bit26_Dis    (~(0x1<<26))
#define    Bit27_Dis    (~(0x1<<27))
#define    Bit28_Dis    (~(0x1<<28))
#define    Bit29_Dis    (~(0x1<<29))
#define    Bit30_Dis    (~(0x1<<30))
#define    Bit31_Dis    (~(0x1<<31))

//typedef enum{
//	SpiMode0 = 0,	    				//PH = 0, P0 = 0
//	SpiMode1 = 1,   					//PH = 0, P0 = 1
//	SpiMode2 = 2,	                    //PH = 1, P0 = 0
//	SpiMode3 = 3,					    //PH = 1, P0 = 1	
//}SpiMode;

#define SpiMode0  0
#define SpiMode1  1
#define SpiMode2  2
#define SpiMode3  3

#define SPI_BUFFER_SIZE      (270)

extern u8    RxStartFlag;
extern u8    SpiRxFinishFlag;
extern u8  SPI_CachBuffer[SPI_BUFFER_SIZE];
extern u8  SPI_ComBuffer[SPI_BUFFER_SIZE];

extern  void ecpSpi_SlaveInit(u8 clkMode);
extern void ecpSpi_DmaRx(u16 len);
extern void ecpSpi_DmaTx(u8 * buf,u16 len);
extern void ecpSpi_stopDMATransmit(void);
extern void SPI_ResetChip(void);
extern u8 SPI_isSoftReset(void);



#else

#define  ecpSpi_SlaveInit(clkMode)
#define  ecpSpi_DmaRx(len)
#define  ecpSpi_DmaTx(buf,len)
#define  ecpSpi_stopDMATransmit()
#define  SPI_ResetChip(void)
#define  SPI_isSoftReset(void)


#endif

#endif
