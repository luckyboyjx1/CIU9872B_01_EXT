#include "../gw/PrivateAuthentication.h"

#include	"string.h"
#include   "SysPlatformConfigProfile.h"
#include "profile.h"
#include "../gw/CosPriAuthentication.h"

#if UICC_FUCTION_PRIVATEAUTH
__attribute__((section("PRIROTATA_ENTRY")))
void PriRotata_Process_JumpArea(BYTE input[16],BYTE output[16],BYTE r)
{
	PriRotata_Process(input,output,r);
}
__attribute__((section("PRIF2TOF5FUNCTION_ENTRY")))
void PriF2toF5Function_Process_JumpArea(BYTE * inData, BYTE input[16], BYTE output[16], BYTE mode)
{
	PriF2toF5Function_Process(inData,input,output,mode);
}
__attribute__((section("PRIF1FUNCTION_ENTRY")))
void PriF1Function_Process_JumpArea(BYTE * inData, BYTE input[16], BYTE output[16])
{
	PriF1Function_Process(inData,input,output);
}
__attribute__((section("PRICHECKSQN_ENTRY")))
BYTE PriCheckSQN_Process_JumpArea(BYTE * input, BYTE * inData)
{
	return PriCheckSQN_Process(input,inData);
}
__attribute__((section("PRIMILENAGE_ENTRY")))
BYTE PriMilenage_Process_JumpArea(BYTE * input, BYTE * output)
{
	return PriMilenage_Process(input, output);
}
__attribute__((section("PRIMILENAGE2G_ENTRY")))
void PriMilenage2G_Process_JumpArea(BYTE *input, BYTE * output)
{
	PriMilenage2G_Process(input,output);
}	
__attribute__((section("PRIUSIMAUTHENTICATE_ENTRY")))
BYTE PriUSIMAuthenticate_Process_JumpArea(BYTE mode, BYTE *input)
{
	return PriUSIMAuthenticate_Process(mode, input);
}

__attribute__((section("priauthfunctinonlist")))
PriAuthFunctionEntry const PriAuthFunctionList={
						PriRotata_Process_JumpArea,
						PriF2toF5Function_Process_JumpArea,
						PriF1Function_Process_JumpArea,
						PriCheckSQN_Process_JumpArea,
						PriMilenage_Process_JumpArea,
						PriMilenage2G_Process_JumpArea,
						PriUSIMAuthenticate_Process_JumpArea,
};


void PriRotata_Process(BYTE input[16],BYTE output[16],BYTE r)
{
	BYTE idata i, shift;
	BYTE xdata temp[16];

	shift = r >> 3;

	for(i=0; i<16; i++)
	{
		output[(i + 16 - shift) &0x0F] = input[i];
	}

    shift = r & 0x07;

	if(shift != 0x00)
	{
		for(i=0;i<16;i++)
		{
			temp[i] = ((output[i] << shift) | (output[(i+1)&0x0F] >> (8-shift)));
		}

		CosPriAuthFunctionList.EcpMem_Copy_Process_entry(output, temp, 16);
	}
}


void PriF2toF5Function_Process(BYTE * inData, BYTE input[16], BYTE output[16], BYTE mode)
{
	BYTE idata i;

#define	OPc__	(&inData[117])		//保存卡内鉴权用OP字段内容（16字节）
#define	R	(&inData[133])		//保存卡内鉴权常数R字段内容（5字节）
#define C	(&inData[138])		//保存卡内鉴权常数C字段内容（80字节）


	PriRotata_Process(input, output, R[mode]);

	for (i=0; i<16; i++)
	{
		output[i] ^= C[16*mode + i];
	}

	CosPriAuthFunctionList.AESEncrypt_Process_entry(output, output);

	for(i=0; i<16; i++)
	{
		output[i] ^= OPc__[i];
	}

	return;
}

