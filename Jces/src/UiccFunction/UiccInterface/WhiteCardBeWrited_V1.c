/*
 * HISTROY:	V1.0  开发根据 《中国移动白卡入网卡片技术要求_v1_0_0_20180601》，20180607   
 		
 	             

*/
#include  "ecpDefine.h"
#include  "string.h"
#include  "error.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if UICC__APP__WHITECARD

u8 X_A keyBuf[16];
u8 X_A keyResultBuf[16];
u8 X_A WhiteCardBuf[0xFF];
u8 X_A RandomBuf[16];

u8    X_A    G_personalDataTagM;
u8    X_A    G_personalDataTagO;
u8    X_A    G_AID_tag;
u8    X_A    G_Lgth_P3;
u8    X_A    G_SDK_command_step;


u8 CONST WhiteCardWriteSuc[]={
	//写卡成功，请您在30秒后重启手机
0x20,0x51,0x99,0x53,0x61,0x62,0x10,0x52,0x9F,0xFF,0x0C,0x8B,0xF7,0x60,0xA8,0x57,0x28,0x00,0x33,0x00,0x30,0x79,0xD2,0x54,0x0E,0x91,0xCD,0x54,0x2F,0x62,0x4B,0x67,0x3A
};

u8 CONST WhiteCardWriteCardErr[]={
	// 0   写卡失败，请携带有效证件前往移动营业厅处理
42,0x51,0x99,0x53,0x61,0x59,0x31,0x8D,0x25,0xFF,0x0C,0x8B,0xF7,0x64,0x3A,0x5E,0x26,0x67,0x09,0x65,0x48,0x8B,0xC1,0x4E,0xF6,0x52,0x4D,0x5F,0x80,0x79,0xFB,0x52,0xA8,0x84,0x25,0x4E,0x1A,0x53,0x85,0x59,0x04,0x74,0x06
};


/*
 *@description:更新PIN1-09,PIN2-0A,PUK1-0B,PUK2-0C
*/
u8 UpdateKeyWK(void)
{
	memref I_A fileAddr;
	
	//00DC010414  010133AA31323334FFFFFFFF 3131313131313131(9000);01：PIN1/PUK1
	//00DC020414  810133AA 35363738FFFFFFFF 3232323232323232(9000);81：PIN2/PUK2
	
	fileAddr = SelectLFFile(0x3F00, 0x2E10);
	//ReadLFBuf(fileAddr,0x01,0x02,ResData[0],0x02;);
	//ResData[0] = ResData[0]
	
	if(RecData[0] == 0x18)//PIN1
	{
		ResData[0] = 0x33;
		ResData[1] = 0xAA;
		
		WriteLFBuf(fileAddr,1,2,&ResData[0],2);//修改PIN，PUK的次数为初始值
		WriteLFBuf(fileAddr,1,4,&RecData[2],8);
	}
	else if(RecData[0] == 0x19)//PIN2
	{
		WriteLFBuf(fileAddr,2,4,&RecData[2],8);
	}
	else if(RecData[0] == 0x1A)//PUK1
	{
		WriteLFBuf(fileAddr,1,12,&RecData[2],8);
	}
	else if(RecData[0] == 0x1B)//PUK2
	{
		WriteLFBuf(fileAddr,2,12,&RecData[2],8);
	}
	else
	{
		return FALSE;
	}
		

	return TRUE;
}

/*--------------------------------------------------------------------------------------------------------------------

  Function name:	ResponseFunction

  Input parameter:	SW_1,SW_2:期望返回的状态字

  Output parameter:	Nlll

  Description:		用于私有指令在执行过程中，期望返回的状态字

  Author:			wangyanwen

  Test:				

  Status:			OK
 -------------------------------------------------------------------------------------------------------------------*/


void ResponseFunction(u8 SW_1,u8 SW_2)
{
	SW1 = SW_1;
	SW2 = SW_2;
	//SendResponse();
}

/*--------------------------------------------------------------------------------------------------------------------

  Function name:	SaveRandomNumber

  Input parameter:	RandomNu:需保存的随机数

  Output parameter:	Nlll

  Description:		保存当前命令产生的随机数，用于后续交互指令。

  Author:			wangyanwen

  Test:				

  Status:			OK
 -------------------------------------------------------------------------------------------------------------------*/
void SaveRandomNumber(u8 * RandomNu)
{
	memref X_A fileAddr;
	
	fileAddr = SelectTFFile(DF_STK,EF_WhiteCardRandom);
	WriteTFBuf(fileAddr,0x00,(u8 *)&RandomNu[0],0x04);
}

