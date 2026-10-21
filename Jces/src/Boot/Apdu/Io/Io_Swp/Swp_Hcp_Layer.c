#include "Swp_Hcp_Layer.h"
#include "Swp_Llc_Layer.h"
#include "SwpTimer.h"
#include "SwpHal.h"
#include "ecpReadWrite.h"
#include "BootNoAreaInitRam.h"
#include "JcvmConfig.h"
#include "MemNvmOperation.h"
#include "SwpDataConfig.h"

#if BOOT__IO__SWP
/*  private prototypes
 ****************************************************************/
static u8 AdminPipeInitProcess(void);
static u8 EndInitProcess(void);
static u8 TypeAInitProcess(void);
static u8 TypeBInitProcess(void);
static u8 TypeAReaderInitProcess(void);
static u8 TypeBReaderInitProcess(void);
static u8 ConnectivityInitProcess(void);
static u8 IdentifyMgrInitProcess(void);

/*  defines
****************************************************************/

typedef enum {
	CMD_TYPE_OPEN_PIPE,
	CMD_TYPE_CLOSE_PIPE,
	CMD_TYPE_GET_PARA,
	CMD_TYPE_SET_PARA,
	CMD_TYPE_CREATE_PIPE,
	CMD_TYPE_DELETE_PIPE,
	CMD_TYPE_CLEAR_ALL_PIPE,
	CMD_TYPE_GET_SESSION_ID,
	CMD_TYPE_END_OF_OPERATION,
}CMD_TYPE;


// typedef struct {
// 	u8 PipeType;
// 	u8 ParaInfo[3];  // source Gate id, destination Host id, destination Gate id
// }CREATE_PIPE_PARA_INFO;

typedef struct {
	u8 Status;
	u8 cmdType;
}INIT_SUB_LIST;

// /// @brief: Create dynamic pipes list
// static const CREATE_PIPE_PARA_INFO CreatePipesList[] = {
// 	INIT_TYPEA_PROCESS             ,  {HCP_GATE_ID_CARD_TYPEA, HCP_CONTROL_HOST, HCP_GATE_ID_CARD_TYPEA},
// 	INIT_TYPEB_PROCESS             ,  {HCP_GATE_ID_CARD_TYPEB, HCP_CONTROL_HOST, HCP_GATE_ID_CARD_TYPEB},
// 	INIT_TYPEA_READER_PROCESS      ,  {HCP_GATE_ID_READER_TYPEA, HCP_CONTROL_HOST, HCP_GATE_ID_READER_TYPEA},
// 	INIT_TYPEB_READER_PROCESS      ,  {HCP_GATE_ID_READER_TYPEB, HCP_CONTROL_HOST, HCP_GATE_ID_READER_TYPEB},
// 	INIT_CONNECTIVITY_PROCESS      ,  {HCP_GATE_ID_CONNECTIVITY, HCP_CONTROL_HOST, HCP_GATE_ID_CONNECTIVITY},
// 	INIT_FINISH                    ,  {0x00, 0x00, 0x00},
// };


static const INIT_SUB_LIST AdminPipeInitList[] = {
	{INIT_START                ,    CMD_TYPE_OPEN_PIPE        },
	{INIT_GET_SESSIONID        ,    CMD_TYPE_GET_SESSION_ID   },
	{INIT_CLEAR_ALL_PIPES      ,    CMD_TYPE_CLEAR_ALL_PIPE   },
	{INIT_OPEN_ADM_PIPE        ,    CMD_TYPE_OPEN_PIPE        },
//	{INIT_OPEN_LINK_PIPE       ,    OpenPipeAdm               },
	{INIT_SET_WHITELIST        ,    CMD_TYPE_SET_PARA         },
	{INIT_FINISH               ,    0x00                      },
};

static const INIT_SUB_LIST EndInitList[] = {
	{INIT_SET_SESSIONID        ,    CMD_TYPE_SET_PARA         },
//	{INIT_END_OPERATION        ,    0x00                      },
	{INIT_FINISH               ,    0x00                      },
};

/// @brief: Type A pipe list
static const INIT_SUB_LIST TypeAInitList[] = {
	{CMD_TYPE_CREATE_PIPE                              ,   CMD_TYPE_CREATE_PIPE    },
	{CMD_TYPE_OPEN_PIPE                                ,   CMD_TYPE_OPEN_PIPE      },
	{HCP_RFTYPEA_GATE_REG_INDEX_UID                    ,   CMD_TYPE_SET_PARA       },
	{HCP_RFTYPEA_GATE_REG_INDEX_SAK                    ,   CMD_TYPE_SET_PARA       },
	{HCP_RFTYPEA_GATE_REG_INDEX_ATQA                   ,   CMD_TYPE_SET_PARA       },
	{HCP_RFTYPEA_GATE_REG_INDEX_APP_DATA               ,   CMD_TYPE_SET_PARA       },
	{HCP_RFTYPEA_GATE_REG_INDEX_FWI_SFGI               ,   CMD_TYPE_SET_PARA       },
	{HCP_RFTYPEA_GATE_REG_INDEX_CID                    ,   CMD_TYPE_SET_PARA       },
	{HCP_RFTYPEA_GATE_REG_INDEX_DTR_MAX                ,   CMD_TYPE_SET_PARA       },
	{HCP_RFTYPEA_GATE_REG_INDEX_MODE                   ,   CMD_TYPE_SET_PARA       },
	{INIT_FINISH                                       ,   0x00                    },
};


/// @brief: Type B pipe list
static const INIT_SUB_LIST TypeBInitList[] = {
	{CMD_TYPE_CREATE_PIPE                              ,   CMD_TYPE_CREATE_PIPE    },
	{CMD_TYPE_OPEN_PIPE                                ,   CMD_TYPE_OPEN_PIPE      },
	{HCP_RFTYPEB_GATE_REG_INDEX_PUPI                   ,   CMD_TYPE_SET_PARA       },
	{HCP_RFTYPEB_GATE_REG_INDEX_AFI                    ,   CMD_TYPE_SET_PARA       },
	{HCP_RFTYPEB_GATE_REG_INDEX_ATQB                   ,   CMD_TYPE_SET_PARA       },
	{HCP_RFTYPEB_GATE_REG_INDEX_HIGHER_LAYER_RESPONSE  ,   CMD_TYPE_SET_PARA       },
	{HCP_RFTYPEB_GATE_REG_INDEX_DATARATE_MAX           ,   CMD_TYPE_SET_PARA       },
	{HCP_RFTYPEB_GATE_REG_INDEX_MODE                   ,   CMD_TYPE_SET_PARA       },
	{INIT_FINISH                                       ,   0x00                    },
};


/// @brief: Type A Reader pipe init list
static const INIT_SUB_LIST TypeAReaderInitList[] = {
	{CMD_TYPE_CREATE_PIPE                              ,   CMD_TYPE_CREATE_PIPE    },
	{CMD_TYPE_OPEN_PIPE                                ,   CMD_TYPE_OPEN_PIPE      },
	{INIT_FINISH                                       ,   0x00                    },
};


/// @brief: Type B Reader pipe list
static const INIT_SUB_LIST TypeBReaderInitList[] = {
	{CMD_TYPE_CREATE_PIPE                              ,   CMD_TYPE_CREATE_PIPE    },
	{CMD_TYPE_OPEN_PIPE                                ,   CMD_TYPE_OPEN_PIPE      },
	{INIT_FINISH                                       ,   0x00                    },
};


/// @brief: Connectivity pipe list
static const INIT_SUB_LIST ConnectivityInitList[] = {
	{CMD_TYPE_CREATE_PIPE                              ,   CMD_TYPE_CREATE_PIPE    },
	{CMD_TYPE_OPEN_PIPE                                ,   CMD_TYPE_OPEN_PIPE      },
	{INIT_FINISH                                       ,   0x00                    },
};

/// @brief: Identify Manager pipe list
static const INIT_SUB_LIST IdentifyMgrInitList[] = {
	{CMD_TYPE_CREATE_PIPE                              ,   CMD_TYPE_CREATE_PIPE    },
	{CMD_TYPE_OPEN_PIPE                                ,   CMD_TYPE_OPEN_PIPE      },
	{HCP_IDT_MGT_GATE_REG_INDEX_VNDNAME                ,   CMD_TYPE_GET_PARA       },
	{HCP_IDT_MGT_GATE_REG_INDEX_MODELID                ,   CMD_TYPE_GET_PARA       },
	{INIT_FINISH                                       ,   0x00                    },
};



typedef u8 ( CALLBACK_FUNC)(void);

typedef struct {
	u8 Status;
	INIT_SUB_LIST *subStatus;
	CALLBACK_FUNC *InitProcess;
}INIT_MAIN_LIST;


static const INIT_MAIN_LIST swpInitList[] = {
	{INIT_START                ,     (INIT_SUB_LIST *)&AdminPipeInitList       ,   AdminPipeInitProcess     },
	{INIT_TYPEA_PROCESS        ,     (INIT_SUB_LIST *)&TypeAInitList           ,   TypeAInitProcess         },
	{INIT_TYPEB_PROCESS        ,     (INIT_SUB_LIST *)&TypeBInitList           ,   TypeBInitProcess         },
	{INIT_TYPEA_READER_PROCESS ,     (INIT_SUB_LIST *)&TypeAReaderInitList     ,   TypeAReaderInitProcess   },
	{INIT_TYPEB_READER_PROCESS ,     (INIT_SUB_LIST *)&TypeBReaderInitList     ,   TypeBReaderInitProcess   },
	{INIT_CONNECTIVITY_PROCESS ,     (INIT_SUB_LIST *)&ConnectivityInitList    ,   ConnectivityInitProcess  },
	{INIT_GET_CLF_INFO_PROCESS ,     (INIT_SUB_LIST *)&IdentifyMgrInitList     ,   IdentifyMgrInitProcess   },
	{INIT_END_OPERATION        ,     (INIT_SUB_LIST *)&EndInitList             ,   EndInitProcess           },
	{INIT_FINISH               ,     (INIT_SUB_LIST*)0xFF    ,   (CALLBACK_FUNC*)0xFF     },
};



/*  typedefs
****************************************************************/

/*  constants
****************************************************************/

/*  variables
****************************************************************/

SWP_HCP_Context_t  g_HCP_Context;
SWP_HCP_NVM_Context_t	g_HCP_RAM_Context;

