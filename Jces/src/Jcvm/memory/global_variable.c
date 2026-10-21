/*****************************************************************************
 * @(#)mematom.c	1.20 03/08/13
 * Version 1.7
 *
 wang hui
 eastcompeace Technology Corporation

 History:
 Last modified on 07/07/30
 .Created
 Original authors:
 ****************************************************************************/
#include "global_variable.h"
#include "DataOperation.h"
#include "JcvmRam.h"
#include "jcvmConfigMethod.h"
#include "profile.h"
#if VM_WITH_DH
#include "Crypto_DH.h"
#endif
//#include "VMDebug.h"

#if HAL__MEMORY__RAM_MAP_IN_C51
#pragma JCVM_GLOBAL_VARIABLE_RAM_START
#endif

/****************************************************************************************************************************
***************************************************used for stack************************************************************
*****************************************************************************************************************************/
PX8 										D_A 		JSP; // JCVM stack pointer: points to the next free stack cell
PX8 										D_A 		FP; //8-bit pointer to current stack frame: points to the first local of current method
PX8											D_A			FPend;
u8											D_A 		FPsize;
PX8 										D_A 		Top; //top of free space in the stack frame
u32											X_A			VMStack[STACKSIZE/4];//AT FXRAMBase;//modify by jiangxiaohui@20220324 分层
u8											I_A			CardSecurityState;
/****************************************************************************************************************************/
PX8 										D_A 		BackTop;
PX8 										D_A 		BackFP;
/****************************************************************************************************************************
***************************************************used for VM***************************************************************
*****************************************************************************************************************************/ 
memref 										X_A 		cachedClassCompLoc ; //Performance optimization: reduce package table lookup
memref 										X_A 		cachedMethodCompLoc ;
memref 										X_A 		cachedFieldCompLoc ;
PF8		 									D_A 		JPC ;
u8 											I_A 		PCpkgID ;// 8-bit package ID for the current PC
u16 										I_A 		cc ;//Current Context Register.  Low nibble contains the applet ID of current   applet context. High nibble contains the package ID of current group   context. Special JCRE context is 0x00. */
u8 											D_A 		localsize;//Number of locals in current method
u8 											D_A 		paramsize;// Number of params in current method
u8 											I_A 		selected_channel;// context of currently selected applet
u8 											I_A 		lastVMPkg;
jref 										X_A 		tempReferenceIDGRT;//cached jref.
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for temp_ob_manager**************************************************
*****************************************************************************************************************************/
jref 										X_A 		returnedTempArrayId;
u8 											X_A 		noOfTemporaryObjects;
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for sd key management************************************************
*****************************************************************************************************************************/ 
u8 											X_A			g_bSDId[MAX_LOGICAL_CHANNELS];
u8											X_A			g_bISDId[MAX_LOGICAL_CHANNELS];
u8											X_A			g_bIsdCount;

u16											X_A			g_EccKeyParaRef; //F0 的值
u8											X_A			g_EccKeyParaLen;
u8                                          X_A         g_PutEccKey;     //当前的put key指令在执行put ecc key
u8											X_A			g_markEccKey;//bit0 - F0,bit 1 -B0/B1, bit2~bit7 - B2~B7
u8											X_A			g_storeEccKeyMode;
u8											X_A			g_storeDGIKeyFlag;
u8 											X_A			g_isSetDefaultKey;
u8											X_A			g_keyDataOffest;
u8											X_A			g_keyUsageValueOffest;
u8    										X_A			g_keyAccessValueOffest;
u8											X_A			g_PutKeyId;
u8											X_A			g_isNewKey;
u8    										X_A			g_putKeyCommands;
u16											X_A			g_PutKeyRemaindLen;
u16											X_A			g_PutkeyDataLen;
u16											X_A			g_KeepRefForSplit;
u16											X_A			g_putKeyCurrState;
u16											X_A			g_KeepLenForUsageOrAccess;

