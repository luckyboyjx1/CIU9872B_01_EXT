/*
 * NativeAppComponent.h
 *
 *  Created on: 2018Äê6ÔÂ29ÈÕ
 *      Author: jiangxiaohui
 */

#ifndef JCVM_NATIVEAPPAPI_NATIVEAPPCOMPONENT_H_
#define JCVM_NATIVEAPPAPI_NATIVEAPPCOMPONENT_H_

#include "profile.h"

typedef struct
{
	void (*NaAppAPI_Init)(void);
	void (*NaAppAPI_DealBeforThrowExce)(PX16);
	u16 (*NaAppAPI_GetConfig)(u8, PX8);
	u8 (*NaAppAPI_CallPboc)(PX8);
	u8 (*NaAppAPI_CallPbocNative)(u16);
	u8 (*NaAppAPI_IsGetConstructAuthCode)(PX8);
	u8 (*NaAppAPI_IsNativeAppSelected)(void);
	void (*NaAppAPI_DeselectNativeApp)(void);
	u8 (*NaAppAPI_IsSearchEdepAdf)(void);
	u8 (*NaAppAPI_SelectByFileId)(PX8, PX8);
	u16 (*NaAppAPI_ConfigApp)(PX8);
} NativeAppComEntry;

extern void PbocAPI_Init(void);
extern void PbocAPI_DealBeforThrowExce(PX16 errorcode);
extern u16 PbocAPI_GetConfig(u8 configType, PX8 pConf);  
extern u8 PbocAPI_CallPboc(PX8 apduBuf);
extern u8 PbocAPI_CallPbocNative(u16 fun_index);
extern u8 PbocAPI_IsGetConstructAuthCode(PX8 apduBuf);
extern u8 PbocAPI_IsNativeAppSelected(void);
extern void PbocAPI_DeselectNativeApp(void);
extern u8 EdepAPI_IsSearchEdepAdf(void);


#endif /* JCVM_NATIVEAPPAPI_NATIVEAPPCOMPONENT_H_ */
