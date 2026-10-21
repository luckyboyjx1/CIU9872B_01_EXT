/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
            V1.1 修改函数ComputeBERTLVFile，修改SFI的范围只允许为01-30
			V1.2 修改函数RetrieveData，修改当P3为0x00时，取数据错误 1.6->1.7
			V1.3 修改函数MoreDataAvailable，此时G_CurrenResponse[G_ChannelNum] 的响应长度为0x100 @20110806 by xuyz
			V1.4 定义宏BERTLVCmd，将BER-TLV结构的处理放入该宏内 --@20111027 by chenl


 */
#include "uicc_main.h"		   
#include "uicc_error.h"
#include "fileSystem.h"
#include "fileAction.h"
#include "gsmpublic.h"
#include <string.h>
#include "stkhead.h"
#include "Authentication.h"
#include "uicc_interface.h"
#include "DataOrientedCommand.h"

#if UICC__FUNCTION__BERTLVCmd

/*
 *@description :this function is used to read buf from a BER-TLV EF
 *
 *@return      :TRUE/FALSE
 *
 *@history     :create by xyz@090913 10:25
*/
BYTE ReadBERBuf(UWORD fileAddr, WORD offset, BYTE * buf, BYTE len)
{
	WORD   idata fileSize;

	fileSize = GetFileHeadWord(fileAddr,F_SpaceSize_Offset);

	if((offset+len) > fileSize) 
	{
		return FALSE;
	}

	fileAddr = fileAddr+sizeof(BERTLVFileHead);

	ReadEEPromCache(&buf[0],fileAddr+offset,len);

	return TRUE ;	
}
/*
 *@description :this function is used to write a buf to a BER-TLV EF
 *
 *@return      :TRUE/FALSE
 *
 *@history     :create by xyz@090913 10:25
*/
BYTE WriteBERBuf(UWORD fileAddr, WORD offset, BYTE * buf, BYTE len)
{
	WORD  idata fileSize;

	fileSize = GetFileHeadWord(fileAddr,F_SpaceSize_Offset);

	if ((offset+len) > fileSize) 
	{
		return FALSE;
	}

	fileAddr = fileAddr+ sizeof(BERTLVFileHead);

	WritePageCacheEX(fileAddr +offset,&buf[0],len);

	return TRUE ;	
}
/*
*/
void MoveBERTLVTagContent(UWORD fileAddr)
{
    BYTE idata num;
    WORD idata allLength,anotherLength,usedSize;
  
    allLength = FindBERTLVTagAtOffset(fileAddr,G_BERTLV_CurrentPointer[G_ChannelNum]-1,0x01,0x00);

    if(G_BERTLV_OperateTag[G_ChannelNum] == 0xDB)  //前次进行的是Set Data操作
    {
            if(G_BERTLV_CurrentOffset[G_ChannelNum] != FALSE)  //上次set未成功, 对文件中未成功的tag进行整理
            {
                    anotherLength = 0x0000;
            
					usedSize = GetFileHeadWord(fileAddr, F_BERTLV_UsedSize);	
                    
                    for(num = 0x00;num < 4;num++)
                    {
                            if(num != G_ChannelNum)
                            {
                                    if(G_BERTLV_OperateTag[num] == 0xDB)  // another channel is setting data
                                    {
                                            if(G_CurrentEfAddr[num] == fileAddr)
                                            {
                                                    if((G_BERTLV_CurrentPointer[num] != FALSE)&&(G_BERTLV_CurrentOffset[num] != FALSE))  //当前指针指向某tag ,且还未完成操作
                                                    {
                                                            anotherLength += FindBERTLVTagAtOffset(fileAddr,G_BERTLV_CurrentPointer[num]-1,0x01,0x00);					
                                                    }
                                            }
                                    }
                            }
                    }

                    SecureMoveEeprom(fileAddr+sizeof(BERTLVFileHead)+G_BERTLV_CurrentPointer[G_ChannelNum]-1,fileAddr+sizeof(BERTLVFileHead)+G_BERTLV_CurrentPointer[G_ChannelNum]-1+allLength,(usedSize+anotherLength-G_BERTLV_CurrentPointer[G_ChannelNum]+1));
                    
                    for(num=0;num<4;num++)   //上次set未成功，则修改another  channel ‘s tag pointer,offset
                    {
                            if((G_BERTLV_OperateTag[num] != FALSE)&&(num != G_ChannelNum))
                            {
                                    if(G_CurrentEfAddr[num] == fileAddr)   //another channel is operate the same file
                                    {
                                            if(G_BERTLV_CurrentPointer[num] > G_BERTLV_CurrentPointer[G_ChannelNum])
                                            {
                                                    G_BERTLV_CurrentPointer[num] = G_BERTLV_CurrentPointer[num] - allLength;
                                                    
                                                    G_BERTLV_LastOffset[num] = G_BERTLV_LastOffset[num] - allLength;
                                                    
                                                    if(G_BERTLV_CurrentOffset[num] != FALSE)
                                                    {
                                                            G_BERTLV_CurrentOffset[num] = G_BERTLV_CurrentOffset[num] -allLength;
                                                    }  
                                            }
                                    }
                            }
                    }
            }
        }	
}
/*
 *@description  :this function is used to release the object  while reselecting the file or selecting a different file
 *
 *@param        :fileAddr :the current Selected file
 *
 *@history      :create by xyz@090921
*/
void InitBERTLVTagIndicator(void)
{
    if(G_PreviousEfAddr[G_ChannelNum] != FALSE)	   //该全局变量只在BER-TLV文件中有操作，因此，如果有初始值，则文件一定在BER-TLV类型
    {
		MoveBERTLVTagContent(G_PreviousEfAddr[G_ChannelNum]);	
    }
      	
	G_BERTLV_CurrentPointer[G_ChannelNum] = FALSE;
				
	G_BERTLV_OperateTag[G_ChannelNum] = FALSE;
	
	G_BERTLV_LastOffset[G_ChannelNum] = FALSE;
	
	G_BERTLV_CurrentOffset[G_ChannelNum] = FALSE;

	G_PreviousEfAddr[G_ChannelNum] = FALSE;
}

