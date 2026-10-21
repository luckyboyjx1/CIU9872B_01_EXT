/************************ (C) COPYRIGHT  *************************
 * @file                : mhscpu_dma.h
 * @author              : 
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the DMA firmware library
 *****************************************************************************/
#ifndef __MHCARD_DMA_H
#define __MHCARD_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mhcard.h"

#define DMA_CHANNEL_NUM     1

typedef struct
{
    __IO uint32_t SAR_L;
    __IO uint32_t SAR_H;
    __IO uint32_t DAR_L;
    __IO uint32_t DAR_H;
    __IO uint32_t LLP_L;
    __IO uint32_t LLP_H;
    __IO uint32_t CTL_L;
    __IO uint32_t CTL_H;
    __IO uint32_t SSTAT_L;
    __IO uint32_t SSTAT_H;
    __IO uint32_t DSTAT_L;
    __IO uint32_t DSTAT_H;
    __IO uint32_t SSTATAR_L;
    __IO uint32_t SSTATAR_H;
    __IO uint32_t DSTATAR_L;
    __IO uint32_t DSTATAR_H;
    __IO uint32_t CFG_L;
    __IO uint32_t CFG_H;
    __IO uint32_t SGR_L;
    __IO uint32_t SGR_H;
    __IO uint32_t DSR_L;
    __IO uint32_t DSR_H;
} DMA_TypeDef;

typedef struct
{
    DMA_TypeDef DMA_Channel[DMA_CHANNEL_NUM];
    DMA_TypeDef RESERVEDChannel[7];
    
    __I  uint32_t RawTfr_L;
    __I  uint32_t RawTfr_H;
    __I  uint32_t RawBlock_L;
    __I  uint32_t RawBlock_H;
    __I  uint32_t RawSrcTran_L;
    __I  uint32_t RawSrcTran_H;
    __I  uint32_t RawDstTran_L;
    __I  uint32_t RawDstTran_H;
    __I  uint32_t RawErr_L;
    __I  uint32_t RawErr_H;
    
    __I  uint32_t StatusTfr_L;
    __I  uint32_t StatusTfr_H;
    __I  uint32_t StatusBlock_L;
    __I  uint32_t StatusBlock_H;
    __I  uint32_t StatusSrcTran_L;
    __I  uint32_t StatusSrcTran_H;
    __I  uint32_t StatusDstTran_L;
    __I  uint32_t StatusDstTran_H;
    __I  uint32_t StatusErr_L;
    __I  uint32_t StatusErr_H;

    __IO uint32_t MaskTfr_L;
    __IO uint32_t MaskTfr_H;
    __IO uint32_t MaskBlock_L;
    __IO uint32_t MaskBlock_H;
    __IO uint32_t MaskSrcTran_L;
    __IO uint32_t MaskSrcTran_H;
    __IO uint32_t MaskDstTran_L;
    __IO uint32_t MaskDstTran_H;
    __IO uint32_t MaskErr_L;
    __IO uint32_t MaskErr_H;
    
    __O  uint32_t ClearTfr_L;
    __O  uint32_t ClearTfr_H;
    __O  uint32_t ClearBlock_L;
    __O  uint32_t ClearBlock_H;
    __O  uint32_t ClearSrcTran_L;
    __O  uint32_t ClearSrcTran_H;
    __O  uint32_t ClearDstTran_L;
    __O  uint32_t ClearDstTran_H;
    __O  uint32_t ClearErr_L;
    __O  uint32_t ClearErr_H;
    
    __I  uint32_t StatusInt_L;
    __I  uint32_t StatusInt_H;
    
    __IO uint32_t ReqSrcReg_L;
    __IO uint32_t ReqSrcReg_H;
    __IO uint32_t ReqDstReg_L;
    __IO uint32_t ReqDstReg_H;
    __IO uint32_t SglReqSrcReg_L;
    __IO uint32_t SglReqSrcReg_H;
    __IO uint32_t SglReqDstReg_L;
    __IO uint32_t SglReqDstReg_H;
    __IO uint32_t LstSrcReg_L;
    __IO uint32_t LstSrcReg_H;
    __IO uint32_t LstDstReg_L;
    __IO uint32_t LstDstReg_H;
    
    __IO uint32_t DmaCfgReg_L;
    __IO uint32_t DmaCfgReg_H;
    __IO uint32_t ChEnReg_L;
    __IO uint32_t ChEnReg_H;
    __I  uint32_t DmaIdReg_L;
    __I  uint32_t DmaIdReg_H;
    __IO uint32_t DmaTestReg_L;
    __IO uint32_t DmaTestReg_H;
    
    __IO uint32_t RESERVED2[4];
    
    __I  uint32_t DMA_COMP_PARAMS_6_L;
    __I  uint32_t DMA_COMP_PARAMS_6_H;
    __I  uint32_t DMA_COMP_PARAMS_5_L;
    __I  uint32_t DMA_COMP_PARAMS_5_H;
    __I  uint32_t DMA_COMP_PARAMS_4_L;
    __I  uint32_t DMA_COMP_PARAMS_4_H;
    __I  uint32_t DMA_COMP_PARAMS_3_L;
    __I  uint32_t DMA_COMP_PARAMS_3_H;
    __I  uint32_t DMA_COMP_PARAMS_2_L;
    __I  uint32_t DMA_COMP_PARAMS_2_H;
    __I  uint32_t DMA_COMP_PARAMS_1_L;
    __I  uint32_t DMA_COMP_PARAMS_1_H;
    __I  uint32_t DMA_Component_ID_Register_L;
    __I  uint32_t DMA_Component_ID_Register_H;
	
}DMA_MODULE_TypeDef;

