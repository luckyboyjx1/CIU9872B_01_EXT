#include "RfHal.h"
#include "SysConfigProfile.h"
#include "chipHead.h"
#include "CRC_API.h"
#include "RNG_API.h"

#if HAL__IO__ISO14443_TA


/*===========================================================================*
 * Exported Functions:
 *===========================================================================*/
/*============================================================================*/
void ecpCiu_Init(void)
{
	PATCH_HAL_IOHAL_RFHAL_001();
	rfPort_init();
}
u8 ecpCiu_SetBaudrate(u8 inBaudrate, PR8 pOutBaudrate)		SMALL_MODE
{
	PATCH_HAL_IOHAL_RFHAL_002();
	return 0;
}

u16	ecpCiu_Anticollision(PX8 pData, u16 dataSize, PX16 fullness, u8 sakComplete, u8 sakIncomplete, PR8 pCascadeLevel)
{
	PATCH_HAL_IOHAL_RFHAL_003();
	return rfPort_Anticollision(pData, dataSize, fullness, sakComplete, sakIncomplete, pCascadeLevel);
}

u16 ecpCiu_RxBuffer(PX8 pData, u16 dataSize, PR16 fullness, u8 flags, u16 fsc)	SMALL_MODE
{
	PATCH_HAL_IOHAL_RFHAL_006();
	return rfPort_RxBuffer(pData, dataSize, fullness, flags, fsc);
}

void ecpCiu_TxBuffer(PX8 pData, u16 fullness, u8 flags)
{
	PATCH_HAL_IOHAL_RFHAL_009();
	rfPort_TxBuffer(pData, fullness, flags);
}



void ecpCiu_RandomUID(PX8 rUID_Buffer)
{

	Random_number_generator(&rUID_Buffer[1], 3, ALG_SECURE_RANDOM);	
	rUID_Buffer[4] = rUID_Buffer[0] ^ rUID_Buffer[1] ^ rUID_Buffer[2] ^ rUID_Buffer[3];
}

void RfAutoFrequeControl(u8 flag)
{
	return;
}

#if BOOT__IO__ISO14443_TA_RF_LEVEL3

u8 ecpCiu_HardWareSupportStateMachine(PX8 pData, PX16 fullness, u8 isDeselect)
{
	PATCH_HAL_IOHAL_RFHAL_005();

	return pData[0];
}

u8 ecpCiu_HardWareSupportReceiveRatsHalt(void)
{
	return 0;
}

void ecpCiu_HardwareInit(u8 isDeselect, u8 sak, u8 insak,u8 ATQA_b9_b16,u8 ATQA_b1_b8, PX8 uid )
{
	PATCH_HAL_IOHAL_RFHAL_010() ;
}

void ecpCiu_HardwareHandleDeselect(void)
{
	PATCH_HAL_IOHAL_RFHAL_007();

}

void ecpCiu_HardwareHandleErrorCommond(void)
{
	PATCH_HAL_IOHAL_RFHAL_008();
}
#endif

#if WITH_RF_USER_MODIFY_PARAM
void ecpCiu_SetUID(PM8 uidAry)
{

	return;
}

void ecpCiu_SetAtqa(u16 atqaValue)
{
}

void ecpCiu_SetSakComplete(u8 sakComplete)
{
	
}
void ecpCiu_SetSakInComplete(u8 sakIncomplete)
{

}
#endif  //WITH_RF_USER_MODIFY_PARAM

#if WITH_RF_DERIVE_UID
void ecpCiu_derive4UidFrom7Uid(PX8 des)
{
	u32	 			checksum;
	PM8  			src;
	PATCH_HAL_IOHAL_RFHAL_004();
	checksum 		= 0x6363;
	src				= MEMtPM8(0xDFFF80);

	checksum		= CRCDoCal(src, 3, checksum, ALG_ISO3309_CRC16, CRC_MODE_JC3);
	PX_W_U16(des, checksum);
	des[0]			= (u8)(checksum | 0x0f);			 	// the lower nibble of UID0 must be set t0 Fh to indicate the non-unique range.
	des[0]			&= 0xEF; 								// bit 4 of NUID0 shall be set to 0b for compliance reasons.
	checksum		= CRCDoCal(PM8_A_U8(src, 3), 4, checksum, ALG_ISO3309_CRC16, CRC_MODE_JC3);
	PX_W_U16(PX8_A_U8(des, 2), U32t16(checksum));
	return;
}


#endif //WITH_RF_DERIVE_UID
#endif	//HAL__IO__ISO14443_TA
