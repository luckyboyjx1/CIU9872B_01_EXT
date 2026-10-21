/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.3

 * MODEL: 	All model

 * RELEASE:	2008-5-13

 * AUTHER:	徐庆盛

 * HISTORY:	V1.0	创建源文件

 			V1.1    增加宏OLDOTA，老版本OTA功能（1字节表示过程字节长度、无事件触发型应用等）

			V1.2    增加接口函数IF_OTADownloadSetItem，IF_OTADownloadSetMenu，用于处理不同于标准的"下载设置"菜单
					修改IF_OTAExExFuntion():删除指令SPB_OTAAsciiToAdn 
					
			V1.3	修改函数OTASaveDownLoadData()，修改函数CheckLargeSms() 及其调用：修正大数据下载超过32条短信时卡端处理的BUG 

			V1.4    增加接口函数IF_RecommendationSelectItem，IF_RecommendationSetItem用于处理"应用推荐"时的菜单项

			V1.5    修改函数OTASaveDownLoadData,将待更新应用的长度由1字节改为两字节

			V1.6    修改函数OTASaveDownLoadData,修改当下载0x9F应用时，也同时删除下载列表中的对应ID

			V1.7    修改函数OTASaveDownLoadData，修改当某一短信已经收到时，直接抛弃该数据；修改不再根据续传标识的bit2判断该条是否为续传信息，直接由卡端进行判断

			V1.8    修改函数JSselectItemPlugin，修改为列表显示的最大名称长度为16bytes(即8个汉字或16个ASCII)，可根据需求自行修改(江苏需求)

			V1.9    修改函数JsFetionSort、DynamicJsFetionSort，修改for语句判断条件

			V1.10	增加函数ChannelStatus，增加通道状态下行事件处理函数

			V1.11	将OTP获取key的函数移植到接口文件。改变了获取key的方式。可以在库外按需求定义不同的key在密钥文件中的记录号。--xiaoyh 2010/3/22		
					
			V1.12   修改OTASendUplinkCommandMessage(), 添加OTAPATCH接口

			V1.13   修改函数OTAGetSearchResultItem，修改当一级应用搜索列表中出现目录时，对目录列表进行过滤 @20100713 by xyz

			V1.14	增加密宝功能（适用于上海、黑龙江、江苏等OTP需求），需添加宏配置MIBAO

			V1.15   接口文件中增加宏定义MIBAO_HLJ,以适应新的规范＜SIM密宝卡规范--适用于遵循移动OTA3规范的卡>　　added by xyz@20101119、
			
			V1.16   修改函数OTASearchAID，在整理应用空间或计算应用空间时，禁用的应用应该是有效的应用 modify @20101231 
			
			V1.17   修改函数OTASendUplinkCommandMessage，修改对于BIP模式处理上行数据时，数据偏移错误的问题 @20110729 by xuyz
			
			V1.18   修改函数OTASendUplinkCommandMessage,修改上发短信成功后,tempBuf缓冲区可能被冲掉的问题;修改函数OTAGetProcessPermission@20120213
			
			v1.19   修改函数OTASaveDownLoadData,修改其定义与声明,修改部分主要用于处理OTA BIP数据下载@20130821 by xuyz v1.37->1.38

			V1.20   增加接口函数IF_OTAPPHandlerMac，判断指令是否需要校验MAC  ---by xuyz 20140123 

			V1.21   修改IF_OTADownloadSetMenu的注释 @20140630 by xuyz

			V1.22   修改函数OTASaveDownLoadData，修改下载交互应用时，offset未初始化的问题@20140912 by xuyz
			
			V1.23   使用LOCALOTA时增加接口函数IF_UpdateOTADownloadSet，兼容UpdateParameterSet数组不同长度的情况   @20160121 by yangjian
*/


#include "main.h"
#include "gsmpublic.h"
#include "fileSystem.h"
#include "fileAction.h"
#include "stkhead.h"
#include "OTA3Head.h"
#include <string.h>
#include "ceph.h"
#include "authentication.h"
#include "interface.h"



#ifdef OTA

#ifdef NEWFETION
BYTE xdata G_JsFetion_Counter_Open;				//控制解释器执行SendSms 完成后是否对飞信计数器累加
#endif

BYTE code fasongshibai[]={0x08,0x53,0xD1,0x90,0x01,0x59,0x31,0x8D, 0x25};	//发送失败

BYTE code shanchuchenggong[] = {0x0A,0x52,0x20,0x96,0x64,0x62,0x10,0x52,0x9F,0xFF,0x01};	//删除成功!

BYTE code DeletionMgrMenu[]=
{
   	0x08, 0x7E, 0xE7, 0x7E, 0xED, 0x52, 0x20, 0x96, 0x64,			//继续删除
   	0x08, 0x52, 0x20, 0x96, 0x64, 0x5B, 0x8C, 0x62, 0x10,			//删除完成
    0x04, 0x56, 0xDE, 0x90, 0x00						//回退
};
BYTE code userSelect3[]={
 0x04, 0x51, 0x6C, 0x5F, 0x00,					//公开15
 0x06, 0x4E, 0x0D, 0x51, 0x6C, 0x5F, 0x00,			//不公开16
 0x06, 0x62, 0x11, 0x54, 0x0C, 0x61, 0x0F,			//我同意17
};

BYTE code userSelectDefault[] = {0x06,0x00, 0x5B, 0x00, 0x59, 0x00, 0x5D};//[Y]

BYTE code muluDownloadSuccess[]={18,0x4E,0x0B,0x8F,0x7D,0x62,0x10,0x52,0x9F,0xFF,0x0C,0x8B,0xF7,0x4F,0x7F,0x75,0x28,0xFF,0x01};	//下载成功，请使用！

BYTE code noFitNum[] =  {0x0C, 0x65, 0xE0, 0x53, 0x39, 0x91, 0x4D, 0x8B, 0xB0, 0x5F, 0x55,0xFF,0x01}; 		//无匹配记录

BYTE code  menuManager[]={0x8F,0x0A,ID_MENUMANAGE,0x80, 0x5E, 0x94, 0x75, 0x28, 0x7B, 0xA1, 0x74, 0x06};

BYTE code cardNotRis[]={0x0C, 0x00, 0x4F, 0x00, 0x54, 0x00, 0x41, 0x67, 0x2A, 0x6C, 0xE8, 0x51, 0x8C};
BYTE code rigsterInfor[]={0x0C, 0x00, 0x4F, 0x00, 0x54, 0x00, 0x41, 0x67, 0x2A, 0x6C, 0xE8, 0x51, 0x8C};

BYTE code newrecommd[] = {0x10, 0x67, 0x00, 0x65, 0xB0, 0x63, 0xA8, 0x83,0x50, 0x67, 0x09, 0x65, 0xB0, 0x5E, 0x94, 0x75, 0x28};	  //最新推荐有新应用

BYTE code newdownlist[] = {0x10, 0x4E, 0x0B, 0x8F, 0x7D, 0x52, 0x17, 0x88, 0x68, 0x66, 0xF4, 0x65, 0xB0, 0x62, 0x10, 0x52, 0x9F}; //下载列表更新成功

BYTE code cleardownlist[] = {0x0E, 0x4E, 0x0B, 0x8F, 0x7D, 0x52, 0x17, 0x88, 0x68, 0x5D, 0xF2, 0x6E, 0x05, 0x7A, 0x7A};//下载列表已清空

BYTE code clearpushlist[] = {0x0E,0x67,0x00,0x65,0xB0,0x63,0xA8,0x83,0x50,0x5D,0xF2,0x6E,0x05,0x7A,0x7A};

BYTE code regsucce[] = {0x0A, 0x6C, 0xE8, 0x51, 0x8C, 0x62, 0x10, 0x52, 0x9F, 0xFF, 0x01};//注册成功！

BYTE code brandregsucce[] = {0x0E, 0x54, 0xC1, 0x72, 0x4C, 0x6C, 0xE8, 0x51, 0x8C, 0x62, 0x10, 0x52, 0x9F, 0xFF, 0x01};	//品牌注册成功！
BYTE code OtaSmsAlpha[] = {0x00};//用户进行注册、删除、下载、更新等所有发短信到OTA服务器的菜单操作时的提示语的ALPHA标识(LV)

BYTE code processpermission[] = {0x0C, 0x65, 0x3E, 0x5F, 0x03, 0x4E, 0x0A, 0x6B, 0x21, 0x8B, 0xF7, 0x6C, 0x42, 	//放弃上次请求
 								 0x0C, 0x7E, 0xE7, 0x7E, 0xED, 0x4E, 0x0A, 0x6B, 0x21, 0x8B, 0xF7, 0x6C, 0x42	//继续上次请求
								 };
BYTE code processperbydis[] = {18,0x66,0x2F,0x54,0x26,0x65,0x3E,0x5F,0x03,0x4E,0x0A,0x6B,0x21,0x8B,0xF7,0x6C,0x42,0xFF,0x1F};//是否放弃上次请求?

BYTE code appClose[]={16,0x5E,0x94,0x75,0x28,0x6B,0x63,0x57,0x28,0x51,0x73,0x95,0xED,0x4E,0x2D,0x20,0x26};// 应用正在关闭中……

BYTE code  guanjianzi[]={0x08,0x51, 0x73, 0x95, 0x2E, 0x5B, 0x57, 0xFF, 0x1A};	///关键字

BYTE code notenoughspace[] = {0x0E,0x7C,0xFB,0x7E,0xDF,0x7A,0x7A,0x95,0xF4,0x4E,0x0D,0x8D,0xB3,0xFF,0x01};   //系统空间不足

BYTE code MenuSet[]={

	0x08,0x90,0x1A,0x8F,0xC7,0x77,0xED,0x4F,0xE1,                     //通过短信
	0x0C,0x90,0x1A,0x8F,0xC7,0x00,0x47,0x00,0x50,0x00,0x52,0x00,0x53, //通过GPRS
	0x08,0x53,0xC2,0x65,0x70,0x8B,0xBE,0x7F,0x6E,};                   //参数设置

BYTE code ParameterSet[]={

	0x0E,0x8B,0xBE,0x7F,0x6E,0x77,0xED,0x4F,0xE1,0x63,0xA5,0x51,0x65,0x78,0x01, //设置短信接入码
	0x0C,0x8B,0xBE,0x7F,0x6E,0x00,0x49,0x00,0x50,0x57,0x30,0x57,0x40};          //设置IP地址

BYTE code SetSuccess[]={0x0A,0x8B,0xBE,0x7F,0x6E,0x62,0x10,0x52,0x9F,0xFF,0x01}; //设置成功！

BYTE code inputPIN1[]={0x12,0x8B,0xF7,0x8F,0x93,0x51,0x65,0x00,0x50,0x00,0x49,0x00,0x4E,0x00,0x31,0x78,0x01,0xFF,0x1A};	// 请输入PIN1码：

BYTE code PINError[]={0x0A,0x68,0x21,0x9A,0x8C,0x59,0x31,0x8D,0x25,0xFF,0x01}; //校验失败！

BYTE code inputIP[]={0x10,0x8B,0xF7,0x8F,0x93,0x51,0x65,0x00,0x49,0x00,0x50,0x57,0x30,0x57,0x40,0xFF,0x1A}; 	//请输入IP地址:

