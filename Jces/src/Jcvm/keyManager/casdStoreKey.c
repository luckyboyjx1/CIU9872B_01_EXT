/*
 * fm_storeKey.c
 *
 *  Created on: 2013-10-14
 *      Author: liuhanren
 */
#include "KeyLogicManager.h"
#include "casdStoreKey.h"
#include "cardmanager.h"
#include "typejava.h"
#include "global_variable.h"
#include "error.h"
#include "keyData.h"
#include "SecurityDomainData.h"
#include "keyGroup.h"
#include "SCPInterface.h"
#include "opFunction.h"
#include "Crypto_Constant.h"
#include "Crypto_API.h"
#include "keySet.h"
#include "euiccConfig.h" // warning implicit declaration of function 'JcvmConfig_GetSEState' [-Wimplicit-function-declaration]

#if GP__SCP__SCP11__FOR_MASK
#include "SCP11.h"
#endif

#include "ob_manager.h"
#include "Interface_PlatformFromLTE.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if GP__KEY_MANAGER__STORE_DGI_KEY
#include "fm03_keyData.h"	//remove warning,dengyiwen,2022/9/27
#endif

#define			 STORE_KEY_GET_CRT_TAG			    0x01
#define			 STORE_KEY_GET_USAGE				0x02
#define 		 STORE_KEY_GET_TAG					0x03
#define			 STORE_KEY_GET_LENGTH				0x04

#if GP__KEY_MANAGER

#if GP__KEY_MANAGER__STORE_DGI_KEY
static u8 analyseCRTCompomentData(u8 analyseRSAType){

	u8		CrtLen;
	u8		CrtTagValue;

	CrtLen = APDUBuffer[U8_INC(g_keyDataOffest)];

	if(RSA_CRT_TAG != g_putKeyCurrState)
	{
			if(CrtLen != analyseRSAType)
			{
					throw_error(SW_WRONG_DATA);
			}

			if(RSA_CRT_KEY_LENGTH	==	g_putKeyCurrState)
			{
				if(SIZE_81_LENGTH_1 == CrtLen)
				{
					//g_PutkeyDataLen =	PX_R_U16(&APDUBuffer[U8_INC(g_keyDataOffest)]);
					CrtTagValue			=	PX_R_U16(&APDUBuffer[U8_INC(g_keyDataOffest)]);
				}else{

					//g_PutkeyDataLen	=	U8_A_U8(g_PutkeyDataLen, APDUBuffer[U8_INC(g_keyDataOffest)]);
					CrtTagValue			=	APDUBuffer[U8_INC(g_keyDataOffest)];
				}
			}
			else if(RSA_CRT_KET_CHECK_VALUE  ==  g_putKeyCurrState)
			{
				g_keyDataOffest		=	U8_A_U8(g_keyDataOffest, CrtLen);
			}
			else
			{
				CrtTagValue			=	APDUBuffer[U8_INC(g_keyDataOffest)];
			}

			g_PutKeyRemaindLen	=	U16_S_U8(g_PutKeyRemaindLen, CrtLen);//偏移V
	}

	g_PutKeyRemaindLen			=	U16_S_U8(g_PutKeyRemaindLen, 2);//偏移TL

	return 	CrtTagValue;
}
/*
*@function   :newStoreKeySetByVersion
*
*@description:store key方式时，获取到对应key version的keySet，如果keySet为null，则创建。 该函数仅仅用于存储格式为format3的情况
*
*see         :
*
*@parameter  :PN8 sdDataObj --- 当前正在处理的store key的sdData
*@parameter  :u8 version --- 当前正在处理的store key指令中所指示的key version
*@parameter  :u8 storeEccKeyMode --- 当前store ecc key的模式是B0+F0, B1+F0, F0+B2...B7
*@parameter  :u8 keyType --- 当前正在处理的store key指令中所指示的key type,可能为B0+F0, B1+F0,
*@parameter  :u16 newKeySetLen ---
*
*@history    :merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 增加RSA KEY和ECC KEY的处理
*/
jref newStoreKeySetByVersion(PN8 sdDataObj, u8 version, u8 storeEccKeyMode, u8 keyId, u16 newKeySetLen){

		u16 keySetRef;
		u16 keyLen;
		jref nextKeyData;

		keySetRef		=	keySet_getKeySet(sdDataObj, version);
	
		if(keySetRef 	==  REF_NULL)
		{
			keySetRef   	=	keySet_newKeySet(KEY_FORMAT_03, sdDataObj, version, newKeySetLen);
			resetSequenceCounter(keySetRef, version);
		}
		else if(storeEccKeyMode == ECC_StoreKeyMode_KEY_AND_F0)// modify by xuxiaojun@20220604  如果公钥存在，需要更新公钥的话，不应该再增加keyData. 测试用例：704 gpUICC2Compliance203： km_APDU_storeData_ECC_success_update_token_32B_P256_scp03_9b_ef_21
		                                                       // modify by xuxiaojun@20220604  ECC公钥存在，需要新增私钥的话，需要addKeyDataSpace;  测试用例： AmendH测试脚本-SCP11c： Annex_B_scenario_02_DM_with_autoRestore_Taes16B_Raes16B_bb_02_12
		{
			if(FM03_locateKeyId(MEMtPN8(locateObjectInMemory(keySetRef)), keyId, TAG_GET_KEY, &nextKeyData, &keyLen))
			{
				if(keyLen != newKeySetLen) //modify by xuxiaojun@20220609 更新密钥时，如果长度不一致则不允许更新。避免长度超出时会导致对象被冲掉
				{
					throw_error(SW_WRONG_DATA);
				}
			}
			else
			{
				keyData_addKeyDataSpace(MEMtPN8(locateObjectInMemory(keySetRef)), newKeySetLen);
			}
		}
		else{
				keyData_addKeyDataSpace(MEMtPN8(locateObjectInMemory(keySetRef)), newKeySetLen);
		}

		return 	keySetRef;
}

