/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
		
			V1.1 修改计算xdout时的异或偏移错误

 */
#include 	"uicc_main.h"
//#include 	"main.h"
#include 	"fileSystem.h"
#include 	"fileAction.h"
#include 	"gsmpublic.h"
//#include 	"resume.h"
#include 	"Authentication.h"
#include    "stkhead.h"
#include 	<string.h>			 
#include <uicc_error.h>

#if UICC__AUTH__XOR

/*
 *dexcription :this function is used for USIM XOR 鉴权算法
*/
BYTE USIMXORAuthenticate(void)
{
	UWORD idata fileAddr;
	BYTE  idata result, status;

	BYTE idata i;
	BYTE xdata xdout[16],cdout[16];
	BYTE xdata res[16], ck[16], ik[16],ak[6];
	BYTE xdata sqn[6];
	BYTE xdata mac[8];

	fileAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum], EF_KI);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadLFBuf(fileAddr, 0x01, 0x00, &RecData[100], 16);		//读出鉴权的KI

	for(i=0; i<16; i++)
	{
		xdout[i] = RAND[i] ^ RecData[100+i] ;   //计算KI和随机数的异或结果
	}

	fileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum], EF_PRIVATE_AUTHCFG);

	if(fileAddr== FALSE)
	{
		return FALSE;
	}

	ReadTFBuf(fileAddr, 0x6E, &RecData[226], 0x01);			//读出鉴权用的常数字段OPMode，OP，R，C等

	if((AMF[0] == 0xFF)&&(AMF[1] == 0xFF))  //重同步
	{
		for(i=0;i<6;i++)
		{
		   ak[i]=xdout[i+3];
		}
		for(i=0;i<6;i++)
		{
			sqn[i] = AUTH[i] ^ ak[i] ;      //还原SQN内容

			cdout[i] = sqn[i];
		}
		cdout[6] = 0x00; cdout[7] = 0x00;

		for(i=0;i<8;i++)		
		{
		    mac[i]=xdout[i]^cdout[i];	    //计算mac-s
		}

		for(i=0;i<6;i++)				   //得到auts
		{
			RecData[i+2]=sqn[i]^ak[i];		
		}
		for(;i<14;i++)
		{
		    RecData[i+2]=mac[i-6];
		}

		RecData[0] = 0xDC;
		RecData[1] = 0x0E;
		return 16;
	}
	else
	{
		for(i=0;i<16;i++)    //suggested the length of RES is 128 bits
		{
			res[i]=xdout[i];
		}
		
		for(i=0;i<15;i++)		
		{		
		    ck[i] = xdout[i+1];
		}
        ck[i] = xdout[0];

		for(i=0;i<14;i++)
		{		
		    ik[i]=xdout[i+2];	
		}

        ik[i++]=xdout[0];ik[i]=xdout[1];
		
		for(i=0;i<6;i++)
		{
			ak[i]=xdout[i+3];
		}

	  	for(i=0;i<6;i++)
		{
			sqn[i] = AUTH[i] ^ ak[i] ;      //还原SQN内容

			cdout[i] = sqn[i];
		}

		cdout[6] = AMF[0]; cdout[7] = AMF[1];

		for(i=0;i<8;i++)		
		{
		    mac[i]=xdout[i]^cdout[i];	
		}

		if(ecpMem_Compare(&mac[0], &MAC[0], 8) != EQUAL)
		{
			return FALSE;
		}

		RecData[0] = 0xDB;
		RecData[1] = RESLGTH;
		ecpMem_Copy(&RecData[2], &res[0], RESLGTH);    //得到Res内容

		RecData[2+RESLGTH] = 16;
		ecpMem_Copy(&RecData[3+RESLGTH], ck, 16);      //得到CK内容

		RecData[19+RESLGTH] = 16;
		ecpMem_Copy(&RecData[20+RESLGTH], ik, 16);      //得到ik内容

		status = 0x00;

		fileAddr = SelectTFFile(G_DF_ID_USIM, EF_UST);		//判断EF_UST的内容，文件存在ADF下，暂时放在MF下，未完成！

		if(fileAddr != FALSE)
		{
			ReadTFBuf(fileAddr, 0x03, &status, sizeof(BYTE));
		}
		
		if((status&0x04) == 0x04)           //得到kc
		{
			RecData[36+RESLGTH] = 0x08;

			for(result=0; result<8; result++)
			{
				RecData[37+RESLGTH+result] =  RecData[3+RESLGTH+result] ^ RecData[3+RESLGTH+result+8] ^ RecData[20+RESLGTH+result] ^ RecData[20+RESLGTH+result+8];
			}

			return 45+RESLGTH;
		}
		else
		{
			return 36+RESLGTH;
		}	
	}
}
/*
*/
BYTE GSMXORA3A8(void)
{
	BYTE idata i;
	UWORD idata fileAddr;

	fileAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum], EF_KI);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	//ReadLFBuf(fileAddr,0x01,0x00,(BYTE *)&RecData[0x100], 0x10); //read Ki
	ReadLFBuf(fileAddr,0x01,0x00,(BYTE *)&RecData[100], 0x10); //read Ki

	for (i=0x00; i<0x10; i++)
	{
		//tempBuf[i] = RAND[i] ^ RecData[i+0x100];
		tempBuf[i] = RAND[i] ^ RecData[i+100];
	}

	RecData[0] = 0x04;
	ecpMem_Copy(&RecData[1],&tempBuf[0],0x04);
	RecData[5] = 0x08;
	ecpMem_Copy(&RecData[6],&tempBuf[4],0x08);

	return 0x0E;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 UsimAuthentication
  
  Command format:    00    88   00   XX   XX
  
  Data    content:   UICC authentication data from ME                                                       
                                                                                                                                                
  Description:       This function is for UICC authentication

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/

//void UsimAuthenticationXor(void)
//{
//	BYTE idata result, mode;
//	UWORD xdata fileAddr;

//	if(P2 == 0x80)				//GSM Security context
//	{
//		if(P3 != 0x11)
//		{
//		 	P3Error();														//Send 67 00
//			return;
//		}

//		//check "GSM security context" available or not(from "USIM Service Table")
//		result = 0x00;
//		
//		fileAddr = SelectTFFile( G_CurrentAdfName[G_ChannelNum], EF_UST);

//		if(fileAddr != FALSE)
//		{
//			ReadTFBuf(fileAddr, 0x04, &result, sizeof(BYTE));
//		}

//		if ((result&0x20) != 0x20)
//		{
//			GSMSecurityContextNotSupported();								//Send 9864

//			return;
//		}

//		mode = D_AUTHMODE_2G;
//	}
//	else if(P2 == 0x81)			//3G Security context 
//	{
//		if(P3 != 0x22)
//		{
//			P3Error();														//Send 67 00
//			return;
//		}

//		mode = D_AUTHMODE_3G;
//	}
//	else
//	{
//		P1P2Error();														//Send 6B 00
//		return;
//	}

//	ReceiveData();
//	
//	if(mode == D_AUTHMODE_2G)
//	{
//		result = GSMXORA3A8();
//	}
//	else
//	{
//		result = USIMXORAuthenticate();
//	}
//	
//	if(result == FALSE)
//	{
//		AuthenticateError();
//	}
//	else
//	{
//		CommandFinish(result);
//	}
//}
/*
*/


#endif
