#ifndef _PLATFORM_API_H_
#define  _PLATFORM_API_H_
#include "string.h"
#include "typedef.h"
#include "stack.h"
#include "opFunction.h"

//#define CT_APDU_BUFFER_SIZE   APDU_DATA_SIZE  //256, 注释掉未使用的宏定义 by xuxiaojun@20210208

#define ILLEGAL_KEYVERSION                 0xFF


/*************************************Applet Privileges*************************************************************/
#define PRIVILEGE_FIRSTBYTE_SD                       0x80//privilege first byte


extern void IF_SelectSuccessJavaApp(BYTE channel, BYTE*aidBuf,BYTE aidlen);
extern void IF_SelectADFSucess(BYTE channel);
extern void IF_AppSetChannelContext(BYTE cmdChannel, BYTE channelId, BYTE appId);
extern BYTE GetUICCAppId(void);

#define  PushOperandStackShort(value)   pushShort(value)

#define  PopOperandStackShort(void)  popShort(void)

#define  PushOperandStackInt(value)   pushInt(value)
#define  PopOperandStackInt(void)  popInt(void)

#define  PushOperandStackRef(ref)  pushReference(ref)
#define  PopOperandStackRef(void)  popReference(void)

extern BYTE GetAIDDataByAppID(BYTE appID, BYTE* buf);

extern WORD GetAppletRefByAppId(BYTE appId);

extern void GPRunIntoJavaFunction(BYTE mode);
extern void GPSubCallFunction(UWORD staticMethodOff, BYTE pkgId, WORD context);

extern BYTE GetAssoiatedSDByAppId(BYTE appId);

//extern BYTE GPSSDExistCompleteSCP80Key(u8 sdId,u8 keyversion);



extern void GPLoadInit(void);

extern void GPClearSecureSessionInfo(void/*BYTE sdID*/);

//extern BYTE GetAppIdByAIDFromAIDList(BYTE *aidValue,BYTE aidLen);

#define GPSSDExistCompleteSCP80Key(sdId) GPSSDExistCompleteSecureKey(sdId, SCP_80)

#define GPSSDExistCompleteSCP81Key(sdId) GPSSDExistCompleteSecureKey(sdId, SCP_81)

#define GPGetAppletIDByAppletAID              GetAppIdByAIDFromAIDList

//extern void GetAppPrivileges(BYTE appId, BYTE* pri, BYTE len);

extern BYTE GetAppIdByRef(WORD appletRef);
extern void ClearAllCounter(void);
#define IF_GetRAMResWORDLen()            PXB_R_U16(&bShortBuffer[REMOTE_PROCESS_RES_LEN])

extern UWORD GetAddrAndIndexByIndexAddr(UWORD addr, WORD* indexRef);

extern UWORD GetAddrByIndexAddr(UWORD addr);

extern void CallEseInterfaceFunc(BYTE tag);

//extern WORD PopOperandStackRef(void);

extern BYTE calcSHS(BYTE *publicKey, BYTE publicKeyLen, BYTE keyRef, BYTE *privateKey, BYTE privateKeyLen, BYTE *shsBuf);


extern void GetAppPrivileges(BYTE appId, BYTE* pri, BYTE len);


extern void ToolkitAppLifecycleChange(void);

extern WORD GetRAMProcesser(void);

extern u16 GetSetToolkitInfoRef(void);
#endif
