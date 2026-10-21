/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件

 *			V1.1 修改函数cephOpenChannelSingle,修改拷贝AccessName的函数由memcpy改为memcpy_G
 			V1.2 修改cephSendShortMessageSingle，扩展commQualify的最高bit，若commQualify最高bit为1，UDHI为1，否则为0
 			V1.3 修改函数cephLaunchBrowser()，修正错误 --chenl @20111125
 			V1.4 修改函数cephReceiveDataSingle，该函数直接返回主动式命令的返回值@20120213
			V1.5 修改函数cephLaunchBrowser()，去掉了参数dcsUrl，urlString只能输入ASCII字符  --chenl @20120221
			V1.6 增加函数cephLaunchBrowser_JSAPP(),支持江苏移动应用下载功能  --yangjian @20171129
			V1.7 增加函数cephReceiveDataSingleCATTP			                 --yangjian @20180523
 */
#include <string.h>
#include "uicc_main.h"
#include "typedef.h"
#include "uicc_error.h"
#include "stkhead.h"
#include "string.h"
#include "ceph.h"
#include "gsmpublic.h"
#ifdef OTA
#include "ota3head.h"
#endif
#ifdef CDMA
#include 	"cdma.h"
#endif
#ifdef CATTP
#include "CATTP.h"
#endif

