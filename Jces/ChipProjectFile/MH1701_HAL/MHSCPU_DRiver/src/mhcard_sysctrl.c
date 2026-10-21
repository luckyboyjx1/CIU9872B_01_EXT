/**************************************************************************//**
 * @file     mhcard_sysctrl.c
 * @brief    This file provides all the system firmware function.
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
   

#include "mhcard_sysctrl.h"
#include "mhcard_exti.h"
#include "mhcard_dma.h"
#include "mhcard_qspi.h"
#include "mhcard_spi.h"
#include "mhcard_i2c.h"

/**
  * @brief  Enables or disables the AHB peripheral clock.
  * @param  SYSCTRL_AHBPeriph: specifies the AHB peripheral to gates its clock.
  *   
  *   For @b this parameter can be any combination
  *   of the following values:        
  *     @arg SYSCTRL_AHBPeriph_DMA
  *     @arg SYSCTRL_AHBPeriph_USB
  *     @arg SYSCTRL_AHBPeriph_DCMI
  *     @arg SYSCTRL_AHBPeriph_LCD
  *     @arg SYSCTRL_AHBPeriph_CRYPT
  *
  * @param  NewState: new state of the specified peripheral clock.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SYSCTRL_ClockEnable(uint32_t SYSCTRL_Periph, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_SYSCTRL_CLK_EN(SYSCTRL_Periph));
    
    if (NewState)
    {
       SYSCFG->CLK_EN |= SYSCTRL_Periph;
    }
    else
    {
       SYSCFG->CLK_EN &= ~SYSCTRL_Periph;
    }
    return;
}

/**
  * @brief  Enables or disables the APB peripheral clock.
  * @param  SYSCTRL_APBPeriph: specifies the APB peripheral to gates its clock.
  *   
  *   For @b this parameter can be any combination
  *   of the following values:        
  *     @arg SYSCTRL_APBPeriph_TRNG
  *     @arg SYSCTRL_APBPeriph_ADC
  *     @arg SYSCTRL_APBPeriph_CRC
  *     @arg SYSCTRL_APBPeriph_BPU
  *     @arg SYSCTRL_APBPeriph_TIMM0
  *     @arg SYSCTRL_APBPeriph_GPIO
  *     @arg SYSCTRL_APBPeriph_SCI0
  *     @arg SYSCTRL_APBPeriph_SPI2
  *     @arg SYSCTRL_APBPeriph_SPI1
  *     @arg SYSCTRL_APBPeriph_SPI0
  *     @arg SYSCTRL_APBPeriph_UART1
  *     @arg SYSCTRL_APBPeriph_UART0
  *
  * @param  NewState: new state of the specified peripheral clock.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SYSCTRL_SoftReset(uint32_t SYSCTRL_Periph)
{
    /* Check the parameters */
    assert_param(IS_MODULE_SFT_RST(SYSCTRL_Periph));
    
//	u32Tmp = (SYSCTRL_SOFTRST_BU_MASK) ^ (SYSCTRL_Periph & (SYSCTRL_SOFTRST_BU_MASK));
//	u32Tmp |= (SYSCTRL_Periph & SYSCTRL_SOFTRST_ALL);
//	SYSCFG->RST_EN = u32Tmp;
	
	if(SYSCTRL_Periph & (SYSCTRL_GLB_RESET))
	{
		SYSCFG->RST_EN = SYSCTRL_GLB_RESET | (SYSCTRL_SOFTRST_BU_MASK & ~SYSCTRL_GLB_BU_RESET);
	}
	if(SYSCTRL_Periph & (SYSCTRL_SYMC_RESET))
	{
		SYSCFG->RST_EN = SYSCTRL_SYMC_RESET | (SYSCTRL_SOFTRST_BU_MASK & ~SYSCTRL_SYMC_BU_RESET);
	}            
	if(SYSCTRL_Periph & (SYSCTRL_ASYMC_RESET))
	{   
		SYSCFG->RST_EN = SYSCTRL_ASYMC_RESET | (SYSCTRL_SOFTRST_BU_MASK & ~SYSCTRL_ASYMC_BU_RESET);
	}            
	if(SYSCTRL_Periph & (SYSCTRL_RNG_RESET))
	{
		SYSCFG->RST_EN = SYSCTRL_RNG_RESET | (SYSCTRL_SOFTRST_BU_MASK & ~SYSCTRL_RNG_BU_RESET);
	}
	
	if(SYSCTRL_Periph & SYSCTRL_SOFTRST_MASK)
	{
		SYSCFG->RST_EN = (SYSCTRL_Periph & SYSCTRL_SOFTRST_MASK) | SYSCTRL_SOFTRST_BU_MASK;
	}
	return;
    
}

