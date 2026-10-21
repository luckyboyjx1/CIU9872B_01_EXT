#include "object.h"
#include "stack.h"
#include "memory.h"
#include "context.h"
#include "mask.h"
#include "anomaly.h"
#include "context.h"
#include "chn_manager.h"
#include "packageloc.h"
#include "reference.h"
#include "global_variable.h"
#include "RamManager.h"
#include "optspace.h"
#include "error.h"
#include "bc_control.h"
#include "utilMethod.h"

#if EMV_OPTIMIZATION_CODE

#define SELECTED_CONTEXT_OFFSET (MAX_LOGICAL_CHANNELS * 5)

__asm void checkArrayRead(PF8 arrayAddr)		SMALL_MODE
{
	IMPORT throw_error
	// modified by wh 2010.2.22 , this method can reduce read eeprom times.
	LDRB     r1,[r0,#0]
	LSLS     r2,r1,#27
	LSRS     r2,r2,#30
	BNE      cbLable5
	LSLS     r3,r1,#30
	LDR      r1,=__cpp(&cc)
	LDRB     r1,[r1,#1]            
	BMI      cbLable3
	LDRB     r0,[r0,#1]            
	CMP      r0,r1                 
	BNE      cbLable1
	BX      lr
cbLable1
	CMP      r1,#0                 
	BNE      cbLable2
	BX      lr
cbLable2
	PUSH     {r4,lr}
	LDR      r0,cbParam2
	BL       throw_error
	POP      {r4,pc}
cbLable3
	LDRB     r3,[r0,#1]            
	LDR      r0,=__cpp(&selected_channel)
	LDRB     r0,[r0,#0]             
	LSLS     r2,r0,#1              
	LDR      r0,=__cpp(&channelInfo)
	ADDS     r0,r2,r0              
	LDRB     r0,[r0,#SELECTED_CONTEXT_OFFSET]         
	CMP      r3,r0                 
	BNE      cbLable2
	CMP      r1,r0                 
	BNE      cbLable4
	BX      lr
cbLable4
	CMP      r1,#0                 
	BNE      cbLable2
cbLable5
	BX      lr

|cbParam1|
	  DCD      0x00000000
|cbParam2|
	  DCD      0x00008007
}

#endif
