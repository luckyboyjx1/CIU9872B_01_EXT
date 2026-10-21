#include  "string.h"
#include "UploadInfoFunction.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if UICC__APP__UPLOADINFO

u8 GetSEID(u8 sdId, u8 tag, u8 * SEID)
{
	memref X_A  storeDataAddr;
	u16 I_A tagOffset;
	u8 I_A tagLen;
	
	memref X_A sdDataObj;
	
    sdDataObj = sdData_getsdDataObjByAppId(sdId);
	
	tagLen = GetSDGeneraldata(SEID, tag, sdDataObj, 0x50, 0x00);
	
	//ecpMem_Move(SEID,SEID+2,tagLen-2);

	return tagLen;
	
}
// modify by xuweihua@20220107
/*u8 CONST province[]={
	// 0   北京   0   5
	0x04,0x53,0x17,0x4E,0xAC,
	// 1   天津   5   5
	0x04,0x59,0x29,0x6D,0x25,
	// 2   河北   10   5
	0x04,0x6C,0xB3,0x53,0x17,
	// 3   山西   15   5
	0x04,0x5C,0x71,0x89,0x7F,
	// 4   内蒙古   20   7
	0x06,0x51,0x85,0x84,0x99,0x53,0xE4,
	// 5   辽宁   27   6
	0x05,0x8F,0xBD,0x5B,0x80,0x81,
	// 6   吉林   33   5
	0x04,0x54,0x09,0x67,0x97,
	// 7   黑龙江   38   7
	0x06,0x9E,0xD1,0x9F,0x99,0x6C,0x5F,
	// 8   上海   45   5
	0x04,0x4E,0x0A,0x6D,0x77,
	// 9   江苏   50   6
	0x05,0x6C,0x5F,0x80,0x82,0xCF,
	// 10   浙江   56   5
	0x04,0x6D,0x59,0x6C,0x5F,
	// 11   安徽   61   5
	0x04,0x5B,0x89,0x5F,0xBD,
	// 12   福建   66   5
	0x04,0x79,0x8F,0x5E,0xFA,
	// 13   江西   71   5
	0x04,0x6C,0x5F,0x89,0x7F,
	// 14   山东   76   5
	0x04,0x5C,0x71,0x4E,0x1C,
	// 15   河南   81   5
	0x04,0x6C,0xB3,0x53,0x57,
	// 16   湖北   86   5
	0x04,0x6E,0x56,0x53,0x17,
	// 17   湖南   91   5
	0x04,0x6E,0x56,0x53,0x57,
	// 18   广东   96   5
	0x04,0x5E,0x7F,0x4E,0x1C,
	// 19   广西   101   5
	0x04,0x5E,0x7F,0x89,0x7F,
	// 20   海南   106   5
	0x04,0x6D,0x77,0x53,0x57,
	// 21   四川   111   5
	0x04,0x56,0xDB,0x5D,0xDD,
	// 22   贵州   116   5
	0x04,0x8D,0x35,0x5D,0xDE,
	// 23   云南   121   5
	0x04,0x4E,0x91,0x53,0x57,
	// 24   西藏   126   5
	0x04,0x89,0x7F,0x85,0xCF,
	// 25   陕西   131   5
	0x04,0x96,0x55,0x89,0x7F,
	// 26   甘肃   136   6
	0x05,0x75,0x18,0x80,0x80,0x83,
	// 27   青海   142   5
	0x04,0x97,0x52,0x6D,0x77,
	// 28   宁夏   147   6
	0x05,0x5B,0x80,0x81,0x59,0x0F,
	// 29   新疆   153   5
	0x04,0x65,0xB0,0x75,0x86,
	// 30   重庆   158   5
	0x04,0x91,0xCD,0x5E,0x86,
	// 31		物联卡 163	7
	0x06,0x72,0x69,0x80,0x54,0x53,0x61
};
u8 CONST provinceOffset[]= {   0,   5,  10,  15,  20,  27,  33,  38,  45,  50,  56,  61,  66,  71,  76,  81,  86,  91,  96, 101, 106, 111, 116, 121, 126, 131, 136, 142, 147, 153, 158, 163};
*/
u8 CONST cardFirm[]={
	// 0   雅斯拓   0   7
	0x06,0x96,0xC5,0x65,0xAF,0x62,0xD3,
	// 1   武汉天喻   7   9
	0x08,0x6B,0x66,0x6C,0x49,0x59,0x29,0x55,0xBB,
	// 2   江西捷德   16   9
	0x08,0x6C,0x5F,0x89,0x7F,0x63,0x77,0x5F,0xB7,
	// 3   东信和平   25   9
	0x08,0x4E,0x1C,0x4F,0xE1,0x54,0x8C,0x5E,0x73,
	// 4   大唐微电子   34   11
	0x0A,0x59,0x27,0x55,0x10,0x5F,0xAE,0x75,0x35,0x5B,0x50,
	// 5   北京握奇   45   9
	0x08,0x53,0x17,0x4E,0xAC,0x63,0xE1,0x59,0x47,
	// 6   恒宝   54   5
	0x04,0x60,0x52,0x5B,0x9D,
	// 7   北京华虹   59   9
	0x08,0x53,0x17,0x4E,0xAC,0x53,0x4E,0x86,0x79,
	// 8   上海柯斯   68   9
	0x08,0x4E,0x0A,0x6D,0x77,0x67,0xEF,0x65,0xAF,
	// 9   欧贝特   77   7
	0x06,0x6B,0x27,0x8D,0x1D,0x72,0x79,
	// 10   楚天龙   84   7
	0x06,0x69,0x5A,0x59,0x29,0x9F,0x99,
};

