/************************************************************************************************************
* Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   GPAnnexAdminSessionPara.c
*
* Description: 该文件主要用于GP附录B相关的函数，主要用于实现GP附录B 3.7  Administration Session Triggering Parameters中涉及的85 A5tag的处理
               涉及 installforinstall storeData GetData的处理
*
* auther：     xxxx    xxxx.xx.xx
*
* history：    V1.0
*
************************************************************************************************************/
#include "Interpreter.h"
#include "ASN1Operation.h"
#include "Exception.h"
//#include "Heap.h"
//#include "main.h"
//#include "GPPublic.h"
#include "gsmpublic.h"
#include "GPAnnexbAdminSessionPara.h"
//#include "transaction.h"
#include "opFunction.h"
#include "SecurityDomainData.h"
#include "ob_manager.h"
#include "cardManager.h"
#include "nativemethod_impl.h"
#include "T0ApduImpl.h"
#if GP__AMENDMENT__B__FOR_MASK
#include  "uicc_interface.h"
#include "dataManager.h"
/*
*@function   :GPStoreSDAdminSessionPara
*
*@description:处理GP附录B中通过store data指令存储Security Domain Administration Session Parameters
*
*@See        :       
*
*@param      :sdDataObj         --- 需要被存储AnnexB参数的SDID
*@param      :BYTE * dataBuffer --- 85LV/A5LV， 假设这里的L是单字节      
*
*@return     :void
*
*@history    :V1.0----created by xxj @20180320            
*/
void GPStoreSDAdminSessionPara(PN8 sdDataObj, BYTE * dataBuffer)
{
	GPStoreSDAdminSessionPara85(sdDataObj, dataBuffer);
}

/*
*@function   :GPGetSDAdminSessionPara
*
*@description:处理GP附录B中通过get data指令获取Security Domain Administration Session Parameters
*
*@see        :        
*
*@param      :sdDataObj         --- 输入，对应的SDID
*@param      :BYTE tag          --- 输入，0x85或0xA5
*@param      :BYTE * dataBuffer --- 输出，获取到的响应数据
*
*@return     :WORD --- dataBuffer中的有效数据长度
*
*@history    :V1.0----created by xxj @20180320            
*/
WORD GPGetSDAdminSessionPara(PN8 sdDataObj, WORD tag, BYTE * dataBuffer)
{
	WORD idata len;
	
	if(tag == 0x85)
	{
		len = GPGetSDAdminSessionPara85(sdDataObj, dataBuffer);
	}
	else // tag == 0xA5
	{
		len = GPGetSDAdminSessionParaA5(sdDataObj, dataBuffer);
	}
	
	return len;
}

/*@function   :GetSDAdminSessionParaTagOffet                                            
 * 
 *@description:获取SD的附录B参数子TAG 84, 85, 86, 89相对于SDletEntry_AdminSessionParaRef_offset的偏移 
 *
 *@see        :	       
 *                                                                                                                                     
 *@param      :BYTE tag --- 取值为84, 85, 86, 89中的一个     
 *              						                                                                                                  
 *@return     :BYTE     --- 对应着84,85,86,89的返回值分别为0, 2, 4, 6                                                                                                                                                                                                                                                           
 *  
 *@history    :V0.0---created by xwh 20180803
 *                        
 */
