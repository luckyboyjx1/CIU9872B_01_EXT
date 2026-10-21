#include <string.h>
//#include "main.h"
#include "gsmpublic.h"
#include "fileSystem.h"
#include "cdma.h"
#include "stkhead.h"
#include "fileAction.h"
#include "ceph.h"
#include "error.h"
//#include "resume.h"
#include "cdma.h"



#if UICC__CDMA__SMS__DOWNLOAD


/**
 * @brief	把ASCII码值转换成对应的DTMF数值，根据CDMA标准C.S0005的Table 2.7.1.3.2.4-4 转换。
 *
 * @param 	input:需要转换的ASCII码值
 *
 * @Returns	如果函数返回结果为0x0F,则输入的TPOA的地址字符有错误
 */
 
BYTE cdmaAsciiToDTMF(BYTE input)
{
	BYTE idata result;
	
	if((input>=0x31)&&(input<=0x39))
	{
		result = input-0x30;
	}
	else if(input==0x30) // '0' --> 0xA
	{
		result = 0x0A;	
	}
	else if(input==0x2A) // '*' --> 0xB
	{
		result = 0x0B;
	}
	else if(input==0x23) // '#' --> 0xC
	{
		result = 0x0C;
	}
	else
	{
		result = 0x0F; // 非法值
	}

	return result;
}


/**
 * @brief	将CDMA下行短信中的ASCII格式的TPOA转换为ADN格式，例如0x03313233转换为0x03123F，末尾填充'F'
 *
 * @param 	offset: 指向LV, V为ASCII, input[offset]=L, 例如0x03313233, input[offset]=0x03
 *
 * @Returns	TRUE/FALSE
 *
 * @author 	WG
 *
 * @history 	created by WG, at 
 */
BYTE cdmaTpoaAsciiToAdn(BYTE * input)
{
	BYTE idata i,highByte,lowByte;
	
	for(i=1;i<=input[0];i=i+2)
	{
		highByte = cdmaAsciiToDTMF(input[i]);

		if((i==input[0])&&((input[0]%2)==1))
		{
			lowByte = 0x00;
		}
		else
		{
			lowByte = cdmaAsciiToDTMF(input[i+1]);
		}

		if((highByte==0x0F)||(lowByte==0x0F))
		{
			return FALSE;
		}
		input[(i+1)/2] = ((highByte<<4)|lowByte);
	
	}
		
	return TRUE;
}

/**
 * @brief
 *
 * @param 	offset: 指向CDMA格式地址串， RecData[offset] == TPOA.Tag
 *
 * @param 	tpoabuf: 存储转换后的TPOA，格式为(字符个数+字符串)，例如 0x03165F : 地址为3字符，即165, 'F'为填充字符
 *
 * @Returns	TRUE/FALSE
 *
 * @author 	WG
 *
 * @history 	created by WG, at 080603 16:23
 */
BYTE CdmaTpoa2GsmTpoa(BYTE offset, BYTE *tpoabuf)
{
	BYTE xdata * ptr;
	BYTE idata   len, digitMode, numMode, result;

	ptr = &RecData[offset];

	if (0x02 != (*ptr)) // tag 错误，不是02，不是源地址
	{
		return FALSE;
	}

	ptr ++ ;

	if (0x01 >= (*ptr)) // 长度错误
	{
		return FALSE;
	}
	else
	{
		len = (*ptr);
	}

	ptr ++;
	digitMode = ((*ptr)&0x80) >>0x07;
	numMode = ((*ptr)&0x40) >>0x06;

	// 4bit编码格式，高位在前，低位填充，例如源地址为123，则应为0x031230。03为源地址字符数，0为填充
	if ((0x00==digitMode) && (0x00==numMode))
	{
		ecpMem_Copy(tpoabuf, ptr, len);

		ShiftDataUncoil(tpoabuf, (len+1), tpoabuf, 2); // 左移2比特，删掉digitMode、numMode，转换后格式为0x031230格式

		if((tpoabuf[0]%2)==1)
		{
			tpoabuf[(tpoabuf[0]/2)+1] = (tpoabuf[(tpoabuf[0]/2)+1]|0x0F); // L为奇数时，把最后一个字节低四位的值零改为F
		}
		
		return TRUE;
	}
	// 8bit编码格式（即ASCII格式），高字节在前，例如源地址为123，则应为0x03313233。03为源地址字符数，无填充字符
	else if ((0x01==digitMode) && (0x00==numMode)) // 8bit编码格式处理
	{
		ptr ++;
		len --;

		ecpMem_Copy(tpoabuf, ptr, len);

		ShiftDataUncoil(tpoabuf, (len+1), tpoabuf, 1); // 长度减1，并左移1比特，删掉digitMode、numMode、numType、numPlan
		// 转换后格式为0x03313233格式

		result = cdmaTpoaAsciiToAdn(tpoabuf); 
		// 转换后格式为0x03123F格式

		if (FALSE == result)
		{
			return FALSE; // 有非法字符
		}
		else
		{
			return TRUE;
		}
	}
	else
	{
		return FALSE; // 其它格式暂不处理，todo
	}
}

