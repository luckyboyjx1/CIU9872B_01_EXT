/*
 * cgmLogicManager.h
 *
 *  Created on: 2015年10月21日
 *      Author: liuhanren
 */

#ifndef SRC_JCVM_ROOMMANAGER_CGMMANAGER_H_
#define SRC_JCVM_ROOMMANAGER_CGMMANAGER_H_

#include "profile.h"
#include "object.h"
#include "ob_manager.h"
#include "memory.h"
#include "opFunction.h"
#include "CgmField.h"
#include "CgmInit.h"
#include "CgmInstall.h"
#include "CgmDel.h"
#include "CgmLoad.h"
#include "CgmRun.h"
#include "tranAllocate.h"
#if GP__CGM__FOR_APPPARACHECK
/*
*	rule box
*/
#define APP_CGM_LESS_THAN_ASSOCIATED_CGM									0 // maxCGM < remainCGM
#define APP_RESERVE_LESS_THAN_ASSOCIATED_CGM							1 // reserve < remainCGM
#define APP_CONSUME_LESS_THAN_ASSOCIATED_CGM							2 // consume < remainCGM
#define APP_RESERVE_LESS_THAN_QUOTA												3 // reserve < quota
#define APP_CONSUME_LESS_THAN_QUOTA												4 // consume < quota
#define GET_MAX_CONSUME_RESERVE                           5 // get the max value between consume and reserve

#define ROOM_MANAGE_OF_LOAD          	   	                U32t8(1)
#define ROOM_MANAGE_OF_INSTALL       		                  U32t8(2)
#define ROOM_MANAGE_OF_EXTRADITION   			                U32t8(3)
#define ROOM_MANAGE_OF_REGISTRY_UPDATE                    U32t8(4)
#define ROOM_MANAGE_OF_REMOVE_APPLET                      U32t8(5)
#define ROOM_MANAGE_OF_REMOVE_PACKAGE                     U32t8(6)
#define ROOM_MANAGE_OF_CHECK_LOAD                         U32t8(7)

#define ROOM_MANAGE_OF_EXTRADITION_PKG                    U32t8(0x80)


#define INIT_CGM_WHEN_INSTALL_MNOSD                          3
#define INIT_CGM_WHEN_INSTALL_SD                             0
#define UPDATA_SD_CGM_SIZE						              1
#define UPDATA_SD_REMAIN_CGM_SIZE				              2

#define PROP_SECURITY_DOMAIN_BIT                           U32t8(0x80)

#define CHECK_RUN                             						0
#define CHECK_LOAD                            						1
#define CHECK_INSTALL                         						2
#define CHECK_GC							                						3
#define NO_CHECK                              						4
//linguoming:config C7C8
#define CHECK_CLOSE						            	              5
//#define CHECK_LOAD_DATA                                 6

#define CHECK_INSTALL_SYSTEM_CONSUME                      7

#define DELETE_PKG_APP																		0xE1

#if CHINA_MOBILE_DESIGNATED_REQUIREMENT

#define INFO_APP_USED_RAM																	(u8)0X01
#define INFO_APP_USED_NVM                                 (u8)0x02
#define INFO_PKG_LOAD_C6                         					(u8)0x10
#define INFO_PKG_LOAD_C7                    							(u8)0x20
#define INFO_PKG_LOAD_C8                    							(u8)0x30

#define	CMCC_CGM_REPAIR_CONST							  0x1400

#endif

#define HASC7_MASK					                              U32t8(0x80)		//BIT 7		//对应 CGMarray 中存在 quotaRAM(C7)
#define HASC8_MASK					                              U32t8(0x40)		//BIT 6		//对应 CGMarray 中存在 quotaNVM(C8)
#define HASC9_MASK					                              U32t8(0x20)		//BIT 5
#define MAKE_SELECTABLE_MASK					     			          U32t8(0x10)		//BIT 4
#define HAS83_MASK					                              U32t8(0x08)		//BIT 3		//对应 CGMarray 中存在 remainCGMnvm(83) 和 maxCGMnvm(83) ; maxCGMnvm(83)在获取 SD_CGM_NVM_SIZE_8F 参数时存在？
#define HAS82_MASK					                              U32t8(0x04)		//BIT 2		//对应 CGMarray 中存在 remainCGMram(82) 和 maxCGMram(82) ; maxCGMram(82)在获取 SD_CGM_RAM_SIZE_90 参数时存在？
#define HASD8_MASK					                              U32t8(0x02)		//BIT 1		//对应 CGMarray 中存在 reserveNVM(D8)
#define HASD7_MASK					                              U32t8(0x01)		//BIT 0 	//对应 CGMarray 中存在 reserveRAM(D7) 

