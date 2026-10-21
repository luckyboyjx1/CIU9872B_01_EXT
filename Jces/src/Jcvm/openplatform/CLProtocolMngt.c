#include "profile.h"
#include "typejava.h"
#include "Jcvm_Nvm.h"
#include "stack.h"
#include "object.h"
#include "packageloc.h"
#include "cardManager.h"
#include "CLProtocolMngt.h"
#include "clprotocol_global.h"
#include "error.h"
#include "ob_manager.h"
#include "nativemethod_impl.h"
#include "utilMethod.h"
#include "swpDataConfig.h"
//#include "Swp_Private.h"
#include "JcvmRam.h"
#include "JcvmConfig.h"

#if HAL__IO__SWP
#include "Io_Swp.h"
#include "SwpHal.h"
#endif

#if HAL__IO__MIFARE
#include "MifareHal.h"
#endif

#if GP__AMENDMENT__C__FOR_MASK
PN8 bEECFG;

/*增加私有指令E081000000，用于在镜像做卡后将UID更新成芯片实际的UID,by wuronghua 20211230*/
void UpdateCurrentCLParaUid(u8* uidData, u8 uidLen)
{
	if (bEECFG1[U8_A_U8(UID_TLV, 2)] == uidLen)
	{
		memCopyNoAtomic((u8*)&bEECFG1[U8_A_U8(UID_TLV, 3)], uidData, uidLen);
	}
	
	if (bEECFG2[U8_A_U8(UID_TLV, 2)] == uidLen)
	{
		memCopyNoAtomic((u8*)&bEECFG2[U8_A_U8(UID_TLV, 3)], uidData, uidLen);
	}
}

memref GetCurrentCLParaBuf()
{
	if(PN_R_U8(JcvmConfig_GetVmConfigAddr(AMDC_EFCFG_FLAG)) == 1) 
		return (memref)&bEECFG1[0];
	else
		return (memref)&bEECFG2[0];
}

memref GetPrepareCLParaBuf()
{
	return 	(PN_R_U8(JcvmConfig_GetVmConfigAddr(AMDC_EFCFG_FLAG)) == 1) ? ((memref)&bEECFG2[0]) : ((memref)&bEECFG1[0]);
}

void SwitchCLParaBuf()
{
	u32 flag = JcvmConfig_GetVmConfigAddr(AMDC_EFCFG_FLAG);
	

	if(bEECFG == MEMtPX8(&bEECFG1[0])) 
	{
		if(PN_R_U8(flag) == 1)	

			nvmWriteU8NoAtomic(MEMtPM8(flag), 2);
			//EFCFG_FLAG = 2;
	} 
	else 
	{
		if(PN_R_U8(flag) == 2)	
			nvmWriteU8NoAtomic(MEMtPM8(flag), 1);
			//EFCFG_FLAG = 1;

	}
	memRefresh();
	bEECFG = (PN_R_U8(flag) == 1) ? MEMtPX8(&bEECFG1[0]) : MEMtPX8(&bEECFG2[0]);
}

void InitializeProtocolTypeAPara()
{
	TypeAProtoDataCompactToTLV(MEMtPM8(JcvmConfig_GetVmConfigAddr(SYSCONFIG_OFFSET_START)), ComputContext->CurrentProtocolData);

	memCopyAtomic(ComputContext->ProtocolData, ComputContext->CurrentProtocolData, EECFG_TPA_TLVDATA_SIZE);
	memCopyAtomic(ComputContext->MandatoryMask, ComputContext->CurrentMandatoryMask, EECFG_TPA_TLVMASK_SIZE);
}
void InitializeProtocolTypeBPara()
{
	TypeBProtoDataCompactToTLV(MEMtPM8(JcvmConfig_GetVmConfigAddr(SYSCONFIG_OFFSET_START)), ComputContext->CurrentProtocolData);

	memCopyAtomic(ComputContext->ProtocolData, ComputContext->CurrentProtocolData, EECFG_TPB_TLVDATA_SIZE);
	memCopyAtomic(ComputContext->MandatoryMask, ComputContext->CurrentMandatoryMask, EECFG_TPB_TLVDATA_SIZE);
}


#define TypeA0  0
#define TypeA1 	1
static void CheckLV(u8* Scr, u16 checkLen, u8 Type)
{
	u8 			Length;
	
	Length 		= Scr[1];

	if(U8t16(Length) > U16_A_U8(checkLen , 1))
	{
		throw_error(SW_WRONG_DATA);
	}
	
	if(Type == TypeA0)
	{
	    if((Scr[0] == 0x80) && (U8t16(Length) < U16_A_U8(checkLen , 1)) && (Scr[2] != 0) && (Scr[2] != 0x04) && (Scr[2] != 0x07))
		{
			throw_error(SW_WRONG_DATA);
		}		
	if(U8_A_U8(Scr[2] , 1) != Length)
	{
		throw_error(SW_WRONG_DATA);
		}
	}
}

static void CheckBasic(u8* Scr, u16 checkLen)
{
	u8 			Length;
	
	Length 		= Scr[1];
	if(U8t16(Length) != checkLen)
	{
		throw_error(SW_WRONG_DATA);
	}
}

// modify by wuronghua@20211011  1. 增加了对 A1 tag 参数合法性的检查   2.更新时，需要所有参数都出现，长度可以为00

//A0 | L | V | A1 | L | V

// modify by wuronghua@20220811 直接使用ParaBuf（NVM）中的数据时，因为数据可能在cache中，造成数据不一致，因此读到tmpBuf（RAM）再使用
void StoreProtocolTypeAPara(PM8 ParaBuf, u16 ParaLen)
{
	PM8 				ParaLimit;
	PM8 				ParaPtr;
	PM8 				dest;
	PM8 				TagAXLimit;
    u8 					Type;
	u8	 				tmpBuf[PARA_TYPEA_LENGTH + PARAMASK_TYPEA_LENGTH];
	u8					checkTag;
    

    memCopyAtomic(MEMtPM8(ComputContext->ResultData), MEMtPM8(ComputContext->ProtocolData), PARA_TYPEA_LENGTH);
	memCopyAtomic(MEMtPM8(ComputContext->ResultMask), MEMtPM8(ComputContext->MandatoryMask), PARAMASK_TYPEA_LENGTH);

//	 if (sysConfig_GetContactlessParaSwitch())
//	 {
//		 TypeAProtoDataCompactToTLV(MEMtPM8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_START)), ComputContext->ResultData);
//     }

	//flush_flag maybe is 1, need get data from stream_buffer
	memCopyNoAtomic(tmpBuf, ParaBuf, ParaLen);
    ParaPtr			 	= tmpBuf;
	ParaLimit 			= PM8_A_U16(ParaPtr, ParaLen);
	
	while (ParaPtr < ParaLimit) 
	{
		

        if (ParaPtr[0] == 0xA0)
        {
			Type = TypeA0;
        	dest = MEMtPX8(ComputContext->ResultData);
        }
        else //0xA1
        {
		    Type = TypeA1;
        	dest = MEMtPX8(ComputContext->ResultMask);
        }
        
    	TagAXLimit = PM8_A_U8(ParaPtr, (U8_A_U8(ParaPtr[1], 2)));
            
    	ParaPtr    = PM8_A_U8(ParaPtr, 2);
		
		//checkTag = 0x00;
    	
    	while(ParaPtr < TagAXLimit)
   		{
    		u16 destOff = 0;
			u16 length;
    		
   			switch (ParaPtr[0]) {
			case 0x80:
			    CheckLV(ParaPtr, 4, Type);
				if (Type == TypeA1)
				{
					//Length Operation Indicator
					//'00': Application doesn‘t care about UID length.
					//'0F': Application does care about UID length and the XOR operator (strict equality) shall be used.
					//'FF': Application does care about UID length and the MAX operator shall be used
					if ( (ParaPtr[2] != 0x00) && (ParaPtr[2] != 0x0F) && (ParaPtr[2] != 0xFF) )
					{
						throw_error(SW_WRONG_DATA);
					}
				}
				destOff = UID_TLV;
				//checkTag |= 0x01;
				break;
			case 0x81:
				CheckBasic(ParaPtr, 1);
				destOff = SAK_TLV;
				//checkTag |= 0x02;
				break;
			case 0x82:
				CheckBasic(ParaPtr, 2);
				destOff = ATQA_TLV;
				//checkTag |= 0x04;
				break;
			case 0x83:
				CheckLV(ParaPtr, 15, Type);
				if (Type == TypeA1)
				{
					//Length Operation Indicator
					//'00': Application doesn't care about ATS historical bytes length.
					//'0F': Application does care about ATS historical bytes length and the XOR operator (strict equality) shall be used.
					//'FF': Application does care about ATS historical bytes length and the MAX operator shall be used.
					if ( (ParaPtr[2] != 0x00) && (ParaPtr[2] != 0x0F) && (ParaPtr[2] != 0xFF) )
					{
						throw_error(SW_WRONG_DATA);
					}
				}
				destOff = ATS_HIS_TLV;
				//checkTag |= 0x08;
				break;
			case 0x84:
				CheckBasic(ParaPtr, 1);  
				if (Type == TypeA1)
				{
					//Operation Indicator for FWI, SFGI
					//'00': Application doesn't care about the FWI and SFGI values
					//'F0': Application requests FWI does not exceed the specified value, but does not care about the SFGI value.
					//'FF': Application requests that the values for FWI and SFGI do not exceed their specified values.
					//'0F': Application requests that the value of SFGI does not exceed the specified value and does not care about the value of FWI.
					if ( (ParaPtr[2] != 0x00) && (ParaPtr[2] != 0xF0) && (ParaPtr[2] != 0x0F) && (ParaPtr[2] != 0xFF) )
					{
						throw_error(SW_WRONG_DATA);
					}
				}
				destOff = FWI_SFGI_TLV;
				//checkTag |= 0x10;
				break;
			case 0x85:
				CheckBasic(ParaPtr, 1);  
				
				if (Type == TypeA0)
				{
					//CID Support: Support of the Card Identifier field as defined in [14443-4].
					//'01' ATS indicates CID supported
					//'00' ATS indicates CID not supported
					if ( (ParaPtr[2] != 0x01) && (ParaPtr[2] != 0x00) )
					{
						throw_error(SW_WRONG_DATA);
					}
				}
				destOff = CID_TLV;
				//checkTag |= 0x20;
				break;
			case 0x86:
				CheckBasic(ParaPtr, 3);  
				if (Type == TypeA1)
				{
					//Operation Indicator for mandatory bits within DATARATE_MAX.
					//Byte 1: '00': Application does not care about the Maximum bit rate in the PCD to PICC direction
					//		  'FF': Application requests that the indicated Maximum bit rate in the PCD to PICC direction is not exceeded
					//Byte 2: '00': Application does not care about the Maximum bit rate in the PICC to PCD direction
					//        'FF': Application requests that the indicated Maximum bit rate in the PICC to PCD direction is not exceeded
					//Byte 3: '00': Application does not care about this parameter 
					//		  'FF': Application requests that value of Byte 3 of DATARATE_MAX is respected
					if ( ((ParaPtr[2] != 0x00) && (ParaPtr[2] != 0xFF)) || ((ParaPtr[3] != 0x00) && (ParaPtr[3] != 0xFF)) || ((ParaPtr[4] != 0x00) && (ParaPtr[4] != 0xFF)) )
					{
						throw_error(SW_WRONG_DATA);
					}
				}
				destOff = DATA_RATE_MAX_TLV;
				//checkTag |= 0x40;
				break;
			default:
				throw_error(SW_WRONG_DATA);
    		}
   			
   			length 		= U8t16(U8_A_U8(ParaPtr[1], 2));
   			ecpMem_Copy(&dest[destOff], &ParaPtr[0], length);
   			ParaPtr 	= PM8_A_U8(ParaPtr, length);
			
			if ( (Type == TypeA0) && ((destOff == UID_TLV) || (destOff == ATS_HIS_TLV)) )
			{
				PR_W_U8(&ComputContext->ResultMask[U16_A_U8(destOff, 1)], dest[U16_A_U8(destOff, 1)]);
			}
			else if ( (Type == TypeA1) && ((destOff == UID_TLV) || (destOff == ATS_HIS_TLV)) )
			{
				if (length == 3)
				{
					//uid length is 0, set uid mask all is 0
					if (destOff == UID_TLV)
					{
						ecpMem_Fill(&dest[destOff + 3], 0, 10);
					}
				}
			}
    	}
		
//		if (checkTag != 0x7F)
//		{
//			throw_error(SW_WRONG_DATA);
//		}
    }

//	if (sysConfig_GetContactlessParaSwitch())
//	{
//		TypeAProtoDataTLVToCompact(MEMtPM8(ComputContext->ResultData), MEMtPM8(JcvmConfig_GetVmConfigAddr(SYSCONFIG_OFFSET_START)));
//	}
	//else
	{
		memCopyAtomic(ComputContext->ProtocolData, ComputContext->ResultData, PARA_TYPEA_LENGTH);
		memCopyAtomic(ComputContext->MandatoryMask, ComputContext->ResultMask, PARAMASK_TYPEA_LENGTH);
	}

	
	memRefresh();
}

