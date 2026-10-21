/*

 * Copyright (c) 2007,eastcompeace

 * All rights reserved.

 * FILE NAME: authorization.c 公话卡功能

 * RELEASE: 2009-06-09

 * VERSION: V1.6

 * AUTHER:	WG YJG CaiBo
 
 * HISTORY	V1.0	创建源文件
 			V1.1	删除函数GetVarible()，可使用ReadVarible()代替
			V1.2	删除函数Authorization()中，发送完REFRESH命令后的“STKPerform = TRUE”语句
			V1.3	增加广东公话卡的升级功能
			V1.4	根据广东公话卡1.0规范实现其功能
			V1.5	根据广东公话卡1.1规范，修正绑定过程中的提示语判断条件
			V1.6	优化，将全局变量alpha_text[21]改为函数局部变量	--chenl @20100813
 */

#ifdef GH_CARD
#include <string.h>
#include "stkhead.h"
#include "fileAction.h"
#include "ceph.h"
#include "Authentication.h"
#include "authorization.h"
#include "filesystem.h"
#include "arithmatic.h"
#include "gsmpublic.h"
#include "typedef.h"
#include "error.h"

BYTE xdata	G_Authorization_Status;		//鉴权标志位
BYTE xdata	G_ReadLOCI;	

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 PBOC                                                                                                                                                                                                                                                                    Function name :	 OTACalMacKey
  
  input parameter:   BYTE key[16]

  					 BYTE x[8]

  output parameter:	 void

  description:       The algorithm PBOC
  					 
  
  status:			     

 -------------------------------------------------------------------------------------------------------------------*/    
void PBOC(BYTE key[16],BYTE sourceKey[8])
{
	BYTE idata i;
	BYTE xdata nx[8];

	for(i=0;i<8;i++)
	{
		nx[i] = ~sourceKey[i];
	}
	
	TripleDES(sourceKey,key,sourceKey,ECB_Encrypt,8);
	
	TripleDES(nx,key,nx,ECB_Encrypt,8);

	memcpy(&key[0], &sourceKey[0], 8);

	memcpy(&key[8], &nx[0], 8);	
}

/*-----------------------------------------------------------------------------------------------------

  Function name:	EncryptData

  input parameter:	key_offset --> Km 在记录中的偏移

  output parameter:	TRUE --> 找到相应记录； FALSE --> 找不到相应记录

  description:		在密钥文件中查找匹配的话机厂商代码和密钥版本号的记录

  					并计算密文数据，结果存入 RecData[0]

  author:			YuJianguo WG

  test:				YuJianguo WG

  status:
----------------------------------------------------------------------------------------------------*/

BYTE EncryptData(BYTE key_offset)
{
    UWORD xdata fileAddr;

    BYTE  idata i;
	BYTE  idata recNum;
    BYTE  idata temp[2];


    fileAddr = SelectLFFile(DF_TELECOM, GH_Key_File);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadVarible(Response_varID, &RecData[0]); 		// 读取 厂商代码+密钥版本

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

    for (i=1; i<=recNum; i++)
    {
        ReadLFBuf(fileAddr, i, 0x00, &temp[0], sizeof(temp));

        if (memcmp_p(&temp[0], &RecData[0], sizeof(temp)) == EQUAL)
        {
            break;  								// 查找对应记录
        }
    }

    if (i > recNum)
    {
    	return FALSE; 								// 没找到
    }

    ReadLFBuf(fileAddr, i, key_offset, &ResData[0], 0x10); // 读取 Km(16字节)

  	// 采用标准 PBOC 密钥分散算法对 主密钥Km 进行两级分散，得到会话密钥Kc
	// 分散因子包括卡片序列号(ICCID) 的后8个字节 和 RND1

    fileAddr = SelectTFFile(DF_MF, EF_ICCID);

    ReadTFBuf(fileAddr, 0x02, &RecData[0], 0x08);

    PBOC(&ResData[0], &RecData[0]); 				// Kc1 = PBOC(Km, ICCID);


	ReadVarible(Random_varID, &RecData[0]); 			// 读取 RND1

    memset(&RecData[4], 0x20, 4);

    PBOC(&ResData[0], &RecData[0]); 				// Kc = Kc2 = PBOC(Kc1, RND1);

	
	ReadVarible(Random_varID, &RecData[100]); 		// 读取 RND2

//	TripleDES(src, 			 Kc, 		  des, 		   mode, 	    len);
	TripleDES(&RecData[104], &ResData[0], &RecData[0], ECB_Encrypt, 0x10); // Cipher =3DES(Kc, RND2); 最终结果放在RecData[0]

#ifdef GH_DEBUG
	fileAddr = SelectTFFile(DF_TELECOM, GH_DEBUG_File);

	if (fileAddr != FALSE)
	{
		WriteTFBuf(fileAddr, 0x20, &RecData[0], 0x10); // 将最终结果输入到GH_DEBUG文件
	}
#endif

    return TRUE;
}

