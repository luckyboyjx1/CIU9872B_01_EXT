#include "ChipCoreFunction.h"
#include <stdint.h>

//__asm unsigned long disableIRQ(void)
//{
//    MRS     R0, PRIMASK
//    CPSID   I
//    BX      LR
//}
//__asm void restoreIRQ(unsigned long backup)
//{
//    MSR     PRIMASK, R0
//    BX      LR
//}


