/************************************************************************************************/
/**
* @file               hal_dma.h
* @author             Firmware Team
* @brief              Header file of DMA HAL module.
* @version            V1.0
* @date               2024-01-16
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

#ifndef __HAL_DMA_H_
#define __HAL_DMA_H_

/* Includes ------------------------------------------------------------------------------------*/
#include "CIU9872B_01_V1.1.h"
#include "hal_ckmu.h"

/** @addtogroup Standard_Peripheral
  * @{
  */

/** @addtogroup DMA
  * @{
  */

/* Exported types ------------------------------------------------------------------------------*/
/** @defgroup DMA_Export_Types DMA Export Types
  * @{
  */

/**
  * @brief  DMAC CR register bit enumeration
  */
typedef enum
{
    CL  = 8U,
    PR  = 28U,
    DS  = 30U,
    DE  = 31U
} DMAC_CR_REG_BITS;

/**
  * @brief  DMAC CAx register bit enumeration
  */
typedef enum
{
    TC  = 0U,
    BC  = 16U,
    BT  = 20U,
    IS  = 24U,
    ST  = 29U,
    PB  = 30U,
    EB  = 31U
} DMAC_CAx_REG_BITS;

/**
  * @brief  DMAC CAx register bit enumeration
  */
typedef enum
{
    SS  = 16U,
    CI  = 19U,
    EI  = 20U,
    RD  = 21U,
    RS  = 22U,
    RC  = 23U,
    FD  = 24U,
    FS  = 25U,
    TW  = 26U,
    MS  = 28U
} DMAC_CBx_REG_BITS;

/**
  * @brief  DMA Configuration Structure definition
  */
typedef struct
{
    uint32_t Channel;               /*!< Specifies DMA channel.
                                         This parameter can be a value of @ref DMA_Channel */

    uint32_t ChannelSelect;         /*!< Specifies DMA channel select.*/


    uint32_t Priority;              /*!< Specifies channel priority.
                                         This parameter can be a value of @ref DMA_Priority */

    uint32_t TransBeatType;         /*!< Specifies transfer beat type.
                                         This parameter can be a value of @ref DMA_Trans_Beat_Type */

    uint32_t TransModeType;         /*!< Specifies DMA transfer mode.
                                         This parameter can be a value of @ref DMA_Transfer_Mode */

    uint32_t BlockSizeUnit;         /*!< Specifies DMA BlockSize, BlockSize = (BlockSizeUnit + 1) * TW, TW = 1 or 2 or4。
                                         This parameter can be a value of @ref DMA_Block_Count */

    uint32_t DataWidth;             /*!< Specifies DMA transfer data width.
                                         This parameter can be a value of @ref DMA_Data_width */

    uint32_t Direction;             /*!< Specifies DMA data transfer direction.
                                         This parameter can be a value of @ref DMA_Data_transfer_direction */

} DMA_InitTypeDef;


/**
  * @brief  DMA Handle Structure definition
  */
typedef struct __DMA_HandleTypeDef
{
    DMA_InitTypeDef  Init;      /*!< DMA communication parameter        */
    HAL_LockTypeDef  Lock;      /*!< Locking object                     */
    void (* TransCmpltCallback)(struct __DMA_HandleTypeDef *hdma);  /*!< DMA transf complete callback   */
    void (* TransErrorCallback)(struct __DMA_HandleTypeDef *hdma);  /*!< DMA transfer error callback    */
    void *pUpLevelDataPtr;      /*!< DMA point the upper level handle   */
} DMA_HandleTypeDef;
/**
  * @}
  */

/**
  * @brief DMACH SFR definition
  * @{
  */
typedef struct
{
    __IO uint32_t   DMAC_CA;    /*!< DMAC_CA register, Address offset: 0x0 ~ 0x3 */
    __IO uint32_t   DMAC_CB;    /*!< DMAC_CB register, Address offset: 0x4 ~ 0x7 */
    __IO uint32_t   DMAC_SA;    /*!< DMAC_SA register, Address offset: 0x8 ~ 0xb */
    __IO uint32_t   DMAC_DA;    /*!< DMAC_DA register, Address offset: 0xc ~ 0xf */
} DMACH_SFRTypeDef;
/**
  * @}
  */

/* Exported constants --------------------------------------------------------------------------*/
/** @defgroup DMA_Exported_Constants DMA Exported Constants
  * @{
  */
#define DMACH_BASE      (DMA_BA+0x10)
#define DMACH_STEP      (0x10)
/**
  * @}
  */

/** @defgroup DMA_Channel DMA Channel
  * @{
  */
#define DMA_CHANNEL_SPI1_RX             (0x00000000U)
#define DMA_CHANNEL_SPI1_TX             (0x00000001U)
#define DMA_CHANNEL_I2C_RX              (0x00000002U)
#define DMA_CHANNEL_I2C_TX              (0x00000003U)
#define DMA_CHANNEL_SCI_RX              (0x00000002U)
#define DMA_CHANNEL_SCI_TX              (0x00000003U)
/**
  * @}
  */

/** @defgroup DMA_Channel DMA Select
  * @{
  */
#define DMA_CHANNEL_SELECT_SPI_0_1      (0x00000001U)
#define DMA_CHANNEL_SELECT_I2C_0_1      (0x00000002U)
#define DMA_CHANNEL_SELECT_SCI_0_1      (0x00000003U)
#define DMA_CHANNEL_SELECT_SPI_2_3      (0x00000004U)
#define DMA_CHANNEL_SELECT_I2C_2_3      (0x00000008U)
#define DMA_CHANNEL_SELECT_SCI_2_3      (0x0000000CU)


