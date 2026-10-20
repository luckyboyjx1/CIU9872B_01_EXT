/************************************************************************************************/
/**
* @file               hal_crc.c
* @author             Firmware Team
* @brief              crc HAL module driver
* @version            V1.0
* @date               2023-01-16
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
#include "hal_crc.h"

/** @addtogroup Standard_Peripheral
  * @{
  */

/** @addtogroup CRC
  * @{
  */


/* Private types -------------------------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------*/


/** @brief  Calculates the CRC checksum of the specified data int RAM/NVM.
  * @note   @arg The API is not responsible for turning on the CRC32 module gated clock.
  * @param[in]  IV: initial value.
  * @param[in]  DataLen: length of the data to be calculated.
  *                      if this parameter is 0, this function returns the IV value without performing any operation.
  * @param[in]  DataBuf: pointer to data buffer.
  * @param[in]  type: data coding type
  *                   bit31~8: RFU
  *                   bit7~0: encoding type with computational data
  *                           bit7: RFU
  *                           bit6: output data endian conversion
  *                           bit5: input data endian conversion
  *                           bit4: output data bit reversal
  *                           bit3: output data inversion mode: bit
  *                           bit2: input data bit reversal in byte
  *                           bit1: input data inversion mode: bit
  *                           bit0: CRC32 Polynomal
  * @retval 2-byte check code(big-endian format)
  */
uint32_t HED_CRC(uint32_t IV, uint32_t DataLen, uint8_t *DataBuf, uint32_t type)
{
    uint32_t tmp_cnt = (uint32_t)DataBuf & 0x3; //distance to previous word border
    uint32_t div4_quotient;
    uint32_t div4_mod;
	uint32_t clk_bak, ret;

    if(0 == DataLen)
    {
        return IV;
    }

    __disable_irq(); /*note: must call disable_irq. */
	clk_bak = CKMU->SYSAHB2CLKEN;
	CRC_CLK_EN;

    /* Config CRC32CTRL */
    CRC->CRCCTRL = type & 0x7F;

    /* Write InitValue to CRC32RESULT */
    if(CRC_POLY_CRC32 == (type & 1))
    {
        CRC->CRCRESULT = IV;
    }
    else //CRC_POLY_CRC16_CCITT
    {
        CRC->CRCRESULT = (uint16_t)IV;
    }


    if(DataLen <= 0x07)
    {
        div4_quotient = 0;
        div4_mod = DataLen;
    }
    else
    {
        if(tmp_cnt)
        {
            tmp_cnt = 0x4 - tmp_cnt;
            DataLen -= tmp_cnt;

            while(tmp_cnt--)
            {
                *(volatile uint8_t *)&CRC->CRCDATA = *DataBuf++;
            }
        }

        div4_quotient = DataLen >> 0x02;
        div4_mod = DataLen & 0x3;   //word numbers

        if(div4_quotient)
        {
            uint32_t *tmp_databuf = (uint32_t *)DataBuf;

            while(div4_quotient--)
            {
                CRC->CRCDATA = *(uint32_t *)tmp_databuf++;//4 bytes one time
            }

            DataBuf = (uint8_t *)tmp_databuf;
        }
    }

    while(div4_mod)
    {
        *(volatile uint8_t *)&CRC->CRCDATA = *DataBuf++;
        div4_mod--;
    }
	ret = CRC->CRCRESULT;
	
	CRC_CLK_CLR;
	CKMU->SYSAHB2CLKEN = clk_bak;
	__enable_irq();//note:resume

    return ret;
}


/**
  * @}
  */

/**
  * @}
  */
