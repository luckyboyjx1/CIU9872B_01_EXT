/*

 * Copyright (c) 2005,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V6.01

 * PHASE 2  RELEASE:2005-12-25

  * HISTORY: V1.0 创建源文件
  			 V1.1 修改ReloadKey(void)：PIN、PUK的最大错误尝试次数为2E10中对应记录的第3个字节的高4bits
			 V1.2 修改函数EepormErase，修改清除文件系统时的操作，modify by xyz@100112
			 v1.3 修改函数ReloadKey，支持P2 为01 02 03 04 05时的重载@20110715 16:01
			 V1.4 修改函数Initializtion  EepormErase ，删除E_SYSTEM_STATUS  E_APP_TYPE的使用@20141009 by xuyz
			 V1.5 修改函数ReloadVerifyManagenerKey，修改该函数功能为仅仅重载、验证MF下的密钥@20141110 by xuyz
			 	  增加函数ReloadVerifySuperKey，重载，验证SuperKey的权限
			 	  修改函数EepormErase，擦除MF 文件头时，同时如果6F1B存在，则将6F1B 的内容也擦掉
			 V1.6 修改函数ReloadVerifyManagenerKey，在校验MFKEY成功之后恢复校验次数。@20141126	by wgl
			 V1.7 修改函数EepormErase,若存在3F00/2E1B(电信单4G的KI文件),清为全0xFF
			 V1.8 修改UpdateFile函数,新文件系统结构取文件体地址方式改变  	  @20200303	by yangjian

 */ 		 
#include "uicc_main.h"
#include "uicc_error.h"
#include "fileSystem.h"
#include "gsmpublic.h"
//#include "resume.h"
#include "fileaction.h"
//#include "hard_api.h"
#include "interface.h"
#include <string.h>
#include "arithmatic.h"
#include "eepromdefine.h"
//#include "Transaction.h"
//#include "stkhead.h"
#include "Interface_LTEFromBoot.h"
#include "BootLifeCycle.h"
//#include "EUICCMain.h"
#include "SysConfig.h"
#include "JcvmConfig.h"
#if GP__AMENDMENT__B__FOR_MASK	
#include "GPAnnexbAdminSessionPara.h"
#endif



 /*
*@function   : GetIfMfCreated
*
*@description:返回私有文件区或者default文件区MF地址， 若没有创建，返回INVALID_ADDR
*
*@param      :BYTE mode --- MODE_PRIVATE表示获取私有文件系统起始地址 / MODE_DEFAULT表示获取默认文件系统起始地址
*
*@return     :UWORD mfOffset --- 对应的文件系统存在的话就返回该文件系统的起始地址；否则返回INVALID_ADDR
*
*@history    :V1.0:create by xwh@20190306
*            :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
#if !NATIVE_CARD_UICC
//UWORD GetIfMfCreated(BYTE mode)
//{
//	FileIdType xdata fileHead;

//	if((mode != MODE_PRIVATE)&&(mode != MODE_DEFAULT))
//	{
//		return INVALID_ADDR;
//	}
//	
//	if(BootLifeCycle_GetLifeCycle() < LIFE_CYCLE_CREATE)
//	{
//		return INVALID_ADDR;
//	}

//	ReadEEPromCache((BYTE *)&fileHead,ADDR_PRIVATE_FILE_START,sizeof(fileHead));

//	if((fileHead.FileId == DF_MF)&&((fileHead.FileType&MFTYPE)==MFTYPE))                                            //ò??-′′?¨á? Private File System
//	{
//		if(mode == MODE_PRIVATE)
//		{
//			return ADDR_PRIVATE_FILE_START;
//		}
//		else
//		{
//			return VMReadUword(RSPDefaultFileSystem_Addr);
//		}
//	}

//	return INVALID_ADDR;
//}
#endif




#if NATIVE_CARD_UICC
void SetNativeUICCCardState(void)
{
	FileIdType xdata fileHead;
	
	//NATIVE UICC下MF起始地址是固定地址,by xwh 20220325
	ReadEEPromCache((BYTE *)&fileHead,E_FILE_START,sizeof(fileHead));

	if((fileHead.FileId == DF_MF)&&((fileHead.FileType&MFTYPE)==MFTYPE))     
	{
		G_CardStatus=D_CardInit;
	}
}
#else
UWORD GetMFFileStart(void)
{
	UWORD idata mfAddr;
	
	mfAddr = INVALID_ADDR;
		
	mfAddr = UPCOSConfig_GetUPCOSFileSystemAddr();
	
	if(mfAddr != INVALID_ADDR)
	{
		G_CardStatus=D_CardInit;
	}

	return mfAddr;
}
#endif

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 Initializtion
  
  Command Format:    00    FE    00   00   09  
  
  Data  Content:                                                                                                          
                                                                                                                                                
  Description:

  Coder:             CAIBO        Tester:
  
  Status:			待引入参数P3赋值

 -------------------------------------------------------------------------------------------------------------------*/    
 #if NATIVE_CARD_UICC		
