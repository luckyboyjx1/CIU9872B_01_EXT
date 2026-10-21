#ifndef    _UICCREMOTEAPPMANAGEMENT_H
#define	   _UICCREMOTEAPPMANAGEMENT_H
#include  "typedef.h"

/*putkey相关宏定义*/
#define		KEYITEM_LENGTH 	0x1A               //keyType 1 + keyLength 1 + keyValue 24
#define		DeleteKeyVersionTag	    0xD2  //参见GP2.2.1 deletekey
#define		DeleteKeyIdTag	    	0xD0  //参见GP2.2.1 deletekey

#define		KEYVERSION_SCP80MAX 	0x0F

#define		PUTKEY_ADD 	    0x00
#define		PUTKEY_REPLACE 	0x01

#define		KEYINDEX_KIC 	    0x01
#define		KEYINDEX_KID 	    0x02
#define		KEYINDEX_KIK 	    0x03

#define		KEYTYPE_DES3DES 	0x80
#define		KEYTYPE_3DES 	    0x81
#define		KEYTYPE_3DESCBC 	0x82
#define		KEYTYPE_DESECB 	    0x83
#define		KEYTYPE_DESCBC 	    0x84

//#define		ECB_Encrypt 	0x71
//#define		ECB_Decrypt 	0x7E
//#define		CBC_Encrypt 	0x81
//#define		CBC_Decrypt 	0x8E

/***************************Function Define Begin********************************************************/

extern BYTE  UICCNativeImpl_RAMIsValidForTARs(BYTE *tarValue, BYTE tarLgth);

extern BYTE UICCNativeImpl_ProcessRAMAccessDomain(BYTE *buf, BYTE bufLgth, BYTE mode);

extern WORD NativeImpl_InstallForInstall(BYTE * buffer,WORD context, BYTE sdMode);

extern void NativeImpl_EndOfRegister(void);

WORD NativeImpl_InstallForMakeSelectable(BYTE appId);

extern WORD NativeImpl_Delete(BYTE appId);

//extern WORD UICCNativeImpl_GetStatus(BYTE *currentLen);

extern WORD ToolkitRAM_GetData(void);

extern BYTE ToolkitFindSDTars(BYTE *tar);

extern WORD InstallforInstall_RAMSD(BYTE *paraFiled, BYTE paraLen,BYTE tagFlag,BYTE appId);

extern WORD GetLastInstallSequence(void);

extern UWORD FindMenuInfoAddrForSD(BYTE appId,BYTE* recNo);

extern void InstallforInstall_StorSDMenuPara(BYTE *paraFiled, BYTE paraLen,WORD context);

extern void GetStatusSDMenuInfo(void);

extern BYTE ToolKitGetAppMenuStatusByRecNo(BYTE zeroMenuIdNum,  BYTE toolkitAppRecNo);

extern void GetStatusToolkitAppMenuInfo(void);

extern WORD NativeImplSetStatus(BYTE appId);

extern WORD SaveSdRAMTar(BYTE *paraFiled, BYTE paraLen,BYTE appId);

extern void SaveSDTarTemp(BYTE *paraFiled);
/***************************Function Define End**********************************************************/

#endif