// modify by wuronghua@20211011   增加了对 A1 tag 参数合法性的检查

//A0 | L | V | A1 | L | V

// modify by wuronghua@20220811 直接使用ParaBuf（NVM）中的数据时，因为数据可能在cache中，造成数据不一致，因此读到tmpBuf（RAM）再使用
void StoreProtocolTypeBPara(PM8 ParaBuf, u16 ParaLen)
{
	PM8 				ParaLimit;
    PM8 				ParaPtr;
	PM8 				dest;
	PM8 				TagAXLimit;
	u8 					Type;
	u8	 				tmpBuf[PARA_TYPEB_LENGTH + PARAMASK_TYPEB_LENGTH];
	u8					checkTag;
    

	memCopyAtomic(MEMtPM8(ComputContext->ResultData), MEMtPM8(ComputContext->ProtocolData), PARA_TYPEB_LENGTH);
	memCopyAtomic(MEMtPM8(ComputContext->ResultMask), MEMtPM8(ComputContext->MandatoryMask), PARAMASK_TYPEB_LENGTH);

//	if (sysConfig_GetContactlessParaSwitch())
//	 {
//		 TypeAProtoDataCompactToTLV(MEMtPM8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_START)), ComputContext->ResultData);
//     }
	 
	//flush_flag maybe is 1, need get data from stream_buffer
	memCopyNoAtomic(tmpBuf, ParaBuf, ParaLen);
    ParaPtr			 	= tmpBuf;
	ParaLimit 			= PM8_A_U16(ParaPtr, ParaLen);
	
	while (ParaPtr < ParaLimit) 
	{
		//u8* TagAXLimit;

        if (ParaPtr[0] == 0xA0)
        {
			Type = TypeA0 ;
        	dest = MEMtPX8(ComputContext->ResultData);
        }
        else //0xA1
        {
			Type = TypeA1 ;
        	dest = MEMtPX8(ComputContext->ResultMask);
        }
        
    	TagAXLimit = PM8_A_U8(ParaPtr, (U8_A_U8(ParaPtr[1], 2)));
            
    	ParaPtr    = PM8_A_U8(ParaPtr, 2);
		
		//checkTag   = 0;
    	
    	while(ParaPtr < TagAXLimit)
   		{
    		u16 destOff = 0;
	        u16 length;
   			switch (ParaPtr[0]) {
			case 0x80:
				CheckLV(ParaPtr, 5, Type);
				if (Type == TypeA1)
				{
					//Length Operation Indicator
					//'00': Application doesn抰 care about PUPI Length
					//'FF': Application does care about PUPI length and the XOR operator shall be used (strict equality).
					if ( (ParaPtr[2] != 0x00) && (ParaPtr[2] != 0xFF) )
					{
						throw_error(SW_WRONG_DATA);
					}
				}
				destOff = PUPI_TLV;
				//checkTag |= 0x01;
				break;
			case 0x81:
				CheckBasic(ParaPtr, 1);
				destOff = AFI_TLV;
				//checkTag |= 0x02;
				break;
			case 0x82:
				CheckBasic(ParaPtr, 4);
				destOff = ATQB_TLV;
				//checkTag |= 0x04;
				break;
			case 0x83:
				CheckLV(ParaPtr, 15, Type);
				if (Type == TypeA1)
				{
					//Length Operation Indicator
					//'00': Application doesn't care about Higher Layer Response Length.
					//'FF': Application does care about Higher Layer Response Length and the MAX operator shall be used.
					if ( (ParaPtr[2] != 0x00) && (ParaPtr[2] != 0xFF) )
					{
						throw_error(SW_WRONG_DATA);
					}
				}
				destOff = TB_HIS_TLV;
				//checkTag |= 0x08;
				break;
			case 0x84:
				CheckBasic(ParaPtr, 3);  
				if (Type == TypeA1)
				{
					//Operation Indicator for mandatory bits within DATARATE_MAX.
					//Byte 1: '00': Application does not care about the Maximum bit rate in the PCD to PICC direction
					//        'FF': Application requests that the indicated Maximum bit rate in the PCD to PICC direction is not exceeded
					//Byte 2: '00': Application does not care about the Maximum bit rate in the PICC to PCD direction
					//        'FF': Application requests that the indicated Maximum bit rate in the PICC to PCD direction is not exceeded
					//Byte 3: '00': Application does not care about this parameter
					//        'FF': Application requests that value of Byte 3 of DATARATE_MAX is respected
					if ( ((ParaPtr[2] != 0x00) && (ParaPtr[2] != 0xFF)) || ((ParaPtr[3] != 0x00) && (ParaPtr[3] != 0xFF)) || ((ParaPtr[4] != 0x00) && (ParaPtr[4] != 0xFF)) )
					{
						throw_error(SW_WRONG_DATA);
					}
				}
				destOff = TB_DATA_RATE_MAX_TLV;
				//checkTag |= 0x10;
				break;
			default:
				throw_error(SW_WRONG_DATA);
    		}
   			
   			length = U8t16(U8_A_U8(ParaPtr[1], 2));
   			ecpMem_Copy(&dest[destOff], &ParaPtr[0], length);
   			ParaPtr = PM8_A_U16(ParaPtr, length);
			
			if ( (Type == TypeA0) && ( (destOff == PUPI_TLV) || (destOff == TB_HIS_TLV) ) )
			{
				PR_W_U8(&ComputContext->ResultMask[U16_A_U8(destOff, 1)], dest[U16_A_U8(destOff, 1)]);
			}
    	}
		
//		if (checkTag != 0x1F)
//		{
//			throw_error(SW_WRONG_DATA);
//		}

    }
    

//	if (sysConfig_GetContactlessParaSwitch())
//	{
//		TypeBProtoDataTLVToCompact(MEMtPM8(ComputContext->ResultData), MEMtPM8(JcvmConfig_GetVmConfigAddr(SYSCONFIG_OFFSET_START)));
//	}
	//else
	{
		memCopyAtomic(ComputContext->ProtocolData, ComputContext->ResultData, PARA_TYPEB_LENGTH);
		memCopyAtomic(ComputContext->MandatoryMask, ComputContext->ResultMask, PARAMASK_TYPEB_LENGTH);
	}
	memRefresh();
}



static u16 constructTlv(PM8 dest, PM8 src)
{
	u16 		len;

	len = U8t16(U8_A_U8(src[1], 2));
	memCopyAtomic(dest, src, len);
	return len;
}


const u8 TypeA_TLV_OFF[] = {UID_TLV, SAK_TLV, ATQA_TLV, ATS_HIS_TLV, FWI_SFGI_TLV, CID_TLV, DATA_RATE_MAX_TLV};
const u8 TypeB_TLV_OFF[] = {PUPI_TLV, AFI_TLV, ATQB_TLV, TB_HIS_TLV, TB_DATA_RATE_MAX_TLV};

//A0 | L | V | A1 | L | V
u16 GetProtocolPara(memref ArrayObj, u8 offset, u8 type, u8 flag)
{
	u8 					ParaOffset;
	u8 					i;
	u8					j;
	u8 					TL[2];
	PX8 				ParaBufBak;
	PX8					ParaBuf;
	PM8		 			dest;
	PM8 				src;
	u8 					tmpBuf[EECFG_TPA_TLVDATA_SIZE];
	u8					tmpLen;
	u8					dataLength;
	u8 					occur = 1;
	dest 				= MEMtPX8(ComputContext->ResultData);
	ParaBuf 			= MEMtPX8(U32_A_U8(get_array_data(MEMtPF8(ArrayObj)), offset));// modified by zm;  because it need to check the bound of ArrayObj when it is writed
	ParaBufBak  		= ParaBuf;

	#if JCVM__VM__FIRE_WALL
		checkArrayRead(MEMtPF8(ArrayObj));
	#endif  //JCVM__VM__FIRE_WALL
		/* check for firewall */

		check_null(MEMtPF8(ArrayObj));


	check_bounds_interval(MEMtPF8(ArrayObj), offset, 0);
	
	i = 0;
#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	occur = 2;
#endif
	if(PXB_R_U8(PNtMEM(&bRamBuffer[GP_GETSTATUS_P1])) == 0x80)//OPEN typeA/B参数
	{
		occur = 1;
	}
	for(i=0; i<occur; U8_INC(i))
	{
		src = (i==0) ? ComputContext->ProtocolData : ComputContext->MandatoryMask;

		ParaOffset = 0;
		dataLength = 0;

		if(type == APDU_PROTOCOL_TA)
		{
			for(j=0; j<sizeof(TypeA_TLV_OFF); U8_INC(j))
			{
				ParaOffset = U8_A_U8(ParaOffset, U16t8(constructTlv(&dest[ParaOffset], &src[ TypeA_TLV_OFF[j] ])));
				
				if(src[ TypeA_TLV_OFF[j] + 1 ] != 0)
				{
					memCopyNoAtomic(&tmpBuf[dataLength], &src[ TypeA_TLV_OFF[j] ], src[ TypeA_TLV_OFF[j] + 1 ] + 2);
					dataLength += (src[ TypeA_TLV_OFF[j] + 1 ] + 2);
				}
			}
		}
		else
		{
			for(j=0; j<sizeof(TypeB_TLV_OFF); U8_INC(j))
			{
				ParaOffset = U8_A_U8(ParaOffset, U16t8(constructTlv(&dest[ParaOffset], &src[ TypeB_TLV_OFF[j] ])));

				if(src[ TypeB_TLV_OFF[j] + 1 ] != 0)
				{
					memCopyNoAtomic(&tmpBuf[dataLength], &src[ TypeB_TLV_OFF[j] ], src[ TypeB_TLV_OFF[j] + 1 ] + 2);
					dataLength += (src[ TypeB_TLV_OFF[j] + 1 ] + 2);
				}
			}
		}
	
		// added by zm; check bound of array.
			check_bounds_interval(MEMtPF8(ArrayObj), 0, U32t16(U32_A_U8(U32_S_U32(PXtMEM(ParaBuf), PXtMEM(ParaBufBak)), U8_A_U8(offset, U8_A_U8(2, ParaOffset)))));
		
		if (flag == 1)
		{
			//get applet contactless parameters
			if(i == 0)
			{
				TL[0]	= 0xA0;
			}
			else
			{
				TL[0]   = 0xA1;
			}

			TL[1] = dataLength;
			memCopyAtomic(MEMtPM8(ParaBuf), TL, 2);
			
			ParaBuf += 2;
		}
		else
		{
			//if is get current parameters, skip 'A1' tag
			i++ ;
		}
		//memCopyAtomic(MEMtPM8(PX8_A_U8(ParaBuf, 2)), dest, U8t16(ParaOffset));
		//ParaBuf = PX8_A_U8(ParaBuf, U8_A_U8(ParaOffset, 2));
		memCopyAtomic(MEMtPM8(ParaBuf), tmpBuf, U8t16(dataLength));

		ParaBuf = PX8_A_U8(ParaBuf, dataLength);
//#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK		
//		if (sysConfig_GetContactlessParaSwitch())
//		{
//			break;
//		}
//#endif
	}

	// see "GP API contactless";( modified by zm)
	return U32t16(U32_S_U32(PXtMEM(ParaBuf), PXtMEM(ParaBufBak)));
	//return U32t16(U32_A_U8(U32_S_U32(PXtMEM(ParaBuf), PXtMEM(ParaBufBak)), offset));
}