void Initializtion(void) 
{
	FileSystemHead xdata  mfFileHead ;
	BYTE xdata mfKey[9];
	BYTE idata  result;
	UWORD idata mfAddr;

	if(P1P2P3Check(0x09,FALSE) ==FALSE)
	{
		return ;
	}
	
	if(G_CardStatus != D_CardEmpty)
	{
	   ConditionSatisfied();                                                        //send 69 85
	   return;
	}   

//	mfAddr = GetIfMfCreated(MODE_DEFAULT);
//	
//	if(mfAddr != INVALID_ADDR)                                  //Default文件区已创建，不允许重复创建
//	{
//		ConditionSatisfied();
//		return;  
//	}	
	
	ReceiveData(); 
	
	mfAddr = E_FILE_START;
			
	SetEeprom(E_ADDR_INDEX_TABLE,0xFF,MAX_INDEX_NUM*sizeof(UWORD));  //将地址索引表清为全F																	//receive data to recDataBuffer
	
	mfFileHead.FileId = DF_MF;

 	mfFileHead.FileType = 0x78 ;							//MF as sharable

	mfFileHead.ArrNum = 0x07;								//兼容产业部现有产品做法,此处固定默认为0x07
	//mfFileHead.ArrNum = RecData[8];								//MF Arr Num
	
	//mfFileHead.ArrName = 0x2F;
	mfFileHead.ArrName = F_ARRNAME_2F;             //bit7为0表示当前的arrName为2F，bit0-bit6
	
	mfFileHead.DIRNum = 0x00;
	
	/*if(G_FILE_SIZE <= sizeof(FileSystemHead))
	{
		AddressInvilid();
		return;
	}*/
#if !HEAD_BODY_TOGETHER
	mfFileHead.UsedContentSize = 0;

 	mfFileHead.UsedHeadSize = sizeof(FileSystemHead);

	mfFileHead.FileContentRef = FindFreeIndexFromAddrIndexTable();

	VMWriteUword((E_ADDR_INDEX_TABLE + (mfFileHead.FileContentRef-1)*sizeof(UWORD)),E_FileBodyStart);
#else 
	//if support HEAD BODY TOGETHER,add code hear	
#endif
 	mfFileHead.ChildDfNum = 0x01;

 	mfFileHead.ChildEfNum = 0x00;

 	mfFileHead.ParentName[0] = DF_MF;

	mfFileHead.ParentName[1] = DF_MF;

	mfFileHead.ChvStatus[0] = 0x01;
		
	mfFileHead.ChvStatus[1] = 0x81;

	mfFileHead.ChvStatus[2] = 0x0A;
	
	mfFileHead.ChvStatus[3] = 0x0B;

	memcpy((BYTE *)&mfKey[0], &RecData[0], 0x08);
	
	mfKey[8] = 0x0A;													//keyLimit	
	
//	SetBit(G_COMMANDPATH,B_ACCESSEX); 						

	result = WriteEeprom(E_MK_START,(BYTE *)&mfKey[0],9);     //设置MFKEY及KeyLimit
	
	if(result == FALSE)
	{
	   	MemoryError();

		return;
	}		

	
	result = WriteEeprom(mfAddr,(BYTE *)&mfFileHead.FileId,sizeof(FileSystemHead));          //write mf head to the file system start of eeprom 

	if(result == FALSE)
	{
	   	MemoryError();
		return;
	} 
		
	G_CardStatus = D_CardInit;

	SetEeprom(E_CARD_LIFE_CYCLE,D_CardInit,1);
	
	G_CurMFFileStart = mfAddr;

	SetChannel(mfAddr,DF_MF,FALSE);			//set the mf the default df

	CommandFinish(0x00);															 //send 90 00
}
#endif
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	EepormErase
  
  Command format:	00    2A    00   00-02   08
  
  Data    content:	MF KEY(8)                                                        
                                                                                                                                                
  input parameter:	void                                                                
  					                                                                                                         
  output parameter:	void                                                                                                           
                                                                                                                                                
  description:		P1 :0x00 to erase File header
						0x01 to erase All eeprom
						0x02 to erase Flash

  status:    

 -------------------------------------------------------------------------------------------------------------------*/
 #if NATIVE_CARD_UICC	
