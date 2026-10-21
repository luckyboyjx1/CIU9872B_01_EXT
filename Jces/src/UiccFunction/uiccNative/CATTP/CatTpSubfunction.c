/*
*@history: V1.0 修改函数 CATTPSequenceSDUData ，修改G_LASTSDU_NB为FFFF时 ，执行不成功的问题，由于i = 0xFFFF时，下一步 ++会越界@20160524 by xuyz
           V1.1 修改函数CATTPProcessSDUData,当收完一个SDU准备处理时,都要把全局变量执行G_LASTSDU_Flag = FALSE重新置位   		  @20190510 by yangjian

*/
#include  "typedef.h"
#include  "main.h"
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
#include  "LDFile.h"
#include  "Transaction.h"
#include  "CompactManage.h"

#ifdef CATTP

#include "CATTP.h"

//WORD xdata  G_CurrentCommandOffset;  //当前APDU 指令在文件中的偏移
//BYTE xdata  G_CurrentComNUM;         //当前指令执行个数
//BYTE xdata  G_PORTag;               //判断当前是否需要发送POR
//BYTE xdata  G_PORDeliverSubmit;      //判断上行响应是  SMS-Deliver 还是 SMS-Submit  根据测试部陈科SMS-Deliver  只取一条数据，不在连续多条C0取响应数据。影响手机兼容性 

BYTE xdata G_EventTag;

BYTE xdata G_PushCommand;

/*
*@DESCRIPTION :处理timer 事件
*
*@param  : void
*
*@return :void
*
*@hitory :
*
*@author: create by xuyz
*/
//void TimerExpiration(void)
//{
//	G_EventTag = TRUE;

//	PendingMessageAlert(D_TimerExpiration,0x00);

//}



/*
*@description :处理80 EC指令
*
*function    :PushCommand
*
*@param      ：void
*
*@return     :void
*
*@author     :xuyz
*
*@history    :V1.0:create @20160819
*/
void PushCommand(void)
{
	UWORD xdata fileAddr;

	if (((P1 != 0x01)&&(P1 != 0x80))||((P2 != 0x01)&&(P2 != 0x02)))
	{
		P1P2Error();
		return;
	}

	ReceiveData();

	G_PushCommand = TRUE;
	
	fileAddr = SelectLFFile(DF_STK,EF_CATTP);
	
	SetLFConst(fileAddr,P2, 0x00,P3,sizeof(BYTE));
	
	WriteLFBuf(fileAddr,P2, 0x01,&RecData[0],P3);
	
	CommandFinish(0x00);
}