/**
  * @brief  Enables or disables the AHB peripheral reset.
  * @param  SYSCTRL_AHBPeriph: specifies the AHB peripheral to reset(CM3 reset/GLB Reset).
  *   
  *   For @b this parameter can be any combination
  *   of the following values:        
  *     @arg SYSCTRL_AHBPeriph_DMA
  *     @arg SYSCTRL_AHBPeriph_USB
  *     @arg SYSCTRL_AHBPeriph_DCMI
  *     @arg SYSCTRL_AHBPeriph_LCD
  *     @arg SYSCTRL_AHBPeriph_CRYPT
  *     @arg SYSCTRL_GLB_RESET
  *     @arg SYSCTRL_CM3_RESET
  *
  * @param  NewState: new state of the specified peripheral reset.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SYSCTRL_PCLKConfig(uint32_t PCLK_Div)
{
	assert_param(IS_GET_SYSCTRL_PCLK_DIV(PCLK_Div));
    
    SYSCFG->CLK_CFG = (SYSCFG->CLK_CFG & ~SYSCTRL_PCLK_SEL_Mask) | (PCLK_Div << SYSCTRL_PCLK_SEL_Pos);
//	SYSCFG->CLK_CFG &= ~SYSCTRL_PCLK_SEL_Mask;
//	SYSCFG->CLK_CFG |= (PCLK_Div << SYSCTRL_PCLK_SEL_Pos);
	return;
}
/**
  * @brief  Enables or disables the APB peripheral reset.
  * @param  SYSCTRL_APBPeriph: specifies the APB peripheral to reset.
  *   
  *   For @b this parameter can be any combination
  *   of the following values:        
  *     @arg SYSCTRL_APBPeriph_TRNG
  *     @arg SYSCTRL_APBPeriph_ADC
  *     @arg SYSCTRL_APBPeriph_CRC
  *     @arg SYSCTRL_APBPeriph_IMG_COP
  *     @arg SYSCTRL_APBPeriph_TIMM0
  *     @arg SYSCTRL_APBPeriph_GPIO
  *     @arg SYSCTRL_APBPeriph_SCI0
  *     @arg SYSCTRL_APBPeriph_SPI2
  *     @arg SYSCTRL_APBPeriph_SPI1
  *     @arg SYSCTRL_APBPeriph_SPI0
  *     @arg SYSCTRL_APBPeriph_UART1
  *     @arg SYSCTRL_APBPeriph_UART0
  *
  * @param  NewState: new state of the specified peripheral reset.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */

void SYSCTRL_HCLKConfig(uint32_t HCLK_Div)
{
	assert_param(IS_GET_SYSCTRL_HCLK_DIV(HCLK_Div));
    
    SYSCFG->CLK_CFG = (SYSCFG->CLK_CFG & ~SYSCTRL_HCLK_SEL_Mask) | (HCLK_Div << SYSCTRL_HCLK_SEL_Pos);
//	SYSCFG->CLK_CFG &= ~SYSCTRL_HCLK_SEL_Mask;
//	SYSCFG->CLK_CFG |= (HCLK_Div << SYSCTRL_HCLK_SEL_Pos);
	return;
}


#if   defined ( __CC_ARM )
__STATIC_INLINE __ASM void SYSCTRL_Sleep(void)
{
    CPSID i;
    NOP;
    NOP;
    NOP;
    NOP;
    WFI;
    NOP;
    NOP;
    NOP;
    NOP;
    CPSIE i;
    BX LR;
}
#elif defined (__GNUC__)
void SYSCTRL_Sleep(void)
{
    asm("CPSID i");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("wfi");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("CPSIE i");
    asm("BX LR");
}
#elif defined (__ICCARM__)
void SYSCTRL_Sleep(void)
{
    __ASM(
    "CPSID i\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "WFI\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "CPSIE i\n"
    "BX LR"
    );
}
#endif


/**
* @brief
* @param
* @retval
**/



