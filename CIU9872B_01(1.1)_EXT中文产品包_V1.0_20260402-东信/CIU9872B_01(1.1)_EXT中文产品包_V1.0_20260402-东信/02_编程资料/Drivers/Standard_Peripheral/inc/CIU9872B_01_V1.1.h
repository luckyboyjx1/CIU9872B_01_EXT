/************************************************************************************************/
/**
* @file               CIU9872B_01_V1.1.h
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
* <tr><td>2022/11/09  <td>V1.0        <td>Firmware Team   <td>First release   </tr>
* </table>
*
**************************************************************************************************
*/
#ifndef __CIU9872B_01_V1_1_H_
#define __CIU9872B_01_V1_1_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "csi_gcc.h"
#include "CIU9872B_01_V1.1_EXT_SFR_CHIP.h"

/************************************************************************************************/
/**
* @addtogroup Standard_Peripheral
* @{
*/

/**
* @addtogroup SFR
* @{
*/
/************************************************************************************************/

#define SUCCESS                 0x0
#define FAIL                    (~0)
#define TRUE                    1
#define FALSE                   0
#undef NULL  /* others (e.g. <stdio.h>) also define */
#define NULL                    0

#define PRIVILEGED_FUNCTION
#define PRIVILEGED_DATA

/*------------------------------------------Exported types-----------------------------------------------*/
typedef unsigned char   boolean;
typedef unsigned char   bool;
typedef unsigned char   u8;
typedef signed char     s8;
typedef unsigned short  u16;
typedef signed short    s16;
typedef unsigned int    u32;
typedef signed   int    s32;

typedef unsigned long   u64;

/**
  * @brief  HAL Status structures definition
  */
typedef enum
{
    HAL_OK       		= 0x00U,
    HAL_ERROR    		= 0x01U,
    HAL_BUSY     		= 0x02U,
    HAL_TIMEOUT  		= 0x03U,
	HAL_RESEND	 		= 0x04U,
	HAL_RECVLEN_OVER	= 0x05U,
} HAL_StatusTypeDef;

/**
  * @brief  HAL Lock structures definition
  */
typedef enum
{
    HAL_UNLOCKED = 0x00U,
    HAL_LOCKED   = 0x01U
} HAL_LockTypeDef;


/* IO definitions (access restrictions to peripheral registers) */
#define   __I     volatile const       /*!< Defines 'read only' permissions */
#define   __O     volatile             /*!< Defines 'write only' permissions */
#define   __IO    volatile             /*!< Defines 'read / write' permissions */

/* following defines should be used for structure members */
#define     __IM     volatile const      /*! Defines 'read only' structure member permissions */
#define     __OM     volatile            /*! Defines 'write only' structure member permissions */
#define     __IOM    volatile            /*! Defines 'read / write' structure member permissions */

/*------------------------------------------Exported macro-----------------------------------------------*/
#define BIT0    (1ul<<0)
#define BIT1    (1ul<<1)
#define BIT2    (1ul<<2)
#define BIT3    (1ul<<3)
#define BIT4    (1ul<<4)
#define BIT5    (1ul<<5)
#define BIT6    (1ul<<6)
#define BIT7    (1ul<<7)
#define BIT8    (1ul<<8)
#define BIT9    (1ul<<9)
#define BIT10   (1ul<<10)
#define BIT11   (1ul<<11)
#define BIT12   (1ul<<12)
#define BIT13   (1ul<<13)
#define BIT14   (1ul<<14)
#define BIT15   (1ul<<15)
#define BIT16   (1ul<<16)
#define BIT17   (1ul<<17)
#define BIT18   (1ul<<18)
#define BIT19   (1ul<<19)
#define BIT20   (1ul<<20)
#define BIT21   (1ul<<21)
#define BIT22   (1ul<<22)
#define BIT23   (1ul<<23)
#define BIT24   (1ul<<24)
#define BIT25   (1ul<<25)
#define BIT26   (1ul<<26)
#define BIT29   (1ul<<29)
#define BIT30   (1ul<<30)
#define BIT31   (1ul<<31)

#define SET_REG_BIT(reg,bit)                (reg |= (1ul << (bit)))
#define CLEAR_REG_BIT(reg,bit)              (reg &= ~(1ul << (bit)))
#define REG_BIT_IS_SET(reg,bit)             (reg & (1ul << (bit)))
#define REG_BIT_IS_CLEAR(reg,bit)           ((reg & (1ul << (bit))) == 0)
#define SET_BIT(bit)                        (1ul << (bit))
#define CLEAR_BIT(bit)                      (~(1ul << (bit)))

#define UNUSED(x) ((void)(x))

