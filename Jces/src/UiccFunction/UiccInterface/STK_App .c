#include  "string.h"
#include "UploadInfoFunction.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#if UICC__APP__SIMINFO
u8 CONST cardSpace1[] = {//电话号码本存储容量
0x12,0x75,0x35,0x8B,0xDD,0x53,0xF7,0x78,0x01,0x67,0x2C,0x5B,0x58,0x50,0xA8,0x5B,0xB9,0x91,0xCF,
};
u8 CONST cardSpace2[] = {//条，已用电话本
0x0E,0x67,0x61,0xFF,0x0C,0x5D,0xF2,0x75,0x28,0x75,0x35,0x8B,0xDD,0x67,0x2C,
};
u8 CONST cardSpace3[] = {//条，短信存储容量
0x10,0x67,0x61,0xFF,0x0C,0x77,0xED,0x4F,0xE1,0x5B,0x58,0x50,0xA8,0x5B,0xB9,0x91,0xCF,
};
u8 CONST cardSpace4[] = {//条，已用短信
0x0C,0x67,0x61,0xFF,0x0C,0x5D,0xF2,0x75,0x28,0x77,0xED,0x4F,0xE1,
};
u8 CONST cardSpace5[] = {//条
0x02,0x67,0x61,
};


u8 CONST simInfoMenu[] = {
	0x0C,0x53,0x61,0x72,0x47,0x8B,0xE6,0x7E,0xC6,0x4F,0xE1,0x60,0x6F,
0x12,0x75,0x35,0x8B,0xDD,0x53,0xF7,0x78,0x01,0x53,0xCA,0x77,0xED,0x4F,0xE1,0x5B,0xB9,0x91,0xCF,//电话号码及短信容量
	0x10,0x53,0x61,0x72,0x47,0x8B,0xE6,0x7E,0xC6,0x4F,0xE1,0x60,0x6F,0x4E,0x0A,0x62,0xA5,
};

void WordToUCS2(u16 counter, u8 * dataBuf)
{
	u8 I_A i, j;

	i = 0x00;

	if(counter >= 10000)
	{
		j = 10;
	}
	else if(counter >= 1000)
	{
		j = 8;
	}
	else if(counter >= 100)
	{
		j = 6;
	}
	else if(counter >= 10)
	{
		j = 4;
	}
	else if(counter > 0)
	{
		j = 2;
	}
	else
	{
		dataBuf[0] = 0x02;

		dataBuf[1] = 0x00;

		dataBuf[2] = 0x30;

		return;
	}

	while(counter > 0)
	{
		dataBuf[j-i] = (counter%10) | 0x30;

		dataBuf[j-i-1] = 0x00;

		counter = counter / 10;

		i += 2;
	}

	dataBuf[0] = i;
}

/*------------------------------------------------------------------------------------
Function name    :    GetPhonebookNum

Input parameter  :   

Output parameter :    

Description      :     返回号码本数目   
--------------------------------------------------------------------------------------*/

u8 GetPhonebookNum(void)
{
	memref X_A fileAddr;
	u8 I_A recNum, count, i;
	
	fileAddr = SelectLFFile(DF_TELECOM ,0x5F3A);
	
	fileAddr =  SelectFile(fileAddr,0x4F30);  //查找5F3A下EFPBR的地址
 	
 	if(fileAddr == FALSE)
 	{
 		return FALSE;
 	}
 	
 	recNum = GetLFRecNumLgth(fileAddr, FileNum);
 	
 	for(i=1, count=0; i<=recNum; i++)
 	{
 		ReadLFBuf(fileAddr, i, 0, &RecData[0], 1);
 		
 		if((RecData[0]!=0x00)&&(RecData[0]!=0xFF))
 		{
 			count ++;
 		}
 	}
 	
 	return count;
}

/*------------------------------------------------------------------------------------
Function name    :    AdnorSmsSpace

Input parameter  :    type:0x01--ADN/type:0x02---SMS

Output parameter :    totalNum-记录总数

Description      :     返回ADN、SMS中的记录总数    
--------------------------------------------------------------------------------------*/

u16 AdnorSmsSpace(u8 type)
{
	memref X_A fileAddr;
	u16 I_A totalNum, efId;
	u8 I_A pbNum;
	
	totalNum = 0;
	
  if(type == 0x01)
  {
  	efId = 0x4F3A;
		
  	pbNum = GetPhonebookNum();
  }
  else
  {
  	efId = EF_SMS;
		
  	pbNum = 0;
  }
  
	do
	{
	  if(type == 0x01)
  	{
				fileAddr = SelectLFFile(DF_TELECOM,0x5F3A);
				
				fileAddr =  SelectFile(fileAddr,efId);  //查找5F3A下EFPBR的地址
	  }
	  else
	  {
	  	fileAddr = SelectLFFile(DF_TELECOM,efId);
	  }
		
		if(fileAddr == FALSE)
 	  {
 		  return FALSE;
 	  }
	  
	  totalNum += GetLFRecNumLgth(fileAddr,FileNum);
	  
		if(type==0x01)
		{
			efId++;
			
			pbNum--;
		}
	}while(type==0x01 && pbNum>0);
	
  return totalNum;
}


