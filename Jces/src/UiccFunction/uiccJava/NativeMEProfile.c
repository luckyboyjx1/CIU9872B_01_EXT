#include "typedef.h"
#include "fileSystem.h"
//#include "eepromDefine.h"
#include "uicc_main.h"
#include "SimApi.h"
#include "ToolkitAPIPublic.h"
//#include "transaction.h"
//#include "Interpreter.h"
#include "platform_api.h"
#if UICC__FUNCTION__SIMAPI
#include "VMAccessEeprom.h"		//remove warning,dengyiwen,2022/9/27
/*
*@description:  Checks a facility in the handset profile
*
*@函数原型   :public static boolean check(byte index) throws ToolkitException
*
*@param     :byte index
*
*@return    :true if the facility is supported, false if facility is not supported, or if facility-index outside MEProfile data 
*
*@exception :
			ToolkitException - with the following reason codes: 
				ME_PROFILE_NOT_AVAILABLE if Terminal Profile data are not available 
				BAD_INPUT_PARAMETER if index has a negative value
*
*@author   :create by xuyz
*
*@hisoty   :V1.0: create by xuyz
*/
void Toolkit_profileCheckByte1(void)
{
	BYTE idata value;
	jbyte idata index;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	index = PopOperandStackShort();
	
	value = Toolkit_publicProfileCheck1(index,apiMode);

	PushOperandStackShort((jbyte)value);
}
/*
*@description: Checks a set of facilities in the handset profile. The method checks all the facilities corresponding to bits set to 1 in the mask buffer
*
*@函数原型   ：public static boolean check(byte[] mask,
                            short offset,
                            short length)
                     throws java.lang.NullPointerException,
                            java.lang.ArrayIndexOutOfBoundsException,
                            ToolkitException
*@param      mask - a byte array containing the mask to compare with the profile
*@param      offset - the starting offset of the mask in the byte array
@param       length - the length of the mask (at least 1) 
*
*Return      :true if the the bitwise AND of the MEProfile data padded with 0 and the mask is equal to the mask , false otherwise. If length is equal to 0, true is returned. 
*
*@Throws: 
			java.lang.NullPointerException - if mask is null 
			java.lang.ArrayIndexOutOfBoundsException - if check would cause access of data outside mask array bounds 
			ToolkitException - with the following reason codes: 
					ME_PROFILE_NOT_AVAILABLE if Terminal Profile data are not available
*
*@author   :create by xuyz
*
*@hisoty   :V1.0: create by xuyz
*/
void Toolkit_profileCheckPara3(void)
{
	BYTE idata i ,tmp,length;
	UWORD xdata maskAddr;
	
	jshort idata apiMode = PopOperandStackShort();
	jshort idata maskLength = PopOperandStackShort();
    
	jshort idata offset = PopOperandStackShort();
    
	jref idata mask = PopOperandStackShort();

	maskAddr = GetValidArrayDataAddr(mask,offset,maskLength);//check_bounds_interval_buffer(mask, offset, maskLength);	  //异常：ArrayIndexOutOfBoundsException

	if(maskLength == 0x00)
	{
		PushOperandStackShort(TRUE);
		return;
	}

	check_profile_available(apiMode);

	length = (G_TerminalProfile[0] < maskLength)? G_TerminalProfile[0] : maskLength;
	
	for(i=0; i<length; i++)
	{
		tmp = VMReadByte(maskAddr+i);//CopyArrayDataToDest(&tmp,mask,offset+i,1);

		if ((G_TerminalProfile[(i + 1)]&tmp) != tmp)
		{
			PushOperandStackShort(FALSE);
			return;
		}
	}

	for (i = G_TerminalProfile[0]; i < maskLength; i++) 
	{
		tmp = VMReadByte(maskAddr+i);//CopyArrayDataToDest(&tmp,mask,offset+i,1);

		if (tmp != 0) //检查maskByte[]比MEProfile[]多余的字节
		{
			PushOperandStackShort(FALSE);
			return;
		}
	}

	PushOperandStackShort(TRUE);
}
/*
*@description:  Checks a facility in the handset profile
*
*@函数原型   : public static boolean check(short index) throws ToolkitException
*
*@param       :index - the number of the facility to check, according to the table above.
*
*@return       :true if the facility is supported, false if facility is not supported, or if facility-index outside MEProfile data 
*
*@Throws: 
				ToolkitException - with the following reason codes: 
						ME_PROFILE_NOT_AVAILABLE if Terminal Profile data are not available 
						BAD_INPUT_PARAMETER if index has a negative value
*
*@author   :create by xuyz
*
*@hisoty   :V1.0: create by xuyz
*/
void Toolkit_profileCheckShort1(void)
{
	BYTE idata value;	
	jshort idata  index;
	jshort idata apiMode;
		
	apiMode = PopOperandStackShort();
	index = PopOperandStackShort();
	
	value = Toolkit_publicProfileCheck1(index, apiMode);

	PushOperandStackShort((jbyte)value);
}

