/*****************************************************************************
 * @(#)packageloc.c	1.14 03/09/24
 * Version 1.9

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify

 ****************************************************************************/
#include "packageloc.h"
#include "profile.h"
/*#include "typejava.h"		  */
#include "memory.h"
#include "mask.h"
#include "object.h"
#include "context.h"
#include "packageloc.h"
#include "global_variable.h"
#include "ob_manager.h"
#include "NvmMap.h"
#include "SEEUICCInterface.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
/* Table initialized by maskgen */

/**
 * modifed by xxj@20210328: mask中包对象的个数不变，后下载的包在下载过程中创建的对象由5个(class_component / method_component / staticfield_component / export_component / pkg_info)减少为1个(pkg_info)
 *                          pkg_info中的信息，mask中包对象保持不变，存储的分别是4个组建引用(4字节) + 组建大小(2字节) ; 后下载的包对象发生变化，存储的分别是4个组建地址相对于pkg_info起始地址的偏移(4字节) + 组建大小(2字节)
 */
void removePackage(u8 pkgID)		SMALL_MODE
{
	u8		D_A		txn_in_progress;
	//u8		D_A		i;  //remove warning,dengyiwen,2022/9/22
	/* Locate entry in table */
	jref	D_A		pkg_info_addr;
	//jref	D_A		comp_address;  //remove warning,dengyiwen,2022/9/22

	
	PATCH_JCVM_VM_PACKAGELOC_001();	
	
	txn_in_progress	= 0;
	pkg_info_addr	= PF_R_U16(PF8_A_U16(MEMtPF8(G_CurExecuEnvironment.pkgLocationTable + SIZE_U16), U8_M_U8(pkgID, SIZE_MEMREF)));

	if (!(REF_NULL == pkg_info_addr))
	{

		if (isInProgress())
		{
			/* Transaction in progress ! */
			txn_in_progress = 1;
		}
		else
		{
			begin_transaction(SINGLE_APDU_PROCESS);
		}

		/* Deallocate package info structure Null address denotes package not allocated */
		//E2P_free(pkg_info_addr, (PKGLOC_COMPONENT_COUNT * (SIZE_MEMREF + SIZE_U16)));
		NvmManager_Free(pkg_info_addr, MAP_ENTRY_BLOCK_PCK_ID);
		/*@ -disspecopt @*/
		nvmWriteU32Atomic(PF8_A_U16(MEMtPF8(G_CurExecuEnvironment.pkgLocationTable), U8_M_U8(pkgID, SIZE_MEMREF)), REF_NULL);
		/*@ +disspecopt @*/
		if (txn_in_progress == 0)
		{
			commit_transaction();
		}
	}
	return;
}

/**
 * modifed by xxj@20210328: mask中包对象的个数不变，后下载的包在下载过程中创建的对象由5个(class_component / method_component / staticfield_component / export_component / pkg_info)减少为1个(pkg_info)
 *                          pkg_info中的信息，mask中包对象保持不变，存储的分别是4个组建引用(4字节) + 组建大小(2字节) ; 后下载的包对象发生变化，存储的分别是4个组建地址相对于pkg_info起始地址的偏移(4字节) + 组建大小(2字节)
 */
