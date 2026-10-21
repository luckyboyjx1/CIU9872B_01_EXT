#ifndef    _tlv_h_
#define    _tlv_h_
#include   "typedef.h"
//#include   "eepromdefine.h"


//       ___________________________________findTagLen
//      |    _______________________________findLenLen
///     |    |   ___________________________findValueLen
//      |    |   |          ________________findNextLen
//      |    |   |          |
//     __   __   __   ______________
//    |  | |  | |  | |              |
///eg:BF20 81FE XXXX BF21 81F2 XXXXXX
///   |         |    |
//    |         |    |______________________findNextOff
//  	|         |___________________________findValueOff
//  	|_____________________________________findTagOff

/*
 * 各种结构的定义
*/

typedef __packed struct TlvAttrInfo					//tlv属性信息
{
	UWORD bufferAddr;
	WORD  bufferLen;
	WORD  findTagOff;
	BYTE  findTagLen;
	BYTE  findLenLen;
	WORD  findValueOff;	
	WORD  findValueLen;	
	WORD  findNextOff;	
	SWORD  findNextLen;//实际剩余长度	
	WORD  findTLVLen;
	BYTE  levelIndex;  //tag的层级
	BYTE  tagBuffer[6];
	WORD  offBuffer[6];
	WORD  lenBuffer[6];
	WORD  valueLenBuffer[6];
	BYTE  isCompleteBuffer[6];
}TlvInfo;

extern jbool tlvPathConstCompare(const BYTE* destBuffer,BYTE destLen);
extern void initFindTlv(TlvInfo* tlv);
extern void initTlvInfo(TlvInfo* tlv, UWORD bufAddr, WORD bufSize);
extern void appendBuffer(BYTE* buf,WORD len);
extern void setVerticalFind(TlvInfo * tlv);
extern BYTE setReturnFind(TlvInfo * tlv);
extern WORD appendFindValue(TlvInfo * tlv,BYTE mode);

extern void updateTLVBuffer(TlvInfo* tlv, WORD moveOff, BYTE * outData, WORD outDataLen, BYTE * inData, WORD inDataLen);
extern BYTE findTLV(TlvInfo* tlv, BYTE tag, BYTE mask, BYTE occur);
extern void replaceFindTlv(BYTE tag, TlvInfo* tlv, BYTE * buf, WORD len);

extern void replaceFindTlvInRam(BYTE tag, BYTE * buf, WORD len);

extern BYTE checkProprietaryEFInfo(void);
extern BYTE replaceGivenTLV(void);
extern BYTE replaceTLVFromTLVList(void);

extern BYTE getLenLen(UWORD buffer);

#define FromTag 0
#define FromValue 1



//#define InvalidTag  1
//#define NoData      2   //后续没有数据，或者只有1个字节的tag
//#define OnlyTag     3   //只收到2个字节的tag
//#define OnlyLenFlay 4   //只收到1个字节的tag和1个字节的长度标识
//#define ValidTag    5   //有效的tag


#endif

