
#include  "uicc_main.h"
#include  "gsmpublic.h"
#include  "uicc_interface.h"
#include "fileAction.h"
#include  "fileSystem.h"
#include "EapAuthentication.h"
#include "Authentication.h"
#include "Aes.h"
//#include "Sha1.h"
#include "MemOperation.h"
#include "stkhead.h"
#include "Crypto_API.h"
#include "uicc_error.h"

WORD code	SupportedVersions[1] = {0x0001};
EapContext   xdata   G_currentEAPContext;

#define RANDOMSIZE 0x10

/*
*@description: 从eap包中的Attribute列表中得到指定Attribute
*
*@param      :Attributelist :指向eap包中Attribute列表起始（此后的数据为N个TLV结构的Attribute）
*@param      :Attributetag : 指定的Attribute
						 :addrMode:     表示Attributelist指针所指向的地址类型
*
*@return    :offset: 获取到的Attribute在相对Attributelist的偏移位置	,0xFF表示未找到
*
*@author    :create by lj
*
*@hisotry	:V1.0 :create by lj
*/
BYTE GetEapAttribute(BYTE *Attributelist,WORD listlen,BYTE Attributetag)
{
	BYTE idata tag[2];
	WORD idata offset;

	for(offset = 0;offset < listlen;)
	{		
		ReadEeprom(&tag[0],(UWORD)(Attributelist+offset),2);
		
		if(tag[0] == Attributetag)
		{
			return 	offset;
		}

		offset += (tag[1]*4);
	}

	return 0xFF;
}
/*@Function:   SelectDFFilePublic                                            
 * 
 *@Descrip:    EAP初始化函数 主要初始化EF_EAPKEYS,EF_EAPSTATUS,EF_ReID文件
 *                                                                                                                                     
 *@Param:      void
		                                                                                                  
 *@Return:     void                                                                                                                                                                                                                                                           
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
BYTE SelectDFFilePublic(BYTE dirNo,BYTE * buf,BYTE * retOffset,BYTE * retOffset80,BYTE * retOffset81)
{
	UWORD idata fileAddr;
	BYTE  idata a0Length,offset80,offset81,offset,A0offset;
	
	fileAddr = SelectLFFile(DF_MF,0x2F00);

	ReadLFBuf(fileAddr,dirNo,0x00,&buf[0],GetLFRecNumLgth(fileAddr,F_RecLgth_Offset));
	
	if(buf[0] != 0x61)
	{
		return FALSE;
	}
	
	offset = FindTag(&buf[2],0x73,buf[1]);
	
	if(offset == FALSE)
	{
		return FALSE;
	}
	offset = offset+2;
	
	A0offset = FindTag(&buf[offset+1],0xA0,buf[offset]);
	
	if(A0offset == FALSE)
	{
		return FALSE;
	}
	
//	if((buf[2+buf[1]]!=0x73)||(buf[2+buf[1]+2]!=0xA0))  //Discretionary template tag / EAP Application tag
//	{
//		return FALSE;			
//	}

//	a0Length = buf[2+buf[1]+2+1];       //A0 length
//	offset = 2+buf[1]+2+2;  //point to Application EAP supported types list tag "80"
	offset= A0offset+offset+1;
	a0Length = buf[offset];       //A0 length
	offset = offset+1;  //point to Application EAP supported types list tag "80"
	

	offset80 = FindTag(&buf[offset],0x80,a0Length);

	offset81 = FindTag(&buf[offset],0x81,a0Length);

	if((offset80 == FALSE)||(offset81 == FALSE))
	{
		return FALSE;
	}

	if(buf[offset+offset81] != (buf[offset+offset80]*2))          //81 data lengthê? 80 data length μ?2±?
	{
		return FALSE;
	}

	* retOffset = offset;
	* retOffset80 = offset80;
	* retOffset81 = offset81;

	return TRUE;
}

/*@Function:   IF_EapSystemInit                                            
 * 
 *@Descrip:    EAP初始化函数 主要初始化EF_EAPKEYS,EF_EAPSTATUS,EF_ReID文件
 *                                                                                                                                     
 *@Param:      void
		                                                                                                  
 *@Return:     void                                                                                                                                                                                                                                                           
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
void IF_EapSystemInit(void)
{
	UWORD xdata fileAddr,fileAddr2,fileAddr3;
	BYTE  idata num,i,temp;
	BYTE  idata offset,offset4F,offset80,offset81;
	
	tempBuf[0] = P3;
	
	ecpMem_Copy(&tempBuf[1],&RecData[0],P3);
	
	num = GetAIDNumber(&tempBuf[0],FALSE);
	
	if(num == FALSE)
	{
		return;
	}
/*	
	fileAddr = SelectLFFile(DF_MF,0x2F00);
	
	ReadLFBuf(fileAddr,num,0x00,&tempBuf[0x80],GetLFRecNumLgth(fileAddr,FileLgth));	
		
	if(tempBuf[0x80] != 0x61)
	{
		return;
	}
	
	if((tempBuf[0x80+2+tempBuf[0x80+1]]!=0x73)||(tempBuf[0x80+2+tempBuf[0x80+1]+2]!=0xA0))  //Discretionary template tag / EAP Application tag
	{
		return;			
	}

	offset4F =  FindTag(&tempBuf[0x80+2],0x4F,tempBuf[0x80+1]);

	a0Length = tempBuf[0x80+2+tempBuf[0x80+1]+2+1];       //A0 length
	offset = 2+tempBuf[0x80+1]+2+2;  //point to Application EAP supported types list tag "80"

	offset80 = FindTag(&tempBuf[0x80+offset],0x80,a0Length);

	offset81 = FindTag(&tempBuf[0x80+offset],0x81,a0Length);

	if(((offset80 == FALSE)||(offset81 == FALSE))||(offset4F == FALSE))
	{
		return;
	}

	if(tempBuf[0x80+offset+offset81] != (tempBuf[0x80+offset+offset80]*2))          //81 data length是 80 data length 的2倍
	{
		return;
	}
*/
	if( SelectDFFilePublic(num,&tempBuf[0x80],&offset,&offset80,&offset81) == FALSE)
	{
		return;
	}

	offset4F =  FindTag(&tempBuf[0x80+2],0x4F,tempBuf[0x80+1]);

	if(offset4F == FALSE)
	{
		return;
	}

	for(i=0;i<tempBuf[0x80+offset+offset81];i+=2)
	{
		fileAddr = SelectFileByName(E_FILE_START,&tempBuf[0x80+2+offset4F+1],tempBuf[0x80+2+offset4F]); //选adf
		
		fileAddr2 = SelectFile(fileAddr,(tempBuf[0x80+offset+offset81+1+i]<<8)|(tempBuf[0x80+offset+offset81+1+i+1]));

		//fileAddr3 = SelectFile(fileAddr2,EF_EAPKEYS); //modify chentao20220422 eUICC_Profile_Package_Interoperable_Format_Test_V_2_3/TC_8_2_3_17_Installing_EAP_files_by_template 不期望清4F01文件的值
		
// 		if(fileAddr3 != FALSE)
// 		{
// 			SetTFConst(fileAddr3,0,0xFF,GetTFSize(fileAddr3));
// 		}
		
		//ClearFileConst(fileAddr3,0xFF);
		
		fileAddr3 = SelectFile(fileAddr2,EF_EAPSTATUS);

		if(fileAddr3 != FALSE)
		{
			SetTFConst(fileAddr3,0,EAP_AuthEdle,1);
		}

		/*fileAddr3 = SelectFile(fileAddr2,EF_ReID);//modify chentao20220422 eUICC_Profile_Package_Interoperable_Format_Test_V_2_3/TC_8_2_3_17_Installing_EAP_files_by_template 不期望清4F21文件的值

		if(fileAddr3 != FALSE)
		{
			tempBuf[0x20] = 0x80;
			tempBuf[0x21] = 0x00;
			tempBuf[0x22] = 0xFF;
			tempBuf[0x23] = 0x81;
			tempBuf[0x24] = 0x00;
			
			WriteTFBuf(fileAddr3,0,&tempBuf[0x20],0x05);
			
			SetTFConst(fileAddr3,5,0xFF,(GetTFSize(fileAddr3)-5)); 						
		}*/

		fileAddr3 = SelectFile(fileAddr2,EF_PSID);

		if(fileAddr3 != FALSE)
		{
			ReadTFBuf(fileAddr3,0,&temp,1);
			
			if(temp != 0xFF)
			{
				G_currentEAPContext.EapHavePseuID = TRUE;
			}	
		}
		
		fileAddr3 = SelectTFFile(G_CurrentAdfName[G_ChannelNum],EF_EAPAUTHDATA);
		
// 		if(fileAddr3 != FALSE)
// 		{
// 			SetTFConst(fileAddr3,0,0x00,GetTFSize(fileAddr3));	
// 		}
		
		ClearFileConst(fileAddr3,0x00);
	}

	ecpMem_Set((BYTE *)&G_currentEAPContext,0x00,sizeof(G_currentEAPContext));

	G_ResponseLen = 0x00;	
	
}

