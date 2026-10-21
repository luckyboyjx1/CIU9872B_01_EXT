/*
 * casd_storeKey.h
 *
 *  Created on: 2013-10-16
 *      Author: liuhanren
 */

#ifndef CASD_STOREKEY_H_
#define CASD_STOREKEY_H_

#include "profile.h"

//store key:00B9
#define			 CASD_KEY_VERSION					0x74
#define			 RSA_CRT_USAGE_QUALIFIER			0x95
#define			 RSA_CRT_KEY_ACCESS   			    0x96
#define 		 RSA_CRT_KEY_TYPE					0x80
#define 		 RSA_CRT_KEY_LENGTH					0x81
#define 		 RSA_CRT_KEY_IDENTIFIER				0x82
#define 		 RSA_CRT_KEY_VERSION_NUMBER			0x83
#define			 RSA_CRT_KET_CHECK_VALUE			0x84
#define			 CRT_KET_PARA_REF			        0x85   //ecc 曲线参数 F0
#define			 RSA_DGI_LEN_OFF					0x07
#define			 RSA_CRT_TAG						0xB9
#define			 SCP_KEYS_DGI						0x00A6
#define          KEY_INF_DAT_DGI                    0x00B9
#define          RSA_N_DGI                          0x0010
#define          RSA_E_DGI                          0x0011
#define          RSA_D_DGI                          0x8112
#define          KS_DGI 	                        0x8113
#define          RSA_CRT_IQP_DGI                    0x8121
#define          RSA_CRT_DQ_DGI                     0x8122
#define          RSA_CRT_DP_DGI                     0x8123
#define          RSA_CRT_Q_DGI                      0x8124
#define          RSA_CRT_P_DGI                      0x8125


//==========================RSA KEY TYPE==============================================================
#define 		 RSA_N_DGI_TYPE						0xA1
#define 		 RSA_PRIVATE_N_DGI_TYPE				0xA2
#define          RSA_E_DGI_TYPE	                    0xA0
#define          RSA_D_DGI_TYPE	                    0xA3
#define          KS_DGI_TYPE	 	                0x80
#define          RSA_CRT_IQP_DGI_TYPE	            0xA6
#define          RSA_CRT_DQ_DGI_TYPE	            0xA8
#define          RSA_CRT_DP_DGI_TYPE                0xA7
#define          RSA_CRT_Q_DGI_TYPE                 0xA5
#define          RSA_CRT_P_DGI_TYPE                 0xA4



//--------------------ECC KEY DGI-------------------------------
#define          ECC_CurvePara_P_DGI   					0x0030
#define          ECC_CurvePara_A_DGI  					0x0031
#define          ECC_CurvePara_B_DGI  					0x0032
#define          ECC_CurvePara_G_DGI  					0x0033
#define          ECC_CurvePara_N_DGI  					0x0034
#define          ECC_CurvePara_K_DGI  					0x0035
#define          ECC_PublicKey_DGI  					0x0036
#define          ECC_PrivateKey_DGI  					0x8137

//-------------------ECC KEY TYPE---------------------
#define 		 ECC_KEYPARAREF_DGI_TYPE		    	0xF0
#define 		 ECC_CurvePara_P_DGI_TYPE		    	0xB2
#define 		 ECC_CurvePara_A_DGI_TYPE		    	0xB3
#define 		 ECC_CurvePara_B_DGI_TYPE		    	0xB4
#define 		 ECC_CurvePara_G_DGI_TYPE		    	0xB5
#define 		 ECC_CurvePara_N_DGI_TYPE		    	0xB6
#define 		 ECC_CurvePara_K_DGI_TYPE		    	0xB7
#define          ECC_PublicKey_DGI_TYPE  				0xB0
#define          ECC_PrivateKey_DGI_TYPE  				0xB1

#define          ECC_KEYPARAREF_BIT0                0x00   //g_markEccKey 对应的比特位
#define 		 ECC_CurvePara_P_BIT2				0x02
#define 		 ECC_CurvePara_A_BIT3				0x03
#define 		 ECC_CurvePara_B_BIT4				0x04
#define 		 ECC_CurvePara_G_BIT5				0x05
#define 		 ECC_CurvePara_N_BIT6				0x06
#define 		 ECC_CurvePara_K_BIT7				0x07
#define          ECC_PublicKey_BIT1				    0x01
#define          ECC_PrivateKey_BIT1				0x01


//----------STORE ECC KEY MODE---------------------------
#define          ECC_StoreKeyMode_INITIAL           0x00
#define          ECC_StoreKeyMode_F0_AND_DOMAIN     0x01
#define          ECC_StoreKeyMode_KEY_AND_F0        0x02
#define          ECC_StoreKeyMode_KEY_AND_DOMAIN    0x03

#define          ECC_DOMAIN_KID                     0xFF
#define          ECC_DOMAIN_KEY_VERSION             0xFE

#define          ECC_256_BYTELEN                    ALG_EC_FP_KEY_256 //0x20
#define          ECC_384_BYTELEN                    ALG_EC_FP_KEY_384 //0x30
#if WITH_ECC_KEY_512
#define          ECC_512_BYTELEN                    ALG_EC_FP_KEY_512 //0x40
#endif
#if WITH_ECC_KEY_521
#define          ECC_521_BYTELEN                    ALG_EC_FP_KEY_521 //0x42
#endif

#define			 DGI_TAG_LENGTH_BYTE			    0x03
//===========================AES KEY=====================================================================
//#define AES_ECB_MODE_MASK           				0x00
//#define DECRYPT 									0xFE
//#define ENCRYPT 									0xF1
//=======================================================================================================
#if GP__KEY_MANAGER__STORE_DGI_KEY
	extern void storeDGIKey(PN8 sdDataObj);
	void storeCRTKeyForSplit(PN8 sdDataObj);
	#if  GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		void encryptKeyCheckValueForRSA(u8 keyDataOffest, u8	checkValueOffest, PX8 bKeyCheckTag, u8	keyComNum, u8 keyType, u16 keyLen);
	#else
		#define encryptKeyCheckValueForRSA(keyDataOffest, checkValueOffest, bKeyCheckTag, keyComNum,  keyType, keyLen)
	#endif
	#if  GP__KEY_MANAGER__ECC_KEY
	 void checkEccKeyLen(u16 curLen, u8 keyType);
	#endif
#else
	#define storeDGIKey(sdDataObj)
	#define storeCRTKeyForSplit(sdDataObj)
	#define encryptKeyCheckValueForRSA(keyDataOffest, checkValueOffest, bKeyCheckTag, keyComNum,  keyType, keyLen)																							0
	#if  GP__KEY_MANAGER__ECC_KEY
	#define  checkEccKeyLen(curLen,keyType)
	#endif
#endif

#if GP__KEY_MANAGER
	void decryptKeyDataByDEK(u8 offest, u8 keyDataLen,u8 type);
	void encryptKeyCheckValueByDEK(u8 keyDataOffest, u8	checkValueOffest, PX8 bKeyCheckTag, u8	keyComNum, u8 keyType, u16 keyLen);
	extern u8 	 GetCurSDSCP80and02Key(u8 SDId);
	u8 	 GetSCP80and02Key(void);
#else
	#define decryptKeyDataByDEK(offest, keyDataLen, type)
	#define encryptKeyCheckValueByDEK(keyDataOffest, checkValueOffest, bKeyCheckTag, keyComNum, keyType, keyLen)
	#define GetCurSDSCP80and02Key(SDId)
	#define GetSCP80and02Key()
#endif
#endif /* CASD_STOREKEY_H_ */
