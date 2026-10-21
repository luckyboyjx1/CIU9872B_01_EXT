/*
 * KeyExtData.h
 *
 *  Created on: 2013Äê9ÔÂ29ÈÕ
 *      Author: mowenpan
 */

#ifndef KEYEXTDATA_H_
#define KEYEXTDATA_H_

#include "bc_object.h"

#if GP__KEY_MANAGER

#define KEY_EXTRA_DATA_PCK_ID						   EASTCOMPEACE_OPENPLATFORM_ID
#define newKeyExtraData()							   newObject(KEY_EXTRA_DATA_CLASS_OFFSET,KEY_EXTRA_DATA_PCK_ID,BIT_NEED_REFRESH)

#define keyExtData_getKeyDataObj(keyExtDataObj)        locateObjectInMemory(memReadU16(get_object_field_ref(keyExtDataObj, keyData_INDEX)))

#define keyExtData_getNextKeyExtDataField(keyExtDataObj) get_object_field_ref(keyExtDataObj, nextKeyExtraData_INDEX)
#define keyExtData_getNextKeyExtDataRef(keyExtDataObj)   memReadU16(keyExtData_getNextKeyExtDataField(keyExtDataObj))

void keyExtData_addKeyDataSpace(PN8 keySetObj, u16 keyLen);
#else
	#define keyExtData_addKeyDataSpace(keySetObj, keyLen)
#endif

#endif /* KEYEXTDATA_H_ */