/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for opFunction*******************************************************
*****************************************************************************************************************************/
//u8 											X_A			currKeySetVer;
u8 											X_A			sesKey[16];
u8 											X_A			macKey[16];
u8 											X_A			deKey[16];
u8 											X_A			lastMAC[8];
u8 											X_A			cardChallenge[CARD_CHALLENGE_SIZE]	;//AT	CARD_CHALLENGE_BASE;
u8 											X_A			hostChallenge[8];
u8 											X_A			TEMPBuffer[TEMPBUFFER_SIZE]		;//AT 	TEMPBUFFER_BASE;
u8 											X_A			currKeySetVer[MAX_LOGICAL_CHANNELS];
u8 											X_A			moreDataFlag[MAX_LOGICAL_CHANNELS];
u8											X_A			g_ExtraPkgOff[MAX_LOGICAL_CHANNELS];
u8											X_A			g_ExtraAppOff[MAX_LOGICAL_CHANNELS];
u8    										X_A			g_PkgOff[MAX_LOGICAL_CHANNELS];
u8											X_A			g_AppOff[MAX_LOGICAL_CHANNELS];
u8											X_A			g_SdOff[MAX_LOGICAL_CHANNELS];
u16											X_A			g_CommandOffset;
/****************************************************************************************************************************/


#if HAL__MEMORY__RAM_MAP_IN_C51
#pragma JCVM_LOADER_RAM_START
#endif

/****************************************************************************************************************************
***************************************************used for loader***********************************************************
*****************************************************************************************************************************/
u8 											X_A			g_LoaderState; // loader current states
u8 											X_A			g_previous_cap_tag;
u8 											X_A			g_cap_tag ;
u8 											X_A			g_capFlags;
u16 										X_A			g_dataSize;
u16 										X_A			g_dataOffset;
u16 										X_A			t_Component_length;
u16 										I_A			g_offset;
u16 										I_A			g_max;
u32 										I_A			g_componentSize;
u8 											I_A			g_lastBlock;
u8 											I_A			g_capMinor;
u8 											I_A			g_capMajor;
u8 											I_A			g_pkgMinor;
u8 											I_A			g_pkgMajor;
u8 											I_A			g_savedBytesCount;
u8 											X_A			g_savedBytes[3];
u1											B_A			g_more;
u1											B_A			g_bytesLeft;
u1											B_A			g_needCheckHash;
u1											B_A			g_isResolveStaticField;
u16											X_A			g_loadComplete;
u8 											X_A			g_appletAID[16];
u8 											X_A			f_pkgIDs[16];
u16 										X_A			g_staticReferenceCount;
u16 										X_A			class_component_Size;
u16 										X_A			method_component_Size;
u16 										X_A			staticfield_component_Size;
u16 										X_A			export_component_Size;
u16 										X_A			constantpool_component_Size;
#if CONSTANT_POOL_COMPONENT_IN_RAM
u8											X_A			CONSTANTPOOL_COMPONENT[0x100];
#endif
memref 										X_A			class_component_Addresses;
memref 										X_A			method_component_Addresses;
memref 										X_A			staticfield_component_Addresses;
memref 										X_A			export_component_Addresses;
memref 										X_A			constantpool_component_Addresses;

memref                                      X_A         G_PkgComponentAddrStartAddr;
memref                                      X_A         G_PkgComponentAddrEndAddr;

u16 										X_A			class_component_offset;
u16											X_A			method_component_offset;
u16 										X_A			export_component_offset;
u16 										X_A			constantpool_componentoffset;
u8 											X_A 		f_currentState;
u8 											X_A 		g_aidCount;
u8 											X_A			g_aidLength;
u8 											X_A 		g_aidOffset;
#if GPC_2_3_1
u8 											X_A 		g_icvsize;
u8 											X_A 		g_icvbuf[16];
#endif
/*@ -namechecks @*/
u8 											X_A 		m_count;
u8 											X_A 		m_base;
/*@ +namechecks @*/
u8 											X_A 		k_count;
u8 											X_A 		k_base;
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for WatchExeptionISR*************************************************
*****************************************************************************************************************************/
//u8											X_A			patchRamEntry[48];
//u16											X_A			patchEntrySel;
//u8 											X_A 		savedA;
//u32 										X_A			savedEPTR;
//u32											X_A			patchTempAddress;
//											B_A			patchEEP;
//											B_A			patchServiceSel;
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for patch************************************************************
*****************************************************************************************************************************/
//PatchTable *								X_A			PatchTableEntry;
//u16 										X_A			PatchTableNumber;
//u8 											X_A			patchRamArea[PATCH_RAM_SIZE];
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for GC***************************************************************
*****************************************************************************************************************************/
u1											B_A			deletingPackage;		//variable to indicate if package deletion is in progress
u8 											X_A			packageDeletedcontext;	//variable to indicate which package's context is being deleted
u8											X_A			packageDeletedID;		//variable to indicate which package is being deleted
u8											X_A			appletDeletion;
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for contentManager***************************************************
*****************************************************************************************************************************/
memref 										X_A 		Pkg_Tabel_Cash;
memref 										X_A 		Pkg_Tabel_Object_Cash;
memref 										X_A 		APP_Table_Cash;
//memref 										X_A			APP_Table_Object_Cash;
memref 										X_A 		APP_Entry_Object_Cash;
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for memory***********************************************************
*****************************************************************************************************************************/
u1											B_A			e2p_allocation_flag;
u32             							X_A      	g_maxEntryAdd;
u16 										X_A			RAM_channel_info;
u16 										X_A			dtrSize;
u32											X_A			g_TotalReqNVMSize;

