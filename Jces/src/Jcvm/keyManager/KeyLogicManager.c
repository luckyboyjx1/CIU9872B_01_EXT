/*
 * KeyLogicManager.c
 *
 *  Created on: 2013-9-29
 *      Author: liuhanren
 */

#include "CgmManager.h"
#include "typejava.h"
#include "mask.h"
#include "cardmanager.h"
#include "error.h"
#include "object.h"
#include "ob_manager.h"
#include "Loader.h"
#include "KeyLogicManager.h"
#include "KeyManagerOld.h"
#include "bc_object.h"
#include "global_variable.h"
#include "SecurityDomainData.h"
#include "keyData.h"
#include "keySet.h"
#include "casdStoreKey.h"
#include "contentManager.h"
#include "SCPInterface.h"
#include "scp10.h"
#include "opFunction.h"
#include "Crypto_Constant.h"
#include "nativemethod_impl.h"
#include "Crypto_API.h"
#include "JcvmRam.h"
#include "MemBackUp.h"
#include "context.h"
#include "keyGroup.h"
#include "fm02_keyData.h"
#include "fm03_KeyData.h"
#include "casdStoreKey.h"
#include "keyGroup.h"

#if GP__SCP__SCP11__FOR_MASK
#include "SCP11.h"
#endif

#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可
#include "SEEUICCInterface.h"

#if GP__KEY_MANAGER

#define 	DAP_KEY_INDEX                					0x01
#define 	RSA_KEY_LENGTH                					0x80

//#define 	AES_KEY_TYPE									0x88
#define 	DES_KEY_TYPE									0x80

#define 	PUT_KEY_GET_TAG                   				0x00
#define 	PUT_KEY_GET_LENGTH				  				0x01
#define 	PUT_KEY_GET_VALUES				  				0x02
#define 	PUT_KEY_CHECK_VALUES			  				0x03
#define 	PUT_KEY_KEY_USAGE				  				0x04
#define 	PUT_KEY_KEY_ACCESS				  				0x05
#define 	PUT_KEY_FINISH									0x06

#define		ANALYSE_KEY_COMPONENT_PUT		0x00
#define		ANALYSE_KEY_COMPONENT_STORT		0x01
#define 	ANALYSE_KEY_COMPONENT_DECRYPT	0x02

#if   GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
u8		g_PutRsaTokenKey;
#endif

memref getEntryByAppId(u8 sdId){

		memref 		entry, theAppTable;
		u16		 	bFreeEntryIndex;
		u16		 	i;
		u8			bAppID;

/*@ -disspecopt @*/
		bFreeEntryIndex = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
/*@ +disspecopt @*/
		theAppTable = getAppTable();
		for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
		{
			entry = locateObjectInMemory(memReadU16(MEMtPM8(U32_A_U16(theAppTable , U16_M_U16(i , SIZE_REF)))));
			bAppID = memReadU8(MEMtPN8(get_object_field_ref(MEMtPF8(entry), bAppID_INDEX)));
			if(bAppID == sdId)
			{
				return entry;
			}
		}
		return MEM_NULL;
}

u8	 KeyIdCounter(jref keySetRef, u8	currKeyId, u8	currKeyVersion)
{
			u8				keyNum, rebackId;
	
			if(currKeyVersion || g_isSetDefaultKey) //updata
			{
					if(g_putKeyCommands != MORE_COMMAND_FIRST_TAG)//闂佺偓鍎崇槐顡籶data
					{
							 rebackId	= U8_S_U8(g_PutKeyId, 1);
					}else{
							 rebackId	=	U8_AND_U8(APDUBuffer[OFFSET_P2] , MAX_KEY_ID);
					}
			}else{//add
					if((MORE_COMMAND_FIRST_TAG == g_putKeyCommands) 
																		 && !U8_AND_U8(APDUBuffer[OFFSET_P1] , PUT_MORE_KEY_MASK))
					{
							 rebackId	=	U8_AND_U8(APDUBuffer[OFFSET_P2] , MAX_KEY_ID);
					}else{
							 keyNum   = keySet_getKeyNum(MEMtPN8(locateObjectInMemory(keySetRef)));
						   rebackId	=	U16t8(U8_A_U8(keyNum, 1));
					}
							
			}
			
			return 	rebackId;
}

void resetSequenceCounter(jref keySetRef, u8 keyVersion){

		u8		tmpBuf [SCP80_SEQUENCE_CONTER_SIZE];
		u8      keyType;
		PN8 	keySetObj;
	
		keySetObj		  =	MEMtPN8(locateObjectInMemory(keySetRef));
	
		keyType = getSCPTypeByKeyVersion(keyVersion);

		ecpMem_Set(tmpBuf, CLEARED_VALUE, SCP80_SEQUENCE_CONTER_SIZE);

		if(keyType == PUT_KEY_SCP80)
		{
#if (EUICC_ONLY||SE_AND_EUICC)
			if((bRamBuffer[PROFILE_APDU_MODE] == TRUE)&&(G_profileGPState.keyCounter[0] == SCP80_SEQUENCE_CONTER_SIZE))
			{
				ecpMem_Copy(tmpBuf, MEMtPR8(&G_profileGPState.keyCounter[1]), SCP80_SEQUENCE_CONTER_SIZE);
			}
#endif
			keySet_setSequenceCounter(keySetObj, SCP80_SEQUENCE_CONTER_SIZE, MEMtPR8(&tmpBuf));
		}
		else if(keyType == PUT_KEY_SCP02)
		{
#if (EUICC_ONLY||SE_AND_EUICC)
			if((bRamBuffer[PROFILE_APDU_MODE] == TRUE)&&(G_profileGPState.keyCounter[0] == SCP02_SEQUENCE_CONTER_SIZE))
			{
				ecpMem_Copy(tmpBuf, MEMtPR8(&G_profileGPState.keyCounter[1]), SCP80_SEQUENCE_CONTER_SIZE);
			}
#endif
			keySet_setSequenceCounter(keySetObj, SCP02_SEQUENCE_CONTER_SIZE, MEMtPR8(&tmpBuf));
		}
#if  GP__AMENDMENT__D
		else if(keyType == PUT_KEY_SCP03)
		{
#if (EUICC_ONLY||SE_AND_EUICC)
			if((bRamBuffer[PROFILE_APDU_MODE] == TRUE)&&(G_profileGPState.keyCounter[0] == SCP03_SEQUENCE_CONTER_SIZE))
			{
				ecpMem_Copy(tmpBuf, MEMtPR8(&G_profileGPState.keyCounter[1]), SCP80_SEQUENCE_CONTER_SIZE);
			}
#endif
			keySet_setSequenceCounter(keySetObj, SCP03_SEQUENCE_CONTER_SIZE, MEMtPR8(&tmpBuf));
		}
#endif
		else
		{

		}
}
static void keyDataOffestAdd(u8 AddLen){

		#define  		ADD_LEN_MIN         0x00
		#define 		ADD_LEN_MAX			0x7F

		if(AddLen > ADD_LEN_MAX){
				keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
		}

		U8_INC(g_keyDataOffest);
		g_keyDataOffest    =  U16t8(U8_A_U8(g_keyDataOffest, AddLen));
}
/*
 * modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11 and ECC KEY
 */
u8 getFormatByKeyVersion(u8 KeyVersion){
	
		u8 KeyFormat;
	
		KeyFormat	=	KEY_FORMAT_02;

		if(IsSCP80Key(KeyVersion)) // add by xuxiaojun@20220808 profile 中SCP80 的三支key长度不一致导致profile下载失败
		{
			KeyFormat  =	KEY_FORMAT_03;
		}
#if GP__AMENDMENT__B__FOR_MASK
		if(IsSCP81Key(KeyVersion))
		{
			KeyFormat  =	KEY_FORMAT_03;
		}
#endif

#if GP__SCP__SCP11__FOR_MASK
		if(IsSCP11Key(KeyVersion))
		{
			KeyFormat  =	KEY_FORMAT_03;
		}
#endif

#if GP__SCP
		if(IsSpecialKey(KeyVersion))
		{
				KeyFormat  =	KEY_FORMAT_03;
		}
#endif
#if  GP__KEY_MANAGER__ECC_KEY
		if(KeyVersion ==  ECC_DOMAIN_KEY_VERSION)//ECC 曲线参数 storekey时没有keyversion  统一使用FE
		{
			KeyFormat  =	KEY_FORMAT_03;
		}
#endif
		return 	 KeyFormat;
}
#if GP__VERSION__FOR_CHINA_UNICOM_MASK
/*闁告帇鍊栭弻鍥亹閹惧啿顤匰D闁哄嫷鍨伴幆渚�绩椤栨稑鐦柣妤冩嚀閻ｉ箖鎯冮崙绋缂佺嫏鍐ㄧ劶*/
u8 Support_SCP_Mode(u8 mode,PM8 sdDataObj)
{
	u8			 scpType;
	u8			 tempI;
	u8 			 scpArrayLen;
	jref 		 scpBufferRef;
	memref		scpBufferFeild;
	memref 		scpBufferAddr;
	
  scpBufferRef = sdData_getScpBufferRef(sdDataObj);
	scpBufferAddr = locateObjectInMemory(scpBufferRef);
	scpArrayLen = get_array_length(scpBufferAddr);
	scpBufferFeild = get_array_data(MEMtPN8(scpBufferAddr));
	for(tempI = 0x00; tempI < scpArrayLen ;)
	{
		scpType = PX_R_U8(scpBufferFeild);
    if(scpType == mode)
		{
         return true ;
    }
		tempI = U8_A_U8(tempI, 2);
		scpBufferFeild = PM8_A_U8(scpBufferFeild, 2);
	}
	return false ;
}
#endif // GP__VERSION__FOR_CHINA_UNICOM_MASK

void KLG_InitPutKeySequence(u8 p1)
{
	if(g_putKeyCommands)
	{
			keyManager_InsertApduManager(PUT_KEY_COMMAND);
	}

	if (U8_AND_U8(p1, 0x80)) //more putKey commands
	{
		g_moreCommands = CHAINING_PUTKEY;


		if(!g_putKeyCommands){//多条指令第一条
			g_putKeyCommands = MORE_COMMAND_FIRST_TAG;
		}else{//多条指令第二条开始
			g_putKeyCommands = MORE_COMMAND_OTHERS_TAG;
		}
	}
	else // Last (or only) command , g_putKeyCommands有可能既是first，也是last
	{
		g_moreCommands = CHAINING_CLEAR;
		if(!g_putKeyCommands)
		{
				g_putKeyCommands = MORE_COMMAND_FIRST_TAG;
		}else{
				g_putKeyCommands = MORE_COMMAND_LAST_TAG;
		}
	}
}

/**
 * @description: 判断keyVersion是否合法
 * @parm       : keyVersion --- 密钥的version
 * @return     : version合法则返回true，否则返回false
 * @see        : gpBFCCompliance208
 */
u8 KeyVersionIsValid(u8 keyVersion)
{
	if(IsSpecialKey(keyVersion))
	{
		return true;
	}

	if ((JcvmConfig_GetMaxSCPKeyVersion() < keyVersion) || (JcvmConfig_GetMinSCPKeyVersion() > keyVersion))
	{
		return false;
	}

	return true;
}
/*
 * modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11 and ECC KEY
 */
static void  InspectKey(PN8 sdDataObj){

			u8 					bKeyIndex_;
			u8 					bNewKeyVer_;
			u8                  bNewKeyType_;
			u8					bCurrKeyVer_;
			u8 					bMultipleKeys_;
			u8					bCurrKeyTypebyId;
			u16					keySetRef;
#if GPC_CONFI_V2
			u16                 keyGroupRef;
#endif
#if GP__VERSION__FOR_CHINA_UNICOM_MASK 
	    	u8          		bCurrID;
#endif

			#define				INIT_VALUE0				0x00
	
			bMultipleKeys_	    = APDUBuffer[OFFSET_P2];
			bCurrKeyVer_		= U8_AND_U8(APDUBuffer[OFFSET_P1] , MAX_KEY_VERSION);
			bNewKeyVer_			= APDUBuffer[OFFSET_CDATA];
			bNewKeyType_        = APDUBuffer[OFFSET_CDATA + 1];
			bCurrKeyTypebyId	= INIT_VALUE0;
			bKeyIndex_			= U16t8(U8_AND_U8(bMultipleKeys_ , PUT_KEY_MASK));

			if(APDUBuffer[OFFSET_LC] == 0)
			{
					keyManager_abortLogicManagerThorwError(SW_WRONG_LENGTH);
			}
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
			if(MAX_KEY_ID < bKeyIndex_)
			{
					keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
					return;
			}
#else
			if((MAX_KEY_ID < bKeyIndex_) || (MIN_KEY_ID > bKeyIndex_)) // The Key Identifier is coded from '00' to '7F'.
			{
					keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
					return;
			}
			/*濞戞挾鐧凷D Put SCP80 key闁哄啳顔愮槐婵堟啺娴ｅ湱婀碨SD闁革负鍔岄悾銊ф啑閸涱厼妫橀柡浣烘珖ecure Channel Protocol Identifier 濞戞搫鎷� */
			
#if GP__VERSION__FOR_CHINA_UNICOM_MASK 
	    	bCurrID = U16t8(U16_AND_U16( cc, 0xFF));
			if ((bNewKeyVer_ >= 0x01) && (bNewKeyVer_ <= 0x1F)
				&& (bCurrID != 0)&& !(Support_SCP_Mode(SCP_80,sdDataObj)))
			{
				 
					keyManager_abortLogicManagerThorwError(SW_REFERENCED_DATA_NOT_FOUND);
					return;
			}

			#endif			

			if(IsSCP81Key(bNewKeyVer_))
			{
				if ((MAX_KEY_ID < bKeyIndex_) || (0x01 > bKeyIndex_)) // TLS KEY的KID可以不是 1 2 3
				{
					keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
					return;
				}
			}
#if GP__SCP__SCP11__FOR_MASK
			else if(IsSCP11Key(bNewKeyVer_))
			{
				if ((bKeyIndex_ < 0x10) || (bKeyIndex_ > 0x2F) || ((bKeyIndex_> 0x16)&&(bKeyIndex_ < 0x20)))
				{
					keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
					return;
				}
			}
#endif
#if GPC_CONFI_V2
			else if(IsSpecialKey(bNewKeyVer_)) // <GPC_Common_Implementation_Configuration_v2.0.pdf>    Table 4- 2:  Usage of Key Version Numbers and Identifiers for Other Mechanisms
			{
				if(bKeyIndex_ != 0x01)
				{
					keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
					return;
				}
			}
#endif
			else
			{
				if ((0x03 < bKeyIndex_) || (0x01 > bKeyIndex_))//add by zm; UICC CTS km_APDU_putKeyDes_SEC_00_36  key id must be between [1,3]
				{
					keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
					return;
				}
			}

#endif
	
			if(U8_AND_U8(bMultipleKeys_ , PUT_MORE_KEY_MASK) == PUT_MORE_KEY_MASK) // more PUT KEY commands will follow this one
			{
#if GP__AMENDMENT__B__FOR_MASK
				if(IsSCP81Key(bNewKeyVer_) && (U8_A_U8(bKeyIndex_, 1) > MAX_KEY_ID))
				{
					keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
					return;
				}
#endif
				if(U8_A_U8(bKeyIndex_, 2) > MAX_KEY_ID)
				{
					keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
					return;
				}
			}
			
			if(KeyVersionIsValid(bNewKeyVer_) == false)// modify by xuxiaojun@20220426 gpBFCCompliance208
			{
				keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
				return;
			}
            //Visa GlobalPlatform 2.1.1 Card Implementation Requirements v2.0.pdf: If this parameter contains a value other than  '00', a response of '6A86' shall be returned.
			if(CONFIG_VGP && (bNewKeyType_ == RSA_KEY_TYPE_A1) && (APDUBuffer[OFFSET_P1] != 0x00)) //modify by xuxiaojun@20220621 gpBFCCompliance208: km_APDU_putKeyRsa_OPR_loop_10
			{
				keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
				return;
			}
			if(bCurrKeyVer_)//GP221:  The Key Version Number identifies a key or group of keys that is already present on the card. A value of '00' indicates that a new key or group of keys is being added
			{
					keySetRef			=  keySet_getKeySet(sdDataObj, bCurrKeyVer_);
					if(!keySetRef)
					{
							keyManager_abortLogicManagerThorwError(SW_REFERENCED_DATA_NOT_FOUND);
					}
					
					if(bCurrKeyVer_ != bNewKeyVer_)//GP221: The version number of a new key or group of keys that will replace an existing key or group of keys (Key Version Number indicated in P1 is different from zero).
					{
							if(keySet_getKeySet(sdDataObj, bNewKeyVer_))
							{
									keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
							}
					}

					if((CONFIG_GP_FOR_MAPPING_TEST)&&(bCurrKeyVer_ == KEY_DAP_VERIF_ID1))//added by xuxiaojun@20220505 gpMGCompliance103:km_APDU_putKeyRsa_04_NoUpdation
					{
						keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2); //2[1].1.1_Mapping_guidelines_v1.0.1-Final.pdf: If this parameter contains a value other than '00', a response of '6A86' is returned.
					}

					if(getFormatByKeyVersion(bNewKeyVer_) != KEY_FORMAT_03)
					{
							bCurrKeyTypebyId	=	keySet_getKeyType(MEMtPN8(locateObjectInMemory(keySetRef)), bKeyIndex_,THROW_ERROR);
							if(bCurrKeyTypebyId)
							{						
									if (APDUBuffer[U8_A_U8(OFFSET_CDATA, 1)]  != 0xFF)
									{
									#if GP__VERSION__FOR_CHINA_UNICOM_MASK	
										if((0x83 == APDUBuffer[U8_A_U8(OFFSET_CDATA, 1)]) || (0x84 == APDUBuffer[U8_A_U8(OFFSET_CDATA, 1)]))
										{
											throw_error(SW_CONDITIONS_NOT_SATISFIED);
										}
									#endif
										if(bCurrKeyTypebyId != APDUBuffer[U8_A_U8(OFFSET_CDATA, 1)])
										{
											keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
										}
									}else{
									#if GP__VERSION__FOR_CHINA_UNICOM_MASK
										if((0x83 == APDUBuffer[U8_A_U8(OFFSET_CDATA, 2)]) || (0x84 == APDUBuffer[U8_A_U8(OFFSET_CDATA, 2)]))
										{
											throw_error(SW_CONDITIONS_NOT_SATISFIED);
										}
									#endif
										if(bCurrKeyTypebyId != APDUBuffer[U8_A_U8(OFFSET_CDATA, 2)])
										{
											keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
										}
									}									
							}
							else
							{
									keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
							}	
					}
			}
			else //keyversion已put ，P1 =00（new keyversion）创建一个相同版本的key 报错
			{
				keySetRef   	=  keySet_getKeySet(sdDataObj, bNewKeyVer_);
				if(keySetRef)
				{
						if(keyData_getKeyAttr(MEMtPN8(locateObjectInMemory(keySetRef)),  U8_AND_U8(APDUBuffer[OFFSET_P2], MAX_KEY_ID))
								&& (MORE_COMMAND_FIRST_TAG == g_putKeyCommands))
						{
								 keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
						}
				}
#if GPC_CONFI_V2
				else// As long as no Key Set has been successfully created within the Security Domain (including the Issuer SecurityDomain), this parameter  shall have a value of '81', indicating that a complete Secure Channel Key Set is being	created,   otherwise a response of '6A86' shall be returned
				{
					#if (EUICC_ONLY || SE_AND_EUICC)
					if (G_CurExecutingProfileIndex == ILLEGAL_PROFILEINDEX)	//下载profile过程中的putkey都是单支key的扩展格式putkey格式，不能使用以下判断
					#endif
					{
						if(IsSCP03Key(bNewKeyVer_)) //UL:  km_APDU_putKeyAes_error_noKeySet_wrongP2_keyIdentifierNot1_SSD_16B_scp03_9b_18_90
						{                                                          //<GPC_Common_Implementation_Configuration_v2.0>  Table 4- 1  Table 4- 2
							keyGroupRef = sdData_getKeyGroupRef(sdDataObj, SCP_03, false);

							if(((keyGroupRef == REF_NULL)||(memReadU16(MEMtPN8(getFirstKeySetHand(keyGroupRef))) == REF_NULL))&&(bMultipleKeys_ != 0x81)) //UL:  km_APDU_putKeyAes_success_SSD_05_24B_scp03_9b_fe_52
							{
								keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
							}
						}
						else if(IsSCP02Key(bNewKeyVer_))
						{
							keyGroupRef = sdData_getKeyGroupRef(sdDataObj, SCP_02, false);

							if(((keyGroupRef == REF_NULL)||(memReadU16(MEMtPN8(getFirstKeySetHand(keyGroupRef))) == REF_NULL))&&(bMultipleKeys_ != 0x81)) //UL:  km_APDU_putKeyAes_success_SSD_05_24B_scp03_9b_fe_52
							{
								keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
							}
						}
						else if(IsSCP80Key(bNewKeyVer_))
						{
							keyGroupRef = sdData_getKeyGroupRef(sdDataObj, SCP_80, false);

							if(((keyGroupRef == REF_NULL)||(memReadU16(MEMtPN8(getFirstKeySetHand(keyGroupRef))) == REF_NULL))&&(bMultipleKeys_ != 0x81)) //UL:  km_APDU_putKeyAes_success_SSD_05_24B_scp03_9b_fe_52
							{
								keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
							}
						}
					}
				}
#endif
			}
		#if GP__SCP__SCP10__FOR_MASK
			//濞ｅ浂鍠楅弫鍏煎緞閿燂拷			if(BIT_4 != (BIT_4 & sesKeyFlag)&&(SCP_10 == g_current_SCP_Type))//闁哄啰鍤�dec濞村吋淇洪惁鐣岋拷閸℃稒瀵�
			if(BIT_4 != (BIT_4 & sesKeyFlag)&&(SCP_10 == g_current_SCP_Type))//鏃燙-dec浼氳瘽瀵嗛挜
			{
				closeSecuritychannel(SC_TERMINATED);
				throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
			}
		#endif
}

