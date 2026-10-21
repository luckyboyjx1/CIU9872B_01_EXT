
#include	"string.h"
#include 	"uicc_main.h"
//#include 	"main.h"
#include 	"fileSystem.h"
#include 	"fileAction.h"
#include 	"gsmpublic.h"
//#include 	"resume.h"
#include "Authentication.h"
#include "profile.h"
#include "Keccak.h"
#include "tuak.h"
#include    "stkhead.h"
#include 	<string.h>			 
#include <uicc_error.h>


#if UICC__AUTH__TUAK

static const BYTE ALGONAME[] = "TUAK1.0";
#if RUN_IN_SIMULATOR
BYTE xdata INOUTBUF[200];  // bug 510 重入中，执行TUAK鉴权算法，冲掉Resdata，执行失败
#else
__attribute__((aligned(4))) BYTE xdata INOUTBUF[200];  // bug 510
#endif

void Tuak_main(BYTE  instance,BYTE * rand,BYTE * amf,BYTE * sqn,BYTE * key)
{
   BYTE idata   i;
   BYTE xdata TOPC[32];

   ecpMem_Copy(TOPC, TUAK_TOPc, 32);

   if((TUAK_MODE&D_OPc) == D_OPc)
   {
	   TUAK_ComputeTOPC(key, &TOPC[0]);    /*如果是TOP MODE，则需要计算OPc字段 */
   }

   ecpMem_Set(INOUTBUF , 0, sizeof(INOUTBUF));              /* clean INOUT      */
   PUSH_DATA((BYTE *)TOPC , 32, 0);        /* TOPC             */
   PUSH_DATA(&instance , 1 , 32);          /* INSTANCE         */
   PUSH_DATA((BYTE *)ALGONAME , 7 , 33); /* ALGONAME         */
   PUSH_DATA(rand , 16, 40);             /* RAND             */
   if(amf != NULL)
   {
	   PUSH_DATA(amf , 2, 56);        /* amf              */
   }
   if(sqn != NULL)
   {
	   PUSH_DATA(sqn , 6, 58);    /* sqn              */
   }
   PUSH_DATA(key, (instance & 1)?32:16, 64);
   TUAK_ADD_PADDING();       /* Padding bits 768-1087 */

   for(i=0; i<TUAK_KeIte; ++i)
   {
		KECCAK_F(INOUT);
   }
}

void TUAK_ComputeTOPC(BYTE * key,BYTE * TOPC)
{
	BYTE idata  inst;
	BYTE idata i;
  
	inst = TUAK_KeySize>>5;

    ecpMem_Set((BYTE *)INOUT , 0, 200);       /* clean INOUT             */
    PUSH_DATA(TUAK_TOPc , 32, 0);         /* TOP                     */
    PUSH_DATA(&inst , 1 , 32);           /* INSTANCE for TOPC       */
    PUSH_DATA((BYTE *)ALGONAME , 7 , 33);       /* ALGONAME                */
    PUSH_DATA(key , TUAK_KeySize, 64);       /* KEY-128/256             */
    TUAK_ADD_PADDING();               /* Padding bits 768-1087   */

	for(i=0; i<TUAK_KeIte; ++i)
	{ 
		KECCAK_F(INOUT);
	}
	PULL_DATA(TOPC, 32, 0);      /* get the result           */
}

void Tuak_f1(BYTE * key,BYTE * rand,BYTE * sqn,BYTE * amf,BYTE * mac)
{
	Tuak_main((TUAK_KeySize>>5) | TUAK_MACSize,rand,amf,sqn,key);
	PULL_DATA(mac, TUAK_MACSize, 0);
}

/*used when the SQN is error*/
void Tuak_f1s(BYTE * key,BYTE * rand,BYTE * sqn,BYTE * amf,BYTE * mac)
{
	Tuak_main( (TUAK_KeySize>>5) | TUAK_MACSize | 0x80, rand, amf, sqn, key);
	PULL_DATA(mac, TUAK_MACSize, 0);
}

void Tuak_f5s(BYTE *  key,BYTE *  rand,BYTE *  ak)
{
	Tuak_main( (TUAK_KeySize>>5) | 0xc0, rand, 0, 0, key);
	PULL_DATA(ak, 6, 96);
}


