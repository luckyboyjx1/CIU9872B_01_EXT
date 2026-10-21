/************************ (C) COPYRIGHT *************************
 * @file                : mhscpu_crc.c
 * @author              : 
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the CRC firmware functions
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "mhcard_crc.h"
#include "mhcard_sysctrl.h"
#define CRC_USING_INIT_VALUE         (1U)
#define CRC_NOT_USING_INIT_VALUE     (0U)

/* Exported functions -------------------------------------------------------*/
/**
  * @brief  Deinitializes the CRC peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void CRC_DeInit(void)
{
    SYSCTRL_ClockEnable(SYSCTRL_Periph_CRC, ENABLE); 
    SYSCTRL_SoftReset(SYSCTRL_CRC_RESET);
}

static void CRC_ConfigInitValue(uint32_t opt)
{
    uint32_t csr_tmp = 0xC0;
    uint32_t tmp_value;
    switch(opt)
    {
        case CRC_16:
            csr_tmp = ((((csr_tmp&POLY_SEL_RESET)&TYPE_SEL_RESET)|REV_IN_SEL_SET)|REV_OUT_SEL_SET)&XOR_OUT_SEL_RESET;
            tmp_value = CRC_INIT_VALUE_0;
            break;
        case CRC_16_CCITT:
            csr_tmp = ((((csr_tmp|POLY_SEL_SET)&TYPE_SEL_RESET)|REV_IN_SEL_SET)|REV_OUT_SEL_SET)&XOR_OUT_SEL_RESET;
            tmp_value = CRC_INIT_VALUE_0;
            break;
        case CRC_16_Modbus:
            csr_tmp = ((((csr_tmp&POLY_SEL_RESET)&TYPE_SEL_RESET)|REV_IN_SEL_SET)|REV_OUT_SEL_SET)&XOR_OUT_SEL_RESET;
            tmp_value = CRC16_INIT_VALUE_FF;
            break;
        case CRC_CCITT_0xffff:
            csr_tmp = ((((csr_tmp|POLY_SEL_SET)&TYPE_SEL_RESET)&REV_IN_SEL_RESET)&REV_OUT_SEL_RESET)&XOR_OUT_SEL_RESET;
            tmp_value = CRC16_INIT_VALUE_FF;
            break;
        case CRC_CCITT_XModem:
            csr_tmp = ((((csr_tmp|POLY_SEL_SET)&TYPE_SEL_RESET)&REV_IN_SEL_RESET)&REV_OUT_SEL_RESET)&XOR_OUT_SEL_RESET;
            tmp_value = CRC_INIT_VALUE_0;
            break;
        case CRC_32:
            csr_tmp = ((((csr_tmp&POLY_SEL_RESET)|TYPE_SEL_SET)|REV_IN_SEL_SET)|REV_OUT_SEL_SET)|XOR_OUT_SEL_SET;
            tmp_value = CRC32_INIT_VALUE_FF;
            break;
        case CRC_32_FF:
            csr_tmp = ((((csr_tmp&POLY_SEL_RESET)|TYPE_SEL_SET)&REV_IN_SEL_RESET)&REV_OUT_SEL_RESET)&XOR_OUT_SEL_RESET;
            tmp_value = CRC32_INIT_VALUE_FF;
            break;
        case CRC_16_CUSTOM:
            csr_tmp = ((((csr_tmp|POLY_SEL_SET)&TYPE_SEL_RESET)|REV_IN_SEL_SET)&REV_OUT_SEL_RESET)&XOR_OUT_SEL_RESET;
            tmp_value = CRC_INIT_VALUE_0;
            break;
    }
    
    CRC->CRC_CSR = csr_tmp;
    CRC->CRC_INI = tmp_value;
}

static void CRC_CalcAlign(void *PvAddr, uint32_t Size)
{
    uint8_t *pu8Addr;
    uint32_t u32Tmp, *pu32Align;

    pu32Align = (uint32_t *)ALGIN4_UP(PvAddr);
    pu8Addr = (uint8_t *)PvAddr;
    u32Tmp = (uint32_t)PvAddr + Size;
    
    if (pu8Addr < (uint8_t *)pu32Align)
    {
        CRC->CRC_CSR &= ~CRC_CSR_BYTE_NUM_Mask;//(3 << 6)
        
        for (pu8Addr = (uint8_t *)PvAddr; (uint32_t)pu8Addr < (uint32_t)pu32Align && (uint32_t)pu8Addr < u32Tmp; pu8Addr++)
        {
            CRC_IN(*pu8Addr);
        }
        CRC_WAIT();
        CRC->CRC_CSR |= CRC_CSR_BYTE_NUM_Mask;//(3 << 6)
    }
    else
    {
        CRC->CRC_CSR |= CRC_CSR_BYTE_NUM_Mask;//(3 << 6)
    }
    
    while (pu32Align < (uint32_t *)ALGIN4_DOWN(u32Tmp))
    {
        CRC_IN(*pu32Align++);
    }
    if ((uint32_t)pu32Align < u32Tmp)
    {
        CRC_WAIT();
        
        CRC->CRC_CSR &= ~CRC_CSR_BYTE_NUM_Mask;//(3 << 6)
        
        for (pu8Addr = (uint8_t *)pu32Align; pu8Addr < (uint8_t *)u32Tmp; pu8Addr++)
        {
            CRC_IN(*pu8Addr);
        }
    }
    CRC_WAIT();
    
}

/**
* @brief  CRC_CalcBlockCRC : Computes CRC of a given buffer of data byte
* @param  Opt: CRC type
          InitVal: Initial value with CRC
	      PvAddr: Data Pointer 
		  Size: Data len
* @retval CRC Value
*/
uint32_t CRC_CalcBlockCRC(uint32_t Opt, void *PvAddr, uint32_t Size)
{  
    assert_param(IS_CRC_OPT(Opt));
    
    CRC_ConfigInitValue(Opt);

    CRC_CalcAlign(PvAddr, Size);
    
    return CRC_OUT();
}


