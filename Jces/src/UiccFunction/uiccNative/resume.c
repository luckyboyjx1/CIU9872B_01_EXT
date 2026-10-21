/**
 * @file 		resume.c
 *
 * @brief 		TMC 260断电恢复API
 *
 * @author 		YMF CaiBo WG
 *
 * @history 	2008-03-06: fix bug in CopyEepromEX() by WG
 */
#include  "uicc_main.h"
#include "uicc_error.h"
#include  "resume.h"
#include  "gsmpublic.h"
#include  "Interface_LTEFromBoot.h"
#include  "arithmatic.h"
#include "JcvmDataConfig.h"



BYTE xdata G_BufferTag ;
BYTE xdata G_PageTag ;
BYTE xdata G_Bank;

/*
length: 必须保证length的长度为4的倍数
eepAddr: 保证传入的地址正确

*/
/*UWORD WriteFourBytes(UWORD eepAddr,BYTE * srcBuffer,BYTE length)
{
	BYTE idata wrTimes,i;
	UWORD idata result;
	UWORD idata wordValue;
	
	if((length%4)!=0x00)
	{
		return 0xFFFFFFFF;
	}
	
	wrTimes = length/4;
	
  for(i=0;i<wrTimes;i++) 
	{										
		wordValue = srcBuffer[i*4];
		wordValue = (wordValue<<8) + srcBuffer[1+i*4];
		wordValue = (wordValue<<8) + srcBuffer[2+i*4];
		wordValue = (wordValue<<8) + srcBuffer[3+i*4];
		
		result = HED_FlashWriteWord_St(eepAddr+i*4,wordValue);
		
		if(result!=0x00)
		{
			return 0xFFFFFFFF;
		}
	}
	
	return 0x00;
	
}*/


/*
BYTE WriteWithoutErease(UWORD dstAddr,BYTE * srcBuffer,BYTE length)
{
	
	BYTE  idata i,CheckByte; 
	DWORD  idata result;
	UWORD  xdata flashAddr;
		
	flashAddr = FlashBaseAddr+dstAddr;
	
	closeWrSwitch234();
	
	openWrSwitch2();
	
	G_UPDATEFLAG=FALSE_FLAG;
	
	CheckByte=0x00;	

	if(G_FLASH_ADDR!=dstAddr) //检查参数的正确性
	{
		return FALSE;
	}
	
	SetBit(CheckByte,CHECK_PARAMETER); //做了参数检查
	
	if((G_COMMANDPATH&0x07)!=0x07) //检查指令执行序列,无论是从断电函数进入,并执行了CopySector
	{
		return FALSE;
	}
	
	SetBit(CheckByte,CHECK_COMMANDPATH); //做了序列检查	

	openWrSwitch3();
	 
	if(flashAddr<G_CODE_SIZE) //如果更新的地址在CODE区，则返回
	{
		return FALSE ;
	}
		
	SetBit(CheckByte,CHECK_FLASHADDR); //做了地址检查
		
	G_UPDATEFLAG=RIGHT_FLAG;
		
	if((length%4)!=0x00)												//长度必须为4字节倍数
	{
		return FALSE;
	}
	
	for(i = 0x00;i<(length/4);i++)
	{
		result = HED_FlashWriteWord_St(flashAddr+i*4,*((unsigned int*)srcBuffer+i));
			
		if(result != 0x00)
		{
			return FALSE;
		}
	}
	
	G_UPDATEFLAG=FALSE_FLAG;

	return TRUE;

}*/


#if NATIVE_CARD_UICC		
/*
 *@description:判断要写的eeprom数据是否在安全数据区内
 *
 *@param:      UWORD eepAddr:要写的数据地址

 *
 *@Returns     TRUE/FALSE;
 *
 *@history     create by zhxx at 20200225
*/
BYTE InSecurityBlock(UWORD eepAddr)
{
  UWORD xdata mfEndAddr;
	BYTE idata lifeCycle;

	mfEndAddr = GetFileHeaderEndAddr();

  ReadEeprom(&lifeCycle,E_CARD_LIFE_CYCLE,1);

	if((lifeCycle == D_CardSecured) && (eepAddr <= mfEndAddr))
	{
	   return TRUE;
	}
	else
	{
	   return FALSE;
	}
}
#endif


