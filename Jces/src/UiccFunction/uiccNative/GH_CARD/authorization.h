/*

 * Copyright (c) 2007,eastcompeace

 * All rights reserved.

 * FILE NAME: authorization.h 公话卡功能

 * RELEASE: 2007-08-30

 * VERSION: 1.1

 * AUTHER:	WG YJG CaiBo

 */

#ifndef	 _authorization_h_
#define  _authorization_h_

#include <string.h>
#include "stkhead.h"
#include "fileAction.h"
#include "ceph.h"
#include "typedef.h"


//----------	公话卡文件	--------------------------------------------------------------
#define	  GH_Key_File			0x6FE0		//公话卡主密钥
#define   GH_DEBUG_File 		0x5FE0 		//公话卡debug文件
#define   GH_SWITCH_FILE		0x5F01		//公话卡开关文件，MF下

#define		GH_TransKey_File	0x6FE1		//广东公话卡传输密钥文件,16字节的传输密钥
#define		GH_ActiveNum		0x6FE2		//广东公话卡特服号码文件，6F3A文件编码格式
#define		GH_KeyStatus		0x6FE3		//广东公话卡认证密钥状态标识,验证码及状态	8byte	


#define 	Random_varID 	0x01
#define 	Response_varID 	0x02

#define		D_GH_Display1	0x71
#define		D_GH_Display2	0x72
#define		D_GH_Display3	0x73
#define		D_GH_Refresh	0x74


//==========	公话卡全局变量	=========================================
extern BYTE	 xdata   G_Authorization_Status;	//鉴权标志位

extern BYTE  xdata	 G_ReadLOCI;


//==========	提示语	=============================================================
extern BYTE code GHDisplayText1[];

extern BYTE code GHResultDisplay1[];

extern BYTE code GHResultDisplay2[];

extern BYTE code GHResultDisplay3[];


//=========		函数声明	=====================================================
extern BYTE  EncryptData(BYTE key_offset);

extern void  Authorization (void);

extern void GHActiveCall(void);

extern BYTE GHActiveProcess(BYTE udPos);

extern void GHOpenProcess(void);

extern BYTE CompareGHCardAuth(void);


#endif
