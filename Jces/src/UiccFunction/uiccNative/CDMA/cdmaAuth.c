/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

*/ 

#include <string.h>
#include "uicc_main.h"
#include "stkhead.h"		   
#include "gsmpublic.h"
#include "cdma.h"
#include "fileAction.h"
#include "fileSystem.h"
#include "interface.h"
#include "uicc_error.h"


#ifdef CDMA

BYTE  xdata ESN[4];
BYTE  xdata LFSR[4];
BYTE  xdata SAVED_LFSR[4];
BYTE  xdata SAVED_RAND[4];
BYTE  xdata Register[0x10];
WORD  xdata SAVED_OFFSET_1;
WORD  xdata SAVED_OFFSET_2;
BYTE  xdata SSD_A_NEW[8];
BYTE  xdata SSD_B_NEW[8];
BYTE  xdata SAVED_DATA[3];
BYTE  xdata MIN1[3]; // = {0x79,0x29,0x71};
BYTE  xdata MIN2[2];
BYTE  xdata RANDBS[4];												//must store
DWORD xdata AuthBs;													//must store
BYTE  xdata G_CdmaStatus ;


BYTE code CaveTable[256] = 
{ 
	0xD9, 0x23, 0x5F, 0xE6, 0xCA, 0x68, 0x97, 0xB0, 0x7B, 0xF2, 0x0C, 0x34, 0x11, 0xA5, 0x8D, 0x4E, 
	0x0A, 0x46, 0x77, 0x8D, 0x10, 0x9F, 0x5E, 0x62, 0xF1, 0x34, 0xEC, 0xA5, 0xC9, 0xB3, 0xD8, 0x2B, 
	0x59, 0x47, 0xE3, 0xD2, 0xFF, 0xAE, 0x64, 0xCA, 0x15, 0x8B, 0x7D, 0x38, 0x21, 0xBC, 0x96, 0x00, 
	0x49, 0x56, 0x23, 0x15, 0x97, 0xE4, 0xCB, 0x6F, 0xF2, 0x70, 0x3C, 0x88, 0xBA, 0xD1, 0x0D, 0xAE, 
	0xE2, 0x38, 0xBA, 0x44, 0x9F, 0x83, 0x5D, 0x1C, 0xDE, 0xAB, 0xC7, 0x65, 0xF1, 0x76, 0x09, 0x20, 
	0x86, 0xBD, 0x0A, 0xF1, 0x3C, 0xA7, 0x29, 0x93, 0xCB, 0x45, 0x5F, 0xE8, 0x10, 0x74, 0x62, 0xDE, 
	0xB8, 0x77, 0x80, 0xD1, 0x12, 0x26, 0xAC, 0x6D, 0xE9, 0xCF, 0xF3, 0x54, 0x3A, 0x0B, 0x95, 0x4E, 
	0xB1, 0x30, 0xA4, 0x96, 0xF8, 0x57, 0x49, 0x8E, 0x05, 0x1F, 0x62, 0x7C, 0xC3, 0x2B, 0xDA, 0xED, 
	0xBB, 0x86, 0x0D, 0x7A, 0x97, 0x13, 0x6C, 0x4E, 0x51, 0x30, 0xE5, 0xF2, 0x2F, 0xD8, 0xC4, 0xA9, 
	0x91, 0x76, 0xF0, 0x17, 0x43, 0x38, 0x29, 0x84, 0xA2, 0xDB, 0xEF, 0x65, 0x5E, 0xCA, 0x0D, 0xBC, 
	0xE7, 0xFA, 0xD8, 0x81, 0x6F, 0x00, 0x14, 0x42, 0x25, 0x7C, 0x5D, 0xC9, 0x9E, 0xB6, 0x33, 0xAB, 
	0x5A, 0x6F, 0x9B, 0xD9, 0xFE, 0x71, 0x44, 0xC5, 0x37, 0xA2, 0x88, 0x2D, 0x00, 0xB6, 0x13, 0xEC, 
	0x4E, 0x96, 0xA8, 0x5A, 0xB5, 0xD7, 0xC3, 0x8D, 0x3F, 0xF2, 0xEC, 0x04, 0x60, 0x71, 0x1B, 0x29, 
	0x04, 0x79, 0xE3, 0xC7, 0x1B, 0x66, 0x81, 0x4A, 0x25, 0x9D, 0xDC, 0x5F, 0x3E, 0xB0, 0xF8, 0xA2, 
	0x91, 0x34, 0xF6, 0x5C, 0x67, 0x89, 0x73, 0x05, 0x22, 0xAA, 0xCB, 0xEE, 0xBF, 0x18, 0xD0, 0x4D, 
	0xF5, 0x36, 0xAE, 0x01, 0x2F, 0x94, 0xC3, 0x49, 0x8B, 0xBD, 0x58, 0x12, 0xE0, 0x77, 0x6C, 0xDA
};

