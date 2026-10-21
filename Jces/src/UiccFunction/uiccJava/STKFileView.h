#ifndef    _STKFILEVIEW_H
#define	   _STKFILEVIEW_H
#include  "typedef.h"

/*****************************************************************************/
extern  BYTE    xdata    G_ToolkitFileSystemMode; 
extern  WORD    xdata    G_ToolkitCurrentAdfName;
extern  WORD    xdata    G_ToolkitCurrentDfName;
extern  UWORD   xdata    G_ToolkitCurrentDfAddr;
extern  WORD    xdata    G_ToolkitCurrentEfName;
extern  UWORD   xdata    G_ToolkitCurrentEfAddr; 
extern  BYTE    xdata    G_ToolkitCurrentRecNum; 

/********************function define******************************************/
extern BYTE Toolkit_ComputeBinary(BYTE action,jshort fileOffset,jshort length, jshort apiMode);
extern BYTE Toolkit_SetChannel(UWORD fileAddr,WORD fileName,BYTE option, jshort apiMode);
extern UWORD ToolkitSelectByFileId(jshort fileName);
extern BYTE Toolkit_GetKeyStatus(UWORD fileAddr,BYTE action,jshort apiMode);
extern BYTE Toolkit_computeRecord(jshort recNumber,jbyte mode,jshort recOffset,jshort respLength,BYTE action, jshort apiMode);
extern BYTE Toolkit_GetAccessDomainInfo(WORD offset, BYTE *domainBuffer, BYTE arrNum,BYTE recNum);

#endif


