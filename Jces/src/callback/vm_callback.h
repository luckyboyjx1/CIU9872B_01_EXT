/*
 * .h
 *
 *  Created on: 2016年4月8日
 *      Author: huangzhigao
 */

#ifndef VM_CALLBACK_H_
#define VM_CALLBACK_H_

#include "ecpDefine.h"
#include "boot_callback.h"
#include "profile.h"

////////////////////////////////////
//要上层实现
////////////////////////////////////
//void (* vmjd_DebugComponentInit)(PX8 buffer, u16 offset);
//boolean (* vmjd_DebugComponentProcess)(PX8 buffer, u16 offset, u16 len);
//boolean (* vmjd_DebugComponentpostProcess)(u8 pkgId, memref classCmpAddr, memref methodCmpAddr);
//void (* vmjd_deletePackageDebugComponent)(u8 pkgId);
//void (* vmjd_Deallocate)(void);
//
//void (* vmjd_loadSysPkg)();
//void (* vmjd_vmjd_initJcdt)(void);
//void (* vmjd_Interpreter1)(memref curJPC);
//void (* vmjd_Interpreter2)(memref curJPC, u8 byteCode);

////////////////////////////////////
//供上层使用
////////////////////////////////////
extern void romInit();
extern void eepConfigInit();
extern void ramReset();
extern void initVmCallback();


extern u32 vm_PACKAGE_LOCATION_TABLE_ADDRESS();
extern u32 vm_DEBUG_COMPONENT_PKG_COUNT();

extern memref vm_GET_ARRAY_DATA(PF8 array_ref);
extern u16 vm_GET_ARRAY_LENGTH(PF8 array_ref);
extern memref vm_GET_OBJECT_FIELD_REF(PM8 objref, u16 token);

extern	u8  vm_MEMREADU8(PM8 addr);
extern	u16 vm_MEMREADU16(PM8 addr);
extern	u32 vm_MEMREADU32(PM8 addr);
extern  u8  vm_NVMREADU8(PN8 addr);
extern  u16 vm_NVMREADU16(PN8 addr);
extern  u32 vm_NVMREADU32(PN8 addr);

extern	void  vm_MEMCOPYATOMIC(PM8 dest, PM8 src, u16 length);
extern  memref 	vm_GETPACKAGECOMPONENTLOCATION(u8 pkgID, u8 compID);
extern  u16 	vm_JCVMCONFIG_GETMAXENTRYCOUNTINMAP(void);
extern  memref vm_LOCATEOBJECTINMEMORY(jref objectId);
extern  memref vm_GETENTITYCONTENT(jref objectId);

extern  PM8 vm_GETDEBUGMASK();
#endif /* SRC_CALLBACK_VM_CALLBACK_H_ */
