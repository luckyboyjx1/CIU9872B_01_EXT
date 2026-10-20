/**
  ******************************************************************************
  * @file    hal_factorycode.c
  * @author  HED
  * @brief   读取工厂码 HAL层源码实现
  ******************************************************************************
  * @attention
  *
  * 读取工厂码接口函数实现
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
/** @addtogroup CIU9872B_01_V1_1_HAL_Driver
  * @{
  */

/** @addtogroup HAL_FactoryCode
  * @{
  */
#include "hal_factorycode.h"

#define FACTORY_CODE_SADDR      0x1FFFFE00

/* Serial Num */
void HED_GetChipInfo(uint8_t *DataBuf, uint32_t FlashAddr, uint32_t DataLen)
{
    while(DataLen--)
    {
        *DataBuf++ = *(uint8_t *)FlashAddr;
        FlashAddr += 0x00000001;
    }
}

/**
* @brief  获取40字节工厂码
* @note   无
* @param[in]  FactoryCode：输出的芯片工厂码数据在内存RAM中起始地址
* @param[out] FactoryCode地址中的内容
* @retval 无
  */
void HED_GetFactoryCode(uint8_t *FactoryCode)
{
    HED_GetChipInfo(FactoryCode, FACTORY_CODE_SADDR, 40);
}


/**
  * @}
  */

/**
  * @}
  */

