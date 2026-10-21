/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*Description :该文件主要用于存放实现CB短信的一些函数
*
	
author    time      version    description
ZDM 2019/01/24   V1.0       创建该文件
************************************************************************************************************************************************************************************************/

#include    "hard_api.h"
#include    "fileAction.h"
#include    "fileSystem.h"
#include    "gsmpublic.h"
#include    "stkhead.h"
#include    "stkalert.h"
#include    "ToolkitRFM.h"
#include    "ToolkitAPIPublic.h"
#include    "STKCATRuntimeEnviorment.h"
#include    "CompactManage.h"
#include    "Crypto_Constant.h"
#include 	"uicc_interface.h"		//remove warning,dengyiwen,2022/9/27
/*@Function:    ToolkitCBExcute                                            
 * 
 *@Descrip:     该函数处理CB信息，触发相应的ToolkitApplet应用
 *
 *@See:	        ts_143019v050600p.pdf 6.2 
 *                                                                                                                                     
 *@Param:       event   ---事件类型，ToolkitCBFormattedEvent/ToolkitCBUnFormattedEvent
 *              refrence---RFU
 *				                                                                                                         
 *@Return:      NULL                                                                                                                                                                                                                                                                      
 * 
 *@Author:      XWH 20130910
 *  
 *@History:     V0.0---Create by XWH 20130910   
 * 
 *                   V0.1---Modify by XWH 20140214,支持一个包多条CB 的处理
 */
