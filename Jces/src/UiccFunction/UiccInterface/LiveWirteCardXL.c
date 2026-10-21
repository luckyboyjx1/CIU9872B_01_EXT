/*
 *2013-4-24：LiveGetCardInfoHandler中删除对ICCID文件为空的判断
 *2013-5-6：根据《中国移动现场写卡技术规范v2.0.0-20130426.pdf》，新增多号写卡时，无论主号或副号写卡错误都回滚的处理
 *2013-5-16：修改mac会话密码计算方法
 *2013-5-17：修改一卡双号程序
 *2013-5-22：修改解密数据部分，增加对PCNTR的判断
 *2013-5-24：修改mac计算部分
 *2013-6-4：修改mac计算部分——包括加密时的填充数据，再补位
 *2013-6-25：规范升级：1、增加结果代码0x33—出现不支持的Tag值
 *										2、修改结果代码0x32—加密填充数据的判断
 *2013-7-2：1、修改UpdateOPC
 *					2、修改一卡多号
 *2013-7-16：根据《现场写卡功能测试规范V0.9.0.doc》增加下行异常判断条件：
 *					1、出现不必要tag或重复的tag，返Result_WrongTag
 *2014-01-28: 根据移动要求将现场写卡和两不一快的代码整合到一张卡中－－王秋艳
 *
 *2014-03-02：根据研发徐云珍审核代码的情况修改地方如下：
 *            1).UpdateKey(void)--对于PIN,PUK的备份的函数有问题进行修改。
 *            2).RecoverData(u8 * originNumber)－－－数据恢复的时候原来的函数会将PIN2给恢复为FF，导致数据混乱。－－－验证方法： 测试的时候可                 *                                                   以不创建6F07，执行完写卡数据后读2E10 文件即可以看到现象
 *            3).LiveGetCardInfoHandler(u8 ptr)－－－去掉了对于2F02文件第一个字节为0的判断，因为在规范中这个字节的范围是00-99.
 *            4).LiveWriteCardHandler(void)－－－在寻找批次的时候添加对TAR的判段，避免找到的第一条数据是OTA/RFM的数据进行了解密,同时将保存批次                 *                                            索引短信总数的BUF由原来的  tempBuf修改为packageInfo，避免数据被冲掉;G_LIVEWRITECARDTYPE全局变量使用之前应该先赋 *                                            值
 *            5).LiveWriteCardWriter( u8 *packageInfo)--用之前已经找到的批次而不是重新查找批次
 *            6).LiveWriteCardSendSMS(u8 result,u8 *packageInfo)--用之前已经找到的批次而不是重新查找批次
 *
 *
 *2014-04-14: 两不一快上行的TPDA文件为6F03文件
 *2014-05-30：增加上发的重发机制
 *2014-7-1：规范升级至V1.1.4，修改上行重发机制的细节
 *2014-9-5：LiveWriteCardHandler中加入应用类型判断，使现场写卡卡不能处理两不一快下行，反之亦然
 *2015-2-6：根据两不一快新增要求，增加函数ClearLociSqn()，数据更新成功的同时将Loci   psloci  epsloci 文件和sqn恢复到卡片出厂设置值。
 */

#include <string.h>
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if UICC__APP__LIVECARD
u8 X_A G_NowNumber;
//u8 X_A G_setNum;
//u8 X_A G_WriteCardStatus;
u8 X_A G_SendControl ;
u8    X_A     G_LIVEWRITECARDTYPE;//保存当前下发数据的命令类型，是两不一快0x0c,还是现场写卡0X0B

u8 X_A G_macPadding;

/*
 *@description :更新ICCID,只校验长度
*/
u8 UpdateICCID(void)
{
	u8 I_A result;
	memref I_A fileAddr;
	
#ifndef MORENUMBER
	fileAddr = SelectTFFile(DF_MF, EF_ICCID);
#else
	if(G_NowNumber == 0x01)
	{
		fileAddr = SelectTFFile(DF_MF, EF_ICCID);
	}
	else
	{
		fileAddr = SelectLFFile(DF_NUMBACKUP, EF_ICCID_GSM_C);
	}
#endif

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
#ifndef MORENUMBER	
	result = WriteTFBuf(fileAddr,0x00,&RecData[2],RecData[1]);
#else
	if(G_NowNumber == 0x01)
	{
		result = WriteTFBuf(fileAddr,0x00,&RecData[2],RecData[1]);
		
		if(result == FALSE)
		{
			return FALSE;
		}
		
		fileAddr = SelectLFFile(DF_NUMBACKUP, EF_ICCID_GSM_C);
		
		result = WriteLFBuf(fileAddr,G_NowNumber,0x00,&RecData[2],RecData[1]);
	}	
	else
	{
		result = WriteLFBuf(fileAddr,G_NowNumber,0x00,&RecData[2],RecData[1]);
	}
#endif

	if(result == FALSE)
	{
		return FALSE;
	}
	
	return TRUE;
}


u8 UpdateACC(u8 lastByte)
{
	memref X_A fileAddr;
	u16 X_A ACCValue;
	u8 X_A i, result, tmp[2];

    #ifndef MORENUMBER		
	fileAddr = SelectTFFile(DF_PHASE, EF_ACC);
    #else
	if(G_NowNumber == 0x01)
	{
		fileAddr = SelectTFFile(DF_PHASE, EF_ACC);
	}
	else
	{
		fileAddr = SelectLFFile(DF_NUMBACKUP, EF_ACC_C);
	}
    #endif  

  if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	result = ((lastByte>>4) & 0x0F);
	
	if(result == 0x00)
	{
	  tmp[0] = 0x00;
	           
	  tmp[1] = 0x01;
	}
	else if(result >= 0x01)
	{
	  ACCValue = 0x0001;
	
	  for(i=1; i<=result; i++)
	  {
	      ACCValue *= 2;
	  }
	
	  tmp[0] = (ACCValue>>0x08)&0x00FF;
	
	  tmp[1] = ACCValue&0xFF;
	}

    #ifndef MORENUMBER	
	result = WriteTFBuf(fileAddr, 0, &tmp[0], 2);
    #else
	if(G_NowNumber == 0x01)
	{
		result = WriteTFBuf(fileAddr, 0, &tmp[0], 2);
		
		if(result == FALSE)
		{
			return FALSE;
		}
		
		fileAddr = SelectLFFile(DF_NUMBACKUP, EF_ACC_C);
		
		result = WriteLFBuf(fileAddr, G_NowNumber, 0, &tmp[0], 2);
	}
	else
	{
		result = WriteLFBuf(fileAddr, G_NowNumber, 0, &tmp[0], 2);
	}
    #endif

	if(result == FALSE)
	{
		return FALSE;   //ACC写入时发生错误
	}

	return TRUE;
}