/**
 * @brief 		带断电的拷贝函数,注意：源地址一定要大于目的地址
 *
 * @param 		dstAddr
 * @param 		srcAddr
 * @param 		length: ??
 *
 * @Returns		TRUE/FALSE
 *
 * @author 		YMF
 *
 * @history 	fix bug: (i==0)-->(i=0) by WG, at 080306 10:40
 * @history 	fix bug: (srcAddr, dstAddr, length)-->(dstAddr, srcAddr, length) by WG, at 080306 9:30
 * @history 	created by YMF, at 080217
 */
/*
BYTE CopyEepromEX(UWORD desAddr,UWORD sourseAddr,WORD  length)   
{
	
	BODYLLV xdata  llvStruct;
	WORD  xdata copyLgth;
	WORD  xdata currentLgth;
	WORD  xdata moveSize;

	if(desAddr >= sourseAddr)
	{
		return FALSE;
	}

	if(sourseAddr > (desAddr+PFPageSize))
	{
		moveSize = PFPageSize;
	}
	else
	{
		moveSize = sourseAddr - desAddr; 
	}

	llvStruct.destntnAddr = desAddr  ;

	llvStruct.sourceAddr = sourseAddr;

	llvStruct.lgth        = length   ;

	WriteEepromEX(E_APP_DUANDIAN_START,(BYTE *)&llvStruct,sizeof(llvStruct));

	if(((desAddr%PFPageSize)!= 0x00))						
	{
		copyLgth = PFPageSize - (desAddr%PFPageSize);

		copyLgth = (copyLgth > moveSize) ?  moveSize : copyLgth;

		CopyEeprom(desAddr,sourseAddr,copyLgth);

		llvStruct.destntnAddr = desAddr + copyLgth;

		llvStruct.sourceAddr  = sourseAddr + copyLgth;

		llvStruct.lgth        = llvStruct.lgth - copyLgth;
	}
	else
	{
		copyLgth = 0x00;
	}

	for(;copyLgth<length;copyLgth = copyLgth + moveSize)
	{
		WriteEepromEX(E_APP_DUANDIAN_START+sizeof(llvStruct),(BYTE *)&llvStruct,sizeof(llvStruct));

		if((copyLgth+moveSize)>length)
		{
			currentLgth =length - copyLgth;
		}
		else
		{
			currentLgth = moveSize ;
		}

		CopyEeprom(llvStruct.destntnAddr,llvStruct.sourceAddr,currentLgth);

		llvStruct.sourceAddr = llvStruct.sourceAddr + currentLgth;

		llvStruct.destntnAddr = llvStruct.destntnAddr + currentLgth;

		llvStruct.lgth        = llvStruct.lgth - currentLgth;
	}

	SetEepromEX(E_APP_DUANDIAN_START,0xFF,2*sizeof(llvStruct));

	return TRUE ;

}*/



/**
 * @brief 		automatic to copy eeprom data to another address
 *
 * @author 		YMF
 *
 * @history 	created by YMF, at 080307 10:40
 */
/*
void CopyEepromResume(void)
{
	BODYLLV xdata  llvStruct;

	ReadEeprom((BYTE *)&llvStruct, E_APP_DUANDIAN_START, sizeof(llvStruct));

	if ((llvStruct.sourceAddr == 0xFFFFFFFF) && (llvStruct.destntnAddr == 0xFFFFFFFF))
	{
		return;
	}

	ReadEeprom((BYTE *)&llvStruct, E_APP_DUANDIAN_START +sizeof(llvStruct), sizeof(llvStruct));

	if ((llvStruct.sourceAddr == 0xFFFFFFFF) && (llvStruct.destntnAddr == 0xFFFFFFFF))
	{
		ReadEeprom((BYTE *)&llvStruct, E_APP_DUANDIAN_START, sizeof(llvStruct));
	}

	CopyEepromEX(llvStruct.destntnAddr, llvStruct.sourceAddr, llvStruct.lgth);
}*/

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 个人化的过程中读EEPROM
  
  Command format:    C0    D8    xx   xx   P3  
  
  Data    content:   void    
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       读EEPROM,使用逻辑地址(P1,P2)
  
  status:			  

 -------------------------------------------------------------------------------------------------------------------*/    
