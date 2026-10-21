/************************ (C) COPYRIGHT **************************************************************
 * @file                : mhscpu_qspi.h
 * @author              :
 * @version             : V1.0.0
 * @date                : 29-June-2019
 * @brief               : This file contains all the functions prototypes for the QSPI firmware library
 *******************************************************************************************************************/
 
#ifndef __MHCARD_QSPI_H
#define __MHCARD_QSPI_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "mhcard.h"

typedef struct
{
	__IO uint32_t CR0;				/*!< QSPI Control register0		 		*/
	__IO uint32_t FLCR;				/*!< QSPI Flow Control register	 		*/
	__IO uint32_t FLSR;  			/*!< QSPI Flow Status register 			*/
	__IO uint32_t FCR;				/*!< QSPI FIFO Control register 		*/
	__I  uint32_t RDR;				/*!< QSPI Read Data register 			*/
	__O  uint32_t WDR;				/*!< QSPI Write Data register 			*/
	__I  uint32_t SR;				/*!< QSPI Status register 				*/
	__IO uint32_t CR1;				/*!< QSPI Control register1 			*/
	__IO uint32_t FSR;				/*!< QSPI FIFO Status register			*/
	__IO uint32_t DCR;				/*!< QSPI DMA Control register 			*/	
	__I  uint32_t TISR;				/*!< QSPI TX Interrput Status register	*/	
	__I  uint32_t RISR;				/*!< QSPI RX Interrput Status register	*/	
}QSPI_TypeDef;

#define QSPI_BASE                               (MHCARD_APB_BASE + 0x4020)
#define QSPI									((QSPI_TypeDef *) QSPI_BASE)

typedef struct 
{
	uint8_t QSPI_Enable;
	uint8_t QSPI_Mode;
	uint8_t QSPI_FirstBit;
	uint8_t QSPI_CPOL;
	uint8_t QSPI_CPHA;
	uint8_t QSPI_DivideEnable;	
	uint8_t QSPI_BaudRatePrescaler;
	uint8_t QSPI_ReceiveDataLength;
	uint8_t QSPI_RXFIFOFullThreshold;
	uint8_t QSPI_TXFIFOEmptyThreshold;
}QSPI_InitTypeDef;

typedef struct 
{
	uint8_t QSPI_DMAReceiveEnable;
	uint8_t QSPI_DMATransmitEnable;
	uint8_t QSPI_DMAReceiveLevel;
	uint8_t QSPI_DMATransmitLevel;
}QSPI_DMAInitTypeDef;
	

/** @defgroup QSPI_Mode 
  * @{
  */
  
#define QSPI_MODE_STANDARD											(0x01)
#define QSPI_MODE_DUAL_RECEIVE										(0x02)
#define QSPI_MODE_DUAL_TRANSMIT										(0x03)
#define QSPI_MODE_QUAD_RECEIVE										(0x04)
#define QSPI_MODE_QUAD_TRANSMIT										(0x05)
#define IS_QSPI_MODE(MODE)											(((MODE) == QSPI_MODE_STANDARD)			|| \
																	((MODE) == QSPI_MODE_DUAL_RECEIVE)		|| \
																	((MODE) == QSPI_MODE_DUAL_TRANSMIT)		|| \
																	((MODE) == QSPI_MODE_QUAD_RECEIVE)		|| \
																	((MODE) == QSPI_MODE_QUAD_TRANSMIT))																
/**
  * @}
  */  

/** @defgroup QSPI_Clock_Polarity 
  * @{
  */

#define QSPI_CPOL_LOW												(0x00)
#define QSPI_CPOL_HIGH												(0x01)
#define IS_QSPI_CPOL(CPOL)											(((CPOL) == QSPI_CPOL_LOW)|| \
																	((CPOL) == QSPI_CPOL_HIGH)) 
/**
  * @}
  */				
  
																
/** @defgroup QSPI_Clock_Phase 
  * @{
  */
  
#define QSPI_CPHA_1EDGE												(0x00)
#define QSPI_CPHA_2EDGE												(0x01)
#define IS_QSPI_CPHA(CPHA)											(((CPHA) == QSPI_CPHA_1EDGE) || \
																	(CPHA) == QSPI_CPHA_2EDGE)
/**
  * @}
  */	

/** @defgroup QSPI_First_Bit 
  * @{
  */
  
#define QSPI_FIRST_BIT_MSB											(0x00)
#define QSPI_FIRST_BIT_LSB											(0x01)
#define IS_QSPI_FIRST_BIT(BIT)   									(((BIT) == QSPI_FIRST_BIT_MSB) || \
																	((BIT) == QSPI_FIRST_BIT_LSB))
/**
  * @}
  */	
															 
/** @defgroup QSPI_BaudRate_Prescaler 
  * @{
  */
