#include    "uicc_main.h"
#include    "uicc_interface.h"
#include    "fileAction.h"
#include    "gsmpublic.h"
#include    "hard_api.h"
#include    "UICCToolkitAPI.h"
#include    "UICCToolkitConstants.h"
#include    "CSIMApiImpl.h"
#include    "ToolkitAPIPublic.h"
#include "platform_api.h"

#include "Interpreter.h"
#include "VMAccessEeprom.h"

#if UICC__FUNCTION__CSIMAPI
/*
*@description: 查找 EnvelopeHandlerBuf中的UserData长度/偏移，SecuredData长度/偏移
*
*@param     : mode: MODE_SECURE_LENGTH/MODE_SECURE_OFFSET/MODE_MESSAGE_LENGTH/MODE_MESSAGE_OFFSET
*
*@return    :secureDataOffset/secureDataLen/userDataLen/userDataOffset
*
*@author    :create by lgy
*
*@history   :V1.0 :Create by lgy@20180427
*/
WORD CSIM_GetSecureDataLenOffset(BYTE mode)
{
	WORD idata offset,secureDataLen,UDL;
	BYTE idata tempLen,chl,pntr;
    WORD idata CurrentTLVLength,CurrentTLVValueOffset;
    UWORD xdata dstBuffer;
    
    if(G_EnvelopeTag == 0xD1)	 //SMS TPDU
	{
       
        ToolkitFindTLVPublic(Index_EnvelopeHandler,TAG_CSIM_SMS_TPDU,1,UICCAPI_Mode);   //找CSIM 短信TPDU  C8 tag 
        
        if(ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset == 0xFFFF)//UNAVAILABLE_ELEMENT  in case of unavailable TPDU TLV element or wrong data format
        {
            UICCThrowToolkitException(UNAVAILABLE_ELEMENT); 
        }
       
        offset = 0; // point to Message Type
        
        tempLen = Toolkit_getValueBytePublic(Index_EnvelopeHandler,2,UICCAPI_Mode);//Teleservice Identifier len 
        
        offset = 2 + 1 + tempLen + 1;// point to Originating Address len;
        
        tempLen = Toolkit_getValueBytePublic(Index_EnvelopeHandler,offset,UICCAPI_Mode);//Originating Address len
        
        offset = offset + 1 + tempLen + 1 + 1 +1 ;// point to Message Identifier  len   (Originating Address len (1byte) + len + Bearer Data tag (1byte) + len(1byte) + Message Identifier tag(1byte))
        
		tempLen = Toolkit_getValueBytePublic(Index_EnvelopeHandler,offset,UICCAPI_Mode);// Message Identifier  len
        
        offset = offset + 1 + tempLen + 3 + 1;//point to User Data Len  (Message Identifier len(1byte) + len +  Message Display Mode(3byte) + User Data tag(1byte) 
        
        dstBuffer= (UWORD)(ToolkitHandlerList[Index_EnvelopeHandler].HandlerBuffer);

        ReadEeprom(&tempBuf[0],dstBuffer+offset + ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset,4);	// offset 该偏移是相对于ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset的偏移 
    
        if(tempBuf[0] == 0x81)
        {
            UDL = 2; 
            CurrentTLVLength = tempBuf[1];
        }
        else if(tempBuf[0] == 0x82)
        {
            UDL = 3;
            CurrentTLVLength = ((tempBuf[1]<<8)|tempBuf[2]);
        }
        else
        {
            UDL = 1;
            CurrentTLVLength = tempBuf[0];
        }
        
        offset = offset + UDL;      //point to user data      , User Data len(x byte))	
    
        CurrentTLVValueOffset = ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset + offset;
        
        ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVLength =  CurrentTLVLength  ;
        ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset = CurrentTLVValueOffset;
        
        if(mode == MODE_MESSAGE_OFFSET)
        {
            return CurrentTLVValueOffset;
        }
        else if(mode == MODE_MESSAGE_LENGTH)
        {
            return CurrentTLVLength;
        }
        
        if((G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] == EVENT_UNFORMATTED_SMS_PP_ENV)||(G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] == EVENT_UNFORMATTED_SMS_PP_UPD_bit))
		{

//        	if(getCurrentTrigeMode() == UICCAPI_Mode)
//			{	
//#if UICC__FUNCTION__UICCAPI
				UICCThrowToolkitException(UNAVAILABLE_ELEMENT); 
//#endif
//			}
//			else
//			{
//#if UICC__FUNCTION__SIMAPI
//				ThrowToolkitException(UNAVAILABLE_ELEMENT_SIM);	
//#endif	
//			}
		
		}	
        offset = 0;// 相对于ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset的偏移    指向UDL后面的字节
        
        offset = offset + 3 + 1; //跳过 MSG_ENCODING(1byte) + 02 7000 指向CPL
        
		offset += 2;                                 //跳过CPL,指向CHL
	
		chl = Toolkit_getValueBytePublic(Index_EnvelopeHandler,offset,UICCAPI_Mode);
	
		pntr = Toolkit_getValueBytePublic(Index_EnvelopeHandler,offset+13,UICCAPI_Mode);
	
		offset += (1+chl);                     //跳过chl,ch,指向secure data
        
        secureDataLen = (ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVLength)-offset-pntr; 

    //CSIM API 测试Test_Api_Ceh_Gsdl  80C2000040 D13E82028381C8380000021002020300C6A8082C00031000080F01E00122010013800000D8A8100000A9000808000000000807E809BEA8A36AACA880889098A0
    //调用 getSecuredDataLength()后 调用UICCToolkit_getValueByte(2) 期望返回70   即当前TLV指向UDL后面

         
		if(mode == MODE_SECURE_OFFSET)
		{
            return offset;
		}
        else
        {
           	return secureDataLen; 
        }

	}
    else
    {
         UICCThrowToolkitException(UNAVAILABLE_ELEMENT); 
    }
    
    return 0xFFFF;		
}
/**
 *  Looks for the length of the Secured Data from the user data field in the first SMS TPDU
 *  String TLV contained in the Envelope handler. <br>
 *  <br>
 *  This can be used on the events:<ul>
 *  <li>EVENT_FORMATTED_SMS_PP_ENV, if the SMS TP-UD is formatted according to 
 *  Single or Concatenated Short Message.
 *  If the element is available it becomes the TLV selected.</ul>
 *
 *  @return the length of the Secured Data contained in the first SMS TPDU String element (without padding bytes).
 *         If the Secured Data length is zero, no exception shall be thrown.
 *
 *  @exception ToolkitException with the following reason codes: <ul>
 *        <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable SMS TPDU element or wrong data format </ul>
 */