void PersoReadEeprom(void)
{
    UWORD xdata eepAddr;
	
	BYTE idata tempResult,decryDataFlag;	
	
	
	memset(tempBuf,0x00,0x20);
    
    memset(&tempBuf[0x20],0xFF,0x10);
    
    decryDataFlag = FALSE;	

    if(P3 != 0x80)
    {
        P3Error();

        return;
    }    

	eepAddr=(((UWORD)P1<<8)&0xFF00)+P2;
		
	eepAddr = eepAddr + G_Bank*0x10000;
						
	if(G_CardStatus != D_CardEmpty)  //创建文件系统 
	{
			tempResult = ReadKey(SuperKey_Type,&tempBuf[0]); //tempBuf[4]开始的16字节是Super Key密钥
	    
	    if(tempResult == FALSE)
	    {
	    	ReferencedDataInvalid();
	    	return;
	    }
	    	   
			if(memcmp(&tempBuf[4],&tempBuf[0x20],0x10) == EQUAL)                //the key not find 
	    {
				decryDataFlag = TRUE;											//6A 88		
			}
			else
			{
				if(G_SuperKeyAccess  != SuperKeyAccess_PassVerify)  //读E2的权限修改成SuperKey验证成功
		    {
					SecurityError();
					return;
		    }
			}
		
	}
	else
	{
		decryDataFlag = TRUE;	   	
	}
	
	//if((eepAddr+0x80)>(E_FileBodyStart-FlashBaseAddr))
	if((eepAddr+0x80)>(EEPROM_LIMIT-FlashBaseAddr))						//读写范围不限于文件系统,为除COS外的整个flash
	{
	 	EepromError();

		return;
	}
	
	eepAddr = eepAddr+EEPROM_BASE;										//加上COS基地址后再读才正确

	ReadEeprom(&RecData[0],eepAddr,P3);

   if(decryDataFlag == FALSE)
   {
    	tempResult = ReadKey(EncryKey_Type,&tempBuf[0]); //tempBuf[4]开始的16字节是Encry Key密钥
    
	    if(tempResult == FALSE)
	    {
	    	ReferencedDataInvalid();

	    	return;
	    }
       	       
    	TripleDES(&RecData[0x00],&tempBuf[0x04],&RecData[0x00],CBC_Encrypt,0x80,16);
 	}

	SendData();

    CommandFinish(0x00);
}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 个人化的过程中写EEPROM
  
  Command format:    C0    DA    xx   xx   P3  
  
  Data    content:   void    
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       读EEPROM,使用逻辑地址(P1,P2)
  
  status:			  

 -------------------------------------------------------------------------------------------------------------------*/    
void PersoWriteEeprom(void)
{
  UWORD xdata eepAddr,flashAddr;	
	BYTE  idata result,CheckByte,bPage,i;			
	
	CheckByte=0x00;
	
	if(P3 != 0x80)
  {
      P3Error();
      return;
  }
		
		
	eepAddr=(((UWORD)P1<<8)&0xFF00)+P2;							

	eepAddr = eepAddr + G_Bank*0x10000;						//普通写镜像模式
	
 
	if(G_CardStatus != D_CardEmpty)  //创建文件系统 
	{
		if(G_Bank == 0xFE)
		{
				if (ValueBit(G_SystemStatus,B_cmk) == FALSE)
				{
					SecurityError();
					return;
				}
		}
		else
		{			
			if(G_SuperKeyAccess  != SuperKeyAccess_PassVerify)  //写E2的权限修改成SuperKey验证成功	
			{
				SecurityError();
				return;
			}
		}
	}
	
	
	//if((eepAddr+0x80)>(E_FileBodyStart-FlashBaseAddr))
	if((eepAddr+0x80)>(EEPROM_LIMIT-FlashBaseAddr))
	{
	 	EepromError();
		return;
	}
					
	ReceiveData();
	
	eepAddr = eepAddr+EEPROM_BASE;
		
	ecpNvm_WriteErase((PN8)eepAddr, PX8_A_CONST(APDUBuffer, 5), APDUBuffer[4]);
	
	//MemNvmOperation_WriteErase(eepAddr, MEMtPN8(&RecData[0]) ,0x80);
		
}



