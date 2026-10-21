/**************************************************************************//**
 * @file     system_<Device>.c
 * @brief    CMSIS Cortex-M# Device Peripheral Access Layer Source File for
 *           Device <Device>
 * @version  V3.10
 * @date     23. November 2012
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


#include <stdint.h>
#include "system_mhcard.h"


#include "Apdu.h"
#include "UartHal.h"
#include "Io_7816T0.h"

/*----------------------------------------------------------------------------
  DEFINES
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
/* ToDo: add here your necessary defines for device initialization
         following is an example for different system frequencies             */
#define __HSI             ( 6000000UL)
#define __XTAL            (12000000UL)    /* Oscillator frequency             */
#define __SYS_OSC_CLK     (    ___HSI)    /* Main oscillator frequency        */

#define __SYSTEM_CLOCK    (4*__XTAL)


/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
/* ToDo: initialize SystemCoreClock with the system core clock frequency value
         achieved after system intitialization.
         This means system core clock frequency after call to SystemInit()    */
uint32_t SystemCoreClock = __SYSTEM_CLOCK;  /*!< System Clock Frequency (Core Clock)*/


/*----------------------------------------------------------------------------
  Clock functions
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)            /* Get Core Clock Frequency      */
{
/* ToDo: add code to calculate the system frequency based upon the current
         register settings.
         This function can be used to retrieve the system core clock frequeny
         after user changed register sittings.                                */
  SystemCoreClock = __SYSTEM_CLOCK;
}

/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System.
 */
void PowerConfig_BeforeAsymc(uint32_t *pu32Bak, uint32_t HCLK_Div, uint32_t ALGCLK_Div)
{
    uint32_t u32Tmp, u32Clk;
    
    *pu32Bak = SYSCFG->CLK_CFG;
    u32Tmp =  SYSCFG->CLK_CFG;
    
    u32Clk = (u32Tmp >> 16) & 0x1F;

    while(u32Clk > HCLK_Div)
    {
        if((u32Clk >= 20) && (u32Clk <= 28))
        {
            u32Clk = u32Clk - 3;
        }
        else if((u32Clk >= 13) && (u32Clk < 20))
        {
            u32Clk = u32Clk - 2;
        }
        else
        {
            u32Clk = u32Clk - 1;
        }
        u32Tmp = (u32Tmp & ~(0x1FUL << 16)) | (u32Clk << 16);
        SYSCFG->CLK_CFG = u32Tmp;
    }
    
    u32Clk = (u32Tmp >> 8) & 0x1F;
    
    if (u32Clk >= ALGCLK_Div)
    {
        u32Tmp = (u32Tmp & ~(0x1FUL << 8)) | (ALGCLK_Div << 8);
        SYSCFG->CLK_CFG = u32Tmp;
    }
    else 
    {
        while(u32Clk < ALGCLK_Div)
        {
            u32Clk = u32Clk + 1;
            if (u32Clk > ALGCLK_Div)
                break;
            u32Tmp = (u32Tmp & ~(0x1FUL << 8)) | (u32Clk << 8);
            SYSCFG->CLK_CFG = u32Tmp;
            u32Tmp =  SYSCFG->CLK_CFG;
            u32Clk = (u32Tmp >> 8) & 0x1F;
        }
    }    
}

void PowerConfig_AfterAsymc(uint32_t *pu32Bak)
{
    uint32_t u32Tmp, u32Clk, u32BakCLK;
    uint8_t i;
    
    //config Aysmc to 18MHz
    u32Tmp =  SYSCFG->CLK_CFG;
    u32Clk = (u32Tmp >> 8) & 0x1F;
	
	if(u32Clk >= SYSCTRL_HCLK_D32M8)
	{
		u32Tmp = (u32Tmp & ~(0x1FUL << 8)) | (SYSCTRL_HCLK_D32M8 << 8);
        SYSCFG->CLK_CFG = u32Tmp;		
	}
	else
	{
		while(u32Clk != SYSCTRL_HCLK_D32M8)
		{
			u32Clk = u32Clk + 1;
			if (u32Clk > 0x07)
				break;
			u32Tmp = (u32Tmp & ~(0x1FUL << 8)) | (u32Clk << 8);
			SYSCFG->CLK_CFG = u32Tmp;
			u32Tmp =  SYSCFG->CLK_CFG;
			u32Clk = (u32Tmp >> 8) & 0x1F;
		}
    }
	
    //config mcu to high power mode
    u32Tmp =  SYSCFG->CLK_CFG;
    u32Clk = (u32Tmp >> 16) & 0x1F;
    u32BakCLK = (*pu32Bak >> 16) & 0x1F;
    
    if (u32Clk > u32BakCLK)
    {
        u32Tmp = (u32Tmp & ~(0x1FUL << 16)) | (u32BakCLK << 16);
        SYSCFG->CLK_CFG = u32Tmp;
    }
    else
    {
		while(u32Clk != u32BakCLK)
		{
			if((u32BakCLK - u32Clk) <= 3)
			{
				u32Clk = u32Clk + 1;		
			}
			else
			{
				if((u32Clk >= 20) && (u32Clk <= 28))
				{
					u32Clk = u32Clk + 3;
				}
				else if((u32Clk >= 13) && (u32Clk < 20))
				{
					u32Clk = u32Clk + 2;
				}
				else
				{
					u32Clk = u32Clk + 1;
				}			
			}
            u32Tmp = (u32Tmp & ~(0x1FUL << 16)) | (u32Clk << 16);
            SYSCFG->CLK_CFG = u32Tmp;
            u32Clk = (u32Tmp >> 16) & 0x1F;
        }
    }
}

