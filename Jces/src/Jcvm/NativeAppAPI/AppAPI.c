#include "AppAPI.h"
#include "pbocAPI.h"
#include "Apdu.h"
#include "MemOperation.h"
#include "memory.h"
#include "BootLifeCycle.h"
#include "boot.h"
#include "ob_manager.h"
#include "cardManager.h"
#include "contentManager.h"
#include "bc_object.h"
#include "packageloc.h"
#include "reference.h"
#include "JcvmRam.h"
#include "jcvmConfigMethod.h"


#if JCVM__CONFIG_FILE

	#define FILE_ID_SYS							0

	static u16 AppAPI_CheckConigFile(u8 fileType)
	{
		u16 result;
		
		if (SECURITY_STATE_ATTACK == boot_GetSecurityState()) 
		{
			result	= BOOT_SECURITY_STATUS_NOT_SATISFIED;
		}
		else if(LIFE_CYCLE_INIT > BootLifeCycle_GetLifeCycle())
		{
			result	= BOOT_SECURITY_STATUS_NOT_SATISFIED;
		}
		else
		{
			switch(fileType)
			{
				case FILE_ID_SYS:
					result		= BOOT_NO_ERROR;
					break;
				default:
					result	= BOOT_REFERENCED_DATA_NOT_FOUND;
					break;
			}
		}
		
		return result;
	}

	static u16 AppAPI_GetConfigFileSize(u8 fileType)
	{
		u16 fileSize;
		
		switch(fileType)
		{
			case FILE_ID_SYS:
				fileSize	= JcvmConfig_GetFileSysIdMapSize();
				break;
			default:
				fileSize	= 0;
				break;
		}
		
		return fileSize;
	}

	static void AppAPI_SetConfigFileRef(u8 fileType, jref fileRef)
	{
		PN8	refAddress;
		
		switch(fileType)
		{
			case FILE_ID_SYS:
				refAddress	= MEMtPN8(JcvmConfig_GetFileSysIdMapBase());
				break;
			default:
				refAddress	= MEMtPN8(MEMREF_NULL);
				break;
		}
		nvmWriteU16Atomic(refAddress, fileRef);
	}

	static memref AppAPI_GetConfigFileStart(u8 fileType)
	{
		jref 	fileSystemRef;
		memref 	addr;
		
		switch(fileType)
		{
			case FILE_ID_SYS:
				addr	= MEMtPN8(JcvmConfig_GetFileSysIdMapBase());
				break;
			default:
				addr	= REF_NULL;
				break;
		}
		
		fileSystemRef	= PN_R_U16(addr);
		
		if(REF_NULL == fileSystemRef)
		{
			addr		= U8_A_U32(SIZE_REF, addr);
		}
		else
		{
			addr		= locateObjectInMemory(fileSystemRef);
		}
		
		return addr;
	}

	u16 AppAPI_ClearConfigFile(u16 sw)    SMALL_MODE
	{
		u16		D_A result;
		
		if(SW_NO_ERROR != sw)
		{
			result	= sw;
		}
		else
		{
			AppAPI_ClearFileIdSys();
			result	= SW_NO_ERROR;
		}
		
		return result;
	}

u16 AppAPI_WriteConfigFile(PX8 apduBuffer)      SMALL_MODE
	{
		u8	  D_A	fileType;
		jref  D_A	configFileRef;
		u16   D_A	totalLength;
		PN8	  D_A	fileSystemStart;
		u16	  D_A	offset;
		u16	  D_A	result;
		
		apdu_ApduSetIncomingAndReceive();
		
		fileType	= apduBuffer[APDU_P1_OFFSET];
		result		= AppAPI_CheckConigFile(fileType);
		
		if(BOOT_NO_ERROR == result)
		{
			switch (apduBuffer[APDU_P2_OFFSET])
			{
				case 0x00: //start loading

					totalLength = PXB_R_U16(&apduBuffer[APDU_DATA_OFFSET]);

					if(totalLength <= U16_S_U16(AppAPI_GetConfigFileSize(fileType), REF_NULL))
					{
						configFileRef	= REF_NULL;
					}
					else
					{
						configFileRef 	= NvmManager_Allocate(U16t32(totalLength), MAP_ENTRY_BLOCK_PCK_ID);
					}
					
					AppAPI_SetConfigFileRef(fileType, configFileRef);
					
					result		= BOOT_NO_ERROR;
					break;
				case 0x01: //downloading
					fileSystemStart		= MEMtPN8(AppAPI_GetConfigFileStart(fileType));
					offset				= PXB_R_U16(&apduBuffer[APDU_DATA_OFFSET]);

					nvmCopyNoAtomic(MEMtPN8(U16_A_U32(offset,fileSystemStart)), MEMtPM8(&apduBuffer[U8_A_U8(APDU_DATA_OFFSET , 2)]), U8t16(U8_S_U8(apduBuffer[APDU_P3_OFFSET] , 2)));
				
					result		= BOOT_NO_ERROR;
					break;
				default:
					result 		= BOOT_CONDITIONS_NOT_SATISFIED;
					break;
			}
		}
		return result;
	}