/*Kc[GSM] = CK1 xor CK2 xor IK1 xor IK2*/
BYTE Tuak_calc_kc(BYTE *  ck,BYTE *   ik,BYTE *  kc, BYTE is2G)
{
		UWORD idata  fileAddr;
		BYTE idata   status, i;
    BYTE idata   kcLength;
		status = 0x00;
	
		fileAddr = SelectTFFile(G_DF_ID_USIM, EF_UST);		//判断EF_UST的内容，文件存在ADF下，暂时放在MF下，未完成！
	
		if(fileAddr != FALSE)
		{
			ReadTFBuf(fileAddr, 0x03, &status, sizeof(BYTE));
		}
		
		if(((status&0x04) == 0x04) || is2G)
		{

            if((TUAK_CKSize%2) != 0)
            {
            	return FALSE;
            }
            /*get th KC length*/
            kcLength = (TUAK_CKSize/2);

			for(i=0; i<kcLength; i++)  //计算kc
			{
				kc[i] =ck[i] ^ ck[(kcLength+i)] ^ ik[i] ^ ik[(kcLength+i)];
			}

			return TRUE;
		}
		else
		{
			return FALSE;
		}
}

void Tuak_f2345(BYTE * key,BYTE * rand,BYTE * res,BYTE* ck,BYTE* ik,BYTE* ak)
{
	Tuak_main( (TUAK_KeySize>>5) | ((TUAK_IKSize>>4)&0x02) | ((TUAK_CKSize>>3)&0x04)
	| (TUAK_RESSize&0x38) | 0x40, rand, 0, 0, key);
	PULL_DATA(res, TUAK_RESSize, 0 );
	PULL_DATA(ck , TUAK_CKSize , 32);
	PULL_DATA(ik , TUAK_IKSize , 64);
	PULL_DATA(ak , 6 , 96);

}

void PULL_DATA(BYTE * tmpData,BYTE length,BYTE location)
{
	while(length --)
	{
#if KECCAK_VERSION_BITS==64
	tmpData[length] = (u8)(INOUT[location>>3] >> ((location++ & 7)<<3));
#elif KECCAK_VERSION_BITS==32
	tmpData[length] = (u8)(INOUT[location>>2] >> ((location++ & 3)<<3));
#elif KECCAK_VERSION_BITS==8
	tmpData[length] = INOUT[location++];
#endif
	}
}
/**
*modify by chentao@EPX产业化\INFENION_SLM97\Develop
*/
void PUSH_DATA(const BYTE* tmpData,BYTE length,BYTE location)
{
    while(length --)
    {
#if (KECCAK_VERSION_BITS==64)
    	INOUT[location>>3] |= ((u64)tmpData[length]) << ((location & 7)<<3);
#elif (KECCAK_VERSION_BITS==32)
    	INOUT[location>>2] |= (u32)((u32)tmpData[length]) << ((location & 3)<<3);		
#elif (KECCAK_VERSION_BITS==8)
    	INOUT[location] = tmpData[length];
#endif
		location++;
    }

}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 TUAKInit
  
  Data   content:    output：	true/false
					 
  Description:       初始化TUAK

  Coder:       Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/
BYTE TUAKInit(void)
{
	UWORD xdata fileAddr,adfAddr;
	
	if(G_CurrentAdfName[G_ChannelNum] == FALSE)//2g,默认选择USIM下面的文件
	{
		adfAddr = SelectLFFile(DF_MF,G_DF_ID_USIM);
	}
	else
	{
		adfAddr = SelectLFFile(DF_MF,G_CurrentAdfName[G_ChannelNum]);//3g
	}
		
	fileAddr = SelectFile(adfAddr, EF_PRIVATE_AUTHCFG);

	if(fileAddr== FALSE)
	{
		return FALSE;
	}
	
	ReadTFBuf(fileAddr,0, &RecData[132],  39);  //TOPCMode(1字节) + TOPC(32字节) + KISize(1字节) + ResSize(1字节) + CK(1)+IK(1字节) + MAC(1字节)+KIiTA(1字节)


	fileAddr = SelectFile(adfAddr, EF_KI);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadLFBuf(fileAddr, 0x01, 0x00, &RecData[100],TUAK_KeySize);		//读出鉴权的KI
	
	return TRUE;
}



