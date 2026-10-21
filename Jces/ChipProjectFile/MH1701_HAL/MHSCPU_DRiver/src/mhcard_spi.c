/************************ (C) COPYRIGHT *************************************************
 * @file                : mhscpu_spi.c
 * @author              : 
 * @version             : V1.0.0
 * @date                : 29-June-2019
 * @brief               : This file provides all the UART firmware functions
 ******************************************************************************************************/
 
#include "mhcard_spi.h"

/** @defgroup SPI_Private_Defines
  * @{
  */
  
#define SPI_CR_CLEAR_MASK											((uint32_t)~0x180007)
#define SPI_CR_DMA_TRANSMIT_LEVEL_CLEAR_MASK						((uint32_t)~0x7F800)
#define SPI_CR_DMA_RECEIVE_LEVEL_CLEAR_MASK							((uint32_t)~0x7F8)
#define SPI_FCR_CLEAR_MASK											((uint32_t)~0x3F3F3F00)

#define SPI_CR_PARAM_SAMPLE_POS										(0x13)
#define SPI_CR_PARAM_DMA_TRANSMIT_LEVEL_POS 						(0x0C)
#define SPI_CR_PARAM_DMA_RECEIVE_LEVEL_POS							(0x04)
#define SPI_CR_PARAM_DMA_TRANSMIT_ENABLE_POS						(0x0B)
#define SPI_CR_PARAM_DMA_RECEIVE_ENABLE_POS							(0x03)
#define SPI_CR_PARAM_INTERRPUT_ENABLE_POS							(0x02)
#define SPI_CR_PARAM_RECEIVE_ENABLE_POS								(0x01)
#define SPI_CR_PARAM_TRANSMIT_ENABLE_POS							(0x00)

#define SPI_FCR_PARAM_TRANSIMIT_FIFO_FULL_THRESHOULD_POS			(0x00)
#define SPI_FCR_PARAM_TRANSIMIT_FIFO_EMPTY_THRESHOULD_POS			(0x08)
#define SPI_FCR_PARAM_RECEIVE_FIFO_FULL_THRESHOULD_POS				(0x10)
#define SPI_FCR_PARAM_REVEIVE_FIFO_EMPTY_THRESHOULD_POS				(0x18)

  
 /**
  * @}
  */ 
  
 
/**
  * @brief  Initializes the SPI peripheral according to the specified 
  *         parameters in the SPI_InitStruct.
  * @param  SPI_InitStruct: pointer to a SPI_InitTypeDef structure that
  *         contains the configuration information for the specified SPI peripheral.
  * @retval None
  */
  
void SPI_Init(SPI_InitTypeDef* SPI_InitStruct)
{
	SPIS->FCR = (uint32_t)((SPIS->FCR & SPI_FCR_CLEAR_MASK)
						| ((SPI_InitStruct->SPI_TXFIFOEmptyThreshold & 0x3F) << SPI_FCR_PARAM_TRANSIMIT_FIFO_EMPTY_THRESHOULD_POS)
						| ((SPI_InitStruct->SPI_RXFIFOFullThreshold & 0x3F) << SPI_FCR_PARAM_RECEIVE_FIFO_FULL_THRESHOULD_POS));
	
	SPIS->CR = (uint32_t)((SPIS->CR & SPI_CR_CLEAR_MASK)
						| ((SPI_InitStruct->SPI_TXEnable & 0x01) << SPI_CR_PARAM_TRANSMIT_ENABLE_POS)
						| ((SPI_InitStruct->SPI_RXEnable & 0x01) << SPI_CR_PARAM_RECEIVE_ENABLE_POS)
						| ((SPI_InitStruct->SPI_Sample & 0x03) << SPI_CR_PARAM_SAMPLE_POS));
}


/**
  * @brief  Initializes the SPI DMA interface according to the specified parameters.
  * @param  SPI_DMAInitStruct: pointer to a SPI_DMAInitStruct structure that contains the configuration information.
  * @retval None
  */

