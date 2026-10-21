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
extern void Image$$HAL_CONFIG_DATA_FILE$$Base;
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

#define EEPAGE_SIZE								    0x100            //=128
#define FLASH_PAGE_SIZE						        0x100

#define	HAL_EEPAGE_SIZE						        0x100
#define	HALF_HAL_EEPAGE_SIZE						(HAL_EEPAGE_SIZE >> 1)
#define	HAL_EEPAGE_MASK						        (HAL_EEPAGE_SIZE - 1)
#define NOR_HAL_EEPAGE_MASK				            (~(u32)(HAL_EEPAGE_SIZE - 1))

#define ERASEBLOCKSIZE								0x4
#define FLASH_BLOCK_SIZE						    U8_M_U16(ERASEBLOCKSIZE, HAL_EEPAGE_SIZE)
#define NOR_HAL_BLOCK_MASK				            (~(u32)(FLASH_BLOCK_SIZE - 1))

#define EEPAGE_MASK 							    0x00ff
#define NOR_EEPAGE_MASK 					        0xFFFFFF00
#define EECELL_SIZE								    0x04
#define EECELL_MASK								    0x0003
#define NOR_EECELL_MASK						        0xFFFFFFFC		//4 byte aligned

#define MAX_RX_LENGTH_PER_INT						259


#define EEPROM_BASE								    0x100C8000//0x100C7D00//0x100BD800//
#define	COS_CONFIG_START							0x100C8000//0x100C7D00//0x100BD800//
#define EEPROM_LIMIT                                0x101F9000

#define	HE_FLASH_BASE		                        0x0011C000								//HE_FLASH start addr
#define	HE_FLASH_SIZE		                        0x2000									//大小为8k
#define EEPROM_LIMIT_HE                             0x0011DFFF


#define ROM_BASE									0x10000000
#define ROM_MASK_OFFSET								0x001D6A00//0x00075800
#define	ROM_LIMIT									(0x101F3700-1)     // 依据sct里COSB C0DE的尾地址设置

#define CHIP_RAM_BASE									0x20006600
#define	CHIP_RAM_LIMIT									0x2000B600

extern memref MapTable_GetEndAddr(void);

#define EUICC_CFG_START									(memref)(COS_CONFIG_START + CONFIG_END_ALIGN) // MapTable_GetEndAddr() //这个返回的是一个页对齐地址



#define MAX_BACK_UP_PAGE                            0x26

/*==========================================================================*/

/***硬件对应寄存器长度***/
#define	CLK_CONFIG_CB_LENGTH						1
#define	CLK_CONFIG_CL_LENGTH						1
#define	CLK_CONFIG_CL_COM_LENGTH					1
#define	CLK_CONFIG_RSA_KEYGEN_LENGTH				1
#define	CLK_CONFIG_RSA_OPERATION_LENGTH				1


/*	RF WTXM 	*/
#define WTXM    									1

#endif /* SYSCONFIGTARGET_H_ */
