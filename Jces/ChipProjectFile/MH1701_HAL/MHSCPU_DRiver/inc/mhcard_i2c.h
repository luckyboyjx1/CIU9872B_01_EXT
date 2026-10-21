/************************ (C) COPYRIGHT *************************
 * @file                : mhcard_i2c.h
 * @author              :
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the I2C firmware library
 *****************************************************************************/

#ifndef __MHCARD_I2C_H
#define __MHCARD_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhcard.h"

typedef struct
{
    __IO uint32_t IC_CON;
    __IO uint32_t IC_TAR;
    __IO uint32_t IC_SAR;
    __IO uint32_t IC_HS_MADDR;
    __IO uint32_t IC_DATA_CMD;
    __IO uint32_t IC_SS_SCL_HCNT;
    __IO uint32_t IC_SS_SCL_LCNT;
    __IO uint32_t IC_FS_SCL_HCNT;
    __IO uint32_t IC_FS_SCL_LCNT;
    __IO uint32_t IC_HS_SCL_HCNT;
    __IO uint32_t IC_HS_SCL_LCNT;
    __I  uint32_t IC_INTR_STAT;
    __IO uint32_t IC_INTR_MASK;
    __I  uint32_t IC_RAW_INTR_STAT;
    __IO uint32_t IC_RX_TL;
    __IO uint32_t IC_TX_TL;
    __I  uint32_t IC_CLR_INTR;
    __I  uint32_t IC_CLR_RX_UNDER;
    __I  uint32_t IC_CLR_RX_OVER;
    __I  uint32_t IC_CLR_TX_OVER;
    __I  uint32_t IC_CLR_RD_REQ;
    __I  uint32_t IC_CLR_TX_ABRT;
    __I  uint32_t IC_CLR_RX_DONE;
    __I  uint32_t IC_CLR_ACTIVITY;
    __I  uint32_t IC_CLR_STOP_DET;
    __I  uint32_t IC_CLR_START_DET;
    __I  uint32_t IC_CLR_GEN_CALL;
    __IO uint32_t IC_ENABLE;
    __I  uint32_t IC_STATUS;
    __I  uint32_t IC_TXFLR;
    __I  uint32_t IC_RXFLR;
    __IO uint32_t IC_SDA_HOLD;
    __I  uint32_t IC_TX_ABRT_SOURCE;
    __IO uint32_t IC_SLV_DATA_NACK_ONLY;
    __IO uint32_t IC_DMA_CR;
    __IO uint32_t IC_DMA_TDLR;
    __IO uint32_t IC_DMA_RDLR;
    __IO uint32_t IC_SDA_SETUP;
    __IO uint32_t IC_ACK_GENERAL_CALL;
    __I  uint32_t IC_ENABLE_STATUS;
    __IO uint32_t IC_FS_SPKLEN;
    __IO uint32_t IC_HS_SPKLEN; 
} I2C_TypeDef;

#define I2C_BASE                               (MHCARD_APB_BASE + 0x7000)
#define I2C                                    ((I2C_TypeDef *)I2C_BASE)

/** @defgroup I2C_mode 
  * @{
  */
typedef enum
{
	I2C_Mode_Master					=((uint32_t)0x0001),
	I2C_Mode_Slave					=((uint32_t)0x0002)
}I2CMode_TypeDef;

#define IS_I2C_MODE(MODE)			(((MODE) == I2C_Mode_Master) || \
									((MODE) == I2C_Mode_Slave))
/**
  * @}
  */	 
	 
typedef struct
{
	uint32_t I2C_ClockSpeed;			/*!< Specifies the clock frequency. */

	I2CMode_TypeDef I2C_Mode;          

	uint32_t I2C_DutyCycle;				/*!< Specifies the I2C fast mode duty cycle.
                                         This parameter can be a value of @ref I2C_duty_cycle_in_fast_mode */
	uint32_t I2C_SDASetupTime;

	uint32_t I2C_SDAHoldTime;

	uint32_t I2C_TargetAddress;

	uint32_t I2C_TargetAddressMode;

	uint32_t I2C_OwnAddress;			/*!< Specifies the slave mode own address.
                                         This parameter can be a 7-bit or 10-bit address. */

	uint32_t I2C_AcknowledgedAddress;	/*!< Specifies if 7-bit or 10-bit address is acknowledged.
                                         This parameter can be a value of @ref I2C_acknowledged_address */

	uint32_t I2C_RXFIFOFullThreshold;

	uint32_t I2C_TXFIFOEmptyThreshold;

	FunctionalState I2C_GenerateRestartEnable;
}I2C_InitTypeDef;


typedef struct
{
	uint32_t I2C_DMAReq;
	uint32_t I2C_DMAReceiveLevel;
	uint32_t I2C_DMATransmitLevel;
	FunctionalState I2C_DMAEnCmd;
}I2C_DMAInitTypeDef;


