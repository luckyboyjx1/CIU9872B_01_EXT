/*
 * VMDebug.c
 *
 *  Created on: 2014Äê12ÔÂ25ÈÕ
 *      Author: mowenpan
 */
#include "hal_callback.h"
#include "vm_callback.h"
#include "VMDebug.h"

#include "packageloc.h"

#include "ecpDefine.h"
#include "object.h"
#include "grt.h"
#include "global_variable.h"
#include "ob_manager.h"


#if VM_WITH_DEBUG
#include "debugMask.h"
#include "JCDebug.h"

void JavaDebug_DebugComponentInit()
{
	JCDebugComponentInit(g_buffer,g_dataOffset);
}

boolean JavaDebug_DebugComponentProcess()
{
	return JCDebugComponentProcess(g_buffer, g_offset, U16_S_U16(g_max, g_offset));
}

boolean JavaDebug_DebugComponentpostProcess()
{
	u8 pkgId = nvmReadU8(MEMtPN8(PM_g_pkgIdInProcess_JC));
	return JCDebugComponentpostProcess(pkgId, class_component_Addresses, method_component_Addresses);
}


void JavaDebug_deletePackageDebugComponent(u8 pkgID){
	JCDeletePackageDebugComponent(pkgID);
}

void JavaDebug_Deallocate(){
	JCDebugDeallocate();
}
void JavaDebug_VM_initJcdt(){
	JavaDebug_initJcdt();
}
void JavaDebug_VM_loadSysPkg(){
	JavaDebug_loadSysPkg();
}

void JavaDebug_VM_Interpreter1(memref curJPC){
	JavaDebug_Interpreter1(curJPC);
}
void JavaDebug_VM_Interpreter2(memref curJPC, u8 byteCode){
	JavaDebug_Interpreter2(curJPC,byteCode);
}
#endif
