#include "nativemethod_impl.h"
#include "nativemethod_impl_JavaCard305.h"
#include "lifecycle.h"
#include "profile.h"
#include "context.h"
#include "object.h"
#include "firewall.h"
#include "except.h"
#include "mask.h"
#include "memory.h"
#include "stack.h"
#include "anomaly.h"
#include "method.h"
#include "memory.h"
#include "except.h"
#include "bc_array.h"
#include "gc.h"
#include "temp_ob_manager.h"
#include "packageloc.h"
#include "reference.h"
#include "ob_manager.h"
#include "chn_manager.h"
#include "contentManager.h"
/*#include "assert.h" */
#include "Apdu.h"
#include "Io.h"
#include "utilMethod.h"
#include "global_variable.h"
#include "cardManager.h"
#include "RamManager.h"
#include "rtrSpace.h"
#include "tranAllocate.h"
#include "AppAPI.h"
#include "JcvmConfig.h"
#include "Mifare.h"
#include "opFunction.h"
#include "target.h"
#include "memOperationServe.h"
#include "NvmMap.h"
#include "JcvmDataConfig.h"
#include "CgmManager.h"
#include "jcvmConfigMethod.h"

#if GP__AMENDMENT__H__FOR_MASK
#include "OPELFUpgrade.h"
#endif

#include "Boot.h"
#include "Interface_LTEFromPlatform.h"
#include "SEEUICCInterface.h"
#if  JCVM__VM__JAVACARDX_FRAMEWORK_UTIL
#include "util_nm.h"
#endif
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 


#if GP__HCI_API__FOR_MASK
#include "Swp_Hci_Api.h"
#endif

//#include "VMDebug.h"
/* Constants */
#define NOT_A_TRANSIENT_OBJECT          0x00
#define CLEAR_ON_RESET                  0x01
#define CLEAR_ON_DESELECT               0x02
#define INSTALL_METHOD_ADDRESS          0x01
#define PACKAGE_ID                      0x02
#define APPLET_INDEX                    0x03
#define INSTALL_METHOD_ADDRESS          0x01

//modify by xuxiaojun@20221020: 根据 合代码时代码评审(20220722).xlsx进行修改： 将JAVA CARD305的本地方法放在nativemethod_impl_JavaCard305中；javaUicc的本地方法放在javaUiccAPI中


/**
 * public static native u8 getMaxPackageIdentifier();
 * ----------------------------------------------------
 * This method returns the highest package identifier allocated to the packages
 * in the ROM mask. Package ids are used for context assignment. This method is used
 * by the Installer to manage its Package management tables and for
 * package id assignment for installed packages. Package ids are used as contexts.
 * @return the highest package identifier in ROM.
 */
void getMaxPackageIdentifier()
{
	pushShort(U16_S_U8(NPACKAGES, 1));
}

void GetContentConfigValue(u8 tag)
{
	#define GET_MAX_PKG			1
	#define GET_MAX_APP_PKG		2
	#define GET_APP_END			3
	u16 result;
	result = 0;
	switch(tag)
	{
		case  GET_MAX_PKG:
			result = ON_CARD_PKG_MAX;
		  break;
		case  GET_MAX_APP_PKG:
			result = ON_CARD_APPLET_PKG_MAX;
		  break;
		case  GET_APP_END:
			result = APP_END;
		  break;
		default:
					break;
	}
	pushShort(result);
}
/* used to free allocated objects and return memory back to memory pool
 * @param address the address of the memory to be freed
 * @param length the length of the memory to be freed
 */
// void freeHeap()
// {
// 	u16			D_A		length;
// 	PN8			D_A		address;

// 	length		= popShort();
// 	address		= MEMtPN8(popAddress());

// 	//E2P_free(U32t16(PNtMEM(address)), length);
// 	//NvmManager_Free(address, MAP_ENTRY_BLOCK_PCK_ID);
// }

/* the next three methods will return the amount of memory left in the three memory pools
 * E2P_Available returns the amount of available E2P memory
 * rtr_Available() returns the amount of clear on reset transient memory available
 * dtr_Available() returns the amount of clear on deselect transient memory available
 */
u16 getAvailableMemory()
{
	u32		D_A    availableEEP;
    u16     D_A    curAvaiEep;
#if JCVM__RAM__DTR__RTR__NEW	
	u16     D_A    backChangeCC;
	backChangeCC = cc;
#endif
	curAvaiEep = 0x00;
	switch(popShort())
	{
	case MEMORY_TYPE_PERSISTENT:
	#if GP__CGM__FOR_MASK
		if(g_bCheckFlag != CHECK_CLOSE)
		{
			availableEEP = CgmField_getSpaceInfo(U32t8(APP_FIRST), U32t8(SD_CGM_REMAIN_NVM_SIZE));
		}
		else
		{
			availableEEP = NvmManager_GetAvailMemory();
		}
	#else
		availableEEP = NvmManager_GetAvailMemory();
	#endif
		if (availableEEP > 0x7FFF)
		{
			curAvaiEep = 0x7FFF;
		}
		else
		{
			curAvaiEep = U32t16(availableEEP);
		}
		break;
#if JCVM__RAM__DTR__RTR__NEW		
		case MEMORY_TYPE_TRANSIENT_RESET:
#if GP__CGM__FOR_MASK
		if (g_bCheckFlag == CHECK_CLOSE)
		{
			curAvaiEep = tranAllocate_rtrAvailable();
		}
		else
		{
			curAvaiEep = CgmField_getSpaceInfo(0, SD_CGM_REMAIN_RAM_SIZE) - RamManager_getUsedDtrSize();

			if(cc != g_backupCC)
			{
				backChangeCC = g_backupCC;
				CgmRun_ChangeCC(cc);
			}

			if(g_sD7Value > g_UsedRAM)
			{
				curAvaiEep += (g_sD7Value - g_UsedRAM);
			}
			if(curAvaiEep > g_sC7Value - g_UsedRAM)
			{
				curAvaiEep = g_sC7Value - g_UsedRAM;
			}
			if(cc != backChangeCC)
			{
				CgmRun_ChangeCC(backChangeCC);
			}

		}
#else
		curAvaiEep = U32t16(tranAllocate_rtrAvailable());
#endif
		break;
    case MEMORY_TYPE_TRANSIENT_DESELECT:
		curAvaiEep = tranAllocate_dtrAvailable(GET_PKGID(cc));

		//按照中国电信DTR RTR测试脚本，DTR空间要加上D7预留空间中未实际使用的空间,by jiangxiaohui 20220516
		if(cc != g_backupCC)
		{
			backChangeCC = g_backupCC;
			CgmRun_ChangeCC(cc);
		}
		if(g_sD7Value > g_UsedRAM)
		{
			curAvaiEep += (g_sD7Value - g_UsedRAM);
		}
		if(cc != backChangeCC)
		{
			CgmRun_ChangeCC(backChangeCC);
		}
		break;
#else		
	case MEMORY_TYPE_TRANSIENT_RESET:
    case MEMORY_TYPE_TRANSIENT_DESELECT:
#if GP__CGM__FOR_MASK
		if (g_bCheckFlag == CHECK_CLOSE)
		{
			curAvaiEep = tranAllocate_rtrAvailable();
		}
		else
		{
			curAvaiEep = CgmField_getSpaceInfo(0, SD_CGM_REMAIN_RAM_SIZE) - RamManager_getUsedDtrSize();
		}
#else
		curAvaiEep = U32t16(tranAllocate_rtrAvailable());
#endif
		break;
#endif		
    default:
		throw_error(SYSTEMEXCEPT_ILLEGAL_VALUE);
		break;
    }
	return curAvaiEep;
}
/**
     * Obtains the amount of memory of the specified type that is available to the applet. Note that implementation-dependent
     * memory overhead structures may also use the same memory pool. The requested memory information is returned as a 32 bit number
     * stored in the specified short array - buffer. The 32 bit number number is the concatenation of buffer[offset] and buffer[offset+1].
     * Notes:
     * The number of bytes returned is only an upper bound on the amount of memory available due to overhead requirements.
     * Allocation of CLEAR_ON_RESET transient objects may affect the amount of CLEAR_ON_DESELECT transient memory available.
     * Allocation of CLEAR_ON_DESELECT transient objects may affect the amount of CLEAR_ON_RESET transient memory available.
     * The 32 bit number is not an indicator of the size of object which may be created since memory fragmentation is possible.

     * @param buffer - the output buffer for storing memory size information
     * @param offset - the offset within the buffer where memory size information begins
     * @param memoryType - the type of memory being queried. One of the MEMORY_TYPE_* constants defined above. See MEMORY_TYPE_PERSISTENT.
     * @throws SystemException - with the following reason codes:SystemException.ILLEGAL_VALUE if memoryType is not a valid memory type.
     * @throws ArrayIndexOutOfBoundsException - if buffer[offset] or buffer[offset+1] outside array bounds
     * @throws NullPointerException - if buffer is null
     */
void getAvailableMemoryBuf(void)
{
	u8		D_A	   memoryType;
	u32     D_A    curAvaiEep;
	u16		D_A    bufOffest;
	PM8   		   bufAdd;
#if JCVM__RAM__DTR__RTR__NEW	
	u16     D_A    backChangeCC;
	backChangeCC = cc;
#endif

	memoryType	= U16t8(popShort());
	bufOffest   = popShort();
	bufAdd		= getJavaObjectDataRef();

	curAvaiEep  = 0x00;
	check_null(bufAdd);
	if(bufOffest > get_array_length(bufAdd))
	{
		throw_error(ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION);
	}
	bufAdd      = PM8_A_U16(MEMtPM8(get_array_data((PF8)bufAdd)), U16_M_U16(bufOffest, sizeof(u16)));
	switch(memoryType)
	{
	case MEMORY_TYPE_PERSISTENT:
	#if GP__CGM__FOR_MASK
		if(g_bCheckFlag != CHECK_CLOSE)
		{
			curAvaiEep = CgmField_getSpaceInfo(U32t8(APP_FIRST), U32t8(SD_CGM_REMAIN_NVM_SIZE));
		}
		else
		{
			curAvaiEep = NvmManager_GetAvailMemory();
		}
	#else
		curAvaiEep = NvmManager_GetAvailMemory();
	#endif

		break;
#if JCVM__RAM__DTR__RTR__NEW		
		case MEMORY_TYPE_TRANSIENT_RESET:
#if GP__CGM__FOR_MASK
		if (g_bCheckFlag == CHECK_CLOSE)
		{
			curAvaiEep = tranAllocate_rtrAvailable();
		}
		else
		{
			curAvaiEep = CgmField_getSpaceInfo(0, SD_CGM_REMAIN_RAM_SIZE) - RamManager_getUsedDtrSize();

			if(cc != g_backupCC)
			{
				backChangeCC = g_backupCC;
				CgmRun_ChangeCC(cc);
			}

			if(g_sD7Value > g_UsedRAM)
			{
				curAvaiEep += (g_sD7Value - g_UsedRAM);
			}
			if(curAvaiEep > g_sC7Value - g_UsedRAM)
			{
				curAvaiEep = g_sC7Value - g_UsedRAM;
			}
			if(cc != backChangeCC)
			{
				CgmRun_ChangeCC(backChangeCC);
			}

		}
#else
		curAvaiEep = U32t16(tranAllocate_rtrAvailable());
#endif
		break;
    case MEMORY_TYPE_TRANSIENT_DESELECT:
		curAvaiEep = tranAllocate_dtrAvailable(GET_PKGID(cc));

		//按照中国电信DTR RTR测试脚本，DTR空间要加上D7预留空间中未实际使用的空间,by jiangxiaohui 20220516
		if(cc != g_backupCC)
		{
			backChangeCC = g_backupCC;
			CgmRun_ChangeCC(cc);
		}
		if(g_sD7Value > g_UsedRAM)
		{
			curAvaiEep += (g_sD7Value - g_UsedRAM);
		}
		if(cc != backChangeCC)
		{
			CgmRun_ChangeCC(backChangeCC);
		}
		break;
#else		
	case MEMORY_TYPE_TRANSIENT_RESET:
    case MEMORY_TYPE_TRANSIENT_DESELECT:
#if GP__CGM__FOR_MASK
		if (g_bCheckFlag == CHECK_CLOSE)
		{
			curAvaiEep = tranAllocate_rtrAvailable();
		}
		else
		{
			curAvaiEep = CgmField_getSpaceInfo(0, SD_CGM_REMAIN_RAM_SIZE) - RamManager_getUsedDtrSize();
		}
#else
		curAvaiEep = U32t16(tranAllocate_rtrAvailable());
#endif
		break;
#endif		
    default:
		throw_error(SYSTEMEXCEPT_ILLEGAL_VALUE);
		break;
    }

	memWriteU32Atomic(bufAdd, curAvaiEep);
}

void callInstall()
{
	u8		D_A		pkgId;
	u8		D_A		bLen;
	u16		D_A		bOffset;
	jref	D_A		bArray;
	u16		D_A		context;
	jref	D_A		addrref;
	PF8		D_A		address; 
	PN8		D_A		PkgContexTable;

	bLen	= U16t8(popShort());
	bOffset = popShort();
	bArray	= popReference();
	context = popShort();
	addrref = popReference();

	 // set flag of load and vm
	PXB_W_U8(&bRamBuffer[INSTALL_TRANSACTION_FLAG], 1);
	PXB_W_U8(&bRamBuffer[PROCESS_METHOD_FLAG], 0x00);
	PXB_W_U8(&bRamBuffer[APP_IN_PROCESS], U16t8(context));
	/* pass the parameters to the method we are about to call */
	/*@ -disspecopt @*/
	PkgContexTable						= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(nvmReadU16(G_CurExecuEnvironment.packageContextTable)))));
	/*@ +disspecopt @*/
	pkgId								= memReadU8(PN8_A_U8(PkgContexTable, U16_SHR_U8(context, 8)));

	pushShort(bArray);
	pushShort(bOffset);
	pushShort(U8t16(bLen));
	address 							= MEMtPF8(resolveReferenceAddress(addrref, pkgId, COMPONENT_METHOD));
	/* do a callmethod on method located at 'address' with the params in installParms */
	callmethod(address, pkgId, context);
}
/**
*added by jiangxiaohui:优化性能, EPX产业化/THC_THD89/Branch/EPX_THC_Mobile5G/Develop
*/
void initAIDImpl()
{
	u16		D_A		length;
	u16		D_A		srcOff;
	PN8		D_A		src;
	jref	D_A		aidRef;

	length	= popShort();
	srcOff	= popShort();
	src		= MEMtPN8(locateObjectInMemory(popReference()));

	/* Check for src  being 'null' */
	check_null(src);
	/* check array bounds */
	check_bounds_interval(src, srcOff, length);
	aidRef	= initAID(MEMtPN8(get_array_data(src)), srcOff, length);
	memRefresh();
	pushReference(aidRef);
}
// added by xuxiaojun@20220621 用于判断安装的实例是否来源于可执行模块SecurityDomain
void isSecurityDomain(void)
{
	u8		pkgID;

	pkgID	= U16t8(popShort());

	if(SSD_PkgId == pkgID)
	{
		pushShort(0x01);
	}
	else
	{
		pushShort(0x00);
	}

}
void resolveE2C4commandNM(void)
{
	u16		resolveResult;
	u8		type;  //(lzm modified according [MISA])
	
	type	= U16t8(popShort());  //(lzm modified according [MISA])	  //wzq:   type	= U16t8(popShort());

	switch(type)
	{
		case RESOLVE:
			resolveResult	= resolveE2C4command();
			if(resolveResult != SW_NO_ERROR)
			{
				setLoaderException(resolveResult);
			}
			pushShort(U8t16(g_resolvResult));
			break;
		case GET_TAG:
			pushShort(U8t16(g_E2C4tag));
			break;
		case GET_SD_LEN:
			pushShort(U8t16(g_aidLength));
			break;
		case GET_SIGDATA_LEN:
			pushShort(U8t16(g_SigDataSize));
			break;
		case INIT_OFFSET:
			g_offset = OFFSET_CDATA;
			pushShort(0);
			break;
		default:
			pushShort(0);
			break;
	}
}
/**
 * public static native  void setChannelContext ( u8 channelId,
 *     u8 target_context, u8 bMultiSelect );
 * -----------------------------------------------------------------
 * This method is called to set the current selected applet's contextId, on the
 * specified channel.
 * @param channelId channel on which to perform the context switch
 * @param target_context the contextId of selected applet
 * @param bMultiSelect if this context to be set has the MultiSelectable flag enabled.
 * @return true if context switch possible, false if no context switch possible.
 */
