/*
 * JavaDebug.c
 *
 *  Created on: 2015年3月6日
 *      Author: mowenpan
 */
#include "JavaDebug.h"

#include "winbase.h"
#include "cardAPI.h"
#include "Packet.h"
#include "JDWP.h"
#include "pthread.h"


#define WAIT_TIME							5

#define READ_U32(d)							(* (PX32)(d))
#define READ_U8(d)							(* (PX8)(d))
#define WRITE_U8(d,v)						((*(PX8)(d)) = v)

#define OFFSET_HEAD						1

#define OFFSET_COUNT					1
#define OFFSET_PACKET					1
#define OFFSET_CONTENT					5

#define VMDATA_BUF_SIZE					2

#define STEP_SIZE 						24
#define STEP_MAX_COUNT					1
#define STEP_BUF_SIZE					(STEP_SIZE * STEP_MAX_COUNT + 5)

#define BREAKPOINT_SIZE					8
#define BREAKPOINT_MAX_COUNT			128
#define BREAKPOINT_BUF_SIZE				(BREAKPOINT_MAX_COUNT * BREAKPOINT_SIZE + 5)

#define CB_BUF_SIZE						0x20002

#define BUFFER_SIZE						(VMDATA_BUF_SIZE + STEP_BUF_SIZE + BREAKPOINT_BUF_SIZE + CB_BUF_SIZE)

#define P_OFFSET_VMDATA					0
#define P_OFFSET_STEP					(P_OFFSET_VMDATA + VMDATA_BUF_SIZE)
#define P_OFFSET_BREAKPOINT				(P_OFFSET_STEP + STEP_BUF_SIZE)
#define P_OFFSET_CB						(P_OFFSET_BREAKPOINT + BREAKPOINT_BUF_SIZE)

#define VM_FUNC_GET_TOP								1
#define VM_FUNC_ADD_BREAKPOINT						2
#define VM_FUNC_CLEAR_BREAKPOINT					3
#define VM_FUNC_CLEAR_ALL_BREAKPOINT				4

#define VM_FUNC_GET_FRAMES							10
#define VM_FUNC_GET_FRAME_COUNT						11
#define VM_FUNC_GET_PC_LINE							12
#define VM_FUNC_ADD_STEP							13
#define VM_FUNC_CLEAR_STEP							14

#define VM_FUNC_SUSPEND								15
#define VM_FUNC_RESUME								16
#define VM_FUNC_VMSTATUS							17
#define VM_FUNC_VM_EXIT								20

	//about MEMORY OPRATION (0x5X,6X)
#define VM_FUNC_GET_ARRAYDATA                  0x57

#define VM_FUNC_WRITEDATA                      0x5B
#define VM_FUNC_GET_DATA                       0x5C

#define VM_FUNC_GET_CLASSLOCATION              0x60
#define VM_FUNC_GET_OBJECT                     0x61
#define VM_FUNC_GET_OBJFIELDADDR               0x62
#define VM_FUNC_GET_ARRAYADDRESS               0x63
#define VM_FUNC_GET_METHODLOCATION             0x64
#define VM_FUNC_GET_FIELDINDEX                 0x65


//about Stack(0x7X,0x8X)
#define VM_FUNC_GetVMFP						   0x70
#define VM_FUNC_GETVMSTACK					   0x71
#define VM_FUNC_GETVMTOP					   0x72
#define VM_FUNC_GET_STATICTABLE				   0x73


#define setVmStatus(status) WRITE_U8(PX8_A_U8(g_vmDataBuf, OFFSET_HEAD), status)
#define getVmStatus() READ_U8(PX8_A_U8(g_vmDataBuf, OFFSET_HEAD))

pthread_t recvDataThreadId;
pthread_t sendDataThreadId;
JavaDebug_BreakpointData g_breakpointData;
JavaDebug_Step g_step;

//JavaDebug_VmStatus g_vmStatus;

PX8 g_vmDataBuf;
PX8 g_stepBuf;
PX8 g_breakpointBuf;
PX8 g_cbFuncBuf;
PX8 g_debugBuf;

boolean g_isJavaDeubg;

HANDLE hMapFile;
TCHAR szName[]=TEXT("Global\\JavaDebugFile");

