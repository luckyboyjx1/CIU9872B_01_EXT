/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
			V1.1 增加ISIM鉴权功能，需定义宏(#define ISIM) --xuqs @20140408

 */
#include	"string.h"
#include 	"uicc_main.h"
//#include 	"main.h"
#include 	"fileSystem.h"
#include 	"fileAction.h"
#include 	"gsmpublic.h"
#include 	"CDMA.h"
//#include 	"resume.h"
#include 	"Authentication.h"
#include    "stkhead.h"
#include 	<string.h>			 
#include <uicc_error.h>
#include "EUICCInterface.h"
#include "profileApi.h"
#include "tuak.h"
#include "Mbms.h"
//#include "sha1.h"
#include "crypto_impl.h"
#include "profile.h"
#include "uicc_interface.h"

#if UICC_FUCTION_PRIVATEAUTH
#include "PrivateAuthentication.h"
#endif
#if UICC__AUTH__EAP
#include "EapAuthentication.h"
#endif
#if COS_UPDATE_ONLINE_MUTUAL
#include "UpCos.h"
#endif
//BYTE xdata G_Auth_Rand[16];			//缓存GBA Bootstrapping Mode接收的鉴权随机数
//BYTE xdata G_Auth_Ks[33];			//缓存GBA Bootstrapping Mode计算出的Ks = (CK || IK)
BYTE xdata G_GBA_AUTH_Status ;//= 0;	//GBA Bootstrapping Mode鉴权状态：0x00未进行该模式鉴权，或者鉴权失败; 0x01该模式鉴权成功
BYTE xdata G_Auth_Rand[17];			//缓存GBA Bootstrapping Mode接收的鉴权随机数
BYTE xdata G_Auth_Ks[33];			//缓存GBA Bootstrapping Mode计算出的Ks = (CK || IK)

BYTE xdata G_ResponseLen;

#if UICC__AUTH__COUNTER
//BYTE xdata roundKeys[11][4][4];		//用于AES的Key
BYTE AuthCounter()
{
	UWORD xdata fileAddr,counter;
	
	BYTE xdata authCounter[4];
	
	if(G_CommandMode == Command_Mode_2G)   // add by chentao@20221207 V1/V2/V3鉴权counter是用的USIM下的Counter文件
	{
		fileAddr = SelectLFFile(DF_MF,G_DF_ID_USIM);  

		fileAddr = SelectFile(fileAddr,EF_AuthMaxCounter);
	}
	else
	{

		fileAddr=SelectTFFile(G_CurrentAdfName[G_ChannelNum],EF_AuthMaxCounter);
	}
		
	if(fileAddr == FALSE)
	{
		return TRUE;
	}
	
	ReadTFBuf(fileAddr,0x00,&authCounter[0],4);	
	
	counter = BuildUWord(&authCounter[0]);
	
	if(counter == 0xFFFFFFFF)
	{
		return TRUE;
	}
		
	if(counter == 0)
	{
		return FALSE;
	}
	else
	{
		counter--;
		
		if(counter == 0)  //WQ_Offline_profile_Test//TC_4_2_5_PE_AKAParameters_authCounterMax_ok
		{
			return FALSE;
		}
	
		SetUWord(&authCounter[0], counter);
		
		WriteTFBuf(fileAddr,0x00,&authCounter[0],4);
		
		return TRUE;
	}
}
#endif
#if(!UICC__GSM_2G_NOT_SUPPORT)
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 RunGsmAlgorithm2G
  
  Command format:    A0    88    00   00   10
  
  Data    content:   Random                                                       
                                                                                                                                                
  Description:       This function add the value given by the ME to the value of the last increased/updated record of the current cyclic EF

  Coder:             Caio         Tester

  Status:			 

 -------------------------------------------------------------------------------------------------------------------*/    

void RunGsmAlgorithm2G(void)
{
   	UWORD xdata keyAddr,adfUSIMAddr;
	BYTE idata algorithmID;

	if(P1P2P3Check(0x10,FALSE) ==FALSE)  
	{
		return;
	}
	
	if((G_CurrentDfName[G_ChannelNum]&0xFFFE) != 0x7F20)
    {
		FileStructureError();							//send 94 08
		return;
	}                                           		

	if(ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE)
    {
		SecuritySatisfied();
		
		return;
	}  
	
	adfUSIMAddr = SelectLFFile(DF_MF,G_DF_ID_USIM);

	keyAddr = SelectFile(adfUSIMAddr,EF_KI);
  
	if( keyAddr == FALSE )
    {													//send 6f 00(no find ki)
		TechnicalProblem();
		return;
	} 

#if UICC__AUTH__COUNTER
	if(AuthCounter() == FALSE)
	{
		return;
	} 
#endif

	ReceiveData();

	ecpMem_Move(&RecData[0x01], &RecData[0x00], 0x10);

#if UICC__AUTH__TUAK
	algorithmID = getAlgorithmID(adfUSIMAddr);

	if(algorithmID == 2)
	{
		if(TUAK2G() == FALSE)
		{
			AuthenticateError();
		}
		else
		{
			ecpMem_Move(&RecData[0x00], &RecData[0x01], 4);

			ecpMem_Move(&RecData[0x04], &RecData[0x06], 8);
			
			CommandFinish(0x0C);
		}
	}
	else
#endif
	{	
		if(Milenage2G(&tempBuf[0]) == FALSE)
		{
			AuthenticateError();
			
			#if UICC__FUNCTION_CMCCAPI
			RAM_Online_Flag = 0x00;
			#endif
		}
		else
		{
			ecpMem_Move(&RecData[0x00], &tempBuf[0x01], 4);

			ecpMem_Move(&RecData[0x04], &tempBuf[0x06], 8);
			
		#if UICC__FUNCTION_CMCCAPI
		    RAM_Online_Flag = 0x01;
        #endif	
		}
		
		CommandFinish(0x0C);
	}
}
#endif
#if UICC_FUCTION_PRIVATEAUTH
BYTE PrivateAuthentication(BYTE mode)
{
	BYTE  idata result;
	UWORD xdata fileAddr,adfAddr;

	if(G_CurrentAdfName[G_ChannelNum] == FALSE)//2g
	{
		adfAddr = SelectLFFile(DF_MF,G_DF_ID_USIM);
	}
	else
	{
		adfAddr = SelectLFFile(DF_MF,G_CurrentAdfName[G_ChannelNum]);
	}

	fileAddr = SelectFile(adfAddr, EF_KI);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadLFBuf(fileAddr, 0x01, 0x00, &RecData[100], 16);			//读出鉴权的KI

	fileAddr = SelectFile(adfAddr, EF_PRIVATE_AUTHCFG);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadTFBuf(fileAddr,0x00, &RecData[116], 111);			//读出鉴权用的常数字段Opmode,OP,R,C等
	
	result = PriAuthFunctionList.PriUSIMAuthenticate_Process_entry(mode,RecData);

	return result;
}
#endif

#if UICC__AUTH__EPS
/*                                                                                     
 *@Function name :	 EPSKeyDerivation
 *					 
 *@Description: derived key = HMAC-SHA-256 ( Key , S )
				String S shall be constructed from n input parameters as follows:
				S = FC || P0 || L0 || P1 || L1 ... || Pn || Ln
				where
					-	FC = 0x10,
					-	P0 = SN id,
					-	L0 = length of SN id (i.e. 0x00 0x03),
					-	P1 = SQN ? AK
					-	L1 = length of SQN ? AK (i.e. 0x00 0x06)
					The input key Key shall be equal to the concatenation CK || IK of CK and IK.     
  *
  *@Coder: 			 yjr            
  *
  *@Status:			   
*/
void EPSKeyDerivation(void)
{
	UWORD idata localAddr,epsNSCAddr; 

	tempBuf[50] = 0x10;	   			//FC固定值为0x10

	localAddr = SelectTFFile(G_DF_ID_USIM,EF_LOCI);

	ReadTFBuf(localAddr,4,(BYTE *)&tempBuf[51],3);

	tempBuf[54] = 0x00;
	tempBuf[55] = 0x03;

	memcpy((BYTE *)&tempBuf[56], (BYTE *)&SQNAK[0],6);		//SQNAK值从前面Milenage函数中得到
	  
	tempBuf[62] = 0x00;
	tempBuf[63] = 0x06;

	tempBuf[100] = 0x81;
	tempBuf[101] = 0x20;
	
	memcpy(&tempBuf[27], &tempBuf[28],16);
	
    //HMAC_SHA256(&tempBuf[11], 0x20, &tempBuf[50], 0x0E, &tempBuf[102]);    //CKIK的值从鉴权计算结果中得到
	HMACSha256DoFinalSingleCall(&tempBuf[11],0x20,&tempBuf[50],0x0E,&tempBuf[102]);
   
	epsNSCAddr = SelectLFFile(G_DF_ID_USIM,EF_EPSNSC);

	if(epsNSCAddr!= FALSE)
	{
		WriteLFBuf(epsNSCAddr, 0x01, 0x05,(BYTE *)&tempBuf[100],0x22);
	}
	
}
#endif

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 UsimAuthentication
  
  Command format:    00    88   00   XX   XX
  
  Data    content:   UICC authentication data from ME                                                       
                                                                                                                                                
  Description:       This function is for UICC authentication

  Coder:                     Tester:
  
  Status:			   

  modify by chentao@20211111 GBA鉴权参数ISIM和USIM分开存储（YJR确认）
 -------------------------------------------------------------------------------------------------------------------*/

void UsimAuthentication(void)
{
	BYTE  idata result,status,algorithmID,tag,flag,gbaAuthMode,fileStatus,recLen,BPLen;
	UWORD xdata fileAddr,adfAddr,fileAddrbp,fileAddrKs,fileGBAModeAddr,fileAddr1;

	//"a particular USIM application or any sub-directory must be
	//selected as the Current Directory" --TS31122
	fileAddrKs = FALSE;
	
	adfAddr = CheckCurrentDfForADF();

	if(adfAddr == FALSE)
	{
#if UICC__AUTH__GBA
		ReceiveData();

		if((P2 == 0x84) && (RecData[0] == 0xDD))
		{
#if UICC__OPERATOR__CHINATELECOM
			SetGBAAuthMode(AUTH_MODE_GBA_CLEAR);
#else
			G_GBA_AUTH_Status = 0x00;
#endif
		}
#endif
		ConditionSatisfied();
		return;
	}



	if(P1 != 0x00)
	{
		P1P2Error();														//Send 6B 00
		return;
	}

	if(P3 == 0x00)
	{
		P3Error();														//Send 6B 00
		return;
	}


// 	if(ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE)
// 	{
// 		SecuritySatisfied();												//Send 69 82
// 		return;
// 	}
#if UICC__AUTH__COUNTER
	if(AuthCounter() == FALSE)
	{
		TechnicalProblem();
		return;
	} 
#endif

#if UICC__ISIM	
	if(G_CurrentAdfName[G_ChannelNum] == G_DF_ID_ISIM)
	{
		fileAddr = SelectTFFile( G_DF_ID_ISIM, EF_IST);
	}
	else
#endif		
	{
		fileAddr = SelectTFFile( G_DF_ID_USIM, EF_UST);
	}
	
	/*if(fileAddr == FALSE)  // modify by  chentao@20211021  没有考虑CSIM
	{
		ConditionSatisfied();												//Send 69 85
		return;
	}*/


	result = 0x00;
	tag = 0x00;
	flag = 0;
	
	if(G_CurrentAdfName[G_ChannelNum] == G_DF_ID_CSIM)
	{
		flag = 1;
	}
	
	if(flag == 0)
	{
#if UICC__CDMA__SMS__DOWNLOAD
		G_IsCdmaSms = FALSE;
#endif
		ReceiveData();
		
		algorithmID = getAlgorithmID(adfAddr);
#if UICC__GUOWANG__AUTH_SPECIAL_CONFIG
		//国网特殊需求，除了国网私有鉴权，其他鉴权时，都要根据国网私有（鉴权标识）而定
		//私有鉴权的64字节空间中，第20字节表示是否正常鉴权，00表示正常鉴权，A5表示拒绝鉴权，返回状态字6985
		if(algorithmID != 0x0F)
		{
			Pirvate_GetGWSecData(19,&tag,0,1);

			if(tag==0xA5)
			{
				ConditionSatisfied();
				return;
			}
		}
#endif
		switch(P2)
		{
#if UICC__AUTH__EAP
			case 0x00:   // add 20220418 add EAP
			{
				if(MOBILE_Operator == OPERATOR_CHINAUNICOM)//联通蝴蝶结测试工具测试，AUTHENTICATE命令测试，P2是00期望抛P1P2错误
				{
					P1P2Error();
					return;
				}
				result = EAPAuthentication();

				if(result == 3)
				{
					return;
				}

				if(result != FALSE)
				{
					result = G_ResponseLen;
				}

				break;
			}
#endif
			case 0x80:
			{
				if(ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE)
				{
					SecuritySatisfied();												//Send 69 82
					return;
				}
				
				if(algorithmID != 2)
				{
					if(P3 != 0x11)
					{
						P3Error();														//Send 67 00
						return;
					}
				}

				if(fileAddr != FALSE)
				{
					ReadTFBuf(fileAddr, 0x04, &result, sizeof(BYTE));
				}

				if ((result&0x20) != 0x20)
				{
					GSMSecurityContextNotSupported();								//Send 9864

					return;
				}	

			#if UICC__AUTH__XOR
				if(algorithmID == 3)//xor
				{
					result = GSMXORA3A8();
				}
			#endif
			
			#if UICC__AUTH__TUAK
				if(algorithmID == 2)
				{
					result = TUAK2G();
				}	
			#endif
				
#if UICC_FUCTION_PRIVATEAUTH
				if(algorithmID == 0x0F)
				{
					result = PrivateAuthentication(D_AUTHMODE_2G);
				}
#endif
				if(algorithmID == 1)
				{
					result = Milenage2G(&tempBuf[0]);
					
					ecpMem_Copy(&RecData[0], &tempBuf[0], result);	
				}
	
				
				break;
			}
			case 0x81:
			{
				if(ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE)
				{
					SecuritySatisfied();												//Send 69 82
					return;
				}
				
				if(algorithmID != 2)
				{
					if(P3 != 0x22)
					{
						P3Error();														//Send 67 00
						return;
					}
				}

		#if UICC__AUTH__XOR
				if(algorithmID == 3)
				{
					result = USIMXORAuthenticate();
				}
		#endif
		
		#if UICC__AUTH__TUAK
				if(algorithmID == 2)
				{
					result = TUAK();
				}
		#endif
				
#if UICC_FUCTION_PRIVATEAUTH
				if(algorithmID == 0x0F)
				{
					result = PrivateAuthentication(D_AUTHMODE_3G);
				}
#endif
				if(algorithmID == 1)
				{
					result = Milenage(&tempBuf[0]);
				
					ecpMem_Copy(&RecData[0], &tempBuf[0], result);
				}
				
				if(result == (36+RESLGTH))							//????
				{
				#if UICC__ISIM
					if(G_CurrentAdfName[G_ChannelNum] == G_DF_ID_ISIM)
					{
						break;
					}
				#endif
			
					status = 0x00;				
			
					if(fileAddr != FALSE)
					{
						ReadTFBuf(fileAddr, 0x03, &status, sizeof(BYTE));
					}
					
					if((status&0x04) == 0x04)           			//??kc
					{
						RecData[36+RESLGTH] = 0x08;
			
						for(result=0; result<8; result++)
						{
							RecData[37+RESLGTH+result] =  RecData[3+RESLGTH+result] ^ RecData[3+RESLGTH+result+8] ^ RecData[20+RESLGTH+result] ^ RecData[20+RESLGTH+result+8];
						}

						result = 45+RESLGTH;
					}
					#if UICC__FUNCTION_CMCCAPI
						RAM_Online_Flag = 0x01;
					#endif	
					
#if UICC__AUTH__EPS
					if(fileAddr != FALSE)
					{
						ReadTFBuf(fileAddr, 0x0A, &status, sizeof(BYTE));
					}
					
					if((status&0x10) == 0x10)
					{
						EPSKeyDerivation();
					}
#endif
				}
				else
				{
					G_GBA_AUTH_Status = 0;	
					
					#if UICC__FUNCTION_CMCCAPI
						RAM_Online_Flag = 0;
					#endif	

					#if UICC__OPERATOR__CHINATELECOM
								SetGBAAuthMode(AUTH_MODE_GBA_CLEAR);
					#else
								G_GBA_AUTH_Status = 0;
					#endif
				}
					
				break;
			}	
			case 0x84: //GBA
			{

				if(RecData[0] == 0xDD)
				{
					#if UICC__OPERATOR__CHINATELECOM
							SetGBAAuthMode(AUTH_MODE_GBA_CLEAR);
					#else
							G_GBA_AUTH_Status = 0x00;
					#endif
				}

				if(ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE)
				{
					SecuritySatisfied();												//Send 69 82
					return;
				}
				
				if(fileAddr != FALSE)
				{
				#if UICC__ISIM
					if(G_CurrentAdfName[G_ChannelNum] == G_DF_ID_ISIM)
					{
						ReadTFBuf(fileAddr, 0x00, &result, sizeof(BYTE));
						
						if ((result&0x02) != 0x02)
						{
							ConditionSatisfied();											//Send 6985

							return;
						}

						fileAddr = SelectTFFile(G_DF_ID_ISIM, EF_GBABP);
					}				
					else	
                #endif						
					{
						ReadTFBuf(fileAddr, 0x08, &result, sizeof(BYTE));
						
						if ((result&0x08) != 0x08)
						{
							ConditionSatisfied();											//Send 6985

							return;
						}	

						fileAddr = SelectTFFile(G_DF_ID_USIM, EF_GBABP_USIM);
					}



					if(fileAddr == FALSE)
					{
						ConditionSatisfied();											//Send 6985

						return;
					}

					fileStatus = GetFileHeadByte(fileAddr,F_Status_Offset);

					if((fileStatus&0x01) == FILE_INVALIDATE)
					{
						ConditionSatisfied();											//Send 6985

						return;
					}
				}
				else
				{
					ConditionSatisfied();												//Send 69 85
					return;
				}
							
				if (RecData[0] == 0xDD)				//GBA Bootstrapping Mode
				{
					//电信备案测试，5G测试工程集汇总版/03_运营商脚本(Telecom).sol/东信5G新增功能测试log（WD工具）/Bootstrapping/TC_Bootstrapping_WrongLc
					//GBA_天喻/GBA_U指令测试_参数测试/19.USIM_GBA_U_P004，P013有测到
					if(P3 != 0x23)
					{
						P3Error();														//Send 67 00
						return;
					}
									
					if((RecData[1] != 0x10) || (RecData[18]!=0x10))//整理数据
					{
						ConditionSatisfied();
						return ;
					}
					
					ecpMem_Copy(&RecData[0], &RecData[1], P3-1);
					
					result = Milenage(&tempBuf[0]);
					
					ecpMem_Copy(&RecData[0], &tempBuf[0], result);
					
					if(result == (36+RESLGTH))
					{
						RecData[1+RESLGTH] ^= 0x01;					//The UICC then transfers RES (after flipping the least significant bit) to the ME						
						
						result = 2+RESLGTH;
					}
					else
					{
						#if UICC__OPERATOR__CHINATELECOM
									SetGBAAuthMode(AUTH_MODE_GBA_CLEAR);
						#else
									G_GBA_AUTH_Status = 0;
						#endif
					}
				}
				else if (RecData[0] == 0xDE)		//GBA NAF Derivation Mode
				{


		#if UICC__OPERATOR__CHINATELECOM
					fileGBAModeAddr =  SelectTFFile(G_CurrentAdfName[G_ChannelNum],EF_AUTH_MODE);

					ReadTFBuf(fileGBAModeAddr,0,&gbaAuthMode,0x01);//鉴权复位之后，模式还要维持上一次的的值，电信备案测试

					if(gbaAuthMode != 0x01)
		#else
					if (G_GBA_AUTH_Status != 0x01)									//尚未执行GBA Bootstrapping Mode鉴权
		#endif
					{
						ConditionSatisfied();										//Send 69 85
						
						return;
					}
					
					if(G_CurrentAdfName[G_ChannelNum] == G_DF_ID_USIM)//USIM :P3|DE|L|NAFID|L|IMPI
					{
						fileAddrbp = SelectTFFile(G_DF_ID_USIM, EF_GBANL_USIM);
						
						fileAddr1 =  SelectTFFile(G_DF_ID_USIM, EF_GBABP_USIM);
						
						recLen = GetLFRecNumLgth(fileAddrbp,FileLgth);//获取6FDA文件的记录长度
						
						ReadTFBuf(fileAddr1, 0x11, &BPLen, sizeof(BYTE));//获取6FD6文件的B-TID长度
						
						BPLen = (BPLen == 0xFF) ? 0 : BPLen;//如果终端没有更新EF_GBABP，则不对EF_GBANL的B-TID数据域进行更新
						
						//modify by zhxx@20221206,判断80L[NAF_ID]+81L[B-TID]不能超过6FDA文件的记录长度
						//modify by zhxx@20230330,CTCC V1.1.B code review,add ()
						if((RecData[1] >= (P3-2)) || ((P3-3) != (RecData[1] + RecData[RecData[1]+2])) || ((RecData[1]+2 + BPLen+2) > recLen) /*|| (RecData[1]>0x30)*/)//最新中国移动GBA技术规范升级，NAFID的长度最大为0x30
						{
							IncorrentParameter();
							return;
						}

						
					
					}
					else//ISIM:P3|DE|L|NAFID
					{
						if(P3 != (RecData[1] + 2))
						{
							IncorrentParameter();
							return;
						}
						
						fileAddrbp = FALSE;
#if UICC__ISIM
						fileAddrbp = SelectTFFile(G_DF_ID_ISIM, EF_GBANL);
					
#endif						
					}
					
					fileAddrKs = SelectLFFile(G_CurrentAdfName[G_ChannelNum],EF_GBA_KS);

//					fileAddrKs = SelectLFFile(G_CurrentAdfName[G_ChannelNum],EF_GBA_KS);
	
					fileAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum],EF_AUTH_MODE);
					
					if((fileAddr == FALSE) || (fileAddrKs == FALSE) || (fileAddrbp == FALSE))
					{
						ConditionSatisfied();
						return;
					}
					
					fileStatus = GetFileHeadByte(fileAddrbp,F_Status_Offset);
			
					if((fileStatus&0x01) == FILE_INVALIDATE)
					{
						ConditionSatisfied();											//Send 6985

						return;
					}						
					
					
					result = GBANAFAuthenticate();
				}
				else
				{
					FunctionNotSupport();
					return;
				}
							
				break;
			}

			default:
			{
				P1P2Error();
				return;
			}	
		}

		if(result == FALSE)
		{
			AuthenticateError();
		}
		else
		{
			CommandFinish(result);
		}	
	}
