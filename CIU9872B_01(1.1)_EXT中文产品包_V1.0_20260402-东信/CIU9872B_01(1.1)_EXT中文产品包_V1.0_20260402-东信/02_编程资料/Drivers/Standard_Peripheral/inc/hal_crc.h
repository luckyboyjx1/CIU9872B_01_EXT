/************************************************************************************************/
/**
* @file               hal_crc.h
* @author             Firmware Team
* @brief              Header file of CRC HAL module.
* @version            V1.0
* @date               2023-01-16
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
* <tr><td>2022/11/09  <td>V1.0        <td>Firmware Team   <td>First release   </tr>
* </table>
*
**************************************************************************************************
*/

#ifndef __HAL_CRC_H_
#define __HAL_CRC_H_

/* Includes ------------------------------------------------------------------------------------*/
#include "CIU9872B_01_V1.1.h"

/** @addtogroup Standard_Peripheral
  * @{
  */

/** @addtogroup CRC
  * @{
  */

/* Exported types ------------------------------------------------------------------------------*/
/** @defgroup CRC32_Exported_Types CRC32 Exported Types
  * @{
  */

/**
  * @brief  CRC32 CTRL register enumeration definition
  */
typedef enum
{
    CRCCTRL_CRCPOLY       = 0U,
    CRCCTRL_XORIN         = 1U,
    CRCCTRL_REFIN         = 2U,
    CRCCTRL_XOROUT        = 3U,
    CRCCTRL_REFOUT        = 4U,
    CRCCTRL_SWAPIN        = 5U,
    CRCCTRL_SWAPOUT       = 6U
} CRCCTRL_REG_BITS;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------------------------*/
/** @defgroup CRC32_Exported_Constants CRC32 Exported Constants
  * @{
  */

/** @defgroup Bit definition for CRC32CTRL register
  * @{
  */
/* CRC Polynomal */
#define CRC_POLY_CRC32                          (0 << CRCCTRL_CRCPOLY)  //x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1
#define CRC_POLY_CRC16_CCITT                    (1 << CRCCTRL_CRCPOLY)  //x16+x12+x5+1
/* CRC input data inversion mode: bit */
#define CRC_INPUTDATA_BIT_INVERSION             (1 << CRCCTRL_XORIN)
/* CRC input data bit reversal in byte */
#define CRC_INPUTDATA_BYTE_BITREVERSAL          (1 << CRCCTRL_REFIN)
/* CRC output data inversion mode: bit */
#define CRC_OUTPUTDATA_BIT_INVERSION            (1 << CRCCTRL_XOROUT)
/* CRC output data bit reversal */
#define CRC_OUTPUTDATA_BIT_REVERSAL             (1 << CRCCTRL_REFOUT)
/* CRC input data endian conversion */
#define CRC_INPUT_ENDIAN_CONVER                 (1 << CRCCTRL_SWAPIN)
/* CRC output data endian conversion */
#define CRC_OUTPUT_ENDIAN_CONVER                (1 << CRCCTRL_SWAPOUT)
/**
  * @}
  */

/** @defgroup CRC32 type
  * @{
  */
#define CRC_TYPE_CRC32                            (CRC_INPUT_ENDIAN_CONVER | CRC_OUTPUTDATA_BIT_INVERSION | CRC_POLY_CRC32)
#define CRC_TYPE_CCITT                            (CRC_INPUT_ENDIAN_CONVER | CRC_OUTPUTDATA_BIT_REVERSAL | CRC_INPUTDATA_BYTE_BITREVERSAL | CRC_POLY_CRC16_CCITT)
#define CRC_TYPE_TYPE_A                           (CRC_OUTPUT_ENDIAN_CONVER | CRC_INPUT_ENDIAN_CONVER | CRC_POLY_CRC16_CCITT)
#define CRC_TYPE_TYPE_B                           (CRC_OUTPUT_ENDIAN_CONVER | CRC_INPUT_ENDIAN_CONVER | CRC_OUTPUTDATA_BIT_INVERSION | CRC_POLY_CRC16_CCITT)
#define CRC_TYPE_JAVATCK                          (CRC_OUTPUT_ENDIAN_CONVER | CRC_INPUT_ENDIAN_CONVER | CRC_OUTPUTDATA_BIT_REVERSAL | CRC_OUTPUTDATA_BIT_INVERSION | CRC_INPUTDATA_BYTE_BITREVERSAL | CRC_POLY_CRC16_CCITT)
#define CRC_TYPE_SWP                              (CRC_INPUT_ENDIAN_CONVER | CRC_OUTPUTDATA_BIT_REVERSAL | CRC_INPUTDATA_BYTE_BITREVERSAL | CRC_POLY_CRC16_CCITT)
/**
  * @}
  */
#define CRC_CLK_EN                                  (CKMU->SYSAHB2CLKEN |= BIT4)
#define CRC_CLK_CLR                                 (CKMU->SYSAHB2CLKCLR |= BIT4)
/**
  * @}
  */
#define HED_CRC16(IV, DataLen, DataBuf, type)           HED_CRC((IV & 0xFFFF), DataLen, DataBuf, (type | BIT0))
#define HED_CRC32(IV, DataLen, DataBuf, type)           HED_CRC(IV, DataLen, DataBuf, (type & (~BIT0)))
/* Exported Macro ------------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------*/
/** @addtogroup CRC_Export_Function CRC Export Function
  * @brief CRC Exported Functions
  * @{
  */

uint32_t HED_CRC(uint32_t IV, uint32_t DataLen, uint8_t *DataBuf, uint32_t type) PRIVILEGED_FUNCTION;

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __HAL_CRC32_H_ */