void JavaDebug_init()
{
	g_packetCommandId = 1;
	memset(&g_breakpointData, 0, sizeof(JavaDebug_BreakpointData));
	memset(&g_step, 0 , sizeof(JavaDebug_Step));

   hMapFile = OpenFileMapping(
				   FILE_MAP_ALL_ACCESS,   // read/write access
				   FALSE,                 // do not inherit the name
				   szName);               // name of mapping object

   if (hMapFile == NULL)
   {
	  //_tprintf(TEXT("Could not open file mapping object (%d).\n"),
	//	 GetLastError());
	  return;
   }

   g_debugBuf = (PX8) MapViewOfFile(hMapFile, // handle to map object
			   FILE_MAP_ALL_ACCESS,  // read/write permission
			   0,
			   0,
			   BUFFER_SIZE);

   if (g_debugBuf == NULL)
   {
      //_tprintf(TEXT("Could not map view of file (%d).\n"),
      //       GetLastError());

      CloseHandle(hMapFile);

      return ;
   }

   g_vmDataBuf = PX8_A_CONST(g_debugBuf, P_OFFSET_VMDATA);
   g_stepBuf = PX8_A_CONST(g_debugBuf, P_OFFSET_STEP);
   g_breakpointBuf = PX8_A_CONST(g_debugBuf, P_OFFSET_BREAKPOINT);
   g_cbFuncBuf = PX8_A_CONST(g_debugBuf, P_OFFSET_CB);
   setVmStatus(THREAD_STATUS_ZOMBIE);


	g_packetRecvData.mutex = PTHREAD_MUTEX_INITIALIZER;
	g_packetRecvData.cond = PTHREAD_COND_INITIALIZER;

	g_packetSendData.mutex = PTHREAD_MUTEX_INITIALIZER;
	g_packetSendData.cond = PTHREAD_COND_INITIALIZER;

	/*
	g_vmStatus.vmstatus = THREAD_STATUS_RUNNING;
	g_vmStatus.mutex = PTHREAD_MUTEX_INITIALIZER;
	g_vmStatus.cond = PTHREAD_COND_INITIALIZER;
	*/

	pthread_create(&recvDataThreadId, NULL, Packet_recvRun, NULL);
	pthread_create(&sendDataThreadId, NULL, Packet_sendRun, NULL);

	pthread_join(recvDataThreadId, NULL);
	pthread_join(sendDataThreadId, NULL);
}

void testFunc(Packet *command, Packet *reply)
{
	PX8 rt = MEMtU8(malloc(4));
	reply->commandId = command->commandId;
	reply->flgs = PACKET_FLGS_REPLY;
	reply->funcId = 0; //error code

	write_u32(rt, 0xFC);
	reply->dataLen = 4;
	reply->data = rt;
}

void JavaDebug_addBreakpoint(Packet *command, Packet *reply)
{
	PX8 data = command->data;

	u32 requestId;
	u32 pc;
	u8 count;

	requestId = read_u32(data);
	pc = read_u32(&data[4]);

	count = g_breakpointData.count;
	g_breakpointData.JavaDebug_breakpoints[count].requestId = requestId;
	g_breakpointData.JavaDebug_breakpoints[count].pc = pc;
	g_breakpointData.count++;

	reply->funcId = 0;
}

static u32 getFrameCount()
{

	u32 stackDepth;
	u32 count;

	stackDepth = PXtMEM(CARD_VMSTACK) + CARD_STACKSIZE - PXtMEM(CARD_TOP);
	count = stackDepth / 12;

	return count;
}

void JavaDebug_getFrames(Packet *command, Packet *reply)
{
#define P_FRAME_SIZE	10

	u32 frameCount;
	u32 startIndex;
	u32 len;
	PX8 data;
	u32 index;
	u32 offset;

	u32 pc;
	u16 pkgId;

	if(&CARD_TOP == 0){
		offset = 0;
		data = MEMtU8(malloc(14));
		write_u32(data, 0);
		write_u32(&data[offset], 0);
		offset = U32_A_U8(offset, 4);

		write_u16(&data[offset], 0);
		offset = U32_A_U8(offset, 2);

		write_u32(&data[offset], 0);

		reply->data = data;
		reply->dataLen = 14;
		reply->funcId = 0;
		return;
	}

	frameCount = getFrameCount();
	data = command->data;

	startIndex = read_u32(data);
	len = read_u32(&data[4]);

	if (len == 0)
	{
		reply->funcId = 0;
		return ;
	}

	if (len == 0xFFFFFFFF) {
		len = frameCount;
	}

	data = MEMtU8(malloc(len * P_FRAME_SIZE + 4));

	write_u32(data, len);
	offset = 4;
	for (index = startIndex; index < frameCount; index++)
	{
		if (index == 0)
		{
			pkgId = CARD_PCPKGID;
			pc = PFtMEM(CARD_JPC);
		}
		else
		{
			pkgId = PXS_R_U16(&CARD_TOP[(index - 1) * 12 + 2]);
			pc = PXS_R_U32(&CARD_TOP[(index - 1) * 12 + 8]);
		}

		write_u32(&data[offset], index);
		offset = U32_A_U8(offset, 4);

		write_u16(&data[offset], pkgId);
		offset = U32_A_U8(offset, 2);

		write_u32(&data[offset], pc);
		offset = U32_A_U8(offset, 4);
	}

	reply->data = data;
	reply->dataLen = len * P_FRAME_SIZE + 4;
	reply->funcId = 0;
}


void JavaDebug_getFrameCount(Packet *command, Packet *reply)
{
	u32 count;
	PX8 data;

	data = MEMtU8(malloc(4));
	count = getFrameCount();
	write_u32(data, count);

	reply->data = data;
	reply->dataLen = 4;
	reply->funcId = 0;
}

void JavaDebug_getPcLine(Packet *command, Packet *reply)
{
	PX8 data;

	data = MEMtU8(malloc(10));
	write_u16(data, CARD_PCPKGID);
	write_u32(&data[2], PFtMEM(CARD_JPC));
	write_u32(&data[6], PFtMEM(CARD_FP));

	reply->data = data;
	reply->dataLen = 10;
	reply->funcId = 0;
}