const SWP_HCP_NVM_Context_t swpConfigAreaBackup[] = 
{
/* SESSION ID	*/							0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
/* HCP_LINK_MGT_PIPE = 0,     Gate 06*/		0 , 3 , 2 , 2 , 0, 0xF8,
/* HCP_ADMIN_PIPE = 1,        Gate xx*/		0 , 0 , 2 , 0 , 1, 0xF8,
/* PIPE x		*/							255, 255, 255, 255, 255,0xFF,
											255, 255, 255, 255, 255,0xFF,
											255, 255, 255, 255, 255,0xFF,
											255, 255, 255, 255, 255,0xFF,
											255, 255, 255, 255, 255,0xFF,
											255, 255, 255, 255, 255,0xFF,
											255, 255, 255, 255, 255,0xFF,
											255, 255, 255, 255, 255,0xFF,

												// identity management gate registry
		/* VERSION_SW	*/							0x01,0x01,0x00,
		/* HCI_VERSION	*/							0x01,
		/* VERSION_HARD	*/							0x01,0x01,0x00,
		/* VENDOR_NAME	*/							0x45,0x43,0x50,0x2D,0x53,0x57,0x50,0x01,
		/* MODEL_ID		*/							0x05,
		/* GATES_LIST	*/							0x01,0x04,0x05,0x23,0x21,0x41,
		/* REC_ERROR	*/							0xFF,0xFF,
		/* MODE			*/							0x02,
		/* UID			*/							0x04,0x1f,0x00,0x1D,0x51,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		/* SAK			*/							0x20,
		/* ATQA			*/							0x04,0x00,
		/* APPLICATION_DATA		*/					0x03,0x90,0x12,0x34,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		/* FWI_SFGI		*/							0x70,
		/* CID_SUPPORT	*/							0x01,
		/* DATARATE_MAX	*/							0x00,0x00,0x01,

												// RF technology type B
		/* MODE			*/							0x02,
		/* PUPI			*/							0x00,0xFF,0xFF,0xFF,0xFF,
		/* AFI			*/							0x01,
		/* ATQB			*/							0x00,0x00,0x00,0x70,
		/* HIGHT LAYER RESPONSE			*/			0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		/* DATARATE_MAX	*/							0x00,0x00,0x01,

		/* Sync ID      */                          0xFA,0xDA,
};
u8 MySessionId[8];

volatile u8 g_HCP_Card_Status;

volatile u8 g_HCP_NeedProcess;

u8 g_HCI_API_TrigEvent;

/// @brief: SWP Events Manager
u8 g_SwpHciEvents[EVENTS_NUM];


/// @brief indicate hcp package is receive complete or is header
/// bit1 : =1 is header; =0 no header;    bit8 : =1 complete; =0 no complete
u8 g_HCP_PackageStatus;
u8 SelectedCltApplication;


u8 clfName[21];

/*
   存储信息的结构:  Total Len(L) 
                                   (CLF Info Start)0xA5 + CLF Vendor Name Len(L) + Name(V) + ModeID config Len (L) + Value(V)
								   0xA5 xx xxxxxx xx xxxx
   
   CLF Vendor Name:
        NXP:   4E5850(NXP)
		ST :   53544D4943524F454C454354524F4E494353(STMICROELECTRONICS)
   
   注：此处存储的是需要使用兼容性方案的Clf Mode id
*/
u8 checkCLFModeIDConfig(u8 *vendorName, u8 nameLen, u8 modeID)
{
	u8 i;
	u8 len=0;
	u8 totalLen;
	PM8 config;
	
	config = MEMtPM8(JcvmConfig_GetSysConfigAddr(clfModeIDConfig));
	totalLen = memReadU8(config++);
	while(1)
	{
		if (memReadU8(config++) == 0xA5)
		{
			if ( (PMtMEM(config) - JcvmConfig_GetSysConfigAddr(clfModeIDConfig)) > totalLen )
			{
				return TRUE;
			}
			
			len = memReadU8(config++);
			if (len == nameLen)
			{
				// compare vendor name
				if ( memCompare(config, vendorName, nameLen) == ECP_CMP_RESULT_EQUAL )
				{
					// point to ModeID config Len
					config += len;
					
					len = memReadU8(config++);
					
					for (i=0; i<len; i++)
					{
						if (memReadU8(config++) == modeID)
						{
							return FALSE;
						}
					}
				}
			}
			else
			{
				//point to ModeID config Len 
				config += len;
				len = memReadU8(config++);
				//get next CLF Info Start
				config += len;
			}
		}
		else
		{
			return TRUE;
		}
	}
}
 

static u32 SWP_HCP_SendPacket ( u8 PipeId )
{
        u8 NbBytes;
        swpTimer_SwpTimerStop();
		//__disable_irq();

		// write HCP packet header : pipe ID
		g_SWP_LLC_Context.WriteLpdu->Data[1] = PipeId & HCP_PACKET_PIPEID_MASK;

		// maximum number of bytes of a HCp message that can be sent
			// in a single HCP packet is maximum 28 bytes ( 29 bytes for SHDLC LPDU minus
		// one byte of HCp packet header )
		if ( g_HCP_Context.WriteMessage.Size > (LLC_BUF_SIZE - 2))
		{
			 // do a chaining => cb bit is 0

			 // send max nb of bytes
				NbBytes = LLC_BUF_SIZE - 2;
		}
    else
    {
        // no chaining, cb bit is 1
       g_SWP_LLC_Context.WriteLpdu->Data[1] |= HCP_PACKET_CB_MASK;

       // send all remaining bytes
        NbBytes =  g_HCP_Context.WriteMessage.Size;
    }

    // copy data in LLC buffer ( without LLC control byte + HCP packet header)
	// because write message buf calculate from hcp message header, so off - 1
		ecpMem_Copy ( (u8*)&g_SWP_LLC_Context.WriteLpdu->Data[2] ,
              g_HCP_Context.WriteMessage.Buf + g_HCP_Context.NbBytesSent , NbBytes );

    // set number of bytes to sent for the HCP packet ( not counting LLC control byte )
    g_SWP_LLC_Context.WriteLpdu->Size = NbBytes + 1;

	// update nb of byte still to be sent
    g_HCP_Context.WriteMessage.Size -= NbBytes;

    // update nb of bytes already sent
    g_HCP_Context.NbBytesSent += NbBytes;

		if (g_HCP_Context.WriteMessage.Size == 0)
		{
			g_HCP_Context.NbBytesSent = 0;
		}

	  //__enable_irq();

    // send LLC frame
    if ( Swp_Llc_Shdlc_Send() != SWP_HCP_OK )
		{
			return HCP_TRANSMISSION_ERROR;
		}

    if ( SWP_MAC_NoBufferFree( ) )
    {
        // SWP ram buffer is full now
        return SWP_HCP_SWP_BUFFER_FULL;
    }
    else
    {
        return SWP_HCP_OK;
    }

}

static u8  SWP_HCP_CountOpenedPipes ( u8 GateId )
{
      u8 cpt;
	  u8 nbGateOpened;

    nbGateOpened = 0;
    for (cpt=0;cpt<HCP_MAX_PIPES;cpt++)
	{
        if (g_HCP_Context.NvmContext->Pipes[cpt].DestinationGId == GateId )
		{
            if ( g_HCP_Context.NvmContext->Pipes[cpt].State == HCP_PIPE_OPENED )
			{
                nbGateOpened ++;
		    }
        }
    }
    // check result value
    return (nbGateOpened-1);
}

 u8 GetPipeIndex(u8 dstGateId)
{
    u8 i;

    // Search Gate Id in g_HCP_Context.NvmContext->Pipes[i].GateId
    for (i=0; i<HCP_MAX_PIPES; i++)
    {
        if (g_HCP_Context.NvmContext->Pipes[i].DestinationGId == dstGateId)
        {
            return  i;
        }
    }
    return HCP_PIPE_NOT_CREATED;
}

static u8 GetCurrentPipeIndex(u8 PipeId)
{
    u8 i;

    // Search g_HCP_Context.CurrentPipeId in g_HCP_Context.NvmContext->Pipes[i].PipeId
    // link management gate case
    for (i = 0 ; i < HCP_MAX_PIPES ; i++)
    {
        if (g_HCP_Context.NvmContext->Pipes[i].PipeId == PipeId)
        {
            return i;
        }
    }
    return HCP_PIPE_NOT_CREATED;
}

static u32 ProcessAnySetParam(void)
{
    u8 WriteStatus;

    if (g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State != HCP_PIPE_OPENED)
    {
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_PIPE_NOT_OPENED );
        return SWP_HCP_OK;
    }
    if (( g_HCP_Context.ReadMessage.Size < 3 ) ) ///< size should be at least 3 ( 1B header, 1B index and 1B data or more )
    {
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
        return SWP_HCP_OK;
    }
    // link management gate case
    if ( g_HCP_Context.CurrentPipeId == HCP_PIPE_ID_LINK_MGT )
    {
        // set parameter in Link Mgt Gate Registry
        if (( g_HCP_Context.ReadMessage.Buf[1] == HCP_CMD_ANY_SET_PARAMETER ) && ( g_HCP_Context.ReadMessage.Size == 4 ))
        {
                ///< size should be 1 B for header + 1 B for index + 2B for parameter              
                WriteStatus = ecpNvm_WriteErase(MEMtPN8(&g_HCP_Context.NvmContext->LinkMgtGate_Registry.RecError[0]),PM8_A_U8( g_HCP_Context.ReadMessage.Buf ,2),2);
                if (WriteStatus != 0x00)
                {
                     SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
                }
                else
                {
                     SWP_HCP_SetResponseHeader( HCP_REP_ANY_OK );
                }

                return SWP_HCP_OK;
         }
        else
        {
            SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
           return SWP_HCP_OK;
        }
    }
    // Identity management gate case
    else if ( g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].DestinationGId == HCP_GATE_ID_IDENTITY_MGT )
    {
        if(g_HCP_Context.ReadMessage.Buf[1]<= HCP_IDT_MGT_GATE_REG_INDEX_GTLIST)
        {
            // Identity mgt gate is used by the host controller to get info
            // about the host ( UICC ) => there should not be set_parameter sent to this gate
            SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_REG_ACCESS_DENIED );
            return SWP_HCP_OK;
        }
        else
        {
            SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_REG_PAR_UNKNOWN );
            return SWP_HCP_OK;
        }
    }
    else if ( g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].DestinationGId >= 0x10 )
    {
       ;
    }
    else
    {
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_CMD_NOT_SUPPORTED );
    }
    return SWP_HCP_OK;
}

