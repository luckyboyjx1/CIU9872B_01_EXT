/**************************************************************************//**
 * @file     <Device>.h
 * @brief    CMSIS Cortex-M# Core Peripheral Access Layer Header File for
 *           Device <Device>
 * @version  V3.10
 * @date     23. November 2012
 *
 * @note
 *
 ******************************************************************************/
/* Copyright (c) 2012 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list ofC conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/


#ifndef __MHCARD_H      /* ToDo: replace '<Device>' with your device name */
#define __MHCARD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* ToDo: replace '<Device>' with your device name; add your doxyGen comment   */
/** @addtogroup <Device>_Definitions <Device> Definitions
  This file defines all structures and symbols for <Device>:
    - registers and bitfields
    - peripheral base address
    - peripheral ID
    - Peripheral definitions
  @{
*/


/******************************************************************************/
/*                Processor and Core Peripherals                              */
/******************************************************************************/
/** @addtogroup <Device>_CMSIS Device CMSIS Definitions
  Configuration of the Cortex-M# Processor and Core Peripherals
  @{
*/
/*
 * ==========================================================================
 * ---------- Interrupt Number Definition -----------------------------------
 * ==========================================================================
 */
typedef enum IRQn
{
/* -------------------  Cortex-M0 Processor Exceptions Numbers  ------------------- */
    NonMaskableInt_IRQn             = -14,      /*  2 Non Maskable Interrupt */
    HardFault_IRQn                  = -13,      /*  3 HardFault Interrupt */
    MemManage_IRQn                  = -12,


    SVCall_IRQn                     =  -5,      /* 11 SV Call Interrupt */

    PendSV_IRQn                     =  -2,      /* 14 Pend SV Interrupt */
    SysTick_IRQn                    =  -1,      /* 15 System Tick Interrupt */

/* ----------------------  ARMCM0 Specific Interrupt Numbers  --------------------- */
    SYMC_IRQn                      = 0,   
    SEC_IRQn                     = 1,   
    ASYMC_IRQn                        = 2,   
    SCIS_IRQn                       = 3,   
    CRC_IRQn                        = 4,   
    TRNG_IRQn                       = 5,   
    DMA_IRQn                        = 6,   
    NFCS_IRQn                   = 7,   
    TIM0_IRQn                       = 8,   
    TIM1_IRQn                       = 9,   
    SPI_IRQn                       = 10,
    GPIO_IRQn                       = 11,
    FLASH_ECC_IRQn                  = 12,
    FLASH_NVM_IRQn                  = 13,
    TIM2_IRQn                       = 14,
    I2C_IRQn                       =15,
    CB_FLAG_IRQn                   =16
} IRQn_Type;


/*
 * ==========================================================================
 * ----------- Processor and Core Peripheral Section ------------------------
 * ==========================================================================
 */


/* --------  Configuration of the Cortex-M0 Processor and Core Peripherals  ------- */
#define __CM0_REV                 0x0000U   /* Core revision r0p0 */
#define __MPU_PRESENT             1         /* MPU present or not */
#define __VTOR_PRESENT            0         /* no VTOR present*/
#define __NVIC_PRIO_BITS          2         /* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0         /* Set to 1 if different SysTick Config is used */

/*@}*/ /* end of group <Device>_CMSIS */


/* ToDo: include the correct core_cm#.h file
         core_cm0.h if your device is a CORTEX-M0 device
         core_cm3.h if your device is a CORTEX-M3 device
         core_cm4.h if your device is a CORTEX-M4 device                                          */
#include "core_sc000.h"                       /* Cortex-M# processor and core peripherals           */
/* ToDo: include your system_<Device>.h file
         replace '<Device>' with your device name                                                 */
//#include "system_mhcard.h"                /* <Device> System  include file                      */


/******************************************************************************/
/*                Device Specific Peripheral registers structures             */
/******************************************************************************/
/** @addtogroup <Device>_Peripherals <Device> Peripherals
  <Device> Device Specific Peripheral registers structures
  @{
*/

#if defined ( __CC_ARM   )
#pragma anon_unions
#endif

#include <stdint.h>

/** @addtogroup Exported_types
  * @{
  */  

typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

//typedef enum {FALSE = 0, TRUE = !FALSE} Boolean;

typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;

/* ToDo: add here your device specific peripheral access structure typedefs
         following is an example for a timer                                  */

