/*
 * Sym_SM4_HAL.c
 *
 *  Created on: 2014-2-19
 *      Author: jiangxiaohui
 */


#include "Sym_API.h"
#include "Crypto_softSM4.h"
#include "chipHead.h"
//#include "gwAPI.h"

#if WITH_SM4
//extern SM4_para sm4;
//extern unsigned char Xman_skill(SM4_para * sm4Para);
extern void inv_ChipIDMap(unsigned char *mi, unsigned int len, unsigned int offset1,unsigned int offset2, unsigned int mode, unsigned char *mo);

#if !WITH_SOFT_SM4
void SM4KeyExpand(u16 keyBitLen, PX32 pExternKey, u8 cryptoMode)
{
	return;
}
void DoSM4_Crypt(PX32 dataIn, u8 dataInLen, PX32 keyBuffer, u8 keyByteLen, PX32 tICV,u8 option)
{
//	//	PATCH_SM4_001();
//	SM4_Parameters				  SM4_Param;
//	
////	u32                 X_A       chkSumPara;
//	u32			        X_A		  SM4CryptoResult;
//	
//	PATCH_HAL_CRYPTO_SYM_DES_HAL_002();	
//	
//	SM4_Param.iDataPtr 	= dataIn;
//	SM4_Param.oDataPtr 	= dataIn;	
//	SM4_Param.DataLen  	= U8t32(dataInLen);      
//	SM4_Param.iKeyPtr  	= keyBuffer;
//	SM4_Param.IVPtr    	= tICV; 
//	SM4_Param.Type      = option;
//	SM4CryptoResult     = 0;
//	
//	SM4CryptoResult = HED_SM4Block(&SM4_Param);

//	if(SM4_SUCCESS != SM4CryptoResult)
//	{
//		while(1);
//	}
	return;
}
#endif
/**
 * \brief This function implements the SM4 key loading.
 * The parameters shall be passed using the structure ::phSmx2ClSM4_LoadKey_Param
 *
 * \returns Status of the operation
 *
 * \retval ::PH_ERR_SMX2CLSM4_LOADKEY_OK  Operation finished successfully
 * \retval ::PH_ERR_SMX2CLSM4_POSSIBLE_ATTACK  Possible attack detected
 * \retval ::PH_ERR_SMX2CLSM4_WRONG_CHECKSUM   Wrong CRC checksum

 */
u16 Sym_SM4_LoadKey(Sym_LoadKey_Param_t * myKeyParam)
{
	u8 		X_A			keyByteLen;
	u8 		X_A			cryptoMode;

	keyByteLen	 	= myKeyParam->keyByteLen;
  cryptoMode 		= myKeyParam->option;

//    if(SYM_CHECKSUM_VERIFY == myKeyParam->checksum)
//    {
//    	;
//    }

  SM4KeyExpand((U8t16(keyByteLen) << 3), myKeyParam->pKeyBuffer, cryptoMode);
  return ERR_SYM_LOADKEY_OK;
}


/**
 * \brief This function implements the SM4 en-/decryption.
 * The parameters shall be passed using the structure ::phSmx2ClSM4_Crypt_Param
 *
 * \returns Status of the operation
 *
 * \retval ::PH_ERR_SMX2CLSM4_CRYPT_OK  Operation finished successfully
 * \retval ::PH_ERR_SMX2CLSM4_POSSIBLE_ATTACK  Possible attack detected
 */
#define  NewDataIn  ICVUpdate   //re-use ICVUpdate£¬