static u32 ProcessAnyGetParam(void)
{
//    u1 *OutputBuf;
//    u4 OutputSize;

    if (g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State != HCP_PIPE_OPENED)
    {
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_PIPE_NOT_OPENED );
        return SWP_HCP_OK;
    }
    // if the Message size does not contains the index then return error
    if (( g_HCP_Context.ReadMessage.Size != 2 ) ) ///< size should be 1 B for header + 1 B for index
    {
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
        return SWP_HCP_OK;
    }

    if ( g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].DestinationGId == HCP_GATE_ID_LINK_MGT )
    {
        if (( g_HCP_Context.ReadMessage.Buf[1] != 0x01 )) ///< only valid index is 1
        {
            SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_CMD_PAR_UNKNOWN );
            return SWP_HCP_OK;
        }

        // In first byte of answer message write Type (response) + instruction (any_ok)
        SWP_HCP_SetResponse( HCP_REP_ANY_OK ,  (u8 *)&g_HCP_Context.NvmContext->LinkMgtGate_Registry.RecError[0] , 2 );
        return SWP_HCP_OK;
    }
    else if ( g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].DestinationGId == HCP_GATE_ID_IDENTITY_MGT )
    {

        switch (g_HCP_Context.ReadMessage.Buf[1])
        // Now Get the requested directory shoudl be in the second byte of the HCP command ( g_HCP_Context.ReadMessage.Buf[1] )
        {
            case HCP_IDT_MGT_GATE_REG_INDEX_GTLIST:
                SWP_HCP_SetResponse( HCP_REP_ANY_OK , g_HCP_Context.NvmContext->IdentityMgtGate_Registry.GatesList , HCP_GATES_LIST_SIZE );
                break;
            case HCP_IDT_MGT_GATE_REG_INDEX_VERSW:
                SWP_HCP_SetResponse( HCP_REP_ANY_OK , g_HCP_Context.NvmContext->IdentityMgtGate_Registry.VersionSw , 3 );
                break;
            case HCP_IDT_MGT_GATE_REG_INDEX_VERHW:
                SWP_HCP_SetResponse( HCP_REP_ANY_OK , g_HCP_Context.NvmContext->IdentityMgtGate_Registry.VersionHw , 3 );
                break;
            case HCP_IDT_MGT_GATE_REG_INDEX_VERHCI:
                SWP_HCP_SetResponse( HCP_REP_ANY_OK , (u8*)&g_HCP_Context.NvmContext->IdentityMgtGate_Registry.HciVersion , 1 );
                break;
            case HCP_IDT_MGT_GATE_REG_INDEX_MODELID:
                SWP_HCP_SetResponse( HCP_REP_ANY_OK , (u8*)&g_HCP_Context.NvmContext->IdentityMgtGate_Registry.ModelId , 1 );
                break;
            case HCP_IDT_MGT_GATE_REG_INDEX_VNDNAME:
                SWP_HCP_SetResponse( HCP_REP_ANY_OK , g_HCP_Context.NvmContext->IdentityMgtGate_Registry.VendorName , 8 );
                break;

             default:
                SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_CMD_PAR_UNKNOWN );
                break;
        }
    }
    else if ( g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].DestinationGId >= 0x10 )
    {
       ;
    }
    else
    {
        // no callback registered
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_CMD_NOT_SUPPORTED );
    }

    return SWP_HCP_OK;
}

//-------------------------------------------------------------------------
//  Function Name : ProcessAnyOpenPipe
/// @brief  This function manages HCP command
///
/// @note \b Local \b Function.
///
/// \retval 2 cases: 0 #SWP_HCP_OK, #SWP_HCP_INVALID_PARAMETER
//-------------------------------------------------------------------------
static u32 ProcessAnyOpenPipe( void )
{
    u8 WriteStatus;

    // check parameter size
    if ( g_HCP_Context.ReadMessage.Size != 1 ) ///< size should be 1 B for header + 1 B for index + 2B for parameter
    {
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
        return SWP_HCP_OK;
    }
    if (g_HCP_Context.CurrentIndexInHCP_Pipes == HCP_PIPE_NOT_CREATED)
    {
        // the pipe doesn't exist
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
        return SWP_HCP_OK;
    }

    // We have stored the current PipeId. 1st the cirrent pipeId shoudl be closed
    if (g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State == HCP_PIPE_NOT_CREATED)
    {
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOT_CONNECTED );
        return SWP_HCP_OK;
    }
    else if (g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State == HCP_PIPE_CLOSED)
    {
        // update the PipeID and open it
			  WriteStatus = ecpNvm_WriteOneByte((u8 *) (&g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State), HCP_PIPE_OPENED);
        //WriteStatus = Nvm_Write(( (u1 *)( &PipeState ) ),1,(u1 *)&g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State );
// 		    WriteStatus = NVM_Prog1Byte( PipeState, (u1 *)&g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State );
					if (WriteStatus != 0x00)
					{
							 SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
					}
					else
					{
							// answer ANY_OK with the number of pipes already opened to the current destination gate
							SWP_HCP_SetResponseHeader( HCP_REP_ANY_OK );
							g_HCP_Context.WriteMessage.Buf[1] = SWP_HCP_CountOpenedPipes( g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].DestinationGId );
							g_HCP_Context.WriteMessage.Size += 1 ;
					}
        return SWP_HCP_OK;
    }
    else
    {
        // means that the pipe is still opened so we cannot complete the command
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
    }
    return SWP_HCP_OK;
}

//-------------------------------------------------------------------------
//  Function Name : ProcessAnyClosePipe
/// @brief  This function manages AnyClosePipe received command
///
/// @note \b Local \b Function.
///
/// \retval 2 cases: 0 #SWP_HCP_OK, #SWP_HCP_INVALID_PARAMETER
//-------------------------------------------------------------------------
static  u32 ProcessAnyClosePipe( void )
{
    u8 WriteStatus;

    // check parameter size
    if ( g_HCP_Context.ReadMessage.Size != 1 ) ///< size should be 1 B for header + 1 B for index
    {
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
        return SWP_HCP_OK;
    }
    if (g_HCP_Context.CurrentIndexInHCP_Pipes == HCP_PIPE_NOT_CREATED)
    {
       // means that the pipe requested number unexist
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
        return SWP_HCP_OK;
    }

    // We have stored the current PipeId. 1st the current pipeId shoudl be opened
    if (g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State == HCP_PIPE_NOT_CREATED)
    {
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOT_CONNECTED );
        return SWP_HCP_OK;
    }
    else if (g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State == HCP_PIPE_OPENED)
    {
        // update the PipeID and close it
			     WriteStatus = ecpNvm_Fill((u8 *) (&g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State), HCP_PIPE_CLOSED,1);
//         WriteStatus = MemNvmOperation_Write(( (u1 *)( &PipeState ) ),1,(u1 *)&g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State );
						if (WriteStatus != 0x00)
						{
								 SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
								 return SWP_HCP_OK;
						}
						else
						{
								// We suppose that only one PipeId is with this gate so
								// In first byte of answer message write Type (response) + instruction (any_ok)
								SWP_HCP_SetResponseHeader( HCP_REP_ANY_OK );
								return SWP_HCP_OK;
						}
    }
    else
    {
        // means that the pipe is closed so we cannot complete the command we have to informed that
        // the gate has a closed pipe
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_PIPE_NOT_OPENED );
    }

    return SWP_HCP_OK;
}


//-------------------------------------------------------------------------
//  Function Name : SWP_HCP_ProcessAdmNotify
/// @brief  This function manages all Adm_Notify received command
///
/// @param[in]  MessageSize expected command size
/// @note \b Local \b Function.
///
/// \retval 2 cases: 0 #SWP_HCP_OK, #SWP_HCP_PROCESS_COMMMAND_FINISHED
//-------------------------------------------------------------------------
u32 ProcessAdmNotify(u8 MessageSize )
{
    // check parameter size
    ///< size should be: 1 B for header + 5 B for notify_pipe_created command
    ///< size should be: 1 B for header + 1 B for Notify_pipe_deleted and notify_all_pipe_cleared command
    if ( g_HCP_Context.ReadMessage.Size != MessageSize )
    {
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_CMD_PAR_UNKNOWN );
        return SWP_HCP_PROCESS_COMMMAND_FINISHED;
    }
    if (g_HCP_Context.CurrentPipeId != HCP_PIPE_ID_ADMIN)
    {
       // means that the pipe requested number is not 01 HCP_ADMIN_GATE_PIPE_ID
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
        return SWP_HCP_PROCESS_COMMMAND_FINISHED;
    }

    // We have stored the current PipeId. 1st the current pipeId shoudl be opened
    if (g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State != HCP_PIPE_OPENED)
    {
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_PIPE_NOT_OPENED );
        return SWP_HCP_PROCESS_COMMMAND_FINISHED;
    }

    else if (g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State == HCP_PIPE_OPENED)
    {
        return SWP_HCP_OK;
    }

    return SWP_HCP_PROCESS_COMMMAND_ERROR;
}

static u8 SWP_HCP_IsGateInGatesList( u8 GateId )
{
    u8 i=0;

    for ( i=0 ; i < HCP_GATES_LIST_SIZE ; i++ )
    {
        if ( g_HCP_Context.NvmContext->IdentityMgtGate_Registry.GatesList[i] == GateId )
        {
            return 1;
        }
    }

    return 0;
}

static u8 Swp_Hcp_FindFreePipesArrayIndex( void )
{
    u8 i = 0;

    for ( i = 0 ; i < HCP_MAX_PIPES ; i ++ )
    {
        if ( g_HCP_Context.NvmContext->Pipes[i].PipeId == 0xFF )
        {        
            return i;//   free index in pipe array
        }
    }

    return 0xFF;
}


//-------------------------------------------------------------------------
//  Function Name : ProcessAdmNotifyPipeCreated
/// @brief  This function manages AdmNotifyPipeCreated received command
///
/// @note \b Local \b Function.
///
/// \retval 2 cases: 0 #SWP_HCP_OK, #SWP_HCP_PROCESS_COMMMAND_FINISHED
//-------------------------------------------------------------------------
u32 ProcessAdmNotifyPipeCreated( void )
{
    u8 WriteStatus;
    u8 FreePipeIndex = 0;

    // New pipe is created with PIPE_CLOSED status
    HCP_Pipe_t NewPipe={255,255,255,255,255,HCP_PIPE_CLOSED};

    // Check Source host should be different from 03 to BF
    if (   (g_HCP_Context.ReadMessage.Buf[1] >= 0x03)
        && (g_HCP_Context.ReadMessage.Buf[1] <= 0xBF))
    {
           // No available Pipes
            SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_CMD_PAR_UNKNOWN );
            return SWP_HCP_OK;
    }

    // Check that source Gate Id should be different from RFU = [06 to 0F ]
    if (  (g_HCP_Context.ReadMessage.Buf[2] >= 0x06)
       && (g_HCP_Context.ReadMessage.Buf[2] <= 0x0F))
    {
           // No available Pipes
            SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_CMD_PAR_UNKNOWN );
            return SWP_HCP_OK;
    }

    // Check Destination Gate Id should be different from RFU = [06 to 0F ]
    if (  (g_HCP_Context.ReadMessage.Buf[4] >= 0x06)
       && (g_HCP_Context.ReadMessage.Buf[4] <= 0x0F))
    {
           // No available Pipes
            SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_CMD_NOT_SUPPORTED );
            return SWP_HCP_OK;
    }

    // if Destination Gate Id is host specific or reserved for proprietary use, check that it is contained in
    // GATES_LIST of the host ( cf. ETSI TS 102 695-2 Table 4.2 SR5 )
    if (   (g_HCP_Context.ReadMessage.Buf[4] >= 0x10)
        && ! SWP_HCP_IsGateInGatesList( g_HCP_Context.ReadMessage.Buf[4] )  )
    {
           // No available Pipes
            SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_CMD_PAR_UNKNOWN );
            return SWP_HCP_OK;
    }

    // Check Created PipeId should be different from RFU = [70 to 7F ]
    if ( (g_HCP_Context.ReadMessage.Buf[5] >= 0x70) && (g_HCP_Context.ReadMessage.Buf[5] <= 0x7F))
    {
           // No available Pipes
            SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_CMD_PAR_UNKNOWN );
            return SWP_HCP_OK;
    }