#define I2C_ClockSpeed_100KHz					(100000)
#define I2C_ClockSpeed_400KHz					(400000)


/******************************************************************************/
/*                                                                            */
/*                      Inter-integrated Circuit Interface                    */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for IC_CON register  *********************/
#define I2C_IC_CON_MASTER_MODE                  ((uint32_t)0x0001)
#define I2C_IC_CON_SPEED                        ((uint32_t)0x0006)
#define I2C_IC_CON_SPEED_0                      ((uint32_t)0x0002)
#define I2C_IC_CON_SPEED_1                      ((uint32_t)0x0004)
#define I2C_IC_CON_10BITADDR_SLAVE              ((uint32_t)0x0008)
#define I2C_IC_CON_10BITADDR_MASTER             ((uint32_t)0x0010)
#define I2C_IC_CON_RESTART_EN                   ((uint32_t)0x0020)
#define I2C_IC_CON_SLAVE_DISABLE                ((uint32_t)0x0040)

/*******************  Bit definition for IC_TAR register  *********************/
#define I2C_IC_TAR_TAR                          ((uint32_t)0x03FF)
#define I2C_IC_TAR_GC_OR_START                  ((uint32_t)0x0400)
#define I2C_IC_TAR_SPECIAL                      ((uint32_t)0x0800)
#define I2C_IC_TAR_10BITADDR_MASTER             ((uint32_t)0x1000)

/*******************  Bit definition for IC_SAR register  *********************/
#define I2C_IC_SAR_SAR                          ((uint32_t)0x03FF)

/*******************  Bit definition for IC_HS_MADDR register  ****************/
#define I2C_IC_HS_MADDR_MAR                     ((uint32_t)0x0007)
#define I2C_IC_HS_MADDR_MAR_0                   ((uint32_t)0x0001)
#define I2C_IC_HS_MADDR_MAR_1                   ((uint32_t)0x0002)
#define I2C_IC_HS_MADDR_MAR_2                   ((uint32_t)0x0004)

/*******************  Bit definition for IC_DATA_CMD register  ****************/
#define I2C_IC_DATA_CMD_DAT                     ((uint32_t)0x00FF)
#define I2C_IC_DATA_CMD_CMD                     ((uint32_t)0x0100)
#define I2C_IC_DATA_CMD_STOP                    ((uint32_t)0x0200)
#define I2C_IC_DATA_CMD_RESTART                 ((uint32_t)0x0400)

/*******************  Bit definition for IC_SS_SCL_HCNT register  *************/
#define I2C_IC_SS_SCL_HCNT_HCNT                     ((uint32_t)0xFFFF)

/*******************  Bit definition for IC_SS_SCL_LCNT register  *************/
#define I2C_IC_SS_SCL_LCNT_LCNT                     ((uint32_t)0xFFFF)

/*******************  Bit definition for IC_FS_SCL_HCNT register  *************/
#define I2C_IC_FS_SCL_HCNT_HCNT                     ((uint32_t)0xFFFF)

/*******************  Bit definition for IC_FS_SCL_LCNT register  *************/
#define I2C_IC_FS_SCL_LCNT_LCNT                     ((uint32_t)0xFFFF)

/*******************  Bit definition for IC_HS_SCL_HCNT register  *************/
#define I2C_IC_HS_SCL_HCNT_HCNT                     ((uint32_t)0xFFFF)

/*******************  Bit definition for IC_HS_SCL_LCNT register  *************/
#define I2C_IC_HS_SCL_LCNT_LCNT                     ((uint32_t)0xFFFF)

/*******************  Bit definition for IC_INTR_STAT register  ***************/
#define I2C_IC_INTR_STAT_R_RX_UNDER                 ((uint32_t)0x0001)
#define I2C_IC_INTR_STAT_R_RX_OVER                  ((uint32_t)0x0002)
#define I2C_IC_INTR_STAT_R_RX_FULL                  ((uint32_t)0x0004)
#define I2C_IC_INTR_STAT_R_TX_OVER                  ((uint32_t)0x0008)
#define I2C_IC_INTR_STAT_R_TX_EMPTY                 ((uint32_t)0x0010)
#define I2C_IC_INTR_STAT_R_RD_REQ                   ((uint32_t)0x0020)
#define I2C_IC_INTR_STAT_R_TX_ABRT                  ((uint32_t)0x0040)
#define I2C_IC_INTR_STAT_R_RX_DONE                  ((uint32_t)0x0080)
#define I2C_IC_INTR_STAT_R_ACTIVITY                 ((uint32_t)0x0100)
#define I2C_IC_INTR_STAT_R_STOP_DET                 ((uint32_t)0x0200)
#define I2C_IC_INTR_STAT_R_START_DET                ((uint32_t)0x0400)
#define I2C_IC_INTR_STAT_R_GEN_CALL                 ((uint32_t)0x0800)