void setChannelContextNM()
{
    u8			D_A  channelId;
    jboolean	D_A  bChannelClose;
	u16			D_A  target_context;

    bChannelClose	= U16t8(popShort());		//wzq:  bChannelClose = U16t8(popShort());

//	popShort();//bMultiSelect = (jboolean)popShort();
	target_context	= popShort();
//	popShort();//channelId = (u8)popShort();
    channelId		= U16t8(popShort());	

	setChannelContext(channelId, target_context, bChannelClose);
	pushShort(1);
}

/**
 * public static native  u8 getChannelContext (u8 channelId);
 * -------------------------------------------------
 * This method returns the current selected applet's contextId on the selected
 * logical channel.
 */
void getChannelContextNM()
{						   
    u8  channelId;

    channelId	= U16t8(popShort());		 //wzq: 	channelId	= U16t8(popShort());
    pushShort(getChannelContext(channelId));

//	popShort();//channelId = (u8)popShort();

//	pushShort(selected_context);
}

/**
 * This method checks if access is to be allowed to the object being passed to it from
 * the previous context. This method just returns if the access to the object from the
 * previous context (this can be if the previous context is either JCRE or object owner's
 * context) is okay, otherwise it throws a security exception.
 * It is assumed that the object being passed to this method is not null and proper
 * precautions have been taken regarding this before this method is called. This method's
 * behaviour is non-deterministic if the object passed to it is null.
 * @param theObject for which it is to be determined if the access from the previous context
 * should be allowed.
 */

void checkPreviousContextAccessNM()
{
    u16		D_A    type;
	PF8		D_A    theObject;

	type		= popShort();
	theObject	= MEMtPF8(locateObjectInMemory(popShort()));
#if JCVM__VM__NULLOBJ_CHECK
    check_null((PF8)theObject); // merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP:  collis704中用例 gpapi_testApplet_18_CVM_update_nullBuffer_de_20_84
#endif

	if(PRE_CONTEXT_VALUE == checkPreviousContextAccess(theObject, type))
	{/*@ -discast @*/
		/*@ -typedefparam @*/
		/*@ -type @*/
		pushShort((jshort)1);
		/*@ +type @*/
		/*@ +typedefparam @*/
	}/*@ +discast @*/
}

jref checkPreviousContextAccess(PF8 theObject, u16 type)
{
	//chech the globality of baBuffer[], modified by Ding 20130417
	if(type == 1)
	{/*@ -discast @*/
		//pushShort((jshort)IS_GLOBAL_ARRAY(theObject));
		//if((jshort)IS_ENTRY_OBJ(theObject))
		if((jshort)IS_GLOBAL_ARRAY(theObject))
		{
			;
		}
		else
		{
			throw_error(SECURITY_EXCEPTION);
		}	
	}
	else if(type == 2)
	{
		if((jshort)IS_ENTRY_OBJ(theObject))
		{
			;
		}
		else
		{
			throw_error(SECURITY_EXCEPTION);
		}		
    }/*@ +discast @*/
//根据评审意见修改，此为305用，modify by dengyiwen@20220111
#if WITH_JC305
	else if(type == 3)
	{
		if(getPreviousStackContext() != get_object_owner(theObject))
		{
			throw_error(SECURITY_EXCEPTION);
		}		
    }/*@ +discast @*/
#endif
	else
	{	
#if JCVM__VM__FIRE_WALL
	checkPrevContextAccess(theObject);
	//pushShort(1);
#endif
	}
	return PRE_CONTEXT_VALUE;
}
/**
 * public static native  void setJCREentry(Object theObject, boolean temporary);
 * -----------------------------------------------------------------------------
 * This method is called to mark the specified object as being a JCRE entry point object.
 * JCRE entry point objects' methods are accessible from any applet context. In addition,
 * the the JCRE Entry point object is marked as temporary or permanent based on the
 * boolean parameter. User applets cannot store temporary Entry Point Objects in
 * class variables or array components. This method is also used for global arrays.
 * @param theObject the object to be marked as a JCRE Entry point object
 * @param temporary <code>/<code> if the JCRE Entry point object is a
 * temporary JCRE EP object.
 */
void setJCREentry()
{
	boolean	    D_A    isTemporary;
	jref	    D_A    objref;
	PN8		    D_A    objrefaddr;

	isTemporary	= U16t8(popShort()); //wzq
	objref		= popReference();

	objrefaddr	= MEMtPN8(locateObjectInMemory(objref));
	set_JCRE_entry(objrefaddr, isTemporary);
}

void setPckObject(void)
{
	jref		D_A		objref;
	u16			D_A		objCC;
	PN8			D_A		tempAddr;
	
	objref		= popReference();

	tempAddr	= MEMtPN8(locateObjectInMemory(objref));
	tempAddr	= PN8_A_CONST(tempAddr, OBJ_HDR_Context);
	objCC		= PN_R_U16(tempAddr);
	objCC		= U16_AND_U16(objCC, GROUP_MASK);

	memWriteU16Atomic(tempAddr , objCC);
}

void setJCREGlobal()
{
	jref arrayref;
	memref arrayrefaddr;
	arrayref = popReference();
	arrayrefaddr = locateObjectInMemory(arrayref);
	set_JCRE_global(MEMtPN8(arrayrefaddr));
}
/**
*modify by jiangxiaohui@20220324 将apdu_SetCommand(APDUBuffer, APDU_BUFFER_SIZE);从此函数移动到setCardCreated函数中
*/
void initAPDUBuffer()
{
	jref		D_A		arrayref;
	PN8			D_A		object;

	arrayref	= make_Transient_Array(SIZE_BYTE, Header_ArrayType_Byte, Byte_PA_CLASS_OFFSET, CLEAR_ON_RESET, APDU_BUFFER_SIZE, MAKE_TRANSIENT_ARRAY_COMMON_MODE);
	object		= MEMtPN8(locateObjectInMemory(arrayref));
	set_JCRE_global(object);

	JcvmConfig_SetApduBufferRef(arrayref);
	if(APDUBuffer != MEMtPX8(get_array_data(object)))
	{
		while(1);
	}
	//apdu_FetchComand(APDUBuffer);
//    apdu_SetCommand(APDUBuffer, APDU_BUFFER_SIZE); deleted by jiangxiaohui@20220324

	pushShort(arrayref);
}

void initCacheBuffer()
{
	jref arrayref;
	PN8			D_A		object;

	arrayref = make_Transient_Array(SIZE_BYTE,Header_ArrayType_Byte,Byte_PA_CLASS_OFFSET,CLEAR_ON_RESET,CACHE_BUFFER_SIZE, MAKE_TRANSIENT_ARRAY_COMMON_MODE);
	object = MEMtPN8(locateObjectInMemory(arrayref));
	set_JCRE_global(MEMtPF8(object));
	
	JcvmConfig_SetCacheBufferRef(arrayref);
	if(cacheBuffer != MEMtPX8(get_array_data(MEMtPF8(object))))
	{
		while(1);
	}
	pushShort(arrayref);
}
void InitshortBuffer(void)
{
	jref		D_A		arrayref;
	PN8			D_A		object;

	arrayref	= make_Transient_Array(SIZE_SHORT, Header_ArrayType_Short, Short_PA_CLASS_OFFSET, CLEAR_ON_RESET, SHORT_BUFFER_SIZE, MAKE_TRANSIENT_ARRAY_COMMON_MODE);
	object		= MEMtPN8(locateObjectInMemory(arrayref));
	set_JCRE_global(object);

	//nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetShortBufferBase()), arrayref);
	JcvmConfig_SetShortBufferRef(arrayref);
	if(bShortBuffer != MEMtPX16(get_array_data(object))) // keil工程生成的bShortBuffer在.map文件中会自动对齐成WORD对齐，但是本函数生成时有可能不是word对齐的
	{
		while(1);
	}
	pushShort(arrayref);
}

void InitramBuffer(void)
{
	jref		D_A		arrayref;
	PN8			D_A		object_addr;

	arrayref	= make_Transient_Array(SIZE_BYTE, Header_ArrayType_Byte, Byte_PA_CLASS_OFFSET, CLEAR_ON_RESET, RAM_BUFFER_SIZE, MAKE_TRANSIENT_ARRAY_COMMON_MODE);
	object_addr = MEMtPN8(locateObjectInMemory(arrayref));
	set_JCRE_global(object_addr);
	//nvmWriteU16Atomic(MEMtPN8(JcvmConfig_GetRamBufferBase()), arrayref);
	JcvmConfig_SetRamBufferRef(arrayref);
	if(bRamBuffer != MEMtPX8(get_array_data(object_addr)))
	{
		while(1);
	}
	pushShort(arrayref);
}

#define INITRAMBUFFER           0x01
#define T0INITAPDUBUFFER        0x02
#define T0INITCACHEBUFFER       0x03
#define VOLATILEBYTEARRAYBUFFER 0x04
void InitBuffer(void)
{
/*@ -discast @*/	jbyte type = (jbyte) popShort();/*@ +discast @*/
	switch(type)
	{
		case INITRAMBUFFER:
			InitramBuffer();
			break;
		case T0INITAPDUBUFFER:
			initAPDUBuffer();
			break;
		case T0INITCACHEBUFFER:
			initCacheBuffer();
			break;
		default:
			break;
	}
}
jref make_Transient_Array(u8 type, u8 internalType, jref internalTypeClass, u8 event, u16 count, u8 mode)		SMALL_MODE
{
	u32			D_A		size;
	u8			D_A		arrayType;
	u16			D_A		eesize;
	jref		D_A		arrayref;
	PN8			D_A		arrayrefaddr;
	PN8			D_A		tempArrayRefAddr;
	u16			D_A		dataref;
	u8			D_A		transactionStarted;

	transactionStarted	= 0;
	if (event != CLEAR_ON_RESET && event != CLEAR_ON_DESELECT)
	{
		throw_error(SYSTEMEXCEPT_ILLEGAL_VALUE);
	}

	/* check to see if the count is negative; if so, throw exception */
	if (0x8000 == U16_AND_U16(0x8000, count))
	{
		throw_error(NEGATIVE_ARRAY_SIZE_EXCEPTION);
	}

	/* Transient objects of CLEAR_ON_DESELECT type can only be created when the
	 currently active context is the context of the currently selected applet.
	 Except if the current context is the JCRE context.  */
	if (event == CLEAR_ON_DESELECT)
	{
#if JCVM__VM__FIRE_WALL
		checkDTRCreation();
#endif
	}

	size	= U8_M_U16(type, count);  // re-use the variable 'arrayrefaddr' to store the real size to allocate.

	if (size > 0x0000FFFF)  // check whether the size is bigger than one word.
	{
		throw_error(SYSTEMEXCEPT_NO_TRANSIENT_SPACE);	
	}

	//size =(u16) (count * type);

	if (!isInProgress())
	{
		begin_transaction(SINGLE_APDU_PROCESS);
		transactionStarted = 1;
	}/*end if*/


	/* Allocate RAM space in clear-on-reset or clear-on-deselect pool */
	if (event == CLEAR_ON_DESELECT)
	{
		dataref = tranAllocate_dtrAlloc(cc, U32t16(size));
	}
	else
	{
		dataref = tranAllocate_rtrAlloc(U32t16(size));
	}

	/* Did the EEPROM allocation request succeed? */
	if (dataref == NO_MEMORY_AVAILABLE)
	{
		/* out of memory! */
		if (transactionStarted)
		{
			abort_transaction(ABORT_NOT_ON_RESET);
		}/*end if*/
		throw_error(SYSTEMEXCEPT_NO_TRANSIENT_SPACE);
	}

	/* Request EEPROM storage space */
	eesize			= U8t16(U8_A_U8((internalType == Header_ArrayType_Reference ? SIZE_OBJECT_ARRAY_HEADER : SIZE_PRIMITIVE_ARRAY_HEADER) /* size of array header */
								, sizeof(jref))); /* pointer to RAM space */

	arrayref		= NvmManager_Allocate(U16t32(eesize), MAP_ENTRY_OBJECT_PCK_ID);
	arrayrefaddr	= MEMtPN8(MapTable_GetEntryAddr(arrayref));


	/*check if EEPROM allocation was successful*/
	checkNullAddrAndManageTransaction(arrayrefaddr, transactionStarted);

	arrayType		= U8_OR_U8(internalType, HEADER_JCRE_TransObj_IsTrans);

	if(MAKE_TRANSIENT_ARRAY_GLOBAL_MODE == mode)
	{
		arrayType		= U8_OR_U8(arrayType, HEADER_Global_Shared);
	}

	if (event == CLEAR_ON_DESELECT)
	{
		arrayType	= U8_OR_U8(arrayType, HEADER_JCRE_TransMode_Deselect);
	}
	else
	{
#if (EUICC_ONLY||SE_AND_EUICC)
		if(SE_SPACE_ID != RtrSpace_searchExistRtrID())//whether is proile rtr revesied by wpj
		{
			arrayType	= U8_OR_U8(arrayType, HEADER_PROFILE_RTR);//reuse this bit
		}
#endif
	}

	tempArrayRefAddr	= arrayrefaddr;

	nvmWriteU8Atomic(arrayrefaddr, arrayType);
	arrayrefaddr		= PN8_A_CONST(arrayrefaddr, 1);

	nvmWriteU16Atomic(arrayrefaddr, cc);
	arrayrefaddr		= PN8_A_CONST(arrayrefaddr, 2);

	nvmWriteU16Atomic(arrayrefaddr, internalTypeClass);
	arrayrefaddr		= PN8_A_CONST(arrayrefaddr, 2);

	nvmWriteU8Atomic(arrayrefaddr, VM_ARRAY_IMPLEMENTATION_PKG);
	arrayrefaddr		= PN8_A_CONST(arrayrefaddr, 1);

	nvmWriteU16Atomic(arrayrefaddr, count);
	arrayrefaddr		= PN8_A_CONST(arrayrefaddr, 2);

	if (internalType == Header_ArrayType_Reference)
	{
		/* component class is java.lang.Object */
		nvmWriteU16Atomic(arrayrefaddr, OBJECT_CLASS_OFFSET);
		arrayrefaddr	= PN8_A_CONST(arrayrefaddr, 2);

		nvmWriteU8Atomic(arrayrefaddr, JAVA_LANG_PKG_ID);
		arrayrefaddr	= PN8_A_CONST(arrayrefaddr, 1);
	}

	/* store pointer to data in RAM */
	nvmWriteU16Atomic(arrayrefaddr, dataref);
	memRefresh();



	/* init contents to zeros */
	/* Note that Logical channels demand a change to make relative
	 * addresses absolute.*/
	ecpMem_Set(MEMtPX8(get_array_data(tempArrayRefAddr)), 0, U32t16(size));

//	arrayref = addObject(arrayrefaddr,TRANSIENT_TABLE);
	/*check if object addition to object table was successful*/
	checkNullAndManageTransaction(arrayref, transactionStarted);

	if (transactionStarted)
	{
		commit_transaction();
	}/*end if*/

	return arrayref;
}