/*
*@description:  Returns the binary value of a parameter, delimited by two indexes, from the handset profile
*
*@函数原型   ：public static short getValue(short indexMSB,short indexLSB)throws ToolkitException
*
*@Parameters:
*		indexMSB - index of the Most Significant Bit of the handset profile .
*		indexLSB - index of the Lowest Significant Bit of the handset profile . 
*@Returns:
		binary value of the data field indicated in the handset profile. The indexLSB bit in the MEProfile data is the Lowest Significant bit in the short returned value. If padding is necessary, the returned value is left padded with 0. The values outside the MEProfile data available are considered to bet set to 0. The return value is according to the following example:
		If indexMSB=108 and indexLSB=104, the return value is the number of characters down ME display. 
		If indexMSB=31 and indexLSB=16, the return value is a short built from the 4th and 3rd byte of the handset profile with the 4th byte as the Most significant byte.
*@Throws: 
		ToolkitException - with the following reason codes: 
		ME_PROFILE_NOT_AVAILABLE if Terminal Profile data are not available 
		BAD_INPUT_PARAMETER if (indexMSB >= indexLSB +16) or (indexMSB < indexLSB) or (indexMSB < 0) or (indexLSB < 0) 
*
*@author   :create by xuyz
*
*@hisoty   :V1.0: create by xuyz
*/
void Toolkit_profileGetValue(void)
{
	jshort idata   value;
	jshort idata  i,tmp;
	
	jshort idata indexMSB; 
	jshort idata indexLSB;
	jshort idata apiMode;
		
	apiMode = PopOperandStackShort();
	indexLSB = PopOperandStackShort();
	indexMSB = PopOperandStackShort();

	check_profile_available(apiMode);

	if((indexMSB >= (indexLSB+16))||(indexMSB < indexLSB)||(indexMSB < 0)||(indexLSB < 0))
	{
		UiccOrSimThrowToolkitException(BAD_INPUT_PARAMETER_SIM, apiMode);
		
		return;
	}

	value = 0;

	for(i = indexMSB;i >= indexLSB; i--)
	{
		if(Toolkit_publicProfileCheck1(i, apiMode))
		{
			tmp = 1;
		}
		else
		{
			tmp = 0;
		}

		value = (value << 1) | tmp;
	}

	PushOperandStackShort(value);
}
/*
*@description: Copies a part of the handset profile in a buffer. The values outside the MEProfile data available are considered to bet set to 0. 
*
*@函数原型   :public static short copy(short startOffset, byte[] dstBuffer,short dstOffset, short dstLength)throws java.lang.ArrayIndexOutOfBoundsException,java.lang.NullPointerException,ToolkitExceptionCopies
*
*@Parameters:
			startOffset - offset of the handset profile first byte to be copied
			dstBuffer - destination byte array
			dstOffset - offset within destination byte array to start copy into
			dstLength - byte length to be copy 
*
*@Returns:   dstOffset + dstLength 
*
*
*@Throws: 
		java.lang.ArrayIndexOutOfBoundsException - if copy would cause access of data outside array bounds 
		java.lang.NullPointerException - if dstBuffer is null 
		ToolkitException - with the following reason codes: 
				ME_PROFILE_NOT_AVAILABLE if Terminal Profile data are not available 
				BAD_INPUT_PARAMETER if the startOffset is negative
*
*@author   :create by xuyz
*
*@hisoty   :V1.0: create by xuyz
*/
void Toolkit_profileCopy(void)
{
	jshort idata copyLength;

	UWORD xdata dstBuffserData ;
	
	jshort idata startOffset; 
	jref   idata arrayref; 
	jshort idata dstOffset; 
	jshort idata dstLength;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	dstLength = PopOperandStackShort();
	dstOffset = PopOperandStackShort();
	arrayref = PopOperandStackShort();
	startOffset  = PopOperandStackShort();

	dstBuffserData	= GetValidArrayDataAddr(arrayref,dstOffset,dstLength);//GetArrayBufferAddress(arrayref)+ dstOffset; 

	check_profile_available(apiMode);
	
	if(startOffset < 0)
	{
		UiccOrSimThrowToolkitException(BAD_INPUT_PARAMETER_SIM, apiMode);
		return;
	}
	
	if((startOffset+dstLength) > G_TerminalProfile[0] )
	{
		copyLength = G_TerminalProfile[0]  - startOffset;

		VMWriteBuf(dstBuffserData,&G_TerminalProfile[1+startOffset],copyLength); //dstBuffserData 是API传入进来，可能为RAM，可能为E2

		VMSetBuf(dstBuffserData+copyLength,0x00,dstLength-copyLength);
	}
	else
	{
		VMWriteBuf(dstBuffserData,&G_TerminalProfile[1+startOffset],dstLength);
	}

	PushOperandStackShort((jshort)(dstOffset+dstLength));
}
#endif
