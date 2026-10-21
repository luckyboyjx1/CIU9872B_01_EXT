/*
 * JCDebug.h
 *
 *  Created on: 2014Äê12ÔÂ24ÈÕ
 *      Author: mowenpan
 */

#ifndef JCDEBUG_H_
#define JCDEBUG_H_

#include <cardAPI.h>

//#define JCDEBUG_PKG_MAX 	128

//load debug component
extern void JavaDebug_loadSysPkg();
extern void JavaDebug_initJcdt();

extern void setJCDebugComponentSize(u16 size);
extern void JCDebugComponentInit(PX8 buffer, u16 offset);
extern boolean JCDebugComponentProcess(PX8 buffer, u16 offset, u16 len);
extern boolean JCDebugComponentpostProcess(u8 pkgId, memref classComponentAddr, memref methodcomponentAddr);
extern void JCDeletePackageDebugComponent(u8 pkgId);
extern void JavaDebug_Interpreter2(memref curJPC, u8 byteCode);
extern void JavaDebug_Interpreter1(memref curJPC);
extern void JCDebugDeallocate();
extern void initCallback();

#endif /* JCDEBUG_H_ */
