#ifndef    _EUICCINTERFACE_H   
#define	   _EUICCINTERFACE_H 
#include    "ecpDefine.h"

extern u8 CONST euiccInfo_section1[15];
extern u8 CONST euiccInfo_section2[19];
extern u8 CONST euiccInfo_euiccCategory[3];
extern u8 CONST euiccInfo_section3[0x37];
//extern u8 CONST defaultCIPkid[0x14];

extern memref  GPGetCardRemainMemory(u8 mode);

extern void IF_SaveRFMRemoteFileUpdate(u16 position,u16 len);
extern void setProfileInstallAppId(void);
extern void CopySETableToProfile(void);
extern void DeleteProfilePE(u8 deleteProfileIndex);
extern void initialProfileAppFlag(memref profileEntryAddr , memref srcArrayRefAddr);
extern u8 KeyDerivationFunction(u8 * shsBuf, u8 shsBufLen, u8 *shareInfoBuf, u8 shareInfoLen, u8 keyByteLen ,u8* keyCaclBuf,u8 *keyBuf);//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 增加参数shsBufLen

extern u8 SHA256DoFinalSingleCall_Ex(memref inAddr,u16 inLen ,u8 *hashResult);
extern u8 VerifySignForECDSA(u8 * publicKey,u8  publicKeyLen,u8 keyRef, u8  *inBuff, u16  inLen, u8  *sigatureR,u8  *sigatureS,u8  signatureLen ,u8 algType );//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 增加参数algType
extern u8 CalSignValueForECDSA(u8 *privateKey, u8 privateKeyLen, u8 keyRef, u8 *inBuff, u16 inLen, u8 *signOutBuf);
extern u8 CalSignValueForSM2(u8 *privateKey, u8 *inBuff, u16 inLen, u8 *signOutBuf);

extern u8 InitialiseSecureChannel_KeyExchangeForSM2(memref smdpOtpkAddr, u8 *hostId, u8 hostIdLen, u8 exchangeKeyLen, u8 *outBuf);

extern u8 VerifySignForSM2(u8 * publicKey, u8  *inBuff, u16  inLen, u8  *signatureRS);

#define CUSTOMERGP_InstallforInstallISDP           PEExcuteGPCommand

#endif
