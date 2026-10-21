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
			else	//for baihe-test
			{;}
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
		else if( i == ',' )
		{
			return 0x0C;
		}
		else	//for baihe-test
		{;}
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
		
		//if (input[1] != 0x81)							//Add by xuqs in 2007-12-17: 增加对不是81和91的情况的处理
		if ((input[1] != 0x81)&&(input[1] != 0xA1))							//Add by xuqs in 2007-12-17: 增加对不是81和91、A1的情况的处理											
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
BYTE AsciiToUCS2(BYTE * dataBuffer,BYTE * resultBuffer)
{
	BYTE idata  i;

	if (dataBuffer[0] > 0x7F)	   		//异常情况
	{
		return FALSE;
	}

	for (i = dataBuffer[0]; i > 0; i --)
	{
		resultBuffer[2*i] = dataBuffer[i];

		resultBuffer[2*i -1] = 0x00;
	}

	resultBuffer[0] = dataBuffer[0]*2;

	return TRUE;
}

/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 UCS2ToAscii                                                                                                               
                                                                                                                                                
  Input parameter:		* src: 要转换的源数据(LV)
  						* dest:转换后的结果数据(LV)
						(src和dest可以为同一个LV Buffer；当汉字与字母混合时不能同一个BUF)   
  					                                                   
  Output parameter:		TRUE/FALSE                                                                                                                  
                                                                                                                                                
  Description:       change the ucs2  to  Ascii

  example:			UCS2 -> ASCii
  					(0x04, 0x00,0x35,0x00,0x42) -> (0x02, 0x35,0x42)

  test:				test ok	
  
  author:          xuqs

  status:		

 -------------------------------------------------------------------------------------------------------------------*/                   
BYTE UCS2ToAscii(BYTE * src, BYTE * dest)
{
	BYTE idata i,j;

	if((src[0] == 0x00) || (src[0]%2 != 0x00))
	{
		return FALSE;
	}

	j = 0x00;

	for (i = 0x01; i<=src[0]; i+=2)
	{
		if(((src[i] != 0x00) &&(src[i] != 0x20)) || (src[i] == 0x20)&&(src[i+1] != 0xAC))
		{	
			return FALSE;
		}
		if (src[i] == 0x00)
		{
			switch (src[i+1] )
			{
				case 0x5C:
					dest[j+1] = 0x1B;
					dest[j+2] = 0x2F;
					j = j + 2;
					break;

				case 0x5E:
					dest[j+1] = 0x1B;
					dest[j+2] = 0x14;
					j = j + 2;
					break;

				case 0x7E:
					dest[j+1] = 0x1B;
					dest[j+2] = 0x3D;
					j = j + 2;
					break;

				case 0x5B:
					dest[j+1] = 0x1B;
					dest[j+2] = 0x3C;
					j = j + 2;
					break;

				case 0x5D:
					dest[j+1] = 0x1B;
					dest[j+2] = 0x3E;
					j = j + 2;
					break;

				case 0x7B:
					dest[j+1] = 0x1B;
					dest[j+2] = 0x28;
					j = j + 2;
					break;

				case 0x7D:
					dest[j+1] = 0x1B;
					dest[j+2] = 0x29;
					j = j + 2;
					break;

				case 0x40:
					dest[j+1] = 0x00;
					j++;
					break;

				case 0x5F:
					dest[j+1] = 0x11;
					j++;
					break;

				case 0xA5:
					dest[j+1] = 0x03;
					j++;
					break;

				case 0x24:
					dest[j+1] = 0x02;
					j++;
					break;

				case 0xA3:
					dest[j+1] = 0x01;
					j++;
					break;

				case 0xA1:
					dest[j+1] = 0x40;
					j++;
					break;

				case 0xA4:
					dest[j+1] = 0x24;
					j++;
					break;

				default:
					dest[j+1] = src[i+1];
					j++;
					break;
			}			
		}
		else if ((src[i] == 0x20) && (src[i+1] == 0xAC))
		{
			dest[j+1] = 0x1B;
			
			dest[j+2] = 0x65;
			
			j = j + 2;
		}
		else	//for baihe-test
		{;}		
	}

	dest[0] = j;	//结果数据长度

	return TRUE;
}

