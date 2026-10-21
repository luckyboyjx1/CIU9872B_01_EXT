#include "ChipFunction.h"
#include "ChipCoreFunction.h"
#include "ChipHead.h"
#include "ecpDefine.h"
#include "Crypto_Constant.h"
#include "EllipseCurve_API.h"
#include "halCryptoConfig.h"
#include "Crypto_common_API.h"
#include "RSA_API.h"
#include "NvmHal.h"
#include "Io_14443TaL3.h"
#include "error.h"
#include "JcvmInit.h"
#include "Io_Uart.h"

#include "mhcard_sysctrl.h"

//typedef uint32_t nrgRnd_t;
//u8 currRNGMode; 
u32 TRNGLocalCacheBuffer[NUM_OF_WORDS_IN_256BIT_FRAME<<2];
TRNG_Context TRNGLocalCache;


void resetInit_Process(void)
{
	
	u16 i;
	
    //SYSCFG->CLK_EN |= SYSCTRL_Periph_TIMER0 | SYSCTRL_Periph_SECURITY | SYSCTRL_Periph_RNG | SYSCTRL_Periph_GPIO | SYSCTRL_Periph_CRC | SYSCTRL_Periph_SYMC | SYSCTRL_Periph_ASYMC;
	SYSCFG->CLK_EN |= SYSCTRL_Periph_SRAM1;
    //SYSCFG->RST_EN = 0xFFFF0000 | SYSCTRL_Periph_RNG | SYSCTRL_Periph_CRC | SYSCTRL_Periph_TIMER0;
	SYSCFG->RST_EN = 0xFFFF0000;
    SYSCFG->WAKE_SYS_CTRL |= BIT(2)|BIT(1);             
	
//    *(volatile uint32_t *)(0x40005044) |= 0x80001000;    
	
//	NVIC_DisableIRQ(CB_FLAG_IRQn);
//	NVIC_DisableIRQ(SCIS_IRQn);
}

void appMain_Process(void)
{
	//use for app main
	app_main();
}

void IrqEntry1_Process(void)
{
	//use for uart rxtx interface
	Uart_RstIrqHandlerProcess();
	
}

void IrqEntry2_Process(void)
{
	//use for spi rxtx interface
	Uart_RxIrqHandlerProcess();
	
}

void IrqEntry3_Process(void)
{
	
	//use for timer0 irq
}
//modify by xuxiaojun@20221108: SPI_IRQHandlerProcess需要放在HAL__IO__SPI内部，否则编译不过
void IrqEntry4_Process(void)
{
#if HAL__IO__SPI
	SPI_IRQHandlerProcess();
#endif
}
//modify by xuxiaojun@20221108: I2C_IRQHandlerProcess需要放在HAL__IO__I2C内部，否则编译不过
void IrqEntry5_Process(void)
{
#if HAL__IO__I2C	
	I2C_IRQHandlerProcess();
#endif	
}

void IrqEntry6_Process(void)
{

}

void IrqEntry7_Process(void)
{

}

void IrqEntry8_Process(void)
{

}

void IrqEntry9_Process(void)
{

}

__asm void rtnBackup(u8 resetStatus)
{
	PRESERVE8
	IMPORT goto_deselect_handler
	LDR r1, =JUMP_LINE  
	mrs r2,MSP
	STR r0,[r2]
	LDR r3,=0x18
	add r3,r2			 ;msp+18, pc
	STR r1,[r3]
	LDR r3,=0x1c
	add r3,r2			 ;msp+1c, xPSR
	LDR r1, =0x01000000	 
	STR r1,[r3]			 ;xPSR=0x01000000, Thumb state bit = 1
	ldr r2,=0xfffffff9	 ;return to thread mode
	bx r2
JUMP_LINE
	bl.w goto_deselect_handler	
}

