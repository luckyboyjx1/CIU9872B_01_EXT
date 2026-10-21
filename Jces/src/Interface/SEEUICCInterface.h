#ifndef SEEUICCINTERFACE_H
#define SEEUICCINTERFACE_H

#include "ecpDefine.h"
#include "typejava.h"

/*****************macro define ********************************************/
typedef struct
{
	PN8 AppTable;
	PN8 bFreeEntryIndex;
	PN8 AppIdFlag;
	PN8 MutiFlag;
	PN8 ExtraditonFlag;
	PN8 pkgTable;
	PN8 pkgStatus;
	PN8 pkgLocationTable;
	PN8 packageContextTable;
	PN8 appletPkgCount;
	PN8 defaultAppIDAddr;
	PN8 CardStateAddr;
}ExecuEnvironment; // 参考ProfileEntry.java modify by xuxiaojun@20220315 从EUICCMain.h中移动到这个文件来，因为如果不支持通信功能的话，放在原来的位置编译不过

#if SE_ONLY
extern ExecuEnvironment const G_CurExecuEnvironment;
#else
extern ExecuEnvironment X_A G_CurExecuEnvironment;
#endif

#if (EUICC_ONLY||SE_AND_EUICC)
extern u8  X_A G_CurExecutingProfileIndex;
extern u8  X_A G_NextExecutingProfileIndex;

extern u8  X_A G_BackCurEnableProfileIndex;  //备份当前激活的profileIndex，在新安装profile过程中使用
extern u8  X_A G_IsResumeISDPInfoMode;       //判断是否要恢复profile的配置区域
#else
#define  G_CurExecutingProfileIndex  0xFF
#endif


/***************macro define****************************************/
#define SE_PACKAGE_MODE          0x01
#define PROFILE_PACKAGE_MODE     0x02
#define ILLEGAL_PACKAGE_MODE     0x00

#define MODE_ADD_TO_PACKAGE      0x00 // 将package id添加进PackageIdInfo中
#define MODE_REMOVE_FROM_PACKAGE 0x01 // 将package id从PackageIdInfo中移除

/***************SEEUICCVariableInit函数支持的模式*******************/
#define MODE_PROFILE_CHANGE      0x00
#define MODE_RESET               0x01
#define MODE_DOWNLOAD_PROFILE    0x02
#define MODE_HEAP_PIECE_CLEANUP  0x03
#define MODE_MEMORY_RESET		 0x04

/***************DeleteProfileAppletInfo函数支持的模式*******************/
#define MODE_DELETE_PROFILE      0x00
#define MODE_DELETE_PROFILE_PE   0x01

#define  INSTALL_CHECK_ISDPTAR     0x00  //检查正在安装的ISDP tar是否被分配过
#define  NORMAL_CHECK_ISDPTAR      0x01  //检查其他tar 是否与ISDP相同

/*************************function define*********************************/
extern void  SEEUICCVariableInit(u8 mode);
extern u16  SEUICC_GetAppletEntryRefIndex(u8 appId);
extern void SEUICC_AddOrRemovePackageId(u8 pkgId, u8 mode);

extern void SEUICC_AddSEAppletList(u8 appId, u8 *aidAddr,u8 bLength);
extern void SEUICC_SynSEInfoForAllProfile(void);

extern void SEUICC_SynSEForSingleProfile(u8 profileIndex, u8 mode);
extern void SEUICC_AddSEAIDListToNewProfile(memref profileAddr);
extern void SEUICC_AssignISDPAppletTable(u8 pkgId);
extern void SEUICC_BackISDPAppletTable(u8 newIndex);
extern void SEUICC_ResumeISDPAppletTable(void);

extern u8 SEUICC_DeleteProfileContent(void);

extern u8 IF_CheckSurviveKey(u8 keyVersion[2]);
///extern u8 SEEUICC_AllowInstallSEApp(u8 curSDID);
extern u8 ExecuEnvironmentInit(u8 ProfileIndex);
extern void SEUICC_CheckISDRECASDAID(void);

extern u8 Cmac(u8 *key, u8 keylen, u8 *M, u16 MLen, u8 * headMsg, u8 headMsgLen, u8 *T);
extern u8 EUICC_GetEID(PX8 outBuf);

#endif

