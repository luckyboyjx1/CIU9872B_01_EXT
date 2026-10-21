/*
V1.0  创建源文件
*/
 
#include <uicc_error.h>
#include "string.h"
#include "uicc_main.h"
#include "gsmpublic.h"
#include "ceph.h"
#include "fileAction.h"
#include "fileSystem.h"
#include "stkhead.h"
#include "uicc_interface.h"
//#include "resume.h"

WORD    xdata G_VARIABLECONTROL;

/*---------------------------------------------------------------------------------------------------------------------------------                                                                              

  Function name :	 InitVariable                                                                                                                   
                                                                                                                                                
  input parameter:   void                                                                  
  					                                                                                                         
  output parameter:  void                                                                                                                
                                                                                                                                                
  description:       this function is to init the varible buffer    

  Status :           OK      

 -------------------------------------------------------------------------------------------------------------------*/                   

void InitVariable(void)
{
	G_VARIABLECONTROL = 0x00;

	ecpMem_Set((BYTE xdata *)&VARIABLEBUFFER[0],0x00,sizeof(VARIABLEBUFFER));
}
 /*-------------------------------------------------------------------------------------------------------//                                                                                                                                                  

  Function name :	GetVaribleAddr                                                                                                                   
                                                                                                                                                
  Input parameter:   variblename:   the varible name that want to be find in the varible buffer

                                                                 
  Output parameter:  FALSE(if not find the varible)/the start position of in the varible buffer (if find)                                                                                                                 
                                                                                                                                                
  Description:       this function is find varible from varible buffer ,if finded that return the position else find  return 0

  Status:            OK          

 -------------------------------------------------------------------------------------------------------------------*/                   
WORD GetVaribleAddr(BYTE variablename,BYTE * len)
{
	WORD	idata	offset;

   for(offset=0; offset<G_VARIABLECONTROL;)   // if(G_VARIABLECONTROL == 0x01)  return FALSE
	{
		if(VARIABLEBUFFER[offset] == variablename)
		{
			if(len != NULL)
			{
					*len = VARIABLEBUFFER[offset+1];
			}
			
			return offset;
		}
		else
		{
			offset += VARIABLEBUFFER[offset+1]+2 ;
		}	
	}

   	if(len != NULL)
	{
			*len = 0;
	}

	return OFFSET_NULL_VARIABLE;
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
	WORD    idata   variblePosition;
	WORD    idata   copylength;	

	variblePosition = GetVaribleAddr(variablename,NULL);

	if(variblePosition == OFFSET_NULL_VARIABLE)														//如果没有在BUFFER中
	{
		if((G_VARIABLECONTROL + buffer[0] + 2)>(sizeof(VARIABLEBUFFER)-1))
		{
			return	FALSE;
		}	
	}
	else										    									//如果在BUFFER
	{
		 if(buffer[0] > VARIABLEBUFFER[variblePosition+1])
		{
			if((G_VARIABLECONTROL + (buffer[0]-VARIABLEBUFFER[variblePosition+1]))>(sizeof(VARIABLEBUFFER)-1))
			{
				return FALSE;
			}
		 }
		
		 copylength = (VARIABLEBUFFER[variblePosition+1]+2);//需要将移走的数据长度备份起来，否则移动之后，该值会变掉
		 
		 ecpMem_Move(&VARIABLEBUFFER[variblePosition],&VARIABLEBUFFER[variblePosition+copylength],G_VARIABLECONTROL-variblePosition-copylength);

		 G_VARIABLECONTROL -= copylength;		
	}

	VARIABLEBUFFER[G_VARIABLECONTROL] = variablename;								//add tag		
		
	ecpMem_Copy(&VARIABLEBUFFER[G_VARIABLECONTROL+1],buffer,buffer[0]+1);				//add lv

	G_VARIABLECONTROL += (buffer[0] + 2);							//add lenth
  	
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

	variblePosition = GetVaribleAddr(variablename,NULL);

	if(variblePosition == OFFSET_NULL_VARIABLE)							//如果没有在BUFFER中
	{
       	buffer[0x00] = 0x00;

		return FALSE;	
	}
	else										    								//如果在BUFFER
	{
		ecpMem_Copy(&buffer[0],&VARIABLEBUFFER[variblePosition+2], VARIABLEBUFFER[variblePosition+1]);

		return VARIABLEBUFFER[variblePosition+1];
	}
}


