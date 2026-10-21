/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*+
 * File:  object.h
 * Version 1.17
 * Path: /net/mumble/javacard_users/home/oscarm/refimpl.proto2.2/cjdk/VM/cjcre/src/platforms/common/SCCS/s.object.h
 * Last modified on 02/06/03 at 14:21:59
 * Last modifed by: Michael Saltz
 * Original author: David Getreu
 -*/

#ifndef    __OBJECT_H__
#define    __OBJECT_H__

#include "profile.h"
#include "stack.h"
#include "object.h"

#define TERMINAL_CLASS_REF              			0xFFFF

/* Class Record Header bitmask definitions */
#define CRH_IsInterface    							0x80
#define CRH_IsShareable    							0x40
#define CRH_InterfaceCount 							0x0F
/* remote access class record header bitmask definition */
#define CRH_IsAccessRemote 							0x20
#define CRH_JC22           							0x10

/* Method Record Table Structure */
#define SIZE_METHOD_TABLE_HEADER 					2
/* Method Table Header bitmask definitions */
#define MTH_MethodTableBase 						0x00
#define MTH_NumMethods 								(MTH_MethodTableBase + sizeof(u8))

/* Interface Index Table bitmask definitions */
#define IIT_InterfaceClass 							0x00
#define IIT_NumInterfaces  							(IIT_InterfaceClass + sizeof(jref))
#define IIT_FirstIndex     							(IIT_NumInterfaces + sizeof(u8))
#define IIT_IndexSize      							sizeof(u8)

/* size of class_ref structure */
#define SIZE_OF_CLASS_REF  							(2 * sizeof(u8))

/* Method Field Bit that denotes if the referenced
 * method is in the Private Virtual Method Table */
#define PACKAGE_VMT 								(u8)0x80

/* Class Record Structure Element Offset definitions */
#define CR_Header            						0x00
#define CR_SuperClass        						(CR_Header + sizeof(u8))
#define CR_InstanceSize      						(CR_SuperClass + sizeof(jref))
#define CR_FirstRefIndex     						(CR_InstanceSize + sizeof(u8))
#define CR_ReferenceCount    						(CR_FirstRefIndex + sizeof(u8))
#define CR_PublicMethods     						(CR_ReferenceCount + sizeof(u8))
#define CR_PackageMethods    						(CR_PublicMethods + SIZE_METHOD_TABLE_HEADER)
#define CR_MethodsTableStart 						(CR_PackageMethods + SIZE_METHOD_TABLE_HEADER)

#define RM_remote_method_hash 						0x00
#define RM_remote_method_signature 					(RM_remote_method_hash + sizeof(u16))
#define RM_shareable_and_remote_method_token 		(RM_remote_method_signature + sizeof(u16))
#define SIZE_REMOTE_METHOD_INFO 					(sizeof(u16) + sizeof(u16) + 1)

/* masks for Remote Method Information */
#define RMI_ReturnType	   							0x0F
#define RMI_Token          							0x7F
#define RMI_Sharable       							0x80
#define RMI_ParameterType 							0x0F

#define SIZE_OBJECT_HEADER       					(sizeof(u8) * 4 + sizeof(jref))

#define OBJ_HDR_ObjectType     						0x00
#define OBJ_HDR_Context        						(sizeof(u8))
#define OBJ_HDR_ClassRecordREF 						(OBJ_HDR_Context + sizeof(u16))
#define OBJ_HDR_ClassRecordPKG 						(OBJ_HDR_ClassRecordREF + sizeof(jref))

/* OBJ_HDR_TYPE_BYTE Masks */
#define OBJ_HDR_OBJECT_TYPE_MASK   					0xE0
#define OBJ_HDR_GLOBAL_FLAG_MASK   					0x08
#define OBJ_HDR_JCRE_EntryP_MASK   					0x08
#define OBJ_HDR_JCRE_State_MASK    					0x10
#define OBJ_HDR_TransientMode_MASK 					0x02
#define OBJ_HDR_TransientObj_MASK  					0x01
#define OBJ_HDR_TransientMode_DTR					0x02
#define OBJ_HDR_TransientMode_RTR					0
#define OBJ_HDR_PROFILE_RTR_MASK					0x04

/* OBJ_HDR_Context_BYTE Masks */
#define OBJ_HDR_GroupContext_MASK  					GROUP_MASK
#define OBJ_HDR_AppletContext_MASK 					APPLET_MASK

