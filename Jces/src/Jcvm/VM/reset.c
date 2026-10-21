/*****************************************************************************
 * File:  reset.c
 * Version 1.29

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify
 * Original author: David Getreu
 ****************************************************************************/
#include "CgmManager.h"
#include "reset.h"
#include "Apdu.h"
#include "lifecycle.h"
#include "profile.h"
#include "stack.h"
#include "context.h"
#include "native_interface.h"
#include "mask.h"
#include "memory.h"
#include "object.h"
#include "interpreter.h"
#include "method.h"
#include "except.h"
#include "error.h"
#include "ob_manager.h"
#include "chn_manager.h"
#include "packageloc.h"
#include "global_variable.h"
#include "RamManager.h"
#include "MemGetReuseRam.h"
#include "MemBackUp.h"
#include "JcvmConfig.h"
#include "opFunction.h"
#include "PbocAPI.h"
#include "Patch.h"
#include "AppAPI.h"
#include "DataManager.h"
#include "tranDeallocate.h"
#include "SCPAPINativeMethod.h"
#include "tranManager.h"
#include "SCPInterface.h"
#include "LteAPI.h"
#include "CLProtocolMngt.h"
#include "Io_Swp.h"
#include "SEEUICCInterface.h"
#include "BootLifeCycle.h"
#include "Interface_PlatformFromLTE.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if GP__SCP__SCP11__FOR_MASK
#include "SCP11.h"			//remove warning,dengyiwen,2022/9/27
#endif

#if BOOTLOADER||COS_UPDATE_ONLINE_BY_NativeOS
#include "FunImp.h"			//remove warning,dengyiwen,2022/9/27
#endif

#if COS_UPDATE_ONLINE_MUTUAL
#include "UpCos.h"
#endif

/**
 * modified by xuxiaojun@20210328 增加变量的初始化
 * merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
 */
