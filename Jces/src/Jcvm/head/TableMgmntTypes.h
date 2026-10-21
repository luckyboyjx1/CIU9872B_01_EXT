/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

#ifndef __TABLEMGMNT_TYPES_H
#define __TABLEMGMNT_TYPES_H
#include "typejava.h"

/*Total realizations of the TableMgmnt scheme*/
#define TOTAL_NUM_TABLE_IMPL 				3

/* All definitions below are our own. Above is copied from cref*/
#define MEMREF_NULL 						((memref)0)

typedef u8 TableMgmntId;
#define INVALID_TABLE_ID 					((u8)0xFF)

/* This field represents a type passed around as literal values. It is used
 * as argument type and return type for generic functions not concerning with
 * refs keys or values.
 * Note: The type and length of GenericValue MUST be the highest amongst key,ref,val.
 **/
typedef memref GenericValue;
typedef jref Key;
typedef memref Value;

/**
 * Availability byte is byte at the end of all intermediate rows indicating
 * the index into the table pointed to by the corresponding row from which we
 * can get to the first hole in the subtree rooted at that row.
 * Note: we reserve the maximum value - 'FF' to denote that the subtree
 * rooted at row does not have any holes.
 * Note: This implies that we cannot have tables more than 254 row tables. Or else
 * the index would rollover and start from 0 again which would be wrong.
 **/
typedef u8 AvailabilityByte;

/**
 * below define used to denote availability byte state indicating no holes
 * available.
 **/
#define NO_HOLES_AVAIL_VALUE 0xff

/**
 * following Key is an invalid key. TableMgmnt instance cannot store a value
 * for this key because it is reserved to denote table Full
 **/
#define INVALID_KEY 						((Key)0xFFFF)

#define NUM_BYTES_IN_MEMREF 				4
#define NUM_BYTES_IN_VALUE 					4
#define NUM_BYTES_IN_GENERIC_VALUE 			4
#define NUM_BYTES_IN_KEY 					2

/**
 * We define GenericRow as a pointer. For each TableMgmnt instance, we offset
 * into this row as per the meta data to get the different column values.
 **/
typedef memref GenericRow;
#define GENERICROW_NULL 					((GenericRow)0)
#define GENERICVALUE_NULL 					((GenericValue)0)
#endif
