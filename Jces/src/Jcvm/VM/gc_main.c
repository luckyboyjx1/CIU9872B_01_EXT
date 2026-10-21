/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*
 * @(#)gc_main.c	1.36 03/07/30
 */
#include "gc.h"
#include "stack.h"
#include "context.h"
#include "global_variable.h"
#include "chn_manager.h"
#include "cardManager.h"
#include "reference.h"
#include "packageloc.h"
#include "contentManager.h"
#include "ob_manager.h"
#include "RamManager.h"
#include "JcvmConfig.h"
#include "tranManager.h"
#include "NvmMap.h"
#include "JcvmRam.h"
#include "CgmManager.h"
#include "OPELFUpgrade.h"
#include "euiccConfig.h"
#include "SEEUICCInterface.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#define MAX_DEFERRED_OBJECT_TABLE_SIZE 40
static u16  appletDeletionContexts;
/*
 Stack cushion is needed because more stack space is in use when actual marking
 starts than the stack space in use when initializeGC is called.
 */
#define STACK_CUSHION 30

/**************************************************************************************
 Static Method declarations
 ***************************************************************************************/
void sweep(void);
u8 isMarked(u16 bitIndex);
u8 markObject(u16 objectId, u8 reset_reference_null, u32 referenceAddress);
u8 markNonRootObjects(u16 reset_reference_null);
u8 markChildren(u16 objectAddr, u16 limit, s32 remainingDepth,u8 reset_reference_null);
//static void initializeDeferredObjectTable(void);
static void putDeferredObject(u16 value);
static u16 getDeferredObject(void);

/*================================================================================
 * FUNCTION:      resetGarbageCollector
 * OVERVIEW:      Resets the garbage collector's state.
 * INTERFACE:
 *   parameters:  None
 *   returns:     None
 *=================================================================================*/
void resetGarbageCollector(void)
{
	/*reset all the variables so that we don't have any problems in the next cycle of GC*/
	bmp								= 0;
	deferredObjectTable				= 0;
	deferredObjectTableOverflow		= false;
	appletDeletion					= false;
	deletingPackage					= false;
	packageDeletedcontext			= U16t8(NULL_CONTEXT);
	packageDeletedID				= ILLEGAL_ID;
	appletDeletionContexts  = 0xFFFF;
}/*end resetGarbageCollector*/

/*================================================================================
 * FUNCTION:      intializeGC
 * OVERVIEW:      Gets the size of object and transients table to calculate the
 *                size of the bitmap needed to represent all these objects, creates
 *                this bitmap and then calls method "markStaticRoots" for all the
 *                ROM packages. Also finds out if enough memory is available for
 *                Deffereed Object List. If enough memory is available, sets the
 *                defferred object table address to the end of the bitmap.
 * INTERFACE:
 *   parameters:  None
 *   returns:     None
 *=================================================================================*/
#define availableForDefObjs  0x40
u8 initializeGC(void)
{
	
	u16			D_A		count,max_entry_count;
//	short X_A availableForDefObjs = 0;

	/*
	 Get object count which includes transients and use that to calculate the number
	 of bytes required in the bitmap to map all those objects
	 */
//	u16 X_A count = PX_R_U16(MAX_ENTRY_COUNT_IN_MAP) - MASK_MAP_ENTRY_COUNT;
	max_entry_count		= MapTable_GetTopIndex();

	count	= U16_S_U16(max_entry_count, MASK_MAP_ENTRY_COUNT);

#if GP__CGM__FOR_MASK
	if(bRamBuffer[GC_Requested] == 1)
	{
		CgmInit_changeFlag(CHECK_GC);
	}
	else
	{
		CgmInit_changeFlag(NO_CHECK);
	}
#endif

	if (U16_R_U8(count, 8) > 0)
	{
		bitMapCount = U8t16(U8_A_U16(1, U16_D_U8(count, 8)));
	}
	else
	{
		bitMapCount	= U16_D_U8(count, 8);
	}

	/*
	 To differentiate between transient and standard objects we have to know
	 the count of standard objects in the object table. We need to do this
	 because they're both being mapped into a single bitmap.
	 */


	/*
	 Find out what the maximum size of the defferred object list can be. That
	 also depends on how much stack space we have left since we are using the unused stack space
	 */

//	availableForDefObjs = 0x40;//APDUbuffer 最后64个字节


	/*There isn't enough memory available for any operation*/

	if((U16_A_U16(U16_S_U16(U16_S_U16(U32t16(U16_A_U16(availableForDefObjs, U32t16(PXtMEM(Top)))),U32t16(PXtMEM(JSP))),STACK_CUSHION),APDU_BUFFER_SIZE))< bitMapCount	)
	{
		resetGarbageCollector();
		throw_error(SYSTEMEXCEPT_NO_RESOURCE);		
	}
	defferredObjectTableSize	= U16t8(U16_D_U8(availableForDefObjs, sizeof(jref)));
	/*Defferred object list starts where bitmap ends*/
	if(bitMapCount < U16_S_U8(U16_S_U16(U32t16(PXtMEM(Top)), U32t16(PXtMEM(JSP))), STACK_CUSHION))//merged from /svn/cos2/JCES_PRODUCT/V2.0/FINANCE/SLX_SLC36PDL_352_VG/DEVELOP_PRODUCT/C_PROJECT, modify by jiangxiaohui@20220923:修复删除操作中JAVA STACK边界处理问题
	{
		bmp					= PXtMEM(PX8_A_U8(JSP, STACK_CUSHION));
		deferredObjectTable	= PXtMEM(PX8_A_U16(APDUBuffer, U16_S_U16(APDU_BUFFER_SIZE, availableForDefObjs)));
		/*
		 now that we have the starting point and size of our bitmap, initialize it
		 to zeros
		 */
		ecpMem_Set(MEMtPX8(bmp), 0, bitMapCount);
		mapTableTag			= 0;
	}
	else
	{		 // 目前测TCK  该else没有测到   不能确定是否有问题
		firstBmp			= PXtMEM(PX8_A_U8(JSP, STACK_CUSHION));
		firstBmpEnd			= PXtMEM(Top);  //merged from EPX3.0.1  modify by jiangxiaohui@20220804:GC  对象标记 firstBmpEnd 需要记录，后面mark会用到，firstBmpEnd 互用32位loader过程中的变量
//#if HAL__PLATFORM__SIMULATOR
//		secondBmp			= APDUBuffer;
//#endif
		//secondBmpEnd = APDUBuffer + (memref)(bitMapCount - (u16)(Top - JSP - STACK_CUSHION));
		deferredObjectTable = PXtMEM(PX8_A_U16(APDUBuffer, U16_S_U16(APDU_BUFFER_SIZE, availableForDefObjs)));
		/*
		 now that we have the starting point and size of our bitmap, initialize it
		 to zeros
		 */
		ecpMem_Set(MEMtPX8(firstBmp), 0, U16_S_U16(U16_S_U16(PXtMEM(Top), PXtMEM(JSP)), STACK_CUSHION));
		ecpMem_Set(MEMtPX8(secondBmp), 0, U16_S_U16(bitMapCount, U16_S_U8(U16_S_U16(PXtMEM(Top), PXtMEM(JSP)), STACK_CUSHION)));
		
		mapTableTag = 1;
	}

	return true;
}/*end initializeGC*/

/*==============================================================================
 * FUNCTION:      initAppletDeletion
 * OVERVIEW:      Initializes applet deletion by setting the appletDeletion flags
 *                and the address of the contexts array which contains the contexts
 *                of the applets that are going to be deleted. This array is checked
 *                against during marking phase of applet deletion. If any object
 *                is encountered that belongs to one of these contexts, the applet
 *                deletion operation is aborted.
 * INTERFACE:
 *   parameters:  contextsArrayHdr: header address of the contexts array
 *                count: Number of applets that are to be deleted. The contexts array
 *                is present in RAM.
 *
 *=============================================================================*/
void initAppletDeletion(u16 appletContext)
{

	appletDeletion			= true;
	/*
	 Since we're using the APDU buffer as our contexts array, the address
	 we have received is the header address of the APDU buffer. We need to
	 find out the data location in RAM.  Address in RAM is starts from the
	 7th byte in the header of this kind of object.
	 */
	appletDeletionContexts	= appletContext;

}/*end initAppletDeletion*/

/*==================added by linguo ming========================================
 * FUNCTION:      initPackageDeletion
 * OVERVIEW:      Initializes package deletion by setting the deletingPackage flags
 *                and the contexts of the package that are going to be deleted. 
 *                This contexts is checked against during marking phase of package deletion. 
 *                If any object is encountered that belongs to one of these contexts, 
 *                the package deletion operation is aborted.
 * INTERFACE:
 *   parameters:  contextsArrayHdr: header address of the contexts array
 *                count: Number of applets that are to be deleted. The contexts array
 *                is present in RAM.
 *
 *=============================================================================*/
void initPackageDeletion(u8 pkgID)
{

	deletingPackage			= true;
	packageDeletedID		= pkgID;
	/*
	 find out the context of the package
	 */
//	packageDeletedID = pkgID;
	packageDeletedcontext	= U16t8(getPkgContext(pkgID));

}/*end initPackageDeletion*/
/*================================================================================
 * FUNCTION:      belongsToDeletedApplet
 * OVERVIEW:      During applet deletion this method is called to check if the
 *                object being marked belongs to one of the applets being deleted.
 * INTERFACE:
 *   parameters:  objectId: Id of the object being deleted.
 *   returns:     TRUE if the object belongs to any of the context of applets being
 *                deleted. FALSE otherwise
 *=================================================================================*/
u8 belongsToDeletedApplet(jref objectId, u8 reset_reference_null, u32 referenceAddress)
{
	if (appletDeletionContexts == nvmReadU16(PN8_A_CONST(MEMtPN8(locateObjectInMemory(objectId)), OBJ_HDR_Context)))
	{
		if((reset_reference_null == true) || (IS_ENTRY_OBJ(MEMtPN8(locateObjectInMemory(objectId)))) || IS_GLOBAL_ARRAY(MEMtPN8(locateObjectInMemory(objectId))))
		{
			nvmWriteU16NoAtomic(MEMtPN8(referenceAddress), 0);//nvmWriteU32NoAtomic(MEMtPN8(referenceAddress), 0);
		}
		else
		{
			//return TRUE;
			resetGarbageCollector();
			throw_error(SECURITY_EXCEPTION);
		}
	}
	return false;
}/*end belongsToDeletedApplet*/

/*================================================================================
 * FUNCTION:      checkObjectRootSDEqual
 * OVERVIEW:      查找packageContext 或者 appletId 关联的SD是否 与所给 sdId 对应的SD一致
 * INTERFACE:
 *   parameters:  packageContext, appletId, sdId
 *   returns:     TRUE 如果关联的sd或者关联的root SD 与sdId相同
 *=================================================================================*/
u8 checkObjectRootSDEqual(u8 packageContext, u8 appletId, u8 sdId)
{
	memref   X_A	packageContextTable;
	u8 		 X_A	packageId;
	jref 	 X_A	packageEntry;
	memref   X_A	entryAddr;
	u8 		 X_A	assoSDid;
	u8       X_A	index;
	u32 	 X_A	appEntry;

	if(packageContext != 0xFF)		   //包查找	，如果找到即返回TRUE
	{
		packageContextTable 	= get_array_data(MEMtPF8(locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.packageContextTable))));
		packageId			 	= nvmReadU8(MEMtPN8(U32_A_U8(packageContextTable, packageContext)));
		packageEntry			= getPkgEntry(packageId);
		if (packageEntry != REF_NULL)
		{
			entryAddr = locateObjectInMemory(packageEntry);
			assoSDid = nvmReadU8(MEMtPN8((get_object_field_ref(MEMtPF8(entryAddr), sdId_INDEX))));
			if(assoSDid == sdId)
			{
				return true;
			}
			else
		 	{
				//继续查找 assoSDid 对应的SD所关联的root SD
			 	if(cacheBuffer[110] != 0xFD)
				{
					 return checkObjectRootSDEqual(0xFF, assoSDid, sdId);
				}
		 	}
		}
	}
	if(appletId != 0xFF)			 //应用查找	，如果找到即返回TRUE
	{
		 index 		= U16t8(getIndexByID(appletId));
		 appEntry 	= locateObjectInMemory(PF_R_U16(MEMtPF8(U32_A_U16(APP_Table_Cash, U16_M_U16(U8t16(index), SIZE_REF)))));

	//	 if(appEntry != REF_NULL)
		 if((appEntry != REF_NULL) && (appEntry != MEM_NULL) )
		 {
			 assoSDid = nvmReadU8(MEMtPN8((get_object_field_ref(MEMtPF8(appEntry), bsdId_INDEX))));
			 if(assoSDid == sdId)
			 {
					return true;
			 }
			 else
			 {
			 	if(assoSDid == appletId)
				{
					cacheBuffer[110] = 0xFD;			//表明root sd 被找到
					return false;
				}
			 	 //继续查找 assoSDid 对应的SD所关联的root SD
			 	if(cacheBuffer[110] != 0xFD)
				{
					 return checkObjectRootSDEqual(0xFF, assoSDid, sdId);
				}
			 }
		 }
		

	}
	//找不到，返回FALSE
	return false;
}


