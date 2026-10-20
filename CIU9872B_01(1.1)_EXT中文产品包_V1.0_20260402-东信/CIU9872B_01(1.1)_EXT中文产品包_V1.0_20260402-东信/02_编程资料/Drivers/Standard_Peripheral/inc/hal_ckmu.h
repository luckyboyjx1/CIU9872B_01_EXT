/************************************************************************************************/
/**
* @file               hal_ckmu.h
* @author             Firmware Team
* @brief              Header file of CKMU HAL module.
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
#ifndef __HAL_CKMU_H_
#define __HAL_CKMU_H_

/* Includes ------------------------------------------------------------------------------------*/
#include "CIU9872B_01_V1.1.h"

/** @addtogroup Standard_Peripheral
  * @{
  */

/** @addtogroup CLOCK
  * @{
  */

/* Exported types ------------------------------------------------------------------------------*/
/** @defgroup CLOCK_Export_Types CLOCK Export Types
  * @{
  */

/**
  * @brief  总线类型定义
  */
typedef enum
{
    AHB1_CLK    = 0x00000000U,
    AHB2_CLK    = 0x00000001U,
    APB_CLK     = 0x00000002U
} BUS_CLKTypeDef;

/**
  * @brief SYSAHB2CLKEN register bit enumeration
  */
typedef enum
{
    SPIEN      = 0U,
    CRCEN     = 4U,
    DMACEN      = 7U,
    TYPEAEN       = 8U,
    TYPEBEN     = 10U
} SYSAHB2CLKEN_REG_BITS;

/**
  * @brief SYSAHB2CLKCLR register bit enumeration
  */
typedef enum
{
    SPIOFF     = 0U,
    CRCOFF    = 4U,
    DMACOFF     = 7U,
    TYPEAOFF      = 8U,
    TYPEBOFF    = 10U
} SYSAHB2CLKCLR_REG_BITS;

/**
  * @brief SYSAPBCLKEN register bit enumeration
  */
typedef enum
{
    TIMER0PCLKEN    = 0U,
    TIMER1PCLKEN    = 1U,
    TIMER2PCLKEN    = 2U,
    WDTEN           = 8U,
    SCIEN           = 11U,
    I2CEN           = 17U,
    GPIOEN          = 24U,
    RNG1EN          = 25U
} SYSAPBCLKEN_REG_BITS;

/**
  * @brief SYSAPBCLKCLR register bit enumeration
  */
typedef enum
{
    TIMER0PCLKOFF   = 0U,
    TIMER1PCLKOFF   = 1U,
    TIMER2PCLKOFF   = 2U,
    WDTOFF          = 8U,
    SCIOFF          = 11U,
    I2COFF          = 17U,
    GPIOOFF         = 24U,
    RNG1OFF         = 25U
} SYSAPBCLKCLR_REG_BITS;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------------------------*/
/* Exported Macro ------------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------*/
/** @addtogroup CLOCK_Export_Function CLOCK Export Function
  * @brief CLOCK Exported Functions
  * @{
  */
HAL_StatusTypeDef HAL_CKMU_AXBControl(BUS_CLKTypeDef bus, uint32_t clockpos, uint32_t state);


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __HAL_CKMU_H_ */