void reportEAP(BYTE mode)
{
	ecpMem_Set(RecData,0x00,256);
	RecData[0] = EAP_RESPONSE;
	RecData[1] = G_currentEAPContext.Messageindex;
	RecData[2] = 0x00;
	RecData[4] = G_currentEAPContext.Eaptype;
	RecData[5] = mode;
	RecData[6] = RecData[7] = 0x00;		
} 

void ODDResponse(void)
{
	if(INS == AUTHENTICATE_ODD)
	{
		tempBuf[1] = BER_TAG;
		tempBuf[2] = G_ResponseLen;
	
		ecpMem_Copy(&tempBuf[3],RecData,G_ResponseLen);
		
		G_ResponseLen = G_ResponseLen+2;
		
		tempBuf[0] = G_ResponseLen;
		
		WriteVariable(Tag_BakOddData,&tempBuf[0]);

	}
}
/*@Function:   ReadLFFileValue                                            
 * 
 *@Descrip:    从LF文件读取内容
 *
 *@See:	       
 *                                                                                                                                     
 *@Param:      fileAddr      
 *              						                                                                                                  
 *@Return:     BYTE                                                                                                                                                                                                                                                              
 * 
 *@Author:     xwh
 *  
 *@History:    V0.0---Create by xwh 20180801
 *                        
 */
BYTE ReadTFFileValue1ByteLen(UWORD fileAddr,WORD offset,BYTE * buf)
{
		BYTE  idata len;
	
		ReadTFBuf(fileAddr,offset,&len,1);         
		ReadTFBuf(fileAddr,offset+1,buf,len); 
	
		return len;
}


