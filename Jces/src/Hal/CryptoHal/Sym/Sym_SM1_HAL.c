/*
 * Sym_SM1_HAL.c
 *
 *  Created on: 2014-2-19
 *      Author: jiangxiaohui
 */


#include "Sym_API.h"
#include "chipHead.h"
//#include "gwAPI.h"

#if WITH_SM1
extern void whale(unsigned char enc_sel, unsigned char mode, int round_sel, unsigned char sk_sel, unsigned char *SK, unsigned char *MK, unsigned char *IV, unsigned int len, unsigned char *input, unsigned char *output);
extern void inv_ChipIDMap(unsigned char *mi, unsigned int len, unsigned int offset1,unsigned int offset2, unsigned int mode, unsigned char *mo);

#if !WITH_SOFT_SM1
void SM1KeyExpand(u16 keyBitLen, PX32 pExternKey, u8 cryptoMode)
{
	return;
}

void DoSM1_Crypt(PX32 dataIn, u8 dataInLen, PX32 keyBuffer, u8 keyByteLen, PX32 tICV,u8 option)
{
//	//	PATCH_SM4_001();
//	SM1_Parameters				  SM1_Param;
//	
//	u32                 X_A       chkSumPara;
//	u32			        X_A		  SM1CryptoResult;
//	
//	PATCH_HAL_CRYPTO_SYM_DES_HAL_002();	
//	
//	SM1_Param.iDataPtr 	= dataIn;
//	SM1_Param.oDataPtr 	= dataIn;	
//	SM1_Param.DataLen  	= U8t32(dataInLen);      
//	SM1_Param.iKeyPtr  	= keyBuffer;
//	SM1_Param.IVPtr    	= tICV; 
//	SM1_Param.Type      = option ;
//	SM1CryptoResult     = 0;
//	
//	SM1CryptoResult = HED_SM1Block(&SM1_Param);

//	if(SM1_SUCCESS != SM1CryptoResult)
//	{
//		while(1);
//	}
	return;
}
#endif
/**
 * \brief This function implements the SM1 key loading.
 * The parameters shall be passed using the structure ::phSmx2ClSM1_LoadKey_Param
 *
 * \returns Status of the operation
 *
 * \retval ::PH_ERR_SMX2CLSM1_LOADKEY_OK  Operation finished successfully
 * \retval ::PH_ERR_SMX2CLSM1_POSSIBLE_ATTACK  Possible attack detected
 * \retval ::PH_ERR_SMX2CLSM1_WRONG_CHECKSUM   Wrong CRC checksum

 */
u16 Sym_SM1_LoadKey(Sym_LoadKey_Param_t * myKeyParam)
{
	if(myKeyParam->keyByteLen < SM1_BLOCKSIZE*2)
	{
		ecpMem_Set(PX8_A_U8((myKeyParam->pKeyBuffer), BYTE_LENGTH_SM1_128), 0x00, BYTE_LENGTH_SM1_128);
	}
	
  return ERR_SYM_LOADKEY_OK;
}


/**
 * \brief This function implements the SM1 en-/decryption.
 * The parameters shall be passed using the structure ::phSmx2ClSM1_Crypt_Param
 *
 * \returns Status of the operation
 *
 * \retval ::PH_ERR_SMX2CLSM1_CRYPT_OK  Operation finished successfully
 * \retval ::PH_ERR_SMX2CLSM1_POSSIBLE_ATTACK  Possible attack detected
 */
#define  NewDataIn  ICVUpdate   //re-use ICVUpdate£¬


u16 Sym_SM1_Crypt(Sym_Crypt_Param_t * myCryptParam)
{
	u32   X_A   dataBuffer[U8_M_U8(U8_SHR_U8(SM1_BLOCKSIZE, 0x02),BIG_SM1_BLOCKNUM)];
	u32  	X_A		ICVUpdate[U8_SHR_U8(SM1_BLOCKSIZE, 0x02)];
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
	 	ecpMem_Copy(ICVUpdate, myCryptParam->pIV, SM1_BLOCKSIZE);
  }
	 
	//Do with big BLOCK, one big BLOCK equals (BIG_SM4_BLOCKNUM * BLOCK)
	for(i = 0; i < myCryptParam->noOfBlocks; i += BIG_SM1_BLOCKNUM)
	{
     remainBlockNum =  U16_S_U16(myCryptParam->noOfBlocks,i);
	   
		 // if less then one Big BLOCK
	   if(remainBlockNum <= BIG_SM1_BLOCKNUM)
	   {
				operaLen =  U8_M_U16(SM1_BLOCKSIZE,remainBlockNum);
     }
	   else
	   {
				operaLen =  U8_M_U8(SM1_BLOCKSIZE,BIG_SM1_BLOCKNUM);
     }
	   
		 ecpMem_Copy(dataBuffer, pDataIn ,operaLen);
	 
		 DoSM1_Crypt(dataBuffer,operaLen,myCryptParam->pKeyBuffer,keyByteLen,ICVUpdate,type);
	     
		// Get the ICV
		if(type == CBC_ENCRYPT)
		{
			//Get the value which is encrypted form the last block as the ICV
			ecpMem_Copy(ICVUpdate, U32_A_U16(dataBuffer,U16_S_U8(operaLen,SM1_BLOCKSIZE)), SM1_BLOCKSIZE);
		}
		else if(type == CBC_DECRYPT)
		{
			//Get the last block which is decrypted before as the ICV
			ecpMem_Copy(ICVUpdate, U32_A_U16(pDataIn,U16_S_U8(operaLen,SM1_BLOCKSIZE)), SM1_BLOCKSIZE);
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
		ecpMem_Copy(myCryptParam->pOutput, U32_A_U16(dataBuffer,U16_S_U8(operaLen,SM1_BLOCKSIZE)),SM1_BLOCKSIZE);
	}
	
	 // Store ICV
	 if(SYM_ECB_MODE != mode )
	 {
		 ecpMem_Copy(myCryptParam->pIV, ICVUpdate, SM1_BLOCKSIZE);
	 }
   return ERR_SYM_CRYPT_OK;

}

#endif //WITH_SM1