ComputContextStruct * ComputContext; 

void InitComputContext()
{
	/*@ -discast @*/ComputContext = (ComputContextStruct *)(bTempArr); /*@ +discast @*/
}

void ComputStart(u8 ProtoType)
{
	PM8 CurrentCLParaBuf = MEMtPM8(GetCurrentCLParaBuf());
	PM8 PrepareCLParaBuf = MEMtPM8(GetPrepareCLParaBuf());

	if(ProtoType == APDU_PROTOCOL_TA)
	{
		ComputContext->CurrentProtocolData = &CurrentCLParaBuf[EECFG_TPA_TLVDATA];
		ComputContext->CurrentMandatoryMask = &CurrentCLParaBuf[EECFG_TPA_TLVMASK];
		//ComputContext->CurrentCompactData = &CurrentCLParaBuf[EECFG_TYPEA];
		 
		ComputContext->PrepaProtocolData = &PrepareCLParaBuf[EECFG_TPA_TLVDATA]; 
		ComputContext->PrepaMandatoryMask = &PrepareCLParaBuf[EECFG_TPA_TLVMASK];
		//ComputContext->PrepaCompactData = &PrepareCLParaBuf[EECFG_TYPEA];
	}
	else
	{
		ComputContext->CurrentProtocolData = &CurrentCLParaBuf[EECFG_TPB_TLVDATA];
		ComputContext->CurrentMandatoryMask = &CurrentCLParaBuf[EECFG_TPB_TLVMASK];
		//ComputContext->CurrentCompactData = &CurrentCLParaBuf[EECFG_TYPEB];
		 
		ComputContext->PrepaProtocolData = &PrepareCLParaBuf[EECFG_TPB_TLVDATA]; 
		ComputContext->PrepaMandatoryMask = &PrepareCLParaBuf[EECFG_TPB_TLVMASK];
		//ComputContext->PrepaCompactData = &PrepareCLParaBuf[EECFG_TYPEB];	
	}

//	ecpMem_Set(ComputContext->ResultData, 0, 49);
//	ecpMem_Set(ComputContext->ResultMask, 0, 49);
	
	memCopyAtomic(PrepareCLParaBuf, CurrentCLParaBuf, EECFG_SIZE);
	memCopyAtomic(MEMtPM8(ComputContext->ResultData), ComputContext->CurrentProtocolData, 49);
	memCopyAtomic(MEMtPM8(ComputContext->ResultMask), ComputContext->CurrentMandatoryMask, 49);
}

void ComputEnd(u8 ProtoType)
{
    /*if(bEECFG == (u8*)bEECFG1)
	{	
		memCopyAtomic(bEECFG2, bEECFG1, EECFG_SIZE);
	}
	else{
		memCopyAtomic(bEECFG1, bEECFG2, EECFG_SIZE);
	}*/
	memCopyAtomic(ComputContext->PrepaProtocolData, ComputContext->ResultData, 49);
	memCopyAtomic(ComputContext->PrepaMandatoryMask, ComputContext->ResultMask, 49);

	if(ProtoType == APDU_PROTOCOL_TA)
	{
	   TypeAProtoDataTLVToCompact(MEMtPX8(ComputContext->ResultData), MEMtPM8(JcvmConfig_GetVmConfigAddr(SYSCONFIG_OFFSET_START)));
	 }
	 else
	 {
	 	 TypeBProtoDataTLVToCompact(MEMtPX8(ComputContext->ResultData), MEMtPM8(JcvmConfig_GetVmConfigAddr(SYSCONFIG_OFFSET_START)));
	 }
	//DT???D???????
	SwitchCLParaBuf();
}

void LvDataComput(short TagOff)
{

	u8 AppLOI = ComputContext->MandatoryMask[U8_A_U8(TagOff, 2)];
	u8 AppL = ComputContext->ProtocolData[U8_A_U8(TagOff, 2)];
	u8 CurrLOI = ComputContext->CurrentMandatoryMask[U8_A_U8(TagOff, 2)];
	u8 CurrL = ComputContext->CurrentProtocolData[U8_A_U8(TagOff, 2)];
	
	u8 AppAllLength = ComputContext->ProtocolData[U8_A_U8(TagOff, 1)];
	u8 CurrAllLength = ComputContext->CurrentProtocolData[U8_A_U8(TagOff, 1)];
	
	if(AppLOI == XOR_OPERATOR) 
	{
	}
	
	else if(AppLOI == MAX_OPERATOR)
	{
		if(CurrLOI == MAX_OPERATOR)
		{
			if(CurrL < AppL)
			{
				AppL = CurrL;
				AppAllLength = CurrAllLength;
			}
				
		}
		
		//CurrAllLength = AppAllLength ;
	}
	else
	{
		if((CurrLOI == MAX_OPERATOR) || (CurrLOI == XOR_OPERATOR))	   
		{
			AppL = CurrL;
			AppAllLength = CurrAllLength;
		}
		
		if(CurrL > AppL)
		{
			AppL = CurrL;
			AppAllLength = CurrAllLength;
		}
			
	}

    ComputContext->ResultData[U8_A_U8(TagOff, 1)] = AppAllLength;
	ComputContext->ResultData[U8_A_U8(TagOff, 2)] = AppL;
	
	BasicDataComput(U8t16(U8_A_U8(TagOff, 3)), AppL);
}

void BasicDataComput(u16 ValueOff, u16 Len)
{
	u8 				CD;
	u8 				AD;
	u8 				AM;
	u16 			i;
	u16 			Limit;

	Limit = U16_A_U16(ValueOff, Len);

	for(i = ValueOff; i < Limit; U16_INC(i))
	{
		CD = ComputContext->CurrentProtocolData[i];
		AD = ComputContext->ProtocolData[i];
		AM = ComputContext->MandatoryMask[i];
		
		ComputContext->ResultData[i] = U8_OR_U8(U8_OR_U8(U8_AND_U8(CD, AD), U8_AND_U8(AD, AM)), U8_AND_U8(CD, U8_INV(AM)));
	}
}

/*
 * 
 * */
void NumericDataComput(u16 ValueByteOff, u8 fullbyte)
{
	u8 						CD;
	u8 						CM;
	u8 						AD;
	u8 						AM;
	
	CD 		= ComputContext->CurrentProtocolData[ValueByteOff];
	CM 		= ComputContext->CurrentMandatoryMask[ValueByteOff];
	AD 		= ComputContext->ProtocolData[ValueByteOff];
	AM 		= ComputContext->MandatoryMask[ValueByteOff];

	if(fullbyte == true)
	{
		if(AM == MAX_OPERATOR)
		{
			if((CM == MAX_OPERATOR))
			{
				if(AD < CD) 
				   ComputContext->ResultData[ValueByteOff] = AD ;
			}else{
				 ComputContext->ResultData[ValueByteOff] = AD ;
			}
		}else{ //4.4.2  If an Application does not request a maximum value, then the current Protocol Parameter is kept
			 ComputContext->ResultData[ValueByteOff] = CD ;
		}
	}
	else 
	{
		u8 tmp = ComputContext->CurrentProtocolData[ValueByteOff];

		if(U8_AND_U8(AM, 0x0F) == 0x0F)
		{
		    if(U8_AND_U8(CM, 0x0F) == 0x0F)
			{
				if(U8_AND_U8(AD, 0x0F) < U8_AND_U8(CD, 0x0F))
			    {
					tmp = U8_AND_U8(tmp, 0xF0);
					tmp = U8_OR_U8(tmp, U8_AND_U8(AD, 0x0F));
			    }
			}else{
				tmp = U8_AND_U8(tmp, 0xF0);
			    tmp = U8_OR_U8(tmp, U8_AND_U8(AD, 0x0F));
			}
		}else{
			tmp = U8_AND_U8(tmp, 0xF0);
			tmp = U8_OR_U8(tmp, U8_AND_U8(CD, 0x0F));
		}
		
		CM = U8_SHR_U8(CM, 4);
		AM = U8_SHR_U8(AM, 4);
		CD = U8_SHR_U8(CD, 4);
		AD = U8_SHR_U8(AD, 4);
		
		if(AM == 0x0F)
		{
			if(CM == 0x0F)
			{
				if(AD < CD)
				{
					tmp = U8_AND_U8(tmp, 0x0F);
					tmp = U8_OR_U8(tmp, U8_SHL_U8(AD, 4));
				}
			}else{
				tmp = U8_AND_U8(tmp, 0x0F);
				tmp = U8_OR_U8(tmp, U8_SHL_U8(AD, 4));
			}
		}else{
			tmp = U8_AND_U8(tmp, 0x0F);
			tmp = U8_OR_U8(tmp, U8_SHL_U8(CD, 4));
		}
		
		ComputContext->ResultData[ValueByteOff] = tmp;
	}
	
	
}

void BasicMaskComput(u16 TagOff, u8 ValueOff)
{

	u16 				Len;
	u16 				i; // i:13
	u16 				Limit; //limit:15

	Len 				= U8t16(ComputContext->ResultMask[U16_A_U8(TagOff, 1)]) ;
	i 					= U16_A_U8(TagOff, ValueOff);
	Limit 				= U16_A_U16(i, Len); //limit:15
	
	while(i < Limit)
	{
		ComputContext->ResultMask[i] = U8_OR_U8(ComputContext->CurrentMandatoryMask[i], ComputContext->MandatoryMask[i]);
		U16_INC(i);
	}
}

void LMaskComput(u16 TagOff)
{
	u8 					CM_LOI;
	u8 					AM_LOI;
	u8 					result ;
	u16 				LOI_OFF;
	
	LOI_OFF 			= U16_A_U8(TagOff, 2);
	CM_LOI 				= ComputContext->CurrentMandatoryMask[LOI_OFF];
	AM_LOI 				= ComputContext->MandatoryMask[LOI_OFF];

	if((CM_LOI == 0x0F) || (AM_LOI == 0x0F))
	{
		result = 0x0F;
	}
	else if((CM_LOI == 0) && (AM_LOI == 0))
	{
		result = 0;
	}
	else
	{
		result = 0xFF;
	}
	
	ComputContext->ResultMask[U16_A_U8(TagOff, 1)] = ComputContext->ResultData[U16_A_U8(TagOff, 1)] ;
	ComputContext->ResultMask[LOI_OFF] = result;
}

