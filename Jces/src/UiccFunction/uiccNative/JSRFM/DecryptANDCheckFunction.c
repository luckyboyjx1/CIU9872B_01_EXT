/*-------------------------------------------------------------------------------------------------------//
  Copyright (c) 2003,eastcompeace

  All rights reserved.

  MODEL:EMOSYN TG96-3G

  MOTOCOS V4.10

  PHASE 2 + STK(WIB)  RELEASE:2003-2-25
  
  V1.0   创建源文件

 --------------------------------------------------------------------------------------------------------*/

#include  "typedef.h"
#include  "main.h"
#include  "fileAction.h"
#include  "string.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include  "rfm.h"
#include  "Authentication.h"
#include  "fileSystem.h"
#include  "resume.h"
#include  "hard_api.h"
#include  "arithmatic.h"
#include  "interface.h"
#include  "Transaction.h"

#ifdef _AES_
#include  "AES.h"
#endif


BYTE xdata G_WIBTag[8];    // save from CHL to TAR
BYTE xdata G_WIBMac[8];    // save CC/RC
BYTE xdata G_CheckSumLgth;
BYTE xdata G_CurrentComNUM;
WORD xdata CommandStartNum;
//BYTE xdata G_RefNumber;		//批次号
//BYTE xdata G_ReTarType;
//BYTE xdata G_ReTarSc;
BYTE xdata G_ReTarNo;
BYTE xdata G_TempTarNo;
BYTE xdata G_TempSc;
BYTE xdata G_TarTypeMode;



/*
** description:获取KIC,KID,KIK在4E0A中的索引最大值或者索引值对应在4E06中的记录号
**
** input:	tag:KIC,KID,KIK的标记
**			index:索引值，不用可以设为0
**			mode:为0时，返回对应标记最大索引值;为1时，返回索引值对应在4E06中的记录号
** output：		
*/

BYTE GetKeyIndex(UWORD fileAddr,BYTE tag, BYTE index, BYTE mode)
{
	WORD xdata fileSize;
	BYTE xdata offset=0,offset_index;
	BYTE xdata buf[4];
	
	fileSize = GetTFSize(fileAddr);
	
	while(offset < fileSize)
	{
		ReadTFBuf(fileAddr,0x02+offset,(BYTE *)&buf[0],2);
		
		if(buf[0] == tag)
		{
			if( mode == 0x00)
			{
				ReadTFBuf(fileAddr,offset+0x02+buf[1],(BYTE *)&buf[2],1);
				
				return buf[2];
			}
			else
			{
				offset_index=0;
				
				while(offset_index < buf[1])
				{				
					ReadTFBuf(fileAddr,offset+0x02+2+offset_index,(BYTE *)&buf[2],2);
					
					if(buf[2] == index)
					{
						return buf[3];
					}
					
					offset_index += 2;
				}
				
				return FALSE;
			}
		}
		
		offset += buf[1]+2;
	}
	
	return FALSE;
}


/*
 Function name:	TripleDES_3Key
 

 Description:  根据TripleDES改动，用于TripleDES的3Key模式下数据的加解密

  Status:		need test	

*/
void TripleDES_3Key(BYTE * pbSrcStr,BYTE * keyContext,BYTE * pbTgtStr, BYTE bMod, BYTE lgth)
{
	BYTE xdata dataPointer[8];
	BYTE xdata resultData[8];
	BYTE idata num;
	BYTE idata i,mode,tmp;

	if(((lgth & 0x07) != 0x00) || (lgth == 0x00))
	{
		return; 
	}

	memset(dataPointer, 0x00, sizeof(dataPointer));

	memset(resultData, 0x00, sizeof(resultData));
	
	mode = ( ((bMod&0x0F) == 0x0E) ? 0x01 : 0x00);	    // 0x01	-> DEC
															// 0x00 -> ENC
	for(num = 0x00; num < lgth; num += 0x08)
	{		
		for(i = 0x00; i < 0x08; i++)
		{
			if(bMod == CBC_Encrypt)
			{
				dataPointer[i] ^= pbSrcStr[i + num];
			}
			else
			{
				dataPointer[i] = pbSrcStr[i + num];
			}
		}

		if(mode == 0x00)									// 0x00 -> ENC				
		{
			SDES(&dataPointer[0], &keyContext[0x00], &dataPointer[0], mode);
        		
		    SDES(&dataPointer[0], &keyContext[0x08], &dataPointer[0], 0x01);
			
			SDES(&dataPointer[0], &keyContext[0x10], &dataPointer[0], mode);
				
		}	
		else // if(mode == 0x01)										//0x01	-> DEC													
		{
			SDES(&dataPointer[0], &keyContext[0x10], &dataPointer[0], mode);

			SDES(&dataPointer[0], &keyContext[0x08], &dataPointer[0], 0x00);

			SDES(&dataPointer[0], &keyContext[0x00], &dataPointer[0], mode);			
		}

		for(i = 0x00; i < 0x08; i++)
		{
			if(bMod == CBC_Decrypt)
			{
				tmp = pbSrcStr[i + num];

				pbTgtStr[i + num] = dataPointer[i] ^ resultData[i];

				resultData[i] = tmp;
			}
			else
			{
				pbTgtStr[i + num]	= dataPointer[i];
			}
		}
	}
}



/*---------------------------------------------------------*/
/* FUNCTION : lGenCRC32 */
/* Polynomial: */
/* X32+X26+X23+X22+X16+X12+X11+X10+X8+X7+X5+X4+X2+X1+X0 */
/* */
/* PARAMETERS IN : */
/* lOldCrc :the crc32 value which */
/* was built over previous bytes */
/* ByteVal : the new data byte */
/* --------------------------------------------------------*/
static unsigned long lGenCRC32( unsigned long lOldCRC,BYTE ByteVal )
{
   unsigned long idata TabVal;
   int idata j;

   TabVal = ((lOldCRC) ^ ByteVal) & 0xFF;

   for (j=8; j>0; j--)
   {
      if (TabVal & 1)
	  {         
            TabVal = (TabVal >> 1) ^ 0xEDB88320L;
      }   
      else
      {   
           TabVal >>= 1;
       }  
   }

 	return TabVal ^ (((lOldCRC) >> 8) & 0x00FFFFFFL);
}


/*-------------------------------------------------------------------------------------------------------//

  Function name:	CRC32
 
  Input parameter:	
 			        cpEnter:data

					cpKey  :key	

					cpResult:result

					nEnterLen: length of data
 
  Output parameter:     the number of tlv structure
 
  Description:          return the number that first tlv structure in input buffer 

  Status:		OK

 -------------------------------------------------------------------------------------------------------------------*/