/**
 * @brief 	将8比特编码的字符串压缩为7比特编码格式
 *
 * @param 	input:	V 结构
 * @param 	len
 * @param 	output:	V 结构
 *
 * @author 	XWH
 *
 * @history 	created by XWH
 */
BYTE TransBitFomat(BYTE *input, BYTE len, BYTE *output)
{
	BYTE idata  i, j;

	for (i=0, j=0;j<len;i++)
	{
		if (j == (len-1))
		{
			output[i] = (input[j] << ((j+1)%8)) | 0x00 ;
		}
		else
		{
			output[i] = (input[j] << ((j+1)%8)) | (input[j+1] >> (7-(j+1)%8)) ;
		}

		if ((j+2)%8==0)
		{
			j = j + 2 ;
		}
		else
		{
			j = j + 1 ;
		}
	}

	return ((len*7)%8 == 0x00) ? ((len*7)/8) : ((len*7)/8 + 1);
}

/**
 * @brief 	将CDMA数据短信转换为GSM数据短信
 *
 * @Returns	RecData[pos] == UDL
 *
 * @author 	WG
 *
 * @history 	created by WG, at 080603 15:45
 */

BYTE GetCDMAUDPos(void)
{
	BYTE idata  pos, len, udLen;

#if UICC__CDMA__SMS__PRL
	BYTE xdata  IsPRL;
#endif
	BYTE xdata resultBuf[17];  //LDFILE

	pos = 1 ;

	if (RecData[1] == 0x81)
	{
		pos = 3;
		len = RecData[2];
	}
	else
	{
		pos = 2 ;
		len = RecData[1];
	}
	
	while ((RecData[pos] != 0x48) && (RecData[pos] != 0xC8))	// look up for TPDU
	{	
		pos += (sizeof(BYTE) + sizeof(BYTE) + RecData[pos+1]);	//RecData[pos] == T1; RecData[pos+1] == L;

		if (pos > (len+1))										// RecData[pos] == T2;
		{
			return  FALSE;
		}
	}

	pos ++;
	
	if (RecData[pos] == 0x81)									// RecData[pos] == TPDU.Len
	{
		pos = pos + 3 ;
	}
	else
	{
		pos = pos + 2 ;
	}
	
	while (RecData[pos] != 0x02) 								//0x02:Originating Address
	{
		pos += (sizeof(BYTE) + sizeof(BYTE) + RecData[pos+1]);

		if (pos > (len+1))
		{
			return  FALSE;
		}
	}

//	IF_ProcessCdmaTpoaToGamTpoa(pos);
	
	ecpMem_Set(resultBuf, 0xFF, sizeof(resultBuf));
	
	CdmaTpoa2GsmTpoa(pos, (BYTE *)&resultBuf[1]);
	
	if((resultBuf[1] %2) == 0x00)
	{
		 resultBuf[0] = (resultBuf[1]/2) +1;
	}
	else
	{
		resultBuf[0] = (resultBuf[1]/2) + 2;
	}	
	
//	WriteVariable(LDFileTPOATag, (BYTE *)resultBuf);	//写入CDMA TPOA,后续没问道

	while (RecData[pos]!= 0x08) 								// 0x08:Bearer Data
	{
		pos += (sizeof(BYTE) + sizeof(BYTE) +RecData[pos+1]);

		if (pos > (len+1))
		{
			return  FALSE;
		}
	}

	pos += 2;

#if UICC__CDMA__SMS__PRL
	IsPRL = IsPRLSMS(RecData, pos);
#endif

	while (RecData[pos] != 0x01) 								//0x01:User Data
	{
		pos += (sizeof(BYTE) + sizeof(BYTE) +RecData[pos+1]);

		if (pos > (len+1))
		{
			return  FALSE;
		}
	}

	udLen = RecData[pos+1] ;

	pos = pos +0x02;											// +2后指向数据开始处  pos points to user data value

#if UICC__CDMA__SMS__PRL
	if (IsPRL == TRUE) 											//is PRL message
	{
		UpdatePLMNList((BYTE *)&RecData[pos], 0);

		return 	FALSE;
	}
#endif

	ShiftDataUncoil(&RecData[pos], udLen, &RecData[pos], 5); 	// 左移5个bit，去掉MSG_ENCODING
	
	return pos; 												// RecData[pos] 即OTA SMS 下行报文格式中的UDL字节(1 bytes)
}

