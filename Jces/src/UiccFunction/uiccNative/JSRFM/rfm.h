#ifndef    _RFM_HEAD_H_
#define    _RFM_HEAD_H_


#define TRIPLEDES            0x01
#define SINGLEDES            0x02

#define KEY_KID      		0x80
#define KEY_KIC        		0x00

#define POP_OK           	0x00
#define ERROR_CHECKSUM   	0x01 
#define ERROR_CNTR_LOW   	0x02
#define ERROR_CNTR_HIGH  	0x03
#define ERROR_CNTR_BLCED 	0x04
#define ERROR_CIPHERING  	0x05
#define ERROR_SECURY     	0x06
#define ERROR_MEMERY     	0x07
#define ERROR_TIME       	0x08
#define ERROR_WRONGTAR      0x09
#define ERROR_FILE       	0x0A

#define	ErrorNoTPDU			FALSE
#define	ErrorMTI			FALSE
#define	ErrorCHL			FALSE 
#define ErrorDataLgth		FALSE

#define POR_DATA_MAXLENGTH						0x79
#define POR_DATA_MAX_SUBMIT						0x8C
//-----------------------------------------------------------------------------------------------//
#define   EF_RFMSP         	0x6FD0     	 //include 2 bytes
#define   EF_COUNTER       	0x6FD2        //include 2*5 bytes
#define   EF_MANY_SMS      	0x6FD6        //include 550 bytes



//----------------------------------------------------------------------------------------------//
#define    RFMMAXSMS        0x0A
#define    ABNORMTRUE 		2

#define	   D_OTARfmFristSms			0x01
#define	   D_RfmOtherSms			0x02
#define	   D_WIBRfmFristSms			0x03
#define	   D_WIBFristSms			0x05

/***********************TAR type*******************************/
#define	    	WIB_PULL_TAR 			                0x00
#define	    	WIB_PUSH_TAR 			                0x01
#define	    	WIB_ADMIN_TAR 			                0x02
#define	    	RFM_TAR 			                	0x03
#define	    	ErrorTAR				        		0xFF
#define     EF_USIMTAR              0x4E16

#define	    WIBMODE 			                    0xF0
#define	    RFMMODE 			                    0xF1

#define     NormalMODE							  0x00
#define     USIMMODE							  0x01

/***********************tag******************************/
#define	D_RFMSucessTag			                0xA3		//
#define	D_WIBRFMProactiveCommandTag	            0xA4		//


#define	D_ReceiInfoDisplay  		            0xA5        //Display Receiving Info
#define D_UnknownTag							0xBF
#define	D_RFMFinishTag  		                0xA7

 


/***********************SC tag value*******************************/
#define     TAG_COUNTERS                            0x80
#define     TAG_OUTGOING_SPIKICKID                  0x81
#define     TAG_INCOMING_SPIKICKID                  0x82
#define     TAG_KICKEY_SET                          0x83
#define     TAG_KIDKEY_SET                          0x84
#define	    TAG_KIKKEY_SET							            0x85		//for TMN BAP�?
/***********************TAG*******************************/
#define     TAG_DONOTSETUPMENU                      0x01
#define     TAG_DISPLAYTIMES		                0x02
#define     TAG_POLLTITERVAR     	                0x03
#define     TAG_DISPLAYUNKNOWWIBTAR                 0x04
#define     TAG_DISPLAYRFMERROR                     0x05
//#define     TAG_NODISPLAYCOMMANDTAG                 0x06
/***********************Variable Range*******************************/
#define     VARIBLE_01TODF                          0x00
#define     VARIBLE_E0TOFF                          0x01
#define     VARIBLE_01TOFF                          0x02
//------------------------Default Errors----------------------------
#define     ErrorDefault                                        0xDF          

