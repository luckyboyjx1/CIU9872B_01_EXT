/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.1

 * MODEL: 	All model

 * RELEASE:	2008-5-13

 * AUTHER:	徐庆盛

 * HISTORY: V1.0 创建源文件

			V1.1 修改函数OTASaveToEditBuf()，增加SEND SMS时，将TERMINAL_RESPONSE返回的结果(TLV_RESULT)存入指定的EDIT_BUF中
		
			V1.2 修改函数OTA3UpdateRecord，修改当更新记录时，也更新记录指针,且可以部分更新记录
				 修改函数OTACopyVar，修改将目标索引的范围由0x10扩大到0x20;

			V1.3 增加ATMEL宏定义

			V1.4 修改函数OTA3EraseFile，增加擦除BER-TLV结构的文件

			V1.5 修改函数OTADisplayVar,修改当变量数据格式编码为08时，卡片拼装数据错误的问题
			V1.6 修改OTA3UpdateRecord，修改判断文件类型时，判断条件出错
			V1.7 修改函数OTA3LinkFile，修改在查找源文件EF时的错误
			V1.8 修改函数ComputerPluginRecord，修改对于循环文件的特殊操作 @20110722 by xuyz
			
			v1.9 修改函数OTA3Select，单独处理MF及7FF0的选择@20140521 by XUYZ V1.27 ->V1.28
			
			V1.10 修改函数 OTA3Select，当fileName 为7FF0时，将其修改为G_CurrentAdfName[G_ChannelNum] @20140619 by xuyz V1.28->1.29
*/

#include "string.h"
#include "fileSystem.h"
#include "fileAction.h"
#include "ceph.h"
#include "stkhead.h"
#include "OTA3Head.h"
#include "gsmpublic.h"
#include "arithmatic.h"
#include "resume.h"
#include "interface.h"
#include "error.h"
#include "Transaction.h"


