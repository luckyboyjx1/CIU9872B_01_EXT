#ifndef  __UTIL_METHOD_H__
#define  __UTIL_METHOD_H__
#include "typejava.h"

#include "ecpReadWrite.h"
#include "ecpOperation.h"

#define  MASK_EXTERNAL									0x80
#define  MASK_HIGH_BIT_ON								0x80
#define  MASK_IS_PKG_METHOD								0x80
#define  MASK_HIGH_BIT_OFF								0x7F
#define  MASK_INTERFACE_COUNT							0x0F

#define store_object_B_filed(objAddr, index , value)	memWriteU8Atomic(MEMtPM8(get_object_field_ref(objAddr, index)), value)
#define store_object_S_filed(objAddr, index , value)	memWriteU16Atomic(MEMtPM8(get_object_field_ref(objAddr, index)), value)
#define store_object_I_filed(objAddr, index , value)	memWriteU32Atomic(MEMtPM8(get_object_field_ref(objAddr, index)), value)
#define makeShort(b1, b2) 								U16_OR_U16(U16_SHL_U8(b1, 8), U8_AND_U8(b2, 0xff))
extern  void check_bounds_interval(PF8 array_ref, u16 index, u16 length);
extern  void check_arraybounds_interval(PF8 array_ref, u16 off, u16 length);
#endif
