
#ifndef FUNCENTRYCONFIG_H
#define FUNCENTRYCONFIG_H

#include "profile.h"
#include "target.h"

#define FuncTionAddrSize  	0x04
#define FuncTionSum  		0x0B

#define NULL_ARRY_SIZE 		(EEPAGE_SIZE - (FuncTionSum * FuncTionAddrSize * 2))
//#define DownLoad_ENKEY    	(FuncTionSum * FuncTionAddrSize * 2)
#define DownLoad_ENKEY_LEN	0x10
//#define DownLoad_ENKEY    	(0)
//#define DownLoad_BL_KEY   DownLoad_ENKEY                         //(DownLoad_ENKEY + DownLoad_ENKEY_LEN)
////#define PORT_MODE					(DownLoad_BL_KEY + DownLoad_ENKEY_LEN)
//#define EN_MODE			  (DownLoad_BL_KEY+DownLoad_ENKEY_LEN)   //(PORT_MODE + 0x01)

#if BOOTLOADER
#if (BOOTLOADER_PRO || COS_UPDATE_ONLINE_MUTUAL || COS_UPDATE_ONLINE_BY_NativeOS)
#define DOWNLOAD_KEY_BASE   UPCOSConfig_GetUPCOSUPKeyBase() //COSUP_PARA_BASE + UPGRADE_DOWNLOAD_KEY
#define DOWNLOAD_EN_MODE		COSUP_PARA_BASE + UPGRADE_EN_MODE
#else
#define	DOWNLOAD_KEY_BASE sysConfig_GetPatchEnkeyBase() 		
#define DOWNLOAD_EN_MODE  sysConfig_GetBLUpdatEnModeBase()                    
#endif
#endif


#if CONFIG_IN_BUFFER
	extern u8 CONST BootLoaderPara [EEPAGE_SIZE];
#endif


#if BOOTLOADER
//  #define	BL_CONFIG_START							0x00230A00
//	
//	#define    BL_MFkey_CONFIG                    0x0000        //FFFF   0x10   00112233445566778899AABBCCDDEEFF
//	#define    BL_ENCRYPT_KEY_CONFIG              0x0200        //FFFF   0x10   9562BBC6D3809EC0DB70C013B6F822EC
//	#define    ENABLE_ENMODE                      0x0210        //FFFF   0x01   01
#endif

/*---------------以下配置用于在线升级---------------*/
#if  SUPPORT_COS_UPDATE
#if  COS_UPDATE_ONLINE_MUTUAL
extern void Image$$ER_COSUP_BACKUP_DATA$$Base;
#endif 
extern void Image$$ER_BOOTLOADER_PARA$$Base;

#if HAL__PLATFORM__SIMULATOR
#if  COS_UPDATE_ONLINE_MUTUAL
#define COSUP_BACKUP_DATA      			((u32)0x101F3A00) //要与SCT对应上 //((memref)&BackUp_data) //修改成与.sct文件中的ER_COSUP_BACKUP_DATA一致
#endif 
#define COSUP_PARA_BASE                 ((u32)0x101F3B00) //要与SCT对应上//((memref)&UPCosPara)//0x0011AA00L
#else
#if  COS_UPDATE_ONLINE_MUTUAL
#define COSUP_BACKUP_DATA      			(u32)&Image$$ER_COSUP_BACKUP_DATA$$Base         //用于存放COS升级过程中需要保留到E2的一些数据
#endif 
#define COSUP_PARA_BASE             (u32)&Image$$ER_BOOTLOADER_PARA$$Base
#endif 
#if  COS_UPDATE_ONLINE_MUTUAL
#define COSUP_BACKUP_DATA_SIZE   0x200
#endif 
#endif

#if  COS_UPDATE_ONLINE_MUTUAL
#define     UPCOS_KEY_LEN               0x10
#define     COSUM_SSD_CNTR_LEN          0x05

