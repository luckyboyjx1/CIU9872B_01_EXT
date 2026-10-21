/*
 * SecOperationHal.c
 *
 *  Created on: 2014Äê11ÔÂ26ÈÕ
 *      Author: jiangxiaohui
 */

#include "SecOperationHal.h"
#include "chipHead.h"
#include "RNG_API.h"

u32 NmiLog_AttackCounter;
u8 edcFlag;
PM8						X_A		g_RSA_IV;
u8						X_A		g_keyTureByteLen;
RSACRTKeyDPQRefStore 			rsaCRTKeydPQRef;
u16                     X_A     g_CodeFlowControlFlag;

#if	SC_EDC_CHECK_SWITCH_ON
void SC_deactiveEDC(void)
{
	
	if(SC_NVMParityIsActive())
	{
		SC_deactiveNVMParityCheck();
		edcFlag = 0x5A;
	}
	else
	{
		edcFlag ++;
	}
}
void SC_activeEDC(void)
{
	if(edcFlag == 0x5A)
	{
		SC_activeNVMParityCheck();
	}
	else
	{
		edcFlag --;
	}
}
#endif
#if RAM_COUNTER
u32    X_A    g_sensitiveState;
#endif

#if VM_ALLOW_CARD_RESET
void reset_card(void)
{
	// Software reset, the device can then only be reset after a power-on cycle.
	
}
#endif

//System fault operation function
void faultDetected(void)
{
#if !MUTE_CARD
	while(1);
#else
	u32 i;
	u32 j;
	u32 attackFlag;
	u16 progSuccess;
	u8  attackCounterByte;
	u8 * attackCtrPtr;
	
	progSuccess       = 0x5A7A;
	attackCounterByte = STHardConfig_GetStEraseramCount(); //use configure area to set in prepersonalization (0x20)
	attackCtrPtr      = (PM8)&NmiLog_AttackCounter;
	
	r_SEC_CTRL_BIT(SEC_CTRL_PF_EDC_DATA_READ_EN_BIT) = 0;
	for(i = 0; i < attackCounterByte; i++)
	{
		attackFlag = *attackCtrPtr;
		if(attackFlag == 0xFF)
		{
			for(j = 0; j < 3; j++)
			{
				NVM_ProgByte(0x00, attackCtrPtr);
				if (*attackCtrPtr == 0x00)
				{
					progSuccess = 0x7A5A;
					break;
				}
			}
		}
		if(progSuccess == 0x7A5A)
		{
			if(*attackCtrPtr == 0x00)
			{
				break;
			}
		}
		attackCtrPtr++;		
	}
	r_SEC_CTRL_BIT(SEC_CTRL_PF_EDC_DATA_READ_EN_BIT) = 1;
	
	reset_card();
#endif
}



#if RANDOM_DELAY_ENABLE
void randomDelay()
{
	u16		      X_A retStatus;
	u8            X_A randByte;
	retStatus = ECP_ERR_RNG_GETRANDOM_FAILURE;
	do {
		retStatus = getDataFromRngCache(&randByte, 0x01);
		if (retStatus != ECP_ERR_RNG_GETRANDOM_OK)
		{
			faultDetected();
		}
	} while(randByte == 0);
	while(randByte--);
}
#endif

#if ATTACK_COUNTER_AUDIT
u16 checkAttackCtr()
{
	u8 i;
	u8 * attackCtrPtr;
	u8 attackCounterByte;
	u8 attackCounter;
	u8 attackCounterBak;
	
	attackCounterByte = STHardConfig_GetStEraseramCount(); //use configure area to set in prepersonalization (0x20)
	attackCtrPtr      = (PM8)&NmiLog_AttackCounter;
	attackCounter     = 0;
	attackCounterBak  = 0;
	
	for(i = 0; i < attackCounterByte; i++)
	{
		if(*attackCtrPtr != 0xFF)
		{
			attackCounter++;
			attackCounterBak++;
		}
		attackCtrPtr++;
	}
	if(i != attackCounterByte)
	{
		faultDetected();
	}
	if(attackCounter >= attackCounterByte)
	{
		SC_terminateCard();
	}
	if(attackCounterBak >= attackCounterByte)
	{
		SC_terminateCard();
	}
	return ECP_ERR_OK;
}
#endif


