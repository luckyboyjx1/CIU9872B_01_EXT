/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.5

 * MODEL: 	All model

 * RELEASE:	2008-5-13

 * AUTHER:	徐庆盛

 * HISTORY: V1.0 	创建源文件
			
			V1.1    添加对命令数据进行循环冗余校验，当CRC值相同时不更新SIM卡；修改while循环判断条件

			V1.2    修改RecData下标上限的取值
*/

#include "main.h"		   
#include "error.h"
#include "fileSystem.h"
#include "fileAction.h"
#include "gsmpublic.h"
#include <string.h>
#include "resume.h"
#include "stkhead.h"
#include "arithmatic.h"
#include "Authentication.h"
#include "interface.h"
#include "ota3head.h"

#ifdef OTA
/**
 * @brief		更新0级菜单名
 *
 * @param 		cmd: 要更新的内容，LV结构
 *
 * @author		xuqs
 *
 * @history 	add comment by WG, @071221 10:38
 */
void UpdateSUME(BYTE * cmd)
{
	UWORD xdata fileAddr;

	memcpy((BYTE xdata *)&tempBuf[3], &cmd[2], cmd[1]);

	tempBuf[2] = 0x80;

	tempBuf[1] = cmd[1] + 1;

	fileAddr = SelectTFFile(DF_PHASE, EF_STKName);

	if (fileAddr != FALSE)
	{
		 tempBuf[0] = 0x85;

		 WriteTFBuf(fileAddr, 0x00, &tempBuf[0], tempBuf[1]+2);
	}
}



/**
 * @brief		品牌是否注册或者在未注册情况下修改品牌为已注册状态
 *
 * @param		type: 0x00:判断是否注册
 *					  0x01:设置已注册
 *
 * @Returns		TRUE/FALSE
 *
 * @author		xuqs
 *
 * @history 	add comment by WG, @071221 10:38
 */
BYTE BrandChangeRegisterStatus(BYTE type)
{
	return GetSetRegStatus(DF_OTA, EF_BrandConfig, type);
}



/**
 * @brief		品牌注册成功确认
 *
 * @param 		ud:(可以为RecData)，LV结构
 *
 * @Returns		TRUE/FALSE
 *
 * @author		xuqs
 *
 * @history 	add comment by WG, @071221 10:38
 */
#ifdef BrandChange
static BYTE code regAffirm[] = {0x04,0x50,0x50,0x5A,0x43};//注册确认信息

BYTE BrandChangeRegConfirmation(BYTE * ud)
{
	UWORD xdata fileAddr;

	fileAddr = SelectTFFile(DF_OTA, EF_BrandConfig);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	if(memcmp_p(ud, (BYTE CONST *)&regAffirm[0], sizeof(regAffirm)) != EQUAL)
	{
		return FALSE;
	}

	SetTFConst(fileAddr, 0x00, 0x80, sizeof(BYTE));

	PendingMessageAlert(D_OpenDisplayTag);					//显示品牌信息

	PendingMessageAlert(D_OTARegisterTag);			 		//品牌注册成功后，上发OTA注册请求

	return TRUE;
}
#endif



/**
 * @brief 		更新品牌数据到 EF_BrandCmdData ，call by BrandPPHanlder
 *
 * @param 		offset: TF内的偏移
 * @param 		pos: RecData[pos] == 品牌下行数据的TAG
 *
 * @author
 *
 * @history 	add comment by WG, @071221 10:38
 */
void UpdateBrandCmdData(BYTE offset, BYTE pos)
{
	UWORD xdata fileAddr;

	fileAddr = SelectTFFile(DF_OTA, EF_BrandCmdData);

	if (fileAddr != FALSE)
	{
		WriteTFBuf(fileAddr, offset, &RecData[pos], RecData[pos+1]+2);
	}
}



/**
 * @brief 		对品牌可视化下行数据的解析处理，请参考操作手册对 EF_BRANDINFO、EF_BrandCmdData 的描述
 *
 * @param 		pos:
 *
 * @Returns   	TRUE/FALSE
 *
 * @author 		xuqs
 *
 * @history 	add comment by WG, @071221 10:38
 */