/*******************  Bit definition for IC_INTR_MASK register  ***************/
#define I2C_IC_INTR_MASK_M_RX_UNDER                 ((uint32_t)0x0001)
#define I2C_IC_INTR_MASK_M_RX_OVER                  ((uint32_t)0x0002)
#define I2C_IC_INTR_MASK_M_RX_FULL                  ((uint32_t)0x0004)
#define I2C_IC_INTR_MASK_M_TX_OVER                  ((uint32_t)0x0008)
#define I2C_IC_INTR_MASK_M_TX_EMPTY                 ((uint32_t)0x0010)
#define I2C_IC_INTR_MASK_M_RD_REQ                   ((uint32_t)0x0020)
#define I2C_IC_INTR_MASK_M_TX_ABRT                  ((uint32_t)0x0040)
#define I2C_IC_INTR_MASK_M_RX_DONE                  ((uint32_t)0x0080)
#define I2C_IC_INTR_MASK_M_ACTIVITY                 ((uint32_t)0x0100)
#define I2C_IC_INTR_MASK_M_STOP_DET                 ((uint32_t)0x0200)
#define I2C_IC_INTR_MASK_M_START_DET                ((uint32_t)0x0400)
#define I2C_IC_INTR_MASK_M_GEN_CALL                 ((uint32_t)0x0800)

/*******************  Bit definition for IC_RAW_INTR_STAT register  ***********/
#define I2C_IC_RAW_INTR_STAT_RX_UNDER               ((uint32_t)0x0001)
#define I2C_IC_RAW_INTR_STAT_RX_OVER                ((uint32_t)0x0002)
#define I2C_IC_RAW_INTR_STAT_RX_FULL                ((uint32_t)0x0004)
#define I2C_IC_RAW_INTR_STAT_TX_OVER                ((uint32_t)0x0008)
#define I2C_IC_RAW_INTR_STAT_TX_EMPTY               ((uint32_t)0x0010)
#define I2C_IC_RAW_INTR_STAT_RD_REQ                 ((uint32_t)0x0020)
#define I2C_IC_RAW_INTR_STAT_TX_ABRT                ((uint32_t)0x0040)
#define I2C_IC_RAW_INTR_STAT_RX_DONE                ((uint32_t)0x0080)
#define I2C_IC_RAW_INTR_STAT_ACTIVITY               ((uint32_t)0x0100)
#define I2C_IC_RAW_INTR_STAT_STOP_DET               ((uint32_t)0x0200)
#define I2C_IC_RAW_INTR_STAT_START_DET              ((uint32_t)0x0400)
#define I2C_IC_RAW_INTR_STAT_GEN_CALL               ((uint32_t)0x0800)

/*******************  Bit definition for IC_RX_TL register  *******************/
#define I2C_IC_RX_TL_TL                             ((uint32_t)0x00FF)

/*******************  Bit definition for IC_TX_TL register  *******************/
#define I2C_IC_TX_TL_TL                             ((uint32_t)0x00FF)

/*******************  Bit definition for IC_CLR_INTR register  ****************/
#define I2C_IC_CLR_INTR                             ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_RX_UNDER register  ************/
#define I2C_IC_CLR_RX_UNDER                         ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_RX_OVER register  *************/
#define I2C_IC_CLR_RX_OVER                          ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_TX_OVER register  *************/
#define I2C_IC_CLR_TX_OVER                          ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_RD_REQ register  **************/
#define I2C_IC_CLR_RD_REQ                           ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_TX_ABRT register  *************/
#define I2C_IC_CLR_TX_ABRT                          ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_RX_DONE register  *************/
#define I2C_IC_CLR_RX_DONE                          ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_ACTIVITY register  ************/
#define I2C_IC_CLR_ACTIVITY                         ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_STOP_DET register  ************/
#define I2C_IC_CLR_STOP_DET                         ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_START_DET register  ***********/
#define I2C_IC_CLR_START_DET                        ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_GEN_CALL register  ************/
#define I2C_IC_CLR_GEN_CALL                         ((uint32_t)0x0001)

/*******************  Bit definition for IC_ENABLE  register  *****************/
#define I2C_IC_ENABLE_ENABLE                        ((uint32_t)0x0001)
#define I2C_IC_ENABLE_ABORT                         ((uint32_t)0x0002)

/*******************  Bit definition for IC_STATUS  register  *****************/
#define I2C_IC_STATUS_ACTIVITY                      ((uint32_t)0x0001)
#define I2C_IC_STATUS_TFNF                          ((uint32_t)0x0002)
#define I2C_IC_STATUS_TFE                           ((uint32_t)0x0004)
#define I2C_IC_STATUS_RFNE                          ((uint32_t)0x0008)
#define I2C_IC_STATUS_RFF                           ((uint32_t)0x0010)
#define I2C_IC_STATUS_MST_ACTIVITY                  ((uint32_t)0x0020)
#define I2C_IC_STATUS_SLV_ACTIVITY                  ((uint32_t)0x0040)
/*******************  Bit definition for IC_TXFLR  register  ******************/
#define I2C_IC_TXFLR_TXFLR                          (8)

