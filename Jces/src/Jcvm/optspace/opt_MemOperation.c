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

__asm u8 memReadU8(PM8 addr)
{
	LDR      r1,=__cpp(&stream_page)
	LDR      r1,[r1,#0]
	CMP      r1,#0
	BNE      R8lable1
	LDRB     r0,[r0,#0]
	BX       lr

R8lable1
	LSRS     r2,r0,#6
	LSLS     r2,r2,#6
	CMP      r2,r1
	BEQ      R8lable2
	LDRB     r0,[r0,#0]
	BX       lr

R8lable2
	LSLS     r0,r0,#26            
	LDR      r1,=__cpp(&stream_buffer)
	LSRS     r0,r0,#26             
	LDRB     r0,[r1,r0]           
	BX       lr
}

__asm u16 memReadU16(PM8 addr)
{
	LDR      r1,=__cpp(&stream_page)
	LDR      r2,[r1,#0]
	CMP      r2,#0
	BNE      R16lable1
	LDRB     r1,[r0,#1]
	LDRB     r2,[r0,#0]
	LSLS     r0,r2,#8
	ORRS     r0,r0,r1
	BX       lr
R16lable1
	PUSH     {r4}
	LSLS     r1,r0,#26
	LSRS     r4,r0,#6
	LSRS     r1,r1,#26
	LSLS     r4,r4,#6
	LDR      r3,=__cpp(&stream_buffer)
	CMP      r4,r2
	BEQ      R16lable2
	CMP      r1,#0x3f
	BEQ      R16lable3
	LDRB     r1,[r0,#1]
	LDRB     r2,[r0,#0]
	LSLS     r0,r2,#8
	ORRS     r0,r0,r1
	POP      {r4}
	BX       lr
R16lable2
	CMP      r1,#0x3f
	BEQ      R16lable7
	ADDS     r1,r3,r1
	LDRB     r0,[r1,#1]
	LDRB     r1,[r1,#0]
	LSLS     r1,r1,#8
	ORRS     r0,r0,r1
	POP      {r4}
	BX       lr
R16lable3
	LDRB     r1,[r0,#0]
	ADDS     r4,r0,#1
	LSLS     r1,r1,#8
	CMP      r2,r4
	BNE      R16lable4
	LDRB     r0,[r3,#0]
	B        R16lable5
R16lable4
	LDRB     r0,[r0,#1]
R16lable5
	POP      {r4}
	ORRS     r0,r0,r1
	BX       lr
R16lable6
	CMP      r1,#0x3f
	BEQ      R16lable7
	ADDS     r1,r3,r1
	LDRB     r0,[r1,#1]
	LDRB     r1,[r1,#0]
	LSLS     r1,r1,#8
	ORRS     r0,r0,r1
	POP      {r4}
	BX       lr
R16lable7
	LDR      r1,=__cpp(&stream_buffer[0x20])
	LDRB     r0,[r0,#1]
	LDRB     r1,[r1,#0x1f]
	POP      {r4}
	LSLS     r1,r1,#8
	ORRS     r0,r0,r1
	BX       lr
}

__asm u32 memReadU32(PM8 addr)
{
	LDR      r1,=__cpp(&stream_page)
	LDR      r1,[r1,#0]
	CMP      r1,#0
	BNE      R32lable1
	LDRB     r1,[r0,#3]            
	LDRB     r2,[r0,#2]            
	LSLS     r1,r1,#8              
	ORRS     r1,r1,r2              
	LDRB     r2,[r0,#1]            
	LDRB     r0,[r0,#0]            
	LSLS     r2,r2,#16             
	REV16    r1,r1                 
	ORRS     r1,r1,r2              
	LSLS     r0,r0,#24      
	ORRS     r0,r0,r1              
	BX       lr
R32lable1
	PUSH     {r4}
	LSLS     r2,r0,#26
	LSRS     r4,r0,#6
	LSRS     r2,r2,#26             
	LSLS     r4,r4,#6
	LDR      r3,=__cpp(&stream_buffer)
	CMP      r4,r1                 
	BEQ      R32lable6
	CMP      r2,#0x3d              
	BCS      R32lable2
	LDRB     r1,[r0,#3]            
	LDRB     r2,[r0,#2]            
	LSLS     r1,r1,#8              
	ORRS     r1,r1,r2              
	LDRB     r2,[r0,#1]            
	LDRB     r0,[r0,#0]            
	LSLS     r2,r2,#16             
	REV16    r1,r1                 
	ORRS     r1,r1,r2              
	LSLS     r0,r0,#24             
	POP      {r4}
	ORRS     r0,r0,r1              
	BX       lr
R32lable2
	ADDS     r4,r0,#3              
	LSRS     r4,r4,#6              
	LSLS     r4,r4,#6              
	CMP      r4,r1                 
	BEQ      R32lable3
	LDRB     r1,[r0,#3]            
	LDRB     r2,[r0,#2]            
	LSLS     r1,r1,#8              
	ORRS     r1,r1,r2              
	LDRB     r2,[r0,#1]            
	LDRB     r0,[r0,#0]            
	LSLS     r2,r2,#16             
	REV16    r1,r1                 
	ORRS     r1,r1,r2              
	LSLS     r0,r0,#24             
	POP      {r4}
	ORRS     r0,r0,r1              
	BX       lr
R32lable3
	LDR      r4,=__cpp(&stream_buffer)
	CMP      r2,#0x3f              
	LDRB     r1,[r4,#0]           
	LDRB     r4,[r4,#1]           
	BEQ      R32lable4
	CMP      r2,#0x3e            
	LDRB     r2,[r0,#0]           
	BEQ      R32lable5
	LDRB     r3,[r0,#1]           
	LSLS     r2,r2,#24            
	LSLS     r3,r3,#16            
	LDRB     r0,[r0,#2]           
	ORRS     r2,r2,r1             
	ORRS     r3,r3,r2             
	LSLS     r0,r0,#8             
	ORRS     r0,r0,r3             
	B        R32lable7
R32lable4
	LDRB     r0,[r0,#0]            
	LSLS     r2,r0,#24             
	LSLS     r0,r1,#16            
	ORRS     r0,r0,r2             
	LSLS     r2,r4,#8             
	ORRS     r2,r2,r0             
	LDRB     r1,[r3,#2]            
	B        R32lable9
R32lable5
	LDRB     r0,[r0,#1]           
	LSLS     r2,r2,#24            
	LSLS     r0,r0,#16            
	ORRS     r0,r0,r2             
	LSLS     r2,r1,#8             
	ORRS     r2,r2,r0             
	MOV      r1,r4                
	B        R32lable9
R32lable6
	CMP      r2,#0x3d             
	BCS      R32lable8
	ADDS     r0,r3,r2              
	LDRB     r1,[r0,#3]            
	LDRB     r4,[r0,#2]            
	LSLS     r1,r1,#8              
	LDRB     r0,[r0,#1]            
	ORRS     r1,r1,r4              
	REV16    r1,r1                 
	LSLS     r0,r0,#16             
	ORRS     r1,r1,r0              
	LDRB     r0,[r3,r2]            
	LSLS     r0,r0,#24             
R32lable7
	ORRS     r1,r1,r0              
	B        R32lable10
R32lable8
	LDR      r4,=__cpp(&stream_buffer[0x20])
	CMP      r2,#0x3d            
	LDRB     r3,[r4,#0x1e]         
	LDRB     r1,[r4,#0x1f]         
	BEQ      R32lable11
	CMP      r2,#0x3e             
	BEQ      R32lable12
	LSLS     r2,r1,#24          
	LDRB     r1,[r0,#1]         
	LSLS     r1,r1,#16          
	ORRS     r1,r1,r2           
	LDRB     r2,[r0,#2]         
	LSLS     r2,r2,#8           
	ORRS     r2,r2,r1           
	LDRB     r1,[r0,#3]            
R32lable9
	ORRS     r1,r1,r2              
R32lable10
	POP      {r4}
	MOV      r0,r1                 
	BX       lr
R32lable11
	LDRB     r2,[r4,#0x1d]         
	LSLS     r4,r2,#24             
	LSLS     r2,r3,#16             
	LSLS     r3,r1,#8              
	ORRS     r2,r2,r4              
	LDRB     r1,[r0,#3]            
	ORRS     r3,r3,r2              
	ORRS     r1,r1,r3              
	B        R32lable10
R32lable12
	LSLS     r2,r3,#24             
	LSLS     r1,r1,#16             
	ORRS     r1,r1,r2              
	LDRB     r2,[r0,#2]            
	LSLS     r2,r2,#8              
	ORRS     r2,r2,r1              
	LDRB     r1,[r0,#3]            
	B        R32lable9
}

#endif
