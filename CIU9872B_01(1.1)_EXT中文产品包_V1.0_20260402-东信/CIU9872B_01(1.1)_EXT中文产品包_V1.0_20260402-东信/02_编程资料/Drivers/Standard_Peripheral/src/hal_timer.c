/************************************************************************************************/
/**
* @file               hal_timer.c
* @author             Firmware Team
* @brief              设置timer。
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
/*------------------------------------------includes-----------------------------------------------*/
#include "hal_timer.h"

/************************************************************************************************/
/**@addtogroup Standard_Peripheral
* @{
*/

/** @addtogroup TIMER
  * @{
  */
/* Exported Macro -------------------------------------------------------------*/
#define CTMODE_TIMER_LOAD_VALUE(val)    (val*344)
#define RFMODE_TIMER_LOAD_VALUE(val)    (val*212)

#define TIMER_IRQ_EN(NUM)               TMR##NUM##_IRQEN
#define TIMER_IRQ_DIS(NUM)              TMR##NUM##_IRQDIS
#define TIMER_PEND_CLR(NUM)             TMR##NUM##_PENDCLR

#ifdef KIT_DEBUG
#define SYSTICK_REFERENCE_CLOCK_PSMODE_CT         688
#define SYSTICK_REFERENCE_CLOCK_PSMODE_RF         212
#else
#define SYSTICK_REFERENCE_CLOCK_PSMODE_CT         344	///< 88/256
#define SYSTICK_REFERENCE_CLOCK_PSMODE_RF         212	///< 13.56/64
#endif

typedef struct
{
    uint32_t TIMERXLOAD;
    uint32_t TIMERXVALUE;
    uint32_t TIMERXCONTROL;
    uint32_t TIMERXIS;
} TIMERX_TypeDef;

/** @defgroup TIMER_Export_Function TIMER Export Function
  * @{
  */

/* Exported functions --------------------------------------------------------*/

/*!< 启动systick,100M时钟时最大计时160多ms，LOAD寄存器仅有24位有效 */
HAL_StatusTypeDef HAL_SysTick_Start(uint32_t uxDelay_ms)
{
    if(0 == uxDelay_ms)
    {
        return HAL_OK;
    }

    if(RSTMU->SYSINFST & BIT4)
    {
        CPU->CORET_RVR = (uxDelay_ms * SYSTICK_REFERENCE_CLOCK_PSMODE_RF) & 0x00FFFFFF; ///< RF mode
    }
    else
    {
        CPU->CORET_RVR = (uxDelay_ms * SYSTICK_REFERENCE_CLOCK_PSMODE_CT) & 0x00FFFFFF; ///< CT mode
    }

    CPU->CORET_CVR = 0;

    // 设置时钟源,基准时钟
    CLEAR_REG_BIT(CPU->CORET_CSR, 2);

    // 开启SYSTICK中断
    CTIM_IRQEN;
    SET_REG_BIT(CPU->CORET_CSR, 1);

    // 开启SYSTICK
    SET_REG_BIT(CPU->CORET_CSR, 0);

    return HAL_OK;
}

/*!< 停止systick */
HAL_StatusTypeDef HAL_SysTick_Stop(void)
{
    // 关闭SYSTICK中断
    CTIM_IRQDIS;
    CLEAR_REG_BIT(CPU->CORET_CSR, 1);

    CPU->CORET_CSR = 0;
    CPU->CORET_CVR = 0;
    return HAL_OK;
}

/*!< 启动timer */
HAL_StatusTypeDef HAL_Timer_Start(Timer_UseTypeDef uxTimer, Timer_ClkSelectTypeDef uxClkSelect, uint32_t uxDelay_ms)
{
    uint32_t uxClkSource;
    TIMERX_TypeDef *ptimer;

    /*!< 定位timer位置 */
    ptimer = (TIMERX_TypeDef *)(TIMER0_BA + uxTimer * 0x800);


    /*!< 开中断 */
    (uxTimer == 0x02) ? TIMER_IRQ_EN(2) : ((uxTimer == 0x01) ? TIMER_IRQ_EN(1) : TIMER_IRQ_EN(0));

    /*!< 放入值 */
    if(uxClkSelect == TIM_CLK_HOSC)
    {
        ///< 接触供电模式，对内部高频时钟源HOSC的256分频进行计数
        ptimer->TIMERXLOAD = CTMODE_TIMER_LOAD_VALUE(uxDelay_ms);
    }
    else if(uxClkSelect == TIM_CLK_EXTCLK)
    {
        ///< 射频模式
        ptimer->TIMERXLOAD = RFMODE_TIMER_LOAD_VALUE(uxDelay_ms);
    }
    else
    {
        ///< 对pclk计数
        if(CKMU->SYSCLKCFG1 & BIT10)
        {
            ///< 系统时钟二分频
            ptimer->TIMERXLOAD = uxDelay_ms * 44000;
        }
        else
        {
            ///< 系统时钟
            ptimer->TIMERXLOAD = uxDelay_ms * 88000;
        }
    }

    /*!< 配置时钟源 */
    uxClkSource = (uxClkSelect & 0x03) << ExtPluseCnt_Pos;

    /*!< 启动 */
    ptimer->TIMERXCONTROL |= (TimerEnable | uxClkSource | TimerSize | OneShotCount);
    return HAL_OK;
}

