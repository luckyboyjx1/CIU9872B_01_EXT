/*

 * Copyright (c) 2005,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V6.01

 * PHASE 2  RELEASE:2005-12-25

  * HISTORY: V1.0 创建源文件
  			 V1.1 修改ReloadKey(void)：PIN、PUK的最大错误尝试次数为2E10中对应记录的第3个字节的高4bits
			 V1.2 修改函数EepormErase，修改清除文件系统时的操作，modify by xyz@100112
			 V1.3 修改函数ChangeBaud，增加切换TA1功能中对95比特率的支持，modify by zengxiao@20140718
			 V1.4 修改函数Initializtion，文件头中表示2F或者6F的字节arrName修改为由此字节的bit7表示  @20140727	  xuyz
			 V1.5 修改函数Initializtion  EepormErase ，删除E_SYSTEM_STATUS  E_APP_TYPE的使用   
			 	  去掉不使用的UpdateFile指令															@20150430 by yangjian

********************************for uicc apdu modify*******************************************
			V1.6 :修改函数Initializtion，其ARR记录号改成01	@20151201 by xuyz
			
			
			V1.7 : 修改函数ReloadKey，支持ADM1- ADM10的重载，由于NFC 2.0B 要求支持ADM10 ，此处COS 做了该进，可以根据3F00/2E10的值动态支持ADM1-ADM10 modify by xuyz@20170211
			
 */ 
#include "uicc_main.h"
#include "fileSystem.h"
#include "gsmpublic.h"
#include "fileaction.h"
#include <string.h>
#include <uicc_error.h>
#include "Interface_PlatformFromLTE.h"
#include "Interface_LTEFromPlatform.h"
#include "STKFileView.h"
#include "VMAccessEeprom.h"
#include "cardManager.h"
#include "euiccConfig.h"
#include "BootLifeCycle.h"
/*-------------------------------------------------------------------------------------------------------------------  */

 /*
 Function:   Initializtion
 *
 *@Descrip:    初始化文件系统，创建3F00（一种情况是创建Private文件，另外一种情况是创建果通预置文件）
 *
 *@See:
 *
 *@Param:      void
 *
 *@Return:     TRUE/FALSE
 *
 *@Author:     xwh
 *
 *@History:    V1.0---Create by xwh 20190221
 *             V1.1---modify by zhangxiaoxiao 关掉11pin的开关@20210702, 值与EPX_THC_Mobile5G不一致，是否与文件系统有关？
 *             V1.2---modify by xwh 20211229 初始化FillByte，避免出现随机值的情况,by xwh 20211229
 *             V1.3---modify by xxj@20230417 根据V1.2的修改，对mfFileHead整体进行初始化
 */