void EepormErase(void)
{
  	BYTE idata tag,mfKeyNum;
	
//	SetBit(G_COMMANDPATH,B_ACCESSEX); //从ACCESSEX中进入擦写函数，为了保证擦写函数执行的指令路径
	
//	SetBit(G_COMMANDPATH,B_ACCESS); //从ACCESS中进入擦写函数，为了保证擦写函数执行的指令路径
		
	if((P2 != 0x00)||(P1 > 0x02))
  	{
  		P1P2Error();															//send  6B 00
		return;
	}

  	if(P3 != 0x08 )
	{
		P3Error();																//send 6700
		return;
	}
	
	if(G_CardStatus == D_CardEmpty)	   	//没有创建文件系统
	{	
		if (P1 == 0x02)					//在生产过程中,如果只下载COS而没初试化,就清空COS,不用校验MFKEY
		{
			ReturnBootloader();
		}
		else if(P1 == 0x01)
		{
			EraseFileSystem();
		}
		/*
		else if(P1 == 0x03)						
		{
			EraseFileAndCardInfo();
		}
		*/

		CommandFinish(0x00);

        return;
	}
	
	ReadEeprom(&mfKeyNum, E_MK_START+F_MFKEYLimit_Offset,0x01);

  	if((mfKeyNum&0x0F) == 0x00)
  	{
  		PinBlocked();	
        														//send  98 40 
		return;
	}

  	ReceiveData();

	ReadEeprom((BYTE *)&RecData[0x20],E_MK_START,0x08);
  
    tag = memcmp((BYTE *)&RecData[0],(BYTE *)&RecData[0x20],0x08);
	
  	if(tag != EQUAL)
   	{
		mfKeyNum = mfKeyNum -1 ;

        WriteEepromEX(E_MK_START+F_MFKEYLimit_Offset,&mfKeyNum,0x01);

		SecuritySatisfied();
	}
	else
	{
		switch (P1)
		{
			case 0x00:
            {				    	             				
				SetEepromEX(E_SECURE_DATA,0xFF,SECURE_DATA_LEN);    //清空敏感数据区     	
 
				SetEepromEX(E_FILE_START,0xFF,sizeof(FileSystemHead));    //清空MF的文件头内容               
				break;
		    }
			case 0x01:
            {
		    	EraseFileSystem();       //清空EEPROM区
                
				break;
			}
			case 0x02:
            {		
				ReturnBootloader();
				break;
            }	
			/*			
			case 0x03:
            {		
				EraseFileAndCardInfo();
				break;
            }	
*/						
											
			default:
            {		
				P1P2Error();
				return;
            }	
		}

	   G_CardStatus = D_CardEmpty;
		
		G_CommandMode = Command_Mode_First;
		
		CommandFinish(0x00);
	}  
}
#endif
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 UpdateFile
  
  Command format:    C0    F0    P1   P2   P3 
  
  Data    content:   P1 P2 transparent file offset
  					 P3    data length                                                       
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       updata the value of the transparent file with varify the DF key
  
  status:			 Code OK!     Test OK!    

 -------------------------------------------------------------------------------------------------------------------*/    

