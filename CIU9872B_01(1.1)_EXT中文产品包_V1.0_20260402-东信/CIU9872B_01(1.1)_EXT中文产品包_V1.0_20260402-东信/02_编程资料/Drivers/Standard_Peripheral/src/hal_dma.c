/************************************************************************************************/
/**
* @file               hal_dma.c
* @author             Firmware Team
* @brief              DMA HAL module driver.
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

/* Includes ------------------------------------------------------------------------------------*/
#include "hal_dma.h"

/** @addtogroup Standard_Peripheral
  * @{
  */

/** @addtogroup DMA
  * @{
  */


/* Private types -------------------------------------------------------------------------------*/
/** @defgroup DMA_Export_Types DMA Export Types-------------------------------------------------*/

/* Private constants ---------------------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------*/

/**
  * @brief  Initialize the DMA according to the specified parameters in the DMA_InitTypeDef
  *         and initialize the associated handle.
  * @param  hDMA Pointer to a DMA_HandleTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DMA_Init(DMA_HandleTypeDef *hDMA)
{
    __IO uint32_t tmpCAx, tmpCBx;

    DMACH_SFRTypeDef *pSfrDMACH;

    /* Check the DMA handle */
    if(NULL == hDMA)
    {
        return HAL_ERROR;
    }

    /* Allocate lock resource and initialize it */
    __HAL_UNLOCK(hDMA);

    /* Open DMA CLK */
    if(REG_BIT_IS_CLEAR(CKMU->SYSAHB2CLKEN, DMACEN))
    {
        __HAL_OPEN_DMA_CLKEN();
    }

    /* Open total tuel */
    if(REG_BIT_IS_CLEAR(DMA->DMAC_CR, DE))
    {
        __HAL_OPEN_TOTAL_CHANNEL();
    }

    pSfrDMACH = (DMACH_SFRTypeDef *)(DMACH_STEP * (hDMA->Init.Channel) + DMACH_BASE);

//    /* Disable channel x */
//    __HAL_CLOSE_DMA_CHX(pSfrDMACH->DMAC_CA);

    if(hDMA->Init.Channel > 1)
    {
        DMA->DMAC_CR |= ((hDMA->Init.ChannelSelect & 0x0C) << CL);
    }
    else
    {
        DMA->DMAC_CR |= ((hDMA->Init.ChannelSelect & 0x03) << CL);
    }

    /* Config channel priority */
    DMA->DMAC_CR |= ((hDMA->Init.Priority & BIT0) << PR);

    /* Config channelx DMAC_CAX value */
    tmpCAx = 0; // Disable channel defaultly.
    tmpCAx = (((hDMA->Init.TransBeatType & 0x0F) << BT) | ((hDMA->Init.BlockSizeUnit & 0x0F) << BC));

    /* Config channelx DMAC_CBX value */
    tmpCBx = 0;
    tmpCBx = (((hDMA->Init.TransModeType & 0x03) << MS) | ((hDMA->Init.DataWidth & 0x03) << TW));

    if(DMA_MEMORY_TO_PERIPH == hDMA->Init.Direction)
    {
        /* Fix dst addr, inc src addr */
        tmpCBx |= (1 << FD);
        /* Config trigger req, DREQ high level or rising edge. */
        tmpCAx |= (0x0E << IS);
    }
    else if(DMA_PERIPH_TO_MEMORY == hDMA->Init.Direction)
    {
        /* Fix src addr, inc Dst addr */
        tmpCBx |= (1 << FS);
        /* Config trigger req, DREQ high level or rising edge. */
        tmpCAx |= (0x0E << IS);
    }
    else // DMA_MEMORY_TO_MEMORY, INC src addr, INC Dst addr
    {
        /* Config trigger req, soft trigger */
        tmpCAx |= (1 << ST);
    }

    /* Config channelx DMAC_CAX value */
    pSfrDMACH->DMAC_CA = tmpCAx;
    pSfrDMACH->DMAC_CB = tmpCBx;

    return HAL_OK;
}

/**
  * @brief  DeInitialize the DMA peripheral.
  * @param  hDMA Pointer to a DMA_HandleTypeDef structure.
  * @return HAL status
  */