#define QSPI_BAUDRATE_PRESCALER_2         							(0x01)
#define QSPI_BAUDRATE_PRESCALER_4        							(0x02)
#define QSPI_BAUDRATE_PRESCALER_8        							(0x04)
#define QSPI_BAUDRATE_PRESCALER_10        							(0x05)
#define QSPI_BAUDRATE_PRESCALER_12        							(0x06)
#define QSPI_BAUDRATE_PRESCALER_16        							(0x08)
#define QSPI_BAUDRATE_PRESCALER_32         							(0x10)
#define QSPI_BAUDRATE_PRESCALER_64       							(0x20)
#define QSPI_BAUDRATE_PRESCALER_128        							(0x40)
#define QSPI_BAUDRATE_PRESCALER_256       							(0x80)
#define IS_QSPI_BAUDRATE_PRESCALER(PRESCALER)  						(((PRESCALER) == QSPI_BAUDRATE_PRESCALER_2) || \
																	((PRESCALER) == QSPI_BAUDRATE_PRESCALER_4)	|| \
																	((PRESCALER) == QSPI_BAUDRATE_PRESCALER_8)	|| \
                                                                    ((PRESCALER) == QSPI_BAUDRATE_PRESCALER_10)	|| \
                                                                    ((PRESCALER) == QSPI_BAUDRATE_PRESCALER_12)	|| \
                                                                    ((PRESCALER) == QSPI_BAUDRATE_PRESCALER_16)	|| \
                                                                    ((PRESCALER) == QSPI_BAUDRATE_PRESCALER_32)	|| \
                                                                    ((PRESCALER) == QSPI_BAUDRATE_PRESCALER_64)	|| \
                                                                    ((PRESCALER) == QSPI_BAUDRATE_PRESCALER_128)|| \
																	((PRESCALER) == QSPI_BAUDRATE_PRESCALER_256))
/**
  * @}
  */ 

/** @defgroup QSPI_Receive_Data_Length 
  * @{
  */
#define QSPI_RECEIVE_DATA_LENGTH_1_BYTE         					(0x00)
#define QSPI_RECEIVE_DATA_LENGTH_2_BYTE         					(0x01)
#define QSPI_RECEIVE_DATA_LENGTH_3_BYTE         					(0x02)
#define QSPI_RECEIVE_DATA_LENGTH_4_BYTE         					(0x03)
#define QSPI_RECEIVE_DATA_LENGTH_5_BYTE         					(0x04)
#define QSPI_RECEIVE_DATA_LENGTH_6_BYTE         					(0x05)
#define QSPI_RECEIVE_DATA_LENGTH_7_BYTE         					(0x06)
#define QSPI_RECEIVE_DATA_LENGTH_8_BYTE         					(0x07)
#define QSPI_RECEIVE_DATA_LENGTH_9_BYTE         					(0x08)
#define QSPI_RECEIVE_DATA_LENGTH_10_BYTE         					(0x09
#define QSPI_RECEIVE_DATA_LENGTH_11_BYTE         					(0x0A)
#define QSPI_RECEIVE_DATA_LENGTH_12_BYTE         					(0x0B)
#define QSPI_RECEIVE_DATA_LENGTH_13_BYTE         					(0x0C)
#define QSPI_RECEIVE_DATA_LENGTH_14_BYTE         					(0x0D)
#define QSPI_RECEIVE_DATA_LENGTH_15_BYTE         					(0x0E)
#define QSPI_RECEIVE_DATA_LENGTH_16_BYTE         					(0x0F)
#define QSPI_RECEIVE_DATA_LENGTH_32_BYTE							(0x1F)

/**
  * @}
  */

/** @defgroup QSPI_TXFIFOEmptyThreshold
  * @{
  */
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_0		 						((uint32_t)0x0000)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_1       				 		((uint32_t)0x0001)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_2        						((uint32_t)0x0002)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_3        						((uint32_t)0x0003)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_4        						((uint32_t)0x0004)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_5        						((uint32_t)0x0005)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_6        						((uint32_t)0x0006)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_7        						((uint32_t)0x0007)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_8        						((uint32_t)0x0008)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_9        						((uint32_t)0x0009)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_10       						((uint32_t)0x000A)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_11       						((uint32_t)0x000B)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_12       						((uint32_t)0x000C)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_13       						((uint32_t)0x000D)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_14       						((uint32_t)0x000E)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_15       						((uint32_t)0x000F)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_31		  						((uint32_t)0x001F)
#define QSPI_TX_FIFO_EMPTY_THRESHOLD_63       						((uint32_t)0x003F)

#define IS_QSPI_TX_FIFO_EMPTY_THRESHOLD(THRESHOLD)					((((int32_t)(THRESHOLD)) >= QSPI_TX_FIFO_EMPTY_THRESHOLD_0) && \
																	(((uint32_t)(THRESHOLD)) <= QSPI_TX_FIFO_EMPTY_THRESHOLD_63))
