/*
 * gwAPI.h
 *
 *  Created on: 2020年10月19日
 *      Author: jiangxiaohui
 */

#ifndef SRC_GW_GWAPI_H_
#define SRC_GW_GWAPI_H_

#include "profile.h"

#if UICC_FUCTION_PRIVATEAUTH
#define GW_SEC_DATA_ADDRESS		0x0C0ffE00
#define GW_SEC_DATA_MAX_SIZE	0x200

#define SETGWSECDATA_MODE 0x01
#define GETGWSECDATA_MODE 0x02

#define NonnormalSM4								0x00
#define NormalSM4									0x01

#define   SYM_GUOMI_PROPERTY_GW						0x00
#define   SYM_GUOMI_PROPERTY_STANDAND				0x01

//extern u8 		g_SM4NormalFlag; //是否是标准的SM4，0表示非标准SM4，1表示标准SM4
extern u8		g_SymGuomiPro;

#define SetSymGuomiProperty(v)						(g_SymGuomiPro = v)



extern void Pirvate_GetGWSecData(u16 srcOffset, PM8 desAddr, u16 desOffset, u8 length);
extern void Pirvate_SetSWSecData(u16 srcOffset, PM8 desAddr, u16 desOffset, u8 length);
extern void SetOrGetGWSecDataNative(void);

#endif
#endif /* SRC_GW_GWAPI_H_ */
