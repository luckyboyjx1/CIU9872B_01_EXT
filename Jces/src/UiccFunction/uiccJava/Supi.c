
/*	
#include "resume.h"
#include "Stkhead.h"
#include "Authentication.h"
#include "Interface.h"
#include "JcreMain.h"
#include "Transaction.h"
#include "Gsmpublic.h"
#include "ToolkitApiPublic.h"
*/

#include <string.h>
#include <uicc_error.h>
#include "Uicc_Main.h"
//#include  "Main.h"
#include "Supi.h"
#include "FileAction.h"
#include "FileSystem.h"
#include "Gsmpublic.h"
//#include "Sha1.h"
#include "Authentication.h"
//#include "ECC_5G.h"
#include "Stkhead.h"
#include "Crypto_Constant.h"
#include "Crypto_API.h"
#include "T0ApduImpl.h"
#if UICC__AUTH__5G__SUPI
#include "TransFormat.h"

#define SHAREINFO_LEN 0x21

BYTE code type[] = {0x74,0x79,0x70,0x65};	//type

BYTE code rid[] = {0x2E,0x72,0x69,0x64};	//.rid

BYTE code schid[] = {0x2E,0x73,0x63,0x68,0x69,0x64};	//.schid

BYTE code userid[] = {0x2E,0x75,0x73,0x65,0x72,0x69,0x64};	//.userid

BYTE code hnkey[] = {0x2E,0x68,0x6E,0x6B,0x65,0x79};	//.hnkey 

BYTE code ecckey[] = {0x2E,0x65,0x63,0x63,0x6B,0x65,0x79};	//.ecckey 

BYTE code cip[] = {0x2E,0x63,0x69,0x70};	//.cip

BYTE code mac[] = {0x2E,0x6D,0x61,0x63};	//.mac 

//#include "tlsalgutil.h"
//sec1-v2.pdf 3.6.1 ANS X9.63 Key Derivation Function
//modify by xuweihua@20211105 替换函数ANS_X963_KDF成硬件算法
u8 ANS_X963_KDF(u8* Z,u8 ZLen, u8 keydatalen,u8* sharedInfo, u8 shardInfoLen, u8* K)
{
	return ANS_X963_KDF_Hal(Z,ZLen, keydatalen,sharedInfo,shardInfoLen, K); 
}


/*
*@description: 获取ecc类型非对称公私钥
*
*@param      :mode: profile_a/profile_b
*						 :pkBuf:	 	公钥值 (outPut):
*						 :skBuf： 		私钥值 (outPut)
*							
*@returns   :void
*
*@history   :create by yanjr@20190123
*/

void ECCKeyPair(BYTE mode, BYTE * pkBuf, BYTE * skBuf)
{
	
	if(mode==PROFILE_A)
	{
		ECCKeyPair_ProfileA(pkBuf, skBuf);
	}
	if(mode==PROFILE_B)
	{
		ECCKeyPair_ProfileB(pkBuf, skBuf);
	}
	return;
	
}
	
/*
*@description: 获取sharekey
*
*@param      : mode: profile_a/profile_b
							 HNPublicKey: Home Network Public Key 
							 USIMSKey: ECCKeyPair输出的私钥Key
*							 shareKey:输出的sharekey(32字节)
*
*@returns   :void
*
*@history   :create by yanjr@20190123
*/

void ECCKeyAgreet(BYTE mode, BYTE * HNPublicKey,BYTE len, BYTE * USIMSKey, BYTE * shareKey)
{	
	if(mode==PROFILE_A)
	{
		ECCKeyAgreet_ProfileA(HNPublicKey, USIMSKey, shareKey);
	}
	if(mode==PROFILE_B)
	{
		if(len == 0x40)
		{
			ECCKeyAgreet_ProfileB(HNPublicKey, USIMSKey, shareKey);
		}
		else if((len == 0x41) && (HNPublicKey[0] == 0x04))
		{
			ECCKeyAgreet_ProfileB(&HNPublicKey[1], USIMSKey, shareKey);
		}			
		else if((len == 0x21) && ((HNPublicKey[0] == 0x02) || (HNPublicKey[0] == 0x03)))//压缩格式
		{
			ECCPointDecompress(&HNPublicKey[0]);

			ECCKeyAgreet_ProfileB(&HNPublicKey[1], USIMSKey, shareKey);
		}
		else
		{
			
		}
	}
	return;
}

void ECCShareInfo_ProfileA(BYTE * USIMPKey,BYTE * ShareInfo)
{	
//		ShareInfo[0]=0x04;	
		ecpMem_Copy(&ShareInfo[0],&USIMPKey[0],32);
		return;
}

