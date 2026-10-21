/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.0

 * MODEL: 	All model

 * RELEASE:	2008-5-13

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件

			V1.1 修改函数OTASubmitSynchronizeInfo，修改对于未未下载应用AID的第二字节将0x80修改为使用直接从文件中读取
				 的值，因为在添加到下载列表时，已经把应用ID还原成未下载的状态.

			V1.2 修改函数OTAOpenChannelHandle，修改Open Channel中的Network Acess Name字段从6EAB的第14字节开始获得，如果长度为FF或00，则无该字段
			V1.3 修改函数 OTAMenuAddItem
			V1.4 修改函数 OTASubmitSynchronizeInfo与 SendSubmitInfoSyn，将信息同步数据缓存在文件6EA5 中
			
			V1.5 修改函数OTADataAvaiable @20120213 Version: 1.37->1.38
			
			V1.6 增加函数OTACaluteBIPMac,主要用于计算BIP 数据的mac @20130821 V1.38->1.39
			
			V1.7 修改OTASubmitSynchronizeInfo， @20130823 
			
			V1.8 修改函数SendSubmitInfoSyn，当没有短信内容时，也上发同步信息@20130906
			V1.9 修改函数OTASynMenuName,当2700/6F18文件不存在时不继续执行排序   @20170420
			
*/

#include "string.h"
#include "main.h"
#include "gsmpublic.h"
#include "error.h"
#include "fileAction.h"
#include "fileSystem.h"
#include "stkhead.h"
#include "OTA3Head.h"
#include "ceph.h"
#include  "Authentication.h"
#include "resume.h"
#include  "arithmatic.h"
#include "interface.h"



#ifdef OTA


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 MoveTFContent
  
  input parameter:   UWORD fileAddr, WORD offset, BYTE * buf, BYTE len

  output parameter:  TRUE/FALSE
                                                                                                                                                
  description:       Move upward for transparent file data
  
  status:			     

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE MoveTFContent(UWORD fileAddr, WORD srcOffset, WORD desOffset, WORD len)
{
	WORD idata fileSize;
	UWORD xdata des,src;

	ReadEeprom((BYTE *)&fileSize,fileAddr + F_SpaceSize_Offset,sizeof(fileSize));

	if((srcOffset > fileSize)||(desOffset > fileSize)||((srcOffset + len)>fileSize)||((desOffset + len)>fileSize))
	{
		return FALSE;
	}

	fileAddr = getFileContentAddr(fileAddr);

	des = fileAddr+desOffset;

	src = fileAddr+srcOffset;

	CopyEepromEX(des,src,len);

	return TRUE;
}

/*-------------------------------------------------------------------------------------------------------//

  Function name:	OtaRfmMac

  Input parameter:	dataAddr:保存解密后的RFM文件地址

					cpKey[8]:key

					cpResult[8]:result

					totalNum: 保存解包数据的总条数

					packageNum:批次号

  Output parameter:计算MAC使用的填充字节

  Description:		计算RFM 数据MAC，修改记录文件的数据长度为有效命令长度

  Status:			OK

 -------------------------------------------------------------------------------------------------------------------*/
