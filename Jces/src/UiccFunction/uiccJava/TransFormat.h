#ifndef    _TransFormat_h_
#define    _TransFormat_h_
#include	"uicc_main.h"
#include	"typedef.h"

extern BYTE IntToChar(BYTE i, BYTE mode);
extern BYTE CharToInt(BYTE i, BYTE mode);
extern BYTE AsciiToADN(BYTE * input, BYTE * output);
extern BYTE IntToString(BYTE * lv_src, BYTE * lv_dst);
extern BYTE StringToInt(BYTE * src, BYTE * dst);
extern void AsciiToUCS2(BYTE * dataBuffer,BYTE * resultBuffer);
extern BYTE SixteenToTen(WORD src, BYTE * buf);
extern BYTE HexToUCS2(BYTE src, BYTE * lv_dst);
extern BYTE ADNToAscii(BYTE * input, BYTE * output);

#endif