/*******************  Bit definition for IC_RXFLR  register  ******************/
#define I2C_IC_RXFLR_RXFLR                          (8)

/*******************  Bit definition for IC_SDA_HOLD  register  ***************/
#define I2C_IC_SDA_HOLD                             ((uint32_t)0xFFFF)

/*******************  Bit definition for IC_TX_ABRT_SOURCE  register  *********/
#define I2C_IC_TX_ABRT_SOURCE_7B_ADDR_NOACK         ((uint32_t)0x00000001)
#define I2C_IC_TX_ABRT_SOURCE_10ADDR1_NOACK         ((uint32_t)0x00000002)
#define I2C_IC_TX_ABRT_SOURCE_10ADDR2_NOACK         ((uint32_t)0x00000004)
#define I2C_IC_TX_ABRT_SOURCE_TXDATA_NOACK          ((uint32_t)0x00000008)
#define I2C_IC_TX_ABRT_SOURCE_GCALL_NOACK           ((uint32_t)0x00000010)
#define I2C_IC_TX_ABRT_SOURCE_GCALL_READ            ((uint32_t)0x00000020)
#define I2C_IC_TX_ABRT_SOURCE_HS_ACKDET             ((uint32_t)0x00000040)
#define I2C_IC_TX_ABRT_SOURCE_SBYTE_ACKDET          ((uint32_t)0x00000080)
#define I2C_IC_TX_ABRT_SOURCE_HS_NORSTRT            ((uint32_t)0x00000100)
#define I2C_IC_TX_ABRT_SOURCE_SBYTE_NORSTRT         ((uint32_t)0x00000200)
#define I2C_IC_TX_ABRT_SOURCE_10B_RD_NORSTRT        ((uint32_t)0x00000400)
#define I2C_IC_TX_ABRT_SOURCE_MASTER_DIS            ((uint32_t)0x00000800)
#define I2C_IC_TX_ABRT_SOURCE_LOST                  ((uint32_t)0x00001000)
#define I2C_IC_TX_ABRT_SOURCE_SLVFLUSH_TXFIFO       ((uint32_t)0x00002000)
#define I2C_IC_TX_ABRT_SOURCE_SLV_ARBLOST           ((uint32_t)0x00004000)
#define I2C_IC_TX_ABRT_SOURCE_SLVRD_INTX            ((uint32_t)0x00008000)
#define I2C_IC_TX_ABRT_SOURCE_USER_ABRT             ((uint32_t)0x00010000)
#define I2C_IC_TX_ABRT_SOURCE_TX_FLUSH_CNT          ((uint32_t)0xFF000000)

/*******************  Bit definition for IC_SLV_DATA_NACK_ONLY  register  *****/
#define I2C_IC_SLV_DATA_NACK_ONLY                   ((uint32_t)0x0001)

/*******************  Bit definition for IC_DMA_TDLR  register  ***************/
#define I2C_IC_DMA_TDLR_TDLR                        ((uint32_t)0x000F)

/*******************  Bit definition for IC_DMA_RDLR  register  ***************/
#define I2C_IC_DMA_TDLR_TDLR                        ((uint32_t)0x000F)

/*******************  Bit definition for IC_SDA_SETUP  register  **************/
#define I2C_IC_SDA_SETUP                            ((uint32_t)0x00FF)

/*******************  Bit definition for IC_ACK_GENERAL_CALL  register  *******/
#define I2C_IC_ACK_GENERAL_CALL                     ((uint32_t)0x0001)

/*******************  Bit definition for IC_ENABLE_STATUS  register  **********/
#define I2C_IC_ENABLE_STATUS_IC_EN                  ((uint32_t)0x0001)
#define I2C_IC_ENABLE_STATUS_SLV_RX_ABORTED         ((uint32_t)0x0001)
#define I2C_IC_ENABLE_STATUS_SLV_FIFO_FILLED_AND_FLUSHED        ((uint32_t)0x0001)

/*******************  Bit definition for IC_FS_SPKLEN  register  **************/
#define I2C_IC_FS_SPKLEN_SPKLEN                     ((uint32_t)0x00FF)

/*******************  Bit definition for IC_HS_SPKLEN  register  **************/
#define I2C_IC_HS_SPKLEN_SPKLEN                     ((uint32_t)0x00FF)

