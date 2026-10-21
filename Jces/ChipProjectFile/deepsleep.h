#ifndef __DEEPSLEEP_H__
#define __DEEPSLEEP_H__

#ifdef __cplusplus
extern "C" {
#endif  
    
#include <stdint.h>
void MHSYS_ConfigUltraLP(void);
void MHSYS_EnterUltraLP(void);
uint32_t MHSYS_GetVersion(void);

#ifdef __cplusplus
}
#endif
#endif