void vmResume()
{
	setVmStatus(THREAD_STATUS_RUNNING);

	/*
	pthread_mutex_lock(&g_vmStatus.mutex);
	g_vmStatus.vmstatus = THREAD_STATUS_RUNNING;
	pthread_cond_signal(&g_vmStatus.cond);
	pthread_mutex_unlock(&g_vmStatus.mutex);
	*/
}

void vmSuspend()
{
	setVmStatus(THREAD_STATUS_WAIT);
	/*
	pthread_mutex_lock(&g_vmStatus.mutex);
	g_vmStatus.vmstatus = THREAD_STATUS_WAIT;
	pthread_cond_signal(&g_vmStatus.cond);
	pthread_mutex_unlock(&g_vmStatus.mutex);
	*/
}

void JavaDebug_resume(Packet *command, Packet *reply)
{
	vmResume();
	reply->funcId = 0;
}

void JavaDebug_suspend(Packet *command, Packet *reply)
{
	vmSuspend();
	reply->funcId = 0;
}

void JavaDebug_VmExit(Packet *command, Packet *reply)
{
	UnmapViewOfFile(g_debugBuf);
	 CloseHandle(hMapFile);
	exit(-1);
	reply->funcId = 0;
}

/*
void JavaDebug_vmStatus(Packet *command, Packet *reply)
{
	PX8 status = malloc(1);
	pthread_mutex_lock(&g_vmStatus.mutex);
	status[0] = g_vmStatus.vmstatus;
	pthread_mutex_unlock(&g_vmStatus.mutex);

	reply->data = status;
	reply->dataLen = 1;
	reply->funcId = 0;
}
*/

void JavaDebug_addStep(Packet *command, Packet *reply)
{
	PX8 data = command->data;

	u32 requestId;
	u32 size;
	u32 depth;
	u32 fp;
	u32 startPC;
	u32 endPC;

	requestId = read_u32(data);
	size = read_u32(&data[4]);
	depth = read_u32(&data[8]);
	fp = read_u32(&data[12]);
	startPC = read_u32(&data[16]);
	endPC = read_u32(&data[20]);

	g_step.requestId = requestId;
	g_step.size = size;
	g_step.depth = depth;
	g_step.fp = fp;
	g_step.startPC = startPC;
	g_step.endPC = endPC;

	reply->funcId = 0;

	vmResume();
}

void JavaDebug_clearStep(Packet *command, Packet *reply)
{
	memset(&g_step, 0 , sizeof(JavaDebug_Step));

	reply->funcId = 0;
}
/*
void JavaDebug_getByte(Packet *command, Packet *reply)
{
	PX8 data = command->data;
	memref addr = PX_R_U32(&data[0]);
	u8 value = PM_R_U8(addr);
	reply->data = malloc(4);
	PX_W_U32(reply->data, value);
	reply->dataLen = 4;
	reply->funcId = 0;
}
void JavaDebug_readByte(Packet *command, Packet *reply)
{
	PX8 data = command->data;
	memref addr = PX_R_U32(&data[0]);
	u8 value = MEMREADU8(addr);
	reply->data = malloc(4);
	PX_W_U32(reply->data, value);
	reply->dataLen = 4;
	reply->funcId = 0;
}
void JavaDebug_getShort(Packet *command, Packet *reply)
{
	PX8 data = command->data;
	memref addr = PX_R_U32(&data[0]);
	u16 value = PM_R_U16(addr);
	reply->data = malloc(4);
	PX_W_U32(reply->data, value);
	reply->dataLen = 4;
	reply->funcId = 0;
}
void JavaDebug_readShort(Packet *command, Packet *reply)
{
	PX8 data = command->data;
	memref addr = PX_R_U32(&data[0]);
	u16 value = MEMREADU16(addr);
	reply->data = malloc(4);
	PX_W_U32(reply->data, value);
	reply->dataLen = 4;
	reply->funcId = 0;
}
void JavaDebug_getInt(Packet *command, Packet *reply)
{
	PX8 data = command->data;
	memref addr = PX_R_U32(&data[0]);
	u32 value = PM_R_U32(addr);
	reply->data = malloc(4);
	PX_W_U32(reply->data, value);
	reply->dataLen = 4;
	reply->funcId = 0;
}
void JavaDebug_readInt(Packet *command, Packet *reply)
{
	PX8 data = command->data;
	memref addr = PX_R_U32(&data[0]);
	u32 value = MEMREADU32(addr);
	reply->data = malloc(4);
	PX_W_U32(reply->data, value);
	reply->dataLen = 4;
	reply->funcId = 0;
}*/

