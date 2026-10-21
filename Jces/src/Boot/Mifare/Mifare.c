/*
 * Mifare.c
 *
 *  Created on: 2014年4月4日
 *      Author: linguoming1
 */
#include "Mifare.h"
#include "MemOperation.h"
#include "MifareHal.h"
#include "MemBackUp.h"
#include "MifareHal.h"
#include "memBackUpServe.h"
#include "Crypto_API.h"
#include "SysConfig.h"
#include "APDU.h"
#include "Boot.h"

#if BOOT__IO__MIFARE

//兼容金融产业化脚本，使用 e08000 创建mifare
#define CREATE_MIFARE_JCES  0x00
#define CREATE_MIFARE       0x01
#define UPDATE_MIFARE_INFO  0x02
#define DISABLE_MIFARE      0x03
#define RELEASE_MIFARE      0xFF

#define MIFARE_READ_WRITE_SUCCESS   0x00
#define MIFARE_READ_WRITE_FAIL   	0x01


static u8 CheckBlocknum(u16 blockNum)
{
    if((ecpMifareHal_getMifareType() == CARD_4K))
    {
        if(blockNum > 0xFF )
        {
             return true ;
        }
    }
    else if(ecpMifareHal_getMifareType() == CARD_1K)
    {
        if(blockNum > 0x3F )
        {
             return true ;
        }
    }
	return false ;
}

/**
*added by huangzehua@20220408
*/
static void mifare_PrepareDesKey(PX8 mf_keyA, PX8 mf_keyB, PX8 des_keys)  //(lzm modified according [MISA])
{

	u8		X_A			tdeskey[8];
	u8		D_A			tempI;
	u8		D_A			tempJ;
	PATCH_BOOT_MIFARE_001();

	//for mf_keyA
	tdeskey[0]			= 0x00;
	tdeskey[1]			= 0x00;
    for ( tempI = 0; tempI <= 5; U8_INC(tempI) )
    {
    	if ( U8_AND_U8(mf_keyA[tempI], 0x80) == 0x80 )
        {
    		tempJ		= U8_A_U8(tempI, 1);
        	tdeskey[1]	= U8_OR_U8(tdeskey[1], U8_SHL_U8(0x01, tempJ));
        }
    }
//    tdeskey[1]			= U8_AND_U8(U8_SHL_U8(tdeskey[1], 1), 0x7E);

	tempI				= 0x02;
	tempJ				= 0x00;
	for( ; tempJ < 6 ; )
	{
		tdeskey[tempI]	= U8_AND_U8(U8_SHL_U8(mf_keyA[tempJ], 1), 0xFE);
		U8_INC(tempI);
		U8_INC(tempJ);
	}

	ecpMem_Copy(des_keys, tdeskey, DES_BLOCKSIZE);

	//for mf_keyB
	for(tempI = 0 ; tempI < 6 ; U8_INC(tempI) )
	{
		tdeskey[tempI]	= U8_AND_U8(U8_SHL_U8(mf_keyB[tempI], 1), 0xFE);
	}

	tdeskey[6]		= 0x00;
    for ( tempI = 0; tempI <= 5; U8_INC(tempI) )
    {
    	if ( U8_AND_U8( mf_keyB[tempI], 0x80 ) == 0x80 )
        {
    		tempJ		= U8_S_U8(6, tempI);
        	tdeskey[6]	= U8_OR_U8(tdeskey[6], U32_SHL_U32(0x01, U8_A_U8(tempI, 1)));
        }
    }

	tdeskey[7]		= 0x00;

	ecpMem_Copy(&des_keys[DES_BLOCKSIZE], tdeskey, DES_BLOCKSIZE);
	return;
}
//dataBuffer可能是RAM或者EEPROM，调用函数VMWriteBuf写入dataBuffer, by xwh 20211005
u8 mifare_Read(u8 blockNum, u8 *dataBuffer, u8 data_length, u8 * MF_key)
{
	u32 result = MIFARE_READ_WRITE_FAIL;
	u8  arry[16];
//	u8 sectorNum;

	if(CheckBlocknum(blockNum))
	{
		return 0x02;
	}

	if(blockNum < 128)
	{
		//如果读取的是该sector的key block，拒绝读取；
		if((blockNum & 0x03) == 0x03 )
		{
		//	return 0x02;
		}
	}
	//4K Mifare卡后8个扇区，每个扇区有4个Block；
	else
	{
		/*拒绝访问32-39个扇区控制块的内容*/
	    if((blockNum & 0x0F) == 0x0F )
		{
			//return  0x02;
		}
	}

	//result = ecpMifareHal_ReadBlock(blockNum, dataBuffer, MF_key) ;
	result = ecpMifareHal_ReadBlock(blockNum, arry, MF_key) ;
	
	if(result == 0)
	{	
		//initMifare();
		memCopyAtomic(dataBuffer,arry,16); // VMWriteBuf(PXtMEM(dataBuffer),arry,16);
		return MIFARE_READ_WRITE_SUCCESS;
	}
	else
	{
		return MIFARE_READ_WRITE_FAIL;
	}
}
//dataBuffer可能是RAM或者EEPROM，调用函数VMWriteBuf写入dataBuffer, by xwh 20211005
u8 mifare_write(u8 blockNum, u8 *dataBuffer, u8 data_length, u8 *MF_key)
{
	u32 result = MIFARE_READ_WRITE_FAIL;
	u8  arry[16];

	if(CheckBlocknum(blockNum))
	{
		return 0x02;
	}
	memCopyToRam(arry,dataBuffer,16); //	VMReadBuf(arry, (memref)dataBuffer, 16);

	//result = ecpMifareHal_WriteBlock(blockNum, dataBuffer, MF_key) ;
	result = ecpMifareHal_WriteBlock(blockNum, arry, MF_key) ;

	if(result == 0)
	{
	//	initMifare();
		return MIFARE_READ_WRITE_SUCCESS;
	}
	else
	{
		return MIFARE_READ_WRITE_FAIL;
	}
}
/**
*modify by huangzehua@20220408
*/
void mifare_config(u8 isDeselect)	SMALL_MODE
{
	u16			X_A		tempAtqa;
	u8			X_A		tempSakCom;
	u8			X_A		tempSakInCom;
	u8			X_A		tempL3Config;

	PATCH_BOOT_MIFARE_005();

	ecpMifare_GetMifareL3Param(&tempAtqa, &tempSakCom, &tempSakInCom, isDeselect);

	tempL3Config		= sysConfig_GetMifareL3Conf();

	if(U8_AND_U8(tempL3Config, IOCONFIG_MIFARE_L3_ATQA_MASK) == IOCONFIG_MIFARE_L3_ATQA_SMEM)
	{
		tempAtqa		= sysConfig_GetMifareUserATQ();
	}
	else
	{
		tempAtqa = ecpMifare_GetAtqa();
	}

	if(U8_AND_U8(tempL3Config, IOCONFIG_MIFARE_L3_SAK_MASK) == IOCONFIG_MIFARE_L3_SAK_FLEX)
	{
		tempSakCom		= sysConfig_GetMifareUsersakComplete();
		tempSakInCom	= sysConfig_GetMifareUsersakInComplete();
	}
	else
	{
		tempSakCom = ecpMifare_GetSakcomplete();
		tempSakInCom = ecpMifare_GetSakIncomplete();
	}

	ecpMifare_SetMifareL3Param(tempAtqa, tempSakCom, tempSakInCom, tempL3Config);
}
/**
*modify by huangzehua@20220408
*/
u8 mifare_Active(void)
{
	return ecpMifare_Active();
}

