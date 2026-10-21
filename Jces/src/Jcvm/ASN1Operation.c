/*
*@author      : xuyz
* 
*@copyright   : eastcompeace

*@description : 该文件主要用于解析ASN.1 格式的编码
*
*@history     :
*
*@
*/
#include "ASN1Operation.h"
#include "cardManager.h"
#include "MemOperation.h"
#include "error.h"		//remove warning,dengyiwen,2022/9/27

#if  EUICC_FUNC_ASN1_CODING
/*
*@function     :ASN1_CheckIntegerLV 
*
*@description  :校验生成UWROD数据的u8数组的数据是否正确，LV结构，进入该函数时已经确保bufLV中数据长度不会越界
*
*@see          :<ASN1 BER DER 编码子集入门指南.pdf 5.7 INTEGER> 
*
*@param        :u8* bufLV     -- 输入参数，用于生成memref数据的u8数组，LV结构
*
*@return       :memref           -- 格式正确则返回对应的memref数据，否则返回错误0xFFFFFFFF，这里是要求memref最大为0xFFFFFFFE
*
*@RAM Resources:
*
*@author       :xuxiaojun@20190102
*
*@hisotry      :created by xuxiaojun@20190102
*/
memref ASN1_CheckIntegerLV(u8* bufLV)
{
	memref I_A seqNumber, result;

	result = ASN1_INTEGER_CODE_ERROR;
	
	seqNumber = 0;
	
	switch(bufLV[0])  //传入的sequnce number的字节数， 在这里没有对02 0001这种情况进行处理，认为其是合法的。
	{
		case 0x01:
		{
			seqNumber = bufLV[1];
			
			if(seqNumber > 0x7F) // seqNumber编码非法
			{
				return result;
			}
			break;
		}
		case 0x02:
		{
			seqNumber = ((bufLV[1]<<8)|bufLV[2]);
			
			if((seqNumber < 0x80) || (seqNumber > 0x7FFF))   // seqNumber编码非法
			{
				return result;
			}
			break;
		}
		case 0x03:
		{
			seqNumber = ((bufLV[1]<<16)|(bufLV[2]<<8)|bufLV[3]);
			
			if((seqNumber < 0x8000) || (seqNumber > 0x7FFFFF))   // seqNumber编码非法
			{
				return result; 
			}
			break;
		}
		case 0x04:
		{
			seqNumber = ((bufLV[1]<<24)|(bufLV[2]<<16)|(bufLV[3]<<8)|(bufLV[4]));
			
			if((seqNumber < 0x800000) || (seqNumber > 0x7FFFFFFF))   // seqNumber编码非法
			{
				return result;
			}
			break;
		}
		case 0x05:
		{
			seqNumber = ((bufLV[2]<<24)|(bufLV[3]<<16)|(bufLV[4]<<8)|(bufLV[5]));
				
			if((bufLV[1] != 0x00) || (seqNumber < 0x80000000)|| (seqNumber > 0xFFFFFFFE))   // seqNumber编码非法
			{
				return result;
			}
			break;
		}
		default:
		{
			return result;
		}
	}
	return seqNumber;
}


