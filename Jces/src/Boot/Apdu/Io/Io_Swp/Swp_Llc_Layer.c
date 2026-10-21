#include "Swp_Llc_Layer.h"
#include "Swp_Hcp_Layer.h"
#include "SwpTimer.h"
#include "SwpHal.h"
#include "ecpDefine.h"
#include "ecpOperation.h"
#include "sysConfigTarget.h"
#include "BootNoAreaInitRam.h"
#include "chipHead.h"
#include "SysHal.h"
#include "swpDataConfig.h"

#if BOOT__IO__SWP
SWP_LLC_t g_SWP_LLC_Context;
//u8 swpActivateSuccess;
RFDISP_Context_t g_RFDISP_Context;
#if		HAL__PLATFORM__CHIP_ARM
#pragma arm section rwdata
#endif
//#pragma arm section rwdata = "JC_SWP_RODATA"
//SWP_LLC_SyncId_t g_MGR_SyncId = {0xFADA};
//#pragma arm section rwdata


static void SwpHcpSessionInit( void )
{
		g_HCP_Context.Status = SESSION_INIT_PROCESS;
		g_HCP_Context.sessionStatus.mainStatus = INIT_START;
		g_HCP_Context.sessionStatus.subStatus = 0x00;
		g_HCP_Context.sessionStatus.responseStatus = NO_OPERATION;
}

/****************************following functions are for LLC*****************************************/

// void setLlcContextSyncId(u32 syncId)
// {
// 		g_SWP_LLC_Context.SyncId.u1SId[0] = U32t8(U32_SHR_U8(U32_AND_U32(syncId , 0x0000FF00) , 8));//pSyncId[1];
// 		g_SWP_LLC_Context.SyncId.u1SId[1] = U32t8(U32_SHR_U8(U32_AND_U32(syncId , 0x00FF0000) , 16));//pSyncId[2];
// }

void setLlcContextStatus(u8 status)
{
	g_SWP_LLC_Context.Status = status;
}

void Swp_Llc_Init( void )
{
	u8     i;
//	g_SWP_LLC_Context.SyncId.u2SId = g_MGR_SyncId.u2SId; 
//    swpActivateSuccess = 0 ;
    g_SWP_LLC_Context.Ns = 0;
    g_SWP_LLC_Context.Nr = 0;
    g_SWP_LLC_Context.DNr = 0;
	g_SWP_LLC_Context.privNr = 0xFF;
    g_SWP_LLC_Context.Timer.Status = NO_TIMEOUT;
    g_SWP_LLC_Context.Timer.Timercdv = 0;
	 // g_SWP_LLC_Context.SWP_Bit_Duration = (1UL << SWP_LLC_590NS_SUPPORTED) | (1UL << SWP_LLC_10000NS_SUPPORTED);
  	g_SWP_LLC_Context.PowerMode = SWP_LLC_LOW_POWER_MODE;
    g_SWP_LLC_Context.WS = DEFAULT_WS; // Let's Initialize WS to 2 and no Support of Srej
    g_SWP_LLC_Context.NbRetransmit = 0;
    g_SWP_LLC_Context.ResendOngoing = 0;// init flag for resent
	g_SWP_LLC_Context.IrqOccured = 0; // Initialize  no SWP_irq_occurs
	for (i=0; i<DEFAULT_WS; i++)
	{
		g_SWP_LLC_Context.ResendLpdu[i].Size = 0xFF;
	    ecpMem_Set(&g_SWP_LLC_Context.ResendLpdu[i].Data[0], 0x00, LLC_BUF_SIZE);
	}
    g_SWP_LLC_Context.Status = SWP_LLC_INITIALIZED;   // lets initialize SWP_LLC_Layer structure
    
    //The LLC layer which is upper layer call directly MAC Layer for initialization
    Swp_Mac_Init( );
    // Prepare SWP channel to reduce SWP_ACTIVATE duration
    Swp_Llc_Close();
    Swp_Llc_Open();
    Swp_Llc_Act_Send();

//     // to be compliant with etsi test 5.3.2.3.4
    swpTimer_SwpTimerStart( TS2_INHIBIT_TIMEOUT );
//     // Enable SWP IT
 	Swp_EnableInterrupt();//r_IRQ_0_31_SET_ENABLE = IRQ_0_31_SWP;
  
    return ;
}

void Swp_Llc_Close( void )
{
    Swp_Mac_Close( );
    // lets Close SWP_LLC_Layer structure
    g_SWP_LLC_Context.Status = SWP_LLC_CLOSED;

    return ;
}

void Swp_Llc_Open ( void )
{
    // lets Open SWP_LLC_Layer structure. Open is done when we enter in ACTIVATE Mode
    g_SWP_LLC_Context.Status = SWP_LLC_OPENED;
    // The LLC layer which is upper layer call directly MAC Layer for opening
    Swp_Mac_Open( );

    g_SWP_LLC_Context.Ns = 0;
    g_SWP_LLC_Context.Nr = 0;
    g_SWP_LLC_Context.DNr = 0;
	g_SWP_LLC_Context.privNr = 0xFF;

	// 检测到激活信号后停止 TS2_INHIBIT_TIMEOUT
	swpTimer_SwpTimerStop( );

    // set correctly the SWP_TIMER constraints
    g_SWP_LLC_Context.Timer.Status = NO_TIMEOUT;
    g_SWP_LLC_Context.Timer.Timercdv = 0;

    // clear potential resent sequence ongoing
    Swp_Llc_ClearResendFrame();
	
	g_SWP_LLC_Context.ReadLpdu  = ( SWP_LLC_Lpdu_t * )Swp_GetReceiveRegisterAddr();
    g_SWP_LLC_Context.WriteLpdu = ( SWP_LLC_Lpdu_t * )Swp_GetSendRegisterAddr();
    return ;
}

