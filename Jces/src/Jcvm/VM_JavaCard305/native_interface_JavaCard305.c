#include "native_interface_JavaCard305.h"
#include "nativemethod_impl_JavaCard305.h"
#include "SensiTiveResult.h"
#include "mask.h"
#include "stack.h"

u8 JavaCard305API_CallNative(u16 fun_index)    SMALL_MODE
{

	u8		D_A    isCallJavaCard305Native;

	isCallJavaCard305Native = true;

	switch (fun_index)
	{
#if WITH_JC305
		case GetSignatureAlgorithm_NM:
			GetSignatureAlgorithm();
			break;
		case GetCipherAlgorithm_NM:
			GetCipherAlgorithm();
		  break;
		case arrayFill_NM:
			arrayFill();
		  break;
		case sensitiveResultSetTagValUnassigned_NM:
			sensitiveResultSetTagValUnassigned();
			break;
		case sensitiveResultSetBooleanFalse_NM:
			sensitiveResultSetBooleanFalse();
		  break;
		case sensitiveResultSetBooleanTrue_NM:
			sensitiveResultSetBooleanTrue();
		  break;
		case sensitiveResultSetObject_NM:
			sensitiveResultSetObject();
		  break;
		case sensitiveResultSet_NM:
			sensitiveResultSet(popShort());
		  break;
	  case sensitiveResult_NM:
			sensitiveResult();
		  break;

		case setSensitiveArryObj_NM:
			setSensitiveArryObj();
			break;
		case isSensitiveArryObj_NM:
			isSensitiveArryObj();
		  break;
		case clearArray_NM:
			clearArray();
		  break;
#if VM_WITH_DH
		case storeDHKeyRef_NM:
			storeDHKeyRef();
		  break;
		case setDHDefaultPAndG_NM:
			setDHDefaultPAndG();
		  break;
		case DH_NM:
			DH();
		  break;
	  case genDHPriKeyX_NM:
			genDHPriKeyX();
		  break;
#endif // VM_WITH_DH
#endif // WITH_JC305
		default:
			isCallJavaCard305Native = false;
	}

	return isCallJavaCard305Native;
}