#define     UPGRADE_EN_MODE										0x00
#define     UPCOS_KEY_OFFSET            			(UPGRADE_EN_MODE + 0x01)
#define     COSUM_SSD_CNTR_OFFSET       			(UPCOS_KEY_OFFSET + 0x10)
#define     COSUM_SSD_AIDLEN_OFFSET     			(COSUM_SSD_CNTR_OFFSET + 0x05)
#define     COSUM_SSD_AID_OFFSET        			(COSUM_SSD_AIDLEN_OFFSET + 0x01)
#define     COSUM_SSD_KEYVERSION_OFFSET       (COSUM_SSD_AID_OFFSET + 0x10)
#define     UPCOS_DEVICETYPE_OFFSET           (COSUM_SSD_KEYVERSION_OFFSET + 0x01)
#define     UPCOSA_FUNVERSION_OFFSET          (UPCOS_DEVICETYPE_OFFSET + 0x01)
#define     UPCOSA_INCVERSION_OFFSET          (UPCOSA_FUNVERSION_OFFSET + 0x02)
#define     UPCOSB_FUNVERSION_OFFSET          (UPCOSA_INCVERSION_OFFSET + 0x02)
#define     UPCOSB_INCVERSION_OFFSET          (UPCOSB_FUNVERSION_OFFSET + 0x02)
#define     COSUP_NEXT_ACTION_OFFSET          (UPCOSB_INCVERSION_OFFSET + 0x02)
#define     ACTIVE_COS_OFFSET                 (COSUP_NEXT_ACTION_OFFSET + 0x01)

#else

#if COS_UPDATE_ONLINE_BY_NativeOS||BOOTLOADER
#define		 COSUP_PARA_START_OFFSET         0
#define UPGRADE_FILESYSTEM_ADDR				(COSUP_PARA_START_OFFSET)
#define UPGRADE_PRIVATE_FILESYSTEM_ADDR		(UPGRADE_FILESYSTEM_ADDR + 4)


#define UPGRADE_BIPMODULE_RECEIVE_ADDR		(UPGRADE_PRIVATE_FILESYSTEM_ADDR + 4)
#define UPGRADE_DOWNLOAD_KEY				(UPGRADE_BIPMODULE_RECEIVE_ADDR + 4)
#define UPGRADE_EN_MODE						(UPGRADE_DOWNLOAD_KEY +  16)

#define UPGRADE_COS_Identity				(UPGRADE_EN_MODE + 1)
#define UPGRADE_COS_FUNVERSION				(UPGRADE_COS_Identity + 2)
#define UPGRADE_COS_FUNVERSION_FLAG			(UPGRADE_COS_FUNVERSION + 2) 
#define UPGRADE_CARD_PRODUCT				(UPGRADE_COS_FUNVERSION_FLAG + 1)

#define UPGRADE_COSUM_SSD_AID_LEN			(UPGRADE_CARD_PRODUCT + 2)
#define UPGRADE_COSUM_SSD_AID				(UPGRADE_COSUM_SSD_AID_LEN + 1)
#define UPGRADE_COSUM_SSD_APPID				(UPGRADE_COSUM_SSD_AID + 16)

#define UPGRADE_COSUM_SSD_SCP80KVN			(UPGRADE_COSUM_SSD_APPID + 1)
#define UPGRADE_COSUM_SSD_SCP80KIC_LEN		(UPGRADE_COSUM_SSD_SCP80KVN + 1)
#define UPGRADE_COSUM_SSD_SCP80KIC			(UPGRADE_COSUM_SSD_SCP80KIC_LEN + 1)
#define UPGRADE_COSUM_SSD_SCP80KID_LEN		(UPGRADE_COSUM_SSD_SCP80KIC + 24)
#define UPGRADE_COSUM_SSD_SCP80KID			(UPGRADE_COSUM_SSD_SCP80KID_LEN + 1)
#define UPGRADE_COSUM_SSD_TAR				(UPGRADE_COSUM_SSD_SCP80KID + 24)
#define UPGRADE_COSUM_SSD_MSL				(UPGRADE_COSUM_SSD_TAR + 3)
#define UPGRADE_COSUM_SSD_COUNTR_ADDR		(UPGRADE_COSUM_SSD_MSL + 1)