void Swp_Llc_ClearResendFrame(void)
{
	u8 i;

	for(i = 0; i < 4; i++)
	{
		g_SWP_LLC_Context.ResendLpdu[i].Size = 0xFF;
	}
	g_SWP_LLC_Context.ResendOngoing = FALSE;
}


//-------------------------------------------------------------------------
//  Function Name : Swp_Llc_ClearFrame
/// @brief  This function Send Sframe to CLF
/// @note \b Global \b Function.
///
///
///
/// \retval
///
//-------------------------------------------------------------------------

void Swp_Llc_ClearFrame( void )
{
	u8 i, j;
//	u8 Nr;
	SWP_LLC_Lpdu_t *resendLpdu;

	resendLpdu = (SWP_LLC_Lpdu_t*)&g_SWP_LLC_Context.ResendLpdu[0];

	// because this Nr is host ready to receive the next frame, so need to - 1
	for(i = 0; i < g_SWP_LLC_Context.WS; i++)
	{
		for(j = 0; j < g_SWP_LLC_Context.WS; j++)
		{
			if(g_SWP_LLC_Context.DNr >= g_SWP_LLC_Context.WS)
			{
				if( ((resendLpdu[i].Data[0] & SHDLC_NS_MASK) >> 3) == ( (g_SWP_LLC_Context.DNr - g_SWP_LLC_Context.WS + j) & 0x07 ))
				{
					resendLpdu[i].Size = 0xFF;
					break;
				}
			}
			else// if(g_SWP_LLC_Context.DNr < g_SWP_LLC_Context.WS)
			{
				if( ((resendLpdu[i].Data[0] & SHDLC_NS_MASK) >> 3) == ( (8 + g_SWP_LLC_Context.DNr - g_SWP_LLC_Context.WS + j) & 0x07 ))
				{
					resendLpdu[i].Size = 0xFF;
					break;
				}
			}
		}
	}
}

//-------------------------------------------------------------------------
//  Function Name : Swp_Llc_StoreFrame
/// @brief  This function Send Sframe to CLF
/// @note \b Global \b Function.
///
/// @param[in] swp frame data
/// @param[in] data size
///
/// \retval
///
//-------------------------------------------------------------------------

void Swp_Llc_StoreFrame(u8 *Data, u8 size)
{
	u8 i;

	for (i=0; i<g_SWP_LLC_Context.WS; i++)
	{
		if (g_SWP_LLC_Context.ResendLpdu[i].Size == 0xFF)
		{
			ecpMem_Copy(&g_SWP_LLC_Context.ResendLpdu[i].Data[0], Data, size);
			g_SWP_LLC_Context.ResendLpdu[i].Size = size;
			break;
		}
	}
}

int SWP_LLC_Reset(SWP_LLC_t* StateLlc)
{
	  StateLlc->Ns = 0;
    StateLlc->Nr = 0;
    StateLlc->DNr = 0;
	  return SWP_LLC_OK;
}

//-------------------------------------------------------------------------
//  Function Name : SWP_LLC_GetStatus
/// @brief  This function return the Status of the SWP_LLC layer.
/// @param[in]  StateLlc SWP_LLC_layer structure
///
/// @note \b Global \b Function.
///
/// \retval this return SWP_LlcT::tatus. It can be  #SWP_LLC_INITIALIZED,
///   #SWP_LLC_CLOSED,  #SWP_LLC_OPENED, #SWP_LLC_ACT_SYNC_SENT,
///  #SWP_LLC_ACT_STOPPED, #SWP_LLC_ACT_READY_SENT
///   #SWP_LLC_SHD_RSET_RECEIVED,#SWP_LLC_SHD_RSET_SENT, #SWP_LLC_SHD_UA,
///  #SWP_LLC_SHD_I_FRAME, #SWP_LLC_READ_I_FRAME
///
//-------------------------------------------------------------------------
int SWP_LLC_GetStatus(SWP_LLC_t* StateLlc)
{
    return StateLlc->Status ;
}

//-------------------------------------------------------------------------
//  Function Name : SWP_LLC_SetStatus
/// @brief  This function Modify the Status of the SWP_LLC layer if it is p魋sible
/// @param[in]  StateLlc SWP_LLC_layer structure
/// @param[in]  Status Status to be set
///
/// @note \b Global \b Function.
///
/// \retval #SWP_LLC_OK or #SWP_LLC_SETSTATUS_ERROR
///
//-------------------------------------------------------------------------
int SWP_LLC_SetStatus(SWP_LLC_t* StateLlc,int Status)
{
    if( (Status != SWP_LLC_INITIALIZED) || (Status != SWP_LLC_CLOSED) || (Status != SWP_LLC_OPENED) ||
        (Status != SWP_LLC_ACT_SYNC_SENT) ||(Status != SWP_LLC_ACT_STOPPED) ||(Status != SWP_LLC_ACT_READY_SENT) ||
        (Status != SWP_LLC_SHD_RSET_RECEIVED) ||(Status != SWP_LLC_SHD_RSET_SENT) ||(Status != SWP_LLC_SHD_UA))
    {
        return SWP_LLC_SETSTATUS_ERROR;
    }
    else     // depending on the Status value change other things in the SWP_LLC_Layer
    // to be Updated....
 		{
			  StateLlc->Status = Status;
		}
    return SWP_LLC_OK;
}


