#include  "typedef.h"
#include  "Uicc_main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include  "uicc_error.h"
#include  "arithmatic.h"
#include  "ceph.h"
//#include  "main.h"
#include  "ToolkitRFM.h"
#include  "stkalert.h"
#include  "uicc_interface.h"
#include  "UICCToolkitConstants.h"
#include  "CompactManage.h"
#include  "ExpandedManage.h"
#include   "UICCToolkitAPI.h"
#include   "FunctionSwitch.h"
#include "BIPModuleProcess.h"
#include  "ToolkitAPIPublic.h"
#include "CatTp.h"
#include "Crypto_Constant.h"
#include "CDMA.h"

#if UICC__FUNCTION__CATTP

void InitCattpVariable(void)
{
	G_CATTPType = TarType_Error;
	G_CurrentSDUForSendLength = 0x00;
	G_PDUSize[0] = 0;
	G_PDUSize[1] = 0;
}

BYTE xdata G_PushCommand;	


void PushCommand(void)
{
	UWORD xdata fileAddr;

	if (((P1 != 0x01)&&(P1 != 0x80))||((P2 != 0x01)&&(P2 != 0x02)))
	{
		P1P2Error();
		return;
	}

//	ReceiveData();
	
	G_CATTPType = G_TARRecIndex[0];

	G_PushCommand = TRUE;
	
	fileAddr = SelectPrivateLFFile(DF_CATTP,EF_CATTPPUSH); //zhhj
	
	if(fileAddr == FALSE)
	{
		 return ;
	}
	
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
	
	pdufileAddr = SelectPrivateTFFile(DF_CATTP,EF_PDUFILE); //zhhj
	
	if(pdufileAddr == FALSE)
	{
		return FALSE;
	}
	
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
*@description:对SDU data进行整理，顺序排放,从CPI开始
*/
BYTE CATTPSequenceSDUData(void)
{
	WORD idata pduLen;
	WORD idata usedSpace,i,dataSize ,min,offset;
	UWORD xdata pdufileAddr/*,dataAddr*/;
	
	pdufileAddr = SelectPrivateTFFile(DF_CATTP,EF_PDUFILE); //zhhj
	
	if(pdufileAddr == FALSE)
	{
		return FALSE;
	}
	
	ReadTFBuf(pdufileAddr, 0x00, (BYTE *)&usedSpace,sizeof(WORD));
	
	i = CATTPSearchSEQ(0x00);
	
	ReadTFBuf(pdufileAddr, i, (BYTE *)&min,sizeof(WORD));
	
	dataSize = 0x00;
	
	if(min > G_LASTSDU_NB)
	{
		SetTFConst(pdufileAddr, 0x00, 0x00,0x04);
		
		return FALSE;
	}
	
	for(i = min ;i<= G_LASTSDU_NB; i++)
	{
		offset = CATTPSearchSEQ(i);
		
		if(offset == FALSE)
		{
			SetTFConst(pdufileAddr, 0x00, 0x00,0x04);
			return FALSE;
		}
		
		ReadTFBuf(pdufileAddr,offset+2,(BYTE *)&pduLen,sizeof(WORD));
		
			
// 		while(pduLen >0)
// 		{
// 			if ( pduLen > 0xFF )
// 			{
// 				actual = 0xFF;
// 			}
// 			else
// 			{
// 				actual = pduLen;
// 			}
// 			
// 			ReadTFBuf(pdufileAddr,offset+4, &tempBuf[0], actual);
// 			
// 			offset += actual;
// 			
// 			WriteTFBuf(G_EnvelopHanderAddr,dataSize,&tempBuf[0] ,actual);
// 			
// 			dataSize  += actual;		
// 			
// 			pduLen -= actual;		
// 		}

		//数据不重叠，可以调用WriteTFFileNonAtomic
		WriteTFFileNonAtomic(G_EnvelopHanderAddr,dataSize,pdufileAddr+sizeof(TranFileHead)+offset+4,pduLen);
		
		dataSize += pduLen;
	}
	
	SetTFConst(pdufileAddr, 0x00, 0x00,0x08);
	
	return TRUE;
}
			
/*
*@description :
*
*@param  
*
*@history  modify by  xieyong@20211116  添加offset初始化，解决芯片随机出错问题
*/
BYTE CATTPSDUDataDecryAndCheck(void)
{
	BYTE idata result,ccLgth,pcntr;
	WORD idata cplLen,offset,decryLen;
	BYTE xdata tmp[0x10],firstcpl[0x03],spikickid[0x0E];
	BYTE xdata mac[0x08];
	BYTE idata 	processResult,blocksize;
	pcntr = 0x00;
	offset = 0x00;
	
	processResult = POR_OK;	
	
	blocksize = 0x08; //DES
#if UICC__CDMA__CATTP__PRL
	G_PRL = 0x00;  //不是PRL
#endif	
	ReadTFBuf(G_EnvelopHanderAddr , 0x01, &firstcpl[0],sizeof(firstcpl));//cpi  cpl chl spi kic kid tar cnte pcntr cc ...
	
// 	if(firstcpl[0] == 0x82)
// 	{
// 		firstcpl[0] = 0x02;
// 		cplLen = (firstcpl[1]<<0x08)| firstcpl[2];
// 	}
// 	else if(firstcpl[0] == 0x81)
// 	{
// 		firstcpl[0] = 0x01;
// 		cplLen = firstcpl[1];
// 	}
// 	else
// 	{
// 		cplLen = firstcpl[0];
// 		firstcpl[0] = 0x00;
// 	}
    cplLen = GetBerTlvLength(&firstcpl[0],&offset);

	firstcpl[0] = offset-2;
    
	ReadTFBuf(G_EnvelopHanderAddr,0x02+firstcpl[0],&tmp[0x00], 0x08);  //读取CHL(tmp[0])  SPI(tmp[1] tmp[2]) KIC(tmp[3]) KID(tmp[4]) TAR(tmp[5] tmp[6] tmp[7])	
	
//	processResult = CheckSPIRuleForPor(&tmp[1]); //该判断放在这里 以下两个用例测不过   ，TPX 5G放在下面检查    电信CMS2AC_test/SECURED/0348短信加固方案测试/CATTP_CompactCmd_Reinforce_1_1    CATTP_ExpandedRemoteCmd_Reinforce_1_1

	ecpMem_Copy(&G_ToolkitTar[0], &tmp[5], 0x03);  //tar

#if UICC__CDMA__CATTP__PRL  //PRL的TAR设置为0xBF 01 FF,EPRL的TAR设置为0xBF 02 FF,OPLMN的TAR设置为0xBF 03 FF
	if((G_ToolkitTar[0] == 0xBF)&&(G_ToolkitTar[2] == 0xFF)&&((G_ToolkitTar[1] >=0x01)&&(G_ToolkitTar[1]<=0x03))) 
	{
		G_PRL =G_ToolkitTar[1];    //代表此时是PRL更新
	}
	else
#endif
	{
		if(CheckTar(G_ToolkitTar,G_TARRecIndex) == TarType_Error)  //根据tar值，设置当前tar类型
		{
			processResult = TARUnknown;
		}
	}

	//kid kid不支持单des算法
	//Ciphering前提下判断加密模式不是DES in CBC mode和DES in ECB mode
	//Cryptographic和Checksum Digital Signature前提下判断MAC模式不是DES in CBC mode	                                                          
//	if(((((tmp[3]&0x0C) == 0x00)||((tmp[3]&0x0C) == 0x0C))&&(((tmp[1]&0x04) == 0x04)||(tmp[2]&0x10) == 0x10))   //tmp[1]&0x04 = 0x04: 1 : Ciphering   tmp[2]&0x10 = 0x10: 1 : PoR response shall be ciphered  
//	||(((tmp[4]&0x0C) == 0x00)&&( (((tmp[1]&0x03) != 0x00)&&((tmp[1]&0x03) != 0x01)) ||( ((tmp[2]&0x0C) != 0x00)&&((tmp[2]&0x0C) != 0x04) ))) )         
// 	if(((((tmp[3]&0x0C) == 0x0C))&&(((tmp[1]&0x04) == 0x04)||((tmp[2]&0x10) == 0x10)))     //单DES 的各种条件
// 	||(((tmp[4]&0x0C) == 0x0C)&&( (((tmp[1]&0x03) != 0x00)&&((tmp[1]&0x03) != 0x01)) ||( ((tmp[2]&0x0C) != 0x00)&&((tmp[2]&0x0C) != 0x04) ))) )      
	if(IF_CheckTarSupportSingleDES(&tmp[1]) == FALSE) //不支持单DES
	{
		if(processResult == POR_OK)
		{
			processResult = RFMSecurityError;	                    //记录错误，程序继续往下执行
		}
	}
	
	if(((tmp[3]&0x02) == 0x02)&&((tmp[1] & 0x04) == 0x04)) //添加 加密时候才判断是不是AES 
	{
		blocksize = 0x10;
	}

	if(processResult == POR_OK)	   //CATTP 操作暂时仅支持RAM /RFM
	{
#if UICC__CDMA__CATTP__PRL
		if((G_PRL & 0x03)!= 0x00)
		{
			if(BIPPRLSPICheck(tmp[1],tmp[2]) == FALSE)
			{
				if(processResult == POR_OK)
				{
					processResult = RFMSecurityError;	                    //记录错误，程序继续往下执行
				}
			}
		}
		else
#endif
		{
			if(G_TARRecIndex[0] == TarType_RAM)  //表示RAM
			{
				processResult = CheckRAMMinimumSecurityLevel(tmp[1]);	
			}
			else if(G_TARRecIndex[0] == TarType_RFM)
			{
				processResult = CheckRFMMinimumSecurityLevel(&tmp[1]);
			}
#if M2M_EUICC
			else if((G_TARRecIndex[0] == TarType_ISDR)||(G_TARRecIndex[0] == TarType_ISDP))  //zhhj 表示TarType_ISDR,  
			{
				processResult=POR_OK;  //这里要？？？？？？
			}
#endif
			else
			{
				processResult = TARUnknown;
			}
		}
	}
	
	if(CheckSPIRuleForPor(&tmp[1]) != POR_OK)//同步TPX 电信5G代码
	{
		if(processResult == POR_OK)
		{
			processResult = RFMSecurityError;	                    //记录错误，程序继续往下执行
		}		
	}
				
	if(((tmp[1]&0x03) == 0x02)&&((tmp[4] & 0x03) == 0x02))
	{
		ccLgth = GetAESMACLen(tmp[4],TYPE_KEY_KID); 
		if((ccLgth == FALSE) || (ccLgth != tmp[0] -13))
		{
			ccLgth = 0xFF;
		}
	}
	else
	{
		ccLgth = GetCcLength(tmp[1],tmp[4]);
	}
	
	if(ccLgth == 0xFF)
	{
		if(processResult == POR_OK)
		{
			//不支持的MAC 类型，返回RFMSecurityError
			processResult = RFMSecurityError;	                    //程序继续往下执行
		}

		ccLgth = 0x00;		
	}
	
	if(tmp[0] != (ccLgth + 13)) // SPI KIC KID CNTR PCNTR CC/RC  //移动送检工程集 RFMRAM测试2G3G   CMCC_RFM_test/RFM_TEST(3G)/5.1 IMPLEMENTATION FOR SMS-PP/5.1.5 Security Mechanism for the Command Packet/SecurityMechanismForTheCommandPacket1_00   step12 chl长度与实际不符 要求报错
	{
		if(processResult == POR_OK)
		{
			processResult = RFMSecurityError;	                    //程序继续往下执行
		}
	}

	ecpMem_Copy(&spikickid[0x00],&tmp[1], 0x07);  //拷贝SPI 2字节+KIC+KID+TAR = 7字节

	if((spikickid[0]&0x04) == 0)  //不需要加密
	{
		G_RFMRAMKeyversion = spikickid[3]>>4;	//使用kid的版本
	}			
	else if(((spikickid[0]&0x03)==0x00)||((spikickid[0]&0x03)==0x01))	 //不需要算mac or RC(不需要密钥)
	{
		G_RFMRAMKeyversion = spikickid[2]>>4;//使用kic的版本	
	}
	else  //需要解密和算mac，要求kic，kid版本号一致，见ts123.048附录B，lj 20131127
	{
		if((spikickid[2]&0xF0)!= (spikickid[3]&0xF0))
		{
			G_RFMRAMKeyversion = spikickid[2]>>4;	

			if(processResult == POR_OK)
			{
				processResult = RFMSecurityError;
			}
		}
		else
		{
			G_RFMRAMKeyversion = spikickid[2]>>4;
		}
	}
	
#if UICC__CDMA__CATTP__PRL
	
	if((G_PRL & 0x03) != 0x00) //此时PRL更新 G_TARRecIndex 没有赋值，在此先默认赋值 好解密算MAC时候可以使用G_TARRecIndex  算完MAC后还原
	{
		G_TARRecIndex[0] = TarType_RFM;
		G_TARRecIndex[2] = ISD_APPID;
	}
	
#endif
	
	offset = 10 +firstcpl[0]; //待解密数据的起始偏移，从CNTR开始 CNTR PCNTR CC DATA	
	
	if((tmp[1]&0x04) == 0x04)  //Ciphering
	{ 	 
		 decryLen = cplLen - 8;
		
		 if(decryLen%blocksize)
		 {
			if(processResult == POR_OK)
			{
				processResult = RFMSecurityLevel;
			}
		}
			
		ToolkitDeChipherEe(tmp[3],G_EnvelopHanderAddr,offset,MODE_DECRYPT,decryLen);
	} 
	else
	{
// 		 offset = 10 +firstcpl[0]; //待解密数据的起始偏移，从CNTR开始 CNTR PCNTR CC DATA
// 		 
// 		 ReadTFBuf(G_EnvelopHanderAddr,offset,&tempBuf[0], 0X10);			

// 		 ecpMem_Copy(&spikickid[7],&tempBuf[0],5);  //备份counter
	}
	
	ReadTFBuf(G_EnvelopHanderAddr,offset,&spikickid[0], sizeof(spikickid)); //备份counter  mac	中间多了一个pcntr	 

	pcntr = spikickid[5];
	
	if(((tmp[1]&0x04) == 0x04)&&(pcntr > (blocksize-1)))	//padding 数据可以不为0 ，规范没有明确说明 ，相关测试用例：移动RFM&RAM测试脚本_2G3G结合/CMCC_RFM_test/RFM_TEST(2G)/5.15_SecurityMechanismCommandPacket003
	{
		if(processResult == POR_OK)
		{
			processResult = CipheringError;
		}
	}
	
	//判断mac	

	if((tmp[1]&0x03) == 0x02)          //spi0:tmp[1] spi0:tmp[2] kic:tmp[3] kid:tmp[4] 10:Cryptographic Checksum 
	{
			ecpMem_Set(mac,0x00,sizeof(mac));
			result = ToolkitCalculateMac(G_EnvelopHanderAddr+sizeof(TranFileHead),tmp[1],tmp[3],tmp[4],firstcpl[0]+2,ccLgth,cplLen+firstcpl[0]+2,mac);
		
			if((result==FALSE)||(ecpMem_Compare(&spikickid[6],mac,ccLgth)!=EQUAL))
			{					
				if(processResult == POR_OK)	
				{					
					processResult =CCFailed;
				}					
			}
	}
	
#if UICC__CDMA__CATTP__PRL
	if((G_PRL & 0x03) != 0x00) //此时PRL更新  算完MAC后还原
	{		
		ecpMem_Set(G_TARRecIndex,0x00,sizeof(G_TARRecIndex));
	}
	
#endif
	
	if(processResult != POR_OK)
	{
		return processResult;
	}
	result = FormattedRFMSMSCheckCounter(tmp[0x01],&spikickid[0]);   //对于toolkit app的安全报文头，同样检查counter ，因为没有规范说明对于toolkit app的报文头不检查counter的

	if(result != POR_OK)
	{
		return result;						
	}
    
#if UICC__CDMA__CATTP__PRL
    if((G_PRL & 0x03) != 0x00)	
	{
				//判断PRL DowloadData数据的tag，len是否有效
		  ReadTFBuf(G_EnvelopHanderAddr,16+firstcpl[0]+ccLgth, &spikickid[0], 0x04);	 //读取Packet Number Tag、Packet Number Len、Total Number、Current Number
		
		  if((spikickid[0] != 0x80) ||(spikickid[1] != 0x02) ||(spikickid[2] <spikickid[3]))  //PRL Dowload tag 、len error or totalNumber <currentNumber
		  {
			  G_PRL |= PRLFormatError;
			  
			  return processResult;
		  }

		  if(spikickid[3] == 0x01)  //重新收到一组新的数据
		  {
			G_CATTPPreNumber = FALSE;

			G_CATTPCurrentTotalNumber = FALSE;

			CATTPClearFile(EF_CATTPSave);
		  }

		  if(G_CATTPCurrentTotalNumber == FALSE)  //当前收到的是地一包有效的PRL数据
		  {
			ecpMem_Copy(&G_CATTPTar[0],&G_ToolkitTar[0], 0x03);

			G_CATTPCurrentTotalNumber = spikickid[2];

		  }
		  else if(G_CATTPCurrentTotalNumber != spikickid[2]) //当前收到的是另外一包的数据
		  {	
			  G_PRL |= PRLFormatError;
			  
			  return processResult;
		  }


		 if(ecpMem_Compare(&G_ToolkitTar[0],&G_CATTPTar[0], 0x03) != EQUAL) //用例名称:  Test_PRL_EPRL_OPLMN_4_3 -- TestPurpose: PRL_EPRL_OPLMN混合更新测试，CAT-TP链接PRL更新中卡片收到其它类型的非第一条数据应不处理
		 {
			G_PRL |= PRLFormatError;
			 
			return processResult;
		 }

		 //将有效的PRL数据缓存起来，待数据全部收全后再进行PRL更新
		
		 result = CATTPDataIsReceived(spikickid[3]);
		
		 if(result != FALSE)  //当前index数据已收到，暂时处理为抛弃
		 {
			G_PRL |= PRLFormatError;
			 
			return processResult;
		 }
		
		 if(spikickid[3] != (G_CATTPPreNumber +1))	  //PRL短信不是按顺序来的，则报错，	  如果不要该判断，则支持PRL乱序数据过来
		 {
			G_PRL |= PRLFormatError;
			 
			return processResult;
		 }

		 G_CATTPPreNumber = spikickid[3];
		
		 //G_CurrentCommandOffset = tmp[0] + 3+ firstcpl[0] +4;  //指向第一条APDU指令偏移	 4=Download Data Packet Number Tag + 包长度 + 总包数+此条包序号
		 
		 result = CATTPSavePRLData(G_EnvelopHanderAddr, tmp[0] + 3+ firstcpl[0] +4,cplLen-(tmp[0] + 1+pcntr +4),spikickid[3]);	//0E=CHL +SPI + KIC +KID +TAR + CONTR + PCNTR
		
		 if(result == FALSE)
		 {
			G_PRL |= PRLFormatError;
		 }

		 //CATTPClearFile(EF_PDUFILE); // 退出到函数外使用
		

		
		if((CATTPDataIsReceivedAll())&&((processResult == POR_OK))) //FALSE表示当前短信未收全，TRUE 表示所有短信已收全
		{
			G_PRL |= PRLRecivedALL;            //bit 4代表收全
		}
		
		return processResult;	
	}
	else
#endif
	{
		//CATTPClearFile(EF_PDUFILE);  //退出到函数外使用

		return processResult;
	}

}
/*
*@description:通过CATTPS上发RAM的 por
*
*@function   :CATTPResponseRAMPor
*
*@param      :BYTE errorTag
*@param      :BYTE mode:extended 0x01/00:普通模式
*
*@return
*/
void CATTPResponseRAMPor(BYTE errorTag,BYTE mode)
{
	UWORD xdata fileAddr, fileAddr2;
	BYTE xdata spikickid[0x0c],cc[0x08];
	WORD xdata rpiOffset,backRpiOffset,fileSize,resultLength,totalLen,PDULen;
	BYTE idata cclength,pcntr,CATTPHeaderLen,blockSize,addtionlen,backLen,backMacOffset,PDUNUM;
	
 	fileAddr2 = SelectPrivateTFFile(DF_Toolkit, EF_PorDataFile);	// POR文件
	
	if (fileAddr2 == FALSE)
	{
		return;
	}

	fileSize = GetTFSize(fileAddr2);

	ReadTFBuf(G_EnvelopHanderAddr, 0x01, spikickid, 0x03);	//CPL
	
	rpiOffset =  ToolkitGetCPLLength(spikickid[0]);  //CPL长度
	
	rpiOffset += 0x02;   //CPI CHL 2字节
	
	ReadTFBuf(G_EnvelopHanderAddr, rpiOffset, spikickid, 0x0c);	//读取SPI kic KID TAR Counter Pcntr CC..
	
	if ((spikickid[1] & 0x03) != 0x01)			//PoR存在检查
	{
		if (((spikickid[1]  & 0x03) != 0x02) || (errorTag == POR_OK))
		{
			GPRAMRFMExtendResponsePublic(TAG_ClearResponseFile);

			CATTPSendRequestInOpenState(NULL);
			
			return;
		}
	}
	
#if UICC__OPERATOR__CHINATELECOM
	if((errorTag != POR_OK)&&(errorTag != CNTRLow)&&(errorTag != CNTRHigh)&&(errorTag != CNTRBlocked))
#else
	if(errorTag != POR_OK)
#endif
	{
		cclength = 0x00;
	}
	else
	{
		if(((spikickid[1] & 0x0C) == 0x08) && ((spikickid[3] & 0x03) == 0x02) )
		{
			//cclength = G_CcLengthForAES;
			cclength = GetAESMACLen(spikickid[3],TYPE_KEY_KID);
		}
		else
		{
			cclength = GetCcLength((spikickid[1]>>2)&0x03,0);	

			if(cclength == 0xFF)
			{
				return; 
			}
		}
	}	

	pcntr = 0x00;	
	
	blockSize = 0x08;
	
	addtionlen = 0x00;            //扩展模式增加的长度  默认是压缩格式
	
	resultLength = 0x00;
	
	CATTPHeaderLen = 0x012;       //CATTP头长度 偏移位置
	
#if UICC__CDMA__CATTP__PRL
	if((G_PRL & 0x03) != 0x00) //（PRL更新中为1字节）
	{
		resultLength = 0x01;
		
	}
	else
#endif
	{
		if((errorTag==POR_OK)||(errorTag == CNTRLow)||(errorTag == CNTRHigh)||(errorTag == CNTRBlocked))
		{
		if((G_FormatAndType == FormatType_ExtendAA) || (G_FormatAndType == FormatType_ExtendAE))
		{
			fileAddr = SelectPrivateTFFile(DF_Toolkit, EF_RAMExtendResponse); // 执行响应文件
			
			if(fileAddr == FALSE)
			{
				return ;
			}
			
			resultLength = ReadTFFileWord(fileAddr);//ReadTFBuf(fileAddr, ramExtendResLen_offset, (BYTE *)&resultLength, 0x02);	//读取长度
			
			addtionlen = ExtendResTagADD(resultLength);
		}
		else
		{
			
			resultLength = G_CurrentRemoteResponseLgth +3;
		}
		
		if((spikickid[1]&0x10) == 0x10)	 //PoR response shall be ciphered	 
		{
			if((spikickid[2] & 0x02) == 0x02)
			{
				blockSize = 0x10;
			}
			
			pcntr = blockSize-((cclength+7+addtionlen+resultLength)%blockSize);	 //cclength+additionalLen+7 = CNTR(5bytes) PCNTR(1bytes)  StatusCode(1bytes) +cc+addtional
			
			if(pcntr == blockSize)
			{
				pcntr = 0;
			}
		}
		
		if((resultLength + 0x0B+0x12+blockSize+8)>fileSize)  //0x0B
		{
			resultLength = fileSize- (0x0B+0x12+blockSize+8);
			GlobalSetStatus(0x62F1);
			}
		}
    }

	totalLen = 	resultLength +addtionlen+ 0x0B+cclength+pcntr;	  ////不包括RPI 字段 ，RHL + TAR+COUNTER +PCNTR+ERRORTAG = 0X0B	 ，addtionlen 里面的数据是resData的一部分

	rpiOffset = 0x00;
	
    ////待发送的所有数据 modify by zhhj   RPI: 02 RPL: 10 RHL: 0A TAR: B00020 CNTR: 0000000002 PCNTR:00  RSC: 00 执行的指令个数: 02 最后一个指令的SW: 9000
	tempBuf[rpiOffset ++] = 0x02 ;  //RPI
	
	backLen = SetBerTlvLength(&tempBuf[rpiOffset],totalLen);
	
	rpiOffset += backLen;
	
	tempBuf[rpiOffset ++] = 0x0A + cclength;  //RHL  :0x0A = TAR + COUNTER + PCNTR + Response Status Code Octet
	
	ecpMem_Copy(&tempBuf[rpiOffset],&spikickid[4],0x08);  //copy tar CNTR
	
	//if(errorTag != POR_OK)      //根据最新102225POR时候状态字不为OK则counter清0或者不发送POR，目前清counter
	if((errorTag != POR_OK)
				&&(ValueBit(JcvmConfig_GetConfigUicc(), BIT_UICCConfig_Por_Counter_Clear) == POR_COUNTER_CLEAR))      //根据最新102225POR时候状态字不为OK则counter清0或者不发送POR，目前清counter
	{
		switch(errorTag)
		{
			case CNTRLow:
			case CNTRHigh:
			case CNTRBlocked:			// HB TC1_A6_A8_TestProfile1 counter 错期望counter和下行一致
		    //case CCFailed:  //CCFailed按照标准，也应该把counter清掉，这样更加安全,simapi  mac错误也是期望counter 为0x01  fwk_fws_ouda,该错误忽略
				break;
			default:
				ecpMem_Set(&tempBuf[rpiOffset + 0x03],0x00,5);
			break;
		}
		//ecpMem_Set(&spi[7],0x00,5); //按照ts_102225v120100p.pdf ,4.1 Protocol for generalised secured packets,either send a Response Packet indicating the error cause without any security being applied to the Response Packet and the Counter (CNTR) field set to zero;
	}
	
	backRpiOffset = (rpiOffset + 0x03);      //备份counter的位置  加密时候用

	rpiOffset += 0x08;

	tempBuf[rpiOffset ++] = pcntr;  //PCNTR

	tempBuf[rpiOffset ++] = errorTag;   //RSC

	backMacOffset = CATTPHeaderLen + rpiOffset;         //备份mac位置
	
	totalLen ++;  //加上1字节RPI的长度
	
	totalLen += backLen;

	WriteTFBuf(fileAddr2,0x00+CATTPHeaderLen,&tempBuf[0],rpiOffset);      //将第一部分数据写入文件
	
	GPRAMRFMExtendResponsePublic(TAG_ClearResponseFile);
	
	if(((cclength != 0x00) || ((spikickid[1]&0x10)==0x10)) && (G_NewSCPKey != FALSE))
	{
		GetSCP80KeyValueBeforeUpdate(NULL,MODE_KIC_KID);
	}
#if UICC__CDMA__CATTP__PRL
	if((G_PRL & 0x03) != 0x00) //PRL时候1字节
	{
	
		if((mode == POR_OK)&&(errorTag == POR_OK)&&((G_PRL & PRLFormatError)!= PRLFormatError)) //如果PRL更新成功，返回‘01’，如果PRL更新失败，返回‘00’。服务器端需根据返回的响应数据告诉操作人员更新完成情况。
		{
			tempBuf[0] = 0x01;   
		}
		else
		{
			tempBuf[0] = 0x00;   	
		}
		
		WriteTFBuf(fileAddr2,rpiOffset+CATTPHeaderLen,&tempBuf[0],0x01);

		rpiOffset++;	
	}
	else
#endif
	{
	if(errorTag == POR_OK)
	{
	
		if((G_FormatAndType == FormatType_ExtendAA) || (G_FormatAndType == FormatType_ExtendAE))
		{
			rpiOffset = ExtendResValueADD(fileAddr2,rpiOffset+CATTPHeaderLen,resultLength);
			
			rpiOffset -= CATTPHeaderLen;   //传进去加了CATTPHeaderLen头长度
		}
		else
		{	
			if(resultLength >=0x03)
			{
				tempBuf[0] = G_CurrentComNUM ;   	//CommandNumber								
				tempBuf[1] = SW1;					
				tempBuf[2] = SW2;
				
				WriteTFBuf(fileAddr2,rpiOffset+CATTPHeaderLen,&tempBuf[0],3); 
				rpiOffset +=3;
				
				if(((resultLength-3)>0xFF)&&(G_CurrentRFMEfAddr!=FALSE))
				{
						//SecureMoveEeprom(fileAddr2+sizeof(TranFileHead)+rpiOffset+CATTPHeaderLen,G_CurrentRFMEfAddr,G_CurrentRemoteResponseLgth);
						VMCopyDataToAddr(fileAddr2+sizeof(TranFileHead)+rpiOffset+CATTPHeaderLen,GetTFSize(fileAddr2)-rpiOffset-CATTPHeaderLen,G_CurrentRFMEfAddr,G_CurrentRemoteResponseLgth,MODE_PROTECT_WRITE);
						G_CurrentRFMEfAddr = FALSE;
				}
				else
				{
					WriteTFBuf(fileAddr2,rpiOffset+CATTPHeaderLen,RecData,resultLength-3);
				}

				rpiOffset +=(resultLength-3);
			}
		}
	}	
    }
				
	SetTFConst(fileAddr2,rpiOffset+CATTPHeaderLen,0x00,pcntr+8);                                                      //加密和计算  CC 的 padding 字节都在这里设置，可以减少一次写文件

	
	if((spikickid[2] & 0x02) == 0x02)
	{
		tempBuf[backRpiOffset] = 0x80;
	}	
				
	if(cclength != 0x00)
	{
		ToolkitCalculateMac(fileAddr2+ sizeof(TranFileHead)+CATTPHeaderLen,(spikickid[1]>>2),spikickid[2],spikickid[3],0x00,0x00,totalLen-cclength,cc);
		
		//SecureMoveEeprom(fileAddr2+sizeof(TranFileHead)+backMacOffset+cclength,(fileAddr2+ sizeof(TranFileHead)+backMacOffset),totalLen-cclength-(backMacOffset-CATTPHeaderLen));
		
		VMCopyDataToAddr(fileAddr2+sizeof(TranFileHead)+backMacOffset+cclength,GetTFSize(fileAddr2)-(backMacOffset+cclength),(fileAddr2+ sizeof(TranFileHead)+backMacOffset),totalLen-cclength-(backMacOffset-CATTPHeaderLen),MODE_PROTECT_WRITE);
		
		WriteEepromNonAtomic(fileAddr2+sizeof(TranFileHead)+backMacOffset,(BYTE *)(cc),cclength);
	}
	
	if(((spikickid[1]&0x10)==0x10)&&((errorTag==POR_OK)||(errorTag == CNTRLow)||(errorTag == CNTRHigh)||(errorTag == CNTRBlocked)))
	{
		if((spikickid[2] & 0x02) == 0x02)     // AES
		{
			AESDeChipher(fileAddr2,backRpiOffset+CATTPHeaderLen, 0x00, totalLen-backRpiOffset,spikickid[2]);
		}
		else
		{		
			ToolkitDeChipherEe(spikickid[2],fileAddr2,backRpiOffset+CATTPHeaderLen,MODE_ENCRYPT,totalLen-backRpiOffset);
		}
	}

	tempBuf[1] = CATTP_HEADER_ACK_BIT|CATTP_VERSION|CATTP_HEADER_SEG_BIT;	
// 	tempBuf[2] = 0x00;  //RFU	
// 	tempBuf[3] = 0x00; //rfu	
	CATTPHeaderPDU(&tempBuf[2]);
	tempBuf[4] = 0x12; //header length	
// 	tempBuf[5] = (CATTPRecord.srcPort >>8)&0xFF;	
// 	tempBuf[6] = CATTPRecord.srcPort&0xFF;	
// 	tempBuf[7] = (CATTPRecord.desPort>>8)&0xFF;	
// 	tempBuf[8] = CATTPRecord.desPort&0xFF;	
	tempBuf[9] = (totalLen >> 8)&0xFF;	
	tempBuf[10] = totalLen&0xFF;	
	tempBuf[11] = (CATTPRecord.SND_NXT_SEQ_NB >>8)&0xFF;	
	tempBuf[12] = CATTPRecord.SND_NXT_SEQ_NB&0xFF;	
	tempBuf[13] = (CATTPRecord.RCV_CUR_SEQ_NB >>8)&0xFF;	
	tempBuf[14] = CATTPRecord.RCV_CUR_SEQ_NB&0xFF;	
	tempBuf[15] = (CATTPRecord.RCV_WIN_SIZE>>8)&0xFF;
	tempBuf[16] = CATTPRecord.RCV_WIN_SIZE&0xFF;	
// 	tempBuf[17] = 0x00; //check sum1	
// 	tempBuf[18] = 0x00; //check sum2

	rpiOffset = totalLen;
	
	PDULen = CATTPRecord.SYN_PDU_SIZE_MAX-CATTPHeaderLen;   //除去头长度
	
	//PDUNUM = totalLen/PDULen +1;
	
	PDUNUM = GetBlockNumber(totalLen,PDULen);
	
	CATTPRecord.SND_WIN_SIZE =  (((WORD)PDUNUM) <<8)&0xFF00 | PDUNUM;
	
	G_CurrentSDUForSendLength = totalLen;
	
	WriteTFBuf(fileAddr2,0x00,&tempBuf[1],CATTPHeaderLen);

	IF_SendPDU();
	 
	if(G_NewSCPKey != FALSE) //add chentao 更新SCP80key后，发响应值需要用更新前的key，发完响应后用新的key
	{		
		G_NewSCPKey = FALSE;
	}
	
	
} 
	
	
/*
*@description :一条完整的SDU数据接收完成后，对SDU数据进行处理，用于EUICC
*
*@param
*
*@return
*
*@history
*/
BYTE CATTPProcessSDUDataEUICC(void)
{
	BYTE idata result,PRLresult; //PRLresult这个变量用来看执行更新PRL是否成功,result表示加解密格式等判断是否成功，两个都成功了 才是更新PRL成功，然后发display等
	WORD offset;
	
	result = CATTPSDUDataDecryAndCheck();		
	
	G_LASTSDU_Flag = FALSE;	    //一个SDU处理成功

	
	CATTPClearFile(EF_PDUFILE); //清空BIP接收缓存
	
#if UICC__CDMA__CATTP__PRL
	PRLresult = POR_OK; //默认PRL更新失败   TestPRL_1_13用例 更新4001字节数据，到CATTPExcutePRLCommand时候报错，期望执行失败 同时RSC状态字是成功,即CATTPSDUDataDecryAndCheck解密没问题
#endif
	
	if(result == POR_OK)
	{
#if UICC__CDMA__CATTP__PRL
		if((G_PRL & 0x03) != 0x00)
		{
			if((G_PRL & PRLRecivedALL)== PRLRecivedALL)
			{
				PRLresult = CATTPExcutePRLCommand();
		
				G_CATTPCurrentTotalNumber = FALSE;  //清空总包数
				
				CATTPClearFile(EF_CATTPSave);
			}
		}
		else
#endif
		{
			G_Source_Type = SourceType_CATTP;
			
			offset = 0x01;            //CPI 
			
			RemoteAPDUProcessFunction(G_EnvelopHanderAddr,offset,0x00,RemoteDataEndBlock,NULL);
		}
	}
	else
	{
	
		//CATTPResponseRAMPor(result,0x01);  //此时mode字节无意义	，RFM 和RAM 的报文类型一致，所以采用同一个函数进行POR 的发送
		
	}
	
	CATTPResponseRAMPor(result,PRLresult);  //mode用来表示PRL执行情况
	
	CATTPStartTimerOnly(); //BIP EPRL下载 脚本:CATTPStartTimer改为CATTPStartTimerOnly,14回来之后 有PRL更新成功则display,然后cephMoreTimeSingle
	
#if UICC__CDMA__CATTP__PRL
	//增加配置，PRL下载完成后不弹窗(澳门电信需求),by xwh 20220414
	if(ValueBit(JcvmConfig_GetConfigUicc(), BIT_UICCConfig_Prl_Not_DisplayInfo)==0)
	{
		if((G_PRL & PRLRecivedALL) == PRLRecivedALL)
		{
			if(PRLresult == POR_OK)
			{
				if((G_PRL & 0x03) == BIP_PRL)
				{
					cephDisplayTextSingle(0x01,UCS2,(BYTE *)PRLSuccess);  //prl下载已完成
				}
				else if((G_PRL & 0x03) == BIP_EPRL)
				{
					cephDisplayTextSingle(0x01,UCS2,(BYTE *)EPRLSuccess);  //eprl下载已完成
				}
				else//BIP_OPLMN  OPLMN更新成功后，不需要有更新成功的提示语。
				{
				}
			}
		}
	}
#endif
	IF_TriggerRemoteFileUpdate();       //远程文件更新事件
	
	ecpMem_Set(G_TARRecIndex, 0x00, sizeof(G_TARRecIndex));  //还原初值
		
	G_RFMRAMKeyversion = 0xFF;//还原初值
	
	G_FormatAndType = FormatType_NULL;;						//还原初值

	//CATTPStartTimer();
	
	if(G_BIPDataAvaiableEvent != TRUE)////BIP EPRL下载 脚本:CATTPStartTimer改为CATTPStartTimerOnly,14回来之后 有PRL更新成功则display,然后cephMoreTimeSingle
	{
		result = cephMoreTimeSingle();
	}
	
//	G_CATTPCommand = FALSE;
	
	return result;
}

	
/*
*@description :一条完整的SDU数据接收完成后，对SDU数据进行处理
*
*@param
*
*@return
*
*@history
*/
BYTE CATTPProcessSDUData(void)
{
    
	if(CATTPSequenceSDUData() == FALSE)
	{			
		return FALSE;
	}
    
	return CATTPProcessSDUDataEUICC();
	
	}
 
/*@Function:   CATTPSendMoreData                                            
 * 
 *@Descrip:    CATTP 发送多条数据 
 *
 *@See:	       
 *                                                                                                                                     
 *@Param:      tagAddr      
 *              						                                                                                                  
 *@Return:     BYTE                                                                                                                                                                                                                                                              
 * 
 *@Author:     xwh
 *  
 *@History:    V0.0---Create function
 *             V0.1---Modify by xwh 20220707, 修改原函数发送一条数据后break退出整个循环，无法发送下一条数据的BUG， 用局部变量retryTimes控制重发3次                     
	           V0.2---Modify by xwh 20220816, 单条发送备份数据，设置G_RetryTimers = 0x03，后续timer时间到重发;多条直接重发,设置G_RetryTimers = 0x00；
 */  	
void CATTPSendMoreData(UWORD PorAddr,WORD totalLen, BYTE channelId)
{
	WORD idata offset;
	BYTE idata dataLgth,result,retryTimes;

	offset = 0;

	result = CMD_Success;
	
	//单条发送
	if(totalLen <= 0xF0)
	{
		tempBuf[0] = totalLen;

		//ecpMem_Move(&tempBuf[1], &tempBuf[offset], dataLgth);
		ReadTFBuf(PorAddr, offset, (BYTE *)&tempBuf[1], totalLen);
		
		if((CATTPRecord.status == CATTP_SYN_SENT)||((CATTPRecord.status == CATTP_OPEN)&&(tempBuf[0] != 0)))	    //cattp链接未建立或者CATTP链接建立后，但是发给服务器的POR服务器端未处理
		{
			WriteVariable(VaribleTag_BackPDU,&tempBuf[0]);
		
			G_RetryTimers = 0x03;
		}
		
		result = cephSendDataSingle(0x01,channelId,&tempBuf[0]);
	}
	else //多条发送
	{
		while(totalLen > 0)
		{
			retryTimes = 0x03;
			
			if(totalLen > 0xF0)
			{
				dataLgth = 0xF0;	
			}	
			else
			{
				dataLgth = totalLen;
			}
			tempBuf[0] = dataLgth;

			//ecpMem_Move(&tempBuf[1], &tempBuf[offset], dataLgth);
			ReadTFBuf(PorAddr, offset, (BYTE *)&tempBuf[1], dataLgth);
			
			while(retryTimes--)
			{
					//多条发送不需要备份数据
//				if((CATTPRecord.status == CATTP_SYN_SENT)||(CATTPRecord.status == CATTP_OPEN)&&(tempBuf[0] != 0))	    //cattp链接未建立或者CATTP链接建立后，但是发给服务器的POR服务器端未处理
//				{
//					WriteVariable(VaribleTag_BackPDU,&tempBuf[0]);
//				
//					G_RetryTimers = 0x03;
//				}
			
				if((totalLen - dataLgth) == 0x00)
				{
					result = cephSendDataSingle(0x01,channelId,&tempBuf[0]);
				}
				else
				{
					result = cephSendDataSingle(0x00,channelId,&tempBuf[0]);
				}

				if(result == CMD_Success)
				{
					break;
				}
			}
			offset += dataLgth;

			totalLen -= dataLgth;
		}

		//多条直接重发,设置G_RetryTimers = 0x00
		G_RetryTimers = 0x00;
	}
}
#endif