#endif
	
#if JCVM__IO__SWP_SELECT_BY_FILE_ID

	#pragma 	JCVM_APP_API_START
	static  u16 		g_CurrentDF;
	JCVM_APP_API_END

	#define		FILE_ID_SYS_MAP_NUM_OFFSET			0
	#define		FILE_ID_SYS_MAP_FIRST_REC_OFFSET	U8_A_U8(FILE_ID_SYS_MAP_NUM_OFFSET , 1)

	#define		FILE_TYPE_OFFSET					0
	#define		FILE_DF_ID_OFFSET					U8_A_U8(FILE_TYPE_OFFSET , 1)
	#define		FILE_ID_OFFSET						U8_A_U8(FILE_DF_ID_OFFSET , 2)
	#define		FILE_AID_LENGTH_OFFSET				U8_A_U8(FILE_ID_OFFSET , 2)
	#define		FILE_AID_OFFSET						U8_A_U8(FILE_AID_LENGTH_OFFSET , 1)

	#define		FILE_HEADER_LENGTH					FILE_AID_OFFSET

	#define 	JDFTYPE 							0x03
	#define 	JEFTYPE 							0x02
	#define		NOT_FIND							0xA0
	#define 	NOT_SUPPROT							0xA1
	#define 	MATCH 								0xA2
	#define 	MATCHDF 							0xA3

	#define		AppAPI_GetFileSysIdNum(addr)						PN_R_U8(PN8_A_U8(addr,  FILE_ID_SYS_MAP_NUM_OFFSET))
	#define		AppAPI_GetFileSysFirstRecAddr(addr)					PN8_A_U8(addr,  FILE_ID_SYS_MAP_FIRST_REC_OFFSET)
	#define		AppAPI_GetFileType(addr)							PN_R_U8(PN8_A_U8(addr,  FILE_TYPE_OFFSET))
	#define		AppAPI_GetDfFileId(addr)							PN_R_U16(PN8_A_U8(addr, FILE_DF_ID_OFFSET))
	#define		AppAPI_GetFileId(addr)								PN_R_U16(PN8_A_U8(addr, FILE_ID_OFFSET))
	#define		AppAPI_GetFileAidLength(addr)						PN_R_U8(PN8_A_U8(addr,  FILE_AID_LENGTH_OFFSET))
	#define		AppAPI_GetFileAid(addr)								PN8_A_U8(addr, FILE_AID_OFFSET)

	void AppAPI_initFileIdSys(void)
	{
		g_CurrentDF		= 0x3F00;
	}

	void AppAPI_ClearFileIdSys(void)
	{
		nvmFillAtomic(MEMtPN8(JcvmConfig_GetFileSysIdMapBase()), 0, JcvmConfig_GetFileSysIdMapSize());
	}

	u8 AppAPI_SelectByFileId(PX8 apduBuffer, PX8 errorCode)
	{
		u8  	bMaxRecNo;
		u8		i;
		u8		bAidLen;
		u8		bFileType;
		u8		result;
		u16 	bFileId;
		u16 	bDFid;
		u16 	CurrentSeleteFileId;
		PN8		curFileAddr;
		
		if(apduBuffer[APDU_P1_OFFSET] != 00 || apduBuffer[APDU_P2_OFFSET] != 00 || apduBuffer[APDU_P3_OFFSET] != 0x02)
		{
			result	= NOT_SUPPROT;
		}
		else
		{
			bFileId 				= 0;
			bAidLen					= 0;
			result					= NOT_FIND;
			curFileAddr				= MEMtPN8(AppAPI_GetConfigFileStart(FILE_ID_SYS));
			bMaxRecNo 				= AppAPI_GetFileSysIdNum(curFileAddr);
			curFileAddr				= AppAPI_GetFileSysFirstRecAddr(curFileAddr);

			CurrentSeleteFileId 	= PXB_R_U16(&apduBuffer[APDU_DATA_OFFSET]);
			
			for(i = 0 ; i< bMaxRecNo;U8_INC(i))
			{
				bFileType				= AppAPI_GetFileType(curFileAddr);
				bDFid 					= AppAPI_GetDfFileId(curFileAddr);
				bFileId					= AppAPI_GetFileId(curFileAddr);
				bAidLen 		    	= AppAPI_GetFileAidLength(curFileAddr);
				if(bFileType == JEFTYPE && (bFileId == CurrentSeleteFileId))
				{
					if(g_CurrentDF == bDFid)
					{
						ecpMem_Copy(MEMtPX8(&apduBuffer[APDU_DATA_OFFSET]), AppAPI_GetFileAid(curFileAddr), bAidLen);
						apduBuffer[2] 		= 0x04;
						apduBuffer[4] 		= bAidLen;
						result				= MATCH;
						break;
					}

				}
				else if(bFileType == JDFTYPE ||(g_CurrentDF == 0x00)||(g_CurrentDF == 0x3F00))
				{
					if(CurrentSeleteFileId == bFileId)
					{
						g_CurrentDF			= bFileId; 
						if(bAidLen == 0x00)
						{
							result			= MATCHDF;
							break;
						}
						else
						{
							ecpMem_Copy(MEMtPX8(&apduBuffer[5]), AppAPI_GetFileAid(curFileAddr), bAidLen);
							apduBuffer[2] 	= 0x04;
							apduBuffer[4] 	= bAidLen;
							result			= MATCH;
							break;								 
						}
					}
				}
				curFileAddr					= PN8_A_U8(curFileAddr, U8_A_U8(bAidLen, FILE_HEADER_LENGTH));
			}
		}
		
		if (result == MATCHDF)
		{
			PXB_W_U16(errorCode, SW_NO_ERROR);
			result	= true;
		}
		else
		{
			result	= false;
		}
		
		return result;
	}
