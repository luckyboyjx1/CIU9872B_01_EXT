/**************************************************************************//**
 * @file     mhscpu_gpio.c
 * @brief    This file provides all the GPIO firmware function.
 * @version  V1.00
 * @date     11. April 2015
 *
 * @note
 *
 ******************************************************************************/
/* Copyright (c) 2012 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/
#include "mhcard_gpio.h"
#include "mhcard_sysctrl.h"

//static volatile uint16_t GPIO_OD_Mark[4] = {0};

/**
  * @brief  Get GPIO Number.
  * @param  GPIO_Pin: GPIO_Pin only have one group
  * @retval None
  */


/**
  * @brief  Deinitializes the GPIOx peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void GPIO_DeInit(void)
{
	//SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_GPIO, ENABLE);
    SYSCTRL_ClockEnable(SYSCTRL_Periph_GPIO, ENABLE);
    SYSCTRL_SoftReset(SYSCTRL_GPIO_RESET);
    //GPIO->MODE_H = 0x0D; // Rst pin 
}


/**
  * @brief  Initializes the GPIOx peripheral according to the specified
  *         parameters in the GPIO_InitStruct.
  * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
  * @param  GPIO_InitStruct: pointer to a GPIO_InitTypeDef structure that
  *         contains the configuration information for the specified GPIO peripheral.
  * @retval None
  */
void GPIO_Init(GPIO_InitTypeDef *GPIO_InitStruct)
{
	uint32_t i;

    assert_param(IS_GPIO_PIN(GPIO_InitStruct->GPIO_Pin));
	assert_param(IS_GPIO_MODE(GPIO_InitStruct->GPIO_Mode));
    assert_param(IS_GPIO_REMAP_MODE(GPIO_InitStruct->GPIO_Remap));
    
    GPIO_RemapConfig(GPIO_InitStruct->GPIO_Pin,  GPIO_InitStruct->GPIO_Remap);

    if (GPIO_Remap_IO != GPIO_InitStruct->GPIO_Remap)
	{
        return;		
	}
			
	for (i = 0; i < 8; i++)
    {
        if (GPIO_InitStruct->GPIO_Pin & (0x01U << i))
        {
			GPIO->CON &= ~(0x03U<<(i*2));
			GPIO->CON |= (GPIO_InitStruct->GPIO_Mode << (i*2));
		}
	}
}

/**
  * @brief  function configuration of GPIO
  *         
  * @param  GPIOx:only one group GPIO_Pin can be selected
  * @param  GPIO_pin: pin_0 - 7 can be selected
  * @param  Mode:select configuration state of GPIO
  *
  * @retval None
  */
void GPIO_RemapConfig(uint8_t GPIO_Pin, GPIO_RemapTypeDef GPIO_Remap)
{
	uint32_t i;
	//uint32_t Pin_Conf;
	assert_param(IS_GPIO_PIN(GPIO_Pin));
	assert_param(IS_GPIO_REMAP_MODE(GPIO_Remap));

	for (i = 0; i < 8; i++)
    {
		if (GPIO_Pin & (0x01U << i))
		{
			if(i < 4)
			{
                GPIO->MODE_L = (GPIO->MODE_L & ~(0xFFU<<(i*8))) | (GPIO_Remap << (i*8));
//				GPIO->MODE_L &= ~(0xFFU<<(i*8));
//				GPIO->MODE_L |= (GPIO_Remap << (i*8));
			}
			else
			{
                GPIO->MODE_H = (GPIO->MODE_H & ~(0xFFU<<((i - 4)*8))) | (GPIO_Remap << ((i - 4)*8));
//				GPIO->MODE_H &= ~(0xFFU<<((i - 4)*8));
//				GPIO->MODE_H |= (GPIO_Remap << ((i - 4)*8));
			}
		}
	}
	
}




/**
  * @brief  Fills each GPIO_InitStruct member with its default value.
  * @param  GPIO_InitStruct : pointer to a GPIO_InitTypeDef structure which will
  *         be initialized.
  * @retval None
  */
void GPIO_StructInit(GPIO_InitTypeDef* GPIO_InitStruct)
{
	/* Reset GPIO init structure parameters values */
	GPIO_InitStruct->GPIO_Pin  = GPIO_Pin_All;
	GPIO_InitStruct->GPIO_Mode = GPIO_Mode_IN;
	//GPIO_InitStruct->GPIO_Remap = GPIO_Remap_0;
    GPIO_InitStruct->GPIO_Remap = GPIO_Remap_IO;
}

/**
  * @brief  Reads the specified input port pin.
  * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
  * @param  GPIO_Pin:  specifies the port bit to read.
  *   This parameter can be GPIO_Pin_x where x can be (0..15).
  * @retval The input port pin value.
  */
uint8_t GPIO_ReadInputDataBit(uint8_t GPIO_Pin)
{
	uint8_t bitstatus = 0x00;
	/* Check the parameters */
	//assert_param(IS_GPIO_PERIPH(GPIOx));
	assert_param(IS_GET_GPIO_PIN(GPIO_Pin)); 
	   
    if ((((GPIO->IOD >> 8) & 0xFF) & GPIO_Pin) != (uint32_t)Bit_RESET)
	{
		bitstatus = (uint8_t)Bit_SET;
	}
	else
	{
		bitstatus = (uint8_t)Bit_RESET;
	}
	return bitstatus;
}