void Initializtion(void) 
{
	BYTE  idata result;
	UWORD idata usedSize,mfAddr;
#if (EUICC_ONLY||SE_AND_EUICC)
	BYTE  idata i;
	BYTE  xdata temp[PROFILE_MAX_NUMBER];
#endif
	FileSystemHead xdata  mfFileHead ;	

    ecpMem_Set((BYTE*)&mfFileHead, 0x00, sizeof(mfFileHead));

#if (EUICC_ONLY||SE_AND_EUICC)	
	VMReadBuf(temp,PROFILE_VALIDTAG_CFG,PROFILE_MAX_NUMBER);    //获取当前所有profile的状态
		
	for(i= 0; i< PROFILE_MAX_NUMBER ; i++)
	{		  
		if(temp[i] == PROFILE_VALIDFLAG)                         //存在profile,此时不允许执行再创建3F00
		{
			ConditionSatisfied();
			return;                                        
		}
	}
#endif	
	mfAddr = GetIfMfCreated(MODE_DEFAULT);
	
	if(mfAddr != INVALID_ADDR)                                  //Default文件区已创建，不允许重复创建
	{
		ConditionSatisfied();
		return;  
	}

	if(P1P2P3Check(0x09,FALSE) ==FALSE)
	{
		return;
	}
	
	ReceiveData(); 																	//receive data to recDataBuffer

	mfFileHead.FileId = DF_MF;

 	mfFileHead.FileType = 0x78 ;							//MF as sharable

	mfFileHead.ArrNum = RecData[8];					//MF Arr Num

	mfFileHead.ArrName = F_ARRNAME_2F;             //modify @20140727  bit7为0表示当前的arrName为2F，bit0-bit6
	
	mfFileHead.DIRNum = 0x00;
	
	mfFileHead.LocalPinIndex = 1;                  // added by zhangxiaoxiao@20221202： 创建3F00时，3f00头，这个变量没赋初值
	
	mfFileHead.FillByte = 0x00;                    //初始化FillByte，避免出现随机值的情况,by xwh 20211229
	
	mfAddr = GetIfMfCreated(MODE_PRIVATE);
	
	if(mfAddr != INVALID_ADDR)                     //已经创建了 Private File System
	{
		usedSize = GetFileHeadUsedSize(ADDR_PRIVATE_FILE_START);
		
		mfAddr = ADDR_PRIVATE_FILE_START + usedSize;

		VMWriteUword(RSPDefaultFileSystem_Addr,mfAddr);
	}
	else                                          //没有创建 Private File System
	{
		mfAddr = ADDR_PRIVATE_FILE_START;
	}

	mfFileHead.MaxSize = GetHeapBase() - mfAddr;
	
	if(mfFileHead.MaxSize <= sizeof(FileSystemHead))
	{
		AddressInvilid();
		return;
	}
	
 	mfFileHead.UsedSize = sizeof(FileSystemHead);

 	mfFileHead.ChildDfNum = 0x01;

 	mfFileHead.ChildEfNum = 0x00;

 	mfFileHead.ParentName[0] = DF_MF;

	mfFileHead.ParentName[1] = DF_MF;
#if UICC__FUNCTION__UniversalPin
	mfFileHead.ChvStatus[0] = 0x11;
	mfFileHead.ChvStatus[1] = 0x01;
	mfFileHead.ChvStatus[2] = 0x0A;	
	mfFileHead.ChvStatus[3] = 0xFF;
#else	
	mfFileHead.ChvStatus[0] = 0x01;
	mfFileHead.ChvStatus[1] = 0x81;
	mfFileHead.ChvStatus[2] = 0x0a;	
	mfFileHead.ChvStatus[3] = 0x0b;
#endif
	mfFileHead.Rfu = 0xFF;

	mfFileHead.KeyLmit = 0x0A;

	ecpMem_Copy((BYTE *)&mfFileHead.MF_Key[0], &RecData[0], 0x08);
	
	result = WriteEeprom(mfAddr,(BYTE *)&mfFileHead.FileId,sizeof(FileSystemHead));          //write mf head to the file system start of eeprom 

	if(result == FALSE)
	{
	   	MemoryError();
		return;
	}
	
	G_CardStatus = D_CardInit;	

	G_CurMFFileStart = mfAddr;

	SetChannel(mfAddr,DF_MF,FALSE);											//set the mf the default df

	CommandFinish(0x00);															 //send 90 00
}


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
 
	if( G_CurrentEfAddr[G_ChannelNum] == FALSE )                                                                        //当前选择的不是EF
	{	
		NoEfSelected();											//69 86

		return;
	}           

	if(ValueBit(G_AdmStatus,2) == FALSE)
	{
		SecuritySatisfied();				//send 98 04

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
	 	 WriteEepromEX(G_CurrentEfAddr[G_ChannelNum] + sizeof(TranFileHead) + offset,(BYTE *)&RecData[0],P3);

		 CommandFinish(0x00);
	}
	else
	{
		P3Error();
	}  

}

/*EF文件个数     DFname  EFname1 EFname2 EFname3 ...*/
                                      //DF_AnnexB_DIR,EF_TLSsessionData,EF_HTTPTriggerPara,EF_HTTPSResponseData
short code PrivateFileNameListR[] = {0x03,0xA100,0x1A01,0x1A03,0x1A05,
		                         //DF_CATTP,EF_CATTPPUSH,EF_PDUFILE,EF_EACKSEQ,EF_CATTPDuanDian,EF_CATTPSave
		                     0x05,0xA300,0x3A01,0x3A02,0x3A03,0x3A04,0x3A05,
							 //0x02,0xA400,0x4A01,0x4A02,//
							 //0x01,0xA500,0x5A01,
							 //DF_Toolkit,EF_ToolkitRFMSmsHead,EF_ToolkitRFMSms,EF_ToolkitAppContext,EF_RAMExtendResponse,EF_HTTPRFM,EF_PorDataFile,EF_EnvelopeHandlerBuf,EF_EnvelopSecondHandlerBuf,EF_EnvelopeCBMessage,EF_CSMS_Head,EF_STKAlert,EF_ScriptChainingFile
							 0x0C,0xA800,0x8A01,0x8A02,0x8A04,0x8A05,0x8A06,0x8A07,0x8A08,0x8A09,0x8A0A,0x8A0B,0x8A0C,0x8A0D,
							 //DFProfileId,TemplateFileId,PrivateTemplateFileId,SpecPrivateTemplateFileId,TemplateFileId_2_1
							 0x13,0xA900,0x9A61,0x9A71,0x9A72,0x9A73,0x9A63,0x9A74,0x9A75,0x9A76,0x9A66,0x9A77,0x9A78,0x9A79,0x9A7A,0x9A7B,0x9A7C,0x9A7D,0x9A7E,0x9A6F,0x9A6E};