//u8 isMifareActivated(void)
//{
//	if (isMifareAvalible() == CARD_NONE)
//	{
//		return 0x00;
//	}

//	return 0x01;
//}


//u8 CheckBlocknum(u16 other_block)
//{
//	if((isMifareAvalible() == CARD_4K))
//	{
//		if(other_block > 0xFF )
//		{
//			return true ;
//		}
//    }
//	else if(isMifareAvalible() == CARD_1K)
//	{
//		if(other_block > 0x3F )
//		{
//			return true ;
//		}
//	}
//	return false ;
//}

void mifare_GenPw(u8 *pKeyA, u8 *pKeyB, u8 *pAccPWD)
{
	u8 bInitValue[8], bKeyA[6], bKeyB[6], bDKeyA[8], bDKeyB[8], bTemp_Key[16], bTemp_PWD[8];
	u8 i, j;
	//在这计算password时没有用到ICVBuff
	u8 ICVBuff[8] = {0};

	memcpy(bKeyA,pKeyA,6);

	memcpy(bKeyB,pKeyB,6);

	memset(pAccPWD,0x00,sizeof(pAccPWD));
/////////////////////////////////////////////////////////////////////////
	////DKeyB计算
	bDKeyB[7]=(bKeyB[5]&0x7F)<<1;
	bDKeyB[6]=(bKeyB[4]&0x7F)<<1;
	bDKeyB[5]=(bKeyB[3]&0x7F)<<1;
	bDKeyB[4]=(bKeyB[2]&0x7F)<<1;
	bDKeyB[3]=(bKeyB[1]&0x7F)<<1;
	bDKeyB[2]=(bKeyB[0]&0x7F)<<1;
	
	bDKeyB[1]= 0x00 | ((bKeyB[5]&0x80)>>1) | ((bKeyB[4]&0x80)>>2) | ((bKeyB[3]&0x80)>>3) | ((bKeyB[2]&0x80)>>4)
		| ((bKeyB[1]&0x80)>>5) | ((bKeyB[0]&0x80)>>6) ;
	bDKeyB[0]=0x00;
//////////////////////////////////////////////////////////////////////////
	////DKeyA计算
	bDKeyA[7]=0x00;

	bDKeyA[6]= 0x00 | ((bKeyA[0]&0x80)>>1) | ((bKeyA[1]&0x80)>>2) | ((bKeyA[2]&0x80)>>3) | ((bKeyA[3]&0x80)>>4)
		| ((bKeyA[4]&0x80)>>5) | ((bKeyA[5]&0x80)>>6) ;

	bDKeyA[5]=(bKeyA[5]&0x7F)<<1;
	bDKeyA[4]=(bKeyA[4]&0x7F)<<1;
	bDKeyA[3]=(bKeyA[3]&0x7F)<<1;
	bDKeyA[2]=(bKeyA[2]&0x7F)<<1;
	bDKeyA[1]=(bKeyA[1]&0x7F)<<1;
	bDKeyA[0]=(bKeyA[0]&0x7F)<<1;
	
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//	memcpy(bTemp_Key,bDKeyA,8);
	for (i=0,j=7;i<8;i++,j--)
	{
		bTemp_Key[j]=bDKeyA[i];
	}

//	memcpy(bTemp_Key+8,bDKeyB,8);
	for (i=0,j=7;i<8;i++,j--)
	{
		bTemp_Key[8+j]=bDKeyB[i];
	}
//////////////////////////////////////////////////////////////////////////
	memset(bInitValue,0x00,sizeof(bInitValue));

	//Des(8,EN_TRIPLE_DES,bTemp_Key,bInitValue,bTemp_PWD);
	Triple_DES(bInitValue, bTemp_Key, bTemp_PWD, 8, ICVBuff, ECB_Encrypt, SENSITIVE_DATA_MASKED);

	pAccPWD[0]=bTemp_PWD[7];
	pAccPWD[1]=bTemp_PWD[6];
	pAccPWD[2]=bTemp_PWD[5];
	pAccPWD[3]=bTemp_PWD[4];
	pAccPWD[4]=bTemp_PWD[3];
	pAccPWD[5]=bTemp_PWD[2];
	pAccPWD[6]=bTemp_PWD[1];
	pAccPWD[7]=bTemp_PWD[0];	

}


