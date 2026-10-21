/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
 			V1.1 增加开发函数EnvelopeCommand,便于根据需求改动。应用开发若无明确需求,此函数不要作改动							 by yangjian 20190626
			V1.2 修改函数MilenageInit,电信单4G卡使用不同的KI文件ID									by yangjian 20190821
			V1.3 修改函数MilenageInit,从敏感数据区中取OPC,KI等值									by zhangxiaoxiao 20200330
			V1.4 增加兼容中国电信CSIM卡算法接口与中国铁路V3算法接口									by yangjian 20200514
			V1.5 增加兼容多IMSI功能的鉴权处理                                       by yangjian 20210105

 */
#include	"string.h"
#include 	"uicc_main.h"		   
#include "uicc_error.h"
#include 	"fileSystem.h"
#include 	"fileAction.h"
#include 	"gsmpublic.h"
//#include 	"resume.h"
#include 	"Authentication.h"
#include    "stkhead.h"
#include    "typedef.h"
#include    "interface.h"
#include 	<string.h>	
#ifdef CDMA	
#include    "cdma.h"
#endif
#ifdef MULTI_IMSI
#include "MultiImsi.h"
#endif

#ifdef V4
BYTE xdata G_Random;
#endif

#ifndef SingleSIM
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 UsimAuthentication
  
  Command format:    00    88   00   XX   XX
  
  Data    content:   UICC authentication data from ME                                                       
                                                                                                                                                
  Description:       This function is for UICC authentication

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/

void UsimAuthentication(void)
{
	BYTE idata result, mode;
	UWORD xdata fileAddr;

	

	//"a particular USIM application or any sub-directory must be
	//selected as the Current Directory" --TS31122
	if (G_CurrentAdfName[G_ChannelNum] == FALSE)
	{
		fileAddr = FALSE;
	}
	else 
	{
		fileAddr = SearchFile(E_FILE_START, G_CurrentAdfName[G_ChannelNum]);

		fileAddr = SearchFile(fileAddr, G_CurrentDfName[G_ChannelNum]);
	}	

	if (fileAddr == FALSE)
	{
		ConditionSatisfied();												//Send 69 85
		return;
	}

	if(P1 != 0x00)
	{
		P1P2Error();														//Send 6B 00
		return;
	}

	if(P2 == 0x80)
	{
		if(P3 != 0x11)
		{
		 	P3Error();														//Send 67 00
			return;
		}

		//check "GSM security context" available or not(from "USIM Service Table")
		result = 0x00;
		
		fileAddr = SelectTFFile( G_CurrentAdfName[G_ChannelNum], EF_UST);

		if(fileAddr != FALSE)
		{
			ReadTFBuf(fileAddr, 0x04, &result, sizeof(BYTE));
		}

		if ((result&0x20) != 0x20)
		{
			GSMSecurityContextNotSupported();								//Send 9864

			return;
		}

		mode = D_AUTHMODE_2G;
	}
	else if(P2 == 0x81)
	{
		if(P3 != 0x22)
		{
			P3Error();														//Send 67 00
			return;
		}

		mode = D_AUTHMODE_3G;
	}
	else
	{
		P1P2Error();														//Send 6B 00
		return;
	}

	if(ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE)
	{
		SecuritySatisfied();												//Send 69 82
		return;
	}

	ReceiveData();

	result = USIMAuthenticate(mode);

	if(result == FALSE)
	{
		AuthenticateError();
	}
	else
	{
		CommandFinish(result);
	}
}



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 USIMAuthenticate
  
  Data   content:    mode:	鉴权模式
					 
  Description:       鉴权主控函数的实现,网络端的内容存放在RecData[0]开始的地方，根据命令的P2字节判断鉴权的类型
  					 如果是3G鉴权模式，需要根据6F38文件中的内容判断命令返回的长度

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/

