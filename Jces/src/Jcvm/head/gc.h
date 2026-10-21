/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*
 * %W% %E%
 */


#include "ob_manager.h"
#include "object.h"
#include "mask.h"

/*
 We're interested in the following types:
 1. Instance 0x20
 2. Primitive array values: 0x60, 0x80, 0xA0, 0xC0
 3. Reference type array value: 0xE0
 4. Primitive transient array values: 0x61, 0x81, 0xA1, 0xC1
 5. Reference type transient array value:0xE1

 In other words we're interested in the following bits
 x x x _  _ _ _ x  or 0xE1
 */
#define TYPEMASK 0xE1
#define TYPE(n) U8_AND_U8(n, TYPEMASK)

/*Enumeration of object types*/
enum
{
	GC_SIMPLE_INSTANCE = 0x20, GC_BOOLEAN_ARRAY = 0x60, GC_BYTE_ARRAY = 0x80, GC_SHORT_ARRAY = 0xA0, GC_INT_ARRAY = 0xC0, GC_REFERENCE_ARRAY = 0xE0, GC_TRANSIENT_BOOLEAN_ARRAY = 0x61, GC_TRANSIENT_BYTE_ARRAY = 0x81, GC_TRANSIENT_SHORT_ARRAY = 0xA1, GC_TRANSIENT_INT_ARRAY = 0xC1, GC_TRANSIENT_REFERENCE_ARRAY = 0xE1
};

/*Initializes the garbage collection process*/
extern u8 initializeGC(void);
extern u8 markNonRootObjects(u16 reset_reference_null);
extern void sweep(void);
//void resetGarbageCollector(void);
/*Initializes the applet deletion process*/
extern void initAppletDeletion(u16 contexts);
extern void initPackageDeletion(u8 pkgID);

/*Marks the objects pointed to by static fields as roots*/
extern u8 markStaticRoots(u8 pkgID, u16 refCount, u8 reset_reference_null)			SMALL_MODE;
extern u8 markUpgradeOnSaveObjects(u8 reset_reference_null); //merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP to support GP__AMENDMENT__H__FOR_MASK
extern u8 markObject(u16 objectId, u8 reset_reference_null, u32 referenceAddress);
extern u8 markChildren(u16 objectId, u16 limit, s32 remainingDepth, u8 reset_reference_null);
/*Marks the applet table as the root of all applets*/
extern u8 markAppletRoot(jref objectId)					SMALL_MODE;
extern u8 isMarked(u16 objectId);
/*Starts the garbage collection process*/
extern u8 doGC(void);
extern void resetGarbageCollector(void);
extern void setGCRequestedFlag(u8 flg)                      SMALL_MODE;

extern  void initializeDeferredObjectTable(void);

