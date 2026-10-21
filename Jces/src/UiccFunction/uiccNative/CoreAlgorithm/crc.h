#ifndef    _CRC_H_
#define    _CRC_H_

#include "typedef.h"

extern WORD CalcCrc16(BYTE *ptr, WORD len);
extern UWORD CalcCrc32( BYTE * buf, UWORD size);

#endif