void JavaDebug_getArrayData(Packet *command, Packet *reply)
{
	int i = 0;
    u16 len ;

   PX8 data = command->data;
   //get arrayObject address
   memref addr = read_u32(&data[0]);
   memref arraddr = GET_ARRAY_DATA(MEMtPF8(addr));

   //get array length
   u16 arrayLen = GET_ARRAY_LENGTH(addr);
   u8 header = (u8)(PF_R_U8(MEMtPF8(addr)) & JCDEBUG_OBJ_HDR_OBJECT_TYPE_MASK);
   u8 elementLen = 1;
   switch (header) {
   		case JCDEBUG_Header_ArrayType_Boolean:
   		case JCDEBUG_Header_ArrayType_Byte:
   			elementLen = 1;
   			break;
   		case JCDEBUG_Header_ArrayType_Short:
   			elementLen = 2;
   			break;
   		case JCDEBUG_Header_ArrayType_Int:
   			elementLen = SIZE_INTEGER;
   			break;
   		case JCDEBUG_Header_ArrayType_Reference:
   			elementLen = SIZE_OBJECT_ID;
   			break;
   		default:
   			elementLen = SIZE_OBJECT_ID;
   	}

   //fill data
   len  = arrayLen *elementLen;
   reply->data = MEMtU8(malloc(len+8));
   reply->dataLen = len+8;
   write_u32(reply->data, arrayLen);
   write_u32(&reply->data[4], arraddr);
   switch (elementLen) {
   case 1:
	   for(i=0;i<len;i++){
		   write_u8(&reply->data[8+i], MEMREADU8(MEMtPM8(arraddr+i)));
	    }
	   break;
   case 2:
	   for(i=0;i<len;i+=2){
		   write_u16(&reply->data[8+i], MEMREADU16(MEMtPM8(arraddr+i)));
	   }
	   break;
   case 4:
   default:
	   for(i=0;i<len;i+=4){
		   write_u32(&reply->data[8+i], MEMREADU32(MEMtPM8(arraddr+i)));
	  }
   }

   reply->funcId = 0;
}

/**
 * len[4] addr[4] data[len]
 */
void JavaDebug_writeData(Packet *command, Packet *reply)
{
	PX8 data = command->data;
	int len = read_u32(&data[0]);
	PM8 addr = MEMtPM8(read_u32(&data[4]));
	MEMCOPYATOMIC(addr,&data[8],len);
//	memWriteU8Atomic(addr,value);
	reply->dataLen =0;
	reply->funcId = 0;
}
/**
 * len[4] addr[4]
 */
void JavaDebug_getData(Packet *command, Packet *reply)
{
	int i = 0;
	PX8 data = command->data;
	int len = read_u32(&data[0]);
	PM8 addr = MEMtPM8(read_u32(&data[4]));
	reply->data = MEMtU8(malloc(len));

	for(i=0;i<len;i++){
		write_u8(&reply->data[i], MEMREADU8(addr+i));
	}
	//MEMCOPYATOMIC(&reply->data[0],MEMtPX8(addr),len);
	reply->dataLen =len;
	reply->funcId = 0;
}
void JavaDebug_getClasslocation(Packet *command, Packet *reply)
{
	PX8 data = command->data;
	u32 pkgId = read_u32(&data[0]);
	reply->dataLen = 0;
	if((PF_R_U32(PF8_A_U16(MEMtPF8(CARD_PACKAGE_LOCATION_TABLE_ADDRESS), U8_M_U8(pkgId, CARD_SIZE_MEMREF))))){
		u32 value = GETPACKAGECOMPONENTLOCATION(pkgId, COMPONENT_CLASS);
		reply->data = MEMtU8(malloc(4));
		write_u32(reply->data, value);
		reply->dataLen = 4;
	}
	reply->funcId = 0;
}
void JavaDebug_getMethodlocation(Packet *command, Packet *reply)
{
	PX8 data = command->data;
	u32 pkgId = read_u32(&data[0]);
	reply->dataLen = 0;

	if((PF_R_U32(PF8_A_U16(MEMtPF8(CARD_PACKAGE_LOCATION_TABLE_ADDRESS), U8_M_U8(pkgId, CARD_SIZE_MEMREF))))){
		//get the base  address  of the package witch has a ID values pkgid.
		u32 value = GETPACKAGECOMPONENTLOCATION(pkgId, COMPONENT_METHOD);
		reply->data = MEMtU8(malloc(4));
		write_u32(reply->data, value);
		reply->dataLen = 4;
	}
	reply->funcId = 0;
}
void JavaDebug_getObject(Packet *command, Packet *reply)
{

	int i=0;
	u32 objId = 0;
	PX8 data = command->data;
	if(&data[0]!=0){//if data not found(by huang 21051201)
		objId = read_u32(&data[0]);
	}


	if((objId & 0xffff0000) != 0 || objId>MAPTABLE_GETTOPINDEX() ||objId==0){//if objId is wrong(by huang 21051201)
		reply->dataLen = 0;
		reply->funcId = 0;
		return ;
	}

	PM8 addr = MEMtPM8(MAPTABLE_GETENTRYADDR(objId));
	//PM8 addr = GetObjectFromMapTable(objId);


	int len = 6;
	if((MEMREADU8(addr) & 0xc0) != 0){
		len = 12;
	}

	reply->data = MEMtU8(malloc(len +8));
	write_u32(&reply->data[0], len);
	write_u32(&reply->data[4], PXtMEM(addr));
	for(i=0;i<len;i++){
		write_u8(&reply->data[8+i], MEMREADU8(addr+i));
	}
	reply->dataLen = len +8;
	reply->funcId = 0;
}

