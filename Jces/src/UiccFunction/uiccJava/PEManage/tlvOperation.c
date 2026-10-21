/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   tlvOperation.c
*
* Description: 该文件主要实现了tlv操作的基本函数
*
* auther：     zxx    20170623
*
* history：    V1.0
*
************************************************************************************************************/


#include  "typedef.h"
#include "fileAction.h"
#include "ProfileInterpreter.h"
#include  "tlvHeader.h"
#include  "profileApi.h"
#include  "VMAccessEeprom.h"
#include  "Gsmpublic.h"

#if (EUICC_ONLY||SE_AND_EUICC)
/*
*@funtion    :initTlvInfo
*
*@description: 初始化tlv信息
*
*@param      : TlvInfo*  tlv          --需要初始化的tlv
*              UWORD     bufAddr      --初始化的buffer地址
*              WORD      bufSize      --初始化的buffer大小
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*/
void initTlvInfo(TlvInfo* tlv, UWORD bufAddr, WORD bufSize)
{
	tlv->bufferAddr = bufAddr;
	
	tlv->bufferLen = bufSize;
	
	tlv->findNextOff = 0;
	
	tlv->findNextLen = bufSize;

	tlv->levelIndex = 0;
}


/*
*@funtion    :initFindTlv
*
*@description: 初始化查找的tlv
*
*@param      : TlvInfo*  tlv          --需要初始化的tlv

*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*/
void initFindTlv(TlvInfo* tlv)
{
	tlv->findNextOff = 0;
	
	tlv->findNextLen = tlv->bufferLen;
	
	tlv->findTagLen = 1;
	
	tlv->levelIndex = 0;
}


/*
*@funtion    :setVerticalFind
*
*@description: tlv嵌套时设置垂直查找的信息
*
*@param      : TlvInfo*  tlv          --需要设置垂直查找的tlv

*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*/
void setVerticalFind(TlvInfo * tlv)
{
	tlv->findNextOff = tlv->findValueOff;
	
	if(tlv->isCompleteBuffer[tlv->levelIndex] == TRUE)
	{
		tlv->findNextLen = tlv->findValueLen;
	}
	else
	{
		tlv->findNextLen = tlv->bufferLen - tlv->findValueOff;
	}
	
	tlv->levelIndex++;
}


/*
*@funtion    :setReturnFind
*
*@description: tlv嵌套结束时返回上一层的设置
*
*@param      : TlvInfo*  tlv          --需要返回的tlv

*@return     :BYTE    --返回上一层    TRUE: levelIndex还未到栈底  
*                                     FALSE: levelIndex已经到栈底      
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*/
BYTE setReturnFind(TlvInfo * tlv)
{
	if(tlv->levelIndex <= 0) 
	{
		return FALSE;
	}
	
	tlv->levelIndex--;
	
	tlv->findNextOff = tlv->offBuffer[tlv->levelIndex];
	
	tlv->findNextLen = tlv->lenBuffer[tlv->levelIndex];
		
	return TRUE;
}