void UpdateFile(void)
{
	WORD  idata offset,fileSize;
	BYTE  idata fileStructure;                                                                  //send 67 00
	UWORD idata fileBodyAddr;

	
	if( G_CurrentEfAddr[G_ChannelNum] == FALSE )                                                                        //当前选择的不是EF
	{	
		NoEfSelected();											//69 86

		return;
	}           

	if (ValueBit(G_SystemStatus,B_cmk) == FALSE)
	{
		SecuritySatisfied();
		
		return;
	}

	fileStructure = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum],F_Type_Offset);

	if(fileStructure != TRANTYPE)
	{
		FileStructureError();

		return ;
	}

	ReceiveData();

	offset = ((P1 << 8)&0xFF00 )|(P2 & 0x00FF);

	fileSize = GetTFSize(G_CurrentEfAddr[G_ChannelNum]);


	if((offset + P3) <= fileSize)
	{
		  fileBodyAddr = FindTFAddr(G_CurrentEfAddr[G_ChannelNum], offset, P3);	 				//根据文件头地址找到文件体地址 

		  WriteEepromEX(fileBodyAddr,(BYTE *)&RecData[0],P3);
		 
		  CommandFinish(0x00);
	}
	else
	{
		P3Error();
	}  


}

#if NATIVE_CARD_UICC	
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ReloadKey
  
  Command format:    C0    F8    00/01   01-05   10 
  
  Data    content:   Pin(8)/Adm(8) + Puk(8)/Isc(8)                                                        
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       Install or Reload the key to surityFile
  
  status:			 Code OK!     Test OK!    

 -------------------------------------------------------------------------------------------------------------------*/    

void ReloadKey(void)
{ 
	KeyStructer xdata admKey;
	BYTE idata result;                       								
	
	if((P1 != 0x00))
	{
		P1P2Error();
		return;
	}   
	                           									
	if( P3 != 0x10 )
    {
		P3Error();													//Send   67 00
		return ;
	}                                        		 				
	if((P2==0xA1)||(P2==0xA2))
	{
		P1P2Error();
		return;
	}
	if(ValueBit(G_AdmStatus,2) == FALSE)
	{											
		SecuritySatisfied();				//send 98 04
		
		return;
	}
  	
	ReceiveData();
	
	switch (P2)
	{
		case 0x01:
			admKey.KeyId = 0x01;
			break;
		case 0x02:
			admKey.KeyId = 0x81;
			break;
		case 0x03:
			admKey.KeyId = 0x0A;
			break;
		case 0x04:
			admKey.KeyId = 0x0B;
			break;
		case 0x05:
			admKey.KeyId = 0x0C;
			break;
		default:
			admKey.KeyId = P2;  //P2:0xA2 Encry Key
			break;
	}


	result = ReadKey(admKey.KeyId, (BYTE*)&admKey.KeyId);	   //P2的取值范围 ：PIN1:01 PIN2:81 ADM1:0A ADM2:0B ADM3:0C p2:01 02 03 04 05

	if(result == FALSE)
	{
		ReferencedDataNotFound();											//6A 88
	
		return;
	}

	admKey.KeyStatus = PIN_ENABLED;

	admKey.KeyLimit = KeyResume(admKey.KeyLimit);

	admKey.IscLimit = KeyResume(admKey.IscLimit);	
	
	memcpy((BYTE *)&admKey.KeyContent[0],(BYTE *)&RecData[0],P3);

	result = WriteKey(admKey.KeyId,(BYTE *)&admKey.KeyId);

	if( result == FALSE)
	{
		AddressInvilid();											//send   94  02
		return;
	}                                       						//send 94 14
 
	ReadPinStatus();

	CommandFinish(0x00);
}



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ReloadManagenerKey or VerifyManagenerKey
  
  Command format:    ReloadManagenerKey:	00    F6    00   00   10
  					 VerifyManagenerKey:	00    FC    00   00   08
  
  Data    content:   ReloadManagenerKey:	old MFkey(8)	new MFkey(8)
  					 VerifyManagenerKey:	Current DF KEY(8)                                                        
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       ReloadManagenerKey:	Verify the MF key and Reload the file system
  					 VerifyManagenerKey:	check the key limit and verify df key and set the status of varible
  
  status:			 Code OK!     Test OK!    

 -------------------------------------------------------------------------------------------------------------------*/