u8 CONST cardFirmOffset[]= {   0,   7,  16,  25,  34,  45,  54,  59,  68,  77,  84, };
u8 CONST cardFirmCode[]= {0x00,0x02,0x03,0x04,0x05,0x07,0x08,0x09,0x0A,0x0B,0x0C};

u8 CONST fixCardInfo1[]={
	// 0   产品类型：   0   11
	0x0A,0x4E,0xA7,0x54,0xC1,0x7C,0x7B,0x57,0x8B,0xFF,0x1A,
};
u8 CONST fixCardInfo2[]={
	// 1   归属省：   11   9
	0x0C,0x00,0x0D,0x00,0x0A,0x5F,0x52,0x5C,0x5E,0x77,0x01,0xFF,0x1A,
};
u8 CONST fixCardInfo3[]={
	// 2   SEID：   20   11
	0x0E,0x00,0x0D,0x00,0x0A,0x00,0x53,0x00,0x45,0x00,0x49,0x00,0x44,0xFF,0x1A,
};
u8 CONST fixCardInfo4[]={
	// 3   供卡商：   31   9
	0x0C,0x00,0x0D,0x00,0x0A,0x4F,0x9B,0x53,0x61,0x55,0x46,0xFF,0x1A,
};
u8 CONST fixCardInfo5[]={
	// 4   COS版本：   40   15
	0x10,0x00,0x0D,0x00,0x0A,0x00,0x43,0x00,0x4F,0x00,0x53,0x72,0x48,0x67,0x2C,0xFF,0x1A,
};
u8 CONST fixCardInfo6[]={
	// 5   制卡日期：   55   11
	0x0E,0x00,0x0D,0x00,0x0A,0x52,0x36,0x53,0x61,0x65,0xE5,0x67,0x1F,0xFF,0x1A,
};

u8 CONST txtSendSucceeded[]={
	// 0   信息上报成功   0   13
	0x0C,0x4F,0xE1,0x60,0x6F,0x4E,0x0A,0x62,0xA5,0x62,0x10,0x52,0x9F,
};

u8 CONST txtSendFailed[]={
	// 0   信息上报失败   0   13
	0x0C,0x4F,0xE1,0x60,0x6F,0x4E,0x0A,0x62,0xA5,0x59,0x31,0x8D,0x25,
};
// modify by xuweihua@20220107
/*void Debug1(u8 offset, u8 tag)
{
	memref X_A f;
	
	f = SelectTFFile(0x3F00, 0x2F50);
	
	SetTFConst(f, offset, tag, 1);
}

void Debug2(u8 offset, u8 * buffer, u8 length)
{
	memref X_A f;
	
	f = SelectTFFile(0x3F00, 0x2F50);
	
	WriteTFBuf(f, offset, &buffer[0], length);
}
*/