/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	 CharacterUnPack                                                                                                                   
                                                                                                                                                
  Input parameter:   void:   be added buffer :the buffer must tlv structure 					        see 11.14 page 44

  Output parameter:  THE length of user data                                                                                                                 
                                                                                                                                                
  Description:       This function is to add the tlv structure to the buffer of have data :the buffer must have data of thv structure
  
  example:
             
---------------------------------------------------------------------------------------------------------------------------------------------------*/
void CharacterUnPack(BYTE  * pDataStr,BYTE  * pResultStr,BYTE Lgth, BYTE fillBits)
{
	BYTE idata i;	//Destination Buffer Index
	BYTE idata num;	//Remain Bits in Source Data
	BYTE idata j;	//Source Buffer Index
	
	num = 8 - fillBits;
	
	for(i = 1, j = 0 ;  j<Lgth;  i++)
	{
		if( num == 0 )
		{
			num = 8;

			j ++;
		}

		if( num < 7 )
		{
			pResultStr[i] = (((pDataStr[j]&0x00FF ) >> (8-num)))&0x7F;

			pResultStr[i] |= (((pDataStr[j+1]&0x00FF) << num )&0x7F );
			
			j ++;
			
			num ++;
		}
		else
		{
			pResultStr[i] = (pDataStr[j] >> ( 8 - num ) )&0x7F;

			num = num - 7;
		}		
	}
	
	pResultStr[0x00] = (Lgth/7)*8 + Lgth%7;
}

/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	 CharacterPack                                                                                                                   
                                                                                                                                                
  Input parameter:   * pDataStr		要打包的字符串
  					 Lgth			字符串长度
					 type			打包类型：TYPE_NORMAL	普通字串打包
					 						  TYPE_USSD 	USSD字串打包			        

  Output parameter:  打包后的7BIT字串长度                                                                                                                 
                                                                                                                                                
  Description:        将8BIT字串打包成7BIT，返回打包后的7BIT字串长度，分为普通字串和USSD字串两种情况处理

  Example:				 8bit: 31323334353637
						 7bit: 31D98C56B3DD00
					ussd 7bit: 31D98C56B3DD1A

						 8bit: 7F7F7F7F7F7F7F7F
						 7bit: FFFFFFFFFFFFFF
					ussd 7bit: FFFFFFFFFFFFFF
            
---------------------------------------------------------------------------------------------------------------------------------------------------*/

BYTE CharacterPack(BYTE * pDataStr, BYTE Lgth, BYTE type)
{
	BYTE idata 	i;
	BYTE idata 	num;
	BYTE idata 	j;
	BYTE idata 	lastByte;
	BYTE idata 	returnLgth;

	j = 0x00;

	i = 0x00;

	while (i < Lgth)
	{
		num = 0;

		while ((num<8)&&((i+num)<Lgth))
		{			
			switch (num)
			{
				case 0x00:
						pDataStr[j+num] = pDataStr[i+num]&0x7F;
						break;

				case 0x01:
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05:
				case 0x06:
						pDataStr[j+num-1] = ((pDataStr[i+num]<<(8-num))&(0xFF<<(8-num))) | pDataStr[j+num-1];
						pDataStr[j+num] = (pDataStr[i+num]>>num)&(0x7F>>num);
						break;
				case 0x07:
						pDataStr[j+num-1] = ((pDataStr[i+num]<<1)&0xFE)|pDataStr[j+num-1];
						break;
				default:		//for baihe-test
						break;
			}
		
			num=num+1;		
		}

		i = i + 8;
 
		j = j + 7;
	}

	lastByte =  0xFF>>(Lgth % 8);

	returnLgth = Lgth%8 + Lgth/8*7 ;
	
	if(type == TYPE_NORMAL)
	{
		pDataStr[j+num] = pDataStr[i+num]&lastByte;
	}
	else	//TYPE_USSD
	{
		pDataStr[j+num-8] = pDataStr[j+num-8]&lastByte;

		if((Lgth%8) == 0x07)
		{
			pDataStr[j+num-8] = pDataStr[j+num-8] | 0x1A ;    // j 多加了7，NUM多加了1。
		}

		if((Lgth%8) == 0x00)
		{
			if((pDataStr[j+num-8]&0xFE) == 0x1A)
			{
				pDataStr[j+num -7] = 0x0D ;

				returnLgth ++ ;
			}
		}
	}
	
	return	returnLgth ; 	
}