/*******************  Bit definition for IC_COMP_PARAM_1  register  ***********/
#define I2C_IC_COMP_PARAM_1_APB_DATA_WIDTH          ((uint32_t)0x0003)
#define I2C_IC_COMP_PARAM_1_APB_DATA_WIDTH_0        ((uint32_t)0x0001)
#define I2C_IC_COMP_PARAM_1_APB_DATA_WIDTH_1        ((uint32_t)0x0002)
#define I2C_IC_COMP_PARAM_1_MAX_SPEED_MODE          ((uint32_t)0x000C)
#define I2C_IC_COMP_PARAM_1_MAX_SPEED_MODE_0        ((uint32_t)0x0004)
#define I2C_IC_COMP_PARAM_1_MAX_SPEED_MODE_1        ((uint32_t)0x0008)
#define I2C_IC_COMP_PARAM_1_HC_COUNT_VALUES         ((uint32_t)0x0010)
#define I2C_IC_COMP_PARAM_1_INTR_IO                 ((uint32_t)0x0020)
#define I2C_IC_COMP_PARAM_1_HAS_DMA                 ((uint32_t)0x0040)
#define I2C_IC_COMP_PARAM_1_ADD_ENCODED_PARAMS      ((uint32_t)0x0080)
#define I2C_IC_COMP_PARAM_1_RX_BUFFER_DEPTH         ((uint32_t)0x0000FF00)
#define I2C_IC_COMP_PARAM_1_TX_BUFFER_DEPTH         ((uint32_t)0x00FF0000)

/*******************  Bit definition for IC_COMP_VERSION  register  ***********/
#define I2C_IC_COMP_VERSION                         ((uint32_t)0xFFFFFFFF)

/*******************  Bit definition for IC_COMP_TYPE  register  **************/
#define I2C_IC_COMP_TYPE                            ((uint32_t)0xFFFFFFFF)


/** @defgroup I2C_duty_cycle_in_fast_mode 
  * @{
  */
/*Tlow/Thigh = ((I2C_DutyCycle_x_y & 0xFF00) >> 8) / (I2C_DutyCycle_x_y & 0x00FF)*/
#define I2C_DutyCycle_1					((uint16_t)0x0101) /*!< I2C standard mode Tlow/Thigh = 1 */
#define I2C_DutyCycle_16_9              ((uint16_t)0x1009) /*!< I2C fast mode Tlow/Thigh = 16/9 */
#define I2C_DutyCycle_2                 ((uint16_t)0x0201) /*!< I2C fast mode Tlow/Thigh = 2 */
#define I2C_DutyCycle_8_3               ((uint16_t)0x0803) /*!< I2C high mode Tlow/Thigh = 8/3 */
#define IS_I2C_DUTY_CYCLE(CYCLE)		(((CYCLE) == I2C_DutyCycle_16_9) || \
										((CYCLE) == I2C_DutyCycle_2))
/**
  * @}
  */ 

#define IS_I2C_SDA_SETUP_TIME(TIME)		((TIME) <= 0x000000FF)
#define IS_I2C_SDA_HOLD_TIME(TIME)		((TIME) <= 0x0000FFFF)

/** @defgroup I2C_acknowledged_address 
  * @{
  */
#define I2C_AcknowledgedAddress_7bit			((uint16_t)0x0001)
#define I2C_AcknowledgedAddress_10bit			((uint16_t)0x0002)
#define IS_I2C_ACKNOWLEDGE_ADDRESS(ADDRESS)		(((ADDRESS) == I2C_AcknowledgedAddress_7bit) || \
												((ADDRESS) == I2C_AcknowledgedAddress_10bit))
/**
  * @}
  */ 
	 
/** @defgroup Target_Address_Bit
  * @{
  */
#define I2C_TargetAddressMode_7bit				((uint16_t)0x0001)
#define I2C_TargetAddressMode_10bit				((uint16_t)0x0002)
#define IS_I2C_TARGET_ADDRESS_MODE(ADDRESS)		(((ADDRESS) == I2C_TargetAddressMode_7bit) || \
												((ADDRESS) == I2C_TargetAddressMode_10bit))
/**
  * @}
  */ 


/** @defgroup SPI_RXFIFOFullThreshold 
  * @{
  */
#define I2C_RXFIFOFullThreshold_1				((uint32_t)0x0000)
#define I2C_RXFIFOFullThreshold_2				((uint32_t)0x0001)
#define I2C_RXFIFOFullThreshold_3				((uint32_t)0x0002)
#define I2C_RXFIFOFullThreshold_4		        ((uint32_t)0x0003)
#define I2C_RXFIFOFullThreshold_5				((uint32_t)0x0004)
#define I2C_RXFIFOFullThreshold_6				((uint32_t)0x0005)
#define I2C_RXFIFOFullThreshold_7				((uint32_t)0x0006)
#define I2C_RXFIFOFullThreshold_8				((uint32_t)0x0007)

#define IS_I2C_RX_FIFO_FULL_THRESHOLD(THRESHOLD)	(THRESHOLD <= I2C_RXFIFOFullThreshold_8)
/**
  * @}
  */ 

/** @defgroup SPI_TXFIFOEmptyThreshold
  * @{
  */