/*
*@function     :ASN1_GenIntegerLV 
*
*@description  :生成INTEGER类型数据的LV结果
*
*@see          :<ASN1 BER DER 编码子集入门指南.pdf 5.7 INTEGER> 
*
*@param        :memref seqNumber -- 输入参数，需要被转换成lv结构的memref类型数据值
*@param        :u8* bufLV     -- 输出参数，生成的memref组成的u8数组
*
*@return       :void
*
*@RAM Resources:
*
*@author       :xuxiaojun@20181126
*
*@hisotry      :created by xuxiaojun@20181126
*/
void ASN1_GenIntegerLV(memref seqNumber, u8* bufLV)
{
	u8 I_A offset;
	
	offset = 0;
	
	if(seqNumber <= 0x7F)       // 017F
	{
		bufLV[offset++] = 0x01;
		
		bufLV[offset++] = (seqNumber&0xFF);
	}
	else if(seqNumber <= 0xFF)  // 02 00 80 -- 02 00 FF
	{
		bufLV[offset++] = 0x02;
		
		bufLV[offset++] = 0x00;
		
		bufLV[offset++] = (seqNumber&0xFF);
	}
	else if(seqNumber <= 0x7FFF)// 02 0100 -- 02 7FFF
	{
		bufLV[offset++] = 0x02;
		
		bufLV[offset++] = ((seqNumber>>8)&0xFF);

		bufLV[offset++] = (seqNumber&0xFF);
	}
	else if(seqNumber <= 0xFFFF)// 03 00 8000 -- 03 00 FFFF
	{
		bufLV[offset++] = 0x03;
		
		bufLV[offset++] = 0x00;
		
		bufLV[offset++] = ((seqNumber>>8)&0xFF);

		bufLV[offset++] = (seqNumber&0xFF);
	}	
	else if(seqNumber <= 0x7FFFFF) // 03 010000 -- 03 7FFFFF
	{
		bufLV[offset++] = 0x03;
		
		bufLV[offset++] = ((seqNumber>>16)&0xFF);

		bufLV[offset++] = ((seqNumber>>8)&0xFF);
		
		bufLV[offset++] = (seqNumber&0xFF);
	}
	else if(seqNumber <= 0xFFFFFF) // 04 00 800000 -- 04 00 FFFFFF
	{
		bufLV[offset++] = 0x04;
		
		bufLV[offset++] = 0x00;
		
		bufLV[offset++] = ((seqNumber>>16)&0xFF);

		bufLV[offset++] = ((seqNumber>>8)&0xFF);
		
		bufLV[offset++] = (seqNumber&0xFF);
	}
	else if(seqNumber <= 0x7FFFFFFF) // 04 01 000000 -- 04 7FFFFFFF
	{
		bufLV[offset++] = 0x04;
		
		bufLV[offset++] = ((seqNumber>>24)&0xFF);
		
		bufLV[offset++] = ((seqNumber>>16)&0xFF);

		bufLV[offset++] = ((seqNumber>>8)&0xFF);
		
		bufLV[offset++] = (seqNumber&0xFF);
	}
	else // if(seqNumber <= 0xFFFFFFFF) // 05 00 80000000 -- 05 00 FFFFFFFF
	{
		bufLV[offset++] = 0x05;
		
		bufLV[offset++] = 0x00;
		
		bufLV[offset++] = ((seqNumber>>24)&0xFF);
		
		bufLV[offset++] = ((seqNumber>>16)&0xFF);

		bufLV[offset++] = ((seqNumber>>8)&0xFF);
		
		bufLV[offset++] = (seqNumber&0xFF);
	}
}
/*
*@function     :ASN1_GetBERTlvTag 
*
*@description  :获取BER编码规则的TLV中，T域占用字节数，以及T的值
*
*@see          :  
*
*@param        :memref srcAddr -- 输入参数，指向 BER TLV编码TAG的起始
*@param        :u8* tag        -- 输出参数，BER TLV结构中的TAG的值, 最大2字节
*
*@return       :u8          -- 返回T域占用的字节数,即u8* tag中有效数据的字节数
*
*@hisotry      :V1.0 created by xuxiaojun@20190521
*注意          :调用该函数时要求u8* tag至少有2字节，目前调用时都可以保证
*/
u8 ASN1_GetBERTlvTag(memref srcAddr,u8* tag)
{
	memCopyToRam(MEMtPX8(&tag[0]),MEMtPM8(srcAddr),2);
	
	if((tag[0]&0x1F) == 0x1F) //ASN1 编码规则，tag值超过30 需要用2字节编码，高字节的低5比特固定为”1“
	{
		return 2;
	}
	else
	{
		return 1;
	}
}
/*
*@function     :ASN1_GetBERTlvLen 
*
*@description  :获取BER编码规则的TLV中，L域占用字节数，以及L的值即value占用的字节数
*
*@see          :  
*
*@param        :memref srcAddr -- 输入参数，指向 BER TLV编码Len的起始即Tag后第一字节，可能是 xx/81 xx /82 xxxx / 83 xxxxxx
*@param        :memref* len    -- 输出参数，BER TLV结构中的Len值
*
*@return       :u8          -- 返回L域占用的字节数
*
*@RAM Resources:
*
*@hisotry      :V1.0 created by lgy@20181113
*              :V1.1 modify by xuxiaojun@20190326  eUICC test RSP 20190301\TY_Offline_RSP_ManageCmdTest \ Download_260K_size_profile 要求下载260K的profile，传入的长度为83 029696
*/
u8 ASN1_GetBERTlvLen(memref srcAddr,memref* len)
{
	u8 X_A temp[4];
	memref I_A tempLen;
	u8 I_A lenNum;

	tempLen	= 0x00;

	lenNum = 0x00;

	memCopyToRam(MEMtPX8(&temp[0]),MEMtPM8(srcAddr),4); //len最大可能为4字节  83 xx xx xx

	if(temp[0] <= 0x80)
	{
		tempLen = temp[0];

		lenNum = 1;
	}
	else if(temp[0] == 0x81)
	{
		tempLen = temp[1];

		lenNum = 2;
	}
	else if(temp[0] == 0x82)
	{
		tempLen = (((temp[1]<<8)&0xFF00)|temp[2]);

		lenNum = 3;
	}
	else if(temp[0] == 0x83)
	{
		tempLen = (((temp[1]<<16)&0xFF0000)|((temp[2]<<8)&0xFF00)|temp[3]);

		lenNum = 4;
	}
	else	//HB_Offline_Test/5KeyEstablish指令异常测试/Tele_TC9_ISDR_KeyEstablishment_ERROR_01/步骤10、ISD-R,EstablishISDRKeySet cmd1 cert len error
	{
		throw_error(0x6A80);//throw_error(ASN1_Exception);
	}

	*len = tempLen;

	return lenNum;
	
//	return GetBERTlvLen(srcAddr,len); 该函数在public.c中定义,放开之后这里分层有问题
}

