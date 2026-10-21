
/* Includes ------------------------------------------------------------------*/
#include "misc.h"

#define AIRCR_VECTKEY_MASK    ((uint32_t)0x05FA0000)


/**
  * @brief  Initializes the NVIC peripheral according to the specified
  *   parameters in the NVIC_InitStruct.
  * @param  NVIC_InitStruct: pointer to a NVIC_InitTypeDef structure that contains
  *   the configuration information for the specified NVIC peripheral.
  * @retval None
  */
void NVIC_Init(NVIC_InitTypeDef* NVIC_InitStruct)
{
  uint32_t tmpPriority = 0x00;
  
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NVIC_InitStruct->NVIC_IRQChannelCmd));
  assert_param(IS_NVIC_PRIORITY(NVIC_InitStruct->NVIC_IRQChannelPriority));  

  if (NVIC_InitStruct->NVIC_IRQChannelCmd != DISABLE)
  {
    /* Compute the Corresponding IRQ Priority --------------------------------*/    
	tmpPriority = NVIC->IP[NVIC_InitStruct->NVIC_IRQChannel >> 0x02];
	tmpPriority &= (uint32_t)(~(((uint32_t)0xFF) << ((NVIC_InitStruct->NVIC_IRQChannel & 0x03) * 8))); 
	tmpPriority |= (uint32_t)((((uint32_t)NVIC_InitStruct->NVIC_IRQChannelPriority << 6) & 0xFF) << \
	               ((NVIC_InitStruct->NVIC_IRQChannel & 0x03) * 8));
    NVIC->IP[NVIC_InitStruct->NVIC_IRQChannel >> 0x02] = tmpPriority;
    /* Enable the Selected IRQ Channels --------------------------------------*/
	NVIC_EnableIRQ((IRQn_Type)NVIC_InitStruct->NVIC_IRQChannel);    
  }
  else
  {
    tmpPriority = NVIC->IP[NVIC_InitStruct->NVIC_IRQChannel >> 0x02];
	tmpPriority &= (uint32_t)(~(((uint32_t)0xFF) << ((NVIC_InitStruct->NVIC_IRQChannel & 0x03) * 8))); 
	tmpPriority |= (uint32_t)((((uint32_t)NVIC_InitStruct->NVIC_IRQChannelPriority << 6) & 0xFF) << \
	               ((NVIC_InitStruct->NVIC_IRQChannel & 0x03) * 8));
    NVIC->IP[NVIC_InitStruct->NVIC_IRQChannel >> 0x02] = tmpPriority;
    /* Disable the Selected IRQ Channels -------------------------------------*/
    NVIC_DisableIRQ((IRQn_Type)NVIC_InitStruct->NVIC_IRQChannel);
  }
}

/**
  * @brief  Sets the vector table location and Offset.
  * @param  NVIC_VectTab: specifies if the vector table is in RAM or FLASH memory.
  *   This parameter can be one of the following values:
  *     @arg NVIC_VectTab_RAM
  *     @arg NVIC_VectTab_FLASH
  * @param  Offset: Vector Table base offset field. This value must be a multiple of 0x100.
  * @retval None
  */
void NVIC_SetVectorTable(uint32_t NVIC_VectTab, uint32_t Offset)
{ 
  /* Check the parameters */
  assert_param(IS_NVIC_VECTTAB(NVIC_VectTab));
  assert_param(IS_NVIC_OFFSET(Offset));  
   
//  SCB->VTOR = NVIC_VectTab | (Offset & (uint32_t)0x1FFFFF80);
}


