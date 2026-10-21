/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*
 * @(#)memory.h	1.55 03/09/24
 */

#ifndef   __MEMORY_H__
#define   __MEMORY_H__

/**
 * This module manages the memory heaps (in E2PROM and RAM) and
 * reading from and writing to the various memory spaces.
 * @module Memory
 **/

/*#include "target.h"		*/
#include "MemBackUp.h"
#include "temp_ob_manager.h"
#include "context.h"
#include "mask.h"
#include "global_variable.h"
#include "MemOperation.h"
#include "JcvmConfig.h"
#include "MemBackUp.h"
#include "IoBuffer.h"
#include "NvmManager.h"
#include "RamManager.h"

#define dTP_NUM											U8_SHR_U8(NUM_GROUPS, 3)

/* =============================START OF NVM INFORMATION AREA==================================================== */

//#define E2P_VM_REGISTERS_START 						E2PBase

/* calculate current available heap start after first E2P Heap Table */
//#define E2P_Start  								     E2P_IMAGE_START //+ EEPROM_IMAGE_SIZE

/* use this constant to represent out-of-memory errors. */
#define NO_MEMORY_AVAILABLE 						0xFFFF

/* Number of entries in a EEPROM Table */
//#define MAX_E2P_TBL_SIZE 							64

/* The miniumum size of a remaining E2P memory segment after space has been
 * allocated from a segment.
 */
#define MIN_E2P_SIZE 								8

/**
 * Constants representing DTR spaces in the card.
 */
#define DTR_AVAILABLE								0x00
#define DTR_INUSE									0x01
#define DTR_DISABLED								0x02
#define DTR_STATE_MASK  							0x03

/**
 * Beginning of allocated RAM memory
 * @type memref */
#define rambase 									RAMBase

/**
 * The size of the Temporary Array memory pool
 */

#define Temporary_Array_SIZE 						U8_M_U8(MAX_TEMPORARY_OBJECTS, U8_A_U8(SIZE_PRIMITIVE_ARRAY_HEADER, 2))

/**
 * Offset of the Beginning of Transient RAM allocated from RAM memory
 * @type memref
 */
#define Transient_BASE 								RAMBase

/**
 * Maximum amount of Transient RAM allocated from RAM memory, minus the STACK
 * space and temporary object space
 * @type memref
 */
//#define Transient_SIZE 								(RAMSize - (MAX_TEMPORARY_OBJECTS * TEMP_OBJECT_HEADER_LENGTH))

#define Transient_RTR_BASE 							(U32_A_U32(Transient_BASE, U32_S_U8(Transient_SIZE, 1)))
/**
 * Beginning of allocated temporary array memory
 * @type memref
 */
#define Temporary_Array_BASE 						U32_A_U32(Transient_BASE, Transient_SIZE)

/*****************************************************************************
 * Allocating memory in the different memory pools
 *****************************************************************************/
// initialize the ram space and Initialize Logical Channels RAM management structure
extern  void init_RAM(void);
extern  void getData_FF20(void);  //for get data command case FF20

extern  void begin_transaction(u8 isMultiApduProcess);
/*
 * VM层终止事物
 * */
extern  void abort_transaction(u1 isReset);
/*
 * VM层事物提交
 * */
extern  void commit_transaction(void);


extern  void reset_transaction_buffer(void);

//extern  void abortTransactionByJCRE(void);

#endif /* __CREF_MEMORY_H__ */