#define CGM_2BYTE_MASK                                    U8_OR_U8(HASD7_MASK, HASD8_MASK)  //D7D8 len:2byte,others 2 or 4 bytes
#define CGM_RAM_MASK                                      U8_OR_U8(U8_OR_U8(HASC7_MASK, HASD7_MASK), HAS82_MASK)

#define	CGM_GET_RAM										  1
#define	CGM_GET_NAM										  2
#define	CGM_GET_ALL										  3

#define NO_CHANGED                                        0
#define NVM_SIZE_CHANGED                                  1
#define RAM_SIZE_CHANGED                                  2
#define	NVM_SIZE_CHANGED_LOAD_INSTALL					  3
#define	RAM_SIZE_CHANGED_LOAD_INSTALL					  4

#define SD_INCREASE_SIZE                                  U32t8(0x00)
#define SD_DECREASE_SIZE                                  U32t8(0x01)
#define SD_EXTRADITION_ONESELF                            U32t8(0x10) //自引渡标记
#define SD_EXTRADITION_ONESELF_MASK                       U8_INV(0x10)

#define TAG_LEN_2                                         0x02
#define TAG_LEN_4                                         0x04

#define CGM_INIT_ENABLE_TAG                               0x208B
#if CHINA_UMICOM_SPECIAL_MANAGE
/*
* 联通对SSD自身消耗的空间限制为756bytes，而我们实现消耗空间大约为1250-1300。
* 因此这里做特殊处理：
* 1. 在安装时，SSD最大NVM空间为752；
* 2. 多余的消耗算到ISD内；
* 3. 运行中SSD的消耗不做特殊处理，与标准一致；
* 4. SSD删除时，除了要恢复SSD的CGM空间，还要恢复算到ISD内的空间。
* 设计：
* 1. 安装时，让ISD替SD承担固定大小消耗(X byte),则SSD自身CGM消耗=MAX(D8,g_iUsedNVM)-X;
* 2. 安装时，“consumeNVM” = MAX(D8,g_iUsedNVM) - X;
* 3. 删除时，除了恢复SSD CGM总空间，还要恢复ISD的替SD承担的X大小空间；
* 4. 运行时、迁移引渡，均按标准处理。
*/
#define CHINA_UMICOM_SSD_ONESELF_MAX_NVM									0x2F0 // 752bytes
#define CHINA_UNICOM_ISD_CONSUME_FOR_SSD									0x250 // 512

#endif
//#define getPkgsdIdObj(pkgEntryAddr)                                     	get_object_field_ref(pkgEntryAddr, sdId_INDEX)
#define CgmManager_getAppIdFromEntry(appEntryObj)                           memReadU8(MEMtPM8(get_object_field_ref((memref)locateObjectInMemory(appEntryObj), bAppID_INDEX)))
#define CgmManager_getAppIdFromEntryObj(appEntryObj)                        memReadU8(MEMtPM8(get_object_field_ref((memref)appEntryObj, bAppID_INDEX)))
#define CgmManager_getPkgUsedNvmObj(pkgEntryAddr)                           get_object_field_ref(pkgEntryAddr, pkgConsumeNVM_INDEX)
/* 求两者中最大值 */
#define CgmManager_getMax1of2(value1, value2)											(((value1) > (value2)) ? (value1) : (value2))
//#define CgmManager_setDeleteFlag(v)										(g_deletPkgAppFlag = v)

#if CHINA_MOBILE_DESIGNATED_REQUIREMENT

typedef struct
{
	u8		pkgId;
	u16		loadC6;
	u16		loadC7;
	u16		loadC8;
}pkgInfoc6c7c8;

extern pkgInfoc6c7c8	p_PkgInfo;

#define maxLoadPkgNum														(ON_CARD_PKG_MAX - 30) //30:mask in pkg
#define totalSizeOfPkgCgmArray												(maxLoadPkgNum * sizeof(pkgInfoc6c7c8)) //546B

