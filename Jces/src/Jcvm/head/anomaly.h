/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*+
 * File:  anomaly.h
 * Version 1.5
 * Path: /net/mumble/javacard_users/home/vo113324/ws/javacard_refimpl_victor/cjdk/VM/cjcre/src/platforms/common/SCCS/s.anomaly.h
 * Last modified on 01/09/17 at 16:55:26
 * Last modifed by: Alexandre Frey, Nicolas Regnault
 * Original author: David Getreu
 -*/

#ifndef   __ANOMALY_H__
#define   __ANOMALY_H__

#include "profile.h"
#include "error.h"

/* java exception */
#define ARITHMETIC_EXCEPTION                0x8001
#define ARRAY_STORE_EXCEPTION               0x8002
#define CLASS_CAST_EXCEPTION                0x8003
#define NEGATIVE_ARRAY_SIZE_EXCEPTION       0x8004
#define ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION 0x8005
#define NULL_POINTER_EXCEPTION              0x8006
#define SECURITY_EXCEPTION                  0x8007

/* APDU_EXCEPTION */
#define APDU_EXCEPTION_ILLEGAL_USE			0xB001
#define APDU_EXCEPTION_BUFFER_BOUNDS		0xB002
#define APDU_EXCEPTION_BAD_LENGTH			0xB003
#define APDU_EXCEPTION_IO_ERROR				0xB004
#define APDU_EXCEPTION_NO_T0_GETRESPONSE	0xB0AA
#define APDU_EXCEPTION_T1_IFD_ABORT			0xB0AB
#define APDU_EXCEPTION_NO_T0_REISSUE		0xB0AC

#define TOOLKIT_EXCEPTION_MASK              0x1000  /* ---1 ---- ---- ---- */
#define UICC_EXCEPTION_MASK                 0x2000  /* --1- ---- ---- ---- */
#define ADMIN_EXCEPTION_MASK                0x3000  /* --11 ---- ---- ---- */
#define SIM_TOOLKIT_EXCEPTION_MASK          0x4000  /* -1-- ---- ---- ---- */
#define SIM_VIEW_EXCEPTION_MASK             0x5000  /* -1-1 ---- ---- ---- */

//#define NATIVE_TOOLKIT_EXCEPTION			0x800f
//#define NATIVE_SIMVIEW_EXCEPTION            0x8010
//#define NATIVE_UICCVIEW_EXCEPTION           0x8011
//#define NATIVE_ADMIN_EXCEPTION              0x8012
//#define NATIVE_ISO_EXCEPTION                0x8013
/* TransactionException. Lower byte contains the reason code -- see JCRE Specification JCRE Constants */
#define TRANSACTIONEXCEPT_IN_PROGRESS       0xA001
#define TRANSACTIONEXCEPT_NOT_IN_PROGRESS   0xA002
#define TRANSACTIONEXCEPT_BUFFER_FULL       0xA003
#define TRANSACTIONEXCEPT_INTERNAL_FAILURE  0xA004

/* SystemException with reason codes -- see JCRE Specification JCRE Constants */
#define SYSTEMEXCEPT_ILLEGAL_VALUE          0x9001
#define SYSTEMEXCEPT_NO_TRANSIENT_SPACE     0x9002
#define SYSTEMEXCEPT_ILLEGAL_TRANSIENT      0x9003
#define SYSTEMEXCEPT_ILLEGAL_AID            0x9004
#define SYSTEMEXCEPT_NO_RESOURCE            0x9005
#define ASSERTION_FAILED                    0x9006
#define SYSTEMEXCEPT_WRITE_MEM_FAIL         0x9007
#define SYSTEMEXCEPT_RMI_LC_MISMATCH        0x9078
#define SYSTEMEXCEPT_RMI_RESOURCE           0x9088

/* UtilException with reason codes -- see JCRE Specification JCRE Constants */
#define UTILEXCEPT_ILLEGAL_VALUE            0x8801
#define UTILEXCEPT_TYPE_MISMATCHED          0x8802