void PriF1Function_Process(BYTE * inData, BYTE input[16], BYTE output[16])
{
	BYTE xdata sqnAMFsqnAMF[16];
	BYTE idata i;

#define SQN_		(&inData[18])		//保存网络下发的SQN字段（6字节）
#define	AMF_		(&inData[24])		//保存网络下发的AMF字段（2字节）
#define	OPc_		(&inData[117])		//保存卡内鉴权用OP字段内容（16字节）
#define	R_		(&inData[133])		//保存卡内鉴权常数R字段内容（5字节）
#define C_		(&inData[138])		//保存卡内鉴权常数C字段内容（80字节）

	for (i=0; i<2; i++)                 //得到SQN||AMF||SQN||AMF
	{
		CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&sqnAMFsqnAMF[i*8], &SQN_[0], 6);

		CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&sqnAMFsqnAMF[i*8+6], &AMF_[0], 2);
	}

	for (i=0; i<16; i++)
	{
		sqnAMFsqnAMF[i] = sqnAMFsqnAMF[i] ^ OPc_[i];
	}

	PriRotata_Process(&sqnAMFsqnAMF[0], &output[0], R_[0]);

	for(i=0; i<16; i++)
	{
		output[i] ^= input[i];
	}

	for(i=0; i<16; i++)
	{
		output[i] ^= C_[i];
	}

	CosPriAuthFunctionList.AESEncrypt_Process_entry(output, output);

	for (i=0; i<16; i++)
	{
		output[i] ^= OPc_[i];  // the first 8 byte of output is just XMAC
	}

	return;
}

BYTE PriCheckSQN_Process(BYTE * input, BYTE * inData)
{
	BYTE  		idata index;
	signed char 	idata i;
	BYTE  		xdata seq[6], seqms[6];
	BYTE		xdata delta[6], tmp[6], L[6];
	BYTE		idata ind, deltaC;
	BYTE        xdata sqnMax[6];  //获取当前文件中的最大SQN 值
	
	BYTE        xdata indexSeq[0x06];
	BYTE		xdata SQN[6];
			
	BYTE * tempBuf;
#if U8_2_U32_INVERSE
    BYTE        xdata zero[6];
      
    CosPriAuthFunctionList.EcpMem_Set_Process_entry(&zero[0],0x00,sizeof(zero));    
#endif
	
	tempBuf = CosPriAuthFunctionList.GetGtempBuf_Process_entry();
	CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&SQN[0],&input[0], 6);
	CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&tempBuf[0],&inData[0], 8);
	
	ind = tempBuf[0];

	CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&L[0],&tempBuf[1],sizeof(L));

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

	CosPriAuthFunctionList.EcpMem_Set_Process_entry(sqnMax, 0x00, sizeof(sqnMax));
	
	CosPriAuthFunctionList.GetOrSetSqn_Process_entry(SQN_MAX_Mode,0x00, sqnMax);

	for(i=5; i>=0; i--)					//get seqms
	{
		seqms[i] = sqnMax[i] >> ind;

		if(i > 0x00)
		{
			seqms[i] |= sqnMax[i-1]<<(8-ind);
		}
	}

	if(CosPriAuthFunctionList.ecpMem_Compare_Process_entry(&seq[0], &seqms[0], 0x06) > EQUAL)
	{
		CosPriAuthFunctionList.StringSub_Process_entry(&seq[0], &seqms[0], &tmp[0], 0x06);

		CosPriAuthFunctionList.EcpMem_Set_Process_entry(&delta[0], 0x00, sizeof(delta));

		i = deltaC >> 3;

		delta[5-i] = 0x01 << (deltaC & 0x07);				//根据卡内配置的数值n，计算delta=2^n

		if(CosPriAuthFunctionList.ecpMem_Compare_Process_entry(&tmp[0], &delta[0], 0x06) > EQUAL)  	// compare with delta
		{                  
			CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&input[0], sqnMax, 0x06);//替换当前数据的SQN为卡内的最大的SQNms

			return D_F1Start_Mode;
				
		}
	}
	else		  										//添加对"L"的判断				 
	{	

#if U8_2_U32_INVERSE
        	if (CosPriAuthFunctionList.ecpMem_Compare_Process_entry(&L[0], &zero[0], 0x06) != EQUAL)	//当L等于0时不对L进行判断
#else
        	if (CosPriAuthFunctionList.ecpMem_Compare_Process_entry(&L[0], (BYTE CONST*)&zero[0], 0x06) != EQUAL)	//当L等于0时不对L进行判断
#endif
		{	
			CosPriAuthFunctionList.StringSub_Process_entry(&seqms[0], &seq[0], &tmp[0], 0x06);
	
			if(CosPriAuthFunctionList.ecpMem_Compare_Process_entry(&tmp[0], &L[0], 0x06) >= EQUAL)  	  	//SQN - SQNms < L
			{
				CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&input[0], sqnMax, 0x06);//替换当前数据的SQN为卡内的最大的SQNms

				return D_F1Start_Mode;
				
			}
		}
	}
	
	CosPriAuthFunctionList.GetOrSetSqn_Process_entry(INDEX_SQN_Mode, index, delta);
	
	for(i=5; i>=0; i--)					//当前索引的seq值
	{
		indexSeq[i] = delta[i] >> ind;

		if(i > 0x00)
		{
			indexSeq[i] |= delta[i-1]<<(8-ind);
		}
	}
	
	if(CosPriAuthFunctionList.ecpMem_Compare_Process_entry(&seq[0], &indexSeq[0], 0x06) > EQUAL)  //由于文件初始化为全0 ，如果直接比较SQN ，则会导致当下行SEQ为0，但是SQN不为0时，该比较成立，其实不应该成立
	{
		//WriteLFBuf(fileAddr, index, 0x00, &SQN[0], 0x06);        //更新当前的Index的SQN内容
		CosPriAuthFunctionList.GetOrSetSqn_Process_entry(SET_SQN_Mode, index, SQN);

		return TRUE;
	}

	CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&input[0], sqnMax, 0x06);//替换当前数据的SQN为卡内的最大的SQNms

	return D_F1Start_Mode;	
}

