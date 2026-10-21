/*
V1.0  创建源文件
*/
 
#include  "typedef.h"
#include  "uicc_main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include "uicc_error.h"
#include  "arithmatic.h"
#include  "ceph.h"
#include  "interface.h"

WORD   xdata G_VARIABLECONTROL;

#ifdef UnicomSET
WORD   xdata OTATempVarAllLength;
#endif 


/*---------------------------------------------------------------------------------------------------------------------------------                                                                              

  Function name :	 InitVariable                                                                                                                   
                                                                                                                                                
  input parameter:   void                                                                  
  					                                                                                                         
  output parameter:  void                                                                                                                
                                                                                                                                                
  description:       this function is to init the varible buffer    

  Status :           OK      

 -------------------------------------------------------------------------------------------------------------------*/                   

void InitVariable(void)
{
	G_VARIABLECONTROL = 0x01;

#ifdef UnicomSET
	OTATempVarAllLength = 0x00;
#endif 

	memset((BYTE *)&VARIABLEBUFFER[0],0x00,sizeof(VARIABLEBUFFER));
}


 /*-------------------------------------------------------------------------------------------------------//                                                                                                                                                  

  Function name :	GetVaribleAddr                                                                                                                   
                                                                                                                                                
  Input parameter:   variblename:   the varible name that want to be find in the varible buffer

                                                                 
  Output parameter:  FALSE(if not find the varible)/the start position of in the varible buffer (if find)                                                                                                                 
                                                                                                                                                
  Description:       this function is find varible from varible buffer ,if finded that return the position else find  return 0

  Status:            OK          

 -------------------------------------------------------------------------------------------------------------------*/                   
WORD GetVaribleAddr(BYTE variablename)
{
	WORD	idata	num;

	variablename = IF_GetVaribleAddr(variablename);

    for(num=1;num<G_VARIABLECONTROL;)   // if(G_VARIABLECONTROL == 0x01)  return FALSE
	{
		if(VARIABLEBUFFER[num] == variablename)
		{
			return num;
		}
		else
		{
			num += VARIABLEBUFFER[num+1]+2 ;
		}	
	}

	return FALSE;
}



/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	WriteVariable                                                                                                                   
                                                                                                                                                
  Input parameter:   variablename:    variable name that be add (the buffer must tlv structure) 					       

                     buffer:  want to add object (must the lv structure)                                                                    
  					                                                                                                         
  Output parameter:  TRUE/FALSE                                                                                                                 
                                                                                                                                                
  Description:       This function is to add tlv structure to the varible buffer of have data           

 -------------------------------------------------------------------------------------------------------------------*/                   
