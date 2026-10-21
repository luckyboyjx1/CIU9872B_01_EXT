/*
 * SCP03.h
 *
 *  Created on: 2013-9-24
 *      Author: jiangxiaohui
 */

#ifndef SCP03_H_
#define SCP03_H_
#include "profile.h"

#if GP__AMENDMENT__D

#define SCP03_I_N_RMAC_ENC		 		0x00
#define SCP03_I_RMAC				 	0x20
#define SCP03_I_RMAC_RENC			 	0x60
#define SCP03_I_PSEURAND				 0x10
#define SCP03_I_PSEURAND_RMAC			 0x30
#define SCP03_I_PSEURAND_RMAC_RENC		 0x70

#define SCP03_L_DERI_LEN64					 (u16)0x0040
#define SCP03_L_DERI_LEN128					 (u16)0x0080
#define SCP03_L_DERI_LEN192					 (u16)0x00C0
#define SCP03_L_DERI_LEN256					 (u16)0x0100

#define SCP03_I1_DERI_COUT					 (u8)0x01
#define SCP03_I2_DERI_COUT					 (u8)0x02

#define SCP03_DERI_CONST_CARD_CRY			 (u8)0x00
#define SCP03_DERI_CONST_HOST_CRY			 (u8)0x01
#define SCP03_DERI_CONST_CARD_RAND			 (u8)0x02
#define SCP03_DERI_CONST_S_ENC				 (u8)0x04
#define SCP03_DERI_CONST_S_MAC   			 (u8)0x06
#define SCP03_DERI_CONST_S_RMAC				 (u8)0x07



#define VALUE_40							0x40
#define VALUE_01							0x01

#define OFFSET_11							0x0B
#define OFFSET_12							0x0C
#define OFFSET_13							0x0D
#define OFFSET_14							0x0E
#define	OFFSET_15							0x0F
#define OFFSET_16							0x10
#define OFFSET_17							0x11
#define OFFSET_18							0x12
#define OFFSET_19							0x13


extern void SCP03_generatePseudoRandom(PN8 keySetObj, PX8 cardChallengeBuffer, memref bCurentAidAdd, u8 bCurentAidLen);
extern void SCP03_generateKey(PN8 keySetObj);
extern void SCP03_generateKeyProc(PN8 staticKey, u8 keyLen ,PX8 sessionKeyAdd, u8 keyType , PX8 derivationData);
extern void SCP03_CalMAC(PX8 srcAddress, u8 len, PX8 keyBuff, u8 keyByteLen);
extern void SCP03_generateCardCryptogram(PX8 firstChallenge, PX8 secondChallenge, PX8 derivationData, u8 bOp);
extern void SCP03_increaseICVCount(void);
//extern void SCP03_decryptCommandData(void);
extern void SCP03_decryptCommandData(PX8 apduBuf);
extern u8 	SCP03_dealRMacData(PX8 SrcAddr, u8 length, u8 option, u8 sw1, u8 sw2);
extern u16 	SCP03_EncryptOfResponse(PX8 SrcAddr ,u16 length);
#endif
extern u32 readScp03Counter(PX8 CounterFieldAddr);

#endif /* SCP03_H_ */