#define     STKCommandLen		(ResData[2])

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephDisplayTextSingle

  input parameter:   comamndQulify

					 dcs:UCS2/GSMDEFAULT/GSMCOMPRESSED

					 textString(LV,Variable Reference)

  output parameter:  Sucess/ExitStkPerform/BackMove

  description:       Display the textSring and return the genral result

  code status 		 OK

  test status:		 OK

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephDisplayTextSingle(BYTE comamndQulify,BYTE dcs, BYTE * textString)
{
	BYTE idata result;

	//init the command details and device iden in command of select item
	InitProactiveCommand(CMD_DISPLAY,comamndQulify,DEV_DISPLAY);

	RecData[0] = 0x8D;

	if(dcs == UCS2)
	{
		RecData[1] = 0x01;

		RecData[2] = 0x08;
	}
	else
	{
		RecData[1] = 0x01;

		RecData[2] = dcs;
	}
	
	if(textString != NULL)
	{
		if(textString[0] > 240)
		{
			textString[0] = 240;
		}

	    //change varible to temp buffer
		AddValue((BYTE *)&RecData[1], (BYTE  *)&textString[0]);	
	}

	if(RecData[1] == 0x01)
	{
		RecData[1] = 0x00;
	}

	result = AddTLV((BYTE *)&ResData[1],(BYTE  *)&RecData[0]);	//add alpha text

	if(result == FALSE)
	{
		return ExitStkPerform;
	}

	result = SendProactiveCommand();

	return result;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephSetupCallSingle

  input parameter:   commQualify

  					 dcs:UCS2/GSMDEFAULT

					 alphaIdentifier(LV,Variable Reference,only dcs is 0x80)

					 address(LV,Variable Reference)

  output parameter:  Success/BackMove/ExitStkPerform

  description:       Setup Call

  code status:       OK

  test status:

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephSetupCallSingle(BYTE commQualify, BYTE dcs,BYTE * alphaIdentifier, BYTE * address)
{
	BYTE idata result;

	//init the command details and device iden in command of selec
	InitProactiveCommand(CMD_SETUPCALL,commQualify,DEV_NETWORD);

	if (alphaIdentifier != 0)
	{
		RecData[0] = 0x85; // alpha identifier

		if(dcs == UCS2)
		{
			RecData[1] = 0x01;

			RecData[2] = 0x80;
		}
		else
		{
			RecData[1] = 0x00;
		}

		AddValue((BYTE *)&RecData[1], (BYTE  *)&alphaIdentifier[0]);

		result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	RecData[0] = 0x86; // Address

	RecData[1] = 0x00;

	if (address != NULL)
	{
		AddValue((BYTE *)&RecData[1], (BYTE  *)&address[0]);
	}

	result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

	if(result == FALSE)
	{
		return ExitStkPerform;
	}

	result = SendProactiveCommand();

	return result;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephSendShortMessageSingle

  input parameter:   commQualify

					 dcs:UCS2/GSMDEFAULT

					 alphaIdentifier(LV,Variable Reference, only dcs is 0x80)

					 address(LV)

					 smsTPDU(mti,tp-mr,pid,dcs,udl,ud,Variable Reference)

  output parameter:  Success/BackMove/ExitStkPerform

  description:       Send the message

  Code Status:  	 OK

  test status:

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephSendShortMessageSingle(BYTE commQualify,BYTE  * alphaIdentifier, BYTE * address, BYTE * smsTPDU)
{
	BYTE idata result;

	//init the command details and device iden in command of select item
	//InitProactiveCommand(CMD_SENDSHORTMESSAGE,commQualify,DEV_NETWORD);
	InitProactiveCommand(CMD_SENDSHORTMESSAGE,(commQualify&0x7F),DEV_NETWORD);          //扩展commQualify的最高bit，若commQualify最高bit为1，UDHI为1，否则为0

	if ((alphaIdentifier != NULL)&&(alphaIdentifier[0] != 0x00))
	{
		RecData[0] = 0x85;	// alpha identifier

		memcpy_G((BYTE *)&RecData[1], &alphaIdentifier[0],alphaIdentifier[0]+1);

		result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	if (address != NULL)
	{
		RecData[0] = 0x86; 	// Address

		memcpy((BYTE *)&RecData[1], &address[0], address[0]+1);

		if (RecData[1] != 0x00)	//no Address
		{
			result = AddTLV(&ResData[1], (BYTE  *)&RecData[0]);

			if (result == FALSE)
			{
				return ExitStkPerform;
			}
		}
	}

	RecData[0] = 0x8B;	// SMS TPDU

	RecData[1] = 0x02;

	if((commQualify&0x80)==0x80)                                   //扩展commQualify的最高bit，若commQualify最高bit为1，UDHI为1，否则为0
	{
		RecData[2] = 0x41;
	}
	else
	{
		RecData[2] = 0x01;
	}

	RecData[3] = 0x00;

	if (smsTPDU != NULL)
	{
		AddValue((BYTE *)&RecData[1], (BYTE  *)&smsTPDU[0]);
	}

	result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

	if (result == FALSE)
	{
		return ExitStkPerform;
	}

#ifdef CDMASMS
	
	if(G_NetWorkMode == CDMA_NETWORK)  //C网短信需要转换
	{		
		result = TransSMSFomat();
		
		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}
#endif

	result = SendProactiveCommand();

	return result;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephGetInputSingle

  input parameter:   comamndQulify

					 varId

					 dcs:USC2/GSMDEFAULT/GSMCOMPRESSED

					 textString(LV,Variable Reference)

					 minLength

					 maxLength


  output parameter:  Success/BackMove/ExitStkPerform

  description:       Input a string and store the string in varId

  Code status:		OK

  test status:		OK

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephGetInputSingle(BYTE commQualify,BYTE varId,BYTE dcs, BYTE  * textString, BYTE minLength, BYTE maxLength)
{
	BYTE idata result;
	BYTE idata commandQualifier;

	commandQualifier = commQualify;

	if((ValueBit(G_SystemStatus,B_meSurportUcs2) == FALSE)&&((commQualify&0x02) == 0x02))
	{
		commandQualifier = commQualify&0xFD ;

		SetBit(G_SystemStatus,B_needChange);
	}
	else
	{
		ClearBit(G_SystemStatus,B_needChange);
	}

	InitProactiveCommand(CMD_GETINPUT, commandQualifier, DEV_ME);

    RecData[0] = 0x8D;

	if(dcs == UCS2)
	{
		RecData[1] = 0x01;

		RecData[2] = 0x08;
	}
	else
	{
		RecData[1] = 0x01;

		RecData[2] = dcs;
	}

	if(textString != NULL)
	{
		AddValue((BYTE *)&RecData[1], (BYTE  *)&textString[0]);
	}

	if(RecData[1] == 0x01)
	{
		RecData[1] = 0x00;
	}

	result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);	// add Alpha identifier(optional)

	if (result == FALSE)
	{
		return ExitStkPerform;
	}

	RecData[0] = 0x91;

	RecData[1] = 0x02;

	RecData[2] = minLength;

	RecData[3] = maxLength;

	result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);	// add text string

	if (result == FALSE)
	{
		return ExitStkPerform;
	}

	result = SendProactiveCommand();

	if((result == CMD_Success)&&(varId != 0x00))
	{
	   GetResponseTextString(commQualify,&ResData[0],maxLength);

	   WriteVariable(varId,&ResData[0]);
	}

	return result;
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephSelectItemSingle

  input parameter:   comamndQulify

					 dcs:UCS2/GSMDEFAULT

					 alphaIdentifier(LV,Variable Reference)

					 itemDataObjects(LVLV...,Variable Reference)

					 itemLgth

                     output parameter:  Item Number/ExitStkPerform/BackMove

  description:       Display menu to choose and return the number of item you choose

  code Status:		OK

  test status:      OK

 -------------------------------------------------------------------------------------------------------------------*/
BYTE cephSelectItemSingle(BYTE commQualify, BYTE dcs,BYTE * alphaIdentifier, BYTE  * itemDataObjects,BYTE itemLgth)
{
	BYTE idata i;
	BYTE idata j;
	BYTE idata result;

	InitProactiveCommand(CMD_SELECTITEM,commQualify,DEV_ME);

	if(alphaIdentifier != NULL)
	{
	    RecData[0] = 0x85;

		if(dcs == UCS2)
		{
			RecData[1] = 0x01;

			RecData[2] = 0x80;
		}
		else
		{
			RecData[1] = 0x00;
		}

		AddValue((BYTE *)&RecData[1], (BYTE  *)&alphaIdentifier[0]);

		result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);	// add Alpha identifier(optional)

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	j = 1;

	for(i=0; i<itemLgth; i=i+itemDataObjects[i]+1)
	{
#ifdef LOCALOTA
		if(G_DeleteItem == TRUE)
		{
			if(j == 1)
			{
				j=0x0D;
			}
			else if(j == 0x0E )
			{
				j= 0x0C;
			}
			else if(j == 0x0D)
			{
				j=0x0E;
			}
			else	//for baihe-test
			{;}
		}
#endif
		RecData[0] = 0x8F;

		if(dcs == UCS2)
		{
			RecData[1] = 0x02;

			RecData[2] = j;

			RecData[3] = 0x80;
		}
		else
		{
			RecData[1] = 0x01;

			RecData[2] = j;
		}

		j++;

		AddValue((BYTE *)&RecData[1], (BYTE  *)&itemDataObjects[i+0]);

		result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);	//add item data object

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	result = SendProactiveCommand();

	return result;
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephRefreshSingle

  input parameter:   commQualify

					 fileList(LV)

  output parameter:  Success/BackMove/ExitStkPerform

  description:       this function (Refresh)is to init the buffer

  Code status:		OK

  test status:      OK

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephRefreshSingleList(BYTE commQualify, BYTE * fileList)
{
	BYTE idata result;

	InitProactiveCommand(CMD_REFRESH, commQualify, DEV_ME);

	if((commQualify == 0x02)||(commQualify == 0x01))
	{
		RecData[0] = 0x92;

		RecData[1] = 0x00;

		if(fileList != NULL)
		{
			memcpy((BYTE *)&RecData[1], (BYTE *)&fileList[0], fileList[0]+1);

			result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);	//add file list

			if (result==FALSE)
			{
				return ExitStkPerform;
			}
		}
		else
		{
			return ExitStkPerform;
		}
	}
	else
	{
		if (fileList != NULL)
		{
			RecData[0] = 0x92;

			RecData[1] = 0x00;

			memcpy((BYTE *)&RecData[1], &fileList[0], fileList[0]+1);

			result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);	//add file list

			if (result == FALSE)
			{
				return ExitStkPerform;
			}
		}
	}

	result = SendProactiveCommand();

	return result;
}



/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephRefreshSingle

  input parameter:   commQualify

					 fileList(LV)

  output parameter:  Success/BackMove/ExitStkPerform

  description:       this function (Refresh)is to init the buffer

  Code status:		OK

  test status:      OK

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephRefreshSingle(BYTE commQualify)
{
	BYTE idata result;

	InitProactiveCommand(CMD_REFRESH, commQualify, DEV_ME);	//目前电信的代码只支持commQualify为03或者04的情况

	result = SendProactiveCommand();

	return result;
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephReceiveDataSingle

  input parameter:   void

  output parameter:  Success/ExitStkPerform

  description:       More Time

  code status:       OK

  test status:

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephReceiveDataSingle(BYTE channleId, BYTE dcs, BYTE * alpha, BYTE dataLgth)
{
	BYTE idata result;

	InitProactiveCommand(CMD_RECEIVEDATA, 0x00, (channleId+0x20));

	if(alpha != NULL)
	{
		RecData[0] = 0x85;	// alpha identifier

		if(dcs == UCS2)
		{
			RecData[1] = 0x01;

			RecData[2] = 0x80;
		}
		else
		{
			RecData[1] = 0x00;
		}

		AddValue((BYTE *)&RecData[1], (BYTE  *)&alpha[0]);

		result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	RecData[0] = 0x37;

	RecData[1] = 0x01;

	RecData[2] = dataLgth;

	AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

	result = SendProactiveCommand();

#ifndef OTA
	if(result == CMD_Success)
	{
		result = FindTag((BYTE *)&RecData[0],TLV_CHANNELDATA,P3);

		if(result == FALSE)
		{
			return ExitStkPerform;
		}
	}
#endif

	return result;
}


BYTE cephReceiveDataSingleCATTP(BYTE channleId, BYTE dcs, BYTE * alpha, BYTE dataLgth)
{
	BYTE idata result;

	InitProactiveCommand(CMD_RECEIVEDATA, 0x00, (channleId+0x20));

	if(alpha != NULL)
	{
		RecData[0] = 0x85;	// alpha identifier

		if(dcs == UCS2)
		{
			RecData[1] = 0x01;

			RecData[2] = 0x80;
		}
		else
		{
			RecData[1] = 0x00;
		}

		AddValue((BYTE *)&RecData[1], (BYTE  *)&alpha[0]);

		result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	RecData[0] = 0x37;

	RecData[1] = 0x01;

	RecData[2] = dataLgth;

	AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

	result = SendProactiveCommand();

	if(result == CMD_Success)
	{
		result = FindTag((BYTE *)&RecData[0],TLV_CHANNELDATA,P3);

		if(result == FALSE)
		{
			return ExitStkPerform;
		}
	}


	return result;
}



/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephCloseChannelSingle

  input parameter:   commQualify

					 dcs:UCS2/GSMDEFAULT

					 alphaIdentifier(LV,Variable Reference, only dcs is 0x80)

					 channel

  output parameter:  CMD_Success/BackMove/ExitStkPerform

  description:       Send the message

  Code Status:  	 OK

  test status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE cephCloseChannelSingle(BYTE commQualify, BYTE channel)
{
	if( channel > 7 )
	{
		return ExitStkPerform;
	}

	channel = DEVICE_CHANNEL1 + channel - 1;

	//init the command details and device iden in command of selec
	InitProactiveCommand(CMD_CLOSECHANNEL,commQualify,channel);

	return SendProactiveCommand();
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephSendDataSingle

  input parameter:   commQualify

					 dcs:UCS2/GSMDEFAULT

					 alphaIdentifier(LV,Variable Reference, only dcs is 0x80)

					 channel

					 data(LV)

  output parameter:  Success/BackMove/ExitStkPerform

  description:       Send the message

  Code Status:  	 OK

  test status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE cephSendDataSingle(BYTE commQualify, BYTE channel, BYTE* dataBuf )
{
	BYTE idata result;

	if( ( channel > 7 ) || ( dataBuf == NULL ) || (dataBuf[0] == 0 ) )
	{
		return ExitStkPerform;
	}

	channel = DEVICE_CHANNEL1 + channel - 1;

	//init the command details and device iden in command of selec
	InitProactiveCommand(CMD_SENDDATA,commQualify,channel);

	RecData[0] = 0x36;

	memcpy(&RecData[1], &dataBuf[0], dataBuf[0]+1 );

	result = AddTLV(&ResData[1], (BYTE *)&RecData[0]);

	if (result == FALSE)
	{
		return ExitStkPerform;
	}

	result = SendProactiveCommand();

	return result;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephGetChannelStatus

  input parameter:   channel_num(  0xFF--->all available channels status,
  							others ---->specific channel status     )
  				    varId

  output parameter:  Success/BackMove/ExitStkPerform

  description:       Get Channel Status

  code status:       OK

  test status:

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephGetChannelStatusSingle(BYTE varId)
{
	BYTE idata result;
	BYTE idata pos;

	//init the command details and device iden in command of selec
	InitProactiveCommand(CMD_GETCHANNELSTATUS, 0x00, DEV_ME);

	result = SendProactiveCommand();

	if(result == CMD_Success)
	{
		pos = FindTag(&RecData[0], TLV_CHANNELSTATUS, P3);

		if(pos == FALSE)
		{
			return ExitStkPerform;
		}

		if(varId != 0xFF)
		{
			WriteVariable(varId, &RecData[pos]);
		}

		return (RecData[pos+1]&0x07);
	}

	return result;
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephOpenChannelSingle

  input parameter:

  output parameter:  Success/BackMove/ExitStkPerform

  description:       Open Channel

  code status:       OK

  test status:

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephOpenChannelSingle(BYTE commQualify, BYTE* address, BYTE bearerType, BYTE  * bearer, BYTE * bufSize, BYTE * portNum, BYTE tpType,BYTE  *  AccessName)
{
	BYTE idata result, num;

	//init the command details and device iden in command of selec
	InitProactiveCommand(CMD_OPENCHANNEL,commQualify,DEV_ME);

	RecData[0] = TLV_BEARERDESCRIPTION;	//bearer description

	if( bearer == NULL )
	{
		RecData[1] = 0x01;

		RecData[2] = bearerType;
	}
	else
	{
		RecData[1] = bearer[0] + 1;

		RecData[2] = bearerType;

		memcpy_G(&RecData[3], (BYTE  *)&bearer[1], bearer[0]);
	}

	result = AddTLV(&ResData[1], (BYTE  *)&RecData[0]);

	if (result == FALSE)
	{
		return ExitStkPerform;
	}

	if(bufSize != NULL)
	{
		RecData[0] = TLV_BUFFERSIZE;	//Buffer Size

		RecData[1] = 0x02;

		RecData[2] = bufSize[0];

		RecData[3] = bufSize[1];

		result = AddTLV(&ResData[1], (BYTE  *)&RecData[0]);

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	if(AccessName != NULL)
	{
		RecData[0] = TLV_NETWORKACCESSNAME;

		memcpy_G( &RecData[1], (BYTE *)&AccessName[0], AccessName[0]+1);

		result = AddTLV(&ResData[1], (BYTE  *)&RecData[0]);

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	if( portNum != NULL )
	{
		RecData[0] = TLV_SIMMEINTERFACETRANSPORT;

		RecData[1] = 0x03;

		RecData[2] = tpType;

		RecData[3] = portNum[0];

		RecData[4] = portNum[1];

		result = AddTLV(&ResData[1], (BYTE  *)&RecData[0]);

		if(result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	if( address != NULL )
	{
		RecData[0] = 0x3E;		//address

		memcpy( &RecData[1], &address[0], address[0]+1);

		result = AddTLV(&ResData[1], (BYTE  *)&RecData[0]);

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	result = SendProactiveCommand();

	if(result == CMD_Success)
	{
		num = FindTag(&RecData[0],TLV_CHANNELSTATUS,P3);

		if(num == FALSE)
		{
			return ExitStkPerform;
		}
		else
		{
			return (RecData[num+1] & 0x07);
		}
	}

	return result;
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephGetInkeySingle

  input parameter:   commQualify:	命令限制
  				vaiId:		存放输入的内容的变量
  				dcs:			USC2/GSMDEFAULT/GSMCOMPRESSED
  				textString:	LV结构

  output parameter:       Success/BackMovePre/ExitStkPerform

  description:

  coder:       xuqs

  test status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE cephGetInkeySingle(BYTE commQualify,BYTE varId, BYTE dcs, BYTE  * textString)
{
	BYTE idata result;

	if((ValueBit(G_SystemStatus,B_meSurportUcs2)==FALSE)&&((commQualify&0x02) == 0x02))//若终端不支持UCS2
	{
		commQualify = commQualify&0xFD ;//则限制只允许ASCII

		SetBit(G_SystemStatus,B_needChange);
	}
	else
	{
		ClearBit(G_SystemStatus,B_needChange);
	}

	InitProactiveCommand(CMD_GETINKEY, commQualify, DEV_ME);

	RecData[0] = 0x8D;

	if(dcs == UCS2)
	{
		RecData[1] = 0x01;

		RecData[2] = 0x08;
	}
	else
	{
		RecData[1] = 0x01;

		RecData[2] = dcs;
	}

	if(textString != NULL)
	{
		AddValue((BYTE *)&RecData[1], (BYTE  *)&textString[0]);
	}

	if(RecData[1] == 0x01)
	{
		RecData[1] = 0x00;
	}

	result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);	// add Alpha identifier(optional)

	if (result == FALSE)
	{
		return ExitStkPerform;
	}

	result = SendProactiveCommand();

	if((result == CMD_Success)&&(varId != 0x00))
	{
		GetResponseTextString(commQualify,&ResData[0],10);

		WriteVariable(varId,&ResData[0]);
	}

	return result;
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephMoreTimeSingle

  input parameter:    void

  output parameter:  Success/BackMove/ExitStkPerform

  description:

  coder:       xuqs

  test status:

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephMoreTimeSingle(void)
{
	BYTE idata result;

	InitProactiveCommand(CMD_MORETIME, 0x00, DEV_ME);

	result = SendProactiveCommand();

	return result;
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephMoreTimeSingleTelecom,此用法为中国电信平台使用

  input parameter:    void

  output parameter:  Success/BackMove/ExitStkPerform

  description:

  coder:       xuqs

  test status:

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephMoreTimeSingleTelecom(void)
{
	BYTE idata result;

		
  while(1)
	{
		InitProactiveCommand(CMD_MORETIME, 0x00, DEV_ME);
	
		result = SendProactiveCommand();	
	}

	return result;	
	
}





/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephPlayToneSingle

  input parameter:    dcs: 		 UCS2/GSMDEFAULT/GSMCOMPRESS
  				alphaIdentifier: Alpha内容(LV)
  				tone:              	语音类型
  				duration:		语音时间长度(LV)

  output parameter:  Success/BackMove/ExitStkPerform

  description:

  coder:       xuqs

  test status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE cephPlayToneSingle( BYTE dcs,BYTE * alphaIdentifier, BYTE tone, BYTE timeUint, BYTE timeInterval)
{
	BYTE idata result;

	InitProactiveCommand(CMD_PLAYTONE, 0x00, DEV_EARPIECE);

	if(alphaIdentifier != NULL)
	{
		RecData[0] = 0x8D;

		if(dcs == UCS2)
		{
			RecData[1] = 0x01;

			RecData[2] = 0x08;
		}
		else
		{
			RecData[1] = 0x01;

			RecData[2] = dcs;
		}

		AddValue((BYTE *)&RecData[1], (BYTE  *)&alphaIdentifier[0]);

		if(RecData[1] == 0x01)
		{
			RecData[1] = 0x00;
		}

		result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);	// add Alpha identifier(optional)

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	if(tone != FALSE)
	{
		RecData[0] = 0x8E;//tone

		RecData[1] = 0x01;

		RecData[2] = tone;

		result = AddTLV(&ResData[1], (BYTE  *)&RecData[0]);

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	RecData[0] = 0x84;//duration

	RecData[1] = 0x02;

	RecData[2] = timeUint;

	RecData[3] = timeInterval;

	result = AddTLV(&ResData[1], (BYTE  *)&RecData[0]);

	if (result == FALSE)
	{
		return ExitStkPerform;
	}

	result = SendProactiveCommand();

	return result;

}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephPollIntervalSingle

  input parameter:    duration: 时间长度

  output parameter:

  description:

  coder:

  test status:

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephPollIntervalSingle(BYTE cmdType,BYTE timeUint, BYTE timeInterval)
{
	BYTE idata result;

	InitProactiveCommand(cmdType, 0x00, DEV_ME);

	if (cmdType == CMD_POLLINTERVAL)
	{
		RecData[0] = 0x84;//duration
	
		RecData[1] = 0x02;
	
		RecData[2] = timeUint;
	
		RecData[3] = timeInterval;
	
		result = AddTLV(&ResData[1], (BYTE  *)&RecData[0]);
	
		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	result = SendProactiveCommand();

	return result;
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephProLocalInforSingle

  input parameter:   varibleId

  					 commQualify

  output parameter:  Success/BackMove/ExitStkPerform

  description:       Provide Local Information and store in varibleId

  code status:       OK

  test status:

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephProLocalInforSingle(BYTE varibleId,BYTE commQualify)
{
	BYTE idata result,index;

	InitProactiveCommand(CMD_PLINFO,commQualify,DEV_ME);	//init the command details and device iden in command of selec

	result = SendProactiveCommand();

	if(result == CMD_Success)
	{
		if(commQualify == 0x00)
		{
			index = FindTag((BYTE *)&RecData[0],TLV_LOCATIONINFOMRATION,P3);
		}
		else if(commQualify == 0x01)
		{
			index = FindTag((BYTE *)&RecData[0],TLV_IMEI,P3);
		}
		else if(commQualify == 0x02)
		{
			index = FindTag((BYTE *)&RecData[0],TLV_NETWORKMEASUREMENTRESULT,P3);
		}
		else if(commQualify == 0x03)
		{
			index = FindTag((BYTE *)&RecData[0],TLV_DATETIMEANDTIMEZONE,P3);
		}
		else if(commQualify == 0x04)
		{
			index = FindTag((BYTE *)&RecData[0],TLV_LANGUAGE,P3);
		}
		else if(commQualify == 0x05)
		{
			index = FindTag((BYTE *)&RecData[0],TLV_TIMINGADVANCE,P3);
		}
		else		//for baihe-test
		{
			index = 0x00;
		}

		if(index == 0x00)
		{
			RecData[0] = 0x00;

			WriteVariable(varibleId, (BYTE *)&RecData[0]);
		}
		else
		{
			WriteVariable(varibleId, (BYTE *)&RecData[index]);
		}
	}

	return result;
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephSendUssd

  input parameter:   dcsAlpha			alpha标识的DCS
  				 	* alphaIdentifier	alpha标识
  				 	dcsUssd				Data coding scheme，详见TS 123 038,CBS Data Coding Scheme
  				 	* ussdString 		USSD串数据,采用与dcsUssd相应的编码格式 详见TS 131 111
										MMI Mode 7bit	or	Application Mode 8bit

  output parameter:  Success/BackMove/ExitStkPerform

  description:       Send Ussd Proactive Command

  code status:

  test status:

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephSendUssd(BYTE dcsAlpha, BYTE  * alphaIdentifier, BYTE dcsUssd, BYTE * ussdString)
{
	BYTE idata result;

	InitProactiveCommand(CMD_SENDUSSD,0x00,DEV_NETWORD);

	if (alphaIdentifier != NULL)
	{
		RecData[0] = 0x05;	//alpha identifier,option

		if(dcsAlpha == UCS2)
		{
			RecData[1] = 0x01;

			RecData[2] = 0x80;
		}
		else
		{
			RecData[1] = 0x00;
		}

		AddValue((BYTE *)&RecData[1], (BYTE  *)&alphaIdentifier[0]);

		result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	RecData[0] = 0x8A;	//tag USSD

	RecData[1] = 0x01;

	RecData[2] = dcsUssd;	//DCS Cell Broadcast,

	AddValue((BYTE *)&RecData[1], (BYTE  *)ussdString );

	result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

	if (result == FALSE)
	{
		return ExitStkPerform;
	}

	result = SendProactiveCommand();

	return result;
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephLaunchBrowser

  input parameter:   commQualify:
					 urlString:		LV 网址字符串，不包括 http://
					 alphaIdentifier:

  output parameter:	 Success/BackMove/ExitStkPerform

  description:

  test:

  status:			new

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephLaunchBrowser(BYTE commQualify,BYTE * urlString,BYTE  * alphaIdentifier)
{
	BYTE idata result;

	InitProactiveCommand(CMD_LAUNCHBROWSER,commQualify,DEV_ME);

	if(urlString == NULL)
	{
		return ExitStkPerform;
	}

	RecData[0] = 0x30;	//Tag BrowerIdentity

	RecData[1] = 0x01;

	RecData[2] = 0x00;

	result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

	if (result == FALSE)
	{
		return ExitStkPerform;
	}

	RecData[0] = 0xB1;	//Tag URL

	RecData[1] = 0x00;	//use the default URL

	/*--------添加http://---------*/
	RecData[2] = 0x68;
	RecData[3] = 0x74;
	RecData[4] = 0x74;
	RecData[5] = 0x70;
	RecData[6] = 0x3A;
	RecData[7] = 0x2F;
	RecData[8] = 0x2F;

	RecData[1] = 0x07;

	if(urlString[0] != 0x00)
	{
		AddValue((BYTE *)&RecData[1], (BYTE  *)&urlString[0]);	//URL string code in "SMS 7bits default alphabet"
	}

	result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

	if (result == FALSE)
	{
		return ExitStkPerform;
	}

	if (alphaIdentifier != NULL)
	{
		RecData[0] = 0x05;	// alpha identifier,option

		RecData[1] = 0x01;

		RecData[2] = 0x80;

		AddValue((BYTE *)&RecData[1], (BYTE  *)&alphaIdentifier[0]);

		result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}
	result = SendProactiveCommand();

	return result;
}



/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephLaunchBrowser

  input parameter:   commQualify:
					 urlString:		LV 网址字符串，不包括 http://
					 alphaIdentifier:

  output parameter:	 Success/BackMove/ExitStkPerform

  description:

  test:

  status:			new

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephLaunchBrowser_JSAPP(BYTE commQualify,BYTE * urlString,BYTE  * alphaIdentifier)
{
	BYTE idata result;

	InitProactiveCommand(CMD_LAUNCHBROWSER,commQualify,DEV_ME);

	if(urlString == NULL)
	{
		return ExitStkPerform;
	}

//===========================根据脚本 launch browser  不需要浏览器标示  20160421

//	RecData[0] = 0x30;	//Tag BrowerIdentity
//
//	RecData[1] = 0x01;
//
//	RecData[2] = 0x00;
//
//	result = AddTLV((BYTE xdata *)&ResData[1], (BYTE  *)&RecData[0]);
//
//	if (result == FALSE)
//	{
//		return ExitStkPerform;
//	}
//===========================根据脚本 launch browser  不需要浏览器标示  20160421

	RecData[0] = 0xB1;	//Tag URL

	RecData[1] = 0x00;	//use the default URL

	/*--------添加http://---------*/
//	RecData[2] = 0x68;
//	RecData[3] = 0x74;
//	RecData[4] = 0x74;
//	RecData[5] = 0x70;
//	RecData[6] = 0x3A;
//	RecData[7] = 0x2F;
//	RecData[8] = 0x2F;
//
//	RecData[1] = 0x07;

	if(urlString[0] != 0x00)
	{
		AddValue((BYTE *)&RecData[1], (BYTE  *)&urlString[0]);	//URL string code in "SMS 7bits default alphabet"
	}

	result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

	if (result == FALSE)
	{
		return ExitStkPerform;
	}

	if (alphaIdentifier != NULL)
	{
		RecData[0] = 0x05;	// alpha identifier,option

		RecData[1] = 0x01;

		RecData[2] = 0x80;

		AddValue((BYTE *)&RecData[1], (BYTE  *)&alphaIdentifier[0]);

		result = AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}
	result = SendProactiveCommand();

	return result;
}


//BYTE cephTimerManagementSingle(BYTE commQualify,BYTE timerTd, BYTE  * timerValue)
//{
//	BYTE idata result,pos;

//	InitProactiveCommand(CMD_TIMERMANAGE,commQualify,DEV_ME);
//	
//	RecData[0] = 0x24;

//	RecData[1] = 0x01;

//	RecData[2] = timerTd;

//	result = AddTLV(&ResData[1], &RecData[0]);

//	if (result == FALSE)
//	{
//		return ExitStkPerform;
//	}

//	if (timerValue != NULL)
//	{
//		RecData[0] = 0x25;

//		RecData[1] = 0x00;

//		AddValue(&RecData[1], &timerValue[0]);

//		result = AddTLV(&ResData[1], &RecData[0]);

//		if (result == FALSE)
//		{
//			return ExitStkPerform;
//		}
//	}
//	
//	result = SendProactiveCommand();

//	if (result == CMD_Success)
//	{
//		pos = FindTag(&RecData[0],TLV_RESULT,P3);
//		
//		if (pos != FALSE)
//		{
//			result = RecData[pos+1];	
//		}	
//	}
//	return result;	
//} 
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephSendUssd

  input parameter:  dcs
  				 	alphaIdentifier
  					ssString

  output parameter:  Success/BackMovePre/ExitStkPerform

  description:       Send Ussd Proactive Command

  code status:       

  test status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE cephSendSsSingle(BYTE dcs,BYTE  * alphaIdentifier,BYTE  * ssString)
{
	BYTE idata result;

	InitProactiveCommand(CMD_SENDSS,0x00,DEV_NETWORD);		

	if (alphaIdentifier != NULL)
	{
		RecData[0] = 0x05; 						//alpha identifier,option

		if(dcs == UCS2)
		{
			RecData[1] = 0x01;

			RecData[2] = 0x80;
		}
		else
		{
			RecData[1] = 0x00;
		}

		AddValue((BYTE *)&RecData[1],&alphaIdentifier[0]);
	
		result = AddTLV((BYTE *)&ResData[1], (BYTE *)&RecData[0]);
		
		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}

	RecData[0] = 0x89;						
 
	RecData[1] = 0x00;

	AsciiToADN((BYTE *)&ssString[0], &RecData[1]);					

	result = AddTLV((BYTE *)&ResData[1], (BYTE *)&RecData[0]);
	
	if (result == FALSE)
	{
		return ExitStkPerform;
	}

	result = SendProactiveCommand();

	return result;
}



/**
 * @brief 		Get Response Text String
 * 
 * @param 		commQualify 
 * @param 		buf : 存放数据内容的buffer(LV,无DCS)
 * @param 		maxLength ：需获取的数据内容的长度
 * 
 * @author 		WG 
 * 
 * @history 	add comment by WG, @071224 13:59
 */
void GetResponseTextString(BYTE commQualify,BYTE * buf,BYTE maxLength)
{
    BYTE idata num,index;
#ifdef UnicomSET
	BYTE idata des;
#endif

	num = FindTag((BYTE *)&RecData[0],TLV_TEXTSTRING,P3);

	if(num == 0x00)
	{
		buf[0] = 0x00;
	}
	else
	{
		index = num;

		if (RecData[index] < 2)
		{
			buf[0] = 0x00;
		}
		else
		{
			if((ValueBit(G_SystemStatus,B_needChange) == FALSE)&&((RecData[index+1]&0x04) == 0x04)&&((commQualify&0x02) == 0x02))
			{
				SetBit(G_SystemStatus,B_needChange);
			}

#ifdef UnicomSET
			des = RecData[num+1];
#endif
			RecData[num+1] = RecData[num] - 1;

			num++;

			if(ValueBit(G_SystemStatus,B_needChange) == TRUE)
			{
  				if(RecData[num] > (maxLength/2))
				{
					RecData[num] = (maxLength/2);
				}

  				AsciiToUCS2((BYTE *)&RecData[num],(BYTE  *)&buf[0]);
			}
			else
    		{
				if(RecData[num] > maxLength)
				{
					RecData[num] = maxLength;
				}

				memcpy(&buf[0],&RecData[num],RecData[num]+1);
      		}
#ifdef UnicomSET
            RecData[num] = des;
#endif

		}
	}
}


/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 cephRefreshSingle_DX	中国电信平台使用函数

  input parameter:   commQualify

					 fileList(LV)

  output parameter:  Success/BackMove/ExitStkPerform

  description:       this function (Refresh)is to init the buffer

  Code status:		OK

  test status:      OK

 -------------------------------------------------------------------------------------------------------------------*/

BYTE cephRefreshSingle_DX(BYTE commQualify)
{
	BYTE idata result;

	InitProactiveCommand(CMD_REFRESH, commQualify, DEV_ME);	//目前电信的代码只支持commQualify为03或者04的情况

	result = SendProactiveCommand();

	return result;
}


