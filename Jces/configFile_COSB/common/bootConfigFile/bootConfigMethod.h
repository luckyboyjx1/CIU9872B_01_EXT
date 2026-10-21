/*
 * bootConfigMethod.h
 *
 *  Created on: 2014-12-3
 *      Author: mafuxuan
 */

#ifndef BOOTCONFIGMETHOD_H_
#define BOOTCONFIGMETHOD_H_

#include "SysConfigProfile.h"
#include "chipHead.h"
#include "bootPlaformConfigProfile.h"
#include "halCryptoProfile.h"
#include "bootConfigProfile.h"
#include "NvmMap.h"

#if RUN_IN_SIMULATOR
#include "boot_callback.h"
#endif

extern memref JcvmConfig_GetSaveTransFlagAreaBase(void);
	
#define			SAVE_TRANS_FLAG_SIZE						(BACK_UP_PAGE_HEADER_SIZE * 2)
#define			SAVE_TRANS_FLAG_AREA						JcvmConfig_GetSaveTransFlagAreaBase()

#if !BOOT__MEMORY__BACK_UP
	#define clearBackUpPageFlag()
	#define abortTransactionByJCRE()
	#define copyBackUpData(des, src, length)
	#define searchBlocks(backUpDes, backUpLength, lastFitOffset, type)				0
	#define save_E2P(e2p_addr, length)

	#define  memBeginTransaction(isMultiApduProcess)
	#define  memAbortTransaction(isReset)
	#define  memCommitTransaction()
	#define  memSetInProgress()
	#define  memInitTransaction()
	#define  memCopyBackUpData(des, src, length)
	#define  getBackupStart()														0
	#define  initMemBackUpGloableVar()
	#define  memSetNoBackAreaPointer(maxEntryAddr, usedHeapAddr)
	#define  isInProgress()															0
	#define  setWriteEepType(typeValue)
	#define  setCacheDataType(typeValue)
	#define  getWriteEepType()														0
	#define  getCacheDataType()														0
	#define  isWriteAndCacheTheSame()												0
	#define  isInSingleApduProgress()												0
	#define  isInMultiApduProgress()												0
	#define  closeProgress()
	#define  openBackUpPageNumStart()
	#define  get_max_commit_capacity()												0
	#define  get_unused_commit_capacity()											0
	#define  getBackUpMaxPage()														0
#endif

#if	!WITH_RSA_KEY_DEAL
	#define  dealPKIData(keyBuffer, keyLen)
#else
	#define  dealPKIData(keyBuffer, keyLen)											dataReversal(keyBuffer, keyLen)
#endif

#if !HAL__IO__TIMER
	#define	wtxTimer_WtxTimerChose(timerChose)
	#define	wtxTimer_WtxTimerInit()
	#define	wtxTimer_WtxTimerStart()
	#define	wtxTimer_WtxTimerSuspend()
	#define	wtxTimer_WtxTimerStop()
	#define	wtxTimer_WtxTimerWakeUp()
	#define wtxTimer_InteruptInit(timeInteruptAddress)
#endif


#if HAL__IO__ISO14443_TA
	#if RUN_IN_SIMULATOR                                                          
		#define	chip_GetSakComplete()									/*ecpCiu_GetSakComplete()*/0	
		#define	chip_GetSakInComplete()									/*ecpCiu_GetSakInComplete()*/0	
		#define	chip_GetAtqLowByte()									/*ecpCiu_GetAtqLowByte()*/0													
		#define	chip_GetAtqHighByte()									/*ecpCiu_GetAtqHighByte()*/0													
		#define	chip_GetCascade1UID(uidBuff, uidLevel)      		    /*ecpCiu_GetCascade1UID(uidBuff, uidLevel)*/						
		#define	chip_GetCascade2UID(c2UID_Buffer)                   	/*ecpCiu_GetCascade2UID(c2UID_Buffer)*/                                     
		#define	chip_GetCascade3UID(c3UID_Buffer)                   	/*ecpCiu_GetCascade3UID(c3UID_Buffer)*/                                             
	#else	
		#define	chip_GetSakComplete()									1//sysConfig_GetL3CONFIGSAK()//must implement	
		#define	chip_GetSakInComplete()									1//sysConfig_GetL3CONFIGINSAK() //must implement	
		#define	chip_GetAtqLowByte()									1//sysConfig_GetL3CONFIGATQAL() //must implement													
		#define	chip_GetAtqHighByte()									1//sysConfig_GetL3CONFIGATQAH()//must implement														
		#define	chip_GetCascade1UID(uidBuff, uidLevel)      		    1//GetChipUID( uidBuff, uidLevel)//must implement						
		#define	chip_GetCascade2UID(c2UID_Buffer)						//must implement															
		#define	chip_GetCascade3UID(c3UID_Buffer)						//must implement		
	#endif
