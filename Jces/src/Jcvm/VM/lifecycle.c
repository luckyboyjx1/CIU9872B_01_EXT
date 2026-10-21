/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*+
 * File:  lifecycle.c
 * Version 1.13
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.lifecycle.c
 * Last modified on 03/06/06 at 13:22:54
 * Last modifed by: Eric Nellen
 * Original authors: David Getreu, Alexandre Frey, Nicolas Regnault
 -*/
#include "CgmManager.h"
#include "lifecycle.h"
#include "global_variable.h"
#include "stack.h"
#include "memory.h"
#include "MemBackUp.h"
#include "CardManager.h"
#include "BootLifeCycle.h"
#include "ecpReadWrite.h"
#include "ecpOperation.h"
#include "apdu.h"
#include "nativemethod_impl.h"
#include "gc.h"
#include "euiccConfig.h"
#include "CgmManager.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#if COS_UPDATE_ONLINE_MUTUAL
#include "FunEntryConfig.h"
#endif
/* Address in E2PROM containing lifecycle
 * UNINITIALIZED_MEMORY_CELL: uninitialized
 * != UNINITIALIZED_MEMORY_CELL: initialized
 * modify by jiangxiaohui@20220324 将apdu_FetchComand(APDUBuffer);和apdu_SetCommand(APDUBuffer, APDU_BUFFER_SIZE);从initAPDUBuffer中移动到此函数中
 */
void setCardCreated()
{
    u16		D_A		jcreEntryEnd;

    //by zhangruifeng
	//gpDefaultKeyInit();
	jcreEntryEnd	= MapTable_GetTopIndex();

	JcvmConfig_SetJcreEntryEnd(jcreEntryEnd);
	setGCRequestedFlag(false);
	//nvmWriteU16Atomic(MEMtPN8(NON_JCRE_RTR_SIZE), PN_R_U16(vmConfig_GetRtpBase()));
	 
	/*@ -disspecopt @*/
	nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetCardStateBase()), CARD_OP_READY);
	/*@ +disspecopt @*/
#if GP__CGM__FOR_APPPARACHECK
#if (SE_ONLY || SE_AND_EUICC)
	if(CARD_SE_BE_OR_NOT)//modify by liguiyu@20220111 if (U8_AND_U8(JcvmConfig_GetSeBeOrNot(), 0x01) == 0x01)//EUICC+SE 但是没有创建SE不需要执行该操作
	{
		CgmInit_ValueForISD();
	}
#endif
#endif

#if (EUICC_ONLY || SE_AND_EUICC)
	Euicc_BakJCRE();
#endif

#if COS_UPDATE_ONLINE_MUTUAL
	nvmFillAtomic(MEMtPN8(COSUP_BACKUP_DATA), 0, COSUP_BACKUP_DATA_SIZE);
#endif
	BootLifeCycle_SetLifeCycle(LIFE_CYCLE_CREATE);
	apdu_FetchComand(APDUBuffer); // 这里为什么要增加这一行？原本在initAPDUBuffer中这一行是被注释掉的
    apdu_SetCommand(APDUBuffer, APDU_BUFFER_SIZE);
}

void writeTerminationFlag()
{
	BootLifeCycle_SetLifeCycle(LIFE_CYCLE_MUTE);
}