void _HalRamCopy(u8 *pbDest, u8 *pbSrc, u16 wLen)
{
	u16 i;
	if ((pbSrc + wLen) > pbDest && (pbDest > pbSrc))
	{
		if (((u32)pbDest & 0x03) || ((u32)pbSrc & 0x03) || (wLen &0x03))
		{
			for (i = wLen; i > 0; i--)
			{
				*(pbDest+i-1) = *(pbSrc+i-1);
			}
		}
		else
		{
			for (i = wLen/4; i > 0; i--)
			{
				*((u32 *)(pbDest) + i-1) = *((u32 *)(pbSrc) + i - 1);
			}
		}
	}
	else 
	{
		if (((u32)pbDest & 0x03) || ((u32)pbSrc & 0x03) || (wLen &0x03))
		{
			for (i = 0; i < wLen; i++)
			{
				*(pbDest+i) = *(pbSrc+i);
			}
		}
		else
		{
			for (i = 0; i < wLen/4; i++)
			{
				*((u32 *)(pbDest)+i) = *((u32 *)(pbSrc)+i);
			}
		}
	}
}




void GetChipUID(PX8 uid,u8 uidlevel)
{
//		if(0xAA!=(*((u8*)0x14000057)))
//	{
//		uid[0]=0x88;
//		memcpy(uid+1,(u8*)0x14000051,3);
//		memcpy(uid+5,(u8*)0x14000054,4);
//	
//		 if(0xAA!=(*((u8*)0x1400005A)))
//		{
//			uid[5]=0x88;
//			memcpy(uid+6,(u8*)0x14000054,3);
//			memcpy(uid+10,(u8*)0x14000057,4);
//		}
//	}
//	else
//	{
//		memcpy(uid,(u8*)0x14000051,4);
//	}
}

#if WITH_SM2
#define SM2_KEY_GEN_PUBLICKEY							0
#define SM2_KEY_GEN_PUBLICKEY_AND_PRIVATEKEY			1

void chipSM2GenKeyPair(PM8 privateKey, PX8 tempPublicKey)
{
//	u8 							result;
//	u32							priKeyBuffer[SM2_PRIVEYLEN >> 2];
//	u32							pubKeyBuffer[SM2_PUBLICKEYLEN >> 2];
//	
//	SM2_Parameters * 			SM2_Param;
//  SM2_Parameters  			SM2_ParamVar;
//	
//	SM2_Param	= &SM2_ParamVar;	
//	SM2_Param -> rndPtr			= (unsigned int *)priKeyBuffer;
//  SM2_Param -> priKeyPtr 	= (unsigned int *)priKeyBuffer;  
//	SM2_Param -> pubKeyPtr 	= (unsigned int *)pubKeyBuffer; 
//  SM2_Param -> argPtr 		= NULL;     
//		
//	ecpMem_Copy(priKeyBuffer, privateKey, SM2_PRIVEYLEN);
//	
//	result = HED_SM2GenKey2(SM2_Param);
//	if(EC_PROCESS_OK != result)
//	{
//		throw_error(SM2_VERIFY_FAILED);
//	}
//	
//	ecpMem_Copy(tempPublicKey, pubKeyBuffer, SM2_PUBLICKEYLEN);	
	return;
}

/*********************************************************************
 * Function name: SM2_ECP_GetKey()
 * Description: Generate the key pair or SM2 computation.
 * Input Parameters:  (precondition)
 *      PX8 * prikey             input pointer for storing private key, with length of 32 bytes.
 *      PX8 * pubkey             input pointer for storing public key, with length of 65 bytes: 0x04 || x || y.
 *      u8 isgenkeypair                 flag to indicate whether to generate key pair.
 * Output Parameters:  (postcondition)
 *      The keys are put in prikey and pubkey, and the operation status is returned.
 * Note: The following notes are just for using Fame2 of NXP SmartMX2.
 *       1. The output key is in big-endian format.
 *********************************************************************/
void chipSM2GetKey(PX8 prikey, PX8 publickey, u8 isgenkeypair)
{
//	u8 							result;
//	u32							priKeyBuffer[SM2_PRIVEYLEN >> 2];
//	u32							pubKeyBuffer[SM2_PUBLICKEYLEN >> 2];
//	u32							rngData[8];
//	
//	SM2_Parameters * 			SM2_Param;
//  SM2_Parameters  			SM2_ParamVar;
//	
//	SM2_Param	= &SM2_ParamVar;	
//	SM2_Param -> rndPtr			= (unsigned int *)rngData;
//  SM2_Param -> priKeyPtr 	= (unsigned int *)priKeyBuffer;  
//	SM2_Param -> pubKeyPtr 	= (unsigned int *)pubKeyBuffer; 
//  SM2_Param -> argPtr 		= NULL;    
//	
//	do
//	{
//		result = HED_SM2GenKey(SM2_Param);
//		if(EC_PROCESS_OK == result)
//		{
//			break;
//		}
//	}while(1);
//	
//	ecpMem_Copy(prikey, priKeyBuffer, SM2_PRIVEYLEN);
//	PX_W_U8(publickey, 0x04);
//	ecpMem_Copy(publickey + 1, pubKeyBuffer, SM2_PUBLICKEYLEN);
//	
//	if(isgenkeypair)
//	{
//		memCopyNoAtomic(g_SM2KeyStore.privteKeyBuffer, prikey, SM2_PRIVEYLEN);
//		memCopyNoAtomic(g_SM2KeyStore.publicKeyBuffer, publickey, SM2_PUBLICKEYLEN + 1);
//	}
	return;
}