void CRC32(UWORD eepAddr,BYTE * c_result,WORD lgth)
{
	BYTE idata i,j;
    unsigned long idata Crc32;
	BYTE idata cpEnter[16];
	BYTE idata k,len;

	k = lgth / 16;
  
    Crc32 = 0xFFFFFFFF; 

	for (i=0;i <= k;i++)
	{
		len = (i==k?(lgth % 16):16);

		ReadEeprom((BYTE *)&cpEnter[0],eepAddr+i*16,len);
		
		for(j=0; j<len; j++)
		{
			Crc32 = lGenCRC32(Crc32,*(cpEnter + j));
		}							
	}

    Crc32 = ~Crc32;    

    for(j=0; j<0x04; j++)
    {
       *(c_result + (3-j)) = Crc32 >>(j*8);
    }
}



/*-------------------------------------------------------------------------------------------------------//

  Function name:	DesMacFunction
 
  Input parameter:	
 			        cpEnter:data

					cpKey  :key	

					cpResult:result

					nEnterLen: length of data

					mode: DES/3DES

					tripleDESMode:  2key/3key
 
  Output parameter:     the number of tlv structure
 
  Description:          return the number that first tlv structure in input buffer 

  Status:		OK

 -------------------------------------------------------------------------------------------------------------------*/

void DesMacFunction(UWORD eepromAddr,BYTE * cpKey,BYTE cpResult[8],WORD nEnterLen,BYTE mode,BYTE tripleDESMode)
{
	WORD xdata i;	
	BYTE idata j;
	BYTE xdata cpEnter[8];

	memset(cpResult,0x00,0x08);

	for(i=0x00;i < nEnterLen;i=i+0x08)
	{
		ReadEeprom((BYTE *)&cpEnter[0],eepromAddr+i,8);

		for(j=0;j<0x08;j++)
		{
		  	cpEnter[j]=cpEnter[j] ^ cpResult[j];
	    } 

		if(mode == TRIPLEDES)   							
		{
			if(tripleDESMode == TripleDES2Key)
			{
				TripleDES(&cpEnter[0],&cpKey[0],&cpResult[0],CBC_Encrypt,0x08);
			}
			else // if(tripleDESMode == TripleDES3Key)
			{
				TripleDES_3Key(&cpEnter[0],&cpKey[0],&cpResult[0],CBC_Encrypt,0x08);
			}
			
		}
		else
		{
			SingleDES(&cpEnter[0],&cpKey[0],&cpResult[0],CBC_Encrypt,0x08); 
		}
	}	
}


/*-------------------------------------------------------------------------------------------------------//

  Function name:	SmartISO9797Algorithm3Ee
 
  Input parameter:	
 			        cpEnter:data

					cpKey  :key	

					cpResult:result

					nEnterLen: length of data
 
  Output parameter:     the number of tlv structure
 
  Description:          return the number that first tlv structure in input buffer 

  Status:		OK

 -------------------------------------------------------------------------------------------------------------------*/

void SmartISO9797Algorithm3Ee(UWORD eepromAddr,BYTE * cpKey,BYTE cpResult[8],WORD nEnterLen)
{
	WORD xdata i;	
	BYTE idata j;
	BYTE xdata cpEnter[8];

	memset(cpResult,0x00,0x08);

	for (i=0x00;i < nEnterLen;i=i+0x08)
	{
		ReadEeprom((BYTE *)&cpEnter[0],eepromAddr+i,8);

		for(j=0;j<0x08;j++)
		{
		  	cpEnter[j]=cpEnter[j] ^ cpResult[j];
	    }     

		SingleDES(&cpEnter[0],&cpKey[0],&cpResult[0],CBC_Encrypt,0x08);
	}

	SingleDES(&cpResult[0],&cpKey[0],&cpResult[0],ECB_Decrypt,0x08);
	
	TripleDES(&cpResult[0],&cpKey[8],&cpResult[0],CBC_Encrypt,0x08);	
}



/*-------------------------------------------------------------------------------------------------------//

  Function name:	SmartISO9797Algorithm4Ee
 
  Input parameter:	
 			        cpEnter:data

					cpKey  :key	
					       cpKey[0]-cpKey[7]  : K''
						   cpKey[16]-cpKey[23]: K
						   cpKey[24]-cpKey[31]: K'

					cpResult:result

					nEnterLen: length of data
 
  Output parameter:     the number of tlv structure
 
  Description:          return the number that first tlv structure in input buffer 

  Status:		OK

 -------------------------------------------------------------------------------------------------------------------*/


void SmartISO9797Algorithm4Ee(UWORD eepromAddr,BYTE * cpKey,BYTE cpResult[8],WORD nEnterLen,WORD initLen)
{
	WORD idata i;	
	BYTE idata j;
	BYTE xdata cpEnter[8];
	BYTE xdata ucTemp[8];
		
	memset(&ucTemp[0],0x00,sizeof(ucTemp));
	
	ucTemp[6] = ((initLen*8)>>8)&0xFF;
	
	ucTemp[7] = (initLen*8)&0xFF;                                            
	
	SingleDES(&ucTemp[0],&cpKey[8],&ucTemp[0],ECB_Encrypt,0x08);	 // eK(D1)

	SingleDES(&ucTemp[0],&cpKey[0],&cpResult[0],ECB_Encrypt,0x08);        // eK''(eK(D1))	

	for (i=0x00;i < nEnterLen;i=i+0x08)
	{
		ReadEeprom((BYTE *)&cpEnter[0],eepromAddr+i,8);

		for(j=0;j<0x08;j++)
		{
		  	cpEnter[j]=cpEnter[j] ^ cpResult[j];
	    }     

		SingleDES(&cpEnter[0],&cpKey[8],&cpResult[0],CBC_Encrypt,0x08);
	}
	
	SingleDES(&cpResult[0],&cpKey[16],&cpResult[0],ECB_Encrypt,0x08);
}



/*-------------------------------------------------------------------------------------------------------//

  Function name:	ISO9797Algorithm3
 
  Input parameter:	
 			        cpEnter:data

					cpKey  :key	

					cpResult:result

					nEnterLen: length of data
 
  Output parameter:     the number of tlv structure
 
  Description:          return the number that first tlv structure in input buffer 

  Status:		OK

 -------------------------------------------------------------------------------------------------------------------*/

void ISO9797Algorithm3(BYTE * cpEnter,BYTE * cpKey,BYTE * cpResult,WORD nEnterLen)
{
	BYTE idata j;
	WORD idata i;
	BYTE idata copyData[8];

	SingleDES(&cpEnter[0],&cpKey[0],&cpResult[0],ECB_Encrypt,0x08);
		
	for(i=0x08; i<(nEnterLen); i=i+0x08)
	{
		for(j=0;j<0x08;j++)
		{
			copyData[j] = cpResult[j]^cpEnter[i+j];
		}

		SingleDES(&copyData[0],&cpKey[0],&cpResult[0],ECB_Encrypt,0x08);
	}

	SingleDES(&cpResult[0],&cpKey[8],&copyData[0],ECB_Decrypt,0x08);
	
	SingleDES(&copyData[0],&cpKey[0],&cpResult[0],ECB_Encrypt,0x08);
}



