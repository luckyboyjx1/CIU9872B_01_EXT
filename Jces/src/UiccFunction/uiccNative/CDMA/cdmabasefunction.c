/**
 * @file 	cdmaBasefunction.c
 *
 * @brief 	Copyright (c) 2002, eastcompeace
 *
 * 		All rights reserved.
 *
 * 		MODEL: EM TG96-3G/TCG96-3G, EMOSYN, ATMEL, SAMSUANG, INFINEON, RENESAS, SOLOMON
 *
 * 		MOTOCOS V4.03
 *
 * 		PHASE 2  RELEASE:2003-2-25
 *
 * @author 	蔡勃
 *
 * @see 	<3GPP2 C.S0023-C v1.0> 		Chapter 3.4.52/3.5.3/4.6/4.7/4.8
 *		<3GPP2 C.S0016-C>（即TIA-683-C） 	Chapter 3.5.8.12/3.5.8.14
 *		<中国联通800MHz CDMA数字蜂窝移动通信网UIM卡技术要求（V 2.0）> 6.2节
 *
 * @note	84指令对于MEID、OTASP的支持，留待下一个版本添加;
 *
 * @history 	2008-05-20, modified by WG:
 * 			1. 将注释转换为新格式，并给部分函数添加注释、修正注释
 * 			2. ShiftDataPlus()的参数i 由 idata 改为 xdata
 * 			3. CDMAAuth()删除对 G_CurrentDfName 的判断
 * 			4. 根据<C.S0023-C v1.0> 4.4节，及比照RS36MASK4代码，将UpdateSSD命令序列错误时的响应修正为98 34		
 * 			5. 根据<C.S0023-C v1.0> 4.2.1节，及比照RS36MASK4代码，将BaseStationChallenge输出修改为不等于输入
 * 			6. 修改函数Run3GAccessAKA，修正使用memcmp函数方式的错误		  2015-07-17, modified by yangjian
 *      7. 修改函数ConfirmSSD，去掉支持CDMA一卡多号的功能
 *			
 			
 */
#include <string.h>
#include "uicc_main.h"
#include "stkhead.h"
#include "fileAction.h"
#include "fileSystem.h"
#include "uicc_error.h"
#include "gsmpublic.h"
#include "cdma.h"
#include "interface.h"
//#include "typedef.h"
//#include 	"Authentication.h"


#ifdef CDMA

#ifdef AKA

#include "aka.h"

BYTE    xdata    G_AMF[0x02];
BYTE    xdata    G_K[0x10];
BYTE    xdata    G_Rand[0x10];

#endif


/**
 * @brief 	Store_ESN_MEID_ME(A0 DE 00/01 00 08)
 *
 * @author 	XWH, WG, TL
 *
 * @see		《中国联通800MHz CDMA数字蜂窝移动通信网UIM卡技术要求（V 2.0）》 6.1.6.1节
 * 		<C.S0023-C v1.0> 4.6.1节
 *
 * @history 	20080520, modify by WG, TL: 增加对MEID的支持
 */