void ECCShareInfo_ProfileB(BYTE * USIMPKey,BYTE * ShareInfo)
{
		u8 Py;
		
		Py=USIMPKey[63]%2;
		if(Py==0)
		{
			ShareInfo[0]=0x02;
		}
		
		if(Py==1)
		{
			ShareInfo[0]=0x03;
		}
	
		ecpMem_Copy(&ShareInfo[1],&USIMPKey[0],32);
		return;
}
/*
*@description: 获取ShareInfo1
*
*@param      : USIMPKey: ECCKeyPair输出的公钥Key
*							 ShareInfo1:输出的ShareInfo1
*
*@returns   :void
*
*@history   :create by yanjr@20190123
*/
void ECCShareInfo(BYTE mode, BYTE * USIMPKey,BYTE * ShareInfo)
{
	if(mode==PROFILE_A)
	{
		ECCShareInfo_ProfileA(USIMPKey, ShareInfo);
	}
	else
	{
		ECCShareInfo_ProfileB(USIMPKey, ShareInfo);
	}
	return;
}

/*
*@description: ECCKDF
*
*@param      : shareKey: shareKey
							 shareInfo: shareInfo1
*							 output:输出16字节的encKey，16字节的ICB，32字节的macKey
*
*@returns   :void
*
*@history   :create by yanjr@20190123
*/
void ECCKDF(BYTE mode,BYTE * shareKey, BYTE * shareInfo,BYTE * output)
{
	if(mode == PROFILE_A)
	{
		ANS_X963_KDF(shareKey,32,64,shareInfo,32,output);
	}
	else
	{
		ANS_X963_KDF(shareKey,32,64,shareInfo,33,output);
	}
	
	return;
}
void increaseICB(BYTE *ICB,BYTE *result)
{
	BYTE idata i;
	
	ecpMem_Copy(result,ICB,16);
	
	i = 15;
	
	result[i]++;
	
	while(result[i] == 0) 
	{
		i--;
		
		if(i == 0) 
		{
			ecpMem_Set(result, 0x00,16);
		}
		
		result[i]++;
	}
	
}
/*
*@description: AES_CRT
*
*@param      : encKey: 加密key
							 keyLen: Key长度 
							 ICB:计数器
							 ICBLen:计数器的长度
*							 IMSI:需要加密的数据
							 len:加密数据的长度
							 output：加密之后的数据
*
*@returns   :len:输出长度
*
*@history   :create by yanjr@20190123
*/
BYTE AES_CTR(BYTE * encKey, BYTE keyLen, BYTE * ICB, BYTE ICBLen, BYTE * IMSI, BYTE len, BYTE * output)
{
	BYTE h,i;
	u8 inputblock;
	u8 remainlength;
	u8 AESOutput[16];
	u8 curICB[16];
	u8 curICBAdd1[16];
	
	inputblock=len/16;
	remainlength=len%16;
	
	ecpMem_Copy(curICB,ICB,16);
	
	//KeySchedule(&encKey[0]);								//利用Ki进行AES Key的初始化

	for(h=0;h<inputblock;h++)
	{
			//AESEncrypt(&curICB[0], &AESOutput[0]);
			AESEncrypt_Hal(&encKey[0], &curICB[0], &AESOutput[0]);

			for (i=0;i<16;i++)
			{
				output[h*16+i]=AESOutput[i]^IMSI[h*16+i];
			}
			
			increaseICB(curICB,curICBAdd1);
			ecpMem_Copy(curICB,curICBAdd1,16);
	}
	
	if(remainlength!=0)
	{

			//AESEncrypt(&curICB[0], &AESOutput[0]);
			AESEncrypt_Hal( &encKey[0],&curICB[0], &AESOutput[0]);

			for (i=0;i<remainlength;i++)
			{
				output[h*16+i]=AESOutput[i]^IMSI[h*16+i];
			}
	}
			
	return len;
}