#if UICC__AUTH__CDMA__MD5
	else
	{
#if UICC__CDMA__SMS__DOWNLOAD		
		G_IsCdmaSms = TRUE;
#endif		
		RunCDMAAlgorithm();
	}
#endif
#if COS_UPDATE_ONLINE_MUTUAL
	G_AuthenticationFlag = TRUE;	//鉴权成功标识，目前用于upcos业务中
#endif
}


BYTE UpdateNAFId(BYTE * buf)
{
	UWORD xdata fileAddr, fileAddr1;
	BYTE idata recNum,fileNum;
	BYTE idata i, tag,result,cfgOff;
	
	fileAddr = FALSE;
	
	fileAddr1 = FALSE;
	recNum = 0;
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
	
	if ((fileAddr == FALSE)||(fileAddr1 == FALSE))
	{
		return FALSE;
	}

	tempBuf[0] = 0x80;										//NAF_ID tag
	
	ecpMem_Copy(&tempBuf[1], &buf[0], buf[0]+1);
	
	tempBuf[buf[0]+2] = 0x81;								//B-TID tag
	
	ReadTFBuf(fileAddr1, 0x11, &tempBuf[buf[0]+3], sizeof(BYTE));
	
	//if ((tempBuf[buf[0]+3] == 0xFF) ||(tempBuf[buf[0]+3] == 0x00))						//如果终端没有更新EF_GBABP，则不对EF_GBANL的B-TID数据域进行更新
	if (tempBuf[buf[0]+3] == 0xFF)	
	{
		tempBuf[buf[0]+3] = 0x00;
	}
	
	ReadTFBuf(fileAddr1, 0x12, &tempBuf[buf[0]+4], tempBuf[buf[0]+3]);	
	
	fileNum = GetLFRecNumLgth(fileAddr,FileNum);
	
	result = IF_BeginTransaction();	 

	for(i = 1; i<= fileNum; i++)//-如果给定的NAF_ID记录已经存在，则USIM更新这条记录中相应的B-TID
	{
		ReadLFBuf(fileAddr, i, 0x00 , &tempBuf[200],2);//读取tag及长度字节
		
		if ((tempBuf[200] == 0x80) && (tempBuf[201] == buf[0]))//判断tag为0x80，且长度和输入的nafid长度相等
		{
		   ReadLFBuf(fileAddr, i, 0x02 , &tempBuf[202],tempBuf[201]);//读取nafid数据
     			
		   if(ecpMem_Compare(&tempBuf[202],&buf[1],buf[0]) == 0)
		   {
			    WriteLFBuf(fileAddr, i, 0x00, &tempBuf[0], (buf[0]+4+tempBuf[buf[0]+3])); 									
			
			    WriteEepromEX(E_NAFID_OLD_RECORD + cfgOff, &i, sizeof(BYTE));	//使用配置区来标记最近更新的记录号,cfgOff:1表示USIM，cfgOff:0表示ISIM 

#if (UICC__AUTH__MBMS||UICC__AUTH__LOCALKEY) //modify by xieyong G_MUK_IDX localkey 也需要使用
			    G_MUK_IDX = i;
#endif			
			    recNum = i;
				break;
		   }

		}
	}

    
    if(i > fileNum)	//如果给定的NAF_ID记录不存在，则USIM在一条空记录中保存NAF_ID及其B-TID
	{
		for(i = 1; i<= fileNum; i++)
		{
			ReadLFBuf(fileAddr, i, 0x00 , &tag, 1);
			
			if (tag != 0x80)
			{ 
				WriteLFBuf(fileAddr, i, 0x00, &tempBuf[0], (buf[0]+4+tempBuf[buf[0]+3])); 									
				
				WriteEepromEX(E_NAFID_OLD_RECORD + cfgOff, &i, sizeof(BYTE));	//

			#if (UICC__AUTH__MBMS||UICC__AUTH__LOCALKEY)//modify by xieyong G_MUK_IDX localkey 也需要使用
				G_MUK_IDX = i;
			#endif
				recNum = i;
				break;			   		   			
			}
		}
	}	
	
	if (i > fileNum)
	{
		recNum = ReadEECacheByte(E_NAFID_OLD_RECORD + cfgOff); 	//
		
		if ((recNum+1) > fileNum)				//计算最早更新的记录号
		{
			recNum = 0x01;
		}
		else
		{
			recNum ++;
		}								
 			
 		WriteLFBuf(fileAddr, recNum, 0x00, &tempBuf[0], (buf[0]+4+tempBuf[buf[0]+3])); 
				
		WriteEepromEX(E_NAFID_OLD_RECORD + cfgOff, &recNum, sizeof(BYTE));
	
		#if (UICC__AUTH__MBMS||UICC__AUTH__LOCALKEY) //modify by xieyong G_MUK_IDX localkey 也需要使用
		  G_MUK_IDX = recNum;
		#endif
	}
	
	IF_CommitTransaction(result);	
	
	return recNum;
}

