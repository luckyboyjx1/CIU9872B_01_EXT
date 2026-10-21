#ifndef _INTERFACE_PLATFORMFROMLTE_H_
#define _INTERFACE_PLATFORMFROMLTE_H_

#include "typedef.h"

#define OpenChannelId_Offset          0x24

/************************function define********************************************/
extern BYTE IF_SupportContactlessByProfileIndex(void);
extern void IF_GetADFByAID(void);
extern void IF_PEAnayleGlobalVariableInit(void);
extern void IF_SendRefreshCommandOfUICCResetMode(void);
extern BYTE IF_SendRefreshCommandOfeUICCProfileStateChange(void);
extern BYTE IF_TerminaprofileSupportEUICCChangeMode(void);
extern void IF_ChangeProfileLTEInfoNoReset(void);

extern BYTE IF_GetPfoileExcuteState(void);
extern void SelectSuccessJavaApp(void);
extern BYTE IF_GPGetRAMAPDUSDId(void);
extern void HandleFirstCommand(BYTE mode);
extern void IF_DeleteFilesystemByProfileIndex(BYTE profileIndex);
extern BYTE IF_GetProfileExcuteErrorReason(void);

extern UWORD IF_GetFileSystemEndAddr(void);
extern UWORD IF_GetFileSystemUsedSize(void);
extern BYTE IF_GetRAMProfileIndex(void);
extern void IF_M2M_SetRefreshFlag(void);
extern void IF_SetHttpScriptState(void);
extern void IF_M2M_NotificationConfirmation(void);
extern BYTE IF_GetScp80Dek(BYTE* dek,BYTE keyLen);

extern BYTE IF_GetScp80KeyVersionNum(void);
extern BYTE IF_GetScp81KeyVersionNum(void);
extern BYTE IF_GetScp81TriggerSD(void);
extern BYTE IF_GetScp81TargetSD(void);
extern BYTE IF_IsHTTPCommand(void);
extern BYTE IF_CurrentTarSDIsMNOSD(void);
extern BYTE IF_IsProfileLoadingSession(void);
extern BYTE IF_ProfielChangeOngoing(void);
extern void IF_ProfielDeleteinit(WORD fileName[4]);
extern void IF_ProfielDeletebackFileName(WORD fileName[4]);
extern void IF_RedTea_StoreDataTagFF7F(void);
extern void IF_RedTea_Fallback(void);
#endif