#if GP__KEY_MANAGER__STORE_KEY
static void  AnalyKeyTypeAndLen (PN8  sdDataObj, u8 KeyType, u8 KeyLen){

			u8 					CurrScpType;

			CurrScpType   	=   sdData_getScpType(sdDataObj);

			switch(CurrScpType){
#if GP__SCP__SCP01
				case PUT_KEY_SCP01:

									break;
#endif
#if GP__SCP__SCP02
				case PUT_KEY_SCP02:
								if((KeyType != DES_KEY_TYPE) || (KeyLen != MAX_DES_KEY_LENGTH))
								{
										keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
								}
									break;
#endif
#if  GP__AMENDMENT__D
				case PUT_KEY_SCP03:
								if((KeyType != KEY_AES_TYPE)
										|| ((KeyLen != AES_KEY_LENGTH_16)
												&& (KeyLen != AES_KEY_LENGTH_24)
												&& (KeyLen != MAX_AES_KEY_LENGTH)))
								{
										keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
								}
								break;
#endif
				case PUT_KEY_SCP80:

									break;
#if GP__SCP__SCP10__FOR_MASK
				case PUT_KEY_SCP10:

									break;
#endif
				default:
					break;

			}
}
/*
 *merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: decryptKeyDataByDEK的第3个参数不能直接传0
 */
void decryptStoreKeyData(){

		u8  i;
		u8	keyDataOff;
		u8	keyRelatedDataOff;

		#define 		CHECK_VALUE_LEN          	0x03
		#define 		HIGHEST_ORDER_BYTES     	0x03
   		#define 		INIT_VALUES				        0x00
//		#define     	COMPARE_VALUES						0x00

		keyDataOff			  =   STORE_KEY_DATA_OFF;
		keyRelatedDataOff     =   STORE_KEY_CHECK_VALUE_OFF;
//		decryptKeyDataByDEK(keyDataOff, STORE_KEY_DATA_LEN,0);

		for (i = 0; i < MAX_KEYS_NUM; U8_INC(i))
		{
				decryptKeyDataByDEK(keyDataOff, MAX_DES_KEY_LENGTH,AnalyseEncKeyType()); //http解密key使用人是CBC模式，所以不能3支key一起解密
				encryptKeyCheckValueByDEK(keyDataOff, U8_S_U8(keyRelatedDataOff, 1), MEM_NULL, 0, 0x80, DES_KEY_LENGTH_8);
				keyRelatedDataOff  =  U16t8(U8_A_U8(keyRelatedDataOff, CHECK_VALUE_LEN));
				keyDataOff		   =  U16t8(U8_A_U8(keyDataOff, MAX_DES_KEY_LENGTH));
		}
}

static void  AnalyCurrKeyVerAndNewKeyVer (PN8  sdDataObj, u8 CurrKeyVer, u8 NewKeyVer){

		#define         SCP_MIN_KEY_CURRVER       0x00
		#define			SCP_MIN_KEY_NEWVER_KEY	  0x01

		if((SCP_MIN_KEY_CURRVER != CurrKeyVer) && (SCP_MAX_VERSION < CurrKeyVer))
		{
				keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
		}
		
		if((SCP_MIN_KEY_NEWVER_KEY > NewKeyVer) || (SCP_MAX_VERSION < NewKeyVer))
		{
				keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
		}
		
		if(CurrKeyVer != NewKeyVer)
		{
				if(keySet_getKeySet(sdDataObj, NewKeyVer))
				{
						keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
				}
		}else if(CurrKeyVer == NewKeyVer)
		{
				if(!keySet_getKeySet(sdDataObj, CurrKeyVer))
				{
						keyManager_abortLogicManagerThorwError(SW_REFERENCED_DATA_NOT_FOUND);
				}
		}
}

static void InspectstoreKeyForDES(PN8  sdDataObj){

		u8		keyLen;
		u8		keyRelatedDataLen;
		u8		keyType, keyRDataLen;
		u8		CurrkeyVer,  NewKeyVer;

		#define			 keyRelatedDataTag1ForDES			0x7F
		#define			 keyRelatedDataTag2ForDES			0x01

		keyRelatedDataLen	=  STORE_KEY_RELATED_DATA_OFF;

		if((APDUBuffer[U8_INC(keyRelatedDataLen)] != keyRelatedDataTag1ForDES)
								|| (APDUBuffer[U8_INC(keyRelatedDataLen)] != keyRelatedDataTag2ForDES)){
				throw_error(SW_WRONG_DATA);
		}

		keyRDataLen  = APDUBuffer[U8_INC(keyRelatedDataLen)];
		keyLen	 = APDUBuffer[STORE_KEY_DATA_LEN_OFF];
		if((keyRDataLen != STORE_KEY_RELATED_DATA_LEN) || (keyLen != STORE_KEY_DATA_LEN)){
				throw_error(SW_WRONG_DATA);
		}

		CurrkeyVer   =  APDUBuffer[U8_INC(keyRelatedDataLen)];
		NewKeyVer	 =  APDUBuffer[U8_INC(keyRelatedDataLen)];
		keyType		 =  APDUBuffer[U8_INC(keyRelatedDataLen)];

		AnalyKeyTypeAndLen(sdDataObj, keyType, MAX_DES_KEY_LENGTH);
		AnalyCurrKeyVerAndNewKeyVer(sdDataObj, CurrkeyVer, NewKeyVer);
}
//modify by xuxiaojun@20220621: Visa GlobalPlatform 2.1.1 Card Implementation Requirements v2.0.pdf: DAP key的exponent值只能是3或010001
static void InspectstoreKeyForDAP(){

		u8		RelatedDataOff, KeyDataLenOff;
		u8		keyLen, exponentLen;
		u8      exponentOff;
		u16 	    D_A			bPrivRef;
		memref	    D_A		    entry;
		memref	    D_A		    bPrivArray;

		#define			 KEY_RELATED_DATA1_DAP				0x01
		#define			 KEY_RELATED_DATA2_DAP				0x02

		KeyDataLenOff	 =	 STORE_KEY_DAP_LEN_OFF;
		keyLen			 =	 APDUBuffer[U8_INC(KeyDataLenOff)];

		if((keyLen != 0x81) && (keyLen != 0x83))
		{
				throw_error(SW_WRONG_DATA);
		}

		exponentLen		 =	 U8_LOW(keyLen);

		if(EXPONENT_LEN_BYTE1  == exponentLen)
		{
			if(CONFIG_VGP)//modify by xuxiaojun@20220906: 第二次代码评审_release_20220815_v1.3(20220822).xlsx: 中对于E值的判断放在配置中。否则的话，会导致兼容性变差
			{
				exponentOff	 =	 U16t8(U8_S_U8(U8_A_U8(KeyDataLenOff, keyLen), EXPONENT_LEN_BYTE1));

				if(APDUBuffer[exponentOff] != 0x03)
				{
					throw_error(SW_WRONG_DATA);
				}
			}
		}
		else if(EXPONENT_LEN_BYTE3 == exponentLen)
		{
			if(CONFIG_VGP)//modify by xuxiaojun@20220906: 第二次代码评审_release_20220815_v1.3(20220822).xlsx: 中对于E值的判断放在配置中。否则的话，会导致兼容性变差
			{
				exponentOff	 =	 U16t8(U8_S_U8(U8_A_U8(KeyDataLenOff, keyLen), EXPONENT_LEN_BYTE3));

				if((APDUBuffer[U8_INC(exponentOff)] != 0x01) || (APDUBuffer[U8_INC(exponentOff)] != 0x00) || (APDUBuffer[U8_INC(exponentOff)] != 0x01))
				{
					throw_error(SW_WRONG_DATA);
				}
			}
		}
		else
		{
			throw_error(SW_WRONG_DATA);
		}

		entry		=	getEntryByAppId(getCurrentSDId());
		bPrivRef	=	memReadU16(MEMtPN8(get_object_field_ref(entry, bPriv_INDEX)));
		bPrivArray 	= 	get_array_data(MEMtPN8(locateObjectInMemory(bPrivRef)));
		if(U8_AND_U8(PM_R_U8(bPrivArray), DAP_VERIFICATION_PRIVILEGES_TAG) != DAP_VERIFICATION_PRIVILEGES_TAG)
		{
			throw_error(SW_FUNC_NOT_SUPPORTED); //2[1].1.1_Mapping_guidelines_v1.0.1-Final.pdf: data field of the STORE DATA command containing the DAP Verification key information If the Supplementary Security Domain does not have DAP Verification privileges, a response of '6A81' is returned
		}

		//Visa GlobalPlatform 2.1.1 Card Implementation Requirements v2.0.pdf:  To populate the DAP Verification key within a Supplementary Security Domain on a configuration 3 implementation, the	Security Domain may only set up the Secure Channel  using the Security Domain’s own Secure Channel keys.
		if((CONFIG_GP_FOR_MAPPING_TEST || CONFIG_VGP)&&(g_bSDId[selected_channel] != channelInfo.selected_key_appId[selected_channel]))//added by xuxiaojun@20220505 gpMGCompliance103: km_APDU_putKeyRsa_01_NoDesKeys
		{
			throw_error(SW_CONDITIONS_NOT_SATISFIED); //2[1].1.1_Mapping_guidelines_v1.0.1-Final.pdf:  To populate the DAP Verification key within a Supplementary Security Domain, the Security Domain may only set up the Secure Channel using the Security Domain's own Secure Channel keys.Data field of the STORE DATA command containing the DAP Verification key information If the Supplementary Security Domain does not yet have its own Secure Channel Key Version Number, a response of '6985' is returned
		}

		RelatedDataOff	 =	 U16t8(U8_A_U8(KeyDataLenOff, keyLen));
		if((APDUBuffer[U8_INC(RelatedDataOff)] != KEY_RELATED_DATA1_DAP) // Related Data DGI  0102
				||	APDUBuffer[U8_INC(RelatedDataOff)] != KEY_RELATED_DATA2_DAP)
		{
				throw_error(SW_WRONG_DATA);
		}

		if((APDUBuffer[U8_INC(RelatedDataOff)] != DAP_RELATED_DATA_LEN)
				|| (APDUBuffer[U8_INC(RelatedDataOff)] != DAP_CURRENT_VERSION)
				|| (APDUBuffer[U8_INC(RelatedDataOff)] != DAP_NEW_VERSION))
		{
				throw_error(SW_WRONG_DATA);
		}
		
		if((APDUBuffer[U8_INC(RelatedDataOff)] != DAP_MODULUS_KEY_TYPE)
				|| (APDUBuffer[U8_INC(RelatedDataOff)] != DAP_MODULUS_LEN)
				|| (APDUBuffer[U8_INC(RelatedDataOff)] != DAP_EXPONENT_KEY_TYPE))
		{
				throw_error(SW_WRONG_DATA);
		}
		
		if(keySet_getKeySet(MEMtPN8(getCurrentSdObj(IS_NO_NEED_CHECK_SD)), DAP_NEW_VERSION))
		{
			throw_error(SW_WRONG_DATA);
		}	
}
/*
 * merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP:增加ECC KEY的处理
 */
static u16 InspectstoreKey(PN8  sdDataObj){

			u8  	keyDataTagOffest;
			u16 	storeKeyType;

			keyDataTagOffest	=  OFFSET_CDATA;
			storeKeyType		=  U8t16(APDUBuffer[U8_INC(keyDataTagOffest)]);
			storeKeyType		=  U16_OR_U16(U8_LEFT_SHIFT(storeKeyType), APDUBuffer[keyDataTagOffest]);

			if(DES_SECURE_CHANNEL_KEY == storeKeyType)
			{
				InspectstoreKeyForDES(sdDataObj);
			}
			else if(DAP_VERIFICATION_KEY == storeKeyType)
			{
				InspectstoreKeyForDAP();
			}
#if GP__KEY_MANAGER__STORE_DGI_KEY
			else if( (KEY_INF_DAT_DGI != storeKeyType) \
				&& (RSA_N_DGI != storeKeyType) \
				&& (RSA_E_DGI != storeKeyType) \
				&& (RSA_D_DGI != storeKeyType) \
				&& (KS_DGI != storeKeyType) \
				&& (RSA_CRT_IQP_DGI != storeKeyType) \
				&& (RSA_CRT_DQ_DGI != storeKeyType) \
				&& (RSA_CRT_DP_DGI != storeKeyType) \
				&& (RSA_CRT_Q_DGI != storeKeyType) \
				&& (RSA_CRT_P_DGI != storeKeyType)

#if GP__KEY_MANAGER__ECC_KEY
				&& (ECC_CurvePara_P_DGI != storeKeyType) \
				&& (ECC_CurvePara_A_DGI != storeKeyType) \
				&& (ECC_CurvePara_B_DGI != storeKeyType) \
				&& (ECC_CurvePara_G_DGI != storeKeyType) \
				&& (ECC_CurvePara_N_DGI != storeKeyType) \
				&& (ECC_CurvePara_K_DGI != storeKeyType) \
				&& (ECC_PublicKey_DGI != storeKeyType) \
				&& (ECC_PrivateKey_DGI != storeKeyType)
#endif
			)
#else
            else
#endif
			{
				throw_error(SW_WRONG_DATA);
			}

			return 		storeKeyType;
}
#endif // GP__KEY_MANAGER__STORE_KEY
/*
 * merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 增加默认key的SCP返回，增加ecc key的SCP返回
 */