void StoreESNMe(void)
{
	UWORD xdata  fileAddr, uimidFileAddr;
	BYTE  idata  flag;
	
#ifdef CSIM

	/*
	BYTE idata tem;																			//只在2G-CDMA模式中使用.CSIM-CDMA中不使用
 
	fileAddr = SelectTFFile(DF_CDMA, EF_CDMASST);							
	
	ReadTFBuf(fileAddr,0x02,&tem,0x01);			
	
	if(G_CommandMode == Command_Mode_2G)					//2G模式下。。
	{
		if((tem&0x03)!=0x03)								//If service n9 is not allocated and activated,不支持P1=0x01
		{
			if(P1 != 0x00)
			{
				P1P2Error();
				return;
			}
		}	
	}	
*/	
	
#endif
	
	// If service n9 is allocated and activated and ME is assigned with MEID
	// then P1 = ‘01’
	if (((P1 != 0x00) && (P1 != 0x01)) || (P2 != 0x00))
	{
		P1P2Error();
		return;
	}                                       //send 6B 00

	if (P3 != 0x08)
	{
		P3Error();							//send  67 00
		return;
	}

	/*************************************************************************
* Conformance Requirement
* CHV1 	shall be successfully verified before issuing any cdma2000 command. 
* DFCDMA  shall be selected prior to executing any cdma2000 command. 
**************************************************************************/  
	if(G_CommandMode == Command_Mode_2G)
	{	
		if (G_CurrentDfName[G_ChannelNum] != DF_CDMA)
		{
			FileInconsistent();		
			return;
		}
	}
	else
	{
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
	}	

	if (ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE )
    {
		SecurityError();						//98 04
		return;
	}
/************************************************************************/

	ReceiveData();

	//fileAddr = SelectTFFile(DF_CDMA, EF_ESNME);
	fileAddr = SelectTFFile(G_DF_ID_CSIM, EF_ESNME);

	if (fileAddr == FALSE)
	{
		FileNoFind();						//Send 94 04
		return;
	}

	ReadTFBuf(fileAddr, 0x00, &RecData[0x10], P3);			//read old esn

	if (((RecData[0]&0x0F)>0x07)||((RecData[0]&0x0F)<0x04))
	{
		TechnicalProblem();								//Send 6F 00
		return;
	}

	RecData[0x00] = RecData[0x00]&0x0F;

	RecData[0x10] = RecData[0x10]&0x0F;

//	resultStatus = memcmp_p(&RecData[0x10], &RecData[0], P3);

	// 	Bit 0: 指示ESN_ME是否与EF 6F38中存储的ESN相同
	// 	Bit 4: 设置为‘1’表示使用UIMID；设置为‘0’表示使用ESN
	// 	当前，EASTCOMPEACE的所有CDMA COS都默认使用UIMID
	// 	其它为RFU

	//uimidFileAddr = SelectTFFile(DF_CDMA, EF_UIMID);//read EF_USGIND(UIM_ID/SF_EUIMID Usage Indicator) 0x6F42
	uimidFileAddr = SelectTFFile(G_DF_ID_CSIM, EF_UIMID);

	ReadTFBuf(uimidFileAddr, 0x00, (BYTE *)&flag, 1);

#ifdef CSIM

	if(P1 == 0x00)																		//for change20101221
	{
		flag = flag&0x01;
	}
	else
	{
		flag = flag&0x03;
	}
	
#endif

 //if (resultStatus != EQUAL)
	if (memcmp_p(&RecData[0x10], &RecData[0], P3) != EQUAL)
	{
		// ESN is encoded with the lowest-order byte first to match the coding for EFESN-ME.
		WriteTFBuf(fileAddr, 0x00, (BYTE xdata *)&RecData[0], P3);
		
#ifdef CSIM
		RecData[0] = (0x10*flag) +0x01;													//
#else
		RecData[0] = (0x10*flag) +0x01+P1*0x20;											//
#endif	
			
	}
	else
	{	
#ifdef CSIM
		RecData[0] = (0x10*flag) +0x00;
#else
		RecData[0] = (0x10*flag) +0x00+P1*0x20;
#endif	
		
	}

	CommandFinish(0x01);
}


/**
 * @brief
 *
 * @param 	RANDSSD: const, 7 bytes
 * @param 	A_key: const, 8 bytes
 *
 * @author
 *
 * @history 	created by
 */
 
void SSD_Generation(const BYTE * RANDSSD, const BYTE * A_key)
{
	BYTE idata  i;
	
	int idata offset_1,offset_2;

	for (i=0;i<4;i++)
	{
		LFSR[i]=RANDSSD[i+3] ^ A_key[i] ^ A_key[i+4];
	}

	if ((LFSR[0] | LFSR[1] | LFSR[2] | LFSR[3])==0)
	{
		for (i=0;i<4;i++)
		{
			LFSR[i] = RANDSSD[i+3];
		}
	}

	for (i=0;i<8;i++)
	{
		Register[i]=A_key[i];
	}

	Register[8] = AAV;

	for (i=9;i<12;i++)
	{
		Register[i]=RANDSSD[i-9];
	}

	for (i=12;i<16;i++)
	{
		Register[i] = ESN[i-12];
	}

	offset_1 = 128;

	offset_2 = 128;

	CAVE(8, &offset_1, &offset_2);

	for (i=0;i<8;i++)
	{
		SSD_A_NEW[i]=Register[i];

		SSD_B_NEW[i]=Register[i+8];
	}
}


