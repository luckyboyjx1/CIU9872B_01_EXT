/*
 * AbroadLteOp.c
 *
 *  Created on: 2015年8月19日
 *      Author: linguoming
 */

#include "profile.h"
#include "DataOperation.h"
#include "cardManager.h"
#include "loader.h"
#include "opFunction.h"
#include "mask.h"
#include "SCPInterface.h"
#include "stack.h"
#include "global_variable.h"
#include "Apdu.h"
#include "swpDataConfig.h"
#include "KeyLogicManager.h"
#include "opInstaller.h"
#include "object.h"
#include "keyGroup.h"
#include "bc_array.h"
#include "ob_manager.h"
#include "keySet.h"
#include "SecurityDomainData.h"
#include "tranAllocate.h"

#if GP__VERSION__FOR_MINI_MASK

#if GP__SCP
	u16 checkOPCommand(void){
		u8 	bCla_;
		u8 	p1;
		u8 	p2;
		u8 	bIns_;
		u16 errorCode;
		u16 	bPrivRef;
		memref	entry;
		memref 	bPrivArray;
#if GP__KEY_MANAGER__DELETE_KEY
		u8 	result;
#endif

		bCla_		= APDUBuffer[OFFSET_CLA];
		bIns_		= APDUBuffer[OFFSET_INS];
		p1 			= APDUBuffer[OFFSET_P1];
		p2			= APDUBuffer[OFFSET_P2];
		errorCode	= SW_NO_ERROR;

		switch (bIns_) {
			case INS_INITIALIZE_UPDATE:

	//			if (JcvmConfig_GetCardState() == CARD_LOCKED)
	//			{
	//				return SW_FUNC_NOT_SUPPORTED;
	//			}

				if (GP_80 == U8_AND_U8(APDUBuffer[OFFSET_CLA], GP_NO_LOGIC_MASK))
				{
					errorCode = openSecureChannel(APDU_DATAFIELD_IS_NOT_RECEIVED);
					if (errorCode == SW_NO_ERROR
				#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
					|| errorCode == MAX_AUTHENTICATION_WARNING
				#endif
					   )
					{
						apdu_ApduSetOutgoingAndSend(0x05, U8t16(U8_AND_U8(APDUBuffer[OFFSET_LC], 0xFF)));
					}
				}
				else
				{
					return SW_CLA_NOT_SUPPORTED;
				}
				return errorCode;
			case INS_EXTERNAL_AUTHENTICATION:
				if (GP_84 == (U8_AND_U8(APDUBuffer[OFFSET_CLA], GP_NO_LOGIC_MASK))) //mask logical channel number
				{

					errorCode = verifyExternalAuthenticate(APDU_DATAFIELD_IS_NOT_RECEIVED);

					if ((SW_NO_ERROR != errorCode)/* && (errorCode != MAX_AUTHENTICATION_WARNING)*/)
					{
						// if verifyExternalAuthenticate error first close the Security channel.
						closeSecuritychannel(SC_TERMINATED);

						// second update the try count
					#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
						managerSDLifeCycle();
					#endif
					}
					return errorCode;
				}
				else
				{
					return SW_CLA_NOT_SUPPORTED;
				}

			case U32t8(0xE1):
				if(CLEAR_COUNTER_NO_AUTH_TEST) {
					return SW_NO_ERROR;
				}
				break;

			default:
				break;
		}

		switch (bIns_)
		{
			case INS_OP_PIN_CHANGE:
			case INS_OP_SET_STATUS:
			case INS_OP_GET_STATUS:
				return checkIsAuthenticated(DEMAND_AUTHENTICATE);
			case INS_OP_GET_DATA:
			{
				//if ((bRamBuffer[ISD_SECURITY_LEVEL] & 0xff) > SCF_AUTHENTICATED)

				if((U8_AND_U8(channelInfo.security_level[selected_channel], 0x80) == 0x80 && U8_AND_U8(channelInfo.security_level[selected_channel], 0xFF) >= SCF_AUTHENTICATED) ||
								   (U8_AND_U8(channelInfo.security_level[selected_channel], 0x40) == 0x40 && U8_AND_U8(channelInfo.security_level[selected_channel], 0xFF) >= SCF_ANY_AUTHENTICATED))
				{
					errorCode = checkIsAuthenticated(DEMAND_AUTHENTICATE);
					if(errorCode != SW_NO_ERROR)
					{
						return errorCode;
					}
				}
				else if (U8_AND_U8(bCla_, 0x04) == 0x04)
				{
					 return SW_CONDITIONS_NOT_SATISFIED;
				}
				else if (p1==0xff && p2 == 0x1f)
				{
					apdu_ApduSetIncomingAndReceive();

					if (APDUBuffer[OFFSET_LC] == 0)
					{
						return SW_WRONG_LENGTH;
					}
				}

				return errorCode;
			}
		}
		errorCode  = checkIsAuthenticated(DEMAND_AUTHENTICATE);
		if(errorCode != SW_NO_ERROR){
			return errorCode;
		}

		if (JcvmConfig_GetCardState() == CARD_LOCKED) {
			return SW_CONDITIONS_NOT_SATISFIED;
		}

		switch (bIns_) {
			case INS_OP_PUT_KEY:
			case INS_OP_STORE_DATA:
				if (JcvmConfig_GetCardState() == CARD_LOCKED) {
					//pushShort(SW_CONDITIONS_NOT_SATISFIED);
					return SW_CONDITIONS_NOT_SATISFIED;
				}
				if(INS_OP_PUT_KEY == bIns_)
				{
					if (APDUBuffer[OFFSET_CDATA] == KEY_DAP_VERIF_ID1)
					{
						entry		=	getEntryByAppId(getCurrentSDId());
						bPrivRef	=	memReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(entry), bPriv_INDEX)));

						bPrivArray 	= 	get_array_data(MEMtPN8(locateObjectInMemory(bPrivRef)));
						if(U8_AND_U8(PM_R_U8(bPrivArray), DAP_VERIFICATION_PRIVILEGES_TAG) != DAP_VERIFICATION_PRIVILEGES_TAG)
						{
					 		return SW_CONDITIONS_NOT_SATISFIED;
					 	}
					}

					KM_InitPutKeySequence(p1);
				}
				//pushShort(checkIsAuthenticated(DEMAND_AUTHENTICATE));
				return errorCode;
			case INS_OP_LOAD:
				if ( (U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], (U8_INV(U8_SHL_U8(1, selected_channel)))) != 0) // if other logical channel not this one is processing load
					|| (U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], (U8_SHL_U8(1, selected_channel))) == 0) ) // the load process is broken (maybe select command etc.)
				{
					//RAM的load过程也会被同一个SD的其他RAM命令打断
					//if(bRamBuffer[INSTALL_FOR_LOAD_BY_C2] != 0x01)	//command by linguoming
					{
						return SW_CONDITIONS_NOT_SATISFIED;
					}
				}

				if ((p1 != 0) && (p1 != 0x80)) {
					return SW_INCORRECT_P1P2;
				}
				if (bRamBuffer[INSTALLER_STATE_INDEX] == INSTALLER_STATE_ERROR) {
					return SW_CONDITIONS_NOT_SATISFIED;
				}
				if (p2 != bRamBuffer[BLOCK_NUMBER]) {
					return SW_INCORRECT_P1P2;
				}
				return SW_NO_ERROR;
			case INS_OP_INSTALL:
				if (p2 != 0) {
					return SW_INCORRECT_P1P2;
				}
				switch (p1) {
					case P1_OP_INSTALL_FOR_LOAD:

						errorCode = opInstallForLoad();
					  if(errorCode != SW_NO_ERROR)
					  {
						   return  errorCode;
				  	}
				  	bRamBuffer[BLOCK_NUMBER] = 0;
					  return  errorCode;

					case P1_OP_INSTALL_FOR_INSTALL:
					case P1_OP_INSTALL_FOR_INSTALL_AND_MAKE_SELECTABLE:
						return  opInstallForInstall();

					case P1_OP_INSTALL_FOR_EXTRADITION:
					case P1_OP_INSTALL_FOR_MAKE_SELECTABLE:
					case P1_OP_INSTALL_FOR_PERSONALIZE:
						if(P1_OP_INSTALL_FOR_PERSONALIZE == p1)
						{
							bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL] = U8_OR_U8(bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL], U8_SHL_U8(1, selected_channel));
						}
						break;
					default:
						return SW_INCORRECT_P1P2;
				}
				break;
			case INS_OP_DELETE:
				if ((p1 != 0) || ((p2 != 0) & (p2 != 0x80))) {
					return SW_INCORRECT_P1P2;
				}
