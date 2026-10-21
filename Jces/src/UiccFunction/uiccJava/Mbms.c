#include	"string.h"
#include 	"uicc_main.h"
//#include 	"main.h"
#include 	"fileSystem.h"
#include 	"fileAction.h"
#include 	"gsmpublic.h"
//#include 	"resume.h"
#include 	"Authentication.h"
#include    "stkhead.h"
#include 	<string.h>			 
#include <uicc_error.h>
#include "Mbms.h"
#include "Uicc_interface.h"
#include "Crypto_API.h"


#if UICC__AUTH__MBMS
//mikey in mskupdate: HDR(12bytes)+EXT(5bytes)+TS(4bytes)+MIKEY RAND(16bytes)+MUK ID(4bytes)+KEMAC(nbytes)

//const BYTE nullMsk[EFMSK_RECLEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xff, 0xff, 0xff, 0xff};
//const BYTE nullMskKv[Ef_MSK_KV_RECLEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};
const BYTE ConstantEncr[5] = {0x15, 0x05, 0x33, 0xE1, 0xFF}; 
const BYTE ConstantAuth[5] = {0x2D, 0x22, 0xAC, 0x75, 0xFF};
const BYTE ConstantSalt[5] = {0x29, 0xB8, 0x89, 0x16, 0xFF};

const BYTE para_T = 5; //默认值
BYTE xdata G_MUK_IDX;

BYTE  findPayload(BYTE * dataBuf, BYTE mikeyLen, BYTE payloadId, BYTE appearence)
{
	BYTE idata currentPayloadId,nextPayloadId;
	BYTE idata i;
	//BYTE idata buf[5];

	if((appearence == 0)||(mikeyLen<3))
	{
		return FALSE;	
	}	
	
	currentPayloadId = dataBuf[2]; //从head中取出next payload id

	for(i=10; i<mikeyLen;) //从head下一个开始查找
	{
		nextPayloadId = dataBuf[i]; //备份next payload id		
		
		if( currentPayloadId == payloadId) 
		{
			if(appearence == 1)
			{
				return i;
			}
			appearence--;
		}		

		switch(currentPayloadId){ 
		case PAYLOAD_Last: 
			return FALSE;

		case PAYLOAD_KEMAC: 
			i += (4 + (dataBuf[i+2]<<8) + dataBuf[i+3]);
			
			if(i >= mikeyLen)
			{
				return FALSE;
			}
			
			i += (dataBuf[i] ? 21 : 1);
			break;

		case PAYLOAD_TS: 
			i += 6;
			break;

		case PAYLOAD_ID: 
			i += (4 + (dataBuf[i+2]<<8) + dataBuf[i+3]);
			break; 

		case PAYLOAD_SP: 
			i += (4 + (dataBuf[i+3]<<8) + dataBuf[i+4]);
			break; 

		case PAYLOAD_RAND: 
			i += (2 + dataBuf[i+1]);
			break; 

		case PAYLOAD_EXT: 
			i += (4 + (dataBuf[i+2]<<8) + dataBuf[i+3]);
			break;	

		default:
		   return FALSE;//todo:其他没有用到的payload处理
		}
	
		currentPayloadId = nextPayloadId;
	}

	return FALSE;
}

/*
   PRF(inkey, label) = P(s_1, label, m) XOR P(s_2, label, m) XOR ...
                       XOR P(s_n, label, m).
                       
   P (s, label, m) = HMAC (s, A_1 || label) ||
                     HMAC (s, A_2 || label) || ...
                     HMAC (s, A_m || label)
   where

      A_0 = label,
      A_i = HMAC (s, A_(i-1))
      s is a key (defined below)
      m is a positive integer (also defined below).
      
   *  let n = inkey_len / 256, rounded up to the nearest integer if not
      already an integer

   *   split the inkey into n blocks, inkey = s_1 || ... || s_n, where *
      all s_i, except possibly s_n, are 256 bits each

   *  let m = outkey_len / 160, rounded up to the nearest integer if not
      already an integer

	本业务对于业务密钥消息，Inkey为256位（Ks_int_NAF）,
	对于节目流密钥消息，Inkey为128位（MSK）。 
	outkey根据密钥类型有不同的长度，加密密钥(encr_key)为128位，
	完整性认证密钥(auth_key)为160位，Salt key为112位。
	
	故PRF(inkey, label)可简化为如下函数：
	PRF(inkey, label) = HMAC(inkey，A_1||label)
	其中：
	A_1 =HMAC(inkey，label) 
	label=  constant || 0xFF || csb_id || RAND
             
*/	
#define LABEL_LEN 25 //
#define HMAC_LEN 20
void prf_for_mikey(BYTE inkey[], BYTE inkeyLen, BYTE outkey[], BYTE outkeyLen, BYTE * csbid, BYTE rand, BYTE randLen, BYTE type)
{
	const u8 * constant;
	//u8 *dataOut = G_tempBuf + 0x80;
	//u8 * _p = G_tempBuf;
	BYTE idata off;
	
	//A_1 =HMAC(inkey，label)
	if(type==ENCR_KEY)
	{
		constant = ConstantEncr;
	}
	else if(type==AUTH_KEY)
	{
		constant = ConstantAuth;
	}
	else
	{
		constant = ConstantSalt;
	}

	off = 0x00;
	ecpMem_Copy(&tempBuf[off], (BYTE *)constant, 5);
	off+=5;

	ecpMem_Copy(&tempBuf[off],csbid, 4);
	off+=4;

	ecpMem_Copy(&tempBuf[off],&csbid[rand], randLen);
	off+=randLen;

	//HMACSha1DoFinalSingleCall(inkey, inkeyLen, tempBuf, off, &tempBuf[0x80]);
	GeneralHMAC(inkey, inkeyLen, tempBuf, off, &tempBuf[0x80], SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);

	off = 0x80+20;
	ecpMem_Copy(&tempBuf[off], (BYTE *)constant, 5);
	off+=5;

	ecpMem_Copy(&tempBuf[off],csbid, 4);
	off+=4;

	ecpMem_Copy(&tempBuf[off],&csbid[rand], randLen);
	off+=randLen;

	//HMACSha1DoFinalSingleCall(inkey, inkeyLen, &tempBuf[0x80], (off-0x80), outkey);
	GeneralHMAC(inkey, inkeyLen, &tempBuf[0x80], (off-0x80), outkey, SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);
}