/*@Function:   eap_SIMCalculateMK                                            
 * 
 *@Descrip:    EAP_SIM中计算MK,参考标准RFC4186 7章
 *                                                                                                                                     
 *@Param:      kc：鉴权数据通过Milenage2G计算的结果
			   mkbuf:计算结果
		                                                                                                  
 *@Return:     TRUE/FALSE                                                                                                                                                                                                                                                           
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
BYTE eap_SIMCalculateMK(BYTE * kc,BYTE * mkbuf)
{
	UWORD xdata fileaddr;
	BYTE idata offset,i;

	fileaddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_CurID);
	
	if(fileaddr == FALSE)
	{
		return FALSE;
	}

	//ReadTFBuf(fileaddr,1,&tempBuf[0],1);  //buf 0 为长度
	//offset = tempBuf[0];
	//ReadTFBuf(fileaddr,2,&tempBuf[0],tempBuf[0]);
	offset = ReadTFFileValue1ByteLen(fileaddr, 1,&tempBuf[0]);

	ecpMem_Copy(&tempBuf[offset],&kc[0],24);
	offset += 24;
	ecpMem_Copy(&tempBuf[offset],Nonce,16);
	offset += 16;	
	
	for(i=0;i<G_currentEAPContext.VersionListlen;i=i+2)
	{
		tempBuf[offset++] = (BYTE)(G_currentEAPContext.VersionList[i]>>8);
		tempBuf[offset++] = (BYTE)(G_currentEAPContext.VersionList[i]);
	}
	
	tempBuf[offset] = (BYTE)(G_currentEAPContext.SelectedVersion>>8);
	tempBuf[offset+1] = (BYTE)(G_currentEAPContext.SelectedVersion);

	offset+=2;
	
	//SHA1DoFinalSingleCall(mkbuf,tempBuf,offset,0,0);
	SHA1Final(mkbuf,tempBuf,offset,0,0);

	return TRUE;
}
/*@Function:   eap_AKACalculateMK                                            
 * 
 *@Descrip:    EAP_AKA中计算MK,参考标准RFC4187 7章
 *                                                                                                                                     
  *@Param:      ck、ik：鉴权数据通过Milenage计算的结果
			   mkbuf:计算结果    
		                                                                                                  
 *@Return:     TRUE/FALSE                                                                                                                                                                                                                                                           
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
BYTE eap_AKACalculateMK(BYTE * ck,BYTE * ik,BYTE * mkbuf)
{
	UWORD xdata fileaddr;
	BYTE idata offset;

	fileaddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_CurID);
	
	if(fileaddr == FALSE)
	{
		return FALSE;
	}

	//ReadTFBuf(fileaddr,1,&tempBuf[0],1);  //buf 0 为长度
	//offset = tempBuf[0];
	//ReadTFBuf(fileaddr,2,&tempBuf[0],tempBuf[0]);
	offset = ReadTFFileValue1ByteLen(fileaddr, 1,&tempBuf[0]);

	ecpMem_Copy(&tempBuf[offset],&ik[0],16);
	offset += 16;

	ecpMem_Copy(&tempBuf[offset],&ck[0],16);
	offset += 16;

	//SHA1DoFinalSingleCall(mkbuf,tempBuf,offset,0,0);
	SHA1Final(mkbuf,tempBuf,offset,0,0);

	return TRUE;
}
/*@Function:   eap_SIMCalcReauthXKEY                                            
 * 
 *@Descrip:    EAP_SIM中计算XKEY，参考标准RFC4186 7章
 *                                                                                                                                     
  *@Param:     mk：上次FULL鉴权数据通过Milenage2G计算的结果
			   counter：服务器下行counter
			   XKEY:计算结果    
		                                                                                                  
 *@Return:                                                                                                                                                                                                                                                                
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
void eap_SIMCalcReauthXKEY(BYTE * mk,WORD counter,BYTE * XKEY)
{
	UWORD xdata fileaddr;
	BYTE idata offset;

	fileaddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_CurID);

	ReadTFBuf(fileaddr,0,tempBuf,2);

	if(tempBuf[0]!= 0x02)
	{
		return;
	}

	offset = tempBuf[1];
	ReadTFBuf(fileaddr,2,tempBuf,offset);//第二字节为id长度

	tempBuf[offset] = (BYTE)(counter>>8);
	tempBuf[offset+1] = (BYTE)counter;
	offset +=2;

	ecpMem_Copy(&tempBuf[offset],Nonce,16);

	offset+=16;

	ecpMem_Copy(&tempBuf[offset],mk,20);

	offset+=20;

	//SHA1DoFinalSingleCall(XKEY,tempBuf,offset,0,0);
	SHA1Final(XKEY,tempBuf,offset,0,0);
}

/*@Function:   getEapAuthIdentity                                            
 * 
 *@Descrip:    获取EAP鉴权ID的子函数
 *                                                                                                                                     
 *@Param:     idtype：需求获取id的类型
			   buf：获取到的ID数据 
		                                                                                                  
 *@Return:     id的长度                                                                                                                                                                                                                                                          
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
BYTE getEapAuthIdentity(BYTE idtype,BYTE * buf)
{
	UWORD xdata fileaddr;
	WORD xdata filename;
	WORD idata filesize	,idlen,i;

	filename = (idtype ==AT_PERMANENT_ID_REQ) ?EF_PUID:((idtype == AT_FULLAUTH_ID_REQ) ?EF_PSID:EF_ReID);

	fileaddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],filename);

	idlen = filesize = GetTFSize(fileaddr);

	if(filesize >= sizeof(tempBuf))
	{
		idlen = filesize = (sizeof(tempBuf) -2 );
	}

	ReadTFBuf(fileaddr,0,buf,filesize);

	if(filename == EF_ReID)
	{
		if(buf[0]!= 0x80)
		{
			return 0x00;
		}

		idlen =	buf[1];

		ecpMem_Move(buf,buf+2,buf[1]);

		return idlen;
	}

	for(i = filesize;i>0;i--)
	{
		if(buf[i-1] == 0xFF)
		{
			idlen -= 1;
		}
		else
		{
			break;
		}
	}

	return idlen;
}

/*@Function:   reportEAP_ClintError                                            
 * 
 *@Descrip:    EAP错误消息的拼装
 *                                                                                                                                     
 *@Param:     errorcode：错误类型
		                                                                                                  
 *@Return:                                                                                                                                                                                                                                                             
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
void reportEAP_ClintError(BYTE errorcode)
{
	BYTE idata AToffset;
	UWORD xdata fileaddr;

	if(G_currentEAPContext.Eaptype == EAP_Identity)
	{
		return;
	}

	fileaddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_EAPSTATUS);
	SetTFConst(fileaddr,0,EAP_AuthFailure,1);
	reportEAP(EAP_ClintError);

	AToffset = 0x08;
	RecData[AToffset] = AT_CLIENT_ERROR_CODE;
	RecData[AToffset+1] = 0x01;
	RecData[AToffset+2] = 0x00;
	RecData[AToffset+3] = errorcode;

	AToffset += 0x04;

	RecData[3] = AToffset;
	G_ResponseLen = RecData[3];			
}

/*@Function:   reportEAP_AKAAuthReject                                            
 * 
 *@Descrip:    EAP_AKA鉴权在执行  Milenage 算法时，失败
 *                                                                                                                                     
 *@Param:     
		                                                                                                  
 *@Return:                                                                                                                                                                                                                                                             
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
void reportEAP_AKAAuthReject(void)
{
	UWORD xdata fileaddr;

	fileaddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_EAPSTATUS);

	if(fileaddr != FALSE)
	{
		SetTFConst(fileaddr,0,EAP_AuthFailure,1);
	}	
	reportEAP(AKA_AuthenticationReject);
	

	RecData[3] = 0x08;
	G_ResponseLen = 0x08;
}

/*@Function:   notifyEAP_Result                                            
 * 
 *@Descrip:    EAP通知结果的拼装
 *                                                                                                                                     
 *@Param:    errorcode：通知类型
		                                                                                                  
 *@Return:                                                                                                                                                                                                                                                             
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
void notifyEAP_Result(BYTE errorcode)
{	
	reportEAP(EAP_Notification);
	RecData[3] = 0x08;
	G_ResponseLen = 0x08;
}

/*@Function:   processEAP_Identity                                            
 * 
 *@Descrip:    获取EAP鉴权的Pseudonym ID   并拼装响应数据  
 *                                                                                                                                     
 *@Param:    
		                                                                                                  
 *@Return:                                                                                                                                                                                                                                                             
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
void processEAP_Identity(void)
{
	BYTE idata idlen;
	UWORD xdata fileaddr;

	if((RecData[2]== 0x00)&&(RecData[3]== 0x05))
	{
		RecData[0] = EAP_RESPONSE;
		RecData[1] = G_currentEAPContext.Messageindex;
		RecData[2] = 0x00;
		RecData[4] = EAP_Identity;
  		idlen = getEapAuthIdentity(AT_PERMANENT_ID_REQ,tempBuf+2);
		ecpMem_Copy(RecData+5,tempBuf+2,idlen);

		fileaddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_CurID);
		
		if(fileaddr == FALSE)
		{
			return ;
		}

		tempBuf[0] = 0x00;//permenent id
		tempBuf[1] = idlen;

		WriteTFBuf(fileaddr,0,&tempBuf[0],idlen+2);

		RecData[3] = idlen+5;
		G_ResponseLen = RecData[3];
	}
	else
	{
	}
}

BYTE processIdAttribute()
{
	BYTE  * xdata  Attributelist;
	BYTE  idata idtype,i,j;
	BYTE  idata attibute,attiLen,tlLength;
	WORD  idata offset,Attributelistlen;
	UWORD idata dataFileAddr;
	BYTE  idata haveidflag;//三个id request只允许同时存在一个
	
	idtype = 0x00;
	
	haveidflag = FALSE;
	
	dataFileAddr = SelectTFFile(DF_PROFILECFG,EF_AUTHRECDATA);  //modify by xieyong@20221227  EF_AUTHRECDATA放在DF_PROFILECFG下面,和之前关于EF_AUTHRECDATA的修改一致
	
	dataFileAddr += sizeof(TranFileHead);

	if(INS == AUTHENTICATE_ODD)
	{		
		tlLength = GetTLLength(dataFileAddr);
		
		Attributelistlen = GetDataLength(dataFileAddr) - 8;			
	}
	else	
	{
		tlLength = 0;
				
		Attributelistlen = P3 -8;		
	}
	
	Attributelist = (BYTE *)(dataFileAddr + tlLength + 8); //指向第一个attribute,从偏移8开始为ATlist
	
	for(offset = 0;offset < Attributelistlen;)
	{
		ReadEeprom(&attibute,(UWORD)(Attributelist+offset),sizeof(BYTE));
		
		switch(attibute)
		{
			case AT_ANY_ID_REQ:
			case AT_FULLAUTH_ID_REQ:
			case AT_PERMANENT_ID_REQ:
			{
				if(haveidflag == TRUE)
				{
					reportEAP_ClintError(0x00);
					
					return FALSE;
				}
				haveidflag = TRUE;
				
				idtype = attibute;
				
				break;
			}
			case AT_VERSION_LIST:
			{
				//G_currentEAPContext.VersionListlen = Attributelist[offset+3];//第一word最后字节表示真实长度
				
				ReadEeprom(&G_currentEAPContext.VersionListlen,(UWORD)(Attributelist+offset+3),sizeof(BYTE));
								
				for(j=0;j<G_currentEAPContext.VersionListlen;j+=2)
				{
					for(i=0;i< sizeof(SupportedVersions);i++)
					{												
						//G_currentEAPContext.VersionList[j/2]= (WORD )(Attributelist[offset+4+j]<<8)|Attributelist[offset+5+j];
						
						//ReadEeprom(&tempBuf[0],(UWORD)(Attributelist+offset+4+j),0x02);
						
						//G_currentEAPContext.VersionList[j/2]= (WORD )(tempBuf[0]<<8)|tempBuf[1];

						G_currentEAPContext.VersionList[j/2] = ReadWORD((UWORD)(Attributelist+offset+4+j));

						if(SupportedVersions[i] == G_currentEAPContext.VersionList[j/2])
						{
							G_currentEAPContext.SelectedVersion	= SupportedVersions[i];
							break;
						}
					}

					if(G_currentEAPContext.SelectedVersion != 0x00)
					{
						break;
					}
				}
				break;
			}					
			default:
				if(attibute < 0x80)
				{
					reportEAP_ClintError(0x00);
					
					return FALSE;
				}
		}

		ReadEeprom(&attiLen,(UWORD)(Attributelist+offset+1),sizeof(BYTE));
		
		offset += (attiLen*4);
	}
	
	if((G_currentEAPContext.EapHaveReauID == FALSE)&&(idtype == AT_ANY_ID_REQ))
	{
		idtype = AT_FULLAUTH_ID_REQ;
	}
	
	if((G_currentEAPContext.EapHavePseuID == FALSE)&&(idtype == AT_FULLAUTH_ID_REQ))
	{
		idtype = AT_PERMANENT_ID_REQ;
	}		
	
	tempBuf[0] = idtype;
	
	return TRUE;
}

BYTE responseID(BYTE AToffset,BYTE idtype)
{
	BYTE  idata idlen;
	UWORD xdata fileaddr;
	
	RecData[AToffset] = AT_IDENTITY;
	RecData[AToffset+2] = 0x00;
	idlen = getEapAuthIdentity(idtype,tempBuf+2);	
	ecpMem_Copy(&RecData[AToffset+4],tempBuf+2,idlen);

	RecData[AToffset+3] = idlen;
	RecData[AToffset+1]	= ((RecData[AToffset+3]+4)+ 3)/4; //+3是为了凑填充
	AToffset = AToffset+ RecData[AToffset+1]*4;

	fileaddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_CurID);
		
	tempBuf[0] = (idtype == AT_PERMANENT_ID_REQ)?0x00:((idtype == AT_FULLAUTH_ID_REQ)?0x01:0x02);//permenent id
	tempBuf[1] = idlen;

	WriteTFBuf(fileaddr,0,&tempBuf[0],idlen+2);
	
	return AToffset;
}

/*@Function:   processEAP_AKAIdentity                                            
 * 
 *@Descrip:   EAP_AKA鉴权中根据属性类型获取对应ID ,并拼装响应数据
 *                                                                                                                                     
 *@Param:    
		                                                                                                  
 *@Return:                                                                                                                                                                                                                                                             
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
void processEAP_AKAIdentity(void)
{	
	BYTE  idata idtype,AToffset,result;

	result = processIdAttribute();
	
	if(result == FALSE)
	{
		return;
	}
	idtype = tempBuf[0];
	
	reportEAP(G_currentEAPContext.Eapsubtype);

	AToffset = 0x08;

	if(idtype != 0)
	{
		AToffset = responseID(AToffset,idtype);

		RecData[3] = AToffset;//包总长度
		G_ResponseLen = RecData[3];
		
		ODDResponse();

		return;

	}
	else
	{
		reportEAP_ClintError(0x00);
	}
}

/*@Function:   processEAP_SIMStart                                            
 * 
 *@Descrip:   鉴权开始，EAP_SIM鉴权的子类型Start鉴权处理， 主要处理属性为ID,版本，卡片同时生成随机数,并拼装响应数据
 *                                                                                                                                     
 *@Param:    
		                                                                                                  
 *@Return:                                                                                                                                                                                                                                                             
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
void processEAP_SIMStart(void)
{
	BYTE idata idtype,result,AToffset;
	UWORD xdata tempFileAddr;
	
	result = processIdAttribute();
	
	if(result == FALSE)
	{
		return;
	}
	
	idtype = tempBuf[0];
	
	if(G_currentEAPContext.SelectedVersion !=0)
	{		
		reportEAP(G_currentEAPContext.Eapsubtype);
	
		AToffset = 0x08;

		if(idtype != 0)
		{
			AToffset = responseID(AToffset,idtype);

			if(idtype == AT_ANY_ID_REQ)	  //快速鉴权不拼装version等字段
			{
				RecData[3] = AToffset;//包总长度
				G_ResponseLen = RecData[3];
				return;
			}
		}

		RecData[AToffset] = AT_NONCE_MT;
		RecData[AToffset+1] = 0x05;
		RecData[AToffset+2] = RecData[AToffset+3] = 0x00;

		if(G_currentEAPContext.EapResendFlag == FALSE)
		{
			Random_Number_Generator_Get(Nonce,16);
			tempFileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum],EF_EAPAUTHDATA);
			if(tempFileAddr != FALSE)
			{
				WriteTFBuf(tempFileAddr,NonceOffset,Nonce,0x10);
			}
			
			
		}
		ecpMem_Copy(&RecData[AToffset+4],Nonce,16);
		
		AToffset = AToffset+20;

		RecData[AToffset] = AT_SELECTED_VERSION;
		RecData[AToffset+1] = 0x01;
		RecData[AToffset+2]	= (BYTE)(G_currentEAPContext.SelectedVersion>>8);
		RecData[AToffset+3]	= (BYTE)(G_currentEAPContext.SelectedVersion);

		AToffset = AToffset+4;

		RecData[3] = AToffset;//包总长度
		G_ResponseLen = RecData[3];
	}
	else
	{
		reportEAP_ClintError(0x00);
	}
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 AESArithFunction
  
  Data   content:    

															 
  Description:       
  					 

  Coder:                    Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/
void  AESArithFunction(BYTE* initVector,BYTE * input,BYTE * keyContext,BYTE * output, BYTE bMod, BYTE lgth,BYTE addrMode)
{

	BYTE xdata resultData[16];
	BYTE idata num;
	BYTE idata i,mode,tmp;
	BYTE idata inputData[16],outputData[16];
	
	if(((lgth%16) != 0x00) || (lgth == 0x00)) 		// 长度不是 16 的整数倍
	{
		return; 
	}
	
	//AES_Nk = 4;           //AES 全局变量初始化   默认密钥长度为16字节  AES使用前需要进行密钥扩展
	//AES_Nr = 10;         //密钥扩展前key_expansion 要对这两个全局变量初始化
	if(CheckAesKeyLength(0x10)  == FALSE)
	{
		return ;
	}

	KeyExpansion(&keyContext[0], &tempBuf[80]);  
	//KeySchedule(&keyContext[0]);
	
	ecpMem_Copy(resultData, initVector, sizeof(resultData));

	mode = ( ((bMod&0x0F) == 0x05) ? 0x01 : 0x00);	    // 0x00 -> ENC 0x01	-> DEC
														
	for(num = 0x00; num < lgth; num += 0x10)
	{
		ReadEeprom(inputData,(UWORD)(input+num),16);	
        
		for(i = 0x00; i < 0x10; i++)
		{
			if(bMod == CBC_Encrypt)
			{
				//initVector[i] ^= input[i + num];
				initVector[i] ^= inputData[i];
			}
			else
			{
				//initVector[i] = input[i + num];
				initVector[i] = inputData[i];
			}
		}

//		KeySchedule(&keyContext[0]);	

		if(mode==0x00)
		{
			//AESEncrypt(&initVector[0], &initVector[0]);
			AESEncryptForUICC(&initVector[0], &initVector[0],&tempBuf[80]);
		}
		else
		{
			//AESDecrypt(&initVector[0], &initVector[0]);
			AESDecryptForUICC(&initVector[0], &initVector[0],&tempBuf[80]);
		}

		for(i = 0x00; i < 0x10; i++)
		{
			if(bMod == CBC_Decrypt)
			{
				//tmp = input[i + num];
				tmp = inputData[i];

				//output[i+num] = initVector[i] ^ resultData[i];
				outputData[i] = initVector[i] ^ resultData[i];

				resultData[i] = tmp;
			}
			else
			{
				//output[i+num]	= initVector[i];
				outputData[i]	= initVector[i];
			}
		}		

		//WriteEepromEX((UWORD)(output+num),outputData,16);	
		memCopyNoAtomic(output+num,outputData,16);
	}	
}

BYTE processAttribute(BYTE *Attributelist,WORD Attributelistlen,BYTE tlLength)
{

	UWORD xdata dataFileAddr;
	BYTE idata randomnum;
	BYTE  idata attibute;
	BYTE  idata attiLen;
	WORD idata offset;
	
	
	dataFileAddr = SelectTFFile(DF_PROFILECFG,EF_AUTHRECDATA);//modify by xieyong@20221227  EF_AUTHRECDATA放在DF_PROFILECFG下面,和之前关于EF_AUTHRECDATA的修改一致
	
	ecpMem_Set(tempBuf,0x00,sizeof(tempBuf));
	
	randomnum = 0x01;
	
	for(offset = 0;offset < Attributelistlen;)
	{		
		ReadEeprom(&attibute, (UWORD)(Attributelist+offset),sizeof(BYTE));
				
		switch(attibute)
		{
		  	case AT_RAND:				
				
				if(G_currentEAPContext.Eapsubtype == SIM_Challenge)
					{
						ReadEeprom(&randomnum,(UWORD)(Attributelist+offset+1),sizeof(BYTE));
					
						randomnum/=4;
						
						tempBuf[1] = randomnum;
					}

				  //ReadEeprom(&random[0],(UWORD)(Attributelist+offset+4),randomnum*16);
					ReadEeprom(&tempBuf[AT_RAND_OFFSET],(UWORD)(Attributelist+offset+4),randomnum*16);
				
				break;
			case AT_MAC:
			{				
				//HaveAtMacFlag = TRUE;
				tempBuf[2] = TRUE;
				
				//ReadEeprom(bakmac,(UWORD)(Attributelist+offset+4),16);
				ReadEeprom(&tempBuf[AT_MAC_OFFSET],(UWORD)(Attributelist+offset+4),16);
				
				//if(INS == AUTHENTICATE_ODD)
				//{					
				//	SetTFConst(dataFileAddr,tlLength+8+offset+4,0x00,16);
				//}
				//else
				//{
				//	SetTFConst(dataFileAddr,8+offset+4,0x00,16);					
				//}

				SetTFConst(dataFileAddr,((INS == AUTHENTICATE_ODD)?tlLength:0)+8+offset+4,0x00,16);

				break;
			}
			case AT_IV:
			{								
				ReadEeprom(&tempBuf[AT_IV_OFFSET],(UWORD)(Attributelist+offset+4),16);
				break;
			}
			case AT_RESULT_IND:
				G_currentEAPContext.EapHaveATResultInd = TRUE;
				break;	
			
			case AT_AUTN:
				//HaveAtAutnFlag = TRUE;
				tempBuf[3] = TRUE;			
			 
				ReadEeprom(&tempBuf[AT_AUTN_OFFSET],(UWORD)(Attributelist+offset+4),16);
			
				break;			
			
			case AT_CHECKCODE:
				break;
			default:
				if(attibute < 0x80)
				{
					reportEAP_ClintError(0x00);
					return FALSE;
				}
		}

		ReadEeprom(&attiLen,(UWORD)(Attributelist+offset+1),sizeof(BYTE));
		
		offset += (attiLen*4);
	}
	
	return TRUE;
}


BYTE processAttributeEncry(BYTE *Attributelist,WORD Attributelistlen,BYTE attiLen)
{
	BYTE  idata attibute[2],len;
	WORD idata offset;
	UWORD xdata fileaddr;
	WORD  xdata cardcounter;
	
	
	for(offset = 0;offset < Attributelistlen;)
	{
		ReadEeprom(&attibute[0],(UWORD)(Attributelist+offset),2);
		switch(attibute[0])
		{
			case AT_COUNTER:					
				
				ReadEeprom(&RecData[0],(UWORD)(Attributelist+offset+2),2);					
					
				//servecounter = (tempBuf[0]<<8)|tempBuf[1];	  									
				break;
			case AT_NONCE_S:				
					
				ReadEeprom(Nonce,(UWORD)(Attributelist+offset+4),16);
				
				break;
			
			case AT_NEXT_REAUTH_ID:

				fileaddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_ReID);
			
				//filesize = GetFileHeadWord(fileaddr,F_MSize_Offset);
				//filesize = GetTFSize(fileaddr);
				
				if(G_currentEAPContext.Eapsubtype == EAP_Reauthentication)
				{
					//ReadTFBuf(fileaddr,0,tempBuf,filesize);	

					//cardcounter = (tempBuf[tempBuf[1]+2+2]<<8)|tempBuf[tempBuf[1]+2+3];

					ReadTFBuf(fileaddr,1,&len,1);	

					cardcounter = ReadWORD(fileaddr+ sizeof(TranFileHead)+len+4);
			 	}
				else
				{
					cardcounter = 0x0001;
				}

				//if(filesize >= 0x100)
				//{
				//	filesize = 0xFF;
				//}
									
				//ecpMem_Set(tempBuf,0xFF,filesize);
				ClearFileConst(fileaddr,0xFF);
				
				tempBuf[0] = 0x80;	 //re_id				
			
				ReadEeprom(&tempBuf[1],(UWORD)(Attributelist+offset+3),sizeof(BYTE));
				
				ReadEeprom(&tempBuf[2],(UWORD)(Attributelist+offset+4),tempBuf[1]);
				
				//cardcounter = (tempBuf[tempBuf[1]+2+2]<<8)|tempBuf[tempBuf[1]+2+3];

				tempBuf[2+tempBuf[1]] = 0x81; //counter
				tempBuf[3+tempBuf[1]]= 0x02;
				
				//if(G_currentEAPContext.Eapsubtype == EAP_Reauthentication)
				//{
					tempBuf[4+tempBuf[1]] = (BYTE)(cardcounter>>8);
					tempBuf[5+tempBuf[1]] = (BYTE)cardcounter;	
				//}
				//else
				//{
					//tempBuf[tempBuf[1]+2+2]= 0x00;
					//tempBuf[tempBuf[1]+2+3]= 0x01;					
				//}
				WriteTFBuf(fileaddr,0,tempBuf,tempBuf[1]+6);

				G_currentEAPContext.EapHaveReauID = TRUE;									
				break;
			case AT_NEXT_PSEUDONYM:

				//haveidflag = TRUE;

				fileaddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_PSID);
			
// 				filesize = GetTFSize(fileaddr);

// 				if(filesize >= 0x100)
// 				{
// 					filesize = 0xFF;
// 				}
// 									
// 				ecpMem_Set(tempBuf,0xFF,filesize);
// 

				ClearFileConst(fileaddr,0xFF);
			
				ReadEeprom(&len,(UWORD)(Attributelist+offset+3),sizeof(BYTE));
				//ReadEeprom(tempBuf,(UWORD)(Attributelist+offset+4),len);

				//WriteTFBuf(fileaddr,0,tempBuf,len);
				WriteTFFileNonAtomic(fileaddr,0x00,(UWORD)(Attributelist+offset+4),len);
				
				G_currentEAPContext.EapHavePseuID = TRUE;
				break;
			case AT_PADDING:
				break;
			default:
				if(attibute[0] < 0x80)
				{
					reportEAP_ClintError(0x00);
					return FALSE;
				}
		}
		
		//ReadEeprom(&attiLen,(UWORD)(Attributelist+offset+1),sizeof(BYTE));
		
		offset+= (attibute[1]*4);
		
	}
	return TRUE;

}


void saveEAPKEYS(BYTE off)
{
	UWORD xdata tempFileAddr;
	
	tempFileAddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_EAPKEYS);

	if(tempFileAddr != FALSE)
	{
		RecData[0] = 0x80;
		RecData[1] = 0x40;
		ecpMem_Copy(&RecData[2],tempBuf+off,0x40);
		RecData[0x42] = 0x81;
		RecData[0x43] = 0x40;
		ecpMem_Copy(&RecData[0x44],tempBuf+off+64,0x40);
		WriteTFBuf(tempFileAddr,0,RecData,0x84);
	}
}
/*@Function:   processEAP_SIMChallenge                                            
 * 
 *@Descrip:   FULL 鉴权,EAP_SIM鉴权的子类型Challenge鉴权处理, 计算MK,及由MK分散K_encr和K_auth key,并拼装响应数据
 *                                                                                                                                     
 *@Param:    
		                                                                                                  
 *@Return:                                                                                                                                                                                                                                                             
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
void processEAP_Challenge(void)
{
	BYTE * xdata  Attributelist;
	BYTE idata result,randomnum;
	BYTE xdata bakmac[16],cardmac[20],random[48],sres[12],ik[16],ck[16],iv[16],kc[24];
	UWORD xdata dataFileAddr,tempFileAddr;
	WORD xdata Attributelistlen;
	BYTE  idata attiLen,tlLength,resLength;
	BYTE idata HaveAtAutnFlag,HaveAtMacFlag;
	BYTE idata addrMode;
	WORD idata offset,tempoffset;	
	BYTE  idata i;	
	
	G_currentEAPContext.EapAuthMode = EAP_FullAuth;
	G_currentEAPContext.EapHaveATResultInd = FALSE;	//不存在 AT_ResultInd是该变量为FALSE

	HaveAtAutnFlag = FALSE;
	HaveAtMacFlag = FALSE;
	//haveidflag = FALSE;
	tlLength = 0;
	resLength = 0;
	randomnum = 0x01;

 	dataFileAddr = SelectTFFile(DF_PROFILECFG,EF_AUTHRECDATA);//modify by xieyong@20221227  EF_AUTHRECDATA放在DF_PROFILECFG下面,和之前关于EF_AUTHRECDATA的修改一致

 	if(INS == AUTHENTICATE_ODD)
 	{		
 		tlLength = GetTLLength(dataFileAddr+sizeof(TranFileHead));
		
		Attributelistlen = GetDataLength(dataFileAddr+sizeof(TranFileHead)) - 8;
		
		addrMode = 0x01;
	}
	else	
	{
		tlLength = 0;

		Attributelistlen = P3 -8;
		
		addrMode = 0x00;

	}
	
	Attributelist = (BYTE *)(dataFileAddr + sizeof(TranFileHead)+ tlLength + 8); //指向第一个attribute

	result = processAttribute(Attributelist,Attributelistlen,tlLength);
	
	if(result == FALSE)
	{
		return;
	}
	
	if(G_currentEAPContext.Eapsubtype == AKA_Challenge)
	{
		if(tempBuf[2] == TRUE)
		{
			HaveAtMacFlag = TRUE;
		}
		
		if(tempBuf[3] == TRUE)
		{
			HaveAtAutnFlag = TRUE;
		}
			
	}
	else
	{
		randomnum = tempBuf[1];
	}
	
	
	ecpMem_Copy(&random[0],&tempBuf[AT_RAND_OFFSET],randomnum*16);
	
	ecpMem_Copy(bakmac,&tempBuf[AT_MAC_OFFSET],16);
	
	ecpMem_Copy(iv,&tempBuf[AT_IV_OFFSET],16);
	
	tempFileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum],EF_EAPAUTHDATA);
	
	if(tempFileAddr == FALSE)
	{
		reportEAP_ClintError(0x00);
		return;
	}
		
	if(G_currentEAPContext.Eapsubtype == AKA_Challenge)
	{
		if((HaveAtAutnFlag == FALSE)||(HaveAtMacFlag == FALSE))
		{
			reportEAP_ClintError(0x00);	
			return;	
		}
				
		ecpMem_Copy(&RecData[1],&tempBuf[AT_RAND_OFFSET],0x10);
		ecpMem_Copy(&RecData[18],&tempBuf[AT_AUTN_OFFSET],0x10);
		result = Milenage(&tempBuf[0]);
		resLength = RESLGTH;

		if(result == FALSE)
		{
			reportEAP_AKAAuthReject();
			return;
		}
		else if(result == 16)  //sqn同步
		{
	 /*
	DB
	0x0E +14字节auts
	*/
			
			reportEAP(AKA_SynchronizationFailure);
					
			offset = 0x08;

			RecData[offset] = AT_AUTS;
			RecData[offset+1] = 0x04;
			ecpMem_Copy(&RecData[offset+2],&tempBuf[2],14);

			RecData[3] = offset+ 16;

			G_ResponseLen = RecData[3];

			return;
		}

	 /*
	DB
	该函数输出格式为
	08 +8字节sres
	0x10+ 16字节ck
	0x10+16字节ik
	*/
		ecpMem_Copy(sres,&tempBuf[2],8);
		ecpMem_Copy(ck,&tempBuf[11],16);
		ecpMem_Copy(ik,&tempBuf[28],16);
		
		eap_AKACalculateMK(&ck[0],&ik[0],&ResData[MKOffset]);

		//fips186_2_prf(&ResData[MKOffset],20,tempBuf,160);
		
		ecpMem_Copy(K_encr,tempBuf,16);
		ecpMem_Copy(K_auth,tempBuf+16,16);
				
	}
	else
	{
		for(i=0;i<randomnum;i++)
		{			
			ecpMem_Copy(&RecData[1],&random[i*16],0x10);
			
			Milenage2G(&tempBuf[0]);
			
			ecpMem_Copy(&sres[i*4],&tempBuf[1],0x04);
						
			ecpMem_Copy(&kc[i*8],&tempBuf[6],0x08);
			
		}		
	

		ReadTFBuf(tempFileAddr,NonceOffset,Nonce,16);
		
		eap_SIMCalculateMK(kc,&ResData[MKOffset]);
		
	}	

	fips186_2_prf(MK,20,tempBuf,160);
	
	ecpMem_Copy(K_encr,tempBuf,16);
	ecpMem_Copy(K_auth,tempBuf+16,16);

	WriteTFBuf(tempFileAddr,MKOffset,MK,20);
	WriteTFBuf(tempFileAddr,K_encrOffset,tempBuf,0x20);

	saveEAPKEYS(0x20);	
		
	if(G_currentEAPContext.Eapsubtype == SIM_Challenge)
	//the MAC code is calculated over the following data: EAP packet| NONCE_MT
	{
		if(INS == AUTHENTICATE_ODD)
		{		
			WriteTFBuf(dataFileAddr,G_BlockOffset,Nonce,16);

		//HMAC_SHA1(K_auth,16,Attributelist-8,G_EapBlockOffset-tlLength+16,cardmac);
		//HMACSha1DoFinalSingleCall(K_auth,16,Attributelist-8,G_BlockOffset-tlLength+16,cardmac);
			GeneralHMAC(K_auth,16,Attributelist-8,G_BlockOffset-tlLength+16,cardmac, SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);
			
		}
		else
		{		
			WriteTFBuf(dataFileAddr,P3,Nonce,16);
		
		//HMAC_SHA1(K_auth,16,Attributelist-8,P3+16,cardmac);
		//HMACSha1DoFinalSingleCall(K_auth,16,Attributelist-8,P3+16,cardmac);
			GeneralHMAC(K_auth,16,Attributelist-8,P3+16,cardmac,SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);
		}
	}
	else
	{
		if(INS == AUTHENTICATE_ODD)
		{			
			//HMAC_SHA1(K_auth,16,Attributelist-8,G_BlockOffset-tlLength,cardmac);
			//HMACSha1DoFinalSingleCall(K_auth,16,Attributelist-8,G_BlockOffset-tlLength,cardmac);
			GeneralHMAC(K_auth,16,Attributelist-8,G_BlockOffset-tlLength,cardmac,SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);
		}
		else
		{
			//HMAC_SHA1(K_auth,16,Attributelist-8,P3,cardmac);
			//HMACSha1DoFinalSingleCall(K_auth,16,Attributelist-8,P3,cardmac);
			GeneralHMAC(K_auth,16,Attributelist-8,P3,cardmac,SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);
		}
	}
	
	if(ecpMem_Compare(cardmac,bakmac,16) != EQUAL)
	{
		reportEAP_ClintError(0x00);
		return;
	}
	
	tempoffset = GetEapAttribute(Attributelist,Attributelistlen,AT_ENCR_DATA);//校验完MAC后处理密文数据

	if(tempoffset != 0xFF)
	{				
		ReadEeprom(&attiLen,(UWORD)(Attributelist+tempoffset+1),1);
		
		AESArithFunction(&iv[0], &Attributelist[tempoffset+4], K_encr, &Attributelist[tempoffset+4], CBC_Decrypt, (attiLen-1)*4, addrMode);	//解密
		
		Attributelistlen = (attiLen-1)*4;

		Attributelist = Attributelist+tempoffset+4;//密文数据中的AT

		result = processAttributeEncry(Attributelist,Attributelistlen,attiLen);
		
		if(result == FALSE)
		{
			return;
		}
 	}

	reportEAP(G_currentEAPContext.Eapsubtype);
	
	offset = 0x08;
	
	if(G_currentEAPContext.Eapsubtype == AKA_Challenge)
	{
		RecData[offset] = AT_RES;	
		RecData[offset+2] = 0x00;
		RecData[offset+3] = resLength*8;
		ecpMem_Copy(&RecData[offset+4],sres,resLength);
		
		if(resLength%4 !=0x00)
		{
			ecpMem_Set(&RecData[offset+4+resLength],0,(4-resLength%4));
			resLength = resLength+(4-resLength%4); 
		}
		RecData[offset+1] = (4+resLength)/4;

		offset+= 4+resLength;
	}
	
	if(G_currentEAPContext.EapHaveATResultInd == TRUE)
	{
		RecData[offset] = AT_RESULT_IND;
		RecData[offset+1] = 0x01;
		RecData[offset+2] = RecData[offset+3] = 0x00;

		offset+= 0x04;
	}
	RecData[offset] = AT_MAC;
	RecData[offset+1] = 0x05;
	RecData[offset+2] = RecData[offset+3] = 0x00;

	RecData[3] = offset+20;//总长度

	if(G_currentEAPContext.Eapsubtype == SIM_Challenge)
	{
		ecpMem_Copy(&RecData[offset+20],&sres[0],12);
	//HMAC_SHA1(K_auth,16,RecData,offset+20+12,cardmac);
	//HMACSha1DoFinalSingleCall(K_auth,16,RecData,offset+20+12,cardmac);	
		GeneralHMAC(K_auth,16,RecData,offset+20+12,cardmac,SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);
	}
	else
	{
		//HMAC_SHA1(K_auth,16,RecData,offset+20,cardmac);
		//HMACSha1DoFinalSingleCall(K_auth,16,RecData,offset+20,cardmac);
		GeneralHMAC(K_auth,16,RecData,offset+20,cardmac,SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);
	}
	ecpMem_Copy(&RecData[offset+4],cardmac,16);
	G_ResponseLen = RecData[3];	
	
	ODDResponse();
}