void ReloadVerifyManagenerKey(void)
{	
	BYTE idata mode;
	BYTE idata mfKeyNum,tag,limit;

	mode = ((INS == 0xF6) ? mode_ReloadMFKey : mode_VerifyMFKey);

  	if(P3 != mode)
	{
		P3Error();																//send  6700
		return;
	}

	ReadEeprom(&mfKeyNum, E_MK_START+F_MFKEYLimit_Offset,0x01);

  	if((mfKeyNum&0x0F) == 0x00)
  	{
  		PinBlocked();															//Send 98 40
		return;
	}

	ReceiveData();

	ReadEeprom((BYTE *)&RecData[0x20],E_MK_START,0x08);

	tag = memcmp((BYTE *)&RecData[0],(BYTE *)&RecData[0x20],0x08);

	//SetBit(G_COMMANDPATH,B_ACCESSEX); 

	if(tag == EQUAL)
	{
		if(mode == mode_ReloadMFKey)
		{
			WriteEeprom(E_MK_START,(BYTE *)&RecData[0x08],0x08);
		}
		else	//mode_VerifyMFKey
		{
			SetBit(G_SystemStatus,B_cmk);

			limit = 0x0A;

			WriteEeprom(E_MK_START+F_MFKEYLimit_Offset,&limit,0x01);	 	   //恢复到最大校验次数				
		}

		CommandFinish(0x00);												//Send  90 00
	}
	else
	{
		if(mode == mode_VerifyMFKey)
		{
			ClearBit(G_SystemStatus,B_cmk);
		}		

		mfKeyNum = mfKeyNum - 1;	
		
		WriteEeprom(E_MK_START+F_MFKEYLimit_Offset,(BYTE *)&mfKeyNum,sizeof(BYTE));
		  														
   		SecuritySatisfied();			//Send  9804 / 6982
	}
}
#endif

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ReloadVerifySuperKey
  
  Command format:    C0    85  00   00  10  //RecData[0]- RecData[15] 
  
  					//RecData[0x20]-RecData[0x27] 随机数
  
  Data    content:   void    
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:      验证SuperKey
  
  status:			     

 -------------------------------------------------------------------------------------------------------------------*/    