void chipSM2Sign(PX8 E_Buff, PX8 privteKeyBuffer, PM8 rs)
{
//	u8 							result;
//	u8 							dataOut[64];
//	u32							rngData[8];
//	u32							privteKey[SM2_PRIVEYLEN >> 2];
//	
//	SM2_Parameters * 			SM2_Param;
//  SM2_Parameters  			SM2_ParamVar;
//	
//	SM2_Param	= &SM2_ParamVar;	
//  SM2_Param -> iDataPtr 	= E_Buff;
//  SM2_Param -> oDataPtr 	= dataOut;
//	SM2_Param -> rndPtr			= (unsigned int *)rngData;
//  SM2_Param -> priKeyPtr 	= (unsigned int *)privteKey;  
//  SM2_Param -> argPtr 		= NULL;      
//	
//	ecpMem_Copy(privteKey, privteKeyBuffer, SM2_PRIVEYLEN);
//	
//	result = HED_SM2Sign(SM2_Param);
//	if(EC_PROCESS_OK != result)
//	{
//		while(1);
//	}

//	memCopyNoAtomic(rs, dataOut, 64);
//	
//	memRefresh();
	
	return;
}

#define	ALG_SM2_VERIFY_PARAM_ERROR1			0xF241
#define	ALG_SM2_VERIFY_PARAM_ERROR2			0xF242
#define	ALG_SM2_VERIFY_RESULT_ERROR			0xF243
#define	ALG_SM2_VERIFY_PUBLICKEY_ERROR		0xF244

u8 chipSM2Verify(PM8 E, PM8 publicKey, PM8 rs) SMALL_MODE
{
//	u8 							result;
//	u32							rngData[8];
//	u32							publickeyBuffer[SM2_PUBLICKEYLEN >> 2];
//		
//	SM2_Parameters * 			SM2_Param;
//  SM2_Parameters  			SM2_ParamVar;
//	
//	SM2_Param	= &SM2_ParamVar;	
//  SM2_Param -> iDataPtr 	= E;
//  SM2_Param -> oDataPtr 	= rs;
//	SM2_Param -> rndPtr			= (unsigned int *)rngData;
//  SM2_Param -> pubKeyPtr 	= (unsigned int *)publickeyBuffer; 
//  SM2_Param -> argPtr 		= NULL;           
//	
//	ecpMem_Copy(publickeyBuffer, publicKey, SM2_PUBLICKEYLEN);
//	
//	result = HED_SM2Verify(SM2_Param);
//	if(EC_PROCESS_OK != result)
//	{
//		return SM2_VERIFY_FAILED;
//	}	
	
	return SM2_VERIFY_SUCCESS;
}

//The max value of inLen is 183( 280 - SM2_CIPHER_PADDING_LEN)
//Note: The content of inBuff is modified after the calculation!
/************************************************************
 ******Fuction name:SM2_Encrypt
 ****** input parameter:
 ******      inBuff:   				ram address;
 ******			publickeyBuff:    e2p address
 ******			outBuff:    			e2p address
 ************************************************************/