/*
 *@description : 更新卡片IMSI,只校验长度
*/
u8 UpdateIMSI(void)
{
	memref X_A fileAddr;
	u8 I_A result;
	
	result = UpdateACC(RecData[10]);
	
	if(result == FALSE)
	{
		return FALSE;   //ACC写入时发生错误
	}

    #ifndef MORENUMBER	
	fileAddr = SelectTFFile(DF_PHASE, EF_IMSI);
    #else
	if(G_NowNumber == 0x01)
	{
		fileAddr = SelectTFFile(DF_PHASE, EF_IMSI);
	}
	else
	{
		fileAddr = SelectLFFile(DF_NUMBACKUP, EF_IMSI_GSM_C);
	}
    #endif

	if(fileAddr == FALSE)
	{
		return FALSE;    //IMSI文件不存在
	}

    #ifndef MORENUMBER	
	result = WriteTFBuf(fileAddr,0x00,&RecData[2],RecData[1]);
    #else
	if(G_NowNumber == 0x01)
	{
		result = WriteTFBuf(fileAddr,0x00,&RecData[2],RecData[1]);
		
		if(result == FALSE)
		{
			return FALSE;
		}
		
		fileAddr = SelectLFFile(DF_NUMBACKUP, EF_IMSI_GSM_C);
		
		result = WriteLFBuf(fileAddr,G_NowNumber,0x00,&RecData[2],RecData[1]);
	}
	else
	{
		result = WriteLFBuf(fileAddr,G_NowNumber,0x00,&RecData[2],RecData[1]);
	}
    #endif

	if(result == FALSE)
	{
		return FALSE;   //IMSI写入时发生错误
	}

	return TRUE;
}


/*
 *@description :更新卡片SMSP,只校验长度
*/
u8 UpdateSMSP(void)
{
	u8 I_A result,RecLgth;
	memref X_A fileAddr;

    #ifndef MORENUMBER	
	fileAddr = SelectLFFile(DF_TELECOM, EF_SMSP);
    #else
	if(G_NowNumber == 0x01)
	{
		fileAddr = SelectLFFile(DF_TELECOM, EF_SMSP);
	}
	else
	{
		fileAddr = SelectLFFile(DF_NUMBACKUP, EF_SMSC_C);
	}
    #endif

	if(fileAddr == FALSE)
	{
		return FALSE;   //文件不存在
	}

	RecLgth = GetLFRecNumLgth(fileAddr,FileLgth);

    #ifndef MORENUMBER	
	result = WriteLFBuf(fileAddr,0x01,RecLgth-0x0F,&RecData[1],RecData[1]+1);
	
	SetLFConst(fileAddr,0x01,12,0xFD,1);
    #else
	if(G_NowNumber == 0x01)
	{
		result = WriteLFBuf(fileAddr,0x01,RecLgth-0x0F,&RecData[1],RecData[1]+1);
	
		SetLFConst(fileAddr,0x01,12,0xFD,1);
		
		if(result == FALSE)
		{
			return FALSE;
		}
		
		fileAddr = SelectLFFile(DF_NUMBACKUP, EF_SMSC_C);
		
		result = WriteLFBuf(fileAddr,G_NowNumber,RecLgth-0x0F,&RecData[1],RecData[1]+1);
	
		SetLFConst(fileAddr,G_NowNumber,12,0xFD,1);
	}
	else
	{
		result = WriteLFBuf(fileAddr,G_NowNumber,RecLgth-0x0F,&RecData[1],RecData[1]+1);
	
		SetLFConst(fileAddr,G_NowNumber,12,0xFD,1);
	}
    #endif	

	if(result == FALSE)
	{
		return FALSE;  //SMSP写入时发生错误	
	}

	return TRUE;
}


