/*

 * AUTHER:	yangjian

 * HISTORY: V1.0 创建源文件

 
 */
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
#include "Uicc_interface.h"
#include    "Mbms.h"

#if UICC__AUTH__LOCALKEY

BYTE code verifsucces[23] = {0x76,0x65,0x72,0x69,0x66,0x69,0x63,0x61,0x74,0x69,0x6F,0x6E,0x20,0x73,0x75,0x63,0x63,0x65,0x73,0x73,0x66,0x75,0x6C};	

#define	CounterLimit_tag	    0x81
#define	RequestMAC_tag          0x82
#define KeyIdentifier_tag       0xA0
#define	NAF_ID_tag 				0x83
#define	Terminal_ID_tag         0x84
#define Terminal_appli_ID_tag   0x85
#define UICC_appli_ID_tag       0x86
#define RANDx_tag               0x87

/*                                                                                     
 *@Function name :	 GetDateOff
 *					 
 *@Description:   获取制定tag数据的长度
  *
  *@Coder: 			 yjr            
  *
  *@Status:			   
*/

WORD GetDateOff(UWORD dataAddr,WORD dataLen,BYTE findTag)
{
	BYTE idata tag,tlLen;
	WORD idata offset,len;
			
	for(offset = 0;offset < dataLen;)
	{		
		tag = ReadEECacheByte(dataAddr+offset);
				
		tlLen = GetTLLength(dataAddr+offset);
		len = GetDataLength(dataAddr+offset);
		
		if(tag == findTag)
		{									
			ReadEEPromCache(&RecData[2],dataAddr+offset+tlLen,len);
			
			if(tag == KeyIdentifier_tag)
			{
				RecData[0]= (len>>8) & 0xFF;
				
				RecData[1]= len & 0xFF; 
				
				return (offset+tlLen);
			}						
			
			return 	len;						
		}
			
		
		offset += len + tlLen;
	}

	return 0xFFFF;
}
/*                                                                                     
 *@Function name :	 addPL
 *					 
 *@Description:   往tempBuf中增加指定TAG数据
  *
  *@Coder: 			 yjr            
  *
  *@Status:			   
*/
BYTE addPL(UWORD dataAddr,WORD off,WORD dataLen,BYTE tag,BYTE mode)
{
	WORD idata templen;
	BYTE idata len;
	
	templen = GetDateOff(dataAddr,dataLen, tag);
	
	len = templen&0xFF;
	
	if(len != 0xFF)
	{
		ecpMem_Copy(&tempBuf[off],&RecData[2],len);    //P
	
		if(mode == 0x01)
		{
			tempBuf[off+len] = ((len)>>8)&0xFF;		//L
			
			tempBuf[off+len+1] = (len)&0xFF;			//L
			
			off += len+2;
		}
		else
		{
			off += len;
		}
	}		
	
	return off;
}

