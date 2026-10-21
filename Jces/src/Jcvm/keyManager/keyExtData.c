/*
 * keyExtData.c
 *
 *  Created on: 2013年10月8日
 *      Author: mowenpan
 */
#include "typejava.h"
#include "object.h"
#include "ob_manager.h"
#include "mask.h"
#include "KeyExtData.h"

#if GP__KEY_MANAGER

#define getKeyExtDataHand(keySetObj)          get_object_field_ref(keySetObj, KeyExtraData_INDEX)
#define getNextKeyExtDataHand(extKeyData)     get_object_field_ref(MEMtPF8(locateObjectInMemory(extKeyData)), nextKeyExtraData_INDEX)

#define getKeyDataHand(keyExtDataRef)         get_object_field_ref(MEMtPF8(locateObjectInMemory(keyExtDataRef)), keyData_INDEX)

static memref getEmptyKeyExtDataHand(PN8 keySetObj)
{
	PN8 extKeyDataHand;
	jref extKeyData;

	extKeyDataHand = MEMtPN8(getKeyExtDataHand(keySetObj));

	while (true)
	{
		extKeyData = memReadU16(extKeyDataHand);

		if (REF_NULL == extKeyData)
		{
			break;
		}
		else
		{
			extKeyDataHand = MEMtPN8(getNextKeyExtDataHand(extKeyData));
		}
	}

	return PNtMEM(extKeyDataHand);
}
/*
 * modified by xuxiaojun@20210414 对于类型确定的数组，_newarrayObject紧接着给数组内容写值时，避免调用get_array_data，可以减少上面一次创建数组时cache的提交
 */
void keyExtData_addKeyDataSpace(PN8 keySetObj, u16 keyLen)
{
	PN8 keyExtDataHand;
	PN8 keyDataHand;
	jref keyExtData;
	jref keyData;

	keyExtDataHand = MEMtPN8(getEmptyKeyExtDataHand(keySetObj));

	keyExtData = newKeyExtraData();
	nvmWriteU16Atomic(keyExtDataHand, keyExtData);

	keyDataHand = MEMtPN8(getKeyDataHand(keyExtData));

	keyData = _newarrayObject(AType_Byte, keyLen, BIT_NEED_NOTHING);
	nvmWriteU16Atomic(keyDataHand, keyData);

	memRefresh();
}
#endif

