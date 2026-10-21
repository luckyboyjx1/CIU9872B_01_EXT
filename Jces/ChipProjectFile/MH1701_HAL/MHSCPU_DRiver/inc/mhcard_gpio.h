#ifndef __MHSCPU_GPIO_H
#define __MHSCPU_GPIO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhcard.h"
#include "mhcard_conf.h"

typedef struct
{
    __IO uint32_t IOD;//0x00
    __IO uint32_t BSR;//0x04
    __IO uint32_t CON;//0x08
    __IO uint32_t MODE_L;//0x0c
    __IO uint32_t MODE_H;//0x10
    __IO uint32_t INT_TYPE;//0x14
    __IO uint32_t RES;//0x18
    __IO uint32_t INT_STAT;//0x1C
    __IO uint32_t WAKEUP_EN;//0x20
} GPIO_TypeDef;

#define GPIO_BASE                           (MHCARD_APB_BASE + 0x5000)
#define GPIO                                ((GPIO_TypeDef *)GPIO_BASE)


#define GPIO_Pin_0                 ((uint16_t)0x0001)  /*!< Pin 0 selected */
#define GPIO_Pin_1                 ((uint16_t)0x0002)  /*!< Pin 1 selected */
#define GPIO_Pin_2                 ((uint16_t)0x0004)  /*!< Pin 2 selected */
#define GPIO_Pin_3                 ((uint16_t)0x0008)  /*!< Pin 3 selected */
#define GPIO_Pin_4                 ((uint16_t)0x0010)  /*!< Pin 4 selected */
#define GPIO_Pin_5                 ((uint16_t)0x0020)  /*!< Pin 5 selected */
#define GPIO_Pin_6                 ((uint16_t)0x0040)  /*!< Pin 6 selected */
#define GPIO_Pin_7                 ((uint16_t)0x0080)  /*!< Pin 7 selected */
#define GPIO_Pin_All               ((uint16_t)0x00ff)  /*!< Pin All selected */

#define IS_GPIO_PIN(PIN) (((((PIN) & ~(uint16_t)0xFF)) == 0x00) && ((PIN) != (uint16_t)0x00))

#define IS_GET_GPIO_PIN(PIN)			(((PIN) == GPIO_Pin_0) || \
                                        ((PIN) == GPIO_Pin_1) || \
                                        ((PIN) == GPIO_Pin_2) || \
                                        ((PIN) == GPIO_Pin_3) || \
                                        ((PIN) == GPIO_Pin_4) || \
                                        ((PIN) == GPIO_Pin_5) || \
                                        ((PIN) == GPIO_Pin_6) || \
                                        ((PIN) == GPIO_Pin_7))

#define GPIO_IOD_DAT_IN_Pos             (8)
#define GPIO_IOD_DAT_IN_Msk             (0xFF << GPIO_IOD_DAT_IN_Pos)
#define GPIO_IOD_DAT_OUT_Pos            (0)
#define GPIO_IOD_DAT_OUT_Msk            (0xFF << GPIO_IOD_DAT_OUT_Pos)

#define GPIO_BSR_BIT_CLR_Pos            (8)
#define GPIO_BSR_BIT_CLR_Msk            (0xFF << GPIO_BSR_BIT_CLR_Pos)
#define GPIO_BSR_BIT_SET_Pos            (0)
#define GPIO_BSR_BIT_SET_Msk            (0xFF << GPIO_BSR_BIT_SET_Pos)
/**
  *GPIO_CON
  */
#define GPIO_CON_PA0_Pos                (0)
#define GPIO_CON_PA0_Msk                (0x3 << GPIO_CON_PA0_Pos)//(BITS(1,0))

#define GPIO_CON_PA1_Pos                (2)
#define GPIO_CON_PA1_Msk                (0x3 << GPIO_CON_PA1_Pos)//(BITS(3,2))

#define GPIO_CON_PA2_Pos                (4)
#define GPIO_CON_PA2_Msk                (0x3 << GPIO_CON_PA2_Pos)//(BITS(5,4))

#define GPIO_CON_PA3_Pos                (6)
#define GPIO_CON_PA3_Msk                (0x3 << GPIO_CON_PA3_Pos)//(BITS(7,6))

#define GPIO_CON_PA4_Pos                (8)
#define GPIO_CON_PA4_Msk                (0x3 << GPIO_CON_PA4_Pos)//(BITS(9,8))

#define GPIO_CON_PA5_Pos                (10)
#define GPIO_CON_PA5_Msk                (0x3 << GPIO_CON_PA5_Pos)//(BITS(9,8))
#define GPIO_CON_PA6_Pos                (12)
#define GPIO_CON_PA6_Msk                (0x3 << GPIO_CON_PA6_Pos)//(BITS(9,8))
#define GPIO_CON_PA7_Pos                (14)
#define GPIO_CON_PA7_Msk                (0x3 << GPIO_CON_PA7_Pos)//(BITS(9,8))

#define GPIO_CON_IN_PULL_NONE           (0x00)
#define GPIO_CON_IN_PULL_UP             (0x01)
#define GPIO_CON_IN_PULL_DOWN           (0x02)
#define GPIO_CON_OUT                    (0x03)