/*================================================================================
 * FUNCTION:      belongsToDeletedPackage
 * OVERVIEW:      During package deletion this method is called to check if the
 *                object being marked belongs to one of the applets being deleted.
 * INTERFACE:
 *   parameters:  objectId: Id of the object being deleted.
 *   returns:     TRUE if the object belongs to any of the context of applets being
 *                deleted. FALSE otherwise
 *=================================================================================*/
u8 belongsToDeletedPackage(jref objectId, u8 reset_reference_null, u32 referenceAddress)
{
	  u16	  D_A    objectCC;
	  u8 	  D_A	 result;
	/*Get the object address and then it's context and try to see if it matches any of
	 of the contexts of the package that are being deleted*/

	objectCC	= nvmReadU16(PN8_A_CONST(MEMtPN8(locateObjectInMemory(objectId)), OBJ_HDR_Context));

	if (packageDeletedcontext == U16t8(U16_SHR_U8(objectCC, 8)))
	{
		if(((U16t8(NULL_CONTEXT) == packageDeletedcontext) && (packageDeletedID == U16t8(U16_AND_U16(objectCC, APPLET_MASK))))
		   || (U16t8(NULL_CONTEXT) != packageDeletedcontext)
		  )
		{
			if(reset_reference_null == true)
			{
				if(cacheBuffer[110] != 0xff)				  //110的注释在java层，= OPPlatform.SD_ID
				{
					//检查objectId 对应的包或者应用 关联的sd是否与被删除包的相同，如果不同，则不能删除该包，只能逻辑删除
					result = checkObjectRootSDEqual(U16t8(U16_SHR_U8(objectCC, 8)), U16t8(objectCC), cacheBuffer[110]);
					if(result == false)
					{
						cacheBuffer[110] = 0xFE;			//0xFE的注释在java层，= GarbageCollector.LOGICALLY_DELETE_TAG
						return true;
					}
				}
				nvmWriteU16NoAtomic(MEMtPN8(referenceAddress), 0);//nvmWriteU32NoAtomic(MEMtPN8(referenceAddress), 0);
			}
			else if((IS_ENTRY_OBJ(MEMtPN8(locateObjectInMemory(objectId)))) || IS_GLOBAL_ARRAY(MEMtPN8(locateObjectInMemory(objectId))))
			{
				nvmWriteU16NoAtomic(MEMtPN8(referenceAddress), 0);//nvmWriteU32NoAtomic(MEMtPN8(referenceAddress), 0);
			}
			else
			{
				//return TRUE;
				resetGarbageCollector();
				throw_error(SECURITY_EXCEPTION);
			}
		}
	}/*end if*/

	return false;
}/*end belongsToDeletedPackage*/
/*================================================================================
 * FUNCTION:      markAppletRoot
 * OVERVIEW:      This method marks the applet table as the root of all applets.
 * INTERFACE:
 *   parameters:  ObjectId of the applet table.
 *   returns:     1 or TRUE
 *=================================================================================*/