/*
*@funtion    :appendFindValue
*
*@description: 将find的tlv追加到目的buffer中,返回拷贝的长度
*
*@param      : TlvInfo*  tlv              --find数据的tlv
*              TlvInfo*  desbuffer        --追加的数据tlv
*              BYTE      mode             --mode为FromTag：表示从查找到的tag开始拷贝
*                                         --mode为FromValue：表示只拷贝查找的值
*@return     : WORD    --追加的数据长度
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
WORD appendFindValue(TlvInfo * tlv,BYTE mode)
{
	WORD idata copyLen;
	WORD idata copyOff;
	
	if(mode == FromTag)
	{
		copyLen = tlv->findNextOff-tlv->findTagOff;
		
		copyOff = tlv->findTagOff;
	}
	else //if(mode == FromValue)
	{
		copyLen =  tlv->findValueLen;
		
		copyOff = tlv->findValueOff;
	}
	
	if((G_TempLen + copyLen) > sizeof(tempBuf))
	{
		appendErrorResponse(NOT_ENOUGH_MEMORY,G_PEControlContext.PE_identification);
		return FALSE;
	}
	
	
	VMReadBuf(tempBuf+G_TempLen, tlv->bufferAddr + copyOff, copyLen);
	
    G_TempLen += copyLen;
	
	return copyLen;
}





/*
*@funtion    :findTLV
*
*@description: 查找tlv
*
*@param      : TlvInfo*  tlv      --待查找的tlv
*              BYTE tag           --查找的tag，为0xFF表示查找下一个tlv
*              BYTE mask          --查找tag的掩码
*              BYTE occur         --需要查找的次数
*@return     : BYTE               --TRUE：查找到
*                                 --FALSE:未查找到
             
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 :modify by chentao@20210325 合并笑笑修改的代码，支持MNOSD自关联，支持文件系统多层30tag, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE findTLV(TlvInfo* tlv, BYTE tag, BYTE mask, BYTE occur)
{	
	BYTE idata findTag;
	BYTE idata lenByte0;
	SWORD idata nextLen;
	
	while (tlv->findNextLen >= 2)
	{
		tlv->findTagOff = tlv->findNextOff;
	
		findTag = VMReadByte(tlv->bufferAddr+tlv->findTagOff);
		
		if(findTag == 0xFF) //无效的tag,WQ_Offline_Profile_Test_TC_2_1_3_ProfileHeader_UnknownTag_Error
		{
			tlv->tagBuffer[tlv->levelIndex] = findTag;
			return TRUE;  
		}
	
		//根据ASN.1 BER tag 编码规则：
		//Identifier octets，有两种形式：较小的标签值（标签值在0和30之间）和较大的标签值（标签值大于等于31）
        //Low-tag-number form：一个字节。Bit8和bit7表示类（如表2），bit6值为0，表示编码方法为简单化的。Bit5－1给出了标签值
		//High-tag-number form：两个或多个octet。第一个octet形式如low-tag-number form，但是bit5－1均为1。第二个和以后的字节给出标签值，基于128，最高位在先，以便使用尽可能少的数字，除了最后一个字节以外，每个字节的bit 8都置为1。最后一个字节的为0。
		//目前EUICC 的PE 中不存在两个或多个 tag 类型
		if((findTag & 0x1F) == 0x1F)   
		{
			tlv->findTagLen = 2;	//tag占用2字节
			
			if(tlv->findNextLen == 2)//只收到2个字节的tag，直接退出去
			{
				return FALSE;
				//return 2;//只收到2个字节的tag，直接退出去
			}
			
			findTag = VMReadByte(tlv->bufferAddr+tlv->findTagOff+1);
			
			findTag &= 0x7F;
		}
		else
		{
			tlv->findTagLen = 1;	//tag占用1字节
		}
		
		lenByte0 = VMReadByte(tlv->bufferAddr+tlv->findTagOff+tlv->findTagLen);  //读取tagLen的第一字节
		
		
		// Length octets：有两种格式：短型（长度在0至127之间）和长型（长度在0至21008?1之间）
        // Short form: 一个字节，bit8为0，bit 7－1表示长度。
		// Long form: 2－127个字节。第一个字节的Bit 8为1，bit 7－1表示后面有多少个用于表示实际长度的octet。第二个和随后的octet给出实际长度，基于256，高位数字在先。		
		tlv->findLenLen = ((lenByte0 & 0x80) == 0x80) ? (lenByte0 & 0x7F) + 1 : 1; 
		
		switch(tlv->findLenLen)
		{
			case 1:
			{
				if((tlv->findNextLen == 2) && (lenByte0 > 0))//只收到一个字节的长度
				{
					return FALSE;
				}
				tlv->findValueLen = lenByte0;
				
			    break;
			}
			case 2:
			{
				if(tlv->findNextLen == 2)//只收到1个字节的tag和一个字节的长度tag81，直接退出去
				{
					return FALSE;
				}
				lenByte0 = VMReadByte(tlv->bufferAddr+tlv->findTagOff+tlv->findTagLen+1);
			
			    tlv->findValueLen = lenByte0;
				
				break;
			}
			case 3:
			{
				if((tlv->findNextLen == 2) || (tlv->findNextLen == 3)|| (tlv->findNextLen == 4))//1、只收到1个字节的tag和一个字节的长度tag82，直接退出去   2、只收到1个字节的tag和一个字节的长度tag82，一个字节的长度值 3、长度刚好收齐
				{
					return FALSE;
				}
				
			    tlv->findValueLen = VMReadShort(tlv->bufferAddr+tlv->findTagOff+tlv->findTagLen+1);
				
				break;
			}
			default:  //白盒测试添加
				break;
		}
	
		tlv->findValueOff = tlv->findTagOff + tlv->findTagLen + tlv->findLenLen;  //point to the Tag Value Offset
		
		tlv->findTLVLen = tlv->findValueLen+tlv->findTagLen+tlv->findLenLen;   //当前TLV所占的总的字节数
		
		tlv->findNextOff =  tlv->findValueOff + tlv->findValueLen;  //point to the Next Tag Offset		
		
		
		if(tlv->findNextLen < tlv->findTLVLen) 
		{
			//return FALSE; //找到的Len已经超出buffer的边界
			tlv->isCompleteBuffer[tlv->levelIndex] = FALSE;
			nextLen = 0;
		}
		else
		{
			tlv->isCompleteBuffer[tlv->levelIndex] = TRUE;
			nextLen = tlv->findNextLen - tlv->findTLVLen;      //获取剩余TLV的总长度
		}
		
				
		tlv->findNextLen = nextLen;
				
		tlv->tagBuffer[tlv->levelIndex] = findTag;  //登记当前查找到的tag到 tlv->levelIndex中
		
		tlv->offBuffer[tlv->levelIndex] = tlv->findNextOff;  //登记Next tag的偏移
		
		tlv->lenBuffer[tlv->levelIndex] = tlv->findNextLen;    //登记剩余TLV 的总长度
		
		tlv->valueLenBuffer[tlv->levelIndex] = tlv->findValueLen;
		
		if ((((tag & mask) == (findTag & mask))||(tag ==0xFF)))
		{
			occur--;
			
			if(occur == 0)
			{
				return TRUE;
			}		
		}

	}
	
	return FALSE; //只收到1个字节的tag或者剩余长度为0，直接退出去
}



/*
*@funtion    :replaceFindTlvInRam
*
*@description: 替换查找到的tlv
*
*@param      : BYTE tag           --查找的tag
*              TlvInfo*  dstTlv      --待查找的dstTlv, dstTlv  -> bufferAddr 必须在RAM 中        
*              BYTE * buf         --替换的数据
*              WORD len           --替换数据的长度
*@return     : void              
*
*@author     :xwh
*
*@history    :V1.0 :create by xwh 20170713
*/
void replaceFindTlvInRam(BYTE tag, BYTE * srcBuf, WORD srcLen)
{
	BYTE idata result,result1;
    BYTE idata tempLen,findtlvlen;
    TlvInfo xdata srcTlv;
    TlvInfo xdata templateA5Tlv;
    BYTE idata bBoolSrcHasC1OrC2;
    BYTE idata bBooltemplateA5HasC1OrC2;
    
	result = FindTag(tempBuf,tag,G_TempLen);
	
	if(result == FALSE)//若未找到，则追加到dstTlv后面
	{
		VMReadBuf(tempBuf + G_TempLen, (UWORD)srcBuf, srcLen);
		
		G_TempLen += srcLen;		
	}
	else
	{
		findtlvlen = tempBuf[result]+2;
        if (tag==Tag_fPInfo)  // 0xA5下面的C1 C2要判断 zhhj 20180607
        {
            //如果srcBuf的C1和C2tag都没有，那么需要到dstTlv->bufferAddr里面去找
            
            //如果srcBuf有C1 tag
            //如果srcBuf有C2 tag
            bBoolSrcHasC1OrC2=FALSE;
            initTlvInfo(&srcTlv, (UWORD)srcBuf+2, srcLen-2);
            initFindTlv(&srcTlv);
            result1 = findTLV(&srcTlv, 0xC1, 0xFF, 1);
            
            if(result1 == FALSE) //没找到
            {
            	initFindTlv(&srcTlv);
                result1 = findTLV(&srcTlv, 0xC2, 0xFF, 1);
                if(result1==FALSE)
                {
                    bBoolSrcHasC1OrC2=FALSE;
                }
                else
                {
                    bBoolSrcHasC1OrC2=TRUE;
                }
            }
            else
            {
                bBoolSrcHasC1OrC2=TRUE;
            }        
             
            if (bBoolSrcHasC1OrC2==FALSE)
            {
                //拼出一个
                //bBooltemplateA5HasC1OrC2=FALSE;
                initTlvInfo(&templateA5Tlv, (UWORD)(tempBuf + result+1), tempBuf[result]);
               
                 initFindTlv(&templateA5Tlv);
                 result1 = findTLV(&templateA5Tlv, 0xC1, 0xFF, 1);
            
                if(result1 == FALSE) //没找到
                {
                	initFindTlv(&templateA5Tlv);
                    result1 = findTLV(&templateA5Tlv, 0xC2, 0xFF, 1);
                    if(result1==FALSE)
                    {
                        bBooltemplateA5HasC1OrC2=FALSE;
                    }
                    else
                    {
                        bBooltemplateA5HasC1OrC2=TRUE;
                    }
                }
                else
                {
                    bBooltemplateA5HasC1OrC2=TRUE;
                }             
             
                if(bBooltemplateA5HasC1OrC2==FALSE) //源和模板都没有C1，C2
                {
                    ecpMem_Move((BYTE *)(tempBuf + result-1+ srcLen),(BYTE *)(tempBuf + result-1+findtlvlen),G_TempLen-(result-1+findtlvlen));
		
                    VMReadBuf((BYTE *)(tempBuf + result-1), (UWORD)srcBuf, srcLen);
			
					G_TempLen = (G_TempLen + srcLen-findtlvlen);	
                }
                else   //源没有C1C2,模板有C1C2
                {
                    tempLen=srcLen+templateA5Tlv.findTLVLen;
                    //移动也是有讲究的，不然数据会被冲掉哦
                    ecpMem_Move((BYTE *)(tempBuf + result-1+ tempLen),(BYTE *)(tempBuf + result-1 +  findtlvlen),G_TempLen - (result-1 + findtlvlen));
                    //这个一定要在下个的上面，不然数据会被冲掉，不能用memcpy,这样也会冲掉，前面的先改变了
                    ecpMem_Move((BYTE *)(tempBuf+ result-1+srcLen),(BYTE *)(templateA5Tlv.bufferAddr + templateA5Tlv.findTagOff),templateA5Tlv.findTLVLen);
                    //最后才是这个
                    VMReadBuf((BYTE *)(tempBuf + result-1), (UWORD)srcBuf, srcLen);
                    //注意，还要修改A5后面的长度值
                     ecpMem_Set((BYTE *)(tempBuf + result),tempLen-2,1);
                    
                    G_TempLen = ( G_TempLen + tempLen - findtlvlen);	
                }
                   
            }
            else //源有C1C2
            {
                ecpMem_Move((BYTE *)(tempBuf+ result-1+ srcLen),(BYTE *)(tempBuf + result-1 + findtlvlen),G_TempLen -(findtlvlen+result-1));
		
                VMReadBuf((BYTE *)(tempBuf + result-1), (UWORD)srcBuf, srcLen);
			
                G_TempLen = (G_TempLen + srcLen - findtlvlen);	
            }
            
        }
        else
        {
            ecpMem_Move((BYTE *)(tempBuf + result-1+ srcLen),(BYTE *)(tempBuf+ result-1 + findtlvlen),G_TempLen -(result-1+findtlvlen));
		
            VMReadBuf((BYTE *)(tempBuf +result-1), (UWORD)srcBuf, srcLen);
			
            G_TempLen = (G_TempLen + srcLen - findtlvlen);	
        }
	}
	 
}