#define DMA_BASE                                (MHCARD_AHB_BASE + 0x6000)
#define DMA                                     ((DMA_MODULE_TypeDef *)DMA_BASE)
#define DMA_Channel_0                           ((DMA_TypeDef *)DMA_BASE)

typedef struct
{
  uint32_t DMA_Peripheral;

  uint32_t DMA_PeripheralBaseAddr; /*!< Specifies the peripheral base address for DMAy Channelx. */

  uint32_t DMA_MemoryBaseAddr;     /*!< Specifies the memory base address for DMAy Channelx. */

  uint32_t DMA_DIR;                /*!< Specifies if the peripheral is the source or destination.
                                        This parameter can be a value of @ref DMA_data_transfer_direction */

  uint32_t DMA_PeripheralInc;      /*!< Specifies whether the Peripheral address register is incremented or not.
                                        This parameter can be a value of @ref DMA_incremented_mode */

  uint32_t DMA_MemoryInc;          /*!< Specifies whether the memory address register is incremented or not.
                                        This parameter can be a value of @ref DMA_incremented_mode */

  uint32_t DMA_PeripheralDataSize; /*!< Specifies the Peripheral data item width.
                                        This parameter can be a value of @ref DMA_data_size */

  uint32_t DMA_MemoryDataSize;     /*!< Specifies the Memory data item width.
								   This parameter can be a value of @ref DMA_data_size */

  uint32_t DMA_PeripheralBurstSize; /*!< Specifies the Peripheral Number of data items during per burst transaction.
									read or write from the Peripheral every time a burst transaction request 
									This parameter can be a value of @ref DMA_burst_size */

  uint32_t DMA_MemoryBurstSize;     /*!< Specifies the Memory Number of data items during per burst transaction.
									read or write from the Memory every time a burst transaction request 
									This parameter can be a value of @ref DMA_burst_size */

  uint32_t DMA_PeripheralHandShake;	/*!< Specifies the HandShake to control the DMA transacation.
									This parameter can be a value of @ref DMA_peripheral_handshake */

  uint32_t DMA_BlockSize;			/*!< Specifies the Total Number of data items during the transaction. */

  uint32_t DMA_Priority;           /*!< Specifies the software priority for the DMAy Channelx.
										This parameter can be a value of @ref DMA_priority_level */
}DMA_InitTypeDef;

/**
 * DMA多块传输内存表
 *
 */
typedef struct _lli
{
	uint32_t SAR;
	uint32_t DAR;
	uint32_t LLP;
	uint32_t CTL_L;
	uint32_t CTL_H;
	uint32_t DSTAT;
}LLI;



/** @defgroup DMA_data_transfer_direction 
  * @{
  */
#define DMA_DIR_Memory_To_Memory								((uint32_t)0x0000)
#define DMA_DIR_Peripheral_To_Memory							((uint32_t)0x0001)
#define DMA_DIR_Memory_To_Peripheral							((uint32_t)0x0002)
/**
  * @}
  */

