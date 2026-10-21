/************************ (C) COPYRIGHT ******************************************************************
 * @file                : mhscpu_qspi.c
 * @author              : 
 * @version             : V1.0.0
 * @date                : 29-June-2019
 * @brief               : This file provides all the QSPI firmware functions
 *********************************************************************************************************************/

 /* Includes --------------------------------------------------------------------------------------------------------*/
#include "mhcard_qspi.h"

/** @defgroup QSPI_Private_Defines
  * @{
  */

#define QSPI_CR0_CLEAR_MASK											((uint32_t)~0xFFEEFFFF)
#define QSPI_CR1_CLEAR_MASK											((uint32_t)~0xFFFFF)
#define QSPI_FCR_CLEAR_MASK											((uint32_t)~0x3F3F3F00)
#define QSPI_DCR_RECEIVE_LEVEL_CLEAR_MASK							((uint32_t)~0x3F80)
#define QSPI_DCR_TRANSMIT_LEVEL_CLEAR_MASK							((uint32_t)~0x7F)

#define QSPI_CR0_PARAM_ENABLE_POS									(0x18)
#define QSPI_CR0_PARAM_DMA_RECEIVE_ENABLE_POS						(0x14)
#define QSPI_CR0_PARAM_DMA_TRANSMIT_ENABLE_POS						(0x10)
#define QSPI_CR0_PARAM_INTERRPUT_ENABLE_POS							(0x0D) 
#define QSPI_CR0_PARAM_MODEL_SELECT_POS								(0x0A)
#define QSPI_CR0_PARAM_FIRST_BIT_POS								(0x09)
#define QSPI_CR0_PARAM_CPOL_POS										(0x08)
#define QSPI_CR0_PARAM_CPHA_POS										(0x07)
#define QSPI_CR0_PARAM_DIVIDE_ENABLE_POS							(0x02)
#define QSPI_CR0_PARAM_TRANSMIT_ENABLE_POS							(0x01)
#define QSPI_CR0_PARAM_BUSY_POS										(0x00)

#define QSPI_CR1_PARAM_BAUDRATE_POS									(0x0A)
#define QSPI_CR1_PARAM_RECEIVE_DATA_LENGTH_POS						(0x00)

#define QSPI_DCR_PARAM_DMA_RECEIVE_LEVEL_POS						(0x07)
#define QSPI_DCR_PARAM_DMA_TRANSMIT_LEVEL_POS						(0x00)		

#define QSPI_FCR_PARAM_TRANSIMIT_FIFO_EMPTY_THRESHOULD_POS			(0x08)
#define QSPI_FCR_PARAM_RECEIVE_FIFO_FULL_THRESHOULD_POS				(0x10)

/**
  * @}
  */

/**
  * @brief  Initializes the QSPI peripheral according to the specified parameters.
  * @param  QSPI_InitStruct: pointer to a QSPI_InitTypeDef structure that
  *         contains the configuration information for the specified QSPI peripheral.
  * @retval None
  */
