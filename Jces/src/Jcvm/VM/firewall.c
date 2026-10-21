/*****************************************************************************
 * File:  bc_array.c
 * Version 1.10

 wang hui
 eastcompeace Technology Corporation

 History:
 Last modified on 07/07/03 at 13:23:48
 00/12/04 at 13:23:48 .Created

 ****************************************************************************/

#include "firewall.h"
#include "profile.h"
#include "object.h"
#include "stack.h"
#include "context.h"
#include "memory.h"
#include "anomaly.h"
#include "chn_manager.h"
#include "ob_manager.h"
#include "global_variable.h"

#if JCVM__VM__FIRE_WALL

/****************************************************************************
 * The FireWall module implements all of the security policies of the Java
 * Card 2.1 Virtual Machine.
 * @module FireWall
 *****************************************************************************/

void checkDTRAccess(PF8 arrayAddr)			SMALL_MODE;

/******************************************************************************
 * Checks the common read access in the Firewall.
 * 2 rules are checked :
 * <ul>
 * <li> If the JCRE is the currently active context, then access is allowed.
 * <li> If the object is owned by an applet in the currently active context, then access is allowed.
 * </ul>
 * otherwise, access is denied : a security exception is thrown.<br>
 *
 * Used by instructions :
 * <ul>
 * <li> checkcast (non shareable interface)
 * <li> instanceof (non shareable interface)
 * <li> invokeinterface (non shareable interface)
 * </ul>
 *
 * @param objRef the accessed object reference
 *****************************************************************************/
#if JCVM__VM__FIRE_WALL

void checkRead(PF8 objAddr)
{

	if (IS_OBJECT_CONTEXT_CURRENT(objAddr) || IS_JCRE_CURRENT_CONTEXT())
	{
		return;
	}

	throw_error(SECURITY_EXCEPTION);
}

/******************************************************************************
 * Checks the common invoke access in the Firewall.
 * 3 rules are checked :
 * <ul>
 * <li> If the object is designated a JCRE Entry Point Object.
 * <li> If the JCRE is the currently active context, then access is allowed.
 * <li> If the object is owned by an applet in the currently active context, then access is allowed.
 * </ul>
 * otherwise, access is denied : a security exception is thrown.<br>
 *
 * Used the instructions :
 * <ul>
 * <li> invokevirtual (non array object)
 * <li> athrow
 * </ul>
 *
 * @param objRef the accessed object reference
 *****************************************************************************/
void checkEntryPoint(PF8 objAddr)
{
	if (IS_ENTRY_OBJ(objAddr))
	{
		return;
	}


#if JCVM__VM__FIRE_WALL
	checkRead(objAddr);
#endif

}

/******************************************************************************
 * Checks storing access to object field.
 * 3 rules are checked :
 * <ul>
 * <li> If the JCRE is the currently active context, then access is allowed.
 * <li> If field being storing is a reference type and the reference being stored is a
 *      reference to a temporary JCRE Entry Point or a Global array, then access is denied.
 * <li> If the object is owned by an applet in the currently active context, then access is allowed.
 * </ul>
 * otherwise, access is denied : a security exception is thrown.<br>
 *
 * Used only by the instruction :
 *
 * putfield_a
 *
 * @param objRef the accessed object reference
 * @param value the value reference to store.
 *****************************************************************************/
void checkFieldStore(PF8 objAddr, jref value)	SMALL_MODE
{

	PF8		D_A	valueaddr;

	if (IS_JCRE_CURRENT_CONTEXT())
		return;

	valueaddr	= MEMtPF8(locateObjectInMemory(value));

	if (((value != REF_NULL) && (IS_TEMP_ENTRY_OBJ(valueaddr) || IS_GLOBAL_ARRAY(valueaddr))) || !IS_OBJECT_CONTEXT_CURRENT(objAddr))
	{
		throw_error(SECURITY_EXCEPTION);
	}
}

/******************************************************************************
 * Checks storing access to array.
 * 4 rules are checked :
 * <ul>
 * <li> If the JCRE is the currently active context, then access is allowed.
 * <li> If component being storing is a reference type and the reference being stored is a
 *      reference to a temporary JCRE Entry Point or a Global array, then access is denied.
 * <li> If the array is owned by an applet in the currently active context, then access is allowed.
 * <li> If the array is designated global, then access is allowed.
 * </ul>
 * otherwise, access is denied : a security exception is thrown.<br>
 *
 * Used only by the instruction :
 *
 * aastore
 *
 * @param arrayRef the accessed array reference
 * @param value the value reference to store.
 *****************************************************************************/
