#ifndef    _EXPANDEDMANAGE_H_
#define    _EXPANDEDMANAGE_H_
//#include   "typedef.h"

#define M2M_NoKeepSCP03tChaining  0x11//先来84xx /85xx /再来83010186xx 清空会话信息

//G_FormatAndType变量的值
#define FormatType_NULL           0x00
#define FormatType_Compat         0x01
#define FormatType_ExtendAA       0x0A
#define FormatType_ExtendAE      0x0E

extern BYTE ProcessExtendFun(UWORD fileAddr,WORD commandOffset, WORD cplLen,BYTE mode);
extern BYTE ExtendedRFMExcuteCommand(UWORD fileAddr,WORD commandOffset, WORD cplLen);
extern BYTE ExtendedRFMProcessErrorAction(BYTE isRAM);
extern BYTE  GPRAMRFMExtendResponsePublic(BYTE tag);
extern BYTE IF_ExtendResTagChange(BYTE additionalLen);
extern UWORD GetBerTlvLength(BYTE * buffer,WORD * lenByte);
extern BYTE ExtendResTagADD(WORD additionalLen);
extern WORD ExtendResValueADD(UWORD PorAddr,WORD offset,WORD total);
#endif