BYTE PriMilenage_Process(BYTE *input, BYTE *output)
{
	BYTE  idata i;
	BYTE  xdata tmp[16], xRes[16], eKresult[16];
	
	#define	RAND	(&input[1])		//保存网络下发的随机数（16字节）
	//#define	AUTH	(&input[18])		//保存网络下发的AUTH字段（16字节）
	#define SQN		(&input[18])		//保存网络下发的SQN字段（6字节）
	#define	AMF		(&input[24])		//保存网络下发的AMF字段（2字节）
	#define	MAC		(&input[26])		//保存网络下发的MAC字段（8字节）
	//#define	ki		(&input[100])		//保存卡内鉴权的Key（16字节）
	#define	OPMode	(input[116])		//保存卡内鉴权用OP字段的模式（Bit2）和是否SQN需要AK进行异或（Bit1）（1字节）
	#define	OPc		(&input[117])		//保存卡内鉴权用OP字段内容（16字节）
	//#define	R		(&input[133])		//保存卡内鉴权常数R字段内容（5字节）
	//#define C		(&input[138])		//保存卡内鉴权常数C字段内容（80字节）
	#define RESLGTH	(input[226])		//保存RES输出长度(1字节)
	
	
	for(i=0; i<16; i++)
	{
		tmp[i] = RAND[i] ^ OPc[i];					//计算出随机数和OPc的异或结果
	}

	CosPriAuthFunctionList.AESEncrypt_Process_entry(&tmp[0], &tmp[0]);					//AES计算,得到F1到F5的公用输入参数

	CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&eKresult[0], &tmp[0], 16);				//eKresult保存了F1的输入参数

	for(i=0; i<16; i++)								//经过AES计算的结果和OPc进行异或
	{
		tmp[i]  ^= OPc[i];					//tmp内保存了F2到F5的输入参数
	}

	//CosPriAuthFunctionList.F2toF5Function_Process_entry(tmp, xRes, D_F2_Mode);//得到xRES和AK的内容，前6个字节是AK，后8个字节是xRes的值
		
	PriF2toF5Function_Process(input, tmp, xRes, D_F2_Mode);//得到xRES和AK的内容，前6个字节是AK，后8个字节是xRes的值

	if((OPMode&D_F5_Mode) == D_F5_Mode)				//需要还原SQN的内容
	{
		for(i=0; i<6; i++)
		{
			SQN[i] = xRes[i] ^ SQN[i];			
		}
	}

	//CosPriAuthFunctionList.F1Function_Process_entry(eKresult, output);//计算xMac
	PriF1Function_Process(input, eKresult, output);//计算xMac
	
	if(CosPriAuthFunctionList.ecpMem_Compare_Process_entry(&output[0], &MAC[0], 8) != EQUAL)
	{
		return FALSE;
	}


	i = PriCheckSQN_Process(SQN,&input[218]);

	if(i == FALSE)
	{
		return FALSE;
	}

	if(i == D_F1Start_Mode)
	{
		AMF[0] = 0x00;

		AMF[1] = 0x00;                                   //计算 MAC-S时，AMF设置为全0，参考TS133102                     

		//CosPriAuthFunctionList.F1Function_Process_entry(eKresult, output);					//计算xMac
		PriF1Function_Process(input,eKresult, output);					//计算xMac
		
		//CosPriAuthFunctionList.F2toF5Function_Process_entry(tmp, xRes, D_F5Start_Mode);		//得到F5*计算AK*的内容
		PriF2toF5Function_Process(input, tmp, xRes, D_F5Start_Mode);		//得到F5*计算AK*的内容

		if((OPMode&D_F5_Mode) == D_F5_Mode)				//需要还原SQN的内容
		{
			for(i=0; i<6; i++)
			{
				SQN[i] = xRes[i] ^ SQN[i];			
			}
		}

		output[0] = 0xDC;

		output[1] = 0x0E;

		CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&output[2], SQN, 6);

		return 16;
	}
	else
	{
		output[0] = 0xDB;
	
		output[1] = RESLGTH;			  				//RES
		
		//CosPriAuthFunctionList.F2toF5Function_Process_entry(tmp, xRes, D_F2_Mode);			 //得到xRES和AK的内容前8个字节是xRes的值
		PriF2toF5Function_Process(input, tmp, xRes, D_F2_Mode);			 //得到xRES和AK的内容前8个字节是xRes的值

		CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&output[2], &xRes[8], RESLGTH);
	
		//CosPriAuthFunctionList.F2toF5Function_Process_entry(tmp, xRes, D_F3_Mode);			//得到xRES和CK的内容
		PriF2toF5Function_Process(input, tmp, xRes, D_F3_Mode);			//得到xRES和CK的内容
	
		output[2+RESLGTH] = 16;
		
		CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&output[3+RESLGTH], xRes, 16);
	
		//CosPriAuthFunctionList.F2toF5Function_Process_entry(tmp, xRes, D_F4_Mode);			//得到xRES和IK的内容
		PriF2toF5Function_Process(input, tmp, xRes, D_F4_Mode);			//得到xRES和IK的内容
	
		output[19+RESLGTH] = 16;
		
		CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&output[20+RESLGTH], xRes, 16);
				
		return 36+RESLGTH;
	}
}


