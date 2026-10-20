/************************************************************************************************/
/**
* @file               hal_ckmu.c
* @author             Firmware Team
* @brief              CKMU HAL module driver.
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
#include "hal_ckmu.h"

/** @addtogroup Standard_Peripheral
  * @{
  */

/** @addtogroup CKMU
  * @{
  */

/* Private types -------------------------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------------------------*/
/** @defgroup clk_Private_Constants CLOCK Private Constants
  * @{
  */
#define CKMU_OPEN_AXB(reg,pos)          do{ \
                                            uint32_t tmpreg; \
                                            tmpreg = reg;   \
                                            if(((tmpreg >> (pos)) & 0x01) == 0x00)    \
                                            {   \
                                                tmpreg |= (uint32_t)(0x01 << (pos)); \
                                                reg = tmpreg;      \
                                            }   \
                                        } while(0U)

#define CKMU_CLOSE_AXB(reg,pos)         do{ \
                                            uint32_t tmpreg; \
                                            tmpreg = reg;   \
                                            tmpreg |= (uint32_t)(0x01 << (pos)); \
                                            reg = tmpreg;   \
                                        } while(0U)
/**
  * @}
  */

/* Private macros ------------------------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------*/
/**
  * @brief  AHB/APB总线功耗下模块时钟控制
  *
  * @param[in]  bus 总线类型：AHB1_CLK/AHB2_CLK/APB_CLK
  * @param[in]  clockpos 要控制的模块时钟位,与手册寄存器位对应
  * @param[in]  state 0关闭模块时钟，其它值开启模块时钟
  *
  * @return 返回状态
  *     @retval HAL_OK      成功
  *     @retval HAL_ERROR   失败
  *     @retval HAL_BUSY    进程IP模块被占用
  *     @retval HAL_TIMEOUT 等待超时
  */
HAL_StatusTypeDef HAL_CKMU_AXBControl(BUS_CLKTypeDef bus, uint32_t clockpos, uint32_t state)
{
    HAL_StatusTypeDef ret = HAL_OK;

    switch(bus)
    {
        case AHB1_CLK:

            if(state)
            {
                CKMU_OPEN_AXB(CKMU->SYSAHB1CLKEN, clockpos);
            }
            else
            {
                CKMU_CLOSE_AXB(CKMU->SYSAHB1CLKCLR, clockpos);
            }

            break;

        case AHB2_CLK:

            if(state)
            {
                CKMU_OPEN_AXB(CKMU->SYSAHB2CLKEN, clockpos);
            }
            else
            {
                CKMU_CLOSE_AXB(CKMU->SYSAHB2CLKCLR, clockpos);
            }

            break;

        case APB_CLK:

            if(state)
            {
                CKMU_OPEN_AXB(CKMU->SYSAPBCLKEN, clockpos);
            }
            else
            {
                CKMU_CLOSE_AXB(CKMU->SYSAPBCLKCLR, clockpos);
            }

            break;

        default :
            ret = HAL_ERROR;
            break;
    }

    return ret;
}



/**
  * @}
  */

/**
  * @}
  */