/* JCVM Internal Runtime Object Header ObjectType Definitions */
/* 842 1 8 4 2 1 */
#define Header_Simple_Object            			0x20 /* 001 - - - - - */
#define	Header_NoDefine								0x40 /* 010 - - - - - */
#define Header_ArrayType_Boolean        			0x60 /* 011 - - - - - */
#define Header_ArrayType_Byte           			0x80 /* 100 - - - - - */
#define Header_ArrayType_Short          			0xA0 /* 101 - - - - - */
#define Header_ArrayType_Int            			0xC0 /* 110 - - - - - */
#define Header_ArrayType_Reference      			0xE0 /* 111 - - - - - */

/* Object Header Global Definitions */
#define HEADER_Global_NotShared         			0x0  /* --- 0 - - - - */
#define HEADER_Global_Shared            			0x8 /* --- 1 - - - - */

/* Object Header JCRE Entry Point Object Definitions */
#define HEADER_JCRE_EntryPoint_No       			0x0 /* --- - 0 - - - */
#define HEADER_JCRE_EntryPoint_Yes      			0x8 /* --- - 1 - - - */
/* sensitivearry object*/
#define HEADER_SENSITIVEARRY_No               0x0 /* --- - 0 - - - */
#define HEADER_SENSITIVEARRY_Yes              0x10 /* --- - 1 - - - */
/* Object Header JCRE Entry Point Object Definitions */
#define HEADER_JCRE_STATE_Temporary     			0x0 /* --- - - 0 - - */
#define HEADER_JCRE_STATE_Permanent     			0x10 /* --- - - 1 - - */

/* Object Header profile Rtr Definitions */
#define HEADER_SE_RTR			     				0x0 /* --- - - 0 - - */
#define HEADER_PROFILE_RTR     						0x4 /* --- - - 1 - - */

/* Object Header Transience Mode Definitions */
#define HEADER_JCRE_TransMode_Reset     			0x0 /* --- - - - 0 - */
#define HEADER_JCRE_TransMode_Deselect  			0x2 /* --- - - - 1 - */

/* Object Header Transience Object Definitions */
#define HEADER_JCRE_TransObj_NotTrans   			0x0 /* --- - - - - 0 */
#define HEADER_JCRE_TransObj_IsTrans    			0x1 /* --- - - - - 1 */
#define Object_Fields_START             			(SIZE_OBJECT_HEADER)
#define SIZE_PRIMITIVE_ARRAY_HEADER 				(SIZE_OBJECT_HEADER + sizeof(u16))

#define PA_NumFields_WORD               			(OBJ_HDR_ClassRecordPKG + sizeof(u8))
#define PA_DataArray_START              			(PA_NumFields_WORD + sizeof(u16))

#define SIZE_OBJECT_ARRAY_HEADER 					(SIZE_OBJECT_HEADER + sizeof(jref) + sizeof(u16) + sizeof(u8))

#define OA_NumFields_WORD               			(OBJ_HDR_ClassRecordPKG + sizeof(u8))
#define OA_ComponentClassClassRec       			(OA_NumFields_WORD + sizeof(u16))
#define OA_ComponentClassClassRecPkg    			(OA_ComponentClassClassRec + sizeof(jref))
#define OA_DataArray_START              			(OA_ComponentClassClassRecPkg + sizeof(u8))

/*Transient primitive type arrays header size*/
#define SIZE_TRANSIENT_PRIMIVITE_ARRAY_HEADER 		(SIZE_PRIMITIVE_ARRAY_HEADER + sizeof(u16))

/*Transient Primitve arrays data pointer in RAM*/
#define TPA_Data_Pointer                			(PA_NumFields_WORD + sizeof(u16))

/*transient reference type array data pointer in RAM*/
#define TRA_Data_Pointer                			(OA_ComponentClassClassRecPkg + sizeof(u8))
/*
 * METHOD
 */
extern  memref get_array_data(PF8 array_ref);

#define get_object_field_ref(objref, token) 		PF8_A_U16(objref, U8_A_U16(Object_Fields_START, U8_M_U8(token, SIZE_FIELD)))

#define get_object_reference_field(objref, token) 	PF_R_U16(get_object_field_ref(objref, token))
#define get_object_short_field(objref, token)		PF_R_U16(get_object_field_ref(objref, token))
#define get_object_byte_field(objref, token)		(get_object_short_field(objref, token))

#define get_object_int_field(objref, token) 		PF_R_U32(get_object_field_ref(objref, token))

#define get_object_owner(objref) 					PF_R_U16(PF8_A_CONST(objref, OBJ_HDR_Context))
#define get_object_class(objref) 					get_object_class_impl(objref)
#define get_object_class_pkg(objref) 				get_object_class_impl_pkg(objref)

#define get_object_header_type(x) 					(PF_R_U8(x))
#define get_array_length(array_ref) 				(PF_R_U16(PF8_A_CONST(array_ref, PA_NumFields_WORD)))
#define get_array_component_class(array_ref) 		(PF_R_U16(PF8_A_CONST(array_ref, OA_ComponentClassClassRec)))
#define get_array_component_pkg(array_ref) 			(PF_R_U8(PF8_A_CONST(array_ref, OA_ComponentClassClassRecPkg)))

