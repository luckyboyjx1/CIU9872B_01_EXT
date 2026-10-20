/************************************************************************************************/
/**
* @file               SCI_Config.h
* @author             Firmware Team
* @brief              SCI configuration related definition and declarations
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

#ifndef _SCI_CONF_H_
#define _SCI_CONF_H_

/* Includes ------------------------------------------------------------------------------------*/
/** @addtogroup APP_Demonstrations
  * @{
  */

/** @addtogroup SCI_Config
  * @{
  */

#define SCI_TDMAE              1
#define SCI_RDMAE              0

#define __HAL_SCI_DMATx_ENABLE()        (SET_REG_BIT(S7816->SCIDMACR,SCI_TDMAE))
#define __HAL_SCI_DMARx_ENABLE()        (SET_REG_BIT(S7816->SCIDMACR,SCI_RDMAE))
#define __HAL_SCI_DMATx_CLOSE()         (CLEAR_REG_BIT(S7816->SCIDMACR,SCI_TDMAE))
#define __HAL_SCI_DMARx_CLOSE()         (CLEAR_REG_BIT(S7816->SCIDMACR,SCI_RDMAE))

//DMAC_CBX bit define
#define DMA_FIN_STA            0x05
#define SCI_DMA_RXLEN          (DMA->DMAC_CA2 & 0xFFFF)
//#define SCI_DMA_RXFIN          (while(SCI_DMA_RXLEN != 0)
#define SCI_DMA_TXLEN          (DMA->DMAC_CA3 & 0xFFFF)

#define __HAL_CLR_SCI_DMARx_STA()  (DMA->DMAC_CB2 &= (~(0x07<<SS)))
#define __HAL_CLR_SCI_DMATx_STA()  (DMA->DMAC_CB3 &= (~(0x07<<SS)))

#define __HAL_OPEN_SCI_RX_INT()  \
{\
    S7816->SCISSR &= ~BIT2;/*clear SCI Rx int flag*/\
    S7816RX_IRQDIS;/*Clear 7816 rec int*/\
    S7816RX_IRQEN;/*enable 7816 rec int*/\
}

//#define SCI_DMA_TXFIN          {while(SCI_DMA_TXLEN != 0);}

#define  HARDWARE_SEND3BH

void GVar_SCI_Init(void);
void SCI_Init(void);
//void SCI_Init2(void);
HAL_StatusTypeDef SCI_DMATransfer_Poll(uint8_t channelNum, uint8_t *buf, uint16_t transLen);

/**
  * @}
  */

/**
  * @}
  */

#endif