void TypeAComputeCurrentProtocolParameter()
{ 
	ComputStart(APDU_PROTOCOL_TA);
	
	LvDataComput(UID_TLV);
	BasicDataComput(U8t16(U8_A_U8(SAK_TLV, 2)), 1);
	//BasicDataComput(ATQA_TLV +2, 1);
	BasicDataComput(U8t16(U8_A_U8(ATQA_TLV, 2)), 2);
	LvDataComput(ATS_HIS_TLV);
	NumericDataComput(U8t16(U8_A_U8(FWI_SFGI_TLV, 2)), false);
	BasicDataComput(U8t16(U8_A_U8(CID_TLV, 2)), 1);
	NumericDataComput(U8t16(U8_A_U8(DATA_RATE_MAX_TLV, 2)), true);
	NumericDataComput(U8t16(U8_A_U8(DATA_RATE_MAX_TLV, 3)), true);
	BasicDataComput(U8t16(U8_A_U8(DATA_RATE_MAX_TLV, 4)), 1);
	
	LMaskComput(UID_TLV); 
	BasicMaskComput(UID_TLV, 3);
	BasicMaskComput(SAK_TLV, 2);
	BasicMaskComput(ATQA_TLV, 2);
	LMaskComput(ATS_HIS_TLV); 
	BasicMaskComput(ATS_HIS_TLV, 3);
	BasicMaskComput(FWI_SFGI_TLV, 2);
	BasicMaskComput(CID_TLV, 2);
	BasicMaskComput(DATA_RATE_MAX_TLV, 2);

	ComputEnd(APDU_PROTOCOL_TA);

}

void TypeBComputeCurrentProtocolParameter()
{
	ComputStart(APDU_PROTOCOL_TB);
	
	LvDataComput(PUPI_TLV);
	BasicDataComput(U8t16(U8_A_U8(AFI_TLV, 2)), 1);
	BasicDataComput(U8t16(U8_A_U8(ATQB_TLV, 2)), 4);
	LvDataComput(TB_HIS_TLV);
	NumericDataComput(U8t16(U8_A_U8(TB_DATA_RATE_MAX_TLV, 2)), true);
	NumericDataComput(U8t16(U8_A_U8(TB_DATA_RATE_MAX_TLV, 3)), true);
	BasicDataComput(U8t16(U8_A_U8(TB_DATA_RATE_MAX_TLV, 4)), 1);
	
	LMaskComput(PUPI_TLV); 
	BasicMaskComput(PUPI_TLV, 3);
	BasicMaskComput(AFI_TLV, 2);
	BasicMaskComput(ATQB_TLV, 2);
	LMaskComput(TB_HIS_TLV); 
	BasicMaskComput(TB_HIS_TLV, 3);
	BasicMaskComput(TB_DATA_RATE_MAX_TLV, 2);

	ComputEnd(APDU_PROTOCOL_TB);

}

// modify by wuronghua@20211011 
// V1.1 modify by xwh@20211210，若更新CLF不成功，打上标记G_Swp_NeedUpdataPara，在SWP_Process中自动更新（解决更新UID后需要重启手机的问题）
void TypeAProtoDataTLVToCompact(PM8 scr, PM8 dest)
{
	u8 							tmp;
	u8							TmpATSLen;
	u8 							byte1;
	u8 							byte2;
	u8 							byte3;
	u8 							TA1dr;
	u8 							TA1ds;
	u8 							TA1d;
	u8                          updateMifareFlag;
	
#if JCVM__IO__SWP
	SWP_CONFIG_NVM_Context_t *swpContext;

	swpContext = (SWP_CONFIG_NVM_Context_t *)JcvmConfig_GetVmConfigAddr(SWP_CONFIG_START);
#endif
	
#if INFINEON_FSR8
	u8 sak = bEECFG[EECFG_TYPEA_SAK];			   //get the default sak.
#endif

#if JCVM__IO__SWP

	updateMifareFlag = 0;

	if (scr[U8_A_U8(UID_TLV, 2)] != 0x00)
	{	//Mifare disable
		//UID LV
		if (ecpMem_Compare(&swpContext->TypeA_Registry.uid[0], &scr[U8_A_U8(UID_TLV, 2)], U8_A_U8(scr[U8_A_U8(UID_TLV, 2)], 1)) != 0)
		{
			memCopyNoAtomic(MEMtPM8(&swpContext->TypeA_Registry.uid[0]), MEMtPM8(&scr[U8_A_U8(UID_TLV, 2)]), U8_A_U8(scr[U8_A_U8(UID_TLV, 2)], 1));
			
			Swp_UpdateTypeAParameters(SWP_RFTYPEA_GATE_REG_INDEX_UID, MEMtPM8(&scr[U8_A_U8(UID_TLV, 3)]), scr[U8_A_U8(UID_TLV, 2)]);

			updateMifareFlag  = 1;
		}
		ecpMem_Copy(&aReg_CardRFTypeA_UID[1], MEMtPM8(&scr[U8_A_U8(UID_TLV, 3)]),  scr[U8_A_U8(UID_TLV, 2)]);
		updateMifareFlag  = true;
		updateFlag = true;
	}
	
	
#if GP__VERSION__FOR_CHINA_UNICOM_MASK
	if (PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CONTROL))) == 0x01)
	{
		//SAK	
		memCopyAtomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CURRENT_SAK)), MEMtPM8(&scr[SAK_TLV + 2]), 1);
		//ATQA				
		memCopyAtomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CURRENT_ATQA)), MEMtPM8(&scr[ATQA_TLV + 2]), 2);
	}
	else
#endif
    if (scr[U8_A_U8(SAK_TLV, 1)] != 0x00)
	{
		//SAK	
		if (swpContext->TypeA_Registry.sak != scr[U8_A_U8(SAK_TLV, 2)])
		{
			memCopyNoAtomic(MEMtPM8(&swpContext->TypeA_Registry.sak), MEMtPM8(&scr[U8_A_U8(SAK_TLV, 2)]), 1);
			
			Swp_UpdateTypeAParameters(SWP_RFTYPEA_GATE_REG_INDEX_SAK, MEMtPM8(&scr[SAK_TLV + 2]), 1);
			
			updateMifareFlag  = 1;
		}
    }

	
	if (scr[U8_A_U8(ATQA_TLV, 1)] != 0x00)
	{
		//ATQA				
		if (ecpMem_Compare(MEMtPM8(&swpContext->TypeA_Registry.atqa[0]), MEMtPM8(&scr[U8_A_U8(ATQA_TLV, 2)]), 2) != 0)
		{
			memCopyNoAtomic(MEMtPM8(&swpContext->TypeA_Registry.atqa[0]), MEMtPM8(&scr[U8_A_U8(ATQA_TLV, 2)]), 2);
			
			Swp_UpdateTypeAParameters(SWP_RFTYPEA_GATE_REG_INDEX_ATQA, MEMtPM8(&scr[ATQA_TLV + 2]), 2);
			
			updateMifareFlag  = 1;
		}
    }


	//ats history bytes
	//tmp = scr[U8_A_U8(ATS_HIS_TLV, 2)];
	//TmpATSLen =  tmp + 5 ;
	//MEM_write_array((memref)&swpContext->TypeA_Registry.appData[0], (memref)&TmpATSLen, 1);
	if (scr[U8_A_U8(ATS_HIS_TLV, 2)] != 0)
	{
		if (ecpMem_Compare(MEMtPM8(&swpContext->TypeA_Registry.appData[0]), MEMtPM8(&scr[U8_A_U8(ATS_HIS_TLV, 2)]), U8_A_U8(scr[U8_A_U8(ATS_HIS_TLV, 2)], 1)) != 0)
		{
			memCopyNoAtomic(MEMtPM8(&swpContext->TypeA_Registry.appData[0]), MEMtPM8(&scr[U8_A_U8(ATS_HIS_TLV, 2)]), U8_A_U8(scr[U8_A_U8(ATS_HIS_TLV, 2)], 1));
			
			Swp_UpdateTypeAParameters(SWP_RFTYPEA_GATE_REG_INDEX_APP_DATA, MEMtPM8(&scr[U8_A_U8(ATS_HIS_TLV, 3)]), scr[U8_A_U8(ATS_HIS_TLV, 2)]);
		}
		
		ecpMem_Copy(&aReg_CardRFTypeA_History[0], MEMtPM8(&scr[U8_A_U8(ATS_HIS_TLV, 2)]),scr[U8_A_U8(ATS_HIS_TLV, 2)]+1);
		updateFlag = true;
	}
	if (scr[U8_A_U8(FWI_SFGI_TLV, 1)] != 0x00)
	{
	//copy FWI,SFGI	
		if (swpContext->TypeA_Registry.fwi_sfgi != scr[U8_A_U8(FWI_SFGI_TLV, 2)])
		{
			memCopyNoAtomic(MEMtPM8(&swpContext->TypeA_Registry.fwi_sfgi), MEMtPM8(&scr[U8_A_U8(FWI_SFGI_TLV, 2)]), 1);
		
			Swp_UpdateTypeAParameters(SWP_RFTYPEA_GATE_REG_INDEX_FWI_SFGI, MEMtPM8(&scr[U8_A_U8(FWI_SFGI_TLV, 2)]), 1);
		}
    }
	//copy CID
	if (scr[U8_A_U8(CID_TLV, 1)] != 0x00)
	{
	    //tmp = ((scr[CID_TLV +2] == 1) ? 0x03 : 0x01);
		if (swpContext->TypeA_Registry.cid != scr[U8_A_U8(CID_TLV, 2)])
		{
			memCopyNoAtomic(MEMtPM8(&swpContext->TypeA_Registry.cid), MEMtPM8(&scr[U8_A_U8(CID_TLV, 2)]), 1);
			
			Swp_UpdateTypeAParameters(SWP_RFTYPEA_GATE_REG_INDEX_CID, MEMtPM8(&scr[U8_A_U8(CID_TLV, 2)]), 1);
		}
	}
	//copy DATA_RATE_MAX:
	if (scr[U8_A_U8(DATA_RATE_MAX_TLV, 1)] != 0x00)
	{
		if (memcmp(MEMtPM8(&swpContext->TypeA_Registry.dataRate[0]), MEMtPM8(&scr[U8_A_U8(DATA_RATE_MAX_TLV,  2)]), 3) != 0)
		{
			memCopyNoAtomic(MEMtPM8(&swpContext->TypeA_Registry.dataRate[0]), MEMtPM8(&scr[U8_A_U8(DATA_RATE_MAX_TLV,  2)]), 3);
		
			Swp_UpdateTypeAParameters(SWP_RFTYPEA_GATE_REG_INDEX_DTR_MAX, MEMtPM8(&scr[U8_A_U8(DATA_RATE_MAX_TLV,  2)]), 3);
		}
		
		ecpMem_Copy(&aReg_CardRFTypeA_MAXDataRate[1], MEMtPM8(&scr[U8_A_U8(DATA_RATE_MAX_TLV,  2)]), 3);
		updateFlag = true;
    }

	//reset SWP init
	if (Swp_isNeedClearSessionID())
	{
		SwpHal_ClearSessionID();
	}
	
	if (updateMifareFlag)
	{
		//flush steam buffer
		memRefresh();
#if HAL__IO__MIFARE
		ecpMifareHal_UpdateBlock0();
#endif
	}


#else
	//UID LV
