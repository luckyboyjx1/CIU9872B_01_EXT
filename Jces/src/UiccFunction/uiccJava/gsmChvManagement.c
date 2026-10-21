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
			V1.4 修改函数VerifyChangeUnblockChv，在校验PIN时，如果当前PIN的次数已经是最大次 ，则不执行写E2的操作@20140714 by xuyz  V1.3->1.4

 */
#include "uicc_main.h"	
//#include  "main.h"
#include "gsmpublic.h"
//#include "resume.h"
#include <string.h>
#include <uicc_error.h>
#include "stkhead.h"
#include "fileAction.h"
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




/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   getADFAddr
*
* Description: 该文件主要是获取2g时ADF的地址
*
* auther：     zhxx    20190419
*
* history：    V1.0
*
************************************************************************************************************/

UWORD getADFAddr(void)
{
	BYTE idata dirNum,i;
	UWORD xdata parentfileaddr;
	
	dirNum = GetFileHeadByte(G_CurrentDfAddr[G_ChannelNum],F_DIRNum_offset);//获取当前DF地址的F_DIRNum_offset

	if(dirNum != 0)
	{
		return G_CurrentDfAddr[G_ChannelNum];
	}
	else
	{
		for(i=0; i < DF_Depth; i++)//查找父目录
		{	
			if(i == 0)
			{
				parentfileaddr = G_CurrentDfAddr[G_ChannelNum];
			}
			else
			{
				parentfileaddr = GetParentOrMapFileAddr(parentfileaddr,F_ParentName_Offset);
			}
			
			dirNum = GetFileHeadByte(parentfileaddr,F_DIRNum_offset);
			
			if(dirNum != 0)
			{
				return parentfileaddr;
			}
			
			if(parentfileaddr == E_FILE_START)
			{
				break;
			}
		}
	}
	
	return FALSE;

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
 * @history 	      : V1.1: 增加ADM4- ADM10的支持，初始需求：由于NFC2.0 SWP卡要求AC 应用下文件的更新权限为ADM10 modify by xuyz      
 */
void VerifyChangeUnblockChv(void)
{
	BYTE idata oprateMode;//, tempResult;
	KeyStructer xdata pinKey;
	PukStructer	xdata pukInfo;
	UWORD xdata  tempResult;
	

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
			InsError();
			return;
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
			if(P2==0x00)  //支持PIN1 PIN2  ADM1-ADM10
			{
				P1P2Error();	                                            //6B 00
				return;
			}
			
			//ReadEeprom(&lteCardCfg,CARDCFG_LIANTONGATR, 0x01);	    //联通测试判断
			
			//if(lteCardCfg == SPECIAL_CHINAUNICOM)


			if(MOBILE_Operator == OPERATOR_CHINAUNICOM)
			{
				if(P2 >0x05)
				{
					P1P2Error();	                                            //6B 00
					return;
				}
			}
			else if(MOBILE_Operator == OPERATOR_CHINAMOBILE)
			{
				if(((P2&0x7F) >= 0x03) &&( (P2 &0x7F) <= 0x0E) )  //移动java卡要求指令，A020000308 与 A020000A08都为校验ADM1的指令
				{
					if(((P2&0x7F) == 0x08) || ( (P2 &0x7F) == 0x09) )
					{
						P1P2Error();	                                            //6B 00
						return;
					}
				}
				else
				{
					if(P2 > 0x02)
					{
						P1P2Error();	                                            //6B 00
						return;
					}
				}
			}
			else
			{
				if(((P2&0x7F) >= 0x04) &&( (P2 &0x7F) <= 0x0E) )  //海外同理按照移动操作，A020000408 与 A020000A08都为校验ADM1的指令
				{
					if( (P2 &0x7F) == 0x09) 
					{
						P1P2Error();	                                            //6B 00
						return;
					}
				}
				else
				{
					if(P2 > 0x02)
					{
						P1P2Error();	                                            //6B 00
						return;
					}
				}			
			}
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
#if UICC__FUNCTION__UniversalPin
		if(((P2&0x1F) > 0x0E)&&(P2 != 0x11))//CHV ||ADM :01 - 0E,根据客户需求，profile中ADM的属性也可以配置为change，disable,也可以进行相应的操作								
#else
		if((P2&0x1F) > 0x0E)
#endif		
		{
			P1P2Error();                                                													
			return;
		}	
	}


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

			if((G_CommandMode == Command_Mode_3G)&&(oprateMode == mode_Unblock)&&(P3 == 0x00))	 //unblock  P3 == 0x10,0x00
			{
			}
			else	    //change   P3 = 0x10
			{
			      P3Error();                                         // 67 00																//67 00
			      return;				
			}
		} 
	}

	if(G_CommandMode == Command_Mode_2G)
	{
		if((oprateMode == mode_Verify) && (P2 > 0x02))
		{

			if(MOBILE_Operator == OPERATOR_CHINAMOBILE)
			{
				if((P2&0x7F) <= 0x07)
				{
					P2 = P2 + 0x07; //ADM1 - ADM10: 0A 0B 0C 0D 0E 8A 8B 8C 8D 8E
				}
			}
			else
			{
				if(P2 == 0x03)
				{
					 P1P2Error();                                        														//67 00
					 return;
				}

				if((P2&0x7F) <= 0x08)
				{
					P2 = P2 + 0x06; //ADM1 - ADM10: 0A 0B 0C 0D 0E 8A 8B 8C 8D 8E
				}
			}

		}

		if((oprateMode == mode_Unblock) && (P2 == 0x00))
		{
			P2 = 0x01;
		}

#if (EUICC_ONLY || SE_AND_EUICC)
		if(getADFAddr() != FALSE)
#endif
		{
			if(P2 == 0x02)
			{
				P2 = 0x81;										//PIN2
			}	
		}			
	}

	ReceiveData();

	tempResult = ReadKey(P2,(BYTE*)&pinKey.KeyId);

	if(tempResult == FALSE )                                                     //the key not find 
    {
#if UICC__OPERATOR__CHINAUNICOM     //联通测试 华弘测试脚本/RFM(SMS)/UICC_PIN相关/步骤07  002000880831323334FFFFFFFF  期望6B00
					
			P1P2Error(); 
#else
			ReferencedDataNotFound();											//6A 88
#endif	
	
			return;
	}
	
	ReadOrWritePukInfo(pinKey.pukRef,(BYTE*)&pukInfo.PukId,Read_Key);
	

 	if(P3 == 0x00)
	{
		if(oprateMode == mode_Verify)
		{
			PinRetryCounter(pinKey.KeyLimit);	     //63cx
		}
		else  //oprateMode == mode_Unblock
		{
			PinRetryCounter(pukInfo.PukLimit);		 //63cx
		}
		return;	
	}

	
	//bit2:Set to 1: PIN can be changed    Set to 0: PIN change forbidden
	if((oprateMode == mode_Change) && ((pinKey.KeyStatus & 0x02) == 0))
	{
		SecuritySatisfied();
		return;
	}

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

		tempResult = memcmp_p((BYTE xdata *)&pinKey.KeyContent[0],RecData,0x08);
	}
	else		//mode_Unblock
	{
		if((pukInfo.PukLimit&0x0F)  == PIN_BLOCKED)
		{
			PinBlocked();													   //69 83
		
			return;
		}
		pukInfo.PukLimit= pukInfo.PukLimit- 1;		    	//记数器减1?
	
		tempResult = memcmp_p((BYTE xdata *)&pukInfo.PukContent[0],RecData,0x08);
	}	
	
	if(tempResult != EQUAL)
	{
		if(oprateMode == mode_Unblock)
		{									
			if(G_CommandMode == Command_Mode_2G)
			{
				if( (pukInfo.PukLimit&0x0F) == PIN_BLOCKED)                                      //pin blocked 
				{
					PinBlocked();                 //98 40
				}
				else
				{
					SecuritySatisfied();
				}
			}

			else
			{
				PinRetryCounter(pukInfo.PukLimit);										//63 CX
			}

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

				else 
				{
					if(oprateMode == mode_Verify)
					{
						pinKey.KeyStatus = pinKey.KeyStatus | PIN_ENABLED;           
					}
				}

			}
			else 
			{
				SecuritySatisfied();
			}

			if(G_CommandMode == Command_Mode_3G)
			{
				PinRetryCounter(pinKey.KeyLimit);								//63 CX 
			}
			
		}
		tempResult = WriteKey(P2,(BYTE xdata *)&pinKey.KeyId);					//记录回写
		
		ReadOrWritePukInfo(pinKey.pukRef,(BYTE*)&pukInfo.PukId,Write_Key);

		if(tempResult == FALSE)
		{
			MemoryError();													//65 81/92 40

			return;
		}
  	
	}
	else
	{
		if(oprateMode == mode_Verify)  //modify @20140714,为 减少断电时 重新更新PIN所在页，若当前为第一次校验且校验成功，则不进行PIN 文件的更新
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
			pukInfo.PukLimit = KeyResume(pukInfo.PukLimit);

			pinKey.KeyStatus |= PIN_ENABLED ;
		}

		pinKey.KeyLimit = KeyResume(pinKey.KeyLimit);
		
	 	ConfigureKeyStatus(P2,SetStatusMode);  //设置当前key的状态为已校验

		if(oprateMode != mode_Verify)
		{
			ecpMem_Copy((BYTE xdata *)&pinKey.KeyContent[0],(BYTE xdata *)&RecData[8],0x08);
		}

		tempResult = WriteKey(P2,(BYTE xdata *)&pinKey.KeyId);					//记录回写
		
		ReadOrWritePukInfo(pinKey.pukRef,(BYTE*)&pukInfo.PukId,Write_Key);

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
 * @history 	 :V1.0 : modify by zhxx 20220419, if((pinKey.KeyStatus & 0x04) == 0)，电信测试和eUICC_Profile_Package_Interoperable_Format_Test_V_2_3 返回的状态字使用配置
 */