short  PrivateFileNameListP[] = {0x02,0xB100,0x1B01,0x1B02,
		                             0x0A,0xB200,0x2B80,0x2B81,0x2B82,0x2B83,0x2B84,0x2B7B,0x2B05,0x2B94,0x8A18,0x2B06,
									/* 0x1B02,0x1B1B,0x1B05,0x1B06,0x1B10,0x1B11,0x1B1D,0x1B03,0x1B04,0x1B0D,0x1B0E,0x1BBB,0x1B80,0x1B81,0x1B82,0x1B83,0x1B84*/
									 0x0A,0,0x1B1B,0x1B05,0x1B06,0x1B10,0x1B11,0x1B1D,0x1B03,0x1B04,0x1B0D,0x1B0E,//USIM下面需要创建的文件
									 0x08,0,0x1B1B,0x1B05,0x1B06,0x1B10,0x1B11,0x1B1D,0x1B03,0x1B04,//ISIM下面需要创建的文件
									 0x07,0,0x1B1B,0x1BBB,0x1B80,0x1B81,0x1B82,0x1B83,0x1B84//CSIM下面需要创建的文件
									 };



/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 PrivateFileCheck

  Command format:    void

  Data    content:

  input parameter:   void

  output parameter:  void

  description:       统一检查-R以及-P下面的私有文件是否存在

  status:			 Code OK!     Test OK!

  history:          V1.0 --- create by zhxx@20230313

 -------------------------------------------------------------------------------------------------------------------*/