/*
*@description:
*
*@see        :
*
*@function   :analyseCRTCompoment
*
*@param      :u16 keySetRef        --- input
*@param      :PN8 sdDataObj        --- input
*@param      :PX8 keyId            --- output
*@param      :PX8 keyLen           --- output
*@param      :PX8 keyType          --- output
*@param      :PX8 keyCheckValueOff --- output
*@param      :PX8 keyFormat        --- output
*@param      :PX8 keyVer           --- output
*@param      :PX8 keyAccess        --- output
*@param      :PX8 keyUsageQ        --- output
*
*@return     :jref
*@history    :modify by xuxiaojun@20220519: merged from EPX\v3.0\DEVELOP: 增加参数
*            :modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32
*/
jref analyseCRTCompoment(u16 keySetRef, PN8 sdDataObj, PX8 keyId, PX8 keyLen, PX8 keyType, PX8 keyCheckValueOff,PX8 keyFormat,PX8 keyVer,PX8 keyAccess,PX8 keyUsageQ)
{

			u8					curKeyType;
			u8					isNewKeyTag;
			u8					keyUsage;
			u8					keyVersion;
			u8                  keyNum;
			//u8					setKeepCheckValueOffTag;
//			u16					keySetRef;
			u16					curkeyLen; // 明文数据的实际长度
//			PN8 				keySetObj;
//			u8					allKeyTag;
			#define 			INIT_KEY_VALUE                 0x00
			#define				RSA_TAG_LENGTH_BYTE1           0x01
			#define				RSA_TAG_LENGTH_BYTE2           0x02
			#define				RSA_TAG_LENGTH_BYTE3		   0x03

			keyNum 					= 	INIT_KEY_VALUE;
			curkeyLen				=	INIT_KEY_VALUE;
//			keySetRef				=	INIT_KEY_VALUE;
			keyUsage				=	INIT_KEY_VALUE;
			//setKeepCheckValueOffTag	=	INIT_KEY_VALUE;
			g_PutKeyId				=	RSA_TAG_LENGTH_BYTE1;
			g_PutkeyDataLen			=	INIT_KEY_VALUE;
			isNewKeyTag             =   true;
//			g_keyDataOffest			=	RSA_DGI_LEN_OFF;
			g_PutKeyRemaindLen		=	U8t16(APDUBuffer[U8_INC(g_keyDataOffest)]);
			g_storeDGIKeyFlag       =   TRUE;

			keyFormat[0]               =   KEY_FORMAT_03;
			while(g_PutKeyRemaindLen > 0)
			{
				  g_putKeyCurrState	 =	U8t16(APDUBuffer[U8_INC(g_keyDataOffest)]);

				  switch(g_putKeyCurrState)
				  {
						case	RSA_CRT_TAG: // B9
						{
								U8_INC(keyNum);//1个B9 CRT描述一个key属性
								analyseCRTCompomentData(RSA_TAG_LENGTH_BYTE1);
								break;
						}
						case	RSA_CRT_USAGE_QUALIFIER://95 GP2.3 升级成了2字节  需要注意只是取了 KEY结构需要增加一个字节  暂时没有做
						{
								keyUsageQ[(keyNum-1)*KEY_USAGE_QUALIFIER_LENGTH] = APDUBuffer[g_keyDataOffest];

								if(APDUBuffer[g_keyDataOffest] == 2)
								{
									keyUsage	=	analyseCRTCompomentData(RSA_TAG_LENGTH_BYTE2);
									keyUsageQ[(keyNum-1)*KEY_USAGE_QUALIFIER_LENGTH + 1] = keyUsage;
									keyUsageQ[(keyNum-1)*KEY_USAGE_QUALIFIER_LENGTH + 2] = APDUBuffer[U8_INC(g_keyDataOffest)];
								}
								else if(APDUBuffer[g_keyDataOffest] == 1)
								{
									keyUsage	=	analyseCRTCompomentData(RSA_TAG_LENGTH_BYTE1);
									keyUsageQ[(keyNum-1)*KEY_USAGE_QUALIFIER_LENGTH + 1] = keyUsage;
									keyUsageQ[(keyNum-1)*KEY_USAGE_QUALIFIER_LENGTH + 2] = 0;
								}
								else
								{
									throw_error(SW_WRONG_DATA);
								}
								break;
						}
						case    RSA_CRT_KEY_ACCESS: // 96
						{
								keyAccess[(keyNum-1)] = analyseCRTCompomentData(RSA_TAG_LENGTH_BYTE1);
								break;
						}
						case	RSA_CRT_KEY_TYPE: // 80
						{
								curKeyType		=	analyseCRTCompomentData(RSA_TAG_LENGTH_BYTE1);

								if(keyTypeValid(curKeyType) == false)
								{
									throw_error(SW_WRONG_DATA);
									break;
								}

								keyType[(keyNum-1)]   =  curKeyType;

	#if  GP__KEY_MANAGER__ECC_KEY
								if(curKeyType == ECC_KEYPARAREF_DGI_TYPE) // ecc Key parameter reference  'F0'
								{
									SetBit(g_markEccKey,ECC_KEYPARAREF_BIT0);
								}
								else if((curKeyType >= ECC_PublicKey_DGI_TYPE)&&(curKeyType <= ECC_CurvePara_K_DGI_TYPE)) // b0-b7
								{
									if((curKeyType == ECC_PublicKey_DGI_TYPE)||(curKeyType == ECC_PrivateKey_DGI_TYPE))
									{
										if(ValueBit(g_markEccKey,ECC_PublicKey_BIT1))//B0/B1同时出现在一组key里
										{
											throw_error(SW_WRONG_DATA);
										}
										SetBit(g_markEccKey,ECC_PublicKey_BIT1);
									}
									else
									{
										SetBit(g_markEccKey,U8_AND_U8(curKeyType,0x07));
									}
								}
	#endif
	//									#if GP__AMENDMENT__B__FOR_MASK
	//										if ( (keyType[(keyNum-1)] != 0x85) && (keyType[(keyNum-1)] != 0x88) && (keyType[(keyNum-1)] != 0x80) )
	//										{
	//											throw_error(SW_WRONG_DATA);
	//										}
	//									#endif
								break;
						}
						case	RSA_CRT_KEY_LENGTH: // 81
						{
								if((APDUBuffer[g_keyDataOffest] != RSA_TAG_LENGTH_BYTE1) && (APDUBuffer[g_keyDataOffest] != RSA_TAG_LENGTH_BYTE2))
								{
										throw_error(SW_WRONG_DATA);
								}

								curkeyLen	=	U8t16(analyseCRTCompomentData(APDUBuffer[g_keyDataOffest])); // Key Length indicated in '00B9'

								keyLen[(keyNum-1)]   =  curkeyLen;
	#if  GP__KEY_MANAGER__ECC_KEY
								//检查每个元素的长度是否合法， 注意：B0 B5 B7 长度特殊  其余长度应与keybitlen/8一致
								checkEccKeyLen(curkeyLen,curKeyType);// todo  :checkEccKeyLen

								if(curKeyType == ECC_CurvePara_K_DGI_TYPE)
								{
									curkeyLen = 0x02;//固定2字节存储
								}

								if((curKeyType >= ECC_CurvePara_P_DGI_TYPE)&&(curKeyType <= ECC_CurvePara_K_DGI_TYPE))// ECC curve para 没有keyversion项 只有 80 81tag,所以在这里计算长度,其余在处理RSA_CRT_KEY_VERSION_NUMBER时计算长度
								{
									g_PutkeyDataLen     =   keyData_computeKeyDataLen(KEY_FORMAT_03, g_PutkeyDataLen, curkeyLen, isNewKeyTag);
									isNewKeyTag			=	false;
								}
	#endif
								break;
						}
						case	RSA_CRT_KEY_IDENTIFIER:
						{
								keyId[(keyNum-1)]			=	analyseCRTCompomentData(RSA_TAG_LENGTH_BYTE1);

								break;
						}
						case	RSA_CRT_KEY_VERSION_NUMBER:
						{
								keyVersion			=	analyseCRTCompomentData(RSA_TAG_LENGTH_BYTE1);
								keyFormat[0]        =   getFormatByKeyVersion(keyVersion);
								keyVer[0]           =   keyVersion;
								//此处因为需要兼容scp11的key，所以调整判断范围

								if(KeyVersionIsValid(keyVersion) == false)// modify by xuxiaojun@20220601, GP UICC2 Compliance203: km_APDU_storeData_AES_error_invalid_KVN_00_scp03_9b_38_6b
								{
									throw_error(SW_WRONG_DATA);
								}
								if(keyFormat[0] == KEY_FORMAT_03)
								{
									if((g_PutKeyId != keyId[(keyNum-1)]) && (g_PutkeyDataLen))
									{
										keySetRef			=	newStoreKeySetByVersion(sdDataObj, keyVersion, g_storeEccKeyMode, keyId[0], g_PutkeyDataLen);
	//												keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));
	//												allKeyTag	= keySet_getkeyAllTag(keySetObj);
	//												allKeyTag   = U8_OR_U8(allKeyTag, U8_SHL_U8(1, U8_S_U8(g_PutKeyId, 1)));
	//												keySet_SetkeyAllTag(keySetObj, allKeyTag);
										g_PutkeyDataLen		=	INIT_KEY_VALUE;
										isNewKeyTag     	=   true;
									}

									g_PutkeyDataLen     =   keyData_computeKeyDataLen(KEY_FORMAT_03, g_PutkeyDataLen, curkeyLen, isNewKeyTag);
								}
								else
								{
									g_PutkeyDataLen     =   curkeyLen;
								}
								isNewKeyTag			=	false;
								g_PutKeyId    		= 	keyId[(keyNum-1)];
								bRamBuffer[0] 		= 	keyVersion;
								break;
						}
						case	RSA_CRT_KET_CHECK_VALUE:
						{
								{
									PX_W_U8(PX8_A_U8(keyCheckValueOff, (keyNum-1)), g_keyDataOffest);
									//U8_INC(keyNum);
								}
								analyseCRTCompomentData(RSA_TAG_LENGTH_BYTE3);
								break;
	#if  GP__KEY_MANAGER__ECC_KEY
						case    CRT_KET_PARA_REF:
								if(APDUBuffer[g_keyDataOffest] == 2)
								{
									g_EccKeyParaRef	=	analyseCRTCompomentData(RSA_TAG_LENGTH_BYTE2);
									g_EccKeyParaRef = (g_EccKeyParaRef<<8)&0xFF00;
									g_EccKeyParaRef |= APDUBuffer[U8_INC(g_keyDataOffest)];

								}else{
									g_EccKeyParaRef	=	analyseCRTCompomentData(RSA_TAG_LENGTH_BYTE1);
								}
							break;
	#endif
						}
						default:
								throw_error(SW_WRONG_DATA);
								break;
					}
			}
#if  GP__KEY_MANAGER__ECC_KEY
			if(g_storeEccKeyMode == ECC_StoreKeyMode_KEY_AND_DOMAIN)//key + b2~b7,已经有了key的信息，本次需b2~b7
			{
				if((g_markEccKey != 0xFE)&&(g_markEccKey != 0x7E))
				{
					throw_error(SW_WRONG_DATA);
				}
				keyVersion = keySet_getKeyVersion(MEMtPN8(locateObjectInMemory(keySetRef)));
				keyVer[0] = keyVersion;

				keyId[0] =  ECC_DOMAIN_KID;//
				keyType[0] = ECC_KEYPARAREF_DGI_TYPE;
			}

			if(g_markEccKey == 0x03)
			{
				g_storeEccKeyMode = ECC_StoreKeyMode_KEY_AND_F0;
			}
			else if((g_markEccKey == 0x02)||(g_markEccKey == 0xFE)||(g_markEccKey == 0x7E))
			{
				g_storeEccKeyMode = ECC_StoreKeyMode_KEY_AND_DOMAIN;
			}
			else if((g_markEccKey == 0xFD)||(g_markEccKey == 0x7D))//ecc 曲线参数 固定使用以下属性
			{
				g_storeEccKeyMode = ECC_StoreKeyMode_F0_AND_DOMAIN;
				keyVer[0] = ECC_DOMAIN_KEY_VERSION;
				keyVersion = ECC_DOMAIN_KEY_VERSION;
				keyId[0] =  ECC_DOMAIN_KID;//
				//g_PutKeyId = ECC_DOMAIN_KID;
				keyType[0] = ECC_KEYPARAREF_DGI_TYPE;
			}
			else
			{
				if(g_markEccKey != 0x00)
				{
					throw_error(SW_WRONG_DATA);
				}
			}
#endif
			if(keyFormat[0] == KEY_FORMAT_03)
			{
				if(!keySetRef)
				{
						keySetRef	=	newStoreKeySetByVersion(sdDataObj, keyVersion, g_storeEccKeyMode, keyId[0], g_PutkeyDataLen);
						if(keyUsage)
						{
								//keyData_setKeyUsageQualifier((NUM_TO_NPTR)locateObjectInMemory(keySetRef), keepKeyId, &keyUsage);
						}
#if  GP__KEY_MANAGER__ECC_KEY
						if(ValueBit(g_markEccKey,ECC_KEYPARAREF_BIT0)) //set F0
						{
							keySet_setEccKeyParaRefValidTag(MEMtPN8(locateObjectInMemory(keySetRef)),TRUE);
							keySet_setEccKeyParaRefValue(MEMtPN8(locateObjectInMemory(keySetRef)),g_EccKeyParaRef);
						}
#endif

				}else{
					keyData_addKeyDataSpace(MEMtPN8(locateObjectInMemory(keySetRef)), g_PutkeyDataLen);
				}

//				keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));
//				allKeyTag	= keySet_getkeyAllTag(keySetObj);
//				allKeyTag   = U8_OR_U8(allKeyTag, U8_SHL_U8(1, U8_S_U8(g_PutKeyId, 1)));
//				keySet_SetkeyAllTag(keySetObj, allKeyTag);
				g_PutkeyDataLen		=	INIT_KEY_VALUE;
				isNewKeyTag     	=   true;

			}
			return 	keySetRef;
}
#endif // GP__KEY_MANAGER__STORE_DGI_KEY

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
void encryptKeyCheckValueForRSA(u8 keyDataOffest, u8	checkValueOffest, RAM_U8_P bKeyCheckTag, u8	keyComNum, u8 keyType, u16 keyLen)
{			
			#define													ICV_SIZE							0x08
			#define													KEY_SIZE							0x10
			#define 												INIT_VALUES				    0x00
			#define													PAD_BYTE_SIZE					0x08
	
			u8															BlockNum;
			u8															BlockOffest;
			u8				                      ICV[ICV_SIZE];	
			u8															Key[KEY_SIZE];
			u8															i							 = INIT_VALUES;
			u8															PaddingByte[8] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00};
			u16															tmpKeyLen			 =	keyLen;
			RAM_U8_P												dataIn       	 = (RAM_U8_P)&bTempArr;

			if(!APDUBuffer[checkValueOffest])
			{
				return;
			}
			PX_W_U8((NPTR_TO_NUM)bKeyCheckTag, (SetBit(PX_R_U8(bKeyCheckTag),keyComNum)));
			
			ecpMem_Set(ICV, INIT_VALUES, ICV_SIZE);
			ecpMem_Set(Key, INIT_VALUES, KEY_SIZE);
			ecpMem_Copy(dataIn,  U8_A_U8(APDUBuffer , keyDataOffest), keyLen);
			
			do{
					ecpMem_Set(U32_A_U16(dataIn , U16_A_U8(keyLen , i)), PaddingByte[i], 0x01);
					U8_INC(i);
					U8_INC(tmpKeyLen);
			}while(tmpKeyLen % 8);
					
			Triple_DES(dataIn, MEMtPX8(Key), dataIn, tmpKeyLen, MEMtPX8(ICV), CBC_Encrypt, SENSITIVE_DATA_MASKED);
			
			BlockNum		=	U16_D_U8(tmpKeyLen , 8 );
			BlockOffest = U16_S_U8(tmpKeyLen, 8);
			if (memCompare(PXtMEM(U32_A_U8(dataIn , BlockOffest)), PXtMEM(U8_A_U8(APDUBuffer , U8_A_U8(checkValueOffest,1))), HIGHEST_ORDER_BYTES) != ECP_CMP_RESULT_EQUAL)
			{
				//throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);		
				keyManager_abortLogicManagerThorwError(SW_INVALID_KEY_CHECK_VALUE);
				return;
			}
}