void PowerConfig_BeforeFcu(uint32_t *pu32Bak)
{
    uint32_t u32Tmp, u32Clk;
    uint8_t i;
    
    *pu32Bak = SYSCFG->CLK_CFG;
    //config mcu to low power mode
    u32Tmp =  SYSCFG->CLK_CFG;
    u32Clk = (u32Tmp >> 16) & 0x1F;
    while(u32Clk > 15)
    {
        u32Clk = u32Clk - 1;
        u32Tmp = (u32Tmp & ~(0x1FUL << 16)) | (u32Clk << 16);
        SYSCFG->CLK_CFG = u32Tmp;
        u32Tmp =  SYSCFG->CLK_CFG;
        u32Clk = (u32Tmp >> 16) & 0x1F;
    }
}

void PowerConfig_AfterFcu(uint32_t *pu32Bak)
{
    uint32_t u32Tmp, u32Clk, u32BakCLK;
    //config mcu to high power mode
    u32Tmp =  SYSCFG->CLK_CFG;
    u32Clk = (u32Tmp >> 16) & 0x1F;
    u32BakCLK = *pu32Bak;
    u32BakCLK = (u32BakCLK >> 16) & 0x1F;
    
    if (u32Clk > u32BakCLK)
    {
        u32Clk = u32BakCLK;
        u32Tmp = (u32Tmp & ~(0x1FUL << 16)) | (u32Clk << 16);
        SYSCFG->CLK_CFG = u32Tmp;
    }
    else
    {
        while(u32Clk != u32BakCLK)
        {
            u32Clk = u32Clk + 1;
            u32Tmp = (u32Tmp & ~(0x1FUL << 16)) | (u32Clk << 16);
            SYSCFG->CLK_CFG = u32Tmp;
            u32Tmp =  SYSCFG->CLK_CFG;
            u32Clk = (u32Tmp >> 16) & 0x1F;
        }
    }
}

#define ASYMC_START_REG (*(volatile uint32_t *)0x40001800)
void MHAsymc_Enable()
{
    uint32_t u32Tmp;
    ASYMC_START_REG = 1;
    __disable_irq();
    PowerConfig_BeforeAsymc(&u32Tmp, SYSCTRL_HCLK_D32M11, SYSCTRL_ALGCLK_D32M11);
    while(ASYMC_START_REG & 0x01UL);
    PowerConfig_AfterAsymc(&u32Tmp);
    __enable_irq();    
}

//void init_gpio(void);
void init_gpio(void) 
{
    SYSCTRL_ClockEnable(SYSCTRL_Periph_GPIO, ENABLE); 
    SYSCTRL_SoftReset(SYSCTRL_GPIO_RESET);
    
    GPIO->MODE_H = (GPIO->MODE_H & ~(GPIO_MODE_H_PA7_Msk)) | (GPIO_MODE_GPIO << GPIO_MODE_H_PA7_Pos);
    GPIO->CON = (GPIO->CON & ~GPIO_CON_PA7_Msk) | (GPIO_CON_OUT << GPIO_CON_PA7_Pos);
        
    SYSCFG->CBCL_FLAG |= BIT(4) | BIT(5);
    NVIC_EnableIRQ(CB_FLAG_IRQn);
    
}

void CL_FLAG_IRQHandler(void)
{
    SYSCFG->CBCL_FLAG |= BIT(6);
    #if USE_FPGA
    Soft_Restart();
    #else
    SYSCFG->RST_EN = 0x0000FFFF;
    #endif
    while(1);
}

void SystemIdleConfig(void)
{
    FCU->CFG |= BIT(15);//flash deepsleep
    SYSCFG->SYS_CON |= BIT(31);//flash deep sleep
    
//    SSC->TRM_CR1 = *(volatile uint32_t *)(0x03000020); //if softreset the trimcode will not -10 so need reload
    
    SCB->SCR = (SCB->SCR & ~(SCB_SCR_SLEEPDEEP_Msk|SCB_SCR_SLEEPONEXIT_Msk))| SCB_SCR_SLEEPDEEP_Msk;
    
    if (SYSCFG->CBCL_FLAG & SYSCTRL_CBCL_FLAG_CB_FLG)
    {
        GPIO->CON = 0x5555;
        GPIO->MODE_H = 0;
        GPIO->MODE_L = 0;
        
        SYSCTRL_LowPowerConfig(LP_RAM_MODE, SleepState_DeepSleep);
    }
    
    SYSCTRL_LowPowerConfig(LP_ROM_MODE, SleepState_ShutDown);
    

}

void HardFault_Handler()
{
    GPIO->IOD ^= 0x80;
}

void tpdu_init(void);
void SystemInit(void)
{
	  
}

void flash_init_cfg(void)
{
    SYSCTRL_ClocksTypeDef clk;
    SYSCTRL_GetClocksFreq(&clk);
    NVIC_DisableIRQ(SEC_IRQn);
    
//    FLASH_TimInit(clk.HCLK_Frequency);
    FLASH_InfoEn(ENABLE);
    FLASH_ProtectConfig(DISABLE);
    FLASH_SetAllRO(SYS_BASE, SYS_SIZE, DISABLE);
    FLASH_SetAllRO(FLASH_BASE, FLASH_SIZE, DISABLE);
    FLASH_SetAllRO(OTP_BASE, OTP_SIZE, DISABLE);
    
//    FCU->CFG |= BIT(16);
    FCU->CFG &= ~BIT(4);        
    NVIC_EnableIRQ(FLASH_NVM_IRQn);
}

//void FLASH_NVM_IRQHandler()
//{
//    FCU->CS = 0;
//}

void SYSCLK_ADAPT(uint32_t HCLK_Div, uint32_t ALGCLK_Div)
{
    SYSCTRL_HCLKConfig(HCLK_Div);
    SYSCTRL_AlgClkConfig(ALGCLK_Div);
}