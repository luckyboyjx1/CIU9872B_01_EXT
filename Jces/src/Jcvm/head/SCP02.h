/*
 * scp02.h
 *
 *  Created on: 2013-9-24
 *      Author: jiangxiaohui
 */


#ifndef SCP02_H_
#define SCP02_H_
#include "profile.h"

#if GP__SCP__SCP02
	extern void generatePseudoRandomSCP02(PX8 cardChallengeBuffer, memref bCurentAidAdd, u8 bCurentAidLen);
	extern void BeginRmacGenKeyScp02(void);
	extern void generateKeyScp02(PN8 keySetObj);
	extern void CalMACSCP02(PX8 srcAddress, u8 len, PX8 keyBuff, PX8 ICVBuff);
	extern u8 dealSCP0102DataRMac(PX8 SrcAddr, u8 length, u8 option, u8 sw1, u8 sw2);
	extern void generateCardCryptogramSCP0102(PX8 firstChallenge, PX8 secondChallenge, PX8 derivationData);
	extern void decryptCommandDataSCP0102(PX8 apduBuf, u8 dataLen);
#else
	#define generatePseudoRandomSCP02(cardChallengeBuffer, bCurentAidAdd, bCurentAidLen)
	#define BeginRmacGenKeyScp02()
	#define generateKeyScp02(keySetObj)
	#define CalMACSCP02(srcAddress, len, keyBuff, ICVBuff)
	#define dealSCP0102DataRMac(SrcAddr, length, option, sw1, sw2)												0
	#define generateCardCryptogramSCP0102(firstChallenge, secondChallenge, derivationData)
	#define decryptCommandDataSCP0102(apduBuf,dataLen)
#endif  /*  GP__SCP__SCP02 */

#endif /* SCP02_H_ */