BYTE OtaRfmMac(UWORD dataAddr,BYTE  cpKey[8],BYTE  cpResult[8],BYTE totalNum,BYTE packageNum)
{
	BYTE idata i,j,k,lgth;
	BYTE idata recIndex,len;
	BYTE xdata ucTemp[8];
	BYTE xdata cpEnter[8];
	WORD idata cpl;

	recIndex = RfmDataInfo(dataAddr,packageNum,1);

	ReadLFBuf(dataAddr, recIndex, 4, &ucTemp[0], 2);

	cpl = (((WORD)ucTemp[0]<<8)&0xFF00) + ucTemp[1];		//有效数据长度

	lgth = 8 - (cpl - 2) % 8;								//算MAC填充字节数

	if(totalNum == 1)										//单条
	{
		SetLFConst(dataAddr, recIndex,cpl + 2,0x20, lgth);		//补lgth个0x20,cpl+2 = 4 + cpl - 2

		SetLFConst(dataAddr, recIndex, 3,(cpl - 2) + lgth, sizeof(BYTE));
	}
	else												//多条
	{
		for(i = 1;i <= totalNum; i ++)						//处理由于第一条多2字节造成的不满足8字节整数倍
		{
			recIndex = RfmDataInfo(dataAddr,packageNum,i);

			ReadLFBuf(dataAddr, recIndex, 3, &len, sizeof(BYTE));

			if(i == 1)									//第一条
			{
				ReadLFBuf(dataAddr, recIndex,2+len,&ucTemp[0],2);	//本条的尾2字节拷贝到下一条

				len -= 2;

				SetLFConst(dataAddr, recIndex, 3,len, sizeof(BYTE));	//修改长度
			}
			else
			{
				ReadLFBuf(dataAddr, recIndex,4+len,&ucTemp[0],2);	//本条的尾2字节拷贝到下一条
			}

			if(i != totalNum)
			{
				recIndex = RfmDataInfo(dataAddr,packageNum,i+1);

				WriteLFBuf(dataAddr, recIndex,4, &ucTemp[0],2);

				cpl -= len;
			}
			else
			{
				SetLFConst(dataAddr, recIndex,cpl + 2 ,0x20, lgth);	//补lgth个0x20, cpl+2 = 4 + cpl - 2

				SetLFConst(dataAddr, recIndex, 3,(cpl - 2) + lgth, sizeof(BYTE));
			}
		}
	}

	memset(ucTemp,0x00,8);

	for (k = 1;k <= totalNum; k ++)						//计算MAC
	{
		recIndex = RfmDataInfo(dataAddr,packageNum, k);

		ReadLFBuf(dataAddr, recIndex, 3, &len, sizeof(BYTE));

		for(i = 0 ; i < len ;i += 8)
		{
			ReadLFBuf(dataAddr, recIndex, 4+i, &cpEnter[0],8);

			for(j = 0 ; j < 8 ; j ++)
			{
			  	ucTemp[j] = ucTemp[j] ^ cpEnter[j];
			}

	       	SingleDES(&ucTemp[0],&cpKey[0],&cpResult[0],ECB_Encrypt,8);

		  	memcpy(&ucTemp[0],&cpResult[0],8);
		}
	}

	return lgth;
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	PBOCAuth

  input parameter:   	BYTE key[16]

  			BYTE x[8]

  output parameter:	void

  description:       	The algorithm PBOC


  status:

 -------------------------------------------------------------------------------------------------------------------*/
void PBOCAuth(BYTE key[16],BYTE sourceKey[8])
{
	BYTE idata i;
	BYTE xdata nx[8];

	for(i=0;i<8;i++)
	{
		nx[i] = ~sourceKey[i];
	}

	TripleDES(sourceKey,key,sourceKey,ECB_Encrypt,8);

	TripleDES(nx,key,nx,ECB_Encrypt,8);

	memcpy(&key[0], &sourceKey[0], 8);

	memcpy(&key[8], &nx[0], 8);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 GetFreeAppSpaceSize

  Input parameter:   void

  Output parameter:  应用文件中的空间

  Description:       应用文件中的空间

  Coder:			 xuqs

  status:			 OK

 -------------------------------------------------------------------------------------------------------------------*/
WORD GetFreeAppSpaceSize(void)
{
	WORD  xdata datalen, fileSize, tmplen;

	fileSize = GetTFSize(G_AppFileAddr);

	datalen = 0;

	for (G_STKAppOffset = 0; ; G_STKAppOffset += tmplen)
	{
		tmplen = OTASearchAID(0x00, D_DelTagApp, TRUE);

		if (tmplen == FALSE)
		{
			break;
		}

		datalen += tmplen;
	}

	if(datalen >= fileSize)			  				//应用异常
	{
		return FALSE;
	}

	return (fileSize - datalen - 5);					//需要减去最后结束标志的五字节
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAGetAid

  Input parameter:   menuId：		需要查找AID的类型
  					 searchType: 	查找类型
  					 num:			需要查找的第几个AID
  					 buf:			存放查找结果(菜单名称)
  					 onOff;			标识查找到匹配的AID后G_AppOffset是否指向下一个AID

  Output parameter:  TRUE/FALSE

  Description:       在文件中搜索符合条件的AID，并将结果存入Buf

  Coder:			 Caibo

  Status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE OTAGetAid(BYTE menuId, BYTE searchType,BYTE num,BYTE * buf,BYTE onOff)
{
	BYTE idata i;
	WORD idata result;

	if((num != (G_LastNum +1))||(num == 0x01) )
	{
		G_STKAppOffset = 0x00;

		for(i = 0x00;i<(num-1);i++,G_STKAppOffset = G_STKAppOffset + result)
		{
		   	result = OTASearchAID(menuId,searchType,TRUE);
		}
	}

	result = OTASearchAID(menuId,searchType,TRUE);			//查找第num个AID

	if(result != FALSE)
	{
		if(onOff == TRUE)
		{
			ReadTFBuf(G_AppFileAddr, G_STKAppOffset+5,&buf[0],sizeof(BYTE));	//获取应用名称

			ReadTFBuf(G_AppFileAddr, G_STKAppOffset+5,&buf[0],buf[0]+1);

			G_STKAppOffset = G_STKAppOffset + result;

			G_LastNum = num;
		}

		return TRUE;
	}

	return FALSE;

}

/*-------------------------------------------------------------------------------------------------------//

  Function name:	singleDesMac
 
  Input parameter:	
 			        cpEnter:data

					cpKey  :key	

					cpResult:result

					nEnterLen: length of data

					mode:	 ECB_Encrypt/CBC_Encrypt
 
  Output parameter:     the number of tlv structure
 
  Description:          return the number that first tlv structure in input buffer 

  Status:		OK

 -------------------------------------------------------------------------------------------------------------------*/
void singleDesMac(BYTE  * dataBuf,BYTE  cpKey[8],BYTE  cpResult[8],WORD len,BYTE mode)
{	
	WORD idata i;
	BYTE idata j;
	BYTE xdata d[8];

	memset(cpResult, 0x00, 8);

	for(i=0; i<len; i+=8)
	{
		if ((i + 8) > len)						//最后一页
		{
			memset(d, 0x20, 8);

			memcpy(d, &dataBuf[i], len - i);
		}
		else
		{
			memcpy(d, &dataBuf[i], 8);
		}

		for(j=0; j<8; j++)
		{
			d[j] = d[j]^cpResult[j];
		}

		SingleDES(d, cpKey, cpResult, mode, 8);
	}	
}

/*
*@description:计算BIP数据的mac ，前0xFF个字节的数据已经 存放在dataBuf中了，后续数据要从文件中取得
*
*@dataBuf:前FF
*@mac
*@key
*@len    :总长度
*
*@return  :TRUE/FALSE
*
*@history
*/
BYTE OTACaluteBIPMac(BYTE * dataBuf, BYTE mac[4], BYTE key[16], WORD len)
{
	WORD xdata length,headLen;
	BYTE xdata cc[0x08];
	BYTE xdata d[8];
	BYTE idata j;
	WORD xdata offset,i;
	
		
	ReadTFBuf(G_BipDataAddr, 0x00, &cc[0],0x02);   //获取BIP通道数据长度
	
	length = (((WORD)cc[0] << 8)&0xFF00)|cc[1];
	
	if(length <= 0xFD)   //当前BIP数据可以直接在DATABUF中计算
	{
		singleDesMac(dataBuf,key,cc,len,CBC_Encrypt);

	}
	else
	{
		offset = 0xFF;   //文件中数据的起始偏移
		
		headLen = 0xF6;   //dataBuf中的有效数据长度
		
		memset(cc, 0x00, 8);

		for(i=0; i<len; i+=8)
		{
			if( i < headLen)
			{
				if((i+8) > headLen)
				{
					memcpy(d, &dataBuf[i], headLen-i);
					
					ReadTFBuf(G_BipDataAddr,offset,&d[headLen-i],8+i-headLen);
					
					offset = offset+(8+i-headLen);
				}
				else
				{
					memcpy(d, &dataBuf[i], 8);
				}
			}
			else if ((i + 8) > len)						//最后一页
			{
				memset(d, 0x20, 8);
				
				ReadTFBuf(G_BipDataAddr, offset, d, (len - i));
			}
			else
			{
				ReadTFBuf(G_BipDataAddr, offset, d, 8);
				
				offset += 8;
			}

			for(j=0; j<8; j++)
			{
				d[j] = d[j]^cc[j];
			}

			SingleDES(d, key, cc, CBC_Encrypt, 8);
		}	
	}
	
	if(memcmp(&mac[0], &cc[0], 4) == EQUAL)
	{
		return TRUE;
	}

	return FALSE;
}
/*--------------------------------------------------------------------------------------------------------------------
  Function name:	OTACaluteMac

  input parameter:	dataBuf：	数据

  					mac:		mac内容,如果(type==1)则把Mac的计算结果存入，否则比较Mac的内容

					key:		key的内容

					len:		计算数据的长度

					type:		0x01:计算MAC
							0x00:校验MAC

  output parameter:	TRUE/FALSE

  description:		计算并且比较Mac的内容

  author:			Zhuli

  Testor:			Caibo

  status:			OK
 -------------------------------------------------------------------------------------------------------------------*/

BYTE OTACaluteMac(BYTE * dataBuf, BYTE mac[4], BYTE key[16], WORD len, BYTE type)
{
  	BYTE xdata c[8];

  	singleDesMac(dataBuf,key,c,len,CBC_Encrypt);

	if (type == 0x00)
	{
		if(memcmp(&mac[0], &c[0], 4) == EQUAL)
		{
			return TRUE;
		}

		return FALSE;
	}

	memcpy(&mac[0], &c[0], 4);

	return TRUE;
}
/*
*@description  : 对菜单文件进行操作
*
*@param        :  menuId:	OTA目录ID
				menuType: 目录类型
				offset:	访问内容相对记录首的偏移。（MENU_OFFSET_TYPE,
				MENU_OFFSET_ID,MENU_OFFSET_MENUID,MENU_OFFSET_PAGE_NUMBER,
				MENU_OFFSET_MENU_ATTRIBUTE,MENU_OFFSET_NAME）
				buf:	读取时，为目标缓冲区，写入时为源缓冲区，查询时保留。
						注：buf在读写目录名称(MENU_OFFSET_NAME)时，
						为LV结构，其它为V结构
				mode	指定访问方式。(ACCESS_READ, ACCESS_WRITE,
						ACCESS_LOCATE,ACCESS_ENABLE_CATALOG,
						ACCESS_DISABLE_CATALOG,ACCESS_DELETE_CATALOG,
						ACCESS_ENABLE_MENU_MGT,ACCESS_DISABLE_MENU_MGT)
				fileAddr :菜单文件地址
*/
BYTE MenuAcess(BYTE menuId, BYTE offset, BYTE * buf, BYTE mode, BYTE menuType,UWORD fileAddr)
{
	BYTE  idata  i,recNum,j;
	BYTE  idata  v1;
	BYTE  xdata  tmp[5];

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	for(i=1; i<=recNum; i++)
	{
		ReadLFBuf(fileAddr, i, 0, tmp, sizeof(tmp));

		if((tmp[0] == menuType)&&(tmp[2] == menuId))	//找到匹配的目录记录
		{
			break;
		}
	}

	if((i > recNum) || (mode > 7))						//操作模式允许的最大值为7
	{
		return FALSE;
	}

	if(mode == ACCESS_DELETE_CATALOG)					//删除目录mode3
	{
		SetLFConst(fileAddr, i, MENU_OFFSET_TYPE, 0xFF, 8);

		if(menuId < 0xA0)    //仅对一级目录处理其下载列表和最新推荐列表
		{
			recNum =OTALocatIndexInServerListFile(menuId);

			for(j = recNum; j < (recNum + 5); j++)
			{
				SetLFConst(G_ServerListFileAddr, j, 0x00, 0xFF, 0x04);
			}
			for(j = 1; j < 16; j++)
			{
				SetLFConst(G_ServerListFileAddr, j, 0x00, 0xFF, 0x04);
			}
		}
	}
	else if (mode == ACCESS_LOCATE)						//定位记录
	{
		return i;
	}
	else if(mode == ACCESS_READ)						//读操作mode2
	{
		ReadLFBuf(fileAddr, i, offset, buf, 1);

		if(offset == MENU_OFFSET_NAME)				//读目录名称
		{
			ReadLFBuf(fileAddr, i, offset+1, &buf[1], buf[0]);
		}
	}
	else if(mode == ACCESS_WRITE)					//写操作mode4
	{
		if(offset == MENU_OFFSET_NAME)				//写目录名称
		{
			v1 = buf[0]+1;
		}
		else
		{
			v1 = 1;
		}

		WriteLFBuf(fileAddr, i, offset, buf, v1);
	}
	else									//mode 5&6 or mode 1&0 only
	{
		ReadLFBuf(fileAddr, i, MENU_OFFSET_MENU_ATTRIBUTE, &tmp[0], 1);

		if(mode > 1)							//mode 5 & 6
		{
			v1 = 0x08;						//use bit 4

		}
		else								//mode 0 & 1
		{
			v1 = 0x80;						//use bit 8
		}

		if((mode == ACCESS_SHOW_MENAGE_MENU) || (mode == ACCESS_ENABLE_CATALOG))//启用或显示
		{
			tmp[0] |= v1;
		}
		else								//禁用或隐藏
		{
			v1 = ~v1;

			tmp[0] &= v1;

		}

		WriteLFBuf(fileAddr, i, MENU_OFFSET_MENU_ATTRIBUTE, &tmp[0], 1);
	}

	return i;
}
/*-------------------------------------------------------------------------------------------------

Function name	:	OTAMenuAccess

input parameter	:	menuId:	OTA目录ID
					menuType: 目录类型
					offset:	访问内容相对记录首的偏移。（MENU_OFFSET_TYPE,
					MENU_OFFSET_ID,MENU_OFFSET_MENUID,MENU_OFFSET_PAGE_NUMBER,
					MENU_OFFSET_MENU_ATTRIBUTE,MENU_OFFSET_NAME）
					buf:	读取时，为目标缓冲区，写入时为源缓冲区，查询时保留。
							注：buf在读写目录名称(MENU_OFFSET_NAME)时，
							为LV结构，其它为V结构
					mode	指定访问方式。(ACCESS_READ, ACCESS_WRITE,
							ACCESS_LOCATE,ACCESS_ENABLE_CATALOG,
							ACCESS_DISABLE_CATALOG,ACCESS_DELETE_CATALOG,
							ACCESS_ENABLE_MENU_MGT,ACCESS_DISABLE_MENU_MGT)

output parameter:	成功：	非FALSE.
					失败：	返回FALSE

description		:	根据menuId在EF_STKMenuFile中定位记录号,然后根据mode进行相应操作

author			:	xuzhichu

test			:	caibo
----------------------------------------------------------------------------------------------------*/
BYTE OTAMenuAccess(BYTE menuId, BYTE offset, BYTE * buf, BYTE mode, BYTE menuType)
{	   
	UWORD xdata  fileAddr;

	if((menuId == 0x00)&&(offset == MENU_OFFSET_PAGE_NUMBER))  //更新"应用管理"页码
	{
		menuId = AppManageMenuId;

		menuType = MenuType_SOLID;
	}

	fileAddr = SelectLFFile(DF_OTA, EF_Menu);

	return 	MenuAcess(menuId,offset, buf, mode ,menuType,fileAddr);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAAddAppTOServerList

  Input parameter:   type:(01:最新推荐；02:下载列表)
  					 index: 记录号
					 databuf:服务器列表数据(v)
  					 replace:列表已满时是否替换最旧的应用

  Output parameter:  void

  Description:       添加应用到服务器列表

  Coder:			 xuqs

  Status:
 -------------------------------------------------------------------------------------------------------------------*/
void OTAAddAppTOServerList(BYTE type, BYTE index, BYTE *databuf, BYTE replace)
{
	BYTE idata tmp,ch,i, len;
	BYTE xdata aid[3];

#ifdef LOCALOTA
	if( (databuf[0] >= 0xA0)&&(databuf[0] <= 0xB7))   //可更名固化应用不出现在下载列表、应用推荐中
	{
		return;
	}
#endif
	for(i=1;i<GetLFRecNumLgth(G_ServerListFileAddr,FileNum);i++)
	{
		ReadLFBuf(G_ServerListFileAddr, i, 0x01, &aid[0],0x03);

		if ((aid[0] == databuf[0])&&((aid[1]&0x03) == (databuf[1]&0x03))&&(aid[2] == databuf[2]))
		{
			return;
		}
	}

	ReadLFBuf(G_ServerListFileAddr, index + 4 , 0x00, &tmp, sizeof(BYTE));	//读本块最后一条记录

	if ((tmp != 0xFF) &&(replace == FALSE) &&(type != TYPE_Recommend_List))	//下载列表已满时，由replace控制是否添加新列表
	{
		return;
	}

	tmp = GetLFRecNumLgth(G_ServerListFileAddr,FileLgth);

 	if ((type == TYPE_Download_List)||(type == TYPE_Catalog_List))
	{
		ch = 2;
	}
	else
	{
		ch = 5;

		ReadTFBuf(G_ServerListFileAddr, tmp * (index + 2*ch - 1), &tempBuf[0], 4*tmp);

		WriteTFBuf(G_ServerListFileAddr, tmp * (index + 2*ch), &tempBuf[0], 4*tmp);
	}

	len = ch*tmp;

	ReadTFBuf(G_ServerListFileAddr, tmp * (index + ch - 1), &tempBuf[0], len);

	WriteTFBuf(G_ServerListFileAddr, tmp * (index + ch), &tempBuf[0], len);

	ReadTFBuf(G_ServerListFileAddr, tmp * (index-1), &tempBuf[0], len);

	WriteTFBuf(G_ServerListFileAddr, tmp * index, &tempBuf[0], len);

	tempBuf[1] = type;

	tempBuf[0] = 7 + databuf[5];

	if (type == TYPE_Recommend_List)
	{
		tempBuf[0] = databuf[tempBuf[0] -1] + tempBuf[0]+ 1;		//计算添加服务介绍后的长度

		tempBuf[0] = databuf[tempBuf[0] -1] + tempBuf[0]+ 1;		//计算添加目的号码后的长度
	}

	if((type != TYPE_Catalog_List) ||((type == TYPE_Catalog_List)&&(databuf[0] >0x9F)) )  //目录列表(01-9F)下载时为0xHH 0x00 0x00
	{
		databuf[1] |= D_EnableApp;						//启用

		databuf[1] &= 0x8F;							//未下载&未删除
	}

	memcpy(&tempBuf[2], &databuf[0], tempBuf[0] -1);

	WriteLFBuf(G_ServerListFileAddr, index, 0x00, &tempBuf[1], tempBuf[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTADelAppFromServerList

  Input parameter:   index:记录号(0x01表示最新推荐，其他值为下载列表)
  					 databuf:待删除或更新的列表数据(v)
					 type:0x00  列表删除/0x01 列表更新

  Output parameter:  void

  Description:       从服务器列表中删除或更新一条指定的列表数据

  Coder:			 xuqs

  Status:
 -------------------------------------------------------------------------------------------------------------------*/

void OTADelAppFromServerList(BYTE index, BYTE * databuf)
{
	BYTE idata pos, length;
	WORD xdata srcOffset,desOffset,movelen;
	BYTE xdata tmp[3];

	pos = (index == TYPE_Recommend_List) ? index +14 : index + 4;

	while(TRUE)
	{
		ReadLFBuf(G_ServerListFileAddr, index, 0x01, &tmp[0], sizeof(tmp));

		if ((tmp[0] == databuf[0])&&((tmp[1]&0x03) == (databuf[1]&0x03))&&(tmp[2] == databuf[2]))
		{
			break;
		}

		index ++;

		if (index > pos)  				//无匹配记录
		{
			return;
		}
	}

	length = GetLFRecNumLgth(G_ServerListFileAddr, FileLgth);

	desOffset = (index-1) * length;

	srcOffset = index * length;

	movelen = (pos - index) * length;

	MoveTFContent(G_ServerListFileAddr, srcOffset, desOffset, movelen);

	SetLFConst(G_ServerListFileAddr, pos, 0x00, 0xFF, 0x04);
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAPureAppScript

  Input parameter:   securityFileAddr
  					 tmpOffset:用于计算移动的目标地址

  Output parameter:  void

  Description:       整理应用脚本文件空间(带断电保护功能)

  Coder:			 xuqs

  Status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAPureAppScript(UWORD securityFileAddr, WORD tmpOffset)
{
	BYTE idata result;
	WORD xdata datalen;
	WORD xdata srcOffset,desOffset;
	OTARESUME xdata Resume;

	SetLFConst(securityFileAddr, 0x01, 0x00, 0x01, sizeof(BYTE));		//设置断电保护标志

	srcOffset = G_STKAppOffset;

	desOffset = tmpOffset;							//初始化第一次移动的目标地址

	result = FALSE;

	for (; G_STKAppOffset < GetTFSize(G_AppFileAddr);)
	{
		datalen = OTASearchAID(0x00, D_DelTagApp, FALSE);		//查找删除AID

		desOffset = G_STKAppOffset - tmpOffset;

		if (datalen != FALSE)
		{
			result = TRUE;						//标识至少有一个匹配AID

			datalen = OTASearchAID(0x00, D_DelTagApp, TRUE);	//查找有用AID

			srcOffset = G_STKAppOffset;

			datalen = OTASearchAID(0x00, D_DelTagApp, FALSE);	//查找删除AID

			tmpOffset = srcOffset - desOffset;

			Resume.sourceOffset = tmpOffset;

			Resume.destntnOffset = G_STKAppOffset;

			WriteLFBuf(securityFileAddr, 0x01,0x01, (BYTE *)&Resume, 2*sizeof(WORD)); //记录下一次移动的起始查找地址

			Resume.lgth = G_STKAppOffset - srcOffset;

			MoveTFContent( G_AppFileAddr, srcOffset, desOffset, Resume.lgth + 5);
		}
		else
		{
			break;
		}
	}

	if (result == TRUE)
	{
   		SetTFConst(G_AppFileAddr, desOffset, 0xFF, 0x05);		//设置结束标志
	}

	SetLFConst(securityFileAddr, 0x01, 0x00, 0xFF, 0x01);	 		//取消断电保护标志
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 	OTALocatIndexInServerListFile

  Input parameter:    	type:
  					    menuId:指定被定位应用的目录ID

  Output parameter:   	起始记录号

  Description:        	定位下载列表的第一项的记录号(最新推荐固定为0x01)

  Coder:			  	xuqs

  Status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE OTALocatIndexInServerListFile(BYTE menuId)
{
	BYTE index, tmp, nullindex, num;

	nullindex = 0x00;

	num = GetLFRecNumLgth(G_ServerListFileAddr, FileNum);

	for(index = TYPE_Download_List; index <= num; index += 5)
	{
		ReadLFBuf(G_ServerListFileAddr, index, 0x01, &tmp, sizeof(tmp));

		if (tmp == menuId)
		{
			return index;
		}
		else if((tmp == 0xFF) &&(nullindex == 0x00))
		{
			nullindex = index;
		}
		else	//for baihe-test
		{;}
	}

	return nullindex;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 	OTADeleteApp

  Input parameter:    	IsDownloadList: 是否加入下载列表
  			type: 			D_DelNotFinishTagApp 主动删除时添加删除标记，待发送删除通知
						D_DelTagApp	远程/网上删除时添加删除标记，不用发送删除通知

  Output parameter:   	void

  Description:        	删除应用

  Coder:			  	xuqs

  Status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTADeleteApp(BYTE IsDownloadList, BYTE type)
{
	BYTE xdata menu[3];

	ReadTFBuf(G_AppFileAddr, G_STKAppOffset, &menu[0], 3);

	if (menu[0] == 0x9F)
	{
		OTARemoveEventReg(menu, TRUE);
	}

	menu[1] |= type;							//扩展AID的BIT11，标记删除的应用（需待空间整理）

	SetTFConst(G_AppFileAddr, G_STKAppOffset+1, menu[1], 1);

	if (IsDownloadList == TRUE)						//删除的应用加入到下载列表
	{
		ReadTFBuf(G_AppFileAddr, G_STKAppOffset, (BYTE *)&ResData[0], 0x23);

		if(menu[0] < 0xA0)     //非一级应用添加到下载列表
		{
			OTAAddAppTOServerList(TYPE_Download_List, OTALocatIndexInServerListFile(menu[0]),(BYTE *)&ResData[0], FALSE);
		}
		else                  //一级应用添加到目录列表
		{
			OTAAddAppTOServerList(TYPE_Catalog_List, TYPE_Catalog_List,(BYTE *)&ResData[0], FALSE);
		}
	 }
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 	OTAAppDataResume

  Input parameter:    	void

  Output parameter:

  Description:        	OTA应用空间整理时的断电恢复

  Coder:			  	xuqs

  Status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAAppDataResume(void)
{
	BYTE idata result;
	UWORD xdata fileAddr;
	WORD xdata offset;

	fileAddr = SelectLFFile(DF_OTA, EF_OTASecurity);

	G_AppFileAddr = SelectTFFile(DF_OTA, EF_OTAScript);

	if ((fileAddr == FALSE)||(G_AppFileAddr == FALSE))
	{
		return;
	}

	ReadLFBuf(fileAddr, 0x01, 0x00, &result, sizeof(BYTE));

	if (result != 0xFF)
	{
		 ReadLFBuf(fileAddr, 0x01, 0x01, (BYTE *)&G_STKAppOffset, sizeof(WORD));

		 ReadLFBuf(fileAddr, 0x01, 0x03, (BYTE *)&offset, sizeof(WORD));

		 OTAPureAppScript(fileAddr, offset);
	}
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAIsMenuActive

  input parameter:   menuid

  output parameter:	 TRUE/FALSE

  description:     	 判断目录是否起用

  status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE OTAIsMenuActive(BYTE menuid)
{
	BYTE idata tmp;
	BYTE idata result;

	result	= OTAMenuAccess(menuid, MENU_OFFSET_MENU_ATTRIBUTE, &tmp, ACCESS_READ, MenuType_OTA);

	if(result == FALSE)
	{
		return FALSE;
	}

	if((tmp&0x80) == 0x80)
	{
		return TRUE;
	}

	return FALSE;
}

/*---------------------------------------------------------------------------------------------------

Function name	:	OTAMenuAddItem

Input parameter	:	buf:	目录数据(目录索引＋应用名称)。(详见应用数据组成定义)
input parameter :   mode    :0x00:OTA模式/0x01：OTP模式

Iutput parameter:	成功：	返回TRUE
					失败：	返回FALSE

Description		:	根据EF_Menu中添加一条新记录

Coder			:	ZhuLi

Status			:
---------------------------------------------------------------------------------------------------------*/

BYTE OTAMenuAddItem(BYTE * buf,BYTE mode)
{
	UWORD xdata fileAddr;
	BYTE  idata i, recNum, recordNo;
	BYTE  xdata tmp[5];

	fileAddr = SelectLFFile(DF_OTA, EF_Menu);

	recNum = GetLFRecNumLgth(fileAddr, FileNum);

	for(recordNo = 0,i = 1; i <= recNum; i ++)
	{
		ReadLFBuf(fileAddr, i, 0, &tmp[0], sizeof(tmp));

		if ((tmp[0] == 0x04)&&(tmp[2] == buf[0]))						//menuType & menuId
		{
			return FALSE;
		}

		if((recordNo == 0x00) && ((tmp[0] == 0x00)||(tmp[0] == 0xFF)))	//待添加目录的记录号
		{
			recordNo = i;
		}
	}

	if(recordNo != 0x00)
	{
		tmp[0] = 0x04;

		tmp[1] = recordNo;						//Item Identifier = 记录号

		tmp[2] = buf[0];						//Menu ID

		tmp[3] = 0x01;							//起始页码

		if(buf[0] == 0x9F)                      //9F 目录对用户是透明的
		{
			tmp[4] = 0x00;
		}
		else
		{
			tmp[4] = 0x88;						//新增OTA目录默认有"应用管理"项
		}

		if(mode == 0x01)                      
		{
			tmp[4] = 0x80;	  					//OTP均已MT方式下发，无"应用管理"项
		}

		WriteLFBuf(fileAddr, recordNo, MENU_OFFSET_NAME, &buf[1], buf[1] + 1);	//目录名称

		WriteLFBuf(fileAddr, recordNo, 0, tmp, 5);

		return TRUE;
	}

	return FALSE;
}

/*-----------------------------------------------------------------------------------------------------

Function name	:	OTARegisterCurrentSession

Input parameter	:	Parameter:	当前操作的参数（列表更新为目录ID、菜单搜索为目录ID、MO下载为AID）
					oprationType:操作的命令类型

Output parameter:	成功：	返回TRUE.
					失败：	返回FALSE.

Description		:	在实时登记文件中登记当前的操作与相关参数

Coder			:	xuqs

Status			:
-----------------------------------------------------------------------------------------------------------*/
BYTE OTARegisterCurrentSession(BYTE parameter[3], BYTE oprationType)
{
	UWORD xdata fileAddr;
	BYTE  xdata buf[4];

	fileAddr = SelectTFFile(DF_OTA, EF_RuntimeReg);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	memset(&buf[0], 0x00, sizeof(buf));					//清buf为全0

	buf[0] = oprationType;

	switch(oprationType)
	{
		case MO_Download:
		case D_AppClose:
			memcpy(&buf[1], parameter, 3);
			break;

		case List_Update:
		case Menu_Search:
			buf[1] = parameter[0];                      //保存目录索引
			break;

		default:
			return FALSE;
	}

	WriteTFBuf(fileAddr, 0, buf, 4);

	return TRUE;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTADataReceivedAck

  Input parameter:   void

  Output parameter:	 void

  Description:       上发大数据量信息接收确认信息

  Coder:			 xuqs

  Status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE OTADataReceivedAck(void)
{
	RecData[0] = ReadVarible(OTABigDataResult, &RecData[1]);

	if (RecData[0] == FALSE)
	{
		return FALSE;
	}

	tempBuf[1] = 0x06;							//命令类型

	tempBuf[3] = RecData[1];						//批次号

	if(RecData[2] == 0)								//本组完成
	{
		tempBuf[2] = 2;							    //命令长度。

		tempBuf[4] = 1;								//本组完成。
	}
	else										    //本组未完成
	{
		tempBuf[2] = 2 + RecData[2] + 1;			//命令长度。

		tempBuf[4] = 0;								//本组未完成。

		memcpy(&tempBuf[5], &RecData[2], RecData[2]+1);
	}

	tempBuf[0] = tempBuf[2] + 2;					//数据长度

	OTASendUplinkCommandMessage(0x01, &tempBuf[0]);

	ClearVariable(OTABigDataResult);

	return TRUE;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAProcessInterApp

  input parameter:   void

  output parameter:	 void

  description:       OTA交互应用执行函数

  Coder:			 xuqs

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAProcessInterApp(void)
{
	G_STKAppOffset = G_AppOffset;

	OTAStartRun();

    G_STKAppOffset = FALSE;
}
/*
*@description : 组装信息同步响应上行数据，并发送出去
*
*@param       :aidCount :当前信息同步中将上发的AID数目
*@param       :menuCount：待上发的目录数据总数
*@param       :SubmitType :0x00 :OTA同步响应数据 0x01-OTP同步响应数据
*
*@return      :void
*
*@history     :create by xyz@20100713 14:46
*/
void SendSubmitInfoSyn(BYTE aidCount,BYTE menuCount,BYTE SubmitType)
{
	BYTE idata smsCount,sendCount,i;
	BYTE idata smsIndex,aidCurrent;
	UWORD xdata fileAddr;
	UWORD xdata synfileAddr;

	synfileAddr = SelectTFFile(DF_OTA,EF_SubmitSyn);  //选择6EA5 文件
	
	smsCount = aidCount / D_MaxServiceNum;

	if ((aidCount%D_MaxServiceNum) > 0)
	{
		smsCount ++;
	}

	if(smsCount == 0x00)
	{
		smsCount = 0x01;
	}
	
	aidCurrent = 0x00;  //当前已发送的AID个数

	sendCount = 0x00;

	for(smsIndex = 0x01; smsIndex <= smsCount; smsIndex ++)
	{

		if(SubmitType == 0x00)
		{
			tempBuf[1] = 0x05;//命令类型
		}

		tempBuf[3] = smsCount;

		tempBuf[4] = smsIndex;

		i = 5;

		if (smsIndex == 0x01)	 										//第一条短信
		{
			tempBuf[5] = menuCount;									   	//目录索引总数

			if(SubmitType == 0x00)
			{
				ReadVarible(OTAMenuIdTag, &tempBuf[6]);
			}
			
			i = i + tempBuf[5] + 1;
		}

		tempBuf[i] = (smsIndex == smsCount) ? (aidCount - (smsIndex - 1)*D_MaxServiceNum) : D_MaxServiceNum;	//本条短信AID总数

		sendCount = tempBuf[i];    //保存当前发送的AID的个数

		if (tempBuf[i] != 0x00)
		{		
			ReadTFBuf(synfileAddr,aidCurrent*3, &tempBuf[i+1],sendCount *3);  // 从上次读取偏移末尾处读取sendCount 个应用AID的数据到输出buf

			aidCurrent += 	sendCount;		
		
			if (smsIndex == 0x01)
			{
				tempBuf[2] = sendCount*3 + tempBuf[5] +4;
			}
			else
			{
				tempBuf[2] = sendCount*3 +3;
			}
		}
		else
		{
			tempBuf[2] = tempBuf[5] +4;
		}

		if(smsIndex == smsCount)      //最后一条短信中添加COS版本号
		{
			if(SubmitType == 0x00)
			{
				fileAddr = SelectTFFile(DF_OTA, EF_OTAConfig);

				ReadTFBuf(fileAddr,0x0B,&tempBuf[tempBuf[2] +3],0x02);
			}

			tempBuf[2] += 2;
		}
		tempBuf[0] = tempBuf[2] +2;

		if(SubmitType == 0x00)
		{
			OTASendUplinkCommandMessage(0x01, &tempBuf[0]);
		}
	}
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTASubmitSynchronizeInfo

  input parameter:   void

  output parameter:	 TRUE/FALSE

  description:       上发信息同步响应

  Coder:			 xuqs

  status:

 -------------------------------------------------------------------------------------------------------------------*/
/*
*@description :将信息同步响应数据暂时缓存在文件6EA5中
*/
BYTE OTASubmitSynchronizeInfo(void)
{
	BYTE idata i, num, menuCount, aidCount;
	WORD idata datalen;
	BYTE xdata tmp[5];
	UWORD xdata fileAddr;
	UWORD xdata synfileAddr;  // 信息同步数据缓存文件,只存放全部的AID,不存放目录数据
	WORD idata synOffset;   //缓存数据偏移

	

	fileAddr = SelectLFFile(DF_OTA, EF_Menu);

	synfileAddr = SelectTFFile(DF_OTA,EF_SubmitSyn);  //选择6EA5 文件

	if ((fileAddr == FALSE)||(synfileAddr == FALSE))
	{
		return FALSE;
	}

	num = GetLFRecNumLgth(fileAddr, FileNum);

	aidCount = 0;

	synOffset = 0x00; //初始化缓存数据偏移

	menuCount = 0;

	tempBuf[0] = 0x00;

	for (i=1; i<=num; i++)
	{
		ReadLFBuf(fileAddr, i, 0x00, &tmp[0], sizeof(tmp));
		
		if (tmp[0] == 0x04)									//OTA目录或固化可更名应用
		{
			if ((tmp[4]&D_AcitveAID) == D_AcitveAID) 		//已启用
			{
#ifdef LOCALOTA
				if (tmp[2] < 0xA0)						   //目录
#else
	  			if ((tmp[2] < 0xA0) || ((tmp[2] > 0xEF)&&(tmp[2] < 0xFC)) ||(tmp[2] == 0xFF))	//目录及固化可更名应用
#endif
				{
					tempBuf[tempBuf[0]+1] = tmp[2];

					tempBuf[0]++;		
				}
#ifdef LOCALOTA
				else if ((tmp[2] >= 0xA0)&&(tmp[2]<= 0xB7))					//固化可更名应用				
				{
					tmp[0] = tmp[2];

					tmp[1] = 0xC0;
			
					tmp[2] = 0x00;
				
					WriteTFBuf(synfileAddr,synOffset,&tmp[0],0x03);

					synOffset += 3;  //计算下一个待写入AID的偏移地址

					aidCount ++;
				}
				else	//for baihe-test
				{;}
#endif
			}
		}	
	}

	menuCount = tempBuf[0];

	WriteVariable(OTAMenuIdTag, &tempBuf[0]);				//保存目录ID到ROM

	for(i=0x00;i<0x02;i++)			//OTA应用AID			
	{
		for (G_STKAppOffset = 0; ; G_STKAppOffset += datalen)
		{
			datalen = OTASearchAID(0x00, D_DelTagApp, TRUE);

			if (datalen != FALSE)
			{
				ReadTFBuf(G_AppFileAddr, G_STKAppOffset, &tmp[0], 0x03);

				if(i == 0x00)             //先上发一级应用
				{
					if(tmp[0] < 0xA0)
					{
						continue;
					}
				}
				else
				{
					if(tmp[0] >= 0xA0)      //后上发各目录下的应用
					{
						continue;
					}
				}

				if (((tmp[1]&D_AcitveAID) != D_AcitveAID) ||((OTAIsMenuActive(tmp[0]) == FALSE)&&(tmp[0] != 0x9F)))  // 禁用的应用信息不上发
				{
					continue;
				}

				WriteTFBuf(synfileAddr,synOffset,&tmp[0],0x03);

				synOffset += 3;  //计算下一个待写入AID的偏移地址

				aidCount ++;
			}
			else
			{
				break;
			}
		}

		if(i == 0x00)
		{
			G_FileNum = TYPE_Catalog_List;
		}
		else
		{
			G_FileNum = TYPE_Recommend_List;
		}

		for (; G_FileNum <= GetLFRecNumLgth(G_ServerListFileAddr, FileNum); G_FileNum ++)	//下载列表/最新推荐列表AID
		{
			ReadLFBuf(G_ServerListFileAddr, G_FileNum, 0x00, &tmp[0], 0x04);

			if(i == 0x00)
			{
				if(tmp[0] != TYPE_Catalog_List)  //上发一级"应用管理"下的目录下载列表
				{
					continue;
				}
				if((tmp[1] >0x9F)&&((tmp[2]&D_EnableApp)!=D_EnableApp)) //一级应用禁用
				{
					continue;
				}
			}
			else
			{
				if(tmp[0] == TYPE_Catalog_List)  //上发各目录下的应用下载列表
				{
					continue;
				}

				if (((tmp[2]&D_EnableApp) != D_EnableApp) ||(tmp[0] == 0xFF) ||((OTAIsMenuActive(tmp[1]) == FALSE)&&(tmp[1] != 0x9F)))
				{
					continue;
				}
			}

			WriteTFBuf(synfileAddr,synOffset,&tmp[1],0x03);
	
			synOffset += 3;  //计算下一个待写入AID的偏移地址
	
			aidCount ++;
		}
	}

	SendSubmitInfoSyn(aidCount, menuCount,0x00);

	return TRUE;
}
/*
*@description :获取命令执行结果
*
*@param        :void
*
*@return       :CMD_Success\ExitStkPerform\BackMove\
*/
BYTE GetResResult(void)
{
	BYTE idata pos;

	pos = FindTag(&RecData[0],TLV_RESULT,P3);

	if ( pos == 0x00)
	{
		return ExitStkPerform;
	}
	else
	{
		if(RecData[pos+1] == 0x11)
		{
			return BackMove;
		}
		else if (RecData[pos+1] == 0x10)
		{
			return ExitStkPerform;
		}
		else
		{
			return CMD_Success;
		}
	}
}
/*
*@description  : 同步一级应用，将一级应用的菜单名称写入指定的菜单文件空记录中
*
*@param         : appAddr :应用文件地址 
*@param         :menuAddr:菜单文件地址
*@param          :synType:0x00-同步OTA一级菜单，0x01-同步OTP一级菜单
*
*@returns     :void
*
@history       :create by xyz@20100714 9:32
*/
void SynAppMenuName(UWORD appAddr,UWORD menuAddr,BYTE synType)
{
	WORD  xdata offset, dataSize;

	for(offset=0; offset<=GetTFSize(appAddr);)
	{
		ReadTFBuf(appAddr, offset, &tempBuf[0], 5);

		dataSize = ((tempBuf[3]<<8)&0xFF00) + tempBuf[4];

		if((tempBuf[3] == 0xFF)&&(tempBuf[4] == 0xFF))
		{
			return;
		}
		if(   ((tempBuf[0] > 0x9F)&&(synType == 0x00) )   || ( (tempBuf[2] == 0x00)&&(synType == 0x01) )  )  //OTA一级应用或OTP目录应用,即大类菜单的二级
		{
			if( ((tempBuf[1]&0x80) == 0x00) ||((tempBuf[1]&D_DelTagApp) == D_DelTagApp)) //禁用或已删除
			{
				MenuAcess(tempBuf[0], FALSE, NULL, ACCESS_DELETE_CATALOG, MenuType_OTA,menuAddr);  //删除6F18中存在的目录索引
			}
			else
			{
				if( MenuAcess(tempBuf[0], 0, NULL, ACCESS_LOCATE, MenuType_OTA,menuAddr) == FALSE)
				{	
					ReadTFBuf(appAddr, offset+5, &tempBuf[1], sizeof(BYTE));

					ReadTFBuf(appAddr, offset+6, &tempBuf[2], tempBuf[1]);	//获取目录名称
					
					OTAMenuAddItem(&tempBuf[0],synType);
				}
			}
		}
		offset = offset + dataSize;
	}

}
/*--------------------------------------------------------------------------------------------------------------------
  Function name:	OTASynMenuName

  input parameter:	void

  output parameter:	void

  description:		同步出现在零级菜单应用的名称,即把零级菜单的应用名称写入6F18的空记录中

  author:			Zhuli

  status:			OK
 -------------------------------------------------------------------------------------------------------------------*/

void OTASynMenuName(void)
{
	UWORD xdata menuAddr;

	menuAddr = SelectLFFile(DF_OTA,EF_Menu);
	
	if((menuAddr == FALSE)||(G_AppFileAddr==FALSE))			
	{
		return;
	}

	SynAppMenuName(G_AppFileAddr,menuAddr,0x00);
}

/**
 * @brief 		以BIP模式上发OTA上行命令
 *
 * @param 		uplinkData : 上发的数据（LV结构，其中L占两字节）
 *
 * @Returns   	TRUE:发送成功 / FALSE:发送失败
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @080610 15:37
 */

BYTE OTASendUplinkCommandInBIPFormat(BYTE * uplinkData)
{
	BYTE idata tmp;
	WORD idata offset;
	WORD idata dataLen;
	BYTE idata cmq;

	dataLen = ((uplinkData[0]<< 0x08) & 0xFF00) + uplinkData[1] + 2;

	if((G_OpenChannel & BIPCurrentOpened) != BIPCurrentOpened) // if the channel is closed, open it !
	{
		G_BIPChannelId = OTAOpenChannelHandle();

		if(G_BIPChannelId == FALSE)
		{
			return FALSE;
		}

		G_OpenChannel |= BIPNeedClosed;					//通道由卡端打开，应由卡端负责关闭
	}

	offset = 0;								// channel currently is opened . send data here

	while(dataLen > offset)
	{
		if(dataLen - offset > 0xE0)
		{
			tempBuf[0] = 0xE0;

			cmq = 0x00;
		}
		else
		{
			tempBuf[0] = dataLen - offset;

			cmq = 0x01;						//最后一个SendData命令限定符是01
		}

		memcpy(&tempBuf[1], &uplinkData[offset], tempBuf[0]);

		tmp = cephSendDataSingle(cmq, G_BIPChannelId, &tempBuf[0]);

		if(tmp != CMD_Success)
		{
			return FALSE;
		}

		offset += 0xE0;
	}

	if((G_OpenChannel&BIPNeedClosed) == BIPNeedClosed) 			// 通道由卡端打开，应由卡端负责关闭
	{
		if(uplinkData[25] == 0x03)  
		{
#ifdef LOCALOTA
			if (G_LevelApp == TRUE)	   						//若删除目录，需等待服务器下发目录管理的删除下行
#endif
			{
				cephCloseChannelSingle(0x00, G_BIPChannelId);
	
				G_OpenChannel &= BIPCurrentClosed;			//通道为关闭状态
			}
		}

	}

	return TRUE;
}

/**
 * @brief 		OTA处理通道数据下行事件函数
 *
 * @Returns   	TRUE:成功 / FALSE:失败
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @080610 15:37
 */
BYTE DataAvailable(BYTE pos)
{
	if((RecData[pos] & 0x7F) != 0x38)
	{
		return FALSE;
	}

	if(RecData[pos+1] != 0x02)
	{
		return FALSE;
	}

	if((RecData[pos+2]&0x80) != 0x80)
	{
		return FALSE;
	}

	G_BIPChannelId = RecData[pos+2]&0x07;

	if(G_BIPChannelId == 0x00)
	{
		return FALSE;
	}

	if((RecData[pos+4] & 0x7F) != 0x37)
	{
		return FALSE;
	}

	if(RecData[pos+5] != 0x01)
	{
		return FALSE;
	}

	G_BIPDataLgth = RecData[pos+6];		//point to data length;

	PendingMessageAlert(D_DataAvaiable,0x00);

	return TRUE;
}

/**
 * @brief 		在标记处理时候的处理通道数据下行的接收和存储
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @080610 15:37
 */
UWORD xdata G_BipDataAddr;
extern BYTE OTABIPPPHanlder(BYTE pos);
void OTADataAvaiable(void)
{
	WORD idata offset;
	BYTE idata pos, length;
	BYTE idata result, ptr;
	//BYTE idata * dataStr;
	
	if(G_BipDataAddr == FALSE)    //未创建6EAF文件
	{
		return;
	}

	offset = 0x00;

	while(TRUE)
	{
		length = (G_BIPDataLgth > 0xC8) ? 0xC8 : G_BIPDataLgth;

		pos = cephReceiveDataSingle(G_BIPChannelId, 0x00, NULL, length);
		
		
		if(pos != CMD_Success)
		{
			return;
		}
		
		pos = FindTag((BYTE *)&RecData[0],TLV_CHANNELDATA,P3);
		
		if (pos != FALSE)
		{
		 	if((offset+ RecData[pos]) > 0x400) //只能接受1K的BIP数据
			{
				return;
			}

			WriteTFBuf(G_BipDataAddr, offset, &RecData[pos+1], RecData[pos]);

			offset += RecData[pos];
		}

		pos = FindTag((BYTE *)&RecData[0],TLV_CHANNELDATALENGTH,P3);

		if ((pos == FALSE)||(RecData[pos+1] == 0x00))
		{
			break;
		}		
		else
		{
			G_BIPDataLgth = RecData[pos+1];
		}
	}

	G_OpenChannel |= BIPCurrentOpened;					//通道已打开

	G_ResponseMode = Mode_BIP;							//若有通道数据，则设置为BIP承载方式

	ReadTFBuf(G_BipDataAddr, 0x00, &RecData[0],0xFF);

	ptr = 2;

  result = FindTag(&RecData[ptr+1], 0x70, RecData[ptr]);

	if (result != FALSE)
	{
		ptr = ptr + RecData[ptr] + 3;				//point to the CHL

		if(RecData[ptr] != 0x11)
		{
			return;
		}

		ptr = ptr +5;                              //point to TAR

		if(RecData[ptr] != 0xB0 || (RecData[ptr+1] != 0x00))	//check TAR 2/3
		{
			return;
		}
                      
		switch (RecData[ptr+2])
		{
			case OTA_APP_TAR:						//OTA命令数据
				G_ResponseMode = Mode_BIP;
				
				if((RecData[ptr-4] != 0x02)||(RecData[ptr-3] != 0x00)) //SPI值固定为0200
				{
					return;
				}	
				OTAPPHanlder(&RecData[5]);
				break;
			

#ifdef LOCALOTA
			case OTA_RFM_TAR:                        //江苏远程文件更新允许BIP模式
				G_ResponseMode = Mode_BIP;
				
				if((RecData[ptr-4] != 0x06)||(RecData[ptr-3] != 0x00)) //SPI值固定为0600
				{
					return;
				}	

				G_RFMTag = 0xB0;

				SaveBIPRFMDataSms(0);
				break;					
#endif	

			default:						
				break;
		}	
	}
	else
	{
	
	}
}

/**
 * @brief 		OTAOpenChannelHandle ,根据文件的内容打开一个Channel
 *
 * @Returns   	成功：返回Channel id
 *
 *				失败：返回FALSE
 *
 * @author 		xyz
 *
 * @history 	created by xyz, @081211
 */
#ifdef LOCALOTA
static code BYTE bearer[] = {0x06,0x00,0x00,0x03,0x00,0x00,0x02};
#else
static code BYTE bearer[] = {0x06,0x02,0x03,0x03,0x06,0x1F,0x02};
#endif

BYTE OTAOpenChannelHandle(void)
{
	BYTE  idata bufferSize[2];
	BYTE  idata result;
	BYTE  xdata ipAddr[8];
	BYTE  xdata networkAcessName[0x0B];
	UWORD xdata fileAddr;

	fileAddr = SelectTFFile(DF_OTA, EF_OTAConfig);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadTFBuf(fileAddr, 0x04, &ipAddr[1], 0x07);			//获取BIP配置，包括下载模式、IP地址和端口号

	ipAddr[0] = 0x05;

	ipAddr[1] = 0x21;

#ifdef LOCALOTA
	bufferSize[0] = 0x05;

	bufferSize[1] = 0x78;
#else
	bufferSize[0] = 0x02;

	bufferSize[1] = 0x00;
#endif

#ifdef LOCALOTA
	networkAcessName[0] = 0x00;

	ReadTFBuf(fileAddr, 0x0D, &networkAcessName[0], sizeof(BYTE));
	
	if((networkAcessName[0] == 0xFF)||(networkAcessName[0] == 0x00) ||(networkAcessName[0] >0x0A))   // 无networkaccessname
	{
		result = cephOpenChannelSingle(0x01, &ipAddr[0], 0x02, (BYTE CONST*)bearer, (BYTE *)bufferSize, &ipAddr[6], 0x02,NULL);	
	}
	else
	{
		ReadTFBuf(fileAddr, 0x0E, &networkAcessName[1], networkAcessName[0]);

		result = cephOpenChannelSingle(0x01, &ipAddr[0], 0x02, (BYTE CONST*)bearer, (BYTE *)bufferSize, &ipAddr[6], 0x02,(BYTE *)networkAcessName);	
	}

#else
	result = cephOpenChannelSingle(0x01, &ipAddr[0], 0x02, (BYTE CONST*)bearer, (BYTE *)bufferSize, &ipAddr[6], 0x02,NULL);
#endif

	if(GetResponseResult()== CMD_Success)
	{
		return result;
	}
	else
	{
		return FALSE;
	}
}

/**
 * @brief 		OTABipPushHandler , BIP Push请求响应的上行命令
 *
 * @param   	type:OTA BIP Push响应请求类型
 *
 * @author 		xyz
 *
 * @history 	created by xyz, @081211
 */
void OTABipPushHandler(BYTE type)
{
	BYTE  idata result;

	G_OpenChannel &= BIPCurrentClosed;					//通道关闭状态

	if(type == D_BipPushClose)
	{
		if(G_BIPChannelId == FALSE)
		{
			result = cephGetChannelStatusSingle(0xFF);         //只有在关闭通道时,才获取通道状态;获得当前通道ID 

			if((result == BackMove)||(result == ExitStkPerform))
			{
				return;
			}
	
			result = cephCloseChannelSingle(0x00, result);
		}
		else
		{
			result = cephCloseChannelSingle(0x00, G_BIPChannelId) ;
		}

		if(result != CMD_Success)
		{
			G_OpenChannel |= BIPCurrentOpened;
		}
	}
	else
	{
		G_BIPChannelId = OTAOpenChannelHandle();

		if(G_BIPChannelId== FALSE)
		{
			return;
		}

		G_OpenChannel |= BIPCurrentOpened;

		UpdateDownloadConfig(TRUE);//BIP PUSH命令需要更新OTA配置文件的BIP下载状态

		tempBuf[0] = 0x03;

		tempBuf[1] = 0x0B;

		tempBuf[2] = 0x01;

		tempBuf[3] = G_BIPPushCntr;

		OTASendUplinkCommandMessage(1, &tempBuf[0]);
	}
}

/*
*@description:执行特定的OTA应用操作
*
*@param      :aid[3]
*
*@return     :void 
*
*@history    :create by xyz 20623 14:38
*/
void ProcessOTAApp(BYTE aid[3])
{
	G_STKAppOffset = OTAGetAppOffset((BYTE CONST*)&aid[0]);	//查找AID的偏移

	if (G_STKAppOffset != ErrorNoApp)
	{
		OTAStartRun();
	}
	
	G_STKAppOffset = 0;
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTAGdEventDownloadData

  input parameter:  		void

  output parameter:	 	TRUE/FALSE

  description:       		OTA广东修改版本的事件型应用触发流程

  coder:				xuqs

  test:

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAGdEventDownloadData(void)
{
	BYTE idata i;
	BYTE idata pos;
	UWORD xdata fileAddr;

	fileAddr = SelectLFFile(DF_OTA, EF_OTAEventRegister);

	if (fileAddr == FALSE)
	{
		return;
	}

	pos = 0x00;

	if(RecData[0] == 0xD6)
	{
		if(RecData[1] == 0x81)
		{
			pos = 3;
		}
		else
		{
			pos = 2;
		}

		if((RecData[pos]& 0x7F) != 0x19)		//event tag
		{
			return;
		}
		pos = pos + 2;	
	}	

	G_EventTag = RecData[pos];

	i = LocateRecordInFile(fileAddr, RecData[pos]);	//查找事件是否注册

	if (i == FALSE)					                //事件没注册，不触发应用
	{
		return;
	}
	
	if(RecData[0] == 0xD6)
	{
		OTASaveEventData(FALSE);
	}
	else
	{
		OTASaveEventData(TRUE);
	}

	PendingMessageAlert(D_GDOTAEventRun,0x00);
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTASaveEventData

  input parameter:  		type	:TRUE	非D6
  						FALSE	D6

  output parameter:	 	void

  description:       		OTA广东修改版本的保存事件的详细数据

  coder:				xuqs

  test:

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTASaveEventData(BYTE type)
{
	BYTE idata shift, ch;
	BYTE idata savelen, alllen;
	UWORD xdata fileAddr;

	if (RecData[1] == 0x81)
	{
		ch = 0x02;
	}
	else
	{
		ch = 0x01;
	}
	alllen = RecData[ch];

	if (type == TRUE)
	{
		shift = FindTag(&RecData[ch+1], TLV_DEVICEIDENTITY, RecData[ch]);
	}
	else
	{
		shift = FindTag(&RecData[ch+1], TLV_EVENTLIST, RecData[ch]);
	}

	savelen = alllen - RecData[ch+shift+1] -2;			//DEVICEIDENTITY/EVENT LIST后的所有字段的长度

	shift =ch + shift +RecData[ch+shift+1]+2;


	fileAddr = SelectTFFile(DF_OTA, EF_OTAEventContent);

	if (fileAddr == FALSE)
	{
		return;
	}
	
	WriteTFBuf(fileAddr, 0x00, &G_EventTag, sizeof(BYTE));

	WriteTFBuf(fileAddr, 0x01, &savelen, sizeof(BYTE));

	WriteTFBuf(fileAddr, 0x02, &RecData[shift], savelen);			//保存事件详细数据到文件6EB0，解析器可以直接从这个文件里读取数据
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTAEventDataReg

  input parameter:  		void

  output parameter:	 	void

  description:       		OTA应用下载的事件注册

  coder:				xuqs

  test:

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAEventDataReg(void)
{
	ProcessOTAApp(&G_EventAID[0]);

	if (G_IsUpdateEventList == TRUE)	//事件列表更新后，才通知终端更新事件列表
	{
		PendingMessageAlert(D_OTASetUpEventList,0x00);
	}
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTAEventRun

  input parameter:  		eventTag

  output parameter:	 	void

  description:       		OTA应用下载的事件执行

  coder:				xuqs

  test:

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAEventRun(void)
{
	UWORD xdata fileAddr;
	BYTE  idata record;
	BYTE  idata i, num;
	BYTE  idata aid[3];

	fileAddr = SelectLFFile(DF_OTA, EF_OTAEventRegister);

	if (fileAddr == FALSE)
	{
		return;
	}
	record = LocateRecordInFile(fileAddr, G_EventTag);

	if (record == FALSE)
	{
		return;
	}

	ReadLFBuf(fileAddr, record, 0x01, &num, sizeof(BYTE));

	for (i=0; i<num; i++)
	{
		ReadLFBuf(fileAddr, record, 0x02+i*3, &aid[0], 3);

		ProcessOTAApp(&aid[0]);
	}
}

/*--------------------------------------------------------------------------------------------------------------------
  Function name:	OTARemoveEventReg

  input parameter:	aid[3]: 需要删除的AID
  				IsSetupEventList: TRUE 要求更新事件列表
  							FALSE 不要求更新事件列表

  output parameter:	void

  description:		删除在事件登记文件中的AID

  author:			Zhuli

  status:			OK
 -------------------------------------------------------------------------------------------------------------------*/

void OTARemoveEventReg(BYTE  aid[3], BYTE IsSetupEventList)
{
	UWORD xdata eventTableAddr;
	BYTE idata recordNum,i, j,tag;
#ifdef LOCALOTA
	BYTE idata eventId;
#endif

	eventTableAddr = SelectLFFile(DF_OTA, EF_OTAEventRegister);

	if(eventTableAddr == FALSE)
	{
		return ;
	}

	tag = FALSE;    //在删除一个触发型应用时，仅打一个标记值D_OTASetUpEventList

	recordNum = GetLFRecNumLgth(eventTableAddr,FileNum);

	for(i = 1; i <= recordNum; i ++)
	{
		ReadLFBuf(eventTableAddr, i, 0x01, &RecData[0], sizeof(BYTE));

		if(RecData[0] == 0xFF)
		{
			continue;
		}

		ReadLFBuf(eventTableAddr, i, 0x02, &RecData[1], RecData[0]*3);

		for(j=0; j<RecData[0]; j++)
		{
			if((aid[0] == RecData[j*3 + 1]) && ((aid[1] & 0x03) == (RecData[j*3 + 2] & 0x03)) && (aid[2] == RecData[j*3 + 3]))
			{
				RecData[0]--;

				if(RecData[0] == 0x00)
				{
					if (IsSetupEventList == TRUE) //更新应用时，删除注册事件以及增加注册事件后才通知终端更新事件列表
					{
#ifdef LOCALOTA
						ReadLFBuf(eventTableAddr, i, 0x00, &eventId, sizeof(BYTE));
	
						if((eventId < 0x0B)&&(tag == FALSE))
						{
							tag = TRUE;

							PendingMessageAlert(D_OTASetUpEventList,0x00);
						}
#else
						if(tag == FALSE)
						{
							tag = TRUE;

							PendingMessageAlert(D_OTASetUpEventList);
						}
#endif
					}

					SetLFConst(eventTableAddr, i, 0x00, 0xFF, 2);
				}
				else
				{
					memmove(&RecData[j*3+1], &RecData[j*3+4], RecData[0]*3);

					WriteLFBuf(eventTableAddr, i, 0x01, &RecData[0], RecData[0]*3+1);
				}
			}
		}
	}
}

/*
*@description:判断当前应用是否已注册，已注册，返回TRUE;若未注册，则注册
*
*@param      :	DFName 
				EFName
				mode :	RegStatus_Get	获取注册状态
						RegStatus_Set	设定注册状态
*
*@returns    :TRUE	 已注册	
			  FALSE	 未注册
*
*@history    :create by xyz @100607 14:44
*/

BYTE GetSetRegStatus(WORD dfName, WORD efName, BYTE mode)
{
	BYTE idata result;
	UWORD xdata fileAddr;

	fileAddr = SelectTFFile(dfName, efName);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadTFBuf(fileAddr, 0x00, &result, sizeof(BYTE));

	if((result&0x80) == 0x80)	//用户已注册.
	{
		return TRUE;
	}
	else		//未注册		
	{
		if(mode == RegStatus_Set)		//设定注册状态
		{
			result  |=  0x80;

			WriteTFBuf(fileAddr, 0x00, &result, sizeof(BYTE));
		}

		return FALSE;
	}
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 AccessDefined                                                                                                                   
                                                                                                                                                
  input parameter:   fileaddr:the start address of ef file head

					 action : ef's operation
                                           
  					                                                                                                         
  output parameter:  TRUE/FALSE                                                                                                               
                                                                                                                                                
  description:       define the  accessConditon of the action
  
  status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE AccessDefined(UWORD fileAddr,BYTE action)
{
	BYTE idata actionStatus;

	actionStatus = GetAcessPurview(fileAddr,action);

	if((actionStatus == FALSE)||(actionStatus == 0x90)||(actionStatus == 0x97))
	{
		return actionStatus;
	}
	else
	{
		if((actionStatus&0x0F) < 0x0A)	 //PIN
		{
			return 0x01;
		}
		else				             //ADM
		{
			return 0x0A;
		}
	}
}
/*
*@description:在应用文件中删除特定目录下的所有应用
*
*@param      :目录ID
*
*@return      :void
*
*@history    :create by xyz@100609 16:50
*/
void DeleteAppUnderMenu(BYTE menuId)
{
	WORD idata offset;

	if(menuId == 0x00)  
	{
		return;
	}

	for (G_STKAppOffset = 0; ;G_STKAppOffset += offset)
	{	
		offset = OTASearchAID(menuId,D_AllAID,FALSE);   //仅查找属于当前目录下的AID，不查找所有应用

		if(offset != FALSE)
		{
			OTADeleteApp(FALSE, D_DelTagApp);
		}
		else
		{
			break;
		}
	}
}

#ifdef LOCALOTA
/*
description :读取更新计数器，命令执行权限ADM

命令格式：   A0 CC 00/01  01/02  05	
*/
void ReadUpdateCounter(void)
{
	UWORD xdata fileAddr;

	if(P1 > 0x01)
	{
		P1P2Error();
		return;
	}

	if(P3 != 0x05)
	{
		P3Error();
		return;
	}

	if(ValueBit(G_AdmStatus,1) == FALSE)
	{
		SecurityError();											//send 98 04
		return;
	}

	fileAddr = SelectLFFile(DF_OTA, EF_CNTRFile);

	if(fileAddr == FALSE)
	{
		FileNotFound();
		return ;
	}

	if(P1 == 0x00)  //Read Counter P2 = 01 Counter A ;P2 = 02 Counter B
	{
		ReadLFBuf(fileAddr, P2, 0x00, (BYTE *)&RecData[0x00], P3);

		SendData();
	}
	else           //Update Counter P2 = 01 Counter A;P2 = 02 ,Counter B
	{
		ReceiveData();

		WriteLFBuf(fileAddr,P2,0x00,(BYTE *)&RecData[0],P3); 
	}

	CommandFinish(0x00);
}
/*
description :修改OTA注册状态，命令执行权限ADM

命令格式：   A0 CC 01 03 01
*/
void UpdateRegister(void)
{
	UWORD xdata fileAddr;

	if(P1 != 0x01)
	{
		P1P2Error();
		return;
	}

	if(P3 != 0x01)
	{
		P3Error();
		return;
	}

	if(ValueBit(G_AdmStatus,1) == FALSE)
	{
		SecurityError();											//send 98 04
		return;
	}

	fileAddr = SelectTFFile(DF_OTA, EF_OTAConfig);

	if(fileAddr == FALSE)
	{
		FileNotFound();
		return ;
	}

	ReceiveData();

	if(RecData[0] == 0x00)  //未注册
	{
		SetTFConst(fileAddr,0x00,0x00,sizeof(BYTE)); 	
	}
	else if(RecData[0] == 0x01) //已注册
	{
		SetTFConst(fileAddr,0x00,0x80,sizeof(BYTE));
	}
	else	//for baihe-test
	{;}
	CommandFinish(0x00);
}
/*
descriprion :读取OTA参数,命令执行权限ADM

命令格式 :   A0  CC 00   04 08
*/
void ReadOTAPara(void)
{ 
	UWORD xdata fileAddr;
	WORD xdata freeSize;

	if(P1 != 0x00)
	{
		P1P2Error();
		return;
	}

	if(P3 != 0x08)
	{
		P3Error();
		return;
	}

	if(ValueBit(G_AdmStatus,1) == FALSE)
	{
		SecurityError();											//send 98 04
		return;
	}

	fileAddr = SelectTFFile(DF_OTA, EF_OTAConfig);

	if(fileAddr == FALSE)
	{
		FileNotFound();
		return ;
	}

	G_AppFileAddr = SelectTFFile(DF_OTA, EF_OTAScript);

	freeSize = GetFreeAppSpaceSize();    //卡片剩余空间

	RecData[0] = 0x00;

	RecData[1] = (freeSize >> 0x08) & 0x00FF;

	RecData[2] = freeSize & 0x00FF;

	ReadTFBuf(fileAddr,0x01,&RecData[3],sizeof(BYTE));   //协议版本号

	ReadTFBuf(fileAddr,0x03,&RecData[4],sizeof(BYTE));   //批次号

	ReadTFBuf(fileAddr,0x02,&RecData[5],sizeof(BYTE));   //功能标识

	ReadTFBuf(fileAddr, 0x0B, &RecData[6],0x02);         //COS版本号

	SendData();

	CommandFinish(0x00);
}


/*
description  :  OTA企业化接口指令
*/
void ReadUpdateLocalJSPara(void)
{
	if((P2 == 0x01)||(P2 == 0x02))
	{
		ReadUpdateCounter();
	}
	else if(P2 == 0x03)
	{
		UpdateRegister();
	}
	else if(P2 == 0x04)
	{
		ReadOTAPara();
	}
	else
	{
		P1P2Error();
		return;
	}
}
#endif

#endif
