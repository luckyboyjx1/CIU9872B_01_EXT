#ifndef    _resume_h_
#define    _resume_h_

//#define D_HAVEDATA 0xA5
//#define D_NODATA   0x00

#define	   D_HAVEDATA         	0x005A                                       //有 断电数据 需要恢复标记
#define	   T_HAVEDATA         	0x006A                                       //有 事务数据 需要恢复标记

#define	   VALIDDATA            0xFFFFFFFF                                 //大/小标记有效
#define	   NOVALIDDATA          0x00000000                                 //大/小标记无效

typedef  __packed struct tagResume
{
    UWORD    startTag;														//  4    
    UWORD   eepAddr;                               //  4    
    UWORD   dataAddr;                                //  4   
    WORD    pages;                               //  2    
    UWORD    endTag;                               //  4    
	  WORD    crc;                                //  2    
	  UWORD    eraserTag;
}RESUME;

#define  eraserTagadd    (4*sizeof(UWORD)+2*sizeof(WORD))


typedef  __packed struct tagLLV
{
	UWORD   destntnAddr;
	UWORD   sourceAddr;
	WORD    lgth;
}BODYLLV;


typedef struct tagCOPY
{
    BYTE   startTag;					//  1    1 
    UWORD  idleAddr;
    BYTE   endTag;					//  4    1 
}sameLLV;


#define D_EEPageSize		0x80
#define D_MoveForward		0x01
#define D_MoveBackward		0x02
#define D_EEPMoveTag		0xA5
#define D_EEPNoMoveTag		0x5A
#define D_DeleteFile		0x40
#define D_Resize			0x80
#define D_CreateFile        0x20


typedef struct
{
	UWORD startaddr_scr;//源数据起始地址
	UWORD curraddr_scr;
	UWORD endaddr_scr;//源数据结束地址
	UWORD startaddr_dst;//目的空间起始地址
	UWORD curraddr_dst;
	UWORD endaddr_dst;//目的空间结束地址

	BYTE completed;//数据移动是否结束
	BYTE rfu1;
	BYTE rfu2;
	BYTE rfu3;
}resizeprotect;


#define BLOCKNUM 4


#define   		  HeapCleanUpTag_Offset           0x00
#define   		  moveTag_Offset           	     (sizeof(UWORD))
#define   		  dstAddr_Offset                 (sizeof(UWORD)*2)
#define   		  srcAddr_Offset                 (sizeof(UWORD)*3)
#define   		  moveSize_Offset                (sizeof(UWORD)*4)
#define   		  nextDataAddr_Offset            (sizeof(UWORD)*5)

#define   		  CleanUpProcessing_Tag			      0xA55AA55A	//正在进行碎片整理
#define   		  MoveData_Start_Tag			      0xAA55AA55	  //碎片整理流程中已打移动标记，正要移动数据/已经开始移动数据
#define   		  MoveData_End_Tag			          0x55AA55AA	  //碎片整理流程中所有数据已经移动完毕，地址索引表还未同步结束 
	
extern BYTE xdata G_BufferTag ;
extern BYTE xdata G_BlockTag ;
extern BYTE xdata G_PageTag ;

extern BYTE xdata G_Bank;


//#define WriteEepromEX(eepAddr, ramAddr, length) AccessEepromEX((eepAddr), (ramAddr), NULL, (length), (EEP_Write))

//#define SetEepromEX(eepAddr, HEX, length) AccessEepromEX((eepAddr), NULL, (HEX), (length), (EEP_Set))

//extern BYTE AccessEepromEX(UWORD eepAddr, BYTE * ramAddr, BYTE HEX, WORD length, BYTE mode);

extern BYTE CopyEepromEX(UWORD desAddr,UWORD sourseAddr,WORD  length );                       //带有断电保护功能的函数

extern void SyncAddrIndexTable(UWORD fileAddr,SDWORD size,BYTE mode);

extern void ResumeData(void);

extern void CopyEepromResume(void);

//extern BYTE SecureMoveEeprom(UWORD srcAddr, UWORD desAddr, UWORD dataLen, BYTE * dateBuf);

extern void MoveEepromResume(void);	 

extern void DeleteResume(void);

extern void ResizeResume(void);

extern void FileAdmiResume(void);

extern void EraseFileSystem(void);

extern void SetBank(void);

extern UWORD WriteFourBytes(UWORD eepAddr,BYTE * srcBuffer,BYTE length);

extern BYTE WriteWithoutErease(UWORD dstAddr,BYTE * srcBuffer,BYTE length);

extern UWORD GetFileContentSize(UWORD fileAddr);

extern void MoveHugeEepromEXPrepare(UWORD destination, UWORD source, UWORD length, BYTE blkIdx);

extern void MoveOneBlock(BYTE blkIdx);

extern void MoveHugeEepromRunning(void);

#endif
