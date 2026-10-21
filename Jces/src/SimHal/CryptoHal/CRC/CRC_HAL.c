/************************************************************
 Copyright (C), 1988-1999, Eastcompeace. Co., Ltd.
 FileName: Crypto_HAL.c.
 Author: Lizumeng.
 Version : 1.0.
 Date: 2010/02/20.
 Description: This source file contains functions used to encrypt or
 decrypt with hardware implement.
 History:
 <author>    <time>    <version>    <desc>
 Lizumeng   10/02/20     1.0     build this moudle
 *************************************************************/

#include "CRC_API.h"
#include "Crypto_SoftCRC.h"
#include "chipHead.h"

#if WITH_CRC

/**********************************************************************************
 * Function name: CRCHardwareInit
 * Description: This function does CRC16/CRC32 kernel initialization.
 * Input parameter:
 *      u32 initValue       initial value
 *      u8 algType          algorithm mask
 * Output parameter:
 *		none
 * NOTE: none
 **********************************************************************************/
void CRCHardwareInit(u32 initValue, u8 algType) 	SMALL_MODE
{
	PATCH_HAL_CRYPTO_CRC_HAL_001();	
	return;
}

/**********************************************************************************
 * Function name: CRCHardwareCal
 * Description: This function does CRC16/CRC32 kernel operation with hardware implement.
 * Input parameter:
 *		MEM_U8_P inBuff		address of buffer
 *		u16 inLen           length of buffer
 * Output parameter:
 *		Checksum result
 * NOTE: none
 **********************************************************************************/
u32 CRCDoCal(PM8 inBuff, u16 inLen, u32 initValue, u8 algType, u8 mode)
{
	PATCH_HAL_CRYPTO_CRC_HAL_002();
	CRCHardwareInit(initValue, algType);

	switch(algType)
	{
		case ALG_ISO3309_CRC16:
			initValue	= U16t32(reflectShort(U32t16(initValue)));
#if CRC16_BIT_CACULATE
			initValue 	= Calculate_CRC16_CCITT_BitCalculateMethod(inBuff, inLen, U32t16(initValue));
#endif

#if CRC16_LOOKUPTABLE_CACULATE
			if (inLen != 0)
			{
				initValue = Calculate_CRC16_CCITT_LookTableMethod(inBuff, inLen, U32t16(initValue));
			}
#endif
			initValue = ~initValue;
			break;

		case ALG_ISO3309_CRC32:
			initValue	= reflectLong(initValue);
#if CRC32_BIT_CACULATE
			initValue = Calculate_CRC32_IEEE802_3_BitCalculateMethod(inBuff, inLen, initValue, mode);
#endif
#if CRC32_LOOKUPTABLE_CACULATE
			if (inLen != 0)
			{
				initValue = Calculate_CRC32_LookTableMethod(inBuff, inLen, initValue, mode);
			}
#endif
			initValue = reflectLong(initValue);
			initValue = ~initValue;
			break;
	   default:
	        break;
	}

	return initValue;
}

#if WITH_CHECKSUM
/**********************************************************************************
 * Function name: setKeyChecksumAlg
 * Description: This function generates checksum of the secret key/value, and append
 *              the checksum to the end of the key buffer.
 * Input parameter:
 *      MEM_U8_P keyBuff    secret value/key buffer
 *      u16 keyLen          length of the buffer
 * Output parameter:
 *		 none
 * NOTE: none
 **********************************************************************************/
void setKeyChecksumAlg(PM8 keyBuff, u16 keyLen)
{
	u8 checkValue[2];

	CRC_CON = CRC_CON_INIT_MASK; // set init mode and CRC16 mode
	CRC_CON |= CRC_CON_SET2F_MASK; // fast initialization to 0xFFFF
	CRC_CON ^= CRC_CON_INIT_MASK; // set calculate mode

	//if (isE2PAddress(inBuff))
	if (*(u8 *)&keyBuff != 0)
	{
		while (keyLen != 0)
		{
			CRC_DAT0 = PM_R_U8(keyBuff);
			keyBuff++;
			keyLen--;
		}
	}
	else
	{
		while (keyLen != 0)
		{
			CRC_DAT0 = *keyBuff;
			keyBuff++;
			keyLen--;
		}
	}

	checkValue[0] = CRC_DAT0;
	checkValue[1] = CRC_DAT1;

	//if (isE2PAddress(keyBuff))
	if (*(u8 *)&keyBuff != 0)
	{
		E2P_write_u16((memref)keyBuff, *(u16 *)checkValue);
	}
	else
	{
		RAM_write_u16(keyBuff, *(u16 *)checkValue);
	}
}
#endif  // WITH_CHECKSUM

#endif  // WITH_CRC
