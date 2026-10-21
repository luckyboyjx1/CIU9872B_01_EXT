#ifndef _DataOrientedCommand_H_
#define _DataOrientedCommand_H_

#if UICC__FUNCTION__BERTLVCmd

#include "typedef.h"
extern void MoreDataAvailable(void);

extern  void MoreDataExpected(void);

extern UWORD CreateBERTLVFile(UWORD fileMaxSize,UWORD fileUseSize,WORD fileName);

extern BYTE FindBERTLVTagInFile(UWORD fileAddr,BYTE commpare);

extern WORD  FindBERTLVTagAtOffset(UWORD fileAddr,WORD offset,BYTE mode,BYTE commpare);

extern void DeleteBERTLVTag(UWORD fileAddr);

extern BYTE CreateReplaceBERTLVTag(UWORD fileAddr);

extern void SetData(void);

extern BYTE ReadBERBuf(UWORD fileAddr, WORD offset, BYTE * buf, BYTE len);

extern BYTE WriteBERBuf(UWORD fileAddr, WORD offset, BYTE * buf, BYTE len);

extern BYTE GetTopListFromBERTLVFile(UWORD fileAddr);

extern  void RetrieveData(void);

extern BYTE GetTagLgth(BYTE *tag);

extern BYTE GetAvailableTag(void);

extern BYTE ComputeBERTLVFile(BYTE action);

extern void InitBERTLVTagIndicator(void);

extern void MoveBERTLVTagContent(UWORD fileAddr);

extern BYTE    xdata G_BERTLV_OperateTag[4];     //当前正在进行的操作（Retrieve or Set ）
extern WORD    xdata G_BERTLV_LastOffset[4];     //当前文件中的重传偏移
extern WORD    xdata G_BERTLV_CurrentOffset[4];  //当前文件的偏移
extern UWORD   xdata G_PreviousEfAddr[4];        //上次进行setData时操作的文件
extern WORD    xdata G_BERTLV_CurrentPointer[4];

#endif

#endif