/*!< 停止timer */
HAL_StatusTypeDef HAL_Timer_Stop(Timer_UseTypeDef uxTimer)
{
    TIMERX_TypeDef *ptimer;
    /*!< 定位timer位置 */
    ptimer = (TIMERX_TypeDef *)(TIMER0_BA + uxTimer * 0x800);

    /*!< 清中断标记 */
    ptimer->TIMERXIS = 0;

    ptimer->TIMERXCONTROL = 0;


    /*!< 关闭中断 */
    (uxTimer == 0x02) ? TIMER_IRQ_DIS(2) : ((uxTimer == 0x01) ? TIMER_IRQ_DIS(1) : TIMER_IRQ_DIS(0));

    /*!< 清挂起事件 */
    (uxTimer == 0x02) ? TIMER_PEND_CLR(2) : ((uxTimer == 0x01) ? TIMER_PEND_CLR(1) : TIMER_PEND_CLR(0));

    return HAL_OK;
}

/*!< 获取timer状态 */
HAL_StatusTypeDef HAL_GetTimer_Status(Timer_UseTypeDef uxTimer)
{
    TIMERX_TypeDef *ptimer;

    /*!< 定位timer位置 */
    ptimer = (TIMERX_TypeDef *)(TIMER0_BA + uxTimer * 0x800);

    if(ptimer->TIMERXCONTROL & TimerEnable_Mak)
    {
        return HAL_BUSY;
    }
    else
    {
        return HAL_OK;
    }
}

HAL_StatusTypeDef HAL_Timer_Reset(Timer_UseTypeDef uxTimer, Timer_ClkSelectTypeDef uxClkSelect, uint32_t uxDelay_ms)
{
    HAL_Timer_Stop(uxTimer);
    HAL_Timer_Start(uxTimer, uxClkSelect, uxDelay_ms);
    return HAL_OK;
}

/*!< 启动Watchdog  */
HAL_StatusTypeDef HAL_WDT_Start(WatchDog_CtlModeDef uxWdt, uint32_t uxDelay_ms)
{
    WatchDog_CtlModeDef ucMode = uxWdt;


    /*!< load value */
    WDT->WDTLoad = uxDelay_ms;

    /*!< ctrl mode */
    if(ucMode == USE_RST)
    {
        __HAL_ENABLE_WDT_RESET();
    }
    else
    {
        __HAL_DISABLE_WDT_RESET();
        WDT_IRQEN;
    }

    /*!< 启动 */
    __HAL_ENABLE_WDT_INT();

    return HAL_OK;
}

/*!< 计时时间：~ucCountVal * 0.4us,最大102us  */
void HAL_SysDelay_us(uint8_t ucCountVal)
{
    CKMU->SYSCLKTIM2 = ucCountVal & 0xFF;
    SETSFR_BIT(CKMU->SYSCLKTIM2, SYSCLKTIM2_EN);

    while(CHECKSFR_BIT(CKMU->SYSCLKTIM2, SYSCLKTIM2_EN));
}

/*!< systick延时函数  */
HAL_StatusTypeDef HAL_SysTickDelay_ms(uint32_t uxDelay_ms)
{
    uint32_t tmp = 0;

    if(0 == uxDelay_ms)
    {
        return HAL_OK;
    }

    CPU->CORET_CSR &= ~(1 << 2);

    if(RSTMU->SYSINFST & BIT4)
    {
        CPU->CORET_RVR = (uxDelay_ms * SYSTICK_REFERENCE_CLOCK_PSMODE_RF) & 0x00FFFFFF;	///< RF mode
    }
    else
    {
        CPU->CORET_RVR = (uxDelay_ms * SYSTICK_REFERENCE_CLOCK_PSMODE_CT) & 0x00FFFFFF;	///< CT mode
    }

    CPU->CORET_CVR = 0;
    CPU->CORET_CSR |= (1 << 0);

    do
    {
        tmp = CPU->CORET_CSR;
    } while((tmp & 0x01) && !(tmp & (1 << 16)));

    CPU->CORET_CSR &= ~(1 << 0);
    CPU->CORET_CVR = 0;
    return HAL_OK;
}

/**
  * @}
  */
