/**************************************************************************//**
 * @file     mhcard_sysctrl.h
 * @brief    
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
#ifndef __MHCARD_SYSCTRL_H
#define __MHCARD_SYSCTRL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhcard.h"

/*******Defination of Struct of SYSCFG************/
typedef struct
{
    __IO uint32_t CLK_EN;					//0x00
    __IO uint32_t RST_EN;					//0x04
    __IO uint32_t CLK_CFG;					//0x08
    __IO uint32_t SYS_CON;					//0x0C
    __IO uint32_t RF_DET;					//0x10
    __IO uint32_t RES0[(0x24-0x14) >> 2];	//0x14~0x20
	__IO uint32_t MEM_SIZE_SEL;				//0x24
    __IO uint32_t RES1[(0x40-0x28) >> 2];	//0x28~0x3C
    __IO uint32_t WAKE_MCU_CTRL;			//0x40
    __IO uint32_t WAKE_SYS_CTRL;			//0x44
    __IO uint32_t RES2[(0x60-0x48) >> 2];	//0x48~0x5C
    __IO uint32_t RST_CFG;					//0x60
    __IO uint32_t RES3[(0x80 - 0x64) >> 2];	//0x64~0x7C
    __IO uint32_t DMA_CH;					//0x80
    __IO uint32_t RES4[(0xBC - 0x84) >> 2];	//0x84 ~ 0xB8
	__IO uint32_t MAGDET;					//0xBC	
	__O	 uint32_t PERIFERAL_DISABLE;		//0xC0
    __IO uint32_t ANA_CS;					//0xC4
    __IO uint32_t CBCL_FLAG;				//0xC8
	__IO uint32_t ANA_CTRL;					//0xCC

} SYSCFG_TypeDef;

#define SYSCFG_BASE                         (MHCARD_AHB_BASE + 0x5000)
#define SYSCFG                              ((SYSCFG_TypeDef *) SYSCFG_BASE)   
/** @defgroup SYSCTRL_Exported_Types
  * @{
  */

typedef struct
{
    uint32_t SYS_Frequency;     /*!< returns PLL frequency expressed in Hz */
    uint32_t HCLK_Frequency;    /*!< returns HCLK frequency expressed in Hz */
    uint32_t PCLK_Frequency;    /*!< returns PCLK frequency expressed in Hz */
    uint32_t ALGCLK_Frequency;  /*!< returns ALGCLK frequency expressed in Hz */
}SYSCTRL_ClocksTypeDef;


typedef enum
{
    SYSCTRL_SYSCLK_RC_72MHz     = (uint32_t)0x00,
	SYSCTRL_SYSCLK_ISO7816     	= (uint32_t)0x01,
	SYSCTRL_SYSCLK_RFCLK      	= (uint32_t)0x02,	
}SYSCTRL_SYSCLK_TypeDef;
#define IS_SYSCLK_SOURCE(FREQ)                                                 \
  (((FREQ) == SYSCTRL_SYSCLK_RC_72MHz) || ((FREQ) == SYSCTRL_SYSCLK_ISO7816) ||     \
   ((FREQ) == SYSCTRL_SYSCLK_RFCLK))


typedef enum
{
	SYSCTRL_FLASH_NO_DEEP		= (uint32_t)0x00,
	SYSCTRL_FLASH_ENTER_DEPP	= (uint32_t)0x01,
}FlashDeepState_TypeDef;
#define IS_SYSCLK_FLASHDEEP_STATE(STATE)                                                 \
  (((STATE) == SYSCTRL_FLASH_NO_DEEP) || ((STATE) == SYSCTRL_FLASH_ENTER_DEPP))