void JavaDebug_getObjFieldaddress(Packet *command, Packet *reply)
{
	PX8 data = command->data;
	reply->data = MEMtU8(malloc(4));
	u32 objAddr = read_u32(&data[0]);

	u32 index = read_u32(&data[4]);//0

	u32 value = PNtMEM(GET_OBJECT_FIELD_REF(objAddr, index));
	write_u32(reply->data, value);
	reply->dataLen = 4;
	reply->funcId = 0;
}
void JavaDebug_getArrayaddress(Packet *command, Packet *reply)
{
	PX8 data = command->data;
	reply->data = MEMtU8(malloc(4));
	memref addr = read_u32(&data[0]);
	memref arraddr = GET_ARRAY_DATA(MEMtPF8(addr));

	write_u32(reply->data, arraddr);
	reply->dataLen = 4;
	reply->funcId = 0;
}
void JavaDebug_getFieldindex(Packet *command, Packet *reply)
{
	u32	index;

	PX8 data = command->data;
	reply->data = MEMtU8(malloc(4));
	u32 superClassAddr = read_u32(&data[0]);//60115

	index = nvmReadU8(MEMtPN8(PF8_A_CONST(superClassAddr, OFFSET_CLASS_DEC_INST_SIZE)));//PF_R_U8(PF8_A_CONST(classAddress, CR_FirstRefIndex));//6


	write_u32(reply->data, index);
	reply->dataLen = 4;
	reply->funcId = 0;
}
void JavaStackCopy(PX16 des, PX16 src, u16 length)
{
	u16 i;
	u16 wordLeng = length >> 1;

	for(i = 0; i < wordLeng ; i ++)
	{
		write_u16(&des[i],src[i]);
	}

}
void JavaDebug_vmFp(Packet *command, Packet *reply)
{
	int len = U32_S_U32(CARD_VMSTACK, CARD_FP) +CARD_STACKSIZE;
	reply->data = MEMtU8(malloc(len+8));
	reply->dataLen = len+8;
	write_u32(reply->data, len);
	write_u32(&reply->data[4], PXtMEM(CARD_FP));
	JavaStackCopy(MEMtPX16(&reply->data[8]),MEMtPX16(CARD_FP),len);
	reply->funcId = 0;
}

void JavaDebug_vmStack(Packet *command, Packet *reply)
{
	int len = CARD_STACKSIZE;
	reply->data = MEMtU8(malloc(len+8));
	reply->dataLen = len+8;
	write_u32(reply->data, len);
	write_u32(&reply->data[4], PXtMEM(CARD_VMSTACK));
	JavaStackCopy(MEMtPX16(&reply->data[8]),MEMtPX16(CARD_VMSTACK),len);
	reply->funcId = 0;
}

void JavaDebug_vmTop(Packet *command, Packet *reply)
{
	int len;
	len = CARD_VMSTACK - CARD_TOP +CARD_STACKSIZE;
	reply->data = MEMtU8(malloc(len+8));
	reply->dataLen = len+8;
	write_u32(reply->data, len);
	write_u32(&reply->data[4], PXtMEM(CARD_TOP));
	JavaStackCopy(MEMtPX16(&reply->data[8]),MEMtPX16(CARD_TOP),len);
	reply->funcId = 0;
}
/**
 * modifed by xxj@20210328: mask中包对象的个数不变，后下载的包在下载过程中创建的对象由5个(class_component / method_component / staticfield_component / export_component / pkg_info)减少为1个(pkg_info)
 * pkg_info中的信息，mask中包对象保持不变，存储的分别是4个组建引用(4字节) + 组建大小(2字节) ; 后下载的包对象发生变化，存储的分别是4个组建地址相对于pkg_info起始地址的偏移(4字节) + 组建大小(2字节)
 */
void JavaDebug_getStaticTable(Packet *command, Packet *reply)
{
   PF8 FieldCompLoc;
   u16 arrayLen;

   PX8 data = command->data;
   //get PKG ID
   memref pkg = read_u32(&data[0]);

   //get static field address and length
   memref pkg_info_addr = MAPTABLE_GETENTITYCONTENT(U32t16(NVMREADU32(PF8_A_U16(MEMtPF8(CARD_PACKAGE_LOCATION_TABLE_ADDRESS), U8_M_U8(pkg, SIZE_MEMREF)))));
   arrayLen = NVMREADU16((PN8)PF8_A_CONST(pkg_info_addr, ((COMPONENT_STATICFIELD * (SIZE_MEMREF + SIZE_U16))+SIZE_MEMREF))); //获取COMPONENT_STATICFIELD的size
   if(pkg < NPACKAGES)
   {
	   FieldCompLoc	= MEMtPF8(MAPTABLE_GETENTITYCONTENT(U32t16(NVMREADU32((PN8)PF8_A_CONST(pkg_info_addr, (COMPONENT_STATICFIELD * (SIZE_MEMREF + SIZE_U16)))))));//获取COMPONENT_STATICFIELD的起始地址
   }
   else
   {
	   FieldCompLoc	= MEMtPF8(U32_A_U32(pkg_info_addr, NVMREADU32((PN8)PF8_A_CONST(pkg_info_addr, (COMPONENT_STATICFIELD * (SIZE_MEMREF + SIZE_U16))))));
   }
   //fill data
   reply->data = MEMtU8(malloc(arrayLen+8));
   reply->dataLen = arrayLen+8;
   write_u32(reply->data, arrayLen);
   write_u32(&reply->data[4], PXtMEM(FieldCompLoc));
//   memcpy(&reply->data[8],memoryToPoint(PFtMEM(FieldCompLoc)),arrayLen);
   memcpy(&reply->data[8],MEMtPX8(FieldCompLoc),arrayLen);

   if((FieldCompLoc <=CARD_STREAM_PAGE) && (FieldCompLoc+arrayLen >CARD_STREAM_PAGE)){
	 int offset = CARD_STREAM_PAGE - FieldCompLoc;
	 int len = FieldCompLoc+arrayLen - CARD_STREAM_PAGE;
	 len =(len>CARD_EEPAGE_SIZE)?CARD_EEPAGE_SIZE:len;
	 memcpy((&reply->data[8]+offset),&CARD_STREAM_BUFFER,len);
   }else if((FieldCompLoc >CARD_STREAM_PAGE) && (FieldCompLoc+arrayLen <CARD_STREAM_PAGE+CARD_EEPAGE_SIZE)){
	   int offset = FieldCompLoc - CARD_STREAM_PAGE;
	   memcpy((&reply->data[8]),&CARD_STREAM_BUFFER[offset],arrayLen);
   }else if((FieldCompLoc <CARD_STREAM_PAGE+CARD_EEPAGE_SIZE) && (FieldCompLoc+arrayLen >CARD_STREAM_PAGE+CARD_EEPAGE_SIZE)){
	   int offset = FieldCompLoc - CARD_STREAM_PAGE;
	   int len = CARD_STREAM_PAGE +CARD_EEPAGE_SIZE -FieldCompLoc;
	   len =(len>CARD_EEPAGE_SIZE)?CARD_EEPAGE_SIZE:len;
	   memcpy((&reply->data[8]),&CARD_STREAM_BUFFER[offset],len);
   }

   reply->funcId = 0;
}

