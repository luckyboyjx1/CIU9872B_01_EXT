/*
*@Vesion         :V1.0
*
*@description    :该文件主要用于实现TS102 226 中5.2 Extended Remote Application Data Format所描述的功能
*
*@history        :V1.0: create file
*				  基于条件:不支持TAR不同,短信批次相同的情况
*				          V1.1 修改函数ExtendedRFMExcuteCommand,增加备份G_CurrentAdfName[G_ChannelNum]与G_CurrentSfi,G_ChannelNum                 
*/
#include  "typedef.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include "uicc_error.h"
#include  "arithmatic.h"
#include  "ceph.h"
#include  "CompactManage.h"
#include  "ExpandedManage.h"
#include "BIPModuleProcess.h"
#include  "ToolkitRFM.h"
//#include  "ToolkitApiPublic.h"
//#include "EUICCProfile.h"
//#include  "ProfileInterpreter.h"
//#include "profileApi.h"
#if GP__AMENDMENT__B__FOR_MASK
#include "Interface_AnnexB.h"
#include "httpprocess.h"
#endif
#include "Interface_LTEFromBoot.h"

BYTE xdata G_CurrentChainingValueTag;   //保存当前操作的ChainingValue
WORD xdata G_ErrorActionOffsetLgth[2];      //指示当前最后一个ErrorActionTLV在文件中的偏移
BYTE xdata G_EarlyResponseTag ;                 //标志当前是否存在early response TAG
BYTE xdata G_CAPDUResponse;            //用来判断有没有执行过TAG_R_APDU  810182/或者最后执行的是81 等tag时候需要拼装数据
BYTE xdata G_ChainingTagRec;      //用来表示83 tag是否收到(在Http一个协议数据过程中可能带83 tag(目前情况仅适合一个协议数据存在一个83tag),由于Http在ReceiveData可能在任一点断开数据,会造成先处理了83tag,返回继续接收数据后再次进入102226的没有83tag)
                                  //目前遇到的情况是 830102 86XX (86不完整的,先处理了83,返回Http继续接收数据,再次进入102226的数据不带83tag)

extendedRFMContext xdata ChainingValue;  //note:为保持一致性,卡片复位后,若firstScriptValue = 01 或firstScriptValue = 0x11 &&previousValue = 0x03 ,则清空该文件

