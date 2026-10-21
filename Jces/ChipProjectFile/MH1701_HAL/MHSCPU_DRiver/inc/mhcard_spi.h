/************************ (C) COPYRIGHT ****************************************************
 * @file                : mhscpu_spi.h
 * @author              :  
 * @version             : V1.0.0
 * @date                : 29-June-2019
 * @brief               : This file contains all the functions prototypes for the SPI firmware library
 *********************************************************************************************************/
 
#ifndef __MHCARD_SPI_H
#define __MHCARD_SPI_H
#ifdef __cplusplus
extern
{
#endif

#include "mhcard.h"
	


typedef struct 
{
	__IO uint32_t CR_1;				/*!< SPI Control register		 		*/
	__IO uint32_t SR;				/*!< SPI Status register 				*/
	__IO uint32_t WDR;				/*!< SPI Write Data register 			*/
	__IO uint32_t RDR;				/*!< SPI Read Data register 			*/
	__IO uint32_t FCR;				/*!< SPI FIFO Control register 			*/
	__I  uint32_t TISR;				/*!< SPI TX Interrput Status register	*/	
	__IO uint32_t FSR;				/*!< SPI FIFO Status register 			*/
	__I  uint32_t RISR;				/*!< SPI RX Interrput Status register	*/	
}SPI_TypeDef;

#define SPIS_BASE                              	(MHCARD_APB_BASE + 0x4000)
#define SPIS									((SPI_TypeDef *) SPIS_BASE) 


typedef struct
{
	uint8_t SPI_Sample;
	uint8_t SPI_RXEnable;
	uint8_t SPI_TXEnable;
	uint8_t SPI_RXFIFOFullThreshold;
	uint8_t SPI_RXFIFOEmptyThreshold;
	uint8_t SPI_TXFIFOFullThreshold;
	uint8_t SPI_TXFIFOEmptyThreshold;
	
}SPI_InitTypeDef;


typedef struct 
{
	uint8_t SPI_DMAReceiveEnable;
	uint8_t SPI_DMATransmitEnable;
	uint8_t SPI_DMAReceiveLevel;
	uint8_t SPI_DMATransmitLevel;
	
}SPI_DMAInitTypeDef;
	

/** @defgroup SPI_Sample
  * @{
  */
  
#define SPI_SAMPLE_MODE_0											(0x00)
#define SPI_SAMPLE_MODE_1											(0x01)
#define SPI_SAMPLE_MODE_2											(0x02)
#define SPI_SAMPLE_MODE_3											(0x03)

#define IS_SPI_SAMPLE(SAMPLE)										(((SAMPLE) == SPI_SAMPLE_MODE_0) || \
																	((SAMPLE) == SPI_SAMPLE_MODE_1)  || \
																	((SAMPLE) == SPI_SAMPLE_MODE_2)  || \
																	(SAMPLE) == SPI_SAMPLE_MODE_3)
/**
  * @}
  */
  

/** @defgroup SPI_TXFIFOEmptyThreshold
  * @{
  */
#define SPI_TX_FIFO_EMPTY_THRESHOLD_0		 						((uint32_t)0x0000)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_1       				 		((uint32_t)0x0001)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_2        						((uint32_t)0x0002)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_3        						((uint32_t)0x0003)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_4        						((uint32_t)0x0004)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_5        						((uint32_t)0x0005)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_6        						((uint32_t)0x0006)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_7        						((uint32_t)0x0007)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_8        						((uint32_t)0x0008)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_9        						((uint32_t)0x0009)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_10       						((uint32_t)0x000A)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_11       						((uint32_t)0x000B)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_12       						((uint32_t)0x000C)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_13       						((uint32_t)0x000D)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_14       						((uint32_t)0x000E)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_15       						((uint32_t)0x000F)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_31		  						((uint32_t)0x001F)
#define SPI_TX_FIFO_EMPTY_THRESHOLD_63       						((uint32_t)0x003F)

#define IS_SPI_TX_FIFO_EMPTY_THRESHOLD(THRESHOLD)					((((int32_t)(THRESHOLD)) >= SPI_TX_FIFO_EMPTY_THRESHOLD_0) && \
																	(((int32_t)(THRESHOLD)) <= SPI_TX_FIFO_EMPTY_THRESHOLD_63))
/**
  * @}
  */ 