#define BIT(n)              (1UL << (n))

//H_start >= L_end, use as [],not (),(],[);
#define BITS(H_start,L_end)         ((0xFFFFFFFFUL>>(31 - (H_start))) & (0xFFFFFFFFUL << (L_end)))
//typedef struct
//{
//    __IO uint32_t CLK_EN;//0x00
//    __IO uint32_t RST_EN;//0x04
//    __IO uint32_t CLK_CFG;//0x08
//    __IO uint32_t SYS_CON;//0x0C
//    __IO uint32_t RES0[(0x3C-0x10) >> 2];//0x10~0x3C
//    __IO uint32_t WAKE_MCU_CTRL;//0x40
//    __IO uint32_t WAKE_SYS_CTRL;//0x44
//    __IO uint32_t RST_CFG;//0x48
//    __IO uint32_t RES1[(0xC4-0x4C) >> 2];
//    __IO uint32_t CBCL_FLAG;//0xC8

//} SYSCFG_TypeDef;

//typedef struct
//{
//    __IO  uint32_t  MODHW;      //0x00
//    __O   uint32_t  RES0;       
//    __IO  uint32_t  CTRL;       //0x08
//    __O   uint32_t  STAT;      //0x0C
//    __IO  uint32_t  INTIO1;     //0x10
//    __O   uint32_t  RES1;
//    __IO  uint32_t  EDCCTRL;    //0x18
//    __IO  uint32_t  BGTCTRL;    //0x1C
//    __IO  uint32_t  BUFHW;      //0x20
//    __O   uint32_t  RES2;
//    __IO  uint32_t  ETUDATA;    //0x28
//    __IO  uint32_t  BGTDATA;    //0x2C
//    __IO  uint32_t  CWTDATA;    //0x30
//    __IO  uint32_t  EDCDATA;    //0x34
//    
//} SCI_SLAVE_TypeDef;



                                                      

//typedef struct
//{
//    union
//    {
//        __I  uint32_t RBR;
//        __O  uint32_t THR;
//        __IO uint32_t DLL;
//    } OFFSET_0;
//    union
//    {
//        __IO uint32_t DLH;
//        __IO uint32_t IER;
//    } OFFSET_4;
//    union
//    {
//        __I uint32_t IIR;
//        __O uint32_t FCR;
//    } OFFSET_8;
//    __IO uint32_t LCR;
//    __IO uint32_t MCR;
//    __I  uint32_t LSR;
//    __I  uint32_t MSR;
//    __IO uint32_t SCR;
//    __IO uint32_t LPDLL;
//    __IO uint32_t LPDLH;
//    __I  uint32_t RES0[2];
//    union
//    {
//        __I  uint32_t SRBR[16];
//        __O  uint32_t STHR[16];
//    } OFFSET_48;
//    __IO uint32_t FAR;
//    __I  uint32_t TFR;
//    __O  uint32_t RFW;
//    __I  uint32_t USR;
//    __I  uint32_t TFL;
//    __I  uint32_t RFL;
//    __O  uint32_t SRR;
//    __IO uint32_t SRTS;
//    __IO uint32_t SBCR;
//    __IO uint32_t SDMAM;
//    __IO uint32_t SFE;
//    __IO uint32_t SRT;
//    __IO uint32_t STET;
//    __IO uint32_t HTX;
//    __O uint32_t DMASA;
//    __I  uint32_t RES1[18];
//    __I  uint32_t CPR;
//    __I  uint32_t UCV;
//    __I  uint32_t CTR;
//    
//} UART_TypeDef;

//typedef struct
//{
//    __IO uint32_t WDT_CR;
//    __IO uint32_t RESERVED0;
//    __I  uint32_t WDT_CCVR;
//    __O  uint32_t WDT_CRR;
//    __I  uint32_t WDT_STAT;
//    __I  uint32_t WDT_EOI;  
//    __I  uint32_t RESERVED1;
//    __IO uint32_t WDT_RLD;
//    __I  uint32_t RESERVED[53];
//    __I  uint32_t WDT_COMP_PARAMS_1;
//    __I  uint32_t WDT_COMP_VERSION;
//    __I  uint32_t WDT_COMP_TYPE;
//} WDT_TypeDef;


