/************************************************************************************************/
/**
* @file               hal_gpio.h
* @author             Firmware Team
* @brief              Header file of GPIO HAL module.
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
#ifndef __HAL_GPIO_H_
#define __HAL_GPIO_H_

/* Includes ------------------------------------------------------------------------------------*/
#include "CIU9872B_01_V1.1.h"
#include "hal_ckmu.h"

/** @addtogroup Standard_Peripheral
  * @{
  */

/** @addtogroup GPIO
  * @{
  */

/* Exported types ------------------------------------------------------------------------------*/
/** @defgroup GPIO_Export_Types GPIO Export Types
  * @{
  */

/**
  * @brief  GPIO PIN direction definition.
  */
typedef enum
{
    GPIO_PIN_DIR_IN     = 0U,
    GPIO_PIN_DIR_OUT    = 1U
} GPIO_PIN_DIR;

/**
  * @brief  GPIO PIN level status definition.
  */
typedef enum
{
    GPIO_PIN_RESET  = 0U,
    GPIO_PIN_SET    = 1U
} GPIO_PIN_STATE;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------------------------*/
/* Exported Macro ------------------------------------------------------------------------------*/
/** @defgroup SPI_Exported_Macros GPIO Exported Macros
  * @{
  */
/** @brief  Open GPIO clock.
  * @retval None
  */
#define __HAL_OPEN_GPIO_CLKEN()                          SET_REG_BIT(CKMU->SYSAPBCLKEN, GPIOEN)
/**
  * @}
  */

/* Exported functions --------------------------------------------------------------------------*/
/** @addtogroup GPIO_Export_Function GPIO Export Function
  * @brief GPIO Exported Functions
  * @{
  */

void                HAL_GPIO_Direct(uint32_t PinPos, GPIO_PIN_DIR GPIO_Dir);
HAL_StatusTypeDef   HAL_GPIO_WritePin(uint32_t PinPos, GPIO_PIN_STATE PinState);
GPIO_PIN_STATE      HAL_GPIO_ReadPin(uint32_t PinPos);
void                HAL_GPIO_IRQHandler(uint32_t PinPos);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __HAL_GPIO_H_ */