void AnalysePutKeyResponseForRSA(jref keySetRef, RAM_U8_P bKeyCheckTag)
{
			u8 								i,	j, index;
			u8								bKeyType;
			u8								sendReponStart;
			u8								sendOutDataLen;
			u8 								getKeyIdNum;
			u8								getKeyCheckTag;
			u8				        ICV[8];	
			u8								Key[16];
			u8								PaddingByte[8] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00};
			u16								bKeyLen, tmpKeyLen;
			//RAM_U32_P					getKeyData;
			u8							getKeyData[RSA_CRT_KEY_LENGTH]
			PN8					keySetObjcet;

			#define						DES_KEY_VALUE						 	0x00
			#define 					INIT_VALUES				    		0x00
			#define 					HIGHEST_ORDER_BYTES     	0x03
	
			sendOutDataLen		=	INIT_VALUES;			
			sendReponStart		=	OFFSET_CDATA;			
			j									=	INIT_VALUES;
			bKeyType					=	KEY_TYPE_RSA_PIVATE;
			keySetObjcet			=	MEMtPN8(locateObjectInMemory(keySetRef));
			getKeyCheckTag		=	PX_R_U8(bKeyCheckTag);
			getKeyIdNum			  =	keySet_getKeyNum(keySetObjcet);

			APDUBuffer[sendReponStart]     =	keySet_getKeyVersion(keySetObjcet);
			U8_INC(sendOutDataLen);
		
			if(getKeyCheckTag)
			{
					for(i=getKeyIdNum; i<=2; U8_INC(i))
					{        
							index			=	INIT_VALUES;
							ecpMem_Set(Key, INIT_VALUES, 0x10);
							ecpMem_Set(ICV, INIT_VALUES, 0x08); //有些平台计算完一次会改变
						
							bKeyLen		=	keyData_getKeyDataById(keySetObjcet, getKeyIdNum, bKeyType, getKeyData);
							tmpKeyLen	=	bKeyLen;
							
							ecpMem_Copy(bTempArr, getKeyData, bKeyLen);
							do{
							ecpMem_Set(U32_A_U16(bTempArr , U16_A_U8(bKeyLen , index)), PaddingByte[index], 0x01);
							U8_INC(index);
							U8_INC(tmpKeyLen);
							}while(U8_R_U8(tmpKeyLen , 8));
					
							Triple_DES((MEMtPX8(bTempArr), MEMtPX8(Key), MEMtPX8(bTempArr), tmpKeyLen, MEMtPX8(ICV), CBC_Encrypt, SENSITIVE_DATA_MASKED);
					
							ecpMem_Copy(&APDUBuffer[U8_A_U8(U8_A_U8(OFFSET_CDATA, 1) , j)], MEMtPX8(U32_A_U16(bTempArr , U16_S_U8(tmpKeyLen, 8))), HIGHEST_ORDER_BYTES);
							j					   		 =	U8_A_U8(j, HIGHEST_ORDER_BYTES);
							sendOutDataLen	 =	U8_A_U8(sendOutDataLen, HIGHEST_ORDER_BYTES);
							bKeyType				 =	KEY_TYPE_RSA_PUBLIC;
					}
			}
	
			apdu_ApduSetOutgoingAndSend(sendReponStart, sendOutDataLen);
}
#endif