//typedef struct
//{
//    __IO uint32_t CRC_CSR;
//    __O  uint32_t CRC_INI;
//    union
//    {
//        __I uint32_t DOUT;
//        __O uint32_t  DIN;
//    } CRC_DATA;
//    __IO uint32_t MASK;
//} CRC_TypeDef;

//typedef struct
//{
//    __IO uint32_t LoadCount;
//    __I  uint32_t CurrentValue;
//    __IO uint32_t ControlReg;
//    __IO  uint32_t EOI;
//    __I  uint32_t IntStatus;
//} TIM_TypeDef;

//typedef struct
//{
//    #define TIM_NUM 2
//    TIM_TypeDef TIM[TIM_NUM];
//    __I uint8_t RESERVED[0xA0 - TIM_NUM * sizeof(TIM_TypeDef)];
//    __I  uint32_t TIM_IntStatus;
//    __I  uint32_t TIM_EOI;
//    __I  uint32_t TIM_RawIntStatus;
//    __I  uint32_t TIM_Comp;
//    __IO uint32_t TIM_ReloadCount[TIM_NUM];
//} TIM_Module_TypeDef;

//typedef struct 
//{
//    __IO uint32_t ADC_CR;
//    __I  uint32_t ADC_SR;
//} ADC_TypeDef;

//typedef struct
//{
//    __IO uint32_t IOD;//0x00
//    __IO uint32_t BSR;//0x04
//    __IO uint32_t CON;//0x08
//    __IO uint32_t MODE;//0x0c
//    __IO uint32_t MODE_H;//0x10
//    __IO uint32_t TYPE;//0x14
//    __IO uint32_t RES;//0x18
//    __IO uint32_t STAT;//0x1C
//} GPIO_TypeDef;



//typedef struct
//{
//    __IO uint32_t KEY[32];
//    __IO uint32_t BPK_RDY;
//    __IO uint32_t BPK_CLR;
//    __IO uint32_t BPK_LRA;
//    __IO uint32_t BPK_LWA;
//    __IO uint32_t BPK_RR;
//    __IO uint32_t BPK_LR;
//    __O  uint32_t BPK_SCR;
//    __I  uint32_t BPK_RSVD0;
//    __IO uint32_t RTC_CS;
//    __IO uint32_t RTC_REF;
//    __IO uint32_t RTC_ARM;
//    __I  uint32_t RTC_TIM;
//    __O  uint32_t RTC_INTCLR;
//    __IO uint32_t OSC32K_CR;
//    __IO uint32_t RTC_ATTA_TIM;
//    __IO uint32_t RESERVED2;
//    __O uint32_t SEN_EXT_TYPE;
//    __IO uint32_t SEN_EXT_CFG;
//    __IO uint32_t SEN_SOFT_EN;
//    __IO uint32_t SEN_STATE;
//    __IO uint32_t SEN_BRIDGE;
//    __IO uint32_t SEN_SOFT_ATTACK_SOFT_ATTACK;
//    __IO uint32_t SEN_SOFT_LOCK;
//    __IO uint32_t SEN_ATTACK_CNT;
//    __IO uint32_t SEN_REG;
//    __IO uint32_t SEN_VG_DETECT;
//    __IO uint32_t SEN_RNG_INI;
//    __IO uint32_t RESERVED3[(0x0104 - 0x00EC) >> 2];
//    __IO uint32_t SEN_EN[13];
//    __IO uint32_t RESERVED4[(0x0134 - 0x011C) >> 2];
//    __IO uint32_t SEN_EXTS_START;
//    __IO uint32_t SEN_EXTS_LOCK;
//    __IO uint32_t SEN_ANAO;
//    __IO uint32_t SEN_RSVD;
//} BPU_MODULE_TypeDef;

//typedef struct
//{
//    __IO uint32_t KEY[32];
//    __IO uint32_t BPK_RDY;
//    __IO uint32_t BPK_CLR;
//    __IO uint32_t BPK_LRA;
//    __IO uint32_t BPK_LWA;
//    __IO uint32_t BPK_RR;
//    __IO uint32_t BPK_LR;
//    __O  uint32_t BPK_SCR;
//    __I  uint32_t BPK_RSVD0;
//} BPK_TypeDef;

//typedef struct
//{
//    __IO uint32_t RTC_CS;
//    __IO uint32_t RTC_REF;
//    __IO uint32_t RTC_ARM;
//    __I  uint32_t RTC_TIM;
//    __O  uint32_t RTC_INTCLR;
//    __IO uint32_t OSC32K_CR;
//    __IO uint32_t RTC_ATTA_TIM;
//} RTC_TypeDef;