void ChvHandle(void)
{ 
	BYTE idata tempResult;
	BYTE idata oprateMode;
	KeyStructer xdata pinKey;
  
	oprateMode = ((INS == 0x26)?PIN_DISABLED:PIN_ENABLED);

	if(P1 != 0x00)
	{
		if((G_CommandMode == Command_Mode_3G)&&(oprateMode == PIN_DISABLED))
		{
#if UICC__OPERATOR__CHINATELECOM
			//电信测试-东信5G 新增功能测试log(WD 工具)\WorkDirecory\rfm/func/cmd/uicc/T_DisablePIN_PORSubmit 期望值：016B00/016A86
			P1P2Error();		
#else
			FunctionNotSupport();										//6A 81	
#endif			
		}
		else
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
	else	//Command_Mode_3G
	{
		if((P2 == 0x00)) 
		{
			 FunctionNotSupport();													//6A 81
			 return;		
		}
		
		//ReadEeprom(&tempResult,CARDCFG_LIANTONGATR, 0x01);	    //联通测试判断
#if (EUICC_ONLY || SE_AND_EUICC)
		if(MOBILE_Operator == OPERATOR_CHINAUNICOM)
#endif
		{
		
			if((P2&0x80) == 0x80)
			{
				P1P2Error();             //联通不允许对PIN2进行disable
				return;
			}
		}


#if (EUICC_ONLY || SE_AND_EUICC)
#if UICC__FUNCTION__UniversalPin		
		if( (((P2&0x1F) > 0x0E)&&(P2 != 0x11))||((P2&0x60) != 0x00))//CHV ||ADM :01 - 0E,根据客户需求， profile中ADM的属性也可以配置为change，disable,也可以进行相应的操作		
#else
		if( ((P2&0x1F) > 0x0E)||((P2&0x60) != 0x00))//CHV ||ADM :01 - 0E,根据客户需求， profile中ADM的属性也可以配置为change，disable,也可以进行相应的操作		
#endif		
#else
#if UICC__FUNCTION__UniversalPin	
		if( (((P2&0x1F) > 0x08)&&(P2 != 0x11))||((P2&0x60) != 0x00))//UICC补充测试不允许disable adm
#else		
		if( ((P2&0x1F) > 0x08)||((P2&0x60) != 0x00))//UICC补充测试不允许disable adm
#endif		
#endif
		{
			P1P2Error();															//6B 00
			return;
		}                                                      
	}

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
	
	
	//pinstatus  bit3:
	//Set to 1: PIN state change allowed
	//Set to 0: PIN state change not allowed
	if((pinKey.KeyStatus & 0x04) == 0)                                                    
    {
		#if SE_ONLY  //modify by zhxx@0816，代码评审修改，SE和EUICC用宏隔开，EUICC修改不影响SE卡片，而且节省空间
		    ChvContradiction();
		#else
			if(MOBILE_Operator == OPERATOR_CHINATELECOM)
			{
				 ChvContradiction();//modify by chentao@20211122 中国电信离线脚本测试--pofile结构测试库V2.3.1\\data\04-PINAttributes\\data_04_PINAttributes_05  期望返回6984
			}
			else//eUICC_Profile_Package_Interoperable_Format_Test_V_2_3/TC_8_2_5_7_Checking_the_PIN_state_change_allowed_and_PIN_state_change_not_allowed_status
			{
				SecuritySatisfied();
			}
		#endif			
	
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

//	tempResult = ecpMem_Compare((BYTE xdata *)&pinKey.KeyContent[0],(BYTE xdata *)&RecData[0],P3);

	if ( ecpMem_Compare((BYTE  *)&pinKey.KeyContent[0],(BYTE  *)&RecData[0],P3) == EQUAL )
	{
		pinKey.KeyStatus = (pinKey.KeyStatus & 0x06) | oprateMode ;    //bit3，bit2不变，只改变bit1的状态                                    
							
		ConfigureKeyStatus(P2,SetStatusMode);  //设置当前key的状态为已校验	  	//	chv_1_enabled = PIN_ENABLED ;chv_1_verified = TRUE ;

		pinKey.KeyLimit = KeyResume(pinKey.KeyLimit);
		 
		tempResult = WriteKey(P2,(BYTE xdata *)&pinKey.KeyId);					//记录回写

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
		
		/*If the PIN presented is false, the number of remaining PIN attempts shall be decremented and PIN remains disabled.
			After 3 consecutive false PIN presentations, not necessarily in the same card session, PIN shall be blocked and may
			optionally be set to "enabled".*/
		if(UICC__DONOT_ENABLEPIN_WHEN_BLOCK)
		{
		}
		else
		{
			if((G_CommandMode == Command_Mode_3G)&&(oprateMode == PIN_ENABLED)&&((pinKey.KeyLimit&0x0F) == PIN_BLOCKED))
			{
					pinKey.KeyStatus |= PIN_ENABLED;
					ConfigureKeyStatus(P2,ClearStatusMode); 
			}
		}

		tempResult = WriteKey(P2,(BYTE xdata *)&pinKey.KeyId);					//记录回写
				
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
		else
		{
			PinRetryCounter(pinKey.KeyLimit);			//63 CX										 
		}
   	}       
}