/*-----------------------------------------------------------------------------------------------------
  Function name:	Authorization

  input parameter:	void

  output parameter:	void

  description:	    在开机后实现对移动公话机的认证
                    认证通过，使话机重启进行登网；否则显示"移动公话专用SIM卡只能用于移动公话话机！"

  author:			YuJianguo CaiBo WG

  test	:			YuJianguo WG

  status: 			20070703 modified
----------------------------------------------------------------------------------------------------*/

BYTE code Error_text[] =
	{0x2A,0x79,0xFB,0x52,0xA8,0x51,0x6C,0x8B,0xDD,0x4E,0x13,0x75,0x28,0x00,0x53,0x00,0x49,
	 0x00,0x4D,0x53,0x61,0x53,0xEA,0x80,0xFD,0x75,0x28, 0x4E,0x8E,0x79,0xFB,0x52,0xA8,
	 0x51,0x6C,0x8B,0xDD,0x8B,0xDD,0x67,0x3A,0xFF,0x01}; // "移动公话专用SIM卡只能用于移动公话话机！"

BYTE code Error_text_GD[] = {
		50, 0x79,0xFB,0x52,0xA8,0x51,0x6C,0x8B,0xDD,0x4E,0x13,
			0x75,0x28,0x00,0x53,0x00,0x49,0x00,0x4D,0x53,0x61,
			0x53,0xEA,0x80,0xFD,0x75,0x28,0x4E,0x8E,0x5D,0xF2,
			0x7E,0xD1,0x5B,0x9A,0x76,0x84,0x79,0xFB,0x52,0xA8,
			0x51,0x6C,0x8B,0xDD,0x8B,0xDD,0x67,0x3A,0xFF,0x01};		//"移动公话专用SIM卡只能用于已绑定的移动公话话机！"

 
void Authorization (void)
{
	BYTE xdata alpha_text[21];
    BYTE idata result,key_offset;
#ifdef GH_DEBUG
	UWORD idata fileAddr;
#endif

	InitVariable();

	if (G_Authorization_Status == TRUE)
	{
		return;
	}
	
	G_Authorization_Status = FALSE;	        	// "允许网络鉴权"状态置为 FALSE

#ifdef GH_DEBUG
	fileAddr = SelectTFFile(DF_TELECOM, GH_DEBUG_File);

	if (fileAddr != FALSE)
	{
		ReadTFBuf(fileAddr, 0x00, &RecData[1], 0x14); 	// GH_DEBUG 模式，从debug文件里读取 20 个字节，代替随机数
	}
	else
	{
		Random_number_generator(&RecData[1], 0x14); // 随机数（20个字节），存储在变量0x01
	}
#else
	Random_number_generator(&RecData[1], 0x14); // 随机数（20个字节），存储在变量0x01
#endif
	RecData[0] =0x14;

#ifdef GH_CARD_GD
	key_offset =(((RecData[4]&0x07) %0x03) *0x10) +2;// Km 偏移
#else
	key_offset =(((RecData[4]&0x07) %0x05) *0x10) +2;// Km 偏移
#endif

	WriteVariable(Random_varID, &RecData[0]);

	alpha_text[0] = 0x14;
	memcpy(&alpha_text[1],&RecData[1],0x14);

#ifdef GH_CARD_GD
	result = cephGetInputSingle(0x03, Response_varID, 0x80, alpha_text, 0x00, 0x14);
#else
	result = cephGetInputSingle(0x03, Response_varID, 0x80, alpha_text, 0x12, 0x12);
#endif

	if (result == CMD_Success) 						// 返回结果存放在变量0x02
    {
		result = EncryptData(key_offset);           // 计算 卡端数据，存储在 RecData[0]

		if (result == TRUE)
		{
			ReadVarible(Response_varID, &ResData[0]);// 读取 话机返回结果

			result = memcmp_p(&RecData[0],&ResData[2],0x10); // memcmp(CARD, PHONE)

			if (result == EQUAL)
			{
				G_Authorization_Status = TRUE;	    // "允许网络鉴权"状态置为TRUE

#ifdef GH_CARD_GD
				InitProactiveCommand(CMD_REFRESH, 0x03, DEVICE_ME);
				STKPerform = TRUE ;
#else
				//cephRefreshSingle(0x03, NULL);
			    cephRefreshSingle(0x03);
#endif				

				return;
			}
		}

    }

#ifdef GH_CARD_GD
	cephDisplayTextSingle(0x81,0x08, (BYTE *)Error_text_GD); 	//"移动公话专用SIM卡只能用于已绑定的移动公话话机！"
#else
	cephDisplayTextSingle(0x80,0x08, (BYTE *)Error_text); 	// "移动公话专用SIM卡只能用于移动公话话机！"
#endif

}