u8 GetStatus(u8 offset)
{
	memref X_A fileAddr;
	u8 X_A status;
	
	fileAddr = SelectTFFile(DF_STK, EF_Uploadconfig);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	ReadTFBuf(fileAddr, offset, &status, 1);
	
	return status;
}


void UpdateCounter(void)
{
	memref X_A fileAddr;
	u8 X_A i;
	u16 X_A counter;
	
	fileAddr = SelectTFFile(DF_STK, EF_Uploadconfig);
	
	if(fileAddr == FALSE)
	{
		return ;
	}
	
	ReadTFBuf(fileAddr, OFFSET_CNTR, &ResData[0], 5);
	
	counter = 0x0001;

	for(i=0; i<5; i++)
	{
		counter = counter + ResData[4-i];

		ResData[4-i] = counter&0xFF;

		counter = (counter>>0x08)&0x0001;
	}

	//counter = counter&0xFF;
	
	WriteTFBuf(fileAddr, OFFSET_CNTR, &ResData[0], 5);
}


void StringToUcs2(u8 * dataBuf)
{
	u8 I_A i;
	
	for(i=dataBuf[0]; i>0; i--)
	{
		dataBuf[(i*4)] = IntToChar((dataBuf[i]&0x0F), HextoASCii);
		dataBuf[(i*4)-1] = 0x00;
		dataBuf[(i*4)-2] = IntToChar(((dataBuf[i]>>4)&0x0F), HextoASCii);
		dataBuf[(i*4)-3] = 0x00;
	}
	dataBuf[0] = dataBuf[0]*4;
}


u8 SixteenToTen2(u8 input)
{
	return input - (input/16)*6;
}

/*void SendCardInfo(void)
{
	
}
*/

u8 GetTagVauleOf2F12(u8 tag, u8 * value)
{
	memref X_A entryAddr;
	u8 X_A offset, length;
	memref X_A sdDataObj;
	
    sdDataObj = sdData_getsdDataObjByAppId(ISD_APPID);
	
    ResData[0] = GetSDGeneraldata(&ResData[1], 0x2F12, sdDataObj, sizeof(ResData), 0x00);

	offset = FindTag(&ResData[1], tag, ResData[0]);
	
	if(offset != FALSE)
	{
		//length = ResData[offset+1] - 1;//L-'04'
		
		//ecpMem_Copy(&value[0], &ResData[offset+3], length);
		length = ResData[offset+1];
			
		ecpMem_Copy(&value[0], &ResData[offset+2], length);
		
		return length;
	}
	else
	{
		return FALSE;
	}
}

/*
 * mode=0: display
 *      1: send
 */
u8 GetProvince(u8 mode, u8 * value)
{
	memref X_A fileAddr;
//	u8 X_A offset;
	
	fileAddr = SelectTFFile(DF_MF,EF_DateOfProduction); // modify by xuweihua@20220107
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	if(mode == 0x00)
	{		
		ReadTFBuf(fileAddr, 5, &ResData[0], 1);
		
		if(ResData[0] == 0)
		{
			return FALSE;
		}
		
		ReadTFBuf(fileAddr, 6, &value[0], ResData[0]);
		
		return ResData[0];
	}
	else
	{
		ReadTFBuf(fileAddr, 4, &value[0], 1);
		
		return 1;
	}
}


/*
 * mode=0: display
 *      1: send
 */
u8 GetCardFirm(u8 mode, u8 * value)
{
	//memref X_A fileAddr;
	u8 X_A i;
	
	ResData[0] = GetTagVauleOf2F12(0xA2, &ResData[1]);
	
	if(ResData[0] == FALSE) // modify by xuweihua@20220107
	{
		ResData[0] = 0x04;
	}
	else
	{
		ResData[0] = CharToInt(ResData[2], ASCiitoHex);
	}
	
	if(mode == 0x01)
	{
		value[0] = ResData[0];
		
		return 1;
	}
	else
	{
		for(i=0; i<sizeof(cardFirmCode); i++)
		{
			if(ResData[0] == cardFirmCode[i])
			{
				break;	
			}
		}
		
		if(i == sizeof(cardFirmCode))
		{
			return FALSE;	
		}
		else
		{
			ecpMem_Copy(&value[0], (u8 *)&cardFirm[cardFirmOffset[i]+1], cardFirm[cardFirmOffset[i]]);
			
			return cardFirm[cardFirmOffset[i]];
		}
	}
}