void makeTransientArray(u8 size_type, u8 internalType, jref internalTypeClass)    SMALL_MODE
{
	u8		D_A		event;
	u16		D_A		count;

	event	= U16t8(popShort());	  //wzq:   event	= U16t8(popShort())
	count	= popShort();

	pushShort(make_Transient_Array(size_type, internalType, internalTypeClass, event,count, MAKE_TRANSIENT_ARRAY_COMMON_MODE));
}

void makeGlobalArray(void)
{
	u8		D_A		ArrayType;
	u8		D_A		sizeType;
	u8		D_A		iType;
	u8		D_A		Event;
	u16		D_A		ArrayLen;
	jref			iTypeClass;


	ArrayLen	=	popShort();
	ArrayType	=	U16t8(popShort());
	Event		=	CLEAR_ON_RESET;
	iTypeClass  =   0; // remove warning
	iType       =   0;
	sizeType    =   0;
	
	switch(ArrayType)
	{
		case ARRAY_TYPE_BOOLEAN:
			sizeType	= SIZE_BOOLEAN;
			iType		= Header_ArrayType_Boolean;
			iTypeClass	= Boolean_PA_CLASS_OFFSET;
			break;
		case ARRAY_TYPE_BYTE:
			sizeType	= SIZE_BYTE;
			iType		= Header_ArrayType_Byte;
			iTypeClass	= Byte_PA_CLASS_OFFSET;
			break;
		case ARRAY_TYPE_SHORT:
			sizeType	= SIZE_SHORT;
			iType		= Header_ArrayType_Short;
			iTypeClass	= Short_PA_CLASS_OFFSET;
			break;
		case ARRAY_TYPE_INT:
			sizeType	= SIZE_INT;
			iType		= Header_ArrayType_Int;
			iTypeClass	= Int_PA_CLASS_OFFSET;
			break;
		case ARRAY_TYPE_OBJECT:
			sizeType	= SIZE_REF;
			iType		= Header_ArrayType_Reference;
			iTypeClass	= Object_PA_CLASS_OFFSET;
			break;
		default:
			throw_error(SYSTEMEXCEPT_ILLEGAL_VALUE);
			break;
	}
	pushShort(make_Transient_Array(sizeType, iType, iTypeClass, Event, ArrayLen, MAKE_TRANSIENT_ARRAY_GLOBAL_MODE));
}
#if JCVM__VM__JAVACARDX_FRAMEWORK_UTIL
boolean isPrimitiveComponent(PF8 array_ref)
{
	u16 objType;
	objType = (get_object_header_type(array_ref) & OBJ_HDR_OBJECT_TYPE_MASK);
	switch(objType)
	{
		case Header_ArrayType_Byte:
	  case Header_ArrayType_Short:
		case Header_ArrayType_Int:
			break;
		default:
			return false;
	}
	return true;
}

boolean isInstanceof(PF8 array_ref,u8 arrayType)
{
	u8 objType;
	
	objType = get_object_header_type(array_ref) & OBJ_HDR_OBJECT_TYPE_MASK;
	
	if(objType == arrayType)
	{
		return true;
	}
	else
	{
		return false;
	}
}

u8 check_arraypack_type(PM8 destAddr,PM8 srcAddr)
{
	u8 result;
	
	result = 0;
	
	check_null(destAddr);
	check_null(srcAddr);
	
#if JCVM__VM__FIRE_WALL
	checkArrayRead(destAddr);
	checkArrayRead(srcAddr);
#endif  //__FIRE_WALL__
	
	if(!isPrimitiveComponent(destAddr) || !isPrimitiveComponent(srcAddr))
	{
		 throw_error(UTILEXCEPT_ILLEGAL_VALUE);
	}
#if VM_INTS_SUPPORTED
	if(isInstanceof(srcAddr,Header_ArrayType_Int) && isInstanceof(destAddr,Header_ArrayType_Int))
	{
		result = INT_INT;
	}
  	/*********copy short array to int array*************/
	else if(isInstanceof(srcAddr,Header_ArrayType_Short) && isInstanceof(destAddr,Header_ArrayType_Int))
	{
    result = SHORT_INT;
	}
	/*********copy byte array to int array*************/
	else if(isInstanceof(srcAddr,Header_ArrayType_Byte) && isInstanceof(destAddr,Header_ArrayType_Int))
	{
    result = BYTE_INT;
	}
	/*********copy int array to byte array*************/
	else if(isInstanceof(srcAddr,Header_ArrayType_Int) && isInstanceof(destAddr,Header_ArrayType_Byte))
	{
    result = INT_BYTE;
	}
	/*********copy int array to short array*************/
	else if(isInstanceof(srcAddr,Header_ArrayType_Int) && isInstanceof(destAddr,Header_ArrayType_Short))
	{
	  result = INT_SHORT;
	}else
#endif
	/*********copy byte array to short array*************/
	if(isInstanceof(srcAddr,Header_ArrayType_Byte) && isInstanceof(destAddr,Header_ArrayType_Short))
	{
    result = BYTE_SHORT;
	}
	/*********copy short array to byte array*************/
	else if(isInstanceof(srcAddr,Header_ArrayType_Short) && isInstanceof(destAddr,Header_ArrayType_Byte))
	{
    result = SHORT_BYTE;
	}
	/*********copy byte array to byte array*************/
	else if(isInstanceof(srcAddr,Header_ArrayType_Byte) && isInstanceof(destAddr,Header_ArrayType_Byte))
	{
    result = BYTE_BYTE;
	}
	/*********copy short array to short array*************/
	else if(isInstanceof(srcAddr,Header_ArrayType_Short) && isInstanceof(destAddr,Header_ArrayType_Short))
	{
	  result = SHORT_SHORT;
	}

	return result;
}

u16 copyRepack(PM8 destAddr,u16 destOff,PM8 srcAddr,u16 srcOff,u16 srcLen,u8 isAtomicCopy)
{
	u8 packType;
	u16 dataLen;
	u16 mulDestOff;
	u16 mulSrcOff;
	PM8 dest;
	PM8 src;
	u16 destLen;
	
	packType = check_arraypack_type(destAddr,srcAddr);
	
  check_arraybounds_interval(srcAddr,srcOff,srcLen);
		
	switch(packType)
	{
#if VM_INTS_SUPPORTED

		case INT_INT:
			mulDestOff = 4;
			mulSrcOff = 4;
			destLen = srcLen;
			dataLen = srcLen * 4;
		  break;
    case SHORT_INT:
			if((srcLen % 2) != 0)
			{
				throw_error(UTILEXCEPT_ILLEGAL_VALUE);
			}
			mulDestOff = 4;
			mulSrcOff = 2;
			destLen = srcLen / 2;
			dataLen = srcLen*2;
	    break;
	  case BYTE_INT:
			if((srcLen % 4) != 0)
			{
				throw_error(UTILEXCEPT_ILLEGAL_VALUE);
			}
			mulDestOff = 4;
			mulSrcOff = 1;
			destLen = srcLen / 4;
			dataLen = srcLen;
			break;
	  case INT_BYTE:
			mulDestOff = 1;
			mulSrcOff = 4;
			destLen = srcLen * 4;
			dataLen = srcLen * 4;
		  break;
	  case INT_SHORT:
			mulDestOff = 2;
			mulSrcOff = 4;
			destLen = srcLen * 2;
			dataLen = srcLen * 4;
		  break;
#endif
		case BYTE_SHORT:
			if((srcLen % 2) != 0)
			{
				throw_error(UTILEXCEPT_ILLEGAL_VALUE);
			}
			mulDestOff = 2;
			mulSrcOff = 1;
			destLen = srcLen / 2;
			dataLen = srcLen;
		  break;
	  case SHORT_BYTE:
			mulDestOff = 1;
			mulSrcOff = 2;
			destLen = srcLen * 2;
			dataLen = srcLen * 2;
		  break;
	  case BYTE_BYTE:
			mulDestOff = 1;
			mulSrcOff = 1;
			destLen = srcLen;
			dataLen = srcLen;
		  break;
	  case SHORT_SHORT:
			mulDestOff = 2;
			mulSrcOff = 2;
			destLen = srcLen;
			dataLen = srcLen * 2;
		default:
			break;

}
	check_arraybounds_interval(destAddr,destOff,destLen);
#if SENSITIVE_ARRAY_SUPPORT
	src = PM8_A_U16(MEMtPM8(get_array_data(srcAddr)), srcOff * mulSrcOff);
	if(IS_SENSITIVEARRY_OBJ(destAddr) && (U8_AND_U8((get_object_header_type(destAddr)), OBJ_HDR_TransientObj_MASK) == HEADER_JCRE_TransObj_NotTrans))
	{
		if(isAtomicCopy)
		{
			dest = PM8_A_U16(MEMtPM8(get_array_data(destAddr)), destOff * mulDestOff);
		}
		else
		{
		  throw_error(SYSTEMEXCEPT_ILLEGAL_VALUE);
		}
	}
	else
	{
		dest = PM8_A_U16(MEMtPM8(get_array_data(destAddr)), destOff * mulDestOff);
	}
#else
	dest = PM8_A_U16(MEMtPM8(get_array_data(destAddr)), destOff * mulDestOff);
	src = PM8_A_U16(MEMtPM8(get_array_data(srcAddr)), srcOff * mulSrcOff);
#endif
	
	if(isAtomicCopy == true)
	{
		memCopyAtomic(dest,src,dataLen);
	}
	else
	{
		memCopyNoAtomic(dest,src,dataLen);
	}
	
	return destOff + destLen;
}

u16 fillGenericNonAtomic(PM8 destAddr,u16 destOff,PM8 srcAddr,u16 srcOff,u16 length)
{
	PM8 dest;
	//PM8 src;  //remove warning,dengyiwen,2022/9/22
	u32 val;
	u8 i;
	u8 packType;
	
	packType = check_arraypack_type(destAddr,srcAddr);
#if SENSITIVE_ARRAY_SUPPORT
	if(IS_SENSITIVEARRY_OBJ(destAddr) && (U8_AND_U8((get_object_header_type(destAddr)), OBJ_HDR_TransientObj_MASK) == HEADER_JCRE_TransObj_NotTrans))
	{
		 throw_error(SYSTEMEXCEPT_ILLEGAL_VALUE);
	}
#endif
	check_arraybounds_interval(destAddr,destOff,length);
	check_arraybounds_interval(srcAddr,srcOff,1);
	
	switch(packType)
	{
#if VM_INTS_SUPPORTED	
		case INT_INT:
			val = memReadU32(PM8_A_U16(MEMtPM8(get_array_data(srcAddr)), srcOff * 4));
		  dest = PM8_A_U16(MEMtPM8(get_array_data(destAddr)), destOff * 4);
		  for(i = 0;i < length * 4;i += 4)
		  {
			   memWriteU32NoAtomic(dest + i,val);
	  	}
			break;
#endif
		case SHORT_SHORT:
			val = memReadU16(PM8_A_U16(MEMtPM8(get_array_data(srcAddr)), srcOff * 2));
		  dest = PM8_A_U16(MEMtPM8(get_array_data(destAddr)), destOff * 2);
		  for(i = 0;i < length * 2;i += 2)
		  {
			   memWriteU16NoAtomic(dest + i,val);
	  	}
			break;
		case BYTE_BYTE:
		  val = memReadU8(PM8_A_U16(MEMtPM8(get_array_data(srcAddr)), srcOff));
		  dest = PM8_A_U16(MEMtPM8(get_array_data(destAddr)), destOff);
		  memFillNoAtomic(dest,val,length);
		  break;
		default:
		  throw_error(UTILEXCEPT_TYPE_MISMATCHED);
		  break;
	}
  return destOff + length;
}

s8 compareGeneric(PM8 destAddr,u16 destOff,PM8 srcAddr,u16 srcOff,u16 length)
{
	PM8 dest;
	PM8 src;
	s8 result;
	//u8 i;  //remove warning,dengyiwen,2022/9/22
	//s32 s;  //remove warning,dengyiwen,2022/9/22
	//s32 t;  //remove warning,dengyiwen,2022/9/22
	u8 packType;
	
	result = 0;
	packType = check_arraypack_type(destAddr,srcAddr);
	
	check_arraybounds_interval(destAddr,destOff,length);
	check_arraybounds_interval(srcAddr,srcOff,length);

	switch(packType)
	{
		case BYTE_BYTE:
			dest = PM8_A_U16(MEMtPM8(get_array_data(destAddr)), destOff);
		    src = PM8_A_U16(MEMtPM8(get_array_data(srcAddr)), srcOff);
			result = memCompareSigned(src,dest,length);
		break;
		case SHORT_SHORT:
			dest = PM8_A_U16(MEMtPM8(get_array_data(destAddr)), destOff * 2);
		    src = PM8_A_U16(MEMtPM8(get_array_data(srcAddr)), srcOff * 2);
            result = memCompareU16(src,dest,length);
			break;
#if VM_INTS_SUPPORTED
		case INT_INT:
		    dest = PM8_A_U16(MEMtPM8(get_array_data(destAddr)), destOff * 4);
	        src = PM8_A_U16(MEMtPM8(get_array_data(srcAddr)), srcOff * 4);
		    result = memCompareU32(src,dest,length);
			break;
#endif
		default:
			throw_error(UTILEXCEPT_TYPE_MISMATCHED);
			break;
	}
	return result;
}