/** @defgroup DMA_incremented_mode 
  * @{
  */

#define DMA_Inc_Increment						((uint32_t)0x00000000)
#define DMA_Inc_Decrement						((uint32_t)0x00000001)
#define DMA_Inc_Nochange						((uint32_t)0x00000002)
#define IS_DMA_INC_STATE(STATE) (((STATE) == DMA_Inc_Increment) || \
                                 ((STATE) == DMA_Inc_Decrement) || \
                                 ((STATE) == DMA_Inc_Nochange))
/**
  * @}
  */

/** @defgroup DMA_data_size 
  * @{
  */

#define DMA_DataSize_Byte					    ((uint32_t)0x0000)
#define DMA_DataSize_HalfWord				    ((uint32_t)0x0001)
#define DMA_DataSize_Word						((uint32_t)0x0002)
#define IS_DMA_DATA_SIZE(SIZE)					(((SIZE) == DMA_DataSize_Byte) || \
												 ((SIZE) == DMA_DataSize_HalfWord) || \
												 ((SIZE) == DMA_DataSize_Word))
/**
  * @}
  */

/** @defgroup DMA_burst_size
  * @{
  */
#define DMA_BurstSize_1							((uint32_t)0x00)
#define DMA_BurstSize_4							((uint32_t)0x01)
#define DMA_BurstSize_8							((uint32_t)0x02)
/**
  * @}
  */

/** @defgroup DMA_peripheral_handshake
  * @{
  */
#define DMA_PeripheralHandShake_Hardware				((uint32_t)0x0000)
#define DMA_PeripheralHandShake_Software				((uint32_t)0x0001)
/**
  * @}
  */

/** @defgroup DMA_Priority
  * @{
  */
#define DMA_Priority_0									((uint32_t)0x00000000)
#define DMA_Priority_1									((uint32_t)0x00000020)
#define DMA_Priority_2									((uint32_t)0x00000040)
#define DMA_Priority_3									((uint32_t)0x00000060)
/**
  * @}
  */

/** @defgroup DMA_IT
  * @{
  */
#define DMA_IT_BlockTransferComplete					((uint32_t)0x01)
#define DMA_IT_DestinationTransactionComplete			((uint32_t)0x02)
#define DMA_IT_Error									((uint32_t)0x04)
#define DMA_IT_SourceTransactionComplete				((uint32_t)0x08)
#define DMA_IT_DMATransferComplete						((uint32_t)0x10)
/**
  * @}
  */

void DMA_Init(DMA_TypeDef* DMA_Channelx, DMA_InitTypeDef* DMA_InitStruct);
void DMA_ChannelCmd(DMA_TypeDef* DMA_Channelx, FunctionalState NewState);
void DMA_Cmd(FunctionalState NewState);

void DMA_SetSRCAddress(DMA_TypeDef* DMA_Channelx, uint32_t Address);
void DMA_SetDSRAddress(DMA_TypeDef* DMA_Channelx, uint32_t Address);
void DMA_SetChannelHardShakePeriph(DMA_TypeDef* DMA_Channelx,uint32_t HardwareHandshake_Peripheral);
	 
void DMA_ITConfig(DMA_TypeDef* DMA_Channelx, uint32_t DMA_IT, FunctionalState NewState);
	 
FlagStatus DMA_GetFlagStatus(uint32_t DMA_FLAG);	 
void DMA_ClearFlag(uint32_t DMA_FLAG);

FunctionalState DMA_IsChannelEnabled(DMA_TypeDef* DMA_Channelx);
	 
ITStatus DMA_GetITStatus(DMA_TypeDef* DMA_Channelx,uint32_t DMA_IT);
FlagStatus DMA_GetRawStatus(DMA_TypeDef* DMA_Channelx,uint32_t DMA_IT);
void DMA_ClearITPendingBit(DMA_TypeDef* DMA_Channelx,uint32_t DMA_IT);

void DMA_InitLLI(DMA_TypeDef* DMA_Channelx,LLI *lli,LLI *next_lli,
							void *src_addr,void *dest_addr,uint16_t btsize);


#ifdef __cplusplus
}
#endif

#endif 
