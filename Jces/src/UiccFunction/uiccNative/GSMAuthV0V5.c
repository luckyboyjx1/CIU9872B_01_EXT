/*
*HISTORY:
		V1.1 修改函数ReadAuthPara，参数输出只采用大端模式进行输出 @20110804 by xuyz
*/

#include "fileSystem.h"
#include "fileaction.h"
#include "interface.h"
#include <string.h>
#include "main.h"		   
#include "error.h"
#include "gsmpublic.h"

#ifdef V0V5
#define		EF_AuthParameter	0x0100
#define		EF_RandomBuffer		0x0200
#define		EF_Auth1			0x0101
#define		EF_Auth2			0x0102
#define		EF_Auth3			0x0103

#define		LIMIT1				0x80000
#define		LIMIT2				2*LIMIT1
#define		LIMIT3				3*LIMIT1

typedef __packed struct AuthPrameter
{
	BYTE	prelock;
	BYTE	lockcount;
	DWORD	counter;
	DWORD	AuthMax;
	DWORD	pnext;
	BYTE	randindex;
	BYTE	rfu;		
}AUTHPAR;

 
/**************************************************************************
*	读取防克隆功能参数：C0 FF 00 01 0A
*  （获取长度为0x0A的参数，依次为PreLock(1BYTE),LockCount(1BYTE),counter(4BYTE),MaxCounter(4BYTE)）
*	
*	安全SIM卡解锁指令： C0 FF 00 02 00
****************************************************************************/
void ReadAuthPara(void)
{
	UWORD idata fileAddr;
	AUTHPAR xdata AuthPar;

	if((P1!=0x00)||((P2!=0x01)&&(P2!=0x02)))
	{
		P1P2Error();
		return;
	}
	if((P2==0x01)&&(P3!=0x0A))
	{
		P3Error();
		return;
	}
	if((P2==0x02)&&(P3!=0x00))
	{
		P3Error();
		return;
	}
	
	if(ValueBit(G_AdmStatus,1) == FALSE)
	{
		SecurityError();
		return;	
	}

	fileAddr =  SelectTFFile(DF_PHASE,EF_AuthParameter);

	if( fileAddr == FALSE )
    {
		TechnicalProblem();
		return;
	}
	
	ReadTFBuf(fileAddr,0x00,(BYTE *)&AuthPar,sizeof(AUTHPAR));

	if((AuthPar.rfu==0x01)||(AuthPar.rfu==0x02)||(AuthPar.rfu==0x03))
	{
		fileAddr =  SelectTFFile(DF_PHASE,(EF_AuthParameter+AuthPar.rfu));

		if( fileAddr == FALSE )
    	{
			TechnicalProblem();
			return;
		}

		ReadTFBuf(fileAddr,0x00,(BYTE *)&AuthPar,sizeof(AUTHPAR));
	}

	if(P2==0x01)
	{
		RecData[0] =  AuthPar.prelock;
		RecData[1] =  AuthPar.lockcount;
		
		RecData[2] =  (AuthPar.counter>>24)&0xFF;
		RecData[3] =  (AuthPar.counter>>16)&0xFF;
		RecData[4] =  (AuthPar.counter>>8)&0xFF;
		RecData[5] =  AuthPar.counter&0xFF;
		
		RecData[6] =  (AuthPar.AuthMax>>24)&0xFF;
		RecData[7] =  (AuthPar.AuthMax>>16)&0xFF;
		RecData[8] =  (AuthPar.AuthMax>>8)&0xFF;
		RecData[9] =  AuthPar.AuthMax&0xFF;

		
		SendData();
	}
	else
	{
		if(AuthPar.lockcount < 0xFF)
		{
			AuthPar.prelock = 0xFF;

			WriteTFBuf(fileAddr,0x00,(BYTE *)&AuthPar,sizeof(AuthPar));		
		}
		else
		{
			SecurityError();
			return;
		}
	}

	CommandFinish(0x00);
}