/**
 * @brief 	
 * 
 * @param 	inputBuffer 
 * @param 	outBuffer 
 * 
 * 
 */
void formatDisplay(BYTE * inputBuffer,BYTE * outBuffer)
{
	BYTE idata i,Lgth,offset;
	
	offset = 0;

	if (inputBuffer[1] == 0x81)
	{
		offset = 0x00;
	}
	else if (inputBuffer[1] == 0x91)
	{
		outBuffer[0x01] = 0x2B;

		offset = 0x01;
	}

	Lgth = inputBuffer[0] - 0x01 ; 		//0x01 means 81 or 91

	for (i=0x00;i<Lgth;i++)
	{
		outBuffer[i*2+offset+1] = (inputBuffer[i+2]&0x0F)|0x30;

		outBuffer[i*2+offset+2] = ((inputBuffer[i+2]>>4)&0x0F)|0x30;
	}

	if (outBuffer[i*2+offset] == 0x3F)
	{
		outBuffer[0x00] = Lgth * 2 - 0x01 + offset;					
	}
	else
	{
		outBuffer[0x00] = Lgth * 2 + offset;
	}
}

/**
 * @brief 	功能:将字符串12345等转化为16进制数如:0x12,0x34,0x50;
 *		输出output[0]表示数字个数，不是字节数. 填充位在最后字节的低位
 * 
 * @param 	input 
 * @param 	output 
 * 
 * 
 */
void GSMtoDTMF(BYTE * input, BYTE * output)
{
	BYTE idata i,num;

	num = 0;

	output[0] = 0;

	for(i=0;;i++)
	{
		if (input[num]==0x20)
		{
			break;
		}

		if (input[num+1]==0x20)
		{
			output[i+1] = ((input[num]&0x0F)<<4);	

			output[0]=output[0]+1;

			num =num +1;
		}

		else
		{
			output[i+1] = input[num]&0x0F;	

			output[i+1] = (output[i+1]<<4)|(input[num+1]&0x0F);

			output[0] =output[0] +2;

			num =num + 2;
		}
	}
}

BYTE code UTKTPDUCDMA[] = {0x48,0x07,0x00,0x00,0x02,0x10,0x02,0x04,0x00};
BYTE code UTKBEARCDMA[] = {0x08,0x06,0x00,0x03,0x20,0x00,0x00,0x01};
BYTE code UTKBEARIMME[] = {0x08,0x09,0x00,0x03,0x20,0x00,0x00,0x0F,0x01,0x00,0x01};

/**
 * @brief 	将G网SMS转换为C网SMS，支持GSMDEFAULT、UCS2、直显
 * 
 * @Returns	TRUE/FALSE
 * 
 * @author 	wg
 * 
 * @version 	Ver 1.1, by wg, at 081105 11:15: 添加对直显短信的转换支持。
 */
