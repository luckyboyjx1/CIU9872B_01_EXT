/*
 * JavaDebug.h
 *
 *  Created on: 2015Äê3ÔÂ6ÈÕ
 *      Author: mowenpan
 */

#ifndef SRC_HW_IO_JAVADEBUG_H_
#define SRC_HW_IO_JAVADEBUG_H_
#include "Packet.h"

#define JAVA_DEBUG_BREAKPOINT_LINK_SIZE			60

extern boolean g_isJavaDeubg;

extern boolean JavaDebug_callVmFunc(Packet *command, Packet *reply);
extern boolean JavaDeubg_callDebugFunc(Packet * commandPacket, Packet * replyPacket);
extern void JavaDebug_init();
extern void JavaDebug_loadSysPkg();

typedef struct
{
	u32 requestId;
	u32 pc;
} JavaDebug_Breakpoint;

typedef struct
{
	JavaDebug_Breakpoint JavaDebug_breakpoints[JAVA_DEBUG_BREAKPOINT_LINK_SIZE];
	u8 count;
} JavaDebug_BreakpointData;

extern JavaDebug_BreakpointData g_breakpointData;


typedef struct
{
	u32 requestId;
	u32 size;
	u32 depth;
	u32 fp;
	u32 startPC;
	u32 endPC;
} JavaDebug_Step;

extern JavaDebug_Step g_step;

#define JAVADEBUG_FUNCID_LOAD_PACKAGE				1
#define JAVADEBUG_FUNCID_ADD_EVENT					2
#define JAVADEBUG_FUNCID_INIT						3
#define JAVADEBUG_FUNCID_DEL_PACKAGE				4
#define JAVADEBUG_FUNCID_DEALLOCATE				    5

#define JAVADEBUG_EVENT_KIND_SINGLE_STEP			1
#define JAVADEBUG_EVENT_KIND_BREAKPOINT				2


#define JAVADEBUG_STEP_DEPTH_INTO                   0
#define JAVADEBUG_STEP_DEPTH_OVER                   1
#define JAVADEBUG_STEP_DEPTH_OUT                    2
#define JAVADEBUG_STEP_DEPTH_NONE                   -1

#define THREAD_STATUS_ZOMBIE                               0
#define THREAD_STATUS_RUNNING                              1
#define THREAD_STATUS_SLEEPING                             2
#define THREAD_STATUS_MONITOR                              3
#define THREAD_STATUS_WAIT                                 4

#endif /* SRC_HW_IO_JAVADEBUG_H_ */