/**
 * Marks the root of applet roots, the applet table as a root.
 */
u8 markAppletRoot(jref objectId)		SMALL_MODE
{
	if (objectId == 0)
	{
		throw_error(ASSERTION_FAILED);
	}
	markObject(objectId, 0, 0);
	return true;
}/*end markAppletRoot*/
							  	
/*================================================================================
 * FUNCTION:      markStaticRoots
 * OVERVIEW:      marks the objects being pointed to be the fields in the static
 *                components of a package as roots.
 * INTERFACE:
 *   parameters:  SFCAddress: Static Field Component address
 *                refCount: Count of reference type fields in the static field
 *                          component.
 *   returns:     TRUE or 1 if successful, FALSE or 0 if failed. This function can
 *                only fail during applet deletion, when an attempt is made to
 *                mark an object that belongs to one of the applets being deleted.
 *=================================================================================*/
u8 markStaticRoots(u8 pkgID, u16 refCount, u8 reset_reference_null)
{
	u16			D_A		j;
	PF8			D_A		SFCAddress;
	jref		D_A	    objectId;
	if (((deletingPackage == true) && (packageDeletedID == pkgID)) || (refCount == 0))
	{
		return true;
	}

	SFCAddress	= MEMtPF8(getPackageComponentLocation(pkgID, COMPONENT_STATICFIELD));
	for (j = 0; j < refCount; U16_INC(j))
	{
		objectId	= nvmReadU16(SFCAddress);
		/*
		 if the object Id is greater than 0 and less than INVALID_ID. We cannot use
		 assertion here because we are just going blindly through the static field
		 components and static fields may have been left uninitialized.
		 */
		if (objectId > 0 && objectId < INVALID_ID)
		{
			/*This can only fail during applet deletion. The case will be when the object
			 pointed to by a static field belongs to one of the applets being deleted*/
			if (!markObject(objectId, reset_reference_null, PFtMEM(SFCAddress)))
			{
				resetGarbageCollector();
				return false;
			}/*end if*/
		}/*end if*/
		/*Go to the next reference type field*/
		SFCAddress	= PF8_A_U8(SFCAddress, sizeof(jref));
	}/*end for*/
	return true;
}/*end markStaticRoots*/

/*================================================================================
 * FUNCTION:      doGC
 * OVERVIEW:      main function that is to be called by the JCRE to start garbage
 *                collection process.
 * INTERFACE:
 *   parameters:  None
 *   returns:     TRUE if successful, FALSE otherwise. This function can only fail
 *                during applet deletion when an attempt is made to mark an object
 *                belonging to one of the applets being deleted.
 *=================================================================================*/
u8 doGC()
{
	u8		D_A		GCSuccessful;

	GCSuccessful = false;
	/*mark non-root objects*/
	if (markNonRootObjects(U16t8(popShort())))
	{
		/*Since marking was successful, start the sweep cycle.*/
		sweep();
		GCSuccessful = true;
	}/*end if*/

	resetGarbageCollector();
	return GCSuccessful;
}/*end doGC*/

/*================================================================================
 * FUNCTION:      isMarked
 * OVERVIEW:      Checks if a certain object has been marked.
 * INTERFACE:
 *   parameters:  Object Id
 *   returns:     A non-zero value of the object is marked, 0 otherwise.
 *=================================================================================*/
