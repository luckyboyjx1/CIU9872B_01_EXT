/************************************************************************************************/
/**
* @file               SCI_Config.c
* @author             Firmware Team
* @brief              SCI initial confuguration
* @version            V1.0
* @date               2021-03-12
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
* <tr><td>2021/03/12  <td>V1.0        <td>Firmware Team   <td>First release     </tr>
* </table>
*
**************************************************************************************************
*/

/* Includes ------------------------------------------------------------------------------------*/
#include "Declare.h"

/************************************************************************************************/
/** @addtogroup APP_Demonstrations
  * @{
  */

/** @addtogroup SCI_Config
  * @{
  */

/* Private types -------------------------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------*/

/**
  * @brief  SCI IP initialization, enabling the hardware to automaticallt send 3B function.
  * @param  None
  * @return None
  */
void SCI_Init(void)
{
    HED_ChangeSYSFreq(SYSCLK_88Mhz, SYSCLK_11Mhz, SYSCLK_LF_13p56Mhz, SYSCLK_LF_3p39Mhz);
    G_bINF = INTERFACE_7816;     //flag of SCI interface working

    /*  SCI interface init*/

    HAL_CKMU_AXBControl(APB_CLK, SCIEN, 1);    ///< 打开CT时钟

    #ifdef HARDWARE_SEND3BH
    S7816->SCISCON = 0x1E53;//RTS==1,auto 0x3B sent by IP(AWTS=4096CLK),SE=1,RCNT=3,other default setting
    #else
    S7816->SCISCON = 0x1B53;//RTS==1,auto 0x3B sent by IP(AWTS=4096CLK),SE=1,RCNT=3,other default setting
    #endif

    while((S7816->SCISCON & BIT6) == 0x00);       ///< 等待串口使能

    RSTMU->SYSRSTCFG |= BIT2;       ///< 7816复位作为系统复位

    SCI_Send_ATR();
}


/**
  * @}
  */

/**
  * @}
  */