/*
*@description: ECCProcess
*
*@param      : mode: profile_a/profile_b
							 HNPublicKey: Home Network Public Key 
							 len:Home Network Public Key 长度
*							 //output:输出16字节的encKey，16字节的ICB，32字节的macKey
*
*@returns   :void
*
*@history   :create by yanjr@20190123
*/
void ECCProcess(BYTE mode,BYTE * HNPublicKey, BYTE len)
{				
	ECCKeyPair(mode, (BYTE *)&RecData[0], (BYTE *)&tempBuf[70]);		
		
	ECCKeyAgreet(mode, HNPublicKey,len, &tempBuf[70], (BYTE *)&RecData[120]); //生成SHARE KEY
	
	ECCShareInfo(mode, &RecData[0],(BYTE *)&tempBuf[0]); //tempBuf[0]输出shareinfo 33个字节				
	
	ECCKDF(mode,&RecData[120], &tempBuf[0],(BYTE *)&tempBuf[0x40]); //share key ,share info 计算ENCKEY，ICB,MAC KEY	
}
/*
*@description: getImsi
*
*@param      : buf: 转换后的IMSI
*
*@returns   :TRUE :IMSI 方式有效
						 FALSE : IMSI为0XFF
*
*@history   :create by zdm@20190617
*           :modify by xuweihua@20211027 优化78指令性能，减少SearchFile的次数
*/
BYTE  getImsi(UWORD fileAddrDf,BYTE * buf)
{
	UWORD xdata fileAddr;
	BYTE idata i,MNCLen,MSINLen,tag;
	
	//fileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum], EF_IMSI);
	fileAddr = SelectFile(fileAddrDf, EF_IMSI);
		
	ReadTFBuf(fileAddr,0x01,&tempBuf[100],0x08);//imsi
	
	if((tempBuf[100]==0xFF)&&(tempBuf[101]==0xFF)&&(tempBuf[102]==0xFF)) //IMSI文件为0XFF,认为无有效IMSI，采用NAI方式
	{
		return FALSE;
	}

	tag = 0x00;
	
	//fileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum], EF_AD);
	fileAddr = SelectFile(fileAddrDf, EF_AD);
	
	ReadTFBuf(fileAddr,0x03,&MNCLen,0x01);//imsi mnc lgth
	
	//buf[0] =((tempBuf[101]<<4)|(tempBuf[100]>>4));
	
	if(MNCLen==0x02)
	{
		buf[0] =((tempBuf[101]<<4)|(tempBuf[100]>>4));
		buf[1] =0xF0|(tempBuf[101] >>0x04);
		buf[2] =tempBuf[102];
		for(i=3;i<8;i++)
		{
			buf[i]=tempBuf[100+i];
			
			MSINLen = i-2;
			
			if((buf[i]&0xF0)==0xF0)
			{				
				
				if((buf[i]&0xFF)==0xFF)
				{
					MSINLen = MSINLen - 1;
				}
				
				break;
			}	
			
		}		 
	}
	else if(MNCLen==0x03)
	{
		buf[0] =((tempBuf[101]<<4)|(tempBuf[100]>>4));
		buf[1] =((tempBuf[103]<<4)|(tempBuf[101] >>0x04));
		buf[2] =tempBuf[102];
		
		//buf[0] =((tempBuf[101]<<4)|(tempBuf[100]>>4));
		
		for(i=3;i<7;i++)
		{
			
			buf[i]=((tempBuf[101+i]<<4)|(tempBuf[100+i]>>4));
			
			MSINLen = i-2;
			
			if((tempBuf[101+i] & 0xF0) == 0xF0)
			{
				tag = 0x01;
				break;
			}
		}
		
		if(tag == 0x00)
		{
			buf[7]=0xF0|(tempBuf[107]>>4);
			
			MSINLen = MSINLen+1;
		}
				
	}
	else
	{
		return FALSE;
	}
	return MSINLen;
}

/*
*@description: getNSI
*
*@param      :
*
*@returns   :TRUE :NSI方式有效  service 130 is "available",
						 FALSE : NSI为0XFF 
*
*@history   :create by zdm@20190617
*/
BYTE getNSI(UWORD fileAddr5G, BYTE * NAI, BYTE mode)
{
	BYTE idata i,tmpbuf[4],allLenth,userLenth;

	UWORD idata fileAddr;
	
	userLenth = 0x00;

	fileAddr = SelectFile(fileAddr5G, EF_NSI);
	
	if(fileAddr==FALSE)
	{
			return FALSE;
	}		

	ReadTFBuf(fileAddr,0x00,&tmpbuf[0],0x04);//NAI
	
	if(tmpbuf[0]!=0x80)
	{
		return FALSE;
	}
	if(tmpbuf[1]!=0x81)//小于0x80
	{
		 ReadTFBuf(fileAddr,0x02,&NAI[0],tmpbuf[1]);//NAI
		 allLenth=tmpbuf[1];
	}
	else //2 个字节
	{
		  ReadTFBuf(fileAddr,0x03,&NAI[0],tmpbuf[2]);//NAI
		  allLenth=tmpbuf[2];
	}
	
	
	for(i=0;i<allLenth;i++)
	{
			if(NAI[i]==0x40)
			{
				userLenth=i;
			}	
			
	}
  switch (mode)
	{
		case NULL_SCHEME:
		{
			 return allLenth;
		}
		case PROFILE_A:
		case PROFILE_B:
		{
			return userLenth;
		}
		case DOMAIN:
		{
		#define DOMAINBUF	NAI+110	
			ecpMem_Copy(DOMAINBUF,&NAI[userLenth],allLenth-userLenth);
		#undef	DOMAINBUF	
			return (allLenth-userLenth);
		}
		default:
			return FALSE;
	}
}

