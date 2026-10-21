/*
 * SysConfigTarget.h
 *
 *  Created on: 2014-2-20
 *      Author: yangfajun
 */

#ifndef SYSCONFIGTARGET_H_
#define SYSCONFIGTARGET_H_

#include "DataOperation.h"
//#include "SysConfig.h"

//===========================================================================
//  JAVA  VM Addr config
//  If you change the value of E2PBase or ROMBase, be sure to update mask.cfg
//  accordingly. The Mask Generator depends on these values to correctly link
//  the pre-installed packages.
//===========================================================================
/********************************************* memory *************************************/
/********************************************* commom *************************************/
#define TRUE     									1
#define FALSE    									0

#define SIZE_U8   								    1
#define SIZE_U16  								    2
#define SIZE_U32  								    4

#define EEPAGE_SIZE								    0x200            //=128
#define FLASH_PAGE_SIZE						        0x200

#define	HAL_EEPAGE_SIZE						        0x200
#define	HALF_HAL_EEPAGE_SIZE						(HAL_EEPAGE_SIZE >> 1)
#define	HAL_EEPAGE_MASK						        (HAL_EEPAGE_SIZE - 1)
#define NOR_HAL_EEPAGE_MASK				            (~(u32)(HAL_EEPAGE_SIZE - 1))

#define ERASEBLOCKSIZE								0x4
#define FLASH_BLOCK_SIZE						    U8_M_U16(ERASEBLOCKSIZE, HAL_EEPAGE_SIZE)
#define NOR_HAL_BLOCK_MASK				            (~(u32)(FLASH_BLOCK_SIZE - 1))

#define EEPAGE_MASK 							    0x01ff
#define NOR_EEPAGE_MASK 					        0xFFFFFE00
#define EECELL_SIZE								    0x04
#define EECELL_MASK								    0x0003
#define NOR_EECELL_MASK						        0xFFFFFFFC		//4 byte aligned

#define MAX_RX_LENGTH_PER_INT						1//259


#define EEPROM_BASE								    0x00121400//0x0C095800
#define	COS_CONFIG_START							0x00121400//0x0C095800
#define EEPROM_LIMIT                                0x001217FF

#define	HE_FLASH_BASE		                        0x0011C000								//HE_FLASH start addr
#define	HE_FLASH_SIZE		                        0x2000									//???8k
#define EEPROM_LIMIT_HE                             0x0011DFFF


#define ROM_BASE									0x00000000L
#define ROM_MASK_OFFSET								0x0002E000//0x00075800
#define	ROM_LIMIT									(EEPROM_BASE-1)     //(EEPROM_BASE - 1)    //0x00055000L
//??????, ?JCVM_RAM_LIMIT???CHIP_RAM_LIMIT,JCVM_RAM_BASE?CHIP_RAM_BASE
#define CHIP_RAM_BASE									0x20004850
#define	CHIP_RAM_LIMIT									0x2000BFFF

extern memref MapTable_GetEndAddr(void);

#define EUICC_CFG_START									(memref)(COS_CONFIG_START + CONFIG_END_ALIGN) // MapTable_GetEndAddr() //?????????????



#define MAX_BACK_UP_PAGE                            0x26

/*==========================================================================*/

/***?????????***/
#define	CLK_CONFIG_CB_LENGTH						1
#define	CLK_CONFIG_CL_LENGTH						1
#define	CLK_CONFIG_CL_COM_LENGTH					1
#define	CLK_CONFIG_RSA_KEYGEN_LENGTH				1
#define	CLK_CONFIG_RSA_OPERATION_LENGTH				1


/*	RF WTXM 	*/
#define WTXM    									1

#endif /* SYSCONFIGTARGET_H_ */