void ReloadVerifySuperKey(void)
{
	//UWORD xdata fileAddr;
	BYTE idata tempResult;
		
	
	if((P1 != SuperKey_Type)&&(P1 != EncryKey_Type))
	{
		P1P2Error();
		return;
	}

	
	if((P2 != 0x00)&&(P2 != 0x01))
	{
		P1P2Error();
		return;
	}
		
	ReceiveData();

	tempResult = ReadKey(P1,&tempBuf[0]); //tempBuf[4]开始的16字节是Super Key密钥

	if(tempResult == FALSE )                                                     //the key not find 
    {
		ReferencedDataNotFound();											//6A 88
	
		return;
	}
	
	if(P2 == 0x00)  //verify superkey
	{
		
		if(P1 != SuperKey_Type)
		{
			P1P2Error();
			return;
		}
		
		if(P3 != 0x10)
		{
			P3Error();
			return;
		}
		
		if(G_SuperKeyAccess != SuperKeyAccess_GetChallange)  //随机数未获取
		{
			ConditionSatisfied();
			return;
		}
		
				
		TripleDES(&G_SuperKeyRamdomBuf[0],&tempBuf[0x04],&RecData[0x20],CBC_Encrypt,P3,16);
			
		if(memcmp(&RecData[0],&RecData[0x20],P3) == EQUAL)
		{
			G_SuperKeyAccess = SuperKeyAccess_PassVerify;
			
			CommandFinish(0x00);
		}
		else
		{
			G_SuperKeyAccess = SuperKeyAccess_NoVerify;
			AuthenticateError();
		}
		
		return;
	}
	else  //Reload superKey/EncryKey
	{
		if(P3 != 0x20)  //密文的 new superkey(RecData[0]-RecData[15]) + 密文的old superkey RecData[16] -RecData[31]
		{
			P3Error();
			return;
		}
		
		TripleDES(&RecData[0x00],&tempBuf[0x04],&RecData[0x00],CBC_Decrypt,P3,16);
		
		if(memcmp(&RecData[0x10],&tempBuf[4], 0x10) == EQUAL)  //原key相等
		{
			memcpy(&tempBuf[4],&RecData[0], 0x10);  //拷贝新的key在buf中，待写入文件
			
			tempResult = WriteKey(P1,&tempBuf[0]);					//记录回写

	   		if(tempResult == FALSE)
			{
				MemoryError();													//65 81/92 40

				return;
			} 
			
			CommandFinish(0x00);
			
			return;
		}
		else
		{
			ConditionSatisfied();
			return;
		}
		
	}
	
}
#if NATIVE_CARD_UICC
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SetGetCOSCofigData
  
  Command format:    C0    E8  XX   XX  XX  
                                                                                                                                                                                                                                                      
  description:      功能配置指令
  
  status:			     

 -------------------------------------------------------------------------------------------------------------------*/ 