/*
	kemac_payload的完整性校验及解密
*/

BYTE kemac_payload_process(BYTE * dataBuf, BYTE mikeyLen, BYTE kemac, BYTE rand, BYTE ts, BYTE preSharedKey[], BYTE keyLen)
{
	BYTE idata randLen;
	BYTE idata Bufa[20], Bufb[20], dataOut[16];
	BYTE idata i,j,counter;
		
	if(rand == 0x00)
	{
		randLen = 0; //MTK MIKEY消息中不含RAND
	}
	else
	{
		randLen = dataBuf[rand + 1];
	}

	//if(mikey[mikeyLen-21] == 1) //需要完整性校验

		//Bufa[]为完整性校验的密钥			
	
	prf_for_mikey(preSharedKey, keyLen, Bufa, 20, &dataBuf[4], rand+2-4, randLen, AUTH_KEY); 
		
	if(mikeyLen < 20)
	{
		ReferencedDataInvalid();
		return 3;			
	}

		//Bufb[]为校验值
		//hmac_sha1 (Bufa, 20, mikey, mikeyLen-20, Bufb); 
	//HMACSha1DoFinalSingleCall(Bufa, 20, dataBuf, mikeyLen-20, Bufb);
	GeneralHMAC(Bufa, 20, dataBuf, mikeyLen-20, Bufb, SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);
	if(ecpMem_Compare(&dataBuf[mikeyLen-20], Bufb, 20) != EQUAL)
	{
		GlobalSetStatus(0x9862);
		
		return  3;
	}

	//ReadEeprom(&kemac, Kemac+1,0x01);
	if(dataBuf[kemac + 1] == 1) //AES-CM-128
	{
		//初始向量(16字节)
		//IV = (S XOR (0x0000 || CSB ID || T)) || 0x0000
		prf_for_mikey(preSharedKey, keyLen, Bufa, 14, &dataBuf[4], (rand+2-4), randLen, SALT_KEY); //S(salt)在Bufa(14字节)
		
		//异或CSB ID
		Bufa[2] ^= dataBuf[4]; 
		Bufa[3] ^= dataBuf[5];
		Bufa[4] ^= dataBuf[6];
		Bufa[5] ^= dataBuf[7];
		
		//rfc3830 4.2.3 T:=64bit-timestamp 
		Bufa[10] ^= dataBuf[ts+2];
		Bufa[11] ^= dataBuf[ts+3];
		Bufa[12] ^= dataBuf[ts+4];
		Bufa[13] ^= dataBuf[ts+5];
		
		Bufa[14] = 0;
		Bufa[15] = 0;
		
		//计算解密密钥(在Bufb)
		prf_for_mikey(preSharedKey, keyLen, Bufb, 16, &dataBuf[4], (rand+2-4), randLen, ENCR_KEY); 
		
		counter = dataBuf[kemac + 3]/16 ;
		
		//AES_Nk = 4;           //AES 全局变量初始化   默认密钥长度为16字节  AES使用前需要进行密钥扩展
		//AES_Nr = 10;         //密钥扩展前key_expansion 要对这两个全局变量初始化

		//key_expansion(&Bufb[0], &tempBuf[80]);  
//		KeySchedule(&Bufb[0]);	//利用Ki进行AES Key的初始化
		G_AESKeyByteLen = 16;
		KeyExpansion(&Bufb[0],roundKeys);	//利用Ki进行AES Key的初始化
		
		for(i = 0;i < counter;i++)
		{
			//AES 输入参数会被冲掉  下面还有在用  需备份
			ecpMem_Copy(dataOut, Bufa, 16);
//			AESEncrypt(dataOut,dataOut);
			AESEncryptForUICC(dataOut,dataOut,roundKeys);

			for(j = 0;j<16;j++)
			{
				dataBuf[kemac+4+j+(16*i)] ^= dataOut[j] ;
			}
			Bufa[15] ++;
		}
		counter = (BYTE)(dataBuf[kemac + 3]%16);
		if(counter != 0)
		{
//			AESEncrypt(Bufa,dataOut);
			AESEncryptForUICC(Bufa,dataOut,roundKeys);

			for(j = 0;j<counter;j++)
			{
				dataBuf[kemac+4+j+(16*i)] ^= dataOut[j] ;
			}
		}

	}
	else if(kemac == 2) //AES-KW-128
	{
		//todo:
	}
	
	return TRUE;
}

/*
 *  modify by chentao@20211012 修改UICC__AUTH__MBMS 宏打开的编译错误
 *  modify by chentao@20211111 GBA鉴权参数ISIM和USIM分开存储（YJR确认）
 *  modify by zhxx@20220424 最后一次MRK请求EF_GBANL被更新的记录号是G_MUK_IDX，读取EFgbanlAddr文件时应该使用这条记录，D:\TestSuite\EPX-测试列表\eUICC平台基本通信功能测试工程集\01_基本通信功能测试_01\MBMS测试脚本\MSK\6.3.28测试修改
 */