void PrivateFileCheck()
{

	WORD idata i,colNum,j,k;

	UWORD xdata DFFileAddr,EFFileAddr;


	if(BootLifeCycle_GetLifeCycle() < LIFE_CYCLE_CREATE)//发卡后才做检查
	{
		ConditionSatisfied();
		return;
	}

	for(i=0; i < (sizeof(PrivateFileNameListR)/2);)
	{
		colNum = PrivateFileNameListR[i];

		DFFileAddr = SelectPrivateFile(ADDR_PRIVATE_FILE_START,PrivateFileNameListR[i+1]) ;//选择DF文件

		if(DFFileAddr == FALSE)
		{
			FileNotFound();
			return ;
		}

		for(j=i+2; j< (i+2+colNum) ;j++)
		{
			EFFileAddr = SelectPrivateFile(DFFileAddr, PrivateFileNameListR[j]);//选择EF文件

			if(EFFileAddr == FALSE)
			{
				FileNotFound();
				return ;
			}
		}

		i += (2+colNum);
	}

	if(G_CurEnabledProfileIndex != ILLEGAL_PROFILEINDEX)//存在激活的profile
	{

		for(i=0,k=0; i < (sizeof(PrivateFileNameListP)/2);)
		{
			colNum = PrivateFileNameListP[i];

			k++;//表示行数

			if(k == 3)
			{
				PrivateFileNameListP[i+1] = G_DF_ID_USIM;
			}
			if(k == 4)
			{
				PrivateFileNameListP[i+1] = G_DF_ID_ISIM;
			}
			if(k == 5)
			{
				PrivateFileNameListP[i+1] = G_DF_ID_CSIM;
			}

			if(PrivateFileNameListP[i+1] != 0)
			{
				DFFileAddr = SelectFile(E_FILE_START,PrivateFileNameListP[i+1]) ;//选择ADF文件

				if(DFFileAddr == FALSE)
				{
					FileNotFound();
					return ;
				}

				for(j=i+2; j< (i+2+colNum) ;j++)
				{
					EFFileAddr = SelectPrivateFile(DFFileAddr, PrivateFileNameListP[j]);//选择EF文件

					if(EFFileAddr == FALSE)
					{
						FileNotFound();
						return ;
					}
				}
			}

			i += (2+colNum);
		}

	}

	CommandFinish(0x00);
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ReloadKey
  
  Command format:    C0    F8    00/01   01-05   10 
  
  Data    content:   Pin(8)/Adm(8) + Puk(8)/Isc(8)                                                        
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       Install or Reload the key to surityFile
  
  status:			 Code OK!     Test OK!    

  history:          V1.0 --- create function
					V1.1 --- modify by xwh 20211213，ReloadKey只有在2G条件下才把02 PIN转换成81 PIN（跟校验PIN保持一致）
					V1.2 --- modify by xwh 20220106, 按照TPX 5G的指令格式，兼容组件指令
					V1.3 --- modify by zhxx 20220805, 权限修改，产业化需求，euicc时，如果profile中没有ADMB，后个人化就会有问题，因此先判断ADMB是否存在，不存在的话再校验ADMA
 -------------------------------------------------------------------------------------------------------------------*/   
void ReloadKey(void)
{ 
	KeyStructer xdata admKey;
	BYTE idata result;   
    PukStructer	xdata pukInfo;	

	if(P1 != 0x00)//P1:00,reload PIN  and PUK
	{
		P1P2Error();
		return;
	}   
	                           									
	if(P3 != 0x10)
    {
		P3Error();													//Send   67 00
		return ;
	}                                        		 				
//未锁卡时可以执行该指令
	//result = ReadEECacheByte(CARD_LIFE_CYCLE);
	
    //if((result&0x0F) == 0x0F)
    //{
	//	SecuritySatisfied();
	//	return ;
    //}

//	result = JcvmConfig_GetSEState();//VMReadByte(CARD_LIFE_CYCLE);
//
//	if(result > CARD_OP_READY)   //锁卡后不能执行该指令
//	{
//		//if (ValueBit(G_SystemStatus,B_cmk) == FALSE)
//		//if(SECURITY_STATE_MFKEY_VERIFIED != g_securityState)
//		{
//			SecuritySatisfied();
//			return ;
//		}
//	}

	//modify by zhxx@20220805:产业化需求，euicc时，如果profile中没有ADMB，后个人化就会有问题，因此先判断ADMB是否存在，不存在的话再校验ADMA
	result = ReadKey(0x0B, (BYTE*)&admKey.KeyId);	   //先获取ADMB是否存在

	if(result == FALSE)//如果不存在ADMB,校验ADMA
	{
		if(ValueBit(G_AdmStatus,1) == FALSE)
		{
			SecuritySatisfied();				//send 98 04
			return;
		}
	}
	else//如果存在ADMB,校验ADMB
	{
		if(ValueBit(G_AdmStatus,2) == FALSE)
		{
			SecuritySatisfied();				//send 98 04
			return;
		}
	}



	ReceiveData();	

	if(!(((P2&0x7F) >= 1 && (P2&0x7F) <=8) || ((P2&0x7F) >= 0xA && (P2&0x7F) <= 0x0E))) //PIN的取值范围0x01~0x08   0x81~0x88  0x0A~0x0E   0x8A~0x8E
	{
		IncorrentParameter();
		return;
	}

	//ReloadKey只有在2G条件下才把02 PIN转换成81 PIN（跟校验PIN保持一致）
	if(G_CommandMode == Command_Mode_2G)
	{
		P2 = ((P2 == 2) ? 0x81 : P2);
	}
	
	result = ReadKey(P2, (BYTE*)&admKey.KeyId);	   //P2的取值范围 ：PIN1:01 PIN2:81 ADM1:0A ADM2:0B ADM3:0C

	if(result == FALSE)
	{
		ReferencedDataNotFound();											//6A 88
	
		return;
	}
	
	admKey.KeyStatus |= PIN_ENABLED;

	admKey.KeyLimit = KeyResume(admKey.KeyLimit);
	
	ecpMem_Copy((BYTE xdata *)&admKey.KeyContent[0],(BYTE xdata *)&RecData[0],8);//更新PIN的内容

	result = WriteKey(P2,(BYTE xdata *)&admKey.KeyId);	

	if(result == FALSE)
	{
		AddressInvilid();											//send   94  02
		return;
	}                                       						//send 94 14
	
	if(ReadOrWritePukInfo(admKey.pukRef,(BYTE*)&pukInfo.PukId,Read_Key) != FALSE)//读取P2所指向的puk信息
	{
		pukInfo.PukLimit = KeyResume(pukInfo.PukLimit);	
	
		ecpMem_Copy((BYTE xdata *)&pukInfo.PukContent[0],(BYTE xdata *)&RecData[8],8);//更新P2所指向的puk信息
		
		if(ReadOrWritePukInfo(admKey.pukRef,(BYTE*)&pukInfo.PukId,Write_Key) == FALSE)
		{
			AddressInvilid();											//send   94  02
			return;
		}
	}
	 	
	CommandFinish(0x00);
}
