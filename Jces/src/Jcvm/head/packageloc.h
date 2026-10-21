/*****************************************************************************
 * @(#)packageloc.h	1.14 03/09/24
 * Version 1.9

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify

 ****************************************************************************/

#ifndef   __PACKAGE_LOC_H__
#define   __PACKAGE_LOC_H__

#include "profile.h"
#include "memory.h"

#include "ecpReadWrite.h"
#include "ecpOperation.h"

#define COMPONENT_CLASS         		0x00
#define COMPONENT_METHOD        		0x01
#define COMPONENT_STATICFIELD   		0x02
#define COMPONENT_EXPORT        		0x03

#define PACKAGE_LOC_TABLE_ENTRY_SIZE    32
#define PKGLOC_COMPONENT_COUNT          4

extern void 	removePackage(u8 pkgID)												SMALL_MODE;
//void addPackageLocation(u8 pkgID, u8 compID, memref location, u16 size);
extern  memref 	getPackageComponentLocation(u8 pkgID, u8 compID);
extern  u8 		getPackageIdForAddr(PF8 addr)  SMALL_MODE;
extern  memref 	getClassComponentBaseAddr(PF8 addr);
extern  memref 	getmethodComponentBaseAddr_MapIndex(u8 pkgID, u8 compID);

#endif /*  __PACKAGE_LOC_H__ */
