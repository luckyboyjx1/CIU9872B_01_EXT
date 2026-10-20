/**
  ******************************************************************************
  * @file    HED_API_ChipID.h
  * @author  Firmware Team
  * @brief   Alg Api Entry Header File
  * @date    2022-11-09
  * @version V1.0
  ******************************************************************************
  * @attention HED_API_ChipID.h
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
  * <tr><th>Date        <th>Version     <th>Author          <th>Description   </tr>
  * <tr><td>2022/11/09  <td>V1.0        <td>Firmware Team   <td>Create        </tr>
  * </table>
  ******************************************************************************
  */
/** @defgroup CHIP ID
  * @brief CHIP ID Header File
  * @{
  */
/** @defgroup HED_API_ChipID.h
  * @brief HED_API_ChipID.h
  * @{
  */
#ifndef HED_API_GETCHIPID_H__
#define HED_API_GETCHIPID_H__
/******************************************************************************/
/*
################################################################################
************************ Function Prototype Declaration ************************
################################################################################
*/
uint32_t HED_GetChipId(uint8_t *ChipId, uint32_t DataLen);
uint32_t HED_ChipIDAPI_Version(void);
/******************************************************************************/
#endif/* HED_API_GETCHIPID_H__ */
/**
  * @}
  */
/**
  * @}
  */
