/************************************************************************************************/
/**
* @file               Protocol_Process.c
* @author             Firmware Team
* @brief              Protocol cos architecture processing demonstration
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
/** @addtogroup APP_Demonstrations
  * @{
  */

/** @addtogroup Protocol_Process
  * @{
  */
#define RF_RxFrame_NOFIN  0x100

/*************************************************
  Function:       Get_APDUResp
  Description:
  Input:          SW1SW2, store sw1 sw2
                  SCIAPDUBuf, store apdu sx data
                  G_APDU_SxLen, store  sx data' length
  Output:         SCIAPDUBuf, store apdu sx data  + sw
                  G_APDU_SxLen, store  sx data+ sw ' length
  Return:         Null
  Others:         Null
*************************************************/
void Get_APDUResp(void)
{
    if(G_bINF == INTERFACE_7816)//7816 interface
    {
        *(G_SendBuf + G_APDU_SxLen) = (uint8_t)(SW1SW2 >> 8);
        *(G_SendBuf + G_APDU_SxLen + 1) = (uint8_t)(SW1SW2 & 0xFF);
        G_APDU_SxLen += 2;
    }
}

/**
  * @brief  Demonstrate SPI/I2C/7816 interface APDU command distribution processing
  * @note   According to the constraint of the chip enhanced stability scheme, when
            calling the openwrswitch2 function and openwrswitch3 function, should
            be separated as far as possible.
  * @param  none
  * @retval none
  */
void APDU_Dispatch(uint32_t interfaceFlag)
{
    SW1SW2 = 0x6D00;//default APDU return is 0x6D00
    G_APDU_SxLen = 0;

    switch(SCIAPDUBuf[1]) //INS
    {
        case    0xF0:
            cmd_Symmetry_ALG();
            break;

        case    0xF2:
            cmd_CRC_Calculate();
            break;

        case    0xF8:
            cmd_HASH_ALG();
            break;

        case    0xFA:
            cmd_ECC_ALG();
            break;

        case    0xC0:
            cmd_getResponse(interfaceFlag);
            break;

        case    0xC4:
            cmd_LOOP_BACK(interfaceFlag);
            break;

        case    0xC6:
            cmd_GET_INFO(interfaceFlag);
            break;

        case    0xD4:
            cmd_back2Loader();
            break;

        case    0xD8:
            cmd_HEFLASH_Operation();
            break;

        default:
            SW1SW2 = 0x6D00;
            break;
    }

    Get_APDUResp();
}

/**
  * @brief  SCI Protocol Process
  * @note   None
  * @param  None
  * @retval None
  */
void SCI_Process(void)
{
#define APDU_NOTIS_FIRSTCMD      0x345678AB

    while(1)
    {
        //HED_SetToStandby_St();//not call set standby API in kit
        #ifndef KIT_DEBUG
#warning "7816 interface enter into low power!"
        HED_SetToStandby_St();//debug时，开发工具不支持低功耗模式，如用开发工具调试时，请将此函数注掉
        //closeWrSwitch234();
        #else
#warning "7816 interface does not enter into low power!"
        closeWrSwitch234();
        #endif
        HED_T0_ReceiveNBytesRam(SCIAPDUBuf, 1);

        if(SCIAPDUBuf[0] == 0xFF)
        {
            PPSExchange();
        }
        else
        {
            openWrSwitch2();
            HED_T0_ReceiveNBytesRam(SCIAPDUBuf + 1, 4);
            openWrSwitch3();

            if(APDU1stTag != APDU_NOTIS_FIRSTCMD)
            {
                APDU1stTag = APDU_NOTIS_FIRSTCMD;

                //  if(((SYSCLKCFG & (0x0F << 16)) >> 16) > CLKSET_MATCH_18Mhz)
                //  {
                //OpenCache();
                //  }

                //Call RAMInit
                if(G_RAMInitFlag == 0)
                {
                    HED_RAMInit();
                    G_RAMInitFlag = 0x5A;
                }

                /* !!!NOTE:  COS must call HED_HEFlash_Init before any operation of writing HEFlash.
                   This apdu is 1st APDU.
                   For example :You can add 'call HED_HEFlash_Init' in here .
                */
                if(PowerdownRecoveryIn1stAPDU() != 0)
                {
                    SW1SW2 = 0x6A80;
                    G_APDU_SxLen = 0;
                    Get_APDUResp();
                    goto SCISendSW1SW2;
                }

                //could Add COS application data 'power down recovery', code in here.
            }

            SW1SW2 = 0x6D00;//default APDU return is 0x6D00
            HED_OpenSend60();
            APDU_Dispatch(INTERFACE_7816);

            HED_CloseSend60();
            //closeWrSwitch();//disable PC_EEPROM operation
SCISendSW1SW2:
            HED_DelayNetu(4);
            HED_T0_SendNBytes(G_SendBuf, G_APDU_SxLen);
        }
    }
}

/**
  * @brief  SPI/I2C/7816 interface cos architecture processing flow demonstration and standby function processing.
  * @note   User can decide whether they need to implement all interfaces according to the actual needs.
  * @param  none
  * @retval none
  */
void ProtocolProcess(void)
{
    if((RSTMU->SYSINFST & BIT4) == BIT4)//RF interface
    {

    }
    else//7816 interface
    {
        SCI_Init();
        SCI_Process();
    }
}

/**
  * @}
  */

/**
  * @}
  */