/* ConnectionException with reason codes -- see uicc.connection.ConnectionException */
#define CONNECTIONEXCEPT_TRANSPORT_PROTOCOL_NOT_SUPPORTED	 0xD000		//the transport protocol type is not supported
#define CONNECTIONEXCEPT_TOOLKIT_RESOURCE_NOT_AVAILABLE	 	 0xD001		//a proactive session is pending
#define CONNECTIONEXCEPT_INVALID_STATE	 					 0xD002		//the state of a connection or of a link does not correspond the requested action
#define CONNECTIONEXCEPT_TIME_OUT							 0xD003		//the requested action was aborted because of a timeout
#define CONNECTIONEXCEPT_USER_CANCELLATION	 				 0xD004		//the requested action was aborted by the user
#define CONNECTIONEXCEPT_OUT_OF_DATA_BOUNDARIES_scp81		 0xD005		//the requested action would cause access out of data
#define CONNECTIONEXCEPT_OPERATION_NOT_ALLOWED	 		 	 0xD006		//the requested operation is not allowed at this time
#define CONNECTIONEXCEPT_CONNECTION_REFUSED					 0xD007		//the connection has been refused by the remote entity
#define CONNECTIONEXCEPT_INVALID_PARAMETERS					 0xD008		//the parameters are invalid
#define CONNECTIONEXCEPT_UNSUPPORTED_OPERATION				 0xD009		//the terminal does not support the required mechanisms to perform the action
#define CONNECTIONEXCEPT_RESOURCE_NOT_AVAILABLE				 0xD00A		//there are insufficient resources on the UICC or on the terminal
#define CONNECTIONEXCEPT_SCP81_KEY_NOT_FOUND				 0xD00B		//no SCP81 key could be found to establish the TLS session
#define CONNECTIONEXCEPT_INVALID_HEADER_FIELD				 0xD00C		//invalid header fields set by application

#define	CRYPTOEXCEPT_ILLEGAL_VALUE			0x8401
#define	CRYPTOEXCEPT_UNINITIALIZED_KEY		0x8402
#define	CRYPTOEXCEPT_NO_SUCH_ALGORITHM		0x8403
#define	CRYPTOEXCEPT_INVALID_INIT			0x8404
#define	CRYPTOEXCEPT_ILLEGAL_USE			0x8405


/* Error and Exception Mask */
#define JAVA_EXCEPTION_MASK                 0x8000  /* 1--- ---- ---- ---- */
#define IO_EXCEPTION_ERROR_MASK             0xC000  /* 11-- ---- ---- ---- */
#define TRANSACTION_EXCEPTION_MASK          0xA000  /* 1-1- ---- ---- ---- */
#define SYSTEM_EXCEPTION_MASK               0x9000  /* 1--1 ---- ---- ---- */
#define UTIL_EXCEPTION_MASK                 0x8800  /* 1--- 1--- ---- ---- */
#define CRYPTO_EXCEPTION_MASK               0x8400  /* 1--- -1-- ---- ---- */
#define EXCEPTION_REASON_MASK               0x00ff
#define APDU_EXCEPTION_MASK					0xB000  /* 1-11 ---- ---- ---- */
#define CONNECTION_EXCEPTION_MASK           0xD000  /* 11-1 ---- ---- ---- */

#define Priv_EXCEPTION                      0x90FE //STK UICCMAIN调回主流程标记

#define IS_IO_ERROR(error) ((error & IO_EXCEPTION_ERROR_MASK) == IO_EXCEPTION_ERROR_MASK)
#define IS_TRANSACTION_EXCEPTION(error) ((error & TRANSACTION_EXCEPTION_MASK) == TRANSACTION_EXCEPTION_MASK)
#define IS_SYSTEM_EXCEPTION(error) ((error & SYSTEM_EXCEPTION_MASK) == SYSTEM_EXCEPTION_MASK)
#define IS_CRYPTO_EXCEPTION(error) ((error & CRYPTO_EXCEPTION_MASK) == CRYPTO_EXCEPTION_MASK)
#define IS_UTIL_EXCEPTION(error) ((error & UTIL_EXCEPTION_MASK) == UTIL_EXCEPTION_MASK)
#define IS_APDU_EXCEPTION(error) ((error & APDU_EXCEPTION_MASK) == APDU_EXCEPTION_MASK)
#define IS_TOOLKITAPI_EXCEPTION(error) ((error&0xFF00) == (error&0x7000))
#define IS_CONNECTION_EXCEPTION(error) ((error & CONNECTION_EXCEPTION_MASK) == CONNECTION_EXCEPTION_MASK)

/* fatal errors */
#define STACK_UNDER_FLOW_ERROR              0x0801
#define STACK_OVER_FLOW_ERROR               0x0802
#define NO_MORE_CODE_EXCEPTION              0x0803
#define BAD_ARRAY_TYPE_ERROR                0x0804
#define FIELD_INDEX_OUT_OF_OBJECT_ERROR     0x0805
#define BAD_DUPX_MN_VALUE_ERROR             0x0806
#define BAD_SWAPX_MN_VALUE_ERROR            0x0807
#define RESOLVE_INTERFACE_METHOD_ERROR      0x0808
#define INT_NOT_SUPPORTED_ERROR             0x0809
#define METHOD_TOKEN_ERROR                  0x080A
#define UNCAUGHT_EXCEPTION_ERROR            0x080B
#define INVALID_DTR_SPACEID_ERROR           0x080C
#define ALLOCATION_DURING_TRANSACTION		0x080D
#define UNKNOWN_NATIVE						0x080E
#define	NO_MORE_SPACE_FOR_TR_OBJ_NODE		0x080F
#define ILLEGAL_OBJECT_EXCEPTION            0x8010
#define RESOLVE_VIRTUAL_METHOD_ERROR        0x0810
#endif /* __ANOMALY_H__ */
