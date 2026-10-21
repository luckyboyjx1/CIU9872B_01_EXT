#ifndef __DTR_SPACE_H__
#define __DTR_SPACE_H__

#include "target.h"
#include "profile.h"
#include "DataOperation.h"

#define		INVALID_SPACE_ID											0xFF

#if JCVM__MEMORY__DTR_SPACE
	#define		INVALID_DTR_SPACE_ADDRESS									0xFFFF

	#define		DTR_SPACE_INFO_CTX_OFFSET									0
	#define		DTR_SPACE_INFO_USED_OFFSET									2
	#define		DTR_SPACE_INFO_PROFILE_OFFSET								3
	#define		DTR_SPACE_INFO_ENTRY_SIZE									4
	#define		DTR_SPACE_INFO_TATOL_SIZE									U8_M_U8(DTR_SPACE_INFO_ENTRY_SIZE, MAX_LOGICAL_CHANNELS)

	#define		FIND_APP_SPACE_INFO											0
	#define		FIND_PCK_SPACE_INFO											1
	#define		FIND_NEXT_SPACE_INFO										2

	/************************************************************************************************************************************************
	*************************************************************************************************************************************************/
	extern void 	dtrSpace_init(void);
	extern memref 	dtrSpace_getDtrSpaceBase(u8 spaceId);
	extern u8 		dtrSpace_getDtrSpaceId(u8 channelId);
	extern u8		dtrSpace_searchExistDtrCtx(u16 contextId);
	extern memref 	dtrSpace_getDtrSpaceEntry(u8 spaceId);
	extern memref 	tranManager_getDtrPtrEntry(u8 ctxId);
	extern u8		dtrSpace_assignDtrSpace(u16 contextId, u1 isPckActive);
	extern void 	dtrSpace_setChannelSpaceId(u8 channelId, u8 spaceId);
	extern void		dtrSpace_closeDtrSpace(u8 spaceId, u16 contextId);
#else
	#define dtrSpace_init()
	#define dtrSpace_getDtrSpaceBase(spaceId)								0
	#define dtrSpace_getDtrSpaceId(channelId)								0
	#define dtrSpace_searchExistDtrCtx(contextId)							0
	#define dtrSpace_getDtrSpaceEntry(spaceId)								0
	#define tranManager_getDtrPtrEntry()									0
	#define dtrSpace_assignDtrSpace(contextId, isPckActive)					0
	#define dtrSpace_setChannelSpaceId(channelId, spaceId)
	#define dtrSpace_closeDtrSpace(spaceId, contextId)
#endif

#endif