#ifdef OTA

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAUpdateVariable

  input parameter:   file: 应用文件地址
					 offset: 当前执行的AID偏移
					 id: Variable index
					 srclen:
					 src:

  output parameter:	 TRUE/FALSE

  description:       更新OTA空间分配变量

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE OTAUpdateVariable(BYTE idName, BYTE * src)
{
	WORD idata pos;
	BYTE idata tmp;

	pos = OTALocateVarConstPos(idName,0xE0);

	if( pos == FALSE )
	{
		return FALSE;
	}

	ReadTFBuf(G_AppFileAddr, G_STKAppOffset+pos, &tmp, 1);

	pos++;

	if (tmp < (src[0]+1))
	{
		src[0] = tmp -1;
	}

	WriteTFBuf(G_AppFileAddr, G_STKAppOffset+pos, &src[0],src[0]+1);

	return TRUE;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAPAGEDISPADN

  input parameter:   void

  output parameter:  void

  description:       Display all record in EF_adn, and store the selection into OTA2SelectedADN

  Tester:			 xuzhichu,Caibo

  status:			 OK

 -------------------------------------------------------------------------------------------------------------------*/
void OTAPageDispAdn(void)
{
	BYTE idata result;
	BYTE xdata pages[33];

	pages[0]= 1 ;

	result = OTAPageListInformation(TYPE_Adn_List, &pages[0], 8, NULL);

	if(result == BackMove )
	{
		G_HandleResponse = 4;						//goto BackMove Finish
	}
	else if(result == ExitStkPerform)
	{
		G_HandleResponse = 3;						//exit STK
	}
	else	//for baihe-test
	{;}
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAAdnToAscii

  input parameter:   void

  output parameter:  void

  description:       Convent ADN to ascii

  Tester:			 xuzhichu,Caibo

  status:			 OK

 -------------------------------------------------------------------------------------------------------------------*/
void OTAAdnToAscii(void)
{
	BYTE  idata reclgth;
	UWORD xdata fileAddr;

	fileAddr = SelectLFFile(DF_TELECOM,EF_ADN);

	reclgth = GetLFRecNumLgth(fileAddr,FileLgth);

	ReadLFBuf(fileAddr, G_SelectedNum, reclgth - 14, &ResData[0], 14);

	ADNToAscii(&ResData[0],&RecData[0]);

	ReadEeprom(&reclgth,G_ByteCodeAddr+3,sizeof(BYTE));

	WriteVariable(reclgth, &RecData[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAAsciiToAdn

  input parameter:   void

  output parameter:  void

  description:       ascii to adn

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAAsciiToAdn(void)
{
	ReadEeprom(tempBuf, G_ByteCodeAddr+1, 5);

	if(tempBuf[2] == 0x00)	 // application variable
	{
		RecData[0] = OTAGetVariable(tempBuf[4], &RecData[1]);

		if(RecData[0] == 0)
		{
			return ;
		}
	}
	else
	{
		RecData[0] = tempBuf[0] - 3;

		ReadEeprom(&RecData[1],G_ByteCodeAddr+5,RecData[0]);
	}

	AsciiToADN(&RecData[0], &ResData[0]);

	WriteVariable(tempBuf[3], &ResData[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTALocationInforToAscii

  input parameter:   void

  output parameter:  void

  description:       Convent LOCATION INFO to ascii

  Tester:			 xuzhichu,Caibo

  status:			 OK

 -------------------------------------------------------------------------------------------------------------------*/
void OTALocationInforToAscii(void)
{
	ReadEeprom(tempBuf, G_ByteCodeAddr+1, 4);		//tempBuf[2] src, tempBuf[3] dst

	RecData[0] = ReadVarible(tempBuf[2], &RecData[1]);

	IntToString(&RecData[0], &ResData[0]);		//Hex to ASCII

	WriteVariable(tempBuf[3], &ResData[0]);	
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTASetpositionService

  input parameter:   void

  output parameter:  void

  description:       set Positon service config func

  Tester:			 xuzhichu,Caibo

  status:            OK

 -------------------------------------------------------------------------------------------------------------------*/
void OTASetpositionService(void)
{
	BYTE  idata ptr;
	BYTE  idata result,i,num,lgth;
	UWORD xdata fileAddr;

	fileAddr = SelectTFFile(DF_OTA, EF_LocalInfoConfig);

	if (fileAddr == FALSE)
	{
		return ;
	}

	while (1)
	{
		ReadTFBuf(fileAddr, 0, &ptr, 1);

		for(num = 0x00,lgth = 0x00,i=0x01;i<0x04; i = i+1)
		{
			memcpy_G(&tempBuf[lgth],(BYTE CONST *)&userSelect3[num],userSelect3[num]+1);

		   	if(ptr == i)
		   	{
			   	AddVlaue(&tempBuf[lgth], (BYTE CONST *)userSelectDefault);

			   	lgth = lgth + userSelectDefault[0];
		   	}

			lgth = lgth + userSelect3[num]+1;

		   	num =  num + userSelect3[num] +1;
		}

		result = cephSelectItemSingle(0x00,UCS2,NULL, (BYTE CONST *)&tempBuf[0],lgth);

		if (result != CMD_Success)
		{
			G_HandleResponse = 1;

			return;
		}

		result = GetResponseItemIdentifier();

		SetTFConst(fileAddr, 0, result, 1);
	}
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTASaveToEditBuf

  input parameter:   void

  output parameter:  void

  description:       Save to Edit Buffer function

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
extern BYTE code tagArray[6];
void OTASaveToEditBuf(void)
{
	BYTE idata ptr;

	ReadEeprom(tempBuf, G_ByteCodeAddr+1, 4);	//tempBuf[0] len, tempBuf[2] type, tempBuf[3] dst

	if(tempBuf[2] == 0x00)
	{
		switch(RecData[3])
		{
			case CMD_GETINPUT:
			case CMD_SENDUSSD:
			{
				ptr = FindTag(&RecData[0], TLV_TEXTSTRING, P3);

				if(ptr != FALSE)
				{
					ptr++;

					RecData[ptr] = RecData[ptr-1] - 1;
				}

				break;
			}
			case CMD_SELECTITEM:
				ptr = FindTag(&RecData[0],TLV_ITEMIDENTIFIER,P3);

				break;
			case CMD_PROVIDELOCALINFORMATION:
			{
				if (RecData[4] < (sizeof(tagArray)/sizeof(BYTE)))
				{
					ptr = FindTag(&RecData[0], tagArray[RecData[4]], P3);
				}
				else
				{
					return;
				}

				break;
			}
			default:
			{
				ptr = FindTag(&RecData[0],TLV_RESULT,P3);

				break;
			}
		}
	}
	else
	{
		if(tempBuf[0] == 0x03)
		{
			ClearVariable(tempBuf[3]);

			return;
		}
		else
		{
			RecData[0] = tempBuf[0] - 3;

			ReadEeprom(&RecData[1], G_ByteCodeAddr+5, RecData[0]);

			ptr = 0x00;
		}
	}

	WriteVariable(tempBuf[3], &RecData[ptr]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTADisPlayVar

  input parameter:   void

  output parameter:  void

  description:       Display Variable

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/

void OTADisPlayVar(void)
{
	BYTE idata i,result,dcs;
	WORD idata num;
	BYTE xdata tmp[4];

	ReadEeprom(&tmp[0],G_ByteCodeAddr+3,sizeof(tmp));

	if((tmp[0] < 0x10)^(tmp[1] < 0x10))			//应用工具已保证类型合法，此处仅判断类型是否一致，2008-03-02
	{
		goto L_EXIT_OTA ;
	}

	for(i = tmp[0],result = 0,num = 0 ; i <= tmp[1] ; i = i + tmp[2])	//tmp[1] as des, tmp[2] as step
	{
		RecData[0] = OTAGetVariable(i,&RecData[1]);

		if(RecData[0] > 0)
		{
			if((RecData[1] == 0x04) || (RecData[1] == 0x08))
			{
				if(RecData[1] == 0x04)
				{
					dcs = 0x04;
				}
				else
				{
					dcs = 0x80;
				}

				RecData[1] = RecData[0] -1;

				memcpy(&tempBuf[num], &RecData[1], RecData[0]);
			}
			else
			{
				continue;
			}

			num = num + tempBuf[num] + 1;

			if(num > 0xFF)						//2008-03-02,ITEM项出界
			{
				goto L_EXIT_OTA ;
			}

			result ++;
		}
	}

	if(result == 0)
	{
		cephDisplayTextSingle(0x00,UCS2,(BYTE CONST *)liebiaokong);		//列表为空

		G_HandleResponse = 4;						//goto BackMove Finish

		return ;
	}
	else
	{
		result = cephSelectItemSingle(0x00, dcs, NULL, (BYTE CONST *)&tempBuf[0], num);

		if(result != CMD_Success)
		{

L_EXIT_OTA :

			G_HandleResponse = 1;					//回退,2008-03-01

			return;
		}

		result = GetResponseItemIdentifier();

		RecData[1] = (result-1) * tmp[2] + tmp[0];	 		//variable index

		RecData[0] = 0x01;

		WriteVariable(tmp[3], &RecData[0]);				//tmp[3] as varId
	}
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTACopyVar

  input parameter:   void

  output parameter:  void

  description:       Copy Variable

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTACopyVar(void)
{
	BYTE idata tindex;
	BYTE xdata num[3];

	ReadEeprom(&num[0],G_ByteCodeAddr+3,sizeof(num));

	if ((num[0]&0x02) == 0x02)				//num[0] as num3
	{
		OTAGetVariable(num[2], &tindex);	//num[2] as num5 
	}
	else
	{
		tindex = num[2];
	}
	RecData[0]= OTAGetVariable(tindex, &RecData[1]);

	if ((num[0]&0x01) == 0x01)				//num[0] as num3
	{
		OTAGetVariable(num[1], &tindex);	//num[1] as num4
	}
	else
	{
		tindex = num[1];
	}

	if (tindex < 0x20)
	{
		WriteVariable(tindex,&RecData[0]);
	}
	else if (tindex >0xDF)
	{
		OTAUpdateVariable(tindex,&RecData[0]);
	}
	else	//for baihe-test
	{;}
}
/*
*@Description:把函数参数3的内容填充到函数参数1指定的EDIT_BUF中的内容的前面left(后面right)再存到函数参数2指定的EDIT_BUF
*
*@param      :tag:OTA2_EX_RIGHTEXPAND/OTA2_EX_RIGHTEXPAND
*
*@return     :void
*
*@history    :create by xyz@100622 15:22
*/
void OTATextPand(BYTE tag)
{
	ReadEeprom(tempBuf, G_ByteCodeAddr+1, 4); 		//tempBuf[2] src, tempBuf[3] dst

	tempBuf[10] = tempBuf[0] - 3;	//获取参数3中的数据长度	pad data len

	ReadEeprom(&tempBuf[11], G_ByteCodeAddr+5, tempBuf[10]);	//获取参数3中的实际数据 pad data

	if(tag == OTA2_EX_LEFTTEXPAND)  //left expand
	{
		memcpy(&RecData[0], &tempBuf[10], tempBuf[10]+1);

		RecData[0] += OTAGetVariable(tempBuf[2], &RecData[tempBuf[10]+1]);
	}
	else		//right expand
	{
		RecData[0] = OTAGetVariable(tempBuf[2], &RecData[1]);

		AddVlaue(&RecData[0], &tempBuf[10]);
	}

	WriteVariable(tempBuf[3], &RecData[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTASubstring

  input parameter:  void

  output parameter:  void

  description:       Sub-String

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTASubString(void)
{
	BYTE idata r[2];

	ReadEeprom(tempBuf, G_ByteCodeAddr+1, 7);	//tempBuf[2] 起始偏移, tempBuf[3] 截取长度, tempBuf[4] src, tempBuf[5] dst, tempBuf[6] 返回状态

	RecData[0] = OTAGetVariable(tempBuf[4], &RecData[1]);	//源字符串

	r[0] = 1;

	if(tempBuf[2] > RecData[0])
	{
		r[1] = 2;

		RecData[0] = 0;
	}
	else if((tempBuf[2] + tempBuf[3]) > RecData[0])
	{
		r[1] = 1;

		RecData[0] -= tempBuf[2];

		memmove(&RecData[1], &RecData[tempBuf[2]+1], RecData[0]);
	}
	else
	{
		r[1] = 0;

		RecData[0] = tempBuf[3];

		memmove(&RecData[1], &RecData[tempBuf[2]+1], RecData[0]);
	}	

	WriteVariable(tempBuf[5], &RecData[0]);

	WriteVariable(tempBuf[6], &r[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTADeleteVarContent

  input parameter:   void

  output parameter:  void

  description:       Delete Variable content

  test:				 zhuli

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTADeleteVarContent(void)
{
	BYTE idata tag;

	ReadEeprom(&tag,G_ByteCodeAddr+3,sizeof(BYTE));

	RecData[0] = ReadVarible(tag,&RecData[1]);

	if(RecData[0] != 0x00)
	{
		tag = 0x00;	  

		tag = OTAUpdateVariable(RecData[1], (BYTE *)&tag);
	}

	if(tag == FALSE)
	{
		G_HandleResponse = 0x03;
	}
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAFindNullVar

  input parameter:   void

  output parameter:  void

  description:       Found empty Variable

  test:				 zhuli

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAFindNullVar(void)
{
	BYTE xdata tmp[5];

	ReadEeprom(&tmp[1],G_ByteCodeAddr+3,4);

	for(;tmp[1] <= tmp[2]; tmp[1] += tmp[3])
	{
		tmp[0] = OTASeekConstVar(tmp[1],NULL,0xE0);

		if(tmp[0] == FALSE)
		{
			tmp[0] = 1;

			WriteVariable(tmp[4], &tmp[0]);

			return;
		}
	}

	cephDisplayTextSingle(0x00,UCS2,(BYTE CONST *)noFitNum);			//无比配记录

	G_HandleResponse = 2;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAStringCompare

  input parameter:   void

  output parameter:  void

  description:       String Compare

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAStringCompare(void)
{
	BYTE idata r[2];
	BYTE idata i;

	ReadEeprom(tempBuf, G_ByteCodeAddr+1, 6);	//tempBuf[2] 起始偏移, tempBuf[3] 结束偏移, tempBuf[4] src, tempBuf[5] dst

	if((tempBuf[0] < 6) || (((tempBuf[0] - 0x05) &0x01) != 0))
	{
		G_HandleResponse = 0x02;

		return;
	}

	RecData[0] = ReadVarible(tempBuf[4],&RecData[1]);		//src

	if((tempBuf[2] >= RecData[0]) ||(RecData[0] == 0x00)||(tempBuf[2] > tempBuf[3] ))		
	{
		G_HandleResponse = 0x02;

		return;
	}

	tempBuf[3] = ((tempBuf[3] >= RecData[0]) ? (RecData[0] - 0x01) : tempBuf[3]);

	r[0] = 1;

	r[1] = 0;

	tempBuf[10] = tempBuf[0] - 5;		//比较参数长度

	ReadEeprom(&tempBuf[11], G_ByteCodeAddr+7, tempBuf[10]);	//比较参数

	for(i=11; (tempBuf[2]<=tempBuf[3])&&(i<=tempBuf[10]+10); i+=2)
	{
		if((RecData[tempBuf[2]+1] < tempBuf[i]) || (RecData[tempBuf[2]+1] > tempBuf[i+1]))
		{
			r[1] = 1;

			break;
		}

		tempBuf[2]++;
	}

	WriteVariable(tempBuf[5], &r[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAAsciiToUcs2

  input parameter:   void

  output parameter:  void

  description:       ascii to UCS2

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAAsciiToUcs2(void)
{
	BYTE idata tmp[2];

	ReadEeprom(&tmp[0],G_ByteCodeAddr+3,sizeof(tmp));

	RecData[0] = ReadVarible(tmp[0], &RecData[1]);

	if (RecData[1] != UCS2)
	{
		AsciiToUCS2(&RecData[0], &ResData[0]);
	}
	else
	{
		RecData[0] --;

		memcpy(&ResData[0],&RecData[1], RecData[0] - 2);
	}

	WriteVariable(tmp[1], &ResData[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAReplaceChar

  input parameter:   void

  output parameter:  void

  description:       replace char

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAReplaceChar(void)
{
	BYTE idata i;

	ReadEeprom(&tempBuf[0],G_ByteCodeAddr+3,0x03); //tempBuf[0] as buf索引 tempBuf[1] as 函数参数2，tempBuf[2]as 函数参数3

	RecData[0] = ReadVarible(tempBuf[0], &RecData[1]);			//tempBuf[0] as varId

	for (i = 1; i <= RecData[0]; i ++)
	{
		if (tempBuf[1] == RecData[i])					//tempBuf[1] as ch1
		{
			RecData[i] = tempBuf[2];					//tempBuf[2] as ch2
		}
	}

	WriteVariable(tempBuf[0] , &RecData[0]);					//tempBuf[0] as varId
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAJmpFormEditbufO,TAJmpZero

  input parameter:   void

  output parameter:  void

  description:       Jump to edit buffer specific command

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/

void OTAJmpZeroEditBuf(BYTE cmdType)
{
	BYTE idata jmpTag;
	BYTE idata status;
	
	status = 0x00;

	if(cmdType  ==  OTA2_EX_JMPFROMEDITBUF)
	{
		status = 0x01;	
	}
	
	jmpTag = 0x00;
		
	ReadEeprom(tempBuf, G_ByteCodeAddr+1, 7+status);
	
	RecData[0] = ReadVarible(tempBuf[2+status], &RecData[1]);
	
	if(cmdType  ==  OTA2_EX_JMPFROMEDITBUF)
	{
		if((RecData[1] == tempBuf[2]) && (RecData[0] != 0x00))	   // 当参数1中的数据和参数2 EditBuf中的数据相等时，根据PID和命令序号跳转
		{
			jmpTag = 0x01;
		}
	}
	else	                     //变量为空时，跳转到参数2指定的过程命令
	{
		if ((RecData[0] == 0)||(RecData[0] == 0xFF))
		{
			jmpTag = 0x01;
		}
	}
	
	if(jmpTag == 0x01)
	{
		memcpy(&G_Pid[0], &tempBuf[3+status], 3);

		G_CmdIndex = tempBuf[6 + status];
	}
	else
	{
		G_CmdIndex ++;
	}
	
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTASubAddOperation

  input parameter:   command

  output parameter:  void

  description:       numeric sub or add operation

  test:				 new

  status:

 -------------------------------------------------------------------------------------------------------------------*/

void OTASubAddOperation(BYTE command)
{
	WORD idata r;
	BYTE idata i;
	BYTE idata d[2];
	BYTE xdata tmp[4];

	ReadEeprom(&tmp[0],G_ByteCodeAddr+3,sizeof(tmp));

	i = ReadVarible(tmp[0], &RecData[1]);		//tmp[0] as index

	RecData[0] = i;

	if((i == 0)||(i == 0xFF))
	{
		return ;
	}

	for(r = (tmp[3]&0xFF); i > 0; i --)            //tmp[3] as data length
	{
		if(r == 0)
		{
			break;
		}

        if(command == OTA2_EX_SUBOPERATION)        	//mode is -
        {
			RecData[i] = RecData[i] - r;

    		if(r > RecData[i])
    		{
    			r = 1;
    		}
    		else
    		{
    			r = 0;
    		}
        }
        else                            		//mode is +
        {
            r = RecData[i] + r;

            RecData[i] = (r&0xFF);

            r = ((r >> 8)&0xFF);
        }
	}

	d[0] = 1;

	d[1] = r&0xFF;

	WriteVariable(tmp[2], &d[0]);			//tmp[2] as BorrowBit varId

	WriteVariable(tmp[1], &RecData[0]);//tmp[1] as Result varId
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAModOperation

  input parameter:   void

  output parameter:  void

  description:       numeric mod operation

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTAModOperation(void)
{
	WORD xdata value;
	BYTE xdata tmp[3];

	ReadEeprom(&tmp[0],G_ByteCodeAddr+3,sizeof(tmp));

	if(tmp[2] == 0x00)
	{
		return ;
	}

	RecData[0] = ReadVarible(tmp[0], &RecData[1]);			//tmp[0] as index

	if ((RecData[0] == 0) || (RecData[0] == 0xFF))
	{
		return ;
	}

	if (RecData[0] == 1)
	{
		value = RecData[1];
	}
	else
	{
		value = ((RecData[1] << 8)&0xFF00) + RecData[2];
	}

	value = value % tmp[2];

	RecData[0] = 1;

	RecData[1] = (value&0xFF);

	WriteVariable(tmp[1], &RecData[0]);		//tmp[1] as varId
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTASendTab

  input parameter:   void

  output parameter:  void

  description:       send tab command

  test:				 xuzhichu

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTASendTab(void)
{
	BYTE idata index;
	BYTE idata itemTag[2];

	index = GetResponseItemIdentifier();				//index as item identify

	ReadEeprom(&itemTag[0],G_ByteCodeAddr+3,sizeof(itemTag));

	index = itemTag[1] + index - 1;					//itemTag[1] as tag

	ResData[0] = OTAGetVariable(index, &ResData[1]);

	WriteVariable(itemTag[0],&ResData[0]);				//itemTag[0] as item in byte code
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :		array compare (tool class)

  input parameter:  		void

  output parameter:	 	void

  description:       		array compariation (specification didn't mention that how to handle if the compare length larger than the (source/dest length - shift))

  test:				DK Zhuli

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3ArrayCompare(void)
{
	BYTE 			idata srcOffset, desOffset;
	signed char 	idata result;

	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 0x08);

	tempBuf[10] = OTAGetVariable(tempBuf[2], &RecData[0]);		//src

	OTAGetVariable(tempBuf[3], &srcOffset);		//src offset

	tempBuf[11] = OTAGetVariable(tempBuf[4], &ResData[0]);		//dst

	OTAGetVariable(tempBuf[5], &desOffset);		//dst offset

	if((srcOffset>tempBuf[10])||(desOffset>tempBuf[11])||(tempBuf[6]>tempBuf[10])||(tempBuf[6]>tempBuf[11])||((srcOffset+tempBuf[6])>tempBuf[10])||((desOffset+tempBuf[6])>tempBuf[11]))
	{
		result = 0x03;
	}
	else
	{
		result = memcmp_p(&RecData[srcOffset], &ResData[desOffset], tempBuf[6]);

		if(result > 0)
		{
			result = 0x02;
		}
		else if(result < 0)
		{
			result = 0x00;
		}
		else
		{
			result = 0x01;
		}
	}

	tempBuf[0] = 0x01;

	tempBuf[1] = result;

	WriteVariable(tempBuf[7], &tempBuf[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		array fill (tool class)

  input parameter:  		void

  output parameter:	 	void

  description:       		fill array

  test:				DK zhuli

  status:

 -------------------------------------------------------------------------------------------------------------------*/

void OTA3ArrayFill(void)
{
	BYTE idata dat, offset, len;

	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 6);

	OTAGetVariable(tempBuf[3], &offset); // shift

	OTAGetVariable(tempBuf[4], &len); //fill data len

	OTAGetVariable(tempBuf[5], &dat); // fill data

	RecData[0] = OTAGetVariable(tempBuf[2], &RecData[1]);

	memset(&RecData[offset+1], dat, len);

	if ((RecData[0] - offset + 1) < len)
	{
		RecData[0] = len +  offset -1;
	}
	WriteVariable(tempBuf[2], &RecData[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Get AID  (tool class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Get AID

  test:				DK zhuli

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3GetAID(void)
{
	
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+3, sizeof(BYTE));

	tempBuf[1] = 0x03;

	ReadTFBuf(G_AppFileAddr, G_STKAppOffset, &tempBuf[2], 3);

	WriteVariable(tempBuf[0], &tempBuf[1]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		String Merge  (tool class)

  input parameter:  		void

  output parameter:	 	void

  description:       		String Merge

  test:				DK zhuli

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3StringMerge(void)
{
	BYTE idata i;

	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, sizeof(BYTE));

	ReadEeprom(&tempBuf[1], G_ByteCodeAddr+2, tempBuf[0]);

	RecData[0] = 0x00;

	for(i=0; i<=(tempBuf[0]-3); i++)
	{
		RecData[0] += OTAGetVariable(tempBuf[3+i], &RecData[RecData[0]+1]);
	}

	WriteVariable(tempBuf[2], &RecData[0]);

	return ;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Bit Operation  (tool class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Bit Operation

  test:				DK zhuli

  status:

 -------------------------------------------------------------------------------------------------------------------*/

void OTA3BitOperation(void)
{
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 6);

	if(tempBuf[0] != 0x05)
	{
		return ;
	}

	ResData[0] = OTAGetVariable(tempBuf[3], &ResData[1]);		//被操作数

	RecData[0] = OTAGetVariable(tempBuf[4], &RecData[1]);		//操作数

	if((ResData[0] == 0x00) || (RecData[0] == 0x00))
	{
		ClearVariable(tempBuf[5]);

		return ;
	}

	tempBuf[0] = ((ResData[0]>=RecData[0]) ?  RecData[0] : ResData[0]); // compare length equal to the shorter one byte array !

	for(tempBuf[1]=1; tempBuf[1]<=tempBuf[0]; tempBuf[1] ++)
	{
		switch(tempBuf[2])
		{
			case 0x01:
				ResData[tempBuf[1]] = ResData[tempBuf[1]] & RecData[tempBuf[1]];
				break;
 			case 0x02:
				ResData[tempBuf[1]] = ResData[tempBuf[1]] | RecData[tempBuf[1]];
				break;
			case 0x03:
				ResData[tempBuf[1]] = ResData[tempBuf[1]] ^ RecData[tempBuf[1]];
				break;
			default:
				break;
		}
	}

	ResData[0] = tempBuf[0];

	WriteVariable(tempBuf[5], ResData);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Init Proactive Command  (SAT / USAT class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Init Proactive Command

  test:				DK

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3InitProactiveCommand(void)
{
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 5);

	if(tempBuf[0] != 0x04)
	{
		return ;
	}

	InitProactiveCommand(tempBuf[2], tempBuf[3], tempBuf[4]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Append Array  (SAT / USAT class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Append one string start from one shift to the end of the proactive command , additional data is indepent with previous data(OTA3 specification).

  test:				DK zhuli

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3AppendArray(void)
{
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 5);

	if(tempBuf[0] != 0x04)
	{
		return ;
	}

	RecData[0] = OTAGetVariable(tempBuf[2], &RecData[1]);	//src

	if(OTAGetVariable(tempBuf[3], &tempBuf[10]) != 0x01)
	{
		return;
	}

	if(OTAGetVariable(tempBuf[4], &tempBuf[11]) != 0x01)
	{
		return;
	}

	if((tempBuf[11]+tempBuf[10]) > RecData[0])
	{
		return;
	}

	RecData[tempBuf[10]] = tempBuf[11];

	AddVlaue(&ResData[2], (BYTE CONST *)&RecData[tempBuf[10]]);

	return ;

}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Append TLV  (SAT / USAT class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Append a TLV data to the current proactive command TLV list, the additional data should be a complete TLV format data.(OTA3 specification)

  test:				DK zhuli

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3AppendTLV(void)
{
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 6);

	if(tempBuf[0] != 0x05) // checked error in command list
	{
		return ;
	}

	RecData[0] = OTAGetVariable(tempBuf[3], &RecData[1]);		//src

	if(OTAGetVariable(tempBuf[4], &tempBuf[4]) != 0x01) // tempbuf[4] is the shift
	{
		return ;
	}

	if(OTAGetVariable(tempBuf[5],&tempBuf[11]) != 0x01)
	{
		return ;
	}

	if((tempBuf[4] > RecData[0])||((tempBuf[4]+tempBuf[11]) > RecData[0]))
	{
		return;
	}

	RecData[0] = tempBuf[2] | 0x80;

	memmove(&RecData[2], &RecData[tempBuf[4]+1], tempBuf[11]);

	RecData[1] = tempBuf[11];

	AddTLV(&ResData[1], (BYTE CONST *)&RecData[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Check Profile  (SAT / USAT class)

  input parameter:  		void

  output parameter:	 	void

  description:       		check ME's terminal profile have the assigned data (OTA3 Specification)

  test:				DK

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3CheckProfile(void)
{
	BYTE idata i;

	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 4);

	if(tempBuf[0] != 0x03) //  checked error in command list
	{
		return ;
	}

	RecData[0] = OTAGetVariable(tempBuf[2], &RecData[1]);

	tempBuf[0] = 0x01;

	tempBuf[1] = 0x01;

	if(RecData[0] > G_TerminalProfile[0])
	{
		tempBuf[1] = 0x00;

		WriteVariable(tempBuf[3], &tempBuf[0]);

		return ;
	}

	for(i=1; i<= RecData[0]; i++)
	{
		if((G_TerminalProfile[i]&RecData[i]) != RecData[i])
		{
			tempBuf[1] = 0x00;

			break;
		}
	}
	WriteVariable(tempBuf[3], &tempBuf[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Copy Value By Tag  (SAT / USAT class)

  input parameter:  		void

  output parameter:	 	void

  description:       		copy the specifiy tag 's data form the current terminal response to the des string, if tag not found, set the des string to NULL (OTA3 Specification)

  test:				DK zhuli

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3CopyValueByTag(void)
{
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 4);

	if(tempBuf[0] != 0x03) //  checked error in command list
	{
		return ;
	}

	tempBuf[0] = FindTag(RecData, tempBuf[2], P3);

	if(tempBuf[0] == FALSE)
	{
		ClearVariable(tempBuf[3]);
	}
	else
	{
		WriteVariable(tempBuf[3], &RecData[tempBuf[0]]);
	}
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Get General Result  (SAT / USAT class)

  input parameter:  		void

  output parameter:	 	void

  description:       		return the result of the proactive command (OTA3 Specification)

  test:				DK zhuli

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3GetGeneralResult(void)
{
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 3);

	if(tempBuf[0] != 0x02)
	{
		return;
	}

	tempBuf[0] = 0x01;

	tempBuf[1] = GetResponseResult();

	WriteVariable(tempBuf[2], &tempBuf[0]);
}
/*
*@description:判断OTA解释器中透明文件的权限是否满足
*
*@param      :action :A_ReadFile,A_UpdateFile
*
*@returns   :TRUE权限满足/FALSE：权限不满足
*
*@history   :create by xyz@100608 16:34
*/
BYTE ComputePluginBinary(BYTE action)
{
	BYTE idata tmp;
	UWORD xdata CurrentEfHeadAddr;
	
	if( G_CurrentEfAddr[G_ChannelNum] == FALSE )
	{
		return FALSE;
	}
	
	CurrentEfHeadAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);//若当前文件是镜像文件，则查找实际镜像文件的地址

	tmp = AccessDefined(CurrentEfHeadAddr,action);

	tmp = IF_AccessDefined(tmp,G_CurrentEfName[G_ChannelNum]);

	if((tmp !=0x90 )&& (tmp !=0x01)) //只能read具有ALW/PIN权限的文件
	{
		return FALSE;
	}


	tmp = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum],F_Type_Offset);

	if(tmp != TRANTYPE)
	{
		return FALSE;
	}

	tmp = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum],F_Status_Offset);

	if((tmp&0x01) == FILE_INVALIDATE)
	{
		return FALSE;
	}

	return TRUE;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Read Binary  (File Operation class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Read Current Binary File (OTA3 Specification 8.2.3 for detail description)

  test:				DK

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3ReadBinary(void)
{
	WORD idata offset;
	BYTE idata len;
	BYTE xdata cmd[4];

	ReadEeprom(&cmd[0], G_ByteCodeAddr+2, 4);

	len = OTAGetVariable(cmd[1], (BYTE *)&offset);

#ifdef LittleEndian
    if (len == 0x01)
	{
		offset = (offset &0x00FF);
	}
#else
	if (len == 0x01)
	{
		offset = (offset >> 0x08) & 0x00FF;
	}
#endif
	OTAGetVariable(cmd[3], &len);

 	if(len == 0x00)
	{
		ClearVariable(cmd[2]);
		return;
	}

 	if(ComputePluginBinary(A_ReadFile) == FALSE)
	{
		ClearVariable(cmd[2]);
		return;	
	}

	ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],offset,&RecData[0x01],len);

	RecData[0] = len;

	WriteVariable(cmd[2], RecData);

	return ;
}
/*
*@description:计算解释器函数中update renord ,read record的权限判断
*
*@param       :acrtion:A_ReadFile A_UpdateFile
*@param       :mode   :模式，参考GSM11.11
*@param       :tmp:记录号引用
*
*@returns   :TRUE/FALSE
*
*@history   :create by xyz @100607 16:30
*/
BYTE ComputerPluginRecord(BYTE action,BYTE mode,BYTE tmp)
{
	BYTE idata i, totalRecNum;
	BYTE idata currentRec;
	UWORD xdata CurrentEfHeadAddr;
	
	if((mode< 2)||(mode> 4)||(G_CurrentEfAddr[G_ChannelNum] == FALSE))
	{
		return FALSE;
	}

	CurrentEfHeadAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);//若当前文件是镜像文件，则查找实际镜像文件的地址

	i = AccessDefined(CurrentEfHeadAddr,action);

	i = IF_AccessDefined(i,G_CurrentEfName[G_ChannelNum]);
	
	if((i !=0x90 )&& (i !=0x01))
	{
		return FALSE;
	}
	

	totalRecNum = GetLFRecNumLgth(G_CurrentEfAddr[G_ChannelNum],FileNum);

	i = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum],F_Type_Offset);

	if(mode== ABSOLUTE_CURRENT)
	{
		if( i == CYCTYPE)	    //循环文件的记录个数范围为(1 totalRecNum - 1)
		{
			if(tmp > (totalRecNum-1))
			{
				return FALSE;
			}
		}
		else
		{
			if(tmp > totalRecNum)
			{
				return FALSE;
			}
		}
	}
	i = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum],F_Status_Offset);

	if((i&0x01) == FILE_INVALIDATE)
	{
		return FALSE;
	}

	i = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum],F_Type_Offset);

	if((i != LINTYPE)&&(i != CYCTYPE ))
	{
		return FALSE;
	}
	if((action == A_UpdateFile)&&(i == CYCTYPE)&&(mode != PRIVOUS_RECORD))
	{
		return FALSE;
	}

	//tmp = GetRecordNo(i,mode,tmp,action);		  
	tmp =  GetPhyNum(action,G_CurrentEfAddr[G_ChannelNum],mode,tmp,&currentRec);

	return tmp;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Read Record  (File Operation class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Read Current Record File (OTA3 Specification)

  test:				DK

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3ReadRecord(void)
{
	BYTE idata i;
	BYTE xdata cmd[7],tmp[3];

	ReadEeprom(&cmd[0], G_ByteCodeAddr+1, sizeof(cmd));

	if(cmd[0] != 0x06)
	{
		return;
	}

	for(i=1; i<4; i++)
	{
		OTAGetVariable(cmd[i*2], &tmp[i-1]);
	}

	if(tmp[2] == 0x00)
	{
		ClearVariable(cmd[5]);

		return;
	}

	tmp[0] =  ComputerPluginRecord(A_ReadFile,cmd[3],tmp[0]);

	if(tmp[0] == FALSE)
	{
		ClearVariable(cmd[5]);

		return;
	}

	if(cmd[3]!= ABSOLUTE_CURRENT)
	{
		G_CurrentRecNum[G_ChannelNum] = tmp[0];
	}
	
	ReadLFBuf(G_CurrentEfAddr[G_ChannelNum],tmp[0],tmp[1],&RecData[1],tmp[2]);	//get the co

	RecData[0] = tmp[2];

	WriteVariable(cmd[5], RecData);

	return ;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Select  (File Operation class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Select File as 11.11 (OTA3 Specification)

  test:				DK

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3Select(void)
{
	UWORD xdata fileAddr;
	WORD  idata fileName;
	BYTE  idata fileType, des, len;
	BYTE  idata num;

	if (STKControl == TRUE)
	{
		BackFilePath(&FilePathInfo);	      //备份当前路径

		STKControl = FALSE;
	}

	ReadEeprom(tempBuf, G_ByteCodeAddr + 1, 5);

	if(tempBuf[0] != 0x04)
	{
		return ;
	}

	des = tempBuf[3];

	len = tempBuf[4];

	RecData[0] = OTAGetVariable(tempBuf[2],&RecData[1]);

	if(RecData[0] == 0x02)    //按PID选择
	{
		OTAGetVariable(tempBuf[2], (unsigned char *)(&fileName));

#ifdef LittleEndian
	    fileName = (fileName<<0x08) + (fileName>>0x08);
#endif

		if((fileName == DF_MF) || (fileName == 0x7FF0))
		{
		
			if(fileName == DF_MF)
			{
				fileAddr = E_FILE_START;
			}
			else  //选择7FF0，即选择USIM ADF
			{
				fileName = G_CurrentAdfName[G_ChannelNum];
				
				if(G_CurrentAdfName[G_ChannelNum] == FALSE)  //当前并未选中ADF
				{
					fileAddr = FALSE;
				}
				else
				{
					fileAddr = SelectFile(E_FILE_START, G_CurrentAdfName[G_ChannelNum]); 
				}
			}
		}
		else
		{
			fileAddr = SelectByFileName(fileName);
		}
	}
	else                                                       //路经选择
	{
		fileAddr = E_FILE_START;
		
		for(num=0x01;num<RecData[0];num=num+2)
		{
			fileName = ((RecData[num]<<8)&0xFF00) + RecData[num+1];
			
			if((fileName == 0x7FFF))
			{
				fileName = G_CurrentAdfName[G_ChannelNum];
			}
			
			fileAddr = SelectFile(fileAddr,fileName);
			
			if(fileAddr == FALSE)
			{
			 	break;
			}
			else
			{
			  SetChannel(fileAddr,fileName,FALSE);
			}
		}
	}

	if( fileAddr == FALSE)
	{
		ClearVariable(des);

		return;
	}

	fileType = SetChannel(fileAddr,fileName,TRUE); // file type now is the length of the response data!!

	memcpy(&tempBuf[1], &RecData[0], fileType);

	tempBuf[0] = (len < fileType) ? len : fileType;    //select the len

	WriteVariable(des, tempBuf);

	return ;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Update Binary  (File Operation class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Update Current Binary File (OTA3 Specification)

  test:				DK zhuli

  status:				unfinished, untested

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3UpdateBinary(void)
{
	BYTE idata  tmp;
	WORD idata fileOffset;
	BYTE xdata cmd[7];

	ReadEeprom(cmd, G_ByteCodeAddr + 1, 7);

	if(cmd[0] != 0x06)
	{
		return;
	}

	tmp = OTAGetVariable(cmd[2],  (BYTE *)&fileOffset);
#ifdef LittleEndian
	if (tmp == 0x01)
	{
		fileOffset = (fileOffset & 0x00FF);
	}
#else
	if (tmp == 0x01)
	{
		fileOffset = (fileOffset >> 0x08) & 0x00FF;
	}
#endif
	if(cmd[5] == 0x00)
	{
		return;
	}
  	if(ComputePluginBinary(A_UpdateFile) == FALSE)
	{
		ClearVariable(cmd[6]);
		return;	
	}

	 RecData[0] = OTAGetVariable(cmd[3], &RecData[1]);

	tmp = WriteTFBuf(G_CurrentEfAddr[G_ChannelNum],fileOffset,&RecData[cmd[4] + 1],cmd[5]);

	if(tmp == TRUE)
	{
		cmd[0] = 2;

		cmd[1] = 0x90;

		cmd[2] = 0x00;

		WriteVariable(cmd[6], cmd);

		return ;
	}
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Update Record  (File Operation class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Update Current Record File (OTA3 Specification)

  test:				DK

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3UpdateRecord(void)
{
	BYTE idata recordNo, tmp,maxLength,fileType;
	BYTE idata updateShift;
	BYTE idata need_commit,tag;
	BYTE xdata cmd[9];

	ReadEeprom(cmd, G_ByteCodeAddr + 1, 9);

	if(cmd[0] != 0x08)
	{
		return;
	}

	OTAGetVariable(cmd[2], &recordNo);

	OTAGetVariable(cmd[4], &updateShift);

	recordNo = ComputerPluginRecord(A_UpdateFile,cmd[3],recordNo);

	if(recordNo == FALSE)
	{
		ClearVariable( cmd[8]);
		
		return;	
	}

	maxLength = GetLFRecNumLgth(G_CurrentEfAddr[G_ChannelNum],FileLgth);

	if((cmd[7]+ cmd[6]) > maxLength)
	{
		ClearVariable( cmd[8]);

		return;
	}			


	RecData[0] = OTAGetVariable(cmd[5], &RecData[1]);

	if(cmd[6]> RecData[0])
	{
		ClearVariable( cmd[8]);
		
		return;		
	}

	if(cmd[7] > RecData[0])
	{
		cmd[7] = RecData[0];
	}

	if(cmd[3] != ABSOLUTE_CURRENT)
	{
		G_CurrentRecNum[G_ChannelNum] = recordNo;
	}
	
	need_commit = IF_BeginTransaction();

	tmp = WriteLFBuf(G_CurrentEfAddr[G_ChannelNum],recordNo,updateShift,&RecData[1 + cmd[6]], cmd[7]);

	fileType = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_Type_Offset);//获取文件类型	

	if((fileType & 0x07) == CYCTYPE)														   //循环文件才会更新
	{
		tag = tmp & SetEepromEX(getFileContentAddr(G_CurrentEfAddr[G_ChannelNum]),recordNo,sizeof(BYTE));
		
		if(tag == FALSE)
		{
			  MemoryError();
			   
				if(need_commit == TRUE)
				{
					TransactionAbort();
				}		
				
				return; 
			}
	}
	
	TransactionCommit(need_commit);

	if(tmp == TRUE)
	{
		cmd[0] = 0x02;

		cmd[1] = 0x90;

		cmd[2] = 0x00;

		WriteVariable(cmd[8], cmd);
	}

	return ;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Extract SMS Data(File Operation class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Extract Specified Data From SMS, Save to Corresponding Edit Buffer (OTA3 Specification)

  test:				zhuli

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3ExtractSMSData(void)
{
	BYTE idata result;

	ReadEeprom(tempBuf, G_ByteCodeAddr + 1, 6);

	if(tempBuf[0] != 0x05)
	{
		return;
	}

	result = ReadVarible(tempBuf[2], &RecData[0]);

	if(result == FALSE)
	{
		return;
	}

	ReadSmsData(&RecData[0], tempBuf[3], tempBuf[4], tempBuf[5]);

	return;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Verify CHV/PUK  (File Operation class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Compare CHV/PUK Value(OTA3 Specification)

  test:				DK

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3VerifyCHVPUK(void)
{
	BYTE idata i;

	ReadEeprom(tempBuf, G_ByteCodeAddr+1, 5);

	tempBuf[10] = ReadVarible(tempBuf[3], &tempBuf[11]);

	if((tempBuf[2] == 0x01)||(tempBuf[2] == 0x03))   //CHV1 PUK1
	{
		i = 0x01;
	}
	else if((tempBuf[2] == 0x02)||(tempBuf[2] == 0x04))  //CHV2 PUK2
	{
		i = 0x81;
	}
	else
	{
		return;
	}
	ReadKey(i, &tempBuf[100]);

	tempBuf[50] = 0x00;

	memcpy(&tempBuf[51],&tempBuf[100 + 4 + ((tempBuf[2] / 3) * 8)],0x08);

	for(i = 0;i < 8;i++)        //统计PIN码的实际个数，排除后续的0xFF
	{
		if(tempBuf[51+i] == 0xFF)
		{
			break;
		}
		tempBuf[50] ++;
	}


	tempBuf[0] = 1;

	tempBuf[1] = 1;

//	if((memcmp(&tempBuf[11], &tempBuf[100 + 4 + ((tempBuf[2] / 3) * 8)], tempBuf[50]) != EQUAL)||(tempBuf[10] != tempBuf[50]))
	if((memcmp(&tempBuf[11], &tempBuf[51], tempBuf[50]) != EQUAL)||(tempBuf[10] != tempBuf[50]))	
	{
		tempBuf[1] = 0;
	}

	WriteVariable(tempBuf[4], tempBuf);

	return ;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Extract ADN Data  (File Operation class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Extract Name and Number From a Specified ADN Record (OTA3 Specification)

  test:				DK

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3ExtractADNData(void)
{
	BYTE idata i, len;

	ReadEeprom(tempBuf, G_ByteCodeAddr + 1, 5);

	if(tempBuf[0] != 0x04)  // check error length in this ots3 ex function
	{
		return ;
	}

	memset(&tempBuf[100], 0xFF, 50);

	len = ReadVarible(tempBuf[2], &tempBuf[50]);

	if(tempBuf[50] == 0x80)
	{
		for(i = 1; i<14; i+=2)
		{
			if(tempBuf[50 + i] == 0xFF)
			{
				if((i==13)||(tempBuf[50 + i + 1] == 0xFF))
				{
					break;
				}
			}
		}
	}
	else
	{
		for(i = 0; i<14; i++)
		{
			if(tempBuf[50 + i] == 0xFF)
			{
				break;
			}
		}
	}

	tempBuf[49] = i;

	WriteVariable(tempBuf[3], &tempBuf[49]);

	WriteVariable(tempBuf[4], &tempBuf[len - 14 + 50]);

	return ;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		var pad operation  (securily application class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Fill the source data to result var

  test:				xuqs

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3VarPadOperation(void)
{
	BYTE idata filldata[2];

	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 6);

	if(tempBuf[0] != 0x05)
	{
		return;
	}

	RecData[0] = ReadVarible(tempBuf[3], &RecData[1]);      //source data

	filldata[0] = OTAGetVariable(tempBuf[5], &filldata[1]);	//filldata

	if((filldata[0] != 0x01) || (tempBuf[4] == 0x00) || (RecData[0] == 0x00))// illegal case, set the buffer to empty
	{
		ClearVariable(tempBuf[2]);

		return ;
	}

	filldata[0] = tempBuf[4];

	if((RecData[0] % filldata[0]) != 0x00 || RecData[0] < filldata[0])        //fill source data
	{
		filldata[0] = (((RecData[0] / filldata[0])  * filldata[0]) + filldata[0]);

		memset(&RecData[RecData[0] + 1], filldata[1], (filldata[0]-RecData[0]));

		RecData[0] = filldata[0];
	}

	WriteVariable(tempBuf[2], &RecData[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Key operation  (securily application class)

  input parameter:  		void

  output parameter:	 	void

  description:                key operation

  test:				xuqs

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3KeyOperation(void)
{
	BYTE idata calMethod, desMethod;
	BYTE idata i;

	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, sizeof(BYTE));

	if ((tempBuf[0] != 0x06) && (tempBuf[0] != 0x07))
	{
		return;
	}

	ReadEeprom(&tempBuf[1], G_ByteCodeAddr+2, tempBuf[0]);//tempBuf[2] as结果变量索引 ，tempBuf[3] as 主密钥变量所引，tempBuf[4] as 分散因子变量索引

	OTAGetVariable(tempBuf[5], &calMethod);

	OTAGetVariable(tempBuf[6], &desMethod);

	RecData[0] = OTAGetVariable(tempBuf[3], &RecData[1]);	//main key

	RecData[20] = OTAGetVariable(tempBuf[4], &RecData[21]);	//source key

	if((RecData[0] != 0x10) || (RecData[20] != 0x08))        // illegal case !
	{
		ClearVariable(tempBuf[2]);

		return ;
	}

	if (desMethod == 0x01)              	//pboc mothod
	{
		if ((calMethod == 0x01) && (tempBuf[0] == 0x07))//有初始值
		{
			RecData[40] = OTAGetVariable(tempBuf[7], &RecData[41]);

			for (i = 0x01; i <= 0x08; i ++)
			{
				RecData[20+i] ^= RecData[40+i];
			}
		}

		PBOCAuth(&RecData[1], &RecData[21]);
	}

	WriteVariable(tempBuf[2], &RecData[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Random data operation  (securily application class)

  input parameter:  		void

  output parameter:	 	void

  description:       		Random data

  test:				xuqs

  status:

 -------------------------------------------------------------------------------------------------------------------*/

void OTA3RandomDataOperation(void)
{
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 4);

	if(tempBuf[0] != 0x03)
	{
		return;
	}

	RecData[0] = tempBuf[3];

	Random_Number_Generator_Get(&RecData[1], tempBuf[3]);

	WriteVariable(tempBuf[2], &RecData[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		Encrypt Decrypt Operation  (securily application class)

  input parameter:  		void

  output parameter:	 	void

  description:                Encrypt /Decrypt the key

  test:				xuqs

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3EncryptDecryptOperation(void)
{
	BYTE idata algorithm, status;
	BYTE idata i;

	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 8);

	if((tempBuf[0] != 0x06)&&(tempBuf[0] != 0x07))
	{
		return;
	}

	OTAGetVariable(tempBuf[5], &algorithm);         		// the algorithm

	RecData[0] = OTAGetVariable(tempBuf[3],&RecData[1]); 		// the source

	tempBuf[10] = OTAGetVariable(tempBuf[4], &tempBuf[11]);		//get the value of key

	if( algorithm < 0x05)	   	//01 02 03 04 为DES/3DES时
	{
		if((tempBuf[10] != 0x10) && (tempBuf[10]!=0x08))
		{
			ClearVariable(tempBuf[2]);

			return;
		}
	}
	else 	                   	//05 06 07 为RSA512时
	{
		if(tempBuf[10] != 0x03)
		{
			ClearVariable(tempBuf[2]);

			return ;
		}
	}

	
	if (tempBuf[0] == 0x07)
	{
		tempBuf[30] = OTAGetVariable(tempBuf[7], &tempBuf[31]);//获得初始值
	}

	if (((algorithm == 0x01)||(algorithm == 0x03)) && (tempBuf[6] == 0x00))//加密时初始值
	{
		for (i = 0x01; i <= 0x08; i ++)
		{
			RecData[i] ^= tempBuf[30+i];
		}
	}

	status = TRUE;

	switch(algorithm)
	{
		case 0x01:		//DEC CBC
		case 0x03:     //3DES_CBC
		{
			if (tempBuf[6] == 0x00)   // Encrypt
			{
				DESCalc(&RecData[1], &tempBuf[11], &ResData[1], CBC_Encrypt, RecData[0],(algorithm-0x01));
			}
			else if(tempBuf[6] == 0x01)  //Decrypt
			{
				DESCalc(&RecData[1], &tempBuf[11], &ResData[1], CBC_Decrypt, RecData[0],(algorithm-0x01));
			}
			else
			{
				if((tempBuf[6] == 0x02)&&(algorithm == 0x01)) // cal mac ,DES CBC
				{
					singleDesMac(&RecData[1],&tempBuf[11],&ResData[1],RecData[0],ECB_Encrypt);
	
					RecData[0] = 0x08;
	
				}
				else
				{
					status = FALSE;
				}
			}

			break;
		}
		case 0x02:	  //DEC ECB
		case 0x04:	  //3DES_ECB
		{
			if (tempBuf[6] == 0x00)   // Encrypt
			{
				DESCalc(&RecData[1], &tempBuf[11], &ResData[1], ECB_Encrypt, RecData[0],(algorithm-0x02));
			}
			else if(tempBuf[6] == 0x01)  //Decrypt
			{
				DESCalc(&RecData[1], &tempBuf[11], &ResData[1], ECB_Decrypt, RecData[0],(algorithm-0x02));
			}
			else
			{
				status = FALSE;
			}
			
			break;
		}
		default:
			status = FALSE;
			break;
	}

	if (((algorithm == 0x01)||(algorithm == 0x03)) && (tempBuf[6] == 0x01))//解密时初始值
	{
		for (i = 0x01; i <= 0x08; i ++)
		{
			ResData[i] ^= tempBuf[30+i];
		}
	}

	if(status == FALSE)
	{
		ResData[0] = FALSE;
	}
	else
	{
		if(algorithm != 0x05)
		{
			ResData[0] = RecData[0];
		}		
	}

	WriteVariable(tempBuf[2], &ResData[0]);
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTA3HexToAscii

  input parameter:  		void

  output parameter:	 	void

  description:       		将源字串中的16进制数据转成ASCII码

  coder:				xuqs

  test:

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3HexToAscii(void)
{
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 4);

	if (tempBuf[0] != 0x03)
	{
		return;
	}

	RecData[0] = OTAGetVariable(tempBuf[2], &RecData[1]);	//src

	IntToString(&RecData[0], &RecData[0]);		//Hex to ASCII

	WriteVariable(tempBuf[3], &RecData[0]);
}


/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTA3CountOperation

  input parameter:  		void

  output parameter:	 	void

  description:       		应用可以使用这个命令来进行加减乘除运算操作。

  coder:				xuqs

  test:

  status:				modify by ChenL for OTP	20091130

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3CountOperation(void)
{
#ifndef OTP
	BYTE xdata cmd[7];
	WORD xdata operanded;//被操作数
	WORD xdata operand;//操作数
	DWORD xdata result = 0;

	ReadEeprom(&cmd[0], G_ByteCodeAddr+1,7);

	if (cmd[0] != 6)
	{
		return;
	}
	tempBuf[0] = OTAGetVariable(cmd[3], &tempBuf[1]);	//读取被操作数字符串

	RecData[0] = OTAGetVariable(cmd[4], &RecData[1]);	//读取操作数字符串

	if (tempBuf[0] == 0x02)
	{
		operanded = ((tempBuf[1]<<8)&0xFF00) + (tempBuf[2]&0x00FF);
	}
	else
	{
		operanded = tempBuf[1]&0x00FF;
	}
	if (RecData[0] == 0x02)
	{
		operand = ((RecData[1]<<8)&0xFF00) + (RecData[2]&0x00FF);
	}
	else
	{
		operand = RecData[1]&0x00FF;
	}

	RecData[0] = 0x01;			//辅助变量

	RecData[1] = 0x00;

	switch(cmd[2])
	{
		case 0x01:		//加法运算

			result = (DWORD)operanded + (DWORD)operand;

			NumToHEX(result, &tempBuf[0]); 		//结果变量

			break;

		case 0x02:	//减法运算

			result =  (DWORD)operanded - (DWORD)operand;

			NumToHEX(result, &tempBuf[0]);

			if(tempBuf[0] > 2)
			{
				RecData[1] = 0x01;
			}
			break;

		case 0x03:		//乘法运算

			result = (DWORD)operanded * (DWORD)operand;

			NumToHEX(result, &tempBuf[0]);

			break;

		case 0x04:		//除法运算

			if (operand == 0)	//操作数为0，商和余数都置为0xFFFFFFFF
			{
				tempBuf[0] = 0x04;

				memset(&tempBuf[1], 0xFF, tempBuf[0]);

				RecData[0] = 0x04;

				memset(&RecData[1], 0xFF, RecData[0]);
			}
			else
			{
				result = (DWORD)operanded / (DWORD)operand;

				NumToHEX(result, &tempBuf[0]);


				result = (DWORD)operanded % (DWORD)operand;

				NumToHEX(result, &RecData[0]);
			}
			break;

		default:
			break;

	}
	WriteVariable(cmd[5], &tempBuf[0]);

	WriteVariable(cmd[6], &RecData[0]);

#else

	BYTE idata i;
	
	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1,7);
	if (tempBuf[0] != 6)
	{
		return;
	}
	
	tempBuf[10] = OTAGetVariable(tempBuf[3], &tempBuf[11]);	//读取被操作数字符串

	tempBuf[20] = OTAGetVariable(tempBuf[4], &tempBuf[21]);	//读取操作数字符串

	RecData[0] = 0x01;		//辅助变量
	RecData[1] = 0x00;
	
	switch(tempBuf[2])
	{
		case 0x01:		//加法
		{
			AddOperation(&tempBuf[31], &tempBuf[30], &tempBuf[11], tempBuf[10], &tempBuf[21], tempBuf[20]);

			break;
		}
		case 0x02:		//减法
		{
			SubOperation(&tempBuf[31], &tempBuf[30], &RecData[1], &tempBuf[11], tempBuf[10], &tempBuf[21], tempBuf[20]);

			break;
		}
		case 0x03:		//乘法
		{
			MultOperation(&tempBuf[31], &tempBuf[30], &tempBuf[11], tempBuf[10], &tempBuf[21], tempBuf[20]);

			break;
		}
		case 0x04:		//除法
		{	
			for(i=0; i<tempBuf[20]; i++)
			{
				if(tempBuf[21+i] != 0x00)
				{
					break;
				}
			}
			if(i == tempBuf[20])		//操作数为0，商和余数都置为8字节全FF
			{
				 tempBuf[30] = 0x08;

				memset(&tempBuf[31], 0xFF, tempBuf[30]);

				RecData[0] = 0x08;

				memset(&RecData[1], 0xFF, RecData[0]);
			}
			else
			{
				DivOperation(&tempBuf[31], &tempBuf[30], &RecData[1], &RecData[0], &tempBuf[11], tempBuf[10], &tempBuf[21], tempBuf[20]);
			}
			
			break;
		}
		default:
			break;
	}
	
	WriteVariable(tempBuf[5], &tempBuf[30]);

	WriteVariable(tempBuf[6], &RecData[0]);	
#endif
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTA3EraseFile

  input parameter:  		void

  output parameter:	 	void

  description:       		应用可以使用这个命令来将一个文件内容清空(将文件内容置为全F)

  coder:				xuqs

  test:

  status:

 -------------------------------------------------------------------------------------------------------------------*/

void OTA3EraseFile(void)
{
	UWORD xdata address;
	WORD idata spacesize;
	BYTE idata tmp;
	BYTE idata opt;

	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 4);	//tempBuf[0] as length ,tempBuf[1] as 0x37 tempBuf[2] as 源文件ID ，tempBuf[3] as 操作类型

	if (tempBuf[0] != 0x03)
	{
		return;
	}
	opt = tempBuf[3];

	address = GetLinkEFAddr(tempBuf[2]);  //源文件ID 若该文件为影像文件，则指向影像文件地址	 

	if(address == FALSE)
	{
		return;
	}

	tmp = AccessDefined(address, A_UpdateFile);	//对于映像文件而言 ，要求使用本身的权限，而不是目标文件的权限

	if ((tmp != 0x90) && (tmp != 0x01))	//不具有PIN/ALW Update权限
	{
		return;
	}
	tmp = GetFileHeadByte(address,F_Status_Offset);

	if((tmp&F_MAP_FILE) == F_MAP_FILE)	  //若该文件为影像文件，则指向实际目标文件地址
	{
		address = GetParentOrMapFileAddr(address, F_MapFile_Offset);			
	}

	tmp = GetFileHeadByte(address, F_Type_Offset);	//EF的类型

	if ((tmp == TRANTYPE)||(tmp == 0x39))	//二进制文件   or  BER-TLV structure EF
	{
		spacesize = GetTFSize(address);
	}
	else 									//记录文件
	{
		spacesize = (WORD)(GetLFRecNumLgth(address,FileNum) * GetLFRecNumLgth(address,FileLgth));
	}
	if (opt == 0x00)	//操作类型:0 清空,其他值保留
	{
		if(tmp != 0x39)
		{
			address = getFileContentAddr(address);
		}


		SetEeprom(address, 0xFF, spacesize);
	}
}

/*
*@description :  获取变量索引里存在的源文件DF+EF，多个DF，按选择顺序保存，例如：3F007F106F3A
*
*@param       :  变量索引
*
*@return      :	  FALSE/ef addr
*
*@history     :  create  by xyz @100705 10:38
*/
UWORD GetLinkEFAddr(BYTE tmp)
{
	BYTE idata i;
	WORD idata fileId;
	UWORD xdata  address;

   	RecData[0] = OTAGetVariable(tmp, &RecData[1]);	//dest id
	
	address = E_FILE_START;

	for(i=1; i< RecData[0];  i+= 2)
	{
		fileId = ((RecData[i]<<8)&0xFF00) + (RecData[i+1]&0x00FF);

		if(fileId == 0x7F21)
	    {
	        fileId = 0x7F20;
	    }
		
		address = SearchFile(address, fileId);	

		if (address == FALSE)
		{
			return  FALSE; 
		}
	}
		
	if(GetFileHeadByte(address ,F_Type_Offset) == ADFTYPE )   //必须为EF文件
	{
		return FALSE;
	}

	return address;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTA3LinkFile

  input parameter:  		void

  output parameter:	 	void

  description:       		应用可以使用这个命令来将两个文件作链接关系，链接的两个文件必须为EF文件

  coder:				DK

  test:

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3LinkFile(void)
{
	BYTE idata accessCon;
	BYTE xdata cmd[3];
	UWORD xdata desFileAddr, srcFileAddr;

	ReadEeprom(&cmd[0], G_ByteCodeAddr + 3, 3);	  //tmp[0] as 源文件ID ，tmp[1] as 目标文件ID tmp[2]as 链接类型

	desFileAddr =  GetLinkEFAddr(cmd[1]);

	if(desFileAddr == FALSE)  // can't select the dest file ..return
	{
		return ;
	}

	accessCon = AccessDefined(desFileAddr, A_UpdateFile);

	if ((accessCon != 0x90) && (accessCon != 0x01))	//不具有PIN/ALW Update权限
	{
		return;
	}

	accessCon = AccessDefined(desFileAddr, A_ReadFile);

	if ((accessCon != 0x90) && (accessCon != 0x01))	//不具有PIN/ALW Read权限
	{
		return;
	}
	
 	srcFileAddr = GetLinkEFAddr(cmd[0]);

	if(srcFileAddr == FALSE)
	{
		return ;
	}

	accessCon = cmd[2]; // the operation

	RecData[0] = OTAGetVariable(cmd[1], &RecData[1]);     //获得目标文件的路径ID

	ReadEeprom(&tempBuf[0], srcFileAddr, sizeof(EFileHead));

	if(accessCon == 0)
	{
		if(RecData[0] == 0x04)
		{
			tempBuf[F_MapFile_Offset] = 0x3F;

			tempBuf[F_MapFile_Offset+1] = 0x00;

			memcpy(&tempBuf[F_MapFile_Offset+2], &RecData[RecData[0] - 3], 0x04);
		}
		else
		{  
			memcpy(&tempBuf[F_MapFile_Offset], &RecData[RecData[0] - 5], 0x06);
		}

		tempBuf[F_Status_Offset] |= F_MAP_FILE;
	}
	else if(accessCon == 1)
	{
		tempBuf[F_Status_Offset] &= ~F_MAP_FILE;
	}
	else	//for baihe-test
	{;}

	WriteEepromEX(srcFileAddr, &tempBuf[0], sizeof(EFileHead));

	return;
}


/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTA3GetBufferLength

  input parameter:  		void

  output parameter:	 	void

  description:       		返回Edit_Buf/应用变量/OTA空间分配变量中的当前数据长度(L值)

  coder:				DK

  test:

  status:

 -------------------------------------------------------------------------------------------------------------------*/

void OTA3GetBufferLength(void)
{
    ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, 4);

	tempBuf[5] = 0x01;

	tempBuf[6] = 0x00;

	if(tempBuf[0] != 0x03)
	{
		WriteVariable(tempBuf[3], &tempBuf[5]);

		return;
	}

	tempBuf[6] = OTAGetVariable(tempBuf[2], &tempBuf[7]);

	WriteVariable(tempBuf[3], &tempBuf[5]);
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTA3ArrayCopy

  input parameter:  	void

  output parameter:	 	void

  description:       	从指定的源字串的某个指定位置开始拷贝指定长度的数据到目的字串的指定位置

  coder:				DK

  test:

  status:

 -------------------------------------------------------------------------------------------------------------------*/

void OTA3ArrayCopy(void)
{
	BYTE idata source, sourceShift, dest, destShift, len;

	ReadEeprom(&tempBuf[0], G_ByteCodeAddr + 1, 7);

	source = tempBuf[2];

	OTAGetVariable(tempBuf[3], &sourceShift);

	dest = tempBuf[4];

	OTAGetVariable(tempBuf[5], &destShift);

	OTAGetVariable(tempBuf[6], &len);

	tempBuf[0] = OTAGetVariable(source, &tempBuf[1]);

	RecData[0] = OTAGetVariable(dest, &RecData[1]);

	len = ((tempBuf[0] - sourceShift) < len) ? (tempBuf[0] - sourceShift) : len;

	memcpy(&RecData[destShift+1], &tempBuf[sourceShift+1], len);

	RecData[0] = len + destShift;

	WriteVariable(dest, &RecData[0]);
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTA3EventRegister

  input parameter:  		void

  output parameter:	 	void

  description:       		OTA解析器事件注册命令: 事件注册或应用执行

  coder:				xuqs

  test:

  status:

 -------------------------------------------------------------------------------------------------------------------*/
void OTA3EventRegister(void)
{
	BYTE idata regStatus[2];	//事件注册状态
	BYTE idata eventId;		//注册事件的ID
	BYTE idata i;
	UWORD xdata fileAddr;
	BYTE idata index,tag;

	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, sizeof(BYTE));

	ReadEeprom(&tempBuf[1], G_ByteCodeAddr+2, tempBuf[0]);

	regStatus[0] = OTAGetVariable(tempBuf[1], &regStatus[1]);

	tag = FALSE;

	for (i = 0x00; i < tempBuf[2]; i++)	//tempBuf[2]:注册事件的数量
	{
		eventId = tempBuf[5*i +3];

		G_IsUpdateEventList = FALSE;//不需要更新事件列表

		if (regStatus[1] == 0x00)	//事件未注册，执行注册操作
		{
			if(eventId < 0x0B)
			{
				tag = TRUE;
			}

			fileAddr = SelectLFFile(DF_OTA, EF_OTAEventRegister);

			if (fileAddr == FALSE)
			{
				return;
			}
			index = LocateRecordInFile(fileAddr, eventId);	//查找事件ID是否曾经给其他应用注册过

			if (index == FALSE)//事件ID没有被注册过，则查找空记录
			{
				if(tag == TRUE)
				{
					G_IsUpdateEventList = TRUE;	//需要更新事件列表
				}

				index = LocateRecordInFile(fileAddr, 0xFF);
			}
			ReadLFBuf(fileAddr, index, 0x01, &RecData[0], 0x01);

			if (RecData[0] == 0xFF)
			{
				RecData[0] = 0x00;
			}
			ReadLFBuf(fileAddr, index, 0x02, &RecData[1], RecData[0]*3);

			memcpy(&RecData[RecData[0]*3+1], &G_EventAID[0], 0x03);

			RecData[0] = RecData[0] + 1;

			SortApplicationID(&RecData[0], 3);	//AID排序

			SetLFConst(fileAddr, index, 0x00, eventId, sizeof(BYTE));	//保存事件ID到文件EF_OTAEventRegister

			WriteLFBuf(fileAddr, index, 0x01, &RecData[0], RecData[0]*3+1);//保存事件已下载的AID到文件EF_OTAEventRegister

			if(i == (tempBuf[2] - 1))  //最后一个事件注册完成后，退出
			{
				regStatus[1] = 0x01;

				OTAUpdateVariable(tempBuf[1], &regStatus[0]);

				G_HandleResponse = 0x03;

				return;
			}
		}
		else		//事件已注册，应用执行
		{
			fileAddr = SelectTFFile(DF_OTA, EF_OTAEventContent);//该文件用于缓存事件的数据内容

			if (fileAddr == FALSE)
			{
				return;
			}
			ReadTFBuf(fileAddr, 0x01, &ResData[0], sizeof(BYTE));

			ReadTFBuf(fileAddr, 0x02, &ResData[1], ResData[0]);

			if(eventId == G_EventTag) //注册事件的IDn 等于当前正在触发的事件
			{
				WriteVariable(tempBuf[5*i+4], &ResData[0]);	//事件获取的详细数据存入EDITBUF

				memcpy(&G_Pid[0], &tempBuf[5*i +5], 0x03);	//获取到事件时，卡片跳转到事件处理的PID

				G_CmdIndex = 1;

				G_HandleResponse = 0x00;

				return;
			}
		}
	}
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :		OTA3EventAnalyse

  input parameter:  		void

  output parameter:	 	void

  description:       	事件数据解析命令

  coder:				xuqs

  test:

  status:

 -------------------------------------------------------------------------------------------------------------------*/

#ifdef LOCALOTA
void OTA3EventAnalyse(void)
{
	BYTE idata i,result,length;

	ReadEeprom(&tempBuf[0], G_ByteCodeAddr+1, sizeof(BYTE)); //tempBuf[0] :后续长度，tempBuf[1]:存放索引，tempBuf[2] :后续期望数据个数 

	ReadEeprom(&tempBuf[1], G_ByteCodeAddr+2, tempBuf[0]);

	length = OTAGetVariable(tempBuf[1], &RecData[0]);  //存放的数据

	for(i=1;i<=tempBuf[2];i++)
	{
		result = FindTag(&RecData[0], tempBuf[i*2 +1], length);

		if(result == FALSE)
		{
			G_HandleResponse = 0x03;

			return;
		}

		WriteVariable(tempBuf[2*i+2], &RecData[result]);	//事件获取的详细数据存入EDITBUF
	}

	G_HandleResponse = 0x00;

	G_CmdIndex ++;  //继续执行后续指令
	
}

/*
*@description : 本地OTA 中子API功能的正确实现
*/
void LocalOTA3SubAPI(void)
{
	BYTE idata index;

	ReadEeprom((BYTE *)&index,G_ByteCodeAddr+3,sizeof(BYTE));  //函数索引

	switch(index)
	{
		case 0x01:   //display text
		{
			OTADisplayText(0x05);  //命令限定符相对于G_ByteCodeAddr的起始偏移
			
			break;	
		}
		default:
			break;
	}
}
#endif
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 OTAPlugInFuction

  input parameter:   void

  output parameter:	 void

  description:       OTA2 Get Input G_Command

  Coder:			 OTA extern fuction

  status:

 -------------------------------------------------------------------------------------------------------------------*/

void OTAPlugInFuction(void)
{
	G_HandleResponse = 0;

	ReadEeprom(&G_Command,G_ByteCodeAddr+2,sizeof(BYTE));
	
	if (IF_OTAExExFuntion() == FALSE)
	{
			switch (G_Command)
			{
				case OTA2_EX_PAGEDISPADN:
					  OTAPageDispAdn();
					break;
				case OTA2_EX_ADNTOASCII:
					OTAAdnToAscii();
					break;
				case OTA2_EX_ASCIITOADN:
					OTAAsciiToAdn();
					break;
				case OTA2_EX_LOCATIONINFOTOASCII:
					OTALocationInforToAscii();
					break;
				case OTA2_EX_SETPOSITIONSERVICE:
					OTASetpositionService();
					break;
				case OTA2_EX_SAVETOEDITBUF:
					OTASaveToEditBuf();
					break;
				case OTA2_EX_COPYVAR:
					OTACopyVar();
					break;
				case OTA2_EX_DISPLAYVAR:
					OTADisPlayVar();
					break;
				case OTA2_EX_DELETEOTAVARCONTENT:
					OTADeleteVarContent();
					break;
				case OTA2_EX_FINDNULLOTAVAR:
					OTAFindNullVar();
					break;
				case OTA2_EX_LEFTTEXPAND:
				case OTA2_EX_RIGHTEXPAND:
					OTATextPand(G_Command);
					break;
				case OTA2_EX_STRINGCOMPARE:
					OTAStringCompare();
					break;
				case OTA2_EX_SUBSTRING:
					OTASubString();
					break;
				case OTA2_EX_ASCIITOUCS2:
					OTAAsciiToUcs2();
					break;
				case OTA2_EX_REPLACECHAR:
					OTAReplaceChar();
					break;
		 		case  OTA2_EX_JMPZERO:
				case  OTA2_EX_JMPFROMEDITBUF:
					OTAJmpZeroEditBuf(G_Command);
					G_CmdIndex --;
					break;
				case OTA2_EX_ADDOPERATION:
				case OTA2_EX_SUBOPERATION:
					OTASubAddOperation(G_Command);
					break;
				case OTA2_EX_MODOPERATION:
					OTAModOperation();
					break;
				case OTA2_EX_SENDTAB:
					OTASendTab();
					break;
				case OTA3_EX_ARRAY_COMPARE:
					OTA3ArrayCompare();
					break;
				case OTA3_EX_ARRAY_FILL:
					OTA3ArrayFill();
					break;
				case OTA3_EX_INIT_PROACTIVE_COMMAND:
					OTA3InitProactiveCommand();
					break;
				case OTA3_EX_APPEND_ARRAY:
					OTA3AppendArray();
					break;
				case OTA3_EX_APPEND_TLV:
					OTA3AppendTLV();
					break;
				case OTA3_EX_CHECK_PROFILE:
					OTA3CheckProfile();
					break;
				case OTA3_EX_COPY_VALUE_BY_TAG:
					OTA3CopyValueByTag();
					break;
				case OTA3_EX_GET_GENERAL_RESULT:
					OTA3GetGeneralResult();
					break;
				case OTA3_EX_SEND:
					OTA3Send();
					break;
				case OTA3_EX_READ_BINARY:
					OTA3ReadBinary();
					break;
				case OTA3_EX_READ_RECORD:
					OTA3ReadRecord();
					break;
				case OTA3_EX_SELECT:
					OTA3Select();
					break;
				case OTA3_EX_UPDATE_BINARY:
					OTA3UpdateBinary();
					break;
				case OTA3_EX_UPDATE_RECORD:
					OTA3UpdateRecord();
					break;
				case OTA3_EX_EXTRACT_SMS_DATA:
					OTA3ExtractSMSData();
					break;
				case OTA3_EX_VERIFY_CHV_PUK:
					OTA3VerifyCHVPUK();
					break;
				case OTA3_EX_VARPAD_OPERATION:
					OTA3VarPadOperation();
					break;
				case OTA3_EX_KEY_OPERATION:
					OTA3KeyOperation();
					break;
				case OTA3_EX_RANDOM_DATA_OPERATION:
					OTA3RandomDataOperation();
					break;
				case OTA3_EX_ENCRYPT_DECRYPT_OPERATION:
					OTA3EncryptDecryptOperation();
					break;
				case OTA3_EX_GET_AID:
					OTA3GetAID();
					break;
				case OTA3_EX_STRING_MERGE:
					OTA3StringMerge();
					break;
				case OTA3_EX_EXTRACT_ADN_DATA:
					OTA3ExtractADNData();
					break;
				case OTA3_EX_BIT_OPERATION:
					OTA3BitOperation();
					break;
				case OTA3_EX_LINK_FILE:
					OTA3LinkFile();
					break;
				case OTA3_EX_ERASE_FILE:
					OTA3EraseFile();
					break;
				case OTA3_EX_GET_BUFFER_LENGTH:
					OTA3GetBufferLength();
					break;
				case OTA3_EX_HEX_TO_ASCII:
					OTA3HexToAscii();
					break;
				case OTA3_EX_ARRAY_COPY:
					OTA3ArrayCopy();
					break;
				case OTA3_EX_COUNT_OPERATION:
					OTA3CountOperation();
					break;
#ifdef LOCALOTA
				case 0x80 :    //定义的子API索引
				{
					LocalOTA3SubAPI();
					break	;			
				}
#endif
				default:
					G_HandleResponse = 2;				//exit ota2run
					return;
			}
	}


	if(G_HandleResponse == 0)
	{
		G_CmdIndex++;
	}
}

#endif
