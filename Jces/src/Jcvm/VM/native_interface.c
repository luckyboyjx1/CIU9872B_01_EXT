#include "native_interface.h"
#include "nativemethod_impl.h"
#include "SecurityDomainData.h" //warning isdScpData_Init
#include "SCPInterface.h" // warning Remote_EncryptAndGenRMAC
#include "native_interface_JavaCard305.h"
#include "opFunction.h"
#include "object.h"
#include "ob_manager.h"
#include "except.h"
#include "mask.h"
#include "profile.h"
#include "error.h"
#include "context.h"
#include "except.h"
/*#include "assert.h"  */
#include "loader.h"
#include "reset.h"
#include "lifecycle.h"
#include "cardmanager.h"
#include "contentManager.h"
#include "global_variable.h"
#include "OPinstaller.h"
#include "patch.h"
#include "GC.h"
#include "cardManager.h"
#include "CVM.h"
#include "mathNativeImpl.h"
#include "util_nm.h"
#include "memory.h"
#include "JcvmConfig.h"
#include "Crypto_API.h"
#include "crypto_impl.h"
#include "JcvmConfig.h"
#include "PbocAPI.h"
#include "KeyManagerOld.h"
#include "dataManager.h"
#include "tranDeallocate.h"
#include "OPCMCal.h"
#include "CgmManager.h"
#include "jcvmConfigMethod.h"
//#include "VMDebug.h"
#include "LteAPI.h"
#include "Apdu.h"
#include "euiccConfig.h"
#include "SEEUICCInterface.h"
#include "NativeAppComponent.h"
#include "EllipseCurve_API.h"
#include "Crypto_common_API.h"
#include "CLProtocolMngt.h" // 去掉警告
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if UICC_FUCTION_PRIVATEAUTH
#include "gwAPI.h"
#endif

#if GP__HCI_API__FOR_MASK
#include "Swp_Hci_Api.h"
#endif

