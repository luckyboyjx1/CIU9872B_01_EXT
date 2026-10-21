/*
 * CgmField.h
 *
 *  Created on: 2015年11月5日
 *      Author: liuhanren
 */

#ifndef SRC_JCVM_ROOMMANAGER_CGMFIELD_H_
#define SRC_JCVM_ROOMMANAGER_CGMFIELD_H_

#include "CgmManager.h"

#if (!GP__CGM__FOR_MASK&&GP__CGM__FOR_APPPARACHECK)
	/* 共消耗全局ram :  44 bytes*/
	extern memref g_appEntryAddr;  //install 时缓存新增加的 app entry address
	extern u8     g_bCheckFlag;
	//extern u8	  g_deletPkgAppFlag;

	extern u32    g_sC7Value;
	extern u32    g_sC8Value;
	extern u32    g_sD7Value;
	extern u32 	  g_sD8Value;

	extern u32    g_UsedRAM;
	extern u32    g_UsedNVM;
	extern u32    g_OldUsedRAM;
	extern u32    g_OldUsedNVM;
	extern u8     g_bRoomChanged;

	extern u32   cacheD7;
	extern u32   cacheD8;
	extern u32   cacheC7;
	extern u32   cacheC8;
	extern u32   cache82;
	extern u32   cache83;
	/*
	* 由CGM flag的值和CGM_Mask求出对应项的值。
	*/
	
	#define CGM_Mask_PerPara_Len	4
	
