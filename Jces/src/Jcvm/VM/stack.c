/*****************************************************************************
 * File:  stack.c
 * Version 1.6

 wang hui
 eastcompeace Technology Corporation
 Modified :
 add config to push PC into stack,pushTopReference(JSPC) ->>pushTopInt(PC)
 History:
 2007,7,16	.modify
 * Original author: David Getreu
 ****************************************************************************/
#include "stack.h"

#include "profile.h"
#include "mask.h"
#include "memory.h"
#include "object.h"
#include "context.h"
#include "error.h"
#include "packageloc.h"
#include "context.h"
#include "global_variable.h"
#include "anomaly.h"
#include "target.h"
#include "SEEUICCInterface.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

/* size (in bytes) of the top frame context */
//#define FRAME_CONTEXT_SIZE 12
#define FRAME_CONTEXT_SIZE      (u8)12

#define UNDEFINED_MEMORY_CELL           (memref) 0xFFFF

/**
 * Initializes all stacks parameters for the VM beginning.
 */
void initStackParams()
{
	FP	= MEMtPX8(VMStack);
	FPend	= MEMtPX8(VMStack);
	JSP = MEMtPX8(VMStack);
	Top = PX8_A_CONST(VMStack,  STACKSIZE);
// 这两个变量用于备份跳转点 在UICC 和STK中某些情况需要直接回到Java主流程进行派发
	BackTop = NULL;
	BackFP = NULL;
}

/**********************************************************************
 * olny using push PC in stack ,we can dirctoried fetch the pc in stack
 * we assume the pc is 4 byte
 * used this mode ,vm can excute java bytecode faster,but this need more
 * var statc JSPace.
 ***********************************************************************/
#if JCVM__VM__CHECK_STACK
#if !EMV_OPTIMIZATION_CODE
WEAK void pushShort(u16 value)
{
	PXS_W_U16(JSP, value);

	JSP	= PX8_A_CONST(JSP, 2);   //wzq: 	JSP++;
#if JCVM__VM__CHECK_STACK
	if (JSP > Top)
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
#endif
}
#endif //!EMV_OPTIMIZATION_CODE
#endif

