#ifndef __APP_FILE_H_
#define __APP_FILE_H_

#include "profile.h"
#include "uicc_main.h"
#include "uicc_error.h"

#if JAVA_CARD_UICC

#include "ceph.h"
#include "SimApi.h"
#include "PatchHal.h"
#include "arithmatic.h"
#include "interface.h"
#include "fileSystem.h"
#include "fileAction.h"
#include "TransFormat.h"
#include "ToolkitRFM.h"
#include "ToolkitAPIPublic.h"
#include "Authentication.h"
#include "LiveWirteCardXL.h"
#include "USATEnvelopeHandler.h"
#include "TimerModuleProces.h"
#include "ExpandedManage.h"

#include "uicc_interface.h"
#include "Interface_AnnexB.h"

#include "hard_api.h"
#include "T0ApduImpl.h"
#include "stkhead.h"
#include "stkalert.h"
#include "STKFileView.h"
#include "VMAccessEeprom.h"
#include "STKNativeMethod.h"
#include "STKToolkitInterface.h"
#include "STKRemoteAppManagement.h"


#include "platform_api.h"
#include "SCP03Realize.h"
#include "ASN1Operation.h"
#include "gsmpublic.h"


#include "EUICCMain.h"
#include "EUICCRAM.h"
#include "EUICCM2MAPDU.h"
#include "EUICCM2MAPDUPublic.h"
#include "EUICCCustomer.h"
#include "EUICCProfile.h"
#include "EUICCCustomerPublic.h"
#include "EUICCCustomerCertificate.h"
#include "ProfileGPCommand.h"
#include "ProfileInterpreter.h"
#include "SEEUICCInterface.h"


extern u8 recLogicalChninfo(void);
extern void bakLogicalChninfo(void);
extern void Clearresponse(void);
extern void b616crecflag_init(void);



#endif
//#if JAVA_CARD_UICC||BOOTLOADER_PRO
//extern  void apdu_SendStatusAndReceiveCommandSingle(u16 status);
//extern  void io_SendStatusAndReceiveCommandSingle(u16 status);
//#endif
#if TIMERMODULE
#include "TimerModuleProces.h"
#endif

#if UICC__APP__WHITECARD
#include "WhiteCardBeWrited.h"
#include "LiveWirteCardXL.h"
#endif


#if (GP__VERSION__FOR_CHINA_TELECOM_MASK || GP__VERSION__FOR_CHINA_UNICOM_MASK)
#include "fileSystem.h"
#include "fileAction.h"
#endif

#if	GP__AMENDMENT__B__FOR_MASK
#include "BIPModuleProcess.h"
#include "HttpProcess.h"
#include "TlsMainFlow.h"
#include "TlsAlgorithm.h"
#include "Scp81Connection.h"
#include "GPSCP81KeyManagement.h"
#include "GPAnnexbAdminSessionPara.h"
#include "Interface_AnnexB.h"
#endif


#ifdef  COS_Update
#include "Interface_eSEPrivateAPI.h"
#endif

#if UICC__FUNCTION__BERTLVCmd
#include "DataOrientedCommand.h"
#endif

#if UICC__FUNCTION__CATTP
#include "CatTp.h"
#endif

#if M2M_EUICC
#include "uicc_M2M.h"
#include "EUICCM2MAPDU.h"
#endif

#include "uicc_main.h"
#include "Interface_LTEFromBoot.h"
#endif