void JavaDebug_clearBreakpoint(Packet *command, Packet *reply)
{
	PX8 data = command->data;


	u32 requestId;
	u8 count;
	u8 index;

	requestId = read_u32(data);
	count = g_breakpointData.count;

	for (index = 0; index < count; index++)
	{
		if (g_breakpointData.JavaDebug_breakpoints[index].requestId == requestId) {
			if (index + 1 < count)
			{
				PX8 dest;
				PX8 src;


				dest = MEMtPX8(&g_breakpointData.JavaDebug_breakpoints[index]);
				src = MEMtPX8(&g_breakpointData.JavaDebug_breakpoints[index + 1]);
				memcpy(dest ,src, sizeof(JavaDebug_Breakpoint) * (count - index - 1));
			}
			else
			{
				g_breakpointData.JavaDebug_breakpoints[index].requestId = 0;
				g_breakpointData.JavaDebug_breakpoints[index].pc = 0;
			}


			g_breakpointData.count--;
			break;
		}
	}
	reply->funcId = 0;
}

void JavaDebug_clearAllBreakpoint(Packet *command, Packet *reply)
{
	memset(&g_breakpointData, 0, sizeof(JavaDebug_BreakpointData));
	reply->funcId = 0;
}

void JavaDebug_initJcdt(){
	Packet commadnPacket;
	Packet replyPacket;

	commadnPacket.commandId = Packet_getCommandId();
	commadnPacket.flgs = PACKET_FLGS_COMMAND;
	commadnPacket.funcId = JAVADEBUG_FUNCID_INIT;
	commadnPacket.dataLen = 0;
	commadnPacket.data = NULL;

	setVmStatus(THREAD_STATUS_RUNNING);
	JavaDeubg_callDebugFunc(&commadnPacket, &replyPacket);

	if (replyPacket.data != NULL)
	{
		free(replyPacket.data);
		replyPacket.data = NULL;
	}
}


boolean JavaDebug_callVmFunc(Packet *command, Packet *reply)
{
	u32 rtLen;
	u16 funcId;

	funcId = command->funcId;
	rtLen = 0;
	switch (funcId)
	{
		case VM_FUNC_GET_TOP:
			testFunc(command, reply);
			break;
		case VM_FUNC_ADD_BREAKPOINT:
			JavaDebug_addBreakpoint(command, reply);
			break;
		case VM_FUNC_GET_FRAMES:
			JavaDebug_getFrames(command, reply);
			break;
		case VM_FUNC_GET_FRAME_COUNT:
			JavaDebug_getFrameCount(command, reply);
			break;
		case VM_FUNC_GET_PC_LINE:
			JavaDebug_getPcLine(command, reply);
			break;
		case VM_FUNC_ADD_STEP:
			JavaDebug_addStep(command, reply);
			break;
		case VM_FUNC_SUSPEND:
			JavaDebug_suspend(command, reply);
			break;
		case VM_FUNC_VM_EXIT:
			JavaDebug_VmExit(command, reply);
			break;
		/*
		case VM_FUNC_RESUME:
			JavaDebug_resume(command, reply);
			break;
		case VM_FUNC_VMSTATUS:
			JavaDebug_vmStatus(command, reply);
			break;
		*/
		//about MEMORY OPRATION
		case VM_FUNC_GET_ARRAYDATA:
			JavaDebug_getArrayData(command, reply);
			break;
		case VM_FUNC_WRITEDATA:
			JavaDebug_writeData(command, reply);
			break;
		case VM_FUNC_GET_DATA:
			JavaDebug_getData(command, reply);
			break;
		case VM_FUNC_GET_CLASSLOCATION:
			JavaDebug_getClasslocation(command, reply);
			break;
		case VM_FUNC_GET_OBJECT:
			JavaDebug_getObject(command, reply);
			break;
		case VM_FUNC_GET_OBJFIELDADDR:
			JavaDebug_getObjFieldaddress(command, reply);
			break;
		case VM_FUNC_GET_ARRAYADDRESS:
			JavaDebug_getArrayaddress(command, reply);
			break;
		case VM_FUNC_GET_METHODLOCATION:
			JavaDebug_getMethodlocation(command, reply);
			break;
		case VM_FUNC_GET_FIELDINDEX:
			JavaDebug_getFieldindex(command, reply);
			break;


		//about stack
		case VM_FUNC_GetVMFP:
			JavaDebug_vmFp(command, reply);
			break;
		case VM_FUNC_GETVMSTACK:
			JavaDebug_vmStack(command, reply);
			break;
		case VM_FUNC_GETVMTOP:
			JavaDebug_vmTop(command, reply);
			break;
		case VM_FUNC_GET_STATICTABLE:
			JavaDebug_getStaticTable(command, reply);
			break;
		default:
			break;
	}

	return true;
	/*
	Packet packetCommand;
	Packet packetReply;
	boolean rtFlg;
	u16 funcId;
	u16 errorCode;

	while(true)
	{
		rtFlg = pkt_getCommandPacket(&packetCommand);
		if (rtFlg == false)
		{
			//等待
			continue;
		}

		funcId = packetCommand.funcId;
		switch(funcId)
		{
			case 0x1:
				break;
			default:
				break;
		}

		packetReply.funcId = errorCode;
		pkt_addPacketToSend();
	}
	*/
}

