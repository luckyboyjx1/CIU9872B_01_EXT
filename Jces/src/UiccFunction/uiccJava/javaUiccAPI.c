#include  "javaUiccAPI.h"
#include  "uicc_main.h"
#include  "uicc_interface.h"
#include  "SEEUICCInterface.h"
#include  "ProfileGPCommand.h"
#include  "STKNativeMethod.h"
#include  "STKRemoteAppManagement.h"
#include  "platform_api.h"
#include  "UICCToolkitAPI.h"
#include  "CSIMApiImpl.h"
#include  "SimAPi.h"
#include  "EUICCCustomer.h"
#include  "Interface_AnnexB.h"
#include  "Scp81Connection.h"
#include  "EUICCInterface.h"
#include  "tranManager.h"
#include  "EUICCProfile.h"
#include  "EUICCRAM.h"
#include  "euiccConfig.h"
#include  "VMAccessEeprom.h"
#include  "T0ApduImpl.h"
#include  "USATEnvelopeHandler.h"
#include  "EUICCPersonalAPDU.h"
#include  "nativemethod_impl.h"
/*
*@description:NM_GetCOSVersionData
×
*@param      :void
*
*@return     :void
*
*@history    :created by xuxiaojun@20190410
*            :modify by xuxiaojun@20220318 分层
*/
void NM_GetCOSVersionData(void)
{
	u8 I_A len;

	len = VMReadByte(CARDCFG_COSVersion);

	VMReadBuf(&RecData[0], CARDCFG_COSVersion+1, len);

	SetOutgoingAndSend(0x05, len);
}
//modify by xuxiaojun@20221020: 根据 合代码时代码评审(20220722).xlsx进行修改： 将JAVA CARD305的本地方法放在nativemethod_impl_JavaCard305中；javaUicc的本地方法放在javaUiccAPI中
u8 JavaUiccAPI_CallNative(u16 fun_index)    SMALL_MODE
{
	u8		D_A    isCallJavaUiccNative;

	isCallJavaUiccNative = true;

	switch (fun_index)
	{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP//modify by zhxx@20221204，将toolkit应用的信息放在应用中，不放在文件中
	    case GetSetToolkitInfoRef_NM:
		  PushOperandStackShort(GetSetToolkitInfoRef());
		break;

#endif
		case BackCurrentJavaPoint_NM:
			BackCurrentJavaPoint();
			break;
	    case GetGBAAppletEntry_NM: //added by lx@20211019 获取GBA应用的AppletEntry
	    	GetGBAAppletEntry();
	    	break;
		case setSDTarInfo_NM:
			setSDTarInfoNM();
			break;
		case NativeUICC_Main_NM:
			NativeUICC_Main();
			break;
		case processUICCPara_NM:
			ProcessUICCPara();
			break;
		case NativeThrowError_NM:
			throwPrivateError();
			break;
		case SetErrorCode_NM:
			SetJavaReturnSW(popShort());
			break;
		case SetIsoBusyExceptionFlag:
			setISOBusyExceptionFlag();
			break;
		case checkContextForVolatileByteArray_NM:
			STKCheckContextForVolatileByteArray();
			break;
		case getJCREMainProcessMode_NM:
			NM_GetJCREMainProcessMode();
			break;
		case GetCurrentToolkitRef_NM:
			NM_GetCurrentToolkitRef();
			break;
		case GetCurrentToolkitAPPID_NM:
			NM_GetCurrentToolkitAPPID();
			break;
		case getCurrentToolkitEvent_NM:
			NM_GetCurrentToolkitEvent();
			break;
		case deleteAppInfoInFile_NM:
			NativeImpl_Delete(popShort());
			break;
		case setToolkitAppStatus_NM:
			ToolkitAppLifecycleChange();
			break;

#if UICC__FUNCTION__UICCAPI
		case UICCTOOLKIT_appendTLV_3:
			UICCToolkit_appendTLV_3();
			break;

		case UICCTOOLKIT_appendTLV_5:
			UICCToolkit_appendTLV_5();
			break;

		case UICCTOOLKIT_appendTLV_6:
			UICCToolkit_appendTLV_6();
			break;

		case UICCTOOLKIT_appendTLV_8:
			UICCToolkit_appendTLV_8();
			break;

		case UICCTOOLKIT_initMoreTime:
			UICCToolKit_initMoreTime();
			break;

		case UICC_ProactiveResponse_Toolkit_getChannelStatus:
			UICCProactiveResponse_Toolkit_getChannelStatus();
			break;

		case UICCTOOLKIT_getBERTag:
			UICCToolkit_getBERTag();
			break;

		case UICCTOOLKIT_getBERSize:
			UICCToolkit_getBERSize();
			break;

		case UICC_Envelope_Toolkit_getItemIdentifier:
			UICCEnvelope_Toolkit_getItemIdentifier();
			break;

		case UICC_Envelope_Toolkit_getChannelIdentifier:
			UICCEnvelope_Toolkit_getChannelIdentifier();
			break;

		case UICC_Envelope_Toolkit_getChannelStatus:
			UICCEnvelope_Toolkit_getChannelStatus();
			break;

		case UICCTOOLKIT_getValueShort:
			UICCToolkit_getValueShort();
			break;

		case UICCTOOLKIT_setEventList:
			UICCToolkit_setEventList();
			break;

		case UICCTOOLKIT_allocateServiceIdentifier:
			UICCToolkit_allocateServiceIdentifier();
			break;

		case UICCTOOLKIT_releaseServiceIdentifier:
			UICCToolkit_releaseServiceIdentifier();
			break;

		case UICCTOOLKIT_registerFileEvent_7para:
			UICCToolkit_registerFileEvent_7para();
			break;

		case UICCTOOLKIT_deregisterFileEvent_7para:
			UICCToolkit_deregisterFileEvent_7para();
			break;

		case UICCTOOLKIT_deregisterFileEvent_2para:
			UICCToolkit_deregisterFileEvent_2para();
			break;

		case UICCTOOLKIT_registerFileEvent_2para:
			UICCToolkit_registerFileEvent_2para();
			break;

		case UICCTOOLKIT_setMenuEntryTextAttribute:
			UICCToolkit_setMenuEntryTextAttribute();
			break;

		case UICCTOOLKIT_selectPataByteSFI:
			UICCToolkit_selectPataByteSFI();
			break;

		case UICCTOOLKIT_searchRecord:
			UICCToolkit_searchRecord();
			break;

		case UICCTOOLKIT_CopyJavaContextToNativeContext:
			UICCToolkitCopyJavaContextToNativeContext();
			break;

		case UICCTOOLKIT_CopyNativeContextToJavaContext:
			UICCToolkitCopyNativeContextToJavaContext();
			break;

		case UICCTOOLKIT_GetADFName:
			UICCToolkit_GetADFName();
			break;

		case UICCTOOLKIT_deleteFile:
			UICCToolkit_deleteFile();
			break;

		case UICCTOOLKIT_createFile:
			UICCToolkit_createFile();
			break;

		case UICCTOOLKIT_resizeFile:
			UICCToolkit_resizeFile();
			break;

		case UICCTOOLKIT_CopyHandlerContextToNative:
			UICCToolkitCopyHandlerContextToNative();
			break;

		case UICCTOOLKIT_CopyHandlerContextToJava:
			UICCToolkitCopyHandlerContextToJava();
			break;

		case UICC_post:
			UICCpost();
			break;

		case UICC_postAsBERTLV:
			UICCpostAsBERTLV();
			break;
	//
	//	case isAppletRegisterInvoked:
	//		IsAppletRegisterInvoked();
	//		break;

		case UICCBERTLVHandler_GetValueLength:
			UICCBERTLVHandlerGetValueLength();
			break;

		case UICCBERTLVHandler_GetSize:
			UICCBERTLVHandlerGetSize();
			break;

		case UICCBERTLVHandler_GetLength:
			UICCBERTLVHandlerGetLength();
			break;

		case USATEnvelop_GetShortMessageOffset:
			USATEnvelop_getShortMessageOffset();
			break;

		case USATEnvelop_GetShortMessageLength:
			USATEnvelop_getShortMessageLength();
			break;
#endif // UICC__FUNCTION__UICCAPI

#if UICC__FUNCTION__CSIMAPI
		case CSIM_GetSecuredDataOffset:
			CSIMGetSecuredDataOffset();
			break;
		case CSIM_GetSecuredDataLength:
			CSIMGetSecuredDataLength();
			break;
		case CSIM_GetUserDataLength:
			CSIMGetUserDataLength();
			break;
		case CSIM_FindTLVUnderBearerData:
			CSIMFindTLVUnderBearerData();
			break;
#endif // UICC__FUNCTION__CSIMAPI

#if UICC__FUNCTION__SIMAPI
		case TOOLKIT_SelectFCI:
			Toolkit_SelectFCI();
			break;

		case TOOLKIT_SelectById:
			Toolkit_SelectById();
			break;

		case TOOLKIT_UpdateBinary:
			Toolkit_UpdateBinary();
			break;

		case TOOLKIT_Status:
			Toolkit_Status();
			break;

		case TOOLKIT_ReadBinary:
			Toolkit_ReadBinary();
			break;

		case TOOLKIT_Invalidate:
			Toolkit_Invalidate();
			break;

		case TOOLKIT_Rehabilitate:
			Toolkit_Rehabilitate();
			break;

		case TOOLKIT_SeekCommand:
			Toolkit_SeekCommand();
			break;

		case TOOLKIT_ReadRecord:
			Toolkit_ReadRecord();
			break;

		case TOOLKIT_UpdateRecord:
			Toolkit_UpdateRecord();
			break;

		case TOOLKIT_increase:
			Toolkit_increase();
			break;

		case TOOLKIT_GetHandlerStatus:
			Toolkit_GetHandlerStatus();
			break;

		case TOOLKIT_GetHanlderCapacity:
			Toolkit_GetHanlderCapacity();
			break;

		case TOOLKIT_init:
			ToolKit_init();
			break;

		case TOOLKIT_initDisplayText:
			ToolKit_initDisplayText();
			break;

		case TOOLKIT_initGetInkey:
			ToolKit_initGetInkey();
			break;

		case TOOLKIT_initGetInput:
			ToolKit_initGetInput();
			break;

		case TOOLKIT_initCloseChannel:
			ToolKit_initCloseChannel();
			break;

		case TOOLKIT_Send:
			Toolkit_Send();
			break;

		case TOOLKIT_setEvent:
			Toolkit_setEvent();
			break;

		case TOOLKIT_setEventList:
			Toolkit_setEventList();
			break;

		case TOOLKIT_clearEvent:
			Toolkit_clearEvent();
			break;

		case TOOLKIT_isEventSet:
			Toolkit_isEventSet();
			break;

		case TOOLKIT_disableMenuEntry:
			Toolkit_disableMenuEntry();
			break;

		case TOOLKIT_enableMenuEntry:
			Toolkit_enableMenuEntry();
			break;

		case TOOLKIT_initMenuEntry:
			Toolkit_initMenuEntry();
			break;

		case TOOLKIT_changeMenuEntry:
			Toolkit_changeMenuEntry();
			break;

		case TOOLKIT_allocateTimer:
			Toolkit_allocateTimer();
			break;

		case TOOLKIT_releaseTimer:
			Toolkit_releaseTimer();
			break;

		case TOOLKIT_requestPollInterval:
			Toolkit_requestPollInterval();
			break;

		case TOOLKIT_getPollInterval:
			Toolkit_getPollInterval();
			break;

		case TOOLKIT_copy:
			Toolkit_copy();
			break;

		case TOOLKIT_getLength:
			Toolkit_getLength();
			break;

		case TOOLKIT_findTLV:
			Toolkit_findTLV();
			break;

		case TOOLKIT_getValueLength:
			Toolkit_getValueLength();
			break;

		case TOOLKIT_getValueByte:
			Toolkit_getValueByte();
			break;

		case TOOLKIT_copyValue:
			Toolkit_copyValue();
			break;

		case TOOLKIT_compareValue:
			Toolkit_compareValue();
			break;

		case TOOLKIT_findAndCopyValue_3:
			Toolkit_findAndCopyValue_3();
			break;

		case TOOLKIT_findAndCopyValue_6:
			Toolkit_findAndCopyValue_6();
			break;

		case TOOLKIT_findAndCompareValue_3:
			Toolkit_findAndCompareValue_3();
			break;

		case TOOLKIT_findAndCompareValue_6:
			Toolkit_findAndCompareValue_6();
			break;

		case TOOLKIT_clear:
			Toolkit_clear();
			break;

		case TOOLKIT_appendArray:
			Toolkit_appendArray();
			break;

		case TOOLKIT_appendTLV_1:
			Toolkit_appendTLV_1();
			break;

		case TOOLKIT_appendTLV_2:
			Toolkit_appendTLV_2();
			break;

		case TOOLKIT_appendTLV_3:
			Toolkit_appendTLV_3();
			break;

		case TOOLKIT_appendTLV_4:
			Toolkit_appendTLV_4();
			break;

		case TOOLKIT_getGeneralResult:
			Toolkit_getGeneralResult();
			break;

		case TOOLKIT_getAdditionalInformationLength:
			Toolkit_getAdditionalInformationLength();
			break;

		case TOOLKIT_copyAdditionalInformation:
			Toolkit_copyAdditionalInformation();
			break;

		case TOOLKIT_getItemIdentifier:
			Toolkit_getItemIdentifier();
			break;

		case TOOLKIT_getTextStringLength:
			Toolkit_getTextStringLength();
			break;

		case TOOLKIT_getTextStringCodingScheme:
			Toolkit_getTextStringCodingScheme();
			break;

		case TOOLKIT_copyTextString:
			Toolkit_copyTextString();
			break;

		case TOOLKIT_copyChannelData:
			Toolkit_copyChannelData();
			break;

		case TOOLKIT_getEnvelopeTag:
			Toolkit_getEnvelopeTag();
			break;

		case TOOLKIT_getTPUDLOffset:
			Toolkit_getTPUDLOffset();
			break;

		case TOOLKIT_getSecuredDataOffset:
			Toolkit_getSecuredDataOffset();
			break;

		case TOOLKIT_getSecuredDataLength:
			Toolkit_getSecuredDataLength();
			break;

		case TOOLKIT_getChannelIdentifier:
			Toolkit_getChannelIdentifier();
			break;

		case TOOLKIT_getUserDataLength:
			Toolkit_getUserDataLength();
			break;

		case TOOLKIT_Post:
			Toolkit_Post();
			break;

		case TOOLKIT_PostAsBERTLV:
			Toolkit_PostAsBERTLV();
			break;

		case TOOLKIT_profileCheckByte1:
			Toolkit_profileCheckByte1();
			break;

		case TOOLKIT_profileCheckPara3:
			Toolkit_profileCheckPara3();
			break;

		case TOOLKIT_profileCheckShort1:
			Toolkit_profileCheckShort1();
			break;

		case TOOLKIT_profileGetValue:
			Toolkit_profileGetValue();
			break;

		case TOOLKIT_profileCopy:
			Toolkit_profileCopy();
			break;

		case TOOLKIT_SIMViewCopyFileContext:
			Toolkit_SIMViewCopyFileContext();
			break;

		case TOOLKIT_SIMViewRestoreFileContext:
			Toolkit_SIMViewRestoreFileContext();
			break;
#endif // UICC__FUNCTION__SIMAPI
//	case IsUiccApplet_NM:
//		pushShort(IsUiccApp(popReference()));
//		break;
		case ClearAllCounter_NM:
			ClearAllCounter();
			break;

#if (EUICC_ONLY||SE_AND_EUICC)
		case ECASDPersonal_NM:
			NM_ECASDPersonal();
			break;

		case setProfileInstallAppId_NM:
			setProfileInstallAppId();
			break;

		case GetISDRSelectOtherResponse_NM:
			NM_GetISDRSelectOtherResponse();
			break;

		case getCurrentEnableProfileIndex_NM:
			pushShort(U8tS16(G_CurEnabledProfileIndex));
			break;

		case getChannelProfileIndex_NM:
		{
			pushShort(U8tS16(getChannelProfileIndex(popShort())));
			break;
		}
		case getChannelProfileIndexToBeSet_NM:
		{
			pushShort(U8tS16(getNextProfileIndex()));
			break;
		}
		case getCurExecutingProfileIndex_NM:
		{
			pushShort(U8tS16(G_CurExecutingProfileIndex));
			break;
		}
		case setCurExecutingProfileIndex_NM:
		{
			G_CurExecutingProfileIndex = popShort();
			ExecuEnvironmentInit(G_CurExecutingProfileIndex);
			break;
		}

		case setEUICCRTP_ProfileInitial_NM:
		{
			JcvmConfig_SetEUICCRTP_ProfileInitialBack(tranMananger_getRtp());//备份profile当前RTP 用于恢复
			break;
		}

		case copySETableToProfile_NM:
			CopySETableToProfile();
			break;


		case getProfileGPState_peExtraditeSDID_NM:
		{
			pushShort(U8tS16(G_profileGPState.peExtraditeSDID));
			break;
		}

		case getProfileGPState_peLifeCycle_NM:
		{
			pushShort(U8tS16(G_profileGPState.peLifeCycle));
			break;
		}

		case profileRestore_NM:
			DownloadOrDeleteProfileResume();
			break;

		case IsRAMProcessed_NM:
			NM_IsRAMProcessed();
			break;

#if RSP_EUICC
		case Clearresponse_NM:
			Clearresponse();
			break;

		case ISDRStoreData_NM:
			NM_CUSTOMERISDRStoreData();
			break;

		case ISDR_GetResponse_NM:
			ISDR_GetResponse();
			break;
#endif


#if M2M_EUICC

		case CheckISDRECASDAID_NM:
			SEUICC_CheckISDRECASDAID();
			break;

		case ProcessPersonalProfileCommand_NM:
			NM_PersonalProfileCommand();
			break;

		case M2M_ValidIns_NM:
			NM_M2M_ValidIns();
			break;

		case M2MISDPStoreDataPersonal_NM:
			NM_M2MISDPStoreDataPersonal();
			break;

		case M2M_StoreData_UpdateSMSRAddressingParameters_NM:
			M2M_StoreData_UpdateSMSRAddressingParameters();
			break;

		case M2M_ISDRRAMCommand_NM:
			M2M_ISDRRAMCommand();
			break;

		case M2M_ISDPRAMCommand_NM:
			M2M_ISDPRAMCommand();
			break;

		case GetProfileLifycycle_NM:
			pushShort(TransformISDPLifeCycle(popShort()));
			break;

		case GetProfileAttributes_NM:

			pushShort(M2M_GetProfileAttributes(popShort())); // modify by chentao@20220929 ProfileAttribute include fallback,Emergency and test
			break;
#endif  //M2M_EUICC
#endif //(EUICC_ONLY||SE_AND_EUICC)

		case GetStatusSDMenuInfo_NM:
			GetStatusSDMenuInfo();
			break;

		case GetStatusToolkitAppMenuInfo_NM:
			GetStatusToolkitAppMenuInfo();
			break;
		case IF_GetADFByAID_NM:
		{
			IF_GetADFByAID();
			break;
		}
		case GetRAMProcesser_NM:
			pushShort(GetRAMProcesser());
			break;

#if GP__AMENDMENT__B__requestHTTPAdministrationSession // modify by xuxiaojun@20220318 分层
		case GPRequestHTTPAdministrationSession_NM:
			NM_GPRequestHTTPAdministrationSession();
			break;

		case getCurrentHttpStatus_NM:
			NM_getCurrentHttpStatus();
			break;

		case getHttpAPPID_NM:
			NM_getHttpAPPID();
			break;
#endif
#if GP__AMENDMENT__B__FOR_MASK	 // modify by xuxiaojun@20220318 分层
		case GetAppletRefByAppId_NM:
			NM_GetAppletRefByAppId();
			break;
#endif

#if GP__AMENDMENT__B__createSCP81Connection_FOR_MASK
		case scp81ConnectionOpen_NM:
			NM_scp81ConnectionOpen();
			break;

		case scp81ConnectionSend_NM:
			NM_scp81ConnectionSend();
			break;

		case scp81ConnectionClose_NM:
			NM_scp81ConnectionClose();
			break;

		case scp81getCurrentEventStatus_NM:
			NM_scp81getCurrentEventStatus();
			break;

		case scp81BakToolkitEvent_NM:
			NM_scp81BakToolkitEvent();
			break;

		case scp81RecoverToolkitEvent_NM:
			NM_scp81RecoverToolkitEvent();
			break;

		case scp81ConnectionStatusGetDetails_NM:
			NM_scp81ConnectionStatusGetDetails();
			break;

		case scp81CopyReceivedData_NM:
			NM_scp81CopyReceivedData();
			break;

		case scp81GetReceivedDataLength_NM:
			NM_scp81GetReceivedDataLength();
			break;

		case scp81SetConnectionObserver_NM:
			NM_scp81SetConnectionObserver();
			break;

		case scp81contentTypeIsCorrect_NM:
			NM_scp81contentTypeIsCorrect();
			break;

		case scp81responseIsComplete_NM:
			NM_scp81responseIsComplete();
			break;
#endif //GP__AMENDMENT__B__createSCP81Connection_FOR_MASK

		case SelectSuccessJavaApp_NM:
			SelectSuccessJavaApp();
			break;

		case NativeImpl_EndOfRegister_NM:
			NativeImpl_EndOfRegister();
			break;
	#if (EUICC_ONLY||SE_AND_EUICC)  //modify by zhxx@20221208，解决SE编译不过问题
		case GetCOSVersionData_NM:
			NM_GetCOSVersionData();
			break;

		case getISDRAID_NM:// added by chentao@EPX产业化\STM_ST33G1M2\DEVELOP
			getISDRAID();
			break;
#endif
#if RSP_EUICC
	case IsProfielChangeOngoing_NM:
		pushReference(IF_ProfielChangeOngoing());
		break;
	case IsProfileLoadingSession_NM:
		pushReference(IF_IsProfileLoadingSession());
		break;
	case IsGetResponseOngoing_NM:
		pushReference(RSP_IsGetResponseOngoing());
		break;
	case RSP_InitGobalVariable_NM:
		//RSP_InitGobalVariable();//modify bychentao@20230322 祥承测试特定终端下载profile失败，发现是终端在profile下载过程中有关闭通道操作，所以修改关通道不清会话
		break;
	case SaveICCIDAndAID_NM:
		SaveICCIDAndAID();
		break;
#endif
#if UICC__FUNCTION_CMCCAPI
		case CheckGlobalArray_NM:
			NM_CheckGlobalArray();
			break;
		case GetKs_int_NAF_NM:
			NM_GetKs_int_NAF();
			break;
		case Check_AFL_NM:
			NM_Check_AFL();
			break;

		case CMCCGetOnlineState_NM:
			NM_CMCCGetOnlineState();
			break;

		case CMCCGetM1Record_NM:
			NM_CMCCGetM1Record();
			break;

		case CMCCGetUIDRecord_NM:
			NM_CMCCGetUIDRecord();
			break;

		case CMCCClearM1Record_NM: // clear M1刷卡记录, added by xwh@20220129
			NM_CMCCClearM1Record();
			break;

		case CMCCClearM1Tag_NM: // clear M1刷卡记录, added by xwh@20220129
			NM_CMCCClearM1Tag();
			break;

		case CMCCSetNFCRecordState_NM:
			NM_CMCCSetNFCRecordState();
			break;

		case CMCCGetNFCRecordState_NM:
			NM_CMCCGetNFCRecordState();
			break;

		case CMCCGetCPUShuakaRecord_NM:
			NM_CMCCGetCPUShuakaRecord();
			break;

		case CMCCSetCPUShuakaRecord_NM:
			NM_CMCCSetCPUShuakaRecord();
			break;

		case IsDesignateAPP_NM:
			NM_IsDesignateAPP();
			break;
#endif// UICC__FUNCTION_CMCCAPI
#if UICC__OPERATOR__CHINAMOBILE
		case IsUsimADF_NM:
			isUsimADF_NM();//pushShort(IsUsimADF(locateObjectInMemory(popReference()),popReference(),popReference()));
			break;
#endif

#if UICC__APP__WHITECARD
		case WhiteCard_Main_NM:
			WhiteCard_Main();
			break;

		case WhiteCard_SetState_NM:
			WhiteCard_SetState();
			break;
#endif

	case IsUPCOSCommand_NM:
#if COS_UPDATE_ONLINE_MUTUAL
		pushShort(NM_IsUPCOSCommand());
#else
		pushShort(0);
#endif
		break;
#if COS_UPDATE_ONLINE_MUTUAL
	case STK_UpgradeFunc_NM:
		STK_UpgradeFunc();
		break;
#endif
		default:
			isCallJavaUiccNative = false;
	}

	return isCallJavaUiccNative;
}


