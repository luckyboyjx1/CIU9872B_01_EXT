/************************************************************************************************/
/**
* @file               APDU_Command.h
* @author             Firmware Team
* @brief              The APDU command demonstrates the relevant definitions and declarations
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

#ifndef _APDU_COMMANND_H_
#define _APDU_COMMANND_H_

/**@addtogroup APP_Demonstrations
* @{
*/

/** @addtogroup APDU_Command
  * @{
  */

void cmd_Symmetry_ALG(void);
void cmd_CRC_Calculate(void);
void cmd_RSA_ALG(void);
void cmd_SM2_ALG(void);
void cmd_HASH_ALG(void);
void cmd_ECC_ALG(void);
void cmd_LOOP_BACK(uint32_t interfaceFlag);
void cmd_GET_INFO(uint32_t interfaceFlag);
void cmd_NVM_ErWr(void);
void cmd_back2Loader(void);
void cmd_getResponse(uint32_t interfaceFlag);
void cmd_selectFile(void);
void cmd_HEFLASH_Operation(void);
void cmd_M1_Opt(void);

extern uint16_t SW1SW2;

extern uint32_t HED_GetRandom(uint8_t *RandomData, uint32_t DataLen, uint32_t *paraChkSum);
extern uint32_t HED_GetRandom_Enhanced(uint8_t *RandomData, uint32_t DataLen, uint32_t *paraChkSum);

/**
* @}
*/

/**
* @}
*/


#endif