/*
 *@description   :  this function is used to find a BER-TLV tag in file
 *
 *@param          :fileAddr: the EF fileAddr
 *
 *@return         :FALSE:find the tag and set the current tag pointer
 *@return         :TRUE  :not find the tag
 *
 *@history        :create by xuyz@090915 10:42
*/
BYTE FindBERTLVTagInFile(UWORD fileAddr,BYTE commpare)
{	
	WORD idata length,offset;
	WORD idata usedSize;

	usedSize = GetFileHeadWord(fileAddr, F_BERTLV_UsedSize);
	
	for(offset = 0x00;offset < usedSize;offset += length)
	{
		length = FindBERTLVTagAtOffset(fileAddr,offset,0x00,commpare);
		
		if(length == FALSE)    //查找到待查找的tag
		{
			return FALSE;
		}
		else if(length == TRUE)  //length error or not supported
		{
			return TRUE;
		}
	}
	
	return TRUE;
}

/*
 *@description    :this function is used to find the tag of a BER-TLV data objecr in current offset
 *
 *@param            :fileAddr : the fileAddr of the BER-TLV structure EF
 *                       :offset   :the finded offset
 *                       :p[0]      :save the length of tag value fields and length value fields
 *                       :mode     : 0x00-查找/0x01-获取长度
 *
 *@return           :the moved length of next step/if find the tag ,set current tag pointer and return FALSE
 *
 *@history          :create by xyz @090915 9:00
*/
WORD  FindBERTLVTagAtOffset(UWORD fileAddr,WORD subOffset,BYTE mode,BYTE commpare)
{
	BYTE idata m,n;
	BYTE xdata tag[3],size[3];
	WORD idata subLength;
	
	ReadBERBuf(fileAddr,subOffset,&tag[0],sizeof(tag));
				
	m = GetTagLgth(&tag[0]);

	if((commpare== m)&&(mode == 0x00))
	{
		if(ecpMem_Compare(&RecData[0],&tag[0],commpare) == EQUAL)   //找到待操作的tag
		{
			G_BERTLV_CurrentPointer[G_ChannelNum] = subOffset + 1;			
			
			return FALSE;
		}
	}
	
	ReadBERBuf(fileAddr,subOffset+m,&size[0],sizeof(size));
	
	if((size[0]&0x80) == 0x00)
	{
		n = 0x01;
		subLength = size[0];
	}
	else if((size[0]&0x83) == 0x81)
	{
		n = 0x02;
		subLength = size[1];
	}
	else if((size[0]&0x83) == 0x82)
	{
		n= 0x03;
		subLength = ((size[1]<<8)&0xFF00)+size[2];
	}
	else                                       //length error or not supported
	{
		return TRUE;
	}
	
	return (subLength+m+n);
}