/**
* @brief  Get WKUP_FLAG1 register value
* @param  None 
* @retval The value of WKUP_FLAG1 register
* */
void SYSCTRL_AlgClkConfig(uint32_t ALGCLK_Div)
{
	assert_param(IS_GET_SYSCTRL_ALGCLK_DIV(ALGCLK_Div));

    SYSCFG->CLK_CFG = (SYSCFG->CLK_CFG & ~SYSCTRL_ALGCLK_SEL_Mask) | (ALGCLK_Div << SYSCTRL_ALGCLK_SEL_Pos);
//	SYSCFG->CLK_CFG &= ~SYSCTRL_ASYMCLK_SEL_Mask;
//	SYSCFG->CLK_CFG |= (ASYMCLK_Div << SYSCTRL_ASYMCLK_SEL_Pos);
	return;
}

/**
  * @brief  Select System clock source
  * @param  source_select£ºSystem clock source value¡£
  * @retval None
  */
void SYSCTRL_SYSCLKSourceSelect(SYSCTRL_SYSCLK_TypeDef source)
{
	assert_param(IS_SYSCLK_SOURCE(source));
	
    SYSCFG->CLK_CFG = (SYSCFG->CLK_CFG & ~SYSCTRL_SRCCLK_SEL_Mask) | (source << SYSCTRL_SRCCLK_SEL_Pos);
//	SYSCFG->CLK_CFG &= ~SYSCTRL_SRCCLK_SEL_Mask;
//	SYSCFG->CLK_CFG |= (source << SYSCTRL_SRCCLK_SEL_Pos);
     
    return;
}

/** @brief  Set System clock Freq
  * @param  SYSCLK_Freq£ºSystem clock set value¡£
  * @retval None
  */
void SYSCTRL_TICKRefConfig(uint32_t  Tim_Sel)
{
	assert_param(IS_SYSCTRL_TICK_REF(Tim_Sel));
	
    SYSCFG->CLK_CFG = (SYSCFG->CLK_CFG & ~SYSCTRL_TICK_REF_SEL_Mask) | (Tim_Sel << SYSCTRL_TICK_REF_SEL_Pos);
//	SYSCFG->CLK_CFG &= ~SYSCTRL_TIM_REF_SEL_Mask;
//	SYSCFG->CLK_CFG |= (Tim_Sel << SYSCTRL_TIM_REF_SEL_Pos);
	return;
}

/**
  * @brief  
  * @param  
  * @retval 
  */
  
#define RC_FREQUENCE        (72000000)
#define RC_BASE_FREQUENCE   (RC_FREQUENCE / 32)
void SYSCTRL_GetClocksFreq(SYSCTRL_ClocksTypeDef* SYSCTRL_Clocks)
{
    /*FPGA Stage -- Only 48MHZ Can be used*/
    //SYSCFG->CLK_CFG = 0;
    //PLL system clock source select
    SYSCTRL_Clocks->SYS_Frequency = RC_FREQUENCE;

    //HCLK Sel
    SYSCTRL_Clocks->HCLK_Frequency = ((SYSCFG->CLK_CFG & SYSCTRL_HCLK_SEL_Mask) >> SYSCTRL_HCLK_SEL_Pos) + 1;
    SYSCTRL_Clocks->HCLK_Frequency = SYSCTRL_Clocks->HCLK_Frequency * RC_BASE_FREQUENCE;
    
    //ALGCLK Sel
    SYSCTRL_Clocks->ALGCLK_Frequency = ((SYSCFG->CLK_CFG & SYSCTRL_ALGCLK_SEL_Mask) >> SYSCTRL_ALGCLK_SEL_Pos) + 1;
    SYSCTRL_Clocks->ALGCLK_Frequency = SYSCTRL_Clocks->ALGCLK_Frequency * RC_BASE_FREQUENCE;
    

    //PCLK Sel
    if((SYSCFG->CLK_CFG & SYSCTRL_PCLK_SEL_Mask) == (SYSCTRL_PCLK_Div_None << SYSCTRL_PCLK_SEL_Pos))
    {
        SYSCTRL_Clocks->PCLK_Frequency = SYSCTRL_Clocks->HCLK_Frequency;
    }
    else if((SYSCFG->CLK_CFG & SYSCTRL_PCLK_SEL_Mask) == (SYSCTRL_PCLK_Div2 << SYSCTRL_PCLK_SEL_Pos))
    {
        SYSCTRL_Clocks->PCLK_Frequency = SYSCTRL_Clocks->HCLK_Frequency/2;
    }
    else	//SYSCTRL_PCLK_Div4
    {
        SYSCTRL_Clocks->PCLK_Frequency = SYSCTRL_Clocks->HCLK_Frequency/4;
    } 
    return;    
}