BYTE MikeyMskParse(BYTE * dataBuf, BYTE dataLen)
{
	BYTE idata idi,idr,kemac,ext,ts,rand,payload,keyDomainId,mskId,cfgOff;
	UWORD idata EFgbanlAddr,EFmukAddr,EFmskAddr,EF_ks_int_nafAddr,EF_succ_used_mukAddr,EFmskkvAddr;
	BYTE idata tsbak[4];
	BYTE idata mukIdx,i,EFmsklRecNum,EFmukRecNum,EFmukRecLen,gbaIdx;
	BYTE idata tester,currRecNum,result,offset,reslen,resmacoff;
	UWORD idata MikeyTs,UamTs;
	UWORD xdata fileAddr, fileAddr1;

	fileAddr = FALSE;

	fileAddr1 = FALSE;

#if UICC__ISIM
	if(G_CurrentAdfName[G_ChannelNum] == G_DF_ID_ISIM)
	{

		fileAddr = SelectTFFile(G_DF_ID_ISIM, EF_GBANL);

		fileAddr1 = SelectTFFile(G_DF_ID_ISIM, EF_GBABP);

		cfgOff = 0;

	}
	else
#endif
	{
		fileAddr = SelectTFFile(G_DF_ID_USIM, EF_GBANL_USIM);

		fileAddr1 = SelectTFFile(G_DF_ID_USIM, EF_GBABP_USIM);
		cfgOff = 1;
	}
	
	EF_ks_int_nafAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum], EF_GBA_KS);

	if ((fileAddr == FALSE)||(fileAddr1 == FALSE))
	{
		return FALSE;
	}
		
	ecpMem_Set(tsbak,0,4);
	/*-0-判断是否为MSK更新消息*/
	idi = findPayload(dataBuf, dataLen, PAYLOAD_ID, 1); //定位IDi
	idr = findPayload(dataBuf, dataLen, PAYLOAD_ID, 2); //定位IDr
	kemac = findPayload(dataBuf, dataLen, PAYLOAD_KEMAC, 1);
	ext = findPayload(dataBuf, dataLen, PAYLOAD_EXT, 1); //定位EXT payload
	ts = findPayload(dataBuf, dataLen, PAYLOAD_TS, 1);
	rand = findPayload(dataBuf, dataLen, PAYLOAD_RAND, 1);

	if ( (!idi)||(!idr)||(!kemac)||(!ext)||(!ts)||(!rand) ) 
	{
		IncorrentParameter();
		return 3; 		
	}
	currRecNum = 0;
	
	EFgbanlAddr = SelectLFFile(G_DF_ID_USIM, EF_GBANL_USIM);
	EFmukAddr = SelectLFFile(G_DF_ID_USIM, EF_MUK_USIM);
	EFmskAddr = SelectLFFile(G_DF_ID_USIM, EF_MSK_USIM);
	
