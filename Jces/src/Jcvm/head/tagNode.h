/*
 * tagNode.h
 *
 *  Created on: 2013-10-8
 *      Author: linguoming1
 */

#include "bc_object.h"
#include "mask.h"
#include "object.h"
#include "ecpDefine.h"

#ifndef TAGNODE_H_
#define TAGNODE_H_

#define IS_0070_GROUP_TAG										0x0002

#define TAG_OFFSET												0
#define TAG_ATTRIBUTE_OFFSET									2
#define TAG_LENGTH_OFFSET										4
#define TAG_VALUE_OFFSET										6
#define TAG_ARRAY_HEADER_SIZE									6

#define tagNode_getTagArrayField(tagNodeObj)					get_object_field_ref(tagNodeObj, tagArray_INDEX)
#define tagNode_getTagArrayRef(tagNodeObj)						PN_R_U16(tagNode_getTagArrayField(tagNodeObj))
#define tagNode_setTagArrayRef(tagNodeObj, ref)					memWriteU16Atomic(tagNode_getTagArrayField(tagNodeObj), ref)

#define tagNode_getNextTagNodeField(tagNodeObj)					get_object_field_ref(tagNodeObj,nextTagNode_INDEX)
#define tagNode_getNextTagNodeRef(tagNodeObj)					PN_R_U16(tagNode_getNextTagNodeField(tagNodeObj))
#define tagNode_setNextTagNodeRef(tagNodeObj, ref)				memWriteU16Atomic(tagNode_getNextTagNodeField(tagNodeObj), ref)

#define tagNode_getTagArrayTag(tagArrayAField)					PN_R_U16(PN8_A_U32(tagArrayAField,TAG_OFFSET))
#define	tagNode_getTagArrayAttribute(tagArrayAField)			PN_R_U16(PN8_A_U32(tagArrayAField,TAG_ATTRIBUTE_OFFSET))
#define tagNode_getTagArrayLength(tagArrayAField)				PN_R_U16(PN8_A_U32(tagArrayAField,TAG_LENGTH_OFFSET))
#define tagNode_getTagArrayContent(tagArrayAField)				PN8_A_U32(tagArrayAField,TAG_VALUE_OFFSET)
#define	tagNode_setTagArrayTag(tagArrayAField, tag)				nvmWriteU16Atomic(PN8_A_U32(tagArrayAField,TAG_OFFSET), tag)
#define	tagNode_setTagArrayAttribute(tagArrayAField, att)		nvmWriteU16Atomic(PN8_A_U32(tagArrayAField,TAG_ATTRIBUTE_OFFSET), att)
#define	tagNode_setTagArrayLength(tagArrayAField, len)		    nvmWriteU16Atomic(PN8_A_U32(tagArrayAField,TAG_LENGTH_OFFSET), len)

#define TAG_NODE_PCK_ID											EASTCOMPEACE_OPENPLATFORM_ID

#if GP__DATA_MANAGER__FOR_MASK
	#define newTagNode()										newObject(TAG_NODE_CLASS_OFFSET,TAG_NODE_PCK_ID, BIT_NEED_REFRESH);
	extern jref tagNode_getTagArray(jref firstTagNode, u16 dataTag, u16 tagLen, u16 tagAttri, jref temRef, u8 isAppendNode);// merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 增加类型extern 
#else
	#define newTagNode()																			REF_NULL
	#define tagNode_getTagArray(firstTagNode, dataTag, tagLen, tagAttri, temRef, isAppendNode)		REF_NULL
#endif

#endif /* TAGNODE_H_ */