#define SYSCTRL_Periph_SYMC         		((uint32_t)0x01000000)
#define SYSCTRL_Periph_I2C			    	((uint32_t)0x00001000)
#define SYSCTRL_Periph_SRAM1				((uint32_t)0x00000800)
#define SYSCTRL_Periph_GPIO        			((uint32_t)0x00000400)
#define SYSCTRL_Periph_SPI       			((uint32_t)0x00000200)
#define SYSCTRL_Periph_ASYMC         		((uint32_t)0x00000100)
#define SYSCTRL_Periph_SECURITY    			((uint32_t)0x00000080)
#define SYSCTRL_Periph_WDT         			((uint32_t)0x00000040)
#define SYSCTRL_Periph_TIMER0      			((uint32_t)0x00000020)
#define SYSCTRL_Periph_ISO7816     			((uint32_t)0x00000010)
#define SYSCTRL_Periph_CRC         			((uint32_t)0x00000008)
#define SYSCTRL_Periph_RNG         			((uint32_t)0x00000004)
#define SYSCTRL_Periph_DMAC        			((uint32_t)0x00000002)
#define SYSCTRL_Periph_ISO14443    			((uint32_t)0x00000001) 
#define SYSCTRL_Periph_ALL					((uint32_t)0x01001FFF)
#define IS_SYSCTRL_CLK_EN(PERIPH)		((((PERIPH) & ~SYSCTRL_Periph_ALL) == 0x00) && ((PERIPH) != 0x00))

                                                                                   
#define SYSCTRL_GLB_BU_RESET         	((uint32_t)0x20000000)
#define SYSCTRL_SYMC_BU_RESET  			((uint32_t)0x08000000)
#define SYSCTRL_ASYMC_BU_RESET 			((uint32_t)0x04000000)
#define SYSCTRL_RNG_BU_RESET         	((uint32_t)0x00400000)
#define SYSCTRL_SOFTRST_BU_MASK			((uint32_t)0x3C400000)

#define SYSCTRL_GLB_RESET            	((uint32_t)0x00004000)
#define SYSCTRL_SYMC_RESET     			((uint32_t)0x00001000)
#define SYSCTRL_ASYMC_RESET    			((uint32_t)0x00000400)
#define SYSCTRL_SPI_RESET          		((uint32_t)0x00000200)
#define SYSCTRL_GPIO_RESET           	((uint32_t)0x00000100)
#define SYSCTRL_I2C_RESET           	((uint32_t)0x00000080)
#define SYSCTRL_RNG_RESET 			 	((uint32_t)0x00000040)
#define SYSCTRL_TIMER0_RESET         	((uint32_t)0x00000020)
#define SYSCTRL_ISO7816_RESET        	((uint32_t)0x00000010)
#define SYSCTRL_CRC_RESET            	((uint32_t)0x00000008)
#define SYSCTRL_WDT_RESET            	((uint32_t)0x00000004)
#define SYSCTRL_DMAC_RESET           	((uint32_t)0x00000002)
#define SYSCTRL_ISO14443_RESET       	((uint32_t)0x00000001)
#define SYSCTRL_SOFTRST_MASK 			((uint32_t)0x000003BF)

#define SYSCTRL_SOFTRST_ALL				((uint32_t)0x2C4057FF)

#define IS_MODULE_SFT_RST(PERIPH)		((((PERIPH) & ~SYSCTRL_SOFTRST_ALL) == 0x00) && ((PERIPH) != 0x00))
                                             

#define SYSCTRL_TICK_REF_SEL_Pos                     (28)
#define SYSCTRL_TICK_REF_SEL_Mask                    (0x01 << SYSCTRL_TICK_REF_SEL_Pos)  
#define SYSCTRL_TICK_REF_SEL_RC_DIV64                ((uint32_t)0x00)
#define SYSCTRL_TICK_REF_SEL_RF_DIV24                ((uint32_t)0x01)


#define IS_SYSCTRL_TICK_REF(PERIPH)					(((PERIPH) == SYSCTRL_TICK_REF_SEL_RC_DIV64) || \
                                                    ((PERIPH) == SYSCTRL_TICK_REF_SEL_RF_DIV24))
                                                    
#define SYSCTRL_CLK_CFG_TIM_REF_SEL_Pos             (26)
#define SYSCTRL_CLK_CFG_TIM_REF_SEL_Msk             (0x03UL <<  SYSCTRL_CLK_CFG_TIM_REF_SEL_Pos)
#define SYSCTRL_CLK_CFG_TIM_REF_SEL_RC              (0x00UL)
#define SYSCTRL_CLK_CFG_TIM_REF_SEL_RF              (0x01UL)
#define SYSCTRL_CLK_CFG_TIM_REF_SEL_7816            (0x02UL)