/*                                                                                     
 *@Function name :	 GBANAFAuthenticate for GBA security context (NAF Derivation Mode) 
 *					 
 *@Description:       Ks_ext_NAF = KDF(Ks, "gba-me", RAND, IMPI, NAF_Id) 
								= HMAC-SHA-256 (Key , S)
					 	Key = Ks = CK || IK
					 	S = FC||P0||L0||P1||L1||P2||L2||P3||L3
							FC = 0x01
							P0 = 0x6762612D6D65 ("gba-me")
							L0 = 0x06
							P1 = G_Auth_Rand
							L1 = length of G_Auth_Rand
							P2 = IMPI
							L2 = length of IMPI
							P3 = NAF_ID
							L3 = length of NAF_ID
  *
  *@Coder: 			 xuqs            
  *
  *@Status:			   
*/
BYTE code gbame[] = {0x67,0x62,0x61,0x2D,0x6D,0x65};	//gba-me

BYTE code gbau[] = {0x67,0x62,0x61,0x2D,0x75};	//gba-u

/*
	modify by chentao@20211111 GBA鉴权参数ISIM和USIM分开存储（YJR确认）
*/

BYTE GBANAFAuthenticate(void)
{
    BYTE idata index,len;
	UWORD xdata fileAddrbp,fileAddrKs,fileAuthMode;
	WORD idata pos;
	
#if  UICC__ISIM
	BYTE idata ch[2];
	UWORD xdata fileAddr;
	
	fileAddr = FALSE;
	
	if(G_CurrentAdfName[G_ChannelNum] == G_DF_ID_ISIM)
	{
		fileAddrbp = SelectTFFile(G_DF_ID_ISIM, EF_GBABP);
		
		fileAddr = SelectTFFile(G_DF_ID_ISIM, EF_IMPI);
		
		if(fileAddr == FALSE)
		{
			return FALSE;
		}
		
		ReadTFBuf(fileAddr, 0x00, &ch[0], sizeof(ch));
		
		if (ch[0] != 0x80)
		{
			return FALSE;
		}
		
	}
	else
#endif
	{
		fileAddrbp = SelectTFFile(G_DF_ID_USIM, EF_GBABP_USIM);

	}
	
	fileAddrKs = SelectLFFile(G_CurrentAdfName[G_ChannelNum],EF_GBA_KS);
	
//	fileAddrKs = SelectLFFile(G_CurrentAdfName[G_ChannelNum],EF_GBA_KS);
	
	fileAuthMode = SelectLFFile(G_CurrentAdfName[G_ChannelNum],EF_AUTH_MODE);

	//if((fileAddrbp == FALSE) || (fileAddrKs == FALSE) || (fileAuthMode == FALSE))
	//{
	//	return FALSE;
	//}
	
	index = UpdateNAFId(&RecData[1]);	
	
	if(index == 0)
	{
		return FALSE;
	}
	
	tempBuf[0] = 0x01;												//FC
	
	ecpMem_Copy(&tempBuf[1], (BYTE *)gbame, sizeof(gbame));				//P0

	tempBuf[1+sizeof(gbame)] = 0x00;								//L0
	tempBuf[2+sizeof(gbame)] = sizeof(gbame);	
	
	pos = 1+2+sizeof(gbame);										//point to P1

	//fileAddr = SelectTFFile(G_DF_ID_ISIM, EF_GBABP);			
	if(IsBufAllZeroOrFF(G_Auth_Rand,0,sizeof(G_Auth_Rand))==FALSE)//优化鉴权性能（88/C0指令不写文件，其他指令再写）,移动备案性能测试 by yjr20211213
	{
		ecpMem_Copy(&tempBuf[pos],&G_Auth_Rand[1],16);		
	}
	else
	{
		ReadTFBuf(fileAddrbp, 0x01, &tempBuf[pos], 16);					//读取6FD5中的RAND值		
	}

	tempBuf[pos+16] = 0x00;						//L1
	
	tempBuf[1+pos+16] = 16;
	
	pos += (2+16);									//point to P2

#if UICC__ISIM	
	if(G_CurrentAdfName[G_ChannelNum] == G_DF_ID_ISIM)
	{
		ReadTFBuf(fileAddr, 0x00, &ch[0], sizeof(ch));
	
		ReadTFBuf(fileAddr, 2, &tempBuf[pos], ch[1]);				//P2
		
		len = ch[1];
	}
	else
#endif
	{
		ecpMem_Copy(&tempBuf[pos],&RecData[3+RecData[1]],RecData[2+RecData[1]]);
		len = RecData[2+RecData[1]];

	}
	
	tempBuf[pos+len] = 0x00;									//L2
	tempBuf[1+pos+len] = len;							
	
	pos += (2+len);											//point to P3
	ecpMem_Copy(&tempBuf[pos], &RecData[2], RecData[1]);					//P3

	tempBuf[pos+RecData[1]] = 0x00;									//L3
	tempBuf[1+pos+RecData[1]] = RecData[1];
	
	pos += (2+RecData[1]);
	
	if(IsBufAllZeroOrFF(G_Auth_Ks,0,sizeof(G_Auth_Ks))==FALSE)//优化鉴权性能（88/C0指令不写文件，其他指令再写）,移动备案性能测试 by yjr20211213
	{
		ecpMem_Copy(&tempBuf[pos],&G_Auth_Ks[1],0x20);
	}
	else
	{
		ReadTFBuf(fileAuthMode, 1, &tempBuf[pos], 0x20);
	}
		
	//HMAC_SHA256(&RecData[0x80], 0x20, &tempBuf[0], pos, &RecData[2]);//HMAC-SHA-256 (Key , S)	
	HMACSha256DoFinalSingleCall(&tempBuf[pos],0x20,&tempBuf[0],pos,&RecData[2]);
	
	
	RecData[0] = 0xDB;												//"Successful GBA operation									

	RecData[1] = 0x20;												//Length of Ks_ext_NAF
		
	ecpMem_Copy(&tempBuf[1], (BYTE *)gbau, sizeof(gbau));				//P0

 	tempBuf[1+sizeof(gbau)] = 0x00;								//L0

 	tempBuf[2+sizeof(gbau)] = sizeof(gbau);	
 	
 	ecpMem_Copy(&tempBuf[3+sizeof(gbau)],&tempBuf[3+sizeof(gbame)],pos-9);
 	
 	//HMAC_SHA256(&RecData[0x80], 0x20, &tempBuf[0], pos-1, &RecData[0x30]);//HMAC-SHA-256 (Key , S) // Ks_int_NAF
	HMACSha256DoFinalSingleCall(&tempBuf[pos],0x20,&tempBuf[0],pos-1,&RecData[0x30]);
	
	WriteLFBuf(fileAddrKs,index,0,&RecData[0x30],0x20);//更新NAF_ID相对应记录的Ks_int_NAF
		
	return 2+RecData[1];
	
}

