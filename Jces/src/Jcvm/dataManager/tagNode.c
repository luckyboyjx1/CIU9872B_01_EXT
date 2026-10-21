/*
 * tagNode.c
 *
 *  Created on: 2013-10-8
 *      Author: linguoming1
 */
#include "object.h"
#include "tagNode.h"
#include "profile.h"
#include "bc_array.h"
#include "ob_manager.h"

#if GP__DATA_MANAGER__FOR_MASK
/*
 * modify by xuxiaojun@20220527 当temRef为有效值时，dataTag, tagAtrri, len都是有效值，如果此时把len修改为0，后续没有位置再写长度进去了 SCP11c_1: C_sm_STORE_DATA_Whitelist_SUCCESS_KID20h_256_a1_60_10
 */
static jref tagNode_creatTagArray(u16 dataTag, u16 tagAttri, u16 tagMostLen, jref temRef)
{
	jref		tagArrayRef;
	PN8	tagArrayAField;

	if(MEM_NULL == temRef)
	{
		tagArrayRef		= _newarrayObject(AType_Byte, U16_A_U16(tagMostLen, TAG_ARRAY_HEADER_SIZE), BIT_NEED_REFRESH);

		tagArrayAField	= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(tagArrayRef))));

		tagNode_setTagArrayTag(tagArrayAField, dataTag);
		tagNode_setTagArrayAttribute(tagArrayAField, tagAttri);
		tagNode_setTagArrayLength(tagArrayAField, 0x00);
	}
	else
	{
		tagArrayRef		= temRef;
	}

	return tagArrayRef;
}

jref tagNode_getTagArray(jref firstTagNodeRef, u16 dataTag, u16 tagMostLen, u16 tagAttri, jref tempRef, u8 isAppendNode)
{
	jref				curNodeRef;
	jref				newNodeRef;
	jref				curTagArrayRef;
	jref				newTagArrayRef;
	PN8					tagArrayAField;
	PN8					curTagNodeObj;
	PN8					tempTagNodeObj;
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	u16					tagArrayMostLength;
	PN8					tagArrayObj;
#endif	
	newTagArrayRef		= MEM_NULL;
	curNodeRef			= firstTagNodeRef;

	do
	{
		curTagNodeObj				= MEMtPN8(locateObjectInMemory(curNodeRef));
		curNodeRef					= tagNode_getNextTagNodeRef(curTagNodeObj);
		curTagArrayRef				= tagNode_getTagArrayRef(curTagNodeObj);

		if(MEM_NULL != curTagArrayRef)
		{
			tagArrayAField			= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(curTagArrayRef))));
			if(dataTag == tagNode_getTagArrayTag(tagArrayAField))
			{
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
				tagArrayObj			= MEMtPN8(locateObjectInMemory(curTagArrayRef));
				tagArrayMostLength	= get_array_length(tagArrayObj);
				tagArrayMostLength	= U16_S_U8(tagArrayMostLength, TAG_ARRAY_HEADER_SIZE);
				if(tagMostLen <= tagArrayMostLength)
				{
#endif
					if(IS_0070_GROUP_TAG == U16_AND_U16(IS_0070_GROUP_TAG , tagAttri))
					{
						if(IS_0070_GROUP_TAG == tagNode_getTagArrayAttribute(tagArrayAField))
						{
							return curTagArrayRef;
						}
						
					}
					else
					{
						return curTagArrayRef;
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
					}
				}
				else
				{
					curTagArrayRef = MEM_NULL;
					break;
#endif
				}
			}
		}

	}while(MEM_NULL != curNodeRef);

	if(isAppendNode)
	{
		newTagArrayRef				= tagNode_creatTagArray(dataTag, tagAttri, tagMostLen, tempRef);
		if(MEM_NULL == curTagArrayRef)
		{
			//reuse the current tag node
			tempTagNodeObj		= curTagNodeObj;
		}
		else
		{
			//create a new tag node
			newNodeRef				= newTagNode();
			tempTagNodeObj		= MEMtPN8(locateObjectInMemory(newNodeRef));
			tagNode_setNextTagNodeRef(curTagNodeObj, newNodeRef);
		}
		tagNode_setTagArrayRef(tempTagNodeObj, newTagArrayRef);
	}
	memRefresh();
	return newTagArrayRef;
}
#endif