void QSPI_Init(QSPI_InitTypeDef* QSPI_InitStruct)
{
	assert_param(IS_QSPI_MODE(QSPI_InitStruct->QSPI_Mode));
	assert_param(IS_QSPI_CPOL(QSPI_InitStruct->QSPI_CPOL));
	assert_param(IS_QSPI_CPHA(QSPI_InitStruct->QSPI_CPHA));
	assert_param(IS_QSPI_FIRST_BIT(QSPI_InitStruct->QSPI_FirstBit));
	assert_param(IS_QSPI_BAUDRATE_PRESCALER(QSPI_InitStruct->QSPI_BaudRatePrescaler));
	assert_param(IS_QSPI_TX_FIFO_EMPTY_THRESHOLD(QSPI_InitStruct->QSPI_TXFIFOEmptyThreshold));
	assert_param(IS_QSPI_RX_FIFO_FULL_THRESHOLD(QSPI_InitStruct->QSPI_RXFIFOFullThreshold));
			
	QSPI->CR1 = (uint32_t)((QSPI->CR1 & QSPI_CR1_CLEAR_MASK) 
							| ((QSPI_InitStruct->QSPI_BaudRatePrescaler & 0x1FF) << QSPI_CR1_PARAM_BAUDRATE_POS)
							| ((QSPI_InitStruct->QSPI_ReceiveDataLength & 0x3FF) << QSPI_CR1_PARAM_RECEIVE_DATA_LENGTH_POS));
	
	QSPI->FCR = (uint32_t)((QSPI->FCR & QSPI_FCR_CLEAR_MASK)
							| ((QSPI_InitStruct->QSPI_TXFIFOEmptyThreshold & 0x3F) << QSPI_FCR_PARAM_TRANSIMIT_FIFO_EMPTY_THRESHOULD_POS)
							| ((QSPI_InitStruct->QSPI_RXFIFOFullThreshold & 0x3F) << QSPI_FCR_PARAM_RECEIVE_FIFO_FULL_THRESHOULD_POS));	

	QSPI->CR0 = (uint32_t)((QSPI->CR0 & QSPI_CR0_CLEAR_MASK) 
							| ((QSPI_InitStruct->QSPI_CPOL & 0x01) << QSPI_CR0_PARAM_CPOL_POS)
							| ((QSPI_InitStruct->QSPI_CPHA & 0x01) << QSPI_CR0_PARAM_CPHA_POS)
							| ((QSPI_InitStruct->QSPI_FirstBit & 0x01) << QSPI_CR0_PARAM_FIRST_BIT_POS)
							| ((QSPI_InitStruct->QSPI_Mode & 0x07) << QSPI_CR0_PARAM_MODEL_SELECT_POS)
							| ((QSPI_InitStruct->QSPI_DivideEnable & 0x01) << QSPI_CR0_PARAM_DIVIDE_ENABLE_POS)
							| ((QSPI_InitStruct->QSPI_Enable & 0x01) << QSPI_CR0_PARAM_ENABLE_POS));			
	
}

/**
  * @brief  Initializes the QSPI DMA interface according to the specified parameters.
  * @param  QSPI_DMAInitStruct: pointer to a QSPI_DMAInitStruct structure that contains the configuration information.
  * @retval None
  */
void QSPI_DMAInit(QSPI_DMAInitTypeDef* QSPI_DMAInitStruct)
{
	if (ENABLE == QSPI_DMAInitStruct->QSPI_DMAReceiveEnable)
	{	
		assert_param(IS_QSPI_DMA_RECEIVE_LEVEL(QSPI_DMAInitStruct->QSPI_DMAReceiveLevel));
			
		QSPI->DCR = (uint32_t)((QSPI->DCR & QSPI_DCR_RECEIVE_LEVEL_CLEAR_MASK) 
					| ((QSPI_DMAInitStruct->QSPI_DMAReceiveLevel) << QSPI_DCR_PARAM_DMA_RECEIVE_LEVEL_POS));
		
		QSPI->CR0 |= (uint32_t)((QSPI_DMAInitStruct->QSPI_DMAReceiveEnable & 0x01) << QSPI_CR0_PARAM_DMA_RECEIVE_ENABLE_POS);
	}
	else
	{
		QSPI->CR0 &= ~(uint32_t)(0x01 << QSPI_CR0_PARAM_DMA_RECEIVE_ENABLE_POS);
	}
	
	if (ENABLE == QSPI_DMAInitStruct->QSPI_DMATransmitEnable)
	{
		assert_param(IS_QSPI_DMA_TRANSMIT_LEVEL(QSPI_DMAInitStruct->QSPI_DMATransmitLevel));	
		
		QSPI->DCR = (uint32_t)((QSPI->DCR & QSPI_DCR_TRANSMIT_LEVEL_CLEAR_MASK) 
					| ((QSPI_DMAInitStruct->QSPI_DMATransmitLevel) << QSPI_DCR_PARAM_DMA_TRANSMIT_LEVEL_POS));
		
		QSPI->CR0 |= (uint32_t)((QSPI_DMAInitStruct->QSPI_DMATransmitEnable & 0x01) << QSPI_CR0_PARAM_DMA_TRANSMIT_ENABLE_POS);
	}
	else 
	{
		QSPI->CR0 &= ~(uint32_t)(0x01 << QSPI_CR0_PARAM_DMA_TRANSMIT_ENABLE_POS);
	}
}

