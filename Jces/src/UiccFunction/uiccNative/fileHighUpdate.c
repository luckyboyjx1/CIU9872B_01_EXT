/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * HISTORY: V1.0 强更新文件的相关操作
            V1.1 修改宏定义结构，保证使用宏High_Update_Function后才能定义High_Update_Function_SmallBlock   by yangjian20200904
			V1.2 修改强更新功能的bug(强更新文件不能调用SetTFConst函数)
			

 */
#include  "typedef.h"
#include  "uicc_main.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include  "fileAction.h"
#include  "stkhead.h"
#include  "fileHighUpdate.h"
//#include  "Transaction.h"

BYTE    xdata    G_High_Update_NUM;       //强更新配置页数  根据需求计算  如果不需要此功能 配置0
#ifdef      High_Update_Function
BYTE    xdata    G_High_Update_Times[20]; //暂定最大支持20个强更新文件  需要更改 声明一起修改 (对于轮询的表示当前第几个是有效块,对于文件独占页的备份表示更新次数)
#endif
#ifdef  High_Update_Function_RAM	
BYTE    xdata    G_High_Update_RAM[40]; //强更新区域在RAM  TLV格式
#endif
/**
 * @brief 		IF_SupportHighUpdate 判断是否支持强更新文件 如果不支持来了9101XX tag则按照普通文件处理
 * 
 *
 * @Returns   	TRUE  支持强更新文件/ FALSE 不支持强更新文件
 *
 * @author 		LCZ
 *
 * @history 	created by LCZ 20200321
 */
BYTE IF_SupportHighUpdate(BYTE updateTag)
{
#ifdef  High_Update_Function
	if((updateTag & Hige_UpdateWithEep) != 0x00)
	{
		return TRUE;
	}
#endif

#ifdef  High_Update_Function_RAM
	if((updateTag & Hige_UpdateWithRAM) == Hige_UpdateWithRAM)
	{
		return TRUE;
	}
#endif	
	return FALSE;
}
/**
 * @brief 		InitUpdateVariable 强更新变量初始化(G_High_Update_NUM这个变量要手动给初值)
 * 
 *
 * @Returns   	void
 *
 * @author 		LCZ
 *
 * @history 	created by LCZ 20200321
 */
void InitUpdateVariable(void)
{
#ifdef  High_Update_Function
	memset(G_High_Update_Times, 0x00,sizeof(G_High_Update_Times));
#endif
	
#ifdef  High_Update_Function_RAM	
	memset(G_High_Update_RAM, 0x00,sizeof(G_High_Update_RAM));
#endif		
}
/**
* @brief  IF_Init_High_Update_Config 

* @param index  强更新配置区的索引
* @param config 强更新配置区值

* @returns 
*/
void IF_Init_High_Update_Config(BYTE index,BYTE *config)
{
#if ((defined High_Update_Function) ||(defined High_Update_Function_RAM))
	BYTE idata indexeep;
	
	indexeep = (index & Hige_UpdateWithEep); //强更新RAM的时候传进来的是0x8x;

	WriteEepromEX(E_High_Update_Config + (indexeep-1)*High_Update_ConfigForFID,config,High_Update_ConfigForFID);    //
#endif
}
/**
* @brief  IF_GetConfig_ByIndex 
* @param index  强更新配置区的索引
* @param config 强更新配置区值
* @returns 
*/
void IF_GetConfig_ByIndex(BYTE index,BYTE *config)
{
#if ((defined High_Update_Function) ||(defined High_Update_Function_RAM))
	BYTE idata indexeep;
	
	indexeep = (index & Hige_UpdateWithEep); //强更新RAM的时候传进来的是0x8x;

	ReadEeprom(config,E_High_Update_Config + (indexeep-1)*High_Update_ConfigForFID,High_Update_ConfigForFID);    //
#endif
}
#ifdef  High_Update_Function