#if SWP_HCP_SESSION_SAVE
    // check whether a pipe with the same ID doesn't already exist : in this case
    // the spec doesn't specify clearly what to do
    if ( GetCurrentPipeIndex( g_HCP_Context.ReadMessage.Buf[5] ) != HCP_PIPE_NOT_CREATED )
    {
           // No available Pipes
            SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
            return SWP_HCP_OK;
    }
#endif

    // Check Destination host should be parameter 3
    if (g_HCP_Context.ReadMessage.Buf[3] == HCP_UICC_HOST)
    {
        // write in Newpipe Pid Dest Gid
        ecpMem_Copy( (u8 *)&NewPipe , g_HCP_Context.ReadMessage.Buf + 1 , 5 );

        // find a free pipe element in persistent pipes array
        FreePipeIndex = Swp_Hcp_FindFreePipesArrayIndex();
        if ( FreePipeIndex == 0xFF )
        {
            // No available Pipes
            SWP_HCP_SetResponseHeader( HCP_REP_ADM_E_NO_PIPE_AVAILABLE );
            return SWP_HCP_OK;
        }

        // write NewPipe in g_HCP_Context.NvmContext->Pipes structure
        WriteStatus = ecpNvm_WriteErase(MEMtPN8(&g_HCP_Context.NvmContext->Pipes[FreePipeIndex]),MEMtPM8(&NewPipe ) ,sizeof(NewPipe));
        if (WriteStatus != 0x00)
        {
             SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
        }
        else
        {
             SWP_HCP_SetResponseHeader( HCP_REP_ANY_OK );
        }
			
        return SWP_HCP_OK;
    }
    else
    {
       // Destination host is not the good one
       SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOT_CONNECTED );
       return SWP_HCP_OK;
    }
}


//-------------------------------------------------------------------------
//  Function Name : ProcessAdmNotifyPipeDeleted
/// @brief  This function manages AdmNotifyPipeDeleted received command
///
/// @note \b Local \b Function.
///
/// \retval 2 cases: 0 #SWP_HCP_OK, #SWP_HCP_PROCESS_COMMMAND_FINISHED
//-------------------------------------------------------------------------
static  u32 ProcessAdmNotifyPipeDeleted( void )
{
    u8 WriteStatus = 0;
	u8 cpt;
    u8 PipetoDelete ;
    u8 PipeIndex = 0;
    // pipe is deleted with PIPE_CLOSED status
    //HCP_Pipe_t NewPipe={255,255,255,255,255,HCP_PIPE_NOT_CREATED/*HCP_PIPE_CLOSED*/,(SWP_HCP_GateCallBack_t )(0xFFFFFFFF)};
    // Check Destination host should be parameter 4
    PipetoDelete = g_HCP_Context.ReadMessage.Buf[1] ;
    if (PipetoDelete <= 1 )
    {
        // Sttic Pipes  not possible to delete them
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
        return SWP_HCP_OK;
    }
    // Retrieve the HCP_Pipe_t structure which has the good PipeId
    for (cpt=0;cpt<HCP_MAX_PIPES;cpt++)
    {
        if (g_HCP_Context.NvmContext->Pipes[cpt].PipeId == PipetoDelete )
        {
            PipeIndex = cpt;
            break;
        }
    }
    if ( PipeIndex == 0 )
    {
        // Pipe does not exist
        SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
        return SWP_HCP_OK;
    }
    // Clear Pipe in g_HCP_Context.NvmContext->Pipes structure
    WriteStatus = ecpNvm_Fill(MEMtPN8(&g_HCP_Context.NvmContext->Pipes[PipeIndex]), 0xFF , sizeof(HCP_Pipe_t));
    if (WriteStatus != 0x00)
    {
         SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_NOK );
    }
    else
    {
         SWP_HCP_SetResponseHeader( HCP_REP_ANY_OK );
    }
    return SWP_HCP_OK;
}


//-------------------------------------------------------------------------
//  Function Name : ProcessAdmNotifyAllPipeCleared
/// @brief  This function manages AdmNotifyAllPipeCleared received command
///
/// @note \b Local \b Function.
///
/// \retval 2 cases: 0 #SWP_HCP_OK,
//-------------------------------------------------------------------------
static u32 ProcessAdmNotifyAllPipeCleared( void )
{
    u8 RequestedHost ;
	  SWP_HCP_NVM_Context_t tempHcpContext;

    // pipe is deleted with PIPE_CLOSED status
//    HCP_Pipe_t NewPipe={255,255,255,255,255,HCP_PIPE_NOT_CREATED};

    // Check Destination host should be parameter 4
    RequestedHost = g_HCP_Context.ReadMessage.Buf[1] ;
    ecpMem_Copy((u8*)&tempHcpContext, (u8*)g_HCP_Context.NvmContext, sizeof(SWP_HCP_NVM_Context_t));
	  // erase all pipes except the two static pipes
    ecpMem_Set((u8 *)& ( tempHcpContext.Pipes[2] ),  255, HCP_SIZEOF_PIPE * ( HCP_MAX_PIPES - 2 ));
    if (RequestedHost == HCP_CONTROL_HOST )
    {
				// set administration pipe and link mgt pipe as closed
				tempHcpContext.Pipes[0].State = HCP_PIPE_CLOSED;
				tempHcpContext.Pipes[1].State = HCP_PIPE_CLOSED;
    }
	  ecpNvm_WriteErase ((u8 *)g_HCP_Context.NvmContext,(u8 *)&tempHcpContext , sizeof(SWP_HCP_NVM_Context_t));
    SWP_HCP_SetResponseHeader( HCP_REP_ANY_OK );
    return SWP_HCP_OK;
}

static u8 SWP_HCP_ProcessEvent(void)
{
	HCP_Pipe_t *ptrHcpContext;

    // by default, avoid transmitting anything
    g_HCP_Context.WriteMessage.Size = 0;

	ptrHcpContext = &g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes];

    switch ( g_HCP_Context.ReadMessage.Buf[0] & HCP_MSG_INSTRUCTION_MASK )
    {
        case HCP_EVT_POST_DATA:
            // event only applicable to loopback gate
			if (g_HCP_Context.Status >= SESSION_INIT_FINISH)
			{
				g_HCP_Context.Status = EVT_LOOKBACK;
			}
            // else ignore the event
            break;

		//event for only card aplication or APDU card mode sent data
		case HCP_CE_EVT_SEND_DATA:
			if ( (SWP_HCP_GetCurrentPipeId() == g_HCP_Context.NvmContext->Pipes[GetPipeIndex(HCP_GATE_ID_CARD_TYPEA)].PipeId) ||
				 (SWP_HCP_GetCurrentPipeId() == g_HCP_Context.NvmContext->Pipes[GetPipeIndex(HCP_GATE_ID_CARD_TYPEB)].PipeId) )
			{
				if ( (g_HCP_Card_Status != HCP_CE_EVT_CARD_DEACTIVATED) && (g_HCP_Card_Status != HCP_CE_EVT_FIELD_OFF) )
				{
					g_SwpHciEvents[EVENT_ON_SEND_DATA] = HCI_API_EVENT_ON_SEND_DATA | HCI_API_LISTENER_TRIGGER;
				}
			}
			else if (SWP_HCP_GetCurrentPipeId() == g_HCP_Context.NvmContext->Pipes[GetPipeIndex(HCP_GATE_ID_CONNECTIVITY)].PipeId)
			{
				g_SwpHciEvents[EVENT_STAND_BY] = HCI_API_EVENT_STAND_BY | HCI_API_LISTENER_TRIGGER;
			}
			else if ( (SWP_HCP_GetCurrentPipeId() == g_HCP_Context.NvmContext->Pipes[GetPipeIndex(HCP_GATE_ID_READER_TYPEA)].PipeId) ||
				      (SWP_HCP_GetCurrentPipeId() == g_HCP_Context.NvmContext->Pipes[GetPipeIndex(HCP_GATE_ID_READER_TYPEB)].PipeId) )
			{
				g_SwpHciEvents[EVENT_TARGET_DISCOVERED] = HCI_API_EVENT_TARGET_DISCOVERED | HCI_API_LISTENER_TRIGGER;
			}

			//如果在SWP初始化过程中，终端过快发送send data命令，则认为初始化流程已经完成
			g_HCP_Context.Status = EVT_SEND_DATA;
			break;

		case HCP_CE_EVT_FIELD_ON:
			g_HCP_Card_Status = HCP_CE_EVT_FIELD_ON;
			break;
		case HCP_CE_EVT_CARD_ACTIVATED:
			g_HCP_Card_Status = HCP_CE_EVT_CARD_ACTIVATED;
			break;
		case HCP_CE_EVT_CARD_DEACTIVATED:
		case HCP_CE_EVT_FIELD_OFF:
			if (g_HCP_Card_Status != HCP_CE_EVT_FIELD_OFF)
			{
				g_SwpHciEvents[EVENT_FIELD_OFF] = HCI_API_EVENT_FIELD_OFF | HCI_API_LISTENER_TRIGGER;
			}
			g_HCP_Card_Status = HCP_CE_EVT_FIELD_OFF;
			
			//set flag to 1, only field off event need to process
			g_HCP_NeedProcess = 1;
			break;
       default:
            break;
    }
    return SWP_HCP_OK;
}