u16 fillGeneric(PM8 destAddr,u16 destOff,PM8 srcAddr,u16 srcOff,u16 length)
{
	PM8 dest;
	//PM8 src;  //remove warning,dengyiwen,2022/9/22
	u32 val;
	///u8 arrayTypeSize;  //remove warning,dengyiwen,2022/9/22
	u8 i;
	u8 packType;
	
	packType = check_arraypack_type(destAddr,srcAddr);
	
	check_arraybounds_interval(destAddr,destOff,length);
	check_arraybounds_interval(srcAddr,srcOff,1);

  switch(packType)
	{
		case BYTE_BYTE:
		  val = memReadU8(PM8_A_U16(MEMtPM8(get_array_data(srcAddr)), srcOff));
		  dest = PM8_A_U16(MEMtPM8(get_array_data(destAddr)), destOff);
		  memFillAtomic(dest,val,length);
      break;
		case SHORT_SHORT:
		  val = memReadU16(PM8_A_U16(MEMtPM8(get_array_data(srcAddr)), srcOff * 2));
		  dest = PM8_A_U16(MEMtPM8(get_array_data(destAddr)), destOff * 2);
		  for(i = 0;i < length * 2;i += 2)
		  {
			   memWriteU16Atomic(dest + i,val);
	  	}
      break;
#if VM_INTS_SUPPORTED
    case INT_INT:
		  val = memReadU32(PM8_A_U16(MEMtPM8(get_array_data(srcAddr)), srcOff * 4));
		  dest = PM8_A_U16(MEMtPM8(get_array_data(destAddr)), destOff * 4);
		  for(i = 0;i < length * 4;i += 4)
		  {
			   memWriteU32Atomic(dest + i,val);
	  	}
		  break;
#endif
		default:
			throw_error(UTILEXCEPT_TYPE_MISMATCHED);
			break;
	}
	return destOff + length;
}

s16 findGeneric(PM8 theArray,u16 off,PM8 valArray,u16 valOff)
{
	u16 length;
	u8 i;
	PM8 s;
	PM8 t;
	u8 packType;
	s16 result;
	
	packType = check_arraypack_type(theArray,valArray);
	
	check_arraybounds_interval(theArray,off,0);
	length = get_array_length(theArray);
	result = -1;
	
	s = MEMtPM8(get_array_data(valArray) + valOff);
	t = MEMtPM8(get_array_data(theArray));
	
	switch(packType)
	{
#if VM_INTS_SUPPORTED
		case BYTE_INT:
			check_arraybounds_interval(valArray,valOff,4);
			for(i = 4*off;i < length * 4;i += 4)
			{
				if(memCompare(s,t+i,4) == ECP_CMP_RESULT_EQUAL)
				{
					result = i/4;
					break;
				}
			}
		break;
#endif
		case BYTE_SHORT:
			if(isInstanceof(theArray,Header_ArrayType_Short))
			{
				check_arraybounds_interval(valArray,valOff,2);
				for(i = 2*off;i < length * 2;i += 2)
				{
					if(memCompare(s,t+i,2) == ECP_CMP_RESULT_EQUAL)
					{
			    	result = i/2;
				  	break;
					}
				}
			}
			break;
		case BYTE_BYTE:
			check_arraybounds_interval(valArray,valOff,1);
			for(i = off;i < length;i += 1)
			{
				if(memCompare(s,t+i,1) == ECP_CMP_RESULT_EQUAL)
				{
			  	result = i;
					break;
				}
			}
		break;
		default:
		break;
	}
	return result;
}


void  arrayCopyRepack(u8 isAtomicCopy)
{
	u16 destOff;
	PM8 destAddr;
	u16 srcOff;
	PM8 srcAddr;
	u16 srcLen;
  u16 result;
	
	destOff = popShort();
	destAddr = MEMtPM8(locateObjectInMemory(popReference()));
	srcLen = popShort();
	srcOff = popShort();
	srcAddr = MEMtPM8(locateObjectInMemory(popReference()));
	
  result = copyRepack(destAddr,destOff,srcAddr,srcOff,srcLen,isAtomicCopy);
	pushShort(result);
}

void  arrayFillGenericNonAtomic()
{
	u16 destOff;
	PM8 destAddr;
	u16 length;
	u16 srcOff;
	PM8 srcAddr;
    u16 result;
	
	srcOff = popShort();
	srcAddr = MEMtPM8(locateObjectInMemory(popReference()));
	length = popShort();
	destOff = popShort();
	destAddr = MEMtPM8(locateObjectInMemory(popReference()));
	
	result = fillGenericNonAtomic(destAddr,destOff,srcAddr,srcOff,length);
	pushShort(result);
}
		
void  arrayCompareGeneric()
{
	u16 length;
	u16 destOff;
	PM8 destAddr;
	u16 srcOff;
	PM8 srcAddr;
    s8 result;
	
	length = popShort();
	destOff = popShort();
	destAddr = MEMtPM8(locateObjectInMemory(popReference()));
	srcOff = popShort();
	srcAddr = MEMtPM8(locateObjectInMemory(popReference()));
	
    result = compareGeneric(destAddr,destOff,srcAddr,srcOff,length);
	pushShort(result);
}

void arrayFillGeneric(void)
{
	u16 destOff;
	PM8 destAddr;
	u16 length;
	u16 srcOff;
	PM8 srcAddr;
  u16 result;
	
	srcOff = popShort();
	srcAddr = MEMtPM8(locateObjectInMemory(popReference()));
	length = popShort();
	destOff = popShort();
	destAddr = MEMtPM8(locateObjectInMemory(popReference()));

	result = fillGeneric(destAddr,destOff,srcAddr,srcOff,length);
	pushShort(result);
	return;
}

void  arrayFindGeneric()
{
	u16 valOff;
	PM8 valArray;
	u16 off;
	PM8 theArray;
  s16 result;
	
	valOff = popShort();
	valArray = MEMtPM8(locateObjectInMemory(popReference()));
	off = popShort();
	theArray = MEMtPM8(locateObjectInMemory(popReference()));
	
  result = findGeneric(theArray,off,valArray,valOff);
	pushShort(result);
}
#endif

#if JCVM__VM__JAVACARDX_FRAMEWORK_UTIL_INTX

void  setInt()
{
	u32  setValue;
	u16  offset;
	jref destRef;
	PM8  destArray;
	u8   result;
	setValue = popInt();
	offset   = popShort();
	destRef  = popReference();
	destArray= MEMtPM8(locateObjectInMemory(destRef));
	check_null(destArray);
	check_bounds_interval(destArray, offset, 4);
	destArray	= PM8_A_U16(MEMtPM8(get_array_data(destArray)), offset);
	result      = memReadU8(PM8_A_U8(destArray,3));
	memWriteU32Atomic(destArray,setValue);
	pushShort(result);

}

#endif


void isTransient()
{
	jref		D_A		theObj;
	PN8			D_A		theObjAddr;
	u8			D_A		ObjectType;

	theObj		= popReference();
	theObjAddr	= MEMtPN8(locateObjectInMemory(theObj));


	/* check that object not null */
	if (check_addr_null(theObjAddr))
	{
		pushShort(NOT_A_TRANSIENT_OBJECT);
		return;
	}

	/* check for firewall */
#if JCVM__VM__FIRE_WALL
	checkArrayRead(theObjAddr);
#endif  //__FIRE_WALL__
	ObjectType = get_object_header_type(theObjAddr);
	if (U8_AND_U8(ObjectType, OBJ_HDR_TransientObj_MASK) == OBJ_HDR_TransientObj_MASK)
	{
		if (U8_AND_U8(ObjectType, OBJ_HDR_TransientMode_MASK) == OBJ_HDR_TransientMode_MASK)
		{
			pushShort(CLEAR_ON_DESELECT);
		}
		else
		{
			pushShort(CLEAR_ON_RESET);
		}
	}
	else
	{
		pushShort(NOT_A_TRANSIENT_OBJECT);
	}
}

/**
 * public static native void clearTransientObjs(u8 event);
 * ---------------------------------------------------------
 * Called from Dispatcher to clear transient objects.
 * @param event the event which caused the array elements to be cleared.

void clearTransientObjs()
{
	jbyte event = (jbyte) popShort();
	u8 channelId = (u8) popShort();

    (event == CLEAR_ON_DESELECT) ? ramMnanger_dtrClear(ramMnanger_getDtrSpaceId(channelId)) : ramMnanger_rtrClear();
}
 */

void beginTransaction(u8 isMultiApduProcess)
{

	if (isInProgress())
	{
		/* Transaction in progress ! */
		throw_error(TRANSACTIONEXCEPT_IN_PROGRESS);
	}
	else
	{
		begin_transaction(isMultiApduProcess);
		/* reset the EEPROM allocation flag to detect allocations during a
		 transaction */
		e2p_allocation_flag = false;
	}
}
/***********************************************************************************************
 *
 *the following functions are to support Transaction and atomic
 *
 ************************************************************************************************/

void abortTransaction()
{

	if (!isInProgress())
	{
		/* no transaction in progress */
		throw_error(TRANSACTIONEXCEPT_NOT_IN_PROGRESS);
	}
	else
	{
		abort_transaction(ABORT_NOT_ON_RESET);

		if (e2p_allocation_flag && !IS_JCRE_CURRENT_CONTEXT())
		{
			/* Some blocks were allocated during the transaction. These blocks are
			 deallocated and all persistent references to them have been rolled
			 back. However, transient references are not rolled back. To avoid
			 dangling pointers, force a card reset by locking the session. Card
			 session is not terminated only in case the JCRE is the context because
			 in this case there will not be any pointers on stack.
			 */
			fatal_error(ALLOCATION_DURING_TRANSACTION);
		}
	}
}
void commitTransaction()
{

	if (!isInProgress())
	{
		/* no transaction in progress */

		throw_error(TRANSACTIONEXCEPT_NOT_IN_PROGRESS);
	}
	else
	{
		commit_transaction();
		closeProgress();
	}
}

/****************************************************************************************************************************************
 *
 *JAVA Array operator Native method
 *
 *****************************************************************************************************************************************/
void checkArrayArgs()
{
	u16		D_A		length;
	u16		D_A		srcOff;
	PN8		D_A		src;

	length	= popShort();
	srcOff	= popShort();
	src		= MEMtPN8(locateObjectInMemory(popReference()));

	/* Check for src  being 'null' */
	check_null(src);
	/* check array bounds */
	check_bounds_interval(src, srcOff, length);
}

void arrayCopy()
{
	PM8			D_A		s;
	PM8			D_A		t;
	u16			D_A		length;
	u16			D_A		destOff;
	jref		D_A		dest;
	PM8			D_A		destaddr;
	u16			D_A		srcOff;
	jref		D_A		src;
	PM8			D_A		srcaddr;
    u16         D_A     result;

	length		= popShort();
	destOff 	= popShort();
	dest		= popReference();
	srcOff		= popShort();
	src			= popReference();

	destaddr	= MEMtPM8(locateObjectInMemory(dest));
	srcaddr		= MEMtPM8(locateObjectInMemory(src));


	/* Check for src or dest being 'null' */
	/* check for firewall */

	check_null(srcaddr);
	check_null(destaddr);

#if JCVM__VM__FIRE_WALL
	checkArrayRead(srcaddr);
	checkArrayRead(destaddr);
#endif  //__FIRE_WALL__
	/* check array bounds */
	check_bounds_interval(srcaddr, srcOff, length);
	check_bounds_interval(destaddr, destOff, length);
#if SENSITIVE_RESULT_SUPPORT
  sensitiveResultSetTagValUnassigned();
#endif
	s	= PM8_A_U16(MEMtPM8(get_array_data(srcaddr)), srcOff);
	t	= PM8_A_U16(MEMtPM8(get_array_data(destaddr)), destOff);

	memCopyAtomic(t, s, length);
	result = (U32t16(U16_A_U16(destOff, length)));
#if SENSITIVE_RESULT_SUPPORT
	sensitiveResultSet(result);
#endif
	pushShort(result);
}

void arrayCopyNonAtomic()
{
	PM8			D_A		s;
	PM8			D_A		t;
	u16			D_A		length;
	u16			D_A		destOff;
	jref		D_A		dest;
	PM8			D_A		destaddr;
	u16			D_A		srcOff;
	jref		D_A		src;
	PM8			D_A		srcaddr;
    u16         D_A     result;

	length		= popShort();
	destOff 	= popShort();
	dest		= popReference();
	srcOff		= popShort();
	src			= popReference();

	destaddr 	= MEMtPM8(locateObjectInMemory(dest));
	srcaddr 	= MEMtPM8(locateObjectInMemory(src));


	/* Check for src or dest being 'null' */
	/* check for firewall */
	check_null(srcaddr);
	check_null(destaddr);

#if JCVM__VM__FIRE_WALL
	checkArrayRead(srcaddr);
	checkArrayRead(destaddr);
#endif  //__FIRE_WALL__
	/* check array bounds */
	check_bounds_interval(srcaddr, srcOff, length);
	check_bounds_interval(destaddr, destOff, length);
#if SENSITIVE_ARRAY_SUPPORT
  if(IS_SENSITIVEARRY_OBJ(destaddr) && (U8_AND_U8((get_object_header_type(destaddr)), OBJ_HDR_TransientObj_MASK) == HEADER_JCRE_TransObj_NotTrans))
	{
		throw_error(SYSTEMEXCEPT_ILLEGAL_VALUE);
	}
#endif
#if SENSITIVE_RESULT_SUPPORT
    sensitiveResultSetTagValUnassigned();
#endif
	s = PM8_A_U16(MEMtPM8(get_array_data(srcaddr)), srcOff);
	t = PM8_A_U16(MEMtPM8(get_array_data(destaddr)), destOff);

	memCopyNoAtomic(t, s, length);
	result = (U32t16(U16_A_U16(destOff, length)));
#if SENSITIVE_RESULT_SUPPORT
	sensitiveResultSet(result);
#endif
	pushShort(result);
}