/*
*@description:查找最小的PDU SEQ/某个SEQ，返回其偏移值
*
*@param
*
*return
*/
WORD CATTPSearchSEQ(WORD currentSeq)
{
	WORD idata usedSpace,i,pduLen,offset,min,seq;
	UWORD xdata pdufileAddr;
	
	pdufileAddr = SelectTFFile(DF_OTA,LDFile_RelaxFile);           //原7E06--->5E03
	
	ReadTFBuf(pdufileAddr, 0x00, (BYTE *)&usedSpace,sizeof(WORD));
	
	min = 0x00;

	offset = FALSE;
	
	for(i = 0 ; i<usedSpace;i += pduLen+4)
	{
		ReadTFBuf(pdufileAddr , i+2 , (BYTE*)&seq,sizeof(WORD));
		
		ReadTFBuf(pdufileAddr , i+4 , (BYTE*)&pduLen,sizeof(WORD));
		
		if(currentSeq == 0x00)  //查找最小的SEQ偏移所在位置
		{		
			if(i == 0)
			{
				min = seq;
			
				offset = i+2;
			}
			else
			{
				if(seq < min)
				{
					min = seq;

					offset = i+2;
				}
			}
			
		
		}
		else
		{
			if(currentSeq == seq)
			{
				offset = i+2;
				break;
			}
		}	
	}
	
	if(currentSeq == 0x00)
	{
		return offset;
	}
	else
	{
		if(i == usedSpace)
		{
			return FALSE;
		}
		else
		{
			return offset;
		}
	}
}
/*
*@description:对SDU data进行整理，顺序排放,从CPI开始,EF_PDUFILE文件在退出时清空
*
*@function   :CATTPSequenceSDUData
*
*@param      :void
*
*@return     :TRUE/FALSE
*
*@author     :
*
*@history    :create by xuyz@20160819
*/
BYTE CATTPSequenceSDUData(void)
{
	WORD idata pduLen,actual;
	WORD idata usedSpace,dataSize ,min,offset;
	UWORD xdata pdufileAddr,dataAddr;
	UWORD xdata i;
	
	pdufileAddr = SelectTFFile(DF_OTA,LDFile_RelaxFile);          //原7E06存放PDU --> 5E03存放 
	
	//dataAddr = SelectTFFile(DF_OTA,0x6EAF);		                  //原顺序排放存7E03-->6EAF
	
	dataAddr = BIPModule_Receive_ADDR_START;											//for test
	
	ReadTFBuf(pdufileAddr, 0x00, (BYTE *)&usedSpace,sizeof(WORD));
	
	i = CATTPSearchSEQ(0x00);	 //查找最小的SEQ偏移所在位置
	
	ReadTFBuf(pdufileAddr, i, (BYTE *)&min,sizeof(WORD));
	
	dataSize = 0x00;	

	if(min > G_LASTSDU_NB)
	{		
		return FALSE;
	}
	
	
	for(i = min ;i<= G_LASTSDU_NB; i++)
	{
		offset = CATTPSearchSEQ(i);
		
		if(offset == FALSE)
		{
			return FALSE;
		}
		
		ReadTFBuf(pdufileAddr,offset+2,(BYTE *)&pduLen,sizeof(WORD));
		
			
		while(pduLen >0)
		{
			if ( pduLen > 0xFF )
			{
				actual = 0xFF;
			}
			else
			{
				actual = pduLen;
			}
			
			ReadTFBuf(pdufileAddr,offset+4, &tempBuf[0], actual);
			
			offset += actual;
			
			//WriteTFBuf(dataAddr,dataSize,&tempBuf[0] ,actual);
			
			WriteEeprom(dataAddr+dataSize,&tempBuf[0],actual);				//for test
			
			dataSize  += actual;		
			
			pduLen -= actual;		
		}	
	}
	
	SetTFConst(pdufileAddr, 0x00, 0x00,0x08);           
	
	return TRUE;
}
/*
*@description:
*
*@param
*
*@index
*/
WORD CATTPDataIsReceived(BYTE index)
{
	UWORD xdata fileAddr;
	BYTE  idata i;
	WORD idata usedSpace,totalUsedSpace,offset;
	
	fileAddr = SelectTFFile(DF_OTA,EF_CATTPSave);
	
	if(fileAddr == FALSE)  //缓存文件出错
	{
		return FALSE;
	}
	
	ReadTFBuf(fileAddr, 0x00, (BYTE *)&totalUsedSpace,sizeof(WORD));
	
	for(offset = 0x02;offset < totalUsedSpace; offset +=usedSpace+3)
	{
		ReadTFBuf(fileAddr,offset,(BYTE*)&i,sizeof(BYTE));
		
		if(i == index)
		{
			return offset;
		}
		
		ReadTFBuf(fileAddr,offset+1,(BYTE*)&usedSpace,sizeof(WORD));
		
	}
	
	return FALSE;
}
/*
*@description
*
*@param
*
*@history
*/
BYTE CATTPSavePRLData(UWORD dataAddr, WORD dataOffset,WORD len,BYTE index)
{
	UWORD xdata fileAddr;
	WORD idata offset,saveLen,usedSpace,dataLen;
	
	fileAddr = SelectTFFile(DF_OTA,EF_CATTPSave);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	dataLen = len;  //备份数据长度
	
	ReadTFBuf(fileAddr, 0x00, (BYTE *)&usedSpace,sizeof(WORD));
	
	WriteTFBuf(fileAddr,usedSpace+2,&index, sizeof(BYTE));	              //写 序号
	
	WriteTFBuf(fileAddr,usedSpace+3,(BYTE *)&dataLen, sizeof(WORD));	  
	
	offset = 0x05;
	
	while(dataLen > 0)
	{
		saveLen = (dataLen <= 240)?dataLen:240;  //每次读取240字节进行保存
		
		ReadTFBuf(dataAddr,dataOffset,&tempBuf[0], saveLen);

		WriteTFBuf(fileAddr,usedSpace+offset,&tempBuf[0], saveLen);
		
		offset += saveLen;
		
		dataOffset += saveLen;

		dataLen -= saveLen;
	}
	
	usedSpace += len+3;  //总长度变长
	
	WriteTFBuf(fileAddr, 0x00, (BYTE *)&usedSpace,sizeof(WORD));
	
	return TRUE;
}
/*
*@descriptoon:
*
*@param
*
*@history
*/
BYTE CATTPDataIsReceivedAll(void)
{
	BYTE idata i;
	
	for(i=1 ;i <= G_CATTPCurrentTotalNumber; i++)
	{
		if(CATTPDataIsReceived(i) != FALSE)  //存在第i条PRL数据
		{
		}
		else
		{
			return FALSE;
		}
	}
	
	return TRUE;
}

/*



*/

BYTE BIPPRLSPICheck(BYTE firstSPI,BYTE secondSPI)
{	
	BYTE idata check1, check2;
	
	check1 = firstSPI & 0x03;			//检查校验算法安全级别(目前只判断这个安全级别必须02或以上)
	
	check2 = 0x02;
	
	if (check1 < check2)
	{
		return OtherError;
	}

	check1 = secondSPI & 0x03;			//检查POR返回级别
	
	check2 = 0x01;
	
	if (check1 < check2)
	{
		return OtherError;
	}

	return FALSE;
		
}


