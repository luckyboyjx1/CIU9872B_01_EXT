/************************************************************************************************/
/**
* @file               hal_sci.h
* @author             Firmware Team
* @brief              Header file of SCI HAL module.
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
#ifndef __HAL_SCI_H_
#define __HAL_SCI_H_

/*------------------------------------------includes-----------------------------------------------*/
#include "CIU9872B_01_V1.1.h"

/************************************************************************************************/
/**
* @addtogroup Standard_Peripheral
* @{
*/

/**
* @addtogroup SCI
* @{
*/
/************************************************************************************************/

/*-----------------------------------------Exported types------------------------------------------*/

#define     b_SETU_rd           (S7816->SCISSR & (1<<6))
#define     b_TB_rd             (S7816->SCISSR & (1<<5))
#define     b_TI_rd             (S7816->SCISSR & (1<<4))
#define     b_RB_rd             (S7816->SCISSR & (1<<3))
#define     b_RI_rd             (S7816->SCISSR & (1<<2))
#define     b_PE_rd             (S7816->SCISSR & (1<<1))
/**
* @brief SCI SCISCON SFR
*/
typedef enum
{
    SCISCON_RCNT            = 0U,
    SCISCON_PARITY          = 2U,
    SCISCON_IVC             = 3U,
    SCISCON_EGB             = 4U,
    SCISCON_PS              = 5U,
    SCISCON_SE              = 6U,
    SCISCON_AWTS            = 8U,
    SCISCON_SAFB            = 10U,
    SCISCON_RTS             = 11U
} SCISCON_REG_BITS;

/**
* @brief SCI SSR register bit enumeration
*/
typedef enum
{
    SCISSR_OE               = 0U,
    SCISSR_PE               = 1U,
    SCISSR_RI               = 2U,
    SCISSR_RB               = 3U,
    SCISSR_TI               = 4U,
    SCISSR_TB               = 5U,
    SCISSR_SETU             = 6U
} SCISSR_REG_BITS;

/**
  * @brief  SCI TXFCR register bit enumeration
  */
typedef enum
{
    SCITXFCR_FLUSH_TX_FIFO  = 0U,
    SCITXFCR_TX_FULL        = 1U
} SCITXFCR_REG_BITS;

/**
  * @brief  SCI RXFCR register bit enumeration
  */
typedef enum
{
    SCIRXFCR_FLUSH_RX_FIFO  = 0U,
    SCIRXFCR_RX_NOEMPTY     = 1U,
    SCIRXFCR_WATER_LEVEL    = 8U
} SCIRXFCR_REG_BITS;

/**
  * @brief  SCI DMACR register bit enumeration
  */
typedef enum
{
    SCIDMACR_RDMAE          = 0U,
    SCIDMACR_TDMAE          = 1U
} SCIDMACR_REG_BITS;


/*--------------------------------------------Exported constants----------------------------------------*/
/*--------------------------------------------Exported macros----------------------------------------*/
/* MACRO DEFINE */
/* 7816 Interface */
#define SCI_IS_BUSY                                 (S7816->SCISSR & ((1 << SCISSR_RB)|(1 << SCISSR_TB)))
#define CLEAR_SCI_AUTO_RETRY_LIMIT                  (S7816->SCISCON &= ~(3 << SCISCON_RCNT))
#define SET_SCI_AUTO_RETRY_LIMIT(num)               (S7816->SCISCON |= ((num - 2) & 0x3))

/*7816 send data*/
#define SCI_DATA_IS_SENDING                         REG_BIT_IS_CLEAR(S7816->SCISSR, SCISSR_TI)
#define SCI_PARITY_IS_ERROR                         REG_BIT_IS_SET(S7816->SCISSR, SCISSR_PE)

/*7816 receive data*/
#define SCIRXFCR_RX_IS_EMPTY                        REG_BIT_IS_CLEAR(S7816->SCIRXFCR, SCIRXFCR_RX_NOEMPTY)

/*SCIS60CON bit define*/
#define     B_SWB               0

/*--------------------------------------------Exported functions-------------------------------------*/
void HED_DelayNetu(uint8_t delay);
void HED_OpenSend60(void);
void HED_CloseSend60(void);
void HED_T0_SendByte(uint8_t i);
uint32_t HED_T0_SendNBytes(uint8_t *DataAddr, uint32_t DataLen);
void HED_T0_ReceiveNBytesRam(uint8_t *DataAddr, uint32_t DataLen);

/**
* @}
*/

/**
* @}
*/
#endif  /* __HAL_SCI_H_ */
