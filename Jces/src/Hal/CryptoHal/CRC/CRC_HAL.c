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


#define CRCCON1_ARITH_SEL_32				0x20
#define CRCCON1_RES_SEQ_HIGHT_FIRST			0x08
#define CRCCON1_RES_REV_REFLECT				0x04
#define CRCCON1_DAT_REV_REFLECT				0x02


static void CRC_CalcAlign(void *PvAddr, uint32_t Size)
{
    uint8_t *pu8Addr;
    uint32_t u32Tmp, *pu32Align;

    pu32Align = (uint32_t *)ALGIN4_UP(PvAddr);
    pu8Addr = (uint8_t *)PvAddr;
    u32Tmp = (uint32_t)PvAddr + Size;
    
    if (pu8Addr < (uint8_t *)pu32Align)
    {
        CRC->CRC_CSR &= ~CRC_CSR_BYTE_NUM_Mask;//(3 << 6)
        
        for (pu8Addr = (uint8_t *)PvAddr; (uint32_t)pu8Addr < (uint32_t)pu32Align && (uint32_t)pu8Addr < u32Tmp; pu8Addr++)
        {
            CRC_IN(*pu8Addr);
        }
        CRC_WAIT();
        CRC->CRC_CSR |= CRC_CSR_BYTE_NUM_Mask;//(3 << 6)
    }
    else
    {
        CRC->CRC_CSR |= CRC_CSR_BYTE_NUM_Mask;//(3 << 6)
    }
    
    while (pu32Align < (uint32_t *)ALGIN4_DOWN(u32Tmp))
    {
        CRC_IN(*pu32Align++);
    }
    if ((uint32_t)pu32Align < u32Tmp)
    {
        CRC_WAIT();
        
        CRC->CRC_CSR &= ~CRC_CSR_BYTE_NUM_Mask;//(3 << 6)
        
        for (pu8Addr = (uint8_t *)pu32Align; pu8Addr < (uint8_t *)u32Tmp; pu8Addr++)
        {
            CRC_IN(*pu8Addr);
        }
    }
    CRC_WAIT();
    
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
#define CRC_MAX_BYTE_IN_ONE_STEP	0x20
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
			CRC->CRC_CSR = 0xC0;
			CRC->CRC_CSR |= (POLY_SEL_SET | XOR_OUT_SEL_SET);
		
			CRC->CRC_INI = initValue;
			CRC_CalcAlign(inBuff,inLen);
			initValue = CRC_OUT();
			//initValue = ~initValue;
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
			CRC->CRC_CSR = 0xC0;
			CRC->CRC_CSR |= (TYPE_SEL_SET | XOR_OUT_SEL_SET);
		
			if(CRC_MODE_JC3 == mode)
			{
				CRC->CRC_CSR |= (REV_OUT_SEL_SET | REV_IN_SEL_SET);
			}
			
			CRC->CRC_INI = initValue;
			
			CRC_CalcAlign(inBuff,inLen);
			initValue = CRC_OUT();
			
//			initValue = reflectLong(initValue);
//			initValue = ~initValue;
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
	
}
#endif  // WITH_CHECKSUM

#endif  // WITH_CRC