/****************************following functions are for SHDLC*****************************************/
u8 Swp_Llc_Shdlc_Resend( u8 Ns )
{
	u8 i, flag = 0;

	u8 *pBuffer;
	u8 *size;

	//get write frame data and size
	pBuffer = &g_SWP_LLC_Context.WriteLpdu->Data[0];
	size = &g_SWP_LLC_Context.WriteLpdu->Size;

	// get a frame which as same the Nr number from REJ buffer
	for(i = 0; i < g_SWP_LLC_Context.WS; i++)
	{
		if(g_SWP_LLC_Context.ResendLpdu[i].Size != 0xFF)
		{
			if (Ns == 0xFF)		//timer T2 , retransmit these frames
			{
				if ( ((g_SWP_LLC_Context.ResendLpdu[i].Data[0] & SHDLC_NS_MASK) >> 3) == g_SWP_LLC_Context.DNr )
				{
					flag = 1;
					break;
				}
			}
			else if (((g_SWP_LLC_Context.ResendLpdu[i].Data[0] & SHDLC_NS_MASK) >> 3) == Ns)		//handle REJ frame
			{
				flag = 1;
				break;
			}
			else
			{
				;
			}
		}
	}

	if (flag)
	{
		//__disable_irq();

		ecpMem_Copy(pBuffer, &g_SWP_LLC_Context.ResendLpdu[i].Data[0], g_SWP_LLC_Context.ResendLpdu[i].Size);
		//*size = g_SWP_LLC_Context.ResendLpdu[i].Size;
		*size = g_SWP_LLC_Context.ResendLpdu[i].Size;
		Swp_Mac_SendFrame(MEMtPX8(&pBuffer));

		//update Ns
		g_SWP_LLC_Context.Ns = ( ((g_SWP_LLC_Context.ResendLpdu[i].Data[0] & SHDLC_NS_MASK) >> 3) + 1 ) % 8;

		//__enable_irq();

		//start T2 Timer
		swpTimer_SwpTimerStart( T2_GUARDING_TIMEOUT );

		g_SWP_LLC_Context.ResendOngoing = 1;
	}
	else
	{
		g_SWP_LLC_Context.ResendOngoing = 0;
	}

	return flag;
}

u8 Swp_Llc_Send_StypeFrame( u8 type)
{
    g_SWP_LLC_Context.WriteLpdu->Size      = 1;
   g_SWP_LLC_Context.WriteLpdu->Data[0]   = SHDLC_S_FRAME | ( type & TYPE_SFRAME_MASK )|g_SWP_LLC_Context.Nr ;
    return Swp_Mac_SendFrame(MEMtPX8(&g_SWP_LLC_Context.WriteLpdu));
}



/*-------------------------------------------------------------------------
 Function Name : SWP_LLC_SHDLC_CheckDNR
 \b Purpose      Preload a value in timer2,.do not select the autoreload mode,
 select the external clock, enable the timer 2 IT, then, run the timer2.

 @param[in]  StateLlc SWP_LLC_layer structure
 @param[in]  Y          CLF Nr received to be checked     eq to Ix,y ==> y

 @note \b Global \b Function
-------------------------------------------------------------------------*/
void Swp_Llc_Shdlc_CheckDNR(u8 Y, u8 Rej)
{
    u8 restest = 0;

    if (Rej == SHDLC_REJ_FALSE)
    {
        if (g_SWP_LLC_Context.DNr <= g_SWP_LLC_Context.Ns)
        {
            if (((g_SWP_LLC_Context.DNr < Y)) && (Y <= g_SWP_LLC_Context.Ns))
            {
                restest= 1;
            }
        }
        else if  ((Y > g_SWP_LLC_Context.DNr ) || (Y <= g_SWP_LLC_Context.Ns))
        {
            {
                restest= 1;
            }
        }

        if(restest == 1)
        {
            // Deactivate all T2 for Frames DN(r) to Y-1
            // Stop T2 timeout
						if( g_SWP_LLC_Context.Timer.Status == T2_GUARDING_TIMEOUT )
						{
							//	swpTimer_SwpTimerStop();
						}
            // Reset the maximum number of retransmission
            g_SWP_LLC_Context.NbRetransmit = 0;
            // update DNr
            g_SWP_LLC_Context.DNr = Y;
        }
    }
    else
    {
		//收到REJ帧时的处理
        // it Means we are in rejection management mode
        if (g_SWP_LLC_Context.DNr <= g_SWP_LLC_Context.Ns)
        {
            if (((g_SWP_LLC_Context.DNr <= Y)) && (Y < g_SWP_LLC_Context.Ns))
            {
                restest= 1;
            }
        }
        else if ((Y >= g_SWP_LLC_Context.DNr ) ||(Y < g_SWP_LLC_Context.Ns))
        {
            {
                restest= 1;
            }
        }
        // We are in the case where we have lost frame
        if(restest == 1)
        {
            // Deactivate all T2 for Frames DN(r) to Y-1
            // Stop T2 timeout
          //  swpTimer_SwpTimerStop( );

            // Reset the maximum number of retransmission
            g_SWP_LLC_Context.NbRetransmit = 0;
            // Here we should retransmit Frames if necessary
            //SWP_MAC_RejMgt();
            // update DNr
            g_SWP_LLC_Context.DNr = Y;
            // Update Ns we manage Rej Case
            g_SWP_LLC_Context.Ns = Y;
			//g_SWP_LLC_Context.WaitFor_RR = 0;
        }
    }
	  Swp_Llc_ClearFrame();
}



