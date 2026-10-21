/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件

 			V1.1 修改函数ReadSmsData，在读取短信时，如果短信内容为空时返回FALSE		@2012/1/4

			V1.2 根据中国移动M2M专网写号功能增加IF_STKTerminalResponse();  zdm  2014/9/30

 */ 
#include  "typedef.h"
#include  "uicc_main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include "uicc_error.h"
#include  "arithmatic.h"
#include  "stkhead.h"
#include  "ceph.h"
#include  "interface.h"

//WORD    xdata G_VARIABLECONTROL;

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 SendProactiveCommand

  input parameter:   void

  output parameter:  TRUE/FALSE

  description:       Send the data of proactive command and to pasre the terminal response

  		             for returnnig the general result

  Code status:		OK

  test status:

 -------------------------------------------------------------------------------------------------------------------*/
static code INSLIST CephInsList[]=
{ 
//SIM and USIM command
	{0xA4,SelectCommand},
	{0xD6,UpdateBinary},
	{0xB2,ReadRecord},
	{0xDC,UpdateRecord},
  	{0xB0,ReadBinary},
  	{0x04,FileStatusHandler},
  	{0x44,FileStatusHandler},
  	{0x20,VerifyChangeUnblockChv},
  	{0x24,VerifyChangeUnblockChv},
  	{0x26,ChvHandle},
  	{0x28,ChvHandle},
  	{0x2C,VerifyChangeUnblockChv},	
  	{0xF2,STKStatusCommand},
  	{0xC0,GetResponseCommand},
   	{0xC2,EnvelopeCommandSTK},
  	{0x12,FetchCommand},
   	{0xA2,SeekCommand},
  	{0x32,IncreaseCommand},
	{0x10,STKTerminalProfile},
	{0x70,UsimManageChannel},
	{EOL,IF_ReceiveAPDU},
};

BYTE SendProactiveCommand(void)
{
	BYTE idata i,num;

	STKPerform = TRUE ;

	G_STKBusy = TRUE;					//产生了主动式命令

	CommandFinish(0x00);														//Send 91 XX

	SendResponse();

	for(;;)
	{
#ifdef ST33G384			
 	
		while(1)
		{
			SleepModeMain();					
			
			if(G_FirstCommand == 0x00)				//只有复位后第一条指令才做PPS请求判断
			{
				if(G_IsoRxCmd == G_ISOSTATUS_PPS)	  
				{
					PpsProcess();

					T0_ReadyReceiveCommand();		
				}		
			}
			
			if( G_IsoRxCmd == G_ISOSTATUS_APDU )	  
			{
				memcpy(G_APDU,(BYTE *)G_CTAPDU,5);
				break;
			}
		}
	
#else	
	
		
#endif

		G_ChannelNum = CLA&0x03;

		if(INS == 0x14)
		{
			if(P1P2P3Check(0x00,TRUE) ==FALSE)  
			{
				SendResponse();

				continue;
			}
				
			ReceiveData();

			G_STKBusy = FALSE;											//已收到“14”响应，G_STKBusy返回0x00
		
			if(STKPerform == TRUE)		 	
			{
				CommandFinish(0x00);														//Send 91 XX
				SendResponse();
				continue;
			}

			if(STKPerform == 0x02)		 	
			{
				STKPerform = FALSE;
			}

			IF_STKTerminalResponse();

			if( ResData[6] == RecData[3] )										//compare the response of command que the command send the last before
			{
				num = GetResponseResult();

				return num;
			}
			else
			{
				return ExitStkPerform;											//exit application
			}
		}

		CommandAttemper(CephInsList);

		continue;                        						
	}
	
//	return ExitStkPerform;	
}
/*-------------------------------------------------------------------------------------------------------//

  Function name:	InitProactiveCommand
 
  Input parameter:	
 					com_type:  type of command 				        	see 11.14 page 89

 					com_qualifier: qualifiers specific to the command  	see 11.14 page 89

 					des_device:   destination device identity          	see 11.14 page 92
 
 Output parameter:  void
 
 Description:       init the head of stk_command in the stk buffer:command details ,command device identities

 Status:			OK

 -------------------------------------------------------------------------------------------------------------------*/

