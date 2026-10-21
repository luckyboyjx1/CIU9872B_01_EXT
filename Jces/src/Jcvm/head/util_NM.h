/*
 * util_NM.h
 *
 *  Created on: 2010-6-12
 *      Author: sunmengsai
 */

#ifndef UTIL_NM_H_
#define UTIL_NM_H_

#include "typejava.h"

#if  JCVM__VM__JAVACARDX_FRAMEWORK_UTIL
#define INT_INT            0x01
#define INT_SHORT          0x02
#define INT_BYTE           0x03
#define BYTE_INT           0x04
#define SHORT_INT          0x05
#define BYTE_BYTE          0x06
#define SHORT_SHORT        0x07
#define BYTE_SHORT         0x08
#define SHORT_BYTE         0x09

extern	void arrayCopyRepack(u8 isAtomicCopy);
extern	void arrayCompareGeneric(void);
extern	void arrayFillGenericNonAtomic(void);
extern	void arrayFindGeneric(void);
extern  void arrayFillGeneric(void);
extern  boolean	isPrimitiveComponent(PF8 array_ref);
extern  boolean isInstanceof(PF8 array_ref,u8 arrayType);
extern  u8 check_arraypack_type(PM8 destAddr,PM8 srcAddr);
#else
#define  arrayCopyRepack(isAtomicCopy)
#define  arrayCompareGeneric()
#define  arrayFillGenericNonAtomic()
#define  arrayFindGeneric()
#endif

#if  JCVM__VM__JAVACARDX_FRAMEWORK_UTIL_INTX
extern	void setInt(void);
#else
#define  setInt()
#endif

#endif /* UTIL_NM_H_ */
