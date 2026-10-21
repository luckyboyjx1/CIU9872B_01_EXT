#include "SensiTiveResult.h"
#include "ob_manager.h"
#include "object.h"
#include "nativemethod_impl.h"

/*
 * file name:  SensitiveResult.c
 * creater: dengyiwen
 *
 * 数组结构：
 *  -------------------------------------------------------------------------------------
 * |           2byte            |    2byte    |    2byte    |     2byte    |    2byte    |
 * |存放结果(bool值，对象索引...)|数组有效标志位|16位结果标志位|bool结果标志位 |对象结果标志位|
 *  -------------------------------------------------------------------------------------
 */

/*
 * function name:  sensitiveResultSetTagValUnassigned
 * function:       初始化数组
 */
#if WITH_JC305
void sensitiveResultSetTagValUnassigned(void)
{
	#if SENSITIVE_RESULT_SUPPORT
	memref theInternAddr;
	memref theDataInternAddr;
	
	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL)
	{
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data((u8*)theInternAddr);
		// check that object not null 
		if (theInternAddr == REF_NULL) {
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}
    
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool), SENSITIVE_RESULT_constantFalse);
		//memWriteU16NoAtomic((memref)(theDataInternAddr + SENSITIVE_RESULT_offTypeShort), SENSITIVE_RESULT_constantFalse);
		//memWriteU16NoAtomic((memref)(theDataInternAddr + SENSITIVE_RESULT_offTypeBoolean), SENSITIVE_RESULT_constantFalse);
		//memWriteU16NoAtomic((memref)(theDataInternAddr + SENSITIVE_RESULT_offTypeObject), SENSITIVE_RESULT_constantFalse);
	}

	return;
	#endif
}
/*
 * function name:    sensitiveResultSetBooleanFalse
 * function:         设置结果为bool类型的false
 */
void sensitiveResultSetBooleanFalse(void)
{
	#if SENSITIVE_RESULT_SUPPORT
	memref theInternAddr;
	memref theDataInternAddr;

	//if(sensitiveResultInternStruct.internalState == REF_NULL)
	//	Java_NativeMethod_com_sun_javacard_impl_NativeMethods_sensitiveResultInitialize();

	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL)
	{
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data((u8*)theInternAddr);
		// check that object not null 
		if (theInternAddr == REF_NULL)
		{
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}
    
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offShort), SENSITIVE_RESULT_constantFalse);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool), SENSITIVE_RESULT_constantTrue);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeShort), SENSITIVE_RESULT_constantFalse);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeBoolean), SENSITIVE_RESULT_constantTrue);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeObject), SENSITIVE_RESULT_constantFalse);
	}
	#endif
}
/*
 * function name:        sensitiveResultSetBooleanTrue
 * function              设置结果为bool类型的true
 */
void sensitiveResultSetBooleanTrue(void)
{
	#if SENSITIVE_RESULT_SUPPORT
	memref theInternAddr;
	memref theDataInternAddr;
	
	//if(sensitiveResultInternStruct.internalState == REF_NULL)
	//	Java_NativeMethod_com_sun_javacard_impl_NativeMethods_sensitiveResultInitialize();

	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL)
	{
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data((u8*)theInternAddr);
		// check that object not null 
		if (theInternAddr == REF_NULL)
		{
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}
    
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offShort), SENSITIVE_RESULT_constantTrue);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool), SENSITIVE_RESULT_constantTrue);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeShort), SENSITIVE_RESULT_constantFalse);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeBoolean), SENSITIVE_RESULT_constantTrue);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeObject), SENSITIVE_RESULT_constantFalse);
	}
	#endif
}
/*
 * function name:      sensitiveResultSetObject
 * function:           设置结果为对象类型，在数组头的地方保存对象索引
 */