//-------------------------------------------------------------------------
//  Function Name : SWP_LLC_SHDLC_CheckReceivedIFrame
/// @brief  This function Check NS and NR receveid from CLF
/// @note \b Global \b Function.
///
/// @param[in]  StateLlc SWP_LLC_layer structure
/// @param[in]     pBuffer pointer to the LLC_SHDLC control Word
/// \retval 2 cases: 1 #SWP_LLC_REJ_FRAME, 0 #SWP_LLC_OK
///
//-------------------------------------------------------------------------
u8 Swp_Llc_Shdlc_CheckReceivedIframeNsNr ( void )
{
    u8    NsClf;
	  u8    NrClf;

    NsClf = (g_SWP_LLC_Context.ReadLpdu->Data[0] & SHDLC_NS_MASK)>>3;       // eq to Ix,y ==> x
    NrClf = (g_SWP_LLC_Context.ReadLpdu->Data[0] & SHDLC_NR_MASK);           // eq to Ix,y ==> y
    // check if there is a hole in reception
    if (NsClf != g_SWP_LLC_Context.Nr)
    {
				if (NsClf != g_SWP_LLC_Context.privNr)
				{
						Swp_Llc_Send_StypeFrame(SHDLC_REJ); // Send S Frame with Nr reject
				}
				else
				{
					    Swp_Llc_Send_StypeFrame(SHDLC_RR);
				}
        return SWP_LLC_REJ_FRAME;
    }
    if (NrClf != g_SWP_LLC_Context.Ns)
    {
        ;// To be done
    }
    // Check T1 activity    ==> this way we deactivate T2
	  if ( g_SWP_LLC_Context.Timer.Status != T1_ACKNOWLEDGE_TIMEOUT )
    {
        swpTimer_SwpTimerStop( );
        // Activate T1
        swpTimer_SwpTimerStart( T1_ACKNOWLEDGE_TIMEOUT );
    }

	  if (NsClf == g_SWP_LLC_Context.Nr)
	  {
		    g_SWP_LLC_Context.privNr = g_SWP_LLC_Context.Nr;
	    // Update Nr for next time. It means that this function is used once it has received one frame
	      g_SWP_LLC_Context.Nr = (g_SWP_LLC_Context.Nr +1 ) %8;
	  }
    // Check DN(R)
    Swp_Llc_Shdlc_CheckDNR( NrClf,SHDLC_REJ_FALSE);
    return 0x0;
}
//-------------------------------------------------------------------------
//  Function Name : SWP_LLC_SHDLC_Receive
/// @brief  This function receive ACT frame through the SWP_LLC layer.
/// @note \b Global \b Function.
///
/// @param[in]  StateLlc SWP_LLC_layer structure
/// @param[out] pBuffer pointer address were are located data to receive
/// @param[out] pLength of the frame to receive
///
//-------------------------------------------------------------------------
u8 Swp_Llc_Shdlc_Receive( void )
{
    u8 result ;
	  result  =  SWP_LLC_OK;

    if ( g_SWP_LLC_Context.Status >= SWP_LLC_ACT_SYNC_SENT )
    {
        // Check if we have received Uframe Control Command
        if (((g_SWP_LLC_Context.ReadLpdu->Data[0] & SHDLC_U_FRAME) == SHDLC_U_FRAME))
        {
            //Stop   T3_CONNECTION_TIMEOUT
            swpTimer_SwpTimerStop();
            // Check if WS is Equal to 2 and no Support of Srej
            if (( g_SWP_LLC_Context.ReadLpdu->Data[0] & 0x1f ) == SHDLC_RSET)
            {
				if (g_SWP_LLC_Context.ReadLpdu->Size >= 2)
				{
					if ((g_SWP_LLC_Context.ReadLpdu->Data[1] > DEFAULT_WS) || (g_SWP_LLC_Context.ReadLpdu->Data[1] < 2) /*||DEFAULT_WS < g_SWP_LLC_Context.ReadLpdu->Data[1]*/)
					{
							g_SWP_LLC_Context.WS = DEFAULT_WS ;	// Set WS value to default value
						  g_SWP_LLC_Context.Status = SWP_LLC_SHD_RSET_RECEIVED;	//resend REST frame
					}
					else if (DEFAULT_WS > g_SWP_LLC_Context.ReadLpdu->Data[1] )
					{
						  g_SWP_LLC_Context.WS = g_SWP_LLC_Context.ReadLpdu->Data[1];// Set WS value to CLF value
						  g_SWP_LLC_Context.Status = SWP_LLC_SHD_RSET_SENT;//resend REST frame
					}
				  else
				{
						  g_SWP_LLC_Context.Status = SWP_LLC_SHD_RSET_SENT;
				}

					if ( (0x03 == g_SWP_LLC_Context.ReadLpdu->Size) && g_SWP_LLC_Context.ReadLpdu->Data[2] != DEFAULT_SREJ )
					{
						  g_SWP_LLC_Context.Status = SWP_LLC_SHD_RSET_RECEIVED;		//resend REST frame
					}
				}
				else if (0x01 == g_SWP_LLC_Context.ReadLpdu->Size)
                {
                    g_SWP_LLC_Context.WS = DEFAULT_WS ;// Set WS value to default value
					          g_SWP_LLC_Context.Status = SWP_LLC_SHD_RSET_SENT;//g_SWP_LLC_Context.Status = SWP_LLC_SHD_RSET_RECEIVED ;
                }
                // At the reception of RSET SHDLC control Ns and Nr has to be reset
				        //must clear value before send first frame
                g_SWP_LLC_Context.Ns = 0;
                g_SWP_LLC_Context.Nr = 0;
                g_SWP_LLC_Context.DNr = 0;
                // send the answer to the CLF
				// avoid problems of reentrance
				Swp_Llc_Shdlc_Send( );
				result = 0x2;
            }
            else if ((g_SWP_LLC_Context.ReadLpdu->Data[0] & 0x1F ) == SHDLC_UA )
            {
                if ((g_SWP_LLC_Context.Status == SWP_LLC_SHD_RSET_RECEIVED) || (g_SWP_LLC_Context.Status == SWP_LLC_SHD_RSET_SENT ))
                {
                    //shdlc link have established, goto init pipes
                    g_SWP_LLC_Context.Status = SWP_LLC_SHD_UA ;

					if (g_HCP_Context.Status == SESSION_INIT_PROCESS)
					{
						  SwpHcpSessionInit();//只有第一次RSET帧时才需要做初始化，否则只重置帧号
					}
					g_HCP_NeedProcess++;
					result = 0x2;
                }
                else
                {
                    return result;
                }
            }
			
			if (g_SWP_LLC_Context.Status == SWP_LLC_SHD_UA)
			{
				if(g_SWP_LLC_Context.PowerMode == SWP_LLC_LOW_POWER_MODE)
				{
					LowPower_SetConfig();
					g_HCP_Context.Status = SESSION_INIT_FINISH;
				}
			}
			
        }
        else if (((g_SWP_LLC_Context.ReadLpdu->Data[0] & 0xE0 ) == SHDLC_S_FRAME)) // Check if we have received Sframe Control Command
        {
            // Check S frame differents cases and resend corresponding Frame
            switch (( g_SWP_LLC_Context.ReadLpdu->Data[0] & TYPE_SFRAME_MASK ) >> 3)
            {
					case SFRAME_RR:
						Swp_Llc_Shdlc_CheckDNR((g_SWP_LLC_Context.ReadLpdu->Data[0] & SHDLC_NR_MASK), SHDLC_REJ_FALSE);

						// if previous Sframe was RNR let's resend if any forecasted frame
						  if ( (g_SWP_LLC_Context.Status == SWP_LLC_READ_RNR_S_FRAME) )
						  {
							g_SWP_LLC_Context.Status = SWP_LLC_READ_RR_S_FRAME;

							//当收到的前一帧为RNR时，后续收到RR帧，如果没有数据需要发送，则一直返回空帧
							// ETSI 102 613(V11) 10.7.7: The RR frame that follows an RNR frame shall be retransmitted every 5 to 20 ms
							//(defined from the last bit of the EOF to the first bit of the SOF) until a new I frame is received;
							//If the entity that received the RR has no more data to send, it shall send
							//an I-frame with empty information field to signal the proper reception of the RR frame.
							if (SWP_HCP_IsMessageSent())
							{
								g_SWP_LLC_Context.WriteLpdu->Size = 0;
								Swp_Llc_Shdlc_Send( );
							}
						  }
							else if (g_SWP_LLC_Context.Status == SWP_LLC_READ_REJ_S_FRAME)
							{
								//if privous receive frame is REJ frame, then check REJ frames is send finish
								// check exist need to resend frame, than resend them
								//如果前一帧收到的是REJ帧，则需要重发从REJ帧开始的所有帧，而且每一帧都需要RR帧确认
								g_SWP_LLC_Context.ResendFrameNum = g_SWP_LLC_Context.ReadLpdu->Data[0] & SHDLC_NR_MASK;
								swpTimer_SwpTimerStart(T_REJ_FRAMES_RESEND);
							}
							break;
					case SFRAME_REJ:
							 g_SWP_LLC_Context.Status = SWP_LLC_READ_REJ_S_FRAME;
							// Compare received Ns and NR
							 Swp_Llc_Shdlc_CheckDNR((g_SWP_LLC_Context.ReadLpdu->Data[0] & SHDLC_NR_MASK), SHDLC_REJ_TRUE);
							//if have frame resend, then set resend flag, wait these frames send finish
							 Swp_Llc_Shdlc_Resend( g_SWP_LLC_Context.ReadLpdu->Data[0] & SHDLC_NR_MASK );
							 break;
					case SFRAME_RNR:
														///< We have to Stop transmission and to wait to the next RR
														// first stop next transmission(s) if pending
									g_SWP_LLC_Context.Status = SWP_LLC_READ_RNR_S_FRAME;
									if (g_SWP_LLC_Context.Timer.Status == T2_GUARDING_TIMEOUT)			//如果重发机制已启动，则停止重发
									{
										  swpTimer_SwpTimerStop( );
									}
									break;

					case SFRAME_SREJ:
									break;///< this mode is not supported
		    }
      }
      else if ((g_SWP_LLC_Context.ReadLpdu->Data[0] & 0xc0) == SHDLC_I_FRAME) // Check if we have received Iframe Control Command
      {
            if ( g_SWP_LLC_Context.Status < SWP_LLC_SHD_UA )   // error test 5.6.2.2.2_6
			      {
				         return result;
			      }
            // suppress LLC_CONTROL_Field and transfer at the upper Level
            // 1st check if we have No hole in reception and transmission
            result = Swp_Llc_Shdlc_CheckReceivedIframeNsNr();
            if ( !result )
            {
				g_SWP_LLC_Context.Status = SWP_LLC_READ_I_FRAME;
                result = SWP_HCP_ProcessPacket();  // we assume that all non-empty I-frames are HCP packets
		    }
      }
      else
      {
            // Error
            result = SWP_LLC_SHD_RECEIVED_ERROR;
      }
	 }

    g_SWP_MAC_Context.CptRead = 1; //this is the first available byte  without llc_ctrl_byte
     return result;
}