/**
  *GPIO_MODE
  */
#define GPIO_MODE_L_PA0_Pos         (0)
#define GPIO_MODE_L_PA0_Msk         (0xFFUL << GPIO_MODE_L_PA0_Pos)
#define GPIO_MODE_L_PA1_Pos         (8)
#define GPIO_MODE_L_PA1_Msk         (0xFFUL << GPIO_MODE_L_PA1_Pos)
#define GPIO_MODE_L_PA2_Pos         (16)
#define GPIO_MODE_L_PA2_Msk         (0xFFUL << GPIO_MODE_L_PA2_Pos)
#define GPIO_MODE_L_PA3_Pos         (24)
#define GPIO_MODE_L_PA3_Msk         (0xFFUL << GPIO_MODE_L_PA3_Pos)

#define GPIO_MODE_H_PA4_Pos         (0)
#define GPIO_MODE_H_PA4_Msk         (0xFFUL << GPIO_MODE_H_PA4_Pos)
#define GPIO_MODE_H_PA5_Pos         (8)
#define GPIO_MODE_H_PA5_Msk         (0xFFUL << GPIO_MODE_H_PA5_Pos)
#define GPIO_MODE_H_PA6_Pos         (16)
#define GPIO_MODE_H_PA6_Msk         (0xFFUL << GPIO_MODE_H_PA6_Pos)
#define GPIO_MODE_H_PA7_Pos         (24)
#define GPIO_MODE_H_PA7_Msk         (0xFFUL << GPIO_MODE_H_PA7_Pos)

#define GPIO_MODE_GPIO                              (0 ) 
#define GPIO_MODE_SPI_SLV_CLK                       (39) 
#define GPIO_MODE_SPI_SLV_CSN                       (40) 
#define GPIO_MODE_SPI_SLV_SDI                       (41) 
#define GPIO_MODE_SPI_SLV_SDO                       (42) 
#define GPIO_MODE_SPI_MST_CLK                       (43) 
#define GPIO_MODE_SPI_MST_CSN                       (44) 
#define GPIO_MODE_SPI_MST_SDI                       (45) 
#define GPIO_MODE_SPI_MST_SDO                       (46) 
#define GPIO_MODE_SCL                               (50) 
#define GPIO_MODE_SDA                               (51) 


/* GPIO_INT_TYPE */
#define GPIO_INT_TYPE_PA0_Pos                       (0)
#define GPIO_INT_TYPE_PA0_Msk                       (0x3 << GPIO_INT_TYPE_PA0_Pos)
#define GPIO_INT_TYPE_PA1_Pos                       (2)
#define GPIO_INT_TYPE_PA1_Msk                       (0x3 << GPIO_INT_TYPE_PA1_Pos)
#define GPIO_INT_TYPE_PA2_Pos                       (4)
#define GPIO_INT_TYPE_PA2_Msk                       (0x3 << GPIO_INT_TYPE_PA2_Pos)
#define GPIO_INT_TYPE_PA3_Pos                       (6)
#define GPIO_INT_TYPE_PA3_Msk                       (0x3 << GPIO_INT_TYPE_PA3_Pos)
#define GPIO_INT_TYPE_PA4_Pos                       (8)
#define GPIO_INT_TYPE_PA4_Msk                       (0x3 << GPIO_INT_TYPE_PA4_Pos)
#define GPIO_INT_TYPE_PA5_Pos                       (10)
#define GPIO_INT_TYPE_PA5_Msk                       (0x3 << GPIO_INT_TYPE_PA5_Pos)
#define GPIO_INT_TYPE_PA6_Pos                       (12)
#define GPIO_INT_TYPE_PA6_Msk                       (0x3 << GPIO_INT_TYPE_PA6_Pos)
#define GPIO_INT_TYPE_PA7_Pos                       (14)
#define GPIO_INT_TYPE_PA7_Msk                       (0x3 << GPIO_INT_TYPE_PA7_Pos)

#define GPIO_INT_TYPE_NONE                          (0x00)
#define GPIO_INT_TYPE_RISING                        (0x01)
#define GPIO_INT_TYPE_FALLING                       (0x02)
#define GPIO_INT_TYPE_RISING_FALLING                (0x03)

/* GPIO_INT_STATE */




//Interrupt State Mask Select
#define GPIO_INTP_STAT_Mask_0      BIT(0)
#define GPIO_INTP_STAT_Mask_1      BIT(1)
#define GPIO_INTP_STAT_Mask_2      BIT(2)
#define GPIO_INTP_STAT_Mask_3      BIT(3)
#define GPIO_INTP_STAT_Mask_4      BIT(4)
#define GPIO_INTP_STAT_Mask_5      BIT(5)
#define GPIO_INTP_STAT_Mask_6      BIT(6)
#define GPIO_INTP_STAT_Mask_7      BIT(7)
/** @addtogroup	SCPU_StdPeriph_Driver
  * @{
  */

/** @addtogroup GPIO pin selection
  * @{
  */