void sensitiveResultSetObject(void)
{
	#if SENSITIVE_RESULT_SUPPORT
		memref theInternAddr;
	memref theDataInternAddr;
	jref array = popReference();


	//if(sensitiveResultInternStruct.internalState == REF_NULL)
	//	Java_NativeMethod_com_sun_javacard_impl_NativeMethods_sensitiveResultInitialize();

	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL) {
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data((PM8)theInternAddr);
		// check that object not null 
		if (theInternAddr == REF_NULL) {
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}
    
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offShort), array);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool), SENSITIVE_RESULT_constantTrue);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeShort), SENSITIVE_RESULT_constantFalse);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeBoolean), SENSITIVE_RESULT_constantFalse);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeObject), SENSITIVE_RESULT_constantTrue);
	}
	#endif
}
/*
 * function name:  sensitiveResultSet
 * function:       设置一个16位的结果
 */
void sensitiveResultSet(short x)
{
	#if SENSITIVE_RESULT_SUPPORT
	memref theInternAddr;
	memref theDataInternAddr;
	
	//if(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) == REF_NULL)
	//	Java_NativeMethod_com_sun_javacard_impl_NativeMethods_sensitiveResultInitialize();

	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL) {
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data((PM8)theInternAddr);
		// check that object not null 
		if (theInternAddr == REF_NULL) {
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}
    
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offShort), x);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool), SENSITIVE_RESULT_constantTrue);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeShort), SENSITIVE_RESULT_constantTrue);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeBoolean), SENSITIVE_RESULT_constantFalse);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeObject), SENSITIVE_RESULT_constantFalse);
	}
	#endif
}
/*
 * function name:    sensitiveResultAssertEqualsObj
 * function:         设置一个对象类型的结果
 */
void sensitiveResultAssertEqualsObj(u16 obj)
{
	#if SENSITIVE_RESULT_SUPPORT 
	memref theInternAddr;
	memref theDataInternAddr;

	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL)
	{
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data((PF8)theInternAddr);
		// check that object not null 
		if (theInternAddr == REF_NULL) 
		{
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}
		
		if ((memReadU16((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeObject)) == SENSITIVE_RESULT_constantTrue) && (memReadU16((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool)) == SENSITIVE_RESULT_constantTrue))
		{
			if (! (obj == memReadU16((PM8)theDataInternAddr + SENSITIVE_RESULT_offObject)))
			{ 
				throw_error(SECURITY_EXCEPTION);
				return;
			}
		} 
		else
		{
			throw_error(SECURITY_EXCEPTION);
			return;
		}
	} 
	else 
	{
		throw_error(SECURITY_EXCEPTION);
		return;
	}
	return;
	#endif
}
/*
 * function name:     sensitiveResultAssertEquals
 * function:          对比结果是否与期望值一致
 * param:
 * u16 val:           期望值
 * return:            不一致抛安全异常
 */
void sensitiveResultAssertEquals(u16 val)
{
	#if SENSITIVE_RESULT_SUPPORT 
	memref theInternAddr;
	memref theDataInternAddr;

	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL)
	{
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data((PM8)theInternAddr);
		// check that object not null 
		if (theInternAddr == REF_NULL)
		{
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}

		if ((memReadU16( (PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeShort) ) == SENSITIVE_RESULT_constantTrue) && (memReadU16((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool)) == SENSITIVE_RESULT_constantTrue) )
		{
			if ( memReadU16((PM8)theDataInternAddr + SENSITIVE_RESULT_offShort) != val )
			{ 
				throw_error(SECURITY_EXCEPTION);
				return;
			}
		} 
		else 
		{
			throw_error(SECURITY_EXCEPTION);
			return;
		}
	} 
	else
	{
		throw_error(SECURITY_EXCEPTION);
		return;
	}
	return;
	#endif
}
/*
 * function name:   sensitiveResultAssertTrue
 * function:        判断结果是否为bool true
 *                  不为true抛安全异常
 */
void sensitiveResultAssertTrue(void)
{
	#if SENSITIVE_RESULT_SUPPORT 
	memref theInternAddr;
	memref theDataInternAddr;

	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL)
	{
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data((PM8)theInternAddr);
		// check that object not null 
		if (theInternAddr == REF_NULL)
		{
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}

		if ((memReadU16( (PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeBoolean) ) == SENSITIVE_RESULT_constantTrue) && (memReadU16((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool)) == SENSITIVE_RESULT_constantTrue))
		{
			if (memReadU16((PM8)(theDataInternAddr + SENSITIVE_RESULT_offShort) ) != SENSITIVE_RESULT_constantTrue ) { 			
				throw_error(SECURITY_EXCEPTION);
				return;
			}
		} 
		else 
		{
			throw_error(SECURITY_EXCEPTION);
			return;
		}
	} 
	else
	{
		throw_error(SECURITY_EXCEPTION);
		return;
	}
	return;
	#endif
}
/*
 * function name:   sensitiveResultAssertFalse
 * function:        判断结果是否为bool false
 *                  不为false抛安全异常
 */