void CSIMGetSecuredDataLength()
{
    PushOperandStackShort(CSIM_GetSecureDataLenOffset(MODE_SECURE_LENGTH));
}
/**
 *  Looks for the Secured Data from the user data field in the first SMS TPDU
 *  String TLV contained in the Envelope handler.<br>
 *  <br>
 *  This can be used on the events:<ul>
 *  <li>EVENT_FORMATTED_SMS_PP_ENV, EVENT_FORMATTED_SMS_PP_UPD, according to Single or Concatenated Short Message.
 *  If the element is available it becomes the TLV selected.</ul>
 *  <br>
 *  @return the offset of the Secured Data first byte in the first SMS TPDU String TLV element.
 *          If the Secured Data length is zero the value returned shall be the offset of the first byte following
 *          the Command Packet structure.
 *
 *  @exception ToolkitException with the following reason codes: <ul>
 *       <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable SMS TPDU TLV element or wrong data format </ul>
 */
void CSIMGetSecuredDataOffset(void)
{
    PushOperandStackShort(CSIM_GetSecureDataLenOffset(MODE_SECURE_OFFSET));
}

/**
 * Looks for the TP-User Data field in the first TPDU TLV element contained in the Envelope Handler. <br>
 * <br>
 * This method can be used on the events:<ul>
 * <li>EVENT_FORMATTED_SMS_PP_ENV, EVENT_UNFORMATTED_SMS_PP_ENV.
 * If the element is available it becomes the TLV selected.</ul>
 * <br>
 * @return the length of the User Data contained in the first SMS TPDU TLV element.
 *
 * @exception ToolkitException with the following reason codes: <ul>
 *      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable TPDU TLV element or wrong data format </ul>
 */