/**
  * @brief  Enable or disable the QSPI peripheral
  * @param  NewState: new state of the QSPI peripheral. 
  *   		This parameter can be: ENABLE or DISABLE
  * @retval None
  */
void QSPI_Cmd(FunctionalState NewState)
{		
	if (NewState != DISABLE)
	{
		QSPI->CR0 |= 1 << QSPI_CR0_PARAM_ENABLE_POS;
	}
	else
	{
		QSPI->CR0 &= ~(1 << QSPI_CR0_PARAM_ENABLE_POS);
	}
}

/**
  * @brief  Transmits single data through the QSPI peripheral.
  * @param  Mode: Select the QSPI bus mode. 
  * @param  Data: the data to be transmitted
  * @retval None
  */
void QSPI_SendData(uint8_t Data)
{
	QSPI->WDR = Data;
}

/**
  * @brief  Receive single data through the QSPI peripheral.
  * @param  Mode: Select the QSPI bus mode. 
  * @retval The received data
  */
uint8_t QSPI_ReceiveData(void)
{
	return (uint8_t)QSPI->RDR;
}

/**
  * @brief  Checks whether the specified QSPI flag is set or not.
  * @retval The new state of QSPI_FLAG (SET or RESET).
  */

FlagStatus QSPI_GetFlagStatus(uint32_t QSPI_Flag)
{
	if ((QSPI->SR & QSPI_Flag) != RESET)
	{
		return SET;  
	} 
	return RESET; 
}

/**
  * @brief  Checks whether the specified QSPI interrupt has occurred or not.
  * @retval The new state of QSPI_IT (SET or RESET).
  */
ITStatus QSPI_GetITStatus(uint32_t QSPI_IT)
{
	uint8_t itmask = 0;
	
	itmask = (uint8_t)QSPI->RISR;
	itmask <<= 4;
	itmask = (uint8_t)(itmask | QSPI->TISR);
	
	if ((itmask & QSPI_IT) != RESET)
	{
		return SET;
	}	
	return RESET;
}

/**
  * @brief  Checks whether the specified QSPI_TramsmitDone is set or not.
  * @retval The new state of QSPI_FLAG (SET or RESET).
  */
FlagStatus QSPI_IsTransmitDone(void)
{
	if ((QSPI->FLSR & 0x01) != RESET)
	{
		return SET;
	}
	return RESET;
}


FlagStatus QSPI_IsReady(void)
{
	if ((QSPI->CR1 & 0x1) == SET)
	{
		return RESET;
	}
	return SET;
}
/**
  * @brief Reset QSPI FIFO(TX or RX).
  */
void QSPI_FIFOReset(uint32_t QSPI_FIFO)
{
	QSPI->FCR |= QSPI_FIFO;
	
	QSPI->FCR &= ~QSPI_FIFO;
}

/**
  *@brief  Enable or disable the specified QSPI interrupt.
  */

void QSPI_ITConfig(uint32_t QSPI_IT, FunctionalState NewState)
{
	if (NewState != DISABLE)
	{
		QSPI->CR0 |= QSPI_IT;
	}
	else
	{
		QSPI->CR0 &= ~QSPI_IT;
	}
}

/**
  * @brief  Enables or disables the QSPI transfer.
  * @param  NewState: new state of the QSPI peripheral. 
  *   		This parameter can be: ENABLE or DISABLEE.
  * @retval None
  */

void QSPI_TransmitCmd(FunctionalState NewState)
{
	if (NewState != DISABLE)
	{
		QSPI->CR0 |= 1 << QSPI_CR0_PARAM_TRANSMIT_ENABLE_POS;
	}
	else
	{
		QSPI->CR0 &= ~(1 << QSPI_CR0_PARAM_TRANSMIT_ENABLE_POS);
	}
}


void QSPI_ReceiveCmd(FunctionalState NewState)
{
	if (NewState != DISABLE)
	{
		QSPI->FLCR |= 0x01;
	}
	else
	{
		QSPI->FLCR &= 0x00;
	}
}


/**************************      (C) COPYRIGHT     *****END OF FILE***********************************************************************/
