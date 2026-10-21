/**
 * @file 		resume.c
 *
 * @brief 		TMC 260断电恢复API
 *
 * @author 		YMF CaiBo WG
 *
 * @history 	2008-03-06: fix bug in CopyEepromEX() by WG
 */
#include  "uicc_main.h"
#include "uicc_error.h"
#include  "resume.h"
#include  "gsmpublic.h"
#include  "Interface_LTEFromBoot.h"
#include  "arithmatic.h"
//#include "Transaction.h"
#include "fileAction.h"
//#include  "CIU51228F_REG.h"


#if !HEAD_BODY_TOGETHER
void MoveHugeEepromEXPrepare(UWORD destination, UWORD source, UWORD length, BYTE blkIdx)
{
	resizeprotect protect;
	
	protect.rfu1 = 0xFF;
	
	protect.rfu2 = 0xFF;
	
	protect.rfu3 = 0xFF;

	protect.startaddr_scr = source;

	protect.endaddr_scr = source + length - 1;

	protect.startaddr_dst = destination;

	protect.endaddr_dst=destination+length-1;

	if(destination <= source)//向低地址移动
	{
		protect.curraddr_scr = protect.startaddr_scr;

		protect.curraddr_dst = protect.startaddr_dst;
	}
	else//向高地址移动
	{
		protect.curraddr_scr = protect.endaddr_scr;

		protect.curraddr_dst = protect.endaddr_dst;
	}


	protect.completed = 0xCC;

	WriteEepromEX((E_RESIZEPROTECT + (blkIdx*sizeof(resizeprotect))), (BYTE*)&protect, sizeof(resizeprotect));
}

/*-------------------------------------------------------------------------------------------------------//

-------------------------------------------------------------------------------------------------------------*/

void MoveOneBlock(BYTE blkIdx)
{
	UWORD length,copyLgth;

	resizeprotect protect;
	BYTE need_commit = FALSE;
	UWORD protectContext = E_RESIZEPROTECT + (blkIdx*sizeof(resizeprotect));

	ReadEeprom((BYTE*)&protect, protectContext, sizeof(resizeprotect));

	if(protect.completed != 0xCC)
	{
		return;
	}
	

	if(protect.startaddr_dst < protect.startaddr_scr)//向低地址移动
	{
		length = protect.endaddr_scr - protect.curraddr_scr + 1;
		
		copyLgth = (PFPageSize-GetPageOffset(protect.curraddr_dst)); 

	    copyLgth = (length > copyLgth)?copyLgth:length;
		
		need_commit = IF_BeginTransaction();
		
		ReadEeprom(tempBuf,protect.curraddr_scr,copyLgth);
		
		WriteEepromEX(protect.curraddr_dst,tempBuf, copyLgth);
		protect.curraddr_scr += copyLgth;
		protect.curraddr_dst += copyLgth;		

		WriteEepromEX(protectContext, (BYTE*)&protect, sizeof(resizeprotect));
		
		IF_CommitTransaction(need_commit);
		
		while(protect.curraddr_scr <= protect.endaddr_scr)
		{
			length = protect.endaddr_scr - protect.curraddr_scr + 1;

			length = (length >= sizeof(tempBuf)) ? sizeof(tempBuf) : length;
			
			need_commit = IF_BeginTransaction();
			
			ReadEeprom(tempBuf,protect.curraddr_scr,length);
			
			WriteEepromEX(protect.curraddr_dst,tempBuf, length);
			protect.curraddr_scr += length;
			protect.curraddr_dst += length;			

			WriteEepromEX(protectContext, (BYTE*)&protect, sizeof(resizeprotect));
			
			IF_CommitTransaction(need_commit);
			
		}

		if(protect.endaddr_dst < protect.startaddr_scr)
		{
			SetEepromEX(protect.startaddr_scr, 0xFF, protect.endaddr_scr-protect.startaddr_scr+1);
		}
		else
		{
			SetEepromEX(protect.endaddr_dst+1, 0xFF, protect.endaddr_scr-protect.endaddr_dst);
		}
	}
	else if(protect.startaddr_dst > protect.startaddr_scr)//向高地址移动
	{
		
		length = protect.curraddr_scr-protect.startaddr_scr+1;
		
		copyLgth = GetPageOffset(protect.curraddr_dst)+1; 

	    copyLgth = (length > copyLgth)?copyLgth:length;
		
		need_commit = IF_BeginTransaction();		
		
		ReadEeprom(tempBuf,(protect.curraddr_scr-(copyLgth-1)),copyLgth);

		WriteEepromEX((protect.curraddr_dst-(copyLgth-1)),tempBuf,copyLgth);

		protect.curraddr_scr -= copyLgth;

		protect.curraddr_dst -= copyLgth;

		WriteEepromEX(protectContext, (BYTE*)&protect, sizeof(resizeprotect));
			
		IF_CommitTransaction(need_commit);
		
		while(protect.curraddr_scr>=protect.startaddr_scr)
		{

			length = protect.curraddr_scr-protect.startaddr_scr+1;
			length = (length >= sizeof(tempBuf)) ? sizeof(tempBuf) : length;

			need_commit = IF_BeginTransaction();
			
			ReadEeprom(tempBuf,(protect.curraddr_scr-(length-1)),length);

			WriteEepromEX((protect.curraddr_dst-(length-1)),tempBuf,length);

			protect.curraddr_scr -= length;

			protect.curraddr_dst -= length;

			WriteEepromEX(protectContext, (BYTE*)&protect, sizeof(resizeprotect));

			IF_CommitTransaction(need_commit);
		}


		if(protect.endaddr_scr < protect.startaddr_dst)
		{
			SetEepromEX(protect.startaddr_scr, 0xFF, protect.endaddr_scr-protect.startaddr_scr+1);
		}
		else
		{
			SetEepromEX(protect.startaddr_scr, 0xFF, protect.startaddr_dst-protect.startaddr_scr);
		}

	}

	protect.completed = 0xFF;
	WriteEepromEX(protectContext, (BYTE*)&protect, sizeof(resizeprotect));
}