#if GP__KEY_MANAGER__STORE_DGI_KEY
/*
*@description:
*
*@see        :
*
*@function   :storeDGIKeyData
*
*@param      :jref keySetRef
*@param      :u16 curDGILen
*@param      :u8 keyId
*@param      :u8 keyLen
*@param      :u8 DGIType
*@param      :PX8 keyCheckValueOff
*@param      :u8 enterNum
*
*@return     :u16
*@history    :modify by xuxiaojun@20220519: merged from EPX\v3.0\DEVELOP:增加参数keyLen
*            :modify by xuxiaojun@20220604 测试用例：704 gpUICC2Compliance203:  km_APDU_storeData_AES_success_SSD_DV_AES16_scp03_b4_5d_20
*/
static u16 storeDGIKeyData(jref keySetRef, u16 curDGILen, u8 keyId, u8 keyLen, u8 DGIType, PX8 keyCheckValueOff, u8	enterNum)
{

			u8					bkeyCheckValueOff;
			u16					DGILen;

			#define				KEY_COMPOMENT_TYPE_LENGTH   0x03
			#define				RSA_DGI_TYPE				0x0000
			#define				DGI_STORE_TYPE_NEW			0x00

			DGILen				=	U8t16(APDUBuffer[U8_INC(g_keyDataOffest)]);//指令中密文的长度，对于AES的0x18长度的key来说，这里DGILen的值为0x20
			curDGILen			=	DGILen;
			if(RSA_N_DGI != g_putKeyCurrState)
			{
					decryptKeyDataByDEK(g_keyDataOffest, DGILen, AnalyseEncKeyType()); // modify by EPX3.0
			}
			
			//检查CheckValue
			if(MEM_NULL != keyCheckValueOff)
			{
				bkeyCheckValueOff   =   PX_R_U8(keyCheckValueOff);
				if (DGIType == KEY_SCP_81_TYPE) {
					bkeyCheckValueOff	=	U8_A_U8(bkeyCheckValueOff, U8_M_U8(U8_S_U8(enterNum, 1), 0x16));
				}
//					if(APDUBuffer[RSA_DGI_LEN_OFF] <= 0x12)//SCP81 KEY ,只有一组B9
//					{
//						encryptKeyCheckValueByDEK(g_keyDataOffest, bkeyCheckValueOff, MEM_NULL, 0, KEY_SCP_81_TYPE, DGILen);
//					}else{
						encryptKeyCheckValueByDEK(g_keyDataOffest, bkeyCheckValueOff, MEM_NULL, 0, DGIType, keyLen); //校验check value时是对明文密钥长度进行计算  UL: 		km_APDU_putKeyAes_success_SSD_08_24B_scp03_9b_bc_b4
//					}
					
			}
//#if GP__AMENDMENT__B__FOR_MASK 
			setKeyData(keySetRef, DGI_STORE_TYPE_NEW, keyId, DGIType, curDGILen, keyLen, g_keyDataOffest);//modify by xuxiaojun@20220604: 由于前面计算key data所占用的字节数时是以密钥的明文长度来计算，所以这里存储时，传入的长度也应该是密钥的明文长度. 测试用例：704 gpUICC2Compliance203: km_APDU_storeData_AES_success_SSD_DV_AES24_scp03_b4_5d_21
//#else
//			setKeyData(keySetRef, DGI_STORE_TYPE_NEW, g_PutKeyId, DGIType, curDGILen, DGILen, g_keyDataOffest);
//#endif

			g_keyDataOffest			=	U16t8(U8_A_U16(g_keyDataOffest, DGILen));
			g_PutKeyRemaindLen		=	U16_S_U8(g_PutKeyRemaindLen, U8_A_U8(DGI_TAG_LENGTH_BYTE, DGILen));
			g_putKeyCurrState		=	RSA_DGI_TYPE;

			return  curDGILen;
}
/*
*@description:
*
*@see        :
*
*@function   :storeFormat2DGIKeyData
*
*@param      :u8 keyVersion
*@param      :jref keySetRef
*@param      :u16 curDGILen
*@param      :u8 keyId
*@param      :u8 keyLen
*@param      :u8 keyType
*@param      :PX8 keyCheckValueOff ---
*@param      :PX8 keyAccess
*@param      :PX8 keyUsageQ --- input, LV, 最大长度为KEY_USAGE_QUALIFIER_LENGTH
*
*@return     :u16
*@history    :modify by xuxiaojun@20220519: merged from EPX\v3.0\DEVELOP
*/
static u16 storeFormat2DGIKeyData(u8 keyVersion, jref keySetRef, u16 curDGILen, u8 keyId, u8 keyLen, u8 keyType, PX8 keyCheckValueOff, PX8 keyAccess,PX8 keyUsageQ)
{

			u8					bkeyCheckValueOff;
			u16					DGILen;
			u8					currentKeyState;
			#define				KEY_COMPOMENT_TYPE_LENGTH   0x03
			#define				RSA_DGI_TYPE				0x0000
			#define				DGI_STORE_TYPE_NEW			0x00
			#define				DGI_STORE_TYPE_UPDATE		0x01

			DGILen				=	U8t16(APDUBuffer[U8_INC(g_keyDataOffest)]); // the length of '8113'
			curDGILen			=	DGILen;
			currentKeyState     =   DGI_STORE_TYPE_NEW;
			decryptKeyDataByDEK(g_keyDataOffest, DGILen, AnalyseEncKeyType());//decryptKeyDataByDEK(g_keyDataOffest, DGILen, 0);

			//检查CheckValue
			if(MEM_NULL != keyCheckValueOff)
			{
				bkeyCheckValueOff   =   PX_R_U8(keyCheckValueOff);

				encryptKeyCheckValueByDEK(g_keyDataOffest, bkeyCheckValueOff, MEM_NULL, 0, keyType, keyLen);

			}
			if((keyVersion != 0)&&(keyData_getKeyData(g_bSDId[selected_channel], keyVersion, keyId, PXtMEM(NULL))!= 0))
			{
				currentKeyState    =    DGI_STORE_TYPE_UPDATE;
			}
			setKeyData(keySetRef, currentKeyState, keyId, keyType, curDGILen, keyLen, g_keyDataOffest);
			keyData_setKeyUsageQualifier(MEMtPN8(locateObjectInMemory(keySetRef)), keyId, &keyUsageQ[0]);
			keyData_setKeyAccess(MEMtPN8(locateObjectInMemory(keySetRef)), keyId, &keyAccess[0]);

			g_keyDataOffest			=	U16t8(U8_A_U16(g_keyDataOffest, DGILen));
			g_PutKeyRemaindLen		=	U16_S_U8(g_PutKeyRemaindLen, U8_A_U8(DGI_TAG_LENGTH_BYTE, DGILen));
			g_putKeyCurrState		=	RSA_DGI_TYPE;

			return  curDGILen;
}
#if      GP__KEY_MANAGER__ECC_KEY
/*
*@description:
*
*@see        :
*
*@function   :storeEccDGIKeyData
*
*@param      :jref keySetRef
*@param      :u8 keyId
*@param      :u8 keyLen
*@param      :u16 DGIType
*
*@return     :u16
*@history    :modify by xuxiaojun@20220519: merged from EPX\v3.0\DEVELOP
*/
static u16 storeEccDGIKeyData(jref keySetRef, u8 keyId, u8 keyLen, u16 DGIType)
{

			u16					curDGILen;
			u8	   				keyVersionState,tmpKID;
			memref  			keyDataStart;

			#define				KEY_COMPOMENT_TYPE_LENGTH   0x03
			#define				DGI_STORE_TYPE_NEW			0x00

			curDGILen =	U8t16(APDUBuffer[U8_INC(g_keyDataOffest)]);

			if(ECC_PrivateKey_DGI == g_putKeyCurrState)
			{
				decryptKeyDataByDEK(g_keyDataOffest, curDGILen, AnalyseEncKeyType());
			}


			if(DGIType > ECC_PrivateKey_DGI_TYPE)// key+ domain 且不是第一条指令进来才考虑keyId == MEM_NULL， key部分信息第一条指令可以发完，这里仅考虑domain不完整导致非第一条指令下发的数据传入keyId = MEM_NULL
			{
				tmpKID  = ECC_DOMAIN_KID;
			}
			else
			{
				tmpKID  = keyId;
			}

			if(0 != keyData_getKeyDataById(MEMtPN8(locateObjectInMemory(keySetRef)), tmpKID, DGIType, NULL))//modify by xuxiaojun@20220604 需要判断当前是属于更新key，还是属于新增key。测试用例：704 gpUICC2Compliance203： km_APDU_storeData_ECC_success_update_token_32B_P256_scp03_9b_ef_21
			{
				keyVersionState    =    TAG_UPDATE_KEY;
			}
			else
			{
				keyVersionState    =    TAG_NEW_KEY;
			}

			keyDataStart	=	keyData_prepareKeyData(MEMtPN8(locateObjectInMemory(keySetRef)), keyVersionState, tmpKID, DGIType, keyLen);  //private key是被加密过的，所以明文的密钥数据长度是keyLen。  C_sm_PSO_ERROR_certificateChainsNotSupported_bySD_521_a1_33_4a

			SO_storeSecData(MEMtPX8(APDUBuffer), g_keyDataOffest, keyLen,  MEMtPM8(keyDataStart));
			memRefresh();

			g_keyDataOffest			=	U16t8(U8_A_U16(g_keyDataOffest, curDGILen));
			g_PutKeyRemaindLen		=	U16_S_U8(g_PutKeyRemaindLen, U8_A_U8(DGI_TAG_LENGTH_BYTE, curDGILen));
			g_putKeyCurrState		=	RSA_DGI_TYPE;

			return  curDGILen;
}
/*
*@description:
*
*@see        :
*
*@function   :checkEccKeyLen
*
*@param      :u16 curLen
*@param      :u8 keyType
*
*@return     :void
*@history    :modify by xuxiaojun@20220519: merged from EPX\v3.0\DEVELOP
*/
void checkEccKeyLen(u16 curLen, u8 keyType)
{
	u8     tmpLen   =  0;

	if((keyType >= ECC_PublicKey_DGI_TYPE)&&(keyType <= ECC_CurvePara_K_DGI_TYPE))
	{
		if(keyType == ECC_CurvePara_K_DGI_TYPE)//B7--> K最大支持2字节  一般是1字节
		{
			if((curLen != 0x01)&&(curLen != 0x02))
			{
				throw_error(SW_WRONG_DATA);
			}
		}
		else if((keyType == ECC_PublicKey_DGI_TYPE)||(keyType == ECC_CurvePara_G_DGI_TYPE))//B0  B5 --> 04+x+y
		{
			tmpLen = (curLen -1)/2;
		}
		else // B1 B2 B3 B4 B6
		{
			tmpLen  = curLen;
		}

		if((tmpLen != 0)&&(tmpLen != ECC_256_BYTELEN)&&(tmpLen != ECC_384_BYTELEN)
			#if WITH_ECC_KEY_512
		    &&(tmpLen != 	ECC_512_BYTELEN)
			#endif
			#if WITH_ECC_KEY_521
			&&(tmpLen != ECC_521_BYTELEN)
			#endif
		)
		{
			throw_error(SW_WRONG_DATA);
		}

		if(tmpLen != 0)
		{
			if((g_EccKeyParaLen != 0)&&(g_EccKeyParaLen != tmpLen))
			{
				throw_error(SW_WRONG_DATA);
			}

			g_EccKeyParaLen = tmpLen;
		}
	}
}
#endif //end of GP__KEY_MANAGER__ECC_KEY
#endif //end of GP__KEY_MANAGER__STORE_DGI_KEY


