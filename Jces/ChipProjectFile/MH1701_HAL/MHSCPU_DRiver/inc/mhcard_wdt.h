
#ifndef __MHSCPU_WDT_H
#define __MHSCPU_WDT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhcard.h"
#include "mhcard_conf.h"

typedef struct
{
    __IO uint32_t WDT_CR;
    __IO uint32_t RESERVED0;
    __I  uint32_t WDT_CCVR;
    __O  uint32_t WDT_CRR;
    __I  uint32_t WDT_STAT;
    __I  uint32_t WDT_EOI;  
    __I  uint32_t RESERVED1;
    __IO uint32_t WDT_RLD;
    __I  uint32_t RESERVED[53];
    __I  uint32_t WDT_COMP_PARAMS_1;
    __I  uint32_t WDT_COMP_VERSION;
    __I  uint32_t WDT_COMP_TYPE;
} WDT_TypeDef;

#define WDG_BASE                            (MHCARD_APB_BASE + 0x1000)
#define WDT                                 ((WDT_TypeDef *)WDG_BASE)     
     
typedef enum
{
	WDT_Mode_CPUReset = 0,
	WDT_Mode_Interrupt = 1
}WDT_ModeTypeDef;
	 
void WDT_SetReload(uint32_t Reload);
void WDT_ReloadCounter(void);
void WDT_Enable(void);
void WDT_ModeConfig(WDT_ModeTypeDef WDT_Mode);
	 
ITStatus WDT_GetITStatus(void);
void WDT_ClearITPendingBit(void);

#ifdef __cplusplus
}
#endif

#endif