void InitProactiveCommand(BYTE commandType,BYTE commandQlify,BYTE desDevice)
{
	ResData[0] = 0xD0;

	ResData[1] = 0xD0;

	ResData[2] = 0x09;

	ResData[3] = 0x81;

	ResData[4] = 0x03;

	ResData[5] = 0x01;

	ResData[6] = commandType;

	ResData[7] = commandQlify;

	ResData[8] = 0x82;

	ResData[9] = 0x02;

	ResData[10]= 0x81;

	ResData[11]= desDevice; 
}


/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	 AddTLV                                                                                                                   
                                                                                                                                                
  input parameter:   TLV_AIM:     be added buffer :the buffer must tlv structure 					        see 11.14 page 44

                     TLV_OBJECT:  want to add object :must the tlv structure                                                                    
  					                                                                                                         
  output parameter:  TRUE/FALSE                                                                                                                 
                                                                                                                                                
  description:       This function is to add the tlv structure to the buffer of have data :the buffer must have data of thv structure
  
  Status:			OK but need to add function       
---------------------------------------------------------------------------------------------------------------------------------------------------*/
     
BYTE AddTLV(BYTE * TLV_AIM,BYTE  * TLV_OBJECT)                                                                                                      
{                                                                                                                                                
	BYTE idata num,addLgth;     							                                                                                                                        
    WORD idata length;                                                                                                                        
                                                                                         
   	num = TLV_OBJECT[1] + 0x02;
    		
    length = TLV_AIM[1] + num;                                                                                                                                  

	if( length > MAX_LENGTH )
	{                                                           //CHECK LENGTH IS OVER FLOWER                               
		return  FALSE ;                                                                                                                              
 	}
    
    if(TLV_OBJECT[1] >= 0x80)
    {
		TLV_AIM[ TLV_AIM[1] + 0x02] = TLV_OBJECT[0] ;

		TLV_AIM[ TLV_AIM[1] + 0x03] = 0x81;

		addLgth = 1 ;

		memcpy_G(&TLV_AIM[ TLV_AIM[1] + 0x04],&TLV_OBJECT[1],num-1);
  
    }
    else
    {
        memcpy_G(&TLV_AIM[ TLV_AIM[1] + 0x02],&TLV_OBJECT[0],num);

		addLgth = 0x00 ;
    }                                                                                   
                                                                                                                                                 
	TLV_AIM[1] = TLV_AIM[1] + num +addLgth;                                                 //LENGTH ADD LENGTH                                         
                                                                                                                                                 
	return TRUE;
	
}

/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	 AddValue                                                                                                                   
                                                                                                                                                
  input parameter:   TLV_AIM:   be added buffer :the buffer must tlv structure 					        see 11.14 page 44

                     LV_OBJECT:  want to add object :must the tlv structure                                                                    
  					                                                                                                         
  output parameter:  TRUE/FALSE                                                                                                                 
                                                                                                                                                
  description:       This function is to add the data structure to lv structure
  
  Status:			 OK but need to add function      
---------------------------------------------------------------------------------------------------------------------------------------------------*/
     
BYTE AddValue(BYTE * LV_AIM,BYTE * LV_OBJECT)                                                                                                      
{                                                                                                                                                                                                                                                                                
    WORD idata length;
                                                                        
	if(LV_OBJECT[0] == 0x00)
	{
		return TRUE ;
	}

	length = LV_AIM[0] +  LV_OBJECT[0];                                                                                                                                  

	if( length > MAX_LENGTH )
	{                                                           //CHECK LENGTH IS OVER FLOWER                               
		return  FALSE ;                                                                                                                              
 	}

     memcpy_G(&LV_AIM[ LV_AIM[0] + 0x01], &LV_OBJECT[1], LV_OBJECT[0]);
                                                                                                                 
	 LV_AIM[0] = length;	                                     //LENGTH ADD LENGTH         
                                                                                                                                                 
	 return TRUE;
}






/*---------------------------------------------------------------------------------------//                                                                                                                                              
  Function name :	IncreaseOTPCounter
  
  description:		将计数器值加1	                                                                                                               
                                                                                                                                                
  input parameter:	* cntr		计数器值	
					counterLen	计数器长度
  					                                                             					                                                                                                         
  return:			void

 example:			none
  
  author:			                                                                                                                   
                                                                                                                                                       
  Status:								      
-----------------------------------------------------------------------------------------*/