#define	ALG_SM2_ENCRYPT_C2_LENGTH_ERROR		0xF211
#define	ALG_SM2_ENCRYPT_PUBKEY_ERROR		0xF212
#define	ALG_SM2_ENCRYPT_RNG_ERROR			0xF213
#define	ALG_SM2_ENCRYPT_OUTBUFFER_ERROR		0xF214
void chipSM2Encrypt(PX8 inBuff, u16 inLen, PM8 publickeyBuff, PM8 outBuff)
{
//	u8							indata[1025];
//	u8 							result;
//	u8							dataOut[SM2_DOMAIN_NLEN + 1025 + SM2_DOMAIN_POINTXY_LEN + 4];
//	u32							rngData[8];
//	u32							publickey[SM2_PUBLICKEYLEN >> 2];
//	u8							C2buff[SM3_DIGEST_SIZE];
//	
//	SM2_Parameters * 			SM2_Param;
//  SM2_Parameters  			SM2_ParamVar;
//	
//	SM2_Param	= &SM2_ParamVar;	
//		
//  SM2_Param -> iDataPtr 	= indata;
//  SM2_Param -> oDataPtr 	= dataOut;
//	SM2_Param -> rndPtr		  = (unsigned int *)rngData;
//  SM2_Param -> pubKeyPtr 	= (unsigned int *)publickey; 
//  SM2_Param -> argPtr 		= NULL;    
//	memCopyToRam(indata, inBuff, inLen);
//	memCopyToRam((u8*)publickey, publickeyBuff, SM2_PUBLICKEYLEN);	
//	
//	result = HED_SM2Encrypt(SM2_Param, inLen); 
//	if(EC_PROCESS_OK != result)
//	{
//		while(1);
//	}

//	if(!OS_CONF4_SM2_ENC_C132_MODE)
//	{
//		ecpMem_Copy(C2buff, dataOut + SM2_PUBLICKEYLEN, SM3_DIGEST_SIZE);
//		ecpMem_Move(dataOut + SM2_PUBLICKEYLEN, dataOut + SM2_PUBLICKEYLEN + SM3_DIGEST_SIZE, inLen);
//		ecpMem_Copy(dataOut + SM2_PUBLICKEYLEN + inLen, C2buff, SM3_DIGEST_SIZE);
//	}
//		
//	//memWriteU8NoAtomic(outBuff, 0x04);
//	memCopyNoAtomic(outBuff, dataOut, inLen + 96);
//	memRefresh();
	return;
}

//The max value of inLen is 183( 280 - SM2_CIPHER_PADDING_LEN)
/*
 * the structure of algType is as follows
 *  ------------------------------------------------------------------------------------------
 *  | SM2_DOMAIN_NLEN 						| 184		 |SM2_DOMAIN_POINTXY_LEN + 4 		 |
 *  | privare Key, SM2_DOMAIN_POINTX, HASH  | Indata ,C2 |C1buff, KDF RAM,  SM2_DOMAIN_POINTY|
 *  ------------------------------------------------------------------------------------------
 *
*/

#define	ALG_SM2_DECRYPT_C2_LENGTH_ERROR		0xF221
#define	ALG_SM2_DECRYPT_C1_PARAM_ERROR		0xF222
#define	ALG_SM2_DECRYPT_KDF_ERROR			0xF223
#define	ALG_SM2_DECRYPT_C3_PARAM_ERROR		0xF224
#define	ALG_SM2_DECRYPT_OUTBUFFER_ERROR		0xF225
u8 chipSM2Decrypt(PM8 inBuff, u16 inLen, PM8 privatekeybuff, PM8 outBuff)
{
//	u8 							result;
//	u8							dataIn[SM2_DOMAIN_NLEN + 1025 + SM2_DOMAIN_POINTXY_LEN + 4];
//	u8							dataOut[1025];
//	u32							rngData[8];
//	u32							privatekey[SM2_PRIVEYLEN >> 2];
//	u8							C2buff[SM3_DIGEST_SIZE];

//	SM2_Parameters * 			SM2_Param;
//  SM2_Parameters  			SM2_ParamVar;
//	
//	
//	memCopyToRam(dataIn, inBuff, inLen);
//	if(!OS_CONF4_SM2_ENC_C132_MODE)
//	{
//		ecpMem_Copy(C2buff, dataIn + SM2_PUBLICKEYLEN + inLen - 96, SM3_DIGEST_SIZE);
//		ecpMem_Move(dataIn + SM2_PUBLICKEYLEN + SM3_DIGEST_SIZE, dataIn + SM2_PUBLICKEYLEN, inLen - 96);
//		ecpMem_Copy(dataIn + SM2_PUBLICKEYLEN, C2buff, SM3_DIGEST_SIZE);
//	}	
//	
//	SM2_Param	= &SM2_ParamVar;	
//  SM2_Param -> iDataPtr 	= dataIn;
//  SM2_Param -> oDataPtr 	= dataOut;
//	SM2_Param -> rndPtr			= (unsigned int *)rngData;
//  SM2_Param -> priKeyPtr 	= (unsigned int *)privatekey; 
//  SM2_Param -> argPtr 		= NULL;        
//	
//	memCopyToRam((u8*)privatekey, privatekeybuff, SM2_PRIVEYLEN);	
//	
//	result = HED_SM2Decrypt(SM2_Param, inLen); 
//	if(EC_PROCESS_OK != result)
//	{
//		return SM2_VERIFY_FAILED;
//	}
//	
//	memCopyNoAtomic(outBuff, dataOut, inLen - 96);
//	
//	memRefresh();
	
	return SM2_VERIFY_SUCCESS;
}
#endif