#define DMA_CHANNEL_0      (0x00000000U)
#define DMA_CHANNEL_1      (0x00000001U)
#define DMA_CHANNEL_2      (0x00000002U)
#define DMA_CHANNEL_3      (0x00000003U)


/**
  * @}
  */

/** @defgroup DMA_Priority DMA Priority
  * @{
  */
#define DMA_PRIORITY_FIX                (0x00000000U)
#define DMA_PRIORITY_CIRCLE             (0x00000001U)
/**
  * @}
  */

/** @defgroup DMA_Trans_Beat_Type DMA Trans Beat Type
  * @{
  */
#define DMA_TRANS_BEAT_TYPE_NORMAL      (0x00000000U)
#define DMA_TRANS_BEAT_TYPE_SINGLE      (0x00000008U)
#define DMA_TRANS_BEAT_TYPE_INCR        (0x00000009U)
#define DMA_TRANS_BEAT_TYPE_WRAP4       (0x0000000AU)
#define DMA_TRANS_BEAT_TYPE_INCR4       (0x0000000BU)
#define DMA_TRANS_BEAT_TYPE_WRAP8       (0x0000000CU)
#define DMA_TRANS_BEAT_TYPE_INCR8       (0x0000000DU)
#define DMA_TRANS_BEAT_TYPE_WRAP16      (0x0000000EU)
#define DMA_TRANS_BEAT_TYPE_INCR16      (0x0000000FU)
/**
  * @}
  */

/** @defgroup DMA_Transfer_Mode DMA Transfer Mode
  * @{
  */
#define DMA_TRANS_MODE_BLOCK            (0x00000000U)
#define DMA_TRANS_MODE_BURST            (0x00000001U)
#define DMA_TRANS_MODE_NEED             (0x00000002U)
/**
  * @}
  */

/** @defgroup DMA_Block_Count DMA Block Count
  * @{
  */
#define DMA_BLOCKSEIZE_0                (0x00000000U)
#define DMA_BLOCKSEIZE_1                (0x00000001U)
#define DMA_BLOCKSEIZE_3                (0x00000003U)
/**
  * @}
  */

/** @defgroup DMA_Data_width DMA Data width
  * @{
  */
#define DMA_DATA_WIDTH_BYTE             (0x00000000U)
#define DMA_DATA_WIDTH_HALFWORD         (0x00000001U)
#define DMA_DATA_WIDTH_WORD             (0x00000002U)
/**
  * @}
  */

/** @defgroup DMA_Data_transfer_direction DMA Data transfer direction
  * @{
  */
#define DMA_MEMORY_TO_PERIPH            (0x00000000U)
#define DMA_PERIPH_TO_MEMORY            (0x00000001U)
#define DMA_MEMORY_TO_MEMORY            (0x00000002U)
/**
  * @}
  */

/**
  * @}
  */

/* Exported Macro ------------------------------------------------------------------------------*/
/** @defgroup DMA_Exported_Macros DMA Exported Macros
  * @{
  */

/** @brief  Open DMA clock.
  * @retval None
  */
#define __HAL_OPEN_DMA_CLKEN()                      SET_REG_BIT(CKMU->SYSAHB2CLKEN, DMACEN)

/** @brief  Close DMA clock.
  * @retval None
  */
#define __HAL_CLOSE_DMA_CLKEN()                     SET_REG_BIT(CKMU->SYSAHB2CLKCLR, DMACOFF)

/** @brief  Open DMA total channel.
  * @retval None
  */
#define __HAL_OPEN_TOTAL_CHANNEL()                  SET_REG_BIT(DMA->DMAC_CR, DE)

/** @brief  Open DMA channel.
  * @param  DMAC_CAx DMA CA register
  * @retval None
  */
#define __HAL_OPEN_DMA_CHX(DMAC_CAx)                SET_REG_BIT(DMAC_CAx, EB)

/** @brief  Close DMA channel.
  * @param  DMAC_CAx DMA CA register
  * @retval None
  */
#define __HAL_CLOSE_DMA_CHX(DMAC_CAx)               CLEAR_REG_BIT(DMAC_CAx, EB)

/** @brief  Disable DMA IRQ.
  * @retval None
  */
#define __HAL_DISABLE_DMA_IRQEN()                   (CPU->VIC_ICER = (1<<IRQ_DMA))
/** @brief  Ensable DMA IRQ.
  * @retval None
  */
#define __HAL_ENABLE_DMA_IRQEN()                    (CPU->VIC_ISER = (1<<IRQ_DMA))

/** @brief  Clear DMA pend IRQ.
  * @retval None
  */
#define __HAL_CLR_DMA_PENDIRQ()                     (CPU->VIC_ICPR = (1<<IRQ_DMA))

/**
  * @}
  */

#define GET_RX_TC(hDMARx)                               ((((DMACH_SFRTypeDef *)(DMACH_STEP * (hDMARx->Init.Channel) + DMACH_BASE))->DMAC_CA) & 0xFFFF)



/* Exported functions --------------------------------------------------------------------------*/
/** @addtogroup DMA_Export_Function DMA Export Function
  * @brief DMA Exported Functions
  * @{
  */

HAL_StatusTypeDef HAL_DMA_Init(DMA_HandleTypeDef *hDMA);
HAL_StatusTypeDef HAL_DMA_DeInit(DMA_HandleTypeDef *hDMA);
HAL_StatusTypeDef HAL_DMA_Start(DMA_HandleTypeDef *hDMA, uint32_t DstAddr, uint32_t SrcAddr, uint32_t BlockCount);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __CIU98M50_HAL_DMA_H_ */
