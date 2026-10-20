/************************************************************************************************/
/**
* @file               declare.h
* @author             Firmware Team
* @brief              APP Mutli Interface demo all header files
* @version            V1.0
* @date               2022-11-09
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
* <tr><td>2022/11/09  <td>V1.1        <td>Firmware Team   <td>First release   </tr>
* </table>
*
**************************************************************************************************
*/

#ifndef _DECLARE_H_
#define _DECLARE_H_

#include <string.h>
#include "stdint.h"
#include "CIU9872B_01_V1.1.h"
/* HAL API inc define*/
#include "hal_timer.h"
#include "hal_dma.h"
#include "hal_ckmu.h"
#include "hal_pmu.h"
#include "hal_sci.h"
#include "hal_factorycode.h"
#include "hal_crc.h"
#include "hal_ramInit.h"
#include "HED_API_ChipID.h"
#include "HED_API_NVM.h"
#include "HED_GCM.h"
#include "HED_CCM.h"
#include "CIU9872B_01_V1.1_AES.h"
#include "CIU9872B_01_V1.1_ALG_API_HEX.h"
#include "CIU9872B_01_V1.1_API_NVM_Switch.h"

/*APP COS inc define*/
#include "TYPEDEF_Demo.h"
#include "SubFunc.h"
#include "APDU_Command.h"
#include "SCI_Config.h"
#include "SCI_Protocol.h"
#include "Interrupt.h"
#include "Global_Var.h"

#endif /* _DECLARE_H_ */