/*
 *@description      :delete a specified tag in file
 *
 *@param             :fileAddr
 *
 *@returns         :
 *
 *@history          :create by xyz@090915 10:47
*/
void DeleteBERTLVTag(UWORD fileAddr)
{
	BYTE idata result;
	WORD idata usedSize;
	WORD idata length,len;
	UWORD idata des,src;
	
	result = FindBERTLVTagInFile(fileAddr,P3);
	
	if(result == TRUE)
	{
		return ;
	}
	
	usedSize = GetFileHeadWord(fileAddr, F_BERTLV_UsedSize);
	
	length = FindBERTLVTagAtOffset(fileAddr,G_BERTLV_CurrentPointer[G_ChannelNum]-1,0x01,P3);
	
	des = fileAddr + sizeof(BERTLVFileHead)+G_BERTLV_CurrentPointer[G_ChannelNum]-1;
	
	src = fileAddr + sizeof(BERTLVFileHead) +G_BERTLV_CurrentPointer[G_ChannelNum]-1+length;
	
	len = usedSize - (G_BERTLV_CurrentPointer[G_ChannelNum]-1+length);
	
	if(len != 0x0000)
	{	
		SecureMoveEeprom(des,src,len);
	}
	
	usedSize = usedSize - length;
	
	WritePageCacheEX(fileAddr+ F_BERTLV_UsedSize,(BYTE  *)&usedSize,sizeof(WORD));
	
	G_BERTLV_CurrentPointer[G_ChannelNum] = FALSE;
}

/*
 *@description     : this function is used to create a new tag in file
 *
 *@param            :fileAddr :the current EF addr
 *
 *@return          :TRUE:success\FALSE:no enough space
 *
 *@histroy          :create by xyz @090915 11:16
*/
BYTE CreateReplaceBERTLVTag(UWORD fileAddr)
{
	WORD idata offset;
	WORD idata currentLength,allLength;
	WORD idata usedSize;
	
	if((P2&0xC0) == 0x40)    //retransmit previous block
	{
		offset =  G_BERTLV_LastOffset[G_ChannelNum] ;
	}
	else
	{
		offset = G_BERTLV_CurrentOffset[G_ChannelNum];
	}	

	currentLength = offset-G_BERTLV_CurrentPointer[G_ChannelNum] + P3;
	

	allLength = FindBERTLVTagAtOffset(fileAddr,G_BERTLV_CurrentPointer[G_ChannelNum]-1,0x01,0x00);	
	
	if(currentLength > allLength)   //P3 ERROR 6700
	{	
		return FALSE;
	}	
	else
	{
		 WriteBERBuf(fileAddr,offset-1,&RecData[0],P3);	

		 if((P2&0xC0) != 0x40)                    //next block or first block
		 {
		 	  G_BERTLV_LastOffset[G_ChannelNum] = G_BERTLV_CurrentOffset[G_ChannelNum];
				
		 	  if(currentLength == allLength)  // 90 00
			  {	
					G_BERTLV_CurrentOffset[G_ChannelNum] = 0x0000;
					
					G_PreviousEfAddr[G_ChannelNum] = FALSE;
					
					usedSize = GetFileHeadWord(fileAddr, F_BERTLV_UsedSize);
					
					usedSize += allLength;
					
					WritePageCacheEX(fileAddr+F_BERTLV_UsedSize,(BYTE *)&usedSize,sizeof(WORD));
			  }
			  else	                      //63 F1
			  {		
					G_BERTLV_CurrentOffset[G_ChannelNum] += P3;
			  }
		 }

		  if(currentLength == allLength)  // 90 00
		  {
			   return 0x01;
		  }
		  else
		  {
		  	   return 0x02;
		  }
	}  
}
/*
 *@Description    :this function is used to get the list of tags
 *
 *@param          :fileAddr:the EF 
 *
 *
 *@history        :create by xyz@090923
*/
BYTE GetTopListFromBERTLVFile(UWORD fileAddr)
{
	BYTE idata n,num;
	BYTE xdata tag[3];
	WORD idata length,offset,anotherLength;
	WORD idata usedSize;
	
	usedSize = GetFileHeadWord(fileAddr, F_BERTLV_UsedSize);

	anotherLength = 0x0000;

	for(num = 0;num<4;num++)
	{
		if(num != G_ChannelNum)
		{
			if(G_BERTLV_OperateTag[num] == 0xDB)  //当前another application 正在进行set data
			{
				if(G_CurrentEfAddr[num] == fileAddr)
				{
					anotherLength += FindBERTLVTagAtOffset(fileAddr,G_BERTLV_CurrentPointer[num]-1,0x01,0x00);
				}
			}
		}
	}
	
	for(offset = 0x00,num=0x03;offset < usedSize+anotherLength;offset += length,num=num+n)
	{
		length = FindBERTLVTagAtOffset(fileAddr,offset,0x01,0x00);

		ReadBERBuf(fileAddr,offset,&tag[0],sizeof(tag));

		n = GetTagLgth(&tag[0]);

		ecpMem_Copy(&RecData[num],&tag[0],n);
	}

	RecData[0] = 0x5C;

   	num = num - 3;																

	if(num > 0x80)
	{
		RecData[1] = 0x81;

		RecData[2] = num;

		num = num + 3;
	}
	else
	{
		RecData[1] = num;

		ecpMem_Copy((BYTE xdata *)&RecData[2],(BYTE xdata *)&RecData[3],num);

		num = num + 2 ; 
	} 

	return num;
}