#define MAC_        (tempBuf)
#define AK          (MAC_  + 32)
#define RES         (AK + 8)
#define CK          (RES + 32)
#define IK          (CK  + 32)
#define KC          (IK  + 32)

/*
*@description: GSM Context进行TUAK算法
*/
BYTE TUAK2G(void)
{
	BYTE idata i,sResLen,kcLen;
	BYTE idata offset;
	
	if(TUAKInit() == FALSE)
	{
		return FALSE;
	}
	
	ecpMem_Set(tempBuf,0x00,sizeof(tempBuf));
	
	Tuak_f2345(TUAK_KI,RAND,RES,CK,IK,AK);
	
	//sResLen = U8_D_U8(TUAK_RES_sz,2);
	sResLen = 0x04;

	/*compute AK，RES，CK，IK*/
	Tuak_f2345(TUAK_KI,RAND,RES,CK,IK,AK);

	for(i = 0; i < sResLen; i++)
	{
        RES[i] = RES[i] ^ RES[i+sResLen];
	}
	/*Compute kc*/
	kcLen = TUAK_CKSize/2;
	Tuak_calc_kc(CK,IK,KC,TRUE);

	/*get the response*/
	 offset = 0;
	 /*padding sRes*/
	 RecData[offset++] = sResLen;
	 ecpMem_Copy(&RecData[offset], RES, sResLen);
	 offset = offset+sResLen;

	 /*padding kc*/
	 RecData[offset++] = kcLen;
	 ecpMem_Copy(&RecData[offset], KC, kcLen);
	 offset = offset+kcLen;

	 return offset;
}


/*
*@description: USIM Context进行TUAK算法
*/
BYTE TUAK(void)
{
	BYTE idata i;
    BYTE idata offset;
	
	if(TUAKInit() == FALSE)
	{
		return FALSE;
	}
	
	ecpMem_Set(tempBuf,0x00,sizeof(tempBuf));
	
	/*compute AK，RES，CK，IK*/
	Tuak_f2345(TUAK_KI,RAND,RES,CK,IK,AK);

	/*DEncrypt SQN*/
	if((TUAK_MODE & D_ANONY_Mode) == D_ANONY_Mode)
	{
		for(i=0; i<6; i++)
		{
			SQN[i] ^= AK[i];
		}
	}

	//----------EPS 要用到	SQN和AK的异或值------
	for(i=0; i<6; i++)
  {
		SQN_AK[i] = SQN[i] ^ AK[i];
	}
	
	/*Compute mac*/
	Tuak_f1(TUAK_KI,RAND,SQN,AMF,MAC_);

	/*Veirfy mac*/
	if(ecpMem_Compare(MAC_, &MAC[0], TUAK_MACSize) != EQUAL)
	{
		return FALSE;
	}


	i = CheckSQN();
	
	if(i == FALSE)
	{
		return FALSE;
	}

	/*if SQN is error,
	 *   AUTS =  Conc(SQNMS ) || MACS;
	 *   Conc(SQNMS) = SQNMS  f5*K(RAND)
	 *   MACS = f1*K(SQNMS || RAND || AMF)
	 *    AMF is all zero*/

	if(i == D_F1Start_Mode)
	{
		AMF[0] = 0;
		AMF[1] = 0;

		/*Compute MAC* with the AMF is "0000"*/
	    Tuak_f1s(TUAK_KI,RAND,SQN,AMF,MAC_);

	    /*Compute MACS*/
	    Tuak_f5s(TUAK_KI,RAND,AK);

		/*Encrypt SQN*/
		for(i=0; i<6; i++)
		{
			SQN[i] ^= AK[i];
		}

		/*Make up the response*/
		RecData[0] = 0xDC;
		RecData[1] = TUAK_MACSize+6;
		ecpMem_Move(&RecData[2], SQN, 6);
		ecpMem_Copy(&RecData[8], MAC_, TUAK_MACSize);

	    return TUAK_MACSize+8;
	}
	else
	{
        offset = 0;
		RecData[offset++] = 0xDB;

		/*Padding RES*/
		RecData[offset++] = TUAK_RESSize;
	  ecpMem_Copy(&RecData[offset], RES, TUAK_RESSize);
	  offset = TUAK_RESSize+offset;

		/*Padding CK*/
		RecData[offset++] = TUAK_CKSize;
		ecpMem_Copy(&RecData[offset], CK, TUAK_CKSize);
		offset = TUAK_CKSize + offset;

	    /*Padding IK*/
		RecData[offset++] = TUAK_IKSize;
		ecpMem_Copy(&RecData[offset], IK, TUAK_IKSize);
		offset = TUAK_IKSize + offset;

		/*To Do kc*/
		if(G_CurrentAdfName[G_ChannelNum] == G_DF_ID_USIM)
		{
			if(Tuak_calc_kc(CK,IK,KC,FALSE) == TRUE)
			{
				 /*Padding KC*/
				RecData[offset++] = TUAK_CKSize/2;
				ecpMem_Copy(&RecData[offset], KC, TUAK_CKSize/2);
				offset = (TUAK_CKSize/2)+offset;
			}	
		}
		

		return offset;
	}

}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 UsimAuthenticationTuak
  
  Command format:    00    88   00   XX   XX
  
  Data    content:   UICC authentication data from ME                                                       
                                                                                                                                                
  Description:       This function is for UICC authentication

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/

