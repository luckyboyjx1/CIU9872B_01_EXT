#ifndef    _stkalert_h_
#define    _stkalert_h_

extern void PendingMessageAlert(BYTE ch,BYTE refNumber);

extern void ProcessPendingMessageAlert(void);
//extern void ClearingMessageAlert(BYTE ch);
//extern void ClearingAllMessageAlert(void);
extern void InitMessageAlert(void);
#endif