void DecreaseNativesFunction(void)
{
	u16 context = popShort();
	switch(U16t8(popShort()))
	{
	case 1:
		//getMaxPackageIdentifier();
		pushShort((NPACKAGES - 1));
		break;
	case 2:		
		pushShort(U8t16(initializeGC()));
		break;
	case 3:/*@ -discast @*/
		/*@ -typedefparam @*/
		/*@ -type @*/
		getCurrentContext();
		/*@ +type @*/
		/*@ +typedefparam @*/
		break;	/*@ +discast @*/

	case 4:
		cc = context;
		pushShort(cc);
		break;

	} 
}	
//modify by xuxiaojun@20221020: 根据 合代码时代码评审(20220722).xlsx进行修改： 将JAVA CARD305的本地方法放在nativemethod_impl_JavaCard305中
void callnative(u16 func_index)
{
	switch (func_index)
	{
	case setCardInitialized_NM:
		setCardCreated();
		break;


		
	case getAvailableMemory_NM:
		pushShort(getAvailableMemory());
		break;
	
	case getAvailableMemoryBuf_NM:
		getAvailableMemoryBuf();
		break;

	case callInstall_NM:
		callInstall();
		break;
	
	case initAID_NM: //added by jiangxiaohui:优化性能, EPX产业化/THC_THD89/Branch/EPX_THC_Mobile5G/Develop
		initAIDImpl();
		break;
		
	case isSecurityDomain_NM: // added by xuxiaojun@20220621 用于判断安装的实例是否来源于可执行模块SecurityDomain
		isSecurityDomain();
		break;

	case resolveE2C4command_NM:
		resolveE2C4commandNM();
		break;
		
	case setChannelContext_NM:
		setChannelContextNM();
		break;
		
	case getChannelContext_NM:
		getChannelContextNM();
		break;
		
// 	case getCurrentContext_NM:
// 		getCurrentContext();
// 		break;
		
	case getPreviousContext_NM:
		getPreviousContext();
		break;
		
	case checkPreviousContextAccess_NM:
#if JCVM__VM__FIRE_WALL
		checkPreviousContextAccessNM();
#endif
		break;

	case setJCREentry_NM:
		setJCREentry();
		break;
		
		case setJCREGlobal_NM :
		setJCREGlobal();
		break;

	case InitBuffer_NM:
		InitBuffer();
		break;
	/*case t0InitAPDUBuffer_NM:
		initAPDUBuffer();
		break;*/
	/*case receiptGeneration_NM:
		receiptGeneration();
		break;*/
	case isTransient_NM:
		isTransient();
		break;
/*
	case clearTransientObjs_NM:
		clearTransientObjs();
		break;
*/
	case makeTransientBooleanArray_NM:
		/*@ -discast @*/
		/*@ -typedefparam @*/
		/*@ -type @*/
		makeTransientArray(SIZE_BOOLEAN, (jbyte) Header_ArrayType_Boolean, Boolean_PA_CLASS_OFFSET);
		/*@ +discast @*/
		/*@ +typedefparam @*/
		/*@ +type @*/
		break;
		
	case makeTransientByteArray_NM:
		makeTransientArray(SIZE_BYTE, U32t8( Header_ArrayType_Byte), Byte_PA_CLASS_OFFSET);
		break;

	case makeTransientShortArray_NM:
		makeTransientArray(SIZE_SHORT, U32t8( Header_ArrayType_Short), Short_PA_CLASS_OFFSET);
		break;

	case makeTransientObjectArray_NM:
		makeTransientArray(SIZE_REF, U32t8( Header_ArrayType_Reference), Object_PA_CLASS_OFFSET);
		break;

	case makeGlobalArray_NM:
		makeGlobalArray();
		break;

	case beginTransaction_NM:
		beginTransaction(SINGLE_APDU_PROCESS);
		break;

	case abortTransaction_NM:
		abortTransaction();
		break;

	case commitTransaction_NM:
		commitTransaction();
		break;
		/*@ -discast @*/
		/*@ -typedefparam @*/
		/*@ -type @*/
	case getTransactionDepth_NM:
		getTransactionDepth();
		break;

	case getUnusedCommitCapacity_NM:
		getUnusedCommitCapacity();
		break;
		/*@ +discast @*/
		/*@ +typedefparam @*/
		/*@ +type @*/
	case getMaxCommitCapacity_NM:
		getMaxCommitCapacity();
		break;

	case checkArrayArgs_NM:
		checkArrayArgs();
		break;

	case nvmFreeObject_NM:
		//FreeObject();
		tranDeallocate_compactRam(COMPACT_SPACE);
		NvmManager_FreeObject(false);
		ExecuEnvironmentInit(G_CurExecutingProfileIndex); // modify by xuxiaojun@20220513 根据分层代码 第一次代码评审(20220415).xlsx 将G_CurExecutingProfileIndex宏定义为0xFF
		//NvmManager_Deallocate(false);
		JCDebug_Deallocate();
		setGCRequestedFlag(false);
		break;

	case arrayCopy_NM:
		arrayCopy();
		break;

	case arrayCopyNonAtomic_NM:
		arrayCopyNonAtomic();
		break;

	case arrayFillNonAtomic_NM:
		arrayFillNonAtomic();
		break;

	case arrayCompare_NM:
		arrayCompare();
		break;

	case setShort_NM:
		setShort();
		break;
	case getShort_NM:
		getShort_java();
		break;

// 	case initGC_NM:
//  		initGC();
// 		break;

	case initAppletDeletion_NM:
		initAD();
		break;

	case markNonRootObjects_NM:
		pushShort(U8t16(markNonRootObjects(U16t8(popShort()))));
		break;

	case GCsweep_NM:
		sweep();
		break;

	case markAppRoots_NM:
		markAppRoots();
		break;

	case doGC_NM:
		startGC();
		break;

	case markStaticRoots_NM:
		markStatics();
		break;

    case checkSIOaccess_NM:
		checkSIOaccess();
		break;
		
	case initPackageDeletion_NM:
		initPackageDeletionNM();
		break;

	case removePackage_NM:
		removePackageNM();
		break;

	case clearGlobalPkgAddr_NM:
		clearGlobalPkgAddrNM();
		break;

	case releaseStaticFeild_NM:
		releaseStaticRef();
		break;
		
//	case GetSecurityLevel_NM:
//		pushShort( ManageSecurityLevel(popShort()) );
//		break;
#if !GP__VERSION__FOR_MINI_MASK
	case getChannelStatus_NM:
		getChannelStatusNM();
		break;
#endif		
	case getMaxChannels_NM:
		getMaxChannelsNM();
		break;
		
	case getContextStatus_NM:
		getContextStatusNM();
		break;
		
	case manageAppFlag_NM:
		manageAppFlagNM();
		break;
		
	case getCurrentlySelectedChannel_NM:
		getCurrentlySelectedChannelNM();
		break;
	
#if !GP__VERSION__FOR_MINI_MASK		
	case setCurrentlySelectedChannel_NM:
		setCurrentlySelectedChannelNM();
		break;

	case channelManagerCommand_NM:
		channelManagerCommand();
		break;
#endif
/*	case getPkgIDForAddress_NM:
		getPkgIdForAddress();
		break;*/
	
	case getEntryByInstAID_NM:
		getEntryByInstAIDNM();
		break;
	case SearchEntryByInstAID_NM:
		SearchEntryByInstAIDNM();
		break;
	
	case getNewID_NM:
		getNewIDNM();
		break;

	case getIndex_NM:
		getIndexByIDNM();
		break;
	case addEntry_NM:
		addEntryNM();
		break;
	case getPkgContext_NM:
		getPkgContextNM();
		break;
	case getPkgIdByContext_NM:
		getPkgIdByContextNM();
		break;

#if VM_WITH_RANDOMDATA
	case genRandomData_NM:
		genRandomData();
		break;
#endif

#if VM_WITH_CRC
	case CRCInit_NM:
		CRCInit();
		break;
	case CRCUpdate_NM:
		CRCUpdate();
		break;
	case CRCDofinal_NM:
		CRCDofinal();	 
		break;
#endif

#if VM_WITH_CHECKSUM
	case setKeyChecksum_NM:
		setKeyChecksum();
		break;
#endif // 

#if VM_WITH_SYM//(VM_WITH_DES || VM_WITH_AES || VM_WITH_KOREANSEED || VM_WITH_SM1 || VM_WITH_SM4 || VM_WITH_SSF33)
#if WITH_AEAD
	case SymmetricCipherInit_NM:
		AEADCipherInit(U16t8(popShort()));
		break;
	case SymmetricCipherTag_NM:
		AEADCipherTag(U16t8(popShort()));
		break;
#endif
	case SymmetricCipherUpdate_NM:
		CipherUpdate(U16t8(popShort()));
		break;
	case SymmetricCipherDofinal_NM:
		CipherDofinal(U16t8(popShort()));
		break;
	case SymmetricSignatureUpdate_NM:
		SignatureUpdate(U16t8(popShort()));
		break;
	case SymmetricSignatureDofinal_NM:
		SignatureDofinal(U16t8(popShort()));
		break;
	case SymmetricSignatureVerify_NM:
		SignatureDofinal(U16t8(popShort()));
		break;
#if UICC_FUCTION_PRIVATEAUTH
	case GwSymmetricCipherUpdate_NM:
		SetSymGuomiProperty(SYM_GUOMI_PROPERTY_GW);
		CipherUpdate(U16t8(popShort()));
		SetSymGuomiProperty(SYM_GUOMI_PROPERTY_STANDAND);
		break;
	case GwSymmetricCipherDofinal_NM:
		SetSymGuomiProperty(SYM_GUOMI_PROPERTY_GW);
		CipherDofinal(U16t8(popShort()));
		SetSymGuomiProperty(SYM_GUOMI_PROPERTY_STANDAND);
		break;
	case GwSymmetricSignatureUpdate_NM:
		SetSymGuomiProperty(SYM_GUOMI_PROPERTY_GW);
		SignatureUpdate(U16t8(popShort()));
		SetSymGuomiProperty(SYM_GUOMI_PROPERTY_STANDAND);
		break;
	case GwSymmetricSignatureDofinal_NM:
		SetSymGuomiProperty(SYM_GUOMI_PROPERTY_GW);
		SignatureDofinal(U16t8(popShort()));
		SetSymGuomiProperty(SYM_GUOMI_PROPERTY_STANDAND);
		break;
	case GwSymmetricSignatureVerify_NM:
		SetSymGuomiProperty(SYM_GUOMI_PROPERTY_GW);
		SignatureDofinal(U16t8(popShort()));
		SetSymGuomiProperty(SYM_GUOMI_PROPERTY_STANDAND);
		break;
#endif
	case secureStoreSensitiveData_NM:
		secureStoreSensitiveData();
		break;
	case secureGetSensitiveData_NM:
		secureGetSensitiveData();
		break;
	case secureStorePINData_NM:
		secureStorePINData();
		break;
#endif	

#if VM_WITH_MD5 || VM_WITH_SHA1 || VM_WITH_RIPEMD160 || VM_WITH_SHA224 || VM_WITH_SHA256 || VM_WITH_SHA384 || VM_WITH_SHA512
	case MessageDigest_NM:
		MessageDigest(false);
		break;
	case MDUpdate_NM:
		MDUpdate(false);
		break;
#if VM_WITH_MD_INIT
	case initDigest_NM:
	    MDInit();
		break;
#endif
#endif

#if VM_WITH_DES || VM_WITH_RSA
	case initCodeFlowFlag_NM:
		initCodeFlowControlFlag();
		break;
	case checkCodeFlowFlag_NM:
		checkCodeFlowControlFlag(U16t8(popShort()));
		break;
#endif

#if VM_WITH_RSA
	case RSACipherUpdate_NM:
		RSACipherUpdate();//AsymmetricCipherUpdate
		break;
	case RSACipherDofinal_NM:
		RSACipherDofinal();//AsymmetricCipherDofinal
		break;
	case RSASignatureUpdateFinal_NM:
		RSASignatureUpdateFinal(RSA_SIGN);//AsymmetricSignatureUpdateFinal
		break;
	case RSASignatureDofinal_NM:
		RSASignatureDofinal(RSA_SIGN);//AsymmetricSignatureDofinal
		break;
	case RSAVerifyUpdateFinal_NM:
		RSASignatureUpdateFinal(RSA_VERIFY);
		break;
	case RSAVerifyDofinal_NM:
		RSASignatureDofinal(RSA_VERIFY);
		break;		
	case storeRSACrtKeyRef_NM:		  	
		storeRSACrtKeyRef();
		break;
	case storeRSAMEKeyRef_NM:
		storeRSAMEKeyRef();
		break;
	case storeRSAKeyPair_NM:
		storeRSAKeyPair();
		break;
//	case calcModuleLength_NM:
//		calcModuleLength();
//		break;
	case genRSAKeyPair_NM:
		RSAKeyGenCore(RSA_JAVA_API_GEN_KEY);
		break;
	case secureSetPKIKey_NM:
		secureSetPKIKey();
		break;
	case secureGetPKIKey_NM:
		secureGetPKIKey();
		break;
#endif  // VM_WITH_RSA

#if VM_WITH_SIGNMR
	case signMRDofinal_NM:
		signMRDofinal();
		break;
	case signMRUpdateFinal_NM:
		signMRUpdateFinal();
		break;
    case msgRecover_NM:
	   	msgRecover();
	   	break;
    case signMRVerify_NM:
	   	signMRVerify();
	   	break;
//	case signMRUpdateVerify_NM:
//		signMRUpdateVerify();
//		break;
#endif	 //	
#if VM_WITH_ECC
	case storeECDomainParams_NM:
		storeECDomainParams();
		break;
	case ECSignatureUpdateFinal_NM:
	    ECSignatureUpdateFinal();
	    break;
	case ECSignatureDofinal_NM:
		ECSignatureDofinal(ECC_SIGN);
		break;
	case ECVerifyUpdateFinal_NM:
	    ECVerifyUpdateFinal();
	    break;
	case ECVerifyDofinal_NM:
		ECSignatureDofinal(ECC_VERIFY);
		break;
    case storeECPubOrPrivKeyRef_NM:
	   	storeECPubOrPrivKeyRef();
	   	break;
    case ECDH_NM:
	   	ECDH();
    	break;
#if VM_WITH_ECCMATH
    case ECCMathX__NM:
    	ECCMathX();
    	break;
#endif
	case genECKeyPair_NM:
	   	ECKeyGen();
	   	break;
#endif

#if (LTE__CRYPTO__GENERALHMAC_MD5__FOR_JAVA || LTE__CRYPTO__GENERALHMAC_SHA__FOR_JAVA)
	case GeneralHMAC_NM:
		GeneralHMACNM();
		break;
#endif

#if VM_WITH_HMAC

    case HMACInit_NM:
	    HMAC_Init();
		break;
	case HMACUpdate_NM:
	    HMAC_Update();
		break;
	case HMACDofinal_NM:
	    HMAC_Dofinal();
		break;
	case HMACVerify_NM:
		HMAC_Verify();
		break;
#endif  //

#if VM_WITH_KOREANSEED

	case KoreanSEEDCipherUpdate_NM:
		CipherUpdate(KOREANSEED_ALG);
		break;
	case KoreanSEEDCipherDofinal_NM:
		CipherDofinal(KOREANSEED_ALG);
		break;
	case KoreanSEEDSignatureUpdate_NM:
		SignatureUpdate(KOREANSEED_ALG);
		break;
	case KoreanSEEDSignatureDofinal_NM:
	    SignatureDofinal(KOREANSEED_ALG | SBC_SIGN);
		break;
	case KoreanSEEDSignatureVerify_NM:
		SignatureDofinal(KOREANSEED_ALG | SBC_VERIFY);
		break;

#endif


#if VM_WITH_SM2
	 case computeZa_NM:
	      computeZa();
	 break;
     case genSM2KeyPair_NM:
	      genSM2KeyPair();
	 break;
	 case storeSM2KeyRef_NM:
	      storeSM2KeyRef();
	 break;
     case SM2CipherDofinal_NM:
		 SM2CipherDofinal();	  
	 break;
	 case SM2SignatureUpdateFinal_NM:
	      SM2SignatureUpdateFinal(SM2_SIGN);
	 break;
	 case SM2SignatureDofinal_NM:
		  SM2SignatureDofinal(SM2_SIGN);
	 break;
     case SM2VerifyUpdateFinal_NM:
		  SM2SignatureUpdateFinal(SM2_VERIFY);
	 break;
	 case SM2VerifyDofinal_NM:
		  SM2SignatureDofinal(SM2_VERIFY);
	 break;
	 case SM2NoSM3Signature_NM:
	 	  SM2Sig_VerifyWithNoSM3(SM2_SIGN);
	 break;
	 case SM2NoSM3Verify_NM:
	 	  SM2Sig_VerifyWithNoSM3(SM2_VERIFY);
	 break;					   
	 case GMStandardSM2KeyExchange_NM:
		 SM2KeyExchange();
		 break;
	 case GMStandardSM2CalPublicByPrivate_NM:
		 SM2CalPublicByPrivate();
		 break;
	 case GMStandardSM2ComputeZ_NM:
		 SM2ComputeZ();
		 break;
#endif


#if JCVM__IO__MIFARE
	case readWriteMifare_NM:
		MF_readWrite_NM();
		break;
#endif


	case getCurrentAPDU_NM:
		pushReference(nativeMethod_GetCurrentAPDU());
		break;

	case getCurrentAPDUBuffer_NM:
		pushReference(nativeMethod_GetCurrentAPDUBuffer());
		break;

	case getCLAChannel_NM:
		pushShort(U8t16(apdu_getCLAChannel()));
		break;

	case getInBlockSize_NM:
		pushShort(apdu_getInBlockSize());
		break;

	case GetIncomingLength_NM:
		popReference();  // pop this
		pushShort(apdu_GetIncomingLength());
		break;

	case SetOutgoing_NM:
		pushShort(apdu_SetOutgoing());
		break;

	case SetOutgoingNoChaining_NM:
		pushShort(apdu_SetOutgoingNoChaining());
		break;

	case SetOutgoingLength_NM:
		apdu_SetOutgoingLength(popShort());
		break;

	case getOutBlockSize_NM:
		pushShort(apdu_GetOutBlockSize());
		break;
		/*@ -typedefparam @*/
		/*@ -discast @*/
		/*@ -type @*/
	case getProtocol_NM:
		pushShort(U8tS16(nativeMethod_GetProtocol()));
		break;
		/*@ +type @*/
		/*@ +discast @*/
		/*@ +typedefparam @*/
	case ReceiveBytes_NM:
		pushShort(apdu_ReceiveBytes(popShort()));
		break;

	case SetIncomingAndReceive_NM:
		pushShort(apdu_ApduSetIncomingAndReceive());
		break;

	case SendBytes_NM:
		nativeMethod_SendBytes();
		break;

	case SendBytesLong_NM:
		nativeMethod_SendBytesLong();
		break;

	case SetOutgoingAndSend_NM:
		nativeMethod_SetOutgoingAndSend();
		break;

	case waitExtension_NM:
		apdu_WaitExtension();
		break;

	case GetCurrentState_NM:
		pushShort(U8t16(apdu_GetCurrentState()));
		break;

	/*case InitramBuffer_NM:
		InitramBuffer();
		break;*/

	case InitshortBuffer_NM:
		InitshortBuffer();
		break;

	case GetNAD_NM:
		//popReference();  // pop this
		pushShort(U8t16(apdu_GetNAD()));
		break;

	case GetOffsetCdata_NM:
		//popReference();  // pop this
		pushShort(apdu_GetOffsetCdata());
		break;
	case GetCLA_NM:
		nativeMehtod_GetCLA();
		break;
	/*case IsCommandChainingCLA_NM:
		//popReference();  // pop this
		pushShort(apdu_IsCommandChainingCLA());
		break;

	case IsValidCLA_NM:
		//popReference();  // pop this
		pushShort(apdu_IsValidCLA());
		break;

	case IsISOInterindustryCLA_NM:
		//popReference();  // pop this
		pushShort(apdu_IsISOInterindustryCLA());
		break;

	case IsSecureMessagingCLA_NM:
		//popReference();  // pop this
		pushShort(apdu_IsSecureMessagingCLA());
		break;*/


#if JCVM__VM__RMI

	case cleanupExportedObjectsArray_NM:
		cleanupExportedObjectsArray();
		break;

	case checkAccess_NM:
		checkAccess();
		break;

	case getAnticollisionString_NM:
		getAnticollisionString();
		break;

	case getRemoteInterfaceNumber_NM:
		getRemoteInterfaceNumber();
		break;

	case getRemoteInterfaceAddress_NM:
		getRemoteInterfaceAddress();
		break;

	case copyInterfaceNameIntoBuffer_NM:
		copyInterfaceNameIntoBuffer();
		break;

	case copyStringIntoBuffer_NM:
		copyStringIntoBuffer();
		break;

	case getObjectID_NM:
		getObjectID();
		break;

	case getClassNameAddress_NM:
		getClassNameAddress();
		break;

	case getRemoteMethodInfo_NM:
		getRemoteMethodInfo();
		break;

	case getReturnType_NM:
		getReturnType();
		break;

	case deleteAllTempArrays_NM:
		deleteAllTempArrays();
		break;

	case isAPIException_NM:
		isAPIException();
		break;

	case rmi_invoker_void_NM:
		rmi_invoker_common();
		break;

	case rmi_invoker_boolean_NM:
		rmi_invoker_common();
		break;

	case rmi_invoker_byte_NM:
		rmi_invoker_common();
		break;

	case rmi_invoker_short_NM:
		rmi_invoker_common();
		break;

	case rmi_invoker_int_NM:
		rmi_invoker_common();
		break;

	case rmi_invoker_reference_NM:
		rmi_invoker_common();
		break;

	case rmi_invoker_array_NM:
		rmi_invoker_common();
		break;

	case getRMIErrorCode_NM:
		getRMIErrorCode();
		break;
	case writeArray_NM:
		writeArray();
		break;
#endif


	case writeTerminationFlag_NM:
		writeTerminationFlag();
		break;
#if !GP__VERSION__FOR_MINI_MASK
	case setATRHistBytes_NM:
		setATRHistBytesNM();
		break;
#endif

#if GP__GP_API__FOR_V1_7_MASK
	case getATRHistBytes_NM:
		pushShort(U8t16(getATRHistBytesNM()));
			break;

#if BOOT__IO__SWP
	case getUIDorPUPIBytes_NM:
		pushShort(U8t16(getUIDorPUPIBytesNM()));
			break;
#endif

#endif

	case load_NM:
		load();
		break;

	case setLoaderStatus_NM:
		setLoaderStatus();
		break;

	case Dispatcher1_NM:
		Dispatcher1();
		break;

	case getCardManagerAID_NM:
		getManagerAID();
		break;

	case iSetSDPersonalizeState_NM:
		pushShort(U8t16(iSetSDPersonalizeState(U16t8(popShort()))));
		break;
		 /*@ -typedefparam @*/
		 /*@ -discast @*/
		 /*@ -type @*/
	case checkOPCommand_NM:
		pushShort(checkOPCommand());
		break;
		/*@ +type @*/
		/*@ +discast @*/
		/*@ +typedefparam @*/

#if GP__AMENDMENT__A__FOR_MASK
	case CASDAPINativeMethod_NM:
		CASDAPINativeMethod();
		break;
#endif
#if GP__CVM__FOR_MASK
	case checkPinValue_NM:
		pushShort(U8t16(checkPinValue()));
		break;

	case updatePinValue_NM:
		pushShort(U8t16(updatePinValue()));
		break;

	case accessPinInfo_NM:
		pushShort(U8t16(accessPinInfo()));
		break;
	case getIsdGlobalPinData_NM:
		getIsdGlobalPinData();
		break;
#endif
	/*case secureSetKeysetKey_NM:
		secureSetKeysetKey();
		break;
	case secureGetKeysetKey_NM:
		secureGetKeysetKey();
		break;*/

#if (GP__RECEIPT__FOR_MASK || GP__TOKEN__FOR_MASK)
	case OPGetKeySetStatus_NM:
		OPGetKeySetStatus();
		break;
#endif
#if	GP__RECEIPT__FOR_MASK
	case OPCMGenData_NM:
		OPCMGenData();
		break;
#endif
#if (GP__TOKEN__FOR_MASK | GP__DAP__FOR_MASK)
	case OPCMVerify_NM://新加
		OPCMVerifyImplement();
		break;
#endif

#if  !GP__VERSION__FOR_MINI_MASK
	case Get_CPLC_Data_NM:
		Get_CPLC_Data();
		break;
	case Put_CPLC_Data_NM://新添加
		Put_CPLC_Data();
		break;
#endif

	case initDiversifiedDataArr_NM: // added by xuxiaojun@20220426 gpBFCCompliance208
		initDiversifiedDataArr();
		break;

	case DecipheredLoadBlockData_NM:
		pushShort(decipheredLoadBlockData());
		break;

#if  (GP__SCP__API__FOR_MASK)
	case SCPAPIImplement_NM:
		SCPAPIImplement();
		break;
#endif

	/*case getDesDapKeyVersion_NM:
		getDesDapKeyVersionNM();
		break;*/
#if GP__AMENDMENT__A__FOR_MASK//新添加
	case checkSCP10Process_NM:
		pushShort((jshort)scp10_process());
		//scp10_process();
		break;
	case clearPKOCE_NM:
		clearPKOCE();
		break;
#endif
	case getSDInfo_NM:
		getSDInfo();
		break;
#if	GP__CGM__FOR_APPPARACHECK
	case roomManage_NM:
		CgmManager_Main();
		break;
	case getCGMarraySize_NM:///added by jiangxiaohui:EPX产业化/THC_THD89/Branch/EPX_THC_Mobile5G/Develop
		getCGMarraySize();
		break;
#endif
	case checkMandatedDAP_NM:
		pushShort(U8t16(checkMandatedDAP()));
		break;
#if !GP__VERSION__FOR_MINI_MASK
	case isActiveApp_NM:
		isActiveApp((u16)popShort());
		break;
#endif
	case MultiSelectState_NM:
		pushShort(U8t16(MultiSelectState(popShort(), selected_channel)));
		break;	

	case getCurrSdId_NM:
		pushShort(U8t16(g_bSDId[selected_channel]));
		break;	

//	case manageKeyMapOfSD_NM:
//		manageKeyMapOfSD();
//		break;
#if GP__AMENDMENT__A__FOR_MASK
	case GenSetKeys_NM:
		GenSetKeys();
		break;
#endif

#if !GP__VERSION__FOR_MINI_MASK
	case dealC9Tag_NM:
		dealC9Tag();
		break;
#endif
	/*case getCurrIsdId_NM:
		pushShort(g_bISDId[selected_channel]);
		break;	*/
	case dataManagerFunction_NM:        //新添加
		dataManager_dataManagerFunction();
		break;
	case GetCardRecognitionData_NM:
		dataManager_getCardRecognitionData();
		break;
	case keyManagerFunction_NM://新添
		keyManagerFunction();
		break;
#if (GP__AMENDMENT__C__FOR_MASK || JCVM__IO__SWP )
	case setRFCommunication_NM:
		setRFCommunication();
		break;
	case getRFCommunication_NM:
		getRFCommunication();
		break;
	case addClEntry_NM://added by jiangxiaohui:优化性能, EPX产业化/THC_THD89/Branch/EPX_THC_Mobile5G/Develop
		addClEntryNM();
		break;
#endif
/*
#if	JCVM__IO__MIFARE
	case readSecurityCodeData_NM:
		pushShort(nativeMethods_readSecurityCodeDataNative());
		break;	
#endif
*/
#if JCVM__VM__JAVACARDX_MATH
// Native methods for javacardx.math package
	case convertToHex_NM:
		conversionPreProcessor(FORMAT_HEX);
		break;
	case convertToBCD_NM:
		conversionPreProcessor(FORMAT_BCD);
		break;
	case isBCDFormat_NM:
		isBCDNumberPreProcessor();
		break;
	case checkForValidBNSize_NM:
		checkForValidBNSize();
		break;
	case setMaxBNValue_NM:
		setMaxBigNumberValue();
		break;
	case initBigNumber_NM:
  	    initBigNumber();
		break;
	case addBigNumbers_NM:
		mathHelper(ADD_NUMBERS);
		break;
	case subtractBigNumbers_NM:
		mathHelper(SUBTRACT_NUMBERS);
		break;
	case multiplyBigNumbers_NM:
		mathHelper(MULTIPLY_NUMBERS);
		break;
	case compareBigNumbers_NM:
		compareBigNumbers();
		break;
	case bigNumberToBytes_NM:
		toBytes();
		break;
	case getBNByteLength_NM:
		getByteLength();
		break;
// end native methods for javacardx.math package
#endif


#if JCVM__VM__JAVACARDX_FRAMEWORK_UTIL_INTX
	case setInt_NM:
		setInt();
		break;
	case makeTransientIntArray_NM:
		makeTransientArray(SIZE_INT, (u8) Header_ArrayType_Int, Int_PA_CLASS_OFFSET);
		break;
#endif

#if JCVM__VM__JAVACARDX_FRAMEWORK_UTIL
	case arrayCopyRepack_NM:
		arrayCopyRepack(true);
		break;
	case arrayCopyRepackNonAtomic_NM:
		arrayCopyRepack(false);
		break;
	case arrayFillGenericNonAtomic_NM:
		arrayFillGenericNonAtomic();
		break;
	case arrayFillGeneric_NM:
	  arrayFillGeneric();
	  break;
	case arrayCompareGeneric_NM:
		arrayCompareGeneric();
		break;
	case arrayFindGeneric_NM:
		arrayFindGeneric();
		break;
#endif //JAVACARDX_FRAMEWORK_UTIL
	
	case getConfig_NM:
		pushShort(getConfig(U16t8(popShort())));
		break;
	/*case getKeyManagerConfig_NM:
		pushShort(getKeyManagerConfig((u8)popShort()));
		break;
	case privateNativeMethodOne_NM:
	  	privateNativeMethod(0);
		break;
		
	case privateNativeMethodTwo_NM:
	  	privateNativeMethod(1);
		break;
		
	case privateNativeMethodThree_NM:
	  	privateNativeMethod(2);
		break;
		
	case privateNativeMethodFour_NM:
	  	privateNativeMethod(3);
		break;
		
	case privateNativeMethodFive_NM:
	  	privateNativeMethod(4);
		break;

	case privateNativeMethodSix_NM:
	  	privateNativeMethod(5);
		break;


		break;

	case getIsdCount_NM:
		pushShort(g_bIsdCount);
		break;
	case checkIsGlobal_NM:
		checkIsGlobal();
		break;
	case getSdPkgId_NM:
		pushShort(JC_SECURITYDOMAIN_ID);
		break;*/
	//TODO by zhangruifeng
	case setGCRequestedFlag_NM:
		setGCRequestedFlag(U16t8(popShort()));
		break;
	case DecreaseNativesFunction_NM:
		DecreaseNativesFunction();
		break;
	case GetContentConfigValue_NM:
		GetContentConfigValue(U16t8(popShort()));
	  break;
	case initISDPrivilege_NM:
		initISDPrivilegeNM();
		break;
	case setISDPrivilege_NM:
		setISDPrivilegeNM();
		break;
	case isdScpData_Init_NM:
		isdScpData_Init();
		break;

//	case changeCheckFlag_NM:
//#if GP__CGM__FOR_MASK
//		CgmInit_SetFlag();
//#endif
		//break; remove warning

	case checkIsVGPconfig_NM:
		pushShort(JcvmConfig_GetVgpConfig()); //pushShort(JcvmConfig_GetOsConf()); modify by xuxiaojun: JcvmConfig_GetVgpConfig的bit位含义与JAVA层checkIsVGPconfig对应
		break;
	case GetImplicitSelectedAppletId_NM:
		pushShort(GetImplicitSelectedAppletIdNM()); //类型 转换为了保证0xFF能正确表示-1
		break;
	case GetNextAppEntryIdxByPriority_NM:
		GetNextAppEntryIdxByPriorityNM();
		break;
	case GetAlgVersion_NM:
		GetAlgVersion();
		break;
	case GetChipIDdata_NM:
		GetChipIDdata();
		break;
	
	case updateManage_NM:
		//updateManage();
		break;
	
	case SeInfo_NM:
		SeInfoNM();
		break;
#if HAL__PATCH	
	case nativeMethod_decrypto_NM:
		nativeMethod_decrypto();
		break;
	case sepatch_init_NM:
		nativeMethod_sepatch_init();
		break;
	case sepatch_download_NM:
		nativeMethod_sepatch_download();
		break;
	case sepatch_InstallandRegister_NM:
		nativeMethod_sepatch_InstallandRegister();
		break;		
	case nativeMethod_sepatch_updatelib_NM:
		nativeMethod_sepatch_updatelib();
		break;		
#endif
#if GP__AMENDMENT__C__FOR_MASK
	case ComputeCurrentProtocolParameter_NM:
		ComputeCurrentProtocolParameterNM();
		break;
	case conflictDetection_NM:
		conflictDetectionNM();
		break;
	case ProtocolParameterBootstrap_NM:
		ProtocolParameterBootstrapNM();
		break;
	case StoreProtocolPara_NM:
		StoreProtocolParaNM();
		break;
	case GetProtocolPara_NM:
		GetProtocolParaNM();
		break;
	case SetCurrentProtocolData_NM:
		SetCurrentProtocolDataNM();
		break;
#endif
#if GP__HCI_API__FOR_MASK
	case hciNoParaNative_NM:
	    nativeMethod_HciNoParaProcess();
		break;
	case hciOneParaNative_NM:
		nativeMethod_HciOneParaProcess();
		break;
	case hciMultiParaNative_NM:
		nativeMethod_HciMulParaProcess();
		break;
	case hciGetReceiveBuffer_NM:
		Swp_Hci_GetReceiveBuffer();
		break;
	case hciSendTransactionEvent_6paras_NM:
		Swp_Hci_Interface_TransactionEvent_6paras();
		break;
	case hciSendTransactionEvent_3paras_NM:
		Swp_Hci_Interface_TransactionEvent_3paras();
		break;
#endif

/*@ -unrecog @*/
/*@ -disspecopt @*/
/*@ -typedefparam @*/
#if GP__NATIVE_METHOD__FOR_LTE
	case NativeFunc0_NM:
		NativeFunc0();
		break;

	case NativeFunc1_NM:
		NativeFunc1();
		break;

	case NativeFunc2_NM:
		NativeFunc2();
		break;

	case NativeFunc3_NM:
		NativeFunc3();
		break;

	case NativeFunc4_NM:
		NativeFunc4();
		break;

	case NativeFunc5_NM:
		NativeFunc5();
		break;

#if LIVE_WRITE_CARD
	case NativeFunc6_NM:
		NativeFunc6();
		break;
#endif

	case NativeFuncBASS_NM:
		NativeFuncBASS();
		break;

	case NativeFuncSBASS_NM:
		NativeFuncSBASS();
		break;
	case NativeFuncSSBASS_NM:
		NativeFuncSSBASS();
		break;

	case readRecord_NM:
		readRecordNM();
		break;
	case updateRecord_NM:
		updateRecordNM();
		break;

	case searchRecord_NM:
		searchRecordNM();
		break;
	case increase_NM:
		increase();
		break;
	case searchRecord_Extend_NM:
		searchRecord_ExtendNM();
		break;
	case getFileSize_NM:
		getFileSize();
		break;

	case markNestedSession_NM:
		markNestedSession();
		break;
	case exitNestedSession_NM:
		exitNestedSession();
		break;

#endif
//	case serviceReturn_NM:
//		serviceReturnNM();
//		break;
//	
//	case sendParaObj_NM:
//		sendParaObj();
//		break;

//	case sendParaShort_NM:
//		sendParaShort();
//		break;

//	case rcvParaShort_NM:
//		rcvParaShort();
//		break;

//	case rcvParaObj_NM:
//		rcvParaObj();
//		break;

//	case resetPointer_NM:
//		resetPointer();
//		break;

	case makeInt_NM:
	{
		u16 right = popShort();
		u16 left = popShort();
		pushInt((left<<16) | right);
		break;
	}
/*@ +typedefparam @*/
/*@ +disspecopt @*/
/*@ +unrecog @*/
//	case objectToShort_NM:
//		//pushReference(popShort()); 不需要处理
//		break;

//	case shortToObject_NM:
//		//pushShort(popReference()); 不需要处理
//		break;

	case setCurrentInvokeAppletNM:
		setCurrentInvokeApplet();
		break;
	
	case verifyTokenAndReciptKey_NM:
		pushShort(U8t16(verifyTokenAndReciptKey()));
		break;

#if LTE__CRYPTO__DES_CIPHER__FOR_JAVA
	case DesCipherInit_NM:
		DesInitNative();
		break;

	case DesCipherCal_NM:
		DesCalcNative();
		break;
#endif
#if LTE__CRYPTO__DES_MAC__FOR_JAVA
	case DesSigInit_NM:
		DesSigInitNative();
		    break;
	case DesSigUpdate_NM:
		DesSigUpdateNative();
			break;
	case DesSigSign_NM:
		DesSigSignNative();
			break;
#endif
#if LTE__CRYPTO__AES_CIPHER__FOR_JAVA
	case AesCipherInit_NM:
		aesInitNative();
			break;
	case AesCipherCal_NM:
		aesCalcNative();
			break;
#endif
#if LTE__CRYPTO__AES_CMAC__FOR_JAVA
	case AesCmacInit_NM:
		aesCmacInitNative();
				break;
	case AesCmacCal_NM:
		aesCmacCalNative();
				break;
#endif

#if LTE__CRYPTO__CRC__FOR_JAVA
	case CrcInit_NM:
		CrcInitNative();
			break;
	case CrcUpdate_NM:
		 CrcUpdateNative();
			break;
	case CrcDofinal_NM:
		 CrcDofinalNative();
			break;
#endif

//	case BackCurrentJavaPoint_NM:
//		BackCurrentJavaPoint();
//		break;

	
#if APP_AUTH
	case F1Function_NM:
		F1FunctionNative();
		break;

	case F2toF5Function_NM:
		F2toF5FunctionNative();
		break;

	case CheckSQN_NM:
		CheckSQNNative();
		break;

	case SetAuthPara_NM:
		SetAuthParaNative();
		break;
	case SetAppAuthEnable_NM:
		SetAppAuthEnableNative();
		break;
#endif
#if UICC__GUOWANG__AUTH_SPECIAL_CONFIG
	case SetOrGetGWSecData_NM:
		SetOrGetGWSecDataNative();
		break;
#endif

	
	case SetbSDId_NM: //modify by lixin@20210816 增加选择、安装、删除应用时对FINAL权限的处理；https://192.168.0.6/svn/COS/EPX/EPX/v3.0/BRANCH/lixin
		SetbSDId();
		break;
	
	case Remote_EncryptAndGenRMAC_NM:
		pushShort(Remote_EncryptAndGenRMAC());
		break;
	case getAppIndexByPrivilege_NM: //modify by lixin@20210816 增加选择、安装、删除应用时对FINAL权限的处理；https://192.168.0.6/svn/COS/EPX/EPX/v3.0/BRANCH/lixin
		pushShort(NM_getAppIndexByPrivilege());
		break;

#if GP__AMENDMENT__H__FOR_MASK
	//用于读写升级过程中Element中的boolean类型的数据
	case BoolElementWrite_NM:
		Upgrade_BoolElementWrite();
	break;
	case BoolElementRead_NM:
	pushShort(Upgrade_BoolElementRead());
	break;
#endif

	case getCrcOrScidInfo_NM:// added by chentao@EPX产业化\STM_ST33G1M2\DEVELOP
		NM_GetCrcOrScidInfo();
		break;

//	case GetGBAAppletEntry_NM: //added by lx@20211019 获取GBA应用的AppletEntry
//		GetGBAAppletEntry();
//		break;

	case getIoBaseChannel_NM:
		pushShort(getIoBaseChannel());
		break;
	
	case getIoMaxChannel_NM:
		pushShort(getIoMaxChannel());
		break;
	default:
		#if VM_WITH_PBOC_API
		if(true == PbocAPI_CallPbocNative(func_index))
		{
			break;
		}
		#endif
		#if JAVA_CARD_UICC
		if(true == JavaUiccAPI_CallNative(func_index)) //modify by xuxiaojun@20220810
		{
			break;
		}
		#endif
		#if WITH_JC305
		if(true == JavaCard305API_CallNative(func_index)) //modify by xuxiaojun@20221020
		{
			break;
		}
		#endif
		fatal_error(UNKNOWN_NATIVE);
		break;
	}
}