#define UPGRADE_COSUM_SSD_SCP81KVN			(UPGRADE_COSUM_SSD_COUNTR_ADDR + 4)
#define UPGRADE_COSUM_SSD_PSK_LEN			(UPGRADE_COSUM_SSD_SCP81KVN + 1)
#define UPGRADE_COSUM_SSD_PSK				(UPGRADE_COSUM_SSD_PSK_LEN + 1)
#define UPGRADE_CARD_SEID_LEN				(UPGRADE_COSUM_SSD_PSK + 32)
#define UPGRADE_CARD_SEID    				(UPGRADE_CARD_SEID_LEN + 1)

#define UPGRADE_END_FLAG					(UPGRADE_CARD_SEID + 16)
#endif

#endif

/*-------------------------END------------------------*/
/*-------------------------FUNC DEFINE------------------------*/
#if  COS_UPDATE_ONLINE_MUTUAL
#define     UPCOSConfig_GetUPCOSUPKeyBase()                 (U32_A_U32(COSUP_PARA_BASE, UPCOS_KEY_OFFSET))
#define     UPCOSConfig_GetUPCOSUPKeyValue(dst)             ecpMem_Copy(MEMtPN8(dst), MEMtPN8(UPCOSConfig_GetUPCOSUPKeyBase()), UPCOS_KEY_LEN)
#define     UPCOSConfig_SetUPCOSUPKeyValue(dst,src)         memCopyNoAtomic(MEMtPM8(dst), MEMtPM8(src),UPCOS_KEY_LEN)


#define     UPCOSConfig_GetCOSUMSSDCNTRBase()               (U32_A_U32(COSUP_PARA_BASE, COSUM_SSD_CNTR_OFFSET))
#define     UPCOSConfig_GetCOSUMSSDCNTR(dst)                ecpMem_Copy(dst, (UPCOSConfig_GetCOSUMSSDCNTRBase()), COSUM_SSD_CNTR_LEN)
#define     UPCOSConfig_SetCOSUMSSDCNTR(src)                memCopyNoAtomic(MEMtPM8(UPCOSConfig_GetCOSUMSSDCNTRBase()), MEMtPM8(src),(COSUM_SSD_CNTR_LEN))


#define     UPCOSConfig_GetCOSUMSSDAIDLenBase()             (U32_A_U32(COSUP_PARA_BASE,COSUM_SSD_AIDLEN_OFFSET))
#define     UPCOSConfig_GetCOSUMSSDAIDLen()                 nvmReadU8(MEMtPN8(UPCOSConfig_GetCOSUMSSDAIDLenBase()))
#define     UPCOSConfig_SetCOSUMSSDAIDLen(aidLen)           nvmWriteU8NoAtomic(MEMtPN8(UPCOSConfig_GetCOSUMSSDAIDLenBase()), aidLen)

#define     UPCOSConfig_GetUPCOSUMSSDAIDBase()                 (U32_A_U32(COSUP_PARA_BASE, COSUM_SSD_AID_OFFSET))
#define     UPCOSConfig_GetUPCOSUMSSDAIDValue(dst,len)         ecpMem_Copy(dst, MEMtPN8(UPCOSConfig_GetUPCOSUMSSDAIDBase()), len)
#define     UPCOSConfig_SetUPCOSUMSSDAIDValue(dst,src,len)     memCopyNoAtomic((dst), (src),len)


#define     UPCOSConfig_GetUPCOSUMSSDSubmitKeyVersionBase()           (U32_A_U32(COSUP_PARA_BASE, COSUM_SSD_KEYVERSION_OFFSET))
#define     UPCOSConfig_GetUPCOSUMSSDSubmitKeyVersion()               nvmReadU8(MEMtPN8(UPCOSConfig_GetUPCOSUMSSDSubmitKeyVersionBase()))
#define     UPCOSConfig_SetUPCOSUMSSDSubmitKeyVersion(keyVersion)     nvmWriteU8NoAtomic(MEMtPN8(UPCOSConfig_GetUPCOSUMSSDSubmitKeyVersionBase()), keyVersion)

