/*
 * Sym_KOREANSEED_HAL.c
 *
 *  Created on: 2014-2-19
 *      Author: jiangxiaohui
 */


#include "Sym_API.h"
#include "Crypto_softKOREANSEED.h"

#if WITH_KOREANSEED

#if !WITH_SOFT_KOREANSEED
void loadKoreanSeedKey(u8 keyByteLen, PX32 pExternKey)
{
	return;
}

void DoKoreanSeed_Crypt(PX8 dataIn, PX8 dataOut, u8 en_deCrypt, PX32 pExternKey, u8 KeyByteLen)
{
	return;
}
#endif

/**
 * \brief This function implements the KOREANSEED key loading.
 * The parameters shall be passed using the structure ::phSmx2ClKOREANSEED_LoadKey_Param
 *
 * \returns Status of the operation
 *
 * \retval ::PH_ERR_SMX2CLKOREANSEED_LOADKEY_OK  Operation finished successfully
 * \retval ::PH_ERR_SMX2CLKOREANSEED_POSSIBLE_ATTACK  Possible attack detected
 * \retval ::PH_ERR_SMX2CLKOREANSEED_WRONG_CHECKSUM   Wrong CRC checksum

 */
u16 Sym_KOREANSEED_LoadKey(Sym_LoadKey_Param_t * myKeyParam)
{
	u8 		X_A			keyByteLen;

	keyByteLen	= myKeyParam->keyByteLen;

//    if(SYM_CHECKSUM_VERIFY == myKeyParam->checksum)
//    {
//    	;
//    }

	loadKoreanSeedKey(keyByteLen, myKeyParam->pKeyBuffer);

    return ERR_SYM_LOADKEY_OK;
}


/**
 * \brief This function implements the KOREANSEED en-/decryption.
 * The parameters shall be passed using the structure ::phSmx2ClKOREANSEED_Crypt_Param
 *
 * \returns Status of the operation
 *
 * \retval ::PH_ERR_SMX2CLKOREANSEED_CRYPT_OK  Operation finished successfully
 * \retval ::PH_ERR_SMX2CLKOREANSEED_POSSIBLE_ATTACK  Possible attack detected
 */
u16 Sym_KOREANSEED_Crypt(Sym_Crypt_Param_t * myCryptParam)
{
    PX8 			pDataIn;
    PX8 			pDataOut;
    PM8				pIcv;
    u8  	X_A		ICVUpdate[KOREANSEED_BLOCKSIZE];
    u8  	X_A		mode;
    u8		X_A		enOrDeCrypt;
    u8  	X_A		keyByteLen;
    u8  	X_A		j;
    u8 		X_A		i;

#define  NewDataIn  ICVUpdate   //re-use ICVUpdate£¬

	if(0 == myCryptParam->noOfBlocks)
	{
		return ERR_SYM_CRYPT_OK;
	}

    pIcv 			= myCryptParam->pIV;
    mode        	= U8_AND_U8(myCryptParam->option, SYM_ECB_CBC_MAC_MASK);        // CBC, ECB or CBC MAC
    enOrDeCrypt 	= U8_AND_U8(myCryptParam->option, MODE_EN_OR_DE_MASK);  // encrypt or decrypt
    keyByteLen 		= myCryptParam->keyByteLen;   // single KOREANSEED, Three KOREANSEED 2key or Three KOREANSEED 3key

    for(i = 0;i < myCryptParam->noOfBlocks; U8_INC(i))
    {
        pDataIn 	= PX8_A_U16(myCryptParam->pInput, U8_M_U8(i, KOREANSEED_BLOCKSIZE));

        if(SYM_ECB_MODE != mode)    // CBC ro CBC MAC
        {
            if(MODE_ENCRYPT == enOrDeCrypt)   //CBC(MAC) encrypt
            {
            	ecpMem_Copy(NewDataIn, pDataIn, KOREANSEED_BLOCKSIZE);
                for (j = 0x00; j < KOREANSEED_BLOCKSIZE; U8_INC(j))  //the result of XOR can't put in cryptParam->pInput,we can't change the input data	 
				{
					PX_W_U8(PX8_A_U8(NewDataIn, j), U8_XOR_U8(PX_R_U8(PX8_A_U8(NewDataIn, j)), PM_R_U8(PM8_A_U8(pIcv, j)))); //*(NewDataIn + j) ^= *(pIcv + j);
                }
                pDataIn = NewDataIn;
            }
            else                                      //CBC decrypt
            {
            	ecpMem_Copy(ICVUpdate, pDataIn, KOREANSEED_BLOCKSIZE);
            }
        }

        if(SYM_CBC_MAC_MODE == (mode & SYM_CBC_MAC_MASK))
		{
            pDataOut = myCryptParam->pOutput;
		}
        else
		{
            pDataOut = PX8_A_U16(myCryptParam->pOutput, U8_M_U8(i, KOREANSEED_BLOCKSIZE));
		}

        // Start the KOREANSEED operation
        // Update result with Output Data Register

        DoKoreanSeed_Crypt(pDataIn, pDataOut, enOrDeCrypt, myCryptParam->pKeyBuffer, keyByteLen);

        // Update result with Output Data Register
        if(SYM_ECB_MODE != mode)        // CBC (MAC) crypt
        {
            if((MODE_ENCRYPT == enOrDeCrypt))     //CBC encrypt
            {
            	ecpMem_Copy(pIcv, pDataOut, KOREANSEED_BLOCKSIZE);
            }
            else                                           //CBC decrypt
            {
                for (j = 0x00; j < KOREANSEED_BLOCKSIZE; U8_INC(j))
				{
				  PX_W_U8(PX8_A_U8(pDataOut, j), U8_XOR_U8(PX_R_U8(PX8_A_U8(pDataOut, j)), PM_R_U8(PM8_A_U8(pIcv, j))));       // *(pDataOut + j) ^= *(pIcv + j);
				}
                ecpMem_Copy(pIcv, ICVUpdate, KOREANSEED_BLOCKSIZE);
            }
        }
    }

//    if(SYM_ENABLE_DFA_PROTECTION == myCryptParam->dfaProtect)
//    {
//    	;
//    }

    return ERR_SYM_CRYPT_OK;

}

#endif //WITH_KOREANSEED