/*
 *@description:	 this function is used to compute the condition
 *
 *@action     :   retrieve/set
 *
 *#history    :create by xyz
*/
BYTE ComputeBERTLVFile(BYTE action)
{
	BYTE idata tag;

	if(P1 != 0x00)
	{
		P1P2Error();
		return FALSE;
	}
	
	if((P2&0xC0) == 0xC0)       //this value is RFU
	{
		P1P2Error();
		return FALSE;
	}
	
	if(((P2&0x80) == 0x00)&&((P2&0x1F) != 0x00))     //P2 indicating "Next Block" and "Current File"
	{
		P1P2Error();
		return FALSE;
	}

	if((P3 > 0x03)&&(action == A_ReadFile)&&((P2&0xC0) == 0x80))                 // the length of the tag value is three 
	{
		IncorrentParameter();             // 6A 80
		return FALSE;
	}

	if((P2&0x1F) != 0x00)                                        //SFI
	{
		tag = P2&0x1F;
		
		if(tag == 0x1F)
		{
			P1P2Error();
			return FALSE;
		}	
	}
	else
	{
		tag = 0x00;	
	}	

	tag = ComputeAll(action,tag,2);	

	if(tag ==FALSE) 
	{
		return FALSE;
	}

	if((P2&0x80) == 0x00)	 //next block or retransmit previous block
	{					
		if(G_BERTLV_OperateTag[G_ChannelNum] != INS)                //if P2 indicating "Next Block"  and a data object is allocted
		{
			P1P2Incorrect();                                                              //6A 86
			return FALSE;
		}

		if((G_BERTLV_CurrentOffset[G_ChannelNum] == FALSE)&&((P2&0xC0) == 0x00))                 //P2 indicate "NEXT BLOCK" and retrieve data or set data successfully
		{
			P1P2Incorrect();                                                              //6A 86
			return FALSE;
		}

	}

	if((P2&0xC0) == 0x80)	   //first block
	{
        MoveBERTLVTagContent(G_CurrentEfAddr[G_ChannelNum]);
	}
	return TRUE;
}

 /*
 *@description:判断当前输入tag是否有效
 */
