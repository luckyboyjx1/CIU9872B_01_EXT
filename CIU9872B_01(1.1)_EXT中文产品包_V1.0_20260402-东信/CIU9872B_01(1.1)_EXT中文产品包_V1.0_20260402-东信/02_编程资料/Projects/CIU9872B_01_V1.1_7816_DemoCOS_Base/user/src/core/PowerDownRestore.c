/************************************************************************************************/
/**
* @file               PowerDownRestore.c
* @author             Firmware Team
* @brief              HEFlash power failure hangling demonstration
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
* <tr><td>2022/11/09  <td>V1.0        <td>Firmware Team   <td>First release   </tr>
* </table>
*
**************************************************************************************************
*/

/* Includes ------------------------------------------------------------------------------------*/
#include "Declare.h"

/** @addtogroup APP_MutliInterface_Demo
  * @{
  */

/** @addtogroup PowerDownRestore
  * @{
  */

#define HEFLASH_FAIL            0xFFFFFFFF

/**
  * @brief  Demonstrate the HED_HEFLASH_Init function call method after recriving
            the first APDU command.
  * @note   none
  * @param  none
  * @retval none
  */
uint32_t PowerdownRecoveryIn1stAPDU(void)
{
    uint32_t ret;
    /* !!!NOTE:  COS must call HED_HEFlash_Init before any operation of writing HEFlash.
       This apdu is 1st APDU.
       For example :You can add 'call HED_HEFlash_Init' in here .
    */
    ret = 0;

    //#warning "skip HEFlash init API!!!"
    ret = HED_HEFlash_Init();

    if(ret != 0)
    {
        /* !!!NOTE:
           User need to judge ret in detail */
        if(ret == HEFLASH_FAIL)
        {
            //Maybe it means HEFlash become invalid . Restart IC agagin will be best method.

        }

        return ret;
    }
    else
    {
        //could Add COS application data 'power down recovery', code in here.


    }

    return ret;

}

/**
  * @}
  */

/**
  * @}
  */
