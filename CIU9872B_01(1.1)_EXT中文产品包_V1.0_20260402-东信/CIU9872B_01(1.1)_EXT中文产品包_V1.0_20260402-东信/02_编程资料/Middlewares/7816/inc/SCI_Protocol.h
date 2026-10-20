/************************************************************************************************/
/**
* @file               SCI_Protocol.h
* @author             Firmware Team
* @brief              SCI Protocol related definitions and declarations
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

#ifndef _SCI_H_
#define _SCI_H_

/************************************************************************************************/
/**@addtogroup APP_Demonstrations
* @{
*/

/** @addtogroup SCI_Protocol
  * @{
  */
#define     b_SE_rd             (S7816->SCISCON & (1<<6))
#define     MACRO_bSE_ENABLE    S7816->SCISCON |= (1<<6)
#define     MACRO_bSE_DISABLE   S7816->SCISCON &= ~(1<<6)

#define     Rst7816En           2
#define     SCI_APDUBufSize     261

void SCI_Send_ATR(void);
void GVar_SCI_Init(void);
void SendINS(uint8_t ins);
uint8_t PPSExchange(void);

#define SetBaudRate(FDValue)                    \
{                                               \
    MACRO_bSE_DISABLE;                          \
    while(b_SE_rd != 0);                        \
    S7816->SCISCNT = FDValue;  /*48A or 80C*/          \
    MACRO_bSE_ENABLE;                           \
    while(b_SE_rd == 0);                        \
}
/**
* @}
*/

/**
* @}
*/

#endif
