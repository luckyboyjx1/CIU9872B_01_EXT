/**
  ******************************************************************************
  * @file    libHED_API_SymmetricAlg_SecSM4.h
  * @author  Firmware Team
  * @brief   Alg Api Entry Header File
  * @date    2024-03-22
  * @version V1.0
  ******************************************************************************
  * @attention libHED_API_SymmetricAlg_SecSM4.h
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
  * <tr><td>2021/04/08  <td>V1.0        <td>Firmware Team   <td>Create        </tr>
  * <tr><td>2021/04/08  <td>V1.1        <td>Firmware Team   <td>ModifyXXXX    </tr>
  * </table>
  ******************************************************************************
  */
/** @defgroup ALG
  * @brief ALG Header File
  * @{
  */
/** @defgroup libHED_API_SymmetricAlg_SecSM4.h
  * @brief libHED_API_SymmetricAlg_SecSM4.h
  * @{
  */
#ifndef libHED_API_SymmetricAlg_SecSM4_H__
#define libHED_API_SymmetricAlg_SecSM4_H__
/******************************************************************************/

/******************************************************************************/
/******************************************************************************/
/*
################################################################################
********************* Enumeration and structure Definition *********************
################################################################################
*/
// Symmetric algorithm
typedef struct
{
    unsigned int *iDataPtr;
    unsigned int DataLen;
    unsigned int *iKeyPtr;
    unsigned int *IVPtr;
    unsigned int *oDataPtr;
    unsigned char Type;
    unsigned int  paraChkSum;
} SM4_Parameters;
unsigned int HED_SM4Block(SM4_Parameters *SM4_Param);
/******************************************************************************/
#endif/* libHED_API_SymmetricAlg_SecSM4_H__ */
/**
  * @}
  */
/**
  * @}
  */