BYTE USIMAuthenticate(BYTE mode)
{
	UWORD idata fileAddr;
	BYTE  idata result, status;

	if(MilenageInit() == FALSE)
	{
		return FALSE;
	}

	if(mode == D_AUTHMODE_2G)
	{
		Milenage2G(&tempBuf[0]);						 //处理在3G网络中2G的鉴权算法（需要进一步确认）
	
		memcpy(&RecData[0], &tempBuf[0], 14);
		
		return 14;
	}
	else
	{
		result = Milenage(&tempBuf[0]);						//处理3G的鉴权算法

		if(result == (36+RESLGTH))
		{
			memcpy(&RecData[0], &tempBuf[0], 36+RESLGTH);

			status = 0x00;
	
			fileAddr = SelectTFFile(DF_ADF, EF_UST);		//判断EF_UST的内容，文件存在ADF下，暂时放在MF下，未完成！
	
			if(fileAddr != FALSE)
			{
				ReadTFBuf(fileAddr, 0x03, &status, sizeof(BYTE));
			}
			
			if((status&0x04) == 0x04)           //得到kc
			{
				RecData[36+RESLGTH] = 0x08;
	
				for(result=0; result<8; result++)
				{
					RecData[37+RESLGTH+result] =  RecData[3+RESLGTH+result] ^ RecData[3+RESLGTH+result+8] ^ RecData[20+RESLGTH+result] ^ RecData[20+RESLGTH+result+8];
				}

				return 45+RESLGTH;
			}
			else
			{
				return 36+RESLGTH;
			}
		
		}
		else if(result == 16)
		{
			memcpy(&RecData[0], &tempBuf[0], 16);

			return 16;
		}
		else
		{
			return FALSE;
		}
	}
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 MilenageInit
  
  Data   content:   
					 
  Description:       Milenage算法初始化
  					 

  Coder:                     
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/


BYTE  MilenageInit(void)
{
	BYTE idata i;
//#ifdef CSIM		
	UWORD xdata fileAddr;
//#endif	


#if  NATIVE_CARD_UICC
	
#ifdef CSIM	
	fileAddr = SelectLFFile(G_CurrentDfName[G_ChannelNum], EF_KI);		 	//按照中国电信LTE卡配置数据，USIM与SIM使用不同的IMSI，KI数据

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
#ifdef MULTI_IMSI
	ReadLFBuf(fileAddr, G_Cur_IMSI_RecNo, 0x00, &RecData[100], 16);		//支持多IMSI功能,使用G_Cur_IMSI_RecNo数据,G_Cur_IMSI_RecNo默认值为1
#else
	ReadLFBuf(fileAddr, 0x01, 0x00, &RecData[100], 16);		//读出鉴权KI
#endif	
	
#else																													//非CSIM卡即为单4G卡,直接从敏感数据区读取文件

  ReadEeprom(&RecData[100],E_SECURE_DATA + KI_OFF,16); 	//读出鉴权的KI

	ReadEeprom(&RecData[116],E_SECURE_DATA + AUTH_CFG_OFF,111); 	//读出鉴权用的常数字段OPMode，OP，R，C等
#endif

#else

fileAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum], EF_KI);		 	

if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadLFBuf(fileAddr, 0x01, 0x00, &RecData[100], 16);		//读出鉴权KI


#endif
	
	
#if NATIVE_CARD_UICC	
	
#ifdef CSIM		
	fileAddr = SelectTFFile(DF_MF, EF_AuthCFG);

	if(fileAddr== FALSE)
	{
		return FALSE;
	}
	
#ifdef MULTI_IMSI  
	ReadTFBuf(fileAddr,(G_Cur_IMSI_RecNo-1)*OPC_SIZE, &RecData[116], 111);			
#else
   ReadTFBuf(fileAddr,0x00, &RecData[116], 111);			
#endif	

#endif
#else
  fileAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum], EF_PRIVATE_AUTHCFG);
	
	if(fileAddr== FALSE)
	{
		return FALSE;
	}
	
	 ReadTFBuf(fileAddr,0x00, &RecData[116], 111);			