/*------------------------------------------------------------------------------------
Function name    :    AdnorSmsUsed

Input parameter  :    type:0x01--ADN/type:0x02---SMS

Output parameter :    counter-已用记录

Description      :     返回ADN、SMS中已用记录数      
--------------------------------------------------------------------------------------*/
u16 AdnorSmsUsed(u8 type)
{
  memref X_A fileAddr;
  u8 I_A length,nameLength,i,singleNum, pbNum;
  u16 I_A count, efId;

  count = 0;
  
  if(type == 0x01)
  {
  	efId = 0x4F3A;
		
  	pbNum = GetPhonebookNum();
  }
  else
  {
  	efId = EF_SMS;
		
  	pbNum = 0;
  }
  	
  do
	{
	if(type == 0x01)
	{
				fileAddr = SelectLFFile(DF_TELECOM,0x5F3A);
				
				fileAddr =  SelectFile(fileAddr,efId);  //查找5F3A下EFPBR的地址
	  }
	  else
	  {
	  	fileAddr = SelectLFFile(DF_TELECOM,efId);
	  }
		
		if(fileAddr == FALSE)
 	  {
 		  return FALSE;
 	  }

    singleNum = GetLFRecNumLgth(fileAddr,FileNum);            //记录数
  	 
    length = GetLFRecNumLgth(fileAddr,FileLgth);               //记录长度
    
    for(i=1; i<=singleNum; i++)                                
		{
			ReadLFBuf(fileAddr,i,0,&RecData[0],length);
			
			if((RecData[0]!=0x00)&&(RecData[0]!=0xFF))         //记录不为空
		  {
				count++;
	    }
	    else if(type==0x01)                                 //如果姓名为空，判断是否存在电话号码
	    {
        nameLength=length-0x0E;
        
        if((RecData[nameLength]!=0x01)&&(RecData[nameLength]!=0x00)&&(RecData[nameLength]!=0xFF))
        {  
					count++;
        }      
	    }
		}//end for
		
		if(type==0x01)
		{
			efId++;
			
			pbNum--;
		}
	}while(type==0x01 && pbNum>0);
	
  return count;
}


/*------------------------------------------------------------------------------------
Function name    :    CheckCardSpace

Input parameter  :    

Output parameter :    

Description      :     拼装电话号码及短信容量display内容
--------------------------------------------------------------------------------------*/

void CheckCardSpace(void)
{   
	u8 X_A temp[7];
	u16 I_A num;
	
	tempBuf[0] = 0;
	
	AddValue(&tempBuf[0],(u8 *)cardSpace1);
	
	num = AdnorSmsSpace(0x01);
	
	WordToUCS2(num,&temp[0]); 
	
	AddValue(&tempBuf[0],(u8 *)&temp[0]);
	
	AddValue(&tempBuf[0],(u8 *)cardSpace2);
	
	num = AdnorSmsUsed(0x01);
	
	WordToUCS2(num,&temp[0]); 
	
	AddValue(&tempBuf[0],(u8 *)&temp[0]);
	
	AddValue(&tempBuf[0],(u8 *)cardSpace3);
	
	num = AdnorSmsSpace(0x02);
	
	HexToUCS2(num,&temp[0]); 
	
	AddValue(&tempBuf[0],(u8 *)&temp[0]);
	
	AddValue(&tempBuf[0],(u8 *)cardSpace4);
	
	num = AdnorSmsUsed(0x02);
	
	HexToUCS2(num,&temp[0]); 
	
	AddValue(&tempBuf[0],(u8 *)&temp[0]);
	
	AddValue(&tempBuf[0],(u8 *)cardSpace5);
	
	cephDisplayTextSingle(0x81,UCS2,(u8 *)&tempBuf[0]);
}


void SimInfor(void)
{
  u8 I_A result;
  
	while(1)
	{       
		result = cephSelectItemSingle(0x00, UCS2, NULL, (u8 *)simInfoMenu, sizeof(simInfoMenu));
		
		if((result == BackMove)||(result == ExitStkPerform))
		{
			return ;
		}
	
		result = GetResponseItemIdentifier();
	
    switch(result)
    {
    	case 1:
    		DisplayCardInfo();
	      break;
			
	    case 2:
	    	CheckCardSpace();                  //  拼装电话号码及短信容量display内容
	    	break;
		
			case 3:
			UploadCardInfo();
			break;
	    default:
	    	return;
	  }
  }
}
#endif