//typedef struct
//{
//    __O uint32_t SEN_EXT_TYPE;
//    __IO uint32_t SEN_EXT_CFG;
//    __IO uint32_t SEN_SOFT_EN;
//    __IO uint32_t SEN_STATE;
//    __IO uint32_t SEN_BRIDGE;
//    __IO uint32_t SEN_SOFT_ATTACK;
//    __IO uint32_t SEN_SOFT_LOCK;
//    __IO uint32_t SEN_ATTACK_CNT;
//    __IO uint32_t SEN_ATTACK_TYP;
//    __IO uint32_t SEN_VG_DETECT;
//    __IO uint32_t SEN_RNG_INI;
//    __O uint32_t RESERVED3[(0x0104 - 0x00EC)/4];
//    __IO uint32_t SEN_EN[19];
//    __IO uint32_t SEN_EXTS_START;
//    __IO uint32_t SEN_LOCK;
//} SEN_TypeDef;




/** 
  * @brief DCMI&IMG COP
  */


//typedef struct
//{
//      __IO uint32_t  VD_EN;//0x00
//      __IO uint32_t  TD_GD_EN;//0x04
//      __IO uint32_t  ISO_EN;//0x08
//      __IO uint32_t  RC_LD_EN;//0x0C
//      __IO uint32_t  TRM_CR1;//0x10
//      __IO uint32_t  VD_TD_OFFSET;//0x14
//      __IO uint32_t  RES[(0x1C - 0x14) >> 2];//0x18~0x1C
//      __IO uint32_t  INT_STAT;//0x20
//      __IO uint32_t  ATT_CR;//0x24
//      __IO uint32_t  PD_RNG_INT;//0x28
//      __IO uint32_t  ST_EN;//0x2C
//      __IO uint32_t  ST_ISR;//0x30
//      __IO uint32_t  RES2[(0x7C - 0x30) >> 2];
//      __IO uint32_t  SRAM_DKEY;//0x80
//      __IO uint32_t  SRAM_AKEY;//0x84
//      __IO uint32_t  CRAM_DKEY;//0x88
//      __IO uint32_t  CRAM_AKEY;//0x8C
//      __IO uint32_t  FLASH_DKEY;//0x90
//      __IO uint32_t  FLASH_AKEY;//0x94
//      __IO uint32_t  MPU_CFG;//0x98
//      __IO uint32_t  VTOR_REMAP;//0x9C
//      __IO uint32_t  DITHER;//0xA0
//      __IO uint32_t  POL_CR;//0xA4
//      __IO uint32_t  TD_VD_FLT;//0xA8
//      __IO uint32_t  JTAG_CR;//0xAC
//      __IO uint32_t  ROM_CR;//0xB0
//      __IO uint32_t  STARTUP_SEL;//0xB4
//      __IO uint32_t  MESH_LFSR_CFG;//0xB8
//      __IO uint32_t  MESH_FREQ;//0xBC
//      __IO uint32_t  RES3[(0xF4 - 0xBC) >> 2];//0xC0~0xF4
//      __IO uint32_t  FUSE_FLAG;//0xF8
//      __IO uint32_t  RC_TRIM_CR;//0xFC
//} SSC_TypeDef;



//typedef struct
//{
//    __IO uint32_t  TST_JTAG;
//    __IO uint32_t  TST_ROM;
//    __IO uint32_t  TST_FLASH;
//} MH_SMCU_TST_TypeDef;
//typedef struct
//{
//  __IO uint32_t RES[(0x20 - 0x00)/4];
//  __IO uint32_t CACHE_CS;//0x20
//  __IO uint32_t CACHE_REF;//0x24
//  __IO uint32_t RES1[(0x40 - 0x28)/4];
//  __IO uint32_t CACHE_BACK_DOOR;//0x40

//}CACHE_TypeDef;

#if defined ( __CC_ARM   )
#pragma no_anon_unions
#endif

/*@}*/ /* end of group <Device>_Peripherals */


/******************************************************************************/
/*                         Peripheral memory map                              */
/******************************************************************************/
/* ToDo: add here your device peripherals base addresses
         following is an example for timer                                    */
/** @addtogroup <Device>_MemoryMap <Device> Memory Mapping
  @{
*/

