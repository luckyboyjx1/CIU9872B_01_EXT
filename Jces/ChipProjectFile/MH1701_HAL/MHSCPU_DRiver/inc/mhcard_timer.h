#ifndef __MHSCARD_TIMER_H
#define __MHSCARD_TIMER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhcard.h"
#include "mhcard_conf.h"

typedef struct
{
    __IO uint32_t LoadCount;
    __I  uint32_t CurrentValue;
    __IO uint32_t ControlReg;
    __IO  uint32_t EOI;
    __I  uint32_t IntStatus;
} TIM_TypeDef;

typedef struct
{
    #define TIM_NUM 3  //2   WCQ 
    TIM_TypeDef TIM[TIM_NUM];
    __I uint8_t RESERVED[0xA0 - TIM_NUM * sizeof(TIM_TypeDef)];
    __I  uint32_t TIM_IntStatus;
    __I  uint32_t TIM_EOI;
    __I  uint32_t TIM_RawIntStatus;
    __I  uint32_t TIM_Comp;
    __IO uint32_t TIM_ReloadCount[TIM_NUM];
} TIM_Module_TypeDef;

#define TIMM0_BASE                          (MHCARD_APB_BASE + 0x2000)
#define TIMM0                               ((TIM_Module_TypeDef *)TIMM0_BASE)     
     
typedef enum
{
	TIM_0		= 0,
	TIM_1		= 1,
    TIM_2		= 2
}TIM_NumTypeDef;

typedef struct 
{
	TIM_NumTypeDef TIMx;
	uint32_t TIM_Period;			/*!< Specifies the period value to be loaded into the active
									   Auto-Reload Register at the next update event.
									   This parameter must be a number between 0x0000 and 0xFFFFFFFF.  */ 
}TIM_InitTypeDef;

typedef struct 
{
	TIM_NumTypeDef TIMx;
	uint32_t TIM_LowLevelPeriod;
	uint32_t TIM_HighLevelPeriod;
}TIM_PWMInitTypeDef;

typedef enum
{
	TIM_Mode_General	= 0,
	TIM_Mode_PWM		= 1
}TIM_ModeTypeDef;
#define IS_TIM_MODE(MODE)							(MODE == TIM_Mode_General || MODE == TIM_Mode_PWM)


void TIM_DeInit(TIM_Module_TypeDef * TIMMx);
void TIM_Init(TIM_Module_TypeDef* TIMMx, TIM_InitTypeDef* TIM_InitStruct);
void TIM_PWMInit(TIM_Module_TypeDef* TIMMx, TIM_PWMInitTypeDef* TIM_PWMInitStruct);
void TIM_Cmd(TIM_Module_TypeDef* TIMMx, TIM_NumTypeDef TIMx, FunctionalState NewState);
void TIM_ModeConfig(TIM_Module_TypeDef* TIMMx, TIM_NumTypeDef TIMx, TIM_ModeTypeDef TIM_Mode);
void TIM_SetPeriod(TIM_Module_TypeDef* TIMMx, TIM_NumTypeDef TIMx, uint32_t Period);
void TIM_SetPWMPeriod(TIM_Module_TypeDef* TIMMx, TIM_NumTypeDef TIMx, uint32_t PWM_LowLevelPeriod, uint32_t PWM_HighLevelPeriod);
void TIM_ITConfig(TIM_Module_TypeDef* TIMMx, TIM_NumTypeDef TIMx, FunctionalState NewState);
void TIM_ClearITPendingBit(TIM_Module_TypeDef* TIMMx, TIM_NumTypeDef TIMx);
ITStatus TIM_GetITStatus(TIM_Module_TypeDef* TIMMx, TIM_NumTypeDef TIMx);
uint32_t TIM_GetAllITStatus(TIM_Module_TypeDef* TIMMx);
ITStatus TIM_GetRawITStatus(TIM_Module_TypeDef* TIMMx, TIM_NumTypeDef TIMx);
uint32_t TIM_GetAllRawITStatus(TIM_Module_TypeDef* TIMMx);

#ifdef __cplusplus
}
#endif

#endif