u8	getSCPTypeByKeyVersion(u8	keyVersion)
{
		u8		bScpType;
	
		if(JcvmConfig_GetCmScpTypeIsSCP03)//modify by xuxiaojun@20220602 初始值应该与卡片的默认协议保持一致，测试用例：704 gpUICC2Compliance203： sm_APDU_initializeUpdate_success_SSD_kvnOutsideRangeScp03_onlyOneScpSupported_scp03_16B_de_a8_04
		{
			bScpType		=	PUT_KEY_SCP03;
		}
		else
		{
			bScpType		=	PUT_KEY_SCP02;
		}

		if(keyVersion == 0xFF)//default key ,根据SCP设置的默认值返回scptype
		{
			return bScpType;
		}
#if JAVA_CARD_UICC
		if(IsSCP80Key(keyVersion))
		{
				bScpType  =	PUT_KEY_SCP80;
		}
#endif
		if(IsSpecialKey(keyVersion))
		{
				bScpType  =	PUT_KEY_SCPFF;
		}
#if GP__AMENDMENT__B__FOR_MASK
		if(IsSCP81Key(keyVersion))
		{
				bScpType  =	PUT_KEY_SCP81;
		}
#endif
#if  GP__AMENDMENT__D
		if(IsSCP03Key(keyVersion))
		{
			bScpType  =	PUT_KEY_SCP03;
		}
#endif

#if GP__SCP__SCP11__FOR_MASK
		if(IsSCP11Key(keyVersion))
		{
			bScpType  =	PUT_KEY_SCP11;
		}
#endif

#if  GP__KEY_MANAGER__ECC_KEY
		if(keyVersion ==  ECC_DOMAIN_KEY_VERSION)//ECC 曲线参数 storekey时没有keyversion  统一使用FE
		{
			bScpType  =	PUT_KEY_SCPFF;
		}
#endif
		if(IsSCP02Key(keyVersion))//modify by xuxiaojun@20220628 修改卡片默认配置SCP03协议，安装一个只支持SCP02的SSD后给SSD put了SCP02 KEY之后，用该支key鉴权返回6A88的缺陷
		{
			bScpType  =	PUT_KEY_SCP02;
		}

		return bScpType;
}
/**
* merged from EPX\v3.0\DEVELOP by xuxiaojun@20220228 
*/
void setKeyData(jref keySet, u8 keyVersion, u8 keyId, u8 keyType, u16 keyLen, u16 SinglekeyComponentLen, u8 Off)
{
	u8	   				keyVersionState;
	memref  			keyDataStart;

#if		GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	PN8			sdDataObj;
#endif

	if(!keyVersion)
	{
			keyVersionState				=	TAG_NEW_KEY;
			if(g_isSetDefaultKey)
			{
					keyVersionState		=	TAG_UPDATE_KEY;
			}
	}else{
			keyVersionState		=	TAG_UPDATE_KEY;
	}
	
#if		GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	if((KEY_DES_TYPE	<=	keyType) && (KEY_DES_CBC_MODE >= keyType))
	{
			if(g_bSDId[selected_channel]) //SSD
			{
					sdDataObj		=	getCurrentSdObj(IS_NO_NEED_CHECK_SD);
					if(sdData_getDESKeyCount(sdDataObj))
					{	
							sdData_desDESKeyCount(sdDataObj);
					}else{
							keyManager_abortLogicManagerThorwError(SW_FILE_FULL);
					}
			}	
	}else if(RSA_KEY_TYPE_A1 == keyType)
	{
				g_PutRsaTokenKey = 0x01;
	}
#endif
	keyDataStart	=	keyData_prepareKeyData(MEMtPN8(locateObjectInMemory(keySet)), keyVersionState, keyId, keyType, SinglekeyComponentLen);

	//nvmCopyAtomic(keyDataStart, (memref)(MEM_U8_P)&APDUBuffer[Off], keyLen);
	//nvmCopyAtomic(MEMtPN8(keyDataStart), MEMtPM8(&APDUBuffer[Off]), keyLen);
	if(keyDataStart == MEM_NULL)//modify by xuxiaojun@20220725 更新密钥时，key type不能被改变 gp221: When replacing keys, the new keys shall be presented to the card in the same format as they are already present on the card
	{
		keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
	}
	else
	{
		SO_storeSecData(MEMtPX8(APDUBuffer), Off, SinglekeyComponentLen,  MEMtPM8(keyDataStart)); // UL: km_APDU_putKeyAes_success_SSD_05_24B_scp03_9b_fe_52

		memRefresh();
	}
// 	if(g_putKeyCommands == MORE_COMMAND_FIRST_TAG)
// 	{
// 			if (!isInProgress())
// 			{
// 				begin_transaction(MULTIL_APDU_PROCESS);
// 			}
// 	}
}
/*
*@function   :AnalyseKeyType
*
*@description:get the next state according to KeyType and currState
*
*see         :
*
*@parameter  :u8 KeyType     --- the current key type that is analyzing
*@parameter  :u8 currState   --- the current state that is analyzing
*@return     :the next state
*
*@history    :merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 增加RSA KEY和ECC KEY的处理
*/
static u8  AnalyseKeyType(u8 KeyType, u8 currState){

	u8 	  CurrState;
	
	CurrState = 0;

	if(g_keyUsageValueOffest)
	{
		CurrState  =  PUT_KEY_KEY_USAGE;
	}
	else if(g_keyAccessValueOffest)
	{
		CurrState  =  PUT_KEY_KEY_ACCESS;
	}
	else if(KeyType < DES_KEY_TYPE_MIN)     //keyType: 00 - 7F for private use, or maybe is equal to Length of key check value
	{
		if(currState == PUT_KEY_GET_TAG)
		{
			CurrState  =  PUT_KEY_CHECK_VALUES;
		}
		else if(currState == PUT_KEY_GET_VALUES)
		{
			CurrState  =  PUT_KEY_CHECK_VALUES;
		}
		else if(currState == PUT_KEY_CHECK_VALUES)
		{
			CurrState  =  PUT_KEY_KEY_USAGE;
		}
		else
		{
			CurrState  =  PUT_KEY_KEY_ACCESS;
		}
	}
	else if((KeyType <= DES_KEY_TYPE_MAX) // 80-85
					  || (KeyType  == AES_KEY_TYPE_MAX) // 88
					  || (KeyType  == SHA1_KEY_TYPE_MIN) // 90
					  || (KeyType  == SHA1_KEY_TYPE_MAX) // 91
					  )
	{
		if(currState == PUT_KEY_GET_TAG) // 这里为什么没有别的状态？80-85以及88应该有checkValue的啊
		{
				CurrState 	   =  PUT_KEY_GET_LENGTH;   // the next state is PUT_KEY_GET_VALUES
		}
		else
		{
				CurrState 	   =  PUT_KEY_GET_TAG;
		}
	}
#if GP__KEY_MANAGER__PUT_RSA_KEY
	else if((KeyType >= RSA_E_DGI_TYPE) && (KeyType <= RSA_CRT_DQ_DGI_TYPE))   // A0 - A8
	{
		if(currState == PUT_KEY_GET_TAG)
		{
				CurrState 	   =  PUT_KEY_GET_LENGTH;   // the next state is PUT_KEY_GET_VALUES
		}
		else
		{
				CurrState 	   =  PUT_KEY_GET_TAG;
		}
	}
#endif

#if GP__KEY_MANAGER__ECC_KEY
	else if(((KeyType <= ECC_CurvePara_K_DGI_TYPE)&&(KeyType >= ECC_PublicKey_DGI_TYPE)) || (KeyType == ECC_KEYPARAREF_DGI_TYPE)) //B0 B1 B2-B7 F0
	{
		if(currState == PUT_KEY_GET_TAG)
		{
			CurrState 	   =  PUT_KEY_GET_LENGTH; // the next state is PUT_KEY_GET_VALUES
		}
		else
		{
			CurrState 	   =  PUT_KEY_GET_TAG;
		}
	}
#endif
	else
	{
		keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
	}
	return 	CurrState;
}
/*
 * @function :  this function only be called when the currState is PUT_KEY_GET_VALUES
 * @parameter:  u8 currKenLen --- the current key length
 * @parameter:  u8 Off        --- the offset point to Key Component Block
 * @parameter:  u8 currState  --- PUT_KEY_GET_VALUES
 *
 * @return   :  the next state
 */
static u8 AnalyseKeyValues(u8 currKeyLen, u8 Off, u8 currState){

	u8   KeyType;

	KeyType		=   APDUBuffer[U8_A_U8(Off, currKeyLen)];

	return AnalyseKeyType(KeyType, currState);

}
/**
*merged from EPX\v3.0\DEVELOP by xuxiaojun@20220228 缺陷系统编号363： AES put key mac len 取值错误
*merged from EPX\v3.0\DEVELOP by xuxiaojun@20220520 support SCP11, GPC_2_3_1
*modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32
*/
static void setKeyDataForKeyIdUnit(u8 keyDataOffestStart, u16 keyCompomentLen, jref keySet, u8 currKeyVersion, u8 newKeyVersion, u8 keyId, u8 KeyFormat,u8  encryptkeyType){

	u8 		KeyType, KeyLen;
	u8		keyCurrState, IshasKeyUsage, AccessLen;
	u8		OffestStart, checkValueLen,SingleKeyPaddingLen;
	u16		currLen;
	u16		SinglekeyComponentLen;
	u8		keyUsageQualifier[3]; //merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 兼容key usage两字节的情况
#if JAVA_CARD_UICC
	u8 		aesMacLen;//add by xuxiaojun 20220218 ,兼容AES 8810 A8B71742BBBC4169A8B71742BBBC4169格式不带mac len的 put key
#endif
	#define			INIT_VALUE0						0x00
	#define			INIT_VALUE1						0x01
	#define 		PUT_KEY_LENGTH_BYTE				0x02
	#define         PUT_KEY_RSA_PUBLIC_LEN			0x80
	#define         PUT_KEY_FORMAT2_TAG				0xFF

	IshasKeyUsage	=	INIT_VALUE0;
	keyCurrState	=	PUT_KEY_GET_TAG;
	OffestStart		= keyDataOffestStart;
	currLen         =	INIT_VALUE0;
	KeyType         =	INIT_VALUE0;
	SingleKeyPaddingLen = INIT_VALUE0;
#if JAVA_CARD_UICC
	aesMacLen		= INIT_VALUE0;
#endif
	while(keyCompomentLen > 0)
	{
		switch(keyCurrState)
		{
			case   PUT_KEY_GET_TAG:

				   KeyType					= APDUBuffer[U8_INC(OffestStart)];
				   SingleKeyPaddingLen = 0;
				   aesMacLen = 0;// 同步bug516
				   if(KeyType == PUT_KEY_FORMAT2_TAG)
				   {
						   KeyType			=	APDUBuffer[U8_INC(OffestStart)];
						   IshasKeyUsage	=	INIT_VALUE1;
						   U16_DEC(keyCompomentLen);
				   }
#if GP__AMENDMENT__B__FOR_MASK
				   //NewKeyVersion KeyType KeyDataLgth KeyData CheckValueLength CheckValue               KeyType KeyDataLgth KeyData CheckValueLength CheckValue
				   //NewKeyVersion KeyType KeyDataLgth KeyData CheckValueLength CheckValue NewKeyVersion KeyType KeyDataLgth KeyData CheckValueLength CheckValue
				   else if((IsSCP81Key(newKeyVersion))&&(KeyType == newKeyVersion)) //这行代码不会走到 ，因为进入这个函数之前，已经把这个key version处理掉了 the second key start with keyVersion:80D800813A 41 85 2120AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA00 41 8011103489DEAE78BC45EC3489DEAE78BC45EC00
				   {
					   KeyType			=	APDUBuffer[U8_INC(OffestStart)];
					   U16_DEC(keyCompomentLen);
				   }
#endif
				   U8_INC(keyCurrState);
				   U16_DEC(keyCompomentLen);
				   break;

			case   PUT_KEY_GET_LENGTH:
			{
					if(0x82 == APDUBuffer[OffestStart])
					{
						currLen  			= PX_R_U16(&APDUBuffer[U16_A_U8(OffestStart , 1)]);
						OffestStart			= U16t8(U8_A_U8(OffestStart, SIZE_82_LENGTH_1));
						keyCompomentLen     = U16_S_U8(keyCompomentLen, SIZE_82_LENGTH_1);
					}else if(0x81 == APDUBuffer[OffestStart]){

						currLen  			= PX_R_U8(&APDUBuffer[U16_A_U8(OffestStart , 1)]);
						OffestStart	    	= U16t8(U8_A_U8(OffestStart, SIZE_81_LENGTH_1));
						keyCompomentLen     = U16_S_U8(keyCompomentLen, SIZE_81_LENGTH_1);
					}else if(APDUBuffer[OffestStart] <= 0x80){

						currLen				= PX_R_U8(&APDUBuffer[OffestStart]);		//11
						if(IsRSAKeyType(KeyType)//SCP03时   (encryptkeyType == KEY_AES_TYPE)，会做LLV的判断，由于RSA ECC的长度不一定是8的整数倍 会导致正常数据结构被当成LLV  所以在这里做了限制
#if  GP__KEY_MANAGER__ECC_KEY
							||IsECCKeyType(KeyType)
#endif
						){}//非对称秘钥 暂时不支持LLV结构 不做判断
						else
						{
							if((IsSCP81Key(newKeyVersion))||((KeyType == KEY_AES_TYPE)||(encryptkeyType == KEY_AES_TYPE)/*&&(APDUBuffer[OFFSET_CDATA] <= SCP_MAX_VERSION)*/))//参考附录D和HTTPS规范
							{
								//当key格式是88 11 10 11223344556677889910111213141516才要偏移， 如果是 88 10 11223344556677889910111213141516则不用往后偏移1
								if ((APDUBuffer[OffestStart] != 0x10) && (APDUBuffer[OffestStart] != 0x18) && (APDUBuffer[OffestStart] != 0x20))
								{
									SingleKeyPaddingLen = U8_S_U8(APDUBuffer[OffestStart],APDUBuffer[OffestStart+1]);
									U8_DEC(SingleKeyPaddingLen);//102226  8.2.1.5.1 : used with an AES key as encryption key (DEK).
#if JAVA_CARD_UICC
									if(((APDUBuffer[OffestStart] - 1) == 0x10)||((APDUBuffer[OffestStart] - 1) == 0x18) || ((APDUBuffer[OffestStart] - 1) == 0x20))		// mac  not  exist, modify by xuxiaojun@20220306 电信研究院测试本地APDU执行put aes 192长度key时，密文长度不是0x10的整数倍  0x19 + 0x18 + V
									{
										aesMacLen = 0;
									}
									else if(((APDUBuffer[OffestStart] - 2) == 0x10)||((APDUBuffer[OffestStart] - 2) == 0x18) || ((APDUBuffer[OffestStart] - 2) == 0x20)) // mac  exist  and length is correct, modify by xuxiaojun@20220306 电信研究院测试本地APDU执行put aes 192长度key时，密文长度不是0x10的整数倍
									{
										aesMacLen = 1;
									}
									else
									{
										//keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);	//要不要抛异常
									}
#endif
									//U16_DEC(currLen);
									U8_INC(OffestStart);
									U16_DEC(keyCompomentLen);
									currLen = APDUBuffer[OffestStart];
																// xxFF881110-------03XXXXXX ---
															  // XXFF881210-----MAClEN 03XXXXXX---
	//								if(((IsSCP80Key(APDUBuffer[OFFSET_CDATA]))||(APDUBuffer[OFFSET_CDATA] == 0x11))
	//								&&(KeyType == KEY_AES_TYPE)&&(keyId == 0x02))//profile规范，附录D ，102226 ：The field "length of the MAC" shall be present if "ciphered key" contains an AES key with key identifier '02' and key version '01' to '0F' or '11'
	//								{
	//									U16_DEC(currLen);
	//								}
								}
								else
								{
									aesMacLen = 0; // 同步bug516
								}
							}
						}
						U8_INC(OffestStart);
						U16_DEC(keyCompomentLen);
					}else if(APDUBuffer[OffestStart] == PUT_KEY_RSA_PUBLIC_LEN){
						currLen				= PUT_KEY_RSA_PUBLIC_LEN;
						U8_INC(OffestStart);
						U16_DEC(keyCompomentLen);
				}
				   U8_INC(keyCurrState);
				   break;
			}
			case   PUT_KEY_GET_VALUES:

				   KeyLen                   = U16t8(currLen);
				   SinglekeyComponentLen	= currLen;

				   setKeyData(keySet, currKeyVersion, keyId, KeyType, KeyLen, SinglekeyComponentLen, OffestStart);

				   if(keyCompomentLen)     
				   {
					   keyCurrState			= AnalyseKeyValues(U16t8(U16_A_U8(SinglekeyComponentLen,SingleKeyPaddingLen)), OffestStart, keyCurrState);
				   }
				   OffestStart    			= U16t8(U8_A_U8(OffestStart, U8_A_U8(currLen,SingleKeyPaddingLen)));
				   keyCompomentLen  		= U16_S_U8(keyCompomentLen, U8_A_U8(currLen,SingleKeyPaddingLen));
#if JAVA_CARD_UICC
				   if(((IsSCP80Key(APDUBuffer[OFFSET_CDATA]))||(APDUBuffer[OFFSET_CDATA] == 0x11))
					&&(KeyType == KEY_AES_TYPE)&&(keyId == 0x02))//profile规范，附录D ，102226 ：The field "length of the MAC" shall be present if "ciphered key" contains an AES key with key identifier '02' and key version '01' to '0F' or '11'
					{
						if(aesMacLen == 1)
						{
							keySet_setAesKeyMacLen(MEMtPN8(locateObjectInMemory(keySet)),APDUBuffer[OffestStart-1]);
						}
						else
						{
							keySet_setAesKeyMacLen(MEMtPN8(locateObjectInMemory(keySet)),AES_DEFAULT_MAC_LENGTH); //add by xuxiaojun 20220218 ,兼容AES 8810 A8B71742BBBC4169A8B71742BBBC4169格式不带mac len的 put key
						}
					}
					else
					{
						if(aesMacLen == 1)
						{
							// 要不要抛异常
						}
					}
#endif
				   break;
			case   PUT_KEY_CHECK_VALUES:
//					if(((IsSCP80Key(APDUBuffer[OFFSET_CDATA]))||(APDUBuffer[OFFSET_CDATA] == 0x11))
//						&&(KeyType == KEY_AES_TYPE)&&(keyId == 0x02))//profile规范，附录D ，102226 ：The field "length of the MAC" shall be present if "ciphered key" contains an AES key with key identifier '02' and key version '01' to '0F' or '11'
//					{
//						keySet_setAesKeyMacLen(MEMtPN8(locateObjectInMemory(keySet)),APDUBuffer[OffestStart]);
//						U8_INC(OffestStart);//指向CHECK_VALUE_LEN
//						U16_DEC(keyCompomentLen);
//					}

				   checkValueLen    		= U16t8(U8_A_U8(APDUBuffer[OffestStart], 1));
				   OffestStart      		= U16t8(U8_A_U8(OffestStart, checkValueLen));
				   keyCompomentLen  		= U16_S_U8(keyCompomentLen, checkValueLen);

				   if(IshasKeyUsage)
				   {
						   U8_INC(keyCurrState);
				   }
				   else{
						   keyCurrState		= PUT_KEY_GET_TAG;
						   U8_INC(keyId);
				   }
				   break;
			case	PUT_KEY_KEY_USAGE:
				   keyUsageQualifier[0]	= APDUBuffer[U8_INC(OffestStart)];
				   if(keyUsageQualifier[0] == 0x01) // merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 兼容GP2.3中keyUsageQualifier为2字节的情况
				   {
					   keyUsageQualifier[1] = APDUBuffer[OffestStart];
					   keyUsageQualifier[2] = 0x00;
				   }
				   else
				   {
					   ecpMem_Copy(&keyUsageQualifier[1], &APDUBuffer[OffestStart], 0x02);
				   }
				   keyData_setKeyUsageQualifier(MEMtPN8(locateObjectInMemory(keySet)), keyId, &keyUsageQualifier[0]);
				   OffestStart				=	U16t8(U8_A_U8(OffestStart, keyUsageQualifier[0]));
				   keyCompomentLen			=	U16_S_U8(keyCompomentLen, U8_A_U8(keyUsageQualifier[0], 1));
				   U8_INC(keyCurrState);
				   break;
			case   PUT_KEY_KEY_ACCESS:

				   AccessLen				=	APDUBuffer[U8_INC(OffestStart)];
				   keyData_setKeyAccess(MEMtPN8(locateObjectInMemory(keySet)), keyId, &APDUBuffer[OffestStart]);
				   OffestStart				=	U16t8(U8_A_U8(OffestStart, AccessLen));
				   keyCompomentLen			=	U16_S_U16(keyCompomentLen, U8_A_U8(AccessLen, 1));
				   U8_INC(keyId);
				   keyCurrState				=	PUT_KEY_GET_TAG;
				   break;
			default:
					keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
				break;
		}
	}
}
/*
 * merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 兼容GP2.3中keyUsageQualifier为2字节的情况
 * modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32
 */