/*
*@description:
*
*@function
*
*@param
*/
BYTE CATTPSDUDataDecryAndCheck(void)
{
	BYTE idata i,k,result,ccLgth,pcntr;
	WORD idata cplLen,offset,decryLen,dataLen,tmpLen,rfmoffset;
	BYTE xdata tmp[0x10],key[0x10],firstcpl[0x03];
	BYTE xdata cc[0x08];
	UWORD xdata dataAddr,fileAddr;
	BYTE xdata tag[0x04];
	BYTE idata processResult;
	BYTE idata spiStatus;
			
//	dataAddr = SelectTFFile(DF_OTA,EF_CATTPTempFile);     

	dataAddr = SelectTFFile(DF_OTA,EF_OTABipTempFile);        //原7E03存放顺序PDU——->6EAF
	
	fileAddr = SelectTFFile(DF_OTA,LDFile_RelaxFile);  //供 RFM使用 
	
	ReadTFBuf(dataAddr , 0x01, &firstcpl[0],sizeof(firstcpl));//cpi  cpl chl spi kic kid tar cnte pcntr cc ...
	
	if(firstcpl[0] == 0x82)		                        //82 L1 L2 总长度 3
	{
		firstcpl[0] = 0x02;
		cplLen = (firstcpl[1]<<0x08)| firstcpl[2];		
	}
	else if(firstcpl[0] == 0x81)                       // 81 L   总长度 2
	{
		firstcpl[0] = 0x01;
		cplLen = firstcpl[1];
	}
	else                                             //    总长度 1
	{
		cplLen = firstcpl[0];
		firstcpl[0] = 0x00;
	}
	
	G_PORTag = FALSE;

	pcntr = 0x00;
	
	processResult = POR_OK;
	
	ReadTFBuf(dataAddr,0x02+firstcpl[0],&tmp[0x00], 0x08);  //读取CHL SPI KIC KID TAR
	
	if(IsRFMTar(&tmp[5]) == TRUE)  //当前是RFM tar
	{
		G_PORTag = 0x03;
		
		result = LDFileSPICheck(tmp[1]);				//安全级别检测
	
		if (result != FALSE)		
		{
			if(processResult == POR_OK)
			{
				processResult = OtherError;	            //记录错误，程序继续往下执行
			}
		}
	}
	else   //是否为PRL tar
	{
		if((tmp[5] !=0xBF)||(tmp[7] !=0xFF)||(tmp[6] == 0x00)||(tmp[6] > 0x03))   //tar的取值范围是BF01FF BF02FF BF03FF 
		{
			if(processResult == POR_OK)
			{
				processResult = TAR_Unkown;
			}
		}

		if(processResult == POR_OK)
		{
			spiStatus = BIPPRLSPICheck(tmp[1],tmp[2]);

			if(spiStatus == OtherError)
			{
				processResult = OtherError;	
			}
		}
		
		/*
		if ((tmp[1] != 0x06)||(tmp[2] != 0x01))  //spi的值不为0601	  电信实网测试不需要此限制	
		{
			if(processResult == POR_OK)
			{
				processResult = OtherError;	                    //记录错误，程序继续往下执行
			}
		}
		*/
	}
	
	//kid kid不支持单des算法
	//Ciphering前提下判断加密模式不是DES in CBC mode和DES in ECB mode
	//Cryptographic和Checksum Digital Signature前提下判断MAC模式不是DES in CBC mode	                                                          
	if(((((tmp[3]&0x0C) == 0x00)||((tmp[3]&0x0C) == 0x0C))&&((tmp[1]&0x04) == 0x04))    
	||(((tmp[4]&0x0C) == 0x00)&&((tmp[1]&0x03) != 0x00)&&((tmp[1]&0x03) != 0x01)))         
	{
		if(processResult == POR_OK)
		{
			processResult = UnSecurityError;	                    //记录错误，程序继续往下执行
		}
	}
	
	if((tmp[1]&0x03) == 0x01)  //冗余校验  即CRC32
	{
		ccLgth = 0x04;
	}
	else if((tmp[1]&0x03) == 0x02)  //加密校验
	{
		ccLgth = 0x08;
	}
	else
	{
		ccLgth = 0x00;
	}
	
	if((tmp[1]&0x04) == 0x04)
	{ 
		 offset = 10 +firstcpl[0]; //待解密数据的起始偏移，从CNTR开始 CNTR PCNTR CC DATA
		 
		 decryLen = cplLen - 8;	   //CHL + SPI + KIC + KID +TAR = 8
		 
		 k = 0;
		 
		 while(decryLen >0)
		 {
		 	dataLen = (decryLen <= 240)?decryLen:240;   //每次最大读取240字节进行解密
		 	
		 	ReadTFBuf(dataAddr,offset,&tempBuf[0], dataLen);
		 	
		 	if((tmp[3]&0x0C) != 0x0C)
		 	{
		 	
		 		if(k ==0)
				{	
					memcpy(&RecData[0],&tempBuf[dataLen-8], 0x08);  //保存解密前的最后8字节数据
				}
				else
		 		{
			 		memcpy(&RecData[8],&tempBuf[dataLen-8], 0x08);  //保存解密前的最后8字节数据
			 	}
			 }
		 			
			result = CATTPChipheRe(tmp[3],&tempBuf[0],&tempBuf[0],DECRYPT,dataLen); //从CNTR开始解密
			
			if(result != POP_OK)
			{
				if(processResult == POR_OK)
				{
					processResult = result;	                    //记录错误，程序继续往下执行
				}
			}
		 	
		 	if(k == 0x00)
		 	{
		 		k ++;

				pcntr = tempBuf[5];
		 	}
		 	else
		 	{
		 		if((tmp[3]&0x0C) != 0x0C)
		 		{
			 		for(i = 0; i <8;i++)
					{
						tempBuf[i] = tempBuf[i]^RecData[i];
					}		 
					 
					memcpy((BYTE  *)&RecData[0], (BYTE   *)&RecData[8], 8);
				}
			}
						
		 	
		 	WriteTFBuf(dataAddr,offset,&tempBuf[0], dataLen);
		 	
		 	offset += dataLen;
		 	
		 	decryLen -= dataLen;
		 }	
	} 
	
//	if(G_PORTag == 0x03)  //对于RFM数据来说，由于目前兼容的是数据短信的处理方式，因此需要把解密后的数据写入更新进文件5E03中，以便可以和短信流程同时处理数据
//	{
//		offset = 0x0E;  //先从第14字节的位置将数据写入5E03中，，5E03的格式为结构为：L(2字节) + SPI(两字节) + KIC(1字节) + KID(1字节) +TAR(3字节) + counter(此次，5字节) + vlaue(N字节，不大于512字节)
//		
//		decryLen = 16+firstcpl[0] + ccLgth; //复用，指向 读取数据的数据区 
//		
//		tmpLen = cplLen-0x0E-ccLgth-pcntr; //有效数据长度
//		
//		while(tmpLen >0)
//		{
//			dataLen = (tmpLen <= 240)?tmpLen:240;   //每次最大读取240字节进行更新
//			
//			ReadTFBuf(dataAddr,decryLen, &tempBuf[0], dataLen)	;  //
//			
//			WriteTFBuf(fileAddr,offset,&tempBuf[0],dataLen);
//			
//			offset += dataLen;
//			
//			decryLen += dataLen; 
//
//			tmpLen -=  dataLen;
//		}
//		
//		ReadTFBuf(dataAddr, 3+firstcpl[0],&tempBuf[0] , 0x0C);  //读取SPI1 SPI2 KIC KID TAR(3字节) COUNTER(5字节)
//		
//		WriteTFBuf(fileAddr,0x02,&tempBuf[0],0x0C);
//		offset =  firstcpl[0] +2;                //从CHL  开始读取数据  存放在 5E03
//
//		tmpLen = cplLen-0x0E-ccLgth-pcntr; //有效数据长度
//
//			
//	}
	
	//判断mac	
	ReadTFBuf(dataAddr,16+firstcpl[0], &cc[0], ccLgth)	;  //读取mac

	ReadTFBuf(dataAddr, 0x00,&tempBuf[0],16+firstcpl[0]);

	WriteTFBuf(dataAddr, ccLgth,&tempBuf[0],16+firstcpl[0]);  //将CPI后移ccLgth个字节计算mac
	
	SetTFConst(dataAddr,0x00,ccLgth,sizeof(BYTE));			  //保存CC长度  上发响应时候需要

	if(G_PORTag == 0x03)
	{
		offset = firstcpl[0] +2 + ccLgth;	   //读取CPL后面的数据，CPL长度在RFM操作返回前直接写在5E03前两个字节  firstcpl[0] +2 = CPI + CPL

		tmpLen = cplLen-ccLgth-pcntr; //有效数据长度

		rfmoffset = 0x02+ccLgth;   //在5E03 的偏移  为了和RFM  解密数据格式一样  偏移 02 + CCLength	  这里默认CPL长度为2字节！！！！最后CPL长度写在5E03第1-2字节

		while(tmpLen>0)
		{
			dataLen = (tmpLen <= 240)?tmpLen:240;   //每次最大读取240字节进行更新

			ReadTFBuf(dataAddr,offset, &tempBuf[0], dataLen);

			WriteTFBuf(fileAddr,rfmoffset,&tempBuf[0],dataLen);
			
			offset += dataLen;
			
			rfmoffset += dataLen;
			
			tmpLen -= dataLen;	
		}		
	}

	if(processResult != POR_OK)
	{
		return processResult;
	}

	if((tmp[1]&0x03) == 0x02)
	{
		result = CATGetRFMKey(((tmp[4]&0xF0)>>0x04),&key[0],MODE_KID);                 //获取计算mac key   ，mac计算从CPL开始，不包括CC数据

		if(result == FALSE)
		{
			if(processResult == POR_OK)
			{
				processResult = Securit_Error;	                    //记录错误，程序继续往下执行
			}
		}
		
		if((tmp[4] & 0x03) == 0x01)                          			//DES
        { 
			if((tmp[4]&0x0C)==0x00)
			{			    
        DesMacFunction(getFileContentAddr(dataAddr)+ccLgth,&key[0x00],tempBuf,cplLen-ccLgth+firstcpl[0]+2,SINGLEDES);  //cPacketLgth-8+2 = CPL- CClength + CPL本身的字节长度,计算MAC从CPL开始

				
      		}
			else if(((tmp[4]&0x0C)==0x04)||((tmp[4]&0x0C)==0x08))
			{			    
				DesMacFunction(getFileContentAddr(dataAddr)+ccLgth,&key[0x00],tempBuf,cplLen-ccLgth+firstcpl[0]+2,TRIPLEDES);  //cPacketLgth-8+2 = CPL- CClength + CPL本身的字节长度,计算MAC从CPL开始

			}
			else
			{
					if(processResult == POR_OK)
					{
							processResult = Securit_Error;
					}
      		}
		}

		if(memcmp(&tempBuf[0],&cc[0],ccLgth) != EQUAL)
		{	
			if(processResult == POR_OK)
			{
					processResult = CC_Failed;
			}
		}
	}  
	
	if(G_PORTag == 0x03)    // 对于CATTPRFM操作直接返回
	{
	    G_CurrentCommandOffset = tmp[0] + 0x03 ;  //CPL 2  CHL  1	 tmp[0]= CHL
		
		cplLen = cplLen-pcntr; //有效数据长度

		tmp[0] = (BYTE)((cplLen >> 0x08) & 0xFF);	//添加长度
	
		tmp[1] = (BYTE)(cplLen & 0xFF);
	
		WriteTFBuf(fileAddr, 0x00, (BYTE *)tmp, 0x02);		//写入缓存文件前端

		return processResult;
	}
	else  //执行CATTP PRL操作
	{
			//判断PRL DowloadData数据的tag，len是否有效
  	  ReadTFBuf(dataAddr,16+firstcpl[0]+ccLgth, &tag[0], 0x04);	 //读取Packet Number Tag、Packet Number Len、Total Number、Current Number
	
	  if((tag[0] != 0x80) ||(tag[1] != 0x02) ||(tag[2] <tag[3]))  //PRL Dowload tag 、len error or totalNumber <currentNumber
	  {
		return Cipher_Error;
	  }

	  if(tag[3] == 0x01)  //重新收到一组新的数据
	  {
		G_CATTPPreNumber = FALSE;

		G_CATTPCurrentTotalNumber = FALSE;

		CATTPClearFile(EF_CATTPSave);
	  }

	  if(G_CATTPCurrentTotalNumber == FALSE)  //当前收到的是地一包有效的PRL数据
	  {
		memcpy(&G_CATTPTar[0],&tmp[5], 0x03);

		G_CATTPCurrentTotalNumber = tag[2];

	  }
	  else if(G_CATTPCurrentTotalNumber != tag[2]) //当前收到的是另外一包的数据
	  {	
		return Cipher_Error;
	  }


	 if(memcmp(&tmp[5],&G_CATTPTar[0], 0x03) != EQUAL)
	 {
		return Cipher_Error;
	 }

	 //将有效的PRL数据缓存起来，待数据全部收全后再进行PRL更新
	
	 result = CATTPDataIsReceived(tag[3]);
	
	 if(result != FALSE)  //当前index数据已收到，暂时处理为抛弃
	 {
		return Cipher_Error;
		
	 }
	
	 if(tag[3] != (G_CATTPPreNumber +1))	  //PRL短信不是按顺序来的，则报错，	  如果不要该判断，则支持PRL乱序数据过来
	 {
			return Cipher_Error;
	 }

	 G_CATTPPreNumber = tag[3];
	
	 G_CurrentCommandOffset = tmp[0] + 3+ firstcpl[0] +4;  //指向第一条APDU指令偏移	 4=Download Data Packet Number Tag + 包长度 + 总包数+此条包序号

	 result = CATTPSavePRLData(dataAddr, G_CurrentCommandOffset,cplLen-0x0E-ccLgth-pcntr-4,tag[3]);	//0E=CHL +SPI + KIC +KID +TAR + CONTR + PCNTR
	
     CATTPClearFile(LDFile_RelaxFile);
	
	 if(result == FALSE)
	 {
		return Cipher_Error;
 	 }
	
	 G_PORTag = CATTPDataIsReceivedAll();  //FALSE表示当前短信未收全，TRUE 表示所有短信已收全
	
	 return POR_OK;
	}
}
/*
*@description: 在CATTP 下上发RFM POR
*
*@function  :CATTPResponseRFMPor
*
*@param     :errorTag
*
*@return    :void
*/
void CATTPResponseRFMPor(BYTE errorTag)
{
	UWORD xdata fileAddr, fileAddr2;
	BYTE xdata spikickid[0x0c],leninfo[0x02];
	WORD xdata len;
	BYTE idata k;
	
	fileAddr2 = SelectTFFile(DF_OTA, LDFile_RelaxFile);	//选择执行缓存文件
	
	if (fileAddr2 == FALSE)
	{
		return;
	}
	
	ReadTFBuf(fileAddr2, G_CurrentCommandOffset-13, spikickid, 0x0c);	//读取SPI
	
	if ((spikickid[1] & 0x03) != 0x01)			//PoR存在检查
	{
		if (((spikickid[1]  & 0x03) != 0x02) || (errorTag == PoROK))
		{
			GPRAMRFMExtendResponsePublic(TAG_ClearResponseFile);

			CATTPSendRequestInOpenState(NULL);  //当不需要发送POR时，发送空的ACK, by xwh 20160321

			if(G_EarlyResponseTag == 0x11)  //当前响应为early response且存在refresh指令
			{
				cephRefreshSingle(0x04);;
			}
			
			G_TARRecIndex[1] = FALSE;		//清除LDFile标记

			
			return;
		}
	}

	fileAddr = SelectTFFile(DF_OTA, LDFile_UpFile);

   	ReadTFBuf(fileAddr, RFM_RES_LEN_OFF, (BYTE*)&leninfo, 0x02);	

	len = BuildWord(&leninfo[0]); // (((WORD)leninfo[0] << 8)&0xFF00) +  leninfo[1];

	if(len>0xD0)
	{		
 	   len = 0xD0;                                 //对于CATTP中执行RFM操作压缩格式的时候（扩展格式在执行下行的时候已经截取），目前不支持级联返回，ACK 头长 0x12,	   	
	}
	
	RecData[1] = 0x02 ;  //RPI

	RecData[3] = 0x0A;  //RHL
	
	RecData[2] = RecData[3] +1;
			
	memcpy(&RecData[4],&spikickid[4],0x08);  //copy tar CNTR

	RecData[0x0C] = 0x00;  //PCNTR
		
	if(errorTag == POR_OK )  
	{
		RecData[0x0D] = 0x00;
		
		if(spikickid[5] == 0x01)                       //扩展执行时候 需要加 tag AB	  长度字节
		{
			RecData[0x0E] = 0xAB;

			if(len>=0x80)
			{
				RecData[0x0F] = 0x81;
				
				RecData[0x10] = len;
				
				k=0x01;	
			}
			else
			{
				RecData[0x0F] = len;
				
				k= 0x00;	
			}

			ReadTFBuf(fileAddr,RFM_RESPONSE_COMNUM_OFF,&RecData[0x10+k],len);  //注意考虑多条发送的情况
			
			RecData[2] =RecData[2] + len + 2+k;	                       //2 = 长度字节  +tag  (AB)	  扩展执行
		}
		else                                           //压缩执行  则直接读取返回数据
		{
			ReadTFBuf(fileAddr,RFM_RESPONSE_COMNUM_OFF,&RecData[0x0E],len);  //注意考虑多条发送的情况
			
			RecData[2]  += len;                       //压缩执行    两个长度字节不需要			
		}

	}
	else                  
	{
		RecData[0x0D] = errorTag;
		
	}

	if(RecData[2]>=0x80)
	{
		memmove((BYTE *)&RecData[3], (BYTE *)&RecData[2], RecData[2]+1); 
		
		RecData[2] = 0x81; 

		RecData[0] = RecData[3] + 2+1;
	}
	else
	{
		RecData[0] = RecData[2] + 2;	
	}

	GPRAMRFMExtendResponsePublic(TAG_ClearResponseFile);
	
	CATTPSendRequestInOpenState(&RecData[0]);

	if(G_EarlyResponseTag == 0x11)  //当前响应为early response且存在refresh指令
	{
		cephRefreshSingle(0x04);
	}
	
	G_TARRecIndex[1] = FALSE;		//清除LDFile标记
		
}
/*
*@description:上发响应信息
*
*·param    :errorCode:0x00-成功
*
*·return
*
*@history
*/

