/*
 * @(#)native_interface_JavaCard305.h	2022/10/20
 */

#ifndef    __nativemethod_impl_JavaCard305_H__
#define    __nativemethod_impl_JavaCard305_H__

#include "typejava.h"
#include "SensiTiveResult.h"
//根据评审意见修改，将nativeinterface的挪到305专有文件中 modifybydengyiwen@20230111
#if SENSITIVE_RESULT_SUPPORT
#define AssertEqualsObj         0x01
#define AssertEquals            0x02
#define AssertTrue              0x03
#define AssertFalse             0x04
#define AssertGreaterThan       0x05
#define AssertLessThan          0x06
#define AssertNegative          0x07
#define AssertPositive          0x08
#define AssertZero              0x09
#define Reset                   0x0A
#endif

extern void GetSignatureAlgorithm(void);
extern void GetCipherAlgorithm(void);
extern void arrayFill(void);
extern void setSensitiveArryObj(void);
extern void isSensitiveArryObj(void);
extern void clearArray(void);

#endif /* __nativemethod_impl_JavaCard305_H__ */