void InitGlobalVariable()
{
	#if (!BOOT__MEMORY__VALUE_INIT_BY_SEGMENT)
	    u8  D_A  loop_counter;
	
		initMemBackUpGloableVar();
//		JcvmConfig_Init();
			
		g_bytesLeft     					= 0;
		g_needCheckHash 					= 0;
		g_isResolveStaticField 				= 0;
		e2p_allocation_flag	   				= 0;
		isFirstPage     					= 0;
		g_more          					= 0;
		selected_channel   = 0;
		tempReferenceIDGRT 					= 0xffff;	
		packageDeletedcontext 				= 0xff;
		compactTimer		  				= 0;
		deletingPackage 					= 0x00;
		packageDeletedID 					= 0;

		Pkg_Tabel_Cash						= 0;
		APP_Table_Cash						= 0;

	// 	// used for loader.c
		g_LoaderState 						= 0; // loader current states
		g_previous_cap_tag 					= 0;
		g_cap_tag 							= 0;
		g_capFlags 							= 0;
		g_dataSize 							= 0;
		g_dataOffset	 					= 0;
		t_Component_length 					= 0;
		g_offset 							= 0;
		g_max 								= 0;
		g_componentSize 					= 0;
		g_lastBlock 						= 0;

		G_PkgComponentAddrStartAddr         = 0;
		G_PkgComponentAddrEndAddr           = 0;

		g_capMinor 							= 0;
		g_capMajor 							= 0;
		g_pkgMinor 							= 0;
		g_pkgMajor 							= 0;
		g_savedBytesCount 					= 0;
		g_staticReferenceCount 				= 0;
		class_component_Size 				= 0;
		method_component_Size 				= 0;
		staticfield_component_Size 			= 0;
		export_component_Size 				= 0;
		constantpool_component_Size 		= 0;
		class_component_Addresses 			= 0;
		method_component_Addresses 			= 0;
		staticfield_component_Addresses 	= 0;
		export_component_Addresses 			= 0;
		constantpool_component_Addresses 	= 0;
		returnedTempArrayId 				= 0;
		noOfTemporaryObjects 				= 0;
		CardSecurityState 					= 0;
#if GPC_2_3_1
		g_icvsize							= 0;
#endif
		#if GP__AMENDMENT__C__FOR_MASK
			bEECFG                              = MEMtPN8(GetCurrentCLParaBuf());
		#endif

		m_base								= 0;
		k_base								= 0;
		
		
		PCpkgID								= 0xFF;
		
//		bTempArr							= MEMtPX8(MemGetReuseRam_GetTempArrBuffer(0x118));
#if VM_WITH_PBOC_API
		PbocAPI_Init();
#endif
		AppAPI_initFileIdSys();

		logicalChannelNums 					= JcvmConfig_GetOsLogChnNum();
		dtrSize                             = JcvmConfig_GetOsDtrSize();


		for(loop_counter = 0; loop_counter < MAX_LOGICAL_CHANNELS; U8_INC(loop_counter))
		{
			g_bSDId[loop_counter] = 0;
			g_bISDId[loop_counter] = 0;
			g_SCP_I_Option[loop_counter] = 0;
			g_SCP_Type[loop_counter] = 0;
			g_SCP_seqCounter[loop_counter] = 0;
			g_SCP_BeginEnd_R_MAC[loop_counter] = 0;
		}

		g_bIsdCount = JcvmConfig_GetIsdCount();
		
		//the under variable must be init after delete the clear all ram function.
/*@ -type @*/		_error_env					= 0; /*@ +type @*/
		ecpMem_Set(moreDataFlag, 0, sizeof(moreDataFlag));

		ecpMem_Set(handle_2F00Tag, 0 , sizeof(handle_2F00Tag));

		g_moreCommands				= CHAINING_CLEAR;
		g_putKeyCommands			= 0;
		g_CommandOffset             = 0;
		g_storeDGIKeyFlag           = 0;
		g_KeepRefForSplit           = INIT_KEEP_REF_FOR_SPLIT;
		ecpMem_Set(&channelInfo, 0 , sizeof(channelInfo));

#if  GP__KEY_MANAGER__ECC_KEY
		g_EccKeyParaRef            = 0;
		g_EccKeyParaLen            = 0;
		g_markEccKey               = 0;
		g_storeEccKeyMode          = 0;
#endif

#if (EUICC_ONLY||SE_AND_EUICC)
		for(loop_counter = 0; loop_counter < MAX_LOGICAL_CHANNELS; U8_INC(loop_counter))
		{
			setChannelProfileIndex(loop_counter, ILLEGAL_PROFILEINDEX);
		}
#endif
		g_KeepLenForUsageOrAccess	= 0;
		g_SCP_SCP03t_Flag = 0;

		keyManager_initLogicManager();
		RamManager_init();
		NvmManager_Init(MEMtPN8(U32_A_U16(PFtMEM(staticinit), U16_S_U16(EEPROM_IMAGE_SIZE, U16_M_U16(MASK_MAP_ENTRY_COUNT, MAP_ENTRY_SIZE))))
/*@ -disspecopt @*/			,MEMtPN8(U32_S_U16(MASK_MAP_START, U16_M_U16(MASK_MAP_ENTRY_COUNT, MAP_ENTRY_SIZE))) /*@ +disspecopt @*/
							,MASK_MAP_ENTRY_COUNT, MEMtPN8(getBackupStart()));
		dataManager_init();

#if GP__SCP__API__FOR_MASK
		SCPAPI_initGlobalScpAPI();
#else
		initScpInterface();
#endif
		#if GP__CGM__FOR_APPPARACHECK //改在芯片在，以段形式来初使，加快速度
 			CgmInit_GlobalVar();
		#endif
			scpClr();
		#if GP__RAM_INIT__FOR_LTE
/*@ -unrecog @*/
			initLTEVar();
			lteAlgVarInit();
/*@ +unrecog @*/
		initInterServiceVar();
		#endif

	#else
		initMemBackUpGloableVar();
		JcvmConfig_Init();
		tempReferenceIDGRT 					= 0xffff;	
		packageDeletedcontext 				= 0xff;
		#if GP__AMENDMENT__C__FOR_MASK
			bEECFG                              = MEMtPN8(GetCurrentCLParaBuf());
		#endif

		PCpkgID								= 0xFF;
		
		bTempArr							= MEMtPX8(MemGetReuseRam_GetTempArrBuffer(0x118));
#if VM_WITH_PBOC_API
		PbocAPI_Init();
#endif
		AppAPI_initFileIdSys();

		logicalChannelNums 					= JcvmConfig_GetOsLogChnNum();
		dtrSize                             = JcvmConfig_GetOsDtrSize();

		g_bIsdCount = JcvmConfig_GetIsdCount();

		keyManager_initLogicManager();
		RamManager_init();
		NvmManager_Init(MEMtPN8(U32_A_U32(PFtMEM(staticinit), (EEPROM_IMAGE_SIZE - MASK_MAP_ENTRY_COUNT * MAP_ENTRY_SIZE)))
							,MEMtPN8(MASK_MAP_START - (MASK_MAP_ENTRY_COUNT * MAP_ENTRY_SIZE))
							,MASK_MAP_ENTRY_COUNT, MEMtPN8(getBackupStart()));
		dataManager_init();
		
#if GP__SCP__API__FOR_MASK
		SCPAPI_initGlobalScpAPI();
#else
		initScpInterface();
#endif
	
		#if GP__CGM__FOR_MASK //鏀瑰湪鑺墖鍦紝浠ユ褰㈠紡鏉ュ垵浣匡紝鍔犲揩閫熷害
 			CgmInit_GlobalVar();
		#endif
			scpClr();
/*@ -unrecog @*/
		#if  GP__RAM_INIT__FOR_LTE
			initLTEVar();
/*@ +unrecog @*/
			lteAlgVarInit();
		initInterServiceVar();
		#endif
	#endif

#if (EUICC_ONLY||SE_AND_EUICC)
	G_CurEnabledProfileIndex = ILLEGAL_PROFILEINDEX;
	G_CurExecutingProfileIndex = ILLEGAL_PROFILEINDEX;
#endif
	

	if (BootLifeCycle_IsCardInit())
	{
		MoveEepromResume(); // modify by chentao @20230308 HandleFirstCommand 函数中除MoveEepromResume以外的操作挪到私有指令后处理
		
		StartSecureMoveEepromMapTableExtend();//map表扩展过程中，断电，继续文件系统的搬移

#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
		CheckHeFlashRecordResume();  //对于强更新文件区可能出现的弱写页进行擦除或者实写,by xwh 20211117
#endif

#if (EUICC_ONLY||SE_AND_EUICC)
		EUICCInitGobalVariable();//UICC API测试修改，EventDownloadEvent,复位后，没有清除

//		ExecuEnvironmentInit(ILLEGAL_PROFILEINDEX); //modify by xxj:SE_ONLY, EUICC时，该函数的执行在EUICCInitGobalVariable内部已经完成，出来之后再执行该语句，会把ExecuEnvironmentInit(G_CurExecutingProfileIndex)的结果冲掉
#endif

//		HandleFirstCommand(MODE_RESET);
// modify by xuxiaojun@20221101: 支持SCP11功能时，需要对使用到的多个通道上的全局变量进行初始化,这里不能直接调用SCP11PerformSecurityOperation_InitGlobalVar()		
#if GP__SCP__SCP11__FOR_MASK // merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP

		#if GP__SCP__SCP11__CERT_IN_GLOBAL_RAM	
		ecpMem_Set(G_TmpCertInRam, 0x00, TmpCert_LENGTH * MAX_LOGICAL_CHANNELS);
		#endif
	
		ecpMem_Set(G_SCP11SessionPara, 0x00, SCP11SessionPara_LENGTH * MAX_LOGICAL_CHANNELS);

		ecpMem_Set(g_SCP_CARDCHALLENGE, 0x00, SCP_KEY_LENGTH); // ecpMem_Set(g_current_PerformSecurityOperation, 0x00, PerformSecurityOperation_LENGTH);		
#endif
	}
#if COS_UPDATE_ONLINE_MUTUAL
	InitUpCOSVar();
#endif
	
#if BOOTLOADER||COS_UPDATE_ONLINE_BY_NativeOS
	initBootLoaderVar();
#endif
}

