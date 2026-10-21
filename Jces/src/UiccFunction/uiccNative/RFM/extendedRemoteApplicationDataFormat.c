/*
*@Vesion         :V1.0
*
*@description    :该文件主要用于实现TS102 226 中5.2 Extended Remote Application Data Format所描述的功能
*
*@history        :V1.0: create file
*				  基于条件:不支持TAR不同,短信批次相同的情况
*				          V1.1 修改函数ExtendedRFMExcuteCommand,增加备份G_CurrentAdfName[G_ChannelNum]与G_CurrentSfi,G_ChannelNum                 
*/
#include  "typedef.h"
#include  "uicc_main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include  "error.h"
#include  "arithmatic.h"
#include  "ceph.h"
#include  "interface.h"
#ifdef CATTP
#include "CATTP.h"
#endif


UWORD xdata G_FourByteTest;								//for test
WORD xdata G_TwoByteTest;	
BYTE xdata G_OneByteTest;	

#ifdef LDFile
#include  "LDFile.h"

BYTE xdata G_RFMAllReceived;				//表示已收完一个批次的RFM短信 ，准备处理
BYTE xdata G_ExtendedRFMComNUM;     //扩展型RFM指令执行个数，用于response时使用     ;  该字段后期可考虑与CATTP中相关全局变量共用
BYTE xdata G_CurrentChainingValueTag;   //保存当前操作的ChainingValue
WORD xdata G_ErrorActionOffsetLgth[2];      //指示当前最后一个ErrorActionTLV在文件中的偏移
BYTE xdata G_EarlyResponseTag ;                 //标志当前是否存在early response TAG
WORD xdata G_RFMResponseLgth;                  //当前响应数据长度

BYTE xdata G_CompactExtendedTag;              //根据该字段表明当前文件系统 是共享文件(0x20) 还是CSIM ADF(0x40);卡片初始化时需将该字段初始化为0


WORD xdata  G_CurrentCommandOffset;  //当前APDU 指令在文件中的偏移
BYTE xdata  G_CurrentComNUM;         //当前指令执行个数
BYTE xdata  G_PORTag;               //判断当前是否需要发送POR
BYTE xdata  G_PORDeliverSubmit;      //判断上行响应是  SMS-Deliver 还是 SMS-Submit  根据测试部陈科SMS-Deliver  只取一条数据，不在连续多条C0取响应数据。影响手机兼容性 
BYTE xdata  G_RFMFileIn2F08;			 //表示要进行RFM操作的文件是否在3F00/2F08文件中有定义,默认为FLASE



extendedRFMContext xdata ChainingValue;  //note:为保持一致性,卡片复位后,若firstScriptValue = 01 或firstScriptValue = 0x11 &&previousValue = 0x03 ,则清空该文件
/*
*@description:获取当前执行文件的路径
*
*@param      :buf
*
*@return
*
*@history
*/
BYTE RFMGetCurrentEFPath(BYTE * currentPath)
{
	WORD xdata fileName[0x02];
	
	memset(currentPath, 0xFF, 0x08);
	
	if(G_CurrentDfName[G_ChannelNum] != DF_MF)  //当前访问的EF的父目录不是MF，继续查找上级目录
	{
		 ReadEeprom((BYTE *)&fileName,G_CurrentDfAddr[G_ChannelNum] + F_ParentName_Offset,2*sizeof(WORD));
		
		 if(fileName[1] != DF_MF)  //三级目录全适用
		 {
		 	currentPath[0] = (fileName[0] >>8)&0xFF;
		 	
		 	currentPath[1] = fileName[0]&0xFF;
		 	
		 	currentPath[2] = (fileName[1] >>8)&0xFF;
		 	
		 	currentPath[3] = fileName[1]&0xFF;
		 	
		 	currentPath[4] = (G_CurrentDfName[G_ChannelNum] >>8)&0xFF;
		 	
		 	currentPath[5] = G_CurrentDfName[G_ChannelNum]&0xFF;
		 	
		 	currentPath[6] = (G_CurrentEfName[G_ChannelNum]>>8)&0xFF;
		 	
		 	currentPath[7] = G_CurrentEfName[G_ChannelNum]&0xFF;
		 	
		 	return 0x06;
		 	
		 }
		 else
		 {
		 	currentPath[0] = (fileName[1] >>8)&0xFF;
		 	
		 	currentPath[1] = fileName[1]&0xFF;
		 	
		 	currentPath[2] = (G_CurrentDfName[G_ChannelNum] >>8)&0xFF;
		 	
		 	currentPath[3] = G_CurrentDfName[G_ChannelNum]&0xFF;
		 	
		 	currentPath[4] = (G_CurrentEfName[G_ChannelNum]>>8)&0xFF;
		 	
		 	currentPath[5] = G_CurrentEfName[G_ChannelNum]&0xFF;
		 	
		 	return 0x04;	 
		 	
		 }		
	}
	else
	{
 	 	currentPath[0] = (G_CurrentDfName[G_ChannelNum] >>8)&0xFF;
	 	
	 	currentPath[1] = G_CurrentDfName[G_ChannelNum]&0xFF;
	 	
	 	currentPath[2] = (G_CurrentEfName[G_ChannelNum]>>8)&0xFF;
	 	
	 	currentPath[3] = G_CurrentEfName[G_ChannelNum]&0xFF;	
	 	
	 	return 0x02;
	}
	
}
/*
*@description: 判断当前正在执行的文件操作是否满足EFFCL文件中所规定的权限
*
*@param      ：efaddr ，action
*
*@return     ：TRUE：允许执行操作/FALSE：权限不满足
*
*@history
*/
BYTE RFMAccessCondition(UWORD fileAddr,BYTE action)
{	
	WORD xdata fileName;
	BYTE idata fileType,i;
	BYTE xdata currentPath[0x08];  //最多支持3级目录，EF FCL文件初始化记录全ff
	UWORD xdata FCLfileAddr;
	BYTE xdata name[0x09];
	
	G_RFMFileIn2F08 = FALSE;											//执行前,默认该文件没有在2F08中有配置
	
	memset((BYTE *)currentPath, 0xFF, sizeof(currentPath));
	
	i = RFMGetCurrentEFPath(currentPath);
	
	//if(G_CompactExtendedTag == TAR3_SHARE_FILE)  //共享文件系统只能访问MF，MF下的所有DF和EF文件，应用开始运行时，默认当前路径为MF
	if ((G_CompactCompressTag == TAR3_SHARE_FILE)		//共享文件
		||(G_CompactCompressTag == TAR3_SIM)
	 )
	{
		fileName = ((WORD)currentPath[2]<<8)+currentPath[3];
		
		if((fileName == G_DF_ID_CSIM)||(fileName == G_DF_ID_USIM))  // 当前文件为CSIM / USIM / ISIM
		{
			G_RFMFileIn2F08 = TRUE;							  	//此情况可认为在3F00/2F08中有配置
			return FALSE;
		}
	}
	
	if(G_CurrentEfName[G_ChannelNum] == FALSE)  //对于删除文件而言，当前拼装的数据经过上述组装可能只是3F000000等结构
	{
		ReadEeprom((BYTE *)&fileName,fileAddr,sizeof(WORD));
		
		currentPath[i] = (fileName>>8)&0xFF;
	 	
	 	currentPath[i+1] = fileName&0xFF;	
	}
	

	if(((action == A_CreateDF)||(action == A_CreateEF))&&(INS == 0xE0))  //创建文件时，不需要在此验证权限
	{
		
		fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

		if( fileType == ADFTYPE )
		{
			G_RFMFileIn2F08 = TRUE;							  	//此情况可认为在3F00/2F08中有配置
			return TRUE;
		}
	}
	
	FCLfileAddr = SelectLFFile(DF_MF,EF_FCL);
	
	if(FCLfileAddr == FALSE)
	{
		return TRUE;
	}
	
	if(action == A_ReadFile)
	{
		action = Action_ReadBit;
	}
	else if(action == A_UpdateFile)
	{
		action = Action_UpdateBit;
	}
	else if(action == A_DeleteFile)
	{
		action = Action_DeleteBit;		
	}
	else if(action == A_ResizeFile)
	{
		action = Action_ResizeBit;		
	}
	/*
	else if(action == A_DeleteFile)
	{
		if(INS == 0xD4)  //resize
		{
			action = Action_ResizeBit;
		}
		else
		{
			action = Action_DeleteBit;
		}
	}
	*/
	else if(action == A_DeactivateFile)
	{
		action = Action_DeactiveBit;
	}
	else if(action == A_ActivateFile)
	{
		action = Action_ActiveBit;
	}
	
	for(i = 0; i <GetLFRecNumLgth(FCLfileAddr, FileNum) ; i++)
	{
		ReadLFBuf(FCLfileAddr,i+1, 0x00, (BYTE *)name, 0x09);  //读取文件路径ID  
		
		fileName = ((WORD)name[0]<<8)+name[1];
		
		if(fileName != DF_MF)   //卡端获取文件的权限都从MF开始，为防止FCL中某些文件的起始路径不是0x3F00，在此处添加MF一级目录
		{
			memmove(&name[2] ,&name[0],0x06);
			
			name[0] = 0x3F;
			
			name[1] = 0x00;
		}
		
		if(memcmp((BYTE *)name,(BYTE *)currentPath,0x08) == EQUAL)  //在FCL文件中找到当前正在被操作的文件
		{		
			G_RFMFileIn2F08 = TRUE;							  	//找到该文件在3F00/2F08中有配置
			return ValueBit(name[0x08], action);
		}
	}
	
	//return FALSE;  //在FCL列表中，未找到相关文件,规范0815版本指令FCL列表中的文件为不可被远程操作的文件，对于不在FCL列表中的文件是否允许远程操作，需求未详细说明，暂时不允许RFM操作不在FCL列表中的文件，待需求明确后修改
	return TRUE;	//不在FCL列表中的文件，都默认允许RFM
}

