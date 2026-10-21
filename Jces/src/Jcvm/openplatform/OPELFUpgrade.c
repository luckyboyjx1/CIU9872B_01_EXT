#include "opFunction.h"
#include "OPELFUpgrade.h"
#include "global_variable.h"
#include "loader.h"
#include "cardManager.h"
#include "ecpdefine.h"
#include "contentManager.h"
#include "Crypto_API.h"
#include "MemOperation.h"

//for ELF Upgrade

 u16 opElfUpgradeStart(void)
{
	u8 					 upgradeRquestTag;
	u16 				 offset;

	u8                   oldElfUpgradeAidLen;
	u8                   newElfUpgradeAidLen;
//	u8                   upgradeRquestlength;
//	u8                   upgradeRquestOffset;
//	u8                   upgradeSessionDigestTag;
//	PM8                  sha256digest;
//	PM8                  upgradeSessionDigest;
//	u8                   arrayLenth;
	offset = OFFSET_CDATA;
	//apdu_ApduSetIncomingAndReceive();
	upgradeRquestTag = APDUBuffer[offset];

	if (upgradeRquestTag != 0xA1)
	{
		return SW_WRONG_DATA;
	}
//	upgradeRquestlength =APDUBuffer[(offset+1)];

	offset = U8_A_U8(offset, 2);
//	upgradeRquestOffset = offset;
	upgradeRquestTag = APDUBuffer[offset++];
	if (upgradeRquestTag == 0x4F)
	{
		oldElfUpgradeAidLen = APDUBuffer[offset++];
		if (AIDLenInvalid(oldElfUpgradeAidLen))
		{
			return SW_WRONG_DATA;
		}
	}
	else
	{
		return SW_WRONG_DATA;
	}

	offset = U8_A_U8(oldElfUpgradeAidLen,offset);
	upgradeRquestTag = APDUBuffer[offset++];

	if (upgradeRquestTag == 0x4F)
	{
		newElfUpgradeAidLen = APDUBuffer[offset++];
		if (AIDLenInvalid(newElfUpgradeAidLen))
		{
			return SW_WRONG_DATA;
		}
	}
//	offset = upgradeRquestOffset +upgradeRquestlength;
//	upgradeSessionDigestTag = APDUBuffer[offset];
//	if(upgradeSessionDigestTag == 0x82){
//		arrayLenth = OFFSET_LC + APDUBuffer[OFFSET_LC] +1;
//		upgradeSessionDigest = &APDUBuffer[upgradeRquestOffset];
//		sha256digest = &APDUBuffer[arrayLenth];
//		offset = offset+2;
//
//		SHA256DoFinalSingleCall(sha256digest, upgradeSessionDigest,(u16)upgradeRquestlength, 1, &APDUBuffer[upgradeRquestOffset]);
//		if(ecpMem_Compare(MEMtPX8(&APDUBuffer[offset]), sha256digest, 0x20) != 0){
//			return SW_DATA_INVALID;
//		}
//	}

	return SW_NO_ERROR;
}
