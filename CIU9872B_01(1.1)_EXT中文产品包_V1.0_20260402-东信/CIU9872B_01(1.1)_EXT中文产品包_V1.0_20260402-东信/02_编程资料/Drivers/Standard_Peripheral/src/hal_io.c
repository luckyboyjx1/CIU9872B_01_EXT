/************************************************************************************************/
/**
* @file               hal_io.c
* @author             Firmware Team
* @brief              IO Control HAL module driver.
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
#include "hal_io.h"

/** @addtogroup Standard_Peripheral
  * @{
  */

/** @addtogroup IOCONTROL
  * @{
  */


/* Private types -------------------------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------------------------*/
/** @defgroup IO_Private_Constants IO Private Constants
  * @{
  */
#define IO_PIN_NUMBER                       (7U)   /*!< GPIO number, range from 0 to 7. */
/**
  * @}
  */

/* Private macros ------------------------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------*/

/**
  * @brief  After using this interface, you can complete the following two functions.
  *         @arg The IO properties are configured. Such as Push Pull and Open Drain mode,
  *              pull-up and pull-down resistance, driver capability.
  *         @arg The IO multiplexing function is configured.
  *              The IO multiplexing function can be used as interfaces 7816,SPI1,SPI2,I2C,UART,GPIO.
  * @param[in] Init  pointer to a IO_InitTypeDef structure that contains the configuration
  *                  information for the specified IO peripheral.
  * @note   The user needs to initialize struct IO_InitTypeDef before calling the API.
  * @retval none
  */
void HAL_IO_Init(IO_InitTypeDef *Init)
{
    uint32_t Position, temp;

    if(Init->PinPos > IO_PIN_NUMBER)
    {
        return;
    }

    Position = Init->PinPos;

    /* Set IO Push Pull or Open Drain mode. */
    if(IO_OPENDRAIN == Init->OpenDrain)
    {
        IOCTRL->IOCTRLOD |= (1 << Position);
    }
    else //IO_PUSHPULL
    {
        IOCTRL->IOCTRLOD &= ~(1 << Position);
    }

    /* Set the IO driver capability. */
    if(IO_DRIVER_BIG == Init->Driver)
    {
        IOCTRL->IOCTRLDS |= (1 << Position);
    }
    else //IO_DRIVER_SMALL
    {
        IOCTRL->IOCTRLDS &= ~(1 << Position);
    }

    /* Set the IO pull-up and pull-down resistance. */
    if(IO_PULLUP == Init->Pull)
    {
        IOCTRL->IOCTRLPUEN |= (1 << Position);
    }
    else if(IO_PULLDOWN == Init->Pull)
    {
        IOCTRL->IOCTRLPDEN |= (1 << Position);
    }
    else
    {
        IOCTRL->IOCTRLPUEN &= ~(1 << Position);
        IOCTRL->IOCTRLPDEN &= ~(1 << Position);
    }

    /* Set the IO multiplexing function. */
    temp = IOCTRL->IOCTRLAF;

    if(Position > 3)
    {
        temp &= ~((0x07U << 8) << ((Position - 4) * 3));
        temp |= ((Init->Alternate << 8) << ((Position - 4) * 3));
    }
    else
    {
        temp &= ~(0x03U << (Position * 2));
        temp |= ((Init->Alternate) << (Position * 2));
    }

    IOCTRL->IOCTRLAF = temp;

    /* Enable IO configuration. */
    IOCTRL->IOCTRLEN |= (1 << Position);
}

/**
  * @brief  De-initialize the IO peripheral registers to their default reset values.
  * @param[in]  PinPos: IO number. PIN value ranges from 0 to 7.
  * @retval none
  */
void HAL_IO_DeInit(uint32_t PinPos)
{
    if(PinPos > IO_PIN_NUMBER)
    {
        return;
    }

    /* Set IO to push-pull mode by default. */
    IOCTRL->IOCTRLOD &= ~(1 << PinPos);

    /* Set IO driver capability to small by default. */
    IOCTRL->IOCTRLDS &= ~(1 << PinPos);

    /* Set the default status of the IO pull-down resistance. */
    if((IO_PIN_0 == PinPos) || (IO_PIN_1 == PinPos))
    {
        IOCTRL->IOCTRLPDEN |= (1 << PinPos); //The default status of IO0 power-on is 7816_RST. By default, it is disabled by pull-down.
    }
    else
    {
        IOCTRL->IOCTRLPDEN &= ~(1 << PinPos); //The remaining PINs are off.
    }

    /* Set the default status of the IO pull-up resistance. */
    if(IO_PIN_2 == PinPos)
    {
        IOCTRL->IOCTRLPUEN |= (1 << PinPos); //The default status of IO0 power-on is 7816_IO. By default, it is disabled by pull-up.
    }
    else
    {
        IOCTRL->IOCTRLPUEN &= ~(1 << PinPos); //Other PINs are turned on by default.
    }

    /* Set the default status of the IO multiplexing function. */
    if(PinPos > 3)
    {
        IOCTRL->IOCTRLAF &= ~((0x07U << 8) << ((PinPos - 4) * 3));
    }
    else
    {
        IOCTRL->IOCTRLAF &= ~(0x03U << (PinPos * 2));
    }

    /* Enable IO con figuration. */
    IOCTRL->IOCTRLEN |= (1 << PinPos);
}

void HAL_IO_DeInitAll(void)
{
    /* Set IO to push-pull mode by default. */
    IOCTRL->IOCTRLOD = 0;

    /* Set IO driver capability to small by default. */
    IOCTRL->IOCTRLDS = 0;

    /* Set the default status of the IO pull-down resistance. */
    IOCTRL->IOCTRLPDEN = 3;


    /* Set the default status of the IO pull-up resistance. */
    IOCTRL->IOCTRLPUEN = 4;

    /* Set the default status of the IO multiplexing function. */
    IOCTRL->IOCTRLAF = 0;


    /* Enable IO con figuration. */
    IOCTRL->IOCTRLEN |= 0xFF;
}

/**
  * @}
  */

/**
  * @}
  */