/**end of CAVE header **/
/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 bit_val                                                                                                          

  Input parameter:   octet

  bit1				       

  Output parameter:  value                                                                                                                 

Description:       

status:            ok!,not debug !      

-------------------------------------------------------------------------------------------------------------------*/        

static BYTE bit_val(const BYTE octet, const BYTE bit1)
{
	return((octet << (7 - bit1 )) & 0x80);
}
/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 Rotate_right_registers                                                                                                          

  Input parameter:   void				       


  Output parameter:  void                                                                                                                 

Description:       

status:            ok!,not debug !      

-------------------------------------------------------------------------------------------------------------------*/        

static void LFSR_cycle(void)
{
	BYTE idata temp;
	BYTE idata i;

	temp  = bit_val(LFSR_B,6);

	temp ^= bit_val(LFSR_D,2);

	temp ^= bit_val(LFSR_D,1);

	temp ^= bit_val(LFSR_D,0);

	for (i = 3 ;i >0 ; i--)    // Shift right LFSR, Discard LFSR_D[0] bit 
	{
		LFSR[i] =LFSR[i] >>1;

		if (LFSR[i-1] & 0x01)
		{
			LFSR[i] =LFSR[i] | 0x80;
		}
	}

	LFSR[0] >>= 1;

	LFSR_A |=temp;
}
/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 Rotate_right_registers                                                                                                          

  Input parameter:   void				       


  Output parameter:  void                                                                                                                 

Description:       

status:            ok!,not debug !      

-------------------------------------------------------------------------------------------------------------------*/        

static void Rotate_right_registers(void)
{
	BYTE idata temp_reg;
	BYTE idata i;

	temp_reg = Register[15]; /* save lsb */

	for (i=15; i>0; i--)
	{
		Register[i] >>=1;

		if (Register[i-1] & 0x01)
		{
			Register[i] |= 0x80;
		}
	}

	Register[0] >>=1;

	if (temp_reg & 0x01)
	{
		Register[0] |= 0x80;
	}	
}
/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 roll_LFSR                                                                                                          

  Input parameter:   void				       


  Output parameter:  void                                                                                                                 

Description:       This function is to init the ESN,MIN1,MIN2

status:            ok!,not debug !      

-------------------------------------------------------------------------------------------------------------------*/        

static void roll_LFSR(void)
{
	BYTE idata i;

	LFSR_A = Register[0];

	LFSR_B = Register[1];

	LFSR_C = Register[14];

	LFSR_D = Register[15];

	if ((LFSR_A | LFSR_B | LFSR_C | LFSR_D) == 0)
	{
		for (i=0; i<4; i++)
		{
			LFSR[i] = SAVED_RAND[i];
		}
	}
}

/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 CAVE                                                                                                          

  Input parameter:   number_of_rounds,offset_1	,offset_2				       


  Output parameter:  void                                                                                                                 

Description:       This function is to init the ESN,MIN1,MIN2

status:            ok!,not debug !      

-------------------------------------------------------------------------------------------------------------------*/        