#if GP__KEY_MANAGER__STORE_DGI_KEY
/*
*@description:
*
*@see        :
*
*@function   :setStoreKeyData
*
*@param      :PN8 sdDataObj --- input
*@param      :u8 keyVersion --- input
*@param      :jref keySetRef --- input
*@param      :u8 keyDataLen --- input
*@param      :u8 keyDataOff --- input
*@param      :PX8 keyId --- input, 可能包括多支key的key id
*@param      :PX8 keyLen --- input, 可能包括多支key的key length， 与keyId的有效性是一样的
*@param      :PX8 keyType --- input, 可能包括多支key的key type， 与keyId的有效性是一样的
*@param      :PX8 keyCheckValueOff --- input, 可能包括多支key的key check， 与keyId的有效性是一样的
*@param      :PX8 keyAccess --- input, 可能包括多支key的key access， 与keyId的有效性是一样的
*@param      :PX8 keyUsageQ --- input, 可能包括多支key的key length， 与keyId的有效性是一样的。每一支对应的格式为LV
*@param      :u8 keyFormat --- input
*
*@return     :void
*@history    :modify by xuxiaojun@20220519: merged from EPX\v3.0\DEVELOP：增加参数sdDataObj, keyVersion, keyLen, keyAccess, keyUsageQ, keyFormat
*/
void setStoreKeyData(PN8 sdDataObj,u8 keyVersion, jref keySetRef, u8 keyDataLen, u8 keyDataOff, PX8 keyId, PX8 keyLen, PX8 keyType, PX8 keyCheckValueOff, PX8 keyAccess, PX8 keyUsageQ,u8 keyFormat)
{

			u8					benterNum;
			u16					curDGILen;
			u8                  keyNum;

			#define      		INIT_CURDGI_LEN				0x00
			#define				INIT_ENTER_NUM				0x01
			#define				DGI_TYPE_LEN				0x02
			#define				INIT_KS_DGI_ID				0x02
			#define				RSA_DGI_TYPE				0x0000

			benterNum				=	INIT_ENTER_NUM;
			keyNum                  =   0;
			g_putKeyCurrState		=	RSA_DGI_TYPE;
			curDGILen				=	INIT_CURDGI_LEN;
			g_PutKeyRemaindLen		=	U8t16(keyDataLen);
			g_keyDataOffest			=	keyDataOff;

			while(g_PutKeyRemaindLen > 0)
			{
				switch(g_putKeyCurrState)
				{
					case	RSA_DGI_TYPE:

							g_putKeyCurrState	=	U8t16(APDUBuffer[U8_INC(g_keyDataOffest)]);
							g_putKeyCurrState	=	U16_A_U8(U8_LEFT_SHIFT(g_putKeyCurrState), APDUBuffer[U8_INC(g_keyDataOffest)]);
							break;
					case    RSA_CRT_TAG:// key内容后又跟了00B9 CRT
							g_PutKeyRemaindLen -= 2; //偏开2字节tag，指向L
							return;
					case 	RSA_N_DGI: // 0010

							curDGILen 			=	storeDGIKeyData(keySetRef, curDGILen, keyId[keyNum], keyLen[keyNum], RSA_N_DGI_TYPE, MEM_NULL, benterNum);
							U8_INC(keyNum); // modify by xuxiaojun@20220604 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_7a_78_36_scp03_b4_e4_8e
							break;
					case 	RSA_E_DGI: // 0011

							curDGILen 			=	storeDGIKeyData(keySetRef, curDGILen, keyId[keyNum], keyLen[keyNum], RSA_E_DGI_TYPE, MEM_NULL, benterNum);
							g_KeepRefForSplit   =   0;
							break;
					case 	RSA_D_DGI:

							curDGILen 			=	storeDGIKeyData(keySetRef, curDGILen, keyId[keyNum], keyLen[keyNum], RSA_D_DGI_TYPE, MEM_NULL, benterNum);
							g_KeepRefForSplit   =   0;
							break;
					case 	KS_DGI: // 8113
// modify by xuxiaojun@20220604 可能会通过store key设置多个Symmetric Scheme in Secret Key Format, 这里不能修改g_PutKeyId的值。 测试用例：704 gpUICC2Compliance203:  km_APDU_storeData_AES_success_SSD_DV_AES16_scp03_b4_5d_20
//							if(g_PutKeyId != INIT_KS_DGI_ID)
//							{
//									g_PutKeyId	=	INIT_KS_DGI_ID;
//							}else{
//#if UICC1_0
//									U8_DEC(g_PutKeyId);
//#else
//									U8_INC(g_PutKeyId);
//#endif //		UICC1_0
//							}
							if(keyFormat == KEY_FORMAT_02)
							{
								curDGILen 	=	storeFormat2DGIKeyData(keyVersion,keySetRef, curDGILen, keyId[keyNum], keyLen[keyNum], keyType[keyNum], &keyCheckValueOff[keyNum],&keyAccess[keyNum],&keyUsageQ[keyNum*KEY_USAGE_QUALIFIER_LENGTH]);//modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32
							}
							else
							{
								curDGILen 	=	storeDGIKeyData(keySetRef, curDGILen, keyId[keyNum], keyLen[keyNum], keyType[keyNum], &keyCheckValueOff[keyNum], benterNum);
							}

							U8_INC(benterNum); //0x8113,需要对每组checkValue进行检查
							U8_INC(keyNum);
							if(g_PutKeyRemaindLen == 0)//对称秘钥较短 不考虑级联
							{
								g_KeepRefForSplit = 0;
							}
							break;
					case 	RSA_CRT_IQP_DGI:

							curDGILen 			=	storeDGIKeyData(keySetRef, curDGILen, keyId[keyNum], keyLen[keyNum], RSA_CRT_IQP_DGI_TYPE, MEM_NULL, benterNum);
							break;
					case 	RSA_CRT_DQ_DGI:

							curDGILen 			=	storeDGIKeyData(keySetRef, curDGILen, keyId[keyNum], keyLen[keyNum], RSA_CRT_DQ_DGI_TYPE, MEM_NULL, benterNum);
							break;
					case 	RSA_CRT_DP_DGI:

							curDGILen 			=	storeDGIKeyData(keySetRef, curDGILen, keyId[keyNum], keyLen[keyNum], RSA_CRT_DP_DGI_TYPE, MEM_NULL, benterNum);
							break;
					case 	RSA_CRT_Q_DGI:

							curDGILen 			=	storeDGIKeyData(keySetRef, curDGILen, keyId[keyNum], keyLen[keyNum], RSA_CRT_Q_DGI_TYPE, MEM_NULL, benterNum);
							break;
					case 	RSA_CRT_P_DGI:

							curDGILen 			=	storeDGIKeyData(keySetRef, curDGILen, keyId[keyNum], keyLen[keyNum], RSA_CRT_P_DGI_TYPE, MEM_NULL, benterNum);
							g_KeepRefForSplit	=	INIT_KEEP_REF_FOR_SPLIT;
							break;
#if GP__KEY_MANAGER__ECC_KEY
					case    ECC_CurvePara_P_DGI:
								storeEccDGIKeyData(keySetRef, keyId[keyNum], keyLen[keyNum], ECC_CurvePara_P_DGI_TYPE);
								break;
					case    ECC_CurvePara_A_DGI:
								storeEccDGIKeyData(keySetRef, keyId[keyNum], keyLen[keyNum], ECC_CurvePara_A_DGI_TYPE);
								break;
					case    ECC_CurvePara_B_DGI:
								storeEccDGIKeyData(keySetRef, keyId[keyNum], keyLen[keyNum], ECC_CurvePara_B_DGI_TYPE);
								break;
					case    ECC_CurvePara_G_DGI:
								storeEccDGIKeyData(keySetRef, keyId[keyNum], keyLen[keyNum], ECC_CurvePara_G_DGI_TYPE);
								break;
					case    ECC_CurvePara_N_DGI:
								if(((g_storeEccKeyMode == ECC_StoreKeyMode_KEY_AND_DOMAIN)
									||(g_storeEccKeyMode == ECC_StoreKeyMode_F0_AND_DOMAIN))
									&&(ValueBit(g_markEccKey,ECC_CurvePara_K_BIT7) == 0))
								{
									g_KeepRefForSplit = 0;
								}
								storeEccDGIKeyData(keySetRef, keyId[keyNum], keyLen[keyNum], ECC_CurvePara_N_DGI_TYPE);
								break;
					case    ECC_CurvePara_K_DGI:
								storeEccDGIKeyData(keySetRef, keyId[keyNum], keyLen[keyNum], ECC_CurvePara_K_DGI_TYPE);
								if((g_storeEccKeyMode == ECC_StoreKeyMode_KEY_AND_DOMAIN)
									||(g_storeEccKeyMode == ECC_StoreKeyMode_F0_AND_DOMAIN))
								{
									g_KeepRefForSplit = 0;
								}

								break;
					case    ECC_PublicKey_DGI:
								storeEccDGIKeyData(keySetRef, keyId[keyNum], keyLen[keyNum], ECC_PublicKey_DGI_TYPE);
								keyData_setKeyUsageQualifier(MEMtPN8(locateObjectInMemory(keySetRef)), keyId[0], &keyUsageQ[0]);
								keyData_setKeyAccess(MEMtPN8(locateObjectInMemory(keySetRef)), keyId[0], &keyAccess[0]);
								setKeyType(MEMtPN8(locateObjectInMemory(keySetRef)),ECC_PublicKey_DGI_TYPE) ;
#if GP__SCP__SCP11__FOR_MASK
								if (IsSCP11Key(keyVersion))
								{
									SCP11InitKey(sdDataObj, keyVersion, keyVersion, keyId[0], ECC_PublicKey_DGI_TYPE, TAG_NEW_KEY);
								}
#endif // GP__SCP__SCP11__FOR_MASK
								if(g_storeEccKeyMode != ECC_StoreKeyMode_KEY_AND_DOMAIN)
								{
									g_KeepRefForSplit = 0;
								}
								break;
					case    ECC_PrivateKey_DGI:
								storeEccDGIKeyData(keySetRef, keyId[keyNum], keyLen[keyNum], ECC_PrivateKey_DGI_TYPE);
								keyData_setKeyUsageQualifier(MEMtPN8(locateObjectInMemory(keySetRef)), keyId[0], &keyUsageQ[0]);
								keyData_setKeyAccess(MEMtPN8(locateObjectInMemory(keySetRef)), keyId[0], &keyAccess[0]);
								setKeyType(MEMtPN8(locateObjectInMemory(keySetRef)),ECC_PrivateKey_DGI_TYPE) ;
#if GP__SCP__SCP11__FOR_MASK
								if (IsSCP11Key(keyVersion))
								{
									SCP11InitKey(sdDataObj, keyVersion, keyVersion, keyId[0], ECC_PrivateKey_DGI_TYPE, TAG_NEW_KEY);
								}
#endif // GP__SCP__SCP11__FOR_MASK
								if(g_storeEccKeyMode != ECC_StoreKeyMode_KEY_AND_DOMAIN)
								{
									g_KeepRefForSplit = 0;
								}
							break;
#endif// GP__KEY_MANAGER__ECC_KEY
					default:
							throw_error(SW_WRONG_DATA);
							break;
				}
			}
}
/*
*@description:
*
*@see        :
*
*@function   :storeDGIKey
*
*@param      :PN8 sdDataObj
*@param      :u8 keyVersion
*@param      :jref keySetRef
*@param      :u8 keyDataLen
*@param      :u8 keyDataOff
*@param      :PX8 keyId
*@param      :PX8 keyLen
*@param      :PX8 keyType
*@param      :PX8 keyCheckValueOff
*@param      :PX8 keyAccess
*@param      :PX8 keyUsageQ
*@param      :u8 keyFormat
*
*@return     :void
*@history    :modify by xuxiaojun@20220519: merged from EPX\v3.0\DEVELOP：增加参数sdDataObj, keyVersion, keyLen, keyAccess, keyUsageQ, keyFormat
*/
void storeDGIKey(PN8 sdDataObj)
{
	u8		currKeyVersion;
	u8		storeKeyLen;
	u8		crtRemaindLen;
	u8		keyDataLen;
	u8		keyCheckValueOff[3];
	u8      keyType[7];// ECC F0 , B2~B7
	u8      keyLen[7]; // ECC F0 , B2~B7
	u8      keyId[5]; // RSA CRT
	u16		keySetRef;
	u8      keyFormat;
	u8      keyVersion;
	u8      keyAccess[5];// RSA CRT
	u8      keyUsageQ[15];//按照GP2.3  最多可以有2字节, KEY_USAGE_QUALIFIER_LENGTH*5 + 5(LV格式) modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32

//			ecpMem_Set(keyAccess, 0x00, sizeof(keyAccess));
//			ecpMem_Set(keyUsageQ, 0x00, sizeof(keyUsageQ));

	ecpMem_Fill(keyLen, 0x00, sizeof(keyLen));

	ecpMem_Fill(keyCheckValueOff, 0x00, sizeof(keyCheckValueOff));

	g_keyDataOffest	=	RSA_DGI_LEN_OFF; // point to the length of the first '00B9'

	storeKeyLen		=	APDUBuffer[OFFSET_LC];

	crtRemaindLen	=	APDUBuffer[RSA_DGI_LEN_OFF]; // the length of '00B9'

	keyDataLen		=	U8_S_U8(storeKeyLen, crtRemaindLen);//storeKeyLen < crtRemaindLen + 3的情况抛异常？

	currKeyVersion  =   0;

	if(storeKeyLen < U8_A_U8(crtRemaindLen,DGI_TAG_LENGTH_BYTE))//数据不是一个完整的00B9
	{
		throw_error(SW_WRONG_DATA);
	}

	//keyDataLen					=	U8_S_U8(keyDataLen, DGI_TAG_LENGTH_BYTE);

	while(U8_A_U8(storeKeyLen,OFFSET_CDATA) > g_keyDataOffest)
	{
		ecpMem_Set(keyAccess, 0x00, sizeof(keyAccess));

		ecpMem_Set(keyUsageQ, 0x00, sizeof(keyUsageQ));

		keySetRef		=	analyseCRTCompoment(g_KeepRefForSplit,sdDataObj, &keyId[0], &keyLen[0], &keyType[0], &keyCheckValueOff[0],&keyFormat,&keyVersion,&keyAccess[0],&keyUsageQ[0]);

		g_KeepRefForSplit  =   keySetRef; // 考虑多条store key指令的情况

		keyDataLen	=	U8_S_U8(U8_A_U8(storeKeyLen,OFFSET_CDATA), g_keyDataOffest);

		if(0 == keyDataLen)//CRT/DGI结束    case 1: 只有00B9 没有后续的key内容 /other:
		{
				//g_KeepRefForSplit 	=	keySetRef;
		}
		else
		{									//00B9 后还有key的DGI
				if(keyFormat == KEY_FORMAT_03)
				{
						if  (IsSCP11Key(keyVersion))		//不改变原来逻辑，scp11需要传入keyVersion
						{
							setStoreKeyData(sdDataObj,keyVersion,keySetRef, keyDataLen, g_keyDataOffest, &keyId[0], &keyLen[0], &keyType[0], &keyCheckValueOff[0],&keyAccess[0],&keyUsageQ[0],keyFormat);
						}
						else
						{
							setStoreKeyData(sdDataObj,0,keySetRef, keyDataLen, g_keyDataOffest, &keyId[0], &keyLen[0], &keyType[0], &keyCheckValueOff[0],&keyAccess[0],&keyUsageQ[0],keyFormat);
						}
				}
				else
				{
					keySetRef   	=  keySet_getKeySet(sdDataObj, keyVersion);
					if(keySetRef)
					{
						currKeyVersion   =  keyVersion;  //替换密钥   ，这里是不是还需要判断替换密钥的长度和类型是否合法？？
					}
					keySetRef 	=   newKeySetByKeyVersion(sdDataObj, keyFormat, currKeyVersion, keyVersion, SO_calSecDataAreaLen(g_PutkeyDataLen));

					setStoreKeyData(sdDataObj,currKeyVersion,keySetRef, keyDataLen, g_keyDataOffest, &keyId[0], &keyLen[0], &keyType[0], &keyCheckValueOff[0],&keyAccess[0],&keyUsageQ[0],keyFormat);


					g_KeepRefForSplit = 0;// security channel key  单条数据可以发完  ，未考虑多条的情况
				}
		}
	}
}