u32 g_commandId;

boolean JavaDeubg_callDebugFunc(Packet * commandPacket, Packet * replyPacket)
{
	Packet_addPacketToSend(commandPacket);
	return Packet_getReply(replyPacket);
}

static void clearStep()
{
	memset(g_stepBuf, 0, STEP_BUF_SIZE);
}

void JavaDebug_addEvent(u32 requestId, u8 eventKind)
{
	Packet commandPacket;
	Packet replyPacket;

	PX8 data;
	int dataLen;

	if (eventKind == JAVADEBUG_EVENT_KIND_SINGLE_STEP)
	{
		dataLen = 11;
		data = MEMtU8(malloc(dataLen));

		write_u32(data, requestId);
		write_u8(&data[4], eventKind);
		write_u16(&data[5], CARD_PCPKGID);
		write_u32(&data[7], PFtMEM(CARD_JPC));
	}
	else
	{
		dataLen = 5;
		data = MEMtU8(malloc(dataLen));

		write_u32(data, requestId);
		write_u8(&data[4], eventKind);
	}


	commandPacket.commandId = Packet_getCommandId();
	commandPacket.flgs = PACKET_FLGS_COMMAND;
	commandPacket.data = data;
	commandPacket.dataLen = dataLen;
	commandPacket.funcId = JAVADEBUG_FUNCID_ADD_EVENT;

	JavaDeubg_callDebugFunc(&commandPacket, &replyPacket);

	if (replyPacket.data != NULL)
	{
		free(replyPacket.data);
		replyPacket.data = NULL;

	}


	/*
	pthread_mutex_lock(&g_vmStatus.mutex);
	g_vmStatus.vmstatus = THREAD_STATUS_WAIT;
	while (g_vmStatus.vmstatus == THREAD_STATUS_WAIT)
	{
		pthread_cond_wait(&g_vmStatus.cond, &g_vmStatus.mutex);
	}
	pthread_mutex_unlock(&g_vmStatus.mutex);
	*/
	setVmStatus(THREAD_STATUS_WAIT);
	while (getVmStatus() != THREAD_STATUS_RUNNING)
	{
		Sleep(WAIT_TIME);
	}
}

void JavaDebug_Interpreter1(memref curJPC)
{
	u32 breakpointCount;
	u32 requestId;
	u8 index;
	boolean flg;

	u32 size;
	u32 depth;
	u32 fp;
	u32 startPc;
	u32 endPc;
	u32 offset;
	u32 pc;

	flg = false;



	//pthread_mutex_lock(&g_vmStatus.mutex);
	offset = OFFSET_CONTENT;
	requestId = READ_U32(PX8_A_U8(g_stepBuf, OFFSET_CONTENT));
	offset = U8_A_U8(offset, 4);

	if (requestId != 0) {
		size = READ_U32(PX8_A_U8(g_stepBuf, offset));
		offset = U8_A_U8(offset, 4);

		depth = READ_U32(PX8_A_U8(g_stepBuf, offset));
		offset = U8_A_U8(offset, 4);

		fp = READ_U32(PX8_A_U8(g_stepBuf, offset));
		offset = U8_A_U8(offset, 4);

		startPc = READ_U32(PX8_A_U8(g_stepBuf, offset));
		offset = U8_A_U8(offset, 4);

		endPc = READ_U32(PX8_A_U8(g_stepBuf, offset));
		offset = U8_A_U8(offset, 4);

		if (depth == JAVADEBUG_STEP_DEPTH_OVER)
		{
			if (PFtMEM(CARD_FP) == fp)
			{
				if (curJPC < startPc || curJPC > endPc)
				{
					flg = true;
				}
			}
		}
	}
	//pthread_mutex_unlock(&g_vmStatus.mutex);

	if (flg == true)
	{
		clearStep();
		JavaDebug_addEvent(requestId, JAVADEBUG_EVENT_KIND_SINGLE_STEP);
		return;
	}


	//when jpc == breakpoint
	flg = false;
	//pthread_mutex_lock(&g_vmStatus.mutex);
	breakpointCount = READ_U32(PX8_A_U8(g_breakpointBuf, OFFSET_COUNT));
	offset = OFFSET_CONTENT;
	if (breakpointCount > 0)
	{
		for (index = 0; index < breakpointCount; index++)
		{
			requestId = READ_U32(PX8_A_CONST(g_breakpointBuf, offset));
			offset = U32_A_U32(offset, 4);

			pc = READ_U32(PX8_A_CONST(g_breakpointBuf, offset));
			offset = U32_A_U32(offset, 4);

			if (pc == curJPC) {
				// breakpoint event
				flg = true;
				break;
			}
		}

	}
	//pthread_mutex_unlock(&g_vmStatus.mutex);

	if (flg == true)
	{
		JavaDebug_addEvent(requestId, JAVADEBUG_EVENT_KIND_BREAKPOINT);
		return;
	}

}