#define     UPCOSConfig_GetUPCOSDeviceTypeBase()           (U32_A_U32(COSUP_PARA_BASE, UPCOS_DEVICETYPE_OFFSET))
#define     UPCOSConfig_GetUPCOSDeviceType()               nvmReadU8(MEMtPN8(UPCOSConfig_GetUPCOSDeviceTypeBase()))
#define     UPCOSConfig_SetUPCOSDeviceType(Type)           nvmWriteU8NoAtomic(MEMtPN8(UPCOSConfig_GetUPCOSDeviceTypeBase()), Type)

#define     UPCOSConfig_GetUPCOSAFunVersionBase()           (U32_A_U32(COSUP_PARA_BASE, UPCOSA_FUNVERSION_OFFSET))
#define     UPCOSConfig_GetUPCOSAFunVersion()               nvmReadU16(MEMtPN8(UPCOSConfig_GetUPCOSAFunVersionBase()))
#define     UPCOSConfig_SetUPCOSAFunVersion(version)         nvmWriteU16NoAtomic(MEMtPN8(UPCOSConfig_GetUPCOSAFunVersionBase()), version)

#define     UPCOSConfig_GetUPCOSAIncVersionBase()           (U32_A_U32(COSUP_PARA_BASE, UPCOSA_INCVERSION_OFFSET))
#define     UPCOSConfig_GetUPCOSAIncVersion()               nvmReadU16(MEMtPN8(UPCOSConfig_GetUPCOSAIncVersionBase()))
#define     UPCOSConfig_SetUPCOSAIncVersion(version)         nvmWriteU16NoAtomic(MEMtPN8(UPCOSConfig_GetUPCOSAIncVersionBase()), version)

#define     UPCOSConfig_GetUPCOSBFunVersionBase()           (U32_A_U32(COSUP_PARA_BASE, UPCOSB_FUNVERSION_OFFSET))
#define     UPCOSConfig_GetUPCOSBFunVersion()               nvmReadU16(MEMtPN8(UPCOSConfig_GetUPCOSBFunVersionBase()))
#define     UPCOSConfig_SetUPCOSBFunVersion(version)         nvmWriteU16NoAtomic(MEMtPN8(UPCOSConfig_GetUPCOSBFunVersionBase()), version)

#define     UPCOSConfig_GetUPCOSBIncVersionBase()           (U32_A_U32(COSUP_PARA_BASE, UPCOSB_INCVERSION_OFFSET))
#define     UPCOSConfig_GetUPCOSBIncVersion()               nvmReadU16(MEMtPN8(UPCOSConfig_GetUPCOSBIncVersionBase()))
#define     UPCOSConfig_SetUPCOSBIncVersion(version)         nvmWriteU16NoAtomic(MEMtPN8(UPCOSConfig_GetUPCOSBIncVersionBase()), version)

#define     UPCOSConfig_GetUPCOSNextActionBase()           (U32_A_U32(COSUP_PARA_BASE, COSUP_NEXT_ACTION_OFFSET))
#define     UPCOSConfig_GetUPCOSNextAction()               nvmReadU8(MEMtPN8(UPCOSConfig_GetUPCOSNextActionBase()))
#define     UPCOSConfig_SetUPCOSNextAction(action)         nvmWriteU8NoAtomic(MEMtPN8(UPCOSConfig_GetUPCOSNextActionBase()), action)

#define     UPCOSConfig_GetActiveCosBase()                (U32_A_U32(COSUP_PARA_BASE, ACTIVE_COS_OFFSET))
#define     UPCOSConfig_GetActiveCos()                    nvmReadU8(MEMtPN8(UPCOSConfig_GetActiveCosBase()))
#define     UPCOSConfig_SetActiveCos(activeCos)           nvmWriteU8NoAtomic(MEMtPN8(UPCOSConfig_GetActiveCosBase()), activeCos)

#else

