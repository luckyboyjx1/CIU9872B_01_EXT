/************************************************************************************************/
/**
* @file               hal_pmu.c
* @author             Firmware Team
* @brief              PMU HAL module driver.
* @version            V1.0
* @date               2024-01-16
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
* @par Modify log
* <table>
* <tr><th>Date        <th>Version     <th>Author          <th>Description     </tr>
* <tr><td>2024/01/16  <td>V1.0        <td>Firmware Team   <td>First release   </tr>
* </table>
*
**************************************************************************************************
*/

/* Includes ------------------------------------------------------------------------------------*/
#include "hal_pmu.h"

/** @addtogroup Standard_Peripheral
  * @{
  */

/** @addtogroup PMU
  * @{
  */

/* Private types -------------------------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------------------------*/
/** @defgroup pmu_Private_Constants PMU Private Constants
  * @{
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------*/

/**
  * @brief  唤醒源前置LP模式配置，用户必须实现如下内容：
  *         1.standby/power-down模式需要配置唤醒源.
  *
  * @return none
  */
uint8_t __weak PrePWS_Setting(void)
{
#define IDLE (0)
#define BUSY (1)
    uint32_t ulPSR = __get_PSR();
    PMUM->SYSSTDWKPCON |= 0xFFFFFFFF;
    ///< 使用中断无效
    ulPSR &= ~(BIT6 | BIT8);
    __set_PSR(ulPSR);

//    if(InterfaceEvent() == IDLE)
//    {
//        return 1;
//    }
//    else
    {
        return 1;
    }
}

/**
  * @brief  唤醒源后置LP模式配置
  *
  * @return none
  */
void __weak PWS_Setting(void)
{
    uint32_t ulPSR = __get_PSR();
    PMUM->SYSSTDWKPSTATUS1 = 0;
    ///< 使能中断有效
    ulPSR |= (BIT6 | BIT8);
    __set_PSR(ulPSR);
}

/**
  * @brief  LP模式进入
  *
  * @param[in]  lp_moden PMU模块控制设置，与SYSPMDCON寄存器位对应
  *
  * @return none
  */
void HAL_PMU_EnterLPMode(SYSPMDCON_LP_MODE lp_moden)
{
    uint32_t uxBakBuf[5];

    if(STANDBY_MODE == lp_moden)
    {
        PreSet_SYSCLK_ICS(STANDBY_MODE, 0, uxBakBuf);

        if(PrePWS_Setting())
        {
            PMUM->SYSPMDCON = lp_moden;
            //< enter std mode
            #ifdef MDK
            __sev();
            __wfe();
            __wfe();
            #else
			if((CPU->VIC_ISPR & (BIT13 | BIT15 | BIT16 | BIT17)) == 0)
			{
				asm(
					"doze"//set STD
				);
			}
            #endif
        }

        PostSet_SYSCLK_ICS(STANDBY_MODE, uxBakBuf);
        PWS_Setting();
    }
    else
    {
        PrePWS_Setting();
        //< enter pwd mode
        PMUM->SYSPMDCON = lp_moden;
		
		asm(
			"doze"//set STD
		);

        while(1);
    }
}
/**
  * @}
  */

/**
  * @}
  */
