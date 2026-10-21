/*****************************************************************************
 * File:  reference.c
 * Version 1.10

 wang hui
 eastcompeace Technology Corporation

 History:
 Last modified on 07/07/03 at 13:23:48
 00/12/04 at 13:23:48 .Created

 ****************************************************************************/
#include "grt.h"
#include "ob_manager.h"
#include "reference.h"
#include "packageloc.h"
#include "context.h"
#include "global_variable.h"
#include "SEEUICCInterface.h"

#include "mask.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

/******************************************************************************
 Note: Parameter PKGID must be associated with the current
 reference context.
 modifed by xxj@20210328: mask中包对象的个数不变，后下载的包在下载过程中创建的对象由5个(class_component / method_component / staticfield_component / export_component / pkg_info)减少为1个(pkg_info)
                          pkg_info中的信息，mask中包对象保持不变，存储的分别是4个组建引用(4字节) + 组建大小(2字节) ; 后下载的包对象发生变化，存储的分别是4个组建地址相对于pkg_info起始地址的偏移(4字节) + 组建大小(2字节)
 ******************************************************************************/
memref resolveReferenceAddress(jref referenceID, u8 pkgID, u8 comp_type)
{
	u16		D_A		map_index;
	u32     D_A		map_offset;
	PF8		I_A		map_addr;
	PF8     I_A		map_addr_start;

	if (U16_AND_U16(referenceID, 0x8000) == 0)
	{
		// removed by WH 2007.7.4 no used , this can be done in the founction getPackageComponentLocation
		//but do like this must be faster
		/* Performance: Do not lookup on packageloc table if in current package */
		
		lastVMPkg = pkgID;
		if (pkgID == PCpkgID)
		{
			switch (comp_type)
			{
			case COMPONENT_CLASS:
				return U16_A_U32(referenceID, cachedClassCompLoc);
			case COMPONENT_METHOD:
				return U16_A_U32(referenceID, cachedMethodCompLoc);
			case COMPONENT_STATICFIELD:
//				if (cachedFieldCompLen >= referenceID)
				{
					return U16_A_U32(referenceID, cachedFieldCompLoc);
				}
//				faultDetected();
				//break; remove warning
			default:
				break;
			}
		}
		/* Reference is internal - resolve using package table */
		//TODO:check asm code RECHECK U8 FAR * OK
//		return (loadAddress_NOCache((u8 far *)loadAddress_NOCache((u8 far *)(G_CurExecuEnvironment.pkgLocationTable	+ pkgID * SIZE_MEMREF)) + (memref)(comp_type * (SIZE_MEMREF + SIZE_U16))) + referenceID);
		map_index	= PF_R_U16(PF8_A_U16(MEMtPF8(G_CurExecuEnvironment.pkgLocationTable + SIZE_U16), U8_M_U8(pkgID, SIZE_MEMREF)));
		map_addr_start	= MEMtPF8(MapTable_GetEntityContent(map_index));
		map_addr	= PF8_A_U16(map_addr_start, U8_M_U8(comp_type, (SIZE_MEMREF + SIZE_U16))); // 组件引用所处地址
		if (PF_R_U16(PF8_A_U8(map_addr , SIZE_U32)) >= referenceID) // 组建大小与referenceID进行比较
		{
			if(pkgID < NPACKAGES)
			{
				map_index	= PF_R_U16(map_addr + SIZE_U16); // 获取组件引用
				map_addr	= MEMtPF8(MapTable_GetEntityContent(map_index)); // 获取组件数据存储地址
			}
			else
			{
				map_offset	= PF_R_U32(map_addr);
				map_addr	= PF8_A_U32(map_addr_start, map_offset);

			}
			map_addr 	= PF8_A_U16(map_addr, referenceID);
			return PFtMEM(map_addr);
		}
		faultDetected();
	}
	
	/* Reference is external - resolve using global reference table */
	return (getReferenceAddress(referenceID));
}

u8 resolveReferencePackage(jref referenceID, u8 pkgID)
{
	return ((U16_AND_U16(referenceID, 0x8000) == 0) ? pkgID : getReferencePackageID(referenceID));
}