/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for nativemethod_impl************************************************
*****************************************************************************************************************************/
memref 										X_A			wFreeEESize;
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for map table bit map************************************************
*****************************************************************************************************************************/
u1											B_A 		isFirstPage;
u8  										X_A  		compactTimer;
u32											X_A			NVM_FREE_TABLE_START;
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for logical channel**************************************************
*****************************************************************************************************************************/
u8  										X_A			logicalChannelNums;
u8	  		 								X_A			protocol_type;
/****************************************************************************************************************************/


/****************************************************************************************************************************
***************************************************used for apdu_native******************************************************
*****************************************************************************************************************************/
#if HAL__PLATFORM__SIMULATOR
PX8		 									I_A 		bRamBuffer;
PX8		 									I_A 		APDUBuffer;
PX16										I_A	 		bShortBuffer;
//PX8											X_A			bTempArr;
PX8											X_A			cacheBuffer;
#endif
/****************************************************************************************************************************/


/****************************************************************************************************************************
***************************************************used for HMAC********************************************************
*****************************************************************************************************************************/
#if VM_WITH_HMAC
	u8 										X_A			g_HmacDigestSize;
	u8 										X_A			g_HmacBlockSize;
	u8 										X_A			g_HashType;
	u8 										X_A			g_HmackKeyLen;
	PR8										X_A			g_HmacKey;
	PR8										X_A			g_HmacComputeKey;
#endif
/****************************************************************************************************************************/

/****************************************************************************************************************************/
#if VM_WITH_DH
  DHKeyRefStore g_DHKeyStore;
	const u8 DHdefaultG[1] = {0x02};
#endif
/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for timer interrupt**************************************************
*****************************************************************************************************************************/
u8 											X_A			InterruptRegisterPro[13];
/****************************************************************************************************************************/

//u8											X_A			isVGPconfig;
//u8											X_A			isGPconfig221; 目前在C层未使用
/****************************************************************************************************************************
***************************************************used for pboc*************************************************************
*****************************************************************************************************************************/
//B_A			isPBOCselected;		//如果不定义在data区要在复位的时候初始化为0
//u8											XDATA_AREA		g_AppType;
/****************************************************************************************************************************/




//#pragma userclass (HCONST=ROM)
/****************************************************************************************************************************
***************************************************used for timer opfunction*************************************************
*****************************************************************************************************************************/
u8 											CONST 			rData1[3] 				= { 0x9F, 0x6E, 0x06 };
u8											CONST			rData2[4]				= { 0x9F, 0x65, 0x01, 0xff };
//u8											CONST			defaultKey[16]			    = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f };
u8											CONST			defaultKey_32Bytes[32]	= { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f };
u8											CONST			cardIndentify[16]		= {0x4A,0x43,0x45,0x53,0x56,0x32,0x30,0x5F,0x44,0x55,0x30,0x4D,0x31,0x45,0x33,0x36};
/*@ -disspecopt @*/
/*@ -type @*/
u8											CONST 			defaultProtocolConfig	= DEFAULT_WARM_TCK_ABSENT | DEFAULT_COLD_TCK_ABSENT | DEFAULT_DERECT_CONVENTION | DEFAULT_T0 | DEFAULT_NO_SLEEP_MODE | DEFAULT_PPS;
/*@ -type @*/
/*@ +disspecopt @*/