/*
static u8 isMarked(u16 objectId)
{
	u16 arrayIndex;
	u8 bitChecker = 1;
	u8 bitNumber;

	if (objectId > TRANSIENT_MASK)
		objectId = objectId - TRANSIENT_MASK + objectTableObjectCount;
	//find the index in the array where this bit may be	
	arrayIndex = (u16) (objectId / (u16) 8);
	bitNumber = objectId % 8;


	//getting the nth power of 2
	bitChecker = bitChecker << bitNumber;

	return (RAM_read_u8((memref)(bmp + arrayIndex)) & bitChecker);
}//end isMarked	  */
u8 isMarked(u16 objectId)
{
	u16			D_A		arrayIndex;
	u8			D_A		bitChecker;
	u8			D_A		bitNumber;

	bitChecker	= 1;
	if (objectId <= MASK_MAP_ENTRY_COUNT)
		return false;

	objectId	= U16_S_U16(objectId, MASK_MAP_ENTRY_COUNT);
	/*find the index in the array where this bit may be	*/
	arrayIndex	= U16_D_U8(objectId, 8);
	bitNumber	= U16t8(U16_R_U8(objectId, 8));


	/*getting the nth power of 2*/
	bitChecker = U8_SHL_U8(bitChecker, bitNumber);
	if(!mapTableTag)
	{
		return (U8_AND_U8(PX_R_U8(PX8_A_U16(bmp, arrayIndex)), bitChecker));
	}
	else
	{
	//	if(arrayIndex <= U16_S_U16(PXtMEM(firstBmpEnd), PXtMEM(firstBmp)))
	    if(arrayIndex <  U32t16(U32_S_U32(firstBmpEnd, firstBmp))) //merged from EPX3.0.1 modify by jiangxiaohui@20220804:GC  对象标记 firstBmpEnd 需要记录，后面mark会用到，firstBmpEnd 互用32位loader过程中的变量
		{
			return U8_AND_U8(PX_R_U8(PX8_A_U16(firstBmp, arrayIndex)), bitChecker);	
		}
		else
		{
		//	arrayIndex	= U16_S_U16(arrayIndex, U16_S_U8(U16_S_U16(PXtMEM(firstBmpEnd), PXtMEM(firstBmp)), 1));// - 1 ：取得secondMap的首地址  //(lzm modified according [MISA])
			arrayIndex	= U16_S_U16(arrayIndex, U32t16(U32_S_U32(firstBmpEnd, firstBmp)));// - 1 ：取得secondMap的首地址  //(lzm modified according [MISA]) merged from EPX3.0.1  modify by jiangxiaohui@20220804:GC  对象标记 firstBmpEnd 需要记录，后面mark会用到，firstBmpEnd 互用32位loader过程中的变量
			return U8_AND_U8(PX_R_U8(PX8_A_U16(secondBmp, arrayIndex)), bitChecker);	
		}
	}
}/*end isMarked*/

/*================================================================================
 * FUNCTION:      markObject
 * OVERVIEW:      Marks an object as reachable in the bitmap. During applet
 *                this function calls belongsToDeletedApplet() function to verify
 *                if the object being marked does not belong to any of the applets
 *                being deleted.
 * INTERFACE:
 *   parameters:  object Id
 *   returns:     TRUE or 1 if successful, 0 otherwise.
 *=================================================================================*/
u8 markObject(u16 objectId, u8 reset_reference_null, u32 referenceAddress)
{
	u8		D_A		bitChecker;
	u16		D_A		arrayIndex;
	u8		D_A		bitNumber;

	bitChecker = 1;

	if (objectId <= MASK_MAP_ENTRY_COUNT)
		return true;
	/*
	 If we're doing applet deletion, then we have to check before marking an object
	 if the object belongs to the applet being deleted. If that's the case, we cannot
	 do applet deletion and FALSE is returned.
	 */
	if (appletDeletion == true)
	{
		/*Check if this object being marked belongs to the applet being deleted*/
		if (belongsToDeletedApplet(objectId, reset_reference_null, referenceAddress) == true)
		{
			/*if object did belong to one of the applets being deleted, return FALSE*/
			//return FALSE;
			throw_error(SECURITY_EXCEPTION);
		}/*end if*/
	}/*end if*/


	/*
	 linguoming:check that if the current package reference the objects belonging to the
	 deleted package
	 */
	if (deletingPackage == true)
	{
		/*Check if this object being marked belongs to the package being deleted*/
		if (belongsToDeletedPackage(objectId, reset_reference_null, referenceAddress) == true)
		{
			/*if object did belong to one of the applets being deleted, return FALSE*/
			//return FALSE;
			throw_error(SECURITY_EXCEPTION);
		}/*end if*/
	}/*end if*/

	objectId	= U16_S_U16(objectId, MASK_MAP_ENTRY_COUNT);
	arrayIndex	= U16_D_U8(objectId, 8);
	bitNumber	= U16t8(U16_R_U8(objectId, 8));
	
	if(arrayIndex >= bitMapCount)//merged from /svn/cos2/JCES_PRODUCT/V2.0/FINANCE/SLX_SLC36PDL_352_VG/DEVELOP_PRODUCT/C_PROJECT, modify by jiangxiaohui@20220923:修复删除操作中JAVA STACK边界处理问题
	{
		throw_error(SW_BIPMAP_OVER_FLOW);
	}

	/*getting the nth power of 2*/
	bitChecker = U8_SHL_U8(bitChecker, bitNumber);
	if(!mapTableTag)
	{
		PX_W_U8(PX8_A_U16(bmp, arrayIndex), U8_OR_U8(PX_R_U8(PX8_A_U16(bmp, arrayIndex)), bitChecker));
	}
	else
	{
	//	if(arrayIndex <= U16_S_U16(PXtMEM(firstBmpEnd), PXtMEM(firstBmp)))
		if(arrayIndex < U32t16(U32_S_U32(firstBmpEnd, firstBmp))) //merged from EPX3.0.1 modify by jiangxiaohui@20220804:GC  对象标记 firstBmpEnd 需要记录，后面mark会用到，firstBmpEnd 互用32位loader过程中的变量
		{                                                         //merged from /svn/cos2/JCES_PRODUCT/V2.0/FINANCE/SLX_SLC36PDL_352_VG/DEVELOP_PRODUCT/C_PROJECT, modify by jiangxiaohui@20220923:修复删除操作中JAVA STACK边界处理问题
			PX_W_U8(PX8_A_U16(firstBmp, arrayIndex), U8_OR_U8(PX_R_U8(PX8_A_U16(firstBmp, arrayIndex)), bitChecker));
		}
		else
		{
			arrayIndex	= U16_S_U16(arrayIndex, U16_S_U8(U32t16(U32_S_U32(firstBmpEnd, firstBmp)), 1));// - 1 ：取得secondMap的首地址  //(lzm modified according [MISA]) merged from EPX3.0.1  modify by jiangxiaohui@20220804:GC  对象标记 firstBmpEnd 需要记录，后面mark会用到，firstBmpEnd 互用32位loader过程中的变量
			PX_W_U8(PX8_A_U16(secondBmp, arrayIndex), U8_OR_U8(PX_R_U8(PX8_A_U16(secondBmp, arrayIndex)), bitChecker));
		}
	}

	return true;
		
}/*end markObject*/