/*
*@funtion    :checkProprietaryEFInfo
*
*@description: 检查0xa5 tag的信息
*
*@param      : void
*@return     : TRUE/FALSE              
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx 20190624
*/
BYTE checkProprietaryEFInfo()
{
	BYTE  idata result,fillTag,repeatTag;
	TlvInfo xdata a5TlvInfo;
		
	fillTag = FALSE;
	repeatTag = FALSE;
	
	initTlvInfo(&a5TlvInfo,G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff,G_PETlvInfo.findValueLen);
	
	initFindTlv(&a5TlvInfo);
	
	while (1)
	{	
		result = findTLV(&a5TlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
			break;
		}
		switch(a5TlvInfo.tagBuffer[0])
		{
			case 0xc0://specialFileInformation [PRIVATE 0] OCTET STRING (SIZE (1)) DEFAULT '00'H,如果存在specialFileInformation，但是长度不为1，抛错
			{
				if(a5TlvInfo.findValueLen != 1)
				{
					return FALSE;
				}									
				
				break;
			}
			case 0xc1://fillPattern [PRIVATE 1] OCTET STRING (SIZE(1..200)) OPTIONAL,
				if((a5TlvInfo.findValueLen < 1) || (a5TlvInfo.findValueLen > 200))//fillPattern [PRIVATE 1] OCTET STRING (SIZE(1..200)) OPTIONAL,
				{
					return FALSE;
				}
				fillTag = TRUE;
				break;
			case 0xc2://repeatPattern [PRIVATE 1] OCTET STRING (SIZE(1..200)) OPTIONAL,
				if((a5TlvInfo.findValueLen < 1) || (a5TlvInfo.findValueLen > 200))//fillPattern [PRIVATE 1] OCTET STRING (SIZE(1..200)) OPTIONAL,
				{
					return FALSE;
				}
				repeatTag = TRUE;
				break;

		}
	}
   
	if(fillTag && repeatTag)//这两个tag不能同时存在
	{
		return FALSE;
	}
	
	return TRUE;
}


