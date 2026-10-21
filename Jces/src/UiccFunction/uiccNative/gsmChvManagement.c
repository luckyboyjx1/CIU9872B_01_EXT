/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
 			V1.1 修改PIN、puk校验成功后，恢复最大尝试次数为2E10对应记录的第三各字节的高4个BIT
 			V1.2 为兼容联通脚本测试和UICC脚本测试，对于USIM卡，不允许Disable PIN2 -20100323
 			
 			V1.3  修改函数ChvHandle，当卡片锁住时，保持之前的卡片状态不变 @20110810 
 			
 			V1.4  修改函数VerifyChangeUnblockChv，当第一次校验PIN或ADM时，如果校验成功，则不更新PIN 所在的文件，主要时考虑断电时可能会重复擦写3F00所在页，导致该页写穿 @20140408 byxuyz

 */
#include "uicc_main.h"		   
#include "uicc_error.h"
#include "gsmpublic.h"
//#include "resume.h"
#include "interface.h"
#include <string.h>
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 KeyResume                                                                                                                   
                                                                                                                                                
  Input parameter:   x
                              					                                                                                                         
  Output parameter:                                                                                                                 
                                                                                                                                                
  description:       重置KEY的次数为最大次数
  
  status:		        

 -------------------------------------------------------------------------------------------------------------------*/
BYTE KeyResume(BYTE x)
{
	return (x&0xF0)+((x&0xF0)>>4);
}

/**
 * @brief 		       VERIFY PIN / CHANGE PIN / UNBLOCK PIN
 					   VreifyChvCommand:	This function verify the CHV with the value stored in the SIM
 					   ChangeChvCommand:	This function change the CHV with the value
					   UnblockPINCommand:	This function is used switch off the  PIN
 *
 * @command format 	   VreifyChvCommand:	A0 20 00 chvNo(01-04) 08 /0X 20 00 referenceData 08(00)  (CHV||ADM)
 					   ChangeChvCommand:	A0 24 00 chvNo(01 02) 10 /0X 24 00 referenceData 10
					   UnblockPINCommand:	A0 2C 00 chvNo(00 02) 10 /0X 2C 00 P2 00(10)

   @Data Content       	VreifyChvCommand:	PIN Value
   						ChangeChvCommand:	Old PIN Value    New PIN Value
 *						UnblockPINCommand:	none  OR  Unblock PIN Value,New PIN Value

 * @Returns		       void
 *
 * @history 	       
 */