#ifdef GH_CARD_GD

/*-----------------------------------------------------------------------------------------------------
  Function name:	GHCalcMAC

  input parameter:	* dataBuf 要效验的源数据
					mac 计算出的MAC数据
					key
					len 源数据长度 

  output parameter:	TRUE	效验成功
					FALSE	效验失败

  description:		对源数据计算MAC，保存在mac中	    

  author:			

  test	:			

  status: 			
----------------------------------------------------------------------------------------------------*/

BYTE GHCalcMAC(BYTE * dataBuf, BYTE mac[8], BYTE key[8], BYTE len)
{
	BYTE idata i;
	BYTE idata j;
	BYTE xdata c[8];
	BYTE xdata d[8];

	memset(c, 0x00, 8);

	for(i=0; i<len; i+=8)
	{
		if ((i + 8) > len)						//最后一页
		{
			memset(d, 0x20, 8);

			memcpy(d, &dataBuf[i], len - i);
		}
		else
		{
			memcpy(d, &dataBuf[i], 8);
		}

		for(j=0; j<8; j++)
		{
			d[j] = d[j]^c[j];
		}

		SingleDES(d, key, c, CBC_Encrypt, 8);
	}

	memcpy(&mac[0], &c[0], 8);	

	return TRUE;	
}



/*-----------------------------------------------------------------------------------------------------
  Function name:	GHActiveCall

  input parameter:	void

  output parameter:	void

  description:		发送SetupCall激活号码	    

  author:			

  test	:			

  status: 			
----------------------------------------------------------------------------------------------------*/

BYTE code GHCallAlphaId[] = {40, 0x60,0xA8,0x76,0x84,0x67,0x3A,0x53,0x61,0x5C,0x1A,
								0x67,0x2A,0x7E,0xD1,0x5B,0x9A,0xFF,0x0C,0x62,0xE8,
								0x62,0x53,0x5B,0xA2,0x67,0x0D,0x75,0x35,0x8B,0xDD,
								0x8F,0xDB,0x88,0x4C,0x7E,0xD1,0x5B,0x9A,0xFF,0x1F
							};		//您的机卡尚未绑定，拨打客服电话进行绑定？

void GHActiveCall(void)
{
	UWORD xdata fileAddr;

	fileAddr = SelectLFFile(DF_TELECOM, GH_ActiveNum);

	if(fileAddr != FALSE)
	{
		ReadLFBuf(fileAddr, 0x01, 0x00, &tempBuf[0], 0x01);

		ReadLFBuf(fileAddr, 0x01, 0x01, &tempBuf[1], tempBuf[0]);

		cephSetupCallSingle(0x00, UCS2, (BYTE *)GHCallAlphaId, &tempBuf[0]);
	}	
}


/*-----------------------------------------------------------------------------------------------------
  Function name:	GHActiveProcess

  input parameter:	udPos	指向下行数据的UDL

  output parameter:	void

  description:		激活过程	    

  author:			

  test	:			

  status: 			
----------------------------------------------------------------------------------------------------*/

BYTE code GHMessageHead[] = {0x59,0x44,0x47,0x48,0x4D,0x59,0x58,0x58};		//YDGHMYXX

BYTE code GHResultDisplay1[] = {26, 0x8B,0xDD,0x67,0x3A,0x7E,0xD1,0x5B,0x9A,0x59,0x31,
									0x8D,0x25,0xFF,0x0C,0x8B,0xF7,0x80,0x54,0x7C,0xFB,
									0x5B,0xA2,0x67,0x0D,0xFF,0x01};			//话机绑定失败，请联系客服！

BYTE code GHResultDisplay2[] = {26, 0x53,0x61,0x72,0x47,0x7E,0xD1,0x5B,0x9A,0x59,0x31,
									0x8D,0x25,0xFF,0x0C,0x8B,0xF7,0x80,0x54,0x7C,0xFB,
									0x5B,0xA2,0x67,0x0D,0xFF,0x01};			//卡片绑定失败，请联系客服！