#define IS_SYSCTRL_TIM_REF(PERIPH)					(((PERIPH) == SYSCTRL_CLK_CFG_TIM_REF_SEL_RC) || \
                                                     ((PERIPH) == SYSCTRL_CLK_CFG_TIM_REF_SEL_RF) || \
                                                     ((PERIPH) == SYSCTRL_CLK_CFG_TIM_REF_SEL_7816)  \
                                                    )


#define SYSCTRL_HCLK_SEL_Pos                        (16)
#define SYSCTRL_HCLK_SEL_Mask                       (0x1F << SYSCTRL_HCLK_SEL_Pos)  

#define SYSCTRL_HCLK_D32M1                          ((uint32_t)0x00000000)
#define SYSCTRL_HCLK_D32M2                          ((uint32_t)0x00000001)
#define SYSCTRL_HCLK_D32M3                          ((uint32_t)0x00000002)
#define SYSCTRL_HCLK_D32M4                          ((uint32_t)0x00000003)
#define SYSCTRL_HCLK_D32M5                          ((uint32_t)0x00000004)
#define SYSCTRL_HCLK_D32M6                        	((uint32_t)0x00000005)
#define SYSCTRL_HCLK_D32M7                        	((uint32_t)0x00000006)
#define SYSCTRL_HCLK_D32M8                        	((uint32_t)0x00000007)
#define SYSCTRL_HCLK_D32M9                        	((uint32_t)0x00000008)
#define SYSCTRL_HCLK_D32M10                        	((uint32_t)0x00000009)
#define SYSCTRL_HCLK_D32M11                        	((uint32_t)0x0000000A)
#define SYSCTRL_HCLK_D32M12                        	((uint32_t)0x0000000B)
#define SYSCTRL_HCLK_D32M13                        	((uint32_t)0x0000000C)
#define SYSCTRL_HCLK_D32M14                        	((uint32_t)0x0000000D)
#define SYSCTRL_HCLK_D32M15                        	((uint32_t)0x0000000E)
#define SYSCTRL_HCLK_D32M16                        	((uint32_t)0x0000000F)
#define SYSCTRL_HCLK_D32M17                        	((uint32_t)0x00000010)
#define SYSCTRL_HCLK_D32M18                        	((uint32_t)0x00000011)
#define SYSCTRL_HCLK_D32M19                        	((uint32_t)0x00000012)
#define SYSCTRL_HCLK_D32M20                        	((uint32_t)0x00000013)
#define SYSCTRL_HCLK_D32M21                        	((uint32_t)0x00000014)
#define SYSCTRL_HCLK_D32M22                        	((uint32_t)0x00000015)
#define SYSCTRL_HCLK_D32M23                        	((uint32_t)0x00000016)
#define SYSCTRL_HCLK_D32M24                        	((uint32_t)0x00000017)
#define SYSCTRL_HCLK_D32M25                        	((uint32_t)0x00000018)
#define SYSCTRL_HCLK_D32M26                        	((uint32_t)0x00000019)
#define SYSCTRL_HCLK_D32M27                        	((uint32_t)0x0000001A)
#define SYSCTRL_HCLK_D32M28                        	((uint32_t)0x0000001B)
#define SYSCTRL_HCLK_D32M29                        	((uint32_t)0x0000001C)
#define SYSCTRL_HCLK_D32M30                        	((uint32_t)0x0000001D)
#define SYSCTRL_HCLK_D32M31                         ((uint32_t)0x0000001E)
#define SYSCTRL_HCLK_D32M32                         ((uint32_t)0x0000001F)
#define IS_GET_SYSCTRL_HCLK_DIV(DIV)                (((DIV) == SYSCTRL_HCLK_D32M1) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M2)  || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M3)  || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M4)  || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M5)  || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M6) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M7) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M8) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M9) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M10) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M11) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M12) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M13) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M14) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M15) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M16) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M17) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M18) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M19) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M20) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M21) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M22) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M23) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M24) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M25) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M26) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M27) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M28) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M29) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M30) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M31) || \
                                                    ((DIV) == SYSCTRL_HCLK_D32M32))
                                                    
#define SYSCTRL_ALGCLK_SEL_Pos                        (8)
#define SYSCTRL_ALGCLK_SEL_Mask                       (0x1F << SYSCTRL_ALGCLK_SEL_Pos)  
                