//	EF_ks_int_nafAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum], EF_GBA_KS);
	EF_succ_used_mukAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum], EF_MUK_SUCUSED_USIM);
	EFmskkvAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum], EF_MSK_KV_USIM);
	
	
	EFmsklRecNum = GetLFRecNumLgth(EFmskAddr, F_RecNum_Offset);

	EFmukRecNum = GetLFRecNumLgth(EFmukAddr, F_RecNum_Offset);
	
	EFmukRecLen = GetLFRecNumLgth(EFmukAddr, F_RecLgth_Offset);	
		
		
	if( (dataBuf[kemac+2] == 0) && (dataBuf[kemac+3] == 0) ) 
	{
		GlobalSetStatus(0x6A88);
		
		return 3; 
	}
	
	payload = ext+ 4; //_payload指向Key Domain ID sub-payload
	
	if( dataBuf[payload+1] != 3 ) //检查Key Domain ID Length
	{
		FunctionNotSupport();
		return 3; 
	} 

	keyDomainId = payload+2; //标记Key Domain ID

	payload +=  dataBuf[payload+1] + 2;	
	
	if( dataBuf[payload + 1] != 4 ) //检查MSK ID Length
	{
		FunctionNotSupport();
		return 3; 
	} 
	mskId = payload+ 2; //标记MSK ID
	
	mukIdx = G_MUK_IDX;
	
	gbaIdx = ReadEECacheByte(E_NAFID_OLD_RECORD + cfgOff);//最后一次MRK请求EF_GBANL被更新的记录号
	
	if(mukIdx == 0) //没有进行过MRK请求
	{
		GlobalSetStatus(0x6A88);
		
		return 3;			
	}
	
	i = FALSE;
	
		
	//'80' | NAF_ID_LEN | FQDN | UaSPI | '81' | B-TID_LEN | B-TID_ID
	//ReadRecFile(EFgbanlAddr, mukIdx, (u8)1, (u8 *)BUFFER256, (u8)33); //读出(NAF_ID_LEN | NAF_ID)
	ReadLFBuf(EFgbanlAddr, gbaIdx, 0x01 , &RecData[0], 33);//读出(NAF_ID_LEN | NAF_ID)		

	RecData[0] -= MBMSUaSPI_LEN; //FQDN_LEN = NAF_ID_LEN - MBMSUaSPI_LEN	
	
	if(ecpMem_Compare(&RecData[0], &dataBuf[idi+3], RecData[0]+1) == EQUAL)
	{
		ReadLFBuf(EFgbanlAddr, gbaIdx, (u8)(3+RecData[0]+MBMSUaSPI_LEN), &RecData[0], 65); //读出(B-TID_LEN | B-TID_ID)

		//比较(IDr_LEN | IDr) 和 (B-TID_LEN | B-TID_ID)
		if(ecpMem_Compare(&RecData[0], &dataBuf[idr+3], RecData[0]+1) == EQUAL)
		{
			i = TRUE;
			
		}
			
	}		
	
	
	if(i == FALSE) //"接收到的MUK ID与卡内上次产生的MUK不对应"
	{
		//与上次成功使用过的比较
		//'80' | NAF_ID_LEN | FQDN | UaSPI | '81' | B-TID_LEN | B-TID_ID
		ReadTFBuf(EF_succ_used_mukAddr, 0x01, RecData, 33); //读出(NAF_ID_LEN | NAF_ID)

		RecData[0] -= MBMSUaSPI_LEN; //FQDN_LEN = NAF_ID_LEN - MBMSUaSPI_LEN

		//比较(IDi_LEN | IDi) 和 (FQDN_LEN | FQDN) 
		if( ecpMem_Compare(RecData, &dataBuf[idi+3], RecData[0]+1) == EQUAL )
		{
			ReadTFBuf(EF_succ_used_mukAddr, 3+RecData[0]+MBMSUaSPI_LEN, RecData, 65); //读出(B-TID_LEN | B-TID_ID)		

			//比较(IDr_LEN | IDr) 和 (B-TID_LEN | B-TID_ID)
			if( ecpMem_Compare(RecData, &dataBuf[idr+3], RecData[0]+1) == EQUAL )
			{
				ReadTFBuf(EF_succ_used_mukAddr, 100, RecData, 32); //
				
				result = kemac_payload_process(dataBuf, dataLen, kemac, rand, ts, RecData, 32);
				
				if((SW1 == 0x98) &&(SW2 == 0x62))
				{
					return 3; //mac不正确
				}
				else
				{
					GlobalSetStatus(0x9865);	//密钥过期
					
					return 3; 
				}
			}
		}

		GlobalSetStatus(0x6A88);//与上次成功使用过的不一致
		
		return 3;
	}
	else //"接收到的MUK ID与卡内上次产生的MUK对应"
	{				
		/*发生了新的NAF分散过程, 更新EF_MUK*/
		for(i=1; i<=EFmukRecNum; i++)
		{
			//MukIdTag | MukIdLen | IdrTag | IDrLen | IDr | IdiTag | IDiLen | IDi | TsTag(1) | TsLen(1) | Ts(4)
			ReadLFBuf(EFmukAddr, i, 0, RecData, EFmukRecLen);

			if( (RecData[0] != 0xA0) || ((RecData[1]+2+6)>EFmukRecLen) ) //MukIdTag | MukIdLen 
			{
				continue;
			}

			if((offset = FindTag(&RecData[2], 0x82, RecData[1])) == FALSE) //IdiTag
			{
				continue;
			}
			
			if(ecpMem_Compare(&RecData[2+offset], &dataBuf[idi+3], dataBuf[idi+3]+1) == EQUAL) //比较IDiLen | IDi
			{
				//"IDi（包含在MUK ID中）对应的记录存在" (增加对TS的判断)				
				
				MikeyTs = BuildUWord(&dataBuf[ts+2]);
				UamTs = BuildUWord(&RecData[RecData[1]+4]);

				if(MikeyTs <= UamTs) 
				{
					if((UamTs-MikeyTs) >= para_T) //差值大于等于5时,不接受消息
					{
						G_ResponseLen = 0;
						return TRUE;						
					}

					ecpMem_Copy(tsbak,&RecData[RecData[1]+4],4); //TS保持不变
				}
				else
				{
					ecpMem_Copy(tsbak,&dataBuf[ts+2],4); //TS需要更新为MIKEY消息包中的TS
				}
				
				break;
			}
		}


		if(i>EFmukRecNum) //没有找到匹配的记录
		{
			ecpMem_Copy(tsbak,&dataBuf[ts+2],sizeof(tsbak)); //TS需要更新为MIKEY消息包中的TS
					
			i = G_MUK_IDX;
			
			if ((G_MUK_IDX+1) > GetLFRecNumLgth(EFgbanlAddr,FileNum))				//计算最早更新的记录号
			{
				G_MUK_IDX = 0x01;
			}
			else
			{
				G_MUK_IDX ++;
			}								
					
		}

		
		offset = 0;
		RecData[offset++] = 0xA0; //'MukIdTag'
		
		RecData[offset++] = 2 + dataBuf[idr+3] + 2 + dataBuf[idi+3];
		
		RecData[offset++] = 0x80; //'IdrTag'
		
		ecpMem_Copy(&RecData[offset],&dataBuf[idr+3],dataBuf[idr+3]+1);
		//memCopyNoAtomic((u8 *)&BUFFER256[offset],(u8 *)(_idr+3),(u8)(_idr[3]+1)); //IDrLen | IDr
		offset += dataBuf[idr+3]+1;

		RecData[offset++] = 0x82; //IdiTag 
		ecpMem_Copy(&RecData[offset],&dataBuf[idi+3],dataBuf[idi+3]+1);
		
		//memCopyNoAtomic((u8 *)&BUFFER256[offset],(u8 *)(_idi+3),(u8)(_idi[3]+1)); //IDiLen | IDi
		offset += dataBuf[idi+3]+1;
		
		RecData[offset++]=0x81;
		RecData[offset++]=0x04;
		ecpMem_Copy(&RecData[offset], tsbak,4);
		
		offset += 4;
		
		WriteLFBuf(EFmukAddr, i, 0x00, RecData, offset);
		
		ReadLFBuf(EF_ks_int_nafAddr, i, 0, RecData,32);//读取该MUK,处理kemac_payload		//此处读取第一条记录usim

		result = kemac_payload_process(dataBuf, dataLen, kemac, rand, ts, RecData, 32);

		if(result == 3)
		{
			return result;
		}		
	}	
	
	payload = kemac + 4; //_payload指向key data sub-payload明文 
	
	if( (dataBuf[payload +1] & 0x70) != 0x20 ) //检查Type字节
	{
		FunctionNotSupport();
		return 3;
	}	

	offset = 4 + (dataBuf[payload +2]<<8) + dataBuf[payload +3]; //offset指向KV data段
	
	if(dataBuf[payload + offset] != 4) //检查VF Length
	{
		FunctionNotSupport();
		return 3;
	}
	
	if(dataBuf[payload + offset + 5] != 4) //检查VT Length
	{
		FunctionNotSupport();
		return 3;
	}

	
	/*-3-根据ext payload中的key domain id和msk id更新EF_MSK, 同时保存MSK和KV到Ef_MSK_KV*/	
	
	