void ReadKiFromFileV0V5(BYTE kiValue[16])
{
	
	AUTHPAR xdata AuthPar;
	UWORD idata fileAddrCom,fileAddrRan,filrAddrTemp;
	BYTE  idata i,j,num,foundTag,rngbuf;
	
	Random_Number_Generator_Get(&kiValue[0], 0x10);

	fileAddrCom = SelectTFFile(DF_PHASE,EF_AuthParameter);
	
	if(fileAddrCom == FALSE)
	{
		return;
	}	

	ReadTFBuf(fileAddrCom,0x00,(BYTE *)&AuthPar,sizeof(AUTHPAR));

	if((AuthPar.rfu==0x01)||(AuthPar.rfu==0x02)||(AuthPar.rfu==0x03))
	{
		fileAddrCom =  SelectTFFile(DF_PHASE,(EF_AuthParameter+AuthPar.rfu));

		if(fileAddrCom == FALSE)
		{
			return;
		}

		ReadTFBuf(fileAddrCom,0x00,(BYTE *)&AuthPar,sizeof(AUTHPAR));
	}
	
	if((AuthPar.AuthMax==LIMIT1)||(AuthPar.AuthMax==LIMIT2)||(AuthPar.AuthMax==LIMIT3))
	{
		fileAddrCom =  SelectTFFile(DF_PHASE,(EF_AuthParameter+(AuthPar.AuthMax/LIMIT1)));

		if(fileAddrCom == FALSE)
		{
			return;
		}

		WriteTFBuf(fileAddrCom,0x00,(BYTE *)&AuthPar,sizeof(AuthPar));

		filrAddrTemp =  SelectTFFile(DF_PHASE,EF_AuthParameter);
		
		if(filrAddrTemp == FALSE)
		{
			return;
		}

		SetTFConst(filrAddrTemp, 0x0F, (BYTE)(AuthPar.AuthMax/LIMIT1),sizeof(BYTE));
	}
	
	if(AuthPar.AuthMax == 0x00)
	{
		return;		
	}

	AuthPar.AuthMax = AuthPar.AuthMax-0x01;

	if(AuthPar.prelock==0x00)											//判断预锁定计数器是否记到最大
	{	
		WriteTFBuf(fileAddrCom,0x00,(BYTE *)&AuthPar,sizeof(AuthPar));
		
		return;
	}

	fileAddrRan =  SelectLFFile(DF_PHASE,EF_RandomBuffer);

	if(fileAddrRan == FALSE)
	{
		return;
	}	

	num = GetLFRecNumLgth(fileAddrRan,FileNum);
	
	for(i=1;i<=num;i++)													//比较当前随机数是否为攻击
	{
		foundTag = 0x00;

		ReadLFBuf(fileAddrRan,i,0x00,&RecData[0x20],0x10);
		
		for(j=0;j<0x10;j++)
		{
			if(RecData[0x20+j] == RecData[j])
			{
				foundTag = foundTag+1;
			}
		}
		if((foundTag>=0x05)&&(foundTag<=0x0F))
		{
			AuthPar.prelock = AuthPar.prelock-0x01;										//锁定攻击prelock减1
		
			if(AuthPar.prelock==0x00)
			{
				AuthPar.lockcount = AuthPar.lockcount+0x01;
			}

			WriteTFBuf(fileAddrCom,0x00,(BYTE *)&AuthPar,sizeof(AuthPar));
			
			return;	
		}
		if(foundTag==0x10)
		{
			break;
		}	
	}

	if(foundTag!=0x10)
	{
		AuthPar.counter = AuthPar.counter+1;												//攻击检测通过，计数器加1
	
		if((AuthPar.counter==AuthPar.pnext)||(AuthPar.counter<num))
		{	
			WriteLFBuf(fileAddrRan,AuthPar.randindex,0x00,&RecData[0],0x10);			//存储随机数
		
			AuthPar.randindex = (++AuthPar.randindex)%num;
		
			if(AuthPar.randindex==0x00)
			{
				AuthPar.randindex = num;
			}	
		}

		if(AuthPar.counter==AuthPar.pnext)
		{
			Random_Number_Generator_Get(&rngbuf, 0x01);
		
			AuthPar.pnext = AuthPar.pnext+(rngbuf/0x10)+1;									//计算新pnext			
		}
	}
	WriteTFBuf(fileAddrCom,0x00,(BYTE *)&AuthPar,sizeof(AuthPar));
	
	ReadEeprom(&kiValue[0],E_SECURE_DATA+KI_OFF,0x10);	//File Type	

	//以下为原来V0防克隆算法代码，现整合到一起
	fileAddrRan =  SelectLFFile(DF_PHASE,EF_Random);

   	if(fileAddrRan != FALSE)
	{
		foundTag = 0x00 ;

		for(i=0;i<5;i++)
		{
			ReadLFBuf(fileAddrRan,i+1,0x00,&RecData[0x30+i*0x18],0x18);

			for(j=0;j<8;j++)
			{
				if((RecData[j] == RecData[0x30+j*3+1+i*24])&&( RecData[j+0x08] == RecData[0x30+j*3+2+i*24])&&(RecData[0x30+j*3+i*24]!=0xFF) )
				{
					foundTag = foundTag + 1 ;
				}
			}
		}
		
		if(foundTag != 0x00)
		{
			fileAddrRan = SelectTFFile(DF_PHASE,EF_WrongKi);

			ReadLFBuf(fileAddrRan,1,0x00,&kiValue[0],0x10);						

			//ReadEeprom(&kiValue[0],fileAddrRan+sizeof(RecordFileHead),0x10);
		}


		for(foundTag = 0;foundTag<RecData[0x01];foundTag++)
		{
			;
		}
		
	}
	
}

 
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 RunGsmAlgorithm
  
  Command format:    A0    88    00   00   10
  
  Data    content:   random                                                       
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       This function add the value given by the ME to the value of the last increased/updated record of the current cyclic EF

  status:			 Code OK!     Test OK!
  					 
					 
 -------------------------------------------------------------------------------------------------------------------*/
void RunGsmAlgorithmV0V5(void)
{
	UWORD idata keyAddr;
	
	if((P1 != 0x00)||(P2 != 0x00))
	{
		P1P2Error();
	}

	if( P3 != 0x10 )
	{
		P3Error();
		return;
	}	
	                                                   //send 67 00
	if(( G_CurrentDfName[G_ChannelNum] != 0x7F20 )&&( G_CurrentDfName[G_ChannelNum] != 0x7F21 ))
    {
		FileStructureError();
		return;
	}	                                           		//send 94 08

	//if(ValueBit(G_AdmStatus,1) == FALSE)
	if(ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE)
    {
		SecurityError();								//send 98 04
		return;
	}  
     
	/*                                  
	keyAddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],EF_KI);
  
	if( keyAddr == FALSE )
    {
		TechnicalProblem();
		return;
	} 
	*/                                          //send 6f 00(no find ki)
  
	ReceiveData();

	ReadKiFromFileV0V5(&RecData[0x10]);
		
	RunA3A8_V1(&RecData[0x10],&RecData[0x00]);

	CommandFinish(0x0C);                                          			  //send 9f 0c	                                            		
}


#endif
