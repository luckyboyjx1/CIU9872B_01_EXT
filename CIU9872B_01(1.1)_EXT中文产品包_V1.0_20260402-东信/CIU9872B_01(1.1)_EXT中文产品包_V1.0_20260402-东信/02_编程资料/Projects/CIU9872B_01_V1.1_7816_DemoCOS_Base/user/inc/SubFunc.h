/************************************************************************************************/
/**
* @file               SubFunc.h
* @author             Firmware Team
* @brief              Sub function process related definitions and declarations
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

#ifndef __SUBFUNC_H_
#define __SUBFUNC_H_

/* Includes ------------------------------------------------------------------------------------*/
/** @addtogroup APP_Demonstrations
  * @{
  */

/** @addtogroup SubFunc
  * @{
  */

//#define Poll_Handle
//#define NO_SUPPORT_CHAIN

//#define POLL_PROCS_SCI

#define APPDemoVer  0x10

#define SCI_BUSY 0x5A
#define SCI_IDLE 0x00

#define INTERFACE_7816   0x01
#define INTERFACE_I2C    0x02
#define INTERFACE_SPI    0x03
#define INTERFACE_RF     0x04

#define CASE3  3
#define CASE2  2
#define CASE1  1

void ProtocolProcess(void);

void Standby_Process(void);

uint32_t PowerdownRecoveryIn1stAPDU(void);
void PowerOnResumeLoader(void);

void HED_RAMInit(void);
void HAL_Init(void);
void OpenCache(void);

void ChipSysInit(void);

/**
  * @}
  */

/**
  * @}
  */

#endif /* __SUBFUNC_H_ */