#if COS_UPDATE_ONLINE_BY_NativeOS||BOOTLOADER
#define     UPCOSConfig_GetUPCOSFileSystemAddrBase()                (U32_A_U32(COSUP_PARA_BASE, UPGRADE_FILESYSTEM_ADDR))
#define     UPCOSConfig_GetUPCOSFileSystemAddr()             		nvmReadU32(MEMtPN8(UPCOSConfig_GetUPCOSFileSystemAddrBase()))
#define     UPCOSConfig_SetUPCOSFileSystemAddr(value)             nvmWriteU32Atomic(MEMtPN8(UPCOSConfig_GetUPCOSFileSystemAddrBase()),value)


#define     UPCOSConfig_GetUPCOSPrivateFileSystemAddrBase()                (U32_A_U32(COSUP_PARA_BASE, UPGRADE_PRIVATE_FILESYSTEM_ADDR))
#define     UPCOSConfig_GetUPCOSPrivateFileSystemAddr()             		nvmReadU32(MEMtPN8(UPCOSConfig_GetUPCOSPrivateFileSystemAddrBase()))
#define     UPCOSConfig_SetUPCOSPrivateFileSystemAddr(value)             nvmWriteU32Atomic(MEMtPN8(UPCOSConfig_GetUPCOSPrivateFileSystemAddrBase()),value)

#define     UPCOS_KEY_LEN               0x10
#define     COSUM_SSD_CNTR_LEN          0x05

#define     UPCOSConfig_GetUPCOSUPKeyBase()                 (U32_A_U32(COSUP_PARA_BASE, UPGRADE_DOWNLOAD_KEY))
#define     UPCOSConfig_GetUPCOSUPKeyValue(dst)             ecpMem_Copy(MEMtPN8(dst), MEMtPN8(UPCOSConfig_GetUPCOSUPKeyBase()), UPCOS_KEY_LEN)
//#define     UPCOSConfig_SetUPCOSUPKeyValue(dst,src)         memCopyAtomic(MEMtPM8(dst), MEMtPM8(src),UPCOS_KEY_LEN)
#endif


#if COS_UPDATE_ONLINE_BY_NativeOS

#define     UPCOSConfig_GetUPCOSBIPModuleReceiveAddrBase()                (U32_A_U32(COSUP_PARA_BASE, UPGRADE_BIPMODULE_RECEIVE_ADDR))
#define     UPCOSConfig_GetUPCOSBIPModuleReceiveAddr()             		nvmReadU32(MEMtPN8(UPCOSConfig_GetUPCOSBIPModuleReceiveAddrBase()))
#define     UPCOSConfig_SetUPCOSBIPModuleReceiveAddr(value)             nvmWriteU32Atomic(MEMtPN8(UPCOSConfig_GetUPCOSBIPModuleReceiveAddrBase()),value)

#define     UPCOSConfig_GetCOSIdentityBase()                 (U32_A_U32(COSUP_PARA_BASE, UPGRADE_COS_Identity))
#define     UPCOSConfig_GetCOSIdentityValue(dst)             ecpMem_Copy(MEMtPN8(dst), MEMtPN8(UPCOSConfig_GetCOSIdentityBase()), 2)
//#define     UPCOSConfig_SetCOSIdentityValue(src)         	memCopyAtomic(MEMtPM8(UPCOSConfig_GetCOSIdentityBase()), MEMtPM8(src),2)

#define     UPCOSConfig_GetUPCOSFunVersionBase()           (U32_A_U32(COSUP_PARA_BASE, UPGRADE_COS_FUNVERSION))
#define     UPCOSConfig_GetUPCOSFunVersion()               nvmReadU16(MEMtPN8(UPCOSConfig_GetUPCOSFunVersionBase()))
#define     UPCOSConfig_SetUPCOSFunVersion(version)         nvmWriteU16Atomic(MEMtPN8(UPCOSConfig_GetUPCOSFunVersionBase()), version)