//    memCopyAtomic(&dest[UIDLEN_TYPEA_OFF], &scr[UID_TLV + 2], scr[UID_TLV + 2] + 1);
//
//	//SAK
//	memCopyAtomic(&dest[SAK_TYPEA_OFF], &scr[SAK_TLV + 2], 1);
//	//ATQA
//	memCopyAtomic(&dest[ATQA_TYPEA_OFF], &scr[ATQA_TLV + 2], 2);
//
//	//ats history bytes
//
//	tmp = scr[ATS_HIS_TLV +2];
//	TmpATSLen =  tmp + 5 ;
//	memCopyAtomic(&dest[ATSLEN_TYPEA_OFF], &TmpATSLen, 1);
//	memCopyAtomic(&dest[ATS_LEN_TYPEA_OFF], &TmpATSLen, 1);
//	memCopyAtomic(&dest[ATS_HIS_TYPEA_OFF], &scr[ATS_HIS_TLV +3], tmp);
//
//	//copy FWI,SFGI
//	memCopyAtomic(&dest[ATS_TB1_TYPEA_OFF], &scr[FWI_SFGI_TLV + 2], 1);
//
//	//copy CID
//	tmp = ((scr[CID_TLV +2] == 1) ? 0x03 : 0x01);
//	memCopyAtomic(&dest[ATS_TC1_TYPEA_OFF], &tmp, 1);
//
//	//copy DATA_RATE_MAX:
//	byte1 = scr[DATA_RATE_MAX_TLV + 2];//Byte 1 defines the maximum divisor supported in direction PCD to PICC:
//	byte2 = scr[DATA_RATE_MAX_TLV + 3];//Byte 2 defines the maximum divisor supported in direction PICC to PCD:
//	byte3 = scr[DATA_RATE_MAX_TLV + 4];//Byte 3 defines a limitation to support different divisors for each direction:
//
//	if(byte1 == 1)
//	{
//		TA1dr = 1;
//	}
//	else if(byte1 == 2)
//	{
//		TA1dr = 3;
//	}
//	else if(byte1 == 3)
//	{
//		TA1dr = 7;
//	}
//
//	if(byte2 == 1)
//	{
//		TA1ds = 10;
//	}
//	else if(byte2 == 2)
//	{
//		TA1ds = 30;
//	}
//	else if(byte2 == 3)
//	{
//		TA1ds = 70;
//	}
//
//	if(byte3 == 1)
//	{
//		TA1d = 0x80;
//	}
//
//	tmp = TA1dr | TA1ds | TA1d ;
//	memCopyAtomic(&dest[ATS_TA1_TYPEA_OFF], &tmp, 1);
#if INFINEON_FSR8
	sak = ((scr[SAK_TLV + 2] & 0x83) | (sak & 0x7C));		 // see infineon requirement FSR #8 
		ModifyCurrentSAKValue((u32)(dest - EECFG_TYPEA), (u8)sak);		 
#endif
#endif
}
/**
*modify by wuronghua@20211006  修正了非接触状态下更新UID不成功的问题
*/
void TypeAProtoDataCompactToTLV(PM8 src, PM8 dest)
{
	u8 					tmp;
	PM8			 		tempBuffer;
	
	tempBuffer 			= ComputContext->ResultMask;
	
	ecpMem_Set(tempBuffer, 0, EECFG_TPA_TLVDATA_SIZE);

#if JCVM__IO__SWP
	//UID
	tempBuffer[UID_TLV] = 0x80;	//TAG

	if (src[SWP_TYPEA_UID_LEN] != 0x00)
	{	//Mifare disable
		tempBuffer[U8_A_U8(UID_TLV, 1)] = U8_A_U8(src[SWP_TYPEA_UID_LEN], 1);
		ecpMem_Copy(&tempBuffer[U8_A_U8(UID_TLV, 2)], &src[SWP_TYPEA_UID_LEN], U8_A_U8(src[SWP_TYPEA_UID_LEN], 1));	//UID LV
	}
//	else
//	{
//		//uid LV
//		tempBuffer[U8_A_U8(UID_TLV, 2)] = SwpHal_GetChipUid(&tempBuffer[U8_A_U8(UID_TLV, 3)]);
//		tempBuffer[U8_A_U8(UID_TLV, 1)] = tempBuffer[U8_A_U8(UID_TLV, 2)] + 1;   //LLV struct
//	}
	
	//SAK
	tempBuffer[SAK_TLV] = 0x81;	//TAG
	tempBuffer[U8_A_U8(SAK_TLV, 1)] = 0x01; //LENGTH
	
#if GP__VERSION__FOR_CHINA_UNICOM_MASK
	if (PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CONTROL))) == 0x01)
	{
		tempBuffer[U8_A_U8(SAK_TLV, 2)] = memReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CURRENT_SAK)));
	}
	else
#endif
	{
		tempBuffer[U8_A_U8(SAK_TLV, 2)] = src[SWP_TYPEA_SAK]; //SAK
	}

	//ATQA
	tempBuffer[ATQA_TLV] = 0x82;	//TAG
	tempBuffer[U8_A_U8(ATQA_TLV, 1)] = 0x02; //LENGTH

#if GP__VERSION__FOR_CHINA_UNICOM_MASK
	if (PN_R_U8(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CONTROL))) == 0x01)
	{
		//ATQA				
		memCopyAtomic(MEMtPM8(&tempBuffer[U8_A_U8(ATQA_TLV, 2)]), MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CURRENT_ATQA)), 2);
	}
	else
#endif
	{
		tempBuffer[U8_A_U8(ATQA_TLV, 2)] = src[SWP_TYPEA_ATQA]; //ATQA
		tempBuffer[U8_A_U8(ATQA_TLV, 3)] = src[SWP_TYPEA_ATQA + 1]; //ATQA
	}
	//ATS_HIS
	tempBuffer[ATS_HIS_TLV] = 0x83;	//TAG
	tempBuffer[U8_A_U8(ATS_HIS_TLV, 1)] = U8_A_U8(src[SWP_TYPEA_APPDATA_LEN], 1); //LENTGH
	//tempBuffer[U8_A_U8(ATS_HIS_TLV, 2)] = src[SWP_TYPEA_APPDATA_LEN]; //LENTGH
	ecpMem_Copy(&tempBuffer[U8_A_U8(ATS_HIS_TLV, 2)], &src[SWP_TYPEA_APPDATA_LEN], src[SWP_TYPEA_APPDATA_LEN] + 1); //LENGTH + ATS_HIS

	//FWI SFGI
	tempBuffer[FWI_SFGI_TLV] = 0x84;	//TAG
	tempBuffer[U8_A_U8(FWI_SFGI_TLV, 1)] = 0x01; //LENGTH
	tempBuffer[U8_A_U8(FWI_SFGI_TLV, 2)] = src[SWP_TYPEA_FWI_SFGI]; //FWI_SFGI

	//CID
	tempBuffer[CID_TLV] = 0x85;	//TAG
	tempBuffer[U8_A_U8(CID_TLV, 1)] = 0x01; //LENGTH
	tempBuffer[U8_A_U8(CID_TLV, 2)] = src[SWP_TYPEA_CID_SUPPORT]; //CID

	//DATA RATE MAX
	tempBuffer[DATA_RATE_MAX_TLV] = 0x86;	//TAG
	tempBuffer[U8_A_U8(DATA_RATE_MAX_TLV, 1)] = 0x03; //LENGTH
	tempBuffer[U8_A_U8(DATA_RATE_MAX_TLV, 2)] = src[SWP_TYPEA_DATARATE_MAX] ;
	tempBuffer[U8_A_U8(DATA_RATE_MAX_TLV, 3)] = src[SWP_TYPEA_DATARATE_MAX + 1] ;
	tempBuffer[U8_A_U8(DATA_RATE_MAX_TLV, 4)] = src[SWP_TYPEA_DATARATE_MAX + 2] ;
#else
//	PX_W_U8(&tempBuffer[UID_TLV], 0x80);	//TAG
//	tmp = PN_R_U8(&scr[UIDLEN_TYPEA_OFF]) + 1;
//	ecpMem_Copy(&tempBuffer[UID_TLV +2], &scr[UIDLEN_TYPEA_OFF], tmp);	//UID LV
//	PX_W_U8(&tempBuffer[UID_TLV +1], tmp); //LENTGH
//
//	PX_W_U8(&tempBuffer[SAK_TLV], 0x81);	//TAG
//	PX_W_U8(&tempBuffer[SAK_TLV +1], 0x01); //LENGTH
//	PX_W_U8(&tempBuffer[SAK_TLV +2], PN_R_U8(&scr[SAK_TYPEA_OFF])); //SAK
//
//	PX_W_U8(&tempBuffer[ATQA_TLV], 0x82);	//TAG
//	PX_W_U8(&tempBuffer[ATQA_TLV +1], 0x02); //LENGTH
//	PX_W_U8(&tempBuffer[ATQA_TLV +2], PN_R_U8(&scr[ATQA_TYPEA_OFF])); //ATQA
//	PX_W_U8(&tempBuffer[ATQA_TLV +3], PN_R_U8(&scr[ATQA_TYPEA_OFF +1])); //ATQA
//
//	PX_W_U8(&tempBuffer[ATS_HIS_TLV], 0x83);	//TAG
//	tmp = (PN_R_U8(&scr[ATSLEN_TYPEA_OFF]) - 5);
//	PX_W_U8(&tempBuffer[ATS_HIS_TLV +1], tmp+1); //LENTGH
//	PX_W_U8(&tempBuffer[ATS_HIS_TLV +2], tmp); //LENTGH
//	ecpMem_Copy(&tempBuffer[ATS_HIS_TLV +3], &scr[ATS_HIS_TYPEA_OFF], tmp); //ATS_HIS
//
//	PX_W_U8(&tempBuffer[FWI_SFGI_TLV], 0x84);	//TAG
//	PX_W_U8(&tempBuffer[FWI_SFGI_TLV +1], 0x01); //LENGTH
//	PX_W_U8(&tempBuffer[FWI_SFGI_TLV +2], PN_R_U8(&scr[ATS_TB1_TYPEA_OFF])); //FWI_SFGI
//
//	PX_W_U8(&tempBuffer[CID_TLV], 0x85);	//TAG
//	PX_W_U8(&tempBuffer[CID_TLV +1], 0x01); //LENGTH
//	tmp = PN_R_U8(&scr[ATS_TC1_TYPEA_OFF]);
//	PX_W_U8(&tempBuffer[CID_TLV +2], (tmp == 3) ? 0x01 : 0x00); //FWI_SFGI
//
//	PX_W_U8(&tempBuffer[DATA_RATE_MAX_TLV], 0x86);	//TAG
//	PX_W_U8(&tempBuffer[DATA_RATE_MAX_TLV +1], 0x03); //LENGTH
//
//	PX_W_U8(&tempBuffer[DATA_RATE_MAX_TLV + 2], PN_R_U8(&scr[DATA_RATE_MAX_TLV + 2])) ;
//	PX_W_U8(&tempBuffer[DATA_RATE_MAX_TLV + 3], PN_R_U8(&scr[DATA_RATE_MAX_TLV + 3])) ;
//	PX_W_U8(&tempBuffer[DATA_RATE_MAX_TLV + 4], PN_R_U8(&scr[DATA_RATE_MAX_TLV + 4])) ;
#endif
	memCopyAtomic(dest, tempBuffer, EECFG_TPA_TLVDATA_SIZE);
}

