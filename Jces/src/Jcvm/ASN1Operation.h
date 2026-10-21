#ifndef    _ASN1OPERATION_H   
#define	   _ASN1OPERATION_H 
#include "ecpDefine.h"

/*****************************universe tag********************************************/
//REC X680
#define   ASN1_UNIVERCLASS_BOOLEAN          0x01
#define   ASN1_UNIVERCLASS_INTEGER          0x02
#define   ASN1_UNIVERCLASS_BITSTRING        0x03
#define   ASN1_UNIVERCLASS_OCTETSTRING      0x04
#define   ASN1_UNIVERCLASS_NULL             0x05
#define   ASN1_UNIVERCLASS_OBJECTIDENTIFIER 0x06
#define   ASN1_UNIVERCLASS_SEQUENCE         0x30
#define   ASN1_UNIVERCLASS_SEQUENCEOF       0x30
#define   ASN1_UNIVERCLASS_SET              0x31
#define   ASN1_UNIVERCLASS_SETOF            0x31
#define   ASN1_UNIVERCLASS_PrintableString  0x13


#define   TAG_NOT_FOUND                     0xFFFF
#define   ASN1_INTEGER_CODE_ERROR           0xFFFFFFFF

/************************************macro define*************************************/

//       ___________________________________curTagLen
//      |    _______________________________curLenLen
///     |    |   ___________________________curValueLen
//      |    |   |          ________________findNextLen
//      |    |   |          |
//     __   __   __   ______________
//    |  | |  | |  | |              |
///eg:BF20 81FE XXXX BF21 81F2 XXXXXX
///   |         |    |
//    |         |    |______________________findNextOff
//    |         |___________________________curValueOff
//    |_____________________________________curTagOff


#define ASN1_DEPTH                0x06   //假设支持ASN.1的深度最大为6级
typedef struct
{
	u32 bufferAddr;
	u16  bufferLen;
	u16  curTagOff;     //当前ASN.1 tag 在bufferAddr 中的偏移
	u8  curTagLen;     //当前ASN.1 tag本身所占的字节数
	u8  curLenLen;     //当前ASN.1 len字段本身所占的字节数
	u16  curValueOff;	 //当前ASN.1 tag中的Value相对于bufferAddr的偏移
    u16  curValueLen;	 //当前ASN.1 tag中的Value的长度
	u16  curTLVLen;     //当前ASN1 TLV 的总长度 = curTagLen + curLenLen+curValueLen
	u16  nextTagOff;	 //下一次要查找的起始偏移
    u16  nextTotalLen;	 //下一次要查找数据的总长度
	u8  curDepth;      //tag的已遍历深度
	u8  curDepthTag[ASN1_DEPTH];    //当前每一个深度的当前 tag值
	u16  remainDataOff[ASN1_DEPTH];    //每一个深度的下一次查找 tag 的起始偏移
	u16  remainDataLen[ASN1_DEPTH];    //每一个深度的剩余待查找数据长度
}ASN1Info;


/************************************function define**********************************/
extern void ASN1_InitTLVInfo(ASN1Info * asn1Info,u32 addr, u16 len);
extern u8 ASN1_FindTLV(ASN1Info * asn1Info, u8 tag , u8 nextMode, u8 tagOccur);
extern void ASN1_SetNextDepthToFindTLV(ASN1Info * asn1Info);
extern void ASN1_ReturnFromDepthToFindTLV(ASN1Info * asn1Info);


extern u16 ASN1_FindTagTLV(u32 srcAddr,u16 Len, u16 tag);
extern u8 ASN1_PaddingBERTlvTagAndLen(u8* buf,u16 tag,u32 len);
extern u32 ASN1_GetBERTlvData(u32 srcAddr,u8* buf, u32 validLen);
extern u8 ASN1_GetBERTlvTag(u32 srcAddr,u8* tag);
extern u8 ASN1_GetBERTlvLen(u32 srcAddr,u32* len);

extern void ASN1_GenIntegerLV(u32 seqNumber, u8* bufLV);
extern u32 ASN1_CheckIntegerLV(u8* bufLV);

#endif