#if G_COUNTER_CODE_FLOW_CTRL
void checkCodeFlowControlFlag(u8 algType)
{
	switch(algType)
	{
		case 0x7A:    //DES signature
			if ((g_CodeFlowControlFlag % 5) != 0)
			{
				faultDetected();
			}
			break;
		case 0x7B:    //DES signature verify
			if ((g_CodeFlowControlFlag % 5) != 4)
			{
				faultDetected();
			}
			break;
		case 0x7C:    //RSA signature
			if (g_CodeFlowControlFlag != 7)
			{
				faultDetected();
			}
			break;
		case 0x7D:    //RSA signature verify
			if (g_CodeFlowControlFlag != 7)
			{
				faultDetected();
			}
			break;
		default:
			faultDetected();
			break;
	}
	initCodeFlowControlFlag();
}
#endif


void SetupShares(u16 num_bytes, u8 * plain_bytes, u8 * share_1, u8 * share_2)
{
	u16 * inData1;
	u16 * inData2;
	u16 * outData;
	u16 randValue;
	u16 b;
	u16 index;
	u16 length;

	do{
		//randValue 	= Hrng_GetRandomWord_internal(&randValue);
		randValue 	= 0X5AA5;
	}while(randValue == 0);

	length 		= num_bytes >> 1;
	index		= randValue & (length - 1);

	if(randValue & 0x0104)
	{
		inData1 = MEMtPX16(plain_bytes);
		inData2 = MEMtPX16(share_1);
	}
	else
	{
		inData1 = MEMtPX16(share_1);
		inData2 = MEMtPX16(plain_bytes);
	}
	outData		= MEMtPX16(share_2);

	if(randValue & 0x1040)
	{
		for (b = index; b < length; b++)
		{
			// initialize second share with masked data
			outData[b] = inData1[b] ^ inData2[b];
		}
		for (b = 0; b < index; b++)
		{
			// initialize second share with masked data
			outData[b] = inData1[b] ^ inData2[b];
		}
	}
	else
	{
		for (b = index; b > 0; )
		{
			b--;
			// initialize second share with masked data
			outData[b] = inData1[b] ^ inData2[b];
		}
		for (b = length; b > index; )
		{
			b--;
			// initialize second share with masked data
			outData[b] = inData1[b] ^ inData2[b];
		}
	}
}