#if UICC__FUNCTION__RFMRAM_EXTEND
/*
*@function:  GPRAMRFMExtendResponsePublic
*
*@description: 组装扩展模式下RAM的响应数据 ，
*
*param       :tag
*
*@return     :TRUE :数据正常/others error
*		
*			:modify by licaizhang-20210512 http模式时的AB拼装，命令执行个数使用G_HTTPSStatus.HttpApduNum
*/
BYTE  GPRAMRFMExtendResponsePublic(BYTE tag)
{
	UWORD xdata fileAddr;
	WORD idata offset,lgth,fileSize;
	WORD idata resLength;
	//BYTE idata buf[2];
	BYTE idata AAorAE;
	
	resLength = G_CurrentRemoteResponseLgth;


	if (G_Source_Type == SourceType_HTTPS)
	{
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM		
	if(G_FormatAndType == FormatType_ExtendAA)   //AA TAG需要发送当前执行的指令条数, EF_PorExtendDataFile文件的前3字节为 8001xx
	{
		AAorAE = 3;                  
	}
	else                          	//AE TAG不需要发送当前执行的指令条数
	{
		AAorAE = 0;
	}
	
	if(tag == TAG_ClearResponseFile)
	{
		//offset = ReadTFFileWord(fileAddr);//ReadTFBuf(fileAddr, ramExtendResLen_offset, (BYTE *)&offset ,sizeof(WORD));
		offset = BuildWord(HTTP_ResponseData_ADDR);
		
		if(offset != FALSE)
		{		
			//WriteTFFileWord(fileAddr, 0x00);//SetTFConst(fileAddr,ramExtendResLen_offset, 0x00,0x02);
			ecpMem_Fill(&HTTP_ResponseData_ADDR[0], 0x00, 2);
		}

		return TRUE;
	}

	//fileSize = GetTFSize(fileAddr);
	fileSize = HTTPSResponseData_SIZE;
	
	//offset = ReadTFFileWord(fileAddr);//ReadTFBuf(fileAddr, 0x00, (BYTE *)&offset ,sizeof(WORD));
	offset = BuildWord(&HTTP_ResponseData_ADDR[0]);
	
//	offset = BuildWord(&buf[0]);
	
	if(G_EarlyResponseTag != FALSE)  //?ù?¨?¨????μ?early responseoó???êDí·￠?í
	{
		return TRUE;
	}

	if(tag == TAG_BadFormat)
	{
		offset += 3;
		
		if((offset+7) >fileSize)  //response buffer2?1?ó?
		{
			return InsufficientMemory;
		}
	}
	
	if(tag == TAG_R_APDU)  //判断response buffer是否够用
	{
		if(resLength <= 0x7D)  //lgth <= 0x7F (包括两个字节的状态响应字节)
		{
			offset += resLength+1+ AAorAE;
		}
		else  if(resLength <= 0xFD)  // 0x80<=lgth  <=0xFF
		{
			offset += resLength+2+ AAorAE;
		}
		else
		{
			offset += resLength+3+ AAorAE;
		}
		
		if((offset+7) >fileSize)  //response buffer不够用
		{
			return InsufficientMemory;
		}
	}
	
	if(tag == TAG_BadFormat)   //update bad format TLV
	{
		tempBuf[0] = TAG_BadFormat;
		
		tempBuf[1] = 0x01;
		
		tempBuf[2] = RecData[0];
		
		//offset = ReadTFFileWord(fileAddr);//ReadTFBuf(fileAddr, ramExtendResLen_offset, (BYTE *)&offset ,sizeof(WORD));
		offset = BuildWord(&HTTP_ResponseData_ADDR[0]);
		
		//WriteTFBuf(fileAddr, offset+2+AAorAE, &tempBuf[0],0x03);
		ecpMem_Copy(&HTTP_ResponseData_ADDR[offset+2+AAorAE], &tempBuf[0], 0x03);
		
		offset += 3;
		
	}
	else if((tag == TAG_ScriptChainingResponse) ||(tag == TAG_ImmediateActionResponse)) //table 5.15
	{		
		tempBuf[0] = tag;
		
		tempBuf[1] = 0x01;
		
		tempBuf[2] = RecData[0];
		
		//WriteTFBuf(fileAddr, 2+AAorAE, &tempBuf[0],0x03);
		ecpMem_Copy(&HTTP_ResponseData_ADDR[2+AAorAE], &tempBuf[0], 0x03);
		
		offset = 0x03;
	}

	//对于 TAG_ResponseRamExtendRes 来说，需要判断空间是否足够，目前暂时假设为空间足够

	if(tag == TAG_NumberCommandTLV)	 //该tag最后一次写入,  //23+L+response+sw
	{
		if(G_FormatAndType == FormatType_ExtendAA) //AA--AB  <ts_102226v090000p.pdf>  Table 5.10/Table 5.10a
		{
			tempBuf[0] = 0x80;
			
			tempBuf[1] = 0x01;
			
			//AB因为只有一次进入本函数，在http中，此时的命令执行个数应该是G_HTTPSStatus.HttpApduNum，缺陷编号260, 用例：电信Https能力要求/Chunked_64Apdu_definite_length_AA
			if(G_Source_Type == SourceType_HTTPS)
			{
				tempBuf[2] =  G_HTTPSStatus.HttpApduNum;
			}
			else
			{
				tempBuf[2] = G_CurrentComNUM;
			}
			
			//offset = ReadTFFileWord(fileAddr);//ReadTFBuf(fileAddr, 0x00, (BYTE *)&offset ,sizeof(WORD));
			offset = BuildWord(&HTTP_ResponseData_ADDR[0]);
			
			//offset = BuildWord(&buf[0]);
			
			//WriteTFBuf(fileAddr, 0x02, &tempBuf[0], 0x03);
			ecpMem_Copy(&HTTP_ResponseData_ADDR[0x02], &tempBuf[0], 0x03);
			
			offset += 3;
		}
	}
	else if(tag == TAG_R_APDU)	
	{
		G_CAPDUResponse = TRUE; //标志已经执行过C_APDU 810182时候使用
		
		//offset = ReadTFFileWord(fileAddr);//ReadTFBuf(fileAddr, 0x00, (BYTE *)&offset ,sizeof(WORD));
		offset = BuildWord(&HTTP_ResponseData_ADDR[0]);

 		//SetTFConst(fileAddr, offset+2+AAorAE, TAG_R_APDU,0x01); //写入 TAG_R_APDU
		ecpMem_Fill(&HTTP_ResponseData_ADDR[offset+2+AAorAE], TAG_R_APDU, 0x01);
		
		if(resLength <= 0x7D)  //lgth <= 0x7F (包括两个字节的状态响应字节)
		{	
			//SetTFConst(fileAddr, offset+3+AAorAE, resLength+2,0x01); 
			ecpMem_Fill(&HTTP_ResponseData_ADDR[offset+3+AAorAE], resLength+2, 0x01);
			
			RecData[resLength]  = SW1;
			
			RecData[resLength+1]  = SW2;
			
			//WriteTFBuf(fileAddr, offset +4+AAorAE, &RecData[0],resLength+2);
			ecpMem_Copy(&HTTP_ResponseData_ADDR[offset +4+AAorAE], &RecData[0], resLength+2);
			
			offset += resLength+2 +2;
		}
		else  if(resLength <= 0xFD)  // 0x80<=lgth  <=0xFF
		{
			//SetTFConst(fileAddr, offset+3+AAorAE, 0x81,0x01); 
			ecpMem_Fill(&HTTP_ResponseData_ADDR[offset+3+AAorAE], 0x81, 0x01);
			
			//SetTFConst(fileAddr, offset+4+AAorAE, resLength+2,0x01);
			ecpMem_Fill(&HTTP_ResponseData_ADDR[offset+4+AAorAE], resLength+2, 0x01);
			
			RecData[resLength]  = SW1;
			
			RecData[resLength+1]  = SW2;
			
			//WriteTFBuf(fileAddr, offset+5+AAorAE, &RecData[0],resLength+2);
			ecpMem_Copy(&HTTP_ResponseData_ADDR[offset+5+AAorAE], &RecData[0], resLength+2);
			
			offset += resLength+2 +3;
		}
		else 
		{		
			lgth = resLength +2;  //添加状态响应字节的长度
/*			
			SetTFConst(fileAddr, offset+6, 0x82,0x01);
			
			SetWord(&buf[0],lgth);
			
			WriteTFBuf(fileAddr, offset+7, &buf[0],sizeof(WORD));
*/
            tempBuf[0] = 0x82;         //TL改成统一按照大端写  

			tempBuf[1] = (lgth>>8)&0xFF;
			
			tempBuf[2] = lgth & 0xFF;
			
			//WriteTFBuf(fileAddr, offset+3+AAorAE, &tempBuf[0],3);
			ecpMem_Copy(&HTTP_ResponseData_ADDR[offset+3+AAorAE], &tempBuf[0], 3);
			
			if((G_CurrentRemoteResponseLgth !=0x00)&&(G_CurrentRFMEfAddr != FALSE))   //RFM数据  这两个变量POR或者 执行前都要清空
			{
				//fileSize = GetTFSize(fileAddr);
				fileSize = HTTPSResponseData_SIZE;
		
				//VMCopyDataToAddr(fileAddr+sizeof(TranFileHead)+offset+6+AAorAE, fileSize-(offset+6+AAorAE), G_CurrentRFMEfAddr, resLength, MODE_PROTECT_WRITE);
				VMReadBuf(&HTTP_ResponseData_ADDR[offset+6+AAorAE], G_CurrentRFMEfAddr, resLength);
			}
			else                                                                      //CASDGetData时候缓存的数据  RAM时候  获取CA证书时候长度超过FF,但是数据还是存放在RecData 后面 直接取
			{
				//WriteTFBuf(fileAddr, offset +6+AAorAE, &RecData[0],resLength);
				ecpMem_Copy(&HTTP_ResponseData_ADDR[offset +6+AAorAE], &RecData[0], resLength);
			}

			//SetTFConst(fileAddr, offset +6+AAorAE+resLength, SW1,0x01);
			ecpMem_Fill(&HTTP_ResponseData_ADDR[offset +6+AAorAE+resLength], SW1, 0x01);
			
			//SetTFConst(fileAddr, offset +6+AAorAE+resLength+1, SW2,0x01);
			ecpMem_Fill(&HTTP_ResponseData_ADDR[offset +6+AAorAE+resLength+1], SW2, 0x01);
			
			offset += resLength+2 +4;
		}				
	}
	else
	{
		//	白盒测试添加  tag仅存在3种值  tag == TAG_ClearRamExtendRes 的情况已经在前面返回 不会走到此处
	}
	
//	SetWord(&buf[0], offset);
	
	//WriteTFFileWord(fileAddr, offset);//WriteTFBuf(fileAddr, 0x00, (BYTE *)&offset, sizeof(WORD));
	SetWord(&HTTP_ResponseData_ADDR[0], offset);
	
#if UICC_SUPPORT_CHAGE_DELIVER_POR
	//por deliver
	if((G_BakCommandNumber==0x00)&&(G_Source_Type == SourceType_SMS)&&(tag == TAG_R_APDU)&&(offset > POR_DATA_MAX_DELIVER))
	{
		G_BakCommandNumber = G_CurrentComNUM;
	}
#endif
	return TRUE;
#else
		fileAddr = SelectPrivateTFFile(DF_TLS, EF_HTTPSResponseData);
#endif
	}
	else
	{
		fileAddr = SelectPrivateTFFile(DF_Toolkit,EF_RAMExtendResponse);  //lv 结构，前2字节为L ,
	}

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	if(G_FormatAndType == FormatType_ExtendAA)   //AA TAG需要发送当前执行的指令条数, EF_PorExtendDataFile文件的前3字节为 8001xx
	{
		AAorAE = 3;                  
	}
	else                          //AE TAG不需要发送当前执行的指令条数
	{
		AAorAE = 0;
	}
	
	if(tag == TAG_ClearResponseFile)
	{
		offset = ReadTFFileWord(fileAddr);//ReadTFBuf(fileAddr, ramExtendResLen_offset, (BYTE *)&offset ,sizeof(WORD));
		
		if(offset != FALSE)
		{		
			WriteTFFileWord(fileAddr, 0x00);//SetTFConst(fileAddr,ramExtendResLen_offset, 0x00,0x02);
		}

		return TRUE;
	}

	fileSize = GetTFSize(fileAddr);
	
	offset = ReadTFFileWord(fileAddr);//ReadTFBuf(fileAddr, 0x00, (BYTE *)&offset ,sizeof(WORD));
	
//	offset = BuildWord(&buf[0]);
	
	if(G_EarlyResponseTag != FALSE)  //?ù?¨?¨????μ?early responseoó???êDí·￠?í
	{
		return TRUE;
	}

	if(tag == TAG_BadFormat)
	{
		offset += 3;
		
		if((offset+7) >fileSize)  //response buffer2?1?ó?
		{
			return InsufficientMemory;
		}
	}
	
	if(tag == TAG_R_APDU)  //判断response buffer是否够用
	{
		if(resLength <= 0x7D)  //lgth <= 0x7F (包括两个字节的状态响应字节)
		{
			offset += resLength+1+ AAorAE;
		}
		else  if(resLength <= 0xFD)  // 0x80<=lgth  <=0xFF
		{
			offset += resLength+2+ AAorAE;
		}
		else
		{
			offset += resLength+3+ AAorAE;
		}
		
		if((offset+7) >fileSize)  //response buffer不够用
		{
			return InsufficientMemory;
		}
	}
	
	if(tag == TAG_BadFormat)   //update bad format TLV
	{
		tempBuf[0] = TAG_BadFormat;
		
		tempBuf[1] = 0x01;
		
		tempBuf[2] = RecData[0];
		
		offset = ReadTFFileWord(fileAddr);//ReadTFBuf(fileAddr, ramExtendResLen_offset, (BYTE *)&offset ,sizeof(WORD));
		
		WriteTFBuf(fileAddr, offset+2+AAorAE, &tempBuf[0],0x03);
		
		offset += 3;
		
	}
	else if((tag == TAG_ScriptChainingResponse) ||(tag == TAG_ImmediateActionResponse)) //table 5.15
	{		
		tempBuf[0] = tag;
		
		tempBuf[1] = 0x01;
		
		tempBuf[2] = RecData[0];
		
		WriteTFBuf(fileAddr, 2+AAorAE, &tempBuf[0],0x03);
		
		offset = 0x03;
	}

	//对于 TAG_ResponseRamExtendRes 来说，需要判断空间是否足够，目前暂时假设为空间足够

	if(tag == TAG_NumberCommandTLV)	 //该tag最后一次写入,  //23+L+response+sw
	{
		if(G_FormatAndType == FormatType_ExtendAA) //AA--AB  <ts_102226v090000p.pdf>  Table 5.10/Table 5.10a
		{
			tempBuf[0] = 0x80;
			
			tempBuf[1] = 0x01;
#if GP__AMENDMENT__B__FOR_MASK			
			//AB因为只有一次进入本函数，在http中，此时的命令执行个数应该是G_HTTPSStatus.HttpApduNum，缺陷编号260, 用例：电信Https能力要求/Chunked_64Apdu_definite_length_AA
			if(G_Source_Type == SourceType_HTTPS)
			{
				tempBuf[2] =  G_HTTPSStatus.HttpApduNum;
			}
			else
#endif			
			{
				tempBuf[2] = G_CurrentComNUM;
			}
			
			offset = ReadTFFileWord(fileAddr);//ReadTFBuf(fileAddr, 0x00, (BYTE *)&offset ,sizeof(WORD));
			
			//offset = BuildWord(&buf[0]);
			
			WriteTFBuf(fileAddr, 0x02, &tempBuf[0], 0x03);
			
			offset += 3;
		}
	}
	else if(tag == TAG_R_APDU)	
	{
		G_CAPDUResponse = TRUE; //标志已经执行过C_APDU 810182时候使用
		
		offset = ReadTFFileWord(fileAddr);//ReadTFBuf(fileAddr, 0x00, (BYTE *)&offset ,sizeof(WORD));
 		//offset = BuildWord(&buf[0]);
 		SetTFConst(fileAddr, offset+2+AAorAE, TAG_R_APDU,0x01); //写入 TAG_R_APDU
		
		if(resLength <= 0x7D)  //lgth <= 0x7F (包括两个字节的状态响应字节)
		{	
			SetTFConst(fileAddr, offset+3+AAorAE, resLength+2,0x01); 
			
			RecData[resLength]  = SW1;
			
			RecData[resLength+1]  = SW2;
			
			WriteTFBuf(fileAddr, offset +4+AAorAE, &RecData[0],resLength+2);
			
			offset += resLength+2 +2;
		}
		else  if(resLength <= 0xFD)  // 0x80<=lgth  <=0xFF
		{
			SetTFConst(fileAddr, offset+3+AAorAE, 0x81,0x01); 
			
			SetTFConst(fileAddr, offset+4+AAorAE, resLength+2,0x01);
			
			RecData[resLength]  = SW1;
			
			RecData[resLength+1]  = SW2;
			
			WriteTFBuf(fileAddr, offset+5+AAorAE, &RecData[0],resLength+2);
			
			offset += resLength+2 +3;
		}
		else 
		{		
			lgth = resLength +2;  //添加状态响应字节的长度
/*			
			SetTFConst(fileAddr, offset+6, 0x82,0x01);
			
			SetWord(&buf[0],lgth);
			
			WriteTFBuf(fileAddr, offset+7, &buf[0],sizeof(WORD));
*/
            tempBuf[0] = 0x82;         //TL改成统一按照大端写  

			tempBuf[1] = (lgth>>8)&0xFF;
			
			tempBuf[2] = lgth & 0xFF;
			
			WriteTFBuf(fileAddr, offset+3+AAorAE, &tempBuf[0],3);
			
			if((G_CurrentRemoteResponseLgth !=0x00)&&(G_CurrentRFMEfAddr != FALSE))   //RFM数据  这两个变量POR或者 执行前都要清空
			{
				fileSize = GetTFSize(fileAddr);
		
				//VMCopyDataToAddr(fileAddr+sizeof(TranFileHead)+offset+6+AAorAE, fileSize-(offset+6+AAorAE), G_CurrentRFMEfAddr, resLength, MODE_PROTECT_WRITE);
				VMCopyDataToAddr(fileAddr,offset+6+AAorAE,0,G_CurrentRFMEfAddr,G_CurrentRFMEfOffset, resLength, MODE_PROTECT_WRITE,1,1);// modify by xuweihua@20220514 同时兼容配置区地址、文件头体分开、文件头体连接这三种情况下数据的移动
			}
			else                                                                      //CASDGetData时候缓存的数据  RAM时候  获取CA证书时候长度超过FF,但是数据还是存放在RecData 后面 直接取
			{
				WriteTFBuf(fileAddr, offset +6+AAorAE, &RecData[0],resLength);
			}

			SetTFConst(fileAddr, offset +6+AAorAE+resLength, SW1,0x01);
			
			SetTFConst(fileAddr, offset +6+AAorAE+resLength+1, SW2,0x01);
			
			offset += resLength+2 +4;
		}				
	}
	else
	{
		//	白盒测试添加  tag仅存在3种值  tag == TAG_ClearRamExtendRes 的情况已经在前面返回 不会走到此处
	}
	
//	SetWord(&buf[0], offset);
	
	WriteTFFileWord(fileAddr, offset);//WriteTFBuf(fileAddr, 0x00, (BYTE *)&offset, sizeof(WORD));
	
#if UICC_SUPPORT_CHAGE_DELIVER_POR
	//por deliver
	if((G_BakCommandNumber==0x00)&&(G_Source_Type == SourceType_SMS)&&(tag == TAG_R_APDU)&&(offset > POR_DATA_MAX_DELIVER))
	{
		G_BakCommandNumber = G_CurrentComNUM;
	}
#endif
	return TRUE;
}
/*
*@description:RemoteApduResult执行APDU后执行结果保存
*
*@param      :fileAddr:待处理数据的文件地址
*@param      :commandOffset: 命令数据在文件中的起始偏移
*@param      :dataLen  :待处理的命令数据的长度
*            :ReMainLen:剩余数据长度
*            ：RemoteResult APDU执行结果
*
*@return     :TRUE 执行成功/FALSE 执行失败
*
*@history    :create by LCZ @20181115
*            :modify by xuxiaojun@20200710 同步5G代码
*            :modify by xuxiaojun@20201120 get status指令时，APDU头会被JAVA层冲掉
*/
BYTE RemoteApduResult(UWORD fileAddr,WORD commandOffset,WORD dataLen,WORD ReMainLen,BYTE RemoteResult, BYTE ins)
{
	BYTE idata i,le,result,tag;
	
	G_CurrentRemoteResponseLgth = 0x00;          //恢复默认初始值
	
	if(RemoteResult != POR_OK)
	{
		return ExtendedRFMProcessBadTLV(FLAG_TagNotFound);
	}
	
	i=0x00;
	
	if((ins != 0xB0)&&(ins != 0xB2)&&(ins != 0xC0))
	{
		i= P3;
	}
	
	//if((STKPerform[0] == TRUE)&&(SW1 == 0x91)) //10222系列测试脚本  RAM_RFM_EXPANDEDREMOTE_TEST  RFM_ExpandedRemoteCmd_2_12
	if((STKPerform == TRUE)&&(SW1 == 0x91)) 
	{
		GlobalSetStatus(0x9000);  //扩展指令处理STK中 执行00A40C0002 3F00返回状态字为911A,期望返回状态字为0x9000  如果把91XX当成正常状态字处理，在POR时候还是要改成9000，故在这里修改
	}
	
	tag = 0x00;      //默认指令是指向成功 状态字成功
	
	if((SW1 != 0x61)&&(SW1 != 0x90)&&(SW1 != 0x9F))
	{
		if((SW1 ==0x62)&&(SW2 == 0x83))
		{
			//警告 下面A4选择时候处理
		}
		else if((SW1 ==0x62)&&(SW2 == 0xF1))
		{
			//http CA时候返回数据长度 过长  暂时用这个状态字   短信也是可能的
		}
		else if((SW1 ==0x63)&&(SW2 == 0x10))
		{
			//get status
		}
		else
		{
			tag = 0x01;            //有指令指向失败
			
			ReMainLen = 0x00;
			if((G_ErrorActionOffsetLgth[0] != 0x00) || (G_ErrorActionOffsetLgth[1] != 0x00))    //有82tag   8200也算执行 10222系列测试脚本  RFM_ExpandedRemoteCmd_3_11
			{
				G_CurrentComNUM ++;
				
				IF_AnnexB_ChangeComNUM(ComNUM_INCREASE);	//HttpApduNum 跟随G_CurrentComNUM递增
			}
	   }
	}
	
	if((i +5)<dataLen)     //指令完整 且 AA0a 2208A0A40000023F0000  如果指令不完整 AA12 2210   80F240021B 4F05A0000005595C024F53  则不会存在Le字节  i+5 >dataLen  指令完整  AA09 2207A0A40000023F00  则 i+5 = dataLen
	{
//		ReadTFBuf(fileAddr, commandOffset+5+i, &le,sizeof(BYTE));  //读取le字节,若存在的话
		if(G_Source_Type == SourceType_HTTPS)
		{
			ReadEeprom(&le,fileAddr+commandOffset+5+i, 0x01);
		}
		else
		{
			ReadTFBuf(fileAddr, commandOffset+5+i, &le, 0x01);  //读取le字节,若存在的话
		}
	}
	
	if(tag == 0x01)      //执行执行失败
	{
		G_CurrentRemoteResponseLgth = 0x00;
	}
	else if(G_Data_Type == DataType_RFM)
	{
		if((ins == 0xB0)||(ins == 0xB2))
		{
			if((SW1 == 0x90)&&(SW2 == 0x00))
			{
				if(ins == 0xB0)
				{									
					if(P3 == 0x00)  //返回剩余所有数据
					{
						G_CurrentRemoteResponseLgth = GetTFSize(G_CurrentEfAddr[G_ChannelNum])- (WORD)((((WORD)P1 << 8)&0xFF00 )|P2); 
						
						//G_CurrentRFMEfAddr = G_CurrentEfAddr[G_ChannelNum] + sizeof(TranFileHead)+ (WORD)((((WORD)P1 << 8)&0xFF00 )|P2); //保存实际读取数据开始地址												
						G_CurrentRFMEfAddr = G_CurrentEfAddr[G_ChannelNum]; // modify by xuweihua@20220514
						G_CurrentRFMEfOffset = (WORD)((((WORD)P1 << 8)&0xFF00 )|P2);
					}
					else
					{
						G_CurrentRemoteResponseLgth = P3; 
					}										
				}
				else
				{
					G_CurrentRemoteResponseLgth = P3;
				}
			}
			else
			{
				G_CurrentRemoteResponseLgth = (((SW1 == 0x62)&&(SW2 == 0xF1))?0x100:0x00);   //,暂时只支持最大256字节的读取.the data is truncated in the response,This shall terminate the processing of the command list.
				
			}

		}
		else if((ins == 0x32)||(ins == 0xA2)||(ins == 0xA4))  //increase OR SEEK OR SELECT
		{
			if((SW1 == 0x61)||(SW1 == 0x9F)||(SW1 == 0x90))  //指令执行成功
			{
				if((i + 5 ) >= dataLen)  //不存在le字节 对于AE tag返状态字 AAtag不需要返数据  最后一条    如果指令不完整 AA12 2210   80F240021B 4F05A0000005595C024F53  则不会存在Le字节  i+5 >dataLen  指令完整  AA09 2207A0A40000023F00  则 i+5 = dataLen 
				{
					G_CurrentRemoteResponseLgth = 0x00;  

					//电信和移动测试，无LE 字节的情况下，要求返回9000，不返回61XX， by xwh 20210512
					GlobalSetStatus(0x9000);
				}
				else
				{
					if(SW1 == 0x90)   //当前无响应数据，命令执行成功
					{
						G_CurrentRemoteResponseLgth = 0x00;
					}
					else if(le == 0x00)  //对所有的case 2 or case 4指令，返回所有的有效响应数据
					{
						G_CurrentRemoteResponseLgth = SW2;
						
						GlobalSetStatus(0x9000);
					}		
					else if(le <= SW2)
					{
						G_CurrentRemoteResponseLgth = le;
						
						GlobalSetStatus(0x9000);
					}
					else
					{
						G_CurrentRemoteResponseLgth = 0x00;
						
						SW1 = 0x6C;   //期望获取的数据长度过长,返回响应状态字		
					}
				}
			}
			else if((ins == 0xA4)&&(SW1 == 0x62)&&(SW2 == 0x83))   //选择的文件为失效文件，没有le字节则继续执行
			{
				if((i + 5 ) >= dataLen)  //不存在le字节 对于AE tag返状态字 AAtag不需要返数据  最后一条  如果指令不完整 AA12 2210   80F240021B 4F05A0000005595C024F53  则不会存在Le字节  i+5 >dataLen  指令完整  AA09 2207A0A40000023F00  则 i+5 = dataLen
				{
					G_CurrentRemoteResponseLgth = 0x00;  
				}
				else
				{
					if(le == 0x00)  //对所有的case 2 or case 4指令，返回所有的有效响应数据
					{
						G_CurrentRemoteResponseLgth = G_CurrentReponse[G_ChannelNum];
						
					}		
					else if(le <= G_CurrentReponse[G_ChannelNum])
					{
						G_CurrentRemoteResponseLgth = le;
					}
					else
					{
						G_CurrentRemoteResponseLgth = 0x00;
						
						SW1 = 0x6C;   //期望获取的数据长度过长,返回响应状态字	
						SW2 = G_CurrentReponse[G_ChannelNum];	
					}
				}
				
				if(((ReMainLen != 0x00)&&(G_FormatAndType == FormatType_ExtendAA)) || ((ReMainLen != 0x02)&&(G_FormatAndType == FormatType_ExtendAE))) //不是最后一条指令
				{
					if(G_CurrentRemoteResponseLgth != 0x00)
					{
						SW1 = 0x61;   	
						SW2 = G_CurrentReponse[G_ChannelNum];
					}
					else
					{
						//5g联通https测试，天谕脚本， 当选择的文件失效时，期望抛错6283 (指令AE802209 00A4080C04 7F106F3A 2205 0044000000 0000 期望响应 AF80 23026283 23029000 0000) 用例： 联通测试第三项/3-3-Y/国密卡测试数据https/RFM测试Input command/06rehablitate_N002
						if(JcvmConfig_GetOPerator() != OPERATOR_CHINAUNICOM)
						{
							GlobalSetStatus(0x9000);  //文件状态是失效的  A4+44指令期望执行成功
						}
					}
				}
			}
			else
			{
				G_CurrentRemoteResponseLgth = 0x00;    //指令执行失败
			}
		}
		else
		{
			if(SW1 != 0x90)
			{
				G_CurrentRemoteResponseLgth = 0x00;    //指令执行失败
			}
				
		}	
	}
	else
	{
		if((ins == 0xCA)||(ins == 0xCB))  //Get data 和RFM的读文件一样 不带Le也是要返回数据
		{
//			if((SW1 ==0x62)&&(SW2 == 0xF1)) //http CA时候返回数据长度超过一个字节 不能用61XX状态字表示  短息也是可能 去掉Http判断
//			{
//				
//				G_CurrentRemoteResponseLgth = IF_GetRAMResWORDLen(); 
//					
//			}
//			else
			{				
				G_CurrentRemoteResponseLgth = SW2;
			}

			GlobalSetStatus(0x9000);			
		}
		else
		{

// 			if(tag == 0x01)      //执行执行失败
// 			{
// 				G_CurrentRemoteResponseLgth = 0x00;
// 			}
// 			else
// 			{
				
				if((i + 5 ) >= dataLen)   //不存在Le  如果指令不完整 AA12 2210   80F240021B 4F05A0000005595C024F53  则不会存在Le字节  i+5 >dataLen  指令完整  AA09 2207A0A40000023F00  则 i+5 = dataLen
				{
					G_CurrentRemoteResponseLgth = 0x00;
					
					if(JcvmConfig_GetOPerator() == OPERATOR_CHINATELECOM)
					{
						GlobalSetStatus(0x9000);
					}

				}
				else
				{
					if(le == 0x00)  //对所有的case 2 or case 4指令，返回所有的有效响应数据
					{
						//G_CurrentRemoteResponseLgth  = IF_GetRAMResWORDLen(); //SW2;

						if((SW1 == 0x63)&&(SW2 == 0x10))
						{
							// 6310
						}
						else
						{
							GlobalSetStatus(0x9000);
						}
					}
					else if(le <= SW2)
					{
						G_CurrentRemoteResponseLgth = le;
						
						if((SW1 == 0x63)&&(SW2 == 0x10))
						{
							// 6310
						}
						else
						{
							GlobalSetStatus(0x9000);
						}
					}
					else
					{
						G_CurrentRemoteResponseLgth = 0x00;
						
						SW1 = 0x6C;   //期望获取的数据长度过长,返回响应状态字		
					}
				}
// 			}


 		}
		
	}
	
	if((G_FormatAndType == FormatType_ExtendAE)&&(ReMainLen == 0x02))
	{
		ReMainLen -= 0x02;
	}
	
	if((G_FormatAndType == FormatType_ExtendAE) || ((G_FormatAndType == FormatType_ExtendAA)&&(G_CurrentRemoteResponseLgth!=0x00)) || (ReMainLen == 0x00))
	{
		result = GPRAMRFMExtendResponsePublic(TAG_R_APDU);  //对于AE tag  每个C-APDU都回复R-APDU/对于AA tag Case 2/Case 4才回复 R-APDU，但是最后一条是C-APDU时候都要回复
		
		if(result != TRUE)
		{
			GPRAMRFMExtendResponsePublic(TAG_NumberCommandTLV);  //更新指令条数tag
			
			return InsufficientMemory;
		}
	}
	
	if(tag == 0x01)
	{
		result = GPRAMRFMExtendResponsePublic(TAG_NumberCommandTLV);  //更新指令条数tag
		
		return InsufficientMemory;       //指令执行失败  结束  RFM_ExpandedRemoteCmd_2_16
	}
	
	return POR_OK;

}

