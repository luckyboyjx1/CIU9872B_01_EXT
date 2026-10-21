/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.0

 * MODEL: 	All model

 * RELEASE:	2008-5-13

 * AUTHER:	徐庆盛

 * HISTORY: V1.0 创建源文件
			
			V1.1 增加普通OTA3的"下载设置"的接口函数

			V1.2 修改JSIndustryManageMenu函数;修改在显示"企业公文"列表时,每页由8个改为10个 

			V1.3 修改函数OTAAppDelCatalogNotice，修改当6F18中存在某个一级应用目录时，先还原删除标记;若0x6EA4中有该一级应用，则从6F18中删除该条记录，
				 若0x6EA4中无该应用，则继续搜索后续待删除的目录。
			V1.4 修改函数OTAMenuManageDelete ,OTAAppDelNotice, 在用户选择"继续删除"时,仅将当前应用打上D_DelNotFinishTagApp标记,不加入到下载列表中,仅在上发删除通知的同时,将应用加入在下载列表中	 -modify xyz 20100816 ver:1.72 -> 1.73
			V1.5 修改函数OTAMenuManageDelete,修改buf使用错误
			V1.6 修改函数OTAAppDelNotice ，OTAAppDelCatalogNotice，修改当删除的AID个数为一条短信允许删除的最大AID个数的整数倍时，上行短信总数错误的问题
			v1.7 修改函数OTAMenuManageDeleteCatalog。江苏OTA在处理应用管理完成后直接回退到一级菜单
			V1.8 修改函数JSOTASubmitSynchronizeInfo，修改将信息同步数据存放在 文件6EA5中，减少对varibleBuf的使用 @20110402 
			
			V1.9 修改函数OTAMenuManageDelete，OTAAppDelNotice 增加对应用删除时的bug处理  V1.20 ->1.21@20130906

			V1.10 修改函数UpdateOTADownloadSet，当item不等于ERROR时，如果item = BackMove，则返回上一级菜单，否则退出该菜单 modify by xuyz＠20140505
			
			V1.11 修改函数UpdateOTADownloadSet，调用数组UpdateParameterSet处改为调用接口IF_UpdateOTADownloadSet，使得该数组大小可以改动           by yangjian＠20160121

*/

#include <string.h>
#include "main.h"
#include "arithmatic.h"
#include "fileAction.h"
#include "stkhead.h"
#include "error.h"
#include "OTA3Head.h"
#include "fileSystem.h"
#include "gsmpublic.h"
#include "ceph.h"
#include "interface.h"


#ifdef OTA
BYTE xdata G_BIPPushCntr;