#define MAX_GC_DEPTH 4
/*
* 对升级旧包的删除采用私有的mark方式，避免升级所需数据被删除
* merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
*/
u8 markUpgradeOnSaveObjects(u8 reset_reference_null)
{
#if GP__AMENDMENT__H__FOR_MASK
	u8 markbmp[STACKSIZE+APDU_BUFFER_SIZE-STACK_CUSHION];
	jref		D_A		pkgEntry;
	PF8			D_A		pkgEntryAddr;
//	u16			D_A		count,max_entry_count;
	u16 refCount;
	u16 i;
	jref		D_A		scanner;
	u16			D_A		bitIndex;
	u16			D_A		TotalObjectCount;

	u1            deletingPackageBak;
	deletingPackageBak = deletingPackage;
	deletingPackage = false;
	if(mapTableTag == 0)
	{

		ecpMem_Copy(markbmp,bmp,bitMapCount);
		ecpMem_Set(MEMtPX8(bmp), 0, bitMapCount);
	}
	else
	{
		ecpMem_Copy(markbmp,firstBmp,U16_S_U16(U16_S_U16(PXtMEM(Top), PXtMEM(JSP)), STACK_CUSHION));
		ecpMem_Set(MEMtPX8(firstBmp), 0, U16_S_U16(U16_S_U16(PXtMEM(Top), PXtMEM(JSP)), STACK_CUSHION));
		ecpMem_Copy(PX8_A_U16(markbmp, U16_S_U16(U16_S_U16(PXtMEM(Top), PXtMEM(JSP)), STACK_CUSHION)),secondBmp,U16_S_U16(bitMapCount, U16_S_U8(U16_S_U16(PXtMEM(Top), PXtMEM(JSP)), STACK_CUSHION)));
		ecpMem_Set(MEMtPX8(secondBmp), 0, U16_S_U16(bitMapCount, U16_S_U8(U16_S_U16(PXtMEM(Top), PXtMEM(JSP)), STACK_CUSHION)));


	}
	pkgEntry		= getPkgEntry(UPGRADE_IMPL_PKG_ID);
	pkgEntryAddr	= MEMtPN8(locateObjectInMemory(pkgEntry));
	refCount	= nvmReadU16(MEMtPN8(get_object_field_ref(pkgEntryAddr, pkgStaticReferenceCount_INDEX)));
	//refCount=getStaticComponent_ReferenceCount(UPGRADE_IMPL_PKG_ID,COMPONENT_STATICFIELD);

	if(markStaticRoots(UPGRADE_IMPL_PKG_ID, refCount, reset_reference_null))
	{


		//u8 			D_A		reset_reference_null;
		TotalObjectCount	= MapTable_GetTopIndex();
		//reset_reference_null= U16t8(popShort());
		do
		{
			initializeDeferredObjectTable();
			/*
			 This for loop starts from the start of the bitmap and calls markChildren on all
			 objects that have been marked already.
			 */
			for (scanner = 1, bitIndex = 1; bitIndex <= TotalObjectCount; U16_INC(scanner), U16_INC(bitIndex))
			{
	#if (EUICC_ONLY||SE_AND_EUICC)
				if(scanner > MASK_MAP_ENTRY_COUNT)
				{
					if(!MapTable_IsCurrentProfile(scanner, G_CurExecutingProfileIndex))
					{
						markProfileObject(scanner);
						continue;
					}
				}
	#endif
				if(!MapTable_IsCreationObject(scanner))
				{
					continue;
				}

				if  ((bitIndex > MASK_MAP_ENTRY_COUNT) && isMarked(bitIndex))
	//			if ((bitIndex <= MASK_MAP_ENTRY_COUNT)  && isMarked(bitIndex))
				{
					if (markChildren(scanner, bitIndex, MAX_GC_DEPTH, reset_reference_null) == 0)
					{
						return false;
					}/*end if*/
				}/*end if*/
			}/*end for*/
			/* This loop runs exactly once in almost all cases */
		} while (deferredObjectTableOverflow);/*end do-while*/
		deletingPackage = deletingPackageBak;

		if(mapTableTag == 0)
		{
			for(i=0;i<bitMapCount;i++)
			{
				PX_W_U8(PX8_A_U8(bmp, i), U8_OR_U8(PX_R_U8(PX8_A_U8(bmp, i)), PX_R_U8(PX8_A_U8(markbmp, i))));
				//bmp[i]=U8_OR_U8(bmp[i],markbmp[i]);
			}

		}
		else
		{
			for(i=0;i<U16_S_U16(U16_S_U16(PXtMEM(Top), PXtMEM(JSP)), STACK_CUSHION);i++)
			{
				PX_W_U8(PX8_A_U8(firstBmp, i), U8_OR_U8(PX_R_U8(PX8_A_U8(firstBmp, i)), PX_R_U8(PX8_A_U8(markbmp, i))));
				//firstBmp[i]=U8_OR_U8(firstBmp[i],markbmp[i]);
			}
			for(i=0;i<U16_S_U16(bitMapCount, U16_S_U8(U16_S_U16(PXtMEM(Top), PXtMEM(JSP)), STACK_CUSHION));i++)
			{
				PX_W_U8(PX8_A_U8(secondBmp, i), U8_OR_U8(PX_R_U8(PX8_A_U8(secondBmp, i)), PX_R_U8(PX8_A_U8(markbmp, i+U16_S_U16(U16_S_U16(PXtMEM(Top), PXtMEM(JSP)), STACK_CUSHION)))));
				//secondBmp[i]=U8_OR_U8(secondBmp[i],markbmp[i+U16_S_U16(U16_S_U16(PXtMEM(Top), PXtMEM(JSP)), STACK_CUSHION)]);
			}
		}
		}
#endif
    return 1;
}