void VerifyChangeUnblockChv(void)
{
	BYTE idata oprateMode, tempResult,result;
	KeyStructer xdata pinKey;
	
	#if !NATIVE_CARD_UICC
		PukStructer	xdata pukInfo;
	#endif

	switch(INS)
	{
		case 0x20:
			oprateMode = mode_Verify;	//0x02
			break;
		case 0x24:
			oprateMode = mode_Change;	//0x01
			break;
		case 0x2C:
			oprateMode = mode_Unblock;	//0x00
			break;
		default:
			break;
	}

	if (P1 != 0x00) 
	{
		P1P2Error();                                            	//6B 00													
		return ;
	}

	if(G_CommandMode == Command_Mode_2G)
	{
		if(oprateMode == mode_Verify)
		{
			
			 result = IF_VerifyChv();
	
			 if(result == FALSE)
			 {
			 	return;
			 }
									
			/*
			if((P2==0x00)||(P2>0x05))
			{
				P1P2Error();	                                            //6B 00
				return;
			}
			*/
		}
		else     		//ChangeChv(0x01)	unblock(0x00)
		{
			if((P2 != oprateMode)&&(P2 != 0x02))
			{
				P1P2Error();													//6B 00
				return;
			}
		}
	}
#ifndef SingleSIM
	else	//Command_Mode_3G
	{		
		if((P2 == 0x00))                                       
		{
			FunctionNotSupport();										//6A 81
			return;
		}

		if((P2&0x60) != 0x00)
		{
			P1P2Error();                                                //6B 00														
			return;
		}

		if((P2&0x1F) > 0x08)											
		{
			if((oprateMode == mode_Verify)&&((P2&0x1F) < 0x0D))	   //CHV ||ADM :01 - 0E
			{
			}
			else                                                    //CHV :01-07
			{
				P1P2Error();                                                													
				return;
			}
		}	
	}
#endif

	if(oprateMode == mode_Verify)
	{
		if(P3 != 0x08)
		{
			if((G_CommandMode == Command_Mode_2G)||((G_CommandMode == Command_Mode_3G)&&(P3 != 0x00)))
			{
				P3Error();													//67 00 
		
				return;
			}
		}
	}
	else 
	{
		if( P3 != 0x10 )													//check the p3
		{
#ifndef SingleSIM
			if((G_CommandMode == Command_Mode_3G)&&(oprateMode == mode_Unblock)&&(P3 == 0x00))	 //unblock  P3 == 0x10,0x00
			{
			}
			else	    //change   P3 = 0x10
#endif
			{
			      P3Error();                                         // 67 00																//67 00
			      return;				
			}
		} 
	}

	if(G_CommandMode == Command_Mode_2G)
	{


		IF_VerifyChv2(oprateMode);

/*
#ifndef CSIM
		if((oprateMode == mode_Verify) && (P2 > 0x02))
		{
			P2 = P2 + 0x07;
		}
#endif

		if((oprateMode == mode_Unblock) && (P2 == 0x00))
		{
			P2 = 0x01;
		}

		if(P2 == 0x02)
		{
			P2 = 0x81;										//PIN2
		}
*/				
	}

	
	ReceiveData();

	tempResult = ReadKey(P2,(BYTE*)&pinKey.KeyId);

	if(tempResult == FALSE )                                                     //the key not find 
    {
		ReferencedDataNotFound();											//6A 88
	
		return;
	}

#if !NATIVE_CARD_UICC	
	ReadOrWritePukInfo(pinKey.pukRef,(BYTE*)&pukInfo.PukId,Read_Key);
#endif

#ifndef SingleSIM
 	if(P3 == 0x00)
	{
		if(oprateMode == mode_Verify)
		{
			PinRetryCounter(pinKey.KeyLimit);	     //63cx
		}
		else  //oprateMode == mode_Unblock
		{
			#if NATIVE_CARD_UICC
			PinRetryCounter(pinKey.IscLimit);		 //63cx
			#else		
			PinRetryCounter(pukInfo.PukLimit);		 //63cx
			#endif
		}
		return;	
	}
#endif

	if(oprateMode != mode_Unblock)		//mode_Verify and mode_Change
	{
		if((pinKey.KeyLimit&0x0F)  == PIN_BLOCKED)
		{	
			PinBlocked();													//69 83

			return;
		}

    	if((pinKey.KeyStatus&0x01) == PIN_DISABLED)
		{	
			ChvContradiction();
		
			return;
		}

		pinKey.KeyLimit = pinKey.KeyLimit - 1;									//记数器减一

		tempResult = memcmp((BYTE *)&pinKey.KeyContent[0],RecData,0x08);
	}
	else		//mode_Unblock
	{
	#if 	NATIVE_CARD_UICC
		if((pinKey.IscLimit&0x0F)  == PIN_BLOCKED)
		{
			PinBlocked();													   //69 83
		
			return;
		}
		pinKey.IscLimit = pinKey.IscLimit - 1;		    	//记数器减1?
	
		tempResult = memcmp((BYTE *)&pinKey.IscContent[0],RecData,0x08);
		#else
		
		if((pukInfo.PukLimit&0x0F)  == PIN_BLOCKED)
		{
			PinBlocked();													   //69 83
		
			return;
		}
		pukInfo.PukLimit= pukInfo.PukLimit- 1;		    	//记数器减1?
	
		tempResult = memcmp_p((BYTE xdata *)&pukInfo.PukContent[0],RecData,0x08);
		#endif
	}	
	
	if(tempResult != EQUAL)
	{
		if(oprateMode == mode_Unblock)
		{									
			if(G_CommandMode == Command_Mode_2G)
			{
				#if 	NATIVE_CARD_UICC
				if( (pinKey.IscLimit&0x0F) == PIN_BLOCKED)                                      //pin blocked 
				#else
					if( (pukInfo.PukLimit&0x0F) == PIN_BLOCKED)
				#endif
				{
					PinBlocked();                 //98 40
				}
				else
				{
					SecuritySatisfied();
				}
			}
#ifndef SingleSIM
			else
			{
			#if 	NATIVE_CARD_UICC
				PinRetryCounter(pinKey.IscLimit);										//63 CX
				#else
				PinRetryCounter(pukInfo.PukLimit);										//63 CX
				#endif
			} 
#endif
		}
		else  //verify CHV or change CHV
		{
			if((pinKey.KeyLimit&0x0F) == PIN_BLOCKED )
			{
			   	ConfigureKeyStatus(P2,ClearStatusMode);    //清除当前key的校验状态

				if(G_CommandMode == Command_Mode_2G)
				{
					PinBlocked();                                      //98 40
				}
#ifndef SingleSIM
				else 
				{
					if(oprateMode == mode_Verify)
					{
						pinKey.KeyStatus = pinKey.KeyStatus | PIN_ENABLED;           
					}
				}
#endif
			}
			else 
			{
				SecuritySatisfied();
			}
#ifndef SingleSIM
			if(G_CommandMode == Command_Mode_3G)
			{
				PinRetryCounter(pinKey.KeyLimit);								//63 CX 
			}
#endif
		}

		tempResult = WriteKey(P2,(BYTE *)&pinKey.KeyId);					//记录回写
		
		#if 	!NATIVE_CARD_UICC	
		ReadOrWritePukInfo(pinKey.pukRef,(BYTE*)&pukInfo.PukId,Write_Key);
		#endif

		if(tempResult == FALSE)
		{
			MemoryError();													//65 81/92 40

			return;
		}
  	
	}
	else
	{	
		
		if(oprateMode == mode_Verify)  //modify @20140408,为 减少断电时 重新更新PIN所在页，若当前为第一次校验且校验成功，则不进行PIN 文件的更新
		{
			if(((pinKey.KeyLimit&0x0F)+1) == ((pinKey.KeyLimit&0xF0) >>4 ))   //第一次校验PIN 或ADM 就校验成功
			{
				ConfigureKeyStatus(P2,SetStatusMode);  //设置当前key的状态为已校验
				
				CommandFinish(0x00);
				
				return;
			}
		}
		
		if(oprateMode == mode_Unblock)
		{
			#if 	NATIVE_CARD_UICC
			pinKey.IscLimit = KeyResume(pinKey.IscLimit);
			#else
			pukInfo.PukLimit = KeyResume(pukInfo.PukLimit);
			#endif

			pinKey.KeyStatus = PIN_ENABLED ;
		}

		pinKey.KeyLimit = KeyResume(pinKey.KeyLimit);
		
	 	ConfigureKeyStatus(P2,SetStatusMode);  //设置当前key的状态为已校验

		if(oprateMode != mode_Verify)
		{
			memcpy((BYTE *)&pinKey.KeyContent[0],(BYTE *)&RecData[8],0x08);
		}

		tempResult = WriteKey(P2,(BYTE *)&pinKey.KeyId);					//记录回写
	
#if 	!NATIVE_CARD_UICC			
		ReadOrWritePukInfo(pinKey.pukRef,(BYTE*)&pukInfo.PukId,Write_Key);
	#endif

   		if(tempResult == FALSE)
		{
			MemoryError();													//65 81/92 40

			return;
		} 

		CommandFinish(0x00);
	}		
}