#define I2C_TXFIFOEmptyThreshold_0				((uint32_t)0x0000)
#define I2C_TXFIFOEmptyThreshold_1				((uint32_t)0x0001)
#define I2C_TXFIFOEmptyThreshold_2				((uint32_t)0x0002)
#define I2C_TXFIFOEmptyThreshold_3				((uint32_t)0x0003)
#define I2C_TXFIFOEmptyThreshold_4				((uint32_t)0x0004)
#define I2C_TXFIFOEmptyThreshold_5				((uint32_t)0x0005)
#define I2C_TXFIFOEmptyThreshold_6				((uint32_t)0x0006)
#define I2C_TXFIFOEmptyThreshold_7				((uint32_t)0x0007)
#define I2C_TXFIFOEmptyThreshold_8				((uint32_t)0x0008)

#define IS_I2C_TX_FIFO_EMPTY_THRESHOLD(THRESHOLD)	(THRESHOLD <= I2C_TXFIFOEmptyThreshold_8)
/**
  * @}
  */ 


/** @defgroup I2C_DMAReceiveLevel 
  * @{
  */
#define I2C_DMAReceiveLevel_1				    ((uint32_t)0x0000)
#define I2C_DMAReceiveLevel_2			        ((uint32_t)0x0001)
#define I2C_DMAReceiveLevel_3					((uint32_t)0x0002)
#define I2C_DMAReceiveLevel_4					((uint32_t)0x0003)
#define I2C_DMAReceiveLevel_5					((uint32_t)0x0004)
#define I2C_DMAReceiveLevel_6					((uint32_t)0x0005)
#define I2C_DMAReceiveLevel_7					((uint32_t)0x0006)
#define I2C_DMAReceiveLevel_8					((uint32_t)0x0007)

#define IS_I2C_DMA_RECEIVE_LEVEL(LEVEL)			(LEVEL <= I2C_DMAReceiveLevel_8)
/**
  * @}
  */ 

/** @defgroup I2C_DMATransmitLevel 
  * @{
  */
#define I2C_DMATransmitLevel_0					((uint32_t)0x0000)
#define I2C_DMATransmitLevel_1					((uint32_t)0x0001)
#define I2C_DMATransmitLevel_2					((uint32_t)0x0002)
#define I2C_DMATransmitLevel_3					((uint32_t)0x0003)
#define I2C_DMATransmitLevel_4					((uint32_t)0x0004)
#define I2C_DMATransmitLevel_5					((uint32_t)0x0005)
#define I2C_DMATransmitLevel_6					((uint32_t)0x0006)
#define I2C_DMATransmitLevel_7					((uint32_t)0x0007)

#define IS_I2C_DMA_TRANSMIT_LEVEL(LEVEL)	    (LEVEL <= I2C_DMATransmitLevel_7)
/**
  * @}
  */ 

/** @defgroup I2C_DMA_transfer_requests 
  * @{
  */
#define I2C_DMAReq_Rx               ((uint32_t)0x0001)
#define I2C_DMAReq_Tx               ((uint32_t)0x0002)
#define IS_I2C_DMAREQ(DMAREQ)		(((DMAREQ) & I2C_DMAReq_Tx) || \
									((DMAREQ) & I2C_DMAReq_Rx))
/**
  * @}
  */

#define I2C_IT_RXUDF					((uint32_t)0x0001)				//clear by hardware
#define I2C_IT_RXOVF					((uint32_t)0x0002)				//clear by read 
#define I2C_IT_RXF						((uint32_t)0x0004)				//clear by read
#define I2C_IT_TXOVF					((uint32_t)0x0008)				//clear by read  
#define I2C_IT_TXE						((uint32_t)0x0010)				//clear by hardware
#define I2C_IT_RD_REQ					((uint32_t)0x0020)				//clear by read
#define I2C_IT_TX_ABRT					((uint32_t)0x0040)				//clear by read  
#define I2C_IT_RX_DONE					((uint32_t)0x0080)				//clear by read 
#define I2C_IT_ACTIVITY					((uint32_t)0x0100)				//clear by read 
#define I2C_IT_STOP_DET					((uint32_t)0x0200)				//clear by read 
#define I2C_IT_START_DET				((uint32_t)0x0400)
#define I2C_IT_GEN_CALL					((uint32_t)0x0800)
#define I2C_IT_ALL						((uint32_t)0x0FFF)

#define IS_I2C_CONFIG_IT(IT)			((((IT) & (~(uint32_t)0x0FFF)) == 0x00) && ((IT) != 0x00))
#define IS_I2C_CLEAR_IT(IT)				((((IT) & (~(uint32_t)0x0FFF)) == 0x00) && ((IT) != 0x00))
#define IS_I2C_GET_IT(IT)				(((IT) == I2C_IT_RXUDF) || \
										((IT) == I2C_IT_RXOVF) || \
										((IT) == I2C_IT_RXF) || \
										((IT) == I2C_IT_TXOVF) || \
										((IT) == I2C_IT_TXE) || \
										((IT) == I2C_IT_RD_REQ) || \
										((IT) == I2C_IT_TX_ABRT) || \
										((IT) == I2C_IT_RX_DONE) || \
										((IT) == I2C_IT_ACTIVITY) || \
										((IT) == I2C_IT_STOP_DET) || \
										((IT) == I2C_IT_START_DET) || \
										((IT) == I2C_IT_GEN_CALL) || \
										((IT) == I2C_IT_ALL))