#if GP__KEY_MANAGER__DELETE_KEY
				if((APDUBuffer[OFFSET_CDATA] == 0xD0) || (APDUBuffer[OFFSET_CDATA] == 0xD2))
				{
					//result = deleteKeySet();
					result	 =	delSetKey();
					if(result == true)
					{
						return SW_NO_ERROR;
					}
					else if (result == 2)
					{
						return SW_CONDITIONS_NOT_SATISFIED;
					}
					else
					{
						return SW_REFERENCED_DATA_NOT_FOUND;
					}
				}
#endif
				break;
			case U32t8(0xE1):
				break;
			default:
				return SW_INS_NOT_SUPPORTED;
		}
		return SW_NO_ERROR;
	}

#else
	u16 checkOPCommand(void){
		u8 	bCla_;
		u8 	p1;
		u8 	p2;
		u8 	bIns_;
		u16 errorCode;

		bCla_		= APDUBuffer[OFFSET_CLA];
		bIns_		= APDUBuffer[OFFSET_INS];
		p1 			= APDUBuffer[OFFSET_P1];
		p2			= APDUBuffer[OFFSET_P2];
		errorCode	= SW_NO_ERROR;

		switch (bIns_) {
			case INS_OP_PUT_KEY:
			case INS_OP_STORE_DATA:
				if (JcvmConfig_GetCardState() == CARD_LOCKED) {
					return SW_CONDITIONS_NOT_SATISFIED;
				}
				if(INS_OP_PUT_KEY == bIns_)
				{
					KM_InitPutKeySequence(p1);
				}
				return checkIsAuthenticated(IGNORE_AUTHENTICATE);
		}

		switch (bIns_)
		{
			case INS_OP_PIN_CHANGE:
			case INS_OP_SET_STATUS:
			case INS_OP_GET_STATUS:
				return checkIsAuthenticated(IGNORE_AUTHENTICATE);
			case INS_OP_GET_DATA:
			{
				if (U8_AND_U8(bRamBuffer[ISD_SECURITY_LEVEL], 0xff) > SCF_AUTHENTICATED)
				{
					errorCode = checkIsAuthenticated(IGNORE_AUTHENTICATE);
				}
				 else if (U8_AND_U8(bCla_, 0x04) == 0x04)
				{
					 return SW_CONDITIONS_NOT_SATISFIED;
				}
				if(errorCode != SW_NO_ERROR)
				{
					return errorCode;
				}

				if (p1==0xff && p2 == 0x1f)
				{
					apdu_ApduSetIncomingAndReceive();

					if (APDUBuffer[OFFSET_LC] == 0)
					{
						return SW_WRONG_LENGTH;
					}
				}

				return errorCode;
			}
		}
		errorCode  = checkIsAuthenticated(IGNORE_AUTHENTICATE);
		if(errorCode != SW_NO_ERROR){
			return errorCode;
		}

		if (JcvmConfig_GetCardState() == CARD_LOCKED) {
			return SW_CONDITIONS_NOT_SATISFIED;
		}

		switch (bIns_) {
			case INS_OP_LOAD:
				if (bRamBuffer[INSTALL_FOR_LOAD] != 1) {
					return SW_CONDITIONS_NOT_SATISFIED;
				}
				if ((p1 != 0) && (p1 != 0x80)) {
					return SW_INCORRECT_P1P2;
				}
				if (bRamBuffer[INSTALLER_STATE_INDEX] == INSTALLER_STATE_ERROR) {
					return SW_CONDITIONS_NOT_SATISFIED;
				}
				if (p2 != bRamBuffer[BLOCK_NUMBER]) {
					return SW_INCORRECT_P1P2;
				}
				return SW_NO_ERROR;
			case INS_OP_INSTALL:
				if (p2 != 0) {
					return SW_INCORRECT_P1P2;
				}
				switch (p1) {
					case P1_OP_INSTALL_FOR_LOAD:
						
						errorCode = opInstallForLoad();
					  if(errorCode != SW_NO_ERROR)
					  {
						   return  errorCode;
				  	}
				  	bRamBuffer[BLOCK_NUMBER] = 0;
					  return  errorCode;
				
					case P1_OP_INSTALL_FOR_INSTALL:
					case P1_OP_INSTALL_FOR_INSTALL_AND_MAKE_SELECTABLE:
						return  opInstallForInstall();
			
					case P1_OP_INSTALL_FOR_EXTRADITION:
					case P1_OP_INSTALL_FOR_MAKE_SELECTABLE:
					case P1_OP_INSTALL_FOR_PERSONALIZE:
						if(P1_OP_INSTALL_FOR_PERSONALIZE == p1)
						{
							bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL] |= 1 << selected_channel;
						}
						break;
					default:
						return SW_INCORRECT_P1P2;
				}
				break;
			case INS_OP_DELETE:
				if ((p1 != 0) || ((p2 != 0) & (p2 != 0x80))) {
					return SW_INCORRECT_P1P2;
				}
				break;
			case (u8)0xE1:
				break;
			default:
				return SW_INS_NOT_SUPPORTED;
		}
		return SW_NO_ERROR;
	}