HAL_StatusTypeDef HAL_DMA_DeInit(DMA_HandleTypeDef *hDMA)
{
    DMACH_SFRTypeDef *pSfrDMACH;

    /* Check the DMA handle allocation */
    if(NULL == hDMA)
    {
        return HAL_ERROR;
    }

    pSfrDMACH = (DMACH_SFRTypeDef *)(DMACH_STEP * (hDMA->Init.Channel) + DMACH_BASE);

    /* Open DMA CLK */
    if(REG_BIT_IS_CLEAR(CKMU->SYSAHB2CLKEN, DMACEN))
    {
        __HAL_OPEN_DMA_CLKEN();
    }

    /* Reset DMA IP */
    DMA->DMAC_CR             = 1 << DS;
    pSfrDMACH->DMAC_CA  = 0x0E << IS;
    pSfrDMACH->DMAC_CB  = 0;
    pSfrDMACH->DMAC_SA  = 0;
    pSfrDMACH->DMAC_DA  = 0;

    __HAL_DISABLE_DMA_IRQEN();
    __HAL_CLR_DMA_PENDIRQ();
    __HAL_CLOSE_DMA_CLKEN();
    __HAL_UNLOCK(hDMA);

    return HAL_OK;
}

/**
  * @brief  Sets the DMA Transfer parameter.
  * @param  hDMA Pointer to a DMA_HandleTypeDef structure.
  * @param  DstAddr The destination memory Buffer address
  * @param  SrcAddr The source memory Buffer address
  * @param  BlockCount The length of data to be transferred, Transfer total length = BlockCount × BlockSize(BlockSize refer to definition BlockSizeUnit)
  * @return HAL status
  */
static void DMA_SetConfig(DMA_HandleTypeDef *hDMA, uint32_t DstAddr, uint32_t SrcAddr, uint32_t BlockCount)
{
    __IO uint32_t tmpSFR;
    DMACH_SFRTypeDef *pSfrDMACH;

    pSfrDMACH = (DMACH_SFRTypeDef *)(DMACH_STEP * (hDMA->Init.Channel) + DMACH_BASE);

    /* Disable channel x */
    __HAL_CLOSE_DMA_CHX(pSfrDMACH->DMAC_CA);

    /* Config src addr */
    pSfrDMACH->DMAC_SA = SrcAddr;
    pSfrDMACH->DMAC_DA = DstAddr;

    tmpSFR = (pSfrDMACH->DMAC_CA & 0xFFFF0000);
    tmpSFR |= ((BlockCount & 0xFFFF) << TC);

    if(DMA_MEMORY_TO_MEMORY == hDMA->Init.Direction)
    {
        /* Config trigger req, soft trigger */
        tmpSFR |= (1 << ST);
    }

    pSfrDMACH->DMAC_CA = tmpSFR;

    /* Must clear the status flag before DMA transfer */
    pSfrDMACH->DMAC_CB &= (~(0x07 << SS));

    /* Open ChannelX tuel */
    __HAL_OPEN_DMA_CHX(pSfrDMACH->DMAC_CA);
}

/**
  * @brief  Start DMA transfers based on DMA handle configuration information.
  * @param  hDMA Pointer to a DMA_HandleTypeDef structure.
  * @param  DstAddr The destination memory Buffer address
  * @param  SrcAddr The source memory Buffer address
  * @param  BlockCount The length of data to be transferred, Transfer total length = BlockCount × BlockSize(BlockSize refer to definition BlockSizeUnit)
  * @return HAL status
  */
HAL_StatusTypeDef HAL_DMA_Start(DMA_HandleTypeDef *hDMA, uint32_t DstAddr, uint32_t SrcAddr, uint32_t BlockCount)
{
    /* Check the DMA handle */
    if((NULL == hDMA) || (0 == BlockCount))
    {
        return HAL_ERROR;
    }

    //__HAL_LOCK(hDMA);

    DMA_SetConfig(hDMA, DstAddr, SrcAddr, BlockCount);

    //__HAL_UNLOCK(hDMA);
    return HAL_OK;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