/*********************************************************************
 * Function name: ecpAModB()
 * C = A mod B
 *********************************************************************/
void ecpAModB(PX8 APoint, PX8 BPoint, PX8 CPoint, u16 byteLen)
{
//	u32 					tempBigDataBuf[MAX_RSA_MUD_BYTE_LEN >> 2];
//	PX8						resultCusor;
//	u8						tempLength;

//	/************************************************************************
//	1.填充计算数据  usOffsA       被减数A在PKERAM中的偏移量，以字为单位
//	************************************************************************/	
//	tempLength				= (byteLen + 3) & 0xFFFC;
//	resultCusor 			= MEMtPX8(PX8_A_U16(tempBigDataBuf, tempLength - byteLen));
//	
//	ecpMem_Copy(resultCusor, APoint, byteLen);
//	BigEndian2LittleEndian(MEMtPX8(tempBigDataBuf), tempLength);
//    CryptoRAMCopyToUserRAM(MEMtPX8(RAM1_PARAM_A_ADDRESS), MEMtPX8(tempBigDataBuf), tempLength);
//	
//	/************************************************************************
//	2.填充计算数据  usOffsB       减数B在PKERAM中的偏移量，以字为单位
//	************************************************************************/		
//	ecpMem_Copy(resultCusor, BPoint, byteLen);
//	BigEndian2LittleEndian(MEMtPX8(tempBigDataBuf), tempLength);
//    CryptoRAMCopyToUserRAM(MEMtPX8(RAM1_PARAM_B_ADDRESS), MEMtPX8(tempBigDataBuf), tempLength);
//	
//	/************************************************************************
//	3.填充计算数据  usOffsN       模数N在PKERAM中的偏移量，以字为单位
//	************************************************************************/	
////	ecpMem_Copy(resultCusor, BPoint, byteLen);
//    CryptoRAMCopyToUserRAM(MEMtPX8(RAM1_PARAM_N_ADDRESS), MEMtPX8(tempBigDataBuf), tempLength);
//	
//	
//	/************************************************************************
//	4.调用库接口计算大数
//	************************************************************************/
//	PkeModSub(RAM1_PARAM_A_OFFSET >> 2, RAM1_PARAM_B_OFFSET >> 2, RAM1_PARAM_N_OFFSET >> 2, RAM1_PARAM_M_OFFSET >> 2, tempLength >> 2);
//	
//	/************************************************************************
//	5.导出数据	usOffsM       模数M在PKERAM中的偏移量，以字为单位
//	************************************************************************/
//    CryptoRAMCopyToUserRAM(MEMtPX8(tempBigDataBuf), MEMtPX8(RAM1_PARAM_M_ADDRESS), tempLength);
//	BigEndian2LittleEndian(MEMtPX8(tempBigDataBuf), tempLength);
//	ecpMem_Copy(CPoint, resultCusor, byteLen);

    return;
}



// 31-bit OCC signature value for TRNG mode
const uint32_t RNG_ExpectedOccSignatureValue = 0x00000000;  	//TRNG
//const uint32_t RNG_ExpectedOccSignatureValue = 0x000000EE;	//HRNG (E0)
//const uint32_t RNG_ExpectedOccSignatureValue = 0x000000A8;	//HRNG (E4)
//const uint32_t RNG_ExpectedOccSignatureValue = 0x00000012;	//DRNG