void JavaDebug_Interpreter2(memref curJPC, u8 byteCode) {
	boolean flg = false;
	u32 requestId;

	u32 size;
	u32 depth;
	u32 fp;
	u32 startPc;
	u32 endPc;
	u8 offset;

	offset = OFFSET_CONTENT;
	requestId = READ_U32(PX8_A_U8(g_stepBuf, OFFSET_CONTENT));
	offset = U8_A_U8(offset, 4);
	//pthread_mutex_lock(&g_vmStatus.mutex);
	if (requestId != 0)
	{
		size = READ_U32(PX8_A_U8(g_stepBuf, offset));
		offset = U8_A_U8(offset, 4);

		depth = READ_U32(PX8_A_U8(g_stepBuf, offset));
		offset = U8_A_U8(offset, 4);

		fp = READ_U32(PX8_A_U8(g_stepBuf, offset));
		offset = U8_A_U8(offset, 4);

		startPc = READ_U32(PX8_A_U8(g_stepBuf, offset));
		offset = U8_A_U8(offset, 4);

		endPc = READ_U32(PX8_A_U8(g_stepBuf, offset));
		offset = U8_A_U8(offset, 4);

		switch (depth)
		{
			case JAVADEBUG_STEP_DEPTH_INTO:
				if ((byteCode >= 139) && (byteCode <= 142)) //0x8B - 0x8E
				{
					flg = true;
				}
				if (PFtMEM(CARD_FP) == fp)
				{
					if (curJPC < startPc || curJPC > endPc)
					{
						flg = true;
					}
				}
				/* no break */
			case JAVADEBUG_STEP_DEPTH_OVER:
				if (PFtMEM(CARD_FP)  < fp
						&& (byteCode >= 119)
						&& (byteCode <=122)) //0x77 - 0x7A
				{
					flg = true;
				}
				break;
			case JAVADEBUG_STEP_DEPTH_OUT:
				if ((PFtMEM(CARD_FP) < fp) && (byteCode >= 119) && (byteCode <=122)) //0x77 - 0x7A
				{
					flg = true;
				}
				break;
			default:
				flg = false;
				break;
		}
	}
	//pthread_mutex_unlock(&g_vmStatus.mutex);

	//send event
	if (flg == true)
	{
		clearStep();
		JavaDebug_addEvent(requestId, JAVADEBUG_EVENT_KIND_SINGLE_STEP);
	}
}


#if VM_WITH_DEBUG
void JavaDebug_loadSysPkg()
{
	u32 offset;
	u16 pkgId;
	u16 dataLen;
	u16 count;
	memref classComponentAddr;
	memref methodComponentAddr;
	Packet commadnPacket;
	Packet replyPacket;
	JavaDebug_initJcdt();


	offset = 0;

	for (count = 0; count < CARD_DEBUG_COMPONENT_PKG_COUNT; count++)
	{
		dataLen = read_u16(CARD_DEBUGMASK + offset + 1);
		dataLen += 3;
		commadnPacket.data = MEMtPX8(malloc(dataLen + 8));

		pkgId = read_u8(CARD_DEBUGMASK + offset);
		classComponentAddr = GETPACKAGECOMPONENTLOCATION(pkgId, COMPONENT_CLASS);
		methodComponentAddr = GETPACKAGECOMPONENTLOCATION(pkgId, COMPONENT_METHOD);
		write_u32(commadnPacket.data, classComponentAddr);
		write_u32(&commadnPacket.data[4], methodComponentAddr);

		memcpy(&commadnPacket.data[8], (u8 *)(CARD_DEBUGMASK + offset), dataLen);
		offset += dataLen;

		commadnPacket.commandId = Packet_getCommandId();
		commadnPacket.flgs = PACKET_FLGS_COMMAND;
		commadnPacket.funcId = JAVADEBUG_FUNCID_LOAD_PACKAGE;
		commadnPacket.dataLen = dataLen + 8;


		JavaDeubg_callDebugFunc(&commadnPacket, &replyPacket);

		if (replyPacket.data != NULL)
		{
			free(replyPacket.data);
			replyPacket.data = NULL;

		}
	}
}
#endif