void ToolkitCBExcute(WORD SN,WORD MID,BYTE pageByte)
{
	UWORD xdata fileAddr;
	BYTE  idata len,ptr/*,cploffset*/,event,lenBytes;
	WORD  idata cpl,offset,totalLen;
	BYTE  xdata tmp[6],recordList[16];
	BYTE  idata /*macDataLen,chl,spi,kic,kid,*/recNum,i,num,totalPage,lastPage;
	BYTE  idata lgth,lastLgth,ccLength,result,startOffset;
	BYTE  xdata mac[8],cc[8],spiKicKid[4];
	BYTE idata mode;


	fileAddr = SelectPrivateLFFile(DF_Toolkit, EF_EnvelopeCBMessage);
	
	if(fileAddr==FALSE)
	{
		return;
	}

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	if((MID >= CBS_FORMATTED_MIDMIN)&&(MID <= CBS_FORMATTED_MIDMAX)) //see TS 23041 9.4.1.2.2 Message Identifier
	{
		event  = ToolkitCBFormattedEvent;
	}
	else
	{
		event  = ToolkitCBUnFormattedEvent;
	}

	totalPage = pageByte&0x0F;
	lastPage = (pageByte>>4)&0x0F;
	
	recordList[0] = 0x00;             //保存从1-N 条CB 在文件中的记录号
	totalLen = 0x00;                    //TLV_CELLBROADCASTPAGE数据总长度

	for(i=1;i<=totalPage;i++)	
	{
		for(num=1 ;num<= recNum; num++)
		{
			ReadLFBuf(fileAddr, num,0x00,&tmp[0],2);

			if((tmp[0] != 0x00)&&(tmp[1] != 0x00)) //非空记录
			{
				ReadLFBuf(fileAddr, num,0x07,&tmp[0],sizeof(tmp));   //Serial Number,Message Identifier,DCS,Page Parameter

				if((SN==((tmp[0]<<8)|tmp[1]))&&(MID==((tmp[2]<<8)|tmp[3]))&&(tmp[5]==((i<<4)|totalPage))) //Serial Number,Message Identifier,Page Parameter相同，是重复短信
				{
					//保存从1-N 条CB 在文件中的记录号
					recordList[recordList[0]+1] = num;
					recordList[0]++;

					//计算总长度
					if(i==1)
					{
						totalLen+=0x58;
					}
					else
					{
						totalLen+=0x52;
					}
					
					break;
				}
			}
		}			
	}

	//CB没有收齐，退出
	if(recordList[0]!=totalPage)
	{
		return;
	}
	
	ccLength = 0x00;  //无意义的初始化
	//kid = 0x00;	   
	//spi = 0x00;
	//kic = 0x00;
	ecpMem_Set(spiKicKid,0x00,4);
	cpl = 0x00;

	if(event == ToolkitCBFormattedEvent)
	{
		//从第一条CB 读取SPI,KIC,KID,TAR 等信息
		len = ReadLFFileValue1ByteLen(fileAddr,recordList[1],0,tempBuf);

		ptr = FindTag(tempBuf,TLV_CELLBROADCASTPAGE,len);
		ptr++;            //point to Serial Number

		offset = 6;	      //skip Serial Number(2),Message Identifie(2),DCS(1),page number(1)，point to cpl

		cpl = ByteToWord(tempBuf[ptr+offset],tempBuf[ptr+offset+1]);	
		offset += 2;	  //skip CPL(2),point to chl

		//spi = tempBuf[ptr+offset+1];
		//kic = tempBuf[ptr+offset+3];
		//kid = tempBuf[ptr+offset+4];
		ecpMem_Copy(spiKicKid,&tempBuf[ptr+offset+1],4);

		lastLgth = 0x00;

		//if((spi&0x03) == 0x03)    //Digital 	Signature  ,暂时不支持
		if(IF_CheckTarSupportSingleDES(spiKicKid) == FALSE) //不支持单DES USIM API　Test_Api_2_Ueh_Gsml 有单DES的指令需要过滤不触发应用
		{
			for(i=1;i<=totalPage;i++)			
			{
				SetLFConst(fileAddr, recordList[i],0x00,0x00,2);                               //清除CB 记录文件
			}
			return;
		}
		else if((spiKicKid[0]&0x03) == 0x01)  //Redundancy Check
		{
			ccLength = 0x04;
		}
		else if((spiKicKid[0]&0x03) == 0x02)  //Cryptographic Checksum
		{
			ccLength = 0x08;
		}
		else
		{
			ccLength = 0x00;
		}

		offset += 5; // 5 = chl(1字节)+spi(2字节)+kic(1字节)+kid(1字节) point to TAR

		ecpMem_Copy(&G_ToolkitTar[0], &tempBuf[ptr+offset], 0x03);   //拷贝tar值

		if(CheckTar(G_ToolkitTar,G_TARRecIndex) != TarType_ToolkitApplet)  //0表示toolkit applet
		{
			for(i=1;i<=totalPage;i++)			
			{
				SetLFConst(fileAddr, recordList[i],0x00,0x00,2);                               //清除CB 记录文件
			}
			return;
		}

		offset += 3;     //skip TAR,point to counter

		if((spiKicKid[0]&0x04) == 0x04)  //Ciphering
		{
			ecpMem_Set(tempBuf,0x00,sizeof(tempBuf));
			//解密每条CB 信息并写入原记录
			for(i=1;i<=totalPage;i++)	
			{
				ReadLFBuf(fileAddr,recordList[i],0x0D,tempBuf,0x52); //从Page Parameter后开始读
			
				if(i == 0x01)
				{
					if(totalPage == 1)
					{
						mode = FIRST_LAST_SMS;
					}
					else
					{
						mode = FIRST_FOLLOW_SMS;
					}
					
					lgth = 0x52 - 2 - 1 - 4 - 3;  // cpl(2) chl(1) SPI KIC KID(4) TAR(3),需要解密的数据长度
		
					lastLgth = lgth%8;                 
	
					if(lastLgth != 0x00)   //本条不是8的整数倍的长度，此数据不参与本次的解密，先保存起来
					{
						lgth = lgth - lastLgth;
			
						ecpMem_Move(&tempBuf[D_TmpBufStart+8], &tempBuf[0x0A+lgth], lastLgth);
					}
		
					if((spiKicKid[2]&0x0C) != 0x0C)  //des in CBC
					{
						ecpMem_Move(&tempBuf[D_TmpBufStart], (BYTE xdata *)&tempBuf[0x0A+lgth-8], 8);  //保存本次解密前最后8字节数据
					}
		
					ToolkitDeChipherRe(spiKicKid[2],&tempBuf[0x0A],&tempBuf[0x0A],MODE_DECRYPT,lgth,mode);  //从CNTR开始解密
								
					WriteLFBuf(fileAddr, recordList[i],0x0D+0x0A,&tempBuf[0x0A], lgth);	
					SetLFConst(fileAddr, recordList[i],0x06,lgth+0x0A+0x06,1);                             //解密后数据长度变化，此处更改数据长度	
				}
				else
				{
					if(totalPage == i)
					{
						mode = LAST_SMS;
					}
					else
					{
						mode = FOLLOW_SMS;
					}
					
					lgth = 0x52 + lastLgth; 							//本次数据的长度
					
					ecpMem_Move((BYTE xdata *)&tempBuf[lastLgth], (BYTE xdata *)&tempBuf[0], 0x52); //将上次剩余的未解密的数据和该次数据拼在一起。
					
					ecpMem_Copy((BYTE xdata *)&tempBuf[0], (BYTE xdata *)&tempBuf[D_TmpBufStart+8], lastLgth);
					
					lastLgth = lgth % 8;

					if(lastLgth != 0x00)
					{							
						lgth = lgth - lastLgth;
						
						ecpMem_Move(&tempBuf[D_TmpBufStart+8], &tempBuf[lgth], lastLgth);
					}

					ecpMem_Copy((BYTE xdata *)&tempBuf[D_TmpBufStart+16], (BYTE xdata *)&tempBuf[lgth-8], 8);	//将此次的原始数据的后8个字节放入BUFFER中。
					
					ToolkitDeChipherRe(spiKicKid[2],&tempBuf[0],&tempBuf[0],MODE_DECRYPT,lgth,mode);
								
					if((spiKicKid[2]&0x0C) != 0x0C)  //des in CBC
					{
						for(num=0; num<8; num++) 											//解密后的头8个字节要做异或操作。
						{
							tempBuf[num] = tempBuf[num] ^ tempBuf[D_TmpBufStart+num];
						}

						ecpMem_Copy((BYTE xdata *)&tempBuf[D_TmpBufStart], (BYTE xdata *)&tempBuf[D_TmpBufStart+16], 8);
					}
							
					WriteLFBuf(fileAddr, recordList[i],0x0D,&tempBuf[0], lgth);	
					SetLFConst(fileAddr, recordList[i],0x06,lgth+0x06,1);                             //解密后数据长度变化，此处更改数据长度	
				}		
			}
		}

	}

	offset = 0x00;

	startOffset = 0x00;

	for(i=1;i<=totalPage;i++)	
	{
		if(i==0x01)
		{
			//shall correspond to the ones in the last received Short Message
			ReadLFBuf(fileAddr, recordList[lastPage],0x01,tempBuf,5);   //82028381 8C
			offset = 0x05;

			lenBytes = SetBerTlvLength(&tempBuf[5],totalLen);
			offset += lenBytes;	
						
			ReadLFBuf(fileAddr, recordList[lastPage],0x07,&tempBuf[offset],6);   // Serial Number,Message Identifier,Data Coding Scheme,Page Parameter
			offset += 6;
			startOffset = offset;   //计算MAC 的数据起始

			ReadLFBuf(fileAddr, recordList[1],0x06,&len,1);  
			ReadLFBuf(fileAddr, recordList[1],0x07+0x06,&tempBuf[offset],len-6);   //Serial Number,Message Identifier,Data Coding Scheme,Page Parameter,Content of Message

			WriteTFBuf(G_EnvelopHanderAddr,0x00,tempBuf,offset+len-6);
			SetLFConst(fileAddr, recordList[1],0x00,0x00,2);                               //清除CB 记录文件
			
			offset += (len-6);               //offset表示在G_EnvelopHanderAddr的数据偏移量
			
		}
		else
		{
			ReadLFBuf(fileAddr, recordList[i],0x06,&len,1);  
			ReadLFBuf(fileAddr, recordList[i],0x0D,tempBuf,len-6);   //Serial Number,Message Identifier,Data Coding Scheme,Page Parameter,Content of Message

			WriteTFBuf(G_EnvelopHanderAddr,offset,tempBuf,len-6);
			SetLFConst(fileAddr, recordList[i],0x00,0x00,2);                               //清除CB 记录文件
			
			offset += (len-6);               //offset表示在G_EnvelopHanderAddr的数据偏移量
		}
	}

	if(event == ToolkitCBUnFormattedEvent)
	{	
		ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength = offset;

		G_EnvelopeTag = TAG_CB_DOWNLOAD;

		JavaToolkitProcess(EVENT_UNFORMATTED_SMS_CB_bit,0);

		return;
	}
	else
	{
		if((ccLength == 0x08)||(ccLength == 0x04))  //mac
		{
			ReadTFBuf(G_EnvelopHanderAddr,startOffset + 14 + 2,&mac[0],ccLength);	 //拷贝解密后的mac
			ecpMem_Set(cc,0x00,sizeof(cc));

			result = ToolkitCalculateMac(G_EnvelopHanderAddr+sizeof(TranFileHead)+startOffset,spiKicKid[0],spiKicKid[2],spiKicKid[3],2,ccLength,cpl+2,cc);
	
			if((result==FALSE)||(ecpMem_Compare(mac,cc,ccLength)!=EQUAL))
			{
				return;
			}
		}

		ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength = offset;

		G_EnvelopeTag = TAG_CB_DOWNLOAD;

		JavaToolkitProcess(EVENT_FORMATTED_SMS_CB_bit,0);	
	}							
}



 /*@Function:    CB_Download_Hanlder                                            
 * 
 *@Descrip:     该函数存储CB信息，包括EVENT_FORMATTED_SMS_CB, EVENT_UNFORMATTED_SMS_CB
 *
 *@See:	        ts_143019v050600p.pdf 6.2 
 *                                                                                                                                     
 *@Param:       NULL 
 *				                                                                                                         
 *@Return:      NULL                                                                                                                                                                                                                                                                      
 * 
 *@Author:      XWH 20130910
 *  
 *@History:     V0.0---Create by XWH 20130910     
 * 
 *              V0.1---Modify by XWH 20140214,支持一个包多条CB 的处理
 *
 *              V0.2---Modify by xwh 20170505,处理page parameter = 00的情况 (当做 11 处理)
 */
