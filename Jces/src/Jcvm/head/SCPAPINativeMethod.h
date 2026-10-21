/*
 * SCPAPINativeMethod.h
 *
 *  Created on: 2013-9-27
 *      Author: jiangxiaohui
 */

#ifndef SCPAPINATIVEMETHOD_H_
#define SCPAPINATIVEMETHOD_H_

#include "typejava.h"
#include "profile.h"

#define ORG_GLOBALPLATFORM_PROCESSSECURITY   				0x10
#define ORG_GLOBALPLATFORM_WRAP   							0x11
#define ORG_GLOBALPLATFORM_UNWRAP   						0x12
#define ORG_GLOBALPLATFORM_DECRYPTDATA	  					0x13
#define ORG_GLOBALPLATFORM_ENCRYPTDATA	 					0x14
#define ORG_GLOBALPLATFORM_RESETSECURITY  					0x15
#define ORG_GLOBALPLATFORM_GETSECURITYLEVEL	   				0x16
#define ORG_GLOBALPLATFORM_SETSECURITYLEVEL	   				0x17

#define VISA_OPENPLATFORM_OPSNSECURECHANNEL 				0x20
#define VISA_OPENPLATFORM_CLOSESECURECHANNEL 				0x21
#define VISA_OPENPLATFORM_DECRYPTVERIFYKEY   				0x22
#define VISA_OPENPLATFORM_UNWRAP			 				0x23
#define VISA_OPENPLATFORM_VERIFYEXTERNALAUTHENTICATE		0x24

#define VISA_OPENPLATFORM_VERIFYDAP 						0x30
#define VISA_OPENPLATFORM_DECIPHEREDLOADBLOCKDATA   		0x31

#if GP__SCP__API__FOR_MASK
	extern void SCPAPI_initGlobalScpAPI(void);
	extern u8 	getGlobalScpAPI(void);
	extern u16 	SCPAPI_implementMethod(u8 SCPAPITpye, PX8 SrcAddr, u16 sLength, u8 channel, u8 bSecurityLevel, jref bKey_ref);
	extern u16 	SCPAPI_processSecurityOrgApi(PX8 SrcAddr);
	extern u16 	SCPAPI_wrapOrgApi(PX8 SrcAddr, u16 sLength);
	extern u16 	SCPAPI_unwrapOrgApi(PX8 SrcAddr, u16 sLength);
	extern u16 	SCPAPI_decryptDataOrgApi(PX8 SrcAddr, u16 sLength, jref bKey_ref);
	extern u16 	SCPAPI_encryptDataOrgApi(PX8 SrcAddr, u16 sLength, jref bKey_ref);
	extern void SCPAPI_resetSecurityOrgApi(void);
	extern u8 	SCPAPI_getSecurityLevelOrgApi(void);
	extern void SCPAPI_setSecurityLevelOrgApi(u8 bSecurityLevel);
	extern void SCPAPI_closeSecureChannelVisaApi(u8 bchannel);
	extern boolean SCPAPI_decryptVerifyKeyVisaApi(u8 bchannel, PX8 SrcAddr);
	extern u8	SCPAPI_openSecureChannelVisaApi(PX8 SrcAddr);
	extern void SCPAPI_unwrapVisaApi(u8 bchannel, PX8 SrcAddr);
	extern void SCPAPI_verifyExternalAuthenticateVisaApi(u8 bchannel, PX8 SrcAddr);
#else
	#define SCPAPI_initGlobalScpAPI()
	#define getGlobalScpAPI()																				0
	#define SCPAPI_implementMethod(SCPAPITpye, SrcAddr, sLength, channel, bSecurityLevel, bKey_ref)			0
	#define SCPAPI_processSecurityOrgApi(SrcAddr)															0
	#define SCPAPI_wrapOrgApi(SrcAddr, sLength)																0
	#define SCPAPI_unwrapOrgApi(SrcAddr, sLength)															0
	#define SCPAPI_decryptDataOrgApi(SrcAddr, sLength, bKey_ref)											0
	#define SCPAPI_encryptDataOrgApi(SrcAddr, sLength, bKey_ref)											0
	#define SCPAPI_resetSecurityOrgApi()
	#define SCPAPI_getSecurityLevelOrgApi()																	0
	#define SCPAPI_setSecurityLevelOrgApi(bSecurityLevel)
	#define SCPAPI_closeSecureChannelVisaApi(bchannel)
	#define SCPAPI_decryptVerifyKeyVisaApi(bchannel, SrcAddr)												0
	#define SCPAPI_openSecureChannelVisaApi(SrcAddr)														0
	#define SCPAPI_unwrapVisaApi(bchannel, SrcAddr)
	#define SCPAPI_verifyExternalAuthenticateVisaApi(bchannel, SrcAddr)
#endif



#endif /* SCPAPINATIVEMETHOD_H_ */