#endif

#if !GP__DATA_MANAGER__FOR_MASK
	u8 dataManager_getData(PN8 sdDataObj)
	{
		u16	curGetDataTag;
		u8  off;
		u32 iFreeSize_;
#if GP__KEY_MANAGER
		u16  curKeyVersion;
#endif
		PX8	tempBuf;
		u16 sendLength;
		u8  startOff;

#define				GET_DATA_START_OFFSET		5
#define				SEND_MAX_LEN_ONE_DATA		U8_S_U8(255, GET_DATA_START_OFFSET)

		curGetDataTag	= U16_OR_U16(U16_SHL_U8(U8t16(APDUBuffer[APDU_P1_OFFSET]), 8), U8t16(APDUBuffer[APDU_P2_OFFSET]));

		tempBuf			= MEMtPX8(&APDUBuffer[GET_DATA_START_OFFSET]);
		off             = 0;
		startOff        = GET_DATA_START_OFFSET;
		if(CARD_E2E_SPACE == curGetDataTag)
		{
			iFreeSize_ = NvmManager_GetAvailMemory();
			iFreeSize_ = (iFreeSize_ > U16t32(0xFFFF)) ? U16t32(0xFFFF) : iFreeSize_;

			tempBuf[U8_INC(off)] = U32t8(U32_SHR_U8(iFreeSize_, 8));
			tempBuf[U8_INC(off)] = U32t8(iFreeSize_);
			tempBuf[U8_INC(off)]	= getAppletNum();

			sendLength = U8t16(off);
			if (U8_AND_U8(APDUBuffer[APDU_CLA_OFFSET], 0x80) == 0x80)
			{
				startOff        = U8_S_U8(GET_DATA_START_OFFSET, 3);
				PX_W_U16(&APDUBuffer[startOff], curGetDataTag);
				APDUBuffer[U8_A_U8(startOff, 2)] = U16t8(sendLength);
				sendLength = U32t16(U16_A_U8(sendLength, 3));
			}

			apdu_ApduSetOutgoingAndSend(U8t16(startOff), sendLength);
		}
		else if (CARD_RESOURCE_DATA == curGetDataTag)
		{
			tempBuf[U8_INC(off)] = 0x81;  //Number of installed application tag
			tempBuf[U8_INC(off)] = 0x01;

			tempBuf[U8_INC(off)] = getAppletNum();  //install application number

			tempBuf[U8_INC(off)] = 0x82;   //Free Non Volatile Memeory tag
			tempBuf[U8_INC(off)] = 0x04;

			iFreeSize_ = NvmManager_GetAvailMemory();

			tempBuf[U8_INC(off)] = U32t8(U32_SHR_U8(iFreeSize_, 24));
			tempBuf[U8_INC(off)] = U32t8(U32_SHR_U8(iFreeSize_, 16));
			tempBuf[U8_INC(off)] = U32t8(U32_SHR_U8(iFreeSize_, 8));
			tempBuf[U8_INC(off)] = U32t8(iFreeSize_);

			tempBuf[U8_INC(off)] = 0x83;   //Free Volatile Memeory tag
			tempBuf[U8_INC(off)] = 0x02;   //Free Volatile Memeory tag

			iFreeSize_ = tranAllocate_rtrAvailable();

			tempBuf[U8_INC(off)] = U32t8(U32_SHR_U8(iFreeSize_, 8));
			tempBuf[U8_INC(off)] = U32t8(U32_AND_U32(iFreeSize_, 0xFF));

			sendLength = U8t16(off);
			if (U8_AND_U8(APDUBuffer[APDU_CLA_OFFSET], 0x80) == 0x80)
			{
				startOff        = U8_S_U8(GET_DATA_START_OFFSET, 3);
				PX_W_U16(&APDUBuffer[startOff], curGetDataTag);
				APDUBuffer[U8_A_U8(startOff, 2)] = U16t8(sendLength);
				sendLength = U32t16(U16_A_U8(sendLength, 3));
			}

			apdu_ApduSetOutgoingAndSend(U8t16(startOff), sendLength);
		}
		else if (KEY_INFO == curGetDataTag)
		{
#if GP__KEY_MANAGER
			curKeyVersion		= 0xFFFF;
			sendLength			= sdData_getKeyInfo(tempBuf, SEND_MAX_LEN_ONE_DATA, sdDataObj, &curKeyVersion);
#endif
			if (U8_AND_U8(tempBuf[APDU_CLA_OFFSET], 0x80) == 0x00)
			{
				sendLength = U8t16(off);
			}
			else
			{
				APDUBuffer[U8_S_U8(GET_DATA_START_OFFSET, 2)] = 0xE0;
				APDUBuffer[U8_S_U8(GET_DATA_START_OFFSET, 1)] = U16t8(sendLength);
				startOff   = U8_S_U8(GET_DATA_START_OFFSET, 2);
				sendLength = U32t16(U16_A_U8(sendLength, 2));
			}
			apdu_ApduSetOutgoingAndSend(U8t16(startOff), sendLength);
		}
		else if ( (MENU_PARAMETER == curGetDataTag) || (OP_TLV_ADMIN_SESSION_PARA == curGetDataTag) || (OP_TLV_DNS_Resolver == curGetDataTag))
		{
			return 1;
		}
		else
		{
			throw_error(SW_REFERENCED_DATA_NOT_FOUND);
		}

		return 0;
	}