void EraseFileSystem(void)
{
#if NATIVE_CARD_UICC
	 
	UWORD xdata i;
	 	
																			
	for (i=0;i<((E_FileBodyStart-E_COS_ConfigData)/PFPageSize);i++)
  {		
			SetEeprom(E_COS_ConfigData+i*PFPageSize,0xFF,PFPageSize);
  }  
#else	
	//调用platformfillEeprom删除eeprom时会坏卡，还未确定原因,by xwh 20220317
		platformfillEeprom(MEMtPN8(E2PBase),0xFF,FlashLimit-FlashBaseAddr);
#endif	
}


/* 
void EraseFileAndCardInfo(void)
{
#if NATIVE_CARD_UICC	
		
	UWORD xdata i;
	 	
	for (i=0;i<((E_FileBodyStart-EEPROM_BASE)/PFPageSize);i++)
  {		
			SetEeprom(EEPROM_BASE+i*PFPageSize,0xFF,PFPageSize);
  }  
#else	
		platformfillEeprom(MEMtPN8(E2PBase),0xFF,FlashLimit-FlashBaseAddr);
#endif	
}
*/


void SetBank(void)
{
	if(P3!=0x00)
  {
    P3Error();			//Send   67 00
		return ;
  }
	
	if((P2>0x20)&&(P2!=0xFE))										//最大支持0x20个bank的切换
	{
    P1P2Error();			//Send   6B 00
		return ;		
	}

	G_Bank=P2;

	CommandFinish(0x00);	
}



//CalCRC指令的子函数 P1=0：计算EEPROM的CRC  P1=1：计算COS的CRC
/*
  使用此函数时,注意保证输入参数，BaseAddr：为页起始地址  
                                 Length：长度为页大小的整数倍
*/
//CalCRC指令的子函数 P1=0：计算EEPROM的CRC  P1=1：计算COS的CRC
/*
  使用此函数时,注意保证输入参数，BaseAddr：为页起始地址  
                                 Length：长度为页大小的整数倍
*/
//WORD CRC_calculate16(UWORD BaseAddr,DWORD Length,BYTE mode)
//{
//	WORD idata crcResult,result;
//	WORD idata cTimes,i;
//	UWORD idata flashAddr;
//	BYTE *	address;
//	
//	if(mode == 0x01)
//	{
//		flashAddr = BaseAddr;
//	}
//	else
//	{
//		flashAddr = BaseAddr+FlashBaseAddr;
//	}
//	
//	cTimes = Length/0x80;							
//	
//	crcResult = 0x00;
//		
//	for(i=0x00;i<cTimes;i++)
//	{	
//		address = (BYTE *)(flashAddr+i*0x80);			//已经加上了COS的偏移地址

//	  memcpy(&tempBuf[0], (BYTE *)address, 0x80);	
//	
//		result = HED_CRC16(CRC16_TypeA, 0x80,&tempBuf[0] ,2);          //每次计数0x80个字节的值
//		
//		crcResult = result+crcResult;
//	}
//	
//	
//	return crcResult;
//}



#if NATIVE_CARD_UICC	
void CalCRC(void)
{
	WORD  idata result,tmp;
  															
	if(P3!=0x02)	
	{
   	P3Error();
  	return ;
	}
		
	if((P1>0x02) || (P2>0x01))
	{
   	P1P2Error();
   	return;   
	}
	
	if(P1 == 0x00)
 	{
  		result=Get_crc16((PM8)EEPROM_BASE, (UWORD)E_FileBodyStart-EEPROM_BASE);	//数据区CRC计算除断电保护区不计算外，其他都计算。
 	}	
 	else if(P1 == 0x01)
  {
    	result=Get_crc16((PM8)COS_BASE,(UWORD)EEPROM_BASE-COS_BASE);			//#define COS_BASE                      
  }
	else													  				//P1==0x02,计算敏感数据区CRC,区域大小在此芯片中为1页
	{
		result=Get_crc16((PM8)E_SECURE_DATA,0x200);					  
	}
	
  if(P2==0x01)
	{
 		ReceiveData(); 
	   
 		tmp=(((WORD)RecData[0]<<8)&0xFF00)|RecData[1];

 		if(result!=tmp)
 		{
  			EepromError();
   			return;
 		}  
	}
	else
	{
 		RecData[0]=(result&0xFF00)>>8;
 
 		RecData[1]=(result&0x00FF);

 		SendData();
	}
	
  CommandFinish(0x00);
 
 	return;
}
#endif