/**
 * @brief 	UpdateSSD        A0	84	00	00	0F
 *
 * 		input:
 * 			RecData[0-6]: RANDSSD
 * 			RecData[7]: Process_Control
 * 			RecData[8-E]: ESN
 * 		output:
 * 			AuthBs
 *
 * @author
 *
 * @see		<C.S0023-C v1.0> 4.2.1 4.4.1
 *
 * @note	84指令对于MEID、OTASP的支持，留待下一个版本添加;
 *
 * @history 	20080520, modified by WG:
 * 		根据<C.S0023-C v1.0> 4.4节，及比照RS36MASK4代码，将命令序列错误时的响应修正为98 34
 */

void UpdateSSD(void)
{
	UWORD xdata fileAddr;

	if(G_CommandMode != Command_Mode_2G)     //84 now only used as update SSD command in R_UIM
	{
		InsError();
		return;
	}

	if ((P1 != 0x00) || (P2 != 0x00))
	{
		P1P2Error();
		return;
	}                                       //send 6B 00

	if (P3 != 0x0F)
	{
		P3Error();			//send  67 00
		return;
	}

	if (G_CurrentDfName[G_ChannelNum] != DF_CDMA)
	{
		FileInconsistent();						//94 08
		return;
	}

	if (ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE )
    {
		SecurityError();						//98 04
		return;
	}

	if (ValueBit(G_CdmaStatus, B_BSrun) == FALSE)
	{
		// <C.S0023-C v1.0> chapter 4.4
		// 'If either Update SSD or Confirm SSD are run out of sequence,
		// the card shall return SW1=‘98’ and SW2=‘34’.'

		CDMAError();	//send 98 34
		return;
	}

	//fileAddr = SelectLFFile(DF_CDMA, EF_Ki);
	fileAddr = SelectLFFile(G_DF_ID_CSIM, EF_Ki); 

	if (fileAddr == FALSE)
	{
		TechnicalProblem();						//send 6f 00
		return;
	}

	ReadLFBuf(fileAddr, 0x01, 0x00, (BYTE *)&RecData[0x10], 0x08);	//Read A_key

	ReceiveData();

	SSD_Generation(&RecData[0], &RecData[0x10]);				//RANDSSD, A_key

	AuthBs = Auth_Signature(RANDBS, MIN1, SSD_A_NEW, (RecData[7] &0x10));	//Rand_CHALLENGE AUTHDATA SSDAUTH SAVEREGISTERS
	
	G_CdmaStatus = (G_CdmaStatus & 0xFE)|0x02;

	CommandFinish(0x00);
}


/**
 * @brief 	ConfirmSSD        A0	82	00	00	03
 *
 * 		input: Network's AuthBs
 *
 * @author
 *
 * @history 	
 */
