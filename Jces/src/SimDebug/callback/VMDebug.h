/*
 * VMDebug.h
 *
 *  Created on: 2014Äê12ÔÂ25ÈÕ
 *      Author: mowenpan
 */

#ifndef VMDEBUG_H_
#define VMDEBUG_H_

#include "DataOperation.h"
#include "profile.h"
#include "hal_callback.h"
#define gotoResetHandler()	hw_gotoResetHandler()
#if VM_WITH_DEBUG
	extern void JavaDebug_DebugComponentInit();
	extern boolean JavaDebug_DebugComponentProcess();
	extern boolean JavaDebug_DebugComponentpostProcess();
	extern void JavaDebug_deletePackageDebugComponent(u8 pkgID);
	extern void JavaDebug_Deallocate();
	extern void JavaDebug_VM_loadSysPkg();
	extern void JavaDebug_VM_initJcdt();
	extern void JavaDebug_VM_Interpreter1(memref curJPC);
	extern void JavaDebug_VM_Interpreter2(memref curJPC, u8 byteCode);

	#define DebugComponentInit 				JavaDebug_DebugComponentInit
	#define DebugComponentProcess			JavaDebug_DebugComponentProcess
	#define DebugComponentpostProcess		JavaDebug_DebugComponentpostProcess
#if !VM_WITH_DEBUG_JCADT
	#define  loadSysPkg()                   JavaDebug_VM_loadSysPkg()
#else
	#define  loadSysPkg()					JavaDebug_VM_initJcdt()
#endif
	#define JCDebug_Interpreter1(curJPC)          JavaDebug_VM_Interpreter1(curJPC)
	#define JCDebug_Interpreter2(curJPC,byteCode) JavaDebug_VM_Interpreter2(curJPC,byteCode)
	#define JCDebug_deletePackageDebugComponent(pkgID) JavaDebug_deletePackageDebugComponent(pkgID)
	#define JCDebug_Deallocate()                    JavaDebug_Deallocate()
#endif


#endif /* VMDEBUG_H_ */
