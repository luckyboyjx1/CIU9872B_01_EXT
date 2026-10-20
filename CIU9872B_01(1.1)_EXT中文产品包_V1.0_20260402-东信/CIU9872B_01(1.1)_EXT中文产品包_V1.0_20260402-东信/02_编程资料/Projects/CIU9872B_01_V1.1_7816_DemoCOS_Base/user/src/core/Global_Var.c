/************************************************************************************************/
/**
* @file               Global_Var.c
* @author             Firmware Team
* @brief              Golbal variable definition
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
* <tr><td>2021/03/12  <td>V1.0        <td>Firmware Team   <td>First release   </tr>
* </table>
*
**************************************************************************************************
*/

/* Includes ------------------------------------------------------------------------------------*/
#include "Declare.h"

/** @addtogroup APP_Demonstrations
  * @{
  */

/** @addtogroup Global_Var
  * @{
  */

/*SCI interface related variables*/
const uint8_t G_ATR[11] = {0x0A, 0x3B, 0x16, 0x96, 0x81, 0x17, 0x19, 0xDD, 0xB0, APPDemoVer}; //L+V:Length + Value
uint8_t G_BlockBuf[261];
uint16_t SW1SW2;

/*DMA channe related variables*/
DMA_HandleTypeDef hDMA_RxStVar, hDMA_TxStVar;

/*public var define*/
uint32_t APDU1stTag;
uint32_t G_insNum;
uint32_t G_output[100];
uint32_t G_paraCheckSum;


uint8_t  G_SendBuf[256];
uint32_t G_bINF;
uint32_t G_bDeSelect;
uint16_t SW1SW2;
uint16_t G_APDU_SxLen;      //APDU send len
uint32_t G_paraCheckSum;
uint32_t APDU1stTag;
uint32_t HEInit1stTag;
uint32_t G_RAMInitFlag;

/**
  * @}
  */

/**
  * @}
  */