/*
*@description: gerROUTINGID
*
*@param      : buf: 转换后的RoutingID
*
*@returns   :TRUE :转换正确的方式
						 FALSE :未找到该文件
*
*@history   :create by zdm@20190617
*/
BYTE getRoutingID(UWORD fileAddr5G, BYTE dataMode, BYTE * buf)
{
	BYTE idata tmp[2],result;
	UWORD idata fileAddr;

	fileAddr = SelectFile(fileAddr5G, EF_ROUTING_INDICATOR);	
	
	result = ReadTFBuf(fileAddr,0x00,&tmp[0],0x02);	
	
	if(dataMode == NAI_MODE)
	{
		
		if(tmp[1] == 0xFF)
		{
			if((tmp[0] & 0xF0) == 0xF0)
			{
				if(tmp[0] == 0xFF)
				{
					buf[0] = 0x03;
					buf[1] = IntToChar(tmp[0]&0x0f,HextoASCii);
					buf[2] = buf[1];
					buf[3] = buf[1];
					
				}
				else
				{
					buf[0] = 0x01;
					buf[1] = IntToChar(tmp[0]&0x0f,HextoASCii);
				}
					
			}
			else
			{
				buf[0] = 0x02;
			
				buf[1] = IntToChar(tmp[0]&0x0f,HextoASCii);
			
				buf[2] = IntToChar((tmp[0]>>0x04)&0x0f,HextoASCii);
			}
		}
		else if((tmp[1]& 0xF0) == 0xF0)
		{
			buf[0] = 0x03;
			
			buf[1] = IntToChar(tmp[0]&0x0f,HextoASCii);
			
			buf[2] = IntToChar((tmp[0]>>0x04)&0x0f,HextoASCii);
			
			buf[3] = IntToChar(tmp[1]&0x0f,HextoASCii);
						
		}
		else
		{
			buf[0] = 0x04;
			
			buf[1] = IntToChar(tmp[0]&0x0f,HextoASCii);
			
			buf[2] = IntToChar((tmp[0]>>0x04)&0x0f,HextoASCii);
			
			buf[3] = IntToChar(tmp[1]&0x0f,HextoASCii);
			
			buf[4] = IntToChar((tmp[1]>>0x04)&0x0f,HextoASCii);
		}						
	}
	else
	{
		ecpMem_Copy(&buf[0],&tmp[0],0x02);
	}
	
	return result;

}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 HextoASCii
  
  input parameter:   lv_src: 要转换的源数据(LV)
  					 lv_dst: 转换后的结果数据(LV)
					 (lv_src和lv_dst可以为同一个LV Buffer)
  				
  output parameter:		TRUE/FALSE	 
                                                                                                                                                
  description:       Convert Hex to ASCII (string)

  example:		Hex -> ASCii
  				(0x02, 0x01,0x2B) -> (0x04, 0x30,0x31,0x32,0x42)
  
  test:			xuqs	test ok
  
  status:			      

 -------------------------------------------------------------------------------------------------------------------*/    

BYTE HextoASC(BYTE srcLen, BYTE * lv_src, BYTE * lv_dst)
{
	BYTE idata i, j;
	
	for (i = srcLen, j = srcLen*2;( i > 0)&&( j > 1); i--, j -=2)
	{
		lv_dst[j-1] = IntToChar((lv_src[i-1]&0x0F), HextoASCii);

		lv_dst[j-2] = IntToChar(((lv_src[i-1]>>0x04)&0x0F), HextoASCii);
	}


	return TRUE;
}