/* Peripheral and SRAM base address */
#define MHCARD_FLASH_BASE                       (0x00800000UL)                              /*!< (FLASH     ) Base Address */
#define MHCARD_FLASH_SIZE                       (512 * 1024) //512 KB
#define MHCARD_FLASH_END                        (MHCARD_FLASH_BASE + MHCARD_FLASH_SIZE)

#define MHCARD_SRAM_BASE                        (0x20000000UL)                              /*!< (SRAM      ) Base Address */
#define MHCARD_PERIPH_BASE                      (0x40000000UL)                              /*!< (Peripheral) Base Address */



#define MHCARD_EXT_FLASH_BASE                   (0x60000000UL)
#define MHCARD_EXT_RAM_BASE                     (0x80000000UL)

#define MHCARD_FLASH_SCRATCH_START              (0x00800000UL)
#define MHCARD_FLASH_SCRATCH_END                (0x00870000UL)

/*the flash end 64KB is can't sscratch*/
#define MHCARD_FLASH_SCRATCH_NOT_START          (0x00870000UL)
#define MHCARD_FLASH_SCRATCH_NOT_END            (0x00880000UL)          

#define SYSCTL_SCR_BASE       (0xE000ED10UL)
#define SYS_CTL_SCR           ((SYS_CTL_SCR_TYPE *)SYSCTL_SCR_BASE)
//sleep mode set reg
typedef struct
{
	volatile uint32_t rfu0        :1;
	volatile uint32_t sleeponexit :1;
	volatile uint32_t sleepdeep   :1;
	volatile uint32_t rfu1        :1;
	volatile uint32_t sevonpend   :1;
	     uint32_t rfu2        :27;
}SYS_CTL_SCR_BITS;
typedef union
{
	__IO uint32_t   sys_ctl_scr_all;
	SYS_CTL_SCR_BITS sys_ctl_scr_bit;
}SYS_CTL_SCR_TYPE;


#define FLASH_SIZE                              (1UL << 19)  //Program Data region:512KByte
#define FLASH_BASE                              (MHCARD_FLASH_BASE)

#define OTP_SIZE                                (1UL << 9)  //OTP Size:512Bytes
#define OTP_BASE                                (0x02000000UL)

#define SYS_SIZE                                (1UL << 12)    //System Security:4KBytes
#define SYS_BASE                                (0x03000000UL)

#define SRAM_SIZE                               (1UL << 16)
#define SRAM_BASE                               (0x20000000UL)

/* Peripheral memory map */
#define MHCARD_AHB_BASE                     (0x40000000UL)
#define MHCARD_APB_BASE                     (0x50000000UL)

/* AHB Peripheral map */
//#define SYSCFG_BASE                         (MHCARD_AHB_BASE + 0x5000)
//#define CRC_BASE                            (MHCARD_AHB_BASE + 0x2000)

//#define SM3_SHA_BASE                        (MHCARD_AHB_BASE + 0x3100)
//#define CACHE_BASE                          (MHCARD_AHB_BASE + 0x7000)

/* APB Peripheral map */

//#define GPIO_BASE                           (MHCARD_APB_BASE + 0x5000)
//#define UART_BASE                           (MHCARD_APB_BASE + 0x4000)

//#define WDG_BASE                            (MHCARD_APB_BASE + 0x1000)
//#define TIMM0_BASE                          (MHCARD_APB_BASE + 0x2000)
//#define SSC_BASE                            (MHCARD_APB_BASE + 0x3000)

/*@}*/ /* end of group <Device>_MemoryMap */


/******************************************************************************/
/*                         Peripheral declaration                             */
/******************************************************************************/
/* ToDo: add here your device peripherals pointer definitions
         following is an example for timer                                    */

/** @addtogroup <Device>_PeripheralDecl <Device> Peripheral Declaration
  @{
*/
//#define SYSCFG                                  ((SYSCFG_TypeDef *) SYSCFG_BASE)




//#define UART                                    ((UART_TypeDef *)UART_BASE)

//#define CRC                                     ((CRC_TypeDef *)CRC_BASE)

//#define GPIO                                    ((GPIO_TypeDef *)GPIO_BASE)

//#define WDT                                     ((WDT_TypeDef *)WDG_BASE)

//#define SSC                                     ((SSC_TypeDef *)SSC_BASE)   
//#define TST                                     ((MH_SMCU_TST_TypeDef *)TST_BASE)