void PriMilenage2G_Process(BYTE * input, BYTE * output)
{
	BYTE xdata tmp[16], sRes[4];
	BYTE idata i;

	for(i=0; i<16; i++)
	{
		tmp[i] = RAND[i] ^ OPc[i];
	}	

	CosPriAuthFunctionList.AESEncrypt_Process_entry(tmp, tmp);							//得到输入参数Ek的内容

	for(i=0; i<16; i++)								//经过AES计算的结果和OPc进行异或
	{
		tmp[i] = tmp[i] ^ OPc[i];					//tmp内保存了F2到F5的输入参数
	}

	//CosPriAuthFunctionList.F2toF5Function_Process_entry(tmp, output, D_F2_Mode);			//得到sRes的值

	PriF2toF5Function_Process(input, tmp, output, D_F2_Mode);			//得到sRes的值

	CosPriAuthFunctionList.ecpMem_Move_Process_entry(&output[0], &output[8], 8);

	CosPriAuthFunctionList.EcpMem_Set_Process_entry(&output[8], 0x00, 8);

	for (i=0; i<4; i++)
	{
		sRes[i] = output[i] ^ output[i + 4] ^ output[i + 8] ^ output[i + 12];
	}

	//CosPriAuthFunctionList.F2toF5Function_Process_entry(tmp, &output[0], D_F3_Mode);		//得到CK的值
	PriF2toF5Function_Process(input, tmp, &output[0], D_F3_Mode);		//得到CK的值

	//CosPriAuthFunctionList.F2toF5Function_Process_entry(tmp, &output[16], D_F4_Mode);	//得到IK的值
	PriF2toF5Function_Process(input, tmp, &output[16], D_F4_Mode);	//得到IK的值

	for (i=0; i<8; i++)
	{
		tmp[i] = output[i] ^ output[i+8] ^ output[16+i] ^ output[16+i+8];
	}

	output[0] = 0x04;

	CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&output[1], sRes, 4);

	output[5] = 0x08;

	CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&output[6], tmp, 8);

	return;
}



