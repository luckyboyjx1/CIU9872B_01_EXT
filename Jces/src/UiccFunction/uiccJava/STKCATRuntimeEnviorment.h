#ifndef _STKCATRUNTIMEENVIORMENT_H_
#define _STKCATRUNTIMEENVIORMENT_H_

/***********************************function define**********************************************************/
extern BYTE isProactiveHandlerAvailableExist(BYTE buf[20]);

extern void JavaToolkitProcess(BYTE eventbit,BYTE identifier);

extern void ToolkitInitEnvelopResponseHandler(BYTE eventbit);

extern BYTE getApplistByEvent(BYTE eventbit,BYTE identifier,BYTE buff[20]);

extern void SortApplistEvent(BYTE *buff, BYTE num);

extern BYTE SetToolkitContext(BYTE RecordNum,BYTE eventbit);

extern BYTE publicToolkitEventTrigerApplet(BYTE eventbit,BYTE identifier);

extern jbyte Toolkit_transLocalEventTypeToStandardEvent(BYTE localEvent);

extern void HanlderInitBeforeProcessToolkit(BYTE eventBit);

extern void ToolkitSaveAPPContext(void);

extern void ToolkitRestoreAPPContext(void);


extern void JavaToolkitProcessProactiveHandlerEvent(void);

extern BYTE JavaToolkitFindProactiveHandlerApp(BYTE currentAppId);

extern BYTE JavaTookitProcessSamePriority(BYTE * buff, BYTE num, BYTE curAppId);

#endif