void ConfirmSSD(void)
{
	UWORD xdata  fileAddr;
	BYTE  xdata  p[4];
	//BYTE  idata  phoneNum;
	
	if(G_CommandMode == Command_Mode_2G)
	{
		if ((P1 != 0x00) || (P2 != 0x00))
		{
			P1P2Error();
			return;                        	
		}
		
		if (P3 != 0x03)
		{
			P3Error();			
			return;
		}

		if (G_CurrentDfName[G_ChannelNum] != DF_CDMA)
		{
			FileInconsistent();		
			return;
		}

		if (ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE )
	    {		
			SecurityError();										//98 04
			return;	
		}

		if (ValueBit(G_CdmaStatus, B_USrun) == FALSE)
		{
			CDMAError();	 		
			return;
		}
	}
	else
	{
		if ((P1 != 0x00) || ((P2 != 0x00)&&(P2 != 0x01)))
		{
			P1P2Error();
			return;                        	
		}

		if(P2 == 0x00)         //Update SSD command
		{
			if(P3 != 0x0F)
			{
				P3Error();			
				return;
			}
		}

		if(P2 == 0x01)        //Confirm SSD command
		{
			if(P3 != 0x03)
			{
				P3Error();			
				return;
			}
		}

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

		if (ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE )
	    {		
			SecuritySatisfied();												//for change 2010.11.23   3G模式下该返回码应为6982
			//SecurityError();						//98 04
			return;
		}

		if(P2 == 0x00)         //Update SSD command
		{
			if (ValueBit(G_CdmaStatus, B_BSrun) == FALSE)
			{
				CDMAError();	
				return;
			}
		}

		if(P2 == 0x01)        //Confirm SSD command
		{
			if (ValueBit(G_CdmaStatus, B_USrun) == FALSE)
			{
				CDMAError();	 		
				return;
			}
		}	
	}

	ReceiveData();

	if((G_CommandMode == Command_Mode_2G)||(P2 == 0x01))         //Confirm SSD command
	{	
		p[1] = (AuthBs>>16)&0xFF ;
	
		p[2] = (AuthBs>>8)&0xFF ;
	
		p[3] = (AuthBs)&0xFF ;
	
		G_CdmaStatus = G_CdmaStatus&0xFC;
	
		if ((p[3]!=RecData[2])||(p[2]!=RecData[1])||(p[1]!=RecData[0]))
		{	
			if(G_CommandMode == Command_Mode_2G)											//for change2010.11.23  区分2G和3G模式下的返回状态码  
			{
				SecurityError();
			}
			else
			{
				SecuritySatisfied();
			}		
		}
		else
		{
			//fileAddr = SelectTFFile(DF_CDMA, EF_SSD);
			fileAddr = SelectTFFile(G_DF_ID_CSIM, EF_SSD);
	
			if (fileAddr == FALSE)
			{
				TechnicalProblem();	
				return;
			}
	
			memcpy(&RecData[0], &SSD_A_NEW[0], 0x08);
	
			memcpy(&RecData[8], &SSD_B_NEW[0], 0x08);
	
			WriteTFBuf(fileAddr, 0x00, (BYTE *)&RecData[0], 0x10);
	
			// CDMA 一卡多号支持：SSD切换
			//-----UPDATE ssd to the copy record---------------------------------------------
			/*
			fileAddr = SelectLFFile(DF_STK, EF_CARDNUMBER);
	
			if (fileAddr != FALSE)
			{
				ReadLFBuf(fileAddr, 0x01, 0x00, &phoneNum, sizeof(BYTE));
	
				fileAddr = SelectLFFile(DF_NUMBACKUP, EF_SSD_CDMABAK);	  
	
				if (fileAddr != FALSE)
				{
					WriteLFBuf(fileAddr, (phoneNum&0x0F), 0x00, (BYTE xdata *)&RecData[0], 0x10);
				}
			}
			*/
			
	
			CommandFinish(0x00);					   
		}
	}
	else                                                       //Update SSD command
	{
		//fileAddr = SelectLFFile(DF_CDMA, EF_Ki);
		fileAddr = SelectLFFile(G_DF_ID_CSIM, EF_Ki); 

		if (fileAddr == FALSE)
		{
			TechnicalProblem();						
			return;
		}
	
		ReadLFBuf(fileAddr, 0x01, 0x00, (BYTE *)&RecData[0x10], 0x08);	//Read A_key
	
		SSD_Generation(&RecData[0], &RecData[0x10]);				            //RANDSSD, A_key
	
		AuthBs = Auth_Signature(RANDBS, MIN1, SSD_A_NEW, (RecData[7] &0x10));	//Rand_CHALLENGE AUTHDATA SSDAUTH SAVEREGISTERS
	
		G_CdmaStatus = (G_CdmaStatus & 0xFE)|0x02;
	
		CommandFinish(0x00);
	}
}