void sensitiveResultAssertFalse(void)
{
	#if SENSITIVE_RESULT_SUPPORT 
		memref theInternAddr;
	memref theDataInternAddr;

	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL)
	{
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data(MEMtPM8(theInternAddr));
		// check that object not null 
		if (theInternAddr == REF_NULL)
		{
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}

		if ((memReadU16( (PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeBoolean) ) == SENSITIVE_RESULT_constantTrue) && (memReadU16((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool)) == SENSITIVE_RESULT_constantTrue))
		{
			if (memReadU16((PM8)theDataInternAddr + SENSITIVE_RESULT_offShort) != SENSITIVE_RESULT_constantFalse )
			{ 
				throw_error(SECURITY_EXCEPTION);
				return;
			}
		}
		else
		{
			throw_error(SECURITY_EXCEPTION);
			return;
		}
	}
	else
	{
		throw_error(SECURITY_EXCEPTION);
		return;
	}
	return;
	#endif
}
/*
 * function name:   sensitiveResultAssertGreaterThan
 * function:        判断结果是否大于期望值
 * param:
 * u16 val:         期望值
 *                  不大于期望值抛安全异常
 */
void sensitiveResultAssertGreaterThan(u16 val)
{
	#if SENSITIVE_RESULT_SUPPORT 
	memref theInternAddr;
	memref theDataInternAddr;

	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL)
	{
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data((PM8)theInternAddr);
		// check that object not null 
		if (theInternAddr == REF_NULL)
		{
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}

		if ((memReadU16( (PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeShort) ) == SENSITIVE_RESULT_constantTrue) && (memReadU16((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool)) == SENSITIVE_RESULT_constantTrue))
		{
			if ( U16tS16(memReadU16((PM8)theDataInternAddr + SENSITIVE_RESULT_offShort)) <= U16tS16(val) )
			{ 
				throw_error(SECURITY_EXCEPTION);
				return;
			}
		}
	 else
		{
			throw_error(SECURITY_EXCEPTION);
			return;
		}
	}
	else
	{
		throw_error(SECURITY_EXCEPTION);
		return;
	}
	return;
	#endif
}
/*
 * function name:   sensitiveResultAssertLessThan
 * function:        判断结果是否小于期望值
 * param:
 * u16 val:         期望值
 *                  不小于期望值抛安全异常
 */
void sensitiveResultAssertLessThan(u16 val)
{
	#if SENSITIVE_RESULT_SUPPORT 
	memref theInternAddr;
	memref theDataInternAddr;

	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL)
	{
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data((PM8)theInternAddr);
		// check that object not null 
		if (theInternAddr == REF_NULL)
		{
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}

		if ((memReadU16( (PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeShort) ) == SENSITIVE_RESULT_constantTrue) && (memReadU16((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool)) == SENSITIVE_RESULT_constantTrue))
		{
			if ( U16tS16(memReadU16((PM8)theDataInternAddr + SENSITIVE_RESULT_offShort)) >= U16tS16(val) )
			{ 
				throw_error(SECURITY_EXCEPTION);
				return;
			}
		}
		else
		{
			throw_error(SECURITY_EXCEPTION);
			return;
		}
	}
	else
	{
		throw_error(SECURITY_EXCEPTION);
		return;
	}
	return;
	#endif
}
/*
 * function name:         sensitiveResultAssertNegative
 * function:              判断16位结果是否为负数
 *                        结果为正数抛安全异常
 */