#define BUFFER32A   RecData
#define BUFFER32C  (BUFFER32A+64)
#define BUFFER32B  (BUFFER32C+64)
	
		//提取下发的MSK和KV
	payload = kemac + 6; //定位到key data subpayload的key data len
	
	if( (dataBuf[payload] != 0) && (dataBuf[payload+1] != MSKLEN) )
	{
		FunctionNotSupport();
		return 3;
	}

	payload += 2; //指向key data(MSK)
	ecpMem_Copy(&BUFFER32C[MSK1], &dataBuf[payload], MSKLEN);
	
	
	payload += MSKLEN+1; //指向VF
	//ReadEeprom(&BUFFER32C[VF1], kemac + offset, KVLEN);
	ecpMem_Copy(&BUFFER32C[VF1], &dataBuf[payload], KVLEN);

	payload += (KVLEN+1); //指向VT
	ecpMem_Copy(&BUFFER32C[VT1], &dataBuf[payload], KVLEN);

	BUFFER32C[CNTER1] = 0; //cnter清零
	
	for(i=1; i<=EFmsklRecNum; i++) //以key domain id和key group id为索引搜索EF_MSK
	{
		//KEY_DOMAIN_ID(3) | NUM(1) | MSK_ID_1(4) | TS_1(4) | MSK_ID_2(4) | TS_2(4)
		ReadLFBuf(EFmskAddr, i, 0, BUFFER32A, EFMSK_RECLEN);

		// "key domain id" 相同且" key group id" 相同
		if((ecpMem_Compare(&BUFFER32A[KEYDOM_ID], &dataBuf[keyDomainId], 3) == EQUAL) && (ecpMem_Compare(&BUFFER32A[MSK_ID_1], &dataBuf[mskId], 2) == EQUAL))
		{
			tester = 0;
			
			//MSK1(16) | VF1(4) | VT1(4) | CNTER1(1) | MSK2(16) | VF2(4) | VT2(4) | CNTER2(1)
			ReadLFBuf(EFmskkvAddr, i, 0, &BUFFER32B[128], Ef_MSK_KV_RECLEN);

			//第一个MSK ID的KEY Number与接收到的相同
			if(ecpMem_Compare(&BUFFER32A[MSK_ID_1 +2], &dataBuf[mskId+2], 2) == EQUAL)
			{
				//并且第一个KV终止时刻与接收到的相同
				if(ecpMem_Compare(&BUFFER32B[VT1], &BUFFER32C[VT1], KVLEN) == EQUAL)
				{
					//更新TS1
					WriteLFBuf(EFmskAddr, i, TS_1, &dataBuf[ts+2], TS_LEN);

					//更新第一个Msk,Kv,cnter
					WriteLFBuf(EFmskAddr, i, MSK1, BUFFER32C, Ef_MSK_KV_RECLEN/2);

					tester ++;
				}
			}

			//第二个MSK ID的KEY Number与接收到的相同
			if( (ecpMem_Compare(&BUFFER32A[MSK_ID_2 +2], &dataBuf[mskId+2], 2) == EQUAL) && (BUFFER32A[KEYNUM]==2) )
			{
				//并且KV终止时刻与接收到的相同
				if(ecpMem_Compare(&BUFFER32B[VT2], &BUFFER32C[VT1], KVLEN) == EQUAL)
				{
					//更新TS2
					
					WriteLFBuf(EFmskAddr, i, TS_2, &dataBuf[ts+2], TS_LEN);	

					//更新第二个Msk,Kv,cnter
					WriteLFBuf(EFmskkvAddr, i, MSK2, BUFFER32C, Ef_MSK_KV_RECLEN/2);

					tester ++;
				}
			}

			if(tester == 0)
			{
				//用MSK_ID_1 | TS_1 覆盖 MSK_ID_2 | TS_2
				WriteLFBuf(EFmskAddr, i, MSK_ID_2, &BUFFER32A[MSK_ID_1], MSKID_LEN + TS_LEN);
					
				//新的MSK ID被保存在第1个MSK ID的位置，其TS清零
				ecpMem_Copy(&BUFFER32A[KEYDOM_ID], &dataBuf[keyDomainId], KEYDOM_LEN);
																
				BUFFER32A[KEYNUM] = 2;
				
				ecpMem_Copy(&BUFFER32A[MSK_ID_1], &dataBuf[mskId], MSKID_LEN);
				
				ecpMem_Set(&BUFFER32A[TS_1], 0x00, TS_LEN);
				
				WriteLFBuf(EFmskAddr, i, 0, (  u8*)BUFFER32A, (u8)12);

				//用MSK_1 | KV_L_1 | KV_U_1 覆盖 MSK_2 | KV_L_2 | KV_U_2
				//ReadLFBuf(EFmskkvAddr, i, (u8)MSK1, (u8*)BUFFER32B, (u8)(Ef_MSK_KV_RECLEN/2));
				
				//WriteLFBuf(EFmskkvAddr, i, (u8)MSK2, (  u8*)BUFFER32B, (u8)(Ef_MSK_KV_RECLEN/2));
				
				ReadEEPromCache(tempBuf+0x80,(EFmskkvAddr+sizeof(RecordFileHead)+(i-1)*Ef_MSK_KV_RECLEN+MSK1), (u8)(Ef_MSK_KV_RECLEN/2));
				
				VMWriteBuf(EFmskkvAddr+sizeof(RecordFileHead)+(i-1)*Ef_MSK_KV_RECLEN+MSK2,tempBuf+0x80,(u8)(Ef_MSK_KV_RECLEN/2));

				//新的Msk,Kv被保存在第1个位置, cnter清零
				WriteLFBuf(EFmskkvAddr, i, (u8)MSK1, (u8*)BUFFER32C, (u8)(Ef_MSK_KV_RECLEN/2));

			}
		 break;
		}

		if((BUFFER32A[0]==0xff)  && (currRecNum==0))
		{
			currRecNum = i;
		}
	}
	
	if(i>EFmsklRecNum) //没有找到匹配的记录
	{
		if((currRecNum > EFmsklRecNum)||(currRecNum == 0)){			
			GlobalSetStatus(0x9866);
			
			return 3;
		}

		i= currRecNum;
		//使用一条空记录保存下发的key domain id, MSKID; TS_1清零
		ecpMem_Copy(&BUFFER32A[KEYDOM_ID], &dataBuf[keyDomainId], KEYDOM_LEN);
		
		BUFFER32A[KEYNUM] = 1;
		
		ecpMem_Copy(&BUFFER32A[MSK_ID_1], &dataBuf[mskId], MSKID_LEN);
		
		ecpMem_Set(&BUFFER32A[TS_1], 0x00, TS_LEN);
		
		WriteLFBuf(EFmskAddr, i, (u8)0, (  u8*)BUFFER32A, (u8)12);
		//保存新的Msk,Kv; cnter清零
		WriteLFBuf(EFmskkvAddr, i, (u8)MSK1, (  u8*)BUFFER32C, (u8)(Ef_MSK_KV_RECLEN/2));
	}

	//保存本次成功使用的muk和muk id
	ReadLFBuf(EFgbanlAddr, mukIdx, (u8)0, (u8 *)BUFFER32B, (u8)100);
	ReadLFBuf(EF_ks_int_nafAddr, mukIdx, 0, (u8 *)(BUFFER32B+100), (u8)32);
	
	WriteTFBuf(EF_succ_used_mukAddr, (u16)0, (u8 *)BUFFER32B, (u16)132);
	
