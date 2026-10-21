/*****************************************************************************
 * File:  bc_field.c
 * Version 1.9

* wang hui
* eastcompeace Technology Corporation

*History:
* 2007,7,3	.modify
* Original author: David Getreu
 *****************************************************************************/
#include "bc_field.h"
#include "global_variable.h"
#include "context.h"
#include "stack.h"
#include "object.h"
#include "firewall.h"
#include "memory.h"
#include "anomaly.h"
#include "mask.h"
#include "ob_manager.h"
/*****************************************************************************
 * This routine implements the common portion of the Java Card 2.1
 * putfield_x_xxx & getfield_x_xxx instructions.
 *****************************************************************************/

//static void _putgetfield_common(u8 index, PN8 objaddr)      SMALL_MODE
//{
//	
//	if (index > get_object_size(objaddr))
//	{
//		fatal_error(FIELD_INDEX_OUT_OF_OBJECT_ERROR);
//	}
//}
PN8  _putgetfield_abs_common(u8 index,jref objref)
{
    PN8		D_A		objaddr;

	objaddr	= MEMtPN8(locateObjectInMemory(objref));
	
	if(PFtMEM(objaddr)!= REF_NULL) 
	{
		if (index <= get_object_size(objaddr))
		{
#if JCVM__VM__FIRE_WALL
			checkArrayRead(objaddr);
#endif
			objaddr = MEMtPN8(get_object_field_ref(objaddr, index));
			return objaddr;
		}
		else
		{
			fatal_error(FIELD_INDEX_OUT_OF_OBJECT_ERROR);
		}
	}
	else
	{
		throw_error(NULL_POINTER_EXCEPTION);
	}
}
/*****************************************************************************
 * This routine implements the common portion of the Java Card 2.1
 * getfield_a_xxx instructions.
 *****************************************************************************/
static void _getfield_a_common(u8 index, jref objref)		SMALL_MODE
{
	jref		D_A		value;
	PN8			D_A		objaddr;
	
   objaddr	= _putgetfield_abs_common(index,objref);

	value = nvmReadU16(objaddr);

	pushReference(value);
}

#if VM_INTS_SUPPORTED
/*****************************************************************************
 * This routine implements the common portion of the Java Card 2.1
 * getfield_a_xxx instructions.
 *****************************************************************************/
static void _getfield_i_common(u8 index, jref objref)		SMALL_MODE
{
	u32		D_A		value;
	PN8		D_A		objaddr;
	objaddr	= MEMtPN8(locateObjectInMemory(objref));
	
	if(PFtMEM(objaddr)!= REF_NULL) 
	{
		if ((index + 1)  <= get_object_size(objaddr))
		{
	#if JCVM__VM__FIRE_WALL
			checkArrayRead(objaddr);
	#endif
			objaddr = MEMtPN8(get_object_field_ref(objaddr, index));
			value	= nvmReadU32(objaddr);
			pushInt(value);
		}
		else
		{
			fatal_error(FIELD_INDEX_OUT_OF_OBJECT_ERROR);
		}
	}
	else
	{
		throw_error(NULL_POINTER_EXCEPTION);
	}
}


/*****************************************************************************
 * This routine implements the common portion of the Java Card 2.1
 * putfield_i_xxx instructions.
 *****************************************************************************/
static void _putfield_i_common(u8 index, u32 value, jref objref)		SMALL_MODE
{
	PN8		D_A		objaddr;

	objaddr	= MEMtPN8(locateObjectInMemory(objref));
	
	if(PFtMEM(objaddr)!= REF_NULL) 
	{
		if( (index + 1) <= get_object_size(objaddr))
		{
	#if JCVM__VM__FIRE_WALL
			checkArrayRead(objaddr);
	#endif
			objaddr = MEMtPN8(get_object_field_ref(objaddr, index));
			objaddr	= _putgetfield_abs_common(index,objref);
			nvmWriteU32Atomic(objaddr,value);
		}
		else
		{
			fatal_error(FIELD_INDEX_OUT_OF_OBJECT_ERROR);
		}
	}
	else
	{
		throw_error(NULL_POINTER_EXCEPTION);
	}
}

#endif //VM_INTS_SUPPORTED
/*****************************************************************************
 * This method implements the Java Card 2.1 getfield_a instruction.
 * Format:
 * getfield_a
 *
 * Stack Image:
 * ..., objref --> ..., value

 *****************************************************************************/
