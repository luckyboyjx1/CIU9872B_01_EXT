/************************************************************************************************/
/**
* @file               Global_Var.h
* @author             Firmware Team
* @brief              APP Demo global variable related definition and declaration
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


#ifndef __GLABAL_VAR_H
#define __GLABAL_VAR_H

/** @addtogroup APP_Demonstrations
  * @{
  */

/** @addtogroup Global_Var
  * @{
  */

/*SCI interface related variables*/
extern const uint8_t G_ATR[];//L+V:Length + Value
extern uint16_t SW1SW2;
#define 	G_APDUHead 	G_BlockBuf
#define     SCIAPDUBuf  G_BlockBuf
#define     CLA        G_APDUHead[0]
#define     INS        G_APDUHead[1]
#define     P1         G_APDUHead[2]
#define     P2         G_APDUHead[3]
#define     P3         G_APDUHead[4]
#define     EXTENDLC_H G_APDUHead[5]
#define     EXTENDLC_L G_APDUHead[6]

/*public var define*/
extern uint8_t G_BlockBuf[261];
extern uint32_t APDU1stTag;
extern uint32_t G_insNum;
extern uint32_t G_output[];
extern uint32_t G_paraCheckSum;


extern uint8_t  G_SendBuf[256];
extern uint32_t G_bINF;
extern uint32_t G_bDeSelect;
extern uint16_t SW1SW2;
extern uint16_t G_APDU_SxLen;      //APDU send len
extern uint32_t G_paraCheckSum;
extern uint32_t APDU1stTag;
extern uint32_t HEInit1stTag;
extern uint32_t G_RAMInitFlag;

/**
  * @}
  */

/**
  * @}
  */

#endif