#define SYSCTRL_ALGCLK_D32M1                            ((uint32_t)0x00000000)
#define SYSCTRL_ALGCLK_D32M2                            ((uint32_t)0x00000001)
#define SYSCTRL_ALGCLK_D32M3                            ((uint32_t)0x00000002)
#define SYSCTRL_ALGCLK_D32M4                            ((uint32_t)0x00000003)
#define SYSCTRL_ALGCLK_D32M5                            ((uint32_t)0x00000004)
#define SYSCTRL_ALGCLK_D32M6                            ((uint32_t)0x00000005)
#define SYSCTRL_ALGCLK_D32M7                            ((uint32_t)0x00000006)
#define SYSCTRL_ALGCLK_D32M8                            ((uint32_t)0x00000007)
#define SYSCTRL_ALGCLK_D32M9                            ((uint32_t)0x00000008)
#define SYSCTRL_ALGCLK_D32M10                           ((uint32_t)0x00000009)
#define SYSCTRL_ALGCLK_D32M11                           ((uint32_t)0x0000000A)
#define SYSCTRL_ALGCLK_D32M12                           ((uint32_t)0x0000000B)
#define SYSCTRL_ALGCLK_D32M13                           ((uint32_t)0x0000000C)
#define SYSCTRL_ALGCLK_D32M14                           ((uint32_t)0x0000000D)
#define SYSCTRL_ALGCLK_D32M15                           ((uint32_t)0x0000000E)
#define SYSCTRL_ALGCLK_D32M16                           ((uint32_t)0x0000000F)
#define SYSCTRL_ALGCLK_D32M17                           ((uint32_t)0x00000010)
#define SYSCTRL_ALGCLK_D32M18                           ((uint32_t)0x00000011)
#define SYSCTRL_ALGCLK_D32M19                           ((uint32_t)0x00000012)
#define SYSCTRL_ALGCLK_D32M20                           ((uint32_t)0x00000013)
#define SYSCTRL_ALGCLK_D32M21                           ((uint32_t)0x00000014)
#define SYSCTRL_ALGCLK_D32M22                           ((uint32_t)0x00000015)
#define SYSCTRL_ALGCLK_D32M23                           ((uint32_t)0x00000016)
#define SYSCTRL_ALGCLK_D32M24                           ((uint32_t)0x00000017)
#define SYSCTRL_ALGCLK_D32M25                           ((uint32_t)0x00000018)
#define SYSCTRL_ALGCLK_D32M26                           ((uint32_t)0x00000019)
#define SYSCTRL_ALGCLK_D32M27                           ((uint32_t)0x0000001A)
#define SYSCTRL_ALGCLK_D32M28                           ((uint32_t)0x0000001B)
#define SYSCTRL_ALGCLK_D32M29                           ((uint32_t)0x0000001C)
#define SYSCTRL_ALGCLK_D32M30                           ((uint32_t)0x0000001D)
#define SYSCTRL_ALGCLK_D32M31                           ((uint32_t)0x0000001E)
#define SYSCTRL_ALGCLK_D32M32                           ((uint32_t)0x0000001F)
#define IS_GET_SYSCTRL_ALGCLK_DIV(DIV)                (((DIV) == SYSCTRL_ALGCLK_D32M1) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M2)  || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M3)  || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M4)  || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M5)  || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M6) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M7) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M8) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M9) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M10) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M11) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M12) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M13) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M14) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M15) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M16) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M17) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M18) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M19) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M20) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M21) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M22) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M23) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M24) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M25) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M26) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M27) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M28) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M29) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M30) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M31) || \
                                                    ((DIV) == SYSCTRL_ALGCLK_D32M32))

#define SYSCTRL_PCLK_SEL_Pos                        (24)
#define SYSCTRL_PCLK_SEL_Mask                       (0x03 << SYSCTRL_PCLK_SEL_Pos)  
#define SYSCTRL_PCLK_Div_None                       ((uint32_t)0x00)
#define SYSCTRL_PCLK_Div2                           ((uint32_t)0x01)
#define SYSCTRL_PCLK_Div4                           ((uint32_t)0x02)
#define IS_GET_SYSCTRL_PCLK_DIV(DIV)                (((DIV) == SYSCTRL_PCLK_Div_None) || \
                                                    ((DIV) == SYSCTRL_PCLK_Div2) || \
                                                    ((DIV) == SYSCTRL_PCLK_Div4))