#define PA0_Mode   (BIT(0))
#define PA1_Mode   (BIT(8))
#define PA2_Mode   (BIT(16))
#define PA3_Mode   (BIT(24))
     


typedef enum
{    
	GPIO_Remap_IO  =  0,
	GPIO_Remap_SPI_SLV_CLK = 39,
	GPIO_Remap_SPI_SLV_CSN,
	GPIO_Remap_SPI_SLV_SDI,
	GPIO_Remap_SPI_SLV_SDO,
	GPIO_Remap_SPI_MST_CLK,
	GPIO_Remap_SPI_MST_CSN,
	GPIO_Remap_SPI_MST_SDI,
	GPIO_Remap_SPI_MST_SDO,
    GPIO_Remap_SCL = 50,
    GPIO_Reamp_SDA,
}GPIO_RemapTypeDef;

#define IS_GPIO_REMAP_MODE(MODE)			(((MODE) == GPIO_Remap_IO) || \
											((MODE) == GPIO_Remap_SPI_SLV_CLK) || \
											((MODE) == GPIO_Remap_SPI_SLV_CSN) || \
											((MODE) == GPIO_Remap_SPI_SLV_SDI) || \
											((MODE) == GPIO_Remap_SPI_SLV_SDO) || \
											((MODE) == GPIO_Remap_SPI_MST_CLK) || \
											((MODE) == GPIO_Remap_SPI_MST_CSN) || \
											((MODE) == GPIO_Remap_SPI_MST_SDI) || \
											((MODE) == GPIO_Remap_SPI_MST_SDO) || \
                                            ((MODE) == GPIO_Remap_SCL) || \
                                            ((MODE) == GPIO_Reamp_SDA))

/** @defgroup GPIO_Exported_Types
  * @{
  */

/** 
  * @brief  Configuration Mode enumeration  
  */
typedef enum
{ 
	GPIO_Mode_IN           = 0x00,
	GPIO_Mode_IN_PU		   = 0x01,  //Input pull up
	GPIO_Mode_IN_PD 	   = 0x02,  //Input pull down
	GPIO_Mode_OUT 	       = 0x03
}GPIO_ModeTypeDef;
#define IS_GPIO_MODE(MODE)				              (((MODE) == GPIO_Mode_IN) || \
										              ((MODE) == GPIO_Mode_IN_PU) || \
										              ((MODE) == GPIO_Mode_IN_PD) || \
										              ((MODE) == GPIO_Mode_OUT))


/** 
  * @brief  GPIO Interrupt  definition  
  */
typedef enum
{
  No_INPT = 0x00,
  RE_INPT = 0x01,   //rising edge trigger
  TE_INPT = 0x02,   //trailing edge trigger
  BE_INPT = 0x03    //Bilateral edge trigger
}GPIO_InptTypeDef;

#define IS_GPIO_INT_TYPE(TYPE)			              (((TYPE) == No_INPT) || \
										              ((TYPE) == RE_INPT) || \
										              ((TYPE) == TE_INPT) || \
										              ((TYPE) == BE_INPT))


/** 
  * @brief  GPIO Init structure definition  
  */
typedef struct
{
	uint8_t GPIO_Pin;             /*!< Specifies the GPIO pins to be configured.
									  This parameter can be any value of @ref GPIO_pins_define */	
	GPIO_ModeTypeDef GPIO_Mode;
	GPIO_RemapTypeDef GPIO_Remap;
	
}GPIO_InitTypeDef;


/** 
  * @brief  Bit_SET and Bit_RESET enumeration  
  */
typedef enum
{ 
	Bit_RESET = 0,
	Bit_SET
}BitAction;

/**
  * @}
  */

/** @defgroup GPIO_Exported_Constants
  * @{
  */


/** @defgroup GPIO_pins_define 
  * @{
  */






void GPIO_DeInit(void);
void GPIO_Init(GPIO_InitTypeDef *GPIO_InitStruct);

void GPIO_StructInit(GPIO_InitTypeDef* GPIO_InitStruct);
uint8_t GPIO_ReadInputDataBit(uint8_t GPIO_Pin);
uint8_t GPIO_ReadInputData(void);
uint8_t GPIO_ReadOutputDataBit(uint8_t GPIO_Pin);
uint8_t GPIO_ReadOutputData(void);
void GPIO_SetBits(uint8_t GPIO_Pin);
void GPIO_ResetBits(uint8_t GPIO_Pin);
void GPIO_WriteBit(uint8_t GPIO_Pin, BitAction BitVal);
void GPIO_Write(uint8_t PortVal);
void GPIO_PullUpCmd(uint8_t GPIO_Pin, FunctionalState NewState);
void GPIO_ModeConfig(uint8_t GPIO_Pin, GPIO_ModeTypeDef MODE);
void GPIO_RemapConfig(uint8_t GPIO_Pin, GPIO_RemapTypeDef Mode);

void GPIO_InterConfig(uint8_t GPIO_Pin, GPIO_InptTypeDef Type);
void GPIO_InterClear(uint8_t GPIO_Pin);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif 

/**************************      (C) COPYRIGHT    *****END OF FILE****/