static u32 SWP_HCP_ProcessResponse(void)
{
		// for HCI API
		if (g_SwpHciEvents[EVENT_GET_PARAMETER_RESPONSE] == HCI_API_EVENT_GET_PARAMETER_RESPONSE)
		{
			g_SwpHciEvents[EVENT_GET_PARAMETER_RESPONSE] |= HCI_API_LISTENER_TRIGGER;
		}
		else if (g_SwpHciEvents[EVENT_WRITE_EXCHANGE_DATA_RESPONSE] == HCI_API_EVENT_WRITE_EXCHANGE_DATA_RESPONSE)
		{
			g_SwpHciEvents[EVENT_WRITE_EXCHANGE_DATA_RESPONSE] |= HCI_API_LISTENER_TRIGGER;
		}
		else if ( g_HCP_Context.sessionStatus.responseStatus == INIT_WAIT_FOR_RESPONSE )
		{
			//__disable_irq();
			g_HCP_Context.sessionStatus.responseStatus = INIT_RESPONSE_PROCESS;
			//__enable_irq();
		}

    return SWP_HCP_OK;
}

u32 SWP_HCP_ProcessCommand(void)
{
    switch ( SWP_HCP_ExtractResponse( ) )
    {
         // generic commands
        //
        case HCP_CMD_ANY_SET_PARAMETER:
            ProcessAnySetParam();
            break;

        case HCP_CMD_ANY_GET_PARAMETER:
            ProcessAnyGetParam();
            break;

        case HCP_CMD_ANY_OPEN_PIPE:
            ProcessAnyOpenPipe();
            break;
        case HCP_CMD_ANY_CLOSE_PIPE:
            ProcessAnyClosePipe();
            break;

        //
        // administration commands
        //
        case HCP_CMD_ADM_NOTIFY_PIPE_CREATED:
            //
            if (ProcessAdmNotify(ADM_NOTIFY_PIPE_CREATED_PARAM_SIZE)==SWP_HCP_OK) {
                ProcessAdmNotifyPipeCreated();
            }
            break;

        case HCP_CMD_ADM_NOTIFY_PIPE_DELETED:
            // get gate ID, deduce pipe
            if (ProcessAdmNotify(ADM_NOTIFY_PIPE_DELETED_PARAM_SIZE)==SWP_HCP_OK) {
                ProcessAdmNotifyPipeDeleted();
            }
            break;

        // CLEAR_ALL_PIPE : not applicable to a host

        case HCP_CMD_ADM_NOTIFY_ALL_PIPE_CLEARED:
            // get gate ID, deduce pipe
            if (ProcessAdmNotify(ADM_NOTIFY_ALL_PIPE_CLEARED_PARAM_SIZE)==SWP_HCP_OK) {
                ProcessAdmNotifyAllPipeCleared();
            }
            break;

        default:
            // command not supported
            SWP_HCP_SetResponseHeader( HCP_REP_ANY_E_CMD_NOT_SUPPORTED );
            break;

    }

	if (!SWP_HCP_IsMessageSent( ))
	{
		SWP_HCP_SendPacket( g_HCP_Context.CurrentPipeId );
	}

    return SWP_HCP_OK;
}

static void SWP_HCP_AdmCreatePipe ( u8 SrcGateId , u8 DestHostId, u8 DestGateId )
{
    // write ADM_CREATE_PIPE command
    SWP_HCP_SetCommandHeader( HCP_CMD_ADM_CREATE_PIPE );
    g_HCP_Context.WriteMessage.Buf[1] = SrcGateId;
    g_HCP_Context.WriteMessage.Buf[2] = DestHostId;
    g_HCP_Context.WriteMessage.Buf[3] = DestGateId;
    g_HCP_Context.WriteMessage.Size += 3;

    // send command
    SWP_HCP_InitMessageTransmission( );
    SWP_HCP_SendPacket( HCP_PIPE_ID_ADMIN );
    // wait for reception of response from host controller
}


//-------------------------------------------------------------------------
//  Function Name : SWP_HCP_AdmDeletePipe
/// @brief  This function sends a SWP_HCP_AdmDeletePipe command to the CLF.
/// @param[in]  PipeId Pipe ID to be deleted
///
/// @note \b Global \b Function.
///
/// \retval cases: Reponse code ( HCO 6.2 )
//-------------------------------------------------------------------------
static void SWP_HCP_AdmDeletePipe ( u8 PipeId )
{

    SWP_HCP_SetCommandHeader( HCP_CMD_ADM_DELETE_PIPE );
    g_HCP_Context.WriteMessage.Buf[1] = PipeId;
    g_HCP_Context.WriteMessage.Size += 1;
    // send command
    SWP_HCP_InitMessageTransmission( );
    SWP_HCP_SendPacket( HCP_PIPE_ID_ADMIN );
    // wait for reception of response from host controller
}

//-------------------------------------------------------------------------
//  Function Name : SWP_HCP_AdmClearAllPipe
/// @brief  This function sends a SWP_HCP_AdmClearAllPipe command to the CLF.
/// @param[in]  IdentityRefData Identity reference data ( HCI 6.1.3.5 )
///
/// @note \b Global \b Function.
///
/// \retval cases: Reponse code ( HCO 6.2 )
//-------------------------------------------------------------------------
static void SWP_HCP_AdmClearAllPipe ( u16 IdentityRefData )
{
		SWP_HCP_SetCommandHeader( HCP_CMD_ADM_CLEAR_ALL_PIPE );
		*( (u16 *)& ( g_HCP_Context.WriteMessage.Buf[1] ) ) = IdentityRefData;
		g_HCP_Context.WriteMessage.Size += 2;
	  // send command
    SWP_HCP_InitMessageTransmission( );
	  SWP_HCP_SendPacket( HCP_PIPE_ID_ADMIN );
    // wait for reception of response from host controller
}


//-------------------------------------------------------------------------
//  Function Name : SWP_HCP_AnySetParameter
/// @brief  This function sends a AnySetParameter command to the CLF.
/// @param[in]  PipeId Pipe ID on which the command must be sent
/// @param[in]  Index in the registry
/// @param[in]  Registry Pointer to data to be written in the registry
/// @param[in]  Size of data to be written in the registry
///
/// @note \b Global \b Function.
///
/// \retval cases: Reponse code ( HCO 6.2 )
//-------------------------------------------------------------------------
static void SWP_HCP_AnySetParameter ( u8 PipeId , u8 Index , u8 *Registry, u8 Size )
{

    SWP_HCP_SetCommandHeader( HCP_CMD_ANY_SET_PARAMETER );
    g_HCP_Context.WriteMessage.Buf[1] = Index;
    ecpMem_Copy ((u8 *) & ( g_HCP_Context.WriteMessage.Buf[2] ) , Registry, Size );
    g_HCP_Context.WriteMessage.Size += Size + 1; // index in registry + nb of bytes to write

    // send command
    SWP_HCP_InitMessageTransmission( );
    SWP_HCP_SendPacket( PipeId );
    // wait for reception of response from host controller
}


//-------------------------------------------------------------------------
//  Function Name : SWP_HCP_AnyGetParameter
/// @brief  This function sends a SWP_HCP_AnyGetParameter command to the CLF.
/// @param[in]  PipeId Pipe ID on which the command must be sent
/// @param[in]  Index in the registry
/// @param[out]  Registry Pointer to data read in the registry
/// @param[out]  Size of data read in the registry
///
/// @note \b Global \b Function.
///
/// \retval cases: Reponse code ( HCO 6.2 )
//-------------------------------------------------------------------------
static void SWP_HCP_AnyGetParameter ( u8 PipeId , u8 Index)
{
    SWP_HCP_SetCommandHeader( HCP_CMD_ANY_GET_PARAMETER );
    g_HCP_Context.WriteMessage.Buf[1] = Index;
    g_HCP_Context.WriteMessage.Size += 1;

    // send command
    SWP_HCP_InitMessageTransmission( );
    SWP_HCP_SendPacket( PipeId );

    // wait for reception of response from host controller
}


//-------------------------------------------------------------------------
//  Function Name : SWP_HCP_AnyOpenPipe
/// @brief  This function sends a ANY_OPEN_PIPE command to the CLF.
/// @param[in]  PipeId Pipe ID to be opened
/// @param[out] NbAlreadyOpened Nb Pipe Laready opened
///
/// @note \b Global \b Function.
///
/// \retval cases: Reponse code ( HCO 6.2 )
//-------------------------------------------------------------------------
static u8 SWP_HCP_AnyOpenPipe ( u8 PipeId )
{
	u8 PipeIndex;

	if (PipeId != 0x00 && PipeId != 0x01)
	{
		PipeIndex = GetCurrentPipeIndex(PipeId);
		if (PipeIndex == HCP_PIPE_NOT_CREATED)
		{
			return HCP_PIPE_NOT_CREATED;
		}
	}
    SWP_HCP_SetCommandHeader( HCP_CMD_ANY_OPEN_PIPE );
    // send command
    SWP_HCP_InitMessageTransmission( );
    SWP_HCP_SendPacket( PipeId );
    // wait for reception of response from host controller

	return SWP_HCP_OK;
}

//-------------------------------------------------------------------------
//  Function Name : SWP_HCP_AnyClosePipe
/// @brief  This function sends a ANY_CLOSE_PIPE command to the CLF.
/// @param[in]  PipeId Pipe ID to be closed
///
/// @note \b Global \b Function.
///
/// \retval cases: Reponse code ( HCI 6.2 )
//-------------------------------------------------------------------------
static void SWP_HCP_AnyClosePipe ( u8 PipeId )
{

    SWP_HCP_SetCommandHeader( HCP_CMD_ANY_CLOSE_PIPE );
    // send command
    SWP_HCP_InitMessageTransmission( );
    SWP_HCP_SendPacket( PipeId );

    // wait for reception of response from host controller
}

static u8 SWP_HCP_CompareId ( u8* SessionId , u8* resSessionId )
{
//    u1 cpt,identical = 0;
	u32 *sessionId1 = (u32*)SessionId;
	u32 *sessionId2 = (u32*)resSessionId;


	if ( ((*sessionId1) == 0xFFFFFFFF) && (*(sessionId1 + 1) == 0xFFFFFFFF) )
	{
		return 0xFF;
	}
	else if ( ((*sessionId1) == (*sessionId2)) && (*(sessionId1 + 1) == *(sessionId2 + 1)) )
	{
		return 0x00;
	}
	return 0xFF;
}

