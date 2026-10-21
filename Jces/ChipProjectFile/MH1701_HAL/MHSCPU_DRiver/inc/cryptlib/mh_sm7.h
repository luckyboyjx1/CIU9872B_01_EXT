#ifndef _MH_SM7_H_
#define _MH_SM7_H_

#include "mh_crypt.h"

#define MH_SM7_OPT_MODE_Mask        (BIT(0))         
#define MH_SM7_OPT_MODE_ENCRYPT     (0)
#define MH_SM7_OPT_MODE_DECRYPT     (BIT(0))          
#define MH_SM7_OPT_BLK_Mask         (BIT(1)|BIT(2))
#define MH_SM7_OPT_BLK_CBC          (BIT(1))
#define MH_SM7_OPT_BLK_ECB          (BIT(2))

#define MH_RET_SM7_SUCCESS                      (0x5C347AAA)
#define MH_RET_SM7_INIT                         (MH_RET_SM7_SUCCESS - 0x100)
#define MH_RET_SM7_PARAM_ERR                    (MH_RET_SM7_INIT + 1)
#define MH_RET_SM7_ATTACKED                     (MH_RET_SM7_INIT + 2)
#define MH_RET_SM7_PACK_MODE_ERROR              (MH_RET_SM7_INIT + 3)
#define MH_RET_SM7_OUTBUF_TOO_SMALL             (MH_RET_SM7_INIT + 4)
#define MH_RET_SM7_INPUT_SIZE_ERROR             (MH_RET_SM7_INIT + 5)
#define MH_RET_SM7_MODE_ERROR                   (MH_RET_SM7_INIT + 6)



uint32_t MHSM7_EncDec(MH_SYM_CRYPT_CALL *pCall);

uint32_t MHSM7_GetVersion(void);

#endif // _MH_SM7_H_
