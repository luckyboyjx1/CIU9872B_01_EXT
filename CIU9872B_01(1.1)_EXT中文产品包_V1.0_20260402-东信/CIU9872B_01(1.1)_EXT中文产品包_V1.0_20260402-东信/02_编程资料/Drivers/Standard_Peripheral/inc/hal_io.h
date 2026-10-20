/************************************************************************************************/
/**
* @file               hal_io.h
* @author             Firmware Team
* @brief              Header file of IO Control HAL module.
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
#ifndef __HAL_IO_H_
#define __HAL_IO_H_

/* Includes ------------------------------------------------------------------------------------*/
#include "CIU9872B_01_V1.1.h"

/** @addtogroup Standard_Peripheral
  * @{
  */

/** @addtogroup IOCONTROL
  * @{
  */

/* Exported types ------------------------------------------------------------------------------*/
/** @defgroup IOCONTROL_Export_Types IOCONTROL Export Types
  * @{
  */

/**
  * @brief  IOCONTROL Configuration Structure definition
  */
typedef struct
{
    uint32_t PinPos;        /*!< Select PIN NUM.
                                 This parameter can be any value of @ref IO_pins */

//    uint32_t Mode;        /*!< 0: analog mode; 1/2: gpio mode;3: Alternate function mode ref define of reg IOCTRLMODE */

    uint32_t Alternate;     /*!< Set pin Alternate function.
                                 This parameter can be any value of @ref IO_Alternate */

    uint32_t Pull;          /*!< Set pin pull mode.
                                 This parameter can be any value of @ref IO_pull */

    uint32_t Driver;        /*!< Set pin driver mode.
                                 This parameter can be any value of @ref IO_driver */

    uint32_t OpenDrain;     /*!< Set pin open drain mode.
                                 This parameter can be any value of @ref IO_Open_Drain */
} IO_InitTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------------------------*/
/** @defgroup IOCONTROL_Exported_Constants IOCONTROL Exported Constants
  * @{
  */

/** @defgroup IO_pins IO pins
  * @brief IO pin number
  * @{
  */
#define IO_PIN_0                            ((uint32_t)0)       /*!< Pin 0 selected    */
#define IO_PIN_1                            ((uint32_t)1)       /*!< Pin 1 selected    */
#define IO_PIN_2                            ((uint32_t)2)       /*!< Pin 2 selected    */
#define IO_PIN_3                            ((uint32_t)3)       /*!< Pin 3 selected    */
#define IO_PIN_4                            ((uint32_t)4)       /*!< Pin 4 selected    */
#define IO_PIN_5                            ((uint32_t)5)       /*!< Pin 5 selected    */
#define IO_PIN_6                            ((uint32_t)6)       /*!< Pin 6 selected    */
#define IO_PIN_7                            ((uint32_t)7)       /*!< Pin 7 selected    */
/**
  * @}
  */

/** @defgroup IO_pull IO pull
  * @{
  */
#define IO_NOPULL                           (0x00000000U)
#define IO_PULLUP                           (0x00000001U)
#define IO_PULLDOWN                         (0x00000002U)
/**
  * @}
  */

/** @defgroup IO_driver IO driver
  * @{
  */
#define IO_DRIVER_SMALL                     (0x00000000U)
#define IO_DRIVER_BIG                       (0x00000001U)
/**
  * @}
  */

/** @defgroup IO_Open_Drain IO Open Drain
  * @{
  */
#define IO_PUSHPULL                         (0x00000000U)
#define IO_OPENDRAIN                        (0x00000001U)
/**
  * @}
  */

/** @defgroup IO_Alternate IO Alternate
  * @{
  */
#define IO0_AF_7816_RST                     (0x00000000U)
#define IO0_AF_GPIO0                        (0x00000001U)
#define IO0_AF_RST                          (0x00000002U)

#define IO1_AF_7816_CLK                     (0x00000000U)
#define IO1_AF_GPIO1                        (0x00000001U)
#define IO1_AF_JTAG_CLK                     (0x00000002U)
#define IO1_AF_I2C_SCL                      (0x00000003U)

#define IO2_AF_7816_IO                      (0x00000000U)
#define IO2_AF_GPIO2                        (0x00000001U)
#define IO2_AF_JTAG_IO                      (0x00000002U)
#define IO2_AF_I2C_SDA                      (0x00000003U)

#define IO3_AF_GPIO3                        (0x00000000U)
#define IO3_AF_SPI_CS                       (0x00000001U)

#define IO4_AF_GPIO4                        (0x00000000U)
#define IO4_AF_SPI_CLK                      (0x00000001U)
#define IO4_AF_SPI_MISO                     (0x00000002U)
#define IO4_AF_SPI_MOSI                     (0x00000003U)
#define IO4_AF_I2C_SDA                      (0x00000004U)
#define IO4_AF_I2C_SCL                      (0x00000005U)


#define IO5_AF_GPIO5                        (0x00000000U)
#define IO5_AF_SPI_CLK                      (0x00000001U)
#define IO5_AF_SPI_MISO                     (0x00000002U)
#define IO5_AF_SPI_MOSI                     (0x00000003U)
#define IO5_AF_I2C_SDA                      (0x00000004U)
#define IO5_AF_I2C_SCL                      (0x00000005U)

#define IO6_AF_GPIO6                        (0x00000000U)
#define IO6_AF_SPI_CLK                      (0x00000001U)
#define IO6_AF_SPI_MISO                     (0x00000002U)
#define IO6_AF_SPI_MOSI                     (0x00000003U)
#define IO6_AF_I2C_SDA                      (0x00000004U)
#define IO6_AF_I2C_SCL                      (0x00000005U)

#define IO7_AF_GPIO7                        (0x00000000U)
#define IO7_AF_SPI_CLK                      (0x00000001U)
#define IO7_AF_SPI_MISO                     (0x00000002U)
#define IO7_AF_SPI_MOSI                     (0x00000003U)
#define IO7_AF_I2C_SDA                      (0x00000004U)
#define IO7_AF_I2C_SCL                      (0x00000005U)
/**
  * @}
  */

/**
  * @}
  */
/* Exported Macro ------------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------*/
/** @addtogroup IOCONTROL_Exported_Functions
  * @brief IOCONTROL Exported Functions
  * @{
  */

void HAL_IO_Init(IO_InitTypeDef *Init);
void HAL_IO_DeInit(uint32_t PinPos);
void HAL_IO_DeInitAll(void);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __HAL_IO_H_ */
