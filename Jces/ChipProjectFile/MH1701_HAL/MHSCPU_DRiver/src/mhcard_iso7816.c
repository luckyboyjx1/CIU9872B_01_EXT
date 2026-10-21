#include "mhcard_iso7816.h"
#include "mhcard_timer.h"
#include "mhcard_sysctrl.h"
#include "string.h"
#include "mhcard.h"

uint32_t SCI_GetCwtData(void)
{
    return SCI->CWTDATA;
}

uint32_t SCI_GetBgtData(void)
{
    return SCI->BGTDATA;
}

void SCI_SetCwtData(uint16_t u16CwtData)
{
    assert_param(IS_CWT_DATA(u16CwtData));
    SCI->CWTDATA = u16CwtData;
}

void SCI_SetBgtData(uint16_t u16BgtData)
{
    SCI->BGTDATA = u16BgtData;
}

void SCI_BgtCwtEnable(uint32_t u32Whom, FunctionalState state)
{
    assert_param(IS_BGT_CWT(u32Whom));
    if (state == ENABLE)
    {
        SCI->BGTCTRL |= u32Whom;
    }
    else
    {
        SCI->BGTCTRL &= ~u32Whom;
    }
}

void SCI_EdcEnable(FunctionalState state)
{
    assert_param(IS_FUNCTIONAL_STATE(state));
    if (state == ENABLE)
    {
        SCI->EDCCTRL |= SCI_EDCCTRL_EDC_EN;
    }
    else
    {
        SCI->EDCCTRL &= ~SCI_EDCCTRL_EDC_EN;
    }
}

void SCI_SetSendNULL(FunctionalState state)
{
    assert_param(IS_FUNCTIONAL_STATE(state));
    if (state == ENABLE)
    {
        SCI->MODHW |= SCI_MODHW_SEND_NULL_EN;
    }
    else
    {
        SCI->MODHW &= ~SCI_MODHW_SEND_NULL_EN;
    }
}

void SCI_SetTMode(uint32_t u32TMode)
{
    assert_param(IS_SCI_T_MODE(u32TMode));
    if (u32TMode == SCI_MODHW_TMODE_T1)
    {
        SCI->MODHW |= SCI_MODHW_TMODE_T1;
    }
    else
    {
        SCI->MODHW &= ~SCI_MODHW_TMODE_T1;
    }
}

void SCI_SetRetryTimes(uint32_t u32ReTimes)
{
    assert_param(IS_SCI_RETRY_TIMES(u32ReTimes));
    SCI->MODHW = (SCI->MODHW & ~SCI_MODHW_RETRY_SEL_Msk) | u32ReTimes;
}

void SCI_SetExtraETU(uint32_t u32ExETU)
{
    assert_param(IS_SCI_EXTRA_ETU(u32ExETU));
    SCI->MODHW = (SCI->MODHW & ~SCI_MODHW_EX_ETU_SEL_Msk) | u32ExETU;
}

uint8_t SCI_GetChar(void)
{
    return (SCI->BUFHW & SCI_BUFHW_DATA_Msk);
}

uint8_t SCI_SendChar(uint8_t u8Char)
{
    SCI->BUFHW = u8Char;
    return u8Char;
}

void SCI_ITConfig(uint32_t u32Int, FunctionalState state)
{
    assert_param(IS_SCI_IT(u32Int));
    if (state == ENABLE)
    {
        SCI->CTRL |= u32Int;
    }
    else
    {
        SCI->CTRL &= ~u32Int;
    }
}

void SCI_ClearFifo(uint32_t u32Whom)
{
    assert_param(IS_SCI_FIFO_CLR(u32Whom));
    SCI->CTRL |= u32Whom;
}

FlagStatus SCI_GetStatus(uint32_t u32Flag)
{
    assert_param(IS_SCI_FLAG(u32Flag));
    return (0 != (SCI->STAT & u32Flag));
}

ITStatus SCI_GetITStatus(uint32_t u32ITStatus)
{
    assert_param(IS_SCI_IT_FLAG(u32ITStatus));
    return (0 != (SCI->INTIO1 & u32ITStatus));
}

void SCI_ClearIT(uint32_t u32ITStatus)
{
    assert_param(IS_SCI_IT_FLAG(u32ITStatus));
    SCI->INTIO1 = u32ITStatus;
}

void SCI_RetryEnable(FunctionalState state)
{
    assert_param(IS_FUNCTIONAL_STATE(state));
    if (state == ENABLE)
    {
        SCI->CTRL |= SCI_CTRL_RETRY_EN;
    }
    else
    {
        SCI->CTRL &= ~SCI_CTRL_RETRY_EN;
    }
}

void SCI_ExRstIntEdge(uint32_t u32Edge)
{
    if (u32Edge == SCI_CTRL_EXTRST_INT_FALL_EDGE)
    {
        SCI->CTRL |= SCI_CTRL_EXTRST_INT_FALL_EDGE;
    }
    else
    {
        SCI->CTRL &= ~SCI_CTRL_EXTRST_INT_FALL_EDGE;
    }
}

void SCI_SetEtuData(uint16_t u16Etu)
{
    SCI->ETUDATA = u16Etu;
}

uint8_t SCI_GetEdcData(void)
{
    return SCI->EDCDATA & 0xFF;
}
