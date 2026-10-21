/*
 * Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*
 * @(#)memory.c	1.76 03/09/26
 */

/*
 * This file holds the low level memory management routines. With Java Card 2.2
 * the memory management has gone from a flat memory space, that grows, but
 * does not return used memory, even if it is no longer used, to one that can
 * expand and contract based on allocation and garbage collection. To do this,
 * memory is now kept in a table of memory segment entries that can grow as
 * new segments are freed and contract as they are used up. The memory heap
 * looks like a table containing individual entries representing the size and
 * address of each available memory segment. When memory is allocated, an
 * appropriate segment is found and the memory is taken from that memory
 * segment, reducing the size of the available segment by the size of the
 * requested segment. When memory is freed, it is added to any existing
 * contiguous segment or a new available memory segment is created for
 * the returned segment. To reduce the required size of the Heap table,
 * only space for up to the MAX_E2P_TBL_SIZE is allocated. If more space
 * is required, a new structure of the MAX_E2P_TBL_SIZE is then allocated.
 * So far, it seems that it will require quite a bit of use to get into
 * a second heap table. We haven't succeeded in doing it yet, other than
 * for testing purposes.
 */
#include "CgmManager.h"
#include "memory.h"
#include "profile.h"
#include "context.h"
#include "lifecycle.h"
#include "mask.h"
#include "ob_manager.h"
#include "object.h"
#include "global_variable.h"
#include "nativemethod_impl.h"
#include "RamManager.h"
#include "MemOperation.h"
#include "Crypto_common_API.h"
#include "MemBackUp.h"
#include "MemOperationServe.h"
#include "tranManager.h"

void init_RAM(void)
{
	/* Clear all RAM */
	//TODO:RTR has been cleared before YES
	//ecpMem_Set(MEMtPX8(rambase), 0, RAMSize);
	tranMananger_initGv();

	/* Initialize the stack locations to "0xFF" */
	ecpMem_Set(MEMtPX8(&VMStack[0]), 0xff, STACKSIZE);
	clearCryptoSpace();
}

void begin_transaction(u8 isMultiApduProcess)
{
	CgmRun_setConsumeInfo();
	memBeginTransaction(isMultiApduProcess);
}

void commit_transaction(void)
{
	CgmRun_setConsumeInfo();
	tranMananger_commitCache();
	memCommitTransaction();
}

void abort_transaction(u1 isReset)
{
	tranMananger_abortCache();
	memAbortTransaction(isReset);
}


void reset_transaction_buffer(void)
{
	memRefresh();
	abort_transaction(ABORT_ON_RESET);
}

void abortTransactionByJCRE(void)
{
	abort_transaction(ABORT_NOT_ON_RESET);
	memSetInProgress();
}