/**
  * @}
  */ 

   
/** @defgroup QSPI_RXFIFOFullThreshold 
  * @{
  */
#define QSPI_RX_FIFO_FULL_THRESHOLD_1       						((uint32_t)0x0001)
#define QSPI_RX_FIFO_FULL_THRESHOLD_2        					 	((uint32_t)0x0002)
#define QSPI_RX_FIFO_FULL_THRESHOLD_3       						((uint32_t)0x0003)
#define QSPI_RX_FIFO_FULL_THRESHOLD_4        						((uint32_t)0x0004)
#define QSPI_RX_FIFO_FULL_THRESHOLD_5        						((uint32_t)0x0005)
#define QSPI_RX_FIFO_FULL_THRESHOLD_6        						((uint32_t)0x0006)
#define QSPI_RX_FIFO_FULL_THRESHOLD_7        						((uint32_t)0x0007)
#define QSPI_RX_FIFO_FULL_THRESHOLD_8        						((uint32_t)0x0008)
#define QSPI_RX_FIFO_FULL_THRESHOLD_9        						((uint32_t)0x0009)
#define QSPI_RX_FIFO_FULL_THRESHOLD_10       						((uint32_t)0x000A)
#define QSPI_RX_FIFO_FULL_THRESHOLD_11       						((uint32_t)0x000B)
#define QSPI_RX_FIFO_FULL_THRESHOLD_12       						((uint32_t)0x000C)
#define QSPI_RX_FIFO_FULL_THRESHOLD_13       						((uint32_t)0x000D)
#define QSPI_RX_FIFO_FULL_THRESHOLD_14       						((uint32_t)0x000E)
#define QSPI_RX_FIFO_FULL_THRESHOLD_15       						((uint32_t)0x000F)
#define QSPI_RX_FIFO_FULL_THRESHOLD_16       						((uint32_t)0x0010)
#define QSPI_RX_FIFO_FULL_THRESHOLD_32       						((uint32_t)0x0020)
#define QSPI_RX_FIFO_FULL_THRESHOLD_63       						((uint32_t)0x003F)


#define IS_QSPI_RX_FIFO_FULL_THRESHOLD(THRESHOLD)					((((uint32_t)(THRESHOLD)) >= QSPI_RX_FIFO_FULL_THRESHOLD_1) && \
																	(((uint32_t)(THRESHOLD)) <= QSPI_RX_FIFO_FULL_THRESHOLD_63))
														
/**
  * @}
  */ 
   
/** @defgroup SPI_DMAReceiveLevel 
  * @{
  */
#define QSPI_DMA_RECEIVE_LEVEL_1				    				((uint32_t)0x0001)
#define QSPI_DMA_RECEIVE_LEVEL_2			        				((uint32_t)0x0002)
#define QSPI_DMA_RECEIVE_LEVEL_3									((uint32_t)0x0003)
#define QSPI_DMA_RECEIVE_LEVEL_4									((uint32_t)0x0004)
#define QSPI_DMA_RECEIVE_LEVEL_5									((uint32_t)0x0005)
#define QSPI_DMA_RECEIVE_LEVEL_6									((uint32_t)0x0006)
#define QSPI_DMA_RECEIVE_LEVEL_7									((uint32_t)0x0007)
#define QSPI_DMA_RECEIVE_LEVEL_8									((uint32_t)0x0008)
#define QSPI_DMA_RECEIVE_LEVEL_9									((uint32_t)0x0009)
#define QSPI_DMA_RECEIVE_LEVEL_10									((uint32_t)0x000A)
#define QSPI_DMA_RECEIVE_LEVEL_11									((uint32_t)0x000B)
#define QSPI_DMA_RECEIVE_LEVEL_12									((uint32_t)0x000C)
#define QSPI_DMA_RECEIVE_LEVEL_13									((uint32_t)0x000D)
#define QSPI_DMA_RECEIVE_LEVEL_14									((uint32_t)0x000E)
#define QSPI_DMA_RECEIVE_LEVEL_15									((uint32_t)0x000F)
#define QSPI_DMA_RECEIVE_LEVEL_16									((uint32_t)0x0010)
#define QSPI_DMA_RECEIVE_LEVEL_32									((uint32_t)0x0020)
#define QSPI_DMA_RECEIVE_LEVEL_64									((uint32_t)0x0040)

#define IS_QSPI_DMA_RECEIVE_LEVEL(LEVEL)							((((uint32_t)(LEVEL)) >= QSPI_DMA_RECEIVE_LEVEL_1) && \
																	(((uint32_t)(LEVEL)) <= QSPI_DMA_RECEIVE_LEVEL_64))