void storeCRTKeyForSplit(PN8 sdDataObj)
{

			u8		keyDataLen;
			u16		keySetRef;

			keyDataLen	  		=		APDUBuffer[OFFSET_LC];

			if(g_KeepRefForSplit)
			{
					keySetRef	=	g_KeepRefForSplit;
			}else{
					throw_error(SW_WRONG_DATA);
			}

			setStoreKeyData(sdDataObj,0,keySetRef, keyDataLen, OFFSET_CDATA, MEM_NULL, MEM_NULL, MEM_NULL, MEM_NULL,MEM_NULL,MEM_NULL,KEY_FORMAT_03);
}
#endif // GP__KEY_MANAGER__STORE_DGI_KEY

#define KEY_KIK 3
/*
 * 判断keyVersion和keyId所指示的SCP81Key是否存在
 */
//u8 checkSCP81KeyExist(u8 sdId, u8 keyVersion, u8 keyId)
//{
//	jref				keySetRef;
//	PN8					sdDataObject;
//	PN8					keySetObjcet;
//
//	sdDataObject		=	MEMtPN8(getSdDataObjBySdid(sdId));
//	keySetRef			=	keySet_getKeySet(sdDataObject, keyVersion);
//	if (REF_NULL == keySetRef)
//	{
//		return FALSE;
//	}
//	keySetObjcet		=	MEMtPN8(locateObjectInMemory(keySetRef));
//
//#if GP__KEY_MANAGER__FORMAT3
//	if (KEY_FORMAT_03 == keySet_getStoreKeyFormmat(keySetObjcet))
//	{
//		PN8 keyLocation;
//		keyLocation = MEMtPN8(fm03_locateKeyType(keySetObjcet, keyId));
//
//		if (REF_NULL == keyLocation)
//		{
//			return FALSE;
//		}
//	}
//#endif
//	return TRUE;
//}
/*
 * merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
 */