static void	SetKeyUsageAndKeyAccess(jref KeySetRef, u8 keyId)
{
	u8 keyUsageQualifier[3];

	if(KeySetRef)
	{
		if(g_putKeyCurrState == PUT_KEY_KEY_USAGE)
		{
			keyUsageQualifier[0] = APDUBuffer[g_keyDataOffest];

			if(APDUBuffer[g_keyDataOffest] == 0x01)
			{
				keyUsageQualifier[1] = APDUBuffer[g_keyDataOffest+1];
				keyUsageQualifier[2] = 0x00;
			}
			else
			{
				ecpMem_Copy(&keyUsageQualifier[1], &APDUBuffer[g_keyDataOffest+1], 0x02);
			}

			keyData_setKeyUsageQualifier(MEMtPN8(locateObjectInMemory(KeySetRef)), keyId, &keyUsageQualifier[0]);
		}
		else if(g_putKeyCurrState == PUT_KEY_KEY_ACCESS)
		{
			keyData_setKeyAccess(MEMtPN8(locateObjectInMemory(KeySetRef)), keyId, &APDUBuffer[g_keyDataOffest+1]);
		}
	}
}
/*
*@function   :AnalyseKeyCheckOrUsageOrAccess
*
*@description:解析key check value / Usage / Access, 如果KeySetRef不为null, 则将值更新到对应的位置；如果为空，则只返回
*
*see         :GP231 Format of the Key Data Field

*@param      :jref KeySetRef --- 本次操作的key set的引用
*@param      :u8 keyId       --- 本次操作的key set的id
*
*@return     :相应的check value / Usage / Access的长度值 + 长度域占用的字节数
*
*@history    :V1.0:
*/
static u8 AnalyseKeyCheckOrUsageOrAccess(jref KeySetRef, u8 keyId)
{
	u8 	TypeLen;
	u8	SplitLen;

	if(!g_KeepLenForUsageOrAccess)
	{
		TypeLen      =  APDUBuffer[g_keyDataOffest]; //g_keyDataOffest point to Length of key check value / Length of key usage qualifier / Length of key access

		if(TypeLen)
		{
			SetKeyUsageAndKeyAccess(KeySetRef, keyId);
		}

		if(g_PutKeyRemaindLen < U8t16(TypeLen))        //Check\Usage\Access:
		{
			if(g_putKeyCurrState != PUT_KEY_CHECK_VALUES)
			{
				SplitLen    =  U8_S_U8(TypeLen, g_PutKeyRemaindLen);
				if(g_putKeyCurrState == PUT_KEY_KEY_USAGE)
				{
					g_keyUsageValueOffest	 =	SplitLen;
				}
				else
				{
					g_keyAccessValueOffest 	 =  SplitLen;
				}
				g_KeepLenForUsageOrAccess	 =  U8t16(TypeLen);
			}
			else
			{   //check value 閼界粯瀵�
				keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
			}
		}

		keyDataOffestAdd(TypeLen);
				
        U8_INC(TypeLen);

	}
	else
	{
		if(g_putKeyCurrState == PUT_KEY_KEY_USAGE)
		{
			TypeLen	 	   =  U16t8(U16_S_U8(g_KeepLenForUsageOrAccess , g_keyUsageValueOffest));
		}
		else
		{
			TypeLen	 	   =  U16t8(U16_S_U8(g_KeepLenForUsageOrAccess , g_keyAccessValueOffest));
		}
		SetKeyUsageAndKeyAccess(KeySetRef, keyId);
		keyDataOffestAdd(TypeLen);
	}

	return TypeLen;

}
/*
 * return the length of key data, which used to new key data space
 */
static u16 AnalyseKeyLength(u8 isNewKey, u8 KeyFormat/*,u8 KeyType*/){

			u16     			SingleKeyComponentLen;

			#define 			INIT_CURR_KEY_LEN				0x00

			SingleKeyComponentLen	=   INIT_CURR_KEY_LEN;

			if(0x82 == APDUBuffer[g_keyDataOffest])
			{
				if(g_PutKeyRemaindLen < SIZE_82_LENGTH_1)
				{
					keyManager_abortLogicManagerThorwError(SW_WRONG_LENGTH);
				}

				g_keyDataOffest	    = U16t8(U8_A_U8(g_keyDataOffest, SIZE_82_LENGTH_1));
				g_PutKeyRemaindLen    = U16_S_U8(g_PutKeyRemaindLen, SIZE_82_LENGTH_1);
				SingleKeyComponentLen  	= PX_R_U16(&APDUBuffer[U8_A_U8(g_keyDataOffest , 1)]);

			}else if(0x81 == APDUBuffer[g_keyDataOffest])
			{
				if(g_PutKeyRemaindLen < SIZE_81_LENGTH_1)
				{
					keyManager_abortLogicManagerThorwError(SW_WRONG_LENGTH);
				}

				SingleKeyComponentLen  	= PX_R_U8(&APDUBuffer[U16_A_U8(g_keyDataOffest , 1)]);
				g_keyDataOffest	    = U16t8(U8_A_U8(g_keyDataOffest, SIZE_81_LENGTH_1));
				g_PutKeyRemaindLen    = U16_S_U8(g_PutKeyRemaindLen, SIZE_81_LENGTH_1);
			}else if(APDUBuffer[g_keyDataOffest] <= 0x80){

				SingleKeyComponentLen		= PX_R_U8(&APDUBuffer[g_keyDataOffest]);
				U8_INC(g_keyDataOffest);
				U16_DEC(g_PutKeyRemaindLen);
			}else{
					keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
			}

			if(g_PutKeyRemaindLen < SingleKeyComponentLen)
			{//KeyCompoment
				if (CONFIG_GP_FOR_BFC)	//merged from EPX\v2.1\BRANCH_MODULE\FINANCE_TMC_THD89: UL-BFC: km_APDU_putKeyDes_OPR_11_20
				{
					keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
				}
				else
				{
					keyManager_abortLogicManagerThorwError(SW_WRONG_LENGTH);
				}
			}

			if(KeyFormat  ==  KEY_FORMAT_03)
			{
				g_PutkeyDataLen 	   = keyData_computeKeyDataLen(KeyFormat, g_PutkeyDataLen, SingleKeyComponentLen, isNewKey);
			}else{
				if(!U16_S_U16(g_PutKeyRemaindLen, SingleKeyComponentLen))   //閸垶瀵氬寮卐y comcheckValue閹告ut
				{
							keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
				}

#if	(GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK | GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK)
				if(!g_PutkeyDataLen) //FORMAT3,閻愭柨顨�
				{
							g_PutkeyDataLen   =  SingleKeyComponentLen;
				}

				if(g_PutkeyDataLen != SingleKeyComponentLen) //濮濐晳eyId v 閸婇棿绔�
				{

					keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
				}
#else
				if(g_PutkeyDataLen != SingleKeyComponentLen)
				{
					g_PutkeyDataLen   =  SingleKeyComponentLen;
				}
#endif
			}

			return 	SingleKeyComponentLen;

}
/*
 * merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
 */
/*static*/ u16  newKeySetByKeyVersion(PN8 sdDataObj, u8 KeyFormat, u8 curKeyVersion, u8 newKeyVersion, u16 keyLen){


		u16	 				KeySetRef, tmpKeySetRef;
		PN8 				keyDataAField;
		u8					temKeyLen;

		#define				SET_HIGH_BIT			0x80
	
		KeySetRef   	=  keySet_getKeySet(sdDataObj, newKeyVersion);
	
		if(!curKeyVersion)
		{
				if(KeySetRef)
				{
						if(keyData_getKeyAttr(MEMtPN8(locateObjectInMemory(KeySetRef)),  U8_AND_U8(APDUBuffer[OFFSET_P2], MAX_KEY_ID))
							                                                  && (MORE_COMMAND_FIRST_TAG == g_putKeyCommands))
						{
								 keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
						}
				}
				
				if((getFormatByKeyVersion(newKeyVersion) != KEY_FORMAT_03))//keydata婵☆偒鍎锟筋儑閸氥劎鏁囬敓绱籈S key 娑擄拷\16\24
				{
						if(!g_bSDId[selected_channel]) // only valid for ISD
						{
							tmpKeySetRef  	=  keySet_getKeySet(sdDataObj, KEY_DEFAULT_VERSION);//閸垳绔磒utkey, 姒涙Βey
							if((tmpKeySetRef) && ((getSCPTypeByKeyVersion(newKeyVersion) ==  PUT_KEY_SCP02)||(getSCPTypeByKeyVersion(newKeyVersion) ==  PUT_KEY_SCP03)) )
							{
									g_isSetDefaultKey	=	true;
									KeySetRef					=	tmpKeySetRef;
									keySet_replaceKeyVersion(MEMtPN8(locateObjectInMemory(KeySetRef)), newKeyVersion);

									keyDataAField = MEMtPN8(keySet_getKeyDataAField(MEMtPN8(locateObjectInMemory(KeySetRef))));

									switch (sdData_getScpType(sdDataObj)) //参考函数initIsdKey todo 这里是用来做什么的？现在由于CM_SCP_TYPE的定义在EPX2.1中发生了变化，所以这里的使用不正确
									{
										case 0x01:
										case 0x02:
											temKeyLen	=	DES_KEY_LENGTH;
											break;
										case 0x03:
											temKeyLen	=	MAX_AES_KEY_LENGTH;
											break;
										case 0x80:
											temKeyLen	=	AES_KEY_LENGTH_16;
											break;
										default:
											keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
											break;
									}

									if (keyLen <= SO_calSecDataAreaLen(temKeyLen)) //新默认密钥长度不能超过初始默认密钥长度temKeyLen
									{
											memWriteU8NoAtomic(PN8_A_U16(keyDataAField, FM02_KEY_LENGTH_OFFSET), (u8)keyLen);
									}
									else
									{
											keyManager_abortLogicManagerThorwError(SW_FILE_FULL);
									}
								
									//set keyFormat
									if(((0xFF	==	APDUBuffer[U8_A_U8(OFFSET_CDATA, 1)])||(g_storeDGIKeyFlag == TRUE))//store DGI KEY 00B9
#if EUICC_ONLY||SE_AND_EUICC
											&&(bRamBuffer[PROFILE_APDU_MODE] != TRUE)//profile的key  getdata时返回简单格式 HB脚本 6KeyEstablish指令正常测试/Tele_TC1_ISDR_KeyEstablishment_Test_01
#endif
									)
									{
											KeyFormat   = U8_OR_U8(KeyFormat, SET_HIGH_BIT); // 指令中使用的是Key Information Data Structure – Extended
									}
									keySet_setKeyFormat(MEMtPN8(locateObjectInMemory(KeySetRef)), KeyFormat);
							}
						}
				}
				
				if(!KeySetRef)
				{
						if(((0xFF	==	APDUBuffer[U8_A_U8(OFFSET_CDATA, 1)])||(g_storeDGIKeyFlag == TRUE))//store DGI KEY 00B9
#if EUICC_ONLY||SE_AND_EUICC
							&&(bRamBuffer[PROFILE_APDU_MODE] != TRUE)//profile的key  getdata时返回简单格式 HB脚本 6KeyEstablish指令正常测试/Tele_TC1_ISDR_KeyEstablishment_Test_01
#endif
						)
						{
								KeyFormat   = U8_OR_U8(KeyFormat, SET_HIGH_BIT);
						}

						KeySetRef  	=  keySet_newKeySet(KeyFormat, sdDataObj, newKeyVersion, keyLen);
						g_isNewKey	=  true;
				}
		}else
		{
				if(newKeyVersion   !=  curKeyVersion)
				{
						KeySetRef  	= keySet_getKeySet(sdDataObj, curKeyVersion);
						keySet_replaceKeyVersion(MEMtPN8(locateObjectInMemory(KeySetRef)), newKeyVersion);

						//set keyFormat
						if(((0xFF	==	APDUBuffer[U8_A_U8(OFFSET_CDATA, 1)])||(g_storeDGIKeyFlag == TRUE))//todo 为什么store key时全部用extended格式呢？
#if EUICC_ONLY||SE_AND_EUICC
							&&(bRamBuffer[PROFILE_APDU_MODE] != TRUE)//profile的key  getdata时返回简单格式 HB脚本 6KeyEstablish指令正常测试/Tele_TC1_ISDR_KeyEstablishment_Test_01
#endif
						)
						{
								KeyFormat   = U8_OR_U8(KeyFormat, SET_HIGH_BIT); // 指令中使用的是Key Information Data Structure – Extended
						}
						keySet_setKeyFormat(MEMtPN8(locateObjectInMemory(KeySetRef)), KeyFormat);
				}
		}

		if((KeyFormat != KEY_FORMAT_03)||(IsSCP80Key(newKeyVersion))) //SCP KEYuupdate count
		{
			#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK /*modified by zm: sequence counter;1.0  */
			//if ((APDUBuffer[OFFSET_P1]) || ((true != IsKeySetExits) || (0x03 > currentKeyNum)))
			if (0x04 != APDUBuffer[OFFSET_P2])
			#endif
			{
					resetSequenceCounter(KeySetRef, newKeyVersion);
			}
		}
		
		return 		KeySetRef;
}
#if GP__KEY_MANAGER__ECC_KEY

/*
 * merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
 */
void AnalysePutKeyResponseForECC(jref keySetRef, PR8 bKeyCheckTag)
{
	u8					sendReponStart;
	u8					sendOutDataLen;
	PN8					keySetObjcet;
#define 						INIT_VALUES				    		0x00
#if (EUICC_ONLY||SE_AND_EUICC)
	if(bRamBuffer[PROFILE_APDU_MODE] == TRUE)//profile 鍐呴儴鎸囦护涓嶅彂鍝嶅簲
	{
		return;
	}
#endif
	sendOutDataLen		=	INIT_VALUES;
	sendReponStart		=	OFFSET_CDATA;
	keySetObjcet		=	MEMtPN8(locateObjectInMemory(keySetRef));

	APDUBuffer[sendReponStart]     =	keySet_getKeyVersion(keySetObjcet);
	U8_INC(sendOutDataLen);

	#if (EUICC_ONLY||SE_AND_EUICC)
		if(bRamBuffer[PROFILE_APDU_MODE] == TRUE)//profile 鍐呴儴鎸囦护涓嶅彂鍝嶅簲
		{
			return;
		}
	#endif

	apdu_ApduSetOutgoingAndSend(U8t16(sendReponStart), U8t16(sendOutDataLen));

}
#endif // GP__KEY_MANAGER__ECC_KEY

//modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
void AnalysePutKeyResponse(jref keySetRef, PR8 bKeyCheckTag){

	u8 					i,	j, keyCheckTagOffest;
	u8					bKeyType, mode;
	u8					sendReponStart;
	u8					sendOutDataLen;
	u8 					getKeyIdNum;
	u8					getKeyCheckTag;
	u16					bKeyLen;
	//memref				getKeyData;
	u8 		X_A		        getKeyData[RSA_KEY_LENGTH];
	PN8					keySetObjcet;

	#define				DES_KEY_VALUE				0x00
	#define 			INIT_VALUES				    0x00
	#define 			HIGHEST_ORDER_BYTES     	0x03
#if (EUICC_ONLY||SE_AND_EUICC)
	if(bRamBuffer[PROFILE_APDU_MODE] == TRUE)//profile 内部指令不发响应
	{
		return;
	}
#endif
	mode		   		=   ECB_Encrypt;
	sendOutDataLen		=	INIT_VALUES;
	sendReponStart		=	OFFSET_CDATA;
	j									=	INIT_VALUES;
	i									= U8_AND_U8(APDUBuffer[OFFSET_P2], MAX_KEY_ID);
	keyCheckTagOffest	=	INIT_VALUES;
	keySetObjcet			=	MEMtPN8(locateObjectInMemory(keySetRef));
	getKeyCheckTag		=	PX_R_U8(bKeyCheckTag);
	getKeyIdNum			  =	keySet_getKeyNum(keySetObjcet);

	APDUBuffer[sendReponStart]     =	keySet_getKeyVersion(keySetObjcet);
	U8_INC(sendOutDataLen);
	for(; i<U16t8(U8_A_U8(getKeyIdNum, U8_AND_U8(APDUBuffer[OFFSET_P2], MAX_KEY_ID))); U8_INC(i))
	{
		if(MORE_COMMAND_LAST_TAG == g_putKeyCommands)
		{
			if(!getKeyCheckTag)
			{
				break;
			}
		}
		else if(!U8_AND_U8(APDUBuffer[OFFSET_P2], PUT_MULTIPLE_KEY_MASK))
		{
			if(getKeyCheckTag)
			{
// 						i = 	(APDUBuffer[OFFSET_P2] & MAX_KEY_ID);
				getKeyIdNum		=	INIT_VALUES;
			}
			else
			{
				break;
			}
		}
		else if(!U8_AND_U8(getKeyCheckTag, U8_SHL_U8(1, keyCheckTagOffest)))
		{
			U8_INC(keyCheckTagOffest);
			continue;
		}

		U8_INC(keyCheckTagOffest);
		bKeyType	=	keySet_getKeyType(keySetObjcet, i,THROW_ERROR);

		bKeyLen		=	keyData_getKeyDataById(keySetObjcet, i, bKeyType, getKeyData);
//		if(getKeyData == MEM_NULL)
//		{
//			throw_error(SW_WRONG_DATA);
//		}

		if( ((KEY_DES_TYPE	<=	bKeyType) && (KEY_DES_CBC_MODE >= bKeyType))
#if		GP__AMENDMENT__B__FOR_MASK
			|| ((KEY_SCP_81_TYPE == bKeyType) && (0x01 != i))
#endif
		)
		{
			ecpMem_Set(TEMPBuffer, INIT_VALUES, DES_KEY_LENGTH_8);
			if(8 == bKeyLen)
			{
				mode   = U8_OR_U8(mode, 0x40);
			}
			else if(0x18 == bKeyLen)
			{
				mode   = U8_OR_U8(mode, 0x80);
			}
			Triple_DES(MEMtPX8( TEMPBuffer), MEMtPN8(getKeyData), MEMtPX8(TEMPBuffer), DES_KEY_LENGTH_8, MEMtPX8(bTempArr), mode, SENSITIVE_DATA_MASKED);
		}
		else if(KEY_AES_TYPE	==	bKeyType)
		{
			ecpMem_Set(TEMPBuffer, 0x01, KEY_CHECK_LENGTH_16);
#if VM_WITH_AES  //it shall directly invoke AesCal ,but not Api.
			//SymCipherDofinal(MEMtPX8(TEMPBuffer), KEY_CHECK_LENGTH_16, MEMtPX8(TEMPBuffer), getKeyData, bKeyLen*8, AES_ECB_MODE_MASK, MODE_ENCRYPT);
			SymCipherDofinal(MEMtPM8(TEMPBuffer), KEY_CHECK_LENGTH_16, MEMtPM8(TEMPBuffer), MEMtPM8(getKeyData), U16_M_U16(bKeyLen, 8), MEMtPM8(NULL), NULL, ALG_AES_BLOCK_128_ECB_NOPAD, U8_OR_U8(SYM_ECB_MODE,  MODE_ENCRYPT), 0x00, ALG_AES);
#endif
		}
		else if(KEY_SCP_81_TYPE ==	bKeyType)
		{
			HASHDoFinal(MEMtPM8(getKeyData), bKeyLen, MEMtPM8(TEMPBuffer), HASH_INIT, MEM_NULL, MEM_NULL, ALG_SHA_1);
		}

		ecpMem_Copy(&APDUBuffer[U8_A_U8(U8_A_U8(OFFSET_CDATA, 1), j)], MEMtPX8(TEMPBuffer), HIGHEST_ORDER_BYTES);
		//j					   =	i;
		j					   =	U16t8(U8_A_U8(j, HIGHEST_ORDER_BYTES));
		sendOutDataLen		   =	U16t8(U8_A_U8(sendOutDataLen, HIGHEST_ORDER_BYTES));
	}

	apdu_ApduSetOutgoingAndSend(U8t16(sendReponStart), U8t16(sendOutDataLen));
	
}
/*
 * merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
 */