/*
*@description:处理扩展型指令中的84 tag/EUICC特有
*
*@param      :WORD lgth,WORD offset
*
*@return     :TRUE/FALSE
*
*@history
*/
#if M2M_EUICC
/*
*@funtion    :PaddingScp03tExtendRes
*
*@description:拼装执行扩展格式的RAM指令中84/85 tag 的响应
*
*@param      :fileAddr	--	响应存放的地址
*							resDataLen -- 执行84/85 tag 后的响应数据的长度(响应数据放在ResData中)
*
*@return     :void
*
*@author     :duan ss
*
*@history    :V1.0 duan ss @20170623      create
*/
void PaddingScp03tExtendRes(BYTE resDataLen)
{
	UWORD xdata fileAddr;
	WORD	idata resLgth;
	BYTE    idata AAOrAE;
//	BYTE	idata buf[2];

	if(G_FormatAndType  == FormatType_ExtendAA)
	{
		AAOrAE = 0x03;
	}
	else
	{
		AAOrAE = 0x00;
	}

	if (G_Source_Type == SourceType_HTTPS)
	{
		fileAddr = SelectPrivateTFFile(DF_TLS, EF_HTTPSResponseData);
	}
	else
	{
		fileAddr = SelectPrivateTFFile(DF_Toolkit,EF_RAMExtendResponse);  //lv 结构，前2字节为L ,
	}


	resLgth = ReadTFFileWord(fileAddr);//ReadTFBuf(fileAddr, 0x00, (BYTE *)&resLgth ,sizeof(WORD));

#if GP__AMENDMENT__B__FOR_MASK
	if (G_Source_Type == SourceType_HTTPS)
	{
		WriteTFBuf(fileAddr, 2+resLgth, &ResData[0], resDataLen);
	}
	else
#endif
	{
		WriteTFBuf(fileAddr, AAOrAE+2+resLgth, &ResData[0], resDataLen);										//这里 +5 表示流出L的两个字节，以及后面要补充的 80 L V 三个字节
	}

	resLgth += resDataLen;

	WriteTFFileWord(fileAddr, resLgth);//WriteTFBuf(fileAddr, 0x00, (BYTE *)&resLgth, sizeof(WORD));
}