#endif


	key_expansion(&RecData[100], &tempBuf[80]);          //密钥扩展

	if((RecData[116]&D_OPc) == D_OPc)						//如果需要计算OPc字段
	{		
		AESEncryptCoreCipher(&RecData[117], &tempBuf[0],&tempBuf[80]);
		
		for (i=0; i<16; i++)
		{
			RecData[117+i] ^= tempBuf[i];
		}
	}
	
	return TRUE;
}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 Milenage
  
  Data   content:    output：	鉴权函数计算的输出结果
					 
  Description:       3G鉴权中的主控函数的实现,网络端的内容存放在RecData[0]开始的地方

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/
BYTE Milenage(BYTE * output)
{
	BYTE  idata i;
	BYTE  xdata tmp[16], xRes[16], eKresult[16];

	for(i=0; i<16; i++)
	{
		tmp[i] = RAND[i] ^ OPc[i];					//计算出随机数和OPc的异或结果
	}

	AESEncryptCoreCipher(&tmp[0], &tmp[0],&tempBuf[80]);					//AES计算,得到F1到F5的公用输入参数

	memcpy(&eKresult[0], &tmp[0], 16);				//eKresult保存了F1的输入参数

	for(i=0; i<16; i++)								//经过AES计算的结果和OPc进行异或
	{
		tmp[i]  ^= OPc[i];					//tmp内保存了F2到F5的输入参数
	}

	F2toF5Function(tmp, xRes, D_F2_Mode);			//得到xRES和AK的内容前8个字节是xRes的值

	if((OPMode&D_F5_Mode) == D_F5_Mode)				//需要还原SQN的内容
	{
		for(i=0; i<6; i++)
		{
			SQN[i] = xRes[i] ^ SQN[i];			
		}
	}

	F1Function(eKresult, output);						//计算xMac

	if(memcmp(&output[0], &MAC[0], 8) != EQUAL)
	{
		return FALSE;
	}

	i = CheckSQN();

	if(i == FALSE)
	{
		return FALSE;
	}

	if(i == D_F1Start_Mode)
	{
		AMF[0] = 0x00;

		AMF[1] = 0x00;                                   //计算 MAC-S时，AMF设置为全0，参考TS133102                     

		F1Function(eKresult, output);					//计算xMac
		
		F2toF5Function(tmp, xRes, D_F5Start_Mode);		//得到F5*计算AK*的内容

		if((OPMode&D_F5_Mode) == D_F5_Mode)				//需要还原SQN的内容
		{
			for(i=0; i<6; i++)
			{
				SQN[i] = xRes[i] ^ SQN[i];			
			}
		}

		output[0] = 0xDC;

		output[1] = 0x0E;

		memcpy(&output[2], SQN, 6);

		return 16;
	}
	else
	{
		output[0] = 0xDB;
	
		output[1] = RESLGTH;			  				//RES
	
		memcpy(&output[2], &xRes[8], RESLGTH);
	
		F2toF5Function(tmp, xRes, D_F3_Mode);			//得到xRES和CK的内容
	
		output[2+RESLGTH] = 16;
		
		memcpy(&output[3+RESLGTH], xRes, 16);
	
		F2toF5Function(tmp, xRes, D_F4_Mode);			//得到xRES和IK的内容
	
		output[19+RESLGTH] = 16;
		
		memcpy(&output[20+RESLGTH], xRes, 16);

		return 36+RESLGTH;
	}
}



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 CheckSQN
  
  Data   content:   
                                                                                                                                                
  Description:       3G鉴权中检查邋SQN是否超过范围

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/
//#ifndef LittleEndian
//BYTE code zero_[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
//#endif
BYTE CheckSQN(void)
{
	UWORD 		xdata fileAddr;
	BYTE  		idata index;
	signed char 	idata i;
	BYTE  		xdata seq[6], seqms[6];
	BYTE		xdata delta[6], tmp[6], L[6];
	BYTE		idata ind, deltaC;
	BYTE        xdata sqnMax[6];  //获取当前文件中的最大SQN 值
	BYTE        xdata indexSeq[0x06];
//#ifdef LittleEndian    
    BYTE        xdata zero[6];
      
    memset(&zero[0],0x00,sizeof(zero));    
//#endif
	
	#if NATIVE_CARD_UICC
	fileAddr = SelectTFFile(DF_MF, EF_AuthCFG);
	#else
	
	fileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum], EF_PRIVATE_AUTHCFG);
	#endif
	
