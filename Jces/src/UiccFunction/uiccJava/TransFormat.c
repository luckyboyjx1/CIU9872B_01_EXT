/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
 			V1.1 修改函数UCS2TOASCII,modify by xyz@20101207 Version: V1.19->V1.20

 */
#include <uicc_error.h>
#include  "typedef.h"
#include  "uicc_main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include  "arithmatic.h"
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 IntToChar
  
  input parameter:   i:		源数据
  					 mode:	HextoASCii		Hex转为ASCii
					 		BCDtoASCii		BCD转为ASCii

  output parameter:	 转换的结果数据
                                                                                                                                                
  description:       Convert BYTE or BCD to ASCII(char)

  example:			 
  
  test:				 xuzhichu,Caibo
  
  status:			 OK     

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE IntToChar(BYTE i, BYTE mode)
{
	if(i > 0x0F)
	{
		return 0;
	}

	if(i <= 9)
	{
		return ( i + '0' );
	}
	else
	{
		if(mode == BCDtoASCii)
		{
			if( i == 0x0A )
			{
				return '*';
			}
			else if( i == 0x0B )
			{
				return '#';
			}
			else if ( i == 0x0C )
			{
				return ',';
			}
			else
			{
				;
			}
		}
		return (i + 'A' - 10);   
	}
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 CharToInt
  
  input parameter:   i:		源数据
  					 mode:	ASCiitoHex		ASCii转为Hex
					 		ASCiitoBCD		ASCii转为BCD

  output parameter:	 转换的结果数据
                                                                                                                                                
  description:       Convert ASCII to BYTE or BCD(char)

  example:			 
  
  test:				 xuzhichu,Caibo
  
  status:			 OK     

 -------------------------------------------------------------------------------------------------------------------*/
BYTE CharToInt(BYTE i, BYTE mode)
{
	if(mode == ASCiitoBCD)
	{
		if( i == '*' )
		{
			return 0x0A;
		}
		else if( i == '#' )
		{
			return 0x0B;
		}
		else // if( i == ',' )
		{
			if( i == ',' )
			{
				return 0x0C;
			}
		}
	}

	if( (i >='0' ) && ( i <='9' ) )
	{
		return ( i - '0' );
	}
	else if( (i >= 'A' ) && ( i <= 'F') )
	{
		return ( i - 'A' + 10 );
	}
	else if( (i >='a' ) && ( i <= 'f') )
	{
		return ( i - 'a' + 10 );
	}
	else
	{
		return 0 ;
	}
}

/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 AsciiToADN                                                                                                               
                                                                                                                                                
  Input parameter:   input : need Convert ascii ,lvstring
  					 outputr   : correct format ascii ,lvstring
					 (input和output不能为同一个LV Buffer) 
  					                                                   
  Output parameter:  TRUE/FALSE
  
  example:			ASCii -> ADN
  					(0x07, 0x2b,0x31,0x32,0x33,0x34,0x2C,0x2A) -> (0x04, 0x91,0x21,0x43,0xAC)
  
  test:             test ok                                                                                                    
                                                                                                                                                
  Description:       change the Ascii to ADN format
  
  status:            

 -------------------------------------------------------------------------------------------------------------------*/                   

BYTE AsciiToADN(BYTE * input, BYTE * output)
{
	BYTE idata i, j;
	BYTE idata tmp1,tmp2;
		
    if(input[0] == 0x00)
    {
       output[0] = 0x00;
   
       return FALSE;
    }                              //add by xyz for SPB ,当输入参数长度为0 ，输出直接返回

	if(input[1] == '+')
	{
		output[1] = 0x91;

		i = 0x02;		
	}
	else
	{
		output[1] = 0x81;

		i = 0x01;	
	}

	j = 0x02;

	while(1)
	{
		tmp1 = CharToInt(input[i], ASCiitoBCD);

		if(i == input[0])
		{
			tmp2 = 0xFF;
		}
		else
		{
			tmp2 = CharToInt(input[i+1], ASCiitoBCD);
		}

		output[j] = ((tmp2<<4)&0xF0) + (tmp1&0x0F);

		i = i+2;

		j++;

		if((tmp2 == 0xFF) || (i == input[0]+1))
		{
			break;
		}
	}

	output[0] = j - 1;

	return TRUE;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 IntToString
  
  input parameter:   lv_src: 要转换的源数据(LV)
  					 lv_dst: 转换后的结果数据(LV)
					 (lv_src和lv_dst可以为同一个LV Buffer)
  				
  output parameter:		TRUE/FALSE	 
                                                                                                                                                
  description:       Convert Hex to ASCII (string)

  example:		Hex -> ASCii
  				(0x02, 0x01,0x2B) -> (0x04, 0x30,0x31,0x32,0x42)
  
  test:			xuqs	test ok
  
  status:			      

 -------------------------------------------------------------------------------------------------------------------*/    

BYTE IntToString(BYTE * lv_src, BYTE * lv_dst)
{
	BYTE idata i, j;
	
	for (i = lv_src[0], j = lv_src[0]*2;( i > 0)&&( j > 0); i--, j -=2)
	{
		lv_dst[j] = IntToChar((lv_src[i]&0x0F), HextoASCii);

		lv_dst[j-1] = IntToChar(((lv_src[i]>>0x04)&0x0F), HextoASCii);
	}

	lv_dst[0] = lv_src[0]*2;

	return TRUE;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 StringToInt
  
  input parameter:   lv_src: 要转换的源数据(LV)
  					 lv_dst: 转换后的结果数据(LV)
					 (lv_src和lv_dst可以为同一个LV Buffer)
  				
  output parameter:		TRUE/FALSE	 
                                                                                                                                                
  description:       Convert ASCii to Hex (string)

  example:			ASCii -> Hex
  					(0x04, 0x30,0x31,0x32,0x42) -> (0x02, 0x01,0x2B)		
  
  test:				test ok
  
  status:			      

 -------------------------------------------------------------------------------------------------------------------*/
BYTE StringToInt(BYTE * lv_src, BYTE * lv_dst)
{
	BYTE idata i, j;

	if((lv_src[0] == 0) || (lv_src[0]%2 != 0))
	{
		return FALSE;
	}

	for(i=1,j=1; ( i<=lv_src[0]-1)&&( j<=lv_src[0]/2); i+=2,j++)
	{
		lv_dst[j] = (CharToInt(lv_src[i], ASCiitoHex)<<4) + CharToInt(lv_src[i+1], ASCiitoHex);
	}

	lv_dst[0] = lv_src[0]/2;

	return TRUE; 
}
/**
 * @brief 		change ascii to ucs2
 * 
 * @input param 		dataBuffer :data lv structure
 						resultBuffer :result lv structure
						(dataBuffer和resultBuffer可以为同一个LV Buffer)
						 
 * @output param		TRUE/FALSE
 
 	example			ASCii -> UCS2
					(0x02, 0x35,0x42) -> (0x04, 0x00,0x35,0x00,0x42)
					
	test		test ok 		 
 * 
 * @author 		WG 
 * 
 * @history 	add comment by WG, @071224 13:59
 */
void AsciiToUCS2(BYTE * dataBuffer,BYTE * resultBuffer)
{
	BYTE idata  i;

	if (dataBuffer[0] > 0x7F)	   		//异常情况
	{
		return;
	}

	for (i = dataBuffer[0]; i > 0; i --)
	{
		resultBuffer[2*i] = dataBuffer[i];

		resultBuffer[2*i -1] = 0x00;
	}

	resultBuffer[0] = dataBuffer[0]*2;
}
/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 SixteenT0Ten的ascii                                                                                                               
                                                                                                                                                
  Input parameter:   WORD src: 要转换的源数据
  					 buf: 转换后的结果数据(LV)
  					                                                   
  Output parameter:  TRUE/FALSE                                                                                                               
                                                                                                                                                
  Description:       16进制转换为10进制 

  code:	xuqs

  example:			Sixteen -> Ten
  					(0x0013) -> (0x02, 0X31,0X39) 十进制19
					(0x007B) -> (0x03, 0x31,0x32,0x33) 十进制123
                    (0x0112) -> (0x03, 0x32,0x37,0x34) 十进制的274
					(0x1234) -> (0x04, 0x34,0x36,0x36,0x30) 十进制的4660

  test:				test ok
  
  status:            

 -------------------------------------------------------------------------------------------------------------------*/

BYTE SixteenToTen(WORD src, BYTE * buf)
{
	buf[1] = src/10000 + '0';

	buf[2] = (src%10000)/1000 + '0';

	buf[3] = (src%1000)/100 + '0';

	buf[4] = (src%100)/0x0A + '0';

	buf[5] = src%10 + '0';

	buf[0] = 0x05;		//到此完成16进制TO 10进制

	for(; buf[0] > 1;)
	{	
		if(buf[1] != 0x30)
		{
			break;
		}
		else
		{
			ecpMem_Move(&buf[1], &buf[2], buf[0]-1);

			buf[0] --;
		}
	}

	return TRUE;
}
/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 HexToUCS2                                                                                                               
                                                                                                                                                
  Input parameter:   src: 要转换的源数据
  					 lv_dst: 转换后的结果数据(LV)
  					                                                   
  Output parameter:  TRUE/FALSE                                                                                                               
                                                                                                                                                
  Description:       Convert HEX  to UCS2 of decimal 

  code:	xuqs

  example:			Hex -> UCS2
  					(0x13) -> (0x04, 0x00,0x31,0x00,0x39)

  test:				test ok
  
  status:            

 -------------------------------------------------------------------------------------------------------------------*/                   
BYTE HexToUCS2(BYTE src, BYTE * lv_dst)
{
	SixteenToTen(src, lv_dst);

	AsciiToUCS2(lv_dst, lv_dst);		//ASCii to UCS2，该行代码在17.8.21被梦飞注释掉(注释掉的原因已经无从知道了)，18.11.7产业部测试时出现《拼装电话本条数及容量是乱码》问题，所以将该代码放开。

	return TRUE;
}

/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 ADNToAscii                                                                                                               
                                                                                                                                                
  Input parameter:   input : need Convert adn ,lvstring
  					 output   : correct format adn ,lvstring
					 (input和output不能为同一个LV Buffer) 
  					                                                   
  Output parameter:	 TRUE/FALSE
  
  example:           ADN -> ASCii
  					(0x04, 0x91,0x21,0x43,0xAC) -> (0x07, 0x2b,0x31,0x32,0x33,0x34,0x2C,0x2A)
					
  test:				test ok                                                                                                        
                                                                                                                                                
  Description:       Convert adn phone no to correct format to display
  
  status:            

 -------------------------------------------------------------------------------------------------------------------*/
BYTE ADNToAscii(BYTE * input, BYTE * output)
{
	BYTE idata i,j;

	i = 0x02;

	if(input[1] == 0x91)
	{
		output[1] = '+';

		j = 0x02;
	}
	else
	{
		j = 0x01;
		
		if (input[1] != 0x81)							//Add by xuqs in 2007-12-17: 增加对不是81和91的情况的处理											
		{
			i = 0x01;	
		}
	}

	for(; i<(input[0]+1); i++)		   //i<input[0]-1 to i<input[0]+1			by xuqs
	{
		output[j] =	IntToChar((input[i]&0x0F), BCDtoASCii);
		
		output[j+1] = IntToChar(((input[i]>>4)&0x0F), BCDtoASCii);

		if((output[j] == 'F')||(output[j+1] == 'F'))
		{
		   j = j - 1;						 //add by xuqs
		}
		
		j = j + 2; 
	}

	output[0] = j - 1;

	return TRUE;
}
