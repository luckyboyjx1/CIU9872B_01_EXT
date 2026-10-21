/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.1

 * MODEL: 	All model

 * RELEASE:	2008-5-13

 * AUTHER:	徐庆盛

 * HISTORY: V1.0 创建源文件

	    	V1.1 修改OTASendSMS()：修改当发送的数据DCS为“00”时，UDL字段错误；修改OTALaunchBrowser()：修改承载列表部分长度一直为00的错误。

			V1.2 修改OTASendSMS()：修改压缩7BIT编码与非压缩7BIT编码的UDL字段的值
	
			V1.3 增加宏ATMEL,消除编译器警告

			V1.4 修改函数OTACallUpLinkCommand，修改将应用下载上行命令的应用大小修改为应用最小所需空间0x05

			V1.5 修改OTAPATCH接口
*/


#include "string.h"
#include "fileSystem.h"
#include "fileAction.h"
#include "ceph.h"
#include "stkhead.h"
#include "OTA3Head.h"
#include  "gsmpublic.h"




#ifdef OTA

UWORD 	xdata  G_AppFileAddr;
UWORD 	xdata  G_ServerListFileAddr;			//服务器列表文件地址
UWORD 	xdata  G_ByteCodeAddr;
BYTE 	xdata  G_HandleResponse;				//解释器状态定义4:goto BackMove Finish;3:退出STK;2:退出;1:取响应;0:continue
BYTE 	xdata  G_Pid[3];
BYTE 	xdata  G_Cpid[3];
BYTE 	xdata  G_Command;
BYTE 	xdata  G_CmdIndex;
WORD  	xdata  G_AppOffset;						//应用脚本文件偏移
WORD	xdata  G_STKAppOffset;					//应用脚本文件偏移(STK时)
BYTE 	xdata  G_OpenChannel;
BYTE 	xdata  G_BIPChannelId;
BYTE 	xdata  G_BIPDataLgth;
BYTE 	xdata  G_ChannelStatusID;   			//通道状态事件中的通道ID
BYTE    xdata  G_CounterType;
BYTE	xdata  G_ResponseMode;					//发送上行命令时承载方式的模式:若服务器主动发起的交互过程，必须对此变量赋值
BYTE	xdata  G_IsUpdateEventList;				//是否需要更新事件列表:TRUE(更新)，FALSE(不更新)
BYTE 	xdata  G_EventAID[3];
BYTE    xdata  G_EventTag;
BYTE 	xdata  G_PopUpAID[3];					//弹出应用的应用索引
BYTE    xdata  G_MTPerformResult;               //保存MT命令执行结果(列表更新,应用推荐,应用全部删除)
PathInfo xdata FilePathInfo;
BYTE 	xdata  G_DeleteItem;
BYTE 	xdata  G_LevelApp;  					// 根据此变量判断删除的目录中是否有一级应用