#endif

#if JCVM__APP_CONFIG

	#define NO_APP_CONFIG					0
	#define MOST_APP_MOD					4

	#define APP_MOD1_AID_NEED_CHANGE		0x01
	#define APP_MOD2_AID_NEED_CHANGE		0x02
	#define APP_MOD3_AID_NEED_CHANGE		0x04
	#define APP_MOD4_AID_NEED_CHANGE		0x08
	#define	APP_MOD_AID_NOT_NEED_CHANGE		0x00
	#define APP_PKG_AID_NEED_CHANGE			0x10
	#define APP_PKG_SD_NEED_CHANGE			0x20
	#define APP_APP_MOV_TO_OTHER_PKG        0x80

	#define APP_MOD_AID_CHANGE_MASK			(APP_MOD1_AID_NEED_CHANGE | APP_MOD2_AID_NEED_CHANGE | APP_MOD3_AID_NEED_CHANGE | APP_MOD4_AID_NEED_CHANGE )

	static u8 searchPkgID(PF8 aid, u16 offset, u16 length )    SMALL_MODE
	{
		u8		D_A		pkgID;
		jref	D_A		pkgAID;
		jref	D_A     pkgEntry;
		//PF8		D_A		EntryAddr;
		for (pkgID = 0; pkgID < ON_CARD_PKG_MAX; U8_INC(pkgID))
		{
			pkgEntry	= getPkgEntry(pkgID);
			if (pkgEntry != REF_NULL)
			{
				//EntryAddr 	= MEMtPF8(locateObjectInMemory(pkgEntry));
				pkgAID		= getPkgAID(pkgEntry);
				if (AID_equals_array(pkgAID, aid, offset, length))
				{
					return pkgID;
				}
			}
		}
		return ILLEGAL_ID;
	}
	
	static void AppAPI_ChangeAppModAID(PC8 oldAppInfArrAddr, PN8 newAppInfArrAddr, u8 appAidLength, PN8 appModAidBase, u8 token)
	{
		PM8			I_A			oldAppInfAddr;
		PM8			I_A			newAppInfAddr;

		jref		I_A			oldAppInfRef;
		jref		I_A			newAppInfRef;

		jref		I_A			newModAIDRef;
		jref		I_A			AppInstallMethodAddr;

		oldAppInfRef			= PC_R_U16(PC8_A_U16(oldAppInfArrAddr, U8_M_U8(token, SIZE_FIELD)));
		oldAppInfAddr			= MEMtPN8(locateObjectInMemory(oldAppInfRef));

		newAppInfRef			= newObject(AppletInfo_CLASS_OFFSET, IMPL_PKG_ID, BIT_NEED_REFRESH);
		newAppInfAddr			= MEMtPN8(locateObjectInMemory(newAppInfRef));

		newModAIDRef = initAID(appModAidBase, 0x00, appAidLength);

		nvmWriteU16NoAtomic(MEMtPN8(get_object_field_ref(newAppInfAddr, AppletInfoClassAID_INDEX)), newModAIDRef);
		AppInstallMethodAddr	= nvmReadU16(MEMtPN8(get_object_field_ref(oldAppInfAddr, AppletInfoInstallMethodAddr_INDEX)));
		nvmWriteU16NoAtomic(MEMtPN8(get_object_field_ref(newAppInfAddr, AppletInfoInstallMethodAddr_INDEX)), AppInstallMethodAddr);

		nvmWriteU16NoAtomic(PN8_A_U16(newAppInfArrAddr, U8_M_U8(token, SIZE_FIELD)), newAppInfRef);
		return;
	}
	
	
	
	/*
	*将指定的包的PackageEntry对象内容拷贝新new出PackageEntry中，
	*并将pkg_table中的内容与新的PackageEntry关联
	*input: package id
	*out:   newPkgEntryAddr
	*/
	static PM8 AppAPI_CopyPkgEntry(u8 pkgId )
	{
		PM8		X_A		oldPkgEntryAddr;
		PM8		X_A		newPkgEntryAddr;
		jref	X_A		newPkgEntryRef;
		jref	X_A		oldPkgEntryRef;
		u8		X_A		pkgEntrySize ;
		
		pkgEntrySize		= get_class_size(resolveReferenceAddress(PackageEntry_CLASS_OFFSET, IMPL_PKG_ID, COMPONENT_CLASS));
		pkgEntrySize        = U8_M_U16(SIZE_SHORT, pkgEntrySize);
		
		oldPkgEntryRef		= getPkgEntry(pkgId);
		oldPkgEntryAddr		= MEMtPM8(locateObjectInMemory(oldPkgEntryRef));
		oldPkgEntryAddr		= get_object_field_ref(oldPkgEntryAddr, 0);

		//create a new package entry and copy the content of old package entry
		newPkgEntryRef		= newObject(PackageEntry_CLASS_OFFSET, IMPL_PKG_ID, BIT_NEED_NOTHING);
		newPkgEntryAddr		= MEMtPN8(locateObjectInMemory(newPkgEntryRef));
		newPkgEntryAddr		= get_object_field_ref(newPkgEntryAddr, 0);

		nvmCopyNoAtomic(newPkgEntryAddr, oldPkgEntryAddr, pkgEntrySize);
		nvmWriteU16NoAtomic(PN8_A_U16(MEMtPM8(Pkg_Tabel_Cash), U8_M_U8(pkgId, SIZE_REF)) , newPkgEntryRef);
		
		memRefresh();
		return newPkgEntryAddr ;
	}
	
	
	/*
	*@ desc：move applet form one package to another package 
	*@ input: 
	*	   	newSrcPEAddr          a new src package entry object addr
	*      	oldSrcAppInfArryAddr  
	*      	oldSrcAppInfArrNum
	*      	destPkgAidAddr        dest package aid
	*      	length                dest package aid length
	*/
	static u16 AppAPI_MoveAppletToOtherPkg(PM8 newSrcPEAddr, PM8 oldSrcAppInfArryAddr,u8 oldSrcAppInfArrNum, PM8 destPkgAidAddr, u8 length, u8 token, u32 pkgLoctionAddr)
	{
		//PM8     X_A   oldAppInfArrAddr;
		PN8     X_A	  newAppInfArrAddr;
		PN8     X_A	  newSrcAppInfArrAddr;
		PN8     X_A   destPkgEntryAddr ;
		
		jref    X_A	  movAppInfRef;
		jref    X_A	  oldAppInfArrRef;
		jref    X_A	  newAppInfArrRef;
		jref    X_A	  newSrcAppInfArrRef;

		u8      X_A   pkgId;
		u8      X_A   offset ;
		u8      X_A   AppInfArrNum ;
		u8      X_A   AppInfArrSize;
		
		//find packages
		pkgId = searchPkgID(destPkgAidAddr, 0, length);
		if(ILLEGAL_ID == pkgId)
		{
			return SW_REFERENCED_DATA_NOT_FOUND;
		}
		destPkgEntryAddr = AppAPI_CopyPkgEntry(pkgId) ;
		
		//create dest appletInfo array, and move applet to new appletInfo[]
		oldAppInfArrRef 	= nvmReadU16(PN8_A_U8(destPkgEntryAddr, U8_M_U8(SIZE_FIELD , applets_INDEX)));
		if(REF_NULL != oldAppInfArrRef)
		{
			return SW_WRONG_DATA ;
		}
		AppInfArrNum       = 1 ;
		
		AppInfArrSize			= U8_M_U16(SIZE_REF, AppInfArrNum ) + SIZE_OBJECT_ARRAY_HEADER;
		newAppInfArrRef			= NvmManager_Allocate(AppInfArrSize, MAP_ENTRY_OBJECT_PCK_ID);
		newAppInfArrAddr		= MEMtPN8(MapTable_GetEntryAddr(newAppInfArrRef));
		
		nvmCopyAtomic(newAppInfArrAddr, oldSrcAppInfArryAddr, SIZE_OBJECT_ARRAY_HEADER );        //copy appletInf[] header
		nvmWriteU16NoAtomic( PN8_A_U8(newAppInfArrAddr, 6), AppInfArrNum);             			//change ref array length
		newAppInfArrAddr		= newAppInfArrAddr + SIZE_OBJECT_ARRAY_HEADER;
		//oldAppInfArrAddr		= newAppInfArrAddr + SIZE_OBJECT_ARRAY_HEADER;
		
		/*get need move applet reference*/
		movAppInfRef			= PC_R_U16(PC8_A_U16(PM8_A_U8(oldSrcAppInfArryAddr,SIZE_OBJECT_ARRAY_HEADER), U8_M_U8( (token-1), SIZE_FIELD)));
		
		nvmWriteU16NoAtomic(newAppInfArrAddr, movAppInfRef );                          //add a new applet
		memRefresh();

		
		/*delete move's applet  form AppletInfo[] */
		AppInfArrSize		= U8_M_U16(SIZE_REF, (oldSrcAppInfArrNum-1)) + SIZE_OBJECT_ARRAY_HEADER;
		newSrcAppInfArrRef	= NvmManager_Allocate(AppInfArrSize, MAP_ENTRY_OBJECT_PCK_ID);
		newSrcAppInfArrAddr	= MEMtPN8(MapTable_GetEntryAddr(newSrcAppInfArrRef));

		offset =   U8_A_U8( U8_M_U8(U8_S_U8(token, 1), SIZE_FIELD), SIZE_OBJECT_ARRAY_HEADER);
		nvmCopyAtomic(newSrcAppInfArrAddr, oldSrcAppInfArryAddr, offset );  
		nvmWriteU16NoAtomic( PN8_A_U8(newSrcAppInfArrAddr, 6), U8_S_U8(oldSrcAppInfArrNum, 1) );            //change length
		
		newSrcAppInfArrAddr  = PM8_A_U8(newSrcAppInfArrAddr , offset );
		oldSrcAppInfArryAddr = PM8_A_U8(oldSrcAppInfArryAddr , U8_A_U8(offset, 2) );
		
		nvmCopyAtomic(newSrcAppInfArrAddr, oldSrcAppInfArryAddr, U8_M_U8( U8_S_U8(oldSrcAppInfArrNum, token), SIZE_FIELD) );    		

		
		//build new relation
		nvmWriteU16NoAtomic(PN8_A_U16(newSrcPEAddr, U8_M_U8(SIZE_FIELD , applets_INDEX)) , newSrcAppInfArrRef );
		nvmWriteU16NoAtomic(PN8_A_U16(destPkgEntryAddr, U8_M_U8(SIZE_FIELD , applets_INDEX)) , newAppInfArrRef );
		
		addAppletPackage(pkgId);       //set nullpkg is appletpackage;
		nvmWriteU32Atomic(PN8_A_U16(MEMtPN8(G_CurExecuEnvironment.pkgLocationTable), U8_M_U8(pkgId, SIZE_MEMREF)), pkgLoctionAddr);
		memRefresh();
		return 0x9000;
	}
	
	u16 AppAPI_ConfigApp(PX8 apduBuffer)    SMALL_MODE
	{
		u8			D_A			appConfig;
		u8			D_A			pkgAidLength;
		u8			D_A			i;
		u8          D_A         dataCount;
		u8			D_A			appModAidLength;
		u32    	    D_A     	pkgLoctionAddr ;
		
		PX8			D_A			appConfigMapCurAddr;
		
		PC8 		D_A			oldPkgEntryAddr;
		PN8 		D_A			newPkgEntryAddr;

		PC8 		D_A			oldAppInfArrAddr;
		PN8 		D_A			newAppInfArrAddr;

		jref		D_A			oldAppInfArrRef;
		jref		D_A			newAppInfArrRef;
		
		jref		I_A			newPkgAIDRef;

		u8			I_A			AppInfArrNum;
		u8			I_A			AppInfArrSize;
		u16			I_A			result;

		cc						= JCRE_CONTEXT;

		
		if (SECURITY_STATE_ATTACK == boot_GetSecurityState())
		{
		    result	= BOOT_SECURITY_STATUS_NOT_SATISFIED;
	    }
		else if(LIFE_CYCLE_INIT != BootLifeCycle_GetLifeCycle())
		{
			result	= BOOT_SECURITY_STATUS_NOT_SATISFIED;
		}
		else
		{
			apdu_ApduSetIncomingAndReceive();
			result					= BOOT_NO_ERROR;
			newPkgEntryAddr			= MEMtPN8(0);
			newAppInfArrAddr		= MEMtPN8(0);
			appConfigMapCurAddr		= MEMtPX8(&apduBuffer[APDU_DATA_OFFSET]);
			appConfig				= PX_R_U8(appConfigMapCurAddr);
			dataCount               = 0x01;
			
			while ((NO_APP_CONFIG != appConfig) && (dataCount < apduBuffer[APDU_P3_OFFSET]))
			{
				//find package
				PX8_INC(appConfigMapCurAddr);
				U8_INC(dataCount);
				pkgAidLength			= PX_R_U8(appConfigMapCurAddr);
				PX8_INC(appConfigMapCurAddr);
				i						= searchPkgID(appConfigMapCurAddr, 0, pkgAidLength);
				if(ILLEGAL_ID == i)
				{
					result	= BOOT_REFERENCED_DATA_NOT_FOUND;
					break;
				}
				
				//copy package Entry 
				newPkgEntryAddr = AppAPI_CopyPkgEntry(i) ;
				oldPkgEntryAddr = newPkgEntryAddr ;            

				appConfigMapCurAddr	= PX8_A_U8(appConfigMapCurAddr, pkgAidLength);
				dataCount           = U8_A_U8(dataCount , pkgAidLength);
				
				
				// third get PBOC PKG AID array
				if(APP_MOD_AID_NOT_NEED_CHANGE != (APP_MOD_AID_CHANGE_MASK & appConfig) )
				{
					oldAppInfArrRef 	= nvmReadU16(PN8_A_U8(oldPkgEntryAddr, U8_M_U8(SIZE_FIELD , applets_INDEX)));
					oldAppInfArrAddr	= MEMtPC8(locateObjectInMemory(oldAppInfArrRef));
					AppInfArrNum		= get_array_length(oldAppInfArrAddr);
					AppInfArrSize		= U8_M_U16(SIZE_REF, AppInfArrNum) + SIZE_OBJECT_ARRAY_HEADER;
					
					if( U8_AND_U8(appConfig, 0x80) == 0x80)
					{	
						pkgLoctionAddr = PN_R_U32( PN8_A_U16(MEMtPN8(G_CurExecuEnvironment.pkgLocationTable), U8_M_U8(i, SIZE_MEMREF)) );
						
						i =  appConfig & 0x07 ;
						appModAidLength	= PX_R_U8(appConfigMapCurAddr);
						PX8_INC(appConfigMapCurAddr);
						result = AppAPI_MoveAppletToOtherPkg(newPkgEntryAddr, oldAppInfArrAddr, AppInfArrNum,  appConfigMapCurAddr, appModAidLength, i, pkgLoctionAddr);
						U8_INC(dataCount);
						dataCount = U8_A_U8(dataCount , appModAidLength);
						break;
					}
					else
					{
						newAppInfArrRef		= NvmManager_Allocate(AppInfArrSize, MAP_ENTRY_OBJECT_PCK_ID);
						newAppInfArrAddr	= MEMtPN8(MapTable_GetEntryAddr(newAppInfArrRef));
						nvmCopyAtomic(newAppInfArrAddr, oldAppInfArrAddr, AppInfArrSize);
						nvmWriteU16NoAtomic(PN8_A_U16(newPkgEntryAddr, U8_M_U8(SIZE_FIELD , applets_INDEX)) , newAppInfArrRef );

						oldAppInfArrAddr	= PC8_A_U8(oldAppInfArrAddr, SIZE_OBJECT_ARRAY_HEADER);
						newAppInfArrAddr	= PN8_A_U8(newAppInfArrAddr, SIZE_OBJECT_ARRAY_HEADER);

						for(i = 0; i < MOST_APP_MOD; i++)
						{
							if(0 != U8_AND_U8(appConfig, U8_SHL_U8(1, i)))
							{
								appModAidLength	= PX_R_U8(appConfigMapCurAddr);
								PX8_INC(appConfigMapCurAddr);
								U8_INC(dataCount);
								JcvmConfig_SetOptimizeCardmanager(1);
								AppAPI_ChangeAppModAID(oldAppInfArrAddr, newAppInfArrAddr, appModAidLength, appConfigMapCurAddr, i);
								appConfigMapCurAddr	= PX8_A_U8(appConfigMapCurAddr, appModAidLength);
								dataCount           = U8_A_U8(dataCount , appModAidLength);
							}
						}
					}
				}
				
				if(APP_PKG_AID_NEED_CHANGE == (APP_PKG_AID_NEED_CHANGE & appConfig))
				{
					pkgAidLength		= PX_R_U8(appConfigMapCurAddr);
					PX8_INC(appConfigMapCurAddr);
					U8_INC(dataCount);

					newPkgAIDRef = initAID(appConfigMapCurAddr, 0x00, pkgAidLength);
					nvmWriteU16NoAtomic(PN8_A_U8(newPkgEntryAddr,U8_M_U8(pkgAID_INDEX, SIZE_FIELD)), newPkgAIDRef);
					appConfigMapCurAddr	= PX8_A_U8(appConfigMapCurAddr, pkgAidLength);
					dataCount           = U8_A_U8(dataCount , pkgAidLength);
				}
				
				if(APP_PKG_SD_NEED_CHANGE == (APP_PKG_SD_NEED_CHANGE & appConfig))
				{
					nvmWriteU8NoAtomic(PN8_A_U8(newPkgEntryAddr,U8_M_U8(sdId_INDEX, SIZE_FIELD)), PX_R_U8(appConfigMapCurAddr));
					PX8_INC(appConfigMapCurAddr);
					U8_INC(dataCount);
				}
			
				appConfig				= PX_R_U8(appConfigMapCurAddr);
			}
		}
		return result;
	}