void SPI_DMAInit(SPI_DMAInitTypeDef* SPI_DMAInitStruct)
{
	if (ENABLE == SPI_DMAInitStruct->SPI_DMAReceiveEnable)
	{	
		SPIS->CR = (uint32_t)((SPIS->CR & SPI_CR_DMA_RECEIVE_LEVEL_CLEAR_MASK)
						| ((SPI_DMAInitStruct->SPI_DMAReceiveLevel & 0x7F) << SPI_CR_PARAM_DMA_RECEIVE_LEVEL_POS)
						| ((SPI_DMAInitStruct->SPI_DMAReceiveEnable & 0x01) << SPI_CR_PARAM_DMA_RECEIVE_ENABLE_POS));
	}
	else
	{
		SPIS->CR &= ~(uint32_t)((~SPI_DMAInitStruct->SPI_DMAReceiveEnable & 0x01) << SPI_CR_PARAM_DMA_RECEIVE_ENABLE_POS);
	}
	
	if (ENABLE == SPI_DMAInitStruct->SPI_DMATransmitEnable)
	{
		SPIS->CR = (uint32_t)((SPIS->CR & SPI_CR_DMA_TRANSMIT_LEVEL_CLEAR_MASK)
						| ((SPI_DMAInitStruct->SPI_DMATransmitLevel & 0x7F) << SPI_CR_PARAM_DMA_TRANSMIT_LEVEL_POS)
						| ((SPI_DMAInitStruct->SPI_DMATransmitEnable & 0x01) << SPI_CR_PARAM_DMA_TRANSMIT_ENABLE_POS));
	}
	else 
	{
		SPIS->CR &= ~(uint32_t)((~SPI_DMAInitStruct->SPI_DMATransmitEnable & 0x01) << SPI_CR_PARAM_DMA_TRANSMIT_ENABLE_POS);
	}															 					  
}


/**
  * @brief  Transmits single data through the SPI peripheral.
  * @param  data: the data to be transmitted
  * @retval None
  */
void SPI_SendData(uint8_t Data)
{
	SPIS->WDR = Data;
}

/**
  * @brief  Receive single data through the SPI peripheral.
  * @retval The received data
  */
uint8_t SPI_ReceiveData(void)
{
	return (uint8_t)SPIS->RDR;
}


/**
  *@brief  Enable or disable the specified SPI interrupt.
  */

void SPI_ITConfig(uint32_t SPI_IT, FunctionalState NewState)
{
	if (NewState != DISABLE)
	{
		SPIS->CR |= SPI_IT;
	}
	else
	{
		SPIS->CR &= ~SPI_IT;
	}
}

/**
  * @brief  Checks whether the specified SPI flag is set or not.
  * @retval The new state of SPI_FLAG (SET or RESET).
  */
FlagStatus SPI_GetFlagStatus(uint32_t SPI_Flag)
{
	if ((SPIS->SR & SPI_Flag) != RESET)
	{
		return SET;
	} 
	return RESET;
}

/**
  * @brief  Checks whether the specified SPI interrupt has occurred or not.
  * @retval The new state of SPI_IT (SET or RESET).
  */
ITStatus SPI_GetITStatus(uint32_t SPI_IT)
{
	uint8_t itmask = 0;
	
	itmask = (uint8_t)SPIS->RISR;
	itmask <<= 4;
	itmask = (uint8_t)(itmask | SPIS->TISR);
	
	if ((itmask & SPI_IT) != RESET)
	{
		return SET;
	}	
	return RESET;
}

/**
  * @brief Reset SPI FIFO(TX or RX).
  */
void SPI_FIFOReset(uint32_t SPI_FIFO)
{
	SPIS->FCR |= SPI_FIFO;
	
	SPIS->FCR &= ~SPI_FIFO;
}


/**************************      (C) COPYRIGHT     *****END OF FILE**************************/