void checkArrayStore(PF8 arrayAddr, jref value)		SMALL_MODE
{

	PF8		D_A		valueaddr;

	if (IS_JCRE_CURRENT_CONTEXT())
		return;

	if (IS_ARRAY_DTR(arrayAddr))
#if JCVM__VM__FIRE_WALL
		checkDTRAccess(arrayAddr);
#endif

	valueaddr = MEMtPF8(locateObjectInMemory(value));

	if (((value != REF_NULL) && (IS_TEMP_ENTRY_OBJ(valueaddr) || IS_GLOBAL_ARRAY(valueaddr))) || (!IS_OBJECT_CONTEXT_CURRENT(arrayAddr) && !IS_GLOBAL_ARRAY(arrayAddr)))
	{
		if(!IS_ENTRY_OBJ(arrayAddr))
		{	
			throw_error(SECURITY_EXCEPTION);
		}
	}

}

/**v****************************************************************************
 * Checks storing access to object static field.
 * 2 rules are checked :
 * <ul>
 * <li> If the JCRE is the currently active context, then access is allowed.
 * <li> If field being storing is a reference type and the reference being stored is a
 *      reference to a temporary JCRE Entry Point or a Global array, then access is denied.
 * </ul>
 * otherwise, access is denied : a security exception is thrown.<br>
 *
 * Used only by the instruction :
 *
 * putstatic_a
 *
 * @param objRef the object reference being stored.
 *****************************************************************************/
void checkStaticStore(PF8 objAddr)		SMALL_MODE
{
	u8		D_A		objType;

	if (IS_JCRE_CURRENT_CONTEXT())
		return;
	if (!check_addr_null(objAddr))
	{
		objType = get_object_header_type(objAddr);
        /*sensitivearray标志与JCRE state共用一个bit位*/
		if ((U8_AND_U8(objType, U8_OR_U8(OBJ_HDR_JCRE_EntryP_MASK, OBJ_HDR_JCRE_State_MASK)) == HEADER_JCRE_EntryPoint_Yes)
			/*|| (U8_AND_U8(objType, U8_OR_U8(OBJ_HDR_JCRE_EntryP_MASK, OBJ_HDR_GLOBAL_FLAG_MASK)) == HEADER_Global_Shared)*/)
		{
			throw_error(SECURITY_EXCEPTION);
		}
	}
}

/**v****************************************************************************
 * Checks access to shareable object.
 * 3 rules are checked :
 * <ul>
 * <li> If the JCRE is the currently active context, then access is allowed.
 * <li> If the object's class implements a Shareable interface, then access is allowed.
 * <li> If the object is owned by an applet in the currently active context, then access is allowed.
 * </ul>
 * otherwise, access is denied : a security exception is thrown.<br>
 *
 * Used the instructions :
 * <ul>
 * <li> invokeinterface (shareable interface)
 * <li> checkcast (shareable interface)
 * <li> instanceof (shareable interface)
 * </ul>
 *
 * @param objRef the accessed object reference
 * @param value the value reference to store.
 *****************************************************************************/
void checkShareable(PF8 objAddr)		SMALL_MODE
{
	if (IS_SHAREABLE_OBJECT(objAddr))
		return;


#if JCVM__VM__FIRE_WALL
	checkRead(objAddr);
#endif
}

/******************************************************************************
 * Checks access to shareable objects across multiple logical channels.
 *
 * Used the instructions :
 * <ul>
 * <li> invokeinterface (shareable interface)
 * </ul>
 *
 * @param objRef the accessed object reference
 *****************************************************************************/

void checkChannelAccess(PF8 objAddr)
{

	u16 contextId;

	contextId = get_object_owner(objAddr);
	if(contextId == JCRE_CONTEXT)//场景：0通道选择MNO-SD ，1通道下载profile 安装ISD-P时 OPPlatform.theCardAgent.opCommand，调用invokeinterface ，获取到 OPPlatform.theCardAgent的owner是JCRE（0），与通道0上的上下文一致，会检查contextId= 0的应用是否支持多选，此时访问的区域G_CurExecutingProfileIndex是正在下载的profile，还未创建MNOSD
	{
		return;//用例：eSIM_Test_additional/方案评审补充测试/During_Load_Profile_break_in_LoadCap_Test_01
	}
	if(IS_ENTRY_OBJ(objAddr))
	{
		return;
	}

	if (checkShareableObjectAccess(contextId))
		return;

	throw_error(SECURITY_EXCEPTION);
}

/******************************************************************************
 * Checks read access to array.
 * 3 rules are checked :
 * <ul>
 * <li> If the JCRE is the currently active context, then access is allowed.
 * <li> If the array is owned by an applet in the currently active context, then access is allowed.
 * <li> If the array is designated global, then access is allowed.
 * </ul>
 * otherwise, access is denied : a security exception is thrown.<br>
 *
 * Used the instructions :
 * <ul>
 * <li> arraylength
 * <li> xaload
 * <li> xastore (x != a)
 * <li> checkcast (array)
 * <li> instanceof (array)
 * <li> invokevirtual (array method)
 * </ul>
 *
 * @param arrayRef the accessed array reference
 *****************************************************************************/
