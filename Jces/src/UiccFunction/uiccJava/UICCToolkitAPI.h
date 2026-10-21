#ifndef    _UICCTOOLKITAPI_H
#define	   _UICCTOOLKITAPI_H
#include  "typedef.h" 
#include  "stkhead.h"
//#include   "transaction.h"
//#include 	"JCREMain.h"
//#include 	"main.h"
#include 	"uicc_main.h"
//#include   "ChannelManagement.h"

//***********定义Toolkit下当前文件系统结构******************************************************************
#define    SIMFileSystem        0x80        //认为SIM API 下可以访问所有的DF
#define    ADFFileSystem        0xA0	   //ADFFileSystem :访问ADF下的DF或EF
#define    UICCFileSystem       0xA1	   //仅可访问MF下的DF 和EF
#define    AdminADFFileSystem 	0xA2		//ADF文件系统的管理权限
#define    AdminUICCFileSystem  0xA3       //UICC文件系统的管理权限

extern  BYTE    xdata    G_ToolkitFileContextEvent;

extern  UWORD   xdata    G_BERTLVEditHandlerContext;

#define  FileContextLength 	(3*sizeof(WORD) +2 *sizeof(UWORD)+2*sizeof(BYTE))
/**************************Others Variables Define END***************************************/


/**************************Functions Define Begin*******************************************/

/**************************UICCEnvelopHandler.c*********************************************/
extern  void UICCEnvelope_Toolkit_getItemIdentifier(void);

extern void UICCEnvelope_Toolkit_getChannelIdentifier(void);

extern  void UICCEnvelope_Toolkit_getChannelStatus(void);

extern  void UICCToolkit_getBERTag(void) ;

extern void UICCToolkit_getBERSize(void);

/************************UICCViewHandler.c***********************************************/
extern void UICCToolkit_getValueShort(void);

/*********************************EditHandler.c*************************************************/
extern void UICCToolkitWriteBERTLVHandlerContext(void);

extern void UICCToolkitCopyHandlerContextToNative(void);

extern void UICCToolkitCopyHandlerContextToJava(void);

extern void UICCBERTLVHandlerGetValueLength(void);

extern void UICCBERTLVHandlerGetLength(void);

extern void UICCBERTLVHandlerGetSize(void);

extern  void UICCToolkit_appendTLV_3(void);

extern  void UICCToolkit_appendTLV_5(void);

extern  void UICCToolkit_appendTLV_6(void);

extern  void UICCToolkit_appendTLV_8(void);

/***********************************************ProactiveHandler.c**********************************/
extern void UICCToolKit_initMoreTime(void);

/***********************************************ProactiveResponseHandler.c**********************************/	
extern void UICCProactiveResponse_Toolkit_getChannelStatus(void);	
	
/*********************************ToolkitRegistry.c*************************************************/
extern void UICCToolkit_isEventAllowed(BYTE localEvent,BYTE mode);

extern BYTE UICCToolkit_transferStandardToLocalEventType(jshort standardEvent);

extern BYTE UICCToolkit_isStandardEventAvaible(jshort standardEvent);

extern void UICCToolkit_setEventList(void);

extern void UICCToolkit_allocateServiceIdentifier(void);

extern void UICCToolkit_releaseServiceIdentifier(void);

extern void UICCToolkit_registerFileEvent_2para(void);

extern void UICCToolkit_registerFileEvent_7para(void);

extern void UICCToolkit_deregisterFileEvent_2para(void);

extern void UICCToolkit_deregisterFileEvent_7para(void);

extern void  registerFileEvent(BYTE event, BYTE *filelist);

extern void  deregisterFileEvent(BYTE event, BYTE *filelist);

extern BYTE  isFileEventAvaible(jshort fileEvent);

extern void UICCToolkit_setMenuEntryTextAttribute(void);

/************************************UICCFileView.c**************************************************************/
extern void UICCToolkit_selectPataByteSFI(void);

extern void UICCToolkit_searchRecord(void);

extern void UICCToolkitCopyJavaContextToNativeContext(void);

extern void UICCToolkitCopyNativeContextToJavaContext(void);

extern UWORD ToolkitSelectByFileId(jshort fid);

extern void UICCToolkit_GetADFName(void);

/*********************************UICCAdminFileView.c**************************************************************/
extern void UICCToolkit_createFile(void);

extern void UICCToolkit_deleteFile(void);

extern void UICCToolkit_resizeFile(void);
/***********************************UICCToolkitInterface.c**************************************************************/
extern  BYTE UICCToolkitPollOff(void);

extern BYTE UICCToolkitPollInterval(WORD duration);
/**********************************UICCEnvelopReponseHandler.c*******************************************************/
extern void UICCpost(void) ;

extern void UICCpostAsBERTLV(void) ;

/*******************************************************************/
#endif