u8 keyTypeValid(u8 keyType)
{
	if((keyType >= KEY_DES_TYPE)&&(keyType <= KEY_SCP_81_TYPE)) // 80-85
	{

	}
	else if(keyType == KEY_AES_TYPE) // 0x88
	{

	}
	else if((keyType == 0x90) || (keyType == 0x91))
	{

	}
	else if((keyType >= RSA_E_DGI_TYPE)&&(keyType <= RSA_CRT_DQ_DGI_TYPE)) // 0xA0-0xA8
	{

	}
	else if(((keyType >= ECC_PublicKey_DGI_TYPE)&&(keyType <= ECC_CurvePara_K_DGI_TYPE)) || (keyType == ECC_KEYPARAREF_DGI_TYPE)) // 0xB0-0xB7 0xF0
	{

	}
	else
	{
		return false;
	}

	return true;
}
/*
*@function   :CheckKeyTypeValid
*
*@description:determine current put key command valid or not according to keepKeyType and keyType
*
*see         :GP2.3.1

*@parameter  :u8 keepKeyType --- the first key type in the current put key command
*@parameter  :u8 keyType     --- the current key type that is being analysed
*
*@return     :true: if keyType is permitted;  otherwise false;
*
*@history    :V1.0:create by xuxiaojun@20210513
*/
u8 CheckKeyTypeValid(u8* keepKeyType, u8 keyType)
{
	if(keyTypeValid(keyType) == false)
	{
		return false;
	}

	if(*keepKeyType == 0x00)
	{
		*keepKeyType = keyType;

		return true;
	}

	if(keyType != *keepKeyType) // 多支key的类型不一致时，哪些情况是合法的
	{
		if(KEY_SCP_81_TYPE == *keepKeyType) //80D800812F40 85 11 10 07F8E377DF416CDF9CE0A123F9A6DB8603438916 88 11 10 07F8E377DF416CDF9CE0A123F9A6DB8603F2A8DF00
		{
			if ((KEY_DES_TYPE == keyType) || (KEY_AES_TYPE == keyType))
			{
				return true;
			}
		}
#if GP__KEY_MANAGER__ECC_KEY
		else if((*keepKeyType == ECC_PublicKey_DGI_TYPE) || (*keepKeyType == ECC_PrivateKey_DGI_TYPE)) // B0 / B1
		{
			if(keyType == ECC_KEYPARAREF_DGI_TYPE) // B0/B1 + F0
			{
				return true;
			}
			else if((keyType >= ECC_CurvePara_P_DGI_TYPE)&&(keyType <= ECC_CurvePara_K_DGI_TYPE)) // B0/B1 + B2-B7
			{
				return true;
			}
		}
		else if(*keepKeyType == ECC_KEYPARAREF_DGI_TYPE)// F0
		{
			if((keyType >= ECC_CurvePara_P_DGI_TYPE)&&(keyType <= ECC_CurvePara_K_DGI_TYPE)) // F0 + B2-B7
			{
				return true;
			}
		}
		else if((*keepKeyType >= ECC_CurvePara_P_DGI_TYPE)&&(*keepKeyType <= ECC_CurvePara_K_DGI_TYPE))
		{
			if((keyType >= ECC_CurvePara_P_DGI_TYPE)&&(keyType <= ECC_CurvePara_K_DGI_TYPE)) // B2-B7
			{
				return true;
			}
		}
#endif

#if GP__KEY_MANAGER__PUT_RSA_KEY
		else if(RSA_N_DGI_TYPE == *keepKeyType)
		{
			if((RSA_E_DGI_TYPE == keyType) || (RSA_D_DGI_TYPE == keyType))
			{
				return true;
			}
		}
		else if(RSA_PRIVATE_N_DGI_TYPE == *keepKeyType)// RSA Private Key
		{
			if(RSA_D_DGI_TYPE == keyType)
			{
				return true;
			}
		}
		else if(RSA_CRT_P_DGI_TYPE == *keepKeyType)//
		{
			if((keyType >= RSA_CRT_Q_DGI_TYPE)&&(keyType <= RSA_CRT_DQ_DGI_TYPE))
			{
				return true;
			}
		}
#endif
		return false;
	}
	else
	{
		return true;
	}
}

#if GP__KEY_MANAGER__ECC_KEY
/*
*@function   :ProcessEccKey
*
*@description:g_keyDataOffest point to Key Component Block when this function is called
*
*@param      :u8 keyType                --- 当前正在处理的key的Type
*@param      :u16 SingleKeyComponentLen --- 当前正在处理的key的长度
*@param      :PN8 sdDataObj             --- 当前正在处理的key所属的sd
*@param      :u8 curKeyVersion          --- p1中所指示的key version
*@param      :u8 newKeyVersion          --- recdata[0]中所指示的key version
*
*@return     :memref                    --- 返回当前这支key所属的key set引用
*
*@history    :v1.0:create by xuxiaojun@20210518
*/
memref ProcessEccKey(u8 keyType, u16 SingleKeyComponentLen, PN8 sdDataObj, u8 curKeyVersion, u8 newKeyVersion)
{
	jref				keySetRef;
	u16                 keyLen;
	memref              keyDataAfield;
	u8                  status;

	keyLen = 0x00;
	status = 0x00;

	keySetRef =  keySet_getKeySet(sdDataObj, newKeyVersion);

	if(curKeyVersion == 0x00)
	{
		status = TAG_NEW_KEY;
	}
	else if(newKeyVersion != curKeyVersion) // update key version
	{
		status = TAG_UPDATE_KEY_VERSION;
	}
	else // update key value
	{
		status = TAG_UPDATE_KEY;
	}

	if((keyType >= ECC_CurvePara_P_DGI_TYPE) && (keyType <= ECC_CurvePara_B_DGI_TYPE))// B2-B4
	{
		if(ValueBit(g_markEccKey, ECC_CurvePara_P_BIT2 + (keyType - ECC_CurvePara_P_DGI_TYPE)))
		{
			keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
		}
		else
		{
			SetBit(g_markEccKey, ECC_CurvePara_P_BIT2 + (keyType - ECC_CurvePara_P_DGI_TYPE));

			if(g_EccKeyParaLen == 0x00)
			{
				g_EccKeyParaLen = SingleKeyComponentLen; // 这里是否需要对所支持的SingleKeyComponentLen进行判断

				// 如果是update key时，SingleKeyComponentLen需要跟之前储蓄的key len一致

				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, 0x00, SingleKeyComponentLen, true); // B2
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, SingleKeyComponentLen, false); // B3
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, SingleKeyComponentLen, false); // B4
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, SingleKeyComponentLen*2 + 1, false); // B5
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, SingleKeyComponentLen, false); // B6
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, SingleKeyComponentLen, false); // B7
			}
			else if(g_EccKeyParaLen != SingleKeyComponentLen)
			{
				keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
			}
		}

		g_PutEccKey = true;

	}
	else if(keyType == ECC_CurvePara_G_DGI_TYPE) // B5
	{
		if(ValueBit(g_markEccKey, ECC_CurvePara_P_BIT2 + (keyType - ECC_CurvePara_P_DGI_TYPE)))
		{
			keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
		}
		else
		{
			SetBit(g_markEccKey, ECC_CurvePara_P_BIT2 + (keyType - ECC_CurvePara_P_DGI_TYPE));

			if(g_EccKeyParaLen == 0x00)
			{
				g_EccKeyParaLen = (SingleKeyComponentLen-1) / 2; // 这里是否需要对所支持的SingleKeyComponentLen进行判断

				// 如果是update key时，SingleKeyComponentLen需要跟之前储蓄的key len一致

				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, 0x00, g_EccKeyParaLen, true);    // B2
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, g_EccKeyParaLen, false); // B3
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, g_EccKeyParaLen, false); // B4
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, g_EccKeyParaLen*2 + 1, false);       // B5
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, g_EccKeyParaLen, false); // B6
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, g_EccKeyParaLen, false); // B7
			}
			else if(g_EccKeyParaLen != ((SingleKeyComponentLen-1)/2))
			{
				keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
			}
		}

		g_PutEccKey = true;
	}
	else if(((keyType >= ECC_CurvePara_N_DGI_TYPE) && (keyType <= ECC_CurvePara_K_DGI_TYPE)))// B6, B7
	{
		if(ValueBit(g_markEccKey, ECC_CurvePara_P_BIT2 + (keyType - ECC_CurvePara_P_DGI_TYPE)))
		{
			keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
		}
		else
		{
			SetBit(g_markEccKey, ECC_CurvePara_P_BIT2 + (keyType - ECC_CurvePara_P_DGI_TYPE));

			if(g_EccKeyParaLen == 0x00)
			{
				g_EccKeyParaLen = SingleKeyComponentLen; // 这里是否需要对所支持的SingleKeyComponentLen进行判断

				// 如果是update key时，SingleKeyComponentLen需要跟之前储蓄的key len一致

				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, 0x00, g_EccKeyParaLen, true); // B2
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, g_EccKeyParaLen, false); // B3
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, g_EccKeyParaLen, false); // B4
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, g_EccKeyParaLen*2 + 1, false); // B5
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, g_EccKeyParaLen, false); // B6
				keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyLen, g_EccKeyParaLen, false); // B7
			}
			else if(g_EccKeyParaLen != SingleKeyComponentLen)
			{
				keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
			}
		}

		g_PutEccKey = true;

	}
	else if((keyType == ECC_PublicKey_DGI_TYPE) || (keyType == ECC_PrivateKey_DGI_TYPE)) // B0 B1
	{
		if(ValueBit(g_markEccKey, ECC_PublicKey_BIT1))
		{
			keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
		}
		else
		{
			SetBit(g_markEccKey, ECC_PublicKey_BIT1); // 这里是否需要对所支持的公钥私钥长度进行判断

			// 如果是update key时，SingleKeyComponentLen需要跟之前储蓄的key len一致

			keyLen = keyData_computeKeyDataLen(KEY_FORMAT_03, 0x00, SingleKeyComponentLen, true);
		}
#if GP__SCP__SCP11__FOR_MASK
		if (IsSCP11Key(newKeyVersion))
		{
			SCP11InitKey(sdDataObj, curKeyVersion,newKeyVersion, (APDUBuffer[APDU_P2_OFFSET] & 0x7F), keyType, status);
		}
#endif
		g_PutEccKey = true;
	}
	else if(keyType == ECC_KEYPARAREF_DGI_TYPE) // F0
	{
		if(ValueBit(g_markEccKey, ECC_KEYPARAREF_BIT0)) // 只允许出现1次
		{
			keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
		}
		else
		{
			SetBit(g_markEccKey, ECC_KEYPARAREF_BIT0);
		}
		if(SingleKeyComponentLen == 1)
		{
			g_EccKeyParaRef = APDUBuffer[g_keyDataOffest];
		}
		else if(SingleKeyComponentLen == 2)
		{
			g_EccKeyParaRef = PX_R_U16(&APDUBuffer[g_keyDataOffest]);
		}
		else
		{
			keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
		}

		// 要判断g_EccKeyParaRef曲线在卡上存不存在

		g_PutEccKey = true;

		return keySetRef;
	}
	else
	{
		return keySetRef;
	}

	if(REF_NULL == keySetRef) // add key set
	{
		if(keyLen != 0x00) // 当前解析到 B0 / B1 / B2 - B7:  B0+F0 或者F0+B2-B7
		{
			keySetRef = newKeySetByKeyVersion(sdDataObj, KEY_FORMAT_03, curKeyVersion, newKeyVersion, keyLen); // key version 和   key format在函数内部被写入

			if(ValueBit(g_markEccKey, ECC_KEYPARAREF_BIT0))
			{
				keyDataAfield = keyData_prepareKeyData(MEMtPN8(locateObjectInMemory(keySetRef)), TAG_NEW_KEY, ECC_DOMAIN_KID, genKeyType(keyType), SingleKeyComponentLen);
			}
			else
			{
				keyDataAfield = keyData_prepareKeyData(MEMtPN8(locateObjectInMemory(keySetRef)), TAG_NEW_KEY, g_PutKeyId, genKeyType(keyType), SingleKeyComponentLen);
			}

			SO_storeSecData(MEMtPX8(APDUBuffer), g_keyDataOffest, SingleKeyComponentLen,  MEMtPM8(keyDataAfield));

			memRefresh();
		}
		else // 当前解析到F0
		{

		}
	}
	else  // B0+B2-B7的B2-B7 或者是update key的B0
	{
		if(status == TAG_NEW_KEY)
		{
			if((keySet_getKeyType(MEMtPN8(locateObjectInMemory(keySetRef)), g_PutKeyId, NO_ERROR) != genKeyType(keyType))&&(keySet_getKeyExtDataRef(MEMtPN8(locateObjectInMemory(keySetRef))) == REF_NULL)) //只会出现在 B0 / B1 + B2-B7情况下
			{
				keyData_addKeyDataSpace(MEMtPN8(locateObjectInMemory(keySetRef)), keyLen);

				keyDataAfield = keyData_prepareKeyData(MEMtPN8(locateObjectInMemory(keySetRef)), status, g_PutKeyId, keyType, SingleKeyComponentLen);

				SO_storeSecData(MEMtPX8(APDUBuffer), g_keyDataOffest, SingleKeyComponentLen,  MEMtPM8(keyDataAfield));

				memRefresh();
			}
			else //可能是，也可能是B0 / B1 + B2-B7的B2 - B7,需要先判断B2-B7是在keydata[]中还是在KeyExtraData中
			{
				if(ValueBit(g_markEccKey, ECC_KEYPARAREF_BIT0))// F0 + B2-B7的B2-B7, 此时keyDataAfield是keydata[]
				{
					keyDataAfield = keyData_prepareKeyData(MEMtPN8(locateObjectInMemory(keySetRef)), status, g_PutKeyId, keyType, SingleKeyComponentLen);
				}
				else // B1 + B2-B7的B2 - B7,此时keyDataAfield是KeyExtraData
				{
					keyDataAfield = keyData_prepareKeyData(MEMtPN8(locateObjectInMemory(keySetRef)), status, ECC_DOMAIN_KID, keyType, SingleKeyComponentLen);
				}

				SO_storeSecData(MEMtPX8(APDUBuffer), g_keyDataOffest, SingleKeyComponentLen,  MEMtPM8(keyDataAfield));

				memRefresh();
			}
		}
		else if(status == TAG_UPDATE_KEY)
		{
			if(ValueBit(g_markEccKey, ECC_CurvePara_P_BIT2))
			{
				keyDataAfield = keyData_prepareKeyData(MEMtPN8(locateObjectInMemory(keySetRef)), status, ECC_DOMAIN_KID, keyType, SingleKeyComponentLen);
			}
			else
			{
				keyDataAfield = keyData_prepareKeyData(MEMtPN8(locateObjectInMemory(keySetRef)), status, g_PutKeyId, keyType, SingleKeyComponentLen);
			}

			SO_storeSecData(MEMtPX8(APDUBuffer), g_keyDataOffest, SingleKeyComponentLen,  MEMtPM8(keyDataAfield));

			memRefresh();
		}
		else
		{
			keySet_replaceKeyVersion(MEMtPN8(locateObjectInMemory(keySetRef)), newKeyVersion);
		}
	}
	return keySetRef;
}
#endif // GP__KEY_MANAGER__ECC_KEY
/*
 * merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 去掉未使用的参数AnalyseType, KeySaveMode
 */
