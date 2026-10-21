/*
 * SCPParamCheck.h
 *
 *  Created on: 2013-9-24
 *      Author: jiangxiaohui
 */

#ifndef SCPPARAMCHECK_H_
#define SCPPARAMCHECK_H_
#include "typejava.h"

#if GP__SCP
	extern void checkInitializeUpdate_param(u8 p1, u8 p2, u8 lc);
	extern void checkExternalAuthentication_param(u8 p1, u8 p2, u8 lc);
	extern void checkBeginRmacSession_param(u8 p1, u8 p2, u8 lc);
	extern void checkEndRmacSession_param(u8 p1, u8 p2, u8 lc);
	extern void checkManageSecurityEnvironment_param(u8 p1, u8 p2, u8 lc);
	extern void check_p1p2Lc_param(u8 ins);
	extern u16 	check_scp_i_param(u8 checkTpye);
	extern u16 	check_security_level(void);
	extern u8 	checkSecureLevel(PX8 apduBuf, u8 bSecureLevel, PX8 needDecryptCommandData, PX8 needVerifyMAC );
	extern u16 checkIsSDAcitveOnOtherLogicChannel(void);//modify by xuxiaojun@20221025: SCP11的多通道问题修改，增加GP__SCP__SCP11__CERT_IN_GLOBAL_RAM开关，1表示SCP11C的建立会话过程中临时证书存放在全局变量中；0表示临时证书存放在SSD的DTR中。修改之后，代码可以合并到CIU9872B_01中
	extern u8 	checkSDIsLocked(memref theAppletTableEntry, u8 bOp);
	extern u16 	checkSequenceCount(PN8 keySetObj);
	extern u16 checkKeyUsageQualifier(PN8 keySetObj);
	extern void checkTryAuth(PN8 keySetObj);
	extern void checkIsSDAuthOnOtherLogicChannel(void);
#else
	#define checkInitializeUpdate_param(p1, p2, lc)
	#define checkExternalAuthentication_param(p1, p2, lc)
	#define checkBeginRmacSession_param(p1, p2, lc)
	#define checkEndRmacSession_param(p1, p2, lc)
	#define checkManageSecurityEnvironment_param(p1, p2, lc)
	#define check_p1p2Lc_param(ins)
	#define check_scp_i_param(checkTpye)														0
	#define check_security_level()																0
	#define checkSecureLevel(apduBuf,bSecureLevel, needDecryptCommandData, needVerifyMAC)				0
	#define checkIsSDAcitveOnOtherLogicChannel()
	#define checkSDIsLocked(theAppletTableEntry, bOp)											0
	#define checkSequenceCount(keySetObj)														0
	#define checkTryAuth(keySetObj)
	#define checkIsSDAuthOnOtherLogicChannel()
#endif

#endif /* SCPPARAMCHECK_H_ */
