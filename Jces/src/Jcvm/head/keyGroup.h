/*
 * keyGroup.h
 *
 *  Created on: 2013年10月8日
 *      Author: mowenpan
 */

#include "profile.h"

#ifndef KEYGROUP_H_
#define KEYGROUP_H_

#include "ecpDefine.h"



//#define KEY_GROUP_TYPE_CONTENT                             0xFF merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 用SCP_FF代替，更具有可读性

#define KEY_GROUP_PCK_ID							       EASTCOMPEACE_OPENPLATFORM_ID
#define newKeyGroup()								       newObject(KEY_GROUP_CLASS_OFFSET,KEY_GROUP_PCK_ID, BIT_NEED_REFRESH)

#define keyGroup_getNextKeyGroupHand(keyGroupObj)          get_object_field_ref(keyGroupObj, nextKeyGroup_INDEX)
#define keyGroup_getNextKeyGroupRef(keyGroupObj)           nvmReadU16(keyGroup_getNextKeyGroupHand(keyGroupObj))

#define keyGroup_getScpTypeField(keyGroupObj)			   get_object_field_ref(keyGroupObj, ScpType_INDEX)
#define keyGroup_getScpType(keyGroupObj)        		   nvmReadU8(MEMtPN8(keyGroup_getScpTypeField(keyGroupObj)))
#define keyGroup_setScpType(keyGroupObj, scpType)          nvmWriteU8NoAtomic(keyGroup_getScpTypeField(keyGroupObj), scpType)

#define keyGroup_getKeySetField(keyGroupObj)               get_object_field_ref(keyGroupObj, keySet_INDEX)
#define keyGroup_getKeySetRef(keyGroupObj)                 nvmReadU16(MEMtPN8(keyGroup_getKeySetField(keyGroupObj)))

#if GP__KEY_MANAGER

	u8 keyGroup_getScpTypeByKeyVersion(PN8 sdDataAddr, u8 keyVersion);
	jref keyGroup_getKeySet(PN8 sdDataObj, u8 scpType, u8 keyVersion);

#else
	#define keyGroup_getScpTypeByKeyVersion(sdDataAddr, keyVersion)						0
	#define keyGroup_getKeySet(sdDataObj, scpType, keyVersion)							0
#endif

#endif /* KEYGROUP_H_ */