BYTE BrandPPHanlder(BYTE pos)
{
	BYTE  xdata key[16];
	BYTE  xdata random[4], mac[4];
	BYTE  idata len,tag;
	WORD idata crc16;
#ifdef CMDTYPE
   	BYTE  idata result;
#endif
	UWORD idata cmdfileAddr,fileAddr;

    cmdfileAddr = SelectTFFile(DF_OTA, EF_BrandCmdData);

	memcpy(&random[0], (BYTE xdata *)&RecData[pos+24], 4);  //保存随机数

#ifdef DaTangKey
	if(OTAGetKey(&random[0], &key[0], D_BrandKey) == FALSE)
	{
		return FALSE;
	}
#else
	if(OTAGetKey(&random[0], &key[0], D_ZJ_BrandKey) == FALSE)
	{
		return FALSE;
	}
#endif

#ifdef CMDTYPE
	len = TRUE;
	result = RecData[pos+28];
#else
	len = FALSE;
#endif

	memmove((BYTE xdata *)&RecData[pos+8+len], (BYTE xdata *)&RecData[pos+4], 20);

	pos += 8 + len;								//point to CPL

	TripleDES((BYTE xdata *)&RecData[pos+10], key, (BYTE xdata *)&RecData[pos+10], ECB_Decrypt, (RecData[pos+1]-12) - len);  //从CNTR进行数据解密

	memcpy(&mac[0], &RecData[pos+16], 4);

	memcpy(&RecData[pos+16], &random[0], 4);

#ifdef CMDTYPE
	if(len == TRUE)
	{
		pos--;
		
		memmove((BYTE xdata *)&RecData[pos], (BYTE xdata *)&RecData[pos+1], 20);

		RecData[pos+20] = result;
	}
#endif
	if(OTACaluteMac((BYTE xdata *)&RecData[pos], &mac[0], key, RecData[pos+1]-2, 0x00) == FALSE)
	{
		return FALSE;
	}
	tag = FindTag(&RecData[pos+20+len],0x88,RecData[pos+1]);

	mac[0] = ( pos + 20 + len)+ ( RecData[pos+20+len+tag]+tag+1);         //RecData 的下标上限

//------------------------------------------------------------------------------------------------------------	
 //对命令数据的整体做CRC校验，上一次的CRC存放在6EC2的前两个BYTE ,若信息相同，不更新SIM卡
	tag = FindTag(&RecData[pos+20+len],0x88,mac[0]);

	tag = tag+RecData[pos+20+len+tag]+1;

	crc16 = CRC16Calc(&RecData[pos+20+len],tag);

	tempBuf[0] = ((crc16>>0x08)&0x00FF);

	tempBuf[1] = crc16&0xFF;
	
	if(memcmp_p(&tempBuf[0],&RecData[pos+20+tag+len],0x02) != EQUAL)  //下行数据中的CRC值与卡端计算出的CRC不相同，数据传输错误，退出
	{
		return FALSE;
	}

	ReadTFBuf(cmdfileAddr,D_CrcValue, &mac[1], 0x02);

	if(memcmp_p(&tempBuf[0],&mac[1],0x02) == EQUAL)  //不更新SIM卡
	{
		PendingMessageAlert(D_BrandChangeConfirmTag,0x00);		 		//更新成功上行确认信息

		return TRUE;
	}
	else
	{
		WriteTFBuf(cmdfileAddr,D_CrcValue, &tempBuf[0], 0x02);
	}
 //---------------------以上部分为循坏冗余校验部分，请根据需求自行决定是否需要该部分代码---------------------------------------------------------------
	pos += 20 + len;

	while(pos < mac[0])
	{
		if (RecData[pos+1] >= 1)	//TLV中的L至少为1
		{
			switch (RecData[pos])
			{
				// 请根据各省份不同的品牌协议，修改或添加不同的TAG
				case 0x82:
					UpdateBrandCmdData(D_UserLevel, pos);
					break;

				case 0x83:
					UpdateBrandCmdData(D_BrandDisplay, pos);
					break;

				case 0x84:
					UpdateBrandCmdData(D_UserLevelDisplay, pos);
					break;

				case 0x85:
					UpdateSUME(&RecData[pos]);
					PendingMessageAlert(D_SetUpMenuTag,0x00);
					break;

				case 0x86:
					UpdateBrandCmdData(D_AppId, pos);

					fileAddr = SelectTFFile(0x7F10,0x7B02);

					if((RecData[pos+2]&0x04) == 0x00)  //关闭口令安全认证功能 
					{
						SetTFConst(fileAddr,0x02,0x00,sizeof(BYTE));
					}
					else                               //激活口令安全认证功能
					{
						SetTFConst(fileAddr,0x02,0x01,sizeof(BYTE));
					}
					break;

				case 0x87:
					UpdateBrandCmdData(D_UserInf, pos);
					break;

				case 0x88:
					UpdateBrandCmdData(D_VipCardNum, pos);
					break;

				default:
					break;
			}
		}
		
		if(RecData[pos] == 0x88)
		{
			break;
		}
		else
		{
			pos = pos + RecData[pos+1] + 2;
		}
	}

	PendingMessageAlert(D_BrandChangeConfirmTag,0x00);		 		//更新成功上行确认信息

	BrandChangeRegisterStatus(1);						//设置品牌为已注册,并显示品牌信息

	PendingMessageAlert(D_OTARegisterTag,0x00);			 	//品牌注册成功后，上发OTA注册请求

	return TRUE;

}
#endif