void arrayFillNonAtomic()
{
	PM8			D_A		s;
	u8			D_A		bvalue;
	u16			D_A		bLen;
	u16			D_A		bOff;
	jref		D_A		bArray;
	PM8			X_A		bArrayAddr;
    u16         D_A     result;

	bvalue		= U16t8(popShort());    //wzq
	bLen		= popShort();
	bOff		= popShort();
	bArray 		= popReference();
	bArrayAddr	= MEMtPM8(locateObjectInMemory(bArray));


	/* Check for bArray being 'null' */
	/* check for firewall */
	check_null(bArrayAddr);

#if JCVM__VM__FIRE_WALL
	checkArrayRead(bArrayAddr);
#endif  //__FIRE_WALL__
	/* check array bounds */
	check_bounds_interval(bArrayAddr, bOff, bLen);
#if SENSITIVE_ARRAY_SUPPORT
	if(IS_SENSITIVEARRY_OBJ(bArrayAddr) && (U8_AND_U8((get_object_header_type(bArrayAddr)), OBJ_HDR_TransientObj_MASK) == HEADER_JCRE_TransObj_NotTrans))
	{
		throw_error(SYSTEMEXCEPT_ILLEGAL_VALUE);
	}
#endif
#if SENSITIVE_RESULT_SUPPORT
	sensitiveResultSetTagValUnassigned();
#endif
	s	= PM8_A_U16(MEMtPM8(get_array_data(bArrayAddr)), bOff);

	memFillNoAtomic(s, bvalue, bLen);
	result = (U32t16(U16_A_U16(bOff, bLen)));
#if SENSITIVE_RESULT_SUPPORT
	sensitiveResultSet(result);
#endif
	pushShort(result);
}

void arrayCompare()
{
	PM8		D_A		s;
	PM8		D_A		t;
	u16		D_A		length;
	u16		D_A		destOff;
	PM8		D_A		destaddr;
	u16		D_A		srcOff;
	PM8		D_A		srcaddr;
	u8		D_A		cmp;
    u16      D_A     result;

	length		= popShort();
	destOff		= popShort();
	destaddr	= MEMtPM8(locateObjectInMemory(popReference()));
	srcOff		= popShort();
	srcaddr		= MEMtPM8(locateObjectInMemory(popReference()));

	/* Check for src or dest being 'null' */
	/* check for firewall */
	check_null(srcaddr);
	check_null(destaddr);

#if JCVM__VM__FIRE_WALL
	checkArrayRead(srcaddr);
	checkArrayRead(destaddr);
#endif  //__FIRE_WALL__

	/* check array bounds */
	check_bounds_interval(srcaddr, srcOff, length);
	check_bounds_interval(destaddr, destOff, length);
#if SENSITIVE_RESULT_SUPPORT
	sensitiveResultSetTagValUnassigned();
#endif
	s	= PM8_A_U16(MEMtPM8(get_array_data(srcaddr)), srcOff);
	t	= PM8_A_U16(MEMtPM8(get_array_data(destaddr)), destOff);

	cmp = memCompareSigned(s, t, length);
	if (0xFF == cmp)
	{
		result = 0xFFFF;
#if SENSITIVE_RESULT_SUPPORT
		sensitiveResultSet(result);
#endif
		pushShort(result);
	}
	else if (cmp > 0)
	{
		result = 1;
#if SENSITIVE_RESULT_SUPPORT
		sensitiveResultSet(result);
#endif
		pushShort(result);
	}
	else
	{
		result = 0;
#if SENSITIVE_RESULT_SUPPORT
		sensitiveResultSet(result);
#endif
		pushShort(result);
	}
}

void getShort_java(void)
{
	memref s;
	memref bArrayAddr;
	u16 value;
	u16 bOff;
	u16 bArray;

	bOff = popShort();
	bArray = popReference();
	bArrayAddr = locateObjectInMemory(bArray);

	check_null(MEMtPF8(bArrayAddr));

#if JCVM__VM__FIRE_WALL
	checkArrayRead(MEMtPF8(bArrayAddr));
#endif  //__FIRE_WALL__
	check_bounds_interval(MEMtPF8(bArrayAddr), bOff, sizeof(jshort));

	s 		= get_array_data(MEMtPF8(bArrayAddr)) ;
	value 	= memReadU16(MEMtPM8(U32_A_U16(s, bOff)));
	pushShort(value);
}
void setShort()
{
	PM8		D_A		s;
	u16		D_A		sValue;
	u16		D_A		bOff;
	jref	D_A		bArray;
	PM8		D_A		bArrayAddr;

	sValue		= popShort();
	bOff		= popShort();
	bArray		= popReference();

	bArrayAddr	= MEMtPM8(locateObjectInMemory(bArray));


	/* Check for bArray being 'null' */
	/* check for firewall */

	check_null(bArrayAddr);

#if JCVM__VM__FIRE_WALL
	checkArrayRead(bArrayAddr);
#endif  //__FIRE_WALL__
	/* check array bounds */
	check_bounds_interval(bArrayAddr, bOff, sizeof(jshort));

	s	= PM8_A_U16(MEMtPM8(get_array_data(MEMtPF8(bArrayAddr))), bOff);
	memWriteU16Atomic(s, sValue);
	bOff		= U32t16(U16_A_U8(bOff, SIZE_SHORT));
	pushShort(bOff);
}
/*************************************************************************************************************************************
 *
 * the following functions are to support garbage collection
 *
 **************************************************************************************************************************************/
/**
 * Initializes the state for garbage collection
 */
void initGC()
{
	pushShort(U8t16(initializeGC()));
}

/*   public static native void initializeAppletDeletion(u8[] contextsArray, u8 count);*/
void initAD()
{
	u16 contexts;
	contexts = popShort();
	initAppletDeletion(contexts);
}

void initPackageDeletionNM()
{
	u8 pkgID;
	pkgID = U16t8(popShort());    //wzq
	initPackageDeletion(pkgID);
}

/**
 * Marks the root of applet roots: the applet table
 */
void markAppRoots()
{
	jref ref;
	ref = popReference();
	pushShort(U8t16(markAppletRoot(ref)));
}

/**
 * Check if the server applet is not multiselectable and is currently active on another logical channel
 */
void checkSIOaccess()
{							 
	u8		D_A  appID;
	jref	D_A  ref;

	appID	= U16t8(popShort());
	ref		= popReference();
	check_SIOaccess(ref, appID);
}

#define	API_VERSION_0300    0x0300
#define	API_VERSION_0202	0x0202 

void getAPI_VERSION()
{
	if(OS_JC2_MODE == OS_CONF3_JC2_JC3_MODE)
	{
		pushShort(API_VERSION_0202);
	}
	else
	{
		pushShort(API_VERSION_0300);	
	}
}
			 
/**
 * Marks the objects pointed to be static fields of a package
 */
void markStatics()
{
	u8		D_A  pkgId;
	jref	D_A  count;
	u8      D_A	 reset_reference_null;

	reset_reference_null 	= U16t8(popShort());
	count					= popShort();
	pkgId					= U16t8(popShort());
#if GP__AMENDMENT__H__FOR_MASK
	if((UPGRADE_IMPL_PKG_ID == pkgId)&&( PN_R_U8(upGradeStatus_INDEX)> UPGRADE_STATS_COMPLETED))
	{
		pushShort(U8t16(true));
	}
	else
#endif
	{
		pushShort(U8t16(markStaticRoots(pkgId, count, reset_reference_null)));
	}
}

void releaseStaticRef()
{
	u16		appContext;

	appContext	= popShort();
	releaseStaticFeild(appContext);
	pushShort(1);
}

/**
 * Starts the general garbage collection cycle.
 */
void startGC()
{
	pushShort(U8t16(doGC()));
}

/*
 This method removes the following from the card memory:
 1. all the components of the package from the memory.
 2. the exception table entry corresponding to this package.
 3. Any references that belong to this package from the global reference table
 */

void removePackageNM()
{
	u8		D_A		pkgId;

	popShort();
	pkgId	= U16t8(popShort());
	// remove all the components of the package from the memory.
	removePackage(pkgId);
	//remove the exception table entry corresponding to this package.
	remove_exception_table(pkgId);
	//remove any references that belong to this package from the global reference table
	removeReferencesFromPackage(pkgId);
	JCDebug_deletePackageDebugComponent(pkgId);
	tempConstantObjFree();
	NvmManager_Deallocate(false);

	pushShort(1);
}

void clearGlobalPkgAddrNM()
{
	G_PkgComponentAddrStartAddr = 0x00;
	G_PkgComponentAddrEndAddr = 0x00;
}

/****************************************************************************************************************
 *
 *the following functions are to support RMI
 *
 ******************************************************************************************************************/
#if JCVM__VM__RMI

/**
 * This method goes through the complete exported objects arrays and removes any
 * entries from it that do not point to objects existing on the card.
 */
void cleanupExportedObjectsArray()
{
	jref	expArray = popReference();
	memref	expArrayAddr = 0;
	jref count = 0;
	jref index = 0;

	expArrayAddr = locateObjectInMemory(expArray);
	count = nvmReadU16(expArrayAddr + PA_NumFields_WORD);
	expArrayAddr = get_array_data(expArrayAddr);

	for (index = 0; index < count; index++)
	{
		u16 objectId = nvmReadU16((memref)(expArrayAddr + (index * sizeof(jref))));
//		if (locateObjectInMemory(objectId) == 0)
		if (!MapTable_IsAvailObject(entry_index))
		{
			//invalid entry in the exported array
			memWriteU16Atomic((memref) (expArrayAddr + (index * sizeof(jref))), (jref) 0);
		}
	}

}

/**
 * Returns true if object ID given is accessable by the caller.
 */
void checkAccess()
{
	jref object = popReference();
	memref objectaddr;
	objectaddr = locateObjectInMemory(object);
	if (!IS_JCRE_CURRENT_CONTEXT() && !IS_OBJECT_CONTEXT_CURRENT(objectaddr))
		throw_error(SECURITY_EXCEPTION);
}

static memref getCollStringAddr(memref classAddr)
{
	memref collStringAddr;
	u8 numInterfaces;
	u8 i;


	/* First, get past the public and package virtual method tables */
	collStringAddr = classAddr + CR_MethodsTableStart + sizeof(jref) * (PF_R_U8((memref)(classAddr + CR_PublicMethods + MTH_NumMethods))) + sizeof(jref) * (PF_R_U8((memref)(classAddr + CR_PackageMethods + MTH_NumMethods)));
	/* Then get the count of the implemented interfaces */
	numInterfaces = PF_R_U8((memref)(classAddr + CR_Header)) & (u8) CRH_InterfaceCount;


	/* use the interface count to walk through all of the implemented interface info structures */
	for (i = 0; i < numInterfaces; i++)
	{
		collStringAddr = collStringAddr + IIT_FirstIndex + PF_R_U8((memref)(collStringAddr + IIT_NumInterfaces)) * IIT_IndexSize;
	}
	/* add size of any remote method info structures and the count of the structures
	 * to obtain the address of the class name */
	collStringAddr = collStringAddr + (PF_R_U8(collStringAddr) * (SIZE_REMOTE_METHOD_INFO)) + sizeof(u8);
	return (collStringAddr);
}

/**
 * returns the physical address of the class name for the object, which is one of the
 * fields of the the class info structure.
 */

void getClassNameAddress()
{
	memref nameAddress;
	memref classAddr;
	jref object = popReference();
	memref objectaddr = locateObjectInMemory(object);
	classAddr = get_object_class(objectaddr);
	nameAddress = getCollStringAddr(classAddr);


	/* skip the anti collision string and it's count */
	nameAddress = nameAddress + (PF_R_U8(nameAddress) * sizeof(u8)) + sizeof(u8);
	/* now at the class name address */
	pushAddress((s32)nameAddress);
}

/*
 * copies the anticollision string into the provided buffer
 */
void getAnticollisionString()
{
	memref colStringAddr;
	memref classAddr;
	u8 offset = (u8) popShort();
	memref buffer = locateObjectInMemory(popReference());
	jref object = popReference();
	memref dest;
	u16 length;
	memref objectaddr = locateObjectInMemory(object);
	classAddr = get_object_class(objectaddr);
	colStringAddr = getCollStringAddr(classAddr);


	/* should now be pointing to the collision string length */
	length = (u16)nvmReadU8(colStringAddr);
	dest = get_array_data(buffer) + (memref) offset;
	check_bounds_interval(buffer, offset, (short) (length + 1));
	memCopyToRam((u8*) dest, (u8 *) colStringAddr, (u16) (++length));
	pushShort((jshort)(offset + length));
}

void getRemoteInterfaceNumber()
{
	memref rem_interfcs_count_addr;
	memref classAddr;
	jref object = popReference();
	memref objectaddr = locateObjectInMemory(object);
	classAddr = get_object_class(objectaddr);

	rem_interfcs_count_addr = getCollStringAddr(classAddr);
	/* add length of collision string and it's string length */
	rem_interfcs_count_addr += ((PF_R_U8(rem_interfcs_count_addr) * sizeof(u8)) + sizeof(u8));


	/* add length of class name and it's length and we should be there */
	rem_interfcs_count_addr += ((PF_R_U8(rem_interfcs_count_addr) * sizeof(u8)) + sizeof(u8));
	pushShort((jshort)nvmReadU8(rem_interfcs_count_addr));
}

void getRemoteInterfaceAddress()
{
	memref rem_interfcs_addr;
	jref rem_interfcs_ref;
	memref classAddr;
	u8 classPkg;
	u8 index = (u8) popShort();
	jref object = popReference();
	memref objectaddr = locateObjectInMemory(object);
	classAddr = get_object_class(objectaddr);
	classPkg = get_object_class_pkg(objectaddr);
	rem_interfcs_addr = getCollStringAddr(classAddr);


	/* add length of collision string and it's string length */
	rem_interfcs_addr += ((PF_R_U8(rem_interfcs_addr) * sizeof(u8)) + sizeof(u8));


	/* add length of class name and it's length and we should be there */
	rem_interfcs_addr += ((PF_R_U8(rem_interfcs_addr) * sizeof(u8)) + sizeof(u8));


	/* add size of interface count */
	rem_interfcs_addr += sizeof(u8);

	rem_interfcs_addr = (memref) (rem_interfcs_addr + (index * SIZE_OF_CLASS_REF)); // 2 bytes per class ref
	rem_interfcs_ref = CODE_read_u16(rem_interfcs_addr);

	pushAddress((s32)resolveReferenceAddress(rem_interfcs_ref, classPkg, (u8)COMPONENT_CLASS));
}

void copyInterfaceNameIntoBuffer()
{
	u8 offset = (u8) popShort();
	memref buffer = locateObjectInMemory(popReference());
	memref interface_name_addr = (u32)popAddress();
	memref dest;
	u16 length;
	length = PF_R_U8(interface_name_addr) & 0x0f;
	interface_name_addr += 1 + length * sizeof(jref);
	length = PF_R_U8(interface_name_addr);
	dest = get_array_data(buffer) + (memref) offset;
	/* copy into destination */
	check_bounds_interval(buffer, offset, (short) (length + 1));
	memCopyToRam((u8*) dest, (u8 *) interface_name_addr, (u16) (++length));
	pushShort((jshort)(offset+length));
}