static jref AnalyseKeyComponent(u8 KeyFormat, u8 curKeyVersion, u8 newKeyVersion, PN8 sdDataObj, PR8 bKeyCheckTag,u8  encryptkeyType)
{

			u8					bMultipleKeys_;
			u8					bKeyIdStart;
			u8					keyCompNum;
			u8					CheckValueLen;
			u8					KeyUsageLen;
			u8					KeyTypeLen;
			u8					IshasKeyUsage;
			u8					KeyType;
			u8					keyTagStart;
			u8					KeyAccessLen;
			u8					backupKeyDataOff;
			u16					SingleKeyComponentLen;
			u8					SingleKeyPaddingLen;
			u16					KeySetRef;
			u8					aesMacLen;
			u8                  keepKeyType;
			jref                nextKeyDataRef;
			u16                 keyDataLen;

			#define 			INIT_VAULE0							0x00
			#define 			PUT_KEY_KEY_TYPE_LENGTH				0x01

#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
			u16                 tempKeyLen;

			tempKeyLen							=	INIT_VAULE0;
#endif

			KeyType								=	INIT_VAULE0;
			IshasKeyUsage						=	false;
			KeySetRef							=	REF_NULL;
			backupKeyDataOff					=	INIT_VAULE0;
			SingleKeyComponentLen				= 	INIT_VAULE0;
			SingleKeyPaddingLen                 = 	INIT_VAULE0;
			CheckValueLen						=	INIT_VAULE0;
			KeyUsageLen							=	INIT_VAULE0;
			KeyAccessLen						=	INIT_VAULE0;
			keyCompNum							= 	INIT_VAULE0;
			KeyTypeLen							=	PUT_KEY_KEY_TYPE_LENGTH; // 0x01
			bMultipleKeys_						=	APDUBuffer[OFFSET_P2];
			bKeyIdStart							=	g_PutKeyId;		
			keyTagStart		  					=	g_keyDataOffest;
			aesMacLen							=	INIT_VAULE0;
			keepKeyType							=	INIT_VAULE0;

			while(g_PutKeyRemaindLen > 0)
			{
				  switch(g_putKeyCurrState)
				  {
						case PUT_KEY_GET_TAG:
						{
							if(g_PutKeyRemaindLen < U8t16(KeyTypeLen))
							{
								keyManager_abortLogicManagerThorwError(SW_WRONG_LENGTH);
							}
								
							if(MORE_COMMAND_FIRST_TAG == g_putKeyCommands)
							{
								if((U8_AND_U8(bMultipleKeys_, PUT_MULTIPLE_KEY_MASK) != PUT_MULTIPLE_KEY_MASK)
										&& (g_PutKeyId > bKeyIdStart))
								{
										keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
								}
							}
							SingleKeyPaddingLen = 0;
								aesMacLen = 0; //bug 516
							KeyType = APDUBuffer[g_keyDataOffest];

							if((IsSCP81Key(newKeyVersion))||(IsSCP80Key(newKeyVersion)))
							{
								keyTagStart = g_keyDataOffest;
							}

							if((KeyType == EXTENDED_FROMAT) || (newKeyVersion == KeyType))// 第2支KEY的起始，可能是KeyVersion, 可能是keyType eg:D800812F 40 85111007F8E377DF416CDF9CE0A123F9A6DB8603438916 40 801007F8E377DF416CDF9CE0A123F9A6DB8603A4B7D600
							{
								if(KeyType == EXTENDED_FROMAT)
								{
									IshasKeyUsage  =  true;
								}
								else
								{
									U8_INC(keyTagStart); //newKeyVersion == KeyType，这种格式目前只有scp81的出现过这种格式
								}
								U8_INC(g_keyDataOffest);
								KeyType	       	   =  APDUBuffer[g_keyDataOffest];
#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
								if(!keepKeyType)
								{
													 keepKeyType		 =	KeyType;
								}
								else
								{
									 if(KeyType != keepKeyType)
									 {
										keyManager_abortLogicManagerThorwError(SW_ALOG_NOT_SUPPORT);
									 }
								}
#endif
								g_keyDataOffest	   =  U16t8(U8_A_U8(g_keyDataOffest, KeyTypeLen)); //g_keyDataOffest point to Length of Key Component Block
								U8_INC(KeyTypeLen);
	//								U8_INC(keyTagStart); //会造成keyType拓展字段FF偏移错误，此处要注释掉，测试用例：eSIM_Test_RSP\TY_OfflineProfileFormatTest\Security domains\正常案例\Security_domains_N09
							}
							else
							{
								g_keyDataOffest	   =  U16t8(U8_A_U8(g_keyDataOffest, KeyTypeLen)); //g_keyDataOffest point to Length of Key Component Block
							}

#if	GP__VERSION__FOR_IRAN
							if((KEY_DES_ECB_MODE == KeyType) || (KEY_DES_CBC_MODE == KeyType))
							{
								keyManager_abortLogicManagerThorwError(0x9484);
							}
#endif

							if(false == CheckKeyTypeValid(&keepKeyType, KeyType))
							{
								keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
							}

#if	 GP__VERSION__FOR_CHINA_UNICOM_MASK

							if((0x83 == KeyType) || (0x84 == KeyType))
							{
								keyManager_abortLogicManagerThorwError(SW_CONDITIONS_NOT_SATISFIED);
							}
#endif
							g_putKeyCurrState   	  	   =  U8t16(AnalyseKeyType(KeyType, g_putKeyCurrState));
							g_PutKeyRemaindLen       	   =  U16_S_U8(g_PutKeyRemaindLen, KeyTypeLen);

							if(IsRSAKeyType(KeyType)//SCP03时   (encryptkeyType == KEY_AES_TYPE)，会做LLV的判断，由于RSA ECC的长度不一定是8的整数倍 会导致正常数据结构被当成LLV  所以在这里做了限制
#if  GP__KEY_MANAGER__ECC_KEY
								||IsECCKeyType(KeyType)
#endif
							)//非对称秘钥 暂时不支持LLV结构 不做判断
							{
								if(KeyType == ECC_PrivateKey_DGI_TYPE)//modify by xuxiaojun@20220629 ALG_EC_FP_KEY_521的私钥长度是0x42，不是块的整数呗，密文需要填重，所以其格式是LLV
								{
									if((U8_R_U8(APDUBuffer[g_keyDataOffest], 0x10)) != 0x00)//第一个L不是块的整数倍，表示其是LLV格式
									{
										SingleKeyPaddingLen = U8_S_U8(APDUBuffer[g_keyDataOffest], APDUBuffer[g_keyDataOffest+1]);
										U8_DEC(SingleKeyPaddingLen); // LLV结构第一个L包括了第二个L字段
										U8_INC(g_keyDataOffest);
										U16_DEC(g_PutKeyRemaindLen);
									}
								}
								if(U8_AND_U8(bMultipleKeys_, PUT_MULTIPLE_KEY_MASK) == PUT_MULTIPLE_KEY_MASK) // P2.b8 shall be set to 0 ("single key"), otherwise a response of '6A86' shall be returned.
								{
									keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
								}
							}
							else
							{	
								//AES LLV: <ts_102226v120000p.pdf>  session 8.2.1.5.1, eg: 80D8008147 01 8811 10 11223344556677889910111213141516 03 000000 881210 1122334455667788991011121314151608 03000000 881110 11223344556677889910111213141516 03000000(61xx/9000) ;ISDR SCP 80key
								//PSK LLV:  <GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf>   session 3.9.1,  eg: 80D8000118 41 85 11 01 88000000000000000000000000000000 03000000
								//GP2.3.1, DES also may use LLV format  in  GP2.3.1 UL test
								if( (IsSCP81Key(newKeyVersion)) || (encryptkeyType == KEY_AES_TYPE) ||((KEY_AES_TYPE == KeyType)/*&&(newKeyVersion <= SCP_MAX_VERSION)*/))// 参考附录D  AES key,eg:格式：88 11 10 11223344556677889910111213141516 || PSK key,eg:格式：85 21 20 F0C0FAAC0EF1364A3E5EB4229CF797A3752CD0C8277844576B3E05D505A03F21
								{
									//当key格式是88 11 10 11223344556677889910111213141516才要偏移， 如果是 88 10 11223344556677889910111213141516则不用往后偏移1
									//if ((APDUBuffer[g_keyDataOffest] == 0x11) || (APDUBuffer[g_keyDataOffest] == 0x19) || (APDUBuffer[g_keyDataOffest] == 0x21))
									if ((APDUBuffer[g_keyDataOffest] != 0x10) && (APDUBuffer[g_keyDataOffest] != 0x18) && (APDUBuffer[g_keyDataOffest] != 0x20))
									{
										SingleKeyPaddingLen = U8_S_U8(APDUBuffer[g_keyDataOffest], APDUBuffer[g_keyDataOffest+1]);
										U8_DEC(SingleKeyPaddingLen); // LLV结构第一个L包括了第二个L字段
//										if(U8_R_U8(APDUBuffer[g_keyDataOffest] - 1, 0x10) == 0x00)// mac  not  exist
//										{
//											aesMacLen = 0;
//										}
//										else if(U8_R_U8(APDUBuffer[g_keyDataOffest] - 2, 0x10) == 0x00) // mac  exist  and length is correct
//										{
//											aesMacLen = 1;
//										}
//										else
//										{
//											//keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);	//Ҫ��Ҫ���쳣
//										}

										//modify by zhxx@20230209,电信5G测试：11.Telecom__基本通信功能/20.CTCC_Test_工程集-国密SWP/5G补充RFM/scp80_mac_cipher/T_192bits_4MAC_CounterAvailable_PORSubmit_CC，T_192bits_8MAC_CounterOneHigher_PORSubmit_CC
										if(((APDUBuffer[g_keyDataOffest] - 1) == 0x10)||((APDUBuffer[g_keyDataOffest] - 1) == 0x18) || ((APDUBuffer[g_keyDataOffest] - 1) == 0x20))		// mac  not  exist, modify by xuxiaojun@20220306 电信研究院测试本地APDU执行put aes 192长度key时，密文长度不是0x10的整数倍  0x19 + 0x18 + V
										{
											aesMacLen = 0;
										}
										else if(((APDUBuffer[g_keyDataOffest] - 2) == 0x10)||((APDUBuffer[g_keyDataOffest] - 2) == 0x18) || ((APDUBuffer[g_keyDataOffest] - 2) == 0x20)) // mac  exist  and length is correct, modify by xuxiaojun@20220306 电信研究院测试本地APDU执行put aes 192长度key时，密文长度不是0x10的整数倍
										{
											aesMacLen = 1;
										}
										else
										{
											//keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);	//要不要抛异常
										}
										U8_INC(g_keyDataOffest);
										U16_DEC(g_PutKeyRemaindLen);
									}
									else
									{
										aesMacLen = 0;  //bug 516
									}

								}
							}

							break;
						}
						case	PUT_KEY_GET_LENGTH:
						{
								//currKeyDataLen: KeyCompoment 婵傚倹瀚㈤弻锟�								SingleKeyComponentLen  	   =  AnalyseKeyLength(g_isNewKey, KeyFormat);
								SingleKeyComponentLen  	   =  AnalyseKeyLength(g_isNewKey, KeyFormat/*,KeyType*/);
#if  GP__KEY_MANAGER__ECC_KEY
								checkEccKeyLen(SingleKeyComponentLen,KeyType);
#endif
								if((IsSCP02Key(newKeyVersion))&&(SingleKeyComponentLen < 0x10))//GP功能补充测试脚本_接触_工程集/雅斯拓/密钥长度/6.1.9  PUT SCP02 KEY 秘钥长度为8
								{
									keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
								}
//								if(KEY_SCP_81_TYPE == KeyType)
//								{
//										SingleKeyComponentLen  	   =  AnalyseKeyLength(g_isNewKey, KeyFormat);
//								}
#if	 GP__VERSION__FOR_CHINA_UNICOM_MASK
								if((0x08 == SingleKeyComponentLen) && (0x80 == KeyType))
								{
										keyManager_abortLogicManagerThorwError(SW_CONDITIONS_NOT_SATISFIED);				
								}
#endif

#if		GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
								else if((KEY_DES_TYPE	<= KeyType) && (KEY_DES_CBC_MODE >= KeyType))
								{
										if((tempKeyLen != SingleKeyComponentLen) && (tempKeyLen))
										{
												keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
										}
								}
#endif
								
								if(IsRSAKeyType(KeyType))
								{
										if((RSA_N_DGI_TYPE == KeyType) && (0x80 != SingleKeyComponentLen))
										{
												keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
										}
										if(RSA_E_DGI_TYPE	==	KeyType)
										{
												
#if	 GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK //鐠愶拷
											if(g_PutRsaTokenKey != 0x01)
											{
													keyManager_abortLogicManagerThorwError(SW_CONDITIONS_NOT_SATISFIED);
											}
#else
											if((EXPONENT_LEN_BYTE1 != SingleKeyComponentLen) && (EXPONENT_LEN_BYTE3 != SingleKeyComponentLen))
											{
													keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
											}
#endif														
										}
								}
								//currKeyDataLen		   	   =  U32t16(U16_A_U16(currKeyDataLen, SingleKeyComponentLen));
#if	 GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
								tempKeyLen               =  SingleKeyComponentLen;
#endif
								U16_INC(g_putKeyCurrState);

								break;
						}
						case	PUT_KEY_GET_VALUES:
						{
														
								//AnalyKeyTypeAndLen(KeyType, SingleKeyComponentLen);
								if(((KEY_DES_TYPE	<=	KeyType) && (KEY_DES_CBC_MODE >= KeyType)) || (KEY_AES_TYPE == KeyType)
#if GP__AMENDMENT__B__FOR_MASK
									|| (KEY_SCP_81_TYPE	==	KeyType)
#endif								
#if GP__KEY_MANAGER__ECC_KEY
									||(ECC_PrivateKey_DGI_TYPE == KeyType)  // modify by xuxiaojun@20220625 ECC private key  Data Content Encrypted
#endif
								)
								{
#if GP__SCP
#if (EUICC_ONLY||SE_AND_EUICC)
									if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)	//PROFILE下载过程拼装的指令都是明文 不需要解密
#endif
									{
										if ((JcvmConfig_GetSEState() > CARD_OP_READY)||(channelInfo.security_level[selected_channel] != SCF_NO_AUTHENTICATED))// CARD_OP_READY并且没有鉴权时 明文putkey ，增加此判断是因为AESkey无法通过本地的方式预置 ，scp02鉴权用的是DES 无法保护更高级别的AESkey
										{
											//The field "length of the MAC" shall be present if "ciphered key" contains an AES key with key identifier '02' and key version '01' to '0F' or '11'  see <ts_102226v130000p.pdf> 8.2.1.5.1  PUT KEY for AES 
											//if(((IsSCP80Key(newKeyVersion))||(newKeyVersion == 0x11))&&(KeyType == KEY_AES_TYPE)&&(g_PutKeyId == 0x02))
						
											decryptKeyDataByDEK(g_keyDataOffest, SingleKeyComponentLen+SingleKeyPaddingLen-aesMacLen,encryptkeyType);
										}
									}
#endif
									backupKeyDataOff   	 =   g_keyDataOffest;
								}
								
#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
								if(IsRSAKeyType(KeyType))
								{
										backupKeyDataOff   	 =   g_keyDataOffest;
								}
#endif

#if GP__KEY_MANAGER__ECC_KEY
								if(KeyFormat == KEY_FORMAT_03)	//非KEY_FORMAT_03时，KeySetRef在本函数还没有申请，如果此处KeySetRef为有效值，下一个casd将会保存KEY_FORMAT_02的keyUsage，后续将会重复写入导致flash状态出错(用例：下载profile过程的putkey流程)
								{
									KeySetRef = ProcessEccKey(KeyType, SingleKeyComponentLen, sdDataObj, curKeyVersion, newKeyVersion); // 璇ュ嚱鏁颁腑浼氭墽琛宯ewKeySetByKeyVersion
								}
#endif
								g_PutKeyRemaindLen  =  U16_S_U8(g_PutKeyRemaindLen, SingleKeyComponentLen);
								g_PutKeyRemaindLen = U16_S_U8(g_PutKeyRemaindLen,SingleKeyPaddingLen);

									if(g_PutKeyRemaindLen == 0x00)//merged from EPX\v2.1\BRANCH_MODULE\FINANCE_TMC_THD89: Length of key check value is Mandatory Item
									{
										if((CONFIG_VGP)&&(newKeyVersion == KEY_DAP_VERIF_ID1)&&(KeyType == RSA_KEY_TYPE_MIN)) //modify by xuxiaojun@20220621 Visa GlobalPlatform 2.1.1 Card Implementation Requirements v2.0.pdf规范中，DAP的RSA类型不带有check value Table 14 - Public Key Data Field Structure
										{
											KeySetRef =  newKeySetByKeyVersion(sdDataObj, KeyFormat, curKeyVersion, newKeyVersion, g_PutkeyDataLen);

											setKeyDataForKeyIdUnit(keyTagStart, U8_S_U8(APDUBuffer[OFFSET_LC], 1), KeySetRef, curKeyVersion, newKeyVersion, g_PutKeyId, KeyFormat,encryptkeyType);
										}
										else
										{
											keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
										}
									}
									else
									{
										g_putKeyCurrState   =  U8t16(AnalyseKeyValues(U16t8(U16_A_U8(SingleKeyComponentLen,SingleKeyPaddingLen)), g_keyDataOffest, g_putKeyCurrState));
										g_keyDataOffest	  	=  U32t8(U8_A_U16(g_keyDataOffest, U16_A_U8(SingleKeyComponentLen,SingleKeyPaddingLen)));

										g_isNewKey					=  false;

										if((g_putKeyCurrState != PUT_KEY_CHECK_VALUES)&&(keepKeyType <= KEY_AES_TYPE))	// （RSA非对称秘钥）A0  A1	，非对称秘钥的checkvalue len = 0 且第一只秘钥不带checkvalue,其余（DES/AES）暂时统一做限制
										{
											keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
										}
									}
//								}
								
								U8_INC(keyCompNum);//keyComp瑜帮拷
								break;
						}
						case 	PUT_KEY_CHECK_VALUES:
						{
//								if(IsSCP80Key(newKeyVersion)&&(KeyType == KEY_AES_TYPE)&&(g_PutKeyId == 0x02))//profile规范，附录D ，102226 ：The field "length of the MAC" shall be present if "ciphered key" contains an AES key with key identifier '02' and key version '01' to '0F' or '11'
//								{
//									U8_INC(g_keyDataOffest);
//									U16_DEC(g_PutKeyRemaindLen);
//								}
								
								if(((KEY_DES_TYPE	<=	KeyType) && (KEY_DES_CBC_MODE >= KeyType)) || (KEY_AES_TYPE == KeyType)
#if GP__AMENDMENT__B__FOR_MASK
									|| (KEY_SCP_81_TYPE	==	KeyType)
#endif								
								)
								{
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK 
								  if(((KEY_DES_TYPE	<= KeyType) && (KEY_DES_CBC_MODE >= KeyType)) && (MAX_DES_KEY_LENGTH != SingleKeyComponentLen))
									{

									}else
#endif
									{
#if GP__SCP
											encryptKeyCheckValueByDEK(backupKeyDataOff, g_keyDataOffest, bKeyCheckTag, U8_S_U8(keyCompNum,1), KeyType, SingleKeyComponentLen);
#endif
									}
								}
								
#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
								if(IsRSAKeyType(KeyType))
								{
										encryptKeyCheckValueForRSA(backupKeyDataOff, g_keyDataOffest, (RAM_U8_P)bKeyCheckTag, U8_S_U8(keyCompNum,1), KeyType, SingleKeyComponentLen);
								}
#endif		
								CheckValueLen     	=  AnalyseKeyCheckOrUsageOrAccess(KeySetRef, g_PutKeyId);
								g_PutKeyRemaindLen  =  U16_S_U8(g_PutKeyRemaindLen, CheckValueLen);

								if((KeyFormat == KEY_FORMAT_03)&&(IsECCKeyType(KeyType) == 0x00)) // ECC的在前面已经处理
								{
									KeySetRef =  keySet_getKeySet(sdDataObj, newKeyVersion);

									if(REF_NULL == KeySetRef)
									{
										KeySetRef		  	=  newKeySetByKeyVersion(sdDataObj, KeyFormat, curKeyVersion, newKeyVersion, g_PutkeyDataLen);
										//g_KeepRefForSplit   =  KeySetRef;
									}
									else
									{
										if(MEM_NULL == FM03_locateKeyId(MEMtPN8(locateObjectInMemory(KeySetRef)), g_PutKeyId, TAG_GET_KEY, &nextKeyDataRef, &keyDataLen)) // modify by xuxiaojun@20220720: 对于update key时，不应该调用keyData_addKeyDataSpace.否则get data获取key ifno时，响应数据错误,且会生成无效的引用及空间。但是实际使用时，KEY值没有问题
										{
											keyData_addKeyDataSpace(MEMtPN8(locateObjectInMemory(KeySetRef)), g_PutkeyDataLen);
										}
									}
									g_isNewKey			=  true;
									g_PutkeyDataLen	  	=  INIT_VAULE0;

									if((IsSCP81Key(newKeyVersion))||(IsSCP80Key(newKeyVersion)))
									{
										//modify by xuxiaojun@20220824,第二个参数需要考虑SingleKeyPaddingLen的情况，EPX-测试列表\eUICC平台基本通信功能测试工程集\01_基本通信功能测试_03.sol\RFM_RAM_Test20190524效率版\putkey（RAM，远程安装ssd，putkey时）有测到
										/*	80D8008176 01
											8021 18 78AEBA1FACF28013BBDB02212FEAC7913297216B46575DC8F308100336D0666A 0339D6BB
											8021 18 98EBF24D8D19B51B72CD4E70A266F5725B410DBDAE69E4462509EECAEC6D1C95 03B7740B
											8021 18 50AB925E6FA67DDB93A58DB39B650218BFF2527345BC7F388CBB5A4576E8F7D4 03485961 00C0000000
											80D800816E 05 FF8811 10 1122334455667788991011121314151603000000 0118 0100 FF881A18 112233445566778899101112131415161122334455667788 08 03000000 0118 0100 FF882120 1122334455667788991011121314151611223344556677889910111213141516 030000000118 0100
										 * */
											setKeyDataForKeyIdUnit(keyTagStart, backupKeyDataOff - keyTagStart + CheckValueLen + SingleKeyComponentLen + SingleKeyPaddingLen, KeySetRef, curKeyVersion, newKeyVersion, g_PutKeyId, KeyFormat,encryptkeyType);
									}
									else
									{
										setKeyDataForKeyIdUnit(keyTagStart, U8_S_U8(APDUBuffer[OFFSET_LC], 1), KeySetRef, curKeyVersion, newKeyVersion, g_PutKeyId, KeyFormat,encryptkeyType);
									}
								}

								//keyId娑撹桨缍卬ew KeySet
//								if(KeyFormat == KEY_FORMAT_03)
//								{
//									if(!g_KeepRefForSplit)
//									{
//										if(INIT_VAULE0 == KeySetRef)
//										{
//											KeySetRef	  	  =  newKeySetByKeyVersion(sdDataObj, KeyFormat, curKeyVersion, newKeyVersion, g_PutkeyDataLen);
//										}
//									}else{
//										KeySetRef	  	  =	 g_KeepRefForSplit;
//									}
//
//									if(!g_isNewKey)
//									{
//										keyData_addKeyDataSpace(MEMtPN8(locateObjectInMemory(KeySetRef)), g_PutkeyDataLen);
//									}
//#if		GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
//								  setKeyDataForKeyIdUnit(keyTagStart, U8_A_U8(SingleKeyComponentLen, 2),
//																				KeySetRef, curKeyVersion, newKeyVersion, (APDUBuffer[OFFSET_P2] & MAX_KEY_ID), KeyFormat);
//#else
//								  if((0x85 == KeyType) || (KEY_AES_TYPE == KeyType))//put 81key
//								  {
//									  setKeyDataForKeyIdUnit(keyTagStart, g_keyDataOffest-keyTagStart + 1 + SingleKeyComponentLen,
//									  																				KeySetRef, curKeyVersion, newKeyVersion, g_PutKeyId, KeyFormat);
//								  }
//								  else
//								  {
//									  setKeyDataForKeyIdUnit(keyTagStart, g_keyDataOffest-keyTagStart + 1 + SingleKeyComponentLen,
//									  																				KeySetRef, curKeyVersion, newKeyVersion, g_PutKeyId, KeyFormat);
//								  }
//
//#endif
//
//									g_PutkeyDataLen	  	  =  INIT_VAULE0;
//									//currKeyDataLen	  	  =  INIT_VAULE0;
//									g_isNewKey			  =  true;
//									keyTagStart	 		  =  g_keyDataOffest;
//								}else
								{
									if(!SingleKeyComponentLen)
									{
											keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
									}
								}
#if GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK
								if(APDUBuffer[g_keyDataOffest] == 0)
								{
									keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
								}
# endif
								if(KeyFormat == KEY_FORMAT_03)	//非对称密钥
								{
									if ((!IshasKeyUsage) && (IsSCP81Key(newKeyVersion) || IsSCP80Key(newKeyVersion)))	//简单格式时的scp81key需要keyid增加
									{
										U8_INC(g_PutKeyId);
									}
								}
								else	//对称密钥
								{
									if(!IshasKeyUsage)	//简单格式下行
									{
										U8_INC(g_PutKeyId);
									}
								}

								if(IshasKeyUsage)	//扩展格式下行
								{
									U16_INC(g_putKeyCurrState);
								}
								else
								{
									g_putKeyCurrState =  PUT_KEY_GET_TAG;
								}

								break;
						}
						case	PUT_KEY_KEY_USAGE:

								KeyUsageLen       	=  AnalyseKeyCheckOrUsageOrAccess(KeySetRef, g_PutKeyId);
								g_PutKeyRemaindLen  =  U16_S_U8(g_PutKeyRemaindLen, KeyUsageLen);
								U16_INC(g_putKeyCurrState);

								break;
						case 	PUT_KEY_KEY_ACCESS:

								KeyAccessLen     	=  AnalyseKeyCheckOrUsageOrAccess(KeySetRef, g_PutKeyId);
								g_PutKeyRemaindLen  =  U16_S_U8(g_PutKeyRemaindLen, KeyAccessLen);
								//U8_INC(g_PutKeyId);
								KeyTypeLen		  	=  PUT_KEY_KEY_TYPE_LENGTH;
								keyTagStart		  	=  g_keyDataOffest; // g_keyDataOffest point to Key type of key component
								g_putKeyCurrState 	=  PUT_KEY_GET_TAG;
								g_isNewKey		  	=  true;

								U8_INC(g_PutKeyId);

								break;
						default:
								keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
								break;
					}
			}
			
			if(MORE_COMMAND_FIRST_TAG == g_putKeyCommands)
			{
					if((U8_AND_U8(bMultipleKeys_, PUT_MULTIPLE_KEY_MASK) == PUT_MULTIPLE_KEY_MASK) // P1指示多支key，但是实际只有一支key
														 && (g_PutKeyId <= U16t8(U8_A_U8(bKeyIdStart, 1)))) // 这个地方的keyId的判断有啥意义？这个时候，经过前面的while循环之后，用+1判断可能并不准确
					{
							keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
					}
			}

