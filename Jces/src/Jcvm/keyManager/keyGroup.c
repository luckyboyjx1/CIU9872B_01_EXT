/*
 * keyGroup.c
 *
 *  Created on: 2013年10月8日
 *      Author: mowenpan
 */

#include "keyGroup.h"
#include "SecurityDomainData.h"
#include "keySet.h"
#include "keyLogicManager.h"
#include "opFunction.h"

#if GP__KEY_MANAGER
/*
 * merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 修改函数名称使函数功能更明显
 */
u8 keyGroup_getScpTypeByKeyVersion(PN8 sdDataAddr, u8 keyVersion)
{
	u8 scpType;

	if (keySet_isCMKeyVersion(keyVersion))
	{
		scpType = SCP_FF;
	}
	else
	{
		//scpType = sdData_getScpType(sdDataAddr);
		 scpType  =	getSCPTypeByKeyVersion(keyVersion);
	}

	return scpType;
}

jref keyGroup_getKeySet(PN8 sdDataObj, u8 scpType, u8 keyVersion)
{
		jref keyGroupRef;
		PN8 keySetField;

		keyGroupRef = sdData_getKeyGroupRef(sdDataObj, scpType, FALSE);
		if (REF_NULL == keyGroupRef)
		{
			return REF_NULL;
		}

		keySetField = MEMtPN8(keyGroup_getKeySetField(MEMtPF8(locateObjectInMemory(keyGroupRef))));
		return memReadU16(MEMtPM8(keySet_getKeySetHand(keySetField, keyVersion)));
}
#endif