/*-------------------------------------------------------------------------------------------------------//

-------------------------------------------------------------------------------------------------------------*/
void MoveHugeEepromRunning()
{
	BYTE i;
	for(i = 0; i <  BLOCKNUM; i++)
	{
		MoveOneBlock(i);
	}	
}





void SyncAddrIndexTableSubFunc(UWORD fileAddr,SDWORD size,BYTE mode,WORD index)
{
	UWORD xdata addrValue;
				
	addrValue = BuildUWord(tempBuf+index*4);//VMReadUword(E_ADDR_INDEX_TABLE + index * sizeof(UWORD));
			
	if(addrValue != 0xFFFFFFFF)
	{
		if((addrValue < fileAddr) || ((addrValue == fileAddr) && (mode == D_Resize)))
		{
			addrValue += size;

			SetUWord(tempBuf+index*4,addrValue);  //VMWriteUword(E_ADDR_INDEX_TABLE + index * sizeof(UWORD),addrValue);			
		}								
	}
}


/*
 *@Function: SyncAddrIndexTable
 *
 *@Descrip:	 文件删除或者resize后，同步地址索引表信息
 *
 *@Param:  	 void	  
 *			 
 *@Return:   void
 *
 *@Author:   zhxx 20200306
 *
 *@History:  V0.0--
 */
 void SyncAddrIndexTable(UWORD fileAddr,SDWORD size,BYTE mode)
{
	WORD idata index,blockNum,i,leftNum;
	
	blockNum = MAX_INDEX_NUM/0x80;
	
	leftNum = MAX_INDEX_NUM%0x80;
	
	for(i=0; i<blockNum; i++)
	{
		ReadEeprom(tempBuf,E_ADDR_INDEX_TABLE+i*0x200,sizeof(tempBuf));

		for(index=0; index < 0x80; index++)	//
		{			
			SyncAddrIndexTableSubFunc(fileAddr,size,mode,index);			
		}

		WriteEepromEX(E_ADDR_INDEX_TABLE+i*0x200,tempBuf,sizeof(tempBuf));
	}
	
	ReadEeprom(tempBuf,E_ADDR_INDEX_TABLE+i*0x200,4*leftNum);

	for(index=0; index < leftNum; index++)	//
	{		
		SyncAddrIndexTableSubFunc(fileAddr,size,mode,index);		
	}

	WriteEepromEX(E_ADDR_INDEX_TABLE+i*0x200,tempBuf,4*leftNum);
		 					
}
#endif


//#endif
