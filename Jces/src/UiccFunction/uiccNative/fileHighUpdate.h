#ifndef    _fileHighUpdate_h_
#define    _fileHighUpdate_h_
#include	"uicc_main.h"
#include	"typedef.h"

extern BYTE xdata    G_High_Update_NUM;       //强更新配置页数
extern BYTE xdata    G_High_Update_Times[20]; //暂定最大支持20个强更新文件
extern BYTE xdata    G_High_Update_RAM[40];

#define Hige_UpdateWithRAM       0x80//强更新 在RAM区域  不在E2 比如6F5B  910180 tag
#define Hige_UpdateWithEep       0x7F//强更新在E2                         9101XX tag 

#define Hige_Update_mode           0x00            //Write更新  
#define Hige_Read_mode             0x01            // 读模式
#define Hige_Set_mode              0x02            //Set模式

#define High_UpdateNum_Switch    100   //强更新100次后 切换备份文件
//#define Cal_CRC_Int    0x01
//#define Cal_CRC_USE    0x02
extern BYTE code High_Upadte_Tag[2];
extern BYTE IF_SupportHighUpdate(BYTE updateTag);
extern WORD IF_Cal_High_Upadte_CRC(BYTE *ptr,BYTE calsize);
extern BYTE IF_TFUpdate_Smallblock(UWORD addrDst);
extern void IF_Init_High_Update_Config(BYTE index,BYTE *config);
extern UWORD GetAddrFormHigh_Update_START(UWORD assignSize,BYTE* index,BYTE updatetag,BYTE filetype,BYTE * HighUpdateCfg);
extern UWORD IF_GetHigtUpdateAddr(UWORD  fileAddr,UWORD  headAddr);
extern void IF_Init_High_Update_Area(UWORD fileBodyAddr,WORD bodySize,BYTE updatetag);
extern void InitUpdateVariable(void);
extern void IF_Switch_High_Update_START(void);
extern UWORD IF_GetUpdate_By_index(BYTE index);
extern void IF_InitFileResize(UWORD  fileAddr,WORD  filesize,BYTE index);
extern BYTE IF_UpdateFile_Resize(UWORD oldfilesize,UWORD newfilesize,BYTE index);
extern void IF_GetConfig_ByIndex(BYTE index,BYTE *config);
extern BYTE IF_UpdateOrReadWithRAM(UWORD fileAddr,WORD offset, BYTE * buf, WORD len,BYTE mode);
extern BYTE IF_Update_SmallBlock(UWORD  fileAddr,UWORD  headAddr,BYTE * buf,WORD offset, BYTE lgth,BYTE mode);
extern void CopyFileContent(UWORD addrDst,UWORD addrSrc,BYTE pagenum);

#endif
