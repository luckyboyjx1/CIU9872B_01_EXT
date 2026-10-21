/*****************************************************************************
 (#)grt.c	1.24 03/09/26
 * Version 1.9

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify

 ****************************************************************************/	
#include "memory.h"
#include "grt.h"
#include "MemBackUp.h"
#include "object.h"
#include "global_variable.h"

#include "ecpReadWrite.h"
#include "ecpOperation.h" 
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

//Following variables exists for optimization
/******************************************************************************
 * the optimization is that during call to getReferenceAddress(), we remember
 * the key whose value is needed and we also remember the GenericRow the Key points to.
 * Then during a call to getReferencePackageID(), if the key is the same as the
 * one we remembered, we don't search for the GenericRow again, we just use the
 * cached GEnericRow and just read the packageID off it. This optimization
 * works because of the observation that very often we call getReferenceAddress()
 * followed by getReferencePackageID for the same key.
 ******************************************************************************/

/******************************************************************************
 * Return the element's 32 or 16-bit address whose reference
 * is given as parameter.
 * Assumed that jref is same type as Key.
 * Assumed that memref is same value as Value.
 * @param referenceID the ref id
 * @return the actual address
 * last modified by wh 2010.2.23, used cache , maybe faster
 ******************************************************************************/
memref getReferenceAddress(jref referenceID)
{
	tempReferenceIDGRT		= referenceID;	
	referenceID				= U16_AND_U16(referenceID, 0x7FFF);
	
	lastVMPkg				= MapTable_GetEntryPckId(referenceID);

	return MapTable_GetEntryAddr(referenceID);
}

/******************************************************************************
 * function has the optimization to return cached package id if
 * reference is the same as last time.
 * @return the packageID given the referenceID
 * @param referenceID the reference ID
 * last modified by wh 2010.2.23, used cache , maybe faster
 ******************************************************************************/
u8 getReferencePackageID(jref referenceID)
{
	if (referenceID != tempReferenceIDGRT)
	{
		tempReferenceIDGRT	= referenceID;
		referenceID			= U16_AND_U16(referenceID, 0x7FFF);	
		lastVMPkg			= MapTable_GetEntryPckId(referenceID);		
	}	
	return lastVMPkg;
}

/******************************************************************************
 * function adds an address to grt with a package id and
 * returns the jref to it.
 * Note: In this function jref should be same type as Key
 *  and memref the same type as Value(see TableMgmntTypes.h).
 * Assumed that memref is 2 bytes. If more, implementation needs
 *  to change.
 * @param address the memref to map
 * @param packageID the package id of this address
 * @param jref key to the address parameter.
 ******************************************************************************/
jref addReference(PN8 address, u8 packageID)		SMALL_MODE
{	
	u16		D_A		index;

	index					= MapTable_AddEntry(address, packageID);
	
	if(REF_NULL != index)
	{
		index	= U16_OR_U16(index , 0x8000);
	}

	return index;
}


/******************************************************************************
 * function removes all references mapped in the TableMgmnt
 * that have the packageID specified
 * @param packageID the package id criteria
 ******************************************************************************/
void removeReferencesFromPackage(u8 packageID)		SMALL_MODE
{
	u16		D_A	index;
	u16		D_A	totalCounter;
	u8		D_A	grtTransactionStarted;

	grtTransactionStarted = 0;

	totalCounter = MapTable_GetTopIndex();

		//start transaction if not started
	if (!isInProgress())
	{
		begin_transaction(SINGLE_APDU_PROCESS);
		grtTransactionStarted = 1;
	}/*end if*/

	for(index = MASK_MAP_ENTRY_COUNT; index < totalCounter; U16_INC(index))
	{
		if(MapTable_GetEntryPckId(index) == packageID)
		{
			if(MapTable_IsCurrentProfile(index, G_CurExecutingProfileIndex))
			{
				//clean_map_entry(index);
				NvmManager_Free(index, packageID);
			}
		}
	}
	
	if (grtTransactionStarted)
	{
		commit_transaction();
	}/*end if*/
}