/**********************************************************************
 PERSONAL HELP NOTE
 "jref" is used where address is being stored into a variable.
 u16 is used where we need an unsigned short.
 u8 is used where we need an unsigned char (in other words byte)

 Our bit indexes in the bitmap start with one. This is to take care of
 a special case when the bit index is 0, which was becoming a headache
 because it's the same as 8th bit if we go with the remainder technique
 to check the bit's state in the bitmap.
 -Saqib
 ***********************************************************************/

/*================================================================================
 * FUNCTION:      markNonRootObjects
 * OVERVIEW:      Once all roots have been marked, this method is called to start
 *                the general marking process to mark all the objects reachable
 *                from roots.
 * INTERFACE:
 *   parameters:  None
 *   returns:     TRUE or 1 if successful, 0 otherwise.
 *   merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP to support GP__AMENDMENT__H__FOR_MASK
 *=================================================================================*/
u8 markNonRootObjects(u16 reset_reference_null)
{
	jref		D_A		scanner;
	u16			D_A		bitIndex;
	u16			D_A		TotalObjectCount;
	//u8 			D_A		reset_reference_null;
	TotalObjectCount	= MapTable_GetTopIndex();
	//reset_reference_null= U16t8(popShort());
	do
	{
		initializeDeferredObjectTable();
		/*
		 This for loop starts from the start of the bitmap and calls markChildren on all
		 objects that have been marked already.
		 */
		for (scanner = 1, bitIndex = 1; bitIndex <= TotalObjectCount; U16_INC(scanner), U16_INC(bitIndex))
		{
#if (EUICC_ONLY||SE_AND_EUICC)
			if(scanner > MASK_MAP_ENTRY_COUNT)
			{
				if(!MapTable_IsCurrentProfile(scanner, G_CurExecutingProfileIndex))
				{
					markProfileObject(scanner);
					continue;
				}
			}
#endif
			if(!MapTable_IsCreationObject(scanner))
			{
				continue;
			}

			if ((bitIndex <= MASK_MAP_ENTRY_COUNT) || ((bitIndex > MASK_MAP_ENTRY_COUNT) && isMarked(bitIndex)))
//			if ((bitIndex <= MASK_MAP_ENTRY_COUNT)  && isMarked(bitIndex))
			{
				if (markChildren(scanner, bitIndex, MAX_GC_DEPTH, reset_reference_null) == 0)
				{
					return false;
				}/*end if*/
			}/*end if*/
		}/*end for*/
		/* This loop runs exactly once in almost all cases */
	} while (deferredObjectTableOverflow);/*end do-while*/

#if GP__AMENDMENT__H__FOR_MASK
		if( PN_R_U8(upGradeStatus_INDEX)> UPGRADE_STATS_COMPLETED)
		{
			markUpgradeOnSaveObjects(false);
		}
#endif
	return true;
}/*end markNonRootObjects*/