#define __HAL_LOCK(__HANDLE__)             \
do                                         \
{                                          \
    if((__HANDLE__)->Lock == HAL_LOCKED)   \
    {                                      \
       return HAL_BUSY;                    \
    }                                      \
    else                                   \
    {                                      \
       (__HANDLE__)->Lock = HAL_LOCKED;    \
    }                                      \
}while (0U)

#define __HAL_UNLOCK(__HANDLE__)           \
do                                         \
{                                          \
  (__HANDLE__)->Lock = HAL_UNLOCKED;       \
}while (0U)

#define __HAL_LINKDMA(__HANDLE__, __DMA_FIELD__, __DMA_HANDLE__, __UP_FIELD__)               \
                        do{                                                      \
                              (__HANDLE__).__DMA_FIELD__ = &(__DMA_HANDLE__); \
                              (__DMA_HANDLE__).__UP_FIELD__ = &(__HANDLE__);             \
                          } while(0U)

#define __HAL_QSPI_LINKDMA(__HANDLE__, __DMA_HANDLE__, __UP_FIELD__)               \
                        do{                                                      \
                              (__DMA_HANDLE__).__UP_FIELD__ = &(__HANDLE__);             \
                          } while(0U)

#define __HAL_RST_MODULE(SFR,bitPos)    CLEAR_REG_BIT(SFR,bitPos) //0 is reset the module.


/********************************************************************************
                             Interrupt Source
********************************************************************************/
/**
 * @brief Interrupt Number Definition
 *
 */
typedef enum IRQn
{
    IRQ_TMR0              = 0,
    IRQ_TMR1              = 1,
    IRQ_WDT               = 2,
    IRQ_NVMCTRL           = 3,
    IRQ_EMMU              = 4,
    IRQ_SECRST            = 5,
    IRQ_RNG1              = 6,
    IRQ_DMA               = 7,
    IRQ_7816RST           = 8,
    IRQ_7816TX            = 9,
    IRQ_7816RX            = 10,
    IRQ_7816FIFO          = 11,
    IRQ_GPIO              = 12,
    IRQ_RFON              = 13,
    IRQ_RFOFF             = 14,
    IRQ_I2C               = 15,
    IRQ_SPI               = 16,
    IRQ_TYPEA             = 17,
    IRQ_TYPEB             = 18,
    IRQ_TMR2              = 19,
    IRQ_CTIM              = 20,
} IRQn_Type;

/********************************************************************************
                           M0+ Interrupt Source
********************************************************************************/
#define IRQ_TMR0_OFFSET             IRQ_TMR0
#define IRQ_TMR1_OFFSET             IRQ_TMR1
#define IRQ_WDT_OFFSET              IRQ_WDT
#define IRQ_NVMCTRL_OFFSET          IRQ_NVMCTRL
#define IRQ_EMMU_OFFSET             IRQ_EMMU
#define IRQ_SECRST_OFFSET           IRQ_SECRST
#define IRQ_RNG1_OFFSET             IRQ_RNG1
#define IRQ_DMA_OFFSET              IRQ_DMA
#define IRQ_7816RST_OFFSET          IRQ_7816RST
#define IRQ_7816TX_OFFSET           IRQ_7816TX
#define IRQ_7816RX_OFFSET           IRQ_7816RX
#define IRQ_7816FIFO_OFFSET         IRQ_7816FIFO
#define IRQ_GPIO_OFFSET             IRQ_GPIO
#define IRQ_RFON_OFFSET             IRQ_RFON
#define IRQ_RFOFF_OFFSET            IRQ_RFOFF
#define IRQ_I2C_OFFSET              IRQ_I2C
#define IRQ_SPI_OFFSET              IRQ_SPI
#define IRQ_TYPEA_OFFSET            IRQ_TYPEA
#define IRQ_TYPEB_OFFSET            IRQ_TYPEB
#define IRQ_TMR2_OFFSET             IRQ_TMR2
#define IRQ_CTIM_OFFSET             IRQ_CTIM