BYTE WriteVariable(BYTE variablename,BYTE * buffer)
{
	BYTE 	idata	variblenum;
	WORD    idata   variblePosition,i;
	WORD    idata   positionAim,positionSource,copylength;	
				

	variblePosition = GetVaribleAddr(variablename);
	
	if(IF_WriteVariable(variablename) == FALSE)
	{
		return FALSE;
	}
	
	variablename = IF_GetVaribleAddr(variablename);

	if(variblePosition == FALSE)														//如果没有在BUFFER中
	{
		if((G_VARIABLECONTROL + buffer[0] + 2)>(sizeof(VARIABLEBUFFER)-1))
		{
			return	FALSE;
		}

		VARIABLEBUFFER[G_VARIABLECONTROL] = variablename;								//add tag		
		
		memcpy(&VARIABLEBUFFER[G_VARIABLECONTROL+1],buffer,buffer[0]+1);				//add lv

		G_VARIABLECONTROL = G_VARIABLECONTROL + buffer[0] + 2;							//add lenth

#ifdef UnicomSET
		if(variablename < 0x10)
		{
			OTATempVarAllLength += buffer[0];      //modified by pxm@08/11/27 用于联通OTA临时变量512  value最大长度控制辅助计算		
		}
#endif

	}
	else										    									//如果在BUFFER
	{
		variblenum = VARIABLEBUFFER[variblePosition+1];

		if(variblenum > buffer[0])
		{
			positionAim = variblePosition+buffer[0]+2;

			positionSource = variblePosition+2+variblenum;

			copylength = G_VARIABLECONTROL-variblePosition-VARIABLEBUFFER[variblePosition +1];

			memcpy(&VARIABLEBUFFER[positionAim],&VARIABLEBUFFER[positionSource],copylength);					//copy the left data to right

			G_VARIABLECONTROL = G_VARIABLECONTROL - (variblenum - buffer[0]);

#ifdef UnicomSET
			if(variablename < 0x10)     //modified by pxm@08/11/27 用于联通OTA临时变量512  value最大长度控制辅助计算
			{
				OTATempVarAllLength -= variblenum - buffer[0];
			}
#endif

		}
		else if(buffer[0] > variblenum)
		{
			if((G_VARIABLECONTROL + (buffer[0]-variblenum))>(sizeof(VARIABLEBUFFER)-1))
			{
				return FALSE;
			}

			positionAim = G_VARIABLECONTROL+(buffer[0]-variblenum)-1;

			positionSource = G_VARIABLECONTROL-1;

			if(G_VARIABLECONTROL > (variblePosition+VARIABLEBUFFER[variblePosition +1]+2))
			{
				copylength = G_VARIABLECONTROL-variblePosition-VARIABLEBUFFER[variblePosition +1]-2;
			}
			else
			{
				copylength = 0 ;
			}
		
			for(i=0;i<copylength;i++)											//copy the left data to right
			{
				VARIABLEBUFFER[positionAim-i]=VARIABLEBUFFER[positionSource-i];						
			}		
	
			G_VARIABLECONTROL = G_VARIABLECONTROL + (buffer[0] - variblenum);

#ifdef UnicomSET
			if(variablename < 0x10)         //modified by pxm@08/11/27 用于联通OTA临时变量512  value最大长度控制辅助计算
			{
				OTATempVarAllLength += buffer[0] - variblenum;
			}
#endif

		}
		else	//for baihe-test
		{;}

		memcpy(&VARIABLEBUFFER[variblePosition+1],buffer,buffer[0]+1);
	}

#ifdef UnicomSET
	if(OTATempVarAllLength > 512)
	{
		return FALSE;
	}
#endif
  	
	return TRUE; 
}


/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	ClearVariable                                                                                                                   
                                                                                                                                                
  Input parameter:   variablename:    variable name that be add (the buffer must tlv structure) 					       
  					                                                                                                         
  Output parameter:  TRUE/FALSE                                                                                                                 
                                                                                                                                                
  Description:       This function is to clear tlv structure to the varible buffer of have data           

 -------------------------------------------------------------------------------------------------------------------*/                   

BYTE ClearVariable(BYTE varId)
{
	BYTE idata tmp[1];

	tmp[0] = 0x00;

	return WriteVariable(varId, &tmp[0]);
}
/**
 * @brief 		This function is to read the varible buffer by the variable name
 * 
 * @param 		variablename :    variable name that be read (the buffer must tlv structure)
 * @param 		buffer :  want to store object (V structure)
 * 
 * @Returns   	FALSE / DATA length 
 * 
 * @author 		WG
 * 
 * @history 	add comment by WG, @071224 13:59
 */
BYTE ReadVarible(BYTE variablename,BYTE * buffer)
{
	WORD idata variblePosition;

	variblePosition = GetVaribleAddr(variablename);

	if(variblePosition == FALSE)							//如果没有在BUFFER中
	{
       	buffer[0x00] = 0x00;

		return FALSE;	
	}
	else										    								//如果在BUFFER
	{
		memcpy(&buffer[0],&VARIABLEBUFFER[variblePosition+2], VARIABLEBUFFER[variblePosition+1]);

		return VARIABLEBUFFER[variblePosition+1];
	}
}




