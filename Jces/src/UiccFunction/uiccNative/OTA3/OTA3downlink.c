/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.1

 * MODEL: 	All model

 * RELEASE:	2008-5-13

 * AUTHER:	徐庆盛

 * HISTORY: V1.0 创建源文件

			V1.1 修改OTAPPHander(), 添加对下行指令0x0E(用户注册触发指令)的处理

			V1.2 修改OTAPPHander(),在验证mac时，对CPL计算两个字节的长度

			V1.3 修改函数IsDataSmsReceived，修改对于某些编译器，当%0时,编译器无法正确解析的问题
			
			V1.4 修改函数OTAPPHanlder，在应用下载过程中， 如果空间不够，提示用户后直接退出，不进行计数器的更新
			
			V1.5 修改函数OTAPPHanlder，修改当应用删除时，卡片拒绝对可更名应用的删除操作

			V1.6 修改函数IncreatCounter()：增加参数counterLen
			V1.7 修改函数OTAPPHanlder，修改在删除应用时，对可更名固化应用的判断错误
			V1.8 修改函数OTADownListUpdate，修改在对一级列表进行应用更新时，“添加列表”时的错误 
			V1.9 修改函数OTAPPHanlder。增加接口函数IF_OTAPPHanlder的返回值处理 @20110401 Version 1.23->1.24
			V1.10 修改函数OTAPPHanlder,对于BIP数据,其计算mac需要从文件中获取进行计算@20130821 V1.24->1.25
		    V1.11  修改函数OTAPPHanlder,增加接口IF_OTAPPHanlderMac的返回值的处理 @20140123 
			V1.12 :修改函数OTADownListUpdate ，修改局部变量len可能越界的问题，当重写整个列表时，最大支持5条列表@20140912 by xuyz
					修改函数OTAPPHanlder，当某个指令不需要校验mac时，则设置checkCounterResult = FALSE,该指令执行成功，则也不更新counter@20140912 
*/

#include <string.h>
#include "stkhead.h"
#include "ceph.h"
#include "fileAction.h"
#include "gsmpublic.h"
#include "OTA3Head.h"
#include "fileSystem.h"
#include  "Authentication.h"
#include  "interface.h"


#ifdef OTA

BYTE xdata G_BigDataLastNum;			//记录大数据下载的分组最后一条或者重传的最后一条
BYTE xdata G_BigDataGroupNum;			//记录大数据下载的分组的Index，下载成功时候要置零
/*--------------------------------------------------------------------------------------------------------------------
  Function name:	IncreatCNTR

  input parameter:	计数器的内容

  output parameter:	无

  description:		对计数器进行累加

  author:			Caibo

  testor

  status:
 -------------------------------------------------------------------------------------------------------------------*/