//-------------------------------------------------------------------------
//  Function Name : SWP_LLC_SHDLC_Send
/// @brief  This function send frame through the SWP_LLC layer, for SHDLC mode.
/// @note \b Global \b Function.
///
/// @param[in]  StateLlc SWP_LLC_layer structure
/// @param[in] pBuffer pointer address were are located data to sent
/// @param[in]Length of data  were are located data to sent
///
/// \retval 2 cases: 1 SWP_LLC_NO_TX_FRAME_FREE, 0 SWP_LLC_OK
//-------------------------------------------------------------------------
u8 Swp_Llc_Shdlc_Send( void )
{
    PX8 pLocBuff;
	  u8  result;
    result = 0x0;
    // This part is for SHDL_Establishment
    if ((g_SWP_LLC_Context.Status == SWP_LLC_SHD_RSET_RECEIVED))
    {
         pLocBuff   =  ( u8 * )g_SWP_LLC_Context.WriteLpdu;
        *pLocBuff++ = 3; // size is first byte of ram buffer
        *pLocBuff++ = SHDLC_U_FRAME | SHDLC_RSET ;  // SEt the SHDLC RSET_command
        *pLocBuff++ = g_SWP_LLC_Context.WS;         // Write the Window Sliding WS correctly
        *pLocBuff   = DEFAULT_SREJ;                 // Write the Srej Support in the RSET Frame

        result  = Swp_Mac_SendFrame(MEMtPX8(&pLocBuff));
        swpTimer_SwpTimerStart(T3_CONNECTION_TIMEOUT); // We should start the timer with connection timeout setting
        g_SWP_LLC_Context.Status = SWP_LLC_SHD_RSET_SENT ; //change the status now it should be SWP_LLC_ACT_SYNC_SENT
    }
    else if (g_SWP_LLC_Context.Status == SWP_LLC_SHD_RSET_SENT )
    {
        g_SWP_LLC_Context.WriteLpdu->Data[0]= SHDLC_U_FRAME | SHDLC_UA ;// Set the UA_command
        g_SWP_LLC_Context.WriteLpdu->Size = 1;
        result = Swp_Mac_SendFrame(MEMtPX8(&g_SWP_LLC_Context.WriteLpdu));
        swpTimer_SwpTimerStop( );// Now we can stop T3_connection timeout
		    //shdlc link have established, goto init pipes
        g_SWP_LLC_Context.Status = SWP_LLC_SHD_UA ;    //change the status now it should be SWP_LLC_SHD_UA
		if (g_HCP_Context.Status == SESSION_INIT_PROCESS)
		{
			 SwpHcpSessionInit();	//只有第一次RSET帧时才需要做初始化，否则只重置帧号
	    }
	    g_HCP_NeedProcess++;
    }
    else if (g_SWP_LLC_Context.Status >= SWP_LLC_SHD_UA )            // This part is for Etablished SHDLC
    {
		 //__disable_irq();
		// Write LLC_CONTROL_FIELD
		g_SWP_LLC_Context.WriteLpdu->Data[0] = (SHDLC_I_FRAME | ((g_SWP_LLC_Context.Ns & 0x07)<<3) | g_SWP_LLC_Context.Nr) ;
		g_SWP_LLC_Context.WriteLpdu->Size += 1;
		//把当前发送帧备份，重发此帧时用
		Swp_Llc_StoreFrame(&g_SWP_LLC_Context.WriteLpdu->Data[0], SWP_LLC_SendLpduSize());
		swpTimer_SwpTimerStop( );
		if ( g_SWP_LLC_Context.Status == SWP_LLC_READ_RNR_S_FRAME )
		{
			g_SWP_LLC_Context.Status = SWP_LLC_RNR_S_FRAME_RESEND;
			//__enable_irq();
			return SWP_HCP_BAD_COMMUNICATION;
		}
		
		result = Swp_Mac_SendFrame(MEMtPX8(&g_SWP_LLC_Context.WriteLpdu));
		//__enable_irq();
		
		if ( !result )
		{
				g_SWP_LLC_Context.Ns = ((g_SWP_LLC_Context.Ns + 1) %8);// update NS
		}
		swpTimer_SwpTimerStart(T2_GUARDING_TIMEOUT);
				// *** Check if ret_value is effectively sent. Start the TIMER for Acknowledge timeout
		g_SWP_LLC_Context.Status = SWP_LLC_SHD_I_FRAME ;    // Modify the status to sent i Frame
    }
	g_SWP_MAC_Context.CptWrite  = 1; //this is the first available byte  without llc_ctrl_byte
    return result;
}