void _getfield_a()
{
	u8			D_A		index;
	jref		D_A		objref;

	index	= fetchByte();
	objref	= popReference();
	
	_getfield_a_common(index, objref);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 getfield_a_w instruction.
 * Format:
 * getfield_a_w
 *
 * Stack Image:
 * ..., objref --> ..., value

 *****************************************************************************/
void _getfield_a_w()
{
	u8			D_A		index;
	jref		D_A		objref;


	/*
	 IMPORTANT!  Assumption is that the on-card installer will
	 resolve all indices to be in the range of 0-255, since all
	 Class Instances (Objects) may not exceed 255, per the JCVM SPEC.
	 */
	index	= U16t8(fetchShort());
	objref	= popReference();
	
	_getfield_a_common(index, objref);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 getfield_a_this instruction.
 * Format:
 * getfield_a_this
 *
 * Stack Image:
 * ..., objref --> ..., value
 * last modified by wh 2010.2.23, direct to used FP must be faster 
 *****************************************************************************/
void _getfield_a_this()
{
	u8		D_A  index;
	jref	D_A  objref;

	index	= fetchByte();
	objref	= getLocalReference(0);

	_getfield_a_common(index, objref);
}

/*****************************************************************************
 * This routine implements the common portion of the Java Card 2.1
 * getfield_b_xxx instructions.
 *****************************************************************************/
static void _getfield_b_common(u8 index, jref objref)		SMALL_MODE
{
	u8		D_A		value;
	PN8		D_A		objaddr;
    objaddr	= _putgetfield_abs_common(index,objref);

	value	= nvmReadU8(objaddr);

		pushShort(U8tS16(value));

}

/*****************************************************************************
 * This method implements the Java Card 2.1 getfield_b instruction.
 * Format:
 * getfield_b
 *
 * Stack Image:
 * ..., objref --> ..., value

 *****************************************************************************/
void _getfield_b()
{
	u8		D_A  index;
	jref	D_A  objref;

	index	= fetchByte();
	objref	= popReference();

	_getfield_b_common(index, objref);
}

/****************************************************************************
 * This method implements the Java Card 2.1 getfield_b_w instruction.
 * Format:
 * getfield_b_w
 *
 * Stack Image:
 * ..., objref --> ..., value

 *****************************************************************************/
void _getfield_b_w()
{
	u8		D_A    index;
	jref	D_A    objref;
	/*
	 IMPORTANT!  Assumption is that the on-card installer will
	 resolve all indices to be in the range of 0-255, since all
	 Class Instances (Objects) may not exceed 255, per the JCVM SPEC.
	 */
	index	= U16t8(fetchShort());
	objref	= popReference();

	_getfield_b_common(index, objref);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 getfield_b_this instruction.
 * Format:
 * getfield_b_this
 *
 * Stack Image:
 * ..., objref --> ..., value

 *****************************************************************************/
void _getfield_b_this()
{
	u8		D_A    index;
	jref	D_A    objref;

	index	= fetchByte();
	objref	= getLocalReference(0);

	_getfield_b_common(index, objref);
}

/*****************************************************************************
 * This routine implements the common portion of the Java Card 2.1
 * getfield_s_xxx instructions.
 *****************************************************************************/
//static void _getfield_s_common(u8 index, jref objref)		SMALL_MODE
//{
//	u16		D_A		value;
//	PN8		D_A		objaddr;

//	objaddr	= MEMtPN8(locateObjectInMemory(objref));


//	check_null(objaddr);	
//	if (PFtMEM(objaddr)!= REF_NULL) 
//	{
//		_putgetfield_common(index, objaddr);

//#if JCVM__VM__FIRE_WALL
//		checkArrayRead(objaddr);
//#endif

//		value	= nvmReadU16(MEMtPN8(get_object_field_ref(objaddr, index)));

//		pushShort(value);
//		return;
//	}
//	throw_error(NULL_POINTER_EXCEPTION);
//}

/*****************************************************************************
 * This method implements the Java Card 2.1 getfield_s instruction.
 * Format:
 * getfield_s
 *
 * Stack Image:
 * ..., objref --> ..., value

 *****************************************************************************/
//void _getfield_s()
//{
//	u8		D_A    index;
//	jref	D_A    objref;

//	index	= fetchByte();
//	objref	= popReference();

//	_getfield_s_common(index, objref);
	
//}

/*****************************************************************************
 * This method implements the Java Card 2.1 getfield_s_w instruction.
 * Format:
 * getfield_s_w
 *
 * Stack Image:
 * ..., objref --> ..., value

 *****************************************************************************/
//void _getfield_s_w()
//{
//	u8		D_A    index;
//	jref	D_A    objref;

//	index	= U16t8(fetchShort());
//	objref	= popReference();

//	_getfield_s_common(index, objref);
//	
//}

/*****************************************************************************
 * This method implements the Java Card 2.1 getfield_s_this instruction.
 * Format:
 * getfield_s_this
 *
 * Stack Image:
 * ..., objref --> ..., value

 *****************************************************************************/
//void _getfield_s_this()
//{
//	u8		D_A    index;
//	jref	D_A    objref;

//	index	= fetchByte();
////	objref	= getLocalReference(0);
//	
//#if JCVM__VM__CHECK_LOCAL
//	if(FPsize > 0)
//	{
//#endif
//		_getfield_s_common(index, PXS_R_U16(FP));
//		return;
//#if JCVM__VM__CHECK_LOCAL
//	}
//	else
//	{
//		fatal_error(STACK_UNDER_FLOW_ERROR);
//	}
//#endif
//}

/*****************************************************************************
 * This routine implements the common portion of the Java Card 2.1
 * putfield_a_xxx instructions.
 * 性能优化 byxuxiaojun@20211130
 *****************************************************************************/
static void _putfield_a_common(u8 index, jref value, jref objref)		SMALL_MODE
{
	PN8		D_A		objaddr;

	objaddr	= MEMtPN8(locateObjectInMemory(objref));

	if(PFtMEM(objaddr)!= REF_NULL) 
	{
		if (index <= get_object_size(objaddr))
		{

#if JCVM__VM__FIRE_WALL
			checkFieldStore(objaddr, value); /* Exit early if the security check fails */
#endif
			/* place the data where it belongs... */
			objaddr = get_object_field_ref(objaddr, index);
			
			if(memReadU16(objaddr) != value)
			{
				nvmWriteU16Atomic(MEMtPN8(objaddr), value);
			}
			return;
		}
		else
		{
			fatal_error(FIELD_INDEX_OUT_OF_OBJECT_ERROR);
		}
	}
	else
	{
		throw_error(NULL_POINTER_EXCEPTION);
	}
}

/*****************************************************************************
 * This method implements the Java Card 2.1 putfield_a instruction.
 * Format:
 * putfield_a
 *
 * Stack Image:
 * ..., objref, value --> ...

 *****************************************************************************/
void _putfield_a()
{
	u8		D_A  index;
	jref	D_A  objref;
	jref	D_A  value;

	index	= fetchByte();
	value	= popReference();
	objref 	= popReference();

	_putfield_a_common(index, value, objref);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 putfield_a_w instruction.
 * Format:
 * putfield_a_w
 *
 * Stack Image:
 * ..., objref, value --> ...

 *****************************************************************************/
void _putfield_a_w()
{
	u8		D_A  index;
	jref	D_A  objref;
	jref 	D_A  value;


	/*
	 IMPORTANT!  Assumption is that the on-card installer will
	 resolve all indices to be in the range of 0-255, since all
	 Class Instances (Objects) may not exceed 255, per the JCVM SPEC.
	 */
	index	= U16t8(fetchShort());
	value	= popReference();
	objref	= popReference();

	_putfield_a_common(index, value, objref);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 putfield_a_this instruction.
 * Format:
 * putfield_a_this
 *
 * Stack Image:
 * ..., objref, value --> ...

 *****************************************************************************/
void _putfield_a_this()
{
	u8		D_A    index;
	jref	D_A    objref;
	jref	D_A    value;

	index	= fetchByte();
	value	= popReference();
	objref	= getLocalReference(0);

	_putfield_a_common(index, value, objref);
}

/*****************************************************************************
 * This routine implements the common portion of the Java Card 2.1
 * putfield_b_xxx instructions.3
 * 性能优化 by xuxiaojun@20211130
 *****************************************************************************/
static void _putfield_b_common(u8 index, u8 value, jref objref)		SMALL_MODE
{
	PN8		D_A		objaddr;
    objaddr = _putgetfield_abs_common(index,objref);

    if(memReadU8(objaddr) != value)
    {
    	nvmWriteU8Atomic(objaddr,value);
    }

}

/*****************************************************************************
 * This method implements the Java Card 2.1 putfield_b instruction.
 * Format:
 * putfield_b
 *
 * Stack Image:
 * ..., objref, value --> ...

 *****************************************************************************/
void _putfield_b()
{
	u8		D_A  index;
	jref	D_A  objref;
	u8		D_A  value;

	index	= fetchByte();
	value	= U16t8(popShort());
	objref	= popReference();

	_putfield_b_common(index, value, objref);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 putfield_b_w instruction.
 * Format:
 * putfield_b_w
 *
 * Stack Image:
 * ..., objref, value --> ...

 *****************************************************************************/
void _putfield_b_w()
{
	u8		D_A  index;
	jref	D_A  objref;
	u8		D_A  value;

	index	= U16t8(fetchShort());
	value	= U16t8(popShort());
	objref	= popReference();

	_putfield_b_common(index, value, objref);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 putfield_b_this instruction.
 * Format:
 * putfield_b_this
 *
 * Stack Image:
 * ..., objref, value --> ...

 *****************************************************************************/
void _putfield_b_this()
{
	u8		D_A    index;
	jref	D_A    objref;
	u8		D_A    value;

	index	= fetchByte();
	value	= U16t8(popShort());
	objref	= getLocalReference(0);
	
	_putfield_b_common(index, value, objref);
}

/*****************************************************************************
 * This routine implements the common portion of the Java Card 2.1
 * putfield_s_xxx instructions.
 *****************************************************************************/
static void _putfield_s_common(u8 index, u16 value, jref objref)		SMALL_MODE
{

	PN8		D_A		objaddr;
    objaddr = _putgetfield_abs_common(index,objref);

	memWriteU16Atomic(objaddr,value);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 putfield_s instruction.
 * Format:
 * putfield_s
 *
 * Stack Image:
 * ..., objref, value --> ...

 *****************************************************************************/
void _putfield_s()
{
	u8		D_A  index;
	jref	D_A  objref;
	u16		D_A  value;

	index	= fetchByte();
	value	= popShort();
	objref	= popReference();
	
	_putfield_s_common(index, value, objref);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 putfield_s_w instruction.
 * Format:
 * putfield_s_w
 *
 * Stack Image:
 * ..., objref, value --> ...

 *****************************************************************************/
void _putfield_s_w()
{
	u8		D_A  index;
	jref	D_A  objref;
	u16		D_A  value;

	index	= U16t8(fetchShort());
	value	= popShort();
	objref	= popReference();
	
	_putfield_s_common(index, value, objref);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 putfield_s_this instruction.
 * Format:
 * putfield_s_this
 *
 * Stack Image:
 * ..., objref, value --> ...

 *****************************************************************************/
void _putfield_s_this()
{
	u8		D_A    index;
	jref	D_A    objref;
	u16		D_A    value;

	index	= fetchByte();
	value	= popShort();
	objref	= getLocalReference(0);

	_putfield_s_common(index, value, objref);
}

#if VM_INTS_SUPPORTED

/*****************************************************************************
 * This method implements the Java Card 2.1 getfield_i instruction.
 * Format:
 * getfield_i
 *
 * Stack Image:
 * ..., objref --> ..., value

 *****************************************************************************/
void _getfield_i()
{
	u8		D_A  index;
	jref	D_A  objref;

	index	= fetchByte();
	objref	= popReference();

	_getfield_i_common(index, objref);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 getfield_i_w instruction.
 * Format:
 * getfield_i_w
 *
 * Stack Image:
 * ..., objref --> ..., value.word1, value.word2

 *****************************************************************************/
void _getfield_i_w()
{
	u8		D_A  index;
	jref	D_A  objref;

	index	= U16t8(fetchShort());
	objref	= popReference();

	_getfield_i_common(index, objref);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 getfield_i_this instruction.
 * Format:
 * getfield_i_this
 *
 * Stack Image:
 * ..., objref --> ..., value.word1, value.word2

 *****************************************************************************/
void _getfield_i_this()
{
	u8		D_A  index;
	jref 	D_A  objref;

	index	= fetchByte();
	objref	= getLocalReference(0);

	_getfield_i_common(index, objref);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 putfield_i instruction.
 * Format:
 * putfield_i
 *
 * Stack Image:
 * ..., objref, value.word1, value.word2 --> ...

 *****************************************************************************/
void _putfield_i()
{								   
	u8		D_A		index;
	jref	D_A		objref;
	u32		D_A		value;

	index	= fetchByte();
	value	= popInt();
	objref	= popReference();

	_putfield_i_common(index, value, objref);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 putfield_i_w instruction.
 * Format:
 * putfield_i_w
 *
 * Stack Image:
 * ..., objref, value.word1, value.word2 --> ...

 *****************************************************************************/
void _putfield_i_w()
{
	u8		D_A		index;
	jref	D_A		objref;
	u32		D_A		value;

	index	= U16t8(fetchShort());
	value	= popInt();
	objref	= popReference();

	_putfield_i_common(index, value, objref);
}

/*****************************************************************************
 * This method implements the Java Card 2.1 putfield_i_this instruction.
 * Format:
 * putfield_i_this
 *
 * Stack Image:
 * ..., objref, value --> ...

 *****************************************************************************/
void _putfield_i_this()
{
	u8		D_A		index;
	jref	D_A		objref;
	u32		D_A		value;

	index	= fetchByte();
	value	= popInt();
	objref	= getLocalReference(0);

	_putfield_i_common(index, value, objref);
}

#endif

