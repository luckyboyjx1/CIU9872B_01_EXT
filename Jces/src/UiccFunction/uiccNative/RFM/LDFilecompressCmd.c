/*
*@Vesion         :V1.0
*
*@description    :
*
*@history        :V1.0: 创建原文件
*				  							
*									V1.1: 修改函数CompressCMDProgress,增加备份G_CurrentAdfName[G_ChannelNum]与G_CurrentSfi,G_ChannelNum
*/
#include  "typedef.h"
#include  "uicc_main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include  "error.h"
#include  "arithmatic.h"
#include  "ceph.h"
#include  "interface.h"


#ifdef LDFile
#include  "LDFile.h"




/**
 * @description:	该函数主要用于处理compress RFM,
 *					数据已缓存在2700/LDFile_RelaxFile中，
 *					该文件的结构为L(2字节)+SPI(2字节)+KIC(1字节)+KID(1字节 )+TAR(3字节)+COUNTER(5字节)+ V	
 *
 * @return:			处理结果
 *
 * @history :		create by PL @20121106		 
 */
BYTE CompressCMDProgress(void)
{
	UWORD xdata fileAddr, fileAddr2;
	WORD xdata len, ptr, length;
	BYTE idata num, resultState, state,pos,result;
	BYTE xdata infor[5]; 
	
	//Todo :user may need back-up the current ef's(df's) information ,channel_mode
	
	BYTE idata RFMCurrentRecord,RFMCommandMode,RFMSFI,RFMChannel;
	WORD idata RFMDFName,RFMEFName,RFMADFName;
	UWORD xdata RFMDFAddr,RFMEFAddr;
	
	RFMChannel = G_ChannelNum;
	
	RFMDFAddr = G_CurrentDfAddr[G_ChannelNum];                        //a data copy

  	RFMEFAddr = G_CurrentEfAddr[G_ChannelNum];

  	RFMDFName = G_CurrentDfName[G_ChannelNum];

  	RFMEFName = G_CurrentEfName[G_ChannelNum];
	
	RFMADFName  = G_CurrentAdfName[G_ChannelNum];
		 
	RFMSFI = G_CurrentSfi;

  	RFMCurrentRecord = G_CurrentRecNum[G_ChannelNum];

	RFMCommandMode = G_CommandMode;
	
	fileAddr = SelectTFFile(DF_OTA,LDFile_RelaxFile);   //5E03  解密缓存文件	
	
	fileAddr2 = SelectTFFile(DF_OTA, LDFile_UpFile);    //上行缓存地址
	
	if (fileAddr2 == FALSE)                     // fileAddr  不用判断  if  fileAddr == FALSE 不会进来
	{
		return NoMemory;
	}
	
	ExtendedRFMUpdateResponseTLV(TAG_ClearResponseFile);			//复用扩展型处理，清除上行缓存文件
	
	G_CompressRFMComNUM = 0x00;		//复用，压缩型指令执行条数

	ptr = G_CurrentCommandOffset;		//数据偏移
	
	ReadTFBuf(fileAddr, ptr-0x07, &G_CompactCompressTag,sizeof(BYTE));  //复用，获取当前tar值。   
	
	if ((G_CompactCompressTag == TAR3_SHARE_FILE)		//共享文件
		||(G_CompactCompressTag == TAR3_SIM))
	{
		G_CurrentDfName[G_ChannelNum] = DF_MF;  

		G_CurrentDfAddr[G_ChannelNum] = E_FILE_START;		
	}
	else if (G_CompactCompressTag == TAR3_CSIM_ADF)
	{
		G_CurrentDfName[G_ChannelNum] = G_DF_ID_CSIM;

		G_CurrentDfAddr[G_ChannelNum] = SelectFile(E_FILE_START,G_DF_ID_CSIM);
		
		G_CurrentAdfName[G_ChannelNum] = G_DF_ID_CSIM;
	}
	else if (G_CompactCompressTag == TAR3_USIM_ADF)
	{
		G_CurrentDfName[G_ChannelNum] = G_DF_ID_USIM;

		G_CurrentDfAddr[G_ChannelNum] = SelectFile(E_FILE_START,G_DF_ID_USIM);
		
		G_CurrentAdfName[G_ChannelNum] = G_DF_ID_USIM;
	}
	else  //暂时不支持ISIM
	{
		return NoMemory;
	}

	
	G_CurrentEfName[G_ChannelNum] = FALSE;

	G_CurrentEfAddr[G_ChannelNum] = FALSE;
	
	ReadTFBuf(fileAddr, 0x00, (BYTE *)tempBuf, 0x02);		//读取长度  CPL
	
	len = (WORD)((tempBuf[0] << 0x08) & 0xFF00) | (tempBuf[1] & 0xFF);
	
	len += 0x02;	//指向结尾+1
	
	length = 0x00;	//结果数据长度
	
	SetBit(G_SystemStatus,B_RfmPerform);	//初始状态设置
	
	G_CommandMode = Command_Mode_First;
	
	SW1 = 0x90;
	
	resultState = PoROK;
	
	while((resultState == PoROK) && (ptr < len) && ((SW1 == 0x90)||(SW1 == 0x9F) || (SW1 == 0x61)))	//循环执行指令
	{		
		fileAddr = SelectTFFile(DF_OTA,LDFile_RelaxFile);   //5E03  解密缓存文件
		
		ReadTFBuf(fileAddr, ptr, (BYTE *)G_APDU, 0x05);	//获取APDU

		if(G_APDU[0] == G_APDU[1])  //CLA == INS ,退出
		{
			break;
		}

		if(((ptr+P3)>len)&&((INS != 0xB0) && (INS != 0xB2) && (INS != 0xC0)))           //下行本身有问题跳出？
		{
			break;
		}

		G_CompressRFMComNUM ++;		//执行指令条数

			if(((G_CompactExtendedTag == TAR3_SIM)&&(CLA != 0xA0))
		 || ((G_CompactExtendedTag == TAR3_CSIM_ADF)&&(CLA != 0x80)&&(CLA != 0x00))
		 || ((G_CompactExtendedTag == TAR3_USIM_ADF)&&(CLA != 0x80)&&(CLA != 0x00))
		 )
		{
		 		SW1 = 0x6E;
				SW2 = 0x00;
				break;
		}
			
		ptr += 0x05;		//向下偏移
		
		if(G_CommandMode == Command_Mode_First)					//第一个APDU命令
		{
			if (CLA == 0xA0)
			{
				G_CommandMode = Command_Mode_2G;					//当前为2G操作模式
			}
			else if (((CLA&0xFC) == 0x80)||((CLA&0xF0) == 0x00))
			{
				G_CommandMode = Command_Mode_3G;					//当前为3G操作模式
						
			}		
		}
		
		G_ChannelNum = CLA & 0x03;		//通道
		
		if (((G_CommandMode == Command_Mode_2G)^(CLA==0xA0))||(((CLA&0xFC) != 0x80)&&((CLA&0xF0) != 0x00)&&(CLA != 0xA0)))		//拒绝非激活的操作模式							
		{
	        ClaError();	        	        
		
			break;
		}
		
		if ((P3 != 0x00) && ((INS != 0xB0) && (INS != 0xB2) && (INS != 0xC0)))	//case3,case4
		{
			fileAddr = SelectTFFile(DF_OTA,LDFile_RelaxFile);   //5E03  解密缓存文件
			
			ReadTFBuf(fileAddr, ptr, (BYTE *)RecData, P3);		//获取数据
			
			ptr += P3;			//继续偏移
		}
		
		if(INS == 0xE0)
		{
			SetBit(G_SystemStatus,B_cmk);

			pos = FindTag(&RecData[0],0x62,P3);

			if(pos == FALSE)
			{
				SW1 = 0x6A;
				SW2 = 0x80;
				break;
			}
		
			pos ++;

			result = FindTag(&RecData[pos],0x84,P3 -pos);										//Not Perform Creat ADF
		
			if(result == TRUE)
			{
				SW1 = 0x6A;
				SW2 = 0x80;													 	//Send 6A 80
				break;
		
			}

			ClearBit(G_SystemStatus,B_RfmPerform);	//远程文件管理在创建文件时，使用的权限是0x8B，因此在这里先将RFM标记清掉

		}

		if(INS == 0xA4)
		{
			if( (G_CommandMode == Command_Mode_3G)&&(P1 == 0x04))  //SELECT不支持 P1 = 04的情况
			{
				SW1 = 0x6A;
				SW2 = 0x86;
				break ;
			}
			
			if ((G_CommandMode == Command_Mode_2G)&&(P2 != 0x00))
			{
				SW1 = 0x6B;
				SW2 = 0x00;
				break;
			}
		}

		if(INS == 0xC0)
		{
			if(P3 == 0x00)
			{
				P3 = G_CurrentReponse[G_ChannelNum];
			}
		}
				
		for (num=0;;num++)
	    { 
	 		if (INS == 0xC0)		//GetResponse 特殊处理
	 		{
	 			fileAddr2 = SelectTFFile(DF_OTA, LDFile_UpFile);    //上行缓存地址
				
				ReadTFBuf(fileAddr2, 0x00+21, (BYTE *)&state, 0x01);	//获取标记           RPL + RHL +TAR +COUNTR + PCNTR +STATUS +CCLength(留8字节)   =21
	 			
	 			if (state != 0xFF)		//不是上一批
	 			{
	 				GetResponseCommand();
	 			}
	 			else if (P3 == 0x00)		//取上一批
	 			{
	 				fileAddr2 = SelectTFFile(DF_OTA, LDFile_UpFile);    //上行缓存地址
					
					ReadTFBuf(fileAddr2, 0x01+21, (BYTE *)&SW2, 0x01);	//获取数据长度	  RPL + RHL +TAR +COUNTR + PCNTR +STATUS +CCLength(留8字节)   =21
	 				
	 				ReadTFBuf(fileAddr2, 0x07+21, (BYTE *)RecData, SW2);	//获取数据	RPL + RHL +TAR +COUNTR + PCNTR +STATUS +CCLength(留8字节)   =21
	 				
	 				SW1 = 0x90;
	 			}
	 			else
	 			{
	 				P3Error();
	 			}
	 			
	 			break;
	 		}
	 		
	 		if((remoteRfm_list[num].ins == INS)||(remoteRfm_list[num].ins == EOL))
	        {	
	        	if(remoteRfm_list[num].ins == EOL)  //INS 非法
	        	{
	        		InsError();
	        		
	        		break;
	        	}
	        				
				remoteRfm_list[num].func();

	            break;
			}                           												//perform the command now!
	    }

 		if(INS == 0xE0)
		{
			SetBit(G_SystemStatus,B_RfmPerform);	//设置RFM状态
		}

		
		if ((ptr == len) && ((SW1 == 0x61) || (SW1 == 0x9F)))		//说明结尾为61xx/9Fxx
		{
			infor[0] = 0xFF;		//标记GetRespones情况
			
			infor[1] = SW2;
			
			fileAddr2 = SelectTFFile(DF_OTA, LDFile_UpFile);    //上行缓存地址
			
			WriteTFBuf(fileAddr2, 0x00+21, (BYTE *)infor, 0x02);	//写入必要信息	    RPL + RHL +TAR +COUNTR + PCNTR +STATUS +CCLength(留8字节)   =21
			
			WriteTFBuf(fileAddr2, 0x07+21, (BYTE *)RecData, SW2);	//写入附加数据		RPL + RHL +TAR +COUNTR + PCNTR +STATUS +CCLength(留8字节)   =21
		}
		
		if ((ptr != len) && (INS == 0xA4) && (SW1 == 0x62) && (SW2 == 0x83))	//文件失效特殊处理
		{
			SW1 = 0x90;
		}
		
		if ((INS == 0xB0) || (INS == 0xB2) || (INS == 0xC0))			//case 2
		{
			fileAddr2 = SelectTFFile(DF_OTA, LDFile_UpFile);    //上行缓存地址
			
			if (SW1 == 0x62)									  		//表示长度截取62F1,最大支持0x100长度数据
			{
				//WriteTFBuf(fileAddr2, 0x07+21, (BYTE *)RecData, 0xFF);		//写入结果		
				//length += 0xFF;

				WriteTFBuf(fileAddr2, 0x07+21, (BYTE *)&RecData[0], 0x80);		//写入结果
				WriteTFBuf(fileAddr2, 0x07+21+0x80, (BYTE *)&RecData[0x80], 0x80);		
				length += 0x100;	
									
			}
			else if(SW1 == 0x90)
			{
				if(P3 == 0x00)										   		//表示该文件长度刚好为0x100
				{
					WriteTFBuf(fileAddr2, 0x07+21, (BYTE *)&RecData[0], 0x80);		//写入结果
					WriteTFBuf(fileAddr2, 0x07+21+0x80, (BYTE *)&RecData[0x80], 0x80);		
					length += 0x100;
				}
				else
				{				
					WriteTFBuf(fileAddr2, 0x07+21, (BYTE *)RecData, P3);		//写入结果		
					length += P3;	
				}
			}
			
			if (ptr != len)	//case2不在最后一条，异常
			{
				resultState = CipheringError;
			}
		}
	}	
	
	length += 0x03; 
	
	infor[0] = (BYTE)(length >> 0x08) & 0xFF;	//记录长度
	
	infor[1] = (BYTE)(length & 0xFF);
	
	infor[2] = G_CompressRFMComNUM;		//记录执行条数
	
	memcpy((BYTE *)&infor[3], (BYTE *)G_StatusResponse, 0x02);	//记录状态字

	fileAddr2 = SelectTFFile(DF_OTA, LDFile_UpFile);    //上行缓存地址

	WriteTFBuf(fileAddr2, 23, (BYTE *)infor, 0x05);			//写入必须信息  RPL + RHL +TAR +COUNTR + PCNTR +STATUS +CCLength(留8字节) +RFU +RFU  =23
	
	SetTFConst(fileAddr, 0x00, 0x00, 0x02);		//清除执行缓存文件（懒惰删除，不可清除其他信息，以后有用）
	
	ClearBit(G_SystemStatus,B_RfmPerform);		//清除状态
	
	//Todo:here ,user may resume the ef's(df's) information 
	
	G_ChannelNum = RFMChannel;
	
	G_CurrentDfAddr[G_ChannelNum] = RFMDFAddr;                        

  	G_CurrentEfAddr[G_ChannelNum] = RFMEFAddr;

    G_CurrentDfName[G_ChannelNum] = RFMDFName;

  	G_CurrentEfName[G_ChannelNum] = RFMEFName;
		
	G_CurrentAdfName[G_ChannelNum] = RFMADFName;
		 
	G_CurrentSfi = RFMSFI;

  	G_CurrentRecNum[G_ChannelNum] = RFMCurrentRecord;

	G_CommandMode = RFMCommandMode;
	
	return resultState;
}


RemoteAPDUProcessFunction

#endif