u16 Sym_SM4_Crypt(Sym_Crypt_Param_t * myCryptParam)
{
	u32   X_A   dataBuffer[U8_M_U8(U8_SHR_U8(SM4_BLOCKSIZE, 0x02),BIG_SM4_BLOCKNUM)];
	u32  	X_A		ICVUpdate[U8_SHR_U8(SM4_BLOCKSIZE, 0x02)];
	u8  	X_A		mode;
	u8		X_A		enOrDeCrypt;
	u8    X_A   type;
	u8  	X_A		keyByteLen;
	u16  	X_A		remainBlockNum;
	u16 	X_A		i;
	u16   X_A   operaLen;
	PX8 			  pDataIn;
	PX8 			  pDataOut;  
	
	PATCH_HAL_CRYPTO_SYM_DES_HAL_004();	

	if(0 == myCryptParam->noOfBlocks)
	{
		return ERR_SYM_CRYPT_OK;
	}

	pDataOut        = myCryptParam->pOutput;
	pDataIn         = myCryptParam->pInput;
  keyByteLen		  = myCryptParam->keyByteLen;   									// 16byte
	operaLen        = 0;
	type            = ECB_ENCRYPT;

	mode        	= U8_AND_U8(myCryptParam->option, SYM_ECB_CBC_MAC_MASK);        // CBC, ECB or CBC MAC
  enOrDeCrypt 	= U8_AND_U8(myCryptParam->option, MODE_EN_OR_DE_MASK);  		// encrypt or decrypt
   
  //find mode	
  if(MODE_DECRYPT == enOrDeCrypt)
  {
	 	type |= ECB_DECRYPT;
  }
  if(SYM_ECB_MODE != mode)
  {
		type |= CBC_ENCRYPT;
	 	ecpMem_Copy(ICVUpdate, myCryptParam->pIV, SM4_BLOCKSIZE);
  }
	 
	//Do with big BLOCK, one big BLOCK equals (BIG_SM4_BLOCKNUM * BLOCK)
	for(i = 0; i < myCryptParam->noOfBlocks; i += BIG_SM4_BLOCKNUM)
	{
     remainBlockNum =  U16_S_U16(myCryptParam->noOfBlocks,i);
	   
		 // if less then one Big BLOCK
	   if(remainBlockNum <= BIG_SM4_BLOCKNUM)
	   {
				operaLen =  U8_M_U16(SM4_BLOCKSIZE,remainBlockNum);
     }
	   else
	   {
				operaLen =  U8_M_U8(SM4_BLOCKSIZE,BIG_SM4_BLOCKNUM);
     }
	   
		 ecpMem_Copy(dataBuffer, pDataIn ,operaLen);
	 
		 DoSM4_Crypt(dataBuffer,operaLen,myCryptParam->pKeyBuffer,keyByteLen,ICVUpdate,type);
	     
		// Get the ICV
		if(type == CBC_ENCRYPT)
		{
			//Get the value which is encrypted form the last block as the ICV
			ecpMem_Copy(ICVUpdate, U32_A_U16(dataBuffer,U16_S_U8(operaLen,SM4_BLOCKSIZE)), SM4_BLOCKSIZE);
		}
		else if(type == CBC_DECRYPT)
		{
			//Get the last block which is decrypted before as the ICV
			ecpMem_Copy(ICVUpdate, U32_A_U16(pDataIn,U16_S_U8(operaLen,SM4_BLOCKSIZE)), SM4_BLOCKSIZE);
		}
	   
		if(SYM_CBC_MAC_MODE != (mode & SYM_CBC_MAC_MASK))
		{
			//keep the result	
			ecpMem_Copy(pDataOut, dataBuffer, operaLen);
			pDataOut =  PX8_A_U16(pDataOut,operaLen);
		}
	   
		pDataIn  =  PX8_A_U16(pDataIn,operaLen); 

	}
    
	//CBC MAC,Store MAC
	if(SYM_CBC_MAC_MODE == (mode & SYM_CBC_MAC_MASK))
	{
		ecpMem_Copy(myCryptParam->pOutput, U32_A_U16(dataBuffer,U16_S_U8(operaLen,SM4_BLOCKSIZE)),SM4_BLOCKSIZE);
	}
	
	 // Store ICV
	 if(SYM_ECB_MODE != mode )
	 {
		 ecpMem_Copy(myCryptParam->pIV, ICVUpdate, SM4_BLOCKSIZE);
	 }
   return ERR_SYM_CRYPT_OK;

}

#endif //WITH_SM4