#ifdef MULTI_IMSI
	//支持多IMSI切换功能,使用G_Cur_IMSI_RecNo对应数据
	ReadTFBuf(fileAddr, (G_Cur_IMSI_RecNo-1)*OPC_SIZE+D_SQNCheck_Offset, &tempBuf[0], 8);  
#else
	ReadTFBuf(fileAddr, D_SQNCheck_Offset, &tempBuf[0], 8);
#endif	

	ind = tempBuf[0];

	memcpy(&L[0],&tempBuf[1],sizeof(L));

	deltaC= tempBuf[7];
	
	index = ~(0xFF<<ind);			//根据配置的IND的bit位计算IND的值

	index = (SQN[5] & index) + 1;

	for(i=5; i>=0; i--)					//得到网络的SEQ的值
	{
		seq[i] = SQN[i] >> ind;

		if(i > 0x00)
		{
			seq[i] |= SQN[i-1]<<(8-ind);
		}
	}

	
	#if NATIVE_CARD_UICC
	
#ifdef MULTI_IMSI	
	fileAddr = SelectTFFile(DF_MF, EF_SQNCheck);						//使用多IMSI功能时该文件定义为透明文件
#else
  fileAddr = SelectLFFile(DF_MF, EF_SQNCheck);
#endif	
	#else
	 fileAddr = SelectLFFile(G_CurrentAdfName[G_ChannelNum], EF_SQNCheck);
	#endif

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	memset(sqnMax, 0x00, sizeof(sqnMax));
	
	for(i=1;  i<= 32; i++)  //记录当前文件中最大的那条记录的SQN
	{
		
#ifdef MULTI_IMSI		
		ReadTFBuf(fileAddr, (i-1)*6+(G_Cur_IMSI_RecNo-1)*SQN_SIZE, &tmp[0], 6);	
#else
		ReadLFBuf(fileAddr, i, 0x00, &tmp[0], 6);	
#endif		

		if(memcmp_p(&tmp[0], &sqnMax[0], 0x06) > EQUAL) 
		{
			memcpy(sqnMax, tmp, 0x06);
		}
	}

	for(i=5; i>=0; i--)					//get seqms
	{
		seqms[i] = sqnMax[i] >> ind;

		if(i > 0x00)
		{
			seqms[i] |= sqnMax[i-1]<<(8-ind);
		}
	}

	if(memcmp_p(&seq[0], &seqms[0], 0x06) > EQUAL)
	{
		if(deltaC != 0xFF)
		{
			StringSub(&seq[0], &seqms[0], &tmp[0], 0x06);

			memset(&delta[0], 0x00, sizeof(delta));

			i = deltaC >> 3;

			delta[5-i] = 0x01 << (deltaC & 0x07);				//根据卡内配置的数值n，计算delta=2^n

			if(memcmp_p(&tmp[0], &delta[0], 0x06) > EQUAL)  	// compare with delta
			{                  
				memcpy(&SQN[0], sqnMax, 0x06);//替换当前数据的SQN为卡内的最大的SQNms

				return D_F1Start_Mode;			
			}
		}
	}
	else		  										//添加对"L"的判断				 
	{	
//#ifndef LittleEndian
//		if (memcmp_p(&L[0], (BYTE CONST*)&zero_[0], 0x06) != EQUAL)	//当L等于0时不对L进行判断
//#else
        if (memcmp_p(&L[0], &zero[0], 0x06) != EQUAL)	//当L等于0时不对L进行判断  
//#endif
		{	
			StringSub(&seqms[0], &seq[0], &tmp[0], 0x06);
	
			if(memcmp_p(&tmp[0], &L[0], 0x06) >= EQUAL)  	  	//SQN - SQNms < L
			{
				//memcpy(&SQN[0], sqnMax, 0x06);//替换当前数据的SQN为卡内的最大的SQNms

				//return D_F1Start_Mode;
				goto CheckSqnFailure;
				
			}
		}
	}

	
