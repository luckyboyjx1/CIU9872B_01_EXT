/*
 * hw.c
 *
 *  Created on: 2014年2月14日
 *      Author: mowenpan
 */

#include "../SimDebug/hw.h"

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#include "hw_memory.h"
#include "pthread.h"
#include "hw.h"

#include "vm_callback.h"
#include "hal_callback.h"
#include "TLP224.h"
#include "hwConfig.h"
#include "JavaDebug.h"
#include "JDWP.h"

pthread_t javaDebugConnThreadId;
pthread_t callbackThreadId;

static void usage() {
	fprintf(stderr, "用法：\n");
	fprintf(stderr, "  [-javadebug] 开启调试 Java 功能\n");
	fprintf(stderr, "  [-host portNO] 指定虚拟拟读卡器主机\n");
	fprintf(stderr, "  [-port portNO] 指定虚拟读卡器 APDU 传输端口\n");
	fprintf(stderr, "  [-eport portNO] 指定虚拟读卡器事件端口\n");
	fprintf(stderr, "  [-dport portNO] 指定 Java 调试端口\n");
	exit(EXIT_FAILURE);
}


void *javaDebugConn (void *arg)
{
	struct sockaddr_in cli_addr;
	int clilen = sizeof(cli_addr);

	if ((g_javaDebugSocket = accept(fd_socket[fd_primary], (struct sockaddr far*) &cli_addr,
			(int far *) &clilen)) == INVALID_SOCKET)
	{
		//return false;

	}
	else
	{
		JavaDebug_init();
	}

	while (true)
	{

	}
}
/**
 * establish a socket connection
 */
boolean tlp_getConnection(TLP_MSG *msg)
{
	struct sockaddr_in cli_addr;
	int clilen = sizeof(cli_addr);

#if DEBUG_VERBOSE
	servertrace("getConnection()");
#endif

	if (g_isJavaDeubg == true)
	{
		pthread_create(&javaDebugConnThreadId, NULL, javaDebugConn, NULL);
	}
	return true;
}

void hw_init(int argc, char* argv[]) {
	int i;
	g_isJavaDeubg = false;
	u16 port = 29500;
	u16 eport = 29501;
	u16 dport = JC_DEFAULT_PORT_NUMBER;
	char * host;

	char* initHexFileName = NULL;
	char* outHexFileName = NULL;

	host = "127.0.0.1";

	fprintf(stderr, "command: ");
	for (i = 0; i < argc; i++) {
		fprintf(stderr, " %s", argv[i]);
	}
	fprintf(stderr, "\n");

	//analyse the arguments
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-initEep") == 0) {
			initHexFileName = argv[++i];
		} else if (strcmp(argv[i], "-outEep") == 0) {
			outHexFileName = argv[++i];
		} else if (strcmp(argv[i], "-javadebug") == 0) {
			g_isJavaDeubg = true;
		} else if (strcmp(argv[i], "-port") == 0) {
			i++;
			if (i >= argc) {
				usage();
			}
			port = (u16) atol(argv[i]);
		} else if (strcmp(argv[i], "-eport") == 0) {
			i++;
			if (i >= argc) {
				usage();
			}
			eport = (u16) atol(argv[i]);
		} else if (strcmp(argv[i], "-dport") == 0) {
			i++;
			if (i >= argc) {
				usage();
			}
			dport = (u16) atol(argv[i]);
		} else if (strcmp(argv[i], "-host") == 0) {
			i++;
			if (i >= argc) {
				usage();
			}
			host = argv[i];
		} else {
			//usage();
		}
	}

#if EN_EEPSAVE
	hw_memoryInit(initHexFileName, outHexFileName);
#else
	eepConfigInit();
#endif
	HwIo_Init(host, port, eport);

#if VM_WITH_DEBUG
	g_isJavaDeubg = true; //TODO: 要从输入参数取值.
#else
	g_isJavaDeubg = false;
#endif

	if (!tlp_createSockets(dport)) {
		/* unable to create socket */
		exit(1);
	}

	if (!tlp_getConnection(&g_msg)) {
		exit(1);
	}

	// if EN_AUTOSTOP is defined, It means that the Simulator run in dubug mode,timeout function will not open.
#if EN_AUTOSTOP
	//pthread_create(&sendMonitorId, NULL, sendMonitor, NULL);
#endif
}