//#define DCMI                                    ((DCMI_TypeDef *) DCMI_BASE)
//#define IMG_COP                                 ((IMG_COP_TypeDef *) IMG_COP_BASE)

//#define BPU                                     ((BPU_MODULE_TypeDef *)BPU_BASE)
//#define BPK                                     ((BPK_TypeDef *)BPU_BASE)
//#define RTC                                     ((RTC_TypeDef *)(BPU_BASE + 0xA0))
//#define SENSOR                                  ((SEN_TypeDef *)(BPU_BASE + 0xC0)) 
//#define TIMM0                                   ((TIM_Module_TypeDef *)TIMM0_BASE)

/** @addtogroup Exported_constants
  * @{
  */
  
  /** @addtogroup Peripheral_Registers_Bits_Definition
  * @{
  */
    
/******************************************************************************/
/*                         Peripheral Registers_Bits_Definition               */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*                              SYSCFG                                        */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for CLK_CFG register  *******************/
//#define CFG_TIM_SRC_CLK                         (BIT(13))            /*!< Data value */


/*******************  Bit definition for UART_IER register  *******************/



#if 0
/******************************************************************************/
/*                                                                            */
/*                Universal Asynchronous Receiver Transmitter                 */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for UART_RBR register  *******************/
#define UART_RBR_RBR                         ((uint32_t)0x01FF)            /*!< Data value */

/*******************  Bit definition for UART_THR register  *******************/
#define UART_THR_THR                         ((uint32_t)0x01FF)            /*!< Data value */

/*******************  Bit definition for UART_DLH register  *******************/
#define UART_DLH_DLH                         ((uint32_t)0x0FF)

/*******************  Bit definition for UART_DLL register  *******************/
#define UART_DLL_DLL                         ((uint32_t)0x0FF)

/*******************  Bit definition for UART_IER register  *******************/
#define UART_IER_ERBFI                          ((uint32_t)0x0001)
#define UART_IER_ETBEI                          ((uint32_t)0x0002)
#define UART_IER_ELSI                           ((uint32_t)0x0004)
#define UART_IER_EDSSI                          ((uint32_t)0x0008)
#define UART_IER_PTIME                          ((uint32_t)0x0080)

/*******************  Bit definition for UART_IIR register  *******************/
#define UART_IIR_IID                            ((uint32_t)0x0007)
#define UART_IIR_IID_0                          ((uint32_t)0x0001)
#define UART_IIR_IID_1                          ((uint32_t)0x0002)
#define UART_IIR_IID_2                          ((uint32_t)0x0004)
#define UART_IIR_IID_3                          ((uint32_t)0x0008)
#define UART_IIR_FIFOSE                         ((uint32_t)0x0060)
#define UART_IIR_FIFOSE_0                       ((uint32_t)0x0020)
#define UART_IIR_FIFOSE_1                       ((uint32_t)0x0040)

/*******************  Bit definition for UART_FCR register  *******************/
#define UART_FCR_FIFOE                          ((uint32_t)0x0001)
#define UART_FCR_RFIFOR                         ((uint32_t)0x0002)
#define UART_FCR_XFIFOR                         ((uint32_t)0x0004)
#define UART_FCR_DMAM                           ((uint32_t)0x0008)
#define UART_FCR_TET                            ((uint32_t)0x0030)
#define UART_FCR_TET_0                          ((uint32_t)0x0010)
#define UART_FCR_TET_1                          ((uint32_t)0x0020)
#define UART_FCR_RCVER                          ((uint32_t)0x00C0)
#define UART_FCR_RCVER_0                        ((uint32_t)0x0040)
#define UART_FCR_RCVER_1                        ((uint32_t)0x0080)

/*******************  Bit definition for UART_LCR register  *******************/
#define UART_LCR_DLS                            ((uint32_t)0x0003)
#define UART_LCR_DLS_0                          ((uint32_t)0x0001)
#define UART_LCR_DLS_1                          ((uint32_t)0x0002)
#define UART_LCR_STOP                           ((uint32_t)0x0004)
#define UART_LCR_PEN                            ((uint32_t)0x0008)
#define UART_LCR_EPS                            ((uint32_t)0x0010)
#define UART_LCR_SP                             ((uint32_t)0x0020)
#define UART_LCR_BC                             ((uint32_t)0x0040)
#define UART_LCR_DLAB                           ((uint32_t)0x0080)