#define I2C_FLAG_ACTIVITY				((uint32_t)0x0001)
#define I2C_FLAG_TXNF					((uint32_t)0x0002)
#define I2C_FLAG_TXE					((uint32_t)0x0004)
#define I2C_FLAG_RXNE					((uint32_t)0x0008)
#define I2C_FLAG_RXF					((uint32_t)0x0010)
#define I2C_FLAG_MST_ACTIVITY			((uint32_t)0x0020)
#define I2C_FLAG_SLV_ACTIVITY			((uint32_t)0x0040)
#define I2C_FLAG_ALL					((uint32_t)0x007F)
#define IS_I2C_GET_FLAG(FLAG)			(((FLAG) == I2C_FLAG_ACTIVITY) || \
										((FLAG) == I2C_FLAG_TXNF) || \
										((FLAG) == I2C_FLAG_TXE) || \
										((FLAG) == I2C_FLAG_RXNE) || \
										((FLAG) == I2C_FLAG_RXF) || \
										((FLAG) == I2C_FLAG_MST_ACTIVITY) || \
										((FLAG) == I2C_FLAG_SLV_ACTIVITY) || \
										((FLAG) == I2C_FLAG_ALL))

#define I2C_TX_ABRT_7BIT_ADDR_NOACK					((uint32_t)0x00000001)
#define I2C_TX_ABRT_10BIT_ADDR1_NOACK				((uint32_t)0x00000002)
#define I2C_TX_ABRT_10BIT_ADDR2_NOACK				((uint32_t)0x00000004)
#define I2C_TX_ABRT_TXDATA_NOACK					((uint32_t)0x00000008)
#define I2C_TX_ABRT_GEN_CALL_NOACK					((uint32_t)0x00000010)
#define I2C_TX_ABRT_GEN_CALL_READ					((uint32_t)0x00000020)
#define I2C_TX_ABRT_HIGH_SPEED_ACKDET				((uint32_t)0x00000040)
#define I2C_TX_ABRT_START_BYTE_ACKDET				((uint32_t)0x00000080)
#define I2C_TX_ABRT_HIGH_SPEED_NORSTRT				((uint32_t)0x00000100)
#define I2C_TX_ABRT_START_BYTE_NORSTRT				((uint32_t)0x00000200)
#define I2C_TX_ABRT_10BIT_RD_NORSTRT				((uint32_t)0x00000400)
#define I2C_TX_ABRT_LOST_ARB						((uint32_t)0x00000800)
#define I2C_TX_ABRT_SLVFLUSH_TXFIFO					((uint32_t)0x00001000)
#define I2C_TX_ABRT_SLV_LOST_ARB					((uint32_t)0x00002000)
#define I2C_TX_ABRT_SLV_RD_INTX						((uint32_t)0x00004000)
#define I2C_TX_ABRT_MASTER_DISABLE					((uint32_t)0x00008000)
#define I2C_TX_ABRT_USER_ABRT						((uint32_t)0x00010000)
#define I2C_TX_ABRT_TX_FLUSH_CNT					((uint32_t)0x00020000)
#define IS_I2C_TX_ABRT(ABRT)						(((ABRT) == I2C_TX_ABRT_7BIT_ADDR_NOACK) || \
													((ABRT) == I2C_TX_ABRT_10BIT_ADDR1_NOACK) || \
													((FLAG) == I2C_TX_ABRT_10BIT_ADDR2_NOACK) || \
													((FLAG) == I2C_TX_ABRT_TXDATA_NOACK) || \
													((FLAG) == I2C_TX_ABRT_GEN_CALL_NOACK) || \
													((FLAG) == I2C_TX_ABRT_GEN_CALL_READ) || \
													((FLAG) == I2C_TX_ABRT_HIGH_SPEED_ACKDET) || \
													((FLAG) == I2C_TX_ABRT_START_BYTE_ACKDET) || \
													((FLAG) == I2C_TX_ABRT_HIGH_SPEED_NORSTRT) || \
													((FLAG) == I2C_TX_ABRT_START_BYTE_NORSTRT) || \
													((FLAG) == I2C_TX_ABRT_10BIT_RD_NORSTRT) || \
													((FLAG) == I2C_TX_ABRT_LOST_ARB) || \
													((FLAG) == I2C_TX_ABRT_SLVFLUSH_TXFIFO) || \
													((FLAG) == I2C_TX_ABRT_SLV_LOST_ARB) || \
													((FLAG) == I2C_TX_ABRT_SLV_RD_INTX) || \
													((FLAG) == I2C_TX_ABRT_MASTER_DISABLE) || \
													((FLAG) == I2C_TX_ABRT_USER_ABRT) || \
													((FLAG) == I2C_TX_ABRT_TX_FLUSH_CNT))