///
/// @brief : process card and clf session init functions {
///
static u8 getPipesDataInvalid(void)
{
	u8 i;
	u8 j;
	u8 result;
	PN8 Addr;
	result = 0;
	Addr = (u8 *)&g_HCP_Context.NvmContext->Pipes[0];
	for(i = 0; i<0x5;i++)
	{
		if(PN_R_U8(Addr) == 0xFF)
		{
			result = 1;
		}
		else
		{
			return 0;
		}
		PN8_INC(Addr);
	}
	return result;
}
static u8 AdminPipeInitProcess(void)
{
	u8 para;

	switch (AdminPipeInitList[g_HCP_Context.sessionStatus.subStatus].Status)
	{
		case INIT_START:
			if(getPipesDataInvalid())
			{
				ecpNvm_WriteErase((u8 *)g_HCP_Context.NvmContext,(u8 *)&swpConfigAreaBackup[0] , sizeof(SWP_HCP_NVM_Context_t));
			}
			SWP_HCP_AnyOpenPipe(HCP_PIPE_ID_ADMIN);
			return INIT_WAIT_FOR_RESPONSE;

		case INIT_GET_SESSIONID:
			SWP_HCP_AnyGetParameter(HCP_PIPE_ID_ADMIN, HCP_ADMIN_GATE_REG_SESSION_IDENTITY);
			return INIT_WAIT_FOR_RESPONSE;

		case INIT_CLEAR_ALL_PIPES:
			// clear all existing pipes ( ETSI TS 102 695-2 5.5.4.1 RQ4 )
		    Random_number_generator(MySessionId, 2, true);
			//g_SWP_LLC_Context.SyncId.u2SId = *(u16*)&MySessionId[0];
			ecpNvm_WriteErase ((u8 *)(&g_HCP_Context.NvmContext->SyncId.u1SId[0] ),(u8 *)&MySessionId[0] , 2);
			SWP_HCP_AdmClearAllPipe ( *(u16*)&MySessionId[0] );
			return INIT_WAIT_FOR_RESPONSE;

		case INIT_OPEN_ADM_PIPE:
			SWP_HCP_AnyOpenPipe(HCP_PIPE_ID_ADMIN);
			return INIT_WAIT_FOR_RESPONSE;

//		case INIT_OPEN_LINK_PIPE:
//			break;

		case INIT_SET_WHITELIST:
			para = 0x01;
			SWP_HCP_AnySetParameter(HCP_PIPE_ID_ADMIN, HCP_ADMIN_GATE_REG_WHITELIST, &para, sizeof(para));
			return INIT_WAIT_FOR_RESPONSE;

		default:
			break;
	}

	return INIT_GOTO_NEXT_STATUS;
}


static u8 EndInitProcess(void)
{
	switch (EndInitList[g_HCP_Context.sessionStatus.subStatus].Status)
	{
		case INIT_SET_SESSIONID:
			Random_number_generator(MySessionId, 8, true);
			SWP_HCP_AnySetParameter ( HCP_PIPE_ID_ADMIN, HCP_ADMIN_GATE_REG_SESSION_IDENTITY, &MySessionId[0], 8 );
			return INIT_WAIT_FOR_RESPONSE;

		case INIT_END_OPERATION:
			SWP_HCP_SetEventHeader(HCP_EVT_HCI_END_OF_OPERATION);

			// send event
			SWP_HCP_InitMessageTransmission( );
			SWP_HCP_SendPacket( HCP_PIPE_ID_LINK_MGT );
			return INIT_FINISH;

		default:
			break;
	}

	return INIT_GOTO_NEXT_STATUS;
}

/// @brief : functions of dynamic pips create and init process {
static u8 TypeAInitProcess(void)
{
	u8 registryIndex;
	u8 PipeId, dataLen;
	u8 *dataBuf;
	u8 PipeIndex;

	if (TypeAInitList[g_HCP_Context.sessionStatus.subStatus].Status == INIT_FINISH)
	{
		return INIT_GOTO_NEXT_STATUS;
	}
	
	PipeIndex = GetPipeIndex(HCP_GATE_ID_CARD_TYPEA);
	if (PipeIndex == HCP_PIPE_NOT_CREATED)
	{
		SWP_HCP_AdmCreatePipe(HCP_GATE_ID_CARD_TYPEA, HCP_CONTROL_HOST, HCP_GATE_ID_CARD_TYPEA);
		return INIT_WAIT_FOR_RESPONSE;
	}
	
	PipeId = g_HCP_Context.NvmContext->Pipes[PipeIndex].PipeId;

	if (g_HCP_Context.NvmContext->Pipes[PipeIndex].State != HCP_PIPE_OPENED)
	{
		SWP_HCP_AnyOpenPipe(PipeId);
		return INIT_WAIT_FOR_RESPONSE;
	}

	registryIndex = TypeAInitList[g_HCP_Context.sessionStatus.subStatus].Status;

	//get set para list
	switch (registryIndex)
	{
		case HCP_RFTYPEA_GATE_REG_INDEX_UID:
			dataLen = g_HCP_Context.NvmContext->TypeA_Registry.uid[0];
			dataBuf = &g_HCP_Context.NvmContext->TypeA_Registry.uid[1];
			break;
		case HCP_RFTYPEA_GATE_REG_INDEX_SAK:
			dataLen = 1;
			dataBuf = &g_HCP_Context.NvmContext->TypeA_Registry.sak;
			break;
		case HCP_RFTYPEA_GATE_REG_INDEX_ATQA:
			dataLen = 2;
			dataBuf = &g_HCP_Context.NvmContext->TypeA_Registry.atqa[0];
			break;
		case HCP_RFTYPEA_GATE_REG_INDEX_APP_DATA:
			dataLen = g_HCP_Context.NvmContext->TypeA_Registry.appData[0];
			dataBuf = &g_HCP_Context.NvmContext->TypeA_Registry.appData[1];
			break;
		case HCP_RFTYPEA_GATE_REG_INDEX_FWI_SFGI:
			dataLen = 1;
			dataBuf = &g_HCP_Context.NvmContext->TypeA_Registry.fwi_sfgi;
			break;
		case HCP_RFTYPEA_GATE_REG_INDEX_CID:
			dataLen = 1;
			dataBuf = &g_HCP_Context.NvmContext->TypeA_Registry.cid;
			break;
		case HCP_RFTYPEA_GATE_REG_INDEX_DTR_MAX:
			dataLen = 3;
			dataBuf = &g_HCP_Context.NvmContext->TypeA_Registry.dataRate[0];
			break;
		case HCP_RFTYPEA_GATE_REG_INDEX_MODE:
			dataLen = 1;
			dataBuf = &g_HCP_Context.NvmContext->TypeA_Registry.mode;
			break;
		default:
			// type A init finish
			return INIT_GOTO_NEXT_STATUS;
	}
	SWP_HCP_AnySetParameter ( PipeId, registryIndex, dataBuf, dataLen );
	//Type A init not finish return 1
	return INIT_WAIT_FOR_RESPONSE;
}

static u8 TypeBInitProcess(void)
{
	u8 registryIndex;
	u8 PipeId, dataLen;
	u8 * dataBuf;
	u8 PipeIndex;

	if (TypeBInitList[g_HCP_Context.sessionStatus.subStatus].Status == INIT_FINISH)
	{
		return INIT_GOTO_NEXT_STATUS;
	}

	PipeIndex = GetPipeIndex(HCP_GATE_ID_CARD_TYPEB);
	if (PipeIndex == HCP_PIPE_NOT_CREATED)
	{
		SWP_HCP_AdmCreatePipe(HCP_GATE_ID_CARD_TYPEB, HCP_CONTROL_HOST, HCP_GATE_ID_CARD_TYPEB);
		return INIT_WAIT_FOR_RESPONSE;
	}
	
	PipeId = g_HCP_Context.NvmContext->Pipes[PipeIndex].PipeId;
	if (g_HCP_Context.NvmContext->Pipes[PipeIndex].State != HCP_PIPE_OPENED)
	{
		SWP_HCP_AnyOpenPipe(PipeId);
		return INIT_WAIT_FOR_RESPONSE;
	}

	registryIndex = TypeBInitList[g_HCP_Context.sessionStatus.subStatus].Status;

	switch(registryIndex)
	{
		case HCP_RFTYPEB_GATE_REG_INDEX_PUPI:
			dataLen = g_HCP_Context.NvmContext->TypeB_Registry.pupi_reg[0];
			dataBuf = &g_HCP_Context.NvmContext->TypeB_Registry.pupi_reg[1];
			break;
		case HCP_RFTYPEB_GATE_REG_INDEX_AFI:
			dataLen = 1;
			dataBuf = &g_HCP_Context.NvmContext->TypeB_Registry.afi;
			break;
		case HCP_RFTYPEB_GATE_REG_INDEX_ATQB:
			dataLen = 4;
			dataBuf = &g_HCP_Context.NvmContext->TypeB_Registry.atqb[0];
			break;
		case HCP_RFTYPEB_GATE_REG_INDEX_HIGHER_LAYER_RESPONSE:
			dataLen = g_HCP_Context.NvmContext->TypeB_Registry.higher_layer_response[0];
			dataBuf = &g_HCP_Context.NvmContext->TypeB_Registry.higher_layer_response[1];
			break;
		case HCP_RFTYPEB_GATE_REG_INDEX_DATARATE_MAX:
			dataLen = 3;
			dataBuf = &g_HCP_Context.NvmContext->TypeB_Registry.datarate[0];
			break;
		case HCP_RFTYPEB_GATE_REG_INDEX_MODE:
			dataLen = 1;
			dataBuf = &g_HCP_Context.NvmContext->TypeB_Registry.mode;
			break;

		default:
			// type B init finish
			return INIT_GOTO_NEXT_STATUS;
	}
	SWP_HCP_AnySetParameter ( PipeId, registryIndex, dataBuf, dataLen );

	return INIT_WAIT_FOR_RESPONSE;
}


static u8 TypeAReaderInitProcess(void)
{
	u8 PipeId;
	u8 ListIndex;
	u8 PipeIndex;

	if (TypeAReaderInitList[g_HCP_Context.sessionStatus.subStatus].Status == INIT_FINISH)
	{
		return INIT_GOTO_NEXT_STATUS;
	}

	PipeIndex = GetPipeIndex(HCP_GATE_ID_READER_TYPEA);
	if (PipeIndex == HCP_PIPE_NOT_CREATED)
	{
		SWP_HCP_AdmCreatePipe(HCP_GATE_ID_READER_TYPEA, HCP_CONTROL_HOST, HCP_GATE_ID_READER_TYPEA);
		return INIT_WAIT_FOR_RESPONSE;
	}
	
	PipeId = g_HCP_Context.NvmContext->Pipes[PipeIndex].PipeId;
	if (g_HCP_Context.NvmContext->Pipes[PipeIndex].State != HCP_PIPE_OPENED)
	{
		SWP_HCP_AnyOpenPipe(PipeId);
		return INIT_WAIT_FOR_RESPONSE;
	}

	ListIndex = TypeAReaderInitList[g_HCP_Context.sessionStatus.subStatus].Status;

	//get set para list
	switch (ListIndex)
	{
		default:
			break;
	}

	// type A Reader init finish
	return INIT_GOTO_NEXT_STATUS;
}