void decryptKeyDataByDEK(u8 offest, u8 keyDataLen, u8 Type)
{

	u16 kenlen = 0;
	u8  mode;
#if JAVA_CARD_UICC
	u8 keyType;
#endif
	//u8 curSDId = getCurrentSDId();
	//if( ((KEY_AES_TYPE == Type) && (SCP_03 == sdData_getScpType(MEMtPF8(getCurrentSdObj(IS_NO_NEED_CHECK_SD))))) ||
	//sdData_getScpType获取出来的是指定sd的sddata中的scpType，而不是当前通道的scpType， 用例：ul704-scp81：sm_scp81_script_target_SSD_without_SCP03_16B_keys_19_e9_42
	if( ((KEY_AES_TYPE == Type) && (SCP_03 == g_current_SCP_Type)) || // todo: SCP_11安全通道建立之后，再给SD put AES KEY的话，这里可能会有问题
		((KEY_AES_TYPE == Type) && (bRamBuffer[PROCESS_LEVEL] == ROCESS_LEVEL_OTA )) )
	{
#if JAVA_CARD_UICC // 代码分层 by xuxiaojun@20220524
		//if(IsSCP80Key(bRamBuffer[REMOTE_SCP_KEY_VERSION]))//SCP80 AESkey
		if((g_SCP02SDID == APP_NULL)&&(bRamBuffer[PROCESS_LEVEL] == ROCESS_LEVEL_OTA))// RAM指令中没有自己开scp会话  用SCP80的KIK解密, mergerd from EPX\v3.0\DEVELOP:增加ROCESS_LEVEL_OTA的判断
		{
			if(IsSCP80Key(bRamBuffer[REMOTE_SCP_KEY_VERSION]))//SCP80 AESkey
			{
				kenlen = getScp80Key(getCurrentSDId(), bRamBuffer[REMOTE_SCP_KEY_VERSION], KEY_KIK, PXtMEM(g_current_SCP_DEK_KEY));
			}
			else //if(IsSCP81Key(bRamBuffer[REMOTE_SCP_KEY_VERSION]))
			{
				kenlen = getScp80Key(getCurrentSDId(), bRamBuffer[REMOTE_SCP_KEY_VERSION], 0x02, PXtMEM(g_current_SCP_DEK_KEY));
			}
//			ecpMem_Set(bTempArr, 0x00, 16);
//			SymCipherDofinal(PX8_A_U8(APDUBuffer, offest), keyDataLen, PX8_A_U8(APDUBuffer, offest), MEMtPX8(g_current_SCP_DEK_KEY), U8_M_U8(kenlen,8), bTempArr, NULL, ALG_AES_BLOCK_128_CBC_NOPAD, U8_OR_U8(SYM_CBC_MODE, MODE_DECRYPT), 0x00, ALG_AES);
//			return ;
		}
		else
#endif
		{
			kenlen = g_SCP_dekKeyLen; //根据RAM中开的会话秘钥取长度和值
		}
		if(U8_R_U8(keyDataLen,AES_BLOCKSIZE))// key长度不是16的整数倍 ，密文是按16整数倍加密的  102226 8.2.1.5.1  an AES key as encryption key (DEK) ：Length of the key in bytes ：16, 24 or 32 for AES  / 16 or 24 for triple DES
		{
			keyDataLen = U8_M_U8(U8_D_U8(U8_A_U8(keyDataLen,AES_BLOCKSIZE),AES_BLOCKSIZE),AES_BLOCKSIZE);//keyDataLen += 8;
		}
#if VM_WITH_AES
		//SymCipherDofinal((PX8) (APDUBuffer + offest), keyDataLen, (PX8) (APDUBuffer + offest), g_current_SCP_DEK_KEY, SCP_KEY_LENGTH*8, AES_ECB_MODE_MASK, MODE_DECRYPT);
//#if GP__AMENDMENT__B__FOR_MASK
		ecpMem_Set(bTempArr, 0x00, 16);
		SymCipherDofinal(PX8_A_U8(APDUBuffer, offest), keyDataLen, PX8_A_U8(APDUBuffer, offest), MEMtPX8(g_current_SCP_DEK_KEY), U8_M_U8(kenlen,8), bTempArr, NULL, ALG_AES_BLOCK_128_CBC_NOPAD, U8_OR_U8(SYM_CBC_MODE, MODE_DECRYPT), 0x00, ALG_AES);
		//SymCipherDofinal(PX8_A_U8(APDUBuffer, offest), keyDataLen, PX8_A_U8(APDUBuffer, offest), MEMtPX8(g_current_SCP_DEK_KEY), U8_M_U8(SESSION_KEY_LENGTH,8), bTempArr, NULL, ALG_AES_BLOCK_128_CBC_NOPAD, U8_OR_U8(SYM_CBC_MODE, MODE_DECRYPT), 0x00, ALG_AES);
//#else
//		SymCipherDofinal(PX8_A_U8(APDUBuffer, offest), keyDataLen, PX8_A_U8(APDUBuffer, offest), MEMtPX8(g_current_SCP_DEK_KEY), U8_M_U8(SCP_KEY_LENGTH,8), NULL, NULL, ALG_AES_BLOCK_128_ECB_NOPAD, U8_OR_U8(SYM_ECB_MODE, MODE_DECRYPT), 0x00, ALG_AES);
//#endif

#endif
		return;
	}

	mode = ECB_Decrypt;
#if JAVA_CARD_UICC		 // modify by xuxiaojun@20220318 分层
	if(bRamBuffer[PROCESS_LEVEL] == ROCESS_LEVEL_OTA )
	{

		if((IF_IsHTTPCommand() == TRUE)&&(G_RAMSessionSdId == 0xFF))
		{

			kenlen = keyData_getKeyData(IF_GetScp81TriggerSD(), IF_GetScp81KeyVersionNum(), 0x02, PXtMEM(g_current_SCP_DEK_KEY));
			mode = CBC_Decrypt;// bug 514 http方式更新ISDRSCP81密钥失败
			if((8 == kenlen)&&(APDUBuffer[OFFSET_INS]==0xD8))
			{
				mode  = U8_OR_U8(mode , 0x40);
			}
			else if((0x18 == kenlen)&&(APDUBuffer[OFFSET_INS]==0xD8))
			{
				mode  =U8_OR_U8(mode , 0x80);
			}
			ecpMem_Set(bTempArr, 0x00, 8);
			Triple_DES(PX8_A_U8(APDUBuffer, offest), MEMtPN8(g_current_SCP_DEK_KEY), PX8_A_U8(APDUBuffer, offest), keyDataLen, MEMtPX8(bTempArr), mode, SENSITIVE_DATA_MASKED);

			return;
		}
		else
		{
			keyType = GetSCP80and02Key();
			if((keyType == 0x80) && (channelInfo.security_level[selected_channel] == SCF_80_AUTHENTICATED))// 表示当前触发的SD或其关联安全域存在SCP80 KEY
			{
				//kenlen = keyData_getKeyData(getCurrentSDId(), APDUBuffer[5], KEY_KIK, g_current_SCP_DEK_KEY);

				if(IsSCP80Key(APDUBuffer[5]))//SCP80 AESkey 当前put的key的SCP类型
				{
					kenlen = keyData_getKeyData(getCurrentSDId(), APDUBuffer[5], KEY_KIK, PXtMEM(g_current_SCP_DEK_KEY)); // bRamBuffer[REMOTE_SCP_SDID], bRamBuffer[REMOTE_SCP_KEY_VERSION]
				}
				else //if(IsSCP81Key(bRamBuffer[REMOTE_SCP_KEY_VERSION]))
				{
					kenlen = keyData_getKeyData(getCurrentSDId(), APDUBuffer[5], 0x02, PXtMEM(g_current_SCP_DEK_KEY));
				}
				if(kenlen == 0)
				{
					//kenlen = getScp80Key(getCurrentSDId(), bRamBuffer[REMOTE_SCP_KEY_VERSION], KEY_KIK, g_current_SCP_DEK_KEY);
					if(IsSCP81Key(bRamBuffer[REMOTE_SCP_KEY_VERSION]))
					{
						kenlen = getScp80Key(getCurrentSDId(), bRamBuffer[REMOTE_SCP_KEY_VERSION], 0x02, PXtMEM(g_current_SCP_DEK_KEY));
						mode = CBC_Decrypt;
						if((8 == kenlen)&&(APDUBuffer[OFFSET_INS]==0xD8))
						{
							mode  = U8_OR_U8(mode , 0x40);
						}
						else if((0x18 == kenlen)&&(APDUBuffer[OFFSET_INS]==0xD8))
						{
							mode  =U8_OR_U8(mode , 0x80);
						}
						ecpMem_Set(bTempArr, 0x00, 8);
						Triple_DES(PX8_A_U8(APDUBuffer, offest), MEMtPN8(g_current_SCP_DEK_KEY), PX8_A_U8(APDUBuffer, offest), keyDataLen, MEMtPX8(bTempArr), mode, SENSITIVE_DATA_MASKED);
						return;
					}
					else
					{
						kenlen = getScp80Key(getCurrentSDId(), bRamBuffer[REMOTE_SCP_KEY_VERSION], KEY_KIK, PXtMEM(g_current_SCP_DEK_KEY));
						if((8 == kenlen)&&(APDUBuffer[OFFSET_INS]==0xD8))
						{
							mode  = U8_OR_U8(mode , 0x40);
						}
						else if((0x18 == kenlen)&&(APDUBuffer[OFFSET_INS]==0xD8))
						{
							mode  =U8_OR_U8(mode , 0x80);
						}
					}
				}
			}
		}

	}
#endif
	Triple_DES(PX8_A_U8(APDUBuffer, offest), MEMtPM8(g_current_SCP_DEK_KEY), PX8_A_U8(APDUBuffer, offest), keyDataLen, bTempArr, mode, SENSITIVE_DATA_MASKED);
}

