#include "string.h"
#include "main.h"
#include "gsmpublic.h"
#include "error.h"
#include "ceph.h"
#include "fileAction.h"
#include "fileSystem.h"
#include "stkhead.h"
#include "interface.h"
#include "resume.h"

#ifdef MULTI_IMSI

#define EVENT_LOCATIONSTATUS                    0x03

#define TLV_LOCATIONSTATUS						0x9B
#define TLV_LOCATIONINFOMRATION					0x93

#define LOCATIONSTATUS_NORMAL                   0x00
#define LOCATIONSTATUS_LIMITED                  0x01
#define LOCATIONSTATUS_NOSERVICE                0x02

#define EF_IMSI                                 0x6F07
#define EF_IMSI2                                0x9F01
#define EF_IMSIROAMING                          0x9F02
#define EF_MCCMNCDEFAULT                        0x9F05

#define FORMAT_MCCMNC                           0x00
#define FORMAT_MCC                              0x01
#define FORMAT_NULL                             0xFF

#define D_SendProvideLocalInfo00                    0xB0
#define D_SendRefresh03                             0xB1

#define LOCALINFOR	    						0x00



extern void ProcessLocationStatusEvent(void);
extern void ProcessProvideLocalInfoResponse(void);
extern void SendRefresh03(void);
extern void SendSetUpEventList(void);
extern void ProcessImsiFplmnScychronze(void);
#endif
