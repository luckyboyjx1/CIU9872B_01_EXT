/**
  ******************************************************************************
  * @file    CIU9872B_01_V1.1_AES.h
  * @author  Firmware Team
  * @brief   Alg lib Header File
  * @date    2022-08-03
  * @version V1.0
  ******************************************************************************
  * @attention CIU9872B_01_V1.1_AES.h
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
  * @par 修改日志
  * <table>
  * <tr><th>Date        <th>Version     <th>Author          <th>Description   </tr>
  * <tr><td>2022/08/03  <td>V1.0        <td>Firmware Team   <td>创建            </tr>
  * <tr><td>2021/04/08  <td>V1.1        <td>Firmware Team   <td>修改XXXX        </tr>
  * </table>
  ******************************************************************************
  */
/** @defgroup ALG
  * @brief ALG core code
  * @{
  */
/** @defgroup AES
  * @brief CIU9872B_01_V1.1_AES.h
  * @{
  */
#ifndef CIU9872B_01_V1_1_AES_H__
#define CIU9872B_01_V1_1_AES_H__
/************************************************************************************/
#include <stdint.h>
/************************************************************************************/
void Rijndael_Encrypt(uint8_t *state, uint8_t *roundkey, uint8_t Nr);
void Rijndael_KeyExpansion(uint8_t *inkey, uint8_t *roundkey, uint16_t bit);
unsigned int HED_AESAPI_Version(void);
/************************************************************************************/
#endif//CIU9872B_01_V1_1_AES_H__
/**
  * @}
  */
/**
  * @}
  */

