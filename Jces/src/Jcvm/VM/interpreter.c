/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*
 * @(#)interpreter.c	1.38 03/06/06
 */
#include "interpreter.h"

#include "profile.h"
#include "mask.h"
#include "memory.h"
#include "stack.h"
#include "method.h"
#include "except.h"
#include "object.h"
#include "anomaly.h"
#include "context.h"
#include "native_interface.h"
#include "ob_manager.h"
#include "packageloc.h"
#include "global_variable.h"
#include "lifeCycle.h"
#include "BootErrorHandle.h"
#include "jcvmConfigMethod.h"
/*
 * Bytecodes
 */
#include "bc_arith.h"
#include "bc_array.h"
#include "bc_const.h"
#include "bc_control.h"
#include "bc_field.h"
#include "bc_load.h"
#include "bc_object.h"
#include "bc_return.h"
#include "bc_stack.h"
#include "bc_static.h"
#include "bc_store.h"
#include "MemBackUp.h"

#include "ecpReadWrite.h"
#include "ecpOperation.h"

#include        <setjmp.h>
//#include "VMDebug.h"

typedef void (*bytecode_t)();


static ROM_ARRAY(bytecode_t, bytecode_ptr) =
//static code bytecode_t bytecode_ptr[] =
{ _nop, /* 0 */
_aconst_null, /* 1 */
_sconst_m1, /* 2 */
_sconst_0, /* 3 */
_sconst_1, /* 4 */
_sconst_2, /* 5 */
_sconst_3, /* 6 */
_sconst_4, /* 7 */
_sconst_5, /* 8 */
_iconst_m1, /* 9 */
_iconst_0, /* 10 */
_iconst_1, /* 11 */
_iconst_2, /* 12 */
_iconst_3, /* 13 */
_iconst_4, /* 14 */
_iconst_5, /* 15 */
_bspush, /* 16 */
_sspush, /* 17 */
_bipush, /* 18 */
_sipush, /* 19 */
_iipush, /* 20 */
_aload, /* 21 */
_sload, /* 22 */
_iload, /* 23 */
_aload_0, /* 24 */
_aload_1, /* 25 */
_aload_2, /* 26 */
_aload_3, /* 27 */
_sload_0, /* 28 */
_sload_1, /* 29 */
_sload_2, /* 30 */
_sload_3, /* 31 */
_iload_0, /* 32 */
_iload_1, /* 33 */
_iload_2, /* 34 */
_iload_3, /* 35 */
_aaload, /* 36 */
_baload, /* 37 */
_saload, /* 38 */
_iaload, /* 39 */
_astore, /* 40 */
_sstore, /* 41 */
_istore, /* 42 */
_astore_0, /* 43 */
_astore_1, /* 44 */
_astore_2, /* 45 */
_astore_3, /* 46 */
_sstore_0, /* 47 */
_sstore_1, /* 48 */
_sstore_2, /* 49 */
_sstore_3, /* 50 */
_istore_0, /* 51 */
_istore_1, /* 52 */
_istore_2, /* 53 */
_istore_3, /* 54 */
_aastore, /* 55 */
_bastore, /* 56 */
_sastore, /* 57 */
_iastore, /* 58 */
_pop, /* 59 */
_pop2, /* 60 */
_dup, /* 61 */
_dup2, /* 62 */
_dup_x, /* 63 */
_swap_x, /* 64 */
_sadd, /* 65 */
_iadd, /* 66 */
_ssub, /* 67 */
_isub, /* 68 */
_smul, /* 69 */
_imul, /* 70 */
_sdiv, /* 71 */
_idiv, /* 72 */
_srem, /* 73 */
_irem, /* 74 */
_sneg, /* 75 */
_ineg, /* 76 */
_sshl, /* 77 */
_ishl, /* 78 */
_sshr, /* 79 */
_ishr, /* 80 */
_sushr, /* 81 */
_iushr, /* 82 */
_sand, /* 83 */
_iand, /* 84 */
_sor, /* 85 */
_ior, /* 86 */
_sxor, /* 87 */
_ixor, /* 88 */
_sinc, /* 89 */
_iinc, /* 90 */
_s2b, /* 91 */
_s2i, /* 92 */
_i2b, /* 93 */
_i2s, /* 94 */
_icmp, /* 95 */
_ifeq, /* 96 */
_ifne, /* 97 */
_iflt, /* 98 */
_ifge, /* 99 */
_ifgt, /* 100 */
_ifle, /* 101 */
_ifnull, /* 102 */
_ifnonnull, /* 103 */
_if_acmpeq, /* 104 */
_if_acmpne, /* 105 */
_if_scmpeq, /* 106 */
_if_scmpne, /* 107 */
_if_scmplt, /* 108 */
_if_scmpge, /* 109 */
_if_scmpgt, /* 110 */
_if_scmple, /* 111 */
_goto, /* 112 */
_jsr, /* 113 */
_ret, /* 114 */
_stableswitch, /* 115 */
_itableswitch, /* 116 */
_slookupswitch, /* 117 */
_ilookupswitch, /* 118 */
_areturn, /* 119 */
_sreturn, /* 120 */
_ireturn, /* 121 */
_return, /* 122 */
_getstatic_a, /* 123 */
_getstatic_b, /* 124 */
_getstatic_s, /* 125 */
_getstatic_i, /* 126 */
_putstatic_a, /* 127 */
_putstatic_b, /* 128 */
_putstatic_s, /* 129 */
_putstatic_i, /* 130 */
_getfield_a, /* 131 */
_getfield_b, /* 132 */
_getfield_s, /* 133 */
_getfield_i, /* 134 */
_putfield_a, /* 135 */
_putfield_b, /* 136 */
_putfield_s, /* 137 */
_putfield_i, /* 138 */
_invokevirtual, /* 139 */
_invokespecial, /* 140 */
_invokestatic, /* 141 */
_invokeinterface, /* 142 */
_new, /* 143 */
_newarray, /* 144 */
_anewarray, /* 145 */
_arraylength, /* 146 */
_athrow, /* 147 */
_checkcast, /* 148 */
_instanceof, /* 149 */
_sinc_w, /* 150 */
_iinc_w, /* 151 */
_ifeq_w, /* 152 */
_ifne_w, /* 153 */
_iflt_w, /* 154 */
_ifge_w, /* 155 */
_ifgt_w, /* 156 */
_ifle_w, /* 157 */
_ifnull_w, /* 158 */
_ifnonnull_w, /* 159 */
_if_acmpeq_w, /* 160 */
_if_acmpne_w, /* 161 */
_if_scmpeq_w, /* 162 */
_if_scmpne_w, /* 163 */
_if_scmplt_w, /* 164 */
_if_scmpge_w, /* 165 */
_if_scmpgt_w, /* 166 */
_if_scmple_w, /* 167 */
_goto_w, /* 168 */
_getfield_a_w, /* 169 */
_getfield_b_w, /* 170 */
_getfield_s_w, /* 171 */
_getfield_i_w, /* 172 */
_getfield_a_this, /* 173 */
_getfield_b_this, /* 174 */
_getfield_s_this, /* 175 */
_getfield_i_this, /* 176 */
_putfield_a_w, /* 177 */
_putfield_b_w, /* 178 */
_putfield_s_w, /* 179 */
_putfield_i_w, /* 180 */
_putfield_a_this, /* 181 */
_putfield_b_this, /* 182 */
_putfield_s_this, /* 183 */
_putfield_i_this, /* 184 */
//_ecpIllegal_bytecode, /* 185 */
//_ecpIllegal_bytecode, /* 186 */
//_ecpIllegal_bytecode, /* 187 */
//_ecpIllegal_bytecode, /* 188 */
//_ecpIllegal_bytecode, /* 189 */
//_ecpIllegal_bytecode, /* 190 */
//_ecpIllegal_bytecode, /* 191 */
//_ecpIllegal_bytecode, /* 192 */
//_ecpIllegal_bytecode, /* 193 */
//_ecpIllegal_bytecode, /* 194 */
//_ecpIllegal_bytecode, /* 195 */
//_ecpIllegal_bytecode, /* 196 */
//_ecpIllegal_bytecode, /* 197 */
//_ecpIllegal_bytecode, /* 198 */
//_ecpIllegal_bytecode, /* 199 */
//_ecpIllegal_bytecode, /* 200 */
//_ecpIllegal_bytecode, /* 201 */
//_ecpIllegal_bytecode, /* 202 */
//_ecpIllegal_bytecode, /* 203 */
//_ecpIllegal_bytecode, /* 204 */
//_ecpIllegal_bytecode, /* 205 */
//_ecpIllegal_bytecode, /* 206 */
//_ecpIllegal_bytecode, /* 207 */
//_ecpIllegal_bytecode, /* 208 */
//_ecpIllegal_bytecode, /* 209 */
//_ecpIllegal_bytecode, /* 210 */
//_ecpIllegal_bytecode, /* 211 */
//_ecpIllegal_bytecode, /* 212 */
//_ecpIllegal_bytecode, /* 213 */
//_ecpIllegal_bytecode, /* 214 */
//_ecpIllegal_bytecode, /* 215 */
//_ecpIllegal_bytecode, /* 216 */
//_ecpIllegal_bytecode, /* 217 */
//_ecpIllegal_bytecode, /* 218 */
//_ecpIllegal_bytecode, /* 219 */
//_ecpIllegal_bytecode, /* 220 */
//_ecpIllegal_bytecode, /* 221 */
//_ecpIllegal_bytecode, /* 222 */
//_ecpIllegal_bytecode, /* 223 */
//_ecpIllegal_bytecode, /* 224 */
//_ecpIllegal_bytecode, /* 225 */
//_ecpIllegal_bytecode, /* 226 */
//_ecpIllegal_bytecode, /* 227 */
//_ecpIllegal_bytecode, /* 228 */
//_ecpIllegal_bytecode, /* 229 */
//_ecpIllegal_bytecode, /* 230 */
//_ecpIllegal_bytecode, /* 231 */
//_ecpIllegal_bytecode, /* 232 */
//_ecpIllegal_bytecode, /* 233 */
//_ecpIllegal_bytecode, /* 234 */
//_ecpIllegal_bytecode, /* 235 */
//_ecpIllegal_bytecode, /* 236 */
//_ecpIllegal_bytecode, /* 237 */
//_ecpIllegal_bytecode, /* 238 */
//_ecpIllegal_bytecode, /* 239 */
//_ecpIllegal_bytecode, /* 240 */
//_ecpIllegal_bytecode, /* 241 */
//_ecpIllegal_bytecode, /* 242 */
//_ecpIllegal_bytecode, /* 243 */
//_ecpIllegal_bytecode, /* 244 */
//_ecpIllegal_bytecode, /* 245 */
//_ecpIllegal_bytecode, /* 246 */
//_ecpIllegal_bytecode, /* 247 */
//_ecpIllegal_bytecode, /* 248 */
//_ecpIllegal_bytecode, /* 249 */
//_ecpIllegal_bytecode, /* 250 */
//_ecpIllegal_bytecode, /* 251 */
//_ecpIllegal_bytecode, /* 252 */
//_ecpIllegal_bytecode, /* 253 */
//_ecpIllegal_bytecode, /* 254 */
//_ecpIllegal_bytecode, /* 255 */
};


/*
 * Byte Code dispatching -- the Inner Loop of the Interpreter.
 * Execute JavaCode bytecode until INITIAL_PC is reached or powered down.
 */
void run()
{
	u8 		D_A		byteCode;
	error_t	D_A		error;

	TYPE_JUP_BUF * back_jmp_buf;
	TYPE_JUP_BUF   new_error_env;

	back_jmp_buf = _error_env;
	_error_env = &new_error_env;

	if ((error = set_error_handler(*_error_env)) != NO_ERROR_)
//	if ((error = set_error_handler()) != NO_ERROR_)
	{

		apdu_ResumeAllInterrupts();
		/* An error occurred during execution of bytecode. This error either be
		 * fatal (uncatchable). Stack over and underflow are examples of fatal
		 * errors. Or it can be an exception that may be caught by the JavaCard
		 * code.
		 *
		 * Note this error handler also catches error thrown during error
		 * handling itself.
		 */

		if (error == TRANSACTION_BUFFER_OVERFLOW)
		{
			/* this is an OS error, transform it in a JavaCard exception code */
			abortTransactionByJCRE();
			error = TRANSACTIONEXCEPT_BUFFER_FULL;
		}

		if (IS_APDU_EXCEPTION(error))
		{/*@ -disspecopt @*/
			jref	X_A		ApduExceptionObjectRef;
			PF8		X_A		ApduExceptionObjectAddr;
			PF8		X_A		ApduExceptionArray;

			ApduExceptionObjectRef		= nvmReadU16(MEMtPN8(OBJECT_ApduException_OFFSET));
			ApduExceptionObjectAddr		= MEMtPF8(locateObjectInMemory(ApduExceptionObjectRef));
			ApduExceptionArray			= PF8_A_CONST(ApduExceptionObjectAddr, Object_Fields_START);
			memWriteU16Atomic(MEMtPM8(get_array_data(MEMtPF8(locateObjectInMemory(nvmReadU16(MEMtPN8(ApduExceptionArray)))))), U16_AND_U16(error, EXCEPTION_REASON_MASK));
			handleException(ApduExceptionObjectRef, ApduExceptionObjectAddr);
		}
		#if GP__AMENDMENT__B__createSCP81Connection_FOR_MASK // modify by xuxiaojun@20220318 分层
		else if(IS_CONNECTION_EXCEPTION(error))
		{
			jref	X_A		ConnectionExceptionObjectRef;
			PF8		X_A		ConnectionExceptionArray;
			PF8		X_A		ConnectionExceptionObjectAddr;

			ConnectionExceptionObjectRef = nvmReadU16(MEMtPN8(OBJECT_connectionException_OFFSET));
			ConnectionExceptionObjectAddr = MEMtPF8(locateObjectInMemory(ConnectionExceptionObjectRef));
			ConnectionExceptionArray = PF8_A_CONST(ConnectionExceptionObjectAddr, Object_Fields_START);

			memWriteU16Atomic(MEMtPM8(get_array_data(MEMtPF8(locateObjectInMemory(nvmReadU16(ConnectionExceptionArray))))), U16_AND_U16(error, EXCEPTION_REASON_MASK));
			handleException(ConnectionExceptionObjectRef, ConnectionExceptionObjectAddr);
		}
		#endif
		else
		/* both TransactionException and SystemException handling assume that the reason array
		 * is the first field in the exception object.
		 */
		if (IS_TRANSACTION_EXCEPTION(error))
		{
			/*
			 * TransactionException
			 */
			jref	X_A		TransactionExceptionObjectRef;
			PF8		X_A		TransactionExceptionArray;
			PF8		X_A		TransactionExceptionObjectAddr;

			TransactionExceptionObjectRef	= nvmReadU16(MEMtPN8(OBJECT_TransactionException_OFFSET));
			TransactionExceptionObjectAddr	= MEMtPF8(locateObjectInMemory(TransactionExceptionObjectRef));

			/* store reason code (from low byte of error)
			 * Don't write reason atomically! Now that the buffer is a transient array with the data in RAM,
			 * this is automatically the case...
			 */

			TransactionExceptionArray		= PF8_A_CONST(TransactionExceptionObjectAddr, Object_Fields_START);


			/* In the following statement, the nvmReadU16 obtains the transient array object ID,
			 * the locateObjectInMemory obtains the reference for the transient array,
			 * the get_array_data then returns the reference for the start of the data, which is
			 * where we store the reason code. This is true for the similar statement under SystemException.
			 */

			memWriteU16Atomic(MEMtPM8(get_array_data(MEMtPF8(locateObjectInMemory(nvmReadU16(TransactionExceptionArray))))), U16_AND_U16(error, EXCEPTION_REASON_MASK));
			handleException(TransactionExceptionObjectRef, TransactionExceptionObjectAddr);
		}
		else if (IS_UTIL_EXCEPTION(error)) {/*@ +disspecopt @*/
			/*
			 * UtilException
			 */
#if JCVM__VM__JAVACARDX_FRAMEWORK_UTIL
			PF8 UtilExceptionArray;
			jref UtilExceptionObjectRef = PN_R_U16((memref) OBJECT_UtilException_OFFSET);
			PF8 UtilExceptionObjectAddr = MEMtPF8(locateObjectInMemory(UtilExceptionObjectRef));
			UtilExceptionArray = UtilExceptionObjectAddr + Object_Fields_START;
			memWriteU16Atomic(MEMtPF8(get_array_data(MEMtPF8(locateObjectInMemory(nvmReadU16(UtilExceptionArray))))), (jshort) (error & EXCEPTION_REASON_MASK));
			handleException(UtilExceptionObjectRef, UtilExceptionObjectAddr);
#endif
		}
		else if (IS_SYSTEM_EXCEPTION(error))
		{/*@ -disspecopt @*/
			/*
			 * SystemException
			 */
			jref	X_A		SystemExceptionObjectRef;
			PF8		X_A		SystemExceptionArray;
			PF8		X_A		SystemExceptionObjectAddr;

			if(U16_AND_U16(error, EXCEPTION_REASON_MASK) == EXCEPTION_REASON_MASK) //退出嵌套run()
			{
				_error_env = back_jmp_buf;
				bootErrorHandle_SetLongJmp(_error_env);
				return;
			}

			if(U16_AND_U16(error, EXCEPTION_REASON_MASK) == 0xFE) //调回Java main函数
			{
				if(BackTop == NULL)
				{
					fatal_error(UNCAUGHT_EXCEPTION_ERROR);
				}
				else
				{
					Top = BackTop;
					FP  = BackFP;
					popFrame();
				}
			}
			else
			{
				SystemExceptionObjectRef		= nvmReadU16(MEMtPN8(OBJECT_SystemException_OFFSET));
				SystemExceptionObjectAddr		= MEMtPF8(locateObjectInMemory(SystemExceptionObjectRef));
				SystemExceptionArray 			= PF8_A_CONST(SystemExceptionObjectAddr, Object_Fields_START);

				memWriteU16Atomic(MEMtPM8(get_array_data(MEMtPF8(locateObjectInMemory(nvmReadU16(SystemExceptionArray))))), U16_AND_U16(error, EXCEPTION_REASON_MASK));
				handleException(SystemExceptionObjectRef, SystemExceptionObjectAddr);
			}
		} 

		else if (IS_CRYPTO_EXCEPTION(error))
		{
			/*
			 * CryptoException
			 */
			jref	X_A		CryptoExceptionObjectRef;
			PF8		X_A		CryptoExceptionArray;
			PF8		X_A		CryptoExceptionObjectAddr;
			
			CryptoExceptionObjectRef 		= nvmReadU16(MEMtPN8(OBJECT_CryptoException_OFFSET));
			CryptoExceptionObjectAddr 		= MEMtPF8(locateObjectInMemory(CryptoExceptionObjectRef));
			CryptoExceptionArray 			= PF8_A_CONST(CryptoExceptionObjectAddr, Object_Fields_START);
			
			memWriteU16Atomic(MEMtPM8(get_array_data(MEMtPF8(locateObjectInMemory(nvmReadU16(CryptoExceptionArray))))), U16_AND_U16(error, EXCEPTION_REASON_MASK));
			handleException(CryptoExceptionObjectRef, CryptoExceptionObjectAddr);
		}
		#if JAVA_CARD_UICC // modify by xuxiaojun@20220318 分层
		else if(IS_TOOLKITAPI_EXCEPTION(error))
		{
			jref	X_A		ExceptionObjectRef;
			PF8		X_A		ExceptionArray;
			PF8		X_A		ExceptionObjectAddr;
			u16     X_A     ErrorType;
			u32     X_A     Offset;

			ErrorType = (error&0xFF00);

			if(ErrorType == TOOLKIT_EXCEPTION_MASK)
			{
				Offset = OBJECT_uiccToolkitExptionIns_OFFSET;
			}
			else if(ErrorType == UICC_EXCEPTION_MASK)
			{
				Offset = OBJECT_uiccExptionIns_OFFSET;
			}
			else if(ErrorType == ADMIN_EXCEPTION_MASK)
			{
				Offset = OBJECT_uiccAdminExptionIns_OFFSET;
			}
			else if(ErrorType == SIM_TOOLKIT_EXCEPTION_MASK)
			{
				Offset = OBJECT_simToolkitExptionIns_OFFSET;
			}
			else  //SIM_VIEW_EXCEPTION_MASK
			{
				Offset = OBJECT_simViewExcepyionIns_OFFSET;
			}

			ExceptionObjectRef = nvmReadU16(MEMtPN8(Offset));
			ExceptionObjectAddr = MEMtPF8(locateObjectInMemory(ExceptionObjectRef));
			ExceptionArray = PF8_A_CONST(ExceptionObjectAddr, Object_Fields_START);

			memWriteU16Atomic(MEMtPM8(get_array_data(MEMtPF8(locateObjectInMemory(nvmReadU16(ExceptionArray))))), U16_AND_U16(error, EXCEPTION_REASON_MASK));
			handleException(ExceptionObjectRef, ExceptionObjectAddr);
		}
		#endif
		else 
		{
			PF8		X_A		objAddr;
			jref	X_A		objRef;
			memref  X_A		ISOExceptionArray;

			objRef = 0;
			switch (error)
			{
			/*Saqib: Start Changes*/
			case ARITHMETIC_EXCEPTION:
				objRef = nvmReadU16(MEMtPN8(OBJECT_ArithmeticException_OFFSET));
				break;

			case ARRAY_STORE_EXCEPTION:
				objRef = nvmReadU16(MEMtPN8(OBJECT_ArrayStoreException_OFFSET));
				break;

			case CLASS_CAST_EXCEPTION:
				objRef = nvmReadU16(MEMtPN8(OBJECT_ClassCastException_OFFSET));
				break;

			case NEGATIVE_ARRAY_SIZE_EXCEPTION:
				objRef = nvmReadU16(MEMtPN8(OBJECT_NegativeArraySizeException_OFFSET));
				break;

			case ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION:
				objRef = nvmReadU16(MEMtPN8(OBJECT_ArrayIndexOutOfBoundsException_OFFSET));
				break;

			case NULL_POINTER_EXCEPTION:
				objRef = nvmReadU16(MEMtPN8(OBJECT_NullPointerException_OFFSET));
				break;

			case SECURITY_EXCEPTION:
				objRef = nvmReadU16(MEMtPN8(OBJECT_SecurityException_OFFSET));
				break;
			default:
				objRef = PN_R_U16((memref)OBJECT_IsoException_OFFSET);
				objAddr = MEMtPF8(locateObjectInMemory(objRef));
				ISOExceptionArray = PFtMEM(PF8_A_U8(objAddr, Object_Fields_START));
				/*@ -discast @*/
				memWriteU16Atomic(MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(nvmReadU16(MEMtPN8(ISOExceptionArray)))))), (jshort)error);
				/*@ +discast @*/
				break;
			}
			objAddr		= MEMtPF8(locateObjectInMemory(objRef));
			handleException(objRef, objAddr);
		}/*@ +disspecopt @*/


		/*
		 * at the end of error processing, the VM is placed at the beginning of
		 * the corresponding exception handler. C execution falls through to the
		 * normal VM interpreter loop...
		 */
	}
