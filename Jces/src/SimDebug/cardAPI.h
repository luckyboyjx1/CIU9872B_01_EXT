#ifndef __HW_DATA_OPERATION_H__
#define	__HW_DATA_OPERATION_H__

	#include <vm_callback.h>
#include "ecpOperation.h"
	#include "ecpReadWrite.h"
	#include "packageloc.h"
	#include "global_variable.h"

#include "MemOperationServe.h"
//	#include "Io_14443TaL3.h"
/*************** read write for debug ******************************/
	#define read_u8(addr)                       ((u8)(*((u8 *)(addr))))
	#define read_u16(addr)                      ((u16)(((*(addr)) << 8) | (*((addr) + 1))))
	#define read_u32(addr)                      ((u32)(((*(addr)) << 24) | ((*(addr + 1)) << 16) | ((*(addr + 2)) << 8) | (*(addr + 3))))

	#define write_u8(d, v)						(* (PX8)(memref)(d) = v)
	#define write_u16(d, v)						(* (PX16)(memref)(d) = htoj16(v))
	#define write_u32(d, v)						(* (PX32)(memref)(d) = htoj32(v))

	#define MEMtU8(v)							(u8 *)((memref)(v))
/*************** function from card for hw ******************************/
//get the address of a array by an array_ref
#define  GET_ARRAY_DATA(array_ref)              	vm_GET_ARRAY_DATA(array_ref)

//get the length of array by an array_ref
#define  GET_ARRAY_LENGTH(array_ref)           		vm_GET_ARRAY_LENGTH(array_ref)

//read one byte from memory by an address
#define  MEMREADU8(addr)                        	vm_MEMREADU8(addr)
//read one short from memory by an address
#define  MEMREADU16(addr)                       	vm_MEMREADU16(addr)
//read one int from memory by an address
#define  MEMREADU32(addr)                       	vm_MEMREADU32(addr)

//copy data with a length of length from src to dest
#define  MEMCOPYATOMIC(dest, src, length)       	vm_MEMCOPYATOMIC(dest, src, length)
//get the bytecode address of the package by pkgID a compID
#define  GETPACKAGECOMPONENTLOCATION(pkgID, compID) vm_GETPACKAGECOMPONENTLOCATION(pkgID, compID)
//get the Top index of the map table used.
#define  MAPTABLE_GETTOPINDEX()                 	vm_JCVMCONFIG_GETMAXENTRYCOUNTINMAP()
//get an entry address by an entryIndex
#define  MAPTABLE_GETENTRYADDR(entryIndex)      	vm_LOCATEOBJECTINMEMORY(entryIndex)
//get an entry address by an entryIndex
#define  MAPTABLE_GETENTITYCONTENT(entryIndex)  	vm_GETENTITYCONTENT(entryIndex)

//get field data of an object
#define  GET_OBJECT_FIELD_REF(objref, token)    	vm_GET_OBJECT_FIELD_REF(objref, token)

//read one short from vnm by an address
#define  NVMREADU16(addr)                       	vm_NVMREADU16(addr)
//read one int from nvm by an address
#define  NVMREADU32(addr)                       	vm_NVMREADU32(addr)

#define CARD_DEBUGMASK						vm_GETDEBUGMASK()


#define CARD_OFFSET_CLASS_DEC_INST_SIZE		OFFSET_CLASS_DEC_INST_SIZE
//If no Ta feature
//#if !HAL__IO__ISO14443_TA
//	phIso14443L3_Control_t						phIso14443L3_control;
//#endif

#define CARD_COMPONENT_CLASS 				 COMPONENT_CLASS
#define CARD_COMPONENT_METHOD 				 COMPONENT_METHOD

#define CARD_STACKSIZE 						 STACKSIZE
#define CARD_PACKAGE_LOCATION_TABLE_ADDRESS  vm_PACKAGE_LOCATION_TABLE_ADDRESS()
#define CARD_DEBUG_COMPONENT_PKG_COUNT       vm_DEBUG_COMPONENT_PKG_COUNT()
#define CARD_SIZE_MEMREF                     SIZE_MEMREF
#define CARD_EEPAGE_SIZE					EEPAGE_SIZE            //=128

#define CARD_TOP							 Top
#define CARD_VMSTACK						 MEMtPX8(VMStack) //modify by jiangxiaohui@20220324
#define CARD_JPC                             JPC
#define CARD_PCPKGID                         PCpkgID
#define CARD_FP                              FP

#define CARD_STREAM_BUFFER					stream_buffer
#define CARD_STREAM_PAGE					stream_page

#endif