void copyStringIntoBuffer()
{
	/* length of string to be copied */
	u16 length;
	/* where string is copied to */
	memref dest;
	/* offset in u8 array where string is to be copied to */
	u16 offset = (u16)popShort();
	/* u8 array object which will hold copied string*/
	jref buffer = popReference();
	/* string to be copied */
	memref string = (u32)popAddress();
	memref bufferaddr = locateObjectInMemory(buffer);
	dest = get_array_data(bufferaddr) + (memref) offset;
	length = (u16)nvmReadU8(string);
	check_bounds_interval(bufferaddr, (s16)offset, (short) (length + 1));
	memCopyToRam((u8*) dest, (u8 *) string, (u16) (++length));
	pushShort((jshort)(offset + length));
}

void getObjectID()
{
	jref objRef = popReference();
	pushShort((s16)objRef);
}

void getRemoteMethodInfo()
{
	/* address of the class_info structure */
	memref classAddr;
	/* address of the remote_methods_table_count, used as the beginning point to get
	 * to the remote_method_info
	 */
	memref remoteMethodInfo;
	/* number of remote methods */
	u8 method_count;
	/* index */
	u8 i;
	u8 numInterfaces;


	/* object ID/reference for the object with the remote methods */
	u16 methodID = (u16)popShort();
	jref object = popReference();
	memref objectaddr = locateObjectInMemory(object);

	if (objectaddr == 0)
	{
		pushInt((jint) OBJECT_NOT_FOUND);
		return;
	}

	if (!IS_REF_SIMPLEOBJECT(objectaddr))
	{
		pushInt((jint) OBJECT_IS_NOT_REMOTE);
		return;
	}

	/* get address of class structure for this object */
	classAddr = get_object_class(objectaddr);

	if (!IS_ACCESS_REMOTE(classAddr))
	{
		pushInt((jint) OBJECT_IS_NOT_REMOTE);
		return;
	}

	/* Calculate the Class's remote method information table count Address */

	/* First, get past the public and package virtual method tables */
	remoteMethodInfo = classAddr + CR_MethodsTableStart + sizeof(jref) * (PF_R_U8((memref)(classAddr + CR_PublicMethods + MTH_NumMethods))) + sizeof(jref) * (PF_R_U8((memref)(classAddr + CR_PackageMethods + MTH_NumMethods)));


	/* Then get the count of the implemented interfaces */
	numInterfaces = PF_R_U8((memref)(classAddr + CR_Header)) & (u8) CRH_InterfaceCount;


	/* use the interface count to walk through all of the implemented interface info structures */
	for (i = 0; i < numInterfaces; i++)
	{
		remoteMethodInfo = remoteMethodInfo + IIT_FirstIndex + PF_R_U8((memref)(remoteMethodInfo + IIT_NumInterfaces)) * IIT_IndexSize;
	}

	/* method_count is remote_methods_table_count */
	method_count = (u8)nvmReadU8(remoteMethodInfo);
	/* move remoteMethodInfo address past count to the first remote_method_info
	 * structure.
	 */
	remoteMethodInfo += 1;

	for (i = 0; i < method_count; i++)
	{

		u16 methodIndex = i * 5; // the offset to each remote method info structure
		/* first get method hash and check if we have a match, if we do, return
		 * the address of this remote method info structure.
		 */
		if ((u16) nvmReadU16(
				(memref)(remoteMethodInfo + RM_remote_method_hash + methodIndex)) == methodID)
		{
			/* return the address of this remote method info structure */
#ifdef VM_BIT16
			pushInt((jint)((jint)(remoteMethodInfo + methodIndex) & (jint)0x0000FFFF));
#else
			pushInt((jint) (remoteMethodInfo + methodIndex));
#endif  //BIT16
			return;
		}
	}
	pushInt((jint) METHOD_NOT_FOUND);
}

void getReturnType()
{
	/* address of signature pool */
	memref signature;


	/* number of parameters in this remote method  */
	u8 count;
	u8 temp;	   // 局部变量 2012-2-3 chenyunhui
	/* the results */
	u16 results = (u16) OTHER_EXCEPTION;
	/* the method hash we are looking for */

	memref remoteMethodInfo = (u32)popAddress();
	u8 pkgID = getPackageIdForAddr(remoteMethodInfo);
	memref classCompOffset = getPackageComponentLocation(pkgID, (u8) COMPONENT_CLASS);

	signature = nvmReadU16((memref)
			(remoteMethodInfo + RM_remote_method_signature)) + classCompOffset + 2;
	/* get parameter count from signature. This is in nibbles. */
	if (signature == 0)
	{
		pushShort((s16)results);
		return;
	}

	/* if return type is a reference, the last five nibbles represents the return
	 * type, otherwise, the last nibble is the return type. So, must check the
	 * value of the fifth nibble from the end to see if it is a reference first.
	 */

	count = (u8)nvmReadU8(signature);

	if (count >= 5)
	{
		//comError
//		results = (0x00FF) & nvmReadU8((memref)((signature + (count+1) / 2) - 2 ));
        temp = (u8)nvmReadU8((memref)((signature + (count+1) / 2) - 2 ));
		results = (0x00FF) & temp;
		if (count % 2)
		{
			// odd
			results = (results >> 4) & RMI_ReturnType;
		}
		else
		{
			// even
			results = results & RMI_ReturnType;
		}

		if (results == Ref_Param)
		{
			pushShort((s16)results);
			return;

		}
	}

	/* Return type not reference, so return type is in last u8, first nibble
	 * if count odd, second if even.
	 */
	//comError
//	results = (0x00FF) & nvmReadU8((memref)(signature + (count+1) / 2));
    temp = (u8)nvmReadU8((memref)(signature + (count+1) / 2));
 	results = (0x00FF) & temp;
	if (count % 2)
	{
		// odd
		results = results >> 4;
	}
	else
	{
		// even
		results = results & RMI_ReturnType;
	}

	pushShort((s16)results);

}

void deleteAllTempArrays()
{

	memFillToRam((RAM_U8_P)Temporary_Array_BASE, Temporary_Array_SIZE, (u8) 0);
	resetTempObjManager();

}
void isAPIException()
{
	memref classAddr;
	u8 classPkgID;
	jref object = popReference();
	memref objectaddr;
	objectaddr = locateObjectInMemory(object);
	classAddr = get_object_class(objectaddr);

	classPkgID = getPackageIdForAddr(classAddr);

#ifdef API_PACKAGES_COUNT
	if (classPkgID < API_PACKAGES_COUNT)
#else
	if (classPkgID < 8)		 //API_PACKAGES_COUNT
#endif
	{
		pushShort((u16)true);
	}
	else
	{
		pushShort((u16)false);
	}
	return;
}

#define SIGNATURE_ERROR           0x0003;
#define PARAM_RESOURCES_ERROR     0x0004;

u16 rmi_error_code;

void getRMIErrorCode()
{
	pushShort((s16)rmi_error_code);
}

void rmi_invoker_common()
{
	jref objref;
	memref objrefaddr;
	memref signature;
	memref remoteMethodInfo;
	memref paramData;
	u8 count;
	u16 new_cc;
	u8 methodPkgID;
	u8 methodToken;
	memref methodAddr;
	u8 i;
	u8 preType;
	u8 type;
	u8 LC;	  
	u16 arraydes = 0;
	u8 array_header;
	/* util is used for offset into apdu buffer and for array parameter count */

	jref objID;
	u8 pkgID;
	memref classCompOffset;

	u8 util = (u8) popShort();
	memref buffer = locateObjectInMemory(popReference());
	objref = popReference();

	rmi_error_code = 0;
	objID = objref;

	objrefaddr = locateObjectInMemory(objref);
	remoteMethodInfo = (u32)popAddress();
	paramData = get_array_data(buffer) + util;
	LC = (u8)nvmReadU8((memref)(paramData - 5));	  
	array_header = OBJ_HRD_read_u8(buffer);


	/* firewall access checks */
#if JCVM__VM__FIRE_WALL
	checkRead(objrefaddr);
#endif  //__FIRE_WALL__
	/*
	 * get token and check if this is a sharable interface.
	 */
	/* get method token */
	methodToken = nvmReadU8(
			(memref)(remoteMethodInfo + RM_shareable_and_remote_method_token)) & RMI_Token;


	/* get signature pool */
	pkgID = getPackageIdForAddr(remoteMethodInfo);
	classCompOffset = getPackageComponentLocation(pkgID, (u8) COMPONENT_CLASS);

	signature = nvmReadU16((memref)
			(remoteMethodInfo + RM_remote_method_signature)) + classCompOffset + 2;
	/* get parameter count from method signature. This is in nibbles.
	 *  we need to reduce by one since the count includes the return type */
	count = (u8)nvmReadU8(signature);

	if (count >= 5)
	{
		u8 results;
		results = (0x00FF) & nvmReadU8((memref)((signature + (count+1) / 2) - 2 ));
		if (count % 2)
		{
			// odd
			results = (results >> 4) & RMI_ReturnType;
		}
		else
		{
			// even
			results = results & RMI_ReturnType;
		}

		if (results == Ref_Param)
		{
			count -= 4;
		}
	}
	count--;


	/* increment the signature past the parameter count */
	signature += 1;
	/* walk through the signature pool, parse the apdu buffer according to the signature pool
	 * information and push the parameters onto the stack
	 */

	pushShort((s16)objID);

	rmi_error_code = PARAM_RESOURCES_ERROR; /* in case an exception thrown during arrays creation */

	for (i = 0; i < count; i++)
	{
		/* get nibble of signature pool, pretype holds 2 nibles prior
		 * to obtaining 1 nible type for parameter
		 */
		preType = (u8)nvmReadU8((memref)(signature+(i/2)));

		if (i % 2)
		{
			/* if index is odd, we already have a u8, just need to mask second nibble */
			type = preType & RMI_ParameterType;
		}
		else
		{
			/* index is even, get u8 from signature */
			type = (preType >> 4) & RMI_ParameterType;
		}
		if ((type >= Boolean_Array_Param) && ((u8) nvmReadU8(paramData) == (u8) 0xff))
		{
			pushShort((u16)0);
			++paramData;
		}
		else
		{
			switch (type)
			{
			case Void_Param:
				break;
			case Boolean_Param:
				pushShort(nvmReadU8(paramData));
				paramData += SIZE_BOOLEAN;
				break;
			case Byte_Param:
				pushShort(nvmReadU8(paramData));
				paramData += SIZE_BYTE;
				break;
			case Short_Param:
				pushShort(nvmReadU16(paramData));
				paramData += SIZE_SHORT;
				break;
			case Int_Param:
				pushInt((s32)CODE_read_u32(paramData));
				paramData += SIZE_INT;
				break;
			case Ref_Param:
				pushReference(nvmReadU16(paramData));
				paramData += SIZE_REF;
				break;
				/* for all arrays, util is count of array elements */
			case Boolean_Array_Param:
				util = (u8)nvmReadU8(paramData);	
				arraydes = ++paramData;			
				if ((array_header & HEADER_JCRE_TransObj_IsTrans) == HEADER_JCRE_TransObj_IsTrans)
				{
	  				if (logicalChannelNums != 1)
                    {
                        if ((array_header & HEADER_JCRE_TransMode_Deselect) == HEADER_JCRE_TransMode_Deselect) {
                                // Clear On Deselect
                                arraydes =(u16) (paramData - dtrSpace_getDtrSpaceBase(selected_channel));
                        }
                    }
				}
				pushShort((s16)commonCreateTempArray((u8)AType_Boolean, util, arraydes));
				paramData += (util * SIZE_BOOLEAN);
				break;
			case Byte_Array_Param:
				util = (u8)nvmReadU8(paramData);
				arraydes = ++paramData;			
				if ((array_header & HEADER_JCRE_TransObj_IsTrans) == HEADER_JCRE_TransObj_IsTrans)
				{
	  				if (logicalChannelNums != 1)
                                        {
                                            if ((array_header & HEADER_JCRE_TransMode_Deselect) == HEADER_JCRE_TransMode_Deselect) {
                                                    // Clear On Deselect
                                                    arraydes =(u16) (paramData - dtrSpace_getDtrSpaceBase(selected_channel));
                                            }
                                        }
				}
				pushShort((s16)commonCreateTempArray((u8)AType_Byte, util, arraydes));
				paramData += (util * SIZE_BYTE);
				break;
			case Short_Array_Param:
				util = (u8)nvmReadU8(paramData);		 
				arraydes = ++paramData;			
				if ((array_header & HEADER_JCRE_TransObj_IsTrans) == HEADER_JCRE_TransObj_IsTrans)
				{
	  				if (logicalChannelNums != 1)
                                        {
                                            if ((array_header & HEADER_JCRE_TransMode_Deselect) == HEADER_JCRE_TransMode_Deselect) {
                                                    // Clear On Deselect
                                                    arraydes =(u16) (paramData - dtrSpace_getDtrSpaceBase(selected_channel));
                                            }
                                        }
				}
				pushShort((s16)commonCreateTempArray((u8)AType_Short, util, arraydes));
				paramData += (util * SIZE_SHORT);
				break;
			case Int_Array_Param:
				util = (u8)nvmReadU8(paramData);		
				arraydes = ++paramData;			
				if ((array_header & HEADER_JCRE_TransObj_IsTrans) == HEADER_JCRE_TransObj_IsTrans)
				{
	  				if (logicalChannelNums != 1)
                                        {
                                            if ((array_header & HEADER_JCRE_TransMode_Deselect) == HEADER_JCRE_TransMode_Deselect) {
                                                    // Clear On Deselect
                                                    arraydes =(u16) (paramData - dtrSpace_getDtrSpaceBase(selected_channel));
                                            }
                                        }
				}
				pushShort((s16)commonCreateTempArray((u8)AType_Int, util, arraydes));
				paramData += (util * SIZE_INT);
				break;
			default:
				break;
			}

		}

	}

	rmi_error_code = 0; /* resetting the code back to zero */


	/* check Lc .... */
	if (LC != (paramData - get_array_data(buffer) - 5))
	{
		rmi_error_code = SIGNATURE_ERROR;
		throw_error(SYSTEMEXCEPT_RMI_LC_MISMATCH);
	}

	new_cc = get_object_owner(objrefaddr);
	methodAddr = resolveVirtualMethod(objrefaddr, methodToken);

	methodPkgID = lastVMPkg;
	callmethod(methodAddr, methodPkgID, new_cc);
}

/**
 * A method for RMI which takes any form of array and copies its
 contents into a u8 array
 * writeArray(Object data, u8[] buffer, short offset)
 * where data is the generic array type, buffer is the target u8 array
 and offset is the
 * offset in the target array buffer
 */
