/***************************************************************************************
 * Copyright 2012 Eastcompeace. Co., Ltd.                                              *
 * All rights reserved.                                                                *
 *                                                                                     *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,            *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,           *
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  IN NO       *
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,                 *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;         *
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY             *
 * WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR            *
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF              *
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                          *
 *                                                                                     *
 ***************************************************************************************/

/*  includes
****************************************************************/


#include "SwpTimer.h"
#include "SwpTimerHal.h"
#include "Swp_Llc_Layer.h"
#include "SwpHal.h"

/***************************************************************************/
/// @defgroup SWP_TIMER The SWP Timer module.
/// All SWP_TIMER functions used to manage the Waiting time in SWP protocol
/// @ingroup SWP_Module
///
/***************************************************************************/
#if BOOT__IO__SWP
u8 isSwpTimerRunning(void)
{
	return (g_SWP_LLC_Context.Timer.Status != NO_TIMEOUT);
}


//-------------------------------------------------------------------------
//  Function Name : Swp_TimerStart
/// @brief  Starts a SWP timer.
/// CPU clock is set up at div4 jittered.
///
/// @param[in] SwpTimer : SWP timer as defined in enumeration ShdlcTimeT
///
//-------------------------------------------------------------------------
void swpTimer_SwpTimerStart( u8 swpTimerType )
{
	  u16    timerCounter;

    switch(swpTimerType)
    {
        case T1_ACKNOWLEDGE_TIMEOUT: // T1 <= 1.25ms * WS ( and take a bias to take into account IT handler processing ))
           timerCounter = 0x500;
           g_SWP_LLC_Context.Timer.Status = T1_ACKNOWLEDGE_TIMEOUT;
           break;

        case T2_GUARDING_TIMEOUT:    // T2 >= 10 ms. Since SWP_TIMER_NBCYCLES_FOR_1_25us is < to 1.25ms we have to take a conservative
                                     // multiplicative factor to be sure we are >= 10ms
           timerCounter = 0x2500;
           g_SWP_LLC_Context.Timer.Status = T2_GUARDING_TIMEOUT;
           break;

        case T_REJ_FRAMES_RESEND:
            timerCounter = 600;
            g_SWP_LLC_Context.Timer.Status = T_REJ_FRAMES_RESEND;
            break;

        case T3_CONNECTION_TIMEOUT:  // T3 > 5 ms
            timerCounter = 0x1594;
            g_SWP_LLC_Context.Timer.Status = T3_CONNECTION_TIMEOUT;
            break;

        case TS2_INHIBIT_TIMEOUT:  // TS2_INHIBIT   < 100 ms
            timerCounter = 0xF000;
            g_SWP_LLC_Context.Timer.Status = TS2_INHIBIT_TIMEOUT;
            break;
        default:
					  timerCounter = 0x0;
            break;
    }

	   ecpStartSwpTimer(timerCounter);
}

//-------------------------------------------------------------------------
//  Function Name : Swp_TimerStop
/// @brief  Stops a SWP timer. It stops the corresponding physical timer
/// and update library internal variables.
///
/// @param[in] SwpTimer : SWP timer as defined in enumeration ShdlcTimeT
///
//-------------------------------------------------------------------------
void swpTimer_SwpTimerStop( void )
{
    ecpStopSwpTimer();
    g_SWP_LLC_Context.Timer.Status = NO_TIMEOUT;
}



//-------------------------------------------------------------------------
//  Function Name : Swp_Timer_Handler
/// @brief This function is called whenever Timeout Activities are reached on SWP peripheral.<br>
/// This function may be located in flash area.
///
/// \param[in] state #ManagerState object structure
///
/// @note \b Global \b Function.
///
//-------------------------------------------------------------------------
void swpTimer_SwpTimerHandle(void)
{
    ShdlcTimeT  curStatus ;
	
	curStatus = g_SWP_LLC_Context.Timer.Status;
	swpTimer_SwpTimerStop();
    //__disable_irq();
    switch( curStatus )
    {
        case T1_ACKNOWLEDGE_TIMEOUT:
							 Swp_Llc_Send_StypeFrame(SHDLC_RR);
							 break;
        case T2_GUARDING_TIMEOUT:
							// This means that CLF didn't acknoledge last transmitted frame
							// so we have to retransmit it
							 if (g_SWP_LLC_Context.NbRetransmit < SWP_LLC_MAXRETRANSMIT)
							 {
									 g_SWP_LLC_Context.NbRetransmit++;
									//Now we are in the retransmit the last sent frame case
									 Swp_Llc_Shdlc_Resend( 0xFF );
							 }
							 break;
        case T3_CONNECTION_TIMEOUT://复用NbRetransmit变量控制RESET重发次数
							 if (g_SWP_LLC_Context.NbRetransmit < SWP_LLC_MAXRETRANSMIT)
							 {
									 g_SWP_LLC_Context.NbRetransmit++;

											// We will resend RSET
											// to do so we set status of LLC_layer
												 g_SWP_LLC_Context.Status = SWP_LLC_SHD_RSET_RECEIVED;
					               Swp_Llc_Shdlc_Send( );
							 }
							 else
							 {
									 g_SWP_LLC_Context.NbRetransmit = 0;
							 }
               break;
			 case TS2_INHIBIT_TIMEOUT:
							// We will set S2 line to pull down if no SWP clock Has been detected ( no swp transmission occured)
							// Set S2 in pull down mode. This pull down mode will diseappered if reboot or if deactivate occured after.
				 	 	 	 	 Swp_HardWare_PullDownManagement();
								 g_SWP_LLC_Context.Status = SWP_LLC_CLOSED;// change LLC status to indicate an error
								 break;

			 case T_REJ_FRAMES_RESEND:
									if (Swp_Llc_Shdlc_Resend(g_SWP_LLC_Context.ResendFrameNum) == false)
									{
										g_SWP_LLC_Context.Status = SWP_LLC_READ_RR_S_FRAME;
									}
										break;
			 case NO_TIMEOUT:
						break;
			 default:
						break;
    }
	//__enable_irq();
}
#endif