/*
*@description :将当前指令的R-APDU更新在ResponseFile中,所有指令执行结束后,将Number of executed Command TLV objects 更新到文件中
*             : response文件前2字节暂时不存储任何信息，作为加密时缓存tag使用，第3.4字节存储长度，第5-7字节表示CommandNums，该字段在指令流程结束后，最后更新该tag信息
*
*@param       :BYTE tag
*
*@return
*
*@history
*/
BYTE ExtendedRFMUpdateResponseTLV(BYTE tag)
{
	WORD idata lgth;
	WORD xdata offset,fileSize;
	UWORD xdata fileAddr;
	BYTE xdata tmp[0x05];
	
	fileAddr = SelectTFFile(DF_OTA,EF_ResponseFile);
	
	if(tag == TAG_ClearResponseFile)
	{
		ReadTFBuf(fileAddr, 21,&tmp[0], 0x05);	  //  RPL + RHL +TAR +COUNTR + PCNTR +STATUS +CCLength(留8字节) +RFU +RFU  =23

		if((tmp[0] == 0x00)&&(tmp[1] == 0x00)&&(tmp[2] == 0x00)&&(tmp[3] == 0x00)&&(tmp[4] == 0x00))
		{
			return TRUE;
		}
		 
		SetTFConst(fileAddr,21, 0x00,0x05);
		
		return TRUE;
	}
	
	if(G_EarlyResponseTag != FALSE)  //假定卡片碰到early response后仅允许发送
	{
		return TRUE;
	}
	
	fileSize = GetTFSize(fileAddr);
	
//	ReadTFBuf(fileAddr, 0x02, (BYTE *)&offset ,sizeof(WORD));
	ReadTFBuf(fileAddr, 23, (BYTE *)&offset ,sizeof(WORD));	    //  RPL + RHL +TAR +COUNTR + PCNTR +STATUS +CCLength(留8字节) +RFU +RFU  =23
	
	if(tag == TAG_R_APDU)  //判断response buffer是否够用
	{
		if(G_RFMResponseLgth <= 0x7D)  //lgth <= 0x7F (包括两个字节的状态响应字节)
		{
			offset += G_RFMResponseLgth+4;
		}
		else  if(G_RFMResponseLgth <= 0xFD)  // 0x80<=lgth  <=0xFF
		{
			offset += G_RFMResponseLgth+5;
		}
		else
		{
			offset += G_RFMResponseLgth+6;
		}
		
		if((offset+7) >fileSize)  //response buffer不够用
		{
			return NoMemory;
		}
	}
	else if(tag == TAG_BadFormat)
	{
		offset += 3;
		
		if((offset+7) >fileSize)  //response buffer不够用
		{
			return NoMemory;
		}
	}
	
	if(tag == TAG_BadFormat)   //update bad format TLV
	{
		tempBuf[0] = TAG_BadFormat;
		
		tempBuf[1] = 0x01;
		
		tempBuf[2] = RecData[0];
		
		ReadTFBuf(fileAddr, 23, (BYTE *)&offset ,sizeof(WORD));
		
		WriteTFBuf(fileAddr, offset+7+21, &tempBuf[0],0x03);
		
		offset += 3;
		
	}
	else if(tag == TAG_NumberCommandTLV)   //update Number of executed Command TLV objects
	{
		tempBuf[0] = TAG_NumberCommandTLV;
		
		tempBuf[1] = 0x01;
		
		tempBuf[2] = G_ExtendedRFMComNUM;
		
		ReadTFBuf(fileAddr, 23, (BYTE *)&offset ,sizeof(WORD));
		
		WriteTFBuf(fileAddr, 0x04+21, &tempBuf[0], 0x03);
		
		offset += 3;
	}
	else if(tag == TAG_ScriptChainingResponse)  //table 5.15
	{		
		tempBuf[0] = TAG_ScriptChainingResponse;
		
		tempBuf[1] = 0x01;
		
		tempBuf[2] = RecData[0];
		
		WriteTFBuf(fileAddr, 0x07+21, &tempBuf[0],0x03);
		
		offset = 0x03;
	}
	else if(tag == TAG_ImmediateActionResponse)
	{
		tempBuf[0] = TAG_ImmediateActionResponse;
		
		tempBuf[1] = 0x01;
		
		tempBuf[2] = RecData[0];
		
		WriteTFBuf(fileAddr, 0x07+21, &tempBuf[0],0x03);
		
		offset = 0x03;
	}
	else if(tag == TAG_R_APDU)	
	{
		ReadTFBuf(fileAddr, 0x02+21, (BYTE *)&offset ,sizeof(WORD));
		
		SetTFConst(fileAddr, offset+7+21, TAG_R_APDU,0x01); //写入tag

		if ((G_PORDeliverSubmit == 0x88)&&(G_RFMResponseLgth>=0x70)&&(G_PORTag != 0x03))                       //SMS-Deliver  取数据  只取一条						   G_PORTag  的值来区别是普通RFM    还是CATTP执行 RFM
		{
			G_RFMResponseLgth = 0x70;	
		}
		else if((G_PORTag == 0x03)&&(G_PORDeliverSubmit == 0x88)&&(G_RFMResponseLgth>=0xC0))	                     //CATTP 执行RFM  不支持级联上行   压缩格式
		{
			 G_RFMResponseLgth = 0xC0;
		}
		
		if(G_RFMResponseLgth <= 0x7D)  //lgth <= 0x7F (包括两个字节的状态响应字节)
		{	
			SetTFConst(fileAddr, offset+8+21, G_RFMResponseLgth+2,0x01); 
			
			RecData[G_RFMResponseLgth]  = SW1;
			
			RecData[G_RFMResponseLgth+1]  = SW2;
			
			WriteTFBuf(fileAddr, offset +9+21, &RecData[0],G_RFMResponseLgth+2);
			
			offset += G_RFMResponseLgth+2 +2;
		}
		else  if(G_RFMResponseLgth <= 0xFD)  // 0x80<=lgth  <=0xFF
		{
			SetTFConst(fileAddr, offset+8+21, 0x81,0x01); 
			
			SetTFConst(fileAddr, offset+9+21, G_RFMResponseLgth+2,0x01);
			
			RecData[G_RFMResponseLgth]  = SW1;
			
			RecData[G_RFMResponseLgth+1]  = SW2;
			
			WriteTFBuf(fileAddr, offset +10+21, &RecData[0],G_RFMResponseLgth+2);
			
			offset += G_RFMResponseLgth+2 +3;
		}
		else 
		{		
			lgth = G_RFMResponseLgth +2;  //添加状态响应字节的长度
			
			SetTFConst(fileAddr, offset+8+21, 0x82,0x01);
			
			WriteTFBuf(fileAddr, offset+9+21, (BYTE *)&lgth,sizeof(WORD));
			
			if(G_RFMResponseLgth == 0x100)
			{
				WriteTFBuf(fileAddr, offset +11+21, &RecData[0],0x80);
				
				WriteTFBuf(fileAddr, offset +11+0x80+21, &RecData[0x80],0x80);
			}
			else
			{
				WriteTFBuf(fileAddr, offset +11+21, &RecData[0],G_RFMResponseLgth);
			}
			
			SetTFConst(fileAddr, offset +11+G_RFMResponseLgth+21, SW1,0x01);
			
			SetTFConst(fileAddr, offset +11+G_RFMResponseLgth+1+21, SW2,0x01);
			
			offset += G_RFMResponseLgth+2 +4;
		}				
	}
	
	WriteTFBuf(fileAddr, 23, (BYTE *)&offset, sizeof(WORD));		 //RPL + RHL +TAR +COUNTR + PCNTR +STATUS +CCLength(留8字节) +RFU +RFU  =23  在原有的文件前面增加了  21个字节
	
	return TRUE;
}
/*
*@description: 处理扩展型指令中的C-APDU指令
*
*@param      :当前C-APDU的长度,当前待读取数据的起始偏移
*
*@return
*
*@history
*/
INSLIST code remoteRfm_list[]= 
{ 
  {0xA4,SelectCommand},   //CASE 4 OR CASE 3 command ,返回9000时,为CASE 3 ,else为CASE 4指令
  {0xD6,UpdateBinary},
  {0xDC,UpdateRecord},
  {0xA2,SeekCommand},     //CASE 4 OR CASE 3 command, 返回9000时,为CASE 3 ,else为CASE 4指令
  {0x32,IncreaseCommand}, //CASE 4 command
  {0x20,VerifyChangeUnblockChv},
  {0x24,VerifyChangeUnblockChv},
  {0x26,ChvHandle},
  {0x28,ChvHandle},
  {0x2C,VerifyChangeUnblockChv},
  {0x04,FileStatusHandler},
  {0x44,FileStatusHandler},
  {0xB0,RemoteReadBinary},    //case 2
  {0xB2,ReadRecord},    //case 2
  {0xE0,CreateFile},
  {0xE4,DeleteFile},
  {0xD4,ResizeFile},
#ifdef CATTP	
  {0xEC,PushCommand},
#endif	
  {EOL,InsError},
};
/*
*@description
*
*@param
*
*@return
*
*@author
*
*@history
*/
void RemoteReadBinary(void)
{
	BYTE idata actionTag;
	WORD idata fileLgth ;
	EFileHead xdata transefFile;


	actionTag = ComputeBinary(A_ReadFile);

	if(actionTag == FALSE)
	{
		return ;
	}

	fileLgth = ((P1 << 8)&0xFF00 )|(P2 & 0x00FF);

	if(P3==0x00) //lgth是256个字节
	{	
		ReadEeprom((BYTE *)&transefFile.FileId,G_CurrentEfAddr[G_ChannelNum],sizeof(EFileHead));
	
		if(transefFile.efSize.SpaceSize <( fileLgth+0x100))  // 当剩余有效数据不足0x100时，返回剩余有效数据
		{
			P3 = transefFile.efSize.SpaceSize - fileLgth; 
			
			ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE *)&RecData[0],P3);		
			
			SW1 = 0x90;
				
			SW2 = 0x00;  
		}
		else
		{
			
			ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE *)&RecData[0],0x80);

			ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth+0x80,(BYTE *)&RecData[0x80],0x80);
			
			
			if(transefFile.efSize.SpaceSize ==( fileLgth+0x100))
			{
				SW1 = 0x90;
				
				SW2 = 0x00;  
			}
			else
			{

				SW1 = 0x62;
				
				SW2 = 0xF1;  //剩余数据被切断
			}
		}
		
	}
	else
	{
		ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE xdata *)&RecData[0x00],P3);

		SW1 = 0x90;
			
		SW2 = 0x00;  
	}				                                                                
}

