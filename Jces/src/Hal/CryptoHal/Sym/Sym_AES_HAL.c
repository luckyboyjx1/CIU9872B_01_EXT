/*
 * Sym_AES_HAL.c
 *
 *  Created on: 2014-2-19
 *      Author: jiangxiaohui
 */


#include "Sym_API.h"
#include "Crypto_softAES.h"
#include "chipHead.h"
#include "RNG_API.h"

#if WITH_AES

#if !WITH_SOFT_AES
void AESKeyExpand(u16 keyBitLen, PX32 pExternKey, u8 cryptoMode)
{
	
}

void DoAES_Crypt(PX8 dataIn, PX8 dataOut, u8 en_deCrypt, PX32 pExternKey, u8 KeyByteLen)
{
	MH_SYM_CRYPT_CALL				  AES_CryptParam;
	
	u32             X_A     chkSumPara;
	u32			        X_A		  AESCryptoResult;
	
	PATCH_HAL_CRYPTO_SYM_DES_HAL_002();	
	
	AES_CryptParam.pu8In = dataIn;
	AES_CryptParam.u32InLen = AES_BLOCKSIZE;
	AES_CryptParam.pu8Out = dataOut;
	AES_CryptParam.u32OutLen = AES_BLOCKSIZE;   
	AES_CryptParam.pu8IVout	= NULL;
	AES_CryptParam.pu8Key = MEMtPX8(pExternKey);
	
	AESCryptoResult = 0;
	
	if(MODE_ENCRYPT == en_deCrypt)
	{
		AES_CryptParam.u16Opt = MH_AES_OPT_BLK_ECB | MH_AES_OPT_MODE_ENCRYPT;
	}
	else
	{
		AES_CryptParam.u16Opt = MH_AES_OPT_BLK_ECB | MH_AES_OPT_MODE_DECRYPT;
	}
	
	switch(KeyByteLen)
	{
		case BYTE_LENGTH_AES_128:
			AES_CryptParam.u16Opt |= MH_AES_OPT_KEY_128;	
			break;
		case BYTE_LENGTH_AES_192:
			AES_CryptParam.u16Opt |= MH_AES_OPT_KEY_192;	
			break;
		case BYTE_LENGTH_AES_256:
			AES_CryptParam.u16Opt |= MH_AES_OPT_KEY_256;	
			break;
		default:
			break;		
	}
	
	AES_CryptParam.u32Crc = MHCRC_CalcBuff(0xffff, &AES_CryptParam, sizeof(MH_SYM_CRYPT_CALL)-4); 
	
	//find algorithm and calc	
	AESCryptoResult = MHAES_EncDec(&AES_CryptParam);

  //check the calculate and return value	
	if(MH_RET_AES_SUCCESS != AESCryptoResult)
	{
		while(1);
	}
	return;
}

#endif
/**
 * \brief This function implements the AES key loading.
 * The parameters shall be passed using the structure ::phSmx2ClAES_LoadKey_Param
 *
 * \returns Status of the operation
 *
 * \retval ::PH_ERR_SMX2CLAES_LOADKEY_OK  Operation finished successfully
 * \retval ::PH_ERR_SMX2CLAES_POSSIBLE_ATTACK  Possible attack detected
 * \retval ::PH_ERR_SMX2CLAES_WRONG_CHECKSUM   Wrong CRC checksum

 */
u16 Sym_AES_LoadKey(Sym_LoadKey_Param_t * myKeyParam)
{
	u8 		X_A			keyByteLen;
	u8 		X_A			cryptoMode;
	PATCH_HAL_CRYPTO_SYM_AES_HAL_001();
	keyByteLen	 	= myKeyParam->keyByteLen;
    cryptoMode 		= myKeyParam->option;

//    if(SYM_CHECKSUM_VERIFY == myKeyParam->checksum)
//    {
//    	;
//    }

   // AESKeyExpand((u16)(keyByteLen << 3), myKeyParam->pKeyBuffer, cryptoMode);

    return ERR_SYM_LOADKEY_OK;
}


/**
 * \brief This function implements the AES en-/decryption.
 * The parameters shall be passed using the structure ::phSmx2ClAES_Crypt_Param
 *
 * \returns Status of the operation
 *
 * \retval ::PH_ERR_SMX2CLAES_CRYPT_OK  Operation finished successfully
 * \retval ::PH_ERR_SMX2CLAES_POSSIBLE_ATTACK  Possible attack detected
 */