#define SYSCTRL_WAKE_MCU_CTRL_Pos					(0)
#define SYSCTRL_WAKE_MCU_CTRL_Mask					(0xFF << SYSCTRL_WAKE_MCU_CTRL_Pos)
#define SYSCTRL_TIME2_WK_MCU		((uint32_t)0x00000080)
#define SYSCTRL_SWD_WK_DEEPSLEEP	((uint32_t)0x00000040)
#define SYSCTRL_DMAC_WK_MCU      	((uint32_t)0x00000020)
#define SYSCTRL_CRYPT_WK_MCU     	((uint32_t)0x00000010)
#define SYSCTRL_TIMER1_WK_MCU    	((uint32_t)0x00000008)
#define SYSCTRL_TIMER0_WK_MCU    	((uint32_t)0x00000004)
#define SYSCTRL_ISO14443_WK_MCU  	((uint32_t)0x00000002)
#define SYSCTRL_ISO7816_WK_MCU   	((uint32_t)0x00000001)
#define SYSCTRL_ALL_WK_MCU			((uint32_t)0x000000FF)
#define IS_GET_WAKE_MCU_CTRL(PERIPH)            ((((PERIPH) & ~SYSCTRL_ALL_WK_MCU) == 0x00) && ((PERIPH) != 0x00))


/*******************  Bit definition for CLK_CFG register  *******************/
#define SYSCTRL_SRCCLK_SEL_Pos                  	(0)
#define SYSCTRL_SRCCLK_SEL_Mask                 	(0x03 << SYSCTRL_SRCCLK_SEL_Pos)

#define SYSCTRL_SYSCON_FLASH_DEEPSLEEP_Pos			(31)
#define SYSCTRL_SYSCON_FLAHS_DEEPSLEEP_Mask			((uint32_t)0x1 << SYSCTRL_SYSCON_FLASH_DEEPSLEEP_Pos)

#define SYSCTRL_SYSCON_LP_RAM_MODE_Pos				(29)
#define SYSCTRL_SYSCON_LP_RAM_MODE_Mask				(0x3 << SYSCTRL_SYSCON_LP_RAM_MODE_Pos)

#define SYSCTRL_SYSCON_LP_ROM_MODE_Pos				(27)
#define SYSCTRL_SYSCON_LP_ROM_MODE_Mask				(0x3 << SYSCTRL_SYSCON_LP_ROM_MODE_Pos)

#define SYSCTRL_SYSCON_LP_RAM32K_MODE_Pos			(25)
#define SYSCTRL_SYSCON_LP_RAM32K_MODE_Mask			(0x3 << SYSCTRL_SYSCON_LP_RAM32K_MODE_Pos)

/*Defination of SYS_CON Register*/
#define SYSCTRL_SYSCON_FLASH_DEEPSLEEP_EN      (BIT(31))  

#define SYSCTRL_SYSCON_CACHE_BYPASS_Msk        (0x000000FFUL)
#define SYSCTRL_SYSCON_CACHE_BYPASS            (0xA5) //CPU Catch the Code From Flash Directly
#define SYSCTRL_SYSCON_LP_RAM_MODE_LS          (BIT(29))
#define SYSCTRL_SYSCON_LP_RAM_MODE_DS          (BIT(30))
#define SYSCTRL_SYSCON_LP_RAM_MODE_SD          (BIT(29)|BIT(30))                                             


/*Defination of RF_DET Register*/
#define SYSCTRL_RF_DET_AUTOCLK_CHANGE_EN     (BIT(31))

/*Defination of PCLK_SEL_TABLE register*/
#define SYSCTRL_PCLK_SEL_TABLE7_Pos          (14)
#define SYSCTRL_PCLK_SEL_TABLE6_Pos          (12)
#define SYSCTRL_PCLK_SEL_TABLE5_Pos          (10)
#define SYSCTRL_PCLK_SEL_TABLE4_Pos          (8)
#define SYSCTRL_PCLK_SEL_TABLE3_Pos          (6)
#define SYSCTRL_PCLK_SEL_TABLE2_Pos          (4)
#define SYSCTRL_PCLK_SEL_TABLE1_Pos          (2)
#define SYSCTRL_PCLK_SEL_TABLE0_Pos          (0)