//该函数为标准的102225指令，POR数据从文件中获取
void CATTPResponsePOR(BYTE errorCode,BYTE mode)
{
	BYTE idata cclength;
	BYTE xdata firstcpl,spikickidtar[0x10];
 	UWORD xdata dataAddr;
 	
 	
 	dataAddr = SelectTFFile(DF_OTA,EF_OTABipTempFile);        //原7E03 --->6EAF
 	
 	if(dataAddr ==FALSE)
 	{
 		return ;
 	}
 	
 	ReadTFBuf(dataAddr,0x00,&cclength,sizeof(BYTE));
		
	ReadTFBuf(dataAddr , 1+cclength, &firstcpl,sizeof(BYTE));//cpi  cpl chl spi kic kid tar cnte pcntr cc ...
	
	if(firstcpl == 0x82)
	{
		firstcpl = 0x02;
	}
	else if(firstcpl == 0x81)
	{
		firstcpl = 0x01;
	}
	else
	{
		firstcpl = 0x00;
	}
	
	ReadTFBuf(dataAddr, 1+cclength+firstcpl+2,&spikickidtar[0],13);  //读取SPI KIC KID TAR，CNTR PCNTR
	
	if(((spikickidtar[1]&0x03) == 0x03) ||((spikickidtar[1]&0x03) == 0x00))  //没有POR发送给SE 或保留 
	{
		return ;
	}

	
	RecData[1] = 0x02 ;  //RPI

	RecData[3] = 0x0A;  //RHL
	
	RecData[2] = RecData[3] +1;
			
	memcpy(&RecData[4],&spikickidtar[4],0x08);  //copy tar CNTR

	RecData[0x0C] = 0x00;  //PCNTR
	
	RecData[0x0D] = 0x00;
	
	if(errorCode == 0x00 )  //PRL 更新成功
	{
		RecData[0x0E] = 0x01;
		
		RecData[2] += 1;
	}
	else                   //PRL 更新失败
	{
		RecData[0x0E] = 0x00;
		
		RecData[2] += 1;
	}
	
	RecData[0] = RecData[2] +2;
	
	CATTPSendRequestInOpenState(&RecData[0]);

  CATTPStartTimer(0x01);

	if((mode == 0x01)&&(errorCode == 0x00))
	{	
		if((spikickidtar[4] == 0xBF)&&(spikickidtar[5] == 0x01)&&(spikickidtar[6] == 0xFF))
		{
				cephDisplayTextSingle(0x01,UCS2,(BYTE *)PRLSuccess);  //prl下载已完成
		}
		else if((spikickidtar[4] == 0xBF)&&(spikickidtar[5] == 0x02)&&(spikickidtar[6] == 0xFF))
		{
				cephDisplayTextSingle(0x01,UCS2,(BYTE *)EPRLSuccess);  //eprl下载已完成
		}
	}

	 cephMoreTimeSingleTelecom();
	
}
/*
*@description:在PRL执行或恢复过程中，根据tar值，获取不同的PRL文件地址
*
*@function  :GetPRLAddr
*
*@param     :void
*
*@return     :PrlAddr
*
*@author    :xuyz
*
*@history   :V1.0:create 1.0@20160818
*/
UWORD GetPRLAddr(void)
{
	UWORD xdata fileAddr;
	
	if(G_CATTPTar[1] == 0x01)  //更新的是7F25下的6F30 
	{
		fileAddr = SelectTFFile(0x7F25,0x6F30);
	}
	else if(G_CATTPTar[1] == 0x02)  //更新的是CSIM ADF 下的6F5A
	{
		fileAddr = SelectTFFile(G_DF_ID_CSIM,0x6F5A);
	}
	else   //更新的是USIM ADF下的6F61
	{
		fileAddr = SelectTFFile(G_DF_ID_USIM,0x6F61);
	} 
	
	return fileAddr;
}
/*
*@description:将缓存中EF_CATTPSave的数据更新到PRL 文件中，主要用于PRL 处理和PRL恢复的过程
*
*@function   :UpdatePRLData
*
*@param      :UWORD prlAddr
*@param      :UWORD fileAddr
*
*@return     :void
*
*@author     :xuyz
*
*@history    :V1.0:create @20160818
*/
void UpdatePRLData(UWORD prlAddr, UWORD fileAddr)
{
	
	WORD idata offset,sduLen,actual,dataSize,fileSize;
	BYTE idata i,initialValue;
	
	initialValue = 0x00;
	
	if(G_CATTPTar[1] == 0x03)  //0xBF 01 FF，更新7F25下的6F30/  0xBF 02 FF，更新CSIM ADF下的6F5A  / 0xBF 03 FF，更新USIM ADF下的6F61
	{
		initialValue = 0xFF;
	}
	
	fileSize = GetTFSize(prlAddr);
	
	dataSize = 0x00;
	
	for(i=1; i<= G_CATTPCurrentTotalNumber; i++)
	{
		offset = CATTPDataIsReceived(i);
		
		if(offset == FALSE)
		{
			return;
		}
		
		ReadTFBuf(fileAddr, offset+1, (BYTE*)&sduLen,sizeof(WORD));
		
		offset += 0x03  ;//point to数据
		
		while(sduLen >0)
		{
			actual = (sduLen > 255)?255:sduLen;
			
			ReadTFBuf(fileAddr,offset, &tempBuf[0], actual);
			
			offset += actual;
			
			WriteTFBuf(prlAddr,dataSize,&tempBuf[0] ,actual);
			
			dataSize  += actual;		
			
			sduLen -= actual;		
		}	
	
	}
	
	if(dataSize <fileSize)  //数据未写完，则将剩余数据清空 成文件的初始值
	{
		sduLen = fileSize - dataSize;
		
		while(sduLen >0)
		{
			actual = (sduLen > 255)?255:sduLen;
			
			SetTFConst(prlAddr,dataSize,initialValue ,actual);  //断电写，防止数据错误
		
			dataSize  += actual;	
		
			sduLen -= actual;		
		}
	}

}
/*
*@description:PRL数据收全后，更新ＰＲＬ数据　
*
*@param
*
*@history
*/
BYTE CATTPExcutePRLCommand(void)
{
	BYTE idata i;
	WORD idata offset,sduLen,dataSize,fileSize;
	UWORD xdata prlAddr,fileAddr,tagAddr;
	
	prlAddr = GetPRLAddr();
	
	if(prlAddr == FALSE)
	{
		return Securit_Error;
	}
	fileAddr =SelectTFFile(DF_OTA,EF_CATTPSave);  //该文件执行到这里是一定存在的否则前面更新缓存文件也会出错，因此此处不判断文件是否存在的有效性
	
	dataSize = 0x00;

	for(i=1; i<= G_CATTPCurrentTotalNumber; i++)
	{
		offset = CATTPDataIsReceived(i);
		
		if(offset == FALSE)
		{
			return Securit_Error;
		}
		
		ReadTFBuf(fileAddr, offset+1, (BYTE*)&sduLen,sizeof(WORD));

		dataSize += sduLen;
	}

	fileSize = GetTFSize(prlAddr);  //下行数据中的PRL数据超过了当前文件的总大小

	if(dataSize > fileSize)
	{
		return 	Securit_Error;
	}

	tagAddr =SelectTFFile(DF_OTA,EF_CATTPDuanDian);
	
	if(tagAddr == FALSE)
	{
		return Securit_Error;
	}
	
	tempBuf[0] = 0x01;
	 
	tempBuf[1] = G_CATTPCurrentTotalNumber;
	 
	memcpy(&tempBuf[2] ,&G_CATTPTar[0], 0x03);
	 
	WriteTFBuf(tagAddr, 0x00,tempBuf,0x05); //将断电恢复tag更新进指定文件

	UpdatePRLData(prlAddr,fileAddr);  //更新PRL数据到PRL文件中
	
  SetTFConst(tagAddr, 0x00, 0x00,0x05);
    
	return POR_OK;
	
}

