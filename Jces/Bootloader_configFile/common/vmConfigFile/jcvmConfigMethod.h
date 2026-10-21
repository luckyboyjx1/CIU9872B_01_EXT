/*
 * jcvmConfigMethod.h
 *
 *  Created on: 2014年12月17日
 *      Author: jiangxiaohui
 */

#ifndef JCVMCONFIGMETHOD_H_
#define JCVMCONFIGMETHOD_H_

#include "profile.h"
#include "JcvmConfig.h"
#include "SysConfigProfile.h"

#if !BOOT__MEMORY__BACK_UP
#define abortTransactionByJCRE()
#endif //BOOT__MEMORY__BACK_UP

#if	!VM_NEED_COMPACT
#define	NVM_free_object(isDeletePkg)
#define	NVM_free(entry_index, chunk_size)
#define	NVM_compact(is_rollback)
#define	NVM_compact_rollback()
extern void backupUsedAddr(void);
extern void rollbackUsedAddr(void);
extern void getData_FF32(void);
#endif


//typedef struct
//{
//#if SUPPORT_POINTER_24_BIT
//	PN24		entry_addr;
//	u8			id;
//#else
//	PN32		entry_addr;
//#endif
//}  N_A  * MapEntry_t, MapEntry;

#if	MAP_ADDRESS_NEED_DEAL
	#define		get_MAPEntry(mapEntry, index)				dealMAPAddress(mapEntry,index)
#elif SUPPORT_POINTER_24_BIT
	#define		get_MAPEntry(mapEntry, index)				PNtMEM((*mapEntry)->entry_addr)
#else
	#define		get_MAPEntry(mapEntry, index)				PNtMEM((PXS_R_U32(*mapEntry)) & 0x00FFFFFF)

#endif


#define	BIT_MAP_TBL_SEG_SIZE						0x80
#define	NVM_FREE_TABLE_SEG_SIZE						0x80

#define BIT_MAP_TABLE_SIZE      					U8_M_U16(JcvmConfig_GetMapBitTabSeg(), BIT_MAP_TBL_SEG_SIZE)
#define MAX_FREE_TBL_SIZE							JcvmConfig_GetFreeTabEntry()
#define MAX_FREE_OBJECT_BLOCK_SIZE					U16_S_U8(MAX_FREE_TBL_SIZE, 4)	// 至少预留四项
#define	NVM_FREE_TABLE_SIZE							U8_M_U16(8, U8_A_U16(8, MAX_FREE_TBL_SIZE))	//  NVM_FREE_TABLE_SIZE = (MAX_FREE_TBL_SIZE + 8) * 8(8 is cache size)
#define BIT_MAP_TABLE_START                         U32_S_U16(NVM_FREE_TABLE_START, BIT_MAP_TABLE_SIZE)


#if !VM_WITH_DEBUG
	#define DebugComponentInit 				componentInit
	#define DebugComponentProcess			postProcess
	#define DebugComponentpostProcess		postProcess

	#define  loadSysPkg()
	#define JCDebug_Interpreter1(curJPC)
	#define JCDebug_Interpreter2(curJPC,byteCode)
	#define JCDebug_deletePackageDebugComponent(pkgID)
	#define JCDebug_Deallocate()
#else
#include "../../SimDebug/callback/VMDebug.h"
#endif

#if 1//(VM_WITH_PBOC || VM_WITH_EDEP)

extern void 	JcvmConfig_GetConstructAuthChipBase(PX8 buf);
#else
#define  JcvmConfig_GetConstructAuthChipBase()            0
#endif

extern void dealWithPseudoRandom(void);
#endif /* JCVMCONFIGMETHOD_H_ */