u8 GetDateOfProduction(u8 * date)
{
	memref X_A fileAddr;
	
	//fileAddr = SelectTFFile(DF_MF,EF_DateOfProduction);
	fileAddr = SelectTFFile(0x3F00, 0x2F52);
	if(fileAddr == FALSE)
	{
		return FALSE;
	}	
	ReadTFBuf(fileAddr, 0, &date[0], 4);
	
	if(date[0] != 0x20)
	{
		return FALSE;	
	}
	else
	{
		return 0x04;	
	}
}


void DisplayCardInfo(void)
{
	tempBuf[0] = 0;
	
	AddValue(&tempBuf[0],(u8 *)fixCardInfo1);//产品类型，取自2F12-A3 tag(ASCII)
	
	RecData[0] = GetTagVauleOf2F12(0xA3, &RecData[1]);
	
	if(RecData[0] != 0)
	{
		RecData[1] = RecData[0] - 1;
		
		AsciiToUCS2(&RecData[1], &RecData[1]);
		
		AddValue(&tempBuf[0],(u8 *)&RecData[1]);
	}
	
	AddValue(&tempBuf[0],(u8 *)fixCardInfo2);//归属省，取自ICCID第5字节
	
	RecData[0] = GetProvince(0x00, &RecData[1]);
	
	if(RecData[0] != 0)
	{
		AddValue(&tempBuf[0],(u8 *)&RecData[0]);
	}
	
	AddValue(&tempBuf[0],(u8 *)fixCardInfo3);//SEID，取自45 tag
	
	RecData[0] = GetSEID(0x00, 0x44, &RecData[1]);//930A FFFFFFFFFFFFFFFFFFFF
	
	if((RecData[0] != 0) && (RecData[1] == 0x93) && (RecData[2] != 0))
	{
		StringToUcs2(&RecData[2]);
		
		AddValue(&tempBuf[0],(u8 *)&RecData[2]);
	}
	
	AddValue(&tempBuf[0],(u8 *)fixCardInfo4);
	
	RecData[0] = GetCardFirm(0x00, &RecData[1]);
	
	if(RecData[0] != 0)
	{
		AddValue(&tempBuf[0],(u8 *)&RecData[0]);
	}
	
	AddValue(&tempBuf[0],(u8 *)fixCardInfo5);//COS 版本，取自2F12-A5 tag(ASCII)
	
	RecData[0] = GetTagVauleOf2F12(0xA5, &RecData[1]);
	
	if(RecData[0] != 0)
	{
		RecData[1] = RecData[0] - 1;
		
		AsciiToUCS2(&RecData[1], &RecData[1]);
		
		AddValue(&tempBuf[0],(u8 *)&RecData[1]);
	}
	
	AddValue(&tempBuf[0],(u8 *)fixCardInfo6);//制卡日期，取自私有文件
	
	RecData[0] = GetDateOfProduction(&RecData[1]);
	
	if(RecData[0] != 0)
	{
		StringToUcs2(&RecData[0]);
		
		AddValue(&tempBuf[0], (u8 *)&RecData[0]);
	}
	
	cephDisplayTextSingle(0x81,UCS2,(u8 *)&tempBuf[0]);
}


u8 GetKey(u8 keyBuf[64])
{
	memref X_A fileAddr;
	
	fileAddr = SelectTFFile(0x3F00, EF_SM2PublicKey);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	if(ReadTFBuf(fileAddr, 0, &keyBuf[0], 64) == FALSE)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}	
}