BYTE Process84(UWORD fileAddrEnHander,WORD lgth,WORD offset,BYTE sdId)
{
	BYTE	idata resDataLen;
	
	if(lgth != 0x0C) 
	{
		ResData[0] = 0x9F; //In case of an error, tag '9F44' is used.
		ResData[1] = 0x44; 
		ResData[2] = 0x01; 
		ResData[3] = 0x01; //"01": error in length or structure of command data
	}
	else
	{
		
		if(G_Source_Type == SourceType_HTTPS)
		{
			ReadEeprom(&RecData[0], fileAddrEnHander + offset, lgth);
		}
		else
		{
			ReadTFBuf(fileAddrEnHander, offset, &RecData[0], lgth);	
		}

		IF_M2M_ISDP_Process84(sdId, &RecData[0], lgth, &ResData[0]);
	}

	if (ResData[0] == 0x84)
	{
		resDataLen = GetTlvTandLBytes(&ResData[0]) + GetTlvLValue(&ResData[0]);
		
		PaddingScp03tExtendRes(resDataLen);
	}
	else
	{
		resDataLen = 0x04;
		
		PaddingScp03tExtendRes(resDataLen);
		
		return CipheringError;
	}

	return POR_OK;
}

/*
*@description:处理扩展型指令中的85 tag/EUICC特有
*
*@param      :WORD lgth,WORD offset
*
*@return     :TRUE/FALSE
*
*@history
*/
BYTE Process85(UWORD fileAddrEnHander,WORD lgth,WORD offset,BYTE sdId)
{
	BYTE	idata resDataLen;
	
	if(lgth != 0x13)
	{
		ResData[0] = 0x9F; //In case of an error, tag '9F45' is used.
		ResData[1] = 0x45; 
		ResData[2] = 0x01; 
		ResData[3] = 0x01; //"01": error in length or structure of command data
	}
	else
	{
		if(G_Source_Type == SourceType_HTTPS)
		{
			ReadEeprom(&RecData[0], fileAddrEnHander + offset,lgth);
		}
		else
		{
			ReadTFBuf(fileAddrEnHander, offset, &RecData[0], lgth);	
		}

		IF_M2M_ISDP_Process85(sdId, &RecData[0], lgth, &ResData[0]);
	}

	if (ResData[0] == 0x85)
	{
		resDataLen = GetTlvTandLBytes(&ResData[0]) + GetTlvLValue(&ResData[0]);
		
		PaddingScp03tExtendRes(resDataLen);
	}
	else
	{
		resDataLen = 0x04;
		
		PaddingScp03tExtendRes(resDataLen);
		
		return CipheringError;
	}
		
	//IF_PEAnayleGlobalVariableInit(); //解析profile之前进行全局变量的初始化操作

	return POR_OK;
}
/*
*@description:处理扩展型指令中的86 tag/EUICC特有
*
*@param      :WORD lgth,WORD offset
*
*@return     :TRUE/FALSE
*
*@history
*/
BYTE Process86(UWORD fileAddrEnHander,WORD lgth,WORD offset,BYTE sdId)
{
	BYTE	idata resDataLen;
	
//	G_DownLoadingISDPAppId = sdId;

	if(G_Source_Type == SourceType_HTTPS)
	{
		IF_M2M_ISDP_Process86(sdId, fileAddrEnHander+offset, lgth, &ResData[0]);
	}
	else
	{
		IF_M2M_ISDP_Process86(sdId, fileAddrEnHander+sizeof(TranFileHead)+offset, lgth, &ResData[0]);
	}
	if (ResData[0] == 0x86)
	{
		resDataLen = GetTlvTandLBytes(&ResData[0]) + GetTlvLValue(&ResData[0]);
		
		if((ResData[1] != 0)&&(IF_GetPfoileExcuteState()==INIT_STATE)) //执行出错
		{						
			PaddingScp03tExtendRes(resDataLen);
			
			G_86errorNum++;
			
			return CipheringError;
		}
		else if(ResData[1] != 0)    //执行成功
		{
			G_86errorNum++;
			
		}
		else
		{
			
		}
	}
	else //9F46的情况
	{
		resDataLen = 0x04;
		
		PaddingScp03tExtendRes(resDataLen);
		
		G_86errorNum++;
		
		return CipheringError;
	}				
					
	PaddingScp03tExtendRes(resDataLen);		

	return POR_OK;
}
#endif
/*
*@description:处理扩展型指令中的83 tag /Script Chaining TLV objects
*
*@param      :WORD lgth,WORD offset
*
*@return     :TRUE/FALSE
*
*@history
*          V1.1;添加G_ChainingTagRec变量的赋值判断 用于Http下来的数据带了83tag 处理了83tag 后由于断数据 再次进入102226时候没有带83tag情况
*          V1.2: modify by yangjian@20221205: 删除未使用的参数sdId
*/
BYTE Process83(UWORD fileAddrEnHander,WORD lgth,WORD offset)
{
	BYTE idata chainingValue,result,profileindex;
	UWORD xdata fileAddr;

//先注释编译  SCP03还没有   SCP03T只有ISD-P时候84  85  86 Tag使用	
	if(G_CurrentComNUM != 0x01)  //chaining tag invalid;If present, the Script Chaining TLV shall be present only once and shall be the first Command TLV in the Command Script
	{
		result =  ExtendedRFMProcessBadTLV(FLAG_TagNotFound);

		return CipheringError;
	}
	
	if(lgth != 0x01)  
	{
		RecData[0] = 0x03; //Script Chaining Length error,Unable to process script chaining
		
		GPRAMRFMExtendResponsePublic(TAG_ScriptChainingResponse);
		
		GPRAMRFMExtendResponsePublic(TAG_NumberCommandTLV);
		
		return CipheringError;
	}
	
	G_ChainingTagRec = TRUE;
	
	fileAddr = SelectPrivateTFFile(DF_Toolkit,EF_ScriptChainingFile);
	
	ReadTFBuf(fileAddr, 0x00, (BYTE *)&ChainingValue,sizeof(ChainingValue));
	
//	ReadTFBuf(fileAddrEnHander, offset, (BYTE *)&chainingValue, sizeof(BYTE));
	if(G_Source_Type == SourceType_HTTPS)
	{
		ReadEeprom((BYTE *)&chainingValue, fileAddrEnHander + offset, sizeof(BYTE));
	}
	else
	{
		ReadTFBuf(fileAddrEnHander, offset, (BYTE *)&chainingValue, sizeof(BYTE));
	}
	
	if((chainingValue == 0x02)||(chainingValue == 0x03))
	{
		result = 0x00;
		if(G_Data_Type == DataType_RFM)
		{
			if(((ChainingValue.firstScriptValue != 0x01)&&(ChainingValue.firstScriptValue != 0x11))||((ChainingValue.previousValue != 0x02)&&(ChainingValue.previousValue != 0x01)&&(ChainingValue.previousValue != 0x11)))
			{
				result = 0x01;
			}
		}
		else
		{
			if(G_CurrentChainingValueTag == FALSE)
			{
				result = 0x01;
			}

		}

		if(result == 0x01)
		{
			RecData[0] = 0x01;    //No previous script
		
			GPRAMRFMExtendResponsePublic(TAG_ScriptChainingResponse);
			
			GPRAMRFMExtendResponsePublic(TAG_NumberCommandTLV);
			
			return CipheringError;
		}
	}
		
	if((chainingValue == 0x01)||(chainingValue == 0x11))  //first script chaining
	{
		if(G_Data_Type == DataType_RFM)
		{
			memset((BYTE *)&ChainingValue, 0x00, sizeof(ChainingValue));  //清空结构体缓存
			
			ChainingValue.firstScriptValue = chainingValue;
		}
		else if((G_Data_Type == DataType_RAM))
		{
			profileindex = ILLEGAL_PROFILEINDEX;

#if M2M_EUICC
			profileindex = IF_GetRAMProfileIndex();
#endif
			if(profileindex == ILLEGAL_PROFILEINDEX)//非profile下载过程  来83时清掉会话
			{
				GPClearSecureSessionInfo(/*G_RAMSessionSdId*/);
			}
#if (EUICC_ONLY||SE_AND_EUICC)
			else
			{
			  //为了兼容 先来 84xx 85xx tag再来83010186xx的情况 (SGP.02_v3.2_updated.pdf的Annex K : Script Chaining implementation (Informative))
			              //改为调用IF_GPClearSecureSCP03TSessionInfo

			 //增加配置 CARDCFG_KeepChaining 如果配置上就清空会话SCP03t会话信息


				//ReadEeprom(&result,CARDCFG_KeepChaining, 0x01);	    //HB_Offline_Test\3Profile Download指令异常测试\Tele_profile_download_Error_test_01

				//if(result == M2M_NoKeepSCP03tChaining)
				if(ValueBit(JcvmConfig_GetConfigUicc(), BIT_UICCConfig_ClearSessionWhen83) == CLEAR_SESSION_WHEN83)  //HB_Offline_Test\3Profile Download指令异常测试\Tele_profile_download_Error_test_01
				{
					GPClearSecureSessionInfo(/*G_RAMSessionSdId*/);
				}
			}
#endif
		}
		else
		{
		}
	}
	else
	{
		if(G_Data_Type == DataType_RFM)
		{
			G_CurrentDfAddr[G_ChannelNum] = ChainingValue.dfAddr;  //获取当前filecontext
			
			G_CurrentEfAddr[G_ChannelNum] = ChainingValue.efAddr;
			
			G_CurrentDfName[G_ChannelNum] =  ChainingValue.dfName;
			
			G_CurrentEfName[G_ChannelNum] =  ChainingValue.efName;
			
			G_CurrentRecNum[G_ChannelNum] = ChainingValue.recNum;
			
			G_ChvStatus =  ChainingValue.pinStatus;
		}
	}
	
	if(G_Data_Type == DataType_RFM)
	{
		ChainingValue.previousValue = chainingValue;
	}
	
	if(chainingValue == 0x03)
	{
		G_CurrentChainingValueTag = FALSE;
		
		if(G_Data_Type == DataType_RFM)
		{
			SetTFConst(fileAddr, 0x00,0x00,sizeof(ChainingValue));  //当前script chaining为最后一条
		}
	}
	else
	{	
		G_CurrentChainingValueTag = TRUE;   //第一条指令为正确的script chaining
	}
	
	return POR_OK;
}
/*
*@description:执行扩展型指令中的 82 tag/ERROR ACTION TLV
*
*@param      :lgth ,offset
*
*@return     :
*
*@history
*/
BYTE Process82(UWORD HandlerfileAddr)
{
	UWORD xdata fileAddr;

	if(G_ErrorActionOffsetLgth[0] == 0x00)  //no action shall be performed
	{
		return TRUE;
	}
	else if(G_ErrorActionOffsetLgth[0] == 0x01)  // referenced format,,卡片暂不支持文件EFRMA.不执行任何操作
	{
		if(G_Source_Type == SourceType_HTTPS)
		{
			ReadEeprom(&ResData[0], HandlerfileAddr+G_ErrorActionOffsetLgth[1], 0x01);
		}
		else
		{
			ReadTFBuf(HandlerfileAddr,G_ErrorActionOffsetLgth[1],&ResData[0],0x01);  //获取主动式命令
		}
	
		fileAddr = SelectTFFile(DF_TELECOM,0x6F53);  //102226  Table 5.7: Error Action TLV - referenced format
		
		if(fileAddr == FALSE)
		{
			return ExtendedRFMProcessBadTLV(FLAG_TagNotFound);  //这里应该怎么处理？
		}
		
		//ResData[2] = ReadLFFileValue1ByteLen(fileAddr,ResData[0],0x00,&ResData[3]);

		ResData[2] = ReadLFFileValue1ByteLen(fileAddr,ResData[0],0x00,&ResData[3]);

		
		ResData[1] = 0xD0;
		
	    if((ResData[6] != 0x21)&&(ResData[6] != 0x20))  //支持的主动式命令即不是displaytext也不是playtone ,不执行任何动作
		{
			return TRUE;
		}
		
		SendProactiveCommand();  //发送主动式命令


	}
	else
	{		
		if(G_Source_Type == SourceType_HTTPS)
		{
			ReadEeprom(&ResData[3], HandlerfileAddr+G_ErrorActionOffsetLgth[1], G_ErrorActionOffsetLgth[0]);
		}
		else
		{
			ReadTFBuf(HandlerfileAddr,G_ErrorActionOffsetLgth[1],&ResData[3],G_ErrorActionOffsetLgth[0]);  //获取主动式命令 V
		}
		
		ResData[2] = G_ErrorActionOffsetLgth[0];    // L
		
		ResData[1] = 0xD0;
		
		ResData[0] = 0xD0;
	
		if((ResData[6] != 0x21)&&(ResData[6] != 0x20))  //支持的主动式命令即不是displaytext也不是playtone ,不执行任何动作
		{
			return FALSE;
		}
		
		SendProactiveCommand();  //发送主动式命令
		
	}
	return TRUE;
}
/*
*@description :  处理扩展格式中的81 tag  immediate Action TLV
* 
*@param       :  lgth,offset
*
*@return      : TRUE:继续执行后续指令/FALSE:退出,返回POR,若earlyResponse存在,卡片发送完POR 之后需复位
*
*             :
*@history     :
*/
BYTE Process81(UWORD fileAddrEnHander,WORD lgth ,WORD offset)
{
	UWORD xdata fileAddr;
	
	if((lgth > 0x01)&&(lgth <9))  //主动式命令的长度最短必须为11字节  ,否则长度错误   ，改为9  去掉D0 len
	{
		ExtendedRFMProcessBadTLV(FLAG_WrongLgth);

		return CipheringError;
	}
		
	if(lgth == 0x00)  //当L为0时,无主动式命令执行
	{
		return POR_OK;
	}
	
	if(lgth == 0x01)  //refernce format
	{
//		ReadTFBuf(fileAddrEnHander,offset,&ResData[0],lgth);  //获取immediate action TLV 中的V值  
		if(G_Source_Type == SourceType_HTTPS)
		{
			ReadEeprom(&ResData[0], fileAddrEnHander+offset, lgth);
		}
		else
		{
			ReadTFBuf(fileAddrEnHander,offset,&ResData[0],lgth);  //获取immediate action TLV 中的V值  
		}
		
		if(ResData[0] == 0x81)       //Proactive session indication
		{
			if(G_CurrentChainingValueTag == FALSE)  //The first Command TLV in the script if there is no script chaining
			{
				if(G_CurrentComNUM != 0x01)  //该tag出现在这里是错误的情况
				{
					return ExtendedRFMProcessBadTLV(FLAG_TagNotFound);

				}
			}
			else                                    //The second Command TLV in the script if there is script chaining.
			{
				if(G_CurrentComNUM != 0x02)  //该tag出现在这里是错误的情况
				{
					return ExtendedRFMProcessBadTLV(FLAG_TagNotFound);
				}
			}	
			
			//if(STKPerform[0] == TRUE)  //暂时根据STKPerform判断当前是否正在处理主动式命令,处理不能suspend the script execution,的情况
			if(STKPerform == TRUE)
			{
				RecData[0] = Value_SuspensionError;  //Suspension error
				
				GPRAMRFMExtendResponsePublic(TAG_ImmediateActionResponse);

				GPRAMRFMExtendResponsePublic(TAG_NumberCommandTLV);
				
				return CipheringError;
			}
			
			return  POR_OK;  
		}
		else if(ResData[0] == 0x82)  //Early response,保存当前响应内容，继续执行后续指令
		{
			//GPRAMRFMExtendResponsePublic(TAG_R_APDU);
			
			//GPRAMRFMExtendResponsePublic(TAG_NumberCommandTLV);
			
			if((G_FormatAndType == FormatType_ExtendAA)&&(G_CAPDUResponse == 0x00))    //AA tag这时候才需要写响应数据,AE tag每次执行都会写,到这里时候不需要重写
			{
				GPRAMRFMExtendResponsePublic(TAG_R_APDU);
			}
			
			GPRAMRFMExtendResponsePublic(TAG_NumberCommandTLV);
			
			G_EarlyResponseTag = TRUE;
				
			return POR_OK;
		}
		else                          //RFU
		{
			fileAddr = SelectTFFile(DF_TELECOM,0x6F53);  //102226  Table 5.7: Error Action TLV - referenced format
			
			if(fileAddr == FALSE)
			{
				return ExtendedRFMProcessBadTLV(FLAG_TagNotFound);  //这里应该怎么处理？
			}
			
			//if(STKPerform[0] == TRUE)     //If no "proactive session indication" is present as first Command TLV and another proactive session is ongoing,proactive commands in the script shall be silently ignored.
			if(STKPerform == TRUE)
			{
				return POR_OK;
			}
			
			ResData[2] = ReadLFFileValue1ByteLen(fileAddr,ResData[0],0x00,&ResData[3]);
			
			ResData[1] = 0xD0;
			
		   if((ResData[6] != 0x21)&&(ResData[6] != 0x20)&&(ResData[6] != 0x01))  //支持的主动式命令即不是displaytext也不是playtone REFRESH,不执行任何动作
			{
				return POR_OK;
			}
			
			SendProactiveCommand();  //发送主动式命令

		}
	}
	else
	{
	
		//if(STKPerform[0] == TRUE)       //If no "proactive session indication" is present as first Command TLV and another proactive session is ongoing,proactive commands in the script shall be silently ignored.
		if(STKPerform == TRUE) 
		{
			G_CurrentComNUM--;  //用例名称:  RAM_ExpandedRemoteCmd_2_12 -- TestPurpose: Immediate Action TLV:正常情况不存在Proactive session indication,SUBMIT，使用normal format，另外一个主动式命令正在进行，Immediate Action TLV中的主动式命令忽略
			
			IF_AnnexB_ChangeComNUM(ComNUM_DECREASE);	//HttpApduNum 跟随G_CurrentComNUM递减
			
			return POR_OK;
		}
		
//		ReadTFBuf(fileAddrEnHander,offset,&ResData[3],lgth);  //获取immediate action TLV 中的V值  这里直接使用G_EnvelopHanderAddr 因为在STK中 G_EnvelopHanderAddr地址会变化
		if(G_Source_Type == SourceType_HTTPS)
		{
			ReadEeprom(&ResData[3], fileAddrEnHander+offset, lgth);
		}
		else
		{
			ReadTFBuf(fileAddrEnHander,offset,&ResData[3],lgth);  //获取immediate action TLV 中的V值  这里直接使用G_EnvelopHanderAddr 因为在STK中 G_EnvelopHanderAddr地址会变化
		}
	
		ResData[0] = 0xD0;
		
		ResData[1] = 0xD0;
		
		ResData[2] = lgth;

		if((ResData[6] != 0x21)&&(ResData[6] != 0x20)&&(ResData[6] != 0x01))  //支持的主动式命令即不是displaytext也不是playtone REFRESH,不执行任何动作
		{
			return POR_OK;
		}
		
		if(ResData[6] == 0x01)   //主动式指令为refresh指令
		{
			if(G_EarlyResponseTag == TRUE)   //若之前有碰到early response 指令，则先发送response响应，再复位
			{
				G_EarlyResponseTag = 0x11;
				
				return POR_OK;
			}
		}
		
		SendProactiveCommand();  //发送主动式命令
	}
	
	return POR_OK;
}
/*
*@description:处理当前bad format TLV情况下更新responseFile的情况
*
*@param      :errorCode
*
*@return     
*
*@history
*/
BYTE ExtendedRFMProcessBadTLV(BYTE errorCode)
{
	
	RecData[0] = errorCode;
	
	GPRAMRFMExtendResponsePublic(TAG_BadFormat);		
	
// 	if(result == TRUE)   //解密数据错误
// 	{
		GPRAMRFMExtendResponsePublic(TAG_NumberCommandTLV);
		return CipheringError;
// 	}
// 	else   //空间不足的情况，在主函数里统一更新当前指令执行条数
// 	{
// 		return NoMemory;
// 	}
}