#if UICC_FUCTION_PRIVATEAUTH
void ReadTFBuf_Process(BYTE * filePath, WORD offset, BYTE * buf, WORD len)
{
	UWORD 	xdata fileAddr;

	if(filePath[0] == 0x02)
	{
		fileAddr = SelectTFFile(((filePath[1]<<8)|filePath[2]), ((filePath[3]<<8)|filePath[4]));
	}
	else if(filePath[0] == 0x03)
	{
		fileAddr = SelectTFFile(((filePath[1]<<8)|filePath[2]), ((filePath[3]<<8)|filePath[4]));

		fileAddr = SelectFile(fileAddr,((filePath[5]<<8)|filePath[6]));
	}
	else
	{
		return;
	}


	if(fileAddr==FALSE)
	{
		return;
	}


	ReadTFBuf(fileAddr, offset, buf, len);
}

void ReadLFBuf_Process(BYTE * filePath, BYTE index, WORD offset, BYTE * buf, WORD len)
{
	UWORD 	xdata fileAddr;

	if(filePath[0] == 0x02)
	{
		fileAddr = SelectTFFile(((filePath[1]<<8)|filePath[2]), ((filePath[3]<<8)|filePath[4]));
	}
	else if(filePath[0] == 0x03)
	{
		fileAddr = SelectTFFile(((filePath[1]<<8)|filePath[2]), ((filePath[3]<<8)|filePath[4]));

		fileAddr = SelectFile(fileAddr,((filePath[5]<<8)|filePath[6]));
	}
	else
	{
		return;
	}


	if(fileAddr==FALSE)
	{
		return;
	}


	ReadLFBuf(fileAddr, index, offset, buf, len);
}

