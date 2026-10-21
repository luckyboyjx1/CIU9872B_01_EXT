/**
  ******************************************************************************
  * @file    mhcard_flash.c
  * @author  
  * @version 
  * @date    
  * @brief   This file provides all the FLASH firmware functions.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "mhcard_flash.h"
#include "mhcard_cache.h"
#include "system_mhcard.h"
#include "stdio.h"
static volatile uint32_t u32Hclk1msVal = 48000000/1000;
static volatile uint32_t gu32FLASH_Key1 = 0;
static volatile uint32_t gu32FLASH_Key2 = 0;


void FLASH_SleepConfig(FunctionalState state)
{
    assert_param(IS_FUNCTIONAL_STATE(state));
    if (ENABLE == state)
    {
        FCU->CFG |= FCU_CFG_SLEEP_EN;
    }
    else
    {
        FCU->CFG &= ~FCU_CFG_SLEEP_EN;
    }
}

void FLASH_ProgWordSize(uint8_t u8WordSize)
{
    assert_param(IS_PROG_WORD_SIZE(u8WordSize));
    FCU->CFG &= ~FCU_CFG_PG_SIZE_Msk;
    FCU->CFG |= (FCU_CFG_PG_SIZE(u8WordSize - 1) & FCU_CFG_PG_SIZE_Msk);
}

void FLASH_SetSmartProv(FlashSmartProvider provider)
{
    assert_param(IS_FLASH_SMART_PROVIDER(provider));

    if (FLASH_SMART_PROVIDER_MODULE == provider)
    {
        FCU->CFG |= FCU_CFG_SM_OP_SEL_MOD;
    }
    else
    {
        FCU->CFG &= ~FCU_CFG_SM_OP_SEL_Msk;
    }
}

void FLASH_OprateSel(Flash_Op_ID opId)
{
    assert_param(IS_FLASH_OP_ID(opId));
    FCU->CFG &= ~FCU_CFG_OP_SEL_Msk;
    FCU->CFG |= (opId & FCU_CFG_OP_SEL_Msk); 
}

void FLASH_InfoEn(FunctionalState state)
{
    assert_param(IS_FUNCTIONAL_STATE(state));
    if (ENABLE == state)
    {
        FCU->INFO_EN |= FCU_INFO_EN_INFO_OP_EN;
    }
    else
    {
        FCU->INFO_EN &= ~FCU_INFO_EN_INFO_OP_EN;
    }
}

void FLASH_ITConfig(uint32_t u32Int, FunctionalState state)
{
    assert_param(IS_FLASH_IT(u32Int));
    assert_param(IS_FUNCTIONAL_STATE(state));
    
    if (ENABLE == state)
    {
        FCU->CFG &= ~(u32Int);
    }
    else
    {
        FCU->CFG |= (u32Int);
    }
}

void FLASH_EccConfig(FunctionalState state)
{
    assert_param(IS_FUNCTIONAL_STATE(state));
    if (ENABLE == state)
    {
        FCU->CFG &= ~FCU_CFG_ECC_Msk;
    }
    else
    {
        FCU->CFG |= FCU_CFG_ECC_OPEN;
    }
}

uint32_t FLASH_CopyBufToPgBuf(void *pvSrc, uint16_t u16Len)
{
    uint32_t i;
    uint32_t au32Tmp[64];
    if (u16Len > 256)
    {
        return -1;
    }
    if (!(FCU->CS & FCU_CS_IDLE))
    {   
        return FLASH_BUSY;
    }
    memcpy(au32Tmp, pvSrc, u16Len);
    for (i = 0; i < ((u16Len + 3) >> 2); i += 1)
    {
        FCU->PG_BUF = au32Tmp[i];
        while(FCU->CS & BIT(29));
    }
    return 0;
}


#define FLASH_SLEEP_OP_MODE     (0)
#if (FLASH_SLEEP_OP_MODE)
static uint32_t u32FCU_CS;
void FLASH_NVM_IRQHandler(void)
{
    u32FCU_CS = FCU->CS;
    FCU->CS = 0;
    
}
#endif
uint32_t FLASH_Operate(void)
{
    uint32_t u32NVIC_ENABLE;
    uint32_t u32SysTickCSR;
    
#if (FLASH_SLEEP_OP_MODE)
    //FCU->CFG &= ~BIT(16);
    FCU->CFG |= BIT(16);
    FCU->CS = 0;//clear state
    u32FCU_CS = 0;
    
    FCU->PROT = gu32FLASH_Key1;
	FCU->PROT = gu32FLASH_Key2;
    
    u32NVIC_ENABLE = NVIC->ICER[0];//save 
    u32SysTickCSR = SysTick->CTRL;
    
    SysTick->CTRL &= ~BIT(0); //disable SysTick
    NVIC->ICER[0] = 0xFFFFFFFF;
    NVIC->ISER[0] = BIT(FLASH_NVM_IRQn);//enable
    
    FCU->CS |= BIT(31);
    
    SCB->SCR = (SCB->SCR & ~(SCB_SCR_SLEEPDEEP_Msk|SCB_SCR_SLEEPONEXIT_Msk));
    __wfi();
    SCB->SCR = (SCB->SCR & ~(SCB_SCR_SLEEPDEEP_Msk|SCB_SCR_SLEEPONEXIT_Msk))| SCB_SCR_SLEEPDEEP_Msk;
    
    NVIC->ICER[0] = 0xFFFFFFFF;
    NVIC->ISER[0] = u32NVIC_ENABLE;
    SysTick->CTRL = u32SysTickCSR;
    return (u32FCU_CS & FCU_CS_ILLEGAL);
#else
    __disable_irq();
//    ((void (*)(void))0x00000255)();
    FCU->CS = 0;
    FCU->PROT = gu32FLASH_Key1;
	FCU->PROT = gu32FLASH_Key2;
    FCU->CS |= BIT(31);
    while(FCU->CS & BIT(31));
    __enable_irq();
    return FCU->CS & FCU_CS_ILLEGAL;
#endif
}

uint32_t FLASH_ProtectConfig(FunctionalState state)
{
    assert_param(IS_FUNCTIONAL_STATE(state));
    
    if (!(FCU->CS & FCU_CS_IDLE))
    {
        return FLASH_BUSY;
    }
    if (state == ENABLE) 
    {
        gu32FLASH_Key1 = 0;
        gu32FLASH_Key2 = 0;
        FCU->PROT = FLASH_KEY2;
    }
    else
    {
        gu32FLASH_Key1 = FLASH_KEY1;
        gu32FLASH_Key2 = FLASH_KEY2;
    }
}

uint32_t FLASH_GetIntersection(Intersection_TypeDef *pIn)
{
    pIn->u32OS = pIn->u32IS1 > pIn->u32IS2 ? pIn->u32IS1 : pIn->u32IS2;
    pIn->u32OE = pIn->u32IE1 > pIn->u32IE2 ? pIn->u32IE2 : pIn->u32IE1;
    if (pIn->u32OS >= pIn->u32OE)
    {
        pIn->u32OS = 0;
        pIn->u32OE = 0;
        return 0;
    }
    return pIn->u32OE - pIn->u32OS;
}

#define RANGE_TO_BIT_MAP(base, start, len, sizeBit) (0xFFFFFFFF >> (31 - (((start - base) + (len - 1)) >> sizeBit))) \
                                                        & (0xFFFFFFFF << ((start - base) >> sizeBit))

uint32_t FLASH_SetRO(uint32_t u32Start, uint32_t u32Len, FunctionalState state)
{
    volatile uint32_t u32Map, *pu32Dst, *pu32ROL, u32Tmp;
    
    assert_param(IS_FUNCTIONAL_STATE(state));
    
    if (!(FCU->CS & FCU_CS_IDLE))
    {   
        return FLASH_BUSY;
    }
    if (IS_FLASH_ADDRESS(u32Start) 
        && (u32Start + u32Len) <= (FLASH_BASE + FLASH_SIZE))
    {
        u32Map = RANGE_TO_BIT_MAP(FLASH_BASE, u32Start, u32Len, FLASH_ROL_BLOCK_BIT);
        pu32Dst = &FCU->RO;
        pu32ROL = &FCU->ROL;
    }
    else if (IS_OTP_ADDRESS(u32Start) 
             && (u32Start + u32Len) <= (OTP_BASE + OTP_SIZE))
    {
        u32Map = RANGE_TO_BIT_MAP(OTP_BASE, u32Start, u32Len, OTP_ROL_BLOCK_BIT);
        pu32Dst = &FCU->RO_INFO;
        pu32ROL = &FCU->ROL_INFO;
    }
    else if (IS_SYS_ADDRESS(u32Start) 
             && (u32Start + u32Len) <= (SYS_BASE + SYS_SIZE))
    {
        u32Map = RANGE_TO_BIT_MAP(SYS_BASE, u32Start, u32Len, SYS_ROL_BLOCK_BIT);
        pu32Dst = &FCU->RO_RED;
        pu32ROL = &FCU->ROL_RED;
    }
    else
    {
        return -1;
    }
    if (ENABLE == state)
    {
        u32Tmp = *pu32ROL ^ *pu32Dst;//locked but zero bits
        if (u32Tmp & u32Map)
        {   
            return -2;
        }
        *pu32Dst |= u32Map;
    }
    else
    {
        u32Tmp = *pu32ROL & *pu32Dst;//locked but one bits
        if (u32Tmp & u32Map)
        {
            return -3;
        }
        *pu32Dst &= ~u32Map;
    }
    return 0;
}

uint32_t FLASH_SetROL(uint32_t u32Start, uint32_t u32Len)
{
    volatile uint32_t u32Map, *pu32Dst;
    if (!(FCU->CS & FCU_CS_IDLE))
    {   
        return FLASH_BUSY;
    }
    if (IS_FLASH_ADDRESS(u32Start) 
        && (u32Start + u32Len) <= (FLASH_BASE + FLASH_SIZE))
    {
        u32Map = RANGE_TO_BIT_MAP(FLASH_BASE, u32Start, u32Len, FLASH_ROL_BLOCK_BIT);
        pu32Dst = &FCU->ROL;
    }
    else if (IS_OTP_ADDRESS(u32Start) 
             && (u32Start + u32Len) <= (OTP_BASE + OTP_SIZE))
    {
        u32Map = RANGE_TO_BIT_MAP(OTP_BASE, u32Start, u32Len, OTP_ROL_BLOCK_BIT);
        pu32Dst = &FCU->ROL_INFO;
    }
    else if (IS_SYS_ADDRESS(u32Start) 
             && (u32Start + u32Len) <= (SYS_BASE + SYS_SIZE))
    {
        u32Map = RANGE_TO_BIT_MAP(SYS_BASE, u32Start, u32Len, SYS_ROL_BLOCK_BIT);
        pu32Dst = &FCU->ROL_RED;
    }
    else
    {
        return -1;
    }
    *pu32Dst |= u32Map;
    return 0;
}

uint32_t FLASH_SetSRO(uint32_t u32Start, uint32_t u32Len, uint32_t u32Group, FunctionalState state)
{
    volatile uint32_t *pu32Dsts[] = {&FCU->RO0_512B, &FCU->RO1_512B};
    volatile uint32_t *pu32ROLs[] = {&FCU->RO0L_512B, &FCU->RO1L_512B};
    Intersection_TypeDef in;
    volatile uint32_t u32Map, u32Base, u32End, u32Tmp;
    
    assert_param(IS_FUNCTIONAL_STATE(state));
    
    if (!(FCU->CS & FCU_CS_IDLE))
    {   
        return FLASH_BUSY;
    }
    u32Tmp = (((u32Start - FLASH_BASE) >> FLASH_ROL_BLOCK_BIT) & 0x1F);
    if (0 != *pu32Dsts[u32Group] 
        || 0 != *pu32ROLs[u32Group])
    {
        if (u32Tmp != (FCU->RO_SEL >> (u32Group << 3)))
        {
            return -4;//select one sblock and use it ,can't change sel
        }
    }
    else
    {
        FCU->RO_SEL = (FCU->RO_SEL & ~(0x1F << (u32Group << 3))) | (u32Tmp << (u32Group << 3));//set sel
    }
    
    u32Base = (u32Start) & ~(FLASH_ROL_BLOCK_SIZE - 1);
    u32End = u32Base + FLASH_ROL_BLOCK_SIZE;//(u32Start + u32Len + FLASH_ROL_BLOCK_SIZE - 1) & ~(FLASH_ROL_BLOCK_SIZE - 1);
    if (IS_FLASH_ADDRESS(u32Start) 
        && (u32Start + u32Len) <= (FLASH_BASE + FLASH_SIZE)
        )//&& (u32Start + u32Len) <= (u32End))
    {
        in.u32IS1 = u32Base;
        in.u32IE1 = u32End;
        in.u32IS2 = u32Start;
        in.u32IE2 = u32Start + u32Len;
        if (FLASH_GetIntersection(&in))
        {
            u32Map = RANGE_TO_BIT_MAP(u32Base, in.u32OS, in.u32OE - in.u32OS, FLASH_SROL_BLOCK_BIT);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return -1;//out of range
    }
    
    if (ENABLE == state)
    {
        u32Tmp = *pu32ROLs[u32Group] ^ *pu32Dsts[u32Group];//locked but zero bits
        if (u32Tmp & u32Map)
        {   
            return -2;
        }
        *pu32Dsts[u32Group] |= u32Map;
    }
    else
    {
        u32Tmp = *pu32ROLs[u32Group] & *pu32Dsts[u32Group];//locked but one bits
        if (u32Tmp & u32Map)
        {
            return -3;
        }
        *pu32Dsts[u32Group] &= ~u32Map;
    }
    return 0;
}

uint32_t FLASH_SetSROL(uint32_t u32Start, uint32_t u32Len, uint32_t u32Group)
{
    volatile uint32_t *pu32Dsts[] = {&FCU->RO0_512B, &FCU->RO1_512B};
    volatile uint32_t *pu32ROLs[] = {&FCU->RO0L_512B, &FCU->RO1L_512B};
    Intersection_TypeDef in;
    volatile uint32_t u32Map, u32Base, u32End, u32Tmp;
    if (!(FCU->CS & FCU_CS_IDLE))
    {   
        return FLASH_BUSY;
    }
    u32Tmp = (((u32Start - FLASH_BASE) >> FLASH_ROL_BLOCK_BIT) & 0x1F);
    if (0 != *pu32Dsts[u32Group] 
        || 0 != *pu32ROLs[u32Group])
    {
        if (u32Tmp != (FCU->RO_SEL >> (u32Group << 3)))
        {
            return -4;//select one sblock and used ,can't change sel
        }
    }
    else
    {
        FCU->RO_SEL = (FCU->RO_SEL & ~(0x1F << (u32Group << 3))) | (u32Tmp << (u32Group << 3));//set sel
    }
    
    u32Base = (u32Start) & ~(FLASH_ROL_BLOCK_SIZE - 1);
    u32End = (u32Start + u32Len + FLASH_ROL_BLOCK_SIZE - 1) & ~(FLASH_ROL_BLOCK_SIZE - 1);
    if (IS_FLASH_ADDRESS(u32Start) 
        && (u32Start + u32Len) <= (FLASH_BASE + FLASH_SIZE)
        )//&& (u32Start + u32Len) <= u32End)
    {
        in.u32IS1 = u32Base;
        in.u32IE1 = u32End;
        in.u32IS2 = u32Start;
        in.u32IE2 = u32Start + u32Len;
        if (FLASH_GetIntersection(&in))
        {
            u32Map = RANGE_TO_BIT_MAP(u32Base, in.u32OS, in.u32OE - in.u32OS, FLASH_SROL_BLOCK_BIT);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }
    
    *pu32Dsts[u32Group] |= u32Map;
    return 0;
}

uint32_t FLASH_GetSROSelAddr(uint32_t u32Group)
{
    uint32_t u32Tmp;
    u32Tmp = (FCU->RO_SEL >> (u32Group << 3)) & 0x1F;
    u32Tmp = u32Tmp << FLASH_ROL_BLOCK_BIT;
    u32Tmp += FLASH_BASE;
    
    return u32Tmp;
}

/*
ret:
    bit0 : SROL[0] exist readonly block(s)
    bit1 : SROL[1] exist readonly block(s)
    bit2 : RO      exist readonly bolck(s)
    bit3 : RO_RED  exist readonly block(s)
    bit4 : RO_SYS  exist readonly block(s)
*/
uint32_t FLASH_AreaExistRO(uint32_t u32Addr, uint32_t u32Len)
{   
    uint32_t u32SROAddr, u32Tmp, u32Ret = 0;
    volatile uint32_t *pu32ROs[] = {&FCU->RO0_512B, &FCU->RO1_512B};
//    volatile uint32_t *pu32ROLs[] = {&FCU->RO0L_512B, &FCU->RO1L_512B};
    Intersection_TypeDef in;
    if (IS_FLASH_ADDRESS(u32Addr))
    {
        //SROL check
        u32SROAddr = FLASH_GetSROSelAddr(0);
        in.u32IS1 = u32SROAddr;
        in.u32IE1 = u32SROAddr + FLASH_SROL_BLOCK_SIZE;
        in.u32IS2 = u32Addr;
        in.u32IE2 = u32Addr + u32Len;
        if (FLASH_GetIntersection(&in))
        {
            u32Tmp = RANGE_TO_BIT_MAP(in.u32IS1, in.u32OS, in.u32OE - in.u32OS, FLASH_SROL_BLOCK_BIT);
            if (u32Tmp & *pu32ROs[0])
            {
                u32Ret |= 0x01;
            }
        }
        
        u32SROAddr = FLASH_GetSROSelAddr(1);
        in.u32IS1 = u32SROAddr;
        in.u32IE1 = u32SROAddr + FLASH_SROL_BLOCK_SIZE;
        in.u32IS2 = u32Addr;
        in.u32IE2 = u32Addr + u32Len;
        if (FLASH_GetIntersection(&in))
        {
            u32Tmp = RANGE_TO_BIT_MAP(in.u32IS1, in.u32OS, in.u32OE - in.u32OS, FLASH_SROL_BLOCK_BIT);
            if (u32Tmp & *pu32ROs[1])
            {
                u32Ret |= 0x02;
            }
        }
        //ROL check
        u32Tmp = RANGE_TO_BIT_MAP(FLASH_BASE, u32Addr, u32Len, FLASH_ROL_BLOCK_BIT);
        if (u32Tmp & FCU->RO)
        {
            u32Ret |= 0x04;
        }
    }
    else if (IS_SYS_ADDRESS(u32Addr))
    {
        u32Tmp = RANGE_TO_BIT_MAP(SYS_BASE, u32Addr, u32Len, SYS_ROL_BLOCK_BIT);
        if (u32Tmp & FCU->RO_RED)
        {
            u32Ret |= 0x10;
        }
    }
    else if (IS_OTP_ADDRESS(u32Addr))
    {
        u32Tmp = RANGE_TO_BIT_MAP(OTP_BASE, u32Addr, u32Len, OTP_ROL_BLOCK_BIT);
        if (u32Tmp & FCU->RO_INFO)
        {
            u32Ret |= 0x08;
        }
    }
    return u32Ret;
}