/*-------------------------------------------------------------------------------------------------------------------
Function name :	   GetScTagOffset

input parameter:   scIndex

output parameter:  sc len offset

description:       this function return the sc index of the given tag

status:			   need Test

author:            XuWeiHua

 -------------------------------------------------------------------------------------------------------------------*/

WORD GetScTagOffset(UWORD fileAddr,BYTE scIndex,BYTE tag)
{
	BYTE  idata buf[2],i;
	WORD  idata offset,fileSize;

	fileSize = GetTFSize(fileAddr);

	offset = 0;

	for(i=1;i<scIndex;i++)
	{
		ReadTFBuf(fileAddr,offset,(BYTE *)&buf[0],2);

	    offset = offset + buf[1] + 2;

		if(offset >= fileSize)
		{
			return FALSE;
		}
	}
	
	offset = offset + 2;

	while(offset < fileSize)
	{
		ReadTFBuf(fileAddr,offset,(BYTE *)&buf[0],2);

		if(buf[0] == tag)
		{
			return offset;
		}		
		else
		{
			offset = offset + buf[1] + 2;
		}
	}

	return FALSE;
}



/*-------------------------------------------------------------------------------------------------------//

  Function name:	ChipheRe
 
  Input parameter:	Spi
  					Kic
					pointer
					result_pointer
					mode：ENCRYPT/DECRYPT
					lgth
 			       
  Output parameter: the buffer of the key
 
  Description:      加/解密   

  status: 			need test
  
  author :          XuWeiHua      

 -------------------------------------------------------------------------------------------------------------------*/
BYTE ChipheRe(BYTE kic,BYTE * pointer,BYTE * result_pointer,BYTE mode,WORD lgth)
{
	BYTE idata result;
  	BYTE idata keyId;
  	BYTE idata decryptType;
	BYTE xdata keyValue[24];
#ifdef _AES_ 
	BYTE idata aesTag;
#endif

#ifndef _AES_  		
	if((kic&0x03) != 0x01)                           //that is , KIC(2-1bit)=01, DES 
  	{
	 	return ERROR_CIPHERING;
  	}
#endif

 	keyId = ((kic&0xF0)>>0x04);                       //get the keyID

	result = Findkey(KEY_KIC, keyId, &keyValue[0],0);

	if(result == FALSE)
	{
		return ERROR_CIPHERING; 
	}

#ifdef _AES_	
	aesTag = kic&0x03;					

	if(aesTag == 0x02)		  	//表示使用AES算法
	{	
		AESCalc(&pointer[0],&keyValue[0],&result_pointer[0], AES_DECRYPT, lgth,0x10);
		return POP_OK;
	}
#endif


  	decryptType = kic &0x0C;                          //get the KIC bit(4-3),that is ,mode

 	switch (decryptType)
	{
		case 0:
			SingleDES(&pointer[0],&keyValue[0],&result_pointer[0],CBC&mode,lgth);
			break;
		case 4:
			TripleDES(&pointer[0],&keyValue[0],&result_pointer[0],CBC&mode,lgth);			//2key
			break;
		case 8:	
			TripleDES_3Key(&pointer[0],&keyValue[0],&result_pointer[0],CBC&mode,lgth);          		//3key模式
			break;
		default:
			SingleDES(&pointer[0],&keyValue[0],&result_pointer[0],ECB&mode,lgth);
			break;
	}

   	return POP_OK;                                                     		//REQUEST COMPAREING THE DATA   
}




/*-------------------------------------------------------------------------------------------------------//

  Function name:	Findkey
 
  Input parameter:	keyType:	KIC: 0x00
  								KID: 0x80

 			        keyIndex:	the number of the key

  Output parameter: the buffer of the key
 
  Description:      this function read out the key by the key index and key type      

  status: 			need test
  
  author :          XuWeiHua      

 -------------------------------------------------------------------------------------------------------------------*/
BYTE Findkey(BYTE keyType, BYTE keyIndex, BYTE * dataBuffer,BYTE length)
{
	UWORD xdata fileAddr,keyFileAddr;
	BYTE  xdata result,i;
	BYTE  xdata buf[2],len;
	BYTE  xdata keyLgth,scId;
	WORD  idata fileOffset;


	keyFileAddr = SelectLFFile(DF_RFM, EF_WIBRFM_Key);

	keyLgth = GetLFRecNumLgth(keyFileAddr, FileLgth);					

	if(G_RFM_TAR == TRUE)						 //RFM下行时
	{
		scId = G_TempSc;
	}
	else
	{
		scId = 0x01;					//江苏写号应用,默认使用0x01
	}
		
	fileAddr = SelectTFFile(DF_RFM,EF_RFM_Sc);

	if(fileAddr == FALSE)
	{
		result = ReadLFBuf(keyFileAddr, keyIndex+1, 0x00, &dataBuffer[0], length==0?keyLgth:length);
		
		return result;
	}	

	if(keyType == KEY_KIC)
   	{	
		fileOffset = GetScTagOffset(fileAddr,scId,TAG_KICKEY_SET);                              // get the sc kic key set offset
	}
	else
	{                           
		fileOffset = GetScTagOffset(fileAddr,scId,TAG_KIDKEY_SET);  						    // get the sc kid key set offset	
	}

	if(fileOffset == FALSE)
	{
		return FALSE;
	}

	ReadTFBuf(fileAddr,fileOffset,(BYTE *)&buf[0],2); 

	len = buf[1];	

	for(i=0;i<(len/2);i++)
	{
		ReadTFBuf(fileAddr,fileOffset+2+2*i,(BYTE *)&buf[0],2);

		if(keyIndex == buf[0])    
		{
			keyIndex = buf[1];                     // Kic/Kid key reference

			break;
		}
	}

	if(i == (len/2))
	{
		return FALSE;
	}

	result = ReadLFBuf(keyFileAddr, keyIndex+1, 0x00, &dataBuffer[0], length==0?keyLgth:length);

	return result;

}



/*-------------------------------------------------------------------------------------------------------//

  Function name:	CalculateMacValue
 
  Input parameter:	
 			        lgth: from to the end
 
  Output parameter:     the number of tlv structure
 
  Description:          return the number that first tlv structure in input buffer 

  Status:		OK

 -------------------------------------------------------------------------------------------------------------------*/


