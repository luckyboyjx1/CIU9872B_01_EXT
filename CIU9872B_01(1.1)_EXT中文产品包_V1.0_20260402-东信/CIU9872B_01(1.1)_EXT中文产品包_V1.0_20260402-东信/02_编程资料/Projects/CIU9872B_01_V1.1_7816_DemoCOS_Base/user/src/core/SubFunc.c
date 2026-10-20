/************************************************************************************************/
/**
* @file               SubFunc.c
* @author             Firmware Team
* @brief              Sub function process
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

/* Includes ------------------------------------------------------------------------------------*/
#include "Declare.h"

/** @addtogroup APP_MutliInterface_Demo
  * @{
  */

/** @addtogroup SubFunc
  * @{
  */
void HED_RAMInit(void)
{
#define CHIP_SRAM_START_ADDR    0x20000000
#define RAM_SIZE                0x5000      //RAM size 20KB
    hal_RAMInit(CHIP_SRAM_START_ADDR, RAM_SIZE);
}

/**
  * @brief  Chip init function
  * @note   none
  * @param  none
  * @retval none
  */
void HAL_Init(void)
{
    APDU1stTag = 0;
    HEInit1stTag = 0;
    G_RAMInitFlag = 0;
    ChipSysInit();
    __HAL_OPEN_TIMER0_CLK();
    openWrSwitch1();

}

//SYSICACON bit define
#define b_CA_EN           28
#define b_CA_DATA_INV     25
#define b_CA_INV_ALL      24

/**
  * @brief  Demonstrate DCache config method
  * @note   none
  * @param  none
  * @retval none
  */
void CacheDemo(void)
{
    //config alg API
    HED_CACHE->SYSCAMASK0 = (0x3FF << 10);  ///< cachesize = 1024KB
    HED_CACHE->SYSCAAREA0 = 0x0000;         ///< user flash start addr
    HED_CACHE->SYSCAMASK1 = (0x7F << 10);   ///< cachesize = 128KB
    HED_CACHE->SYSCAAREA1 = 0x1FF80000;     ///< chip alg API start addr
    HED_CACHE->SYSCAMASK2 = (0x0F << 10);   ///< cachesize = 16KB
    HED_CACHE->SYSCAAREA2 = 0x1FF40000;     ///< NVM API start addr

    HED_CACHE->SYSCACON &= (~(1 << b_CA_EN)); //close
    HED_CACHE->SYSCACON |= ((1 << b_CA_DATA_INV) | (1 << b_CA_INV_ALL)); // delete cache data and cache line

    while(HED_CACHE->SYSCACON & (1 << b_CA_INV_ALL));

    HED_CACHE->SYSCACON |= ((1 << 28) | 0xAA7); ///< enable data cache0~2 SYSCACON
}

/**
  * @brief  Demonstrate Cache config method
  * @note   none
  * @param  none
  * @retval none
  */
void OpenCache(void)
{
    CacheDemo();
}


/*************************************************
  Function:     HED_SetToStandby(void)
  Description:  set chip to standby mode
  Calls:
  Input: none
  Output:none
  Return:
  Others: both SYSCLKCFG and SYSCLKEN remain the same before and after the execution of this API
*************************************************/
#define SPI_IDLE 0x00
#define SPI_BUSY 0x5A
#define I2C_IDLE 0x00
#define SCI_IDLE 0x00

/**
  * @brief  Demonstrate the standby function processing under the condition of
            multi-interface concurrency, by calling SetToStandby_St function to
            achieve.
  */
void HED_SetToStandby_St(void)
{
    closeWrSwitch234();
    /*if run on Develepment Tool's DEBUG MODE,don't enter standby*/
    /*if run on Develepment Tool's RUN MODE,do enter standby*/
    HAL_PMU_EnterLPMode(STANDBY_MODE);//set chip to standby

}

/**
  * @brief  SWP wake up Setting
  * @note   none
  * @param  none
  * @retval none
  */
void ChipSysInit(void)
{
	PMUM->SYSCPUHLDCON |= 0xFFFF;	// CPU hold during algorithm operation
    OpenCache();
}

/**
  * @}
  */

/**
  * @}
  */
