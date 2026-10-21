/**
  ******************************************************************************
  * @file    stm32f10x_flash.h
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    11-March-2011
  * @brief   This file contains all the functions prototypes for the FLASH
  *          firmware library.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MHSCPU_FLASH_H
#define __MHSCPU_FLASH_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhcard.h"
/**
  * @brief  FLASH Status
  */

/*************************hardware typedef**************************************/
#if (1)

typedef struct
{
    __IO uint32_t  CFG;//0x00
    __IO uint32_t  CS;//0x04
    __IO uint32_t  PROT;//0x08
    __IO uint32_t  ADDR;//0x0C
    __IO uint32_t  RES0;//0x10
    __IO uint32_t  RO;//0x14
    __IO uint32_t  ROL;//0x18
    __IO uint32_t  RO_INFO;//0x1C
    __IO uint32_t  ROL_INFO;//0x20
    __IO uint32_t  RO_RED;//0x24
    __IO uint32_t  ROL_RED;//0x28
    __IO uint32_t  RES1;//0x2C
    __IO uint32_t  RES2;//0x30
    __IO uint32_t  ECC_CORR;//0x34
    __IO uint32_t  ECC_ERR;//0x38
    __IO uint32_t  INFO_EN;//0x3C
    __IO uint32_t  IEL;//0x40
    __IO uint32_t  RO0_512B;//0x44
    __IO uint32_t  RO0L_512B;//0x48
    __IO uint32_t  RO1_512B;//0x4C
    __IO uint32_t  RO1L_512B;//0x50
    __IO uint32_t  RO_SEL;//0x54
	__IO uint32_t  RES3[(0xFC - 0x54)/4]; //0x9C~0x58
    __IO uint32_t  PG_BUF;//0x100~0x180
} FCU_TypeDef;

#define FCU_BASE                            (MHCARD_AHB_BASE + 0x4000)
#define FCU                                 ((FCU_TypeDef *)FCU_BASE)

/************************** register bits defines *******************************/


/************************** FCU_CFG *********************************************/
#define FCU_CFG_SLEEP_EN                                 (BIT(15))

#define FCU_CFG_ECC_Pos                                  (14)
#define FCU_CFG_ECC_Msk                                  (BIT(14))//0:open 1:close
#define FCU_CFG_ECC_OPEN                                 (0 << FCU_CFG_ECC_Pos)
#define FCU_CFG_ECC_CLOSE                                (1 << FCU_CFG_ECC_Pos)


#define FCU_CFG_PG_SIZE_Pos                                 (8)
#define FCU_CFG_PG_SIZE_Msk                                 (BITS(13,8))
#define FCU_CFG_PG_SIZE(size)                               (((size) << FCU_CFG_PG_SIZE_Pos) & FCU_CFG_PG_SIZE_Msk)


#define FCU_CFG_SM_OP_SEL_Pos                               (7)
#define FCU_CFG_SM_OP_SEL_Msk                               (BIT(7))//1:func module prov 0:bist prov.
#define FCU_CFG_SM_OP_SEL_MOD                               (1 << FCU_CFG_SM_OP_SEL_Pos)
#define FCU_CFG_SM_OP_SEL_BIST                              (0 << FCU_CFG_SM_OP_SEL_Pos)

#define FCU_CFG_ERR_INTP_MASK                               (BIT(5))
#define FCU_CFG_NVM_INTP_MASK                               (BIT(4))

#define FCU_CFG_OP_SEL_Pos                                  (0)
#define FCU_CFG_OP_SEL_Msk                                  (BITS(3,0))

/**************************************FCU CS *****************************************/
#define FCU_CS_OP_START                                     (BIT(31))
#define FCU_CS_PBUG_BUSY                                    (BIT(29))
#define FCU_CS_ECC_CORR                                     (BIT(4))
#define FCU_CS_ECC_ERR                                      (BIT(3))
#define FCU_CS_ILLEGAL                                      (BITS(2,1))
#define FCU_CS_IDLE                                         (BIT(0))

/*************************************FCU_INFO_EN *************************************/
#define FCU_INFO_EN_INFO_OP_EN                              (BIT(0))

/*************************************FCU_IEL     *************************************/
#define FCU_IEL_IEL                                         (BIT(0))
#define FCU_IEL_RO0L                                        (BIT(1))
#define FCU_IEL_RO1L                                        (BIT(2))

/*Flash unlock key*/
#define FLASH_KEY1              (0xABCD00A5)
#define FLASH_KEY2              (0x1234005A)

#endif//
/*************************hardware typedef**************************************/

/****************************************************
*in:
*   u8WordSize : 1~64
*/
#define IS_PROG_WORD_SIZE(SIZE)     ((SIZE) <= 64)


typedef enum
{
    FLASH_SMART_PROVIDER_MODULE = 1,
    FLASH_SMART_PROVIDER_BIST   = 2
} FlashSmartProvider;

#define IS_FLASH_SMART_PROVIDER(PROVIDER)       (((PROVIDER) == FLASH_SMART_PROVIDER_MODULE) || ((PROVIDER) == FLASH_SMART_PROVIDER_BIST))


#define FLASH_IT_ERR        (FCU_CFG_ERR_INTP_MASK)
#define FLASH_IT_NVM        (FCU_CFG_NVM_INTP_MASK)

#define IS_FLASH_IT(IT)         (((IT) & ~(FLASH_IT_ERR | FLASH_IT_NVM)) == 0)


