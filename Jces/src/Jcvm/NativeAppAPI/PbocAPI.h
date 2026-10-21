/*
 * PbocAPI.h
 *
 *  Created on: 2014Äê6ÔÂ6ÈÕ
 *      Author: linguoming1
 */

#ifndef __PBOC_API_H_
#define __PBOC_API_H_

#include "profile.h"


#if VM_WITH_PBOC_OR_EDEP
	#include "ecpPBOC_Constants.h"
	#include "ecpPBOC_Typedefine.h"
	#include "ecpPBOC_GlobalVar.h"
	#include "ecpPBOC_Main.h"

#if VM_WITH_EDEP
	#include "EDEP_globalvariable.h"
#endif	

	extern	void	PbocAPI_Init(void);
	extern	void	PbocAPI_DealBeforThrowExce(PX16 errorcode);
	extern	u16  	PbocAPI_GetConfig(u8 configType, PX8 pConf)    SMALL_MODE;
	extern	u8 		PbocAPI_CallPboc(PX8 apduBuf);
	extern	u8 		PbocAPI_CallPbocNative(u16 fun_index)          SMALL_MODE;
	extern	u8		PbocAPI_IsGetConstructAuthCode(PX8 apduBuf)    SMALL_MODE;
	extern	memref getATCAddr(void);
#else
//	#define	PbocAPI_Init(void)
//	#define PbocAPI_DealBeforThrowExce(errorcode)
//	#define PbocAPI_GetConfig(configType)																0
//	#define PbocAPI_CallPboc(apduBuf)																	false
//	#define PbocAPI_CallPbocNative(fun_index)															false
//	#define PbocAPI_IsGetConstructAuthCode(apduBuf)														false
#endif

#if VM_WITH_EDEP || VM_WITH_PBOC
	extern	u8		PbocAPI_IsNativeAppSelected(void);
	extern	void 	PbocAPI_DeselectNativeApp(void);
	extern	u8		EdepAPI_IsSearchEdepAdf(void);
#else
//	#define PbocAPI_IsNativeAppSelected(void)																false
//	#define PbocAPI_DeselectNativeApp(void)																	false
//	#define EdepAPI_IsSearchEdepAdf(void)																		false
#endif

#endif /* PBOCAPI_H_ */