static u8 TypeBReaderInitProcess(void)
{
	u8 PipeId;
	u8 ListIndex;
	u8 PipeIndex;

	if (TypeBReaderInitList[g_HCP_Context.sessionStatus.subStatus].Status == INIT_FINISH)
	{
		return INIT_GOTO_NEXT_STATUS;
	}
	
	PipeIndex = GetPipeIndex(HCP_GATE_ID_READER_TYPEB);
	if (PipeIndex == HCP_PIPE_NOT_CREATED)
	{
		SWP_HCP_AdmCreatePipe(HCP_GATE_ID_READER_TYPEB, HCP_CONTROL_HOST, HCP_GATE_ID_READER_TYPEB);
		return INIT_WAIT_FOR_RESPONSE;
	}
	
	PipeId = g_HCP_Context.NvmContext->Pipes[PipeIndex].PipeId;
	if (g_HCP_Context.NvmContext->Pipes[PipeIndex].State != HCP_PIPE_OPENED)
	{
		SWP_HCP_AnyOpenPipe(PipeId);
		return INIT_WAIT_FOR_RESPONSE;
	}

	ListIndex = TypeBReaderInitList[g_HCP_Context.sessionStatus.subStatus].Status;

	//get set para list
	switch (ListIndex)
	{
		default:
			break;
	}

	// type B Reader init finish
	return INIT_GOTO_NEXT_STATUS;
}

static u8 ConnectivityInitProcess(void)
{
	u8 PipeId;
	u8 ListIndex;
	u8 PipeIndex;

	if (ConnectivityInitList[g_HCP_Context.sessionStatus.subStatus].Status == INIT_FINISH)
	{
		return INIT_GOTO_NEXT_STATUS;
	}

	PipeIndex = GetPipeIndex(HCP_GATE_ID_CONNECTIVITY);
	if (PipeIndex == HCP_PIPE_NOT_CREATED)
	{
		SWP_HCP_AdmCreatePipe(HCP_GATE_ID_CONNECTIVITY, HCP_TERMINAL_HOST, HCP_GATE_ID_CONNECTIVITY);
		return INIT_WAIT_FOR_RESPONSE;
	}
	
	PipeId = g_HCP_Context.NvmContext->Pipes[PipeIndex].PipeId;
	if (g_HCP_Context.NvmContext->Pipes[PipeIndex].State != HCP_PIPE_OPENED)
	{
		SWP_HCP_AnyOpenPipe(PipeId);
		return INIT_WAIT_FOR_RESPONSE;
	}

	ListIndex = ConnectivityInitList[g_HCP_Context.sessionStatus.subStatus].Status;

	//get set para list
	switch (ListIndex)
	{
		default:
			break;
	}

	// Connectivity init finish
	return INIT_GOTO_NEXT_STATUS;
}


static u8 IdentifyMgrInitProcess(void)
{
	u8 PipeId;
	u8 registryIndex;
	u8 PipeIndex;
	
 	if (PN_R_U8(JcvmConfig_GetSysConfigAddr(CLF_MODEL_ID)) == 0xff)
	{
		return INIT_GOTO_NEXT_STATUS;
	}
	
	if (IdentifyMgrInitList[g_HCP_Context.sessionStatus.subStatus].Status == INIT_FINISH)
	{
		return INIT_GOTO_NEXT_STATUS;
	}

	PipeIndex = GetPipeIndex(HCP_GATE_ID_IDENTITY_MGT);
	if (PipeIndex == HCP_PIPE_NOT_CREATED)
	{
		SWP_HCP_AdmCreatePipe(HCP_GATE_ID_IDENTITY_MGT, HCP_CONTROL_HOST, HCP_GATE_ID_IDENTITY_MGT);
		return INIT_WAIT_FOR_RESPONSE;
	}
	
	PipeId = g_HCP_Context.NvmContext->Pipes[PipeIndex].PipeId;
	if (g_HCP_Context.NvmContext->Pipes[PipeIndex].State != HCP_PIPE_OPENED)
	{
		SWP_HCP_AnyOpenPipe(PipeId);
		return INIT_WAIT_FOR_RESPONSE;
	}
	registryIndex = IdentifyMgrInitList[g_HCP_Context.sessionStatus.subStatus].Status;
	//send command
	SWP_HCP_AnyGetParameter (PipeId, registryIndex);

	return INIT_WAIT_FOR_RESPONSE;
}

static u8 cmdResponseProcess(void)
{
	u8  PipeIndex, status;
	HCP_Pipe_t NewPipe;
	u8 *resMsgData;
	SWP_HCP_NVM_Context_t tempHcpContext;
	u8 clfModelID;
	u8 pipeState;

	resMsgData = SWP_HCP_GetReadMessageDataAddr();

	status = g_HCP_Context.sessionStatus.mainStatus;

	if ((INIT_SUB_LIST*)&swpInitList[status].subStatus[g_HCP_Context.sessionStatus.subStatus] == (INIT_SUB_LIST*)0xFF)
	{
		return INIT_GOTO_NEXT_STATUS;
	}

	switch (swpInitList[status].subStatus[g_HCP_Context.sessionStatus.subStatus].cmdType)
	{
		case CMD_TYPE_OPEN_PIPE:

			if (SWP_HCP_ExtractResponse() != HCP_REP_ANY_OK)
			{
				return INIT_GOTO_NEXT_STATUS;
			}
			// write pipe as opened
			PipeIndex = GetCurrentPipeIndex( SWP_HCP_GetCurrentPipeId() );
			if ( PipeIndex != HCP_PIPE_NOT_CREATED )
			{
				if (g_HCP_Context.NvmContext->Pipes[PipeIndex].State != HCP_PIPE_OPENED)
				{
					//MemNvmOperation_Fill((u8 *) (&g_HCP_Context.NvmContext->Pipes[PipeIndex].State), HCP_PIPE_OPENED,1);
					//NVM_Prog1Byte( HCP_PIPE_OPENED, (u8 *)&g_HCP_Context.NvmContext->Pipes[PipeIndex].State );
					ecpNvm_WriteOneByte((u8 *) (&g_HCP_Context.NvmContext->Pipes[PipeIndex].State), HCP_PIPE_OPENED);
				}
			}
			break;

		case CMD_TYPE_SET_PARA:
			if (swpInitList[status].subStatus[g_HCP_Context.sessionStatus.subStatus].Status == INIT_SET_SESSIONID)
			{
				//response NOK, clear session id
				if (SWP_HCP_ExtractResponse() == HCP_REP_ANY_OK)
				{
					//MemNvmOperation_Write ((u8 *)&g_HCP_Context.NvmContext->SessionId[0], (u8 *)&MySessionId[0] , sizeof(MySessionId));
					//NVM_ProgNByte( (u8 *)&MySessionId[0] , sizeof(MySessionId) , (u8 *)&g_HCP_Context.NvmContext->SessionId[0] );
					ecpNvm_Write((u8 *)&g_HCP_Context.NvmContext->SessionId[0], (u8 *)&MySessionId[0], sizeof(MySessionId));
				}
			}
			break;

		case CMD_TYPE_GET_PARA:
			if (swpInitList[status].Status == INIT_GET_CLF_INFO_PROCESS)
			{
				if (SWP_HCP_ExtractResponse() != HCP_REP_ANY_OK)
				{
					return INIT_GOTO_NEXT_STATUS;
				}
				
				switch (swpInitList[status].subStatus[g_HCP_Context.sessionStatus.subStatus].Status)
				{
					case HCP_IDT_MGT_GATE_REG_INDEX_VNDNAME:
						
						memFillToRam(&clfName[0], 0x00, sizeof(clfName));
						if (PN_R_U8(JcvmConfig_GetSysConfigAddr(CLF_MODEL_ID_ENABLE)) == 0x01)
						{
							clfName[0] = SWP_HCP_GetReadMessageDataSize();
							memCopyToRam(&clfName[1], &resMsgData[0], SWP_HCP_GetReadMessageDataSize());
						}
						break;
					
					case HCP_IDT_MGT_GATE_REG_INDEX_MODELID:
						clfModelID = resMsgData[0];
						if (PN_R_U8(JcvmConfig_GetSysConfigAddr(CLF_MODEL_ID_ENABLE)) == 0x01)
						{
							clfModelID = checkCLFModeIDConfig(&clfName[1], clfName[0], clfModelID);
						}
						else
						{
							clfModelID = ((clfModelID==0x00) || (clfModelID>1) && (clfModelID<0x12)) ? 0 : 1 ;
						}
					#if	BOOT__IO__MIFARE
						if (PN_R_U8(JcvmConfig_GetSysConfigAddr(CLF_MODEL_ID)) != 0xFF)
						{
							ecpNvm_WriteErase(MEMtPN8(JcvmConfig_GetVmConfigAddr(CLF_MODEL_ID)), MEMtPN8(&clfModelID), 1);
						}
					#endif
						break;
				}

			}
			break;


		case CMD_TYPE_GET_SESSION_ID:

			//compare session id
			if (SWP_HCP_CompareId(g_HCP_Context.NvmContext->SessionId, resMsgData) == 0x00)
			{
				//if equal return 0xFF(break init process)
				return INIT_FINISH;
			}

			//else clear all pipe return 0
			break;
		case CMD_TYPE_CREATE_PIPE:

			if (SWP_HCP_ExtractResponse() != HCP_REP_ANY_OK)
			{
				return INIT_GOTO_NEXT_STATUS;
			}
			else if (SWP_HCP_GetReadMessageDataSize() != 0x05)
			{
				return INIT_GOTO_NEXT_STATUS;
			}
			else if ( (resMsgData[1] != g_HCP_Context.WriteMessage.Buf[1]) ||
					  (resMsgData[2] != g_HCP_Context.WriteMessage.Buf[2]) ||
					  (resMsgData[3] != g_HCP_Context.WriteMessage.Buf[3]) )
			{
				return INIT_GOTO_NEXT_STATUS;
			}
			else
			{
				// return OK for create pipe
				// write new pipe in NVM
				// The first 5 bytes are copied "as it is" in internal data structure
				//response frame : shdlcCtrl, hcpPackageHeader, hcpMsgHeader, srcHid, srcGid, dstHid, dstGid, Pid
				ecpMem_Copy( (u8 *)&NewPipe , SWP_HCP_GetReadMessageDataAddr() , 5 );
				// add two more bytes for state of pipe and callback
				NewPipe.State = HCP_PIPE_CLOSED;
				// find a free pipe in array stored in NVM
				PipeIndex = Swp_Hcp_FindFreePipesArrayIndex();
				if ( PipeIndex != 0xFF )
				{
					
					  ecpNvm_Write ((u8 *)& g_HCP_Context.NvmContext->Pipes[PipeIndex] ,(u8 *)&NewPipe , HCP_SIZEOF_PIPE);
				}
			}
			break;

		case CMD_TYPE_CLEAR_ALL_PIPE:

			ecpMem_Copy((u8*)&tempHcpContext, (u8*)g_HCP_Context.NvmContext, sizeof(SWP_HCP_NVM_Context_t));
			// set administration pipe and link mgt pipe as closed
			tempHcpContext.Pipes[0].State = HCP_PIPE_CLOSED;
			tempHcpContext.Pipes[1].State = HCP_PIPE_CLOSED;

			ecpMem_Set((u8 *)& (tempHcpContext.SessionId[0]), 0xFF, 8);
			// erase all pipes except the two static pipes
			ecpMem_Set((u8 *)& ( tempHcpContext.Pipes[2] ),  255, HCP_SIZEOF_PIPE * ( HCP_MAX_PIPES - 2 ));
			ecpNvm_WriteErase((u8 *)g_HCP_Context.NvmContext,(u8 *)&tempHcpContext , sizeof(SWP_HCP_NVM_Context_t));
			break;
		default:
			break;
	}
	g_HCP_Context.sessionStatus.subStatus++;

	return NO_OPERATION;
}