typedef enum
{
    Flash_Op_ID_M_I_R_PROG_SMART_MODE1       =     (4 << FCU_CFG_OP_SEL_Pos),
    Flash_Op_ID_M_I_R_PROG_SMART_MODE2       =     (5 << FCU_CFG_OP_SEL_Pos),
    Flash_Op_ID_M_I_R_ERASE_PAGE_SMART       =     (6 << FCU_CFG_OP_SEL_Pos),
} Flash_Op_ID;

#define IS_FLASH_OP_ID(OPID)        (((OPID) == Flash_Op_ID_M_I_R_PROG_SMART_MODE1 )|| \
                                     ((OPID) == Flash_Op_ID_M_I_R_PROG_SMART_MODE2 )|| \
                                     ((OPID) == Flash_Op_ID_M_I_R_ERASE_PAGE_SMART ))


#define FLASH_PROG_MODE_NORMAL          (Flash_Op_ID_M_I_R_PROG_NORMAL)
#define FLASH_PROG_MODE_SMART1          (Flash_Op_ID_M_I_R_PROG_SMART_MODE1)
#define FLASH_PROG_MODE_SMART2          (Flash_Op_ID_M_I_R_PROG_SMART_MODE2)

#define FLASH_ERASE_MODE_PAGE_NORMAL    (Flash_Op_ID_M_I_R_ERASE_PAGE_NORMAL)
#define FLASH_ERASE_MODE_PAGE_SMART     (Flash_Op_ID_M_I_R_ERASE_PAGE_SMART)

typedef enum
{
    FLASH_BUSY = 1,
    FLASH_ERROR_PG,
    FLASH_ERROR_WRP,
    FLASH_ERROR_DATA,
    FLASH_COMPLETE,
    FLASH_TIMEOUT
}FLASH_Status;

typedef enum
{
 FLASH_READ_CORR = 0,
 FLASH_READ_ERR
}FLASH_Read_Status;

#define FLASH_OPERATE_PROBE        (BIT(0))

#define IS_FLASH_ADDRESS(ADDRESS) (((ADDRESS) > FLASH_BASE - 1) && ((ADDRESS) < FLASH_BASE + FLASH_SIZE))
#define FLASH_ROL_BLOCK_BIT         (14)
#define FLASH_ROL_BLOCK_SIZE        (BIT(FLASH_ROL_BLOCK_BIT))
#define FLASH_SROL_BLOCK_BIT        (9)
#define FLASH_SROL_BLOCK_SIZE       (BIT(FLASH_SROL_BLOCK_BIT))

#define FLASH_PAGE_SIZE             (512)

#define IS_OTP_ADDRESS(ADDRESS) (((ADDRESS) > OTP_BASE - 1) && ((ADDRESS) < OTP_BASE + OTP_SIZE))
#define OTP_ROL_BLOCK_BIT           (6)

#define IS_SYS_ADDRESS(ADDRESS) (((ADDRESS) > SYS_BASE - 1) && ((ADDRESS) < SYS_BASE + SYS_SIZE))
#define SYS_ROL_BLOCK_BIT           (7)

#define SECOND_LEVEL_GROUP_MAX      (1)

#define FLASH_UNPROTECT_ALL            (0xABCD00A5)

/*  new drivers  */
typedef struct
{
    uint32_t u32IS1;
    uint32_t u32IE1;   //In Start 1 ; In End 1
    uint32_t u32IS2;
    uint32_t u32IE2;   //In Start 2 ; In End 2
    uint32_t u32OS;//Out Start
    uint32_t u32OE;//Out End
} Intersection_TypeDef;


void FLASH_SleepConfig(FunctionalState state);
void FLASH_ProgWordSize(uint8_t u8WordSize);
void FLASH_ScrConfig(FunctionalState state);
void FLASH_SetSmartProv(FlashSmartProvider provider);
void FLASH_OprateSel(Flash_Op_ID opId);
void FLASH_InfoEn(FunctionalState state);
void FLASH_ITConfig(uint32_t u32Int, FunctionalState state);
void FLASH_EccConfig(FunctionalState state);
FlagStatus FLASH_StatusIsSet(uint32_t u32Flag);

uint32_t FLASH_CopyBufToPgBuf(void *pvSrc, uint16_t u16Len);
uint32_t FLASH_Operate(void);
uint32_t FLASH_GetIntersection(Intersection_TypeDef *pIn);
uint32_t FLASH_ProtectConfig(FunctionalState state);

uint32_t FLASH_SetRO(uint32_t u32Start, uint32_t u32Len, FunctionalState state);
uint32_t FLASH_SetROL(uint32_t u32Start, uint32_t u32Len);
uint32_t FLASH_SetSRO(uint32_t u32Start, uint32_t u32Len, uint32_t u32Group, FunctionalState state);
uint32_t FLASH_SetSROL(uint32_t u32Start, uint32_t u32Len, uint32_t u32Group);
uint32_t FLASH_GetSROSelAddr(uint32_t u32Group);
uint32_t FLASH_SetAllRO(uint32_t u32Start, uint32_t u32Len, FunctionalState state);
uint32_t FLASH_AreaExistRO(uint32_t u32Addr, uint32_t u32Len);


uint32_t FLASH_ProgramBuf(uint32_t u32Addr, void *pvBuf, uint32_t u32Len, uint8_t u8Mode);
uint32_t FLASH_ProgramWord(uint32_t u32Addr, uint32_t u32Dat, uint8_t u8Mode);
uint32_t FLASH_ErasePage(uint32_t u32Addr, uint8_t u8Mode);


#ifdef __cplusplus
}
#endif

#endif
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
