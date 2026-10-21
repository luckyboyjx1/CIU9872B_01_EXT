#ifndef	 _SuspendUicc_h_
#define  _SuspendUicc_h_
//#include "resume.h"
#include "Uicc_Main.h"


#define CurrentDfAddr_OFF  0x00
#define CurrentEfAddr_OFF  0x04
#define CurrentAdfName_OFF 0x08
#define CurrentDfName_OFF  0x0C
#define CurrentEfName_OFF  0x0E
#define CurrentRecNum_OFF  0x10
#define ChannelStatus_OFF  0x11
#define appSelected_OFF    0x12
#define context_OFF        0x13
#define pinStatus_OFF      0x15

#define OFF_LEN	0x16

extern BYTE xdata G_TerminalCap;
extern BYTE xdata G_SuspendUiccTag;
extern BYTE xdata G_InvildCommandTag;//在RESUME UICC前，是否执行了非规定的指令（除A4,B0,B2,AA）
#endif