#ifdef High_Update_Function_SmallBlock
/**
* @brief  以CRC_16方式计算一个数据块的CRC值. 

* @param  pucData -  待校验的数据块
* @param  nBytes  -  数据块大小, 单位是字节

* @returns 返回值是无符号的长整型,   其中低16位有效.
*/ 
#define   	CRC16_POLYNOMIAL_file 		0x1021 //   CRC_16校验方式的多项式.
WORD CRC16cal(BYTE *ptr,BYTE count)
{
	BYTE idata i,j;
	WORD idata  crc;

	//crc = 0xFFFF;  
	
	crc = 0x00;

	for(j=0;j<(count);j++)
	{
		crc = crc ^ (WORD)*ptr++ << 8;

		for (i = 0; i < 8; ++i)
		{
			if (crc & 0x8000)
			{
				crc = crc << 1 ^ CRC16_POLYNOMIAL_file;
			}
			else
			{
				crc = crc << 1;
			}
		}
	}

	 return (crc & 0xFFFF);
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       
* @Function     	    :IF_Cal_High_Upadte_CRC  计算强更细区域地址的CRC
*  
* @Discription       ：另外加上强更新方案的tag A55A在一起计算CRC
*
* @See                :
*                                                                                                                                                
* @Parameter          :addrDst   目的地址
* @Parameter          :calsize   计算CRC数据的大小
*
* @Return             :void
*                                                                                                                                                        
* @History		        :V1.0 Creat by LCZ 20200309  
*
 -------------------------------------------------------------------------------------------------------------------*/ 
BYTE code High_Upadte_Tag[2] = {0xA5,0x5A};
WORD IF_Cal_High_Upadte_CRC(BYTE *ptr,BYTE calsize)
{
		BYTE idata offset;
	  WORD idata crc;
	
//		if(calsize>sizeof(tempBuf))     //计算的数据不会超过tempBuf大小 加上tag也是不会  目前的方案是 原文件加 备份文件 够一页才存放一页轮询 否则是一个文件独占页的方式
//		{
//			return FALSE;
//		}
	
//		ReadEeprom(&tempBuf[0], addrDst, calsize);
		
		offset = calsize;
		
//		if((calsize%2) != 0x00)
//		{
//			ptr[calsize] = 0x5A;
//			
//			offset++;
//		}
//		if(mode == Cal_CRC_Int)
		{
			memcpy(&ptr[offset],(BYTE *)&High_Upadte_Tag,sizeof(High_Upadte_Tag));
			
	//		tempBuf[offset++] = 0xA5;
	//		tempBuf[offset++] = 0x5A;
			
			offset += 0x02;
		}
		
		crc = CRC16cal(ptr,offset);
		
		return crc;
	
}
#endif
#endif
/**
 * @brief 		IF_UpdateWithRAM 判断强更新文件是否在RAM的 好判断时候执行写操作
 * 
 * @param       UWORD fileAddr  文件体地址
 *
 * @Returns   	TRUE  RAM/ FALSE E
 *
 * @author 		LCZ
 *
 * @history 	created by LCZ 20200321
 */
#ifdef  High_Update_Function_RAM
BYTE IF_UpdateWithRAM(UWORD fileAddr)
{
	BYTE idata updatetag;
	
	updatetag = GetFileHeadByte(fileAddr, F_Pointer_Offset);
	
	if((updatetag & Hige_UpdateWithRAM) ==Hige_UpdateWithRAM)
	{
		return updatetag;
	}
    return FALSE;	
}
#endif
/**
 * @brief 		IF_UpdateOrReadWithRAM 强更新在RAM的文件的读写
 * 
 * @param       UWORD fileAddr  文件体地址
 *
 * @Returns   	TRUE  已经执行完读写/ 0x02  不是强更新在RAM里面/FALSE 强更新RAM里面 但是执行出错
 *
 * @author 		LCZ
 *
 * @history 	created by LCZ 20200321
				V1.1 20200913添加 Hige_Set_mode
 */
BYTE IF_UpdateOrReadWithRAM(UWORD fileAddr,WORD offset, BYTE * buf, WORD len,BYTE mode)
{
#ifdef  High_Update_Function_RAM
	
	WORD  idata fileSize;
	BYTE  idata updatetag;
	BYTE  idata indexoffset;
	UWORD xdata fileAddrbody;
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	updatetag = IF_UpdateWithRAM(fileAddr);
	
	if(updatetag == FALSE) //不是强更新RAM
	{
		 return 0x02;
	}
	
	fileSize = GetFileHeadWord(fileAddr,F_SpaceSize_Offset);

	if ((offset+len)> fileSize) //出错
	{
		return FALSE;
	}
	
	indexoffset = FindTag(&G_High_Update_RAM[0],(updatetag & Hige_UpdateWithEep),sizeof(G_High_Update_RAM)); //(updatetag & Hige_UpdateWithEep)  bit7 代表强更新  其余位 是索引
	
	if((indexoffset == FALSE) || (fileSize != G_High_Update_RAM[indexoffset]))   //第一次读写 或者Resize等改变了文件大小
	{
		if((fileSize != G_High_Update_RAM[indexoffset])&&(G_High_Update_RAM[indexoffset] != 0x00)&&(indexoffset != FALSE))      //有Resize时候 简单做法  RAM全部清除  下次读写时候重新赋值
		{
			memset(G_High_Update_RAM,0x00,sizeof(G_High_Update_RAM));
			
			indexoffset = 0x00;
		}
		
		for(;indexoffset<sizeof(G_High_Update_RAM);) //缓存TLV格式(T 为index)
		{
			if((G_High_Update_RAM[indexoffset] == 0x00)&&(G_High_Update_RAM[indexoffset+1] == 0x00))
			{
				break;     //空闲的位置
			}
			else
			{
				indexoffset += (G_High_Update_RAM[indexoffset+1] +2);
			}
		}
		
		G_High_Update_RAM[indexoffset]   =  (updatetag & Hige_UpdateWithEep); //G_High_Update_RAM 大小应该是足够的  创建文件的时候做判断
		
		G_High_Update_RAM[indexoffset+1] =  (fileSize & 0xFF);
		
		fileAddrbody = getFileContentAddr(fileAddr); //这里要读取整个文件内容
		
		ReadEeprom(&G_High_Update_RAM[indexoffset+2],fileAddrbody,fileSize);
		
		indexoffset ++ ;//执行L的位置  和 FindTag  匹配
	}

	
	if(mode == Hige_Read_mode)
	{
		memcpy(buf, &G_High_Update_RAM[indexoffset+1+offset], len);
	}
	else if(mode == Hige_Set_mode) //Set
	{
		memset(&G_High_Update_RAM[indexoffset+1+offset], buf[0], len);
	}
	else //Write
	{
		memcpy(&G_High_Update_RAM[indexoffset+1+offset], buf, len);
	}

	return TRUE;
#else
	return 0x02;
#endif	
}
/**
 * @brief 		IF_UpdateFile_Resize 判断强更新文件是否可以Resize
 * 
 * @param       UWORD oldfilesize  久文件大小
 * @param       UWORD newfilesize  新文件大小
 *
 * @Returns   	TRUE 继续往下执行/ FALSE  不继续往下执行
 *
 * @author 		LCZ
							V1.1:LCZ 添加四字节对齐 在CRC后面补0-3字节对齐 20200615
 *
 * @history 	created by LCZ 20200306
 */
BYTE IF_UpdateFile_Resize(UWORD oldfilesize,UWORD newfilesize,BYTE index)
{
#if ((defined High_Update_Function) ||(defined High_Update_Function_RAM))
	BYTE idata oldpagenum,newpagenum;//4字节对齐占用大小
#endif

#if ((defined High_Update_Function_SmallBlock) ||(defined High_Update_Function_RAM))
	BYTE xdata addvalue[High_Update_ConfigForFID];
#endif
	
#ifdef High_Update_Function_SmallBlock //轮询模式
	BYTE idata actuallen;
#endif
	
#ifdef High_Update_Function_RAM	
	if((index & Hige_UpdateWithRAM) == Hige_UpdateWithRAM)
	{
		newpagenum = 0x00;
		
		for(oldpagenum = 0x01;oldpagenum<(PFPageSize/sizeof(UWORD));oldpagenum++)
		{
			ReadEeprom(&addvalue[0],E_High_Update_Config + (oldpagenum-1)*High_Update_ConfigForFID, High_Update_ConfigForFID);
			
			if(addvalue[0] == Hige_UpdateWithRAM) //
			{
				newpagenum += (addvalue[2] +2);   //暂时定义 0x80 index size rfu  缓存在 RAM中 index L V
			}
			else if((addvalue[0]!= 0xFF)&&(addvalue[1]!= 0xFF)) //
			{
				//pagenum += addvalue[3];
			}
			else
			{
				break;
			}
		}
		
		if((newpagenum + (newfilesize-oldfilesize))>sizeof(G_High_Update_RAM)) //新的文件大小 总和 超过了 G_High_Update_RAM大小
		{
			return FALSE;
		}
	}
	else
#endif
	{
#ifdef High_Update_Function
#ifdef High_Update_Function_SmallBlock //轮询模式
		ReadEeprom((BYTE *)&addvalue,E_High_Update_Config+(index-1)*High_Update_ConfigForFID,sizeof(addvalue));	//读取配置区
	
		if(addvalue[3] == 0x01) //轮询模式
		{
#ifdef High_Update_Function_Fourunit //四字节对齐 在CRC后面补充0-3字节FF
			
			actuallen = newfilesize + (((newfilesize%4)==0x00)?0x00:(4-(newfilesize%4)));
#else
			actuallen = newfilesize; //不需要四字节对齐 
#endif
			if(((actuallen +4)*addvalue[0])>PFPageSize)
			{
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}
#endif	
		oldpagenum = ((oldfilesize%PFPageSize) == 0x00)?(oldfilesize/PFPageSize):(oldfilesize/PFPageSize+1);   //久文件所占页大小
		
		newpagenum = ((newfilesize%PFPageSize) == 0x00)?(newfilesize/PFPageSize):(newfilesize/PFPageSize+1);  //新文件所占页大小
		
		if(newpagenum>oldpagenum)   //新的文件大小 超过了原文件大小所占的页数 会导致强更新区域文件错乱
		{
			return FALSE;
		}
#endif
	}
	return TRUE;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       
* @Function     	    :CopyFileContent  备份区数据乱换  不带断电写  源地址和目的地址 不重叠
*  
* @Discription        :
*
* @See                :
*                                                                                                                                                
* @Parameter          :addrDst   目的地址
* @Parameter          :addrSrc   源地址
* @Parameter          :pagenum   拷贝大小  页为单位
*
* @Return             :void
*                                                                                                                                                        
* @History		        :V1.0 Creat by xwh 20180509  
*
*                     :V1.1 
 -------------------------------------------------------------------------------------------------------------------*/ 
#ifdef  High_Update_Function
void CopyFileContent(UWORD addrDst,UWORD addrSrc,BYTE pagenum)
{
	  BYTE idata i;
	
//		SetBit(G_COMMANDPATH,B_ACCESSEX);
	
//		SetBit(G_COMMANDPATH,B_ACCESS);
	
		closeWrSwitch234();
	
	  openWrSwitch2();
		
	  openWrSwitch3();
	
		for(i = 0x00;i<pagenum;i++)
		{
			ReadEeprom(&BAK_BUFFER[0], addrSrc+i*PFPageSize, PFPageSize); 
			
			G_FLASH_ADDR = addrDst+i*PFPageSize;
			
			UpdateFlash(addrDst+i*PFPageSize,BAK_BUFFER);
		}
}
#endif
/**
 * @brief 		IF_GetUpdate_By_index 根据索引来查找强更新区域地址
 * 
 * @param       index  强更新配置区的索引  保存在文件头 
 *
 * @Returns   	UWORD  返回的是 索引的起始地址
 *
 * @author 		LCZ
 *
 * @history 	created by LCZ 20200306
 */
UWORD IF_GetUpdate_By_index(BYTE index)
{
#ifdef  High_Update_Function
		BYTE   idata i,pagenum;
		BYTE   idata configvalue[High_Update_ConfigForFID];
	
		pagenum = 0x00;
	  
	  for(i = 0x01;i<index; i++)
		{
			 ReadEeprom((BYTE *)&configvalue,E_High_Update_Config+(i-1)*High_Update_ConfigForFID,High_Update_ConfigForFID);	//获取此文件强更新配置区内容
			
			 if(configvalue[0] == Hige_UpdateWithRAM) //强更新在RAM
			 {
				 continue;
			 }
			
			 pagenum += configvalue[0x03];
		}
		
		return (E_High_Update_START + pagenum*PFPageSize);    //这时候应该不存在错误的情况
#else
		return FALSE; //如果定义了 强更新 不会FALSE ,没定义的话 返回FALSE 也不会使用
#endif
}
/**
 * @brief 		IF_Switch_High_Update_START 切换强更新备份文件
 * 
 * @param       void
 *
 * @Returns   	VOID
 *
 * @author 		LCZ
 *
 * @history 	created by LCZ 20200306
 */
void IF_Switch_High_Update_START(void)
{
#ifdef  High_Update_Function
		BYTE   idata index;
		BYTE   idata configvalue[High_Update_ConfigForFID];
	    UWORD  xdata desAddr,startAddr,backAddr;
#ifdef High_Update_Function_SmallBlock		
		BYTE   idata filetype;
#endif	
		if(G_CurrentEfAddr[G_ChannelNum] == FALSE)
		{
			return;
		}
	
		index = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_Pointer_Offset);
		
		if(((index & Hige_UpdateWithEep) == 0x00) || ((index & Hige_UpdateWithRAM)==Hige_UpdateWithRAM)) //强更新在E2
		{
				return;
		}
#ifdef High_Update_Function_SmallBlock		
		filetype = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_Type_Offset);

		if((filetype & 0x07) == TRANTYPE)   //支持页擦字节写  透明文件的轮询更新
		{
			ReadEeprom((BYTE *)&configvalue,E_High_Update_Config+(index-1)*High_Update_ConfigForFID,High_Update_ConfigForFID);	//获取此文件强更新配置区内容
			
			if(configvalue[3] == 0x01) //总共大小只占一页  则是轮询方式  读写都在 FindTFAddr 处理  没有切换备份页
			{
				return;
			}
			
		}
#endif	
		
		if(G_High_Update_Times[index-1] >= (High_UpdateNum_Switch-1))// 切换  100次时候切换 0开始
		{
				ReadEeprom((BYTE *)&configvalue,E_High_Update_Config+(index-1)*High_Update_ConfigForFID,High_Update_ConfigForFID);	//获取此文件强更新配置区内容
			
			  startAddr = IF_GetUpdate_By_index(index); //强更新区域的起始地址
			
				backAddr = startAddr + (configvalue[1]*configvalue[2]*PFPageSize); //当前指向的备份地址
			
				if((configvalue[1]+1)<configvalue[0]) //备份区还有空闲 //下标0指示原文件
				{
					 //desAddr = startAddr + (configvalue[2]*PFPageSize);
					
					 configvalue[1]++;     //指向下一个备份文件
				}
				else                               //拷贝回主文件
				{
					 //desAddr = startAddr; 
					
					 configvalue[1] = 0x00;    //指向主文件
				}
				
				desAddr =  startAddr + (configvalue[1]*configvalue[2]*PFPageSize);//指向下一个备份文件地址
				
				
				CopyFileContent(desAddr,backAddr,configvalue[2]); //切换备份文件
				
				WriteEepromEX(E_High_Update_Config+(index-1)*High_Update_ConfigForFID,configvalue,High_Update_ConfigForFID); //更新配置区  这时候要带断电写
			
				G_High_Update_Times[index-1] = 0x00;
		}
		else
		{
				G_High_Update_Times[index-1] ++;
		}
#endif
}
#ifdef  High_Update_Function	
#ifdef High_Update_Function_SmallBlock
 /*
 *@Function: CheckValueFF
 *
 *@Descrip:  判断内容是否是全FF
 *
 *@Param:    BYTE *buf	 -- 传进来的buf
 *@Param:    WORD filesize---文件大小
 *
 *@Return:  TRUE/FALSE
 *
 *@Author:   LCZ 20200927
 *
 */
BYTE CheckValueFF(BYTE *buf,WORD filesize)
{
	WORD idata i;
	
	for(i=0x00;i<filesize;i++)
	{
		if(buf[i] != 0xFF)
		{
			return FALSE;
		}
	}
	
	return TRUE;
	
}
 /*
 *@Function: GetLastActiveSmallBlock
 *
 *@Descrip:  获取轮询模式时候的有效块
 *
 *@Param:    UWORD  fileAddr	 -- 强更新区域的起始地址
 *@Param:    UWORD  headAddr   ---  文件体起始地址
 *
 *@Return:  lastblock最后一个有效块/函数返回值是 断电块个数
 *
 *@Author:   LCZ 20200309 
				V1.1:LCZ 添加四字节对齐 在CRC后面补0-3字节对齐
				V1.2:tmp  替换掉tempBuf 防止 WriteTFBuf等穿件来就是 tempBuf
				V1.3:CheckValueFF函数添加去掉if(memcmp(tmp,BAK_BUFFER,filesize+4) == EQUAL) //全FF 即为有效块
							#define tmp (BAK_BUFFER+((PFPageSize/2)+4))  修改为  #define tmp BAK_BUFFER
 *
 */

BYTE GetLastActiveSmallBlock(UWORD  fileAddr,UWORD  headAddr,BYTE *lastblock,BYTE mode)
{
	BYTE idata index,num,lastnum,i,type;
	WORD idata filesize,actuallen; //actuallen包括补位后的大小 但是没有包括tag 2 +CRC 2
	WORD idata crc;	
	
	#define tmp BAK_BUFFER    //
	
	ReadEeprom((BYTE *)&index,headAddr+F_Pointer_Offset,sizeof(BYTE));	//获取此文件强更新配置取的索引
	
	ReadEeprom((BYTE *)&filesize,headAddr+F_SpaceSize_Offset,sizeof(WORD));	//获取文件大小

#ifdef High_Update_Function_Fourunit
	actuallen = filesize + (((filesize%4)==0x00)?0x00:(4-(filesize%4)));
#else
	actuallen = filesize;
#endif
	
	num = (PFPageSize / (actuallen +4)); //总共有多少个块
	
	//ReadEeprom(&BAK_BUFFER[0], fileAddr, PFPageSize); 
	//memset(BAK_BUFFER,0xFF,filesize+4);//filesize+4 会不会和tmp重叠?
	type = 0x00; //默认不存在断电
	
	lastnum = 0x00; //最后一个有效块
	
	if((G_High_Update_Times[index-1] == 0x00) || (mode == Hige_Update_mode))  //还没初值 第一次读写  对于轮询时候 表示当前第几块是有效块 而不是更新次数
	{
		if(G_High_Update_Times[index-1] != 0x00) //Hige_Update_mode更新时候 当前有效块的下一块也可能是 断电的块  
		{
			i = G_High_Update_Times[index-1];
		}
		else
		{
			i = 0x01;
		}
		for(;i<=num;i++)//查找最近的一个有效块
		{
			ReadEeprom(tmp, fileAddr + ((i-1)*(actuallen +4)), (filesize+4)); //读取文件体 +TAG +CRC
			
			if(CheckValueFF(tmp,filesize+4) == TRUE) //全FF 即为有效块,第一块的时候不可能filesize+4都是全FF
			{
				break;
			}
			
		  memcpy((BYTE *)&crc,&tmp[filesize+2], sizeof(WORD)); //读取CRC
			
			if((IF_Cal_High_Upadte_CRC(tmp,filesize) != crc) || (memcmp(&tmp[filesize],(BYTE *)&High_Upadte_Tag,sizeof(High_Upadte_Tag)) != EQUAL ))//CRC不相等  也不是全FF 则写该区域的时候断电
			{
					type++;//有存在断电  可能有多个断电
				
			    //break;
			}
			else
			{
				lastnum = i; //每次更新最后有效块
				
				type = 0x00;//lastnum  包括了前面的前面断电的块个数
			}
		}
		
	}
	else
	{
		lastnum =  G_High_Update_Times[index-1];
	}
	
	lastblock[0] = lastnum;
	
	#undef	tmp//去掉宏定义
	
	return type;
}
 /*
 *@Function: GetHigtUpdateAddr
 *
 *@Descrip:  获取强更新文件的实际地址
 *
 *@Param:    UWORD  fileAddr	 -- 强更新区域的起始地址
 *@Param:    UWORD  headAddr   ---  文件体起始地址
 *
 *@Return:   如果是读文件 则返回当前有效块的偏移,如果是写文件 更改Recdata里面的数据 返回下一块可用的地址.
 *
 *@Author:   LCZ 20200309 
*       V1.1:LCZ 添加四字节对齐 在CRC后面补0-3字节对齐
 *
 */

WORD GetHigtUpdateAddrSmallBlock(UWORD  fileAddr,UWORD  headAddr)
{
	BYTE idata index,lastnum;
	WORD idata filesize,actuallen; //actuallen文件的实际占用大小 包括补位的0-3字节 不包括tag 2+ crc 2
	

	ReadEeprom((BYTE *)&index,headAddr+F_Pointer_Offset,sizeof(BYTE));	//获取此文件强更新配置取的索引
	
	ReadEeprom((BYTE *)&filesize,headAddr+F_SpaceSize_Offset,sizeof(WORD));	//获取文件大小
	/*	
	num = (PFPageSize / (filesize +4)); //总共有多少个块
	
	//ReadEeprom(&BAK_BUFFER[0], fileAddr, PFPageSize); 
	memset(BAK_BUFFER,0xFF,filesize+4);
	type = 0x00; //默认不存在断电
	
	lastnum = 0x00; //最后一个有效块
	
	if((G_High_Update_Times[index-1] == 0x00) || (INS == 0xD6))  //还没初值 第一次读写  对于轮询时候 表示当前第几块是有效块 而不是更新次数
	{
		if(G_High_Update_Times[index-1] != 0x00) //D6更新时候 当前有效块的下一块也可能是 断电的块  
		{
			i = G_High_Update_Times[index-1];
		}
		else
		{
			i = 0x01;
		}
		for(;i<=num;i++)//查找最近的一个有效块
		{
			ReadEeprom(tempBuf, fileAddr + ((i-1)*(filesize +4)), (filesize+4)); //读取文件体 +TAG +CRC
			
			if(memcmp(tempBuf,BAK_BUFFER,filesize+4) == EQUAL) //全FF 即为有效块
			{
				break;
			}
			
		  memcpy((BYTE *)&crc,&tempBuf[filesize+2], sizeof(WORD)); //读取CRC
			
			if((IF_Cal_High_Upadte_CRC(tempBuf,filesize) != crc) || (memcmp(&tempBuf[filesize],(BYTE *)&High_Upadte_Tag,sizeof(High_Upadte_Tag)) != EQUAL ))//CRC不相等  也不是全FF 则写该区域的时候断电
			{
					type++;//有存在断电  可能有多个断电
				
			    //break;
			}
			else
			{
				lastnum = i; //每次更新最后有效块
				
				type = 0x00;//lastnum  包括了前面的前面断电的块个数
			}
		}
		
	}
	else //读模式 直接使用上一次有效值
	{
		lastnum = G_High_Update_Times[index-1];  
	}
	*/
	
	GetLastActiveSmallBlock(fileAddr,headAddr,(BYTE *)&lastnum,Hige_Read_mode);
	
	G_High_Update_Times[index-1] = lastnum;
#ifdef High_Update_Function_Fourunit	
	actuallen = filesize + (((filesize%4)==0x00)?0x00:(4-(filesize%4)));
#else
	actuallen = filesize;
#endif
		
	return ((lastnum-1)*(actuallen +4));  //返回的是偏移 减去1
	
	//更新模式
//	
//	SetBit(G_COMMANDPATH,B_ACCESSEX);
//	
//	
//	ReadEeprom(tempBuf, fileAddr + ((lastnum-1)*(filesize +4)),filesize); //读取文件体
//	
//	fileLgth = (((WORD)P1 << 8)&0xFF00 )|((WORD)P2 & 0x00FF);  //目前都是小文件
//	
//	if(memcmp(&tempBuf[fileLgth],&RecData[0],P3) == EQUAL) //新数据和更新的旧数据 一样
//	{
//		G_High_Update_Times[index-1] = lastnum;
//		
//		return ((lastnum-1)*(filesize +4));  //返回的是偏移 减去1		
//	}
//	
//	memcpy(&tempBuf[fileLgth],(BYTE *)&RecData[0],P3);  //新数据覆盖旧数据
//	
//	crc =IF_Cal_High_Upadte_CRC(tempBuf,filesize);

//	memcpy(&tempBuf[filesize],(BYTE *)&High_Upadte_Tag,sizeof(High_Upadte_Tag));
//	
//	memcpy(&tempBuf[filesize+2],(BYTE *)&crc,sizeof(WORD));
//	
//	if((lastnum+type)==num) //没有剩余的空闲块
//	{
//		memset(BAK_BUFFER,0x0FF,sizeof(BAK_BUFFER));
//		
//		memcpy(BAK_BUFFER,tempBuf,filesize +4);  //断电写的是新数据
//		
//		AccessEepromEX(fileAddr,BAK_BUFFER,NULL,PFPageSize, (EEP_Write)|(EEP_HighUpdate));  //最后一块有效数据回写
//		
//		G_High_Update_Times[index-1] = 0x01;
//		
//	}
//	else      //还有剩余块 字节写
//	{
//		
//		 G_FLASH_ADDR = fileAddr+((lastnum+type)*(filesize +4)); //

//		 SetBit(G_COMMANDPATH,B_ACCESSEX);
//		
//		 SetBit(G_COMMANDPATH,B_ACCESS);
//		
//		 num = WriteWithoutErease(fileAddr+((lastnum+type)*(filesize +4)),tempBuf,filesize +4);	
//		
//		 if(num == FALSE)
//		 {
//			 return FALSE;
//		 }
//	
//		 G_High_Update_Times[index-1] = (++lastnum + type); //下一个有效块
//	}
//	
//	return (i*(filesize +4)); //写的时候返回地址没意义
	
}

#endif
#endif
  /*
 *@Function: IF_Update_SmallBlock  增加这个函数的原因是因为 优选漫游等写文件 传入的buf等和D6不一样
 *
 *@Descrip:  更新轮询模式时候的块
 *
 *@Param:    UWORD  fileAddr	  -- 轮询模式最后一个有效块的起始地址
 *@Param:    UWORD  headAddr   ---  文件头的起始地址
 *@Param:    BYTE    * buf     ---  更新的数据  传进来的不能是tempBuf
 *@Param:    WORD   offset     ---  更新数据的偏移
 *@Param:    BYTE   lgth       ---  更新数据的长度
 *@Param:    BYTE   mode       ---- Set或者Write模式
 *@Return:  lastblock最后一个有效块/函数返回值是 断电块个数
 *
 *@Author:   LCZ 20200309 
						V1.1:LCZ 添加四字节对齐 在CRC后面补0-3字节对齐
						V1.2:BYTE xdata temp[(PFPageSize/2)+4];去掉 修改为 #define temp BAK_BUFFER 而且位置GetLastActiveSmallBlock之后,GetLastActiveSmallBlock函数里面也是用到了BAK_BUFFER
 *
 */
BYTE IF_Update_SmallBlock(UWORD  fileAddr,UWORD  headAddr,BYTE * buf,WORD offset, BYTE lgth,BYTE mode)
{
#ifdef  High_Update_Function	
#ifdef High_Update_Function_SmallBlock
	
	BYTE idata index,num,lastnum,type;
	WORD idata filesize,actuallen;
	WORD idata crc;
	UWORD  fileAddrStart;
	//BYTE xdata temp[(PFPageSize/2)+4]; //强更新文件至少有一个备份
	 
	
	fileAddrStart = GetPageStart(fileAddr);

	ReadEeprom((BYTE *)&filesize,headAddr+F_SpaceSize_Offset,sizeof(WORD));	//获取文件大小
	
	ReadEeprom((BYTE *)&index,headAddr+F_Pointer_Offset,sizeof(BYTE));	//获取此文件强更新配置取的索引
	
#ifdef High_Update_Function_Fourunit
	actuallen = filesize + (((filesize%4)==0x00)?0x00:(4-(filesize%4)));
#else
	actuallen = filesize;
#endif	
	num = (PFPageSize / (actuallen +4)); //总共有多少个块
	
	type = GetLastActiveSmallBlock(fileAddrStart,headAddr,(BYTE *)&lastnum,Hige_Update_mode); //获取当前有效块 以及有效块后的断电块
	
//	SetBit(G_COMMANDPATH,B_ACCESSEX);

//#define temp BAK_BUFFER //放在 GetLastActiveSmallBlock函数后面
#define temp tempBuf //放在 GetLastActiveSmallBlock函数后面

	ReadEeprom(temp, fileAddrStart + ((lastnum-1)*(actuallen +4)),actuallen); //读取文件体
	
	if(memcmp(&temp[offset],&buf[0],lgth) == EQUAL) //新数据和更新的旧数据 一样
	{
		G_High_Update_Times[index-1] = lastnum;
		
		//#undef temp
		
		return TRUE;
	}
	
	if(mode == Hige_Update_mode) //Write更新模式
	{
	
		memcpy(&temp[offset],(BYTE *)&buf[0],lgth);  //新数据覆盖旧数据
	}
	else          //Set更新模式
	{
		memset(&temp[offset],buf[0],lgth); //新数据覆盖旧数据
	}
	
	crc =IF_Cal_High_Upadte_CRC(temp,filesize);

	memcpy(&temp[filesize],(BYTE *)&High_Upadte_Tag,sizeof(High_Upadte_Tag));
	
	memcpy(&temp[filesize+2],(BYTE *)&crc,sizeof(WORD));
	
	if((lastnum+type)==num) //没有剩余的空闲块
	{
		//memset(BAK_BUFFER,0x0FF,sizeof(BAK_BUFFER));
		
		//memcpy(BAK_BUFFER,temp,actuallen +4);  //断电写的是新数据
		
		memset(temp+actuallen +4,0x0FF,PFPageSize-(actuallen +4));//把后面的数据清成全FF
		
		//AccessEepromEX(fileAddrStart,temp,NULL,PFPageSize, (EEP_Write)|(EEP_HighUpdate));  //最后一块有效数据回写,这里的temp就是BAK_BUFFER
		
		WriteEepromAtomic(fileAddrStart,temp,PFPageSize);
		
		
		G_High_Update_Times[index-1] = 0x01;
		
	}
	else      //还有剩余块 字节写
	{
		
		 G_FLASH_ADDR = fileAddrStart+((lastnum+type)*(actuallen +4)); //

//		 SetBit(G_COMMANDPATH,B_ACCESSEX);
		
//		 SetBit(G_COMMANDPATH,B_ACCESS);
		
		 num = WriteWithoutErease(fileAddrStart+((lastnum+type)*(actuallen +4)),temp,actuallen +4);	
		
		 if(num == FALSE)
		 {
			 return FALSE;
		 }
	
		 G_High_Update_Times[index-1] = (++lastnum + type); //下一个有效块
	}
	
	#undef temp
	
	return TRUE; 
	
#else	
	return TRUE;
#endif 	
#else
	return TRUE;	
#endif
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       
* @Function     	    :IF_TFUpdate_Smallblock  判断文件是否是 才有轮询模式更新
*  
* @Discription        : 传进来文件体地址
*
* @See                :
*                                                                                                                                               
*
* @Return             :TEUE/FALSE
*                                                                                                                                                        
* @History		        :V1.0 Creat by LCZ 20200309  
*
 -------------------------------------------------------------------------------------------------------------------*/ 
BYTE IF_TFUpdate_Smallblock(UWORD addrDst)
{
#ifdef  High_Update_Function
#ifdef High_Update_Function_SmallBlock
	BYTE idata index,value;
	
	index = GetFileHeadByte(addrDst, F_Pointer_Offset);
	if(index != 0x00)
	{
		ReadEeprom((BYTE *)&value,E_High_Update_Config+(index-1)*High_Update_ConfigForFID+3,sizeof(BYTE));	//读取最后一个字节  总共占用大小
		
		if(value == 0x01)
		{
			return TRUE;
		}
	}
#endif	
#endif	
	return FALSE;
}
/*
*function : IF_Init_High_Update_Area
*
*@description:  强更新区域初始化
*
*@param      :UWORD fileBodyAddr 文件体地址
*@param      :bodySize   文件大小
*@param      :updatetag 强更新 tag
*
*@return     :UWORD addr 分配的地址/FALSE 空间不足
*
*@history    : added by LCZ@20200306
						 V1.1 这里不需要添加四字节对齐操作 补位的数据没有参与CRC 没有什么意义
*/
void IF_Init_High_Update_Area(UWORD fileBodyAddr,WORD bodySize,BYTE updatetag)
{
#ifdef  High_Update_Function
#ifdef High_Update_Function_SmallBlock
	
	WORD idata crc;
	
		if((updatetag & Hige_UpdateWithRAM) == Hige_UpdateWithRAM)   //强更新在RAM
		{
			return;
		}

		if(((bodySize+4)*updatetag)>PFPageSize)
		{
			return;
		}
		
		ReadEeprom(tempBuf, fileBodyAddr, bodySize); //读取文件体
		
		crc = IF_Cal_High_Upadte_CRC(tempBuf,bodySize);
		
		if(crc != FALSE)
		{
			memcpy(&tempBuf[0x00],(BYTE *)&High_Upadte_Tag,sizeof(High_Upadte_Tag));
			
			memcpy(&tempBuf[0x02],(BYTE *)&crc,sizeof(WORD));
			
			WriteEeprom(fileBodyAddr+bodySize,&tempBuf[0x00],4);
		}		
#endif
#endif		
}
/*
*function : GetAddrFormHigh_Update_START
*
*@description:  强更新区域地址分配
*
*@param      :UWORD size 需要分配的空间
*@param      :filetype   强更新文件类型
*@param      :
*
*@return     :UWORD addr 分配的地址/FALSE 空间不足
*
*@history    : added by LCZ@20200306
						V1.1 添加四字节对齐操作
*/

UWORD GetAddrFormHigh_Update_START(UWORD assignSize,BYTE* index,BYTE updatetag,BYTE filetype,BYTE * HighUpdateCfg)
{
#if ((defined High_Update_Function) ||(defined High_Update_Function_RAM))
	BYTE  idata offset;
	BYTE  xdata addvalue[High_Update_ConfigForFID];
	BYTE  xdata pagenum; 
	BYTE  idata updateramsize; //强更新在RAM的大小
//#ifdef High_Update_Function_SmallBlock 
	WORD  idata actuallen;      //补齐 四字节的实际长度
//#endif	
		
	pagenum = 0x00; //已经使用的强更新大小
	updateramsize = 0x00; //已经使用的RAM大小
	
	for(offset = 0x01;offset<(PFPageSize/sizeof(UWORD));offset++)
	{
		ReadEeprom(&addvalue[0],E_High_Update_Config + (offset-1)*High_Update_ConfigForFID, High_Update_ConfigForFID);

		if(addvalue[0] == Hige_UpdateWithRAM) //
		{
			updateramsize += (addvalue[2] +2);   //暂时定义 0x80 index size rfu  缓存在 RAM中 index L V
		}
		else if((addvalue[0]!= 0xFF)&&(addvalue[1]!= 0xFF)) //
		{
			pagenum += addvalue[3];
		}
		else
		{
			break;
		}
	}
	
	if(((offset-1)*sizeof(UWORD)) == PFPageSize) //强更新配置区占满
	{
		return ADDR_IS_NULL;
	}
#ifdef  High_Update_Function	
	if(((updatetag & Hige_UpdateWithEep) != 0x00) && (pagenum >= G_High_Update_NUM))  //强更新在E2 空间不够
	{
		return ADDR_IS_NULL;
	}
#endif
	
#ifdef  High_Update_Function_RAM	
	if((updatetag == Hige_UpdateWithRAM) && ((updateramsize+2+assignSize)>sizeof(G_High_Update_RAM))) //强更新区域在RAM 空间不够
	{
		return ADDR_IS_NULL;
	} 
#endif
	
	memset(addvalue,0xFF,sizeof(addvalue));
	
	addvalue[0] = updatetag;                                                                              //总备份文件个数
	
	if(updatetag == Hige_UpdateWithRAM)
	{
		//addvalue[0] = (updatetag | offset);  
		
		addvalue[1] = offset;  //index
		
		addvalue[2] = assignSize; //size
	}
	else
	{
		addvalue[1] = 0x00;                                                                                  //
		addvalue[2] = ((assignSize%PFPageSize) == 0x00)?(assignSize/PFPageSize):(assignSize/PFPageSize+1);   //
	}
	
#ifdef High_Update_Function_Fourunit
	actuallen = assignSize + (((assignSize%4)==0x00)?0x00:(4-(assignSize%4))); //四字节对齐时候实际占用大小
#else
	actuallen = assignSize;
#endif
	

#ifdef High_Update_Function	
#ifdef High_Update_Function_SmallBlock           //支持页擦字节写

	if((filetype & 0x07) == TRANTYPE)
	{
		if(((actuallen+4)*updatetag)<PFPageSize) //四字节是TAG + CRC  如果原文件和备份文件 一页能处理 就一页处理  目前给出的需求文件也是可以的
		{
	    addvalue[3] = 0x01;
		}
		else
		{
			addvalue[3] = addvalue[2] *updatetag;
		}
	}
	else
	{
		addvalue[3] = addvalue[2] *updatetag; //线性 记录文件 目前是原文件独占一页 备份文件也是
	}		
#else                                        //不支持页擦字节写的芯片  或者空间足够大的文件 都独占一页
	addvalue[3] = addvalue[2] *updatetag;          //总共占用的页大小
#endif
#endif	


#ifdef  High_Update_Function		
	if(((pagenum + addvalue[3])>G_High_Update_NUM) && (updatetag != Hige_UpdateWithRAM))
	{
		return ADDR_IS_NULL;
	}
#endif
	memcpy(HighUpdateCfg,addvalue,sizeof(addvalue));  //先将配置信息存放在备份结构体中，后面统一写到配置区s
	
	//WriteEepromEX(E_High_Update_Config + (offset-1)*High_Update_ConfigForFID,addvalue,sizeof(addvalue));    //登记在配置区域
	if(updatetag == Hige_UpdateWithRAM)
	{
		index[0] = (offset | updatetag);  //F_Pointer_Offset或 index好获取的时候知道这个是强更新RAM还是E2 不需要再从配置与获取一次  0x8X.
	}
	else
	{
		index[0] = offset;  
	}		

	
	return (E_High_Update_START + pagenum*PFPageSize); //返回地址  对于强更新在RAM区域 无意义
#else

  return ADDR_IS_NULL;
#endif
}
/*
 *@Function: GetHigtUpdateAddr
 *
 *@Descrip:  获取强更新文件的实际地址
 *
 *@Param:    UWORD  fileAddr	 -- 强更新区域的起始地址
 *@Param:    UWORD  headAddr   ---  文件体起始地址
 *
 *@Return:   destValue
 *
 *@Author:   LCZ 20200306
 *
 */
UWORD IF_GetHigtUpdateAddr(UWORD  fileAddr,UWORD  headAddr)
{
#ifdef  High_Update_Function
#ifdef High_Update_Function_SmallBlock
	BYTE   idata filetype;
#endif
	BYTE   xdata index,configvalue[High_Update_ConfigForFID];
	UWORD  xdata destaddr;
	
	destaddr = fileAddr;
	
	ReadEeprom((BYTE *)&index,headAddr+F_Pointer_Offset,sizeof(BYTE));	//获取此文件强更新配置取的索引
	
	if((index == 0x00) || ((index & Hige_UpdateWithRAM) == Hige_UpdateWithRAM))  // 不是强更新文件
	{
		return destaddr;
	}
	
	ReadEeprom((BYTE *)&configvalue,E_High_Update_Config+(index-1)*High_Update_ConfigForFID,High_Update_ConfigForFID);	//获取此文件强更新配置区内容
	
#ifdef High_Update_Function_SmallBlock
	filetype = GetFileHeadByte(headAddr,F_Type_Offset);
	
	if((filetype == TRANTYPE)&&(configvalue[3] == 0x01))
	{
		destaddr += GetHigtUpdateAddrSmallBlock(destaddr,headAddr);
	}
	else
	{
		destaddr += (configvalue[0x01]*configvalue[2]*PFPageSize);  //0x01指向第几个备份文件    0x02 文件所占页大小
	}
#else
	
	destaddr += (configvalue[0x01]*configvalue[2]*PFPageSize);  //0x01指向第几个备份文件    0x02 文件所占页大小
#endif

	return destaddr;
#else
	return fileAddr;
#endif	
	
}
/*
 *@Function: IF_InitFileResize
 *
 *@Descrip:  Resize减少轮询文件的大小时候 从新做初始化/或者增大空间 但是不带A5初始化tag
 *
 *@Param:    UWORD  fileAddr	 -- 最近有效块的地址
 *@Param:    UWORD  filesize	 -- 新文件大小
 *@Return:   destValue
 *
 *@Author:   LCZ 20200306
					V1.1 LCZ 添加4字节对齐操作
 *
 */
void IF_InitFileResize(UWORD  fileAddr,WORD  filesize,BYTE index)
{
#ifdef High_Update_Function
#ifdef High_Update_Function_SmallBlock
	BYTE idata value,append;
	WORD idata crc;
	
	ReadEeprom((BYTE *)&value,E_High_Update_Config+(index-1)*High_Update_ConfigForFID+3,sizeof(BYTE));	//读取最后一个字节  总共占用大小
	
	if(value != 0x01)
	{
		return;
	}
	
#ifdef High_Update_Function_Fourunit
	
	append = (((filesize%4)==0x00)?0x00:(4-(filesize%4)));
#else
	append = 0x00;
#endif
	
	memset(BAK_BUFFER,0x0FF,sizeof(BAK_BUFFER));
	
	ReadEeprom(BAK_BUFFER, fileAddr,filesize);
	
	crc =IF_Cal_High_Upadte_CRC(BAK_BUFFER,filesize); //这里面 拷贝了tag
	
	memcpy(&BAK_BUFFER[filesize+2],(BYTE *)&crc,sizeof(WORD)); //拷贝CRC
#ifdef High_Update_Function_Fourunit	
	if(append)
	{
		memset(&BAK_BUFFER[filesize+4],0xFF,append); //添加补位
	}
#endif	
	//AccessEepromEX(fileAddr,BAK_BUFFER,NULL,PFPageSize, (EEP_Write)|(EEP_HighUpdate));  //有效数据回写
	
	memcpy(tempBuf,BAK_BUFFER,PFPageSize); 
	
	WriteEepromEX(fileAddr,tempBuf,PFPageSize);
		
	G_High_Update_Times[index-1] = 0x01;
#endif	
#endif	
}