//	bootErrorHandle_SetLongJmp(&_error_env);
	bootErrorHandle_SetLongJmp(_error_env);
	/*
	 * Main interpreter loop
	 */
	while (1)
	{
		/*
		 * execute next bytecode decode and execute bytecode function
		 */
		byteCode = PF_R_U8(JPC);
		JCDebug_Interpreter1(PFtMEM(JPC));
		PF8_INC(JPC);
		bytecode_ptr[byteCode]();

		JCDebug_Interpreter2(PFtMEM(JPC), byteCode);
	}

	/*
	 * Normal execution completes when PC returns to INITIAL_PC,
	 * i.e., at the end of the bootstrap or main method.
	 */
}

void initInterpreter()
{
	PATCH_JCVM_VM_INTERPRETER_002();	
	cc					= JCRE_CONTEXT;
	JPC					= MEMtPF8(INITIAL_PC);


	/* This is to ensure PCpkgID != INITIAL_PKG */
	/* Due to: performance optimization in packageloc.c */
	PCpkgID				= 0xFF;
	//TODO:fix address
	cachedClassCompLoc	= getPackageComponentLocation(INITIAL_PKG, COMPONENT_CLASS);
	cachedMethodCompLoc	= getPackageComponentLocation(INITIAL_PKG, COMPONENT_METHOD);
	cachedFieldCompLoc	= getPackageComponentLocation(INITIAL_PKG, COMPONENT_STATICFIELD);
	PCpkgID				= INITIAL_PKG;
	lastVMPkg			= INITIAL_PKG;
	localsize			= 0;
	paramsize			= 0;
	initStackParams();
}

