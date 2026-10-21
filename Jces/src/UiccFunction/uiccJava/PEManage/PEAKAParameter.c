
/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   PEAKAParameter.c
*
* Description: 该文件主要是对PE-AKAParameter的所有处理
*
* auther：     zxx    20170623
*
* history：    V1.0
*
************************************************************************************************************/


#include  "typedef.h"
#include  "fileAction.h"
#include "tlvHeader.h"
#include "profileApi.h"
#include  "ProfileInterpreter.h"
#include  "gsmpublic.h"
#include  "fileSystem.h"
#include  "uicc_main.h"
#include  "VMAccessEeprom.h"
#include  "opFunction.h"
#if (EUICC_ONLY||SE_AND_EUICC)
/*********************PE-AKAParameter START*****************************/
//	BYTE code  theAKAParameterPEAlgoTag[] = {0xA1};
//	
//BYTE code  mappingParameterTag[] = {0xA1,0xA0};
//	//OCTET STRING (SIZE(1)),
//BYTE code  mappingOptionsTag[] = {0xA1,0xA0,0x80};
//	
//	//ApplicationIdentifier
//BYTE code  mappingSourceTag[] = {0xA1,0xA0,0x81};		
//	
//BYTE code  algoParameterTag[] = {0xA1,0xA1};
//	// milenage,
//BYTE code  algorithmIDTag[] = {0xA1,0xA1,0x80};
//	//'01'H,      -- RES and MAC 64 bits, CK and IK 128 bits 
//BYTE code  algorithmOptionsTag[] = {0xA1,0xA1,0x81};

//	//'000102030405060708090A0B0C0D0E0F'H,
//BYTE code  keyTag[] = {0xA1,0xA1,0x82};
//	//'0102030405060708090A0B0C0D0E0F00'H,
//BYTE code  opcTag[] = {0xA1,0xA1,0x83};
//	//uses default: '4000204060'H,
//BYTE code  rotationConstantsTag[] = {0xA1,0xA1,0x84};  
//	//uses default: '00000000000000000000000000000001000000000000000200000000000000040000000000000008'H,
//BYTE code  xoringConstantsTag[] = {0xA1,0xA1,0x85};
//	//'010203'H
//BYTE code  authCounterMaxTag[] = {0xA1,0xA1,0x86};	
//	
//	//OCTET STRING (SIZE(1)) DEFAULT '02'H
//BYTE code  theAKAParameterPESqnOptionsTag[] = {0x82};
//	//OCTET STRING (SIZE(6)) DEFAULT '000010000000'H,
//BYTE code  theAKAParameterPESqnDeltaTag[] = {0x83};		
//	//OCTET STRING (SIZE(6)) DEFAULT '000010000000'H,
//BYTE code  theAKAParameterPESqnAgeLimitTag[] = {0x84};	
//	//SEQUENCE (SIZE (32)) OF OCTET STRING (SIZE (6)) DEFAULT
//BYTE code  theAKAParameterPESqnInitListTag[] = {0xA5};	//TODO
//	
//BYTE code  theAKAParameterPESqnInitTag[] = {0xA5,0x04};	//TODO

	/*********************PE-AKAParameter END*****************************/


	

//注：以下所有未编码的bit表示RFU

//algorithm-Options的编码
//Bit8 Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 
//       |    |    |    |   |____|____|________RES size(0: 32 bits ,1: 64 bits, 2:128 bits, 3: 256 bits)            
//       |    |    |    |     
//       |    |____|____|______________________MAC-A and MAC-S size (0: 64 bits ,1: 128 bits, 2: 256 bits)
//       |
//       |_____________________________________CK and IK size (0: 128 bits ,1: 256 bits)


//sqnOptions的编码
//  Bit8 Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 
//                       |    |    |    |______Anonymity Key (AK) (1: not used, 0: used)
//                       |    |    |___________SQN wrap around (1: not allowed, 0: allowed) In case SQN wrap around is allowed it means that SQN verification will be disabled if the respective SQN value has reached the maximum value 07FFFFFFFFFF
//                       |    |________________SQN Delta (1: not used, 0: used)
//                       |_____________________SQN Age Limit (1: not used, 0: used)
//