u8 markChildren(u16 objectId, u16 limit, s32 remainingDepth, u8 reset_reference_null)
{

#define MARK_AND_TAIL_RECURSE(child, reset_reference_null, referenceAddress)                                          \
    if (child != 0 && !isMarked(child))                    \
	{                                                      \
         bitIndex = child;                                                     \
         if (markObject(child, reset_reference_null, referenceAddress))                                             \
		 {                                                 \
             if (bitIndex < limit)                         \
			 {                                             \
                if (nextObject != 0)                       \
				{                                          \
                     RECURSE(nextObject);                                      \
                }/*end if*/                                                   \
                 nextObject = (child);                                         \
			 }/*end if*/                                                       \
         }                                                 \
		 else                                               \
		 {                                                  \
		     return false;                                   \
		 }			                                       \
	}/*end if*/                                            \

#define RECURSE(child)                                       \
	if (remainingDepth < 0)                                  \
	{                                                        \
		putDeferredObject(child);                             \
    }                                                        \
	else                                                     \
	{                                                        \
       markChildren(child, limit, remainingDepth, reset_reference_null);           \
    }/*end if-else*/                                         \


	/*
	 If nextObject is non-zero, then it holds the value of object for the next iteration
	 through the loop.  Used to implement tail recursion.
	 */
	u16		nextObject;
	/*bit in the bitmap corresponding to the object being processed*/
	u16		bitIndex;
	u8		back_selected_channelID;
	/* remaining depth for any subcalls */
	nextObject		= 0;
	bitIndex		= 0;
	back_selected_channelID = selected_channel;
	U32_DEC(remainingDepth);
	
	while(1)
	{
		/*first byte of the object header from which we get the object type*/
		u8				header;
		/*number of reference type fields in an object*/
		u16				refCount;
		/*address of the first reference type field in an object*/
		PF8			firstRefAddress;
		/*address of the object being processed*/
		PF8			objectAddress;
		/*counter for for loop*/
		u16				index;
		jref			classRef;
		/*address of the class structure*/
		PF8			classAddress;
		/*class package*/
		u8				clPkg;
		/*variable to hold first reference index in the class structure*/
		u8				firstRefIndex;
		u16				subObject;
		u16				objectContext;


		/*Variable used to hold the channel number for a DTR object belonging to currently selected
		 applet */
		u8		channelId;

		channelId		= 0xFF;
		objectAddress	= MEMtPF8(locateObjectInMemory(objectId));
		//assert that object address is not NULL_POINTER
		if (check_addr_null(objectAddress))
		{
			throw_error(ASSERTION_FAILED);
		}

		header			= PF_R_U8(objectAddress);
		switch (TYPE(header))
		{
		case GC_SIMPLE_INSTANCE:
			/*  The object is a Java object instance.  Mark pointer fields */
			/*  Get the class: 3rd field in the instance*/
			classRef				= PF_R_U16(PF8_A_CONST(objectAddress, OBJ_HDR_ClassRecordREF));
			clPkg					= PF_R_U8(PF8_A_CONST(objectAddress, OBJ_HDR_ClassRecordPKG));
			classAddress 			= MEMtPF8(resolveReferenceAddress(classRef, clPkg, COMPONENT_CLASS));
			do
			{
				refCount			= U8t16(PF_R_U8(PF8_A_CONST(classAddress, CR_ReferenceCount)));
				firstRefIndex		= PF_R_U8(PF8_A_CONST(classAddress, CR_FirstRefIndex));
				if (firstRefIndex != 0xFF)
				{
					firstRefAddress	= PF8_A_U16(objectAddress, U8_A_U16(SIZE_OBJECT_HEADER, U8_M_U8(firstRefIndex, sizeof(jref))));
					for (index = 0; index < refCount; U16_INC(index))
					{
						/*index * sizeof(jref) because references consist of 2 bytes*/
						subObject	= nvmReadU16(PM8_A_U16(firstRefAddress, U8_M_U16(sizeof(jref), index)));
						MARK_AND_TAIL_RECURSE(subObject, reset_reference_null,PMtMEM(PM8_A_U16(firstRefAddress, U8_M_U16(sizeof(jref), index))));
					}/*end for*/
				}/*end if*/
				/*
				 get super class because now we have to go through the fields
				 that come from the super class
				 */
				classRef	= PF_R_U16(PF8_A_CONST(classAddress, CR_SuperClass));
				if (classRef != TERMINAL_CLASS_REF)
				{
					//get the actual address
					classAddress = MEMtPF8(resolveReferenceAddress(classRef, clPkg, COMPONENT_CLASS));
					clPkg = lastVMPkg;
				}
			} while (classRef != TERMINAL_CLASS_REF); /*end do-while*/
			break;

		case GC_BOOLEAN_ARRAY:
		case GC_BYTE_ARRAY:
		case GC_SHORT_ARRAY:
		case GC_INT_ARRAY:
		case GC_TRANSIENT_BOOLEAN_ARRAY:
		case GC_TRANSIENT_BYTE_ARRAY:
		case GC_TRANSIENT_SHORT_ARRAY:
		case GC_TRANSIENT_INT_ARRAY:
			/*
			 The object is a Java array with primitive values. Transient
			 or non-transient, doesn't matter. Don't have to do anything here.*/
			break;
		case GC_REFERENCE_ARRAY:
			/*
			 The object is a reference type array. Mark and recurse on all the entries
			 */

			/*get to the first entry in the array*/
			firstRefAddress		= PF8_A_CONST(objectAddress, SIZE_OBJECT_ARRAY_HEADER);

			/*count is the 5th and 6th byte in the object header*/
			refCount			= PF_R_U16(PF8_A_CONST(objectAddress, OA_NumFields_WORD));
			for (index = 0; index < refCount; U16_INC(index))
			{
				subObject		= nvmReadU16(PM8_A_U16(firstRefAddress, U8_M_U16(sizeof(jref), index)));
				MARK_AND_TAIL_RECURSE(subObject, reset_reference_null, PMtMEM(PM8_A_U16(firstRefAddress, U8_M_U16(sizeof(jref), index))));
			}/*end for*/
			break;
		case GC_TRANSIENT_REFERENCE_ARRAY:
			/*
			 The object is a transient reference type array. Entries for this are
			 in RAM. Mark and recurse on all entries. In case of deselect reference
			 type arrays, mark only objects pointed to by the array belonging to the
			 currently selected context.
			 */
			
			firstRefAddress = MEMtPF8(get_array_data(objectAddress));
			if (U8_AND_U8(header, HEADER_JCRE_TransMode_Deselect) == HEADER_JCRE_TransMode_Deselect)
			{

                // modified by wh 2008.12.8 ,this break the limited of applets number on card.
				//u8 objectContext = PN_R_U8((memref) (objectAddress + OBJ_HDR_Context)); /*object context*/
                objectContext	= PF_R_U16(PF8_A_CONST(objectAddress, OBJ_HDR_Context)); /*object context*/
                for (index = 0; index < U8t16(logicalChannelNums); U16_INC(index)) {
                    if (getChannelState(U16t8(index)) != CHANNEL_CLOSED) {
						// modify by WH 2007.7.4
					#if (EUICC_ONLY||SE_AND_EUICC)
                    	if(getChannelProfileIndex(index) == G_CurExecutingProfileIndex)
					#endif
                    	{
							if(U16_SHR_U8(getChannelContext(U16t8(index)),GROUP_SHIFT_MASK) == U16_SHR_U8(objectContext, GROUP_SHIFT_MASK)){
						   // if((getChannelContext((u8)index) >> GROUP_SHIFT_MASK) == (objectContext >>  GROUP_SHIFT_MASK)){
								channelId	= U16t8(index);
								break;
							}
                    	}
                    }
                }
                if (channelId == 0xFF){
                    break;
                }else{
                    /**
                     * get the address in RAM from the object header
                     //have to use a special logical channel thingy in this case to get the data pointer
                     */
					//modified by WH 2007.7.5 ,we removed the method get_array_data_special(channelId, objectAddress)
					//use get_array_data(objectAddress) replaced;
					selected_channel	= channelId;
					firstRefAddress		= MEMtPF8(get_array_data(objectAddress));
                    //firstRefAddress = get_array_data_special(channelId, objectAddress);
                    // recovered the selected channelID
                    selected_channel	=  back_selected_channelID;
                }
            }

	//		if ((header & HEADER_JCRE_TransMode_Deselect) == HEADER_JCRE_TransMode_Deselect)
		//	{
		//		u16 objectContext = E2P_read_u16((memref) (objectAddress + OBJ_HDR_Context)); /*object context*/  
			//	if((channelInfo.selected_context[(u8) (index)] >> GROUP_SHIFT_MASK) == (objectContext >>  GROUP_SHIFT_MASK))
//				if ((selected_context >> GROUP_SHIFT_MASK) != (objectContext >> GROUP_SHIFT_MASK))
	//			{
		//			break;
		//		}
	//		}
			/** get the address in RAM from the object header for RTR and DTR type of reference array*/

			/*count is the 5th and 6th byte in the object header*/
			refCount	= PF_R_U16(PF8_A_CONST(objectAddress, OA_NumFields_WORD));

			for (index = 0; index < refCount; U16_INC(index))
			{
				subObject	= PM_R_U16(PM8_A_U16(firstRefAddress, U8_M_U16(sizeof(jref), index)));
				MARK_AND_TAIL_RECURSE(subObject, reset_reference_null, PMtMEM(PM8_A_U16(firstRefAddress, U8_M_U16(sizeof(jref), index))));
			}/*end for*/
			break;
		default:
			break;
		} /* End of switch statement */

		if (nextObject != 0)
		{
			/* We can perform a tail recursive call. */
			objectId	= nextObject;
			nextObject	= 0;
			/* continue */
		}
		else if ((remainingDepth == U32_S_U8(MAX_GC_DEPTH, 1)) && (deferredObjectCount > 0))
		{
			/* We're at the outer level of recursion and there is a deferred
			 * object for us to look up.  Note that remainingDepth has
			 * already been decremented, so we don't compare against
			 * MAX_GC_DEPTH
			 */
			objectId = getDeferredObject();
			/* continue */
		}
		else
		{
			break; /* finish "for" loop. */
		}
	} /* end of for(ever) loop */
	objectId = 0;
	return true;
}/*end markChildren*/

