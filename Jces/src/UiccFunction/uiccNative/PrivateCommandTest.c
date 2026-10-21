/*

测试指令文件

 */
#include  "typedef.h"
#include  "main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include  "ceph.h"
#include  "interface.h"
#include  "PrivateCommandTest.h"
#include  "eepromdefine.h"
#include  "error.h"

#ifdef High_Update_FOR_Test
//强更新未见测试--透明文件 INS 0xA5
//P1 00/read;01/set;02/write; p2 偏移

void TFFileUpdateForTest(void)
{
	BYTE idata updateFiletype;
	WORD idata updateFileSize;
	
	updateFiletype = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_Type_Offset);

	if((updateFiletype & 0x07) != 0x01)
	{
		IncorrentParameter();
		return;
	}
	updateFileSize = GetTFSize(G_CurrentEfAddr[G_ChannelNum]);
	
	if((P3 >updateFileSize) || (P3 == 0x00))
	{
		IncorrentParameter();
		return;		
	}
	
	if(P1 == 0x00)
	{
		ReadTFBuf(G_CurrentEfAddr[G_ChannelNum], P2,RecData, P3);
		
		SendData();
		
		CommandFinish(0x00);
		
		return;	
	}
	
	ReceiveData();
	
	memcpy(tempBuf,RecData,P3);
	
	
	if(P1 == 0x01)//Set的时候 RecData数据都一样
	{
		SetTFConst(G_CurrentEfAddr[G_ChannelNum], P2,tempBuf[0], P3);
	}
	else
	{
		WriteTFBuf(G_CurrentEfAddr[G_ChannelNum], P2,tempBuf, P3);
	}
	
	CommandFinish(0x00);
	
	return;	
	
}

/*                                                                                      
*@Function name :	 PersoRead_Update_Config 读强更新配置区
*
*@Command format:    XX    AA P1P2 P3 //P1P2为偏移  P3是读取数据长度
									
*@Data    content:   void    
*
*@input parameter:   void                                                                
*
*
*@status:			  
*/
void PersoRead_Update_Config(void)
{
	UWORD xdata eepAddr;
	
	eepAddr=(((UWORD)P1<<8)&0xFF00)+P2;
	
	if(eepAddr>PFPageSize)
	{
		EepromError();
		
		return;
	}
	
	eepAddr += E_High_Update_Config;
	
	ReadEeprom(&RecData[0],eepAddr,P3);
	
	SendData();

  CommandFinish(0x00);
	
}
/*                                                                                      
*@Function name :	 PersoRead_Update_START 读强更新区域
*
*@Command format:    XX    AB P1P2 P3 //P1 表示读配置区第几页  P2 表示这一页的第几块(以0x80 为单位) /P3 长度固定为0x80
									
*@Data    content:   void    
*
*@input parameter:   void                                                                
*
*@status:			  
*/
void PersoRead_Update_START(void)
{
	UWORD xdata eepAddr;
	
	if(P3 != 0x80)
	{
		P3Error();

		return;		
	}
	
	if((P1 == 0x00) || (P2 == 0x00))
	{
		P1P2Error();
		
		return;
	}
	
	eepAddr=(P1-1) * PFPageSize + (P2-1) * 0x80;
	
	if(eepAddr >(E_High_Update_NUM*PFPageSize))
	{
		EepromError();
		
		return;
	}
	
	eepAddr += E_High_Update_START;
	
	ReadEeprom(&RecData[0],eepAddr,P3);
	
	SendData();

  CommandFinish(0x00);
}


/*                                                                                      
*@Function name :	 
*
*@Command format:    
									
*@Data    content:       
*
*@input parameter:                                                                   
*
*@status:			  
*/
void PersoHighUpdate_TEST(void)
{
	BYTE idata setValue,result;
	BYTE xdata inputTem[0x40];
	BYTE xdata outputTem[0x40];
	UWORD idata fileAddr;
	
	ReceiveData();
	
	memcpy(&inputTem[0],&RecData[0],P3);										
	
  if(P1 == 0x00)
	{
		fileAddr = SelectTFFile(0x7F20, 0x6F7E);              //透明文件7F20/6F7E设置为强更新
		
		if(P2 == 0x00)
		{
			WriteTFBuf(fileAddr,0x00,&inputTem[0],P3);	
			ReadTFBuf(fileAddr,0x00,&outputTem[0],P3);
			if(memcmp(&inputTem[0],&outputTem[0],P3) != EQUAL)
			{
				EepromError();
				return;
			}	
		}	
		else
		{
			setValue = inputTem[0];
			SetTFConst(fileAddr,0x00,setValue,P3);	
			ReadTFBuf(fileAddr,0x00,&outputTem[0],P3);
			memset(&inputTem[0],setValue,P3);
			if(memcmp(&inputTem[0],&outputTem[0],P3) != EQUAL)
			{
				EepromError();
				return;
			}		
		}			
	}	
  else if(P1 == 0x01)	
	{
			fileAddr = SelectLFFile(0x7FF0, 0x6FE4);              //线性文件7FF0/6FE4设置为强更新
		
		  if(P2 == 0x00)
			{
				WriteLFBuf(fileAddr,0x01,0x00,&inputTem[0],P3);							//更新第一条记录
				ReadLFBuf(fileAddr,0x01,0x00,&outputTem[0],P3);
				if(memcmp(&inputTem[0],&outputTem[0],P3) != EQUAL)
				{
					EepromError();
					return;
				}	
			}	
			else
			{
				setValue = inputTem[0];
				SetLFConst(fileAddr,0x02,0x00,setValue,P3);									//更新第二条记录
				ReadLFBuf(fileAddr,0x02,0x00,&outputTem[0],P3);
				memset(&inputTem[0],setValue,P3);
				if(memcmp(&inputTem[0],&outputTem[0],P3) != EQUAL)
				{
					EepromError();
					return;
				}	
			}	
				  
	}
	else if(P1 == 0x10)																		//STK中强更新测试
	{
		result = cephDisplayTextSingle(0x80,0x04, NULL);									//透明文件7F20/6F7E设置为强更新
																							//测试例子中写D6指令更新
																				  


	
	}
	
  CommandFinish(0x00);	
	return;
	

}



#endif