/** @defgroup SPI_TXFIFOFullThreshold 
  * @{
  */
#define SPI_RX_FIFO_FULL_THRESHOLD_0								((uint32_t)0x0000)
#define SPI_TX_FIFO_FULL_THRESHOLD_1       							((uint32_t)0x0001)
#define SPI_TX_FIFO_FULL_THRESHOLD_2        					 	((uint32_t)0x0002)
#define SPI_TX_FIFO_FULL_THRESHOLD_3       							((uint32_t)0x0003)
#define SPI_TX_FIFO_FULL_THRESHOLD_4        						((uint32_t)0x0004)
#define SPI_TX_FIFO_FULL_THRESHOLD_5        						((uint32_t)0x0005)
#define SPI_TX_FIFO_FULL_THRESHOLD_6        						((uint32_t)0x0006)
#define SPI_TX_FIFO_FULL_THRESHOLD_7        						((uint32_t)0x0007)
#define SPI_TX_FIFO_FULL_THRESHOLD_8        						((uint32_t)0x0008)
#define SPI_TX_FIFO_FULL_THRESHOLD_9        						((uint32_t)0x0009)
#define SPI_TX_FIFO_FULL_THRESHOLD_10       						((uint32_t)0x000A)
#define SPI_TX_FIFO_FULL_THRESHOLD_11       						((uint32_t)0x000B)
#define SPI_TX_FIFO_FULL_THRESHOLD_12       						((uint32_t)0x000C)
#define SPI_TX_FIFO_FULL_THRESHOLD_13       						((uint32_t)0x000D)
#define SPI_TX_FIFO_FULL_THRESHOLD_14       						((uint32_t)0x000E)
#define SPI_TX_FIFO_FULL_THRESHOLD_15       						((uint32_t)0x000F)
#define SPI_TX_FIFO_FULL_THRESHOLD_16       						((uint32_t)0x0010)
#define SPI_TX_FIFO_FULL_THRESHOLD_32       						((uint32_t)0x0020)
#define SPI_TX_FIFO_FULL_THRESHOLD_64       						((uint32_t)0x0040)


#define IS_SPI_TX_FIFO_FULL_THRESHOLD(THRESHOLD)					((((int32_t)(THRESHOLD)) >= SPI_TX_FIFO_FULL_THRESHOLD_0) && \
																	(((int32_t)(THRESHOLD)) <= SPI_TX_FIFO_FULL_THRESHOLD_64))														
/**
  * @}
  */ 
  
  
 /** @defgroup SPI_RXFIFOEmptyThreshold
  * @{
  */
#define SPI_RX_FIFO_EMPTY_THRESHOLD_0		 						((uint32_t)0x0000)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_1       				 		((uint32_t)0x0001)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_2        						((uint32_t)0x0002)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_3        						((uint32_t)0x0003)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_4        						((uint32_t)0x0004)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_5        						((uint32_t)0x0005)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_6        						((uint32_t)0x0006)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_7        						((uint32_t)0x0007)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_8        						((uint32_t)0x0008)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_9        						((uint32_t)0x0009)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_10       						((uint32_t)0x000A)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_11       						((uint32_t)0x000B)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_12       						((uint32_t)0x000C)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_13       						((uint32_t)0x000D)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_14       						((uint32_t)0x000E)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_15       						((uint32_t)0x000F)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_31		  						((uint32_t)0x001F)
#define SPI_RX_FIFO_EMPTY_THRESHOLD_63       						((uint32_t)0x003F)

#define IS_SPI_RX_FIFO_EMPTY_THRESHOLD(THRESHOLD)					((((int32_t)(THRESHOLD)) >= SPI_RX_FIFO_EMPTY_THRESHOLD_0) && \
																	(((int32_t)(THRESHOLD)) <= SPI_RX_FIFO_EMPTY_THRESHOLD_63))															
/**
  * @}
  */ 

/** @defgroup SPI_RXFIFOFullThreshold 
  * @{
  */
