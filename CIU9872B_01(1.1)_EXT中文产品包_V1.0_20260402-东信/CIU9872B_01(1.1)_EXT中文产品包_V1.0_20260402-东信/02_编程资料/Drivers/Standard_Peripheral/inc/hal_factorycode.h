/**
  ******************************************************************************
  * @file    hal_factorycode.h
  * @author  HED
  * @brief   读取工厂码 HAL层头文件定义
  ******************************************************************************
  * @attention
  *
  * 读取工厂码 接口函数实现
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
  * <tr><td>2024/05/15  <td>V1.0        <td>Firmware Team   <td>创建            </tr>
  * </table>
  *
  ******************************************************************************
  */
#ifndef __HAL_FACTORYCODE_H
#define __HAL_FACTORYCODE_H

#include "CIU9872B_01_V1.1.h"

void HED_GetFactoryCode(uint8_t *FactoryCode);

#endif

