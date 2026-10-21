#include "string.h"
#include "main.h"
#include "gsmpublic.h"
#include "error.h"
#include "ceph.h"
#include "fileAction.h"
#include "fileSystem.h"
#include "stkhead.h"
#include "interface.h"
#include "resume.h"
#include "MultiImsi.h"



#ifdef MULTI_IMSI

/**
 * @brief 		该函数判断MCC/MNC的值是否是代表中国，以及是否与9F02文件记录匹配，根据比较结果更新IMSI及FPLMN文件
 * 
 * @param 		mccmnc: MCC 及 MNC ，3 BYTES
 * 
 * @Returns   	TRUE/FALSE
 * 
 * @author 		XWH
 * 
 * @history 	add comment by xwh, @20150418
 */
BYTE code MCCMNC[]={0x64,0x00,0x00,0x54,0x05,0x00,0x67,0x01,0x00,0x58,0x01,0x00};
void ProcessMccMncMatchAndChange(BYTE * mccmnc)
{
	BYTE  idata matchRecNo,recNum,recLgth;
	BYTE  idata i;
	WORD  idata offset;
	UWORD xdata fileAddr;

	matchRecNo = 0;         //表示没有匹配的记录

	//1- 卡首先判断接收到的位置信息是否中国或者澳门的位置信息，如果是，使用9F01记录1对应的IMSI
	//表示"中国"的位置信息 : 从终端向卡传递的位置信息，是64x0xx(3GPP格式的位置信息 ，x表示通配符)或者是6701xx(3GPP2格式的位置信息x表示通配符)
	//表示"澳门"的位置信息 : 从终端向卡传递的位置信息，是54x5xx(3GPP格式的位置信息 ，x表示通配符)或者是5801xx(3GPP2格式的位置信息x表示通配符)
	fileAddr = SelectTFFile(G_DF_ID_USIM, EF_MCCMNCDEFAULT);
	
	if(fileAddr == FALSE)               //若文件EF_MCCMNCDEFAULT不存在，使用默认值
	{

//		tempBuf[0x00] = 0x64;
//		tempBuf[0x01] = 0x00;
//		tempBuf[0x02] = 0x00;
//		tempBuf[0x03] = 0x54;
//		tempBuf[0x04] = 0x05;
//		tempBuf[0x05] = 0x00;
//		tempBuf[0x06] = 0x67;
//		tempBuf[0x07] = 0x01;
//		tempBuf[0x08] = 0x00;
//		tempBuf[0x09] = 0x58;
//		tempBuf[0x0A] = 0x01;
//		tempBuf[0x0B] = 0x00;
		
		memcpy(tempBuf,MCCMNC,0x0c);
	}
	else
	{			
		ReadTFBuf(fileAddr,0,tempBuf,0x0C);	
	}

	
	if(((mccmnc[0] == tempBuf[0])&&((mccmnc[1]&0x0F) == tempBuf[1])) \
	|| ((mccmnc[0] == tempBuf[6])&&(mccmnc[1] == tempBuf[7]))        \
	|| ((mccmnc[0] == tempBuf[3])&&((mccmnc[1]&0x0F) == tempBuf[4])) \
	|| ((mccmnc[0] == tempBuf[9])&&(mccmnc[1] == tempBuf[10])))		 
	{
		matchRecNo = 0x01;	
	}
	else
	{		
		fileAddr = SelectLFFile(G_DF_ID_USIM, EF_IMSIROAMING);
		
		recNum = GetLFRecNumLgth(fileAddr,FileNum);
		recLgth = GetLFRecNumLgth(fileAddr,FileLgth);

		//2- 将位置信息与包含MCC及MCC的位置信息记录（9F02记录中，第一个字节为"00"的记录）对比，如果匹配到对应记录，使用对应的IMSI	
		for(i=1; i<=recNum; i++)
		{
			ReadLFBuf(fileAddr,i,0,tempBuf,recLgth);
			
			//第一字节是FF，表示后续记录都是空记录
			if(tempBuf[0] == FORMAT_NULL)
			{
				break;
			}		

			if(tempBuf[0] == FORMAT_MCCMNC)
			{
				for(offset=1; offset<(recLgth-1); offset+=4)
				{
					if(memcmp(&mccmnc[0],&tempBuf[offset],3)==EQUAL)
					{
						matchRecNo = tempBuf[offset+3];
						break;	
					}
				}	
			}

			if(matchRecNo != FALSE)
			{
				break;
			}
		}
		//3- 将位置信息与只包含MCC的位置信息记录（9F02记录中，第一个字节为"01"的记录）对比，如果匹配到对应记录，使用对应的IMSI，如果无法匹配
		if(matchRecNo == FALSE)
		{
			for(i=1; i<=recNum; i++)
			{
				ReadLFBuf(fileAddr,i,0,tempBuf,recLgth);
				
				//第一字节是FF，表示后续记录都是空记录
				if(tempBuf[0] == FORMAT_NULL)
				{
					break;
				}		
	
				if(tempBuf[0] == FORMAT_MCC)
				{
					for(offset=1; offset<(recLgth-1); offset+=4)
					{
						if((memcmp(&mccmnc[0],&tempBuf[offset],2)==EQUAL)&&(tempBuf[offset+2]==0xFF))
						{
							matchRecNo = tempBuf[offset+3];
							break;	
						}
					}	
				}

				if(matchRecNo != FALSE)
				{
					break;
				}
			}
		}		
	}

	//4- 如果经历步骤1-3都无法匹配到相关条件，使用9F01文件第2条记录的IMSI
	if(matchRecNo == FALSE)
	{
		matchRecNo = 0x02;	
	}

	fileAddr = SelectTFFile(G_DF_ID_USIM, EF_IMSI);		
	ReadTFBuf(fileAddr,0,tempBuf,9);		                          //当前IMSI

	fileAddr = SelectLFFile(G_DF_ID_USIM, EF_IMSI2);		

	if(fileAddr == FALSE)
	{
		return;
	}

	recNum = GetLFRecNumLgth(fileAddr,FileNum);
	recLgth = GetLFRecNumLgth(fileAddr,FileLgth);

	if(matchRecNo > recNum)	                                      //matchRecNo没有指向有效的9F01记录
	{
		return;
	}

	ReadLFBuf(fileAddr,matchRecNo,0,tempBuf+9,recLgth);	          //匹配的IMSI和FPLMN
		
	//确定需要使用的IMSI；如果其与当前USIM\6F07的IMSI相同，不需要切换，否则需要切换IMSI并且继续步骤4
	if(memcmp(tempBuf,tempBuf+9,9)!=EQUAL)	
	{		
		fileAddr = SelectTFFile(G_DF_ID_USIM, EF_IMSI);		
		WriteTFBuf(fileAddr,0,tempBuf+9,9);
		
		//USIM\6F07文件切换IMSI的同时，USIM\EFFPLMN也需要切换至与切换后IMSI相匹配的FPLMN内容(9F01文件中对应IMSI记录里的FPLMN内容)	
		fileAddr = SelectTFFile(G_DF_ID_USIM, EF_FPLMN);
		WriteTFBuf(fileAddr,0,tempBuf+9+9,recLgth-9);
		
		//切换IMSI后，Ki，Op，Opc使用G_Cur_IMSI_RecNo对应的数据,by xwh 20150421			
		//切换IMSI后，SQN使用G_Cur_IMSI_RecNo对应的数据,by xwh 20150421		
		G_Cur_IMSI_RecNo = matchRecNo;
		
		//切换成功后，卡片发送REFRESH 03指令
		PendingMessageAlert(D_SendRefresh03,0x00);	
	}
}