#endif


#if (VM_WITH_PBOC || VM_WITH_EDEP || VM_WITH_JT_FUNC || VM_WITH_EID || VM_WITH_EID2)

void copyBackUpData(PX8 des, PN8 src, u16 length)
{
	PX8 I_A tempDes;
	PN8 I_A backUpDataSrc; 
	PN8 I_A backUpBottom;
	PN8 I_A backUpTop;
	u16 D_A backUpLength;
 	PN8 I_A blockAddress;
	u16 D_A blockOffset;
    u16 D_A tempOffset;
    u16 D_A tempLength;
	u16 D_A tempCopyLength;

	PATCH_BOOT_MEMORY_MEMBACKUPSERVE_010();
	
    ecpMem_Copy(des, src, length);
	if((TF_BACKUP_First == gBackUp_PageNumStart) || (TF_BACK_UP_NONE == gBackUp_PageNumStart))
	{
		return;
	}
	SC_deactiveEDC();
	blockOffset = searchBlocks(src, length, gBackUp_curOffset, READ_RECENT_BACK_UP_DATA);
	if(ILLEGAL_BLOCK_OFFSET == blockOffset)
	{
		SC_activeEDC();	
		return;
	}

	do
	{
		blockAddress    = PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), blockOffset);
		backUpLength    = U16_AND_U16(getBackUpBlockTypeSize(blockAddress), BACK_UP_BLOCK_LENGTH_MASK);
		backUpBottom    = MEMtPN8(getBackUpBlockAddress(blockAddress));
		backUpTop       = MEMtPN8(PN8_A_U16(backUpBottom, backUpLength));
		blockAddress    = MEMtPN8(getBackUpBlockContent(blockAddress));

        if(PNtMEM(src)< PNtMEM(backUpBottom))
        {
            tempDes         = PX8_A_U16(des, U32_S_U32(backUpBottom, src));
            tempLength      = U16_S_U16(length, U32_S_U32(backUpBottom, src));
            backUpDataSrc   = MEMtPN8(blockAddress);
            tempOffset      = 0;
        }
        else
        {
            //tempDes         = src;
        	tempDes			= des;
            tempLength      = U32_S_U32(backUpTop, src);
            tempOffset      = U32_S_U32(src, backUpBottom);
            backUpDataSrc   = PN8_A_U16(blockAddress, tempOffset);
            if(tempLength > length)
            {
            	tempLength	= length;
            }
        }
        
        //if(tempLength > backUpLength)
        if(tempLength> U16_S_U16(backUpLength, tempOffset))
        {
            //tempCopyLength  = backUpLength;
        	tempCopyLength  = U16_S_U16(backUpLength, tempOffset);
        }
        else
        {
            tempCopyLength  = tempLength;
        }
        
        copyBackUpBlockInfo(backUpDataSrc, 0, U16_A_U16(U16_A_U16(blockOffset, tempOffset), BACK_UP_BLOCK_HEADER_SIZE), tempCopyLength, tempDes);

		blockOffset = searchBlocks(src, length, blockOffset, READ_PRE_BACK_UP_DATA);
	}while(ILLEGAL_BLOCK_OFFSET != blockOffset);
	SC_activeEDC();	
}
#endif //(VM_WITH_PBOC || VM_WITH_EDEP || VM_WITH_JT_FUNC || VM_WITH_EID || VM_WITH_EID2)