#define get_class_header(x) 						(PF_R_U8(PF8_A_CONST(x, CR_Header)))

#define get_object_method_info(x) 					(PF_R_U16(PF8_A_CONST(x, CR_MethodsTableStart)))

#define IS_REF_SIMPLEOBJECT(x) 						(U8_AND_U8(get_object_header_type(x), OBJ_HDR_OBJECT_TYPE_MASK) \
													== Header_Simple_Object)

#define IS_SHAREABLE(x) 							(U8_AND_U8(get_class_header(x), CRH_IsShareable) \
													== CRH_IsShareable)

#define IS_INTERFACE(x) 							(U8_AND_U8(get_class_header(x), CRH_IsInterface) \
													== CRH_IsInterface)

#define IS_ACCESS_REMOTE(x) 						(U8_AND_U8(get_class_header(x), CRH_IsAccessRemote) \
													== CRH_IsAccessRemote)

#define IS_JC22(x) 									(U8_AND_U8(get_class_header(x), CRH_JC22) \

/* Tests if the object stored at reference x is a temporary JCRE entry point. */
#define IS_TEMP_ENTRY_OBJ(x) 						((get_object_header_type(x) & \
													(OBJ_HDR_JCRE_EntryP_MASK | OBJ_HDR_JCRE_State_MASK)) \
													== (HEADER_JCRE_EntryPoint_Yes))

/* Tests if the object stored at reference x is a JCRE entry point. */
#define IS_ENTRY_OBJ(x) 							((get_object_header_type(x) & \
													  OBJ_HDR_JCRE_EntryP_MASK) \
													  == HEADER_JCRE_EntryPoint_Yes)
/* if the object is sensitivearry object*/
#define IS_SENSITIVEARRY_OBJ(x)  ((get_object_header_type(x) & \
                                  (HEADER_JCRE_EntryPoint_Yes | HEADER_SENSITIVEARRY_Yes) ) \
																	== HEADER_SENSITIVEARRY_Yes)
/* Tests if the object referenced by x, is shareable. */
#define IS_SHAREABLE_OBJECT(x) 						(IS_SHAREABLE(get_object_class(x)))

/* Tests if the object referenced by x, is a global array. */
#define IS_GLOBAL_ARRAY(x) 							(U8_AND_U8(get_object_header_type(x), (OBJ_HDR_JCRE_State_MASK | OBJ_HDR_GLOBAL_FLAG_MASK)) \
													== HEADER_Global_Shared)

#define IS_ARRAY_DTR(x) 							(U8_AND_U8(get_object_header_type(x), OBJ_HDR_TransientMode_MASK) \
													== HEADER_JCRE_TransMode_Deselect)
/**
 * @throws NULL_POINTER_EXCEPTION
 **/
#define check_addr_null(address)					((*((u8 *)&address) == 0x00) \
													&& (*((u8 *)&address + 1) == 0x00) \
													&& (*((u8 *)&address + 2) == 0x00))
/**
 * @throws ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION
 **/
extern  boolean subType(PF8 S, u8 Spkg, PF8 T);
extern  boolean isinstanceof(PF8 ObjectAddr, u8 AType, PF8 ClassAddr)					SMALL_MODE;

extern  void check_bounds(PF8 array_ref, u16 index, u8 arrayType);

extern  void set_JCRE_entry(PF8 objref, boolean isTemporary);
extern  void set_JCRE_global(PN8 arrayref);
extern  memref get_object_class_impl(PF8 objAddr);
#define get_object_class_impl_pkg(objAddr) 			PF_R_U8(PF8_A_CONST(objAddr, OBJ_HDR_ClassRecordPKG))

/**
 * @return 0xffff if no super-class (normally only java.lang.Object has no
 *         super-class)
 **/
#define get_super_class(class_addr) 				PF_R_U16(PF8_A_CONST(class_addr, CR_SuperClass))

#define get_class_size(c) 							PF_R_U8(PF8_A_CONST(c, CR_InstanceSize))
#define get_firstFieldIndex(c)                        get_class_size(c) //by huang for debug
#define get_object_size(o) 							get_class_size(get_object_class(o))

/**
 * @throws SYSTEMEXCEPT_NO_RESOURCE
 **/
extern	void checkNullAndManageTransaction(jref objRef, u8 transactionStarted);
extern	void checkNullAddrAndManageTransaction(PF8 objAddr, u8 transactionStarted);
extern	void check_null(PF8 ref);
extern  void checkIsGlobal(void);


#endif /* __OBJECT_H__ */