/*
*@funtion    :replaceGivenTLV
*
*@description: 替换查找到的tlv
*
*@param      : BYTE tag           --查找的tag
*              TlvInfo*  tlv      --待查找的tlv          
*              BYTE * buf         --替换的数据
*              WORD len           --替换数据的长度
*@return     : void              
*
*@author     :xwh
*
*@history    :V1.0 :create by xwh 20170712
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE replaceGivenTLV(void)
{
	BYTE  idata result;
	BYTE  idata replaceTag;
	
	while(1)
	{	
		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
			break;
		}
		
		replaceTag = G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex];
		
		switch(replaceTag)
		{
			case Tag_GFMfShortID:
				/*if(G_PETlvInfo.findValueLen > 1)//shortEFID [8] OCTET STRING (SIZE (0..1)) OPTIONAL,
				{
					return FALSE;
				}*/
				break;
			case Tag_GFMfAttr:	//securityAttributesReferenced [11] OCTET STRING (SIZE (1..3)) OPTIONAL
				/*if((G_PETlvInfo.findValueLen != 1) && (G_PETlvInfo.findValueLen != 3))
				{
					return FALSE;
				}*/
               // SetBit(G_IsExistFlag1,Attr8BTag); //PE有tag ATTR。//如果PE里面有Tag_fAttr,这时候要先标记一下全局变量G_PEHasTagAttr=TRUE，如果创建文件时，这是个mapfile.根据这个G_PEHasTagAttr==false判断使用实体文件的attr
				
				makeFileAttrInfo(replaceTag);

				break;
					
			case Tag_GFMfPstd:			
				if(makePInfoData(replaceTag) == FALSE)
				{
					return FALSE;
				}
				break;

			case Tag_GFMfLinkPath:
				if(G_PETlvInfo.findValueLen >  (2*DF_Depth))
				{
					return FALSE;
				}
				makeFileLinkPath(replaceTag);
				break;
			case Tag_GFMfPInfo://0xa5
				
			   if(checkProprietaryEFInfo() == FALSE)
			   {
				   return FALSE;
			   }
				
				break;
			case Tag_GFMfDept:
			case Tag_GFMfId:
			case Tag_GFMfdfName:
			case Tag_GFMflcsi:
			case Tag_GFMfSize:

				break;
			
			default: //profile2.3--- TC_8_2_11_4_Check_unknown_tag_in_Profile_Package,未知的tag需要抛错
				return false;
		}
	}
	
	return TRUE;
}

