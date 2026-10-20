/************************************************************************************************/
/**
* @file               hal_pmu.h
* @author             Firmware Team
* @brief              Header file of PMU HAL module.
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
#ifndef __HAL_PMU_H_
#define __HAL_PMU_H_

/* Includes ------------------------------------------------------------------------------------*/
#include "CIU9872B_01_V1.1.h"
#include "CIU9872B_01_V1.1_ALG_API_HEX.h"

/** @addtogroup Standard_Peripheral
  * @{
  */

/** @addtogroup PMU
  * @{
  */

/* Exported types ------------------------------------------------------------------------------*/
/** @defgroup PMU_Export_Types PMU Export Types
  * @{
  */

/**
  * @brief SYSPMDCON register bit enumeration
  */
typedef enum
{
    STANDBY_MODE        = 0x00U,
    POWERDOWN_MODE      = 0x08U
} SYSPMDCON_LP_MODE;


/**
  * @brief SYSSTDWKPCFG register bit enumeration
  */
typedef enum
{
    GPIO_WKPOS      = 0U,
    SPISSWKP        = 4U
} SYSSTDWKPCFG_REG_BITS;

/**
  * @brief SYSWKPCON register bit enumeration
  */
typedef enum
{
    RFOFF_WKPEN         = 0U,
    RFON_WKPEN          = 1U,
    I2C_WKPEN           = 4U,
    SCI_WKPEN           = 8U,
    SPIWKPEN            = 10U,
    GPIO0_WKPEN         = 16U,
    GPIO7_WKPEN         = 17U
} SYSWKPCON_REG_BITS;

/**
  * @brief SYSSTDWKPSTATUS1 register bit enumeration
  */
typedef enum
{
    RFOFF_WKP_FLAG      = 0U,
    RFON_WKP_FLAG       = 1U,
    I2C_WKP_FLAG        = 4U,
    SCI_WKP_FLAG        = 8U,
    SPIWKP_FLAG         = 10U,
    GPIO0WKP_FLAG       = 16U,
    GPIO7WKP_FLAG       = 17U
} SYSSTDWKPSTATUS1_REG_BITS;

/**
  * @brief SYSSTDWKPSTATUS2 register bit enumeration
  */
typedef enum
{
    SCIDET_WKP_FLAG     = 0U,
    SCIRCV_WKP_FLAG     = 1U,
    SCIRST_WKP_FLAG     = 2U
} SYSSTDWKPSTATUS2_REG_BITS;


/**
  * @brief SYSPWDWKPCON register bit enumeration
  */
typedef enum
{
    VRFPDWKPEN      = 0U,
    IO0PDWKPEN      = 1U,
    IO3PDWKPEN      = 2U
} SYSPWDWKPCON_REG_BITS;

/**
  * @brief SYSPWDWKPSTATUS register bit enumeration
  */
typedef enum
{
    VRF_WKP_FLAG        = 0U,
    IO0WKP_FLAG         = 1U,
    IO3WKP_FLAG         = 2U
} SYSPWDWKPSTATUS_REG_BITS;



/**
  * @}
  */

/* Exported constants --------------------------------------------------------------------------*/
/* Exported Macro ------------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------*/

/** @addtogroup PMU_Export_Function PMU Export Function
  * @brief PMU Exported Functions
  * @{
  */

uint8_t PrePWS_Setting(void);
void PWS_Setting(void);
void HAL_PMU_EnterLPMode(SYSPMDCON_LP_MODE lp_mode);



/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __HAL_PMU_H_ */