memref getPackageComponentLocation(u8 pkgID, u8 compID)
{
    u16			D_A		map_index;
	PF8			D_A		map_addr;
	u32         D_A		map_offset;
	
	PATCH_JCVM_VM_PACKAGELOC_002();	
	
	if (pkgID == PCpkgID)
	{
		switch (compID)
		{
		case COMPONENT_METHOD:
			return cachedMethodCompLoc;
		case COMPONENT_CLASS:
			return cachedClassCompLoc;
		case COMPONENT_STATICFIELD:
			return cachedFieldCompLoc;
		default:
			break;
		}
	}
//	return MapTable_GetEntityContent((u16)loadAddress_NOCache((u8 far *)MapTable_GetEntityContent((u16)(loadAddress_NOCache((u8 far *)(G_CurExecuEnvironment.pkgLocationTable + pkgID * SIZE_MEMREF)))) + (memref)(compID * (SIZE_MEMREF + SIZE_U16))));
	map_index	= PF_R_U16(PF8_A_U16(MEMtPF8(G_CurExecuEnvironment.pkgLocationTable + SIZE_U16), U8_M_U8(pkgID, SIZE_MEMREF)));
	map_addr	= MEMtPF8(MapTable_GetEntityContent(map_index));
	if(pkgID < NPACKAGES)
	{
		map_addr 	= PF8_A_U8(map_addr, U8_M_U8(compID, (SIZE_MEMREF + SIZE_U16)));
		map_addr    = PF8_A_U16(map_addr,SIZE_U16);
		map_index 	= PF_R_U16(map_addr);
	//	map_index 	= PF_R_U16(map_addr + SIZE_U16);
		map_addr	= MEMtPF8(MapTable_GetEntityContent(map_index));
	}
	else
	{
		map_offset 	= PF_R_U32(PF8_A_U8(map_addr, U8_M_U8(compID, (SIZE_MEMREF + SIZE_U16))));
		map_addr 	= PF8_A_U32(map_addr,map_offset);
	}

	return PFtMEM(map_addr);   
}

/*This method returns package Id based on an address*/
// this method only used to find from classComponent ,so the param compID not using,romoved!!
/**
 * modifed by xxj@20210328: mask中包对象的个数不变，后下载的包在下载过程中创建的对象由5个(class_component / method_component / staticfield_component / export_component / pkg_info)减少为1个(pkg_info)
 *                          pkg_info中的信息，mask中包对象保持不变，存储的分别是4个组建引用(4字节) + 组建大小(2字节) ; 后下载的包对象发生变化，存储的分别是4个组建地址相对于pkg_info起始地址的偏移(4字节) + 组建大小(2字节)
 */
u8 getPackageIdForAddr(PF8 addr)    SMALL_MODE
{													
	u8			D_A		pkgId;
//	u8 far * pkg_info_addr = loadAddress_NOCache(G_CurExecuEnvironment.pkgLocationTable);
    PF8			D_A		pkg_info_addr;
	PF8			D_A		comp_address;
	PF8			D_A		comp_range;
	
	PATCH_JCVM_VM_PACKAGELOC_003();	
	
	pkgId			= 0;
    pkg_info_addr	= MEMtPF8(MapTable_GetEntityContent(PF_R_U16(MEMtPF8(G_CurExecuEnvironment.pkgLocationTable + SIZE_U16))));

	while (!check_addr_null(pkg_info_addr))
	{
		//TODO:check asm code of all add operation RECHECK U8 FAR *
		if(pkgId < NPACKAGES)
		{
			comp_address 	= MEMtPF8(MapTable_GetEntityContent(PF_R_U16(pkg_info_addr + SIZE_U16))); // classComponent start address
			comp_range		= PF8_A_U16(comp_address, PF_R_U16(PF8_A_CONST(pkg_info_addr, SIZE_MEMREF_L))); // classComponent end address
		}
		else
		{
			comp_address 	= PF8_A_U32(pkg_info_addr, PF_R_U32(pkg_info_addr));
			comp_range		= PF8_A_U16(comp_address, PF_R_U16(PF8_A_CONST(pkg_info_addr, SIZE_MEMREF_L)));
		}
		if ((addr >= comp_address) && (addr <= comp_range))
		{
			return pkgId;
		}
		U8_INC(pkgId);
		pkg_info_addr = MEMtPF8(MapTable_GetEntityContent(U32t16(PF_R_U32(PF8_A_U16(MEMtPF8(G_CurExecuEnvironment.pkgLocationTable), U8_M_U8(pkgId, SIZE_MEMREF))))));
	}
	return 0xFF;
}

// this method add by wh 2009.4.18, input classs addr ,out put the class component base addr
// used in loader.c
//#pragma OT(5)
/**
 * modifed by xxj@20210328: mask中包对象的个数不变，后下载的包在下载过程中创建的对象由5个(class_component / method_component / staticfield_component / export_component / pkg_info)减少为1个(pkg_info)
 *                          pkg_info中的信息，mask中包对象保持不变，存储的分别是4个组建引用(4字节) + 组建大小(2字节) ; 后下载的包对象发生变化，存储的分别是4个组建地址相对于pkg_info起始地址的偏移(4字节) + 组建大小(2字节)
 */