BYTE code IPError[]={0x12,0x00,0x49,0x00,0x50,0x57,0x30,0x57,0x40,0x8F,0x93,0x51,0x65,0x95,0x19,0x8B,0xEF,0xFF,0x01};	//IP地址输入错误！

BYTE code inputTPDA[]={0x12,0x8B,0xF7,0x8F,0x93,0x51,0x65,0x77,0xED,0x4F,0xE1,0x63,0xA5,0x51,0x65,0x78,0x01,0xFF,0x1A}; //请输入短信接入码：

BYTE code closeChannel[]={0x1C, 0x00, 0x4F, 0x00, 0x54, 0x00, 0x41, 0x4E, 0x0B, 0x8F, 0x7D, 0x67, 0x2A, 0x5B, 0x8C, 0x62, 0x10, 0xFF, 0x0C, 0x66, 0x2F, 0x54, 0x26, 0x7B, 0x49, 0x5F, 0x85, 0xFF, 0x1F};//OTA下载未完成，是否等待？

BYTE code closeAPP[]={0x0E,0x8B,0xF7,0x51,0x48,0x51,0x73,0x95,0xED,0x5E,0x94,0x75,0x28,0xFF,0x01}; // 请先关闭应用！

BYTE code closeAPPMulu[]={34,0x8B,0xE5,0x76,0xEE,0x5F,0x55,0x54,0x2B,0x67,0x09,0x67,0x2A,0x51,0x73,0x95,0xED,0x5E,0x94,0x75,0x28,0xFF,0x0C,0x8B,0xF7,0x51,0x48,0x51,0x73,0x95,0xED,0x5E,0x94,0x75,0x28}; 	// 该目录含有未关闭应用，请先关闭应用  

#ifdef BrandChange
BYTE code updateSuccsee[] = {22,0x0C,0x81,0x01,0x56,0x04,0x01,0x10,0x00,0x00,0x00,0x0A,0x50,0x50,0x4B,0x53,0x48,0x20,0x51,0x52,0x58,0x58};  	//更新成功上行确认信息
BYTE code updateRequest[] = {26,0x0C,0x81,0x01,0x56,0x04,0x01,0x10,0x00,0x00,0x00,0x06,0x47,0x58,0x59,0x48,0x58,0x58};						//主动申请更新用户信息
#endif

#ifdef LOCALOTA
BYTE code recommendation[]={
	 0x04, 0x4E, 0x0B, 0x8F, 0x7D,	//下载
	 0x04, 0x7B, 0x80, 0x4E, 0xCB,  //简介
}; 

BYTE code yijicaidan[] = {8,0x4E,0x00,0x7E,0xA7,0x83,0xDC,0x53,0x55};//一级菜单

BYTE code xiazaichenggong[] = {10,0x4E,0x0B,0x8F,0x7D,0x62,0x10,0x52,0x9F,0xFF,0x01};//下载成功！

BYTE code xiade[] = {4,0x4E,0x0B,0x76,0x84};//下的

BYTE code deleted[]={8,0x5D,0xF2,0x52,0x20,0x96,0x64,0xFF,0x01}; 	// 已删除！

BYTE code UpdateParameterSet[]={
	
	0x10,0x66,0xF4,0x65,0xB0,0x77,0xED,0x4F,0xE1,0x63,0xA5,0x51,0x65,0x4E,0xE3,0x78,0x01, //更新短信接入码
	0x0C,0x66,0xF4,0x65,0xB0,0x00,0x49,0x00,0x50,0x57,0x30,0x57,0x40,          	//更新IP地址
};

BYTE code closeMuluApp[]={10,0x4E,0x1A,0x52,0xA1,0x67,0x2A,0x51,0x73,0x95,0xED}; //业务未关闭

BYTE code menuListUpdate[]={20,0x4E,0x00,0x7E,0xA7,0x83,0xDC,0x53,0x55,0x52,0x17,0x88,0x68,0x5D,0xF2,0x66,0xF4,0x65,0xB0, 0xFF,0x01}; 	// 0   一级菜单列表已更新!

BYTE code listUpdate[]={12,0x52,0x17,0x88,0x68,0x5D,0xF2,0x66,0xF4,0x65,0xB0,0xFF,0x01}; 	//列表已更新！ 

BYTE code localRegsucce[]={8,0x5D,0xF2,0x6C,0xE8,0x51,0x8C,0xFF,0x01};//已注册！   

#else

BYTE code recommendation[]={
	 0x08, 0x4E, 0x0B, 0x8F, 0x7D, 0x83, 0xDC, 0x53, 0x55,	//下载菜单
	 0x08, 0x4E, 0x1A, 0x52, 0xA1, 0x7B, 0x80, 0x4E, 0xCB   //业务简介
}; 

BYTE code downSuccess[] = {0x00};	//"应用下载成功的提示语"的前缀，默认无前缀

BYTE code downloadsuccess[] = {0x08,0x5D, 0xF2, 0x4E, 0x0B, 0x8F, 0x7D, 0x52, 0x30};	//已下载到

BYTE code muluzhong[] = {0x06,0x76,0xEE,0x5F,0x55,0x4E,0x2D};	//目录中
#endif

BYTE code  MenuMang[]={

	0x08,0x5E,0x94,0x75,0x28,0x4E,0x0B,0x8F,0x7D,   // 应用下载
	0x08,0x5E,0x94,0x75,0x28,0x52,0x20,0x96,0x64,   //应用删除
	0x08,0x52,0x17,0x88,0x68,0x66,0xF4,0x65,0xB0,   //列表更新
	0x08,0x5E,0x94,0x75,0x28,0x64,0x1C,0x7D,0x22,   //应用搜索
	0x08,0x5E,0x94,0x75,0x28,0x51,0x73,0x95,0xED,   // 应用关闭
};


#ifdef NEWFETION
#define IsBigger(aIndex,bIndexOfIndex)	((RecData[aIndex*2] > RecData[tempBuf[bIndexOfIndex]*2]) || ((RecData[aIndex*2] == RecData[tempBuf[bIndexOfIndex]*2])&&(RecData[aIndex*2 +1] > RecData[tempBuf[bIndexOfIndex]*2 + 1])))
/**
 * @brief 		江苏飞信菜单排序，按照使用频率由高向低，保存对应的应用序号(从0开始计数)到tempBuf中
 *
 * @author 		WG
 *
 * @history 	add comment by WG, @071220 22:37
 */
BYTE DynamicJsFetionSort(BYTE menuId)
{
	BYTE xdata aid[5];
	BYTE idata i,num,p,tmpIndex,j;
	WORD  idata fileSize,dataLen,offset;
	UWORD idata fileAddr;

	num = 0x00;

	i = 0x01;

	fileSize = GetTFSize(G_AppFileAddr);

	fileAddr = SelectTFFile(DF_OTA, EF_JS_FETION);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	for(offset = 0x00;offset <= fileSize; offset = offset+dataLen)
	{
		ReadTFBuf(G_AppFileAddr, offset+3, (BYTE *)&aid[0], 0x02);//读取应用长度

		dataLen = ((aid[0]<<8)&0xFF00) + aid[1];

		if(dataLen == 0xFFFF)						//判断是否到达应用末尾
		{
			break;
		}

		ReadTFBuf(G_AppFileAddr,offset,(BYTE *)&aid[0], 0x03);

		if((aid[0] != menuId)||((aid[1]&D_DelTagApp) == D_DelTagApp)||((aid[1]&D_EnableApp) != D_EnableApp))	//不是飞信应用，继续查找,或者应用已删除，已禁用
		{
			continue;
		}

		num ++;                                                  	//统计应用文件中飞信个数

		ReadTFBuf(fileAddr, 2*(aid[2]-1), &RecData[2*i -2], 0x02);	//存储对应的飞信使用次数

		i++;
	}

	for(p = 0 ; p < num ; p ++)						//初始化队列
	{
		tempBuf[p] = p;							//应用序号p以0开始计数,对应到第p+1个应用
	}

	for(p = 1; p < num ; p ++)						//进行num次排序
	{
		tmpIndex = tempBuf[p];

		for(j = p; (j > 0) && (IsBigger(tmpIndex,j-1) == TRUE); j --)	//大于前一个
		{
			tempBuf[j] = tempBuf[j-1];
		}

		tempBuf[j] = tmpIndex;
	}

	return TRUE;
}

/**
 * @brief 		在6EA4应用文件中，查找第index个飞信应用
 *
 * @param 		index:
 * @param 		menuId： 飞信AID
 *
 * @Returns   	TRUE/FALSE
 *
 * @author 		WG
 *
 * @history 	add comment by WG, @071220 22:37
 */
WORD DynamicFetionSearchAid(BYTE index,BYTE menuId)
{
	BYTE  idata i,aid[3];
	WORD  xdata fileSize,dataLen,offset;

	i = 0x00;

	fileSize = GetTFSize(G_AppFileAddr);

	for(offset = 0x00;offset <= fileSize; offset = offset+dataLen)
	{
		ReadTFBuf(G_AppFileAddr, offset+3, (BYTE *)&aid[0], 0x02);//读取应用长度

		dataLen = ((aid[0]<<8)&0xFF00) + aid[1];

		if(dataLen == 0xFFFF)						//判断是否到达应用末尾
		{
			return ErrorNoApp;
		}

		ReadTFBuf(G_AppFileAddr,offset,(BYTE *)&aid[0], 0x03);

		if((aid[0] != menuId)||((aid[1]&D_DelTagApp) == D_DelTagApp)||((aid[1]&D_EnableApp) != D_EnableApp))        //不是飞信应用，继续查找,或者应用已删除,禁用
		{
			continue;
		}

		if(i == index)
		{
			return offset;
		}

		i++;
	}

	return ErrorNoApp;
}

/**
 * @brief 		获取飞信应用名称
 *
 * @param 		suffix
 * @param 		buf
 *
 * @Returns   	TRUE/FALSE
 *
 * @author 		WG
 *
 * @history 	add comment by WG, @071220 22:37
 */
BYTE DynamicFetionGetAid(BYTE menuId,BYTE suffix,BYTE * buf)
{
	BYTE idata aid[3],num,index;
	WORD  xdata fileSize,dataLen,offset;

	num = 0x00;

	fileSize = GetTFSize(G_AppFileAddr);

	for(offset = 0x00;offset <= fileSize; offset = offset+dataLen)
	{
		ReadTFBuf(G_AppFileAddr, offset+3, (BYTE *)&aid[0], 0x02);//读取应用长度

		dataLen = ((aid[0]<<8)&0xFF00) + aid[1];

		if(dataLen == 0xFFFF)						//判断是否到达应用末尾
		{
			break;
		}

		ReadTFBuf(G_AppFileAddr,offset,(BYTE *)&aid[0], 0x03);

		if((aid[0] != menuId)||((aid[1]&D_DelTagApp) == D_DelTagApp)||((aid[1]&D_EnableApp) != D_EnableApp))        //不是飞信应用，继续查找,或者应用已删除，禁用
		{
			continue;
		}

		num ++;
	}

	if(suffix > num)                                              //应用查找完毕
	{
		return FALSE;
	}

	index = tempBuf[suffix-1];

	G_STKAppOffset = DynamicFetionSearchAid(index,menuId);

	if(G_STKAppOffset == ErrorNoApp)
	{
		return FALSE;
	}

	ReadTFBuf(G_AppFileAddr, G_STKAppOffset+5,&buf[0],sizeof(BYTE));	//获取应用名称

	ReadTFBuf(G_AppFileAddr, G_STKAppOffset+5,&buf[0],buf[0]+1);

	return TRUE;
}