/*Defination of HCLK_SEL_TABLE register*/
#define SYSCTRL_HCLK_SEL_TABLE7_Pos          (21)
#define SYSCTRL_HCLK_SEL_TABLE6_Pos          (18)
#define SYSCTRL_HCLK_SEL_TABLE5_Pos          (15)
#define SYSCTRL_HCLK_SEL_TABLE4_Pos          (12)
#define SYSCTRL_HCLK_SEL_TABLE3_Pos          (9)
#define SYSCTRL_HCLK_SEL_TABLE2_Pos          (6)
#define SYSCTRL_HCLK_SEL_TABLE1_Pos          (3)
#define SYSCTRL_HCLK_SEL_TABLE0_Pos          (0)

/*Defination of SYMCLK_SEL_TABLE register*/
#define SYSCTRL_SYMCLK_SEL_TABLE7_Pos          (21)
#define SYSCTRL_SYMCLK_SEL_TABLE6_Pos          (18)
#define SYSCTRL_SYMCLK_SEL_TABLE5_Pos          (15)
#define SYSCTRL_SYMCLK_SEL_TABLE4_Pos          (12)
#define SYSCTRL_SYMCLK_SEL_TABLE3_Pos          (9)
#define SYSCTRL_SYMCLK_SEL_TABLE2_Pos          (6)
#define SYSCTRL_SYMCLK_SEL_TABLE1_Pos          (3)
#define SYSCTRL_SYMCLK_SEL_TABLE0_Pos          (0)

/*Defination of ASYMCLK_SEL_TABLE register*/
#define SYSCTRL_ASYMCLK_SEL_TABLE7_Pos          (21)
#define SYSCTRL_ASYMCLK_SEL_TABLE6_Pos          (18)
#define SYSCTRL_ASYMCLK_SEL_TABLE5_Pos          (15)
#define SYSCTRL_ASYMCLK_SEL_TABLE4_Pos          (12)
#define SYSCTRL_ASYMCLK_SEL_TABLE3_Pos          (9)
#define SYSCTRL_ASYMCLK_SEL_TABLE2_Pos          (6)
#define SYSCTRL_ASYMCLK_SEL_TABLE1_Pos          (3)
#define SYSCTRL_ASYMCLK_SEL_TABLE0_Pos          (0)

/*Defination of Wake_MCU_CTRL register*/
#define SYSCTRL_WAKE_MCU_CTRL_DMA_WK_EN         BIT(5)
#define SYSCTRL_WAKE_MCU_CTRL_CRYPT_WK_EN       BIT(4)
#define SYSCTRL_WAKE_MCU_CTRL_TIMER1_WK_EN      BIT(3)
#define SYSCTRL_WAKE_MCU_CTRL_TIMER0_WK_EN      BIT(2)
#define SYSCTRL_WAKE_MCU_CTRL_ISO14443_Wk_EN    BIT(1)
#define SYSCTRL_WAKE_MCU_CTRL_ISO7816_WK_EN     BIT(0)

/*Defination of WAKE_SYS_CTRL register*/
#define SYSCTRL_WAKE_SYS_CTRL_SEL_ISO14443_EN    BIT(31) 
#define SYSCTRL_WAKE_SYS_CTRL_SCI_CLK_SEL_INTERNEL  BIT(30)
#define SYSCTRL_WAKE_SYS_CTRL_ASYMC_CONTINUE_EN  BIT(2)
#define SYSCTRL_WAKE_SYS_CTRL_SYMC_CONTINUE_EN   BIT(1)
#define SYSCTRL_WAKE_SYS_CTRL_ISO7816_Wk_EN      BIT(0)

/*Defination of RST_CFG register*/
#define SYSCTRL_RST_CFG_CBFLAG_14443RST_EN       BIT(1)
#define SYSCTRL_RST_CFG_ISO7816_SYSRST_EN        BIT(0)