/**
 * @brief 		该函数判断MCC/MNC的值是否是代表中国，以及是否与9F02文件记录匹配，根据比较结果更新IMSI及FPLMN文件
 * 
 * @param 		mccmnc: MCC 及 MNC ，3 BYTES
 * 
 * @Returns   	TRUE/FALSE
 * 
 * @author 		XWH
 * 
 * @history 	add comment by xwh, @20150418
 */
void ProcessImsiFplmnScychronze(void)
{
	BYTE  idata recNum,recLgth;
	BYTE  idata i;
	UWORD xdata fileAddr;

	fileAddr = SelectTFFile(G_DF_ID_USIM, EF_IMSI);		

	if(fileAddr == FALSE)
	{
		return;
	}

	ReadTFBuf(fileAddr,0,tempBuf,9);                      //当前IMSI

	fileAddr = SelectLFFile(G_DF_ID_USIM, EF_IMSI2);

	if(fileAddr == FALSE)
	{
		return;
	}
		
	recNum = GetLFRecNumLgth(fileAddr,FileNum);
	recLgth = GetLFRecNumLgth(fileAddr,FileLgth);

	for(i=1; i<=recNum; i++)
	{
		ReadLFBuf(fileAddr,i,0,tempBuf+9,recLgth);       //9F01中的IMSI+FPLMN

		if(memcmp(tempBuf,tempBuf+9,9)==EQUAL)           //当前IMSI与9F01中的记录匹配
		{
			G_Cur_IMSI_RecNo = i;
			
			fileAddr = SelectTFFile(G_DF_ID_USIM, EF_FPLMN);

			if(fileAddr != FALSE)
			{
				ReadTFBuf(fileAddr,0,ResData,recLgth-9);
	
				if(memcmp(ResData,tempBuf+9+9,recLgth-9) != EQUAL)  //FPLMN与IMSI对应的FPLMN不匹配，做同步操作
				{
					WriteTFBuf(fileAddr,0,tempBuf+9+9,recLgth-9);
				}
			}
			
			break;	
		}
	}
}
/**
 * @brief 		该函数处理LocationStatusEvent事件，根据MCC/MNC的值切换IMSI
 * 
 * @param 		NONE
 * 
 * @Returns   	TRUE/FALSE
 * 
 * @author 		XWH
 * 
 * @history 	add comment by xwh, @20150418
 */
