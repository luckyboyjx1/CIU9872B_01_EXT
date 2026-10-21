/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*
 * @(#)ob_manager.h	1.21 03/08/13
 */

#ifndef   __OBJECTMANAGER_H__
#define   __OBJECTMANAGER_H__
#include "memory.h"
#include "error.h"
#include "anomaly.h"
#include "bc_array.h"
#include "profile.h"

/*Table Types*/
#define OBJECT_TABLE 					(u8)0x00
#define TRANSIENT_TABLE 				(u8)0x01
#define BOTH_TABLES 					(u8)0x02

#define INVALID_CONTEXT					U32t16(0xFF00)
#define INVALID_ID 						U32t16(0xFFFF)

#define NULL_POINTER 					0
/*index values for transients. Indexes for transients start at 0xFC00*/
#define TRANSIENT_MASK 0xFC00
#define OBJECT_MASK 0

//--below are function declerations

/* High- speed object cache */
//#define OBJ_CACHE_BITS  				2
//#define getCacheEntry(objRef) 			(((1 << OBJ_CACHE_BITS) - 1) & objRef)
/*
typedef struct t_obj_cache
{
	jref objID;
	memref objAddr;
}X_A obj_cache[(1 << OBJ_CACHE_BITS)];
*/


#define FREE_OBJ_CACHE_SIZE 			8

typedef struct
{
	/*fix bug:B100909-001
	 u16 objSize;*/
	memref objAddr;
	u32 objSize;

} FREEOBJCACHE;

extern FREEOBJCACHE 					freeObjCache[FREE_OBJ_CACHE_SIZE];
#define initralizeFreeObjCache() 		ecpMem_Set(MEMtPX8(&freeObjCache[0]),0,sizeof(freeObjCache))

/*called during card initialization to create the appropriate tables in the memory*/
//void initializeObjCache();



#define clearFreeObjCache_P60()         initralizeFreeObjCache() 

extern  void manageObjCache(u8 i);
extern  boolean addFreeObjectToCache(PN8 addr, u32 size)							SMALL_MODE;
extern  u8 commitObjCacheToFreeTable(void);


/*called during card initialization to create the appropriate tables in the memory*/


/*returns the size of the selected table or of both combined if so desired  */
extern  u16 getTableSize(u8 tableType);

/*adds an object to the object table, returns 0 if failed*/
//jref addObject(memref objAddress,u8 objType);

extern	u32	getObjTotalSize(PN8 objAddress);

extern  u8 removeObjectFromMemory(u16 objectId)										SMALL_MODE;
/*removes object from the object table. Object Id is the index of the object in the table*/
extern  u8 removeObject(jref objectId);

/*locates object in the memory*/
extern  memref locateObjectInMemory(jref objectId);

/*locates object in the object table*/
//memref locateObjectInTable(memref objAddr);

/*Gets the first object from the table selected*/
extern  memref getFirstObject(u8 tableType);

/*Assuming that getFirstObject has already been called, returns the next address in the
 selected table. If getFirstObject was not called before calling this function, the result
 of the call is unknown.*/
extern  memref getNextObject(void);

/*returns the object size*/
extern  u32 getObjectSize(PF8 objectAddr);

extern memref locateObjectInMemory(jref objectId);
#endif /* __OBJECTMANAGER_H__ */
