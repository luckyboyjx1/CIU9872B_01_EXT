/************************************************************************************************/
/**
* @file               SCI_Protocol.c
* @author             Firmware Team
* @brief              SCI protocol stack processing demonstration
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

/*------------------------------------------includes-----------------------------------------------*/
#include "Declare.h"

/************************************************************************************************/
/**@addtogroup APP_Demonstrations
* @{
*/

/** @addtogroup SCI_Protocol
  * @{
  */

/**
  * @brief  SCI channel ATR send
  * @note   APP Demo supports concurrency between SCI and other interfaces, so ATR
            sending needs to be processed in interrupt while the hardware sending
            3B function is enabled
  * @param  none
  * @retval none
  */
void SCI_Send_ATR(void)
{
    uint32_t i;
    uint8_t tck;

    #ifdef HARDWARE_SEND3BH

    HED_T0_SendNBytes((uint8_t *)&G_ATR[2], G_ATR[0] - 2);

    #else
    HED_DelayNetu(3);//delay 6*372 = 2232 clk
    HED_T0_SendNBytes((uint8_t *)&G_ATR[1], 1); //0x3B sent by COS

    HED_DelayNetu(4);//delay 6*372 = 2232 clk
    HED_T0_SendNBytes((uint8_t *)&G_ATR[2], G_ATR[0] - 2);

    #endif
    RSTMU->SYSRSTFLAG = 0;
}

/**
  * @brief  SCI channel IP config before receive data.
  * @note   none
  * @param  none
  * @retval none
  */
void SCI_RxStart(void)
{
    //while(SCISSR & BIT5);//wait if sending

    while(S7816->SCISSR & BIT3);  //wait if receiving

    S7816->SCIRXFCR = 0x100;//set waterLevel to 1 byte

    __HAL_OPEN_SCI_RX_INT();
}

/**
  * @brief  SCI channel sends INS
  * @note   none
  * @param  ins：The INS value which is to be send.
  * @retval none
  */
void SendINS(uint8_t ins)
{
    HED_DelayNetu(4);
    HED_T0_SendByte(ins);//send INS
}


#define     PPS_NUM     5

//static const  uint32_t   FD_table[PPS_NUM][3] = {{0x97,0x00070007,0x78},{0x11,0x00010173,0xfe},{0x18,0x0008001E,0xf7},
//                                             {0x94,0x0004003F,0x7B},{0x96,0x0006000F,0x79}}; //{FD,SFR_Value,xor_value}
static const uint32_t FD_table[PPS_NUM][3] =
{
    {0x97, (64 << 11ul) | 7,   0x78},
    {0x11, (1  << 11ul) | 371, 0xFE},
    {0x18, (12 << 11ul) | 30,  0xF7},
    {0x94, (8  << 11ul) | 63,  0x7B},
    {0x96, (32 << 11ul) | 15,  0x79}
}; //{FD,SFR_Value,xor_value}

#ifdef SCI_DMA_EN
/**
  * @brief  SCI channel handing PPS
  * @note   The speed of demo APP is 0x11,0x18,0x94,0x96,0x97,and the rest speed
            can be realized according to the needs of the user.
  * @param  none
  * @retval none
  */