typedef enum
{
	I2C_DataCMD_Write					= (uint32_t)0x01,
	I2C_DataCMD_Read					= (uint32_t)0x02
}I2CDataCMD_TypeDef;
#define IS_I2C_DATA_CMD(CMD)			(((CMD) == I2C_DataCMD_Write) || \
										((CMD) == I2C_DataCMD_Read))

typedef enum
{
	I2C_DataEndCondition_None			= (uint32_t)0x01,
	I2C_DataEndCondition_Stop			= (uint32_t)0x02,
	I2C_DataEndCondition_Restart		= (uint32_t)0x03
}I2CDataEndCondition_TypeDef;
#define IS_I2C_DATA_END_CONDITION(CONDITION)	(((CONDITION) == I2C_DataEndCondition_None) || \
												((CONDITION) == I2C_DataEndCondition_Stop) || \
												((CONDITION) == I2C_DataEndCondition_Restart))


void I2C_DeInit(I2C_TypeDef* I2Cx);
void I2C_Init(I2C_TypeDef* I2Cx,I2C_InitTypeDef* I2C_InitStruct);
void I2C_StructInit(I2C_InitTypeDef* I2C_InitStruct);
void I2C_Cmd(I2C_TypeDef* I2Cx, FunctionalState NewState);
void I2C_SetTargetAddress(I2C_TypeDef* I2Cx, uint32_t TargetAddress, uint32_t TargetAddressMode);
void I2C_DMAInit(I2C_TypeDef* I2Cx, I2C_DMAInitTypeDef* I2C_DMAInitStruct);
void I2C_DMAStructInit(I2C_DMAInitTypeDef* I2C_DMAInitStruct);
void I2C_DMACmd(I2C_TypeDef* I2Cx, uint32_t I2C_DMAReq, FunctionalState NewState);
void I2C_SetSDASetupTime(I2C_TypeDef* I2Cx,uint32_t PCLKCycles);
void I2C_SetSDAHoldTime(I2C_TypeDef* I2Cx,uint32_t PCLKCycles);

void I2C_ITConfig(I2C_TypeDef* I2Cx,uint32_t I2C_IT, FunctionalState NewState);
ITStatus I2C_GetITStatus(I2C_TypeDef* I2Cx,uint32_t I2C_IT);
ITStatus I2C_GetRawITStatus(I2C_TypeDef* I2Cx,uint32_t I2C_IT);
void I2C_ClearITPendingBit(I2C_TypeDef* I2Cx,uint32_t I2C_IT);
FlagStatus I2C_GetFlagStatus(I2C_TypeDef* I2Cx,uint32_t I2C_FLAG);
uint32_t I2C_GetFlagStatusReg(I2C_TypeDef* I2Cx);
FlagStatus I2C_GetTXAbortSource(I2C_TypeDef* I2Cx,uint32_t I2C_TX_ABRT);
uint32_t I2C_GetTXAbortSourceReg(I2C_TypeDef* I2Cx);
I2CMode_TypeDef I2C_GetI2CMode(I2C_TypeDef* I2Cx);

void I2C_MasterGenerateReceiveSCL(I2C_TypeDef* I2Cx, I2CDataEndCondition_TypeDef DataCondition);
uint16_t I2C_ExtendData(uint8_t Data, I2CDataCMD_TypeDef DataCMD, I2CDataEndCondition_TypeDef DataCondition);
void I2C_WriteDataToDR(I2C_TypeDef* I2Cx, uint16_t ExtendData);
uint8_t I2C_ReadDataFromDR(I2C_TypeDef* I2Cx);
void I2C_SendData(I2C_TypeDef* I2Cx, uint8_t Data, I2CDataEndCondition_TypeDef DataCondition);
uint8_t I2C_ReceiveData(I2C_TypeDef* I2Cx, I2CDataEndCondition_TypeDef DataCondition);
void I2C_SendBytes(I2C_TypeDef* I2Cx, uint8_t* Data, uint32_t DataLen, I2CDataEndCondition_TypeDef DataCondition);
void I2C_ReceiveBytes(I2C_TypeDef* I2Cx, uint8_t* Data, uint32_t DataLen, I2CDataEndCondition_TypeDef DataCondition);

void I2C_SlaveGeneralNACKOnlyCmd(I2C_TypeDef* I2Cx, FunctionalState NewState);

FlagStatus I2C_IsEnable(I2C_TypeDef* I2Cx);


#ifdef __cplusplus
}
#endif

#endif 

/**************************      (C) COPYRIGHT    *****END OF FILE****/