void sensitiveResultAssertNegative(void)
{
	#if SENSITIVE_RESULT_SUPPORT 
	memref theInternAddr;
	memref theDataInternAddr;

	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL)
	{
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data((PM8)theInternAddr);
		// check that object not null 
		if (theInternAddr == REF_NULL)
		{
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}

		if ( (memReadU16( (PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeShort) ) == SENSITIVE_RESULT_constantTrue) && (memReadU16((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool)) == SENSITIVE_RESULT_constantTrue) )
		{
			if ( U16tS16(memReadU16((PM8)theDataInternAddr + SENSITIVE_RESULT_offShort)) >= 0 ) { 
				throw_error(SECURITY_EXCEPTION);
				return;
			}
		}
		else
		{
			throw_error(SECURITY_EXCEPTION);
			return;
		}
	}
	else
	{
		throw_error(SECURITY_EXCEPTION);
		return;
	}
	return;
	#endif
}
/*
 * function name:         sensitiveResultAssertPositive
 * function:              判断16位结果是否为正数
 *                        结果为负数抛安全异常
 */
void sensitiveResultAssertPositive(void)
{
	#if SENSITIVE_RESULT_SUPPORT 
	memref theInternAddr;
	memref theDataInternAddr;

	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL)
	{
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data((PM8)theInternAddr);
		// check that object not null 
		if (theInternAddr == REF_NULL)
		{
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}

		if ((memReadU16( (PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeShort) ) == SENSITIVE_RESULT_constantTrue) && (memReadU16((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool)) == SENSITIVE_RESULT_constantTrue) )
		{
			if ( U16tS16(memReadU16((PM8)theDataInternAddr + SENSITIVE_RESULT_offShort)) <= 0 )
			{ 
				throw_error(SECURITY_EXCEPTION);
				return;
			}
		}
		else
		{
			throw_error(SECURITY_EXCEPTION);
			return;
		}
	}
	else
	{
		throw_error(SECURITY_EXCEPTION);
		return;
	}
	return;
	#endif
}
/*
 * function name:         sensitiveResultAssertZero
 * function:              判断16位结果是否为0
 *                        结果不为0抛安全异常
 */
void sensitiveResultAssertZero(void)
{
	#if SENSITIVE_RESULT_SUPPORT 
	memref theInternAddr;
	memref theDataInternAddr;

	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL)
	{
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data((PM8)theInternAddr);
		// check that object not null 
		if (theInternAddr == REF_NULL)
		{
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}

		if ((memReadU16( (PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeShort) ) == SENSITIVE_RESULT_constantTrue) && (memReadU16((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool)) == SENSITIVE_RESULT_constantTrue) )
		{
			if ( memReadU16((PM8)theDataInternAddr + SENSITIVE_RESULT_offShort) != 0 ) { 
				throw_error(SECURITY_EXCEPTION);
				return;
			}
		}
		else
		{
			throw_error(SECURITY_EXCEPTION);
			return;
		}
	}
	else
	{
		throw_error(SECURITY_EXCEPTION);
		return;
	}
	return;
	#endif
}
/*
 * function name:   sensitiveResultReset
 * function:        重置数组
 */
void sensitiveResultReset(void)
{
	#if SENSITIVE_RESULT_SUPPORT 
		memref theInternAddr;
	memref theDataInternAddr;

	if (PN_R_U16(OBJECT_sensitiveResultArry_OFFSET) != REF_NULL)
	{
		theInternAddr = locateObjectInMemory(PN_R_U16(OBJECT_sensitiveResultArry_OFFSET));
		theDataInternAddr = get_array_data((PM8)theInternAddr);
		// check that object not null 
		if (theInternAddr == REF_NULL)
		{
			throw_error(NULL_POINTER_EXCEPTION);
			return;
		}
				
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTagValValidBool), SENSITIVE_RESULT_constantFalse);
		//memWriteU16NoAtomic( (PM8)(theDataInternAddr + SENSITIVE_RESULT_offShort), SENSITIVE_RESULT_constantFalse);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeShort), SENSITIVE_RESULT_constantFalse);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeBoolean), SENSITIVE_RESULT_constantFalse);
		memWriteU16NoAtomic((PM8)(theDataInternAddr + SENSITIVE_RESULT_offTypeObject), SENSITIVE_RESULT_constantFalse);
	}
	else
	{
		throw_error(SECURITY_EXCEPTION);
		return;
	}
	return;
	#endif
}
#endif /*WITH_JC305*/