BYTE CalculateMacValue(BYTE spi,BYTE kic,BYTE kid,UWORD eepAddr,BYTE * result_pointer,WORD lgth)
{
	BYTE idata result,i;
  	BYTE idata kicId,kidId,padNum;
	BYTE xdata keyContent[24];
	WORD xdata initLgth;

	if((spi & 0x03) == 0x01)                                  //rc,4byte
   	{
     	CRC32(eepAddr,&result_pointer[0],lgth);
	}
  	else if((spi & 0x03) == 0x02)                                 //cc,8byte
    { 
		if((kid & 0x03) == 0x00)                                  // 9797 MAC 4,9797 PAD 3
		{
			initLgth = lgth;
		}
				 	
  		padNum = lgth % 8 ;	

		if(padNum != 0x00)					           		 // if the data length is not N*8 byte.then must pad some data. 
		{
			SetEepromEX(eepAddr+lgth,0x00,(8-padNum));			

			lgth = 8*(lgth/8) + 8;
		}	

		kidId = (kid&0xF0)>>0x04;
		kicId = (kic&0xF0)>>0x04;						

	    if((kid & 0x03) == 0x01)                          			//DES
        { 
			result = Findkey(KEY_KID, kidId, &keyContent[0],0);

			if(result == FALSE)
			{
				return ERROR_CIPHERING;
			}

			if((kid&0x0C)==0x00)
			{				    
				DesMacFunction(eepAddr,&keyContent[0],result_pointer,lgth,SINGLEDES,TripleDES2Key);						   	
            }
			else if((kid&0x0C)==0x04)
			{			    
				DesMacFunction(eepAddr,&keyContent[0],result_pointer,lgth,TRIPLEDES,TripleDES2Key);
			}
			else if((kid&0x0C)==0x08)																				//添加支持TripleDES3Key模式
			{			    
				DesMacFunction(eepAddr,&keyContent[0],result_pointer,lgth,TRIPLEDES,TripleDES3Key);
			}
			else
			{
			    return ERROR_CIPHERING;
            }
		}
		else if((kid & 0x03) == 0x03)         // 9797 MAC 3
		{			
			result = Findkey(KEY_KID, kidId, &keyContent[0],8);

			if(result == FALSE)
			{
				return ERROR_CIPHERING;
			}

			result = Findkey(KEY_KIC, kicId, &keyContent[8],16);
		
		  	if(result == FALSE)
		  	{
			 	return ERROR_CIPHERING;
		  	}

			SmartISO9797Algorithm3Ee(eepAddr,&keyContent[0],result_pointer,lgth);   
		}
		else if((kid & 0x03) == 0x00)        // 9797 MAC 4
		{
			result = Findkey(KEY_KID, kidId, &keyContent[8],16);

			if(result == FALSE)
			{
				return ERROR_CIPHERING;
			}

			for(i=0;i<8;i++)
			{
				keyContent[i] = keyContent[i+16]^0xF0;      // generate K"
			}

			SmartISO9797Algorithm4Ee(eepAddr,&keyContent[0],result_pointer,lgth,initLgth);   
		}
		else
		{
			return ERROR_CIPHERING;
		}
	}
	else    //暂时不支持数字签名
	{
		return ERROR_CHECKSUM;
	}
 
    return POP_OK;
}



#ifdef _AES_ 
BYTE CalculateAESMacValue(BYTE kid,UWORD eepAddr,BYTE * result_pointer,WORD lgth)
{
  	BYTE idata result,i;
	BYTE idata n;
  	BYTE idata kidId;
	BYTE xdata keyContent[17];				//LV结构,暂只支持0x10长度的key
	BYTE xdata aesBuf[16];

	memset(&tempBuf[0],0x00,0x100);			//初始化全局变量
	
	kidId = (kid&0xF0)>>0x04;

    result = Findkey(KEY_KID, kidId, &keyContent[1],0);

	keyContent[0] = 0x10;				//设置Key的长度

	if(result == FALSE)
	{
		return ERROR_CIPHERING;
	}

	calcCMacInit(&keyContent[0], NULL, 0x10);

	n = lgth/CMAC_ONEBLOCK_SIZE;		

    if(lgth%CMAC_ONEBLOCK_SIZE!=0x00)
	{
		n = n+1;					   			//n表示分多少块计算
	}

    memset(&aesBuf[0],0x00,CMAC_BLOCKSIZE_MAX);		  	//初始化aesBuf为全0

	for(i=1;i<=n;i++)
	{	
		if(lgth > CMAC_ONEBLOCK_SIZE)
		{	
			ReadEeprom((BYTE *)&tempBuf[0],eepAddr+(i-1)*CMAC_ONEBLOCK_SIZE,CMAC_ONEBLOCK_SIZE);			
		}
		else
		{		
			ReadEeprom((BYTE *)&tempBuf[0],eepAddr+(i-1)*CMAC_ONEBLOCK_SIZE,lgth);
		}

		XorCalc(&aesBuf[0], &tempBuf[0], CMAC_BLOCKSIZE_MAX);	  //第一块时, aesBuf为全0

		if(i==n)
		{
		    calcCMacSegment(&tempBuf[0], lgth, &tempBuf[100], TRUE);

			
					
		}
		else
		{	 	
			calcCMacSegment(&tempBuf[0], CMAC_ONEBLOCK_SIZE, &tempBuf[100], FALSE);
		
			lgth = 	lgth - CMAC_ONEBLOCK_SIZE;

		}

		memcpy(&aesBuf[0],&tempBuf[100],CMAC_BLOCKSIZE_MAX);

	}

    memcpy(&result_pointer[0],&aesBuf[0],G_CheckSumLgth);
  
	return POP_OK;
}
#endif

/*--------------------------------------------------------------------------------------------------------------------
  Function name:	IncreatCNTR

  input parameter:	计数器的内容累加

  output parameter:	无

  description:		对计数器进行累加

  author:			Caibo

  testor

  status:
 -------------------------------------------------------------------------------------------------------------------*/

BYTE IncreatCNTR(BYTE cntr[5])
{
	BYTE idata i;
	WORD idata r;

	r = 0x0001;

	for(i=0x00;i<0x05;i++)
	{
		r = r + cntr[0x04-i];

		cntr[0x04-i] = r&0xFF;

		r = (r>>0x08)&0x0001;
	}

	return (r&0xFF);
}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

Function name :	   WIBCheckSecurityLevel                                                                                                          
                                                                                                                                                
input parameter:   spikickidBuf                                                             
  					                                                                                                         
output parameter:  ErrorBuffer/ErrorByteCode                                                                                                                
                                                                                                                                                
description:       check if the SPI KIC KID is in the incoming SPI_KIC_KID list
 
status:			   need Test
  
author:            XuWeiHua        

 -------------------------------------------------------------------------------------------------------------------*/    

