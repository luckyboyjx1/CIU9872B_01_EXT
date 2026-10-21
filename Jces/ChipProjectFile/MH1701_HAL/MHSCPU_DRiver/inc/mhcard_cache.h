/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_cache.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the CACHE firmware library
 *****************************************************************************/

#ifndef __MHSCPU_CACHE_H
#define __MHSCPU_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhcard.h"


/*************************hardware typedef**************************************/
typedef struct
{
  __IO uint32_t RES[(0x20 - 0x00)/4];
  __IO uint32_t CACHE_CS;//0x20
  __IO uint32_t CACHE_REF;//0x24
  __IO uint32_t RES1[(0x40 - 0x28)/4];
  __IO uint32_t CACHE_BACK_DOOR;//0x40
  __IO uint32_t RES2[(0x7C - 0x44)/4];
  __IO uint32_t VERSION;//0x7C
}CACHE_TypeDef;

#define CACHE_BASE                              (MHCARD_AHB_BASE + 0x7000)
#define CACHE                                   ((CACHE_TypeDef *)CACHE_BASE)
	
/************************** register bits defines *******************************/

#define CACHE_CS_BUSY                           (BIT(0))

#define CACHE_REF_REFRESH_ENABLE                (BIT(31))
#define CACHE_REF_REFRESH_ALL                   (BIT(30))
#define CACHE_REF_REFRESH_INDEX_Pos             (0)
#define CACHE_REF_REFRESH_INDEX_Msk             (BITS(23,0))
#define CACHE_REF_REFRESH_INDEX(index)          (((index) << CACHE_REF_REFRESH_INDEX_Pos) & CACHE_REF_REFRESH_INDEX_Msk)

/************************** function declaration ********************************/
#define CACHE_SIZE                            (512)

#define CACHE_ADDRESS_START                   ((uint32_t)MHCARD_FLASH_BASE)
#define CACHE_ADDRESS_END                     ((uint32_t)MHCARD_FLASH_END)

#define CACHE_CODE_BUS_OFFSET_POS               (0)
#define CACHE_CODE_BUS_OFFSET_WIDTH             (3)
#define CACHE_CODE_BUS_OFFSET_MASK              ((uint32_t)(0x01 << (CACHE_CODE_BUS_OFFSET_WIDTH + 1)) - 1)

#define CACHE_UNIT_SIZE                         ((uint32_t)0x01 << CACHE_CODE_BUS_OFFSET_WIDTH)

#define CACHE_CODE_BUS_SET_POS                  (3)
#define CACHE_CODE_BUS_SET_WIDTH                (4)
#define CACHE_CODE_BUS_SET_MASK                 ((uint32_t)(0x01 << (CACHE_CODE_BUS_SET_WIDTH + 1)) - 1)

#define CACHE_WAY_COUNT                         (CACHE_SIZE / (0x01 << CACHE_CODE_BUS_SET_WIDTH))

#define CACHE_CODE_BUS_TAG_POS                  (7)
#define CACHE_CODE_BUS_TAG_WIDTH                (19)
#define CACHE_CODE_BUS_TAG_MASK                 ((uint32_t)(0x01 << (CACHE_CODE_BUS_TAG_WIDTH + 1)) - 1)

#define CACHE_BUILD_INDEX_OFFSET(x)				(((x) & CACHE_CODE_BUS_OFFSET_MASK) << CACHE_CODE_BUS_OFFSET_POS)
#define CACHE_BUILD_INDEX_SET(x)				(((x) & CACHE_CODE_BUS_SET_MASK) << CACHE_CODE_BUS_SET_POS)
#define CACHE_BUILD_INDEX_TAG(x)				(((x) & CACHE_CODE_BUS_TAG_MASK) << CACHE_CODE_BUS_TAG_POS)
#define CACHE_ADDRESS_BUILD(base,index_set,tag_way,offset)   ((base) | CACHE_BUILD_INDEX_SET(index_set) \
                                                                     | CACHE_BUILD_INDEX_TAG(tag_way) \
                                                                     | CACHE_BUILD_INDEX_OFFSET(offset))

#define CACHE_TAG_NUM(x)						((x >> CACHE_CODE_BUS_TAG_POS) & CACHE_CODE_BUS_TAG_MASK)
#define CACHE_SET_NUM(x)						((x >> CACHE_CODE_BUS_SET_POS) & CACHE_CODE_BUS_SET_MASK)
#define CACHE_OFF_NUM(x)						((x >> CACHE_CODE_BUS_OFFSET_POS) & CACHE_CODE_BUS_OFFSET_MASK)


uint32_t CACHE_Clean(CACHE_TypeDef *Cache, uint32_t u32Addr, uint32_t u32Len);
void CACHE_CleanAll(CACHE_TypeDef *Cache);

#ifdef __cplusplus
}
#endif

#endif 


/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