#define     UPCOSConfig_GetUPCOSFunVersionFlagBase()             (U32_A_U32(COSUP_PARA_BASE,UPGRADE_COS_FUNVERSION_FLAG))
#define     UPCOSConfig_GetUPCOSFunVersionFlag()                 nvmReadU8(MEMtPN8(UPCOSConfig_GetUPCOSFunVersionFlagBase()))
#define     UPCOSConfig_SetUPCOSFunVersionFlag(flag)           nvmWriteU8Atomic(MEMtPN8(UPCOSConfig_GetUPCOSFunVersionFlagBase()), flag)


#define     UPCOSConfig_GetCardProductBase()               (U32_A_U32(COSUP_PARA_BASE, UPGRADE_CARD_PRODUCT))
#define     UPCOSConfig_GetCardProduct()               		nvmReadU16(MEMtPN8(UPCOSConfig_GetCardProductBase()))

#define     UPCOSConfig_GetCOSUMSSDAIDLenBase()             (U32_A_U32(COSUP_PARA_BASE,UPGRADE_COSUM_SSD_AID_LEN))
#define     UPCOSConfig_GetCOSUMSSDAIDLen()                 nvmReadU8(MEMtPN8(UPCOSConfig_GetCOSUMSSDAIDLenBase()))
//#define     UPCOSConfig_SetCOSUMSSDAIDLen(aidLen)           nvmWriteU8Atomic(MEMtPN8(UPCOSConfig_GetCOSUMSSDAIDLenBase()), aidLen)

#define     UPCOSConfig_GetUPCOSUMSSDAIDBase()                 (U32_A_U32(COSUP_PARA_BASE, UPGRADE_COSUM_SSD_AID))
#define     UPCOSConfig_GetUPCOSUMSSDAIDValue(dst,len)         ecpMem_Copy(dst, MEMtPN8(UPCOSConfig_GetUPCOSUMSSDAIDBase()), len)
//#define     UPCOSConfig_SetUPCOSUMSSDAIDValue(src,len)      memCopyAtomic((UPCOSConfig_GetUPCOSUMSSDAIDBase()), (src),len)

#define     UPCOSConfig_GetUPCOSUMSSDAPPIDBase()              (U32_A_U32(COSUP_PARA_BASE, UPGRADE_COSUM_SSD_APPID))
#define     UPCOSConfig_GetUPCOSUMSSDAPPID()                   nvmReadU8(MEMtPN8(UPCOSConfig_GetUPCOSUMSSDAPPIDBase()))
#define     UPCOSConfig_SetUPCOSUMSSDAPPID(appid)                 nvmWriteU8Atomic(MEMtPN8(UPCOSConfig_GetUPCOSUMSSDAPPIDBase()),appid)

#define     UPCOSConfig_GetUPCOSUMSSDScp80KeyVersionBase()           (U32_A_U32(COSUP_PARA_BASE, UPGRADE_COSUM_SSD_SCP80KVN))
#define     UPCOSConfig_GetUPCOSUMSSDScp80KeyVersion()               nvmReadU8(MEMtPN8(UPCOSConfig_GetUPCOSUMSSDScp80KeyVersionBase()))
//#define     UPCOSConfig_SetUPCOSUMSSDScp80KeyVersion(keyVersion)     nvmWriteU8Atomic(MEMtPN8(UPCOSConfig_GetUPCOSUMSSDScp80KeyVersionBase()), keyVersion)

#define     UPCOSConfig_GetUPCOSUMSSDScp80KicLenBase()                 (U32_A_U32(COSUP_PARA_BASE, UPGRADE_COSUM_SSD_SCP80KIC_LEN))
#define     UPCOSConfig_GetUPCOSUMSSDScp80KicLen()                	nvmReadU8(MEMtPN8(UPCOSConfig_GetUPCOSUMSSDScp80KicLenBase()))

#define     UPCOSConfig_GetUPCOSUMSSDScp80KicBase()                (U32_A_U32(COSUP_PARA_BASE, UPGRADE_COSUM_SSD_SCP80KIC))
#define     UPCOSConfig_GetUPCOSUMSSDScp80Kic(dst,len)             ecpMem_Copy(MEMtPN8(dst), MEMtPN8(UPCOSConfig_GetUPCOSUMSSDScp80KicBase()), len)