u8 GetCurSDSCP80and02Key(u8 SDId)
{
	PN8		sdDataObj;
	jref 				keyGroupRef;
//	u8   				tempkeyVersion;
	sdDataObj		=	MEMtPN8(getSdDataObjBySdid(SDId));

	if( 0xff != sdData_changeCurrentScpTypeAndOption(sdDataObj, SCP80_WITHOUT_IOPTION_MODE, 0x00))
	{
		keyGroupRef = sdData_getKeyGroupRef(sdDataObj, SCP_80, false);
		if (REF_NULL != keyGroupRef)
		{
			// check has SCP80 KeySet
			if (REF_NULL != keyGroup_getKeySetRef(MEMtPF8(locateObjectInMemory(keyGroupRef))))
			{
				return 0x80;
			}
		}
	}
	//sdDataObject		=	(NUM_TO_NPTR)getSdDataObjBySdid(appId);
	/*tempkeyVersion	= keySet_getDefaultKeyVersion(sdDataObj);

	if( (tempkeyVersion >= 0x20) && (tempkeyVersion <= 0x2f) )
	{
		return 0x02;
	}*/
	return 0;
}

u8 GetSCP80and02Key()
{
	u8 SDId = getCurrentSDId();
	u8 result = GetCurSDSCP80and02Key(SDId);
	while((result == 0) && (SDId != 0xFF))
	{
		SDId = getAssociateSDId(SDId);
		result = GetCurSDSCP80and02Key(SDId);
	}
	return result;
}

//#define     SetBit(a,b)         (a) |= (1 << (b))  //define  repeation
void encryptKeyCheckValueByDEK(u8 keyDataOffest, u8	checkValueOffest, PR8 bKeyCheckTag, u8	keyComNum, u8 keyType, u16 keyLen)
{

	#define 						INIT_VALUES				    		0x00
	#define 						HIGHEST_ORDER_BYTES   		0x03
	#define							KEYCHECK_LEN							0x03

	u8									keyCheckLen;
	u8									mode;

	keyCheckLen				= APDUBuffer[checkValueOffest];
	mode					= ECB_Encrypt;
#if (EUICC_ONLY||SE_AND_EUICC)
	if(bRamBuffer[PROFILE_APDU_MODE] == TRUE)//profile 下载过程中的指令都是明文 不需要解密和计算checkvalue
	{
		return;
	}
#endif
	if ((JcvmConfig_GetSEState() == CARD_OP_READY)&&(channelInfo.security_level[selected_channel] == SCF_NO_AUTHENTICATED))// CARD_OP_READY并且没有鉴权时 明文putkey ，增加此判断是因为AESkey无法通过本地的方式预置 ，scp02鉴权用的是DES 无法保护更高级别的AESkey
	{
		return;
	}

	//storedata写秘钥 8F01格式，并不存在 keyCheckLen，因此这里不需要判断byxuweihua20221116
	if((APDUBuffer[OFFSET_INS]==0xE2)&&(APDUBuffer[OFFSET_CDATA]==0x8F)&&(APDUBuffer[OFFSET_CDATA+1]==0x01))
	{
	
	}
	else
	{
		if(!keyCheckLen)//当checkvalue为长度0时，不校验
		{
			if(keyType <= KEY_AES_TYPE)	// （RSA非对称秘钥）A0  A1	，非对称秘钥的checkvalue len = 0 ,其余（DES/AES）暂时统一做限制
			{
				keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);//GP功能补充测试脚本_接触_工程集/东信/apdu命令要求/put key指令测试/6.1.2.4.6
			}
			return;
		}else if((KEYCHECK_LEN != keyCheckLen) && (APDUBuffer[OFFSET_INS]==0xD8)){// modified by tulei: store key data
					keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
		}
	}

	if(MEM_NULL != bKeyCheckTag)//记录带checkValue keyid ; modified by zm; null pointer exception
	{
		PX_W_U8(bKeyCheckTag, (SetBit(PX_R_U8(bKeyCheckTag),keyComNum)));
	}

	if((KEY_DES_TYPE	<=	keyType) && (KEY_DES_CBC_MODE >= keyType))
	{
		ecpMem_Set(TEMPBuffer, INIT_VALUES, 8);
		if((8 == keyLen)&&(APDUBuffer[OFFSET_INS]==0xD8))
		{
			mode  = U8_OR_U8(mode , 0x40);
		}else if((0x18 == keyLen)&&(APDUBuffer[OFFSET_INS]==0xD8))
		{
			mode  =U8_OR_U8(mode , 0x80);
		}
		Triple_DES(MEMtPX8(TEMPBuffer), PM8_A_U8(APDUBuffer , keyDataOffest), MEMtPX8(TEMPBuffer), 8, MEMtPM8(bTempArr), mode, SENSITIVE_DATA_MASKED);
	}else if(KEY_AES_TYPE == keyType)
	{
		ecpMem_Set(TEMPBuffer, 0x01, KEY_CHECK_LENGTH_16);
#if VM_WITH_AES
		SymCipherDofinal(MEMtPM8(TEMPBuffer), KEY_CHECK_LENGTH_16, MEMtPM8(TEMPBuffer), PX8_A_U8(APDUBuffer, keyDataOffest), U16_M_U16(keyLen, 8), NULL, NULL, ALG_AES_BLOCK_128_ECB_NOPAD, U8_OR_U8(SYM_ECB_MODE, MODE_ENCRYPT), 0x00, ALG_AES);
#endif
}else if(KEY_SCP_81_TYPE ==	keyType)
	{
		HASHDoFinal(MEMtPM8(PX8_A_U8(APDUBuffer , keyDataOffest)), keyLen, TEMPBuffer, HASH_INIT, MEM_NULL, MEM_NULL, ALG_SHA_1);
	}

	if (memCompare(MEMtPM8(TEMPBuffer), PX8_A_U8(APDUBuffer , U8_A_U8(checkValueOffest,1)), HIGHEST_ORDER_BYTES) != ECP_CMP_RESULT_EQUAL)
	{
		//throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
			if((APDUBuffer[OFFSET_INS]==0xD8))
				keyManager_abortLogicManagerThorwError(SW_SECURITY_STATUS_NOT_SATISFIED); // GP2.2.1_UICC_ComplianceTests(SCP81版)-20200114 / km_APDU_putKeyDes_10_InvalidEnc: UICCRAMAPDUProcessMain方法的catch块只处理了iso execption，其他状态都被处理成6F00
			else
			{
				#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
				throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
				#else
//				throw_error(SW_WRONG_DATA);
				throw_error(SW_SECURITY_STATUS_NOT_SATISFIED); //modify by xuxiaojun@20220427 gpMGCompliance103:  km_APDU_storeData_07_BadKCV
				#endif
			}
		return;
	}
}

#endif