/*
 *@description:更新PIN1-09,PIN2-0A,PUK1-0B,PUK2-0C
*/
u8 UpdateKey(void)
{
	u8 I_A status,result;
	KeyStructer X_A pinKey;
	PukStructer X_A pukInfo;
	u8 I_A  tag;

    #ifdef MORENUMBER	
	if(G_NowNumber != 0x01)	//副号不用写
	{
		return TRUE;
	}	
    #endif
	
	if((RecData[0] == 0x04)||(RecData[0] == 0x06))
	{
		status = PIN1;             //PIN1 PUK1
	}
	else
	{
		status = PIN2;             //PIN2 PUK2
	}
	
	result = ReadKey(status,(u8 X_A *)&pinKey.KeyId);

	if(result == FALSE)
	{                                                  					
 		return FALSE;
	}
	
	result = ReadOrWritePukInfo(pinKey.pukRef,(u8*)&pukInfo.PukId,Read_Key);

	if(result == FALSE)
	{
		return FALSE;
	}

	tag = RecData[0] -1;     //RecData[0]-1:取值分别为3(var_PIN1) 4(var_PIN2) 5(var_PUK1) 6(var_PUK2) 

	ResData[0] = ReadVarible(tag, &ResData[1]);
	
	if((ResData[0] == 0x00) || (ResData[0] == 0xFF))
	{
		if(RecData[0] < 0x06)//备份
		{
			ecpMem_Copy((u8 X_A *)&ResData[1], (u8 X_A *)&pinKey.KeyContent[0], 0x08);
		}
		else
		{
			ecpMem_Copy((u8 X_A *)&ResData[1], (u8 X_A *)&pukInfo.PukContent[0], 0x08);
		}
		
		ResData[0] = 8;
		
		WriteVariable(tag, &ResData[0]);
	}
	
	if(RecData[0] < 0x06)
	{
		ecpMem_Copy((u8 X_A *)&pinKey.KeyContent[0],(u8 X_A *)&RecData[2],0x08);
	}
	else
	{
		ecpMem_Copy((u8 X_A *)&pukInfo.PukContent[0],(u8 X_A *)&RecData[2],0x08);
	}

	result = WriteKey(status,(u8 X_A *)&pinKey.KeyId);
	
	if(result == FALSE)
	{                                                  					
 		return FALSE;
	}
	
	result = ReadOrWritePukInfo(pinKey.pukRef,(u8*)&pukInfo.PukId,Write_Key);

	if(result == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}


/*
 *@description:更新卡片KI,只校验长度
*/
u8 UpdateKI(void)
{
	memref X_A fileAddr;
	u8 I_A result;

#ifndef MORENUMBER	
	fileAddr = SelectLFFile(0x7ff0, EF_KI);
#else
	if(G_NowNumber == 0x01)
	{
		fileAddr = SelectLFFile(DF_PHASE, EF_KI);
	}
	else
	{
		fileAddr = SelectLFFile(DF_NUMBACKUP, EF_KI_C);
	}
#endif

	if(fileAddr == FALSE)
	{
		return FALSE;  //KI文件长度不存在
	}

    #ifndef MORENUMBER	
	result = WriteLFBuf(fileAddr,1,0x00,&RecData[2],RecData[1]);
    #else
	if(G_NowNumber == 0x01)
	{
		result = WriteLFBuf(fileAddr,1,0x00,&RecData[2],RecData[1]);
		
		if(result == FALSE)
		{
			return FALSE;
		}
		
		fileAddr = SelectLFFile(DF_NUMBACKUP, EF_KI_C);
		
		result = WriteLFBuf(fileAddr,G_NowNumber,0x00,&RecData[2],RecData[1]);
	}
	else
	{
		result = WriteLFBuf(fileAddr,G_NowNumber,0x00,&RecData[2],RecData[1]);
	}
    #endif

	if(result == FALSE)
	{
		return FALSE;  //KI写入时发生错误	
	}

	return TRUE;
}

/**
*modify by xuweihua@20210625: 性能优化， EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
*/
u8 UpdateOPC(void)
{
	u8 I_A result;
	memref X_A fileAddr;

	fileAddr = SelectTFFile(0x7ff0,EF_PRIVATE_AUTHCFG);

//	if(fileAddr == FALSE)
//	{
//		return FALSE;
//	}
	
	result = WriteTFBuf(fileAddr,0x01,&RecData[2],RecData[1]);

	if(result == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}


/*
 *@description :出现没有的tag
*/
u8 tagError(void)
{
	return FALSE;
}


static CONST CHECKLIST tag_checkList[] =
{
	{0x01,0x0A},//ICCID
	{0x02,0x09},//IMSI
	{0x03,0x08},//SMSP
	{0x04,0x08},//PIN1
	{0x05,0x08},//PIN2
	{0x06,0x08},//PUK1
	{0x07,0x08},//PUK2
	/*{0x08,0x10},//Ki
	{0x09,0x78},//索引随机数
	{0x0A,0x10},//伪Ki
	{0x0B,0x10},//K(3G)
	{0x0C,0x10},//OPC*/
};

static CONST TAGLIST tag_operateList[] =
{
	{0x01,UpdateICCID},
	{0x02,UpdateIMSI},
	{0x03,UpdateSMSP},
	{0x04,UpdateKey},
	{0x05,UpdateKey},
	{0x06,UpdateKey},
	{0x07,UpdateKey},
	/*{0x08,UpdateKI},
	{0x09,UpdateRandom},
	{0x0A,UpdateFakeKi},
	{0x0B,UpdateKI},
	{0x0C,UpdateOPC},*/
	{EOL,tagError},
};

#ifdef MORENUMBER
/*
 *@description :寻找准备写入的号码：主号、副号1、副号2...，返回其在备份文件中对应的记录号
 */
u8 FindEmptyNumber(void)
{
	memref X_A fileAddr;
	u8 X_A tag, recNum, i;
	
	fileAddr = SelectLFFile(DF_NUMBACKUP, EF_ICCID_GSM_C);
	
	if(fileAddr == FALSE)
	{
		return 0xFF;
	}
	
	recNum = GetLFRecNumLgth(fileAddr, FileNum);
	
	for(i=1; i<=recNum; i++)
	{
		ReadLFBuf(fileAddr, i, 0, &tag, 1);
		
		if(tag == 0xFF)
		{
			return i;
		}
	}
	
	return 0xFF;
}
#endif


void RecoverData(u8 * originNumber)
{
	u8 X_A num;
    #ifdef MORENUMBER	
	for(; G_NowNumber>=originNumber[0]; G_NowNumber--)
    #endif		
	{
		for(num = 0 ; num< 0x07; num ++)
		{
			if((num >= 3) && (num <= 6))//PINPUK
			{
				RecData[0] =  tag_operateList[num].tag;

				if(tag_operateList[num].tag == 0x04)
				{
					RecData[1] = ReadVarible(var_PIN1, &RecData[2]);
				}
				else if(tag_operateList[num].tag == 0x05)
				{
					RecData[1] = ReadVarible(var_PIN2, &RecData[2]);
				}
				else if(tag_operateList[num].tag == 0x06)
				{
					RecData[1] = ReadVarible(var_PUK1, &RecData[2]);
				}
				else if(tag_operateList[num].tag == 0x07)
				{
					RecData[1] = ReadVarible(var_PUK2, &RecData[2]);
				}

				if(RecData[1] == 0x00)
				{
					continue;
				}
			}
			else
			{
				RecData[1] = tag_checkList[num].length;
				
				ecpMem_Set(&RecData[2], 0xFF, RecData[1]);
			}
			
			tag_operateList[num].func();
		}
		
		if(G_NowNumber == 1)
		{
			return;
		}
	}
}
	
	
void LiveGetCardInfoExcute(void)
{
	memref X_A fileAddr;

	tempBuf[1] = 0;
		
	fileAddr = SelectTFFile(DF_MF, EF_ICCID);

	if(fileAddr == FALSE)
	{
		return;
	}

	ReadTFBuf(fileAddr, 0, &ResData[2], 10);
	
	ResData[0] = 0x08;//tag
	ResData[1] = 0x0A;//length
	
	AddTLV(&tempBuf[0], &ResData[0]);
	
	fileAddr = SelectTFFile(DF_MF, 0x2F02);

	if(fileAddr == FALSE)
	{
		return ;
	}

	ReadTFBuf(fileAddr, 0, &ResData[2], 10);
	
	if(ResData[2] == 0xFF) //第一字节为省代码，省代码取值范围为00-99
	{
		return ;
	}
	
	ResData[0] = 0x0E;//tag
	ResData[1] = 0x0A;//length
	
	AddTLV(&tempBuf[0], &ResData[0]);
		
	cephDisplayTextSingle(0x01, 0x04, &tempBuf[1]);
	
	return;

}

void LiveGetCardInfoHandler(u8 ptr)
{
	if((RecData[ptr] == 0x0A) && (RecData[ptr+1] == 0x00))
	{	
		PendingMessageAlert(D_GetCardInfoTag,0);  
		
		return;
	}
}


void LiveWriteCardMac(memref eepromAddr,u8 * cpKey,u8 cpResult[8],u16 nEnterLen)
{
	u16 I_A i,padNum;
	u8 I_A j;
	u8 X_A cpEnter[8];
	
	padNum = nEnterLen % 8 ;

	if(padNum == 0x00)     //写卡/两步一块   填充数据规则
	{
		 SetEepromEX(eepromAddr+nEnterLen,0x80,1);	
 
        SetEepromEX(eepromAddr+nEnterLen+1,0x00,7);				

		nEnterLen = nEnterLen + 8;
  }
	else
	{
		i = (8-padNum) - 1;  //写卡/两步一块 填充80 后需要填充00的字节数
			
		SetEepromEX(eepromAddr+nEnterLen,0x80,1);
			
		SetEepromEX(eepromAddr+nEnterLen+1,0x00,i);
			
		nEnterLen = 8*(nEnterLen/8) + 8;

  }

	ecpMem_Set(cpResult,0x00,0x08);

	for(i=0x00;i < nEnterLen;i=i+0x08)
	{
		ReadEeprom((u8 *)&cpEnter[0],eepromAddr+i,8);

		for(j=0;j<0x08;j++)
		{
		  	cpEnter[j]=cpEnter[j] ^ cpResult[j];
	  } 
		
	   TripleDES(&cpEnter[0], &cpKey[0], &cpResult[0], CBC_Encrypt, 8,0x10);
	    
	}
}


u8 LiveWriteCardDecrypt(u8 reference, u8 headNo, u8 smsType,  u8 *tpoa, u8 random[8])
{	
	u8  I_A result;
	u8  I_A lgth;						//一次解包处理的字节长度
	u8  I_A lastLgth;					//一次解包后剩余的不足8字节的余数
	u8  I_A i;  //根据TAR判断是现场写卡两步一块
	u8  I_A total,ref;
	u8  I_A recIndex;
	memref X_A fileAddr,keyAddr;
	u16 I_A usedSize,offset,nextOffset,cpl;
	u8  X_A mac[4], key[16], recNoBuf[16],cc[8],macKey[16];

	lastLgth = 0;
	
	if((smsType == TYPE_SMS_ENV_SINGLE)||(smsType == TYPE_SMS_UPD_SINGLE))
	{
		total = 0x01;
	}
	else
	{
		total =  ToolkitFindSpecifiedRFMSMS(MODE_FIND_TOTAL,reference,0x00,smsType,tpoa);
	}

	if(total == 0)
	{
		return FALSE;
	}
	
	usedSize = 0x00;
	
	recNoBuf[0] = total;
	
	G_LIVEWRITECARDTYPE = 0x00;
	
	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);

	for(i = 1; i <= total; i ++)
	{
		
		if((smsType == TYPE_SMS_ENV_SINGLE)||(smsType == TYPE_SMS_UPD_SINGLE))
		{
			recIndex = reference;
		}
		else
		{
			recIndex = ToolkitFindSpecifiedRFMSMS(MODE_FIND_RECNO,reference,i,smsType,tpoa);	
		}
		
		recNoBuf[i] = recIndex;
		
		ReadLFBuf(fileAddr, recIndex, 1, &ref, sizeof(u8));   //保存批次
		
		ReadLFBuf(fileAddr, recIndex, 17, (u8 X_A *)&RecData[0], sizeof(u8));       //2800/6F7F  记录缓存的长度字节

		if((RecData[0] == 0x00)||(RecData[0] == 0xFF))
		{
			
			return Result_Incomplete;  
			
		}

		ReadLFBuf(fileAddr, recIndex, 18, (u8 X_A *)&RecData[1], RecData[0]);

		if(i == 0x01)
		{
			if((RecData[4] != 0x06)||(RecData[5] != 0x00))  //spi 的值为 06 00    
			{
				return FALSE;
			}
			
			if((RecData[8]!=0xB0)&&(RecData[9]!=0x00)&&(RecData[10]!=0xF2))             //现场写卡/两步一块TAR  B0 00 F1 /B0 00 F2
			{
				return FALSE;
			}
				 
			keyAddr = SelectLFFile(DF_MF, 0x3F01);
					
			if(keyAddr == FALSE)
			{
					return FALSE;
			}
					
			ReadTFBuf(keyAddr, 0, &key[0], sizeof(key));
			
            lgth = RecData[0] - 10;								//DES加密数据不包含CPL,CHL,SPI,KIC,KID,TAR            两步一块/现场写卡的下行 头部分 没有包含随机数

			lastLgth = lgth &0x07;

			if(lastLgth != 0x00)
			{
				lgth -= lastLgth;

				ecpMem_Copy((u8 X_A *)&RecData[D_TmpBufStart+8], (u8 X_A *)&RecData[lgth + 11], lastLgth);		//保存本次剩余数据
			}

			ecpMem_Copy(&cc[0], (u8 X_A *)&RecData[lgth+3], 8);		//本次的最后一个8字节块

//			TripleDES((u8 X_A *)&RecData[11], &key[0], (u8 X_A *)&RecData[11], CBC_Decrypt, lgth);			//从CNTR开始解密   java平台修改了 TripleDES
			
			TripleDES((u8 X_A *)&RecData[11], &key[0], (u8 X_A *)&RecData[11], CBC_Decrypt, lgth,16);			//从CNTR开始解密   密钥长度字节16？
      
			cpl = (((u16)RecData[1]<<8)&0xFF00) + RecData[2];
							
			if(RecData[30] != (cpl - 28 - RecData[16]))//写卡后续数据长度 ≠ CPL - 固定字节28（CHL~写卡后续数据长度）- 填充字节
			{
				return FALSE;
			}
											
			ecpMem_Copy(&mac[0], &RecData[17], 4);		//MAC

			ecpMem_Move(&RecData[17], &RecData[21], lgth-10);//前移报文体把MAC覆盖掉
					
			if((RecData[17] != 0x0C)&&(RecData[17] != 0x0B))	  //0x0C 自主写卡 
			{
				return FALSE;
			}
					//保存命令类型
					
			G_LIVEWRITECARDTYPE = RecData[17];
					
			ecpMem_Copy(&random[0], &RecData[18], 8);//传回随机数备结果算MAC用
					
			RecData[0] = lgth - 4 + 10;//密文数据长度-mac+CPL~TAR


			WriteLFBuf(fileAddr, recIndex,17, (u8 X_A *)&RecData[0], RecData[0]+1);		//LV格式记录解密后的文件
			
			usedSize = RecData[0];
		}
		else
		{
			lgth = RecData[0];

			if(lastLgth != 0)										//上次有余数
			{
				lgth += lastLgth;									//本次数据的长度加上上次剩余数据

				ecpMem_Move((u8 X_A *)&RecData[lastLgth+1], (u8 X_A *)&RecData[1], RecData[0]); //将上次剩余的未解密的数据和该次数据拼在一起。

				ecpMem_Copy((u8 X_A *)&RecData[1], (u8 X_A *)&RecData[D_TmpBufStart+8], lastLgth);
			}

			lastLgth = (lgth &0x07);

			if(lastLgth != 0)
			{							
				lgth -= lastLgth;
				
				ecpMem_Copy((u8 X_A *)&RecData[D_TmpBufStart+8], (u8 X_A *)&RecData[lgth+1], lastLgth);
			}

			ecpMem_Copy((u8 X_A *)&RecData[D_TmpBufStart+16], (u8 X_A *)&RecData[lgth-7], 8);	//将此次解密的原始数据的后8个字节放入BUFFER中。

//			TripleDES((u8 X_A *)&RecData[1], &key[0], (u8 X_A *)&RecData[0], CBC_Decrypt, lgth);  //java平台修改了 TripleDES函数，
			
			TripleDES((u8 X_A *)&RecData[1], &key[0], (u8 X_A *)&RecData[0], CBC_Decrypt, lgth,16);
			
			for(result=0; result<8; result ++)									//解密后的头8个字节要做异或操作。
			{			
				RecData[result] = RecData[result] ^ cc[result];
			}
			
			ecpMem_Copy(&cc[0], (u8 X_A *)&RecData[D_TmpBufStart+16], 8);	//将此次解密的原始数据的后8个字节放入BUFFER中
		
			SetLFConst(fileAddr, recIndex, 17, lgth, sizeof(u8));				//%%%需要和后条合并一次写入
			
			WriteLFBuf(fileAddr, recIndex,18, (u8 X_A *)&RecData[0x00], lgth);	//空2字节给调整8字节整数倍使用
			
			usedSize += lgth;	
		}
	}
	
	offset  = 0x00;

	offset = ToolkitProcessHeader(headNo,usedSize+3,0x01);  //3为027000的长度

	G_formatedSPIOffset = offset ;	    //指向当前envelopdata的CPL字段

	//todo 将解密后的数据写入envelophandlerbuf中，在该结构体中进行mac计算

	usedSize  = 0x00;

	nextOffset = offset;

	for(i =1; i<= total; i++)
	{
		recIndex = recNoBuf[i];

		ReadLFBuf(fileAddr, recIndex, 17, &RecData[0], sizeof(u8));

		ReadLFBuf(fileAddr,recIndex,18,&RecData[1],RecData[0]);

		WriteTFBuf(G_EnvelopHanderAddr,nextOffset,&RecData[1],RecData[0]); 

		nextOffset += RecData[0];
		
		usedSize += RecData[0];	
	}
	
	TripleDES(&random[0], &key[0], &macKey[0], CBC_Encrypt, 8,0x10);//计算mac会话密钥
				
	for(i=0; i<8; i++)
	{
		macKey[8+i] = ~random[i];
	}
				
	TripleDES(&macKey[8], &key[0], &macKey[8], CBC_Encrypt, 8,0x10);
		
	ecpMem_Copy(&key[0], (u8  *)&macKey[0], 16);
		
	ResData[0] = 16;
    
	ecpMem_Copy(&ResData[1], &macKey[0], sizeof(macKey));
    
	WriteVariable(var_MacKey, &ResData[0]);
	
	LiveWriteCardMac(G_EnvelopHanderAddr+sizeof(TranFileHead)+offset, &key[0], &cc[0], usedSize);//计算mac

	if(ecpMem_Compare(&mac[0], &cc[0], 4) != EQUAL)
	{
		return Result_MacErr;                 //mac校验错误
	}
	else
	{
		return TRUE;
	}
}


/*-------------------------------------------------------------------------------------------------------//

  Function name:	GetRfmCmd
 
  Input parameter:	*offset:当前记录中已读取的长度
  
				fileAddr:RFM数据 解包后所在的文件
				
				rfmDataInfo:批次号、索引、总数

				mode :		0x00: 取tag
				
							0x01:value
 
 Output parameter:FALSE:失败
 
 				TRUE:成功，继续执行

 				0x02:到达填充位，命令结束
 
 Description: 		从RFM明文中取命令数据

 status:			new

 -------------------------------------------------------------------------------------------------------------------*/

u8 LiveWriteCardWriter(memref dataAddr,u16 offset)
{
	u8 I_A   result, num;
	u8 I_A cmdLength,  pCntr;
	u16 I_A ptr;
	memref X_A fileAddr1;

	if(dataAddr == FALSE)
	{
		return FALSE;
	}

    ReadTFBuf(dataAddr,offset+15,&pCntr,0x01); //读取 CPL

	ReadTFBuf(dataAddr,offset,&RecData[0],0x02); //读取 CPL
	
	cmdLength = ((RecData[0]<<8)&0xFF00) + RecData[1] - 28 - pCntr;			//多条写卡数据总长度，包含填充字节；18=CHL+SPI+KIC+KID+ATR+CNTR+PCNTR+MAC+T+Random+L+padding
	
	ptr = offset + 26 ; // 26 = CPL + CHL +SPI +KIC +KID +TAR +CONTR +PCNTR +TAG + RANDOM +L(写卡数据长度)
	
	while(cmdLength > 0)
	{
		ReadTFBuf(dataAddr,ptr,&RecData[0],0x02); //取tag and tag's length
		
		ptr = ptr +2;
		
		ReadTFBuf(dataAddr,ptr,&RecData[2],RecData[1]); //取value
		
		ptr = ptr + RecData[1];
		
        cmdLength = (cmdLength > RecData[1]) ? (cmdLength - RecData[1]-2) : 0;//RecData[1]为该TLV的L
		
		if(RecData[0] == 0x10)
		{
			fileAddr1 = SelectTFFile(0x2700,0x6F02);
	
			WriteTFBuf(fileAddr1, 0, &RecData[1], RecData[1]+1);
			continue;
		}
		
		for(num = 0 ; ; num ++)
		{
			if((tag_operateList[num].tag == RecData[0])||(tag_operateList[num].tag == EOL))
			{
				result = tag_operateList[num].func();
	
				if(result != TRUE)
				{
//					ClearRFMTmpPackage(dataAddr, packageInfo[0]);    //退出后清除
					return (Result_WriteErr | RecData[0]) ;
				}
	
				break;
			}
		}
	}
	
//	ClearRFMTmpPackage(dataAddr, packageInfo[0]);   //退出后清除
	
	return Result_Success;          //写卡成功
}



u8 CONST WriteSuc[]={
	// 0   写卡成功   0   9
	8,0x51,0x99,0x53,0x61,0x62,0x10,0x52,0x9F,
};

u8 CONST WriteError[]={
	// 0   写卡失败   0   9
	8,0x51,0x99,0x53,0x61,0x59,0x31,0x8D,0x25,
};

u8 CONST PIDDCS[] = {0x02,0x00,0x04};

u8 LiveWriteCardSendSMS(memref dataAddr,u8 result,u16 offset)
{
	u8 X_A key[17], ucTemp[8], cpResult[8],cmdresult;
	u8 I_A i, j, sendCounter, maxSendCounter;
	memref X_A fileAddr,fileAddr1, counterAddr;

	fileAddr = SelectTFFile(DF_STK, 0x6EEE);

	ReadTFBuf(fileAddr, 0, &cpResult[0], 8);

	key[0] = ReadVarible(var_MacKey, &key[1]);

	RecData[0] = 9;//result(1)+cpResult(8)
	RecData[1] = result;
	ecpMem_Copy(&RecData[2], &cpResult[0], 8);

	RecData[10] = 0x80;

	for(i=1; RecData[0]<16; i++)//填充以达8倍?
	{
		RecData[0] += 1;

		RecData[10+i] = 0x00;
	}

	ecpMem_Set(ucTemp,0x00,8);

	for(i = 1; i < RecData[0]; i += 8)//算mac
	{
		for(j = 0 ; j < 8 ; j ++)
		{
			ucTemp[j] = ucTemp[j] ^ RecData[i+j];
		}

		TripleDES(&ucTemp[0], &key[1], &cpResult[0], CBC_Encrypt, 8,0x10);

		ecpMem_Copy(&ucTemp[0], &cpResult[0], 8);
	}

	fileAddr = SelectLFFile(DF_STK, 0x6F03);

	ReadLFBuf(fileAddr, 1,0, &tempBuf[0], 1);
    
	ReadLFBuf(fileAddr, 1,1, &tempBuf[1], tempBuf[0]);
    
	AddValue(&tempBuf[0],(u8 *)&PIDDCS[0]);
	
	if((result != FALSE)&&(result != Result_MacErr))
	{
		tempBuf[150] = 0x08;
		tempBuf[151] = 0x07;
		tempBuf[152] = 0x0D;
		tempBuf[153] = 0x05;
		tempBuf[154] = result;
		ecpMem_Copy(&tempBuf[155], &cpResult[0], 4);
		if(result == 0x30)
		{
			G_SendControl = 0x01;
		}
		else
		{
			G_SendControl = 0x00;
		}
	}
	
	if((result != FALSE)&&(result != Result_MacErr) )
	{
		AddValue(&tempBuf[0],(u8 *)&tempBuf[150]);

		WriteVariable(var_sendCode, &tempBuf[0]);

		sendCounter = 0;

		counterAddr = SelectTFFile(0x2700, 0x6F01);

		ReadTFBuf(counterAddr, 1, &maxSendCounter, 1);

		do{
			tempBuf[0] = ReadVarible(var_sendCode, &tempBuf[1]);

			cmdresult=cephSendShortMessageSingle(0x00, NULL, NULL,(u8 *)&tempBuf[0]);

			if((RecData[11] == 0x00) || (RecData[11] == 0x01) || (RecData[11] == 0x02) || (RecData[11] == 0x04))
			//发送成功
			{
				if(G_SendControl == 0x01)//下行校验成功
				{
					cmdresult=LiveWriteCardWriter( dataAddr,offset);

					if(cmdresult==Result_Success)
					{
						fileAddr1 = SelectTFFile(0x2700,0x6F01 ); //写卡成功，不许再更新
						SetTFConst(fileAddr1, 0x00, 0x01, 0x01);  //fwl
#if UICC__APP__WHITECARD		
						fileAddr = SelectTFFile(DF_STK, EF_WhiteCardConfig);
						ReadTFBuf(fileAddr, 2, &tempBuf[0], 1);
						tempBuf[0] = tempBuf[0] | 0x01;
						SetTFConst(fileAddr, 2, tempBuf[0], 0x01);
#endif

						fileAddr1 = SelectTFFile(0x2700,0x6F02);
                        
						ReadTFBuf(fileAddr1, 0, &tempBuf[0],1);
                        
						if((tempBuf[0] == 0x00)||(tempBuf[0] == 0xFF))
						{
							cephDisplayTextSingle(0x81, UCS2, (u8 *)WriteSuc);//默认显示
						}
						else
						{
							tempBuf[0] = tempBuf[0]-1;

							ReadTFBuf(fileAddr1, 1, &tempBuf[1],1);

							if(tempBuf[1] == 0x04)
							{
								ReadTFBuf(fileAddr1, 2, &tempBuf[1],tempBuf[0]);
                                
								AsciiToUCS2(&tempBuf[0],&tempBuf[0]);
							}
							else if(tempBuf[1] == 0x08)
							{
								ReadTFBuf(fileAddr1, 2, &tempBuf[1],tempBuf[0]);
							}
                            
							cephDisplayTextSingle(0x81, UCS2, (u8 *)&tempBuf[0]);

							fileAddr1 = SelectTFFile(0x2700,0x6F02 );
                            
							SetTFConst(fileAddr1, 0x00, 0x00, 0x8C);  //fwl
						}
						
						ClearLociSqn();
					}
					else
					{
						cephDisplayTextSingle(0x81, UCS2, (u8 *)WriteError);
					}
				}
				else
				{
					cephDisplayTextSingle(0x81, UCS2, (u8 *)WriteError);
				}

				break;
			}
			else if((RecData[11] == 0x20) || (RecData[11] == 0x21))//重发
			{
				sendCounter++;
			}
			else//发送失败
			{
				cephDisplayTextSingle(0x81, UCS2, (u8 *)WriteError);

				break;
			}
		}while(sendCounter < maxSendCounter);

		if(sendCounter >= maxSendCounter)
		{
			cephDisplayTextSingle(0x81, UCS2, (u8 *)WriteError);
		}
	}

	return  TRUE;
}


/*此时文件结构：批次/索引/总数 + 该条DL（含填充）+ D [首条含CPL+CHL+SPI/KIC/KID+ATR+CNTR+PCNTR共16字节]
rec1:040102 D8 01061106000505B000F20000000000000B1122334455667788E7010A986800214365870921430209084906001111212299030891683108706505F0040831323334FFFFFFFF050835363738FFFFFFFF060837353833363336330708373538333633363308100A62DDE4C77210DAD5FC47F1693124500978014CFF0298FF032E3B048F38052C300619030753FF08FF0901862E02CEFF03810F040D9C0595810666B2077323FFFFFF01B3C0FFFFFF0386FFFFFFFF057F1C060F4AFFFFFFFFFFFF01C07AFFFFFF0360B60410FF0527FF06463907FFDE0820FFFF 82AB16EF6EFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
rec2:040202 30 FFFFFFFFFFFFFFFF04FF85FFFFFF0648FF0734FF08FF100A10359484778615E5EBA5866A163DB244B080000000000000 FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF 
*增加对数据非法的判断  modify by licaizhang  测试用例：E:\Java-ChinaMobile3GPPCos\develop\others\test\现场写卡-独立\034、049
*/

u8 LiveWriteCardCheak(memref dataAddr,u16 offset)
{
	u8 I_A    num,tagnum;
	u8 I_A cmdLength, pCntr, tagStatus, tempTag, i;
	u16  I_A ptr;
	

	tagStatus = 0;//用于标记必要tag
	tagnum =0x00;//统计TAG 为0x10 的次数是否重复，如果tagnum大于1，就报错

	ReadTFBuf(dataAddr, offset+15, &pCntr, 1);//pCntr	

	ReadTFBuf(dataAddr, offset, (u8 *)RecData, 0x02);		//读取长度  CPL
	
	ReadTFBuf(dataAddr, offset+25, &cmdLength, 1);//写卡数据长度
	
	cmdLength += 5 + 1 + 4 + 10; //加cntr(5)+pcntr(1)+mac(4)+T/RAND/L(10)     加密数据的长度 
	
	if(((cmdLength/8 + 1) * 8 - cmdLength) != pCntr)
	{
		return Result_DecryptErr;
	}
	

	cmdLength = ((RecData[0]<<8)&0xFF00) + RecData[1] - 28 - pCntr;			//多条写卡数据总长度，包含填充字节；18=CHL+SPI+KIC+KID+ATR+CNTR+PCNTR+MAC+T+Random+L+padding
	
	ptr = offset + 26;          // 26 = CPL + CHL +SPI +KIC +KID +TAR +CONTR +PCNTR +TAG + RANDOM +L(写卡数据长度)
	
	while(cmdLength > 0x01)     //至少TL两字节长度
	{
	
		ReadTFBuf(dataAddr,ptr,&RecData[0],0x02); //取tag and tag's length

		ptr = ptr + 2;
		
		cmdLength = cmdLength -2;	

		if((cmdLength == 0x00) ||(cmdLength < RecData[1]))      //V的数据为空 或者  剩余数据长度<当前tag的L
		{
			return FALSE;
		}
		else
		{
			cmdLength -= RecData[1];
		}		
		
		if(RecData[0] == 0x10)
		{
			ReadTFBuf(dataAddr,ptr,&RecData[2],RecData[1]+2); //取value   判断是否填充字节  多读取两个字节				
		} 
		else
		{
			ReadTFBuf(dataAddr,ptr,&RecData[2],RecData[1]); //取value
		}
				
		ptr = ptr + RecData[1];
		 

		if(RecData[0] == 0x10)                          		//增加TAG 为0x10的判断
		{
			if(((RecData[2+RecData[1]]>0x07)&&(RecData[2+RecData[1]]!=0x80)&&(RecData[2+RecData[1]]!=0x10))||((RecData[2+RecData[1]]==0x00)))
			{	
				return 0x4A;
			}
			if(tagnum != 0x00)
			{
				return Result_WrongTag;
			}
	
			tagnum = tagnum+1;
	
			continue;
		}
		//结束
		for(num=0; num<7; num++)			//检查数据正确性
		{
			if(tag_checkList[num].tag == RecData[0])
			{
				for(i=1,tempTag=0x01; i<RecData[0]; i++)//给tag相应bit打标记
				{
					tempTag *= 2;
				}
	
				if((tagStatus&tempTag) == tempTag)//重复tag
				{
					return Result_WrongTag;
				}
	
				tagStatus |= tempTag;
	
	
				if(tag_checkList[num].length == RecData[1])
				{
					break;
				}
				else
				{
					return (Result_DataErr | RecData[0]);
				}
			}
		}	//end for(num=0; num<7; num++)
	
		if(num >= 7)	//出现不必要tag
		{
			return Result_WrongTag;
		}
	}//end while
	
	if(cmdLength != 0x00)   //写卡数据有多余的
	{
		return FALSE;
    }

	if((tagStatus & 0x7f) != 0x7f)	//有必要tag没出现
	{
		return Result_Incomplete;
	}
	else
	{
		return Result_Success;
	}
}


void LiveWriteCardDisplayResult(u8 result, u8 random[8])
{
	u8 X_A key[17], ucTemp[8], cpResult[8];
	u8 X_A i, j;

	key[0] = ReadVarible(var_MacKey, &key[1]);
	
	RecData[0] = 9;//result(1)+random(8)
    
	RecData[1] = result;
    
	ecpMem_Copy(&RecData[2], &random[0], 8);
	
	RecData[10] = 0x80;
	
	for(i=1; RecData[0]<16; i++)//填充以达8倍数
	{
		RecData[0] += 1;
		
		RecData[10+i] = 0x00;
	}
	
	ecpMem_Set(ucTemp,0x00,8);
	
	for(i = 1; i < RecData[0]; i += 8)//算mac
	{
		for(j = 0 ; j < 8 ; j ++)
		{
		  	ucTemp[j] = ucTemp[j] ^ RecData[i+j];
		}
		
    TripleDES(&ucTemp[0], &key[1], &cpResult[0], CBC_Encrypt, 8,0x10); 
    
  	ecpMem_Copy(&ucTemp[0], &cpResult[0], 8);
	}
	
	tempBuf[0] = 5;
    
	tempBuf[1] = result;
    
	ecpMem_Copy(&tempBuf[2], &cpResult[0], 4);
	
	cephDisplayTextSingle(0x00, 0x04, &tempBuf[0]);
}


/*此时文件结构：批次/索引/总数 + 该条DL（含填充）+ D [首条含CPL+CHL+SPI/KIC/KID+ATR+CNTR+PCNTR共16字节]
rec1:040102 D8 01061106000505B000F20000000000000B1122334455667788E7010A986800214365870921430209084906001111212299030891683108706505F0040831323334FFFFFFFF050835363738FFFFFFFF060837353833363336330708373538333633363308100A62DDE4C77210DAD5FC47F1693124500978014CFF0298FF032E3B048F38052C300619030753FF08FF0901862E02CEFF03810F040D9C0595810666B2077323FFFFFF01B3C0FFFFFF0386FFFFFFFF057F1C060F4AFFFFFFFFFFFF01C07AFFFFFF0360B60410FF0527FF06463907FFDE0820FFFF 82AB16EF6EFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
rec2:040202 30 FFFFFFFFFFFFFFFF04FF85FFFFFF0648FF0734FF08FF100A10359484778615E5EBA5866A163DB244B080000000000000 FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF 
*增加对数据非法的判断  modify by licaizhang  目前参考的是测试用例：E:\Java-ChinaMobile3GPPCos\develop\others\test\现场写卡-独立\034、049。要求测试部增加相关用例
*/
u8 LiveWriteCardExecuteCommand(memref dataAddr,u16 offset)
{
	u8 I_A result, num;
	u8 I_A cmdLength, tmpCmdLgth, pCntr, tagStatus, tempTag, i,tagLength;
	u16 I_A ptr;
	

	ReadTFBuf(dataAddr, offset+15, &pCntr, 1);//pCntr	

	ReadTFBuf(dataAddr, offset, (u8 *)RecData, 0x02);		//读取长度  CPL

	cmdLength = ((RecData[0]<<8)&0xFF00) + RecData[1] - 28 - pCntr;			//多条写卡数据总长度，包含填充字节；18=CHL+SPI+KIC+KID+ATR+CNTR+PCNTR+MAC+T+Random+L+padding
	
	tmpCmdLgth = cmdLength;
	
	ptr = offset + 26 ;   // 26 = CPL + CHL +SPI +KIC +KID +TAR +CONTR +PCNTR +TAG + RANDOM +L(写卡数据长度)
	
	tagStatus = 0;//用于标记必要tag
	
	tagLength = 0x00;
	
	while(tmpCmdLgth >0x01)  //至少有2个字节，将其当作tl考虑
	{
	   ReadTFBuf(dataAddr,ptr,&RecData[0],0x02); //取tag and tag's length

       ptr = ptr + 2;
    
       tmpCmdLgth = tmpCmdLgth -2; //剩余数据的长度
		
		if((tmpCmdLgth == 0x00) ||(tmpCmdLgth < RecData[1]))       //V的数据为空 或者  剩余数据长度<当前tag的L
		{
			return FALSE;
		}
		else
		{
			tmpCmdLgth -= RecData[1];
		}	
		
		ReadTFBuf(dataAddr,ptr,&RecData[2],RecData[1]); //取value
		 
		ptr = ptr + RecData[1];

		for(num=0; num<7; num++)			//检查数据正确性
		{	
			if(tag_checkList[num].tag == RecData[0])
			{
				//if((RecData[0] >= 0x01) && (RecData[0] <= 0x07))//统计必要tag的个数
				//{
                    #ifdef MORENUMBER
					if(RecData[0] == 0x01)//记录多号连写时的号码套数
					{
						count ++;
						
						if(count > 1)
						{
							if((tagStatus & 0x7f) != 0x7f)	//有必要tag没出现
							{
								return Result_Incomplete;
							}	
							
							tagStatus = 0x00;
						}
					}
                    #endif					
					
					for(i=1,tempTag=0x01; i<RecData[0]; i++)//给tag相应bit打标记
				  {
				      tempTag *= 2;
				  }
				  
				  if((tagStatus&tempTag) == tempTag)//重复tag
				  {				  
				  	return Result_WrongTag;
				  }
				  
				  tagStatus |= tempTag;
				//}
				
				if(tag_checkList[num].length == RecData[1])
				{
					tagLength += (RecData[1]+2);
					
					break;
				}	
				else
				{
					return (Result_DataErr | RecData[0]);
				}
			}
		}	//end for(num=0; num<7; num++)
			
		if(num >= 7)	//出现不必要tag
		{
			return Result_WrongTag;
		}
	}//end while
	
	if((tagStatus & 0x7f) != 0x7f)	//有必要tag没出现	
	{			
		return Result_Incomplete;
	}
	
	if((tagLength != cmdLength)||(tmpCmdLgth !=0x00))  //tag数据总的和写卡数据长度不一样，比如写卡数据后特意加了别的数据。  剩余数据长度不为0
	{
		return FALSE;
    }
	
	ptr = offset + 26 ;
	
	while(cmdLength > 0) //此处处理时，已经是合法数据
	{		
		ReadTFBuf(dataAddr,ptr,&RecData[0],0x02); //取tag and tag's length

		ptr = ptr + 2;	

		ReadTFBuf(dataAddr,ptr,&RecData[2],RecData[1]); //取value
		 
		cmdLength = (cmdLength > RecData[1]) ? (cmdLength - RecData[1]-2) : 0;//RecData[1]为该TLV的L
		 
		ptr = ptr + RecData[1];
		
		for(num = 0 ; ; num ++)
		{
			if((tag_operateList[num].tag == RecData[0])||(tag_operateList[num].tag == EOL))
			{
                #ifdef MORENUMBER					
				if(RecData[0] == 0x01)
				{
					count ++;
					
					if(count > 1)//遇到第二套号码
					{
						G_NowNumber ++;
					} 
				}
                #endif				
				result = tag_operateList[num].func();

				if(result != TRUE)
				{
					return (Result_WriteErr | RecData[0]);
				}

				break;
			}
		}  
	}
	
	return Result_Success;          //写卡成功	
}


void LiveWriteCardHandler(u8 reference, u8 headNo, u8 smsType, u8 *tpoa)
{
	memref X_A fileAddr1,fileAddr;
	u8  I_A result,  originNumber,errorCode;
	u8  X_A random[8];
	
	InitVariable();
	
	G_LIVEWRITECARDTYPE = 0x00;
	
 	errorCode = LiveWriteCardDecrypt( reference,  headNo,  smsType, tpoa, &random[0]);		
	    
	if(G_LIVEWRITECARDTYPE == 0x0C)//两不一快	
	{
#if UICC__APP__WHITECARD
		if (GetWriteCardSupport(B_LBYK) == FALSE)
		{
			return;
		}
#endif
		fileAddr1 = SelectTFFile(0x2700,0x6F01);
	
		ReadTFBuf(fileAddr1, 0, &result, 1);
	
		if(result == 0x00)
		{
			fileAddr1 = SelectTFFile(DF_STK, 0x6EEE);
	
			WriteTFBuf(fileAddr1, 0, &random[0], 8);
	
			if(errorCode == TRUE)
			{
				errorCode = LiveWriteCardCheak(G_EnvelopHanderAddr, G_formatedSPIOffset);  //G_formatedSPIOffset 这里是 CPL偏移位置
	
				if(errorCode == Result_Success)
				{
					LiveWriteCardSendSMS(G_EnvelopHanderAddr,Result_Success,G_formatedSPIOffset);  //G_formatedSPIOffset 这里是 CPL偏移位置
				}
	
				else
				{
					LiveWriteCardSendSMS(G_EnvelopHanderAddr,errorCode,G_formatedSPIOffset);       //G_formatedSPIOffset 这里是 CPL偏移位置
				}
			}
			else if((errorCode == FALSE) || (errorCode == Result_MacErr))
			{
				//ClearRFMTmpPackage(fileAddr, packageInfo[0]);  //退出后 统一清除下行
			}
			else
			{
				LiveWriteCardSendSMS(G_EnvelopHanderAddr,errorCode,G_formatedSPIOffset);         //G_formatedSPIOffset 这里是 CPL偏移位置
			}
		}
	}	
	else if(G_LIVEWRITECARDTYPE == 0x0B)//现场写卡
	{
#if UICC__APP__WHITECARD		
		if (GetWriteCardSupport(B_XCXK) == FALSE)
		{
			return;
		}
#endif
		fileAddr1 = SelectTFFile(DF_MF, EF_ICCID);

		if(fileAddr1 == FALSE)
		{
			return ;
		}
	
		ReadTFBuf(fileAddr1, 0, &result, 1);

		if((result == 0x00 || result == 0xFF))
		{
			if(errorCode == TRUE)
			{
				errorCode = LiveWriteCardExecuteCommand(G_EnvelopHanderAddr, G_formatedSPIOffset);       //G_formatedSPIOffset 这里是 CPL偏移位置
	
				if(errorCode == Result_Success)
				{
					fileAddr1 = SelectTFFile(DF_STK, 0x6F01);
	
					SetTFConst(fileAddr1, 0, 0x01, 1);
#if UICC__APP__WHITECARD					
					fileAddr = SelectTFFile(DF_STK, EF_WhiteCardConfig);
					ReadTFBuf(fileAddr, 2, &tempBuf[0], 1);
					tempBuf[0] = tempBuf[0] | 0x01;
					SetTFConst(fileAddr, 2, tempBuf[0], 0x01);
#endif					
				}
			}
	
			if((errorCode & 0xF0) == Result_WriteErr)
			{
				RecoverData(&originNumber);  // originNumber没用
			}
	
			if((errorCode != FALSE) && (errorCode != Result_MacErr))
			{
				ToolkitClearAllData(smsType,reference,tpoa);      //清除短信, modify by xuweihua@20220107
				
				LiveWriteCardDisplayResult(errorCode, &random[0]);
				
				//cephDisplayTextSingle(0x00, 0x04, &tempBuf[0]);
			}
		}
	}

}


u8 IsEmptyCard(void)
{
	memref X_A fileAddr,fileAddr1;
	u8 I_A result,ch[1];
	
    #ifdef MORENUMBER	
	u8 X_A recNum, i;
    #endif
	fileAddr1 = SelectTFFile(0x2700,0x6F01 ); //更新个人化数据
    
    if(fileAddr1 == FALSE)
	{
		return FALSE;
	}
	ReadTFBuf(fileAddr1, 0, &ch[0], 1);
	
	fileAddr = SelectTFFile(DF_MF, EF_ICCID);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadTFBuf(fileAddr, 0, &result, 1);
	
	if(((result != 0x00) && (result != 0xFF))&&(ch[0]==0x01))
	{
		return FALSE;
	}
	else
	{
        #ifndef MORENUMBER		
		return TRUE;
        #else		
		fileAddr = SelectLFFile(DF_NUMBACKUP, EF_ICCID_GSM_C);
		
		if(fileAddr == FALSE)
		{
			return FALSE;
		}
		
		recNum = GetLFRecNumLgth(fileAddr, FileNum);	
		
		for(i=1; i<=recNum; i++)
		{
			ReadLFBuf(fileAddr, i, 0, &result, 1);
		
			if((result == 0x00) || (result == 0xFF)||(ch[0]!=0x01))
			{
				return TRUE;
			}
		}
		
		return FALSE;
        #endif		
	}		
}


u8 CONST originalLoci2G[] = {0xFF,0xFF,0xFF,0xFF,0x64,0xF0,0x00,0x00,0x00,0xFF,0x01};
u8 CONST originalLoci3G[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x64,0xF0,0x00,0x00,0x00,0xFF,0x01};
u8 CONST originalLoci4G[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x64,0xF0,0x00,0x00,0x00,0x01};
u8 CONST originalKc[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x07};
/**
*modify by xuweihua@20210625: 性能优化， EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
*/
void ClearLociSqn(void)
{
	memref X_A fileAddr;
	u8  I_A num, lgth;

	fileAddr = SelectTFFile(0x7F20,0x6F7E);
	
	//if(fileAddr != FALSE)
	//{
		WriteTFBuf(fileAddr, 0, (u8 *)&originalLoci2G, sizeof(originalLoci2G));
	//}
	
	fileAddr = SelectTFFile(0x7F20,0x6F20);
	
	//if(fileAddr != FALSE)
	//{
		WriteTFBuf(fileAddr, 0, (u8 *)&originalKc, sizeof(originalKc));
	//}
	
	fileAddr = SelectTFFile(0x7FF0,0x6F73);
	
	//if(fileAddr != FALSE)
	//{
		WriteTFBuf(fileAddr, 0, (u8 *)&originalLoci3G, sizeof(originalLoci3G));
	//}
	
	fileAddr = SelectTFFile(0x7FF0,0x6FE3);
	
	//if(fileAddr != FALSE)
	//{
		WriteTFBuf(fileAddr, 0, (u8 *)&originalLoci4G, sizeof(originalLoci4G));
	//}

	fileAddr = SelectLFFile(0x7ff0,EF_SQNCheck);
	
	if(fileAddr != FALSE)
	{
		num = GetLFRecNumLgth(fileAddr, FileNum);
        
		lgth = GetLFRecNumLgth(fileAddr,FileLgth);

		fileAddr += sizeof(RecordFileHead);
		
		SetEepromEX(fileAddr, 0x00, num*lgth);
	}
}
#endif
