/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*
 * %W% %E%
 */

#ifndef   __GRT_H__
#define   __GRT_H__

#include "typejava.h"
#define REMOVE_ALL_PACKAGE_ENTRIES 255
#define ONE_TRANSACTION_REMOVAL_NUMBER 10


/**
 * Return the element's 32-bit address whose reference
 * is given as parameter.
 * Assumed that jref is same type as Key.
 * Assumed that memref is same value as Value.
 * @param referenceID the ref id
 * @return the actual address
 **/
extern memref getReferenceAddress(jref referenceID);

/**
 * @return the packageID given the referenceID
 * @param referenceID the reference ID
 **/
extern u8 getReferencePackageID(jref referenceID);

/**
 * function adds an address to grt with a package id and
 * returns the jref to it.
 * Note: In this function jref should be same type as Key
 *  and memref the same type as Value.
 * Assumed that memref is 2 bytes. If more, implementation needs
 *  to change.
 * @param address the memref to map
 * @param packageID the package id of this address
 * @param jref key to the address parameter.
 **/
extern jref addReference(PN8 address, u8 packageID)			SMALL_MODE;

/**
 * function removes all references mapped in the TableMgmnt
 * that have the packageID specified
 * @param packageID the package id criteria
 **/
extern void removeReferencesFromPackage(u8 packageID)			SMALL_MODE;

/**
 * function returns the jref key to the memref address value. If value does not
 * exist in the grt, value FFFF is returned.
 * @param address the address whose corresponding jref is returnes
 * @return jref the key whose value is the address parameter
 **/
extern jref getReferenceGivenAddress(PN8 address);

#endif /* __GRT_H__  */