/****************************following functions are for ACT*****************************************/

/*-------------------------------------------------------------------------
 Function Name : SWP_LLC_ACT_ChangeMode
 @brief Change SWP ACTivation mode to be able to answer to SWP activate Event\n
 at highest baudrate. The change is done only if the product still wait for \n
 it's first activation. Then the SWP peripheral is stopped and restarted.\n
 after the restart, ve verify if an ACTivation occured and modify a Semaphore\n
 in this case.
-------------------------------------------------------------------------*/
void Swp_Llc_Act_ChangeMode(void)
{
    u8 i;

	  Swp_CheckActiveAndHandle();
    // After a SWP activation SWP clock should start when UICC resume SWP.
    if (g_SWP_LLC_Context.Timer.Status == TS2_INHIBIT_TIMEOUT )
    {
        swpTimer_SwpTimerStop();
    }
    if(isActivateActSent())
    {
        return ;
    }

	  Swp_StopSwpRun();
    // Clear SWP_RUN_BIT Stop SWP clock
    //r_SWP_CTRL2_BIT(SWP_CTRL2_RUN_BIT)=0 ;

    // update IrqOccured flag
    g_SWP_LLC_Context.IrqOccured = 1;
    // reinitialise LLC_context to be aligned with SWP restart
	  g_SWP_MAC_Context.CurRead = 0;
	  g_SWP_MAC_Context.CurWrite = 0;
  	g_SWP_MAC_Context.CptWrite = 0;
    // reinitialize part of SWP_MAC layer to be aligned with SWP restart
    g_SWP_MAC_Context.Status = SWP_MAC_LAYER_DEACTIVATED;
    g_SWP_MAC_Context.LastSent_Frame = 0;
    g_SWP_MAC_Context.Received_Frame= 3;
    g_SWP_MAC_Context.Nb_FrameToTransmit = 0;

    Swp_StartSwpRun();

    // This loop is manadatory to lets time for SWP register correct update
    i = 21;
    while ( i -- )
    {
        __nop();
    }

	Swp_CheckSuspendAndHandle();
    return;
}