#if (GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK | GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK)
			if(bMultipleKeys_ & PUT_MULTIPLE_KEY_MASK)
			{
						if(PX_R_U8(bKeyCheckTag) && (PX_R_U8(bKeyCheckTag) != 0x07))
						{
								keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
						}
			}
#endif
			return 	KeySetRef;
}
/*
 *merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 去掉未使用的参数saveMode
 */
void  SaveKey(PN8 sdDataObj){

			u8												KeyFormat,encryptkeyType;
			u8												bKeyCheckTag;
			u8												newKeyVersion;
			u8												curKeyVersion;
			u16												keySetRef;

			g_keyDataOffest						=  OFFSET_CDATA;
			g_putKeyCurrState					=  PUT_KEY_GET_TAG;
			keySetRef							=  REF_NULL;
			curKeyVersion							=  U8_AND_U8(APDUBuffer[OFFSET_P1], MAX_KEY_VERSION);
			g_PutKeyRemaindLen				=  U8t16(APDUBuffer[OFFSET_LC]);

			bKeyCheckTag						=  INIT_VALUE0;
			newKeyVersion 						=  APDUBuffer[OFFSET_CDATA]; // 如果这里不是第一条put key指令，该值是否需要和第一条的put key保持一致？
			KeyFormat							=  getFormatByKeyVersion(newKeyVersion);
			U8_INC(g_keyDataOffest);
			U16_DEC(g_PutKeyRemaindLen); // skip the key version
			
			if(g_putKeyCommands == MORE_COMMAND_FIRST_TAG)
			{
				g_isNewKey					=   true;
				g_isSetDefaultKey			=	false;
				g_KeepRefForSplit			=	REF_NULL;
				g_PutkeyDataLen				=	0x00;
				g_keyAccessValueOffest      =	INIT_VALUE0;
				g_keyUsageValueOffest       =	INIT_VALUE0;
				g_PutKeyId					=	U8_AND_U8(APDUBuffer[OFFSET_P2], MAX_KEY_ID);
#if GP__KEY_MANAGER__ECC_KEY
				g_PutEccKey                 =   false;
				g_markEccKey                =   0x00;
				g_EccKeyParaLen             =   0x00;
#endif
				if (!isInProgress())
				{
					begin_transaction(MULTIL_APDU_PROCESS);
				}
			}
			else
			{
				if(!keySet_getKeySet(sdDataObj, newKeyVersion))
				{
					 keyManager_initLogicManager();
					 if(isInProgress())
					 {
							abortTransaction();
					 }
				}
			}
			
			encryptkeyType = AnalyseEncKeyType();
			keySetRef	   = AnalyseKeyComponent(KeyFormat, curKeyVersion, newKeyVersion, sdDataObj, &bKeyCheckTag,encryptkeyType);

			if(KeyFormat != KEY_FORMAT_03)
			{
					keySetRef = newKeySetByKeyVersion(sdDataObj, KeyFormat, curKeyVersion, newKeyVersion, SO_calSecDataAreaLen(g_PutkeyDataLen));
					
					setKeyDataForKeyIdUnit(U16t8(U8_A_U8(OFFSET_CDATA, 1)), U8_S_U8(APDUBuffer[OFFSET_LC], 1), keySetRef, curKeyVersion, newKeyVersion, KeyIdCounter(keySetRef, INIT_KEY_ID, curKeyVersion), KeyFormat,encryptkeyType);
			}
			else if(IsSCP80Key(newKeyVersion)) //SCP02的是在哪里处理的？ SCP KEYuupdate count， 这个清SCP80 COUNTER的操作，放在setKeyDataForKeyIdUnit中是不是更合理？但是放的这个里面的话影响面比较多
			{
				resetSequenceCounter(keySetRef, newKeyVersion);//EPX-测试列表\euicc卡下载移动profile后相关测试\个人esim测试脚本-移动测试版 - EPX_02.sol\同步bug测试\GetStatusAddition_1_1,GetStatusAddition_1_2,GetStatusAddition_2_1,GetStatusAddition_2_2连续执行会有问题
			}

#if (EUICC_ONLY||SE_AND_EUICC)
			if(bRamBuffer[PROFILE_APDU_MODE] == TRUE)
			{
				resetSequenceCounter(keySetRef, newKeyVersion);
			}
#endif
			if(!U8_AND_U8(APDUBuffer[OFFSET_P1], PUT_MORE_KEY_MASK)) // the last put key command
			{
#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
					if(RSA_KEY_TYPE_A1 == APDUBuffer[U8_A_U8(OFFSET_CDATA, 1)]) // 如果是级联的put key指令，这个地方的判断可能会有问题
					{
							 g_PutRsaTokenKey	=	0x01;
					}else
#endif
#if GP__KEY_MANAGER__ECC_KEY
					if(g_PutEccKey)
					{
						if((IsLoadingofECCCurveParameters) || (IsLoadingofECCWithF0))
						{
							keySet_setEccKeyParaRefValidTag(MEMtPN8(locateObjectInMemory(keySetRef)), true);

							keySet_setEccKeyParaRefValue(MEMtPN8(locateObjectInMemory(keySetRef)), g_EccKeyParaRef);
						}
						else if (IsLoadingofECCWithOwnParas)
						{

						}
						else
						{
							keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
						}
					}
#endif

					commit_transaction();

#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
					if(KEY_TOKEN_ID1 == newKeyVersion)
					{
							AnalysePutKeyResponseForRSA(keySetRef, &bKeyCheckTag);
					}else
#endif
#if GP__KEY_MANAGER__ECC_KEY
					if(g_PutEccKey)
					{
						AnalysePutKeyResponseForECC(keySetRef, &bKeyCheckTag);
					}else
#endif
					{
							AnalysePutKeyResponse(keySetRef, &bKeyCheckTag);
					}
					
#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
					g_PutRsaTokenKey = 0;
#endif
					g_putKeyCommands = 0;

#if GP__KEY_MANAGER__ECC_KEY
					g_PutEccKey                 =   false;
					g_markEccKey                =   0x00;
					g_EccKeyParaLen             =   0x00;
#endif
			}
}

#if GP__KEY_MANAGER__STORE_KEY
//warning: 去掉static
void storeDesKey(PN8 sdDataObj){

		  u8		i, APDUOffest;
		  u8 		currKeyVersion, newKeyVersion;
		  u16		KeySetRef;

		  #define        STORE_DES_KEY_LENGTH_BYTE	 	 	 0x03
		  #define		 CURRENT_KEY_DATA_LEN				 0x00
		  #define		 NEW_CURRENT_KEY_VERSION			 0x00
		  #define 	     STORE_DES_KEY_ID					 0x01
		  #define 	     COMPUTE_KEY_LEN_TAG_NEWKEY			 0x01

		  APDUOffest		=	STORE_KEY_DATA_OFF;
		  //keyLen			=   APDUBuffer[STORE_KEY_DATA_LEN_OFF];
		  currKeyVersion    =	APDUBuffer[STORE_KEY_CURRENT_VERSION];
		  newKeyVersion	    =	APDUBuffer[U8_A_U8(STORE_KEY_CURRENT_VERSION, 1)];

		  KeySetRef			=	newKeySetByKeyVersion(sdDataObj, KEY_FORMAT_02, currKeyVersion, newKeyVersion, SO_calSecDataAreaLen(STORE_KEY_LEN));
		  for(i = 1; i<=MAX_KEYS_NUM; U8_INC(i))
		  {
			  setKeyData(KeySetRef, currKeyVersion, i, STORE_KEY_TYPE, STORE_KEY_LEN, STORE_KEY_LEN, APDUOffest);
			  APDUOffest	=   U16t8(U8_A_U8(APDUOffest, STORE_KEY_LEN));
		  }
}
//warning: 去掉static
void storeDAPKey(PN8 sdDataObj){

	     u8 		  keyLen, APUDOffest;
		 u8			  exponentLen;
		 u8			  moduluLen;
		 u16		  KeySetRef, newKeyLen;

		 #define	  CURRENT_KEY_DATA_LEN	  0x00
		 #define	  DAP_KEY_ID			  0x01
		 #define      DAP_MODULUS_LEN		  0x80
		 #define	  DAP_MODULUS_TYPE		  0xA1
		 #define      DAP_EXPONENT_TYPE		  0xA0
		 #define	  KEY_TYPE_LENGHT_BYTE	  0x02

		 keyLen		  =	 APDUBuffer[STORE_KEY_DAP_LEN_OFF];
		 APUDOffest	  =	 U16t8(U8_A_U8(STORE_KEY_DAP_LEN_OFF, 1));
		 exponentLen  =  U8_LOW(keyLen);
		 moduluLen	  =  U8_HIGH(keyLen);

		 newKeyLen	  =  keyData_computeKeyDataLen(KEY_FORMAT_03, CURRENT_KEY_DATA_LEN, moduluLen, true);//count modulu len
		 newKeyLen	  =	 keyData_computeKeyDataLen(KEY_FORMAT_03, newKeyLen, exponentLen, false);        //count exponent Len
		 KeySetRef	  =	 newKeySetByKeyVersion(sdDataObj, KEY_FORMAT_03, DAP_CURRENT_VERSION, DAP_NEW_VERSION, newKeyLen);

		 //閸愭獜eyType: A1 COMPOMENT
		 setKeyData(KeySetRef, DAP_CURRENT_VERSION, DAP_KEY_ID,
				 	 	 	   DAP_MODULUS_TYPE, DAP_MODULUS_LEN,
				 	 	 	   DAP_MODULUS_LEN, APUDOffest);
		 APUDOffest   =  U16t8(U8_A_U8(APUDOffest, DAP_MODULUS_LEN));
		 //閸愭獜eyType: A0 COMPOMENT
		 setKeyData(KeySetRef, DAP_CURRENT_VERSION, DAP_KEY_ID,
				   	   	   	   DAP_EXPONENT_TYPE, exponentLen,
				   	   	   	   exponentLen, APUDOffest);
}
#endif // GP__KEY_MANAGER__STORE_KEY
/*
 *merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 去掉未使用的参数saveMode
 */
u16 PutKeys(PN8 sdDataObj){
		PN8 sdEntryAddr;
		u8 lifeCycle, appId;
		InspectKey(sdDataObj);

		//PutKey
		SaveKey(sdDataObj);
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	
#else
		getAppTable();

		appId = U16t8(U16_AND_U16(get_object_owner(sdDataObj), APPLET_MASK)) ; //appID与sdDataObj对应的不一致

		sdEntryAddr = MEMtPN8(getSdEntryObj(appId));

		lifeCycle = getlifeCycleTransition(sdEntryAddr);
		//SD鐟曪拷
		//iSetSDPersonalizeState(g_bSDId[selected_channel]);
#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		if(!lifeCycle)
			iSetSDPersonalizeState(appId);
#endif
#endif
		return	SW_NO_ERROR;
}

