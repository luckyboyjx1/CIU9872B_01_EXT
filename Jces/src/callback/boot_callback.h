/*
 * boot_collback.h
 *
 *  Created on: 2016年4月10日
 *      Author: huangzhigao
 */

#ifndef CALLBACK_BOOT_CALLBACK_H_
#define CALLBACK_BOOT_CALLBACK_H_

#include "ecpDefine.h"
#include "hal_callback.h"

#ifndef MAP_ENTRY_SIZE
#define MAP_ENTRY_SIZE							((u32)4)
#endif

////////////////////////////////////
//要上层实现
////////////////////////////////////
extern u16 getPrivateData(PN8 apduBuffer,u8 type);
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

#endif /* CALLBACK_BOOT_CALLBACK_H_ */