BYTE WIBCheckSecurityLevel(BYTE * spikickidBuf)
{
	BYTE  idata i,msl;
	//BYTE  idata result;
	WORD  xdata fileOffset;
	UWORD xdata fileAddr;

	     
	/*添加MSL功能，by xwh 20120522
	*if SPI1.b2b1 is equal to or greater than MSLD.b2b1 and
	*if SPI1.b3 is equal to or greater than MSLD.b3; and
	*if SPI1.b5b4 is equal to or greater than MSLD.b5b4.
	*MSL功能优先于RFM_Sc,即MSL存在的情况下，只判断MSL是否通过，不判断RFM_Sc
	*/
	fileAddr = SelectTFFile(DF_RFM,EF_RFM_Msl);

	if(fileAddr != FALSE)
	{
		ReadTFBuf(fileAddr,0,&msl,sizeof(BYTE));
			
		if(msl!=0xFF) //FF表示无MSL
		{
			if(((spikickidBuf[0]&0x03)>=(msl&0x03))&&((spikickidBuf[0]&0x04)>=(msl&0x04))&&((spikickidBuf[0]&0x18)>=(msl&0x18)))
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}

	
	fileAddr = SelectTFFile(DF_RFM,EF_RFM_Sc);
	
	if(fileAddr == FALSE)
	{
		return TRUE;
	}
						
	spikickidBuf[2] = spikickidBuf[2]&0x0F;      // ignore the high nibble of KIC

	spikickidBuf[3] = spikickidBuf[3]&0x0F;      // ignore the high nibble of KID		

	fileOffset = GetScTagOffset(fileAddr,G_TempSc,TAG_INCOMING_SPIKICKID);                              // get the sc incoming SPI_KIC_KID offset

	if(fileOffset == FALSE)
	{
		return ErrorConfigurationError;
	}

	ReadTFBuf(fileAddr,fileOffset,(BYTE *)&RecData[0x3C],2);
                    
	ReadTFBuf(fileAddr,fileOffset+2,(BYTE *)&RecData[0x3E],RecData[0x3D]);	

	for(i=0;i<RecData[0x3D];i=i+4)
	{
		RecData[0x3E +2+i] = RecData[0x3E +2+i]&0x0F;      // ignore the high nibble of KIC

		RecData[0x3E +3+i] = RecData[0x3E +3+i]&0x0F;      // ignore the high nibble of KID

		if(memcmp(&spikickidBuf[0],&RecData[i+0x3E],4) == EQUAL)
		{
			return TRUE;
		}
	}

	return ErrorInvalidIncomingMessage;	 
	 
}


/*-------------------------------------------------------------------------------------------------------//

  Function name:	WIBCheckCounter
 
  Input parameter:	
 			        spi:
					counterPointer:
 
  Output parameter: true/ERROR_CNTR_LOW/ERROR_CNTR_HIGH/ERROR_CNTR_BLCED
 
  Description:      return the number that first tlv structure in input buffer 

  Status:			OK

 -------------------------------------------------------------------------------------------------------------------*/

BYTE WIBCheckCounter(BYTE spi,BYTE * counterPointer)           
{                                        
    UWORD idata fileAddr;
	BYTE  idata counterMode;			//counterPointer is the value that has been deciphered
	BYTE  idata counter[5];				//par spi is the high byte of SPI
	BYTE  idata acc,couterIndex,scId;
	WORD  idata indicator;
	

	if((G_RFM_TAR == TRUE)||(G_RFM_TAR == ErrorTAR))
	{
			scId = G_TempSc;
	}
	else
	{
			scId = 0x01;									//江苏写号应用,默认使用0x01
	}	
	  

	fileAddr = SelectTFFile(DF_RFM,EF_RFM_Sc);
		
	if(fileAddr != FALSE)						
	{
		indicator = GetScTagOffset(fileAddr,scId,TAG_COUNTERS);              // get the sc counters offset
				                
		ReadTFBuf(fileAddr,indicator+3,&couterIndex,0x01);  
	}
	else
	{
		couterIndex = 0x01;			  		//无SC文件7110/4E0A时默认用第一条counter
	} 

	fileAddr = SelectLFFile(DF_RFM,EF_RFM_Counter);		

  if(fileAddr == FALSE)
  {
		return ERROR_SECURY ;
  }
	
	ReadLFBuf(fileAddr, couterIndex, 0,&counter[0],sizeof(counter));	//get counter value from EF_COUNTER to counter[]

	acc = IncreatCNTR(&counter[0]);

	if(acc == 0x01)
	{
		return ERROR_CNTR_BLCED ;
	}

	counterMode = spi&0x18; 	 						 //get the bit 5-4 of the SPI high byte ,means countermode

	if(counterMode == 0x10)                             //SPI high byte ,bit5-4 = 10, higer is ok
	{
		if(memcmp_p(&counterPointer[0],&counter[0],sizeof(counter)) < EQUAL)        
		{
			return ERROR_CNTR_LOW;
		}
	}
	else if(counterMode == 0x18)					// SPI high byte ,bit5-4 = 11,only one higher
	{
		if(memcmp_p(&counterPointer[0],&counter[0],sizeof(counter) )> EQUAL)	// changed by yu jianguo
		{
			return ERROR_CNTR_HIGH;
		}
		else  //if(memcmp_p(&counterPointer[0],&counter[0],sizeof(counter)) < EQUAL)	 // changed by yu jianguo
		{
			if(memcmp_p(&counterPointer[0],&counter[0],sizeof(counter)) < EQUAL)	 // changed by yu jianguo
			{
				return ERROR_CNTR_LOW;
			}
		}

	}
	else  if(counterMode == 0x00)						//no counter available
	{
		return POP_OK  ;
	}
	else  // 08 : Counter available; no replay or sequence  checking  ,该功能暂未实现 
	{
		;
	}

	WriteLFBuf(fileAddr,couterIndex,0,&counterPointer[0],sizeof(counter));//write sms counter to the eeprom file

	return POP_OK  ;
}



/*--------------------------------------------------------------------------------------------------------------------                                                                                       
  Function name:	IsRFMReceiveAllWIB
  
  input parameter:	fileAddr

  					packageNum 

  output parameter:	收满所有返回TRUE，否则返回FALSE
                                                                                                                                         
  description:		把数据按照TLV的结构存入文件，文件的开始是数据类型（TLV）包括了批次、已收短信数目、短信总数、文件已用空间
  					
					以后的结构是按照短信的索引作为Tag存放的TLV结构

  author:			Zhuli

  Test:				Caibo
  
  status:		    OK	
 -------------------------------------------------------------------------------------------------------------------*/

BYTE IsRFMReceiveAllWIB(UWORD fileAddr, BYTE packageNum)
{
	BYTE  idata i, recNum, totalNum;

	recNum = LocalRFMTmpRecord(packageNum, 1);	                           //寻找第一条
	
	if(recNum == FALSE)
	{
		return FALSE;
	}
	
	ReadLFBuf(fileAddr, recNum, 0x02, &totalNum, sizeof(BYTE));
	
	if(totalNum == 0x00)
	{
		return FALSE;
	}	

	for(i=1; i<=totalNum; i++)
	{
		recNum = LocalRFMTmpRecord(packageNum, i);                       //判断是否每一条都收到

		if(recNum == FALSE)
		{
			return FALSE;
		}
	}

	return TRUE;	
}


/*--------------------------------------------------------------------------------------------------------------------                                                                                       
  Function name:	LocalRFMTmpRecord
  
  input parameter:	packageNum

  					index 

  output parameter:	收满所有返回TRUE，否则返回FALSE
                                                                                                                                         
  description:		把数据按照TLV的结构存入文件，文件的开始是数据类型（TLV）包括了批次、已收短信数目、短信总数、文件已用空间
  					
					以后的结构是按照短信的索引作为Tag存放的TLV结构

  author:			Zhuli

  Test:				Caibo
  
  status:			OK
 -------------------------------------------------------------------------------------------------------------------*/

BYTE LocalRFMTmpRecord(BYTE packageNum, BYTE index)
{
	UWORD idata fileAddr;
	BYTE  idata recNum, i, tmp[2];

	fileAddr = SelectLFFile(DF_STK, EF_WIBDownLoadFile);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	for(i=1; i<=recNum; i++)
	{
		ReadLFBuf(fileAddr, i, 0x00, &tmp[0], sizeof(tmp));

		if((tmp[0] == packageNum)&&(tmp[1] == index))
		{
			return i;
		}
	}

	return FALSE;
}


/*--------------------------------------------------------------------------------------------------------------------                                                                                       
  Function name:	StoreDataSms
  
  input parameter:	保存下载的RFM数据短信

  output parameter:	成功：	TRUE
					
					失败：	FALSE

					存满：  0xFF
                                                                                                                                         
  description:		对OTA RFM下行的处理

                    UDL  

  author:			XuWeiHua
  
  status:           need test			
 -------------------------------------------------------------------------------------------------------------------*/

BYTE StoreDataSms(BYTE * buf,BYTE refNumber,BYTE seqNumber,BYTE totalNumber)
{
	UWORD idata fileAddr;
	BYTE  idata recordNo,pos;
	BYTE  idata result,len;
	BYTE  idata recNum,i,tem[4];
			
	fileAddr = SelectLFFile(DF_STK, EF_WIBDownLoadFile);	

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	recordNo = FALSE;

	for(i=1;i<=recNum;i++)
	{
		ReadLFBuf(fileAddr,i,0,&tem[0],sizeof(tem));

		if((tem[0] == refNumber)&&(tem[1] == seqNumber)&&(tem[2] == totalNumber)&&(totalNumber != 0x01))
		{
			//return TRUE;
			/*收到重复的信息，则覆盖存储并继续处理，by XWH 20110829*/
			recordNo = i;
			break;
		}

		if(((tem[0] == 0x00)&&(tem[1] == 0x00)&&(tem[2] == 0x00)&&(tem[3] == 0x00)) || ((tem[0] == 0xFF)&&(tem[1] == 0xFF)&&(tem[2] == 0xFF)&&(tem[3] == 0xFF)))
		{
			if(recordNo == FALSE)
			{
				recordNo = i;
			}
		}
	}	

	if(recordNo == FALSE)
	{		
		for(i=1; i<=recNum; i++)
		{
			ReadLFBuf(fileAddr, i, 0x00, &tem[0], sizeof(BYTE));

			if(tem[0] == refNumber)
			{
				continue;
			}	
			
			result = IsRFMReceiveAllWIB(fileAddr, tem[0]);

			if(result == FALSE)
			{
				SetLFConst(fileAddr,i, 0x00, 0x00, 0x04);

				recordNo = i;   

				break;	
			}
		}

		/*若所有记录已满，清除所有记录以免阻塞，by XWH 20110829*/
		if(recordNo == FALSE)          
		{  	
			for(i=1; i<=recNum; i++)
			{				
				SetLFConst(fileAddr,i, 0x00, 0x00, 0x04);	
			}

			recordNo = 1;  
		}

	}

	if(buf[1] < 2)
	{
		return FALSE;
	}

	len = 3 + 1 + buf[0] - (buf[1] + 1);                  // length of refNumber,seqNumber,totalNumber,L,CPL to data end

	pos = buf[1] - 2;

	buf[pos] = refNumber;                                 // reference number

	buf[pos+1] = seqNumber;	                              // sequence number

	buf[pos+2] = totalNumber;                             // total number

	buf[pos+3] = len - 4;                                 // user data len	

	result = WriteLFBuf(fileAddr, recordNo, 0x00, (BYTE *)&buf[pos], len);

	if(result == FALSE)
	{
		return FALSE;
	}

	if(totalNumber == 0x01)	                              // 已收满
	{
		return 0xFF;
	}
  
	result = IsRFMReceiveAllWIB(fileAddr,refNumber);

	if(result == TRUE)
	{
		return 0xFF;
	}
    else
	{
		return TRUE;
	}
}


/*--------------------------------------------------------------------------------------------------------------------                                                                                       
  Function name:	ReadDataSms
  
  input parameter:	buf
	                refNumber
					seqNumber
			
  output parameter:	成功：	TRUE
					
					失败：	返回FALSE
                                                                                                                                         
  description:		读出保存的数据短信

                    UDL  

  author:			XuWeiHua
  
  status:           need test			
 -------------------------------------------------------------------------------------------------------------------*/

BYTE ReadDataSms(BYTE * buf,BYTE refNumber,BYTE seqNumber)
{
	UWORD idata fileAddr;
	BYTE  idata recordNo;
	BYTE  idata dataLen;
		
	fileAddr = SelectLFFile(DF_STK, EF_WIBDownLoadFile);	

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	recordNo = LocalRFMTmpRecord(refNumber,seqNumber);

	if(recordNo == FALSE)                                   
	{
		return FALSE;
	}

	ReadLFBuf(fileAddr,recordNo,3,&dataLen,sizeof(BYTE));	

	ReadLFBuf(fileAddr,recordNo,0,&buf[0],dataLen+4);	

	return recordNo;
}


/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	 WIBDwonLoadDecrypt                                                                                                                   
                                                                                                                                                
  input parameter:   fileAddr:	保存下行数据信息的文件地址
  
  					 dataAddr： 解密后的数据存储地址
					 
					 packageNum：解密的数据短信的批次                                                                    
  					                                                                                                         
  output parameter:  TRUE/FALSE                                                                                                                
                                                                                                                                                
  description:       save the information(Table 6 from page the 16ist /Table 7 from page the 17ist in 3GPP TS 03.48) in WIB_RFM
  
  author: 			 zhuli

  status:			 OK
---------------------------------------------------------------------------------------------------------------------------------------------------*/

BYTE WIBDwonLoadDecrypt(UWORD fileAddr,UWORD dataAddr, BYTE packageNum,BYTE totalNo)
{
	WORD idata usedSize, dataSize;
	BYTE idata recNum, i, result,udl;
	BYTE idata lgth, lastLgth;
                                    
	usedSize = 0;

	dataSize = GetTFSize(dataAddr);

	for(i=0x01; i<=totalNo; i++)
	{		
		recNum = ReadDataSms(&RecData[0],packageNum,i);

		if(recNum == FALSE)
		{
			return 0xFE;
		}
		   
		SetLFConst(fileAddr, recNum, 0x00, 0x00, 4);						//clear the sms temp file

		udl = RecData[3];                                                   // data length

		if(udl == 0xFF)
		{
			return ErrorInvalidIncomingMessage;			
		}

		if(udl == 0x00)
		{
			return TRUE;	
		}

		if((usedSize+udl+8) > dataSize)
		{
			return ErrorE2PROMMemoryProblem;			
		}								 

		if(i == 0x01)
		{
			memcpy(&G_WIBTag[0], &RecData[6],sizeof(G_WIBTag)); 		    //save from chl to tar			
			
			if((G_WIBTag[1]&0x03) == 0x01)						    
			{
				G_CheckSumLgth = 0x04;                              //RC, the result is 4byte
			}
			else if((G_WIBTag[1]&0x03) == 0x02)
			{
				if(((G_WIBTag[4]&0x03) == 0x01)||((G_WIBTag[4]&0x03) == 0x00))//CRC or 9797 mac4
				{
					G_CheckSumLgth = 0x08;                          
				}
#ifdef _AES_
				else if((G_WIBTag[4]&0x03) == 0x02)					//AES-MAC
				{
					G_CheckSumLgth = G_WIBTag[0] - 0x0D;
				}
#endif								
				else
				{
					G_CheckSumLgth = 0x04;
				}
			}
			else
			{
				G_CheckSumLgth = 0x00;
			}
		}

		if((G_WIBTag[1]&0x04) == 0x00) //原来的数据没有加密，直接保存。
		{
			if(i == 0x01)
			{
				memcpy(&G_WIBMac[0], (BYTE *)&RecData[20], G_CheckSumLgth);	                           // copy RC/CC to buffer

				memmove((BYTE *)&RecData[4+G_CheckSumLgth], (BYTE *)&RecData[4], 0x10);            // 将  CPL..PCNTR 后移G_CheckSumLgth个字节

				usedSize = udl - G_CheckSumLgth;

				WriteTFBuf(dataAddr, 0, (BYTE *)&RecData[4+G_CheckSumLgth], usedSize);	   // start from CPL, not include CC/RC
			}
			else
			{
				WriteTFBuf(dataAddr, usedSize, (BYTE *)&RecData[4], udl);

				usedSize = usedSize + udl;
			}
		}
#ifdef _AES_
		else if(((G_WIBTag[1]&0x03) == 0x02)&&((G_WIBTag[3]&0x03) == 0x02)) 		//AES加密
		{
			if(i == 1)
			{
				lgth = udl - 10;                    // 本条数据的长度 减去 CPL..TAR的长度

				lastLgth = lgth%16;                  //本条不是16的整数倍的长度，此数据不参与本次的解密

				if(lastLgth != 0x00)
				{
					lgth = lgth - lastLgth;
		
					memmove((BYTE *)&RecData[D_TmpBufStart2+16], (BYTE *)&RecData[udl+4-lastLgth], lastLgth);
				}
				
				//if((G_WIBTag[3]&0x0C)!=0x0C)				//表示是CBC mode,AES算法暂不需要此判断  
				//{
					memmove((BYTE *)&RecData[D_TmpBufStart2], (BYTE *)&RecData[udl+4-lastLgth-16], 16);
				//}

				result = ChipheRe(G_WIBTag[3],(BYTE *)&RecData[14],(BYTE *)&RecData[14],DECRYPT,lgth);

				if(result != POP_OK)
				{
					return ErrorInvalidIncomingMessage;					
				}

				memcpy(&G_WIBMac[0], (BYTE *)&RecData[20], G_CheckSumLgth);

				memmove((BYTE *)&RecData[4+G_CheckSumLgth], (BYTE *)&RecData[4], 0x10);							 // 将  CPL..PCNTR 后移G_CheckSumLgth个字节

				usedSize = udl - G_CheckSumLgth - lastLgth;

				WriteTFBuf(dataAddr,0,(BYTE *)&RecData[4+G_CheckSumLgth],usedSize);
			}
			else
			{
				lgth = udl + lastLgth; 							//本次数据的长度

				memmove((BYTE *)&RecData[4+lastLgth], (BYTE *)&RecData[4], udl); //将上次剩余的未解密的数据和该次数据拼在一起。						

				memcpy((BYTE *)&RecData[4], (BYTE *)&RecData[D_TmpBufStart2+16], lastLgth);

				lastLgth = lgth % 16;

				if(lastLgth != 0x00)
				{							
					lgth = lgth - lastLgth;
					
					memmove((BYTE *)&RecData[D_TmpBufStart2+16], (BYTE *)&RecData[4+lgth], lastLgth);
				}

                memcpy((BYTE *)&RecData[D_TmpBufStart2+32], (BYTE *)&RecData[4+lgth-16], 16);	 //将此次的原始数据的后16个字节放入BUFFER中。		

				result = ChipheRe(G_WIBTag[3],(BYTE *)&RecData[4],(BYTE *)&RecData[4],DECRYPT,lgth);	

				
				if(result != POP_OK)
				{
					return ErrorInvalidIncomingMessage;					
				}
				
							
				//if((G_WIBTag[3]&0x0C)!=0x0C)
				//{
					for(result=0; result<16; result++) 											//解密后的头16个字节要做异或操作。
					{
						RecData[4+result] = RecData[4+result] ^ RecData[D_TmpBufStart2+result];
					}

					memcpy((BYTE *)&RecData[D_TmpBufStart2], (BYTE *)&RecData[D_TmpBufStart2+32], 16);				
				//} 

				WriteTFBuf(dataAddr, usedSize, (BYTE *)&RecData[4], lgth);

				usedSize = usedSize + lgth;
			}
		
		}
#endif
		else
		{
			if(i == 1)
			{
				lgth = udl - 10;                    // 本条数据的长度 减去 CPL..TAR的长度(CPL2+CHL1+SPIKICKID4+TAR3)

				lastLgth = lgth%8;                  //本条不是8的整数倍的长度，此数据不参与本次的解密

				if(lastLgth != 0x00)
				{
					lgth = lgth - lastLgth;
		
					memmove((BYTE *)&RecData[D_TmpBufStart+8], (BYTE *)&RecData[udl+4-lastLgth], lastLgth);
				}
				
				if((G_WIBTag[3]&0x0C)!=0x0C)
				{
					memmove((BYTE *)&RecData[D_TmpBufStart], (BYTE *)&RecData[udl+4-lastLgth-8], 8);
				}

				result = ChipheRe(G_WIBTag[3],(BYTE *)&RecData[14],(BYTE *)&RecData[14],DECRYPT,lgth);


				memcpy(&G_WIBMac[0], (BYTE *)&RecData[20], G_CheckSumLgth);

				memmove((BYTE *)&RecData[4+G_CheckSumLgth], (BYTE *)&RecData[4], 0x10);							 // 将  CPL..PCNTR 后移G_CheckSumLgth个字节

				usedSize = udl - G_CheckSumLgth - lastLgth;

				WriteTFBuf(dataAddr,0,(BYTE *)&RecData[4+G_CheckSumLgth],usedSize);
						
				if(result != POP_OK)
				{
					return ErrorInvalidIncomingMessage;					
				}
			}
			else
			{
				lgth = udl + lastLgth; 							//本次数据的长度

				memmove((BYTE *)&RecData[4+lastLgth], (BYTE *)&RecData[4], udl); //将上次剩余的未解密的数据和该次数据拼在一起。						

				memcpy((BYTE *)&RecData[4], (BYTE *)&RecData[D_TmpBufStart+8], lastLgth);

				lastLgth = lgth % 8;

				if(lastLgth != 0x00)
				{							
					lgth = lgth - lastLgth;
					
					memmove((BYTE *)&RecData[D_TmpBufStart+8], (BYTE *)&RecData[4+lgth], lastLgth);
				}

				memcpy((BYTE *)&RecData[D_TmpBufStart+16], (BYTE *)&RecData[4+lgth-8], 8);	//将此次的原始数据的后8个字节放入BUFFER中。
			
				result = ChipheRe(G_WIBTag[3],(BYTE *)&RecData[4],(BYTE *)&RecData[4],DECRYPT,lgth);	

				if(result != POP_OK)
				{
					return ErrorInvalidIncomingMessage;					
				}
							
				if((G_WIBTag[3]&0x0C)!=0x0C)
				{
					for(result=0; result<8; result++) 											//解密后的头8个字节要做异或操作。
					{
						RecData[4+result] = RecData[4+result] ^ RecData[D_TmpBufStart+result];
					}

					memcpy((BYTE *)&RecData[D_TmpBufStart], (BYTE *)&RecData[D_TmpBufStart+16], 8);				
				} 

				WriteTFBuf(dataAddr, usedSize, (BYTE *)&RecData[4], lgth);

				usedSize = usedSize + lgth;
			}
		}
	}

	return TRUE;
}



/*------------------------------------------------------------------------------------------------------------                                                                                                                                               
  Function name :	BuildWORD                                                                                                                   
                                                                                                                                                
  input parameter:   void                                                                    
  					                                                                                                         
  output parameter:  void                                                                                                                 
                                                                                                                                                
  description:       This function manage the bookmark
  
  status: 			need test
  
  author :          XuWeiHua      
 --------------------------------------------------------------------------------------------------------------*/

WORD BuildWORD(BYTE buf[2])
{
	return ((((WORD)(buf[0]))<<8)&0xFF00)|(((WORD)(buf[1]))&0x00FF);
}


/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	 WIBDecryptANDCheck                                                                                                                   
                                                                                                                                                
  input parameter:   void                                                                   
  					                                                                                                         
  output parameter:  TRUE/FALSE                                                                                                                
                                                                                                                                                
  description:       下行的WIB的数据短信的执行,普通短信做RFM

  author: 			 zhuli

  status:			 OK
---------------------------------------------------------------------------------------------------------------------------------------------------*/
BYTE WIBDecryptANDCheck(BYTE refNumber)
{
	UWORD idata fileAddr;
	UWORD idata dataAddr,dataBodyAddr;
	BYTE  idata result;
	BYTE  idata recNum;
	WORD idata cPacketLgth;												//  CPL													
	BYTE xdata reslutCc[8];

	fileAddr = SelectLFFile(DF_STK, EF_WIBDownLoadFile);

	dataAddr = SelectTFFile(DF_STK, EF_WIBTempFile);
	
	dataBodyAddr = 	getFileContentAddr(dataAddr);

	if((fileAddr == FALSE)||(dataAddr == FALSE))
	{
		return 0xFE;
	}

	recNum = ReadDataSms(&RecData[0],refNumber,1);

	if(recNum == FALSE)
	{
		return 0xFE;
	}

    G_TarTypeMode = NormalMODE;	

	G_ReTarNo = G_TempTarNo;

	result = WIBCheckSecurityLevel(&RecData[7]);
	

	if(result == TRUE)
	{
		result = WIBDwonLoadDecrypt(fileAddr,dataAddr, refNumber,RecData[2]);

		if((G_RFM_TAR != TRUE)&&(G_VIMSI_TAR != TRUE))						 		
		{
			return ERROR_WRONGTAR;								
		}
		
		if(result == ErrorInvalidIncomingMessage)					
		{
			return ERROR_CIPHERING;
		}
	}
	else
	{					
		WIBDwonLoadDecrypt(fileAddr,dataAddr, refNumber,RecData[2]);				   //用于把解密后的数据放在文件EF_WIBTempFile中，在POR响应需要时使用
		
		if((G_RFM_TAR == TRUE)||(G_VIMSI_TAR == TRUE))
		{
			return ERROR_SECURY;		
		}
		else
		{
			result = ERROR_WRONGTAR;	
		}
		
		SetLFConst(fileAddr,recNum, 0x00, 0x00, 0x04);
		
		return result;			
	}

	ReadTFBuf(dataAddr, 0x00, (BYTE *)&RecData[200], 0x10);         // from CPL to PCTER

	cPacketLgth = BuildWORD(&RecData[200]);

	result = POP_OK;

	if((G_WIBTag[1]&0x03) != 0x00)   
	{
	 	if((G_WIBTag[4]&0x03)==0x02)  		//表示采用AES-CMAC校验码
		{
#ifdef _AES_ 			
			result = CalculateAESMacValue(G_WIBTag[4],dataBodyAddr,&reslutCc[0],(cPacketLgth-G_CheckSumLgth+0x02));// check the rc_cc_data
#endif		
		}
	    else
		{
			result = CalculateMacValue(G_WIBTag[1],G_WIBTag[3],G_WIBTag[4],dataBodyAddr,&reslutCc[0],(cPacketLgth-G_CheckSumLgth+0x02));// check the rc_cc_data
		}

	    if(result == POP_OK)
		{
			if(memcmp(&reslutCc[0],&G_WIBMac[0],G_CheckSumLgth) != EQUAL)
			{
				result = ERROR_CHECKSUM;
			}
		}
	}

	if(result == POP_OK)
	{
		result = WIBCheckCounter(G_WIBTag[1],(BYTE * )&RecData[210]); // changed by yu jianguo
	}   


	if(result == POP_OK)
	{
	    RecData[200] = ((cPacketLgth -G_CheckSumLgth-RecData[215])>>8) & 0xFF; 	
	
	    RecData[201] = (cPacketLgth -G_CheckSumLgth-RecData[215]) & 0xFF; //modify CPL - PCNTR - THE LENGTH OF MAC,for we have deleted the macCc data before
		
		WriteTFBuf(dataAddr, 0, (BYTE *)&RecData[200], 2);
		
		CommandStartNum = 0x03 + G_WIBTag[0]-G_CheckSumLgth;        //record the 1st command from whert to begining 

		return  0xFF;
	}
	else
	{
		if((G_RFM_TAR != TRUE)&&(G_VIMSI_TAR != TRUE))		
		{
			if(result != ErrorConfigurationError)
			{	
				result = ErrorInvalidIncomingMessage;
			}                                       				
		}

		return result;
	}
}


void VIMSIHandler(BYTE refNumber)
{
	BYTE idata result;

	result =  WIBDecryptANDCheck(refNumber);

	if(result == 0xFF)
	{
			
	
	}

}


void ClearTempFile(void)
{
	UWORD idata  dataAddr;

    dataAddr = SelectTFFile(DF_STK, EF_WIBTempFile);

	if(dataAddr == FALSE)
	{
		return;
	}

	SetTFConst(dataAddr,0x00,0xFF,0x10);		  	

}