#if GP__KEY_MANAGER__STORE_KEY
// merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
u16 storeKey(PN8  sdDataObj){

		u16 	storeKeyType;
		PN8 sdEntryAddr;
		u8 lifeCycle;
	
		storeKeyType		=	InspectstoreKey(sdDataObj);
		g_isSetDefaultKey	=	false;
	
		if (!isInProgress())
		{
			begin_transaction(SINGLE_APDU_PROCESS);
		}

		if(storeKeyType == DES_SECURE_CHANNEL_KEY) // 8f01
		{
				decryptStoreKeyData();
				storeDesKey(sdDataObj);
		}else if(storeKeyType == DAP_VERIFICATION_KEY) // 9102
		{
				storeDAPKey(sdDataObj);
		}
#if	GP__KEY_MANAGER__STORE_DGI_KEY
		else if(storeKeyType == KEY_INF_DAT_DGI) // 00b9
		{
				storeDGIKey(sdDataObj);
		}else{
				storeCRTKeyForSplit(sdDataObj);
		}
#endif

		if(g_KeepRefForSplit == INIT_KEEP_REF_FOR_SPLIT)//if(storeKeyType != KEY_INF_DAT_DGI)
		{
			g_storeDGIKeyFlag = FALSE;
#if  GP__KEY_MANAGER__ECC_KEY
			g_EccKeyParaLen   = 0;
			g_markEccKey      = 0;
			g_EccKeyParaRef   = 0;
			g_storeEccKeyMode = 0;
#endif
			getAppTable();
			sdEntryAddr = MEMtPN8(getSdEntryObj(U16t8(U16_AND_U16(cc , APPLET_MASK))));
			lifeCycle = getlifeCycleTransition(sdEntryAddr);
			if(!lifeCycle)
			{
				iSetSDPersonalizeState(U16t8(U16_AND_U16(cc , APPLET_MASK)));
			}
		}

		if (isInProgress())
		{
			commit_transaction();
		}
		//memRefresh();
		return SW_NO_ERROR;
}
#endif // GP__KEY_MANAGER__STORE_KEY

#if GP__KEY_MANAGER__DELETE_KEY
u8	delSetKey(){
	
	PN8		sdDataObj;
	
	sdDataObj		=	MEMtPN8(getSdDataObjBySdid(g_bSDId[selected_channel]));
	deleteKey(sdDataObj);
	
	return true;
	
}
#endif

//modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
u16 keyData_getKeyData(u8 sdId, u8 keyVersion, u8 keyId, memref keyDataBuffer)
{
	u8					keyType;
	jref				keySetRef;
	PN8					sdDataObject;
	PN8					keySetObjcet;
	PR32				keyDataAField;
	u16 keyLen = 0;	

	sdDataObject		=	MEMtPN8(getSdDataObjBySdid(sdId));	
	keySetRef			=	keySet_getKeySet(sdDataObject, keyVersion);
	if (REF_NULL == keySetRef)
	{
		return keyLen;
	}
	keySetObjcet		=	MEMtPN8(locateObjectInMemory(keySetRef));
	keyType				=	keySet_getKeyType(keySetObjcet, keyId, NO_ERROR);

	if (REF_NULL == keyType)
	{
		return keyLen;
	}

	keyLen		= getKeyDataByIdforSCP80(keySetObjcet, keyId, keyType, MEMtPR32(&keyDataAField));

	if(keyDataBuffer != PNtMEM(NULL))
	{
		memCopyNoAtomic(MEMtPM8(keyDataBuffer), MEMtPM8(keyDataAField), keyLen);

		memRefresh();
	}

	return keyLen;
}
//modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
u16	keyData_fetchKey(u8	sdId, u8 keyVersion, u8 keyId, memref keyDataBuffer)
{
	u8					keyType;
	jref				keySetRef;
	jref      			keyLen;
	jref   	  			sdRef;
	jref	  			sdDataRef;
	memref    			enTry;
	PN8		  			sdObj;
	PN8					sdDataObject;
	PN8					keySetObjcet;
	PR32				keyDataAField;

	keyLen	   			=   0;
	enTry	   			=	getEntryByAppId(sdId);
	sdRef      			=	nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(enTry), theApplet_INDEX)));
	sdObj      			=   MEMtPN8(locateObjectInMemory(sdRef));
	sdDataRef  			=   PN_R_U16(get_object_field_ref(sdObj,sdData_INDEX));

	if(!sdDataRef)
	{
			return keyLen;
	}
	else
	{
		sdDataObject	=	MEMtPN8(locateObjectInMemory(sdDataRef));
	}

	keySetRef			=	keySet_getKeySet(sdDataObject, keyVersion);

	if(!keySetRef)
	{
			return keyLen;
	}
	else
	{
		keySetObjcet	=	MEMtPN8(locateObjectInMemory(keySetRef));
	}

	keyType				=	keySet_getKeyType(keySetObjcet, keyId,THROW_ERROR);

	keyLen				=   getKeyDataByIdforSCP80(keySetObjcet, keyId, keyType, MEMtPR32(&keyDataAField));
	memCopyNoAtomic(MEMtPM8(keyDataBuffer), MEMtPM8(keyDataAField), keyLen);
	memRefresh();
	return keyLen;
}

u16 getScp80Key(u8 appId, u8 keyVersion, u8 keyId, memref keyDataBuffer)
{
	u16 keyLen = 0;	
	//u8  tempkeyVersion;
	//PN8			sdDataObject;
	
	while((keyLen == 0) && (appId != 0xFF))
	{
		keyLen = keyData_getKeyData(appId, keyVersion, keyId, keyDataBuffer);
		
		if(keyLen == 0)
			appId	= getAssociateSDId(appId);
	}
	
	return keyLen;
}


void setKeyCounterForSCP80(u8 sdId, u8 keyVersion, u8 len, PR8 counterValue)
{
	jref				keySetRef;
	jref                sdRef;
	jref				sdDataRef;
	PN8					sdDataObject;
	PN8					keySetObjcet;
	PN8					sdObject;
	memref              entry;

	keySetRef           = REF_NULL;

	while((REF_NULL == keySetRef) && (sdId != APP_NULL))
	{
		entry				=	getEntryByAppId(sdId);
		sdRef 				= 	nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(entry), theApplet_INDEX)));
		sdObject			= 	MEMtPN8(locateObjectInMemory(sdRef));
		sdDataRef			=	PN_R_U16(get_object_field_ref(MEMtPF8(sdObject),sdData_INDEX));

		if(REF_NULL == sdDataRef)
		{
			sdId    =   getAssociateSDId(sdId);
			continue;
		}
		else
		{
			sdDataObject		= 	MEMtPN8(locateObjectInMemory(sdDataRef));
		}

		keySetRef			=	keySet_getKeySet(sdDataObject, keyVersion);

		if(REF_NULL == keySetRef)
		{
			sdId    =   getAssociateSDId(sdId);
		}
	}
	//TODO: 如果keyVersion先没put进去的话，要抛错？
	if(REF_NULL == keySetRef){
		return;
	}
	keySetObjcet		=	MEMtPN8(locateObjectInMemory(keySetRef));
	keySet_setSequenceCounter(keySetObjcet, len, counterValue);

}

void setAllKeyCounterForSCP80(u8 sdId, u8 len, PR8 counterValue)
{
	jref				keySetRef;
	jref                sdRef;
	jref				sdDataRef;
	PN8					sdDataObject;
	PN8					keySetObjcet;
	PN8					sdObject;
	memref              entry;
	jref                keyGroupRef;
	PN8                 keySetHand;
	u8                  i;


	keyGroupRef           = REF_NULL;

	while((REF_NULL == keyGroupRef) && (sdId != APP_NULL))
	{
		entry				=	getEntryByAppId(sdId);
		sdRef 				= 	nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(entry), theApplet_INDEX)));
		sdObject			= 	MEMtPN8(locateObjectInMemory(sdRef));
		sdDataRef			=	PN_R_U16(get_object_field_ref(MEMtPF8(sdObject),sdData_INDEX));

		if(REF_NULL == sdDataRef)
		{
			sdId    =   getAssociateSDId(sdId);
			continue;
		}
		else
		{
			sdDataObject		= 	MEMtPN8(locateObjectInMemory(sdDataRef));
		}

		keyGroupRef = sdData_getKeyGroupRef(sdDataObject, PUT_KEY_SCP80, FALSE);

		if (REF_NULL == keyGroupRef)
		{
			sdId    =   getAssociateSDId(sdId);
		}
	}
	//TODO: 如果keyVersion先没put进去的话，要抛错？
	if(REF_NULL == keyGroupRef){
		return;
	}

	for(i=KEY_SCP80_MIN; i<=KEY_SCP80_MAX; i++)
	{
		keySetHand = MEMtPN8(get_object_field_ref(MEMtPF8(locateObjectInMemory(keyGroupRef)), keySet_INDEX));
		keySetHand = MEMtPN8(keySet_getKeySetHand(keySetHand, i));

		if (MEM_NULL != keySetHand)
		{
			keySetRef =  memReadU16(keySetHand);
			keySetObjcet		=	MEMtPN8(locateObjectInMemory(keySetRef));
			keySet_setSequenceCounter(keySetObjcet, len, counterValue);
		}
	}
}

memref getCounterAddrByVersion(u8 sdId, u8 keyVersion) //获取对应keyVersion的Counter的地址
{
	jref keySetRef;
	jref sdRef;
	jref sdDataRef;
	PN8 sdDataObject;
	PN8	keySetObjcet;
	PN8	sdObject;
	memref entry;
	memref counterAddr;

	entry				=	getEntryByAppId(sdId);
	sdRef 				= 	nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(entry), theApplet_INDEX)));
	sdObject			= 	MEMtPN8(locateObjectInMemory(sdRef));
	sdDataRef			=	PN_R_U16(get_object_field_ref(MEMtPF8(sdObject),sdData_INDEX));

	if(REF_NULL != sdDataRef)
	{
		sdDataObject = MEMtPN8(locateObjectInMemory(sdDataRef));
	}
	else
	{
		return FALSE;
	}

	keySetRef =	keySet_getKeySet(sdDataObject, keyVersion);

	if(REF_NULL == keySetRef)
	{
		return FALSE;
	}

	keySetObjcet = MEMtPN8(locateObjectInMemory(keySetRef));

	counterAddr = keySet_getSequenceCounter(keySetObjcet, 5);

	return counterAddr;
}

jref getSCP80KeySEQ(u8 sdId, u8 keyVersion, PX8 counterppointer)
{
	jref                sdRef;
	jref				sdDataRef;
	jref				keySetRef;
	PN8					sdObject;
	PN8					sdDataObject;
	PN8					keySetObjcet;
	PN8					SEQP;
	memref              entry;

	keySetRef           = REF_NULL;

	while((REF_NULL == keySetRef) && (sdId != 0xFF))
	{
		entry				=	getEntryByAppId(sdId);
		sdRef 				= 	nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(entry), theApplet_INDEX)));
		sdObject			= 	MEMtPN8(locateObjectInMemory(sdRef));
		sdDataRef			=	PN_R_U16(get_object_field_ref(sdObject,sdData_INDEX));

		if(REF_NULL == sdDataRef)
		{
			sdId    =   getAssociateSDId(sdId);
			continue;
		}
		else
		{
			sdDataObject		= 	MEMtPN8(locateObjectInMemory(sdDataRef));
		}

		keySetRef			=	keySet_getKeySet(sdDataObject, keyVersion);

		if(REF_NULL == keySetRef)
		{
			sdId    =   getAssociateSDId(sdId);
		}
	}

	if(REF_NULL == keySetRef)
	{
		return REF_NULL;
	}

	keySetObjcet		=	MEMtPN8(locateObjectInMemory(keySetRef));
	SEQP = MEMtPN8(keySet_getSequenceCounter(keySetObjcet, SCP80_SEQUENCE_CONTER_SIZE));
	memCopyToRam(counterppointer, MEMtPM8(SEQP), SCP80_SEQUENCE_CONTER_SIZE);
		
	return keySetRef;
}

/*
memref getAssociateScp80KeySEQ(u8 appId, u8 keyVersion,PN8 counterppointer)
{
	//u8  tempkeyVersion;
	//PN8			sdDataObject;
	jref				keySetRef = REF_NULL;
	
	while((REF_NULL == keySetRef) && (appId != 0xFF))
	{
		keySetRef = getSCP80KeySEQ(appId, keyVersion, counterppointer);
		
		if(REF_NULL == keySetRef)
			appId	= getAssociateSDId(appId);
	}
	if(REF_NULL != keySetRef)
	{
		return	locateObjectInMemory(keySetRef);
	}
	else
	{
		return MEM_NULL;
	}
}
*/

void keyManager_initLogicManager()
{
			g_putKeyCurrState  			= 0;
			g_keyDataOffest					= 0;
			g_PutKeyRemaindLen			= 0;
			g_keyUsageValueOffest 	= 0;
			g_keyAccessValueOffest 	= 0;
			g_PutKeyId 							= 0;
			g_PutkeyDataLen 				= 0;
			g_putKeyCommands	 			= 0;
			g_KeepLenForUsageOrAccess =	0;
#if		GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
			g_PutRsaTokenKey				=	0;
#endif
}

void keyManager_abortLogicManager()
{
		 keyManager_initLogicManager();
		 if(gBackUp_inMutilApdu)
		 {
				abortTransaction();
		 }
}

void keyManager_abortLogicManagerThorwError(u16 error)
{
		keyManager_abortLogicManager();
		throw_error(error);
}
/*
 * merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 将宏定义移到KeyLogicManager.h
 */
void keyManager_InsertApduManager(u8 InsertType)
{
	u8						newKeyVersion;
	PN8						sdDataObj;
	
	switch(InsertType)
	{
		case OTHER_COMMAND:
		{
			keyManager_abortLogicManager();
			break;
		}
		case PUT_KEY_COMMAND:
		{
			newKeyVersion				=	APDUBuffer[OFFSET_CDATA];
			if(0xFF == newKeyVersion)
			{
					newKeyVersion		=	APDUBuffer[U8_A_U8(OFFSET_CDATA, 1)];
			}
			sdDataObj					=	MEMtPN8(getCurrentSdObj(IS_NO_NEED_CHECK_SD));
			if(!keySet_getKeySet(sdDataObj, newKeyVersion))
			{
					keyManager_abortLogicManager();
			}
			break;
		}
		default:
			break;
	}
}

void resetAllCounter()
{
		u16		 	i;
		u16			appletRef;
		u16			sdDataRef;
		u16			keyGroupRef;
		u16			keySetRef;
		u16		 	bFreeEntryIndex;
		memref 		entry;
		memref		theAppTable;
		PN8			appObject;
		PN8			sdDataObject;
		PN8			keySetObj;
		PN8			keyGroupObject;
		u8	  X_A	initValue[5];

		ecpMem_Set(initValue, 0x00, SCP80_SEQUENCE_CONTER_SIZE);
		/*@ -disspecopt @*/bFreeEntryIndex = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);/*@ +disspecopt @*/
		theAppTable = getAppTable();
		for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
		{
			entry 				= locateObjectInMemory(PN_R_U16(U32_A_U8(theAppTable , U8_M_U8(i , SIZE_REF))));
			appletRef 			= 	nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(entry), theApplet_INDEX)));

			if(!appletRef)
			{
				continue;
			}
			appObject			= 	MEMtPN8(locateObjectInMemory(appletRef));
			sdDataRef			=	PN_R_U16(get_object_field_ref(appObject, sdData_INDEX));

			if(!sdDataRef)
			{
				continue;
			}

			sdDataObject		= 	MEMtPN8(locateObjectInMemory(sdDataRef));
			keyGroupRef			=	PN_R_U16(get_object_field_ref(sdDataObject, firstKeyGroup_INDEX));
			while(REF_NULL != keyGroupRef)
			{
				keyGroupObject	=	MEMtPN8(locateObjectInMemory(keyGroupRef));
				keySetRef		=	keyGroup_getKeySetRef(keyGroupObject);

				while(REF_NULL != keySetRef)
				{
					keySetObj 	= 	MEMtPN8(locateObjectInMemory(keySetRef));
					keySet_setSequenceCounter(keySetObj, 0x05, MEMtPR8(initValue));
					keySetRef	=	keySet_getNextKeySetRef(keySetObj);
				}

				keyGroupRef		=   PN_R_U16(keyGroup_getNextKeyGroupHand(keyGroupObject));
			}
		}
}
/*
 * @history: modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
 *           merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
 */
u8 AnalyseEncKeyType(void)
{

	u8					encryptkeyType;
	jref				keySetRef;
	PN8					sdDataObject;
	PN8					keySetObjcet;
	u8 					keyId;


	#define    INIT_VAULE0     0x00

	keyId = INIT_VAULE0;
    encryptkeyType = INIT_VAULE0;

#if JAVA_CARD_UICC
	if(bRamBuffer[PROCESS_LEVEL] == ROCESS_LEVEL_OTA)
	{
		if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)//非下载profile流程
		{
		if(g_SCP02SDID == APP_NULL)// RAM指令中没有自己开scp会话  用SCP80的KIK解密
		{

			if(IsSCP81Key(bRamBuffer[REMOTE_SCP_KEY_VERSION]))
			{
					keyId = 0x02;  // 这个地方用0x02不一定正确，因为SCP81的key id可以不从1开始
			}
			else if(IsSCP80Key(bRamBuffer[REMOTE_SCP_KEY_VERSION]))
			{
				keyId = 0x03; //KIK
			}

			sdDataObject		=	MEMtPN8(getSdDataObjBySdid( bRamBuffer[REMOTE_SCP_SDID] ));
			keySetRef			=	keySet_getKeySet(sdDataObject, bRamBuffer[REMOTE_SCP_KEY_VERSION]);
			if (REF_NULL != keySetRef)
			{
				keySetObjcet		=	MEMtPN8(locateObjectInMemory(keySetRef));
				encryptkeyType		=	keySet_getKeyType(keySetObjcet, keyId,THROW_ERROR);

				if (getScp80Key(bRamBuffer[REMOTE_SCP_SDID], bRamBuffer[REMOTE_SCP_KEY_VERSION], keyId, PXtMEM(g_current_SCP_DEK_KEY)) == 0)
				{
					encryptkeyType = 0x00;
				}
			}
		}
			else				//indicate that scp02 or scp03 session has open, 用例：ul704-scp80:scenario_01_CATTP_aes16B_scp03_16B_30_6e_05
			{
				if (g_current_SCP_Type == SCP_02)		//current session is SCP02 session	
				{
					encryptkeyType = KEY_DES_TYPE;
				}
				else									//current session is SCP03 session
				{
					encryptkeyType = KEY_AES_TYPE;
				}
			}
		}
	}
	else  //本地APDU
#endif
	{
		//if(channelInfo.security_level[selected_channel] != SCF_NO_AUTHENTICATED)
		if(g_current_SCP_Type == SCP_03) // todo: SCP11鉴权成功之后，给SD PUT AES KEY的话，这里可能存在问题
		{
			encryptkeyType = KEY_AES_TYPE;
		}
	}
	return encryptkeyType;
}
// modify by xuxiaojun@20220318 分层，该函数被移动到SCPAPINativeMethod.c文件中
//u8 IF_GetSCP80KeyInfo(u8 *dek,u8 ramKeyversion,u8 sdId, u8 *keyType)
#endif