void WriteTFBuf_Process(BYTE * filePath, WORD offset, BYTE * buf, WORD len)
{
	UWORD 	xdata fileAddr;

	if(filePath[0] == 0x02)
	{
		fileAddr = SelectTFFile(((filePath[1]<<8)|filePath[2]), ((filePath[3]<<8)|filePath[4]));
	}
	else if(filePath[0] == 0x03)
	{
		fileAddr = SelectTFFile(((filePath[1]<<8)|filePath[2]), ((filePath[3]<<8)|filePath[4]));

		fileAddr = SelectFile(fileAddr,((filePath[5]<<8)|filePath[6]));
	}
	else
	{
		return;
	}


	if(fileAddr==FALSE)
	{
		return;
	}


	WriteTFBuf(fileAddr, offset, buf, len);
}

void WriteLFBuf_Process(BYTE * filePath, BYTE index, WORD offset, BYTE * buf, WORD len)
{
	UWORD 	xdata fileAddr;

	if(filePath[0] == 0x02)
	{
		fileAddr = SelectTFFile(((filePath[1]<<8)|filePath[2]), ((filePath[3]<<8)|filePath[4]));
	}
	else if(filePath[0] == 0x03)
	{
		fileAddr = SelectTFFile(((filePath[1]<<8)|filePath[2]), ((filePath[3]<<8)|filePath[4]));

		fileAddr = SelectFile(fileAddr,((filePath[5]<<8)|filePath[6]));
	}
	else
	{
		return;
	}

	if(fileAddr==FALSE)
	{
		return;
	}


	WriteLFBuf(fileAddr, index, offset, buf, len);
}