/*
*@description:在开机时判断之前是否在进行CATTP PRL更新操作，如果是，且断电了，则重新进行更新
*
*
*/

BYTE code timerId[8] = {0x01,0x02,0x03,0x04,0x005,0x06,0x07,0x08};
void ResumePrlData(void)
{
	UWORD xdata prlAddr,fileAddr,tagAddr;
	
	tagAddr = SelectTFFile(DF_STK, EF_TIMER);	  //先清空timer

	 if(tagAddr != FALSE)
	 {
	 	ReadTFBuf(tagAddr,0x00,&tempBuf[0x10],0x08);

		if(memcmp((BYTE *)&timerId[0],&tempBuf[0x10], 0x08) != EQUAL)
		{
	 		WriteTFBuf(tagAddr,0x00,(BYTE *)&timerId[0],0x08);
		}
   }

	
	tagAddr =SelectTFFile(DF_OTA,EF_CATTPDuanDian);
	
	if(tagAddr == FALSE)
	{
		return ;
	}
	
	ReadTFBuf(tagAddr, 0x00,tempBuf,0x05);  //从PRL恢复区读取恢复tag
	
	if(tempBuf[0] == 0x00)   //tag无效，则清除当前的PRL缓存区域
	{
		memset(G_CATTPTar, 0x00,0x03);
		
		G_CATTPCurrentTotalNumber = FALSE;
		
		CATTPClearFile(EF_CATTPSave);
		return;
	}
	 
  G_CATTPCurrentTotalNumber = tempBuf[1] ;
	 
	memcpy(&G_CATTPTar[0], &tempBuf[2] , 0x03);
	 
	prlAddr = GetPRLAddr();
	
	if(prlAddr == FALSE)
	{
		return;
	}
	
	fileAddr =SelectTFFile(DF_OTA,EF_CATTPSave);
	
	if(fileAddr == FALSE)
	{
		return;
	}
	
	UpdatePRLData(prlAddr,fileAddr);

  SetTFConst(tagAddr, 0x00, 0x00,0x05);
    
  memset(G_CATTPTar, 0x00,0x03);
    
  G_CATTPCurrentTotalNumber = FALSE;

}