void CAVE(const int number_of_rounds, int * offset_1, int * offset_2)
{
	BYTE idata tempReg0;
	BYTE idata lowNibble;
	BYTE idata hiNibble;
	BYTE idata temp;
	int  idata round_index;
	BYTE idata R_index;
	BYTE idata fail_count;
	BYTE xdata T[16];

	for(round_index = number_of_rounds - 1;	round_index >= 0;round_index --)
	{
		tempReg0 = Register[0];			// save R0 for reuse later 

		for(R_index = 0 ; R_index < 16 ; R_index++)
		{
			fail_count = 0;

			while(1)
			{
				*offset_1 += (LFSR_A ^ Register[R_index]);// will overflow;mask to prevent 

				*offset_1 &= 0xff;

				lowNibble = CaveTable[*offset_1] & LOMASK;

				if (lowNibble == (Register[R_index] & LOMASK))
				{
					LFSR_cycle();

					fail_count++;

					if(fail_count ==32)
					{
						LFSR_D++; /* no carry to LFSR_C */
						break;
					}
				}
				else
				{
					break;
				}
			}//end while

			fail_count = 0;

			while(1)
			{
				*offset_2 += (LFSR_B ^ Register[R_index]);// will overflow; mask to prevent 

				*offset_2 &= 0xff;

				hiNibble = CaveTable[*offset_2] & HIMASK;

				if (hiNibble == (Register[R_index] & HIMASK))
				{
					LFSR_cycle();

					fail_count++;

					if (fail_count ==32)
					{
						LFSR_D++; //no carry to LFSR_C
						break;
					}
				}
				else 
				{
					break;
				}
			}

			temp = lowNibble | hiNibble;

			if (R_index == 15)
			{
				Register[R_index] = tempReg0 ^ temp;
			}
			else
			{
				Register[R_index] = Register[R_index + 1] ^temp;
			}

			LFSR_cycle();
		}//end for r_index

		Rotate_right_registers();   /* shuffle the mixing registers */

		for (R_index = 0; R_index < 16; R_index ++)
		{
			temp = CaveTable[16*round_index + R_index] & LOMASK;

			T[temp] = Register[R_index];
		}

		for (R_index = 0; R_index < 16 ; R_index ++)
		{
			Register[R_index] = T[R_index];
		}

	}//end for round_index
}//end CAVE

/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 Key_VPM_Generation                                                                                                          

  Input parameter:   SSD_B,cmeakey,Vpm					       


  Output parameter:  void                                                                                                                 

Description:       This function is to init the ESN,MIN1,MIN2

status:            ok!,debug ok !      

-------------------------------------------------------------------------------------------------------------------*/                   

void Key_VPM_Generation(BYTE * SSD_B,BYTE * cmeakey,BYTE * VPM)
{
	BYTE idata i,r_ptr,vpm_ptr;
	int  idata offset_1,offset_2;

	for (i=0;i<4;i++)
	{
		LFSR[i] = SAVED_LFSR[i] ^ SSD_B[i] ^ SSD_B[i+4];
	}

	if ((LFSR_A | LFSR_B | LFSR_C | LFSR_D) == 0)
	{
		for (i =0;i<4;i++)
		{
			LFSR[i]=SAVED_RAND[i];
		}
	}

	for (i=0;i<8;i++)
	{
		Register[i]=SSD_B[i];
	}

	Register[8]=AAV;

	for (i=9;i<12;i++)
	{
		Register[i]=SAVED_DATA[i-9];
	}

	for (i=12;i<16;i++)
	{
		Register[i]=ESN[i-12];
	}

	offset_1=SAVED_OFFSET_1;

	offset_2=SAVED_OFFSET_2;

	CAVE(8,&offset_1,&offset_2);

	roll_LFSR();

	CAVE(4,&offset_1,&offset_2);

	for (i=0; i<4; i++)
	{
		cmeakey[i]=Register[i+4] ^ Register[i+8];
	}

	roll_LFSR();

	CAVE(4,&offset_1,&offset_2);

	for (i=4;i<8;i++)
	{
		cmeakey[i]=Register[i] ^ Register[i+4];
	}

	vpm_ptr = 0;

	for (i=0; i<11; i++)
	{
		roll_LFSR();

		CAVE(4,&offset_1,&offset_2);

		for (r_ptr=0;r_ptr<6;r_ptr++)
		{
			VPM[vpm_ptr]=Register[r_ptr+2] ^ Register[r_ptr+8];

			vpm_ptr++;
		}
	}
}
/*---------------------------------------------------------------------------------------------------------------                                          

  Function name :	 Auth_Signature                                                                                                          

  Input parameter:   rand_challenge,auth_data,ssd_auth,save_Registers					       


  Output parameter:  void                                                                                                                 

Description:       This function is to init the ESN,MIN1,MIN2

status:            ok!,debug ok !      

-------------------------------------------------------------------------------------------------------------------*/                   

