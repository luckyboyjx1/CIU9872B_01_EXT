/************************************************************************************************/
/**
* @file               hal_sci.c
* @author             Firmware Team
* @brief              SCI HAL module driver.
* @version            V1.0
* @date               2024-01-05
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
* <tr><td>2024/01/16  <td>V1.0        <td>Firmware Team   <td>First release   </tr>
* </table>
*
**************************************************************************************************
*/
/*------------------------------------------includes-----------------------------------------------*/
#include "hal_sci.h"

/************************************************************************************************/
/**@addtogroup Standard_Peripheral
* @{
*/

/**
* @addtogroup SCI
* @{
*/
/************************************************************************************************/


/*--------------------------------------------Private types---------------------------------------*/
/*--------------------------------------------Private constants----------------------------------------*/
/*--------------------------------------------Private macros-----------------------------------------*/
/*--------------------------------------------Private variables-------------------------------------*/
/*--------------------------------------------Private functions-------------------------------------*/
/*--------------------------------------------Exported functions-------------------------------------*/

/**
  * @brief  sci etu延时接口
  *
  * @param[in]  delay 延时的etu个数
  *
  * @return none
  */
void HED_DelayNetu(uint8_t delay)
{
    do
    {
        S7816->SCISSR |= (1 << 6);

        while(b_SETU_rd != 0);

    } while(delay--);
}
/**
  * @brief  sci 单字符发送接口
  *
  * @param[in]  i 要发送的字符
  *
  * @return none
  */
void HED_T0_SendByte(uint8_t i)
{
    /* this API use 'inquire mode send' */
    S7816->SCISSR = 0x00;

    S7816->SCISBUF = i;

    while(b_TI_rd == 0);

    /* T=0, PE is set to 1 when the data sending error occurs and the number of
       retransmissions reaches the number of automatic retransmissions. in this
       case, PE indicates that the parity check bit of the data sent is incorrect.
       At this time, if the send FIFO is not cleared, and PE is cleared directly,
       the serial port starts to continue to send the data that has not been
       successfully sent before, therefore, SCISSR=0 can not be used directly*/
    /*T＝0时，发送数据出错且重发次数已达到自动重发数时，PE被置为1，
      此时PE表示发送数据的奇偶校验位出错；此时若不清空发送FIFO，
      而直接清零PE，则串口开始继续发送之前未发送成功的数据；
      因此不能用直接用sci->SCISSR = 0;                                      */

    if(b_PE_rd == 0)
    {
        S7816->SCISSR = 0;                            //用于清除Ti
    }
    else
    {
        S7816->SCITXFCR = 0x01;                   //清空发送FIFO

        while((S7816->SCITXFCR & 0x01) == 0x01) {};

        S7816->SCISSR = 0;                            //用于清除Ti\PE
    }
}

/**
* @brief  The T0 protocol send data.
* @note   if a byte is incorrectly sent, the byte is sent a maximum of three times. (Send a maximum of four times.)
* @note   The API is not responsible for turning on the SCI module gate clock.
* @param[in]  DataAddr: Start address for sending data.
* @param[in]  DataLen: The byte length of the sent data.
* @retval 0x00: Data sent successfully.
* @retval 0x01: Data sent failure.
* @retval 0x02: The length of data to be sent is incorrect.
  */
uint32_t HED_T0_SendNBytes(uint8_t *DataAddr, uint32_t DataLen)
{
    if(DataLen == 0)
    {
        return (2);
    }

    //wait for idle time
    while(SCI_IS_BUSY);

    //set auto retry time = 3
    CLEAR_SCI_AUTO_RETRY_LIMIT;
    SET_SCI_AUTO_RETRY_LIMIT(3);

    while(DataLen--)
    {
        //write one byte to buffer a time
        S7816->SCISBUF = (*DataAddr++);

        while(SCI_DATA_IS_SENDING)
        {
            #ifdef INTERRUPT

            if(RSTMU->SYSINTSRC & BIT0)
            {
                return (1);
            }

            #endif
        }

        if(SCI_PARITY_IS_ERROR == 0)
        {
            S7816->SCISSR = 0;                         //clr SCI status
        }
        else
        {
            S7816->SCITXFCR = 0x01;                    //clr trans FIFO control

            while((S7816->SCITXFCR & 0x01) == 0x01);

            S7816->SCISSR = 0;                         //clr SCI status

            return (1);
        }
    }

    return (0);
}

/**
* @brief  The T0 protocol receives data.
* @note   The API is not responsible for turning on the SCI module gate clock.
* @param[out]  DataAddr: Start address for receiving data.
* @param[in]  DataLen: The byte length of the received data.
* @retval none
  */
void HED_T0_ReceiveNBytesRam(uint8_t *DataAddr, uint32_t DataLen)
{
    while(SCI_IS_BUSY);

    while(DataLen--)
    {
        //wait for data
        while(SCIRXFCR_RX_IS_EMPTY);

        (*DataAddr++) = (uint8_t)S7816->SCISBUF;
    }
}

/**
  * @brief  sci 打开自动发送0x60机制
  *
  * @return none
  */
void HED_OpenSend60(void)
{
    SET_REG_BIT(S7816->SCIS60CON, B_SWB);
}
/**
  * @brief  sci 关闭自动发送0x60机制
  *
  * @return none
  */
void HED_CloseSend60(void)
{
    CLEAR_REG_BIT(S7816->SCIS60CON, B_SWB);

    while(b_TB_rd);
}

/**
* @}
*/

/**
* @}
*/