#ifdef MULTI_IMSI	
	
	ReadTFBuf(fileAddr, (G_Cur_IMSI_RecNo-1)*SQN_SIZE+(index-1)*6, &delta[0], 0x06);	

	for(i=5; i>=0; i--)					
	{
		indexSeq[i] = delta[i] >> ind;

		if(i > 0x00)
		{
			indexSeq[i] |= delta[i-1]<<(8-ind);
		}
	}
	
	if(memcmp_p(&seq[0], &indexSeq[0], 0x06) > EQUAL)  
	{
		WriteTFBuf(fileAddr, (G_Cur_IMSI_RecNo-1)*SQN_SIZE+(index-1)*6, &SQN[0], 0x06); 

		return TRUE;
	}
	

#else	
	ReadLFBuf(fileAddr, index, 0x00, &delta[0], 0x06);


	for(i=5; i>=0; i--)					//当前索引的seq值
	{
		indexSeq[i] = delta[i] >> ind;

		if(i > 0x00)
		{
			indexSeq[i] |= delta[i-1]<<(8-ind);
		}
	}
	
	if(memcmp_p(&seq[0], &indexSeq[0], 0x06) > EQUAL)  //由于文件初始化为0，如果直接比较SQN,则会导致下行SEQ为0，但是SQN不为0时，该比较成立，其实不应该成立
	{
		WriteLFBuf(fileAddr, index, 0x00, &SQN[0], 0x06);        //更新当前的Index的SQN内容

		return TRUE;
	}
#endif	
	
CheckSqnFailure:
	memcpy(&SQN[0], sqnMax, 0x06);//替换当前数据的SQN为卡内的最大的SQNms

	return D_F1Start_Mode;	
	
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 RunGsmAlgorithm
  
  Command format:    A0    88    00   00   10
  
  Data    content:   Random                                                       
                                                                                                                                                
  Description:       This function add the value given by the ME to the value of the last increased/updated record of the current cyclic EF

  Coder:             Caio         Tester

  Status:			 

 -------------------------------------------------------------------------------------------------------------------*/    

void RunGsmAlgorithm(void)
{
#ifdef CSIM	
	UWORD xdata keyAddr;
#endif
	
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

#ifdef CSIM
	keyAddr = SelectLFFile(G_CurrentDfName[G_ChannelNum],EF_KI);
  
	if( keyAddr == FALSE )
  	{													//send 6f 00(no find ki)
		TechnicalProblem();
		return;
	}
#endif
	                                         		
  
	ReceiveData();

	memmove(&RecData[0x01], &RecData[0x00], 0x10);

	if(USIMAuthenticate(D_AUTHMODE_2G) == FALSE)
	{
		AuthenticateError();
	}
	else
	{
		memmove(&RecData[0x00], &RecData[0x01], 4);

		memmove(&RecData[0x04], &RecData[0x06], 8);
	}
	
	CommandFinish(0x0C);
}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 RunAlgorithm
  
  Command format:    
  
  Data    content:                                                          
                                                                                                                                                
  Description:       

  Coder:             

  Status:			 

 -------------------------------------------------------------------------------------------------------------------*/ 
void RunAlgorithm(void)
{
			
	if(G_CommandMode == Command_Mode_2G)
	{
		RunGsmAlgorithm();       
	}
	else	//Command_Mode_3G
	{			
		UsimAuthentication();	   					   			
	
	}
}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 RunAuth
  
  Command format:    
  
  Data    content:                                                          
                                                                                                                                                
  Description:   双模卡算法接口函数(可兼容C网与G网算算法)    

  Coder:             

  Status:			 

 -------------------------------------------------------------------------------------------------------------------*/