void SetGetCOSCofigData(void)
{
	BYTE  idata len;
	WORD  idata offset;
	
	if((ValueBit(G_SystemStatus,B_cmk) == FALSE)&&(ValueBit(G_AdmStatus,1) == FALSE))	    	  		//必须通过MFKEY校验或ADM1校验
	{			
		SecuritySatisfied();																				   //send 98 04/69 82
		return;
	}

	if(P1 > MODE_CONFIG_READALL)                //不是支持的5种模式之一
	{
		P1P2Error();
		return;
	}

	if((P1 == MODE_CONFIG_WRITE)||(P1 == MODE_CONFIG_READ))	  	//读或写模式,不支持P3=0x00
	{
		if(P3 == 0x00)
		{
			P3Error();
			return;
		}	
	}	   			
	
	if((P1 == MODE_CONFIG_CLEARTAG)||(P1 == MODE_CONFIG_CLEARALL))			
	{
	 	if(P3!=0x00)
		{
			P3Error();
			return;		
		}
	}
			
					
	if((P3 > 0x10)&&(P3!=E_ConfigData_LEN))              				//除了读整个配置区数据指令P3==0x40,目前最大支持P3为0x10,不支持P3为0的情况 
	{
		P3Error();
		return;
	}

	if(P2!=0xFE)	 				  //只在P2=0xFE时,允许P3不为0或1或E_ConfigData_LEN,满足目前需求
	{
		if((P3!=0x00)&&(P3!=0x01)&&(P3!=E_ConfigData_LEN))
		{
			P3Error();
			return;	
		}	
	}
	
	//SetBit(G_COMMANDPATH,B_ACCESSEX);
  	
	if(P1 == MODE_CONFIG_READALL)
	{
		if(P3 != E_ConfigData_LEN)                         
		{
			P3Error();
			return;
		}

		ReadEeprom(RecData,E_COS_ConfigData,P3);			
		SendData();
		
		CommandFinish(0x00);
		return;
		
	}
	else if(P1 == MODE_CONFIG_CLEARALL)               //清除所有配置信息
	{
		if(P2!=0x00)
		{
			P1P2Error();
			return;	
		}
		
		SetEeprom(E_COS_ConfigData,PRIVATETAG_NULL,E_ConfigData_LEN);
		CommandFinish(0x00);
		return;
	}
	else if(P1 == MODE_CONFIG_WRITE)               //写配置信息
	{
		ReceiveData(); 
	}
	
	offset = FindCOSConfigTag(P2,NULL);                      //查找tag
	
	if(offset != OFFSET_NULL)                                  //找到tag
	{
		if(P1 == MODE_CONFIG_CLEARTAG) 
		{		
			SetEeprom(E_COS_ConfigData+offset,PRIVATETAG_INVALID,1); 		
			CommandFinish(0x00);		
			return;
		}
		else if(P1 == MODE_CONFIG_READ)
		{	
			ReadEeprom(RecData,E_COS_ConfigData+offset+2,P3);//读取value
			
			SendData();
								
			CommandFinish(0x00);		
			return;
		}
		else
		{
			ReadEeprom(&len,E_COS_ConfigData+offset+1,1);//读取length

			if(len != P3)                                                                       //数据长度与旧数据长度不相同，返回错误
			{
				TechnicalProblem();
				return;
			}
		}
	}
	else
	{
		
		if(P1 == MODE_CONFIG_WRITE)
		{
			offset = FindCOSConfigTag(PRIVATETAG_NULL,NULL);                     //若不存在，找到空位置存入

			if(offset == OFFSET_NULL)			 //写的模式下offset为0xFF,表示找不到写空间
			{
				NoEnoughSpace();
				return;
			}

		}
	}
	
	if(offset == OFFSET_NULL)
	{
		TechnicalProblem();
		return;
	}

    if((offset+P3+2)>E_ConfigData_LEN)
	{
		NoEnoughSpace();
		return;
	}

	memmove(&RecData[2],&RecData[0],P3);                                         //V
	
	RecData[0] = P2;                                                             //T
	RecData[1] = P3;                                                             //L

	WriteEeprom(E_COS_ConfigData+offset,&RecData[0],P3+2);                      //Write TLV
	
	//执行NATIVE UICC配置TA1（波特率）指令C0E8000401 11/94/95/96，将TA1写入EPX COS TA1实际位置,by xwh 20220329
	if((P1 == MODE_CONFIG_WRITE)&&(P2 == Tag_Config_TA1)&&(P3 == 0x01))
	{
		if((RecData[2] == 0x11)||(RecData[2] == 0x94)||(RecData[2] == 0x95)||(RecData[2] == 0x96))
		{
			SetEepromEX(sysConfig_GetColdAtrIfBase()+1,RecData[2],0x01);
			SetEepromEX(sysConfig_GetWarmAtrIfBase()+1,RecData[2],0x01);
		}
	}
	
	CommandFinish(0x00);
	
} 
#endif
/*                                                                                      
*@Function name :	 读卡片内部信息，包括COS版本，芯片类型等
*
*@Command format:    C0    CA    P1   P2   P3  
*
*@Data    content:   P1P2P3=00 00 00  ,返回卡片基本信息，芯片厂家，芯片型号，COS系列,cradInfoData在芯片底层中定义
						
*@input parameter:   void                                                                
*
*@output parameter:  void                                                                                                           
*
*@description:     
*
*@status:			  
*/
BYTE code  CradInfoData[21] = {0x48,0x44,0x5F,0x48,0x44,0x42,0x32,0x38,0x4B,0x46,0x35,0x30,0x5F,0x39,0x36,0x39,0x35,0x39,0x34,0x31,0x31};  //HD_HDB28KF50_96959411


