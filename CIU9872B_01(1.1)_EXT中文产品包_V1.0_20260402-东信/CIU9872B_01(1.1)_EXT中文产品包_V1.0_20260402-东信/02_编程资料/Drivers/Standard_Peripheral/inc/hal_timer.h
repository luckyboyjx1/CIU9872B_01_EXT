/************************************************************************************************/
/**
* @file               hal_timer.h
* @author             Firmware Team
* @brief              TIMER HAL驱动模块相关定义与声明
* @version            V1.0
* @date               2024-01-18
*
**************************************************************************************************
* @attention
*
* <h2><center>CEC Huada Electronic Design Co.,Ltd</center></h2>
*
* The copyright of this manual is reserved by CEC Huada Electronic Design Co., Ltd.
* Without prior permission, any duplication, printing or publication and distribution
* of the manual will constitute a violation of copyright towards CEC Huada Electronic
* Design Co., Ltd. CEC Huada Electronic Design Co., Ltd. reserves the right to bring
* them to court. \n
* The right to change the manual without notice to the user is reserved by CEC Huada
* Electronic Design Co., Ltd. Though we have checked content of this manual, errors
* are unavoidable. So, we will regularly check the content and make necessary
* modifications in the next version. We suggest you obtain the latest version of this
* manual from CEC Huada Electronic Design Co., Ltd. before making final design.
*
* @par 修改日志
* <table>
* <tr><th>Date        <th>Version     <th>Author          <th>Description     </tr>
* <tr><td>2024/01/18  <td>V1.0        <td>Firmware Team   <td>首次发布        </tr>
* </table>
*
**************************************************************************************************
*/

#ifndef __HAL_TIMER_H_
#define __HAL_TIMER_H_

/*------------------------------------------includes-----------------------------------------------*/
#include "CIU9872B_01_V1.1.h"
#include "hal_ckmu.h"

/************************************************************************************************/
/**@addtogroup Standard_Peripheral
* @{
*/

/** @addtogroup TIMER
  * @{
  */

/** @defgroup TIMER_Export_Types TIMER Export Types
  * @{
  */

/* Exported Macro -------------------------------------------------------------*/
/********************  Bits definition for TimerXLoad register  *****************/
#define TimerLoad_Pos                   (0)
#define TimerLoad_Mak                   (0xFFFFFFFF << TimerLoad_Pos)
#define TimerLoad                       TimerLoad_Mak

/********************  Bits definition for TimerXValue register  *****************/
#define TimerValue_Pos                  (0)
#define TimerValue_Mak                  (0xFFFFFFFF << TimerValue_Pos)
#define TimerValue                      TimerValue_Mak

/********************  Bits definition for TimerXControl register  *****************/
#define TimerEnable_Pos                 (7)
#define TimerEnable_Mak                 (0x01 << TimerEnable_Pos)
#define TimerEnable                     TimerEnable_Mak
#define TimerMode_Pos                   (6)
#define TimerMode_Mak                   (0x01 << TimerMode_Pos)
#define TimerMode                       TimerMode_Mak
#define ExtPluseCnt_Pos                 (4)
#define ExtPluseCnt_Mak                 (0x03 << ExtPluseCnt_Pos)
#define ExtPluseCnt                     ExtPluseCnt_Mak
#define TimerPre_Pos                    (2)
#define TimerPre_Mak                    (0x03 << TimerPre_Pos)
#define TimerPre                        TimerPre_Mak
#define TimerSize_Pos                   (1)
#define TimerSize_Mak                   (0x01 << TimerSize_Pos)
#define TimerSize                       TimerSize_Mak
#define OneShotCount_Pos                (0)
#define OneShotCount_Mak                (0x01 << OneShotCount_Pos)
#define OneShotCount                    OneShotCount_Mak

/********************  Bits definition for TimerXIS register  *****************/
#define TimerXIS_Pos                    (0)
#define TimerXIS_Mak                    (0xFFFFFFFF << TimerXIS_Pos)
#define TimerXIS                        TimerXIS_Mak