/*                                                                                     
 *@Function name :	 LocalKeyDeriv
 *					 
 *@Description:    derived key Ks_local:
					-	FC = 0x01,
					-	P0 = B-TID,
					-	L0 = length of B-TID is variable (not greater than 65535),
					-	P1 = Terminal_ID,
					-	L1 = length of Terminal ID is variable (not greater than 10 octets),
					-	P2 = ICCID,
					-	L2 = length of ICCID is variable (not greater than 10 octets),
					-	P3 = Terminal_appli_ID,
					-	L3 = length of Terminal_appli_ID is variable (not greater than 32 octets),
					-	P4 = UICC_appli_ID,
					-	L4 = length of UICC_appli_ID is variable (not greater than 16 octets),
					-	P5 = RANDx,
					-	L5 = length of RANDx is variable (not greater than 16 octets).
					-	P6 = Counter Limit.
					-	L6 = length of Counter Limit is 16 octets.
  *
  *@Coder: 			 yjr            
  *
  *@Status:			   
  EPX/EPX_SUITE/CHIP/THC_THD89/RELEASE/RSP_TEST_RELEASE_20210426合并过来 by jiangxiaohui@20210426
  modify by chentao@20211111 GBA鉴权参数ISIM和USIM分开存储（YJR确认）
*/
BYTE LocalKeyDeriv(UWORD dataAddr,WORD dataLen)
{
	WORD idata iccidLen,A0Dataoff,A0DataLen,tempLen;	
	UWORD xdata fileAddr,gbanlFileAddr;
	BYTE idata EFgbanlRecNum,EFgbanlRecLen,i,len,off;

	
	A0Dataoff = GetDateOff(dataAddr,dataLen-1,KeyIdentifier_tag);
	
	A0DataLen = (RecData[0]<<8)| RecData[1];
	
	
	tempLen = GetDateOff(dataAddr+A0Dataoff,A0DataLen,NAF_ID_tag);
	
	len = tempLen & 0xFF;		
	
#if UICC__ISIM
	if(G_CurrentAdfName[G_ChannelNum] == G_DF_ID_ISIM)
	{
		gbanlFileAddr = SelectTFFile(G_DF_ID_ISIM, EF_GBANL);
	}
	else	
#endif
	{
		gbanlFileAddr = SelectTFFile(G_DF_ID_USIM, EF_GBANL_USIM);
	}
	
	EFgbanlRecNum = GetLFRecNumLgth(gbanlFileAddr , F_RecNum_Offset);
	
	EFgbanlRecLen = GetLFRecNumLgth(gbanlFileAddr , F_RecLgth_Offset);			

	for(i=1; i<=EFgbanlRecNum; i++)
	{
		ReadLFBuf(gbanlFileAddr, i, 0x00, &tempBuf[0], EFgbanlRecLen);		

		if(tempBuf[1] != len)
		{
			continue;
		}
		
		if(memcmp_p(&tempBuf[2],&RecData[2],len) == EQUAL)
		{
			break;
		}
	}
	if(i>EFgbanlRecNum) //没有找到匹配的记录
	{
		GlobalSetStatus(0x6A88);
		
		return 3;
	}
		
	off = 0;
	
	tempBuf[off] = 0x01;						//FC
	
	len = tempBuf[2+len+1];
	//templen = tempBuf[2+len+1];
	
	ecpMem_Copy(&tempBuf[off+1],&tempBuf[2+tempBuf[1]+2],len);    //P0:B-TID
	
	
	tempBuf[off+1+len] = 0x00;
	
	tempBuf[off+2+len] = len;
	
	
	off += len+3;
	
	off = addPL(dataAddr+A0Dataoff,off,A0DataLen,Terminal_ID_tag,0x01);		//P1
	
	
	fileAddr = SelectTFFile(DF_MF, EF_ICCID);
	
	iccidLen= GetTFSize(fileAddr);
	
	if (fileAddr != FALSE)
	{
		ReadTFBuf(fileAddr,0,&tempBuf[off], iccidLen); 		//P2:ICCID
	}
	
	len = iccidLen & 0xFF;
	
	tempBuf[off+len] = 0x00;		//L2
	
	tempBuf[off+len+1] = len;			//L2
	
	off += len+2;
	
	off = addPL(dataAddr+A0Dataoff,off,A0DataLen,Terminal_appli_ID_tag,0x01);		//P3
	
	
	off = addPL(dataAddr+A0Dataoff,off,A0DataLen,UICC_appli_ID_tag,0x01);		//P4
	
	
	off = addPL(dataAddr+A0Dataoff,off,A0DataLen,RANDx_tag,0x01);		//P5
	
	
	off = addPL(dataAddr,off,dataLen-1,CounterLimit_tag,0x01);		//P6
	
	
	fileAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum],EF_GBA_KS);
		
	ReadLFBuf(fileAddr,G_MUK_IDX,0,&RecData[0],0x20);	//Ks_int_NAF
	
	//HMAC_SHA256(&RecData[0x00], 0x20, &tempBuf[0], off, &RecData[0xA0]);	 //ks_local
	HMACSha256DoFinalSingleCall(&RecData[0x00], 0x20, &tempBuf[0], off, &RecData[0xA0]);	 //ks_local
	
	off = 0x00;
	
	off = addPL(dataAddr+A0Dataoff,off,A0DataLen,NAF_ID_tag,0x00);		//NAF_ID
	
	off = addPL(dataAddr+A0Dataoff,off,A0DataLen,Terminal_ID_tag,0x00);		//Terminal_ID
	
	fileAddr = SelectTFFile(DF_MF, EF_ICCID);
	
	iccidLen= GetTFSize(fileAddr);
	
	if (fileAddr != FALSE)
	{
		ReadTFBuf(fileAddr,0,&tempBuf[off], iccidLen); 				//ICCID
	}
	len = iccidLen & 0xFF;
	
	off += len;
	
	off = addPL(dataAddr+A0Dataoff,off,A0DataLen,Terminal_appli_ID_tag,0x00);		//Terminal_appli_ID
 
	off = addPL(dataAddr+A0Dataoff,off,A0DataLen,UICC_appli_ID_tag,0x00);		//UICC_appli_ID
	
	off = addPL(dataAddr+A0Dataoff,off,A0DataLen,RANDx_tag,0x00);		//RANDx
	
	off = addPL(dataAddr,off,dataLen-1,CounterLimit_tag,0x00);		//Counter Limit
		
	//HMAC_SHA256(&RecData[0xA0], 0x20, &tempBuf[0], off, &RecData[0x20]);//HMAC-SHA-256 (Key , S) //mac
	HMACSha256DoFinalSingleCall(&RecData[0xA0], 0x20, &tempBuf[0], off, &RecData[0x20]);//HMAC-SHA-256 (Key , S) //mac
	
	tempLen = GetDateOff(dataAddr,dataLen-1,RequestMAC_tag);
	
	len = tempLen&0xFF;
	
	if(memcmp_p(&RecData[2],&RecData[0x20],len) != EQUAL)
	{
		return FALSE;
	}
	
	//HMAC_SHA256(&RecData[0xA0], 0x20, (BYTE CONST *)verifsucces, 23, &RecData[5]);//HMAC-SHA-256 (Key , S) //mac		
	HMACSha256DoFinalSingleCall(&RecData[0xA0], 0x20, (BYTE *)verifsucces, 23, &RecData[5]);//HMAC-SHA-256 (Key , S) //mac
	
	ReadEEPromCache(&tempBuf[1], (UWORD)(dataAddr+A0Dataoff),A0DataLen);
	
	tempBuf[0] = A0DataLen&0xFF;
	
	fileAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum],EF_LOCALKEY_ID);

	WriteLFBuf(fileAddr, i, 0x00, &tempBuf[0], A0DataLen+1);
	
	RecData[0] = 0x73;
	RecData[1] = 0x13;
	RecData[2] = 0xDB;
	RecData[3] = 0x82;
	RecData[4] = 0x10;
	
	G_ResponseLen = 0x15;
	return TRUE;
}