/*
*@funtion    :ProcessExtendFun
*
*@description:解释扩展格式指令 和拼装响应
*
*@param      :fileAddr	--	命令的地址
*							dataOffset	--	命令的偏移
*							dataLen  -- 命令的长度
*
*@return     :void
*
*@author     :LCZ
*
*@history    :V1.0 LCZ @20181115      create
*
*		    :modify by jiangqignbin-20210513	退出主循环apdu分派执行后，优先把http情况时的执行指令个数增加完整，在进行相应数据拼装
*/
BYTE ProcessExtendFun(UWORD fileAddr,WORD commandOffset, WORD cplLen,BYTE mode)
{
	
	BYTE idata sdId,result,tag,tag83;
	BYTE xdata firstCpl[0x04];
	WORD xdata valuelgth,offset,cpLgth;
	
	BYTE idata RFMCurrentRecord,RFMCommandMode,RFMSFI;
	WORD idata RFMADFName;
	WORD idata fileName[4];     //前3备份DF name  4备份 EF name
	UWORD xdata ScriptChainfileAddr;
	BYTE    idata    RFMChvStatus;			//PIN1 status
	BYTE    idata    bakIns;
#if !SE_ONLY
	BYTE    xdata    RFMLocalPinStatus[8];			//PIN2 status
#endif
	WORD    idata    RFMAdmStatus;
	
	if(G_ChannelNum != 0x00)  //根据GSM11.11 TS102221，C2指令的CLA 只能为A0\80,因此一定是在通道0执行C2命令
	{
		return FALSE;
	}

	RFMCommandMode = G_CommandMode;

	//开机时不再初始化 EF_HTTPSResponseData/EF_RAMExtendResponse 文件，SMS 在进入扩展格式处理时初始化,by xwh 20210628
	if (G_Source_Type == SourceType_SMS)
	{
		GPRAMRFMExtendResponsePublic(TAG_ClearResponseFile);    //http断指令不能清空
	}
	
	memset(G_ErrorActionOffsetLgth, 0x00, sizeof(G_ErrorActionOffsetLgth));   //清空当前ErrorAction的标志字节
	
	G_CurrentComNUM = 0x00;        //扩展 / 压缩公用 指令执行个数变量  初值0
	G_CAPDUResponse = 0x00;      //没有执行过 R_APDU
	G_EarlyResponseTag = FALSE;
	G_CommandMode = Command_Mode_First;
	
	tag83 = 0x00;
	
	sdId = ILLEGAL_APPID; //初始化 去掉警告
	
	RFMSFI = G_CurrentSfi;
	
	RFMADFName  = G_CurrentAdfName[G_ChannelNum];//7FF0
		 
	RFMCurrentRecord = G_CurrentRecNum[G_ChannelNum];	//00
	
	if(cplLen<4)      //最小的完整数据 AE 80 0000     对于http则不需要移动数据此时
	{		
		return POR_OK;

	}
	
	offset = commandOffset;
	
//	G_FormatAndType = FormatType_NULL;//初值	
	
//	ReadTFBuf(fileAddr, offset, &firstCpl[0],sizeof(firstCpl));
#if GP__AMENDMENT__B__FOR_MASK	
	if(G_Source_Type == SourceType_HTTPS)
	{
		ReadEeprom(&firstCpl[0], fileAddr + offset, sizeof(firstCpl));
	}
	else
#endif		
	{
		ReadTFBuf(fileAddr, offset, &firstCpl[0],sizeof(firstCpl));
	}
	
	if(firstCpl[0] == TAG_CommandScriptingTemplateDefiniteLength)   //0xAA tag
	{
		G_FormatAndType = FormatType_ExtendAA;
		
		//cpLgth = GetBerTlvLength(&firstCpl[1],&offset);       //AA tag时候   AA15830101220700A4000C023F00220700A4000C027F10   TLV结构
		
		GetBerTlvLength(&firstCpl[1],&offset);       //要修改是因为对于Http的时候 AAtag后面的长度和cplLen不对应
		
		cpLgth  = cplLen-(offset - commandOffset);  //有效数据长度是跳过AA XX
		
	}
	else if(firstCpl[0] == TAG_CommandScriptingTemplateInDefiniteLength)                             //0xAE tag
	{
		G_FormatAndType = FormatType_ExtendAE;
		
		cpLgth = (cplLen-2);                                   // AE tag时候   AE80 220E8050300008079B66B25331E4DB000000        第一个L固定为0x80     最后面两字节0000 最为结束标志
		
		offset += 0x02;        //偏移过0x80这个固定字节
    }
	else                                                   //
	{
		ExtendedRFMProcessBadTLV(FLAG_TagNotFound);
		
		//IF_AnnexB_ProcessRemoteAppendDataForPor();
		
		//G_FormatAndType = FormatType_NULL;//初值	WQ_Offline_Test/WQ_Offline_Function_Test/TC_ES8_DownloadAndInstallation_HTTPS_ProfileData_Error_13  来了一个完整的AE8084XX85XX00,接着来Len86期望报错时候 AFtag回复  故 恢复值放到这里
		
		return CipheringError;
	}
	
	RFMChvStatus = G_ChvStatus;

#if !SE_ONLY
	memcpy(&RFMLocalPinStatus, &G_LocalPinStatus[0], 0x08);
#endif

	RFMAdmStatus = G_AdmStatus;
	
	SetBit(G_SystemStatus,B_RfmPerform);	//设置RFM 状态
#if UICC__FUNCTION__RFM	
	if(G_Data_Type == DataType_RFM)
	{
		ProcessRFMinit(fileName);
	}
	else
#endif		
	{
#if UICC__FUNCTION__RAM		
		if (G_Source_Type == SourceType_HTTPS)
		{
			sdId = HttpProcessRAMinit();
		}
		else
		{
			sdId = ProcessRAMinit();
		}
#endif
	}
#if UICC__FUNCTION__RFM	
	if(G_Source_Type == SourceType_HTTPS)
	{
		IF_AnnexB_HTTPRFM(HTTP_RFM_MODE_READ);
	}
#endif	
	result = FALSE;
	
	tag83 = G_ChainingTagRec;
	
	while(cpLgth > 0)
	{
		if(cpLgth<5)        //判断小于5 对于http来830101等的话  留到下一次执行 
		{
//			ReadTFBuf(fileAddr,offset,&G_APDU[0],0x02); 
#if GP__AMENDMENT__B__FOR_MASK				
			if(G_Source_Type == SourceType_HTTPS)
			{
				ReadEeprom(&G_APDU[0], fileAddr + offset, 0x02);
			}
			else
#endif				
			{
				ReadTFBuf(fileAddr, offset, &G_APDU[0],0x02);
			}
			
			if((cpLgth == 0x02)&&(G_FormatAndType == FormatType_ExtendAE))
			{
				if((G_APDU[0] == 0x00)&&(G_APDU[1] == 0x00))   //AE tag最后两个00
				{
					cpLgth -= 0x02; 
					break;
				}
			}
			
			if(G_APDU[0] != TAG_ImmediateAction) // 81tag的话 不需要留到下一次执行   如果tag不完整 则后面判断时候跳出
			{
#if GP__AMENDMENT__B__FOR_MASK		
			 if((G_Source_Type == SourceType_HTTPS)&&(mode != RemoteDataEndBlock))
			 {
				IF_ProcessRemoteMoveFalshForHTTP(fileAddr,offset,cpLgth);   //这个时候上一个 TLV执行完
				 
				break;
			 }
#endif
			}			 
		}
		
		G_CurrentComNUM++;
		
		IF_AnnexB_ChangeComNUM(ComNUM_INCREASE);	// HttpApduNum 跟随G_CurrentComNUM递增
		
#if GP__AMENDMENT__B__FOR_MASK	
		if(G_Source_Type == SourceType_HTTPS)
		{
			ReadEeprom(&tag, fileAddr+offset, 0x01);
		}
		else
#endif			
		{
			ReadTFBuf(fileAddr, offset, &tag,0x01);     //读取tag
		}
		
		if((tag != TAG_C_APDU)&&(tag != TAG_ImmediateAction)&&(tag != TAG_ErrorAction)&&(tag !=TAG_ScriptChaining)&&(tag!=0x84)&&(tag!=0x85)&&(tag!=0x86))//HB_Offline_Test/3Profile Download指令异常测试/Tele_profile_download_Error_test_01  有缺少tag测试
		{
			result = ExtendedRFMProcessBadTLV(FLAG_TagNotFound);
			break;
		}
		
		if((tag != TAG_ScriptChaining) && (G_CurrentChainingValueTag == TRUE) &&(G_CurrentComNUM == 0x01))   //RAM_ExpandedRemoteCmd_1_8   83tag间插了不带83tag 则清除状态
		{
			if(G_ChainingTagRec != TRUE)       //一次协议数据中 没有执行过83 tag
			{
				if(G_Data_Type == DataType_RAM)
				{

	//				memset(G_RAMGetStatus,0x00,sizeof(G_RAMGetStatus));

					//SCP03T只有ISD-P下载profile时候 84  85  86 tag使用
				
					GPClearSecureSessionInfo(/*G_RAMSessionSdId*/);

				}
				else
				{
					memset((BYTE *)&ChainingValue, 0x00, sizeof(ChainingValue));  //清空结构体缓存

					ScriptChainfileAddr = SelectPrivateTFFile(DF_Toolkit,EF_ScriptChainingFile);

					WriteTFBuf(ScriptChainfileAddr, 0x00, (BYTE *)&ChainingValue,sizeof(ChainingValue));
				}

					
				G_CurrentChainingValueTag = FALSE;
			}
		}

		if(G_Source_Type == SourceType_HTTPS)
		{
			ReadEeprom(&firstCpl[0], fileAddr+offset+1, 0x03);
		}
		else
		{
			ReadTFBuf(fileAddr, offset+1, &firstCpl[0],0x03); //读取L部分 XX / 81XX /82 XX XX
		}
		
// 		if(firstCpl[0] > 0x82)  //03 ,lgth not found
// 		{
// 			result =  ExtendedRFMProcessBadTLV(FLAG_WrongLgth);  //eUICC_M2M_SGP_11_v3_3_Card_Compliance_Test_Suite_v2_2_0/TC_SAR_5_SCP03T_ErrorManagement_03_Error_Case_Incorrect_Length_in_Ext_Auth  期望报错  900102
// 			
// 			break;
// 		}
		
		valuelgth = GetBerTlvLength(&firstCpl[0],&offset);

		firstCpl[0] =(((firstCpl[0]!=0x81)&&(firstCpl[0]!=0x82))?0x00:(firstCpl[0]-0x80));
		
		if(valuelgth > (cpLgth - (firstCpl[0]+2)))   // TLV 不完整 比如22 07 00a4000002
		{
			if((G_Source_Type == SourceType_HTTPS)&&(mode != RemoteDataEndBlock))  //表示当前是HTTP指令，需要从此处跳出该函数
			{	
#if GP__AMENDMENT__B__FOR_MASK				
				IF_ProcessRemoteMoveFalshForHTTP(fileAddr,offset-(firstCpl[0]+2),cpLgth);   //TLV不完整  从T开始返回
					 
				IF_AnnexB_ChangeComNUM(ComNUM_DECREASE);	// HttpApduNum 减1是因为当前的apdu指令头不完整，前面对HttpApduNum已进行+1，此处需要-1，返回http继续接收数据，然后再进来本函数处理指令
				break;
#endif
			}
			
// 			result =  ExtendedRFMProcessBadTLV(FLAG_LgthNotFound);  //HB_Offline_Test/3Profile Download指令异常测试/Tele_profile_download_Error_test_01  期望指令层出错
// 			
// 			break;
		}

		switch(tag)
		{
			case TAG_C_APDU:			//0x22
				
				if(valuelgth < 5)   //length error ,(02:wrong length found)
				{		
					//TODO:	以下代码存在极小概率的测试错误情况时的bug(暂时不修改)，因为此处肯定是指令格式非法，不存在数据未收齐的情况，假如http下行指令是 220700a40000023f00 2203010203 220700a40000027ff0, 在2203时期望报错,但是http有可能没有收完整的2203010203，此时break是跳不出while循环的bug
					if((G_Source_Type == SourceType_HTTPS)&&(mode != RemoteDataEndBlock))  //表示当前是HTTP指令，需要从此处跳出该函数
					{ 	
						IF_ProcessRemoteMoveFalshForHTTP(fileAddr,offset-(firstCpl[0]+2),cpLgth);   //TLV不完整  从T开始返回
							 
						break;
						
					}	
					
					result =  ExtendedRFMProcessBadTLV(FLAG_WrongLgth);
			
					break;
				}
#if GP__AMENDMENT__B__FOR_MASK					
				if(G_Source_Type == SourceType_HTTPS)
		   		{
					ReadEeprom(G_APDU, fileAddr+offset, 0x05);
				}
				else
#endif					
				{
					ReadTFBuf(fileAddr, offset, G_APDU, 0x05);	//获取APDU
				}
				
				bakIns = INS;
#if UICC__FUNCTION__RAM
				if(G_Data_Type == DataType_RAM)	
				{
				
//					if((G_RAMCAPInstallTag != CAPCOMPONENT_InvalidTag)&&(INS != 0xE8)&&(INS != 0xc0))  //此处无测试用例支撑，按照普通APDU的处理实现@2017118 by xuyz
//					{    // 中国移动NFC2.0项目20160415/GP2.2.1_UICC_ComplianceTests(移动版)-20141112/uicc_configuration_additional_test/MultiChannelTest_SECURED/MultiChannelTest_10_1、MultiChannelTest_10_2
//						if(G_RAMCAPInstallTag == sdId)// 中国移动NFC2.0项目20170116/GP2.2.1_UICC_ComplianceTests(移动版)-20141112/uicc_configuration_additional_test/MultiChannelTest_SECURED/MultiChannelTest_11_1 / Transaction 1:  ISD的RAM Load流程不会被另一个SD的其他非下载RAM操作打断
//						{
//							GPLoadInit();  //当前 RAM 正在执行load过程,打断
//						}
//
//					}

					//if( (bRamBuffer[INSTALL_FOR_LOAD_TO_SDID] == sdId) && (INS != 0xE8) && (INS != 0xc0))
					//{
						//bRamBuffer[INSTALL_FOR_LOAD_TO_SDID] = 0;

						//GPLoadInit();
					//}
				}
#endif				
				if(ProcessRemoteCheckCAPDU(cpLgth - (firstCpl[0]+2)) == FALSE)  //判断APDU是否完整
				{				
					if((G_Source_Type == SourceType_HTTPS)&&(mode != RemoteDataEndBlock))  //表示当前是HTTP指令，需要从此处跳出该函数
					{ 				
						IF_ProcessRemoteMoveFalshForHTTP(fileAddr,offset-(firstCpl[0]+2),cpLgth);   //TLV不完整  从T开始返回
							 
						IF_AnnexB_ChangeComNUM(ComNUM_DECREASE);	// HttpApduNum 减1是因为当前的apdu指令data域不完整，前面对HttpApduNum已进行+1，此处需要-1，返回http继续接收数据，然后再进来本函数处理指令	
						
						break;						
					}	
				}
#if UICC__FUNCTION__RFM
				if(G_Data_Type == DataType_RFM)
				{
					result = RFMApduProcess(fileAddr,offset, valuelgth);
				}
				else
#endif					
				{
#if UICC__FUNCTION__RAM					
					result = RAMApduProcess(fileAddr,offset, valuelgth);
#endif					
				}
				
				result = RemoteApduResult(fileAddr,offset,valuelgth, (cpLgth - (firstCpl[0]+2+valuelgth)),result, bakIns);
				
#if GP__AMENDMENT__B__FOR_MASK	//todo，该问题如何修改需要进一步讨论，中国电信native平台备案脚本发现的问题, http会话中当指令抛错后， 目前方案是不再执行后续的apdu
			 if (G_Source_Type == SourceType_HTTPS)
			 {
				 //经检查, tpx和epx都有以下问题，后续需要同步，包括RFM和RAM模式，简单格式和扩展格式
				 //目前只有http的apdu有可能不完整(包括apdu不完整和apdu完整但不是最后一块), resultState退出本函数时仅http对返回值判断;	//用例：通用AmendMentB测试/RFM/T_024_HTTP_Seg_RFM_USIM
				if ((SW1 != 0x90) && (SW1 != 0x9F) && (SW1 != 0x61) && (SW1 != 0x62) && (SW1 != 0x63))	//指令异常时该判断是否合理？
				{
					result = GPAnnexBError;	//此处赋值时为了本函数返回值用于外部函数判断APDU指令执行结果
					//break;
				}
			 }
#endif			 
				break;
				
			case TAG_ImmediateAction:		//0x81
			
				result = Process81(fileAddr,valuelgth ,offset);
				
				break;
				
			case TAG_ErrorAction:			//0x82
			
				if((valuelgth > 0x01)&&(valuelgth <9))  //主动式命令的长度最短必须为11字节  ,否则长度错误   改为 9  去掉D0 Len
				{
					result = ExtendedRFMProcessBadTLV(FLAG_WrongLgth);
			
					break;
				}

				G_CurrentComNUM --;     //只需要执行最近的一次82tag  之前的不算在总的执行条数里面?
	
				IF_AnnexB_ChangeComNUM(ComNUM_DECREASE);	//HttpApduNum 跟随G_CurrentComNUM递减
				
				G_ErrorActionOffsetLgth[0] = valuelgth;   //保存当前 ErrorAction TLV 的长度
				
				G_ErrorActionOffsetLgth[1] = offset;     //保存当前ErrorAction TLV 数据对象的V的起始偏移
				
				result = POR_OK;
				
				break;
				
			case TAG_ScriptChaining:    		//0x83
				
				result = Process83(fileAddr,valuelgth,offset);
				
				if(tag83 == 0x00)
				{
					tag83 = 0x01;
				}
				
				break;
				
#if M2M_EUICC
			case 0x84:
				result = Process84(fileAddr, valuelgth+(firstCpl[0]+2), offset-(firstCpl[0]+2), sdId);
				G_86errorNum = 0x00;
				if(result != POR_OK)
				{
					GPRAMRFMExtendResponsePublic(TAG_NumberCommandTLV);  //更新指令条数tag
				}
				break;
			case 0x85:
				result = Process85(fileAddr,valuelgth+(firstCpl[0]+2),offset-(firstCpl[0]+2),sdId);
				if(result != POR_OK)
				{
					GPRAMRFMExtendResponsePublic(TAG_NumberCommandTLV);  //更新指令条数tag
				}
				break;
			case 0x86:
				if((tag83 == 0) && (G_CurrentChainingValueTag == TRUE))
				{
					ResData[0]	= 0x9F; //bad format T
				
					ResData[1]	= 0x46;
						
					ResData[2]	= 0x01;  
					
					ResData[3]	= 0x02; 
						
					PaddingScp03tExtendRes(4); // return a bad format TLV as defined in ETSI TS 102 226 [5] instead of the SCP03t-specific error tag.//
							
					cpLgth = 0;	

					G_CurrentChainingValueTag = 0;
						
					break;
				}
				
				if((valuelgth > (cpLgth - (firstCpl[0]+2)))||((firstCpl[0]==0)&&(valuelgth>0x7F)))   // TLV 不完整 或者编码格式不符合BER TLV
				{
					ResData[0]	= 0x9F; //bad format T
				
					ResData[1]	= 0x46;
						
					ResData[2]	= 0x01;  
					
					ResData[3]	= 0x01; 
						
					PaddingScp03tExtendRes(4); // return a bad format TLV as defined in ETSI TS 102 226 [5] instead of the SCP03t-specific error tag.//
					
					result = CipheringError;
							
					cpLgth = 0;	
						
					break;
				}
					
				
				if(G_86errorNum != 0)
				{
					cpLgth = 0;	
					
					result = CipheringError;
					
					break;
				}

				result = Process86(fileAddr,valuelgth+(firstCpl[0]+2),offset-(firstCpl[0]+2),sdId);	
				if(result != POR_OK)
				{
					GPRAMRFMExtendResponsePublic(TAG_NumberCommandTLV);  //更新指令条数tag
				}				
				break;
				
#endif
			default:    //Bad format TLV(01: unknown tag found)
			{
// // 				result =  ExtendedRFMProcessBadTLV(FLAG_TagNotFound);
				break;
			}
			
		}

		if(G_EarlyResponseTag == 0x11)  //有early response且碰到了refresh指令
		{
			break;
		}
		
		if((SW1 != 0x90)&&(SW1 != 0x9F)&&(SW1 != 0x61))  //指令执行错误,判断the start of the script and the C-APDU resulting in an error之前是否存在error action TLV
		{
			if(G_ErrorActionOffsetLgth[1] < offset)  //between the start of the script and the C-APDU resulting in an error
			{
                 Process82(fileAddr);
				
			}
			
		}

		if(result !=  POR_OK)   //表示指令执行失败 
		{		
			break;
		}

		cpLgth = cpLgth - (firstCpl[0]+2+valuelgth);  //剩余数据长度
		
		if((tag83 == 0x00)&&(cpLgth == 0x00)&&(tag != TAG_C_APDU)&&(G_CAPDUResponse == 0x00)) //10222系列测试脚本 不支持Tooken  RFM_ExpandedRemoteCmd_2_1用例 最后一条为83tag  期望拼装TAG_R_APDU
		{
			GPRAMRFMExtendResponsePublic(TAG_R_APDU); //102226  5.2.2   In case no Script Chaining is present in the command list or processing of the Script Chaining produces no error, it shallbe formatted according to table 5.10 or table 5.10a.说明要拼装TAG_R_APDU？
		}
		
		offset += valuelgth;  //指向下一个tag			
	}
#if GP__AMENDMENT__B__FOR_MASK	
	if(G_Source_Type == SourceType_HTTPS)     //保存当前Http RFM上下文
	{
		if((cpLgth == 0x00)&&(mode != RemoteDataEndBlock))                                          //指令完整时候移动 tag
		{
			IF_ProcessRemoteMoveFalshForHTTP(fileAddr,offset,cpLgth);   //
		}
//		if((cpLgth != 0x00)&&(result == POR_OK))
//		{
//			G_CurrentComNUM --;       //有断开的指令  算执行完整的指令个数 或者执行出错的
//		}
#if UICC__FUNCTION__RFM
		IF_AnnexB_HTTPRFM(HTTP_RFM_MODE_WRITE);
#endif		
	}
#endif
	if(result == POR_OK)
	{
		if(G_FormatAndType == FormatType_ExtendAA)
		{
			if((cpLgth == 0x00)&&(mode == RemoteDataEndBlock))
			{
				GPRAMRFMExtendResponsePublic(TAG_NumberCommandTLV);  //更新指令条数tag
			}
		}
		
		//若当前指令为script chaining,指令执行成功后，则保存当前的chainging infomration
#if UICC__FUNCTION__RFM			
		if((G_CurrentChainingValueTag == TRUE)&&(G_Data_Type == DataType_RFM))
		{
			ScriptChainfileAddr = SelectPrivateTFFile(DF_Toolkit,EF_ScriptChainingFile);
			
			if(ChainingValue.previousValue == 0x03)
			{
				//SetTFConst(fileAddr, 0x00,0x00,sizeof(ChainingValue));  //当前script chaining为最后一条
			}
			else
			{
				ChainingValue.dfAddr = G_CurrentDfAddr[G_ChannelNum];  

				ChainingValue.efAddr  = G_CurrentEfAddr[G_ChannelNum];

				ChainingValue.dfName = G_CurrentDfName[G_ChannelNum];

				ChainingValue.efName = G_CurrentEfName[G_ChannelNum];

				ChainingValue.recNum = G_CurrentRecNum[G_ChannelNum];

				ChainingValue.pinStatus = G_ChvStatus;
				
				WriteTFBuf(ScriptChainfileAddr, 0x00, (BYTE *)&ChainingValue,sizeof(ChainingValue));
			}
		}
#endif		
	}
	
	G_CommandMode = RFMCommandMode;
	
	G_ChvStatus = RFMChvStatus;
#if !SE_ONLY
	memcpy(&G_LocalPinStatus, &RFMLocalPinStatus[0], 0x08);
#endif
	G_AdmStatus = RFMAdmStatus;
	
	ClearBit(G_SystemStatus,B_RfmPerform);
#if UICC__FUNCTION__RAM	
	if(G_Data_Type == DataType_RAM)
	{
		if(G_RAMSessionSdId != ILLEGAL_APPID)
		{
			if(G_CurrentChainingValueTag != TRUE)        //  RAM扩展格式83 tag时候 保持级联信息
			{
					
				//HTTPS 不清会话信息: GP_UICC_Compliance_SCP81---GP_UICC_Compliance_SCP81---scenario_07_rsaToken
				if (G_Source_Type != SourceType_HTTPS)		
				{
					GPClearSecureSessionInfo(/*G_RAMSessionSdId*/);	 //RAM APDU执行成功，清除安全会话信息      压缩格式时候只有 SCP02 没有SCP03T
				}
			}
		}
		
// 			memset(G_RAMGetStatus,0x00,sizeof(G_RAMGetStatus));	 //指令层会清  而且http 会陆续来完整的AE tag来多条StoreData等
	}
	else
#endif	
	{
#if UICC__FUNCTION__RFM		
		ReloadDFOrEFAddr(fileName);
		
		G_CurrentSfi = RFMSFI;

		G_CurrentAdfName[G_ChannelNum] = RFMADFName;
			 
		G_CurrentRecNum[G_ChannelNum] = RFMCurrentRecord;
#endif		
	}	
	
	if(G_EarlyResponseTag == 0x11)  //当前响应为early response且存在refresh指令
	{
		//cephRefreshSingle(0x04,NULL);
		cephRefreshSingle(0x04);
	}
	
	G_EarlyResponseTag = 0x00;
	
	return result;

}
/*
*@description:根据长度判断扩展格式Por数据长度需要添加的82XX等
*
*@param       
*@mode        :
*
*@return      :offset  返回文件的总偏移
*
*@author      :
*
*@history     :
*
*                  
*/
BYTE ExtendResTagADD(WORD additionalLen)
{	
	BYTE idata additionalLenTLLen;
	
	additionalLenTLLen = 4;
	if (G_FormatAndType == FormatType_ExtendAA)						// 		AA tag
	{
		if(additionalLen > 0xFF)
		{
			additionalLenTLLen = 4; //0xAB+0x82 +XXXX+	additionalLen					
		}
		else if(additionalLen > 0x7F)
		{
			additionalLenTLLen = 3; //0xAB+0x81+xx +additionalLen					
		}
		else
		{
			additionalLenTLLen = 2; //0xAB+xx+additionalLen					
		}
	}
//	else
//	{
//		additionalLenTLLen = 4; ////0xAF+0x80+ additionalLen	+0000		
//	}				
    return additionalLenTLLen;
}
/*
*@description:添加扩展执行响应到POR文件
*
*@param       ：PorAddr传入的POR地址,offset要写入的por文件的偏移，total要写入por文件的数据长度
*@mode        :
*
*@return      :offset  返回文件的总偏移
*
*@author      :
*
*@history     :
*
*                  
*/
WORD ExtendResValueADD(UWORD PorAddr,WORD offset,WORD total)
{
	BYTE  idata pos;

	UWORD xdata fileAddr;
	
	if(total == 0x00)        //针对deliver转submit时候 第一条数据长度为0
	{
      return offset;
	}
	
	fileAddr = SelectPrivateTFFile(DF_Toolkit,EF_RAMExtendResponse);
		
		//这里要区分 AA tag 和 AE tag
		
	if (G_FormatAndType == FormatType_ExtendAA)						// 		AA tag
	{
		tempBuf[16] = TAG_ResponseScriptingTemplateDefiniteLength;    // Response Scripting Template for definite length coding
			
		pos = SetBerTlvLength(&tempBuf[17],total);
			
		WriteTFBuf(PorAddr,offset,&tempBuf[16],pos+1);      //写AB82XXXX

		offset += (pos+1);
		//VMCopyDataToAddr(PorAddr+sizeof(TranFileHead)+offset, fileSize-offset, (fileAddr+sizeof(TranFileHead)+ramExtendResValue_offset),total, MODE_PROTECT_WRITE);
		VMCopyDataToAddr(PorAddr,offset,0, fileAddr,ramExtendResValue_offset,total, MODE_PROTECT_WRITE,1,1);// modify by xuweihua@20220514 同时兼容配置区地址、文件头体分开、文件头体连接这三种情况下数据的移动

		offset +=total;
	}
	else
	{
		tempBuf[16] = TAG_ResponseScriptingTemplateInDefiniteLength;
			
		tempBuf[17] = 0x80;
			
		WriteTFBuf(PorAddr,offset,&tempBuf[16],0x02);      //写AF80
			
		offset += 0x02;		
		
		
		//VMCopyDataToAddr(PorAddr+sizeof(TranFileHead)+offset, fileSize-offset, (fileAddr+sizeof(TranFileHead)+ramExtendResValue_offset),total, MODE_PROTECT_WRITE);
		VMCopyDataToAddr(PorAddr,offset,0, fileAddr,ramExtendResValue_offset,total, MODE_PROTECT_WRITE,1,1);// modify by xuweihua@20220514 同时兼容配置区地址、文件头体分开、文件头体连接这三种情况下数据的移动
			
		offset += total;
			
		tempBuf[18] = 0x00;
			
		tempBuf[19] = 0x00;
			
		WriteTFBuf(PorAddr,offset,&tempBuf[18],0x02);      //写0x00
			
		offset += 0x02;
	}
	
	G_FormatAndType = FormatType_NULL;						//添加完数据后还原默认值  压缩格式
	
	G_CurrentRFMEfAddr = FALSE;
	G_CurrentRFMEfOffset = 0x0000;// modify by xuweihua@20220514
	
	GPRAMRFMExtendResponsePublic(TAG_ClearResponseFile);
	
	return offset;	
}
/*
*@description: 扩展格式时候 如果数据过长 Por为deliver,又不支持deliver级联 则需要修改扩展格式响应数据
*
*@param       
*@mode        :
*
*@return      :
*
*@author      :
*
*@history     :
*
*                  
*/
BYTE IF_ExtendResTagChange(BYTE additionalLen)
{
	BYTE idata tag;
//	BYTE idata tagNum;
	WORD idata tagLen;
	BYTE idata tlv;
	WORD idata dataoffset;
	BYTE idata offset;
	BYTE idata AAorAE;
	BYTE idata tlvdata[4];
	UWORD xdata fileAddr;
	
	fileAddr = SelectPrivateTFFile(DF_Toolkit,EF_RAMExtendResponse);  //lv 结构，前2字节为L ,
	
	if (G_FormatAndType == FormatType_ExtendAA)   // AA tag  则响应数据的 tag偏移为 2 +3（8001XX）
	{
		offset = 0x05;
	}
	else                                                     // AE tag 则响应数据的偏移为 2
	{
		offset = 0x02;
	}
	
	//dataoffset = offset;
	AAorAE = offset;
	
	tlv = 0x00;
// 	tagNum = 0x00;
	
	while(offset<additionalLen)
	{
		dataoffset = 0x00;
		
// 		tagNum ++;
		
		ReadTFBuf(fileAddr,offset,tlvdata,sizeof(tlvdata));      //读取TLV
		
		tagLen = GetBerTlvLength(&tlvdata[1],&dataoffset);
		
		if((offset + tagLen + dataoffset)<additionalLen)
		{
			
			ReadTFBuf(fileAddr,offset,&tempBuf[tlv],tagLen+dataoffset);      //读取TLV
		}
		else
		{

			tag = tlvdata[0];
			
			tagLen = additionalLen - offset - 3; //-3  默认 tag +L 为三字节 防止返回数据 比additionalLen还长一个字节
			
			tempBuf[tlv++] = tag;
			
			if(tagLen>=0x80)
			{
				tempBuf[tlv++] = 0x81;
			}
			
			tempBuf[tlv++] = tagLen;
			
			ReadTFBuf(fileAddr,offset+dataoffset,&tempBuf[tlv],tagLen);      //读取V
			
			tlv += tagLen;
			
			break;
			
		}
		
		offset += (tagLen+ dataoffset);
		
		tlv += (tagLen+ dataoffset);
		
	}
	
	tempBuf[tlv-2] = 0x62;   //修改状态字
	
	tempBuf[tlv-1] = 0xF1;
	
	WriteTFBuf(fileAddr,AAorAE,tempBuf,(tlv+1)); //数据写回
	
#if UICC_SUPPORT_CHAGE_DELIVER_POR
	if((AAorAE == 0x05)&&(G_BakCommandNumber != 0))
	{
 		//WriteTFBuf(fileAddr,(AAorAE-1),&tagNum,0x01); //num写回       不能写  AAtag时候 写数据tag数和执行的tag并不对等
		WriteTFBuf(fileAddr,(AAorAE-1),&G_BakCommandNumber,0x01); //num写回 
		G_BakCommandNumber = 0x00;
	}
#endif

	dataoffset = (AAorAE+tlv-2); //2是总长度  前两个字节
	
	WriteTFFileWord(fileAddr, dataoffset);//WriteTFBuf(fileAddr,0x00,(BYTE *)&dataoffset,0x02); //长度写回

	return (AAorAE+tlv-2);
}
#endif