/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 TenToSixteen                                                                                                                 
                                                                                                                                                
  Input parameter:   lvString: 要转换的源数据(LV)
  					 outPut：转换后的结果数据	
  					                                                   
  Output parameter:  TRUE/FALSE                                                                                                          
                                                                                                                                                
  Description:       10进制转换为16进制，只转换255以内的值   
  
  coder:		zhangxl

  example:			(0x03, 0x31,0x32,0x33) -> (0x7B)
  					(0x02, 0X31,0X39) -> (0x13)

  status: 

 -------------------------------------------------------------------------------------------------------------------*/                   

BYTE TenToSixteen(BYTE * lvString, BYTE * outPut)
{
	BYTE idata i;

	if(lvString[0] >= 0x04)
	{
		return FALSE;
	}

	for(i=0x00; i<lvString[0]; i++)
	{
		if(((lvString[i+1]&0xF0) != 0x30) ||((lvString[i+1]&0x0F) > 0x09))
		{
			return FALSE;
		}
	}

	outPut[0] = 0x00;

	for(i=1; i<=lvString[0]; i++)
	{
		outPut[0] = (outPut[0]*10) + (0x0F&lvString[i]);	
	}

	return TRUE;
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
			memmove(&buf[1], &buf[2], buf[0]-1);

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

	AsciiToUCS2(lv_dst, lv_dst);		//ASCii to UCS2

	return TRUE;
}


/*
 *@description :this function is used to reverse the buf,例如："abcd" -> "dcba"
 *
 *@param       :buf : LV
 *
 *@history     :create @091116
*/
void reverse(BYTE * buf)
{
	BYTE idata i,j,tmp;

	for(i=1,j=buf[0];i<j;i++,j--)	//将字符串倒转
	{
		tmp = buf[i];

		buf[i] = buf[j];

		buf[j]= tmp;
	}
}



/*
 *@description :this function is used to convert decimalist-Number TO hex
 *
 *@param       :Num:the  decimalist-Number ;resultBuf : LV
 * 
 *@returns

  example:		src -> dst
  				(0x123456) -> (0x03, 0x12,0x34,0x56)
 *
 *@history     :create @091116 11:00
*/
void NumToHEX(DWORD src,BYTE *buf)
{
	BYTE idata i;
	BYTE idata high,low;

	i = 0;
	
	if(src == 0)
	{
		buf[0] = 0x01;
		buf[1] = 0x00;
		return;
	}

	while(src >0)
	{
		low = (BYTE)(src%16);
		
		high = (BYTE)((src/16)%16);

		buf[++i] = ((high<<4)&0xF0)|low;
	
		src = (src/16)/16;
	}
	
	buf[0] = i;

	reverse(&buf[0]);
}



/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 IsASCIIString                                                                                                                 
                                                                                                                                                
  Input parameter:   Lv结构，要判断的字符buf
  					                                                   
  Output parameter:                                                                                                            
                                                                                                                                                
  Description:       判断一串字符是否为ASCII编码
  
  coder:		xiaoyh

  status: 

 -------------------------------------------------------------------------------------------------------------------*/       
BYTE IsASCIIString( BYTE* buf)
{
	BYTE xdata i =1;
	for(;i <buf[0];i=i+2)
	{
		if( buf[i] != 0x00)
		{
			return FALSE;
		}
	}
	return TRUE;
}


/**
 * @brief 		计算ADN个数 
 * 
 * @param 		buf: ADN string(V) 
 * @param 		len: length of ADN string 
 * 
 * @Returns   	ADN个数
 * 
 * @author 		WG 
 * 
 * @history 	add comment by WG, @071224 13:59
 */
BYTE GetBCDCount(BYTE * buf, BYTE len)
{
	BYTE idata c;
	
	c = (len - 1) * 2;

	if ((buf[len - 1] & 0xF0) == 0xF0)
	{
		c = c -1;
	}

	return c;
}