#undef BUFFER32A
#undef BUFFER32C
#undef BUFFER32B
	
/*-4-拼装确认消息mikey(hdr,ts,idr,v)*/
	//AckMsgConstruct:	
	
#define BUFFER32   (tempBuf+200)
#define BUFFER20   (BUFFER32+32)	
	
	offset = 0;
	RecData[offset++] = 0x53; 
	RecData[offset++] = 0x01;
	RecData[offset++] = 0xDB;
	

	if((dataBuf[3]&0x80) == 0x80 ) //需要发送确认消息
	{
		RecData[offset++] = 0;

		ecpMem_Copy(&RecData[offset], &dataBuf[0], 10);
		
		RecData[offset+1] = 1; //data type: PSK ver msg
		RecData[offset+2] = PAYLOAD_TS; //next payload
		RecData[offset+3] = 0;
		offset += 10;

		//ts payload
		ecpMem_Copy(&RecData[offset], &dataBuf[ts], 6);
		RecData[offset] = PAYLOAD_ID; //next payload
		offset += 6;

		//IDr payload
		ecpMem_Copy(&RecData[offset], &dataBuf[idr], dataBuf[idr+3]+4);
		RecData[offset] = PAYLOAD_V; //next payload
		offset += (dataBuf[idr+3] +4);
		
		//v payload
		RecData[offset++] = PAYLOAD_Last; //next payload
		RecData[offset++] = 0x01; //Auth alg = HMAC-SHA-1-160

		RecData[3] = offset-4 +20; //mikey length
		resmacoff = offset;
		//确认消息到此为止, 以下为拼装MAC

		ecpMem_Copy(&RecData[offset], &dataBuf[idi+4], dataBuf[idi+3]);
		offset += dataBuf[idi+3];
		
		ecpMem_Copy(&RecData[offset], &dataBuf[idr+4], dataBuf[idr+3]);
		offset += dataBuf[idr+3];

		ecpMem_Copy(&RecData[offset], &dataBuf[ts+2], 4);
		offset += 4;

		ReadLFBuf(EF_ks_int_nafAddr, mukIdx, 0, BUFFER32, 32); //read MUK
		
		prf_for_mikey(BUFFER32, 32, BUFFER20, 20, &dataBuf[4], rand+2-4, dataBuf[rand+1], AUTH_KEY); 

		//HMACSha1DoFinalSingleCall(BUFFER20,20,&RecData[4],offset-4,&RecData[resmacoff]);
		GeneralHMAC(BUFFER20, 20, &RecData[4], offset-4, &RecData[resmacoff], SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);
		RecData[1] = RecData[3]+2;
		reslen = RecData[1]+2;

	}else{
		reslen = 0x03;
	}

	G_ResponseLen = reslen;

	return TRUE;

#undef BUFFER32
#undef BUFFER20
	
}