u16 mifare_Function(PX8 apduBuffer)
{
   
	u16 result = BOOT_NO_ERROR;
	u8 type = apduBuffer[OFFSET_P1];
	u8 sectorNumber = apduBuffer[OFFSET_P2];
	u8 dataLength ;
	u8 blockNumber = 0;
    u8 i;
	
	PATCH_BOOT_022();
		
	if(SECURITY_STATE_ATTACK == boot_GetSecurityState())
	{
	 	return BOOT_SECURITY_STATUS_NOT_SATISFIED;
	}
	
	apdu_ApduSetIncomingAndReceive();

	//modified by zm;
	dataLength = apduBuffer[OFFSET_LC];
	/*
	if(securityState == SECURITY_STATE_ATTACK)
	{
	 	return SW_SECURITY_STATUS_NOT_SATISFIED;
	}*/
	switch(type)
	{
		case CREATE_MIFARE_JCES:
		case CREATE_MIFARE:
        {   
			if(ecpMifareHal_getMifareType() != CARD_NONE)
			{
				result = BOOT_CONDITIONS_NOT_SATISFIED;
			}
			else
			{
				if((apduBuffer[OFFSET_P2] == CARD_1K) || (apduBuffer[OFFSET_P2] == CARD_4K))
				{
					ecpMifareHal_Create(apduBuffer[OFFSET_P2]);
				}
				else
				{
					result = BOOT_CONDITIONS_NOT_SATISFIED;
				}
            }
        }
		break;

		case UPDATE_MIFARE_INFO:

			if(dataLength != 0x10)
			{
				return BOOT_WRONG_LENGTH;
			}

			if(ecpMifareHal_getMifareType() == CARD_1K)
			{
				if(sectorNumber < 16)
				{
					blockNumber = (sectorNumber * 4) + 3;
					
					//ecpMifareHal_WriteBlock(blockNumber, &apduBuffer[OFFSET_CDATA]);
				}
				else
				{
					result = BOOT_CONDITIONS_NOT_SATISFIED;
					break;
				}
			}
			else if(ecpMifareHal_getMifareType() == CARD_4K)
			{
				if(sectorNumber < 32)
				{
					blockNumber = (sectorNumber * 4) + 3;
				}
				else if(sectorNumber < 40)
				{
					blockNumber = 128 + (sectorNumber - 32)*16 + 15;
				}
				else
				{
					result = BOOT_CONDITIONS_NOT_SATISFIED;
					break;
				}
				//ecpMifareHal_WriteBlock(blockNumber, &apduBuffer[OFFSET_CDATA]);
			}
			else
			{
				result = BOOT_CONDITIONS_NOT_SATISFIED;
			}

			break;
		
		case RELEASE_MIFARE:
            result = ecpMifareHal_Release();
			break;

		case DISABLE_MIFARE:
			if (apduBuffer[OFFSET_P2] == 0x00)
			{
				//enable mifare
				//nvmWriteU8Atomic( MEMtPN8(JcvmConfig_GetVmConfigAddr(IFX_SWITCH)), nvmReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(IFX_SWITCH))) & ~MIFARE_SWITCH_MASK);
			}
			else if (apduBuffer[OFFSET_P2] == 0x80)
			{
				//disable mifare
				//nvmWriteU8Atomic( MEMtPN8(JcvmConfig_GetVmConfigAddr(IFX_SWITCH)), nvmReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(IFX_SWITCH))) | MIFARE_SWITCH_MASK);
			}
			else
			{
				result = BOOT_MIFARE_WRONG_P1;
			}
			
			//changeMifareUID();
			break;

		default:
			result = BOOT_MIFARE_WRONG_P1;
	}
	return result;
}