void GetOrSetSqn_Process(BYTE mode, BYTE index, BYTE *output)
{
	UWORD xdata fileAddr;
	BYTE idata i,tmp[6];

	fileAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum], EF_SQNCheck);

	if(fileAddr == FALSE)
	{
		return;
	}

	if(mode == SQN_MAX_Mode)
	{
		for(i=1;  i<= 32; i++)  //
		{
			ReadLFBuf(fileAddr, i, 0x00, &tmp[0], 6);

			if(memcmp_p(&tmp[0], &output[0], 0x06) > EQUAL)
			{
				ecpMem_Copy(output, tmp, 0x06);
			}
		}
	}
	else if(mode == INDEX_SQN_Mode)
	{
		ReadLFBuf(fileAddr, index, 0x00, &output[0], 0x06);
	}
	else if(mode == SET_SQN_Mode)
	{
		WriteLFBuf(fileAddr, index, 0x00, &SQN[0], 0x06);
	}
	else
	{
		return;
	}


}
#endif
// add by chentao 20221207 俄罗斯MTS招标需要测试 V1V2V3

#if (UICC__AUTH__V1||UICC__AUTH__V2V3)
#define ALGV1 0x01
#define ALGV2 0x02
#define ALGV3 0x03


#if  UICC__AUTH__ANTICLON                                                                                                                                                                                                                                                               
void ReadKiFromFile(BYTE Ki[0x10])                                                                                                                                                                                                                                      
{                                                                                                                                                                                                                                                                       
	UWORD idata fileAddr;                                                                                                                                                                                                                                           
	BYTE  idata wrongTag,foundTag;                                                                                                                                                                                                                                  
	BYTE  idata i,j;                                                                                                                                                                                                                                                
	DWORD idata counter;                                                                                                                                                                                                                                            
                                                                                                                                                                                                                                                                        
	fileAddr =  SelectLFFile(DF_PROFILECFG,EF_RAND);                                                                                                                                                                                                                     
                                                                                                                                                                                                                                                                        
	wrongTag = 0x00;                                                                                                                                                                                                                                                
                                                                                                                                                                                                                                                                        
	for(i=0;i<5;i++)                                                                                                                                                                                                                                                
	{                                                                                                                                                                                                                                                               
		ReadLFBuf(fileAddr,i+1,0x00,&RecData[0x10+i*0x10],0x10);                                                                                                                                                                                                
                                                                                                                                                                                                                                                                        
		foundTag = 0x00 ;                                                                                                                                                                                                                                       
                                                                                                                                                                                                                                                                        
		for(j=0;j<0x10;j++)                                                                                                                                                                                                                                     
		{                                                                                                                                                                                                                                                       
			if(RecData[0x10+i*0x10+j] == RecData[j])                                                                                                                                                                                                        
			{                                                                                                                                                                                                                                               
				foundTag = foundTag+1;                                                                                                                                                                                                                  
			}                                                                                                                                                                                                                                               
		}                                                                                                                                                                                                                                                       
                                                                                                                                                                                                                                                                        
		if((foundTag > 0x0B)&&(foundTag != 0x10))                                                                                                                                                                                                               
		{                                                                                                                                                                                                                                                       
			wrongTag = wrongTag+1;                                                                                                                                                                                                                          
		}                                                                                                                                                                                                                                                       
	}                                                                                                                                                                                                                                                               
                                                                                                                                                                                                                                                                        
	WriteLFBuf(fileAddr,(G_RandomClone%0x05)+1,0x00,&RecData[0x00],0x10);                                                                                                                                                                                                
                                                                                                                                                                                                                                                                        
	if(wrongTag >0x00)                                                                                                                                                                                                                                              
	{                                                                                                                                                                                                                                                               
		fileAddr = SelectLFFile(DF_PROFILECFG,EF_WRONG_COUNTER);                                                                                                                                                                                                                 
                                                                                                                                                                                                                                                                        
		if(fileAddr != FALSE )                                                                                                                                                                                                                                  
		{                                                                                                                                                                                                                                                       
			ReadTFBuf(fileAddr,0,(BYTE  *)&counter,sizeof(counter));  

#if (!BIG_ENDIAN)	
			counter = (DWORD)(counter<<24)+ ((DWORD)(counter<<8)&0x00FF0000) +  ((DWORD)(counter>>8)&0x0000FF00) +(DWORD) (counter >> 24);		
#endif				
                                                                                                                                                                                                                                                                        
			counter = counter + wrongTag;   
			
#if (!BIG_ENDIAN)
			counter = (DWORD)(counter<<24)+ ((DWORD)(counter<<8)&0x00FF0000) +  ((DWORD)(counter>>8)&0x0000FF00) +(DWORD) (counter >> 24);		
#endif				
                                                                                                                                                                                                                                                                        
			WriteTFBuf(fileAddr,0,(BYTE  *)&counter,sizeof(counter));                                                                                                                                                                                       
		}    
			
		Random_number_generator(&Ki[0],0x10,ALG_SECURE_RANDOM);
                                                                                                                                                                                                                                                                        
		//fileAddr = SelectTFFile(DF_PHASE,EF_WrongKi);		//File Type TF                                                                                                                                                                                  
	}                                                                                                                                                                                                                                                               
	else                                                                                                                                                                                                                                                            
	{                                                                                                                                                                                                                                                               
		//fileAddr = SelectLFFile(DF_PHASE,EF_KI);

		fileAddr = SelectLFFile(DF_MF,G_DF_ID_USIM);  // modify chentao@20221207 2G 鉴权的ki固定用USIM中的ki

		fileAddr = SelectFile(fileAddr,EF_KI);
		
		//ReadEeprom(&Ki[0],fileAddr+sizeof(RecordFileHead),0x10);	//File Type   
		ReadLFBuf(fileAddr, 1, 0x00, &Ki[0], 0x10);		//读出鉴权的KI
	}                                                                                                                                                                                                                                                               
                                                                                                                                                                                                                                                                        
	//for(wrongTag = 0;wrongTag<RecData[00];wrongTag++);                                                                                                                                                                                                              
                                                                                                                                                                                                                                                                        
	G_RandomClone = G_RandomClone + 1 ;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              	                                                                                                                                                                                  
}                                                                                                                                                                                                                                                                       
#else                                                                                                                                                                                                                                                                   
void ReadKiFromFile(BYTE Ki[0x10])                                                                                                                                                                                                                                      
{                                                                                                                                                                                                                                                                       
	UWORD idata fileAddr;                                                                                                                                                                                                                                           
                                                                                                                                                                                                                                                                        
	fileAddr = SelectLFFile(G_CurrentDfName[G_ChannelNum],EF_KI);                                                                                                                                                                                                                          
                                                                                                                                                                                                                                                                        
	ReadLFBuf(fileAddr,1,0,&Ki[0],0x10);                                                                                                                                                                                                                            
}                                                                                                                                                                                                                                                                       
#endif


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 RunGsmAlgorithmA3A8
  
  Command format:    
  
  Data    content:   random                                                       
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       This function add the value given by the ME to the value of the last increased/updated record of the current cyclic EF

  status:			 Code OK!     Test OK!
  					 
				
 -------------------------------------------------------------------------------------------------------------------*/    