uint8_t PPSExchange(void)
{
#define PPS_PPSS SCIAPDUBuf[0]
#define PPS_PPS0 SCIAPDUBuf[1]
#define PPS_PPS1 SCIAPDUBuf[2]
#define PPS_PPS2 SCIAPDUBuf[3]
#define PPS_PPS3 SCIAPDUBuf[4]
#define PPS_PCK  SCIAPDUBuf[5]
#define PPS_NUM 5

    register uint32_t len, pck;
    register uint32_t i;

    PPS_PPSS = 0xff;
    len = 1;

    for(i = 4; i < 7; i++)
    {
        if(((PPS_PPS0 >> i) & 1) == 1)
        {
            len++;
        }
    }

    if(G_SCIRecieveIndex != (len + 2))  //wait pps data all received
    {
        return false;
    }

    pck = 0x00;

    for(i = 0; i < (len + 2); i++)
    {
        pck = pck ^ SCIAPDUBuf[i];
    }

    if(pck == 0)
    {
        HED_DelayNetu(4);

        if((PPS_PPS0 & 0x9F) == 0x10)
        {
            /* find FD */
            for(i = 0; i < PPS_NUM; i++)
            {
                if(PPS_PPS1 == FD_table[i][0])
                {
                    /* send pps response */
                    PPS_PPS0 = 0x10;
                    SCIAPDUBuf[3] = FD_table[i][2];
                    HED_T0_SendNBytes(SCIAPDUBuf, 4);

                    SetBaudRate(FD_table[i][1]);
                    return true;
                }
            }
        }

        /* send pps err response */
        SCIAPDUBuf[1] = 0;
        SCIAPDUBuf[2] = 0xff;
        HED_T0_SendNBytes(SCIAPDUBuf, 3);
    }
    else
    {
        /*means pps pck is err ,do nothing */
    }

    return true;
}

/**
  * @brief  SCI Case command data reception processing
  * @note   In order to support multi-interface well generation, SCI must adopt
            DMA channel to receive APDU data block.
  * @param  none
  * @retval none
  */
void SCI_RxCase3Data(void)
{
    S7816RX_IRQDIS;//Clear 7816 rec int

    //  SendINS(APDU_INS);

    /* Turn on SCI receive channel DMA transmission */
    SCI_DMATransfer_Poll(hDMA_RxStVar.Init.Channel, (APDUBuf + 5), APDU_P3);

    __HAL_OPEN_SCI_RX_INT();

}


/**
  * @brief  SCI send data processing
  * @note   In order to support multi-interface well generation, SCI must adopt
            DMA channel to send APDU data block.
  * @param  none
  * @retval none
  */
void SCI_SendAPPData(void)
{
    /* Turn on SCI send channel DMA transmission */
    SCI_DMATransfer_Poll(hDMA_TxStVar.Init.Channel, APDUBuf, (LeLength + 2));
}
#else
uint8_t PPSExchange(void)
{
#define PPS_PPSS G_SendBuf[0]
#define PPS_PPS0 G_SendBuf[1]
#define PPS_PPS1 G_SendBuf[2]
#define PPS_PPS2 G_SendBuf[3]
#define PPS_PPS3 G_SendBuf[4]
#define PPS_PCK  G_SendBuf[5]
#define PPS_DATA G_SendBuf
#define PPS_NUM 5
    /* IN: 已经收到了PPSS字节  */

    register uint8_t len, pck;
    register uint8_t i;

    /*recevie PPS0 */
    PPS_PPSS = 0xff;
    HED_T0_ReceiveNBytesRam(&PPS_PPS0, 1);

    len = 1;

    for(i = 4; i < 7; i++)
    {
        if(((PPS_PPS0 >> i) & 1) == 1)
        {
            len++;
        }
    }

    HED_T0_ReceiveNBytesRam(&PPS_PPS1, len++);

    /*get PCK */
    pck = 0xFF;

    for(i = 1; i <= len; i++)  //len = pps request 总长度-1
    {
        pck = pck ^ PPS_DATA[i];
    }

    if(pck == 0)
    {
        HED_DelayNetu(5);

        if((PPS_PPS0 & 0x9F) == 0x10)
        {
            /* find FD */
            for(i = 0; i < PPS_NUM; i++)
            {
                if(PPS_PPS1 == FD_table[i][0])
                {
                    /* send pps response */
                    PPS_PPS0 = 0x10;
                    PPS_PPS2 = FD_table[i][2];
                    HED_T0_SendNBytes(G_SendBuf, 4);

                    SetBaudRate(FD_table[i][1]);
                    return true;
                }
            }
        }

        /* send pps err response */
        G_SendBuf[1] = 0;
        G_SendBuf[2] = 0xff;
        HED_T0_SendNBytes(G_SendBuf, 3);
    }
    else
    {
        /*means pps pck is err ,do nothing */
    }

    return true;
}
#endif  ///< SCI_DMA_EN
/**
* @}
*/

/**
* @}
*/