#endif //HAL__IO__ISO14443_TA

#if HAL__PATCH
	#define	chip_getPatchFlagStartBase(patchAreaPtr)				ecpPatch_getPatchFlagStartBase(patchAreaPtr) 	/*patchAreaPtr*/
	#define	chip_getPatchFlagInitBase(patchAreaPtr)					ecpPatch_getPatchFlagInitBase(patchAreaPtr) 	/*patchAreaPtr*/
	#define	chip_getPatchEntryTable(patchAreaPtr)					ecpPatch_getPatchEntryTable(patchAreaPtr) 		/*patchAreaPtr*/
	#define	chip_getPatchLenthTable(patchAreaPtr)					ecpPatch_getPatchLenthTable(patchAreaPtr) 		/*patchAreaPtr*/
	#define	chip_getPatchTableBase(patchAreaPtr)					ecpPatch_getPatchTableBase(patchAreaPtr) 		/*patchAreaPtr*/
	#define chip_getPatchServiceBase(patchAreaPtr)					ecpPatch_getPatchServiceBase(patchAreaPtr) 		/*patchAreaPtr*/

	#define	chip_getPatchSizeBase(patchAreaPtr)						ecpPatch_getPatchSizeBase(patchAreaPtr) 		/*patchAreaPtr*/
	#define chip_getPatchEntrySelPtr(patchAreaPtr)					ecpPatch_getPatchEntrySelPtr(patchAreaPtr) 		/*patchAreaPtr*/
	#define	chip_getPatchConfigPtr(patchAreaPtr)					ecpPatch_getPatchConfigPtr(patchAreaPtr) 		/*patchAreaPtr*/
	#define chip_getPatchTable(patchAreaPtr)                       	ecpPatch_getPatchEntryTable(patchAreaPtr) 		/*patchAreaPtr*/
	#define chip_getPatchLen(patchAreaPtr)                         	ecpPatch_getPatchLenthTable(patchAreaPtr)) 		/*patchAreaPtr*/
	#define chip_getPatchDynamicArea(patchAreaPtr)             		ecpPatch_getPatchDynamicArea(patchAreaPtr)  	/*patchAreaPtr*/
#endif  //HAL__PATCH

#if !WITH_SOFT_SM2													
	#define SM2_ECP_GenKeyPair(privateKey, tempPublicKey)				//chipSM2GenKeyPair(privateKey, tempPublicKey)
	#define SM2_ECP_GetKey(prikey, publickey, isgenkeypair)			//	chipSM2GetKey(prikey, publickey, isgenkeypair)	
	#define SM2_ECP_Sign(E_Buff, privteKeyBuffer, rs)					//chipSM2Sign(E_Buff, privteKeyBuffer, rs)
	#define SM2_ECP_Verify(E, publicKey, rs)						//	chipSM2Verify(E, publicKey, rs)
	#define SM2_ECP_Encrypt(inBuff, inLen, publickeyBuff, outBuff)		//chipSM2Encrypt(inBuff, inLen, publickeyBuff, outBuff)
	#define SM2_ECP_Decrypt(inBuff, inLen, privatekeybuff, outBuff)		//chipSM2Decrypt(inBuff, inLen, privatekeybuff, outBuff)
#endif // WITH_SOFT_SM2	


extern u8	HardWareCheckBackUpFlag(PX8	tempbackUpTempBuf, PN8	tempAddress, u8 i,u1 isReset,PX8 tempRealWriteBuf);
extern u8	HardWareCheckCompFlag(PX8	tempbackUpTempBuf, PN8	tempAddress, u8 i,u1 isReset) ;
extern void RealWriteBakupPage(PN8	tempAddress,PX8 tempRealWriteBuf);
extern u16 HardWareCheckBackUpBlock(u16 tempOffset);
extern void platformfillEeprom(PN8 dest, u8 value, u32 length);

 
#if (!MAP_ADDRESS_NEED_DEAL&&!RUN_IN_SIMULATOR)
#define dealMAPAddress(mapEntry, index)	PN_R_U24(MEMtPN8(mapEntry))

#else
extern memref dealMAPAddress(MapEntry_t  mapEntry, u16 index);
#endif
//extern void rtnBackup(void);

#endif /* BOOTCONFIGMETHOD_H_ */