BYTE GetSDAdminSessionParaTagOffet(BYTE tag)
{
	return 2*((tag==0x89)?3:(tag-0x84));
}
/*
*@function   :GPStoreSDAdminSessionParaSub
*
*@description:处理GP附录B中通过store data指令存储Security Domain Administration Session Parameters
*
*@See        :《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf》 The STORE DATA command with tag '85' (in TLV mode)  shall  be used to create or update the complete  set 
*              of SD Administration Session Parameters . Parameters may be removed all at once using tag '85' with a null length.        
*
*@param      :sdDataObj
*@param      :tag          0085LV 中的子tag: 84 LV,85 LV,86 LV,89 LV
*@param      :dataBuffer   子tag的L起始下标
*
*@return     :BYTE  返回子tag的长度，LV的总长度
*
*@history    :V1.0----created by xxj @20180322
*@history    :V1.1----modified by xuxiaojun@20210414 对于类型确定的数组，_newarrayObject紧接着给数组内容写值时，避免调用get_array_data，可以减少上面一次创建数组时cache的提交
*@history    :V1.2----modify by xuxiaojun@20220513 remove warning,by xuxiaojun 20220517
*/
BYTE GPStoreSDAdminSessionParaSub(PN8 sdDataObj, BYTE tag, BYTE * dataBuffer)
{
	WORD idata subTagLen, heapLen, tagRef, tmpRef, offset;
	PX8	 X_A   tempAmendBPtr;
	u16  I_A   index;
	
	offset = 0;
	
	subTagLen = dataBuffer[offset++];
	
	if(subTagLen == 0x81)
	{				
		subTagLen = dataBuffer[offset++];
		
		heapLen = subTagLen + 2;			
	}
	else
	{	
		heapLen = subTagLen + 1;
	}
	
	switch(tag)
	{
		case 0x84:
		{
			index = Amend_B_84_buffer_INDEX;

			break;
		}
		case 0x85:
		{
			index = Amend_B_85_buffer_INDEX;

			break;
		}
		case 0x86:
		{
			index = Amend_B_86_buffer_INDEX;

			break;
		}
		case 0x89:
		{
			index = Amend_B_89_buffer_INDEX;

			break;
		}
		default: //todo: 不识别TAG，是忽略还是抛异常？
			return heapLen; // warning: C3017W: index may be used before being set
	}
	
	tagRef =  nvmReadU16(get_object_field_ref(sdDataObj, index));

#if M2M_EUICC
	if((sdDataObj == MEMtPN8(sdData_getsdDataObjByAppId(ISDR_APPID))) && (index == Amend_B_85_buffer_INDEX)) //SGP02.pdf  Table 4: PSK-ID Format
	{
		ecpMem_Copy(tempBuf, dataBuffer+offset, dataBuffer[offset]);

		StringToInt(tempBuf, tempBuf);

		if ((tempBuf[1] != 0x80) || (tempBuf[2] != 0x01) || (tempBuf[3] != 0x02) || (tempBuf[4] != 0x81) || (tempBuf[5] != 0x10))
		{
			ThrowError(SW_WRONG_DATA);
		}

		if (VMReadByte(EUICCEIDInfo_Addr) == 0x10)				//卡片已更新EID
		{
			if(NvmCmp((BYTE *)(EUICCEIDInfo_Addr), tempBuf+5, 0x11) != EQUAL) //EID不匹配，抛异常
			{
				ThrowError(SW_WRONG_DATA);
			}
		}
		else
		{
			VMWriteBuf(EUICCEIDInfo_Addr, tempBuf+5, 0x11);
		}
	}
#endif

	if(tagRef != REF_NULL) //前面已经分配过，则需要把上一次分配的释放掉
	{
#if (EUICC_ONLY||SE_AND_EUICC)
		if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
		{
			bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
		}

	}
	
	if(heapLen == 1) //子tag的长度为0,eg: 8400
	{
		tmpRef = REF_NULL;
	}
	else
	{
		tmpRef	= _newarrayObject(AType_Byte, subTagLen, BIT_NEED_REFRESH);
		
		tempAmendBPtr = MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(tmpRef))));

		nvmCopyAtomic(MEMtPM8(tempAmendBPtr),&dataBuffer[offset],  U8t16(subTagLen));
	}

	nvmWriteU16Atomic(get_object_field_ref(sdDataObj, index), tmpRef); //将新的引用写入

	return heapLen;

}
/*
*@function   :GPStoreSDAdminSessionPara85
*
*@description:处理GP 附录B中通过store data指令存储Security Domain Administration Session Parameters, tag为85
*
*@See        :《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf》 The STORE DATA command with tag '85' (in TLV mode)  shall  be used to create or update the complete  set 
*              of SD Administration Session Parameters . Parameters may be removed all at once using tag '85' with a null length.        
*
*@param      :sdDataObj  ---
*@param      :BYTE *dataBuffer  --- 85LV/A5LV
*
*@return     :void
*
*@history    :V1.0----created by xxj @20180320        
             :V1.1----modify by xuxiaojun@20190314 通过85TAG来更新参数时，如果子TAG中的某一个缺失，则将该子TAG移除。<TC_8_2_6_5_Check_OTA_HTTPs_Personalisation>
*/
void GPStoreSDAdminSessionPara85(PN8 sdDataObj, BYTE * dataBuffer)		//GPStoreSDAdminSessionPara85(entryAddr + SDletEntry_AdminSessionParaRef_offset, dataBuffer,sdID);
{
	BYTE idata tagExist, transActioinFlag, tag;
	WORD idata offset, heapLen;
	UWORD xdata length;
	
	tag = dataBuffer[0];
	
	offset = 1+ GetBERTlvLen((UWORD)&dataBuffer[1], &length);//length = dataBuffer[1]; ，offset指向V

	if((offset + length) > 0xFF) //modify by xuxiaojun 整个85LV/A5LV的总长度，不支持超过一条指令的情况   RSP下载移动profile激活测试/通用版HTTPS测试/storeHttpsPara_Tag8A_Length_FC
	{
		dataManager_abortStoreDataThrowError(SW_FILE_FULL);
	}
	
	transActioinFlag = FALSE;
	
	tagExist = 0; //bit0为1表示84tag存在，bit1为1表示85tag存在，bit2为1表示86tag存在，bit3为1表示89tag存在
	
	if(length == 0) 
	{
		if(tag == 0xA5) // If a  sub - TLV is not present, no creation/ update shall be performed for the corresponding SD Administration Session Parameter
		{
			return;
		}
		
		transActioinFlag = IF_BeginTransaction();
		
		sdData_setAmend_B_84_bufferRef(sdDataObj, REF_NULL);      // Parameters may be removed all at once using tag '85' with a null length
		sdData_setAmend_B_85_bufferRef(sdDataObj, REF_NULL);
		sdData_setAmend_B_86_bufferRef(sdDataObj, REF_NULL);
		sdData_setAmend_B_89_bufferRef(sdDataObj, REF_NULL);
#if (EUICC_ONLY||SE_AND_EUICC)
		if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
		{
			bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
		}

		IF_CommitTransaction(transActioinFlag);
		
		return;		
	}	

	transActioinFlag = IF_BeginTransaction();
	
	while(offset <= length)
	{		
		tagExist |= ((dataBuffer[offset] == 0x89)?0x08:(1<<(dataBuffer[offset]-0x84)));
		
		heapLen = GPStoreSDAdminSessionParaSub(sdDataObj, dataBuffer[offset], (BYTE*)&dataBuffer[offset+1]);

		offset += (heapLen+1);
	}
		
	if(tag != 0xA5)
	{
		if((tagExist&0x01) == 0x00)  //84缺失
		{
			sdData_setAmend_B_84_bufferRef(sdDataObj, REF_NULL);
#if (EUICC_ONLY||SE_AND_EUICC)
			if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
			{
				bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
			}
		}
		
		if((tagExist&0x02) == 0x00)  //85缺失
		{
			sdData_setAmend_B_85_bufferRef(sdDataObj, REF_NULL);
#if (EUICC_ONLY||SE_AND_EUICC)
			if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
			{
				bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
			}
		}
		
		if((tagExist&0x04) == 0x00)  //86缺失
		{
			sdData_setAmend_B_86_bufferRef(sdDataObj, REF_NULL);
#if (EUICC_ONLY||SE_AND_EUICC)
			if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
			{
				bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
			}
		}
		
		if((tagExist&0x08) == 0x00)  //89缺失
		{
			sdData_setAmend_B_89_bufferRef(sdDataObj, REF_NULL);
#if (EUICC_ONLY||SE_AND_EUICC)
			if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
			{
				bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
			}
		}
	}
	IF_CommitTransaction(transActioinFlag);
}
/*
*@function   :GPGetSDAdminSessionPara85
*
*@description:处理GP 附录B中通过GET data指令存储Security Domain Administration Session Parameters, tag为85
*
*@See        :《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf》 The full set of  SD Administration Session Parameters defined above may be retrieved using the GET DATA 
*              command as define d in  [GPCS]   with P1- P2 set to '00  85'. The response to the GET DATA command shall contain  tag '85' encapsulating the  full set of  SD Administration 
*              Session Parameters  (i.e. parameters that are currently stored).         
*
*@param      :UWORD tagRefAddr ---  85tag的子tag引用在sdentry中的存储地址,即对应的SDEntry的SDletEntry_AdminSessionParaRef_offset的逻辑地址
*@param      :BYTE * dataBuffer---  获取到的响应数据，格式为85LV
*            
*@return     :WORD --- dataBuffer中有效数据长度
*
*@history    :V1.0----created by xxj @20180320    
*             V1.1----modify by xwh 20201123,只判断tag是否存在时dataBuffer传NULL进来，此时不用拷贝数据
*/
/*
WORD GPGetSDAdminSessionPara85(PN8 sdDataObj, BYTE * dataBuffer)
{
	u8 			arrayLen;
	jref 		bufferRef;
	memref		bufferFeild;
	memref 		bufferAddr;
	u8          tmp[5];
	u16         dataBufferOffset;
	u8          offset;
	u8          i;
	u8          tag;
	
	offset = 0;

	dataBufferOffset = 0;

	for(i=0;i<4;i++)
	{
		switch(i)
		{
			case 0:
			{
				bufferRef = sdData_getAmend_B_84_bufferRef(sdDataObj);
				tag = 0x84;
				break;
			}
			case 1:
			{
				bufferRef = sdData_getAmend_B_85_bufferRef(sdDataObj);
				tag = 0x85;
				break;
			}
			case 2:
			{
				bufferRef = sdData_getAmend_B_86_bufferRef(sdDataObj);
				tag = 0x86;
				break;
			}
			case 3:
			{
				bufferRef = sdData_getAmend_B_89_bufferRef(sdDataObj);
				tag = 0x89;
				break;
			}

		}

		if(bufferRef != REF_NULL)
		{
			bufferAddr = locateObjectInMemory(bufferRef);

			arrayLen   = U16t8(get_array_length(bufferAddr));

			//offset = ASN1_PaddingBERTlvTagAndLen(tmp, tag, arrayLen);

			tmp[0] = tag;
			if(arrayLen >= 0x80)
			{
				tmp[1] = 81;
				tmp[2] = arrayLen;
				offset = 3;
			}
			else
			{
				tmp[1] = arrayLen;
				offset = 2;
			}

			
			if(dataBuffer != NULL)
			{
				ecpMem_Copy(&dataBuffer[dataBufferOffset], tmp, offset);
			}

			dataBufferOffset += offset;

			bufferFeild = get_array_data(MEMtPF8(bufferAddr));

			if(dataBuffer != NULL)
			{
				ecpMem_Copy(&dataBuffer[dataBufferOffset], bufferFeild, arrayLen);
			}

			dataBufferOffset += arrayLen;
		}

	}
	
	return dataBufferOffset;
}
*/
/*
*@function   :GPGetSDAdminSessionParaA5
*
*@description:处理GP 附录B中通过GET data指令存储Security Domain Administration Session Parameters, tag为85
*
*@See        :《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf》 A single SD Administration Session Parameter may be retrieved using the GET DATA command as defined 
*              in [GPCS]   with P1- P2 set to '00 A5' and with a data field containing a Tag List indicating which parameter is requested.         
*
*@param      :UWORD tagRefAddr ---  85tag的子tag引用在sdentry中的存储地址,即对应的SDEntry的SDletEntry_AdminSessionParaRef_offset的逻辑地址
*@param      :BYTE * dataBuffer---  既是输入也是输出，获取A5时，输入数据应该为5c84/5c85/5c86/8c89，返回时数据格式为A5L 85LV
*
*@return     :WORD --- dataBuffer中有效数据长度
*
*@history    :V1.0----created by xxj @20180320            
*/
/*
WORD GPGetSDAdminSessionParaA5(PN8 sdDataObj, BYTE * dataBuffer)
{
	u8 			arrayLen;
	jref 		bufferRef;
	memref		bufferFeild;
	memref 		bufferAddr;
	u8          tmp[5];
	u16         dataBufferOffset;
	u8          offset;
	u8          tag;
	
	offset = 0;
	dataBufferOffset = 0;
		
	if((P3 != 0x03) || (RecData[0] !=  0x5C) || (RecData[1] !=  0x01) || ((RecData[2] != 0x84) && (RecData[2] != 0x85) && (RecData[2] != 0x86) && (RecData[2] != 0x89)))
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	switch(RecData[2])
	{
		case 0x84:
		{
			bufferRef = sdData_getAmend_B_84_bufferRef(sdDataObj);

			tag = 0x84;

			break;
		}
		case 0x85:
		{
			bufferRef = sdData_getAmend_B_85_bufferRef(sdDataObj);

			tag = 0x85;

			break;
		}
		case 0x86:
		{
			bufferRef = sdData_getAmend_B_86_bufferRef(sdDataObj);

			tag = 0x86;

			break;
		}
		case 0x89:
		{
			bufferRef = sdData_getAmend_B_89_bufferRef(sdDataObj);

			tag = 0x89;

			break;
		}
	}
	if(bufferRef != REF_NULL)
	{
		bufferAddr = locateObjectInMemory(bufferRef);

		arrayLen   = U16t8(get_array_length(bufferAddr));

		//offset = ASN1_PaddingBERTlvTagAndLen(tmp, tag, arrayLen);
		tmp[0] = tag;

		if(arrayLen >= 0x80)
		{
			tmp[1] = 0x81;
			tmp[2] = arrayLen;
			offset = 3;
		}
		else
		{
			tmp[1] = arrayLen;
			offset = 2;
		}

		ecpMem_Copy(&dataBuffer[dataBufferOffset], tmp, offset);

		dataBufferOffset += offset;

		bufferFeild = get_array_data(MEMtPF8(bufferAddr));

		ecpMem_Copy(&dataBuffer[dataBufferOffset], bufferFeild, arrayLen);

		dataBufferOffset += arrayLen;
	}
	else
	{
		dataBuffer[dataBufferOffset++] = RecData[2];
		dataBuffer[dataBufferOffset++] = 0x00;
	}
	
	return dataBufferOffset;
}
*/
/**
*modify by xuweihua@20210625: 优化空间， EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
*/
WORD GPGetSDAdminSessionPara85orA5(PN8 sdDataObj, BYTE * dataBuffer,BYTE mode)
{
	u8 			arrayLen;
	jref 		bufferRef;
	memref		bufferFeild;
	memref 		bufferAddr;
	u8          tmp[5];
	u16         dataBufferOffset;
	u8          offset;
	u8          i;
	u8          tag;
	
	offset = 0;
	tag    = 0;

	dataBufferOffset = 0;
	
	if(mode == 0xA5)
	{
			if((P3 != 0x03) || (RecData[0] !=  0x5C) || (RecData[1] !=  0x01) || ((RecData[2] != 0x84) && (RecData[2] != 0x85) && (RecData[2] != 0x86) && (RecData[2] != 0x89)))
			{
				ThrowError(SW_WRONG_DATA);
			}
	}
	
	for(i=0;i<4;i++)
	{
		switch(i)
		{
			case 0:
			{
				bufferRef = sdData_getAmend_B_84_bufferRef(sdDataObj);
				tag = 0x84;
				break;
			}
			case 1:
			{
				bufferRef = sdData_getAmend_B_85_bufferRef(sdDataObj);
				tag = 0x85;
				break;
			}
			case 2:
			{
				bufferRef = sdData_getAmend_B_86_bufferRef(sdDataObj);
				tag = 0x86;
				break;
			}
			case 3:
			{
				bufferRef = sdData_getAmend_B_89_bufferRef(sdDataObj);
				tag = 0x89;
				break;
			}
			default: // warning: C3017W: tag may be used before being set
			{
				bufferRef = REF_NULL;
				break;
			}

		}
		
		if((mode == 0xA5)&&(RecData[2] != tag))
		{
				continue;
		}

		if(bufferRef != REF_NULL)
		{
			bufferAddr = locateObjectInMemory(bufferRef);

			arrayLen   = U16t8(get_array_length(bufferAddr));

			//offset = ASN1_PaddingBERTlvTagAndLen(tmp, tag, arrayLen);

			tmp[0] = tag;
			if(arrayLen >= 0x80)
			{
				tmp[1] = 0x81;
				tmp[2] = arrayLen;
				offset = 3;
			}
			else
			{
				tmp[1] = arrayLen;
				offset = 2;
			}

			
			if(dataBuffer != NULL)
			{
				ecpMem_Copy(&dataBuffer[dataBufferOffset], tmp, offset);
			}

			dataBufferOffset += offset;

			bufferFeild = get_array_data(MEMtPF8(bufferAddr));

			if(dataBuffer != NULL)
			{
				ecpMem_Copy(&dataBuffer[dataBufferOffset], bufferFeild, arrayLen);
			}

			dataBufferOffset += arrayLen;
		}
		else
		{
			if(mode == 0xA5)
			{
				dataBuffer[dataBufferOffset++] = RecData[2];
				dataBuffer[dataBufferOffset++] = 0x00;
			}
		}

	}
	
	return dataBufferOffset;
}
/*
*@function   :handleSDAdmSessionParameter
*
*@description:处理GP 附录B中通过installforinstall指令存储Security Domain Administration Session Parameters, tag为85
*
*@See        :《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf》 The administration parameters may be set, using tag '85',   during Security Domain installation ,  using tag '85 '  inside the application spe cific install  parameters       
*               Note that it is not possible to provide such long value at the time of Security Domain installation as Java Card Specifications [JCS]   limits the length of Installation Parameters to 127 bytes.
*             
*@param      :AdmSessParaOffset   安装指令中85L V中L在RecData中的下标, LV, 在只有一条的安装指令中，85L 84LV
*            :tagRef              输出参数，对应的84 85 86 89的ref
*
*@return     :void
*
*@history    :V1.0----created by xxj @20180322          
*/
void StoreInstallforInstallSDAdmSessionParameter(BYTE AdmSessParaOffset, WORD* tagRef)
{
}
/*
*@function   :GPGetSDTriggerParaBySDID
*
*@description:通过sdID来获取该sd指定tag的值，tag的值为84,85,86,89
*
*@See        :   
*
*@param      :BYTE sdID            指定的sd
*            :BYTE tag             84,85,86,89中的一个
*            :BYTE *dataBuffer     获取到的值
*            ：WORD dataBufferLen   dataBuffer的大小，即允许的最大读取大小，否则数据会越界
*
*@return     :WORD                 dataBuffer中有效数据的长度
*
*@history    :V1.0----created by xxj @20180525
*@history    ：modify -jiangqb-20200720， 增加输入参数  dataBufferLen，防止读取数据时越界
*            :modify by xuxiaojun@20220513 去除警告
*/
WORD GPGetSDTriggerParaBySDID(BYTE sdID, BYTE tag, BYTE *dataBuffer,WORD dataBufferLen)
{
	PN8		    sdDataObj;
	u8 			arrayLen;
	jref 		bufferRef;
	memref		bufferFeild;
	memref 		bufferAddr;

	sdDataObj =	MEMtPN8(getSdDataObjBySdid(sdID));

	switch(tag)
	{
		case 0x84:
		{
			bufferRef = sdData_getAmend_B_84_bufferRef(sdDataObj);
			break;
		}
		case 0x85:
		{
			bufferRef = sdData_getAmend_B_85_bufferRef(sdDataObj);
			break;
		}
		case 0x86:
		{
			bufferRef = sdData_getAmend_B_86_bufferRef(sdDataObj);
			break;
		}
		case 0x89:
		{
			bufferRef = sdData_getAmend_B_89_bufferRef(sdDataObj);
			break;
		}
		default:
		{
			bufferRef = REF_NULL;
			break;
		}
	}

	if(bufferRef == REF_NULL)
	{
		return 0;
	}
	else
	{
		bufferAddr = locateObjectInMemory(bufferRef);

		arrayLen   = U16t8(get_array_length(bufferAddr));

		if (dataBufferLen >= arrayLen)	//判断数据不会越界才会读取数据到目标buf中
		{
			bufferFeild = get_array_data(MEMtPF8(bufferAddr));

			ecpMem_Copy(dataBuffer, bufferFeild, arrayLen);

			return arrayLen;
		}
		else
		{
			return 0;
		}
	}
}
#endif