BYTE	xdata G_PPoint;
BYTE 	xdata G_PFlowStack[MaxProcess][4];  //保存当前过程的AID，CmdIndex
UWORD 	xdata G_PAddrStack[MaxProcess];     //保存当前过程的Addr
//***********************************************************************************************************
//*
//* Application Process implementation for OTA2
//*
//***********************************************************************************************************
/*
*@description :获取当前命令序号的偏移地址或判断当前跳转的命令序号是否有效
*
*@param        :offset :偏移地址
*@param        :mode  :0x00- 获取当前命令序号的偏移地址 0x01-判断当前命令序号是否有效
*
*@returns     :offset/FALSE
*
*@history     :create by xyz 20100629@9:10
*/
WORD OTAFindCmd(WORD offset,BYTE mode)
{
	BYTE idata length,i;
	WORD xdata allLength,sumLength;
	BYTE xdata p[5];

	ReadTFBuf(G_AppFileAddr,offset,&p[0],sizeof(p));
	
	if(mode == 0x00)
	{
		if(memcmp(&G_Cpid[0],&p[0],sizeof(G_Cpid))!=EQUAL)
		{
			return FALSE;
		}
	}
			
	offset = offset + 3;

	allLength = p[3];   //规范OTA ，PID长度为1BYTES

	allLength = (((WORD)p[3]<<0x08)&0xFF00) + p[4];  //广东OTA，PID长度为2BYTES

	offset++;

	sumLength = 1;

	for(i=1; i<G_CmdIndex; i++)
	{
		if(sumLength < allLength)
		{
			ReadTFBuf(G_AppFileAddr,(offset + sumLength) +1,&length,1);

			sumLength = (sumLength + length) + 2;
		}
		else
		{
			if(G_CmdIndex == 0xFF)					//查找process中的最后一个cmd
			{
				sumLength = (sumLength - length) - 2;

				G_CmdIndex = i-1;

				return (offset + sumLength);
			}
			else
			{
				return FALSE;
			}
		}
	}

	offset = offset + sumLength ;

	if(sumLength > allLength)
	{
		return FALSE;
	}

	return offset;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAFindProcess

  input parameter:   fileAddr

  output parameter:	 UWORD

  description:       查找指定的pid的偏移值

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
WORD OTAFindProcess(void)
{
	BYTE idata varSize[3];
	BYTE idata length;
	WORD idata allLength;
	WORD idata pidLgth,appsize;
	BYTE xdata pidLocal[5];

	ReadTFBuf(G_AppFileAddr,G_STKAppOffset+3,&varSize[0],3);		//varSize[0],[1]该应用的的大小,varSize[2]该应用的alphaIdentity的长度

	appsize = ((varSize[0]<<8)&0xFF00)|varSize[1];

	allLength = (G_STKAppOffset + varSize[2]) + 6;

	for(length = 0xFD;length >= 0xFA;length = length-3)
	{
		ReadTFBuf(G_AppFileAddr,allLength,&varSize[0],3);

		if(varSize[0] != length)
		{
			return FALSE;
		}
	
		pidLgth = ((varSize[1]<<8)&0xFF00)|varSize[2];
	
		allLength = (allLength + pidLgth) + 3;
	}
  
	ReadTFBuf(G_AppFileAddr,allLength,&pidLocal[0],5);

	while((memcmp(&G_Pid[0],&pidLocal[0],3) != EQUAL)&&(allLength < (G_STKAppOffset+appsize)))	//若不是要查找的pid，继续往下找
	{
		allLength += 5 + ((pidLocal[3]<<8 )& 0xFF00) + (pidLocal[4] & 0x00FF);

		ReadTFBuf(G_AppFileAddr, allLength, &pidLocal[0], 5);
	}
	if(allLength >= (G_STKAppOffset+appsize))
	{
		return FALSE;
	}
	else
	{
		return allLength;
	}
}

/*
*@description:定位OTA应用变量或OTA空间分配变量在应用文件中的偏移
*
*@param       :idName:需定位的ID
*@param       :initialIndex   :0x20-定位OTA应用变量，0xE0-定位OTA空间分配变量 ，即变量的引用起始索引
*
*@returns    :FALSE/offset:当前变量的偏移地址
*
*@history    :creat by xyz 20100629@13:43
*/
WORD OTALocateVarConstPos(BYTE idName,BYTE initialIndex)
{
	BYTE idata tmp[3];
	BYTE idata vcount;
	WORD idata pos;

	ReadTFBuf(G_AppFileAddr,G_STKAppOffset+5, &tmp[0], sizeof(BYTE));	// aid + app size + len of app name

	pos = tmp[0] + 6;

	ReadTFBuf(G_AppFileAddr,G_STKAppOffset+pos, tmp, sizeof(tmp));		// Constant Variable + length of constant variable
	
	pos += 3 ;
	
	if (tmp[0] != 0xFD)
	{
		return FALSE;
	}
	
	if(initialIndex == 0xE0)
	{
		pos = pos + ((tmp[1]<<8)&0xFF00) + tmp[2] ;

		ReadTFBuf(G_AppFileAddr, G_STKAppOffset+pos, tmp, sizeof(tmp));		//Variable + length of variable

		pos += 3 ;

		if((tmp[0] != 0xFA))
		{
			return FALSE;
		}
	}

	if(((tmp[1] == 0x00)&&(tmp[2] == 0x00)))
	{
		return FALSE;
	}

	ReadTFBuf(G_AppFileAddr, G_STKAppOffset+pos, &vcount, sizeof(BYTE));	// variable count

	pos ++ ;

	tmp[1] = initialIndex;

	for (tmp[2] = 0; tmp[2] < vcount; tmp[2] ++)
	{
		ReadTFBuf(G_AppFileAddr,G_STKAppOffset+pos,&tmp[0], sizeof(BYTE));

		if (idName == tmp[1])
		{
			return pos;
		}

		pos = pos + tmp[0] + 1;

		tmp[1] ++;
	}

	return FALSE;
}

 /*
*@description :查找特定的OTA应用变量或空间变量，并将其变量数据拷贝到指定的buf里
*
*@param        :id :特定的OTA应用变量或空间变量索引
*@param        :des :buf(V buffer)
*@param        :initialIndex :应用变量或OTA空间变量的起始索引（0x20或0XE0）
*
@return        :FALSE/变更数据的实际长度
*
*@history    :creat by xuyz@20100629 14:26
*/
BYTE OTASeekConstVar(BYTE id, BYTE * des,BYTE initialIndex)
{
	BYTE idata flag;
	WORD idata pos;
	BYTE idata tmp[2];

	pos = OTALocateVarConstPos(id,initialIndex);

	if( pos == FALSE )
	{
		return FALSE;
	}

	ReadTFBuf(G_AppFileAddr, G_STKAppOffset+pos, &tmp[0], sizeof(tmp));	   //tmp[0] as Length or maxLength tmp[1] as OTA空间变量实际长度

	flag = 0x00;
	
	if(initialIndex == 0xE0)
	{
		flag = 0x01;
	}
	
	if(des != NULL)
	{
		ReadTFBuf(G_AppFileAddr, G_STKAppOffset+pos+1+flag, des, tmp[0+flag]);
	}

	return tmp[0+flag];
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTA2PrepareNextPID

  input parameter:   DWORD G_Pid

  output parameter:	 DWORD

  description:       Prepare next process id

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAFindNextPID(BYTE pidLocal[3])
{
	BYTE idata i,item;

	if ((pidLocal[2]&0x0F) != 0)
	{
		return;
	}

	item = GetResponseItemIdentifier();

	for(i=0;i<6;i++)
	{
		if((pidLocal[i/2]&0xF0)==0)
		{
			pidLocal[i/2] |= ((item << 4)&0xF0);

			break;
		}

		else if((pidLocal[i/2]&0x0F) == 0)
		{
			pidLocal[i/2] |= (item & 0x0F);

			break;
		}
		else	//for baihe-test
		{;}
	}
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTA2PreparePreviousPID

  input parameter:   DWORD G_Pid

  output parameter:	 DWORD

  description:       Prepare Previous PID

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAFindPreviousPID(BYTE pidLocal[3])
{
	BYTE idata i;

	if(pidLocal[0] == 0)
	{
		memset(pidLocal, 0x00, 3);

		return;
	}

	for(i=0;i<6;i++)
	{
		if((pidLocal[i/2]&0xF0) == 0)
		{
			pidLocal[(i-1)/2] &= 0xF0;

			break;
		}
		else if((pidLocal[i/2]&0x0F) == 0)
		{
			pidLocal[i/2] &= 0x0F;

			break;
		}
		else	//for baihe-test
		{;}
	}
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAGetVariable

  input parameter:   index: Editbuf index
					 des: Data buffer(V structure)

  output parameter:	 FALSE/DATA length

  description:       OTA2 Copy Variable

  test:				 Zhuli,Caibo

  status:            OK

 -------------------------------------------------------------------------------------------------------------------*/
BYTE OTAGetVariable(BYTE index,BYTE * des)
{
	if(index < 0x20)
	{
		return ReadVarible(index,des);
	}
	else if(index < 0xE0)
	{
		return OTASeekConstVar(index,des,0x20);
	}
	else
	{
		return OTASeekConstVar(index,des,0xE0);
	}
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAGetText

  Input parameter:   cmdType, cmd, byteCodeAddr, ch

  Output parameter:	 void

  Description:       Concate text from constant and variable

  Coder:

  Test:				 xuzhichu

  Status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE OTAGetText(UWORD byteCodeAddr,BYTE tag,BYTE * ch)
{
	BYTE idata tmp[2];
	BYTE idata i,len;

	ReadEeprom(&tmp[0],byteCodeAddr,sizeof(tmp));	  	//tmp[0] as  变量个数

	len = 0x00;

	for(i = 0; i < tmp[0]; i ++)
	{
		ReadEeprom(&tmp[1],byteCodeAddr+1+i,sizeof(BYTE));

		len =len + OTAGetVariable(tmp[1], &ch[2+len]);
	}
 
	if(((tag&0x7F) == 0x0D)&&(len == 0x01))   //仅存在DCS字节时，不拼装text数据
	{
		ch[1] = 0x00;
	}
	else
	{
		ch[1] = len;
	}

	ch[0] = tag;              //变量个数为0时，返回tag 0x00

	return tmp[0];
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAProvideLocalInformation

  Input parameter:   void

  Output parameter:	 void

  Description:       OTA provide local information G_Command

  Test:				 xuzhichu

  Status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE code tagArray[]={
	TLV_LOCATIONINFOMRATION,
	TLV_IMEI,
	TLV_NETWORKMEASUREMENTRESULT,
	TLV_DATETIMEANDTIMEZONE,
	TLV_LANGUAGE,
	TLV_TIMINGADVANCE};

void OTAProvideLocalInformation(void)
{
	BYTE idata result;

	ReadEeprom(&tempBuf[0],G_ByteCodeAddr+1,0x03);

	InitProactiveCommand(CMD_PROVIDELOCALINFORMATION,tempBuf[1],DEV_ME);

	result = SendProactiveCommand();

	if (result == CMD_Success)
	{
		result = FindTag(&RecData[0], tagArray[tempBuf[1]], P3);

		WriteVariable(tempBuf[2], &RecData[result]);
	}
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTARefresh

  input parameter:   void

  output parameter:	 void

  description:       OTA2 Refresh G_Command

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTARefresh(void)
{
	ReadEeprom(&tempBuf[0],G_ByteCodeAddr+1,0x02);

	InitProactiveCommand(CMD_REFRESH,tempBuf[1],DEV_ME);		//tempBuf[1] as commandQualifier

	if(tempBuf[0] > 1)							//tempBuf[0] as length
	{
		ReadEeprom(&RecData[0],G_ByteCodeAddr+3,tempBuf[0]-1);

		AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);
	}

	SendProactiveCommand();
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTASendSMS

  input parameter:   void

  output parameter:	 void

  description:       OTA2 Send SMS

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTASendSMS(void)
{
	BYTE idata pos,phoneLgth,tempLen;
	BYTE idata tmp[2];
	BYTE xdata smsp[0x10];
	
	ReadEeprom(&tmp[0],G_ByteCodeAddr+1,sizeof(tmp));		//tmp[0] as length

	InitProactiveCommand(CMD_SENDSHORTMESSAGE,tmp[1],DEV_NETWORD);		//tmp[1] as commandQualifier
	
	ReadEeprom(&tempBuf[0],G_ByteCodeAddr+3,tmp[0] -1);		//tempBuf[0] as PID tempBuf[1] as dcs
	
	pos = 0x02;
	
	if(tempBuf[pos]== 0x07)   //存在短信中心标示和短信中心变量
	{
		smsp[1] = OTAGetVariable(tempBuf[pos+1], &smsp[2]);
		
		pos = pos+2;
	}
	else
	{
		LoadSCAddress(&smsp[1]);
	}

	if(tempBuf[pos] == 0x05)   //存在@标示
	{
		OTAGetText(G_ByteCodeAddr+pos +4 ,0x85,&RecData[0]);

		AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);

		pos = pos + tempBuf[pos+1] + 2;
	}
	
	if(smsp[1] != 0x00)
	{
		smsp[0] = 0x86;

		AddTLV(&ResData[1], (BYTE CONST *)&smsp[0]);
	}
	
	if(tempBuf[pos] != 0x06)  //地址标示不存在
	{
		return;
	}
	
	RecData[4] = OTAGetVariable(tempBuf[pos+1], &RecData[5]);
	
	phoneLgth = GetBCDCount(&RecData[5], RecData[4]);
	
	pos = pos + 2;
	
	if(tempBuf[pos] != 0x0D)
	{
		return;
	}
	
	OTAGetText(G_ByteCodeAddr+pos +4,0x00,&RecData[49]);
	
	tmp[0] = RecData[50];
	
	RecData[0] = 0x8B;

	RecData[2] = 0x01;

	RecData[3] = 0x00;

	RecData[1] = RecData[4] + 3;

	RecData[4] = phoneLgth;

	RecData[RecData[1]+2] = tempBuf[0];	    //PID

	RecData[RecData[1]+3] = tempBuf[1];		//DCS

	RecData[1] += 2;

	if ((tempBuf[1] == 0x00)&&((tmp[1]&0x01) != 0x01))			//不要求ME打包的7BIT编码						
	{
		tempLen = (RecData[50] * 8) / 7;					   //7bit编码的字符个数

		if(((RecData[50+RecData[50]]&0xFE) == 0x00) && ((tempLen%8) == 0x00))
		{
			tempLen--;
		}

		RecData[50] = tempLen;
	}

	memcpy(&RecData[RecData[1]+2],&RecData[50],tmp[0]+1);

	RecData[1] = RecData[1] + tmp[0] + 1;

	AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);

	SendProactiveCommand();

	IF_OTASendSMS();
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	OTASendSsOrUssd

  input parameter:  cmdType

 					tlvTag
  output parameter:

  description:

  test:

  status:		new

 -------------------------------------------------------------------------------------------------------------------*/
void OTASendSsOrUssd(BYTE cmdType, BYTE tlvTag)
{
	BYTE idata pos;
	BYTE idata tmp[2];
	
	ReadEeprom(&tmp[0],G_ByteCodeAddr+1,sizeof(tmp));   //tmp[0] as length ，tmp[1] as 命令限定符
	
	InitProactiveCommand(cmdType,tmp[1],DEV_NETWORD);
	
	ReadEeprom(&tempBuf[0],G_ByteCodeAddr+3, tmp[0] -1);   //后续数据保存在tempBuf缓冲区中
	
	pos= 0x00;
	
	if(tempBuf[pos] == 0x05)
	{
		OTAGetText(G_ByteCodeAddr+pos+4,0x85,&RecData[0]);

		AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);

		pos = pos + tempBuf[pos+1] + 2;
	}
	
	if(tempBuf[pos]  != (tlvTag&0x7F))
	{
		return;
	}
	
	OTAGetText(G_ByteCodeAddr+pos+4,tlvTag,&RecData[0]);

	AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);

	SendProactiveCommand();
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTASetupCall

  input parameter:   void

  output parameter:	 void

  description:       OTA2 Setup Call

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTASetupCall(void)
{
	BYTE idata result;
	BYTE idata pos;
	BYTE idata tmp[2];
	
	ReadEeprom(&tmp[0],G_ByteCodeAddr+1,sizeof(tmp)); //tmp[0]  as length ,tmp[1] as 命令限定符
	
	InitProactiveCommand(CMD_SETUPCALL,tmp[1],DEV_NETWORD);	//tmp[1] as commandQualifier
	
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+3, tmp[0]-1);		   //将后续数据存储在tempBuf缓冲区中
	
	result = 0x00;
	
	for(pos =0x00;pos<(tmp[0]-1);)
	{
		switch(tempBuf[pos])
		{
			case 0x05:
			{
				OTAGetText(G_ByteCodeAddr+pos+4,0x85,&RecData[0]);
			
				AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);
			
				pos = pos +tempBuf[pos+1]+2;
			
				break;
		     }
			case 0x06:
			{
				result ++;
				
				RecData[1] = OTAGetVariable(tempBuf[pos+1], &RecData[2]);
	
				RecData[0] = 0x86;
	
				AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);

				pos = pos + 2;
				
				break;
			}
			default:      //存在其余tag
				return;
		}
	}

	if(result != 0x01)  //不存在tag 0x06
	{
		return;
	}
	
	SendProactiveCommand();
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTA2PlayTone

  input parameter:   void

  output parameter:	 void

  description:       OTA2 Play Tone G_Command

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAPlayTone(void)
{
	BYTE idata length;
	BYTE idata pos;
	
	InitProactiveCommand(CMD_PLAYTONE,0x00,DEV_EARPIECE);

	ReadEeprom(&length,G_ByteCodeAddr+1,sizeof(BYTE));	//length as 后续数据长度

	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+2, length);		   //将后续数据存储在tempBuf缓冲区中

	for(pos=0x00; pos < length;)
	{
		switch(tempBuf[pos])
		{
			case 0x05:
			{
				OTAGetText(G_ByteCodeAddr+3,0x85,&RecData[0]);

				AddTLV(&ResData[1],&RecData[0]);

				pos = pos + tempBuf[pos+1] + 2;
				
				break;
			}
			case 0x0E:
			{
				RecData[0] = 0x8E;
		
				RecData[1] = 0x01;
			
				RecData[2] = tempBuf[pos+1];

				AddTLV(&ResData[1],&RecData[0]);

				pos = pos + 2;
				
				break;
			}
			case 0x04:
			{
				tempBuf[pos] = 0x84;

				AddTLV(&ResData[1],&tempBuf[pos]);

				pos = pos + 4;
				
				break;
			}
			default:
				break;
		}
	}

	SendProactiveCommand();
}
/*
*@descrption : 实现displaytext命令结构
*
*@param      : offset : 命令限定符的相对于G_ByteCodeAddr的起始偏移
*
*@return     :void
*
*@history    :create by xyz@100705 14:00
*/
void OTADisplayText(BYTE offset)
{
	BYTE idata tmp[2];

	ReadEeprom(&tmp[0],G_ByteCodeAddr+offset,sizeof(tmp));

	InitProactiveCommand(CMD_DISPLAYTEXT,tmp[0],DEV_DISPLAY);		//tmp[0] as commandQualifier

	if(tmp[1] != 0x0D)							//tmp[1] as tag
	{
		return;
	}

	OTAGetText(G_ByteCodeAddr + offset + 2,0x8D,&RecData[0]);

	if((RecData[1] > 1) && (RecData[2] == 0x80))
	{
		RecData[2] = 0x08 ;
	}

	AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);

	SendProactiveCommand();
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAGetInput

  input parameter:   void

  output parameter:	 void

  description:       OTA2 Get Input G_Command

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAGetInput(void)
{
	BYTE idata pos,maxLgth,result;
	BYTE idata tmp[2];
	
	ReadEeprom(&tmp[0],G_ByteCodeAddr+1,sizeof(tmp));  //tmp[0] as length,tmp[1] as 命令限定符
	
	if((ValueBit(G_SystemStatus,B_meSurportUcs2)==FALSE)&&((tmp[1]&0x02) == 0x02))
	{
		tmp[1] = tmp[1]&0xFD ;				//tmp[1] as commandQualifier

		SetBit(G_SystemStatus,B_needChange);
	}
	else
	{
		ClearBit(G_SystemStatus,B_needChange);
	}
	
	InitProactiveCommand(CMD_GETINPUT,tmp[1],DEV_ME);	//tmp[1] as commandQualifier
	
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+3, tmp[0]-1);    //tempBuf[0] as 全局缓冲区，....
	
	pos = 0x01;
	
	if((tempBuf[pos]&0x7F) != 0x0D)
	{
		return;
	}
	RecData[0] = tempBuf[pos];
	
	if(tempBuf[pos+1] == 0x81)
	{
	   maxLgth = 0x01;
	}
	else
	{
		maxLgth = 0x00;
	}

	memcpy(&RecData[1],&tempBuf[pos+1],tempBuf[pos+1+maxLgth] + 1 + maxLgth);
	
	pos = pos + tempBuf[pos+1+maxLgth]  + 2 + maxLgth;
	
	AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);
	
	RecData[0] = 0x91;

	RecData[1] = 0x02;
	
	RecData[2] = tempBuf[pos];
	
	RecData[3] = tempBuf[pos+1];

	AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);
	
	maxLgth = RecData[3];
	
	pos = pos+2;
	
	if(  pos < (tmp[0] -1)  )
	{
		if( (tempBuf[pos]& 0x7F)  != 0x0D )
		{
			return;
		}
		
		OTAGetText(G_ByteCodeAddr+pos+4,0x97,&RecData[0]);

		AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);
	}
	
	result = SendProactiveCommand();

	if(result == CMD_Success)
	{
	   GetResponseTextString(tmp[1],&ResData[0],maxLgth);

	   WriteVariable(tempBuf[0],&ResData[0]);
	}
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTASelectItem

  input parameter:   void

  output parameter:	 void

  description:       OTA2 Select Item G_Command

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTASelectItem(void)
{
	BYTE idata tmp[2],pos;
	UWORD xdata usedOffset;
	
	ReadEeprom(&tmp[0],G_ByteCodeAddr+1,sizeof(tmp));  //tmp[0] as length tmp[1] as 命令限定符
	
	InitProactiveCommand(CMD_SELECTITEM,tmp[1],DEV_ME);
	
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+3, tmp[0]-1);		   //将后续数据存储在tempBuf缓冲区中
	
	pos = 0x00;
	
	if((tempBuf[pos] &0x7F) == 0x05)
	{
		OTAGetText(G_ByteCodeAddr+pos+4,0x85,&RecData[0]);

		AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);
		
		pos = pos + tempBuf[pos+1] +2;
	}
	
	while (pos < (tmp[0] -1))
	{
		RecData[0] = 0x8F;
		
		memcpy(&RecData[1], &tempBuf[pos+1],tempBuf[pos+1]+1);
		
		AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);

		pos = pos + tempBuf[pos+1] + 2;
	}
	
	pos = SendProactiveCommand();

	if (pos == CMD_Success)
	{
		G_CmdIndex ++;

		usedOffset = OTAFindProcess();
 
		usedOffset = OTAFindCmd(usedOffset,0x00);

		if (usedOffset == FALSE)
		{
			OTAFindNextPID(G_Pid);

			G_CmdIndex = 0;
		}
		else
		{
			G_ByteCodeAddr = usedOffset + getFileContentAddr(G_AppFileAddr);					 
			G_CmdIndex -- ;
		}
	}
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTALaunchBrowser

  input parameter:   void

  output parameter:	 void

  description:       OTA2 Launch  Browser G_Command

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTALaunchBrowser(void)
{
	BYTE idata pos,tagStatus;
	BYTE idata tmp[2];
	
	ReadEeprom(&tmp[0],G_ByteCodeAddr+1,sizeof(tmp));  //tmp[0] as length tmp[1] as 命令限定符
	
	InitProactiveCommand(CMD_LAUNCHBROWSER,tmp[1],DEV_ME);		//tmp[1] as commandQualifier
	
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+3, tmp[0] - 1);		   //将后续数据存储在tempBuf缓冲区中
	
	tagStatus = 0x00;
	
	for(pos = 0x00;  pos<(tmp[0] -1) ; )
	{
		switch((tempBuf[pos]&0x7F))
		{
			case 0x30:
				RecData[0] = 0xB0;

				RecData[1] = 0x01;
			
				RecData[2] = tempBuf[pos+1]; 

				AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);
			
				pos = pos +2;
			
				break;
			
			case 0x32:
				RecData[0] = 0xB2;
				
				memcpy(&RecData[1] , &tempBuf[pos+1], tempBuf[pos+1] +1);
			
				AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);
			
				pos = pos +tempBuf[pos+1] +2;
			
				break;
			
			case 0x31:
				tagStatus ++;   //必须存在 0x31 tag
			case 0x0D:
			case 0x05:
				OTAGetText(G_ByteCodeAddr+pos +4,  (tempBuf[pos]|0x80), &RecData[0]);

				AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);
			
				pos = pos + tempBuf[pos+1] +2;
			
				break;
			default:          //存在其余tag ，直接退出
				return;   
				
		}
	}
	
	if(tagStatus != 0x01)   //不存在0x31 tag 
	{
		return ;
	}
	
	SendProactiveCommand();
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAJmp

  input parameter:   void

  output parameter:	 void

  description:       对于出现不存在的PID和命令序号的特殊说明如下：
						1．	如果PID＝FF FF FF，命令序号＝01，则卡片跳转到该应用的二级菜单，即与"应用管理"同级菜单。
						2．	除情况1外，如果跳转的PID或命令序号不存在，则卡片直接退出STK/USAT菜单（即返回状态字9000）

  test:				 xyz

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAJmp(void)
{
	WORD xdata offset,usedOffset;

	ReadEeprom(&G_Pid[0],G_ByteCodeAddr+2,3);

	ReadEeprom(&G_CmdIndex,G_ByteCodeAddr+5,1);

	if ((G_Pid[0] == 0xFF)&&(G_Pid[1] == 0xFF)&&(G_Pid[2] == 0xFF)&&(G_CmdIndex == 0x01))
	{
		G_HandleResponse = 2;						//退出解释器应用
	}
	else
	{
		if((G_Pid[0] == 0x00)&&(G_Pid[1] == 0x00)&&(G_Pid[2] == 0x00)&&(G_CmdIndex == 0x00))					
		{
			G_HandleResponse = 2;										//退出解释器应用
		}
		else if(G_CmdIndex == 0x00)
		{
			G_HandleResponse = 3;
		}
		else
		{
			offset = OTAFindProcess();
		
			if (offset == FALSE)
			{
				G_HandleResponse = 3;							//退出STK,返回9000
				
				return;
			}
	
			usedOffset = OTAFindCmd(offset,0x01);
	
			if (usedOffset == FALSE)
			{
				G_HandleResponse = 3;							//退出STK,返回9000
				
				return;
			}
	
			G_HandleResponse = 0;								//继续执行后续指令
		}
	}
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAOpenChannel

  input parameter:   void

  output parameter:	 void

  description:       OTA3 OpenChannel

  test:				 xyz

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAOpenChannel(void)
{
	BYTE idata tmp[3],pos;
	BYTE idata tagStatus;    //记录当前API函数中是否一定存在 35 39tag
	
	ReadEeprom(&tmp[0],G_ByteCodeAddr+1,sizeof(tmp)); //tmp[0] as length,tmp[1] as qualifier,tmp[2] as bufferId
	
	InitProactiveCommand(CMD_OPENCHANNEL,tmp[1],DEV_ME);
	
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+4, tmp[0]-2);		   //将后续数据存储在tempBuf缓冲区中
	
	tagStatus = 0x00;
	
	for(pos = 0x00; pos< (tmp[0]-2); )
	{
		switch((tempBuf[pos]&0x7F))
		{
			case 0x39:			
				tagStatus ++;
			
				RecData[0] = tempBuf[pos];

				RecData[1] = 0x02;
			
				RecData[2] = tempBuf[pos+1];
			
				RecData[3] = tempBuf[pos+2];

				AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);
			
				pos = pos +3;
			
				break;
			case 0x35:
				tagStatus ++;
		
			case 0x47:
			case 0x3C:
			case 0x3E:
				
				RecData[1] = OTAGetVariable(tempBuf[pos+1],&RecData[2]);

				RecData[0] = tempBuf[pos];

				AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);
			
				pos = pos+2;

				break;

			default: 	  //如果该API
				return;
		}
		
	}
	
	if(tagStatus != 0x02)	   //tag 0x35 0x39必须存在
	{
		return;
	}
	
	pos = SendProactiveCommand();

	if(pos == CMD_Success)
	{
		pos = FindTag(&RecData[0],TLV_CHANNELSTATUS,P3);

		WriteVariable(tmp[2], &RecData[pos]);   //返回打开的通道号状态
	}	
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTACloseChanne;

  input parameter:   void

  output parameter:	 void

  description:       OTA CloseChannel  G_Command

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTACloseChannel(void)
{
	BYTE idata varId,channel;

	ReadEeprom(&varId,G_ByteCodeAddr+2,sizeof(BYTE));

	ReadVarible(varId,&channel);//Edit Buf(0x00-0x1F)

	if((channel& 0x07) == 0x00)
	{
		return ;
	}

	channel = DEVICE_CHANNEL1 + (channel& 0x07) - 1;

	InitProactiveCommand(CMD_CLOSECHANNEL,0x00,channel);   	//init the command details and device iden in command of selec

	SendProactiveCommand();
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAReceiveData;

  input parameter:   void

  output parameter:	 void

  description:       OTAReceiveData  G_Command

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAReceiveData(void)
{
	BYTE idata channel;
	
	ReadEeprom(&tempBuf[0],G_ByteCodeAddr+1,0x05); //tempBuf[0] as length,tempBuf[1] as channelId,tempBuf[2] as bufferId,tempBuf[3] as  0x37 tempBuf[4] as 通道数据长度
	
	ReadVarible(tempBuf[1],&channel);//Edit Buf(0x00-0x1F)
	
	channel = channel & 0x07;

	if(channel == FALSE)	 //没有通道可获得
	{
		return;
	}

	InitProactiveCommand(CMD_RECEIVEDATA, 0x00, (channel+0x20));
	
	if((tempBuf[3]&0x7F) != 0x37)
	{
		return;
	}
	
	RecData[0] = tempBuf[3];
	
	RecData[1] = 0x01;
	
	RecData[2] = tempBuf[4];
	
	AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);

	channel = SendProactiveCommand();

	if(channel == CMD_Success)
	{
		channel = FindTag(&RecData[0],TLV_CHANNELDATA,P3);

		WriteVariable(tempBuf[2], &RecData[channel]);
	}
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTASendData;

  input parameter:   void

  output parameter:	 void

  description:       OTASendData  G_Command

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTASendData(void)
{
	BYTE idata channel;
	
	ReadEeprom(&tempBuf[0],G_ByteCodeAddr+1,0x05); //tempBuf[0] as length,tempBuf[1] as qualifier,tempBuf[2] as channelId,etmpBuf[3] as = 0x36,tempBuf[4]as 通道数据引用
	
	ReadVarible(tempBuf[2],&channel);//Edit Buf(0x00-0x1F)

	if((channel&0x07) == 0x00)    //没有通道可获得
	{
		return;
	}
	
	channel = DEVICE_CHANNEL1 + (channel&0x07) - 1;
	
	InitProactiveCommand(CMD_SENDDATA,tempBuf[1],channel);
	
	if((tempBuf[3]&0x7F)  != 0x36)
	{
		return;
	}
	
	RecData[1] = ReadVarible(tempBuf[4],&RecData[2]); //Edit Buf(0x00-0x1F)

	RecData[0] = tempBuf[3];
	
	AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);

	SendProactiveCommand();

}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAGetChannelStatus;

  input parameter:   void

  output parameter:	 void

  description:       OTAGetChannelStatus  G_Command

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAGetChannelStatus(void)
{
	BYTE idata buffer,result;

	ReadEeprom(&buffer,G_ByteCodeAddr+2,sizeof(BYTE));	//保存从ME返回的信道状态

	InitProactiveCommand(CMD_GETCHANNELSTATUS, 0x00, DEV_ME);		//init the command details and device iden in command of selec

	result = SendProactiveCommand();

	if(result == CMD_Success)
	{
		result = FindTag(&RecData[0], TLV_CHANNELSTATUS, P3);

		if(buffer < 0x20)
		{
			WriteVariable(buffer, &RecData[result]);
		}
	}
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTA3CallUplinkCmd

  input parameter:  	void

  output parameter:	 	void

  description:       	for ota3 call uplink command

  test:			    	Zhuli

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTACallUpLinkCommand(void)
{
	BYTE idata cmdType;
	BYTE xdata aid[5];

	ReadEeprom(&cmdType, G_ByteCodeAddr+2, sizeof(BYTE));

	if((cmdType != 0x09)&&(OTAGetProcessPermission(cmdType) == FALSE))	//判断前一请求是否完成。
	{
		G_HandleResponse = 0x03;
		return;
	}

	switch(cmdType)
	{
		case 0x01:   //应用下载请求
		{
			ReadEeprom(&aid[0], G_ByteCodeAddr+3, 0x03);

			aid[3] = 0x00;

			aid[4] = 0x05;   //因为API未提供应用大小，所以设置应用大小为应用最小所需空间0x05

			OTAMODownloadCmdData(&aid[0], &tempBuf[0]);

			break;
		}
		case 0x09:   //主动获取品牌信息
		{
			tempBuf[0] = 0x02;

			tempBuf[1] = 0x09;
			
			tempBuf[2] = 0x00;
			
			break;
		}
		case 0x0A: //应用开通\关闭请求
		{
			ReadEeprom(&tempBuf[3], G_ByteCodeAddr+3, 4);

			tempBuf[2] = 0x04;

			tempBuf[1] = 0x0A;

			tempBuf[0] = 0x06;

			break;
		}
		default:
		{
			G_HandleResponse = 0x03;
			return;
		}
	}

	OTASendUplinkCommandMessage(1, &tempBuf[0]);

	G_HandleResponse = 0x03;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTACallProcess

  input parameter:  	void

  output parameter:	 	void

  description:       	过程调用函数

  test:			    	xyz
 -------------------------------------------------------------------------------------------------------------------*/
void OTACallProcess(void)
{
	if(G_PPoint >= MaxProcess) // current process excess the maximun process supported of the cos !
	{
		G_HandleResponse = 0x03;

		return;
	}

	G_PAddrStack[G_PPoint] = G_ByteCodeAddr;

	memcpy(&G_PFlowStack[G_PPoint][0], &G_Cpid[0], 0x03);

	G_PFlowStack[G_PPoint][3] = G_CmdIndex;

	G_PPoint += 0x01;

	ReadEeprom(&G_Pid[0], G_ByteCodeAddr + 2, 0x03); // read out the call process id in the ota application

	G_CmdIndex = 1;

	G_HandleResponse = 0; // jump immediately
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTAReturn

  input parameter:  	void

  output parameter:	 	void

  description:       	RETURN FUN

  test:			    	xyz
 -------------------------------------------------------------------------------------------------------------------*/
void OTAReturn(void)
{
	BYTE idata tmp[2];
	BYTE idata returnNum, i;
	BYTE idata returnIndex;

	if(G_PPoint == 0)  // error, can not return in this scenarial
	{
		return ;
	}

	ReadEeprom(&returnNum, G_ByteCodeAddr + 1, 0x01);

	ReadEeprom(&tmp[0], G_PAddrStack[G_PPoint-1] + 5, 0x02) ;  // call process:tmp[0] as input pamarater number;,tmp[1] as out parameter number

	tmp[0] += 8;

 	for(i = 0; i < tmp[1]; i ++) // start to move the editbuffer of the current process to the previous output parameters!
	{
		// read out the output index and input index form the bytecode addr respectivily !
		ReadEeprom(&returnIndex, G_ByteCodeAddr + 2 + i, 0x01);

		tempBuf[0] = ReadVarible(returnIndex, &tempBuf[1]);

		G_PPoint -=1;

		ReadEeprom(&returnIndex,G_PAddrStack[G_PPoint] + tmp[0] + i, 0x01);
		
		if(i >= returnNum)	   //return 返回的参数个数小于call process输出的参数个数,其余参数赋空值
		{
			ClearVariable(returnIndex);
		}
		else	   		       //return 返回的参数个数大于call process输出的参数个数,多余的参数舍弃	   
		{
			WriteVariable(returnIndex, &tempBuf[0]);
		}

		G_PPoint += 1;
	}

	for(i = 0; i < 0x20; i ++)
	{
		if(ReadVarible(i, &tempBuf[0]))
		{
			ClearVariable(i);
		}
	}

	G_PPoint -= 1;

	memcpy(&G_Pid[0], &G_PFlowStack[G_PPoint][0], 0x03);

	G_CmdIndex = G_PFlowStack[G_PPoint][3] + 1;  // go to the next command ^^

	G_HandleResponse = 0; // jump immediately ?
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :		InternalConvertVarID

  input parameter:  		editBufIndex:	the index of the edit buf when run the Call Porcess command

  output parameter:	 	the real index value in the Variable Buffer

  description:                change the edit buf index value

  test:				DK

  status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE InternalConvertVarID(BYTE editBufIndex)
{
	BYTE idata tmp,i;

	if(G_PPoint == 0)  // normal edit buffer index without any process
	{
		return editBufIndex;
	}
	else
	{
		if(editBufIndex<= 0x17)  // normal edit buffer in process
 		{
			return editBufIndex+ G_PPoint * 0x20;
		}
		else if((editBufIndex >= 0x18) && (editBufIndex <= 0x1F)) // input parameter form previous process
		{
			for(i = 1; i <= G_PPoint; i ++)
			{
				tmp = editBufIndex - 0x17;

				ReadEeprom(&editBufIndex, G_PAddrStack[G_PPoint - i] + 7 + tmp, 0x01);

				if(editBufIndex <= 0x17)
				{
					editBufIndex += (G_PPoint -i) * 0x20;

					break;
				}
			}

			return editBufIndex;
		}
		else	//for baihe-test
		{	
			;
			//return editBufIndex;
		}
	}
}




/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAStartRun

  input parameter:   void

  output parameter:	 BYTE

  description:       OTA2 Script Interpreter

  test:

  status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE OTAStartRun(void)
{
	WORD xdata offset,usedOffset;						//process在应用文件中的偏移
	BYTE xdata backmoveInfo[4];
	BYTE idata lastCmd;	
	
	lastCmd = 0;	//for baihe-test						
	
	BackFilePath(&FilePathInfo);	      				//备份当前路径
	
	STKControl = FALSE;
	
	InitVariable();

	memset(&G_Pid[0],0x00,3);

	memset(&G_Cpid[0],0xFF,3);

	G_CmdIndex = 1;

	for(;;)
	{
		G_HandleResponse = 1;

		if(memcmp(&G_Pid[0],&G_Cpid[0],3) != EQUAL)
		{
			offset = OTAFindProcess();

			memcpy(&G_Cpid[0],&G_Pid[0],3);
		}

		if (offset == FALSE)
		{
			if (STKControl == FALSE)
			{
				ResumeFilePath(&FilePathInfo);	      //备份当前路径
			}
			return FALSE;
		}

		usedOffset = OTAFindCmd(offset,0x00);

		if(usedOffset == FALSE)
		{
			G_CmdIndex--;

			goto BackMoveFinish;
		}

		G_ByteCodeAddr = getFileContentAddr(G_AppFileAddr) + usedOffset;

		ReadEeprom(&G_Command,G_ByteCodeAddr,sizeof(BYTE));

		if (IF_OTAExFuntion() == FALSE)
		{
			switch (G_Command)
			{
					case CMD_PLAYTONE:
						OTAPlayTone();
						break;
					case CMD_PROVIDELOCALINFORMATION:
						OTAProvideLocalInformation();
						break;
					case CMD_REFRESH:
						OTARefresh();
						break;
					case CMD_SENDSHORTMESSAGE:
						OTASendSMS();
						break;
					case CMD_SENDSS:
						OTASendSsOrUssd(CMD_SENDSS,TLV_SSSTRING);
						break;
					case CMD_SENDUSSD:
						OTASendSsOrUssd(CMD_SENDUSSD,TLV_USSDSTRING);
						break;
					case CMD_SETUPCALL:
						IF_OtaSetupCall();
						break;
					case CMD_LAUNCHBROWSER:
						OTALaunchBrowser();
						break;
					case CMD_DISPLAYTEXT:
						OTADisplayText(0x02);
						break;
					case CMD_SELECTITEM:
					 	OTASelectItem();
						break;
					case CMD_GETINPUT:
						OTAGetInput();
						break;
					case OTA_BACKWARD:
						G_CmdIndex = G_CmdIndex+1;
	
						if(lastCmd == OTA_BACKWARD)
						{
							goto BackMoveFinish;
						}
						else
						{
							G_HandleResponse = 0;
						}
						break;
					case OTA_JMP:
						OTAJmp();
						break;
					case CMD_OPENCHANNEL:
						OTAOpenChannel();
						break;
					case CMD_CLOSECHANNEL:
						OTACloseChannel();
						break;
					case CMD_RECEIVEDATA:
						OTAReceiveData();
						break;
					case CMD_SENDDATA:
						OTASendData();
						break;
					case CMD_GETCHANNELSTATUS:
						OTAGetChannelStatus();
						break;
					case OTA_CALLPROCESS:
						OTACallProcess();
						break;
					case OTA_RETURN:
						OTAReturn();
						break;
					case OTA_CALLUPLINKCOMMAND:
						OTACallUpLinkCommand();
						break;
					case OTA_EVENTREG:
						OTA3EventRegister();
						break;
#ifdef LOCALOTA
					case OTA_EVENTANALYSE:
						OTA3EventAnalyse();
						break;
#endif
					case OTA_FUNCTIONCALL:
						OTAPlugInFuction();
						break;
					default:
						if (STKControl == FALSE)
						{
							ResumeFilePath(&FilePathInfo);	      //备份当前路径
						}
						return FALSE;
			}
		}


		lastCmd = G_Command;			//2007-12-05

		if(G_HandleResponse == 0)
		{
			continue;
		}
		else if (G_HandleResponse == 1)
		{
			G_HandleResponse = GetResResult();

			if (G_HandleResponse == CMD_Success)
			{
				G_CmdIndex++;
			}
			else if (G_HandleResponse == BackMove) // backward move
			{
BackMoveFinish:
				G_CmdIndex --;				//???

			   	if (G_CmdIndex == 0)
				{
					if ((G_Pid[0] == 0)&&(G_Pid[1] == 0)&&(G_Pid[2] == 0))
					{
						if (STKControl == FALSE)
						{
							ResumeFilePath(&FilePathInfo);	      //备份当前路径
						}
						return 0;
					}
					else
					{
						OTAFindPreviousPID(G_Pid);

						offset = OTAFindProcess();

						G_CmdIndex = 0xFF;

						memcpy(G_Cpid, G_Pid, 3);

						goto BackwardProcess;
					}
				}
				else
				{
BackwardProcess:
					while(G_CmdIndex!=0)
					{
						G_ByteCodeAddr = OTAFindCmd(offset,0x00) + getFileContentAddr(G_AppFileAddr);

						ReadEeprom(&G_Command,G_ByteCodeAddr,sizeof(BYTE));

						if((G_Command == CMD_SELECTITEM)||(G_Command == CMD_GETINPUT)||(G_Command == CMD_DISPLAYTEXT))
						{
							break;
						}
						else if(G_Command == OTA_FUNCTIONCALL)
						{
							ReadEeprom(&G_Command,G_ByteCodeAddr+2,sizeof(BYTE));

							if((G_Command == OTA2_EX_PAGEDISPADN)||(G_Command == OTA2_EX_SETPOSITIONSERVICE)||(G_Command == OTA2_EX_DISPLAYVAR))
							{
								break;
							}
						}
						else if(G_Command == OTA_BACKWARD)
						{
							ReadEeprom(&backmoveInfo[0],G_ByteCodeAddr+2,4);

							memcpy(&G_Pid[0],&backmoveInfo[0],3);

							G_CmdIndex = backmoveInfo[3];

							break;
						}
						else	//for baihe-test
						{;}

					 	G_CmdIndex --;

						lastCmd = G_Command;		//2007-12-05
					}

					if(G_CmdIndex == 0)
					{
						if ((G_Pid[0] == 0)&&(G_Pid[1] == 0)&&(G_Pid[2] == 0))
						{
							if (STKControl == FALSE)
							{
								ResumeFilePath(&FilePathInfo);	      //备份当前路径
							}							
							return 0;
						}
						else
						{
							OTAFindPreviousPID(G_Pid);

							G_CmdIndex = 0xFF;	//最后一个CmdIndex标志
						}
					}
				}
			}
			else
			{
				if (STKControl == FALSE)
				{
					ResumeFilePath(&FilePathInfo);	      //备份当前路径
				}
				return ExitStkPerform;				//2007-12-14，用户取消
			}
		}
		else if(G_HandleResponse == 4)
		{
			goto BackMoveFinish;
		}
		else if(G_HandleResponse == 2)
		{
			if (STKControl == FALSE)
			{
				ResumeFilePath(&FilePathInfo);	      //备份当前路径
			}
			return 0;
		}
		else
		{	
			if (STKControl == FALSE)
			{
				ResumeFilePath(&FilePathInfo);	      //备份当前路径
			}
			return ExitStkPerform;
		}
	}
}
#endif