void IncreatCounter(BYTE *cntr,BYTE counterLen) 						//modify by caibo pls test
{
	BYTE idata i,offset;
	WORD xdata r;

	offset = counterLen-1;

	r = 0x0001;

	for(i=0x00;i<counterLen;i++)
	{
		r = r + cntr[offset-i];

		cntr[offset-i] = r&0xFF;

		r = (r>>0x08)&0x0001;
	}
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAGetAppOffset

  input parameter:   fileAddr:	存放应用的文件地址

  					 Aid	 ：	应用的AID

  output parameter:  应用在文件的偏移

  description:       在文件中搜索符合条件的AID，并且返回偏移值

  author:			 Zhuli

  test:		         Caibo

  status:			 OK

 -------------------------------------------------------------------------------------------------------------------*/

WORD OTAGetAppOffset(BYTE CONST aid[3])
{
	WORD  xdata fileSize, offset, dataLen;
	BYTE  xdata tmp[3];

	fileSize = GetTFSize(G_AppFileAddr);

	for(offset=0; offset<=fileSize;offset = offset+dataLen)
	{
		ReadTFBuf(G_AppFileAddr, offset, (BYTE *)&tmp[0], sizeof(tmp));

		if((aid[0]==tmp[0])&&((aid[1]&0x03)==(tmp[1]&0x03))&&(aid[2]==tmp[2])
			&&(((tmp[1]&D_DelTagApp) != D_DelTagApp)||(aid[1] == 0xFF)))		//过滤已删除AID, 并保留结束AID
		{
			return offset;
		}

		ReadTFBuf(G_AppFileAddr, offset+3, &tmp[0],0x02);		//读取应用长度

		dataLen = ((tmp[0]<<8)&0xFF00) + tmp[1];

		if(dataLen == 0xFFFF)						//判断是否到达应用末尾
		{
			return ErrorNoApp;
		}
	}

	return ErrorNoApp;
}

/*------------------------------------------------------------------------------------------------

Function name	:	OTAClearCurrentSession

input parameter	:	void

output parameter:	成功：	返回TRUE
					失败：	返回FALSE

description		:	清除实时登记文件中的当前操作记录

author			:	xuzhichu

test			:	Caibo
--------------------------------------------------------------------------------------------------------*/
void OTAClearCurrentSession(void)
{
	UWORD xdata fileAddr;

	fileAddr = SelectTFFile(DF_OTA, EF_RuntimeReg);

	SetTFConst(fileAddr, 0, 0x00, 4);
}

/*-----------------------------------------------------------------------------------------------------
Function name	:	OTAVerifyCurrentSession

Input parameter	:	Parameter:	当前操作的参数（列表更新为目录ID、菜单搜索为目录ID、MO下载为AID）
					operationType:			操作的命令类型

Output parameter:	一致：	返回TRUE
					不一致：返回FALSE

Description		:	比较实时登记文件中的内容与参数中的Parameter与OP是否一致

Coder:			:	xuzhichu	 tester:	caibo

--------------------------------------------------------------------------------------------------------*/
BYTE OTAVerifyCurrentSession(BYTE Parameter[3], BYTE operationType)
{
	UWORD xdata fileAddr;
	BYTE xdata buf[4];

	fileAddr = SelectTFFile(DF_OTA, EF_RuntimeReg);

	ReadTFBuf(fileAddr, 0, &buf[0], 4);

	if((buf[0] == 0x00)&&(operationType != D_AppClose))		//只有登记了应用关闭请求才提示用户"应用正在关闭中..."
	{
		return TRUE;
	}

	if((buf[0] == operationType) && (buf[1] == Parameter[0]))			   //命令类型和目录ID匹配
	{
		if ((operationType == MO_Download)||(operationType == D_AppClose)) //MO_Download和D_AppClose需要判断AID是否匹配
		{
			if ((buf[3] == Parameter[2]) && ((buf[2]&0x03) == (Parameter[1]&0x03)))
			{
				return TRUE;
			}
		}
		else 																//List_Update和Menu_Search命令类型和目录ID匹配即校验通过
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*--------------------------------------------------------------------------------------------------------------------
  Function name:	CheckLargeSms

  input parameter:	resultBuf:	保存下载信息的Buffer， LV结构

  					dataBuf:	保存结果的Buffer, LV结构

					largeCntr:	大数据量信息下载的分组值

					maxNum:		最大的短信索引值

  output parameter:	无

  description:		检查大数据信息下载的接受情况

  author:			Zhuli

  status:			OK
 -------------------------------------------------------------------------------------------------------------------*/
void CheckLargeSms(BYTE * resultBuf, BYTE * dataBuf, BYTE largeCntr,BYTE maxNum)
{
	BYTE idata i;

	dataBuf[0] = 0x00;

	if(ValueBit(resultBuf[4 + ((G_BigDataGroupNum-1)>>0x03)], ((G_BigDataGroupNum-1)&0x07)) == 0x00)
	{
		return;
	}

	for(i=0; i<maxNum; i++)						//得到改分组的收到情况
	{
		if(ValueBit(resultBuf[i>>0x03], (i&0x07)) != 0x00)
		{
			dataBuf[0]++;

			dataBuf[dataBuf[0]] = (i+1) + (G_BigDataGroupNum - 1) * largeCntr;
		}
	}
}

/*--------------------------------------------------------------------------------------------------------------------
  Function name:	IsDataSmsReceived

  input parameter:	index:	    数据短信的短信索引

					largeCntr:	续传标识

					dataBuf:	收到短信的结果（收到短信索引（4Byte）和收到全短信的分组（1Byte））

  output parameter:	TRUE：短信没有收到

					FALSE：短信收到

  description:		检查大数据信息下载的接受情况

  author:			Zhuli

  status:			OK
 -------------------------------------------------------------------------------------------------------------------*/

BYTE IsDataSmsReceived(BYTE index, BYTE largeCntr, BYTE * dataBuf)
{
	BYTE idata groupNum;

	if((largeCntr&0x01) == 0x00)						//非大数据信息下载,或者大数据下载的最后一组?
	{
		groupNum = 0x1F;						//只有一组,同时限制短信最大组数为groupNum组
	}
	else
	{
		largeCntr = ((largeCntr >> 3) & 0x1F);				//大数据的分组数

		groupNum = (index-1) / largeCntr;				//当前属于第几组(第groupNum + 1组)
	}

	if(ValueBit(dataBuf[4 + (groupNum>>0x03)], (groupNum&0x07)) == FALSE)	//改组是否已经收全
	{
		return FALSE;
	}

	if(largeCntr != 0x00)
	{
		index = (index -1)%largeCntr;                              //第几条短信
	}
	else
	{
		index = index -1 ;
	}

	return ValueBit(dataBuf[index>>0x03],(index&0x07))	;
}

/*
*@description  : 在列表更新操作中判断并添加列表 
*
*@param        :cmd  ---列表ID+列表名称
*@param        :type ---类型(目录列表or下载列表) 
8param         :index----记录偏移
*
*@return       :void
*
*@history      :create by xyz @100628 9:22
*/
void SubAppendListToServer(BYTE *cmd,BYTE type,BYTE index)
{
	WORD idata offset;
	
	if((cmd[0]<0xA0)&&(index == TYPE_Catalog_List))  	//目录
	{
		offset = OTAMenuAccess(cmd[0], 0, NULL, ACCESS_LOCATE, MenuType_OTA);

		if(offset == FALSE)
		{
			offset = ErrorNoApp;
		}
	}
	else	 											//应用（包括一级和二级）
	{
		offset = OTAGetAppOffset((BYTE CONST *)&cmd[0]);
	}

	if (offset == ErrorNoApp)
	{
		OTAAddAppTOServerList(type, index, &cmd[0], TRUE);
	}	
}

/**
 * @brief 		OTADownListUpdate 对下载列表进行更新的操作
 *
 * @param       cmd:	存放下载列表更新的数据信息(LV)
 *
 * @Returns   	TRUE/FALSE
 *
 * @author 		xyz
 *
 * @history 	created by xyz, @081211
 */

#ifdef LOCALOTA
BYTE xdata G_MenuId[2];                //本地OTA中保存目录ID
#endif

BYTE OTADownListUpdate(BYTE * cmd)
{
	BYTE  idata ptr,index;
	BYTE  idata i,l, type;
	BYTE  xdata len[4];

	if(cmd[2]<2)        //二级目录下的下载列表
	{
		index = OTALocatIndexInServerListFile(cmd[4]);

		type = TYPE_Download_List;

		i = cmd[4];
	}
	else               //一级目录下的下载列表
	{
		index = TYPE_Catalog_List;

		type = TYPE_Catalog_List;

		i = 0x00;
	}

	if(cmd[1] != 0xFF)  //更新当前列表页码
	{
		OTAMenuAccess(i, MENU_OFFSET_PAGE_NUMBER, &cmd[1], ACCESS_WRITE, MenuType_OTA);
	}

	if(OTAVerifyCurrentSession(&i, List_Update) == FALSE)
	{
		return FALSE;
	}

	OTAClearCurrentSession();

	ptr = 4;

	//清空下载列表
	if ((cmd[3] == 0x01)								  	//列表数据总数未1
		&&(cmd[5]==0x00)&&(cmd[6]==0x00)					//应用索引为0xHH/0x00,0x00,0x00
		&&(cmd[9]==0x00)									//应用菜单名称长度为0
		&&(cmd[1] == 0xFF))  								//当前列表页码为0xFF
	{
		for(i = index; i <index + 5; i++)	 				//清空整个列表
		{
			SetLFConst(G_ServerListFileAddr, i, 0x00, 0xFF, 0x04);	//前4个字节值FF(TYPE+AID)
		}

		PendingMessageAlert(D_DownListDeleteTag,0x00);  			//提示清空列表成功

		return TRUE;
	}

	//更新下载列表
	if ((cmd[2] == 0)||(cmd[2] == 2))						//write all items into list
	{
		for(i = index; i <index + 5; i++)	 			
		{
			SetLFConst(G_ServerListFileAddr, i, 0x00, 0xFF, 0x04);	
		}
		
		if(cmd[3] > 0x05)
		{
			cmd[3] = 0x05;   //根据实际需求，目前最多支持5个列表
		}

		for (i=0 ; i<cmd[3]-1; i++)	  		//查找最后一个列表数据的偏移
		{
		   	l = 6 + cmd[ptr+5];
			
			ptr = ptr + l + 1; 
			
			len[i] = l;
		}

		for (i=cmd[3]; i>=1; i--)
		{
			SubAppendListToServer(&cmd[ptr],type ,index);

		    if(i > 1)
			{
				ptr = ptr - (len[i-2] + 1);	
			}
		}
	}
	else if ((cmd[2] == 1)||(cmd[2] == 3)) 					//do opeation according to type
	{

		for (i = 0 ; i < cmd[3]; i ++)
		{
			l = 6 + cmd[ptr+5];

			switch(cmd[ptr+l])
			{
				case  0x00:				//删除
				{
					OTADelAppFromServerList(index, &cmd[ptr]);
					break;
				}	
				case 0x01: 				// 添加
				{ 
					SubAppendListToServer(&cmd[ptr],type ,index);

					break;
				}
				case 0x02: 				// 修改
				{
					OTADelAppFromServerList(index, &cmd[ptr]);

					OTAAddAppTOServerList(type, index, &cmd[ptr], TRUE);

					break;
				}
            
				default:
					return FALSE;
			}
			ptr	 = ptr + l + 1;
		}
	}
	else	//for baihe-test
	{;}

#ifdef LOCALOTA
	G_MenuId[0] = cmd[2];			//命令细节

	G_MenuId[1] = cmd[4];           //保存目录索引
#endif

	return TRUE;
}

/*--------------------------------------------------------------------------------------------------------------------
  Function name:	OTAPPHanlder

  input parameter:	下行命令的类型在RecData中的偏移

  output parameter:	成功：	TRUE

					失败：	返回errorCode

  description:		对OTA下行命令的处理

  author:			Zhuli

  status:
 -------------------------------------------------------------------------------------------------------------------*/
BYTE OTAPPHanlder(BYTE * dataptr)
{
	BYTE  idata result, i;
	BYTE  idata  checkCounterResult;				//该变量保存校验结果，可能为FALSE/TRUE/0x02,严格判断是否更新技术器
	BYTE  xdata  key[16];
	BYTE  xdata  mac[5];
	BYTE  xdata  * pointer;
	WORD  idata offset, appSize;
	UWORD xdata fileAddr;
	BYTE  idata handResult,handlerMac;

#ifdef LOCALOTA
	BYTE idata tag,deletetag;
#endif

	result = OTAGetKey(&dataptr[20], &key[0], D_OTAKey);		//result for case 0x02,0x05,0x06

	if(result == FALSE)
	{
		return FALSE;
	}

	memcpy(&mac[0], (BYTE *)&dataptr[16], sizeof(mac));

	memmove((BYTE *)&dataptr[4], (BYTE *)&dataptr[0], 16);

	//checkCounterResult = TRUE;
	checkCounterResult = FALSE;  //对于不需要校验MAC的指令，后续也不对计数器进行更新

	handlerMac = IF_OTAPPHandlerMac(dataptr[25]);	 //增加处理指令是否需要校验MAC的接口，TRUE:不校验，FALSE:需要校验
	
	if(handlerMac == FALSE)
	{
		if(dataptr[25] != 0x0E)     //用户注册请求，不校验MAC和Counter
		{
			if(G_ResponseMode == Mode_BIP)   //OTA数据通过bip下来，数据缓存在G_BipDataAddr中
			{
				if(OTACaluteBIPMac(&dataptr[4], &mac[0], &key[0], (((dataptr[4]<<8)&0xFF00)+dataptr[5]-2)) == FALSE)
				{
					return FALSE;
				}
			}
			else
			{
							
				if(OTACaluteMac(&dataptr[4], &mac[0], &key[0], (((dataptr[4]<<8)&0xFF00)+dataptr[5]-2), 0x00) == FALSE)
				{
					return FALSE;
				}			
				
			}
	
		 	//checkCounterResult = TRUE;   //本地OTA测试脚本不校验计数器
																																				
			checkCounterResult = OTAVarifyCNTR(&dataptr[14], dataptr[24], 0);
	
			if(checkCounterResult == FALSE)
			{
				return FALSE;
			} 
			
		}
	}  

	memcpy(&key[0], &dataptr[14], 5);

	key[5] = dataptr[24];

	G_CounterType = dataptr[24];	//用于判断是否服务器主动下载应用

	dataptr = dataptr+ 25;//point to command type

	if((G_OpenChannel&BIPCurrentOpened) == BIPCurrentClosed)	//短信通道
	{
		memmove(&dataptr[2], &dataptr[1], dataptr[1]+1);

		dataptr[1] = 0x00;
	}
	


	handResult = IF_OTAPPHander(&dataptr[0]);  //接口指令处理结果，0x01:接口指令处理正确 0x02:接口指令处理错误 0x00-未处理接口指令，执行后续操作
	
	if(handResult == FALSE)
	{
		switch(dataptr[0])
		{
			case 0x01:                  //应用下载
			case 0x09:                  //远程应用更新
			case 0x0A:					//OTA data download
			{
				i = 0;

				if (dataptr[0] == 0x0A)
				{
					if (dataptr[3] == 0x01)		//事件触发型应用下载
					{
						dataptr[0] = 0x01;
					}
					else if (dataptr[3] == 0x02)	//事件触发型应用更新
					{
						dataptr[0] = 0x09;
					}
					else	//for baihe-test
					{;}
			 
					dataptr[2] --;
	
					memmove(&dataptr[1], &dataptr[0], 3);	//去掉操作类型字节
	
					i = 1;
				}

				if(dataptr[i] == 0x01)		//应用下载
				{
					if(OTAVerifyCurrentSession(&dataptr[3+i], MO_Download) == FALSE)
					{
						return FALSE;
					}
				}
	
				if(dataptr[i] != 0x0A)
				{
					appSize = 0;

					if(dataptr[i] == 0x09)//远程应用更新
					{
						offset = OTAGetAppOffset((BYTE CONST *)&dataptr[3+i]);
	
						if(offset != ErrorNoApp)
						{
							ReadTFBuf(G_AppFileAddr, offset, &mac[0], 0x05);//AID and space size

							if (mac[0] == 0x9F)//事件触发型应用
							{
								OTARemoveEventReg(&mac[0], FALSE);	//在注册文件里删除应用原来注册的事件ID
							}

							appSize = (WORD)((mac[3]<<8)&0xFF00) + mac[4];	 //远程应用更新时，若应用存在则删除原有业务再进行更新
						}
					}

					offset = (WORD)((dataptr[10+i]<<8)&0xFF00) + dataptr[11+i];		//应用数据空间
	
					if(offset > (GetFreeAppSpaceSize() + appSize)) //应用下载或远程应用更新时，判断空间是否够
					{
						if(dataptr[i] == 0x01)
						{
							OTAClearCurrentSession();
						}

						return FALSE;
					}
	
					if(dataptr[3+i] < 0xA0)	//9F目录不可见时也能下载应用
					{
						if(OTAIsMenuActive(dataptr[3+i]) == FALSE)
						{
							if (dataptr[3+i] != 0x9F)
							{
								return FALSE;
							}
						}
					}
				}
#ifdef LOCALOTA
				if( (dataptr[3+i] >= 0xA0)&&(dataptr[3+i] <= 0xB7))  //SIM卡应拒绝执行远程下载更新中关于可更名固化应用的部分
				{

					if ((G_CounterType==D_CNTRB)&&(dataptr[i]!=D_OTAInterDown))
					{
						tempBuf[2] = dataptr[i];  //保存命令类型
					
						tempBuf[6] = 0x00;      //保存执行结果
					
						tempBuf[1] = 0x05;
					
						memcpy(&tempBuf[3],&dataptr[3+i],0x03); //保存应用索引
					
						WriteVariable(OTADowUpdateAPPTag, (BYTE *)&tempBuf[1]);
					
						PendingMessageAlert(D_UpdateDowloadConfirm,0x00);	
					}

					return FALSE;
				}
#endif
				result = OTASaveDownLoadData(&dataptr[1+i], dataptr[i],31+i);

				break;
			}
			case 0x02:				   	//down list update
			{
				if(((dataptr[1] == 0x00)&&(dataptr[2] == 0x00))||(dataptr[5] == 0x00)) //列表长度或列表数据总数为0时
				{
					OTAClearCurrentSession();
	
					break;
				}
	
				result = OTADownListUpdate(&dataptr[2]);

#ifdef LOCALOTA
				if(G_CounterType==D_CNTRB)
				{
					G_MTPerformResult = result;

					PendingMessageAlert(D_DownListUpdateConfirm,0x00);
				}
#else
			
				PendingMessageAlert(D_DownListUpdateTag,0x00);

#endif
	
				break;
			}
			case 0x03:					//push list update
			{	
				if(dataptr[3] == 0xFF)
				{
					for(i = 1; i <=15; i++)
					{
						SetLFConst(G_ServerListFileAddr, i, 0x00, 0xFF, 0x04);	//前4个字节值FF(TYPE+AID)
					}

#ifndef LOCALOTA	
					PendingMessageAlert(D_PushListClearTag,0x00);	//提示清空成功
#endif
				}
				else
				{
					result = 0;
	
					for(i=0; i<dataptr[3]; i++)		//add the list data to file
					{
						mac[3] = 6 + dataptr[9+result];
	
						mac[3] += dataptr[4+result+mac[3]] + 1;
	
						mac[3] += dataptr[4+result+mac[3]] + 1;
	
						if(dataptr[4+result] < 0xA0)	//过滤PUSH时卡片没有该目录时的应用
						{
							if(OTAMenuAccess(dataptr[4+result], 0, NULL, ACCESS_LOCATE, MenuType_OTA) == FALSE)  
							{
								goto _added_;
							}
						}
	
						G_AppOffset = OTAGetAppOffset((BYTE CONST *)&dataptr[4+result]);
	
						if( G_AppOffset == ErrorNoApp )
						{
							OTAAddAppTOServerList(TYPE_Recommend_List, 0x01, &dataptr[4+result], TRUE);	//add it to push list
						}
_added_:
						result = result + mac[3];
	
						if(result > dataptr[2])
						{
							break;
						}
					}
#ifndef LOCALOTA
					PendingMessageAlert(D_OTAPushUpdateTag,0x00);
#endif
				}
	
				result = TRUE;

#ifdef LOCALOTA
				G_MTPerformResult = result;
				
				PendingMessageAlert(D_PushListConfirm,0x00);
#endif
	
				break;
			}
			case 0x04:					//user regist
			{
				result = GetSetRegStatus(DF_OTA, EF_OTAConfig, RegStatus_Set);

				result = ((result == TRUE) ? FALSE : TRUE);	//已注册为FALSE，未注册为TRUE
	
				if (result == TRUE)	//未注册，已设置注册状态，修改承载方式为SMS
				{	
					UpdateDownloadConfig(0x00);  
				}
				else
				{
					return FALSE;
				}
	
				IF_OTAPPHanlder_Register();						//提示注册成功	
	
				break;
			}
			case 0x05:					//synchronization
			{	
				PendingMessageAlert(D_OTASynchronizTag,0x00);
	
				result = TRUE;
	
				break;
			}
			case 0x06:					//delete AID
			{

#ifdef LOCALOTA
				if(G_CounterType == D_CNTRB)
				{
					WriteVariable(LocalOTAAppDelNoticeTag, (BYTE *)&dataptr[2]);
	
					PendingMessageAlert(D_OTAAppDelNoticeTag,0x00);
				}			
#endif	
				if((dataptr[3]&0x02) == 0x00)		//delete all
				{
					for (G_STKAppOffset = 0;; G_STKAppOffset += offset)
					{
						offset = OTASearchAID(0x00, D_AllAID, FALSE);
	
						if (offset != 0)
						{
							ReadTFBuf(G_AppFileAddr, G_STKAppOffset, &mac[0], 1);
	
							if(mac[0] > 0x9F)
							{
								PendingMessageAlert(D_SetUpMenuTag,0x00);//只在需要删除的服务中有一级菜单应用时才SetUpMenu
							}
	
							OTADeleteApp(!(dataptr[3]&0x01), D_DelTagApp);
						}
						else
						{
							break;
						}
					}
				}
				else
				{
					for (i = 0,pointer = (BYTE  *)&dataptr[5]; i < dataptr[4]; i++, pointer += 3)
					{
#ifdef LOCALOTA
						if((pointer[0] >= 0xA0)&&(pointer[0] <= 0xB7))  //卡片拒绝执行可更名固化应用的删除
						{
							continue;
						}
#endif
						G_STKAppOffset = OTAGetAppOffset((BYTE CONST *)pointer);
	
						if (G_STKAppOffset != ErrorNoApp)
						{
							if(pointer[0] > 0x9F)  //只在需要删除的服务中有一级菜单应用时才SetUpMenu
							{
								PendingMessageAlert(D_SetUpMenuTag,0x00);
							}
	
							OTADeleteApp(!(dataptr[3]&0x01), D_DelTagApp);  //设置删除标记,原为OTADeleteApp(!(dataptr[3]&0x01), D_DelTagApp);
						}
					}
				}	

				IF_OTAPPHander_DelAid(&dataptr[0]);		//特殊需求接口				
	
				result = TRUE;
	
				break;
			}
			case 0x07:					//disable\enable AID
			{
#ifdef LOCALOTA
				WriteVariable(LocalDisableEnableTag, (BYTE *)&dataptr[2]);
	
				PendingMessageAlert(D_DisableEnbleAppConfirm,0x00);
#endif

	
				for(i=1,pointer = (BYTE *)&dataptr[4]; i<=dataptr[3]; i++ ,pointer += 3)
				{
#ifdef LOCALOTA
					if((pointer[0] >= 0xA0)&&(pointer[0] <= 0xB7))   //对可更名固化应用进行启用、禁用
					{
						if ((pointer[1]&0x80) == 0x80)		//启用
						{
							OTAMenuAccess(pointer[0], 0, NULL, ACCESS_ENABLE_CATALOG, MenuType_OTA);				
						}
						else
						{
							OTAMenuAccess(pointer[0], 0, NULL, ACCESS_DISABLE_CATALOG, MenuType_OTA);
						}
				
						PendingMessageAlert(D_SetUpMenuTag,0x00);
				
						continue;
					}
#endif
					G_AppOffset = OTAGetAppOffset((BYTE CONST *)pointer);

					if(G_AppOffset != ErrorNoApp)
					{
						ReadTFBuf(G_AppFileAddr, G_AppOffset, &mac[0], 0x03);	//获得应用原来的AID
	
						if ((pointer[1]&0x80) == 0x80)		//启用
						{
							mac[1] |= 0x80;
						}
						else					//禁用
						{
							mac[1] &= 0x7F;
						}
	
						SetTFConst(G_AppFileAddr, G_AppOffset+1, mac[1], sizeof(BYTE));
	
						if (pointer[0] >0x9F)    //对一级应用的启、禁用需setup menu
						{
							PendingMessageAlert(D_SetUpMenuTag,0x00);
						}
					}
					else
					{
						for(result = 1; result < GetLFRecNumLgth(G_ServerListFileAddr, FileNum); result ++)
						{
							ReadLFBuf(G_ServerListFileAddr, result, 0x01, &mac[0], 0x03);
	
							if ((pointer[0] == mac[0])&&(pointer[2] == mac[2]))
							{
								if ((pointer[1]&0x80) == 0x80)	//启用
								{
									mac[1] |= 0x80;
								}
								else				//禁用
								{
									mac[1] &= 0x7F;
								}
	
								SetLFConst(G_ServerListFileAddr, result, 0x02, mac[1], sizeof(BYTE));
							}
						}
					}
				}
				result = TRUE;
	
				break;
			}
			case 0x08:					//OTA menu management
			{
				result = 0;

#ifdef LOCALOTA
				tag = FALSE;

				deletetag = FALSE;
#endif
				for(i = 0; i < dataptr[3]; i++)
				{
					if(result >= dataptr[2])
					{
						break;
					}
	
					mac[0] = dataptr[4+result];			//point to munuId
	
					mac[3] = dataptr[5+result] + 1;			//get the menu data length
	
					mac[2] = dataptr[5+mac[3]+result] ;		//point to the hanlde type
#ifdef LOCALOTA
					if(mac[2] != 0x06)
					{
						tag = TRUE;
					}
#endif
	
					switch(mac[2])
					{
						case 0x00:                   //启用
						case 0x01:                   //禁用
						case 0x03:                   //删除
						case 0x04:                   //更名
						case 0x05:                   //显示"应用管理"
						case 0x06:                   //隐藏"应用管理"
						{
							if(mac[2] == 0x04)
							{
								OTASynMenuName();
							}
	
							if (mac[2] == 0x03) //远程目录管理删除的目录，应出现在一级应用下载列表中
							{
#ifdef LOCALOTA								
								if((mac[0] >= 0xA0)&&(mac[0] <= 0xB7))
								{
									break;
								}

								deletetag = TRUE;
#endif
								ResData[0] = mac[0];

								memset(&ResData[1], 0x00, 0x04);
	

								OTAMenuAccess(mac[0], MENU_OFFSET_NAME, &ResData[5], ACCESS_READ, MenuType_OTA);//获取菜单名称
	
								OTAAddAppTOServerList(TYPE_Catalog_List, TYPE_Catalog_List,(BYTE *)&ResData[0], FALSE);

								DeleteAppUnderMenu(mac[0]);		 //删除特定目录下的应用，删除的应用不添加到应用下载列表中
							}
	
							OTAMenuAccess(mac[0], MENU_OFFSET_NAME, &dataptr[result+5], mac[2], MenuType_OTA);
	
							break;
						}
						case 0x02:     //添加
						{
							if(mac[0] < 0xA0)		//排除一级应用
							{
								OTAMenuAddItem(&dataptr[4+result],0x00);
	
								mac[1] = 0x00;
	
								mac[2] = 0x00;
	
								OTADelAppFromServerList(TYPE_Catalog_List, &mac[0]);	//删除目录下载列表中数据
	
								if(OTAVerifyCurrentSession(&mac[0], MO_Download) == TRUE)
								{
									OTAClearCurrentSession();
								}
							}
	
							break;
						}
						default:		//for baihe-test
							break;
					}
	
					result = result + mac[3] + 2;
				}

#ifdef LOCALOTA
				if(G_CounterType == D_CNTRB)
				{
					WriteVariable(OTADirectoryNotice, (BYTE *)&dataptr[2]);   //远程目录管理确认命令
		
					PendingMessageAlert(D_DirectoryNoticeTag,0x00);	
				}
#endif
#ifndef LOCALOTA
				PendingMessageAlert(D_SetUpMenuTag,0x00);
#endif
	
				result = TRUE;
	
				break;
			}
			case 0x0B:   //应用搜索结果
			{
				result = OTAVerifyCurrentSession(&dataptr[3], Menu_Search);
	
				if (result == TRUE)
				{
					WriteVariable(MenuSearchResult, (BYTE *)&dataptr[2]);
	
					PendingMessageAlert(D_MenuSearchTag,0x00);
				}
	
				break;
			}
			case 0x0C://对于BIP PUSH不管是由卡端还是服务器端主动发起的交互过程，都强制用BIP承载方式,并更新OTA配置文件
			{
				G_ResponseMode = Mode_EF;
	
				PendingMessageAlert(dataptr[3]+D_BipPushOpen,0x00);
	
				G_BIPPushCntr = dataptr[4];
	
				break;
			}
			case 0x0D:
			{	
				for (i =1; i <= dataptr[3]; i++)
				{
					memcpy(&mac[0], &dataptr[i*3+1], 3);

					mac[1] &= 0xC0;
	
#ifdef LOCALOTA
					if((mac[0] >= 0xA0)&&(mac[0] <= 0xB7))  //不允许对可更名固化应用开通/关闭
					{
						continue;
					}
#endif	
					if (G_CounterType == D_CNTRA)  //卡发起命令，服务器产生下行报文
					{
						result = OTAVerifyCurrentSession(&mac[0], D_AppClose);
					}
					else
					{
						result = TRUE;
					}
	
					if(result == TRUE)
					{
						OTAClearCurrentSession();
	
						offset = OTAGetAppOffset((BYTE CONST *)&mac[0]);
	
						if(offset != ErrorNoApp)
						{
							ReadTFBuf(G_AppFileAddr, offset, &mac[0], 0x03);	//获得应用原来的AID
	
							if ((dataptr[i*3+2] & 0x04) == 0x04)
							{
								mac[1] |= 0x04;		//开通应用
							}
							else
							{
								mac[1] &= 0xFB;		//关闭应用
							}
	
							WriteTFBuf(G_AppFileAddr, offset, &mac[0], 0x03);
						}
					}
				}
#ifdef LOCALOTA
				if(G_CounterType == D_CNTRB)
				{
					WriteVariable(OTAAPPOpenClose, (BYTE *)&dataptr[2]);
	
					PendingMessageAlert(D_OTAAppOpenCloseTag,0x00);
				}			
#endif
				break;
			}
			case 0x0E:         //用户注册请求
			{
				result = FALSE ;			//不更新B计数器
	
				PendingMessageAlert(D_OTARegisterTag,0x00);
	
				break;
			}
#ifdef LOCALOTA
			case 0x0F:
			{
				if(dataptr[3] == 0x00)       //更新短信接入号
				{
					fileAddr = SelectLFFile(DF_OTA, EF_TPDA);	

					dataptr[3] = dataptr[4]+1;    //保存个数

					dataptr[4] = GetBCDCount(&dataptr[5], dataptr[4]);	
	
					WriteLFBuf(fileAddr, 1, 0x00, &dataptr[3], dataptr[3]+1);    //更新6EAC中的上行TPDA
				}
				else if(dataptr[3] == 0x01)  //BIP 接入IP更新
				{
					if(dataptr[4] != 0x05)    //长度不正确
					{
						return FALSE;
					}
				
					if(dataptr[5] != 0x21)   
					{
						return FALSE;
					}
	
					fileAddr = SelectTFFile(DF_OTA,EF_OTAConfig);
	
					WriteTFBuf(fileAddr, 0x05, &dataptr[6], 0x04);				
				}
				else	//for baihe-test
				{;}

				if(G_CounterType == D_CNTRB)
				{					
					PendingMessageAlert(D_UpdateIPSmsNumber,0x00);	
				}
				result = TRUE;
	
				break;
			}
			case 0x10:			//弹出应用
			{
				if( (dataptr[3] >= 0xA0)&&(dataptr[3] <= 0xB7))  //不允许对可更名固化应用进行弹出
				{
					return FALSE;
				}
				else
				{
					memcpy(&G_PopUpAID[0], &dataptr[3], sizeof(G_PopUpAID));		//记录弹出应用的应用索引
			
					PendingMessageAlert(D_PopUpAppliction,0x00);	
				}
				break;
			}
#endif
			default:
				G_ResponseMode = Mode_EF;
				result = FALSE;
				break;
		}
	}
	else
	{
		if(handResult == TRUE)
		{
			result = TRUE;
		}
		else
		{
			result = FALSE;
		}
	}

	if(result == TRUE)
	{
		OTAVarifyCNTR(&key[0], key[5], checkCounterResult);

		if(((G_OpenChannel&BIPNeedClosed) == BIPNeedClosed) && (dataptr[0] != 0x0B))
		{
			PendingMessageAlert(D_CloseChannelTag,0x00);
		}
		                    
		switch(dataptr[0])		    //列表更新提示语在通道关闭后提示
		{
			case 0x01:
			{
				if (dataptr[3+i] == 0x9F)
				{
					PendingMessageAlert(D_EventDataDownload,0x00);    //执行事件触发型应用注册信息
					break;
				}
				else
				{
					PendingMessageAlert(D_DataDownSucessTag,0x00);
				}  	
			
			}
			case 0x09:
			{
#ifndef OLDOTA
				if (dataptr[3+i] == 0x9F)
				{
					result =   D_EventDataDownload;
				}
#endif
				if((dataptr[3+i] > 0x9F)&&(dataptr[i] != D_OTAInterDown))
				{
					result = D_SetUpMenuTag;
				}

				PendingMessageAlert(result,0x00);

				break;
			}
			case 0x0A:                                     //增加交互应用处理 modify by xyz@20100118
			{
				PendingMessageAlert(D_InterDownSucessTag,0x00);

				break;
			}
#ifdef LOCALOTA 
			case 0x02:
			{
				if(G_CounterType == D_CNTRA)
				{
				 	if(((dataptr[1] == 0x00)&&(dataptr[2] == 0x00))||(dataptr[5] == 0x00)) //列表长度或列表数据总数为0时
					{
						break; 
					}

					PendingMessageAlert(D_DownListUpdateTag,0x00);
				}
				break;
			}
			case 0x08:
			{
				if((deletetag == TRUE)&&(G_CounterType == D_CNTRA))
				{
					PendingMessageAlert(D_DisplayMenuManagement,0x00) ;
				}
				if(tag == TRUE)
				{
					PendingMessageAlert(D_SetUpMenuTag,0x00);   //本地OTA中，显示，隐藏应用管理不setupmenu
				}
				break;
			}
#endif
			default: 
				break;
		}
	}

	return TRUE;
}
#endif