#define     UPCOSConfig_GetUPCOSUMSSDScp80KidLenBase()                 (U32_A_U32(COSUP_PARA_BASE, UPGRADE_COSUM_SSD_SCP80KID_LEN))
#define     UPCOSConfig_GetUPCOSUMSSDScp80KidLen()                	nvmReadU8(MEMtPN8(UPCOSConfig_GetUPCOSUMSSDScp80KidLenBase()))

#define     UPCOSConfig_GetUPCOSUMSSDScp80KidBase()                (U32_A_U32(COSUP_PARA_BASE, UPGRADE_COSUM_SSD_SCP80KID))
#define     UPCOSConfig_GetUPCOSUMSSDScp80Kid(dst,len)             ecpMem_Copy(MEMtPN8(dst), MEMtPN8(UPCOSConfig_GetUPCOSUMSSDScp80KidBase()), len)

#define     UPCOSConfig_GetUPCOSUMSSDTarBase()                (U32_A_U32(COSUP_PARA_BASE, UPGRADE_COSUM_SSD_TAR))
#define     UPCOSConfig_GetUPCOSUMSSDTar(dst)             		ecpMem_Copy(MEMtPN8(dst), MEMtPN8(UPCOSConfig_GetUPCOSUMSSDTarBase()), 3)

#define     UPCOSConfig_GetUPCOSUMSSDMSLBase()           		(U32_A_U32(COSUP_PARA_BASE, UPGRADE_COSUM_SSD_MSL))
#define     UPCOSConfig_GetUPCOSUMSSDMSL()               		nvmReadU8(MEMtPN8(UPCOSConfig_GetUPCOSUMSSDMSLBase()))
#define     UPCOSConfig_SetUPCOSUMSSDMSL(msl)     			    nvmWriteU8Atomic(MEMtPN8(UPCOSConfig_GetUPCOSUMSSDMSLBase()), msl)

#define     UPCOSConfig_GetUPCOSUMSSDCounterAddrBase()                (U32_A_U32(COSUP_PARA_BASE, UPGRADE_COSUM_SSD_COUNTR_ADDR))
#define     UPCOSConfig_GetUPCOSUMSSDCounterAddr()             		nvmReadU32(MEMtPN8(UPCOSConfig_GetUPCOSUMSSDCounterAddrBase()))
#define     UPCOSConfig_SetUPCOSUMSSDCounterAddr(value)             nvmWriteU32Atomic(MEMtPN8(UPCOSConfig_GetUPCOSUMSSDCounterAddrBase()),value)

#define     UPCOSConfig_GetUPCOSUMSSDScp81KeyVersionBase()           (U32_A_U32(COSUP_PARA_BASE, UPGRADE_COSUM_SSD_SCP81KVN))
#define     UPCOSConfig_GetUPCOSUMSSDScp81KeyVersion()               nvmReadU8(MEMtPN8(UPCOSConfig_GetUPCOSUMSSDScp81KeyVersionBase()))

#define     UPCOSConfig_GetUPCOSUMSSDPSKLenBase()                 (U32_A_U32(COSUP_PARA_BASE, UPGRADE_COSUM_SSD_PSK_LEN))
#define     UPCOSConfig_GetUPCOSUMSSDPSKLen()                	 nvmReadU8(MEMtPN8(UPCOSConfig_GetUPCOSUMSSDPSKLenBase()))

#define     UPCOSConfig_GetUPCOSUMSSDPSKBase()                (U32_A_U32(COSUP_PARA_BASE, UPGRADE_COSUM_SSD_PSK))
#define     UPCOSConfig_GetUPCOSUMSSDPSK(dst,len)              ecpMem_Copy(MEMtPN8(dst), MEMtPN8(UPCOSConfig_GetUPCOSUMSSDPSKBase()), len)