#if VM_INTS_SUPPORTED
void pushInt(u32 value)
{
	PX_W_U32(JSP, value);

	JSP	= PX8_A_CONST(JSP, 4);    //wzq: 	JSP += 2;

#if JCVM__VM__CHECK_STACK
	if (JSP <= Top)
	{
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
#endif
}
#endif

#if JCVM__VM__CHECK_STACK
void checkPop(PX8 p)    SMALL_MODE
{
	if (p >= FPend)
	{
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}
#endif

u16 popShort(void)
{
	JSP	= PX8_S_CONST(JSP, 2);

#if JCVM__VM__CHECK_STACK
	if (JSP >= FPend)
	{
#endif
		return PXS_R_U16(JSP);
#if	JCVM__VM__CHECK_STACK
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
#endif

}

#if VM_INTS_SUPPORTED

u32 popInt(void)
{
	JSP	= PX8_S_CONST(JSP, 4);

#if JCVM__VM__CHECK_STACK
	if (JSP >= FPend)
	{
#endif
		return PX_R_U32(JSP);
#if	JCVM__VM__CHECK_STACK
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
#endif

}

#endif

//void popOperands()
//{
//	JSP	= FPend;
//}

#if JCVM__VM__CHECK_STACK
void checkLocal(u8 offset)
{
#if JCVM__VM__CHECK_LOCAL
	if(FPsize > offset)
	{
		return;
	}
	else
	{
		fatal_error(STACK_UNDER_FLOW_ERROR);
	}
#endif
}

u16 getLocalShort(u8 offset)
{
#if JCVM__VM__CHECK_LOCAL
	if(FPsize > offset)
	{
#endif
		return PXS_R_U16(PX8_A_U16(FP, U8_M_U8(2, offset)));
#if JCVM__VM__CHECK_LOCAL
	}
	else
	{
		fatal_error(STACK_UNDER_FLOW_ERROR);
	}
#endif

}


#if !EMV_OPTIMIZATION_CODE
WEAK void setLocalShort(u8 offset, jshort value)
{
#if JCVM__VM__CHECK_LOCAL
	if(FPsize > offset)
	{
#endif
		PXS_W_U16(PX8_A_U16(FP, U8_M_U8(2, offset)), value);
#if JCVM__VM__CHECK_LOCAL
	}
	else
	{
		fatal_error(STACK_UNDER_FLOW_ERROR);
	}
#endif
}
#endif

/*
jref getLocalReference(u8 offset)
{
#if JCVM__VM__CHECK_LOCAL
	if(U8_A_U8(paramsize, localsize) < offset)
	{
		fatal_error(STACK_UNDER_FLOW_ERROR);
	}
#endif
	return PX_R_U16(PX8_A_U8(FP, U8_M_U8(2, offset)));
}


void setLocalReference(u8 offset, jref value)
{
#if JCVM__VM__CHECK_LOCAL
	if(localsize < offset)
	{
		fatal_error(STACK_UNDER_FLOW_ERROR);
	}
#endif
	*(FP + offset) = value;
}
*/
#endif

#if VM_INTS_SUPPORTED

u32 getLocalInt(u8 offset)
{
#if JCVM__VM__CHECK_LOCAL
	if(FPsize > (offset + 1))
	{
#endif
		return PX_R_U32(PX8_A_U8(FP, U8_M_U8(2, offset)));
#if JCVM__VM__CHECK_LOCAL
	}
	else
	{
		fatal_error(STACK_UNDER_FLOW_ERROR);
	}
#endif

}

void setLocalInt(u8 offset, jint value)		SMALL_MODE
{
#if JCVM__VM__CHECK_LOCAL
	if(FPsize > (offset + 1))
	{
#endif
		PX_W_U32(PX8_A_U8(FP, U8_M_U8(2, offset)), value);
#if JCVM__VM__CHECK_LOCAL
	}
	else
	{
		fatal_error(STACK_UNDER_FLOW_ERROR);
	}
#endif
}

#endif

u16 getPreviousStackContext(void)
{
	u16		D_A		PreviousContext;
	u16		D_A		tempCc;
	PX8		D_A		top;
		
	top		= Top;
	tempCc 	= U16_AND_U16(cc, OBJ_HDR_GroupContext_MASK);

	do
	{
		PreviousContext	= PXS_R_U16(PX8_A_U8(top, 4));
		if(tempCc != U16_AND_U16(PreviousContext, OBJ_HDR_GroupContext_MASK))
		{
			break;
		}
		top				= PX8_A_U8(top, FRAME_CONTEXT_SIZE);
	} while (top != PX8_A_CONST(VMStack, STACKSIZE));
	return PreviousContext;
}

/*
 Direct access in the stack
 */
//jref fetchStackReference(u8 nth)
//{
//	return PXS_R_U16(PX8_S_U16(JSP, U8_M_U8(2, nth)));
//}

boolean isStackNotEmpty()
{
	return Top < PX8_A_CONST(VMStack,  STACKSIZE);
}

/**
 * Creates a new stack frame.
 *
 * @param m_paramSize number of params (in words)
 * @param m_localSize number of locals (in words)
 * @param m_maxstack  the maximum operands (in words) needed by the method
 *
 * @throws STACK_OVER_FLOW_ERROR (fatal)
 */
#if JCVM__VM__CHECK_STACK_POP
void pushFrame(u8 m_paramsize, u8 m_localsize, u8 m_maxstack)	SMALL_MODE
#else
void pushFrame(u8 m_paramsize, u8 m_localsize)
#endif
{
#if (LOCALS_ARE_FFFF || LOCALS_ARE_0000)
	u8		D_A		i;
#endif
	 /* composite temp to push the previous paramsize
	 and localsize global registers onto the stack.
	 This is necessary to handle the demands of the
	 'Athrow' instruction.
	 */
	PATCH_JCVM_VM_STACK_001();	
	FPsize		= U8_A_U8(m_paramsize, m_localsize);
	Top			= Top - FRAME_CONTEXT_SIZE;

	if (PX8_A_U8(JSP, U8_M_U8(2, U8_A_U8(m_maxstack, localsize))) <= Top)
	{
		/* push old pc */
		PXS_W_U32(&Top[8], PFtMEM(JPC));
		
		/* push current paramsize & localsize registers */
		/* push current context */
		PXS_W_U16(&Top[4], cc);
		PX_W_U8(&Top[6], localsize);
		PX_W_U8(&Top[7], paramsize);
		
		paramsize	= m_paramsize;
		localsize	= m_localsize;

		/* push old pkg id */
		PXS_W_U16(&Top[2], PCpkgID);

		/* push old FP (frame pointer) */
		PXS_W_U16(&Top[0], U16_S_U16(PXtMEM(FP), PXtMEM(VMStack)));
		FP			= PX8_S_U16(JSP, U8_M_U8(2, paramsize));

		if(FP >= FPend)
		{
#if LOCALS_ARE_FFFF
			for (i = 0; i < m_localsize; U8_INC(i))
			{
				/* pre-init non-parameter locals to 0xFFFF */
				pushShort(UNDEFINED_MEMORY_CELL);
			}
#elif LOCALS_ARE_0000
			for (i = 0; i < m_localsize;  U8_INC(i))
			{
				/* pre-init non-parameter locals to 0x0000*/
				pushShort(0);
			}
#else
			JSP			= PX8_A_U16(JSP, U8_M_U8(2, localsize));
			FPend		= JSP;
			return;
		}
		else
		{
			/* stack underflow */
			fatal_error(STACK_OVER_FLOW_ERROR);
		}
#endif
	}
	else
	{

		/* stack overflow */
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/**
 * Pops a stack frame.
 *
 * @throws STACK_UNDER_FLOW_ERROR (fatal)
 *
 * modifed by xxj@20210328: mask中包对象的个数不变，后下载的包在下载过程中创建的对象由5个(class_component / method_component / staticfield_component / export_component / pkg_info)减少为1个(pkg_info)
 *                          pkg_info中的信息，mask中包对象保持不变，存储的分别是4个组建引用(4字节) + 组建大小(2字节) ; 后下载的包对象发生变化，存储的分别是4个组建地址相对于pkg_info起始地址的偏移(4字节) + 组建大小(2字节)
 */
void popFrame()
{
	PF8			D_A		pkg_info_addr;
	u8			D_A		new_pkgID;

	/* composite temp to pop the previous paramsize and localsize global registers from the stack.
	 This is necessary to handle the demands of the 'Athrow' instruction.
	 */
	PATCH_JCVM_VM_STACK_002();

	if (Top <= MEMtPX8(&VMStack[STACKSIZE - FRAME_CONTEXT_SIZE]))
	{
		JSP			= FP; 

		/* pop old FP (frame pointer) */
		FP			= PX8_A_U16(VMStack, PXS_R_U16(&Top[0]));
		
		/* pop old pkg id */
		new_pkgID	= PXS_R_U16(&Top[2]);
		
		/* pop current paramsize & localsize registers */
		/* pop current context */
		cc			= PXS_R_U16(&Top[4]);
		localsize	= PX_R_U8(&Top[6]);
		paramsize	= PX_R_U8(&Top[7]);
		
		/* pop old pc */
		JPC			= MEMtPF8(PXS_R_U32(&Top[8]));

		Top 		= Top + FRAME_CONTEXT_SIZE;

		FPsize		= U8_A_U8(paramsize, localsize);
		FPend		= PX8_A_U8(FP, U8_M_U8(2, FPsize));
		if (new_pkgID != PCpkgID)
		{
			pkg_info_addr		= MEMtPF8(MapTable_GetEntityContent(PF_R_U16(PN8_A_U16(MEMtPN8(G_CurExecuEnvironment.pkgLocationTable + SIZE_U16), U8_M_U8(new_pkgID, SIZE_MEMREF)))));

			if(new_pkgID < NPACKAGES)
			{
				cachedClassCompLoc	= MapTable_GetEntityContent(PF_R_U16(PN8_A_U16(pkg_info_addr, (COMPONENT_CLASS * (SIZE_MEMREF + SIZE_U16) + SIZE_U16))));
				cachedMethodCompLoc = MapTable_GetEntityContent(PF_R_U16(PN8_A_U16(pkg_info_addr, (COMPONENT_METHOD * (SIZE_MEMREF + SIZE_U16) + SIZE_U16))));
				cachedFieldCompLoc	= MapTable_GetEntityContent(PF_R_U16(PN8_A_U16(pkg_info_addr, (COMPONENT_STATICFIELD * (SIZE_MEMREF + SIZE_U16) + SIZE_U16))));
			}
			else
			{
				cachedClassCompLoc	= PFtMEM(pkg_info_addr + PF_R_U32(PN8_A_U16(pkg_info_addr, (COMPONENT_CLASS * (SIZE_MEMREF + SIZE_U16)))));
				cachedMethodCompLoc = PFtMEM(pkg_info_addr + PF_R_U32(PN8_A_U16(pkg_info_addr, (COMPONENT_METHOD * (SIZE_MEMREF + SIZE_U16)))));
				cachedFieldCompLoc	= PFtMEM(pkg_info_addr + PF_R_U32(PN8_A_U16(pkg_info_addr, (COMPONENT_STATICFIELD * (SIZE_MEMREF + SIZE_U16)))));
			}
			PCpkgID				= new_pkgID;
			return;
		}
	}
	else
	{
		/* stack underflow */
		fatal_error(STACK_UNDER_FLOW_ERROR);
	}
}

#if		BOOT__MEMORY__NVM_RESIZE
u8 JCstack_GetUpdatePtrNum(void)
{
	u8		D_A		i;
	PX8		D_A		top;

	i		= 0;
	top		= Top;

	do
	{
		U8_INC(i);
		top				= PX8_A_U8(top, FRAME_CONTEXT_SIZE);
	} while (top != PX8_A_CONST(MEMtPX8(VMStack), STACKSIZE));

	i		= U8_A_U8(i, GLOBABL_VM_PTR_NUM);
	U8_DEC(i);

	return i;
}

memref JCstack_UpdatePtr(u8 i, u32 changeLen, u1 isIncrease, u1 isUpdatePtr)
{
	u16		D_A		curFrameNum;
	u16		D_A		curFrameJpcOffset;
	PX8		D_A		top;
	PR8		D_A		ptrAddress;
	PM8		D_A		ptrValue;

	switch(i)
	{
		case JPC_PTR:
			ptrValue				= MEMtPM8(JPC);
			ptrAddress				= MEMtPR8(&JPC);
			break;
		case CACHED_CLASS_PTR:
			ptrValue				= MEMtPM8(cachedClassCompLoc);
			ptrAddress				= MEMtPR8(&cachedClassCompLoc);
			break;
		case CACHED_METHOD_PTR:
			ptrValue				= MEMtPM8(cachedMethodCompLoc);
			ptrAddress				= MEMtPR8(&cachedClassCompLoc);
			break;
		case CACHED_STATIC_FIELD_PTR:
			ptrValue				= MEMtPM8(cachedFieldCompLoc);
			ptrAddress				= MEMtPR8(&cachedClassCompLoc);
			break;
		default:
			curFrameNum				= U8_S_U8(i, GLOBABL_VM_PTR_NUM);
			curFrameJpcOffset		= U16_A_U16(U16_M_U16(curFrameNum, FRAME_CONTEXT_SIZE), STACK_JPC_SAVE_OFFSET);
			top						= PX8_A_U16(Top, curFrameJpcOffset);
			ptrValue				= MEMtPM8(PX_R_U32(top));
			ptrAddress				= MEMtPR8(top);
			break;
	}

	if(true == isUpdatePtr)
	{
		if(false == isIncrease)
		{
			ptrValue					= PM8_A_U32(ptrValue, changeLen);
		}
		else
		{
			ptrValue					= PM8_S_U32(ptrValue, changeLen);
		}

		PR_W_U32(ptrAddress, PMtMEM(ptrValue));
	}

	return PMtMEM(ptrValue);
}
#endif

void BackCurrentJavaPoint(void)
{
	if(BackTop == NULL)
	{
		BackTop = Top;
		BackFP  = FP;
	}
}