//store in ResData
void MakeAppDataForUplink(void)
{
	tempBuf[0] = 0x00;
	
	//tempBuf[200] = GetTagVauleOf2F12(0xA1, &tempBuf[201]);//SEID的类型标识
	tempBuf[202] = GetSEID(0x00, 0x44, &tempBuf[203]);//930A FFFFFFFFFFFFFFFFFFFF
	
	tempBuf[200] = 0x01;
	tempBuf[201] = tempBuf[209];//SEID的第5字节类型标识
	
	AddValue(&tempBuf[0], &tempBuf[200]);
			
	tempBuf[200] = GetProvince(0x01, &tempBuf[201]);//省代码
	
	AddValue(&tempBuf[0], &tempBuf[200]);
	
	AddValue(&tempBuf[0], &tempBuf[204]);//SEID
	
	tempBuf[200] = GetCardFirm(0x01, &tempBuf[201]);//卡商代码
	
	AddValue(&tempBuf[0], &tempBuf[200]);
	
	tempBuf[200] = GetDateOfProduction(&tempBuf[201]);//制卡时间
	
	AddValue(&tempBuf[0], &tempBuf[200]);
	
	tempBuf[201] = GetTagVauleOf2F12(0xA5, &tempBuf[202]);//COS 版本，取自2F12-A5 tag(ASCII)
	
	tempBuf[200] = tempBuf[201] + 1;
	
	AddValue(&tempBuf[0], &tempBuf[200]);
}


void MakeRpduForSendUplink(u8 * tpduBuf)
{
	memref X_A fileAddr;
	u8 X_A offset = 0;
	//u8 X_A key[16], mac[4];
	
	RecData[offset++] = 0x00;//UDL	
	RecData[offset++] = 0x00;//RPL
	RecData[offset++] = 0x00;//offset = 2
	RecData[offset++] = 0xB0;//TAR
	RecData[offset++] = 0x01;
	RecData[offset++] = 0x4E;
	
	//cntrOffset = offset;
	
	fileAddr = SelectTFFile(DF_STK, EF_Uploadconfig);
	
	ReadTFBuf(fileAddr, OFFSET_CNTR, &RecData[offset], 5);
	
	offset += 5;
	
	GetKey(&RecData[191]);
	
	MakeAppDataForUplink();//LV结构保存在&tempBuf[0]
//Debug2(16,	&tempBuf[0], tempBuf[0]+1);
//Debug2(0,	&RecData[191], 16);
	SM2_ECP_Encrypt(&tempBuf[1], tempBuf[0], &RecData[191],&RecData[offset+1]);
	
	RecData[offset] =  tempBuf[0]+0x60;
	
//Debug2(16+tempBuf[0]+1,&RecData[offset],RecData[offset]+1);	
	
	offset += RecData[offset];
	
	RecData[0] = offset;
	RecData[2] = RecData[0] - 2;
	
	fileAddr = SelectTFFile(DF_STK, EF_Uploadconfig);
	
	ReadTFBuf(fileAddr, OFFSET_TPDA, &tpduBuf[0], 1);
	ReadTFBuf(fileAddr, OFFSET_TPDA+1, &tpduBuf[1], tpduBuf[0]);//TPDA
	
	offset = tpduBuf[0] + 1;
	
	tpduBuf[offset++] = 0x00;	//PID
	tpduBuf[offset++] = 0x04; //DCS
	
	ecpMem_Copy(&tpduBuf[offset], &RecData[0], RecData[0]+1);
	
	tpduBuf[0] += 3 + RecData[0];
}


void UploadCardInfo(void)
{
	//u8 X_A address[16];
	u8 X_A counter=0;
	
	MakeRpduForSendUplink(&tempBuf[0]);
	
	while(counter < 4)//retry max = 3
	{
		//LoadSCAddress(&address[0]);	
		cephSendShortMessageSingle(0x00, NULL, NULL, tempBuf);
		
		////重发机制////
		if((RecData[11] == 0x00) || (RecData[11] == 0x01) || (RecData[11] == 0x02) || (RecData[11] == 0x04))
		{
			UpdateCounter();
			
			cephDisplayTextSingle(0x81,UCS2,(u8 *)txtSendSucceeded);
			
			return;
		}
		else if((RecData[11] == 0x20) || (RecData[11] == 0x21))
		{
			counter ++;
			
			continue;			
		}
		else
		{
			break;
		}			
	}
	
	cephDisplayTextSingle(0x81,UCS2,(u8 *)txtSendFailed);
	
	//UpdateCounter();
}

#endif