DWORD Auth_Signature(const BYTE RAND_CHALLENGE[4],const BYTE AUTH_DATA[3],const BYTE *SSD_AUTH,const BYTE SAVE_REGISTERS)
{
	BYTE  idata i;
	int   idata offset_1,offset_2;
	DWORD xdata AUTH_SIGNATURE;

	for (i=0;i<4;i++)
	{
		LFSR[i]=RAND_CHALLENGE[i] ^ SSD_AUTH[i] ^ SSD_AUTH[i+4];
	}
	if ((LFSR_A | LFSR_B | LFSR_C | LFSR_D) == 0)
	{
		for (i=0;i<4;i++)
		{
			LFSR[i]=RAND_CHALLENGE[i];
		}
	}

	for (i=0;i<8;i++)
	{
		Register[i]=SSD_AUTH[i];
	}

	Register[8]=AAV;

	for (i=9;i<12;i++)
	{
		Register[i]=AUTH_DATA[i-9];
	}

	for (i=12;i<16;i++)
	{
		Register[i]=ESN[i-12];
	}

	offset_1 = 128;

	offset_2 = 128;

	CAVE(8,&offset_1,&offset_2);

	AUTH_SIGNATURE=( ((unsigned long)(Register[0] ^ Register[13]) << 16) +((unsigned long)(Register[1] ^ Register[14]) << 8)  +
		((unsigned long)(Register[2] ^ Register[15]) )        )
		& 0x3ffff;

	if(SAVE_REGISTERS)
	{
		SAVED_OFFSET_1 = offset_1;

		SAVED_OFFSET_2 = offset_2;

		for(i = 0;i < 4; i++)
		{
			SAVED_LFSR[i] = LFSR[i];

			SAVED_RAND[i] = RAND_CHALLENGE[i];

			if (i < 3)
			{
				SAVED_DATA[i] = AUTH_DATA[i];
			}
		}
	}

	return(AUTH_SIGNATURE);
}

/**
 * @brief 	RunCDMAAlgorithm	
 * 
 * 
 */
void RunCDMAAlgorithm(void)
{
	UWORD xdata keyAddr,fileAddr;
	BYTE  idata  result;

	if(G_CommandMode == Command_Mode_2G)
	{
		if( (P1 != 0x00)||(P2 != 0x00))
		{
			P1P2Error();														//send 6b 00
			return;
		}
		
		if(G_CurrentDfName[G_ChannelNum] != DF_CDMA)
		{
			FileInconsistent();//send 94 08
			return;
		} 
		
		if(P3 != 0x11)
		{
			P3Error();														//send  67 00
			return ;
		}
	}
	else
	{	
		if((P1>0x02)||(P2!=0x00)&&(P2 != 0x80)&&(P2 != 0x81)&&(P2 != 0x82))
		{
			P1P2Error();														//send 6b 00
			return;
		} 

		//"a particular USIM application or any sub-directory must be
		//selected as the Current Directory" --TS31122
													   						
		if (G_CurrentAdfName[G_ChannelNum] == FALSE)
		{
			fileAddr = FALSE;
		}
		else 
		{
			fileAddr = SearchFile(E_FILE_START, G_CurrentAdfName[G_ChannelNum]);
	
			fileAddr = SearchFile(fileAddr, G_CurrentDfName[G_ChannelNum]);
		}

		if (fileAddr == FALSE)
		{
			ConditionSatisfied();												//Send 69 85
			return;
		}	
	}

	if( ValueBit(G_ChvStatus, B_Chv_1_Verified) == FALSE )
	{	
		if(G_CommandMode == Command_Mode_2G)											//for change2010.11.23  区分2G和3G模式下的返回状态码  
		{
			SecuritySatisfied();
			return;
		}
		else
		{
			SecuritySatisfied();
			return;
		}			
	}

	//keyAddr = SelectLFFile(DF_CDMA, EF_Ki);  
	keyAddr = SelectLFFile(G_DF_ID_CSIM, EF_Ki);  	

	if( keyAddr == FALSE )
	{
		TechnicalProblem();
		return;
	}

	result = InitCave();

	if (result == FALSE)
	{
		TechnicalProblem();													//返回 6F 00
		return;	
	}

	ReceiveData();

	CDMAAuth();

}

#endif