/**
 * @brief 		       DisableEnablePINCommand:This function is used switch off/on the  PIN
 *
 * @command format 	    A0 26 00 chvNo(01 02) 08 /0X 26 P1 P2 08
 *						A0 28 00 chvNo(01 02) 08 /0X 28 00 P2 08
 *
 * @Data Content       PIN Value       
 *     
 * @Returns		       void
 *
 * @history 	       
 */
void ChvHandle(void)
{ 
	BYTE idata tempResult;
	BYTE idata oprateMode;
	KeyStructer xdata pinKey;

	oprateMode = ((INS == 0x26)?PIN_DISABLED:PIN_ENABLED);

	if(P1 != 0x00)
	{
#ifndef SingleSIM
		if((G_CommandMode == Command_Mode_3G)&&(oprateMode == PIN_DISABLED))
		{
			FunctionNotSupport();										//6A 81	
		}
		else
#endif
		{
			P1P2Error();															//6B 00		
		}
		
		return;
	}
	
	if(G_CommandMode == Command_Mode_2G)
	{
		if(P2 != 0x01)
		{
			P1P2Error();															//6B 00		
		
			return;
		}
	} 
#ifndef SingleSIM
	else	//Command_Mode_3G
	{
		if((P2 == 0x00)) 
		{
			 FunctionNotSupport();													//6A 81
			 return;		
		}
		
		if((P2&0x80) == 0x80)
		{
			P1P2Error();             //联通不允许对PIN2进行disable
			return;
		}
	
		if( ((P2&0x1F) > 0x08)||((P2&0x60) != 0x00))														//???
		{
			P1P2Error();															//6B 00
			return;
		}                                                      
	}
#endif

	if( P3 != 0x08 )                                                            
	{
		P3Error();																//67 00
		return;
	}                                                       
 
	ReceiveData(); 

	tempResult = ReadKey(P2,(BYTE*)&pinKey.KeyId);

	if( tempResult == FALSE )                                                    
    {
		ReferencedDataNotFound();									//6A 88
	
		return;
	}
    
	if((pinKey.KeyLimit&0x0F)  == PIN_BLOCKED)
	{	
		PinBlocked();											//69 83
	
		return;
	}

    if((pinKey.KeyStatus&0x01) == oprateMode)
	{
		ChvContradiction();
	
		return;
	}                                                  										

	tempResult = memcmp((BYTE *)&pinKey.KeyContent[0],(BYTE *)&RecData[0],P3);

	if ( tempResult == EQUAL )
	{
		pinKey.KeyStatus = oprateMode ;                                        
							
		ConfigureKeyStatus(P2,SetStatusMode);  //设置当前key的状态为已校验	  	//	chv_1_enabled = PIN_ENABLED ;chv_1_verified = TRUE ;

		pinKey.KeyLimit = KeyResume(pinKey.KeyLimit);
		 
		tempResult = WriteKey(P2,(BYTE *)&pinKey.KeyId);					//记录回写

		if(tempResult == FALSE)
		{
			MemoryError();													//65 81/92 40

			return;
		}   

		CommandFinish(0x00);
	}
	else
	{
		pinKey.KeyLimit = pinKey.KeyLimit - 1 ;									//记数器减1

		tempResult = WriteKey(P2,(BYTE *)&pinKey.KeyId);					//记录回写
		
		if(tempResult == FALSE)
		{
			MemoryError();													//65 81/92 40

			return;
		}
		       
		if((pinKey.KeyLimit&0x0F) == PIN_BLOCKED )								//如果已经为零，则登记
		{
			if(G_CommandMode == Command_Mode_2G)
			{
				PinBlocked();										// 98 40
				
				return ;	                                         
			}
		}
	
		if(G_CommandMode == Command_Mode_2G)
		{                                                   
			SecuritySatisfied();			//98 04
		}  
#ifndef SingleSIM                          											
		else
		{
			PinRetryCounter(pinKey.KeyLimit);			//63 CX										 
		}
#endif
   	}       
}