void Swp_Hcp_Init(void)
{
//	u8 i;

	g_HCP_Context.Status = SESSION_INIT_PROCESS;
	g_HCP_Context.ChainingOnGoing = 0;
	g_HCP_Context.NbBytesSent = 0;
	g_HCP_Context.ReadMessage.Size = 0;
	g_HCP_Context.WriteMessage.Size = 0;
	g_HCP_Context.CurrentPipeId = 0;
	g_HCP_Context.sessionStatus.mainStatus = INIT_START;
	g_HCP_Context.sessionStatus.subStatus = 0x00;
	g_HCP_Context.sessionStatus.responseStatus = NO_OPERATION;

	ecpMem_Set(g_SwpHciEvents, 0x00, EVENTS_NUM);
//	ecpMem_Set(g_HCP_Context.ReadMessage.Buf, 0x00, HCP_BUF_SIZE);
//	ecpMem_Set(g_HCP_Context.WriteMessage.Buf, 0x00, HCP_BUF_SIZE);
	g_HCI_API_TrigEvent = 0;
	g_HCP_NeedProcess = 0;
	g_HCP_Card_Status = 0;
	g_HCP_Context.NvmContext = (SWP_HCP_NVM_Context_t *)sysConfig_GetSwpConfigAddr();
	
	return;
}

u8 SWP_HCP_ProcessPacket(void)
{

	//当收到两条链式命令，且这两条命令的PIPE ID不一致，则只认最后一条命令
	if (g_HCP_Context.CurrentPipeId != (g_SWP_LLC_Context.ReadLpdu->Data[1] & HCP_PACKET_PIPEID_MASK))
	{
	   g_HCP_Context.ChainingOnGoing = 0;
	}

    // first we check whether it is a fragment of a message
    if ( g_SWP_LLC_Context.ReadLpdu->Data[1] & HCP_PACKET_CB_MASK )
    {
        // CB is set => no chaining or last fragment of the message
        if ( g_HCP_Context.ChainingOnGoing )
        {
            //chaining on going : it is the last fragment of the message
            g_HCP_Context.ChainingOnGoing = 0;
			g_HCP_PackageStatus = HCP_PKG_NO_HEADER;
        }
        else
        {
			g_HCP_PackageStatus = HCP_PKG_IS_HEADER;
            g_HCP_Context.ReadMessage.Size = 0;// no chaining
            g_HCP_Context.CurrentIndexInHCP_Pipes = GetCurrentPipeIndex(SWP_HCP_GetPacketPipeId()) ; // get the pipe ID of the message
			if(g_HCP_Context.CurrentIndexInHCP_Pipes == HCP_PIPE_NOT_CREATED)
			{
			   return SWP_HCP_INVALID_PARAMETER;
			}
			g_HCP_Context.CurrentPipeId = SWP_HCP_GetPacketPipeId();
        }
        // copy data from SWP ram buffer into HCP buffer, without LLC control byte and HCP packet header
        ecpMem_Copy (g_HCP_Context.ReadMessage.Buf + g_HCP_Context.ReadMessage.Size,
                    (u8 *)(g_SWP_LLC_Context.ReadLpdu)+3,g_SWP_LLC_Context.ReadLpdu->Size - 2 );
        // update size of message
        g_HCP_Context.ReadMessage.Size += g_SWP_LLC_Context.ReadLpdu->Size - 2;
    }
    else
    {
		g_HCP_PackageStatus = HCP_PKG_NO_HEADER;

        if ( ! g_HCP_Context.ChainingOnGoing )
        {
			// mask package header receive
			g_HCP_PackageStatus = HCP_PKG_IS_HEADER;
            // this is the first fragment of a HCP message -> initialize the chaining process
            g_HCP_Context.ReadMessage.Size = 0;
            g_HCP_Context.ChainingOnGoing = 1;
            // get the pipe ID of the message
            g_HCP_Context.CurrentIndexInHCP_Pipes = GetCurrentPipeIndex(SWP_HCP_GetPacketPipeId());
			if(g_HCP_Context.CurrentIndexInHCP_Pipes == HCP_PIPE_NOT_CREATED)
			{
					return SWP_HCP_INVALID_PARAMETER;
			}
            g_HCP_Context.CurrentPipeId = SWP_HCP_GetPacketPipeId() ;
        }

		// copy data from SWP ram buffer into HCP buffer, without LLC control byte and HCP packet header
		ecpMem_Copy ( g_HCP_Context.ReadMessage.Buf + g_HCP_Context.ReadMessage.Size,(u8 *)(g_SWP_LLC_Context.ReadLpdu)+3,g_SWP_LLC_Context.ReadLpdu->Size - 2 );
		// update size of message
		g_HCP_Context.ReadMessage.Size += g_SWP_LLC_Context.ReadLpdu->Size - 2;
        // nothing else to do
        return 0x2;
    }

    // at this point the message is complete, let's process it
	g_HCP_PackageStatus |= HCP_PKG_COMPLETE;
	g_HCP_NeedProcess++;
    // first check the type of instruction
    if ( ( g_HCP_Context.ReadMessage.Buf[0] & HCP_MSG_TYPE_MASK ) == HCP_MSG_COMMAND_TYPE )
    {
		//__disable_irq();
		g_HCP_Context.Status = PROCESS_COMMAND;
		//__enable_irq();
    }
    else if(( g_HCP_Context.ReadMessage.Buf[0] & HCP_MSG_TYPE_MASK ) == HCP_MSG_EVENT_TYPE )
    {
        // event message
        return  SWP_HCP_ProcessEvent();
    }
    else
    {
        // Response message
        SWP_HCP_ProcessResponse( );
    }

    return 0x2;
}

u8 SWP_HCP_SessionIsFinish( void )
{
	if (g_HCP_Context.Status >= SESSION_INIT_FINISH)
	{
		return 1;
	}

	return 0;
}

/// @addtogroup interface for SWP_HCP_API
/// @{

void Swp_Hcp_SendMessageByPipeID ( u8 PipeId )
{
    while ( !SWP_HCP_IsMessageSent( ) )
    {
				if ( (SWP_LLC_WaitForSend()) &&
						(g_SWP_LLC_Context.Status != SWP_LLC_READ_RNR_S_FRAME) &&
						(g_SWP_LLC_Context.Status != SWP_LLC_RNR_S_FRAME_RESEND) &&
						(g_SWP_LLC_Context.ResendOngoing == 0) )
				{
					SWP_HCP_SendPacket( PipeId );
					ecpSwpTimeDelay(600);  //delay ~135us
				}
    }
}

void Swp_Hcp_SendMessage (void)
{
    while ( !SWP_HCP_IsMessageSent( ) )
    {
				if ( (SWP_LLC_WaitForSend()) &&
						(g_SWP_LLC_Context.Status != SWP_LLC_READ_RNR_S_FRAME) &&
						(g_SWP_LLC_Context.Status != SWP_LLC_RNR_S_FRAME_RESEND) &&
						(g_SWP_LLC_Context.ResendOngoing == 0) )
				{
					SWP_HCP_SendPacket( g_HCP_Context.CurrentPipeId );
					ecpSwpTimeDelay(900);  //delay ~135us
				}
    }
}

void Swp_SessionInit(void)
{
	u8 result;
	u8 *main_Status = &g_HCP_Context.sessionStatus.mainStatus;
	u8 *response_Status = &g_HCP_Context.sessionStatus.responseStatus;
	if (*main_Status == INIT_FINISH)
	{
		g_HCP_Context.Status = SESSION_INIT_FINISH;
		return;
	}
	
	// if privous command is need process response
	if (*response_Status == INIT_RESPONSE_PROCESS)
	{
		//clear response status
		*response_Status = NO_OPERATION;
		// process command response
		result = cmdResponseProcess();
		if (result == INIT_FINISH)
		{
			g_HCP_Context.Status = SESSION_INIT_FINISH;
			*main_Status = INIT_FINISH;
			return;
		}
		else if (result == INIT_GOTO_NEXT_STATUS)
		{
			// goto next main status
			(*main_Status)++;
			g_HCP_Context.sessionStatus.subStatus = 0;
		}
	}

	do {
		//if current response status is no operation, get next status
				if ( *response_Status == NO_OPERATION )
				{
					if (swpInitList[*main_Status].Status == INIT_FINISH)
					{
						//if init finish ,  break;
						*main_Status = INIT_FINISH;
						return;
					}
					result = swpInitList[*main_Status].InitProcess();//get next main or sub status

					if (result == INIT_FINISH)
					{
						//if init finish ,  break;
						*main_Status = INIT_FINISH;
						return;
					}
					else if (result == INIT_GOTO_NEXT_STATUS)
					{
						// goto next status
						(*main_Status)++;
						*response_Status = NO_OPERATION;
						g_HCP_Context.sessionStatus.subStatus = 0;
					}
					else if (result == INIT_WAIT_FOR_RESPONSE)
					{
						*response_Status = result;//need for waiting response
					}
				}
	  }while (result == INIT_GOTO_NEXT_STATUS);
}

void SWP_HCP_ResetHCPStatus(void)
{
	g_HCP_Context.Status = SESSION_INIT_FINISH;
}

u8 SWP_HCP_isNeedProcessEvent(void)
{
	return (g_HCP_NeedProcess != 0);
}

void setSwpTypeAPara(u8 index, u8 *data, u8 size)
{
	if (SWP_LLC_SHDLCLinkEstablished())
	{
		SWP_HCP_AnySetParameter (g_HCP_Context.NvmContext->Pipes[GetPipeIndex(HCP_GATE_ID_CARD_TYPEA)].PipeId, index, data, size );
	}
}

#endif