/**
  * @brief  Timer选择定义
  */
typedef enum
{
    USE_TIMER0      = 0x00U,            /*!< Timer 0            */
    USE_TIMER1      = 0x01U,            /*!< Timer 1            */
    USE_TIMER2      = 0x02U,            /*!< Timer 2            */
} Timer_UseTypeDef;

/**
  * @brief  Timer计时时钟源选择
  */
typedef enum
{
    TIM_CLK_PCLK        = 0x00,
    TIM_CLK_HOSC        = 0x01,         /*!< 接触供电推荐       */
    TIM_CLK_EXTCLK      = 0x02          /*!< 非接供电推荐       */
} Timer_ClkSelectTypeDef;

/**
  * @brief  watchdog模式控制
  */
typedef enum
{
    USE_INT      = 0x00U,            /*!< watchdog interrupt    */
    USE_RST      = 0x01U,            /*!< watchdog reset        */
} WatchDog_CtlModeDef;

/**
  * @brief  时钟控制TIM2定义
  */
typedef enum
{
    SYSCLKTIM2_EN = 31,
    SYSCLKTIM2_VALUE = 0,
} SYSCLKTIM2_BITS;

/**
  * @}
  */



/* Exported Macro ------------------------------------------------------------------------------*/
/** @defgroup SPI_Exported_Macros TIMER Exported Macros
  * @{
  */
/** @brief  Open TIMER clock.
  * @retval None
  */
#define __HAL_OPEN_TIMER0_CLK()                         SET_REG_BIT(CKMU->SYSAPBCLKEN, TIMER0PCLKEN)
#define __HAL_OPEN_TIMER1_CLK()                         SET_REG_BIT(CKMU->SYSAPBCLKEN, TIMER1PCLKEN)
#define __HAL_OPEN_TIMER2_CLK()                         SET_REG_BIT(CKMU->SYSAPBCLKEN, TIMER2PCLKEN)


/** @brief  Close TIMER clock.
  * @retval None
  */
#define __HAL_CLOSE_TIMER0_CLK()                        SET_REG_BIT(CKMU->SYSAPBCLKCLR, TIMER0PCLKOFF)
#define __HAL_CLOSE_TIMER1_CLK()                        SET_REG_BIT(CKMU->SYSAPBCLKCLR, TIMER1PCLKOFF)
#define __HAL_CLOSE_TIMER2_CLK()                        SET_REG_BIT(CKMU->SYSAPBCLKCLR, TIMER2PCLKOFF)


/** @brief  Open WDT clock.
  * @retval None
  */
#define __HAL_OPEN_WDT_CLK()                            SET_REG_BIT(CKMU->SYSAPBCLKEN, WDTEN)

/** @brief  Enable WDT RESET.
  * @retval None
  */
#define __HAL_ENABLE_WDT_RESET()                        SET_REG_BIT(WDT->WDTControl, USE_RST)

/** @brief  Disable WDT RESET.
  * @retval None
  */
#define __HAL_DISABLE_WDT_RESET()                       CLEAR_REG_BIT(WDT->WDTControl,USE_RST)


/** @brief  Open WDT interrupt.
  * @retval None
  */
#define __HAL_ENABLE_WDT_INT()                          SET_REG_BIT(WDT->WDTControl, USE_INT)

//置位
#define SETSFR_BIT(x, y)                                (x |= (1ul << y))
//清零
#define CLRSFR_BIT(x, y)                                (x &= (~(1ul << y)))
//检测
#define CHECKSFR_BIT(x, y)                              (x & (1ul << y))

/**
  * @}
  */


/** @addtogroup TIMER_Export_Function TIMER Export Function
  * @{
  */
/* Exported functions --------------------------------------------------------*/
/**
  * @brief      启动SYSTICK
  * @param[in]  uxDelay_ms 延时时间，单位：毫秒.
  *
  * @return     HAL status
  */
