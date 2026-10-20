/************************************************************************************************/
/**
* @file               hal_gpio.c
* @author             Firmware Team
* @brief              GPIO HAL module driver.
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
#include "hal_gpio.h"

/** @addtogroup Standard_Peripheral
  * @{
  */

/** @addtogroup GPIO
  * @{
  */

/* Private types -------------------------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------------------------*/
/** @defgroup IO_Private_Constants IO Private Constants
  * @{
  */
#define GPIO_NUMBER                         (7U)   /*!< GPIO pin number */
/**
  * @}
  */

/* Private macros ------------------------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------*/

/**
* @brief  Set the current PIN to input or output mode.
* @note   Before using this API, you need to execute the IO configuration API and turn on the GPIO module gated clock.
* @note   The interface definition duplicates the function of the IO configuration API inorder to
*         quickly configure the IO input/output mode without changing other attributes of th IO pin.
*         In particular, when the IO is multiplexed into signle-wire IO mode communication,
*         the sending and receiving can be set through this interface.
* @param[in]  PinPos: PIN number. PIN value ranges from 0 to 7.
* @param[in]  GPIO_Dir: GPIO_PIN_DIR type.
* @retval none
  */
void HAL_GPIO_Direct(uint32_t PinPos, GPIO_PIN_DIR GPIO_Dir)
{
    if(PinPos > GPIO_NUMBER)
    {
        return;
    }

    if(GPIO_Dir != GPIO_PIN_DIR_OUT)
    {
        GPIO->GPIODIR &= ~(1 << PinPos);
    }
    else //GPIO_PIN_DIR_OUT
    {
        GPIO->GPIODIR |= (1 << PinPos);
    }
}

/**
* @brief  When the IOCTRL module is reused as a normal GPIO, the API can write value to a specific PinPos.
* @note   Inside the function, PinPos is automatically set as the output direction.
* @note   The API writes only one PIN value. The GPIO module gated clock needs to be enabled before using the API.
* @param[in]  PinPos: PIN number. PIN value ranges from 0 to 7.
* @param[in]  PinState: PIN_State type.
* @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef HAL_GPIO_WritePin(uint32_t PinPos, GPIO_PIN_STATE PinState)
{
    if(PinPos > GPIO_NUMBER)
    {
        return HAL_ERROR;
    }

    /* Set the pin to the output direction. */
    GPIO->GPIODIR |= (1 << PinPos);

    if(PinState != GPIO_PIN_RESET)
    {
        GPIO->GPIODATA |= (1 << PinPos);
    }
    else
    {
        GPIO->GPIODATA &= ~(1 << PinPos);
    }

    return HAL_OK;
}

/**
* @brief  When the IOCTRL module is reused as a normal GPIO, the API can read data from PinPos.
* @note   Inside the function, PinPos is automatically set as the input direction.
* @note   The API reads only one PIN value. The GPIO module gated clock needs to be enabled before using the API.
* @param[in]  PinPos: PIN number. PIN value ranges from 0 to 7.
* @retval GPIO_PIN_STATE
  */
GPIO_PIN_STATE HAL_GPIO_ReadPin(uint32_t PinPos)
{
    GPIO_PIN_STATE bitstatus;

    if(PinPos > GPIO_NUMBER)
    {
        return GPIO_PIN_RESET;
    }

    /* Set the pin to the input direction. */
    GPIO->GPIODIR &= ~(1 << PinPos);

    if((GPIO->GPIODATA & (1 << PinPos)) != (uint32_t)GPIO_PIN_RESET)
    {
        bitstatus = GPIO_PIN_SET;
    }
    else
    {
        bitstatus = GPIO_PIN_RESET;
    }

    return bitstatus;
}

/**
* @brief  When the IOCTRL module is reused as a normal GPIO, the API can clean interrupt from PinPos.
* @note   Inside the function, PinPos is automatically set as the input direction.
* @note   The API reads only one PIN value. The GPIO module gated clock needs to be enabled before using the API.
* @param[in]  PinPos: PIN number. PIN value ranges from 0 to 7.
* @retval none
  */
void HAL_GPIO_IRQHandler(uint32_t PinPos)
{
    if(PinPos >= GPIO_NUMBER)
    {
        return;
    }

    GPIO->GPIOIE &= ~(1 << PinPos);
    GPIO->GPIOIC |= (1 << PinPos);
    return;
}
/**
  * @}
  */

/**
  * @}
  */