/*
*@function     :ASN1_GetBERTlvData 
*
*@description  :获取某个tag的内容缓存到buf ，包括 tag len value
*
*@see          :  
*
*@param        :memref srcAddr  -- 输入参数, 指向 BER TLV编码的TLV中T的起始(单字节TAG/2字节TAG)
*@param        :u8* 输出数据 -- 输出参数, 获取到的TLV数据
*@param        :u16 validLen  -- 输入参数, u8* buf中可使用的字节数
*
*@return       :memref -- buf越界返回FALSE；否则返回获取到的某个TLV的总长度，包括T L V
*
*@RAM Resources:
*
*@hisotry      :V1.0 created by lgy@20181113
*              :V1.1 modify by xuxiaojun@20190522 支持2字节TAG
*              :V1.2 modify by xuxiaojun@20190627 增加参数memref validLen用于避免函数内部越界
*/
memref ASN1_GetBERTlvData(memref srcAddr, u8* buf, memref validLen)
{
	memref I_A valueLen;
	u8 I_A lenLen,tagLen;
	u8 I_A tag[2];
	
	tagLen = ASN1_GetBERTlvTag(srcAddr, tag);
	
	lenLen = ASN1_GetBERTlvLen(srcAddr + tagLen, &valueLen);
	
	if((tagLen + lenLen + valueLen) > validLen)
	{
		return FALSE;
	}
	
	memCopyToRam(buf, MEMtPM8(srcAddr), tagLen + lenLen + valueLen);
	
	return (tagLen + lenLen + valueLen);
}

