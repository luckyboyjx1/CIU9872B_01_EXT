/**
 ******************************************************************************
 * @file    hal_rng.h
 * @author  HED
 * @brief   switch HAL层头文件实现
 * @attention hal_rng
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
 * <tr><th>Date        <th>Version     <th>Author          <th>Description     </tr>
 * <tr><td>2021/04/08  <td>V1.0        <td>Firmware Team   <td>创建            </tr>
 * </table>
 ******************************************************************************
 */

#ifndef __HAL_RNG_H_
#define __HAL_RNG_H_

#include <stdint.h>
/* API FUCTION DEFINE */
/* Random */
uint32_t HED_GetRandom_Enhanced(uint8_t *RandomData,uint32_t DataLen, uint32_t *paraChkSum);
uint32_t HED_GetRandom(uint8_t *RandomData,uint32_t DataLen, uint32_t *paraChkSum);
/* Check */
unsigned int HED_RNG1Check(void);
/*Get RNG API Version*/
uint32_t HED_RNGAPI_Version(void);
#endif   /* __HAL_RNG_H_ */