BYTE GetAvailableTag(void)
{
	if((RecData[0] < 0x80) ||(RecData[0] > 0xBF))
	{
		IncorrentParameter();             // 6A 80
		return FALSE;
	}
	
	if((RecData[0]&0xDF) == 0x9F)  //9F 1F-9F 7F OR BF 1F -BF 7F
	{
		if((RecData[1] < 0x1F)||(RecData[1] == 0x80))
		{
			IncorrentParameter();             // 6A 80
			return FALSE;
		}
	}
	
	if((RecData[0]&0xDF) == 0x9F) 
	{
		if((RecData[1] &0x80) == 0x80)
		{
			if(RecData[2] > 0x7F)
			{
				IncorrentParameter();             // 6A 80
				return FALSE;
			}
		}
	}

	return TRUE;
}
/*
*@description: 获取当前tag名称的长度
*
*@param      :tag:BER-TLV tag
*
*@return     :tag length
*/
BYTE GetTagLgth(BYTE *tag)
{
	BYTE idata n;

	if((tag[0]&0x1F) != 0x1F)   //only a single tag byte
	{
		n= 0x01;
	}
	else
	{	
		if((tag[1]&0x80) == 0x00)      // two tag fileds
		{
			n= 0x02;
		}
		else                           //three tag fields
		{
			n= 0x03;
		}			
	} 

	return n;
}
 /*
  *descriptiom      :this function is used to retrieve a data object from the current BER-TLV struvture EF
  *
  *
  *@command type    :8x cb 00 	92 lc CommandData Le
  *
  *@history         :
 */
 void RetrieveData(void)
 {
 	BYTE idata num;
   	BYTE idata tag,result;
	WORD idata allLength,lgth;
	WORD idata currentLength,currentOffset;

	tag = ComputeBERTLVFile(A_ReadFile);

	 if(tag == FALSE)
	{
		return;
	}
	
	if((P2&0xC0) == 0x80)	           //tag value of TLV object shall ber retrieved
	{
		ReceiveData();

		if((RecData[0] == 0x5C)&&(P3 == 0x01))	  //get the list of tags already allocated in a file
		{
			G_BERTLV_OperateTag[G_ChannelNum] = INS;	//未考虑当前tag数目超过一个APDU所表示的长度		

			num = GetTopListFromBERTLVFile(G_CurrentEfAddr[G_ChannelNum]);

			CommandFinish(num);
		}
		else
		{
		   	if(GetAvailableTag() == FALSE)
			{
				IncorrentParameter();             // 6A 80
				return;
			}
			for(num = 0x00;num < 4;num++)          //another channel is setting data ,can't retrieve it
			{
				if((num != G_ChannelNum)&&(G_BERTLV_OperateTag[num] == 0xDB))
				{
					if(G_CurrentEfAddr[num] == G_CurrentEfAddr[G_ChannelNum])
					{	
						if(FindBERTLVTagAtOffset(G_CurrentEfAddr[G_ChannelNum],G_BERTLV_CurrentPointer[num]-1,0x00,P3) == FALSE )
						{
							ConditionSatisfied();
							return;	
						}
						
		
					}
				}
			}
			result = FindBERTLVTagInFile(G_CurrentEfAddr[G_ChannelNum],P3);

			if(result == TRUE)
			{
				ReferencedDataNotFound();
				return;
			}
		
			G_BERTLV_OperateTag[G_ChannelNum] = INS;			

			G_BERTLV_LastOffset[G_ChannelNum] = G_BERTLV_CurrentPointer[G_ChannelNum];
			
			G_BERTLV_CurrentOffset[G_ChannelNum] = G_BERTLV_CurrentPointer[G_ChannelNum];		
	
			allLength = FindBERTLVTagAtOffset(G_CurrentEfAddr[G_ChannelNum],G_BERTLV_CurrentPointer[G_ChannelNum]-1,0x01,0x00);

			if(allLength <= 0x100)
			{
				G_BERTLV_CurrentOffset[G_ChannelNum] = 0x0000;
			}
			else
			{
				G_BERTLV_CurrentOffset[G_ChannelNum] += 0x100;
			}
							
			if(allLength < 0x100)
			{				
				ReadBERBuf(G_CurrentEfAddr[G_ChannelNum],G_BERTLV_CurrentPointer[G_ChannelNum]-1,&RecData[0],allLength);
				
				CommandFinish(allLength);
			}
			else //if(allLength >= 0x100)
			{
				ReadBERBuf(G_CurrentEfAddr[G_ChannelNum],G_BERTLV_CurrentPointer[G_ChannelNum]-1,&RecData[0],0x80);

				ReadBERBuf(G_CurrentEfAddr[G_ChannelNum],G_BERTLV_CurrentPointer[G_ChannelNum]-1+0x80,&RecData[0x80],0x80);
		
				MoreDataAvailable();
			}
		}
	
	}
	else                //next block 中le为0x00
	{		
		allLength = FindBERTLVTagAtOffset(G_CurrentEfAddr[G_ChannelNum],G_BERTLV_CurrentPointer[G_ChannelNum]-1,0x01,0x00);  //获取当前tag的全部长度
	
		if((P2&0xC0) == 0x40)   //retransmit previous block   
		{
			currentOffset =  G_BERTLV_LastOffset[G_ChannelNum] ;
		}			
		else 
		{
			currentOffset = G_BERTLV_CurrentOffset[G_ChannelNum];
		}
		lgth = P3;

		if(P3 == 0x00)
		{
			lgth = 0x100;
		}
		currentLength = currentOffset - G_BERTLV_CurrentPointer[G_ChannelNum] + lgth;

		if(currentLength <= allLength)
		{
			if(P3 == 0x00)
			{
				ReadBERBuf(G_CurrentEfAddr[G_ChannelNum],currentOffset-1,&RecData[0],0x80);
	
				ReadBERBuf(G_CurrentEfAddr[G_ChannelNum],currentOffset-1+0x80,&RecData[0x80],0x80);
			}
			else
			{
				ReadBERBuf(G_CurrentEfAddr[G_ChannelNum],currentOffset-1,&RecData[0],P3);
			}
		}

		if((P2&0xC0) == 0x00)  //Next Block
		{	
			G_BERTLV_LastOffset[G_ChannelNum] = G_BERTLV_CurrentOffset[G_ChannelNum];
	
			if(currentLength < allLength)
			{
				G_BERTLV_CurrentOffset[G_ChannelNum] += lgth;
			}
			else
			{
				G_BERTLV_CurrentOffset[G_ChannelNum] = 0x0000;
			}
		}
		if(currentLength < allLength)
		{
			 SendData();
			
			 MoreDataAvailable();
		}
		else if(currentLength > allLength)
		{
			if(P3 != 0x00)			//通过P3  = 0x00 来获取剩余数据长度
			{	
				P3Error();

				return;
			}
			ReadBERBuf(G_CurrentEfAddr[G_ChannelNum],currentOffset-1,&RecData[0],0x100+allLength-currentLength);
			
			CommandFinish(0x100+allLength-currentLength);	
		}
		else
		{	 		
			SendData();

			CommandFinish(0x00);
		} 
	}
 }
 