/*=========================================================================
 * Deferred objects, as part of markChildren.
 *
 * These functions create a stack (implemented as an array) of objects that
 * are waiting to be scanned so that their children can be marked.
 *
 * The two main functions are putDeferredObject() and getDeferredObject()
 * which add and remove objects from the stack.  putDeferredObject()
 * correctly handles overflow.  getDeferredObject() presumes that the user
 * has first checked that there is an object to be gotten.
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      initializeDeferredObjectTable
 * TYPE:          scanning the heap
 * OVERVIEW:      Initialize the deferred object table so that it correctly
 *                behaves like a stack.
 * INTERFACE:
 *   parameters:  <none>
 *=======================================================================*/
void initializeDeferredObjectTable(void)
{
	deferredObjectCount = 0;
	deferredObjectTableOverflow = false;
}/*end initializedDeferredObjectTable*/

/*=========================================================================
 * FUNCTION:      putDeferredObject
 * OVERVIEW:      Add an object to the top of the stack.  If the stack is
 *                full, just set the variable deferredObjectTableOverflow
 *                and ignore the request.
 * INTERFACE:
 *   parameters:  value: An object to put on the stack
 *=======================================================================*/
static void putDeferredObject(u16 value)
{
	if (deferredObjectCount >= defferredObjectTableSize)
	{
		deferredObjectTableOverflow = true;
	}/*end if*/
	else
	{
		PX_W_U16(PX8_A_U16(deferredObjectTable, U8_M_U8(deferredObjectCount, sizeof(jref))), value);
		U8_INC(deferredObjectCount);
	}/*end else*/
}/*end putDeferredObject*/

/*=========================================================================
 * FUNCTION:      getDeferredObject
 * TYPE:          scanning the heap
 * OVERVIEW:      Get the next object from the top of the stack.
 *                The user should check the value of deferredObjectCount
 *                before calling this function.
 * INTERFACE:
 *   parameters:  none
 *   returns:     The top object on the stack.
 *=======================================================================*/
static u16 getDeferredObject(void)
{
	U8_DEC(deferredObjectCount);
	return PX_R_U16(PX8_A_U16(MEMtPF8(PXtMEM(deferredObjectTable)), U8_M_U8(deferredObjectCount, sizeof(jref))));
}/*end getDeferredObject*/

/*================================================================================
 * FUNCTION:      sweep
 * OVERVIEW:      Last part of garbage collection. Collects all the unmarked
 *                objects because marked objects are the only ones that are
 *                reachable from the roots.
 * INTERFACE:
 *   parameters:  None
 *   returns:     None
 *=================================================================================*/
/*
 Last part of garbage collection. Collects all the unmarked objects because marked
 objects are the only ones that are reachable from the roots.
 */
void sweep()
{
	jref		D_A		entry_index;
	memref 		D_A		entry_addr;
	u16			D_A		scanStart;
	u16			D_A		TotalObjectCount;

	u8			D_A		obType;
	u16			D_A		context;
#if GP__CGM__FOR_MASK
	u32			D_A		isChangeCC;
	u32			D_A		objSize;
	u16 				backChangeCC;
	u8 					backbCheckFlag;
	
	isChangeCC			= 0;
#endif

	TotalObjectCount	= MapTable_GetTopIndex();
	
	scanStart	= JcvmConfig_GetJcreEntryEnd();

	for (entry_index = scanStart; entry_index <= TotalObjectCount; U16_INC(entry_index))  //end condition: ran out of objects
	{
		if(!MapTable_IsCreationObject(entry_index))
		{
			continue;
		}
		else
		{
			if (!isMarked(entry_index))
			{
				if (locateObjectInMemory(entry_index))
				entry_addr = locateObjectInMemory(entry_index);
				if ((entry_addr != REF_NULL) && (entry_addr != MEM_NULL))
				{
					/*delete the object*/
					//removeObject(entry_index);
					context = get_object_owner(entry_addr);
#if GP__CGM__FOR_MASK
					if(context != 0)
					{
						if(
							((deletingPackage) && (GET_PKGID(context) != packageDeletedcontext ))
							|| ((appletDeletion) && (appletDeletionContexts != context ))
							)
						{
							//obType = nvmReadU8(MEMtPN8(entry_addr));
//							CGM_NVMObjResize(entry_index,context);
							objSize		= 0;
							CgmManager_getSizeByObjId(entry_index, &objSize,MEM_NULL,  CGM_GET_NAM);
							backChangeCC = g_backupCC;
							backbCheckFlag = g_bCheckFlag;
							g_bCheckFlag = CHECK_GC;
							CgmRun_ChangeCC(context);
							CgmManager_Free(objSize, HAS83_MASK);
							g_bCheckFlag = backbCheckFlag;
							isChangeCC	= 1;
						}
					}
#endif
					NvmManager_Free(entry_index, MAP_ENTRY_OBJECT_PCK_ID);
					//context = get_object_owner(entry_addr);
					obType = nvmReadU8(MEMtPN8(entry_addr));
					if (U8_AND_U8(obType, HEADER_JCRE_TransObj_IsTrans) == HEADER_JCRE_TransObj_IsTrans)
					{
						if (U8_AND_U8(obType, HEADER_JCRE_TransMode_Deselect) == HEADER_JCRE_TransMode_Deselect)
						{
							tranMananger_setTransientObjectDeleted(context);
						}
						else
						{
							tranMananger_setTransientObjectDeleted(0);
#if GP__CGM__FOR_MASK
							objSize		= 0;
							CgmManager_getSizeByObjId(entry_index, MEM_NULL, &objSize, CGM_GET_RAM);
							if(isChangeCC)
							{
								g_bCheckFlag = CHECK_GC;
							}
							CgmManager_Free(U32t16(objSize), HAS82_MASK);
#endif
						}
					}
#if GP__CGM__FOR_MASK					
					if(isChangeCC)
					{
						g_bCheckFlag = CHECK_GC;
						CgmRun_ChangeCC(backChangeCC);
						g_bCheckFlag = backbCheckFlag;
						isChangeCC = 0;
					}
#endif					
				}/*end if*/
			}/*end if*/
		}/*end if*/
	}/*end */
	
	resetGarbageCollector();
	NvmManager_Commit();
}/*end sweep*/

void setGCRequestedFlag(u8 flg)    SMALL_MODE
{
	//bRamBuffer[GC_Requested] = flg;
	nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetVmGcRequestStateBase()), flg);
}

