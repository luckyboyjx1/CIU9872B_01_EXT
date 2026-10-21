#ifndef   __TRAN_MANAGER_H__
#define   __TRAN_MANAGER_H__
#include "target.h"
#include "memory.h"
#include "dtrSpace.h"
#include "profile.h"
#include "tranAllocate.h"
#include "tranDeallocate.h"

#define DTP_FLAG					U8_SHL_U8(1, 1)
#define MAXDTR_FLAG					U8_SHL_U8(1, 2)
#define RTR_FLAG					U8_SHL_U8(1, 3)

typedef struct t_system_cache
{
	memref	DTR_Addr;
	u16		DTP_Ptr;
			
	u16		MaxDTR_Ptr;

	u16		RTR_Ptr;
	u8		Flag;
}X_A transient_cache;

extern PX8				X_A									g_rtrStart;
extern transient_cache	X_A									g_transientCache;

#if JCVM__MEMORY__TRAN_MANAGER
	extern void tranMananger_initInfo(void);
	extern void tranMananger_initGv(void);
	extern void tranMananger_initSERtp(void);
	extern void tranMananger_initProfileRtp(void);
	extern void tranMananger_commitCache(void);
	extern void tranMananger_abortCache(void);
	extern u16  tranMananger_getRtp(void);
	extern u16 tranMananger_getSERtp(void);
	extern u16 tranMananger_getProfileRtp(u8 profileindex);
	extern u16  tranMananger_getRtrBase(void);
	extern void tranMananger_setRtp(u16 newRtp);
	extern u16  tranMananger_getMaxDtp(void);
	extern void tranMananger_setMaxDtp(u16 newMaxDtp);
	extern void tranMananger_setDtpByDtrAddress(PN8 dtrAddrss, u16 newDtp);
	extern u1 	tranMananger_IsTransient(PN8 entry_addr);
	extern void tranMananger_setDtpByCtxId(u8 pckId, u16 newDtp);
	extern u16	tranMananger_getDtpByDtrAddress(PN8 dtrAddrss);
	extern u16  tranMananger_getDtpByCtxId(u8 ctxId);
	extern void tranMananger_setTransientObjectDeleted(u16 context);		SMALL_MODE
	extern void tranMananger_getGlobalBuf(void);
#else
	#define tranMananger_initInfo()
	#define tranMananger_initGv()
	#define tranMananger_commitCache()
	#define tranMananger_abortCache()
	#define tranMananger_getRtp()										0
	#define tranMananger_getRtrBase()									0
	#define tranMananger_setRtp(newRtp)
	#define tranMananger_getMaxDtp()									0
	#define tranMananger_setMaxDtp(newMaxDtp)
	#define tranMananger_setDtpByDtrAddress(dtrAddrss, newDtp)
	#define tranMananger_IsTransient(entry_addr)						0
	#define tranMananger_setDtpByCtxId(pckId, newDtp)
	#define tranMananger_getDtpByDtrAddress(dtrAddrss)					0
	#define tranMananger_getDtpByCtxId(ctxId)							0
	#define tranMananger_setTransientObjectDeleted(context)
	#define tranMananger_getGlobalBuf()
#endif

#endif