void GetCardInfoData(void)
{	
	
	BYTE idata infoDataLen;
	
	if((P1 != 0x00)||(P2!=0x00))
	{
		P1P2Error();     //6B00
		return;
	}	

	infoDataLen = sizeof(CradInfoData);
	
	if(P3 != infoDataLen)
	{
		SW1 = 0x6C;
		SW2 = infoDataLen;	
		return;
	}
	
	memcpy(&RecData[0],(BYTE *)CradInfoData ,P3);
	
	SendData();
		
    CommandFinish(0x00);	
  
}


/*
*function : GetCardSpaceInfo
*
*@description: 获取卡片的空间信息到RecData中，格式为MFMaxSize(4bytes)+MFUsed(4bytes)+flashUsedAddr(4bytes),或者当前DF的DFMaxSize(4bytes)+DFUsed(4bytes)+flashUsedAddr(4bytes)
*
*@param      :void 
*
*@return     :void 
*
*@history    : added by zhxx@20200222
*/
void GetCardSpaceInfo(void)
{
	BYTE idata infoDataLen;
	UWORD xdata NVMAvailMemory;

	if((P1!=0x00)&&(P2!=0x00))
	{
		P1P2Error();     //6B00
		return;
	}

  infoDataLen	= sizeof(UWORD);

	if(P3 != infoDataLen)
	{
		SW1 = 0x6C;
		SW2 = infoDataLen;	
		return;
	}

	NVMAvailMemory = GetNVMAvailMemory();

	UWORDToByteBuf(NVMAvailMemory, &RecData[0]);

	SendData();
		
  CommandFinish(0x00);
}

#if NATIVE_CARD_UICC
/*
*function : SetCardLifeCycle
*
*@description: 设置卡片的生命周期，主要用于锁卡
*
*@param      :void 
*
*@return     :void 
*
*@history    : added by zhxx@20200225
*/
void SetCardLifeCycle(void)
{ 																				
	//SetBit(G_COMMANDPATH,B_ACCESSEX);		
	SetEeprom(E_CARD_LIFE_CYCLE,P1,1);	
	SW1 = 0x90;
	SW2 = 0x00;
}

	
/*
*function : SetCardLifeCycle
*
*@description: 设置卡片的生命周期，主要用于锁卡
*
*@param      :void 
*
*@return     :void 
*
*@history    : added by jiangqingbin@20211117	-从epx2.1移植过来
*/
#if UICC__OPERATOR__CHINATELECOM
void SetCARDConfig(void)
{
	SW1 = 0x90;
	SW2 = 0x00;
	
	if (P1 == 0x01)	//P1=1标识是 AMENDMENT_B_CONFIG 的功能
	{	
		//SetEeprom(AMENDMENT_B_CONFIG, P2, 1);
	}
	else if (P1 == 0x02)	//P1=2标识是 TLS_EXTAND_VERSION 的功能 以及 最新增加的配置项,具体参考操作手册
	{
		//SetEeprom(TLS_EXTAND_VERSION, P2, 1);		
	}
	else if (P1 == 0x03)	//P1=3标识是 AMENDMENT_B_Trigger_SMS_POR 的功能
	{
//		if ((P2 != AMENDMENT_B_Trigger_SMS_POR_0) && (P2 != AMENDMENT_B_Trigger_SMS_POR_1) && (P2 != AMENDMENT_B_Trigger_SMS_POR_3))
//		{
//			SW1 = 0x6A;
//			SW2 = 0x80;
//		}
//		else
//		{
//			//SetEeprom(AMENDMENT_B_Trigger_SMS_POR, P2, 1);
//		}
	}
	else if (P1 == 0x04)	
	{
		ReceiveData();
		
		putRFMKey();		//WriteEeprom(E_COS_ConfigData+offset,&RecData[0],P3+2);         
	}
	else if (P1 == 0x05)	
	{
		ReceiveData();
		
		GPStoreSDAdminSessionPara(NULL, RecData);		   
	}
	else //可以扩展p1不同的值配置不同的功能
	{
		SW1 = 0x6A;
		SW2 = 0x80;
	}
}
#endif
#endif