HAL_StatusTypeDef HAL_SysTick_Start(uint32_t uxDelay_ms);

/**
  * @brief      停止SYSTICK
  *
  * @return     HAL status
  */
HAL_StatusTypeDef HAL_SysTick_Stop(void);

/**
  * @brief      启动timer
  * @param[in]  uxTimer 枚举Timer_UseTypeDef.
  * @param[in]  uxClkSelect timer时钟源，枚举Timer_ClkSelectTypeDef.
  * @param[in]  uxDelay_ms 延时时间，单位：毫秒.
  *
  * @return     HAL status
  */
HAL_StatusTypeDef HAL_Timer_Start(Timer_UseTypeDef uxTimer, Timer_ClkSelectTypeDef uxClkSelect, uint32_t uxDelay_ms);

/**
  * @brief      停止timer
  * @param[in]  uxTimer 枚举Timer_UseTypeDef.
  *
  * @return     HAL status
  */
HAL_StatusTypeDef HAL_Timer_Stop(Timer_UseTypeDef uxTimer);

/**
  * @brief      获取timer状态，运行或停止
  * @param[in]  uxTimer 枚举Timer_UseTypeDef.
  *
  * @return     HAL status
  */
HAL_StatusTypeDef HAL_GetTimer_Status(Timer_UseTypeDef uxTimer);

/**
  * @brief      复位timer
  * @param[in]  uxTimer 枚举Timer_UseTypeDef.
  * @param[in]  uxClkSelect timer时钟源，枚举Timer_ClkSelectTypeDef.
  * @param[in]  uxDelay_ms 延时时间，单位：毫秒.
  *
  * @return     HAL status
  */
HAL_StatusTypeDef HAL_Timer_Reset(Timer_UseTypeDef uxTimer, Timer_ClkSelectTypeDef uxClkSelect, uint32_t uxDelay_ms);

/**
  * @brief      接触供电模式启动timer
  * @param[in]  uxTimer 枚举Timer_UseTypeDef.
  * @param[in]  uxDelay_ms 延时时间，单位：毫秒.
  *
  * @return     HAL status
  */
#define CTMODE_Timer_Start(uxTimer, uxDelay_ms) HAL_Timer_Start(uxTimer, TIM_CLK_HOSC, uxDelay_ms)
/**
  * @brief      停止timer
  * @param[in]  uxTimer 枚举Timer_UseTypeDef.
  *
  * @return     HAL status
  */
#define CTMODE_Timer_Stop(uxTimer)  HAL_Timer_Stop(uxTimer)
/**
  * @brief      非接供电模式启动timer
  * @param[in]  uxTimer 枚举Timer_UseTypeDef.
  * @param[in]  uxDelay_ms 延时时间，单位：毫秒.
  *
  * @return     HAL status
  */
#define RFMODE_Timer_Start(uxTimer, uxDelay_ms) HAL_Timer_Start(uxTimer, TIM_CLK_EXTCLK, uxDelay_ms)
/**
  * @brief      停止timer
  * @param[in]  uxTimer 枚举Timer_UseTypeDef.
  *
  * @return     HAL status
  */
#define RFMODE_Timer_Stop(uxTimer)  HAL_Timer_Stop(uxTimer)

/**
  * @brief      系统时钟100微秒及以下时间延时
  * @param[in]  ucCountVal 计数周期个数，最大取值255，计时总时间：~ucCountVal * 0.4us,最大102us左右.
  *
  * @return     None
  */
void HAL_SysDelay_us(uint8_t ucCountVal);

/**
  * @brief      系统Tick毫秒级延时
  * @param[in]  uxDelay_ms 延时时间
  *
  * @return     HAL status
  */
HAL_StatusTypeDef  HAL_SysTickDelay_ms(uint32_t uxDelay_ms);
/**
  * @}
  */

/**
  * @}
  */


/**
  * @}
  */

/**
  * @}
  */


#endif /* __HAL_TIMER_H_ */

