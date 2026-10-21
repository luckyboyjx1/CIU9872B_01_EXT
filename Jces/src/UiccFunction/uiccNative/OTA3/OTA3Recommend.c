/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.0

 * MODEL: 	All model

 * RELEASE:	2008-5-13

 * AUTHER:	徐庆盛

 * HISTORY: V1.0 创建源文件

*/

#include <string.h>
#include "main.h"
#include "fileAction.h"
#include "stkhead.h"
#include "error.h"
#include "ceph.h"
#include "OTA3Head.h"
#include "fileSystem.h"
#include "interface.h"



#ifdef OTA

/*-----------------------------------------------------------------------------------------------------------------------
Function Name	:	OTARecommendation

input parameter	:   无

output parameter:	无

description		:	OTA2最新推荐的入口

Coder			:	caibo

Status			:
---------------------------------------------------------------------------*/
void OTARecommendation(void)
{
	BYTE idata result, item,itemID;
	BYTE xdata aid[5];
	BYTE xdata pages[4];

	aid[0] = 0x00;

	pages[0] = 1;

	while (1)
	{
		result = OTAPageListInformation(TYPE_Recommend_List, &pages[0], 8, &aid[0]);		//显示推荐列表(每页显示８个)

		if(result != CMD_Success)
		{
			return;
		}

_SELECT_:
		result = IF_RecommendationSelectItem();
	//	result = cephSelectItemSingle(0x00,UCS2,NULL,(BYTE CONST *)recommendation,sizeof(recommendation));

		if(result == ExitStkPerform)
		{
			  return;
		}
		else if(result == BackMove)
		{
			 continue;
		}
		else
		{
		 	result = GetResponseItemIdentifier();
		}

		itemID = IF_RecommendationSetItem(result);

		if(itemID == ERROR)
		{
			if(result == 0x01)						//发送下载请求
			{
				if(OTAGetProcessPermission(TRUE) == FALSE)		//判断前一请求是否完成。
				{
					goto _SELECT_;
				}
	
			 	ReadLFBuf(G_ServerListFileAddr, G_SelectedNum,R_OFFSET_AID, &aid[0], 5);
	
				OTAMODownloadCmdData(&aid[0], &tempBuf[0]);
	
				result = OTASendUplinkCommandMessage(1, &tempBuf[0]);
			}
			else								//业务简介
			{
				ReadLFBuf(G_ServerListFileAddr,G_SelectedNum, R_OFFSET_APP_NAME, &tempBuf[100], 29);
	
				item = 100 +tempBuf[100] +1;
	
				memcpy(&tempBuf[20], &tempBuf[item], tempBuf[item] +1);	//上行代码数据
	
				item = item + tempBuf[item] +1;
	
				memcpy(&tempBuf[1], &tempBuf[item], tempBuf[item] +1);	//目的号码数据
	
				result = tempBuf[1];
	
				tempBuf[1] = GetBCDCount(&tempBuf[2], tempBuf[1]);
	
				if(IF_SetOtaRecommendationPidDcs(&tempBuf[result+2]) == FALSE)
				{
					tempBuf[result+2] = 0x00;
	
					tempBuf[result+3] = 0x04;
				}
	
				memcpy(&tempBuf[result+4], &tempBuf[20], tempBuf[20]+1);
	
				tempBuf[0] = result + tempBuf[20] + 4;
	
				result = cephSendShortMessageSingle(0x00,NULL,NULL, &tempBuf[0]);
	
				if (result == CMD_Success)
				{
					result = TRUE;
				}
			}
		}
		else
		{
			result = itemID;
		}

#ifdef LOCALOTA
		if( result != TRUE )   //本地OTA 测试规范中，修改最新推荐回退流程
		{
			return ;
		}
#else
		if( result == TRUE )
		{
			return ;
		}
#endif
	}
}
#endif