#ifdef LOCALOTA
/*
description :上发MT应用下载、应用更新远程管理确认命令
*/
BYTE LocalUpdateDowloadAPPConfirm(void)
{
	BYTE idata result;

	tempBuf[3] = ReadVarible(OTADowUpdateAPPTag, &tempBuf[4]);

	tempBuf[3] = tempBuf[4];     //命令类型
	
	tempBuf[4] = 0x01;           //应用个数

	tempBuf[2] = 0x06;           //命令长度

	tempBuf[1] = 0x0D;           //命令类型

	tempBuf[0] = tempBuf[2] +2;

	result = OTASendUplinkCommandMessage(1, &tempBuf[0]);

	return result;
}
/*
description :上发本地OTA中远程删除命令
*/
BYTE LocalOTAAppDelNoticeConfirm(void)
{
	BYTE idata i;
	BYTE idata result ;

	ResData[0] = ReadVarible(LocalOTAAppDelNoticeTag, &ResData[1]);

	if((ResData[1]&0x02) == 0x00)  //delete all AID
	{
		tempBuf[4] = 0x00;

		tempBuf[5] = 0x01;

		tempBuf[2] = 0x03;       //命令长度
	}
	else                           //delete AIDs
	{	
		tempBuf[4] = ResData[2] ;  //服务个数

		for(i=0;i< ResData[2];i++)
		{
			memcpy_G(&tempBuf[5+i*4],&ResData[3+i*3],0x03);

			if((ResData[3+i*3] >= 0xA0)&&(ResData[3+i*3] <= 0xB7))
			{
				tempBuf[8+i*4] = 0x00;   //命令执行结果：失败
			}
			else
			{
				tempBuf[8+i*4] = 0x01;   //命令执行结果：成功
			}
		}

		tempBuf[2] = 2 + ResData[2] *4 ;
	}

	tempBuf[3] = 0x06;

	tempBuf[1] = 0x0D;

	tempBuf[0] = tempBuf[2] + 2;

	result = OTASendUplinkCommandMessage(1, &tempBuf[0]);

	return result;
}
/*
description :上发本地OTA中远程应用启用/禁用确认命令
*/
BYTE LocalOTADisableEnableConfirm(void)
{
	BYTE idata i;
	BYTE idata result ;

	ResData[0] = ReadVarible(LocalDisableEnableTag, &ResData[1]);

	tempBuf[4] = ResData[1] ;  //服务个数

	for(i=0;i< ResData[1];i++)
	{
		memcpy_G(&tempBuf[5+i*4],&ResData[2+i*3],0x03);

		tempBuf[8+i*4] = 0x01;   //命令执行结果：成功
	}

	tempBuf[2] = 2 + ResData[1] *4 ;

	tempBuf[3] = 0x07;

	tempBuf[1] = 0x0D;

	tempBuf[0] = tempBuf[2] + 2;

	result = OTASendUplinkCommandMessage(1, &tempBuf[0]);

	return result;
}
/*
description :上发本地OTA远程目录管理确认信息
*/
BYTE LocalMenuManageConfirm(void)
{
	BYTE idata i;
	BYTE idata result,length,type;

	result = 0;

	ResData[0] = ReadVarible(OTADirectoryNotice, &ResData[1]);

	for(i=0;i<ResData[1];i++)
	{
		length = ResData[3+result] + 1;		//get the menu data length

		type = ResData[3+length+result]; 

		tempBuf[5+i*2] = ResData[2+result];		//point to munuId

		if((tempBuf[5+i*2] >= 0xA0)&&(tempBuf[5+i*2] <= 0xB7))
		{
			if((type == 0x02)||(type == 0x03))  //不允许对可更名固化应用进行添加或删除
			{
				tempBuf[5+i*2+1] = 0x00;       //失败
			}
			else
			{
				tempBuf[5+i*2+1] = 0x01;       //成功
			}
		}
		else
		{
			tempBuf[5+i*2+1] = 0x01;              //命令执行结果
		}
	
		result = result + length + 2;		
	}

	tempBuf[4] = ResData[1];        //目录个数

	tempBuf[3] = 0x08;

	tempBuf[2] = (tempBuf[4]*2) + 2;   //后续数据长度

	tempBuf[1] = 0x0D;

	tempBuf[0] = tempBuf[2] + 2;    

	result = OTASendUplinkCommandMessage(1, &tempBuf[0]);

	return result;	
}
/*
description :上发本地OTA 应用开通/关闭确认信息
*/
BYTE LocalAPPOpenCloseConfirm(void)
{
	BYTE idata i;
	BYTE idata result ;

	ResData[0] = ReadVarible(OTAAPPOpenClose, &ResData[1]);

	tempBuf[4] = ResData[1] ;  //服务个数

	for(i=0;i< ResData[1];i++)
	{
		memcpy_G(&tempBuf[5+i*4],&ResData[2+i*3],0x03);

		if((ResData[2+i*3] >= 0xA0)&&(ResData[2+i*3] <= 0xB7))
		{
			tempBuf[8+i*4] = 0x00;   //命令执行结果失败
		}
		else
		{
			tempBuf[8+i*4] = 0x01;   //命令执行结果：成功
		}
	}

	tempBuf[2] = 2 + ResData[1] *4 ;

	tempBuf[3] = 0x0D;

	tempBuf[1] = 0x0D;

	tempBuf[0] = tempBuf[2] + 2;

	result = OTASendUplinkCommandMessage(1, &tempBuf[0]);

	return result;
}
/*
description :显示本地OTA列表更新提示信息
*/
void LocalDisplayDownListUpdate(void)
{
	if ((G_MenuId[0]&0x02) == 0x00)
	{
	   	tempBuf[0] = 0x00;

		OTAMenuAccess(G_MenuId[1], MENU_OFFSET_NAME, &tempBuf[0], ACCESS_READ, MenuType_OTA);

		if (tempBuf[1] == 0x80)
		{
			tempBuf[1] = tempBuf[0] - 1;
		}
		else
		{
			AsciiToUCS2(&tempBuf[0], &tempBuf[1]);
		}

		AddVlaue(&tempBuf[1], (BYTE CONST *)&listUpdate[0]);

		cephDisplayTextSingle(0x00, UCS2, (BYTE CONST *)&tempBuf[1]);
	}
	else  //一级应用列表更新
	{
		cephDisplayTextSingle(0x00, UCS2, (BYTE CONST *)menuListUpdate);
	} 
}
/*
description :本地OTA列表更新确认和最新推荐确认信息 及短信接入号和IP更新信息
*/
void LocalDownListPushListIPConfirm(BYTE pendingTag)
{
	tempBuf[1] = 0x0D;

	tempBuf[2] = 0x03;

	tempBuf[3] = (0xCC - pendingTag);   //命令类型

	tempBuf[4] = 0x00;                  //应用个数

	if(tempBuf[3] == 0x0F)	       // 本地OTA短信接入号和IP更新确认信息
	{
		tempBuf[5] = 0x01;     //执行结果 
	}
	else
	{
		tempBuf[5] = G_MTPerformResult;     //执行结果
	}

	tempBuf[0] = tempBuf[2] + 2;

	OTASendUplinkCommandMessage(1, &tempBuf[0]);	
}
#endif
/*
*@description:拼装目录或应用删除数据,并发送
*
*@param      :menuId  :	0x00-目录删除操作  其他：应用删除操作
*@param      :aidCount:	删除的目录总数或AID总数
*@param     : cmd      :	存储命令数据缓冲区
*
*@returns   : TRUE/FALSE
*
*@HISTORY   :create by xyz@100603 15:08
*/
BYTE OTAAppDelNotice(BYTE menuId,BYTE aidcount,BYTE *cmd)
{
	BYTE idata smsIndex,smsCount;
	BYTE idata countAid;	//本条短信中删除的AID总数
	BYTE idata result,i;
	BYTE xdata aid[3];
	WORD idata datalen;
	UWORD xdata  fileAddr;
		
	smsCount = aidcount/D_MaxServiceNum;
	
	if ((aidcount%D_MaxServiceNum) > 0x00)
	{
		smsCount ++;
	}	

	for(smsIndex = 0x01; smsIndex <= smsCount; smsIndex++)	//发送多条短信
	{
		cmd[1] = 0x03;

		//cmd[3] = aidcount/D_MaxServiceNum + 1;
		cmd[3] = smsCount;		        //修改短信总数错误问题

		if (smsIndex == cmd[3])
		{
			countAid = aidcount - (smsIndex - 1)*D_MaxServiceNum;
		}
		else
		{
			countAid = D_MaxServiceNum;
		}

		cmd[4] = smsIndex;

		cmd[5] = countAid;

		if(menuId == 0x00)
		{
			fileAddr = SelectLFFile(DF_OTA, EF_Menu);

			for (i= 0x00,result =0x00;(i < GetLFRecNumLgth(fileAddr,FileNum))&&(result<countAid); i ++)
			{
				ReadLFBuf(fileAddr, i+1, 0x00, &ResData[0], 0x05);

				if((ResData[0] == 0x04)&& ((ResData[4]&0x40) == 0x40)) //查找打待删除标记的目录
				{
					SetLFConst(fileAddr, i+1, 0x04, ResData[4]&0x88, 0x01);	   //还原目录删除标记

					if(ResData[2] <0xA0)  //一级目录
					{
						cmd[6 + result *3] = ResData[2];

						cmd[6 + result *3 + 1] = 0x00;

						cmd[6 + result *3 + 2] = 0x00;
					}
					else    //一级应用
					{
						G_STKAppOffset = 0x00;

						datalen = OTASearchAID(ResData[2], D_AllAID, FALSE);

						if(datalen != FALSE)
						{
							ReadTFBuf(G_AppFileAddr, G_STKAppOffset, &cmd[6 + result *3],0x03);

							OTADeleteApp(TRUE, D_DelTagApp);

							SetLFConst(fileAddr,i+1,0x00,0xFF,GetLFRecNumLgth(fileAddr, FileLgth));

							PendingMessageAlert(D_SetUpMenuTag,0x00);//仅当删除的目录中有一级应用时才setup menu

#ifdef LOCALOTA
							G_LevelApp = TRUE;
#endif
						}
					}

					result ++;
				}
			}
		}
		else
		{
#ifdef LOCALOTA
			G_LevelApp = TRUE;
#endif
			
			for (G_STKAppOffset = 0x00,result = 0x00; result < countAid; result ++, G_STKAppOffset += datalen)
			{
				datalen = OTASearchAID(0x00, D_DelNotFinishTagApp,FALSE);

				if (datalen != FALSE)
				{
					ReadTFBuf(G_AppFileAddr, G_STKAppOffset, &aid[0], 0x03);

					aid[1] &= 0xCF;

					memcpy(&cmd[6 + result *3], &aid[0], 0x03);

					//aid[1] |= D_DelTagApp;

					SetTFConst(G_AppFileAddr, G_STKAppOffset+1, aid[1], 0x01);	//回写AID，标记此AID已删除并发送删除通知

					OTADeleteApp(TRUE, D_DelTagApp);     //拼装删除确认通知时，将删除的应用加入下载列表

					IF_OTAAppDelNotice(&aid[0]);
				}
			}
		}

		cmd[2] = 3 + cmd[5]*3;

		cmd[0] = cmd[2] + 2;

		memcpy(tempBuf, &cmd[0],0xFF);

		result = OTASendUplinkCommandMessage(1, &tempBuf[0]);
	}

	return result;
}
/*-------------------------------------------------------------------------------------------------

Function Name	:	OTAMODownloadCmdData

Input parameter	:	aid:	下载的AID和应用空间

					buf:    存放拼装好的命令数据

Output parameter:		TURE/FALSE

Description		:	拼装MO下载请求的命令数据

Coder			:	xuqs

Status			:
--------------------------------------------------------------------------------------------------------*/
BYTE OTAMODownloadCmdData(BYTE * aid, BYTE * buf)
{
	WORD xdata freeSize, needSize;

	needSize = ((WORD)(aid[3]<<8)&0xFF00) + aid[4];

	freeSize = GetFreeAppSpaceSize();					//获取卡片空闲下载空间

	if(needSize > freeSize)							//系统空间不足！
	{
		cephDisplayTextSingle(0x00,UCS2,(BYTE CONST *)notenoughspace);

		return FALSE;
	}

	memcpy(&buf[3],&aid[0],0x03);

	buf[0] = 0x08;

	buf[1] = 0x01;

	buf[2] = 0x06;

	buf[6] = 0x00;

	buf[7] = (freeSize>>8)&0xFF;

	buf[8] = freeSize&0xFF;

	return TRUE;
}
/*----------------------------------------------------------------------------------------------

Function Name	:	OTAMenuManageDownload

input parameter	:	menuId: 指定被定位应用的目录ID。

output parameter:	成功:           TRUE
  　　　		  　失败:           FALSE
              		          		用户取消: D_BACKMOVE
           		                    用户退出: D_EXIT

description		:	处理菜单下载

Coder			:	xuqs

Status			:
----------------------------------------------------------------------------------------------*/
BYTE OTAMenuManageDownload(BYTE menuId)
{
	BYTE  idata result,type;
	BYTE  xdata pages[5];

	if(menuId == 0x00)
	{
		type = TYPE_Catalog_List;
	}
	else
	{
		type = TYPE_Download_List;
	}

	pages[0] = 1;

	result = OTAPageListInformation(type, &pages[0], 5 , &menuId);

	if( result != CMD_Success)						//用户选择取消或退出
	{
		return result;
	}
	else									//用户选择应用下载
	{
		ReadLFBuf(G_ServerListFileAddr, G_SelectedNum, R_OFFSET_AID, &pages[0],5 );

		result = OTAMODownloadCmdData(&pages[0], &tempBuf[0]);

		if (result == TRUE)
		{
			result = OTASendUplinkCommandMessage(1, &tempBuf[0]);
		}

		return result;
	}
}
/*
*@description:处理应用删除或目录删除操作
*
*@param      :menuId:0x00-删除目录 其他：删除应用
*
*@return     :TRUE :成功
*  　　　	  FALSE:失败
*		      BackMove：用户取消
*		      ExistSTKPerform：用户退出
*
*@history    :create by xyz@100603 15:45
*/
BYTE OTAMenuManageDelete(BYTE menuId)
{
	BYTE idata result;
	BYTE idata delcount,type;
	BYTE idata addTag;
	WORD idata datalen;
	UWORD xdata fileAddr;
	BYTE xdata pages[5];
	
	if(menuId == 0x00)
	{
		type = TYPE_Catalog_Dele;
		
		fileAddr = SelectLFFile(DF_OTA, EF_Menu);
	}
	else
	{
		type = TYPE_Menu_Dele;
	}
	
	pages[0] = 1;
	
	for(delcount = 0x00;;)
	{
		addTag = TRUE;
		
_PageList_:		
		result = OTAPageListInformation (type, &pages[0], 8, &menuId);//建立删除主菜单
		
		if(result == BackMove)
		{
			if(delcount == 0)		   	//用户按手机"取消"键
			{
				return BackMove;
			}
			else
			{
				addTag = FALSE;
			}
		}
		else if (result == ExitStkPerform)
		{
			goto _CancelDelTag_;
		}
		else
		{
			if(menuId == 0x00)
			{
				ReadLFBuf(fileAddr, G_SelectedNum, 0x02, &RecData[0], 0x01);
	
				for (G_STKAppOffset = 0;; G_STKAppOffset += datalen)
				{
					datalen = OTASearchAID(RecData[0],0x04,FALSE);   //查找是否存在未关闭的应用
	
					if(datalen != FALSE)
					{
#ifndef LOCALOTA
						if(RecData[0] > 0x9F) // 请先关闭应用！
						{
							cephDisplayTextSingle(0x00, UCS2, (BYTE CONST *)closeAPP); 
						}
						else   //该目录中含有未关闭应用，请先关闭应用
						{
							cephDisplayTextSingle(0x01, UCS2, (BYTE CONST *)closeAPPMulu); 
						}
#else					
						cephDisplayTextSingle(0x01, UCS2, (BYTE CONST *)closeMuluApp);  //业务未关闭						
#endif
						goto _PageList_;
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				ReadTFBuf(G_AppFileAddr, G_STKAppOffset+1, &RecData[0], sizeof(BYTE));
		
				if((RecData[0]&0x04) == 0x04)   //请先关闭应用
				{
#ifndef LOCALOTA
					cephDisplayTextSingle(0x00, UCS2, (BYTE CONST *)closeAPP);
#else
					cephDisplayTextSingle(0x01, UCS2, (BYTE CONST *)closeMuluApp);
#endif	
					return BackMove;
				}
			}
		}
		
		result = cephSelectItemSingle(0x00,UCS2,NULL,(BYTE CONST *)DeletionMgrMenu,sizeof(DeletionMgrMenu));

		if(result == BackMove)						//如用户按手机"取消"键，则卡片判断卡内本次删除记录列表中菜单记录是否为空，如为空，则不上发删除通知，如不为它，则执行卡内本次删除记录列表中全部菜单的删除操作后，上发删除通知。
		{
			break;
		}
		else if(result == ExitStkPerform)				//退出 ,需取消删除标记
		{
_CancelDelTag_:
			if(menuId == 0x00)
			{
				for (delcount= 0x00;delcount < GetLFRecNumLgth(fileAddr,FileNum); delcount ++)
				{
					ReadLFBuf(fileAddr, delcount+1, 0x00, &RecData[0], 0x05);
		
					if((RecData[0] == 0x04)&& ((RecData[4]&0x40) == 0x40)) //撤销删除标记
					{
						SetLFConst(fileAddr, delcount+1, 0x04, RecData[4]&0x88, 0x01);
					}
				
				}

			}
			else
			{
				for (G_STKAppOffset = 0x00; ; )
				{
					datalen = OTASearchAID(menuId, D_DelNotFinishTagApp,FALSE);

					if (datalen != FALSE)
					{
						 ReadTFBuf(G_AppFileAddr, G_STKAppOffset+1, &result, 0x01);

						 SetTFConst(G_AppFileAddr, G_STKAppOffset+1, result&0xCF, 0x01);
					}
					else
					{
						break;
					}
				}

			}
			return ExitStkPerform;
		}
		else	//for baihe-test
		{;}
		
		result = GetResponseItemIdentifier();

		if (addTag == TRUE)
		{
			delcount = delcount + 1;
		}
#ifndef LOCALOTA
		if((result == 0x01)||(result == 0x02))				//继续删除
		{
			if(menuId == 0x00)
			{
				ReadLFBuf(fileAddr, G_SelectedNum, 0x04, &RecData[0], 0x01);

				SetLFConst(fileAddr, G_SelectedNum, 0x04, RecData[0]|0x40, 0x01);        //打上删除标记

			}
			else
			{
				OTADeleteApp(FALSE, D_DelNotFinishTagApp);  //暂不添加进下载列表中，待点击“删除完成”后才加入下载列表
			}

			if (result == 0x02)					//删除完成
			{
				break;
			}
		}
#else
		if((result == 0x0D)||(result == 0x0C))				//继续删除
		{
			if(menuId == 0x00)
			{
				ReadLFBuf(fileAddr, G_SelectedNum, 0x04, &RecData[0], 0x01);

				SetLFConst(fileAddr, G_SelectedNum, 0x04, RecData[0]|0x40, 0x01);        //打上删除标记
			}
			else
			{
				OTADeleteApp(FALSE, D_DelNotFinishTagApp);	  //暂不添加进下载列表中，待点击“删除完成”后才加入下载列表
			}

			if (result == 0x0C)					//删除完成
			{
				break;
			}
		}
#endif
		else								//回退
		{
			if (addTag == TRUE)
			{
				delcount = delcount - 1;
			}
		}
	}
	
	if(delcount != 0 )
	{
		result = OTAAppDelNotice(menuId,delcount,&RecData[0]);			//组装并发送服务删除通知
		
#ifndef LOCALOTA
		if(result == TRUE )
		{
			cephDisplayTextSingle(0x00, UCS2,(BYTE CONST *)shanchuchenggong);	//删除成功
		}
		else
		{
			result = BackMove;   //本地OTA 中删除回退到应用下载 一级
		}
#else

		result = BackMove;
	
		if(menuId == 0x00)
		{
			result = BackMove;

			if(G_LevelApp == TRUE)
			{
				cephDisplayTextSingle(0x01, UCS2, (BYTE CONST *)deleted);

				result = ExitStkPerform;

				G_LevelApp = FALSE;  //还远标记值
			}
		}
		else
		{
			G_LevelApp = FALSE;
				
			cephDisplayTextSingle(0x01, UCS2, (BYTE CONST *)deleted);

			result = ExitStkPerform;   //本地OTA 中删除退回STK菜单 
		}
#endif
	}
	else
	{
		result = BackMove;
	}

	return result;	
}
/*--------------------------------------------------------------------------------------------------

Function Name	:	OTAListRefresh

input parameter	:	menuId: 指定被定位应用的目录ID。

output parameter:	成功:     TRUE
  　　　		  　失败:     FALSE

description		:	处理列表更新

Coder			:	xuqs

Status			:
-------------------------------------------------------------------------------------------------*/
BYTE OTAListRefresh(BYTE menuId)
{
	tempBuf[0] = 0x04;

	tempBuf[1] = 0x02;

	tempBuf[2] = 0x02;

	OTAMenuAccess(menuId, MENU_OFFSET_PAGE_NUMBER, &tempBuf[3], ACCESS_READ, MenuType_OTA);	//当前列表页码

	tempBuf[4] = menuId;

	return OTASendUplinkCommandMessage(1, &tempBuf[0]);
}
/*---------------------------------------------------------------------------------------------------------

Function Name	:	OTAMenuSearchCmdData

input parameter	:	menuId: 指定被定位应用的目录ID。
					cmd:	存放拼装好的命令数据(LV)
					page:	当前页码
					needInput:关键字是否需要重新输入

output parameter:	成功:          TRUE
  　　　		  　失败:           FALSE
              		用户取消: D_BACKMOVE
           		    用户退出: D_EXIT

description		:	拼装菜单搜索命令数据

Coder			:	xuqs

Status			:
------------------------------------------------------------------------------------------------------------*/
BYTE OTAMenuSearchCmdData( BYTE menuId, BYTE * cmd, BYTE page, BYTE needInput)
{
	BYTE idata result;

	cmd[1] = 0x07;

	cmd[3] = page;						//当前页码

	cmd[4] = menuId;

	if (needInput == TRUE)				 	//关键字要求为UCS2编码格式
	{
		result = cephGetInputSingle(0x03,0x00,UCS2,(BYTE CONST *)guanjianzi ,0x01,0x10);

		if(result != CMD_Success)
		{
			return result;
		}

		GetResponseTextString(0x03,&cmd[5],0x10);

		memmove(&cmd[7], &cmd[6], cmd[5]);

		cmd[6] = 0x80;

		cmd[5] = cmd[5]+1;
	}
	else
	{
		cmd[5] = ReadVarible(SearchKeyWordTag, &cmd[6]);
	}

	cmd[2] = cmd[5] + 2 + 1;

	cmd[0] = cmd[2] + 2;

	return TRUE;
}

/*--------------------------------------------------------------------------------------------------
Function Name	:	OTAMenuManageSearch

input parameter	:	menuId: 指定被定位应用的目录ID。

output parameter:	成功:     TRUE
  　　　		  　失败:     FALSE
					用户取消: BackMove
           		    用户退出: ExistSTKPerform

description		:	处理菜单搜索

Coder			:	xuqs

Status			:
-------------------------------------------------------------------------------------------------*/
BYTE OTAMenuManageSearch(BYTE menuId)
{
	BYTE idata result;

	result = OTAMenuSearchCmdData(menuId, &tempBuf[0], 0x01, TRUE);

	if (result == TRUE)
	{
		result = OTASendUplinkCommandMessage(1, &tempBuf[0]);
	}

	return result;
}
/*---------------------------------------------------------------------------
Function Name	:	OTAMenuManageClose

input parameter	:    menuId

output parameter:	 无

description		:	OTA2应用关闭管理入口

author			:	xyz

test			:
---------------------------------------------------------------------------*/
BYTE OTAMenuManageClose(BYTE menuId)
{
	BYTE  idata result;
	BYTE  xdata pages[5];

	pages[0] = 1;

	result = OTAPageListInformation(TYPE_App_Close, &pages[0], 5 , &menuId);//应用关闭列表，bit14 = 0应用关闭

	if( result != CMD_Success)						//用户选择取消或退出
	{
		return result;
	}
	else
	{
		ReadTFBuf(G_AppFileAddr, G_STKAppOffset, &tempBuf[3], 0x03);

		tempBuf[0] = 0x06;

		tempBuf[1] = 0x0A;               //0x0A：应用开通\关闭请求

		tempBuf[2] = 0x04;

		tempBuf[6] = 0x02;               //应用关闭

		result = OTASendUplinkCommandMessage(1, &tempBuf[0]);

		return BackMove;
	}
}
/*-----------------------------------------------------------------------------------------------------------

Function Name	:	OTAServiceMgtMenu

input parameter	:	menuId:指定被定位应用的目录ID。

output parameter:	void

description		:	显示管理菜单

Coder			:	xuqs

Status			:
--------------------------------------------------------------------------------------------------------*/
BYTE OTAServiceMgtMenu(BYTE menuId)
{
	BYTE  idata result, item;

	while(1)
	{
		result = IF_OTAMgtMenuSelectItem(menuId);
	
		if(result != CMD_Success)
		{
			return result;
		}
	
		result = GetResponseItemIdentifier();
	
		if(OTAGetProcessPermission(result) == FALSE)			//判断前一请求是否完成。
		{
			continue;		
		}
	
		item = IF_OTAServiceMgtMenu(result, menuId);
	
		if(item == ERROR)
		{
			switch (result)
			{
				case 0x01:						//应用下载
				{
					result = OTAMenuManageDownload(menuId);
	
					break;
				}
				case 0x02:						//应用删除
				{
#ifdef LOCALOTA
					G_DeleteItem = TRUE;
#endif
					result = OTAMenuManageDelete(menuId);	
#ifdef LOCALOTA
					G_DeleteItem = FALSE;
#endif
					break;
				}
				case 0x03:						//列表更新
				{
					result = OTAListRefresh(menuId);
	
					break;
				}
				case 0x04:						//应用搜索
				{
					result = OTAMenuManageSearch(menuId);
	
					break;
				}
				case 0x05:                              		//应用关闭
				{
					result = OTAMenuManageClose(menuId);
	
					break;
				}
				default:	//for baihe-test
					break;
			}
		}
		else
		{
			result = item;
		}

		if (result == BackMove)
		{
			continue;
		}
		else
		{
			if (result == FALSE) 		//上行报文发送失败，卡片应给出提示(OTA和LOCALOTA规范都有说明)
			{
			 	cephDisplayTextSingle(0x00, UCS2,(BYTE CONST *)fasongshibai);

				return BackMove;
			}

			return ExitStkPerform;
		}
	}

}


/*--------------------------------------------------------------------------------------------------------------------

  Function name:	OTAMenuSearch

  Input parameter:

  Iutput parameter:	无

  Description:		对下行命令的菜单搜索的处理

  Coder:			xuqs

  status:
 -------------------------------------------------------------------------------------------------------------------*/
void OTAMenuSearch(void)
{
	BYTE  idata menuId, currentPage;
	BYTE  idata result, ptr;
	BYTE  xdata pages[5];

	tempBuf[0] = ReadVarible(MenuSearchResult, &tempBuf[1]);

	menuId = tempBuf[1];

	WriteVariable(SearchKeyWordTag, &tempBuf[2]);

	ptr = tempBuf[2] + 3;

	OTAClearCurrentSession();

	currentPage = tempBuf[ptr+1];

	pages[0] = 0x01;

	result = OTAPageListInformation(TYPE_Search_Result,&pages[0],5,NULL);

	if ((result == BackMove)||(result == ExitStkPerform))		//对于BIP传输流程，回退或退出时流程结束，需关闭通道
	{
		if ((G_OpenChannel&BIPNeedClosed) == BIPNeedClosed)
		{
			PendingMessageAlert(D_CloseChannelTag,0x00);

			return;
		}
	}

	if (result == ID_NEXTPAGE)
	{
		result = OTAMenuSearchCmdData(menuId, &tempBuf[0], currentPage +1, FALSE);
	}
	else if (result == CMD_Success)
	{
		result = OTAMODownloadCmdData(&tempBuf[G_SelectedNum], &tempBuf[0]);
	}
	else
	{
		return;
	}

	if (result == TRUE)
	{
		OTASendUplinkCommandMessage(1, &tempBuf[0]);
	}
}

/**
 * @brief 		UpdateDownloadConfig 更改承载方式
 *
 * @Returns   	CMD_Success
 *
 * @author 		xyz
 *
 * @history 	created by xyz, @081208
 */
void UpdateDownloadConfig(BYTE configByte)
{
	UWORD xdata fileAddr;

	fileAddr = SelectTFFile(DF_OTA,EF_OTAConfig);

	if(fileAddr == FALSE)
	{
		return;
	}

	SetTFConst(fileAddr,0x04,configByte,sizeof(BYTE));
}
/**
 * @brief 		OTAVerfiryPIN 验证PIN码，PIN验证失败不影响错误计数
 *
 * @Returns   	CMD_Success
 *
 * @author 		xyz
 *
 * @history 	created by xyz, @090303
 */
BYTE OTAVerfiryPIN(void)
{
	BYTE idata result,i;
	KeyStructer xdata pinkey;

_INPUT_:

	result = cephGetInputSingle(0x00, 0x00, UCS2, (BYTE CONST *)inputPIN1, 0x00, 0x08);	//提示输入PIN1

	if((result == ExitStkPerform)||(result == BackMove))
	{
		return result;
	}

	GetResponseTextString(0x00,&tempBuf[0], 0x08);	//获取输入的PIN码

	ReadKey(PIN1, (BYTE *)&pinkey.KeyId);		//从KEY文件中读取PIN1

	memcpy(&ResData[1], (BYTE *)&pinkey.KeyContent, sizeof(pinkey.KeyContent));

	ResData[0] = 0x00;

	for(i = 1;i <= 8;i++)        //统计PIN码的实际个数，排除后续的0xFF
	{
		if(ResData[i] == 0xFF)
		{
			break;
		}
		ResData[0] ++;
	}

	if( (memcmp(&ResData[1], &tempBuf[1], tempBuf[0]) != EQUAL) || (ResData[0] != tempBuf[0]))	//PIN1校验失败
	{
		cephDisplayTextSingle(0x00, UCS2, (BYTE CONST *)PINError);

		goto _INPUT_;
	}

	return CMD_Success;
}

#ifdef LOCALOTA
/**
 * @brief 		UpdateOTADownloadSet 本地OTA规范和江苏OTA规范中的"下载设置"菜单
 *
 * @Returns   	CMD_Success
 *
 * @author 		xyz
 *
 * @history 	created by xyz, @090305
 */
void UpdateOTADownloadSet(void)
{
	BYTE  idata result,item ;

	while(1)
	{
		result = IF_OTADownloadSetItem();

		if(result != CMD_Success)
		{
			return;
		}
		result = GetResponseItemIdentifier();

		item = IF_OTADownloadSetMenu(result);

		if(item == ERROR)									
		{
			switch(result)
			{
				case 0x01:
				case 0x02:
				{
					result--;
	
					UpdateDownloadConfig(result);
	
					if (cephDisplayTextSingle(0x00, UCS2, (BYTE CONST *)SetSuccess) == ExitStkPerform)
					{
						return;
					}
	
					break;
				}
				case 0x03:
				{
					
					result = IF_UpdateOTADownloadSet();
						
					//result = cephSelectItemSingle(0x00, UCS2, NULL, (BYTE CONST *)UpdateParameterSet, sizeof(UpdateParameterSet));
					
					if(result == ExitStkPerform)
					{
						return ;
					}
					else if(result == BackMove)
					{
						break;
					}
					else	//for baihe-test
					{;}
					result = GetResponseItemIdentifier();
				
					tempBuf[0] = 0x03;
	
					tempBuf[1] = 0x0E;     

					tempBuf[2] = 0x01;
	
					tempBuf[3] = result -1;
	
					OTASendUplinkCommandMessage(1, &tempBuf[0]);
				
					return;
				}
				default:
					return;
			}
		}
		else
		{
			if(item == BackMove)
			{
				;
			}
			else
			{
				return;
			}
		}
			
	}
}
#else

/**
 * @brief 		UpdateIP 更新IP地址
 *
 * @Returns   	CMD_Success
 *
 * @author 		xyz
 *
 * @history 	created by xyz, @081210
 */
BYTE UpdateIp(void)
{
	BYTE idata result;
	BYTE idata i,j,k;
	UWORD xdata fileAddr;

	fileAddr = SelectTFFile(DF_OTA,EF_OTAConfig);

GETINPUT:
	result = cephGetInputSingle(0x01, OTAIpTag, UCS2, (BYTE CONST *)inputIP, 7, 15);	//ip地址不包括端口号

	if(result == CMD_Success)
	{
		RecData[0] = ReadVarible(OTAIpTag, &RecData[1]);

		i = 0x01;
		j = 0x00;
		k = 0x00;   //统计 . 的数目
		ResData[0] = 0x00;

		while(i <= RecData[0])	//存储IP方案:已"."为界把IP分为四个LV结构，然后分别转化为HEX，把HEX存入配置文件即可
		{
			j = j + 1;	//从value(上一个"."除外)开始查找

			for (; j <= RecData[0]; j++)
			{
				if (RecData[j] == '.')
				{
					k ++;
					break;
				}
			}

			tempBuf[0] = j - i;

			memcpy(&tempBuf[1], &RecData[i], tempBuf[0]);

			if(TenToSixteen(&tempBuf[0], &result)== FALSE)  //待转换的字符长度和或内容无效
			{
				cephDisplayTextSingle(0x00, UCS2, (BYTE CONST *)IPError);

				goto GETINPUT;
			}

			ResData[0] ++;

			ResData[ResData[0]] = result;

			i = j+1; //下一个"."之前的字节的偏移
		}

		if(k != 0x03)  //输入的IP地址不合要求
		{
			cephDisplayTextSingle(0x00, UCS2, (BYTE CONST *)IPError);

			goto GETINPUT;
		}

		WriteTFBuf(fileAddr, 0x05, &ResData[1], ResData[0]);
	}

	return result;
}

/**
 * @brief 		UpdeTPDA 更新短信接入码
 *
 * @Returns   	CMD_Success
 *
 * @author 		xyz
 *
 * @history 	created by xyz, @081210
 */
BYTE UpdeTPDA(void)
{
	BYTE idata result;
	UWORD xdata fileAddr;

	result = cephGetInputSingle(0x00, OTATPDATag, UCS2, (BYTE CONST *)inputTPDA, 1, 20);

	if (result == CMD_Success)
	{
		RecData[0] = ReadVarible(OTATPDATag, &RecData[1]);

		AsciiToADN(&RecData[0], &ResData[1]);

		ResData[0] = ResData[1] + 1;

		ResData[1] = RecData[0];	//修改内容长度

		if(RecData[1] == '+')		//如果包括"+"，"+"号不包括在tpda字节中
		{
			ResData[1]--;
		}

		fileAddr = SelectLFFile(DF_OTA, EF_TPDA);

		WriteLFBuf(fileAddr, 1, 0x00, &ResData[0], ResData[0]+1);    //更新6EAC中的上行TPDA
	}

	return result;
}

/**
 * @brief 		OTADownloadSet 下载设置,标准OTA规范中的菜单，版本号：V1.0.0
 *
 * @Returns   	CMD_Success
 *
 * @author 		xyz
 *
 * @history 	created by xyz, @081205
 */
void OTADownloadSet(void)
{
	BYTE  idata result,item ;

	while(1)
	{
		result = IF_OTADownloadSetItem();

		if(result != CMD_Success)
		{
			return;
		}
		result = GetResponseItemIdentifier();

		item = IF_OTADownloadSetMenu(result);

		if(item == ERROR)
		{
			switch(result)
			{
				case 0x01:
				case 0x02:
				{
					result--;
	
					UpdateDownloadConfig(result);
	
					cephDisplayTextSingle(0x00, UCS2, (BYTE CONST *)SetSuccess);
	
					break;
				}
				case 0x03:
				{
					//注:在STK/USAT应用中验证PIN码不应导致PIN码的锁定
	
					result = OTAVerfiryPIN();
	
					if(result == ExitStkPerform)
					{
						return ;
					}
					else if(result == BackMove)
					{
						break;
					}
					else
					{
	INPUT_PARAMETR:
						result = cephSelectItemSingle(0x00, UCS2, NULL, (BYTE CONST *)ParameterSet, sizeof(ParameterSet));
	
						if(result == ExitStkPerform)
						{
							return ;
						}
						else if(result == BackMove)
						{
							break;
						}
						result = GetResponseItemIdentifier();
	
						switch(result)
						{
							case 0x01:              //设置短信接入码
								result = UpdeTPDA();
								break;
							case 0x02:              //设置IP地址
								result = UpdateIp();
								break;
						}
						if(result == ExitStkPerform)
						{
							return ;
						}
						else if(result == BackMove)
						{
							goto INPUT_PARAMETR;
						}
					}
					break;
				}
				default:
					return;
			}
		}
	}
}
#endif


#endif
