/*****************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: hal_ramInit.c
 Author:
 Version:
 Date:
 Description:
 Function List:
 History:

******************************************************/
#include "hal_ramInit.h"
/*-----------------------------------------------------------------------------
  Function name : HED_RAMInit
  Description:

  Input :

  Output :
  Status:
 -----------------------------------------------------------------------------*/
void hal_RAMInit(uint32_t startAddr, uint32_t len)
{
#define SYSCFGREG4              (*((volatile unsigned long  *)(NVMCTRL_BA + 0x7c) ))
#define SECMCEFLG               (*((volatile unsigned long  *)(SEC_BA + 0x40)))
#define SECHWRSTFLG             (*((volatile unsigned long  *)(SEC_BA + 0x1C)))
    uint32_t ulbkpSFR;
    ulbkpSFR = SYSCFGREG4;
	volatile uint32_t *addr = (volatile uint32_t *)startAddr;
    //disable c4 reset
    SYSCFGREG4 &= (~(1ul << 29));


    for(int i = 0; i <  len/4; i++)
    {
        addr[i] = addr[i];
    }

    //clear c4 error flag
    SECMCEFLG = 0;
    SECHWRSTFLG &= (~(1ul << 29));      //硬件安全置位0
    //enable c4 reset
    SYSCFGREG4 = ulbkpSFR;
}