void ProcessLocationStatusEvent(void)
{
	BYTE  idata position,num;

	num = (RecData[1]>0x80?3:2);
	
	position = FindTag(&RecData[num],TLV_LOCATIONSTATUS,RecData[num-1]);

	if(position == FALSE)
	{
		return;
	}
	
	//如果终端回复Eenvelop命令，location status命令内容是"02"（no service）；卡不做处理
	if(RecData[num+position+1] == LOCATIONSTATUS_NOSERVICE)	
	{
		return;
	}
	else if(RecData[num+position+1] == LOCATIONSTATUS_LIMITED)	
	{
		//如果终端回复ENVELOPE命令，location status命令内容是"01"（Limited service）；卡发送PROVIDE  LOACAL INFORMATION命令给终端，如果终端无法提供位置信息，卡最多再发两次PROVIDE LOCAL INFORMATION命令给终端
		PendingMessageAlert(D_SendProvideLocalInfo00,0x00);		
		return;
	}
	else
	{
		
	}

	position = FindTag(&RecData[num],TLV_LOCATIONINFOMRATION,RecData[num-1]);

	if(position == FALSE)
	{
		return;
	}

   	//Location Information length = 7
	//MCC MNC至少2字节
	if(RecData[num+position] < 0x02)	
	{
		return;
	}
	
	ProcessMccMncMatchAndChange(&RecData[num+position+1]);
}


/**
 * @brief 		该函数处理Provide Local Info 的TerminalResponse,若Provide Local Info 不成功，最多发送3次Provide Local Info 
 * 
 * @param 		NONE
 * 
 * @Returns   	TRUE/FALSE
 * 
 * @author 		XWH
 * 
 * @history 	add comment by xwh, @20150418
 */
void ProcessProvideLocalInfoResponse(void)
{
	BYTE  idata k,result;
	BYTE  idata position;

	k = 3;

	while(k)
	{
		InitProactiveCommand(CMD_PROVIDELOCALINFORMATION,LOCALINFOR,DEV_ME);
		
		result = SendProactiveCommand();

		if(result == CMD_Success)
		{
			position = FindTag(&RecData[0],TLV_LOCATIONINFOMRATION,P3);

			//MCC MNC存在且长度至少是2字节
			if((position != FALSE)&&(RecData[position] >= 2))
			{
				ProcessMccMncMatchAndChange(&RecData[position+1]);
				break;
			}			
		}

		k --;
	}
}


/**
 * @brief 		该函数发送Set Up Event List命令
 * 
 * @param 		NONE
 * 
 * @Returns   	TRUE/FALSE
 * 
 * @author 		XWH
 * 
 * @history 	add comment by xwh, @20150418
 */
void SendSetUpEventList(void)
{
	InitProactiveCommand(CMD_SETUPEVENT,0x00,DEV_ME);

	tempBuf[0] = TLV_EVENTLIST&0x7F;  // Event List Tag	
	tempBuf[1] = 0x01 ;
	tempBuf[2] = 0x03 ;

	AddTLV((BYTE xdata *)&ResData[1],(BYTE *)&tempBuf[0]);
	STKPerform = TRUE;
}

#endif
