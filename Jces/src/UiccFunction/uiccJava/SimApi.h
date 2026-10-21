#ifndef _SIMAPI_H_
#define _SIMAPI_H_
#include "typedef.h"

#include "simtoolkitconstant.h"
#include "interface.h"
//#include "transaction.h"
#include "stkhead.h"
//#include "VMConfig.h"
/************************************************************************************************************************/
//以下全局变量供 SIMView使用
extern BYTE   xdata    G_SIMFileSystemMode[MAX_LOGICAL_CHANNELS];
extern BYTE   xdata    G_SIMCurrentRecNum[MAX_LOGICAL_CHANNELS];
extern WORD   xdata    G_SIMCurrentDfName[MAX_LOGICAL_CHANNELS];
extern WORD   xdata    G_SIMCurrentEfName[MAX_LOGICAL_CHANNELS];
extern UWORD  xdata    G_SIMCurrentDfAddr[MAX_LOGICAL_CHANNELS];
extern UWORD  xdata    G_SIMCurrentEfAddr[MAX_LOGICAL_CHANNELS];

extern void Toolkit_InitSIMView(BYTE channelId, BYTE initValue);
extern void  Toolkit_SIMViewCopyFileContext(void);
extern void Toolkit_SIMViewRestoreFileContext(void);
/******************************NativeEditHandler.c*********************************************************************/
extern void Toolkit_clear(void);
extern void Toolkit_appendArray(void);
extern void Toolkit_appendBuf(jbyte handlerIndex,jbyte tag,jref srcBuf,WORD valueOffset, WORD valueLength,BYTE mode, BYTE specifiedValue, BYTE apiMode);
extern void Toolkit_appendTLV_1(void);
extern void Toolkit_appendSimpleTLV(jbyte handlerIndex,BYTE *tmp, BYTE length,BYTE apiMode);
extern void Toolkit_appendTLV_2(void);
extern void Toolkit_appendTLV_3(void);
extern void Toolkit_appendTLV_4(void);
/****************************NativeEnvelopHandler.c**********************************************************************/
extern void Toolkit_getEnvelopeTag(void);
extern void Toolkit_getTPUDLOffset(void);
extern void Toolkit_getUserDataLength(void);
extern void Toolkit_getSecuredDataOffset(void);
extern void Toolkit_getSecuredDataLength(void);
/***************************NativeEnvelopResponseHandler.c*************************************************************/
extern void Toolkit_publicPostFunction(BYTE statusType, BYTE apiMode);
extern void Toolkit_Post(void);
extern void Toolkit_PostAsBERTLV(void);
/***************************NativeMEProfile.c***************************************************************************/
extern void Toolkit_profileCheckByte1(void);
extern void Toolkit_profileCheckPara3(void);
extern void Toolkit_profileCheckShort1(void);
extern void Toolkit_profileGetValue(void);
extern void Toolkit_profileCopy(void);
/*****************************NativePeroactiveHandler.c****************************************************************/
extern void ToolKit_init(void);
extern void ToolKit_initDisplayText(void);
extern void ToolKit_initGetInkey(void);
extern void ToolKit_initGetInput(void);
extern void ToolKit_initCloseChannel(void);
extern void Toolkit_Send(void);
/***********************************NativeProcativeResponseHandler.c****************************************************/
extern void Toolkit_getGeneralResult(void);
extern void Toolkit_getAdditionalInformationLength(void);
extern void Toolkit_copyAdditionalInformation(void);
extern void Toolkit_getItemIdentifier(void);
extern void Toolkit_getTextStringLength(void);
extern void Toolkit_getTextStringCodingScheme(void);
extern void Toolkit_copyTextString(void);
extern void Toolkit_getChannelIdentifier(void);
extern void Toolkit_copyChannelData(void);
/**********************************NativeSimAccess.c*****************************************************************/
extern void Toolkit_SelectFCI(void);
extern void Toolkit_SelectById(void);
extern void Toolkit_Status(void);
extern void Toolkit_ReadBinary(void);
extern void Toolkit_UpdateBinary(void);
extern BYTE ToolkitSearch(BYTE recordNo,BYTE maxRecnum,signed step,BYTE *dataBuff,WORD pattLength);
extern void Toolkit_ReadRecord(void);
extern void Toolkit_UpdateRecord(void);
extern void Toolkit_SeekCommand(void);
extern void Toolkit_increase(void);
extern void Toolkit_Invalidate(void);
extern void Toolkit_Rehabilitate(void);
/**********************************NativeToolkitRefistry.c*************************************************************/
extern BYTE ToolkitIsEventSupported(jbyte event);
extern void ToolkitIsEventAllowed(BYTE event);
extern void Toolkit_setEvent(void);
extern void Toolkit_setEventList(void);
extern void Toolkit_clearEvent(void);
extern void Toolkit_isEventSet(void );
extern void Toolkit_disableMenuEntry(void);
extern void Toolkit_enableMenuEntry(void);
extern void Toolkit_initMenuEntry(void);
extern void Toolkit_changeMenuEntry(void);
extern void Toolkit_allocateTimer(void);
extern void Toolkit_releaseTimer(void);
extern void Toolkit_requestPollInterval(void);
extern void Toolkit_getPollInterval(void);
/**************************************NativeViewHandler.c*************************************************************/
extern void Toolkit_getLength(void);
extern void Toolkit_copy(void);
extern void Toolkit_findTLV(void);
extern void Toolkit_getValueLength(void);
extern void Toolkit_getValueByte(void);
extern void Toolkit_copyValue(void);
extern void Toolkit_compareValue(void);
extern void Toolkit_findAndCopyValue_3(void);
extern void Toolkit_findAndCopyValue_6(void);
extern void Toolkit_findAndCompareValue_3(void);
extern void Toolkit_findAndCompareValue_6(void);
extern void Toolkit_GetHandlerStatus(void);
extern void Toolkit_GetHanlderCapacity(void);

#endif