BYTE code GHResultDisplay3[] = {52, 0x60,0xA8,0x5D,0xF2,0x7E,0xD1,0x5B,0x9A,0x62,0x10,
									0x52,0x9F,0xFF,0x0C,0x8B,0xF7,0x68,0x39,0x63,0x6E,
									0x4E,0x0B,0x4E,0x00,0x6B,0x65,0x63,0xD0,0x79,0x3A,
									0x76,0x84,0x9A,0x8C,0x8B,0xC1,0x78,0x01,0x8F,0xDB,
									0x88,0x4C,0x53,0xF7,0x78,0x01,0x5F,0x00,0x90,0x1A,
									0xFF,0x01};		//您已绑定成功，请根据下一步提示的验证码进行号码开通！

BYTE GHActiveProcess(BYTE udPos)
{
	BYTE idata result, i;
	UWORD xdata fileAddr;

	udPos += 1;		//指向报文头

	result = memcmp(&RecData[udPos], (BYTE *)GHMessageHead, 8);
	if(result != 0)
	{
		return FALSE;
	}

	fileAddr = SelectLFFile(DF_TELECOM, GH_TransKey_File);
	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadLFBuf(fileAddr, 0x01, 0x00, &tempBuf[0], 16);	//传输密钥 tempBuf[0]

	memcpy(&tempBuf[20], &tempBuf[0], 16);
	memcpy(&tempBuf[40], &RecData[udPos+8], 8);

	PBOC(&tempBuf[20], &tempBuf[40]);		//传输会话密钥 tempBuf[20]

	if(RecData[udPos+131] != 0)	//公话机认证密钥更新状态成功
	{
		TripleDES(&RecData[udPos+18], &tempBuf[20], &tempBuf[50], ECB_Decrypt, 48);		//得到明文认证密钥 tempBuf[50]

		memcpy(&tempBuf[100], &RecData[udPos], 18);
		memcpy(&tempBuf[118], &tempBuf[50], 48);

		GHCalcMAC(&tempBuf[100], &tempBuf[180], &tempBuf[20], 66);

		result = memcmp(&tempBuf[180], &RecData[udPos+66], 8);

		if(result == 0)		//MAC效验正确
		{
			RecData[udPos+74] = 0x01;	//公话卡认证密钥更新状态置为0x01

			fileAddr = SelectLFFile(DF_TELECOM, GH_Key_File);
			if(fileAddr == FALSE)
			{
				return FALSE;
			}

			WriteLFBuf(fileAddr, 0x01, 0x00, &RecData[udPos+16], 2);
			WriteLFBuf(fileAddr, 0x01, 0x02, &tempBuf[50], 48);

			tempBuf[200] = 0x01;	//公话卡认证密钥状态
			
			TripleDES(&RecData[udPos+8], &tempBuf[20], &tempBuf[100], CBC_Encrypt, 8);	//加密的随机数 tempBuf[100]

			for(i=0; i<6; i++)
			{
				tempBuf[102+i] = tempBuf[102+i] % 10;
				tempBuf[102+i] += 0x30;
			}

			tempBuf[201] = 0x01;	//开通验证码显示标识

			memcpy(&tempBuf[202], &tempBuf[102], 6);	//开通验证码

			fileAddr = SelectTFFile(DF_TELECOM, GH_KeyStatus);
			if(fileAddr == FALSE)
			{
				return FALSE;
			}

			WriteTFBuf(fileAddr, 0x00, &tempBuf[200], 8);

		}
	}
	
	if(RecData[udPos+131] == 0x00)	//公话机认证密钥更新状态 失败
	{
		PendingMessageAlert(D_GH_Display1,0x00);	
	}
	else
	{
		if(RecData[udPos+74] == 0x00)	
		{
			PendingMessageAlert(D_GH_Display2,0x00);
		}
		else
		{
			PendingMessageAlert(D_GH_Display3,0x00);
		}
	}

	PendingMessageAlert(D_GH_Refresh,0x00);

	G_Authorization_Status = FALSE;

	return TRUE;

}


/*-----------------------------------------------------------------------------------------------------
  Function name:	GHOpenProcess

  input parameter:	void

  output parameter:	void

  description:		开通过程	    

  author:			

  test	:			

  status: 			
----------------------------------------------------------------------------------------------------*/

BYTE code GHDisplayText1[] = {28,0x6B,0x22,0x8F,0xCE,0x4F,0x7F,0x75,0x28,0x4E,0x2D,
								0x56,0xFD,0x79,0xFB,0x52,0xA8,0x00,0x47,0x00,0x33,
								0x4F,0xE1,0x60,0x6F,0x67,0x3A,0xFF,0x01};		//欢迎使用中国移动G3信息机！