void TypeBProtoDataTLVToCompact(PM8 scr, PM8 dest)
{
	    u8 				byte1;
	    u8 				byte2;
	    u8 				byte3;
	    u8 				temp;
	    u8 				TA1dr;
	    u8 				TA1ds;
	    u8 				TA1d;

#if JCVM__IO__SWP
	SWP_CONFIG_NVM_Context_t *swpContext = (SWP_CONFIG_NVM_Context_t *)JcvmConfig_GetVmConfigAddr(SWP_CONFIG_START);

	//copy PUPI LV
	if(scr[U8_A_U8(PUPI_TLV, 2)] != 0)
	{
		//ecpMem_Copy(&tempBuffer[TYPEB_PUPI_OFF], &scr[PUPI_TLV + 3], scr[PUPI_TLV + 2] + 1);
		memCopyAtomic(MEMtPM8(&swpContext->TypeB_Registry.pupi_reg[0]), MEMtPM8(&scr[U8_A_U8(PUPI_TLV, 2)]), U8_A_U8(scr[U8_A_U8(PUPI_TLV, 2)], 1));
	}
	//copy AFI 
	memCopyAtomic(MEMtPM8(&swpContext->TypeB_Registry.afi), MEMtPM8(&scr[U8_A_U8(AFI_TLV, 2)]), 1);
	//copy ATQB
	memCopyAtomic(MEMtPM8(&swpContext->TypeB_Registry.atqb[0]), MEMtPM8(&scr[U8_A_U8(ATQB_TLV, 2)]), 4);

	//copy history bytes
	if (scr[U8_A_U8(ATS_HIS_TLV, 2)] != 0)
	{
		memCopyAtomic(MEMtPM8(&swpContext->TypeB_Registry.higher_layer_response[0]), MEMtPM8(&scr[U8_A_U8(ATS_HIS_TLV, 3)]), U8_A_U8(scr[U8_A_U8(ATS_HIS_TLV, 2)], 1) );
	}

	//copy DATA_RATE_MAX:
	memCopyAtomic(MEMtPM8(&swpContext->TypeB_Registry.datarate[0]), MEMtPM8(&scr[U8_A_U8(TB_DATA_RATE_MAX_TLV, 2)]), 3);

	//reset SWP init
	memCopyAtomic(MEMtPM8(&swpContext->TypeB_Registry.datarate[0]), MEMtPM8(&scr[U8_A_U8(TB_DATA_RATE_MAX_TLV, 2)]), 3);
	
	//reset SWP init
	Swp_ClearSessionID();
#else

//	    TA1dr 			= 0;
//	    TA1ds 			= 0;
//	    TA1d 			= 0;
//		//copy PUPI LV
//		if(scr[PUPI_TLV + 2] != 0)
//		{
//			//ecpMem_Copy(&tempBuffer[TYPEB_PUPI_OFF], &scr[PUPI_TLV + 3], scr[PUPI_TLV + 2] + 1);
//			memCopyAtomic(&dest[TYPEB_PUPI_OFF], &scr[PUPI_TLV + 3], scr[PUPI_TLV + 2] + 1);
//		}
//		//copy AFI
//		memCopyAtomic(&dest[AFI_TYPEB_OFF], &scr[AFI_TLV + 2], 1);
//
//		memCopyAtomic(&dest[EECFG_TYPEB_ATQB_AFI], &scr[ATQB_TLV + 2], 4);
//
//		//copy history bytes
//		memCopyAtomic(&dest[TYPEB_HISTORICAL_BYTES_OFF], &scr[ATS_HIS_TLV + 3], scr[ATS_HIS_TLV + 2] );
//
//		//copy DATA_RATE_MAX:
//		byte1 = scr[TB_DATA_RATE_MAX_TLV + 2];
//		byte2 = scr[TB_DATA_RATE_MAX_TLV + 3];
//		byte3 = scr[TB_DATA_RATE_MAX_TLV + 4];
//
//		if(byte1 == 1)
//		{
//			TA1dr = 1;
//		}
//		else if(byte1 == 2)
//		{
//			TA1dr = 3;
//		}
//		else if(byte1 == 3)
//		{
//			TA1dr = 7;
//		}
//		if(byte2 == 1)
//		{
//			TA1ds = 10;
//		}
//		else if(byte2 == 2)
//		{
//			TA1ds = 30;
//		}
//		else if(byte2 == 3)
//		{
//			TA1ds = 70;
//		}
//		if(byte3 == 1)
//		{
//			TA1d =  0x80;
//		}
//
//       // tempBuffer[TYPEB_BITRATE_OFF] = (TA1dr | TA1ds | TA1d) ;
//	    temp =  (TA1dr | TA1ds | TA1d) ;
//		memCopyAtomic(&dest[TYPEB_BITRATE_OFF], &temp, 1);
#endif
}


void TypeBProtoDataCompactToTLV(PM8 src, PM8 dest)
 {
	    u8 * tempBuffer;
//#if JCVM__IO__SWP
//	SWP_HCP_NVM_Context_t swpContext;
//#endif
	    
	    tempBuffer 	   = ComputContext->ResultMask;
	
	    ecpMem_Set(tempBuffer, 0, EECFG_TPB_TLVDATA_SIZE);
#if JCVM__IO__SWP
	//memCopyAtomic(MEMtPM8(&swpContext), MEMtPM8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_START)), sizeof(SWP_HCP_NVM_Context_t));
	//copy pupi
	tempBuffer[PUPI_TLV] = 0x80 ;
	tempBuffer[U8_A_U8(PUPI_TLV, 1)] = U8_A_U8(src[SWP_TYPEB_PUPI_LEN], 1) ;
	ecpMem_Copy(&tempBuffer[U8_A_U8(PUPI_TLV, 2)], &src[SWP_TYPEB_PUPI_LEN], U8_A_U8(src[SWP_TYPEB_PUPI_LEN], 1));
	
	//copy AFI 
	tempBuffer[AFI_TLV] = 0x81 ;
	tempBuffer[U8_A_U8(AFI_TLV, 1)] = 0x01 ;
	tempBuffer[U8_A_U8(AFI_TLV, 2)] = src[SWP_TYPEB_AFI];
	
	//copy ATQB
	tempBuffer[ATQB_TLV] = 0x82 ;
	tempBuffer[ATQB_TLV + 1] = 0x04 ;
	ecpMem_Copy(&tempBuffer[U8_A_U8(ATQB_TLV, 2)], &src[SWP_TYPEB_ATQB], 4);
	
	//copy history bytes
	tempBuffer[TB_HIS_TLV] = 0x83 ;
	tempBuffer[U8_A_U8(TB_HIS_TLV, 1)] = U8_A_U8(src[SWP_TYPEB_HIGHT_LAYER_RESPONSE_LEN], 1) ;
	ecpMem_Copy(&tempBuffer[U8_A_U8(TB_HIS_TLV, 2)], &src[SWP_TYPEB_HIGHT_LAYER_RESPONSE_LEN], U8_A_U8(src[SWP_TYPEB_HIGHT_LAYER_RESPONSE_LEN], 1));
	
	//copy DATA_RATE_MAX
	tempBuffer[TB_DATA_RATE_MAX_TLV] = 0x84 ;
	tempBuffer[U8_A_U8(TB_DATA_RATE_MAX_TLV, 1)] = 0x03 ;
	tempBuffer[U8_A_U8(TB_DATA_RATE_MAX_TLV, 2)] = src[SWP_TYPEB_DATARATE_MAX] ;
	tempBuffer[U8_A_U8(TB_DATA_RATE_MAX_TLV, 3)] = src[SWP_TYPEB_DATARATE_MAX + 1] ;
	tempBuffer[U8_A_U8(TB_DATA_RATE_MAX_TLV, 4)] = src[SWP_TYPEB_DATARATE_MAX + 2] ;
#else
        //copy pupi
//	PX_W_U8(&tempBuffer[PUPI_TLV], 0x80) ;
//	PX_W_U8(&tempBuffer[PUPI_TLV + 1], 0x05) ;
//	PX_W_U8(&tempBuffer[PUPI_TLV + 2], 0x04) ;
//	ecpMem_Copy(&tempBuffer[PUPI_TLV + 3], &scr[TYPEB_PUPI_OFF], 4);
//
//		//copy AFI
//	PX_W_U8(&tempBuffer[AFI_TLV], 0x81) ;
//	PX_W_U8(&tempBuffer[AFI_TLV + 1], 0x01) ;
//	PX_W_U8(&tempBuffer[AFI_TLV + 2], PN_R_U8(&scr[AFI_TYPEB_OFF])) ;
//
//		//copy ATQB
//	PX_W_U8(&tempBuffer[ATQB_TLV], 0x82) ;
//	PX_W_U8(&tempBuffer[ATQB_TLV + 1], 0x04) ;
//	PX_W_U8(&tempBuffer[ATQB_TLV + 2], PN_R_U8(&scr[TYPEB_AFI_OFF])) ;
//	PX_W_U8(&tempBuffer[ATQB_TLV + 3], PN_R_U8(&scr[TYPEB_CRC1_OFF])) ;
//	PX_W_U8(&tempBuffer[ATQB_TLV + 4], PN_R_U8(&scr[TYPEB_CRC2_OFF])) ;
//	PX_W_U8(&tempBuffer[ATQB_TLV + 5], PN_R_U8(&scr[TYPEB_APPLICATIONNUMBER_OFF])) ;
//
//        //copy history bytes
//	PX_W_U8(&tempBuffer[TB_HIS_TLV], 0x83) ;
//	PX_W_U8(&tempBuffer[TB_HIS_TLV + 1], 0x0E) ;
//	PX_W_U8(&tempBuffer[TB_HIS_TLV + 2], 0x0D) ;
//	ecpMem_Copy(&tempBuffer[TB_HIS_TLV + 3], &scr[TYPEB_HISTORICAL_BYTES_OFF], 13);
//
//	    //copy DATA_RATE_MAX
//	PX_W_U8(&tempBuffer[TB_DATA_RATE_MAX_TLV], 0x84) ;
//	PX_W_U8(&tempBuffer[TB_DATA_RATE_MAX_TLV + 1], 0x03) ;
//	PX_W_U8(&tempBuffer[TB_DATA_RATE_MAX_TLV + 2], PN_R_U8(&scr[DATA_RATE_MAX_TLV + 2])) ;
//	PX_W_U8(&tempBuffer[TB_DATA_RATE_MAX_TLV + 3], PN_R_U8(&scr[DATA_RATE_MAX_TLV + 3])) ;
//	PX_W_U8(&tempBuffer[TB_DATA_RATE_MAX_TLV + 4], PN_R_U8(&scr[DATA_RATE_MAX_TLV + 4])) ;
#endif
		//copy all data to scr
		memCopyAtomic(dest, tempBuffer, EECFG_TPB_TLVDATA_SIZE);
}




DetectionContextStruct * DetectionContext;
 
void DetectionInit(PM8 Data1, PM8 Mask1, PM8 Data2, PM8 Mask2)
{
	/*@ -discast @*/DetectionContext = (DetectionContextStruct*)bTempArr;/*@ +discast @*/
	
	DetectionContext->ConflictDetected = false;


	DetectionContext->ProtocolData1 = Data1;
	DetectionContext->MandatoryMask1 = Mask1;
	DetectionContext->ProtocolData2 = Data2;
	DetectionContext->MandatoryMask2 = Mask2;
}