uint32_t FLASH_SetAllRO(uint32_t u32Start, uint32_t u32Len, FunctionalState state)
{
    uint32_t u32Ret;
    uint32_t u32SROStart, u32SROEnd;
    Intersection_TypeDef in;
    
    u32Ret = FLASH_SetRO(u32Start, u32Len, state);
    if (u32Ret)
    {
        return u32Ret;
    }
    u32SROStart = FLASH_GetSROSelAddr(0);
    u32SROEnd = u32SROStart + FLASH_ROL_BLOCK_SIZE;
    in.u32IS1 = u32SROStart;
    in.u32IE1 = u32SROEnd;
    in.u32IS2 = u32Start;
    in.u32IE2 = u32Start + u32Len;
    if (FLASH_GetIntersection(&in))
    {
        u32Ret = FLASH_SetSRO(in.u32OS, in.u32OE - in.u32OS, 0, state);
        if (u32Ret)
        {
            return u32Ret;
        }
    }
    
    u32SROStart = FLASH_GetSROSelAddr(1);
    u32SROEnd = u32SROStart + FLASH_ROL_BLOCK_SIZE;
    in.u32IS1 = u32SROStart;
    in.u32IE1 = u32SROEnd;
    in.u32IS2 = u32Start;
    in.u32IE2 = u32Start + u32Len;
    if (FLASH_GetIntersection(&in))
    {
        u32Ret = FLASH_SetSRO(in.u32OS, in.u32OE - in.u32OS, 1, state);
        if (u32Ret)
        {
            return u32Ret;
        }
    }
    return u32Ret;
}