#define     ErrorReadEFBytecode                                 0x01        //Failed to find/read EFBytecode
#define     ErrorReadEFTar                                      0x02        //Failed to find/read EFTAR
#define     ErrorReadEFErrorText                                0x03        //Failed to access/read EFErrorText
#define     ErrorReadEFSmsHeader                                0x04        //Failed to find/read EFSMSHeader
#define     ErrorReadEFKey                                      0x05        //Failed to read key file
#define     ErrorReadEFVersionInfo                              0x06        //Failed to find/read EFVersionInformation
#define     ErrorAccessFile                                     0x1F        //Failed to access file

//----------------------------------------------------------------------------------------------//
#define     ErrorConfigurationError                             0x2E 
#define     ErrorInvalidIncomingMessage                         0x34
#define     ErrorE2PROMMemoryProblem                            0x2C
#define	    D_TmpBufStart			                232                //Buffer pos

#define	    D_TmpBufStart2			                202                

#define     ErrorRFMUpdate                                      0xF0
#define     UCS2_DCS                                0x08
//----------------------------------------------------------------------------------------------//
#define		RFMCurrentOpened		0x01
#define		RFMCurrentClosed 		0x00

#define		SYN_PDU	 				0x80
#define		SYN_ACK_PDU				0xC0
#define		ACK_UNSEG_PDU			0x40
#define		ACK_SEG_PDU				0x44
#define		RST_PDU					0x10
#define		RST_ACK_PDU				0x50
#define		NULL_PDU				0x08
#define		NULL_ACK_PDU			0x48
		
//----------------------------------------------------------------------------------------------//
//extern	  BYTE xdata G_RefNumber;
//extern 	  BYTE  xdata G_ReTarType;
//extern	  BYTE  xdata G_ReTarSc;
extern    BYTE  xdata G_TarTypeMode;
extern 	  BYTE  xdata G_ReTarNo;
extern 	  BYTE  xdata G_TempTarNo;
extern	  BYTE  xdata G_TempSc;
extern 	  UWORD xdata G_AppStartAddr;
//extern 		UWORD xdata G_StkCommandAddr;
//extern 		UWORD xdata StkStack[0x10];
//extern		BYTE xdata G_ErrorType;
//extern 		BYTE  xdata G_StoreName;
//extern 		UWORD xdata G_CurDfEf_Name[2];	                // µ±ǰDF,EF Name

//extern		BYTE 	xdata  G_BIPBUFFER[0x100];

extern   BYTE xdata G_RFM_TAR;
extern BYTE xdata G_VIMSI_TAR;	



//----------------------------------------------------------------------------------------------//
extern BYTE  xdata G_WIBTag[8];    // save from CHL to TAR
extern BYTE  xdata G_WIBMac[8];    // save CC/RC
extern BYTE  xdata G_CheckSumLgth;
extern BYTE  xdata G_CurrentComNUM;
extern WORD  xdata CommandStartNum;


extern BYTE xdata G_RFMChannelId;
extern BYTE xdata G_RFMOpenChannel;
extern BYTE xdata G_RFMBIPDataLgth;
extern BYTE xdata BIP_RfmIndex;
extern BYTE xdata G_BIPMode;

/*-------------------------------------------------------*/
#define		SigDES	0x00
#define		TriDES	0x02


#define    TripleDES2Key    0x00
#define    TripleDES3Key    0x01

#define		D_VIMSITag			    		0xD3
#define		D_RFMTag			    		0xD4
#define	    D_RFMProactiveCommandTag	    0xD5
	

#define   	EF_RFM_Msl		        0x4E8A



#define 	EF_RFM_PorConfig 	    0x6F04		  //POR CONFIGFILE


/*-------------------------RFM file----------------------*/
#define     DF_RFM           	    0x7110

#define		EF_RFM_Tar			    0x4E05
#define	    EF_WIBRFM_Key		    0x4E06
#define		EF_RFM_Counter		    0x4E09
#define   	EF_RFM_Sc		        0x4E0A
#define 	EF_RFM_ScaTpoa 		    0x4E0C		  //the file saved download tpoa and smsc

#define 	DF_WIB				0x2700
#define     EF_WIB_Counter    		0x6F06
#define	    EF_WIB_Tar				0x6F1A
#define	    EF_WIB_Sc				0x6F1B
#define   	EF_WIBDownLoadFile		0x4E02
#define 	EF_WIBTempFile		    0x4E03