void writeArray()
{
	u8 type = 0;
	memref value;
	u16 dataSize = 0;
	u16 offset = (u16)popShort();
	memref buffer = locateObjectInMemory((u16)popShort());
	memref dataRef = locateObjectInMemory((u16)popShort());


	/*if the data is null*/
	if (check_addr_null(dataRef))
	{
		check_bounds_interval(buffer, (s16)offset, 1);
		buffer = get_array_data(buffer) + offset;
		if (isInE2P(buffer))
		{
			nvmWriteU8Atomic(MEMtPN8(buffer), 0xFF);
		}
		else
		{
			PX_W_U8(buffer, 0xFF);
		}
		pushShort((s16)(offset+1));
		return;
	}

	/*
	 else check what type of array it is to check for array bounds
	 */
	if (isInE2P(dataRef))
	{
		type = TYPE(nvmReadU8(dataRef));
	}
	else
	{
		type = TYPE(PX_R_U8(dataRef));
	}

	switch (type)
	{
	case GC_BOOLEAN_ARRAY:
	case GC_BYTE_ARRAY:
	case GC_TRANSIENT_BOOLEAN_ARRAY:
	case GC_TRANSIENT_BYTE_ARRAY:
		dataSize = 1;
		break;
	case GC_SHORT_ARRAY:
	case GC_TRANSIENT_SHORT_ARRAY:
		dataSize = 2;
		break;
	case GC_INT_ARRAY:
	case GC_TRANSIENT_INT_ARRAY:
		dataSize = 4;
		break;
	default:
		pushShort((jshort)-1);
		return;
	}

	dataSize = get_array_length(dataRef) * dataSize;
	if ((offset < 0) || ((offset + dataSize + 1) > 255))
	{
		throw_error(ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION);
	}

	value = get_array_data(dataRef);
	buffer = get_array_data(buffer) + offset;

	if (isInE2P(buffer))
	{
		nvmWriteU8Atomic(MEMtPN8(buffer), (u8) get_array_length(dataRef));
		nvmCopyAtomic((memref) (buffer + 1), MEMtPX8(value), dataSize);
	}
	else
	{
		PX_W_U8(buffer, (u8)get_array_length(dataRef));
		RAM_writeArray((u8 *) (buffer + 1), MEMtPX8(value), dataSize);
	}

	/*return the new offset in the buffer array*/
	pushShort((jshort)(dataSize+offset+1));
}

#endif      //__RMI_SUPPORTED__//@@@@ end of RMI
/****************************************************************************************************************
 *
 *the following functions are to support channel manager
 *
 ******************************************************************************************************************/

/**
 * returns the ID of the package covering a range of addresses that contain the address
 * provided to this method as a parameter.
 */
void getPkgIdForAddress()
{
	u8		D_A  pkgID;
	PN8		D_A  addr;

	addr	= MEMtPN8(popAddress());
	pkgID	= getPackageIdForAddr(addr);

	pushShort(U8t16(pkgID));
}
#if MAX_LOGICAL_CHANNELS != 1
void channelManageNM(void)
{
	u8 channelId;
	u8 operationId;

	operationId = U16t8(popShort());
	channelId = U16t8(popShort());
	/*@ -typedefparam @*/
	/*@ -discast @*/
	/*@ -type @*/
	switch (operationId)
	{
		case U32t8(OP_CHANNEL_CLOSE):
		pushShort((jshort)(closeChannel(channelId)));
		break;
		case U32t8(OP_CHANNEL_OPEN):
		pushShort((jshort)openChannel(channelId));
		break;
		case U32t8(OP_CHANNEL_OPEN_AUTOSELECT):
		channelId = getAvailableChannel();
		if (channelId == U32t8(0xFF))
		{
			pushShort(S16tU16(-1));
		}
		else
		{
			openChannel(channelId);
			pushShort((jshort)channelId);
		}
		break;
		default:
		pushShort(S16tU16(-1));
	}/*@ +type @*/
	/*@ +discast @*/
	/*@ +typedefparam @*/
}
#endif

#if !GP__VERSION__FOR_MINI_MASK
void getChannelStatusNM()
{
	u8		channelId;

	channelId	= U16t8(popShort());
	if(logicalChannelNums != 1)
	{
		pushShort(U8t16(getChannelState(channelId)));
	}
	else
	{
		pushShort(CHANNEL_OPEN);	
	}
}
#endif
/**
 * modify by xuxiaojun@20220513 根据分层代码 第一次代码评审(20220415).xlsx 将G_CurExecutingProfileIndex宏定义为0xFF
 */
void getContextStatusNM()
{					
	boolean		D_A  checkApp;
	u16			D_A  contextId;
			 
	checkApp	= U16t8(popShort());  //wzq
	contextId	= popShort();
	pushShort(U8t16(getContextStatus(contextId, checkApp,G_CurExecutingProfileIndex)));
}

void manageAppFlagNM(void)
{
	u8 		D_A     appFlagManageType;
	u8 		D_A     appID;

	appFlagManageType	= U16t8(popShort());
	appID				= U16t8(popShort());
	pushShort(U8t16(manageAppFlag(appID, appFlagManageType)));
}
/*@ -typedefparam @*/
/*@ -type @*/
void getNewIDNM()
{
	s8 		newID;  //类型 转换为了保证0xFF能正确表示-1

	newID		= getNewID();
	pushShort(newID);
}
/*@ +type @*/
/*@ +typedefparam @*/
void getIndexByIDNM()
{
	u8 		appID;

	appID	= U16t8(popShort());
	pushShort(getIndexByID(appID));
}

void addEntryNM()
{
	boolean		D_A  zIsInst;
	jref		D_A  classaid;

	zIsInst		= U16t8(popShort());   //wzq
	classaid	= popReference();

	pushReference(addEntry(classaid, zIsInst));
}

void getEntryByInstAIDNM()
{
	jref		D_A  classaid;
	classaid	= popReference();
	pushReference(getEntryByInstAID(classaid));
}
/**
*EPX产业化/STM_ST33G1M2/DEVELOP
*/
void SearchEntryByInstAIDNM()
{
	jref		D_A  classaid;
	classaid	= popReference();
	pushReference(SearchEntryByInstAID(classaid));
}

void getPkgContextNM()
{
	u8			PkgID;

	PkgID		= U16t8(popShort());
	pushReference(getPkgContext(PkgID));
}

void getPkgIdByContextNM(void)
{
	u8		D_A 	PkgContent;

	PkgContent	= U16t8(popShort());
	pushReference(U8t16(getPkgIdByContext(PkgContent)));
}

boolean compareRemainedSpace(u16 wLen_, u16 wFlag_)		SMALL_MODE
{
	if (wLen_ == 0xFFFF)
	{
		return true;
	}

	if (wFlag_ == 0x80)//check: Is RAM space enough?
	{/*@ -type @*/
		return (tranAllocate_rtrAvailable() >= U16t32(wLen_));

	}/*@ +type @*/

	//check: Is EEPROM space enough?
	if (wFlag_ == 0)
	{
		wFreeEESize = NvmManager_GetAvailMemory();
	}

	if (wFreeEESize >= wLen_)
	{
		wFreeEESize	= U32_S_U16(wFreeEESize, wLen_);
		return true;
	}

	return false;
}


u8 nativeMethod_GetProtocol(void)
{
	u8	protocolType;
	
	protocolType	= apdu_GetProtocol();
	
	if (protocolType == APDU_PROTOCOL_T0)
	{
		protocolType	= 0x00;
	}
	else if(protocolType == APDU_PROTOCOL_T1)
	{
		protocolType	= 0x01;
	}
#if JCVM__IO__SWP
	else if(protocolType == PROTOCOL_SWP)
	{
		protocolType = io_GetSwpProtocolType();
	}
#endif
#if JCVM__IO__SPI
	else if(protocolType == PROTOCOL_SPI)
	{
		protocolType = 0xD0 | PROTOCOL_T1;
	}
#endif
#if JCVM__IO__I2C
	else if(protocolType == PROTOCOL_I2C)
	{
		protocolType = 0xC0 | PROTOCOL_T1;
	}
#endif
	else
	{
		protocolType	= 0x81;
	}
	
	return protocolType;
}
	
u16 nativeMethod_GetCurrentAPDUBuffer(void) {
#if JCVM__VM__FIRE_WALL
	checkAPDUObjectAccess();
#endif
	return PN_R_U16(NM_APDUBuffer_JC);
}

u16 nativeMethod_GetCurrentAPDU(void)
{
#if JCVM__VM__FIRE_WALL
	checkAPDUObjectAccess();
#endif
	return PN_R_U16(OBJECT_theAPDU_OFFSET);
}

void nativeMethod_SendBytes(void) {
	u16  D_A  len;
	u16  D_A   bOff;

	len		= popShort();
	bOff	= popShort();

	apdu_ApduSendBytes(bOff, len);
}

void nativeMethod_SendBytesLong(void) {
	u16		D_A		len;
	u16		D_A		bOff;
	jref	D_A		outData;
	PN8		D_A		ArrayAddr;

	len			= popShort();
	bOff 		= popShort();
	outData		= popReference();

	ArrayAddr 	= MEMtPN8(locateObjectInMemory(outData));

	check_null(ArrayAddr);

#if JCVM__VM__FIRE_WALL
	checkPrevContextAccess(ArrayAddr);
#endif

	check_bounds_interval(ArrayAddr, bOff, len);

	//ArrayAddr = (memref) (get_array_data(ArrayAddr) + bOff);
	ArrayAddr	= MEMtPN8(U32_A_U16(get_array_data(ArrayAddr) , bOff));

	apdu_SendBytesLong(ArrayAddr, 0, len);
}

void nativeMethod_SetOutgoingAndSend(void) {
	u16   D_A   len;
	u16   D_A   bOff;

	len		= popShort();
	bOff	= popShort();

	apdu_ApduSetOutgoingAndSend(bOff, len);
}

#define  ISCOMMANDCHAININGCLA   1
#define  ISISOINTERINDUSTRYCLA  2
#define  ISSECUREMESSAGINGCLA   3
#define  ISVALIDCLA             4
void nativeMehtod_GetCLA(void)
{
	u8		type;

	type	   = U16t8(popShort());
	switch(type)
	{
		case ISCOMMANDCHAININGCLA:
			pushShort(U8t16(apdu_IsCommandChainingCLA()));
			break;
		case ISISOINTERINDUSTRYCLA:
			pushShort(U8t16(apdu_IsISOInterindustryCLA()));
			break;
		case ISSECUREMESSAGINGCLA:
			pushShort(U8t16(apdu_IsSecureMessagingCLA()));
			break;
		case ISVALIDCLA:
			pushShort(U8t16(apdu_IsValidCLA()));
			break;
		default:
			break;
	}
}

u16 nativeMethods_readSecurityCodeDataNative(void)
{
	PF8		D_A  buf;
	buf			= getArrayDataWithCheck(9);
#if JCVM__EDEP
	AppAPI_ConstructAuthCode(buf);
#endif
	return 0x00;
}

#if JCVM__IO__MIFARE
#define MIFARE_PASSWORD_READ    3
#define MIFARE_PASSWORD_WRITE   5
/*
void MF_readWrite_NM()
{
	u8 far* InBuffer;
	u8 far* MF_keyBuffer;
	u8 result = 0;
	u16 Sector = (u16) popShort();
	u16 inOffset = (u16) popShort();
	memref inBuffer = locateObjectInMemory(popReference());
	u8 mode = (u8) popShort();


	check_null(inBuffer);

	check_bounds_interval(inBuffer, inOffset, 32);

	MF_keyBuffer = (u8 far *) (MEMtPX8(get_array_data(inBuffer)) + (u32) inOffset);
	InBuffer = (u8 far *) (MF_keyBuffer + 16L);

	switch (mode)
	{
	case MIFARE_PASSWORD_READ:
		result = MF_Read(MF_keyBuffer, Sector, InBuffer);
		break;
	case MIFARE_PASSWORD_WRITE:
		result = MF_write(MF_keyBuffer, Sector, InBuffer);
		break;
	default:
		result = 0xff;
		break;

	}
	pushShort(result);
}
*/

void MF_readWrite_NM(void)
{
	u16		D_A		blockNum;
	PM8		D_A		auth_key;
	u16		D_A		data_length;
	PM8		D_A		dataBuffer;
	u8		D_A		mode;
	u8		D_A		result;

	blockNum		= popShort();
	auth_key		= getArrayDataWithCheck(8);
	data_length		= popShort();
	dataBuffer		= getArrayDataWithCheck(data_length);
	mode			= U16t8(popShort());   //wzq
	result			= 0xFF;
//	/*check Block Num beyond the mark*/
//	if(CheckBlocknum(other_block))
//	{
//		pushShort(0xFE);
//		return;
//	}

//	//Mifare disable
//	if (MIFARE_SWITCH != 0x00)
//	{
//		pushShort(0xFE);
//		return;
//	}

	switch(mode)
	{
	case MIFARE_PASSWORD_READ:
//		  if((other_block  > 128) && (other_block & 0x0F) == 0x0F )
//			{
//					pushShort(0xFE);
//			          return;
//			}
		result		= mifare_Read(U16t8(blockNum), dataBuffer, U16t8(data_length), auth_key);
		break;
	case MIFARE_PASSWORD_WRITE:
		result		= mifare_write(U16t8(blockNum), dataBuffer, U16t8(data_length), auth_key);
		break;
	default:
		break;
	}
	pushShort(result);
}

#endif

#define    COLD_ATR_WHOLE_CHARS           (u8)0x01
#define    COLD_ATR_HISTORICALS           (u8)0x02
#define    WARM_ATR_WHOLE_CHARS           (u8)0x03 	
#define    WARM_ATR_HISTORICALS           (u8)0x04
#define    DEFAULT_HOST_COMM_TYPE         (u8)0x05
#define    DEFAULT_RF_COMM_TYPE           (u8)0x06
#define    FDT           		  		  (u8)0x07   	
#define    DEFAULT_CURRENT_LIMITATION     (u8)0x08   	
#define    CLF_TO_SE_BUFFERSIZE           (u8)0x09   	
#define    ADDITIONAL_MAXBYTES_SEND       (u8)0x0A    	
#define    DEFAULT_SE_1                   (u8)0x0B
#define    DEFAULT_SE_2                   (u8)0x0C
#define    DEFAULT_SE_NUMBER              (u8)0x0D
#define    CLF_CONFIG_SIZE                (u8)0x0E
#define    MIN_SELECT_FWT                 (u8)0x0F
#define    SSD_NESTING_SWITCH             (u8)0x10
#define    SCPOPTION                      (u8)0x11
#define    GETDATA_MANDATING_SC_SWITCH    (u8)0x12
#define    STRICT_EMV_MODE                (u8)0x13
#define    PATCH_VERIFICATION_KEYS        (u8)0x14
#define    HOST_CLOCK_THRESHOLD_LEVEL     (u8)0x15

#if  0 // not used ,if used ,give Define
void GetCLFConfigSizeNM(void)
{
	pushShort(memReadU16(MEMtPM8(CLFCONFIGSIZE)));
}

void SSDSupportNestingNM(void)
{
	pushShort(JcvmConfig_GetSupportSSDNesting()); //modify by xuxiaojun@20211009 如果VM_CONFIG_START不是0的话，读取的SUPPORT_SSD_NESTING地址是不对的
}
void privateNativeMethod(u8 methodNumber)
{

}