#define     UPCOSConfig_GetCardSEIDLenBase()                       (U32_A_U32(COSUP_PARA_BASE, UPGRADE_CARD_SEID_LEN))
#define     UPCOSConfig_GetCardSEIDLen()                         nvmReadU8(MEMtPN8(UPCOSConfig_GetCardSEIDLenBase()))
#define     UPCOSConfig_SetCardSEIDLen(len)     				nvmWriteU8Atomic(MEMtPN8(UPCOSConfig_GetCardSEIDLenBase()), len)

#define     UPCOSConfig_GetCardSEIDBase()                       (U32_A_U32(COSUP_PARA_BASE, UPGRADE_CARD_SEID))
#define     UPCOSConfig_GetCardSEID(dst,len)                    ecpMem_Copy(MEMtPN8(dst), MEMtPN8(UPCOSConfig_GetCardSEIDBase()), len)
#define     UPCOSConfig_SetCardSEID(src,len)     				 memCopyAtomic((UPCOSConfig_GetCardSEIDBase()), (src),len)
/*-------------------------END------------------------*/
#define    UPCOS_SCHEME_VERSION              0x0100

#endif

#endif



#if SUPPORT_COS_UPDATE
extern     void valueToBuff(PX8 buff, u32 value, u8 Len);
#if !BOOTLOADER_PRO
extern     u16  UPCOSPrivateCommand(PX8 apduBuffer);
#endif
#endif

#if COS_UPDATE_ONLINE_BY_NativeOS
extern     u16  UPCOS_GetData(void);
#if !BOOTLOADER_PRO
extern     u16  UPCOS_SwitchToLoader(void);
#endif
#endif

#if BOOTLOADER

typedef struct
{
	void (*resetInit)(void);
	void (*appMain)(void);
	void (*Irq1HandlerFunc)(void);
	void (*Irq2HandlerFunc)(void);
	void (*Irq3HandlerFunc)(void);
	void (*Irq4HandlerFunc)(void);
	void (*Irq5HandlerFunc)(void);
	void (*Irq6HandlerFunc)(void);
	void (*Irq7HandlerFunc)(void);
	void (*Irq8HandlerFunc)(void);
	void (*Irq9HandlerFunc)(void);

	void (*resetInit_check)(void);
	void (*appMain_check)(void);
	void (*Irq1HandlerFunc_check)(void);
	void (*Irq2HandlerFunc_check)(void);
	void (*Irq3HandlerFunc_check)(void);
	void (*Irq4HandlerFunc_check)(void);
	void (*Irq5HandlerFunc_check)(void);
	void (*Irq6HandlerFunc_check)(void);
	void (*Irq7HandlerFunc_check)(void);
	void (*Irq8HandlerFunc_check)(void);
	void (*Irq9HandlerFunc_check)(void);

	unsigned char nullarry[NULL_ARRY_SIZE];
} FunctionEntry_t;

typedef struct
{
	void (*resetInit)(void);
	void (*appMain)(void);
	void (*Irq1HandlerFunc)(void);
	void (*Irq2HandlerFunc)(void);
	void (*Irq3HandlerFunc)(void);
	void (*Irq4HandlerFunc)(void);
	void (*Irq5HandlerFunc)(void);
	void (*Irq6HandlerFunc)(void);
	void (*Irq7HandlerFunc)(void);
	void (*Irq8HandlerFunc)(void);
	void (*Irq9HandlerFunc)(void);

} FunctionReadEntry;


extern void IrqEntry1_Process(void);
extern void IrqEntry2_Process(void);
extern void IrqEntry3_Process(void);
extern void IrqEntry4_Process(void);
extern void IrqEntry5_Process(void);
extern void IrqEntry6_Process(void);
extern void IrqEntry7_Process(void);
extern void IrqEntry8_Process(void);
extern void IrqEntry9_Process(void);


extern FunctionEntry_t	CONST	FuncEntry;
extern FunctionEntry_t	CONST	FuncBootEntry;

extern FunctionReadEntry  RealEntry;

//is use for null function
extern void HardFault_Handler(void);
#endif
extern void resetInit_Process(void);
extern void appMain_Process(void);
#endif