BYTE MikeyMtkParse(BYTE * dataBuf, BYTE dataLen)
{
	BYTE  idata  kemac,ext,ts,keyDomainId,mskId,mtkId;
	UWORD idata EFmskAddr,EFmskkvAddr;
	BYTE  idata EFmskRecNum,tester1,tester2,result,i;
	BYTE  idata payload;

	EFmskAddr = SelectLFFile(G_DF_ID_USIM, EF_MSK_USIM);	
	EFmskkvAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum], EF_MSK_KV_USIM);
	EFmskRecNum = GetLFRecNumLgth(EFmskAddr, F_RecNum_Offset);
		
	/*-1-判断是否为MTK更新消息*/	
	kemac = findPayload(dataBuf, dataLen, PAYLOAD_KEMAC, 1);
	ext = findPayload(dataBuf, dataLen, PAYLOAD_EXT, 1); //定位EXT payload
	ts = findPayload(dataBuf, dataLen, PAYLOAD_TS, 1);
	
	if ((!kemac)||(!ext)||(!ts)) 
	{
		IncorrentParameter();
		return 3; 		
	}
	
	/*-2-解析PAYLOAD_EXT*/
	payload = ext+ 4; //_payload指向Key Domain ID sub-payload
	
	if( dataBuf[payload+1] != 3 ) //检查Key Domain ID Length
	{
		FunctionNotSupport();
		return 3; 
	} 

	keyDomainId = payload+2; //标记Key Domain ID	

	payload += dataBuf[payload+1] + 2; //_payload指向MSK ID sub-payload 
	if(dataBuf[payload+1] != 4 ) //检查MSK ID Length
	{
		FunctionNotSupport();
		return 3; 
	} 
	mskId = payload+2; //标记MSK ID
	payload += dataBuf[payload+1] + 2; 
	
	if( dataBuf[payload+1] != 4 ) //检查MSK ID Length
	{
		FunctionNotSupport();
		return 3; 
	} 
	
	mtkId = payload +2;
	
	/*-3-查找对应的MSK,检查KV的有效性,并检查MTK ID的有效性*/
	
	tester1 = 1;
	
	for(i=1; i<=EFmskRecNum; i++) //搜索EF_MSK
	{

		tester1 = 1;
		tester2 = 1;
		//KEY_DOMAIN_ID(3) | NUM(1) | MSK_ID_1(4) | TS_1(4) | MSK_ID_2(4) | TS_2(4)
		ReadLFBuf(EFmskAddr, i, 0, tempBuf, EFMSK_RECLEN);
		
		//先比较KEY_DOMAIN_ID
		if(ecpMem_Compare(&dataBuf[keyDomainId], &tempBuf[KEYDOM_ID], KEYDOM_LEN) != EQUAL)  continue;

		if(ecpMem_Compare(&dataBuf[mskId], &tempBuf[MSK_ID_1], MSKID_LEN) != EQUAL)  tester1 --; //MSKID1不匹配

		if(ecpMem_Compare(&dataBuf[mskId], &tempBuf[MSK_ID_2], MSKID_LEN) != EQUAL)  tester2 --; //MSKID2不匹配

		if((!tester1) && (!tester2))  continue; //该记录没有匹配的KEY_DOMAIN及MSKID
		
		
		//MSK1(16) | VF1(4) | VT1(4) | CNTER1(1) | MSK2(16) | VF2(4) | VT2(4) | CNTER2(1)
		ReadLFBuf(EFmskkvAddr, i, 0, RecData, Ef_MSK_KV_RECLEN);

		if(tester1 && (memcmp_p(&RecData[VF1], &RecData[VT1], KVLEN) > EQUAL))  tester1 --; //第一个KV区间无效

		if(tester2 && (memcmp_p(&RecData[VF2], &RecData[VT2], KVLEN) > EQUAL))  tester2 --; //第二个KV区间无效

		if((!tester1) && (!tester2))
		{
			GlobalSetStatus(0x6985);//不存在一个有效的KV区间[SEQL>SEQU]
			
			return 3;		
		}
		
		//判断MTK ID有效性(VF <= MTK ID < VT为有效)
		if(tester1)
		{
			if( (memcmp_p(&RecData[VF1], &dataBuf[mtkId], KVLEN) > EQUAL) || (memcmp_p(&RecData[VT1], &dataBuf[mtkId], KVLEN) <= EQUAL) )
			{
					tester1 --;
			}
		}
		
		if(tester2)
		{
			if( (memcmp_p(&RecData[VF2], &dataBuf[mtkId], KVLEN) > EQUAL) || (memcmp_p(&RecData[VT2], &dataBuf[mtkId], KVLEN) <= EQUAL) )
			{
					tester2 --;
			}
		}
		
		if((!tester1) && (!tester2))
		{
			GlobalSetStatus(0x9865); //MTK ID在第两个区间都无效(密钥过期)
			return 3;
		}
		
		break;
	} 
	
	
	if(i > EFmskRecNum)
	{
		GlobalSetStatus(0x6A88);
		
		return 3; //没有找到密钥
	}

	/*-4-kemac_payload完整性校验解密*/
	result = kemac_payload_process(dataBuf, dataLen, kemac, 0, ts, &RecData[tester1?MSK1:MSK2], 16);
	
	if(result == 3)
	{
		return 3;
	}	
	
	//if((G_StatusWord != 0x9000)&&(G_StatusWord != 0x62f3)) return 0;
	
	
	
	

	/*-5-用MTK_ID替代SEQL(VF)*/
	//UpdateRecFile(EFmskkvAddr, i, (u8)(tester1?VF1:VF2), (  u8*)_MtkId, (u8)KVLEN);
	
	/*-6-更新EF_MSK的相应TS*/
	//UpdateRecFile(EFmskAddr, i, (u8)(tester1?TS_1:TS_2), (  u8*)_ts+2, (u8)4);
	
	/*-7-输出MTK*/
	//_payload = _kemac + 6; //指向key data len (MTK len), 2字节
	
	RecData[0] = 0x53;
	RecData[1] = 0x00;
	RecData[2] = 0xDB;

	ecpMem_Copy(&RecData[3], &dataBuf[kemac + 7], dataBuf[kemac + 7] +1);
	RecData[1] = RecData[3]+2;
	
	G_ResponseLen = RecData[1]+2;	
		
	return TRUE;
}

/*
	modify by chentao@20211111 GBA鉴权参数ISIM和USIM分开存储（YJR确认）
*/