FlagStatus FLASH_StatusIsSet(uint32_t u32Flag)
{
    return ((FCU->CS & u32Flag) != 0);
}


uint32_t FLASH_ProgramBuf(uint32_t u32Addr, void *pvBuf, uint32_t u32Len, uint8_t u8Mode)
{
    uint32_t u32LenTmp, u32Tmp, u32Ret, u32HalfPageAddr;
    uint8_t *pu8Tmp = pvBuf;
//    if (!(FCU->CS & FCU_CS_IDLE))
//    {   
//        return FLASH_BUSY;
//    }
    
    if (FLASH_AreaExistRO(u32Addr, u32Len))
    {
        return FLASH_ERROR_WRP;//the program area exist readonly block(s)
    }
    if (u8Mode == FLASH_PROG_MODE_SMART1 || u8Mode == FLASH_PROG_MODE_SMART2)
    {
        FLASH_SetSmartProv(FLASH_SMART_PROVIDER_MODULE);
    }
    FLASH_OprateSel(u8Mode);
    
    u32LenTmp = u32Len;
    
    while(0 != u32LenTmp
            && u32LenTmp <= u32Len)
    {
        FCU->ADDR = u32Addr;
        u32HalfPageAddr = (FCU->ADDR + 0x100) & ~(0x100 - 1);
        //flash program can not span half a page
        u32Tmp = u32LenTmp < (u32HalfPageAddr - FCU->ADDR) ?
                    u32LenTmp : (u32HalfPageAddr - FCU->ADDR);
        FLASH_ProgWordSize(((u32Tmp + 0x3) >> 2));
        FLASH_CopyBufToPgBuf(pu8Tmp, u32Tmp);
        u32Ret = FLASH_Operate();
        if (u32Ret)
        {
            return u32Ret;
        }
        u32LenTmp -= u32Tmp;
        pu8Tmp += u32Tmp;
        u32Addr += u32Tmp;
    }
    return FLASH_COMPLETE;
}