#endif
/**
 * @brief 		在文件中搜索符合条件的AID，并将偏移存入G_AppOffset
 *
 * @param 		menuId 		需要查找AID的类型
 * @param 		searchType 	查找过滤条件
 * @param 		onOff 		过滤条件选择开关:(FALSE: 查找符合类型的AID	；TRUE:  排除符合类型的AID)
 *
 * @Returns   	FALSE:			查找失败
  				datalen:		应用大小
 *
 * @author 		WG
 *
 * @history 	add comment by WG, @071220 22:37
 */
WORD OTASearchAID(BYTE menuId, BYTE searchType, BYTE onOff)
{
	WORD  xdata fileSize,dataLen;
	BYTE  xdata tmp[5];

	fileSize = GetTFSize(G_AppFileAddr);

	for(;G_STKAppOffset<=fileSize; G_STKAppOffset = G_STKAppOffset+dataLen)
	{
		ReadTFBuf(G_AppFileAddr, G_STKAppOffset, &tmp[0], sizeof(tmp));

		dataLen = ((tmp[3]<<8)&0xFF00) + tmp[4];

		if(dataLen == 0xFFFF)						//判断是否到达应用末尾
		{
			return FALSE;
		}

		if (onOff == FALSE)	 					//查找符合类型的AID
		{
			if((tmp[1]&searchType) != searchType)
			{
				continue;
			}
		}
		else								//排除符合类型的AID
		{
			if(searchType == D_DelMoTagApp)     //列出应用删除列表时 ，当已删除或不允许MO删除的应用，均不出现在删除列表中
			{
				if(((tmp[1]&0x20) == 0x20)||((tmp[1]&0x08) == 0x08)||((tmp[1]&D_EnableApp) != D_EnableApp))
				{
					continue;
				}
			}
			else if(searchType == D_CloseApp)   //bit14 = 0 不允许出现在应用关闭列表
			{
				if(menuId == 0x00)              //一级应用关闭
				{
					if(tmp[0] <0xA0)
					{
						continue;
					}
				}
				if(((tmp[1]&0x20) == 0x20)||((tmp[1]&0x04) == 0x00)||((tmp[1]&D_EnableApp) != D_EnableApp))
				{
					continue;
				}
#ifdef LOCALOTA
				if( (tmp[0] >= 0xA0)&&(tmp[0] <= 0xB7))  //本地OTA中可更名固化应用不允许出现在应用关闭列表
				{
					continue;
				}
#endif
			}
			else if(searchType == D_ListTagApp)   //显示应用列表，不包括已删除或禁用的应用
			{
				if(((tmp[1]&0x20) == 0x20)||((tmp[1]&D_EnableApp) != D_EnableApp))
				{
					continue;
				}		
			}
			else if(searchType == D_DelTagApp ) //在应用整理空间或计算应用空间时，禁用的应用是有效的
			{
				if((tmp[1]&0x20) == 0x20)
				{
					continue;
				}
			}
			else
			{
				if(((tmp[1]&searchType) == searchType) ||((tmp[1]&D_EnableApp) != D_EnableApp))
				{
					continue;
				}
			}
		}

		if((menuId != tmp[0]) && (menuId != 0x00))
		{
			continue;
		}

		return dataLen;
	}

	return FALSE;
}
/**
 * @brief 		查找匹配的搜索结果的列表数据
 *
 * @Returns   	TURE/FALSE
 *
 * @author 		WG
 *
 * @history 	add comment by WG, @071220 22:37
 */