//----------------------------------------------------------------------------------------------//


//----------------------------------------------------------------------------------------------//

extern BYTE WIBDecryptANDCheck(BYTE refNumber);

extern BYTE ChipheRe(BYTE kic,BYTE *pointer,BYTE * result_pointer,BYTE mode,WORD lgth);

extern BYTE SaveTPOAtoFile(BYTE * smscBuf,BYTE * tpoaBuf);

extern BYTE IsRFMReceiveAllWIB(UWORD fileAddr, BYTE packageNum);

extern BYTE LocalRFMTmpRecord(BYTE packageNum, BYTE index);

extern BYTE StoreDataSms(BYTE * buf,BYTE refNumber,BYTE seqNumber,BYTE totalNumber);

extern BYTE ReadDataSms(BYTE * buf,BYTE refNumber,BYTE seqNumber);

extern void WIBRFMHandlerD0(void);

extern void RFMDownLoadExcute(BYTE refNumber);

extern BYTE WIBCheckSecurityLevel(BYTE * spikickidBuf);

extern BYTE WIBCheckCounter(BYTE spi,BYTE * counterPointer);

extern BYTE WIBDwonLoadDecrypt(UWORD fileAddr,UWORD dataAddr, BYTE packageNum,BYTE totalNo);

extern BYTE CalculateMacValue(BYTE spi,BYTE kic,BYTE kid,UWORD eepAddr,BYTE * result_pointer,WORD lgth);

extern BYTE CalculateAESMacValue(BYTE kid,UWORD eepAddr,BYTE * result_pointer,WORD lgth);

extern void CommonSMSForRFMHandle(void);

extern void STKUpdateRecord(void);

extern BYTE ComputeRecord(BYTE action);

extern WORD GetScTagOffset(UWORD fileAddr,BYTE scIndex,BYTE tag);

extern WORD BuildWORD(BYTE buf[2]);

extern WORD GetWord(BYTE * buf);

//extern BYTE SetWord(BYTE * buf, WORD val);

extern BYTE IncreatCNTR(BYTE cntr[5]);

extern BYTE CheckTar(BYTE * tarPointer);

extern BYTE ReadTagValue(BYTE tag,BYTE * buf);

extern BYTE RFMExcuteCommand(void);

extern void ResponsePack(BYTE errorCode);

extern void PushCommand(void); 

extern BYTE RFMDataAvailable(BYTE pos);

extern BYTE RFMChannelStatus(BYTE pos);

extern BYTE RFMChannelStatus(BYTE pos);

extern void RFMDataAvaiableProcess(void);

extern void StoreBIPDataSms(BYTE ptr);

extern void ExcuteRFMProComamnd(BYTE tag);

extern BYTE Findkey(BYTE keyType, BYTE keyIndex, BYTE * dataBuffer,BYTE length);

extern void PutKeyCommand(void);

extern void ClearTempFile(void);

extern void TripleDES_3Key(BYTE * pbSrcStr,BYTE * keyContext,BYTE * pbTgtStr, BYTE bMod, BYTE lgth);

extern BYTE GetKeyIndex(UWORD fileAddr,BYTE tag, BYTE index, BYTE mode);

extern void CRC32(UWORD eepAddr,BYTE * c_result,WORD lgth);

extern void SmartISO9797Algorithm3Ee(UWORD eepromAddr,BYTE * cpKey,BYTE cpResult[8],WORD nEnterLen);

extern void SmartISO9797Algorithm4Ee(UWORD eepromAddr,BYTE * cpKey,BYTE cpResult[8],WORD nEnterLen,WORD initLen);

extern void DesMacFunction(UWORD eepromAddr,BYTE * cpKey,BYTE cpResult[8],WORD nEnterLen,BYTE mode,BYTE tripleDESMode);

extern void VIMSIHandler(BYTE refNumber);

extern void RFMHandlerD0(void);

extern void ClearSMSBufFile(void);

#endif
