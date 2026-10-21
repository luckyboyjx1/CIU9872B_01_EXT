#ifndef __SpiHal_H__
#define	__SpiHal_H__

#include "DataOperation.h"
#include "ecpDefine.h"
#include "SysConfigProfile.h"
/*=================================================================*
 * Types:
 *=================================================================*/

 
 
#if HAL__IO__SPI

#define SPIHAL_TYPE_NONE	 0
#define SPIHAL_TYPE_TX		 1
#define SPIHAL_TYPE_RX		 2

#define	HAL_IO_SPI_MASTER	0

#define SPI_TXRX_Buffer_Size	0x107
extern u8 SpiHal_TxRx_Buffer[SPI_TXRX_Buffer_Size];


#if !HAL_IO_SPI_MASTER
extern  void ecpSpi_SlaveInit(u8 clkMode);
extern u32 spi0_send_receive_dma(u32 spis_send_count);
extern void ecpSpi_dma_init(void);
extern void SpiHal_rxReady(u16 length);
extern void SpiHal_StopTxData(void);
extern void SpiHal_SlaveInit(void);
extern void SpiHal_StopRxData(void);
extern u16 SpiHal_txData(u8* txData, u16 txLength);
extern u8  ecpSpi_isProcess(void);
extern void SPI_IRQHandlerProcess(void);
#else 
extern u16 SpiMasterHal_txData(u8 *DataAddr, u16 DataLen, u8 isPullUpSsn);
extern void SpiMasterHal_rxData(u8* rxData, u16 DataLen, u8 isPullUpSsn);
extern u8 getTransmitFinishFlag();
extern u8 g_spi_master_set_ssn;
extern u8 g_sendFinish;
extern u8 g_SpiWakeUpTimeUs;
#endif



#else

#define  ecpSpi_SlaveInit(clkMode)
#define  ecpSpi_Slave_Rx_IT()
#define  ecpSpi_Slave_Tx_IT(buf,len)
#define  ecpSpi_enableInterrupt()
#define  ecpSpi_MasterSendNBytes(DataAddr, DataLen)
#define  ecpSpi_MasterReceiveNBytes(DataAddr, DataLen)
#define  ecpSpi_MasterSendNBytes_IT(DataAddr, DataLen)
#define  ecpSpi_MasterReceiveNBytes_IT(DataAddr, DataLen)


#endif

#endif