BYTE OTAGetSearchResultItem(void)
{
	WORD idata offset;

	for (;G_LastNum < tempBuf[0];G_LastNum += tempBuf[G_LastNum+5] + 6)
	{

#ifdef LOCALOTA
//本地OTA 应用搜索结果中屏蔽 可更名固化应用
		if( (tempBuf[G_LastNum] >= 0xA0)&&(tempBuf[G_LastNum] <= 0xB7))
		{
			continue;
		}
		
#endif
		if((tempBuf[G_LastNum] < 0xA0)&&(tempBuf[G_LastNum+1] == 0x00)&&(tempBuf[G_LastNum+2] == 0x00))  //一级应用搜索中 搜索结果出现 目录数据
		{
			offset = OTAMenuAccess(tempBuf[G_LastNum], 0, NULL, ACCESS_LOCATE, MenuType_OTA);
			
			if(offset == FALSE)
			{
				offset = ErrorNoApp;
			}
		}
		else
		{
			offset =  OTAGetAppOffset((BYTE CONST *)&tempBuf[G_LastNum]);
		}

		if (offset == ErrorNoApp)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * @brief 		应用搜索结果列表
 *
 * @param 		num 	查找的第几个记录
 * @param 		buf 	存放查找结果
 *
 * @Returns   	TURE(找到有效的列表数据)
 				FALSE(未找到有效的列表数据)
				ErrorNoNextPage(无下一页)
 *
 * @author 		WG
 *
 * @history 	add comment by WG, @071220 22:37
 */
BYTE OTAGetSearchResultList(BYTE num , BYTE *buf)
{
	BYTE idata  result;

	if(tempBuf[tempBuf[2] + 5] == 0x00)  //列表总数为0时，卡端直接提示"无匹配记录"，不再判断"当前页码与总页数"
	{
		return ErrorNoNextPage;
	}

	if(num == 0x01)
	{
		G_LastNum = tempBuf[2] + 6;					//point to 列表数据
	}

	result = OTAGetSearchResultItem();

	if (result == TRUE)
	{
		memcpy(&buf[0], &tempBuf[G_LastNum+5], tempBuf[G_LastNum+5] + 1);

		G_FileNum = G_LastNum + 1;

		G_LastNum = G_LastNum + tempBuf[G_LastNum+5] + 6;

		return TRUE;
	}

	if (tempBuf[tempBuf[2] +4] == tempBuf[tempBuf[2] +3])         //当前页码等于于页码总数，不需要添加下一页
	{
		return ErrorNoNextPage;
	}

	return FALSE;
}

/**
 * @brief 		用户开机OTA注册过程:若OTA未注册时，先上发OTA注册命令，然后能进入OTA目录	,
				"OTA未注册"在用户进入"菜单管理"时才提示
 *
 * @author 		xuqs
 *
 * @history 	created by xuqs, @071220 20:01
 */
BYTE OTARigsterProgess(void)
{
	BYTE idata tmp;
	UWORD xdata fileAddr;
	WORD idata 	freeSize;

	tmp = GetSetRegStatus(DF_OTA, EF_OTAConfig, RegStatus_Get);

	if (tmp == FALSE)							//OTA未注册
	{	
		fileAddr = SelectTFFile(DF_OTA, EF_OTAConfig);

		if (fileAddr == FALSE)
		{
			return FALSE;
		}

		tempBuf[0] = 0x08;						//data length

		tempBuf[1] = 0x04;						//cmd type

		ReadTFBuf(fileAddr, 0x03, &tempBuf[3],sizeof(BYTE)); 	//批次

		tempBuf[4] = 0x00;

		freeSize = GetFreeAppSpaceSize();

		tempBuf[5] = (freeSize >> 0x08) & 0x00FF;

		tempBuf[6] = freeSize & 0x00FF;

		ReadTFBuf(fileAddr, 0x0B, &tempBuf[7],0x02);          		//COS 版本号，区分SIM和USIM
		
		tempBuf[2] = tempBuf[0] - 2;								//cmd length

		OTASendUplinkCommandMessage(1, &tempBuf[0]);			//上发用户注册命令
        
    return FALSE;
	}
    
    return TRUE;
}

/**
 * @brief 		OTA应用的入口函数：包括OTA应用和菜单管理的入口
 *
 * @param 		menuId ；OTA目录ID
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
#if defined FETION_SORT ||defined NEWFETION
void OTAMenuHandler(BYTE * menuId)
{
	BYTE  idata result;
	BYTE  xdata pages[6];

	G_STKAppOffset = 0x00;

	G_JsFetion_Counter_Open = FALSE;

	pages[0] = 1;

	while(1)
	{
		if (menuId[0] == 0x05)	             //飞信目录ID = 0x05//飞信应用主菜单
		{
			G_JsFetion_Counter_Open = TRUE;

			DynamicJsFetionSort(menuId[0]);

			result = OTAPageListInformation(TYPE_FetionApp_Name,&pages[0],8,menuId);
		}
		else
		{
			result = OTAPageListInformation(TYPE_App_Name,&pages[0],8,menuId);//建立OTA应用主菜单(该目录下的AID按在脚本文件中存储的前后存储在TEMPBUFFER中)。
		}

		if (result == ID_MENUMANAGE)					//菜单管理
		{
			if(GetSetRegStatus(DF_OTA, EF_OTAConfig, RegStatus_Get) == FALSE)
			{
				cephDisplayTextSingle(0x00,UCS2,(BYTE CONST *)rigsterInfor);	//OTA未注册

				continue;
			}

			result = OTAServiceMgtMenu(menuId[0]);			//建立SelectItem,显示为下载菜单、删除菜单、更新列表,搜索菜单

			if (result == BackMove)
			{
				continue;
			}
		}
		else if( result == CMD_Success)
		{						          	//根据序号取得AID.
			result = OTAStartRun();					//解释器执行

			G_STKAppOffset = FALSE;

			G_JsFetion_Counter_Open = FALSE;
		}

		if((result == ExitStkPerform)||(result == BackMove))
		{
			return;
		}
	}
}

#else
void OTAMenuHandler(BYTE * menuId)
{
	BYTE  idata result,menu[3];
	BYTE  xdata pages[6];

	G_STKAppOffset = 0x00;

	pages[0] = 1;

	while(1)
	{
		result = OTAPageListInformation(TYPE_App_Name,&pages[0],8,menuId);//建立OTA应用主菜单(该目录下的AID按在脚本文件中存储的前后存储在TEMPBUFFER中)。

		if (result == ID_MENUMANAGE)					//菜单管理
		{
			if(GetSetRegStatus(DF_OTA, EF_OTAConfig, RegStatus_Get) == FALSE)
			{
				cephDisplayTextSingle(0x00,UCS2,(BYTE CONST *)rigsterInfor);	//OTA未注册

				continue;
			}

			result = OTAServiceMgtMenu(menuId[0]);			//建立SelectItem,显示为下载菜单、删除菜单、更新列表,搜索菜单

			if (result == BackMove)
			{
				continue;
			}
		}
		else if( result == CMD_Success)  //根据序号取得AID.
		{
			ReadTFBuf(G_AppFileAddr, G_STKAppOffset, &menu[0], 0x03);
	  			           			
			if(OTAVerifyCurrentSession(menu, D_AppClose) == TRUE)
			{
				cephDisplayTextSingle(0x01,UCS2,(BYTE CONST *)appClose);

				return;
			}
			
			result = OTAStartRun();					//解释器执行

			G_STKAppOffset = FALSE;
		}
		else	//for baihe-test
		{;}

		if((result == ExitStkPerform)||(result == BackMove))
		{
			return;
		}
	}
}
#endif
/**
 * @brief 		上行短信发送入口函数1
 *
 * @param 		void
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
void GetUplinkMessage(BYTE pos, BYTE *cmdData ,BYTE mode)
{
	BYTE xdata pwd[16];
	BYTE xdata mac[4];
	UWORD xdata fileAddr;
	
	if(mode == 0x00)
	{
		fileAddr = SelectTFFile(DF_OTA, EF_OTAConfig);
	}

	ReadTFBuf(fileAddr, 0x01, &RecData[pos], 0x02);				//卡商代码及协议版本号,品牌标识

	fileAddr = SelectTFFile(DF_PHASE, EF_IMSI);

	ReadTFBuf(fileAddr, 0x01, &RecData[pos+2], 0x08);			//卡片序列号

	fileAddr = SelectLFFile(DF_OTA, EF_CNTRFile);

	if(mode == 0x00)
	{
		ReadLFBuf(fileAddr, 0x01, 0x00, &RecData[pos+10], 0x05);	//同步计数器
	}
#ifdef OTP
	else
	{
		ReadLFBuf(fileAddr, 0x03, 0x00, &RecData[pos+10], 0x05);		//同步计数器
	}
#endif

	Random_Number_Generator_Get(&RecData[pos+15], 4);

	memcpy(&RecData[pos+19], &cmdData[1], cmdData[0]);			//组装校验MAC的数据

	if(mode == 0x00)
	{
		OTAGetKey(&RecData[pos+15], &pwd[0], D_OTAKey);				//获取OTAkey
	}

	OTACaluteMac(&RecData[pos], &mac[0], &pwd[0], 19+cmdData[0], 0x01);	//计算MAC

	memcpy(&RecData[pos+19], &mac[0], 0x04);

	memcpy(&RecData[pos+23], &cmdData[1], cmdData[0]);			//数据拼装完成
}
/**
 * @brief 		上行短信发送入口函数2
 *
 * @param 		void
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
BYTE OTASendUplinkCommandMessage(BYTE tpdaIndex,BYTE *cmdData)
{
	BYTE xdata pwd[16];
	BYTE idata pos;
	BYTE idata tmp;
	//BYTE xdata mac[4];
	BYTE idata st;
	UWORD xdata fileAddr;
	BYTE xdata bufType[0x04];  //存储命令类型，AID(3个字节)

	memset(&RecData[0], 0, sizeof(RecData));

	fileAddr = SelectTFFile(DF_OTA, EF_OTAConfig);

	if (G_ResponseMode == Mode_EF)	//根据G_ResponseMode来选择上行的承载方式
	{
		ReadTFBuf(fileAddr, 0x04, &G_ResponseMode, sizeof(BYTE));
	}

	if(cmdData[1]==0x09)  //主动获取品牌信息，只通过短信承载方式发送
	{
		G_ResponseMode = Mode_SMS;
	}
#ifndef LOCALOTA
	if(cmdData[1]==0x08)  //OTA3中规定远程文件管理确认 ，只通过短信承载方式发送
	{
		G_ResponseMode = Mode_SMS;
	}
#endif
	if (G_ResponseMode == Mode_BIP)									//BIP模式
	{
		memmove(&cmdData[3], &cmdData[2], cmdData[2] + 1);  		//BIP通道用两个字节表示长度

		cmdData[2] = 0x00;

		cmdData[0] += 1;

		pos = 3;													//指向安全数据
	}
	else
	{
		fileAddr = SelectLFFile(DF_OTA, EF_TPDA);

		ReadLFBuf(fileAddr, tpdaIndex, 0x00, &tmp, sizeof(BYTE));

		ReadLFBuf(fileAddr, tpdaIndex, 0x01, &RecData[3], tmp);

		pos = tmp +3;

		RecData[pos] = 0x00;					//PID

		RecData[pos+1] = 0x04;					//DCS

		pos += 2;

		RecData[pos] = cmdData[0] + 23;				//User Data Length

		RecData[2] = RecData[pos] + tmp +3;		//拼装数据总长度

		pos ++;											//指向安全数据
	}

	GetUplinkMessage(pos, cmdData,0x00);

	if (G_ResponseMode == Mode_BIP)									//BIP模式
	{
		RecData[1] = 0x00;

		RecData[2] = cmdData[0] + 23;							//通道数据长度

		memcpy(&tempBuf[1], &RecData[1], RecData[2]+2);
		
		if(G_BipDataAddr == FALSE)
		{
			return FALSE;
		}
		
		
		bufType[0] = tempBuf[pos+23] ;  //备份命令类型
		
		if((bufType[0] == MO_Download) ||(bufType[0] == List_Update) ||(bufType[0] == Menu_Search)||(bufType[0] == D_AppClose))
		{	
			if ((bufType[0] == MO_Download)||(bufType[0] == D_AppClose))
			{
				st = 2+G_ResponseMode;
			}
			else
			{
				st = 3+G_ResponseMode;
			}
			
			memcpy(&bufType[1] ,&tempBuf[(pos+23)+st], 0x03);  //备份当前AID
		}
		
		
		WriteTFBuf(G_BipDataAddr, 0x00, &RecData[0], RecData[2]+3);

		tmp = OTASendUplinkCommandInBIPFormat(&tempBuf[1]);

		if(tmp == TRUE)
		{
			tmp = CMD_Success;
		}

		ReadTFBuf(G_BipDataAddr, 0x02, &tempBuf[2], 1);

		ReadTFBuf(G_BipDataAddr, 0x00, &tempBuf[0], tempBuf[2]+3);  //  重新将命令数据装载到tempBuf中，进行后续判断
	}
	else
	{
		LoadSCAddress(&pwd[0]);										//读短信中心号码

		memcpy(&tempBuf[2], &RecData[2], RecData[2]+1);
		
		bufType[0] = tempBuf[pos+23] ;  //备份命令类型
		
		if((bufType[0] == MO_Download) ||(bufType[0] == List_Update) ||(bufType[0] == Menu_Search)||(bufType[0] == D_AppClose))
		{	
			if ((bufType[0] == MO_Download)||(bufType[0] == D_AppClose))
			{
				st = 2+G_ResponseMode;
			}
			else
			{
				st = 3+G_ResponseMode;
			}
			
			memcpy(&bufType[1] ,&tempBuf[(pos+23)+st], 0x03);  //备份当前AID
		}
		
		tmp = cephSendShortMessageSingle(0x00, (BYTE CONST *)&OtaSmsAlpha[0], (BYTE *)&pwd[0], &tempBuf[2]);
	}

	if (tmp == CMD_Success)
	{
	   	if((bufType[0] == MO_Download) ||(bufType[0] == List_Update) ||(bufType[0] == Menu_Search)||(bufType[0] == D_AppClose))
		{

			OTARegisterCurrentSession(&bufType[1], bufType[0]);	//登记当前的命令
		}
     	
		tmp =  TRUE;
	}
	else
	{
		tmp = FALSE;
	}

	G_ResponseMode = Mode_EF;		//还原承载模式

	return tmp;
}


/**
 * @brief 		对OTA下行命令的计数器进行效验或重新设置计数器的值;

  				江苏版本对于计数器B允许0x0F长度的误差(通过宏WindowCntr处理)：如果下发的计数器B大于卡计数器，需要更新卡端，如果卡发的计数器B小于卡计数器，不需更新
 *
 * @param 		netCntr ：下行命令的计数器内容
 * @param 		cntrType ：下行命令的计数器的类型
 * @param 		optType ：操作类型（0x00:校验CNTR/0x01:设置CNTR）
 *
 * @Returns   	TRUE:成功 / FALSE:失败
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */

//#ifndef WindowCntr
BYTE OTAVarifyCNTR(BYTE netCntr[5], BYTE cntrType, BYTE optType)
{
	UWORD xdata fileAddr;
	BYTE  xdata cardCntr[5];

	fileAddr = SelectLFFile(DF_OTA, EF_CNTRFile);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadLFBuf(fileAddr, cntrType+1, 0x00, &cardCntr[0], sizeof(cardCntr));

	if(cntrType  ==D_CNTRA)		
	{
		if (optType == 0x00)
		{
			if (memcmp(&cardCntr[0], &netCntr[0], 5) != EQUAL)
			{
				IncreatCounter(&cardCntr[0], 5);

				if (memcmp(&cardCntr[0], &netCntr[0], 5) != EQUAL)
				{
					return FALSE;
				}
			}

			return TRUE;
		}
		else
		{
		 	IncreatCounter(&cardCntr[0], 5);				//Counter A 增量
		}
	}
	else if(cntrType == D_CNTRB)	
	{
		if (optType == 0x00)
		{
			if (memcmp_p(&netCntr[0], &cardCntr[0], 5) > EQUAL) 
			{
				return TRUE;
			}

			return FALSE;
		}
		else
		{
			memcpy(&cardCntr[0], &netCntr[0], 5);			//Counter B 使用网络下发的计数器
		}
	}
	else	//for baihe-test
	{;}

	if (optType == TRUE)
	{
		WriteLFBuf(fileAddr, cntrType+1, 0x00, &cardCntr[0], sizeof(cardCntr));
	}

	return FALSE;
}
//#else

/*
BYTE OTAVarifyCNTR(BYTE netCntr[5], BYTE cntrType, BYTE optType)
{
	UWORD idata fileAddr;
	BYTE  xdata cardCntr[5];
	BYTE idata tmp;
	BYTE xdata tmpCntr[5];			//需要增量的计数器，不可以修改传入 的参数

	fileAddr = SelectLFFile(DF_OTA, EF_CNTRFile);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadLFBuf(fileAddr, cntrType+1, 0x00, &cardCntr[0], sizeof(cardCntr));

	if(cntrType == D_CNTRA)
	{
		if (optType == 0x00)
		{
			if (memcmp(&cardCntr[0], &netCntr[0], 5) != EQUAL)
			{
				IncreatCounter(&cardCntr[0], 5);

				if (memcmp(&cardCntr[0], &netCntr[0], 5) != EQUAL)
				{
					return FALSE;
				}
			}

			return TRUE;
		}
		else
		{
		 	IncreatCounter(&cardCntr[0], 5);				//Counter A 增量
		}
	}
	else if((cntrType == D_CNTRB))
	{
		if (optType == 0x00)
		{
			if (memcmp_p(&netCntr[0], &cardCntr[0], 5) > EQUAL)
			{
				return TRUE;
			}

			tmp = 0x0F;

			memcpy(tmpCntr,netCntr,5);

			while(tmp --)
			{
				IncreatCounter(tmpCntr, 5);
			}

			if(memcmp_p(cardCntr, tmpCntr,5) > EQUAL)
			{
				return FALSE;
			}
			else
			{
				return 0x02;					//计数器可接收，但是不更新卡端计数器
			}
		}
		else
		{
			memcpy(&cardCntr[0], &netCntr[0], 5);			//Counter B 使用网络下发的计数器
		}
	}

	if (optType == TRUE)
	{
		WriteLFBuf(fileAddr, cntrType+1, 0x00, &cardCntr[0], sizeof(cardCntr));
	}

	return FALSE;
}
#endif
*/

/**
 * @brief 		得到OTA和品牌的Key,不支持处理WuYouKey
 *
 * @param 		random ：随即数的内容
 * @param 		key ：存储Key的Buffer
 * @param 		keyType ：KEY的类型	: D_OTAKey/D_RFMKeyD_ZJ_BrandKeyD_BrandKey(详见操作手册的对EF_6EA9的说明)
 *
 * @Returns   	TRUE:成功 / FALSE:失败
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
BYTE OTAGetKey(BYTE random[4], BYTE key[16], BYTE keyType)
{
	UWORD xdata fileAddr;
	BYTE  idata recordNum;
	BYTE  xdata imsi[8];

	fileAddr = SelectLFFile(DF_OTA,EF_OTA_KEY);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	switch(keyType)
	{
		case D_OTAKey:
		{
			recordNum = D_OtaKey_BaseRecNo;				
			break;
		}	
		case D_RFMKey:
		{
			recordNum = D_OtaRFMKey_BaseRecNo;
			break;
		}			
		case D_ZJ_BrandKey:
		{
			recordNum = D_ZJKey_BaseRecNo;
			break;
		}	
		case D_BrandKey:
		{
			recordNum = D_BrandKey_BaseRecNo;
			break;
		}

		
#ifdef OTP		
		case D_OTPKey:
		{
			recordNum = D_OTPKey_BaseRecNo;
			break;
		}
		case D_OTPRfmKey:
		{
			recordNum = D_OTPRFMKey_BaseRecNo;
			break;
		}	
#endif		
		default:
		{
			recordNum = 0;	//for baihe-test	
			break;
		}
	}

	if (keyType != D_BrandKey)
	{
		recordNum += ((random[3]&0x07)%5);
	}

	ReadLFBuf(fileAddr, recordNum, 0x01, &key[0], 16);

#ifdef Level3PBOC
	if (keyType == D_BrandKey)						//品牌密钥根据各省需求确定是否需要三级分散
	{
		fileAddr = SelectTFFile(DF_MF, EF_ICCID);

		if(fileAddr == FALSE)
		{
			return FALSE;
		}

		ReadTFBuf(fileAddr, 0x02, &imsi[0], 8);

		PBOCAuth(key, imsi);
	}
#endif

	fileAddr = SelectTFFile(DF_PHASE, EF_IMSI);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadTFBuf(fileAddr, 0x01, &imsi[0], 8);

	PBOCAuth(key, imsi);

	memset(&imsi[4], 0x20, 4);

	memcpy(&imsi[0], &random[0], 4);

	PBOCAuth(key, imsi);

	return TRUE;
}

BYTE code EndAID[] = {0xFF,0xFF,0xFF};
/**
 * @brief 		保存下载的OTA的数据信息到应用的文件。可通过定义宏SendReceivedAckLastGroup来确定大数据下载的最后一组是否上发大数据接受确认
 *
 * @param 		dataBuf ：下行命令的命令类型后的数据（不包括命令类型）
 * @param 		dataType ；命令类型
 *
 * @Returns   	TRUE:成功 / FALSE:失败
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
BYTE    xdata  G_BigResendTag;

#ifndef SendReceivedAckLastGroup
BYTE OTASaveDownLoadData(BYTE * dataBuf, BYTE dataType,BYTE pointer)
{
	UWORD xdata fileAddr;
	WORD  xdata tmpOffset, fileSize, appSize, offset;
	BYTE  idata largeCntr, result, groupAll, pos, datapos,i;
	BYTE  xdata smsTmp[11], aid[5];

	fileAddr = SelectLFFile(DF_OTA, EF_OTASecurity);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadLFBuf(fileAddr, 2, 0x00, &smsTmp[0], sizeof(smsTmp));

	pos = 0x05;

	datapos =  0x0D;

	if(smsTmp[0] != dataBuf[pos])	 //modify  at@080311 15:40  原 smsTmp[0] != dataBuf[4]
	{
		if(smsTmp[0] != 0xFF)
		{
			SetLFConst(fileAddr, 2, 0x00, 0xFF, 7);
		}

		smsTmp[0] = dataBuf[pos];		 			//保存批次

		smsTmp[2] = dataBuf[pos+1];					//保存短信总数

		smsTmp[1] = 0x00;						//已收短信数目

		memset(&smsTmp[3], 0xFF, 8);					//大数据下载清空登记信息
	}

	if(dataType == D_OTAInterDown)
	{
		smsTmp[1]++;

		groupAll = TRUE;
	}
	else
	{
		groupAll = IsDataSmsReceived(dataBuf[7], dataBuf[8], &smsTmp[3]);

		if(groupAll == TRUE)
		{
			smsTmp[1]++;
		}
	}

	if((dataType != D_OTAInterDown)&&(dataBuf[2] < 0xA0))		//modify at@080311 15:43 增加dataType!=D_OTAInterDown判断
	{
		  if(OTAMenuAccess(dataBuf[2], 0, NULL, ACCESS_LOCATE, MenuType_OTA) ==FALSE)	  //无对应目录，丢弃该数据
		  {
		  		return FALSE;
		  }
	}

	fileSize = GetTFSize(G_AppFileAddr);

	if(groupAll == TRUE)
	{
		offset =OTAGetAppOffset((BYTE CONST *)&EndAID[0]);	//应用空间整理完成后，应用文件的应用末尾偏移，也是下载应用在文件中的起始偏移

		if(dataType != D_OTAInterDown)
		{
			G_HandleResponse = 0x03;				//整理OTA应用数据前，要退出解释器的运行

			G_AppOffset = OTAGetAppOffset((BYTE CONST *)&dataBuf[2]);

			if(G_AppOffset != ErrorNoApp)
			{
				if(dataType == D_OTAAppDown)			//已经下载过的应用，不需要再下载
				{
					return FALSE;
				}

				ReadTFBuf(G_AppFileAddr, G_AppOffset+1, &result, sizeof(BYTE));
	
				result |= D_DelTagApp;
				
				SetTFConst(G_AppFileAddr, G_AppOffset+1, result, sizeof(BYTE));		//业务更新时，先删除原先的业务
		
		
			}			
			
			G_AppOffset = G_STKAppOffset;

			G_STKAppOffset = 0;

			OTAPureAppScript(fileAddr, 0);			//整理OTA应用数据

			G_STKAppOffset = G_AppOffset;

			offset =OTAGetAppOffset((BYTE CONST *)&EndAID[0]);	//应用空间整理完成后，应用文件的应用末尾偏移，也是下载应用在文件中的起始偏移

			G_AppOffset = (((WORD)dataBuf[11]<<8) + dataBuf[12]) + offset;	//本条短信的应用数据在文件中的偏移

			if ((G_AppOffset + 5) > fileSize)
			{
				return FALSE;
			}
		}
		else
		{
			appSize = ((dataBuf[pos+4]<<8)&0xFF00) + (dataBuf[pos+5]&0x00FF);  //交互应用所需空间

			if ((fileSize - offset - 5) < appSize)
			{
				return FALSE;
			}

			offset = ((dataBuf[pos+6]<<8)&0xFF00) + (dataBuf[pos+7]&0x00FF);	//计算交互应用的偏移(交互应用暂存在应用文件的最末尾)

			G_AppOffset = fileSize - (appSize+5)  + offset;		//本条短信的应用数据在文件中的偏移

			offset = fileSize - appSize - 5; 			//交互应用在文件中的起始偏移
		}

	  	if(dataBuf[pos+2] == 0x01)
		{
			result = WriteTFBuf(G_AppFileAddr, offset+5, &dataBuf[datapos+5], (((dataBuf[0]<<8)&0xFF00) + dataBuf[1])-datapos-3);//如果是第一条，不写AID和长度
		}
		else
		{
			result = WriteTFBuf(G_AppFileAddr, G_AppOffset, &dataBuf[datapos], (((dataBuf[0]<<8)&0xFF00) + dataBuf[1])- datapos+2);
		}

		if(result == FALSE)
		{
			return FALSE;
		}
	}

	if(smsTmp[1] == smsTmp[2])	//注意,	smsTmp[1]和smsTmp[2]不要覆盖,后面还需要用(江苏)//receive all sms
	{
		SetLFConst(fileAddr, 0x02, 0x00, 0xFF, sizeof(smsTmp));

		tmpOffset = (((WORD)dataBuf[pos+4]<<8)&0xFF00) + dataBuf[pos+5];

		if(dataType == D_OTAInterDown)
		{
			G_AppOffset = offset;					//交互应用解析器执行时用
		}

		if((tmpOffset+5+offset) > fileSize)
		{
			return FALSE;
		}

		SetTFConst(G_AppFileAddr, tmpOffset+offset, 0xFF, 5);		//write the end mark

		if(dataBuf[2] > 0x9F)     //一级应用更新6F18
		{
			if(dataType != D_OTAAppDown)
			{
				OTAMenuAccess(dataBuf[2], FALSE, NULL, ACCESS_DELETE_CATALOG, MenuType_OTA);
			}
		}

		memcpy(&aid[3], &dataBuf[pos+4], 2);

		memcpy(&aid[0], &dataBuf[2], 3);

		aid[1] = aid[1] | 0xC0;                              		//应用已开启、已下载、允许MO删除，无需开通或默认关闭

		WriteTFBuf(G_AppFileAddr, offset, &aid[0], 5);	  		//write the AID and APP space

		if (aid[0] == 0x9F)
		{
			memcpy(&G_EventAID[0], &aid[0], 0x03);

			return TRUE;
		}

		if(dataType == D_OTAAppUpdate)		//业务更新完成
		{
			return TRUE;
		}

		if(dataType == D_OTAAppDown)
		{
			OTAClearCurrentSession();

			OTADelAppFromServerList(TYPE_Recommend_List, &dataBuf[2]);	//删除对应最新推荐列表

			if(dataBuf[2] > 0x9F)
			{
				OTADelAppFromServerList(TYPE_Catalog_List, &dataBuf[2]);//删除目录下载列表
			}
			else
			{
				OTADelAppFromServerList(OTALocatIndexInServerListFile(dataBuf[2]), &dataBuf[2]);//删除对应的下载列表
			}

			aid[0] = 0x03;

			memcpy(&aid[1], &dataBuf[2], aid[0]);

			WriteVariable(OTADownloadedAID, &aid[0]); 		//记录已经下载的AID到ROM
		}

		return TRUE;
	}

	if((dataBuf[8]&0x01) == 0x01)						//大数据下载（包括交互应用）
	{
		largeCntr = ((dataBuf[8] >> 3)&0x1F);				//每组的短信条数

		result = (dataBuf[7]-1)%largeCntr;

		if(groupAll == TRUE)
		{
			ClearBit(smsTmp[3 + (result>>0x03)], result&0x07);	//登记改条短信已经收到
		}

		G_BigDataGroupNum = (dataBuf[7]-1)/largeCntr + 1;		//第几组短信

		if((dataBuf[8]&0x02) == 0x00)
		{
			G_BigDataLastNum = G_BigDataGroupNum * largeCntr;	//最后一条短信的序号
		}

		result = dataBuf[6] - (dataBuf[6] % largeCntr) + 1;		//计算大数据信息单元的分组最后一条的序号

		if(result > dataBuf[6])
		{
			result = result - largeCntr ;
		}

		if((dataBuf[7] >= G_BigDataLastNum)&&(dataBuf[7] < result))	//续传的最后一条或者分组的最后一条,并且不是最后一组
		{
			if((groupAll == TRUE)||(dataBuf[7] >= G_BigDataLastNum))	//存在未收全情况
			{
				CheckLargeSms(&smsTmp[3], &RecData[2], largeCntr,largeCntr);	//检查分组的收到情况
			}
			else
			{
				RecData[2] = 0x00;
			}

			RecData[0] = RecData[2] + 2;

			RecData[1] = dataBuf[5];

			result = WriteVariable(OTABigDataResult, (BYTE *)&RecData[0]);

			if(result == TRUE)
			{
				PendingMessageAlert(D_OTABigDataResultTag,0x00);

				if(groupAll == TRUE)
				{
					if(RecData[2] != 0x00)			//没有收全本组
					{
						G_BigDataLastNum = RecData[RecData[2]+2];	//记录分组的最后一条
					}
					else
					{
						ClearBit(smsTmp[7 + ((G_BigDataGroupNum-1)>>0x03)], (G_BigDataGroupNum-1)&0x07);	//登记该组已经收全

						memset(&smsTmp[3], 0xFF, 4);	//清空短信登记信息
					}
				}
				else
				{
					return FALSE;
				}
			}
			else
			{
				return FALSE;
			}
		}
	}
	else						   			//非大数据下载
	{
		if(groupAll == TRUE)
		{
			ClearBit(smsTmp[3 + ((dataBuf[7]-1)>>0x03)], (dataBuf[7]-1)&0x07);

			smsTmp[7] = 0xFE;
		}
		else
		{
			return FALSE;
		}
	}

	WriteLFBuf(fileAddr, 2, 0x00, &smsTmp[0], sizeof(smsTmp));

	return FALSE;
}

#else
/*
*@pointer:dataBuf相对于RecData的偏移
*/
BYTE OTASaveDownLoadData(BYTE * dataBuf, BYTE dataType,BYTE pointer)
{
	UWORD xdata fileAddr;
	WORD  xdata tmpOffset, fileSize, appSize, offset,maxNum;
	BYTE  idata largeCntr, result, groupAll, pos, datapos;
	BYTE  xdata smsTmp[11], aid[5];
	WORD xdata dataLength,ptrOffset,appOffset;
	BYTE idata current;

	fileAddr = SelectLFFile(DF_OTA, EF_OTASecurity);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadLFBuf(fileAddr, 2, 0x00, &smsTmp[0], sizeof(smsTmp));

	pos = 0x05;

	datapos =  0x0D;

	if(smsTmp[0] != dataBuf[pos])	 //批次不相等		modify  at@080311 15:40  原 smsTmp[0] != dataBuf[4]	
	{
		if(smsTmp[0] != 0xFF)
		{
			SetLFConst(fileAddr, 2, 0x00, 0xFF, 7);
		}

		smsTmp[0] = dataBuf[pos];		 			//保存批次

		smsTmp[2] = dataBuf[pos+1];					//保存短信总数

		smsTmp[1] = 0x00;						//已收短信数目

		memset(&smsTmp[3], 0xFF, 8);					//大数据下载清空登记信息
	}

	if(dataType == D_OTAInterDown)
	{
		smsTmp[1]++;

		groupAll = TRUE;
	}
	else
	{
		groupAll = IsDataSmsReceived(dataBuf[7], dataBuf[8], &smsTmp[3]);

		if(groupAll == TRUE)
		{
			smsTmp[1]++;
		}
		else                      //若该条数据已经收到，则抛弃该数据，不进行后续处理
		{
			return FALSE;
		}
	}

	if((dataType != D_OTAInterDown)&&(dataBuf[2] < 0xA0))		//modify at@080311 15:43 增加dataType!=D_OTAInterDown判断
	{
		  if(OTAMenuAccess(dataBuf[2], 0, NULL, ACCESS_LOCATE, MenuType_OTA) ==FALSE)	  //无对应目录，丢弃该数据
		  {
		  		return FALSE;
		  }
	}

	fileSize = GetTFSize(G_AppFileAddr);

	if(groupAll == TRUE)
	{
		offset = OTAGetAppOffset((BYTE CONST *)&EndAID[0]);			//应用文件的应用末尾偏移

		if(dataType != D_OTAInterDown)
		{
			G_HandleResponse = 0x03;				//整理OTA应用数据前，要退出解释器的运行

	   		G_STKAppOffset = OTAGetAppOffset((BYTE CONST *)&dataBuf[2]);
			
			if(G_STKAppOffset != ErrorNoApp)
			{
				if(dataType == D_OTAAppDown)			//已经下载过的应用，不需要再下载
				{
					return FALSE;
				}

				OTADeleteApp(FALSE,D_DelTagApp);       //打删除标记
			}

			G_STKAppOffset = 0;

			OTAPureAppScript(fileAddr, 0);			//整理OTA应用数据

			offset =OTAGetAppOffset((BYTE CONST *)&EndAID[0]);	//应用空间整理完成后，应用文件的应用末尾偏移，也是下载应用在文件中的起始偏移

			G_AppOffset = (((WORD)dataBuf[11]<<8) + dataBuf[12]) + offset;	//本条短信的应用数据在文件中的偏移

			if ((G_AppOffset + 5) > fileSize)
			{
				return FALSE;
			}
		}
		else
		{
			appSize = ((dataBuf[pos+4]<<8)&0xFF00) + (dataBuf[pos+5]&0x00FF);  //交互应用所需空间

			if ((fileSize - offset - 5) < appSize)
			{
				return FALSE;
			}

			offset = ((dataBuf[pos+6]<<8)&0xFF00) + (dataBuf[pos+7]&0x00FF);	//计算交互应用的偏移(交互应用暂存在应用文件的最末尾)

			G_AppOffset = fileSize - (appSize+5)  + offset;		//本条短信的应用数据在文件中的偏移

			offset = fileSize - appSize - 5; 			//交互应用在文件中的起始偏移
		}

	  	if(dataBuf[pos+2] == 0x01)
		{
			if(G_ResponseMode == Mode_BIP)   //数据通过BIP下来
			{
				dataLength = (((WORD)dataBuf[0]<<8)&0xFF00) + dataBuf[1];
				
				dataLength -= 0x0B;     //应用数据长度
				
				pointer += datapos;  //应用数据在RecData中的偏移
				
				if(dataLength <= (0xFF-pointer))
				{
					result = WriteTFBuf(G_AppFileAddr, offset+5, &dataBuf[datapos+5], (((dataBuf[0]<<8)&0xFF00) + dataBuf[1]) - datapos-3);//如果是第一条，不写AID和长度
				}
				else
				{
					result = WriteTFBuf(G_AppFileAddr, offset+5, &dataBuf[datapos+5], 0xFF-pointer-5);//如果是第一条，不写AID和长
					
					if(result == FALSE)
					{
						return FALSE;
					}
					
					dataLength = dataLength -(0xFF-pointer);  //文件中的剩余数据长度
					
					ptrOffset = 0xFF;
					
					appOffset = offset+0xFF-pointer;
					
					while(dataLength > 0)
					{
						if(dataLength >= 0xFF)
						{
							current = 0xFF;
						}
						else 
						{
							current = dataLength;
						}
						
						ReadTFBuf(G_BipDataAddr,ptrOffset,&tempBuf[0],current);
						
						result = WriteTFBuf(G_AppFileAddr, appOffset,&tempBuf[0],current);
						
						if(result == FALSE)
						{
							return FALSE;
						}
						
						ptrOffset += current;
						
						appOffset += current; 
						
						dataLength -= current;
						
					}
					
					
				}
			}
			else
			{
				result = WriteTFBuf(G_AppFileAddr, offset+5, &dataBuf[datapos+5], (((dataBuf[0]<<8)&0xFF00) + dataBuf[1]) - datapos-3);//如果是第一条，不写AID和长度
			}
		}
		else
		{
			if(G_ResponseMode == Mode_BIP)   //数据通过BIP下来
			{
				dataLength = (((WORD)dataBuf[0]<<8)&0xFF00) + dataBuf[1];
				
				dataLength -= 0x0B;     //应用数据长度
				
				pointer += datapos;  //应用数据在RecData中的偏移
				
				if(dataLength <= (0xFF-pointer))
				{
					result = WriteTFBuf(G_AppFileAddr, G_AppOffset, &dataBuf[datapos], (((dataBuf[0]<<8)&0xFF00) + dataBuf[1]) - datapos+2);//如果是第一条，不写AID和长度
				}
				else
				{
					result = WriteTFBuf(G_AppFileAddr,G_AppOffset, &dataBuf[datapos], 0xFF-pointer);
					
					if(result == FALSE)
					{
						return FALSE;
					}
					
					dataLength = dataLength -(0xFF-pointer);  //文件中的剩余数据长度
					
					
					ptrOffset = 0xFF;
					
					appOffset = G_AppOffset+0xFF-pointer;
					
					while(dataLength > 0)
					{
						if(dataLength >= 0xFF)
						{
							current = 0xFF;
						}
						else 
						{
							current = dataLength;
						}
						
						ReadTFBuf(G_BipDataAddr,ptrOffset,&tempBuf[0],current);
						
						result = WriteTFBuf(G_AppFileAddr, appOffset,&tempBuf[0],current);
						
						if(result == FALSE)
						{
							return FALSE;
						}
						
						ptrOffset += current;
						
						appOffset += current; 
						
						dataLength -= current;
						
					}
							
				}
				
			}
			else
			{
				result = WriteTFBuf(G_AppFileAddr, G_AppOffset, &dataBuf[datapos], (((dataBuf[0]<<8)&0xFF00) + dataBuf[1]) - datapos+2);
			}
		}

		if(result == FALSE)
		{
			return FALSE;
		}
	}

	if(smsTmp[1] == smsTmp[2])	//注意,	smsTmp[1]和smsTmp[2]不要覆盖,后面还需要用(江苏)//receive all sms
	{
		SetLFConst(fileAddr, 0x02, 0x00, 0xFF, sizeof(smsTmp));

		tmpOffset = (((WORD)dataBuf[pos+4]<<8)&0xFF00) + dataBuf[pos+5];

		if(dataType == D_OTAInterDown)
		{
			G_AppOffset = offset;					//交互应用解析器执行时用
		}

		if((tmpOffset+5+offset) > fileSize)        
		{
			return FALSE;
		}

		SetTFConst(G_AppFileAddr, tmpOffset+offset, 0xFF, 5);		//write the end mark

		if(dataBuf[2] > 0x9F)     //一级应用更新6F18
		{
			if(dataType == D_OTAAppUpdate)		//应用更新
			{
				OTAMenuAccess(dataBuf[2], FALSE, NULL, ACCESS_DELETE_CATALOG, MenuType_OTA);
			}
		}

		memcpy(&aid[3], &dataBuf[pos+4], 2);

		memcpy(&aid[0], &dataBuf[2], 3);

		aid[1] = aid[1] | 0xC0;                              		//应用已开启、已下载、允许MO删除，无需开通或默认关闭

		WriteTFBuf(G_AppFileAddr, offset, &aid[0], 5);	  		//write the AID and APP space

		if (aid[0] == 0x9F)
		{
			memcpy(&G_EventAID[0], &aid[0], 0x03);
		}
	
		if(dataType == D_OTAAppDown)
		{
			OTAClearCurrentSession();

			OTADelAppFromServerList(TYPE_Recommend_List, &dataBuf[2]);	//删除对应最新推荐列表

			if(dataBuf[2] > 0x9F)
			{
				OTADelAppFromServerList(TYPE_Catalog_List, &dataBuf[2]);//删除目录下载列表
			}
			else
			{
				OTADelAppFromServerList(OTALocatIndexInServerListFile(dataBuf[2]), &dataBuf[2]);//删除对应的下载列表
			}

			aid[0] = 0x03;

			memcpy(&aid[1], &dataBuf[2], aid[0]);

			WriteVariable(OTADownloadedAID, &aid[0]); 		//记录已经下载的AID到ROM
		}

		if ((dataBuf[8]&0x01) != 0x01)
		{
#ifdef LOCALOTA	
//本地OTA中，MT非大数据应用下载和MT非大数据远程应用更新需上发 远程管理确认命令		
			if ((G_CounterType==D_CNTRB)&&(dataType!=D_OTAInterDown)&&(dataBuf[2] != 0x9F)) //事件触发型应用对用户是透明的
			{
				tempBuf[2] = dataType;  //保存命令类型

				tempBuf[6] = 0x01;      //保存执行结果

				tempBuf[1] = 0x05;

				memcpy(&tempBuf[3],&dataBuf[2],0x03); //保存应用索引

				WriteVariable(OTADowUpdateAPPTag, (BYTE *)&tempBuf[1]);

				PendingMessageAlert(D_UpdateDowloadConfirm,0x00);	
			}
#endif
			return TRUE;
		}
	}

	if((dataBuf[8]&0x01) == 0x01)						//大数据下载（包括交互应用）
	{
		largeCntr = ((dataBuf[8] >> 3)&0x1F);				//每组的短信条数

		result = (dataBuf[7]-1)%largeCntr;

		if(groupAll == TRUE)
		{
			ClearBit(smsTmp[3 + (result>>0x03)], result&0x07);	//登记改条短信已经收到
		}

		G_BigDataGroupNum = (dataBuf[7]-1)/largeCntr + 1;		//第几组短信

		if (G_BigResendTag == FALSE)
		{
			if (dataBuf[6] != dataBuf[7]) 
			{
				G_BigDataLastNum = G_BigDataGroupNum * largeCntr;						//最后一条短信的序号
			}
			else
			{
				G_BigDataLastNum = dataBuf[pos+1];
			}
		}

		result = dataBuf[6] - (dataBuf[6] % largeCntr) + 1;		//计算大数据信息单元的分组最后一条的序号;即最后一组的第一条短信索引

		if(result > dataBuf[6])
		{
			result = result - largeCntr ;
		}

		if(dataBuf[7] >= G_BigDataLastNum)	//续传的最后一条或者分组的最后一条
		{
			if((groupAll == TRUE)||(dataBuf[7] >= G_BigDataLastNum))	//存在未收全情况
			{
				maxNum = largeCntr;

				if(dataBuf[6] == dataBuf[7])
				{
					if ((dataBuf[6] % largeCntr) != 0x00)          //最后一组的短信条数
					{
						maxNum = dataBuf[6] % largeCntr;     
					}	
				}

				CheckLargeSms(&smsTmp[3], &RecData[2], largeCntr,maxNum);	//检查分组的收到情况
			}
			else
			{
				RecData[2] = 0x00;
			}

			RecData[0] = RecData[2] + 2;

			RecData[1] = dataBuf[5];

			result = WriteVariable(OTABigDataResult, (BYTE *)&RecData[0]);

			if(result == TRUE)
			{		
				PendingMessageAlert(D_OTABigDataResultTag,0x00);
			
				if(groupAll == TRUE)
				{
					if(RecData[2] != 0x00)			//没有收全本组
					{
						G_BigDataLastNum = RecData[RecData[2]+2];	//记录分组的最后一条

						G_BigResendTag = TRUE;
					}
					else
					{
						ClearBit(smsTmp[7 + ((G_BigDataGroupNum-1)>>0x03)], (G_BigDataGroupNum-1)&0x07);	//登记该组已经收全

						memset(&smsTmp[3], 0xFF, 4);	//清空短信登记信息

						G_BigResendTag = FALSE;
					}

					if (smsTmp[1] == smsTmp[2])
					{
						return TRUE;
					}
				}
				else
				{
					return FALSE;
				}
			}
			else
			{
				return FALSE;
			}
		}
	}
	else						   			//非大数据下载
	{
		if(groupAll == TRUE)
		{
			ClearBit(smsTmp[3 + ((dataBuf[7]-1)>>0x03)], (dataBuf[7]-1)&0x07);

			smsTmp[7] = 0xFE;
		}
		else
		{
			return FALSE;
		}
	}

	WriteLFBuf(fileAddr, 2, 0x00, &smsTmp[0], sizeof(smsTmp));

	return FALSE;
}
#endif

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTADisplayDownloadSuccess

  Input parameter:   void

  Output parameter:	 void

  Description:       提示下载成功的信息

  Coder:			 xuqs

  Status:

 -------------------------------------------------------------------------------------------------------------------*/
#ifdef LOCALOTA
BYTE OTADisplayDownloadSuccess(void)
{
	RecData[0] = ReadVarible(OTADownloadedAID, &RecData[1]);

	G_AppOffset = OTAGetAppOffset((BYTE CONST *)&RecData[1]);

	tempBuf[0] = 0x00;

 	if (RecData[1] < 0x9F)
	{
		OTAMenuAccess(RecData[1], MENU_OFFSET_NAME, &RecData[0], ACCESS_READ, MenuType_OTA);
	
		if (RecData[1] == 0x80)
		{
			RecData[1] = RecData[0] - 1;
		}
		else
		{
			AsciiToUCS2(&RecData[0], &RecData[1]);
		}
	
		AddVlaue(&tempBuf[0], (BYTE CONST *)&RecData[1]);
	}
	else
	{
		AddVlaue(&tempBuf[0], (BYTE CONST *)&yijicaidan);
	}

	AddVlaue(&tempBuf[0], (BYTE CONST *)xiade);

	if(G_AppOffset != ErrorNoApp)
	{
		ReadTFBuf(G_AppFileAddr, G_AppOffset+5,&ResData[0], sizeof(BYTE));

		ReadTFBuf(G_AppFileAddr, G_AppOffset+6,&ResData[1], ResData[0]);

		if (ResData[1] == 0x80)
		{
			ResData[1] = ResData[0] - 1;
		}
		else
		{
			AsciiToUCS2(&ResData[0], &ResData[1]);
		}

		AddVlaue(&tempBuf[0], (BYTE CONST *)&ResData[1]);

		AddVlaue(&tempBuf[0],(BYTE CONST *)xiazaichenggong);
		
		if (G_CounterType != D_CNTRB)
		{
			cephDisplayTextSingle(0x01,UCS2, (BYTE CONST *)&tempBuf[0]);
		}

		return TRUE;
	}
   
	return FALSE;
}
#else
BYTE OTADisplayDownloadSuccess(void)
{
	RecData[0] = ReadVarible(OTADownloadedAID, &RecData[1]);

	G_AppOffset = OTAGetAppOffset((BYTE CONST *)&RecData[1]);

	tempBuf[0] = 0x00;

	AddVlaue(&tempBuf[0], (BYTE CONST *)downSuccess);

	if(G_AppOffset != ErrorNoApp)
	{
		ReadTFBuf(G_AppFileAddr, G_AppOffset+5,&ResData[0], sizeof(BYTE));

		ReadTFBuf(G_AppFileAddr, G_AppOffset+6,&ResData[1], ResData[0]);

		if (ResData[1] == 0x80)
		{
			ResData[1] = ResData[0] - 1;
		}
		else
		{
			AsciiToUCS2(&ResData[0], &ResData[1]);
		}

		AddVlaue(&tempBuf[0], (BYTE CONST *)&ResData[1]);

		if(RecData[1] > 0x9F)  //一级应用下载成功！
		{
			AddVlaue(&tempBuf[0],(BYTE CONST *)&muluDownloadSuccess[0]);
		}
		else
		{
			AddVlaue(&tempBuf[0],(BYTE CONST *)downloadsuccess);

			OTAMenuAccess(RecData[1], MENU_OFFSET_NAME, &RecData[0], ACCESS_READ, MenuType_OTA);

			if (RecData[1] == 0x80)
			{
				RecData[1] = RecData[0] - 1;
			}
			else
			{
				AsciiToUCS2(&RecData[0], &RecData[1]);
			}

			AddVlaue(&tempBuf[0], (BYTE CONST *)&RecData[1]);

			AddVlaue(&tempBuf[0], (BYTE CONST *)muluzhong);
		}

		if (G_CounterType != D_CNTRB)
		{
			cephDisplayTextSingle(0x00,UCS2, (BYTE CONST *)&tempBuf[0]);
		}

		return TRUE;
	}
   
	return FALSE;
}
#endif

/**
 * @brief 		OTA下行命令"注册确认"后的接口,用于处理各省分特殊需求。如：注册成功不需要提示“注册成功”
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
void IF_OTAPPHanlder_Register(void)
{
	PendingMessageAlert(D_OTAUserRegistTag,0x00);
}

/**
 * @brief 		OTA下行命令"业务删除"处理前的接口，用于处理各省分特殊需求。如：扩展"清空最新推荐列表"
 *
 * @param 		ptr ：命令类型字节的偏移
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
void IF_OTAPPHander_DelAid(BYTE *dataptr)
{

}


/**
 * @brief 		OTA解释器扩展指令的接口，用于扩展特定的 基本 解释器指令；
 *				全局变量G_Command为指令类型, G_ByteCodeAddr为该条指令的指令类型在EEPROM中的地址;
 *
 * @Returns   	TRUE:执行接口里的指令 / FALSE:继续执行后续执行
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
BYTE IF_OTAExFuntion(void)
{
	BYTE idata result;

	result = TRUE;

	switch(G_Command)
	{
		//需添加扩展指令或者修改原OTA指令时，在此处自行添加case语句实现

		default:
			result = FALSE;
			break;
	}

	return result;
}



/**
 * @brief 		OTA解释器扩展指令的接口，用于扩展特定的 扩展 解释器指令；
 *				全局变量G_Command为指令类型, G_ByteCodeAddr为该条指令的指令类型在EEPROM中的地址;
 *
 * @Returns   	TRUE:执行接口里的指令 / FALSE:继续执行后续执行
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
BYTE IF_OTAExExFuntion(void)
{
	BYTE idata result;

	result = TRUE;

	switch(G_Command)
	{
		default:
			result = FALSE;
			break;
	}

	return result;
}
/**
 * @brief 		OTA解释器API"OTASendSMS"发送完短信后的接口
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
void IF_OTASendSMS(void)
{
#ifdef NEWFETION
	BYTE idata aid[3];
	WORD idata counter;
	UWORD idata fileAddr;

	if(G_JsFetion_Counter_Open == FALSE)
	{
		return;
	}

	fileAddr = SelectTFFile(DF_STK,EF_JS_FETION);

	ReadTFBuf(G_AppFileAddr,G_STKAppOffset,&aid[0],0x03);   //取得当前飞信的AID

	ReadTFBuf(fileAddr,2*(aid[2] -1),&ResData[0],0x02);

	counter = ((ResData[0]<<8)&0xFF00) | (ResData[1]&0x00FF);

	if(counter == 0xFFFF)
	{
		SetTFConst(fileAddr,0x00,0x00,GetTFSize(fileAddr));
	}
	else
	{
		counter ++;

		WriteTFBuf(fileAddr,2*(aid[2] -1), (BYTE *)&counter, 2);
	}
#endif
}

/**
 * @brief 		OTA解释器API"OtaSetupCall"的接口，可以处理呼叫完后的特殊需求
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */

void IF_OtaSetupCall(void)					//在OTA解释器中对SETUPCALL的处理
{
	OTASetupCall();

	return;
}

/**
 * @brief 		上发远程文件更新确认信息。通过宏RFMNOSENDRESULT可处理是否上发该确认信息
 *
 * @param 		result :远程文件更新结果(0x00:更新失败/0x01:更新成功)
 * @param 		packageNum :批次
 *
 * @Returns   	TRUE:成功 / FALSE: 失败
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */

BYTE OTASendRFMResult(BYTE result, BYTE packageNum)
{
	tempBuf[0] = 0x04;

	tempBuf[1] = 0x08;

	tempBuf[2] = 0x02;

	tempBuf[3] = packageNum;

	tempBuf[4] = result;
	
	return OTASendUplinkCommandMessage(0x01, &tempBuf[0]);
}



/**
 * @brief 		列出下载设置的子菜单，配合IF_OTADownloadSetMenu使用		
 *
 * @Returns		ExitStkPerform/BackMove/CMD_Success	
 *  
 * @author 		xuqs 
 *  
 * @history 	add comment by xuqs, @071220 22:37
 */
BYTE IF_OTADownloadSetItem(void)
{
	return cephSelectItemSingle(0x00,UCS2,NULL,(BYTE CONST*)MenuSet,sizeof(MenuSet));
}
/**
 * @brief 		处理下载设置的特殊需求接口：如菜单管理下的子菜单顺序更改和子菜单名称的修改;
 *				子菜单名称的修改需要修改;	
 * 
 * @param 		item:下载设置的子菜单建立后，用户选择的item号
 *			
 * @Returns		ERROR:执行COS内标准的菜单管理
 *				其他:BackMove:返回到当前菜单 /其他：退出
 * 
 * @author 		xuqs 		
 * 
 * @history 	add comment by xuqs, @071220 22:37
 */
BYTE IF_OTADownloadSetMenu(BYTE item)
{
	BYTE idata result;

	result = ERROR;	

	return result;		
}
/*
 * @brief      列出"应用推荐"时的子菜单
 * 
 * @Returns		ExitStkPerform/BackMove/CMD_Success	
 *
 * @author      xyz
 *
 * @history     add  by @090722
*/
BYTE IF_RecommendationSelectItem(void)
{
	return cephSelectItemSingle(0x00,UCS2,NULL,(BYTE CONST *)recommendation,sizeof(recommendation));
}
/**
 * @brief 		处理"应用推荐"子菜单功能
 * 
 * @param 		item:子菜单建立后，用户选择的item号
 *			
 * @Returns		ERROR:执行COS内标准的菜单管理
 *				其他:执行接口里的菜单管理
 * 
 * @author 		xuqs 		
 * 
 * @history 	add comment by xuqs, @071220 22:37
 */
BYTE IF_RecommendationSetItem(BYTE item)
{
	BYTE idata result;

	result = ERROR;	

	return result;		
}
/**
 * @brief 		列出菜单管理的子菜单，配合IF_OTAServiceMgtMenu使用
 *
 * @Returns		ExitStkPerform/BackMove/CMD_Success
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
BYTE IF_OTAMgtMenuSelectItem(BYTE menuId)
{
	return cephSelectItemSingle(0x00,UCS2,NULL,(BYTE CONST *)MenuMang,sizeof(MenuMang));
}
/**
 * @brief 		处理菜单管理的特殊需求接口：如菜单管理下的子菜单顺序更改和子菜单名称的修改;
 *				子菜单名称的修改需要修改;
 *				接口编写方法：首先修改数组MenuMang[]的值来修改子菜单名称，
 *							然后根据item的值选择调用OTA上行命令的函数
 *							BYTE OTAMenuManageDownload(BYTE menuId) 	下载菜单
 *							BYTE OTAMenuManageDelete(BYTE menuId)		删除菜单
 *							BYTE OTAListRefresh(BYTE menuId)			列表更新
 *							BYTE OTAMenuManageSearch(BYTE menuId)		搜索菜单
 *							例如:标准菜单管理的编写如下所示(函数内注释部分)；
 *							注意：标准菜单管理COS内已经实现，此处的注释部分只供编写其他特殊需求时的参考，
 *								所以若是标准菜单管理，则不需要修改该函数
 *
 * @param 		item:菜单管理的子菜单建立后，用户选择的item号
 *				menuId:指定被定位应用的目录ID
 *
 * @Returns		ERROR:执行COS内标准的菜单管理
 *				其他:执行接口里的菜单管理
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
BYTE IF_OTAServiceMgtMenu(BYTE item,BYTE menuId)
{
	BYTE idata result;

	result = ERROR;

	switch(item)
	{	
		default:
			break;
	}

	return result;
}

/**
 * @brief 		在发起处理菜单搜索上行、MO下载请求上行、列表更新请求上行
 *				一个新OTA命令之前，检查是否有正在进行中的菜单搜索上行、
 *				MO下载请求上行、列表更新请求上行、远程业务更新下行、
 *				交互应用下行数据命令，若有则由用户选择是放弃上次请求还是
 *				继续上次请求。
 *
 * @param		item:菜单管理的子菜单的item号。如0x01为下载菜单、0x02为删除菜单...
 *
 * @Returns		TRUE:	没有未结束的处理流程或用户放弃上一次请求
 *				FALSE:	用户继续上一次请求
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
BYTE OTAGetProcessPermission(BYTE item)
{
	BYTE idata result;
	UWORD xdata fileAddr;

	if (item == 0x02)							//菜单删除不需要提示是否放弃上次请求的信息
	{
		return TRUE;
	}

	fileAddr = SelectTFFile(DF_OTA, EF_RuntimeReg);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadTFBuf(fileAddr, 0x00, &result, sizeof(BYTE));

	if((result == 0x00)||(result == 0xFF))
	{
		return TRUE;
	}

	result = cephSelectItemSingle(0x00, UCS2, NULL, (BYTE CONST *)processpermission, sizeof(processpermission));

	if (result != CMD_Success)
	{
		return FALSE;
	}

	result = GetResponseItemIdentifier();

	if (result == 0x01)
	{
		OTAClearCurrentSession();					//若放弃上次操作则清除前一命令流程的登记信息。

		return TRUE;
	}	


	return FALSE;
}


/**
 * @brief 		设置OTA业务推荐上发信息的PID和DCS值
 *
 * @param 		BYTE pidDcs[2] :pidDcs[0] as PID , pidDcs[1] as DCS
 *
 * @return		FALSE:使用默认的PID:0x00,DCS:0x04/TRUE:使用本接口函数内设置的PID,DCS值
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @080110
 */
BYTE IF_SetOtaRecommendationPidDcs(BYTE pidDcs[2])
{
	pidDcs[0] = pidDcs[0];

	return FALSE ;
}
/**
 * @brief 		OTA下行指令扩展函数接口；
 *				
 * @param		dataptr:指向下行数据的指令类型
 *
 * @Returns   	TRUE:正确执行接口里的指令，0x02:接口里的指令执行错误，不更新counter / FALSE:继续执行后续执行
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
BYTE IF_OTAPPHander(BYTE * dataptr)
{
	BYTE idata result;
	
	result = TRUE;
	
	switch(dataptr[0])
	{
		default:
			result = FALSE;
			break;
	}
	
	return result;
}
 /**
 * @brief 		OTA下行指令比较mac counter函数接口；
 *				
 * @param		cmdType:指令类型
 *
 * @Returns   	TRUE:当前指令不需要计算mac和counter，直接跳过mac，counter比较，进行后续操作 / FALSE: 后续将进行mac和counter的比较
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @071220 22:37
 */
BYTE IF_OTAPPHandlerMac(BYTE cmdType)
{
	return FALSE;
}


/**
 * @brief 		OTAAppDelNotice()的接口函数，在处理MO删除时，对已删除应用进行的操作
 *
 * @author 		xuqs
 *
 * @history 	created by xuqs, @081220 20:01
 */
void IF_OTAAppDelNotice(BYTE *aid)
{
	
}
/**
 * @brief 		OTA解释器中AccessDefined（）接口函数，对于标准OTA3.5，只能读写具有PIN/ALW权限的当前文件；此处可以自行修改
 *
 * @param       viewIndex:当前文件的读/写权限0x90-ALW; 0x01-PIN; 0x0A-ADM; 0x97-NEVER 
 * @param       fileName: 当前文件名
 *
 * @return      返回权限0x90-ALW; 0x01-PIN; 0x0A-ADM; 0x97-NEVER  
 *
 * @author 		xuqs
 *
 * @history 	created by xuqs, @081220 20:01
 */
BYTE IF_AccessDefined(BYTE viewIndex,WORD fileName)
{
#ifdef BJ
	BYTE idata aid[3];

	ReadTFBuf(G_AppFileAddr, G_STKAppOffset, &aid[0], 3);

	if((aid[0] == 0x9F)&&(aid[1] == 0x03)&&(aid[2] == 0xFE))
	{
		if((viewIndex == 0x97)||(viewIndex == 0x0A))
		{
			viewIndex = 0x90;
		}
	}
#endif
	return viewIndex;
}

/**
 * @brief 		处理通道状态下行事件函数
 *
 * @Returns   	TRUE:成功 / FALSE:失败
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @090910 15:37
 */
BYTE ChannelStatus(BYTE pos)
{
	if((RecData[pos] & 0x7F) != 0x38)
	{
		return FALSE;
	}
	if(RecData[pos+1] != 0x02)
	{
		return FALSE;
	}

	if((RecData[pos+2]&0x80) != 0x00)
	{
		return FALSE;      
	}

	G_ChannelStatusID  = RecData[pos+2]&0x07;

	if(G_ChannelStatusID == 0x00)
	{
		return FALSE;
	}
	
	PendingMessageAlert(D_ChannelStatus,0x00);

	return TRUE;
}


#ifdef LOCALOTA
BYTE IF_UpdateOTADownloadSet(void)
{
	BYTE idata result;
	
	result = cephSelectItemSingle(0x00, UCS2, NULL, (BYTE CONST *)UpdateParameterSet, sizeof(UpdateParameterSet));
		
	return result;					
	
}
#endif




#endif

