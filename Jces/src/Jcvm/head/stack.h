/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*+
 * File:  stack.h
 * Version 1.5
 * Path: /home/sa112047/ws/refimpl.saqib/cjdk/VM/cjcre/src/platforms/common/SCCS/s.stack.h
 * Last modified on 01/03/26 at 15:18:17
 * Last modifed by: Eric Nellen
 * Original author: David Getreu, Alexandre Frey, Nicolas Regnault
 -*/

#ifndef    __STACK_H__
#define    __STACK_H__

#include "profile.h"
#include "memory.h"
#include "optspace.h"
/*#include "typejava.h"*/

/*
 * Methods
 */

#define	JPC_PTR									0
#define CACHED_CLASS_PTR						1
#define CACHED_METHOD_PTR						2
#define CACHED_STATIC_FIELD_PTR					3

#define STACK_PTR_NO_OPE						0
#define STACK_PTR_INCREASE						1
#define STACK_PTR_DECREASE						2

#define STACK_JPC_SAVE_OFFSET					6

#define GLOBABL_VM_PTR_NUM						4

extern  void initStackParams(void);
#if JCVM__VM__CHECK_STACK_POP
extern void pushFrame(u8 m_paramsize, u8 m_localsize, u8 m_maxstack)	SMALL_MODE;
#else
extern void pushFrame(u8 m_paramsize, u8 m_localsize);
#endif

extern void popFrame(void);

#if JCVM__VM__CHECK_STACK
#if !EMV_OPTIMIZATION_CODE
extern void pushShort(u16 value);
#endif //!EMV_OPTIMIZATION_CODE
#define pushReference(ref)	pushShort(ref)

extern u16 popShort(void);
			  
					  
#define popReference()		popShort()								
extern void checkPop(PX8 p)    SMALL_MODE;	
extern void checkLocal(u8 offset);
extern void setLocalShort(u8 offset, jshort value);
extern u16 getLocalShort(u8 offset);
					  
#define getLocalReference(index)				getLocalShort(index)
#define setLocalReference(index, value)			setLocalShort(index, value)
#else
#define pushShort(value)			(PX_W_U16(JSP, value), PX8_A_CONST(JSP, 2))
#define pushReference(ref)			(PX_W_U16(JSP, ref), PX8_A_CONST(JSP, 2))
#define popShort() 					(PX8_S_CONST(JSP, 2), PX_R_U16(JSP))
#define popReference()  			(PX8_S_CONST(JSP, 2), PX_R_U16(JSP))
#define getLocalShort(index)		(PX_R_U16(PX8_A_U8(FP, U8_M_U8(2, index))))
#define getLocalReference(index) 	(PX_R_U16(PX8_A_U8(FP, U8_M_U8(2, index))))
//#define setLocalReference(index,ref) *(FP + (index)) = (ref)
#endif

//extern void popOperands(void);
#define	popOperands() JSP = FPend
extern u16 getPreviousStackContext(void);
//extern jref fetchStackReference(u8 nth);
#define fetchStackReference(nth) PXS_R_U16(PX8_S_U16(JSP, U8_M_U8(2, nth)))
extern boolean isStackNotEmpty(void);

#if VM_INTS_SUPPORTED
extern void pushInt(u32 value);
extern u32 popInt(void);
extern u32 getLocalInt(u8 offset);
extern void setLocalInt(u8 index, jint value)		SMALL_MODE;
#endif

/* 32-bit memory addressing changes */
#define pushAddress(x) 			pushInt(x)
#define popAddress() 			popInt()
#if		BOOT__MEMORY__NVM_RESIZE
	extern u8 		JCstack_GetUpdatePtrNum(void);
	extern memref 	JCstack_UpdatePtr(u8 i, u32 changeLen, u8 opType, u8 isUpdatePtr);
#else
	#define		JCstack_GetUpdatePtrNum()		0
	#define		JCstack_UpdatePtr( i,  changeLen,  opType,  isUpdatePtr)		0
#endif


extern void BackCurrentJavaPoint(void);
#endif /* __STACK_H__ */