//mappingOptions的编码
// Bit8 Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 
//                           |____|____________00: dot not share SQN parameters and/or values
//                                             01: share sqnInit, sqnOptions, sqnDelta, sqnAgeLimit 
//                                             10: share sqnOptions, sqnDelta, sqnAgeLimit
	
	
/**
*modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/	
void setAlgorithmCfg(BYTE AlgorithmId)
{	
	VMWriteBuf((G_fileControlContext.LoadADFAddr + F_AlgID_Offset),(BYTE*)&AlgorithmId,sizeof(BYTE));//将算法ID存储到ADF头中的RFU字节处
}
	

/*
*@function    :PEProcessMappingParameter
*
*@description :处理PEAKA组件中的MappingParameter,此组件暂时只封装了结构，没进行任何实际的操作
*
*@see         :<eUICC Profile Package Interoperable Format Technical Specification v2.0.pdf> SECTION8.4.2

*				MappingParameter ::= SEQUENCE 
				{ 
					  mappingOptions OCTET STRING (SIZE(1)), 
					  mappingSource ApplicationIdentifier 
				}
*
*@param       :void
*
*@return      void
*
*@author     :xuyz
*
*@history    :V1.0 :create by xuyz@20170726
*            :V1.1 :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE PEProcessMappingParameter(void)
{
	BYTE idata result,mappingOptions,algorithmID,num;
	UWORD xdata adfAddr,fileAddr,fileAddr1;
	WORD idata adfId;
	BYTE idata fileRecNumLgth[2];
	
	mappingOptions = 0;
	
	algorithmID = 0;
	
	setVerticalFind(&G_PETlvInfo);

	while(1)
	{
		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
			break;  //处理到最后一个tag
		}
		switch(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex])
		{
			case 0x80:   //mappingOptions OCTET STRING (SIZE(1)),	
				if(G_PETlvInfo.findValueLen != 0x01)
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
				
				mappingOptions = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
				
				if(mappingOptions == 0xFF)  //modify by chentao@20230301 mappingOptions 无意义的bit位忽略，不做异常检查，提高COS兼容性
				{
					if(MOBILE_Operator == OPERATOR_CHINATELECOM)
					{
						appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
						return FALSE;
					}
				}
				break;
			
			case 0x81:   //mappingSource ApplicationIdentifier	
				
			    if(AIDLenInvalid(G_PETlvInfo.findValueLen))
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
			    
				tempBuf[0]	= G_PETlvInfo.findValueLen;
				VMReadBuf(&tempBuf[1], G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen);	
	
				if(GetAIDNumber(&tempBuf[0],TRUE) == FALSE)
				{
					//ERROR  此处需要判断当前profile是否存在该ADF AID，如果不存在则报错
					appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
					return FALSE;	
				}
				
				
				//获取映射文件的ADF地址						
				adfAddr = SelectFileByName(G_CreatingMFOffset,&tempBuf[1],tempBuf[0]);
				
				if(adfAddr == FALSE)//modify by zhxx@20230314,esim代码走查修改，如果映射文件不存在的话，抛错
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}

				VMReadBuf((BYTE xdata *)&adfId,adfAddr,sizeof(WORD));
												
				//The algorithmID, algorithmOptions, key, opc, rotationConstants, xoringConstants and numberOfKeccak are always shared when the mapping is used.
				algorithmID = getAlgorithmID(adfAddr);//获取映射文件的algorithmID				
				setAlgorithmCfg(algorithmID);//设置当前ADF的algorithmID				
				
				//modifyFileMapping(EF_KI);	//将KI文件映射到mapping文件

				//modifyFileMapping(EF_AuthMaxCounter); //将counter文件映射到mapping文件
				num = 2;
				
				mappingOptions &= 0x06;
			  /*00: dot not share SQN parameters and/or values
				02: share sqnInit, sqnOptions, sqnDelta, sqnAgeLimit
				04: share sqnOptions, sqnDelta, sqnAgeLimit
				06: share sqnOptions, sqnDelta, sqnAgeLimit and SQN array
				*/
				if(mappingOptions != 0)
				{
					//modifyFileMapping(EF_PRIVATE_AUTHCFG); //将配置文件映射到mapping文件
					
					num ++;
                    if(mappingOptions == 6)//SQN array是映射文件
					{						
						//modifyFileMapping(EF_SQNCheck);		
                        num	++;					
					}
						
				}
				
				if(createSpecProfilePrivateFile(FALSE,num,adfId) == FALSE)
				{
					return FALSE;
				}
				
				
				if(mappingOptions != 6)//鉴权配置参数一部分从映射文件拷贝，一部分使用自己的
				{
					fileAddr = SelectFile(adfAddr, EF_PRIVATE_AUTHCFG);								
					fileAddr1 = SelectFile(G_fileControlContext.LoadADFAddr, EF_PRIVATE_AUTHCFG);	//将映射文件的AUTH配置数据写入当前ADF的鉴权配置文件中
					ReadTFBuf(fileAddr, 0x00, &tempBuf[0], GetTFSize(fileAddr));
					WriteTFBuf(fileAddr1,0x00,&tempBuf[0],GetTFSize(fileAddr));
										
				}
				
				if(mappingOptions == 2)//sqnInit从映射文件拷贝
				{
					fileAddr = SelectFile(adfAddr, EF_SQNCheck);
					fileAddr1 = SelectFile(G_fileControlContext.LoadADFAddr, EF_SQNCheck);

					ReadEeprom((BYTE idata *)&fileRecNumLgth[0],fileAddr+F_RecNum_Offset,sizeof(fileRecNumLgth));

					VMCopyDataToAddr(fileAddr1+sizeof(RecordFileHead),fileRecNumLgth[0]*fileRecNumLgth[1],fileAddr+sizeof(RecordFileHead),fileRecNumLgth[0]*fileRecNumLgth[1],MODE_PROTECT_WRITE);
				}

				break;
			
			default:
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
		}
	}
	
	setReturnFind(&G_PETlvInfo);
	
	return algorithmID;
}




