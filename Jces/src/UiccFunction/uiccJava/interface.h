#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include "typedef.h"
#define Hige_UpdateRAM_New           0x00            //强更新在RAM 条件下更新
#define Hige_ReadRAM_New             0x01            // 强更新在RAM 条件下 读
#define Hige_SetRAM_New              0x02            // 强更新在RAM 条件下 Set

#define Hige_UpdateWithRAM_New       0x80//强更新 在RAM区域  不在E2 比如6F5B  910180 tag,记录在文件是0x8x形式,81第一个强更新RAM,82第二个.

extern BYTE    xdata G_High_Update_InRAM_new[40];
/************************function define********************************************/
extern void IF_EepromAddressSetting(void);

extern BYTE IF_GPGetRAMAPDUSDId(void);

extern BYTE PostAfterReceiveCommandHeader(void);

extern void UiccOrSimThrowToolkitException(BYTE errorCode, BYTE apiMode);

extern void UiccOrSimViewThrowException(BYTE errorCode, BYTE apiMode);

extern BYTE IF_UpdateWithRAM_New(UWORD fileAddr);
extern BYTE IF_UpdateOrReadWithRAM_New(UWORD fileAddr,WORD offset, BYTE * buf, WORD len,BYTE mode);
extern void InitUpdateInRAMVariable(void);

#endif