void RunGsmAlgorithmA3A8(void)
{
	UWORD xdata keyAddr;
	BYTE idata type;
	
	if((P1 != 0x00)||(P2 != 0x00))
	{
		P1P2Error();
		return;
	}

	if( P3 != 0x10 )
	{
		P3Error();//send 67 00
		return;
	}	
	                                                   
	if(( G_CurrentDfName[G_ChannelNum] != 0x7F20 )&&( G_CurrentDfName[G_ChannelNum] != 0x7F21 ))
	{
		FileStructureError();//send 94 08
		return;
	}	                                           		
	
	if(AuthCounter() == FALSE)
	{
		return;
	}
	
	if( ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE )
	{
		SecurityError();//send 98 04
		return;
	}  

	keyAddr = SelectLFFile(DF_MF,G_DF_ID_USIM);

	keyAddr = SelectFile(keyAddr,EF_KI);

	if( keyAddr == FALSE )
	{													//send 6f 00(no find ki)
		TechnicalProblem();
		return;
	}

	//keyAddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_KI);
  
	if( keyAddr == FALSE )
    {
		TechnicalProblem(); //send 6f 00(no find ki)
		return;
	}

	ReceiveData();   //接收函数里面有清空buffer,提前到读数据那里执行
	
	//ReadLFBuf(keyAddr,1,0,&RecData[0x10],0x10);
	
	ReadKiFromFile(&RecData[0x10]);

	memcpy((BYTE xdata *)&RecData[0x20],(BYTE xdata *)&RecData[0],0x10);  // added by xyz@20101104  9:16
			
	//type=0x01;
	
	//FindCOSConfigTag(Tag_Config_Algorithm,&type);
	
	if(AUTH_V1)
	{
#if UICC__AUTH__V1		
		RUNA3A8_V1(&RecData[0x10],&RecData[0x20]);
#endif
	}
#if UICC__AUTH__V2V3	
 	else if(AUTH_V2) 
	{
		RUNA3A8_V2(&RecData[0x10],&RecData[0x20]);
	}
	else if (AUTH_V3)
	{
		RUNA3A8_V3(&RecData[0x10],&RecData[0x20]);
	}
#endif	
	else
	{
		ConditionSatisfied();
	}

		
	CommandFinish(0x0C);                                          			  //send 9f 0c	                                            		
}
#endif
