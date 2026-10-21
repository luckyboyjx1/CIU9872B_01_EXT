#include "utilMethod.h"
#include "ob_manager.h"
#include "object.h"
#include "firewall.h"
#include "packageloc.h"
#include "context.h"

/*****************************************************************************
 * File:  utilMethod.c
 * Version 1.13

 wang hui modify
 eastcompeace Technology Corporation

 History:
 2007,11,14	.created
 Original author: wang hui
 *****************************************************************************/
void check_bounds_interval(PF8 array_ref, u16 index, u16 length)
{
	u8	D_A  objectType;

#if JCVM__VM__FIRE_WALL
	checkArrayRead(array_ref);
#endif

	objectType = U8_AND_U8(get_object_header_type(array_ref), OBJ_HDR_OBJECT_TYPE_MASK);
	
	if(Header_ArrayType_Byte != objectType)
	{
		halt_session(BAD_ARRAY_TYPE_ERROR);	
	}
	
	if ((0x8000 == U16_AND_U16(0x8000, index)) /*fix bug:B100930-001|| (index >= (jshort)get_array_length(array_ref)) */
		|| (0x8000 == U16_AND_U16(0x8000, length) )				  //wzq
		|| (U16_A_U16(index, length) > get_array_length(array_ref)))
	{
		throw_error(ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION);
	}
}
/*
 * function name: check_arraybounds_interval
 * function：             检查数组是否越界
 * param:
 * PF8 array_ref：  数组对象索引
 * u16 off：                  在数组中的偏移
 * u16 length：          要取的长度
 */
void check_arraybounds_interval(PF8 array_ref, u16 off, u16 length)
{
	if((off < 0) || (length < 0) || ((off + length) > get_array_length(array_ref)))
	{
		 throw_error(ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION);
	}
}