void InitJCRE1()
{
	// initial the Transaction buffer
#if BOOT__MEMORY__BACK_UP
	memInitTransaction();
#endif

	tranMananger_initInfo();
}

void InitJCRE2()
{
	/* initialize E2P Table */
	NvmManager_Boot(MASK_MAP_ENTRY_COUNT, MEMtPN8(getBackupStart()));

	/* Store off the initial exception table list for searching through the exception tables */
	JcvmConfig_SetCurrentExceptionList(INITIAL_EXCEPTION_TABLE_LIST);

	/* initialize content of static fields of masked packages */
	nvmCopyNoAtomic(MEMtPN8(E2P_IMAGE_START), MEMtPC8(staticinit), U16_S_U16(EEPROM_IMAGE_SIZE, U16_M_U16(MASK_MAP_ENTRY_COUNT, MAP_ENTRY_SIZE)));
	/*initialize free table*/
	memRefresh();
}

void InitJCRE3()
{
	init_RAM();
//	isVGPconfig = CONFIG_VGP; // deleted by xuxiaojun@20220621: 不需要浪费一个字节的全局变量，直接用CONFIG_VGP就可以了
//	isGPconfig221 = CONFIG_GP221; 在C层未使用
    
	channel_init();
	initInterpreter();
}

void reset()
{
	PF8		D_A	mainMethodAddr;

	mainMethodAddr = PF8_A_U16(MEMtPF8(getPackageComponentLocation(MAIN_METHOD_PKGID, COMPONENT_METHOD)), MAIN_METHOD_OFFSET);

	// compact DTR and RTR space
	tranDeallocate_compactRam(RE_ALLOCATE_RAM_SPACE);
	
	callmethod(mainMethodAddr, MAIN_METHOD_PKGID, JCRE_CONTEXT);
}