#endif

#endif

#if !GP__SCP
	u16 checkIsAuthenticated(u8 ignoreAuth){
		u16 result;

		result 			= SW_NO_ERROR;

		if (U8_AND_U8(APDUBuffer[OFFSET_CLA], 0x04) == 0x04)
		{
			 return SW_CONDITIONS_NOT_SATISFIED;
		}

		if( bRamBuffer[PROCESS_LEVEL] != ROCESS_LEVEL_OTA )
		{
			apdu_ApduSetIncomingAndReceive();
		}

		return result;
	}
#endif

#if (GP__VERSION__FOR_MINI_MASK && !GP__KEY_MANAGER)
	u16	getKeyGroups(PN8 sdDataObj)
	{
		u16					keyGroupRef;
		PN8 				keyGroupHand;

		keyGroupHand	=	MEMtPN8(get_object_field_ref(sdDataObj, firstKeyGroup_INDEX));
		keyGroupRef		=	PN_R_U16(keyGroupHand);

		if(REF_NULL == keyGroupRef)
		{
			keyGroupRef = newKeyGroup();
			nvmWriteU16Atomic(keyGroupHand, keyGroupRef);
			memRefresh();
		}

		return keyGroupRef;
	}

	void newKeyData(PN8 keySetObj, u16 keyLen)
	{
		u16 			countLen;
		u16				keyDataRef;
		PN8				keyDataHand;

		countLen 	   = U16_M_U16(keyLen, 3); //KEY NUM
		countLen 	   = U16_A_U16(countLen, 1); //KEY LEN

		keyDataHand = MEMtPN8(get_object_field_ref(keySetObj, keyData_INDEX));

		keyDataRef = _newarrayObject(AType_Byte, countLen, BIT_NEED_REFRESH);
		memWriteU16Atomic(keyDataHand, keyDataRef);
		nvmWriteU8Atomic(MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(keyDataRef)))), keyLen);
	}

	u32 getKeyDataField(PN8 keySetObj, u8 keyId, PX8 keyLen)
	{
		memref 			dataField;

		dataField 	= 	keySet_getKeyDataAField(keySetObj);
		PX_W_U8(keyLen, nvmReadU8(MEMtPN8(dataField)));
		dataField   =   U32_A_U32(dataField, 1);//len offest
		dataField   =   U32_A_U32(dataField, U16_M_U16(U8_S_U8(keyId, 1),  PX_R_U8(keyLen))); //value offest

		return  dataField;
	}

	u16 newKeySet(PN8 keySetHand, u8	keyVersion, u16	keyLen)
	{
		u16					keySetRef;
		PN8					keySetObj;

		keySetRef = newObject(KEY_SET_CLASS_OFFSET, EASTCOMPEACE_OPENPLATFORM_ID, BIT_NEED_REFRESH);
		keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));
		newKeyData(keySetObj, keyLen);
		nvmWriteU16Atomic(MEMtPN8(keySetHand), keySetRef);
		keySet_setKeyVersion(keySetObj, keyVersion);
		memRefresh();

		return 	 keySetRef;
	}

	u16	getKeySets(PN8 sdDataObj, u8 keyVersion, u16 keyLen)
	{
		u16					keySetRef;
		PN8 				keySetHand;

		keySetHand			=   MEMtPN8(get_object_field_ref(MEMtPN8(locateObjectInMemory(getKeyGroups(sdDataObj))), keySet_INDEX));
		keySetRef 			= 	nvmReadU16(keySetHand);

		while((keySetRef != REF_NULL) && (keyVersion != keySet_getKeyVersion(locateObjectInMemory(keySetRef))))
		{
			 keySetHand =   MEMtPN8(get_object_field_ref(locateObjectInMemory(keySetRef), nextKeySet_INDEX));
			 keySetRef	=   nvmReadU16(keySetHand);
		}

		if( (keySetRef == REF_NULL) && (keyLen != 0) )
		{
			keySetRef   =  newKeySet(keySetHand, keyVersion, keyLen);
		}

		return keySetRef;
	}

	#define				MAX_KEY_NUM				3
	u16 PutKeys(PN8 sdDataObj, u8 EncryptKey)
	{
		u8					i;
		u8					keyIdStart;
		u8					keyVersion;
		u8					keyLen;
		u16					valueOff;
		u16					keySetRef;
		PN8					keyDataField;
		u8					curKeyVersion;

		keyVersion			=  APDUBuffer[OFFSET_CDATA];
		keyIdStart			=  U8_AND_U8(APDUBuffer[OFFSET_P2], 0x7F);
		keyLen				=  APDUBuffer[U8_A_U8(OFFSET_CDATA, 2)];
		keySetRef			=  getKeySets(sdDataObj, keyVersion, 0);
		valueOff			=  OFFSET_CDATA;
		curKeyVersion		=  U8_AND_U8(APDUBuffer[OFFSET_P1], MAX_KEY_VERSION);

		if (curKeyVersion == 0x00)
		{
			if (keySetRef != REF_NULL)
			{
				throw_error(SW_CONDITIONS_NOT_SATISFIED);
			}
			else
			{
				keySetRef   =  getKeySets(sdDataObj, keyVersion, keyLen);
			}
		}

		for(i = 0; i<MAX_KEY_NUM; U8_INC(i))
		{
			valueOff    		=  U16_A_U16(valueOff, 3); //offest: type + len + kcv len
			keyDataField       	=  MEMtPN8(getKeyDataField(MEMtPN8(locateObjectInMemory(keySetRef)), keyIdStart, MEMtPX8(&keyLen)));
			U8_INC(keyIdStart);
			nvmCopyAtomic(keyDataField, MEMtPM8(&APDUBuffer[valueOff]), keyLen);
			valueOff    		=  U16_A_U16(valueOff, keyLen); //offest: key len
		}
		memRefresh();

		return 0x9000;
	}

	memref getEntryByAppId(u8 sdId)
	{
		memref 		entry, theAppTable;
		u16		 	bFreeEntryIndex;
		u16		 	i;
		u8			bAppID;

		bFreeEntryIndex = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
		theAppTable = getAppTable();
		for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
		{
			entry = locateObjectInMemory(PN_R_U16(U32_A_U8(theAppTable , U8_M_U8(i , SIZE_REF))));
			bAppID = memReadU8(MEMtPN8(get_object_field_ref(entry, bAppID_INDEX)));
			if(bAppID == sdId)
			{
				return entry;
			}
		}
		return MEM_NULL;
	}