u16 Sym_AES_Crypt(Sym_Crypt_Param_t * myCryptParam)
{
   PX8 			pDataIn;
    PX8 			pDataOut;
    PM8				pIcv;
    u8  	X_A		ICVUpdate[AES_BLOCKSIZE];
    u8  	X_A		mode;
    u8		X_A		enOrDeCrypt;
    u8  	X_A		keyByteLen;
    u8  	X_A		j;
    u8 		X_A		i;

#define  NewDataIn  ICVUpdate   //re-use ICVUpdate,
	PATCH_HAL_CRYPTO_SYM_AES_HAL_003();
	if(0 == myCryptParam->noOfBlocks)
	{
		return ERR_SYM_CRYPT_OK;
	}

    pIcv 			= myCryptParam->pIV;
    mode        	= U8_AND_U8(myCryptParam->option, SYM_ECB_CBC_MAC_MASK);        // CBC, ECB or CBC MAC
    enOrDeCrypt 	= U8_AND_U8(myCryptParam->option, MODE_EN_OR_DE_MASK);  // encrypt or decrypt
		keyByteLen		= myCryptParam->keyByteLen; 	// single AES, Three AES 2key or Three AES 3key
	
		//open crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_SYMC,ENABLE);

    for(i = 0;i < myCryptParam->noOfBlocks; U8_INC(i))
    {
        pDataIn 	= PX8_A_U16(myCryptParam->pInput, U8_M_U8(i, AES_BLOCKSIZE));

        if(SYM_ECB_MODE != mode)    // CBC ro CBC MAC
        {
            if(MODE_ENCRYPT == enOrDeCrypt)   //CBC(MAC) encrypt
            {
            	ecpMem_Copy(NewDataIn, pDataIn, AES_BLOCKSIZE);
                for (j = 0x00; j < AES_BLOCKSIZE; U8_INC(j))  //the result of XOR can't put in cryptParam->pInput,we can't change the input data	 
								{
								PX_W_U8(PX8_A_U8(NewDataIn, j), U8_XOR_U8(PX_R_U8(PX8_A_U8(NewDataIn, j)), PM_R_U8(PM8_A_U8(pIcv, j)))); //*(NewDataIn + j) ^= *(pIcv + j);
                }
                pDataIn = NewDataIn;
            }
            else                                      //CBC decrypt
            {
            	ecpMem_Copy(ICVUpdate, pDataIn, AES_BLOCKSIZE);
            }
        }

        if(SYM_CBC_MAC_MODE == (mode & SYM_CBC_MAC_MASK))
				{
            pDataOut = myCryptParam->pOutput;
				}
        else
				{
            pDataOut = PX8_A_U16(myCryptParam->pOutput, U8_M_U8(i, AES_BLOCKSIZE));
				}

        // Start the AES operation
        DoAES_Crypt(pDataIn, pDataOut, enOrDeCrypt, myCryptParam->pKeyBuffer, keyByteLen);

        // Update result with Output Data Register
        if(SYM_ECB_MODE != mode)        // CBC (MAC) crypt
        {
            if((MODE_ENCRYPT == enOrDeCrypt))     //CBC encrypt
            {
            	ecpMem_Copy(pIcv, pDataOut, AES_BLOCKSIZE);
            }
            else                                           //CBC decrypt
            {
                for (j = 0x00; j < AES_BLOCKSIZE; U8_INC(j))
								{
									PX_W_U8(PX8_A_U8(pDataOut, j), U8_XOR_U8(PX_R_U8(PX8_A_U8(pDataOut, j)), PM_R_U8(PM8_A_U8(pIcv, j))));       // *(pDataOut + j) ^= *(pIcv + j);
								}
                ecpMem_Copy(pIcv, ICVUpdate, AES_BLOCKSIZE);
            }
        }
    }

		//close crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_SYMC,DISABLE);
//    if(SYM_ENABLE_DFA_PROTECTION == myCryptParam->dfaProtect)
//    {
//    	;
//    }

    return ERR_SYM_CRYPT_OK;
}

#endif //WITH_AES