u8 CONST MACivData[]={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
/*
*@description: MAC的计算函数
*
*@function   : MAC_Calculate
*
*@param       :u8 keyIndex：密钥索引
*@param       :u8 *inputData 输入数据缓存
*@param       :u16  len     ：数据长度
*@param       :u8 * resultBuf :计算结果缓存
*/
void MAC_Calculate(u8 keyIndex,u8 *inputData,u16 len,u8 * resultBuf)
{
	memref X_A fileAddr;
			
	fileAddr = SelectLFFile(DF_STK, EF_WhiteCardKID);
	ReadLFBuf(fileAddr,keyIndex, 0,&tempBuf[0],16);
	
	fileAddr = SelectTFFile(DF_STK,EF_WhiteCardRandom);
	ReadTFBuf(fileAddr, 0,&RandomBuf[0],16);
	
	//AESCal(u8 * inputData, u8 *outputData, u16 len, u8 * keyBuf, u8 keyLen, u8 *ivData, u8 mode )
	AESCal((u8 *)RandomBuf, &keyBuf[0], 16, (u8 *)tempBuf, 16, (u8 *)MACivData, AES_CBC_ENCRYPT);
	//       Cmac(u8 *key, u8 keylen, u8 *M, u16 MLen, u8 *T)
	Cmac((u8 *)keyBuf, 16, (u8 *)inputData, len, NULL,0,&resultBuf[0]);
}
/*
*@description: TAC的计算函数
*
*@function   : TAC_Calculate
*

*@param       :u8 * resultBuf :计算结果缓存

*/
void TAC_Calculate(u8 * resultBuf)
{
	memref X_A fileAddr;
			
	fileAddr = SelectLFFile(DF_STK, EF_WhiteCardK2);
	ReadLFBuf(fileAddr,1, 0,&keyBuf[0],16);
	
	fileAddr = SelectTFFile(DF_MF, EF_seriesID);
	ReadTFBuf(fileAddr, 0, &WhiteCardBuf[0], 10);
	fileAddr = SelectTFFile(0x7F20, 0x6F07);
	ReadTFBuf(fileAddr, 0, &WhiteCardBuf[10], 9);
	fileAddr = SelectTFFile(DF_MF, EF_ICCID);
	ReadTFBuf(fileAddr, 0, &WhiteCardBuf[19], 10);
	
	//Cmac(u8 *key, u8 keylen, u8 *M, u16 MLen, u8 *T)
	Cmac((u8 *)keyBuf, 16, (u8 *)WhiteCardBuf, 29, NULL,0,&resultBuf[0]);
}

/*--------------------------------------------------------------------------------------------------------------------

  Function name:	SDK_GetInformation

  Input parameter:	NULL

  Output parameter:	NULL

  Description:		白卡入网私有指令，通过这个指令，获取卡片信息和初始化写卡

  Author:			wangyanwen
  
  Date:			20180709

  Test:				

  Status:		OK	
 -------------------------------------------------------------------------------------------------------------------*/

void SDK_GetInformation(void)
{
	u8 X_A offset;
	u8 X_A index;
	memref X_A fileAddr;
	
	if (GetWriteCardSupport(B_BKXK) == FALSE)
	{
		ResponseFunction(0x69,0x85);
		return;
	}
	
	if (P1 == 0x01)
	{
		if (GetWriteCardFlag() == TRUE)//已经写卡
		{
			ClearBit(G_SDK_command_step,B_WRITECARD);
			ClearBit(G_SDK_command_step,B_INITIALIZEFORWRITE);
			ClearBit(G_SDK_command_step,B_WRITECARD);
			
			ResponseFunction(0x69,0x01);
			return;
		}
	}
	
	if (G_AID_tag != G_ChannelNum)
	{
		ResponseFunction(0x6D,0x00);
		return;
	}
	
	if (G_AID_tag == 0xFF)
	{
		//6985	使用条件不满足(未选择一卡多号应用)  
		ResponseFunction(0x69,0x85);
		return;
	}

	if ((P1 != 0x00) && (P1 != 0x01))
	{
		ResponseFunction(0x6A,0x86);     

		return;
	}
	
	if (P2 != 0x00)//6A86 P1/P2错
	{
		//6A86	P1/P2错
		ResponseFunction(0x6A,0x86);
		
		return;
	}
	
	
	if (P3 == 0x00)
	{
		if (P1 == 0x00)//0x00=获取卡片信息命令
		{
			G_Lgth_P3 = 37;

		}
		else //初始化写卡(INITIALIZE FOR WRITE)命令
		{
			G_Lgth_P3 = 41;
			
		}
		
		ResponseFunction(0x6C,G_Lgth_P3);
		return;
			
	}
	else
	{
		if (G_Lgth_P3 == P3)
		{
			offset = 0;
	
			//add 空卡序列号
			RecData[offset] = 0x10;
			RecData[offset+1] = 0x0A;
			fileAddr = SelectTFFile(DF_MF, EF_seriesID);
			ReadTFBuf(fileAddr, 0, &RecData[offset+2], 10);
			
			//ICCID
			offset += 0x0C;
			
			RecData[offset] = 0x11;
			RecData[offset+1] = 0x0A;
			fileAddr = SelectTFFile(DF_MF, EF_ICCID);
			ReadTFBuf(fileAddr, 0, &RecData[offset+2], 10);
			
			//应用版本号
			offset += 0x0C;
		
			RecData[offset] = 0x12;
			RecData[offset+1] = 0x02;
			fileAddr = SelectTFFile(DF_STK, EF_WhiteCardConfig);
			ReadTFBuf(fileAddr, 0, &RecData[offset+2], 2);
				
			offset += 0x04;
		
			RecData[offset] = 0x13;
			RecData[offset+1] = 0x01;
			fileAddr = SelectTFFile(DF_STK, EF_WhiteCardConfig);
			ReadTFBuf(fileAddr, 2, &RecData[offset+2], 1);
			
			offset += 0x03;
			
			//卡状态标识
			if (P1 == 0x00)//0x00=获取卡片信息命令
			{
				//TAC  计算内容：空卡序列号、IMSI、ICCID，采用TAC秘钥（K2）进行计算得出
				
				RecData[offset] = 0x14;
				RecData[offset+1] = 0x04;
				
				
				//参与TAC计算的数据包括：空卡序列号、IMSI、ICCID
							
				TAC_Calculate(&keyResultBuf[0]);
				
				ecpMem_Copy((u8 *)&RecData[offset+2], &keyResultBuf[0], 4);
				
				SetBit(G_SDK_command_step,B_GETCARDINFO);
				
				SendData();
				ResponseFunction(0x90,0x00);//CommandFinish(0x00);//90 00
				
			}
			else if (P1 == 0x01)//初始化写卡(INITIALIZE FOR WRITE)命令
			{	
				//卡片随机数
				RecData[offset] = 0x15;
				RecData[offset+1] = 0x04;
				Random_Number_Generator_Get(&RecData[offset+2], 4);
				SaveRandomNumber(&RecData[offset+2]);
	
				index = (RecData[offset+5]&0x0F)+1;//密钥索引
				
				//MAC   空卡序列号（TLV）、ICCID(TLV)、应用版本号(TLV)、卡状态标识(TLV)、卡片随机数(TLV) 字段参与计算MAC计算
				offset += 0x06;
				//TAC_MAC_Calculate(u8 tpye,u8 keyIndex,u8 *inputData,u16 len,u8 * resultBuf);
				ecpMem_Copy(&WhiteCardBuf[0],&RecData[0],offset);
				MAC_Calculate(index,&WhiteCardBuf[0],offset,&keyResultBuf[0]);
				ecpMem_Copy(&RecData[offset],&keyResultBuf[0],4);
				
				SetBit(G_SDK_command_step,B_INITIALIZEFORWRITE);
				
				SendData();
				ResponseFunction(0x90,0x00);//CommandFinish(0x00);//90 00
			}
			else
			{
				//6A86	P1/P2错
				ResponseFunction(0x6A,0x86);
				return;
			}
		}
		else
		{
			//6700	P3错
			
			ResponseFunction(0x67,0x00);
			return;
		}
	}
	
	
}

/*--------------------------------------------------------------------------------------------------------------------

  Function name:	SDK_WhiteCardWriteCardHandler

  Input parameter:	NULL

  Output parameter:	NULL

  Description:		白卡入网写卡指令的处理过程

  Author:			wangyanwen
  
  Date:			20180709

  Test:				

  Status:			
 -------------------------------------------------------------------------------------------------------------------*/

void SDK_WhiteCardWriteCardHandler(void)
{
	memref X_A fileAddr;
	u8 X_A dataLgth;
	u8 X_A result,index;
	u8 I_A dataMAC[4];
	u8 X_A MACLgth;


	ReceiveData();
	
	if (G_AID_tag != G_ChannelNum)
	{
		ResponseFunction(0x6D,0x00);
		return;
	}
	
	if (G_AID_tag == 0xFF)
	{
		//6985	使用条件不满足(未选择一卡多号应用)
		ResponseFunction(0x69,0x85);
		return;
	}
	
	if ((P1 != 0x00) || (P2 != 0x00))//6A86 P1/P2错
	{
		//6A86	P1/P2错
		ResponseFunction(0x6A,0x86);
		return;
	}
	

	if (GetWriteCardFlag() == TRUE)//已经写卡
	{
		ClearBit(G_SDK_command_step,B_WRITECARD);
		ClearBit(G_SDK_command_step,B_INITIALIZEFORWRITE);
		
		ResponseFunction(0x69,0x01);
		return;
	}
	
	if(ValueBit(G_SDK_command_step, B_INITIALIZEFORWRITE) == FALSE)
	{
		//ClearBit(G_SDK_command_step,B_WRITECARD);
		//ClearBit(G_SDK_command_step,B_INITIALIZEFORWRITE);
		
		ResponseFunction(0x69,0x02);//未进行写卡初始化，空闲状态下返回6902
		return;
	}
	
	//APDU: 00A5000086 1504BBF25E01 89FBB85D07B93F8A51F0101260FC5B5BD99DC7AE35FEBE1F0872A006D7DB53D6D00B9974390A6EC73D0D95A19F3778796B208AF5CE982C5452B269AEFF3FF301A292AB33269A781AAC9AB671B2F87E14F164A2B244B23E1A0E74B1600CEE5CE943D14A155CD7E4D0C9B9F84EF66ECC6BB5AC895BDA7F1C6F5B3EA2882425CC5E	
	
	//**********比较MAC***********************
	ecpMem_Copy(&ResData[0],&RecData[0],6);//add 随机数
	//**********比较MAC***********************
	
	fileAddr = SelectTFFile(DF_STK, EF_WhiteCardRandom);
	ReadTFBuf(fileAddr, 0,&RandomBuf[0],16);
	
	if(ecpMem_Compare((u8 *)RandomBuf, (u8 *)&RecData[2],4) != EQUAL)
        {
        	ResponseFunction(0x9D,0x00);//随机数错误
            	return ;
        }
	
	index = (RandomBuf[3]&0x0F)+1;//密钥索引=KIC高半字节
	
	
	fileAddr = SelectLFFile(DF_STK, EF_WhiteCardKIC);
	ReadLFBuf(fileAddr,index, 0,&WhiteCardBuf[0],16);

	//u8 AESCal(u8 * inputData, u8 *outputData, u16 len, u8 * keyBuf, u8 keyLen, u8 *ivData, u8 mode )
	result = AESCal((u8 *)RandomBuf, &keyBuf[0], 16, (u8 *)WhiteCardBuf, 16, (u8 *)MACivData, AES_CBC_ENCRYPT);

	dataLgth = P3-6;
	
	//  u8 AESCal(u8 * inputData, u8 *outputData, u16 len, u8 * keyBuf, u8 keyLen, u8 *ivData, u8 mode )
	result = AESCal((u8 *)&RecData[6], &WhiteCardBuf[0], dataLgth, (u8 *)keyBuf, 16, (u8 *)MACivData, AES_CBC_DECRYPT);
	
	
	if ((WhiteCardBuf[0] != 0x1E) || (WhiteCardBuf[1] != 0x01) || (WhiteCardBuf[3] != 0x1F))
	{
		SetBit(G_SDK_command_step,B_WRITECARD);
		AppWriteResult = 0x03;
		ResponseFunction(0x9E,0x09);	
			
		return;
	}
	
	if (WhiteCardBuf[2] >= 0x10) //PCNTR的值不应该大于等于16
	{
		SetBit(G_SDK_command_step,B_WRITECARD);
		AppWriteResult = 0x03;
		ResponseFunction(0x9E,0x09);	
			
		return;
	}
	
	//1E01071F70BFD17696978F2D123DDC7239180474D5E9DA850E0CB67C06CA63F3AC47E970E68460D4F9F53AC44EFB2C9418F96C3B96EA898C3FF22A7D37772F7E848AB02DA19840E479EA326DCB66A2A3DA1712883FD41D6DD00781AEDB8E5124F4036D47B3D581A9B0D178E705ED2A6A5A3117BD49 91EAB2E0
	//AES 加密前： 1E01071F70A334F5ED321F114D8597369515C1D1E18E50C83CB36EFC5BEE12465C477A1F26AFA8632F483554F0135484452292B6C0C67AA1717EC3503733AE14B79171CBD917F38CDE73D7C24DBC678290F4390869769BBB5D1E31DAA38054D823FC175094C1C614D3B76A6217797C67558237870A EC12F7D8 80000000000000
	
	//**********比较MAC*********start**************
	//注2：随机数(TLV)、PCNTR(TLV)、命令域(TLV)参与MAC计算，使用KID密钥进行一级分散计算 。
	
	MACLgth = WhiteCardBuf[4]+5;
	ecpMem_Copy(&ResData[6],&WhiteCardBuf[0],MACLgth);//add PCNTR
	
	ecpMem_Copy(&dataMAC[0],&WhiteCardBuf[MACLgth],4);//add PCNTR
	
	MACLgth += 6;
	
	MAC_Calculate(index,&ResData[0],MACLgth,&keyResultBuf[0]);

	if (ecpMem_Compare(&dataMAC[0], &keyResultBuf[0], 4) != EQUAL)
	{
		SetBit(G_SDK_command_step,B_WRITECARD);
		AppWriteResult = 0x02;
		ResponseFunction(0x9E,0x09);	
			
		return;
	}
	//**********比较MAC*******end****************
	
	dataLgth = WhiteCardBuf[4];
	
	ecpMem_Copy(&dataMAC[0],&WhiteCardBuf[dataLgth+5],4);
	
	fileAddr = SelectLFFile(DF_STK, EF_WhiteCardK1);
	ReadTFBuf(fileAddr,0,&RecData[0],16);
	
	result = AESCal((u8 *)RandomBuf, &keyBuf[0], 16, (u8 *)RecData, 16, (u8 *)MACivData, AES_CBC_ENCRYPT);
	
        //  u8 AESCal(u8 * inputData, u8 *outputData, u16 len, u8 * keyBuf, u8 keyLen, u8 *ivData, u8 mode )
	result = AESCal((u8 *)&WhiteCardBuf[5], &RecData[0], dataLgth, (u8 *)keyBuf, 16, (u8 *)MACivData, AES_CBC_DECRYPT);
	
	//110A986800214365870921431609084906001111212299170891683108706505F0180831323334FFFFFFFF190835363738FFFFFFFF1A0837353833363336331B0837353833363336331C100A62DDE4C77210DAD5FC47F1693124501D10D8B125482017C5370C884D661BF8BF99800000 

	result = WhiteCardWriteCardCheakCommand((u8 *)RecData,dataLgth);


	if ((result == 0x01) || (G_personalDataTagM != 0x7F) || (G_personalDataTagO == 0x01) || (G_personalDataTagO == 0x02))//写卡数据异常
	{
		SetBit(G_SDK_command_step,B_WRITECARD);
		AppWriteResult = 0x03;
		ResponseFunction(0x9E,0x09);	
		
		return;
	}
	
	//fileAddr = SelectLFFile(DF_STK, EF_WhiteCardBuf);
	//EF_WhiteCardBuf文件去掉，共用EF_EnvelopeCBMessage文件,by xwh 20210625: EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
	fileAddr = SelectPrivateLFFile(DF_Toolkit, EF_EnvelopeCBMessage);
	SetLFConst(fileAddr,1, 0, dataLgth, 0x01);	
	WriteLFBuf(fileAddr,1,1,&RecData[0],dataLgth);
	
	PowerOffProtection(1);
	
	WhitrCardWriteCardProcess();
	ClearLociSqn();
	SetWriteCardFlag();
	
	SetBit(G_SDK_command_step,B_WRITECARD);
	
	PowerOffProtection(2);
	
	AppWriteResult = 0x00;
	ResponseFunction(0x9E,0x09);
			
}
/*--------------------------------------------------------------------------------------------------------------------

  Function name:	GetWriteCardCommandResult

  Input parameter:	NULL

  Output parameter:	APP写卡命令后的返回结果

  Description:		App通过向卡发送该命令，可以实获取卡片执行APP写卡命令后的返回结果。

9000	写卡数据未接收完整，卡片等待接收后续数据
6985或者6D00	使用条件不满足(未选择白卡入网应用)
6A86	P1/P2错
6700	P3错
9E0X	取响应数据
6903	状态机错误

  Author:			wangyanwen

  Test:				

  Status:		OK	
 -------------------------------------------------------------------------------------------------------------------*/
void SDK_GetWriteCardCommandResult(void)
{
	memref X_A fileAddr;
	u8 I_A index;

	if (G_AID_tag != G_ChannelNum)
	{
		ResponseFunction(0x6D,0x00);
		return;
	}
	
	if (G_AID_tag == 0xFF)
	{
		ResponseFunction(0x69,0x85);
		return;
	}
	
	
	if((P1 != 0x00) || (P2 != 0x00))
	{
		ResponseFunction(0x6A,0x86);
		return ;	
	}
	
	if (P3 != 0x09)
	{
		ResponseFunction(0x67,0x00);
	}
	
	if(ValueBit(G_SDK_command_step, B_WRITECARD) == FALSE)
	{
		//ClearBit(G_SDK_command_step,B_WRITECARD);
		ClearBit(G_SDK_command_step,B_INITIALIZEFORWRITE);
		
		ResponseFunction(0x69,0x03);//未写卡
		return;
	}
	
	if(ValueBit(G_SDK_command_step, B_INITIALIZEFORWRITE) == FALSE)
	{
		ClearBit(G_SDK_command_step,B_WRITECARD);
		//ClearBit(G_SDK_command_step,B_INITIALIZEFORWRITE);
		
		ResponseFunction(0x69,0x03);//未进行写卡初始化
		return;
	}
	
	ClearBit(G_SDK_command_step,B_WRITECARD);
	ClearBit(G_SDK_command_step,B_INITIALIZEFORWRITE);
		
	//写卡状态标识
	RecData[0] = AppWriteResult;
	

	//TAC 
	
	//参与TAC计算的数据包括：空卡序列号、IMSI、ICCID
	
	if (AppWriteResult == 0x00)
	{
		TAC_Calculate(&keyResultBuf[0]);
	}	
	else
	{
		ecpMem_Set(&keyResultBuf[0], 0x00,4);
	}
	
	ecpMem_Copy(&RecData[1], &keyResultBuf[0], 4);
	
	//写卡状态标识(1字节)、TAC(4字节)、随机数（4字节）参与MAC计算，使用KID密钥进行一级分散计算
	//计算mac元素=0042BC4509497587AF
	//ecpMem_Set((u8 *)&WhiteCardBuf[0], 0x00, 16);
	WhiteCardBuf[0] = AppWriteResult;
	ecpMem_Copy(&WhiteCardBuf[1], &keyResultBuf[0], 4);
	fileAddr = SelectTFFile(DF_STK,EF_WhiteCardRandom);
	ReadTFBuf(fileAddr, 0, &WhiteCardBuf[5], 4);
	//WhiteCardBuf[9] = 0x80;
	
	index = (WhiteCardBuf[8]&0x0F)+1;
	MAC_Calculate(index,&WhiteCardBuf[0],9,&keyResultBuf[0]);
	
	ecpMem_Copy(&RecData[5], &keyResultBuf[0], 4);
	
	SendData();

	CommandFinish(0x00);//90 00
	return ;
	

}
/*--------------------------------------------------------------------------------------------------------------------

  Function name:	SDK_selectAID

  Input parameter:	NULL

  Output parameter:	TRUE：符合白卡入网的应用AID，返回TRUE，COS底层将不再处理此条指令后续流程
  			FALSE：不符合白卡入网的应用AID，返回FALSE，COS底层将继续处理此条指令

  Description:		选择白卡入网应用AID

  Author:			wangyanwen
  
  Date:			20180709

  Test:				

  Status:			
 -------------------------------------------------------------------------------------------------------------------*/
//u8 SDK_selectAID(void)
//{
//	memref X_A fileAddr;
//
//	fileAddr = SelectLFFile(DF_STK, EF_WhiteCardAID);
//
//	if(fileAddr == FALSE)
//	{
//		return FALSE;
//	}
//
//	ReadLFBuf(fileAddr, 1, 0, &WhiteCardBuf[0], 0x26);
//
//	if (WhiteCardBuf[0] == P3)
//	{
//		if (ecpMem_Compare(&RecData[0],&WhiteCardBuf[1],WhiteCardBuf[0]) == EQUAL)
//		{
//			if (GetWriteCardSupport(B_BKXK) == FALSE)
//			{
//				//ResponseFunction(0x69,0x85);
//				ResponseFunction(0x6D,0x00);
//				return TRUE;
//			}
//
//			if (P1 != 0x04)
//			{
//				ResponseFunction(0x6A,0x86);
//				return TRUE;
//			}
//
//			//步骤1：发送SELECT指令，P1=04，其中P2的bit1，bit2为非00,10的值，数据域为白卡入网应用的AID
//			if (((P2&0xA0) != 0x00) ||( (P2&0x01) == 0x01))
//			{
//				ResponseFunction(0x6A,0x86);
//				return TRUE;
//			}
//
//		 	if (((P2&0xF0) == 0x50) )
//			{
//			  	ResponseFunction(0x6A,0x86);
//				return TRUE;
//			}
//
//			G_SDK_command_step = 0;
//
//			G_AID_tag = G_ChannelNum;
//			ResponseFunction(0x90,0x00);
//			return TRUE;	   //支持部分选择白卡入网应用
//		}
//	}
//
//	return FALSE;
//}


/*--------------------------------------------------------------------------------------------------------------------

  Function name:	SendSMS_WriteCardApply

  Input parameter:	Retry：用于区别是否是重发时，如果是重发的，将不需要重新生成随机数

  Output parameter:	NULL

  Description:		白卡入网自申请短信

  Author:			wangyanwen
  
  Date:			20180709

  Test:				

  Status:		OK	
 -------------------------------------------------------------------------------------------------------------------*/
void SendSMS_WriteCardApply(u8 Retry)
{
	memref I_A fileAddr;
	u8 I_A offset;
	u8 I_A index,lgth;
	
	if (GetWriteCardSupport(B_BKXK) == FALSE)
	{
		return;
	}
	
	//**************************************************************************
	offset = 0;
	
	//空卡系列号
	offset+=4;
	
	//add 空卡序列号
	RecData[offset] = 0x10;
	RecData[offset+1] = 0x0A;
	fileAddr = SelectTFFile(DF_MF, EF_seriesID);
	ReadTFBuf(fileAddr, 0, &RecData[offset+2], 10);
	
	//ICCID
	offset += 0x0C;
	
	RecData[offset] = 0x11;
	RecData[offset+1] = 0x0A;
	fileAddr = SelectTFFile(DF_MF, EF_ICCID);
	ReadTFBuf(fileAddr, 0, &RecData[offset+2], 10);
	
	//应用版本号
	offset += 0x0C;

	RecData[offset] = 0x12;
	RecData[offset+1] = 0x02;
	fileAddr = SelectTFFile(DF_STK, EF_WhiteCardConfig);
	ReadTFBuf(fileAddr, 0, &RecData[offset+2], 2);
	
	//卡状态标识
	offset += 0x04;

	RecData[offset] = 0x13;
	RecData[offset+1] = 0x01;
	fileAddr = SelectTFFile(DF_STK, EF_WhiteCardConfig);
	ReadTFBuf(fileAddr, 2, &RecData[offset+2], 1);
	
	//卡片随机数
	offset += 0x03;

	RecData[offset] = 0x15;
	RecData[offset+1] = 0x04;
	
	if (Retry == 0x00)
	{
		Random_Number_Generator_Get(&RecData[offset+2], 4);
		SaveRandomNumber(&RecData[offset+2]);
	}
	else
	{
		fileAddr = SelectTFFile(DF_STK,EF_WhiteCardRandom);
		ReadTFBuf(fileAddr, 0, &RecData[offset+2], 4);
	}

	ecpMem_Set(&RandomBuf[0],0x00,16);
	ecpMem_Copy(&RandomBuf[0],&RecData[offset+2],4);
	RandomBuf[4]=0x80;
	
	index = (RecData[offset+5]&0x0F)+1;//密钥索引
	//MAC  命令类型(V)+命令长度(V)+空卡系列号(TLV)+ICCID(TLV)+应用版本(TLV)+卡状态标识(TLV)+随机数(TLV)
	
	offset += 0x06;
	
	RecData[0] = offset+3;
	RecData[1] = offset+2;
	RecData[2] = 0x01;//命令类型
	RecData[3] = offset;
	
	lgth = offset+2;
	ecpMem_Copy(&WhiteCardBuf[0],&RecData[2],lgth);

	
	lgth -= 4;//计算MAC时，实际数据长度需减去4个字节
	
	MAC_Calculate(index,&WhiteCardBuf[0],lgth,&keyResultBuf[0]);
	
	ecpMem_Copy(&RecData[offset],&keyResultBuf[0],4);
	//*******************************************************
	
	fileAddr = SelectTFFile(DF_STK, EF_WhiteCardNumber);
	ReadTFBuf(fileAddr, 0, &lgth, 1);
	ReadTFBuf(fileAddr, 1, &WhiteCardBuf[1], lgth);
	
	WhiteCardBuf[lgth+1] = 0x00;//pid
	WhiteCardBuf[lgth+2] = 0x04;//dcs
	
	ecpMem_Copy(&WhiteCardBuf[lgth+3],&RecData[1],RecData[0]);
	
	WhiteCardBuf[0] = lgth+RecData[0]+2;
	
	G_applySMS = 1;
	WhiteCardcephSendShortMessageSingle(0x00, NULL, NULL, (u8 *)&WhiteCardBuf[0],0x02);
	
	return ;
}


/*--------------------------------------------------------------------------------------------------------------------

  Function name:	SendSMS_NewNumberActivate

  Input parameter:	NULL

  Output parameter:	NULL

  Description:		白卡入网新号激活短信

  Author:			wangyanwen
  
  Date:			20180709

  Test:				

  Status:		OK	
 -------------------------------------------------------------------------------------------------------------------*/
void SendSMS_NewNumberActivate(void)
{
	memref I_A fileAddr;
	u8 I_A offset;
	u8 I_A lgth;

	offset = 0;
	
	RecData[0] = 17;
	RecData[1] = 16;
	RecData[2] = 0x04;//命令类型
	RecData[3] = 0x0E;//命令长度
	
	//空卡系列号
	offset+=4;
	
	//add 空卡序列号
	
	fileAddr = SelectTFFile(DF_MF, EF_seriesID);
	ReadTFBuf(fileAddr, 0, &RecData[offset], 10);
	
	//ICCID
	offset += 10;

	//参与TAC计算的数据包括：空卡序列号、IMSI、ICCID
	
	
	TAC_Calculate(&keyResultBuf[0]);
	
	//add TAC
	ecpMem_Copy(&RecData[offset],&keyResultBuf[0],4);
	
	fileAddr = SelectTFFile(DF_STK, EF_WhiteCardNumber);
	ReadTFBuf(fileAddr, 0, &lgth, 1);
	ReadTFBuf(fileAddr, 1, &WhiteCardBuf[1], lgth);
	
	WhiteCardBuf[lgth+1] = 0x00;//pid
	WhiteCardBuf[lgth+2] = 0x04;//dcs
	
	ecpMem_Copy(&WhiteCardBuf[lgth+3],&RecData[1],RecData[0]);
	
	WhiteCardBuf[0] = lgth+RecData[0]+2;
	
	//smsTPDU(L,Variable Reference,include: tpda,pid,dcs,udl,ud..)
	//cephSendShortMessageSingle(u8 commQualify,u8 CONST * alphaIdentifier, u8 * address, u8 * smsTPDU)
	
	G_applyActivate = 1;
	WhiteCardcephSendShortMessageSingle(0x00, NULL, NULL, (u8 *)&WhiteCardBuf[0],0x02);
	
	//SendSMS();

	return ;
}


/*--------------------------------------------------------------------------------------------------------------------

  Function name:	SendSMS_WriteCardResult

  Input parameter:	WriteCardResult:写卡结果

  Output parameter:	NULL

  Description:		白卡入网写卡结果返回

  Author:			wangyanwen
  
  Date:			20180709

  Test:				

  Status:		OK	
 -------------------------------------------------------------------------------------------------------------------*/
u8 SendSMS_WriteCardResult(u8 WriteCardResult)
{
	memref I_A fileAddr;
	u8 I_A result;
	u8 I_A index,lgth;

	RecData[0] = 8;
	RecData[1] = 7;
	RecData[2] = 0x03;//命令类型
	RecData[3] = 5;//命令长度
	RecData[4] = WriteCardResult;//写卡结果
	
	//参与MAC计算的数据包括：命令类型、命令长度、写卡结果、短信方式申请写卡上行报文中的随机数。
	
	fileAddr = SelectTFFile(DF_STK,EF_WhiteCardRandom);
	
	//RecData[5] = 0x
	ReadTFBuf(fileAddr, 0, &RecData[5], 4);
	index = (RecData[8]&0x0F)+1;//密钥索引
	
	ecpMem_Copy(&WhiteCardBuf[0],&RecData[2],7);

	MAC_Calculate(index,&WhiteCardBuf[0],7,&keyResultBuf[0]);
	
	//add MAC
	ecpMem_Copy(&RecData[5],&keyResultBuf[0],4);
	
	
	fileAddr = SelectTFFile(DF_STK, EF_WhiteCardPresetTPDA);
	ReadTFBuf(fileAddr, 0, &lgth, 1);
	
	if((lgth == 0x00) || (lgth == 0xFF))//如果预制有指定的上发地址，则回复短信使用预制的值，否则，使用下行数据的地址
	{
		fileAddr = SelectTFFile(DF_STK, EF_WhiteCardTPOA);
		ReadTFBuf(fileAddr, 0, &lgth, 1);
	}
	
	ReadTFBuf(fileAddr, 1, &WhiteCardBuf[1], lgth);
	
	WhiteCardBuf[lgth+1] = 0x00;//pid
	WhiteCardBuf[lgth+2] = 0x04;//dcs
	
	ecpMem_Copy(&WhiteCardBuf[lgth+3],&RecData[1],RecData[0]);
	
	WhiteCardBuf[0] = lgth+RecData[0]+2;

	result = WhiteCardcephSendShortMessageSingle(0x00, NULL, NULL, (u8 *)&WhiteCardBuf[0],0x01);

	return result;
}

/*--------------------------------------------------------------------------------------------------------------------

  Function name:	SMS_WhiteCardWriteCardHandler

  Input parameter:	NULL

  Output parameter:	NULL

  Description:		白卡入网短信写卡处理函数

  Author:			wangyanwen
  
  Date:			20180709

  Test:				

  Status:		OK	
 -------------------------------------------------------------------------------------------------------------------*/
u8 SMS_WhiteCardWriteCardHandler(void)
{
	u16 I_A dataLgth,MACLgth;
	u8 I_A result,index,Counter;
	memref X_A fileAddr;
	u8 I_A dataMAC[4];
	
//单条    0068 0D00006262B0012B914F6065734962082C0CB7B4501FB3ABC906428ECFDABE9D7D3548C23FE866D0F13539ABF7EF3CB8A170C082BC8679BBE187AB9B2D9A1CB51FC42655EC6EAEA8363C441FE8309E4F5503B8290EF68EDD2C5513D79C283174832E14ACF7AEBFE2
	
	if (GetWriteCardSupport(B_BKXK) == FALSE)
	{
		return 0;
	}
	
	if (G_SDK_command_step != 0x00)//当卡片处于app写卡流程时，再来短信指令，则无需响应短信写卡指令
	{
		return 0;
	}	
	
	if (GetWriteCardFlag() == TRUE)//已经写卡
	{
		return 0;
	}
		
	if (G_applySMS != 0x02)
	{	
		return 0;
	}
	
	index = (tempBuf[5]>>4)+1;//密钥索引=KIC高半字节
	
	dataLgth = tempBuf[0]*256+tempBuf[1]-8;
	
	fileAddr = SelectTFFile(DF_STK, EF_WhiteCardRandom);
	ReadTFBuf(fileAddr, 0,&RandomBuf[0],16);
	
	fileAddr = SelectLFFile(DF_STK, EF_WhiteCardKIC);
	ReadLFBuf(fileAddr,index, 0,&RecData[0],16);

	
	//u8 AESCal(u8 * inputData, u8 *outputData, u16 len, u8 * keyBuf, u8 keyLen, u8 *ivData, u8 mode )
	result = AESCal((u8 *)RandomBuf, &keyBuf[0], 16, (u8 *)RecData, 16, (u8 *)MACivData, AES_CBC_ENCRYPT);//产生解密会话密钥
	result = AESCal((u8 *)&tempBuf[10], &RecData[0], dataLgth, (u8 *)keyBuf, 16, (u8 *)MACivData, AES_CBC_DECRYPT);//解密
		
	//RecData = 000000000004 0174 88C2B878 D70CAB0A04E391F8B439EC0BC5071C3C2D4DB3BAD6949E8325CCFA8363381093ADB15050C11AF6FB6AE0BA07CB052A8243FC49C5C7560C3978DB058094BAFAE79B212B856738B18BBD96973F099FC98AB0BD141F06DA48F79A4154DBEAD37ADAC1E049D3E6E2FC6F7B92327C75F9EEED80000000
	//0000000000040154C36C4E02B47A655E2E27191F5DEB834B9EA1DA7B34C33B297F9EAA280DA3BAEB34FF858F9CFC65296C2BF5FE9AF3CAE51EB247BE952BBE723B9E2FDBDD48987871B103FBFDA539711DE00E5C4659655D4B14D3AE80000000CC0EF44B81976ED7CC70801F62608F177965332F175E60D551EC5B27E88EA8A087125B4DCC0EF44B81976ED7CC7080000000000000000000000000000000000000000000000000000000000000000000
	
	//**********比较MAC***********************
	//参与MAC计算的数据包括：CPL、CHL、SPI、KIC、KID、TAR、 CNTR、PCNTR、命令数据（不包括MAC本身数据，包括加密的Padding）。
	//单条                   00680D00006262B0012B 914F6065734962082C0CB7B4501FB3ABC906428ECFDABE9D7D3548C23FE866D0F13539ABF7EF3CB8A170C082BC8679BBE187AB9B2D9A1CB51FC42655EC6EAEA8363C441FE8309E4F5503B8290EF68EDD2C5513D79C283174832E14ACF7AEBFE2
	
	if (RecData[6] != 0x02)
	{
		Counter = 0x00;
	
		while(Counter<4)
		{
			result = SendSMS_WriteCardResult(WriteCard_DataWrong);
			
			if(result == SendSMS_NeedResent)
			{	
				Counter++;
			}
			else
			{
				break;
			}	
		}
		

		return 0;
	}
	
	ecpMem_Copy(&dataMAC[0],&RecData[8],4);
	
	ecpMem_Copy(&ResData[0],&tempBuf[0],10);//add CPL、CHL、SPI、KIC、KID、TAR
	ecpMem_Copy(&ResData[10],&RecData[0],8);//add CNTR、PCNTR、
	
	MACLgth = RecData[7]+RecData[5]-4;
	//MACLgth = RecData[7]+RecData[5];
	ecpMem_Copy(&ResData[18],&RecData[12],MACLgth);//add 命令数据（不包括MAC本身数据，包括加密的Padding）。、
	
	MACLgth += 18;
	//**********比较MAC***********************
	
	dataLgth = RecData[7] - 4;
	//dataLgth = RecData[7];
	
	ecpMem_Copy(&WhiteCardBuf[0],&RecData[12],dataLgth);
	
	fileAddr = SelectLFFile(DF_STK, EF_WhiteCardK1);
	ReadTFBuf(fileAddr,0,&RecData[0],16);
	
	result = AESCal((u8 *)RandomBuf, &keyBuf[0], 16, (u8 *)RecData, 16, (u8 *)MACivData, AES_CBC_ENCRYPT);//产生计算K1会话密钥
	
        //参与MAC计算的数据包括：CPL、CHL、SPI、KIC、KID、TAR、 CNTR、PCNTR、命令数据（不包括MAC本身数据，包括加密的Padding）。
        //00680D0000C2C2B0012B000000000004110A986800214365870921431609084906001111212299170891683108706505F0180831323334FFFFFFFF190835363738FFFFFFFF1A0837353833363336331B08373538333633363380000000000000
	result = AESCal((u8 *)&WhiteCardBuf[0], &RecData[0], dataLgth, (u8 *)keyBuf, 16, (u8 *)MACivData, AES_CBC_DECRYPT);//对个性数据进行解密
	
	//**********比较MAC***********************
	MAC_Calculate(index,&ResData[0],MACLgth,&keyResultBuf[0]);//计算MAC

	if (ecpMem_Compare(&dataMAC[0], &keyResultBuf[0], 4) != EQUAL)
	{
		Counter = 0x00;
	
		while(Counter<4)
		{
			result = SendSMS_WriteCardResult(WriteCard_DataMACNotMatch);//写卡数据MAC错误
			
			if(result == SendSMS_NeedResent)
			{
				Counter++;
			}
			else
			{
				break;
			}
		}

		return 0;
	}

	//**********比较MAC***********************
	tempBuf[0] = dataLgth;
	
	ecpMem_Copy(&tempBuf[1],&RecData[0],dataLgth);
	
	//fileAddr = SelectLFFile(DF_STK, EF_WhiteCardBuf);
	//EF_WhiteCardBuf文件去掉，共用EF_EnvelopeCBMessage文件,by xwh 20210625: EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
	fileAddr = SelectPrivateLFFile(DF_Toolkit, EF_EnvelopeCBMessage);
	WriteLFBuf(fileAddr,1,0,&tempBuf[0],dataLgth+1);
	//110A986800214365870921431609084906001111212299170891683108706505F0180831323334FFFFFFFF190835363738FFFFFFFF1A0837353833363336331B0837353833363336331C100A62DDE4C77210DAD5FC47F1693124501D10D8B125482017C5370C884D661BF8BF99000000 E6E2FC6F7B92327C75F9EEED80000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
	result = WhiteCardWriteCardCheakCommand((u8 *)RecData,dataLgth);

	if ((result == 0x01) || (G_personalDataTagM != 0x7F) || (G_personalDataTagO == 0x01) || (G_personalDataTagO == 0x02))//写卡数据异常
	{
		Counter = 0x00;
	
		while(Counter<4)
		{
			result = SendSMS_WriteCardResult(WriteCard_DataWrong);
			
			if(result == SendSMS_NeedResent)
			{	
				Counter++;
			}
			else
			{
				break;
			}	
		}

		return 0;
	}
	
	Counter = 0x00;
	
	while(Counter<4)
	{
		result = SendSMS_WriteCardResult(WriteCard_SUCCESS);
		
		if(result == SendSMS_NeedResent)
		{
			Counter++;
		}
		else
		{
			break;
		}
	}
	
	
	if(result == SendSMS_SUCCESS)
	{
		PowerOffProtection(1);//启动断电保护
	}
	else if(result == SendSMS_NeedResent)
	{
		cephDisplayTextSingle(0x81, UCS2, (u8 *)WhiteCardWriteCardErr);
		return 0;
	}
	else if(result == SendSMS_Fail)
	{
		cephDisplayTextSingle(0x81, UCS2, (u8 *)WhiteCardWriteCardErr);
		return 0;
	}
	else
	{
		return 0;
	}
		
	
	result = WhitrCardWriteCardProcess();
	ClearLociSqn();
	SetWriteCardFlag();
	
	fileAddr = SelectTFFile(DF_STK, EF_WhiteCardConstructor);
	SetTFConst(fileAddr, 0, 0x01, 0x01);//01 写卡结果短信发送成功，重启后发送新号激活短信
	
	PowerOffProtection(2);
	
	cephDisplayTextSingle(0x81, UCS2, (u8 *)WhiteCardWriteSuc);
	return 0;
}

/*--------------------------------------------------------------------------------------------------------------------

  Function name:	WhiteCardWriteCardCheakCommand

  Input parameter:	inputData  存放个性数据数组
  			lgth       有效检查的长度

  Output parameter:	0  执行成功
  			1  存在异常

  Description:		白卡入网短信写卡处理函数

  Author:			wangyanwen
  
  Date:			20180709

  Test:				

  Status:		OK	
 -------------------------------------------------------------------------------------------------------------------*/
u8 WhiteCardWriteCardCheakCommand(u8 * inputData,u8 lgth)
{
	u8 X_A curOffset;
	u8 X_A tmpCmdLgth;
	u8 X_A TempBit,personalByte;

	G_personalDataTagM = 0x00;
	G_personalDataTagO = 0x00;
	
	tmpCmdLgth = lgth;

	curOffset = 0;
	
	while(tmpCmdLgth > 0)
	{
		//ReadLFBuf(dataAddr, recNo, curOffset, &tempBuf[0], 2);
		//ReadLFBuf(dataAddr, recNo, curOffset+2, &tempBuf[2], tempBuf[1]);
		
		ecpMem_Copy(&tempBuf[0], &RecData[curOffset], 2);
		ecpMem_Copy(&tempBuf[2], &RecData[curOffset+2], tempBuf[1]);
		
		curOffset += tempBuf[1] + 2;
		tmpCmdLgth -= tempBuf[1] + 2;
		
		if ( (tempBuf[0]>=0x16) && (tempBuf[0]<=0x1D) )
		{
			;
		}
		else
		{
			if ((tempBuf[0]==0x00) ||(tempBuf[0]==0x80)) //结束
			{
				//?????
				return 0;
			}
			
			if (tempBuf[0]!=0x11)//非法tag
			{
				//?????
				return 1;
			}
		}
		
		if ( (tempBuf[0] == 0x11) && (tempBuf[1] == 0x0A) )
		{
			TempBit = B_ICCID;
			personalByte = G_personalDataTagM;
		}
		else if ( (tempBuf[0] == 0x16) && (tempBuf[1] == 0x09) )
		{
			TempBit = B_IMSI;
			personalByte = G_personalDataTagM;
		}
		else if ( (tempBuf[0] == 0x17) && (tempBuf[1] == 0x08) )
		{
			TempBit = B_SMSP;
			personalByte = G_personalDataTagM;
		}
		else if ( (tempBuf[0] == 0x18) && (tempBuf[1] == 0x08) )
		{
			TempBit = B_PIN1;
			personalByte = G_personalDataTagM;
		}
		else if ( (tempBuf[0] == 0x19) && (tempBuf[1] == 0x08) )
		{
			TempBit = B_PIN2;
			personalByte = G_personalDataTagM;
		}
		else if ( (tempBuf[0] == 0x1A) && (tempBuf[1] == 0x08) )
		{
			TempBit = B_PUK1;
			personalByte = G_personalDataTagM;
		}
		else if ( (tempBuf[0] == 0x1B) && (tempBuf[1] == 0x08) )
		{
			TempBit = B_PUK2;
			personalByte = G_personalDataTagM;
		}
		else if ( (tempBuf[0] == 0x1C) && (tempBuf[1] == 0x10) )
		{
			TempBit = B_K;
			personalByte = G_personalDataTagO;
		}
		else if ( (tempBuf[0] == 0x1D) && (tempBuf[1] == 0x10) ) 
		{
			TempBit = B_OPC;
			personalByte = G_personalDataTagO;
		}
		else
		{
			return 1;
		}
		
		if (personalByte == G_personalDataTagM)
		{
			if ((ValueBit(G_personalDataTagM,TempBit) == FALSE))
			{
				SetBit(G_personalDataTagM,TempBit);
			}
			else
			{
				return 1;//重复Tag
			}
		}
		else
		{
			if ((ValueBit(G_personalDataTagO,TempBit) == FALSE))
			{
				SetBit(G_personalDataTagO,TempBit);
			}
			else
			{
				return 1;//重复Tag
			}
		}
	}
	
	return 0;
}

/*--------------------------------------------------------------------------------------------------------------------

  Function name:	WhitrCardWriteCardProcess

  Input parameter:	NULL

  Output parameter:	0  执行成功
  			1  存在异常

  Description:		白卡入网短信写卡处理函数

  Author:			wangyanwen
  
  Date:			20180709

  Test:				

  Status:		OK	
 -------------------------------------------------------------------------------------------------------------------*/
u8 WhitrCardWriteCardProcess(void)
{
	u8 X_A curOffset;
	u8 X_A tmpCmdLgth;
	memref X_A fileAddr;
	u8 I_A dataLgth;
	
	
	//G_personalDataTagM = 0x00;
	//G_personalDataTagO = 0x00;
	curOffset = 0;
	
	//fileAddr = SelectLFFile(DF_STK, EF_WhiteCardBuf);
	//EF_WhiteCardBuf文件去掉，共用EF_EnvelopeCBMessage文件,by xwh 20210625: EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
	fileAddr = SelectPrivateLFFile(DF_Toolkit, EF_EnvelopeCBMessage);
	ReadLFBuf(fileAddr,1,0,&dataLgth,1);
	ReadLFBuf(fileAddr,1,1,&WhiteCardBuf[0],dataLgth);

	tmpCmdLgth = dataLgth;
	
	while(tmpCmdLgth > 0)
	{
		ecpMem_Copy(&RecData[0], &WhiteCardBuf[curOffset], 2);
		ecpMem_Copy(&RecData[2], &WhiteCardBuf[curOffset+2], RecData[1]);
		
		curOffset += RecData[1] + 2;
		tmpCmdLgth -= RecData[1] + 2;
		
		if ( (RecData[0]>=0x16) && (RecData[0]<=0x1D) )
		{
			;
		}
		else
		{
			if ((RecData[0]==0x00) || (RecData[0]==0x80))//结束
			{
				//?????
				return 0;
			}
			
			if (RecData[0]!=0x11)//非法tag
			{
				//?????
				return 1;
			}
		}
		
		if (RecData[0] == 0x11)
		{
			UpdateICCID();
		}
		else if (RecData[0] == 0x16)
		{
			UpdateIMSI();
		}
		else if (RecData[0] == 0x17)
		{
			UpdateSMSP();
		}
		else if ((RecData[0] >= 0x18) && (RecData[0] <= 0x1B))
		{
			RecData[0] -=0x14;
			
			UpdateKey();
			//UpdateKeyWK();
		}
		/*
		else if (RecData[0] == 0x19)
		{
			UpdateKeyWK();
		}
		else if (RecData[0] == 0x1A)
		{
			UpdateKeyWK();
		}
		else if (RecData[0] == 0x1B)
		{
			UpdateKeyWK();
		}
		*/
		else if (RecData[0] == 0x1C)
		{
			UpdateKI();
		}
		else if (RecData[0] == 0x1D)
		{
			UpdateOPC();
		}
		else
		{
			break;
		}
	}

	return 0;
}


/*--------------------------------------------------------------------------------------------------------------------

  Function name:	SetWriteCardFlag

  Input parameter:	NULL

  Output parameter:	NULL

  Description:		白卡入网写卡成功后，需设置的标记位

  Author:			wangyanwen
  
  Date:			20180709

  Test:				

  Status:		OK	
 -------------------------------------------------------------------------------------------------------------------*/
void SetWriteCardFlag(void)
{
	memref X_A fileAddr;

	fileAddr = SelectTFFile(DF_STK, EF_WhiteCardConfig);
	ReadTFBuf(fileAddr, 2, &tempBuf[0], 1);
	tempBuf[0] = tempBuf[0] | 0x01;
	SetTFConst(fileAddr, 2, tempBuf[0], 0x01);
		
	fileAddr = SelectTFFile(DF_STK, 0x6F01);
	SetTFConst(fileAddr, 0, 0x01, 0x01);//现场写卡，写卡标记  01:写卡成功，不许与再写，其他:可以写卡。 	
}

/*--------------------------------------------------------------------------------------------------------------------

  Function name:	GetWriteCardFlag

  Input parameter:	NULL

  Output parameter:	TRUE：已经写卡
  			FALSE：未写卡

  Description:		获取卡片是否已经写卡的信息

  Author:			wangyanwen
  
  Date:			20180709

  Test:				

  Status:		OK	
 -------------------------------------------------------------------------------------------------------------------*/

u8 GetWriteCardFlag(void)
{
	memref X_A fileAddr;
	u8 X_A tmp[2];
		
	fileAddr = SelectTFFile(DF_STK, EF_WhiteCardConfig);
	ReadTFBuf(fileAddr, 2, &tmp[0], 1);
		
	fileAddr = SelectTFFile(DF_STK, 0x6F01);
	ReadTFBuf(fileAddr, 0, &tmp[1], 1);
	
	if (((tmp[0]&0x01) == 0x01) || (tmp[1] == 0x01))//已经写卡
	{
		return TRUE;
	}
	
	return FALSE;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 Get_ResponseResultNew

  input parameter:   comamndQulify

					 dcs:UCS2/GSMDEFAULT

					 alphaIdentifier(LV,Variable Reference)

					 itemDataObjects(LVLV...,Variable Reference)

					 itemLgth

                     output parameter:  Item Number/ExitStkPerform/BackMove

  description:       Display menu to choose and return the number of item you choose

  code Status:		OK

  test status:      OK

 -------------------------------------------------------------------------------------------------------------------*/
u8 Get_ResponseResultSMS_TR(void)
{
	
	u8 I_A pos;

	pos = FindTag(&RecData[0],TLV_RESULT,P3);

	if ( pos == 0x00)
	{
		return SendSMS_Fail;
	}
	else
	{
		if( (RecData[pos+1] == 0x00) || (RecData[pos+1] == 0x01) || (RecData[pos+1] == 0x02) || (RecData[pos+1] == 0x04) )
		{
			return SendSMS_SUCCESS;
		}
		else if( (RecData[pos+1] == 0x20) || (RecData[pos+1] == 0x21) )
		{
			return SendSMS_NeedResent;
		}
		else
		{
			return SendSMS_Fail;
		}
	}
	
}

u8 WhiteCardcephSendShortMessageSingle(u8 commQualify,u8 CONST * alphaIdentifier, u8 * address, u8 * smsTPDU,u8 type)
{
	u8 I_A result;

	//init the command details and device iden in command of select item
	//InitProactiveCommand(CMD_SENDSHORTMESSAGE,commQualify,DEV_NETWORD);
	InitProactiveCommand(CMD_SENDSHORTMESSAGE,(commQualify&0x7F),DEV_NETWORD);          //扩展commQualify的最高bit，若commQualify最高bit为1，UDHI为1，否则为0

	if ((alphaIdentifier != NULL)&&(alphaIdentifier[0] != 0x00))
	{
		RecData[0] = 0x85;	// alpha identifier

		memcpy_G((u8 X_A *)&RecData[1], &alphaIdentifier[0],alphaIdentifier[0]+1);

		result = AddTLV((u8 X_A *)&ResData[1], (u8 *)&RecData[0]);

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	if (address != NULL)
	{
		RecData[0] = 0x86; 	// Address

		ecpMem_Copy((u8 X_A *)&RecData[1], &address[0], address[0]+1);

		if (RecData[1] != 0x00)	//no Address
		{
			result = AddTLV(&ResData[1], (u8 *)&RecData[0]);

			if (result == FALSE)
			{
				return ExitStkPerform;
			}
		}
	}

	RecData[0] = 0x8B;	// SMS TPDU

	RecData[1] = 0x02;

	if((commQualify&0x80)==0x80)                                   //扩展commQualify的最高bit，若commQualify最高bit为1，UDHI为1，否则为0
	{
		RecData[2] = 0x41;
	}
	else
	{
		RecData[2] = 0x01;
	}

	RecData[3] = 0x00;

	if (smsTPDU != NULL)
	{
		AddValue((u8 *)&RecData[1],&smsTPDU[0]);
	}

	result = AddTLV((u8 X_A *)&ResData[1], (u8  *)&RecData[0]);

	if (result == FALSE)
	{
		return ExitStkPerform;
	}
	
	if (type == 0x01)
	{
		result = SendProactiveCommand();
		
		result = Get_ResponseResultSMS_TR();
	}
	else
	{	
		STKPerform[0] = TRUE;
		STKPerform[1] = G_ChannelNum+1;
	}
	
	return result;
}


/*--------------------------------------------------------------------------------------------------------------------

  Function name:	PowerOffProtection

  Input parameter:	process:断电保护过程的标记

  Output parameter:	NULL

  Description:		断电保护过程标记设置

  Author:			wangyanwen
  
  Date:			20180709

  Test:				

  Status:		OK	
  modify by xuweihua@20210625: 性能优化， EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
 -------------------------------------------------------------------------------------------------------------------*/

void PowerOffProtection(u8 process)
{
	memref X_A fileAddr;
	
	fileAddr = SelectTFFile(DF_STK,EF_WhiteCardConstructor);
	
//	if(fileAddr == FALSE)
//	{
//		return ;
//	}
	
	SetTFConst(fileAddr,1,process,0x01);
}

/*--------------------------------------------------------------------------------------------------------------------

  Function name:	CheckProtection

  Input parameter:	NULL

  Output parameter:	NULL

  Description:		检查断电保护过程的标记，并判断是否需要执行断电保护

  Author:			wangyanwen
  
  Date:			20180709

  Test:				

  Status:		OK	
 -------------------------------------------------------------------------------------------------------------------*/
void CheckProtection(void)
{
	memref I_A fileAddr;

	fileAddr = SelectTFFile(DF_STK,EF_WhiteCardConstructor);//Byte1:缓存文件
	
	if(fileAddr == FALSE )
	{
		return;
	}

	ReadTFBuf(fileAddr, 1, &tempBuf[0], 1);
	
	if (tempBuf[0] == 0x01)
	{
		WhitrCardWriteCardProcess();
		ClearLociSqn();
		SetWriteCardFlag();
		
		fileAddr = SelectTFFile(DF_STK,EF_WhiteCardConstructor);//Byte1:缓存文件
		SetTFConst(fileAddr, 0, 0x01, 0x01);//这里标记为01，表示下次开机需要发送新号激活短信
		SetTFConst(fileAddr,1,0x02,0x01);
	}
}	

/*--------------------------------------------------------------------------------------------------------------------

  Function name:	GetWriteCardSupport

  Input parameter:	NULL

  Output parameter:	NULL

  Description:		8F01的第三个字节，用来控制卡片支持那几种写卡机制，0表示不支持，1表示支持。Bit0=现场写卡、Bit1=两不一快、Bit2=白卡写卡
  
  卡片现场写卡、两不一快、白卡写卡三类功能都支持：
针对“现场写卡、两不一快、白卡写卡三类功能都支持，厂商制卡时，要求：如果预置两不一快，则白卡写卡、现场写卡关闭，如果预置白卡写卡应用并预置临时码号数据，则现场写卡、两不一快关闭；如果预置白卡写卡应用+现场写卡应用，未预置临时。则关闭两不一快。”


  Author:			wangyanwen
  
  Date:			20180709

  Test:				

  Status:		OK	
 -------------------------------------------------------------------------------------------------------------------*/

u8 GetWriteCardSupport(u8 BitIndex)
{
	memref I_A fileAddr;
	u8 I_A  ch;

	fileAddr = SelectTFFile(DF_STK,EF_WhiteCardConstructor);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	ReadTFBuf(fileAddr, 2, &ch, 1);
	
	return ValueBit(ch, BitIndex);
}

#endif