BYTE code GHDisplayText2[] = {40,0x63,0x09,0x78,0x6E,0x5B,0x9A,0x95,0x2E,0x4E,0x0B,
								0x6B,0x21,0x4E,0x0D,0x51,0x8D,0x66,0x3E,0x79,0x3A,
								0x6F,0xC0,0x6D,0x3B,0x78,0x01,0xFF,0x0C,0x54,0x26,
								0x52,0x19,0x8B,0xF7,0x63,0x09,0x53,0xD6,0x6D,0x88
							};		//按确定键下次不再显示激活码，否则请按取消

BYTE code GHCallAlpha1[] = {0x9A,0x8C,0x8B,0xC1,0x78,0x01,0xFF,0x1A};	//验证码：

BYTE code GHCallAlpha2[] = {0xFF,0x0C,0x62,0xE8,0x62,0x53,0x5B,0xA2,0x67,0x0D,
							0x75,0x35,0x8B,0xDD,0x8F,0xDB,0x88,0x4C,0x53,0xF7,
							0x78,0x01,0x5F,0x00,0x90,0x1A,0xFF,0x1F};	//，拨打客服电话进行号码开通？

void GHOpenProcess(void)
{
	UWORD xdata fileAddr;
	BYTE idata offset, result;

	offset = 0x00;

	fileAddr = SelectTFFile(DF_TELECOM, GH_KeyStatus);
	if(fileAddr == FALSE)
	{
		return ;
	}

	tempBuf[20] = 0x06;

	ReadTFBuf(fileAddr, 0x02, &tempBuf[21], 6);

	AsciiToUCS2(&tempBuf[20], &tempBuf[50]);

	tempBuf[100] = 8 + tempBuf[50] + 28;

	memcpy(&tempBuf[101], (BYTE *)GHCallAlpha1, 8);

	offset += 8;	

	memcpy(&tempBuf[101+offset], &tempBuf[51], tempBuf[50]);

	offset += tempBuf[50];

	memcpy(&tempBuf[101+offset], (BYTE *)GHCallAlpha2, 28);

	fileAddr = SelectLFFile(DF_TELECOM, GH_ActiveNum);

	if(fileAddr == FALSE)
	{
		return ;
	}

	ReadLFBuf(fileAddr, 0x01, 0x00, &tempBuf[0], 0x01);
	ReadLFBuf(fileAddr, 0x01, 0x01, &tempBuf[1], tempBuf[0]);

	result = cephSetupCallSingle(0x00, UCS2, &tempBuf[100], &tempBuf[0]);
	if(result == ExitStkPerform)
	{
		return ;
	} 

	result = cephDisplayTextSingle(0x81, UCS2, (BYTE *)GHDisplayText2);

	offset = FindTag(&RecData[0], 0x83, 12);

	if(offset != FALSE)
	{
		offset++;

		if(RecData[offset] == 0x00)
		{
			fileAddr = SelectTFFile(DF_TELECOM, GH_KeyStatus);

			SetTFConst(fileAddr, 0x01, 0x00, 0x01);
		}
	}
}


#endif


/**
 * @brief 		判断公话卡是否鉴权

	return		TRUE:进行鉴权	FALSE: 鉴权错误
 *
 * @author 		chenl
 *
 * @history 	created by chenl, @071220 20:01
 */

BYTE CompareGHCardAuth(void)
{
#ifdef GH_CARD_GD
	UWORD xdata fileAddr;
	BYTE idata status;

	fileAddr = SelectTFFile(DF_TELECOM, GH_KeyStatus);

	ReadTFBuf(fileAddr, 0x00, &status, 0x01);

	if(status == FALSE)		//公话卡认证密钥为空
	{
		G_Authorization_Status = TRUE;
	}
#endif //end of def GH_CARD_GD

	if(G_Authorization_Status == FALSE)
    {
#ifdef GH_CARD_GD
		ReceiveData();
#ifdef SingleSIM
		memset(&RecData[0], 0xFF, 0x0C);
		CommandFinish(0x0C);
#else	
		memset(&RecData[0], 0xDB, 1);
		memset(&RecData[1], 0x08, 1);
		memset(&RecData[2], 0xFF, 8);
		memset(&RecData[10], 0x10, 1);
		memset(&RecData[11], 0xFF, 16);
		memset(&RecData[27], 0x10, 1);
		memset(&RecData[28], 0xFF, 16);
		memset(&RecData[44], 0x08, 1);
		memset(&RecData[45], 0xFF, 8);		//拼装鉴权伪结果
		CommandFinish(53);
#endif		//end of def SingleSIM
#else
		SecuritySatisfied();
#endif	//end of def GH_CARD_GD

		return FALSE;
	}

	return TRUE;
}

#endif




