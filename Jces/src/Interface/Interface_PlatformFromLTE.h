#ifndef _INTERFACE_PLATFORMFROMLTE_H_
#define _INTERFACE_PLATFORMFROMLTE_H_

#include "ecpDefine.h"

#define OpenChannelId_Offset          0x24
#define ILLEGAL_KEYVERSION                 0xFF
//#define PROFILEDELETE_BACKUPFILESYSTEM          (MAX_LOGICAL_CHANNELS*4+4)  // 分别存每个通道的文件信息DFparent1,DFparent2,DFname,EFname以及toolkit应用的
#define PROFILEDELETE_BACKUPFILESYSTEM          (4  *     4  +  4)  // 分别存每个通道的文件信息DFparent1,DFparent2,DFname,EFname以及toolkit应用的
//																							 |		    |      |
//																							 |		    |      |
//																	MAX_LOGICAL_CHANNELS  4*word  toolkit app DFparent1,DFparent2,DFname,EFname
#define PROFILE_CHANGE					0  // enable and disable profile
#define MEMORY_RESET					1
/************************function define********************************************/
extern u8 IF_SupportContactlessByProfileIndex(void);
extern void IF_GetADFByAID(void);
extern void IF_PEAnayleGlobalVariableInit(void);
extern u8 IF_SendRefreshCommandOfUICCResetMode(void); //modify by chentao@20231123
extern u8 IF_SendRefreshCommandOfeUICCProfileStateChange(void);
extern u8 IF_TerminaprofileSupportEUICCChangeMode(void);
extern u8 IF_CATIsTnitialised(void); // add by chentao@20230209
extern u8 IF_TerminalprofileSupportRefresh(void); // add by chentao@20230209
extern void IF_ChangeProfileLTEInfoNoReset(u8 mode);

extern u8 IF_GetPfoileExcuteState(void);
extern void SelectSuccessJavaApp(void);
extern u8 IF_GPGetRAMAPDUSDId(void);
extern void HandleFirstCommand(u8 mode);
extern void IF_DeleteFilesystemByProfileIndex(u8 profileIndex);
extern u8 IF_GetProfileExcuteErrorReason(void);

extern memref IF_GetFileSystemEndAddr(void);
extern memref IF_GetFileSystemUsedSize(void);
extern u8 IF_GetRAMProfileIndex(void);
extern void IF_M2M_SetRefreshFlag(void);
extern void IF_SetHttpScriptState(void);
extern void IF_M2M_NotificationConfirmation(void);
extern u8 IF_GetScp80Dek(u8* dek,u8 keyLen);

extern u8 IF_GetScp80KeyVersionNum(void);
extern u8 IF_GetScp81KeyVersionNum(void);
extern u8 IF_GetScp81TriggerSD(void);
extern u8 IF_GetScp81TargetSD(void);
extern u8 IF_IsHTTPCommand(void);
extern u8 IF_CurrentTarSDIsMNOSD(void);
extern u8 IF_IsProfileLoadingSession(void);
extern u8 IF_ProfielChangeOngoing(void);
extern u8 IF_ProfileLoadingOrChangeOngoing(void);
extern u8 RSP_IsGetResponseOngoing(void);
extern u8 IF_CapLoaingInCurrnetProfile(void);
extern u8 IF_MoreCommandsInCurrnetProfile(void);
extern void RSP_CloseChannel(u8 managedChannel);
extern void IF_ProfielDeleteinit(u16 fileName[PROFILEDELETE_BACKUPFILESYSTEM]); //PROFILEDELETE_BACKUPFILESYSTEM
extern void IF_ProfielDeletebackFileName(u16 fileName[PROFILEDELETE_BACKUPFILESYSTEM]); //PROFILEDELETE_BACKUPFILESYSTEM
extern void IF_RedTea_StoreDataTagFF7F(void);
extern void IF_RedTea_Fallback(void);
extern u8 IsUiccApp(u8  channelNum);
extern void IF_UpdateSeidFile(u16 tag,u8* seidBuf);
extern void TelecomVarInit(void);

extern u16		IF_GetAvailableMapIndex(u16 i,u16 * para_gNvmMap_AppStartIndex,u16 * para_gNvmMap_TopIndex,u16 * para_gNvmMap_LastFindIndex,	u1 isFind);
extern u8 IF_IsRAMActiveApp(u8 appId);
extern void IF_GetDCEPID(PX8 buff);
#endif