//	#define CgmField_getRamSizeFromCGMarray(cgmArrayAField, offset)			   nvmReadU16(MEMtPN8(PN8_A_U8(cgmArrayAField,offset)))//PX_R_U16((memref)((memref)cgmArrayAField + (u8)offset))
	#define CgmField_getNvmSizeFromCGMarray(cgmArrayAField, offset) 			nvmReadU32(MEMtPN8(PN8_A_U8(cgmArrayAField,offset)))//PX_R_U32((memref)((memref)cgmArrayAField + (u8)offset))

	#define CgmField_getMaxCGMOffset(cgmFlag,flagMask)                               	CgmField_ArrayOffset(cgmFlag, flagMask, true)
	#define CgmField_getCGMarrayOffset(cgmFlag,flagMask)                             	CgmField_ArrayOffset(cgmFlag, flagMask, false)

	#define CgmField_getCGMFlag(sdEntryObj)                                          	nvmReadU8(MEMtPN8(get_object_field_ref(sdEntryObj, CGMflag_INDEX)))

	#define CgmField_CacheD7()                                    	    	 	cacheD7
	#define CgmField_CacheD8()                                         			cacheD8

	#define CgmField_CacheC7()                                         			cacheC7
	#define CgmField_CacheC8()                                         			cacheC8

	#define CgmField_Cache82()                                         			cache82
	#define CgmField_Cache83()                                         			cache83

	#define CgmField_updateCgmRamFlag                                  			bRamBuffer[REGISTRY_UPDATE_CGM_RAM_FLAG]
	#define CgmField_updateCgmNvmFlag                                  			bRamBuffer[REGISTRY_UPDATE_CGM_NVM_FLAG]
	#define CgmField_getUpdateCgmRam()                                 			PX_R_U32(&bRamBuffer[REGISTRY_UPDATE_CGM_RAM_VALUE])
	#define CgmField_getUpdateCgmNvm()                                 			PX_R_U32(&bRamBuffer[REGISTRY_UPDATE_CGM_NVM_VALUE])

	#define CgmField_setMax82(cgmArrayObj,cgmFlag,maxCGMRam)           			nvmWriteU32Atomic(MEMtPN8(PN8_A_U8(cgmArrayObj,CgmField_getMaxCGMOffset(cgmFlag,HAS82_MASK))), maxCGMRam)
	#define CgmField_setMax83(cgmArrayObj,cgmFlag,maxCGMNvm)           			nvmWriteU32Atomic(MEMtPN8(PN8_A_U8(cgmArrayObj,CgmField_getMaxCGMOffset(cgmFlag,HAS83_MASK))), maxCGMNvm)
	#define CgmField_setRemain82(cgmArrayObj,cgmFlag,remainCGMRam)     			nvmWriteU32Atomic(MEMtPN8(PN8_A_U8(cgmArrayObj,CgmField_getCGMarrayOffset(cgmFlag,HAS82_MASK))), remainCGMRam)
	#define CgmField_setRemain83(cgmArrayObj,cgmFlag,remainCGMNvm)     			nvmWriteU32Atomic(MEMtPN8(PN8_A_U8(cgmArrayObj,CgmField_getCGMarrayOffset(cgmFlag,HAS83_MASK))), remainCGMNvm)

	#define CgmField_setC7(cgmArrayObj,cgmFlag,C7value)                			nvmWriteU32Atomic(MEMtPN8(PN8_A_U8(cgmArrayObj,CgmField_getCGMarrayOffset(cgmFlag,HASC7_MASK))), C7value)
	#define CgmField_setC8(cgmArrayObj,cgmFlag,C8value)                			nvmWriteU32Atomic(MEMtPN8(PN8_A_U8(cgmArrayObj,CgmField_getCGMarrayOffset(cgmFlag,HASC8_MASK))), C8value)
	#define CgmField_setD7(cgmArrayObj,cgmFlag,D7value)							nvmWriteU32Atomic(MEMtPN8(PN8_A_U8(cgmArrayObj,CgmField_getCGMarrayOffset(cgmFlag,HASD7_MASK))), D7value)
	#define CgmField_setD8(cgmArrayObj,cgmFlag,D8value)                			nvmWriteU32Atomic(MEMtPN8(PN8_A_U8(cgmArrayObj,CgmField_getCGMarrayOffset(cgmFlag,HASD8_MASK))), D8value)//注意，D8为2字节

	#define CgmField_getC7(cgmArrayObj, cgmFlag)                       			CgmField_getNvmSizeFromCGMarray(cgmArrayObj, CgmField_getCGMarrayOffset(cgmFlag, HASC7_MASK))
	#define CgmField_getC8(cgmArrayObj, cgmFlag)                       			CgmField_getNvmSizeFromCGMarray(cgmArrayObj, CgmField_getCGMarrayOffset(cgmFlag, HASC8_MASK))
	#define CgmField_getD7(cgmArrayObj, cgmFlag)                       			CgmField_getNvmSizeFromCGMarray(cgmArrayObj, CgmField_getCGMarrayOffset(cgmFlag, HASD7_MASK))
	#define CgmField_getD8(cgmArrayObj, cgmFlag)                       			CgmField_getNvmSizeFromCGMarray(cgmArrayObj, CgmField_getCGMarrayOffset(cgmFlag, HASD8_MASK))//注意，D8为2字节

	#define CgmField_getRemain82(cgmArrayObj, cgmFlag)                 			CgmField_getNvmSizeFromCGMarray(cgmArrayObj, CgmField_getCGMarrayOffset(cgmFlag, HAS82_MASK))
	#define CgmField_getRemain83(cgmArrayObj, cgmFlag)                 			CgmField_getNvmSizeFromCGMarray(cgmArrayObj, CgmField_getCGMarrayOffset(cgmFlag, HAS83_MASK))
	#define CgmField_getMax82(cgmArrayObj, cgmFlag)                    			CgmField_getNvmSizeFromCGMarray(cgmArrayObj, CgmField_getMaxCGMOffset(cgmFlag, HAS82_MASK))
	#define CgmField_getMax83(cgmArrayObj, cgmFlag)                    			CgmField_getNvmSizeFromCGMarray(cgmArrayObj, CgmField_getMaxCGMOffset(cgmFlag, HAS83_MASK))
	#define CgmField_getPkgID(appEntryObj)                             			nvmReadU8(MEMtPN8(get_object_field_ref(appEntryObj, theContext_INDEX)))
	#define CgmField_getAPPassociatedSDID(appEntryObj)                 			nvmReadU8(MEMtPN8(get_object_field_ref(appEntryObj, bsdId_INDEX)))

	extern u8 CgmField_ArrayOffset(u8 CGMflag,u8 CGM_Mask,boolean isMaxCGMorRemainCGM);
	extern u32 CgmField_get8283Value(u8 sdId, u8 cgmFlag, PN8 sdEntryAddr, u8 cgm_mask, boolean isMaxCGMorRemainCGM);
	extern u32 CgmField_getSpaceInfo(u8 sdId, u8 type);
	extern void CgmField_getEntryInfo(u16 context);
	extern memref CgmField_getArrayObj(PF8 entryAddr);
	extern u32 CgmField_GetValue(PN8 buffer, u8 offset, u8 tagLen, u8 cgm_mask, u8 isInstallForLoad);
	extern void CgmField_set8283Value(PN8 cgmArrayAddr,u8 cgmFlag, u32 bSDmax_CGMRam, u32 bSDmax_CGMNvm, u8 set_mask);
	extern u32	 CgmField_AlignSize(u32 origSize);
#else
	#define	 CgmField_ArrayOffset(CGMflag,CGM_Mask,isMaxCGMorRemainCGM)											0
	#define	 CgmField_get8283Value(sdId, cgmFlag, sdEntryAddr, cgm_mask, isMaxCGMorRemainCGM) 					0
	#define  CgmField_getSpaceInfo(sdId, type,cgmRam , cgmNvm)													0
	#define	 CgmField_getArrayObj(entryAddr) 																	0
	#define	 CgmField_GetValue(buffer, offset, tagLen, cgm_mask, isInstallForLoad) 								0
	#define  CgmField_set8283Value(cgmArrayAddr,cgmFlag, bSDmax_CGMRam, bSDmax_CGMNvm, set_mask)
	#define	 CgmField_getEntryInfo(context)
	#define	 CgmField_AlignSize(origSize)																		0
#endif

#endif /* SRC_JCVM_ROOMMANAGER_CGMFIELD_H_ */