/*@Function:   processEAP_Reauthencation                                            
 * 
 *@Descrip:   快速鉴权,EAP鉴权的子类型Reauthencation鉴权处理, 在上次FULL鉴权的基础上,使用上次FULL鉴权的MK/K_encr/K_auth进行鉴权,并拼装响应数据
 *                                                                                                                                     
 *@Param:    
		                                                                                                  
 *@Return:                                                                                                                                                                                                                                                             
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
void processEAP_Reauthencation(void)
{
	BYTE  * xdata  Attributelist;
	BYTE  xdata bakmac[16],cardmac[20],iv[16];
	WORD  xdata servecounter,cardcounter;
	UWORD xdata fileaddr,dataFileAddr,tempFileAddr;
	BYTE  idata attiLen,tlLength;
	WORD  idata Attributelistlen,offset,tempoffset;
	BYTE  idata addrMode,result,len;

	tlLength = 0;
	servecounter = 0;

	fileaddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_ReID);
	//filesize = GetFileHeadWord(fileaddr,F_MSize_Offset);

	//ReadTFBuf(fileaddr,0,tempBuf,sizeof(tempBuf));	

	//cardcounter = (tempBuf[tempBuf[1]+4]<<8)|tempBuf[tempBuf[1]+5];
	
	ReadTFBuf(fileaddr,1,&len,1);	

	cardcounter = ReadWORD(fileaddr+ sizeof(TranFileHead)+len+4);

	dataFileAddr = SelectTFFile(DF_PROFILECFG,EF_AUTHRECDATA);//modify by xieyong@20221227  EF_AUTHRECDATA放在DF_PROFILECFG下面,和之前关于EF_AUTHRECDATA的修改一致

	if(INS == AUTHENTICATE_ODD)
	{	
		tlLength = GetTLLength(dataFileAddr+sizeof(TranFileHead));
		Attributelistlen = GetDataLength(dataFileAddr+sizeof(TranFileHead)) - 8;
		addrMode = 0x01;
	}
	else	
	{
		tlLength = 0;
		Attributelistlen = P3 -8;
		addrMode = 0x00;
	}

	Attributelist = (BYTE *)(dataFileAddr + sizeof(TranFileHead)+ tlLength + 8); //指向第一个attribute
	
	G_currentEAPContext.EapAuthMode = EAP_FastAuth;
	G_currentEAPContext.EapHaveATResultInd = FALSE;	//不存在 AT_ResultInd是该变量为FALSE
	G_currentEAPContext.EapHaveReauID = FALSE;	 //快速鉴权id只能使用一次

	result = processAttribute(Attributelist,Attributelistlen,tlLength);
	
	if(result == FALSE)
	{
		return;
	}
	
	ecpMem_Copy(bakmac,&tempBuf[AT_MAC_OFFSET],16);
	
	ecpMem_Copy(iv,&tempBuf[AT_IV_OFFSET],16);
	tempFileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum],EF_EAPAUTHDATA);
	if(tempFileAddr != FALSE)
	{
		ReadTFBuf(tempFileAddr,MKOffset,&ResData[MKOffset],52);
	}	
	
	//the MAC code is calculated over the following data: EAP packet| NONCE_MT
	if(INS == AUTHENTICATE_ODD)
	{						
		//HMAC_SHA1(K_auth,16,Attributelist-8,G_EapBlockOffset-tlLength,cardmac);
		//HMACSha1DoFinalSingleCall(K_auth,16,Attributelist-8,G_BlockOffset-tlLength,cardmac);
		GeneralHMAC(K_auth,16,Attributelist-8,G_BlockOffset-tlLength,cardmac,SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);
	}
	else
	{
		//HMAC_SHA1(K_auth,16,Attributelist-8,P3,cardmac);//reauth 的mac不填充nonce
		//HMACSha1DoFinalSingleCall(K_auth,16,Attributelist-8,P3,cardmac);
		GeneralHMAC(K_auth,16,Attributelist-8,P3,cardmac,SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);
	}
	
	if(ecpMem_Compare(cardmac,bakmac,16) != EQUAL)
	{
		reportEAP_ClintError(ERROR_NeedfreshRand);
		return;
	}

	tempoffset = GetEapAttribute(Attributelist,Attributelistlen,AT_ENCR_DATA);//校验完MAC后处理密文数据

	if(tempoffset != 0xFF)
	{
		ReadEeprom(&attiLen,(UWORD)(Attributelist+tempoffset+1),sizeof(BYTE));
		
		AESArithFunction(&iv[0],&Attributelist[tempoffset+4],K_encr,&Attributelist[tempoffset+4],CBC_Decrypt,(attiLen-1)*4,addrMode);	//解密		
		
		Attributelistlen = (attiLen-1)*4;

		Attributelist = Attributelist+tempoffset+4;//密文数据中的AT
		
		result = processAttributeEncry(Attributelist,Attributelistlen,attiLen);
		
		if(result == FALSE)
		{
			return;
		}		
		servecounter = (RecData[0]<<8)|RecData[1];				
	}
	
	
	eap_SIMCalcReauthXKEY(MK,servecounter,cardmac);//cardmac???

	fips186_2_prf(cardmac,20,tempBuf,160);
	
	saveEAPKEYS(0x00);	
	
	reportEAP(G_currentEAPContext.Eapsubtype);

	RecData[8] = AT_IV;
	RecData[9] = 0x05;
	RecData[10]= RecData[11]= 0x00;
	Random_Number_Generator_Get(&iv[0],16);
	ecpMem_Copy(&RecData[12],&iv[0],16);

	RecData[28] = AT_ENCR_DATA;
	RecData[29] = 0x05;
	RecData[30] = 0x00;
	RecData[31]	= 0x00;

	tempBuf[0]= AT_COUNTER;
	tempBuf[1]= 0x01;
	tempBuf[2]= (BYTE)(servecounter>>8);
	tempBuf[3]=	(BYTE)servecounter;

    if(servecounter >= cardcounter)
	{
		ReadTFBuf(fileaddr,1,&tempBuf[99],1);
		tempBuf[100] = 0x81;
		tempBuf[101] = 0x02;
		tempBuf[102]= (BYTE)(servecounter>>8);
		tempBuf[103]= (BYTE)servecounter;
		WriteTFBuf(fileaddr,tempBuf[99]+2,&tempBuf[100],4);//更新卡内的counter

		tempBuf[4]= AT_PADDING;
		tempBuf[5]= 0x03;
		ecpMem_Set(&tempBuf[6],0x00,10);
	}
	else
	{
		tempBuf[4]= AT_COUNTER_TOO_SMALL;
		tempBuf[5]= 0x01;
		tempBuf[6]= 0x00;
		tempBuf[7]=	0x00;
		tempBuf[8]= AT_PADDING;
		tempBuf[9]= 0x02;
		ecpMem_Set(&tempBuf[10],0x00,6);
	}

	AESArithFunction(&iv[0],&tempBuf[0],K_encr,&RecData[32],CBC_Encrypt,16,addrMode);	

	offset = 48;

	if(G_currentEAPContext.EapHaveATResultInd == TRUE)
	{
		RecData[offset] = AT_RESULT_IND;
		RecData[offset+1] = 0x01;
		RecData[offset+2] = RecData[offset+3] = 0x00;

		offset+= 0x04;
	}

	RecData[offset] = AT_MAC;
	RecData[offset+1]	= 0x05;
	ecpMem_Set(&RecData[offset+2],0x00,18);
	ecpMem_Copy(&RecData[offset+2+18],Nonce,16);
	RecData[3] = offset+2+18;

	//HMAC_SHA1(K_auth,16,RecData,offset+2+18+16,cardmac);
	//HMACSha1DoFinalSingleCall(K_auth,16,RecData,offset+2+18+16,cardmac);
	GeneralHMAC(K_auth,16,RecData,offset+2+18+16,cardmac,SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);
	ecpMem_Copy(&RecData[offset+4],cardmac,16);

	G_ResponseLen = RecData[3];
	
	ODDResponse();	
}
/*@Function:   processEAP_Notification                                            
 * 
 *@Descrip:   EAP鉴权的SIM/AKA子类型Notification的处理
 *                                                                                                                                     
 *@Param:    
		                                                                                                  
 *@Return:                                                                                                                                                                                                                                                             
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
void processEAP_Notification(void)
{
	BYTE xdata cardmac[20],bakmac[16],iv[16];
	BYTE * xdata  Attributelist;
	BYTE idata Attributelistlen,offset,len;
	BYTE idata notificationSbit,notificationPbit;
	BYTE idata addrMode;
	WORD xdata notificationcode,cardcounter;
	UWORD xdata fileaddr,tempFileAddr;

	notificationSbit = FALSE;
	notificationPbit = FALSE;
	notificationcode = 0x0000;

	Attributelist = RecData+8;//从偏移8开始为ATlist
	Attributelistlen = P3 -8;
	addrMode = 0x00;										//表示Attributelist所指向的地址为RAM地址
	
	for(offset = 0;offset < Attributelistlen;offset+= Attributelist[offset+1]*4)
	{	
		switch(Attributelist[offset])
		{
			case AT_NOTIFICATION:
				if((Attributelist[offset+2]&0x80) == 0x80)
				{
					notificationSbit = TRUE;
				}
				if((Attributelist[offset+2]&0x40) == 0x40)
				{
					notificationPbit = TRUE;
				}
				notificationcode = (Attributelist[offset+2]<<8)|Attributelist[offset+3];
				notificationcode = notificationcode&0x3FFF;
				break;
			case AT_MAC:
			{
				ecpMem_Copy(bakmac,&Attributelist[offset+4],16);
				ecpMem_Set(&Attributelist[offset+4],0x00,16);
				break;
			}				
			default:
				if(Attributelist[offset] < 0x80)
				{
					reportEAP_ClintError(0x00);
					return;
				}
		}
	}

	if((G_currentEAPContext.EapHaveATResultInd == FALSE)&&(notificationSbit == TRUE))  //没收到Result_ind ,但是收到sucess，报错
	{
		reportEAP_ClintError(0x00);
		return;
	}
	if(notificationSbit == FALSE)
	{
		//the S bit set to zero (code values 0...32767) are used on unsuccessful cases
		fileaddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_EAPSTATUS);
		SetTFConst(fileaddr,0,EAP_AuthFailure,1);

		notifyEAP_Result(notificationcode);
		return;
	}
	
	tempFileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum],EF_EAPAUTHDATA);
	if(tempFileAddr != FALSE)
	{
		ReadTFBuf(tempFileAddr,K_encrOffset,&ResData[K_encrOffset],32);
	}	
	
	//HMAC_SHA1(K_auth,16,RecData,P3,cardmac);
	//HMACSha1DoFinalSingleCall(K_auth,16,RecData,P3,cardmac);
	GeneralHMAC(K_auth,16,RecData,P3,cardmac, SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);

	if(ecpMem_Compare(cardmac,bakmac,16) != EQUAL)
	{
		reportEAP_ClintError(ERROR_NeedfreshRand);
		return;
	}

	reportEAP(G_currentEAPContext.Eapsubtype);
	offset = 0x08;

	if((notificationPbit == FALSE)&&(G_currentEAPContext.EapAuthMode == EAP_FastAuth))
	{
		fileaddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_ReID);
// 		filesize = GetTFSize(fileaddr);
// 	
// 		if(filesize >= 0x100)
// 		{
// 			filesize = 0xFF;
// 		}
// 	
// 		ReadTFBuf(fileaddr,0,tempBuf,filesize);	

// 		cardcounter = 	(tempBuf[tempBuf[1]+2+2]>>8)|tempBuf[tempBuf[1]+2+3];

		ReadTFBuf(fileaddr,1,&len,1);	

		cardcounter = ReadWORD(fileaddr+ sizeof(TranFileHead)+len+4);

		RecData[offset] = AT_IV;
		RecData[offset+1] = 0x05;
		RecData[offset+2]= RecData[offset+3]= 0x00;
		Random_Number_Generator_Get(&iv[0],16);
		ecpMem_Copy(&RecData[offset+4],&iv[0],16);
		
		offset+= 20;

		RecData[offset] = AT_ENCR_DATA;
		RecData[offset+1] = 0x05;
		RecData[offset+2] = RecData[offset+3]= 0x00;

		
		tempBuf[0]= AT_COUNTER;
		tempBuf[1]= 0x01;
		tempBuf[2]= (BYTE)(cardcounter>>8);
		tempBuf[3]=	(BYTE)cardcounter;

		tempBuf[4]= AT_PADDING;
		tempBuf[5]= 0x03;
		ecpMem_Set(&tempBuf[6],0x00,10);
		
		AESArithFunction(&iv[0],&tempBuf[0],K_encr,&RecData[offset+4],CBC_Encrypt,16,addrMode);	//??

		offset+= 20;		
	}

	RecData[offset] = AT_MAC;
	RecData[offset+1] = 0x05;
	RecData[offset+2]= RecData[offset+3]= 0x00;
	ecpMem_Set(&RecData[offset+4],0x00,16);

	RecData[3] = offset+20;

	//HMAC_SHA1(K_auth,16,RecData,offset+20,cardmac);
	//HMACSha1DoFinalSingleCall(K_auth,16,RecData,offset+20,cardmac);
		GeneralHMAC(K_auth,16,RecData,offset+20,cardmac,SHA1_DIGEST_SIZE, ALG_HMAC_SHA1);
	
	ecpMem_Copy(&RecData[offset+4],cardmac,16);

	G_ResponseLen = RecData[3];	
}

/*@Function:   EAPAuthenticate                                            
 * 
 *@Descrip:   EAP鉴权的处理，包括EAP_SIM/EAP_AKA  的分类判断处理
 *                                                                                                                                     
 *@Param:    
		                                                                                                  
 *@Return:    TRUE：鉴权成功/FALSE：鉴权失败  /3:结束本次鉴权 且返回响应的状态字  /4:缓存鉴权数据的文件大小小于 鉴权数据                                                                                                                                                                                                                                                       
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
BYTE EAPAuthenticate(void)
{
	UWORD idata fileaddr,receiveAddr;
	BYTE  idata result,tlLength;
	BYTE  idata mesIndex,codeType,eapType,subType;

	receiveAddr = SelectTFFile(DF_PROFILECFG,EF_AUTHRECDATA);//modify by xieyong@20221227  EF_AUTHRECDATA放在DF_PROFILECFG下面,和之前关于EF_AUTHRECDATA的修改一致
	
	if(INS == AUTHENTICATE_ODD)
	{		
		result = ODDAuth();
		
		if(result == 3)
		{
			return result;
		}
		
		tlLength = GetTLLength(receiveAddr+sizeof(TranFileHead));
		
		receiveAddr += sizeof(TranFileHead)+tlLength;
		
		ReadEeprom(&codeType, receiveAddr,0x01);
		ReadEeprom(&mesIndex, receiveAddr+0x01,0x01);
		ReadEeprom(&eapType, receiveAddr+0x04,0x01);
		ReadEeprom(&subType, receiveAddr+0x05,0x01);

	}
		
	if(INS == 0x88)
	{				
		if(receiveAddr != FALSE )
		{
			if(GetTFSize(receiveAddr) < (P3 + RANDOMSIZE))
			{
				GlobalSetStatus(0x6A84);
				return 3; //文件大小增加的一个模式
			}

			WriteTFBuf(receiveAddr,0x00,&RecData[0],P3);
		}
		codeType = RecData[0];
		mesIndex = RecData[1];		
		eapType = RecData[4];
		subType = RecData[5];
	}

 	if(G_currentEAPContext.Messageindex == mesIndex)
	{
		G_currentEAPContext.EapResendFlag = TRUE;
	}
	else
	{
		G_currentEAPContext.EapResendFlag = FALSE;
	}

 	G_currentEAPContext.Messageindex = mesIndex;

	fileaddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_EAPSTATUS);

	if(codeType == EAP_SUCESS)
	{
		SetTFConst(fileaddr,0,EAP_AuthSucess,1);
		return TRUE;
	}
	else if(codeType == EAP_FAILURE)
	{
		SetTFConst(fileaddr,0,EAP_AuthFailure,1);
		return TRUE;
	}

	if(codeType != EAP_REQUEST)//报文第一字节不是request
	{
		GlobalSetStatus(0x6200);
		return 3;
	}

	SetTFConst(fileaddr,0,EAP_Authenticating,1);

	if(eapType == EAP_Identity)
	{
		processEAP_Identity();
		return TRUE;
	}	

	if(eapType != G_currentEAPContext.Eaptype)
	{
		reportEAP_ClintError(ERROR_UnknowPacket);
		return TRUE;
	}

	G_currentEAPContext.Eapsubtype = subType;

	if(eapType == EAP_SIM)
	{
		switch(G_currentEAPContext.Eapsubtype)
		{
			case SIM_Start:
			{
				processEAP_SIMStart();
				break;
			}
			case SIM_Challenge:
			{
				processEAP_Challenge();
				break;
			}
			case EAP_Reauthentication:
			{
				processEAP_Reauthencation();
				break;
			}
			case EAP_Notification:
			{
				processEAP_Notification();
				break;
			}
			default:
				reportEAP_ClintError(0x00);
		}
	}
	else  if(eapType == EAP_AKA)
	{
		switch(G_currentEAPContext.Eapsubtype)
		{
			case AKA_Identity:
			{
				processEAP_AKAIdentity();
				break;
			}
			case AKA_Challenge:
			{
				processEAP_Challenge();
				break;
			}
			case EAP_Reauthentication:
			{
				processEAP_Reauthencation();
				break;
			}
			case EAP_Notification:
			{
				processEAP_Notification();
				break;
			}
			default:
				reportEAP_ClintError(0x00);
		}		
	} 
	//When all segments of the authentication data are received, the UICC answer with SW1 SW2 '62 F3'.
	if(INS == AUTHENTICATE_ODD)
	{
		GlobalSetStatus(0x62F3);
		return 3;
	}

	return TRUE;
}

/*
*@function   : IF_SelectDFFile
*
*@description:用于在成功选择指定ADF后，SetChannnel完成之前
*
*@param      :void
*
*@return     :false:继续执行后续操作，如setchannel等/TRUE：中止后续函数流程
*
*@history    :V1.0 :create by xuyz@20131129
*/
BYTE IF_SelectDFFile(BYTE dirNo)
{
#if 0
	UWORD idata fileAddr;
	BYTE  idata a0Length,offset80,offset81,offset,i;
	
	fileAddr = SelectLFFile(DF_MF,0x2F00);

	ReadLFBuf(fileAddr,dirNo,0x00,&tempBuf[0],GetLFRecNumLgth(fileAddr,FileLgth));
	
	if(tempBuf[0] != 0x61)
	{
		return FALSE;
	}
	
	if((tempBuf[2+tempBuf[1]]!=0x73)||(tempBuf[2+tempBuf[1]+2]!=0xA0))  //Discretionary template tag / EAP Application tag
	{
		return FALSE;			
	}

	a0Length = tempBuf[2+tempBuf[1]+2+1];       //A0 length
	offset = 2+tempBuf[1]+2+2;  //point to Application EAP supported types list tag "80"

	offset80 = FindTag(&tempBuf[offset],0x80,a0Length);

	offset81 = FindTag(&tempBuf[offset],0x81,a0Length);

	if((offset80 == FALSE)||(offset81 == FALSE))
	{
		return FALSE;
	}

	if(tempBuf[offset+offset81] != (tempBuf[offset+offset80]*2))          //81 data length是 80 data length 的2倍
	{
		return FALSE;
	}
#endif
	BYTE  idata offset80,offset81,offset,i;

	if(SelectDFFilePublic(dirNo,tempBuf,&offset,&offset80,&offset81) == FALSE)
	{
		return FALSE;
	}

	for(i=0;i<tempBuf[offset+offset81];i+=2)
	{
		if(G_CurrentDfName[G_ChannelNum] ==((tempBuf[offset+offset81+1+i]<<8)|(tempBuf[offset+offset81+1+i+1])))
		{
			//G_currentEAPContext.Eaptype = tempBuf[offset+offset80+1+(i/2)];	 //0x12/0x17
			return tempBuf[offset+offset80+1+(i/2)];
		}
	}
	
	return FALSE;
}
/*@Function:   EAPAuthentication                                            
 * 
 *@Descrip:   EAP鉴权的主流程,判断当前是否是EAP鉴权,如果是，则执行EAP鉴权，如果否，则返回FALSE
 *                                                                                                                                     
 *@Param:    
		                                                                                                  
 *@Return:    TRUE：EAP鉴权完成/FALSE：不是EAP鉴权                                                                                                                                                                                                                                                      
 * 
 *@Author:     
 *  
 *@History:    V1.0
 *                        
 */
BYTE EAPAuthentication(void)
{
	BYTE idata result,type,dirNo;	
	
	tempBuf[0] = GetCurrentADFAid(G_CurrentAdfName[G_ChannelNum],&tempBuf[1]);
	
	dirNo = GetAIDNumber(&tempBuf[2],FALSE);//TODO:和陈涛确认，第二个参数

	if(dirNo != FALSE)
	{
		type = IF_SelectDFFile(dirNo);
		
		if(type != FALSE)
		{
			G_currentEAPContext.Eaptype = type;
			
			result = EAPAuthenticate();
			
			return result;	
		}

	}	
		
return FALSE;	
}