/*
*@description: 获取SUCI
*
*@param      :void
*
*@returns   :void
*
*@history   :create by yanjr@20190123
*           :modify by xuweihua@20211027 优化78指令性能，减少SearchFile的次数
*           :modify by xuweihua@20211105 函数HMACSha256DoFinalSingleCall替换成GeneralHMAC
*           :modify by xwh 20220107,优化鉴权性能，0078000100直接调用底层发送函数，在底层处理下一条00780001XX指令
*/
void GetIdentity(void)
{
	UWORD xdata fileAddr,fileAddr5G,fileAddrDfUsim,fileAddr5FD0;
	BYTE idata result,mode,dataMode,keyIndex,identifier,schemeID,A0Len,i,flag,priority,NaiLen,len,MSINLen;
	WORD idata A1Len,datalen,offset,NAIOffset,A1TagLen;
	BYTE xdata IMSI[8],ROUTINGID[5],lenTag,resultBuf[2];
	
	offset = 0;
	lenTag = 0;

	if(G_SUPILEN == 0)
	{
		if(CheckCurrentDfForADF() == FALSE)
		{
			ConditionSatisfied();											//Send 6985

			return;
		}		
		
		if((G_CurrentAdfName[G_ChannelNum] != G_DF_ID_USIM)&&(G_CurrentAdfName[G_ChannelNum] != 0x7FFF))
		{
			ConditionSatisfied();											//Send 6985

			return;
		}
		
		if(ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE)
		{
			SecuritySatisfied();												//Send 6985
			return;
		}
	}
	
	if(P1 != 0x00) //目前卡片只支持计算SUCI
	{
		P1P2Error();
		return;
	}
	
	if(P2 != 0x01)
	{
		P1P2Error();
		return;
	}
	
	if(G_SUPILEN != 0)
	{
		if(P3 == G_SUPILEN)
		{
			ReadVarible(Tag_BakSupiData,RecData);

			SendData();
			
			G_SUPILEN = 0x00;
				
			CommandFinish(0x00);	
		
			return;
		}
		else
		{
			P3Error();
			return;
		}
	}
	
		
	result = 0x00;
	resultBuf[0] = 0;
	resultBuf[1] = 0;
	
	identifier = 0x00;
	
	mode = 0x00;
	
	schemeID = 0x00;
	

	//fileAddr = SelectTFFile(G_DF_ID_USIM, EF_UST);
	if(G_CurrentDfName[G_ChannelNum] == G_DF_ID_USIM)
	{
		fileAddrDfUsim =  G_CurrentDfAddr[G_ChannelNum];
	}
	else
	{
		fileAddrDfUsim = SelectFile(E_FILE_START,G_DF_ID_USIM) ;
	}

	fileAddr = SelectFile(fileAddrDfUsim, EF_UST);

	if(fileAddr != FALSE)
	{
		ReadTFBuf(fileAddr, 0x0F, &resultBuf[0], 2);
	}	
	if ((resultBuf[0]&0x18) != 0x18)
	{
		ConditionSatisfied();											//Send 6985

		return;
	}

	fileAddr5G = SelectFile(fileAddrDfUsim, DF_5GS);
	//modify by chentao@20211123 如果4f01存在则用4f01的值;中国电信离线脚本测试--pofile结构测试库V2.3.1\\data\13-SUCI Calculation by USIM\\data\13-SUCI Calculation by USIM_01
#if EUICC_ONLY || SE_AND_EUICC
	fileAddr5FD0 = SelectFile(fileAddrDfUsim, DF_SAIP);

	fileAddr = SelectFile(fileAddr5FD0, EF_SUCI_CALC_INFO_USIM);
	
	if(fileAddr == FALSE)
#endif
	{
		//modify by chentao@20211103 SE or EUICC 都是用4F07
		fileAddr = SelectFile(fileAddr5G, EF_SUCI_CALC_INFO);
	}
	
	if(fileAddr != FALSE)
	{		
		ReadTFBuf(fileAddr,0x00,&tempBuf[0],0x02);//Protection Scheme Identifier List
	}	
	
	flag = 0x00;
	
	A0Len = 0x00;
	
	A1Len = 0x00;
	
	A1TagLen=0;
	
	datalen = 0x00;
	
	if(tempBuf[0] == 0xA0)
	{
		A0Len = tempBuf[1];	
		
		if(A0Len==0x00)
		{
			mode = NULL_SCHEME;

		}
		else if(A0Len%2 != 0)
		{
						/*电信脚本测试*/
			ConditionSatisfied();											//Send 6985

			return;
		}
		else
		{
			for(priority = 0x01;priority<=A0Len/2;priority++)
			{
				offset = 0x00;
				
				ReadTFBuf(fileAddr,offset+2*priority,&tempBuf[0],0x02);//优先级 
		
				schemeID = tempBuf[0];			
				
				keyIndex = tempBuf[1];
				
				if((schemeID == 0x00) || (keyIndex == 0x00))
				{
					mode = NULL_SCHEME;
					
					break;
				}
				else if(schemeID == 0x01)
				{
					mode = PROFILE_A;
				}
				else if(schemeID == 0x02)
				{
					mode = PROFILE_B;
				}
				else //RFU 卡片不支持，目前暂抛6985
				{
					ConditionSatisfied();											//Send 6985
					return;
				}		
					
				offset = A0Len+2;
				
				ReadTFBuf(fileAddr,offset,&tempBuf[0],0x04);//Home Network Public Key List TAG A1

				A1TagLen=0;				
				
				if(tempBuf[0] != 0xA1)
				{					
					A1Len=0;
					
					mode = NULL_SCHEME;
					
					break;
				}
				else
				{
					A1Len = GetBerTlvLength(&tempBuf[1],(WORD *)&A1TagLen);
										
					if(A1Len == 0x00)
					{
						mode = NULL_SCHEME;
						break;
					}
					
					offset = offset + A1TagLen;
				}
																
				datalen = 0x00;

				for(i = 0x01; i <= keyIndex; i++)
				{
					ReadTFBuf(fileAddr,offset,&tempBuf[0],0x05);//获取TAG 80 81			
					
					datalen = datalen + 5 + tempBuf[4];
					
					if(A1Len >= datalen)
					{
						if(i == keyIndex)
						{
							identifier = tempBuf[2];

							datalen = tempBuf[4];
							
							if(datalen!=0x00)
							{
								ReadTFBuf(fileAddr,offset+5 ,&tempBuf[0],datalen);// Home Network Public Key

								flag = 0x01;
							}
						}
						else
						{
							offset = offset + 5 +tempBuf[4]; 
						}					
					}
					else
					{
						break;
					}			
				}
				if(flag == 0x01)
				{
					break;
				} 					
			}
			if(flag != 0x01)
			{
				mode = NULL_SCHEME;
			}	
		}		
	}
	else
	{
// 		if((tempBuf[0] == 0x00)|| (tempBuf[0] == 0xFF))//文件值为全零或全FF时，默认使用null_scheme？还是抛错？
// 		{
// 			mode = NULL_SCHEME;
// 		}
		/*电信脚本测试, 无A0tag，期望6985*/
		ConditionSatisfied();											//Send 6985

		return;
	}
	
	//fileAddr = SelectTFFile( G_CurrentAdfName[G_ChannelNum], EF_UST);
	
	//ReadTFBuf(fileAddr, 0x10, &result, sizeof(BYTE));
	
	dataMode = 0x00;

	if ((resultBuf[1]&0x02) == 0x02)
	{		
		dataMode = NAI_MODE;
	}
	else
	{
		dataMode = IMSI_MODE;
	}	
			
	if(dataMode==NAI_MODE)
	{
#define NAI   RecData+100
		NaiLen=getNSI(fileAddr5G, NAI, PROFILE_B);
		
		if(NaiLen==FALSE)
		{
			ConditionSatisfied();
			return;
			//dataMode =NAI_MODE;
		}
		if ((mode == PROFILE_B)||(mode == PROFILE_A))
		{
			 ECCProcess(mode, &tempBuf[0],datalen);
			
			NaiLen=getNSI(fileAddr5G, NAI, PROFILE_B);
			 
			datalen = AES_CTR(&tempBuf[0x40], 0x10, &tempBuf[0x40+0x10], 0x10, NAI, NaiLen, &tempBuf[0x90]);
#undef	NAI																	
			//HMACSha256DoFinalSingleCall(&tempBuf[0x40+0x20],0x20, &tempBuf[0x90], datalen, &tempBuf[0x190]); 
			GeneralHMAC(&tempBuf[0x40+0x20],0x20, &tempBuf[0x90], datalen, &tempBuf[0x190],SHA256_DIGEST_SIZE,ALG_HMAC_SHA_256);							
		 }
		 else //profile A
		 {


		 }

	}
	else
	{
		MSINLen=getImsi(fileAddrDfUsim,(BYTE *)&IMSI[0]);
		
		if(MSINLen == FALSE)
		{
			ConditionSatisfied();
			return;
		}
		
		if ((mode == PROFILE_B)||(mode == PROFILE_A))
		{
			ECCProcess(mode, &tempBuf[0],datalen);
			 
			datalen = AES_CTR(&tempBuf[0x40], 0x10, &tempBuf[0x40+0x10], 0x10, &IMSI[3], MSINLen, &tempBuf[0x90]);
			
			//HMACSha256DoFinalSingleCall(&tempBuf[0x40+0x20],0x20, &tempBuf[0x90], datalen, &tempBuf[0x190]); 	
			GeneralHMAC(&tempBuf[0x40+0x20],0x20, &tempBuf[0x90], datalen, &tempBuf[0x190],SHA256_DIGEST_SIZE,ALG_HMAC_SHA_256);		
		 }
		 else //profile A
		 {
			 
			 
		 }
	}
	
	
	if(dataMode == IMSI_MODE)
	{		
	
	    RecData[0] = 0xA1; //TAG
		
		RecData[2] =  0x01;//TYPE SUCI
		
		offset=2+1;
		
		ecpMem_Copy(&RecData[offset],&IMSI[0],3);
	
		offset=offset+3;
		
		result = getRoutingID(fileAddr5G,dataMode,(BYTE* )&ROUTINGID[0]);
		
		if(result == TRUE)
		{
			ecpMem_Copy(&RecData[offset],&ROUTINGID[0],0x02);
		}
		else
		{
			return;
		}		
	 
		 offset=offset+2;	
		 
		 if(mode == NULL_SCHEME)
		 {
			 	RecData[offset] = 0x00;//schemeID
			 
			  offset=offset+1;

				RecData[offset] = 0x00;//Home Network public key Identifier
			 
			  offset=offset+1;
			 
			  ecpMem_Copy(&RecData[offset],&IMSI[3],MSINLen);
		
			  offset = offset + MSINLen;		
			 
			  RecData[0x01] = offset-2;
		 }
		 else
		 {
				RecData[offset] = schemeID;
			 
				offset=offset+1;

				RecData[offset] = identifier;
			 
				offset=offset+1;
			 
				if (mode == PROFILE_B)
				{
					ecpMem_Copy(&RecData[offset],&tempBuf[0], SHAREINFO_LEN);//ECCKeyPair 公钥
					offset = offset + SHAREINFO_LEN;
				}
				else if (mode == PROFILE_A) //23003-f60定义
				{	
					ecpMem_Copy(&RecData[offset],&tempBuf[0], SHAREINFO_LEN-1);//ECCKeyPair 公钥
					offset = offset + SHAREINFO_LEN-1;
				}
				
				ecpMem_Copy(&RecData[offset],&tempBuf[0x90], datalen); //加密后的数据
				
				offset = offset + datalen;
				
				ecpMem_Copy(&RecData[offset],&tempBuf[0x190], 0x08); //mac
				
				offset = offset + 0x08;
				 
				RecData[0x01] = offset-2;		 
		 }
		 offset = offset-2;
	}
	else if(dataMode==NAI_MODE)//NAI模式
	{
		if(mode == PROFILE_A)
		{
			ecpMem_Copy(&RecData[0],&tempBuf[0],SHAREINFO_LEN-1);			
		}
		else
		{
			ecpMem_Copy(&RecData[0],&tempBuf[0],SHAREINFO_LEN);			
			
		}
		
		ecpMem_Copy(&RecData[SHAREINFO_LEN],&tempBuf[0x90],datalen);//公钥
		
		ecpMem_Copy(&RecData[240],&tempBuf[0x190],0x08);//mac
		
		tempBuf[0] = 0xA1; //TAG
		
		tempBuf[2]=0x11;
		
		NAIOffset=2+1;
		
		ecpMem_Copy(&tempBuf[NAIOffset], (BYTE *)type, sizeof(type));				//type
		
		NAIOffset = NAIOffset+sizeof(type);
		
		tempBuf[NAIOffset]= 0x31;//TYPE NSI
		
		NAIOffset= NAIOffset+1;
		
		ecpMem_Copy(&tempBuf[NAIOffset], (BYTE *)rid, sizeof(rid));				//rid
		
		NAIOffset = NAIOffset + sizeof(rid);
		
		result = getRoutingID(fileAddr5G,dataMode,(BYTE* )&ROUTINGID[0]);
		
		if(result == TRUE)
		{
			ecpMem_Copy(&tempBuf[NAIOffset],&ROUTINGID[1],ROUTINGID[0]);
		}
		else
		{
			return;
		}

		
		NAIOffset=NAIOffset+ROUTINGID[0];
		
		ecpMem_Copy(&tempBuf[NAIOffset], (BYTE *)schid, sizeof(schid));
			
		NAIOffset = NAIOffset + sizeof(schid);
				
		if(mode == NULL_SCHEME)
		{
			
			tempBuf[NAIOffset] = 0x30;//schemeID 值为0,将其直接转换为ASCii			
						
			NAIOffset=NAIOffset+1;						
			
			ecpMem_Copy(&tempBuf[NAIOffset], (BYTE *)userid, sizeof(userid));
			
			NAIOffset = NAIOffset + sizeof(userid);
			
#define NAI   RecData+100

			NaiLen=getNSI(fileAddr5G, NAI, NULL_SCHEME);
			
			ecpMem_Copy(&tempBuf[NAIOffset],NAI,NaiLen);

#undef	NAI		
			NAIOffset = NAIOffset + NaiLen;	
			
			NAIOffset = NAIOffset-2;
			
			if(NAIOffset<0x80) //一个字节长度
			{
				tempBuf[0x01] = NAIOffset;

			}
			else
			{
				ecpMem_Move(&tempBuf[3],&tempBuf[2],NAIOffset);
				
				tempBuf[0x01] = 0x81;
					
				tempBuf[0x02]=NAIOffset;
					
				NAIOffset=NAIOffset+1;//长度多一个字节
					
			}
			offset = NAIOffset;
			
			ecpMem_Copy(&RecData[0],&tempBuf[0],offset+2);
		}
		else
		{			
			
				SixteenToTen(schemeID,&tempBuf[NAIOffset]);			
				
				len = tempBuf[NAIOffset];							
			
				ecpMem_Move(&tempBuf[NAIOffset],&tempBuf[NAIOffset+1],len);
			
				NAIOffset=NAIOffset+len;
			
				ecpMem_Copy(&tempBuf[NAIOffset], (BYTE *)hnkey, sizeof(hnkey));
			
				NAIOffset = NAIOffset + sizeof(hnkey);								

				SixteenToTen(identifier,&tempBuf[NAIOffset]);								
				
				len =tempBuf[NAIOffset];
				
				ecpMem_Move(&tempBuf[NAIOffset],&tempBuf[NAIOffset+1],len);
				
				NAIOffset = NAIOffset +len;
			
				ecpMem_Copy(&tempBuf[NAIOffset], (BYTE *)ecckey, sizeof(ecckey));
			
				NAIOffset = NAIOffset + sizeof(ecckey);
				
				if (mode == PROFILE_B)
				{
					
					HextoASC(SHAREINFO_LEN, &RecData[0],&tempBuf[NAIOffset]);
					
					NAIOffset = NAIOffset + SHAREINFO_LEN*2;
				}
				else if (mode == PROFILE_A) //23003-f60定义
				{	
					
					HextoASC(SHAREINFO_LEN-1, &RecData[0],&tempBuf[NAIOffset]);
					
					NAIOffset = NAIOffset + (SHAREINFO_LEN-1)*2;
				}
				
				ecpMem_Copy(&tempBuf[NAIOffset], (BYTE *)cip, sizeof(cip));
				
				NAIOffset = NAIOffset + sizeof(cip);

				HextoASC(datalen, &RecData[SHAREINFO_LEN],&tempBuf[NAIOffset]);
				
				NAIOffset = NAIOffset + datalen*2;
				
				ecpMem_Copy(&tempBuf[NAIOffset], (BYTE *)mac, sizeof(mac));
				
				NAIOffset = NAIOffset + sizeof(mac);
				
				//ecpMem_Copy(&RecData[3+offset],&tempBuf[0x190], 0x08); //mac
				HextoASC(0x08, &RecData[240],&tempBuf[NAIOffset]);
				
				NAIOffset = NAIOffset + 0x08*2;
				
#define	NAI   RecData
#define DOMAINBUF	NAI+110
				NaiLen=getNSI(fileAddr5G, NAI, DOMAIN);
				
				ecpMem_Copy(&tempBuf[NAIOffset],DOMAINBUF, NaiLen); //domain name
#undef	NAI
#undef	DOMAINBUF
				
				NAIOffset = NAIOffset + NaiLen;
				
				NAIOffset = NAIOffset -2;
				
				if(NAIOffset<0x80) //一个字节长度
				{
					tempBuf[0x01] = NAIOffset;

					ecpMem_Copy(&RecData[0],&tempBuf[0],NAIOffset+2);

				}
				else if(NAIOffset<0xFE)
				{					
					ecpMem_Move(&tempBuf[3],&tempBuf[2],NAIOffset);
					
					tempBuf[0x01] = 0x81;
					
					tempBuf[0x02]=NAIOffset;
					
					NAIOffset=NAIOffset+1;//长度多一个字节
					
					ecpMem_Copy(&RecData[0],&tempBuf[0],NAIOffset+2);
					
				}
				else //0x82
				{
					ecpMem_Move(&tempBuf[4],&tempBuf[2],NAIOffset);
					
					tempBuf[0x01] = 0x82;
					
					tempBuf[0x02] = (NAIOffset>>8)&0xFF;
					
					tempBuf[0x03] = NAIOffset&0xFF;
					
					NAIOffset=NAIOffset+2;
					
					ecpMem_Copy(&RecData[0],&tempBuf[0],256);
					
					NAIOffset = NAIOffset+2 - 256;
					
					lenTag = 1;
				}
				offset = NAIOffset;
			}
	}
	else 
	{
	}
	
	if(lenTag == 0)//发送的数据长度没有超过0x100
	{
		if((P3 == 0x00) || (P3 > (offset+2)))
		{
			//by xwh 20220107,优化鉴权性能，0078000100直接调用底层发送函数，在底层处理下一条00780001XX指令
			if(P3 == 0x00)
			{
				apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, offset+2);
				GlobalSetStatus(0x9000);
				return;
			}
			
			RepeatPreviousCmdHeader(offset+2); //发送状态0x6CXX，XX表示总的数据长度

			tempBuf[0] = offset+2;
			ecpMem_Copy(&tempBuf[1],RecData,offset+2);
			WriteVariable(Tag_BakSupiData,&tempBuf[0]);//将数据写到变量里面，等下次发送相同的指令时，读出变量的值，发送XX的数据
		}
		else//P3 <= (offset+2)
		{
			SendDataSingle();//发送p3长度的数据

			ecpMem_Move(&RecData[0],&RecData[P3],offset+2-P3);//将p3后面的数据（剩余数据）移到RecData位置

			apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, offset+2-P3);

			if(offset+2-P3 == 0)//电信测试/5-2_SUCI_Test/6.TC_Error_P3_6XXX
			{
				SW1 = 0x90;


			}
			else
			{
				SW1 = 0x61;       //状态字改为61xx，xx表示剩余的数据长度

			}

			SW2 = offset+2-P3;

			return;
		}
				

	}
	else//发送的数据长度超过0x100
	{
		
		if(P3 == 0x00)
		{
			SendDataSingle();//先发送0x100的数据

			ecpMem_Copy(&RecData[0],&tempBuf[256],offset);//将剩余的数据拷贝到RecData[0]
								
			apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, offset);
			
			SW1 = 0x61;//发送状态字61XX，XX表示剩余的数据长度

			SW2 = offset;

			return;
			
		}
		else
		{
			SendDataSingle();//先发送P3的数据
			
			ecpMem_Move(&RecData[0],&RecData[P3],256-P3);//这里处理的数据长度在第一次发送数据之后，剩余的数据长度小于0x100，大于的目前不支持
			
			ecpMem_Copy(&RecData[256-P3],&tempBuf[256],offset);
			
			apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, offset+256-P3);

			SW1 = 0x61;

			SW2 = offset+256-P3;

			return;			
		}
		

	}
			
	
	G_SUPILEN=offset+2;
	
	return;

}

#endif