BYTE TransSMSFomat(BYTE type)
{
	BYTE idata  ch, result, pos, cdmaPos, num, dcs, addrlength, datalength;
	BYTE xdata  tempData[0x20];
	WORD idata  bits;
	BYTE idata  constlen;
	BYTE idata  mtiByte;
	
	if(G_IsCdmaSms == FALSE)     //先判断是否是C网，不是则不需要转换成C网短信格式
	{
		return FALSE;
	}
	
	ch = 0;

#define buf RecData

	ecpMem_Copy((BYTE *)&buf[0], (BYTE *)&UTKTPDUCDMA[0], sizeof(UTKTPDUCDMA));

	pos = FindTag(&ResData[3], 0x8B, ResData[2]);        // find GSM SMS TPDU

	if (pos == FALSE)
	{
		return FALSE;
	}

	mtiByte = ResData[3+pos+1];	         //8B的length后的第一个字节

	pos = pos + 3 +3;   // GSM TPDA : RecData[pos] == TPDA.Len

	ResData[pos] = (ResData[pos]/2) + (ResData[pos]%2) + 1;

	formatDisplay(&ResData[pos], &tempData[0]);                // LV

	if (tempData[1] == 0x2B)                                   // 去掉" +"
	{
		ecpMem_Copy(&tempData[1], &tempData[2], tempData[0]-1);

		tempData[0] = tempData[0] - 1;
	}

	tempData[tempData[0]+1] = 0x20;

	for(num=1;;)
	{
		if (tempData[num]!=0x20)
		{
			if (tempData[num]==0x30)
			{
				tempData[num] = 0x3A;
			}
			else if (tempData[num] == 0x2A)
			{
				tempData[num] = 0x2B;
			}
			else if (tempData[num] == 0x23)
			{
				tempData[num] = 0x2C;
			}

			num = num + 1;
		}
		else
		{
			break;
		}
	}

	GSMtoDTMF(&tempData[1],&buf[0x0B]);          //transfer the TPDA to DTMF form

	buf[0x09] = 0 ;

	if (( buf[0x0B] % 2) == 0 )
	{
		addrlength = buf[0x0B]/2 + 1;

		ShiftDataPlus(&buf[0x0B],addrlength,&buf[0x09],2);

		addrlength =addrlength + 1;
	}
	else
	{
		addrlength = buf[0x0B]/2 + 2;

		ShiftDataPlus(&buf[0x0B],addrlength,&buf[0x09],2);

	}

	buf[0x08] = addrlength;                               // CDMA TPDA Length

	cdmaPos = 7 + buf[0x08] + 2;

	pos = pos + ResData[pos] + 2;                                // GSM dcs

	dcs = ResData[pos];

	if (dcs == 0xF6)
	{
		dcs = 0x04;
	}

	if ((dcs &0x80) == 0x80)
	{
		ecpMem_Copy((BYTE *)&buf[cdmaPos], (BYTE *)&UTKBEARIMME[0], sizeof(UTKBEARIMME));
		constlen = sizeof(UTKBEARIMME);	// 直显短信
		dcs &= 0x7F;			        // 还原DCS
	}
	else
	{
		ecpMem_Copy((BYTE *)&buf[cdmaPos], (BYTE *)&UTKBEARCDMA[0], sizeof(UTKBEARCDMA));

		if(mtiByte&0x40)                //The beginning of the TP-UD field contains a Header in addition to the short message
		{
			buf[cdmaPos+6] |= 0x08;     //If  this  field  is  set  to  ‘1’,  the  CHARi  field  in  the  User  Data  Subparameter  contains  a  User  Data  Header  	
		}

		constlen = sizeof(UTKBEARCDMA); // 普通短信
	}

	pos ++;                                                      // GSM User Data Length

	ecpMem_Copy(&buf[cdmaPos+constlen+4],&ResData[pos],ResData[pos]+1);     // User,LV

	datalength = ResData[pos] + 1;

	if (dcs==0x08)
	{
		buf[cdmaPos+constlen+4] = buf[cdmaPos+constlen+4]/2 ;
	}

	if (dcs ==0x08)
	{
		buf[cdmaPos+constlen+1] = 0x20;
	}
	else if (dcs ==0x04)
	{
		buf[cdmaPos+constlen+1] = 0x00;
	}
	else if (dcs ==0x00)
	{
		buf[cdmaPos+constlen+1] = 0x10;

		ch = buf[cdmaPos+constlen+4];

		TransBitFomat(&buf[cdmaPos+constlen+5], ch, &buf[cdmaPos+constlen+5]);
	}

	ShiftDataPlus(&buf[cdmaPos+constlen+4],datalength,&buf[cdmaPos+constlen+1],5);


	if (dcs == 0x00)
	{
		bits = 5 +8 + (7*ch);
		datalength = ((bits%8) == 0x00)? (bits/8):((bits/8)+1);

		/////////////////////// 将多余的比特位置为零 by WG, 20080617
		if ((datalength*8) > bits)
		{
			ch = buf[(cdmaPos+constlen) +datalength];

			ch = (ch >> (datalength*8 -bits)) << (datalength*8 -bits);

			buf[(cdmaPos+constlen) +datalength] = ch;
		}
		////////////////////////
	}
	else
	{
		datalength ++ ;
	}

	buf[cdmaPos+constlen] = datalength;                                      // CDMA User Data Length

	if(type == 0)
	{
		pos = FindTag(&ResData[3],0x85,ResData[2]);                          // GSM Alpha Tag

		if (pos == FALSE)
		{
			ResData[2] = 9;
		}
		else
		{
			ResData[2] = 9 + ResData[pos+3] +2;
		}	
	}
	else
	{
		pos = FindTag(&ResData[3],0x85,ResData[2]);                          // GSM Alpha Tag

		if (pos == FALSE)
		{
			ResData[2] = 0x12;
		}
		else
		{
			ResData[2] = 0x12 + ResData[pos+3] +2;
		}	
	}

	buf[cdmaPos+1] = (constlen-3) + buf[cdmaPos+constlen] + 2;                                     // CDMA Bear Data Length

	buf[0x01] = 5 + buf[0x08] + 2 + buf[cdmaPos+1] + 2;                             // CDMA TPDU Length

	result = AddTLV(&ResData[1],&buf[0]);

	if (result == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

#endif