void RunAuth(void)
{
	if (G_CommandMode == Command_Mode_2G)		//2G模式
	{
		if((G_CurrentDfName[G_ChannelNum] == DF_PHASE) || (G_CurrentDfName[G_ChannelNum] == DF_DCS1800))	//2G-GSM模式
		{		
			G_NetWorkMode = GSM_NETWORK;			
#ifdef A3A8			
			RunGsmAlgorithmA3A8();								
#else			
			RunGsmAlgorithm();
#endif			
		}
		else 	//2G-CDMA模式
		{
			G_NetWorkMode = CDMA_NETWORK;
#ifdef CDMA			
			RunCDMAAlgorithm();
#endif
		}
	}
	else	//4G模式
	{
		if (G_CurrentAdfName[G_ChannelNum] == G_DF_ID_USIM)	//4G-USIM模式
		{
			UsimAuthentication();
		}
		else	//4G-CDMA模式
		{
#ifdef CDMA					
			RunCDMAAlgorithm();
#endif
		}
	}
}

#endif



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 RunGSMAlgorithmV3
  
  Command format:    
  
  Data    content:                                                          
                                                                                                                                                
  Description:   V3算法函数，提供给中国铁路项目使用   

  Coder:             

  Status:			 

 -------------------------------------------------------------------------------------------------------------------*/
#ifdef V3
void RunGSMAlgorithmV3(void)
{	
	BYTE xdata kiValue[0x10];
	UWORD idata fileAddr;
#ifdef V4
	BYTE  idata wrongTag,foundTag;
	BYTE  idata i,j;
	DWORD idata counter;
#endif
	
	if((P1 != 0x00)||(P2 != 0x00))
	{
		P1P2Error();
		return;
	}

	if(P3 != 0x10)
	{
		P3Error();
		return;
	}	
	                                                  
	if(( G_CurrentDfName[G_ChannelNum] != 0x7F20 )&&( G_CurrentDfName[G_ChannelNum] != 0x7F21 ))
    {
		FileStructureError();
		return;
	}	                                           		//send 94 08

	
	if(ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE)
    {
		SecurityError();								//send 98 04
		return;
	}  
                                          
	ReceiveData();

#ifdef V4
	fileAddr =  SelectLFFile(DF_PHASE,EF_Random);

	wrongTag = 0x00;

	for(i=0;i<5;i++)
	{
		ReadLFBuf(fileAddr,i+1,0x00,&RecData[0x10+i*0x10],0x10);

		foundTag = 0x00;

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

	WriteLFBuf(fileAddr,(G_Random%0x05)+1,0x00,&RecData[0x00],0x10);

	if(wrongTag >0x00)
	{
		fileAddr = SelectLFFile(DF_PHASE,0x6F90);

		if(fileAddr != FALSE )
		{
			ReadTFBuf(fileAddr,0,(BYTE  *)&counter,sizeof(counter));

			counter = counter + wrongTag;

			WriteTFBuf(fileAddr,0,(BYTE  *)&counter,sizeof(counter));
		}

		fileAddr = SelectTFFile(DF_PHASE,EF_WrongKi);		//File Type TF

		ReadTFBuf(fileAddr,0,&kiValue[0],0x10);
	}
	else
	{
		fileAddr = SelectLFFile(DF_PHASE,EF_Ki);

		ReadLFBuf(fileAddr,1,0,&kiValue[0],0x10);
	}

	for(wrongTag = 0;wrongTag<RecData[00];wrongTag++);

	G_Random = G_Random + 1;
#else
	fileAddr = SelectLFFile(DF_PHASE,EF_Ki);

	ReadLFBuf(fileAddr,1,0,&kiValue[0],0x10);
#endif

	memcpy((BYTE *)&RecData[0x20],(BYTE *)&RecData[0],0x10); 		//RUNA3A8_V3函数中用到RecData[0],所以随机数放在RecData[0x20]中输入
		
	RUNA3A8_V3(&kiValue[0],&RecData[0x20]);

	CommandFinish(0x0C);                                          			  //send 9f 0c	 

}
#endif





