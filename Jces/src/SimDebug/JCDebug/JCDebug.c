/*
 * JCDebug.c
 *
 *  Created on: 2014Äê12ÔÂ25ÈÕ
 *      Author: mowenpan
 */
#include "JCDebug.h"

#include "stdlib.h"
#include <string.h>

#include "cardAPI.h"
#include "JavaDebug.h"
#include "Packet.h"


#define DEBUG_COMPONENT_MAX_SIZE	0xFFFFFFF

//extern FieldId g_fileId;
u32 g_debugComponentSize;
u32 g_debugComponentOffset;
PF8 g_debugComponentAddr;
u8 g_debugComponentData[DEBUG_COMPONENT_MAX_SIZE];
//JDWP_debug_component debug_component[JCDEBUG_PKG_MAX];

void setJCDebugComponentSize(u16 size)
{
	g_debugComponentSize = size;
}

void JCDebugComponentInit(PX8 buffer, u16 offset)
{
	g_debugComponentOffset = 0;
	memset(g_debugComponentData, 0, DEBUG_COMPONENT_MAX_SIZE);
	g_debugComponentAddr = g_debugComponentData;
	memcpy(g_debugComponentData, PM8_A_U32(MEMtPX8(buffer), offset), 2);
	g_debugComponentOffset = 2;
}

boolean JCDebugComponentProcess(PX8 buffer, u16 offset, u16 len)
{
	memcpy(PF8_A_U32(MEMtPF8(g_debugComponentData), g_debugComponentOffset), PX8_A_U16(MEMtPX8(buffer), offset), len);
	g_debugComponentOffset += len;
	return true;
}

boolean JCDebugComponentpostProcess(u8 pkgId, memref classCmpAddr, memref methodCmpAddr)
{
	Packet commadnPacket;
	Packet replyPacket;

	commadnPacket.data = malloc(g_debugComponentOffset + 9);

	PX_W_U32(commadnPacket.data, classCmpAddr);
	PX_W_U32(&commadnPacket.data[4], methodCmpAddr);
	PX_W_U8(&commadnPacket.data[8], pkgId);

	memcpy(&commadnPacket.data[9], g_debugComponentAddr, g_debugComponentOffset);

	commadnPacket.commandId = Packet_getCommandId();
	commadnPacket.flgs = PACKET_FLGS_COMMAND;
	commadnPacket.funcId = JAVADEBUG_FUNCID_LOAD_PACKAGE;
	commadnPacket.dataLen = g_debugComponentOffset + 9;

	JavaDeubg_callDebugFunc(&commadnPacket, &replyPacket);

	if (replyPacket.data != NULL)
	{
		free(replyPacket.data);
		replyPacket.data = NULL;
	}

	return true;
}

/**
 * delete PKG:PKGID
 */
void JCDeletePackageDebugComponent(u8 pkgId){
	Packet commadnPacket;
	Packet replyPacket;

	commadnPacket.data = malloc(1);
	PX_W_U8(&commadnPacket.data[0], pkgId);

	commadnPacket.commandId = Packet_getCommandId();
	commadnPacket.flgs = PACKET_FLGS_COMMAND;
	commadnPacket.funcId = JAVADEBUG_FUNCID_DEL_PACKAGE;
	commadnPacket.dataLen = 1;

	JavaDeubg_callDebugFunc(&commadnPacket, &replyPacket);

	if (replyPacket.data != NULL)
	{
		free(replyPacket.data);
		replyPacket.data = NULL;
	}
}

void JCDebugDeallocate(){
	Packet commadnPacket;
		Packet replyPacket;

		commadnPacket.data = malloc(1);
		PX_W_U8(&commadnPacket.data[0], 0xee);

		commadnPacket.commandId = Packet_getCommandId();
		commadnPacket.flgs = PACKET_FLGS_COMMAND;
		commadnPacket.funcId = JAVADEBUG_FUNCID_DEALLOCATE;
		commadnPacket.dataLen = 1;

		JavaDeubg_callDebugFunc(&commadnPacket, &replyPacket);

		if (replyPacket.data != NULL)
		{
			free(replyPacket.data);
			replyPacket.data = NULL;
		}
}

void initCallback(){
	initVmCallback();
}