/*
*@funtion    :replaceTLVFromTLVList
*
*@description: 替换查找到的tlv
*
*@param      : BYTE tag           --查找的tag
*              TlvInfo*  tlv      --待查找的tlv          
*              BYTE * buf         --替换的数据
*              WORD len           --替换数据的长度
*@return     : void              
*
*@author     :xwh
*
*@history    :V1.0 :create by xwh 20170712
*     		  add comment by zhhj 20180705
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP										
*/
BYTE replaceTLVFromTLVList()
{
	BYTE  idata result;
	BYTE  idata replaceTag,tag_fDeptResult;

	while(1)
	{	
		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
			break;
		}
		
		replaceTag = G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex];
		
		if(replaceTag == Tag_fDept)
		{
			/*if((G_PETlvInfo.findValueLen != 2) && (G_PETlvInfo.findValueLen != 4))//fileDescriptor [2] OCTET STRING (SIZE(2..4)) OPTIONAL,
			{
				return FALSE;
			}*/
			
			tag_fDeptResult = FindTag(tempBuf,Tag_fDept,G_TempLen);
			
			if((tag_fDeptResult != FALSE) && (tempBuf[tag_fDeptResult] != 0))
			{//判断文件模板类型和profile下行文件类型时需要与上FILETYPE：芯安微给的profile，创建2fe2文件时，下行文件类型是02，模板中是42，导致抛错
				if((tempBuf[tag_fDeptResult+1]&FILETYPE) != (VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff) & FILETYPE))//Changing the file type (byte 1 of fileDescriptor) as defined in the template is not allowed in the profile package.
				{
					return FALSE; //eUICC_Profile_PE_boundary/Template_Change_TR_to_LF_test_01  测试MF模板修改了2F08文件属性，希望下载失败
				}
			}
		}
		
		if(replaceTag == Tag_fPInfo)
		{
			if(checkProprietaryEFInfo() == FALSE)
			{
				return FALSE;
			}
		}
		
		switch(replaceTag)
		{			
			case Tag_fDept:	
			case Tag_fId:
			case Tag_fdfName:
			case Tag_flcsi:
			case Tag_fSize:
			case Tag_fShortID:				
			case Tag_fPInfo:
				 replaceFindTlvInRam(replaceTag,(BYTE*)(G_PETlvInfo.bufferAddr+G_PETlvInfo.findTagOff),G_PETlvInfo.findTLVLen);
				
				 makeDescript();
			
				 break;

			case Tag_fAttr:				

				//SetBit(G_IsExistFlag1,Attr8BTag); //PE有tag ATTR。//如果PE里面有Tag_fAttr,这时候要先标记一下全局变量G_PEHasTagAttr=TRUE，如果创建文件时，这是个mapfile.根据这个G_PEHasTagAttr==false判断使用实体文件的attr
				makeFileAttrInfo(replaceTag);
			
				break;
					
			case Tag_fPstd:			
			
				if(makePInfoData(replaceTag) == FALSE)
				{
					return FALSE;
				}
				break;

			case Tag_fLinkPath://映射路径信息
				
				if(G_PETlvInfo.findValueLen >  (2*DF_Depth))
				{
					return FALSE;
				}
				
				makeFileLinkPath(replaceTag);
				break;
			
			default: //白盒测试添加
				break;						
		}
	}
	return TRUE;
}

#endif
