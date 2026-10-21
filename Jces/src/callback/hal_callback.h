/*
 * hal_callback.h
 *
 *  Created on: 2019年12月16日
 *      Author: jiangxiaohui
 */

#ifndef CALLBACK_HAL_CALLBACK_H_
#define CALLBACK_HAL_CALLBACK_H_


#include "ecpDefine.h"

#define VM_WITH_DEBUG							1

#ifndef MAP_ENTRY_SIZE
#define MAP_ENTRY_SIZE							((u32)4)
#endif

#ifndef SINGLE_APDU_PROCESS
	#define SINGLE_APDU_PROCESS								1
	#define MULTIL_APDU_PROCESS								2
#endif

#ifndef ECP_COPY_TYPE_ATOMIC
	#define ECP_COPY_TYPE_RAM_2_RAM							0x1A
	#define ECP_COPY_TYPE_MEM_2_RAM							0xA1
	#define ECP_COPY_TYPE_ROM_2_RAM							0x3A
	#define	ECP_COPY_TYPE_ATOMIC							0x5A
	#define	ECP_COPY_TYPE_NOATOMIC							0xA5
#endif

////////////////////////////////////
//要上层实现
////////////////////////////////////
extern u8 * PhIso14443L3AtqaB1b8WholeByteL;

////////////////////////////////////
//要上层实现
////////////////////////////////////
//extern u16 getPrivateData(PN8 apduBuffer);
extern PN8 GetUsedAddrNO(void);
extern u16 MapTable_GetTopIndex(void);
extern memref JcvmConfig_GetSaveTransFlagAreaBase();

extern PN8  MAP;

u16 (* VM_NVM_alloc)(u32 pSize, u8 pkgID);
memref (* VM_MapTable_GetEntityContent)(u16 index);


////////////////////////////////////
//供上层使用
////////////////////////////////////
void initBootCallback();

#endif /* CALLBACK_HAL_CALLBACK_H_ */