#define SPI_RX_FIFO_FULL_THRESHOLD_0       						 	((uint32_t)0x0000)
#define SPI_RX_FIFO_FULL_THRESHOLD_1       						 	((uint32_t)0x0001)
#define SPI_RX_FIFO_FULL_THRESHOLD_2        					 	((uint32_t)0x0002)
#define SPI_RX_FIFO_FULL_THRESHOLD_3       							((uint32_t)0x0003)
#define SPI_RX_FIFO_FULL_THRESHOLD_4        						((uint32_t)0x0004)
#define SPI_RX_FIFO_FULL_THRESHOLD_5        						((uint32_t)0x0005)
#define SPI_RX_FIFO_FULL_THRESHOLD_6        						((uint32_t)0x0006)
#define SPI_RX_FIFO_FULL_THRESHOLD_7        						((uint32_t)0x0007)
#define SPI_RX_FIFO_FULL_THRESHOLD_8        						((uint32_t)0x0008)
#define SPI_RX_FIFO_FULL_THRESHOLD_9        						((uint32_t)0x0009)
#define SPI_RX_FIFO_FULL_THRESHOLD_10       						((uint32_t)0x000A)
#define SPI_RX_FIFO_FULL_THRESHOLD_11       						((uint32_t)0x000B)
#define SPI_RX_FIFO_FULL_THRESHOLD_12       						((uint32_t)0x000C)
#define SPI_RX_FIFO_FULL_THRESHOLD_13       						((uint32_t)0x000D)
#define SPI_RX_FIFO_FULL_THRESHOLD_14       						((uint32_t)0x000E)
#define SPI_RX_FIFO_FULL_THRESHOLD_15       						((uint32_t)0x000F)
#define SPI_RX_FIFO_FULL_THRESHOLD_16       						((uint32_t)0x0010)
#define SPI_RX_FIFO_FULL_THRESHOLD_32       						((uint32_t)0x0020)
#define SPI_RX_FIFO_FULL_THRESHOLD_64       						((uint32_t)0x0040)


#define IS_SPI_RX_FIFO_FULL_THRESHOLD(THRESHOLD)					((((int32_t)(THRESHOLD)) >= SPI_RX_FIFO_FULL_THRESHOLD_0) && \
																	(((int32_t)(THRESHOLD)) <= SPI_RX_FIFO_FULL_THRESHOLD_64))
														
/**
  * @}
  */ 
  

/** @defgroup SPI_DMAReceiveLevel 
  * @{
  */
#define SPI_DMA_RECEIVE_LEVEL_1				    					((uint32_t)0x0001)
#define SPI_DMA_RECEIVE_LEVEL_2			        					((uint32_t)0x0002)
#define SPI_DMA_RECEIVE_LEVEL_3										((uint32_t)0x0003)
#define SPI_DMA_RECEIVE_LEVEL_4										((uint32_t)0x0004)
#define SPI_DMA_RECEIVE_LEVEL_5										((uint32_t)0x0005)
#define SPI_DMA_RECEIVE_LEVEL_6										((uint32_t)0x0006)
#define SPI_DMA_RECEIVE_LEVEL_7										((uint32_t)0x0007)
#define SPI_DMA_RECEIVE_LEVEL_8										((uint32_t)0x0008)
#define SPI_DMA_RECEIVE_LEVEL_9										((uint32_t)0x0009)
#define SPI_DMA_RECEIVE_LEVEL_10									((uint32_t)0x000A)
#define SPI_DMA_RECEIVE_LEVEL_11									((uint32_t)0x000B)
#define SPI_DMA_RECEIVE_LEVEL_12									((uint32_t)0x000C)
#define SPI_DMA_RECEIVE_LEVEL_13									((uint32_t)0x000D)
#define SPI_DMA_RECEIVE_LEVEL_14									((uint32_t)0x000E)
#define SPI_DMA_RECEIVE_LEVEL_15									((uint32_t)0x000F)
#define SPI_DMA_RECEIVE_LEVEL_16									((uint32_t)0x0010)
#define SPI_DMA_RECEIVE_LEVEL_32									((uint32_t)0x0020)
#define SPI_DMA_RECEIVE_LEVEL_64									((uint32_t)0x0040)

#define IS_SPI_DMA_RECEIVE_LEVEL(LEVEL)								((((int32_t)(LEVEL)) >= SPI_DMA_RECEIVE_LEVEL_1) && \
																	(((int32_t)(LEVEL)) <= SPI_DMA_RECEIVE_LEVEL_64))
/**
  * @}
  */ 

/** @defgroup SPI_DMATransmitLevel 
  * @{
  */
