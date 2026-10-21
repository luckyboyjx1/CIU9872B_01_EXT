/**
 * @file 	cdmaBasefunction.c
 *
 * @brief 	Copyright (c) 2002, eastcompeace
 *
 * 		All rights reserved.
 *
 * 		MODEL: EM TG96-3G/TCG96-3G, EMOSYN, ATMEL, SAMSUANG, INFINEON, RENESAS, SOLOMON
 *
 * 		MOTOCOS V4.03
 *
 * 		PHASE 2  RELEASE:2003-2-25
 *
 * @author 	蔡勃
 *
 * @see 	<3GPP2 C.S0023-C v1.0> 		Chapter 3.4.52/3.5.3/4.6/4.7/4.8
 *		<3GPP2 C.S0016-C>（即TIA-683-C） 	Chapter 3.5.8.12/3.5.8.14
 *		<中国联通800MHz CDMA数字蜂窝移动通信网UIM卡技术要求（V 2.0）> 6.2节
 *
 * @note	
 *
 * @history 	
 *			
 			
 */
#include <string.h>
#include "uicc_main.h"
#include "stkhead.h"
#include "fileAction.h"
#include "fileSystem.h"
#include "error.h"
#include "gsmpublic.h"
#include "cdma.h"

#if UICC__AUTH__CDMA__CAVE
void InitCDMAVariable(void)
{
	SAVED_OFFSET_1 = 0;                           
	SAVED_OFFSET_2 = 0;                                 
	ecpMem_Set(SAVED_DATA,0x00,sizeof(SAVED_DATA));
	ecpMem_Set(ESN,0x00,sizeof(ESN));
	ecpMem_Set(LFSR,0x00,sizeof(LFSR));
	ecpMem_Set(SAVED_LFSR,0x00,sizeof(SAVED_LFSR));
	ecpMem_Set(SAVED_RAND,0x00,sizeof(SAVED_RAND));
	ecpMem_Set(SSD_A_NEW,0x00,sizeof(SSD_A_NEW));
	ecpMem_Set(SSD_B_NEW,0x00,sizeof(SSD_B_NEW));
	ecpMem_Set(MIN1,0x00,sizeof(MIN1));
	ecpMem_Set(MIN2,0x00,sizeof(MIN2));
	ecpMem_Set(RANDBS,0x00,sizeof(RANDBS));
	ecpMem_Set(UpdateSSDForESN,0x00,sizeof(UpdateSSDForESN));
	ecpMem_Set(Register,0x00,sizeof(Register));
	ecpMem_Set(MIP_RRQ_Hash_Buf,0x00,sizeof(MIP_RRQ_Hash_Buf));
	AuthBs = 0;                                  
	G_CdmaStatus   = 0;                                                 
#if UICC__AUTH__CDMA__MD5	
	Nai_Index = 0;  
#endif	
	G_P1 = 0;                                           
	G_P2[0] = 0;                                           
	G_P2[1] = 0;
}
/**
 * @brief 	This function is to init the ESN, MIN1, MIN2
 * 		确定CAVE算法使用ESN还是UIMID
 *
 * @Returns	TRUE/FALSE
 *
 * @author 	
 *
 * @history 	created by
 */
BYTE InitCave(void)
{
	UWORD xdata  fileAddr;
	BYTE  idata  flag;
#if(!UICC__CDMA_2G_NOT_SUPPORT)
	fileAddr = SelectTFFile(DF_CDMA, EF_UIMID);//read EF_USGIND(UIM_ID/SF_EUIMID Usage Indicator) 0x6F42
#else
	fileAddr = SelectTFFile(G_DF_ID_CSIM, EF_UIMID);//read EF_USGIND(UIM_ID/SF_EUIMID Usage Indicator) 0x6F42
#endif
	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadTFBuf(fileAddr, 0x00, (BYTE *)&flag, 1);

	if ((flag&0x01) == 0x01)
	{
#if(!UICC__CDMA_2G_NOT_SUPPORT)	
		fileAddr = SelectTFFile(DF_CDMA, EF_RUIMID);//read from UMID_ 6F31
#else
		fileAddr = SelectTFFile(G_DF_ID_CSIM, EF_RUIMID);//read from UMID_ 6F31
#endif		
	}
	else
	{
		// <C.S0023-C v1.0> 3.4.32 4.2.1 4.4.1
		// 若ME为MEID，且EF_6F42 bit0、bit2都为 0，则不使用EF_6F38里的ESN，
		// 应当使用UpdateSSD中的ESN
#if(!UICC__CDMA_2G_NOT_SUPPORT)		
		fileAddr = SelectTFFile(DF_CDMA, EF_ESNME);//read from ESN_ME 6F38
#else
		fileAddr = SelectTFFile(G_DF_ID_CSIM, EF_ESNME);//read from ESN_ME 6F38
#endif		
	}

	if (fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadTFBuf(fileAddr, 0x01, (BYTE *)&ESN[0], 4);
	
	if((ESN[0] == 0xFF) && (ESN[1] == 0xFF) && (ESN[2] == 0xFF) && (ESN[3] == 0xFF))
	{
		ecpMem_Copy(&ESN[0],&UpdateSSDForESN[0],4);
	}

	flag = ESN[0];//convert to right byte order

	ESN[0] = ESN[3];

	ESN[3] = flag;

	flag = ESN[1];

	ESN[1] = ESN[2];

	ESN[2] = flag;
#if(!UICC__CDMA_2G_NOT_SUPPORT)
	fileAddr = SelectTFFile(DF_CDMA, EF_IMSIM);//read MIN1, MIN2 from file 0x6F22
#else
	fileAddr = SelectTFFile(G_DF_ID_CSIM, EF_IMSIM);//read MIN1, MIN2 from file 0x6F22
#endif

	if (fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadTFBuf(fileAddr, 0x01, MIN2, 2);

	ReadTFBuf(fileAddr, 0x03, MIN1, 3);

	flag = MIN2[0];

	MIN2[0] = MIN2[1];

	MIN2[1] = flag;

	flag = MIN1[0];

	MIN1[0] = MIN1[2];

	MIN1[2] = flag;

	return TRUE;
}


/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 ShiftDataPlus                                                                                                                   
                                                                                                                                                
  Input parameter:   input:    varible name that be add (the buffer must tlv structure) 					       

                                                  
  Output parameter:  input                                                                                                                 
                                                                                                                                                
  Description:       This function is to add tlv structure to the varible buffer of have data           

 -------------------------------------------------------------------------------------------------------------------*/                   
#endif // CDMA
void ShiftDataPlus(BYTE *input,BYTE len,BYTE *output,BYTE pos)
{
	WORD idata i;

	for(i=0;i<len;i++)
	{
		if(i==0)
		{
			output[i] = output[0] | (input[i] >> pos);
		}
		else
		{
			output[i] = (input[i-1] << (8-pos)) | (input[i] >> pos);
		}
	}

	output[i] = (input[i-1] << (8-pos)) | 0;
}







