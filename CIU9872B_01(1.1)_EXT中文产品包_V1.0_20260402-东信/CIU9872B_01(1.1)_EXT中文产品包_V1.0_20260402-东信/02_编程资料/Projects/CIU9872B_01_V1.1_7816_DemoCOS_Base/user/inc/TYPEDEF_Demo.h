/************************************************************************************************/
/**
* @file               Typedef_demo.h
* @author             Firmware Team
* @brief              Base bit op related definitions and declarations
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

#ifndef __TYPEDEF_H_
#define __TYPEDEF_H_

#ifndef false
#define false       (0)
#endif

#ifndef true
#define true        (1)
#endif

typedef     unsigned char   bool;


#define     DEMO_SET_BIT(x,y)       (x |= (0x01 <<y))
#define     DEMO_CLR_BIT(x,y)       (x &= (~(0x01 <<y)))
#define     DEMO_CHECK_BIT(x,y)     (x & (0x01 <<y))

#endif