/**
* @brief  CRC_Calc: Computes CRC of a given buffer of data byte
* @param  CRC_Config: CRC config param
* @param  PvAddr: Data pointer
* @param  Len: Data len
* @retval CRC Value
*/
uint32_t CRC_Calc(CRC_ConfigTypeDef *CRC_Config, uint8_t *PvAddr, uint32_t Len)
{
	uint32_t crcVal;
    
	CRC->CRC_CSR = 0xC0;
	
	switch ((uint32_t)(CRC_Config->CRC_Poly))
	{
		case (uint32_t)CRC_Poly_16_1021:
			CRC->CRC_CSR |= POLY_SEL_SET;
			break;
		case (uint32_t)CRC_Poly_16_8005:
			CRC->CRC_CSR &= ~(POLY_SEL_SET);
			break;
		case (uint32_t)CRC_Poly_32_04C11DB7:
			CRC->CRC_CSR |= TYPE_SEL_SET;
			break;
	}
	
	if (CRC_PolyMode_Reversed == CRC_Config->CRC_PolyMode)
	{
		CRC->CRC_CSR |= REV_OUT_SEL_SET | REV_IN_SEL_SET;
	}
	else
	{
		CRC->CRC_CSR &= ~(REV_OUT_SEL_SET | REV_IN_SEL_SET);
	}
	
	if (CRC_Poly_16_1021 == CRC_Config->CRC_Poly || CRC_Poly_16_8005 == CRC_Config->CRC_Poly)
	{
		CRC->CRC_INI = (uint16_t)CRC_Config->CRC_Init_Value;
	}
	else if(CRC_Poly_32_04C11DB7 == CRC_Config->CRC_Poly)
	{
		CRC->CRC_INI = (uint32_t)CRC_Config->CRC_Init_Value;
	}
	
    CRC_CalcAlign(PvAddr, Len);
    
    crcVal = CRC_OUT();
	
	if (CRC_Poly_16_1021 == CRC_Config->CRC_Poly || CRC_Poly_16_8005 == CRC_Config->CRC_Poly)
	{
		crcVal ^= (uint16_t)CRC_Config->CRC_Xor_Value;
		crcVal &= CRC16_INIT_VALUE_FF;
	}
	else if (CRC_Poly_32_04C11DB7 == CRC_Config->CRC_Poly)
	{
		crcVal ^= (uint32_t)CRC_Config->CRC_Xor_Value;
	}
	
	return crcVal;	
}


/**
        
        CRC_CalcStart + CRC_CalcUpdate + CRC_CalcUpdate + ... + CRC_CalcStop
*/
/**
* @brief  CRC_CalcStart : Init CRC configure with CRC type
* @param  Opt: CRC type
          InitVal: Initial value with CRC
	      PvAddr: Data Pointer 
		  Size: Data len
* @retval None
*/
void CRC_CalcStart(uint32_t Opt)
{
    assert_param(IS_CRC_OPT(Opt));
    
    CRC_ConfigInitValue(Opt);
}

/**
* @brief  CRC_CalcUpdate : Computes CRC of a given buffer of data byte
* @param  PvAddr: Data Pointer 
		  Size: Data len
* @retval None
*/
void CRC_CalcUpdate(void *PvAddr, uint32_t Size)
{
    CRC_CalcAlign(PvAddr, Size);
}

/**
* @brief  CRC_CalcStop : Get CRC result
* @param  None
* @retval CRC Value
*/
uint32_t CRC_CalcStop(void)
{
    return CRC_OUT();
}

/**************************      (C) COPYRIGHT    *****END OF FILE****/