memref getClassComponentBaseAddr(PF8 addr)
{
	u8			D_A		pkgId;
	PF8			D_A		pkg_info_addr;
	PF8 		D_A		comp_address;
	PF8 		D_A		comp_range;
	//u8 far * tempAddr = addr;
	
	PATCH_JCVM_VM_PACKAGELOC_004();	
	
	pkgId			= 0;
	pkg_info_addr	= MEMtPF8(MapTable_GetEntityContent(PF_R_U16(MEMtPF8(G_CurExecuEnvironment.pkgLocationTable + SIZE_U16))));

	while (!check_addr_null(pkg_info_addr))
	{
		//TODO:check asm code of all add operation RECHECK U8 FAR *
		if(pkgId < NPACKAGES)
		{
			comp_address		= MEMtPF8(MapTable_GetEntityContent(PF_R_U16(pkg_info_addr + SIZE_U16))); // classComponent start address
			comp_range			= PF8_A_U16(comp_address, PF_R_U16(PF8_A_CONST(pkg_info_addr, SIZE_MEMREF_L))); // classComponent end address
		}
		else
		{
			comp_address 	= PF8_A_U32(pkg_info_addr, PF_R_U32(pkg_info_addr));
			comp_range		= PF8_A_U16(comp_address, PF_R_U16(PF8_A_CONST(pkg_info_addr, SIZE_MEMREF_L)));
		}
		if ((addr >= comp_address) && (addr <= comp_range))
		{
			return PFtMEM(comp_address);
		}

		U8_INC(pkgId);
		pkg_info_addr = MEMtPF8(MapTable_GetEntityContent(PF_R_U16(PF8_A_U16(MEMtPF8(G_CurExecuEnvironment.pkgLocationTable + SIZE_U16), U8_M_U8(pkgId, SIZE_MEMREF)))));
	}

	return REF_NULL;
}

/**
 * modifed by xxj@20210328: mask中包对象的个数不变，后下载的包在下载过程中创建的对象由5个(class_component / method_component / staticfield_component / export_component / pkg_info)减少为1个(pkg_info)
 *                          pkg_info中的信息，mask中包对象保持不变，存储的分别是4个组建引用(4字节) + 组建大小(2字节) ; 后下载的包对象发生变化，存储的分别是4个组建地址相对于pkg_info起始地址的偏移(4字节) + 组建大小(2字节)
 */
memref getmethodComponentBaseAddr_MapIndex(u8 pkgID, u8 compID)
{
    memref		D_A		map_index;
	PF8			D_A		map_addr;
	
	PATCH_JCVM_VM_PACKAGELOC_005();	
	
//	return  (u16)loadAddress_NOCache((u8 far *)MapTable_GetEntityContent((u16)(loadAddress_NOCache((u8 far *)(G_CurExecuEnvironment.pkgLocationTable + pkgID * SIZE_MEMREF)))) + (memref)(compID * (SIZE_MEMREF + SIZE_U16)));
	map_index	= PF_R_U16(MEMtPF8(PF8_A_U16(G_CurExecuEnvironment.pkgLocationTable + SIZE_U16, U8_M_U8(pkgID, SIZE_MEMREF))));
	map_addr	= MEMtPF8(MapTable_GetEntityContent(map_index));
	if(pkgID < NPACKAGES)
	{
		map_addr 	= PF8_A_U16(map_addr, U8_M_U8(compID, (SIZE_MEMREF + SIZE_U16)));
		map_index 	= U16t32(PF_R_U16(map_addr + SIZE_U16));
	}
	else
	{
		map_index 	= PF_R_U32(PF8_A_U8(map_addr, U8_M_U8(compID, (SIZE_MEMREF + SIZE_U16))));
	}

	return 	map_index;
	
}
