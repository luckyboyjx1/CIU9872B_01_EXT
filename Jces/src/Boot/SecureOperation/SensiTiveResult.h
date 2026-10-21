#ifndef SENSITIVE_RESULT_H
#define SENSITIVE_RESULT_H

#include "ecpDefine.h"
#include "profile.h"
#include "typejava.h"
#if SENSITIVE_RESULT_SUPPORT

#define	SENSITIVE_RESULT_constantTrue		0xA55A
#define	SENSITIVE_RESULT_constantFalse		0x5AA5

// Sensitiveresult Offset declarations MACRO for internal machine state
#define	SENSITIVE_RESULT_offShort			(0 * SIZE_SHORT) 
#define	SENSITIVE_RESULT_offObject			(0 * SIZE_SHORT) 
#define	SENSITIVE_RESULT_offTagValValidBool	(1 * SIZE_SHORT) 
#define	SENSITIVE_RESULT_offTypeShort		(2 * SIZE_SHORT) 
#define	SENSITIVE_RESULT_offTypeBoolean		(3 * SIZE_SHORT)  
#define	SENSITIVE_RESULT_offTypeObject		(4 * SIZE_SHORT)  
#endif /*SENSITIVE_RESULT_SUPPORT*/

extern void sensitiveResultSetTagValUnassigned(void);
extern void sensitiveResultSetBooleanFalse(void);
extern void sensitiveResultSetBooleanTrue(void);
extern void sensitiveResultSetObject(void);
extern void sensitiveResultSet(short x);

extern void sensitiveResultAssertEqualsObj(u16 obj);
extern void sensitiveResultAssertEquals(u16 val);
extern void sensitiveResultAssertTrue(void);
extern void sensitiveResultAssertFalse(void);
extern void sensitiveResultAssertGreaterThan(u16 val);
extern void sensitiveResultAssertLessThan(u16 val);
extern void sensitiveResultAssertNegative(void);
extern void sensitiveResultAssertPositive(void);
extern void sensitiveResultAssertZero(void);
extern void sensitiveResultReset(void);

extern void sensitiveResultInit(void);
extern void sensitiveResult(void);//根据 合代码时代码评审(20220722).xlsx进行修改： 将JAVA CARD305的本地方法放在nativemethod_impl_JavaCard305中

#endif /*SENSITIVE_RESULT_H*/