/**
  * @brief  Reads the specified GPIO input data port.
  * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
  * @retval GPIO input data port value.
  */
uint8_t GPIO_ReadInputData(void)
{
	return ((GPIO->IOD >> 8) & 0xFF);
}

/**
  * @brief  Reads the specified output data port bit.
  * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
  * @param  GPIO_Pin:  specifies the port bit to read.
  *   This parameter can be GPIO_Pin_x where x can be (0..15).
  * @retval The output port pin value.
  */
uint8_t GPIO_ReadOutputDataBit(uint8_t GPIO_Pin)
{
	uint8_t bitstatus = 0x00;

	/* Check the parameters */
	//assert_param(IS_GPIO_PERIPH(GPIOx));
	assert_param(IS_GPIO_PIN(GPIO_Pin)); 

	if (((GPIO->IOD & 0xFF) & BIT(GPIO_Pin)) != (uint32_t)Bit_RESET)
	{
		bitstatus = (uint8_t)Bit_SET;
	}
	else
	{
		bitstatus = (uint8_t)Bit_RESET;
	}
	return bitstatus;
}

/**
  * @brief  Reads the specified GPIO output data port.
  * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
  * @retval GPIO output data port value.
  */
uint8_t GPIO_ReadOutputData(void)
{
	return (GPIO->IOD & 0xFF);
}

/**
  * @brief  Sets the selected data port bits.
  * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
  * @param  GPIO_Pin: specifies the port bits to be written.
  *   This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval None
  */
void GPIO_SetBits(uint8_t GPIO_Pin)
{
    assert_param(IS_GPIO_PIN(GPIO_Pin));
	
    GPIO->BSR |= (GPIO_Pin);       
}

/**
  * @brief  Clears the selected data port bits.
  * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
  * @param  GPIO_Pin: specifies the port bits to be written.
  *   This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval None
  */
void GPIO_ResetBits(uint8_t GPIO_Pin)
{
    assert_param(IS_GPIO_PIN(GPIO_Pin));
	
	GPIO->BSR |= ((GPIO_Pin) << 8);
}

/**
  * @brief  Sets or clears the selected data port bit.
  * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
  * @param  GPIO_Pin: specifies the port bit to be written.
  *   This parameter can be one of GPIO_Pin_x where x can be (0..15).
  * @param  BitVal: specifies the value to be written to the selected bit.
  *   This parameter can be one of the BitAction enum values:
  *     @arg Bit_RESET: to clear the port pin
  *     @arg Bit_SET: to set the port pin
  * @retval None
  */
void GPIO_WriteBit(uint8_t GPIO_Pin, BitAction BitVal)
{
	//assert_param(IS_GPIO_PERIPH(GPIOx));
	assert_param(IS_GET_GPIO_PIN(GPIO_Pin));
	
	if (Bit_RESET == BitVal)
	{
		GPIO_ResetBits(GPIO_Pin);
	} 
	else if (Bit_SET == BitVal)
	{
		GPIO_SetBits(GPIO_Pin);
	}
}

/**
  * @brief  Writes data to the specified GPIO data port.
  * @param  GPIOx: where x can only be A to select the GPIO peripheral.
  * @param  PortVal: specifies the value to be written to the port output data register.
  * @retval None
  */
void GPIO_ModeConfig(uint8_t GPIO_Pin, GPIO_ModeTypeDef Mode)
{
	uint32_t i;
	
	assert_param(IS_GPIO_MODE(Mode));
	assert_param(IS_GPIO_PIN(GPIO_Pin));

	for (i = 0; i < 8; i++)
    {
        if (GPIO_Pin & (0x01U << i))
        {
			GPIO->CON &= ~(0x03U<<(i*2));
			GPIO->CON |= (Mode << (i*2));
		}
	}
	
}

/**
  * @brief  Interrupt pin select
  * @param  GPIOx: where x can be A to select the GPIO peripheral. 
  * @param  Type:  
  * @param  GPIO_pin:Only one pin can be selected,GPIO_pin_All can't be selected
  *   
  * @retval 
  */
void GPIO_InterConfig(uint8_t GPIO_Pin, GPIO_InptTypeDef Type)
{
	uint32_t i;
	
	assert_param(IS_GPIO_INT_TYPE(Type));
	assert_param(IS_GPIO_PIN(GPIO_Pin));

	for (i = 0; i < 8; i++)
    {
        if (GPIO_Pin & (0x01U << i))
        {
			GPIO->INT_TYPE &= ~(0x03U<<(i*2));
			GPIO->INT_TYPE |= (Type << (i*2));
		}
	}
}


/**
  * @brief  Interrupt flag clear
  * @param  None
  * @retval None
  */
void GPIO_InterClear(uint8_t GPIO_Pin)
{
    assert_param(IS_GPIO_PIN(GPIO_Pin));
	
    GPIO->INT_STAT |= (GPIO_Pin);
}

/**
  * @brief  
  * @param  
  * @retval 
  */



/**
  * @brief  
  * @param  
  *   
  * @retval None
  */





/**************************      (C) COPYRIGHT    *****END OF FILE****/