#if WITH_SECURE_DATA_STORAGE
u16 SC_DataMask(SC_DataMaskParaStruct dataMaskPara)
{
	u16		X_A		retStatus;

	retStatus 		= ECP_ERR_FAILURE;

	SC_parameterIntegrityCheck(dataMaskPara);

	switch( dataMaskPara.maskType & DATA_TYPE_MASK)
	{
		case DATA_TYPE_PIN:
			SetupShares(dataMaskPara.Len, dataMaskPara.dataBuff, (PM8)&g_TRNG_Cache.Data[SYM_RNG_KEY_OFFSET], dataMaskPara.dataBuff);
			SetupShares(dataMaskPara.Len, dataMaskPara.dataBuff, MEMtPM8(&rommask[dataMaskPara.maskValue << 2]), dataMaskPara.dataBuff);
			if( DATA_MASK_ENC == (dataMaskPara.maskType & DATA_EN_DE_MASK) )
			{
				SetupShares(dataMaskPara.Len, dataMaskPara.dataBuff, (PM8)&g_TRNG_Cache.Data[SYM_RNG_KEY_OFFSET], dataMaskPara.dataBuff);
			}
			retStatus 		= ECP_ERR_OK;
			break;
		case DATA_TYPE_DES:
			SetupShares(dataMaskPara.Len, dataMaskPara.dataBuff, (PM8)&g_TRNG_Cache.Data[SYM_RNG_KEY_OFFSET], dataMaskPara.dataBuff);
			SetupShares(dataMaskPara.Len, dataMaskPara.dataBuff, MEMtPM8(&rommask[dataMaskPara.maskValue << 2]), dataMaskPara.dataBuff);
			if( DATA_MASK_ENC == (dataMaskPara.maskType & DATA_EN_DE_MASK) )
			{
				SetupShares(dataMaskPara.Len, dataMaskPara.dataBuff, (PM8)&g_TRNG_Cache.Data[SYM_RNG_KEY_OFFSET], dataMaskPara.dataBuff);
			}
			retStatus 		= ECP_ERR_OK;
			break;
		case DATA_TYPE_KEYSET:
			if( DATA_MASK_ENC == (dataMaskPara.maskType & DATA_EN_DE_MASK) )
			{
				SetupShares(dataMaskPara.Len, dataMaskPara.dataBuff, (PM8)&g_TRNG_Cache.Data[SYM_RNG_KEY_OFFSET], dataMaskPara.dataBuff);
				SetupShares(dataMaskPara.Len, dataMaskPara.dataBuff, MEMtPM8(&rommask[dataMaskPara.maskValue << 2]), dataMaskPara.dataBuff);
				SetupShares(dataMaskPara.Len, dataMaskPara.dataBuff, (PM8)&g_TRNG_Cache.Data[SYM_RNG_KEY_OFFSET], dataMaskPara.dataBuff);
			}
			else
			{
				while(dataMaskPara.Len >= MAX_DES_KEY_LENGTH)
				{
					SetupShares(MAX_DES_KEY_LENGTH, dataMaskPara.dataBuff, (PM8)&g_TRNG_Cache.Data[SYM_RNG_KEY_OFFSET], dataMaskPara.dataBuff);
					SetupShares(MAX_DES_KEY_LENGTH, dataMaskPara.dataBuff, MEMtPM8(&rommask[dataMaskPara.maskValue << 2]), dataMaskPara.dataBuff);
					dataMaskPara.dataBuff 	+= MAX_DES_KEY_LENGTH;
					dataMaskPara.Len 		-= MAX_DES_KEY_LENGTH;
				}
			}
			retStatus 		= ECP_ERR_OK;
			break;
		case DATA_TYPE_RSA:
			if( DATA_MASK_ENC == (dataMaskPara.maskType & DATA_EN_DE_MASK) )
			{
				g_keyTureByteLen = SC_rsaKeyMask(dataMaskPara.dataBuff, dataMaskPara.dataBuff, dataMaskPara.Len, (PM8)&rommask[dataMaskPara.maskValue << 2]);
			}
			else
			{
				g_RSA_IV = (PM8)&rommask[dataMaskPara.maskValue << 2];
			}
			retStatus 		= ECP_ERR_OK;
		break;
		default:
			faultDetected();
		break;
	}

	incrementCodeFlowControlFlag();

	return retStatus;
}

void SC_KEYSETUnMask(PM8 keyData, PM8 outBuffer)
{
	u8		X_A		index;

	for(index = 0; index < LEN_OF_KEYSET_ARRAY; index += MAX_DES_KEY_LENGTH)
	{
		SetupShares(MAX_DES_KEY_LENGTH, &keyData[index], (PM8)&g_TRNG_Cache.Data[SYM_RNG_KEY_OFFSET], &keyData[index]);
	}

	SC_checkReturnCode(SC_MemCopyAPI(outBuffer, keyData, LEN_OF_KEYSET_ARRAY, ECP_COPY_TYPE_ATOMIC));

	ecpMem_Set(keyData, g_TRNG_Cache.Data[SYM_RNG_KEY_OFFSET], LEN_OF_KEYSET_ARRAY);
}

u16 SC_rsaKeyMask(PX8 result, PM8 d1_Addr,u16 d1_len, PM8 d2_Addr)
{
	u16 keyTureByteLen;

	keyTureByteLen = d1_len;

	while((memReadU8(d1_Addr)) == 0)
	{
		d1_Addr++;
		keyTureByteLen--;
	}

	if(keyTureByteLen > 4)
	{
		BigData_Sub(result, d1_Addr, d1_len, d2_Addr, (keyTureByteLen - 1), TRUE);
	}

	return keyTureByteLen;
}

#endif