/*
*@description:
*
*@param
*
*@return
*
*@author
*
*@shitory
*/
BYTE ExtendedRFMProcessCAPDU(WORD lgth,WORD offset)
{
	BYTE idata i,num,le,result,apduTag;
	UWORD xdata fileAddr;
	
	G_RFMResponseLgth = 0x00;

	apduTag = FALSE;		
	
	fileAddr = SelectTFFile(DF_OTA,LDFile_RelaxFile);  //缓存解密文件  5E03
	
	ReadTFBuf(fileAddr, offset, &G_APDU[0],0x05);   //获取5个字节的命令头 CLA INS P1 P2 P3 
	
	i = 0x00;
	
	if((INS != 0xB0)&&(INS != 0xB2))  //当前指令不是CASE 2 COMMAND,则有输入数据需要读取
	{		
		ReadTFBuf(fileAddr, offset+5, &RecData[0],P3);   
		
		i = P3;
	}
	else 
	{
		if(lgth != 0x05) //read 的长度 只可能为05
		{
			return ExtendedRFMProcessBadTLV(FLAG_WrongLgth);					
		}
	}
	
	if(((G_CompactExtendedTag == TAR3_SIM)&&(CLA != 0xA0))
	 || ((G_CompactExtendedTag == TAR3_CSIM_ADF)&&(CLA != 0x80)&&(CLA != 0x00))
	 || ((G_CompactExtendedTag == TAR3_USIM_ADF)&&(CLA != 0x80)&&(CLA != 0x00))
	 )
	 {
	 		G_RFMResponseLgth = 0x00;
    		
    		result = ExtendedRFMUpdateResponseTLV(TAG_R_APDU);
    		
    		if(result != TRUE)
    		{
    			return NoMemory;
    		}
    		
    		ExtendedRFMUpdateResponseTLV(TAG_NumberCommandTLV);

			return 	CipheringError;
	 }

	if(G_CommandMode == Command_Mode_First)					//第一个APDU命令
	{
		if (CLA == 0xA0)
		{
			G_CommandMode = Command_Mode_2G;					//当前为2G操作模式
		}
		else if (((CLA&0xFC) == 0x80)||((CLA&0xF0) == 0x00))
		{
			G_CommandMode = Command_Mode_3G;					//当前为3G操作模式
					
		}		
	}
	
	G_ChannelNum = CLA&0x03;
	
	if (((G_CommandMode == Command_Mode_2G)^(CLA==0xA0))||(((CLA&0xFC) != 0x80)&&((CLA&0xF0) != 0x00)&&(CLA != 0xA0)))		//拒绝非激活的操作模式							
	{
		return ExtendedRFMProcessBadTLV(FLAG_TagNotFound);   //CLA  error
	}
	
	if(INS == 0xE0)
	{
		SetBit(G_SystemStatus,B_cmk);

		result = FindTag(&RecData[2],0x84,P3 -2);										//Not Perform Creat ADF
	
		if(result == TRUE)
		{
			SW1 = 0x6A;
			SW2 = 0x80;	
			
			apduTag = TRUE;												 	//Send 6A 80
			
		}

		ClearBit(G_SystemStatus,B_RfmPerform);	//远程文件管理在创建文件时，使用的权限是0x8B，因此在这里先将RFM标记清掉
	}

	if(INS == 0xA4)
	{
		if( (G_CommandMode == Command_Mode_3G)&&(P1 == 0x04))  //SELECT不支持 P1 = 04的情况
		{
			SW1 = 0x6A;
			SW2 = 0x86;
			apduTag = TRUE;		
		}
		
		if ((G_CommandMode == Command_Mode_2G)&&(P2 != 0x00))
		{
			SW1 = 0x6B;
			SW2 = 0x00;
			apduTag = TRUE;		
		}
	}

	if(INS == 0xC0)
	{
		if(P3 == 0x00)
		{
			P3 = G_CurrentReponse[G_ChannelNum];
		}
	}
		
//	if(INS == 0xE4)  //对于删除指令，先备份fileName
//	{
//		fileName = ((RecData[0]<<8)&0xFF00)+RecData[1];
//	}	

	if(apduTag == FALSE)
	{
		for(num=0;;num++)
	    { 
	 		if((remoteRfm_list[num].ins == INS)||(remoteRfm_list[num].ins == EOL))
	        {	
	        	if(remoteRfm_list[num].ins == EOL)  //CLA  非法,BAD format TLV
	        	{
	        		return ExtendedRFMProcessBadTLV(FLAG_TagNotFound);     	        	
	        	}
	        				
				remoteRfm_list[num].func();
	
	            break;
			}                           												//perform the command now!
	    } 
	}

	if(INS == 0xE0)
	{
		SetBit(G_SystemStatus,B_RfmPerform);	//设置RFM状态
	}

    
    ReadTFBuf(fileAddr, offset+5+i, &le,sizeof(BYTE));  //读取le字节,若存在的话
    
    if((INS == 0xB0)||(INS == 0xB2))
    {
    	if((SW1 == 0x62)&&(SW2 == 0xF1))  //,暂时只支持最大256字节的读取.the data is truncated in the response,This shall terminate the processing of the command list.
    	{
    		G_RFMResponseLgth = 0x100;  //G_RFMResponseLgth存储的是当前响应数据的长度,=0时表示存储256字节,数据保存在RecData中
    		
    		result = ExtendedRFMUpdateResponseTLV(TAG_R_APDU);
    		
    		if(result != TRUE)
    		{
    			return NoMemory;  //response buffset不够，终止后续操作
    		}
    		
    		ExtendedRFMUpdateResponseTLV(TAG_NumberCommandTLV);
    		
    		return ReadFailed;   //terminate
    	}
    	else if((SW1 == 0x90)&&(SW2 == 0x00))
    	{
    		if(P3 == 0x00)   //当P3 = 0x00时，获取的数据为记录的整条记录或透明文件的所有有效数据,read时，处理：遇到P3 = 00的情况，将P3 替换成记录长度 or binary文件，当剩余有效数据<0X100时，P3 为剩余有效数据长度，否则为00（表示256字节）
    		{
    			G_RFMResponseLgth = 0x100;
    		}
    		else
    		{
    			G_RFMResponseLgth = P3;
    		}
    		
    		result = ExtendedRFMUpdateResponseTLV(TAG_R_APDU);  //指令执行成功,继续执行后续指令
    		
    		if(result != TRUE)
    		{
    			return NoMemory;
    		}
    	}
    	else   //执行执行失败
    	{
    		G_RFMResponseLgth = 0x00;
    		
    		result = ExtendedRFMUpdateResponseTLV(TAG_R_APDU);
    		
    		if(result != TRUE)
    		{
    			return NoMemory;
    		}
    		
    		ExtendedRFMUpdateResponseTLV(TAG_NumberCommandTLV);
    		
    		return ReadFailed;
    	}
    	
		return TRUE;
    }  
    else if((INS == 0x32)||(INS == 0xA2)||(INS == 0xA4))  //increase OR SEEK OR SELECT
    { 		
    	if((SW1 == 0x61)||(SW1 == 0x9F)||(SW1 == 0x90))  //指令执行成功
    	{
    		if((i + 5 ) == lgth)  //不存在le字节
			{
				return TRUE;
			}
			
			if(SW1 == 0x90)   //当前无响应数据，命令执行成功
			{
				G_RFMResponseLgth = 0x00;
			}
			else if(le == 0x00)  //对所有的case 2 or case 4指令，返回所有的有效响应数据
			{
				G_RFMResponseLgth = SW2;
		
				SW1 = 0x90;
				SW2 = 0x00;
				
			}		
			else if(le <= SW2)
			{
				G_RFMResponseLgth = le;
				
				SW1 = 0x90;
				SW2 = 0x00;
			}
			else
			{
				G_RFMResponseLgth = 0x00;
				
				SW1 = 0x6C;   //期望获取的数据长度过长,返回响应状态字		
			}
			
			result = ExtendedRFMUpdateResponseTLV(TAG_R_APDU);
			
			if(result != TRUE)
    		{
    			return NoMemory;
    		}
		
			if(SW1 == 0x6C)
			{
				ExtendedRFMUpdateResponseTLV(TAG_NumberCommandTLV);
    		
    			return  CipheringError;  //因标准未详细规定其余指令执行失败的POR响应值，因此暂以CipheringError表示失败信息
			}
			else
			{
				return TRUE;
			}
    		
    	}
    	else if((INS == 0xA4)&&(SW1 == 0x62)&&(SW2 == 0x83))   //选择的文件为失效文件，没有le字节则继续执行
    	{ 	
    		SW1 = 0x90;
			SW2 = 0x00;
			
    		if((i + 5 ) == lgth)
    		{	
    			return TRUE;			    		
			}
			
			if(le == 0x00)  //对所有的case 2 or case 4指令，返回所有的有效响应数据
			{
				G_RFMResponseLgth = G_CurrentReponse[G_ChannelNum];
				
			}		
			else if(le <= G_CurrentReponse[G_ChannelNum])
			{
				G_RFMResponseLgth = le;
			}
			else
			{
				G_RFMResponseLgth = 0x00;
				
				SW1 = 0x6C;   //期望获取的数据长度过长,返回响应状态字	
				SW2 = G_CurrentReponse[G_ChannelNum];	
			}
			
			result = ExtendedRFMUpdateResponseTLV(TAG_R_APDU);
			
			if(result != TRUE)
    		{
    			return NoMemory;
    		}
		
			if(SW1 == 0x6C)
			{
				ExtendedRFMUpdateResponseTLV(TAG_NumberCommandTLV);
    		
    			return  CipheringError;  //因标准未详细规定其余指令执行失败的POR响应值，因此暂以CipheringError表示失败信息
 
			}
			else
			{
				return TRUE;
			}
    	}
    	else   //指令执行失败
    	{
    		G_RFMResponseLgth = 0x00;
    		
    		result = ExtendedRFMUpdateResponseTLV(TAG_R_APDU);
    		
    		if(result != TRUE)
    		{
    			return NoMemory;
    		}
    		
    		ExtendedRFMUpdateResponseTLV(TAG_NumberCommandTLV);
    		
    		return  CipheringError;  //因标准未详细规定其余指令执行失败的POR响应值，因此暂以CipheringError表示失败信息
    	}
    	
	
    } 
    else //其余指令  
    {
    	if(SW1 != 0x90)  //执行执行失败
    	{
    		G_RFMResponseLgth = 0x00;
    		
    		result = ExtendedRFMUpdateResponseTLV(TAG_R_APDU);
    		
    		if(result != TRUE)
    		{
    			return NoMemory;
    		}
    		
    		ExtendedRFMUpdateResponseTLV(TAG_NumberCommandTLV);
    		   		
    		return  CipheringError;   //其余指令执行错误，返回 CipheringError;  
    	}
    	
    	return TRUE;
    }	
}
/*
*@description:处理扩展型指令中的Script Chaining TLV objects
*
*@param      :WORD lgth,WORD offset
*
*@return     :TRUE/FALSE
*
*@history
*/
BYTE ExtendedRFMProcessScriptChaining(WORD lgth,WORD offset)
{
	BYTE idata scriptChainingValue;
	UWORD xdata fileAddr;
	
	if(lgth != 0x01)  
	{
		RecData[0] = 0x03; //Script Chaining Length error,Unable to process script chaining
		
		ExtendedRFMUpdateResponseTLV(TAG_ScriptChainingResponse);
		
		ExtendedRFMUpdateResponseTLV(TAG_NumberCommandTLV);
		
		return CipheringError;
	}
	
	fileAddr = SelectTFFile(DF_OTA,EF_ScriptChainingFile);
	
	ReadTFBuf(fileAddr, 0x00, (BYTE *)&ChainingValue,sizeof(ChainingValue));
	
	fileAddr = SelectTFFile(DF_OTA,LDFile_RelaxFile);//缓存解密后的文件 5E03
	
	ReadTFBuf(fileAddr, offset, (BYTE *)&scriptChainingValue, sizeof(BYTE));
	
	if((scriptChainingValue == 0x02)||(scriptChainingValue == 0x03))
	{
		if(((ChainingValue.firstScriptValue != 0x01)&&(ChainingValue.firstScriptValue != 0x11))||((ChainingValue.previousValue != 0x02)&&(ChainingValue.previousValue != 0x01)&&(ChainingValue.previousValue != 0x11)))
		{		
			RecData[0] = 0x01;    //No previous script
		
			ExtendedRFMUpdateResponseTLV(TAG_ScriptChainingResponse);
			
			ExtendedRFMUpdateResponseTLV(TAG_NumberCommandTLV);
			
			return CipheringError;
		}
	}
		
	if((scriptChainingValue == 0x01)||(scriptChainingValue == 0x11))  //first script chaining
	{
		memset((BYTE *)&ChainingValue, 0x00, sizeof(ChainingValue));  //清空结构体缓存
		
		ChainingValue.firstScriptValue = scriptChainingValue;
	}
	else
	{
	
		G_CurrentDfAddr[G_ChannelNum] = ChainingValue.dfAddr;  //获取当前filecontext
		
		G_CurrentEfAddr[G_ChannelNum] = ChainingValue.efAddr;
		
		G_CurrentDfName[G_ChannelNum] =  ChainingValue.dfName;
		
		G_CurrentEfName[G_ChannelNum] =  ChainingValue.efName;
		
		G_CurrentRecNum[G_ChannelNum] = ChainingValue.recNum;
		
		G_ChvStatus =  ChainingValue.pinStatus;
	}
	
	ChainingValue.previousValue = scriptChainingValue;
	
	G_CurrentChainingValueTag = TRUE;   //第一条指令为正确的script chaining
	
	return TRUE;
}
/*
*@description:执行扩展型指令中的ERROR ACTION TLV
*
*@param      :lgth ,offset
*
*@return     :
*
*@history
*/
BYTE ExtendedRFMProcessErrorAction(void)
{
	UWORD xdata fileAddr;

	if(G_ErrorActionOffsetLgth[0] == 0x00)  //no action shall be performed
	{
		return TRUE;
	}
	else if(G_ErrorActionOffsetLgth[0] == 0x01)  // referenced format,,卡片暂不支持文件EFRMA.不执行任何操作
	{
		return TRUE;
	}
	else
	{
		fileAddr = SelectTFFile(DF_OTA,LDFile_RelaxFile);	//缓存解密后的文件 5E03
		
		ReadTFBuf(fileAddr,G_ErrorActionOffsetLgth[1],&ResData[0],G_ErrorActionOffsetLgth[0]);  //获取主动式命令
		
		if(ResData[1] <= 0x7F)  //主动式命令的长度为1字节时
		{
			ReadTFBuf(fileAddr,G_ErrorActionOffsetLgth[1],&ResData[1],G_ErrorActionOffsetLgth[0]);  //改变下主动式命令的存储格式，设置成COS支持的fetch类型
	
		} 
	
		if((ResData[6] != 0x21)&&(ResData[6] != 0x20))  //支持的主动式命令即不是displaytext也不是playtone ,不执行任何动作
		{
			return FALSE;
		}
		
		SendProactiveCommand();  //发送主动式命令
		
	}
	return TRUE;
}
/*
*@description :  执行扩展型RFM中碰到的immediate Action TLV
* 
*@param       :  lgth,offset
*
*@return      : TRUE:继续执行后续指令/FALSE:退出,返回POR,若earlyResponse存在,卡片发送完POR 之后需复位
*
*@history     :
*/
BYTE ExtendedRFMProcessImmediateAction(WORD lgth ,WORD offset)
{
	UWORD xdata fileAddr;
	
	fileAddr = SelectTFFile(DF_OTA,LDFile_RelaxFile);  //缓存解密文件  5E03
		
	ReadTFBuf(fileAddr,offset,&ResData[0],lgth);  //获取immediate action TLV 中的V值
	
	if((lgth == 0x00) ||(lgth == 0x01)&&(ResData[0] <= 0x7F))  //当L为0时,无主动式命令执行 or 不支持Reference to a record in EFRMA
	{
		return TRUE;
	}
	
	if(lgth == 0x01)  //refernce format
	{
		if(ResData[0] == 0x81)       //Proactive session indication
		{
			if(G_CurrentChainingValueTag == FALSE)  //The first Command TLV in the script if there is no script chaining
			{
				if(G_ExtendedRFMComNUM != 0x01)  //该tag出现在这里是错误的情况
				{
					return ExtendedRFMProcessBadTLV(FLAG_TagNotFound);

				}
			}
			else                                    //The second Command TLV in the script if there is script chaining.
			{
				if(G_ExtendedRFMComNUM != 0x02)  //该tag出现在这里是错误的情况
				{
					return ExtendedRFMProcessBadTLV(FLAG_TagNotFound);
				}
			}	
			
			if(STKPerform == TRUE)  //暂时根据STKPerform判断当前是否正在处理主动式命令,处理不能suspend the script execution,的情况
			{
				RecData[0] = Value_SuspensionError;  //Suspension error
				
				ExtendedRFMUpdateResponseTLV(TAG_ImmediateActionResponse);
				
				ExtendedRFMUpdateResponseTLV(TAG_NumberCommandTLV);
				
				return CipheringError;
			}
			
			return  TRUE;  
		}
		else if(ResData[0] == 0x82)  //Early response,保存当前响应内容，继续执行后续指令
		{
			
			ExtendedRFMUpdateResponseTLV(TAG_NumberCommandTLV);
			
			G_EarlyResponseTag = TRUE;
				
			return TRUE;
		}
		else                          //RFU
		{
			return TRUE;
		}
	}
	else
	{
	
		if(ResData[1] <= 0x7F)  //主动式命令的长度为1字节
		{
			memmove(&ResData[1],&ResData[0],ResData[1]+2);
 		} 
		
		if((ResData[6] != 0x21)&&(ResData[6] != 0x20)&&(ResData[6] != 0x01))  //支持的主动式命令即不是displaytext也不是playtone REFRESH,不执行任何动作
		{
			return TRUE;
		}
		
		
		if(ResData[6] == 0x01)   //主动式指令为refresh指令
		{
			if(G_EarlyResponseTag == TRUE)   //若之前有碰到early response 指令，则先发送response响应，再复位
			{
				G_EarlyResponseTag = 0x11;
				
				return TRUE;
			}
		}
		
		
		SendProactiveCommand();  //发送主动式命令
	}
	
	return TRUE;
}
/*
*@description:处理当前bad format TLV情况下更新responseFile的情况
*
*@param      :errorCode
*
*@return     
*
*@history
*/
BYTE ExtendedRFMProcessBadTLV(BYTE errorCode)
{
	BYTE idata result;
	
	RecData[0] = errorCode;
	
	result = ExtendedRFMUpdateResponseTLV(TAG_BadFormat);
			
	
	if(result == TRUE)   //解密数据错误
	{
		ExtendedRFMUpdateResponseTLV(TAG_NumberCommandTLV);
	
		return CipheringError;
	}
	else   //空间不足的情况，在主函数里统一更新当前指令执行条数
	{
		return NoMemory;
	}
}
/*
*@description:该函数主要用于处理extended RFM,数据已缓存在2700/LDFile_RelaxFile中，该文件的结构为L(2字节)+SPI(2字节)+KIC(1字节)+KID(1字节 )+TAR(3字节)+COUNTER(5字节)+ V
*	
*
*@param      :
*
*@return
*
*@history   :create by xuyz @20121106		 
*/
BYTE ExtendedRFMExcuteCommand(void)
{
	BYTE idata tag,result;
	WORD xdata valuelgth,offset,cpLgth;
	BYTE  xdata firstCpl[0x03];
	UWORD xdata fileAddr;
	
	
	//Todo :user may need back-up the current ef's(df's) information ,channel_mode
	
	BYTE idata RFMCurrentRecord,RFMCommandMode,RFMSFI,RFMChannel;
	WORD xdata RFMDFName,RFMEFName,RFMADFName;
	UWORD xdata RFMDFAddr,RFMEFAddr;
	
	RFMChannel = G_ChannelNum;
	
	RFMDFAddr = G_CurrentDfAddr[G_ChannelNum];                        //a data copy

  	RFMEFAddr = G_CurrentEfAddr[G_ChannelNum];

  	RFMDFName = G_CurrentDfName[G_ChannelNum];

  	RFMEFName = G_CurrentEfName[G_ChannelNum];
		
	RFMADFName  = G_CurrentAdfName[G_ChannelNum];
		 
	RFMSFI = G_CurrentSfi;

  	RFMCurrentRecord = G_CurrentRecNum[G_ChannelNum];

	RFMCommandMode = G_CommandMode;
	

	fileAddr = SelectTFFile(DF_OTA,LDFile_RelaxFile);   //解密缓存文件5E03
	
	if(fileAddr == FALSE)
	{
		return CipheringError;
	}
	
	ExtendedRFMUpdateResponseTLV(TAG_ClearResponseFile);
	
	memset(G_ErrorActionOffsetLgth, 0x00, sizeof(G_ErrorActionOffsetLgth));   //清空当前ErrorAction的标志字节
	
	G_CurrentChainingValueTag = FALSE;// 当前是否存在 script chaining 
	
	G_EarlyResponseTag = FALSE;   //是否存在early response,若存在,则遇到该tag后,则退出,POR发送成功后,卡片复位
	
	G_ExtendedRFMComNUM = 0x00;
	
	SW1 = 0x90;

	offset = G_CurrentCommandOffset;    //数据开始的偏移
	
	ReadTFBuf(fileAddr, offset-0x07, &G_CompactExtendedTag,sizeof(BYTE));  //获取当前tar值TAR_3
	
	if ((G_CompactExtendedTag == TAR3_SHARE_FILE)		//共享文件
		||(G_CompactExtendedTag == TAR3_SIM))
	{
		G_CurrentDfName[G_ChannelNum] = DF_MF;  

		G_CurrentDfAddr[G_ChannelNum] = E_FILE_START;		
	}
	else if (G_CompactExtendedTag == TAR3_CSIM_ADF)
	{
		G_CurrentDfName[G_ChannelNum] = G_DF_ID_CSIM;

		G_CurrentDfAddr[G_ChannelNum] = SelectFile(E_FILE_START,G_DF_ID_CSIM);
		
		G_CurrentAdfName[G_ChannelNum] = G_DF_ID_CSIM;
	}
	else if (G_CompactExtendedTag == TAR3_USIM_ADF)
	{
		G_CurrentDfName[G_ChannelNum] = G_DF_ID_USIM;

		G_CurrentDfAddr[G_ChannelNum] = SelectFile(E_FILE_START,G_DF_ID_USIM);
		
		G_CurrentAdfName[G_ChannelNum] = G_DF_ID_USIM;
	}
	else  //暂时不支持ISIM
	{
		return CipheringError;
	}

	ReadTFBuf(fileAddr, G_CurrentCommandOffset-12, (BYTE *)&G_PORDeliverSubmit, 0x01);	//读取SPI第二个字节

	if(((G_PORDeliverSubmit & 0x20)==0x00)||(G_PORTag == 0x03))               //SMS-Deliver  上行只取一条数据，不级联C0取数据标志
	{
	 	G_PORDeliverSubmit = 0x88;
	}

	G_CurrentEfName[G_ChannelNum] = FALSE;

	G_CurrentEfAddr[G_ChannelNum] = FALSE;
		
//	offset = 0x0E;   //	Command Scripting template tag offset
		
	SetBit(G_SystemStatus,B_RfmPerform);
	
	G_CommandMode = Command_Mode_First;
	
	
	ReadTFBuf(fileAddr,0x00,(BYTE *)&valuelgth,sizeof(WORD));  
	
	ReadTFBuf(fileAddr, offset, &tag ,sizeof(BYTE));
		
	if(tag != TAG_CommandScriptingTemplateDefinite)   //TAG ERROR
	{
		return CipheringError;
	}
	
	ReadTFBuf(fileAddr, offset+1, (BYTE *)firstCpl, sizeof(firstCpl));	//获取长度,按照BER-TLV格式编码

	if(firstCpl[0] == 0x82)
	{
		 cpLgth = (firstCpl[1]<<0x08)| firstCpl[2] ;
		 
		 offset += 4;
	}
	else if(firstCpl[0] == 0x81)
	{
		 cpLgth = firstCpl[1];
		 
		 offset += 3;
	}
	else
	{
		cpLgth = firstCpl[0];  //the command Lgth
		
		offset += 2;    //point to the Comand
		
	}
	
	if(cpLgth == 0x00)  //有效数据长度为0
	{
		return PoROK;
	}	
	result = FALSE;
	
	while((cpLgth > 0)&&((SW1 == 0x90)||(SW1 == 0x9F) ||(SW1 == 0x61)))
	{
	
		G_ExtendedRFMComNUM ++;  //The Number of executed C-APDUs shall take into account the incorrectly formatted TLV.
			
		ReadTFBuf(fileAddr, offset, &tag ,sizeof(BYTE));    //ponit to he  tag 
		
		ReadTFBuf(fileAddr, offset+1, (BYTE *)firstCpl, sizeof(firstCpl));	//获取长度,按照BER-TLV格式编码
		
		if(firstCpl[0] > 0x82)  //03 ,lgth not found
		{
			result =  ExtendedRFMProcessBadTLV(FLAG_LgthNotFound);;
			
			break;
		}

		if(firstCpl[0] == 0x82) //lgth error
		{
			 valuelgth = (firstCpl[1]<<0x08)| firstCpl[2] ;
			 
			 if(valuelgth <= 0xFF)
			 {
			 	result = ExtendedRFMProcessBadTLV(FLAG_LgthNotFound);
			
				break;
			 }
			 
			 offset += 4;
			 
			 firstCpl[0] = 0x02;
		}
		else if(firstCpl[0] == 0x81) //lgth error
		{
			 valuelgth = firstCpl[1];
			 
			 if(valuelgth <= 0x7F)
			 {
			 	result = ExtendedRFMProcessBadTLV(FLAG_LgthNotFound);
			
				break;
			 }
			 
			 offset += 3;
			 
			 firstCpl[0] = 0x01;
		}
		else
		{
			valuelgth = firstCpl[0];
			
			 if(valuelgth > 0x7F)  //lgth error
			 {
			 	result =  ExtendedRFMProcessBadTLV(FLAG_LgthNotFound);
			
				break;
			 }  
	
			offset += 2; 
			
			firstCpl[0] = 0x00;  //后面会用到这个值计算当前数据长度
		}
		
		
		if(valuelgth > (cpLgth-2))  // length > length of BER-TLV ,bad format TLV(03: length not found)
		{
			result =  ExtendedRFMProcessBadTLV(FLAG_LgthNotFound);
			
			break;
		}
		
		switch(tag)
		{
			case TAG_C_APDU:
			
				if(valuelgth < 5)   //length error ,(02:wrong length found)
				{	
					result =  ExtendedRFMProcessBadTLV(FLAG_WrongLgth);
			
					break;
				}
				
				result = ExtendedRFMProcessCAPDU(valuelgth,offset);//true Or other error
				
				break;
				
			case TAG_ImmediateAction:
			
				if((valuelgth > 0x01)&&(valuelgth <11))  //主动式命令的长度最短必须为11字节  ,否则长度错误
				{
					result = ExtendedRFMProcessBadTLV(FLAG_WrongLgth);
			
					break;
				}
			
				result = ExtendedRFMProcessImmediateAction(valuelgth,offset); //true Or ciphererror
				
				break;
				
			case TAG_ErrorAction:
			
				if((valuelgth > 0x01)&&(valuelgth <11))  //主动式命令的长度最短必须为11字节  ,否则长度错误
				{
					result = ExtendedRFMProcessBadTLV(FLAG_WrongLgth);
			
					break;
				}
			
				G_ErrorActionOffsetLgth[0] = valuelgth;   //保存当前 ErrorAction TLV 的长度
				
				G_ErrorActionOffsetLgth[1] = offset;     //保存当前ErrorAction TLV 数据对象的V的起始偏移
				
				result = TRUE;
				
				break;
				
			case TAG_ScriptChaining:
			
				if(G_ExtendedRFMComNUM != 0x01)  //chaining tag invalid;If present, the Script Chaining TLV shall be present only once and shall be the first Command TLV in the Command Script
				{
					result =  ExtendedRFMProcessBadTLV(FLAG_TagNotFound);
			
					break;
				}
				
				result = ExtendedRFMProcessScriptChaining(valuelgth,offset);
				
				
				break;
				
			default:    //Bad format TLV(01: unknown tag found)
			{
		
				result =  ExtendedRFMProcessBadTLV(FLAG_TagNotFound);
				
				break;
			}
				
		}
		
		if(G_EarlyResponseTag == 0x11)  //有early response且碰到了refresh指令
		{
			break;
		}
		
		if((SW1 != 0x90)&&(SW1 != 0x9F)&&(SW1 != 0x61))  //指令执行错误,判断the start of the script and the C-APDU resulting in an error之前是否存在error action TLV
		{
			if(G_ErrorActionOffsetLgth[1] < offset)  //between the start of the script and the C-APDU resulting in an error
			{
				ExtendedRFMProcessErrorAction();
			}
		}
			
		if(result !=  TRUE)
		{
			break;
		}
		
		cpLgth = cpLgth - (firstCpl[0]+2+valuelgth);  //剩余数据长度
		
		if((tag == TAG_C_APDU)&&(cpLgth == 0x00))  //上一条C-APDU指令是最后一条C-APDU指令,不论是否存在le字节,均拼装R-APDU
		{
			ReadTFBuf(fileAddr, offset, &G_APDU[0],0x05);   //获取5个字节的命令头 CLA INS P1 P2 P3 
			
			if((SW1 == 0x90)||(SW1 == 0x9F)||(SW1 == 0x61))  //最后一条指令执行成功的情况，指令若执行失败，已在ExtendedRFMProcessCAPDU更新了response
			{
				if((INS != 0xB0)&&(INS != 0xB2))
				{
					if(((P3 + 5) == valuelgth)&&((INS == 0x32)||(INS == 0xA2)||(INS == 0xA4))) //对于seek select,increase,不存在LE字节
					{
						G_RFMResponseLgth = 0x00;  
	    		
	    				result = ExtendedRFMUpdateResponseTLV(TAG_R_APDU);		
					}
					else if((INS != 0x32)&&(INS != 0xA2)&&(INS != 0xA4))  //其余最后一条指令
					{
						G_RFMResponseLgth = 0x00;  
	    		
	    				result = ExtendedRFMUpdateResponseTLV(TAG_R_APDU);	
					}
				}
			}

			 break;
		}
		
		offset += valuelgth;  //指向下一个tag
	}
	
	if(result == NoMemory)  //当response buffer缓存不足时，
	{
		ExtendedRFMUpdateResponseTLV(TAG_NumberCommandTLV);  //更新指令条数tag
	}
	
	if(result == TRUE)
	{
		ExtendedRFMUpdateResponseTLV(TAG_NumberCommandTLV);  //更新指令条数tag
		
		//若当前指令为script chaining,指令执行成功后，则保存当前的chainging infomration
			
		if(G_CurrentChainingValueTag == TRUE)
		{
			fileAddr = SelectTFFile(DF_OTA,EF_ScriptChainingFile);
			
			if(ChainingValue.previousValue == 0x03)
			{
				SetTFConst(fileAddr, 0x00,0x00,sizeof(ChainingValue));  //当前script chaining为最后一条
			}
			else
			{
				ChainingValue.dfAddr = G_CurrentDfAddr[G_ChannelNum];  

				ChainingValue.efAddr  = G_CurrentEfAddr[G_ChannelNum];

				ChainingValue.dfName = G_CurrentDfName[G_ChannelNum];

				ChainingValue.efName = G_CurrentEfName[G_ChannelNum];

				ChainingValue.recNum = G_CurrentRecNum[G_ChannelNum];

				ChainingValue.pinStatus = G_ChvStatus;
				
				WriteTFBuf(fileAddr, 0x00, (BYTE *)&ChainingValue,sizeof(ChainingValue));
			}
		}
	}

	
	//Todo:here ,user may resume the ef's(df's) information 
	
	ClearBit(G_SystemStatus,B_RfmPerform);
	
	G_ChannelNum = RFMChannel;
	
	G_CurrentDfAddr[G_ChannelNum] = RFMDFAddr;                        

  	G_CurrentEfAddr[G_ChannelNum] = RFMEFAddr;

    G_CurrentDfName[G_ChannelNum]  = RFMDFName;

  	G_CurrentEfName[G_ChannelNum] = RFMEFName;

	G_CurrentAdfName[G_ChannelNum] = RFMADFName;
		 
	G_CurrentSfi = RFMSFI;

  	G_CurrentRecNum[G_ChannelNum] = RFMCurrentRecord;

	G_CommandMode = RFMCommandMode;
	
	if(result == TRUE)
	{	
		return PoROK;
	}
	else
	{
		return result;
	}
}
#endif