// Data redundant OCC signature value
const uint32_t RNG_ExpectedInvOccSignatureValue = (0x7FFFFFFFul << 1);   //TRNG
//const uint32_t RNG_ExpectedInvOccSignatureValue = (0x7FFFFF11ul << 1); //HRNG (E0)
//const uint32_t RNG_ExpectedInvOccSignatureValue = (0x7FFFFF57ul << 1); //HRNG (E4)
//const uint32_t RNG_ExpectedInvOccSignatureValue = (0x7FFFFFEDul << 1); //DRNG


// Initialize the RNG peripheral
void Rng_Init()
{
	uint32_t occ;
	
//	do
//	{
//		// 1. Set the module to "configuration mode"
//		occ = SLCxx_RNG-> RNG_OCC;
//		// occ.MODE = 1, configuration mode, xor with 0 result in writing back
//		// => stay in configuration mode
//		// occ.MODE = 0, operational mode, xor with 0xFFFFFFFF and write back 
//		// => enter configuration mode
//		SLCxx_RNG-> RNG_OCC = (((occ >> RNG_OCC_MODE_Pos) & 1ul) - 1) ^ occ;

//		//SLC26 do not have RNG_MODE register
//		// 2. Configure the OCC protected SFRs of the module
////    currRNGMode = rngMode;
////		SLCxx_RNG->RNG_MODE = currRNGMode;
//		// Note: for TRNG mode, no additional settings are needed here as power-on reset
//		// values will configure it by default.
//		// For all other modes, configure SLCxx_RNG-> RNG_MODE here and
//		// define an updated RNG_ExpectedOccSignatureValue accordingly.
//	
//		// 3. Set the module back to "operational mode" using a precalculated value
//		SLCxx_RNG-> RNG_OCC = ( RNG_OCC_MODE_OPR << RNG_OCC_MODE_Pos) | ( RNG_ExpectedOccSignatureValue & RNG_OCC_VALUE_Msk );
//		//Warning:
//		// Verifying that the module is in operational mode is very important
//		// if this is not verified later on while retrieving random numbers.
//		// However, be aware that there is the possibility that the module itself
//		// is back to configuration mode due to a possible error condition, for instance.
//		// A long lasting ISR during this configuration here, may increase the chance of
//		// such a condition in case of the RNG module. A while-loop is one possible solution,
//		// to ensure that the RNG was put correctly in the operational mode.
//		occ = SLCxx_RNG-> RNG_OCC;
//	}
//	// 4. Verify that the module is back in "operational mode"
//	while(( occ & RNG_OCC_MODE_Msk) != (RNG_OCC_MODE_OPR << RNG_OCC_MODE_Pos));
//	
//	// 5. Compare the "configuration signature" against a previously known reference value
//	
//	//ATTENTION: Ensure data redundancy while writing and verifying !
//	// Please, inspect the compiled assembly code and make sure that the inverted reference value
//	// RNG_ExpectedInvOccSignatureValue is not derived from RNG_ExpectedOccSignatureValue.
//	
//	if(RNG_ExpectedInvOccSignatureValue != ((~occ) << 1) )
//	{
//		faultDetected();
//	}
}