/*---------------------------------------------------------------------------------------------------
Function name: des

Input parameter: plain、key、mode、dataOut

dataOutput parameter: nothing

Description: endataOut or dedataOut   for usim used

Status: ok
----------------------------------------------------------------------------------------------------*/
#if (VM_WITH_PBOC || VM_WITH_EDEP || VM_WITH_JT_FUNC || VM_WITH_EID || VM_WITH_EID2)
void HARDWARE_DES(PX8 dataIn, PM8 keypointer, PX8 dataOut, u8 modeEnDe)
{
	Sym_LoadKey_Param_t X_A 	myLoadKeyParam;
	Sym_Crypt_Param_t 	X_A 	myCryptParam;
	u32					X_A		symKeyBuffer[DES_EXT_KEY_BYTE_LEN >> 2];
	u8 					X_A		en_deCrypt;
	// initialize values for SYM operation
	PATCH_BOOT_SYM_API_005();
	
	if(modeEnDe)
	{
		en_deCrypt = MODE_ENCRYPT;
	}
	else
	{
		en_deCrypt = MODE_DECRYPT;
	}

	SymKeyInitParamsInit(&myLoadKeyParam, keypointer, symKeyBuffer, LENGTH_DES, en_deCrypt, SENSITIVE_DATA_NOT_MASK, ALG_DES);
	if (ERR_SYM_LOADKEY_OK != Sym_DES_LoadKey(&myLoadKeyParam))
	{
		faultDetected();
	}

	SymCryptParamsInit(&myCryptParam, NULL, symKeyBuffer, U8t16(myLoadKeyParam.keyByteLen), dataIn, dataOut, en_deCrypt);
	myCryptParam.noOfBlocks = 1;

	if (ERR_SYM_CRYPT_OK != Sym_DES_Crypt(&myCryptParam))
	{
		faultDetected();
	}
	Sym_Wipekey();
}
#endif //(VM_WITH_PBOC || VM_WITH_EDEP || VM_WITH_JT_FUNC || VM_WITH_EID || VM_WITH_EID2)


	