u8 Swp_Llc_Act_Receive(void)
{
    u8 result;
	u16 i;
	result = SWP_LLC_OK;
    // Correct ETSI TS102 694-2 test 5.6.3.2.3 TER 4770.
    if (g_SWP_LLC_Context.Status >= SWP_LLC_SHD_RSET_SENT )
	{
        return result;
	}
    if (g_SWP_LLC_Context.ReadLpdu->Data[0] & ACT_POWER_MODE ) 
	{
	  swpTimer_SwpTimerStop();//stop TS2_INHIBIT timer.
	  if((g_SWP_LLC_Context.ReadLpdu->Data[0] & ACT_FR_SET ))
	  {   
			if (!Swp_Llc_Act_Send())
			{
					// Need to resend Sync ID Frame.
					g_SWP_LLC_Context.Status = SWP_LLC_ACT_SYNC_SENT;
			}
			else
			{
					result = SWP_LLC_ACT_READY_SENT_ERROR;
			}
	  }
	  else
	  {
			if (g_SWP_LLC_Context.ReadLpdu->Size == 0x01 || g_SWP_LLC_Context.ReadLpdu->Data[1] == 0x01)
			{
				  g_SWP_LLC_Context.PowerMode = SWP_LLC_FULL_POWER_MODE;
				 // SwpHardwareHandleFullPower();
			}
							// no need to resend Sync ID Frame. lets send ACTReady
			g_SWP_LLC_Context.Status = SWP_LLC_ACT_READY_SENT ;
			if (Swp_Llc_Act_Send())
			{
									//wait for one fream ready to send but this should be a problem
				 g_SWP_LLC_Context.Status = SWP_LLC_ACT_READY_SENT_ERROR;
				 result = SWP_LLC_ACT_READY_SENT_ERROR;
			}
	  }
	  if ( g_SWP_LLC_Context.PowerMode == SWP_LLC_FULL_POWER_MODE )
	  {
		  ecpSysHal_ManageSetClockConfig(0);
	  }
	}
		
    return result;
}

//-------------------------------------------------------------------------
//  Function Name : SWP_LLC_ACT_Send
/// @brief  This function send frame through the SWP_LLC layer.
/// @note \b Global \b Function.
///
/// @param[in]  StateLlc SWP_LLC_layer structure
///
/// \retval 2 cases: 1 SWP_LLC_NO_TX_FRAME_FREE, 0 SWP_LLC_OK
//-------------------------------------------------------------------------
u8 Swp_Llc_Act_Send (void)
{
    u8 * pBuffer;
    u8 result  = SWP_LLC_OK;

    pBuffer = (u8 *)g_SWP_LLC_Context.WriteLpdu;

    if (( g_SWP_LLC_Context.Status == SWP_LLC_ACT_SYNC_SENT ) || (g_SWP_LLC_Context.Status == SWP_LLC_OPENED))
    {
        *pBuffer++ = 4 ; // size is the first byte in ram buffer
        // SEt the ACT_command
        *pBuffer++ = ACT_FIELD | ACT_INF_SET | ACT_CTRL_SYNC ;
        // copy the ACT_DATA correctly
        *pBuffer++ = g_HCP_Context.NvmContext->SyncId.u1SId[0]; // endianness
        *pBuffer++ = g_HCP_Context.NvmContext->SyncId.u1SId[1]; // endianness
        *pBuffer   = U32t8(BITTOBYTES( SWP_LLC_590NS_SUPPORTED));//lmc,sup590ns

		 result = Swp_Mac_SendFrame(MEMtPX8(&pBuffer));
        //change the status now it should be SWP_LLC_ACT_SYNC_SENT
        g_SWP_LLC_Context.Status = SWP_LLC_ACT_SYNC_SENT ;
    }
    else if ( g_SWP_LLC_Context.Status == SWP_LLC_ACT_READY_SENT )
    {
        *pBuffer++ = 1 ; // size is the first byte in ram buffer
        // SEt the ACT_command
        *pBuffer++ = ACT_FIELD | ACT_CTRL_READY ;
        result = Swp_Mac_SendFrame(MEMtPX8(&pBuffer));
//	    swpActivateSuccess = 1;
    }

    return result;
}

/****************************following functions are for CLT*****************************************/
static u8 SwpCltMifareCheck()
{
    /* Mifare CL_PROTO_INF(A) frames have length 3 (SWP LLC control field + 2 bytes Mifare command) */
    if (g_SWP_LLC_Context.ReadLpdu->Size != 3)
	  {
        return 0;
    }
    /* Check for READ 0: "30 00" */
    if (g_SWP_LLC_Context.ReadLpdu->Data[1] == 0x30 && SWP_LLC_GetLpduData()[2] == 0x00)
	  {
        return 1;
    }
    /* Check for AUTH A/B: "6[01] .." */
    if ((g_SWP_LLC_Context.ReadLpdu->Data[1] & 0xfe) == 0x60)
	  {
        return 1;
    }
    return 0;
}