/*
*@function     :ASN1_PaddingBERTlvTagAndLen 
*
*@description  :根据tag和len拼装BER TLV的 tag len
*
*@see          :  
*
*@param        :u8* buf -- 输出参数，拼装后数据存放的地址 RAM*, 可能是:2字节TAG + 00-0x7F / 8180..81FF / 820100 .. 82FFFF, 最多5字节
*@param        :u16 tag  -- 输入参数，需要拼装的tag 支持2字节tag , tag为1字节时 高位为0 （这里的tag为ASN1根据特定规则编码后的tag） 例： 这里传入的是 0xBF2D ,而不是十进制 45  ; ProfileInfoListResponse ::= [45] CHOICE { -- Tag 'BF2D'
*@param        :u16 len  -- 输入参数，数据长度
*
*@return       :u8 len  -- 编码后tag和len占用的字节数
*
*@RAM Resources:无
*
*@author       :lgy@20181116
*
*@hisotry      :V1.0
*              :V1.1 在个人化预制接近0x7FFF长度的证书测试中，传入数据长度 len 大于2字节，修改为4字节，modify by xieyong@20210827
*注意          :u8* buf中的可用字节数不小于最后的返回字节数，否则就可能会溢出
*/
u8 ASN1_PaddingBERTlvTagAndLen(u8* buf,u16 tag,memref len)
{
	u8 I_A i;
	
	i = 0 ;
	
	if(((tag>>8)&0x1F) == 0x1F)	//根据ASN1 ，tag大于30 需要用2字节编码，高字节的bit0-bit4设置为1
	{
		buf[i++] = ((tag>>8)&0xFF); 		
	}
	
	buf[i++] = (tag&0xFF);	
	
	if(len < 0x80)
	{			
		buf[i++] = len;
	}
	else if(len <= 0xFF)
	{
		buf[i++] = 0x81;
		buf[i++] = len;
	}
	else if(len <= 0xFFFF)
	{
		buf[i++] = 0x82;
		buf[i++] = ((len>>8)&0xFF);
		buf[i++] = (len&0xFF);
	}
	else
	{
		buf[i++] = 0x83;
		buf[i++] = ((len>>16)&0xFF);
		buf[i++] = ((len>>8)&0xFF);
		buf[i++] = (len&0xFF);
	}
	
	return i;
}

/*
*@function     : ASN1_FindSingleTagTLV 
*
*@description  : 从srcAddr作为起始 在Len范围内查找 “tag” 标识的数据
*
*@see          :  BF 20 XX XX tag1 xx value  tag2 xx value  tag3 xx value  
                              |(srcAddr)--------------------|offset
							  
			   ：例： 在BF20下查找tag3 ，传入地址为BF20 tag的value起始地址(srcAddr)，
			   ：查找到tag3以后返回的offset是相对于(srcAddr)的偏移，srcAddr + offset指向tag3的tag值所在地址
*
*@param        :memref srcAddr --- 输入参数, 待查找数据的起始地址
*@param        :u16 Len      --- 输入参数, srcAddr开始的待查找数据长度
*@param        :u16 tag      --- 输入参数, 待查找的TAG
*
*@return       :u16 offset   --- 未找到返回TAG_NOT_FOUND, 否则返回查找的“tag”的T域起始相对于srcAddr的偏移量
*
*@RAM Resources:
*
*@author       :lgy@20181113
*
*@hisotry      :V1.0
*			   :V1.1 支持2字节tag的查找  ，modify by lgy@2018.12.18
*/