/*
*@description :一条完整的SDU数据接收完成后，对SDU数据进行处理,该函数处理完毕后，清空缓存SDU 的文件EF_PDUFILE
*
*@param       ：void
*
*@return      :TRUE/FALSE：返回结果 ，无意义 
*
*@history     :create by @20150512
*/
BYTE CATTPProcessSDUData(void)
{
	BYTE idata result;
	UWORD xdata fileAddr;

	G_LASTSDU_Flag = FALSE;	    //一个SDU被处理,重新置为FALSE
	
	if(CATTPSequenceSDUData() == FALSE)
	{
		return FALSE;
	}

	result = CATTPSDUDataDecryAndCheck();
		
	if(G_PORTag == TRUE)  //短信已全部收全 ,PRL短信收全
	{
		//G_LASTSDU_Flag = FALSE;	       	
		
		result = CATTPExcutePRLCommand();
		
		G_CATTPCurrentTotalNumber = FALSE;  //清空总包数
		
		CATTPClearFile(EF_CATTPSave);
		
		CATTPResponsePOR(result,0x01);

		return TRUE;
		
	}
	else
	{
			//G_LASTSDU_Flag = FALSE;	    //一个SDU处理成功

			if(G_PORTag == 0x03)
			{
				if(result == POR_OK)
				{
					G_Source_Type = SourceType_CATTP;
						
					fileAddr = SelectTFFile(DF_OTA, LDFile_RelaxFile);

					RemoteAPDUProcessFunction(fileAddr, 0x00, 0x00, RemoteDataEndBlock, NULL); 
					//RemoteAPDUProcessFunction(fileAddr, G_CurrentCommandOffset-SMS_Head_CplToPcntr, 0x00, RemoteDataEndBlock, NULL); 
					
//					ReadTFBuf(fileAddr, G_CurrentCommandOffset-0x08, (BYTE *)&type, 0x01);		//读取指令类型	  COUNTR + PCNTR +TAR2-3
// 
//					if (type == 0x01)		//扩展型执行
//					{
//						ExtendedRFMExcuteCommand();
//					}
//					else		//压缩型执行
//					{
//						CompressCMDProgress();
//					}

					G_TARRecIndex[1] = FALSE;
	
				}

				CATTPResponseRFMPor(result);

			  CATTPStartTimer(0x00);
			}
			else	  //对于PRL短信来说，所有的包未收全
			{
				CATTPResponsePOR(result,0x00);
			}

			G_PORTag = 	FALSE;   //执行CATTP RFM操作时候   G_PORTag = 0x03 ，并根据这个G_PORTag = 0x03  来截取ACK-POr数据，执行普通的SMS  Deliver也需要截取数据，只发送一条，此时判断依据 SPI 和  G_PORTag ！= 0x03
	}
	
	return result;
}
#endif