uint32_t FLASH_ProgramWord(uint32_t u32Addr, uint32_t u32Dat, uint8_t u8Mode)
{
    uint32_t u32Ret, u32Len;
    
    u32Len = sizeof(u32Dat);
//    if (!(FCU->CS & FCU_CS_IDLE))
//    {   
//        return FLASH_BUSY;
//    }
    
    if (FLASH_AreaExistRO(u32Addr, u32Len))
    {
        return FLASH_ERROR_WRP;//the program area exist readonly block(s)
    }
    if (u8Mode == FLASH_PROG_MODE_SMART1 || u8Mode == FLASH_PROG_MODE_SMART2)
    {
        FLASH_SetSmartProv(FLASH_SMART_PROVIDER_MODULE);
    }
    FCU->ADDR = u32Addr;
    FLASH_OprateSel(u8Mode);
    FLASH_ProgWordSize(u32Len >> 2);
    
    FCU->PG_BUF = u32Dat;
    u32Ret = FLASH_Operate();
    if (u32Ret)
    {
        return u32Ret;
    }
    return FLASH_COMPLETE;
}

uint32_t FLASH_ErasePage(uint32_t u32Addr, uint8_t u8Mode)
{
    uint32_t u32Start = (u32Addr) & (~(FLASH_PAGE_SIZE - 1));//(u32Addr + (FLASH_PAGE_SIZE - 1)) & (~(FLASH_PAGE_SIZE - 1));
    uint32_t u32Ret;
//    if (!(FCU->CS & FCU_CS_IDLE))
//    {   
//        return FLASH_BUSY;
//    }
    
    if (FLASH_AreaExistRO(u32Start, FLASH_PAGE_SIZE))
    {   
        return FLASH_ERROR_WRP;
    }
    if (FLASH_ERASE_MODE_PAGE_SMART == u8Mode)
    {
        FLASH_SetSmartProv(FLASH_SMART_PROVIDER_MODULE);
    }
    FCU->ADDR = u32Addr;
    FLASH_OprateSel(u8Mode);
    u32Ret = FLASH_Operate();
    if (u32Ret)
    {
        return u32Ret;
    }
    return FLASH_COMPLETE;
    
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