#define setRecord_LoadPkgId(pPInfo,Id)                            	 		nvmWriteU8Atomic(&(((pkgInfoc6c7c8*)pPInfo)->pkgId), Id)
#define setRecord_LoadC6(pPInfo,c6)                                  		nvmWriteU16Atomic((&(((pkgInfoc6c7c8*)pPInfo)->loadC6),  c6)
#define setRecord_LoadC7(pPInfo,c7)                                  		nvmWriteU16Atomic(&(((pkgInfoc6c7c8*)pPInfo)->loadC7),  c7)
#define setRecord_LoadC8(pPInfo,c8)                                  		nvmWriteU16Atomic(&(((pkgInfoc6c7c8*)pPInfo)->loadC8),  c8)

#define getRecord_LoadPkgId(pPInfo)											PX_R_U8(&(((pkgInfoc6c7c8*)pPInfo)->pkgId))
#define getRecord_LoadC6(pPInfo)											PX_R_U16(&(((pkgInfoc6c7c8*)pPInfo)->loadC6))
#define getRecord_LoadC7(pPInfo)											PX_R_U16(&(((pkgInfoc6c7c8*)pPInfo)->loadC7))
#define getRecord_LoadC8(pPInfo)											PX_R_U16(&(((pkgInfoc6c7c8*)pPInfo)->loadC8))

#endif
/*
* 函数声明
*/
extern void CgmManager_manageSdMenSize(u8 sdId, u32 cgmRam, u32 cgmNvm, u8 type, boolean isExtraditionOneself);

extern u32  CgmManager_getNvmConsume(u8 cgmFlag,u16 appId,PN8 cgmArrayAddr);
extern u32  CgmManager_getRamConsume(u8 cgmFlag,u16 appId,PN8 cgmArrayAddr);

extern void CgmManager_Main(void);
extern u8   getCGMarraySizeImpl(u8 flag);
extern void getCGMarraySize(void);
extern void CgmManager_SetSdMenInfo(u8 sdId,u32 cgmRam,u32 cgmNvm, u8 manageType);
extern void CgmManager_error(u16 errorcode);
extern void CgmManager_getSizeByObjId(u16 objectId, PX32 nvmSize, PX32 ramSize, u8 getType);
extern void CgmManager_calaConsumeSpace(u16 Id, PX32 nvmSize, PX32 ramSize, u8 isPkg, u8 type);
extern void CgmManager_RamChange(u16 context, u32 newRamValue, u32 newSystemRamValue);
extern void CgmManager_NvmChange(u16 context, u32 newNvmValue, u32 newSystemNvmValue);
#if GP__CGM__FOR_MASK
extern void CgmManager_Free(u32 chunkSize, u8 cgm_mask);
extern void CgmManager_Alloc(u32 pSize, u8 cgm_mask);
#else
#define  CgmManager_Free(chunkSize, cgm_mask)
#define  CgmManager_Alloc(pSize, cgm_mask)
#endif

#else
#define  CgmManager_manageSdMenSize(sdId, cgmRam, cgmNvm, type, isExtraditionOneself)
#define  CgmManager_Free(chunkSize, cgm_mask)
#define  CgmManager_Alloc(pSize, cgm_mask)
#define  CgmManager_getNvmConsume(cgmFlag,appId,cgmArrayAddr)														0
#define  CgmManager_getRamConsume(cgmFlag,appId,cgmArrayAddr)														0

#define  CgmManager_Main()
#define  getCGMarraySizeImpl()		0
#define  getCGMarraySize()
#define  CgmManager_SetSdMenInfo(sdId,cgmRam,cgmNvm, manageType)
#define  CgmManager_error(errorcode)
#define  CgmManager_getSizeByObjId(objectId, nvmSize, ramSize, getType)
#define  CgmManager_calaConsumeSpace(Id, nvmSize, ramSize, isPkg, type)
#define  CgmManager_RamChange(context, newRamValue, newSystemRamValue)
#define  CgmManager_NvmChange(context, newNvmValue, newSystemNvmValue)
#define  CHECK_GC
//#define CgmManager_setDeleteFlag(v)
#endif
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
void initiateMemInfoArray(void);
memref getPkgInfoAddr(u8 pkgId);
void SetRecordOfMemNull(pkgInfoc6c7c8 * INFOPtr);
void writeRecord(pkgInfoc6c7c8 *pPkgInfo);
u8 getAssociatedSdIdbyAppId(u8 appId);
void getLoadFileInfo(u8 pkgId,u8 *buf);

u16 handlRoomOfGetAppRam(memref cgmArrayObj);
u32 handlRoomOfGetAppNvm(memref cgmArrayObj);
#endif
extern void CGM_NVMObjResize(u16 entry_index,u16 context);
#endif /* SRC_JCVM_ROOMMANAGER_CGMMANAGER_H_ */