/*Defination of DMA_CH Register*/
//#define SYSCTRL_DMA_CH_ISO7816_SYS_EN            BIT(0)
#define SYSCTRL_DMA_CH_ISO1443_RX              (0)
#define SYSCTRL_DMA_CH_ISO1443_TX              (1)
#define SYSCTRL_DMA_CH_SPI_RX                  (2)
#define SYSCTRL_DMA_CH_SPI_TX                  (3)
#define SYSCTRL_DMA_CH_I2C_RX                  (4)
#define SYSCTRL_DMA_CH_I2C_TX                  (5)
#define SYSCRTL_DMA_CH_MASK                    (BIT(0)|BIT(1)|BIT(2))

/*Defination of ANA_CS register*/
#define SYSCTRL_ANA_CS_VTS_CMP                  BIT(1)
#define SYSCTRL_ANA_CS_VGEN_CMP                 BIT(0)

/*Defination of CBCL_FLAG register*/
#define SYSCTRL_CBCL_FLAG_CL_FLG               BIT(1) //iso14443 flag
#define SYSCTRL_CBCL_FLAG_CB_FLG               BIT(0)//iso7816 flag

#define SYSCTRL_MEM_LP_SRAM                         BIT(4)
#define SYSCTRL_MEM_LP_ROM                          BIT(5)

#define IS_SYSCTRL_MEM_LP(MODULE)                   ((((MODULE) & ~SYSCTRL_MEM_LP_ALL) == 0x00) && ((MODULE) != 0x00))

/*Defination of FLASH_TEST_EN register*/
#define SYSCTRL_FLASH_TEST_EN_Msk               (0xFF)
#define SYSCTRL_FLASH_TEST_EN                   (0x5A)

typedef enum
{
	LP_RAM_MODE = 0,
	LP_ROM_MODE,
	LP_RAM32K_MODE
}SleepSelectMode_TypeDef;
#define IS_SYSCON_LP_MODE(MODE)					(((MODE) == LP_RAM_MODE) || ((MODE) == LP_ROM_MODE) || ((MODE) == LP_RAM32K_MODE))

typedef enum
{
    SleepState_Working = 0,            /*!< Working Mode */
    SleepState_LightSleep,             /*!< Light Sleep */
    SleepState_DeepSleep,              /*!< Deep Sleep */
	SleepState_ShutDown,               /*!< Shut Down */
    SleepState_Invalid
}SleepState_TypeDef;
#define IS_ALL_SLEEP_STATE(STATE)                     ((STATE) < SleepState_Invalid) 

typedef enum
{
    SleepMode_CpuOff = 0,       /*!< CPU Sleep  */
    SleepMode_DeepSleep,        /*!< Deep Sleep */
    SleepMode_Invalid
}SleepMode_TypeDef;
#define IS_ALL_SLEEP_MODE(MODE)                     ((MODE) < SleepMode_Invalid) 

/********************* System control reference APIs ***************************************/
void SYSCTRL_ClockEnable(uint32_t SYSCTRL_Periph, FunctionalState NewState);
void SYSCTRL_SoftReset(uint32_t SYSCTRL_Periph);

void SYSCTRL_SYSCLKSourceSelect(SYSCTRL_SYSCLK_TypeDef source);
void SYSCTRL_PCLKConfig(uint32_t PCLK_Div);
void SYSCTRL_HCLKConfig(uint32_t HCLK_Div);
void SYSCTRL_AlgClkConfig(uint32_t ALGCLK_Div);
void SYSCTRL_AsymclkConfig(uint32_t Asymclk_Div);
void SYSCTRL_SymclkConfig(uint32_t Symclk_Div);

void SYSCTRL_GetClocksFreq(SYSCTRL_ClocksTypeDef* SYSCTRL_Clocks);

void SYSCTRL_TIMRefConfig(uint32_t Tim_Sel);

void SYSCTRL_FlashDeepConfig(FlashDeepState_TypeDef FlashDeepState);
void SYSCTRL_LowPowerConfig(uint32_t module, SleepState_TypeDef SleepState);
void SYSCTRL_EnterSleep(SleepMode_TypeDef SleepMode);

void SYSCTRL_WakeMcu(uint32_t WakeSelect);
void SYSCTRL_RstCfg(uint32_t RstCfg);

#ifdef __cplusplus
}
#endif


#endif 

/**************************      (C) COPYRIGHT    *****END OF FILE****/