/*                                                                                     
 *@Function name :	 CheckLocalKeyAvailab
 *					 
 *@Description:   鉴权数据KeyID是否
  *
  *@Coder: 			 yjr            
  *
  *@Status:			   
*/

BYTE CheckLocalKeyAvailab(UWORD dataAddr,WORD dataLen)
{
	BYTE idata tag,i,A0DataLen,tlLength,EFRecNum,EFRecLen;
	WORD idata A0Dataoff;	
	UWORD xdata fileAddr;
	BYTE idata off,tagflag;
	
	tag = ReadEECacheByte(dataAddr);
	
	if(tag != KeyIdentifier_tag)
	{
		GlobalSetStatus(0x6A80);
		
		return 3;
	}
	
	
	A0Dataoff = GetDateOff(dataAddr,dataLen-1,KeyIdentifier_tag);
	
	A0DataLen =  RecData[1];
	
	off = 0x00;
	tagflag = 0x00;
	
	while(off < A0DataLen)
	{
		//tag = MEM_read_u8((Receiveaddr + idoffset));
		tag = ReadEECacheByte(dataAddr+A0Dataoff+off);
		//off++;		
		tlLength = GetTLLength(dataAddr+A0Dataoff+off);
	
		dataLen = GetDataLength(dataAddr+A0Dataoff+off);
		//off += BERTLVLen(Receiveaddr,idoffset);
		
		off += tlLength+dataLen;
		tagflag ++;
	}
	
	if(tagflag != 0x05)
	{
		GlobalSetStatus(0x6A80);
		
		return 3;
	}

	fileAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum],EF_LOCALKEY_ID);	
	
	EFRecNum = GetLFRecNumLgth(fileAddr, F_RecNum_Offset);
	
	EFRecLen = GetLFRecNumLgth(fileAddr, F_RecLgth_Offset);			

	for(i=1; i<=EFRecNum; i++)
	{
		ReadLFBuf(fileAddr, i, 0x00, &tempBuf[0], EFRecLen);		

		if(tempBuf[0] != A0DataLen)
		{
			continue;
		}
		
		if(memcmp_p(&tempBuf[1],&RecData[2],A0DataLen) == EQUAL)
		{
			break;
		}
	}
	if(i>EFRecNum) //没有找到匹配的记录
	{
		GlobalSetStatus(0x6A88);
		
		return 3;
	}
	
	RecData[0] = 0x73;
	RecData[1] = 0x01;
	RecData[2] = 0xDB;
	
	G_ResponseLen = 0x03;
	
	return TRUE;
}

/*                                                                                     
 *@Function name :	 LocalKeyEstablish
 *					 
 *@Description:   接收数据，对数据的第一个字节进行区分处理， LocalKeyDeriv/CheckLocalKeyAvailab
  *
  *@Coder: 			 yjr            
  *
  *@Status:			   
*/
BYTE LocalKeyEstablish(void)
{
	UWORD idata fileAddr;
	BYTE  idata localKeyType,respLen;	
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
	
	//GetBlockData(&localKeyType,0x00,1);
	localKeyType = ReadEECacheByte(fileAddr);
	
	if(localKeyType == 0x01)
	{
		respLen = LocalKeyDeriv(fileAddr+1,dataLen);
	}
	else if(localKeyType == 0x02)
	{
		respLen = CheckLocalKeyAvailab(fileAddr+1,dataLen);
	}
	else
	{
		GlobalSetStatus(0x9864);
		
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
		//apdu_ApduSetOutgoingAndSend(5,G_ResponseLen);
		tempBuf[0] = G_ResponseLen;
		ecpMem_Copy(&tempBuf[1],RecData,G_ResponseLen);
		WriteVariable(Tag_BakOddData,&tempBuf[0]);
		GlobalSetStatus(0x62F3);
		
		return 3;
	}	
	
}

#endif