void CSIMGetUserDataLength(void)
{
    PushOperandStackShort(CSIM_GetSecureDataLenOffset(MODE_MESSAGE_LENGTH));
}
 /**
 * Looks for the indicated tag element from the beginning of
 * the TLV list in BearerData(BearerData of handler buffer). If the method is successful then the
 * TLV becomes current, else no TLV is selected.
 * This search method is Comprehension Required flag independent.
 *
 * @param tag the tag of the TLV element to search
 * @return result of the method: <ul>
 *      <li><code>TLV_NOT_FOUND</code> if the required tag element does not exist.
 *      <li><code>TLV_FOUND_CR_SET</code> if the required tag is set.
 *      <li><code>TLV_FOUND_CR_NOT_SET</code> if the required tag is not set.</ul>
 * @exception  ToolkitException with the following reason codes: <ul>
 *      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy
 *      <li><code>BAD_INPUT_PARAMETER</code> if an input parameter is not valid (e.g. tag = -1)</ul>
 */
void CSIMFindTLVUnderBearerData(void)
{
    SBYTE idata tag = PopOperandStackShort();
    WORD idata offset,tagLength;
	BYTE idata tempLen;
    UWORD xdata dstBuffer;
    BYTE idata tmp[4];

    
    
    if(G_EnvelopeTag == 0xD1)	 //SMS TPDU
    {
        ToolkitFindTLVPublic(Index_EnvelopeHandler,TAG_CSIM_SMS_TPDU,1,UICCAPI_Mode);   //找CSIM 短信TPDU  C8 tag 
        
        offset = 0; // point to Message Type
        
        tempLen = Toolkit_getValueBytePublic(Index_EnvelopeHandler,2,UICCAPI_Mode);//Teleservice Identifier len 
        
        offset = 2 + 1 + tempLen + 1;// point to Originating Address len;
        
        tempLen = Toolkit_getValueBytePublic(Index_EnvelopeHandler,offset,UICCAPI_Mode);//Originating Address len
        
        offset = offset + 1 + tempLen + 1 + 1 ;// point to Bearer Data
       
        offset += ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset;// offset是相对于 CurrentTLVValueOffset的偏移
           
        ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset = 0xFFFF;    //初始化偏移

        dstBuffer = (UWORD)(ToolkitHandlerList[Index_EnvelopeHandler].HandlerBuffer);
     
        for(;offset<ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength;)
        {
            ReadEeprom((BYTE *)&tmp[0],dstBuffer+offset,4);
            
            if(tmp[1] == 0x81)  //
            {			
                tagLength = tmp[2];			
                offset += 3;
            }
            else if(tmp[1] == 0x82)
            {							   
                tagLength = (((WORD)tmp[2] << 8)&0xFF00) + tmp[3];
                offset += 4;
            }
            else    
            {
                offset +=  2;
                tagLength = tmp[1];  
            }
            
            if((tmp[0]&TAG_SET_NO_CR) == (tag&TAG_SET_NO_CR))	    
            {
               
                ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVLength =  tagLength;
                ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset = offset;
    
                if((tmp[0]&TAG_SET_CR) == 0x00)
                {
                    PushOperandStackShort(TLV_FOUND_CR_NOT_SET);
                    return ;
                }
                else
                {
                    PushOperandStackShort(TLV_FOUND_CR_SET);
                    return ;
                }              
              													     
            }          
            
            offset += tagLength ;	
            
        }
        
    }
    
    ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset = 0xFFFF;

    PushOperandStackShort(TLV_NOT_FOUND);
    
	return ;	
    
}
        
#endif