//	memref getKeySetObjBySdid(u8 sdId, u8	keyVersion)
//	{
//		u16					len;
//		PN8					sdDataObject;
//		PN8					keySetObj;
//
//		len					= 0;
//		sdDataObject		= MEMtPN8(sdData_getsdDataObjByAppId(sdId));
//		keySetObj			= MEMtPN8(locateObjectInMemory(getKeySets(sdDataObject, keyVersion, len)));
//
//		return PNtMEM(keySetObj);
//	}

	u16	keyData_fetchKey(u8	sdId, u8 keyVersion, u8 keyId, memref keyDataBuffer)
	{
		u8			keyLen;
		PN8			keySetObj;
		memref		keyDataField;
		jref		keySetRef;
		jref   	  	sdRef;
		jref	  	sdDataRef;
		memref    	enTry;
		PN8		  	sdObj;
		PN8			sdDataObject;
		jref		bPrivRef;
		memref		bPrivArray;
		u8 			bFirstPrivilageByte;

		keyLen		= 0;

		enTry	   	=	getEntryByAppId(sdId);

		//check SSD privilege
		bPrivRef = memReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(enTry), bPriv_INDEX)));
		bPrivArray = get_array_data(MEMtPF8(locateObjectInMemory(bPrivRef)));
		bFirstPrivilageByte = memReadU8(MEMtPM8(bPrivArray));
		if(U8_AND_U8(bFirstPrivilageByte, 0x80) != 0x80) {
			return keyLen;
		}

		sdRef      		=	nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(enTry), theApplet_INDEX)));
		sdObj      		=   MEMtPN8(locateObjectInMemory(sdRef));
		sdDataRef  		=   PN_R_U16(get_object_field_ref(sdObj,sdData_INDEX));

		if(!sdDataRef)
		{
			return keyLen;
		}

		sdDataObject	=	MEMtPN8(locateObjectInMemory(sdDataRef));

		keySetRef		=	getKeySets(sdDataObject, keyVersion, keyLen);

		if(!keySetRef)
		{
			return keyLen;
		}
		else
		{
			keySetObj	=	MEMtPN8(locateObjectInMemory(keySetRef));
		}

		//keySetObj		= MEMtPN8(getKeySetObjBySdid(sdId, keyVersion));
		keyDataField		= getKeyDataField(keySetObj, keyId, MEMtPX8(&keyLen));
		memCopyNoAtomic(MEMtPM8(keyDataBuffer), MEMtPM8(keyDataField), U8t16(keyLen));
		return keyLen;
	}

	void resetAllCounter(void)
	{
		u8	  	X_A		initValue[5];
		u16				keySetRef;
		PN8				keySetObj;
		PN8				sdDataObject;

		ecpMem_Set(initValue, 0x00, SCP80_SEQUENCE_CONTER_SIZE);
		sdDataObject    = MEMtPN8(sdData_getsdDataObjByAppId(0));
		keySetRef		= nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPN8(locateObjectInMemory(getKeyGroups(sdDataObject))), keySet_INDEX)));

		while(REF_NULL != keySetRef)
		{
			keySetObj 	= 	MEMtPN8(locateObjectInMemory(keySetRef));
			nvmCopyAtomic(MEMtPN8(get_object_field_ref(keySetObj, sequenceCounter_INDEX)), MEMtPM8(initValue), SCP80_SEQUENCE_CONTER_SIZE);
			keySetRef	=	keySet_getNextKeySetRef(keySetObj);
		}
		memRefresh();
	}

	jref getSCP80KeySEQ(u8 sdId, u8 keyVersion, PX8 counterppointer)
	{
		u16			keySetRef;
		PN8			SEQP;

		keySetRef	= getKeySets(MEMtPN8(sdData_getsdDataObjByAppId(0)), keyVersion, 0);
		SEQP		= MEMtPN8(get_object_field_ref(MEMtPF8(locateObjectInMemory(keySetRef)), sequenceCounter_INDEX));
		memCopyToRam(counterppointer, MEMtPM8(SEQP), SCP80_SEQUENCE_CONTER_SIZE);
		return keySetRef;
	}

	void setKeyCounterForSCP80(u8 sdId, u8 keyVersion, u8 len, PR8 counterValue)
	{
		u16			keySetRef;

		keySetRef	= getKeySets(MEMtPN8(sdData_getsdDataObjByAppId(0)), keyVersion, 0);
		nvmCopyAtomic(MEMtPN8(get_object_field_ref(MEMtPF8(locateObjectInMemory(keySetRef)), sequenceCounter_INDEX)), MEMtPM8(counterValue), len);
		memRefresh();
	}

	u16 getKeyNM(u16 appId,u16 keyVersion,u16 keyIndex, PM8 keyBuffer)
	{
		return keyData_fetchKey(U16t8(appId),U16t8(keyVersion),U16t8(keyIndex), PMtMEM(keyBuffer));
	}
#endif