BYTE PriUSIMAuthenticate_Process(BYTE mode, BYTE *input)
{
	BYTE  idata result, status;
	BYTE  idata filepath[5]={0x02, 0x7F, 0xF0, 0x6F,0x38};//0x02:当前访问文件的路径级别；文件路径7FF0/6F38
	BYTE  idata RESLEN;
	BYTE  * tempBuf;

	tempBuf = CosPriAuthFunctionList.GetGtempBuf_Process_entry();
	
	RESLEN = input[226];
	
	CosPriAuthFunctionList.KeySchedule_Process_entry(&input[100]);//利用Ki进行AES Key的初始化

	if((input[116]&D_OPc) == D_OPc)						//如果需要计算OPc字段
	{
		CosPriAuthFunctionList.AESEncrypt_Process_entry(&input[117], &tempBuf[0]);
		
		for(result=0; result<16; result++)
		{
			input[117+result] ^= tempBuf[result];
		}
	}

	if(mode == D_AUTHMODE_2G)
	{
		PriMilenage2G_Process(input,&tempBuf[0]);						 	//处理在3G网络中2G的鉴权算法
	
		CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&input[0], &tempBuf[0], 14);
		
		return 14;
	}
	else
	{
		result = PriMilenage_Process(input, &tempBuf[0]);						//处理3G的鉴权算法		

		if(result == (36+RESLEN))							//鉴权成功
		{
			CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&input[0], &tempBuf[0], 36+RESLEN);
			
			CosPriAuthFunctionList.ReadTFBuf_Process_entry(&filepath[0],0x03, &status, sizeof(BYTE));
			
			if((status&0x04) == 0x04)           			//得到kc
			{
				input[36+RESLEN] = 0x08;
	
				for(result=0; result<8; result++)
				{
					input[37+RESLEN+result] =  input[3+RESLEN+result] ^ input[3+RESLEN+result+8] ^ input[20+RESLEN+result] ^ input[20+RESLEN+result+8];
				}

				return 45+RESLEN;
			}
			else
			{
				return 36+RESLEN;
			}
		
		}
		else
		{	
			
			if(result == 16)								//重同步失败
			{
				CosPriAuthFunctionList.EcpMem_Copy_Process_entry(&input[0], &tempBuf[0], 16);

				return 16;
			}
			else												//MAC or SQN 非法
			{
				return FALSE;
			}
		}
		
	}
}

#endif