static u8 GetMaxMandaBitIdx(PM8 MaskTlv)
{
	u16 i;
	u8 valueLen;

	valueLen = MaskTlv[1];

	if(valueLen < 2)
	{
		return 0;
	}
	

	for(i = U8t16(U8_A_U8(valueLen, 1)); i > 2; U16_DEC(i))
	{
		if(MaskTlv[i] != 0)
			break;
	}

	return U16t8(U16_S_U8(i, 2));
}
void LCflctDetc(u16 TagOff, u8 ProtocolType)
{
	u8 				LOIX;
	u8 				LOIY;
	u8 				LX;
	u8 				LY;
	u8 				MIX;
	u8 				MIY;
	u8 				result;

	LOIX 	= DetectionContext->MandatoryMask1[U16_A_U8(TagOff, 2)];
	LOIY 	= DetectionContext->MandatoryMask2[U16_A_U8(TagOff, 2)];
	LX 		= DetectionContext->ProtocolData1[U16_A_U8(TagOff, 2)];
	LY 		= DetectionContext->ProtocolData2[U16_A_U8(TagOff, 2)];
	MIX	 	= GetMaxMandaBitIdx(MEMtPX8(&DetectionContext->MandatoryMask1[TagOff]));
	MIY 	= GetMaxMandaBitIdx(MEMtPX8(&DetectionContext->MandatoryMask2[TagOff]));

	if(DetectionContext->ConflictDetected)
		return;
	
	DetectionContext->ConflictDetected = true; 

    if((ProtocolType == APDU_PROTOCOL_TB) && (TagOff == 0x80))
	{
		if(LOIX == MAX_OPERATOR)
		{
			LOIX = XOR_OPERATOR ;
		}
		if(LOIY == MAX_OPERATOR)
		{
			LOIY = XOR_OPERATOR ;
		}
	}
	
	if((LOIX == XOR_OPERATOR) || (LOIX == MAX_OPERATOR))
	{
		if(MIY > LX)
	    {
			DetectionContext->ConflictDetected = true ;
			return;
		}
	}

	if((LOIY == XOR_OPERATOR) || (LOIY == MAX_OPERATOR))
	{
		if(MIX > LY)
		{
			DetectionContext->ConflictDetected = true ;
			return;
		}
	}
	
	if(LOIX == LOIY)
	{
		if(LOIX == XOR_OPERATOR) 
		{	
			if(LX != LY)
			{
			  	DetectionContext->ConflictDetected = true ;
				return;
	        }
		}
		else if(LOIX == MAX_OPERATOR)
		{
			if((LX > LY) && (MIX > LY))
			{
			  	DetectionContext->ConflictDetected = true ;
				return;
	        }

			if((LY > LX) && (MIY > LX))
			{
			  	DetectionContext->ConflictDetected = true ;
				return;
	        }	
		}
	}
	else 
	{
		if((LOIX == MAX_OPERATOR) && (LOIY == XOR_OPERATOR))
		{
			if(LX < LY)
			{
			  	DetectionContext->ConflictDetected = true ;
				return;
	        }
		}
		else if((LOIY == MAX_OPERATOR) && (LOIX == XOR_OPERATOR))
		{
			if(LY < LX)
			{
			  	DetectionContext->ConflictDetected = true ;
				return;
	        }
		}
	}
	
	result = 0;

	if((LOIX != MAX_OPERATOR) && (LOIY != MAX_OPERATOR))
	{
		result = U8_XOR_U8(LOIX, LOIY);
	}

	if(U8_AND_U8(U8_AND_U8(result, LOIX), LOIY))
	{
		DetectionContext->ConflictDetected = true ;
		return;
	}

	DetectionContext->ConflictDetected = false; 
}

void BasicCflctDetcXOR(u16 ValueOff, u8 length)
{	
	u16 			i;
	u16 			ValueLimit;

	ValueLimit 		= U16_A_U8(ValueOff, length);
	if(DetectionContext->ConflictDetected)
		return;

	for(i = ValueOff; i < ValueLimit; U16_INC(i))
	{
	 	if(U8_AND_U8(U8_AND_U8(U8_XOR_U8(DetectionContext->ProtocolData1[i], DetectionContext->ProtocolData2[i])
		  , DetectionContext->MandatoryMask1[i]), DetectionContext->MandatoryMask2[i]))
		{
			DetectionContext->ConflictDetected = true;
			return;
		}
	}
}


u8 TypeAConflictDetection(PM8 Data1, PM8 Mask1, PM8 Data2, PM8 Mask2, u8 ProtcolType)
{
	DetectionInit(Data1,Mask1,Data2,Mask2);


	if(DetectionContext->ProtocolData1 == MEM_NULL) 
	{
		PM8 CurrentCLParaBuf = MEMtPM8(GetCurrentCLParaBuf());

	 	DetectionContext->ProtocolData1 = &CurrentCLParaBuf[EECFG_TPA_TLVDATA];
		DetectionContext->MandatoryMask1 = &CurrentCLParaBuf[EECFG_TPA_TLVMASK];
	}
	else if(DetectionContext->ProtocolData2 == MEM_NULL) 
	{
		DetectionContext->ProtocolData2 = DetectionContext->ProtocolData1;
		DetectionContext->MandatoryMask2 = DetectionContext->MandatoryMask1;
	}

	LCflctDetc(UID_TLV , ProtcolType);
	BasicCflctDetcXOR(U8t16(U8_A_U8(UID_TLV, 3)), 10);
	BasicCflctDetcXOR(U8t16(U8_A_U8(SAK_TLV, 2)), 1);
	BasicCflctDetcXOR(U8t16(U8_A_U8(ATQA_TLV, 2)), 2);
	LCflctDetc(ATS_HIS_TLV , ProtcolType);
	BasicCflctDetcXOR(U8t16(U8_A_U8(ATS_HIS_TLV, 3)), 15);
	//BasicCflctDetcMAX(FWI_SFGI_TLV +2, false);
	BasicCflctDetcXOR(U8t16(U8_A_U8(CID_TLV, 2)), 1);
	//BasicCflctDetcMAX(DATA_RATE_MAX_TLV +2, true);
	//BasicCflctDetcMAX(DATA_RATE_MAX_TLV +3, true);
	BasicCflctDetcXOR(U8t16(U8_A_U8(DATA_RATE_MAX_TLV, 4)), 1);

	return DetectionContext->ConflictDetected;
}

u8 TypeBConflictDetection(PM8 Data1, PM8 Mask1, PM8 Data2, PM8 Mask2, u8 ProtcolType)
{
	DetectionInit(Data1,Mask1,Data2,Mask2);


	if(DetectionContext->ProtocolData1 == MEM_NULL) 
	{
		PM8 CurrentCLParaBuf = MEMtPM8(GetCurrentCLParaBuf());

	 	DetectionContext->ProtocolData1 = &CurrentCLParaBuf[EECFG_TPB_TLVDATA];
		DetectionContext->MandatoryMask1 = &CurrentCLParaBuf[EECFG_TPB_TLVMASK];
	}
	else if(DetectionContext->ProtocolData2 == MEM_NULL) 
	{
		DetectionContext->ProtocolData2 = DetectionContext->ProtocolData1;
		DetectionContext->MandatoryMask2 = DetectionContext->MandatoryMask1;
	}

	LCflctDetc(PUPI_TLV , ProtcolType);
	BasicCflctDetcXOR(U8t16(U8_A_U8(PUPI_TLV, 3)), 4);
	BasicCflctDetcXOR(U8t16(U8_A_U8(AFI_TLV, 2)), 1);
	BasicCflctDetcXOR(U8t16(U8_A_U8(ATQB_TLV, 2)), 4);
	LCflctDetc(TB_HIS_TLV , ProtcolType);
	BasicCflctDetcXOR(U8t16(U8_A_U8(TB_HIS_TLV, 3)), 13);
	//BasicCflctDetcMAX(TB_DATA_RATE_MAX_TLV +2, true);
	//BasicCflctDetcMAX(TB_DATA_RATE_MAX_TLV +3, true);
	BasicCflctDetcXOR(U8t16(U8_A_U8(TB_DATA_RATE_MAX_TLV, 4)), 1);

	return DetectionContext->ConflictDetected;
}

static memref GetProtoType(memref ProtoObjAddr)
{
	memref classAddr;

	classAddr = U32_A_U32(getPackageComponentLocation(IMPL_AMDC_PKG_ID, COMPONENT_CLASS), PROTOCOL14443TYPEA_CLASS_OFFSET);

	/*@ -unrecog @*/if(isinstanceof(MEMtPF8(ProtoObjAddr),  AType_Object, MEMtPF8(classAddr)))/*@ +unrecog @*/
		return APDU_PROTOCOL_TA;
	else
		return APDU_PROTOCOL_TB;
}
static memref GetDataFromProtoObj(memref ObjAddr)
{
	u16 ProtocolDataRef;
	memref arrayAddr;

	if(ObjAddr == REF_NULL)
		return 0;
	
	ProtocolDataRef = PX_R_U16((u8 *)get_object_field_ref(ObjAddr, ProtocolData_INDEX));

	//return (u8*)get_array_data((u8*)locateObjectInMemory(ProtocolDataRef));
//	return	PXtMEM(MEMtPX8(get_array_data(MEMtPF8(locateObjectInMemory(ProtocolDataRef)))));
	arrayAddr = get_array_data(MEMtPF8(locateObjectInMemory(ProtocolDataRef)));
	return arrayAddr;
}
static memref GetMaskFromProtoObj(memref ObjAddr)
{
	u16 ProtocolDataRef;
	memref arrayAddr;

	if(ObjAddr == REF_NULL)
		return 0;
	
	ProtocolDataRef = PX_R_U16((u8 *)get_object_field_ref(ObjAddr, MandatoryMask_INDEX));

	//return (u8*)get_array_data((u8*)locateObjectInMemory(ProtocolDataRef));
//	return PXtMEM(MEMtPX8(get_array_data(MEMtPF8(locateObjectInMemory(ProtocolDataRef)))));
	arrayAddr = get_array_data(MEMtPF8(locateObjectInMemory(ProtocolDataRef)));
	return arrayAddr;
}

/*void ChDefaultProtocolValueNM()
{	
	memref ProtoParaObject2 = locateObjectInMemory(popReference()) ;   //default Protocol Object
	memref ProtoParaObject1 = locateObjectInMemory(popReference()) ;   //App Protocol Object
	u8 * AppData, * AppMask, * DefaultData ;
	
	AppData = GetDataFromProtoObj(ProtoParaObject1);
	AppMask = GetMaskFromProtoObj(ProtoParaObject1);
	DefaultData = GetDataFromProtoObj(ProtoParaObject2);
	
	if(TypeAConflictDetection(NULL, NULL, AppData, AppMask, APDU_PROTOCOL_TA))
	{
		throw_error(SW_WRONG_DATA);
	}

}	*/
 
void GetProtocolParaNM()
{
	u8 						IsDefaultOrCurrent;
	u8 						getProtoType;
	jshort 					offset ;
	memref	 				ProtoParaObject;
	memref 					ArrayObj;

	ProtoParaObject 		= PNtMEM(getJavaObjectDataRef());
	IsDefaultOrCurrent 		= U16t8(popShort()) ;
	offset 					= U16tS16(popShort());
	ArrayObj 				= PNtMEM(getJavaObjectDataRef());
	getProtoType 			= U32t8(GetProtoType(ProtoParaObject));
	
	InitComputContext();
	
	if(IsDefaultOrCurrent) 
	{
		GetDefaultProtocolParaNM(ProtoParaObject) ;
	}else{
		GetCurrentProtocolParaNM(getProtoType) ;
	}

	//pushShort( (jshort)GetProtocolPara(ArrayData, getProtoType) );
	/*@ -typedefparam @*/
	/*@ -type @*/
	pushShort( U16tS16(GetProtocolPara(ArrayObj, U16t8(S16tU16(offset)), getProtoType, IsDefaultOrCurrent)) );
	/*@ +type @*/
	/*@ +typedefparam @*/
}