#define TMR0_IRQEN                  (CPU->VIC_ISER = (1<<IRQ_TMR0_OFFSET))
#define TMR1_IRQEN                  (CPU->VIC_ISER = (1<<IRQ_TMR1_OFFSET))
#define WDT_IRQEN                   (CPU->VIC_ISER = (1<<IRQ_WDT_OFFSET))
#define NVMCTRL_IRQEN               (CPU->VIC_ISER = (1<<IRQ_NVMCTRL_OFFSET))
#define EMMU_IRQEN                  (CPU->VIC_ISER = (1<<IRQ_EMMU_OFFSET))
#define SECRST_IRQEN                (CPU->VIC_ISER = (1<<IRQ_SECRST_OFFSET))
#define RNG1_IRQEN                  (CPU->VIC_ISER = (1<<IRQ_RNG1_OFFSET))
#define DMA_IRQEN                   (CPU->VIC_ISER = (1<<IRQ_DMA_OFFSET))
#define S7816RST_IRQEN              (CPU->VIC_ISER = (1<<IRQ_7816RST_OFFSET))
#define S7816TX_IRQEN               (CPU->VIC_ISER = (1<<IRQ_7816TX_OFFSET))
#define S7816RX_IRQEN               (CPU->VIC_ISER = (1<<IRQ_7816RX_OFFSET))
#define S7816FIFO_IRQEN             (CPU->VIC_ISER = (1<<IRQ_7816FIFO_OFFSET))
#define GPIO_IRQEN                  (CPU->VIC_ISER = (1<<IRQ_GPIO_OFFSET))
#define RFON_IRQEN                  (CPU->VIC_ISER = (1<<IRQ_RFON_OFFSET))
#define RFOFF_IRQEN                 (CPU->VIC_ISER = (1<<IRQ_RFOFF_OFFSET))
#define I2C_IRQEN                   (CPU->VIC_ISER = (1<<IRQ_I2C_OFFSET))
#define SPI_IRQEN                   (CPU->VIC_ISER = (1<<IRQ_SPI_OFFSET))
#define TYPEA_IRQEN                 (CPU->VIC_ISER = (1<<IRQ_TYPEA_OFFSET))
#define TYPEB_IRQEN                 (CPU->VIC_ISER = (1<<IRQ_TYPEB_OFFSET))
#define TMR2_IRQEN                  (CPU->VIC_ISER = (1<<IRQ_TMR2_OFFSET))
#define CTIM_IRQEN                  (CPU->VIC_ISER = (1<<IRQ_CTIM_OFFSET))


#define TMR0_IRQDIS                 (CPU->VIC_ICER = (1<<IRQ_TMR0_OFFSET))
#define TMR1_IRQDIS                 (CPU->VIC_ICER = (1<<IRQ_TMR1_OFFSET))
#define WDT_IRQDIS                  (CPU->VIC_ICER = (1<<IRQ_WDT_OFFSET))
#define NVMCTRL_IRQDIS              (CPU->VIC_ICER = (1<<IRQ_NVMCTRL_OFFSET))
#define EMMU_IRQDIS                 (CPU->VIC_ICER = (1<<IRQ_EMMU_OFFSET))
#define SECRST_IRQDIS               (CPU->VIC_ICER = (1<<IRQ_SECRST_OFFSET))
#define RNG1_IRQDIS                 (CPU->VIC_ICER = (1<<IRQ_RNG1_OFFSET))
#define DMA_IRQDIS                  (CPU->VIC_ICER = (1<<IRQ_DMA_OFFSET))
#define S7816RST_IRQDIS             (CPU->VIC_ICER = (1<<IRQ_7816RST_OFFSET))
#define S7816TX_IRQDIS              (CPU->VIC_ICER = (1<<IRQ_7816TX_OFFSET))
#define S7816RX_IRQDIS              (CPU->VIC_ICER = (1<<IRQ_7816RX_OFFSET))
#define S7816FIFOIRQDIS             (CPU->VIC_ICER = (1<<IRQ_7816FIFO_OFFSET))
#define GPIO_IRQDIS                 (CPU->VIC_ICER = (1<<IRQ_GPIO_OFFSET))
#define RFON_IRQDIS                 (CPU->VIC_ICER = (1<<IRQ_RFON_OFFSET))
#define RFOFF_IRQDIS                (CPU->VIC_ICER = (1<<IRQ_RFOFF_OFFSET))
#define I2C_IRQDIS                  (CPU->VIC_ICER = (1<<IRQ_I2C_OFFSET))
#define SPI_IRQDIS                  (CPU->VIC_ICER = (1<<IRQ_SPI_OFFSET))
#define TYPEA_IRQDIS                (CPU->VIC_ICER = (1<<IRQ_TYPEA_OFFSET))
#define TYPEB_IRQDIS                (CPU->VIC_ICER = (1<<IRQ_TYPEB_OFFSET))
#define TMR2_IRQDIS                 (CPU->VIC_ICER = (1<<IRQ_TMR2_OFFSET))
#define CTIM_IRQDIS                 (CPU->VIC_ICER = (1<<IRQ_CTIM_OFFSET))

#define TMR0_PENDCLR                 (CPU->VIC_ICPR = (1<<IRQ_TMR0_OFFSET))
#define TMR1_PENDCLR                 (CPU->VIC_ICPR = (1<<IRQ_TMR1_OFFSET))
#define TMR2_PENDCLR                 (CPU->VIC_ICPR = (1<<IRQ_TMR2_OFFSET))
 
 

#define  __weak __attribute__((weak))
/**
* @}
*/

/**
* @}
*/
#endif  /* __CIU9872B_01_V1_1_H_ */
