/**
 ******************************************************************************
 * @file    hal_switch.h
 * @author  HED
 * @brief   switch HAL层头文件实现
 * @attention hal_switch
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

#ifndef __HAL_SWITCH_H_
#define __HAL_SWITCH_H_

#include <stdint.h>

void MACRO_Init_St(void);
void openWrSwitch1(void);
void openWrSwitch2(void);
void openWrSwitch3(void);
void closeWrSwitch12(void);
void closeWrSwitch234(void);
void closeWrSwitch34(void);
uint32_t HED_StSwitchAPI_Version(void);

#endif