/****************************************************************************************************************************
***************************************************used for DataManager*************************************************************
*****************************************************************************************************************************/
u8	 										X_A				handle_2F00Tag[MAX_LOGICAL_CHANNELS] ;//dataManager handle 2F00 linked.
u8	 										X_A				handle_2F00TagLink[MAX_LOGICAL_CHANNELS];
u8 											X_A				g_moreCommands;


/****************************************************************************************************************************
***************************************************used for RoomManager*************************************************************
*****************************************************************************************************************************/
u8  	 									X_A							g_bCheckFlag;
u8     										X_A							g_bRoomChanged;
//u8		  									X_A							g_deletPkgAppFlag;
u8	  										X_A							g_systemConsumeConfig;
//u16	 										X_A						  	g_backupCC;
u16											X_A							g_CCInLoadOrInst;
u32    										X_A							g_sC7Value;
u32    										X_A							g_sC8Value;
u32    										X_A							g_sD7Value;
u32 	 		 							X_A							g_sD8Value;
u32    										X_A							g_UsedRAM;
u32    										X_A							g_UsedNVM;
u32    										X_A							g_OldUsedRAM;
u32    										X_A							g_OldUsedNVM;
u32   										X_A							cacheC7;
u32   										X_A							cacheC8;
u32   										X_A							cacheD7;
u32   										X_A							cacheD8;
u32   										X_A							cache82;
u32   										X_A							cache83;
memref 										X_A							g_appEntryAddr;
//u16											X_A							g_systemUsedRAM;
//u32    										X_A							g_systemUsedNVM;
/****************************************************************************************************************************
***************************************************used for SCP*************************************************************
*****************************************************************************************************************************/

u8 											X_A			g_SCP_Type[MAX_LOGICAL_CHANNELS];
u8 											X_A			g_SCP_I_Option[MAX_LOGICAL_CHANNELS];

u8 											X_A			g_SCP_rmacTemp[RMAC_TEMP_LENGTH];
u16 										X_A			g_SCP_seqCounter[MAX_LOGICAL_CHANNELS];
#if GP__AMENDMENT__D // modify by xuxiaojun@20220602 g_current_SCP_SES_KEY对应的长度, SCP03协议中会话密钥长度可能为0x10,0x18,0x20。 测试用例704 GP UICC2 Compliance203：sm_APDU_initializeUpdate_success_secureChannelSessionAlreadyOpened_otherLc_SSD_scp03_24B_de_3b_58
u8 		X_A 					SCP_sesKeyLen[MAX_LOGICAL_CHANNELS];  // g_current_SCP_SES_KEY对应的长度, SCP03协议中会话密钥长度可能为0x10,0x18,0x20。 测试用例704 GP UICC2 Compliance203：sm_APDU_initializeUpdate_success_secureChannelSessionAlreadyOpened_otherLc_SSD_scp03_24B_de_3b_58
u8 		X_A 					SCP_rencKeyLen[MAX_LOGICAL_CHANNELS];
u8 		X_A 					SCP_dekKeyLen[MAX_LOGICAL_CHANNELS];  // g_current_SCP_DEK_KEY对应的长度
u8 		X_A 					SCP_rdekKeyLen[MAX_LOGICAL_CHANNELS];
u8 		X_A 					SCP_macKeyLen[MAX_LOGICAL_CHANNELS];  // g_current_SCP_MAC_KEY对应的长度
u8 		X_A 					SCP_rmacKeyLen[MAX_LOGICAL_CHANNELS]; // g_current_SCP_RMAC_KEY对应的长度,目前来看，g_SCP_rmacKeyLen的值与g_SCP_macKeyLen的值一致
//#define 		g_SCP_sesKeyLen         SCP_sesKeyLen[selected_channel] // g_current_SCP_SES_KEY对应的长度			//在global_variable.h中已定义
//#define			g_SCP_rencKeyLen        SCP_rencKeyLen[selected_channel]										//在global_variable.h中已定义
//#define			g_SCP_dekKeyLen         SCP_dekKeyLen[selected_channel]   // g_current_SCP_DEK_KEY对应的长度		//在global_variable.h中已定义
//#define 		g_SCP_rdekKeyLen        SCP_rdekKeyLen[selected_channel]											//在global_variable.h中已定义
//#define 		g_SCP_macKeyLen         SCP_macKeyLen[selected_channel]// g_current_SCP_MAC_KEY对应的长度			//在global_variable.h中已定义
//#define 		g_SCP_rmacKeyLen        SCP_rmacKeyLen[MAX_LOGICAL_CHANNELS]// g_current_SCP_RMAC_KEY对应的长度,目前来看，g_SCP_rmacKeyLen的值与g_SCP_macKeyLen的值一致	//在global_variable.h中已定义
#else
u8 											X_A 		g_SCP_sesKeyLen; // g_current_SCP_SES_KEY对应的长度
u8 											X_A 		g_SCP_rencKeyLen;
u8 											X_A 		g_SCP_dekKeyLen; // g_current_SCP_DEK_KEY对应的长度
u8 											X_A 		g_SCP_rdekKeyLen;
u8 											X_A 		g_SCP_macKeyLen; // g_current_SCP_MAC_KEY对应的长度
u8 											X_A 		g_SCP_rmacKeyLen; // g_current_SCP_RMAC_KEY对应的长度,目前来看，g_SCP_rmacKeyLen的值与g_SCP_macKeyLen的值一致
#endif
u8                                          X_A         g_SCP_SCP03t_Flag;//标识是否是scp03t ，是的话在8482里面需要将数据源转成TLV格式