//void UsimAuthenticationTuak(void)
//{
//	BYTE idata result, mode;
//	UWORD xdata fileAddr;
//	//BYTE idata ALGflag;

//	if(P2 == 0x80)				//GSM Security context
//	{
//		if(P3 != 0x11)
//		{
//		 	P3Error();				//Send 67 00
//			return;
//		}

//		//check "GSM security context" available or not(from "USIM Service Table")
//		result = 0x00;
//		
//		fileAddr = SelectTFFile( G_CurrentAdfName[G_ChannelNum], EF_UST);

//		if(fileAddr != FALSE)
//		{
//			ReadTFBuf(fileAddr, 0x04, &result, sizeof(BYTE));
//		}

//		if ((result&0x20) != 0x20)
//		{
//			GSMSecurityContextNotSupported();								//Send 9864

//			return;
//		}

//		mode = D_AUTHMODE_2G;
//	}
//	else if(P2 == 0x81)			//3G Security context 
//	{
//		if(P3 != 0x22)
//		{
//			P3Error();														//Send 67 00
//			return;
//		}

//		mode = D_AUTHMODE_3G;
//	}
//	else
//	{
//		P1P2Error();														//Send 6B 00
//		return;
//	}

//	ReceiveData();

//	fileAddr = SelectTFFile(DF_PROFILECFG, EF_PRIVATE_AUTHCFG);
//	
//	if(fileAddr == FALSE)
//	{	
//		TechnicalProblem();
//		return;
//	} 

//	ReadTFBuf(fileAddr,OFFSET_TUAK_AUTHCFG_TOPCMODE , &RecData[132],  39);  //TOPCMode(1字节) + TOPC(32字节) + KISize(1字节) + ResSize(1字节) + CK(1)+IK(1字节) + MAC(1字节)+KIiTA(1字节)
//	
//	fileAddr = SelectLFFile(G_CurrentDfName[G_ChannelNum], EF_KI);
//	
//	if(fileAddr == FALSE)
//	{
//		fileAddr = SelectLFFile(DF_PROFILECFG, EF_KI);
//	}

//	if(fileAddr == FALSE)
//	{	
//		TechnicalProblem();
//		return;
//	} 
//	
//	ReadTFBuf(fileAddr, 0x00, &RecData[100], TUAK_KeySize);		//读出鉴权的KI
//	
//	if(mode == D_AUTHMODE_2G)
//	{
//		result = TUAK2G();
//	}
//	else
//	{
//		result = TUAK();		
//	}
//	
//	if(result == FALSE)
//	{
//		AuthenticateError();
//	}
//	else
//	{
//		CommandFinish(result);
//	}
//}

#undef KC
#undef IK
#undef CK
#undef RES
#undef AK
#undef MAC_



#endif
