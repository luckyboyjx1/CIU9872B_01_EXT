/*
 * SCPGenKeyInterface.h
 *
 *  Created on: 2013-9-24
 *      Author: jiangxiaohui
 */

#ifndef SCPGENKEYINTERFACE_H_
#define SCPGENKEYINTERFACE_H_

#include "profile.h"
#include "opFunction.h" // BASE_KEY在该头文件中定义，此处不包含的话，会导致函数generateStaticKey、generateStaticKeyScp03函数未声明

#if GP__SCP
	extern void generateSCPKey(PN8 keySetObj);
	extern void generateCMACKey(PX8 derivationData, PN8 keySetObj);
	extern void generateRMACKey(PX8 derivationData, PN8 keySetObj);
	extern void generateCENCKey(PX8 derivationData, PN8 keySetObj);
	extern void generateRENCKey(PX8 derivationData, PN8 keySetObj);
	extern void generateCDECKey(PX8 derivationData, PN8 keySetObj);
	extern void generateRDECKey(PX8 derivationData, PN8 keySetObj);
#else
	#define generateSCPKey(keySetObj)
	#define generateCMACKey(derivationData, keySetObj)
	#define generateRMACKey(derivationData, keySetObj)
	#define generateCENCKey(derivationData, keySetObj)
	#define generateRENCKey(derivationData, keySetObj)
	#define generateCDECKey(derivationData, keySetObj)
	#define generateRDECKey(derivationData, keySetObj)
#endif

#if (GP__SCP && BASE_KEY)
	extern void generateStaticKey(PM8 baseKey, PM8 staticKey, PM8 derivationData);
	extern void generateStaticKeyScp03(u8 * baseKey, u8 * staticKey, u8  * derivationData);
#else
	#define generateStaticKey(baseKey, staticKey, derivationData)
	#define generateStaticKeyScp03(baseKey, staticKey, derivationData)
#endif

#endif /* SCPGENKEYINTERFACE_H_ */