void SYSCTRL_DMA_ChannelConfig(DMA_TypeDef *DMA_Channelx, uint32_t DMA_Peripheral, uint32_t DMA_DIR)
{
    uint32_t u32DmaChannel = 0;
    
    if (DMA_DIR_Peripheral_To_Memory == DMA_DIR)
    {
        switch (DMA_Peripheral)
        {
        //need to add ISO1443 todo
        case SPIS_BASE:
            u32DmaChannel = SYSCTRL_DMA_CH_SPI_RX;      
            break;
        case QSPI_BASE:
            u32DmaChannel = SYSCTRL_DMA_CH_SPI_RX;      
            break;
        case I2C_BASE:
            u32DmaChannel = SYSCTRL_DMA_CH_I2C_RX;
        }
    }
    else if (DMA_DIR_Memory_To_Peripheral == DMA_DIR)
    {
        switch (DMA_Peripheral)
        {
        //need to add ISO1443
        case SPIS_BASE:
            u32DmaChannel = SYSCTRL_DMA_CH_SPI_TX;      
            break;
		case QSPI_BASE:
            u32DmaChannel = SYSCTRL_DMA_CH_SPI_TX;      
            break;
        case I2C_BASE:
            u32DmaChannel = SYSCTRL_DMA_CH_I2C_TX;
        }
    }
	
    if(DMA_Channelx == DMA_Channel_0)
    {
        SYSCFG->DMA_CH &= ~SYSCRTL_DMA_CH_MASK;
        SYSCFG->DMA_CH |= u32DmaChannel;
    }
}

void SYSCTRL_WakeMcu(uint32_t WakeSelect)
{
	assert_param(IS_GET_WAKE_MCU_CTRL(WakeSelect));
	SYSCFG->WAKE_MCU_CTRL &= ~SYSCTRL_WAKE_MCU_CTRL_Mask;
	SYSCFG->WAKE_MCU_CTRL |= (WakeSelect << SYSCTRL_WAKE_MCU_CTRL_Pos);
	return;
}


void SYSCTRL_FlashDeepConfig(FlashDeepState_TypeDef FlashDeepState)
{
	assert_param(IS_SYSCLK_FLASHDEEP_STATE(FlashDeepState));
	
	SYSCFG->SYS_CON &= ~SYSCTRL_SYSCON_FLAHS_DEEPSLEEP_Mask;
	SYSCFG->SYS_CON |= (FlashDeepState << SYSCTRL_SYSCON_FLASH_DEEPSLEEP_Pos);
	
	return;
}

void SYSCTRL_LowPowerConfig(uint32_t module, SleepState_TypeDef SleepState)
{
	assert_param(IS_SYSCON_LP_MODE(module));
    assert_param(IS_ALL_SLEEP_STATE(SleepState));

    if (module == LP_RAM_MODE)
    {
        SYSCFG->SYS_CON &= ~SYSCTRL_SYSCON_LP_RAM_MODE_Mask;
		SYSCFG->SYS_CON |= (SleepState << SYSCTRL_SYSCON_LP_RAM_MODE_Pos);
    }
    else if (module == LP_ROM_MODE)
    {
        SYSCFG->SYS_CON &= ~SYSCTRL_SYSCON_LP_ROM_MODE_Mask;
		SYSCFG->SYS_CON |= (SleepState << SYSCTRL_SYSCON_LP_ROM_MODE_Pos);
    }  
	else if (module == LP_RAM32K_MODE)
    {
        SYSCFG->SYS_CON &= ~SYSCTRL_SYSCON_LP_RAM32K_MODE_Mask;
		SYSCFG->SYS_CON |= (SleepState << SYSCTRL_SYSCON_LP_RAM32K_MODE_Pos);
    }
}

void SYSCTRL_EnterSleep(SleepMode_TypeDef SleepMode)
{
	assert_param(IS_ALL_SLEEP_MODE(SleepMode));

    if (SleepMode == SleepMode_CpuOff)
    {
        SCB->SCR &= (~BIT(2));
        SYSCTRL_Sleep();
    }
    else if (SleepMode == SleepMode_DeepSleep)
    {
        SCB->SCR |= BIT(2);
        SYSCTRL_Sleep();
    }
}

/**************************      (C) COPYRIGHT    *****END OF FILE****/