void GetCurrentProtocolParaNM(u8 ProtoType)
{
	PM8 CurrentCLParaBuf = MEMtPM8(GetCurrentCLParaBuf()) ;
	if(ProtoType == APDU_PROTOCOL_TA)
	{
		ComputContext->ProtocolData = &CurrentCLParaBuf[EECFG_TPA_TLVDATA];
		ComputContext->MandatoryMask = &CurrentCLParaBuf[EECFG_TPA_TLVMASK];
	}else{
		ComputContext->ProtocolData = &CurrentCLParaBuf[EECFG_TPB_TLVDATA];
		ComputContext->MandatoryMask = &CurrentCLParaBuf[EECFG_TPB_TLVMASK];
	}
	
}

void GetDefaultProtocolParaNM(memref ProtoParaObject)
{	
	ComputContext->ProtocolData = MEMtPM8(GetDataFromProtoObj(ProtoParaObject));
	ComputContext->MandatoryMask = MEMtPM8(GetMaskFromProtoObj(ProtoParaObject));
}


void StoreProtocolParaNM()
{
	memref 					ProtoParaObject;
	jshort 					length ;
	memref 					ArrayData;

	ProtoParaObject 		= PNtMEM(getJavaObjectDataRef());
	length 					= U16tS16(popShort());
	/*@ -disspecopt @*/ArrayData 				= PNtMEM(getArrayDataWithoutCheck());/*@ +disspecopt @*/

	InitComputContext();

	ComputContext->ProtocolData = MEMtPM8(GetDataFromProtoObj(ProtoParaObject));
	ComputContext->MandatoryMask = MEMtPM8(GetMaskFromProtoObj(ProtoParaObject));
	
	if(GetProtoType(ProtoParaObject) == APDU_PROTOCOL_TA)
		StoreProtocolTypeAPara(MEMtPX8(ArrayData), S16tU16(length));
	else
		StoreProtocolTypeBPara(MEMtPX8(ArrayData), S16tU16(length));

}

void conflictDetectionNM()
{
	u8 					ProtoType1;
	u8 					ProtoType2;
	PM8 				Data1;
	PM8 				Mask1;
	PM8 				Data2;
	PM8 				Mask2;
	memref 				ProtoParaObject2;
	memref 				ProtoParaObject1;


	ProtoType1 			= 0;
	ProtoType2 			= 0;

	Data1 				= MEM_NULL;
	Mask1 				= MEM_NULL;
	Data2 				= MEM_NULL;
	Mask2 				= MEM_NULL;
	ProtoParaObject2 	= PNtMEM(getJavaObjectDataRef());
	ProtoParaObject1 	= PNtMEM(getJavaObjectDataRef());

	if(ProtoParaObject1 != MEM_NULL)
	{
		Data1 = MEMtPM8(GetDataFromProtoObj(ProtoParaObject1));
		Mask1 = MEMtPM8(GetMaskFromProtoObj(ProtoParaObject1));
		ProtoType1 = U32t8(GetProtoType(ProtoParaObject1));
	}

	if(ProtoParaObject2 != MEM_NULL)
	{
		Data2 = MEMtPM8(GetDataFromProtoObj(ProtoParaObject2));
		Mask2 = MEMtPM8(GetMaskFromProtoObj(ProtoParaObject2));
		ProtoType2 = U32t8(GetProtoType(ProtoParaObject2));
	}

	if(ProtoType1 == 0)
		ProtoType1 = ProtoType2;

	if(ProtoType1 == APDU_PROTOCOL_TA)
	{
		pushShort(U8t16(TypeAConflictDetection(Data1,Mask1,Data2,Mask2,ProtoType1)));
	}
	else
	{
		pushShort(U8t16(TypeBConflictDetection(Data1,Mask1,Data2,Mask2,ProtoType1)));
	}
}

void ComputeCurrentProtocolParameterNM()
{
	memref ProtoParaObject;

	ProtoParaObject 			 = PNtMEM(getJavaObjectDataRef());
	InitComputContext();
	
	ComputContext->ProtocolData  = MEMtPM8(GetDataFromProtoObj(ProtoParaObject));
	ComputContext->MandatoryMask = MEMtPM8(GetMaskFromProtoObj(ProtoParaObject));

	if(GetProtoType(ProtoParaObject) == APDU_PROTOCOL_TA)
		TypeAComputeCurrentProtocolParameter();
	else
		TypeBComputeCurrentProtocolParameter();
}

void SetCurrentProtocolDataNM()
{
	PM8 				CurrentCLParaBuf;
	u8 					ProtocolType;
	memref 				ProtoParaObject;
	
	ProtoParaObject 	= PNtMEM(getJavaObjectDataRef());
	CurrentCLParaBuf 	= MEMtPM8(GetCurrentCLParaBuf());
	ProtocolType 		= 0 ;

	InitComputContext();
	ProtocolType = U32t8(GetProtoType(ProtoParaObject));
	ComputContext->ProtocolData = MEMtPM8(GetDataFromProtoObj(ProtoParaObject));
	ComputContext->MandatoryMask = MEMtPM8(GetMaskFromProtoObj(ProtoParaObject));
	
	if(ProtocolType == APDU_PROTOCOL_TA)
	{
		memCopyAtomic (&CurrentCLParaBuf[EECFG_TPA_TLVDATA], ComputContext->ProtocolData, 49);
	    memCopyAtomic (&CurrentCLParaBuf[EECFG_TPA_TLVMASK], ComputContext->MandatoryMask, 49);
		TypeAProtoDataTLVToCompact(ComputContext->ProtocolData, MEMtPM8(JcvmConfig_GetVmConfigAddr(SYSCONFIG_OFFSET_START)));
	}
	else
	{
		memCopyAtomic (&CurrentCLParaBuf[EECFG_TPB_TLVDATA], ComputContext->ProtocolData, 49);
	    memCopyAtomic (&CurrentCLParaBuf[EECFG_TPB_TLVMASK], ComputContext->MandatoryMask, 49);
		TypeBProtoDataTLVToCompact(ComputContext->ProtocolData, MEMtPM8(JcvmConfig_GetVmConfigAddr(SYSCONFIG_OFFSET_START)));
	}
	memRefresh();
}

void ProtocolParameterBootstrapNM()
{
	u8 ProtoType;

	memref ProtoParaObject;

	ProtoParaObject = PNtMEM(getJavaObjectDataRef());
	InitComputContext();

	ProtoType = U32t8(GetProtoType(ProtoParaObject));
	ComputContext->ProtocolData = MEMtPM8(GetDataFromProtoObj(ProtoParaObject));
	ComputContext->MandatoryMask = MEMtPM8(GetMaskFromProtoObj(ProtoParaObject));

	ComputStart(ProtoType);

	bEECFG	=	MEMtPN8(GetCurrentCLParaBuf());

	if(ProtoType == APDU_PROTOCOL_TA)
		InitializeProtocolTypeAPara();
	else
		InitializeProtocolTypeBPara();
	
}
#if INFINEON_FSR8
void ModifyCurrentSAKValue(u32 protocolParaAddress, u8 sak)
{
    memCopyAtomic((protocolParaAddress+ EECFG_TYPEA_SAK), &sak, 1);
	memCopyAtomic((protocolParaAddress + EECFG_TPA_TLVDATA + SAK_TLV + 2), &sak, 1);		
}
#endif

//#if JCVM__IO__SWP
//void syncPrivateCmdChangeSWPParameter()
//{
//	PN8 CurrentSWPParaBuf = MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_START));
//	PN8 CurrentCLParaBuf = MEMtPN8(GetCurrentCLParaBuf());
//	PN8 PrepareCLParaBuf = MEMtPN8(GetPrepareCLParaBuf());
//	
//	if (sysConfig_GetContactlessParaSwitch())
//	{
//		InitComputContext();
//		
//		ComputContext->CurrentProtocolData = &CurrentCLParaBuf[EECFG_TPA_TLVDATA];
//		TypeAProtoDataCompactToTLV(CurrentSWPParaBuf, ComputContext->CurrentProtocolData);

//		ComputContext->CurrentProtocolData = &CurrentCLParaBuf[EECFG_TPB_TLVDATA];
//		TypeBProtoDataCompactToTLV(CurrentSWPParaBuf, ComputContext->CurrentProtocolData);

//		memCopyAtomic(PrepareCLParaBuf, CurrentCLParaBuf, EECFG_SIZE);
//		
//	}

//	swp_ResetSwpConfigData();
//}
//#endif

void clearProtocolData()
{
#if JCVM__IO__SWP
	u8  tmpBuf[EECFG_TPA_TLVDATA_SIZE + EECFG_TPA_TLVMASK_SIZE + EECFG_TPB_TLVDATA_SIZE + EECFG_TPB_TLVMASK_SIZE];
	u32 flag = JcvmConfig_GetVmConfigAddr(AMDC_EFCFG_FLAG);
	
	nvmWriteU8NoAtomic(MEMtPM8(flag), 1);
	
	ecpMem_Copy(tmpBuf, MEMtPM8(&bEECFG1[EECFG_TPA_TLVDATA]), EECFG_TPA_TLVDATA_SIZE + EECFG_TPA_TLVMASK_SIZE + EECFG_TPB_TLVDATA_SIZE + EECFG_TPB_TLVMASK_SIZE);
	
	ecpMem_Set(&tmpBuf[EECFG_TPA_TLVDATA], 0x00, EECFG_TPA_TLVDATA_SIZE);
	
	ecpMem_Set(&tmpBuf[EECFG_TPB_TLVDATA], 0x00, EECFG_TPB_TLVDATA_SIZE);
	
	ecpMem_Set(&tmpBuf[EECFG_TPA_TLVMASK + UID_TLV + 1], 0x00, 12);
	
	tmpBuf[EECFG_TPA_TLVMASK + SAK_TLV + 2] = 0;
	
	tmpBuf[EECFG_TPA_TLVMASK + ATQA_TLV + 2] = 0;
	tmpBuf[EECFG_TPA_TLVMASK + ATQA_TLV + 3] = 0;
	
	ecpMem_Set(&tmpBuf[EECFG_TPA_TLVMASK + ATS_HIS_TLV + 1], 0x00, 17);
	
	tmpBuf[EECFG_TPA_TLVMASK + FWI_SFGI_TLV + 2] = 0x00;
	
	tmpBuf[EECFG_TPA_TLVMASK + CID_TLV + 2] = 0x00;
	
	ecpMem_Set(&tmpBuf[EECFG_TPA_TLVMASK + DATA_RATE_MAX_TLV + 2], 0x00, 3);
	
	ecpMem_Set(&tmpBuf[EECFG_TPB_TLVMASK + PUPI_TLV + 1], 0x00, 6);
	
	tmpBuf[EECFG_TPB_TLVMASK + AFI_TLV + 2] = 0x00;
	
	ecpMem_Set(&tmpBuf[EECFG_TPB_TLVMASK + ATQB_TLV + 2], 0x00, 4);
	
	ecpMem_Set(&tmpBuf[EECFG_TPB_TLVMASK + TB_HIS_TLV + 1], 0x00, 17);
	
	ecpMem_Set(&tmpBuf[EECFG_TPB_TLVMASK + TB_DATA_RATE_MAX_TLV + 2], 0x00, 3);
	
	memCopyNoAtomic((u8*)&bEECFG1[0], tmpBuf, EECFG_TPA_TLVDATA_SIZE + EECFG_TPA_TLVMASK_SIZE + EECFG_TPB_TLVDATA_SIZE + EECFG_TPB_TLVMASK_SIZE);
	
	fillEeprom((u8*)&bEECFG2[EECFG_TPA_TLVDATA], 0x00, EECFG_TPA_TLVDATA_SIZE + EECFG_TPA_TLVMASK_SIZE + EECFG_TPB_TLVDATA_SIZE + EECFG_TPB_TLVMASK_SIZE);

#endif
}

#endif