/*******************  Bit definition for UART_MCR register  *******************/
#define UART_MCR_DTR                            ((uint32_t)0x0001)
#define UART_MCR_RTS                            ((uint32_t)0x0002)
#define UART_MCR_OUT1                           ((uint32_t)0x0004)
#define UART_MCR_OUT2                           ((uint32_t)0x0008)
#define UART_MCR_LB                             ((uint32_t)0x0010)
#define UART_MCR_AFCE                           ((uint32_t)0x0020)
#define UART_MCR_SIRE                           ((uint32_t)0x0040)

/*******************  Bit definition for UART_LSR register  *******************/
#define UART_LSR_DR                             ((uint32_t)0x0001)
#define UART_LSR_OE                             ((uint32_t)0x0002)
#define UART_LSR_PE                             ((uint32_t)0x0004)
#define UART_LSR_FE                             ((uint32_t)0x0008)
#define UART_LSR_BI                             ((uint32_t)0x0010)
#define UART_LSR_THRE                           ((uint32_t)0x0020)
#define UART_LSR_TEMT                           ((uint32_t)0x0040)
#define UART_LSR_PFE                            ((uint32_t)0x0080)

/*******************  Bit definition for UART_MSR register  *******************/
#define UART_MSR_DCTS                               ((uint32_t)0x0001)
#define UART_MSR_DDSR                               ((uint32_t)0x0002)
#define UART_MSR_TERI                               ((uint32_t)0x0004)
#define UART_MSR_DDCD                               ((uint32_t)0x0008)
#define UART_MSR_CTS                                ((uint32_t)0x0010)
#define UART_MSR_DSR                                ((uint32_t)0x0020)
#define UART_MSR_RI                                 ((uint32_t)0x0040)
#define UART_MSR_DCD                                ((uint32_t)0x0080)

/*******************  Bit definition for UART_SRBR register  *******************/
#define UART_SRBR_SRBR                          ((uint32_t)0x01FF)            /*!< Data value */

/*******************  Bit definition for UART_STHR register  *******************/
#define UART_STHR_STHR                          ((uint32_t)0x01FF)            /*!< Data value */

/*******************  Bit definition for UART_FAR register  *******************/
#define UART_FAR_FAR                            ((uint32_t)0x0001)

/*******************  Bit definition for UART_TFR register  *******************/
#define UART_TFR_TFR                            ((uint32_t)0x00FF)

/*******************  Bit definition for UART_RFW register  *******************/
#define UART_RFW_RFWD                           ((uint32_t)0x00FF)
#define UART_RFW_RFPE                           ((uint32_t)0x0100)
#define UART_RFW_RFFE                           ((uint32_t)0x0200)

/*******************  Bit definition for UART_USR register  *******************/
#define UART_USR_BUSY                               ((uint32_t)0x0001)
#define UART_USR_TFNF                               ((uint32_t)0x0002)
#define UART_USR_TFE                                ((uint32_t)0x0004)
#define UART_USR_RFNE                               ((uint32_t)0x0008)
#define UART_USR_RFF                                ((uint32_t)0x0010)

/*******************  Bit definition for UART_TFL register  *******************/
#define UART_TFL_TFL                                ((uint32_t)0x000F)

/*******************  Bit definition for UART_RFL register  *******************/
#define UART_RFL_RFL                                ((uint32_t)0x000F)

/*******************  Bit definition for UART_SRR register  *******************/
#define UART_SRR_UR                                 ((uint32_t)0x0001)
#define UART_SRR_RFR                                ((uint32_t)0x0002)
#define UART_SRR_XFR                                ((uint32_t)0x0004)

/*******************  Bit definition for UART_SRR register  *******************/
#define UART_SRR_UR                                 ((uint32_t)0x0001)

/*******************  Bit definition for UART_SRTS register  *******************/
#define UART_SRTS_SRTS                              ((uint32_t)0x0001)

/*******************  Bit definition for UART_SBCR register  *******************/
#define UART_SBCR_SBCR                              ((uint32_t)0x0001)

/*******************  Bit definition for UART_SDMAM register  *******************/
#define UART_SDMAM_SDMAM                            ((uint32_t)0x0001)

/*******************  Bit definition for UART_SFE register  *******************/
#define UART_SFE_SFE                                ((uint32_t)0x0001)