/*
 *@description        :         this function is used to create a new data object or replace an already existing data object or delete a data object in current BER-TLV structure EF
 *
 *@command type    :          8x DB 00 P2 P3   data
 *
 *@history            :           create by xyz @090914 9:53
*/
void SetData(void)
{
	BYTE idata tag,result,i,num;
	BYTE xdata tagBer[3];
	WORD idata allLength,anotherLength,length;
	WORD idata usedSize,maxSize;
	WORD idata backPointeroffset;
	
	if(P3 == 0x00)  //setdata时不允许P3为0
	{
		P3Error();
		return;
	}
	
	tag = ComputeBERTLVFile(A_UpdateFile);
	
	if(tag == FALSE)
	{
		return;
	}	

	ReceiveData();

	i = 0x00;
	
	if((P2&0xC0) == 0x80)
	{
		if(GetAvailableTag() == FALSE)
		{
			IncorrentParameter();             // 6A 80
			return;
		}
		i = GetTagLgth(&RecData[0]);
			
		for(num = 0x00;num < 4;num++)
		{
			if((G_BERTLV_OperateTag[num] != FALSE)&&(num != G_ChannelNum))  //retrieve or set
			{
				if(G_CurrentEfAddr[num] == G_CurrentEfAddr[G_ChannelNum])
				{	
					ReadBERBuf(G_CurrentEfAddr[G_ChannelNum],G_BERTLV_CurrentPointer[num]-1,&tagBer[0],sizeof(tagBer));
					
					if(ecpMem_Compare((BYTE *)&tagBer[0],(BYTE *)&RecData[0],i) == EQUAL)
					{
						ConditionSatisfied();
						return;
					}
					
	
				}
			}
		}
	}


	if(P3 == i)             //delete a data object
	{		
		G_BERTLV_OperateTag[G_ChannelNum] = INS;
	
		DeleteBERTLVTag(G_CurrentEfAddr[G_ChannelNum]);
			
		G_BERTLV_OperateTag[G_ChannelNum] = FALSE;
		
		CommandFinish(0x00);
	}
	else                 //create or replace a tag 
	{
		if((P2&0xC0) == 0x80)     //First Block
		{	
			num = i;      //备份当前tag长度

			length = 0x00;
			
			if((RecData[i]&0x80) == 0x00)
			{
				length = RecData[i];
				i+= 0x01;
			}
			else if((RecData[i]&0x83) == 0x81)
			{
				length = RecData[i+1];
				i += 0x02;
			}
			else if((RecData[i]&0x83) == 0x82)
			{
				length = ((RecData[i+1]<<8)&0xFF00)+RecData[i+2];
				i +=  0x03;
			}
			
			if((WORD)P3>(length+i))
			{	
				P3Error();
				return;
			}
						
			allLength = 0x0000;
			
			backPointeroffset = G_BERTLV_CurrentPointer[G_ChannelNum];    //备份当前tag 指针
			
			result = FindBERTLVTagInFile(G_CurrentEfAddr[G_ChannelNum],num);
			
			if(result == FALSE)  //find the tag,replace it
			{			
				allLength = FindBERTLVTagAtOffset(G_CurrentEfAddr[G_ChannelNum],G_BERTLV_CurrentPointer[G_ChannelNum]-1,0x01,0x00);
			}
			
			usedSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_BERTLV_UsedSize);
			
			anotherLength = 0x0000;
			
			for(num = 0x00;num < 4;num++)
			{
				if(num != G_ChannelNum)
				{
					if(G_BERTLV_OperateTag[num] == 0xDB)  // another channel is setting data
					{
						if(G_CurrentEfAddr[num] == G_CurrentEfAddr[G_ChannelNum])
						{	
							if(G_BERTLV_CurrentPointer[num] != FALSE)  //当前指针指向某tag
							{
								anotherLength += FindBERTLVTagAtOffset(G_CurrentEfAddr[G_ChannelNum],G_BERTLV_CurrentPointer[num]-1,0x01,0x00);					
							}
						}
					}
				}
			}
							
			maxSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_SpaceSize_Offset);	//issue xiaoyh
			
			if((maxSize-usedSize-anotherLength+allLength)<(length+i))          //6A 84
			{
				G_BERTLV_CurrentPointer[G_ChannelNum] = backPointeroffset;  //还原当前备份tag 指针
				 
				NoEnoughSpace();
				return;
			}
			if(result == FALSE)
			{
				usedSize = usedSize - allLength ;
						
				SecureMoveEeprom(G_CurrentEfAddr[G_ChannelNum]+sizeof(BERTLVFileHead)+G_BERTLV_CurrentPointer[G_ChannelNum]-1,G_CurrentEfAddr[G_ChannelNum]+sizeof(BERTLVFileHead)+G_BERTLV_CurrentPointer[G_ChannelNum]-1+allLength,usedSize+anotherLength-G_BERTLV_CurrentPointer[G_ChannelNum]+1);
						
				WritePageCacheEX(G_CurrentEfAddr[G_ChannelNum]+F_BERTLV_UsedSize,(BYTE *)&usedSize,sizeof(WORD));
			}
						
			WriteBERBuf(G_CurrentEfAddr[G_ChannelNum],usedSize+anotherLength,&RecData[0],P3);

			usedSize += 1+anotherLength;
	
			G_BERTLV_CurrentOffset[G_ChannelNum] = usedSize;
			
			G_BERTLV_CurrentPointer[G_ChannelNum] = usedSize;

			G_PreviousEfAddr[G_ChannelNum]  = G_CurrentEfAddr[G_ChannelNum];

		}

		G_BERTLV_OperateTag[G_ChannelNum] = INS;
	
		result = CreateReplaceBERTLVTag(G_CurrentEfAddr[G_ChannelNum]);
				
		if(result == FALSE)
		{			
			P3Error();
		}
		else if(result == 0x01)  //9000
		{
			CommandFinish(0x00);
		}
		else                   // 63 F1
		{
			MoreDataExpected();
		} 
	}
}


void MoreDataAvailable(void)
{
	GlobalSetStatus(0x62F1);
	
	G_CurrentReponse[G_ChannelNum] = 0x100;
}

void MoreDataExpected(void)
{
	GlobalSetStatus(0x63F1);
}

#endif