/**
 * @brief 	A0	8A	00	00	04
 * 		BaseStationChallenge, 鉴权流程的起始COMMAND
 *
 * 		input: RANDSeed
 * 		output:RANDBS
 *
 * @author 	?
 *
 * @history 	20080521, modified by WG:
 * 		根据<C.S0023-C v1.0> 4.2.1节，及比照RS36MASK4代码，将输出修改为不等于输入
 */

void BaseStationChallenge(void)
{
	BYTE  idata resultStatus;
	UWORD xdata fileAddr;

	if ((P1 != 0x00) || (P2 != 0x00))
	{
		P1P2Error();
		return;
	}                                       //send 6B 00

	if (P3 != 0x04)
	{
		P3Error();			//send  67 00
		return;
	}

	if(G_CommandMode == Command_Mode_2G)
	{
		if (G_CurrentDfName[G_ChannelNum] != DF_CDMA)
		{
			FileInconsistent();		
			return;
		}
	}
	else
	{
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
	}	

	if (ValueBit(G_ChvStatus, B_Chv_1_Verified)==FALSE)
	{
		if(G_CommandMode == Command_Mode_2G)									//for change2010.11.23  区分2G和3G模式下的返回状态码  
		{
			SecurityError();
			return;
		}
		else
		{
			SecuritySatisfied();
			return;
		}
	}                                      

	ReceiveData();

	memcpy(&RANDBS, &RecData[0], 0x04);

	resultStatus = InitCave();

	if (resultStatus == TRUE)
	{
		SetBit(G_CdmaStatus, B_BSrun);

		//根据<C.S0023-C v1.0> 4.2.1节，及比照RS36MASK4代码，将输出修改为不等于输入
		//RecData[0] ++;

		CommandFinish(0x04);
	}
	else
	{
		TechnicalProblem();
	}
}


/**
 * @brief 	A0 8E 00 00 08 XX XX XX XX XX XX XX XX
 *
 * @author 	
 *
 * @history 	created by
 */
void GenerateKey(void)
{
	BYTE  idata ResLength, offsetStart, offsetEnd;
	UWORD xdata fileAddr;

	if ( (P1 != 0x00) || (P2 != 0x00))
	{
		P1P2Error();
		return;
	}                                           //send 6B 00

	if (P3 != 0x02)
	{
		P3Error();							    //send  67 00
		return;
	}

	if(G_CommandMode == Command_Mode_2G)
	{
		if (G_CurrentDfName[G_ChannelNum] != DF_CDMA)
		{
			FileInconsistent();		
			return;
		}
	}
	else
	{
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
	}

	if (ValueBit(G_ChvStatus, B_Chv_1_Verified)==FALSE)
	{	
		if(G_CommandMode == Command_Mode_2G)									//for change2010.11.23  区分2G和3G模式下的返回状态码  
		{
			SecurityError();
			return;
		}
		else
		{
			SecuritySatisfied();
			return;
		}	
		
	}  	

	if (ValueBit(G_CdmaStatus, B_KeyOk) == FALSE)
	{
		CDMAError();					//98 34
		return;
	}

	ReceiveData();

	//fileAddr = SelectTFFile(DF_CDMA, EF_SSD);
	fileAddr = SelectTFFile(G_DF_ID_CSIM, EF_SSD);

	if (fileAddr == FALSE)
	{
		TechnicalProblem();
		return;
	}

	ReadTFBuf(fileAddr, 8, (BYTE *)&RecData[0x10], 8);	//Read ssd_b

	Key_VPM_Generation((BYTE *)&RecData[0x10], (BYTE *)&RecData[0x20], (BYTE *)&RecData[0x30]);   //SSB  cmeakey VPM

	if ( (RecData[0] <= 0x40) && (RecData[1] <= 0x40) && (RecData[0] <= RecData[1]) )
	{
		ResLength = 8 + RecData[1] - RecData[0] + 1;						//copy the data

		offsetStart = RecData[0];

		offsetEnd = RecData[1];

		memcpy(&RecData[0], &RecData[0x20], 8);								//copy cmeakey

		memcpy(&RecData[8], &RecData[0x30+offsetStart], offsetEnd-offsetStart+1);

		CommandFinish(ResLength);

	}else
	{
		memcpy(&RecData[0], &RecData[0x20], 8);								//copy cmeakey

		CommandFinish(0x08);
	}
}