/*******************  Bit definition for UART_SRT register  *******************/
#define UART_SRT_SRT                                ((uint32_t)0x0003)
#define UART_SRT_SRT_0                              ((uint32_t)0x0001)
#define UART_SRT_SRT_1                              ((uint32_t)0x0002)

/*******************  Bit definition for UART_STET register  *******************/
#define UART_STET_STET                              ((uint32_t)0x0003)
#define UART_STET_STET_0                            ((uint32_t)0x0001)
#define UART_STET_STET_1                            ((uint32_t)0x0002)

/*******************  Bit definition for UART_HTX register  *******************/
#define UART_HTX_HTX                                ((uint32_t)0x0001)

/*******************  Bit definition for UART_DMASA register  *******************/
#define UART_DMASA_DMASA                            ((uint32_t)0x0001)

#endif
/******************************************************************************/
/*                                                                            */
/*                      Timer Control Unit Block                              */
/*                                                                            */
/******************************************************************************/
/**********  Bit definition for TIMER_CONTROL_REG register  *******************/
#define TIMER_CONTROL_REG_TIMER_ENABLE              (0x0001U)
#define TIMER_CONTROL_REG_TIMER_MODE                (0x0002U) 
#define TIMER_CONTROL_REG_TIMER_INTERRUPT           (0x0004U)
#define TIMER_CONTROL_REG_TIMER_PWM                 (0x0008U)
/*****************  Bit definition for IntStatus register  ********************/
#define TIMER_INT_STATUS_INTERRUPT                  (0x0001U)


/******************************************************************************/
/*                                                                            */
/*                          WDT Control Unit Block                            */
/*                                                                            */
/******************************************************************************/
/*****************  Bit definition for WDT_CR register  ***********************/
#define WDT_CR_WDT_EN                               ((uint32_t)0x0001)
#define WDT_CR_RMOD                                 ((uint32_t)0x0002)
/*****************  Bit definition for WDT_CCVR register  *********************/
#define WDT_CCVR_CCVR                               ((uint32_t)0xFFFFFFFF)
/*****************  Bit definition for WDT_CRR register  **********************/
#define WDT_CRR_CRR                                 ((uint32_t)0x00FF)
/*****************  Bit definition for WDT_STAT register  *********************/
#define WDT_STAT_INT                                ((uint32_t)0x0001)
/*****************  Bit definition for WDT_EOI register  **********************/
#define WDT_EOI_EOI                                 ((uint32_t)0x0001)
/*****************  Bit definition for WDT_RLD register  **********************/
#define WDT_RLD_RLD                                 ((uint32_t)0xFFFFFFFF)


#define MHCARD_READ_REG8(reg)                       (*(__IO uint8_t *) reg)
#define MHCARD_READ_REG16(reg)                      (*(__IO uint16_t *) reg)
#define MHCARD_READ_REG32(reg)                      (*(__IO uint32_t *) reg)
#define MHCARD_WRITE_REG8(reg, value)               (*(__IO uint8_t *) reg = value)
#define MHCARD_WRITE_REG16(reg, value)              (*(__IO uint16_t *) reg = value)
#define MHCARD_WRITE_REG32(reg, value)              (*(__IO uint32_t *) reg = value)
#define MHCARD_MODIFY_REG8(reg, clear_mask, set_mask)   \
        MHCARD_WRITE_REG8(reg, (((MHCARD_READ_REG8(reg)) & ~clear_mask) | set_mask))
#define MHCARD_MODIFY_REG16(reg, clear_mask, set_mask)   \
        MHCARD_WRITE_REG16(reg, (((MHCARD_READ_REG16(reg)) & ~clear_mask) | set_mask))
#define MHCARD_MODIFY_REG32(reg, clear_mask, set_mask)   \
        MHCARD_WRITE_REG32(reg, (((MHCARD_READ_REG32(reg)) & ~clear_mask) | set_mask))
      
//#define USE_STDPERIPH_DRIVER   1
//#define USE_FULL_ASSERT        1
//#define RUN_IN_RAM             1

/*@}*/ /* end of group <Device>_PeripheralDecl */

/*@}*/ /* end of group <Device>_Definitions */

#ifdef USE_STDPERIPH_DRIVER
  #include "mhcard_conf.h"
#endif

#ifdef __cplusplus
}
#endif

#endif  /* <Device>_H */