static u8  SwpProcessCltFrame(u8 ** OutputClt , u8 * OutputCltSize )
{
	  u8 CltAdminField ;
	  CltAdminField = g_SWP_LLC_Context.ReadLpdu->Data[0] & SWP_LLC_CLT_ADMIN_FIELD_MASK;

     if ( CltAdminField )
    {
        // manage administrative commands
    
        // if we receive a CL_PROTO_INF(A) we start a new session and select the right application
        if ( CltAdminField == SWP_LLC_CLT_CL_PROTO_INF_A )
        {
            // select MIFARE Classic if and only if CL_PROTO_INF(A) is byte aligned and first byte of payload is 0x6X
            if (   ( g_SWP_LLC_Context.ReadLpdu->Data[0] &  SWP_LLC_CLT_DATA_FIELD_MASK )     // byte aligned
                && ( ( g_SWP_LLC_Context.ReadLpdu->Data[1] & RFDISP_MIFARE_CLASSIC_AUTH_CMD_MASK ) == RFDISP_MIFARE_CLASSIC_AUTH_CMD_BYTE ) )
            {
                // select MIFARE application
                g_RFDISP_Context.SelectedCltApplication = RFDISP_CLT_MIFARE_SELECTED;
            }
            else
            {
                // select MIFARE application
                g_RFDISP_Context.SelectedCltApplication = RFDISP_CLT_NONMIFARE_SELECTED;
                return RFDISP_NOK;
            }
        }
        else if ( CltAdminField == SWP_LLC_CLT_CL_PROTO_INF_F )
        {
            // if a CLT application was selected, deselect it
            g_RFDISP_Context.SelectedCltApplication = RFDISP_CLT_NONMIFARE_SELECTED;
            // ignore the frame. Should we answer with a CL_GOTO_INIT ?
            return RFDISP_NOK;
        }
        else
        {
            // if a CLT application was selected, deselect it
            g_RFDISP_Context.SelectedCltApplication = RFDISP_CLT_NONMIFARE_SELECTED;
            // ignore RFU administrative commands ( ETSI TS 102 613 11.6.2 )
            return RFDISP_NOK;
        }
    }
        
    // Now forward frame to the right application.
    // Only frames whose ADMIN_FIELD is void or CL_PROTO_INF(A) are forwarded.
    if ( g_RFDISP_Context.SelectedCltApplication == RFDISP_CLT_MIFARE_SELECTED )
    {
        tMifareClassicAPIReturnCode retClassic;

        // call MIFARE Classic API
        *OutputCltSize = g_SWP_LLC_Context.ReadLpdu->Size;
		
		//兼容性方案
		if (*OutputCltSize == 0x02)
		{
			u8 clShortFrame;
			clShortFrame = g_SWP_LLC_Context.ReadLpdu->Data[1];
			
			if ( (clShortFrame == 0x26) || (clShortFrame == 0x52) )
			{
				if (PN_R_U8(JcvmConfig_GetSysConfigAddr(CLF_MODEL_ID)) == 0)
				{
					*OutputCltSize = 0;
					return RFDISP_NOK;
				}
			}
		}
		
    retClassic = MifareClassicAPI_ExchangeClt(   ( void * ) g_SWP_LLC_Context.ReadLpdu->Data,
                                                     ( tMifareClassicBytePointer ) &g_HCP_Context.WriteMessage.Buf[0] ,
                                                   ( tMifareClassicBytePointer ) OutputCltSize );
		if ( retClassic != cMifareClassicAPIReturnCodeSuccess )
		{
				*OutputCltSize = 0;
				return RFDISP_OK;
		}
		*OutputClt =  &g_HCP_Context.WriteMessage.Buf[0];
    }
    else if ( g_RFDISP_Context.SelectedCltApplication == RFDISP_CLT_NONMIFARE_SELECTED )
    {
        *OutputCltSize = 0;
        return RFDISP_OK;
    }
    else
    {
        // no selected application yet, error in CLT protocol, ignore the LPDU ( cf. ETSI TS 102 613 11.6.2 )
        return RFDISP_NOK;
    }

   return RFDISP_OK;
}


u8 Swp_Llc_Clt_Receive(void)
{
    u8 *OutputClt;
    u8 OutputCltSize;

// 	//Mifare disable ,lmc
 	if (MIFARE_SWITCH != 0x00)
 	{
         return SWP_LLC_CLT_OK;
 	}

	//注意：必须延时，解决兼容性问题
	// Delay response for ca. 100 us
 	if (PN_R_U8(JcvmConfig_GetSysConfigAddr(CLF_MODEL_ID))== 0)
 	{
 		  ecpSwpTimeDelay(100);
 	}

	SwpProcessCltFrame( &OutputClt , &OutputCltSize );

	if (( OutputCltSize == 0 )|| ( OutputCltSize > LLC_BUF_SIZE))
	{
		return SWP_LLC_CLT_NOK;
	}
	else
	{
		// copy data from OS to SWP Ram buffer
		ecpMem_Copy ( g_SWP_LLC_Context.WriteLpdu->Data , OutputClt , OutputCltSize );
		g_SWP_LLC_Context.WriteLpdu->Size = OutputCltSize;
		Swp_Mac_SendFrame(MEMtPX8(&g_SWP_LLC_Context.WriteLpdu));// sends back the answer CLT frame to the CLF
	}
	
	MifareClassicAPI_LibraryMaintenance();
	
	return SWP_LLC_CLT_OK;
}
#endif