void IncreaseCounter(BYTE *cntr,BYTE counterLen) 						
{
	BYTE idata i,offset;
	WORD idata r;

	offset = counterLen-1;

	r = 0x0001;

	for(i=0x00;i<counterLen;i++)
	{
		r = r + cntr[offset-i];

		cntr[offset-i] = r&0xFF;

		r = (r>>0x08)&0x0001;
	}
}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 LoadSCAddress
  
  input parameter:   BYTE *
   
  output parameter:	 TRUE/FALSE
                                                                                                                                                
  description:       Get OTA2 Service Center address
  
   tester:           caibo
  
   status:			 OK

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE LoadSCAddress(BYTE * buf)
{
	UWORD idata fileAddr;
	BYTE  idata RecLgth;

	fileAddr = SelectLFFile(DF_TELECOM, EF_SMSP);

	RecLgth = GetLFRecNumLgth(fileAddr,FileLgth);

	ReadLFBuf(fileAddr,1, RecLgth - 0x0F, buf, 0x0C);

	if ((buf[0] == 0x00)||(buf[0] == 0xFF)||(buf[0]>11))
	{
		buf[0] = 0x00;
		
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetResponseResult
  
  input parameter:   void
   
  output parameter:	 BYTE, 0 = FAIL otherwise reponse result

  description:       Get Response Result 
  					 
  
  status:			Code ok! test ok!			     

 -------------------------------------------------------------------------------------------------------------------*/    

BYTE GetResponseResult(void)
{
	BYTE idata pos;

	pos = FindTag(&RecData[0],TLV_RESULT,P3);

	if (pos == FALSE)
	{
		return ExitStkPerform;
	}
	else
	{
		if(RecData[pos+1] < 0x0F)
		{
			return CMD_Success;
		}
		else if(RecData[pos+1] == 0x11)
		{
			return BackMove;
		}
		else if(RecData[pos+1] == 0x10)
		{
			return ExitStkPerform;
		}
		else
		{
			return (RecData[pos+1]);
		}
	}
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetResponseItemIdentifier
  
  input parameter:   void
   
  output parameter:	 0 = FAIL, otherwise user selected item identifier

  description:       Get Response Item Identifier
  					 
  
  status:			     

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE GetResponseItemIdentifier(void)
{
	BYTE idata pos,ch;

	if (INS == 0xC2)
	{
		if(RecData[1] == 0x81)
		{
			ch = 0x01;
		}
		else
		{
			ch = 0x00;
		}

		pos = FindTag(&RecData[2+ch],TLV_ITEMIDENTIFIER,RecData[1+ch]);

		if( pos != FALSE)
		{
			return RecData[pos+3+ch];
		}
	}
	else
	{
		pos = FindTag(&RecData[0],TLV_ITEMIDENTIFIER,P3);

		if(pos != FALSE)
		{
			return RecData[pos+1];
		}
	}

	return FALSE;

}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       
  Function name:	LocateRecordInFile
  
  input parameter:	fileAddr:	EF文件的地址
					tag:		寻找的TAG

  output parameter:	成功：记录号
					失败：FALSE
                                                                                                                                                
  description:		在TV结构的记录文件中，根据tag定位记录号

  Test:				Caibo
  
  status:			OK			
 -------------------------------------------------------------------------------------------------------------------*/
BYTE LocateRecordInFile(UWORD fileAddr, BYTE tag)
{
	BYTE idata i, tmp;
	BYTE idata recNum;

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	for(i = 0x00; i < recNum; i ++)
	{
		ReadLFBuf(fileAddr, i+1, 0, &tmp, 1);

		if(tmp == tag)
		{
			return (i+1);
		}
	}

	return FALSE;
}



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ReadSmsData
  
  input parameter:   dataBuf: save the sms data

					 timeTag: the variable tag saved the sms time(NOT 0xFF)

				     numTag: the variable tag that saved the TPOA or TPDA(NOT 0xFF)

				     dataTag: the variable tag that save the user data(with the dcs is only 04 or 08 )(NOT 0xFF)

  output parameter:  FALSE/TRUE
                                                                                                                                                
  description:       get the sms data

  test:			     XYZ
  
  status:			 			

 -------------------------------------------------------------------------------------------------------------------*/    

BYTE ReadSmsData(BYTE * dataBuf, BYTE timeTag, BYTE numTag, BYTE dataTag)
{
	BYTE idata len, pos, mti, dcs, udhl, tmp, smsType;

	if((dataBuf[0]&0x01) == 0)    									//dataBuf[0] = 0x01 or 0x03 or 0x05 or 0x7
	{
		return FALSE;
	}

	dataBuf[0] = dataBuf[0] & 0x07;

	pos = dataBuf[1];					

	mti = dataBuf[pos + 2];

	if((dataBuf[0]==0x01)||(dataBuf[0]==0x03))						//接收短信
	{
		smsType = 0x00;
	}
	else		   													//发送短信
	{
		smsType = 0x01;
	}
	
	len = (dataBuf[pos+3+smsType]>>1) + (dataBuf[pos+3+smsType]&0x01) + 1;  //TPO(D)A长度len = (dataBuf[pos+3]/2) + (dataBuf[pos+3]%2) + 1;

	dataBuf[pos+3+smsType] = len;

	if (numTag != 0xFF)
	{
		WriteVariable(numTag, &dataBuf[pos+3+smsType]);						//write the TP_OA
	}

	pos = pos + len + 0x05+smsType;										//point to the dcs
				
	dcs = dataBuf[pos];  												//dcs

	if (smsType == 0x00)
	{
		dataBuf[pos] = 0x06;                							//TP_SCTS长度(本地OTA测试规范中，TP_SCTS不需要保存第7byte，即timezone)
	}
	else                                        
	{
		if(((mti&0x18) == 0x18)||((mti&0x18) == 0x08))
		{
			len = 0x07 ;
		}
		else if((mti & 0x18) == 0x10)
		{
			len = 0x01 ;
		}
		else
		{
			len = 0x00 ;					
		}

		dataBuf[pos] = len;											  //TP_SCTS长度
	}

	for(len=1; len<=dataBuf[pos]; len++)  
	{
		tmp = ((dataBuf[pos+len]<<4)&0xF0)+((dataBuf[pos+len]>>4)&0x0F);

		dataBuf[pos+len] = tmp; 
	}                                   

	if (timeTag != 0xFF)
	{
		WriteVariable(timeTag, &dataBuf[pos]);						//write the 6 bytes TP_scts
	}

	if(smsType == 0x00)
	{
		pos += 0x08;												//point to the UDL
	}
	else
	{
		pos = pos + dataBuf[pos] + 0x01;							//point to the UDL
	}

	len = dataBuf[pos];

	if((len == 0xFF)||(len == 0x00))
	{
		ClearVariable(dataTag);

		return FALSE; 
	}

	if((mti&0x40) == 0x40)
	{
		udhl = dataBuf[pos+1];

		pos = pos + udhl + 2;

		len = len - udhl - 1;
	}
	else
	{
		udhl = 0x00;

		pos++;
	}

	dataBuf[1] = (dcs&0x0F);                //dcs

	if((dcs&0xC0) == 0x00)
	{
		if((dataBuf[1]&0x04) == 0x04)
		{
			dataBuf[1] = 0x04;
		}
		else if((dataBuf[1]&0x08) == 0x08)
		{
			dataBuf[1] = 0x08;
		}
		else
		{
			dataBuf[1] = dcs;            	//dcs
		}
	}
	else if((dcs&0xF0) == 0xF0)
	{
		if((dataBuf[1]&0x02) == 0x00)
		{
			dataBuf[1] = 0x00;
		}
		else
		{
			dataBuf[1] = 0x04;
		}		
	}
	else
	{
		return FALSE;
	}

	if((dataBuf[1] == 0x00) && (udhl != 0x00))	//7BIT编码  //len = len - [(udhl +1+fillbits)*8]/7 + fillbits   得到8bit字符个数
	{
		len = len - ( ( udhl + 6 ) / 7 );
	
		dcs = ( (udhl+1) * 8 ) % 7; 		// ( a * 8 + x ) % 7 == 0 

		dcs =  ( 7 - dcs ) % 7;
	}
	else
	{
		dcs = 0;
	}

	if((dcs != 0x00)||(dataBuf[1] == 0x00))
	{		
		CharacterUnPack(&dataBuf[pos], &dataBuf[1], len, dcs);
		
		dataBuf[1] = 0x04;		
	}
	else
	{
		memcpy(&dataBuf[2],&dataBuf[pos],len);
	}

	dataBuf[0] = len + 1;

	WriteVariable(dataTag, dataBuf);

	return TRUE;
}


/*@Function:   ClearFileFirstWord                                            
 * 
 *@Descrip:    
 *
 *@See:	       
 *                                                                                                                                     
 *@Param:      dfName    
 *@Param:      efName    
 *              						                                                                                                  
 *@Return:     BYTE                                                                                                                                                                                                                                                              
 * 
 *@Author:     xwh
 *  
 *@History:    V0.0---Create by xwh 20180801
 *                        
 */
void ClearFileFirstWord(WORD dfName,WORD efNAme)
{
	UWORD  xdata fileAddr;

	fileAddr = SelectPrivateTFFile(dfName, efNAme);		

	if(fileAddr != FALSE)
	{
		SetTFConst(fileAddr,0x00,0x00, 0x02);;
	}
}

/**
 * @brief 		取短信内容，存入VARIABLRBUFFER,对于空短信不操作
 * 
 * @param 		dataBuf : save the sms data
 * @param 		dataTag : the variable tag that save the user data(with the dcs is only 04 or 08 )
 * 
 * @Returns   	FALSE/TRUE
 * 
 * @author 		zxl
 * 
 * @history 	add comment by zxl, @071224 13:59
 */

BYTE GetSmsData(BYTE * dataBuf, BYTE dataTag)
{
	BYTE idata len, pos, mti, dcs, udhl;

	if((dataBuf[0]&0x01) == 0)
	{
		return FALSE;
	}

	dataBuf[0] = dataBuf[0] & 0x07;

	pos = dataBuf[1];					

	mti = dataBuf[pos + 2];

	if((dataBuf[0]==0x01)||(dataBuf[0]==0x03))			//DELIVER SMS
	{
		len = (dataBuf[pos+3]/2) + (dataBuf[pos+3]&0x01) + 1;

		pos = pos + len + 0x05;
					
		dcs = dataBuf[pos];  					//dcs

		pos = pos + 0x08;						//point to the UDL
	}
	else //if((dataBuf[0] == 0x05)||(dataBuf[0] == 0x07))		//SUBMIT SMS
	{
		len = (dataBuf[pos+4]/2) + (dataBuf[pos+4]&0x01) + 1;

		pos = pos + len + 0x06;

		dcs = dataBuf[pos];

		if(((mti&0x18) == 0x18)||((mti&0x18) == 0x08))
		{
			len = 0x07 ;
		}
		else if((mti & 0x18) == 0x10)
		{
			len = 0x01 ;
		}
		else
		{
			len = 0x00 ;					
		}
	
		pos = pos + len + 0x01;					//point to the UDL
	}

 
	len = dataBuf[pos];

	if((len == 0xFF)||(len == 0x00))
	{
		return FALSE;
	}

	if((mti&0x40) == 0x40)
	{
		udhl = dataBuf[pos+1];

		pos = pos + udhl + 2;					//2007-12-03

		len = len - udhl - 1;
	}
	else
	{
		udhl = 0x00;

		pos++;
	}

	if((dcs&0xC0) == 0x00)
	{
		dcs &= 0x0F;

		if((dcs&0x04) == 0x04)
		{
			dataBuf[1] = 0x04;
		}
		else if((dcs&0x08) == 0x08)
		{
			dataBuf[1] = 0x08;
		}
		else
		{
			dataBuf[1] = dcs;            //dcs
		}
	}
	else if((dataBuf[1] & 0xF0) == 0xF0)
	{
		dataBuf[1] = dcs&0x0F;                //dcs

		if((dataBuf[1]&0x02) == 0x00)
		{
			dataBuf[1] = 0x00;
		}
		else
		{
			dataBuf[1] = 0x04;
		}		
	}
	else
	{
		return FALSE;
	}

	if((dataBuf[1] == 0x00) && (udhl != 0x00))	//7BIT编码
	{
		len = len - ( ( udhl + 6 ) / 7 );
	
		dcs = ( udhl * 8 ) % 7; 		// ( a * 8 + x ) % 7 == 0 

		dcs =  ( 7 - dcs ) % 7;

	}
	else
	{
		dcs = 0;
	}

	if((dcs != 0x00)||(dataBuf[1] == 0x00))
	{		
		CharacterUnPack(&dataBuf[pos], &dataBuf[1], len, dcs);
		
		dataBuf[1] = 0x04;		
	}
	else
	{
		memcpy(&dataBuf[2],&dataBuf[pos],len);
	}

	dataBuf[0] = len + 1;

	WriteVariable(dataTag, dataBuf);

	return TRUE;
}




/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 GetADNAlphaLen                                                                                                                 
                                                                                                                                                
  Input parameter:   dataBuffer:    the start of paramter pointer,need to count the length
					 
					 maxLenth  :    the max length of input Alpha of ADN(6F3A) 
                                                  
  Output parameter:  the length of input Alpha of ADN(6F3A)                                                                                                                 
                                                                                                                                                
  Description:       Get the length of input Alpha of ADN(6F3A)
  
  status:            

 -------------------------------------------------------------------------------------------------------------------*/                   

BYTE GetADNAlphaLen(BYTE * dataBuffer,BYTE maxLength)
{
	BYTE idata len;
	BYTE idata h, c;

	switch(dataBuffer[0])
	{	
		case 0x81:							//first byte is 0x81(UCS2)
		case 0x82:							//first byte is 0x82(UCS2)
		{
			len = dataBuffer[1] * 2 + 1;

			return len;	
		}
		default:							//0x80   and   gsm default 
		{
			h = (dataBuffer[0] == 0x80)? 0x02 : 0x01;

			c = h/2;

			for (len = c; (len+c) < maxLength; len = len+h)
			{
				if((dataBuffer[len] == 0xFF)&&(dataBuffer[len+c] == 0xFF))
				{
					break;
				}
			}

			return len;	
		}
	}
}

/*-------------------------------------------------------------------------------------------------------

  Function name :	 TransPhoneAdnFormat

  Input parameter:  *input


  Output parameter:  Length

  Description:       将0x81,0x82编码转换为0x80编码格式

  Coder:			 ZXL

  Status:
-----------------------------------------------------------------------------------------------------------*/
BYTE TransPhoneAdnFormat(BYTE * input)
{
	BYTE idata NameLgth,i,j;
	WORD idata BasePointer;

	NameLgth = 2*input[1]+1;

	input[20] = 0x80;

	if(input[0] == 0x81)
	{
		BasePointer = ((WORD)input[2]<<7)&0x7F80;

		j = 3;
	}
	else
	{
		BasePointer = (((WORD)input[2]<<8)&0xFF00)|input[3];

		j = 4;
	}

	for(i=0;i<input[1];i++)
	{
		if((input[i+j]&0x80)==0x00)
		{
			input[20+i*2+1] = 0x00;

			input[20+i*2+2] = input[i+j];
		}
		else
		{
			input[i+j] = input[i+j]&0x7F;

			input[20+i*2+1] = ((BasePointer+input[i+j])>>8)&0xFF;

			input[20+i*2+2] = (BasePointer+input[i+j])&0xFF;
		}
	}

	memcpy((BYTE *)&input[0],(BYTE *)&input[20],NameLgth);

	return NameLgth;
}
/**
 * @brief 		Get OTA2 ADN alpha
 * 
 * @param 		adnAddr : adn File Address
 * @param 		recIndex ：record index of EF_6F3A
 * @param 		dataBuf : the result of the date(the space of the arrey equal to the length of record at least)

 * 
 * @Returns   	FALSE/TRUE
 * 
 * @author 		zxl 
 * 
 * @history 	add comment by zxl, @071224 13:59
 */
BYTE ReadADNname(UWORD adnAddr, BYTE recIndex,BYTE * dataBuf)
{
	BYTE  idata recLgth;
	
	recLgth = GetLFRecNumLgth(adnAddr,FileLgth);

	ReadLFBuf(adnAddr,recIndex,0x00,&dataBuf[1],recLgth);

	if(dataBuf[1] == 0xFF)
	{
		if((dataBuf[recLgth-13] == 0x00)||(dataBuf[recLgth-13] == 0xFF))
		{
			return FALSE;
		}

		ADNToAscii(&dataBuf[recLgth-13], &dataBuf[0]);
	}
	else                 //read the ADN'name
	{
		recLgth = recLgth - 14;

		dataBuf[0] = GetADNAlphaLen(&dataBuf[1], recLgth);

		if((dataBuf[1] == 0x81)||(dataBuf[1] == 0x82))
		{
			TransPhoneAdnFormat((BYTE *)&dataBuf[1]);
		}
	}

	return TRUE;
}

/**
 * @brief 		Get  SMS alpha from tmpbuf(LV)
 * 
 * @param 		buf : 存放短信内容的BUFFER
 * @param 		alphaLen : 短信内容的限制长度
 * 
 * @author 		zxl 
 * 
 * @history 	add comment by zxl, @071224 13:59
 */
static BYTE code Enshenlue[4] = {0x03,0x2E,0x2E,0x2E};
 
static BYTE code Chshenlue[7] = {0x06,0x00,0x2E,0x00,0x2E,0x00,0x2E};

void GetSMSAlpha(BYTE * buf,BYTE alphaLen)
{
	tempBuf[0] = ReadVarible(SmsTag, &tempBuf[1]);

	if(tempBuf[0] != FALSE)
	{		
		if((tempBuf[1] == 0x80) ||(tempBuf[1] == 0x08))
		{
			tempBuf[1] = 0x80;

			buf[0] = ((alphaLen +1) > tempBuf[0]) ? tempBuf[0] : (alphaLen +1);		//(displaylen/2)个UCS2字节+0x80 或者全部

			memcpy(&buf[1],&tempBuf[1],buf[0]);

			if (buf[0] != tempBuf[0])
			{
				memcpy_G(&buf[buf[0]+1], (BYTE CONST *)&Chshenlue[1],Chshenlue[0]);

				buf[0] = buf[0] + Chshenlue[0];
			}
		}
		else
		{			
			buf[0] = ((alphaLen +1) > tempBuf[0]) ? (tempBuf[0]-1) : alphaLen;		//displaylen个ASCII字节或者全部

			memcpy(&buf[1],&tempBuf[2],buf[0]);

			if (buf[0] != tempBuf[0]-1)
			{
				memcpy_G(&buf[buf[0]+1],(BYTE CONST *)&Enshenlue[1],Enshenlue[0]);

				buf[0] = buf[0] + Enshenlue[0];
			}
		}
	}
	else
	{
		buf[0] = 0x00;
	}
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 OTASortApplicationID
  
  input parameter:   list : LV format save the application id 's 	
  					 mode:	application id 的长度
   
  output parameter:	 无
                                                                                                                                                
  description:     	 sort application id in the list in ascendant format
  					 AID由小到大排序
  
  author:			D
  
  status:					      

 -------------------------------------------------------------------------------------------------------------------*/    

void SortApplicationID(BYTE * list, BYTE mode)
{
	BYTE idata i, j;
	BYTE xdata tmp[3];

	for (i=0; i<(list[0]-1); i++)
	{
		for(j=0; j<(list[0]-i-1); j++)
		{
			if(memcmp_p(&list[(j*mode) + 1], &list[(j+1) * mode + 1] , mode) > 0)
			{
				memcpy(tmp, &list[(j*mode)+1], mode);

				memmove(&list[(j*mode)+1],&list[(j+1)*mode+1], mode);

				memcpy(&list[(j+1)*mode+1] , tmp, mode);
			}
		}
	}
}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SetWordToBuffer
  
  input parameter:   void
   
  output parameter:	 0 = FAIL, otherwise user selected item identifier

  description:       Get Response Item Identifier
  					 
  
  status:			     

 -------------------------------------------------------------------------------------------------------------------*/    
void SetWordToBuffer(WORD len,BYTE * buf)
{
#ifdef LIB_HAL_LittleEndian       //小端格式长度
		buf[0] = len&0xFF;
		buf[1] = (len>>8)&0xFF;		
#else                             //大端格式长度
		buf[0] = (len>>8)&0xFF;
		buf[1] = len&0xFF;
#endif	
}

/*
*@Function   :ByteToUWORD
*
*@description: 
*
*@param      : void 
*
*@Return:  	  :void
*
*@Author:      xuyz
*
*@History:     V1.0--
*/
UWORD ByteBufToUWORD(BYTE *buf)
{
	return 	(((unsigned long)buf[0] << 24) + ((unsigned long)buf[1] << 16) + ((unsigned long)buf[2] << 8)  + buf[3]);
}

/*
*@Function   :UWORDToByteBuf
*
*@description: 
*
*@param      : void 
*
*@Return:  	  :void
*
*@Author:      xuyz
*
*@History:     V1.0-- 
*/
void UWORDToByteBuf(UWORD value, BYTE *buf)
{
	buf[0] =  ((value  >> 24) &0xFF);
	buf[1] =  ((value  >> 16) &0xFF);
	buf[2] =  ((value  >> 8) &0xFF);
	buf[3] =  (value  &0xFF);
}