u16 ASN1_FindTagTLV(memref srcAddr,u16 len,u16 tag)
{
	u8 I_A temp[2];
	u16 I_A offset,tempTag;
	memref X_A tempLen;
	u8 I_A paddingLen;
		
	for(offset = 0x00;offset < len ;)
	{
		paddingLen = ASN1_GetBERTlvTag(srcAddr + offset, temp);
		
		if(paddingLen == 0x01) // 单字节TAG
		{
			tempTag = temp[0];
		}
		else // 双字节TAG
		{
			tempTag = MEM_R_U16((memref)temp); //BuildWord(&temp[0]);  modify by xuxiaojun@20220521 代码分层，使用平台层函数，而BuildWord是uicc层定义的
		}
		
		if(tempTag == tag)
		{
			return offset;
		}	

		temp[0] = ASN1_GetBERTlvLen(srcAddr + offset + paddingLen, (memref*)&tempLen);
		
		if(temp[0] > 0x03) // 不支持83xxxxxx
		{
			return TAG_NOT_FOUND;
		}
		
		offset += (paddingLen + temp[0] + tempLen);
	}
	
	return TAG_NOT_FOUND;
}
/*
*@function    :ASN1_InitTLVInfo
*
*@descrition  :初始化ASN1 编码的结构体，以便后续进行tag的遍历查找 ,将初始化信息写入asn1Info结构体中
*
×@param        :ASN1Info * asn1Info
*@param        :memref addr:  整个ASN1编码的Value域的起始
*@param        :u16 len  :  整个ASN1编码的Value的总长度
*
*@return       :void
*
*@test         :NO
*
*@author       :xuyz
*
*@history      :V1.0 :create @2018112
*/
void ASN1_InitTLVInfo(ASN1Info * asn1Info,memref addr, u16 len)
{
	ecpMem_Fill((u8 *)asn1Info, 0x00, sizeof(ASN1Info));
	
	asn1Info->bufferAddr = addr;
	
	asn1Info->bufferLen = len;
	
	asn1Info->nextTagOff = 0;
	
	asn1Info->nextTotalLen = len;

	asn1Info->curDepth = 0;
}
/*
*@function    : ASN1_FindTLV
*
*@descirption : 在一个符合ASN.1的数据体中查找 tag，， 该函数查找的是同级别TAG
*
*@param       : ASN1Info * asn1Info
*@param       : u8   tag      :待查找的tag,当查找的tag为2个字节时(即tagNum > 30)，第一个字节的tag为类型，高3bit无法完全匹配
*@param       : u8   nextMode :是否查找下一个tag，此字段为TURE时 ，则直接返回下一个tag，参数 u8 tag无效
×@param       : u8   tagOccur :tag出现的次数
*
*@return      :TRUE:  tag找到 /FALSE: tag未找到
*
*@author      :xuyz
*
*@history     :V1.0 :create @20181114
*              该函数最多支持2个字节的ASN.1 tag ; 最多支持最大长度为FFFF的L
*/
u8 ASN1_FindTLV(ASN1Info * asn1Info,u8 tag , u8 nextMode, u8 tagOccur)
{
	u8 I_A tmpTag;
	u8 I_A firstLenByte;
	u16 I_A lastLen;
	
	if(asn1Info->curDepth >= ASN1_DEPTH)
	{
		throw_error(ASN1_Exception);
	}
	
	while (asn1Info->nextTotalLen >= 2)
	{
		asn1Info->curTagOff = asn1Info->nextTagOff;
		
		tmpTag = memReadU8(MEMtPM8(asn1Info->bufferAddr + asn1Info->curTagOff));
		
		if((tmpTag & 0x1F) == 0x1F)    //根据ASN.1 BER tag 编码规则：Identifier octets，有两种形式：较小的标签值（标签值在0和30之间）和较大的标签值（标签值大于等于31） //Low-tag-number form：一个字节。Bit8和bit7表示类（如表2），bit6值为0，表示编码方法为简单化的。Bit5－1给出了标签值//High-tag-number form：两个或多个octet。第一个octet形式如low-tag-number form，但是bit5－1均为1。第二个和以后的字节给出标签值，基于128，最高位在先，以便使用尽可能少的数字，除了最后一个字节以外，每个字节的bit 8都置为1。最后一个字节的为0。	
		{
			asn1Info->curTagLen = 2;	//tag占用2字节
			
			tmpTag = memReadU8(MEMtPM8(asn1Info->bufferAddr + asn1Info->curTagOff + 1));
			
			tmpTag &= 0x7F;
			
			if(tmpTag <= 30)   //tag为两个字节，但是tagNumber不对
			{
				return FALSE;
			}
		}
		else
		{
			asn1Info->curTagLen = 1;	//tag占用1字节
			
			if((tmpTag&0x1F) > 30)
			{
				return FALSE;  //tag为1个字节时，tagNumber不能超过30
			}
		}
		
		firstLenByte = memReadU8(MEMtPM8(asn1Info->bufferAddr + asn1Info->curTagOff + asn1Info->curTagLen));  //读取Len的第一字节
		
		asn1Info->curLenLen = ((firstLenByte & 0x80) == 0x80) ? ((firstLenByte & 0x7F) + 1) : 1;   // Length octets：有两种格式：短型（长度在0至127之间）和长型（长度在0至21008?1之间）// Short form: 一个字节，bit8为0，bit 7－1表示长度。// Long form: 2－127个字节。第一个字节的Bit 8为1，bit 7－1表示后面有多少个用于表示实际长度的octet。第二个和随后的octet给出实际长度，基于256，高位数字在先。	
		
		switch(asn1Info->curLenLen)
		{
			case 1:
			{
				asn1Info->curValueLen = firstLenByte;
				
			    break;
			}
			case 2:
			{
				firstLenByte = memReadU8(MEMtPM8(asn1Info->bufferAddr + asn1Info->curTagOff + asn1Info->curTagLen + 1));
			
			    asn1Info->curValueLen = firstLenByte;
				
				break;
			}
			case 3:
			{
			    asn1Info->curValueLen = memReadU16(MEMtPM8(asn1Info->bufferAddr + asn1Info->curTagOff + asn1Info->curTagLen +1));
				
				break;
			}
			
			default:   //该ASN.1 编码暂时不支持超过3
				
				
				return FALSE;
		}
		
		asn1Info->curValueOff = asn1Info->curTagOff + asn1Info->curTagLen + asn1Info->curLenLen;  //point to the Tag Value Offset
		
		asn1Info->curTLVLen = asn1Info->curValueLen + asn1Info->curTagLen + asn1Info->curLenLen;   //当前TLV所占的总的字节数
		
		if(asn1Info->nextTotalLen < asn1Info->curTLVLen) //当前tag所占的字节数已经超出了剩余数据的总长度
		{
			return FALSE;//找到的Len已经超出buffer的边界
		}
		
		asn1Info->nextTagOff =  asn1Info->curValueOff + asn1Info->curValueLen;                    //point to the Next Tag Offset	

		lastLen = asn1Info->nextTotalLen - asn1Info->curTLVLen;      //获取剩余TLV的总长度
		
		asn1Info->nextTotalLen = lastLen;
		
		asn1Info->curDepthTag[asn1Info->curDepth] = tmpTag;  //登记当前查找到的tag到 tlv->levelIndex中
		
		asn1Info->remainDataOff[asn1Info->curDepth] = asn1Info->nextTagOff;  //登记Next tag的偏移
		
		asn1Info->remainDataLen[asn1Info->curDepth] = asn1Info->nextTotalLen;    //登记剩余TLV 的总长度
		
		if ((tag  == tmpTag )||(nextMode  == TRUE))
		{
			tagOccur --;
			
			if(tagOccur == 0)
			{
				return TRUE;
			}		
		}
		
	}
	
	return FALSE;
}
/*
*@function   :  ASN1_SetNextDepthToFindTLV
*
*@description:  从当前深度的 ASN1 编码 进入它的下一级进行下一深度ASN1 编码的遍历查找
*
*@param      : ASN1Info * asn1Info
*
*@return     : void
*
*@test       :
*
*@author     :xuyz
*
*@history    :V1.0 :create @20181115
*/
void ASN1_SetNextDepthToFindTLV(ASN1Info * asn1Info)
{
	asn1Info->nextTagOff = asn1Info->curValueOff;
	
	asn1Info->nextTotalLen = asn1Info->curValueLen;
	
	asn1Info->curDepth++;
}

/*
*@function  :ASN1_ReturnFromDepthToFindTLV
*
*@descritpion:从上一深度回退到当前深度进行继续进行后续ASN1编码的遍历
*
*@param      :
*/


void ASN1_ReturnFromDepthToFindTLV(ASN1Info * asn1Info)
{
	if(asn1Info->curDepth <= 0)   //异常，原则上不可能
	{
		throw_error(ASN1_Exception);
	}
	
	asn1Info->curDepth--;
	
	asn1Info->nextTagOff = asn1Info->remainDataOff[asn1Info->curDepth];
	
	asn1Info->nextTotalLen = asn1Info->remainDataLen[asn1Info->curDepth];
	
	return ;
}

#endif