//#if !EMV_OPTIMIZATION_CODE
WEAK void checkArrayRead(PF8 arrayAddr)		SMALL_MODE
{
	// modified by wh 2010.2.22 , this method can reduce read eeprom times.
	u8		D_A	objectType;

	objectType = get_object_header_type(arrayAddr);

	if (U8_AND_U8(objectType, U8_OR_U8(OBJ_HDR_JCRE_EntryP_MASK, OBJ_HDR_GLOBAL_FLAG_MASK)) != 0)
	{
		return;
	}

	if (U8_AND_U8(objectType, OBJ_HDR_TransientMode_MASK) == HEADER_JCRE_TransMode_Deselect)
	{
#if JCVM__VM__FIRE_WALL
		checkDTRAccess(arrayAddr);
#endif
	}


#if JCVM__VM__FIRE_WALL
	checkRead(arrayAddr);
#endif
}
//#endif //!EMV_OPTIMIZATION_CODE

/******************************************************************************
 * Checks access to class instance object.
 * 3 rules are checked :
 * <ul>
 * <li> If the object is designated a JCRE Entry Point Object.
 * <li> If the JCRE is the currently active context, then access is allowed.
 *      (only for class access)
 * <li> If the object is owned by an applet in the currently active context, then access is allowed.
 * </ul>
 * otherwise, access is denied : a security exception is thrown.<br>
 *
 * Used the instructions :
 * <ul>
 * <li> checkcast (Class)
 * <li> instanceof (Class)
 * </ul>
 *
 * @param arrayRef the accessed array reference
 *****************************************************************************/
void checkInstanceAndCast(PF8 objAddr, PF8 classAddr)		SMALL_MODE
{
	if (IS_REF_SIMPLEOBJECT(objAddr))
	{
		if (IS_INTERFACE(classAddr))
		{
			if (IS_SHAREABLE(classAddr))
			{
				/* class is an interface shareable */
#if JCVM__VM__FIRE_WALL
				checkShareable(objAddr);
#endif
				return;
			}

			/* class is an interface not shareable */
#if JCVM__VM__FIRE_WALL
//			checkRead(objAddr);
			checkEntryPoint(objAddr); //测试usimAPITest_Api_2_Ues_Gthd (instanceof)

#endif
			return;
		}

		/* class is a class object */
#if JCVM__VM__FIRE_WALL
		checkEntryPoint(objAddr);
#endif

		return;
	}


	/* objectref is an array reference */
#if JCVM__VM__FIRE_WALL
	checkArrayRead(objAddr);
#endif
}

void checkDTRCreation()
{
	if (!IS_SELECTED_CONTEXT() && !IS_JCRE_CURRENT_CONTEXT())
	{
		throw_error(SYSTEMEXCEPT_ILLEGAL_TRANSIENT);
	}
}

/******************************************************************************
 * Transient objects of CLEAR_ON_DESELECT type can only be access when the
 * currently active context is the context of the currently selected applet.
 * Except if the current context is the JCRE context.
 *****************************************************************************/
void checkDTRAccess(PF8 arrayAddr)	SMALL_MODE
{
	if (IS_OBJECT_CONTEXT_SELECTED(arrayAddr) && (IS_SELECTED_CONTEXT() || IS_JCRE_CURRENT_CONTEXT()))
	{
		return;
	}
	throw_error(SECURITY_EXCEPTION);
}

void checkPrevContextAccess(PF8 objectAddr)		SMALL_MODE
{
	if (IS_JCRE_CURRENT_CONTEXT())
	{
		u16		D_A		previousContext;
		u16		D_A		objectContext;

		previousContext	= U16_AND_U16(getPreviousStackContext(), OBJ_HDR_GroupContext_MASK);
		objectContext	= U16_AND_U16(get_object_owner(objectAddr), OBJ_HDR_GroupContext_MASK);

		if ((objectContext == previousContext) || (previousContext == JCRE_CONTEXT) || IS_ENTRY_OBJ(objectAddr) || IS_GLOBAL_ARRAY(objectAddr))
		{
			return;
		}
		else
		{
			throw_error(SECURITY_EXCEPTION);
		}
	}
}

void checkAPDUObjectAccess(void)
{
	u16 context;
	u8  contextId;

	context		= PX_R_U16(&channelInfo.selected_context[U8_M_U8((selected_channel),2)]);
	contextId	= U16t8(U16_AND_U16(context, APPLET_MASK));

	if ((contextId != U16t8(U16_AND_U16(cc, APPLET_MASK))) || (PXB_R_U8(&bRamBuffer[PROCESS_METHOD_FLAG]) != 0x01))
	{
		throw_error(SECURITY_EXCEPTION);
	}
}
#endif
#endif