u8 											X_A			g_SCP_BeginEnd_R_MAC[MAX_LOGICAL_CHANNELS];
u16 										X_A 		g_SCP_SequnceCount[MAX_LOGICAL_CHANNELS];    // only used for begin rmac command

#if GP__SCP__SCP10__FOR_MASK
u8											X_A			sesKeyFlag;
#endif

#if GP__AMENDMENT__D
u8 											X_A			g_SCP_ICV_Counter[SCP03_ICV_COUNTER_LENGTH];//顺序计数器，用于计算（C-decrypt/R-decrypt）ICV
#endif

u8 											X_A 		g_SCP_appEnc;
u8 											X_A 		g_SCP_appREnc;
u8 											X_A 		g_SCP_appRmac;

/****************************************************************************************************************************
***************************************************used for CVM*************************************************************
*****************************************************************************************************************************/

memref 										X_A			CvmDataAddress;




/****************************************************************************************************************************/

/****************************************************************************************************************************
***************************************************used for timer loader*****************************************************
*****************************************************************************************************************************/
#if !BOOTLOADER_PRO
ComponentEntry								CONST 		 	f_linkers[13] 			=  {
	    {headerComponentInit,headerComponentProcess,postProcess},
	    // heard component
	    {headerComponentInit,headerComponentProcess,postProcess},
	    // Directory component
	    {DirectoryComponentInit,DirectoryComponentProcess,postProcess},
	    // Applet Component
	    {AppletComponentInit, AppletComponentProcess,postProcess},
	    // import Component
	    {ImportComponentInit, ImportComponentProcess,postProcess},
	    //Constant Component
	    {ConstantComponentInit, ConstantComponentProcess, ConstantComponentPostProcess},
	    // class component
	    {ClassComponentInit,ClassComponentProcess, ClassComponentpostProcess},
	    // method component
	    {componentInit,MethodComponentProcess, MethodComponentpostProcess},
	    // StaticField Component
	    {StaticFieldComponentInit, StaticFieldComponentprocess,postProcess},
        // ReferenceLocation Component
	    {ReferenceLocationComponentInit, ReferenceLocationComponentprocess, postProcess},
	    // export Component
	    {componentInit,ExportComponentProcess,ExportComponentpostProcess},
	    {componentInit,postProcess,postProcess	},
	    {DebugComponentInit,DebugComponentProcess,DebugComponentpostProcess	}
       };
       // skip descriptor component
#endif

	u8											CONST			bLoadOrderArr[13]		=
																							{	(u8) 0,
																								ORDER_HEADER,
																								ORDER_DIRECTORY,
																								ORDER_APPLET,
																								ORDER_IMPORT,
																								ORDER_CONSTANTPOOL,
																								ORDER_CLASS,
																								ORDER_METHOD,
																								ORDER_STATICFIELD,
																								ORDER_REFERENCELOCATION,
																								ORDER_EXPORT,
																								(u8) 11,
																								ORDER_DEBUG
																							};
																							
  u16 const WTXT0[1] = {0x21C0};
  u8  const FEATUREBLOCKBYTES[5] = {0x00,0x00,0x00,0x00,0x00};
//  u8  EFCFG_FLAG = 1;
/****************************************************************************************************************************/