void CB_Download_Hanlder(void)
{
	BYTE  idata iniptr,ptr;
	WORD  idata MID,SN;
	BYTE  idata recNum,i,freeRecno;
	BYTE  xdata  tmp[6];
	UWORD xdata fileAddr;

	iniptr = FindTag(&RecData[0], 0xD2, P3);                                              //iniptr指向D2长度

	ptr= FindTag(&RecData[iniptr+1], TLV_CELLBROADCASTPAGE, RecData[iniptr]) + iniptr+1;  //ptr指向0C长度

	if(RecData[ptr]!=0x58)
	{
		return;
	}
	
    if(((RecData[ptr+6]&0x0F)==0x00)&&(RecData[ptr+7] == 0x00)&&(RecData[ptr+8] == 0x00))	//"0C58 000000000000000000000000000" page parameter = 00 2￠?ò cpl = 0000
	{
		WriteTFBuf(G_EnvelopHanderAddr,0x00,&RecData[iniptr+1],RecData[iniptr]);
		ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength = RecData[iniptr];
	
		G_EnvelopeTag = TAG_CB_DOWNLOAD;
		JavaToolkitProcess(EVENT_UNFORMATTED_SMS_CB_bit,0);
		return;
	}
	
	if(((RecData[ptr+6]&0x0F)==0x00)||((RecData[ptr+6]&0xF0)==0x00)) //?ù?Y03.41 9.3.2.4,if a mobile receives the code 0000,treat as 0001 0001(a single page message)
	{
		RecData[ptr+6] = 0x11;
	}


	fileAddr = SelectPrivateLFFile(DF_Toolkit, EF_EnvelopeCBMessage);
	
	if(fileAddr==FALSE)
	{
		return;
	}

	recNum = GetLFRecNumLgth(fileAddr,FileNum);
	freeRecno = 0x00;

	for(i=1 ;i <= recNum; i++)
	{
		ReadLFBuf(fileAddr, i,0x00,&tmp[0],2);

		if((tmp[0] == 0x00)&&(tmp[1] == 0x00))
		{
			//查找一条空记录用于存放当前CB
			if(freeRecno==0)
			{
				freeRecno = i;
			}
		}
		else
		{
			ReadLFBuf(fileAddr, i,0x07,&tmp[0],sizeof(tmp));

			if((ecpMem_Compare(&RecData[ptr+1],&tmp[0],4)==EQUAL)&&(RecData[ptr+6]==tmp[5])) //Serial Number,Message Identifier,Page Parameter相同，是重复短信
			{
				return;
			}
		}				
	}

	if(freeRecno==0)   //没有空记录, 清除与当前包不是同一包的其他CB
	{
		for(i=1 ;i <= recNum; i++)
		{
			ReadLFBuf(fileAddr, i,0x00,&tmp[0],2);

			if((tmp[0] != 0x00)&&(tmp[1] != 0x00))
			{
				ReadLFBuf(fileAddr, i,0x07,&tmp[0],sizeof(tmp));

				if(ecpMem_Compare(&RecData[ptr+1],&tmp[0],4)!=EQUAL) //Serial Number,Message Identifier 不相同，不是同一包
				{
					SetLFConst(fileAddr, i,0x00,0x00,2);	
					
					if(freeRecno==0) 
					{
						freeRecno = i;
					}
				}															
			}
		}		
	}

	if(freeRecno==0)   //如果还是没有空记录, 清除所有CB
	{
		for(i=1 ;i <= recNum; i++)
		{			
			SetLFConst(fileAddr, i,0x00,0x00,2);																											
		}

		freeRecno = 1;		
	}

	WriteLFBuf(fileAddr,freeRecno, 0, &RecData[iniptr], RecData[iniptr]+1);

	SN = (RecData[ptr+1]<<8)|RecData[ptr+2];
	MID = (RecData[ptr+3]<<8)|RecData[ptr+4];

	//若包已收齐，触发applet，否则退出
	ToolkitCBExcute(SN,MID,RecData[ptr+6]);

	ecpMem_Set(G_TARRecIndex, 0x00, sizeof(G_TARRecIndex));
}
