/**************************************************************************//**
 * @file     mhcard_ssc.c
 * @brief    This file provides all the system firmware function.
 * @version  V1.00
 * @date     6. April 2022
 *
 * @note
 *
 ******************************************************************************/
#include "mhcard_ssc.h"
#include "mhcard_sysctrl.h"
#include "stdio.h"
#include "mhcard_flash.h"

uint32_t SSC_GetAttack(uint32_t u32Det)
{
    return SSC->INT_STAT & u32Det;
}

uint32_t SSC_ClearAttack(uint32_t u32Det)
{
    SSC->INT_STAT = u32Det;
    return SSC->INT_STAT;
}

uint32_t SSC_GetRespMode(uint32_t u32Det)
{
    uint32_t u32Tmp = SSC->ATT_CR & u32Det;
    if (u32Tmp & SSC_DETECTOR_TRNG)
    {
        u32Tmp |= SSC_DETECTOR_RNG;
    }
    return u32Tmp;
}

uint32_t SSC_GetSelfCheckEnable(uint32_t u32Det)
{
    return SSC->ST_EN & u32Det;
}

uint32_t SSC_SetSelfCheckEnable(uint32_t u32Det)
{
    SSC->ST_EN = u32Det;
    while(SSC->ST_END_FALG == 0);
    return SSC->ST_EN;
}

uint32_t SSC_GetSelfCheckResult(uint32_t u32Det)
{
    return SSC->ST_ISR & u32Det;
}

uint32_t SSC_ClearSelfCheckResult(uint32_t u32Det)
{
    SSC->ST_ISR = u32Det;
    return SSC->ST_ISR;
}
ITStatus SSC_GetITStatus(uint32_t SSC_IT)
{
	assert_param(IS_SSC_INT(SSC_IT));
	
	if (SSC->INT_STAT & SSC_IT)
	{
		return SET;
	}
	
	return RESET;
}

void SSC_ClearITPendingBit(uint32_t SSC_IT)
{
	assert_param(IS_SSC_INT(SSC_IT));	
	
	if (SSC->INT_STAT & SSC_IT)
	{
		SSC->INT_STAT = SSC_IT;
	}
    if (SSC_IT & SSC_INT_STAT_GDE_INT_Mask)
    {
        SSC->GLITCH_CLR = SSC_GLITCH_CLR_GDE;
    }
    if (SSC_IT & SSC_INT_STAT_GDI_INT_Mask)
    {
        SSC->GLITCH_CLR = SSC_GLITCH_CLR_GDI;
    }
}


void SSC_SelfCheckConfig(uint32_t SSC_SelfCheck, FunctionalState NewState)
{
	assert_param(IS_SSC_ST_EN(SSC_SelfCheck));

	if (DISABLE != NewState)
	{
		SSC->ST_EN |= SSC_SelfCheck;
	}
	else
	{
		SSC->ST_EN &= ~SSC_SelfCheck;
	}
}

FlagStatus SSC_GetSelfCheckStatus(uint32_t SSC_SelfCheck)
{
	assert_param(IS_SSC_ST_ISR(SSC_SelfCheck));
	
	if (SSC->ST_ISR & SSC_SelfCheck)
	{
		return SET;
	}
	
	return RESET;	
}

void SSC_ClearSelfCheckStatus(uint32_t SSC_SelfCheck)
{
	assert_param(IS_SSC_ST_ISR(SSC_SelfCheck));	
	
	SSC->ST_ISR |= SSC_SelfCheck;
}

void SSC_SecureEnable(void)
{
    uint32_t u32PolCr;
    u32PolCr = SSC->DITHER;
    u32PolCr |= SSC->DITHER << 16;
    u32PolCr &= (0x00ffffff);//config all the polarity
    SSC->POL_CR = u32PolCr;
}

void  SSC_SecureDisable(void)
{
    uint32_t u32PolCr;
    u32PolCr = 0xA5A5A5A5;
    SSC->POL_CR = u32PolCr;
}

void SSC_IflushEnable(void)
{
	uint8_t u8IflushCr;
	u8IflushCr = (uint8_t)SSC->DITHER;
    SSC->IFLUSH_CR = u8IflushCr;
}

void SSC_IflushDisable(void)
{
	uint8_t u8IflushCr;
	u8IflushCr = 0xA5;
    SSC->IFLUSH_CR = u8IflushCr;
}

/**************************      (C) COPYRIGHT    *****END OF FILE****/