/**
  * @}
  */ 

/** @defgroup SPI_DMATransmitLevel 
  * @{
  */
#define QSPI_DMA_TRANSMIT_LEVEL_1									((uint32_t)0x0001)
#define QSPI_DMA_TRANSMIT_LEVEL_2									((uint32_t)0x0002)
#define QSPI_DMA_TRANSMIT_LEVEL_3									((uint32_t)0x0003)
#define QSPI_DMA_TRANSMIT_LEVEL_4									((uint32_t)0x0004)
#define QSPI_DMA_TRANSMIT_LEVEL_5									((uint32_t)0x0005)
#define QSPI_DMA_TRANSMIT_LEVEL_6									((uint32_t)0x0006)
#define QSPI_DMA_TRANSMIT_LEVEL_7									((uint32_t)0x0007)
#define QSPI_DMA_TRANSMIT_LEVEL_8									((uint32_t)0x0008)
#define QSPI_DMA_TRANSMIT_LEVEL_9									((uint32_t)0x0009)
#define QSPI_DMA_TRANSMIT_LEVEL_10									((uint32_t)0x000A)
#define QSPI_DMA_TRANSMIT_LEVEL_11									((uint32_t)0x000B)
#define QSPI_DMA_TRANSMIT_LEVEL_12									((uint32_t)0x000C)
#define QSPI_DMA_TRANSMIT_LEVEL_13									((uint32_t)0x000D)
#define QSPI_DMA_TRANSMIT_LEVEL_14									((uint32_t)0x000E)
#define QSPI_DMA_TRANSMIT_LEVEL_15									((uint32_t)0x000F)
#define QSPI_DMA_TRANSMIT_LEVEL_31									((uint32_t)0x001F)
#define QSPI_DMA_TRANSMIT_LEVEL_63									((uint32_t)0x003F)

#define IS_QSPI_DMA_TRANSMIT_LEVEL(LEVEL)							((((uint32_t)(LEVEL)) >= QSPI_DMA_TRANSMIT_LEVEL_1) && \
																	(((uint32_t)(LEVEL)) <= QSPI_DMA_TRANSMIT_LEVEL_63))
/**
  * @}
  */		
  																
/** @defgroup QSPI_Interrupt Mask and Status Flag 
  * @{
  */
  
#define QSPI_IT_RX													((uint32_t)0x8000)							
#define QSPI_IT_TX													((uint32_t)0x4000)
#define QSPI_IT_EN													((uint32_t)0x2000)

#define QSPI_IT_ID_RXF												((uint32_t)0x0080)	
#define QSPI_IT_ID_RXTF												((uint32_t)0x0040)     							
#define QSPI_IT_ID_RXE												((uint32_t)0x0020)	
#define QSPI_IT_ID_RXTE												((uint32_t)0x0010)

#define QSPI_IT_ID_TXF												((uint32_t)0x0008)
#define QSPI_IT_ID_TXTF												((uint32_t)0x0004)
#define QSPI_IT_ID_TXE												((uint32_t)0x0002)
#define QSPI_IT_ID_TXTE												((uint32_t)0x0001)

#define QSPI_FLAG_TXE 				   								((uint32_t)0x0004)
#define QSPI_FLAG_TXF												((uint32_t)0x0010)
#define QSPI_FLAG_TXTF												((uint32_t)0x0020)

#define QSPI_FLAG_RXE												((uint32_t)0x0400)
#define QSPI_FLAG_RXF												((uint32_t)0x1000)
#define QSPI_FLAG_RXTF												((uint32_t)0x2000)

#define QSPI_FIFO_TX												((uint32_t)0x0080)
#define QSPI_FIFO_RX												((uint32_t)0x0040)

/**
  * @}
  */
	
void QSPI_Init(QSPI_InitTypeDef* QSPI_InitStruct);
void QSPI_DMAInit(QSPI_DMAInitTypeDef* QSPI_DMAInitStruct);	
void QSPI_Cmd(FunctionalState NewState);
void QSPI_TransmitCmd(FunctionalState NewState);
void QSPI_ReceiveCmd(FunctionalState NewState);
void QSPI_SendData(uint8_t Data);
uint8_t QSPI_ReceiveData(void);
void QSPI_ITConfig(uint32_t QSPI_IT, FunctionalState NewState);
void QSPI_FIFOReset(uint32_t QSPI_IT);
FlagStatus QSPI_IsTransmitDone(void);
FlagStatus QSPI_IsReady(void);
FlagStatus QSPI_GetFlagStatus(uint32_t QSPI_Flag);
ITStatus QSPI_GetITStatus(uint32_t QSPI_IT);

#ifdef __cplusplus
}
#endif
#endif

/**************************      (C) COPYRIGHT    *****END OF FILE***********************************************************************/
