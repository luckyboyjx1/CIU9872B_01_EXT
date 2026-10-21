/*
 * Sym_SM7_HAL.c
 *
 *  Created on: 2014-2-19
 *      Author: jiangxiaohui
 */


#include "Sym_API.h"
//#include "Crypto_softSM7.h"
#include "chipHead.h"
//#include "gwAPI.h"

#if WITH_SM7
extern void shark(unsigned int mode,unsigned int sel,unsigned char *iv,unsigned char *mi,unsigned char *mo,unsigned char *key,unsigned int len);
extern void inv_ChipIDMap(unsigned char *mi, unsigned int len, unsigned int offset1,unsigned int offset2, unsigned int mode, unsigned char *mo);

#if !WITH_SOFT_SM7
void SM7KeyExpand(u16 keyBitLen, PX32 pExternKey, u8 cryptoMode)
{
	return;
}

void DoSM7_Crypt(PX8 dataIn, PX8 dataOut, u8 en_deCrypt, PX32 pExternKey, u8 KeyByteLen)
{
	/*
	-----------------
    |	bit	0		|		
	-----------------
    |	0	|	1	|	
	-----------------
    |	00	|	01	|	
	-----------------
    |	EN	|	DE	|
	-----------------
	*/
	
	//参数对接
	u8					operateMode;

	//不反向验,普通安全等级

//	if(MODE_ENCRYPT == en_deCrypt)
//    {
//    	operateMode	= SYM_SM7_ENC;
//    }
//    else
//    {
//    	operateMode	= SYM_SM7_DEC;
//    }	
//	
//	//SM7 运算
//	if( ALG_SYM_OK != cryptoSM7ECB(MEMtPX8(pExternKey), dataIn, dataOut, operateMode))
//	{
//		while(1);
//	}
	
	return;
}
#endif
/**
 * \brief This function implements the SM7 key loading.
 * The parameters shall be passed using the structure ::phSmx2ClSM7_LoadKey_Param
 *
 * \returns Status of the operation
 *
 * \retval ::PH_ERR_SMX2CLSM7_LOADKEY_OK  Operation finished successfully
 * \retval ::PH_ERR_SMX2CLSM7_POSSIBLE_ATTACK  Possible attack detected
 * \retval ::PH_ERR_SMX2CLSM7_WRONG_CHECKSUM   Wrong CRC checksum

 */
u16 Sym_SM7_LoadKey(Sym_LoadKey_Param_t * myKeyParam)
{
    return ERR_SYM_LOADKEY_OK;
}


/**
 * \brief This function implements the SM7 en-/decryption.
 * The parameters shall be passed using the structure ::phSmx2ClSM7_Crypt_Param
 *
 * \returns Status of the operation
 *
 * \retval ::PH_ERR_SMX2CLSM7_CRYPT_OK  Operation finished successfully
 * \retval ::PH_ERR_SMX2CLSM7_POSSIBLE_ATTACK  Possible attack detected
 */
#define  NewDataIn  ICVUpdate   //re-use ICVUpdate，

u16 Sym_SM7_Crypt(Sym_Crypt_Param_t * myCryptParam)
{
    PX8 			pDataIn;
    PX8 			pDataOut;
    PM8				pIcv;
    u8  	X_A		ICVUpdate[SM7_BLOCKSIZE];
    u8  	X_A		mode;
    u8		X_A		enOrDeCrypt;
    u8  	X_A		keyByteLen;
    u8  	X_A		j;
    u8 		X_A		i;
	u8				round_sel;
	PX8 			key;
	u8				enc_mode;
	u8				enc_sel;
	u8				outData[SYM_MAX_BYTES_ONE_STEP];
	u16				inputLength;
	if(0 == myCryptParam->noOfBlocks)
	{
		return ERR_SYM_CRYPT_OK;
	}

    pIcv 			= myCryptParam->pIV;
    mode        	= U8_AND_U8(myCryptParam->option, SYM_ECB_CBC_MAC_MASK);        // CBC, ECB or CBC MAC
    enOrDeCrypt 	= U8_AND_U8(myCryptParam->option, MODE_EN_OR_DE_MASK);  // encrypt or decrypt
	keyByteLen		= myCryptParam->keyByteLen; 	// single SM7, Three SM7 2key or Three SM7 3key
	inputLength		= myCryptParam->noOfBlocks * SM7_BLOCKSIZE;
#if GW_PRIVATE_GM_CRYPTO	
//	if(SYM_GUOMI_PROPERTY_GW == g_SymGuomiPro)
	{
			pDataIn		=	myCryptParam->pInput;
			//pDataOut	=	myCryptParam->pOutput;
			pDataOut	=	outData;
			key			=	(PX8)myCryptParam->pKeyBuffer;
			round_sel	=	0;
			
			if(mode != SYM_ECB_MODE)  //cbc
			{
				if(enOrDeCrypt == MODE_DECRYPT)
				{
					ecpMem_Copy(ICVUpdate, pDataIn, SM7_BLOCKSIZE);
				}
			}
			
			if(enOrDeCrypt == MODE_DECRYPT)
					enc_sel=IFX_SM_DEC;
			else
					enc_sel=IFX_SM_ENC;

			if(mode != SYM_ECB_MODE)			 
					enc_mode=IFX_SM_CBC ;
			else
					enc_mode=IFX_SM_ECB;
			
		#ifdef GW_LIB_TEST
			if(enc_sel != 00)
				enc_sel=0x10;
			else
				enc_sel=0x00;
			
			shark(enc_mode,enc_sel,pIcv,pDataIn,pDataOut,key,inputLength);//std alg
		#else
			//Alg_SM7_Process(enc_mode,enc_sel,pIcv,pDataIn,pDataOut,key,inputLength);
			FunctionList.Alg_SM7_Process_entry(enc_mode,enc_sel,pIcv,pDataIn,pDataOut,key,inputLength);
		#endif
			if(SYM_CBC_MAC_MODE == (mode & SYM_CBC_MAC_MASK))
			{
				ecpMem_Copy(myCryptParam->pOutput, &pDataOut[inputLength-SM7_BLOCKSIZE], SM7_BLOCKSIZE);
			}
			else
			{
				ecpMem_Copy(myCryptParam->pOutput, pDataOut, inputLength);
			}
			
			if(mode != SYM_ECB_MODE)	   //cbc
			{
				if(enOrDeCrypt == MODE_DECRYPT)
				{
					ecpMem_Copy(pIcv, ICVUpdate, SM7_BLOCKSIZE);
				}
				else
				{
					ecpMem_Copy(pIcv, &pDataOut[inputLength-SM7_BLOCKSIZE], SM7_BLOCKSIZE);
				}
			}
	}
#endif

    return ERR_SYM_CRYPT_OK;

}

#endif //WITH_SM7