u8 mifare_DataConfigCheck(u8 tagH, u8 tagL, u8 thisecurityState, u8 lifeCycle, u8 isStoreData)
{
	u8 result = true;

	if(thisecurityState == 0x00 && tagL != 0x00)
	{
	  result = false ;
	}

	if(true != isStoreData && ALL_TAG_ENABLE != 0x00 )
	{
		switch(tagL)
		{
			case 0x00:
				result = true ;
				break ;
			default:
				result = false ;
				break ;	
		}
	}

	return result;
}

u8 mifare_DataConfigGenNewValue(u8 tagH, u8 tagL, u8 oldValue, u8 newValue, u8 isStoreData)
{
	u8		tempShift;

	if(tagL == 0)
	{
		if(true == isStoreData)
		{
			oldValue	= U8_AND_U8(oldValue, 0xFE);
			newValue	= U8_OR_U8(oldValue, newValue);
		}
		else
		{
			newValue	= U8_AND_U8(oldValue, U8_INV(0xFE));
		}
	}

	if((tagL >= 1) && (tagL <= 4))
	{
		tempShift	= U8_S_U8(tagL, 1);

		if(true == isStoreData)
		{
			oldValue	= U8_AND_U8(oldValue, U8_INV(U8_SHL_U8(3, tempShift)));
			newValue	= U8_OR_U8(oldValue, U8_SHL_U8(newValue, tempShift));
		}
		else
		{
			oldValue	= U8_AND_U8(oldValue, U8_SHL_U8(3, tempShift));
			newValue	= U8_SHR_U8(oldValue, tempShift);
		}
	}

	return newValue;
}
/*******************************************************************************
 * Function name: derive4UidFrom7Uid
 * Description: This function is used to derive 4 byte NUID out of 7 byte UID,
 *              and the process method is defined in 3.3.2 of AN10927 (MIFARE
 *              and handling of UIDs) Rev.3.0, 4 August 2011, 190730(NXP).
 * Input Parameters:   (precondition)
 *		None.
 * Output Parameters:  (postcondition)
 *		The result 4 byte NUID is stored in the corresponding configure area.
 * NOTE:
 ********************************************************************************/
u16 mifare_Derive4UidFrom7Uid(void)
{
 return 0;
}


void changeMifareUID(void)
{
//	if ( (nvmReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(IFX_SWITCH))) & MIFARE_SWITCH_MASK) == 0x00)
//	{	//Mifare Enable
//		
//		//backup current uid
//		memCopyNoAtomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CURRENT_UID)), MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_UID)), memReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_UID))) + 1);
//		
//		//set mifare uid
//		memWriteU8NoAtomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_UID)), 0x04);
//		memCopyNoAtomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_UID + 1)), MEMtPN8(JcvmConfig_GetVmConfigAddr(MIFARE_UID)), 4);
//	}
//	else
//	{	//Mifare Disable
//		memCopyNoAtomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_UID)), MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CURRENT_UID)), memReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CURRENT_UID))) + 1);
//	}
}


#endif