/**
 * @brief	for GSMCDMAAltInRunAltIntf( A0    88    00   00   10 )
 *
 * @author
 *
 * @history 	created by
 */
void CDMAAuth(void)
{
	BYTE  idata  i, j;
	UWORD xdata  fileAddr;
	DWORD xdata  V;
	BYTE  xdata  * p;

	memcpy(&RecData[0x20], MIN1, 3);	//统一查询, 可能有输入数据, 有输入数据不用MIN1

	if ( RecData[0] == 0 )			//rand
	{
		if ( RecData[5] != 0 )		//统一查询//数字个数不为0
		{
			i = 2;														// now reinit the Auth_Data

			j = 8;

			RecData[5] = RecData[5]>>2;

			while ( 1 )
			{
				if ( RecData[5] >= 2 )
				{
					RecData[0x20+(i--)] = RecData[j--];

					RecData[5] -= 2;

					if (RecData[5] == 0)
					{
						break;
					}
				}
				else
				{
					RecData[0x20+i] &= 0xF0;

					RecData[0x20+i] |= (RecData[j]&0x0F);

					break;
				}
			}
		}
	}
	else if ( RecData[0] == 1 )											//randu
	{
		if ( RecData[5] != 0 )		//统一查询//数字个数不为0
		{
			i = 2;														// now reinit the Auth_Data

			j = 8;

			RecData[5] = RecData[5]>>2;

			while ( 1 )
			{
				if ( RecData[5] >= 2 )
				{
					RecData[0x20+(i--)] = RecData[j--];

					RecData[5] -= 2;

					if (RecData[5] == 0)
					{
						break;
					}
				}
				else
				{
					RecData[0x20+i] &= 0xF0;

					RecData[0x20+i] |= (RecData[j]&0x0F);

					break;
				}
			}
		}

		for ( i = 1 ; i < 4;i++)
		{
			RecData[i] = RecData[i+1];
		}

		RecData[4] = MIN2[1];
	}
	else
	{
		TechnicalProblem();
		return;
	}

	if ((RecData[9]&0x10) == 0x10)										//process_control的BIT4为1才能执行GenerateKey命令
	{
		SetBit(G_CdmaStatus, B_KeyOk);
	}

	//fileAddr = SelectTFFile(DF_CDMA, EF_SSD);
	fileAddr = SelectTFFile(G_DF_ID_CSIM, EF_SSD);

	if (fileAddr == FALSE)
	{
		TechnicalProblem();
		return;
	}

	ReadTFBuf(fileAddr, 0x00, (BYTE *)&RecData[0x30], 0x10);								//Read SSD_A SSD_B

	V = Auth_Signature((BYTE *)&RecData[1], (BYTE *)&RecData[0x20], (BYTE *)&RecData[0x30], (RecData[9]&0x10));				//RAND MIN SSD_A Process_Control

	p = (BYTE *)&V;
	
#ifdef LittleEndian

	RecData[2] = p[0];

	RecData[1] = p[1];

	RecData[0] = p[2];
#else

	RecData[2] = p[3];

	RecData[1] = p[2];

	RecData[0] = p[1];
#endif

//	G_Authed = G_Authed | 0x01;

	CommandFinish(0x03);
}



/*---------------------------------------------------------------------------------

 function name:      	ShiftDataUncoil

 Input parameter:	   	input:input string

 Output parameter:   	output:string after shift

 Description:       	左移字符串pos个bit

 缓冲要求: input,output不得覆盖,开始地址可以相同

-----------------------------------------------------------------------------------*/

void ShiftDataUncoil(BYTE *input,BYTE len,BYTE *output,BYTE pos)
{
	BYTE idata i;

	if(len <= 1)
	{
		return;
	}

	for(i=0;i<len-1;i++)
	{
		output[i] = (input[i]<<pos) | (input[i+1] >>(8-pos));            //截去头5位，尾3位
	}
}


#endif // CDMA