/*
@brief Get a random word from the RNG peripheral
*
*
@note:
*This routine and its subroutines demonstrate a basic secure method of retrieving random numbers.
*It does not fit to any possible use case, and there are further security improvements possible.
*For instance, it does not contain code flow protection security measures,
*which is highly recommended to be included for productive code.
*Also note, it does not support multi-tasking.
*/
u32 Rng_GetRandomWord( u32 * pReadOutWordInv )
{
	uint32_t Status;
	uint32_t ReadOutWord;
	uint32_t ReReadWord;
//	do
//	{
//		Status = SLCxx_RNG-> RNG_STAT;
//		//wait until data is ready to read, and make sure RNG is in active state
//		while(( Status & (RNG_STAT_STATE_Msk | RNG_STAT_ERR_STATISTIC_Msk | RNG_STAT_DTA_RDY_Msk ))
//											!= ( (RNG_STAT_STATE_ACT << RNG_STAT_STATE_Pos) | (RNG_STAT_DTA_RDY_RDY << RNG_STAT_DTA_RDY_Pos)))
//		{
//			if(( Status & ( RNG_STAT_STATE_Msk | RNG_STAT_ERR_STATISTIC_Msk)) != (RNG_STAT_STATE_ACT << RNG_STAT_STATE_Pos))
//			{
//				//RNG is not in active state, or an allowed error happened, so it must be initialized again.
//				Rng_Init();
//			}
//			Status = SLCxx_RNG-> RNG_STAT;
//		}
//		
//		ReReadWord = SLCxx_RNG-> RNG_AUX; //Always read RNG_RAND_RR first and only once.
//		ReadOutWord = SLCxx_RNG-> RNG_RAND;
//		//rotate left
//		//ReReadWord = (ReReadWord << 1) | (ReReadWord >> (sizeof(ReReadWord) * CHAR_BIT - 1));
//		
//	} while(((SLCxx_RNG-> RNG_OCC) & RNG_OCC_MODE_Msk) != (RNG_OCC_MODE_OPR << RNG_OCC_MODE_Pos));
//	
//	//security check: RNG_RAND_RR and RNG_RAND must be in sync
//	if(ReadOutWord != (~(ReReadWord)))
//	{
//		faultDetected();
//	}

//	//#if defined( OCC_METHOD1_VARIANT )
//	//Security check: OCC signature
//	//Note, for functional reasons the RNG module may leave operational mode w/o an attack.
//	//As a result, two valid OCC signature values are possible:
//	// 1. if OCC is still in operational mode, then the signature value must match the expected value
//	// 2. if OCC is in configuration mode, then the signature valvue must match the inverted expected value
//	//Both situation are covered by the check below.
//	{
//		uint32_t occ = SLCxx_RNG-> RNG_OCC;
//		// occ.MODE = 1, configuration mode, invert and then compare with expected signature
//		// occ.MODE = 0, operational mode, compare with expected signature
//		occ = ((((~occ) >> RNG_OCC_MODE_Pos) & 1ul) - 1) ^ occ;
//		if( occ != RNG_ExpectedOccSignatureValue )
//		{
//			faultDetected();
//		}
//	}
	//#endif
	
	*pReadOutWordInv = ReReadWord;
	return ReadOutWord;
}

/* @brief Get a random word from the RNG peripheral
*
* @note:
* This routine and its subroutines demonstrate a basic secure method of retrieving random numbers.
* It does not fit to any possible use case, and there are further security improvements possible,
* such as for instance a HRNG_PRIGHTS check.
* Also note, that this routine does not support multi-tasking.
*/
u16 Trng_GetRandomWord(TRNG_Context * TRNGCache)
{
	PX32 randBuffer;
	uint32_t u32CrcInit, u32CrcEnd, u32Ret = 0;
	
	randBuffer = TRNGCache->Data;

	u32Ret = MHRAND_Trand((uint32_t*) randBuffer, 16, &u32CrcInit, &u32CrcEnd);
	if(u32Ret != MH_RET_RAND_SUCCESS || u32CrcEnd != MHRAND_CheckBuff(u32CrcInit, (uint32_t*) randBuffer, 16))
	{
		faultDetected();
	}
	
	TRNGCache->IndexIn = 16;
	TRNGCache->IndexOut = 0;
	
	return ECP_ERR_RNG_GETRANDOM_OK;
}

/**
	使用Timer1进行delay clock 计数
**/
void delayNclock(unsigned int etu_num)
{
	u32 tmpV;
	UWORD timerCounter = 0xFFFFFFFF;
	
	tmpV = IoUart_GetF()/IoUart_GetD();
	timerCounter -= etu_num * tmpV;
	
	//初始timer
//	SLCxx_TIM1->TIM_CTRL = TIM_CTRL_RUN_STOP;
//	SLCxx_TIM1->TIM_CFG = 0x01;	//external clock
//	SLCxx_TIM1->TIM_RLD = timerCounter;
//	
//	//运行timer
//	SLCxx_TIM1->TIM_CTRL |= TIM_CTRL_RUN_RUN;
//	
//	//等待溢出（达到延迟）
//	while((SLCxx_TIM1->TIM_CTRL & TIM_CTRL_TF_Msk) == TIM_CTRL_TF_NOP);
//	
//	//清除溢出标志和关闭timer，TF和UNLOCK_TF需同时设置才能清除TF
//	SLCxx_TIM1->TIM_CTRL = TIM_CTRL_RUN_STOP | TIM_CTRL_UNLOCK_TF_Msk;
}