/*
*@function: PEProcessAlgoParameter
*
*@description:
*
*@param    :void
*
*@return   :void
*
*@author   :xuyz
*
*@history  :V1.0 create by xuyz@20170725
            V1.01 modify by zhhj@20180614 增加对TUAK的支持

pe.algoConfiguration = new AlgoConfigurationChoice(1);
	pe.algoConfiguration.algoParameter = new AlgoParameter();
	pe.algoConfiguration.algoParameter.algorithmID = "02"; //80 tag
	pe.algoConfiguration.algoParameter.algorithmOptions = "00"; //81 tag
	pe.algoConfiguration.algoParameter.key = "abababababababababababababababab";  //82 tag
	pe.algoConfiguration.algoParameter.opc = "bd04d9530e87513c5d837ac2ad954623a8e2330c115305a73eb45d1f40cccbff"; //83tag
	pe.algoConfiguration.algoParameter.authCounterMax = "010203";//86 tag
*modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE PEProcessAlgoParameter(void)
{
	BYTE idata result,limit;
	BYTE idata algorithmID,algorithmOptions;
	BYTE xdata temp[4];
	
	algorithmID = 0;	
		
	if(createSpecProfilePrivateFile(FALSE,0,0) == FALSE)
	{
		return FALSE;
	}
	
	ecpMem_Set(temp,0,sizeof(temp));
	
	setVerticalFind(&G_PETlvInfo);

	while(1)
	{
		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
			break;  //处理到最后一个tag
		}
		switch(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex])
		{
			case 0x80:   //algorithmID milenage(1),  tuak(2),usim-test-algorithm(3)
			
				if(G_PETlvInfo.findValueLen != 0x01)
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
				
				algorithmID = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);//milenage(1), tuak(2), usim-test-algorithm(3),私有鉴权(0x0F)
				
				if((algorithmID != 1) && (algorithmID != 2) && (algorithmID != 3) && (algorithmID != 0x0F))
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}

				setAlgorithmCfg(algorithmID);
				temp[0] = 1;
				break;
			
			case 0x81:   //algorithmOptions OCTET STRING (SIZE(1)),
				
				if(G_PETlvInfo.findValueLen != 0x01) 
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
				
				algorithmOptions = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
			
				setAlgorithmOptions(algorithmOptions);
				
				temp[1] = 1;
				break;
			case 0x82:  //key OCTET STRING,
				VMReadBuf(&tempBuf[0],  G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen);
			
			  //Key size: The key OBJECT STRING shall have a length of 16 bytes in case of the Milenage or usim-test-algorithm 
			  //and 16 or 32 bytes in case of the TUAK algorithm.
				if((algorithmID == 0x01) || (algorithmID == 0x03)) 
				{
					if(G_PETlvInfo.findValueLen != 0x10) 
					{
						appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
						return FALSE;
					}
				}
				
			#if RSP_EUICC
				if(IF_CustomerCheckTestProfile() == FALSE) //该函数请勿注释
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
			#endif				
						
				setKI(PROFILEDOWNMODE,&tempBuf[0], G_PETlvInfo.findValueLen); //写入TUAK,milenage,及XOR的KI
				
				temp[2] = 1;
				break;
			
			case 0x83:  // opc OCTET STRING, -- OPc for Milenage; TOPc for TUAK
				
				VMReadBuf(&tempBuf[0],  G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen);
			
			   //OPc size: The opc OBJECT STRING shall have a size of 16 bytes in case of the Milenage and 32 bytes in case of the TUAK algorithm.
				if(algorithmID == 0x01) //opc OCTET STRING, 
				{
					if(G_PETlvInfo.findValueLen != 0x10)
					{
						appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
						return FALSE;
					}
				} //TOPc for TUAK
				else if(algorithmID == 0x02) 
				{
					if(G_PETlvInfo.findValueLen != 0x20)
					{
						appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
						return FALSE;
					}
				}
				
				WriteAuthCfgData(OFFSET_AUTHCFG_OPC,&tempBuf[0],G_PETlvInfo.findValueLen); //TUCK 和MILENGE 偏移相同
				
				temp[3] = 1;
				break;
			case 0x84:  //rotationConstants OCTET STRING (SIZE (5)) DEFAULT '4000204060'H,
				///* rotationConstants only apply for Milenage; ignored in case of TUAK and usim-test-algorithm */
			  if(G_PETlvInfo.findValueLen != 0x05)
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
				if(algorithmID==0x01)
				{
					VMReadBuf(&tempBuf[0],  G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen);
				
					WriteAuthCfgData(OFFSET_AUTHCFG_R,&tempBuf[0],G_PETlvInfo.findValueLen);
				}
			
				break;
			case 0x85:  //xoringConstants OCTET STRING (SIZE (80)) DEFAULT
			/* xoringConstants only apply for Milenage; ignored in case of TUAK and usim-test-algorithm */
				if(G_PETlvInfo.findValueLen != 80)
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
			    if(algorithmID==0x01)///* xoringConstants only apply for Milenage; ignored in case of TUAK and usim-test-algorithm */
				{
					VMReadBuf(&tempBuf[0],  G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen);
			
					WriteAuthCfgData(OFFSET_AUTHCFG_C,&tempBuf[0],G_PETlvInfo.findValueLen);
				}				
			
				break;
			case 0x86:  //authCounterMax OCTET STRING (SIZE(3)) OPTIONAL
				
				if(G_PETlvInfo.findValueLen != 0x03)
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
				
				VMReadBuf(&tempBuf[0],  G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, 3);
			
				if((algorithmID==1)||(algorithmID==2))//milenage,TUCK
				{
						setAuthCounterMax(&tempBuf[0], G_PETlvInfo.findValueLen);
				}

				break;
			case 0x87:  //numberOfKeccak UInt8 DEFAULT 1 
				/* Number of iterations of Keccak-f[1600] (noted: П ) permutation as recommended by 3GPP TS 35.231 [TUAK] in section 7.2. 
            This parameter only applies for TUAK; ignored otherwise.*/
			
				if(G_PETlvInfo.findValueLen != 0x01)
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
				
				if(algorithmID == 2)// This parameter only applies for TUAK; ignored otherwise.
				{
					tempBuf[0] = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
			    
					WriteAuthCfgData(OFFSET_TUAK_AUTHCFG_KIITA,&tempBuf[0],1);
				}
						
				break;
                            
			default:
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
		}
	}
	
	if(algorithmID == 3)///* OPc for Milenage; TOPc for TUAK; ignored in case of usim-test-algorithm */
	{
		limit = 3;
	}
	else
	{
		limit = 4;
	}
	
	for(result = 0; result < limit;result++)
	{
		if(temp[result] == 0)
		{
			appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
			return FALSE;
		}
	}
	
	setReturnFind(&G_PETlvInfo);
	
	return algorithmID;
}
/*
*@funtion    :AKAParameterPEProcess
*
*@description: 对PE-AKAParameter进行解析处理
*
*@param      :
*             
*@return     :BYTE    --TRUE:继续执行
*                     --FALSE:终止执行，直接返回
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE AKAParameterPEProcess(void)
{
	BYTE idata result;
	BYTE idata sqnOption,recNum;
	BYTE idata algorithmID;

	if(ValueBit(G_PEControlContext.IsExistFlag2,PE_Complete) == FALSE)//不够一个PE，将整个数据都备份
	{
		backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr,  G_PETlvInfo.bufferLen);	
		
		return TRUE;
	}	
		
	algorithmID = 0;
	
	sqnOption = 2;//OCTET STRING (SIZE(1)) DEFAULT '02'H, /* ignored in case of usim-test-algorithm */
		
	while(1)
	{
		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
			break;  //处理到最后一个tag
		}
		
		if(G_PETlvInfo.tagBuffer[0] != 0xA0)
		{
			if( G_PEControlContext.TagA0Occ == 0)
			{
				appendErrorResponse(INVALID_PARAMETER,0);
				return FALSE;
			}	
		}
		
		switch(G_PETlvInfo.tagBuffer[0])
		{
			case 0xA0:
				if(CheckTagOcc(G_PEControlContext.TagA0Occ++) == FALSE)
				{
					return FALSE;
				}
				if (PEHeaderAnalyze()== FALSE)  //处理PEHeader信息
				{
					return FALSE;
				}
				break;
			
			case 0xA1:      //algoConfiguration
				
				setVerticalFind(&G_PETlvInfo);
			
				//modify by chentao20220422 TY_OfflineProfileFormatTest/Generic_File_management异常测试/Generic_File_management_E03，
				//该用例通用方式创建ADF时，没有84 tag，导致G_fileControlContext.LoadADFAddr为FALSE，导致setAlgorithmCfg时，卡片无响应
				if((G_fileControlContext.LoadDFAddr == G_CreatingMFOffset) || (G_fileControlContext.LoadDFAddr == FALSE)||(G_fileControlContext.LoadADFAddr == FALSE))//PE-AKAParameters is not allowed in the context of MF, SDs and applications
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
								
					return FALSE;	
				}
			
				while(1)
				{
					result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
					
					if(result == FALSE)
					{
						break;  //处理到最后一个tag
					}
					switch(G_PETlvInfo.tagBuffer[1])
					{
						case 0xA0:   //MappingParameter,
							
						    algorithmID = PEProcessMappingParameter();
							
						    if(algorithmID == FALSE)
							{
								return FALSE;
							}
						
							break;
						
						case 0xA1:   //AlgoParameter	

                            algorithmID	= PEProcessAlgoParameter();	
						
							if(algorithmID == FALSE)
							{
								return FALSE;
							}
						
							break;						
						default:
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
							return FALSE;
							//break;
					}
				}
				
				setReturnFind(&G_PETlvInfo);
				
				break ;
				
			case 0x82:   //sqnOptions OCTET STRING (SIZE(1)) DEFAULT '02'H
				if(G_PETlvInfo.findValueLen != 0x01)
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
				sqnOption = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
				
				setAuthCfg(sqnOption); 
                         
				break;
				
			case 0x83:   //sqnDelta OCTET STRING (SIZE(6)) DEFAULT '000010000000'H,
				if(G_PETlvInfo.findValueLen != 0x06)
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
				VMReadBuf(&tempBuf[0],  G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen);	
			
				if((sqnOption&0x04) != 0x04)//SQN Delta (1: not used, 0: used)
				{
					setSqnDelta(&tempBuf[0] , 6,algorithmID);
				}
			
				break;
			
			case 0x84:   //sqnAgeLimit OCTET STRING (SIZE(6)) DEFAULT '000010000000'H,
				if(G_PETlvInfo.findValueLen != 0x06)
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
				}
				VMReadBuf(&tempBuf[0],  G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen);			
				
				if((sqnOption&0x08) != 0x08) //SQN Age Limit (1: not used, 0: used)
				{
					//setSqnAgeLimit(&tempBuf[0], 6);
					if(algorithmID == 1)//milenage
					{
						WriteAuthCfgData(OFFSET_AUTHCFG_L,&tempBuf[0],6); 
					}
					else if(algorithmID == 2)//TUAK
					{
						WriteAuthCfgData(OFFSET_TUAK_AUTHCFG_L,&tempBuf[0],6); 
					}
					else
					{
						
					}	
				}							
			
				break;
			case 0xA5:   //sqnInittag  LIST: a5 04
				setVerticalFind(&G_PETlvInfo);
			
			    recNum = 0;
			
			    while(1)
				{
					result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
					
					if(result == FALSE)
					{
						break;  //处理到最后一个tag
					}
					switch(G_PETlvInfo.tagBuffer[1])
					{
						case 0x04:   //sqnInit			
                            if(G_PETlvInfo.findValueLen != 6)
							{
								appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
								return FALSE;
							}								
						    recNum++;						  
						    VMReadBuf(&tempBuf[0], G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen);			
						    setSqnInit(&tempBuf[0],G_PETlvInfo.findValueLen,recNum);
							break;
											
						default:
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
							return FALSE;
							//break;
					}
				}
				
			
			    setReturnFind(&G_PETlvInfo);
				break;
			
			default:				
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
		}
	}
	
	return TRUE;
}
#endif