/**********************End of New Native Methods for the 64K implementation***********************/
void DeletePrePackageNeedNM(void)
{
	pushShort(DELETE_PRELOADEDAPPLETS);	
}
#endif

#define READPROTOCOL	0		// read protocol params
#define WRITEPROTOCOL	1	    // write protocol params
void GetNextAppEntryIdxByPriorityNM(void)
{
	pushShort(	GetNextAppEntryIdxByPriority(popShort()) );
}
void isHostActiveNM(void)
{
	pushShort(0);			//return false;
}
void isActiveNM(void)
{
	//u8 index = (u8)popShort();
	pushShort(0);			//return false;
}
void setMifareStatusNM(void)
{
	/*u8 index = (u8)popShort();
	u8 kb =    (u8)popShort();
	u8 status = (u8)popShort();*/
	pushShort(0);			//return false;
}
void deleteMifareNM(void)
{
	/*u8 index = (u8)popShort();
	u8 kb =    (u8)popShort();*/
	pushShort(0);			//return false;
}
void initISDPrivilegeNM(void)
{
	memref privArray = PNtMEM(getArrayDataWithoutCheck());
	//memCopyAtomic(MEMtPN8(privArray), (PM8)ISD_PRIV, PRIVMAXBYTES);
	memCopyAtomic(MEMtPM8(privArray), MEMtPM8(JcvmConfig_GetVmConfigAddr(ISD_PRIV)), PRIVMAXBYTES);
}

void setISDPrivilegeNM(void)
{
	memref privArray  = PNtMEM(getArrayDataWithoutCheck());
	memCopyAtomic(MEMtPM8(JcvmConfig_GetVmConfigAddr(ISD_PRIV)), MEMtPF8(privArray), PRIVMAXBYTES);
}
#if JAVA_CARD_UICC
/*ISD的tar值，SE区的ISD*/
// modify by xuxiaojun@20220318 分层
void setSDTarInfoNM(void)
{
	u16 sw;
	u8  appId  = popShort();
	u16 tarInfoLen  = popShort();
	
	memref tarInfoBuff  = PNtMEM(getArrayDataWithoutCheck());

	CacheFlush();// sdRef可能在cache里 后续查找会死卡

	sw = SaveSdRAMTar(MEMtPF8(tarInfoBuff),tarInfoLen,appId);

	if(sw != 0x9000)
	{
		throw_error(sw);
	}
}
#endif
#if GP__HCI_API__FOR_MASK

u8 nativeMethod_HciNoParaProcess(void)
{
	switch((u8)popShort())
	{
		case 0x00:  // isHeading
			Swp_Hci_IsHeader();
			break;
		case 0x01:  // isComplete
			Swp_Hci_IsComplete();
			break;
		case 0x02:  // getType
			Swp_Hci_GetInstructionType();
			break;
		case 0x03:  // getInstruction
			Swp_Hci_GetInstruction();
			break;
		case 0x04:  // getReceiveOffset
			break;
		case 0x05:  // getReceiveLength
			Swp_Hci_GetDataLength();
			break;
		case 0x06:  // getRFType  include cardRFtype, readerRFType
			Swp_Hci_GetRFType();
			break;
		case 0x07:  // getPowerMode
			Swp_Hci_GetPowerMode();
			break;
		case 0x08:  // prepareAndSendCnnEvent
			Swp_Hci_prepareAndSendConnectivityEvent();
			break;
		case 0x09:  // getListenerEventType
			//Swp_Hci_GetListenerEvent();
			break;
		case 0x0A: //Swp_Hci_RestartReaderModeProcedure
			Swp_Hci_RestartReaderModeProcedure();
			break;
		case 0x0B:
			Swp_Hci_EventReaderRequestProcedure();
			break;
		case 0x0C:
			Swp_getServiceId();
			break;
		case 0x0D:
			Swp_getEventType();
			break;
	}

}

u8 nativeMethod_HciOneParaProcess(void)
{
	switch((u8)popShort())
	{
		case 0x00:  // prepareAndSendGetParameterCommand
			Swp_Hci_GetParaCommand();
			break;
		case 0x01:
			Swp_Hci_IsHciServiceAvaliable();
		default:
			break;
	}
}

void nativeMethod_HciMulParaProcess(void)
{
	switch((u8)popShort())
	{
		case 0x01: //prepareAndSendSendDataEvent
			Swp_Hci_SendDataEvent();
			break;
		
		case 0x02: //prepareAndSendWriteXchgDataCommand
			Swp_Hci_SendWriteXchgDataCommand();
			break;
		default:
			break;
	}

}

//void nativeMethod_GetReceiveBufferHCI(void)
//{
//	getReceiveBufferHCI();
//}

//void nativeMethod_Swp_Hci_TransactionEvent_6paras(void)
//{
//	u16 parametersLen;
//	u16 parametersOffset;
//	jref bArrayRef;
//	jref aidArrayRef;
//	memref bArrayAddr;
//	memref parametersAddr;
//	u16 aidLen;
//	u16 aidOffset;
//	memref aidArrayAddr;
//	memref aidAddr;
//	
//	parametersLen = popShort();
//	parametersOffset = popShort();
//	bArrayRef = popReference();
//	aidLen = popShort();
//	aidOffset = popShort();
//	aidArrayRef = popReference();

//	bArrayAddr = locateObjectInMemory(bArrayRef);
//	parametersAddr =  get_array_data(MEMtPF8(bArrayAddr));

//	aidArrayAddr = locateObjectInMemory(aidArrayRef);
//	aidAddr = get_array_data(MEMtPF8(aidArrayAddr));

//	check_null(MEMtPF8(bArrayAddr));

//	check_bounds_interval(MEMtPF8(bArrayAddr), parametersOffset, parametersLen);

//	check_null(MEMtPF8(aidArrayAddr));

//	check_bounds_interval(MEMtPF8(aidArrayAddr), aidOffset, aidLen);

//	io_Swp_Hci_TransactionEvent_6paras(MEMtPM8(parametersAddr),parametersOffset,parametersLen,MEMtPM8(aidAddr),aidOffset,aidLen);
//}


//void nativeMethod_Swp_Hci_TransactionEvent_3paras(void)
//{
//	u16 parametersLen;
//	u16 parametersOffset;
//	memref bArrayAddr;
//	memref parametersAddr;
//	parametersLen = popShort();
//	parametersOffset = popShort();
//	bArrayAddr = locateObjectInMemory(popReference());
//    parametersAddr = get_array_data(MEMtPF8(bArrayAddr));

//	check_null(MEMtPF8(bArrayAddr));

//	check_bounds_interval(MEMtPF8(bArrayAddr), parametersOffset, parametersLen);
//	io_Swp_Hci_TransactionEvent_3paras(MEMtPM8(parametersAddr),parametersOffset,parametersLen);
//}
#endif //HCI_API
#if (GP__AMENDMENT__C__FOR_MASK || JCVM__IO__SWP )
void setRFCommunication()
{
		u8		protocolSwitch;
		
		protocolSwitch		=	U16t8(popShort()); //protocolSwitch 1: 表示DISABLE, 0: 表示ENABLE
	
		nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(RF_COMMUNICATION_SWITCH)), protocolSwitch);
		E2P_stream_flush();	
}

void getRFCommunication()
{/*@ -discast @*/
	/*@ -typedefparam @*/
	/*@ -type @*/
	
//	if (Swp_UiccContactlessState == 1)
//	{
//		pushShort((jshort)0x01);
//	}
//	else
	{
		pushShort((jshort)(PN_R_U8(JcvmConfig_GetVmConfigAddr(RF_COMMUNICATION_SWITCH))));
	}
	/*@ +type @*/
	/*@ +typedefparam @*/
}/*@ +discast @*/
/**
*added by jiangxiaohui:优化性能, EPX产业化/THC_THD89/Branch/EPX_THC_Mobile5G/Develop
*/
void addClEntryNM()
{
	pushShort((jshort)addClEntry(popShort()));
}

#endif
#if  0 // not used ,if used ,give Define
void getUID_NM(void)
{
	u16 				uidSize;
	u16 				offset;
	u16	 				mode;
	memref 				dataOutBuf;

	uidSize 			= 0;
	mode 				= popShort();
	offset 				= popShort();
	dataOutBuf 			= locateObjectInMemory(popReference());

	
	dataOutBuf 			= U32_A_U32(get_array_data(MEMtPF8(dataOutBuf)), U16t32(offset));
	
	if (mode == 0) //get Mifare UID
	{
		uidSize = 4;
		memCopyNoAtomic(MEMtPM8(dataOutBuf), MEMtPM8(MIFARE_UID), uidSize);
	}
	else if(mode == 1) //get Desfire UID
	{
		uidSize = 7;
		memCopyNoAtomic(MEMtPM8(dataOutBuf), MEMtPM8(DESFIRE_UID), uidSize);
	}
	
	pushShort(uidSize);
}
short getMsgSecuredData(void)
{
	u16 			bOffset;
	memref 			bBufferArray;

	bOffset 		= popShort();
	bBufferArray 	= locateObjectInMemory(popReference());
	bBufferArray 	= U32_A_U32(get_array_data(MEMtPF8(bBufferArray)), U16t32(bOffset));
	memCopyToRam(MEMtPM8(bBufferArray), MEMtPM8(AUTH_CODE), 9);
	return 1;
}
#endif

void setCurrentInvokeApplet(void)
{
	u8 				bAppid;
	u16 			isSetInvokeApplet;

	isSetInvokeApplet = popShort();
	bAppid 			  = U16t8(U16_AND_U16(cc, 0x00FF)) ;

	if(isSetInvokeApplet == 1)		
		g_bSDId[selected_channel] = bAppid;
	else
		g_bSDId[selected_channel] = 0;
}


boolean SetRemoteProcessRes(u16 offset, u16 len)
{
#if JAVA_CARD_UICC // modify by xuxiaojun@20220330 分层
	if(bRamBuffer[PROCESS_LEVEL] == ROCESS_LEVEL_OTA )
	{
		//bShortBuffer[REMOTE_APDU_RES_LEN_OFF] = htoj16(offset);
		PXB_W_U16(MEMtPX8(&bShortBuffer[REMOTE_APDU_RES_LEN_OFF]), offset);
		//bShortBuffer[REMOTE_PROCESS_RES_LEN] = htoj16(len);		
		PXB_W_U16(MEMtPX8(&bShortBuffer[REMOTE_PROCESS_RES_LEN]), len);
		return true;
	}
#endif    
	return false;
}
void GetAlgVersion(void)
{
	u16  datalen;
	memref src = PNtMEM(getArrayDataWithoutCheck());
#if GW_PRIVATE_GM_CRYPTO	// modify by weipingjiao@20220520 将HAL__PATCH修改为GW_PRIVATE_GM_CRYPTO
	#if UICC_FUCTION_PRIVATEAUTH
	datalen = FunctionList.Alg_GetVersion_entry((u8*)(src));
	#endif
#endif
	pushShort(datalen);
	
}
void GetChipIDdata(void)
{
	memref src = PNtMEM(getArrayDataWithoutCheck());
	u16  datalen;
	
	datalen = 0; // remove warning
	
	//datalen = GetChipIDdata_boot((u8*)(src));
	
	pushShort(datalen);
	
}

void SeInfoNM(void)
{
#define READ_DATA       0x00
#define WRITE_DATA      0x01

#if !RUN_IN_SIMULATOR

	u16 desOff = popShort();
	u16 datalen = popShort();
	u16 bufOff = (u16)popShort();
	memref buf = locateObjectInMemory(popReference());
	u8 mode = (u8)popShort();
	u8 result;
	
	buf = (memref)((u8*)(get_array_data((u8 *)buf)  + (u32)bufOff));

	result=sysConfig_SeInfoFun((u8*)buf, desOff,datalen ,mode);
	
	pushShort(result);
#endif
}
/*
*@description:NM_GetCOSVersionData
×
*@param      :void
*
*@return     :void
*
*@history    :created by xuxiaojun@20190410
*            :modify by xuxiaojun@20220318 分层
*            :move to javaUiccAPI.c
*/
//void NM_GetCOSVersionData(void)
/*
*@description:NM_GetCrcOrScidInfo
×
*@param      :void
*
*@return     :void
*
*@history    :created by xuxiaojun@20190410
*/

void NM_GetCrcOrScidInfo(void)
{
#define CRC_VERIFY  	0x00
#define GET_SCIDINFO  	0x01
#if !RUN_IN_SIMULATOR
	u8 lr;

	if(CARD_TESTMODE_OR_NOT)//mdofiy by liguiyu@20220111 Test mode   1.东信/apdu命令要求/无效ins遍历/6.1.2.13.1(2个错): 遍历ins测试时，ins=0x34时，期望值：6d00
	{
		throw_error(SW_INS_NOT_SUPPORTED);  //该指令生产时要用  但是外场测试不允许有  
	}
	
	if(APDUBuffer[OFFSET_P1]== CRC_VERIFY)
	{
	   APDUBuffer[0]=coscrc_CalcVerify(COSCRC_VERIFY);
		apdu_ApduSetOutgoingAndSend(0, 1);
	}
	else if(APDUBuffer[OFFSET_P1]== GET_SCIDINFO)
	{
		lr=APDUBuffer[OFFSET_LC];
		
		sysConfig_SeInfoFun(APDUBuffer, APDUBuffer[OFFSET_P2],APDUBuffer[OFFSET_LC] ,0x00);
	   apdu_ApduSetOutgoingAndSend(0, lr);
	}
	else
	{
		throw_error(SW_INS_NOT_SUPPORTED);
	
	}
	
#endif
}
void ClearrfFirstSelectCmdFlag(void)
{
	if (isInRam(bRamBuffer))
	{
		bRamBuffer[RF_FIRST_SELECT_CMD_FLAG] = 0;
	}
}
#if GP__AMENDMENT__H__FOR_MASK
void Upgrade_BoolElementWrite()
{
	PM8		D_A		outBuff;
	//u16		D_A		outOffset;  //remove warning,dengyiwen,2022/9/22
	boolean value = (boolean)popShort();
	outBuff = getArrayDataWithCheck(1);
	memWriteU8NoAtomic(outBuff, value);

}

 boolean Upgrade_BoolElementRead()
 {
	return (boolean)(memReadU8(MEMtPM8(getArrayDataWithCheck(1))));
 }
#endif
#if UICC__OPERATOR__CHINAMOBILE
void isUsimADF_NM(void)
{
	memref    X_A     aid;
	u16    X_A     offset;
	u8     X_A     aidLen;

	aidLen = popReference();
	offset = popReference();
	aid = get_array_data(MEMtPF8(locateObjectInMemory(popReference())));

	pushShort(IsUsimADF(MEMtPF8(aid),offset,aidLen));
}
#endif