BYTE DeleteMskParse(BYTE * dataBuf, BYTE dataLen)
{
	UWORD idata EFmskAddr;
	BYTE idata EFmskRecNum,EFmskRecLen,i;
	
	EFmskAddr = SelectLFFile(G_DF_ID_USIM, EF_MSK_USIM);
	EFmskRecNum = GetLFRecNumLgth(EFmskAddr, F_RecNum_Offset);	
	EFmskRecLen = GetLFRecNumLgth(EFmskAddr, F_RecLgth_Offset);		
	
	for(i=1; i<=EFmskRecNum ; i++) //以key domain id和key group id为索引搜索EF_MSK
	{
		//KEY_DOMAIN_ID(3) | NUM(1) | MSK_ID_1(4) | TS_1(4) | MSK_ID_2(4) | TS_2(4)
		ReadLFBuf(EFmskAddr, i, 0, tempBuf, EFMSK_RECLEN);		

		// "key domain id" 相同且" key group id" 相同
		if( (ecpMem_Compare(&tempBuf[KEYDOM_ID], &dataBuf[0], 3) == EQUAL) && (ecpMem_Compare(&tempBuf[MSK_ID_1], &dataBuf[3], 2) == EQUAL) )
		{			
			SetLFConst(EFmskAddr,i,0,0xff,EFmskRecLen);
			break;
		}
	}
	
	if(i > EFmskRecNum)
	{
		GlobalSetStatus(0x6A88);
		return 3; 
	}
	
	RecData[0] = 0x53;
	RecData[1] = 0x01;
	RecData[2] = 0xDB;
	
	G_ResponseLen = RecData[1]+2;
	return TRUE;
}

BYTE DeleteMukParse(BYTE * dataBuf, BYTE dataLen)
{
	BYTE  idata EFmukRecNum,EFmukRecLen,i;
	UWORD idata EFmukAddr,EF_ks_int_nafAddr,EFgbanlAddr,EFgbabpAddr;
	
	//ReadEeprom(&RecData[0], dataAddr,dataLen);

	EFmukAddr = SelectLFFile(G_DF_ID_USIM, EF_MUK_USIM);
	EFmukRecNum = GetLFRecNumLgth(EFmukAddr, F_RecNum_Offset);	
	EFmukRecLen = GetLFRecNumLgth(EFmukAddr, F_RecLgth_Offset);	
	
	for(i=1; i<=EFmukRecNum ; i++) //以key domain id和key group id为索引搜索EF_MSK
	{
		ReadLFBuf(EFmukAddr, i, 0, tempBuf, EFmukRecLen);

		if(ecpMem_Compare(tempBuf, dataBuf, dataLen) == EQUAL)
		{
			
			EF_ks_int_nafAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum], EF_GBA_KS);

			EFgbanlAddr = SelectLFFile(G_DF_ID_USIM, EF_GBANL_USIM);
			//EF_ks_int_nafAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum], EF_GBA_KS);
			EFgbabpAddr = SelectTFFile(G_DF_ID_USIM, EF_GBABP_USIM);

			SetLFConst(EFgbanlAddr,i,0,0xff,EFmukRecLen);
			SetLFConst(EF_ks_int_nafAddr,i,0,0xff,EFmukRecLen);
			SetLFConst(EFmukAddr,i,0,0xff,EFmukRecLen);
			SetTFConst(EFgbabpAddr,0,0xff,EFmukRecLen);
			
			break;
		}
	}
	if(i>EFmukRecNum) //没有找到匹配的记录
	{
		GlobalSetStatus(0x6A88);
		
		return 3;
	}

	RecData[0] = 0x53;
	RecData[1] = 0x01;
	RecData[2] = 0xDB;
	
	G_ResponseLen = RecData[1]+2;
		
	return TRUE;
}

BYTE MBMSAuth(void)
{	
	UWORD idata fileAddr;
	BYTE  idata MBMSMode,respLen;	
	BYTE  idata tlLength;
	WORD  idata dataLen;
	
	respLen = ODDAuth();
		
	if(respLen == 3)
	{
		return respLen;
	}
	
	fileAddr = SelectTFFile(DF_PROFILECFG,EF_AUTHRECDATA);
	
	fileAddr += sizeof(TranFileHead);
	
	tlLength = GetTLLength(fileAddr);
	
	dataLen = GetDataLength(fileAddr);
			
	fileAddr += tlLength;
	
	MBMSMode = ReadEECacheByte(fileAddr);
	
	if(dataLen<2)
	{
		IncorrentParameter();
		return 3;
	}
	dataLen = dataLen-2;
	
	ReadEEPromCache(ResData, fileAddr+2,dataLen);
	
	/*'01'	MSK Update Mode
	‘02'	MTK Generation Mode
	'03'	MSK Deletion Mode
	'04'	MUK Deletion Mode*/
	
	if(MBMSMode == 0x01)
	{
		respLen = MikeyMskParse(ResData,dataLen);
	}
	else if(MBMSMode == 0x02)
	{
		respLen = MikeyMtkParse(ResData,dataLen);
	}
	else if(MBMSMode == 0x03)
	{
		respLen = DeleteMskParse(ResData,dataLen);
	}
	else if(MBMSMode == 0x04)
	{
		respLen = DeleteMukParse(ResData,dataLen);
	}		
	else
	{
		GlobalSetStatus(0x6A81);
		
		return 3;
	}
	
	
	if(respLen == 3)
	{
		return respLen;
	}
	else if(respLen == FALSE)
	{
		return FALSE;
	}
	else
	{
		tempBuf[0] = G_ResponseLen;
		ecpMem_Copy(&tempBuf[1],RecData,G_ResponseLen);
		WriteVariable(Tag_BakOddData,&tempBuf[0]);

		GlobalSetStatus(0x62F3);
		
		return 3;
	}	
	
}

#endif

