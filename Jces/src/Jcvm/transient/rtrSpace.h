/*
 * rtrSpace.h
 *
 *  Created on: 2020?6?5?
 *      Author: weipingjiao
 */



#include "ecpDefine.h"
#include "profile.h"

#ifndef SRC_JCVM_TRANSIENT_RTRSPACE_H_
#define SRC_JCVM_TRANSIENT_RTRSPACE_H_

//modify by jiangxiaohui@20220324 ·Ö²ã
#define SE_SPACE_ID 									0
#define SE_RTR_SPACE_CHANNELS							1
#if (EUICC_ONLY||SE_AND_EUICC)
#define PROFILE_ENABLE_SPACE_ID 						1
#define PROFILE_EXECUTION_SPACE_ID 						2


#define ENABLE_PROFILE_RTR_SPACE_CHANNELS				1
#define EXECUTION_PROFILE_RTR_SPACE_CHANNELS			1

#define MAX_RTR_SPACE_CHANNELS					(SE_RTR_SPACE_CHANNELS + ENABLE_PROFILE_RTR_SPACE_CHANNELS + EXECUTION_PROFILE_RTR_SPACE_CHANNELS)
#else
#define MAX_RTR_SPACE_CHANNELS					SE_RTR_SPACE_CHANNELS
#endif


extern void rtrSpace_init(void);

extern memref rtrSpace_getRtrSpaceBase(u8 spaceId);
extern memref rtrSpace_getRtrSpaceEntry(u8 spaceId);
extern u8	RtrSpace_searchExistRtrID(void);
extern u8	RtrSpace_searchProfileRtrID(void);


#endif /* SRC_JCVM_TRANSIENT_RTRSPACE_H_ */