#define SPI_DMA_TRANSMIT_LEVEL_0									((uint32_t)0x0000)
#define SPI_DMA_TRANSMIT_LEVEL_1									((uint32_t)0x0001)
#define SPI_DMA_TRANSMIT_LEVEL_2									((uint32_t)0x0002)
#define SPI_DMA_TRANSMIT_LEVEL_3									((uint32_t)0x0003)
#define SPI_DMA_TRANSMIT_LEVEL_4									((uint32_t)0x0004)
#define SPI_DMA_TRANSMIT_LEVEL_5									((uint32_t)0x0005)
#define SPI_DMA_TRANSMIT_LEVEL_6									((uint32_t)0x0006)
#define SPI_DMA_TRANSMIT_LEVEL_7									((uint32_t)0x0007)
#define SPI_DMA_TRANSMIT_LEVEL_8									((uint32_t)0x0008)
#define SPI_DMA_TRANSMIT_LEVEL_9									((uint32_t)0x0009)
#define SPI_DMA_TRANSMIT_LEVEL_10									((uint32_t)0x000A)
#define SPI_DMA_TRANSMIT_LEVEL_11									((uint32_t)0x000B)
#define SPI_DMA_TRANSMIT_LEVEL_12									((uint32_t)0x000C)
#define SPI_DMA_TRANSMIT_LEVEL_13									((uint32_t)0x000D)
#define SPI_DMA_TRANSMIT_LEVEL_14									((uint32_t)0x000E)
#define SPI_DMA_TRANSMIT_LEVEL_15									((uint32_t)0x000F)
#define SPI_DMA_TRANSMIT_LEVEL_31									((uint32_t)0x001F)
#define SPI_DMA_TRANSMIT_LEVEL_63									((uint32_t)0x003F)

#define IS_SPI_DMA_TRANSMIT_LEVEL(LEVEL)							((((int32_t)(LEVEL)) >= SPI_DMA_TRANSMIT_LEVEL_0) && \
																	(((int32_t)(LEVEL)) <= SPI_DMA_TRANSMIT_LEVEL_63))
/**
  * @}
  */ 
  
/** @defgroup SPI FIFO Reset
  * @{
  */


#define SPI_FIFO_TX													((uint32_t)0x0080)
#define SPI_FIFO_RX													((uint32_t)0x0040)

/**
  * @}
  */ 
  
/** @defgroup SPI_Interrupt Mask and Status Flag 
  * @{
  */
#define SPI_IT_EN													((uint32_t)0x000004)
#define SPI_IT_RX													((uint32_t)0x200000)
#define SPI_IT_TX													((uint32_t)0x400000)

#define SPI_IT_ID_RXF												((uint32_t)0x0080)	 
#define SPI_IT_ID_RXTF												((uint32_t)0x0040)      							
#define SPI_IT_ID_RXE												((uint32_t)0x0020)	 
#define SPI_IT_ID_RXTE												((uint32_t)0x0010)	 	


#define SPI_IT_ID_TXF												((uint32_t)0x0008)
#define SPI_IT_ID_TXE												((uint32_t)0x0002)
#define SPI_IT_ID_TXTE												((uint32_t)0x0001)

#define SPI_FLAG_RXE												((uint32_t)0x0400)
#define SPI_FLAG_RXF												((uint32_t)0x1000)
#define SPI_FLAG_RXTF												((uint32_t)0x2000)

#define SPI_FLAG_TXE												((uint32_t)0x0004)
#define SPI_FLAG_TXF												((uint32_t)0x0010)
#define SPI_FLAG_TXTF												((uint32_t)0x0020)		

/**
  * @}
  */ 
 
void SPI_Init(SPI_InitTypeDef* SPI_InitStruct);
void SPI_DMAInit(SPI_DMAInitTypeDef* QSPI_DMAInitStruct); 
void SPI_FIFOReset(uint32_t SPI_FIFO);
void SPI_ITConfig(uint32_t SPI_IT, FunctionalState NewState);
void SPI_SendData(uint8_t Data);
uint8_t SPI_ReceiveData(void);
FlagStatus SPI_GetFlagStatus(uint32_t SPI_Flag);
ITStatus SPI_GetITStatus(uint32_t SPI_IT);

#ifdef __cplusplus
}
#endif
#endif

/**************************      (C) COPYRIGHT    *****END OF FILE**************************************/
